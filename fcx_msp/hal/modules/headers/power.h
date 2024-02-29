/***************************************************************************//**
 * @brief
 * @file    power.h
 * @author  Robson Cardoso
 * @date    14/02/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef POWER_H_
#define POWER_H_

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
void power_enterSleep(void);
void power_wakeUp();

#endif /* POWER_H_ */
