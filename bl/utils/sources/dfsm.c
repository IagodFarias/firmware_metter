/***************************************************************************//**
 * @brief
 * @file    dfsm.c
 * @author  Robson Cardoso
 * @date    19/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "dfsm.h"
#include "systick.h"
#include "processing.h"

#if DEBUG_DFSM
#include <stdio.h>
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Zero flow timeout in milliseconds. */
#define DFSM_ZERO_FLOW_TIMEOUT                  60000ULL
/** User interact timeout in milliseconds. */
#define DFSM_USER_INTERACT_TIMEOUT              6000U

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Dynamic frequency scheme enumeration. */
typedef enum E_STATE
{
    /** Detect zero flow state. */
    E_STATE_DETECT_ZERO_FLOW,
    /** Wait for zero flow timeout state. */
    E_STATE_WAIT_ZERO_FLOW_TIMEOUT,
    /** Detect positive flow state. */
    E_STATE_DETECT_POSITIVE_FLOW,
    /** Detect user interact. */
    E_STATE_DETECT_USER_INTERACT,
    /** Check timeout user interact. */
    E_STATE_CHECK_USER_INTERACT_TIMEOUT,
} E_STATE_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/



/*******************************************************************************
 * Global variables
 ******************************************************************************/
/* FSM state. */
static E_STATE_t ge_fsmState;

/* Temporary time stamp. */
static uint64_t gull_timestampTickMs;

/* Low frequency scheme. */
static s_freq_scheme_t gs_lowFreqScheme;

/* High frequency scheme. */
static s_freq_scheme_t gs_highFreqScheme;

/* Interact User frequency scheme. */
static s_freq_scheme_t gs_userFreqScheme;

/* Current active frequency scheme. */
static bool gb_isHighFreqScheme;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize dynamic frequency scaling manager.
 */
void dfsm_init(void)
{
#if DEBUG_DFSM
    printf("%llu (DFSM): Initialize dynamic frequency scaling manager.\n",
        systick_getSysTickMs());
#endif
    gull_timestampTickMs = 0;
    gb_isHighFreqScheme = true;
    ge_fsmState = E_STATE_DETECT_ZERO_FLOW;
}

/**
 * @brief
 *      Set high frequency scheme.
 *
 * @param[in]   s_scheme
 *      Frequency scheme. For more information, please refer to @ref
 *      s_freq_scheme_t.
 */
void dfsm_setHighFreqScheme(s_freq_scheme_t const *const s_scheme)
{
    gs_highFreqScheme.ui_wakeUpIntMs = s_scheme->ui_wakeUpIntMs;
    gs_highFreqScheme.d_samplingRate = s_scheme->d_samplingRate;
}

/**
 * @brief
 *      Set low frequency scheme.
 *
 * @param[in]   s_scheme
 *      Frequency scheme. For more information, please refer to @ref
 *      s_freq_scheme_t.
 */
void dfsm_setLowFreqScheme(s_freq_scheme_t const *const s_scheme)
{
    gs_lowFreqScheme.ui_wakeUpIntMs = s_scheme->ui_wakeUpIntMs;
    gs_lowFreqScheme.d_samplingRate = s_scheme->d_samplingRate;
}

/**
 * @brief
 *      Set user interact frequency scheme.
 *
 * @param[in]   s_scheme
 *      Frequency scheme. For more information, please refer to @ref
 *      s_freq_scheme_t.
 */
void dfsm_setUserInteractFreqScheme(s_freq_scheme_t const *const s_scheme)
{
    gs_userFreqScheme.ui_wakeUpIntMs = s_scheme->ui_wakeUpIntMs;
    gs_userFreqScheme.d_samplingRate = s_scheme->d_samplingRate;
}

/**
 * @brief
 *      Check frequency scheme.
 */
