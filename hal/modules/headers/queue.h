//=============================================================================
/*!
 *    FAE Sistemas de Medição
 *    All rights reserved
 *
 *    Coding standards: v1.1
 *
 *    @brief <Brief Description>
 *    @file queue.h
 *    @author Eric Hall
 *    @date 15/06/2016
 *    @details <Detailed Description>
 *
 */
//=============================================================================

#ifndef QUEUE_H_
#define QUEUE_H_

/******************************************************************************
 *                                 Includes
 *****************************************************************************/
#include "general_settings.h"
#include "types.h"

/******************************************************************************
 *                                Definitions
 *****************************************************************************/

/******************************************************************************
 *                               Enumerations
 *****************************************************************************/

/******************************************************************************
 *                                Structures
 *****************************************************************************/
typedef struct QUEUE
{
    int8_t front, rear, size;
    uint8_t capacity;
    uint32_t* array;
} s_queue_t;

/******************************************************************************
 *                             Global Variables
 *****************************************************************************/

/******************************************************************************
 *                             Public Functions
 *****************************************************************************/
void queue_init(uint32_t *pul_buffer, uint8_t uc_size, s_queue_t *ps_queue);
void queue_reset(s_queue_t *ps_queue);
void queue_insertRear(s_queue_t *ps_queue, uint32_t ul_data);
uint32_t queue_getIndex(s_queue_t *ps_queue, uint8_t uc_index);
uint8_t queue_size(s_queue_t *ps_queue);
bool queue_isFull(s_queue_t *ps_queue);

#endif /* QUEUE_H_ */
