/***************************************************************************//**
 * @brief
 * @file    build.h
 * @author  Gustavo Salomão
 * @date    12/03/2018
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "types.h"
#include "processing.h"
#include "us.h"
#include "utils_fcx.h"
//#include "wmbus_defs.h"
//#include "wmbus_config.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

/** Version of the build based on firmware configuration. */
#if !CONFIG_WMBUS_ENABLED && !CONFIG_PULSE_ENABLED &&                          \
    !CONFIG_LOGGER_ENABLED && !CONFIG_OPTCOMM_ENABLED &&                       \
    !CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=N ; Pulse=N ; Logger=N ; Optcomm=N ; Fraud=N */
#define CONFIG_VERSION_BUILD                        0

#elif !CONFIG_WMBUS_ENABLED && CONFIG_PULSE_ENABLED &&                         \
    !CONFIG_LOGGER_ENABLED && !CONFIG_OPTCOMM_ENABLED &&                       \
    !CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=N ; Pulse=Y ; Logger=N ; Optcomm=N ; Fraud=N */
#define CONFIG_VERSION_BUILD                        1

#elif CONFIG_WMBUS_ENABLED && !CONFIG_PULSE_ENABLED &&                         \
    !CONFIG_LOGGER_ENABLED && !CONFIG_OPTCOMM_ENABLED &&                       \
    !CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=Y ; Pulse=N ; Logger=N ; Optcomm=N ; Fraud=N */
#define CONFIG_VERSION_BUILD                        2

#elif !CONFIG_WMBUS_ENABLED && CONFIG_PULSE_ENABLED &&                         \
    CONFIG_LOGGER_ENABLED && CONFIG_OPTCOMM_ENABLED &&                         \
    !CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=N ; Pulse=Y ; Logger=Y ; Optcomm=Y ; Fraud=N */
#define CONFIG_VERSION_BUILD                        3

#elif CONFIG_WMBUS_ENABLED && !CONFIG_PULSE_ENABLED &&                         \
    CONFIG_LOGGER_ENABLED && CONFIG_OPTCOMM_ENABLED &&                         \
    !CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=Y ; Pulse=N ; Logger=Y ; Optcomm=Y ; Fraud=N */
#define CONFIG_VERSION_BUILD                        4

#elif !CONFIG_WMBUS_ENABLED && CONFIG_PULSE_ENABLED &&                         \
    !CONFIG_LOGGER_ENABLED && !CONFIG_OPTCOMM_ENABLED &&                       \
    CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=N ; Pulse=Y ; Logger=N ; Optcomm=N ; Fraud=Y */
#define CONFIG_VERSION_BUILD                        5

#elif CONFIG_WMBUS_ENABLED && !CONFIG_PULSE_ENABLED &&                         \
    !CONFIG_LOGGER_ENABLED && !CONFIG_OPTCOMM_ENABLED &&                       \
    CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=Y ; Pulse=N ; Logger=N ; Optcomm=N ; Fraud=Y */
#define CONFIG_VERSION_BUILD                        6

#elif !CONFIG_WMBUS_ENABLED && CONFIG_PULSE_ENABLED &&                         \
    CONFIG_LOGGER_ENABLED && CONFIG_OPTCOMM_ENABLED &&                         \
    CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=N ; Pulse=Y ; Logger=Y ; Optcomm=Y ; Fraud=Y */
#define CONFIG_VERSION_BUILD                        7

#elif 1 || CONFIG_WMBUS_ENABLED && !CONFIG_PULSE_ENABLED &&                         \
    CONFIG_LOGGER_ENABLED && CONFIG_OPTCOMM_ENABLED &&                         \
    CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=Y ; Pulse=N ; Logger=Y ; Optcomm=Y ; Fraud=Y */
#define CONFIG_VERSION_BUILD                        8


#elif CONFIG_WMBUS_ENABLED && !CONFIG_PULSE_ENABLED &&                         \
    !CONFIG_LOGGER_ENABLED && CONFIG_OPTCOMM_ENABLED &&                         \
    !CONFIG_FRAUD_ENABLED

/** Build: WM-Bus=Y ; Pulse=N ; Logger=N ; Optcomm=Y ; Fraud=N */
#define CONFIG_VERSION_BUILD                        9

#elif DEBUG
#define CONFIG_VERSION_BUILD                        10
#else
#error "Invalid build configuration."
#endif
