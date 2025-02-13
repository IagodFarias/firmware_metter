/***************************************************************************//**
 * @brief
 * @file    verif.c
 * @author  Robson Cardoso
 * @date    04/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "verif.h"
#include "types.h"
#include "systick.h"
#include "screenmng.h"
#include "configmng.h"
#include "processing.h"
#include "userint.h"
#include "battdate.h"
#include "power.h"
#include "optcomm.h"
#if CONFIG_FRAUD_ENABLED
#include "fraud.h"
#endif
#include "logger.h"
#if CONFIG_PULSE_ENABLED
#include "pulsemng.h"
#endif
#if DEBUG
#include "probe.h"
#endif
#include "aes_cmac.h"
#include "batt.h"


/*******************************************************************************
 * Defines
 ******************************************************************************/
/* Normal operating mode update interval in milliseconds. */
#define VERIF_UPDATE_INT_MS                     500
#define VERIF_US_SAMPLING_RATE_HZ               2.0

/* Manual operating mode change detection timeout in milliseconds. */
#define VERIF_MANUAL_DETECT_TIMEOUT_MS          10000ULL

/* Auto operating mode change detection timeout in milliseconds. */
#define VERIF_AUTO_DETECT_TIMEOUT_MS            43200000ULL // 12 hours

/* Number of screens. */
#define VERIF_NUM_OF_SCREENS                    7

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Operating mode's screen list. */
typedef enum E_SCREEN
{
    /** Net volume screen */
    E_SCREEN_NET_VOLUME = 0,
    /** Flow rate screen */
    E_SCREEN_FLOW_RATE,
    /** Reverse volume screen */
    E_SCREEN_REV_VOLUME,
    /** Temperature screen */
    E_SCREEN_TEMPERATURE,
    /** Meter's replacement date screen */
    E_SCREEN_REPLACEMENT_DATE,
    /** Display FW ver*/
    E_SCREEN_FW_VER,
    /** Display test screen */
    E_SCREEN_TEST,
    /** Blank screen screen */
    E_SCREEN_BLANK
} E_SCREEN_t;

/** Manual operating mode change detection state. */
typedef enum E_MANUAL_DETECT_STATE
{
    /** Check state. */
    E_MANUAL_DETECT_STATE_CHECK,
    /** Confirmation state. */
    E_MANUAL_DETECT_STATE_CONFIRM
} E_MANUAL_DETECT_STATE_t;

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Screens */
static const s_screen_t gs_screens[] =
{
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
        .uc_doubleResolution = 4
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
    },
    /** Blank screen (This screen doesn't count as screen) */
    {
        .e_screenType = E_SCREEN_TYPE_BLANK,
        .pf_getData = (void (*)(void)) NULL,
        .uc_doubleResolution = 0
    }
};

/** Current manual operating mode change detection state. */
static E_MANUAL_DETECT_STATE_t ge_manualDetectionState;

/** Verification mode timeout tick in milliseconds. */
static uint64_t gull_verifTimeoutTickMs;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize verification operating mode.
 */
void verif_init(void)
{
    uint8_t uc_screenId;

    // Set wake up time interval (ultrasonic measurement sampling rate)
    systick_setWakeUpIntMs(VERIF_UPDATE_INT_MS);

    // Initialize screen manager
    screenmng_init();

    // Register event indicators
    screenmng_registerEventIndicators(proc_isReverseFlow,
                                      NULL,
                                      proc_isAirInPipe,
#if CONFIG_FRAUD_ENABLED
                                      fraud_wasViolated,
#else
                                      NULL,
#endif /* CONFIG_FRAUD_ENABLED */

#if CONFIG_WMBUS_ENABLED
                                      wmbus_stack_isTransmitting,
#else
                                      NULL,
#endif /* CONFIG_WMBUS_ENABLED */

#if CONFIG_PULSE_ENABLED
                                      pulsemng_isEndPulse,
#else
                                      NULL,
#endif /* CONFIG_PULSE_ENABLED */
                                      battdate_isOld,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL);

    // Add screens less the blank screen
    for(uc_screenId = 0; uc_screenId < VERIF_NUM_OF_SCREENS; uc_screenId++)
    {
        screenmng_addScreen(&gs_screens[uc_screenId]);
    }

    // Enable ultrasonic measurements
    us_enableMeasurements();

    // Enable processing filters
    proc_enableFilters();

    if(!batt_isLow() && !cmac_isNonVerified()){
        // Enable processing
        proc_enableProcessing();
    }


#if DEBUG
    // Reset volume
    proc_resetVolumeCounters();
#endif

    // Set processing sampling rate
    proc_configWriteSamplingRate(VERIF_US_SAMPLING_RATE_HZ);

    // Unlock screen
    screenmng_lockScreen(false);

    // Reset user interface state to prevent any invalid
    // interaction
    userint_resetState();

    // Set change mode timeout
    userint_setChangeModeTimeoutMs(
        USERINT_CMD_DEFAULT_CHANGE_OP_MODE_TIMEOUT_MS);

    // Initialize detection control variables
    ge_manualDetectionState = E_MANUAL_DETECT_STATE_CHECK;

    // Calculate timeout to finish the verification mode
    gull_verifTimeoutTickMs = systick_getSysTickMs();
    gull_verifTimeoutTickMs += VERIF_AUTO_DETECT_TIMEOUT_MS;

#if CONFIG_OPTCOMM_ENABLED
    // Initialize optical communication
    optcomm_init();
#endif /* CONFIG_OPTCOMM_ENABLED */

#if CONFIG_LOGGER_ENABLED
    // Enable logger
    logger_enable(true);
#endif /* CONFIG_LOGGER_ENABLED */

// TODO: Verificar erro de medidor com saída pulsada ativada em modo verif.
#if 0
    // Disable pulse manger on verif mode
    pulsemng_disable();
#endif

#if CONFIG_VARIABLE_US
    // Update configuration for measurements
    configmng_checkAndUpdate();
#endif

    // Configure power module
    power_init();

#if DEBUG_VERIF || DEBUG_PROBE_VERIF || DEBUG_PROC_FLOWRATE
    power_enableCoreWakeupBySerial();
#if DEBUG || DEBUG_PROBE_VERIF
    probe_init();
    probe_enableDataTransmission();
#endif
#endif

    power_enableCoreWakeupByMeasurementFinished();
    power_enableUsWakeupByRTC();
}

