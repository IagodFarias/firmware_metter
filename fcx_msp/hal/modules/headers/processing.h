/*
 * processing.h
 *
 *  Created on: 2 de abr de 2019
 *      Author: Eric Costa Hall
 */

#ifndef _PROCESSING_H_
#define _PROCESSING_H_

#include "types.h"
#include "ussSwLib.h"
#include "utils_fcx.h"
#include "us.h"

/** Size of the speed of sound to temperature lookup table. */
#define PROC_SOS_TO_TEMP_LUT_SIZE               11

/** Size of the temperature to viscosity lookup table. */
#define PROC_TEMP_TO_VISC_LUT_SIZE              11

/** Size of the calibration lookup table. */
#define PROC_CALIBRATION_LUT_SIZE               30

/** Default sampling rate in Hz. */
#define PROC_SAMPLING_RATE_DEFAULT              1.0

/** Default minimum flow rate in mÂ³/h. */
#define PROC_MIN_FLOWRATE_DEFAULT               0.0

/** Default maximum flow rate in mÂ³/h. */
#define PROC_MAX_FLOWRATE_DEFAULT               0.0

/** Mechanical configuration structure. */
typedef struct S_MEC_CONFIG
{
    /** Flow path's length in meters. */
    double_t d_flowPathLength;
    /** Flow path's diameter in meters. */
    double_t d_flowPathDiameter;
    /** Flow path's effective area in squared meters. */
    double_t d_flowPathArea;
} s_mec_config_t;

/** Processing structure. */
typedef struct S_PROCESSING
{
    /** Estimated speed of sound [m/s]. */
    double_t d_speedOfSound;
    /** Uncalibrated velocity [m/s]. */
    double_t d_uncalibVel;
    /** Media temperature [Â°C]. */
    double_t d_temperature;
    /** Media kinematic viscosity [mÂ³/s]. */
    double_t d_viscosity;
    /** Reynolds number [-]. */
    double_t d_reynolds;
    /** Calibration constant [-]. */
    double_t d_calibConst;
    /** Calibrated velocity [m/s]. */
    double_t d_calibVel;
    /** Current volume flow rate [m³/h]. */
    double_t d_currentFlowRate;
    /** Minimum volume flow rate [m³/h]. */
    double_t d_minFlowRate;
    /** Maximum volume flow rate [m³/h]. */
    double_t d_maxFlowRate;
    /** Volume [m³]. */
    double_t d_volume;
    /** Forward volume [m³]. */
    double_t d_forwardVolume;
    /** Reverse volume [m³]. */
    double_t d_reverseVolume;
    /** Ultrasonic measurements. */
    s_us_meas_t s_usMeasurement;
} s_processing_t;

/** Calibration structure. */
typedef struct S_CALIB_CONFIG
{
    /** Delay speed of sound in seconds. It accounts for any measurement delays
     *  that do not arise as a result of the sound traveling in the fluid. */
    double_t d_delaySpeedOfSound;//8
    /** Delay zero calibration in seconds. It accounts for any unbalance between
     *  the downstream and upstream transit times. */
    double_t d_delayZeroCalibration;//8
    /** Meter's calibration lookup table. */
    s_lut_coordinate_t as_calibLut[PROC_CALIBRATION_LUT_SIZE];//480 bytes
    /** Size of meter's calibration lookup table. */
    uint16_t ui_calibLutSize;//2
} s_calib_config_t;

void proc_init(void);

/**
 * @brief
 *      Enable transit time filter.
 */
void proc_enableFilters(void);

/**
 * @brief
 *      Disable transit time filter.
 */
void proc_disableFilters(void);

/**
 * @brief
 *      Reset volume counters.
 */
void proc_resetVolumeCounters(void);

/**
 * @brief
 *      Configure sampling rate.
 *
 * @details
 *      This method configures processing sampling rate.
 *
 * @param[in]   d_samplingRate
 *      Sampling rate in hertz.
 */
void proc_configWriteSamplingRate(double_t d_samplingRate);

/**
 * @brief
 *      Get current sampling rate.
 *
 * @details
 *      This method gets current processing sampling rate.
 *
 * @return
 *      Returns processing sampling rate in hertz.
 */
double_t proc_configReadSamplingRate(void);

/**
 * @brief
 *      Update processing.
 *
 * @details
 *      This method process all process variables.
 */
void proc_update();

/**
 * @brief
 *      Enable processing.
 */
void proc_enableProcessing(void);

/**
 * @brief
 *      Disable processing.
 */
void proc_disableProcessing(void);

/**
 * @brief
 *      Check whether the current measurement indicates a reverse flow rate.
 *
 * @return
 *      Returns 'true' if a reverse flow rate is detected. Otherwise, it returns
 *      'false'.
 */
bool proc_isReverseFlow(void);

/**
 * @brief
 *      Check whether current measurement indicate there's air in pipe.
 *
 * @return
 *      Returns 'true' if air in pipe is detected. Otherwise, it returns 'false'.
 */
bool proc_isAirInPipe(void);

s_processing_t* proc_getResults(void);
s_calib_config_t* proc_getCalibLut(void);

/**
 * @brief
 *      Get net volume.
 *
 * @return
 *      Returns the net volume in cubic meters.
 */
double_t proc_getNetVolume(void);

/**
 * @brief
 *      Get current flow rate.
 *
 * @return
 *      Returns the current flow rate in cubic meters per hour.
 */
double_t proc_getCurrentFlowRate(void);

/**
 * @brief
 *      Get temperature.
 *
 * @return
 *      Returns the temperature in degree Celsius
 */
double_t proc_getTemperature(void);

/**
 * @brief
 *      Get reverse volume.
 *
 * @return
 *      Returns the reverse volume in cubic meters.
 */
double_t proc_getReverseVolume(void);

/**
 * @brief
 *      Get Forward volume.
 *
 * @return
 *      Returns the forward volume in cubic meters.
 */
double_t proc_getForwardVolume(void);
/**
 * @brief
 *      Get Max Flow Rate.
 *
 * @return
 *      Returns the Max Flow Rate in m³/h.
 */

double_t proc_getMaxFlowRate(void);
/**
 * @brief
 *      Get Min Flow Rate.
 *
 * @return
 *      Returns the Min Flow Rate in m³/h.
 */

double_t proc_getMinFlowRate(void);
/**
 * @brief
 *      Reset the maximum and minimum flowrates
 *      for storage, based on each time slice:
 *      weekly/daily/hourly/15-minutly etc.
 */

void proc_resetFlowRates(void);
/**
 * @brief
 *      Update fixed pointers with new calibration variables.
 */

void proc_updateConfig(void);

#if FIXED_POINT_CALC
/**
 * @brief
 *      Get volume calculated in the last measure.
 *
 * @return
 *      Returns the volume calculated in fixed pint of 30.
 */
int32_t proc_getVolFixedPoint(void);
int32_t proc_getForVolFixedPoint(void);
int32_t proc_getRevVolFixedPoint(void);
int32_t proc_getMinFlowRateFixedPoint(void);
int32_t proc_getMaxFlowRateFixedPoint(void);
#endif

#if DEBUG_PROC_FLOWRATE
void proc_toogleFlowRate(void);
#endif

#endif /* PROCESSING_H_ */
