/***************************************************************************//**
 * @brief
 * @file    optcomm.c
 * @author  Gustavo Salomão
 * @date    01/10/2017
 * @details Module which implements the communication by optical interface.
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "optcomm.h"
#include "types.h"
#include "optint.h"
#include "logger.h"
#include "hal.h"
#include "configmng.h"
#include "opmodemng.h"
#include "screenmng.h"
#include "crc.h"
#include "systick.h"

#if DEBUG_OPTCOMM
#include <gpio.h>
#include <stdio.h>
#endif /* DEBUG_OPTCOMM */

#if CONFIG_OPTCOMM_ENABLED
/** Number of tries to receive the first ACK before it's connected. */
#define OPTCOMM_CONN_TRIES                              6

/** SYNC_REQ skip count. */
#define OPTCOMM_SYNC_REQ_SKIP_CNT                       2

/** Max weekday valid. */
#define OPTCOMM_MAX_VALID_WEEKDAY                       7
/** Min valid day of month. */
#define OPTCOMM_MIN_VALID_DAY_MONTH                     1
/** Max valid day of month. */
#define OPTCOMM_MAX_VALID_DAY_MONTH                     28

/** Max bytes crc16 verification per measure cycle.
 *  This value is estimated and spend +- 800ms time processing.
 *  This estimate used an oscilloscope and a GPIO to get the correct time
 *  from process.*/
#define OPTCOMM_CRC_CHECK_MAX_BYTES_PER_CYCLE           0xFFFF

/*******************************************************************************
 * Enumerations
 ******************************************************************************/

/** OPTCOMM state machine. */
typedef enum E_OPTCOMM_STATE
{
    /** Waiting synchronization request state */
    E_OPTCOMM_STATE_WAITING_SYNC_REQ = 0,
    /** Waiting command state */
    E_OPTCOMM_STATE_WAITING_CMD,
    /** Responding NACK */
    E_OPTCOMM_STATE_RESPONDING_NACK,
    /** Responding invalid command state */
    E_OPTCOMM_STATE_RESPONDING_INVALID_CMD,
    /** Reading logger configuration state */
    E_OPTCOMM_STATE_READING_LOG_CONFIG,
    /** Updating logger configuration state */
    E_OPTCOMM_STATE_UPDATING_LOG_CONFIG,
    /** Reading logs state */
    E_OPTCOMM_STATE_READING_LOGS,

#if CONFIG_OPTCOMM_ENABLED
    /** Reading sys config page */
    E_OPTCOMM_STATE_READING_CONFIG_PAGE,
#endif

#if OPTCOMM_CHANGE_CONFIG
    /** Updateing sys config page */
    E_OPTCOMM_STATE_UPDATING_CONFIG_PAGE,
#endif

#if OPTCOMM_CHANGE_MODE
    /** Changing operating mode state */
    E_OPTCOMM_STATE_CHANGING_OPMODE,
#endif /* OPTCOMM_CHANGE_MODE */

    /** Reading serial number */
    E_OPTCOMM_STATE_READING_SERIAL_NUMBER,
    /** Reading firmware version */
    E_OPTCOMM_STATE_READING_FIRMWARE_VERSION,

#if OPTCOMM_ERASE_LOGS
    E_OPTCOMM_STATE_ERASE_LOGS,
#endif /* OPTCOMM_ERASE_LOGS */

#if DEBUG_FLOWRATE
    E_OPTCOMM_STATE_SET_FLOWRATE,
#endif /* DEBUG_FLOWRATE */

#if CONFIG_WMBUS_ENABLED
    /** Enable wM-Bus transmission */
    E_OPTCOMM_STATE_ENABLING_WMBUS_TX,
    /** Disable wM-Bus transmission */
    E_OPTCOMM_STATE_DISABLING_WMBUS_TX,
#endif
    /** Read checksum code memory */
    E_OPTCOMM_STATE_CHECK_CHECKSUM_CODE,

} E_OPTCOMM_STATE_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void processRxMessage(s_optcomm_msg_t *ps_msg);
//static uint16_t startMessage(E_OPTCOMM_CMD_t e_cmd, uint16_t ui_payloadLen);
//static uint16_t finishMessage(void);
static inline bool receiveSyncRequest(void);
static bool receiveMessage(s_optcomm_msg_t *ps_msg);
static inline bool readConfigPage(void);
#if OPTCOMM_CHANGE_CONFIG
static bool receivedSysConfig(s_sys_config_t *ps_sysConfig, s_optcomm_msg_t *gs_msg);
#endif

static inline bool calcCrcFromMemory(void);

#if CONFIG_LOGGER_ENABLED
static inline bool readLogs();
static inline void sendLog(s_logger_reg_t *ps_log);
static inline uint32_t parseAlarmFlags(uint16_t ui_alarms);
static inline void readLoggerConfig(s_logger_config_t *ps_config);
static inline void updateLoggerConfig(s_logger_config_t *ps_config);
#endif /* CONFIG_LOGGER_ENABLED */

#if OPTCOMM_CHANGE_MODE
static inline void changeOpmode(E_OPMODE_t e_mode);
#endif /* OPTCOMM_CHANGE_MODE */

static inline bool isValidParamReadMem(s_code_verif_params_t *ps_param);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** State machine of the OPTCOMM module. */
static E_OPTCOMM_STATE_t ge_state = E_OPTCOMM_STATE_WAITING_SYNC_REQ;

/** Buffer used to as stream buffer for transmission and reception. */
static uint8_t gauc_buffer[OPTCOMM_MAX_FRAME_LEN];

/** Structure of OPTCOMM message. */
static s_optcomm_msg_t gs_msg;

/** Flag to indicate if the OPTCOMM is connected with a Data Reader. */
static bool gb_connected = false;

static uint8_t guc_connTries;

/** SYNC_REQ skip count. */
static uint8_t guc_syncReqSkipCnt;

#if CONFIG_LOGGER_ENABLED
/** Structure of log register. */
static s_logger_reg_t gs_logger;

/** Index of log to read. */
static uint8_t guc_logIdx;
/** Structure of logger configuration. */
static s_logger_config_t gs_logerConfig;
#endif /* CONFIG_LOGGER_ENABLED */

/** Memory checksum parameters. */
static s_code_verif_params_t gs_readMemParam;

/** Pointer to system configuration. */
static s_sys_config_t *gps_sysConfig;

#if OPTCOMM_CHANGE_MODE
/** Operation mode. */
static E_OPMODE_t ge_opmode;

/** Device key. */
static uint32_t gul_key;
#endif /* OPTCOMM_CHANGE_MODE */

/** Current config page being sent. */
static uint8_t guc_configPage;



/** Callback to be called whenever an activity occurs in the OPTCOMM. */
static void (*gfp_activityCallback)(void) = NULL;

#if DEBUG_FLOWRATE
static int32_t gl_flowrate;
#endif /* DEBUG_FLOWRATE */

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize OPTCOMM module.
 */
void optcomm_init()
{
    // Get a pointer to system configuration
    gps_sysConfig = configmng_getSysConfig();

    // Calculate the initial skip count
    guc_syncReqSkipCnt = OPTCOMM_SYNC_REQ_SKIP_CNT;

#if DEBUG_OPTCOMM
    printf("(OPTCOMM): Module init.\n");
#endif /* DEBUG_OPTCOMM */
}

/**
 * @brief
 *      Update OPTCOMM's state.
 */
