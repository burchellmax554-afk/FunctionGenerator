/*
 * CTIMER.c
 *
 *  Created on: Mar 2, 2025, Last modified 02MAR2025
 *      Author: John Brown
 */


/****************************************************************************************
* CTIMER PWM Test
* Quick demo of the configuration of the CTIMER to generate a pulse train
*
* TDM, 01/29/2025
*****************************************************************************************
* Include Header Files
****************************************************************************************/
#include "MCUType.h"
#include "FRDM_MCXN947ClkCfg.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ctInit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
INT32U periodValue;
INT32U dutyValue;
/*******************************************************************************
 * main code
 ******************************************************************************/
void main(void){

    FRDM_MCXN947InitBootClock();
    ctInit();

    while(1){
    }
}

/***********************************************************************************
 * Configure CTIMER0 to generate a 1kHz square wave on P0_11 using pll0_clk (150MHz)
 * Sam Johnson
 * Todd Morton
 **********************************************************************************/
void ctInit(void){
    /* Enable clock gate for CTIMER0 */
   SYSCON0->AHBCLKCTRLSET[1] = SYSCON_AHBCLKCTRL1_TIMER0(1);
   /* Select pll0_clk as CTIMER0's clock */
   SYSCON0->CTIMERCLKSEL[0] = SYSCON_CTIMERCLKSEL_SEL(1);
   /* Setup CTIMER0's clock divider and wait for it to stabilize */
   SYSCON0->CTIMERCLKDIV[0] = 0;
   while (SYSCON0->CTIMERCLKDIV[0] & SYSCON_CTIMERXCLKDIV_CTIMERCLKDIV_UNSTAB(1)){}
   /* Enable clock gate for PORT0 */
   SYSCON0->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT0(1);
   /* Configure P0_11 to ALT4, CT0_MAT1 */
   PORT0->PCR[11] = PORT_PCR_MUX(4);
   /* Timer setup */
   CTIMER0->TCR = CTIMER_TCR_CEN(0); /* Disable timer */
   /* Set CTIMER0 to reset counter on match 0 */
   CTIMER0->MCR = CTIMER_MCR_MR0R(1);
   /* Set PR to 0 (dividing clock by 1) */
   CTIMER0->PR = CTIMER_PR_PRVAL(0);
   /* Set MR0 to 149999, giving CTIMER0 a 1ms period */
   CTIMER0->MR[0] = CTIMER_MR_MATCH(150000 - 1);
   /* Set MR1 to 500, meaning CT0_MAT1 is on half the period */
   CTIMER0->MR[1] = CTIMER_MR_MATCH(75000);
   /* Enable PWM output on CTIMER0_MAT1 */
   CTIMER0->PWMC |= CTIMER_PWMC_PWMEN1(1);
   /* Enable CTIMER0 */
   CTIMER0->TCR |= CTIMER_TCR_CEN(1);

}

void ctUpdateFrequency(INT32U freq){
    if (freq_hz == 0) return; // Prevent zero division

    periodValue = 150000000 / freq;

    // Update Value
    CTIMER0->MR[0] = CTIMER_MR_MATCH(periodValue - 1); //
    CTIMER0->MR[1] = CTIMER_MR_MATCH(periodValue * dutyValue); // Duty Cycle change

    // Reset counter to apply
    CTIMER0->TCR |= CTIMER_TCR_CEN(0); // Disable timer
    CTIMER0->TC = 0; // Reset counter
    CTIMER0->TCR |= CTIMER_TCR_CEN(1); // Enable timer
}

void ctUpdateDutyCycle(INT32U dutyCycle){

    INT32U dutyValue = dutyCycle; // Save the duty cycle

    CTIMER0->MR[1] = CTIMER_MR_MATCH(periodValue * dutyValue); // Duty cycle change

    // Reset counter to apply
    CTIMER0->TCR |= CTIMER_TCR_CEN(0); // Disable timer
    CTIMER0->TC = 0; // Reset counter
    CTIMER0->TCR |= CTIMER_TCR_CEN(1); // Enable timer
}


