/***************************************************************************//**
 * @brief
 * @file 	screenmng.h
 * @author 	Robson Cardoso
 * @date 	14/02/2017
 * @details
 * @section license
 * 			FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

#ifndef SCREENMNG_H_
#define SCREENMNG_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "types.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** Maximum number of active screens. */
#define SCREENMNG_MAX_SCREENS           8

/*******************************************************************************
 * Structures
 ******************************************************************************/

/** Screen types enumeration. */
typedef enum E_SCREEN_TYPE
{
    /** Blank screen type. */
    E_SCREEN_TYPE_BLANK,
    /** Net volume screen type. */
    E_SCREEN_TYPE_NET_VOLUME,
    /** Flow rate screen type. */
    E_SCREEN_TYPE_FLOWRATE,
    /** Reverse volume screen type. */
    E_SCREEN_TYPE_REV_VOLUME,
    /** Temperature screen type. */
    E_SCREEN_TYPE_TEMPERATURE,
    /** Substitution date screen type. */
    E_SCREEN_TYPE_REPLACEMENT_DATE,
    /** Display test screen type. */
    E_SCREEN_TYPE_TEST,
    /** Text screen type. */
    E_SCREEN_TYPE_TEXT
 } E_SCREEN_TYPE_t;

/** Screen class structure. */
typedef struct S_SCREEN
{
    /** Type of the screen. */
    E_SCREEN_TYPE_t e_screenType;
    /** Pointer to a function which generates data to be printed on current
     *  screen. */
    void (*pf_getData)(void);
    /** In case the current screen prints a double, this field defines the
        screen's resolution. */
    uint8_t uc_doubleResolution;
} s_screen_t;

/** Screen manager class structure. */
typedef struct S_SCREENMNG
{
    /** Array of pointer to screens. */
    s_screen_t const *aps_screens[SCREENMNG_MAX_SCREENS];
    /** ID of the current active screen. */
    uint8_t uc_activeScreenId;
    /** Number of screens currently registered. */
    uint8_t uc_screenCount;
    /** Stores initial system tick to detect screen timeout. */
    uint64_t ull_tmtInitSysTickMs;
    /** Pointer to a function which indicates reverse flow is in progress. */
    bool (*pf_isReverseFlow)(void);
    /** Pointer to a function which indicates leakage is is in progress. */
    bool (*pf_isLeakage)(void);
    /** Pointer to a function which indicates there is air in pipe. */
    bool (*pf_isAirInPipe)(void);
    /** Pointer to a function which indicates is the meter was violated. */
    bool (*pf_isTampered)(void);
    /** Pointer to a function which indicates a RF transmission is in
        progress. */
    bool (*pf_isRfTransmission)(void);
    /** Pointer to a function which indicates a pulse transmission is in
        progress. */
    bool (*pf_isPulseTransmission)(void);
    /** Pointer to a function which indicates battery's date is old. */
    bool (*pf_isBatteryDateOld)(void);
    /** Pointer to a function which indicates brush. */
    bool (*pf_isBrush)(void);
    /** Pointer to a function which indicates reverse meter installation. */
    bool (*pf_isRevInstall)(void);
    /** Pointer to a function which indicates no usage. */
    bool (*pf_isNoUsage)(void);
    /** Pointer to a function which indicates high/low temperature. */
    bool (*pf_isTempOutside)(void);
} s_screenmng_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize screen manager.
 */
void screenmng_init(void);

