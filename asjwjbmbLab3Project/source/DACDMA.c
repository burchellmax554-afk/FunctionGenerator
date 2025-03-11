/*
 * DACDMA.c
 *
 *  Created on: Mar 7, 2025
 *      Author: walke58
 */

#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947_GPIO.h"
#include "DACDMA.h"
#include "assert.h"

#define WAVE_SAMPLES_PER_BLOCK 91
#define WAVE_BYTES_PER_SAMPLE 2 //bytes in each sample from data source
#define WAVE_BYTES_PER_BLOCK   (WAVE_SAMPLES_PER_BLOCK*WAVE_BYTES_PER_SAMPLE)
#define WAVE_DMA_CH 0
#define SIZE_CODE_16BIT 1
#define NUM_BLOCKS 2

INT16S DMABuffer[NUM_BLOCKS][WAVE_SAMPLES_PER_BLOCK]; // Ping Pong Buffer = 2x91 array
type_indexBuffer indexBuffer; //'type_indexBuffer' struct has 2 members: '.flag' and '.counter'

/*******************************************************************************************
 *  DACinit():
 *      Initialize DAC2 to receive 14 bits from DMA0 and construct sinusoidal signal
 ******************************************************************************************/
void DACinit(void){
    SYSCON0->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_DAC2(1); /* Enable clock for DAC2   */
    SYSCON0->DAC[2].CLKSEL = SYSCON_DAC_CLKSEL_SEL(1); /* Select pll0_clk */
    SYSCON0->DAC[2].CLKDIV = 2; /* Setup DAC2's clock divider (150/3 = 50MHz) */
    while (SYSCON0->DAC[2].CLKDIV & SYSCON_DAC_CLKDIV_UNSTAB(1)) {
        /* Wait for DAC2CLKDIV to stabilize */
    }
    SPC0->ACTIVE_CFG1 |= SPC_ACTIVE_CFG1_SOC_CNTRL(0x11); /* Enable DAC0 and VREF power*/
    SYSCON0->PRESETCTRLSET[1] = SYSCON_PRESETCTRL1_DAC2_RST(1); /* Set DAC0 reset      */
    SYSCON0->PRESETCTRLCLR[1] = SYSCON_PRESETCTRL1_DAC2_RST(1); /* Clear DAC0 reset    */

    /* Set up DAC0, current sources from VREF, V_ANA reference, buffer enable */
    DAC2->GCR = LPDAC_GCR_IREF_ZTC_EXT_SEL(1)|LPDAC_GCR_IREF_PTAT_EXT_SEL(1)|
                LPDAC_GCR_DACRFS(0x00)|LPDAC_GCR_BUF_EN(1);
    /* Turn it on */
    DAC2->GCR |= LPDAC_GCR_DACEN(1);
}

/*******************************************************************************************
 *  DMAinit():
 *      Configure CTIMER2 to periodically trigger DMA to
 *      transfer data from DMABuffer to DAC2. Enable INTFULL interrupt for ping-pong switching
 *      between DMABuffer[0] and DMABuffer[1]
 ******************************************************************************************/
