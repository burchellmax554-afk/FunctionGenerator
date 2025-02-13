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
*/

/*
*********************************************************************************************************
*                                        DATA QUEUE MANAGEMENT
*
* File    : os_data.c
* Version : V3.09.02
*********************************************************************************************************
*/

#define  CESIUM_SOURCE
#include "os.h"

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *os_data__c = "$Id: $";
#endif

#if (OS_CFG_DATA_EN > 0u)
/*
************************************************************************************************************************
*                                                 CREATE A DATA QUEUE
*
* Description: This function is called by your application to create a data queue.  Data queues MUST be created
*              before they can be used.
*
* Arguments  : p_data      is a pointer to the data queue object
*
*              p_name      is a pointer to an ASCII string that will be used to name the data queue
*
*              p_storage   storage area (RAM) where all data item entries will be stored.  Storage should hold at least
*                          (max_entries * item_size) bytes of storage.
*
*              max_entries indicates the maximum number of data elements that can be placed in the data queue.  This
*                          number cannot be 0.  However, the data queue can hold a single item.
*
*              item_size   the size in #bytes of a data element.
*
*              p_err       is a pointer to a variable that will contain an error code returned by this function.
*
*                              OS_ERR_NONE                    The call was successful
*                              OS_ERR_CREATE_ISR              Can't create from an ISR
*                              OS_ERR_ILLEGAL_CREATE_RUN_TIME If you are trying to create the Queue after you called
*                                                             OSSafetyCriticalStart()
*                              OS_ERR_OBJ_PTR_NULL            If you passed a NULL pointer for 'p_data'
*                              OS_ERR_DATA_SIZE               If the 'item_size' you specified is invalid
*                              OS_ERR_OBJ_CREATED             If the data queue was already created
*
* Returns    : none
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSDataCreate (OS_DATA      *p_data,
                    CPU_CHAR     *p_name,
                    CPU_VOID     *p_storage,
                    OS_MSG_QTY    max_entries,
                    OS_MSG_SIZE   item_size,
                    OS_ERR       *p_err)
{
    CPU_SR_ALLOC();


#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
       *p_err = OS_ERR_ILLEGAL_CREATE_RUN_TIME;
        return;
    }
#endif

#if (OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u)
    if (OSIntNestingCtr > 0u) {                                 /* Not allowed to be called from an ISR                 */
       *p_err = OS_ERR_CREATE_ISR;
        return;
    }
#endif

#if (OS_CFG_ARG_CHK_EN > 0u)
    if (p_data == (OS_DATA *)0) {                               /* Validate arguments                                   */
       *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
    if (max_entries == 0u) {                                    /* Cannot specify a zero size queue (but can be 1)      */
       *p_err = OS_ERR_Q_SIZE;
        return;
    }

    if (item_size == 0u) {                                      /* Data item must at least be able to hold 1 byte       */
       *p_err = OS_ERR_DATA_SIZE;
        return;
    }
#endif

    CPU_CRITICAL_ENTER();
#if (OS_OBJ_TYPE_REQ > 0u)
#if (OS_CFG_OBJ_CREATED_CHK_EN > 0u)
    if (p_data->Type == OS_OBJ_TYPE_DATA) {
        CPU_CRITICAL_EXIT();
       *p_err = OS_ERR_OBJ_CREATED;
        return;
    }
#endif
    p_data->Type    = OS_OBJ_TYPE_DATA;                         /* Mark the data structure as a data queue              */
#endif
#if (OS_CFG_DBG_EN > 0u)
    p_data->NamePtr = p_name;
#else
    (void)p_name;
#endif

    p_data->StorageBaseAddr = p_storage;                        /* Initialize the OS_DATA structure                     */
    p_data->ItemSize        = item_size;
    p_data->MaxEntries      = max_entries;
    OS_DataStorageClr(p_data);

    OS_PendListInit(&p_data->PendList);                         /* Initialize the waiting lists                         */

#if (OS_CFG_DBG_EN > 0u)
    OS_DataDbgListAdd(p_data);
    OSDataQty++;                                                /* One more data queue created                          */
#endif

#if (OS_CREATE_EXT > 0u)
    p_data->CreateOpt = OS_OPT_CREATE_PRIO;
#endif

    OS_TRACE_DATA_CREATE(p_data, p_name);
    CPU_CRITICAL_EXIT();
   *p_err = OS_ERR_NONE;
}


#if (OS_CFG_DATA_CREATE_EXT > 0u)
/*
************************************************************************************************************************
*                                            CREATE A DATA QUEUE (Extended)
*
* Description: This function creates a data queue and provides an option field to configure how it behaves.
*
* Arguments  : p_data      is a pointer to the data queue object
*
*              p_name      is a pointer to an ASCII string that will be used to name the data queue
*
*              p_storage   storage area (RAM) where all data item entries will be stored.  Storage should hold at least
*                          (max_entries * item_size) bytes of storage.
*
*              max_entries indicates the maximum number of data elements that can be placed in the data queue.  This
*                          number cannot be 0.  However, the data queue can hold a single item.
*
*              item_size   the size in #bytes of a data element.
*
*              opt         specifies the options used when creating the data queue
*
*                              OS_OPT_CREATE_PRIO             Pending tasks are posted to in order of prio
*                              OS_OPT_CREATE_FIFO             Pending tasks are posted to in the order they pended
*
*              p_err       is a pointer to a variable that will contain an error code returned by this function.
*
*                              OS_ERR_NONE                    The call was successful
*                              OS_ERR_CREATE_ISR              Can't create from an ISR
*                              OS_ERR_ILLEGAL_CREATE_RUN_TIME If you are trying to create the Queue after you called
*                                                             OSSafetyCriticalStart()
*                              OS_ERR_OBJ_PTR_NULL            If you passed a NULL pointer for 'p_data'
*                              OS_ERR_DATA_SIZE               If the 'item_size' you specified is invalid
*                              OS_ERR_OBJ_CREATED             If the data queue was already created
*                              OS_ERR_OPT_INVALID             If the opt argument is invalid
*
* Returns    : none
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSDataCreateExt (OS_DATA      *p_data,
                       CPU_CHAR     *p_name,
                       CPU_VOID     *p_storage,
                       OS_MSG_QTY    max_entries,
                       OS_MSG_SIZE   item_size,
                       OS_OPT        opt,
                       OS_ERR       *p_err)
{
    CPU_SR_ALLOC();


#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
       *p_err = OS_ERR_ILLEGAL_CREATE_RUN_TIME;
        return;
    }
#endif

#if (OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u)
    if (OSIntNestingCtr > 0u) {                                 /* Not allowed to be called from an ISR                 */
       *p_err = OS_ERR_CREATE_ISR;
        return;
    }
