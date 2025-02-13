/***************************************************************************//**
 * @brief
 * @file    logger.h
 * @author  Eduardo Balbino
 * @date    03/10/2023
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef BL_HEADERS_LOGGER_H_
#define BL_HEADERS_LOGGER_H_

#endif /* BL_HEADERS_LOGGER_H_ */

#include "time.h"
#include "configmng.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

/** Alarm flags. */
#define LOGGER_ALARM_FLAG_LOW_BATTERY               (1 << 0)
#define LOGGER_ALARM_FLAG_BACK_FLOW                 (1 << 1)
#define LOGGER_ALARM_FLAG_LEAKAGE                   (1 << 2)
#define LOGGER_ALARM_FLAG_AIR_IN_PIPE               (1 << 3)
#define LOGGER_ALARM_FLAG_TAMPERED                  (1 << 4)
#define LOGGER_ALARM_FLAG_BRUSH                     (1 << 5)
#define LOGGER_ALARM_FLAG_NO_USAGE                  (1 << 6)
#define LOGGER_ALARM_FLAG_OVER_TEMP                 (1 << 7)


/*******************************************************************************
 * Structures
 ******************************************************************************/

/** Log register. */
#pragma pack(push, 1)
typedef struct S_LOGGER_REG
{
    /** Timestamp. */
    struct tm s_timestamp;
    /** Forward volume in liters. */
    int64_t ll_forwardVol;
    /** Reverse volume in liters. */
    int64_t ll_reverseVol;
    /** Minumum flow rate in mL/h. */
    int32_t l_minFlowRate;
    /** Maximum flow rate in mL/h. */
    int32_t l_maxFlowRate;
    /** Alarm flags. */
    uint16_t ui_alarms;
} s_logger_reg_t;
#pragma pack(pop)

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void logger_init();
void logger_enable(bool b_enable);

#if CONFIG_OPTCOMM_ERASE_LOGS
void logger_erase();
#endif

void logger_updateConfig();
void logger_update();
uint16_t logger_size();
bool logger_read(s_logger_reg_t * const ps_reg, uint16_t uc_idx);
