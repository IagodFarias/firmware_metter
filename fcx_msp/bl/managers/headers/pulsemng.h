/***************************************************************************//**
 * @brief
 * @file    pulsemng.h
 * @author  Robson Cardoso
 * @date    14/02/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/
#ifndef PULSEMNG_H_
#define PULSEMNG_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize pulse manager.
 */
void pulsemng_init(void);

/**
 * @brief
 *      Enable pulse manager
 */
void pulsemng_enable(void);

/**
 * @brief
 *      Disable pulse counter
 */
void pulsemng_disable(void);

/**
 * @brief
 *      Update pulse manager's state.
 */
void pulsemng_update(void);

/**
 * @brief
 *      Configure pulse constant.
 *
 * @param[in]   d_pulseConstant
 *      Pulse constant in pulses per cubic meters.
 */
void pulsemng_configWritePulseConstant(double_t volatile d_pulseConstant);

/**
 * @brief
 *      Sign the end of pulse.
 *
 * @return
 *      Returns 'true' if there's the pulse actived is over.
 */
bool pulsemng_isEndPulse(void);

/**
 * @brief
 *      Checks if pulse is active.
 *
 * @return
 *      Returns 'true' if there's a pulse active. Otherwise, it returns 'false'.
 */
bool pulsemng_isActive(void);

/*******************************************************************************
 * Events and Callback Functions
 ******************************************************************************/
/**
 * @brief
 *      Request the pulse per meter configuration.
 *
 * @return
 *      Return the pulse per meter value from memory.
 */
extern double_t pulsemng_evt_getPulsePerMeter(void);

/**
 * @brief
 *      Do a request to wake up the software in asynchronous mode.
 *
 * @param[in]   ui_wakeUpInMs
 *      Time during of pulse output.
 */
extern void pulsemng_evt_pulseReq(uint16_t ui_wakeUpInMs);

#endif /* PULSEMNG_H_ */
