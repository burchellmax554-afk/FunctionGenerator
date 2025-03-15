/*
 * DACDMA.h
 *
 *  Created on: Mar 7, 2025
 *      Author: walke58
 */
#ifndef DACDMA_H_
#define DACDMA_H_

typedef struct {
    INT16U index;
    OS_SEM flag;
}type_indexBuffer;

void DACinit(void);
void DMAinit(void);
INT16U indexBufferPend(OS_TICK tout, OS_ERR *os_err);
void DMA0_Full_Major_IRQHandler(void);

#endif /* DACDMA_H_ */

