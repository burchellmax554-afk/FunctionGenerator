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

#define APP_CFG_TASK_START_PRIO            6u  // Priority for the starting task, typically lower for initialization.
#define APP_CFG_SW_TASK_PRIO               7u  // Software management task priority, moderate importance.
#define APP_CFG_TASK_FUNCTION_DISPLAY_PRIO 8u // Display update task, lowest priority (non-critical).
#define APP_CFG_TASK_STATE_MANAGEMENT_PRIO 7u  // State management task priority, moderate importance (matches SW task).
#define APP_CFG_TASK_TSI_PRIO              8u  // TSI (touch sensing) task priority, higher priority for responsiveness.
#define APP_CFG_TASK_ROTARY_PRIO           5u  // Rotary encoder task priority, lower priority.
#define APP_CFG_TASK_TOUCH_DETECTION_PRIO  8u  // Touch detection task priority, same as TSI (user interaction).
#define APP_CFG_TASK_ENTER_CHECK_PRIO      9u  // Enter key check task priority, relatively high priority for user input.
#define APP_CFG_GENERATE_SINE_TABLE_TASK_PRIO 10u // Sine wave generation task priority, relatively low (background).
/*
#define APP_CFG_TASK_START_PRIO            6u

#define APP_CFG_SW_TASK_PRIO		       7u
#define APP_CFG_TASK_FUNCTION_DISPLAY_PRIO 10u //Define variable names to better match lab 2 over lab 1
#define APP_CFG_TASK_STATE_MANAGEMENT_PRIO 7u
#define APP_CFG_TASK_TSI_PRIO              8u
#define APP_CFG_TASK_ROTARY_PRIO           5u
#define APP_CFG_TASK_TOUCH_DETECTION_PRIO  8u
#define APP_CFG_TASK_ENTER_CHECK_PRIO      9u
//Shot
#define APP_CFG_GENERATE_SINE_TABLE_TASK_PRIO 10u
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
#define APP_CFG_GENERATE_SINE_TABLE_TASK_STK_SIZE 128u

#endif