void DMAinit(void){

        OS_ERR os_err;

        //Create Semaphore 'Index Buffer Flag'
        OSSemCreate(&indexBuffer.flag, "Index Buffer Flag", 0, &os_err);
        assert(os_err == OS_ERR_NONE);

        SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_DMA0(1); //enable clock for DMA channel 0
        DMA0->CH[WAVE_DMA_CH].TCD_SADDR = DMA_TCD_SADDR_SADDR(DMABuffer[0]); //contains memory address pointing to source data (DMABuffer)
        DMA0->CH[WAVE_DMA_CH].TCD_SOFF = DMA_TCD_SOFF_SOFF(WAVE_BYTES_PER_SAMPLE); //offset applied to the current source address after each read

        //Source size is 2 bytes, destination size is 2 bytes. No source or destination modulo feature.
        DMA0->CH[WAVE_DMA_CH].TCD_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(SIZE_CODE_16BIT)
                                     | DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(SIZE_CODE_16BIT);

        //After major loop is finished set the address back to the first byte of the block (negative offset)
        DMA0->CH[WAVE_DMA_CH].TCD_SLAST_SDA = DMA_TCD_SLAST_SDA_SLAST_SDA(-(WAVE_BYTES_PER_BLOCK));

        //Set destination address to the DAC data register
        DMA0->CH[WAVE_DMA_CH].TCD_DADDR = DMA_TCD_DADDR_DADDR(&DAC2->DATA);

        //No destination address offset
        DMA0->CH[WAVE_DMA_CH].TCD_DOFF = DMA_TCD_DOFF_DOFF(0);

        //No adjustment to destination address after major loop completion.
        DMA0->CH[WAVE_DMA_CH].TCD_DLAST_SGA = DMA_TCD_DLAST_SGA_DLAST_SGA(0);

        //Minor loop size is the sample size
        DMA0->CH[WAVE_DMA_CH].TCD_NBYTES_MLOFFNO = DMA_TCD_NBYTES_MLOFFNO_NBYTES(WAVE_BYTES_PER_SAMPLE);


        //Set minor loop iteration counters to number of minor loops in the major loop (CITER=BITER required for initialization)
        DMA0->CH[WAVE_DMA_CH].TCD_CITER_ELINKNO = DMA_TCD_CITER_ELINKNO_ELINK(0)| //Channel-channel linking disabled
                                                  DMA_TCD_CITER_ELINKNO_CITER(WAVE_SAMPLES_PER_BLOCK); //Current Major Iteration Count

        DMA0->CH[WAVE_DMA_CH].TCD_BITER_ELINKNO = DMA_TCD_BITER_ELINKNO_ELINK(0)| //Channel-channel linking disabled
                                                  DMA_TCD_BITER_ELINKNO_BITER(WAVE_SAMPLES_PER_BLOCK);//Starting Major Iteration Count

        //Initialize the rest of the TCD. 8 cycle stall after each R/W. Enable full major interrupts.
        DMA0->CH[WAVE_DMA_CH].TCD_CSR = DMA_TCD_CSR_BWC(3)|DMA_TCD_CSR_INTHALF(0)|DMA_TCD_CSR_INTMAJOR(1);


        //Configure CTIMER2 Match 0 for trigger
        /* Clock setup */
        SYSCON0->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_TIMER2(1); /* Enable clock for CTIMER2 */
        SYSCON0->CTIMERCLKSEL[2] = SYSCON_CTIMERCLKSEL_SEL(1); /* Select pll0_clk, 150MHz, as CTIMER2's clock */
        SYSCON0->CTIMERCLKDIV[2] = 0; /* Setup CTIMER2's clock divider (no division) */
        while (SYSCON0->CTIMERCLKDIV[2] & SYSCON_CTIMERXCLKDIV_CTIMERCLKDIV_UNSTAB(1)) {
            /* Wait for CTIMER2CLKDIV to stabilize */
        }
        CTIMER2->TCR = CTIMER_TCR_CEN(0);  /* Disable timer */
        CTIMER2->MCR = CTIMER_MCR_MR0R(1); /* Set CTIMER2 to reset counter on match 0 */
        CTIMER2->PR = CTIMER_PR_PRVAL(0);  /* Set prescale to divide by 1) */
        CTIMER2->MR[0] = CTIMER_MR_MATCH(3125 - 1); /* Set MR0 to give CTIMER2 a 48000-cycle period */
        CTIMER2->TCR |= CTIMER_TCR_CEN(1); /* Enable CTIMER2 */

        //Input source is CTIMER2 Match 0
        DMA0->CH[WAVE_DMA_CH].CH_MUX = DMA_CH_MUX_SRC(11);

        //All set to go, enable DMA channel!
        DMA0->CH[WAVE_DMA_CH].CH_CSR = DMA_CH_CSR_ERQ(1);
}



/*******************************************************************************************
 *  DMA_ChannelX_Full_Major_IRQHandler():
 *      Interrupt handler signaled by flag occurring when one block of the ping-pong is full.
 *      Posts semaphore that the signal processing task pends on, which shall receive the index
 *      (either 0 or 1) signaling the task to begin writing to the [index] block of the ping pong.
 ******************************************************************************************/
void DMA0_Full_Major_IRQHandler(void){

    OS_ERR os_err;

    DMA0->CH[WAVE_DMA_CH].CH_INT = DMA_CH_INT_INT(0);
    indexBuffer.index=indexBuffer.index^1;

    OSSemPost(&indexBuffer.flag, OS_OPT_POST_1, &os_err);
    assert(os_err == OS_ERR_NONE);

}

/*******************************************************************************************
 *  indexBufferPend():
 *      indexBufferPend() will move the pending task to the wait state until the indexBuffer semaphore
 *      is posted by the IRQ handle. When the indexBuffer semaphore is signaled, indexBufferPend() will
 *      return the index of the block the task needs to write to next.
 ******************************************************************************************/
INT16U indexBufferPend(OS_TICK tout, OS_ERR *os_err){

    OSSemPend(&indexBuffer.flag, tout, OS_OPT_PEND_BLOCKING,(CPU_TS *)0, os_err);
    assert(*os_err == OS_ERR_NONE);
    return indexBuffer.index;
}


