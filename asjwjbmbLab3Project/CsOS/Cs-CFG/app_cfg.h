/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                              (c) Copyright 2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      APPLICATION CONFIGURATION
*
*                                        Freescale Kinetis K60
*                                               on the
*
*                                        Freescale TWR-K60N512
*                                          Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : DC
*********************************************************************************************************
*/

#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  APP_CFG_SERIAL_EN                          DEF_DISABLED //Change to disabled. TDM


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define APP_CFG_TASK_START_PRIO            6u

#define APP_CFG_SW_TASK_PRIO		       12u
#define APP_CFG_TASK_FUNCTION_DISPLAY_PRIO 10u /*Define variable names to better match lab 2 over lab 1 */
#define APP_CFG_TASK_STATE_MANAGEMENT_PRIO 7u
#define APP_CFG_TASK_TSI_PRIO              8u
#define APP_CFG_TASK_ROTARY_PRIO           5u
#define APP_CFG_TASK_TOUCH_DETECTION_PRIO  8u
#define APP_CFG_TASK_ENTER_CHECK_PRIO      9u

//again before I messed with it
/*
 * #define APP_CFG_TASK_START_PRIO            6u
#define APP_CFG_SW_TASK_PRIO               12u
#define APP_CFG_TASK_FUNCTION_DISPLAY_PRIO 14u //Define variable names to better match lab 2 over lab 1
#define APP_CFG_TASK_STATE_MANAGEMENT_PRIO 10u
#define APP_CFG_TASK_TSI_PRIO              8u
#define APP_CFG_TASK_ROTARY_PRIO           8u
#define APP_CFG_TASK_TOUCH_DETECTION_PRIO  10u
#define APP_CFG_TASK_ENTER_CHECK_PRIO      10u
*/
//Before Jake messed with it
/*
#define APP_CFG_TASK_START_PRIO            6u
#define APP_CFG_SW_TASK_PRIO               14u
#define APP_CFG_TASK_FUNCTION_DISPLAY_PRIO 8u
#define APP_CFG_TASK_STATE_MANAGEMENT_PRIO 10u
#define APP_CFG_TASK_TSI_PRIO              12u
#define APP_CFG_TASK_ROTARY_PRIO           10u
#define APP_CFG_TASK_TOUCH_DETECTION_PRIO  10u
*/

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*********************************************************************************************************
*/

#define APP_CFG_TASK_START_STK_SIZE            128u
#define APP_CFG_SW_TASK_STK_SIZE	           128u
#define APP_CFG_TASK_FUNCTION_DISPLAY_STK_SIZE 128u /*Define variable names to better match lab 3 over lab 1 */
#define APP_CFG_TASK_STATE_MANAGEMENT_STK_SIZE 128u
#define APP_CFG_TASK_ROTARY_STK_SIZE           128u
#define APP_CFG_TASK_TOUCH_DETECTION_STK_SIZE  128u
#define APP_CFG_TASK_TSI_STK_SIZE              128u
#define APP_CFG_TASK_ENTER_CHECK_STK_SIZE      128u

#endif
