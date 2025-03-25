/***************************************************************************//*
 * @brief
 * @file    temp.h
 * @author  Eric Costa Hall
 * @date    04/11/2021
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/
#ifndef TEMP_H_
#define TEMP_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
* @brief
*      Initialize module.
*/
void temp_init(void);

/**
* @brief
*      Update detection buffer.
*/
void temp_update(void);

/**
* @brief
*      Check if high/Low temperature outside limits is detected.
*
* @return
*      Returns 'true' if high/low temperature is detected.
*      Otherwise, it returns 'false'.
*/
bool temp_isOutsideLimit(void);

#endif /* TEMP_H_ */
