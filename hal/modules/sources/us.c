/***************************************************************************//**
 * @brief
 * @file    us.c
 * @author  Robson Cardoso
 * @date    28/02/2017
 * @details
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#if DEBUG_US
#include <stdio.h>
#endif

#include "general_settings.h"
#include "us.h"
#include "USS_userConfig.h"
#include "USS_intrplCorrLUT.h"
#include "USS_meterVFRCalibFlow.h"
#include "USS_temperatureLUT.h"
#include "assert.h"
#include "gpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#ifdef DEBUG_ASSERT
#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            /* Optional: You can add logging here using __FILE__ and __LINE__ */ \
            /* Example: log_assert_failure(__FILE__, __LINE__, #expr); */ \
            while (1) { \
                __disable_interrupt(); \
                /* Enter into LPM4 to save energy and wait for WTD Reset */ \
                __bis_SR_register(LPM4_bits); \
            } \
        } \
    } while (0)
#else
#define ASSERT(expr) \
    do { \
        if (!(expr)) { \
            while (1) { \
                __disable_interrupt(); \
                /* Enter into LPM4 to save energy and wait for WTD Reset */ \
                __bis_SR_register(LPM4_bits); \
            } \
        } \
    } while (0)
#endif /* DEBUG_ASSERT */


#define US_FREQ_TRANSD_IN_HZ                                    2000
#define US_FREQ_SWAP_RANGE_IN_HZ                                10
#define US_FREQ_SWAP_STEP_IN_HZ                                 1
/*! Multiplier of transducer frequency sent by Design Center */
#define US_TRANSFREQ_MULTIPLIER                                 (KILO)*(HZ)
#define US_FIRST_PEEK_DETECTED_LEVEL                            100

#if defined(__TI_COMPILER_VERSION__)
#if (USS_ALG_FILT_IS_FILTER_ENABLED == true)
#pragma PERSISTENT(gUSSFilterCoeffs)
#endif
#if USS_ALG_FILT_IS_FILTER_ENABLED
#pragma PERSISTENT(gUSSHilbertCoeffs)
#endif
#pragma PERSISTENT(gNVMemBlock)

#pragma PERSISTENT(ussSystemConfig)
#pragma PERSISTENT(ussMeterConfig)
#pragma PERSISTENT(singDualToneConfig)
#pragma PERSISTENT(ussPulseConfig)
#pragma PERSISTENT(ussMeasurementConfig)
#pragma PERSISTENT(ussPLLConfig)
#pragma PERSISTENT(ussInterruptConfig)
#pragma PERSISTENT(ussCaptureConfig)
#pragma PERSISTENT(ussTriggerConfig)
#pragma PERSISTENT(ussAlgConfig)
#pragma PERSISTENT(gUssSWConfig)
#define __persistent
#define __no_init
#elif defined (__IAR_SYSTEMS_ICC__)
#else
#error Compiler not supported!
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void trimRatioLobe(void);
double_t utils_calcSumPositives(int16_t *pi_data, uint16_t ui_size);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
#if USS_ENABLE_ABSTOF_LOBE_OFFSET_CORRECTION
const int16_t gUSSAbsToFLUT[USS_INTRPL_CORR_LUT_RES][2]=
        USS_INTRPL_CORR_LUT_VALUES;
#endif

// LEA and FRAM memory buffers
USS_HAL_LEA_DATA(gUSSLEATempMemBlock, 4)
__no_init int16_t gUSSLEATempMemBlock[2 * (USS_SW_LIB_APP_MAX_CAPTURE_SIZE +
                            USS_SW_LIB_APP_MAX_FILTER_LENGTH)];

#if(USS_ALG_DTOF_WINDOWING_MODE != USS_ALG_DTOF_EST_WINDOW_OPTION_DISABLED)
USS_HAL_LEA_DATA(gUSSLEARAMReservedBlock1, 4)
__no_init int16_t gUSSLEARAMReservedBlock1[USS_SW_LIB_APP_MAX_WINDOW_RAMP_SIZE];
#endif

