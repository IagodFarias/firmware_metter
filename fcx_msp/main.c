/*******************************************************************************
 * @brief   <brief description>
 * @file    main.c
 * @author  Eric Costa Hall
 * @date    07/06/2021
 * @section license
 *          FAE Tecnologia. All rights reserved
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
#include "batt.h"
#include "rev_install.h"
#include "no_usage.h"
#include "temp.h"

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
#error "Build inválida. Remover OPTCOMM_CHANGE_CONFIG"
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global variables
 ******************************************************************************/

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

    //printf("Hello, world!\nHello, World");
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
    leak_init();
    brush_init();
    rev_install_init();
    no_usage_init();
    temp_init();
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

    // Initialize operating mode manager
    opmodemng_init();

    /* Main loop  */
    while (1)
    {

        // Update system tick
        systick_updateSysTickMs();

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
        batt_update();
        userint_update();

        // Update operating mode manager
        opmodemng_update();

        // Update Screen
        screenmng_update();

        // Update pulse manager
        pulsemng_update();

        power_enterSleep();
    }
}
