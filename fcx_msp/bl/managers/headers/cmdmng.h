/*******************************************************************************
 * @brief
 * @file    cmdmng.h
 * @author  Robson Cardoso
 * @date    27/03/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/
#ifndef CMDMNG_H_
#define CMDMNG_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CMDMNG_CMD_ACK              0xF1
#define CMDMNG_CMD_NACK             0xF2

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Command enumeration. */
typedef enum E_CMDMNG_CMD
{
    /** Invalid command */
    E_CMDMNG_CMD_INVALID                        = 0x00,
    /** Read firmware version */
    E_CMDMNG_CMD_READ_FIRMWARE_VERSION          = 0x01,
    /** Read serial number */
    E_CMDMNG_CMD_READ_SERIAL_NUMBER             = 0x02,
    /** Write serial number */
    E_CMDMNG_CMD_WRITE_SERIAL_NUMBER            = 0x03,
    /** Read replacement date */
    E_CMDMNG_CMD_READ_REPLACEMENT_DATE          = 0x04,
    /** Write replacement date */
    E_CMDMNG_CMD_WRITE_REPLACEMENT_DATE         = 0x05,
    /** Write pre-calibration operating mode */
    E_CMDMNG_CMD_WRITE_PRE_CALIB_OPMODE         = 0x10,
    /** Write calibration operating mode */
    E_CMDMNG_CMD_WRITE_CALIB_OPMODE             = 0x11,
    /** Write post-calibration operating mode */
    E_CMDMNG_CMD_WRITE_POST_CALIB_OPMODE        = 0x12,
    /** Write standby operating mode */
    E_CMDMNG_CMD_WRITE_STANDBY_OPMODE           = 0x13,
    /** Write normal operating mode */
    E_CMDMNG_CMD_WRITE_NORMAL_OPMODE            = 0x14,
    /** Write verification operating mode */
    E_CMDMNG_CMD_WRITE_VERIFICATION_OPMODE      = 0x15,
    /** Enable data transmission through serial interface */
    E_CMDMNG_CMD_ENABLE_DATA_TRANSMIT           = 0x21,
    /** Disable data transmission through serial interface */
    E_CMDMNG_CMD_DISABLE_DATA_TRANSMIT          = 0x22,
    /** Trim Gain PGA */
    E_CMDMNG_CMD_TRIM_GAIN_PGA                  = 0x23,
    /** Trim Frequency Output channel */
    E_CMDMNG_CMD_TRIM_FREQUENCY                 = 0x24,
    /** Enable filters */
    E_CMDMNG_CMD_ENABLE_FILTERS                 = 0x30,
    /** Disable filters */
    E_CMDMNG_CMD_DISABLE_FILTERS                = 0x31,
    /** Configure calibration parameters */
    E_CMDMNG_CMD_WRITE_CALIB_PARAMETERS         = 0xAA,
    /** Read the whole system configuration */
    E_CMDMNG_CMD_READ_SYS_CONFIG                = 0xBB,
    /** Write the whole system configuration */
    E_CMDMNG_CMD_WRITE_SYS_CONFIG               = 0xCC,
    /** Write actual date and time */
    E_CMDMNG_CMD_WRITE_ACTUAL_DATE_TIME         = 0xD0,
    /** Read actual date and time */
    E_CMDMNG_CMD_READ_ACTUAL_DATE_TIME          = 0xD1,
#if CONFIG_WMBUS_ENABLED
    /** Write wM-Bus configuration */
    E_CMDMNG_CMD_WRITE_WMBUS_CONFIG             = 0xD3,
    /** Read wM-Bus configuration */
    E_CMDMNG_CMD_READ_WMBUS_CONFIG              = 0xD4,
    /** Test wM-Bus transmission */
    E_CMDMNG_CMD_TEST_WMBUS_TX                  = 0xD5,
#endif /* CONFIG_WMBUS_ENABLED */

#if CONFIG_FW_INTEGRITY
    /** Read firmware integrity */
    E_CMDMNG_CMD_READ_FW_INTEGRITY              = 0xDC,
#endif

} E_CMDMNG_CMD_t;

/** Command parameters enumeration. */
typedef enum E_CMDMNG_PARAM
{
    /** Indicates meter is reproved during calibration. */
    E_CMDMNG_PARAM_CALIB_REPROVED           = 0x00,
    /** Indicates meter is approved during calibration. */
    E_CMDMNG_PARAM_CALIB_APPROVED           = 0x01

} E_CMDMNG_PARAM_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void cmdmng_init(void);
void cmdmng_fetch(void);
void cmdmng_dispatch(void);
E_CMDMNG_CMD_t cmdmng_getCmd(void);

#endif /* CMDMNG_H_ */
