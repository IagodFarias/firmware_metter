/***************************************************************************//*
 * @brief
 * @file    mov_avg.h
 * @author  Robson Cardoso
 * @date    01/08/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Size of window. */
#define MOV_AVG_WINDOW_SIZE                     0x07

/** Number of samples used for averaging. */
#define MOV_AVG_FILTER_SIZE                     0x03

/*******************************************************************************
 * Structures
 ******************************************************************************/
#if FIXED_POINT_CALC
/* Moving average filter class. */
typedef struct S_MOV_AVG_FILTER
{
    /** Windowed data. It works as a circular buffer. */
    int32_t ad_window[MOV_AVG_WINDOW_SIZE];
    /** Rank array where the ith element stores the rank of element i of the
     *  windowed data. */
    uint16_t aui_rank[MOV_AVG_WINDOW_SIZE];
    /** Point to the index where the next element should be placed. */
    uint8_t uc_currentIdx;
} s_mov_avg_filter_t;
#else
/* Moving average filter class. */
typedef struct S_MOV_AVG_FILTER
{
    /** Windowed data. It works as a circular buffer. */
    double_t ad_window[MOV_AVG_WINDOW_SIZE];
    /** Rank array where the ith element stores the rank of element i of the
     *  windowed data. */
    uint16_t aui_rank[MOV_AVG_WINDOW_SIZE];
    /** Point to the index where the next element should be placed. */
    uint8_t uc_currentIdx;
} s_mov_avg_filter_t;
#endif
/*******************************************************************************
 * Public functions
 ******************************************************************************/

void mov_avg_init(s_mov_avg_filter_t *const s_filter);
#if FIXED_POINT_CALC
int32_t mov_avg_update(s_mov_avg_filter_t *const s_filter, int32_t l_value);
#else
double_t mov_avg_update(s_mov_avg_filter_t *const s_filter, double_t d_value);
#endif
