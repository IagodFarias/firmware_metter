/***************************************************************************//**
 * @brief
 * @file    comm.c
 * @author  Gustavo SalomÃ£o
 * @date    04/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "comm.h"
#include "general_settings.h"
#include "types.h"
#include "systick.h"
#include "screenmng.h"
#include "optcomm.h"
#include "logger.h"
#include "fraud.h"
#include "power.h"
#include "processing.h"
#include "opmodemng.h"
#include "dfsm.h"

#if DEBUG_COMM
#include <stdio.h>
#endif

#if CONFIG_OPTCOMM_ENABLED
/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Inactivity timout in milliseconds. */
#define COMM_INACTIVITY_TIMEOUT_MS              5000

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Screen list. */
typedef enum E_COMM_SCREEN
{
    /** Splash screen */
    E_COMM_SCREEN_SPLASH = 0
} E_COMM_SCREEN_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static inline uint8_t* getTextString(void);
static void onActivityInOptcomm();

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Screens */
static const s_screen_t gs_commScreens[] =
{
    /** Splash screen */
    {
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) getTextString,
        .uc_doubleResolution = 0
    }
};

        // Not currently needed, maintain normal's speed

/** High frequency scheme. */
static const s_freq_scheme_t gs_highFreqScheme = {
    .ui_wakeUpIntMs = 1000,
    .d_samplingRate = 1.0
};

/** Low frequency scheme. */
static const s_freq_scheme_t gs_lowFreqScheme = {
    .ui_wakeUpIntMs = 1000,
    .d_samplingRate = 1.0
};

static uint64_t gull_inactivityTimeout;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize communication operating mode.
 */
void comm_init(void)
{
    // Initialize dynamic frequency scaling manager
    dfsm_init();

    // Set low and high frequency schemes
    dfsm_setHighFreqScheme(&gs_highFreqScheme);
    dfsm_setLowFreqScheme(&gs_lowFreqScheme);

    // Force a system wake up time update
    dfsm_checkFrequency(false, false);

    // Initialize screen manager
    screenmng_init();

    // Add Splash screen
    screenmng_addScreen(&gs_commScreens[E_COMM_SCREEN_SPLASH]);

    // Set activity callback
    optcomm_setActivityCallback(onActivityInOptcomm);

    gull_inactivityTimeout = systick_getSysTickMs();
    gull_inactivityTimeout += COMM_INACTIVITY_TIMEOUT_MS;

#if DEBUG_COMM
    printf("(COMM): COMM Initialize.\n");
#endif
}

/**
 * @brief
 *      Run communication operating mode.
 */
void comm_run(void)
{

#if CONFIG_LOGGER_ENABLED
    // Update logger
    logger_update();
#endif /* CONFIG_LOGGER_ENABLED */

    // Check if inactivity timeout has occurred
    if(systick_getSysTickMs() < gull_inactivityTimeout)
    {
        // While the inactivity timeout has not occurred, update the optcomm
        optcomm_update();
    }
    else
    {
        // The inactivity timeout has occurred, so disconnect optcomm
        optcomm_disconnect();
    }

#if CONFIG_FRAUD_ENABLED
    // Update fraud detection
    fraud_update();
#endif /* CONFIG_FRAUD_ENABLED */
}

/**
 * @brief
 *       Test whether meter shall returns to previous operating mode.
 *
 * @return
 *      Returns 'true' if this is the case. Otherwise, returns 'false'.
 */
bool comm_testChangeToPreviousOpmode(void)
{
    return optcomm_isConnected()? false : true;
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
    return "   COM   ";
}

static void onActivityInOptcomm()
{
    gull_inactivityTimeout = systick_getSysTickMs();
    gull_inactivityTimeout += COMM_INACTIVITY_TIMEOUT_MS;
}

#endif /* CONFIG_OPTCOMM_ENABLED */
