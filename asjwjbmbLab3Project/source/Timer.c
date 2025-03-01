/*
 * TimeCounter.c
 *
 *  Created on: 20FEB2025, Last modified on 20FEB2025
 *      Author: John Brown
 */


// Includes
/*****************************************************************************************/
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "Timer.h"
/*****************************************************************************************/

// Variable used for checking
INT32U checking = 0;

/**********/
// Timer initialization
void TimerInit(void)
{


    // Initialize clock and timer
    /*Turn on OSC_SYS clock for LPTMRs */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT(1);
    /* Select OSC_SYS clock and bypass prescaler */
    LPTMR0->PSR = LPTMR_PSR_PCS(3)|LPTMR_PSR_PBYP(1);
    /* Set COMPARE register for 10ms period */
    LPTMR0->CMR = LPTMR_CMR_COMPARE(24000000-1);
    /* Clear CSR register and TCF flag, just in case */
    LPTMR0->CSR = LPTMR_CSR_TCF(1);
    /* Enable NVIC for LPTMR1 */
    NVIC_ClearPendingIRQ(LPTMR0_IRQn);
    NVIC_EnableIRQ(LPTMR0_IRQn);

    // Turn on timer interrupts
    LPTMR0->CSR |= LPTMR_CSR_TEN(1);
    LPTMR0->CSR |= LPTMR_CSR_TIE(1);


}
/**********/

/**********/
// Interrupt handler for timer
void LPTMR0_IRQHandler(void){




    /* Clear Timer Compare Flag */
    LPTMR0->CSR |= LPTMR_CSR_TCF(1);

    // Purely to check timer
    checking++;
    BIOPutStrg("\n\r Check");


}
/**********/

