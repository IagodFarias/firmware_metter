/*******************************************************************************
 * @brief
 * @file    cmdmng.c
 * @author  Eric Costa Hall
 * @date    04/08/2021
 * @details
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"

#include "ussSwLib.h"
#include "USS_userConfig.h"
#include "cmdmng.h"
#include "processing.h"
#include "probe.h"
#include "configmng.h"
#include "serial.h"
#include "tlv.h"
#include "opmodemng.h"
#include "calib.h"
#include "verif.h"
#include "normal.h"
#include "precalib.h"
#include "postcalib.h"
#include "standby.h"
#include "systick.h"

#if DEBUG
#include "userint.h"
#endif

#if DEBUG_PROC_FLOWRATE
#include "processing.h"
#endif

#define PROBE_CMD_START_OF_PKG                  0xFFFFFFFFFFFFFFFFULL

/*******************************************************************************
 * Structures
 ******************************************************************************/
/** Command packet structure. */
#pragma pack(1)
typedef struct __attribute__((__packed__)) S_PACKET
{
    /** ID field. */
    uint32_t ul_fcxId;
    /** Command field. For more information, refer to @ref E_CMDMNG_CMD_t. */
    uint8_t e_cmd;
    /** Response field. */
    uint8_t uc_resp;
} s_packet_t;

#pragma pack(1)
typedef struct S_CALENDAR
{
    uint8_t uc_sec;         // Seconds after the minutes (0-59)
    uint8_t uc_min;         // Minutes after the hour (0-59)
    uint8_t uc_hour;        // Hours since midnight (0-23)
    uint8_t uc_day;         // Day of month (1-31)
    uint8_t uc_month;       // Months since January (0-11)
    uint8_t uc_year;        // Years since 1900
} s_calendar_t;

#if CONFIG_FW_INTEGRITY
typedef struct S_FW_INTEGRITY_CMD
{
    uint32_t ul_offset;         // Firmware offset
    uint32_t ul_lenght;         // Firmware test length
} s_fw_integrity_cmd_t;
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static bool receiveCalibrationParameters();
#if CONFIG_FW_INTEGRITY
static bool isFwValidParam(s_fw_integrity_cmd_t *ps_params);
#endif

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Flags whether a new command is received. */
static bool gb_newCmdReceived;

/** Packet structure. */
static s_packet_t gs_packet;

/** System configuration */
static s_sys_config_t *gps_sysConfig;

/*******************************************************************************
 * Public Functions
 ******************************************************************************/
void cmdmng_init(void)
{
    // Get system configuration pointer
    gps_sysConfig = configmng_getSysConfig();

    // Initialize command flag
    gb_newCmdReceived = false;

    // Initialize query fields
    uint8_t length = 0;
    uint16_t *dev_id = NULL;
    TLV_getInfo(TLV_TAG_DIERECORD, 0, &length, &dev_id);

    // fcxId = waferid + die_x + die_y.
    gs_packet.ul_fcxId =  (((uint32_t) dev_id[1] << 16) | dev_id[0]) + ((uint32_t)(dev_id[2] << 8)) + ((uint32_t) dev_id[3] << 16);
}

/**
 * @brief
 *      Fetch the command received through serial interface.
 */
void cmdmng_fetch(void)
{
    if(serial_receiveByte((uint8_t*) &gs_packet.e_cmd))
    {
        gb_newCmdReceived = true;
    } 
}

/**
 * @brief
 *      Service any command previously received.
 */
