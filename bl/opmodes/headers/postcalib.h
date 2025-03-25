/***************************************************************************//**
 * @brief
 * @file    postcalib.h
 * @author  Gustavo SalomÃ£o
 * @date    07/03/2018
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef POSTCALIB_H_
#define POSTCALIB_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void postcalib_init();
void postcalib_run();
bool postcalib_testChangeToStandbyOpMode();

#endif /* POSTCALIB_H_ */
