/***************************************************************************//**
 * @brief
 * @file    opmodemng.c
 * @author  Robson Cardoso
 * @date    01/03/2017
 * @details
 * @section license
 *          FAE Sistemas de Medi��o. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "opmodemng.h"
#include "configmng.h"
#include "general_settings.h"
#include "calib.h"
#include "verif.h"
#include "normal.h"
#include "precalib.h"
#include "standby.h"
#include "postcalib.h"
#include "comm.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/
// Post-calibration operating mode update interval in milliseconds
#define OPMODE_POST_CALIBRATION_UPDATE_INT_MS       1000ULL
#define OPMODE_POST_CALIBRATION_US_SAMPLING_RATE_HZ 0.0

#ifdef DEBUG_ASSERT
#define ASSERT(expr)    ((expr) ? ((void)0) : utils_assert(__FILE__, __LINE__))
#else
#define ASSERT(expr)    ((expr) ? ((void)0) : utils_assert(expr))
#endif /* DEBUG_ASSERT */

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/** Stores the operating mode which is currently running. */
static E_OPMODE_t ge_currentOpMode;

/** Stores the operating mode which was previously running. */
static E_OPMODE_t ge_previousOpMode;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

/**
 * @brief
 *      Initialize operating mode manager.
 *
 * @details
 *      Check whether this is the first time the meter is initialized. If it is
 *      the case, run pre-calibration operating mode. If the meter has been
 *      calibrated and activated, run normal operating mode. Otherwise, run
 *      stand-by operating mode.
 *
 * @note
 *      This logic assures that the calibration mode will be enabled just once
 *      during initialization. This also implies that the firmware should be
 *      downloaded again to allow for a new calibration.
 */
void opmodemng_init(void)
{

#if DEBUG_NORMAL
    ge_currentOpMode = E_OPMODE_STANDBY;
    ge_previousOpMode = E_OPMODE_STANDBY;
    opmodemng_setActivationFlag(true);
    opmodemng_setCalibrationFlag(true);
    standby_init();
#endif

    s_app_config_t *ps_config = configmng_getAppConfig();


    if(ps_config->b_isCalibrated)
    {
        if (ps_config->b_isActivated)
        {
            // Update FSM state
            ge_currentOpMode = E_OPMODE_NORMAL;
            ge_previousOpMode = E_OPMODE_NORMAL;

            // Initialize normal operating mode
            normal_init();
        }
        else
        {
            // Update FSM state
            ge_currentOpMode = E_OPMODE_STANDBY;
            ge_previousOpMode = E_OPMODE_STANDBY;

            // Initialize stand-by operating mode
            standby_init();
        }
    }
    else
    {
        // Update FSM state
        ge_currentOpMode = E_OPMODE_PRE_CALIBRATION;
        ge_previousOpMode = E_OPMODE_PRE_CALIBRATION;


#if DEBUG_NORMAL
            opmodemng_setCalibrationFlag(true);
            opmodemng_setActivationFlag(true);
            opmodemng_setPreviousOpMode(E_OPMODE_STANDBY);
            opmodemng_setCurrentOpMode(E_OPMODE_STANDBY);
            ge_currentOpMode = E_OPMODE_STANDBY;
            ge_previousOpMode = E_OPMODE_STANDBY;
            standby_init();
#else
            // Initialize pre-calibration operating mode
            precalib_init();
#endif
    }
}

/**
 * @brief
 *      Get active operating mode.
 *
 * @return
 *      Returns active operating mode. For more details, please refer to @ref
 *      E_OPMODE_t.
 */
E_OPMODE_t opmodemng_getCurrentOpMode(void)
{
    return ge_currentOpMode;
}

/**
 * @brief
 *      Get previous operating mode.
 *
 * @return
 *      Returns previous operating mode. For more details, please refer to @ref
 *      E_OPMODE_t.
 */
E_OPMODE_t opmodemng_getPreviousOpMode(void)
{
    return ge_previousOpMode;
}

/**
 * @brief
 *      Set active operating mode.
 *
 * @param[in]   e_opmode
 *      Operating mode. For more information, please refer to @ref E_OPMODE_t.
 */
void opmodemng_setCurrentOpMode(E_OPMODE_t e_opmode)
{
    // Set current mode as the previous mode
    opmodemng_setPreviousOpMode(ge_currentOpMode);

    // Change current mode
    ge_currentOpMode = e_opmode;
}

/**
 * @brief
 *      Set previous operating mode.
 *
 * @param[in]   e_opmode
 *      Operating mode. For more information, please refer to @ref E_OPMODE_t.
 */
void opmodemng_setPreviousOpMode(E_OPMODE_t e_opmode)
{
    ge_previousOpMode = e_opmode;
}

/**
 * @brief
 *      Run active operating mode.
 */
