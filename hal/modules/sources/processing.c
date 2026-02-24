
/*******************************************************************************
 * @brief
 * @file    processing.c
 * @author  Robson Cardoso
 * @date    02/03/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdbool.h>

#include "general_settings.h"
#include "processing.h"
#include "utils_fcx.h"
#include "configmng.h"
#include "mov_avg.h"

#include <string.h>

/* --------------------------------------------------------------------------
 * FACTORY RESET SIMPLES – LIMPA VARIÁVEIS PERSISTENTES
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** Minimum allowed transit time value in seconds. */
#define PROC_TT_MIN                              0.0L
#if FIXED_POINT_CALC
#define _IQ44toIQ40(A)                           ((int32_t)(A) >> (44 - 40))
#endif

#define PROC_PROCESSING_DEFAULT                                                 \
{                                                                               \
    .d_speedOfSound                    = 0.0,                                   \
    .d_uncalibVel                      = 0.0,                                   \
    .d_temperature                     = 0.0,                                   \
    .d_viscosity                       = 0.0,                                   \
    .d_reynolds                        = 0.0,                                   \
    .d_calibConst                      = 0.0,                                   \
    .d_calibVel                        = 0.0,                                   \
    .d_currentFlowRate                 = 0.0,                                   \
    .d_minFlowRate                     = PROC_MIN_FLOWRATE_DEFAULT,             \
    .d_maxFlowRate                     = PROC_MAX_FLOWRATE_DEFAULT,             \
    .d_volume                          = 0.0,                                   \
    .d_forwardVolume                   = 0.0,                                   \
    .d_reverseVolume                   = 0.0,                                   \
}

#define FIXED_POINT_PROC_PROCESSING_DEFAULT                                   \
{                                                                             \
    .iq40DNS                               = 0,                               \
    .iq40UPS                               = 0,                               \
    .iq18Sos                               = 0,                               \
    .iq26uncalibVel                        = 0,                               \
    .iq30PathDiameter                      = 0,                               \
    .iq30PathConst                         = 0,                               \
    .iq30VelSOSConst                       = 0,                               \
    .iq40DSOS                              = 0,                               \
    .iq44DZC                               = 0,                               \
    .iq24Temperature                       = 0,                               \
    .iq30Viscosity                         = 0,                               \
    .iq12Reynolds                          = 0,                               \
    .iq29CalibConst                        = 0,                               \
    .iq26CalibVel                          = 0,                               \
    .iq18FlowRate                          = 0,                               \
    .iq18minFlowRate                       = _IQ18(2000.0),                   \
    .iq18maxFlowRate                       = 0,                               \
    .iq30PathArea                          = 0,                               \
    .iq30Volume                            = 0,                               \
    .iq28SamplingRate                      = 0,                               \
}



#if defined(__TI_COMPILER_VERSION__)
#pragma PERSISTENT(gs_processing)
#define __persistent
#elif defined (__IAR_SYSTEMS_ICC__)
#else
#error Compiler not supported!
#endif

/*******************************************************************************
 * Structures
 ******************************************************************************/
#if FIXED_POINT_CALC
typedef struct S_IQ_RESULT
{
    int32_t iq40DNS;
    int32_t iq40UPS;
    int32_t iq18Sos;
    int32_t iq26uncalibVel;
    int32_t iq30PathDiameter;
    int32_t iq30PathConst;
    int32_t iq30VelSOSConst;
    int32_t iq40DSOS;
    int32_t iq44DZC;
    int32_t iq24Temperature;
    int32_t iq30Viscosity;
    int32_t iq12Reynolds;
    int32_t iq29CalibConst;
    int32_t iq26CalibVel;
    int32_t iq18FlowRate;
    int32_t iq18minFlowRate;
    int32_t iq18maxFlowRate;
    int32_t iq30PathArea;
    int32_t iq30Volume;
    int32_t iq28SamplingRate;

} s_iq_result_t;
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if FIXED_POINT_CALC
static int32_t calcSpeedOfSound(int32_t const iq40ttDownstream,
                                  int32_t const iq40ttUpstream,
                                  int32_t const iq30VelSosConst,
                                  int32_t const iq40delaySpeedOfSound);
static int32_t calcVelocity(int32_t const iq40TotalTOF_DNS,
                                  int32_t const iq40TotalTOF_UPS,
                                  int32_t const iq30PathConst,
                                  int32_t const iq40DSOS,
                                  int32_t const iq44DZC,
                                  int32_t const iq44DeltaTOF);
