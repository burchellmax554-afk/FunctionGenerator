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

/****************************************************************************************
* Allocate task control blocks
****************************************************************************************/
static OS_TCB appTaskStartTCB;
static OS_TCB appTaskFunctionDisplayTCB;
static OS_TCB appTaskStateManagementTCB;
/****************************************************************************************
* Allocate task stack space.
****************************************************************************************/
static CPU_STK appTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK appTaskFunctionDisplayStk[APP_CFG_TASK_FUNCTION_DISPLAY_STK_SIZE];
static CPU_STK appTaskStateManagementStk[APP_CFG_TASK_STATE_MANAGEMENT_STK_SIZE];
/****************************************************************************************
* Task Function Prototypes.
****************************************************************************************/
static void appStartTask(void *p_arg);
static void appTaskFunctionDisplay(void *p_arg);
static void appTaskStateManagement(void *p_arg);
void ResetSystemState(void);
INT8U GetNumberOfDigits(INT32U num);


/****************************************************************************************
* Reset Function to set back to default
****************************************************************************************/
void ResetSystemState(void) {
    current_state = (SystemState) {sine, 1000, 10, 1000, 50};
}

/****************************************************************************************
* Calculate the number of digits of a number, to stop leading 0's
****************************************************************************************/
INT8U GetNumberOfDigits(INT32U num) {
    INT8U num_digits = 0;
    if (num == 0) {
        return 1;  /* Special case: 0 has 1 digit */
    }

    while (num > 0) {
        num /= 10;  /* Remove the last digit */
        num_digits++;
    }
    return num_digits;
}

/****************************************************************************************
* main()
****************************************************************************************/
void main(void) {

    OS_ERR  os_err;

    FRDM_MCXN947InitBootClock();
    BIOOpen(BIO_BIT_RATE_115200);	/* Startup BasicIO for asserts */

    CPU_IntDis();               /* Disable all interrupts, OS will enable them  */

    OSInit(&os_err);                    /* Initialize uC/OS-III                         */
    assert(os_err == OS_ERR_NONE);

    OSTaskCreate(&appTaskStartTCB,                  /* Address of TCB assigned to task */
                 "Start Task",                      /* Name you want to give the task */
                 appStartTask,                      /* Address of the task itself */
                 (void *) 0,                        /* p_arg is not used so null ptr */
                 APP_CFG_TASK_START_PRIO,           /* Priority you assign to the task */
                 &appTaskStartStk[0],               /* Base address of task’s stack */
                 (APP_CFG_TASK_START_STK_SIZE/10u), /* Watermark limit for stack growth */
                 APP_CFG_TASK_START_STK_SIZE,       /* Stack size */
                 0,                                 /* Size of task message queue */
                 0,                                 /* Time quanta for round robin */
                 (void *) 0,                        /* Extension pointer is not used */
                 (OS_OPT_TASK_NONE),                /* Options */
                 &os_err);                          /* Ptr to error code destination */

    assert(os_err == OS_ERR_NONE);

    OSStart(&os_err);               /* Start multitasking (i.e. give control to uC/OS)    */
    assert(0);						/* Should never get here */
}


