/***************************************************************************//**
 * @brief   Meter logs manager.
 * @file    logger.c
 * @author  Eduardo Balbino
 * @date    09/10/2023
 * @details Module which manages (create, save and read) all meter logs.
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "hal.h"
#include "systick.h"
#include "framctl_a.h"
#include "logger.h"
#include <string.h>

#if FIXED_POINT_CALC
#include "IQmathLib.h"
#endif

#include "systick.h"
#include "temp.h"
#include "no_usage.h"
#include "fraud.h"
#include "leak.h"
#include "processing.h"
#include "configmng.h"
#include "battdate.h"
#include "brush.h"

#if DEBUG_LOGGER
#include <stdio.h>
#endif

#if CONFIG_LOGGER_ENABLED
///*******************************************************************************
// * Defines
// ******************************************************************************/
//
///** Number of FLASH pages used to store logs. */
//#define LOGGER_FLASH_NUM_PAGES                      4U
//
///** Size of logger buffer in bytes. */
#define LOGGER_BUFFER_SIZE                          (LOGGER_REG_SIZE*LOGGER_NUM_VALID_REGS)
//
///** Size of timestamp field in bytes. */
#define LOGGER_REG_FIELD_TIMESTAMP_SIZE             3U
//
///** Size of timestamp(+ minutes and seconds) field in bytes. */
#define LOGGER_REG_FIELD_TIMESTAMP_EXACT_SIZE             5U
//
///** Size of volume field in bytes. */
#define LOGGER_REG_FIELD_VOL_SIZE                   4U
//
///** Size of flow rate field in bytes. */
#define LOGGER_REG_FLOWRATE_SIZE                    4U
//
///** Size of alarms field in bytes. */
#define LOGGER_REG_FIELD_ALARMS_SIZE                1U
//
///** Size of log register in bytes. (20 bytes) */
#define LOGGER_REG_SIZE                (LOGGER_REG_FIELD_TIMESTAMP_SIZE + \
                                        LOGGER_REG_FIELD_VOL_SIZE + \
                                        LOGGER_REG_FIELD_VOL_SIZE + \
                                        LOGGER_REG_FLOWRATE_SIZE + \
                                        LOGGER_REG_FLOWRATE_SIZE + \
                                        LOGGER_REG_FIELD_ALARMS_SIZE)
//
///** Number of log registers per FLASH page. (25 logs) */
//#define LOGGER_REG_PER_PAGE             (FLASH_PAGE_SIZE / LOGGER_REG_SIZE)
//
///** Number of valid registers stored in log memory. */
#define LOGGER_NUM_VALID_REGS                       1500 // max ~= 2700. Totalling 8100 logs
//
///** Default value of the memory. */
#define LOGGER_DEFAULT_VALUE                        0x00
//

/*******************************************************************************
 * Structures
 ******************************************************************************/

/** Structure to hold timestamp in the log register. */
#pragma pack(push, 1)
typedef struct S_LOGGER_TIMESTAMP
{
    /** Year (from 1900) */
    uint16_t ui_year     : 8;
    /** Month (0 - 11) */
    uint16_t ui_month    : 4;
    /** Day (1 - 31) */
    uint16_t ui_day      : 5;
    /** Hour (0 - 23) */
    uint16_t ui_hour     : 5;
    /** Minutes in slices of 15 min each */
    uint16_t ui_minutes  : 2;
} s_logger_timestamp_t;
#pragma pack(pop)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

//static inline uint16_t getFirstIdxOfPage(uint16_t ui_currentIdx);
//static inline uint16_t getLastIdxOfPage(uint16_t ui_idx);
static inline bool isIdxOutOfLogMemory(uint16_t ui_idx);
static uint64_t calculateTickOfNextLog(s_logger_config_t *ps_config);
static inline bool isTimeToRegisterLog();
static inline void updateAlarmsStatus();
void logger_updateConfig();
static inline void createLog(uint8_t *puc_buffer, uint8_t uc_alarmFlags);
static inline bool saveLog(uint8_t *puc_buffer, uint16_t ui_idx);
static inline bool readLog(uint8_t * const puc_reg, uint16_t uc_idx);
static bool unpack(s_logger_reg_t * const ps_reg, uint16_t uc_idx);



#if DEBUG_LOGGER
static inline void printLogs();
#endif /* DEBUG_LOGGER */