static int32_t calcTemperature(int32_t const iq18speedOfSound);
static int32_t calcViscosity(int32_t const iq24Temperature);
static int32_t calcReynolds(int32_t const iq26Velocity,
                             int32_t const iq30FlowPathDiameter,
                             int32_t const iq30kinematicViscosity);
static int32_t calcAbs(int32_t const d_value);
static int32_t calcCalibConst(int32_t const iq12Reynolds);
static int32_t calcCalibVelocity(int32_t const iq26uncalibVel,
                                  int32_t const iq29CalibConst);
static int32_t calcFlowRate(int32_t const _velocity,
                             int32_t const d_flowPathArea);
static int32_t calcVolume(int32_t const iq26CalibVel,
                           int32_t const iq30PathArea,
                           int32_t const iq28SamplingRate);
static void updateFixedPointers(void);

#else
static double_t calcSpeedOfSound(double_t const d_ttDownstream,
                                 double_t const d_ttUpstream,
                                 double_t const d_flowPathLength,
                                 double_t const d_delaySpeedOfSound);
static double_t calcVelocityBySos(double_t const d_ttDownstream,
                             double_t const d_ttUpstream,
                             double_t const d_flowPathLength,
                             double_t const d_delaySpeedOfSound,
                             double_t const d_delayZeroCalibration,
                             double_t const d_dTof);
static double_t calcTemperature(double_t const d_speedOfSound);
static double_t calcViscosity(double_t const d_temperature);
static double_t calcReynolds(double_t const d_velocity,
                             double_t const d_flowPathDiameter,
                             double_t const d_kinematicViscosity);
static double_t calcAbs(double_t const d_value);
static double_t calcCalibConst(double_t const d_reynolds);
static double_t calcCalibVelocity(double_t const d_velocity,
                                  double_t const d_calibConst);
static double_t calcFlowRate(double_t const d_velocity,
                             double_t const d_flowPathArea);
static double_t calcVolume(double_t const d_velocity,
                           double_t const d_flowPathArea,
                           double_t const d_samplingRate);
#endif

#if CONFIG_LOGGER_ENABLED
STATIC void updateMinFlowRate();
STATIC void updateMaxFlowRate();
#endif

/*******************************************************************************
 * Global variables
 ******************************************************************************/
__persistent static s_processing_t gs_processing = PROC_PROCESSING_DEFAULT;

/** Show the relationship between the speed of sound in meters per second and
    the water temperature in °C. */
static const s_lut_coordinate_t gas_lutSosToTemperature[PROC_SOS_TO_TEMP_LUT_SIZE] =
{
    {1.40238743999999997e+03, 0.0},
    {1.44727957965388487e+03, 10.0},
    {1.48235829856431997e+03, 20.0},
    {1.49670444077783191e+03, 25.0},
    {1.50914502152605496e+03, 30.0},
    {1.51982681204078722e+03, 35.0},
    {1.52888147289023982e+03, 40.0},
    {1.54256760639062486e+03, 50.0},
    {1.55099953496975991e+03, 60.0},
    {1.55480746060519482e+03, 70.0},
    {1.55515184104860759e+03, 74.0}
};

/** Show the relationship between the media temperature in ÂºC and the kinematic
    viscosity of the water in squared meters per second. */
static const s_lut_coordinate_t gas_lutTemperatureToViscosity[PROC_TEMP_TO_VISC_LUT_SIZE] =
{
    {0.0,   1.79199999999999995e-06},
    {10.0,  1.30800000003907788e-06},
    {20.0,  1.00400000064242392e-06},
    {25.0,  8.95526790358651460e-07},
    {30.0,  8.05000002693098820e-07},
    {35.0,  7.27499908689935765e-07},
    {40.0,  6.61000006960899082e-07},
    {50.0,  5.56000023488240412e-07},
    {60.0,  4.77000105337524531e-07},
    {70.0,  4.17000479478736383e-07},
    {74.0,  3.96901043662674571e-07}
};

/** Flags if processing is enabled. */
static bool gb_enableProcessing = true;
static double_t gd_samplingRate = PROC_SAMPLING_RATE_DEFAULT;
static s_sys_config_t *gps_sysConfig;

/** Reverse flow flag. */
static bool gb_isReverseFlow;

/** Air in pipe flag. */
static bool gb_isAirInPipe;

/** Flags if filters are enabled. */
static bool gb_enableFilters;

/** Uncalibrated velocity moving average filter. */
static s_mov_avg_filter_t gs_uncalibVelMovAvgFilter;

/** Speed of sound moving average filter. */
static s_mov_avg_filter_t gs_sosMovAvgFilter;

#if CONFIG_VARIABLE_US
/** Reverse Change us config flag. */
static bool gb_isChangeUsConfigHighFlowRate;
#endif

