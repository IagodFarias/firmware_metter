/***************************************************************************//**
 * @brief
 * @file 	pulseout.c
 * @author 	Eric Costa Hall
 * @date 	15/03/2022
 * @details
 * @section license
 * 			FAE Technology. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pulseout.h"
#include "gpio.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Pulse output pin definition. */
#define PULSEOUT_PULSE_OUT_PORT             GPIO_PORT_P7
#define PULSEOUT_PULSE_OUT_PIN              GPIO_PIN4

/** Direction output pin definition. */
#define PULSEOUT_DIR_OUT_PORT               GPIO_PORT_P7
#define PULSEOUT_DIR_OUT_PIN                GPIO_PIN5

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void pulseout_init(void)
{
    // Initialize pulse output GPIO
    GPIO_setAsPeripheralModuleFunctionOutputPin(
            PULSEOUT_PULSE_OUT_PORT,
            PULSEOUT_PULSE_OUT_PIN,
            GPIO_PRIMARY_MODULE_FUNCTION);

    // Initialize dir output GPIO
    GPIO_setAsPeripheralModuleFunctionOutputPin(
            PULSEOUT_DIR_OUT_PORT,
            PULSEOUT_DIR_OUT_PIN,
            GPIO_PRIMARY_MODULE_FUNCTION);

    // Put all pins to down
    GPIO_setOutputLowOnPin(PULSEOUT_PULSE_OUT_PORT, PULSEOUT_PULSE_OUT_PIN);
    GPIO_setOutputLowOnPin(PULSEOUT_DIR_OUT_PORT, PULSEOUT_DIR_OUT_PIN);
}

void pulseout_setPulseOutPin(void)
{
    GPIO_setOutputHighOnPin(PULSEOUT_PULSE_OUT_PORT, PULSEOUT_PULSE_OUT_PIN);
}

void pulseout_clearPulseOutPin(void)
{
    GPIO_setOutputLowOnPin(PULSEOUT_PULSE_OUT_PORT, PULSEOUT_PULSE_OUT_PIN);
}

void pulseout_setDirOutPin(void)
{
    GPIO_setOutputHighOnPin(PULSEOUT_DIR_OUT_PORT, PULSEOUT_DIR_OUT_PIN);
}

void pulseout_clearDirOutPin(void)
{
    GPIO_setOutputLowOnPin(PULSEOUT_DIR_OUT_PORT, PULSEOUT_DIR_OUT_PIN);
}
