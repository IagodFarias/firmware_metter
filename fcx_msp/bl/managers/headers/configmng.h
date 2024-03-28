/***************************************************************************//**
 * @brief
 * @file    configmng.h
 * @author  Robson Cardoso
 * @date    28/02/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef CONFIGMNG_H_
#define CONFIGMNG_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>

#include "build.h"
#include "types.h"
#include "processing.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Version of the Major number (based on Semantic Versioning 2.0.0) */
#define CONFIG_VERSION_MAJOR                        3

/** Version of the Minor number (based on Semantic Versioning 2.0.0) */
#define CONFIG_VERSION_MINOR                        0

/** Version of the Patch number (based on Semantic Versioning 2.0.0) */
#define CONFIG_VERSION_PATCH                        1

#ifndef CONFIG_VERSION_BUILD
#error "Build not defined."
#endif

/** Version of hardware version (based on Semantic Versioning; 0 to 16) */
#define CONFIG_HARDWARE_VERSION                     2

/** Address where system configuration is stored. */
#define ADDR_SYS_CONFIG                 0xFA00


/** Length of the calibration instant. */
#define CONFIG_CALIBRATION_DATE_LEN     14 //yyyyMMdd + hhmmss

/** Length of the replacement date. */
#define CONFIG_REPLACEMENT_DATE_LEN     8

/** Length of the serial number. */
#define CONFIG_SERIAL_NUMBER_LEN        12

/** Length of the firmware version. */
#define CONFIG_FIRMWARE_VERSION_LEN     8

/** Length of the configuration array. */
#define CONFIG_STRUCT_SIZE              684U

/** Number of bytes in constant array */
#define CONFIG_CONSTS_MAX_SIZE          480U

/** Default mechanical configuration. */
#define CONFIG_MECHANICAL_DEFAULT                                               \
{                                                                               \
    .d_flowPathLength           = 0.084,                                        \
    .d_flowPathDiameter         = 0.012,                                        \
    .d_flowPathArea             = 1.13097335529232550e-04                       \
}

/** Default calibration configuration. */
#define CONFIG_CALIBRATION_DEFAULT                                              \
{                                                                               \
    .d_delaySpeedOfSound        = 4.635524165006239e-06,                        \
    .d_delayZeroCalibration     = -4.078945934813359e-12,                       \
    .as_calibLut                =                                               \
    {                                                                           \
        {5.43697008743456e+01, 0.00000000000000e+00},                           \
        {5.49188897720663e+01, 9.91295289412225e-01},                           \
        {1.47360511314782e+02, 1.10143921045803e+00},                           \
        {3.06583616736170e+02, 1.21244525518681e+00},                           \
        {3.47417702242540e+02, 1.23061535889860e+00},                           \
        {4.64514096456879e+02, 1.29056738043095e+00},                           \
        {5.62522692619045e+02, 1.32796916298646e+00},                           \
        {6.92813615681723e+02, 1.36631593414504e+00},                           \
        {9.96780031337439e+02, 1.42680840689288e+00},                           \
        {2.66487169910239e+03, 1.51038500371970e+00},                           \
        {4.04115302040682e+03, 1.51245584953874e+00},                           \
        {1.02440462599425e+04, 1.51115824143401e+00},                           \
        {1.88997889881089e+04, 1.49957147797925e+00},                           \
        {2.45372629122080e+04, 1.49832426612413e+00},                           \
        {3.74931056535558e+04, 1.51346442419847e+00},                           \
        {5.10665029116272e+04, 1.50551017885878e+00},                           \
        {7.59985846997168e+04, 1.50682529940696e+00},                           \
        {9.10456023149916e+04, 1.50674741224741e+00},                           \
        {1.02643165855042e+05, 1.50809834287638e+00},                           \
        {1.26734269777512e+05, 1.50609883208934e+00},                           \
        {2.92900745451020e+05, 1.50609883208934e+00},                           \
        {2.95829752905531e+05, 0.00000000000000e+00},                           \
        {0, 0},                                                                 \
        {0, 0},                                                                 \
        {0, 0},                                                                 \
        {0, 0},                                                                 \
        {0, 0},                                                                 \
        {0, 0},                                                                 \
        {0, 0},                                                                 \
        {0, 0}                                                                  \
    },                                                                          \
    .ui_calibLutSize            = 22                                            \
}