#if DEBUG_FLOWRATE
static double_t gd_fakeFlowRate;
#endif /* DEBUG_FLOWRATE */

/** Flag to select the velocity calculation method. */
#if FIXED_POINT_CALC
__persistent static s_iq_result_t gs_iqVariables = FIXED_POINT_PROC_PROCESSING_DEFAULT;
#endif

#if DEBUG_PROC_FLOWRATE
static int8_t gc_fakeFlow = 1;
#endif

/*******************************************************************************
 * Public Functions
 ******************************************************************************/
void proc_init(void)
{
    // Get system configuration pointer
    gps_sysConfig = configmng_getSysConfig();

    // Clear reverse flow flag
    gb_isReverseFlow = false;

    // Clear air in pipe flag
    gb_isAirInPipe = false;

    // Initialize moving average filters
    mov_avg_init(&gs_uncalibVelMovAvgFilter);
    mov_avg_init(&gs_sosMovAvgFilter);

    // Disable filters
    gb_enableFilters = false;

    // Disable processing
    gb_enableProcessing = false;

#if DEBUG_FLOWRATE
    // Initialize the fake flow rate
    gd_fakeFlowRate = 0.0;
#endif /* DEBUG_FLOWRATE */

#if FIXED_POINT_CALC
    updateFixedPointers();
#endif

#if DEBUG_PROC_FLOWRATE
    gc_fakeFlow = 1;
#endif

}

double_t proc_configReadSamplingRate(void)
{
    return gd_samplingRate;
}