/*******************************************************************************
 * Global variables
 ******************************************************************************/

/** Memory section where all logs shall be stored. */
//#pragma location="LOGS"
//#pragma PERSISTENT(gs_processing)

//#pragma DATA_SECTION(gauc_logBuffer, ".log_section")
#pragma DATA_SECTION(gauc_logBuffer1, ".log_section")
uint8_t gauc_logBuffer1[LOGGER_BUFFER_SIZE] = {0x00};
#pragma DATA_SECTION(gauc_logBuffer2, ".log_section")
uint8_t gauc_logBuffer2[LOGGER_BUFFER_SIZE] = {0x00};
#pragma DATA_SECTION(gauc_logBuffer3, ".log_section")
uint8_t gauc_logBuffer3[LOGGER_BUFFER_SIZE] = {0x00};

/*#pragma DATA_SECTION(gauc_logBuffer, ".log_alarm_report")
uint8_t gauc_logBuffer[LOGGER_BUFFER_SIZE] = {0};

#pragma DATA_SECTION(gauc_logBuffer, ".log_daily")
uint8_t gauc_logBuffer[LOGGER_BUFFER_SIZE] = {0};

#pragma DATA_SECTION(gauc_logBuffer, ".log_monthly")
uint8_t gauc_logBuffer[LOGGER_BUFFER_SIZE] = {0};*/

/** Log register. */
static s_logger_reg_t gs_reg;

/** Packed log register to be stored.  */
static uint8_t gauc_logReg[LOGGER_REG_SIZE];

/** Alarms flags to be stored. */
static uint8_t guc_alarmFlags = 0;

/** Index of the memory space to store the log. */
static uint16_t gui_memIdx = 0;

/** Indicates if the logger module is enabled. */
static bool gb_isEnabled = false;

/** Tick which a new log must be created. */
static uint64_t gull_tickToLog = SYSTICK_INVALID_TICK;

/** System configuration */
static s_sys_config_t *gps_sysConfig;

/** Indicates if system configuration has been changed. */
static bool gb_configChanged = false;

/** Logs counter. */
static uint16_t guc_numLogs;

/** Timestamp structure pointer. */
static struct tm *gps_timestamp;

/** Logger timestamp structure. */
static s_logger_timestamp_t gs_logTimestamp;

/** Meter's volume. */
static int64_t gl_volume;

/** Meter's flow rate. */
static int64_t gl_flowRate;

// static double_t fpu_helper;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize logger module.
 */
void logger_init()
{
    // Get system configuration pointer
    gps_sysConfig = configmng_getSysConfig();

    gps_timestamp = systick_getActualDateTime();

#if DEBUG_NORMAL
    gps_timestamp->tm_sec = 0;
    gps_timestamp->tm_min = 20;
    gps_timestamp->tm_hour = 5;
    gps_timestamp->tm_mday = 8;
    gps_timestamp->tm_mon = 12;
    gps_timestamp->tm_year = 123;
    systick_setActualDateTime(gps_timestamp);
#endif


    uint8_t ui_auxIdx;

    // Calculate the next memory index to store the next log
    while(gui_memIdx < 3*LOGGER_BUFFER_SIZE)
    {
        switch(gui_memIdx / LOGGER_BUFFER_SIZE)
        {
            case 0:
                ui_auxIdx = gauc_logBuffer1[gui_memIdx];
                break;
            case 1:
                ui_auxIdx = gauc_logBuffer2[gui_memIdx - LOGGER_BUFFER_SIZE];
                break;
            case 2:
                ui_auxIdx = gauc_logBuffer3[gui_memIdx - 2*LOGGER_BUFFER_SIZE];
                break;
        }
        if(0x00 != ui_auxIdx) // 0x00 is an empty log, so it just keeps moving until done.
        {
            gui_memIdx += LOGGER_REG_SIZE;
        }
        else
        {
            break;
        }
    }

    for(guc_numLogs = 0; guc_numLogs < 3*LOGGER_NUM_VALID_REGS; ++guc_numLogs)
    {
        // Unpack log register
        if(!unpack(&gs_reg, guc_numLogs))
        {
            // Invalid log register
            break;
        }
    }

    // Update configuration
    logger_updateConfig();

#if DEBUG_LOGGER
    printf("(LOGGER): Module init. Found %ul logs.\n", guc_numLogs);
#endif /* DEBUG_LOGGER */
}

