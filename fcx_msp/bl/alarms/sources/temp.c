/***************************************************************************//**
 * @brief
 * @file    temp.c
 * @author  Eric Costa Hall
 * @date    04/11/2021
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "temp.h"
#include "processing.h"
#include "configmng.h"

#if DEBUG_TEMP
#include <stdio.h>
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if DEBUG_TEMP
#define TEMP_HIGH_TEMP                              25.00f
#define TEMP_LOW_TEMP                               24.85f
#endif
/*******************************************************************************
 * Enumerations
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static s_alarms_config_t *gps_alarmConfig = NULL;
static bool gb_isAlarm = false;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void temp_init(void)
{
    // Get configuration
    gps_alarmConfig = configmng_getAlarmConfig();
}

void temp_update(void)
{
    bool b_highAlarm = false;
    bool b_lowAlarm = false;
    double_t d_temp = 0.0;

    if(gps_alarmConfig->uc_highTemp || gps_alarmConfig->uc_lowTemp)
    {
        d_temp = proc_getTemperature();
#if DEBUG_TEMP
       printf("(TEMP): Check high low temperature %.2f\n", d_temp);
#endif /* DEBUG_TEMP */
    }
    
    // Check valid temperature
    if(d_temp)
    {
        // Check if alarm is enabled
        if(gps_alarmConfig->uc_highTemp)
        {
#if DEBUG_TEMP
            // Check high temperature
            if(d_temp > TEMP_HIGH_TEMP)
#else
            // Check high temperature
            if(d_temp > (double_t) gps_alarmConfig->uc_highTemp)
#endif
            {
#if DEBUG_TEMP
                printf("(TEMP): High temperature detected.\n");
#endif /* DEBUG_TEMP */

                b_highAlarm = true;
            }
            else
            {
                b_highAlarm = false;
            }
        }

        // Check if alarm is enabled
        if(gps_alarmConfig->uc_lowTemp)
        {
#if DEBUG_TEMP
            // Check high temperature
            if(d_temp < TEMP_LOW_TEMP)
#else
            // Check low temperature
            if(d_temp < (double_t) gps_alarmConfig->uc_lowTemp)
#endif
            {
#if DEBUG_TEMP
                printf("(TEMP): Low temperature detected.\n");
#endif /* DEBUG_TEMP */
                b_lowAlarm = true;
            }
            else
            {
                b_lowAlarm = false;
            }
        }
    }

    // Do an OR operation to get some temperature alarm
    gb_isAlarm = b_highAlarm | b_lowAlarm;
}

bool temp_isOutsideLimit(void)
{
    return gb_isAlarm;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