void proc_update()
{
    if (gb_enableProcessing)
    {
        us_startMeasurements();

#if FIXED_POINT_CALC
        USS_Algorithms_Results_fixed_point algResFixed;

        us_getMeasurement(&algResFixed);

#if DEBUG_PROC_FLOWRATE
        algResFixed.iq40TotalTOF_DNS = 0;
        algResFixed.iq40TotalTOF_UPS = 0;
        
        switch (gc_fakeFlow)
        {
            case 0:
               algResFixed.iq44DeltaTOF = 0;
            break;
            case 1:
               algResFixed.iq44DeltaTOF = 0;
            break;
            case 2:
               algResFixed.iq44DeltaTOF = 0;
            break;
            case 3:
            default:
               algResFixed.iq44DeltaTOF = 0;
            break;
        }
#endif

        if ((algResFixed.iq40TotalTOF_DNS > PROC_TT_MIN)
                && (algResFixed.iq40TotalTOF_UPS > PROC_TT_MIN))
#else
        static float f_deltaTOF = 0;
        us_getMeasurement(&gs_processing.s_usMeasurement, &f_deltaTOF);
        if((gs_processing.s_usMeasurement.d_ttDownstream > PROC_TT_MIN)
                && (gs_processing.s_usMeasurement.d_ttUpstream > PROC_TT_MIN))
#endif
        {
#if FIXED_POINT_CALC
            // Fix upstream
            algResFixed.iq40TotalTOF_UPS =
            _IQ44toIQ40(algResFixed.iq44DeltaTOF)
                    + algResFixed.iq40TotalTOF_DNS;

            gs_iqVariables.iq40DNS = algResFixed.iq40TotalTOF_DNS;
            gs_iqVariables.iq40UPS = algResFixed.iq40TotalTOF_UPS;

            gs_iqVariables.iq18Sos = calcSpeedOfSound(
                    algResFixed.iq40TotalTOF_DNS, algResFixed.iq40TotalTOF_UPS,
                    gs_iqVariables.iq30VelSOSConst, gs_iqVariables.iq40DSOS);
#else
            gs_processing.d_speedOfSound = calcSpeedOfSound(
                    gs_processing.s_usMeasurement.d_ttDownstream,
                    gs_processing.s_usMeasurement.d_ttUpstream,
                    gps_sysConfig->s_mechanical.d_flowPathLength,
                    gps_sysConfig->s_calibration.d_delaySpeedOfSound);
#endif

#if FIXED_POINT_CALC
            gs_iqVariables.iq26uncalibVel = calcVelocity(
                    algResFixed.iq40TotalTOF_DNS, algResFixed.iq40TotalTOF_UPS,
                    gs_iqVariables.iq30PathConst, gs_iqVariables.iq40DSOS,
                    gs_iqVariables.iq44DZC, algResFixed.iq44DeltaTOF);
#else
            gs_processing.d_uncalibVel = calcVelocityBySos(
                    gs_processing.s_usMeasurement.d_ttDownstream,
                    gs_processing.s_usMeasurement.d_ttUpstream,
                    gps_sysConfig->s_mechanical.d_flowPathLength,
                    gps_sysConfig->s_calibration.d_delaySpeedOfSound,
                    gps_sysConfig->s_calibration.d_delayZeroCalibration, f_deltaTOF);
#endif

#if CONFIG_VARIABLE_US
            // printf("(PROC) ULOOP %d\n", gps_sysConfig->s_ultrasonic.s_usUlConfig.ui_ulLoopC);
            // Conditional that indicates if us_config should be changed
            if(US_MAX_VELOCITY_LOW_FLOWRATES <= gs_processing.d_uncalibVel)
            {
                // Sets flag to change us measurement configuration on configmng.c
                gb_isChangeUsConfigHighFlowRate = true;
            }
            else
            {
                // In case the velocity is lower, setup must be for lower flowrates
                gb_isChangeUsConfigHighFlowRate = false;
            }
#endif
        }
        else
        {
#if FIXED_POINT_CALC
            gs_iqVariables.iq18Sos = 0;
            gs_iqVariables.iq26uncalibVel = 0;
#else
            gs_processing.d_speedOfSound = 0.0;
            gs_processing.d_uncalibVel = 0.0;
#endif
        }

        // Apply filters in case they are enabled
        if (gb_enableFilters)
        {
#if FIXED_POINT_CALC
            gs_iqVariables.iq26uncalibVel = mov_avg_update(
                    &gs_uncalibVelMovAvgFilter, gs_iqVariables.iq26uncalibVel);
            gs_iqVariables.iq18Sos = mov_avg_update(&gs_sosMovAvgFilter,
                                                    gs_iqVariables.iq18Sos);
#else
            gs_processing.d_uncalibVel = mov_avg_update(
                    &gs_uncalibVelMovAvgFilter, gs_processing.d_uncalibVel);
            gs_processing.d_speedOfSound = mov_avg_update(
                    &gs_sosMovAvgFilter, gs_processing.d_speedOfSound);
#endif
        }
        // If the speed of sound is greater than zero, it means the pipe is fully
        // filled with water. If this is the case we should calculate water's
        // temperature and fluid calibrated velocity
#if FIXED_POINT_CALC
        if (gs_iqVariables.iq18Sos > 0.0)
#else
        if (gs_processing.d_speedOfSound > 0.0)
#endif
        {
#if FIXED_POINT_CALC
            gs_iqVariables.iq24Temperature = calcTemperature(
                    gs_iqVariables.iq18Sos);

            gs_iqVariables.iq30Viscosity = calcViscosity(
                    gs_iqVariables.iq24Temperature);

            gs_iqVariables.iq12Reynolds = calcReynolds(
                    gs_iqVariables.iq26uncalibVel,
                    gs_iqVariables.iq30PathDiameter,
                    gs_iqVariables.iq30Viscosity);

            gs_iqVariables.iq29CalibConst = calcCalibConst(
                    calcAbs(gs_iqVariables.iq12Reynolds));

            gs_iqVariables.iq26CalibVel = calcCalibVelocity(
                    gs_iqVariables.iq26uncalibVel,
                    gs_iqVariables.iq29CalibConst);
#else
        gs_processing.d_temperature = calcTemperature(
                gs_processing.d_speedOfSound);

        gs_processing.d_viscosity = calcViscosity(gs_processing.d_temperature);

        gs_processing.d_reynolds = calcReynolds(
                gs_processing.d_uncalibVel,
                gps_sysConfig->s_mechanical.d_flowPathDiameter,
                gs_processing.d_viscosity);

        gs_processing.d_calibConst = calcCalibConst(
                calcAbs(gs_processing.d_reynolds));

#if DEBUG_PRINT_CONST
            printf("(PROC) K %f\n", gps_sysConfig->s_ultrasonic.s_usUlConfig.ui_ulLoopC);
#endif
        gs_processing.d_calibVel = calcCalibVelocity(
                gs_processing.d_uncalibVel, gs_processing.d_calibConst);
#endif

            // If the calibrated velocity is different than zero, we should update
            // volume totalizers
#if FIXED_POINT_CALC
            if (gs_iqVariables.iq26CalibVel != 0)
#else
        if (gs_processing.d_calibVel != 0.0)
#endif
            {
#if FIXED_POINT_CALC
            gs_iqVariables.iq18FlowRate = calcFlowRate(
                    gs_iqVariables.iq26CalibVel,
                    gs_iqVariables.iq30PathArea);
#else
            gs_processing.d_currentFlowRate = calcFlowRate(
                    gs_processing.d_calibVel,
                    gps_sysConfig->s_mechanical.d_flowPathArea);
#endif
#if FIXED_POINT_CALC
            gs_iqVariables.iq30Volume = calcVolume(
                    gs_iqVariables.iq26CalibVel,
                    gs_iqVariables.iq30PathArea,
                    gs_iqVariables.iq28SamplingRate);
#else
            gs_processing.d_volume = calcVolume(
                    gs_processing.d_calibVel,
                    gps_sysConfig->s_mechanical.d_flowPathArea,
                    gd_samplingRate);
#endif

#if FIXED_POINT_CALC
                // Detect reverse flow and update totalizers
                if (gs_iqVariables.iq18FlowRate < 0)
#else
            if (gs_processing.d_currentFlowRate < 0)
#endif
                {
#if FIXED_POINT_CALC
                    gs_processing.d_reverseVolume +=
                        (double_t) _IQ30toF(gs_iqVariables.iq30Volume);
#else
                gs_processing.d_reverseVolume += gs_processing.d_volume;
#endif
                    gb_isReverseFlow = true;
                }
                else
                {
#if FIXED_POINT_CALC
                    gs_processing.d_forwardVolume += (double_t) _IQ30toF(
                            gs_iqVariables.iq30Volume);
#else
                gs_processing.d_forwardVolume += gs_processing.d_volume;
#endif
                    gb_isReverseFlow = false;
                }
            }
            else
            {
#if FIXED_POINT_CALC
                // Clear flow rate
                gs_iqVariables.iq18FlowRate = 0;

                // Clear volume
                gs_iqVariables.iq30Volume = 0;
#else
            // Clear flow rate
            gs_processing.d_currentFlowRate = 0.0;

            // Clear volume
            gs_processing.d_volume = 0.0;
#endif
                // Clear reverse flow flag
                gb_isReverseFlow = false;
            }

            // Clear air in pipe flag
            gb_isAirInPipe = false;
        }
        else
        {
            // Set air in pipe flag
            gb_isAirInPipe = true;

            // Clear reverse flow flag
            gb_isReverseFlow = false;

#if FIXED_POINT_CALC
            // Clear temperature
            gs_iqVariables.iq24Temperature = 0;

            // Clear flow rate
            gs_iqVariables.iq18FlowRate = 0;
#else
        // Clear temperature
        gs_processing.d_temperature = 0.0;

        // Clear flow rate
        gs_processing.d_currentFlowRate = 0.0;
#endif
        }
    }
#if CONFIG_LOGGER_ENABLED
            // Update the minimum and maximum flow rates
            updateMinFlowRate();
            updateMaxFlowRate();
#endif
}