//USS_HAL_LEA_DATA(gUSSLEARAMReservedBlock2, 4)
//__no_init int16_t gUSSLEARAMReservedBlock2[USS_SW_LIB_APP_MAX_CAPTURE_SIZE +
//                                           USS_SW_LIB_APP_MAX_HILBERT_FILTER_LENGTH];

static __persistent int32_t gNVMemBlock[(USS_SW_LIB_APP_MAX_CAPTURE_SIZE * 2)]={0};

//USS_HAL_LEA_DATA(gUSSBinaryPattern, 4)
//__no_init int16_t gUSSBinaryPattern[USS_BINARY_ARRAY_MAX_SIZE];

#if USS_ALG_FILT_IS_FILTER_ENABLED
// Filter Hilbert and Band Pass Coefficients
static __persistent int16_t gUSSHilbertCoeffs[
    USS_SW_LIB_APP_MAX_HILBERT_FILTER_OPTIONS][
    USS_SW_LIB_APP_MAX_HILBERT_FILTER_LENGTH] =
    USS_SW_LIB_APP_HILB_FILTER_COEFFICIENTS;
#endif

static __persistent USS_System_Configuration ussSystemConfig = {
    .LFXTFrequency = USS_LFXT_FREQ_IN_HZ,
    .mCLKFrequency = USS_MCLK_FREQ_IN_HZ,
    .timerBaseAddress = USSSWLIB_TIMER_BASE_ADDRESS,
    .measurementPeriod = USS_SYS_MEASUREMENT_PERIOD,
    .diagMode = USS_DIAGNOSTIC_MODE,
};

static __persistent USS_Meter_Configuration ussMeterConfig = {
    .acousticLength = USS_ACOUSTIC_LENGTH,
#if(USS_PULSE_MODE == USS_PULSE_MODE_SINGLE_TONE)
    .transducerFreq = USS_F1_FREQ,
#else
    .transducerFreq = ((USS_F1_FREQ + USS_F2_FREQ) / 2),
#endif
    .volumeScaleFactor = USS_VOLUME_SCALE_FACTOR,
    .volumeAddlScaleFactor = USS_VFR_LARGE_PIPE_ADDL_SF
};

__persistent USS_Pulse_Single_Dual_Tone_Configuration singDualToneConfig = {

#if ((USS_PULSE_MODE == USS_PULSE_MODE_SINGLE_TONE) ||    \
     (USS_PULSE_MODE == USS_PULSE_MODE_DUAL_TONE))
    .numOfF1Cycles = USS_NUM_OF_EXCITATION_PULSES_F1,
    .F1DutyCycle = (float) USS_PULSE_DUTYPERCENT_F1,
#if (USS_PULSE_MODE != USS_PULSE_MODE_SINGLE_TONE)
    .numOfF2Cycles = USS_NUM_OF_EXCITATION_PULSES_F2,
    .F2DutyCycle = (float) USS_PULSE_DUTYPERCENT_F2,
#else
    .numOfF2Cycles = 0,
    .F2DutyCycle   = 0,
#endif
#else
    .numOfF1Cycles = 0,
    .F1DutyCycle   = 0,
    .numOfF2Cycles = 0,
    .F2DutyCycle   = 0,
#endif

};

static __persistent USS_Pulse_Configuration ussPulseConfig = {
    .pauseState = USS_PAUSE_STATE,
    .pulsePolarity = USS_PULSE_POLARITY,
    .F1Frequency = USS_F1_FREQ,
#ifdef USS_NUM_OF_STOP_PULSES
    .numOfStopPulses = USS_NUM_OF_STOP_PULSES,
#else
    .numOfStopPulses = 0,
#endif
#if (USS_PULSE_MODE != USS_PULSE_MODE_SINGLE_TONE)
    .F2Frequency = USS_F2_FREQ,
#endif


#if (USS_PULSE_MODE == USS_PULSE_MODE_SINGLE_TONE)
    .pulseGenMode = USS_measurement_pulse_generation_mode_single_tone,
    .pToneConfig = &singDualToneConfig,
#elif (USS_PULSE_MODE == USS_PULSE_MODE_DUAL_TONE)
    .pulseGenMode = USS_measurement_pulse_generation_mode_dual_tone,
    .pToneConfig = &singDualToneConfig,
#elif (USS_PULSE_MODE == USS_PULSE_MODE_MULTI_TONE)
    .pulseGenMode = USS_measurement_pulse_generation_mode_multi_tone,
    .pToneConfig = &multiToneConfig,
#endif
};

