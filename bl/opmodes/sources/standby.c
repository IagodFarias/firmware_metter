/***************************************************************************//**
 * @brief
 * @file    standby.c
 * @author  Robson Cardoso
 * @date    01/03/2017
 * @details
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"

#include "standby.h"
#include "screenmng.h"
#include "configmng.h"
#include "systick.h"
#include "processing.h"
#include "userint.h"
#include "power.h"
#include "probe.h"
#if CONFIG_FRAUD_ENABLED
#include "fraud.h"
#endif
#if CONFIG_OPTCOMM_ENABLED
#include "optcomm.h"
#endif
#if CONFIG_LOGGER_ENABLED
#include "logger.h"
#endif
#include "aes_cmac.h"
#include "batt.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Stand-by operating mode update interval in milliseconds. */
#define STANDBY_UPDATE_INT_MS                   2000
#define STANDBY_US_SAMPLING_RATE_HZ             0.5

/* Number of screens. */
#define STANDBY_NUM_OF_SCREENS                  5

/** Activation threshold in cubic meters. */
#define STANBY_ACTIVATION_POS_THRESHOLD         0.001
#define STANBY_ACTIVATION_NEG_THRESHOLD         -0.001

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Screen list. */
typedef enum E_SCREEN
{
    /** Blank screen. */
    E_SCREEN_BLANK = 0,
    /** Net volume screen */
    E_SCREEN_NET_VOLUME,
    /** Meter's replacement date screen */
    E_SCREEN_REPLACEMENT_DATE,
    /** Meter's Fw version */
    E_SCREEN_FW_VER,
    /** Display test screen */
    E_SCREEN_TEST
} E_SCREEN_t;

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Screens */
static const s_screen_t gs_standbyScreens[] =
{
    /** Blank screen */
    {
        .e_screenType = E_SCREEN_TYPE_BLANK,
        .pf_getData = (void (*)(void)) NULL,
        .uc_doubleResolution = 0
    },
    /** Net volume screen */
    {
        .e_screenType = E_SCREEN_TYPE_NET_VOLUME,
        .pf_getData = (void (*)(void)) proc_getNetVolume,
        .uc_doubleResolution = 3
    },
    /** Replacement date screen */
    {
        .e_screenType = E_SCREEN_TYPE_REPLACEMENT_DATE,
        .pf_getData = (void (*)(void)) configmng_getReplacementDate,
        .uc_doubleResolution = 0
    },
    /** Fw version date screen */
    {
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) configmng_getFirmwareVersion,
        .uc_doubleResolution = 0
    },
    /** Display test screen */
    {
        .e_screenType = E_SCREEN_TYPE_TEST,
        .pf_getData = (void (*)(void)) NULL,
        .uc_doubleResolution = 0
    }
};

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize stand-by operating mode.
 */
void standby_init(void)
{
    uint8_t uc_screenId;

    // Set wakeup time interval
    systick_setWakeUpIntMs(STANDBY_UPDATE_INT_MS);

    // Initialize screen manager
    screenmng_init();

    // Register event indicators
    screenmng_registerEventIndicators(NULL,
                                      NULL,
                                      NULL,
#if CONFIG_FRAUD_ENABLED
                                      fraud_wasViolated,
#else
                                      NULL,
#endif /* CONFIG_FRAUD_ENABLED */

                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL, 
                                      NULL,
                                      NULL,
                                      NULL);

    // Add screens
    for(uc_screenId = 0; uc_screenId < STANDBY_NUM_OF_SCREENS; uc_screenId++)
    {
        screenmng_addScreen(&gs_standbyScreens[uc_screenId]);
    }

    // Disable data transmission through serial interface
    probe_disableDataTransmission();

    // Enable ultrasonic measurements
    us_enableMeasurements();

    // Enable processing filters
    proc_enableFilters();

    if(!batt_isLow() && !cmac_isNonVerified()){
        // Enable processing
        proc_enableProcessing();
    }

    // Reset volume counters
    proc_resetVolumeCounters();

    // Set processing sampling rate
    proc_configWriteSamplingRate(STANDBY_US_SAMPLING_RATE_HZ);

#if CONFIG_OPTCOMM_ENABLED
    // Initialize optical communication
    optcomm_init();
#endif /* CONFIG_OPTCOMM_ENABLED */

#if CONFIG_LOGGER_ENABLED
    // Disable logger
    logger_enable(false);
#endif /* CONFIG_LOGGER_ENABLED */

    // Configure power module
    power_init();
    power_enableCoreWakeupByMeasurementFinished();
    power_enableUsWakeupByRTC();
}

/**
 * @brief
 *      Run stand-by operating mode.
 */
void standby_run(void)
{
    // Switch screen in case user has pressed optical interface
    if(E_USERINT_CMD_CHANGE_SCREEN == userint_getState())
    {
        screenmng_nextScreen();
    }

#if CONFIG_OPTCOMM_ENABLED
    // Update optcomm
    optcomm_update();
#endif

#if CONFIG_FRAUD_ENABLED
    // Update fraud detection
    fraud_update();
#endif /* CONFIG_FRAUD_ENABLED */
}

/**
 * @brief
 *      Test whether user wants to run the normal operating mode.
 *
 * @return
 *      Returns 'true' if this is the case. Otherwise, returns 'false'.
 */
bool standby_testChangeToVerifOpMode(void)
{
    bool b_status = false;
    double_t d_netVolume = proc_getNetVolume();

    if(d_netVolume != 0.0)
    {
        if(d_netVolume < 0)
        {
            if(d_netVolume <= STANBY_ACTIVATION_NEG_THRESHOLD)
            {
                b_status = true;
            }
        }
        else
        {
            if(d_netVolume >= STANBY_ACTIVATION_POS_THRESHOLD)
            {
                b_status = true;
            }
        }
    }

    return b_status;
}

#if CONFIG_OPTCOMM_ENABLED
/**
 * @brief
 *       Test whether user wants to run the communication operating mode.
 *
 * @return
 *      Returns 'true' if this is the case. Otherwise, returns 'false'.
 */
bool standby_testChangeToCommOpmode(void)
{
    return optcomm_isConnected();
}
#endif /* CONFIG_OPTCOMM_ENABLED */
