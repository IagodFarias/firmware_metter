/***************************************************************************//**
 * @brief
 * @file    general_settings.h
 * @author  Robson Cardoso
 * @date    13/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef GENERAL_SETTINGS_H_
#define GENERAL_SETTINGS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"
#include <time.h>

/*******************************************************************************
 * Build Definitions
 ******************************************************************************/
/**
 * Enable LEA-RAM usage for the USS component.
 */
#define __USS_RUN_ALG_FROM_RAM__                            1
/**
 * Enable WM-Bus.
 */
#define CONFIG_WMBUS_ENABLED                                0

/**
 * Enable Pulse.
 */
#define CONFIG_PULSE_ENABLED                                1

/*
 * Enable logger.
 */
#define CONFIG_LOGGER_ENABLED                               1

/*
 * Enable optical communication.
 */
#define CONFIG_OPTCOMM_ENABLED                              1

/*
 * Enable Firmware Verification Through optical interface
 */
#define CONFIG_FW_INTEGRITY                                 1

/*
 * Enable fraud detection.
 */
#define CONFIG_FRAUD_ENABLED                                0



/*
 * Radio configuration. Indicates if radio is bidirectional
 */
#define CONFIG_RF_RX_ENABLED                                0

/*
 * Fixed Point calculatioormal
 * n. Indicates if should use fixed point in all
 * math calculation methods
 */
#define FIXED_POINT_CALC                                    1

/*
 * Enable EVM hardware build to change code
 */
#define HARDWARE_EVM                                        1

/*******************************************************************************
 * CONFIG Definitions
 ******************************************************************************/

#if CONFIG_OPTCOMM_ENABLED
/*
 * Enable change mode by optic interface
 */
#define CONFIG_OPTCOMM_CHANGE_MODE                          1
/*
 * Enable ability to change configurations through optic interface *DO NOT ACTIVATE*
 */
#define OPTCOMM_CHANGE_CONFIG                               0
#endif /* CONFIG_OPTCOMM_ENABLED */

#if OPTCOMM_CHANGE_CONFIG
#error "Build inválida. Remover OPTCOMM_CHANGE_CONFIG"
#endif

#if CONFIG_LOGGER_ENABLED
/*
 * Enable erase logs
 */
#define CONFIG_OPTCOMM_ERASE_LOGS                           0
#endif

/**
 * Enable US variable configuration on flow velocity
 */
#define CONFIG_VARIABLE_US                                  0


/**
 * Enable Dn25 configuration
 */
#define CONFIG_DN_25                                        0

#define LCD_USE_RESISTOR                                    0
#if LCD_USE_RESISTOR
#define LCD_USE_RES_LADDER
#define LCD_USE_CHARGEPUMP
#endif

/*******************************************************************************
 * DEBUG Definitions
 ******************************************************************************/
/**
 * Enable Debug for Power/state detection.
 */
#define CONFIG_WMBUS_ENABLED                                0
/**
 * Enable mock flow rate to processing module
 */
#define DEBUG_PROC_FLOWRATE                                 0
#define DEBUG_FLOWRATE                                      0

#define DEBUG_NORMAL                                        1

#define DEBUG_CHANGE_TO_STANDBY                             0

/**
 * Enable print of pulse manager information
 */
#define DEBUG_PULSEMNG                                      0

/**
 * Enable serial comm on VERIF mode
 */
#define DEBUG_PROBE_VERIF                                   0

/**
 * Enable serial comm on NORMAL mode
 */
#define DEBUG_PROBE_NORMAL                                  0

/**
 * Enable print of used const
 */
#define DEBUG_PRINT_CONST                                   0

/**
 * Enable printf of configuration struct
 */
#define DEBUG_PRINT_CONFIG                                  0

/**
 * Enable print of used configured ULOOP setting for variable setting in
 * CONFIGMNG
 */
#define DEBUG_PRINT_ULOOP                                   0

