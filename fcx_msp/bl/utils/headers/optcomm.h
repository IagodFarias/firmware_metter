/***************************************************************************//**
 * @brief
 * @file    optcomm.h
 * @author  Gustavo Salomão
 * @date    01/11/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/
#if CONFIG_OPTCOMM_ENABLED

#ifndef OPTCOMM_H_
#define OPTCOMM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "configmng.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * Optical communication configurations
 * Enable change mode by optic interface
 */
#define OPTCOMM_CHANGE_MODE                     CONFIG_OPTCOMM_CHANGE_MODE

#ifndef DEBUG_OPTCOMM
#define DEBUG_OPTCOMM                           DEBUG_PRINT_OPTCOMM
#endif /* DEBUG_OPTCOMM */

#define OPTCOMM_ERASE_LOGS                      CONFIG_OPTCOMM_ERASE_LOGS


/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Version of OPTCOMM protocol. */
#define OPTCOMM_VERSION                         1

/** Length of the command field. */
#define OPTCOMM_MSG_CMD_LEN                     1

/** Length of the version field. */
#define OPTCOMM_MSG_VERSION_LEN                 1

/** Length of the size field. */
#define OPTCOMM_MSG_SIZE_LEN                    2

/** Length of the CRC field. */
#define OPTCOMM_CRC_LEN                         2

/** Length of the protocol header. */
#define OPTCOMM_MSG_HEADER_LEN                  OPTCOMM_MSG_CMD_LEN + \
                                                OPTCOMM_MSG_VERSION_LEN + \
                                                OPTCOMM_MSG_SIZE_LEN

/** Maximum payload length. */
#define OPTCOMM_MAX_PAYLOAD_LEN                 OPTCOMM_LOG_LEN

/** Default frame length. */
#define OPTCOMM_DEFAULT_FRAME_LEN               OPTCOMM_MSG_HEADER_LEN + \
                                                OPTCOMM_CRC_LEN

/** Maximum frame length. */
#define OPTCOMM_MAX_FRAME_LEN                   OPTCOMM_MSG_HEADER_LEN + \
                                                OPTCOMM_MAX_PAYLOAD_LEN + \
                                                OPTCOMM_CRC_LEN

/** Maximum logs transmitted per meter's cycle. */
#define OPTCOMM_MAX_LOGS_PER_CYCLE              10

/** Length of log register. */
#define OPTCOMM_LOG_LEN                         34

/** Maximum Bytes transmitted per meter's cycle. */
#define OPTCOMM_MAX_BYTES_PER_CYCLE              OPTCOMM_MAX_LOGS_PER_CYCLE * \
                                                    OPTCOMM_LOG_LEN

/** Length of logger configuration. */
#define OPTCOMM_LOGGER_CONFIG_LEN               2

/** Alarm flags. */
#define OPTCOMM_ALARM_FLAG_LOW_BATTERY               (1 << 0)
#define OPTCOMM_ALARM_FLAG_BACK_FLOW                 (1 << 1)
#define OPTCOMM_ALARM_FLAG_LEAKAGE                   (1 << 2)
#define OPTCOMM_ALARM_FLAG_AIR_IN_PIPE               (1 << 3)
#define OPTCOMM_ALARM_FLAG_TAMPERED                  (1 << 4)
#define OPTCOMM_ALARM_FLAG_BRUSH                     (1 << 5)
#define OPTCOMM_ALARM_FLAG_NO_USAGE                  (1 << 6)
#define OPTCOMM_ALARM_FLAG_OVER_TEMP                 (1 << 7)


/** Messages length. */
#define OPTCOMM_MSG_LEN_SYNC_ACK                (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_READ_LOGGER_CONFIG      (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_UPDATE_LOGGER_CONFIG    (OPTCOMM_DEFAULT_FRAME_LEN + 2)
#define OPTCOMM_MSG_LEN_READ_LOGS               (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_CHANGE_OPMODE           (OPTCOMM_DEFAULT_FRAME_LEN + 5)
#define OPTCOMM_MSG_LEN_READ_SERIAL_NUMBER      (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_READ_FIRMWARE_VERSION   (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_READ_ERASE_LOGS         (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_SET_FLOWRATE            (OPTCOMM_DEFAULT_FRAME_LEN + 4)
#define OPTCOMM_MSG_LEN_READ_CONFIG_PAGE        (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_ENABLE_WMBUS_TX         (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_DISABLE_WMBUS_TX        (OPTCOMM_DEFAULT_FRAME_LEN)
#define OPTCOMM_MSG_LEN_READ_CODE_CRC           (OPTCOMM_DEFAULT_FRAME_LEN + 8)