#endif

#if (OS_CFG_ARG_CHK_EN > 0u)
    if (p_data == (OS_DATA *)0) {                               /* Validate arguments                                   */
       *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
    if (max_entries == 0u) {                                    /* Cannot specify a zero size queue (but can be 1)      */
       *p_err = OS_ERR_Q_SIZE;
        return;
    }

    if (item_size == 0u) {                                      /* Data item must at least be able to hold 1 byte       */
       *p_err = OS_ERR_DATA_SIZE;
        return;
    }
#endif

    CPU_CRITICAL_ENTER();
    switch (opt) {
        case OS_OPT_CREATE_PRIO:
        case OS_OPT_CREATE_FIFO:
             break;

        default:
            *p_err = OS_ERR_OPT_INVALID;
             return;
    }

#if (OS_OBJ_TYPE_REQ > 0u)
#if (OS_CFG_OBJ_CREATED_CHK_EN > 0u)
    if (p_data->Type == OS_OBJ_TYPE_DATA) {
        CPU_CRITICAL_EXIT();
       *p_err = OS_ERR_OBJ_CREATED;
        return;
    }
#endif
    p_data->Type    = OS_OBJ_TYPE_DATA;                         /* Mark the data structure as a data queue              */
#endif
#if (OS_CFG_DBG_EN > 0u)
    p_data->NamePtr = p_name;
#else
    (void)p_name;
#endif

    p_data->StorageBaseAddr = p_storage;                        /* Initialize the OS_DATA structure                     */
    p_data->ItemSize        = item_size;
    p_data->MaxEntries      = max_entries;
    OS_DataStorageClr(p_data);

    OS_PendListInit(&p_data->PendList);                         /* Initialize the waiting lists                         */

#if (OS_CFG_DBG_EN > 0u)
    OS_DataDbgListAdd(p_data);
    OSDataQty++;                                                /* One more data queue created                          */
#endif

    p_data->CreateOpt = opt;

    OS_TRACE_DATA_CREATE(p_data, p_name);
    CPU_CRITICAL_EXIT();
   *p_err = OS_ERR_NONE;
}
#endif


/*
************************************************************************************************************************
*                                                  DELETE A DATA QUEUE
*
* Description: This function deletes a data queue and readies all tasks pending on the queue.
*
* Arguments  : p_data    is a pointer to the data queue you want to delete
*
*              opt       determines delete options as follows:
*
*                            OS_OPT_DEL_NO_PEND          Delete the queue ONLY if no task pending either for the data
*                                                        queue to contain elements or for tasks waiting because the
*                                                        data queue is already full.
*                            OS_OPT_DEL_ALWAYS           Deletes the data queue even if tasks are waiting.
*                                                        In this case, all the tasks pending will be readied.
*
*              p_err     is a pointer to a variable that will contain an error code returned by this function.
*
*                            OS_ERR_NONE                    The call was successful and the queue was deleted
*                            OS_ERR_DEL_ISR                 If you tried to delete the queue from an ISR
*                            OS_ERR_ILLEGAL_DEL_RUN_TIME    If you are trying to delete the message queue after you
*                                                           called OSStart()
*                            OS_ERR_OBJ_PTR_NULL            If you pass a NULL pointer for 'p_data'
*                            OS_ERR_OBJ_TYPE                If the message queue was not created
*                            OS_ERR_OPT_INVALID             An invalid option was specified
*                            OS_ERR_OS_NOT_RUNNING          If Cs/OS3 is not running yet
*                            OS_ERR_TASK_WAITING            One or more tasks were waiting on the queue
*
* Returns    : == 0          if no tasks were waiting on the queue, or upon error.
*              >  0          if one or more tasks waiting on the queue are now readied and informed.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of the queue MUST
*                 check the return code of OSQPend().
*
*              2) Because ALL tasks pending on the queue will be readied, you MUST be careful in applications where the
*                 queue is used for mutual exclusion because the resource(s) will no longer be guarded by the queue.
************************************************************************************************************************
*/

#if (OS_CFG_DATA_DEL_EN > 0u)
OS_OBJ_QTY  OSDataDel (OS_DATA  *p_data,
                       OS_OPT    opt,
                       OS_ERR   *p_err)
{
    OS_OBJ_QTY     nbr_tasks;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    CPU_SR_ALLOC();


#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

    OS_TRACE_DATA_DEL_ENTER(p_data, opt);

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == OS_TRUE) {
        OS_TRACE_DATA_DEL_EXIT(OS_ERR_ILLEGAL_DEL_RUN_TIME);
       *p_err = OS_ERR_ILLEGAL_DEL_RUN_TIME;
        return (0u);
    }
#endif

#if (OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u)
    if (OSIntNestingCtr > 0u) {                                 /* Can't delete a data queue from an ISR                */
        OS_TRACE_DATA_DEL_EXIT(OS_ERR_DEL_ISR);
       *p_err = OS_ERR_DEL_ISR;
        return (0u);
    }
