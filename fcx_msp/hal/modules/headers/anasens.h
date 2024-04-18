/***************************************************************************//**
 * @brief
 * @file    anasens.h
 * @author  Robson Cardoso
 * @date    17/02/2017
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef ANASENS_H_
#define ANASENS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"
#include "gpio.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void anasens_init(void);
void anasens_startConversion(void);
void anasens_update(void);
void anasens_enableBatteryInput(void);
void anasens_disableBatteryInput(void);

uint32_t anasens_getOpticalInputVoltage(void);
uint16_t anasens_getBatteryInputVoltage(void);

/*******************************************************************************
 * Inline functions
 ******************************************************************************/
#endif /* ANASENS_H_ */
