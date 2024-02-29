/***************************************************************************//**
 * @brief
 * @file 	screenmng.c
 * @author 	Robson Cardoso
 * @date 	22/02/2017
 * @details
 * @section license
 * 			FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "screenmng.h"
#include "display.h"
#include "utils_fcx.h"
#include "systick.h"

#if DEBUG_SCREENMNG
#include <stdio.h>
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** Screen timeout in milliseconds. */
#define SCREENMNG_SCREEN_TIMEOUT_MS         10000ULL

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void setNetVolumeScreen(void);
static void setFlowRateScreen(void);
static void setReverseVolumeScreen(void);
static void setTemperatureScreen(void);
static void setReplacementDateScreen(void);
static void setTextScreen(void);
static void setEventIndicators(void);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Screen manager instance object. */
static s_screenmng_t gs_screenManager;
static bool gb_screenLocked;
static bool gb_blinkFeature = false;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
void screenmng_init(void)
{
    uint8_t uc_screenId;

    // The first registered screen is the default one. Therefore, we set it as
    // the one active at initialization
    gs_screenManager.uc_activeScreenId = 0;

    // Initialize remaining properties
    gs_screenManager.uc_screenCount             = SCREENMNG_MAX_SCREENS;
    gs_screenManager.ull_tmtInitSysTickMs       = systick_getSysTickMs();
    gs_screenManager.pf_isReverseFlow           = NULL;
    gs_screenManager.pf_isLeakage               = NULL;
    gs_screenManager.pf_isAirInPipe             = NULL;
    gs_screenManager.pf_isTampered              = NULL;
    gs_screenManager.pf_isRfTransmission        = NULL;
    gs_screenManager.pf_isPulseTransmission     = NULL;
    gs_screenManager.pf_isLowBattery            = NULL;
    gs_screenManager.pf_isBrush            = NULL;
    gs_screenManager.pf_isRevInstall            = NULL;
    gs_screenManager.pf_isNoUsage               = NULL;

    // Initialize screen array list
    for(uc_screenId = 0; uc_screenId < SCREENMNG_MAX_SCREENS; uc_screenId++)
    {
        screenmng_deleteScreen(uc_screenId);
    }

    gb_screenLocked = false;
}

void screenmng_registerEventIndicators(bool (*pf_isReverseFlow)(void),
                                       bool (*pf_isLeakage)(void),
                                       bool (*pf_isAirInPipe)(void),
                                       bool (*pf_isTampered)(void),
                                       bool (*pf_isRfTransmission)(void),
                                       bool (*pf_isPulseTransmission)(void),
                                       bool (*pf_isLowBattery)(void),
                                       bool (*pf_isBrush)(void),
                                       bool (*pf_isRevInstall)(void),
                                       bool (*pf_isNoUsage)(void),
                                       bool (*pf_isTempOut)(void))
{
    gs_screenManager.pf_isReverseFlow           = pf_isReverseFlow;
    gs_screenManager.pf_isLeakage               = pf_isLeakage;
    gs_screenManager.pf_isAirInPipe             = pf_isAirInPipe;
    gs_screenManager.pf_isTampered              = pf_isTampered;
    gs_screenManager.pf_isRfTransmission        = pf_isRfTransmission;
    gs_screenManager.pf_isPulseTransmission     = pf_isPulseTransmission;
    gs_screenManager.pf_isLowBattery            = pf_isLowBattery;
    gs_screenManager.pf_isBrush                 = pf_isBrush;
    gs_screenManager.pf_isRevInstall            = pf_isRevInstall;
    gs_screenManager.pf_isNoUsage               = pf_isNoUsage;
    gs_screenManager.pf_isTempOutside           = pf_isTempOut;
}

uint8_t screenmng_addScreen(s_screen_t const *const ps_screen)
{
    uint8_t uc_screenId = 0;

    // Find a gap in the screen pointer array
    while((gs_screenManager.aps_screens[uc_screenId] != NULL) &&
          (uc_screenId < SCREENMNG_MAX_SCREENS))
    {
        uc_screenId++;
    }

    // Sanity check
    ASSERT(uc_screenId < SCREENMNG_MAX_SCREENS);

    // Create a new screen if we have not reached the end of the list
    if(uc_screenId < SCREENMNG_MAX_SCREENS)
    {
        gs_screenManager.aps_screens[uc_screenId] = ps_screen;
        gs_screenManager.uc_screenCount++;

#if DEBUG_SCREENMNG
        printf("%llu (SCREENMNG): Adding screen %d.\n",
            systick_getSysTickMs(), uc_screenId);
#endif
    }

    return uc_screenId;
}

