/***************************************************************************//**
 * @brief
 * @file 	systick.c
 * @author 	Robson Cardoso
 * @date 	13/02/2017
 * @details
 * @section license
 * 			FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "systick.h"
#include "rtc_c.h"
#include "utils_fcx.h"
#include "us.h"
#include "USS_userConfig.h"
#include "time.h"

#if CONFIG_PULSE_ENABLED
#include "pulsemng.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if defined(__TI_COMPILER_VERSION__)
#pragma PERSISTENT(gull_sysTickMs)
#define __persistent
#elif defined (__IAR_SYSTEMS_ICC__)
#else
#error Compiler not supported!
#endif

/** Low frequency clock frequency in Hz. */
#define LFCLK_FREQ                                  USS_LFXT_FREQ_IN_HZ

/** Maximum acceptable tick count in seconds (12/31/9999) */
#define SYSTICK_MAX_TICK_CNT_MS                     253402300799UL

/** Convert millisecond to tick */
#define SYSTICK_MS_TO_TICK(ms)                      ((ms * LFCLK_FREQ) / 1000L) \
                                                        - 1

/** Convert millisecond to tick */
//#define SYSTICK_MS_TO_TICK(ms)                      ms << 5

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static bool isValidDateTime(struct tm *ps_date);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
#if defined(__TI_COMPILER_VERSION__)
__persistent STATIC uint64_t gull_sysTickMs  = 0;
#elif defined (__IAR_SYSTEMS_ICC__)
STATIC NO_INIT uint64_t gull_sysTickMs = 0;
#else
#error Compiler not supported!
#endif/** System tick in milliseconds. */

/** System wake up interval in milliseconds. */
STATIC uint16_t gui_wakeUpIntMs = 0;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void systick_init(void)
{
    gui_wakeUpIntMs = SYSTICK_WAKE_UP_INT_MS_DEFAULT_INT;
    systick_setWakeUpIntMs(SYSTICK_WAKE_UP_INT_MS_DEFAULT_INT);
}

void systick_setWakeUpIntMs(uint16_t ui_syncWakeUp)
{
    if (ui_syncWakeUp > 0)
    {
        // Update module's internal wake up time interval
        gui_wakeUpIntMs = ui_syncWakeUp;

        // Check if the required wake up interval has been correctly set
        ASSERT(gui_wakeUpIntMs == ui_syncWakeUp);
    }
}

uint16_t systick_getWakeUpIntMs(void)
{
    return gui_wakeUpIntMs;
}

uint64_t systick_getSysTickMs(void)
{
    return gull_sysTickMs;
}

void systick_updateSysTickMs(void)
{
    // Increment system tick
    gull_sysTickMs += (uint64_t) gui_wakeUpIntMs;

    // Set the background timer period
    USS_configAppTimerPeriod(&gUssSWConfig,
                             SYSTICK_MS_TO_TICK(gui_wakeUpIntMs));

    // Reset flags
    USSSWLIB_USS_interrupt_status = 0;
}

struct tm* systick_getActualDateTime(void)
{
    time_t n_actualTime = time(NULL);
    return gmtime(&n_actualTime);
}

bool systick_setActualDateTime(struct tm *ps_calendar)
{
    bool b_ret = false;

    if(isValidDateTime(ps_calendar))
    {
        gull_sysTickMs = (uint64_t) mktime(ps_calendar) * 1000;

        b_ret = true;
    }

    return b_ret;
}

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
static bool isValidDateTime(struct tm *ps_date)
{
    bool b_ret = true;
    uint64_t ull_expectedSecs = 0;

    /* Converts CTS to UTC timezone before mktime because TI version of
     * count uses CTS (UTC-6) pattern. */
    ps_date->tm_hour -= SYSTICK_CTS_TO_UTC_OFFSET;

    ull_expectedSecs = (uint64_t) mktime(ps_date);

    // Check if it is a valid date
    if(ull_expectedSecs > SYSTICK_MAX_TICK_CNT_MS)
    {
        b_ret = false;
    }

    return b_ret;
}

/*******************************************************************************
 * Events
 ******************************************************************************/

__time64_t (__time64)(__time64_t *t)
{
    __time64_t n_actualUnixTime = gull_sysTickMs/1000;

    if(t)
    {
        *t = n_actualUnixTime;
    }

    return n_actualUnixTime;
}
