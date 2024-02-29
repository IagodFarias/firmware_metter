/***************************************************************************//**
 * @brief   Initializes the hardware abstraction layer.
 * @file    hal.h
 * @author  Robson Cardoso
 * @date    17/05/2016
 * @section license
 *          FAE Tecnologia. All rights reserved.
 ******************************************************************************/

#ifndef HAL_H_
#define HAL_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "USS_userConfig.h"

/*******************************************************************************
 * Defines Macros
 ******************************************************************************/
/* Clock system frequencies */
/*! DCO Frequency in Hz */
#define HAL_SYS_DCO_FREQ_HZ                         (8000000)
/*! MCLK Frequency (derived from DCO) */
#define HAL_SYS_MCLK_FREQ_HZ                        (USS_MCLK_FREQ_IN_HZ)
/*! SMCLK Frequency (derived from DCO) */
#define HAL_SYS_SMCLK_FREQ_HZ                       (USS_SMCLK_FREQ_IN_HZ)
/*! LFXT Frequency in Hz */
#define HAL_SYS_LFXT_FREQ_HZ                        (USS_LFXT_FREQ_IN_HZ)
/*! ACLK Frequency (derived from LFXT) */
#define HAL_SYS_ACLK_FREQ_HZ                        (USS_LFXT_FREQ_IN_HZ)
/*! Delay in cycles using ACLK */
#define HAL_SYS_MSDELAY_ACLK(x)         (((uint32_t)x*HAL_SYS_ACLK_FREQ_HZ)/1000)
/*! Delay in cycles using SMCLK */
#define HAL_SYS_MSDELAY_SMCLK(x)        (((uint32_t)x*HAL_SYS_SMCLK_FREQ_HZ)/1000)


/*******************************************************************************
 * Public functions
 ******************************************************************************/
void hal_init(void);

#endif /* HAL_H_ */