static __persistent USS_Measurement_Configuration ussMeasurementConfig = {
    .ch0DriveStrength = USS_CH0_DRIVE_STRENGHT,
    .ch1DriveStrength = USS_CH1_DRIVE_STRENGHT,
    .sequenceSelection = USS_SEQUENCE_SELECTION,
    .eofSequenceState = USS_EOF_SEQUENCE_SELECTION,
    .pulseConfig = &ussPulseConfig,
    .startPPGCount = USS_START_PPG_COUNT,
    .turnOnADCCount = (uint_least16_t) USS_TURN_ON_ADC_COUNT,
    .startPGAandINBiasCount = USS_PGA_IN_BIAS_COUNT,
    .startADCsamplingCount = (uint16_t)USS_ADC_SAMP_COUNT,
    .restartCaptureCount = (uint_least16_t) USS_RESTART_CAP_COUNT,
    .captureTimeOutCount = USS_TIME_OUT_COUNT,
    .ulpBiasDelay = USS_ULP_BIAS_DELAY,
    .biasImpedance = USS_BIAS_IMPEDANCE,
    .muxChargePumpMode = USS_MUX_CHARGE_PUMP_MODE,
    .restartLowPowerCaptureCount =
        (uint_least16_t) USS_LOW_POWER_RESTART_CAP_COUNT,
#if (USSSWLIB_ENABLE_EXTERNAL_CIRCUITRY_CONTROL == true)
    .startRxEnCount = USS_MEASUREMENT_TURN_ON_EXTERNAL_AMP_COUNT,
#endif
};

static __persistent USS_HSPLL_Configuration ussPLLConfig = {
    .ussXtalFreq = USS_PLL_XTAL_IN_MHZ,
    .pllOutputFreq = USS_HSPLL_FREQ,
    .outputPLLXtal = USS_OUTPUT_PLL_XTAL,
    .HSPLLInputCLKType = USS_HSPLL_INPUT_CLK_TYPE,
    .hspllTolerance = (uint16_t) USS_HSPLL_TOLERANCE,
    .ussXTALsettlingCount = USS_HSPLL_USSXTAL_SETTLING_COUNT,
};

static __persistent USS_Interrupt_Configuration ussInterruptConfig = {
    .enableUUPSPREQIGInterrupt = USS_ENABLE_UUPSPREQIGINTERRUPT,
    .enableSAPHPingTransmitDoneInterrupt = USS_ENABLE_SAPH_PING_TRANSMIT
};

static __persistent USS_Capture_Configuration ussCaptureConfig = {
    .overSampleRate = USS_OVER_SAMPLE_RATE,
    .sampleSize = USS_USER_CONFIG_NUMBER_OF_SAMPLES_PER_CAPTURE,
    .gainRange = USS_GAIN_RANGE,
    .enableWindowHiComp = USS_ENABLE_WINDOW_HI_COMP,
    .enableWindowLoComp = USS_ENABLE_WINDOW_LO_COMP,
    .windowHighThreshold = USS_WINDOW_HIGH_THRESHOLD,
    .windowLowThreshold = USS_WINDOW_LOW_THRESHOLD,
    .agcConstant = USS_AGC_CONSTANT,
    .pCapturesBuffer = &gUSSLEATempMemBlock[0],
    .maxCapture = (sizeof(gUSSLEATempMemBlock) / sizeof(int16_t)),
    .isCapAccumulationEnabled = USS_SW_LIB_ENABLE_ACCUMULATION,
#if USS_SW_LIB_APP_MAX_ACC_BLOCK
    .pAccCaptureBuffer = &gUSSLEARAMReservedAccMemBlock[0],
#endif
};

