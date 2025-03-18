/*****************************************************************************************
 * TSI low level module based on FRDM-N947 board.
 * Only written for the single touchpad on the FRDM-N947 board.
 * Very basic with no extras
 * Todd Morton, 02/19/2025
 *****************************************************************************************/
/*
 *
 */
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "TSI.h"
#include "state.h"

/*****************************************************************************************
 * TSI0Init: Initializes TSI0 module
 * Credit: Jake Sheckler
 *****************************************************************************************/
void TSIInit(void){


    SYSCON0->AHBCLKCTRLSET[3] = SYSCON_AHBCLKCTRL3_TSI(1);
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT1(1);

    /* Select clk_in as clock, 24MHz*/
    SYSCON0->TSICLKSEL = SYSCON_TSICLKSEL_SEL(2);

    /* Configure TSI for the touch pad on the FRDM board */
    TSI0->CONFIG = TSI_CONFIG_S_XCH(0)|TSI_CONFIG_TSICH(3)|TSI_CONFIG_S_CTRIM(0)|TSI_CONFIG_S_SEN(1)|
            TSI_CONFIG_S_XDN(1);
    TSI0->SINC = TSI_SINC_ORDER(1)|TSI_SINC_DECIMATION(28); /* ORDER = 2, DEC = 29 */
    TSI0->SSC0 = TSI_SSC0_PRBS_OUTSEL(2)|TSI_SSC0_CHARGE_NUM(4)|TSI_SSC0_PRBS_OUTSEL(2)|
            TSI_SSC0_BASE_NOCHARGE_NUM(2)|TSI_SSC0_SSC_PRESCALE_NUM(1);

    TSI0->GENCS |= TSI_GENCS_SETCLK(0)|TSI_GENCS_DVOLT(3);
    /* Enable TSI and calibrate */
    TSI0->GENCS |= TSI_GENCS_TSIEN(1);
    tsiChCalibration();
}

/********************************************************************************
 *   tsiCalibration: Calibration to find non-touch baseline
 *                   Note - the sensor must not be pressed when this is executed.
 *                   Credit: Jake Sheckler
 ********************************************************************************/
void tsiChCalibration(void){
    TSI0->GENCS |= TSI_GENCS_SWTS(1);             //start a scan sequence
    while((TSI0->DATA & TSI_DATA_EOSF_MASK) == 0){} //wait for scan to finish
    TSI0->DATA |= TSI_DATA_EOSF(1);    //Clear flag
    tsiLevels.baseline = (INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK);
    tsiLevels.threshold = tsiLevels.baseline +
                                         tsiLevels.offset;
}
/********************************************************************************
 * TSITouchGet: For sensing if the TSI is being touched or not. Basic Function.
 *  Credit: Jake Sheckler
 ********************************************************************************/
INT8U TSITouchGet(void){
    INT8U tflag;
    tflag = tsiLevels.tsiFlag; //Setting tflag to the tsiflag in the struct
    tsiLevels.tsiFlag = 0; //setting the tsiFlag as a 0 (reset)
    return tflag; //sending out the tflag
}

/****************************************************************************************
* Wave swap function: Used for switching states
* Credit: Jake Sheckler
****************************************************************************************/
void TSISwap(void){
    switch (current_state.wave_form){ //relies on the current state wave form.
    case sine: //if current state is sine,
        current_state.wave_form = pulse; //Make it pulse
        current_state.pulse_duty_cycle = (INT16U)PULSE.qeCnt; //set the duty cycle to the pulse one.
        break;
    case pulse: //if current state is pulse,
        current_state.wave_form = sine; //Now it's sine
        current_state.sine_amplitude = SINE.qeCnt; //And sets sineamplitude to the encoder.
        break;
    default: //Default, make it sine.
        current_state. wave_form = sine;
        current_state.sine_amplitude = SINE.qeCnt;
    }
}
