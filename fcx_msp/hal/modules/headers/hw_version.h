/***************************************************************************//**
 * @file    hw_version.h
 * @brief   This module a interface to get the hardware version from board
 *          by digital input pins. Every GPIO represent a bit. So, the board
 *          can select version's value between 1 to 2^4.
 * @date    05/10/2021
 * @author  Eric Costa Hall
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/
#ifndef HW_VERSION_H_
#define HW_VERSION_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "general_settings.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Enumerations
 ******************************************************************************/

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void hw_version_init(void);
uint8_t hw_version_get(void);

#endif /* HW_VERSION_H_ */
