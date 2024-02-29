/******************************************************************************
 *                                 Includes
 *****************************************************************************/
#include <string.h>
#include "queue.h"
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

/******************************************************************************
 *                                Prototypes
 *****************************************************************************/
STATIC void queue_removeData(s_queue_t *ps_queue);

/******************************************************************************
 *                             Global Variables
 *****************************************************************************/

/******************************************************************************
 *                             Public Functions
 *****************************************************************************/
void queue_init(uint32_t *pul_buffer, uint8_t uc_size, s_queue_t *ps_queue)
{
    ps_queue->array = pul_buffer;
    ps_queue->capacity = uc_size;
    ps_queue->rear = -1;
    ps_queue->size = 0;
    ps_queue->front = 0;
}

void queue_reset(s_queue_t *ps_queue)
{
    ps_queue->rear = -1;
    ps_queue->size = 0;
    ps_queue->front = 0;
}

uint32_t queue_getIndex(s_queue_t *ps_queue, uint8_t uc_index)
{
    return ps_queue->array[uc_index];
}

bool queue_isFull(s_queue_t *ps_queue)
{
    return ps_queue->size == ps_queue->capacity;
}

uint8_t queue_size(s_queue_t *ps_queue)
{
    return ps_queue->size;
}

void queue_insertRear(s_queue_t *ps_queue, uint32_t ul_data)
{
    if(!queue_isFull(ps_queue))
    {
        if(ps_queue->rear == ps_queue->capacity - 1)
        {
            ps_queue->rear = -1;
        }

        ps_queue->array[++ps_queue->rear] = ul_data;
        ps_queue->size++;
    }
    else
    {
        queue_removeData(ps_queue);
        ps_queue->array[ps_queue->rear] = ul_data;
    }
}

/******************************************************************************
 *                            Private Functions
 *****************************************************************************/
STATIC void queue_removeData(s_queue_t *ps_queue)
{
    uint8_t uc_index = 0;

    //shifting all other elements
    for(; uc_index < ps_queue->capacity - 1; uc_index++)
    {
        ps_queue->array[uc_index] = ps_queue->array[uc_index + 1];
    }
}
