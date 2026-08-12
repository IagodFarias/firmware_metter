/***************************************************************************//**
 * @brief
 * @file    power.c
 * @author  Robson Cardoso
 * @date    14/02/2017
 * @details
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "power.h"
#include "types.h"
#include "wdog.h"
#include "msp430fr6047.h"
#include "systick.h"
#include "ussSwLib.h"
//#include "USSLibGUIApp.h"
#include "USS_userConfig.h"
//#include "hal.h"
#include "pmm.h"
#include "serial.h"

#if DEBUG_POWER || DEBUG_WDOG
#include <stdio.h>
#endif

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
typedef enum
{
    POWER_WAKEUP_BY_SERIAL = 1 << 0,

} e_power_wakeup_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void checkWakeUp(void);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Core wake up configuration. */
static e_power_wakeup_t ge_wakeupConfig;

/** Selects which LPM level power_enterSleep() uses: false = LPM0 (UART1
 * clock stays alive), true = LPM3 (deeper sleep, UART1 clock gated). */
static bool gb_lpm3Enabled;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize power module.
 */
void power_init(void)
{
    __enable_interrupt();

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PMM_unlockLPM5();

#ifndef __SVSH_ENABLE__
    PMM_disableSVSH();
#else
    PMM_enableSVSH();
#endif

    ge_wakeupConfig = (e_power_wakeup_t) 0;
    gb_lpm3Enabled = false;
}

/**
 * @brief
 *      Get system wake up reason.
 *
 * @return
 *      Returns the wake up reason. For more details, please refer to the
 *      enumeration type definition @ref E_WAKEUP_REASON_t.
 *
 * @note
 *      If more than one wake up event is triggered, returns the wake up reason
 *      according to the following priority sequence:
 *
 *      1. Watch dog timeout
 *      2. First power up;
 *      3. RTC trigger;
 *      4. Measurement finished;
 *      5. Command received.
 */
E_WAKEUP_REASON_t power_getWakeUpReason(void)
{
    E_WAKEUP_REASON_t e_wakeUpReason = E_WAKEUP_UNKNOWN_REASON;
    uint16_t reset_source = SYSRSTIV;

    // Reset cause register
    SYSRSTIV = 0x00;

    switch (__even_in_range(SYSRSTIV, SYSRSTIV_WDTTO))
    {
        case SYSRSTIV_NONE:
            e_wakeUpReason = E_WAKEUP_REASON_NO_OP;
#ifdef DEBUG_POWER
            printf("System No Op.\n");
#endif
        break;
        case SYSRSTIV_RSTNMI:
            e_wakeUpReason = E_WAKEUP_EXTERNAL_RESET;
#ifdef DEBUG_POWER
            printf("System External Reset.\n");
#endif
        break;
        case SYSRSTIV_DOPOR:
            e_wakeUpReason = E_WAKEUP_REASON_POWER_ON_RESET;
#ifdef DEBUG_POWER
            printf("System Power On Reset.\n");
#endif
        break;
        case SYSRSTIV_WDTTO:
            e_wakeUpReason = E_WAKEUP_REASON_SYSTEM_FAILURE;
#ifdef DEBUG_POWER
            printf("System WTD Reset.\n");
#endif
        break;
        default:
            e_wakeUpReason = E_WAKEUP_UNKNOWN_REASON;
#ifdef DEBUG_POWER
            printf("System Unknown reset.\n");
#endif
            reset_source = 0x00;
        break;
    }

    return e_wakeUpReason;
}

/**
 * @brief
 *      Enable core wakeup by RTC module.
 */
void power_enableCoreWakeupByRTC(void)
{
}

/**
 * @brief
 *      Disable core wakeup by RTC module.
 */
void power_disableCoreWakeupByRTC(void)
{

}

/**
 * @brief
 *      Enable core wakeup by serial module.
 */
void power_enableCoreWakeupBySerial(void)
{
    ge_wakeupConfig |= POWER_WAKEUP_BY_SERIAL;
}

/**
 * @brief
 *      Disable core wakeup by serial module.
 */
void power_disableCoreWakeupBySerial(void)
{
    ge_wakeupConfig &= ~POWER_WAKEUP_BY_SERIAL;
}

/**
 * @brief
 *      Enable core wakeup by measurement finished.
 */
void power_enableCoreWakeupByMeasurementFinished(void)
{
}

/**
 * @brief
 *      Disable core wakeup by measurement finished.
 */
void power_disableCoreWakeupByMeasurementFinished(void)
{
}

/**
 * @brief
 *      Enable ultrasonic measurements triggered by RTC module.
 */
void power_enableUsWakeupByRTC(void)
{
}

/**
 * @brief
 *      Disable ultrasonic measurements triggered by RTC module.
 */
void power_disableUsWakeupByRTC(void)
{

}

void power_enableLpm3(void)
{
    gb_lpm3Enabled = true;
}

void power_disableLpm3(void)
{
    gb_lpm3Enabled = false;
}

bool power_isLpm3Enabled(void)
{
    return gb_lpm3Enabled;
}

/**
 * @brief
 *      Put core in sleep mode.
 */
void power_enterSleep(void)
{
    checkWakeUp();

#ifdef __WATCHDOG_ENABLE__
        wdog_feed();
#if DEBUG_WDOG
        printf("%llu (WDOG): Watch dog timer is running. Time left: %lu\n",
            systick_getSysTickMs(), wdog_get_time_last_left(WDOG0L));
#endif
#else
#if DEBUG_WDOG
    printf("%llu (WDOG): Watch dog timer is running. Time left: %lu\n",
        systick_getSysTickMs(), wdog_get_time_last_left(WDOG0L));
#endif
    printf("%llu (WDOG): Watch dog timer is not running.\n",
                systick_getSysTickMs());
#endif

#ifdef DEBUG_POWER
    printf("%llu (POWER): Entering sleep mode.\n", systick_getSysTickMs());
#endif

    if(gb_lpm3Enabled)
    {
        /* Wait until timer elapses in LPM3  */
        USS_waitForAppTimerElapse(&gUssSWConfig,
                                     USS_low_power_mode_option_low_power_mode_3);
    }
    else
    {
        /* Wait until timer elapses in LPM0  */
        USS_waitForAppTimerElapse(
                &gUssSWConfig, USS_low_power_mode_option_low_power_mode_0);
    }
}

static void checkWakeUp(void)
{
    if(ge_wakeupConfig & POWER_WAKEUP_BY_SERIAL)
    {
        serial_enable();
    }
    else
    {
        serial_disable();
    }
}
