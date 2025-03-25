/***************************************************************************//**
 * @brief
 * @file    userint.h
 * @author  Robson Cardoso
 * @date    23/03/2017
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

#ifndef USERINT_H
#define USERINT_H

#include "general_settings.h"

/*******************************************************************************
 * Defines
 ******************************************************************************/

/** Timeout required to trigger an operating mode change. */
#define USERINT_CMD_DEFAULT_CHANGE_OP_MODE_TIMEOUT_MS   6000ULL

/*******************************************************************************
 * Structures
 ******************************************************************************/

/** User command descriptor structure. */
typedef enum E_USERINT_CMD
{
    /** No operation command. */
    E_USERINT_CMD_NO_OP,
    /** Change screen command. */
    E_USERINT_CMD_CHANGE_SCREEN,
    /** Change operating mode command. */
    E_USERINT_CMD_CHANGE_OP_MODE
} E_USERINT_CMD_t;

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void userint_init(void);
void userint_update(void);
void userint_setChangeModeTimeoutMs(uint16_t ui_timeoutMs);
E_USERINT_CMD_t userint_getState(void);
void userint_resetState();
#if DEBUG
void userint_setState(E_USERINT_CMD_t e_userCmd);
#endif

#endif /* USERINT_H */