static __persistent USS_Trigger_Configuration ussTriggerConfig = {
    .triggerConfig = USS_TRIGGER_CONFIG
};

static __persistent USS_Algorithms_User_Configuration ussAlgConfig = {
    .absToFOption              = USS_ALG_ABS_TOF_COMPUTATION,
    .dtoFCalcOption            = (USS_Alg_dToF_Calculation_Option)USS_ALG_DELTA_TOF_COMPUTATION_OPTION,
    .volumeCalOption           = USS_ALG_VOLUME_RATE_COMPUTATION_OPTION,
    .clockRelativeError        = USS_ALG_CLOCK_RELATIVERROR,
    .absTOFInterval            = USS_ALG_ABS_TOF_INTERVAL,
    .dtofVolInterval           = USS_ALG_DTOF_INTERVAL,
    .calibratedDCOffsetUPS     = USS_ALG_CALIBRATED_UPS_DC_OFFSET,
    .calibratedDCOffsetDNS     = USS_ALG_CALIBRATED_DNS_DC_OFFSET,
    .ADCAdditionalCaptureDelay = USS_ALG_ADC_ADDITIONAL_CAP_DLY,
    .dcOffset                  = USS_ALG_DCOFFSET,
    .windowingMode             = USS_ALG_DTOF_WINDOWING_MODE,
#if(USS_ALG_DTOF_WINDOWING_MODE != USS_ALG_DTOF_EST_WINDOW_OPTION_DISABLED)
    .winStartIndexBackOffNumCycles = USS_ALG_WIN_START_INDEX_BACK_OFF_NUM_CYCLES,
    .winTrapzRampOverNumCycles     = USS_ALG_WIN_TRAP_RAMP_NUM_CYCLES,
    .winNumCycles                  = USS_ALG_WIN_NUM_CYCLES,
    .winPeakIndex2EdgeNumCycles    = USS_ALG_WIN_PEAK_INDEX_2_EDGES_NUM_CYLES,
#endif
    .enableFilter = USS_ALG_FILT_IS_FILTER_ENABLED,
#if(USS_ALG_FILT_IS_FILTER_ENABLED == true)
    .filterCoeffs = &gUSSFilterCoeffs[USS_ALG_FILT_OPTION][0],
    .filterLength = (sizeof(gUSSFilterCoeffs[USS_ALG_FILT_OPTION])
                     / sizeof(int16_t)),
#endif

#if defined(USS_ALG_SEARCH_LOBE_SAMP)
     .searchLobeSampleSteps = USS_ALG_SEARCH_LOBE_SAMP,
#else
     .searchLobeSampleSteps = 1,
#endif

#if defined(USS_ALG_NUM_PULS_BEFORE_THRSH)
     .numPulsesBeforeThreshold = USS_ALG_NUM_PULS_BEFORE_THRSH,
#else
     .numPulsesBeforeThreshold = 1,
#endif

#if defined(USS_ALG_RATIO_OF_TRACK_LOBE)
     .ratioOfTrackLobeToPeak = USS_ALG_RATIO_OF_TRACK_LOBE,
#else
     .ratioOfTrackLobeToPeak = 0.11111,
#endif
#if defined(USS_ALG_MAX_RATIO_PEAK_2_PEAK_VAR)
     .maxRatioPeak2PeakVariation = USS_ALG_MAX_RATIO_PEAK_2_PEAK_VAR,
#else
     .maxRatioPeak2PeakVariation = 0.2,
#endif
#if defined(USS_ALG_CORR_VAL_THRSH_CHK_FACT)
     .corrValueThresholdCheckFactor = USS_ALG_CORR_VAL_THRSH_CHK_FACT,
#else
     .corrValueThresholdCheckFactor = 0.05,
#endif
#if defined(USS_ALG_SIGN_VAL_THRSH_CHK)
     .signalValueThresholdCheck = USS_ALG_SIGN_VAL_THRSH_CHK,
#else
     .signalValueThresholdCheck = 100,
#endif
#if defined(USS_ALG_HIGH_FLOW_OPTION)
     .highFlowOption = (USS_highFlow_option)USS_ALG_HIGH_FLOW_OPTION,
#else
     .highFlowOption = (USS_highFlow_option)USS_highFlow_option_disabled,
#endif
#if defined(USS_ALG_CYCLESLIPTHRESHOLD)
     .cycleSlipThreshold = USS_ALG_CYCLESLIPTHRESHOLD,
#else
     .cycleSlipThreshold = 1.1,
#endif

#if defined(USS_ALG_THRESHOLDX1X3)
     .thresholdX1X3 = USS_ALG_THRESHOLDX1X3,
#else
     .thresholdX1X3 = 2000000,
#endif

#if defined(USS_ALG_MAX_SAMPLE_SHIFT)
    .maxSampleShift = USS_ALG_MAX_SAMPLE_SHIFT,
#else
#if ((USS_ALG_ABS_TOF_COMPUTATION_MODE == USS_ALG_ABS_TOF_COMPUTATION_MODE_LOBE_WIDE) || \
     (USS_ALG_ABS_TOF_COMPUTATION_MODE == USS_ALG_ABS_TOF_COMPUTATION_MODE_HILBERT_WIDE))
    .maxSampleShift = 20,
