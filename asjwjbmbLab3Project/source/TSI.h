/*
 * TSI.h
 *
 *  Created on: Feb 15, 2025
 *      Author: jacobsheckler
 */

#ifndef TSI_H_
#define TSI_H_
#include "MCUType.h"
#include "os.h"


static void CTIMERInit(void);
//static void init_freemaster_lpuart(void);
void CTIMER0_IRQHandler(void);
#endif /* TSI_H_ */
