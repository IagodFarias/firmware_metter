/*
 * probe.h
 *
 *  Created on: 3 de abr de 2019
 *      Author: Eric Costa Hall
 */

#ifndef PROBE_H_
#define PROBE_H_

#include "ussSwLib.h"

/*******************************************************************************
 * Public functions
 ******************************************************************************/

void probe_init(void);
void probe_enableDataTransmission(void);
void probe_disableDataTransmission(void);
void probe_update(void);

#endif /* PROBE_H_ */
