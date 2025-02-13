/***************************************************************************//**
 * @brief
 * @file    battdate.c
 * @author  Robson Cardoso
 * @date    14/11/2024
 * @details
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "battdate.h"
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
#if DEBUG
/** Battery detection interval. */
#define BATT_DETECTION_INT_MS                   10000ULL
#else
/** Battery detection interval. */
#define BATT_DATE_DETECTION_INT_MS                   3600000ULL
#endif
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
static void calculateOldBattDate(void);
/*******************************************************************************
 * Global variables
 ******************************************************************************/

static bool gb_isBatteryDateOld;
static time_t gs_dateAlarm;
static uint32_t gul_batteryDetectionCounter;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void battdate_init(void)
{
    // Configure low battery date time
    calculateOldBattDate();

    gb_isBatteryDateOld = false;
}

void battdate_update(void)
{
    if(gul_batteryDetectionCounter > BATT_DATE_DETECTION_INT_MS){
        gul_batteryDetectionCounter = 0;
        struct tm *ps_actualDateTime = systick_getActualDateTime();

    #if DEBUG_BATT
        mktime(ps_actualDateTime);

        printf("(BATT): Now is %02d/%02d/%02d.\n",
                ps_actualDateTime->tm_mday, ps_actualDateTime->tm_mon + 1,
                ps_actualDateTime->tm_year - 100);
    #endif /* DEBUG_BATT */

        gb_isBatteryDateOld =
                difftime(mktime(ps_actualDateTime), gs_dateAlarm) > 0 ? true : false;

    #if DEBUG_BATT
        mktime(ps_actualDateTime);

        printf("(BATT): gb_isBatteryDateOld is %d.\n",(int) gb_isBatteryDateOld);
    #endif /* DEBUG_BATT */
    }
    else{
        gul_batteryDetectionCounter += systick_getWakeUpIntMs();
    }
}

bool battdate_isOld(void)
{
    return gb_isBatteryDateOld;
}

void battdate_updateConfig(void)
{
    // Update the low battery date time
    calculateOldBattDate();
}

static void calculateOldBattDate(void)
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
