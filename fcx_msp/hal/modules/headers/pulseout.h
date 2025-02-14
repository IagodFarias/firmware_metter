/***************************************************************************//**
 * @brief
 * @file 	pulseout.h
 * @author 	Eric Costa Hall
 * @date 	14/09/2021
 * @details
 * @section license
 * 			FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/
#ifndef PULSEOUT_H_
#define PULSEOUT_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "gpio.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize pulse output module.
 */
void pulseout_init(void);

/**
 * @brief
 *      Set pulse output pin.
 */
void pulseout_setPulseOutPin(void);

/**
 * @brief
 *      Clear pulse output pin.
 */
void pulseout_clearPulseOutPin(void);

/**
 * @brief
 *      Set direction output pin.
 */
void pulseout_setDirOutPin(void);

/**
 * @brief
 *      Clear direction output pin.
 */
void pulseout_clearDirOutPin(void);

#endif /* PULSEOUT_H_ */
