#include <Timer.h>
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "TSI.h"
#include "sineTable.h"
#include "state.h"
#include "rotary.h"
#include "CTimer.h"

/******************************************************************************************
* Private Global Variables
******************************************************************************************/
//INT16S qeXCnt = (CNT_MAX*EDGE_DIV)/2;


/******************************************************************************************
* qeCntOutTask - Timeslice task that uses the position difference register to add or
*                subtract counts. Uses EDGE_DIV to make it more or less sensitive.
*                Output is 0-20. Samples every SLICE_PER.
******************************************************************************************/
void qeCntOutTask(void) {
    DB3_TURN_ON();
    switch(current_state.wave_form){
    case sine:
        SINE.qeXCnt = SINE.qeXCnt + (INT16S)(ENC0->POSD);
        if(SINE.qeXCnt > CNT_MAX * EDGE_DIV) {
                    SINE.qeXCnt = CNT_MAX * EDGE_DIV;
                } else if(SINE.qeXCnt < CNT_MIN) {
                    SINE.qeXCnt = CNT_MIN;
                }

        SINE.qeCnt = SINE.qeXCnt / EDGE_DIV;
        updateSine();
        break;
    case pulse:
        PULSE.qeXCnt = PULSE.qeXCnt + (INT16S)(ENC0->POSD);
        if(PULSE.qeXCnt > 100 * EDGE_DIV) {
                    PULSE.qeXCnt = 100 * EDGE_DIV;
                } else if(PULSE.qeXCnt < CNT_MIN) {
                    PULSE.qeXCnt = CNT_MIN;
                }
        PULSE.qeCnt = PULSE.qeXCnt / EDGE_DIV;
        updatePulseTrain();
        break;
    default:
        SINE.qeCnt = SINE.qeXCnt / EDGE_DIV;

    }
    DB3_TURN_OFF();
}
/******************************************************************************************
* qeQDCInit() - Initialize QDC0 for simple rotational encoders.
*               For input on J3_1 (PHA) and J3_3 (PHB) on FRDM-N947 board.
*               Note the old acronym was ENC and the new acronym is QDC. The RM uses both.
*               Code below uses the old acronym.
******************************************************************************************/
void qeQDCInit(void) {
    /* Enable clock for QDC0, PORT1, and PORT2 */
    SYSCON0->AHBCLKCTRLSET[3] = SYSCON_AHBCLKCTRL3_ENC0(1);
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT1(1);
    SYSCON->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_PORT2(1);
    /* Configure Inputs - internal pull-ups and input buffer enabled
     *                    P1_22 to TRIG_IN3 and P2_0  to TRIG_IN5
     */
    PORT1->PCR[22] = PORT_PCR_MUX(1) | PORT_PCR_IBE(1) | PORT_PCR_PS(1) | PORT_PCR_PE(1);
    PORT2->PCR[0] = PORT_PCR_MUX(1) | PORT_PCR_IBE(1) | PORT_PCR_PS(1) | PORT_PCR_PE(1);
    /* Use INPUTMUX to tie TRIG_IN3 to PhaseA and TRIG_IN5 to PhaseB */
    SYSCON0->AHBCLKCTRLSET[0] = SYSCON_AHBCLKCTRL0_MUX(1);
    INPUTMUX0->ENCN[0].ENC_PHASEB = INPUTMUX_ENCN_ENC_PHASEB_INP(0x2f);
    INPUTMUX0->ENCN[0].ENC_PHASEA = INPUTMUX_ENCN_ENC_PHASEA_INP(0x2d);
    SYSCON0->AHBCLKCTRLCLR[0] = SYSCON_AHBCLKCTRL0_MUX(1);
    /* Configure input filter */
    ENC0->FILT = ENC_FILT_FILT_PRSC(7) | ENC_FILT_FILT_CNT(0) | ENC_FILT_FILT_PER(0xff);
}
/******************************************************************************************
* updateSine() - Update the amplitude of the sine wave based on rotary control.
******************************************************************************************/
void updateSine(void) {
    // Update amplitude based on current count (qeCnt)
    current_state.sine_amplitude = SINE.qeCnt;
}
/******************************************************************************************
* updatePulseTrain() - Update the duty cycle of the pulse-train based on rotary control.
******************************************************************************************/
void updatePulseTrain(void) {
    // Update duty cycle based on current count (qeCnt)                                                                          ++++++++++++++++++++++++++++++++

    current_state.pulse_duty_cycle = (INT16U)PULSE.qeCnt ;  // Scale to match 0-100% duty cycle
    ctUpdateDutyCycle(current_state.pulse_frequency, current_state.pulse_duty_cycle);
    // Remove in  final program
//    BIOPutStrg("\r\nPulse Duty Cycle: ");
//    BIOOutDecWord((INT32U)current_state.pulse_duty_cycle, 3, BIO_OD_MODE_AL);)
}