#else
    .maxSampleShift = 6,
#endif
#endif

#if defined(USS_ALG_NUM_CYCLES_SEARCH_CORR)
    .numCycleSearchCorrPeak = USS_ALG_NUM_CYCLES_SEARCH_CORR,
#else
    .numCycleSearchCorrPeak = 2,
#endif
    .pNvMemBuffer = &gNVMemBlock[0],
#if(USS_ALG_DTOF_WINDOWING_MODE != USS_ALG_DTOF_EST_WINDOW_OPTION_DISABLED)
    .pLEAMemBuffer1 = &gUSSLEARAMReservedBlock1[0],
#endif

#if USS_ALG_FILT_IS_FILTER_ENABLED
//    .pLEAMemBuffer2 = &gUSSLEARAMReservedBlock2[0],
    .hilbertTransformCoeffs = &gUSSHilbertCoeffs[USS_HILB_ALG_FILT_OPTION][0],
    .hilbertTransformCoeffsLength =
        (sizeof(gUSSHilbertCoeffs[USS_HILB_ALG_FILT_OPTION])
         / sizeof(int16_t)),
//    .pBinaryPattern = &gUSSBinaryPattern[0],
//    .maxBinaryPatternLenght = (sizeof(gUSSBinaryPattern) / sizeof(int16_t)),
#endif

#if(USS_NUM_OF_ADDTL_TRILL_PULSES>0)
    .pAdditionalBinaryPattern = &additionalBinarySequence[0],
#endif
#ifndef USS_ADDTL_BIN_PATTERN_SIZE
    .additionalPatternLength = 0,
#else
    .additionalPatternLength = USS_ADDTL_BIN_PATTERN_SIZE,
#endif

#if ((USS_ALG_ABS_TOF_COMPUTATION_MODE == USS_ALG_ABS_TOF_COMPUTATION_MODE_HILBERT) || \
     (USS_ALG_ABS_TOF_COMPUTATION_MODE == USS_ALG_ABS_TOF_COMPUTATION_MODE_HILBERT_WIDE) || \
     (USS_ALG_ABS_TOF_COMPUTATION_MODE == USS_ALG_ABS_TOF_COMPUTATION_MODE_LOBE_WIDE))
    .negSearchRange = USS_ALG_ABS_TOF_NEG_SEARCH_RANGE,
    .posSearchRange = USS_ALG_ABS_TOF_POS_SEARCH_RANGE,
