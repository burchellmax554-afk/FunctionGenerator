/*
 * DACDMA.h
 *
 *  Created on: Mar 7, 2025
 *      Author: walke58
 */

#ifndef DACDMA_H_
#define DACDMA_H_

#define WAVE_SAMPLES_PER_BLOCK 64
#define WAVE_BYTES_PER_SAMPLE 2 //bytes in each sample from data source
#define WAVE_BYTES_PER_BUFFER   (WAVE_SAMPLES_PER_BLOCK*WAVE_BYTES_PER_SAMPLE)
#define WAVE_DMA_CH 0
#define SIZE_CODE_16BIT 1
#define BLOCKS 2


/*****************************************************************************************
* Task Function Prototypes.
*   - Private if in the same module as startup task. Otherwise public.
*****************************************************************************************/
INT32U WaveGenPend(OS_TICK tout, OS_ERR *os_err_ptr);
void DMAinit(INT16U* sourceBlock);
void DACinit(void);
INT16U indexBufferPend(OS_TICK tout, OS_ERR *os_err);
void EDMA_0_CH0_IRQHandler(void);
void WaveGenInit(void);




#endif /* DACDMA_H_ */
