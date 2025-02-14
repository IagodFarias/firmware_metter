/***************************************************************************//**
 * @brief
 * @file    rev_install.c
 * @author  Eric Costa Hall
 * @date    03/11/2021
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rev_install.h"
#include "processing.h"
#include "configmng.h"

#if DEBUG_REV_INSTALL
#include <stdio.h>
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
typedef enum E_ALARM_REV_INSTALL
{
    E_ALARM_REV_INSTALL_STATE_0,
    E_ALARM_REV_INSTALL_STATE_1,
    E_ALARM_REV_INSTALL_STATE_2,
} E_ALARM_REV_INSTALL_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void setParamAlarm(void);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static s_alarms_config_t *gps_alarmConfig = NULL;
static E_ALARM_REV_INSTALL_t ge_state;
static double_t gd_volThreshold = 0.0;
static double_t gd_negVolThreshold = 0.0;
static double_t gd_volToStopAlarm = 0.0;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void rev_install_init(void)
{
    // Get configuration
    gps_alarmConfig = configmng_getAlarmConfig();

    // Update parameter from alarm
    setParamAlarm();
}

void rev_install_update(void)
{
    // Check if alarm is enabled
    if(gps_alarmConfig->ui_revInstallParam)
    {
        switch(ge_state)
        {
            case E_ALARM_REV_INSTALL_STATE_0:
#if DEBUG_REV_INSTALL
                printf("(REV_INSTALL): Check initial state\n");
                printf("For Vol: %.3f, Rev Vol: %.3f\n", proc_getForwardVolume(),
                        proc_getReverseVolume());
#endif /* DEBUG_REV_INSTALL */

                // Check initial value and l
                if(proc_getForwardVolume() < gd_volThreshold)
                {
                    if(proc_getReverseVolume() < gd_negVolThreshold)
                    {
                        ge_state = E_ALARM_REV_INSTALL_STATE_1;

                        // Set forward volume to stop alarm
                        gd_volToStopAlarm = proc_getForwardVolume() +
                                gd_volThreshold;
                    }
                }
                else
                {
                    ge_state = E_ALARM_REV_INSTALL_STATE_2;
                }
            break;

            case E_ALARM_REV_INSTALL_STATE_1:
#if DEBUG_REV_INSTALL
                printf("(REV_INSTALL): Alarm occurrence\n");
                printf("For Vol: %.3f, Stop Vol: %.3f\n",
                        proc_getForwardVolume(), gd_volToStopAlarm);
#endif /* DEBUG_REV_INSTALL */
                if(proc_getForwardVolume() >= gd_volToStopAlarm)
                {
                    ge_state = E_ALARM_REV_INSTALL_STATE_2;
                }
            break;

            default:
            case E_ALARM_REV_INSTALL_STATE_2:
#if DEBUG_REV_INSTALL
                printf("(REV_INSTALL): No alarm anymore.\n");
#endif /* DEBUG_REV_INSTALL */
            break;
        }
    }
}

bool rev_install_isDetected(void)
{
    return ge_state == E_ALARM_REV_INSTALL_STATE_1;
}

void rev_install_updateConfig(void)
{
    // Get parameter from configuration
    setParamAlarm();

    // Reset state machine
    ge_state = E_ALARM_REV_INSTALL_STATE_0;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
static void setParamAlarm(void)
{
    // Calculate the param of alarm
    gd_volThreshold = (double_t) gps_alarmConfig->ui_revInstallParam / 1000.0;

    // Get negative parameter
    gd_negVolThreshold = -gd_volThreshold;
}