#endif

#if (OS_CFG_INVALID_OS_CALLS_CHK_EN > 0u)                       /* Is the kernel running?                               */
    if (OSRunning != OS_STATE_OS_RUNNING) {
        OS_TRACE_DATA_DEL_EXIT(OS_ERR_OS_NOT_RUNNING);
       *p_err = OS_ERR_OS_NOT_RUNNING;
        return (0u);
    }
#endif

#if (OS_CFG_ARG_CHK_EN > 0u)
    if (p_data == (OS_DATA *)0) {                               /* Validate 'p_data'                                    */
        OS_TRACE_DATA_DEL_EXIT(OS_ERR_OBJ_PTR_NULL);
       *p_err = OS_ERR_OBJ_PTR_NULL;
        return (0u);
    }
#endif

#if (OS_CFG_OBJ_TYPE_CHK_EN > 0u)
    if (p_data->Type != OS_OBJ_TYPE_DATA) {                     /* Make sure data queue was created                     */
        OS_TRACE_DATA_DEL_EXIT(OS_ERR_OBJ_TYPE);
       *p_err = OS_ERR_OBJ_TYPE;
        return (0u);
    }
#endif

    CPU_CRITICAL_ENTER();
    p_pend_list = &p_data->PendList;
    nbr_tasks   = 0u;
    switch (opt) {
        case OS_OPT_DEL_NO_PEND:                                /* Delete data queue only if no task waiting            */
             if (p_pend_list->HeadPtr == (OS_TCB *)0) {
#if (OS_CFG_DBG_EN > 0u)
                 OS_DataDbgListRemove(p_data);
                 OSDataQty--;
#endif
                 OS_TRACE_DATA_DEL(p_data);
                 p_data->StorageBaseAddr = (CPU_INT08U *)0;
                 p_data->MaxEntries      = 0u;                  /* Any data sent is lost                                */
                 OS_DataStorageClr(p_data);
#if (OS_CREATE_EXT > 0u)
                 p_data->CreateOpt       =  0u;
#endif
                 CPU_CRITICAL_EXIT();
                *p_err = OS_ERR_NONE;
             } else {
                 CPU_CRITICAL_EXIT();
                *p_err = OS_ERR_TASK_WAITING;
             }
             break;

        case OS_OPT_DEL_ALWAYS:                                 /* Always delete the message queue                      */
             while (p_pend_list->HeadPtr != (OS_TCB *)0) {      /* Remove all tasks from the pend list                  */
                 p_tcb = p_pend_list->HeadPtr;
                 OS_PendAbort(p_tcb,
                              0u,
                              OS_STATUS_PEND_DEL);
                 nbr_tasks++;
             }
#if (OS_CFG_DBG_EN > 0u)
             OS_DataDbgListRemove(p_data);                      /* Remove the OS_DATA object from the debuf list        */
             OSDataQty--;
#endif
#if (OS_OBJ_TYPE_REQ > 0u)
             p_data->Type    = OS_OBJ_TYPE_NONE;                /* Mark the data structure as a NONE                    */
#endif
#if (OS_CFG_DBG_EN > 0u)
             p_data->NamePtr = (CPU_CHAR *)((void *)"?DATA");
#endif
             OS_PendListInit(&p_data->PendList);                /* Initialize the waiting list                          */
             p_data->StorageBaseAddr = (CPU_INT08U *)0;
             p_data->MaxEntries      = 0u;                      /* Any data sent is lost                                */
             OS_DataStorageClr(p_data);
             OS_TRACE_DATA_DEL(p_data);
#if (OS_CREATE_EXT > 0u)
             p_data->CreateOpt       =  0u;
#endif
             CPU_CRITICAL_EXIT();
             OSSched();                                         /* Find highest priority task ready to run              */
            *p_err = OS_ERR_NONE;
             break;

        default:
             CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_OPT_INVALID;
             break;
    }
    OS_TRACE_DATA_DEL_EXIT(*p_err);
    return (nbr_tasks);
}
#endif


/*
************************************************************************************************************************
*                                                   FLUSH DATA QUEUE
*
* Description : This function is used to flush the contents of the data queue.
*
* Arguments   : p_data     is a pointer to the data queue to flush
*
*               p_err      is a pointer to a variable that will contain an error code returned by this function.
*
*                              OS_ERR_NONE              Upon success
*                              OS_ERR_FLUSH_ISR         If you called this function from an ISR
*                              OS_ERR_OBJ_PTR_NULL      If you passed a NULL pointer for 'p_data'
*                              OS_ERR_OBJ_TYPE          If you didn't create the data queue
*                              OS_ERR_OS_NOT_RUNNING    If Cs/OS3 is not running yet
*
*
* Returns     : == 0       if no entries were freed, or upon error.
*               >  0       the number of freed entries.
*
* Note(s)     : none
************************************************************************************************************************
*/

#if (OS_CFG_DATA_FLUSH_EN > 0u)
CPU_DATA  OSDataFlush (OS_DATA  *p_data,
                       OS_ERR   *p_err)
{
    CPU_DATA  entries;
    CPU_SR_ALLOC();


#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if (OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u)
    if (OSIntNestingCtr > 0u) {                                 /* Can't flush a data queue from an ISR                 */
       *p_err = OS_ERR_FLUSH_ISR;
        return (0u);
    }
#endif

#if (OS_CFG_INVALID_OS_CALLS_CHK_EN > 0u)                       /* Is the kernel running?                               */
    if (OSRunning != OS_STATE_OS_RUNNING) {
       *p_err = OS_ERR_OS_NOT_RUNNING;
        return (0u);
    }
#endif

#if (OS_CFG_ARG_CHK_EN > 0u)
    if (p_data == (OS_DATA *)0) {                               /* Validate arguments                                   */
       *p_err = OS_ERR_OBJ_PTR_NULL;
        return (0u);
    }
#endif

#if (OS_CFG_OBJ_TYPE_CHK_EN > 0u)
    if (p_data->Type != OS_OBJ_TYPE_DATA) {                     /* Make sure data queue was created                     */
       *p_err = OS_ERR_OBJ_TYPE;
        return (0u);
    }
#endif

    CPU_CRITICAL_ENTER();
    entries         = p_data->Entries;                          /* Any data sent is lost                                */
    OS_DataStorageClr(p_data);
    CPU_CRITICAL_EXIT();
   *p_err           = OS_ERR_NONE;
    return (entries);
}
#endif


