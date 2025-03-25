/***************************************************************************//**
 * @brief
 * @file    mov_avg.c
 * @author  Robson Cardoso
 * @date    01/08/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "mov_avg.h"

#if FIXED_POINT_CALC
#include "IQmathLib.h"
#endif

/*******************************************************************************
 * Private functions
 ******************************************************************************/
#if FIXED_POINT_CALC
static inline void sort(s_mov_avg_filter_t *const s_filter, int32_t l_value);
#else
static inline void sort(s_mov_avg_filter_t *const s_filter, double_t d_value);
#endif

/*******************************************************************************
 * Public functions
 ******************************************************************************/
#if FIXED_POINT_CALC
void mov_avg_init(s_mov_avg_filter_t *const s_filter)
{
    uint8_t uc_idx;

    for(uc_idx = 0; uc_idx < MOV_AVG_WINDOW_SIZE; uc_idx++)
    {
        s_filter->ad_window[uc_idx] = 0.0;
        s_filter->aui_rank[uc_idx] = (uint16_t) uc_idx;
    }
    s_filter->uc_currentIdx = 0;
}

int32_t mov_avg_update(s_mov_avg_filter_t *const s_filter, int32_t l_value)
{
    uint8_t uc_idx;
    int64_t ll_ret = 0;

    // Sort buffer
    sort(s_filter, l_value);

    // Apply filter
    for(uc_idx = 0; uc_idx < MOV_AVG_WINDOW_SIZE; uc_idx++)
    {
        if((0x02 == s_filter->aui_rank[uc_idx]) ||
           (0x03 == s_filter->aui_rank[uc_idx]) ||
           (0x04 == s_filter->aui_rank[uc_idx]))
        {
            ll_ret += s_filter->ad_window[uc_idx];
        }
    }

    ll_ret = ll_ret / MOV_AVG_FILTER_SIZE;

    return (int32_t) ll_ret;
}

#else
/**
 * @brief
 *      Initialize filter.
 */
void mov_avg_init(s_mov_avg_filter_t *const s_filter)
{
    uint8_t uc_idx;

    for(uc_idx = 0; uc_idx < MOV_AVG_WINDOW_SIZE; uc_idx++)
    {
        s_filter->ad_window[uc_idx] = 0.0;
        s_filter->aui_rank[uc_idx] = (uint16_t) uc_idx;
    }
    s_filter->uc_currentIdx = 0;
}

/**
 * @brief
 *      Update filter.
 *
 * @param[in]   s_filter
 *      Filter instance.
 *
 * @param[in]   d_value
 *      Updating value.
 *
 * @return
 *      Returns filtered value.
 */
double_t mov_avg_update(s_mov_avg_filter_t *const s_filter, double_t d_value)
{
    uint8_t uc_idx;
    double_t d_ret = 0.0;

    // Sort buffer
    sort(s_filter, d_value);

    // Apply filter
    for(uc_idx = 0; uc_idx < MOV_AVG_WINDOW_SIZE; uc_idx++)
    {
        if((0x02 == s_filter->aui_rank[uc_idx]) ||
           (0x03 == s_filter->aui_rank[uc_idx]) ||
           (0x04 == s_filter->aui_rank[uc_idx]))
        {
            d_ret += s_filter->ad_window[uc_idx];
        }
    }

    d_ret /= (double_t) MOV_AVG_FILTER_SIZE;

    return d_ret;
}
#endif

/*******************************************************************************
 * Private functions
 ******************************************************************************/
#if FIXED_POINT_CALC
static inline void sort(s_mov_avg_filter_t *const s_filter, int32_t l_value)
{
    uint8_t uc_cnt = MOV_AVG_WINDOW_SIZE;
    uint8_t uc_nextIdx = s_filter->uc_currentIdx;
    uint16_t ui_rank = s_filter->aui_rank[s_filter->uc_currentIdx];

    // Update ranks
    if(s_filter->ad_window[s_filter->uc_currentIdx] < l_value)
    {
        while(uc_cnt--)
        {
            uc_nextIdx = (uc_nextIdx < (MOV_AVG_WINDOW_SIZE-1)) ? (uc_nextIdx+1) : 0;

            if(uc_nextIdx != s_filter->uc_currentIdx)
            {
                if((s_filter->ad_window[uc_nextIdx] >=
                        s_filter->ad_window[s_filter->uc_currentIdx]) &&
                   (s_filter->ad_window[uc_nextIdx] <= l_value))
                {
                    if(s_filter->aui_rank[uc_nextIdx] > s_filter->aui_rank[s_filter->uc_currentIdx])
                    {
                        if(s_filter->aui_rank[uc_nextIdx] > ui_rank)
                        {
                            ui_rank = s_filter->aui_rank[uc_nextIdx];
                        }
                        s_filter->aui_rank[uc_nextIdx]--;
                    }
                }
            }
        }
    }
    else if(s_filter->ad_window[s_filter->uc_currentIdx] > l_value)
    {
        while(uc_cnt--)
        {
            uc_nextIdx = (uc_nextIdx < (MOV_AVG_WINDOW_SIZE-1)) ? (uc_nextIdx+1) : 0;

            if(uc_nextIdx != s_filter->uc_currentIdx)
            {
                if((s_filter->ad_window[uc_nextIdx] <=
                        s_filter->ad_window[s_filter->uc_currentIdx]) &&
                   (s_filter->ad_window[uc_nextIdx] >= l_value))
                {
                    if(s_filter->aui_rank[uc_nextIdx] < s_filter->aui_rank[s_filter->uc_currentIdx])
                    {
                        if(s_filter->aui_rank[uc_nextIdx] < ui_rank)
                        {
                            ui_rank = s_filter->aui_rank[uc_nextIdx];
                        }
                        s_filter->aui_rank[uc_nextIdx]++;
                    }
                }
            }
        }
    }

    s_filter->aui_rank[s_filter->uc_currentIdx] = ui_rank;
    s_filter->ad_window[s_filter->uc_currentIdx] = l_value;
    s_filter->uc_currentIdx = (s_filter->uc_currentIdx < (MOV_AVG_WINDOW_SIZE-1)) ?
                                (s_filter->uc_currentIdx+1) : 0;
}

