/***************************************************************************//**
 * @brief
 * @file    userint.c
 * @author  Robson Cardoso
 * @date    23/03/2017
 * @details
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "userint.h"
#include "types.h"
#include "optint.h"
#include "systick.h"

#if DEBUG_USERINT
#include <stdio.h>
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
/** User interface internal states. */
typedef enum E_USERINT_STATE
{
    /** Idle state. */
    E_USERINT_STATE_IDLE,
    /** Waiting pressing timeout state. */
    E_USERINT_STATE_WAITING_TIMEOUT
} E_USERINT_STATE_t;

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Internal FSM state. */
static E_USERINT_STATE_t ge_userIntState;

/** Store user interface command. */
static E_USERINT_CMD_t ge_userCmd;

/** Store the tick at which the optical interface has been pressed by the
 *  user. */
static uint64_t gull_lastTickMs;

/** Optical interface states. */
static E_OPTINT_STATE_t ge_optintCurrentState;
static E_OPTINT_STATE_t ge_optintPreviousState;

/** Change mode timeout in milliseconds. */
static uint16_t gui_changeModeTimeoutMs;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize user interface module.
 */
void userint_init(void)
{
    userint_resetState();

    ge_optintCurrentState = E_OPTINT_STATE_RELEASED;
    ge_optintPreviousState = E_OPTINT_STATE_RELEASED;

    gui_changeModeTimeoutMs = USERINT_CMD_DEFAULT_CHANGE_OP_MODE_TIMEOUT_MS;
    gull_lastTickMs = systick_getSysTickMs();
}

/**
 * @brief
 *      Update user interface internal state.
 *
 * @details
 *      This method updates user interface's internal state by tracking optical
 *      interface's state. If optical interface is pressed for more than @ref
 *      USERINT_CMD_CHANGE_OP_MODE_TIMEOUT_MS, it means the user wants to change
 *      operating mode. Otherwise, if it is pressed for less time than that, it
 *      just means the use wants to change screen.
 *
 *      If optical interface stays released, nothing should be done.
 */
void userint_update(void)
{
    // Update optical interface and determine its internal state
    ge_optintCurrentState = optint_getState();

    switch(ge_userIntState)
    {
        case E_USERINT_STATE_IDLE:

            if((E_OPTINT_STATE_PRESSED == ge_optintCurrentState) &&
               (E_OPTINT_STATE_RELEASED == ge_optintPreviousState))
            {
                gull_lastTickMs = systick_getSysTickMs();
                ge_userIntState = E_USERINT_STATE_WAITING_TIMEOUT;
                ge_userCmd = E_USERINT_CMD_CHANGE_SCREEN;

#if DEBUG_USERINT
                printf("%llu (USERINT): Change screen command.\n",
                    systick_getSysTickMs());
#endif
            }
            else
            {
                ge_userCmd = E_USERINT_CMD_NO_OP;
            }
            break;

        case E_USERINT_STATE_WAITING_TIMEOUT:

            if((E_OPTINT_STATE_PRESSED == ge_optintCurrentState) &&
               (E_OPTINT_STATE_PRESSED == ge_optintPreviousState))
            {
                if(systick_getSysTickMs() >=
                    (gull_lastTickMs + gui_changeModeTimeoutMs))
                {
                    ge_userIntState = E_USERINT_STATE_IDLE;
                    ge_userCmd = E_USERINT_CMD_CHANGE_OP_MODE;

#if DEBUG_USERINT
                    printf("%llu (USERINT): Change operating mode command.\n",
                        systick_getSysTickMs());
#endif
                    // Alert the timeout event
                    optint_reset();
                }
                else
                {
                    ge_userCmd = E_USERINT_CMD_NO_OP;
                }
            }
            else
            {
                ge_userIntState = E_USERINT_STATE_IDLE;
                ge_userCmd = E_USERINT_CMD_NO_OP;
            }
            break;
    }

    // Update optical interface's state previous state
    ge_optintPreviousState = ge_optintCurrentState;
}

/**
 * @brief
 *      Set change mode timeout in milliseconds.
 *
 * @param[in]   ui_timeoutMs
 *      Timeout in milliseconds.
 */
void userint_setChangeModeTimeoutMs(uint16_t ui_timeoutMs)
{
    gui_changeModeTimeoutMs = ui_timeoutMs;
}

/**
 * @brief
 *      Get user interface state.
 *
 * @return
 *      User interface's state. For more details, please refer to the
 *      enumeration definition @ref E_USERINT_CMD_t.
 */
E_USERINT_CMD_t userint_getState(void)
{
    return ge_userCmd;
}

/**
 * @brief
 *      Reset user interface state.
 */
void userint_resetState()
{
    ge_userCmd = E_USERINT_CMD_NO_OP;
    ge_userIntState = E_USERINT_STATE_IDLE;
}

#if DEBUG
void userint_setState(E_USERINT_CMD_t e_userCmd)
{
    ge_userCmd = e_userCmd;
    ge_optintCurrentState = E_OPTINT_STATE_PRESSED;
    ge_optintPreviousState = E_OPTINT_STATE_RELEASED;
}
#endif
