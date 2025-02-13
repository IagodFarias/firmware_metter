/***************************************************************************//**
 * @brief
 * @file    normal.c
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
#include "normal.h"
#include "types.h"
#include "systick.h"
#include "screenmng.h"
#include "configmng.h"
#include "opmodemng.h"
#include "optcomm.h"
#include "alarmmng.h"
#include "leak.h"
#if CONFIG_FRAUD_ENABLED
#include "fraud.h"
#endif
#include "logger.h"
#include "processing.h"
#include "userint.h"
#include "power.h"
#include "battdate.h"
#include "dfsm.h"
#include "probe.h"
#include "display.h"
#include <brush.h>
#include "rev_install.h"
#include "no_usage.h"
#include "temp.h"
#include "batt.h"
#include "aes_cmac.h"

#if CONFIG_PULSE_ENABLED
#include "pulsemng.h"
#endif
#if CONFIG_WMBUS_ENABLED && CONFIG_OPTCOMM_ENABLED
#include "configmng.h"
#endif

#if DEBUG_NORMAL
#include <stdio.h>
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define SELECTION_CONFIRM_TIMEOUT                       10000ULL

#define SELECTION_HOLDING_TIME_OPMODE_MS                   \
                        USERINT_CMD_DEFAULT_CHANGE_OP_MODE_TIMEOUT_MS

#define SELECTION_HOLDING_TIME_SCREEN_UNLOCKING_MS      20000ULL
#define NORMAL_SCREEN_UNLOCKED_TIMEOUT_MS               120000ULL

/* Number of screens. */
#define NORMAL_NUM_OF_SCREENS                           8

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Operating mode's screen list. */
typedef enum E_SCREEN
{
    /** Net volume screen */
    E_SCREEN_NET_VOLUME  = 0,
    /** Flow rate screen */
    E_SCREEN_FLOW_RATE,
    /** Reverse volume screen */
    E_SCREEN_REV_VOLUME,
    /** Temperature screen */
    E_SCREEN_TEMPERATURE,
    /** Meter's replacement date screen */
    E_SCREEN_REPLACEMENT_DATE,
    /** Display ALARM */
    E_SCREEN_ALARM,
    /** Display HARDWARE FAILURES */
    /*E_SCREEN_HARD_FAILURES,*/
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

typedef struct S_SELECTION_STATE
{
    E_MANUAL_DETECT_STATE_t e_detectState;
    uint8_t uc_screenId;
    uint64_t ull_lastTickMs;
} s_selection_state_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
STATIC INLINE void checkScreenLockingState();
STATIC bool selectionSequenceCompleted(s_selection_state_t *ps_selectionState);
STATIC uint8_t * getAlarmTextAndSetIndicators01();

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
         .uc_doubleResolution = 3
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
        .uc_doubleResolution = 3
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
    /** Alarms screen 01*/
    {
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) getAlarmTextAndSetIndicators01,
        .uc_doubleResolution = 0
    },
    /** Hardware Failures screen 01 */
    /*{
        .e_screenType = E_SCREEN_TYPE_TEXT,
        .pf_getData = (void (*)(void)) getFailuresTextAndSetIndicators01,
        .uc_doubleResolution = 0
    },*/
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

/** High frequency scheme. */
static const s_freq_scheme_t gs_highFreqScheme = {
    .ui_wakeUpIntMs = 1000,
    .d_samplingRate = 1.0
};

/** Low frequency scheme. */
static const s_freq_scheme_t gs_lowFreqScheme = {
    .ui_wakeUpIntMs = 2000,
    .d_samplingRate = 0.5
};

/** User Interact frequency scheme. */
static const s_freq_scheme_t gs_userIntFreqScheme = {
    .ui_wakeUpIntMs = 500,
    .d_samplingRate = 2.0
};

/** Selection state to change operation mode. */
static s_selection_state_t gs_changeModeSelectionState;

/** Selection state to unlock screen. */
static s_selection_state_t gs_unlockScreenSelectionState;

/** Indicate if the selection functionality is locked. As the system
 *  has two selection functionality (change mode and unlock the screen),
 *  when one is used, the another one cannot be used. */
static bool gb_selectionLocked;

/** Screen unlocked timeout in milliseconds. */
static uint64_t gull_screenUnlockedTimeoutMs = 0;

