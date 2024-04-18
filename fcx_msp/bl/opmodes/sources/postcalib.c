/***************************************************************************//**
 * @brief
 * @file    postcalib.c
 * @author  Gustavo SalomÃ£o
 * @date    07/03/2018
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "postcalib.h"
#include "types.h"
#include "systick.h"
#include "screenmng.h"
#include "processing.h"
#include "configmng.h"
#include "power.h"
#include "userint.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** Post-calibration operating mode update interval in milliseconds. */
#define POSTCALIB_UPDATE_INT_MS                  2000

/** Post-calibation timout */
#define POSTCALIB_TIMEOUT_MS                     10800000ULL  // 3 hours

#if CONFIG_WMBUS_ENABLED
/* Number of screens. */
#define POST_NUM_OF_SCREENS                           3
#else
#define POST_NUM_OF_SCREENS                           2
#endif
/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Operating mode's screen list. */
typedef enum E_SCREEN
{
    /** Flow rate screen */
    E_SCREEN_SERIAL_NUMBER,

#if CONFIG_WMBUS_ENABLED
    /** RADIO wmbus serial number sequence */
    E_SCREEN_RADIO_SERIAL_NUMBER,
#endif
     /** Reverse volume screen */
    E_SCREEN_SPLASH

} E_SCREEN_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static inline uint8_t* getPostTextString(void);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Splash screen */
static const s_screen_t gs_postScreens[] =
{
    {
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) configmng_getSerialNumber,
        .uc_doubleResolution = 0
    },
#if CONFIG_WMBUS_ENABLED
    {
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) configmng_getRfSerialNumber,
        .uc_doubleResolution = 0
    },
#endif
    {
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) getPostTextString,
        .uc_doubleResolution = 0
    }
};

/** Timeout to get out of post-calibration operating mode */
static uint64_t gull_postCalibTimeout;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize post-calibration operating mode.
 */
void postcalib_init()
{
    uint8_t uc_screenId;

    // Set wakeup time interval
    systick_setWakeUpIntMs(POSTCALIB_UPDATE_INT_MS);

    // Initialize screen manager
    screenmng_init();

    // Register event indicators
    screenmng_registerEventIndicators(NULL, NULL, NULL, NULL, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL);

    //add screens
    for(uc_screenId = 0; uc_screenId < POST_NUM_OF_SCREENS; uc_screenId++)
    {
        screenmng_addScreen(&gs_postScreens[uc_screenId]);
    }

    // Disable ultrasonic measurements
    us_disableMeasurements();

    // Disable processing
    proc_disableProcessing();

#if CONFIG_OPTCOMM_ENABLED
    // Initialize optical communication
    optcomm_init();
#endif /* CONFIG_OPTCOMM_ENABLED */

    // Calculate the post-calibration timeout
    gull_postCalibTimeout = systick_getSysTickMs();
    gull_postCalibTimeout += POSTCALIB_TIMEOUT_MS;

    // Configure power module
    power_init();
    power_enableCoreWakeupBySerial();
    power_enableCoreWakeupByMeasurementFinished();
    power_enableUsWakeupByRTC();
}

/**
 * @brief
 *      Run post-calibration operating mode.
 */
void postcalib_run()
{

    // Switch screen in case user has pressed optical interface
    if(E_USERINT_CMD_CHANGE_SCREEN == userint_getState())
    {
        screenmng_nextScreen();
    }
}

/**
 * @brief
 *       Test whether meter shall goes to standby.
 *
 * @return
 *      Returns 'true' if this is the case. Otherwise, returns 'false'.
 */
bool postcalib_testChangeToStandbyOpMode()
{
    return (systick_getSysTickMs() > gull_postCalibTimeout)? true : false;
}


/**
 * @brief
 *      Get text string.
 *
 * @return
 *      Returns a pointer to the text string.
 */
static inline uint8_t* getPostTextString(void)
{
    return "POS. CALIB";
}
