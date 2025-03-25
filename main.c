/*******************************************************************************
 * @brief   <brief description>
 * @file    main.c
 * @author  Eduardo Balbino
 * @date    14/11/2024
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved
 ******************************************************************************/
#include <stdint.h>

#include "brush.h"
#include "hal.h"
#include "probe.h"
#include "cmdmng.h"
#include "mpu.h"
#include "configmng.h"
#include "opmodemng.h"
#include "screenmng.h"
#include "userint.h"
#include "alarmmng.h"
#include "power.h"
#include "optint.h"
#include "anasens.h"
#include "systick.h"
#include "leak.h"
#include "battdate.h"
#include "rev_install.h"
#include "no_usage.h"
#include "temp.h"
#include "batt.h"

#include "aes_cmac.h"

#if CONFIG_FRAUD_ENABLED
#include "fraud.h"
#endif

#if CONFIG_LOGGER_ENABLED
#include "logger.h"
#endif /* CONFIG_PULSE_ENABLED */

#if CONFIG_OPTCOMM_ENABLED
#include "optcomm.h"
#endif

#if CONFIG_PULSE_ENABLED
#include "pulsemng.h"
#endif /* CONFIG_PULSE_ENABLED */

#if OPTCOMM_CHANGE_CONFIG
#error "Build inv�lida. Remover OPTCOMM_CHANGE_CONFIG"
#endif

#ifdef DEBUG_ASSERT
#define ASSERT(expr)    ((expr) ? ((void)0) : utils_assert(__FILE__, __LINE__)
#else 
#define ASSERT(expr)    ((expr) ? ((void)0) : utils_assert(expr))
#endif /* DEBUG_ASSERT */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/* These variables can't be inside aes_cmac.c because that would cause an (almost) infinite recursion for the hash, so, to avoid that: */
uint8_t aes_key_main[16] = {0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA};
uint8_t cmac_out[16] = {0};

uint8_t cmac_test[16] = {0x51, 0xE7, 0x06, 0x2D, 0xA5, 0x20, 0xFB, 0xA6, 0x6D, 0x8C, 0x56, 0xF8, 0xCC, 0xCA, 0x46, 0x0C};

/*******************************************************************************
 * Public Functions
 ******************************************************************************/
//*****************************************************************************
//
//! In this software main will call a few initialization routines and then it
//!  will jump to the main loop for the Design Center
//!
//! \return none
//
// *****************************************************************************

void main(void)

{
    // Initializes the basic functionality of the system
    hal_init();

    // Verify the reset source and log/report if there was an error
    ASSERT(E_WAKEUP_UNKNOWN_REASON != power_getWakeUpReason());
    // printf("%d", power_getWakeUpReason());

    /* Initialize system configuration before HAL to check hardware
    compatibility. */
    configmng_init();

    // Initialize command manager
    cmdmng_init();

#if CONFIG_PULSE_ENABLED
    // Initialize pulse manager
    pulsemng_init();
#endif /* CONFIG_PULSE_ENABLED */

    // Initialize user interface
    userint_init();

    // Initialize the alarm manager
    alarmmng_init();

    // Initialize Alarms modules
    battdate_init();
    leak_init();
    brush_init();
    rev_install_init();
    no_usage_init();
    temp_init();
    battdate_init();

    batt_init();
#if CONFIG_FRAUD_ENABLED
    fraud_init();
#endif /* CONFIG_FRAUD_ENABLED */

#if CONFIG_OPTCOMM_ENABLED
    // Initialize optical communication
    optcomm_init();
#endif /* CONFIG_OPTCOMM_ENABLED */

#if CONFIG_LOGGER_ENABLED
    // Initialize logger
    logger_init();
#endif /* CONFIG_OPTCOMM_ENABLED */

    cmac_init();

    // Initialize operating mode manager
    opmodemng_init();

    /* Main loop  */
    while (1)
    {
        // Update system tick
        systick_updateSysTickMs();

        cmac_update(cmac_test);

        // Update processing
        proc_update();

        // Start optint
        optint_start();

        // Trigger analog conversion
        anasens_startConversion();

        // Dispatch any previously command received
        cmdmng_fetch();
        cmdmng_dispatch();
        // Update analog sensors
        anasens_update();
        optint_update();
        userint_update();
        batt_update();

        // Update operating mode manager
        opmodemng_update();

        // Update Screen
        screenmng_update();

        // Update pulse manager
        pulsemng_update();

        power_enterSleep();
    }
}