#else
    .negSearchRange = -20,
    .posSearchRange = 2,
    .crossThreshold = 20,
#endif

#if ((USS_ALG_ABS_TOF_COMPUTATION_MODE == USS_ALG_ABS_TOF_COMPUTATION_MODE_HILBERT) || \
     (USS_ALG_ABS_TOF_COMPUTATION_MODE == USS_ALG_ABS_TOF_COMPUTATION_MODE_HILBERT_WIDE))
    .crossThreshold = USS_ALG_ABS_TOF_HILB_CROSS_THRESHOLD,
#else
    .crossThreshold = 20,
#endif

#if (defined(USS_ENABLE_ABSTOF_LOBE_OFFSET_CORRECTION) && (USS_ENABLE_ABSTOF_LOBE_OFFSET_CORRECTION == true))
    .pInterpCorrectionTablePtr   = gUSSAbsToFLUT,
    .interpCorrectionTableLenght = sizeof(gUSSAbsToFLUT)/(2*sizeof(int16_t)),
#else
    .pInterpCorrectionTablePtr = NULL,
    .interpCorrectionTableLenght = 0,
#endif

    .tempCalcObject = NULL,
    .temperatureCalcOption = USS_Alg_temperature_Calculation_Option_disabled,

#if (USS_ALG_VFR_CALIB_MODE==USS_ALG_VFR_CALIB_OPTION_FLOW)
    .volumeCalibrationOption = USS_Alg_volume_flow_Calibration_Option_flow,
    .calibObject.flowCalibObject = &ussAlgorithmVolumeFlowObj,
#elif (USS_ALG_VFR_CALIB_MODE==USS_ALG_VFR_CALIB_OPTION_FLOW_TEMPERATURE)
    .volumeCalibrationOption = USS_Alg_volume_flow_Calibration_Option_flow_temperature,
    .calibObject.flowTempCalibObject = &ussAlgorithmVolumeFlowTemperatureObj,
#else
    .volumeCalibrationOption = USS_Alg_volume_flow_Calibration_Option_disabled,
    .calibObject = NULL,
#endif
    .isInitAlgorithms = USS_ALG_IS_INIT_ALGORITHMS,
};

__persistent USS_SW_Library_configuration gUssSWConfig = {
    .systemConfig = &ussSystemConfig,
    .meterConfig = &ussMeterConfig,
    .measurementConfig = &ussMeasurementConfig,
    .pllConfiguration = &ussPLLConfig,
    .captureConfig = &ussCaptureConfig,
    .triggerConfig = &ussTriggerConfig,
    .interruptConfig = &ussInterruptConfig,
    .algorithmsConfig = &ussAlgConfig,
};

// Flag to enable/disable measurement
bool gb_enableMeasure = false;
bool gb_p8interruptFlag = false;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize ultrasonic module.
 */
void us_init(void)
{

    // Initialization of Library with default parameters
        USS_message_code code =
                USS_configureUltrasonicMeasurement(&gUssSWConfig);

#if !DEBUG
        // Invalid capture must report error
        ASSERT(USS_message_code_no_error == code);
#endif

        // Initialize the USS algorithms with default parameters
        gUssSWConfig.algorithmsConfig->isInitAlgorithms = false;
        code = USS_initAlgorithms(&gUssSWConfig);

#if !DEBUG
        // Invalid capture must report error
        ASSERT(USS_message_code_no_error == code);
#endif

        // Disable measure
        gb_enableMeasure = false;


}

/**
 * @brief
 *      Trigger an ultrasonic measurement.
 *
 * @note
 *      This is a precondition for a software-triggered measurement.
 *
 * @param[in]   ui_measurementSetup
 *      Measurement configuration.
 */