/*
************************************************************************************************************************
*                                                 PEND ON A DATA QUEUE
*
* Description: This function waits for a data to be sent to a data queue.
*
* Arguments  : p_data        is a pointer to the data queue
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will wait for a
*                            data to arrive at the queue up to the amount of time specified by this argument.  If you
*                            specify 0, however, your task will wait forever at the specified queue or, until a message
*                            arrives.
*
*              opt           determines whether the user wants to block if the data queue is empty or not:
*
*                                OS_OPT_PEND_BLOCKING
*                                OS_OPT_PEND_NON_BLOCKING
*                                OS_OPT_PEND_PEEK           Peek at queue without consuming
*
*              p_msg         is a pointer to the storage area that will receive a COPY of the data sent
*
*              item_size     is the size of the storage area which will receive a COPY of the data sent
*
*              p_err         is a pointer to a variable that will contain an error code returned by this function.
*
*                                OS_ERR_NONE               The call was successful and your task received a copy of
*                                                          the data sent
*                                OS_ERR_OBJ_DEL            If 'p_data' was deleted
*                                OS_ERR_OBJ_PTR_NULL       If you pass a NULL pointer for 'p_data'
*                                OS_ERR_OBJ_TYPE           If the data queue was not created
*                                OS_ERR_OPT_INVALID        You specified an invalid option
*                                OS_ERR_OS_NOT_RUNNING     If Cs/OS3 is not running yet
*                                OS_ERR_PEND_ABORT         The pend was aborted
*                                OS_ERR_PEND_ISR           If you called this function from an ISR
*                                OS_ERR_PEND_TMR           If you tried to PEND from a Timer
*                                OS_ERR_PEND_WOULD_BLOCK   If you specified non-blocking but the queue was empty
*                                OS_ERR_PEND_EMPTY         If you specified the peek option but the queue was empty
*                                OS_ERR_PTR_INVALID        If you passed a NULL pointer of 'p_dest'
*                                OS_ERR_SCHED_LOCKED       The scheduler is locked
*                                OS_ERR_STATUS_INVALID     If the pend status has an invalid value
*                                OS_ERR_TIMEOUT            Data was not received within the specified timeout and
*                                                          would lead to blocking.
*                                OS_ERR_TICK_DISABLED      If kernel ticks are disabled and a timeout is specified
*                                OS_ERR_DATA_SIZE          If the item_size you specify doesn't match that of the queue
*
* Returns    : None
*
* Note(s)    : This API 'MUST NOT' be called from a timer callback function.
************************************************************************************************************************
*/

void  OSDataPend (OS_DATA      *p_data,
                  OS_TICK       timeout,
                  OS_OPT        opt,
                  CPU_VOID     *p_msg,
                  OS_MSG_SIZE   item_size,
                  OS_ERR       *p_err)
{
    CPU_INT08U   *p_src;
    CPU_INT08U   *p_dest;
    OS_MSG_SIZE   i;
    CPU_SR_ALLOC();


#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return ((void *)0);
    }
#endif

    OS_TRACE_DATA_PEND_ENTER(p_data, timeout, opt, p_msg, 0);

#if (OS_CFG_TICK_EN == 0u)
    if (timeout != 0u) {
       *p_err = OS_ERR_TICK_DISABLED;
        OS_TRACE_DATA_PEND_FAILED(p_data);
        OS_TRACE_DATA_PEND_EXIT(OS_ERR_TICK_DISABLED);
        return;
    }
#endif

#if (OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u)
    if (OSIntNestingCtr > 0u) {                                 /* Not allowed to use PEND_BLOCKING from an ISR         */
        if (opt == OS_OPT_PEND_BLOCKING) {
           *p_err = OS_ERR_PEND_ISR;
            OS_TRACE_DATA_PEND_FAILED(p_data);
            OS_TRACE_DATA_PEND_EXIT(OS_ERR_PEND_ISR);
            return;
        }
    }
#endif

#if ((OS_CFG_TMR_EN                 > 0u) && \
     (OS_CFG_CALLED_FROM_TMR_CHK_EN > 0u))
    if (OSTCBCurPtr == &OSTmrTaskTCB) {                         /* Don't allow blocking-pends from Timer Callbacks      */
        if (opt == OS_OPT_PEND_BLOCKING) {
            OS_TRACE_DATA_PEND_FAILED(p_data);
            OS_TRACE_DATA_PEND_EXIT(OS_ERR_PEND_TMR);
           *p_err = OS_ERR_PEND_TMR;
            return;
        }
    }
#endif

#if (OS_CFG_INVALID_OS_CALLS_CHK_EN > 0u)                       /* Is the kernel running?                               */
    if (OSRunning != OS_STATE_OS_RUNNING) {
        OS_TRACE_DATA_PEND_EXIT(OS_ERR_OS_NOT_RUNNING);
       *p_err = OS_ERR_OS_NOT_RUNNING;
        return;
    }
#endif