void optcomm_update()
{
    // Check the OPTCOMM's state
    switch(ge_state)
    {
        default:
        case E_OPTCOMM_STATE_WAITING_SYNC_REQ:
            if(!guc_syncReqSkipCnt)
            {
#if 0 //DEBUG_OPTCOMM
                printf("(OPTCOMM): Waiting sync request.\n");
#endif /* DEBUG_OPTCOMM */

                if(receiveSyncRequest())
                {
                    /* Sync request was received, so send a SYNC */
                    startMessage(E_OPTCOMM_CMD_SYNC, 0);
                    finishMessage();

#if DEBUG_OPTCOMM
                    printf("(OPTCOMM): Received sync request.\n");
#endif /* DEBUG_OPTCOMM */

                    // Reset connection tries
                    guc_connTries = OPTCOMM_CONN_TRIES;

                    // Set OPTCOMM state
                    ge_state = E_OPTCOMM_STATE_WAITING_CMD;
                }

                guc_syncReqSkipCnt = OPTCOMM_SYNC_REQ_SKIP_CNT;
            }
            else
            {
                guc_syncReqSkipCnt--;
            }
            break;

        case E_OPTCOMM_STATE_WAITING_CMD:

            // Send a SYNC_CONN command, if it's connected or send SYNC command
            // if it's not connected
            if (!gb_connected)
            {
                startMessage(E_OPTCOMM_CMD_SYNC, 0);
            }
            else
            {
                startMessage(E_OPTCOMM_CMD_SYNC_CONN, 0);
            }

            finishMessage();
            // Wait for a message from the Data Reader
            if(receiveMessage(&gs_msg))
            {
                // Process received message
                processRxMessage(&gs_msg);
            }
            else // if(!gb_connected)
            {
                if (guc_connTries)
                {
                    guc_connTries--;
                }
                else
                {
                    // Reset OPTCOMM state
                    optcomm_disconnect();
                }
            }

            break;

        case E_OPTCOMM_STATE_RESPONDING_NACK:

            // Start a message to send a NACK
            startMessage(E_OPTCOMM_CMD_NACK, 0);

            // Finish the message
            finishMessage();

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;

        case E_OPTCOMM_STATE_RESPONDING_INVALID_CMD:

            // Start a message to respond invalid command
            startMessage(E_OPTCOMM_CMD_INVALID, 0);

            // Finish the message
            finishMessage();

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;

#if CONFIG_LOGGER_ENABLED
        case E_OPTCOMM_STATE_READING_LOG_CONFIG:

            // Read logger configuration
            readLoggerConfig(&gps_sysConfig->s_logger);

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;

        case E_OPTCOMM_STATE_UPDATING_LOG_CONFIG:

            // If the state is update logger configuration, so update it
            updateLoggerConfig(&gs_logerConfig);

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;

        case E_OPTCOMM_STATE_READING_LOGS:

            // If the state is read logs, so read them
            if(!readLogs())
            {
                // Finished to read logs, so change state to wait command
                ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            }

            if(gfp_activityCallback)
            {
                // If there is an activity callback, so call it
                gfp_activityCallback();
            }
            break;
#endif /* CONFIG_LOGGER_ENABLED */
#if CONFIG_OPTCOMM_ENABLED
        case E_OPTCOMM_STATE_READING_CONFIG_PAGE:
            if(!readConfigPage())
            {
                // Finished sending the config page, so change state to wait command
                ge_state = E_OPTCOMM_STATE_WAITING_CMD;

            }
            if(gfp_activityCallback)
            {
                // If there is an activity callback, so call it
                gfp_activityCallback();
            }
            break;
#endif

#if OPTCOMM_CHANGE_CONFIG
        case E_OPTCOMM_STATE_UPDATING_CONFIG_PAGE:

            if (!gb_connected)
            {
                // If not connected go to wait cmd
                ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            }
            else
            {
                // Must cycle to send SYN_CONN and return to receive other
                // rest of package data
                if(receivedSysConfig(gps_sysConfig, &gs_msg))
                {
#if DEBUG_OPTCOMM
                    printf("(OPTCOMM): Receiving config page \n");
#endif /* DEBUG_OPTCOMM */
                    ge_state = E_OPTCOMM_STATE_UPDATING_CONFIG_PAGE;

                }
                else
                {
                    ge_state = E_OPTCOMM_STATE_WAITING_CMD;

                }

                if(gfp_activityCallback)
                {
                    // If there is an activity callback, so call it
                    gfp_activityCallback();
                }
            }

        break;
#endif

#if OPTCOMM_CHANGE_MODE
        case E_OPTCOMM_STATE_CHANGING_OPMODE:

            // Change operating mode
            changeOpmode(ge_opmode);

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;
#endif /* OPTCOMM_CHANGE_MODE */

        case E_OPTCOMM_STATE_READING_SERIAL_NUMBER:


            // Start a message to send the serial number
            startMessage(E_OPTCOMM_CMD_READ_SERIAL_NUMBER,
                CONFIG_SERIAL_NUMBER_LEN);

            // Add serial number to CRC calculation
            crc_calc(gps_sysConfig->s_production.auc_serialNumber,
                CONFIG_SERIAL_NUMBER_LEN);

            // Send serial number
            optint_send(gps_sysConfig->s_production.auc_serialNumber,
                CONFIG_SERIAL_NUMBER_LEN);

            // Finish the message
            finishMessage();

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;

        case E_OPTCOMM_STATE_READING_FIRMWARE_VERSION:

            // Start a message to send the firmware version
            startMessage(E_OPTCOMM_CMD_READ_FIRMWARE_VERSION,
                CONFIG_FIRMWARE_VERSION_LEN);

            // Add major number to CRC calculation
            crc_calc(
                (uint8_t *) &gps_sysConfig->s_firmware.ui_fwMajorVersion,
                sizeof(gps_sysConfig->s_firmware.ui_fwMajorVersion));

            // Add minor number to CRC calculation
            crc_calc(
                (uint8_t *) &gps_sysConfig->s_firmware.ui_fwMinorVersion,
                sizeof(gps_sysConfig->s_firmware.ui_fwMinorVersion));

            // Add patch number to CRC calculation
            crc_calc(
                (uint8_t *) &gps_sysConfig->s_firmware.ui_fwPatchVersion,
                sizeof(gps_sysConfig->s_firmware.ui_fwPatchVersion));

            // Add build number to CRC calculation
            crc_calc(
                (uint8_t *) &gps_sysConfig->s_firmware.ui_fwBuildVersion,
                sizeof(gps_sysConfig->s_firmware.ui_fwBuildVersion));

            // Send major number
            optint_send(
                (uint8_t *) &gps_sysConfig->s_firmware.ui_fwMajorVersion,
                sizeof(gps_sysConfig->s_firmware.ui_fwMajorVersion));

            // Send minor number
            optint_send(
                (uint8_t *) &gps_sysConfig->s_firmware.ui_fwMinorVersion,
                sizeof(gps_sysConfig->s_firmware.ui_fwMinorVersion));

            // Send patch number
            optint_send(
                (uint8_t *) &gps_sysConfig->s_firmware.ui_fwPatchVersion,
                sizeof(gps_sysConfig->s_firmware.ui_fwPatchVersion));

            // Send build number
            optint_send(
                (uint8_t *) &gps_sysConfig->s_firmware.ui_fwBuildVersion,
                sizeof(gps_sysConfig->s_firmware.ui_fwBuildVersion));

            // Finish the message
            finishMessage();

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;

#if OPTCOMM_ERASE_LOGS
        case E_OPTCOMM_STATE_ERASE_LOGS:

            // Erase logs memory
            logger_erase();

            // Start a message to send a ACK
            startMessage(E_OPTCOMM_CMD_ACK, 0);

            // Finish the message
            finishMessage();

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;
#endif /* OPTCOMM_ERASE_LOGS */

#if DEBUG_FLOWRATE
        case E_OPTCOMM_STATE_SET_FLOWRATE:

            // Convert flow rate from mL/h to m³/h
            fpu_set_accu_dbl(fpu_integer2double(gl_flowrate));
            fpu_calculate_div(10000);

            // Set flow rate
            proc_setFlowRate(fpu_get_result_dbl());

            // Start a message to send a ACK
            startMessage(E_OPTCOMM_CMD_ACK, 0);

            // Finish the message
            finishMessage();

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            break;
#endif /* DEBUG_FLOWRATE */

#if CONFIG_WMBUS_ENABLED
        case E_OPTCOMM_STATE_ENABLING_WMBUS_TX:

            // Enable transmission
            configmng_enableTx(true);

            // Start a message to send a ACK
            startMessage(E_OPTCOMM_CMD_ACK, 0);

            // Finish the message
            finishMessage();

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
        break;

        case E_OPTCOMM_STATE_DISABLING_WMBUS_TX:
            // Enable transmission
            configmng_enableTx(false);

            // Start a message to send a ACK
            startMessage(E_OPTCOMM_CMD_ACK, 0);

            // Finish the message
            finishMessage();

            // Change state to wait command
            ge_state = E_OPTCOMM_STATE_WAITING_CMD;
        break;
#endif /* CONFIG_WMBUS_ENABLE*/

        case E_OPTCOMM_STATE_CHECK_CHECKSUM_CODE:
            if(!calcCrcFromMemory())
            {
                // Finished sending the checksum, so change state to wait command
                ge_state = E_OPTCOMM_STATE_WAITING_CMD;
            }
            if(gfp_activityCallback)
            {
                // If there is an activity callback, so call it
                gfp_activityCallback();
            }
        break;
    }
}