/**
 * @brief
 *      Enable processing.
 */
void proc_enableProcessing(void)
{
    gb_enableProcessing = true;
}

/**
 * @brief
 *      Disable processing.
 */
void proc_disableProcessing(void)
{
    gb_enableProcessing = false;
}

s_processing_t* proc_getResults(void)
{
#if FIXED_POINT_CALC
    gs_processing.s_usMeasurement.d_ttDownstream = utils_fixedToDouble(
            gs_iqVariables.iq40DNS, 40);
    gs_processing.s_usMeasurement.d_ttUpstream = utils_fixedToDouble(
            gs_iqVariables.iq40UPS, 40);

    gs_processing.d_uncalibVel = _IQ26toF(gs_iqVariables.iq26uncalibVel);
    gs_processing.d_calibVel = _IQ26toF(gs_iqVariables.iq26CalibVel);
    gs_processing.d_currentFlowRate = _IQ18toF(gs_iqVariables.iq18FlowRate);
    gs_processing.d_volume = _IQ30toF(gs_iqVariables.iq30Volume);
#endif

    return &gs_processing;
}


double_t proc_getNetVolume(void)
{
    return (gs_processing.d_forwardVolume + gs_processing.d_reverseVolume);
}


double_t proc_getCurrentFlowRate(void)
{
#if FIXED_POINT_CALC
    return gs_processing.d_currentFlowRate = _IQ18toF(gs_iqVariables.iq18FlowRate);
#else
    return gs_processing.d_currentFlowRate;
#endif
}

double_t proc_getTemperature(void)
{
#if FIXED_POINT_CALC
    return gs_processing.d_temperature = _IQ24toF(gs_iqVariables.iq24Temperature);
#else
    return gs_processing.d_temperature;
#endif
}

double_t proc_getReverseVolume(void)
{
    return gs_processing.d_reverseVolume;
}

double_t proc_getForwardVolume(void)
{
    return gs_processing.d_forwardVolume;
}

double_t proc_getMaxFlowRate(void)
{
    return gs_processing.d_maxFlowRate;
}

