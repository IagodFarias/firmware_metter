/***************************************************************************//**
 * @file    hw_version.c
 * @brief   This module is responsible to check the version hardware from board.
 * @date    05/10/2021
 * @author  Eric Costa Hall
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "msp430.h"
#include "gpio.h"
#include "hal.h"
#include "hw_version.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define HW_VERSION_VR00_PORT                        GPIO_PORT_P1
#define HW_VERSION_VR00_PIN                         GPIO_PIN1
#define HW_VERSION_VR01_PORT                        GPIO_PORT_PJ
#define HW_VERSION_VR01_PIN                         GPIO_PIN2
#define HW_VERSION_VR02_PORT                        GPIO_PORT_P7
#define HW_VERSION_VR02_PIN                         GPIO_PIN6
#define HW_VERSION_VR03_PORT                        GPIO_PORT_P7
#define HW_VERSION_VR03_PIN                         GPIO_PIN7

#define HW_VERSION_VR00_SHIFT_BIT                   0x00
#define HW_VERSION_VR01_SHIFT_BIT                   0x01
#define HW_VERSION_VR02_SHIFT_BIT                   0x02
#define HW_VERSION_VR03_SHIFT_BIT                   0x03

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void configAsInput(void);
static void saveEnergy(void);

/*******************************************************************************
 * Global variables
 ******************************************************************************/

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void hw_version_init(void)
{
    configAsInput();
    saveEnergy();
}

uint8_t hw_version_get(void)
{
    uint8_t uc_version = 0;

    // Configure all inputs
    configAsInput();

    uc_version |= GPIO_getInputPinValue(HW_VERSION_VR00_PORT,
                    HW_VERSION_VR00_PIN) << HW_VERSION_VR00_SHIFT_BIT;
    uc_version |= GPIO_getInputPinValue(HW_VERSION_VR01_PORT,
                    HW_VERSION_VR01_PIN) << HW_VERSION_VR01_SHIFT_BIT;
    uc_version |= GPIO_getInputPinValue(HW_VERSION_VR02_PORT,
                    HW_VERSION_VR02_PIN) << HW_VERSION_VR02_SHIFT_BIT;
    uc_version |= GPIO_getInputPinValue(HW_VERSION_VR03_PORT,
                    HW_VERSION_VR03_PIN) << HW_VERSION_VR03_SHIFT_BIT;



    // Configure the GPIO's as pull-down or pull-up to reduce energy consumption
    saveEnergy();

    return uc_version;
}

static void configAsInput(void)
{
    // Initialize VR00, VR01, VR02 and VR03 as input
    GPIO_setAsPeripheralModuleFunctionInputPin(HW_VERSION_VR00_PORT,
        HW_VERSION_VR00_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(HW_VERSION_VR01_PORT,
        HW_VERSION_VR01_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(HW_VERSION_VR02_PORT,
        HW_VERSION_VR02_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionInputPin(HW_VERSION_VR03_PORT,
        HW_VERSION_VR03_PIN, GPIO_PRIMARY_MODULE_FUNCTION);
}

static void saveEnergy(void)
{
    if (GPIO_getInputPinValue(HW_VERSION_VR00_PORT, HW_VERSION_VR00_PIN))
    {
        GPIO_setAsInputPinWithPullUpResistor(HW_VERSION_VR00_PORT,
                                             HW_VERSION_VR00_PIN);
    }
    else
    {
        GPIO_setAsInputPinWithPullDownResistor(HW_VERSION_VR00_PORT,
                                               HW_VERSION_VR00_PIN);
    }

    if (GPIO_getInputPinValue(HW_VERSION_VR01_PORT, HW_VERSION_VR01_PIN))
    {
        GPIO_setAsInputPinWithPullUpResistor(HW_VERSION_VR01_PORT,
                                             HW_VERSION_VR01_PIN);
    }
    else
    {
        GPIO_setAsInputPinWithPullDownResistor(HW_VERSION_VR01_PORT,
                                               HW_VERSION_VR01_PIN);
    }

    if (GPIO_getInputPinValue(HW_VERSION_VR02_PORT, HW_VERSION_VR02_PIN))
    {
        GPIO_setAsInputPinWithPullUpResistor(HW_VERSION_VR02_PORT,
                                             HW_VERSION_VR02_PIN);
    }
    else
    {
        GPIO_setAsInputPinWithPullDownResistor(HW_VERSION_VR02_PORT,
                                               HW_VERSION_VR02_PIN);
    }

    if (GPIO_getInputPinValue(HW_VERSION_VR03_PORT, HW_VERSION_VR03_PIN))
    {
        GPIO_setAsInputPinWithPullUpResistor(HW_VERSION_VR03_PORT,
                                             HW_VERSION_VR03_PIN);
    }
    else
    {
        GPIO_setAsInputPinWithPullDownResistor(HW_VERSION_VR03_PORT,
                                               HW_VERSION_VR03_PIN);
    }
}