void us_startMeasurements(void)
{
    if(gb_enableMeasure)
    {


#ifdef __ENABLE_LPM__
        // Perform a low power measurement, going to LPM3 in between UPS and DNS
        USS_message_code code = USS_startLowPowerUltrasonicCapture(&gUssSWConfig);
#else
        // Perform a regular measurement, staying in LPM0 during capture
        USS_message_code code = USS_startUltrasonicMeasurement(&gUssSWConfig,
                USS_capture_power_mode_active);
#endif

#if !DEBUG_PROC_FLOWRATE
        // Invalid capture must report error
        ASSERT(USS_message_code_no_error == code);
#endif
    }
}

void us_enableMeasurements(void)
{
    gb_enableMeasure = true;
}

void us_disableMeasurements(void)
{
    gb_enableMeasure = false;
}




#if FIXED_POINT_CALC
void us_getMeasurement(USS_Algorithms_Results_fixed_point *const ps_algResFixed)
{
    USS_message_code code = 
        USS_runAlgorithmsFixedPoint(&gUssSWConfig, ps_algResFixed);

    // Check valid measure
    if(!((USS_message_code_valid_results == code)
            || (USS_message_code_algorithm_captures_accumulated == code)))
    {
        ps_algResFixed->iq16Temperature = 0;
        ps_algResFixed->iq16VolumeFlowRate = 0;
        ps_algResFixed->iq16VolumeFlowRateNoCalib = 0;
        ps_algResFixed->iq40TotalTOF_DNS = 0;
        ps_algResFixed->iq40TotalTOF_UPS = 0;
        ps_algResFixed->iq44DeltaTOF = 0;
    }
}

#else
void us_getMeasurement(s_us_meas_t *const ps_measurement, 
    float *const pf_deltaTOF)
{
    USS_message_code code;
    USS_Algorithms_Results_fixed_point algResFixed;
    USS_Algorithms_Results alg_results_float;

    code = USS_runAlgorithmsFixedPoint(&gUssSWConfig, &algResFixed);

    ASSERT((USS_message_code_valid_results == code)
            && (USS_message_code_algorithm_captures_accumulated == code));

    // Convert fixed results to float
    USS_getResultsInFloat(&algResFixed, &alg_results_float);

    ps_measurement->d_ttDownstream = alg_results_float.totalTOF_DNS;
    ps_measurement->d_ttUpstream = alg_results_float.totalTOF_DNS
            + alg_results_float.deltaTOF;
    *pf_deltaTOF = alg_results_float.deltaTOF;
}

#endif

void us_setSamplingRate(double_t d_samplingRate)
{
    gUssSWConfig.systemConfig->measurementPeriod =
            (USS_LFXT_FREQ_IN_HZ / d_samplingRate);
}

bool us_trimGainPGA(void)
{
    bool b_ret = false;

    USS_message_code e_code = USS_calibrateSignalGain(&gUssSWConfig);

    if(USS_message_code_Signal_Gain_Calibration_successful == e_code)
    {
        trimRatioLobe();
        b_ret = true;
    }

    return b_ret;
}

bool us_trimFrequencySwap(void)
{
    bool b_ret = false;
    int16_t *pUPS,*pDNS;
    uint32_t ul_newFreq = 0;
    uint32_t ul_idxFreq;
    double d_maxAvgPeek = 0.0;
    double d_curAvgPeek;
    USS_SW_Library_configuration *ps_config = &gUssSWConfig;

    pUPS = USS_getUPSPtr(ps_config);
    pDNS = USS_getDNSPtr(ps_config);

    for (ul_idxFreq = US_FREQ_TRANSD_IN_HZ - US_FREQ_SWAP_RANGE_IN_HZ;
            ul_idxFreq < US_FREQ_TRANSD_IN_HZ + US_FREQ_SWAP_RANGE_IN_HZ;
            ul_idxFreq += US_FREQ_SWAP_STEP_IN_HZ)
    {
        ps_config->measurementConfig->pulseConfig->F1Frequency =
                US_FREQ_TRANSD_IN_HZ * (uint32_t)US_TRANSFREQ_MULTIPLIER;

        // Update configuration and start a measurement
        USS_updateSDHSConfiguration(ps_config);
        USS_startLowPowerUltrasonicCapture(ps_config);

        d_curAvgPeek = utils_calcSumPositives(pUPS, 
            ps_config->captureConfig->sampleSize);
        d_curAvgPeek += utils_calcSumPositives(pDNS, 
            ps_config->captureConfig->sampleSize);
        d_curAvgPeek /= 2;

        if(d_curAvgPeek > d_maxAvgPeek)
        {
            d_maxAvgPeek  = d_curAvgPeek;
            ul_newFreq = ul_idxFreq;
        }
    }

    if(ul_newFreq)
    {
        // Update with a new frequency
        ps_config->measurementConfig->pulseConfig->F1Frequency = ul_newFreq *
                        (uint32_t)US_TRANSFREQ_MULTIPLIER;

        USS_updateSDHSConfiguration(ps_config);
        USS_updateSAPHConfiguration(ps_config);

        b_ret = true;
    }

    return b_ret;
}