#if (OS_CFG_ARG_CHK_EN > 0u)
    if (p_data == (OS_DATA *)0) {                               /* Validate arguments                                   */
        OS_TRACE_DATA_PEND_FAILED(p_data);
        OS_TRACE_DATA_PEND_EXIT(OS_ERR_OBJ_PTR_NULL);
       *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
    if (p_msg == (CPU_INT08U *)0) {
        OS_TRACE_DATA_PEND_FAILED(p_data);
        OS_TRACE_DATA_PEND_EXIT(OS_ERR_PTR_INVALID);
       *p_err = OS_ERR_PTR_INVALID;
        return;
    }
    switch (opt) {
        case OS_OPT_PEND_BLOCKING:
        case OS_OPT_PEND_NON_BLOCKING:
        case OS_OPT_PEND_PEEK:
             break;

        default:
             OS_TRACE_DATA_PEND_FAILED(p_data);
             OS_TRACE_DATA_PEND_EXIT(OS_ERR_OPT_INVALID);
            *p_err = OS_ERR_OPT_INVALID;
             return;
    }
#endif

#if (OS_CFG_OBJ_TYPE_CHK_EN > 0u)
    if (p_data->Type != OS_OBJ_TYPE_DATA) {                     /* Make sure data queue was created                     */
        OS_TRACE_DATA_PEND_FAILED(p_data);
        OS_TRACE_DATA_PEND_EXIT(OS_ERR_OBJ_TYPE);
       *p_err = OS_ERR_OBJ_TYPE;
        return;
    }
#endif

    if (item_size != p_data->ItemSize) {
       *p_err = OS_ERR_DATA_SIZE;
        return;
    }

    CPU_CRITICAL_ENTER();
    p_dest = (CPU_INT08U *)p_msg;
                                                                /* ---------------- QUEUE IS NOT EMPTY ---------------- */
    if (p_data->Entries > 0u) {                                 /* Any data waiting in the data queue?                  */
        p_src = &((CPU_INT08U *)p_data->StorageBaseAddr)[p_data->OutIx * p_data->ItemSize];
        for (i = 0; i < p_data->ItemSize; i++) {
           *p_dest = *p_src;
            p_dest++;
            p_src++;
        }
        if ((opt & OS_OPT_PEND_PEEK) != OS_OPT_PEND_PEEK) {
            p_data->Entries--;                                  /* One less entry in the data queue                     */
            if (p_data->OutIx == (p_data->MaxEntries - 1u)) {   /* Position to next entry to extract                    */
                p_data->OutIx = 0u;
            } else {
                p_data->OutIx++;
            }
        }
        OS_TRACE_DATA_PEND(p_data);
        CPU_CRITICAL_EXIT();
        OS_TRACE_DATA_PEND_EXIT(OS_ERR_NONE);
       *p_err = OS_ERR_NONE;
        return;                                                 /* Yes, Return a copy of the data                       */
    }
                                                                /* ------------------ QUEUE IS EMPTY ------------------ */
    if ((opt & OS_OPT_PEND_PEEK) != 0u) {                       /* Caller can't peek at empty Queue                     */
        CPU_CRITICAL_EXIT();
        OS_TRACE_DATA_PEND_FAILED(p_data);
        OS_TRACE_DATA_PEND_EXIT(OS_ERR_PEND_EMPTY);
       *p_err = OS_ERR_PEND_EMPTY;
        return;

    } else if ((opt & OS_OPT_PEND_NON_BLOCKING) != 0u) {        /* Caller won't block on empty queue                    */
        CPU_CRITICAL_EXIT();
        OS_TRACE_DATA_PEND_FAILED(p_data);
        OS_TRACE_DATA_PEND_EXIT(OS_ERR_PEND_WOULD_BLOCK);
       *p_err = OS_ERR_PEND_WOULD_BLOCK;
        return;

    } else {                                                    /* Caller wants to pend on empty queue                  */
        if (OSSchedLockNestingCtr > 0u) {                       /* Can't pend when the scheduler is locked              */
            CPU_CRITICAL_EXIT();
            OS_TRACE_DATA_PEND_FAILED(p_data);
            OS_TRACE_DATA_PEND_EXIT(OS_ERR_SCHED_LOCKED);
           *p_err = OS_ERR_SCHED_LOCKED;
            return;
        }
    }

    OSTCBCurPtr->MsgPtr  = p_msg;                               /* Store pointer to where received data will be placed  */
    OSTCBCurPtr->MsgSize = p_data->ItemSize;

    OS_Pend((OS_PEND_OBJ *)((void *)p_data),                    /* Block task pending on Data Queue                     */
                                    OSTCBCurPtr,
                                    OS_TASK_PEND_ON_DATA,
                                    timeout);
    CPU_CRITICAL_EXIT();
    OS_TRACE_DATA_PEND_BLOCK(p_data);

    OSSched();                                                  /* Find the next highest priority task ready to run     */

    CPU_CRITICAL_ENTER();
    switch (OSTCBCurPtr->PendStatus) {
        case OS_STATUS_PEND_OK:                                 /* Data already placed into buffer by OSDataPost()      */
             OS_TRACE_DATA_PEND(p_data);
            *p_err = OS_ERR_NONE;
             break;

        case OS_STATUS_PEND_ABORT:                              /* Indicate that we aborted                             */
             OS_TRACE_DATA_PEND_FAILED(p_data);
            *p_err = OS_ERR_PEND_ABORT;
             break;

        case OS_STATUS_PEND_TIMEOUT:                            /* Indicate that we didn't get event within TO          */
             OS_TRACE_DATA_PEND_FAILED(p_data);
            *p_err = OS_ERR_TIMEOUT;
             break;

        case OS_STATUS_PEND_DEL:                                /* Indicate that object pended on has been deleted      */
             OS_TRACE_DATA_PEND_FAILED(p_data);
            *p_err = OS_ERR_OBJ_DEL;
             break;

        default:
             OS_TRACE_DATA_PEND_FAILED(p_data);
            *p_err = OS_ERR_STATUS_INVALID;
             break;
    }
    CPU_CRITICAL_EXIT();
    OS_TRACE_DATA_PEND_EXIT(*p_err);
    return;
}


