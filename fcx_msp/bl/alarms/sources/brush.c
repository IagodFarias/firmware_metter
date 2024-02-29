/***************************************************************************//**
 * @brief
 * @file    brush.c
 * @author  Eric Costa Hall
 * @date    25/10/2021
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "brush.h"
#include "processing.h"
#include "ring_buffer.h"
#include "systick.h"
#include "configmng.h"

#if DEBUG_BRUSH
#include <stdio.h>
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if DEBUG
/** Detection interval in milliseconds. */
#define BRUSH_ALARM_DEFAULT_MS                    10000ULL // 10 seconds
#endif

#define BRUSH_PARAM_1                             1.05f // 5%
#define BRUSH_PARAM_2                             1.10f // 10%
#define BRUSH_PARAM_3                             1.20f // 20%

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
typedef enum E_STATE_MACHINE
{
    /** State 1 */
    E_STATE_MACHINE_NO_BRUSH,
    /** State 2 */
    E_STATE_MACHINE_DETECTING,
    /** State 3 */
    E_STATE_MACHINE_BRUSH_DETECTED,
} E_STATE_MACHINE_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void calcFlowRateLimit(void);
static inline double_t getDoubleAbs(double_t d_value);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Keeps track of elapsed time. */
static uint64_t gull_tmtTickMs;

/** The flow rate limit to alarm. */
static double_t gd_flowRateLimit = 0;

static E_STATE_MACHINE_t ge_state = E_STATE_MACHINE_NO_BRUSH;
static s_alarms_config_t *gps_alarmConfig = NULL;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize module.
 */
void brush_init(void)
{
    // Get configuration
    gps_alarmConfig = configmng_getAlarmConfig();

    // Calculate flow rate to alarm
    calcFlowRateLimit();
}

/**
 * @brief
 *      Update detection buffer.
 */
void brush_update(void)
{
    // Check alarm is enabled
    if(gps_alarmConfig->uc_brushPercent)
    {
        switch(ge_state)
        {
            default:
            case E_STATE_MACHINE_NO_BRUSH:
#if DEBUG_BRUSH
                printf(
                        "(brush): Max Overflow E_STATE_MACHINE_1 (Trigger: %f).\n",
                        proc_getCurrentFlowRate());
#endif /* DEBUG_BRUSH */
                if(getDoubleAbs(proc_getCurrentFlowRate()) > gd_flowRateLimit)
                {
                    ge_state = E_STATE_MACHINE_DETECTING;

                    // Calculate the detection timeout
                    gull_tmtTickMs = systick_getSysTickMs();
#if DEBUG
                    // Get timeout from memory
                    gull_tmtTickMs += BRUSH_ALARM_DEFAULT_MS;
#else
                    // Get timeout from memory
                    gull_tmtTickMs += gps_alarmConfig->ul_brushTimeout;
#endif
                }
            break;

            case E_STATE_MACHINE_DETECTING:
#if DEBUG_BRUSH
                printf(
                        "(brush): Max Overflow E_STATE_MACHINE_2 (Trigger: %f).\n",
                        proc_getCurrentFlowRate());
#endif /* DEBUG_BRUSH */

                if(getDoubleAbs(proc_getCurrentFlowRate()) <= gd_flowRateLimit)
                {
                    ge_state = E_STATE_MACHINE_NO_BRUSH;
                }
                else if(systick_getSysTickMs() > gull_tmtTickMs)
                {
                    ge_state = E_STATE_MACHINE_BRUSH_DETECTED;
                }
            break;

            case E_STATE_MACHINE_BRUSH_DETECTED:
#if DEBUG_BRUSH
                printf(
                        "(brush): Max Overflow E_STATE_MACHINE_3 (Trigger: %f).\n",
                        proc_getCurrentFlowRate());
#endif /* DEBUG_BRUSH */

                if(getDoubleAbs(proc_getCurrentFlowRate()) <= gd_flowRateLimit)
                {
                    ge_state = E_STATE_MACHINE_NO_BRUSH;
                }
            break;
        }
    }
}

/**
 * @brief
 *      Check if brush is detected.
 *
 * @return
 *      Returns 'true' is brush is detected. Otherwise, it returns 'false'.
 */
bool brush_isOverflowing(void)
{
    return ge_state == E_STATE_MACHINE_BRUSH_DETECTED;
}

void brush_updateConfig(void)
{
    // Update flow rate limit from new configuration
    calcFlowRateLimit();

    // Reset state machine
    ge_state = E_STATE_MACHINE_NO_BRUSH;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
static void calcFlowRateLimit(void)
{
    // Get Q3 meter from memory in L/h to m³/h
    gd_flowRateLimit = (double_t) gps_alarmConfig->ui_q3Meter;
    gd_flowRateLimit /= 1000.0;

    // Calculate flow rate limit
    gd_flowRateLimit *= 1.0 + gps_alarmConfig->uc_brushPercent; // TODO value in m³/h

    gd_flowRateLimit = 1.0;
}

static inline double_t getDoubleAbs(double_t d_value)
{
    if(d_value < 0)
    {
        d_value = -d_value;
    }

    return d_value;
}