void cmdmng_dispatch(void)
{
    if(gb_newCmdReceived)
    {
        // Clear new command flag
        gb_newCmdReceived = false;

        // Process command
        switch(gs_packet.e_cmd)
        {
            /** Read firmware version */
            case E_CMDMNG_CMD_READ_FIRMWARE_VERSION:

                // Send an ACK to indicate the meter is ready to send
                // its firmware version
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Send firmware structure
                serial_transmitBytes((uint8_t*) &gps_sysConfig->s_firmware,
                    sizeof(s_fw_config_t));

#if DEBUG
//                userint_setState(E_USERINT_CMD_CHANGE_SCREEN);
#endif
#if DEBUG_PROC_FLOWRATE
                proc_toogleFlowRate();
#endif
            break;

            /** Read serial number */
            case E_CMDMNG_CMD_READ_SERIAL_NUMBER:

                // Send an ACK to indicate the meter is ready to send
                // its serial number
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Send serial number
                serial_transmitBytes(
                    gps_sysConfig->s_production.auc_serialNumber,
                    CONFIG_SERIAL_NUMBER_LEN);
                break;

            /** Write serial number */
            case E_CMDMNG_CMD_WRITE_SERIAL_NUMBER:

                // Send an ACK for synchronization purpose
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                 // Receive new serial number
                serial_receiveBytes(
                    gps_sysConfig->s_production.auc_serialNumber,
                    CONFIG_SERIAL_NUMBER_LEN);

                // Store new system configuration in flash memory
                if(configmng_setSysConfig(gps_sysConfig))
                {
                    // Publish system configuration
                    configmng_updateSysConfig();
                    gs_packet.uc_resp = CMDMNG_CMD_ACK;
                }
                else
                {
                    // Respond with NACK in case of failure
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                }

                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Read replacement date */
            case E_CMDMNG_CMD_READ_REPLACEMENT_DATE:

                // Send an ACK to indicate the meter is ready to send
                // its replacement date
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Send replacement date
                serial_transmitBytes(
                    gps_sysConfig->s_production.auc_replacementDate,
                    CONFIG_REPLACEMENT_DATE_LEN);
            break;

            /** Write replacement date */
            case E_CMDMNG_CMD_WRITE_REPLACEMENT_DATE:

                // Send an ACK for synchronization purpose
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Receive new replacement date
                serial_receiveBytes(gps_sysConfig->s_production.auc_replacementDate,
                                    CONFIG_REPLACEMENT_DATE_LEN);

                // Store new system configuration in flash memory
                if(configmng_setSysConfig(gps_sysConfig))
                {
                    // Publish system configuration
                    configmng_updateSysConfig();
                    gs_packet.uc_resp = CMDMNG_CMD_ACK;
                }
                else
                {
                    // Respond with NACK in case of failure
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                }

                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
                break;

            /** Enable data transmission. */
            case E_CMDMNG_CMD_ENABLE_DATA_TRANSMIT:
                // Send an ACK to indicate the meter is ready to send
                // its firmware version
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
                probe_enableDataTransmission();
            break;

            /** Disable data transmission. */
            case E_CMDMNG_CMD_DISABLE_DATA_TRANSMIT:
                // Send an ACK to indicate the meter is ready to send
                // its firmware version
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
                probe_disableDataTransmission();
            break;

            /** Trim PGA. */
            case E_CMDMNG_CMD_TRIM_GAIN_PGA:

                if (us_trimGainPGA())
                {
                    // Save the updated configuration to flash memory
                    if (configmng_setSysConfig(gps_sysConfig))
                    {
                        // Publish system configuration
                        configmng_updateSysConfig();
                        gs_packet.uc_resp = CMDMNG_CMD_ACK;
                    }
                    else
                    {
                        gs_packet.uc_resp = CMDMNG_CMD_NACK;
                    }
                }
                else
                {
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                }

                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Trim Frequency. */
            case E_CMDMNG_CMD_TRIM_FREQUENCY:

                if (us_trimFrequencySwap())
                {
                    gs_packet.uc_resp = CMDMNG_CMD_ACK;
                }
                else
                {
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                }

                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Enable filters. */
            case E_CMDMNG_CMD_ENABLE_FILTERS:

                proc_enableFilters();
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Disable filters. */
            case E_CMDMNG_CMD_DISABLE_FILTERS:

                proc_disableFilters();
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Write pre-calibration operating mode. */
            case E_CMDMNG_CMD_WRITE_PRE_CALIB_OPMODE:

                if(opmodemng_getCurrentOpMode() != E_OPMODE_PRE_CALIBRATION)
                {
                    precalib_init();
                    opmodemng_setCurrentOpMode(E_OPMODE_PRE_CALIBRATION);
                }

                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            case E_CMDMNG_CMD_WRITE_CALIB_OPMODE:

                if(opmodemng_getCurrentOpMode() != E_OPMODE_CALIBRATION)
                {
                    calib_init();
                    opmodemng_setCurrentOpMode(E_OPMODE_CALIBRATION);
                }

                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Write post-calibration operating mode */
            case E_CMDMNG_CMD_WRITE_POST_CALIB_OPMODE:

                if(opmodemng_getCurrentOpMode() != E_OPMODE_POST_CALIBRATION)
                {
                    postcalib_init();
                    opmodemng_setCurrentOpMode(E_OPMODE_POST_CALIBRATION);
                }

                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Write stand-by operating mode. */
            case E_CMDMNG_CMD_WRITE_STANDBY_OPMODE:

                if(opmodemng_setCalibrationFlag(true))
                {
                    standby_init();
                    opmodemng_setCurrentOpMode(E_OPMODE_STANDBY);
                    gs_packet.uc_resp = CMDMNG_CMD_ACK;
                }
                else
                {
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                }

                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Write normal operating mode. */
            case E_CMDMNG_CMD_WRITE_NORMAL_OPMODE:

                if(opmodemng_getCurrentOpMode() != E_OPMODE_NORMAL)
                {
                    normal_init();

                    // Set active flag
                    ASSERT(opmodemng_setActivationFlag(true));
                    // Set calibration flag
                    ASSERT(opmodemng_setCalibrationFlag(true));

                    opmodemng_setCurrentOpMode(E_OPMODE_NORMAL);
                }

                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Write verification operating mode. */
            case E_CMDMNG_CMD_WRITE_VERIFICATION_OPMODE:

                if (opmodemng_getCurrentOpMode() != E_OPMODE_VERIFICATION)
                {
                    verif_init();

                    // Set active flag
                    ASSERT(opmodemng_setActivationFlag(true));
                    // Set calibration flag
                    ASSERT(opmodemng_setCalibrationFlag(true));

                    opmodemng_setCurrentOpMode (E_OPMODE_VERIFICATION);
                }

                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;
            /** Write calibration parameters. */
            case E_CMDMNG_CMD_WRITE_CALIB_PARAMETERS:

                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t *)&gs_packet, sizeof(gs_packet));

                // Receive calibration parameters
                if(receiveCalibrationParameters())
                {
                    gs_packet.uc_resp = CMDMNG_CMD_ACK;
                }
                else
                {
                    // Respond with NACK in case of failure
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                }

                serial_transmitBytes((uint8_t *)&gs_packet, sizeof(gs_packet));
            break;

            /** Read the whole system configuration. */
            case E_CMDMNG_CMD_READ_SYS_CONFIG:

                // Send an ACK to indicate the meter is ready to send the whole
                // system configuration structure
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Send system configuration structure
                serial_transmitBytes((uint8_t*) gps_sysConfig, 
                    sizeof(s_sys_config_t));
            break;

            /** Write the whole system configuration */
            case E_CMDMNG_CMD_WRITE_SYS_CONFIG:

                // Send an ACK for synchronization purpose
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                 // Receive new system configuration structure
                serial_receiveBytes((uint8_t *) gps_sysConfig, 
                    sizeof(s_sys_config_t));

                // Publish system configuration
                configmng_setSysConfig(gps_sysConfig);
                gs_packet.uc_resp = CMDMNG_CMD_ACK;

                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Write actual date and time */
            case E_CMDMNG_CMD_WRITE_ACTUAL_DATE_TIME:
            {
                s_calendar_t s_calendar;
                struct tm s_actualDateTime;

                // Send an ACK for synchronization purpose
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Receive the calendar
                serial_receiveBytes((uint8_t *) &s_calendar,
                    sizeof(s_calendar));

                s_actualDateTime.tm_sec = s_calendar.uc_sec;
                s_actualDateTime.tm_min = s_calendar.uc_min;
                s_actualDateTime.tm_hour = s_calendar.uc_hour;
                s_actualDateTime.tm_mday = s_calendar.uc_day;
                s_actualDateTime.tm_mon = s_calendar.uc_month;
                s_actualDateTime.tm_year = s_calendar.uc_year;
                s_actualDateTime.tm_isdst = 0;

                // Encode Year (4 digits)
                gps_sysConfig->s_production.auc_calibrationDate[7] = 48 + (int)((1900 + s_calendar.uc_year) / 1000);
                gps_sysConfig->s_production.auc_calibrationDate[6] = 48 + (int)(((1900 + s_calendar.uc_year) / 100) % 10);
                gps_sysConfig->s_production.auc_calibrationDate[5] = 48 + (int)(((1900 + s_calendar.uc_year) / 10) % 10);
                gps_sysConfig->s_production.auc_calibrationDate[4] = 48 + (int)((1900 + s_calendar.uc_year) % 10);

                // Encode Month (2 digits)
                gps_sysConfig->s_production.auc_calibrationDate[3] = 48 + (int)((s_calendar.uc_month + 1) / 10);
                gps_sysConfig->s_production.auc_calibrationDate[2] = 48 + (int)((s_calendar.uc_month + 1) % 10);

                // Encode Day (2 digits)
                gps_sysConfig->s_production.auc_calibrationDate[1] = 48 + (int)(s_calendar.uc_day / 10);
                gps_sysConfig->s_production.auc_calibrationDate[0] = 48 + (int)(s_calendar.uc_day % 10);

                // Encode Hour (2 digits)
                gps_sysConfig->s_production.auc_calibrationDate[11] = 48 + (int)(s_calendar.uc_hour / 10);
                gps_sysConfig->s_production.auc_calibrationDate[10] = 48 + (int)(s_calendar.uc_hour % 10);

                // Encode Minute (2 digits)
                gps_sysConfig->s_production.auc_calibrationDate[9] = 48 + (int)(s_calendar.uc_min / 10);
                gps_sysConfig->s_production.auc_calibrationDate[8] = 48 + (int)(s_calendar.uc_min % 10);

                // Encode Second (2 digits)
                gps_sysConfig->s_production.auc_calibrationDate[13] = 48 + (int)(s_calendar.uc_sec / 10);
                gps_sysConfig->s_production.auc_calibrationDate[12] = 48 + (int)(s_calendar.uc_sec % 10);


                // Set new date and time
                if(systick_setActualDateTime(&s_actualDateTime) && configmng_setSysConfig(gps_sysConfig))
                {
                    // Respond with ACK
                    gs_packet.uc_resp = CMDMNG_CMD_ACK;
                }
                else
                {
                    // Respond with NACK in case of failure
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                }

                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            }
            break;

            /** Read actual date and time */
            case E_CMDMNG_CMD_READ_ACTUAL_DATE_TIME:
            {
                s_calendar_t s_calendar;
                struct tm *ps_actualDateTime;

                // Read actual date and time
                ps_actualDateTime = systick_getActualDateTime();

                // Send an ACK for synchronization purpose
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                s_calendar.uc_sec = ps_actualDateTime->tm_sec;
                s_calendar.uc_min = ps_actualDateTime->tm_min;
                s_calendar.uc_hour = ps_actualDateTime->tm_hour;
                s_calendar.uc_day = ps_actualDateTime->tm_mday;
                s_calendar.uc_month = ps_actualDateTime->tm_mon;
                s_calendar.uc_year = ps_actualDateTime->tm_year;

                // Send tm structure
                serial_transmitBytes((uint8_t*) &s_calendar,
                    sizeof(s_calendar));
            }
            break;

#if CONFIG_FW_INTEGRITY
            /** Read Firmware integrity */
            case E_CMDMNG_CMD_READ_FW_INTEGRITY:
            {
                s_fw_integrity_cmd_t s_cmdParams;

                // Send an ACK for synchronization purpose
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Receive the calendar
                serial_receiveBytes((uint8_t *) &s_cmdParams,
                    sizeof(s_cmdParams));

                if(isFwValidParam(&s_cmdParams))
                {
                    uint16_t ui_crc =
                            configmng_calcCrcFromMem(s_cmdParams.ul_offset,
                                    s_cmdParams.ul_lenght);

                    // Send an ACK for synchronization purpose
                    gs_packet.uc_resp = CMDMNG_CMD_ACK;
                    serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                    // Send tm structure
                    serial_transmitBytes((uint8_t*) &ui_crc, sizeof(uint16_t));
                }
                else
                {
                    // Send an NACK for invalid parameters
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                    serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
                }
            }
            break;
#endif

#if CONFIG_WMBUS_ENABLED
            /** Write wM-Bus configuration */
            case E_CMDMNG_CMD_WRITE_WMBUS_CONFIG:

                // Send an ACK for synchronization purpose
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Receive the wM-Bus configuration
                serial_receiveBytes((uint8_t *) &gps_sysConfig->s_wmbus,
                    sizeof(s_wmbus_config_t));

                // Save sytem configuration
                configmng_saveSysConfig(gps_sysConfig);

                // Update system configuration
                configmng_updateSysConfig();

                // Respond with ACK
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;

            /** Read wM-Bus configuration */
            case E_CMDMNG_CMD_READ_WMBUS_CONFIG:

                // Send an ACK for synchronization purpose
                gs_packet.uc_resp = CMDMNG_CMD_ACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));

                // Send wM-Bus configuration structure
                serial_transmitBytes((uint8_t*) &gps_sysConfig->s_wmbus,
                    sizeof(s_wmbus_config_t));
            break;

            /** Test wM-Bus transmission */
            case E_CMDMNG_CMD_TEST_WMBUS_TX:
            {
                bool b_txDone = false;
                bool b_isEnabled = wmbus_stack_isEnabled();
                uint16_t ui_oldInterval = gps_sysConfig->s_wmbus.ui_interval;

                // Init stack before send package
                wmbus_stack_init();

                // Enable transmission for the single transmission
                wmbus_stack_enableTransmission();

                // Set next interval to zero to imediatly send a new package
                gps_sysConfig->s_wmbus.ui_interval = 0;

                // Send a package
                wmbus_stack_update();

                // Check if a successful package was sent
                b_txDone = wmbus_stack_isTransmitting();

                // Restore wmbus configuration
                if(b_isEnabled)
                {
                    wmbus_stack_enableTransmission();
                }
                else
                {
                    wmbus_stack_disableTransmission();
                }

                // Reset default interval
                gps_sysConfig->s_wmbus.ui_interval = ui_oldInterval;

                if(b_txDone)
                {
                    // Respond with ACK
                    gs_packet.uc_resp = CMDMNG_CMD_ACK;
                }
                else
                {
                    // Send an NACK
                    gs_packet.uc_resp = CMDMNG_CMD_NACK;
                }

                // Send response
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            }
            break;
