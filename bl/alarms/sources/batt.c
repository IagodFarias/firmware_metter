/***************************************************************************//**
 * @brief
 * @file    batt.c
 * @author  Eduardo Balbino
 * @date    14/11/2024
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "batt.h"
#include "anasens.h"
#include "systick.h"
#include "batt.h"
#include "processing.h"
#include "IQmathLib.h"
#if DEBUG_BATT
#include <stdio.h>
#endif
#include "aes_cmac.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Battery detection interval. */
#define BATT_DETECTION_INT_MS                   2000ULL

/** Battery detection threshold. */
#define BATT_DETECTION_THRESHOLD                615 // Equivalent to 2.5V

/*******************************************************************************
 * Enumerations
 ******************************************************************************/

/** Define possibles states. */
typedef enum E_BATT_STATE
{
    /** Idle state. */
    E_BATT_STATE_IDLE,
    /** Detection state. */
    E_BATT_STATE_DETECT,
} E_BATT_STATE_t;

/*******************************************************************************
 * Global variables
 ******************************************************************************/

static bool gb_isBatteryLow = false;
static uint64_t gull_lastTickMs;
static E_BATT_STATE_t gs_state;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize battery monitor.
 */
void batt_init(void)
{
    gb_isBatteryLow = false;
    gull_lastTickMs = systick_getSysTickMs();
    gs_state = E_BATT_STATE_IDLE;
}

/**
 * @brief
 *      Perform low battery detection.
 */
void batt_update(void)
{
    switch(gs_state)
    {
        case E_BATT_STATE_IDLE:

            if(systick_getSysTickMs() >= (gull_lastTickMs + BATT_DETECTION_INT_MS))
            {
                gull_lastTickMs = systick_getSysTickMs();
                gs_state = E_BATT_STATE_DETECT;
            }
            break;

        case E_BATT_STATE_DETECT:

            if(anasens_getBatteryInputVoltage() < BATT_DETECTION_THRESHOLD)
            {
#if DEBUG_BATT
                printf("%llu (BATT): Low battery: %u. thresh: %u\n",
                    systick_getSysTickMs(), anasens_getBatteryInputVoltage(), (uint16_t) BATT_DETECTION_THRESHOLD);
#endif
                gb_isBatteryLow = true;
                proc_disableProcessing();
            }
            else
            {
#if DEBUG_BATT
                printf("%llu (BATT): Battery OK: %u.thresh: %u\n",
                    systick_getSysTickMs(), anasens_getBatteryInputVoltage(), (uint16_t) BATT_DETECTION_THRESHOLD);
#endif
                gb_isBatteryLow = false;
                if(!cmac_isNonVerified()){
                    proc_enableProcessing();
                }
            }

            gs_state = E_BATT_STATE_IDLE;
            break;
    }
}

/**
 * @brief
 *      Check if battery is low.
 *
 * @return
 *      Returns 'true' is battery is low and 'false' otherwise.
 */
bool batt_isLow(void)
{
    return gb_isBatteryLow;
}
