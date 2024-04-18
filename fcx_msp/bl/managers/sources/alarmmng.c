/***************************************************************************//**
 * @brief
 * @file    alarmmng.c
 * @author  Robson Cardoso
 * @date    10/04/2017
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "brush.h"
#include "alarmmng.h"
#include "leak.h"
#include "processing.h"
#include "general_settings.h"
#include "hal.h"
#include "systick.h"
#include "rev_install.h"
#include "no_usage.h"
#include "temp.h"

#if DEBUG_ALARMMNG
#include <stdio.h>
#endif /* DEBUG_ALARMMNG */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if DEBUG
/** Alarm timeout in milliseconds. */
#define ALARMMNG_ALARM_TIMEOUT_MS               60000UL // 60 seconds
#else
/** Alarm timeout in milliseconds. */
#define ALARMMNG_ALARM_TIMEOUT_MS               3456000000ULL // 40 days
#endif

/** Air in pipe detection timeout in milliseconds. */
#define ALARMMNG_AIR_IN_PIPE_DETEC_TIMEOUT_MS   10000UL

#if DEBUG
/** Reverse flow detection timeout in milliseconds. */
#define ALARMMNG_REVERSE_FLOW_DETEC_TIMEOUT_MS  20000UL // 20 seconds
#else
/** Reverse flow detection timeout in milliseconds. */
#define ALARMMNG_REVERSE_FLOW_DETEC_TIMEOUT_MS  600000UL // 10 minutes
#endif

/** Reverse flow detection threshold in m³. */
#define ALARMMNG_REVERSE_FLOW_TRHESHOLD         -0.010

/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef struct S_REV_FLOW
{
    /** Indicates that the reverse flow alarm is activated. */
    bool b_isAlarmed;
    /** Indicates that the reverse flow was detected. */
    bool b_isDetected;
    /** Volume which will triggers the reverse flow alarm. */
    double_t d_volumeTrigger;
    /** Timeout for reverse flow alarm. */
    uint64_t ull_timeout;
} s_rev_flow_t;

typedef struct S_ALARM
{
    /** Indicates alarm is activated. */
    bool b_isAlarmActived;
    /** Alarm timeout. */
    uint64_t ull_timeout;
    /** Function pointer for alarm verification. */
    bool (*pf_isAlarm)(void);
} s_alarm_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static inline void checkLeakage();
static inline void checkReverseFlow();
static inline void checkAirInPipe();
static inline void checkAlarm(s_alarm_t *ps_alarm);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Indicates that the leakage alarm is activated. */
static bool gb_isLeakage;

/** Timeout for leakage alarm. */
static uint64_t gull_leakageTimeout;

/** Reverse Flow alarm structure. */
static s_rev_flow_t gs_revFlow =
{
     .b_isAlarmed = false,
     .b_isDetected = false,
     .d_volumeTrigger = 0.0,
     .ull_timeout = 0
};

/** Indicates that the air in pipe alarm is activated. */
static bool gb_isAirInPipe;

/** Indicates that the air in pipe alarm detection started. */
static bool gb_airInPipeDetectionStarted;

/** Timeout for air in pipe alarm. */
static uint64_t gull_airInPipeTimeout;

/** Brush (max overflow) alarm structure. */
static s_alarm_t gs_brush = {
    .b_isAlarmActived = false,
    .ull_timeout = 0,
    .pf_isAlarm = brush_isOverflowing
};

/** Reverse install alarm structure. */
static s_alarm_t gs_revInstall = {
    .b_isAlarmActived = false,
    .ull_timeout = 0,
    .pf_isAlarm = rev_install_isDetected
};

/** No usage alarm structure. */
static s_alarm_t gs_noUsage = {
    .b_isAlarmActived = false,
    .ull_timeout = 0,
    .pf_isAlarm = no_usage_isDetected
};

