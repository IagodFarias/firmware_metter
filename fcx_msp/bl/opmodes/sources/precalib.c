/***************************************************************************//**
 * @brief
 * @file    precalib.c
 * @author  Robson Cardoso
 * @date    03/04/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "precalib.h"
#include "general_settings.h"
#include "types.h"
#include "systick.h"
#include "processing.h"
#include "screenmng.h"
#include "userint.h"
#include "power.h"
#include "us.h"
#include "configmng.h"
#include "processing.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Pre-calibration operating mode update interval in milliseconds. */
#define PRECALIB_UPDATE_INT_MS                  1000

/** Number of screens. */
#define PRECALIB_NUM_OF_SCREENS                 3

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Screen list. */
typedef enum E_PRECALIB_SCREEN
{
    /** Blank screen */
    E_PRECALIB_SCREEN_BLANK = 0,
    /** Display test screen */
    E_PRECALIB_SCREEN_TEST,
    /** Firmware version screen */
    E_PRECALIB_SCREEN_FW_VER
} E_PRECALIB_SCREEN_t;

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Screens */
static const s_screen_t gs_precalibScreens[] =
{
    /** Blank screen */
    {
        .e_screenType = E_SCREEN_TYPE_BLANK,
        .pf_getData = (void (*)(void)) NULL,
        .uc_doubleResolution = 0
    },
    /** Test screen */
    {
        .e_screenType = E_SCREEN_TYPE_TEST,
        .pf_getData = (void (*)(void)) NULL,
        .uc_doubleResolution = 0
    },
    /** Text screen */
    {
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) configmng_getFirmwareVersion,
        .uc_doubleResolution = 0
    }
};

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize pre-calibration operating mode.
 */
void precalib_init(void)
{
    // Set wakeup time interval
    systick_setWakeUpIntMs(PRECALIB_UPDATE_INT_MS);

    // Initialize screen manager
    screenmng_init();

    // Add screens
    screenmng_addScreen(&gs_precalibScreens[E_PRECALIB_SCREEN_BLANK]);
    screenmng_addScreen(&gs_precalibScreens[E_PRECALIB_SCREEN_TEST]);
    screenmng_addScreen(&gs_precalibScreens[E_PRECALIB_SCREEN_FW_VER]);

    // Disable ultrasonic measurements
    us_disableMeasurements();

    // Disable processing
    proc_disableProcessing();

    // Reset volume counters
    proc_resetVolumeCounters();

    // Configure power module
    power_init();
    power_enableCoreWakeupBySerial();
    power_enableCoreWakeupByMeasurementFinished();
    power_enableUsWakeupByRTC();
}

/**
 * @brief
 *      Run pre-calibration operating mode.
 */
void precalib_run(void)
{
    // Update active screen index
    if(E_PRECALIB_SCREEN_BLANK == screenmng_getActiveScreenId())
    {
        screenmng_setScreen(E_PRECALIB_SCREEN_TEST);
    }
    else if(E_PRECALIB_SCREEN_TEST == screenmng_getActiveScreenId())
    {
        screenmng_setScreen(E_PRECALIB_SCREEN_BLANK);
    }

    // Show splash screen in case user has pressed optical interface
    if(E_USERINT_CMD_CHANGE_SCREEN == userint_getState())
    {
        screenmng_setScreen(E_PRECALIB_SCREEN_FW_VER);
    }
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
