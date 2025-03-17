/*
 * DACDMA.h
 *
 *  Created on: Mar 7, 2025
 *      Credit: Aidan Walker and Jake Sheckler
 */

#ifndef DACDMA_H_
#define DACDMA_H_

#define BLOCKS 2
#define WAVE_SAMPLES_PER_BLOCK 64
#define WAVE_BYTES_PER_SAMPLE 2
#define WAVE_BYTES_PER_BUFFER    (WAVE_SAMPLES_PER_BLOCK*WAVE_BYTES_PER_SAMPLE)
#define WAVE_DMA_CH 0
#define SIZE_CODE_16BIT 1

/*****************************************************************************************
* Task Function Prototypes.
*   - Private if in the same module as startup task. Otherwise public.
*****************************************************************************************/
INT32U WaveGenPend(OS_TICK tout, OS_ERR *os_err_ptr);
void DMAinit(INT16U* sourceBlock);
void DACinit(void);
INT16U indexBufferPend(OS_TICK tout, OS_ERR *os_err);
void EDMA_0_CH0_IRQHandler(void);
void WaveInit(void);




#endif /* DACDMA_H_ */
