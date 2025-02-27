#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
#include "app_cfg.h"
#include "TSI.h"

/* Struct definition for modes */
typedef enum {
    sine,
    pulse
} WAVE_FORM;

/****************************************************************************************
* Allocate task control blocks
****************************************************************************************/
static OS_TCB appTaskStartTCB;
static OS_TCB appTaskFunctionDisplayTCB;
/****************************************************************************************
* Allocate task stack space.
****************************************************************************************/
static CPU_STK appTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK appTaskFunctionDisplayStk[APP_CFG_TASK_FUNCTION_DISPLAY_STK_SIZE];
/****************************************************************************************
* Task Function Prototypes. 
****************************************************************************************/
static void  appStartTask(void *p_arg);
static void  appTaskFunctionDisplay(void *p_arg);
INT8U GetNumberOfDigits(INT32U num);
/****************************************************************************************
* Default state setup
****************************************************************************************/
static WAVE_FORM wave_form = sine;
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

    OSTaskDel((OS_TCB *)0, &os_err); /* Delete start task */
    assert(os_err == OS_ERR_NONE);
}

/****************************************************************************************
* Display wave information
****************************************************************************************/
static void appTaskFunctionDisplay(void *p_arg) {
    (void)p_arg;
    INT32U sine_frequency = 1000;
    INT32U amplitude = 10;
    INT32U pulse_frequency = 1000;
    INT32U duty_cycle = 50;
    INT16U cur_sense_flag;


    /* Display function */
    switch (wave_form) {
        case sine:
            BIOPutStrg("[sine], ");
            break;
        case pulse:
            BIOPutStrg("[pulse], ");
            break;
        default:
            BIOPutStrg("Invalid Mode! ");
    }

    /* Calculate the number of digits dynamically */
    INT8U sine_frequency_digits = GetNumberOfDigits(sine_frequency);
    INT8U amplitude_digits = GetNumberOfDigits(amplitude);

    /* Display sine frequency, amplitude using BIOOutDecWord */
    BIOOutDecWord(sine_frequency, sine_frequency_digits, BIO_OD_MODE_AR);  /* Dynamic number of digits */
    BIOPutStrg("Hz, ");
    BIOOutDecWord(amplitude, amplitude_digits, BIO_OD_MODE_AR);  /* Dynamic number of digits */
    BIOPutStrg(", ");

    /* Display pulse mode, pulse frequency, and duty cycle */
    switch (wave_form) {
        case sine:
            BIOPutStrg("pulse, ");
            break;
        case pulse:
            BIOPutStrg("sine, ");
            break;
        default:
            BIOPutStrg("Invalid Mode! ");
    }

    /* Calculate the number of digits for pulse frequency and duty cycle dynamically */
    INT8U pulse_frequency_digits = GetNumberOfDigits(pulse_frequency);
    INT8U duty_cycle_digits = GetNumberOfDigits(duty_cycle);

    /* Display pulse frequency and duty cycle using BIOOutDecWord */
    BIOOutDecWord(pulse_frequency, pulse_frequency_digits, BIO_OD_MODE_AR);  /* Dynamic number of digits */
    BIOPutStrg("Hz, ");
    BIOOutDecWord(duty_cycle, duty_cycle_digits, BIO_OD_MODE_AR);  /* Dynamic number of digits */
    BIOPutStrg("%");


    while(1){
        TSITask();
        cur_sense_flag = TSITouchGet();
        if(cur_sense_flag == 1){
            BIOOutCRLF();
        }else{
            BIOPutStrg("Bust");
            BIOOutCRLF();
        }
    }
}