#else
/**
 * @brief
 *      Sort buffer.
 *
 * @details
 *      This method sorts the moving average filter internal buffer.
 *
 *      For an N-element buffer, elements that are less than both the old
 *      element being forced out and the new element moved in will maintain
 *      their rank within the buffer. Likewise, elements greater than both the
 *      old element and the new element will retain their rank within the
 *      buffer.
 *
 *      Only elements whose values fall between the old element being forced out
 *      and the new element being moved in will have their rank changed:
 *      - If the old element has a value less than the new element, the
 *      elements that lie between these two have their rank decreased by one and
 *      the newly-added element is ranked just above them.
 *      - If the old element is greater than the new element, the existing
 *      elements that fall between these two will have their rank incremented by
 *      one and the new element will be ranked just below them.
 *
 * @param[in]   s_filter
 *      Iterator instance.
 *
 * @param[in]   d_value
 *      Updating value.
 */
static inline void sort(s_mov_avg_filter_t *const s_filter, double_t d_value)
{
    uint8_t uc_cnt = MOV_AVG_WINDOW_SIZE;
    uint8_t uc_nextIdx = s_filter->uc_currentIdx;
    uint16_t ui_rank = s_filter->aui_rank[s_filter->uc_currentIdx];

    // Update ranks
    if(s_filter->ad_window[s_filter->uc_currentIdx] < d_value)
    {
        while(uc_cnt--)
        {
            uc_nextIdx = (uc_nextIdx < (MOV_AVG_WINDOW_SIZE-1)) ? (uc_nextIdx+1) : 0;

            if(uc_nextIdx != s_filter->uc_currentIdx)
            {
                if((s_filter->ad_window[uc_nextIdx] >=
                        s_filter->ad_window[s_filter->uc_currentIdx]) &&
                   (s_filter->ad_window[uc_nextIdx] <= d_value))
                {
                    if(s_filter->aui_rank[uc_nextIdx] > s_filter->aui_rank[s_filter->uc_currentIdx])
                    {
                        if(s_filter->aui_rank[uc_nextIdx] > ui_rank)
                        {
                            ui_rank = s_filter->aui_rank[uc_nextIdx];
                        }
                        s_filter->aui_rank[uc_nextIdx]--;
                    }
                }
            }
        }
    }
    else if(s_filter->ad_window[s_filter->uc_currentIdx] > d_value)
    {
        while(uc_cnt--)
        {
            uc_nextIdx = (uc_nextIdx < (MOV_AVG_WINDOW_SIZE-1)) ? (uc_nextIdx+1) : 0;

            if(uc_nextIdx != s_filter->uc_currentIdx)
            {
                if((s_filter->ad_window[uc_nextIdx] <=
                        s_filter->ad_window[s_filter->uc_currentIdx]) &&
                   (s_filter->ad_window[uc_nextIdx] >= d_value))
                {
                    if(s_filter->aui_rank[uc_nextIdx] < s_filter->aui_rank[s_filter->uc_currentIdx])
                    {
                        if(s_filter->aui_rank[uc_nextIdx] < ui_rank)
                        {
                            ui_rank = s_filter->aui_rank[uc_nextIdx];
                        }
                        s_filter->aui_rank[uc_nextIdx]++;
                    }
                }
            }
        }
    }

    s_filter->aui_rank[s_filter->uc_currentIdx] = ui_rank;
    s_filter->ad_window[s_filter->uc_currentIdx] = d_value;
    s_filter->uc_currentIdx = (s_filter->uc_currentIdx < (MOV_AVG_WINDOW_SIZE-1)) ?
                                (s_filter->uc_currentIdx+1) : 0;
}
#endif
