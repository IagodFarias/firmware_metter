/***************************************************************************//**
 * @brief
 * @file    batt.c
 * @author  Robson Cardoso
 * @date    13/04/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "batt.h"
#include "anasens.h"
#include "systick.h"
#include "configmng.h"
#include <stdio.h>

#if DEBUG_BATT
#include <stdio.h>
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BATT_V_SENSE_PORT                       GPIO_PORT_P2
#define BATT_V_SENSE_PIN                        GPIO_PIN3
#define BATT_V_ENABLE_PIN                       GPIO_PIN2

#if DEBUG
/** Battery detection interval. */
#define BATT_DETECTION_INT_MS                   10000ULL
#else
/** Battery detection interval. */
#define BATT_DETECTION_INT_MS                   3600000ULL
#endif
/** Battery detection threshold. */
#define BATT_DETECTION_THRESHOLD                3450

/** Battery alarm maximum interval from INMETRO(180 days). */
#define BATT_MAX_INTERVAL_IN_DAYS               180

/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef struct S_BATT_CALENDAR
{
    /** Year */
    uint8_t auc_year[4];
    /** Month */
    uint8_t auc_month[2];
    /** Day */
    uint8_t auc_day[2];
} s_batt_calendar_t;

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** Define possibles states. */
typedef enum E_BATT_STATE
{
    /** Idle state. */
    E_BATT_STATE_IDLE,
    /** Detection state. */
    E_BATT_STATE_DETECT,
} E_BATT_STATE_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void calculateLowBattDate(void);
/*******************************************************************************
 * Global variables
 ******************************************************************************/
static bool gb_isBatteryLow;
static time_t gs_dateAlarm;
static uint32_t gul_batteryDetectionCounter;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void batt_init(void)
{
    // Configure low battery date time
    calculateLowBattDate();

    gb_isBatteryLow = false;
}

void batt_update(void)
{
    if(gul_batteryDetectionCounter > BATT_DETECTION_INT_MS){
        gul_batteryDetectionCounter = 0;
        struct tm *ps_actualDateTime = systick_getActualDateTime();

    #if DEBUG_BATT
        mktime(ps_actualDateTime);

        printf("(BATT): Now is %02d/%02d/%02d.\n",
                ps_actualDateTime->tm_mday, ps_actualDateTime->tm_mon + 1,
                ps_actualDateTime->tm_year - 100);
    #endif /* DEBUG_BATT */

        gb_isBatteryLow =
                difftime(mktime(ps_actualDateTime), gs_dateAlarm) > 0 ? true : false;

    #if DEBUG_BATT
        mktime(ps_actualDateTime);

        printf("(BATT): gb_isBatteryLow is %d.\n",(int) gb_isBatteryLow);
    #endif /* DEBUG_BATT */
    }
    else{
        gul_batteryDetectionCounter += systick_getWakeUpIntMs();
    }
}

bool batt_isLow(void)
{
    return gb_isBatteryLow;
}

void batt_updateConfig(void)
{
    // Update the low battery date time
    calculateLowBattDate();
}

static void calculateLowBattDate(void)
{
    struct tm s_dateAlarm;
    char c_intStr[5];
    int tempInt = 0;
    s_batt_calendar_t *ps_date =
            (s_batt_calendar_t*) configmng_getReplacementDateCharArray();

    c_intStr[0] = ps_date->auc_year[3];
    c_intStr[1] = ps_date->auc_year[2];
    c_intStr[2] = ps_date->auc_year[1];
    c_intStr[3] = ps_date->auc_year[0];
    c_intStr[4] = '\0';

    sscanf(c_intStr, "%d", &tempInt);

    s_dateAlarm.tm_year = tempInt - 1900;

    c_intStr[0] = ps_date->auc_month[1];
    c_intStr[1] = ps_date->auc_month[0];
    c_intStr[2] = '\0';

    sscanf(c_intStr, "%d", &tempInt);
    s_dateAlarm.tm_mon = tempInt - 1;

    c_intStr[0] = ps_date->auc_day[1];
    c_intStr[1] = ps_date->auc_day[0];
    c_intStr[2] = '\0';

    sscanf(c_intStr, "%d", &tempInt);

    s_dateAlarm.tm_mday = tempInt;
    s_dateAlarm.tm_mday -= BATT_MAX_INTERVAL_IN_DAYS;

    s_dateAlarm.tm_sec = 0;
    s_dateAlarm.tm_min = 0;
    s_dateAlarm.tm_hour = 0;

    gs_dateAlarm = mktime(&s_dateAlarm);

#if DEBUG_BATT
    printf("(BATT): Low battery date is set to %02d/%02d/%02d.\n",
            s_dateAlarm.tm_mday, s_dateAlarm.tm_mon + 1,
            s_dateAlarm.tm_year - 100);
#endif /* DEBUG_BATT */
}
