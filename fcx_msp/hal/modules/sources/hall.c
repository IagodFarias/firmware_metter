/***************************************************************************//**
 * @brief
 * @file    hall.c
 * @author  Eric Costa Hall
 * @date    08/11/2021
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "hall.h"
#include "general_settings.h"
#include "gpio.h"

#if DEBUG
#include <stdio.h>
#endif

#if CONFIG_FRAUD_ENABLED
/*******************************************************************************
 * Defines
 ******************************************************************************/
#if 0 // DEBUG
#define HALL_PORT                                       GPIO_PORT_P7
#define HALL_INPUT_PIN                                  GPIO_PIN4
#define HALL_EN_PIN                                     GPIO_PIN5
#else
#define HALL_PORT                                       GPIO_PORT_P1
#define HALL_INPUT_PIN                                  GPIO_PIN4
#define HALL_EN_PIN                                     GPIO_PIN5
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void enableHall();
static void disableHall();
static E_HALL_STATE_t readHallState();

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static E_HALL_STATE_t ge_state = E_HALL_STATE_PRESSED;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize module.
 */
void hall_init()
{
    ge_state = E_HALL_STATE_PRESSED;

    // Configure GPIO's
    GPIO_setAsInputPin(HALL_PORT, HALL_INPUT_PIN);
    GPIO_setAsOutputPin(HALL_PORT, HALL_EN_PIN);
    
    // Disable hall
    disableHall();
}

/**
 * @brief
 *      Update hall's state.
 */
void hall_update()
{
    // Enable hall
    enableHall();

    // Get hall's state
    ge_state = readHallState();

    // Disable hall
    disableHall();
}

/**
 * @brief
 *      Get last hall's state.
 */
E_HALL_STATE_t hall_getState()
{
    return ge_state;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/

/**
 * @brief
 *      Enable hall's circuit.
 */
static void enableHall()
{
    GPIO_setOutputHighOnPin(HALL_PORT, HALL_EN_PIN);
}

/**
 * @brief
 *      Disable hall's circuit.
 */
static void disableHall()
{
    GPIO_setOutputLowOnPin(HALL_PORT, HALL_EN_PIN);
}

/**
 * @brief
 *      Get hall's state.
 *
 * @return
 *      Hall's state.
 */
static E_HALL_STATE_t readHallState()
{
    E_HALL_STATE_t e_state = E_HALL_STATE_PRESSED;
//    printf("Hall: %u\n", GPIO_getInputPinValue(HALL_PORT, HALL_INPUT_PIN));
    if(!GPIO_getInputPinValue(HALL_PORT, HALL_INPUT_PIN))
    {
        e_state = E_HALL_STATE_RELEASED;
    }

    return e_state;
}
#endif /* CONFIG_FRAUD_ENABLED */