/**
 * @brief
 *      Check if the OPTCOMM is connected with a Data Reader.
 *
 * @return
 *      Returns 'true' in case of it's connected or 'false' otherwise.
 */
bool optcomm_isConnected()
{
    return gb_connected;
}

/**
 * @brief
 *      Disconnect the OPTCOMM with the Data Reader.
 */
void optcomm_disconnect()
{
    gb_connected = false;
    ge_state = E_OPTCOMM_STATE_WAITING_SYNC_REQ;
    guc_syncReqSkipCnt = OPTCOMM_SYNC_REQ_SKIP_CNT;
    guc_connTries = OPTCOMM_CONN_TRIES;
}

/**
 * @brief
 *      Set an activity callback.
 *
 * @detains
 *      When an activity callback is set, any activity (command reception,
 *      data or ACK sent), this callback will be called.
 *
 * @param[in]   fp_callback
 *      Callback to be called whenever an activity occur in the OPTCOMM
 *      interface.
 */
void optcomm_setActivityCallback(void (*fp_callback)(void))
{
    gfp_activityCallback = fp_callback;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
/**
 * @brief
 *      Process received message.
 *
 * @param[in]   ps_msg
 *      Pointer to received message to be processed.
 */
static void processRxMessage(s_optcomm_msg_t *ps_msg)
{
    bool b_isValidCmd = true;

    // Check command
    switch(ps_msg->e_cmd)
    {
        case E_OPTCOMM_CMD_SYNC_ACK:

            if(OPTCOMM_MSG_LEN_SYNC_ACK == ps_msg->ui_len)
            {
                // Set OPTCOMM to connected
                gb_connected = true;
            }
            break;

#if CONFIG_LOGGER_ENABLED
        case E_OPTCOMM_CMD_READ_LOGGER_CONFIG:

            if(OPTCOMM_MSG_LEN_READ_LOGGER_CONFIG == ps_msg->ui_len)
            {
                // Change the OPTCOMM's state to read logger
                // configuration in the next meter's cycle
                ge_state = E_OPTCOMM_STATE_READING_LOG_CONFIG;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;

        case E_OPTCOMM_CMD_UPDATE_LOGGER_CONFIG:

            if(OPTCOMM_MSG_LEN_UPDATE_LOGGER_CONFIG == ps_msg->ui_len)
            {
                // Parse the logger configuration
                gs_logerConfig.uc_interval = ps_msg->auc_payload[0];
                gs_logerConfig.uc_param = ps_msg->auc_payload[1];

                // Change the OPTCOMM's state to update logger
                // configuration in the next meter's cycle
                ge_state = E_OPTCOMM_STATE_UPDATING_LOG_CONFIG;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;

        case E_OPTCOMM_CMD_READ_LOGS:

            if(OPTCOMM_MSG_LEN_READ_LOGS == ps_msg->ui_len)
            {
                // Set the log index to first log
                guc_logIdx = 0;

                // Change the OPTCOMM's state to read logs in next
                // meter's cycle
                ge_state = E_OPTCOMM_STATE_READING_LOGS;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;
#endif /* CONFIG_LOGGER_ENABLED */

#if OPTCOMM_CHANGE_MODE
        case E_OPTCOMM_CMD_CHANGE_OPMODE:

            if(OPTCOMM_MSG_LEN_CHANGE_OPMODE == ps_msg->ui_len)
            {
                uint32_t ul_temp;

                // Parse new mode
                ge_opmode = (E_OPMODE_t) ps_msg->auc_payload[0];

#if DEBUG_OPTCOMM
                 printf("(OPTCOMM): Received mode %d\n", ge_opmode);
#endif /* DEBUG_OPTCOMM */

                // Parse key
                gul_key = ps_msg->auc_payload[1];

                ul_temp = ps_msg->auc_payload[2];
                gul_key |= ul_temp << 8;

                ul_temp = ps_msg->auc_payload[3];
                gul_key |= ul_temp << 16;

                ul_temp = ps_msg->auc_payload[4];
                gul_key |= ul_temp << 24;

                // Change the OPTCOMM's state to change operating
                // mode in next meter's cycle
                ge_state = E_OPTCOMM_STATE_CHANGING_OPMODE;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;
#endif /* OPTCOMM_CHANGE_MODE */

        case E_OPTCOMM_CMD_READ_SERIAL_NUMBER:

            if(OPTCOMM_MSG_LEN_READ_SERIAL_NUMBER == ps_msg->ui_len)
            {
                // Change the OPTCOMM's state to read serial number
                // in the next meter's cycle
                ge_state = E_OPTCOMM_STATE_READING_SERIAL_NUMBER;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;

        case E_OPTCOMM_CMD_READ_FIRMWARE_VERSION:

            if(OPTCOMM_MSG_LEN_READ_FIRMWARE_VERSION == ps_msg->ui_len)
            {
                // Change the OPTCOMM's state to read firmware
                // version in the next meter's cycle
                ge_state = E_OPTCOMM_STATE_READING_FIRMWARE_VERSION;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;

#if OPTCOMM_ERASE_LOGS
        case E_OPTCOMM_CMD_ERASE_LOGS:

            if(OPTCOMM_MSG_LEN_READ_ERASE_LOGS == ps_msg->ui_len)
            {
                // Change the OPTCOMM's state to erase logs memory
                // in next meter's cycle
                ge_state = E_OPTCOMM_STATE_ERASE_LOGS;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;
#endif /* OPTCOMM_ERASE_LOGS */

#if CONFIG_OPTCOMM_ENABLED
        case E_OPTCOMM_CMD_READ_CONFIG_PAGE:

            if(OPTCOMM_MSG_LEN_READ_CONFIG_PAGE == ps_msg->ui_len)
            {
                ge_state = E_OPTCOMM_STATE_READING_CONFIG_PAGE;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;
#endif


#if OPTCOMM_CHANGE_CONFIG
        case E_OPTCOMM_CMD_WRITE_CONFIG_PAGE:

            if(OPTCOMM_MSG_LEN_UPDATE_METER_CONFIG == ps_msg->ui_len)
            {
                ge_state = E_OPTCOMM_STATE_UPDATING_CONFIG_PAGE;

            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }

        break;
#endif

#if CONFIG_WMBUS_ENABLED
        case E_OPTCOMM_CMD_ENABLE_WMBUS_TX:
            if (OPTCOMM_MSG_LEN_ENABLE_WMBUS_TX == ps_msg->ui_len)
            {
                ge_state = E_OPTCOMM_STATE_ENABLING_WMBUS_TX;
            }
            else
            {
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
        break;

        case E_OPTCOMM_CMD_DISABLE_WMBUS_TX:
            if (OPTCOMM_MSG_LEN_DISABLE_WMBUS_TX == ps_msg->ui_len)
            {
                ge_state = E_OPTCOMM_STATE_DISABLING_WMBUS_TX;
            }
            else
            {
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
        break;
#endif /*CONFIG_WMBUS_ENABLE*/
        
        case E_OPTCOMM_CMD_READ_MEMORY_CHECKSUM:
        {
            s_code_verif_params_t s_rxParam;

            if (OPTCOMM_MSG_LEN_READ_CODE_CRC == ps_msg->ui_len)
            {
            // Parse offset
            s_rxParam.ul_offset = ps_msg->auc_payload[0];
            s_rxParam.ul_offset |= ((uint32_t) ps_msg->auc_payload[1]) << 8;
            s_rxParam.ul_offset |= ((uint32_t) ps_msg->auc_payload[2]) << 16;
            s_rxParam.ul_offset |= ((uint32_t) ps_msg->auc_payload[3]) << 24;

            // Parse length
            s_rxParam.ul_length = ps_msg->auc_payload[4];
            s_rxParam.ul_length |= ((uint32_t) ps_msg->auc_payload[5]) << 8;
            s_rxParam.ul_length |= ((uint32_t) ps_msg->auc_payload[6]) << 16;
            s_rxParam.ul_length |= ((uint32_t) ps_msg->auc_payload[7]) << 24;

                if(isValidParamReadMem(&s_rxParam))
                {
                    memcpy(&gs_readMemParam, &s_rxParam,
                            sizeof(s_code_verif_params_t));

                    ge_state = E_OPTCOMM_STATE_CHECK_CHECKSUM_CODE;
                }
                else
                {
                    ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
                }
            }
            else
            {
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
        }
        break;

#if DEBUG_FLOWRATE
        case E_OPTCOMM_CMD_SET_FLOWRATE:

            if(OPTCOMM_MSG_LEN_SET_FLOWRATE == ps_msg->ui_len)
            {
                // Copy the flow rate
                memcpy((uint8_t *) &gl_flowrate, ps_msg->auc_payload,
                    sizeof(int32_t));

                // Change the OPTCOMM's state to set flow rate
                // in next meter's cycle
                ge_state = E_OPTCOMM_STATE_SET_FLOWRATE;
            }
            else
            {
                // Change the OPTCOMM's state to send NACK in the
                // the next meter's cycle
                ge_state = E_OPTCOMM_STATE_RESPONDING_NACK;
            }
            break;
#endif /* DEBUG_FLOWRATE */

        default:

            b_isValidCmd = false;
#if DEBUG_OPTCOMM
            printf("(OPTCOMM): Received invalid COMMAND %d \n", ps_msg->e_cmd);
#endif /* DEBUG_OPTCOMM */

            // Change the OPTCOMM's state to respond invalid
            // command in next meter's cycle
            ge_state = E_OPTCOMM_STATE_RESPONDING_INVALID_CMD;
            break;
    }

    if(b_isValidCmd && gfp_activityCallback)
    {
        // If a valid command has been received and there is an
        // activity callback, so call it
        gfp_activityCallback();
    }
}

/**
 * @brief
 *      Start to send a message.
 *
 * @detains
 *      This function creates the message header and send it through the
 *      OPTCOMM interface.
 *
 * @param[in]   e_cmd
 *      Message's command.
 *
 * @return   uint16
 *      CRC
 *
 * @param[in]   ui_payloadLen
 *      Message's payload length.
 */
uint16_t startMessage(E_OPTCOMM_CMD_t e_cmd, uint16_t ui_payloadLen)
{
    uint8_t uc_len = 0;
    uint16_t ui_msgLen = OPTCOMM_MSG_HEADER_LEN + OPTCOMM_CRC_LEN;

    ui_msgLen += ui_payloadLen;

    // Command
    gauc_buffer[uc_len++] = e_cmd;

    // Protocol version
    gauc_buffer[uc_len++] = OPTCOMM_VERSION;

    // Total message length
    memcpy(&gauc_buffer[uc_len], (uint8_t *) &ui_msgLen, OPTCOMM_MSG_SIZE_LEN);
    uc_len += OPTCOMM_MSG_SIZE_LEN;

    // Start to calculate the message's CRC
    crc_init();
    crc_calc(gauc_buffer, uc_len);

    // Enable transmission
    //optint_enableTx(true);

    // Send the message header
    optint_send(gauc_buffer, uc_len);

    return crc_result();
}

/**
 * @brief
 *      Finish the message.
 *
 * @details
 *      When this function is called, it finishes the CRC calculation and send
 *      it through the optical interface.
 */
uint16_t finishMessage(void)
{
    uint16_t ui_crc;

    // Get CRC calculation
    ui_crc = crc_result();

    // Send the CRC through the optical interface
    optint_send((uint8_t *) &ui_crc, sizeof(uint16_t));

    // Disable transmission
    //optint_enableTx(false);
    return ui_crc;
}

/**
 *  @brief
 *      Receive synchronization request command.
 *
 *  @details
 *      Receive a synchronization request through the optical interface and
 *      check the CRC to see if the message is valid.
 *
 *  @return
 *      Returns 'true' if a valid synchronization request has been received.
 *      Otherwise, it returns 'false'.
 */

//bool optcomm_receiveByte(uint8_t *puc_data)
//{
//    bool b_dataToRx = false;
//
//    b_dataToRx = optcomm_RxPeekByte(puc_data);
//
//    return b_dataToRx;
//}

static inline bool receiveSyncRequest(void)
{
    uint8_t data;
    bool b_sync_req_received = false;

    // Enable reception
    optint_enableRx(true);

    if(optint_receive(&data, 1)){
        if(E_OPTCOMM_CMD_SYNC_REQ == data)
            {
                b_sync_req_received = true;

            };
    };

    // Finish the reception
    optint_enableRx(false);

    return b_sync_req_received;
}

/**
 * @brief
 *      Receive a message.
 *
 * @details
 *      Receive a message through the optical interface and check the CRC to
 *      see if the message is valid.
 *
 * @param[out]  ps_msg
 *      Pointer to the message structure where the received message shall be
 *      stored.
 *
 * @return
 *      Returns 'true' if a valid message has been received. Otherwise, it
 *      returns 'false'.
 */
static bool receiveMessage(s_optcomm_msg_t *ps_msg)
{
    uint16_t ui_rxLen = 0;
    uint16_t ui_msgLen = 0;
    uint16_t ui_bytesToReceive = 20;
    bool b_isOnlyHeader = false;
    bool b_isValidCmd = false;
    bool b_isReception = false;

    // Enable reception
    optint_enableRx(true);

    do
    {
        // Receive a specific number of bytes
        b_isReception = optint_receive(&gauc_buffer[ui_rxLen], ui_bytesToReceive);
        if(!b_isReception)
        {
            // If an error or timeout happens, finish the reception
            ui_rxLen = 0;
            break;
        }


        if(!ui_msgLen)
        {
            // If the message length is not known, so the message header was
            // just received and it contains the message length. So copy it.
            memcpy(&ui_msgLen, &gauc_buffer[2], OPTCOMM_MSG_SIZE_LEN);

        }

        // Update received length
        if(ui_rxLen + ui_bytesToReceive < ui_msgLen){
            ui_rxLen += ui_bytesToReceive;
        }

        // Calculate the number of bytes pending to receive
        if(ui_msgLen < ui_bytesToReceive){
            ui_bytesToReceive = 0;
            ui_rxLen = ui_msgLen;
        }
        else{
            ui_bytesToReceive = ui_msgLen - ui_rxLen;
        }


        // ui_msgLen has been set and has received only 4 bytes
        if(ui_msgLen)
        {
            if(ui_msgLen > 40)
            {
                // ui_rxLen leaves as OPTCOMM_MSG_HEADER_LEN = 4
                // sets flag to indicate only possible to receive header as
                // payload is longer then clcyle allows
                b_isOnlyHeader = true;
                break;
            }
        }


    }
    while(ui_bytesToReceive);

    // Finish the reception
    optint_enableRx(false);

    // Check if it received a message with at least with a number of bytes
    // enough for header and CRC
    //if (ui_rxLen >= (OPTCOMM_MSG_HEADER_LEN + OPTCOMM_CRC_LEN))
    if (ui_rxLen >= (OPTCOMM_MSG_HEADER_LEN + OPTCOMM_CRC_LEN))
    {
        ps_msg->e_cmd = (E_OPTCOMM_CMD_t) gauc_buffer[0];
        memcpy(&ps_msg->ui_len, &ui_msgLen, OPTCOMM_MSG_SIZE_LEN);

        switch (ps_msg->e_cmd){
            case E_OPTCOMM_CMD_READ_LOGGER_CONFIG:
            case E_OPTCOMM_CMD_UPDATE_LOGGER_CONFIG:
            case E_OPTCOMM_CMD_READ_LOGS:
            case E_OPTCOMM_CMD_CHANGE_OPMODE:
            case E_OPTCOMM_CMD_READ_SERIAL_NUMBER:
            case E_OPTCOMM_CMD_READ_FIRMWARE_VERSION:
            case E_OPTCOMM_CMD_READ_CONFIG_PAGE:
#if OPTCOMM_CHANGE_CONFIG
            case E_OPTCOMM_CMD_WRITE_CONFIG_PAGE:
#endif
            case E_OPTCOMM_CMD_SYNC_REQ:
            case E_OPTCOMM_CMD_SYNC:
            case E_OPTCOMM_CMD_SYNC_CONN:
            case E_OPTCOMM_CMD_SYNC_ACK:
            case E_OPTCOMM_CMD_ACK:
            case E_OPTCOMM_CMD_NACK:
            case E_OPTCOMM_CMD_INVALID:
#if CONFIG_OPTCOMM_ERASE_LOGS
            case E_OPTCOMM_CMD_ERASE_LOGS:
#endif
#if DEBUG_FLOWRATE
            case E_OPTCOMM_CMD_SET_FLOWRATE:
#endif
#if CONFIG_WMBUS_ENABLED
            case E_OPTCOMM_CMD_ENABLE_WMBUS_TX:
            case E_OPTCOMM_CMD_DISABLE_WMBUS_TX:
#endif
            case E_OPTCOMM_CMD_READ_MEMORY_CHECKSUM:

            b_isValidCmd = true;
            break;

            default:

            b_isValidCmd = false;
            break;
        }

        if(b_isValidCmd)
        {
            // Copy the valid message to the message structure
            ps_msg->uc_version = gauc_buffer[1];


            // If message is short enough for the cycle CRC should be checked
            if(!b_isOnlyHeader)
            {
                if(ui_rxLen >= (OPTCOMM_MSG_HEADER_LEN + OPTCOMM_CRC_LEN))
                {
                    // If it is not only a header
                    // Copy the received CRC
                    memcpy(&ps_msg->ui_crc, &gauc_buffer[ui_rxLen - OPTCOMM_CRC_LEN],
                    OPTCOMM_CRC_LEN);

                    // Calculate the message CRC
                    crc_init();
                    crc_calc(gauc_buffer, ui_rxLen - OPTCOMM_CRC_LEN);


                    // Check with the received CRC matches with the calculated CRC
                    if(crc_result() == ps_msg->ui_crc || ps_msg->e_cmd == E_OPTCOMM_CMD_CHANGE_OPMODE)
                    {
                        memcpy(&ps_msg->ui_len, &gauc_buffer[2], OPTCOMM_MSG_SIZE_LEN);
                        memcpy(ps_msg->auc_payload,
                            &gauc_buffer[OPTCOMM_MSG_HEADER_LEN],
                            ui_rxLen - OPTCOMM_MSG_HEADER_LEN - OPTCOMM_CRC_LEN);

                    }

                }
            }
        }
        else
        {
            // The received message is invalid
            ui_rxLen = 0;

    #if DEBUG_OPTCOMM
            printf("(OPTCOMM): Received invalid message.\n");
    #endif /* DEBUG_OPTCOMM */
        }


    }
    else
    {
        // The received bytes is too short for a valid message
        ui_rxLen = 0;
#if DEBUG_OPTCOMM
        printf("(OPTCOMM): Message timeout.\n");
#endif /* DEBUG_OPTCOMM */

    }

    return (ui_rxLen > 0)? true : false;
}


/**
 * @brief
 *      Reads and sends the meter's config page
 *
 * @details
 *      This function reads the configuration structure of the meter and sends
 *      the data to the reader
 *
 * @return
 *      Returns 'true' if there is any pending log to send. Otherwise it
 *      returns 'false'.
 */

#if CONFIG_OPTCOMM_ENABLED
static inline bool readConfigPage(void)
{
    bool b_pendingBytesToSend = true;
    uint16_t ui_remainingBytes;
    static uint16_t crc16 = 0;

    if(guc_configPage == 0)
    {
        // Start a message to send the serial number
        crc16 = startMessage(E_OPTCOMM_CMD_READ_CONFIG_PAGE,
                             sizeof(s_sys_config_t));
    }

    if(guc_configPage < OPTCOMM_MSG_CONFIG_NUM_PAGES)
    {
        ui_remainingBytes = (sizeof(s_sys_config_t)-(guc_configPage*OPTCOMM_MSG_CONFIG_MAX_PAYLOAD));

        if(ui_remainingBytes < OPTCOMM_MSG_CONFIG_MAX_PAYLOAD)
        {
            // This changes the number of bytes to send in case that the remaining
            // bytes are less then the max payload
            ui_remainingBytes = OPTCOMM_MSG_CONFIG_MAX_PAYLOAD;
        }

#if DEBUG_OPTCOMM
        printf("(OPTCOMM): page  %d ----\n", page);
        printf("(OPTCOMM): ui_remainingBytes  %d\n", ui_remainingBytes);
        printf("(OPTCOMM): page*ui_maxPayLoad %d\n", page*OPTCOMM_MSG_CONFIG_MAX_PAYLOAD);
        printf("(OPTCOMM): crc %04x\n", crc16);
#endif /* DEBUG_OPTCOMM */

        memcpy(&gauc_buffer,
               (uint8_t *) gps_sysConfig + (guc_configPage*OPTCOMM_MSG_CONFIG_MAX_PAYLOAD),
                ui_remainingBytes);

        // Add config page to CRC calculation
        // First we need to add previuos to initialization
        crc_initWithPrevious(crc16);
        crc_calc(&gauc_buffer[0], ui_remainingBytes);
        // stores crc16 to increment next round
        crc16 = crc_result();

        // Send page
        optint_send(gauc_buffer, ui_remainingBytes);
        guc_configPage++;
        if(guc_configPage < OPTCOMM_MSG_CONFIG_NUM_PAGES)
        {
            // disable transmition to save power between cycles
            // Keep 'on' for the last cicle
            //optint_enableTx(false);
        }
        b_pendingBytesToSend = true;
    }
    if(guc_configPage == OPTCOMM_MSG_CONFIG_NUM_PAGES)
    {
        finishMessage();
        guc_configPage = 0;
        b_pendingBytesToSend = false;
    }

    return b_pendingBytesToSend;
}
#endif
/**
 * @brief
 *      Calculate checksum from code memory using CRC16 algorithm.
 *
 * @details
 *      This function calculates the CRC16 from memory code
 *
 * @return
 *      Returns 'true' if there is any pending log to send. Otherwise it
 *      returns 'false'.
 */
static inline bool calcCrcFromMemory(void)
{
    static uint16_t ui_crc16 = 0;
    bool b_pendingBytesToCheck = true;

    crc_init();

    // Calculate crc16
    crc_calc((uint8_t*) gs_readMemParam.ul_offset, gs_readMemParam.ul_length);

    // Clear remaining
    gs_readMemParam.ul_length = 0;

    // Prepare response to send
    if(gs_readMemParam.ul_length == 0)
    {

        ui_crc16 = crc_result();
        // Start a message to send and set length of message
        startMessage(E_OPTCOMM_CMD_READ_MEMORY_CHECKSUM, 
            sizeof(uint16_t));
        
        // Build response
        gauc_buffer[0] = ui_crc16 >> 8;
        gauc_buffer[1] = ui_crc16;

        // Calculate CRC from PAYLOAD
        crc_calc(gauc_buffer, sizeof(uint16_t));

        // Send CRC result from memory
        optint_send(gauc_buffer, sizeof(uint16_t));

        // Send the crc16 from message
        finishMessage();

        // Reset flag
        b_pendingBytesToCheck = false;
    }

    return b_pendingBytesToCheck;
}

/**
 * @brief
 *      Updates the system configuration
 *
 * @param[in]   ps_config
 *      Logger configuration to be saved.
 */
#if OPTCOMM_CHANGE_CONFIG
static bool receivedSysConfig(s_sys_config_t *ps_sysConfig, s_optcomm_msg_t *gs_msg)
{
    static uint8_t page = 0;
    // Starts with true to send ack command
    static bool b_successSentData = true;

    static bool b_finished = false;
    static uint8_t  ui_tries = 0;
    bool b_pendingReception = true;
    static bool b_firstAck = true;
    static uint16_t crc16 = 0;
    uint16_t receivedCrc16 = 0;

    uint16_t ui_numBytesToReceive = 40;

    if(b_successSentData)
    {
        // Send ACK before opening window
        startMessage(E_OPTCOMM_CMD_ACK, 0);
        finishMessage();
#if DEBUG_OPTCOMM
        printf("****ACK****\n\n");
#endif
    }
    else
    {
        // Send NACK before opening window
        startMessage(E_OPTCOMM_CMD_NACK, 0);
        finishMessage();
#if DEBUG_OPTCOMM
        printf("****NACK****\n\n");
#endif
    }

    // Ends reception and resets all flags
    if(b_finished)
    {
        // Check if crc message was successfull
        if(b_successSentData){
            // Save new configuration
            configmng_setSysConfig(ps_sysConfig);

            // Update all modules with new configuration
            configmng_updateSysConfig();
        }

        // Resets all flags
        b_successSentData = true;
        b_finished = false; // resets flag
        ui_tries = 0;
        page = 0;
        b_firstAck = true;
        b_pendingReception = false;
        return b_pendingReception;
    }

    if(!b_firstAck)
    {


        // Calculate the remaining packages to receive 480 bytes of calibLut and 2
        // bytes for CRC16
        ui_numBytesToReceive = (482-(page*OPTCOMM_MSG_CONFIG_MAX_PAYLOAD));

        if(ui_numBytesToReceive > OPTCOMM_MSG_CONFIG_MAX_PAYLOAD)
        {
            // This changes the number of bytes to send in case that the remaining
            // bytes are less then the max payload
            ui_numBytesToReceive = OPTCOMM_MSG_CONFIG_MAX_PAYLOAD;
        }


        if(page <= OPTCOMM_MSG_CONFIG_NUM_PAGES/*=12*/)
        {

            uint8_t *p_result;

            // Initializes the crc16 with previous calculated value
            // First initialization happens on first ack where the crc16 for the
            // header is calculated and stored in the static crc16 variable
            crc_initWithPrevious(crc16);

            // Enable reception
            optint_enableRx(true);

            // Receive a specific number of bytes
            if(optint_receive(gauc_buffer, ui_numBytesToReceive))
            {
                if(OPTCOMM_CRC_LEN < ui_numBytesToReceive)
                {
                    // Copy received bytes to sysConfig
                    p_result = (uint8_t *) ps_sysConfig->s_calibration.as_calibLut+(page*OPTCOMM_MSG_CONFIG_MAX_PAYLOAD);
                    p_result = memcpy(p_result, gauc_buffer, ui_numBytesToReceive);

                    // Calculates the current crc16 accumulated from previous
                    // packs
                    crc_calc(gauc_buffer, ui_numBytesToReceive);

                    // updates crc16
                    crc16 = crc_result();
#if DEBUG_OPTCOMM
                    printf("(OPTCOMM): %d\n", gauc_buffer[0]);
                    printf("(OPTCOMM): %d\n", gauc_buffer[ui_numBytesToReceive-1]);
#endif
                }
                else
                {
                    if(OPTCOMM_CRC_LEN == ui_numBytesToReceive)
                    {


                        // copy data to crc16 variable
                        p_result = memcpy(&receivedCrc16, &gauc_buffer[0], ui_numBytesToReceive);

                        // indicates the end of reception
                        b_finished = true;

                        crc16 = crc_result();

#if DEBUG_OPTCOMM
                        printf("(OPTCOMM): CRC rece %08x calc %08x\n", receivedCrc16, crc16);
#endif
                        if(receivedCrc16 == crc16)
                        {

                        }

                    }
                }

                // Checks if memory was successfully
                if(NULL != p_result)
                {
                    // Increments the next page
                    page++;

                    // Zeros timeout counter
                    ui_tries = 0;

                    // Sets successfull data. Next cycle stars with ACK
                    b_successSentData = true;
                }
                else
                {
                    // Sets successfull data. Next cycle stars with NACK
                    b_successSentData = false;
                }
            }
            else
            {
                // Sets unsuccessfull flag so next cycle nack is sent
                b_successSentData = false;

                //increments timeout counter
                ui_tries++;

            }

            // Sets a pending reception to continue in the next cycle
            b_pendingReception = true;

            optint_enableRx(false);
        }

    }
    else
    {
       b_firstAck = false;
       crc16 = 0;
       crc_init();
       // Calculates the header crc
       crc_calc((uint8_t *)gs_msg, 4);
       crc16 = crc_result();
    }



    if(ui_tries == OPTCOMM_MSG_CONFIG_NUM_PAGES + 5)
    {
        // sets flag to send NACK
        b_successSentData = false;
        b_finished = true;
        // sets pending reception to
        b_pendingReception = true;
        page = 0;
        ui_tries = 0;
    }

#if DEBUG_OPTCOMM
        printf("(OPTCOMM): num to receive  %d \n", ui_numBytesToReceive);
        printf("(OPTCOMM): ui_tries %d \n", ui_tries);
        printf("(OPTCOMM): pending  %d \n", b_pendingReception);
        printf("(OPTCOMM): IF success sent  %d \n", b_successSentData);
        printf("(OPTCOMM): NEXT page  %d -------------\n", page);
        printf("(OPTCOMM): NEXT LIMIT page*ui_maxPayLoad %d \n", page*OPTCOMM_MSG_CONFIG_MAX_PAYLOAD);
#endif /* DEBUG_OPTCOMM */

    return b_pendingReception;
}
#endif



#if CONFIG_LOGGER_ENABLED
/**
 * @brief
 *      Read all logs.
 *
 * @details
 *      This function reads all logs, but there is a maximum number of logs
 *      read in one meter's cycle defined by @ref OPTCOMM_MAX_LOGS_PER_CYCLE.
 *      If there is more log when the maximum per cycle, this function shall be
 *      called more times in next meter's cycles to finish the transmission of
 *      all logs.
 *
 * @return
 *      Returns 'true' if there is any pending log to send. Otherwise it
 *      returns 'false'.
 */
static inline bool readLogs()
{
    static uint8_t uc_numLogs = 0;
    uint8_t uc_logsToRead;
    uint8_t uc_idx;
    bool b_pendingLogsToRead = false;

    if(!guc_logIdx)
    {
        uint16_t ui_payloadLen;

        // If it has not read any log yet, get the number of logs to send
        uc_numLogs = logger_size();

#if DEBUG_OPTCOMM
        printf("(OPTCOMM): Found %d logs.\n", uc_numLogs);
#endif /* DEBUG_OPTCOMM */

        // Calculate the length of payload
        ui_payloadLen = uc_numLogs * OPTCOMM_LOG_LEN;

        // Start to send the message
        startMessage(E_OPTCOMM_CMD_READ_LOGS, ui_payloadLen);
    }
    else
    {
        // If this is not first cycle which is sending logs, than get the
        // actual number of logs
        uint8_t uc_actalNumLogs = logger_size();

        if(uc_actalNumLogs > uc_numLogs)
        {
            // If there are more logs than the number when initiated to send
            // the logs, update the log index to avoid to send the same log
            // again
            guc_logIdx += (uc_actalNumLogs - uc_numLogs);
            uc_numLogs = uc_actalNumLogs;
        }

        // Enable the transmission because it was disabled in the previous
        // cycle
        //optint_enableTx(true);
    }

    // Calculate the number of logs to read in this cycle
    uc_logsToRead = uc_numLogs - guc_logIdx;

    // Check if the number of logs is not higher than the maximum number of
    // logs per cycle
    if(uc_logsToRead > OPTCOMM_MAX_LOGS_PER_CYCLE)
    {
        uc_logsToRead = OPTCOMM_MAX_LOGS_PER_CYCLE;
    }

    for(uc_idx = 0; uc_idx < uc_logsToRead; uc_idx++)
    {
        // Read a log
        if(logger_read(&gs_logger, guc_logIdx))
        {
            // If it's a valid log, send it
            sendLog(&gs_logger);
        }

        guc_logIdx++;
    }

    // Check if all logs have been sent
    if(guc_logIdx == uc_numLogs)
    {
        // All logs have been sent, so finish the message
        finishMessage();
    }
    else
    {
        // There are logs pended to be sent, so disable the transmission
        // to save power between this and the next meter's cycle
        //optint_enableTx(false);

        // Indicate that has logs pending to be sent
        b_pendingLogsToRead = true;
    }

#if DEBUG_OPTCOMM
    printf("(OPTCOMM): Sent %d logs.\n", uc_logsToRead);

    if(guc_logIdx == uc_numLogs)
    {
         printf("(OPTCOMM): All logs sent.\n");
    }
    else
    {
        printf("(OPTCOMM): Logs pended to send.\n");
    }
#endif /* DEBUG_OPTCOMM */

    return b_pendingLogsToRead;
}

/**
 * @brief
 *      Send log register.
 *
 * @param[in]   ps_log
 *      Log register to be sent.
 */
static void sendLog(s_logger_reg_t *ps_log)
{
    uint8_t uc_len = 0;
    uint32_t ul_alarms;

    // Pack log's timestamp in the buffer
    gauc_buffer[uc_len++] = ps_log->s_timestamp.tm_year;
    gauc_buffer[uc_len++] = ps_log->s_timestamp.tm_mon;
    gauc_buffer[uc_len++] = ps_log->s_timestamp.tm_mday;
    gauc_buffer[uc_len++] = ps_log->s_timestamp.tm_hour;
    gauc_buffer[uc_len++] = ps_log->s_timestamp.tm_min;
    gauc_buffer[uc_len++] = ps_log->s_timestamp.tm_sec;

    // Pack forward volume
    memcpy(&gauc_buffer[uc_len], &ps_log->ll_forwardVol, 8);
    uc_len += 8;

    // Pack reverse volume
    memcpy(&gauc_buffer[uc_len], &ps_log->ll_reverseVol, 8);
    uc_len += 8;

    // Pack minimum flow rate
    memcpy(&gauc_buffer[uc_len], &ps_log->l_minFlowRate, 4);
    uc_len += 4;

    // Pack maximum flow rate
    memcpy(&gauc_buffer[uc_len], &ps_log->l_maxFlowRate, 4);
    uc_len += 4;

    // Pack alarms
    ul_alarms = parseAlarmFlags(ps_log->ui_alarms);
    memcpy(&gauc_buffer[uc_len], (void *) &ul_alarms, 4);
    uc_len += 4;

    // Add log register to CRC calculation
    crc_calc(gauc_buffer, uc_len);

    // Send log register
    optint_send(gauc_buffer, uc_len);
}

/**
 * @brief
 *      Parse alarm flags from log register to optcomm register.
 *
 * @param[in]   ui_alarms
 *      Alarm flags from log register.
 *
 * @return
 *      Alarm flags to be sent through optcomm register.
 */
static inline uint32_t parseAlarmFlags(uint16_t ui_alarms)
{
    uint32_t ul_alarms = 0;

    // Check battery
    if(ui_alarms & LOGGER_ALARM_FLAG_LOW_BATTERY)
    {
        ul_alarms |= OPTCOMM_ALARM_FLAG_LOW_BATTERY;
    }

    // Check reverse flow
    if(ui_alarms & LOGGER_ALARM_FLAG_BACK_FLOW)
    {
        ul_alarms |= OPTCOMM_ALARM_FLAG_BACK_FLOW;
    }

    // Check leakage
    if(ui_alarms & LOGGER_ALARM_FLAG_LEAKAGE)
    {
        ul_alarms |= OPTCOMM_ALARM_FLAG_LEAKAGE;
    }

    // Check air in pipe
    if(ui_alarms & LOGGER_ALARM_FLAG_AIR_IN_PIPE)
    {
        ul_alarms |= OPTCOMM_ALARM_FLAG_AIR_IN_PIPE;
    }

    // Check brush alarm
    if(ui_alarms & LOGGER_ALARM_FLAG_BRUSH)
    {
        ul_alarms |= OPTCOMM_ALARM_FLAG_BRUSH;
    }

#if DEBUG_FRAUD
    // Check fraud
    //if(ui_alarms & LOGGER_ALARM_FLAG_FRAUDED)
    //{
    //    ul_alarms |= OPTCOMM_ALARM_FLAG_FRAUDED;
    //}
#endif

    // Check no usage alarm
    if(ui_alarms & LOGGER_ALARM_FLAG_NO_USAGE)
    {
        ul_alarms |= OPTCOMM_ALARM_FLAG_NO_USAGE;
    }

    // Check overtemperature alarm
    if(ui_alarms & LOGGER_ALARM_FLAG_OVER_TEMP)
    {
        ul_alarms |= OPTCOMM_ALARM_FLAG_OVER_TEMP;
    }

    return ul_alarms;
}

/**
 * @brief
 *      Read logger configuration
 *
 * @param[in]   ps_config
 *      Logger configuration to be sent.
 */
static inline void readLoggerConfig(s_logger_config_t *ps_config)
{
    uint8_t uc_len = 0;
    int8_t c_param = (int8_t) ps_config->uc_param;

    // Parse weekday to system configuration
    // Note: The weekday in IAR's library starts in Monday, ranging from 0 to 6
    // so, as this is different for the standard, change the received weekday,
    // accordly with the standard, to expected by IAR
    if(ps_config->uc_interval == E_LOGGER_CONFIG_INTERVAL_WEEKLY)
    {
        c_param += 1;

        if (c_param > 6)
        {
            c_param -= 7;
        }
    }

    // Start a message to send the logger configuration
    startMessage(E_OPTCOMM_CMD_READ_LOGGER_CONFIG, OPTCOMM_LOGGER_CONFIG_LEN);

    gauc_buffer[uc_len++] = gps_sysConfig->s_logger.uc_interval;
    gauc_buffer[uc_len++] = (uint8_t) c_param;

    // Add logger configuration to CRC calculation
    crc_calc(gauc_buffer, uc_len);

    // Send logger configuration
    optint_send(gauc_buffer, uc_len);

    // Finish the message
    finishMessage();
}

/**
 * @brief
 *      Update logger configuration
 *
 * @param[in]   ps_config
 *      Logger configuration to be saved.
 */
static inline void updateLoggerConfig(s_logger_config_t *ps_config)
{
    bool b_validInterval = false;

    switch(ps_config->uc_interval)
    {
        case E_LOGGER_CONFIG_INTERVAL_DISABLED:
        case E_LOGGER_CONFIG_INTERVAL_HOURLY:
        case E_LOGGER_CONFIG_INTERVAL_QUARTER_DAILY:
        case E_LOGGER_CONFIG_INTERVAL_HALF_DAILY:
        case E_LOGGER_CONFIG_INTERVAL_DAILY:
        case E_LOGGER_CONFIG_INTERVAL_15MIN:
            // Valid interval
            b_validInterval = true;
        break;
        case E_LOGGER_CONFIG_INTERVAL_WEEKLY:
            if(ps_config->uc_param < OPTCOMM_MAX_VALID_WEEKDAY)
            {
                b_validInterval = true;
            }
        break;
        case E_LOGGER_CONFIG_INTERVAL_MONTHLY:
            if ((ps_config->uc_param >= OPTCOMM_MIN_VALID_DAY_MONTH) &&
                (ps_config->uc_param <= OPTCOMM_MAX_VALID_DAY_MONTH))
            {
                b_validInterval = true;
            }
        break;

        default:
            // Invalid interval
            b_validInterval = false;
        break;
    }

    if(b_validInterval)
    {
        int8_t c_param = (int8_t) ps_config->uc_param;

        // Parse logger interval to system configuration
        gps_sysConfig->s_logger.uc_interval = ps_config->uc_interval;

        // We're not using IAR anymore.

            // Parse weekday to system configuration
            // Note: The weekday in IAR's library starts in Monday, ranging from 0 to 6
            // so, as this is different for the standard, change the received weekday,
            // accordly with the standard, to expected by IAR
            //if(ps_config->uc_interval == E_LOGGER_CONFIG_INTERVAL_WEEKLY)
            //{
            //    c_param -= 1;

            //    if(c_param < 0)
            //    {
            //       c_param += 7;
            //    }
            //}

        gps_sysConfig->s_logger.uc_param = (uint8_t) c_param;

        // Save new configuration
        configmng_setSysConfig(gps_sysConfig);

        // Update all modules with new configuration
        configmng_updateSysConfig();

        // Start a message to send a ACK
        startMessage(E_OPTCOMM_CMD_ACK, 0);
    }
    else
    {
        // Start a message to send a NACK
        startMessage(E_OPTCOMM_CMD_NACK, 0);
    }

    // Finish the message
    finishMessage();
}

#endif /* CONFIG_LOGGER_ENABLED */

#if OPTCOMM_CHANGE_MODE
/**
 * @brief
 *      Change operating mode.
 *
 * @details
 *      Change opeation mode only if the new mode is to NORMAL or VERIFICATION
 *      mode.
 */
static inline void changeOpmode(E_OPMODE_t e_mode)
{
    bool b_validMode = false;
#if DEBUG_CHANGE_TO_STANDBY
    s_app_config_t *ps_config;
#endif

    // Check if the new mode is valid
    // Note: A valid mode means a mode which the meter can change to through
    // optical interface
    switch(e_mode)
    {
        case E_OPMODE_NORMAL:
        case E_OPMODE_VERIFICATION:

            // The meter can change only to NORMAL and VERIFICATION mode
            // through optical interface
            opmodemng_setActivationFlag(true);
            b_validMode = true;
            break;

#if DEBUG_CHANGE_TO_STANDBY
        case E_OPMODE_STANDBY:

            // Get the latest application configuration
            ps_config = configmng_getAppConfig();

            // The meter can change to STANDBY mode only when it receives the
            // correct permission key.
            if(gul_key == ps_config->ul_permissionKey)
            {
                opmodemng_setActivationFlag(false);
                b_validMode = true;
            }
            break;
#endif

        default:
#if DEBUG_OPTCOMM
            printf("(OPTCOMM): invalid mode %d.\n", e_mode);
#endif /* DEBUG_OPTCOMM */
            // Any other mode is invalid
            b_validMode = false;
            break;
    }

    if(b_validMode)
    {
#if DEBUG_OPTCOMM
        printf("(OPTCOMM): Changing to mode %d.\n", e_mode);
#endif /* DEBUG_OPTCOMM */

        // Change operating mode
        opmodemng_setPreviousOpMode(e_mode);

        // Start a message to send a ACK
        startMessage(E_OPTCOMM_CMD_ACK, 0);
    }
    else
    {
#if DEBUG_OPTCOMM
        printf("(OPTCOMM): Mode cannot be changed.\n");
#endif /* DEBUG_OPTCOMM */

        // Start a message to send a NACK
        startMessage(E_OPTCOMM_CMD_NACK, 0);
    }

    // Finish the message
    finishMessage();
}
#endif /* OPTCOMM_CHANGE_MODE */

static inline bool isValidParamReadMem(s_code_verif_params_t *ps_param)
{
    if(ps_param->ul_offset < FRAM_START)
    {
        return false;
    }
    else if((ps_param->ul_offset + ps_param->ul_length)
            >= FRAM_START + FRAM_LENGTH)
    {
        return false;
    }
    else if((ps_param->ul_length > OPTCOMM_CRC_CHECK_MAX_BYTES_PER_CYCLE))
    {
        return false;
    }
    return true;
}

#endif /* CONFIG_OPTCOMM_ENABLED */