/**
 * @brief
 *      Enable/Disable logger module.
 *
 * @note
 *      If this function is called to disable logger, it will disable the
 *      module, but if it's called to enable, the module will follow the
 *      logger configuration.
 *
 * @param[in]   b_enable
 *      Enable/Disable logger module.
 */
void logger_enable(bool b_enable)
{
    gb_isEnabled = b_enable;
}

/**
 * @brief
 *      Erase all logs.
 */

#if CONFIG_OPTCOMM_ERASE_LOGS
void logger_erase(void)
{

    FRAMCtl_A_fillMemory32(
        0x00,
        (uint32_t *) &gauc_logBuffer,
        (uint16_t) guc_numLogs
        );

    // Reset log count
    guc_numLogs = 0;

    // Reset memory index
    gui_memIdx = 0;

    // Reset alarm flags
    guc_alarmFlags = 0;
}
#endif

/**
 * @brief
 *      Update logging configuration.
 */
void logger_updateConfig()
{
    // Indicate that the configuration has been changed
    gb_configChanged = true;

#if DEBUG_LOGGER
    printf("(LOGGER): Configuration updated.\n");
#endif /* DEBUG_LOGGER */
}

/**
 * @brief
 *      Update logger's state.
 *
 * @details
 *      This function checks if it's time to register a log. If so, it creates a
 *      log register, pack it to fit the memory model, save it and calculate
 *      the tick for next log.
 */
void logger_update()
{
    if(gb_isEnabled)
    {
        // Check if the configuration has been changed
        if(gb_configChanged)
        {
            // Calculate tick to register the next log
            gull_tickToLog = calculateTickOfNextLog(&gps_sysConfig->s_logger);

            gb_configChanged = false;
        }

        // Check if logger is enabled
        if(gps_sysConfig->s_logger.uc_interval !=
            E_LOGGER_CONFIG_INTERVAL_DISABLED)
        {
            // Update alarms status
            updateAlarmsStatus();

            // Check if it's time to register a log
            if(isTimeToRegisterLog())
            {
                // Create and pack a log register
                createLog(gauc_logReg, guc_alarmFlags);

                // Save log register
                if(saveLog(gauc_logReg, gui_memIdx))
                {
                    // Check if it does not reached the maximum number of logs
                    if(guc_numLogs < 3*LOGGER_NUM_VALID_REGS)
                    {
                        guc_numLogs++;
                    }

                    // Update the memory index for the next log
                    gui_memIdx += LOGGER_REG_SIZE;

                    // Check if the index is out of the log memory
                    if(isIdxOutOfLogMemory(gui_memIdx))
                    {
                        // Update index for the begin of the log memory
                        gui_memIdx = 0;
                    }

                    // Clear alarm flags for the next log
                    guc_alarmFlags = 0;
                }

                // Calculate tick to register the next log
                gull_tickToLog = calculateTickOfNextLog(
                    &gps_sysConfig->s_logger);

#if DEBUG_LOGGER
                // Print all logs stored in the memory
                printLogs();
#endif /* DEBUG_LOGGER */
            }
        }
    }
}

/**
 *  @brief
 *      Get number of logs in stored in memory.
 *
 *  @return
 *      Number of log stored in the memory.
 */
uint16_t logger_size()
{
    return guc_numLogs;
}

/**
 *  @brief
 *      Read a log.
 *
 *  @param[out] ps_reg
 *      Pointer to structure where the log read from the Flash must be placed
 *      in.
 *
 *  @param[in]  uc_idx
 *      Index of the log to be read. (index zero represents the most recent
 *      log)
 *
 *  @return
 *      Returns TRUE if the log has been read with success or FALSE otherwise.
 */