/**  Get application configuration */
static s_app_config_t *gps_config = NULL;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize normal operating mode.
 */
void normal_init(void)
{
    uint8_t uc_screenId;

#if 1//DEBUG_NORMAL
    opmodemng_setActivationFlag(true);
    opmodemng_setCalibrationFlag(true);
#endif

    // Initialize dynamic frequency scaling manager
    dfsm_init();

    // Set low and high frequency schemes
    dfsm_setHighFreqScheme(&gs_highFreqScheme);
    dfsm_setLowFreqScheme(&gs_lowFreqScheme);
    dfsm_setUserInteractFreqScheme(&gs_userIntFreqScheme);

    // Initiates high frequency mode on normal form activation from stand-by
    dfsm_loadFrequencyScheme(gs_highFreqScheme);

    // Reset user interface state to prevent any invalid
    // interaction
    userint_resetState();

    // Initialize screen manager
    screenmng_init();

    // Register event indicators
    screenmng_registerEventIndicators(proc_isReverseFlow,
                                      leak_isLeakage,
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
                                      brush_isOverflowing,
                                      rev_install_isDetected,
                                      no_usage_isDetected,
                                      temp_isOutsideLimit);

    // Add screens
    for(uc_screenId = 0; uc_screenId < NORMAL_NUM_OF_SCREENS; uc_screenId++)
    {
        screenmng_addScreen(&gs_screens[uc_screenId]);
    }

    // Get application configuration
    gps_config = configmng_getAppConfig();

    // Check if the meter was activated
    if(gps_config->b_isActivated)
    {
        // Active the screen locker
        screenmng_lockScreen(true);

        // Force a system wake up time update
        dfsm_checkFrequency(false, true);

        // Set change mode timeout
        userint_setChangeModeTimeoutMs(
            SELECTION_HOLDING_TIME_SCREEN_UNLOCKING_MS);
    }
    else
    {
        // Inactive the screen locker
        screenmng_lockScreen(false);

        // Force a system wake up time update
        dfsm_checkFrequency(false, false);

        // Set change mode timeout
        userint_setChangeModeTimeoutMs(SELECTION_HOLDING_TIME_OPMODE_MS);
    }

    // Enable ultrasonic measurements
    us_enableMeasurements();

    // Enable processing filters
    proc_enableFilters();

    if(!batt_isLow() && !cmac_isNonVerified()){
        // Enable processing
        proc_enableProcessing();
    }

#if CONFIG_OPTCOMM_ENABLED
    // Initialize optical communication
    optcomm_init();
#endif /* CONFIG_OPTCOMM_ENABLED */

#if CONFIG_WMBUS_ENABLED

#if CONFIG_OPTCOMM_ENABLED
    if(configmng_getEnableTx())
    {
         // Enable RF transmission
        wmbus_stack_enableTransmission();
    }
    else
    {
        // Disable RF Transmission
        wmbus_stack_disableTransmission();
    }
#else
    // Enable RF transmission
    wmbus_stack_enableTransmission();

#endif /* CONFIG_OPTCOMM_ENABLED */

#endif /* CONFIG_WMBUS_ENABLED */

#if CONFIG_PULSE_ENABLED
    // Enable pulse manager
    pulsemng_enable();
#endif /* CONFIG_PULSE_ENABLED */

    // Initialize change mode selection
    gs_changeModeSelectionState.e_detectState = E_MANUAL_DETECT_STATE_CHECK;
    gs_changeModeSelectionState.uc_screenId = E_SCREEN_NET_VOLUME;

    // Initialize unlock screen selection
    gs_unlockScreenSelectionState.e_detectState = E_MANUAL_DETECT_STATE_CHECK;
    gs_unlockScreenSelectionState.uc_screenId = E_SCREEN_NET_VOLUME;

    // Unlock the selection functionality
    gb_selectionLocked = false;

#if CONFIG_LOGGER_ENABLED
    // Enable logger
    logger_enable(true);
#endif /* CONFIG_LOGGER_ENABLED */

    // Configure power module
    power_init();

#if DEBUG_SERIAL
    power_enableCoreWakeupBySerial();
#if DEBUG_PROBE_VERIF
    probe_init();
    probe_enableDataTransmission();
#endif
#endif /* DEBUG_NORMAL ||  DEBUG_FLOWRATE || DEBUG_PROBE_NORMAL*/
    power_enableCoreWakeupByMeasurementFinished();
    power_enableUsWakeupByRTC();

#if DEBUG
    proc_resetVolumeCounters();
#endif
}