double_t proc_getMinFlowRate(void)
{
    return gs_processing.d_minFlowRate;
}

void proc_resetFlowRates(void)
{
    gs_processing.d_minFlowRate = 0.0;
    gs_processing.d_maxFlowRate = 0.0;
}

bool proc_isReverseFlow(void)
{
    return gb_isReverseFlow;
}

bool proc_isAirInPipe(void)
{
    return gb_isAirInPipe;
}

void proc_enableFilters(void)
{
    gb_enableFilters = true;
}

void proc_disableFilters(void)
{
    gb_enableFilters = false;
}

void proc_resetVolumeCounters(void)
{
    gs_processing.d_forwardVolume = 0.0;
    gs_processing.d_reverseVolume = 0.0;
}

void proc_configWriteSamplingRate(double_t d_samplingRate)
{
#if FIXED_POINT_CALC
    gs_iqVariables.iq28SamplingRate = _IQ28(d_samplingRate);
#else
    gd_samplingRate = d_samplingRate;
#endif
    us_setSamplingRate(d_samplingRate);
}

void proc_updateConfig(void)
{
#if FIXED_POINT_CALC
    updateFixedPointers();
#endif
}

#if FIXED_POINT_CALC
int32_t proc_getVolFixedPoint(void)
{
    return gs_iqVariables.iq30Volume;
}

int32_t proc_getRevVolFixedPoint(void)
{
    return gs_iqVariables.iq30Volume;
}
#endif



#if DEBUG_PROC_FLOWRATE
void proc_toogleFlowRate(void)
{
    switch (gc_fakeFlow)
    {
        case 0:
        case 1:
            break;
        case 2:
            gc_fakeFlow++;
        break;
        default:
        case 3:
            gc_fakeFlow = 0;
        break;
    }
}
#endif

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
#if FIXED_POINT_CALC
static int32_t calcSpeedOfSound(int32_t const iq40ttDownstream,
                                  int32_t const iq40ttUpstream,
                                  int32_t const iq30VelSosConst,
                                  int32_t const iq40delaySpeedOfSound)
{
    int32_t iq18Sos;

    iq18Sos = iq40ttUpstream + iq40ttDownstream;              // IQ40
    iq18Sos -= iq40delaySpeedOfSound;                         // IQ40
    iq18Sos -= iq40delaySpeedOfSound;                         // IQ40
    iq18Sos = _IQ28div(iq30VelSosConst, iq18Sos);

    return iq18Sos;
}

static int32_t calcVelocity(int32_t const iq40TotalTOF_DNS,
                             int32_t const iq40TotalTOF_UPS,
                             int32_t const iq30PathConst,
                             int32_t const iq40DSOS,
                             int32_t const iq44DZC,
                             int32_t const iq44DeltaTOF)
{
    int32_t iq26Vel = 0;
    int32_t iq49Den = 0;
    int32_t iq40denConst = 0;

    iq40denConst = iq40TotalTOF_DNS; // IQ40
    iq40denConst -= iq40DSOS; // IQ40
    iq40denConst += _IQ44toIQ40(iq44DZC); // IQ40
    
    iq26Vel = iq44DeltaTOF; // IQ44
    iq26Vel -= iq44DZC; // IQ44
    iq26Vel -= iq44DZC; // IQ44

    iq49Den = (int32_t)(((int64_t)iq40denConst * (int64_t)iq40TotalTOF_UPS) >> 31); // IQ49
    iq49Den -= (int32_t)(((int64_t)iq40denConst * (int64_t)iq40DSOS) >> 31); // IQ49
    iq49Den -= (int32_t)(((int64_t)iq40denConst * (int64_t)iq44DZC) >> 35); // IQ49

    iq26Vel = (int32_t)(((int64_t)iq26Vel * (int64_t)iq30PathConst) >> 26); // IQ48

    return _IQ27div(iq26Vel, iq49Den); // IQ26
}

static int32_t calcTemperature(int32_t const iq18speedOfSound)
{
    return utils_doubleToFixed(utils_linearInterp((double_t) _IQ18toF(iq18speedOfSound),
                              gas_lutSosToTemperature,
                              PROC_SOS_TO_TEMP_LUT_SIZE), 24);
}

static int32_t calcViscosity(int32_t const iq24Temperature)
{
    return utils_doubleToFixed(utils_linearInterp((double_t) _IQ24toF(iq24Temperature),
                              gas_lutTemperatureToViscosity,
                              PROC_TEMP_TO_VISC_LUT_SIZE), 30);
}

