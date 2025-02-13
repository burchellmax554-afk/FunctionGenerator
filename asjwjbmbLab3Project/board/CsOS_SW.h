/********************************************************************
* CsOS_SW.h - A push button module that runs under CsOS for SW2 and SW3
* on the FRDM_MCXN947 board.
* The keyCodeTable[] is currently set to generate ASCII codes.
*
* Requires the following be defined in app_cfg.h:
*                   APP_CFG_SW_TASK_PRIO
*                   APP_CFG_SW_TASK_STK_SIZE
*
* 10/06/2024 TDM
*********************************************************************
* Public Resources
********************************************************************/
#ifndef CS_SW_DEF
#define CS_SW_DEF
/*********************************************************************
* SW_T - Switch values
********************************************************************/
typedef enum {SWN,SW2,SW3} SW_T;

/*********************************************************************
* SWPend - Pend on SW press
*          tout - semaphore timeout
*          *err - destination of err code
*
*          Error codes are identical to a semaphore
********************************************************************/
SW_T SwPend(INT16U tout, OS_ERR *os_err);

/*********************************************************************
* SWInit
********************************************************************/
void SwInit(void);             /* Switch Initialization    */

#endif
