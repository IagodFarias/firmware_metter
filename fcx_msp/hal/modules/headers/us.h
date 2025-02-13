/***************************************************************************//**
 * @brief
 * @file    us.h
 * @author  Robson Cardoso
 * @date    28/02/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef US_H_
#define US_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"
#include "ussSwLib.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define US_LOW_FLOWRATE_ULOOPC          20
#define US_SMARK                        683
#define US_SSPACE                       1076
#define TXPCONFIG                       65535
#define TXPCONFIG0                      15
#define TXPCONFIG1                      15
#define TXPCONFIG2                      15
#define TXPCONFIG3                      15
#define EXC_LENGHT                      12

/** Ultrasonic module default configuration. */
#define US_CONFIG_ALL_DEFAULT                                                   \
{                                                                               \
    .s_usUlConfig =                                                             \
    {                                                                           \
        .ui_ulLoopC                 = US_LOW_FLOWRATE_ULOOPC,                   \
        .ui_ulSmark                 = US_SMARK,  /** 42.69 = 683 us | DN25 5.74e-5s 918  */\
        .ui_ulSSpace                = US_SSPACE, /** 67.24 us | DN25 1420 */    \
        .ui_ulAnaSample             = 80,   /** 5 us */                         \
        .ui_ulPreMeasDelayCal       = 4000, /** 250 us */                       \
        .ui_ulPreMeasDelay0         = 4000, /** 250 us */                       \
        .ui_ulPreMeasDelay1         = 4000, /** 250 us */                       \
        .ui_ulInitPulseDelay        = 16,   /** 1 us */                         \
        .ui_ulLastPulseDelay        = 16,   /** 1 us */                         \
        .ui_ulRefConfig             = 544   /** 34 us */                        \
    },                                                                          \
    .s_usAgConfig =                                                             \
    {                                                                           \
        .ui_agStage1AutoR1          = 3,                                        \
        .ui_agStage1AutoGM1         = 11,                                       \
        .ui_agStage1ConfigMinR1     = 3,                                        \
        .ui_agStage1ConfigMinGM1    = 1,                                        \
        .ui_agStage1ConfigLowest    = 1,    /** Enable lower gain search. */    \
        .ui_agStage1ConfigR1Search  = 1,    /** Enable R1_search. */            \
        .ui_agStage1ConfigGM1Search = 0,    /** Disable GM1_search. */          \
        .ui_agStage2A2              = 3,                                        \
        .ui_agStage2R2              = 2,                                        \
        .ui_agOtherSelHP            = 15,                                       \
        .ui_agOtherSelPadBP         = 0,    /** Disable BP-pin output. */       \
        .ui_agPtrigLev              = 15,                                       \
        .ui_agPtrigLevLow           = 31,                                       \
        .ui_agPtrigLevHigh          = 31                                        \
    },                                                                          \
    .s_usTxConfig =                                                             \
    {                                                                           \
        .ui_txPConfig               = TXPCONFIG,                                \
        .ui_txPConfig0              = TXPCONFIG0,                               \
        .ui_txPConfig1              = TXPCONFIG1,                               \
        .ui_txPConfig2              = TXPCONFIG2,                               \
        .ui_txPConfig3              = TXPCONFIG3                                \
    },                                                                          \
    .s_usOutConfig =                                                            \
    {                                                                           \
        .ui_excLength               = EXC_LENGHT,                               \
        .ui_excConfigSelN70         = 15,                                       \
        .ui_mseqExcLevel            = 15,                                       \
        .ui_mseqOutputEnable        = 15                                        \
    }                                                                           \
}

/** MSEQ default excitation configuration. */
#define US_CONFIG_EXTOUT_MSEQ_DEFAULT                                           \
{                                                                               \
    .ui_excLength                   = 12,                                       \
    .ui_excConfigSelN70             = 1,                                        \
    .ui_mseqExcLevel                = 1,                                        \
    .ui_mseqOutputEnable            = 1                                         \
}

/** TX default excitation configuration. */
#define US_CONFIG_EXTOUT_TX_DEFAULT US_CONFIG_EXTOUT_TX_LOW_FLOWRATE

/** TX excitation configuration for low flow rates. */
#define US_CONFIG_EXTOUT_TX_LOW_FLOWRATE                                        \
{                                                                               \
    .ui_excLength                   = 10,                                        \
    .ui_excConfigSelN70             = 15,                                        \
    .ui_mseqExcLevel                = 15,                                        \
    .ui_mseqOutputEnable            = 15                                         \
}

