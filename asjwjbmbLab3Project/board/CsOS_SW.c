/**********************************************************************
* CsOS_SW.h - A push button debouncing task module that runs under CsOS
* for SW2 and SW3 on the FRDM_MCXN947 board.
*
* Requires the following be defined in app_cfg.h:
*                   APP_CFG_SW_TASK_PRIO
*                   APP_CFG_SW_TASK_STK_SIZE
*
* 10/06/2024 TDM
*********************************************************************
* Header Files - Dependencies
********************************************************************/
#include "MCUType.h"
#include "app_cfg.h"
#include "os.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include "BasicIO.h"
/********************************************************************
* Module Defines
* This version is designed for the FRDM_MCXN947 switches, which
* have the following mapping:
*  SW2->P0_23, SW3->P0_6
********************************************************************/
typedef enum{SW_OFF,SW_EDGE,SW_VERF} SWSTATES;

/* Simple synchronous buffer to pass switch presses. */
typedef struct{
    INT8U buffer;
    OS_SEM flag;
}SW_BUFFER;
/********************************************************************
* Private Resources
********************************************************************/
static void swTask(void *p_arg);
static SW_T swScan(void);
static SW_BUFFER swBuffer;
/**********************************************************************************
* Allocate task control blocks
**********************************************************************************/
static OS_TCB swTaskTCB;
/*************************************************************************
* Allocate task stack space.
*************************************************************************/
static CPU_STK swTaskStk[APP_CFG_SW_TASK_STK_SIZE];

/********************************************************************
* SwPend() - A function to provide access to the switch buffer via a
*             semaphore.
********************************************************************/
SW_T SwPend(INT16U tout, OS_ERR *os_err){
    OSSemPend(&(swBuffer.flag),tout, OS_OPT_PEND_BLOCKING, (CPU_TS *)0, os_err);
    return(swBuffer.buffer);
}

/********************************************************************
* SwInit() - Initialization routine for the switch module
********************************************************************/
void SwInit(void){

    OS_ERR os_err;
	/* Switch init */
    GpioSw2Init(GPIO_IRQ_OFF);
    GpioSw3Init(GPIO_IRQ_OFF);
    // Initialize the Switch Buffer and semaphore
    swBuffer.buffer = SWN;
    OSSemCreate(&(swBuffer.flag),"SW Semaphore",0,&os_err);
    assert(os_err == OS_ERR_NONE);
    //Create the switch task
    OSTaskCreate(&swTaskTCB,
                "uCOS SW Task ",
                swTask,
                (void *) 0,
                APP_CFG_SW_TASK_PRIO,
                &swTaskStk[0],
                (APP_CFG_SW_TASK_STK_SIZE / 10u),
                APP_CFG_SW_TASK_STK_SIZE,
                0,
                0,
                (void *) 0,
                (OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                &os_err);
    assert(os_err == OS_ERR_NONE);

}

/********************************************************************
* swTask() - Read and debounce the switches and updates SwBuffer.
*             It is decomposed into states for detecting and
*             verifying switch presses. This task should be called
*             periodically with a period greater than the worst case
*             switch bounce time and less than the shortest switch
*             activation time minus the bounce time. The switch must 
*             be released to have multiple acknowledged presses.
*             Treats the switches as active low.
********************************************************************/
static void swTask(void *p_arg) {

    OS_ERR os_err;
    SW_T cur_sw;
    SW_T last_sw = 0;
    SWSTATES swstate = SW_OFF;
    (void)p_arg;
    while(1){
        DB0_TURN_OFF();
        OSTimeDly(8,OS_OPT_TIME_PERIODIC,&os_err);
	    assert(os_err == OS_ERR_NONE);
        DB0_TURN_ON();
	    /* Get switch input and convert to active high */
	    cur_sw = swScan();
        if(swstate == SW_OFF){    /* Sw released state */
            if(cur_sw != SWN){
                swstate = SW_EDGE;
            }else{ /* wait for sw press */
            }
        }else if(swstate == SW_EDGE){     /* sw press detected state*/
            if(cur_sw == last_sw){        /* sw press verified */
                swstate = SW_VERF;
                swBuffer.buffer = cur_sw;    /*update buffer */
                (void)OSSemPost(&(swBuffer.flag), OS_OPT_POST_1, &os_err);   /* Signal new data in buffer */
        	    assert(os_err == OS_ERR_NONE);
            }else if( cur_sw == SWN){        /* Invalidated, start over */
                swstate = SW_OFF;
            }else{                          /*Invalidated, diff key edge*/
            }
        }else if(swstate == SW_VERF){     /* sw press verified state */
            if((cur_sw == SWN) || (cur_sw != last_sw)){
                swstate = SW_OFF;
            }else{ /* wait for release or key change */
            }
        }else{ /* In case of error */
            swstate = SW_OFF;             /* Should never get here */
        }
        last_sw = cur_sw;                 /* Save key for next time */
    
    }
}

/********************************************************************
* swScan() - Scans the SW2 and SW3 on the FRDM-MCXN947 board and
*            returns a switch code. Assumes switches are active low.
*           - Only allows one switch to be pressed at a time.
* (Private)
********************************************************************/
static SW_T swScan(void) {
    INT32U sw_bits;
    INT8U swcode;
    sw_bits = (SW2_INPUT|SW3_INPUT);
    if((sw_bits & GPIO_PIN(SW2_BIT)) == 0){
        swcode = SW2;
    }else if((sw_bits & GPIO_PIN(SW3_BIT)) == 0){
        swcode = SW3;
    }else{
        swcode = SWN;
    }
    return swcode;
}

