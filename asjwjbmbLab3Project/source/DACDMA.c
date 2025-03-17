/*****************************************************************************************
* WaveGenDMA.c - CTIMER-based DAC2 sinewave output
* This module generates a sinewave output using DAC2 and DMA with a ping-pong
* buffer mechanism for continuous waveform output. It includes initialization
* of the DAC, DMA, and CTIMER for synchronized waveform generation.
* Todd Morton, 02/11/2025
* Bernhardt Krause, 03/15/2025
* Reese Bergeson, 03/15/2025
* Jake Sleppy, 03/16/2025
 ****************************************************************************************/
#include "MCUType.h"
#include "FRDM_MCXN947_GPIO.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "arm_math.h"
#include "os.h"
#include "assert.h"
#include "app_cfg.h"
#include "BasicIO.h"
#include "state.h"
#include "DACDMA.h"
//#include "WaveGenDMA.h"
/**********************************************************************************
* Allocate task control blocks
**********************************************************************************/
static OS_TCB generateSineTableTaskTCB;
/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK generateSineTableTaskStk[APP_CFG_GENERATE_SINE_TABLE_TASK_STK_SIZE];

/****************************************************************************
 *  Define DMA and waveform buffer settings
 ******************************************************************************/
#define BLOCKS 2
#define WAVE_SAMPLES_PER_BLOCK 64
#define WAVE_BYTES_PER_SAMPLE 2
#define WAVE_BYTES_PER_BUFFER    (WAVE_SAMPLES_PER_BLOCK*WAVE_BYTES_PER_SAMPLE)
#define WAVE_DMA_CH 0
#define SIZE_CODE_16BIT 1

/******************************************************************************
 *  Ping-Pong Buffer structure
 ******************************************************************************/
typedef struct{
    INT8U count; /* Buffer count (alternates from 0 to 1 for ping-pong) */
    OS_SEM flag; /* Semaphore to synchronize access*/
} WG_PING_PONG_BUFFER_T;

/******************************************************************************
 *  Global Ping-Pong Buffer instance
 ******************************************************************************/
static WG_PING_PONG_BUFFER_T pingPong;
static INT16U pingPongBuffer[BLOCKS][WAVE_SAMPLES_PER_BLOCK / 2];

/*******************************************************************************
 *  Function Prototypes
 *******************************************************************************/
static void generateSineTableTask(void *p_arg);
static void dacInit(void);
static void wgDMAInit(INT16U *out_block);

/****************************************************************************************
* Function: WaveGenPend
* Purpose: When called, wait for a signal from the Ping-Pong buffer semaphore.
* Then return the current Ping-Pong buffer index.
* Parameters:
*   - tout: Timeout duration in system ticks.
*   - os_err_ptr: Pointer to the operating system error status variable.
* Returns: Current Ping-Pong buffer index.
* Reese Bergeson, 03/15/2025
****************************************************************************************/
INT32U WaveGenPend(OS_TICK tout, OS_ERR *os_err_ptr) {
    OSSemPend(&(pingPong.flag), tout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, os_err_ptr);
    assert(*os_err_ptr == OS_ERR_NONE);
    return(pingPong.count); /* Return current buffer index */
}

/****************************************************************************************
* Function: WaveGenInit
* Purpose: Initialize sine table generation task, dac,dma, semaphore for
* Ping-Pong buffer, and peripherals.
* Todd Morton, 02/11/2025
* Reese Bergeson, 03/15/2025
****************************************************************************************/
void WaveGenInit(void){
    OS_ERR os_err;
    OSSemCreate(&(pingPong.flag), "Ping Pong Buffer Semaphore", 0, &os_err);
    assert(os_err == OS_ERR_NONE);

    dacInit();
    /* Initialize DMA with pointer to 2 dimensional ping-pong buffer array */
    wgDMAInit(&pingPongBuffer[0][0]);

    OSTaskCreate(&generateSineTableTaskTCB,
                "Generate Sine Table Task",
                generateSineTableTask,
                (void *) 0,
                APP_CFG_GENERATE_SINE_TABLE_TASK_PRIO,
                &generateSineTableTaskStk[0],
                (APP_CFG_GENERATE_SINE_TABLE_TASK_STK_SIZE / 10u),
                APP_CFG_GENERATE_SINE_TABLE_TASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                &os_err);
    assert(os_err == OS_ERR_NONE);
}

/****************************************************************************************
 * Function: dacInit
 * Purpose: Initialize, configure and enable DAC2 for waveform generation.
 * Todd Morton, 02/11/2025
 * Bernhardt Krause, 03/15/2025
 * *************************************************************************************/