#define OPTCOMM_MSG_LEN_UPDATE_METER_CONFIG     (OPTCOMM_DEFAULT_FRAME_LEN +    \
                                                    CONFIG_CONSTS_MAX_SIZE)

/** Division of pages on the configuration. */
#define OPTCOMM_MSG_CONFIG_MAX_PAYLOAD           34

/** Division of pages on the configuration. */
#define OPTCOMM_MSG_CONFIG_NUM_PAGES             20

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** OPTCOMM commands. */
typedef enum E_OPTCOMM_CMD
{
    /** Read logger configuration command */
    E_OPTCOMM_CMD_READ_LOGGER_CONFIG = 0,
    /** Update logger configuration command */
    E_OPTCOMM_CMD_UPDATE_LOGGER_CONFIG,
    /** Read logs command */
    E_OPTCOMM_CMD_READ_LOGS,
    /** Change logs command */
    E_OPTCOMM_CMD_CHANGE_OPMODE,
    /** Read serial number */
    E_OPTCOMM_CMD_READ_SERIAL_NUMBER,
    /** Read firmware version */
    E_OPTCOMM_CMD_READ_FIRMWARE_VERSION,
    /** Read configuration page **/
    E_OPTCOMM_CMD_READ_CONFIG_PAGE,
    /** Change configuration page **/
    E_OPTCOMM_CMD_WRITE_CONFIG_PAGE,
    /** Enable wmbus Tx **/
    E_OPTCOMM_CMD_ENABLE_WMBUS_TX,
    /** Disable wmbus Tx **/
    E_OPTCOMM_CMD_DISABLE_WMBUS_TX,
    /** Read memory code checksum **/
    E_OPTCOMM_CMD_READ_MEMORY_CHECKSUM,

    /** Synchronization Request commmand */
    E_OPTCOMM_CMD_SYNC_REQ = 0xA5,
    /** SYNC command (when not connected) */
    E_OPTCOMM_CMD_SYNC,
    /** SYNC command (when connected) */
    E_OPTCOMM_CMD_SYNC_CONN,
    /** SYNC_ACK command */
    E_OPTCOMM_CMD_SYNC_ACK,
    /** ACK command */
    E_OPTCOMM_CMD_ACK,
    /** NACK command */
    E_OPTCOMM_CMD_NACK,
    /** Invalid command */
    E_OPTCOMM_CMD_INVALID

#if CONFIG_OPTCOMM_ERASE_LOGS
    /** Erase logs */
    E_OPTCOMM_CMD_ERASE_LOGS = 0xF0,
#endif

#if DEBUG_FLOWRATE
    /** Set flow rate */
    E_OPTCOMM_CMD_SET_FLOWRATE
#endif
} E_OPTCOMM_CMD_t;

/*******************************************************************************
 * Structures
 ******************************************************************************/
/** OPTCOMM message structure. */
typedef struct S_OPTCOMM_MSG
{
    /** Command (1 byte) */
    E_OPTCOMM_CMD_t e_cmd;
    /** Version (1 byte) */
    uint8_t uc_version;
    /** Length (2 bytes) */
    uint16_t ui_len;
    /** Payload (0 to @ref OPTCOMM_MAX_PAYLOAD_LEN bytes) */
    uint8_t auc_payload[OPTCOMM_MAX_PAYLOAD_LEN];
    /** CRC (2 bytes) */
    uint16_t ui_crc;
} s_optcomm_msg_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void optcomm_init();
void optcomm_update();
bool optcomm_isConnected();
void optcomm_disconnect();
bool optcomm_receiveByte(uint8_t *puc_data);
void optcomm_setActivityCallback(void (*fp_callback)(void));
uint16_t startMessage(E_OPTCOMM_CMD_t e_cmd, uint16_t ui_payloadLen);
uint16_t finishMessage(void);


#endif /* OPTCOMM_H_ */
#endif /* CONFIG_OPTCOMM_ENABLED */
