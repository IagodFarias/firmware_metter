/***************************************************************************//**
 * @file 	ring_buffer.h
 * @brief 	Very simple ring first-in first-out buffer.
 * @details This a simple ring buffer which contains all methods for
 *          initialization, read from and write to a buffer.
 *
 *          Managing overflow: in case of overflow latest incoming data is
 *          dropped.
 *
 * @date 	11/08/2014
 * @author 	Robson Cardoso
 * @section license
 * 			FAE Sistemas de Medição. All rights reserved
 ******************************************************************************/
#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Maximum buffer size. */
#define RBUF_MAX_BUFFER_SIZE	0xFF

/*******************************************************************************
 * Structures
 ******************************************************************************/
/** Data structure for the ring buffer. */
typedef struct
{
	/** This is a pointer to the managed buffer. */
	uint8_t *puc_buffer;

	/** This field stores the size of the buffer. */
	uint8_t uc_size;

	/** This is the head index. The incoming bytes get written to the managed
	 * 	buffer using this index. The arithmetic for this index is in
	 * 	modulo-uc_size. */
	uint16_t ui_head;

	/** This index is used to retrieve the oldest data in the buffer. This index
	 * 	also follow the same modulus arithmetic as in head index case. */
	uint16_t ui_tail;

	/** This field is used to keep track of the total number of elements
	 * 	currently in the buffer. The maximum value of this field is set by the
	 * 	uc_size. */
	uint8_t uc_count;
} s_ring_buf_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void rbuf_init(s_ring_buf_t *const ps_ringBuffer, uint8_t *const puc_buffer,
			   uint8_t const uc_size);
bool rbuf_empty(s_ring_buf_t *const ps_ringBuffer);
bool rbuf_full(s_ring_buf_t *const ps_ringBuffer);
bool rbuf_get(s_ring_buf_t *const ps_ringBuffer, uint8_t *const puc_byte);
bool rbuf_put(s_ring_buf_t *const ps_ringBuffer, uint8_t const uc_byte);

#endif /* RING_BUFFER_H_ */
