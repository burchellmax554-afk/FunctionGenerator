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
*                                             ARMv8-M Port
*
* File      : os_cpu.h
* Version   : V3.09.02
*********************************************************************************************************
* For       : ARMv8-M Cortex-M
* Mode      : Thumb-2 ISA
* Toolchain : GNU C Compiler
*********************************************************************************************************
*/

#ifndef  OS_CPU_H
#define  OS_CPU_H

#ifdef   OS_CPU_GLOBALS
#define  OS_CPU_EXT
#else
#define  OS_CPU_EXT  extern
#endif

#include  <cpu.h>


/*
*********************************************************************************************************
*                                     EXTERNAL C LANGUAGE LINKAGE
*
* Note(s) : (1) C++ compilers MUST 'extern'ally declare ALL C function prototypes & variable/object
*               declarations for correct C language linkage.
*********************************************************************************************************
*/

#ifdef __cplusplus
extern  "C" {                                    /* See Note #1.                                       */
#endif


/*
*********************************************************************************************************
*                                               DEFINES
* Note(s) : (1) Determines the interrupt programmable priority levels. This is normally specified in the
*               Microcontroller reference manual. 4-bits gives us 16 programmable priority levels.
*********************************************************************************************************
*/

#if (defined(__VFP_FP__) && !defined(__SOFTFP__))
#define  OS_CPU_ARM_FP_EN              1u
#else
#define  OS_CPU_ARM_FP_EN              0u
#endif

#ifdef __ARM_FEATURE_CMSE
#if (__ARM_FEATURE_CMSE == 3)
#define  OS_CPU_ARM_CMSE_SECURE        1u                       /* TrustZone Extension Available : Secure Mode          */
#else
#define  OS_CPU_ARM_CMSE_SECURE        0u                       /* TrustZone Extension Available : Non-Secure Mode      */
#endif
#else
#define  OS_CPU_ARM_CMSE_SECURE        0u                       /* TrustZone Extension not available.                   */
#endif

#ifndef CPU_CFG_KA_IPL_BOUNDARY
#error  "CPU_CFG_KA_IPL_BOUNDARY         not #define'd in 'cpu_cfg.h'    "
#endif

#ifndef CPU_CFG_NVIC_PRIO_BITS
#error  "CPU_CFG_NVIC_PRIO_BITS         not #define'd in 'cpu_cfg.h'    "   /* See Note # 1            */
#else
#if (CPU_CFG_KA_IPL_BOUNDARY >= (1u << CPU_CFG_NVIC_PRIO_BITS))
#error  "CPU_CFG_KA_IPL_BOUNDARY        should not be set to higher than max programable priority level "
#endif
#endif


/*
*********************************************************************************************************
*                                               MACROS
*********************************************************************************************************
*/

#define  OS_TASK_SW()               OSCtxSw()

#define  OS_TASK_SW_SYNC()          __asm__ __volatile__ ("isb" : : : "memory")


/*
*********************************************************************************************************
*                                       TIMESTAMP CONFIGURATION
*
* Note(s) : (1) OS_TS_GET() is generally defined as CPU_TS_Get32() to allow CPU timestamp timer to be of
*               any data type size.
*
*           (2) For architectures that provide 32-bit or higher precision free running counters
*               (i.e. cycle count registers):
*
*               (a) OS_TS_GET() may be defined as CPU_TS_TmrRd() to improve performance when retrieving
*                   the timestamp.
*
*               (b) CPU_TS_TmrRd() MUST be configured to be greater or equal to 32-bits to avoid
*                   truncation of TS.
*********************************************************************************************************
*/

#if      OS_CFG_TS_EN == 1u
#define  OS_TS_GET()               (CPU_TS)CPU_TS_TmrRd()   /* See Note #2a.                           */
#else
#define  OS_TS_GET()               (CPU_TS)0u
#endif

#if (CPU_CFG_TS_32_EN    > 0u) && \
    (CPU_CFG_TS_TMR_SIZE < CPU_WORD_SIZE_32)
                                                            /* See Note #2b.                           */
#error  "cpu_cfg.h, CPU_CFG_TS_TMR_SIZE MUST be >= CPU_WORD_SIZE_32"
#endif


/*
*********************************************************************************************************
*                              OS TICK INTERRUPT PRIORITY CONFIGURATION
*
* Note(s) : (1) For systems that don't need any high, real-time priority interrupts; the tick interrupt
*               should be configured as the highest priority interrupt but won't adversely affect system
*               operations.
*
*           (2) For systems that need one or more high, real-time interrupts; these should be configured
*               higher than the tick interrupt which MAY delay execution of the tick interrupt.
*
*               (a) If the higher priority interrupts do NOT continually consume CPU cycles but only
*                   occasionally delay tick interrupts, then the real-time interrupts can successfully
*                   handle their intermittent/periodic events with the system not losing tick interrupts
*                   but only increasing the jitter.
*
*               (b) If the higher priority interrupts consume enough CPU cycles to continually delay the
*                   tick interrupt, then the CPU/system is most likely over-burdened & can't be expected
*                   to handle all its interrupts/tasks. The system time reference gets compromised as a
*                   result of losing tick interrupts.
*********************************************************************************************************
*/

#ifndef  OS_CPU_CFG_SYSTICK_PRIO
#define  OS_CPU_CFG_SYSTICK_PRIO           0u
#endif


/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

OS_CPU_EXT  CPU_STK  *OS_CPU_ExceptStkBase;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

                                                  /* See OS_CPU_A.ASM                                  */
void  OSCtxSw               (void);
void  OSIntCtxSw            (void);
void  OSStartHighRdy        (void);

                                                  /* See OS_CPU_C.C                                    */
void  OS_CPU_SysTickInit    (CPU_INT32U   cnts);
void  OS_CPU_SysTickInitFreq(CPU_INT32U   cpu_freq);

void  OS_CPU_SysTickHandler (void);
void  OS_CPU_PendSVHandler  (void);
                                                  /* CMSIS compliant names for Cortex-M                */
void  PendSV_Handler        (void);
void  SysTick_Handler       (void);


/*
*********************************************************************************************************
*                                   EXTERNAL C LANGUAGE LINKAGE END
*********************************************************************************************************
*/

#ifdef __cplusplus
}                                                 /* End of 'extern'al C lang linkage.                 */
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif
