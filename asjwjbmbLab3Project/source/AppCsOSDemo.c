/*****************************************************************************************
* A simple demo program for uCOS-III. In this case it uses Cesium3 (CsOS) RTOS - The
* most recent version of uCOS.
* It tests multitasking, the timer, and task semaphores.
* This version is written for the MCXN947FRDM board.
* If CsOS is working the green LED should toggle every 100ms and the red LED
* should toggle every 1 second. SW2 can be used to mask off one of the LEDs.
* Version 2024.2
* 10/06/2024, Todd Morton
*****************************************************************************************/
#include "MCUType.h"
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"

#include "app_cfg.h"
/*****************************************************************************************
* Allocate task control blocks
*****************************************************************************************/
static OS_TCB appTaskStartTCB;
static OS_TCB appTask1TCB;
static OS_TCB appTask2TCB;
static OS_TCB appTask3TCB;

/*****************************************************************************************
* Allocate task stack space.
*****************************************************************************************/
static CPU_STK appTaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static CPU_STK appTask1Stk[APP_CFG_TASK1_STK_SIZE];
static CPU_STK appTask2Stk[APP_CFG_TASK2_STK_SIZE];
static CPU_STK appTask3Stk[APP_CFG_TASK3_STK_SIZE];

/*****************************************************************************************
* Task Function Prototypes. 
*   - Private if in the same module as startup task. Otherwise public.
*****************************************************************************************/
static void  appStartTask(void *p_arg);
static void  appTask1(void *p_arg);
static void  appTask2(void *p_arg);
static void  appTask3(void *p_arg);

/*****************************************************************************************
* Variables used for timestamp analysis. Made global for the Global Variable view in
* MCUX. Can be removed if you are not using these for debugging.
*****************************************************************************************/
static CPU_TS cycCnt;
static CPU_TS cycCntDiff;

/*****************************************************************************************
* Variables and typedef used for LED enable state.
*****************************************************************************************/
typedef enum{RED_ON,GREEN_ON,BOTH_ON} LED_EN_STATES;
static LED_EN_STATES LedEnState = BOTH_ON;

/*****************************************************************************************
* main()
*****************************************************************************************/
void main(void) {

    OS_ERR  os_err;

    FRDM_MCXN947InitBootClock();
    BIOOpen(BIO_BIT_RATE_115200);	//Startup BasicIO for asserts

    CPU_IntDis();               /* Disable all interrupts, OS will enable them  */

    OSInit(&os_err);                    /* Initialize uC/OS-III                         */
    assert(os_err == OS_ERR_NONE);

    OSTaskCreate(&appTaskStartTCB,                  /* Address of TCB assigned to task */
                 "Start Task",                      /* Name you want to give the task */
                 appStartTask,                      /* Address of the task itself */
                 (void *) 0,                        /* p_arg is not used so null ptr */
                 APP_CFG_TASK_START_PRIO,           /* Priority you assign to the task */
                 &appTaskStartStk[0],               /* Base address of task�s stack */
                 (APP_CFG_TASK_START_STK_SIZE/10u), /* Watermark limit for stack growth */
                 APP_CFG_TASK_START_STK_SIZE,       /* Stack size */
                 0,                                 /* Size of task message queue */
                 0,                                 /* Time quanta for round robin */
                 (void *) 0,                        /* Extension pointer is not used */
                 (OS_OPT_TASK_NONE),                /* Options */
                 &os_err);                          /* Ptr to error code destination */

    assert(os_err == OS_ERR_NONE);

    OSStart(&os_err);               /*Start multitasking(i.e. give control to uC/OS)    */
    assert(0);						/*Should never get here */
}

/*****************************************************************************************
* STARTUP TASK
* This should run once and be deleted. Could restart everything by creating.
*****************************************************************************************/
static void appStartTask(void *p_arg) {

    OS_ERR os_err;
    (void)p_arg;                        /* Avoid compiler warning for unused variable   */

    OS_CPU_SysTickInitFreq(SystemCoreClock);
    OSStatTaskCPUUsageInit(&os_err);
    GpioLEDGREENInit();
    GpioLEDREDInit();
    GpioDBugBitsInit();
    SwInit();

    OSTaskCreate(&appTask1TCB,                  /* Create Task 1                    */
                "App Task1 ",
                appTask1,
                (void *) 0,
                APP_CFG_TASK1_PRIO,
                &appTask1Stk[0],
                (APP_CFG_TASK1_STK_SIZE / 10u),
                APP_CFG_TASK1_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    assert(os_err == OS_ERR_NONE);

    OSTaskCreate(&appTask2TCB,    /* Create Task 2                    */
                "App Task2 ",
                appTask2,
                (void *) 0,
                APP_CFG_TASK2_PRIO,
                &appTask2Stk[0],
                (APP_CFG_TASK2_STK_SIZE / 10u),
                APP_CFG_TASK2_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    assert(os_err == OS_ERR_NONE);

    OSTaskCreate(&appTask3TCB,    /* Create Task 3                    */
                "App Task3 ",
                appTask3,
                (void *) 0,
                APP_CFG_TASK3_PRIO,
                &appTask3Stk[0],
                (APP_CFG_TASK3_STK_SIZE / 10u),
                APP_CFG_TASK3_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_NONE),
                &os_err);
    assert(os_err == OS_ERR_NONE);

    OSTaskDel((OS_TCB *)0, &os_err); /* Delete start task */
    assert(os_err == OS_ERR_NONE);
}

/*****************************************************************************************
* TASK #1
* Uses OSTimeDelay to signal the Task2 semaphore every second.
* It also toggles the green LED every 100ms.
*****************************************************************************************/
static void appTask1(void *p_arg){

    INT8U timcntr = 0;                              /* Counter for one second flag      */
    OS_ERR os_err;
    (void)p_arg;
    
    while(1){
    
        DB1_TURN_OFF();                             /* Turn off debug bit while waiting */
    	OSTimeDly(100,OS_OPT_TIME_PERIODIC,&os_err);     /* Task period = 100ms   */
        assert(os_err == OS_ERR_NONE);
        DB1_TURN_ON();                          /* Turn on debug bit while ready/running*/
        if((LedEnState == GREEN_ON)||(LedEnState == BOTH_ON)){
        	GREEN_TOGGLE();
        }else{
        }
        timcntr++;
        if(timcntr == 10){                     /* Signal Task2 every second             */
            (void)OSTaskSemPost(&appTask2TCB,OS_OPT_POST_NONE,&os_err);
            assert(os_err == OS_ERR_NONE);
            timcntr = 0;
        }else{
        }
    }
}

/*****************************************************************************************
* TASK #2
* Pends on its semaphore and toggles the red LED every second
*****************************************************************************************/
static void appTask2(void *p_arg){

    OS_ERR os_err;

    (void)p_arg;

    while(1) {                                  /* wait for Task 1 to signal semaphore  */

        DB2_TURN_OFF();                         /* Turn off debug bit while waiting     */
        OSTaskSemPend(0,                        /* No timeout                           */
                      OS_OPT_PEND_BLOCKING,     /* Block until posted                   */
                      &cycCnt,                  /* timestamp destination. Make NULL pointer if not using timestamp */
                      &os_err);
        assert(os_err == OS_ERR_NONE);
        //calculate the task switch time using timestamp. Remove if not using timestamps.
        cycCntDiff = CPU_TS32_to_uSec(OS_TS_GET() - cycCnt); //confirmed by measurement
        DB2_TURN_ON();                          /* Turn on debug bit while ready/running*/
        if((LedEnState == RED_ON)||(LedEnState == BOTH_ON)){
        	RED_TOGGLE();
        }else{
        }
    }
}
/*****************************************************************************************
* TASK #3
* Pends on a SW2 press to change LED masks.
* Green on -> Red on -> Both on -> repeat.
*****************************************************************************************/
static void appTask3(void *p_arg){

    OS_ERR os_err;
    SW_T sw_in = 0;
    (void)p_arg;

    while(1) {
        DB3_TURN_OFF();
        sw_in = SwPend(0, &os_err);
        assert(os_err == OS_ERR_NONE);
        DB3_TURN_ON();
        /* First turn off the LEDs to initialize */
        RED_TURN_OFF();
        GREEN_TURN_OFF();
        BLUE_TURN_OFF();
        if(sw_in == SW2){
        	switch(LedEnState){
        	case(RED_ON):
				LedEnState = GREEN_ON;
        		break;
        	case(GREEN_ON):
				LedEnState = BOTH_ON;
        		break;
        	case(BOTH_ON):
				LedEnState = RED_ON;
        		break;
        	default:
        		LedEnState = RED_ON;
				break;
        	}
        }
    }
}
/********************************************************************************/