static int32_t calcReynolds(int32_t const iq26Velocity,
                             int32_t const iq30FlowPathDiameter,
                             int32_t const iq30kinematicViscosity)
{
    int32_t iq12Reynolds;

    iq12Reynolds =  _IQ30mpy(iq30FlowPathDiameter, iq26Velocity);
    iq12Reynolds = _IQ16div(iq12Reynolds, iq30kinematicViscosity);

    return iq12Reynolds;
}

static int32_t calcAbs(int32_t const l_value)
{
    int32_t l_absValue = l_value;

    if(l_value < 0)
    {
        l_absValue = -l_value;
    }

    return l_absValue;
}

static int32_t calcCalibConst(int32_t const iq12Reynolds)
{
    return utils_doubleToFixed(
            (utils_linearInterp((double_t) _IQ12toF(iq12Reynolds),
                  gps_sysConfig->s_calibration.as_calibLut,
                  (uint8_t) gps_sysConfig->s_calibration.ui_calibLutSize)), 29);
}

static int32_t calcCalibVelocity(int32_t const iq26uncalibVel,
                                  int32_t const iq29CalibConst)
{
    return _IQ29mpy(iq26uncalibVel, iq29CalibConst);
}


static int32_t calcFlowRate(int32_t const iq26CalibVel,
                             int32_t const iq30PathArea)
{
    int32_t iq18FlowRate;
    iq18FlowRate = _IQ26mpy(iq26CalibVel, iq30PathArea);
    return _IQ30mpy(iq18FlowRate, _IQ18(3600.0));
}

static int32_t calcVolume(int32_t const iq26CalibVel,
                           int32_t const iq30PathArea,
                           int32_t const iq28SamplingRate)
{
    int32_t iq30Vol = _IQ28div(iq30PathArea, iq28SamplingRate);
    return _IQ26mpy(iq30Vol, iq26CalibVel);
}

static void updateFixedPointers(void)
{
    // Update flow path into IQMath
    gs_iqVariables.iq30PathDiameter =
        _IQ30((float)gps_sysConfig->s_mechanical.d_flowPathDiameter);

    gs_iqVariables.iq30PathConst = _IQ29div(_IQ30((float)gps_sysConfig->s_mechanical.d_flowPathLength),
                                            _IQ29(2.0));
    gs_iqVariables.iq30VelSOSConst = _IQ29mpy(_IQ30((float)gps_sysConfig->s_mechanical.d_flowPathLength),
                                              _IQ29(2.0));
    gs_iqVariables.iq40DSOS =
        utils_doubleToFixed(gps_sysConfig->s_calibration.d_delaySpeedOfSound,
                        40);

    gs_iqVariables.iq44DZC = utils_doubleToFixed(
        gps_sysConfig->s_calibration.d_delayZeroCalibration, 44);

    gs_iqVariables.iq30PathArea =
        _IQ30(gps_sysConfig->s_mechanical.d_flowPathArea);
}

#if CONFIG_LOGGER_ENABLED
/**
 * @brief
 *      Update minimum flow rate.
 */
STATIC void updateMinFlowRate()
{
    if(gs_processing.d_currentFlowRate < gs_processing.d_minFlowRate || gs_processing.d_minFlowRate == 0.0)
    {
        gs_processing.d_minFlowRate = gs_processing.d_currentFlowRate;
    }
}

/**
 * @brief
 *      Update maximum flow rate.
 */
STATIC void updateMaxFlowRate()
{
    if(gs_processing.d_currentFlowRate > gs_processing.d_maxFlowRate || gs_processing.d_maxFlowRate == 0.0)
    {
        gs_processing.d_maxFlowRate = gs_processing.d_currentFlowRate;
    }
}

#endif

#else

#if CONFIG_LOGGER_ENABLED
/**
 * @brief
 *      Update minimum flow rate.
 */
STATIC void updateMinFlowRate()
{
    double_t d_currentFlowRate = gs_processing.d_currentFlowRate;
    double_t d_lastMinFlowRate = gs_processing.d_minFlowRate;

    // If current flow rate is negative, change it to positive for easier
    // comparison
    if(d_currentFlowRate < 0.0)
    {
        d_currentFlowRate *= -1.0;
    }

    // If last minimum flow rate is negative, change it to positive for easily
    // comparison
    if(d_lastMinFlowRate < 0.0)
    {
        d_lastMinFlowRate *= -1.0;
    }

    // Update minimum flow rate if the current flow rate is different of zero
    // and, the minimum flow rate is different of zero too or the current flow
    // rate is lower than the last minimum flow rate
    if((gs_processing.d_currentFlowRate) && (!gs_processing.d_minFlowRate ||
        (d_currentFlowRate < d_lastMinFlowRate)))
    {
        gs_processing.d_minFlowRate = gs_processing.d_currentFlowRate;
    }
}