/*
************************************************************************************************************************
*                                             ABORT WAITING ON A DATA QUEUE
*
* Description: This function aborts & readies any tasks currently waiting on a data queue.  This function should be used
*              to fault-abort the wait on the queue, rather than to normally signal the queue via OSDataPost().
*
* Arguments  : p_data    is a pointer to the data queue
*
*              opt       determines the type of ABORT performed:
*
*                            OS_OPT_PEND_ABORT_1          ABORT wait for a single task (HPT) waiting on the queue
*                            OS_OPT_PEND_ABORT_ALL        ABORT wait for ALL tasks that are  waiting on the queue
*                            OS_OPT_POST_NO_SCHED         Do not call the scheduler
*
*              p_err     is a pointer to a variable that will contain an error code returned by this function.
*
*                            OS_ERR_NONE                  At least one task waiting on the queue was readied and
*                                                         informed of the aborted wait; check return value for the
*                                                         number of tasks whose wait on the queue was aborted
*                            OS_ERR_OBJ_PTR_NULL          If you pass a NULL pointer for 'p_data'
*                            OS_ERR_OBJ_TYPE              If the message queue was not created
*                            OS_ERR_OPT_INVALID           You specified an invalid option
*                            OS_ERR_OS_NOT_RUNNING        If Cs/OS3 is not running yet
*                            OS_ERR_PEND_ABORT_ISR        If this function was called from an ISR
*                            OS_ERR_PEND_ABORT_NONE       No task were pending
*
* Returns    : == 0      if no tasks were waiting on the queue, or upon error.
*              >  0      if one or more tasks waiting on the queue are now readied and informed.
*
* Note(s)    : none
************************************************************************************************************************
*/

#if (OS_CFG_DATA_PEND_ABORT_EN > 0u)
OS_OBJ_QTY  OSDataPendAbort (OS_DATA  *p_data,
                             OS_OPT    opt,
                             OS_ERR   *p_err)
{
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    OS_OBJ_QTY     nbr_tasks;
    CPU_SR_ALLOC();


#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return (0u);
    }
#endif

#if (OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u)
    if (OSIntNestingCtr > 0u) {                                 /* Not allowed to Pend Abort from an ISR                */
       *p_err = OS_ERR_PEND_ABORT_ISR;
        return (0u);
    }
#endif

#if (OS_CFG_INVALID_OS_CALLS_CHK_EN > 0u)                       /* Is the kernel running?                               */
    if (OSRunning != OS_STATE_OS_RUNNING) {
       *p_err = OS_ERR_OS_NOT_RUNNING;
        return (0u);
    }
#endif

#if (OS_CFG_ARG_CHK_EN > 0u)
    if (p_data == (OS_DATA *)0) {                               /* Validate 'p_data'                                    */
       *p_err = OS_ERR_OBJ_PTR_NULL;
        return (0u);
    }
    switch (opt) {                                              /* Validate 'opt'                                       */
        case OS_OPT_PEND_ABORT_1:
        case OS_OPT_PEND_ABORT_ALL:
        case OS_OPT_PEND_ABORT_1   | OS_OPT_POST_NO_SCHED:
        case OS_OPT_PEND_ABORT_ALL | OS_OPT_POST_NO_SCHED:
             break;

        default:
            *p_err = OS_ERR_OPT_INVALID;
             return (0u);
    }
#endif

#if (OS_CFG_OBJ_TYPE_CHK_EN > 0u)
    if (p_data->Type != OS_OBJ_TYPE_DATA) {                     /* Make sure queue was created                          */
       *p_err = OS_ERR_OBJ_TYPE;
        return (0u);
    }
#endif

    CPU_CRITICAL_ENTER();
    p_pend_list = &p_data->PendList;
    if (p_pend_list->HeadPtr == (OS_TCB *)0) {                  /* Any task waiting on queue?                           */
        CPU_CRITICAL_EXIT();                                    /* No                                                   */
       *p_err = OS_ERR_PEND_ABORT_NONE;
        return (0u);
    }

    nbr_tasks = 0u;
    while (p_pend_list->HeadPtr != (OS_TCB *)0) {
        p_tcb = p_pend_list->HeadPtr;
        OS_PendAbort(p_tcb,
                     0u,
                     OS_STATUS_PEND_ABORT);
        nbr_tasks++;
        if (opt != OS_OPT_PEND_ABORT_ALL) {                     /* Pend abort all tasks waiting?                        */
            break;                                              /* No                                                   */
        }
    }
    CPU_CRITICAL_EXIT();

    if ((opt & OS_OPT_POST_NO_SCHED) == 0u) {
        OSSched();                                              /* Run the scheduler                                    */
    }

   *p_err = OS_ERR_NONE;
    return (nbr_tasks);
}
#endif


