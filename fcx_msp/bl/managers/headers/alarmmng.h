/***************************************************************************//*
 * @brief
 * @file    alarmmng.h
 * @author  Gustavo SalomÃ£o
 * @date    03/04/2018
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/
#ifndef ALARMMNG_H_
#define ALARMMNG_H_

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
void alarmmng_init();

/**
 * @brief
 *      Update alarm manager.
 */
void alarmmng_update();

/**
 * @brief
 *      Check if leak is detected.
 *
 * @return
 *      Returns 'true' is leak is detected. Otherwise, it returns 'false'.
 */
bool alarmmng_isLeakage();

/**
 * @brief
 *      Check if reverse flow was detected.
 *
 * @return
 *      Returns 'true' if reverse flow was detected. Otherwise, it returns
 *      'false'.
 */
bool alarmmng_isReverseFlow();

/**
 * @brief
 *      Check if air in pipe was detected.
 *
 * @return
 *      Returns 'true' if air in pipe was detected. Otherwise, it returns
 *      'false'.
 */
bool alarmmng_isAirInPipe();

/**
 * @brief
 *      Check if the alarm for maximum overflow was detected.
 *
 * @return
 *      Returns 'true' if alarmed. Otherwise, it returns
 *      'false'.
 */
bool alarmmng_isBrush();

/**
 * @brief
 *      Check if an alarm of reverse installation was detected.
 *
 * @return
 *      Returns 'true' if alarmed. Otherwise, it returns
 *      'false'.
 */
bool alarmmng_isRevInstall();

/**
 * @brief
 *      Check if an alarm of no usage was detected.
 *
 * @return
 *      Returns 'true' if alarmed. Otherwise, it returns
 *      'false'.
 */
bool alarmmng_isNoUsage();

/**
 * @brief
 *      Check if an alarm of outside temperature was detected.
 *
 * @return
 *      Returns 'true' if alarmed. Otherwise, it returns
 *      'false'.
 */
bool alarmmng_isOverTemp();

#endif /* ALARMMNG_H_ */
