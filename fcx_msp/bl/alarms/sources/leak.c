/***************************************************************************//**
 * @brief
 * @file    leak.c
 * @author  Robson Cardoso
 * @date    10/04/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "leak.h"
#include "processing.h"
#include "ring_buffer.h"
#include "systick.h"
#if DEBUG_LEAK
#include <stdio.h>
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** Size of detection buffer. */
#define LEAK_BUFFER_SIZE                        24

#if DEBUG
/** Detection interval in milliseconds. */
#define LEAK_UPDATE_INT_MS                      2000ULL
#else
/** Detection interval in milliseconds. */
#define LEAK_UPDATE_INT_MS                      3600000ULL
#endif

/** Detection threshold. */
#define LEAK_DETECTION_THRESHOLD                23

/** Number of no events to disable alarm. */
#define LEAK_NO_ALARM_DETECTION                  LEAK_BUFFER_SIZE - \
                                                  LEAK_DETECTION_THRESHOLD

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void leak_detect(void);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Indicates leakage is detected. */
static bool gb_isLeaking;

/** Stores previous net volume. */
static double_t gd_previousNetVolume;

/** Keeps track of elapsed time. */
static uint64_t gull_tmtTickMs;

/** Buffer to store detection flags. */
static uint8_t gauc_detectionBuffer[LEAK_BUFFER_SIZE];

/** Ring buffer instance to control detection flag storage. */
static s_ring_buf_t gs_ringBuffer;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize module.
 */
void leak_init(void)
{
    gb_isLeaking = false;
    gd_previousNetVolume = proc_getNetVolume();
    gull_tmtTickMs = systick_getSysTickMs();
    rbuf_init(&gs_ringBuffer, gauc_detectionBuffer, LEAK_BUFFER_SIZE);
}

/**
 * @brief
 *      Update detection buffer.
 */
void leak_update(void)
{
    if((systick_getSysTickMs() - gull_tmtTickMs) >= LEAK_UPDATE_INT_MS)
    {
        if(gd_previousNetVolume != proc_getNetVolume())
        {
            rbuf_put(&gs_ringBuffer, 1);
            gd_previousNetVolume = proc_getNetVolume();
        }
        else
        {
            rbuf_put(&gs_ringBuffer, 0);
        }

        leak_detect();
        gull_tmtTickMs = systick_getSysTickMs();
    }
}

/**
 * @brief
 *      Check if leak is detected.
 *
 * @return
 *      Returns 'true' is leak is detected. Otherwise, it returns 'false'.
 */
bool leak_isLeakage(void)
{   
    return gb_isLeaking;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/

/**
 * @brief
 *      Perform leakage detection.
 */
static void leak_detect(void)
{
    uint8_t uc_idx;
    uint8_t uc_count = 0;

    gb_isLeaking = true;

    for(uc_idx = 0; uc_idx < LEAK_BUFFER_SIZE; uc_idx++)
    {
        if(!gauc_detectionBuffer[uc_idx])
        {
            if(++uc_count >= LEAK_NO_ALARM_DETECTION)
            {
                gb_isLeaking = false;
                break;
            }
        }
    }

#if DEBUG_LEAK
    if(gb_isLeaking)
    {
        printf("%llu (LEAK): Leakage detected.\n", systick_getSysTickMs());
    }
    else
    {
        printf("%llu (LEAK): No leakage.\n", systick_getSysTickMs());
    }
#endif
}
