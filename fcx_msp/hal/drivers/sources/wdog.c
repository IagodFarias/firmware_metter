/*******************************************************************************
 * @brief
 * @file    wtd.c
 * @author  Eric Costa Hall
 * @date    11/10/2021
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "wdog.h"
#include "wdt_a.h"
#include "gpio.h"
#include "msp430fr6047.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void wdog_init(void)
{
#ifdef __WATCHDOG_ENABLE__
#if(APPLICATION_ENABLE_ABSTOF_DTOF_OFFSET_CALIBRATION == true)
    // Enable watchdog timer
    // Delay set to ~512K @ ~10K = 51.2sec
    WDT_A_initWatchdogTimer(__MSP430_BASEADDRESS_WDT_A__,
                            WDT_A_CLOCKSOURCE_VLOCLK,
                            WDT_A_CLOCKDIVIDER_512K);
#else
    // Enable watchdog timer
    // Delay set to ~32K @ ~10K = 3.2sec
    WDT_A_initWatchdogTimer(__MSP430_BASEADDRESS_WDT_A__,
                            WDT_A_CLOCKSOURCE_VLOCLK,
                            WDT_A_CLOCKDIVIDER_32K);
#endif
    WDT_A_start(__MSP430_BASEADDRESS_WDT_A__);
#else
    // Stop watchdog timer
    WDT_A_hold(__MSP430_BASEADDRESS_WDT_A__);
#endif
}

void wdog_feed(void)
{
#ifdef __WATCHDOG_ENABLE__
    // Feed watchdog timer
    WDT_A_resetTimer(__MSP430_BASEADDRESS_WDT_A__);
#endif
}
