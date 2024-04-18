/*******************************************************************************
 * @brief
 * @file    pulsemng.c
 * @author  Eric Costa Hall
 * @date    14/02/2021
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "pulsemng.h"
#include "processing.h"
#include "us.h"
#include "power.h"
#include "systick.h"
#include "pulseout.h"
#include "configmng.h"
#include "timeout.h"
#if FIXED_POINT_CALC
#include "IQmathLib.h"
#endif

#if DEBUG_PULSEMNG
#include <stdio.h>
#include "display.h"
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Default pulse width in milliseconds. */
#define PULSEMNG_PULSE_WIDTH_MS                         125

/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef enum E_PULSEMNG_STATE
{
#if FIXED_POINT_CALC
    E_PULSEMNG_STATE_STOP_PULSE,
    E_PULSEMNG_STATE_START_PULSE,
#else
    E_PULSEMNG_STATE_ACTIVE,
#endif
    E_PULSEMNG_STATE_INACTIVE,
} E_PULSEMNG_STATE_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if FIXED_POINT_CALC
static bool hasPulseToSend(void);
#endif
static void setTimeoutPulseOut();
static bool timeoutPulseHighCallback(void);
static bool timeoutPulseLowCallback(void);


/*******************************************************************************
 * Global variables
 ******************************************************************************/
static bool ge_pulseMngState;
#if FIXED_POINT_CALC
/* Maximum value 1023.0 to IQ21, therefore the mininum value is 1L/s */
int32_t gl_pulsesPerCubibMeter;
static int32_t gl_pulsesToSend = 0;
static int32_t giq28_accumulated = _IQ28(0.0);
#else
static int64_t gll_pulseCount;
static double_t gd_pulsesPerCubicMeter;
#endif

static bool gb_isEnabled;

static tFunctionTimer g_timeoutPulseOut;

#define PULSEOUT_DURATION                                4096

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void pulsemng_init(void)
{

    g_timeoutPulseOut.pbFunction_A = &timeoutPulseHighCallback;
    g_timeoutPulseOut.pbFunction_B = &timeoutPulseLowCallback;
    g_timeoutPulseOut.ui16FunctionDelay_A = PULSEOUT_DURATION;
    g_timeoutPulseOut.ui16FunctionDelay_B = PULSEOUT_DURATION;

#if FIXED_POINT_CALC
    gl_pulsesPerCubibMeter = (int32_t) pulsemng_evt_getPulsePerMeter();
    gl_pulsesToSend = 0;
#else
    gll_pulseCount = 0;
    gd_pulsesPerCubicMeter = pulsemng_evt_getPulsePerMeter();
#endif

    gb_isEnabled = false;

    // Put pins of pulse and direction to default value
    pulseout_clearPulseOutPin();
    pulseout_clearDirOutPin();
}

void pulsemng_enable(void)
{
#if FIXED_POINT_CALC
    // Reset variables
    gl_pulsesToSend = 0;
    giq28_accumulated = _IQ28(0.0);
#else
    gll_pulseCount =
            (int64_t) (proc_getNetVolume() * gd_pulsesPerCubicMeter);
#endif

    gb_isEnabled = true;
}

void pulsemng_disable(void)
{
    gb_isEnabled = false;
}

void pulsemng_update(void)
{
    if(gb_isEnabled)
    {

        if(hasPulseToSend())
        {
            ge_pulseMngState = true;
            setTimeoutPulseOut();
        }
        else
        {
            ge_pulseMngState = false;
        }


#if DEBUG_PULSEMNG
        printf("PulseCount: %u\n", gl_pulsesToSend);
#endif
    }
}

void pulsemng_configWritePulseConstant(double_t volatile d_pulseConstant)
{
#if FIXED_POINT_CALC
    gl_pulsesPerCubibMeter = (int32_t) d_pulseConstant;
#else
    gd_pulsesPerCubicMeter = d_pulseConstant;
#endif
}

static bool hasPulseToSend(void)
{
    bool b_pulseToSend = true;
    int32_t iq28_volume;
    int32_t iq28_aux = 0;


    /* Get diff between last volume in fixed point
       For a flow rate maximum of 8000 L/h, should generate a flow rate of
       0.0022 m³/s. This number multiplier by 1000 pulses/m³ makes a variable
       of IQ28 sufficient to take care of multiplier, in a range of
       -8 to 7.99. The result 2.22 say how pulses should be send by
       the interface. */
    iq28_volume = ((_iq28) proc_getVolFixedPoint()) >> 2; //(_iq28)
    iq28_volume = _IQ28mpyI32(iq28_volume, gl_pulsesPerCubibMeter);

    // Accumulate the volume so we can calculate the appropriate pulses
    giq28_accumulated += iq28_volume;

    // Get integer part that represents the number of pulses to send
    iq28_aux = _IQ28int(giq28_accumulated); // abuse of variable to save memory
    gl_pulsesToSend += iq28_aux;

    // Remove pulses already counted
    giq28_accumulated -= _IQ28(iq28_aux);

    // Check if there are negative pulses to send
    if(gl_pulsesToSend == 0)
    {
        b_pulseToSend = false;
    }

    return b_pulseToSend;
}

static void setTimeoutPulseOut(void)
{
    // Start timeout timer
    Timer_startDelayedFunctionTimer(&g_timeoutPulseOut);
    Timer_scheduleDelayedFunction(eTimerDelayedFunction_A); //timeoutPulseHighCallback
}

bool pulsemng_isEndPulse(void)
{
    return ge_pulseMngState;
}

static bool timeoutPulseHighCallback(void)
{
    if(gl_pulsesToSend > 0)
    {
        gl_pulsesToSend--;
        pulseout_clearDirOutPin();
    }
    else if(gl_pulsesToSend < 0)
    {
        gl_pulsesToSend++;
        pulseout_setDirOutPin();
    }
    else
    {
        return false;
    }

    // Pull up the pulse pin
    pulseout_setPulseOutPin();

    // Start timeout timer
    Timer_startDelayedFunctionTimer(&g_timeoutPulseOut); // needed?
    Timer_scheduleDelayedFunction(eTimerDelayedFunction_B); //timeoutPulseLowCallback

    return false;
}

static bool timeoutPulseLowCallback(void)
{
    if(gl_pulsesToSend != 0)
    {
        // Start timeout timer
        Timer_startDelayedFunctionTimer(&g_timeoutPulseOut); // Needed?
        Timer_scheduleDelayedFunction(eTimerDelayedFunction_A); //timeoutPulseHighCallback
    }

    // Pull down the pulse pin
    pulseout_clearPulseOutPin();

    return false;
}
