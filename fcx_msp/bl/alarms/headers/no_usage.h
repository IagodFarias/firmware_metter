/***************************************************************************//*
 * @brief
 * @file    no_usage.h
 * @author  Eric Costa Hall
 * @date    04/11/2021
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/
#ifndef NO_USAGE_H_
#define NO_USAGE_H_

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
void no_usage_init(void);

/**
* @brief
*      Update detection buffer.
*/
void no_usage_update(void);

/**
* @brief
*      Check if no usage is detected.
*
* @return
*      Returns 'true' if no usage is detected.
*      Otherwise, it returns 'false'.
*/
bool no_usage_isDetected(void);

 /**
  * @brief
  *      Update module with a new configuration.
  */
void no_usage_updateConfig(void);

#endif /* NO_USAGE_H_ */
