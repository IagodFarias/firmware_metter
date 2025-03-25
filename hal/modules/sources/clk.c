/*******************************************************************************
 * @brief
 * @file    clk.c
 * @author  Eric Costa Hall
 * @date    11/10/2021
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "clk.h"
#include "cs.h"
#include "gpio.h"
#include "hal.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define CLK_LFXO_GPIO_PORT                              GPIO_PORT_PJ
#define CLK_LFXO_GPIO_PIN0                              GPIO_PIN4
#define CLK_LFXO_GPIO_PIN1                              GPIO_PIN5

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize module.
 */
void clk_init(void)
{
    /* Configure LFXT GPIO pins */
    GPIO_setAsPeripheralModuleFunctionInputPin(CLK_LFXO_GPIO_PORT,
                                               CLK_LFXO_GPIO_PIN0 +
                                               CLK_LFXO_GPIO_PIN1,
                                               GPIO_PRIMARY_MODULE_FUNCTION);

#if (HAL_SYS_MCLK_FREQ_HZ > 8000000)
    //Set wait state to 1
    FRAMCtl_A_configureWaitStateControl(FRAMCTL_A_ACCESS_TIME_CYCLES_1);
#endif

#if (HAL_SYS_DCO_FREQ_HZ == 8000000)
    // Set DCO frequency to default 8MHz
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_6);
#elif (HAL_SYS_DCO_FREQ_HZ == 16000000)
    // Set DCO frequency to default 16MHz
    CS_setDCOFreq(CS_DCORSEL_1, CS_DCOFSEL_4);
#else
#error "Clock configuration not supported"
#endif

#if (HAL_SYS_LFXT_FREQ_HZ == 32768)
    //Set external clock frequency to 32.768 KHz
    CS_setExternalClockSource(32768,0);
#else
#error "Clock configuration not supported"
#endif

#if (HAL_SYS_MCLK_FREQ_HZ == (HAL_SYS_DCO_FREQ_HZ))
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
#elif (HAL_SYS_MCLK_FREQ_HZ == (HAL_SYS_DCO_FREQ_HZ/2))
    CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
#else
    #error "Clock configuration not supported"
#endif
#if (HAL_SYS_SMCLK_FREQ_HZ == (HAL_SYS_DCO_FREQ_HZ))
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
#elif (HAL_SYS_SMCLK_FREQ_HZ == (HAL_SYS_DCO_FREQ_HZ/2))
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_2);
#else
    #error "Clock configuration not supported"
#endif

#if (HAL_SYS_ACLK_FREQ_HZ == (HAL_SYS_LFXT_FREQ_HZ))
    CS_initClockSignal(CS_ACLK, CS_LFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);
#else
    #error "Clock configuration not supported"
#endif

    // Intializes the XT1 crystal oscillator
    CS_turnOnLFXT (CS_LFXT_DRIVE_3);
}