void dfsm_checkFrequency(bool b_userInt, bool b_screenLock)
{
    // Check user interact or timeout state
    if((!b_screenLock) &&
            (b_userInt || (E_STATE_CHECK_USER_INTERACT_TIMEOUT == ge_fsmState)))
    {
        // Change state if user interact
        if(b_userInt)
        {
            ge_fsmState = E_STATE_DETECT_USER_INTERACT;
        }
    }
    else if(gb_isHighFreqScheme)
    {
        if(systick_getWakeUpIntMs() != gs_highFreqScheme.ui_wakeUpIntMs)
        {
            systick_setWakeUpIntMs(gs_highFreqScheme.ui_wakeUpIntMs);
        }

        if(proc_configReadSamplingRate() != gs_highFreqScheme.d_samplingRate)
        {
            proc_configWriteSamplingRate(gs_highFreqScheme.d_samplingRate);
        }

#if DEBUG_DFSM
        printf("%llu (DFSM): High frequency scheme active. Wakeup int: %u ms, Sampling freq: %.1f Hz\n",
            systick_getSysTickMs(), gs_highFreqScheme.ui_wakeUpIntMs,
            gs_highFreqScheme.d_samplingRate);
#endif
    }
    else
    {
        if(systick_getWakeUpIntMs() != gs_lowFreqScheme.ui_wakeUpIntMs)
        {
            systick_setWakeUpIntMs(gs_lowFreqScheme.ui_wakeUpIntMs);
        }

        if(proc_configReadSamplingRate() != gs_lowFreqScheme.d_samplingRate)
        {
            proc_configWriteSamplingRate(gs_lowFreqScheme.d_samplingRate);
        }

#if DEBUG_DFSM
        printf("%llu (DFSM): Low frequency scheme active. Wakeup int: %u ms, Sampling freq: %.1f Hz\n",
            systick_getSysTickMs(), gs_lowFreqScheme.ui_wakeUpIntMs,
            gs_lowFreqScheme.d_samplingRate);
#endif
    }
}

/**
 * @brief
 *      Update dynamic frequency scaling.
 */
void dfsm_update(void)
{
    switch(ge_fsmState)
    {
        case E_STATE_DETECT_ZERO_FLOW:

            if(proc_getCurrentFlowRate() == 0.0)
            {
                gull_timestampTickMs = systick_getSysTickMs();
                ge_fsmState = E_STATE_WAIT_ZERO_FLOW_TIMEOUT;
            }
            break;

        case E_STATE_WAIT_ZERO_FLOW_TIMEOUT:

            if(proc_getCurrentFlowRate() == 0.0)
            {
                if((systick_getSysTickMs() - gull_timestampTickMs) >=
                    DFSM_ZERO_FLOW_TIMEOUT)
                {
                    dfsm_loadFrequencyScheme(gs_lowFreqScheme);
                    gb_isHighFreqScheme = false;
                    ge_fsmState = E_STATE_DETECT_POSITIVE_FLOW;
                }
            }
            else
            {
                ge_fsmState = E_STATE_DETECT_ZERO_FLOW;
            }
            break;

        case E_STATE_DETECT_POSITIVE_FLOW:

            if(proc_getCurrentFlowRate() != 0.0)
            {
                dfsm_loadFrequencyScheme(gs_highFreqScheme);
                gb_isHighFreqScheme = true;
                ge_fsmState = E_STATE_DETECT_ZERO_FLOW;
            }
            break;

        case E_STATE_DETECT_USER_INTERACT:
                gull_timestampTickMs = systick_getSysTickMs();
                ge_fsmState = E_STATE_CHECK_USER_INTERACT_TIMEOUT;
                dfsm_loadFrequencyScheme(gs_userFreqScheme);
            break;

        case E_STATE_CHECK_USER_INTERACT_TIMEOUT:

            if ((systick_getSysTickMs() - gull_timestampTickMs) >=
                    DFSM_USER_INTERACT_TIMEOUT)
            {
                ge_fsmState = E_STATE_DETECT_ZERO_FLOW;
            }
            break;

    }
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/

/**
 * @brief
 *      Load frequency scheme.
 *
 * @param[in]   s_scheme
 *      Frequency scheme. For more information, please refer to
 *      @ref s_freq_scheme_t.
 */
void dfsm_loadFrequencyScheme(s_freq_scheme_t s_scheme)
{
    systick_setWakeUpIntMs(s_scheme.ui_wakeUpIntMs);
    proc_configWriteSamplingRate(s_scheme.d_samplingRate);
}