static void dacInit(void){
    SYSCON0->AHBCLKCTRLSET[3] = SYSCON_AHBCLKCTRL3_DAC2(1); /* Enable clock for DAC2 */
    SYSCON0->DAC[2].CLKSEL = SYSCON_DAC_CLKSEL_SEL(1); /* Select pll0_clk */
    SYSCON0->DAC[2].CLKDIV = 2; /* Setup DAC2's clock divider (150/3 = 50MHz) */
    while (SYSCON0->DAC[2].CLKDIV & SYSCON_DAC_CLKDIV_UNSTAB(1)) {
    /* Wait for DAC2CLKDIV to stabilize */
    }
    SPC0->ACTIVE_CFG1 |= SPC_ACTIVE_CFG1_SOC_CNTRL(0x41); /* Enable DAC2 and VREF power*/
    SYSCON0->PRESETCTRLSET[3] = SYSCON_PRESETCTRL3_DAC2_RST(1); /* Set DAC2 reset */
    SYSCON0->PRESETCTRLCLR[3] = SYSCON_PRESETCTRL3_DAC2_RST(1); /* Clear DAC2 reset */

    /* Set up DAC2, current sources from VREF, V_ANA reference, buffer enable */
    DAC2->GCR = LPDAC_GCR_BUF_EN(1);

    /* Turn it on */
    DAC2->GCR |= LPDAC_GCR_DACEN(1);
}

/****************************************************************************************
 * Function: wgDMAInit
 * Purpose: Configure DMA for Ping-Pong buffer data transfer.
 * Parameters:
 *          - out_block: Pointer to the initial output data block.
 * Todd Morton, 02/11/2025
 * Bernhardt Krause, 03/15/2025
 ****************************************************************************************/
static void wgDMAInit(INT16U *out_block) {
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_DMA0(1); /* Enable DMA Clock */

    DMA0->CH[WAVE_DMA_CH].TCD_SADDR = DMA_TCD_SADDR_SADDR(out_block);
    DMA0->CH[WAVE_DMA_CH].TCD_SOFF = DMA_TCD_SOFF_SOFF(WAVE_BYTES_PER_SAMPLE);  /* Move by 2 bytes per transfer */

    /*Source size is 2 bytes, destination size is 2 bytes. No modulo feature.*/
    DMA0->CH[WAVE_DMA_CH].TCD_ATTR = DMA_TCD_ATTR_SMOD(0) | DMA_TCD_ATTR_SSIZE(SIZE_CODE_16BIT)
                                 | DMA_TCD_ATTR_DMOD(0) | DMA_TCD_ATTR_DSIZE(SIZE_CODE_16BIT);

    /* When major loop is completed, address is set back to the first byte of
     * the block*/
    DMA0->CH[WAVE_DMA_CH].TCD_SLAST_SDA = DMA_TCD_SLAST_SDA_SLAST_SDA(-(WAVE_BYTES_PER_BUFFER));

    /* Destination Address (DAC2 Data Register)*/
    DMA0->CH[WAVE_DMA_CH].TCD_DADDR = DMA_TCD_DADDR_DADDR(&DAC2->DATA);

    DMA0->CH[WAVE_DMA_CH].TCD_DOFF = DMA_TCD_DOFF_DOFF(0); /* No address increment */

    DMA0->CH[WAVE_DMA_CH].TCD_DLAST_SGA = DMA_TCD_DLAST_SGA_DLAST_SGA(0);

    /* Set Minor Loop Size (One sample per transfer)*/
    DMA0->CH[WAVE_DMA_CH].TCD_NBYTES_MLOFFNO = DMA_TCD_NBYTES_MLOFFNO_NBYTES(WAVE_BYTES_PER_SAMPLE);

    /* Set Major Loop Count (Full Buffer Transfer)*/
    DMA0->CH[WAVE_DMA_CH].TCD_CITER_ELINKNO = DMA_TCD_CITER_ELINKNO_ELINK(0) | DMA_TCD_CITER_ELINKNO_CITER(WAVE_SAMPLES_PER_BLOCK);
    DMA0->CH[WAVE_DMA_CH].TCD_BITER_ELINKNO = DMA_TCD_BITER_ELINKNO_ELINK(0) | DMA_TCD_BITER_ELINKNO_BITER(WAVE_SAMPLES_PER_BLOCK);

    /* Enable Half and Full Buffer Interrupts*/
    DMA0->CH[WAVE_DMA_CH].TCD_CSR = DMA_TCD_CSR_BWC(3) | DMA_TCD_CSR_INTHALF(1) | DMA_TCD_CSR_INTMAJOR(1);

    /*Configure CTIMER2 Match 0 for trigger */
    /* Clock setup */
    SYSCON0->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_TIMER2(1); /* Enable clock for CTIMER2 */
    SYSCON0->CTIMERCLKSEL[2] = SYSCON_CTIMERCLKSEL_SEL(1); /* Select pll0_clk, 150MHz, as CTIMER2's clock */
    SYSCON0->CTIMERCLKDIV[2] = 0; /* Setup CTIMER2's clock divider (no division) */
    while (SYSCON0->CTIMERCLKDIV[2] & SYSCON_CTIMERXCLKDIV_CTIMERCLKDIV_UNSTAB(1)) {
        /* Wait for CTIMER0CLKDIV to stabilize */
    }
    CTIMER2->TCR = CTIMER_TCR_CEN(0);  /* Disable timer */
    CTIMER2->MCR = CTIMER_MCR_MR0R(1); /* Set CTIMER2 to reset counter on match 0 */
    CTIMER2->PR = CTIMER_PR_PRVAL(0);  /* Set prescale to divide by 1) */
    CTIMER2->MR[0] = CTIMER_MR_MATCH(780 - 1); /* Set MR0 to give CTIMER2 a 192000-cycle period */
    CTIMER2->TCR |= CTIMER_TCR_CEN(1); /* Enable CTIMER2 */

    /*Input source is CTIMER2 Match 0*/
    DMA0->CH[WAVE_DMA_CH].CH_MUX = DMA_CH_MUX_SRC(11);

    NVIC_ClearPendingIRQ(EDMA_0_CH0_IRQn);
    NVIC_EnableIRQ(EDMA_0_CH0_IRQn);

    /*All set to go, enable DMA channel!*/
    DMA0->CH[WAVE_DMA_CH].CH_CSR = DMA_CH_CSR_ERQ(1);
}