/** High/Low temperature alarm structure. */
static s_alarm_t gs_temperature = {
    .b_isAlarmActived = false,
    .ull_timeout = 0,
    .pf_isAlarm = temp_isOutsideLimit
};

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void alarmmng_init()
{
    gb_isLeakage = false;
    gb_isAirInPipe = false;
    gb_airInPipeDetectionStarted = false;

    gs_revFlow.b_isAlarmed = false;
    gs_revFlow.b_isDetected = false;

    // Initialize brush (maximum overflow) alarm
    gs_brush.b_isAlarmActived = false;

    gull_airInPipeTimeout = SYSTICK_INVALID_TICK;

#if DEBUG_ALARMMNG
    printf("(ALARMMNG): Alarmmng initialized.\n");
#endif /* DEBUG_ALARMMNG */
}

void alarmmng_update()
{
#if DEBUG_ALARMMNG
    printf("(ALARMMNG): Update alarmmng.\n");
#endif /* DEBUG_ALARMMNG */

    // Check alarms
    checkLeakage();
    checkReverseFlow();
    checkAirInPipe();
    checkAlarm(&gs_brush);
    checkAlarm(&gs_revInstall);
    checkAlarm(&gs_noUsage);
    checkAlarm(&gs_temperature);
}

bool alarmmng_isLeakage()
{
    return gb_isLeakage;
}

bool alarmmng_isReverseFlow()
{
    return gs_revFlow.b_isAlarmed;
}

bool alarmmng_isAirInPipe()
{
    return gb_isAirInPipe;
}

bool alarmmng_isBrush()
{
    return gs_brush.b_isAlarmActived;
}

bool alarmmng_isRevInstall()
{
    return gs_revInstall.b_isAlarmActived;
}

bool alarmmng_isNoUsage()
{
    return gs_noUsage.b_isAlarmActived;
}

bool alarmmng_isOverTemp()
{
    return gs_temperature.b_isAlarmActived;
}

/*******************************************************************************
 * Private functions
 ******************************************************************************/
/**
 *  @brief
 *      Check if leakage was detected.
 */
static inline void checkLeakage()
{
    if(leak_isLeakage())
    {
        // Set leakage alarm
        gb_isLeakage = true;

        // Update leakage timeout
        gull_leakageTimeout = systick_getSysTickMs();
        gull_leakageTimeout += ALARMMNG_ALARM_TIMEOUT_MS;

#if DEBUG_ALARMMNG
        printf("(ALARMMNG): Leakage detected.\n");
#endif /* DEBUG_ALARMMNG */
    }
    else if((gb_isLeakage) && (systick_getSysTickMs() > gull_leakageTimeout))
    {
        // Disable leakage alarm
        gb_isLeakage = false;

        // Reset timeout
        gull_leakageTimeout = SYSTICK_INVALID_TICK;

#if DEBUG_ALARMMNG
        printf("(ALARMMNG): Leakage alarm timed out.\n");
#endif /* DEBUG_ALARMMNG */
    }
}

/**
 *  @brief
 *      Check if reverse flow was detected.
 */
