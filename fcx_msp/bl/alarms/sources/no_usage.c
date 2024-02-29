/***************************************************************************//**
 * @brief
 * @file    no_usage.c
 * @author  Eric Costa Hall
 * @date    04/11/2021
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "no_usage.h"
#include "processing.h"
#include "configmng.h"
#include "systick.h"

#if DEBUG_NO_USAGE
#include <stdio.h>
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if DEBUG_NO_USAGE
/** Detection interval in milliseconds. */
#define NO_USAGE_MS_PER_DAY                          1000ULL // 30 days
#else
/** Detection interval in milliseconds. */
#define NO_USAGE_MS_PER_DAY                          86400000ULL // 30 days
#endif

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
typedef enum E_ALARM_NO_USAGE
{
    E_ALARM_NO_USAGE_STATE_0,
    E_ALARM_NO_USAGE_STATE_1,
    E_ALARM_NO_USAGE_STATE_2,
} E_ALARM_NO_USAGE_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static s_alarms_config_t *gps_alarmConfig = NULL;
static E_ALARM_NO_USAGE_t ge_state;
static uint64_t gull_tickAlarmMs;
static double_t gd_lastVol = 0.0;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void no_usage_init(void)
{
    // Get configuration
    gps_alarmConfig = configmng_getAlarmConfig();
}

void no_usage_update(void)
{
    // Check if alarm is enabled
    if(gps_alarmConfig->uc_noUsageDays)
    {
        // Check some flow
        if(proc_getNetVolume() != gd_lastVol)
        {
            ge_state = E_ALARM_NO_USAGE_STATE_0;
        }

        switch(ge_state)
        {
            case E_ALARM_NO_USAGE_STATE_0:
#if DEBUG_NO_USAGE
                printf("(NO_USAGE): Check no flow\n");
                printf("Vol: %.3f, Last Vol: %.3f\n", proc_getNetVolume(),
                    gd_lastVol);
#endif /* DEBUG_NO_USAGE */

                // Check actual volume and last volume
                if(proc_getNetVolume() == gd_lastVol)
                {
                    gull_tickAlarmMs = systick_getSysTickMs();
                    gull_tickAlarmMs += (gps_alarmConfig->uc_noUsageDays *
                        NO_USAGE_MS_PER_DAY);

                    ge_state = E_ALARM_NO_USAGE_STATE_1;
                }
            break;

            case E_ALARM_NO_USAGE_STATE_1:
#if DEBUG_NO_USAGE
                printf("(NO_USAGE): Checking timeout\n");
#endif /* DEBUG_NO_USAGE */
                // Check timeout
                if(systick_getSysTickMs() > gull_tickAlarmMs)
                {
                    ge_state = E_ALARM_NO_USAGE_STATE_2;
                }
            break;

            default:
            case E_ALARM_NO_USAGE_STATE_2:
#if DEBUG_NO_USAGE
                printf("(NO_USAGE): Alarm detected.\n");
#endif /* DEBUG_NO_USAGE */
            break;
        }

        // Update last volume
        gd_lastVol = proc_getNetVolume();
    }
}

bool no_usage_isDetected(void)
{
    return ge_state == E_ALARM_NO_USAGE_STATE_2;
}

void no_usage_updateConfig(void)
{
    // Reset state machine
    ge_state = E_ALARM_NO_USAGE_STATE_0;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
