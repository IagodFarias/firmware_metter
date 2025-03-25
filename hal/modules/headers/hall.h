/***************************************************************************//**
 * @brief   
 * @file 	hall.h
 * @author 	Eric Costa Hall
 * @date 	08/11/2021
 * @section license
 * 			FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/
#ifndef HALL_H_
#define HALL_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

typedef enum
{
    E_HALL_STATE_PRESSED,
    E_HALL_STATE_RELEASED

} E_HALL_STATE_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void hall_init(void);
void hall_update(void);
E_HALL_STATE_t hall_getState();

#endif /* HALL_H_ */