/** Default system configuration. */
#define CONFIG_SYS_DEFAULT                                                      \
{                                                                               \
    .s_firmware =                                                               \
    {                                                                           \
        .ui_fwMajorVersion          = CONFIG_VERSION_MAJOR,                     \
        .ui_fwMinorVersion          = CONFIG_VERSION_MINOR,                     \
        .ui_fwPatchVersion          = CONFIG_VERSION_PATCH,                     \
        .ui_fwBuildVersion          = CONFIG_VERSION_BUILD                      \
    },                                                                          \
    .s_production =                                                             \
    {                                                                           \
        .auc_calibrationDate        = {'1', '2', '3', '4', '5', '6', '7', '8',  \
                                       '9', 'A', 'B', 'C', 'D', 'E'},           \
        .auc_replacementDate        = {'1', '3', '0', '2', '1', '0', '1', '0'}, \
        .auc_serialNumber           = {'1', '2', '3', '4', '5', '6', '7', '8',  \
                                       '9', '0', '0', '0'}                      \
    },                                                                          \
    .s_mechanical                   = CONFIG_MECHANICAL_DEFAULT,                \
    .s_calibration                  = CONFIG_CALIBRATION_DEFAULT,               \
    .s_ultrasonic                   = US_CONFIG_ALL_DEFAULT,                    \
    .s_pulse =                                                                  \
    {                                                                           \
        .d_pulsesPerCubicMeter      = 10.0                                      \
    },                                                                          \
    .s_screen =                                                                 \
    {                                                                           \
        .ui_volResolution           = 3                                         \
    },                                                                          \
    .s_logger =                                                                 \
    {                                                                           \
        .uc_interval                = E_LOGGER_CONFIG_INTERVAL_DISABLED,        \
        .uc_param                   = 0                                         \
    },                                                                          \
    .s_wmbus =                                                                  \
    {                                                                           \
        .auc_addr                   = {'0', '0', '0', '0', '0', '0', '0', '0'}, \
        .auc_key                    = {'0', '0', '0', '0', '0', '0', '0', '0',  \
                                       '0', '0', '0', '0', '0', '0', '0', '0'}, \
        .ui_interval                = (14000) /*WMBUS_FIXED_TX_INTERVAL_SEC*/   \
    },                                                                          \
    .s_alarms =                                                                 \
    {                                                                           \
        .ui_q3Meter                 = 4000,                                     \
        .uc_brushPercent            = 25,                                       \
        .ul_brushTimeout            = 1800000UL,                                \
        .ui_revInstallParam         = 10,                                       \
        .uc_noUsageDays             = 30,                                       \
        .uc_highTemp                = 65,                                       \
        .uc_lowTemp                 = 10,                                       \
    }                                                                           \
}

/** Default application configuration. */
#define CONFIG_APP_DEFAULT                                                      \
{                                                                               \
    .b_isCalibrated                 = false,                                    \
    .b_isActivated                  = false,                                    \
    .ul_permissionKey               = 0                                         \
}

/*******************************************************************************
 * Enumarations
 ******************************************************************************/

/** Logger interval. */
typedef enum E_LOGGER_CONFIG_INTERVAL
{
    E_LOGGER_CONFIG_INTERVAL_DISABLED = 0,           // Logger disabled
    E_LOGGER_CONFIG_INTERVAL_HOURLY,                 // A log every hour
    E_LOGGER_CONFIG_INTERVAL_QUARTER_DAILY,          // A log every 6 hours
    E_LOGGER_CONFIG_INTERVAL_HALF_DAILY,             // A log every 12 hours
    E_LOGGER_CONFIG_INTERVAL_DAILY,                  // A log every day
    E_LOGGER_CONFIG_INTERVAL_WEEKLY,                 // A log every week
//    E_LOGGER_CONFIG_INTERVAL_MONTHLY,                // A log every month
    E_LOGGER_CONFIG_INTERVAL_15MIN,                  // A log every 15 minutes
    E_LOGGER_CONFIG_INTERVAL_MONTHLY                // A log every month

} E_LOGGER_CONFIG_INTERVAL_t;