/**
 * @brief
 *      Run normal operating mode.
 */
void normal_run(void)
{
    bool b_userInt = (E_USERINT_CMD_CHANGE_SCREEN == userint_getState());

    // Check frequency scheme
    dfsm_checkFrequency(b_userInt, screenmng_isScreenLocked());

    // Update frequency scheme
    dfsm_update();

#if CONFIG_OPTCOMM_ENABLED
    // Update optcomm
    optcomm_update();
#endif

    // Switch screen in case user has pressed optical interface
    if(b_userInt)
    {
#if DEBUG_USERINT
        printf("(NORMAL:USERINT): activated:%u, screenlocked:%u\n",gps_config->b_isActivated, screenmng_isScreenLocked());
#endif
        screenmng_nextScreen();
    }

    // Check the screen locking state
    if(gps_config->b_isActivated)
    {
        checkScreenLockingState();
    }

    // Update alarm modules detection
    leak_update();
    brush_update();
    rev_install_update();
    no_usage_update();
    temp_update();
    battdate_update();    

#if CONFIG_FRAUD_ENABLED
    // Update fraud detection
    fraud_update();
#endif /* CONFIG_FRAUD_ENABLED */

#if CONFIG_LOGGER_ENABLED
    // Update logger
    logger_update();
#endif /* CONFIG_LOGGER_ENABLED */

    // Update alarm manager
    alarmmng_update();

#if CONFIG_VARIABLE_US
    // Update configuration for measurements
    configmng_checkAndUpdate();
#endif

#if DEBUG || DEBUG_PROBE_NORMAL
    probe_update();
#endif
}

/**
 * @brief
 *       Test whether user wants to run the verification operating mode.
 *
 * @return
 *      Returns 'true' if this is the case. Otherwise, returns 'false'.
 */
