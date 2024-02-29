/*******************************************************************************
 * @brief   Initializes Hardware Abstraction Layer API.
 * @file    hal.c
 * @author  Robson Cardoso
 * @date    17/05/2016
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#include "hal.h"
#include "serial.h"
#include "display.h"
#include "us.h"
#include "processing.h"
#include "optint.h"
#include "anasens.h"
#include "hw_version.h"
#include "configmng.h"
#include "assert.h"
#include "wdog.h"
#include "clk.h"
#include "systick.h"
#include "power.h"
#include "batt.h"

#include "comm.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
static void checkValidHwFw(void);
static void disablePinsNotUsed(void);

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void hal_init(void)
{
    // Disable interrupt during initialization
    __disable_interrupt();

    // Initialize WATCHDOG
    wdog_init();

    // Configure power module
    power_init();

    // Disable unused pins to save energy
    disablePinsNotUsed();

    // Initialize LFXO
    clk_init();

    // Initialize system tick
    systick_init();

    // Initialize display LCD
    display_init();

    // Initialize hardware version
    hw_version_init();

    // Check if hardware version is valid with firmware version
    checkValidHwFw();

    // Initialize serial module
    serial_init();

    // Initialize ultrassonic module
    us_init();

    // Initialize analog sensors
    anasens_init();

    // Initialize optical interface
    optint_init();

    // Initialize battery monitor
    batt_init();

    // Initialize processing module
    proc_init();

    // Restore interrupt
    __enable_interrupt();
}

static void checkValidHwFw(void)
{
    uint8_t uc_hwVer = hw_version_get();

    // Check invalid version
#if !HARDWARE_EVM
    if(uc_hwVer != CONFIG_HARDWARE_VERSION)
    {
        display_enable();
        display_printString("FAIL. HARD");
        ASSERT(false);
    }
#endif
}

static void disablePinsNotUsed(void)
{

    /* GPIO Configuration
     * All pins initialized as output low, except for:
     *  P1[0:1,4] LED0-2 initialized as output Low (High on EVM 1.0)
     *  P1[5] IRQ (OPEN DRAIN)
     *  P1[6:7]  P1.6: UCB0 I2C SDA & P1.7: UCB0 I2C SCL
     *  P2[2:3] VCC_SENSE, set as input
     *  P2[5:6] BUTTON LEFT-UP initialized as input with internal pull-up
     *  P3[0:2] BUTTON DOWN-SELECT-RIGHT initialized as input with internal pull-up
     *  PJ[4:5] LFXT
     */
    GPIO_setOutputLowOnPin(GPIO_PORT_P1,
                           GPIO_PIN2 | GPIO_PIN3);
#if (__EVM430_VER__==0x10)
        GPIO_setOutputHighOnPin(GPIO_PORT_P1,
                                GPIO_PIN0|GPIO_PIN1|GPIO_PIN4);
#else
    GPIO_setOutputLowOnPin(GPIO_PORT_P1,
                           GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN4);
#endif
    GPIO_setOutputLowOnPin(GPIO_PORT_P2,
                           GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3,
                           GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P4,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P5,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P6,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P7,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P8,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(
            GPIO_PORT_P9,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_PJ,
                           GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN6 | GPIO_PIN7);

    GPIO_setAsOutputPin(GPIO_PORT_P1,
    GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P2,
    GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN4 | GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P3,
                        GPIO_PIN3 | GPIO_PIN4 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P4,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P5,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P6,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P7,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P8,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(
            GPIO_PORT_P9,
            GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN4
                    | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_PJ,
                        GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN6 | GPIO_PIN7);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2,
                                         GPIO_PIN5 | GPIO_PIN6);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P3,
                                         GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);
}
