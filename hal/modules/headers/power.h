/***************************************************************************//**
 * @brief
 * @file    power.h
 * @author  Robson Cardoso
 * @date    14/02/2017
 * @details
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

#ifndef POWER_H_
#define POWER_H_

#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Describes possible wake up reasons. */
typedef enum E_WAKEUP_REASON
{
    /** Wake up after a watch dog timer timeout. */
    E_WAKEUP_REASON_SYSTEM_FAILURE,
    /** Wake up after the first power up. */
    E_WAKEUP_REASON_POWER_ON_RESET,
    /** Wake up after external reset. */
    E_WAKEUP_EXTERNAL_RESET,
    /** No operation. */
    E_WAKEUP_REASON_NO_OP,
    /** No operation. */
    E_WAKEUP_UNKNOWN_REASON,
} E_WAKEUP_REASON_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void power_init(void);
E_WAKEUP_REASON_t power_getWakeUpReason(void);
void power_enableCoreWakeupByRTC(void);
void power_disableCoreWakeupByRTC(void);
void power_enableCoreWakeupBySerial(void);
void power_disableCoreWakeupBySerial(void);
void power_enableCoreWakeupByMeasurementFinished(void);
void power_disableCoreWakeupByMeasurementFinished(void);
void power_enableUsWakeupByRTC(void);
void power_disableUsWakeupByRTC(void);
/** Selects LPM3 (deeper sleep, UART1 clock gated) for the main loop and
 * ultrasonic capture. Use in opmodes where UART1 is expected to be disabled
 * (STANDBY, NORMAL, VERIFICATION). */
void power_enableLpm3(void);
/** Selects LPM0 (UART1 clock stays alive) for the main loop and ultrasonic
 * capture. Use in opmodes where UART1 communication must stay reliable
 * (PRE_CALIBRATION, CALIBRATION, POST_CALIBRATION). */
void power_disableLpm3(void);
bool power_isLpm3Enabled(void);
void power_enterSleep(void);
void power_wakeUp();

#endif /* POWER_H_ */