/*******************************************************************************
 * Structures
 ******************************************************************************/

/** Firmware configuration structure. */
typedef struct S_FW_CONFIG
{
    /** Major version updated when incompatible changes are made in firmware. */
    uint16_t ui_fwMajorVersion;
    /** Minor version updated when backwards-compatible changes are made in
        firmware. */
    uint16_t ui_fwMinorVersion;
    /** Patch version updated when backwards-compatible bug-fixes are made in
        firmware. */
    uint16_t ui_fwPatchVersion;
    /** Build version based on firmware configuration. */
    uint16_t ui_fwBuildVersion;
} s_fw_config_t;

/** Production configuration structure. */
typedef struct S_PROD_CONFIG
{
    /** Replacement date. */
    uint8_t auc_calibrationDate[CONFIG_CALIBRATION_DATE_LEN];
    /** Replacement date. */
    uint8_t auc_replacementDate[CONFIG_REPLACEMENT_DATE_LEN];
    /** Serial number. */
    uint8_t auc_serialNumber[CONFIG_SERIAL_NUMBER_LEN];
} s_prod_config_t;

/** Pulse output configuration structure. */
typedef struct S_PULSE_CONFIG
{
    /** Pulse constant in pulses per cubic meter. */
    double_t d_pulsesPerCubicMeter;
} s_pulse_config_t;

/** Screen configuration structure. */
typedef struct S_SCREEN_CONFIG
{
    /** Volume screen resolution. */
    uint16_t ui_volResolution;
} s_screen_config_t;

/** Logger configuration structure. */
typedef struct S_LOGGER_CONFIG
{
    /** Logger interval. */
    uint8_t uc_interval;
    /** Additional Parameter. */
    uint8_t uc_param;
} s_logger_config_t;

/** Read memory code parameters. */
typedef struct S_CODE_VERIF_PARAMS
{
    /** Memory offset. */
    uint32_t ul_offset;
    /** Length to read. */
    uint32_t ul_length;
} s_code_verif_params_t;

/** wM-Bus configuration structure. */
typedef struct S_WMBUS_CONFIG
{
    /** wM-Bus's address. */
    uint8_t auc_addr[8]; // 8*8 = 8 Bytes
    /** Device's cipher key. */
    uint8_t auc_key[16]; // 16 Bytes
    /** Transmission interval in milliseconds. */
    uint16_t ui_interval; // 2 bytes
} s_wmbus_config_t;

/** Alarms configuration structure. */
typedef struct S_ALARMS_CONFIG
{
    /** Q3 from meter. */
    uint16_t ui_q3Meter; // 2 bytes in Liters
    /** Maximum Overflow Alarm configuration in percent 1 to 100%. 0 disable the alarm*/
    uint16_t uc_brushPercent; // 2 byte
    /** Maximum Overflow Alarm configuration. */
    uint32_t ul_brushTimeout; // 4 byte
    /** Reverse install Alarm in liters 0 to 255, when 0 disable the alarm. */
    uint16_t ui_revInstallParam; // 2 byte
    /** No usage alarm in days 1 to 255, when 0 disable the alarm. */
    uint16_t uc_noUsageDays; // 2 byte
    /** High temperature alarm in Celsius, when 0 disable the alarm. */
    uint16_t uc_highTemp; // 2 byte
    /** Low temperature alarm in Celsius, when 0 disable the alarm. */
    uint16_t uc_lowTemp; // 2 byte
} s_alarms_config_t;

