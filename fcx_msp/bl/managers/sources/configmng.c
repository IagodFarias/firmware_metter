/*******************************************************************************
 * @brief
 * @file    configmng.c
 * @author  Eric Costa Hall
 * @date    27/07/2021
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <brush.h>
#include "configmng.h"
#include "processing.h"
#include "general_settings.h"
#include "batt.h"
#include "crc.h"
#include "logger.h"

#if CONFIG_PULSE_ENABLED
#include "pulsemng.h"
#endif

#include <string.h>

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define PATH_LEN                                0.084f
#define FLOW_PATH_DIAMETER                      0.012f
#define FLOW_PATH_AREA                          1.13097335529233e-04f

#if defined(__TI_COMPILER_VERSION__)
#pragma PERSISTENT(gs_config)
#define __persistent
#elif defined (__IAR_SYSTEMS_ICC__)
#else
#error Compiler not supported!
#endif

/** Default device's configuration. */
#define CONFIG_DEFAULT                                                          \
{                                                                               \
    .s_sysConfig = CONFIG_SYS_DEFAULT,                                          \
    .s_appConfig = CONFIG_APP_DEFAULT                                           \
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
STATIC void setFirmwareVersion(uint8_t uc_majorVersion,
    uint8_t uc_minorVersion, uint8_t uc_patchVersion, uint8_t uc_buildVersion);
STATIC void setSerialNumber(uint8_t * puc_serialNumber);
STATIC void setMeterReplacementDate(uint8_t *puc_replacementDate);
STATIC INLINE void cloneConfig(void);

/*******************************************************************************
 * Structures
 ******************************************************************************/
/** Device's configuration. */
typedef struct S_CONFIG
{
    /** System configuration. */
    s_sys_config_t s_sysConfig;
    /** Application configuration */
    s_app_config_t s_appConfig;
} s_config_t;

/*******************************************************************************
 * Global variables
 ******************************************************************************/

#pragma LOCATION(gs_config, ADDR_SYS_CONFIG)
static s_config_t gs_config = CONFIG_DEFAULT;

static s_config_t gs_editableConfig = CONFIG_DEFAULT;

/** Stores firmware version. */
static uint8_t gauc_firmwareVersion[] = "F.VER.  . . . ";

/** Stores meter replacement date. */
static uint8_t gauc_replacementDate[] = "   .  .    ";

/** Stores meter serial number. */
STATIC uint8_t gauc_serialNumber[] = "         ";

/*******************************************************************************
 * Public Functions
 ******************************************************************************/
void configmng_init(void)
{
    // Set firmware version
    setFirmwareVersion(
        (uint8_t) gs_config.s_sysConfig.s_firmware.ui_fwMajorVersion,
        (uint8_t) gs_config.s_sysConfig.s_firmware.ui_fwMinorVersion,
        (uint8_t) gs_config.s_sysConfig.s_firmware.ui_fwPatchVersion,
        (uint8_t) gs_config.s_sysConfig.s_firmware.ui_fwBuildVersion);

    // Update serial number
    setSerialNumber(
            (uint8_t*) gs_config.s_sysConfig.s_production.auc_serialNumber);

    // Update replacement date
    setMeterReplacementDate(
            (uint8_t*) gs_config.s_sysConfig.s_production.auc_replacementDate);

    // Get non volatile memory to RAM
    cloneConfig();
}

s_sys_config_t * configmng_getSysConfig(void)
{
    return &gs_editableConfig.s_sysConfig;
}

bool configmng_setSysConfig(s_sys_config_t * ps_config)
{
    ps_config->s_firmware.ui_fwMajorVersion = CONFIG_VERSION_MAJOR;
    ps_config->s_firmware.ui_fwMinorVersion = CONFIG_VERSION_MINOR;
    ps_config->s_firmware.ui_fwPatchVersion = CONFIG_VERSION_PATCH;
    ps_config->s_firmware.ui_fwBuildVersion = CONFIG_VERSION_BUILD;

    // Save into FRAM
    memcpy(&gs_config, &gs_editableConfig, sizeof(s_config_t));

    return true;
}

void configmng_updateSysConfig(void)
{
    cloneConfig();

#if CONFIG_PULSE_ENABLED
    // Update pulse manager configuration
    pulsemng_configWritePulseConstant(
        gs_config.s_sysConfig.s_pulse.d_pulsesPerCubicMeter);
#endif /* CONFIG_PULSE_ENABLED */

    // Update serial number
    setSerialNumber(
            (uint8_t*) gs_config.s_sysConfig.s_production.auc_serialNumber);

    // Update replacement date
    setMeterReplacementDate(
            (uint8_t*) gs_config.s_sysConfig.s_production.auc_replacementDate);

#if CONFIG_WMBUS_ENABLED
    configmng_setRfSerialNumber(
        (uint8_t *) gs_config.s_sysConfig.s_wmbus.auc_addr);
#endif

#if CONFIG_LOGGER_ENABLED
    // Update logger configuration
    logger_updateConfig();
#endif /* CONFIG_LOGGER_ENABLED */

    // Update brush (maximum overflow) configuration
    brush_updateConfig();

    // Update processing constants
    proc_updateConfig();

    // Battery update date of low battery
    batt_updateConfig();
}

s_app_config_t * configmng_getAppConfig(void)
{
    return &gs_config.s_appConfig;
}

bool configmng_setAppConfig(s_app_config_t * ps_config)
{
    memcpy(&gs_editableConfig.s_appConfig, ps_config, sizeof(s_app_config_t));

    return true;
}

uint8_t* configmng_getFirmwareVersion(void)
{
    return gauc_firmwareVersion;
}

uint8_t * configmng_getSerialNumber(void)
{
    return gauc_serialNumber;
}

uint8_t * configmng_getReplacementDate(void)
{
    return gauc_replacementDate;
}

uint8_t * configmng_getReplacementDateCharArray(void)
{
    return (uint8_t *) &gs_editableConfig.s_sysConfig.s_production.auc_replacementDate;
}

s_alarms_config_t * configmng_getAlarmConfig(void)
{
    return &gs_editableConfig.s_sysConfig.s_alarms;
}

uint16_t configmng_calcCrcFromMem(uint32_t ul_offset, uint32_t ul_length)
{
    uint8_t* puc_addrMem = (uint8_t*) ul_offset;

    // Clear CRC before calculate
    crc_init();

    // Calculate
    crc_calc(puc_addrMem, ul_length);

    return crc_result();
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
/**
 * @brief
 *      Set firmware version.
 *
 * @param[in]   uc_majorVersion
 *      Major version updated when incompatible changes are made in firmware.
 *
 * @param[in]   uc_minorVersion
 *      Minor version updated when backwards-compatible changes are made in
 *       firmware.
 * @param[in]   uc_patchVersion
 *      Path version updated when backwards-compatible bug-fixes are made in
 *       firmware.
 * @param[in]   uc_buildVersion
 *      Build version version updated when functionalities are added to meter
 */
STATIC void setFirmwareVersion(uint8_t uc_majorVersion,
    uint8_t uc_minorVersion, uint8_t uc_patchVersion, uint8_t uc_buildVersion)
{
    gauc_firmwareVersion[7] = uc_majorVersion + '0';
    gauc_firmwareVersion[9] = uc_minorVersion + '0';
    gauc_firmwareVersion[11] = uc_patchVersion + '0';
    gauc_firmwareVersion[13] = uc_buildVersion + '0';
}

STATIC void setSerialNumber(uint8_t * puc_serialNumber)
{
    // Check which standard the serial number follows
    if((puc_serialNumber[CONFIG_SERIAL_NUMBER_LEN - 1] == '0') &&
       (puc_serialNumber[CONFIG_SERIAL_NUMBER_LEN - 2] == '0'))
    {
        // The serial number follows the ABNT 8194:2005 (ex. 00B18F654321), so
        // this serial number shall be shown in this format: (B  654321)
        gauc_serialNumber[0] = puc_serialNumber[9];
        gauc_serialNumber[2] = ' ';
    }
    else
    {

            // The serial number follows the ABNT 8194:2013 (ex. B18FA7654321),
            // so this serial number shall be shown in this format: (B 7654321)
            gauc_serialNumber[0] = puc_serialNumber[11];
            gauc_serialNumber[2] = puc_serialNumber[6];

    }

    gauc_serialNumber[1] = ' ';
    gauc_serialNumber[3] = puc_serialNumber[5];
    gauc_serialNumber[4] = puc_serialNumber[4];
    gauc_serialNumber[5] = puc_serialNumber[3];
    gauc_serialNumber[6] = puc_serialNumber[2];
    gauc_serialNumber[7] = puc_serialNumber[1];
    gauc_serialNumber[8] = puc_serialNumber[0];
}

/**
 * @brief
 *      Set meter's replacement date.
 *
 * @param[in]   puc_replacementDate
 *      Meter's replacement date.
 */
STATIC void setMeterReplacementDate(uint8_t *puc_replacementDate)
{
    // Set day
    gauc_replacementDate[2] = puc_replacementDate[6];
    gauc_replacementDate[1] = puc_replacementDate[7];

    // Set month
    gauc_replacementDate[5] = puc_replacementDate[4];
    gauc_replacementDate[4] = puc_replacementDate[5];

    // Set year
    gauc_replacementDate[10] = puc_replacementDate[0];
    gauc_replacementDate[9] = puc_replacementDate[1];
    gauc_replacementDate[8] = puc_replacementDate[2];
    gauc_replacementDate[7] = puc_replacementDate[3];
}

/**
 * Copy read-only device's configuration to editable version.
 */
STATIC INLINE void cloneConfig(void)
{
    // Copy the Read-only version to editable version of the device's
    // configuration.
    memcpy((void *) &gs_editableConfig, (void *) &gs_config,
            sizeof(s_config_t));
}

/*******************************************************************************
 * Callback functions
 ******************************************************************************/
double_t pulsemng_evt_getPulsePerMeter(void)
{
    return gs_editableConfig.s_sysConfig.s_pulse.d_pulsesPerCubicMeter;
}