static inline void checkReverseFlow()
{
    if(proc_isReverseFlow())
    {
        double_t d_revFlow = proc_getReverseVolume();

#if DEBUG_ALARMMNG
        printf("(ALARMMNG): Actual reverse flow: %f\n", d_revFlow);
#endif /* DEBUG_ALARMMNG */

        if (!gs_revFlow.b_isDetected)
        {
            // Calculate the volume which will triggers the alarm
            gs_revFlow.d_volumeTrigger = ALARMMNG_REVERSE_FLOW_TRHESHOLD;
            gs_revFlow.d_volumeTrigger += d_revFlow;

            // Calculate the detection timeout
            gs_revFlow.ull_timeout = systick_getSysTickMs();
            gs_revFlow.ull_timeout += ALARMMNG_REVERSE_FLOW_DETEC_TIMEOUT_MS;

            // Set reverse flow as activated
            gs_revFlow.b_isDetected = true;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Reverse flow detection started (Trigger: %f).\n",
                   gs_revFlow.d_volumeTrigger);
#endif /* DEBUG_ALARMMNG */
        }
        else if (d_revFlow < gs_revFlow.d_volumeTrigger)
        {
            // Set reverse flow alarm
            gs_revFlow.b_isAlarmed = true;

            // Update reverse flow timeout
            gs_revFlow.ull_timeout = systick_getSysTickMs();
            gs_revFlow.ull_timeout += ALARMMNG_ALARM_TIMEOUT_MS;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Reverse flow detected.\n");
#endif /* DEBUG_ALARMMNG */
        }
        else
        {
            // Update the detection timeout
            gs_revFlow.ull_timeout = systick_getSysTickMs();
            gs_revFlow.ull_timeout += ALARMMNG_REVERSE_FLOW_DETEC_TIMEOUT_MS;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Detecting reverse flow.\n");
#endif /* DEBUG_ALARMMNG */
        }
    }
    else if(gs_revFlow.b_isAlarmed)
    {
        if (systick_getSysTickMs() > gs_revFlow.ull_timeout)
        {
            // Disable reverse flow alarm
            gs_revFlow.b_isAlarmed = false;

            // Disable reverse flow
            gs_revFlow.b_isDetected = false;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Reverse flow timed out.\n");
#endif /* DEBUG_ALARMMNG */
        }
    }
    else if(gs_revFlow.b_isDetected)
    {
        if (systick_getSysTickMs() > gs_revFlow.ull_timeout)
        {
            // Disable reverse flow
            gs_revFlow.b_isDetected = false;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Reverse flow detection canceled.\n");
#endif /* DEBUG_ALARMMNG */
        }
    }
}

/**
 *  @brief
 *      Check if air in pipe was detected.
 */
static inline void checkAirInPipe()
{
    if(proc_isAirInPipe())
    {
        if (!gb_airInPipeDetectionStarted)
        {
            // Start the air in line alarm detection
            gb_airInPipeDetectionStarted = true;

            // Calculate the detection timeout
            gull_airInPipeTimeout = systick_getSysTickMs();
            gull_airInPipeTimeout += ALARMMNG_AIR_IN_PIPE_DETEC_TIMEOUT_MS;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Air in pipe detection started.\n");
#endif /* DEBUG_ALARMMNG */
        }
        else if ((gb_isAirInPipe) ||
            (systick_getSysTickMs() > gull_airInPipeTimeout))
        {
            // Set air in pipe alarm
            gb_isAirInPipe = true;

            // Update air in pipe timeout
            gull_airInPipeTimeout = systick_getSysTickMs();
            gull_airInPipeTimeout += ALARMMNG_ALARM_TIMEOUT_MS;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Air in pipe detected.\n");
#endif /* DEBUG_ALARMMNG */
        }
    }
    else if(gb_isAirInPipe)
    {
        if(systick_getSysTickMs() > gull_airInPipeTimeout)
        {
            // Disable air in pipe alarm
            gb_isAirInPipe = false;

            // Finish the air in line alarm detection
            gb_airInPipeDetectionStarted = false;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Air in pipe detection canceled.\n");
#endif /* DEBUG_ALARMMNG */
        }
    }
    else if(gb_airInPipeDetectionStarted)
    {
        // Finish the air in line detection
        gb_airInPipeDetectionStarted = false;
    }
}

/**
 *  @brief
 *      Alarm enable/disable if detected.
 */
static inline void checkAlarm(s_alarm_t *ps_alarm)
{
    if(ps_alarm->pf_isAlarm())
    {
        // Start the alarm detection
        ps_alarm->b_isAlarmActived = true;

        // Calculate the detection timeout
        ps_alarm->ull_timeout = systick_getSysTickMs();
        ps_alarm->ull_timeout += ALARMMNG_ALARM_TIMEOUT_MS;

#if DEBUG_ALARMMNG
            printf("(ALARMMNG): Alarm detected.\n");
#endif /* DEBUG_ALARMMNG */
    }
    else if(ps_alarm->b_isAlarmActived)
    {
        if(systick_getSysTickMs() > ps_alarm->ull_timeout )
        {
            // Disable air in pipe alarm
            ps_alarm->b_isAlarmActived = false;
#if DEBUG_ALARMMNG
            printf("(ALARMMNG):Alarm timeout.\n");
#endif /* DEBUG_ALARMMNG */
        }
    }
}