/*
************************************************************************************************************************
*                                                POST DATA TO A QUEUE
*
* Description: This function sends data to a queue.  With the 'opt' argument, your application can specify whether the
*              message is posting the data to the front of the queue (LIFO) or normally (FIFO) at the end of the queue.
*
* Arguments  : p_data        is a pointer to a data queue that must have been created by OSDataCreate().
*
*              p_msg         is a pointer to the data to send.
*
*              item_size     specifies the size of the data message sent (in bytes)
*
*              opt           determines the type of POST performed:
*
*                                OS_OPT_POST_FIFO         POST data to end of queue (FIFO) and wake up a single
*                                                         waiting task.
*                                OS_OPT_POST_LIFO         POST data to the front of the queue (LIFO) and wake up
*                                                         a single waiting task.
*                                OS_OPT_POST_FIFO_OVER    Overwrite last FIFO entry
*
*                                OS_OPT_POST_LIFO_OVER    Overwrite last LIFO entry
*
*                                OS_OPT_POST_NO_SCHED     Do not call the scheduler
*
*                            Note(s): 1) OS_OPT_POST_NO_SCHED can be added (or OR'd) with one of the other options.
*                                     2) Possible combination of options are:
*
*                                        OS_OPT_POST_FIFO
*                                        OS_OPT_POST_LIFO
*                                        OS_OPT_POST_FIFO + OS_OPT_POST_NO_SCHED
*                                        OS_OPT_POST_LIFO + OS_OPT_POST_NO_SCHED
*
*              p_err         is a pointer to a variable that will contain an error code returned by this function.
*
*                                OS_ERR_NONE              The call was successful and the data was sent
*                                OS_ERR_OBJ_PTR_NULL      If 'p_data' is a NULL pointer
*                                OS_ERR_OBJ_TYPE          If the data queue was not initialized
*                                OS_ERR_OPT_INVALID       You specified an invalid option
*                                OS_ERR_OS_NOT_RUNNING    If Cs/OS3 is not running yet
*                                OS_ERR_Q_MAX             If the queue is full
*                                OS_ERR_DATA_SIZE         If the item_size you specify doesn't match that of the queue
*
* Returns    : None
*
* Note(s)    : none
************************************************************************************************************************
*/

