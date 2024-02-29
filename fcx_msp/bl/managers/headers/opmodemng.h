/*******************************************************************************
 * @brief
 * @file    opmodemng.h
 * @author  Robson Cardoso
 * @date    27/03/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef OPMODEMNG_H_
#define OPMODEMNG_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "types.h"

/*******************************************************************************
 * Enumerations
 ******************************************************************************/

/** Operating mode definition enumeration. */
typedef enum E_OPMODE
{
    /** Pre-calibration operating mode - provides means to check board health
        before the meter arrives to calibration bench but at the same time
        allows for low power consumption. */
    E_OPMODE_PRE_CALIBRATION,
    /** Calibration operating mode. */
    E_OPMODE_CALIBRATION,
    /** Post-calibration operating mode. */
    E_OPMODE_POST_CALIBRATION,
    /** Stand-by operating mode. */
    E_OPMODE_STANDBY,
    /** Normal operating mode. */
    E_OPMODE_NORMAL,
    /** Verification operating mode. */
    E_OPMODE_VERIFICATION,
    /** Communication operating mode. */
    E_OPMODE_COMMUNICATION
} E_OPMODE_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void opmodemng_init(void);
void opmodemng_update(void);
E_OPMODE_t opmodemng_getCurrentOpMode(void);
E_OPMODE_t opmodemng_getPreviousOpMode(void);
void opmodemng_setCurrentOpMode(E_OPMODE_t e_opmode);
void opmodemng_setPreviousOpMode(E_OPMODE_t e_opmode);
void opmodemng_loadPreCalibOpMode(void);
void opmodemng_loadCalibOpMode(void);
void opmodemng_loadPostCalibOpmode(void);
void opmodemng_loadStandByOpmode(void);
bool opmodemng_enableDataTransmission(void);
void opmodemng_disableDataTransmission(void);
bool opmodemng_setCalibrationFlag(bool b_calibrated);
bool opmodemng_setActivationFlag(bool b_activated);

#endif /* OPMODEMNG_H_ */