#endif /* CONFIG_WMBUS_ENABLED */

            default:
                gs_packet.e_cmd = E_CMDMNG_CMD_INVALID;
                gs_packet.uc_resp = CMDMNG_CMD_NACK;
                serial_transmitBytes((uint8_t*) &gs_packet, sizeof(gs_packet));
            break;
        }

        // Make sure there is no pending data to be sent through serial
        // interface
        while(serial_stillTransmitting());
    }
}

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/**
 * @brief
 *      Receive calibration parameters through serial interface.
 *
 * @return 
 *      If the calibration parameters have been received with success, it 
 *      returns 'true'. Otherwise, it returns 'false'.
 */
static bool receiveCalibrationParameters()
{
    uint8_t uc_idx;

    // Receive configuration parameters
    if (!serial_receiveBytes((uint8_t *)&gps_sysConfig->s_mechanical.d_flowPathLength,
                             sizeof(gps_sysConfig->s_mechanical.d_flowPathLength)))
    {
        return false;
    }

    if (!serial_receiveBytes((uint8_t *)&gps_sysConfig->s_mechanical.d_flowPathDiameter,
                     sizeof(gps_sysConfig->s_mechanical.d_flowPathDiameter)))
    {
        return false;
    }

    if (!serial_receiveBytes(
            (uint8_t*) &gps_sysConfig->s_mechanical.d_flowPathArea,
            sizeof(gps_sysConfig->s_mechanical.d_flowPathArea)))
    {
        return false;
    }

    if (!serial_receiveBytes(
            (uint8_t *)&gps_sysConfig->s_calibration.d_delaySpeedOfSound,
                             sizeof(gps_sysConfig->s_calibration.d_delaySpeedOfSound)))
    {
        return false;
    }

    if (!serial_receiveBytes((uint8_t*) &gps_sysConfig->s_calibration.d_delayZeroCalibration,
                             sizeof(gps_sysConfig->s_calibration.d_delayZeroCalibration)))
    {
        return false;
    }

    if (!serial_receiveBytes((uint8_t *)&gps_sysConfig->s_calibration.ui_calibLutSize,
                             sizeof(gps_sysConfig->s_calibration.ui_calibLutSize)))
    {
        return false;
    }

#ifdef DEBUG_ASSERT
    // Sanity check
    ASSERT(gps_sysConfig->s_calibration.ui_calibLutSize <= PROC_CALIBRATION_LUT_SIZE);
#endif
    // Check if the calibration table fits inside the available space reserved
    // in flash
    if (gps_sysConfig->s_calibration.ui_calibLutSize > PROC_CALIBRATION_LUT_SIZE)
    {
        gps_sysConfig->s_calibration.ui_calibLutSize = PROC_CALIBRATION_LUT_SIZE;
    }

    // Receive calibration table
    for (uc_idx = 0; uc_idx < gps_sysConfig->s_calibration.ui_calibLutSize; uc_idx++)
    {
        if (!serial_receiveBytes(
                (uint8_t *)&gps_sysConfig->s_calibration.as_calibLut[uc_idx].d_abscissa,
                sizeof(double_t)))
        {
            return false;
        }
    }

    for (uc_idx = 0; uc_idx < gps_sysConfig->s_calibration.ui_calibLutSize; uc_idx++)
    {
        if (!serial_receiveBytes(
                (uint8_t *)&gps_sysConfig->s_calibration.as_calibLut[uc_idx].d_ordinate,
                sizeof(double_t)))
        {
            return false;
        }
    }

    // Save configuration to fram memory
    if(configmng_setSysConfig(gps_sysConfig))
    {
        // Publish system configuration
        configmng_updateSysConfig();

        return true;
    }

    return false;
}

#if CONFIG_FW_INTEGRITY
static bool isFwValidParam(s_fw_integrity_cmd_t *ps_params)
{
    if(ps_params->ul_offset < FRAM_START)
    {
        return false;
    }
    else if((ps_params->ul_offset + ps_params->ul_lenght) > FRAM_LENGTH)
    {
        return false;
    }

    return true;
}
#endif