/******************************************************************************************
 * Function: EDMA_0_CH0_IRQHandler
 * Purpose: Handle DMA interrupt, toggle Ping-Pong buffers, and post semaphore.
 * Reese Bergeson, 03/15/2025
 * ******************************************************************************************/
void EDMA_0_CH0_IRQHandler(void) {
    OS_ERR os_err;
    OSIntEnter();
    DB3_TURN_ON();
    DMA0->CH[0].CH_INT = DMA_CH_INT_INT(1); /* Clear DMA interrupt flag */
    /* Check the DONE flag in the DMA channel Status Register */
    if (DMA0->CH[WAVE_DMA_CH].CH_CSR & DMA_CH_CSR_DONE_MASK) {
        pingPong.count = 1; /* Switch to the secondary buffer (ping) */
    }
    else {
        pingPong.count = 0; /* Switch to the primary buffer (pong) */
    }
    /* Post to the semaphore to signal that the buffer is ready */
    OSSemPost(&pingPong.flag, OS_OPT_POST_1, &os_err);
    assert(os_err == OS_ERR_NONE);
    DB3_TURN_OFF();
    OSIntExit();
}

/******************************************************************************
 * Function: generateSineTableTask
 * Purpose: Generates a buffer holdign sine wave values for DMA transfer. Update
 * sine wave parameters from settings and calculates the table values.
 * Parameters:
 *          - p_arg: Not used task argument
 * Notes:
 *             pingPongBuffer[index][i] = (INT16U) (((scale * ((arm_sin_q31(Xarg)) >> 13)) >> 18) + 8191);
 *               Steps:
 *                1. Call arm_sin_q31(Xarg) to compute the sine value for the current phase (Xarg). The result is in Q31 format.
 *                2. Right-shift the sine value by 13U to normalize it to a smaller range.
 *                3. Multiply the scaled sine value by the amplitude scaling factor (scale). This adjusts the sine wave's amplitude.
 *                4. Right-shift the result by 18U to fit into the desired resolution for the buffer.
 *                5. Add an offset of 8191U to ensure the sine wave values are positive and within the buffer's range.
 * Reese Bergeson: Implementation of all math used to generate ping pong buffer indices.
 * Jake Sleppy: Implementation of mutex_counter if statement which minimizes cpu load by preventing continuous updates to
 *              'localFreq' and 'localAmp'.
 *****************************************************************************/

static void generateSineTableTask(void *p_arg) {
    OS_ERR os_err;
    INT16U scale;
    INT32U index;
    static INT16U phaseStep = 11185;
    static q31_t Xarg = 0;
    INT32U i;
    INT16U localFreq;
    INT16U localAmp;
    INT8U mutex_counter = 0;

    (void)p_arg; /* Suppress unused parameter warning */

    while (1) {
        index = WaveGenPend(0, &os_err);
        assert(os_err == OS_ERR_NONE);
        mutex_counter = mutex_counter + 1;
        /* Get updated settings */
        if (mutex_counter == 100) {

            localFreq = (INT16U)current_state.sine_frequency;  /* Frequency in Hz */
            localAmp = (INT16U)current_state.sine_amplitude;    /* Quadrature encoder output (0-20)*/
            mutex_counter = 0;
        } else {
            /* Do nothing */
        }


        scale = localAmp * 372;

        /* Loop through half the buffer to populate sine wave values in that half*/
        for (i = 0; i < WAVE_SAMPLES_PER_BLOCK / 2; i++) {
            /* Increment phase accumulator based on frequency*/
            Xarg = Xarg + phaseStep * localFreq;
            /* Calculate the sine wave sample value */
            pingPongBuffer[index][i] = (INT16U) (((scale * ((arm_sin_q31(Xarg)) >> 13)) >> 18) + 8191);
        }
        //OSTimeDly(1,OS_OPT_TIME_DLY,&os_err);
    }

}
