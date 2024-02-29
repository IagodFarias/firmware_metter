/***************************************************************************//**
 * @brief
 * @file    normal.h
 * @author  Robson Cardoso
 * @date    03/04/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef NORMAL_H_
#define NORMAL_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void normal_init(void);
void normal_run(void);
bool normal_testChangeToVerifOpmode(void);
bool normal_testChangeToCommOpmode(void);

#endif /* NORMAL_H_ */
