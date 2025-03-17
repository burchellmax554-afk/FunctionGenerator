/*
 * CTIMER.c
 *
 *  Last modified 16MAR2025 by John Brown & Jake Sheckler
 *      Authors: Todd Morton, Sam Johnson
 *
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
#include "CTIMER.h"
#include "state.h"
#include "BasicIO.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************/

//CTIMER ONLY WORKING WITH DEFAULTEFSTATE                                              ++++++++++++++++++++++++++++

/*******************************************************************************
 * Variables
 ******************************************************************************/
INT32U periodValue; // Period value (default is 150000)
INT32U dutyValue; // Duty cycle (default is 50)
//INT32U freq;
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
   SYSCON0->CTIMERCLKDIV[0] = 4; // Divide by 2, for example
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

/**********/
// Function to update frequency
void ctUpdateFrequency(INT32U freq, INT32U dutyCycle){
    if (freq == 0) return; // Prevent zero division

    periodValue = (INT32U)((30000000.0 / freq) + 0.5); // Calculate period value based on new frequency
    dutyValue = 100 - dutyCycle; // Save the duty cycle


    // Update Value
    CTIMER0->MR[0] = CTIMER_MR_MATCH(periodValue - 1); // Change period value
    CTIMER0->MR[1] = CTIMER_MR_MATCH((periodValue * dutyValue) / 100); // Duty Cycle change

    // Reset counter to apply
    CTIMER0->TCR = CTIMER_TCR_CEN(0); // Disable timer
    CTIMER0->TC = 0; // Reset counter
    CTIMER0->TCR = CTIMER_TCR_CEN(1); // Enable timer
}

/**********/
// Function to update duty cycle
void ctUpdateDutyCycle(INT32U freq, INT32U dutyCycle){


    dutyValue = 100 - dutyCycle; // Save the duty cycle
    periodValue = (INT32U)((30000000.0 / freq) + 0.5);
    CTIMER0->MR[0] = CTIMER_MR_MATCH(periodValue - 1); // Change period value
    CTIMER0->MR[1] = CTIMER_MR_MATCH((periodValue * dutyValue) / 100); // Duty cycle change
    // Reset counter to apply
    CTIMER0->TCR = CTIMER_TCR_CEN(0); // Disable timer
    CTIMER0->TC = 0; // Reset counter
    CTIMER0->TCR = CTIMER_TCR_CEN(1); // Enable timer
}
