/***************************************************************************//**
 * @brief
 * @file    standby.h
 * @author  Robson Cardoso
 * @date    03/04/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/
#ifndef STANDBY_H_
#define STANDBY_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void standby_init(void);
void standby_run(void);
bool standby_testChangeToNormalOpMode(void);
bool standby_testChangeToCommOpmode(void);

#endif /* STANDBY_H_ */