/** TX excitation configuration for high flow rates. */
#define US_CONFIG_EXTOUT_TX_HIGH_FLOWRATE                                       \
{                                                                               \
    .ui_excLength                   = 12,                                       \
    .ui_excConfigSelN70             = 1,                                        \
    .ui_mseqExcLevel                = 15,                                        \
    .ui_mseqOutputEnable            = 15                                         \
}



/*******************************************************************************
 * Structures
 ******************************************************************************/

/** Loop controller configuration structure. */
typedef struct S_US_UL_CONFIG
{
    /** The number of sing around loops to use in one ultrasonic measurement. */
    uint16_t ui_ulLoopC;
    /** The beginning of the time frame when the signal should appear expressed
        in 16MHz clock cycles from excitation. No signal is expected during this
        time. */
    uint16_t ui_ulSmark;
    /** The end of the time frame when the signal should appear expressed in
        16MHz clock cycles from ui_ulSmark. If the signal has not been detected
        during this time, a time-out is issued. */
    uint16_t ui_ulSSpace;
    /** This is the time when the signal zero level is captured expressed in
        16MHz clock cycles from ui_ulSmark. AGC needs at lest 5us for
        stabilization. */
    uint16_t ui_ulAnaSample;
    /** Pre-measurement delay for calibration expressed in 16MHz clock cycles.
        This is the time before the ref_clk is turned on. */
    uint16_t ui_ulPreMeasDelayCal;
    /** Pre-measurement delay for measurement direction 0 expressed in 16MHz
        clock cycles. This is the time between start and when the ref_clk is
        turned on. */
    uint16_t ui_ulPreMeasDelay0;
    /** Pre-measurement delay for measurement direction 1 expressed in 16MHz
        clock cycles. This is the time between measurement in direction 0 and
        when ref_clk is turned on for measurement in direction 1. */
    uint16_t ui_ulPreMeasDelay1;
    /** A delay expressed in 16MHz clock cycles to let the driver stabilize
     *  before excitation is triggered. */
    uint16_t ui_ulInitPulseDelay;
    /** A delay expressed in 16MHz clock cycles after the last received pulse to
        let the UFO evaluate PTRIG_HIGH and PTRIG_LOW levels before the
        amplifier becomes inactive. */
    uint16_t ui_ulLastPulseDelay;
    /** A delay expressed in 16MHz clock cycles to let the voltage and current
        references stabilize before starting the measurement sequence. */
    uint16_t ui_ulRefConfig;
} s_us_ul_config_t;

/** Automatic gain controller configuration structure. */
typedef struct S_US_AG_CONFIG
{
    /** Defines the gain in stage one together with AG_STAGE1_AUTO_GM1. It can
        be set manually but if an AGC is issued the value might change. */
    uint16_t ui_agStage1AutoR1;
    /** Defines the gain in stage one together with AG_STAGE1_AUTO_R1. It can be
        set manually, but if AG_STAGE1_AUTO_R1 reaches its maximum or minimum
        value during an AGC calibration the AG_STAGE1_AUTO_GM1 will change. */
    uint16_t ui_agStage1AutoGM1;
    /** The minimum value for R1 during R1_search. */
    uint16_t ui_agStage1ConfigMinR1;
    /** The minimum value for GM1 during Gm1_search. */
    uint16_t ui_agStage1ConfigMinGM1;
    /** If set to 1 searches for lowest gain where signal amplitude is greater
     *  than ptrig_low. If set to 0, search is off. */
    uint16_t ui_agStage1ConfigLowest;
    /** Enable/disable R1_search during AGC. */
    uint16_t ui_agStage1ConfigR1Search;
    /** Enable/disable GM1_search during AGC. */
    uint16_t ui_agStage1ConfigGM1Search;
    /** Defines the gain in stage two together with AG_STAGE2_R2. It will stay
        fixed through an AGC calibration. */
    uint16_t ui_agStage2A2;
    /** Defines the gain in stage two together with AG_STAGE2_A2. It will stay
        fixed through an AGC calibration. */
    uint16_t ui_agStage2R2;
    /** Select high pass frequency for the bandpass filter. Allowed values are
        1-15 and the frequency is proportional to the code value of the inverted
        bit series. */
    uint16_t ui_agOtherSelHP;
    /** Toggles whether or not the signal is output on the BP-pins. If set to 4
        the signal is output. If set to 0 it is not output. */
    uint16_t ui_agOtherSelPadBP;
    /** AG_PTRIGLEV is defining the threshold value for the signal. When the
        signal amplitude exceeds AG_PTRIGLEV the time measurement functions is
        triggered. AG_PTRIGLEV should be set below PTRIG_LOW and higher than
        the signal noise. */
    uint16_t ui_agPtrigLev;
    /** AG_PTRIGLEV_LOW defines the threshold value used by the AGC. It should
        be larger than AG_PTRIGLEV. */
    uint16_t ui_agPtrigLevLow;
    /** AG_PTRIGLEV_HIGH defines the last threshold value. It should be equal to
        or higher than AG_PTRIGLEV_LOW. */
    uint16_t ui_agPtrigLevHigh;
    /** Fine tuning of TX settings. Normally not necessary to do, but if needed
        it can set PCONFIG for all channels. */
} s_us_ag_config_t;

