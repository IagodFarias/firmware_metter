/***************************************************************************//**
 * @brief
 * @file    verif.h
 * @author  Robson Cardoso
 * @date    03/04/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef VERIF_H_
#define VERIF_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void verif_init(void);
void verif_run(void);
bool verif_testChangeToNormalOpmode(void);
bool verif_testChangeToCommOpmode(void);

#endif /* VERIF_H_ */
