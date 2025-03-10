/*****************************************************************************************
 * TSI low level module based on FRDM-N947 board.
 * Only written for the single touchpad on the FRDM-N947 board.
 * Very basic with no extras
 * Todd Morton, 02/19/2025
 *****************************************************************************************/
/*
 *
 */
#include <Timer.h>
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "TSI.h"
#include "sineTable.h"
#include "state.h"

typedef struct{
    INT16U baseline;
    INT16U offset;
    INT16U threshold;
}TOUCH_LEVEL_T;


#define TOUCH_OFFSET  0xF000U    // Touch offset from baseline

static TOUCH_LEVEL_T tsiLevels;
static INT8U tsiFlag;
static void tsiChCalibration(void);
static void appTaskTSI(void *p_arg);
static void TSISwap(void);
static OS_TCB appTaskTSITCB;
static CPU_STK appTaskTSIStk[APP_CFG_TASK_TSI_STK_SIZE];
/*****************************************************************************************
 * TSI0Init: Initializes TSI0 module
 * Notes:
 *    -
 *****************************************************************************************/
void TSIInit(void){

    /* set constant offset count */
    tsiLevels.offset = TOUCH_OFFSET;

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

    OSTaskCreate(&appTaskTSITCB,                  /* Create Task 1                    */
                    "App Task TSI",
                    appTaskFunctionDisplay,
                    (void *) 0,
                    APP_CFG_TASK_TSI_PRIO,
                    &appTaskTSIStk[0],
                    (APP_CFG_TASK_TSI_STK_SIZE / 10u),
                    APP_CFG_TASK_TSI_STK_SIZE,
                    0,
                    0,
                    (void *) 0,
                    (OS_OPT_TASK_NONE),
                    &os_err);
    assert(os_err == OS_ERR_NONE);

    //create tsi task
    // tsi task is the only os task the rest can stay as regular function
    // tsi touch get is probably not needed
}

static void TaskTSI(void *parg){

    DB0_TURN_ON(); /* debug bit measures sensor scan time */
    /*start a scan sequence */
    TSI0->GENCS |= TSI_GENCS_SWTS(1);
    /* wait for scan to finish */
    while((TSI0->DATA & TSI_DATA_EOSF_MASK) == 0){}
    DB0_TURN_OFF();

    TSI0->DATA |= TSI_DATA_EOSF(1);    //Clear flag
    /* Send TSICNT to terminal to help tune settings. For debugging only */

//Keeping this here for future use, DELETE WHEN PUSHING RELEASEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
    // BIOOutHexWord(TSI0->DATA & TSI_DATA_TSICNT_MASK);
    //BIOWrite('\r');

    /* Process channel */
    if((INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK) > tsiLevels.threshold){
        tsiFlag = 1;
    }else{
    }
}

/********************************************************************************
 *   tsiCalibration: Calibration to find non-touch baseline
 *                   Note - the sensor must not be pressed when this is executed.
 ********************************************************************************/
static void tsiChCalibration(void) {
    TSI0->GENCS |= TSI_GENCS_SWTS(1);             // start a scan sequence
    while((TSI0->DATA & TSI_DATA_EOSF_MASK) == 0){} // wait for scan to finish
    TSI0->DATA |= TSI_DATA_EOSF(1);    // Clear flag
    tsiLevels.baseline = (INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK);
    tsiLevels.threshold = tsiLevels.baseline + tsiLevels.offset;
}

/********************************************************************************
 *   TSIGetSensorFlags: Returns value of sensor flag variable and clears it
 *                      to receive sensor press only one time.
 ********************************************************************************/
INT8U TSITouchGet(void){
    INT8U tflag;
    tflag = tsiFlag;
    tsiFlag = 0;
    return tflag;
}

/****************************************************************************************
* Wave swap function
****************************************************************************************/
static void TSISwap(void){
    switch (current_state.wave_form){
    case sine:
    	current_state.wave_form = pulse;
        break;
    case pulse:
        current_state.wave_form = sine;
        break;
    default:
        current_state. wave_form = sine;
    }
}
