/***************************************************************************//**
 * @brief
 * @file    battdate.h
 * @author  Eduardo Balbino
 * @date    14/11/2024
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/
#ifndef BATTDATE_H_
#define BATTDATE_H_

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
void battdate_init(void);

/**
 * @brief
 *      Verify low battery detection.
 */
void battdate_update(void);

/**
 * @brief
 *      Check if battery is low.
 *
 * @return
 *      Returns 'true' is battery is low and 'false' otherwise.
 */
bool battdate_isOld(void);

/**
 * @brief
 *      Update module with the new configuration.
 */
void battdate_updateConfig(void);

#endif /* battdate_H_ */
