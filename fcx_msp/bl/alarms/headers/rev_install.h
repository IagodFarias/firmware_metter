/***************************************************************************//*
 * @brief
 * @file    rev_install.h
 * @author  Eric Costa Hall
 * @date    03/11/2021
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/
#ifndef REV_INSTALL_H_
#define REV_INSTALL_H_

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
void rev_install_init(void);

/**
* @brief
*      Update detection buffer.
*/
void rev_install_update(void);

/**
* @brief
*      Check if reverse installation is detected.
*
* @return
*      Returns 'true' if reverse installation is detected.
*      Otherwise, it returns 'false'.
*/
bool rev_install_isDetected(void);

 /**
  * @brief
  *      Update module with a new configuration.
  */
void rev_install_updateConfig(void);

#endif /* REV_INSTALL_H_ */
