/***************************************************************************//**
 * @brief
 * @file    probe.c
 * @author  Robson Cardoso
 * @date    07/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/
#include "probe.h"
#include "processing.h"
#include "ussSwLib.h"
#include "utils_fcx.h"
#include "serial.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
/** Start of package. */
#define PROBE_CMD_START_OF_PKG                  0xFFFFFFFFFFFFFFFFULL

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static inline void sendProcessVariables();

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Data transmission flag. */
static bool gb_enableDataTransmission = false;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize probe module.
 */
void probe_init(void)
{
    // Initialize data transmission flag
    gb_enableDataTransmission = false;
}

/**
 * @brief
 *      Enable data transmission through serial interface.
 */
void probe_enableDataTransmission(void)
{
    gb_enableDataTransmission = true;
}

/**
 * @brief
 *      Disable data transmission through serial interface.
 */
void probe_disableDataTransmission(void)
{
    gb_enableDataTransmission = false;
}

/**
 * @brief
 *      Update module.
 */
void probe_update(void)
{
    if (gb_enableDataTransmission)
    {
        sendProcessVariables();
    }
}

/**
 * @brief
 *      Send process variables via serial interface.
 */
static inline void sendProcessVariables(void)
{
#if !DEBUG_SERIAL
    s_processing_t *s_procResults = proc_getResults();

    uint64_t ull_startOfPackage = PROBE_CMD_START_OF_PKG;

    serial_transmitBytes((uint8_t *)&ull_startOfPackage,
                         sizeof(ull_startOfPackage));

    serial_transmitBytes((uint8_t *)&(s_procResults->s_usMeasurement.ui_agStatus),
                         sizeof(s_procResults->s_usMeasurement.ui_agStatus));
    serial_transmitBytes((uint8_t *)&(s_procResults->s_usMeasurement.ui_r1),
                         sizeof(s_procResults->s_usMeasurement.ui_r1));
    serial_transmitBytes((uint8_t *)&(s_procResults->s_usMeasurement.ui_gm1),
                         sizeof(s_procResults->s_usMeasurement.ui_gm1));
    serial_transmitBytes((uint8_t *)&(s_procResults->s_usMeasurement.d_hfc),
                         sizeof(s_procResults->s_usMeasurement.d_hfc));
    serial_transmitBytes((uint8_t *)&s_procResults->s_usMeasurement.d_ttDownstream,
                         sizeof(s_procResults->s_usMeasurement.d_ttDownstream));
    serial_transmitBytes((uint8_t *)&s_procResults->s_usMeasurement.d_ttUpstream,
                         sizeof(s_procResults->s_usMeasurement.d_ttUpstream));
    serial_transmitBytes((uint8_t *)&(s_procResults->d_uncalibVel),
                         sizeof(s_procResults->d_uncalibVel));
    serial_transmitBytes((uint8_t *)&(s_procResults->d_calibVel),
                         sizeof(s_procResults->d_calibVel));
    serial_transmitBytes((uint8_t *)&(s_procResults->d_currentFlowRate),
                         sizeof(s_procResults->d_currentFlowRate));
    serial_transmitBytes((uint8_t *)&(s_procResults->d_volume),
                         sizeof(s_procResults->d_volume));
    serial_transmitBytes((uint8_t *)&(s_procResults->d_forwardVolume),
                         sizeof(s_procResults->d_forwardVolume));
    serial_transmitBytes((uint8_t *)&(s_procResults->d_reverseVolume),
                         sizeof(s_procResults->d_reverseVolume));
#endif
}