bool normal_testChangeToVerifOpmode(void)
{
    bool b_status = false;

    if(screenmng_isScreenLocked() == false &&
       selectionSequenceCompleted(&gs_changeModeSelectionState))
    {
#if CONFIG_WMBUS_ENABLED
        // Disable RF transmission
        wmbus_stack_disableTransmission();
#endif /* CONFIG_WMBUS_ENABLED */

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
bool normal_testChangeToCommOpmode(void)
{
    return optcomm_isConnected();
}
#endif /* CONFIG_OPTCOMM_ENABLED */

/**
 * @brief
 *      Check screen locking state.
 *
 * @note
 *      If screen is locked and user complete the unlocking sequence,
 *      then unlock the screen.
 *      If screen is unlocked and the unlocked timeout passed, then lock
 *      the screen again.
 */
STATIC void checkScreenLockingState()
{
    if(screenmng_isScreenLocked())
    {
        if(selectionSequenceCompleted(&gs_unlockScreenSelectionState))
        {
            // Unlock screen
            screenmng_lockScreen(false);

            // Set change mode timeout
            userint_setChangeModeTimeoutMs(SELECTION_HOLDING_TIME_OPMODE_MS);

            // Calculate the timeout to lock the screen again
            gull_screenUnlockedTimeoutMs = NORMAL_SCREEN_UNLOCKED_TIMEOUT_MS;
            gull_screenUnlockedTimeoutMs += systick_getSysTickMs();
        }
    }
    else
    {
        if(systick_getSysTickMs() > gull_screenUnlockedTimeoutMs)
        {
            // Lock screen
            screenmng_lockScreen(true);

            // Set change mode timeout
            userint_setChangeModeTimeoutMs(
                SELECTION_HOLDING_TIME_SCREEN_UNLOCKING_MS);
        }
    }
}

/**
 * @brief
 *       Check if the selection sequence has been completed.
 *
 * @param[in]   ps_selectionState
 *      Structure which holds a selection state.
 */
STATIC bool selectionSequenceCompleted(s_selection_state_t *ps_selectionState)
{
    bool b_status = false;

    switch(ps_selectionState->e_detectState)
    {
        case E_MANUAL_DETECT_STATE_CHECK:
            if(!gb_selectionLocked &&
               (E_USERINT_CMD_CHANGE_OP_MODE == userint_getState()) &&
               (ps_selectionState->uc_screenId == screenmng_getActiveScreenId()))
            {
                screenmng_addScreen(&gs_screens[E_SCREEN_BLANK]);
                screenmng_setScreen(E_SCREEN_BLANK);

                ps_selectionState->ull_lastTickMs = systick_getSysTickMs();
                ps_selectionState->e_detectState = E_MANUAL_DETECT_STATE_CONFIRM;
                gb_selectionLocked = true;
            }
            break;

        case E_MANUAL_DETECT_STATE_CONFIRM:

            if(systick_getSysTickMs() <
                (ps_selectionState->ull_lastTickMs + SELECTION_CONFIRM_TIMEOUT))
            {
                if(E_SCREEN_BLANK == screenmng_getActiveScreenId())
                {
                    screenmng_setScreen(ps_selectionState->uc_screenId);
                }
                else if(ps_selectionState->uc_screenId == screenmng_getActiveScreenId())
                {
                    screenmng_setScreen(E_SCREEN_BLANK);
                }

#if DEBUG_OPTINT
                printf("%d\n", userint_getState());
#endif

                if(E_USERINT_CMD_CHANGE_SCREEN == userint_getState())
                {
                    screenmng_deleteScreen(E_SCREEN_BLANK);
                    screenmng_setScreen(ps_selectionState->uc_screenId);

                    // Reset user interface state to prevent any invalid
                    // interaction
                    userint_resetState();

                    ps_selectionState->e_detectState = E_MANUAL_DETECT_STATE_CHECK;
                    gb_selectionLocked = false;
                    b_status = true;
                }
            }
            else
            {
                screenmng_deleteScreen(E_SCREEN_BLANK);
                screenmng_setScreen(ps_selectionState->uc_screenId);

                ps_selectionState->e_detectState = E_MANUAL_DETECT_STATE_CHECK;
                gb_selectionLocked = false;
            }
            break;
    }

    return b_status;
}

/**
 *  @brief 
 *      Get alarm text to be shown in dispay and set alarm indicators.
 *
 *  @return
 *      Text to be shown in display.
 */
STATIC uint8_t * getAlarmTextAndSetIndicators01()
{
    static uint8_t auc_msg[] = "A.--------";

    // Check reverse flow
    if(alarmmng_isReverseFlow())
    {
        auc_msg[2] = '1';
    }
    else
    {
        auc_msg[2] = '-';
    }

    // Check leakage
    if(alarmmng_isLeakage())
    {
        auc_msg[3] = '2';
    }
    else
    {
        auc_msg[3] = '-';
    }

    // Check air in pipe
    if(alarmmng_isAirInPipe())
    {
        auc_msg[4] = '3';
    }
    else
    {
        auc_msg[4] = '-';
    }

    // Check battery
    if(battdate_isOld())
    {
        auc_msg[5] = '4';
    }
    else
    {
        auc_msg[5] = '-';
    }
#if CONFIG_FRAUD_ENABLED
    // Check Tamper
    if(fraud_wasViolated())
    {
        auc_msg[6] = '5';
    }
    else
    {
        auc_msg[6] = '-';
    }
#endif

    // Check maximum overflow alarm
    if(alarmmng_isBrush())
    {
        auc_msg[7] = '6';
    }
    else
    {
        auc_msg[7] = '-';
    }

    // Check no usage
    if(alarmmng_isNoUsage())
    {
        auc_msg[8] = '7';
    }
    else
    {
        auc_msg[8] = '-';
    }

    if(alarmmng_isOverTemp())
    {
        display_setIcon(E_ICON_ERROR);
        auc_msg[9] = '8';
    }
    else
    {
        auc_msg[9] = '-';
    }

    return auc_msg;
}

/**
 *  @brief
 *      Get failure text to be shown in display and set hardware failure indicators.
 *
 *  @return
 *      Text to be shown in display.
 */
uint8_t * getFailuresTextAndSetIndicators01()
{
    static uint8_t auc_msg[] = "F1.-------";

    // Check temperature outside alarm
    if(batt_isLow())
    {
        display_setIcon(E_ICON_ERROR);
        auc_msg[3] = '1';
    }
    else
    {
        auc_msg[3] = '-';
    }

    return auc_msg;
}


