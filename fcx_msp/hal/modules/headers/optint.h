/***************************************************************************//**
 * @brief
 * @file    optint.h
 * @author  Robson Cardoso
 * @date    17/02/2017
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef OPTINT_H_
#define OPTINT_H_

/*******************************************************************************
 * Structures
 ******************************************************************************/

/** Optical interface state structure. */
typedef enum E_OPTINT_STATE
{
    /** Indicates optical interface is pressed. */
    E_OPTINT_STATE_PRESSED,
    /** Indicates optical interface is released. */
    E_OPTINT_STATE_RELEASED,
    /** Indicates optical interface is in unknown state. */
    E_OPTINT_STATE_UNKNOWN
} E_OPTINT_STATE_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void optint_init(void);
void optint_start(void);
void optint_update(void);
void enableRxCircuit(bool b_enable);
bool optcomm_RxPeekByte(uint8_t *byte);
E_OPTINT_STATE_t optint_getState(void);
void optint_enableRx(bool b_enable);
bool optint_receive(uint8_t *puc_msg, uint8_t uc_len);
void optint_enableTx(bool b_enable);
void optint_send(uint8_t *puc_msg, uint8_t len);
bool optint_isTransmitting();
void optint_reset();
volatile bool gb_optint_isReceiving;
volatile uint16_t guc_optint_bit_count;       // Bit count, used when transmitting byte


#endif /* OPTINT_H_ */