/** System configuration structure. */
typedef struct S_SYS_CONFIG 
{
    /** Firmware configuration */
    s_fw_config_t s_firmware; // 8 bytesConfigM
    /** Production configuration */
    s_prod_config_t s_production; // 34 BytesConfigM
    /** Mechanical configuration */
    s_mec_config_t s_mechanical; // 24 Bytes
    /** Calibration configuration */
    s_calib_config_t s_calibration;// 498 Bytes
    /** Ultrasonic configuration */
    s_us_config_t s_ultrasonic; // 66 bytes
    /** Pulse configuration */
    s_pulse_config_t s_pulse; // 8 bytes
    /** Screen configuration */
    s_screen_config_t s_screen; //2 bytes
    /** Logger configuration  */
    s_logger_config_t s_logger; // 2 bytes
    /** wM-Bus configuration */
    s_wmbus_config_t s_wmbus; // 26 bytes
    /** Alarms configuration */
    s_alarms_config_t s_alarms; // 16 bytes
} s_sys_config_t; //total  = 684

/** Application configuration structure. */
typedef struct S_APP_CONFIG
{
    /** Indicates if the meter has been calibrated. */
    bool b_isCalibrated;
    /** Indicates if the meter has been activated. */
    bool b_isActivated;
    /**  Key used to perform critical operation by optical interface. */
    uint32_t ul_permissionKey;
    /** Pulse configuration */
    s_pulse_config_t s_pulse; // 8 bytes
    /** Logger configuration  */
    s_logger_config_t s_logger; // 2 bytes
} s_app_config_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize configuration manager.
 */
void configmng_init(void);

/**
 * @brief
 *      Get system configuration pointer.
 *
 * @return
 *      Pointer to the editable version of the system configuration.
 */
s_sys_config_t * configmng_getSysConfig(void);

/**
 * @brief
 *      Update changes configuration to non volatile memory.
 *
 * @return
 *      Pointer to the editable version of the system configuration.
 */
bool configmng_setSysConfig(s_sys_config_t * ps_config);

/**
 * @brief
 *      Update system configuration.
 */
void configmng_updateSysConfig(void);

/**
 * @brief
 *      Get application configuration pointer.
 *
 * @return
 *      Pointer to the editable version of the application configuration.
 */
s_app_config_t * configmng_getAppConfig(void);

/**
 * @brief
 *      Save application configuration.
 *
 * @param[in]   *ps_appConfig
 *      Pointer to a application configuration structure.
 *
 * @note
 *      The application configuration structure should point to a RAM region.
 *      Otherwise, it will fail.
 *
 * @return
 *      Returns 'true' if configuration is successfully saved. Otherwise, it
 *      returns 'false'.
 */
bool configmng_setAppConfig(s_app_config_t * ps_config);

/**
 * @brief
 *      Get firmware version.
 *
 * @return
 *      Return firmware version.
 */
uint8_t * configmng_getFirmwareVersion(void);

/**
 * @brief
 *      Get meter serial number.
 *
 * @return
 *      Return meter serial number.
 */
uint8_t * configmng_getSerialNumber(void);

/**
 * @brief
 *      Get meter replacement date.
 *
 * @return
 *      Return meter replacement date.
 */
uint8_t * configmng_getReplacementDate(void);

/**
 * @brief
 *      Get raw meter replacement date in format (0xhh, 0xmm, 0xss,
 *          0xdd, 0xmm, 0xaa)
 *
 * @return
 *      Return meter replacement date.
 */
uint8_t * configmng_getReplacementDateCharArray(void);

/**
 * @brief
 *      Get alarm configurations.
 *
 * @return
 *      Return alarm configurations.
 */
s_alarms_config_t * configmng_getAlarmConfig(void);

/**
 * @brief
 *      Calculate CRC16 from non volatile memory.
 *
 * @return
 *      Return CRC result.
 */
uint16_t configmng_calcCrcFromMem(uint32_t ul_offset, uint32_t ul_lenght);

#endif /* CONFIGMNG_H_ */