/**
 * @brief
 *      Run verification operating mode.
 */
void verif_run(void)
{
    if(systick_getWakeUpIntMs() != VERIF_UPDATE_INT_MS)
    {
        systick_setWakeUpIntMs(VERIF_UPDATE_INT_MS);
    }

#if CONFIG_OPTCOMM_ENABLED
    // Update optcomm
    optcomm_update();
#endif

    // Switch screen in case user has pressed optical interface
    if(E_USERINT_CMD_CHANGE_SCREEN == userint_getState())
    {
        screenmng_nextScreen();
    }

#if CONFIG_FRAUD_ENABLED
    // Update fraud detection
    fraud_update();
#endif /* CONFIG_FRAUD_ENABLED */

#if CONFIG_LOGGER_ENABLED
    // Update logger
    logger_update();
#endif /* CONFIG_LOGGER_ENABLED */

#if DEBUG || DEBUG_PROBE_VERIF
    probe_update();
#endif
}

/**
 * @brief
 *      Test whether user wants to run the normal operating mode.
 *
 * @return
 *      Returns 'true' if this is the case. Otherwise, returns 'false'.
 */
bool verif_testChangeToNormalOpmode(void)
{
    static uint64_t ull_manualInitTickMs;
    bool b_status = false;

    // Manual operating mode change detection
    switch(ge_manualDetectionState)
    {
        case E_MANUAL_DETECT_STATE_CHECK:

            if((userint_getState() == E_USERINT_CMD_CHANGE_OP_MODE) &&
               (screenmng_getActiveScreenId() == E_SCREEN_NET_VOLUME))
            {
                screenmng_addScreen(&gs_screens[E_SCREEN_BLANK]);
                screenmng_setScreen(E_SCREEN_BLANK);
                ull_manualInitTickMs = systick_getSysTickMs();
                ge_manualDetectionState = E_MANUAL_DETECT_STATE_CONFIRM;
            }
            break;

        case E_MANUAL_DETECT_STATE_CONFIRM:

            if((systick_getSysTickMs() - ull_manualInitTickMs) <
                VERIF_MANUAL_DETECT_TIMEOUT_MS)
            {
                if(screenmng_getActiveScreenId() == E_SCREEN_BLANK)
                {
                    screenmng_setScreen(E_SCREEN_NET_VOLUME);
                }
                else if(screenmng_getActiveScreenId() == E_SCREEN_NET_VOLUME)
                {
                    screenmng_setScreen(E_SCREEN_BLANK);
                }

                if(userint_getState() == E_USERINT_CMD_CHANGE_SCREEN)
                {
                    b_status = true;
                    ge_manualDetectionState = E_MANUAL_DETECT_STATE_CHECK;
                }
            }
            else
            {
                screenmng_deleteScreen(E_SCREEN_BLANK);
                screenmng_setScreen(E_SCREEN_NET_VOLUME);
                ge_manualDetectionState = E_MANUAL_DETECT_STATE_CHECK;
            }
            break;
    }

    // Check verification mode timeout
    if(systick_getSysTickMs() > gull_verifTimeoutTickMs)
    {
        b_status = true;
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
bool verif_testChangeToCommOpmode(void)
{
    return optcomm_isConnected();
}
#endif /* CONFIG_OPTCOMM_ENABLED */
