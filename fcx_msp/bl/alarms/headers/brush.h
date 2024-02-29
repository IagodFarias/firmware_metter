    /***************************************************************************//*
 * @brief
 * @file    brush.h
 * @author  Eduardo Balbino
 * @date    25/10/2024
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/
#ifndef BRUSH_H_
#define BRUSH_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
 void brush_init(void);
 void brush_update(void);
 bool brush_isOverflowing(void);
 void brush_updateConfig(void);

 #endif /* BRUSH_H_ */
