/***************************************************************************//**
 * @file 	ring_buffer.c
 * @brief  	Very simple ring first-in first-out buffer.
 * @details This a simple ring buffer which contains all methods for
 * 			initialization, read from and write to a buffer.
 *
 *			Managing overflow: in case of overflow latest incoming data is
 *			dropped.
 *
 * @date    11/08/2014
 * @author	Robson Cardoso
 * @section license
 * 			FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "ring_buffer.h"
#include "utils_fcx.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *		Initialize ring buffer.
 *
 * @details
 *		This function initializes all fields of the ring buffer.
 *
 * @param[in]	*ps_ringBuffer
 * 		Pointer to the ring buffer.
 *
 * @param[in]	*puc_buffer
 * 		Pointer to the buffer instance where data should be stored.
 *
 * @param[in]	uc_size
 * 		Size of the buffer.
 */
void rbuf_init(s_ring_buf_t *const ps_ringBuffer, uint8_t *const puc_buffer,
			   uint8_t const uc_size)
{
	uint8_t uc_idx;

	// Check if uc_size is a valid buffer size
	ASSERT(uc_size < RBUF_MAX_BUFFER_SIZE);

	// Initializes fields of the data structure
	ps_ringBuffer->puc_buffer	= puc_buffer;
	ps_ringBuffer->uc_size 		= uc_size;
	ps_ringBuffer->ui_head 		= 0;
	ps_ringBuffer->ui_tail 		= 0;
	ps_ringBuffer->uc_count		= 0;

	// Flushes the entire buffer
	for(uc_idx = 0; uc_idx < ps_ringBuffer->uc_size; uc_idx++)
	{
		ps_ringBuffer->puc_buffer[uc_idx] = 0;
	}
}

/**
 * @brief
 *		Determine whether or not the buffer is empty.
 *
 * @param[in]	*ps_ringBuffer
 * 		Pointer to the ring buffer.
 *
 * @return
 * 		Returns 'true' if the buffer is empty. Otherwise it returns 'false'.
 */
bool rbuf_empty(s_ring_buf_t *const ps_ringBuffer)
{
	return (bool) (0 == ps_ringBuffer->uc_count);
}

/**
 * @brief
 *		Determine whether or not the buffer is full.
 *
 * @param[in]	*ps_ringBuffer
 * 		Pointer to the ring buffer.
 *
 * @return
 * 		Returns 'true' if the buffer is full. Otherwise it returns 'false'.
 */
bool rbuf_full(s_ring_buf_t *const ps_ringBuffer)
{
	return (bool) (ps_ringBuffer->uc_count == ps_ringBuffer->uc_size);
}

/**
 * @brief
 *		Get a byte from the buffer (tail).
 *
 * @param[in] 	*ps_ringBuffer
 * 		Pointer to the ring buffer.
 *
 * @param[in]	*puc_byte
 * 		Pointer to variable where the byte should be stored.
 *
 * @return
 *		Returns 'true' if there is data available. Otherwise, returns 'false'.
 */
bool rbuf_get(s_ring_buf_t *const ps_ringBuffer, uint8_t *const puc_byte)
{
	bool b_status;

	if(rbuf_empty(ps_ringBuffer))
	{
		b_status = false;
	}
	else
	{
		// Get data from buffer
		*puc_byte = ps_ringBuffer->puc_buffer[ps_ringBuffer->ui_tail];

		// Update tail's index and wrap up if necessary
		ps_ringBuffer->ui_tail =
				(ps_ringBuffer->ui_tail < (ps_ringBuffer->uc_size - 1)) ?
				(ps_ringBuffer->ui_tail + 1) : 0;

		// Update buffer's counter and set status flag
		ps_ringBuffer->uc_count--;
		b_status = true;
	}

	return b_status;
}

/**
 * @brief
 *		Put a byte into the buffer (head).
 *
 * @param[in] 	*ps_ringBuffer
 * 		Pointer to the ring buffer.
 *
 * @param[in]	uc_byte
 * 		Value that should be stored in the buffer.
 *
 * @return
 *		Returns 'true' if the byte has been successfully stores in the buffer.
 *		Otherwise, returns 'false'.
 */
bool rbuf_put(s_ring_buf_t *const ps_ringBuffer, uint8_t const uc_byte)
{
	bool b_status;

	b_status = rbuf_full(ps_ringBuffer);

	// Put data into buffer
	ps_ringBuffer->puc_buffer[ps_ringBuffer->ui_head] = uc_byte;

	// Update head's index and wrap up if necessary
	ps_ringBuffer->ui_head =
			(ps_ringBuffer->ui_head < (ps_ringBuffer->uc_size - 1)) ?
			(ps_ringBuffer->ui_head + 1) : 0;

	// Update buffer's counter and set status flag
	if(!b_status)
	{
		ps_ringBuffer->uc_count++;
	}

	return b_status;
}
