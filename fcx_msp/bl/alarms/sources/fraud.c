/***************************************************************************//**
 * @brief
 * @file    fraud.c
 * @author  Eric Costa Hall
 * @date    08/11/2021
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"

#if CONFIG_FRAUD_ENABLED
#include "fraud.h"
#include "hall.h"

#if DEBUG_FRAUD
#include <stdio.h>
#endif /* DEBUG_FRAUD */

#if defined(__TI_COMPILER_VERSION__)
#pragma PERSISTENT(gul_isActivated)
#pragma PERSISTENT(gul_wasViolated)
#define __persistent
#elif defined (__IAR_SYSTEMS_ICC__)
#else
#error Compiler not supported!
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FRAUD_FLAG_CLEARED                              0xFFFFFFFF
#define FRAUD_FLAG_SET                                  0xAAAAAAAA

typedef enum E_FRAUD_STATE
{
    E_FRAUD_STATE_NOT_ACTIVED = 0,
    E_FRAUD_STATE_ACTIVED,
    E_FRAUD_STATE_VIOLATED

} E_FRAUD_STATE_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void markAsActived();
static inline bool isOpened();
static void markAsViolated();

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Indicates if the fraud detector is activated. */
__persistent static uint32_t gul_isActivated = FRAUD_FLAG_CLEARED;

/** Indicates fraud was detected. */
__persistent static uint32_t gul_wasViolated = FRAUD_FLAG_CLEARED;

/** Hold the taper state. */
static E_FRAUD_STATE_t ge_state;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize module.
 */
void fraud_init(void)
{
    hall_init();

    if(FRAUD_FLAG_SET == gul_wasViolated)
    {
        ge_state = E_FRAUD_STATE_VIOLATED;
    }
    else if(FRAUD_FLAG_SET == gul_isActivated)
    {
        ge_state = E_FRAUD_STATE_ACTIVED;
    }
    else
    {
        ge_state = E_FRAUD_STATE_NOT_ACTIVED;
    }

#if DEBUG_FRAUD
    printf("(FRAUD): State: %d.\n", ge_state);
#endif /* DEBUG_FRAUD */
}

/**
 * @brief
 *      Update detection.
 */
void fraud_update(void)
{


#if DEBUG_FRAUD
    ge_state = E_FRAUD_STATE_NOT_ACTIVED
#endif

    if(ge_state > E_FRAUD_STATE_NOT_ACTIVED)
    {
        if((E_FRAUD_STATE_VIOLATED != ge_state) && isOpened())
        {
            markAsViolated();
        }

#if DEBUG_FRAUD
        if(E_FRAUD_STATE_VIOLATED == ge_state)
        {
            printf("(FRAUD): Fraud detected.\n");
        }
        else
        {
            printf("(FRAUD): No violation.\n");
        }
#endif /* DEBUG_FRAUD */

    }
    else
    {
        if(!isOpened())
        {
            markAsActived();
        }

#if DEBUG_FRAUD
        if(E_FRAUD_STATE_ACTIVED == ge_state)
        {
            printf("(FRAUD): Fraud actived.\n");
        }
        else
        {
            printf("(FRAUD): Fraud not actived.\n");
        }
#endif /* DEBUG_FRAUD */

    }
}

/**
 * @brief
 *      Check if the meter was violated.
 *
 * @return
 *      Returns 'true' if it was violated. Otherwise, it returns 'false'.
 */
bool fraud_wasViolated(void)
{
    return E_FRAUD_STATE_VIOLATED == ge_state? true : false;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
/**
 * @brief
 *      Mark flag as activated.
 */
static void markAsActived()
{
    gul_isActivated = FRAUD_FLAG_SET;

    ge_state = E_FRAUD_STATE_ACTIVED;
}

/**
 * @brief
 *      Check if the meter is opened.
 *
 * @return
 *      Returns 'true' if it is opened. Otherwise, it returns 'false'.
 */
static inline bool isOpened()
{
    hall_update();

    return (E_HALL_STATE_RELEASED == hall_getState())? true : false;
}

/**
 * @brief
 *      Mark flag as violated.
 */
static void markAsViolated()
{
    const uint32_t ul_magicValue = FRAUD_FLAG_SET;

    gul_wasViolated = ul_magicValue;

    ge_state = E_FRAUD_STATE_VIOLATED;
}

#endif /* CONFIG_FRAUD_ENABLED */