uint8_t screenmng_setScreen(uint8_t uc_screenId)
{
    if(uc_screenId < SCREENMNG_MAX_SCREENS)
    {
        gs_screenManager.uc_activeScreenId = uc_screenId;
        gs_screenManager.ull_tmtInitSysTickMs = systick_getSysTickMs();
    }
    else
    {
        uc_screenId = SCREENMNG_MAX_SCREENS;
    }

    return uc_screenId;
}

uint8_t screenmng_nextScreen(void)
{
    if(!gb_screenLocked)
    {
        if(gs_screenManager.uc_activeScreenId < 
            (gs_screenManager.uc_screenCount-1))
        {
            gs_screenManager.uc_activeScreenId++;
        }
        else
        {
            gs_screenManager.uc_activeScreenId = 0;
        }

        gs_screenManager.ull_tmtInitSysTickMs = systick_getSysTickMs();

#if DEBUG_SCREENMNG
    printf("%llu (SCREENMNG): Switching to screen %d.\n",
        systick_getSysTickMs(), gs_screenManager.uc_activeScreenId);
#endif
    }

    return gs_screenManager.uc_activeScreenId;
}

uint8_t screenmng_getActiveScreenId(void)
{
    return gs_screenManager.uc_activeScreenId;
}

void screenmng_update(void)
{
    // Check if a screen timeout has been occurred. If so switch to default 
    // screen
    if(gs_screenManager.uc_activeScreenId > 0)
    {
        if(systick_getSysTickMs() >= 
            (gs_screenManager.ull_tmtInitSysTickMs + SCREENMNG_SCREEN_TIMEOUT_MS))
        {
            gs_screenManager.uc_activeScreenId = 0;

#if DEBUG_SCREENMNG
        printf("%llu (SCREENMNG): Screen timeout. Returning to default screen\n",
            systick_getSysTickMs());
#endif
        }
    }

    // If current screen is valid update it according to its screen type
    if(gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId] != NULL)
    {
#if DEBUG_SCREENMNG
        printf("%llu (SCREENMNG): Updating screen %d.\n",
            systick_getSysTickMs(), gs_screenManager.uc_activeScreenId);
#endif

        // Clear all segments before setting current active screen
        display_clearAllSegments();

        switch(gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->e_screenType)
        {
            case E_SCREEN_TYPE_BLANK:

                // TODO: turn off display to save energy
                break;

            case E_SCREEN_TYPE_NET_VOLUME:

                setNetVolumeScreen();
                break;

            case E_SCREEN_TYPE_FLOWRATE:

                setFlowRateScreen();
                break;

            case E_SCREEN_TYPE_REV_VOLUME:

                setReverseVolumeScreen();
                break;

            case E_SCREEN_TYPE_TEMPERATURE:

                setTemperatureScreen();
                break;

            case E_SCREEN_TYPE_REPLACEMENT_DATE:

                setReplacementDateScreen();
                break;

            case E_SCREEN_TYPE_TEST:

                display_setAllSegments();
                break;

            case E_SCREEN_TYPE_TEXT:

                setTextScreen();
                break;
        }
    }

    // Print current screen
    display_print();
}

void screenmng_deleteScreen(uint8_t const uc_screenId)
{
    if(uc_screenId < SCREENMNG_MAX_SCREENS)
    {
        gs_screenManager.aps_screens[uc_screenId] = NULL;
        gs_screenManager.uc_screenCount--;
#if DEBUG_SCREENMNG
        printf("%llu (SCREENMNG): Deleting screen %d.\n",
            systick_getSysTickMs(), uc_screenId);
#endif
    }
}

bool screenmng_isBusy(void)
{
    return display_stillTransmitting();
}

void screenmng_lockScreen(bool b_lock)
{
    gb_screenLocked = b_lock;

#if DEBUG_SCREENMNG
    printf("%llu (SCREENMNG): Screen locked=%d.\n",
        systick_getSysTickMs(), gb_screenLocked);
#endif
}

