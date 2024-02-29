/***************************************************************************//**
 * @file    serial.h
 * @brief   This module provides serial communication for the application.
 * @details It uses two ring buffers (receive and transmit) to provide serial
 *          data for the main application.
 * @date    11/08/2014
 * @author  Robson Cardoso
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/
#ifndef SERIAL_H_
#define SERIAL_H_


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
 * Public functions
 ******************************************************************************/
void serial_init(void);
uint16_t serial_transmitBytes(uint8_t *puc_data, uint16_t ui_size);
bool serial_receiveByte(uint8_t *puc_data);
bool serial_receiveBytes(uint8_t *puc_data, uint16_t ui_size);
bool serial_stillTransmitting(void);
void serial_enable(void);
void serial_disable(void);

#endif /* SERIAL_H_ */