bool logger_read(s_logger_reg_t * const ps_reg, uint16_t uc_idx)
{
    bool b_status = false;

    // Check if the log index is valid
    if(uc_idx < guc_numLogs)
    {
        // Unpack the log register
        b_status = unpack(ps_reg, uc_idx);
    }

    return b_status;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/

/**
 *  @brief
 *      Get the first index of the FLASH page.
 *
 *  @details
 *      Given an index, get the first index of the FLASH page which the given
 *      index belongs to.
 *
 *  @param[in]  ui_idx
 *      Current index which will be used to find the first index of the FLASH
 *      page which it belongs to.
 *
 *  @return
 *      The first index of the FLASH page which @ref ui_idx belongs to.
 */
//static inline uint16_t getFirstIdxOfPage(uint16_t ui_idx)
//{
//    return (ui_idx / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
//}

/**
 *  @brief
 *      Get the last index of FLASH page.
 *
 *  @details
 *      Given an index, get the lst index of the FLASH page which the given
 *      index belongs to.
 *
 *  @param[in]  ui_idx
 *      Current index which will be used to find the first index of the FLASH
 *      page which it belongs to.
 *
 *  @return
 *      The first index of the FLASH page which @ref ui_idx belongs to.
 */
//static inline uint16_t getLastIdxOfPage(uint16_t ui_idx)
//{
//    // Get the first index of the FLASH page
//    uint16_t ui_lastIdx = getFirstIdxOfPage(ui_idx);

    // Calculate the last index of the FLASH page
//    ui_lastIdx += ((LOGGER_REG_PER_PAGE - 1) * LOGGER_REG_SIZE);

//    return ui_lastIdx;
//}

/**
 *  @brief
 *      Check if a given index is out of the log memory.
 *
 *  @param[in]  ui_idx
 *      Memory index to check if it's out of the log memory.
 *
 *  @return
 *      Return 'true' if @ref ui_idx is out of the log memory. Otherwise, it
 *      returns 'false'.
 */
static inline bool isIdxOutOfLogMemory(uint16_t ui_idx)
{
    return (ui_idx == (3*LOGGER_BUFFER_SIZE));
}

/**
 *  @brief
 *      Calculate the tick of the next log.
 *
 *  @details
 *      Based on a given logger configuration, it calculates the tick which
 *      the meter shall register the next log. The available configurations
 *      are: hourly, every 6 hours, every 12 hours, daily and every week (in
 *      a specific day).
 *
 *  @param[in]  ps_config
 *      Logger configuration to be used to calculate the tick to next log.
 *
 *  @return
 *      Absolute tick which the meter shall register the next log.
 */
static uint64_t calculateTickOfNextLog(s_logger_config_t *ps_config)
{

    // Get current system timestamp
    gps_timestamp = systick_getActualDateTime();

#if DEBUG_LOGGER
    gps_timestamp->tm_sec = 0;
    gps_timestamp->tm_min += 1;
#else
    // Apart of the logger configuration, the meter shall register a log in
    // minute and second zero
    gps_timestamp->tm_sec = 0;

    switch(ps_config->uc_interval)
    {
        // Register a log every hour
        case E_LOGGER_CONFIG_INTERVAL_HOURLY:

            gps_timestamp->tm_min = 0;
            gps_timestamp->tm_hour++;

#if DEBUG_LOGGER
            printf("(LOGGER): Log every 1 hour.\n");
#endif /* DEBUG_LOGGER */
            break;

        // Register a log every 6 hours (even hours -> 0, 6, 12 and 18)
        case E_LOGGER_CONFIG_INTERVAL_QUARTER_DAILY:

            gps_timestamp->tm_min = 0;
            gps_timestamp->tm_hour = (gps_timestamp->tm_hour/6)*6;
            gps_timestamp->tm_hour += 6;

#if DEBUG_LOGGER
            printf("(LOGGER): Log every 6 hours.\n");
#endif /* DEBUG_LOGGER */
            break;

        // Register a log every 12 hours (even hours -> 0 and 12)
        case E_LOGGER_CONFIG_INTERVAL_HALF_DAILY:

            gps_timestamp->tm_min = 0;
            gps_timestamp->tm_hour = (gps_timestamp->tm_hour/12)*12;
            gps_timestamp->tm_hour += 12;

#if DEBUG_LOGGER
            printf("(LOGGER): Log every 12 hours.\n");
#endif /* DEBUG_LOGGER */
            break;

        // Register a log every day at midnight
        case E_LOGGER_CONFIG_INTERVAL_DAILY:

            gps_timestamp->tm_min = 0;
            gps_timestamp->tm_hour = 0;
            gps_timestamp->tm_mday++;

#if DEBUG_LOGGER
            printf("(LOGGER): Log every 1 day.\n");
#endif /* DEBUG_LOGGER */
            break;

        // Register a log every week at midnight of a specific day (given the
        // the logger configuration)
        case E_LOGGER_CONFIG_INTERVAL_WEEKLY:
        {
            int8_t i_dayDiffs;

            gps_timestamp->tm_min = 0;
            gps_timestamp->tm_hour = 0;

            // Calculate the difference of days from now until the day to
            // register the log
            i_dayDiffs = ps_config->uc_param - gps_timestamp->tm_wday;
            gps_timestamp->tm_mday += i_dayDiffs;

            // If the difference is negative, this means the day already passed,
            // so, schedule it for the next week
            if(i_dayDiffs <= 0)
            {
                gps_timestamp->tm_mday += 7;
            }

#if DEBUG_LOGGER
            printf("(LOGGER): Log every 1 week.\n");
#endif /* DEBUG_LOGGER */
        }
        break;

        // Register a log every month at midnight of a specific day (given the
        // the logger configuration)
        case E_LOGGER_CONFIG_INTERVAL_MONTHLY:
        {
            uint8_t uc_todayDay = gps_timestamp->tm_mday;
            gps_timestamp->tm_min = 0;
            gps_timestamp->tm_hour = 0;

            // Update next day logger
            gps_timestamp->tm_mday = ps_config->uc_param;

            // If the day to register the log is later in the current month, or today is the log day
            if (ps_config->uc_param <= uc_todayDay) {
                // Increase the month by one
                gps_timestamp->tm_mon += 1;
            }
#if DEBUG_LOGGER
            printf("(LOGGER): Log every month.\n");
#endif /* DEBUG_LOGGER */
        }
        break;

        case E_LOGGER_CONFIG_INTERVAL_15MIN:

            gps_timestamp->tm_min += 15;

#if DEBUG_LOGGER
            printf("(LOGGER): Log every 15 minutes.\n");
#endif /* DEBUG_LOGGER */

        break;
        // Disable the logger
        default:

            // Force the logger is disabled
            ps_config->uc_interval = E_LOGGER_CONFIG_INTERVAL_DISABLED;

            // Set timestamp which the meter will never reach it
            gps_timestamp->tm_year += 100;

#if DEBUG_LOGGER
            printf("(LOGGER): Logger disabled.\n");
#endif /* DEBUG_LOGGER */
            break;
    }
#endif /* DEBUG_LOGGER */



#if DEBUG_LOGGER
    if(ps_config->uc_interval != E_LOGGER_CONFIG_INTERVAL_DISABLED)
    {
        printf("(LOGGER): Next log: %02d/%02d/%02d - %02d:%02d:00\n",
            gps_timestamp->tm_mday, gps_timestamp->tm_mon + 1,
            gps_timestamp->tm_year - 100, gps_timestamp->tm_hour,
            gps_timestamp->tm_min);
    }
#endif /* DEBUG_LOGGER */

    // OFFSET correction given default Texas Implementation of Timezones
    gps_timestamp->tm_hour -= SYSTICK_CTS_TO_UTC_OFFSET;
    // Get absolute system tick for the given calendar
    return (uint64_t) mktime(gps_timestamp) * 1000;
}

/**
 *  @brief
 *      Check if it's time to register log.
 *
 *  @return
 *      Return 'true' if it's time to register a log. Otherwise, it returns
 *      'false'.
 */
static inline bool isTimeToRegisterLog()
{
    bool b_isTimeToLog = false;

    // Get current system tick
    uint64_t ull_systick = systick_getSysTickMs();

    // Check if it's time to register a log
    if(ull_systick >= gull_tickToLog)
    {
        b_isTimeToLog = true;
    }

    return b_isTimeToLog;
}

/**
 * @brief
 *      Update alarms status.
 *
 * @details
 *      Check all alarms and update the alarms status to register in next log.
 */
static inline void updateAlarmsStatus()
{
    // Check reverse flow
    if(proc_isReverseFlow())
    {
        guc_alarmFlags |= LOGGER_ALARM_FLAG_BACK_FLOW;
    }

    // Check leakage
    if(leak_isLeakage())
    {
        guc_alarmFlags |= LOGGER_ALARM_FLAG_LEAKAGE;
    }

    // Check battery
    if(battdate_isOld())
    {
        guc_alarmFlags |= LOGGER_ALARM_FLAG_LOW_BATTERY;
    }

    // Check air in pipe
    if(proc_isAirInPipe())
    {
        guc_alarmFlags |= LOGGER_ALARM_FLAG_AIR_IN_PIPE;
    }

#if CONFIG_TAMPER_ENABLED
    // Check fraud
    if(fraud_wasViolated())
    {
        guc_alarmFlags |= LOGGER_ALARM_FLAG_FRAUDED;
    }
#endif /* CONFIG_FRAUD_ENABLED */

    // Check maximum flow rate
    if(brush_isOverflowing())
    {
        guc_alarmFlags |= LOGGER_ALARM_FLAG_BRUSH;
    }

    // Check no usage alarm
    if(no_usage_isDetected())
    {
        guc_alarmFlags |= LOGGER_ALARM_FLAG_NO_USAGE;
    }

    // Check overtemperature alarm
    if(temp_isOutsideLimit())
    {
        guc_alarmFlags |= LOGGER_ALARM_FLAG_OVER_TEMP;
    }

}

/**
 *  @brief
 *      Create a log register.
 *
 *  @param[out] ps_reg
 *      Log register to be created.
 *
 *  @param[in]  uc_alarmFlags
 *      Alarm flags to be add to the log register.
 */
static inline void createLog(uint8_t *puc_buffer, uint8_t uc_alarmFlags)
{
    // Get actual timestamp
    gps_timestamp = systick_getActualDateTime();

    // Create a timestamp structure
    gs_logTimestamp.ui_hour =  gps_timestamp->tm_hour;
    gs_logTimestamp.ui_day = gps_timestamp->tm_mday;
    gs_logTimestamp.ui_month = gps_timestamp->tm_mon;
    gs_logTimestamp.ui_year = gps_timestamp->tm_year;
    gs_logTimestamp.ui_minutes = gps_timestamp->tm_min/15;

    // Pack the timestamp structure
    memcpy(puc_buffer, (uint8_t *) &gs_logTimestamp,
        LOGGER_REG_FIELD_TIMESTAMP_SIZE);

    puc_buffer += LOGGER_REG_FIELD_TIMESTAMP_SIZE;

    gl_volume = (uint32_t) (proc_getForwardVolume() * 1000);

    // Pack forward volumes in m�
    memcpy(puc_buffer, (uint8_t *) &gl_volume, LOGGER_REG_FIELD_VOL_SIZE);

    puc_buffer += LOGGER_REG_FIELD_VOL_SIZE;

    // Convert reverse volume from m� to liters (L)
    gl_volume = (uint32_t) (proc_getReverseVolume()* 1000); // get integer part (m^3)

    // Pack reverse volumes in m�
    memcpy(puc_buffer, (uint8_t *) &gl_volume, LOGGER_REG_FIELD_VOL_SIZE);

    puc_buffer += LOGGER_REG_FIELD_VOL_SIZE;

    // Convert minimum flow rate from m�/h to mL/h

    gl_flowRate = (uint32_t) (proc_getMinFlowRate() * 1000000);

    // Pack the minimum flow rate
    memcpy(puc_buffer, (uint8_t *) &gl_flowRate, LOGGER_REG_FLOWRATE_SIZE);

    puc_buffer += LOGGER_REG_FLOWRATE_SIZE;

    // Convert maximum flow rate from m^3/h to mL/h

    gl_flowRate = (uint32_t) (proc_getMaxFlowRate() * 1000000);

    // Pack the maximum flow rate
    memcpy(puc_buffer, (uint8_t *) &gl_flowRate, LOGGER_REG_FLOWRATE_SIZE);

    puc_buffer += LOGGER_REG_FLOWRATE_SIZE;

    // Pack alarms
    memcpy(puc_buffer, &uc_alarmFlags, LOGGER_REG_FIELD_ALARMS_SIZE);

    // Reset minimum and maximum flow rates for the next time slice
    proc_resetFlowRates();
}

/**
 *  @brief
 *      Save log in the FLASH memory.
 *
 *  @details
 *      Save the packed log in the FLASH memory in a specific memory index.
 *
 *  @param[in]   puc_buffer
 *      Buffer which stores the packed log.
 *
 *  @param[in]  ui_idx
 *      Memory index which log must be stored.
 *
 *  @retun
 *      Return 'true' if the logger has been save with success. Otherwise, it
 *      returns 'false'.
 */
static inline bool saveLog(uint8_t *puc_buffer, uint16_t ui_idx)
{
    bool b_status = true;

#if DEBUG_LOGGER
    printf("(LOGGER): Saving=%u\n", ui_idx/20);
//    printLogs();
#endif

    switch(ui_idx / (LOGGER_BUFFER_SIZE))
    {
        case 0:
            FRAMCtl_A_write32(  // Save 4 bytes in the FRAM
                (uint32_t *) puc_buffer,
                (uint32_t *) &gauc_logBuffer1[ui_idx],
                LOGGER_REG_SIZE/4
            );
            break;
        case 1:
            FRAMCtl_A_write32(  // Save 4 bytes in the FRAM
                (uint32_t *) puc_buffer,
                (uint32_t *) &gauc_logBuffer2[ui_idx - LOGGER_BUFFER_SIZE],
                LOGGER_REG_SIZE/4
            );
            break;
        case 2:
            FRAMCtl_A_write32(  // Save 4 bytes in the FRAM
                (uint32_t *) puc_buffer,
                (uint32_t *) &gauc_logBuffer3[ui_idx - 2*LOGGER_BUFFER_SIZE],
                LOGGER_REG_SIZE/4
            );
            break;
    }

    return b_status;
}

/**
 *  @brief
 *      Unpack log register.
 *
 *  @details
 *      Read the log memory in a specific log index and unpack its content
 *      in a log register.
 *
 *  @param[out] ps_reg
 *      Log register to be unpacked.
 *
 *  @param[in]  uc_idx
 *      Index of the log to be read. (index zero represents the most recent
 *      log)
 *
 *  @return
 *      Returns 'true' if it has been unpacked with success. Otherwise, it
 *      returns 'false'.
 */
static bool unpack(s_logger_reg_t * const ps_reg, uint16_t uc_idx)
{
    bool b_status = false;

    // Read the log memory in a specific index
    if(readLog(gauc_logReg, uc_idx))
    {
        uint16_t uc_logIdx = 0;
        s_logger_timestamp_t *ps_timestamp;

        // Clear log register
        memset((void *) ps_reg, 0, sizeof(s_logger_reg_t));

        ps_timestamp = (s_logger_timestamp_t *) gauc_logReg;

        // Unpack the log timestamp
        ps_reg->s_timestamp.tm_sec = 0;
        ps_reg->s_timestamp.tm_min = ps_timestamp->ui_minutes*15;
        ps_reg->s_timestamp.tm_hour = ps_timestamp->ui_hour;
        ps_reg->s_timestamp.tm_mday = ps_timestamp->ui_day;
        ps_reg->s_timestamp.tm_mon = ps_timestamp->ui_month;
        ps_reg->s_timestamp.tm_year = ps_timestamp->ui_year;


        uc_logIdx += LOGGER_REG_FIELD_TIMESTAMP_SIZE;

        // Unpack the forward volume
        memcpy((uint8_t *) &ps_reg->ll_forwardVol, &gauc_logReg[uc_logIdx],
            LOGGER_REG_FIELD_VOL_SIZE);

        uc_logIdx += LOGGER_REG_FIELD_VOL_SIZE;

        // Unpack the reverse volume
        memcpy((uint8_t *) &gl_volume, &gauc_logReg[uc_logIdx],
            LOGGER_REG_FIELD_VOL_SIZE);

        if (gl_volume < 0)
        {
            memset((uint8_t *) &ps_reg->ll_reverseVol, 0x00, //0x00 or 0xFF?
                sizeof(ps_reg->ll_reverseVol));
        }

        memcpy((uint8_t *) &ps_reg->ll_reverseVol, &gl_volume,
            LOGGER_REG_FIELD_VOL_SIZE);

        uc_logIdx += LOGGER_REG_FIELD_VOL_SIZE;

        // Unpack the minimum flow rate
        memcpy((uint8_t *) &ps_reg->l_minFlowRate, &gauc_logReg[uc_logIdx],
            LOGGER_REG_FLOWRATE_SIZE);

        uc_logIdx += LOGGER_REG_FLOWRATE_SIZE;

        // Unpack the maximum flow rate
        memcpy((uint8_t *) &ps_reg->l_maxFlowRate, &gauc_logReg[uc_logIdx],
            LOGGER_REG_FLOWRATE_SIZE);

        uc_logIdx += LOGGER_REG_FLOWRATE_SIZE;

        // Unpack the alarm flags
        memcpy((uint8_t *) &ps_reg->ui_alarms,
            &gauc_logReg[uc_logIdx], LOGGER_REG_FIELD_ALARMS_SIZE);

        b_status = true;
    }

    return b_status;
}

/**
 *  @brief
 *      Read the log memory.
 *
 *  @param[out] puc_buffer
 *      Buffer where the log must be stored after has been read from FLASH
 *      memory.
 *
 *  @param[in]  uc_idx
 *      Index of the log to be read. (index zero represents the most recent
 *      log)
 *
 *  @return
 *      Returns 'true' if it has been read with success. Otherwise, it
 *      returns 'false'.
 */
static inline bool readLog(uint8_t * const puc_buffer, uint16_t uc_idx)
{
    bool b_status = false;

    // Set default value of log memory in buffer
    memset((void *) puc_buffer, LOGGER_DEFAULT_VALUE, LOGGER_REG_SIZE);

    switch(uc_idx / (LOGGER_NUM_VALID_REGS))
    {
        case 0:
            // Check if the log memory has content
            if(memcmp((void *) puc_buffer, (void *) &gauc_logBuffer1[LOGGER_REG_SIZE*uc_idx],
                LOGGER_REG_SIZE))
            {
                // Copy the log content to log buffer
                memcpy((uint8_t *) puc_buffer, (void *) &gauc_logBuffer1[LOGGER_REG_SIZE*uc_idx],
                        LOGGER_REG_SIZE);

                b_status = true;
            }
            break;
        case 1:
            // Check if the log memory has content
            if(memcmp((void *) puc_buffer, (void *) &gauc_logBuffer2[LOGGER_REG_SIZE*(uc_idx - LOGGER_NUM_VALID_REGS)],
                LOGGER_REG_SIZE))
            {
                // Copy the log content to log buffer
                memcpy((uint8_t *) puc_buffer, (void *) &gauc_logBuffer2[LOGGER_REG_SIZE*(uc_idx - LOGGER_NUM_VALID_REGS)],
                        LOGGER_REG_SIZE);

                b_status = true;
            }
            break;
        case 2:
            // Check if the log memory has content
            if(memcmp((void *) puc_buffer, (void *) &gauc_logBuffer3[LOGGER_REG_SIZE*(uc_idx - 2*LOGGER_NUM_VALID_REGS)],
                LOGGER_REG_SIZE))
            {
                // Copy the log content to log buffer
                memcpy((uint8_t *) puc_buffer, (void *) &gauc_logBuffer3[LOGGER_REG_SIZE*(uc_idx - 2*LOGGER_NUM_VALID_REGS)],
                        LOGGER_REG_SIZE);

                b_status = true;
            }
            break;
    }
    return b_status;
}

#if DEBUG_LOGGER
/**
 *  @brief
 *      Print all logs in the memory.
 */
static inline void printLogs()
{
    uint8_t uc_idx;

    for(uc_idx = 0; uc_idx < guc_numLogs; uc_idx++)
    {
        // Unpack the log
        if(!unpack(&gs_reg, uc_idx))
        {
            break;
        }

        // Print each log
        printf("(LOGGER): %02d/%02d/%02d", gs_reg.s_timestamp.tm_mday,
            gs_reg.s_timestamp.tm_mon + 1, gs_reg.s_timestamp.tm_year - 100);

        printf(" - %02d:%02d:00", gs_reg.s_timestamp.tm_hour, gs_reg.s_timestamp.tm_min);

        printf(" - %u L - %u L",  gs_reg.ll_forwardVol,
            gs_reg.ll_reverseVol);

        printf(" - %.1f L/h - %.1f L/h - %d",
            ((float) gs_reg.l_minFlowRate)/1000000,
            ((float) gs_reg.l_maxFlowRate)/1000000,
            gs_reg.ui_alarms);

        printf("\n");
    }

    // Print the number of logs found
    printf("(LOGGER): Found %d logs.\n", uc_idx);
}
#endif /* DEBUG_LOGGER */

#endif /* CONFIG_LOGGER_ENABLED */
