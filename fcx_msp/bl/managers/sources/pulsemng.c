/*******************************************************************************
 * @brief
 * @file    pulsemng.c
 * @author  Eric Costa Hall
 * @date    14/02/2021
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
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
#if FIXED_POINT_CALC
#include "IQmathLib.h"
#endif

#if DEBUG_PULSEMNG
#include <stdio.h>
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Default pulse width in milliseconds. */
#define PULSEMNG_PULSE_WIDTH_MS                 125

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

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static E_PULSEMNG_STATE_t ge_pulseMngState;
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
static bool gb_isPositive;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void pulsemng_init(void)
{
    ge_pulseMngState = E_PULSEMNG_STATE_INACTIVE;
#if FIXED_POINT_CALC
    gl_pulsesPerCubibMeter = (int32_t) pulsemng_evt_getPulsePerMeter();
    gl_pulsesToSend = 0;
#else
    gll_pulseCount = 0;
    gd_pulsesPerCubicMeter = pulsemng_evt_getPulsePerMeter();
#endif

    gb_isEnabled = false;
    gb_isPositive = false;

    // Put pins of pulse and direction to default value
    pulseout_clearPulseOutPin();
    pulseout_clearDirOutPin();
}

void pulsemng_enable(void)
{
    ge_pulseMngState = E_PULSEMNG_STATE_INACTIVE;

#if FIXED_POINT_CALC
    // Reset variables
    gl_pulsesToSend = 0;
    giq28_accumulated = _IQ28(0.0);
#else
    gll_pulseCount =
            (int64_t) (proc_getNetVolume() * gd_pulsesPerCubicMeter);
#endif

    gb_isEnabled = true;
    gb_isPositive = false;
}

void pulsemng_disable(void)
{
    gb_isEnabled = false;
    gb_isPositive = false;
}

void pulsemng_update(void)
{
    if(gb_isEnabled)
    {
        static bool b_pulseToSend = false;

        if(!pulsemng_isActive()) // wasnt wake-upped by pulse to send
            hasPulseToSend();

        switch(ge_pulseMngState)
        {
            case E_PULSEMNG_STATE_INACTIVE:
            {
#if 0 // DEBUG_PULSEMNG
                printf("%u (PULSEMNG): Entering INACTIVE state. ",
                    systick_getSysTickMs());
#endif

#if FIXED_POINT_CALC
                // Check new pulse to send
                if(gl_pulsesToSend != 0)
                {
                    pulsemng_evt_pulseReq(PULSEMNG_PULSE_WIDTH_MS);

                    ge_pulseMngState = E_PULSEMNG_STATE_START_PULSE;
                }
#else
                int64_t ll_netVolume = (int64_t) (proc_getNetVolume() *
                        gd_pulsesPerCubicMeter);

                if((ll_netVolume - gll_pulseCount) > 0)
                {
                    ge_pulseMngState = E_PULSEMNG_STATE_ACTIVE;
                    gb_isPositive = true;
                }
                else if((ll_netVolume - gll_pulseCount) < 0)
                {
                    ge_pulseMngState = E_PULSEMNG_STATE_ACTIVE;
                    gb_isPositive = false;
                }

                if (E_PULSEMNG_STATE_ACTIVE == ge_pulseMngState)
                {
                    pulsemng_evt_pulseReq(PULSEMNG_PULSE_WIDTH_MS);
                }
#endif
            }
            break;
#if FIXED_POINT_CALC
            case E_PULSEMNG_STATE_START_PULSE:
                // Decrease sent pulse
                if(gb_isPositive)
                {
                    gl_pulsesToSend--;
                    pulseout_clearDirOutPin();
                }
                else
                {
                    gl_pulsesToSend++;
                    pulseout_setDirOutPin();
                }

                // Pull up the pulse pin
                pulseout_setPulseOutPin();

                // Check new pulse to send
                b_pulseToSend = !(gl_pulsesToSend == 0);

                ge_pulseMngState = E_PULSEMNG_STATE_STOP_PULSE;
            break;

            case E_PULSEMNG_STATE_STOP_PULSE:
#if DEBUG_PULSEMNG
                printf("%u (PULSEMNG): Entering ACTIVE state. ",
                    systick_getSysTickMs());
#endif
                // Pull down pulse output
                pulseout_clearPulseOutPin();

                // Check pulse again before go to inactive
                if(b_pulseToSend)
                {
                    // Request new pulse
                    pulsemng_evt_pulseReq(PULSEMNG_PULSE_WIDTH_MS);

                    // Update FSM state
                    ge_pulseMngState = E_PULSEMNG_STATE_START_PULSE;
                }
                else
                {
                    // Update FSM state
                    ge_pulseMngState = E_PULSEMNG_STATE_INACTIVE;
                }

                b_pulseToSend = !(gl_pulsesToSend == 0);

                break;
#else
            case E_PULSEMNG_STATE_ACTIVE:
                if(gb_isPositive)
                {
                    gll_pulseCount++;
                    pulseout_clearDirOutPin();
                }
                else
                {
                    gll_pulseCount--;
                    pulseout_setDirOutPin();
                }

                // Pull down the pulse pin
                pulseout_clearPulseOutPin();

#if DEBUG_PULSEMNG
                printf("%llu (PULSEMNG): Entering ACTIVE state. ",
                    systick_getSysTickMs());
#endif

                // Update FSM state
                ge_pulseMngState = E_PULSEMNG_STATE_INACTIVE;
            break;
#endif
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

#if FIXED_POINT_CALC
bool pulsemng_isEndPulse(void)
{
    return E_PULSEMNG_STATE_START_PULSE == ge_pulseMngState;
}

bool pulsemng_isActive(void)
{
    return !(E_PULSEMNG_STATE_INACTIVE == ge_pulseMngState);
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
    if(gl_pulsesToSend > 0)
    {
        gb_isPositive = true;
    }
    else if(gl_pulsesToSend < 0)
    {
        gb_isPositive = false;
    }
    else
    {
        b_pulseToSend = false;
    }

    return b_pulseToSend;
}
#else
bool pulsemng_isActive(void)
{
    return E_PULSEMNG_STATE_ACTIVE == ge_pulseMngState;
}
#endif
