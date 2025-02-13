/*
*********************************************************************************************************
*                                               Cs/OS3
*                                        The Real-Time Kernel
*
*                           Copyright 2023; Weston Embedded Solutions, LLC.
*                                       www.weston-embedded.com
*
*                   All rights reserved. Protected by international copyright laws.
*
*               Your use of this software is subject to your acceptance of the terms of
*               a Weston Embedded Solutions software license, which can be obtained by
*               contacting us at www.weston-embedded.com/company/contact. If you do not
*                 agree to the terms of this license, you may not use this software.
*
*                 Please help us continue to provide the embedded community with the
*                   finest software available. Your honesty is greatly appreciated.
*
*********************************************************************************************************
*                                              uC/OS-III
*                    Copyright 2009-2020 Silicon Laboratories Inc. www.silabs.com
*                                 SPDX-License-Identifier: APACHE-2.0
*               This software is subject to an open-source license and is distributed by
*                Silicon Laboratories Inc. pursuant to the terms of the Apache License,
*                    Version 2.0 available at www.apache.org/licenses/LICENSE-2.0.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                               OS CONFIGURATION (APPLICATION SPECIFICS)
*
* Filename : os_cfg_app.h
* Version  : V3.09.02
*********************************************************************************************************
*/

#ifndef OS_CFG_APP_H
#define OS_CFG_APP_H

/*
**************************************************************************************************************************
*                                                      CONSTANTS
**************************************************************************************************************************
*/
                                                                /* ------------------ MISCELLANEOUS ------------------- */
                                                                /* Stack size of ISR stack (number of CPU_STK elements) */
#define  OS_CFG_ISR_STK_SIZE                             128u
                                                                /* Maximum number of messages                           */
#define  OS_CFG_MSG_POOL_SIZE                             32u
                                                                /* Stack limit position in percentage to empty          */
#define  OS_CFG_TASK_STK_LIMIT_PCT_EMPTY                  10u


                                                                /* -------------------- IDLE TASK --------------------- */
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_IDLE_TASK_STK_SIZE                        64u


                                                                /* ------------------ STATISTIC TASK ------------------ */
                                                                /* Priority                                             */
#define  OS_CFG_STAT_TASK_PRIO  ((OS_PRIO)(OS_CFG_PRIO_MAX-2u))
                                                                /* Rate of execution (1 to 10 Hz)                       */
#define  OS_CFG_STAT_TASK_RATE_HZ                         10u
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_STAT_TASK_STK_SIZE                       100u


                                                                /* ---------------------- TICKS ----------------------- */
                                                                /* Tick rate in Hertz (10 to 1000 Hz)                   */
#define  OS_CFG_TICK_RATE_HZ                            1000u


                                                                /* --------------------- TIMERS ----------------------- */
                                                                /* Priority of 'Timer Task'                             */
#define  OS_CFG_TMR_TASK_PRIO   ((OS_PRIO)(OS_CFG_PRIO_MAX-3u))
                                                                /* Stack size (number of CPU_STK elements)              */
#define  OS_CFG_TMR_TASK_STK_SIZE                        128u

                                                                /* DEPRECATED - Rate for timers (10 Hz Typ.)            */
                                                                /* The timer task now calculates its timeouts based     */
                                                                /* on the timers in the list. It no longer runs at a    */
                                                                /* static frequency.                                    */
                                                                /* This define is included for compatibility reasons.   */
                                                                /* It will determine the period of a timer tick.        */
                                                                /* We recommend setting it to OS_CFG_TICK_RATE_HZ       */
                                                                /* for new projects.                                    */
#define  OS_CFG_TMR_TASK_RATE_HZ                          10u


#endif