bool screenmng_isScreenLocked()
{
    return gb_screenLocked;
}

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Set forward volume screen.
 */
static void setNetVolumeScreen(void)
{
    setEventIndicators();
    display_setDouble((*((double_t (*)(void))
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->pf_getData))(),
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->uc_doubleResolution);
    display_setIcon(E_ICON_M3);
}

/**
 * @brief
 *      Set flow rate screen.
 */
static void setFlowRateScreen(void)
{
    display_setDouble((*((double_t (*)(void))
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->pf_getData))(),
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->uc_doubleResolution);
    display_setIcon(E_ICON_M3);
    display_setIcon(E_ICON_PER_HOUR);
}

/**
 * @brief
 *      Set reverse volume screen.
 */
static void setReverseVolumeScreen(void)
{
    display_setDouble((*((double_t (*)(void))
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->pf_getData))(),
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->uc_doubleResolution);
    display_setIcon(E_ICON_REVERSE_FLOW);
    display_setIcon(E_ICON_M3);
}

/**
 * @brief
 *      Set temperature screen.
 */
static void setTemperatureScreen(void)
{
    display_setTemperature((*((double_t (*)(void))
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->pf_getData))());
}

/**
 * @brief
 *      Set substitution data screen.
 */
static void setReplacementDateScreen(void)
{
    display_setString((*((uint8_t* (*)(void))
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->pf_getData))());
    display_setIcon(E_ICON_LOW_BAT);
}

/**
 * @brief
 *      Set text screen.
 */
static void setTextScreen(void)
{
    display_setString((*((uint8_t* (*)(void))
        gs_screenManager.aps_screens[gs_screenManager.uc_activeScreenId]->pf_getData))());
}

/**
 * @brief
 *      Set event indicators.
 */
static void setEventIndicators(void)
{
    gb_blinkFeature = !gb_blinkFeature;

    if(gs_screenManager.pf_isReverseFlow != NULL)
    {
        if(gs_screenManager.pf_isReverseFlow())
        {
            display_setIcon(E_ICON_REVERSE_FLOW);
        }
    }

    if(gs_screenManager.pf_isLeakage != NULL)
    {
        if(gs_screenManager.pf_isLeakage())
        {
            display_setIcon(E_ICON_WATER);
        }
    }

    if(gs_screenManager.pf_isAirInPipe != NULL)
    {
        if(gs_screenManager.pf_isAirInPipe())
        {
            display_setIcon(E_ICON_ERROR);
        }
    }

    if(gs_screenManager.pf_isTampered != NULL)
    {
        if(gs_screenManager.pf_isTampered())
        {
            display_setIcon(E_ICON_ERROR);
        }
    }

    if(gs_screenManager.pf_isRfTransmission != NULL)
    {
        if(gs_screenManager.pf_isRfTransmission())
        {
            display_setIcon(E_ICON_TRANSMISSION);
        }
    }

    if(gs_screenManager.pf_isPulseTransmission != NULL)
    {
        if(gs_screenManager.pf_isPulseTransmission())
        {
            display_setIcon(E_ICON_TRANSMISSION);
        }
    }

    if(gs_screenManager.pf_isLowBattery != NULL)
    {
        if(gs_screenManager.pf_isLowBattery())
        {
            display_setIcon(E_ICON_LOW_BAT);
        }
    }

    if(gs_screenManager.pf_isBrush != NULL)
    {
        if(gs_screenManager.pf_isBrush())
        {
            display_setIcon(E_ICON_WATER);
        }
    }

    if(gs_screenManager.pf_isRevInstall != NULL)
    {
        if(gs_screenManager.pf_isRevInstall())
        {
            if(gb_blinkFeature)
            {
                display_setIcon(E_ICON_REVERSE_FLOW);
            }
        }
    }

    if(gs_screenManager.pf_isNoUsage != NULL)
    {
        if(gs_screenManager.pf_isNoUsage())
        {
            if(gb_blinkFeature)
            {
                display_setIcon(E_ICON_ERROR);
            }
        }
    }

    if(gs_screenManager.pf_isTempOutside != NULL)
    {
        if(gs_screenManager.pf_isTempOutside())
        {
            if(gb_blinkFeature)
            {
                display_setIcon(E_ICON_ERROR);
            }
        }
    }
}
