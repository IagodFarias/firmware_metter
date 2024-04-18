/***************************************************************************//**
 * @brief
 * @file    batt.h
 * @author  Robson Cardoso
 * @date    13/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/
#ifndef BATT_H_
#define BATT_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize battery monitor.
 */
void batt_init(void);

/**
 * @brief
 *      Verify low battery detection.
 */
void batt_update(void);

/**
 * @brief
 *      Check if battery is low.
 *
 * @return
 *      Returns 'true' is battery is low and 'false' otherwise.
 */
bool batt_isLow(void);

/**
 * @brief
 *      Update module with the new configuration.
 */
void batt_updateConfig(void);

#endif /* BATT_H_ */