void opmodemng_update(void)
{
    // Run active operating mode
    switch(ge_currentOpMode)
    {
        case E_OPMODE_PRE_CALIBRATION:

            // Run pre-calibration operating mode
            precalib_run();
        break;

        case E_OPMODE_CALIBRATION:
            // Run calibration operating mode
            calib_run();
        break;

        case E_OPMODE_POST_CALIBRATION:

            // Run post-calibration
            postcalib_run();

            if(postcalib_testChangeToStandbyOpMode())
            {
                // Mark as calibrated
                if(opmodemng_setCalibrationFlag(true))
                {
                    // Initialize standby operating mode
                    standby_init();

                    // Set current operating mode
                    opmodemng_setCurrentOpMode(E_OPMODE_STANDBY);
                }
            }
        break;

        case E_OPMODE_STANDBY:

            // Run stand-by operating mode
            standby_run();

            if(standby_testChangeToVerifOpMode())
            {
                // Initialize normal operating mode
                verif_init();

                // Set active flag
                ASSERT(opmodemng_setActivationFlag(true));

                // Update FSM state
                opmodemng_setCurrentOpMode(E_OPMODE_VERIFICATION);
            }
#if CONFIG_OPTCOMM_ENABLED
            else if(standby_testChangeToCommOpmode())
            {
                // Initialize communication operating mode
                comm_init();

                // Update FSM state
                opmodemng_setCurrentOpMode(E_OPMODE_COMMUNICATION);
            }
#endif /* CONFIG_OPTCOMM_ENABLED */
        break;

        case E_OPMODE_NORMAL:
            // Run normal operating mode
            normal_run();

            if(normal_testChangeToVerifOpmode())
            {
                // Initialize verification operating mode
                verif_init();

                // Update FSM state
                opmodemng_setCurrentOpMode(E_OPMODE_VERIFICATION);
            }
#if CONFIG_OPTCOMM_ENABLED
            else if(normal_testChangeToCommOpmode())
            {
                // Initialize communication operating mode
                comm_init();

                // Update FSM state
                opmodemng_setCurrentOpMode(E_OPMODE_COMMUNICATION);
            }
#endif /* CONFIG_OPTCOMM_ENABLED */

        break;

        case E_OPMODE_VERIFICATION:
            // Run verification operating mode
            verif_run();

            if(verif_testChangeToNormalOpmode())
            {
                // Initialize normal operating mode
            normal_init();

                // Update FSM state
                opmodemng_setCurrentOpMode(E_OPMODE_NORMAL);
            }
#if CONFIG_OPTCOMM_ENABLED
            else if(verif_testChangeToCommOpmode())
            {
            // Initialize communication operating mode
                comm_init();

                // Update FSM state
                opmodemng_setCurrentOpMode(E_OPMODE_COMMUNICATION);
            }
#endif /* CONFIG_OPTCOMM_ENABLED */
        break;

#if CONFIG_OPTCOMM_ENABLED
        case E_OPMODE_COMMUNICATION:

            // Run communication operating mode
            comm_run();

            if(comm_testChangeToPreviousOpmode())
            {
                switch(ge_previousOpMode)
                {
                    case E_OPMODE_STANDBY:

                        // Initialize standby operating mode
                        standby_init();
                    break;

                    case E_OPMODE_VERIFICATION:

                        // Initialize verification operating mode
                        verif_init();
                    break;

                    default:

                        // Initialize normal operating mode
                        normal_init();

                        // Force the default mode
                        ge_previousOpMode = E_OPMODE_NORMAL;
                    break;
                }

                // Update FSM state
                opmodemng_setCurrentOpMode(ge_previousOpMode);
            }
            break;

        default:
            break;
#endif /* CONFIG_OPTCOMM_ENABLED */
    }
}

/**
 * @brief
 *      Set calibration flag.
 *
 * @param[in]   b_calibrated
 *      Indicates if the meter is calibrated or not.
 * @return
 *      Return 'true' if successfully set or 'false' otherwise.
 */
bool opmodemng_setCalibrationFlag(bool b_calibrated)
{
    bool b_status = true;
    s_app_config_t *ps_config = configmng_getAppConfig();

    if(ps_config->b_isCalibrated != b_calibrated)
    {
        ps_config->b_isCalibrated = b_calibrated;
        b_status = configmng_setAppConfig(ps_config);
    }

    return b_status;
}

/**
 * @brief
 *      Set activation flag.
 *
 * @param[in]   b_activated
 *      Indicates if the meter is activated or not.
 * @return
 *      Return 'true' if successfully set or 'false' otherwise.
 */
bool opmodemng_setActivationFlag(bool b_activated)
{
    bool b_status = true;
    s_app_config_t *ps_config = configmng_getAppConfig();

    if(ps_config->b_isActivated != b_activated)
    {
        ps_config->b_isActivated = b_activated;
        b_status = configmng_setAppConfig(ps_config);
    }

    return b_status;
}
