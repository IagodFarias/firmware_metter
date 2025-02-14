/***************************************************************************//**
 * @brief
 * @file 	systick.h
 * @author 	Robson Cardoso
 * @date 	13/02/2017
 * @details
 * @section license
 * 			FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef SYSTICK_H_
#define SYSTICK_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Maximum wake up interval. */
#define SYSTICK_WAKE_UP_INT_MS_MAX_INT          65535U

/** Default wake up interval. */
#define SYSTICK_WAKE_UP_INT_MS_DEFAULT_INT      1000U

/** Code for invalid tick code.*/
#define SYSTICK_INVALID_TICK                    0xFFFFFFFFFFFFFFFFUL

/** Systick timezone */
#define SYSTICK_CTS_TO_UTC_OFFSET               6

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize system tick.
 *
 * @details
 *      This method initializes module's control variables and sets the system
 *      wake up interval to its default value.
 */
void systick_init(void);

/**
 * @brief
 *      Set system wake up interval.
 *
 * @details
 *      This method sets the system's wake up time interval.
 *
 * @param[in]   ui_wakeUpIntMs
 *      Wake up time interval in milliseconds.
 *
 * @note
 *      @p ui_wakeUpIntMs should be greater than zero. Otherwise, the system
 *      wake up time will not be updated. Additionally, it should be less than
 *      or equal to 65335 (maximum value allowed to a uint16_t variable).
 *
 */
void systick_setWakeUpIntMs(uint16_t ui_wakeUpIntMs);

/**
 * @brief
 *      Get wake up interval in milliseconds.
 *
 * @return
 *      Return the wake up interval in milliseconds.
 */
uint16_t systick_getWakeUpIntMs(void);

/**
 * @brief
 *      Get current system tick in milliseconds.
 *
 * @return
 *      Returns the current system tick in milliseconds.
 */
uint64_t systick_getSysTickMs(void);

/**
 * @brief
 *      Update system tick.
 *
 * @details
 *      This method updates the system tick with the elapsed time since the last
 *      wake up.
 *
 * @note
 *      Please make sure that this method is not called multiple times during
 *      the same wake up cycle.
 */
void systick_updateSysTickMs(void);

/**
 * @brief
 *      Get current time in calendar time format.
 *
 * @return
 *      Returns current time in calendar time format.
 */
struct tm * systick_getActualDateTime(void);

/**
 * @brief
 *      Set current time in calendar time format.
 *
  * @param[in]   ps_calendar
 *      Calendar time format.
 *
 * @return
 *      Returns if true if the date was successful changed.
 */
bool systick_setActualDateTime(struct tm *ps_calendar);

#endif /* SYSTICK_H_ */
