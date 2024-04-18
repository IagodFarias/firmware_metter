/***************************************************************************//**
 * @brief
 * @file    calib.c
 * @author  Robson Cardoso
 * @date    03/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "calib.h"
#include "types.h"
#include "systick.h"
#include "screenmng.h"
#include "configmng.h"
#include "processing.h"
#include "probe.h"
#include "batt.h"
#include "userint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** Calibration operating mode update interval in milliseconds. */
#define CALIB_UPDATE_INT_MS                     500
#define CALIB_US_SAMPLING_RATE_HZ               2.0

/* Number of screens. */
#define CALIB_NUM_OF_SCREENS                    7

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static inline uint8_t* getTextString(void);

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Screen list. */
typedef enum E_CALIB_SCREEN
{
    /** Splash screen */
    E_CALIB_SCREEN_SPLASH       = 0,
    /** Net volume screen */
    E_CALIB_SCREEN_NET_VOLUME,
    /** Flow rate screen */
    E_CALIB_SCREEN_FLOW_RATE,
    /** Reverse volume screen */
    E_CALIB_SCREEN_REV_VOLUME,
    /** Temperature screen */
    E_CALIB_SCREEN_TEMPERATURE,
    /** Meter's replacement date screen */
    E_CALIB_SCREEN_REPLACEMENT_DATE,
    /** Display test screen */
    E_CALIB_SCREEN_TEST
} E_CALIB_SCREEN_t;

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Screens */
static const s_screen_t gs_calibScreens[] =
{
    /** Splash screen */
    {
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) getTextString,
        .uc_doubleResolution = 0
    },
    /** Net volume screen */
    {
        .e_screenType = E_SCREEN_TYPE_NET_VOLUME,
        .pf_getData = (void (*)(void)) proc_getNetVolume,
        .uc_doubleResolution = 6
    },
    /** Flow rate screen */
    {
        .e_screenType = E_SCREEN_TYPE_FLOWRATE,
        .pf_getData = (void (*)(void)) proc_getCurrentFlowRate,
        .uc_doubleResolution = 3
    },
    /** Reverse volume screen */
    {
        .e_screenType = E_SCREEN_TYPE_REV_VOLUME,
        .pf_getData = (void (*)(void)) proc_getReverseVolume,
        .uc_doubleResolution = 6
    },
    /** Temperature screen */
    {
        .e_screenType = E_SCREEN_TYPE_TEMPERATURE,
        .pf_getData = (void (*)(void)) proc_getTemperature,
        .uc_doubleResolution = 0
    },
    /** Replacement date screen */
    {
        .e_screenType = E_SCREEN_TYPE_REPLACEMENT_DATE,
        .pf_getData = (void (*)(void)) configmng_getReplacementDate,
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
 *      Initialize calibration operating mode.
 */
void calib_init(void)
{
    uint8_t uc_screenId;

    // Set wakeup time interval (ultrasonic measurement sampling interval)
    systick_setWakeUpIntMs(CALIB_UPDATE_INT_MS);

    // Initialize screen manager
    screenmng_init();

    // Register event indicators
    screenmng_registerEventIndicators(proc_isReverseFlow,
                                      NULL,
                                      proc_isAirInPipe,
                                      NULL,
                                      NULL,
                                      NULL,
                                      batt_isLow,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL);

    // Add screens
    for(uc_screenId = 0; uc_screenId < CALIB_NUM_OF_SCREENS; uc_screenId++)
    {
        screenmng_addScreen(&gs_calibScreens[uc_screenId]);
    }

    // Enable ultrasonic measurements
    us_enableMeasurements();

    // Disable processing filters
    proc_disableFilters();

    // Enable processing
    proc_enableProcessing();

#if DEBUG
    proc_resetVolumeCounters();
#endif

    // Set processing sampling rate
    proc_configWriteSamplingRate(CALIB_US_SAMPLING_RATE_HZ);

    // Initialize probe module
    probe_init();
}

/**
 * @brief
 *      Run calibration operating mode.
 */
void calib_run(void)
{
    // Switch screen in case user has pressed optical interface
    if(E_USERINT_CMD_CHANGE_SCREEN == userint_getState())
    {
        screenmng_nextScreen();
    }

#if CONFIG_VARIABLE_US
    // Update configuration for measurements
    configmng_checkAndUpdate();
#endif

    // Update probe module
    probe_update();
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/

/**
 * @brief
 *      Get text string.
 *
 * @return
 *      Returns a pointer to the text string.
 */
static inline uint8_t* getTextString(void)
{
    return "  CALIB  ";
}