/*
 * Enable DEBUG via serial
 */
#define DEBUG_PRINT_OPTCOMM                                 0


#define DEBUG_COMM                                          0

/*
 * Enable DEBUG analog sense
 */
#define DEBUG_ANASENS                                       0

/*
 * Enable DEBUG_OPTINT
 */
#define DEBUG_OPTINT                                        0

/*
 * Enable DEBUG_USERINT
 */
#define DEBUG_USERINT                                       0

/*
 * Debug LOGGER
 */
#define DEBUG_LOGGER                                        0

/*
 * DEBUG_SCREENMNG
 */
#define DEBUG_SCREENMNG                                     0

/*
 * DEBUG_ALARMMNG
 */
#define DEBUG_ALARMMNG                                      0

/*
 * DEBUG_BRUSH
 */
#define DEBUG_BRUSH                                         0

/*
 * DEBUG_BATT
 */
#define DEBUG_BATT                                          0

/*
 * DEBUG_REV_INSTALL
 */
#define DEBUG_REV_INSTALL                                   0

/*
 * DEBUG_NO_USAGE
 */
#define DEBUG_NO_USAGE                                      0

/*
 * DEBUG_TEMP
 */
#define DEBUG_TEMP                                          0

/*
 * DEBUG_FRAUD
 */
#define DEBUG_FRAUD                                         0

#if (DEBUG_PROBE_VERIF  || DEBUG_PROBE_NORMAL   || DEBUG_PRINT_CONST    ||   \
    DEBUG_PRINT_CONFIG  || DEBUG_PRINT_ULOOP    || DEBUG_PRINT_OPTCOMM  ||   \
    DEBUG_OPTINT        /*|| DEBUG_ANASENS  */      || DEBUG_USERINT        ||   \
    DEBUG_LOGGER        || DEBUG_SCREENMNG      || DEBUG_PROC_FLOWRATE  ||   \
    DEBUG_ALARMMNG      || DEBUG_BRUSH          || DEBUG_BATT           ||   \
    DEBUG_REV_INSTALL   || DEBUG_NO_USAGE       || DEBUG_TEMP           ||   \
    DEBUG_FRAUD )//|| DEBUG_NORMAL)
#define DEBUG_SERIAL                                        0
#define DEBUG                                               1
#endif

/*******************************************************************************
 * Compiler definitions
 ******************************************************************************/
#ifndef __TI_COMPILER_VERSION__
#define __TI_COMPILER_VERSION__
#endif


#ifndef STATIC
    #if defined(UNIT_TEST) || defined(FUNC_TEST)
        #define STATIC
    #else
        #define STATIC                                      static
    #endif /* UNIT_TEST */
#endif /* STATIC */

#ifndef INLINE
    #ifdef FUNC_TEST
        #define INLINE
    #else
        #define INLINE                                      inline
    #endif /* FUNC_TEST */
#endif /* INLINE */

#ifndef CONST
    #define CONST                                           const
#endif /* CONST */

#ifndef VOLATILE
    #define VOLATILE                                        volatile
#endif /* VOLATILE */

#ifndef EXTERN
    #define EXTERN                                          extern
#endif /* EXTERN */

#ifndef PACKED_STRUCT
    #define PACKED_STRUCT(packedStruct, structName) \
                packedStruct __attribute__((__packed__)) structName
#endif /* PACKED_STRUCT */

#ifdef __ICC430__
    #define NO_INIT                                         __no_init
#else
    #define NO_INIT
#endif /* NO_INIT */

// Definitions for unit prefixes
#define HZ                                                  1
#define DECA                                                10
#define HECTO                                               100
#define KILO                                                1000
#define MEGA                                                1000000
#define GIGA                                                1000000000
#define TERA                                                1000000000000
#define PETA                                                1000000000000000
#define EXA                                                 1000000000000000000

#endif /* GENERAL_SETTINGS_H_ */