/****************************************************************************************
* STARTUP TASK
****************************************************************************************/
static void appStartTask(void *p_arg) {

    OS_ERR os_err;
    (void)p_arg;                        /* Avoid compiler warning for unused variable   */

    OS_CPU_SysTickInitFreq(SystemCoreClock);
    OSStatTaskCPUUsageInit(&os_err);
    GpioLEDGREENInit();
    GpioLEDREDInit();
    GpioDBugBitsInit();
    SwInit();
    TSIInit();
    TimerInit();

    OSTaskCreate(&appTaskFunctionDisplayTCB,                  /* Create Task 1                    */
                "App Task TimerDisplay ",
                appTaskFunctionDisplay,
                (void *) 0,
                APP_CFG_TASK_FUNCTION_DISPLAY_PRIO,
                &appTaskFunctionDisplayStk[0],
                (APP_CFG_TASK_FUNCTION_DISPLAY_STK_SIZE / 10u),
                APP_CFG_TASK_FUNCTION_DISPLAY_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    assert(os_err == OS_ERR_NONE);

    OSTaskCreate(&appTaskStateManagementTCB,                  /* Create Task 1                    */
                "App Task StateManagement ",
                appTaskStateManagement,
                (void *) 0,
                APP_CFG_TASK_STATE_MANAGEMENT_PRIO,
                &appTaskStateManagementStk[0],
                (APP_CFG_TASK_STATE_MANAGEMENT_STK_SIZE / 10u),
                APP_CFG_TASK_STATE_MANAGEMENT_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    assert(os_err == OS_ERR_NONE);

    OSTaskCreate(&appTaskTSITCB,                  /* Create Task 1                    */
                "App Task TSIManagement ",
                appTaskTSI,
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

    OSTaskDel((OS_TCB *)0, &os_err); /* Delete start task */
    assert(os_err == OS_ERR_NONE);
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


/****************************************************************************************
* Update the display only if the state changes
****************************************************************************************/
static void appTaskFunctionDisplay(void *p_arg) {
    (void)p_arg;
    OS_ERR os_err;
    while (1) {
        /* Check if the current state is any different from the previous state */
        if (current_state.wave_form != previous_state.wave_form ||
            current_state.sine_frequency != previous_state.sine_frequency ||
            current_state.sine_amplitude != previous_state.sine_amplitude ||
            current_state.pulse_frequency != previous_state.pulse_frequency ||
            current_state.pulse_duty_cycle != previous_state.pulse_duty_cycle) {



            switch (current_state.wave_form) {
                case sine:
                    BIOPutStrg("\r[sine], ");
                    break;
                case pulse:
                    BIOPutStrg("\rsine, ");
                    break;
                default:
                    BIOPutStrg("\rInvalid Mode! ");
            }

            /* Calculate the number of digits dynamically */
            INT8U sine_frequency_digits = GetNumberOfDigits(current_state.sine_frequency);
            INT8U amplitude_digits = GetNumberOfDigits(current_state.sine_amplitude);

            /* Display sine frequency, amplitude using BIOOutDecWord */
            BIOOutDecWord(current_state.sine_frequency, sine_frequency_digits, BIO_OD_MODE_AR);  /* Dynamic number of digits */
            BIOPutStrg("Hz, ");
            BIOOutDecWord(current_state.sine_amplitude, amplitude_digits, BIO_OD_MODE_AR);  /* Dynamic number of digits */
            BIOPutStrg(", ");

            /* Display pulse mode, pulse frequency, and duty cycle */
            switch (current_state.wave_form) {
                case sine:
                     BIOPutStrg("pulse, ");
                     break;
                case pulse:
                     BIOPutStrg("[pulse], ");
                     break;
                default:
                     BIOPutStrg("Invalid Mode! ");
            }

            /* Calculate the number of digits for pulse frequency and duty cycle dynamically */
            INT8U pulse_frequency_digits = GetNumberOfDigits(current_state.pulse_frequency);
            INT8U duty_cycle_digits = GetNumberOfDigits(current_state.pulse_duty_cycle);

            /* Display pulse frequency and duty cycle using BIOOutDecWord */
            BIOOutDecWord(current_state.pulse_frequency, pulse_frequency_digits, BIO_OD_MODE_AR);  /* Dynamic number of digits */
            BIOPutStrg("Hz, ");
            BIOOutDecWord(current_state.pulse_duty_cycle, duty_cycle_digits, BIO_OD_MODE_AR);  /* Dynamic number of digits */
            BIOPutStrg("%");

            /* Save current state to previous_state for next iteration */
            previous_state = current_state;
        }
        /* Delay 250ms before next update */
    OSTimeDly(250, OS_OPT_TIME_PERIODIC, &os_err);
    assert(os_err == OS_ERR_NONE);

    }
}


static void appTaskStateManagement(void *p_arg) {
    OS_ERR os_err;
    SW_T sw_in = 0; /* Initialize switch to 0 */
    (void)p_arg;  /* Avoid unused parameter warning */
    while(1) {
        sw_in = SwPend(0, &os_err);  /* Wait for switch input */
        assert(os_err == OS_ERR_NONE);

        /* Handle switch presses: */
        if (sw_in == SW2) {  /* SW2 is pressed (active-low) */
            ResetSystemState();  // Reset the system state
        }
    }
}

static void appTaskTSI(void *p_arg) {
    INT16U cur_sense_flag;
    (void)p_arg;
    ResetSystemState();

    while (1) {
    	// OSTDely
        DB0_TURN_ON(); /* debug bit measures sensor scan time */
        /*start a scan sequence */
        TSI0->GENCS |= TSI_GENCS_SWTS(1);
        /* wait for scan to finish */
        while((TSI0->DATA & TSI_DATA_EOSF_MASK) == 0){}
        DB0_TURN_OFF();

        TSI0->DATA |= TSI_DATA_EOSF(1);    //Clear flag
        /* Send TSICNT to terminal to help tune settings. For debugging only */
       // BIOOutHexWord(TSI0->DATA & TSI_DATA_TSICNT_MASK);
        //BIOWrite('\r');
        /* Process channel */
        if((INT16U)(TSI0->DATA & TSI_DATA_TSICNT_MASK) > tsiLevels.threshold){
            tsiFlag = 1;
        }else{
        }
    }
        cur_sense_flag = TSITouchGet();  // Check the TSI for touch input
        if (cur_sense_flag == 1) {
            TSISwap();  // Swap waveforms if touch is detected

      }
}