static void trimRatioLobe(void)
{
    int16_t maxMeasurementValueUPS = 0;
    int16_t maxMeasurementValueDNS = 0;
    int16_t firstSignalUPS = 0;
    int16_t firstSignalDNS = 0;
    uint16_t i;
    int16_t *pUPS, *pDNS;
    float f_ratio = 0.0;
    USS_SW_Library_configuration *ps_config = &gUssSWConfig;

    pUPS = USS_getUPSPtr(ps_config);
    pDNS = USS_getDNSPtr(ps_config);

    USS_startLowPowerUltrasonicCapture(ps_config);

    // Find first UPS and DNS above US_FIRST_PEEK_DETECTED_LEVEL signal amplitude
    for (i = 0; i < (ps_config->captureConfig->sampleSize); i++)
    {
        // Check if UPS[i] value is greater than current max value
        if (pUPS[i] > US_FIRST_PEEK_DETECTED_LEVEL)
        {
            firstSignalUPS = pUPS[i];
        }

        // Check if DNS[i] value is greater than current max value
        if (pDNS[i] > US_FIRST_PEEK_DETECTED_LEVEL)
        {
            firstSignalDNS = pDNS[i];
        }

        if(firstSignalUPS && firstSignalDNS)
        {
            break;
        }
    }

    // Find maximum and minimum for UPS and DNS
    for (i = 0; i < (ps_config->captureConfig->sampleSize); i++)
    {
        // Check if UPS[i] value is greater than current max value
        if (pUPS[i] > maxMeasurementValueUPS)
        {
            maxMeasurementValueUPS = pUPS[i];
        }

        // Check if DNS[i] value is greater than current max value
        if (pDNS[i] > maxMeasurementValueDNS)
        {
            maxMeasurementValueDNS = pDNS[i];
        }
    }

    // Find absolute value of maximum and minimum for UPS and DNS
    if (maxMeasurementValueUPS < 0)
    {
        maxMeasurementValueUPS = -1 * maxMeasurementValueUPS;
    }

    // Find absolute value of maximum and minimum for UPS and DNS
    if (maxMeasurementValueDNS < 0)
    {
        maxMeasurementValueDNS = -1 * maxMeasurementValueDNS;
    }

    // Calculate ratio
    f_ratio = (firstSignalUPS + firstSignalDNS) / 2.0;
    f_ratio /= ((maxMeasurementValueUPS + maxMeasurementValueDNS) / 2.0);

    if (f_ratio)
    {

        // Update with a new ratio
        ps_config->algorithmsConfig->ratioOfTrackLobeToPeak = f_ratio;

        USS_updateSDHSConfiguration(ps_config);
        USS_updateSAPHConfiguration(ps_config);
    }
}

double_t utils_calcSumPositives(int16_t *pi_data, uint16_t ui_size)
{
    int i;
    double_t d_sum = 0;

    for (i = 0; i < ui_size; ++i)
    {
        if (pi_data[i] > 0)
        {
            d_sum += pi_data[i];
        }
    }

    return d_sum;
}