/**
 * @brief
 *      Update maximum flow rate.
 */
STATIC void updateMaxFlowRate()
{
    double_t d_currentFlowRate = gs_processing.d_currentFlowRate;
    double_t d_lastMaxFlowRate = gs_processing.d_maxFlowRate;

    // If current flow rate is negative, change it to positive for easily
    // comparison
    if(d_currentFlowRate < 0.0)
    {
        d_currentFlowRate *= -1;
    }

    // If last maximum flow rate is negative, change it to positive for easily
    // comparison
    if(d_lastMaxFlowRate < 0.0)
    {
        d_lastMaxFlowRate *= -1;
    }

    // Update maximum flow rate it the current flow rate is higher than the
    // last maximum flow rate
    if(d_currentFlowRate > d_lastMaxFlowRate)
    {
        gs_processing.d_maxFlowRate = gs_processing.d_currentFlowRate;
    }
}
#endif


static double_t calcSpeedOfSound(double_t const d_ttDownstream,
                                 double_t const d_ttUpstream,
                                 double_t const d_flowPathLength,
                                 double_t const d_delaySpeedOfSound)
{
    double d_sos = 0;

    d_sos = d_ttUpstream;
    d_sos += d_ttDownstream;
    d_sos -= d_delaySpeedOfSound;
    d_sos -= d_delaySpeedOfSound;
    d_sos = (1.0 / d_sos);
    d_sos *= 2.0;
    d_sos *= d_flowPathLength;

    return d_sos;
}

static double_t calcVelocityBySos(double_t const d_ttDownstream,
                             double_t const d_ttUpstream,
                             double_t const d_flowPathLength,
                             double_t const d_delaySpeedOfSound,
                             double_t const d_delayZeroCalibration,
                             double_t const d_dTof)
{
    // Calculate corrected downstream transit time
    double_t d_num = 0;
    double_t d_denConst = (d_ttDownstream - d_delaySpeedOfSound + d_delayZeroCalibration);
    double_t d_den = 0;
    double_t d_const = (d_flowPathLength / 2.0);

    d_num = d_dTof;
    d_num -= d_delayZeroCalibration;
    d_num -= d_delayZeroCalibration;

    d_den = (d_denConst * d_ttUpstream);
    d_den -= (d_denConst * d_delaySpeedOfSound);
    d_den -= (d_denConst * d_delayZeroCalibration);

    double_t d_vel = d_num / d_den;
    d_vel *= d_const;

    return d_vel;
}

static double_t calcTemperature(double_t const d_speedOfSound)
{
    return utils_linearInterp(d_speedOfSound, gas_lutSosToTemperature,
        PROC_SOS_TO_TEMP_LUT_SIZE);
}

static double_t calcViscosity(double_t const d_temperature)
{
    return utils_linearInterp(d_temperature, gas_lutTemperatureToViscosity,
        PROC_TEMP_TO_VISC_LUT_SIZE);
}

static double_t calcReynolds(double_t const d_velocity,
                             double_t const d_flowPathDiameter,
                             double_t const d_kinematicViscosity)
{
    double_t d_reynolds;

    d_reynolds = d_velocity;
    d_reynolds *= d_flowPathDiameter;
    d_reynolds /= d_kinematicViscosity;

    return d_reynolds;
}

static double_t calcAbs(double_t const d_value)
{
    double_t d_absValue = d_value;

    if(d_value < 0)
    {
        d_absValue *= -1;
    }

    return d_absValue;
}

static double_t calcCalibConst(double_t const d_reynolds)
{
    return utils_linearInterp(d_reynolds, gps_sysConfig->s_calibration.as_calibLut,
                              (uint8_t)gps_sysConfig->s_calibration.ui_calibLutSize);
}

static double_t calcCalibVelocity(double_t const d_velocity,
                                  double_t const d_calibConst)
{
    double_t d_vel;

    d_vel = d_velocity;
    d_vel *= d_calibConst;

    return d_vel;
}

static double_t calcFlowRate(double_t const d_velocity,
                             double_t const d_flowPathArea)
{
    double_t d_flowRate;

    d_flowRate = d_velocity;
    d_flowRate *= d_flowPathArea;
    d_flowRate *= 3600.0;

    return d_flowRate;
}

static double_t calcVolume(double_t const d_velocity,
                           double_t const d_flowPathArea,
                           double_t const d_samplingRate)
{
    double_t d_vol;

    d_vol = d_velocity;
    d_vol *= d_flowPathArea;
    d_vol /= d_samplingRate;

    return d_vol;
}

#endif