/**
 * @brief
 *      Register event indicators.
 *
 * @details
 *      This method registers all event indicators at once.
 *
 * @param[in]   *pf_isReverseFlow
 *      Pointer to a function which indicates reverse flow is in progress.
 *
 * @param[in]   *pf_isLeakage
 *      Pointer to a function which indicates leakage is is in progress.
 *
 * @param[in]   *pf_isAirInPipe
 *      Pointer to a function which indicates there is air in pipe.
 *
 * @param[in]   *pf_isRfTransmission
 *      Pointer to a function which indicates a RF transmission is in
 *      progress.
 *
 * @param[in]   *pf_isPulseTransmission
 *      Pointer to a function which indicates a pulse transmission is in
 *      progress.
 *
 * @param[in]   *pf_isBatteryDateOld
 *      Pointer to a function which indicates battery's date is old.
 *
 * @param[in]   *pf_isBrush
 *      Pointer to a function which indicates Maximum Overflow.
 *
 * @param[in]   *pf_isRevInstall
 *      Pointer to a function which indicates reverse meter installation.
 *
 * @param[in]   *pf_isNoUsage
 *      Pointer to a function which indicates meter no usage.
 *
 * @param[in]   *pf_isTempOut
 *      Pointer to a function which indicates temperature outside limits.
 */
void screenmng_registerEventIndicators(bool (*pf_isReverseFlow)(void),
                                       bool (*pf_isLeakage)(void),
                                       bool (*pf_isAirInPipe)(void),
                                       bool (*pf_isTampered)(void),
                                       bool (*pf_isRfTransmission)(void),
                                       bool (*pf_isPulseTransmission)(void),
                                       bool (*pf_isBatteryDateOld)(void),
                                       bool (*pf_isBrush)(void),
                                       bool (*pf_isRevInstall)(void),
                                       bool (*pf_isNoUsage)(void),
                                       bool (*pf_isTempOut)(void));

/**
 * @brief
 *      Add a new screen.
 *
 * @param[in]   *ps_screen
 *      Pointer to the screen that should be added.
 *
 * @return
 *      Returns the position in the screen array at which the screen has been
 *      added:
 *      - If the return value is SCREENMNG_MAX_SCREENS then the screen could not
 *        be added to the array (there was insufficient space).
 *      - If the return value is less than SCREENMNG_MAX_SCREENS, then the
 *        screen was successfully added.
 *
 * @note
 *      This return value may be required, if a screen is to be subsequently
 *      deleted. For more information, refer to @ref screenmng_deleteScreen().
 */
uint8_t screenmng_addScreen(s_screen_t const *const ps_screen);

/**
 * @brief
 *      Switch to a given screen.
 *
 * @param[in]   uc_screenId
 *      ID of the screen to which is desired to switch.
 *
 * @return
 *      Returns SCREENMNG_MAX_SCREENS if @p uc_screenId exceeds the maximum
 *      number of screens allowed. Otherwise, it returns @p uc_screenId.
 */
uint8_t screenmng_setScreen(uint8_t uc_screenId);

/**
 * @brief
 *      Switch to next screen.
 *
 * @details
 *      This method switches to the next screen and wrap up to the default
 *      screen (first one) if the current active screen is the last one in the
 *      screen list.
 *
 * @return
 *      Returns the ID of the active screen after the switch screen action.
 */
uint8_t screenmng_nextScreen(void);

/**
 * @brief
 *      Get the ID of the active screen.
 *
 * @return
 *      Returns the ID of the active screen.
 */
uint8_t screenmng_getActiveScreenId(void);

/**
 * @brief
 *      Update active screen.
 */
void screenmng_update(void);

/**
 * @brief
 *      Delete a screen.
 *
 * @param[in]   uc_screenId
 *      ID of the screen that should be deleted.
 */
void screenmng_deleteScreen(uint8_t const uc_screenId);

/**
 * @brief
 *      Check if there is any pending data to be sent to LCD controller.
 *
 * @return
 *      Returns 'true' if there is pending data to be sent to LCD controller.
 */
bool screenmng_isBusy(void);

/**
 * @brief
 *      Set to lock screen function.
 *
 * @param[in]   b_lock
 *      If 'true' the interface will be lock, otherwise will unlock.
 */
void screenmng_lockScreen(bool b_lock);

/**
 * @brief
 *      Check if the screen is locked or not.
 *
 * @return
 *      Returns 'true' if the screen is locked, otherwise return false.
 */
bool screenmng_isScreenLocked();

#endif /* SCREENMNG_H_ */