/** Transducer controller configuration structure. */
typedef struct S_US_TX_CONFIG
{
    uint16_t ui_txPConfig;
    /** Set PCONFIG for channel 0. */
    uint16_t ui_txPConfig0;
    /** Set PCONFIG for channel 1. */
    uint16_t ui_txPConfig1;
    /** Set PCONFIG f or channel 2. */
    uint16_t ui_txPConfig2;
    /** Set PCONFIG for channel 3. */
    uint16_t ui_txPConfig3;
} s_us_tx_config_t;

/** Excitation configuration structure. */
typedef struct S_US_OUT_CONFIG
{
    /** Length of excitation pulse in 100MHz clock cycles. */
    uint16_t ui_excLength;
    /** Configure transducer impedance. If set to 0, selects 5 Ohm. If set to 5,
        selects 70 Ohm. */
    uint16_t ui_excConfigSelN70;
    /** Determines if the excitation pulse is active low or high. If set to 0,
        the excitation pulse is active low. If set to 1, the excitation pulse is
        active high. */
    uint16_t ui_mseqExcLevel;
    /** Enable TX or MSEQ. If set to 0, enables TX output. If set to 1, enables
        MSEQ output. */
    uint16_t ui_mseqOutputEnable;
} s_us_out_config_t;

/** Stage 2 trimming result structure. */
typedef struct S_US_STAGE2_t
{
    /** Defines the gain in stage two together with AG_STAGE2_A2. It will stay
        fixed through an AGC calibration. */
    uint16_t ui_r2;
    /** Defines the gain in stage two together with AG_STAGE2_R2. It will stay
        fixed through an AGC calibration. */
    uint16_t ui_a2;
} s_us_stage2_t;

/** PTRIG trimming result structure. */
typedef struct S_US_PTRIG_t
{
    /** AG_PTRIGLEV is defining the threshold value for the signal. When the
        signal amplitude exceeds AG_PTRIGLEV the time measurement functions is
        triggered. AG_PTRIGLEV should be set below PTRIG_LOW and higher than
        the signal noise. */
    uint16_t ui_ptrig;
    /** Begin of the stability region. */
    uint16_t ui_stableRegionBegin;
    /** End of the stability region. */
    uint16_t ui_stableRegionEnd;
} s_us_ptrig_t;

/** Ultrasonic module configuration structure. */
typedef struct S_US_CONFIG
{
    /** Loop controller configuration. */
    s_us_ul_config_t s_usUlConfig; // 20 bytes
    /** Automatic gain configuration. */
    s_us_ag_config_t s_usAgConfig; // 28 Bytes
    /** Transducer configuration. */
    s_us_tx_config_t s_usTxConfig; // 10 bytes
    /** Excitation configuration. */
    s_us_out_config_t s_usOutConfig; // 8 Bytes
} s_us_config_t;

/** Measurement structure. */
typedef struct S_US_MEAS
{
    /** Number of internal clocks per reference clock after a high frequency
        clock calibration. */
    double_t d_hfc;
    /** Gain R1 at stage 1. */
    uint16_t ui_r1;
    /** Gain GM1 at state 1. */
    uint16_t ui_gm1;
    /** Status after an AGC. */
    uint16_t ui_agStatus;
    /** Transit time in upstream direction [s]. */
    double_t d_ttUpstream;
    /** Transit time in downstream direction [s]. */
    double_t d_ttDownstream;
} s_us_meas_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void us_init(void);
void us_enableMeasurements(void);
void us_disableMeasurements(void);
void us_startMeasurements(void);
#if FIXED_POINT_CALC
void us_getMeasurement(USS_Algorithms_Results_fixed_point *const ps_algResFixed);
#else
void us_getMeasurement(s_us_meas_t *const ps_measurement,
    float *const pf_deltaTOF);
#endif
void us_setSamplingRate(double_t d_samplingRate);
bool us_trimGainPGA(void);
bool us_trimFrequencySwap(void);
double_t utils_calcSumPositives(int16_t *pi_data, uint16_t ui_size);


/*******************************************************************************
 * Inline function
 ******************************************************************************/

#endif /* US_H_ */