void  OSDataPost (OS_DATA      *p_data,
                  CPU_VOID     *p_msg,
                  OS_MSG_SIZE   item_size,
                  OS_OPT        opt,
                  OS_ERR       *p_err)
{
    OS_OPT         post_type;
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    CPU_INT08U    *p_src;
    CPU_INT08U    *p_dest;
    OS_MSG_SIZE    i;
    CPU_SR_ALLOC();


#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

    OS_TRACE_DATA_POST_ENTER(p_data, p_msg, msg_size, opt);

#if (OS_CFG_INVALID_OS_CALLS_CHK_EN > 0u)                       /* Is the kernel running?                               */
    if (OSRunning != OS_STATE_OS_RUNNING) {
        OS_TRACE_DATA_POST_EXIT(OS_ERR_OS_NOT_RUNNING);
       *p_err = OS_ERR_OS_NOT_RUNNING;
        return;
    }
#endif

#if (OS_CFG_ARG_CHK_EN > 0u)
    if (p_data == (OS_DATA *)0) {                               /* Validate 'p_data'                                    */
        OS_TRACE_DATA_POST_FAILED(p_data);
        OS_TRACE_DATA_POST_EXIT(OS_ERR_OBJ_PTR_NULL);
       *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
    switch (opt) {                                              /* Validate 'opt'                                       */
        case OS_OPT_POST_FIFO:
        case OS_OPT_POST_LIFO:
        case OS_OPT_POST_FIFO      | OS_OPT_POST_NO_SCHED:
        case OS_OPT_POST_LIFO      | OS_OPT_POST_NO_SCHED:
        case OS_OPT_POST_FIFO_OVER:
        case OS_OPT_POST_LIFO_OVER:
        case OS_OPT_POST_FIFO_OVER | OS_OPT_POST_NO_SCHED:
        case OS_OPT_POST_LIFO_OVER | OS_OPT_POST_NO_SCHED:
             break;

        default:
             OS_TRACE_DATA_POST_FAILED(p_data);
             OS_TRACE_DATA_POST_EXIT(OS_ERR_OPT_INVALID);
            *p_err =  OS_ERR_OPT_INVALID;
             return;
    }
#endif

#if (OS_CFG_OBJ_TYPE_CHK_EN > 0u)
    if (p_data->Type != OS_OBJ_TYPE_DATA) {                     /* Make sure data queue was created                     */
        OS_TRACE_DATA_POST_FAILED(p_data);
        OS_TRACE_DATA_POST_EXIT(OS_ERR_OBJ_TYPE);
       *p_err = OS_ERR_OBJ_TYPE;
        return;
    }
#endif

    if (item_size != p_data->ItemSize) {
       *p_err = OS_ERR_DATA_SIZE;
        return;
    }

    OS_TRACE_DATA_POST(p_data);

    CPU_CRITICAL_ENTER();
   *p_err       = OS_ERR_NONE;
    p_pend_list = &p_data->PendList;
    p_src       = (CPU_INT08U *)p_msg;
    if (p_pend_list->HeadPtr == (OS_TCB *)0) {                  /* Any task waiting on data queue?                      */
                                                                /* NO, store the data in the queue itself               */
        post_type = opt & OS_OPT_POST_FIFO_LIFO_MSK;            /* Extract the option field                             */
        switch (post_type) {
            case  OS_OPT_POST_FIFO:
                  if (p_data->Entries < p_data->MaxEntries) {   /* FIFO only if we have room                            */
                      p_dest = &((CPU_INT08U *)p_data->StorageBaseAddr)[p_data->InIx * p_data->ItemSize];
                      for (i = 0u; i < p_data->ItemSize; i++) {
                         *p_dest = *p_src;
                          p_dest++;
                          p_src++;
                      }
                      if (p_data->InIx == (p_data->MaxEntries - 1u)) {
                          p_data->InIx = 0u;
                      } else {
                          p_data->InIx++;
                      }
                      p_data->Entries++;
                  } else {
                      *p_err = OS_ERR_Q_MAX;
                  }
                  break;

            case OS_OPT_POST_LIFO:
                 if (p_data->Entries < p_data->MaxEntries) {    /* LIFO only if we have room                            */
                     if (p_data->OutIx == 0u) {
                         p_data->OutIx = (OS_MSG_QTY)(p_data->MaxEntries - 1u);
                     } else {
                         p_data->OutIx--;
                     }
                     p_dest = &((CPU_INT08U *)p_data->StorageBaseAddr)[p_data->OutIx * p_data->ItemSize];
                     for (i = 0u; i < p_data->ItemSize; i++) {
                        *p_dest = *p_src;
                         p_dest++;
                         p_src++;
                     }
                     p_data->Entries++;
                 } else {
                      *p_err = OS_ERR_Q_MAX;
                 }
                 break;

            case OS_OPT_POST_FIFO_OVER:                         /* Overwrite next element to remove                     */
                  p_dest = &((CPU_INT08U *)p_data->StorageBaseAddr)[p_data->OutIx * p_data->ItemSize];
                  for (i = 0u; i < p_data->ItemSize; i++) {
                     *p_dest = *p_src;
                      p_dest++;
                      p_src++;
                  }
                  if (p_data->Entries == 0u) {                  /* Add element if queue was empty                       */
                      if (p_data->InIx == (p_data->MaxEntries - 1u)) {
                          p_data->InIx = 0u;
                      } else {
                          p_data->InIx++;
                      }
                      p_data->Entries++;
                  }
                  break;

            case OS_OPT_POST_LIFO_OVER:                         /* Overwrite last element written                       */
            default:
                 if (p_data->Entries > 0u) {
                     if (p_data->InIx == 0u) {
                         p_data->InIx = (OS_MSG_QTY)(p_data->MaxEntries - 1u);
                     } else {
                         p_data->InIx--;
                     }
                 }
                 p_dest = &((CPU_INT08U *)p_data->StorageBaseAddr)[p_data->InIx * p_data->ItemSize];
                 for (i = 0u; i < p_data->ItemSize; i++) {
                    *p_dest = *p_src;
                     p_dest++;
                     p_src++;
                 }
                 if (p_data->InIx == (p_data->MaxEntries - 1u)) {
                     p_data->InIx = 0u;
                 } else {
                     p_data->InIx++;
                 }
                 if (p_data->Entries == 0u) {                   /* Add element if queue was empty                       */
                     p_data->Entries++;
                 }
                 break;
        }
        CPU_CRITICAL_EXIT();
        OS_TRACE_DATA_POST_EXIT(*p_err);
        return;
    }

    p_tcb = p_pend_list->HeadPtr;                               /* YES, copy the data to the HPT waiting                */
    OS_Post((OS_PEND_OBJ *)((void *)p_data),
             p_tcb,
             p_msg,
             item_size,
             0u);

    CPU_CRITICAL_EXIT();

    if ((opt & OS_OPT_POST_NO_SCHED) == 0u) {
        OSSched();                                              /* Run the scheduler                                    */
    }

   *p_err = OS_ERR_NONE;
    OS_TRACE_DATA_POST_EXIT(*p_err);
}


/*
************************************************************************************************************************
*                                      Wipe out the data storage area of a queue
*
* Description: These function is called to clear out the data queue list.
*
* Arguments  : p_data     is a pointer to the data queue to add/remove
*
* Returns    : none
*
* Note(s)    : This function is INTERNAL to Cs/OS3 and your application should not call it.
************************************************************************************************************************
*/

void  OS_DataStorageClr (OS_DATA  *p_data)
{
    OS_MSG_QTY   i;
    OS_MSG_SIZE  j;
    CPU_INT08U  *p_clr;


    p_clr = (CPU_INT08U *)p_data->StorageBaseAddr;
    for (i = 0; i < p_data->MaxEntries; i++) {
        for (j = 0; j < p_data->ItemSize; j++) {
           *p_clr = 0u;
            p_clr++;
        }
    }
    p_data->InIx    = 0u;
    p_data->OutIx   = 0u;
    p_data->Entries = 0u;
}


/*
************************************************************************************************************************
*                                      ADD/REMOVE DATA QUEUE TO/FROM DEBUG LIST
*
* Description: These functions are called by Cs/OS3 to add or remove a data queue to/from a data queue debug
*              list.
*
* Arguments  : p_data     is a pointer to the data queue to add/remove
*
* Returns    : none
*
* Note(s)    : These functions are INTERNAL to Cs/OS3 and your application should not call it.
************************************************************************************************************************
*/

#if (OS_CFG_DBG_EN > 0u)
void  OS_DataDbgListAdd (OS_DATA  *p_data)
{
    p_data->DbgNamePtr               = (CPU_CHAR *)((void *)" ");
    p_data->DbgPrevPtr               = (OS_DATA *)0;
    if (OSDataDbgListPtr == (OS_DATA *)0) {
        p_data->DbgNextPtr           = (OS_DATA *)0;
    } else {
        p_data->DbgNextPtr           =  OSDataDbgListPtr;
        OSDataDbgListPtr->DbgPrevPtr =  p_data;
    }
    OSDataDbgListPtr                 =  p_data;
}


void  OS_DataDbgListRemove (OS_DATA  *p_data)
{
    OS_DATA  *p_data_next;
    OS_DATA  *p_data_prev;


    p_data_prev = p_data->DbgPrevPtr;
    p_data_next = p_data->DbgNextPtr;

    if (p_data_prev == (OS_DATA *)0) {
        OSDataDbgListPtr = p_data_next;
        if (p_data_next != (OS_DATA *)0) {
            p_data_next->DbgPrevPtr = (OS_DATA *)0;
        }
        p_data->DbgNextPtr = (OS_DATA *)0;

    } else if (p_data_next == (OS_DATA *)0) {
        p_data_prev->DbgNextPtr = (OS_DATA *)0;
        p_data->DbgPrevPtr      = (OS_DATA *)0;

    } else {
        p_data_prev->DbgNextPtr =  p_data_next;
        p_data_next->DbgPrevPtr =  p_data_prev;
        p_data->DbgNextPtr      = (OS_DATA *)0;
        p_data->DbgPrevPtr      = (OS_DATA *)0;
    }
}
#endif
#endif
