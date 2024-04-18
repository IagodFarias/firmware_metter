/***************************************************************************//**
 * @brief
 * @file    dfsm.h
 * @author  Robson Cardoso
 * @date    19/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Structures
 ******************************************************************************/
/** Frequency scheme structure. */
typedef struct S_FREQ_SCHEME
{
    /** Wake up time interval in milliseconds. */
    uint16_t ui_wakeUpIntMs;
    /** Sampling rate in Hz. */
    double_t d_samplingRate;
} s_freq_scheme_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void dfsm_init(void);
void dfsm_setHighFreqScheme(s_freq_scheme_t const *const s_scheme);
void dfsm_setLowFreqScheme(s_freq_scheme_t const *const s_scheme);
void dfsm_setUserInteractFreqScheme(s_freq_scheme_t const *const s_scheme);
void dfsm_loadFrequencyScheme(s_freq_scheme_t s_scheme);
void dfsm_checkFrequency(bool b_userInt, bool b_screenLock);
void dfsm_update(void);
