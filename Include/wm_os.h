#ifndef __WM_OS_H__
#define __WM_OS_H__

typedef INT8U OS_PRIO;
typedef void (*OS_TMR_CALLBACK)(void *ptmr, void *parg);

#define OS_LOWEST_PRIO				63u 
#define OS_EVENT_TBL_SIZE			((OS_LOWEST_PRIO) / 8u + 1u) 

typedef struct os_event 
{
	INT8U OSEventType;						/* Type of event control block (see OS_EVENT_TYPE_xxxx) */
	void *OSEventPtr;						/* Pointer to message or queue structure */
	INT16U OSEventCnt;						/* Semaphore Count (not used if other EVENT type) */
	OS_PRIO OSEventGrp;						/* Group corresponding to tasks waiting for event to occur */
	OS_PRIO OSEventTbl[OS_EVENT_TBL_SIZE];	/* List of tasks waiting for event to occur */
} OS_EVENT;

typedef struct os_tmr 
{
	INT8U OSTmrType;						/* Should be set to OS_TMR_TYPE */
	OS_TMR_CALLBACK OSTmrCallback;			/* Function to call when timer expires */
	void *OSTmrCallbackArg;					/* Argument to pass to function when timer expires */
	void *OSTmrNext;						/* Double link list pointers */
	void *OSTmrPrev;
	INT32U OSTmrMatch;						/* Timer expires when OSTmrTime == OSTmrMatch */
	INT32U OSTmrDly;						/* Delay time before periodic update starts */
	INT32U OSTmrPeriod;						/* Period to repeat timer */
	INT8U *OSTmrName;						/* Name to give the timer */
	INT8U OSTmrOpt;							/* Options (see OS_TMR_OPT_xxx) */
	INT8U OSTmrState;						/* Indicates the state of the timer: */
} OS_TMR;

/**********************************************************************************************************
* Description: This function is used to have uC/OS-II manage the execution of a task.  Tasks can either
*              be created prior to the start of multitasking or by a running task.  A task cannot be
*              created by an ISR.
*
* Arguments  : task     is a pointer to the task's code
*
*              p_arg    is a pointer to an optional data area which can be used to pass parameters to
*                       the task when the task first executes.  Where the task is concerned it thinks
*                       it was invoked and passed the argument 'p_arg' as follows:
*
*                           void Task (void *p_arg)
*                           {
*                               for (;;) {
*                                   Task code;
*                               }
*                           }
*
*              ptos     is a pointer to the task's top of stack.  If the configuration constant
*                       OS_STK_GROWTH is set to 1, the stack is assumed to grow downward (i.e. from high
*                       memory to low memory).  'pstk' will thus point to the highest (valid) memory
*                       location of the stack.  If OS_STK_GROWTH is set to 0, 'pstk' will point to the
*                       lowest memory location of the stack and the stack will grow with increasing
*                       memory locations.
*
*              prio     is the task's priority.  A unique priority MUST be assigned to each task and the
*                       lower the number, the higher the priority.
*
* Returns    : OS_ERR_NONE             if the function was successful.
*              OS_PRIO_EXIT            if the task priority already exist
*                                      (each task MUST have a unique priority).
*              OS_ERR_PRIO_INVALID     if the priority you specify is higher that the maximum allowed
*                                      (i.e. >= OS_LOWEST_PRIO)
*              OS_ERR_TASK_CREATE_ISR  if you tried to create a task from an ISR.
**********************************************************************************************************/
INT8U OSTaskCreate(void (*task)(void *p_arg), void *p_arg, OS_STK *ptos,INT8U prio);

/**********************************************************************************************************
* Description: This function is called to suspend a task.  The task can be the calling task if the
*              priority passed to OSTaskSuspend() is the priority of the calling task or OS_PRIO_SELF.
*
* Arguments  : prio     is the priority of the task to suspend.  If you specify OS_PRIO_SELF, the
*                       calling task will suspend itself and rescheduling will occur.
*
* Returns    : OS_ERR_NONE               if the requested task is suspended
*              OS_ERR_TASK_SUSPEND_IDLE  if you attempted to suspend the idle task which is not allowed.
*              OS_ERR_PRIO_INVALID       if the priority you specify is higher that the maximum allowed
*                                        (i.e. >= OS_LOWEST_PRIO) or, you have not specified OS_PRIO_SELF.
*              OS_ERR_TASK_SUSPEND_PRIO  if the task to suspend does not exist
*              OS_ERR_TASK_NOT_EXITS     if the task is assigned to a Mutex PIP
*
* Note       : You should use this function with great care.  If you suspend a task that is waiting for
*              an event (i.e. a message, a semaphore, a queue ...) you will prevent this task from
*              running when the event arrives.
**********************************************************************************************************/
INT8U OSTaskSuspend(INT8U prio);

/**********************************************************************************************************
* Description: This function is called to resume a previously suspended task.  This is the only call that
*              will remove an explicit task suspension.
*
* Arguments  : prio     is the priority of the task to resume.
*
* Returns    : OS_ERR_NONE                if the requested task is resumed
*              OS_ERR_PRIO_INVALID        if the priority you specify is higher that the maximum allowed
*                                         (i.e. >= OS_LOWEST_PRIO)
*              OS_ERR_TASK_RESUME_PRIO    if the task to resume does not exist
*              OS_ERR_TASK_NOT_EXIST      if the task is assigned to a Mutex PIP
*              OS_ERR_TASK_NOT_SUSPENDED  if the task to resume has not been suspended
**********************************************************************************************************/
INT8U OSTaskResume(INT8U prio);

/**********************************************************************************************************
* Description: This function creates a semaphore.
*
* Arguments  : cnt           is the initial value for the semaphore.  If the value is 0, no resource is
*                            available (or no event has occurred).  You initialize the semaphore to a
*                            non-zero value to specify how many resources are available (e.g. if you have
*                            10 resources, you would initialize the semaphore to 10).
*
* Returns    : != (void *)0  is a pointer to the event control block (OS_EVENT) associated with the
*                            created semaphore
*              == (void *)0  if no event control blocks were available
**********************************************************************************************************/
OS_EVENT *OSSemCreate(INT16U cnt);

/**********************************************************************************************************
* Description: This function deletes a semaphore and readies all tasks pending on the semaphore.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            semaphore.
*
*              opt           determines delete options as follows:
*                            opt == OS_DEL_NO_PEND   Delete semaphore ONLY if no task pending
*                            opt == OS_DEL_ALWAYS    Deletes the semaphore even if tasks are waiting.
*                                                    In this case, all the tasks pending will be readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            OS_ERR_NONE             The call was successful and the semaphore was deleted
*                            OS_ERR_DEL_ISR          If you attempted to delete the semaphore from an ISR
*                            OS_ERR_INVALID_OPT      An invalid option was specified
*                            OS_ERR_TASK_WAITING     One or more tasks were waiting on the semaphore
*                            OS_ERR_EVENT_TYPE       If you didn't pass a pointer to a semaphore
*                            OS_ERR_PEVENT_NULL      If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (OS_EVENT *)0 if the semaphore was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the semaphore MUST check the return code of OSSemPend().
*              2) OSSemAccept() callers will not know that the intended semaphore has been deleted unless
*                 they check 'pevent' to see that it's a NULL pointer.
*              3) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the semaphore.
*              4) Because ALL tasks pending on the semaphore will be readied, you MUST be careful in
*                 applications where the semaphore is used for mutual exclusion because the resource(s)
*                 will no longer be guarded by the semaphore.
**********************************************************************************************************/
OS_EVENT *OSSemDel(OS_EVENT *pevent, INT8U opt, INT8U *perr);

/**********************************************************************************************************
* Description: This function waits for a semaphore.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            semaphore.
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for the resource up to the amount of time specified by this argument.
*                            If you specify 0, however, your task will wait forever at the specified
*                            semaphore or, until the resource becomes available (or the event occurs).
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         The call was successful and your task owns the resource
*                                                or, the event you are waiting for occurred.
*                            OS_ERR_TIMEOUT      The semaphore was not received within the specified
*                                                'timeout'.
*                            OS_ERR_PEND_ABORT   The wait on the semaphore was aborted.
*                            OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a semaphore.
*                            OS_ERR_PEND_ISR     If you called this function from an ISR and the result
*                                                would lead to a suspension.
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
*                            OS_ERR_PEND_LOCKED  If you called this function when the scheduler is locked
*
* Returns    : none
**********************************************************************************************************/
void OSSemPend(OS_EVENT *pevent, INT32U timeout, INT8U *perr);

/**********************************************************************************************************
* Description: This function signals a semaphore
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            semaphore.
*
* Returns    : OS_ERR_NONE         The call was successful and the semaphore was signaled.
*              OS_ERR_SEM_OVF      If the semaphore count exceeded its limit.  In other words, you have
*                                  signalled the semaphore more often than you waited on it with either
*                                  OSSemAccept() or OSSemPend().
*              OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a semaphore
*              OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
**********************************************************************************************************/
INT8U OSSemPost(OS_EVENT *pevent);

/**********************************************************************************************************
* Description: This function sets the semaphore count to the value specified as an argument.  Typically,
*              this value would be 0.
*
*              You would typically use this function when a semaphore is used as a signaling mechanism
*              and, you want to reset the count value.
*
* Arguments  : pevent     is a pointer to the event control block
*
*              cnt        is the new value for the semaphore count.  You would pass 0 to reset the
*                         semaphore count.
*
*              perr       is a pointer to an error code returned by the function as follows:
*
*                            OS_ERR_NONE          The call was successful and the semaphore value was set.
*                            OS_ERR_EVENT_TYPE    If you didn't pass a pointer to a semaphore.
*                            OS_ERR_PEVENT_NULL   If 'pevent' is a NULL pointer.
*                            OS_ERR_TASK_WAITING  If tasks are waiting on the semaphore.
**********************************************************************************************************/
void OSSemSet (OS_EVENT *pevent, INT16U cnt, INT8U *perr);

/**********************************************************************************************************
* Description: This function creates a message queue if free event control blocks are available.
*
* Arguments  : start         is a pointer to the base address of the message queue storage area.  The
*                            storage area MUST be declared as an array of pointers to 'void' as follows
*
*                            void *MessageStorage[size]
*
*              size          is the number of elements in the storage area
*
* Returns    : != (OS_EVENT *)0  is a pointer to the event control clock (OS_EVENT) associated with the
*                                created queue
*              == (OS_EVENT *)0  if no event control blocks were available or an error was detected
**********************************************************************************************************/
OS_EVENT *OSQCreate(void **start, INT16U size);

/**********************************************************************************************************
* Description: This function deletes a message queue and readies all tasks pending on the queue.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            queue.
*
*              opt           determines delete options as follows:
*                            opt == OS_DEL_NO_PEND   Delete the queue ONLY if no task pending
*                            opt == OS_DEL_ALWAYS    Deletes the queue even if tasks are waiting.
*                                                    In this case, all the tasks pending will be readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            OS_ERR_NONE             The call was successful and the queue was deleted
*                            OS_ERR_DEL_ISR          If you tried to delete the queue from an ISR
*                            OS_ERR_INVALID_OPT      An invalid option was specified
*                            OS_ERR_TASK_WAITING     One or more tasks were waiting on the queue
*                            OS_ERR_EVENT_TYPE       If you didn't pass a pointer to a queue
*                            OS_ERR_PEVENT_NULL      If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (OS_EVENT *)0 if the queue was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the queue MUST check the return code of OSQPend().
*              2) OSQAccept() callers will not know that the intended queue has been deleted unless
*                 they check 'pevent' to see that it's a NULL pointer.
*              3) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the queue.
*              4) Because ALL tasks pending on the queue will be readied, you MUST be careful in
*                 applications where the queue is used for mutual exclusion because the resource(s)
*                 will no longer be guarded by the queue.
*              5) If the storage for the message queue was allocated dynamically (i.e. using a malloc()
*                 type call) then your application MUST release the memory storage by call the counterpart
*                 call of the dynamic allocation scheme used.  If the queue storage was created statically
*                 then, the storage can be reused.
**********************************************************************************************************/
OS_EVENT *OSQDel (OS_EVENT *pevent, INT8U opt, INT8U *perr);

/**********************************************************************************************************
* Description: This function sends a message to a queue
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired queue
*
*              pmsg          is a pointer to the message to send.
*
* Returns    : OS_ERR_NONE           The call was successful and the message was sent
*              OS_ERR_Q_FULL         If the queue cannot accept any more messages because it is full.
*              OS_ERR_EVENT_TYPE     If you didn't pass a pointer to a queue.
*              OS_ERR_PEVENT_NULL    If 'pevent' is a NULL pointer
*
* Note(s)    : As of V2.60, this function allows you to send NULL pointer messages.
**********************************************************************************************************/
INT8U OSQPost(OS_EVENT *pevent, void *pmsg);

/**********************************************************************************************************
* Description: This function waits for a message to be sent to a queue
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired queue
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for a message to arrive at the queue up to the amount of time
*                            specified by this argument.  If you specify 0, however, your task will wait
*                            forever at the specified queue or, until a message arrives.
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         The call was successful and your task received a
*                                                message.
*                            OS_ERR_TIMEOUT      A message was not received within the specified 'timeout'.
*                            OS_ERR_PEND_ABORT   The wait on the queue was aborted.
*                            OS_ERR_EVENT_TYPE   You didn't pass a pointer to a queue
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer
*                            OS_ERR_PEND_ISR     If you called this function from an ISR and the result
*                                                would lead to a suspension.
*                            OS_ERR_PEND_LOCKED  If you called this function with the scheduler is locked
*
* Returns    : != (void *)0  is a pointer to the message received
*              == (void *)0  if you received a NULL pointer message or,
*                            if no message was received or,
*                            if 'pevent' is a NULL pointer or,
*                            if you didn't pass a pointer to a queue.
*
* Note(s)    : As of V2.60, this function allows you to receive NULL pointer messages.
**********************************************************************************************************/
void *OSQPend(OS_EVENT *pevent, INT32U timeout, INT8U *perr);

/**********************************************************************************************************
* Description: This function creates a message mailbox if free event control blocks are available.
*
* Arguments  : pmsg          is a pointer to a message that you wish to deposit in the mailbox.  If
*                            you set this value to the NULL pointer (i.e. (void *)0) then the mailbox
*                            will be considered empty.
*
* Returns    : != (OS_EVENT *)0  is a pointer to the event control clock (OS_EVENT) associated with the
*                                created mailbox
*              == (OS_EVENT *)0  if no event control blocks were available
**********************************************************************************************************/
OS_EVENT *OSMboxCreate(void *pmsg);

/**********************************************************************************************************
* Description: This function sends a message to a mailbox
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              pmsg          is a pointer to the message to send.  You MUST NOT send a NULL pointer.
*
* Returns    : OS_ERR_NONE          The call was successful and the message was sent
*              OS_ERR_MBOX_FULL     If the mailbox already contains a message.  You can can only send one
*                                   message at a time and thus, the message MUST be consumed before you
*                                   are allowed to send another one.
*              OS_ERR_EVENT_TYPE    If you are attempting to post to a non mailbox.
*              OS_ERR_PEVENT_NULL   If 'pevent' is a NULL pointer
*              OS_ERR_POST_NULL_PTR If you are attempting to post a NULL pointer
*
* Note(s)    : 1) HPT means Highest Priority Task
**********************************************************************************************************/
INT8U OSMboxPost(OS_EVENT *pevent, void *pmsg);

/**********************************************************************************************************
* Description: This function waits for a message to be sent to a mailbox
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for a message to arrive at the mailbox up to the amount of time
*                            specified by this argument.  If you specify 0, however, your task will wait
*                            forever at the specified mailbox or, until a message arrives.
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            OS_ERR_NONE         The call was successful and your task received a
*                                                message.
*                            OS_ERR_TIMEOUT      A message was not received within the specified 'timeout'.
*                            OS_ERR_PEND_ABORT   The wait on the mailbox was aborted.
*                            OS_ERR_EVENT_TYPE   Invalid event type
*                            OS_ERR_PEND_ISR     If you called this function from an ISR and the result
*                                                would lead to a suspension.
*                            OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer
*                            OS_ERR_PEND_LOCKED  If you called this function when the scheduler is locked
*
* Returns    : != (void *)0  is a pointer to the message received
*              == (void *)0  if no message was received or,
*                            if 'pevent' is a NULL pointer or,
*                            if you didn't pass the proper pointer to the event control block.
**********************************************************************************************************/
void *OSMboxPend(OS_EVENT *pevent, INT32U timeout, INT8U *perr);

/*************************************************************************************************************************
* Description: This function is called by your application code to create a timer.
*
* Arguments  : dly           Initial delay.
*                            If the timer is configured for ONE-SHOT mode, this is the timeout used
*                            If the timer is configured for PERIODIC mode, this is the first timeout to wait for
*                               before the timer starts entering periodic mode
*
*              period        The 'period' being repeated for the timer.
*                               If you specified 'OS_TMR_OPT_PERIODIC' as an option, when the timer expires, it will
*                               automatically restart with the same period.
*
*              opt           Specifies either:
*                               OS_TMR_OPT_ONE_SHOT       The timer counts down only once
*                               OS_TMR_OPT_PERIODIC       The timer counts down and then reloads itself
*
*              callback      Is a pointer to a callback function that will be called when the timer expires.  The
*                               callback function must be declared as follows:
*
*                               void MyCallback (OS_TMR *ptmr, void *p_arg);
*
*              callback_arg  Is an argument (a pointer) that is passed to the callback function when it is called.
*
*              pname         Is a pointer to an ASCII string that is used to name the timer.  Names are useful for
*                               debugging.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID_DLY     you specified an invalid delay
*                               OS_ERR_TMR_INVALID_PERIOD  you specified an invalid period
*                               OS_ERR_TMR_INVALID_OPT     you specified an invalid option
*                               OS_ERR_TMR_ISR             if the call was made from an ISR
*                               OS_ERR_TMR_NON_AVAIL       if there are no free timers from the timer pool
*
* Returns    : A pointer to an OS_TMR data structure.
*              This is the 'handle' that your application will use to reference the timer created.
*************************************************************************************************************************/
OS_TMR *OSTmrCreate(INT32U dly, INT32U period, INT8U opt, OS_TMR_CALLBACK callback, void *callback_arg, INT8U *pname, INT8U *perr);

/*************************************************************************************************************************
* Description: This function is called by your application code to start a timer.
*
* Arguments  : ptmr          Is a pointer to an OS_TMR
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID
*                               OS_ERR_TMR_INVALID_TYPE    'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR             if the call was made from an ISR
*                               OS_ERR_TMR_INACTIVE        if the timer was not created
*                               OS_ERR_TMR_INVALID_STATE   the timer is in an invalid state
*
* Returns    : OS_TRUE    if the timer was started
*              OS_FALSE   if an error was detected
*************************************************************************************************************************/
BOOLEAN OSTmrStart(OS_TMR *ptmr, INT8U *perr);

/*************************************************************************************************************************
* Description: This function is called by your application code to stop a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to stop.
*
*              opt           Allows you to specify an option to this functions which can be:
*
*                               OS_TMR_OPT_NONE          Do nothing special but stop the timer
*                               OS_TMR_OPT_CALLBACK      Execute the callback function, pass it the callback argument
*                                                        specified when the timer was created.
*                               OS_TMR_OPT_CALLBACK_ARG  Execute the callback function, pass it the callback argument
*                                                        specified in THIS function call
*
*              callback_arg  Is a pointer to a 'new' callback argument that can be passed to the callback function
*                               instead of the timer's callback argument.  In other words, use 'callback_arg' passed in
*                               THIS function INSTEAD of ptmr->OSTmrCallbackArg
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               OS_ERR_NONE
*                               OS_ERR_TMR_INVALID         'ptmr' is a NULL pointer
*                               OS_ERR_TMR_INVALID_TYPE    'ptmr'  is not pointing to an OS_TMR
*                               OS_ERR_TMR_ISR             if the function was called from an ISR
*                               OS_ERR_TMR_INACTIVE        if the timer was not created
*                               OS_ERR_TMR_INVALID_OPT     if you specified an invalid option for 'opt'
*                               OS_ERR_TMR_STOPPED         if the timer was already stopped
*                               OS_ERR_TMR_INVALID_STATE   the timer is in an invalid state
*                               OS_ERR_TMR_NO_CALLBACK     if the timer does not have a callback function defined
*
* Returns    : OS_TRUE       If we stopped the timer (if the timer is already stopped, we also return OS_TRUE)
*              OS_FALSE      If not
*************************************************************************************************************************/
BOOLEAN OSTmrStop(OS_TMR *ptmr, INT8U opt, void *callback_arg, INT8U *perr);

/**********************************************************************************************************
* Description: This function is used by your application to obtain the current value of the 32-bit
*              counter which keeps track of the number of clock ticks.
*
* Arguments  : none
*
* Returns    : The current value of OSTime
**********************************************************************************************************/
INT32U OSTimeGet (void);

/**********************************************************************************************************
* Description: This function is called to delay execution of the currently running task until some time
*              expires.  This call allows you to specify the delay time in HOURS, MINUTES, SECONDS and
*              MILLISECONDS instead of ticks.
*
* Arguments  : hours     specifies the number of hours that the task will be delayed (max. is 255)
*              minutes   specifies the number of minutes (max. 59)
*              seconds   specifies the number of seconds (max. 59)
*              ms        specifies the number of milliseconds (max. 999)
*
* Returns    : OS_ERR_NONE
*              OS_ERR_TIME_INVALID_MINUTES
*              OS_ERR_TIME_INVALID_SECONDS
*              OS_ERR_TIME_INVALID_MS
*              OS_ERR_TIME_ZERO_DLY
*              OS_ERR_TIME_DLY_ISR
*
* Note(s)    : because each tick of the system is 10ms, in current implementation, the ms will be rounded up to the nearest 10;
		     for example, 45ms will be considerred as 50ms;
**********************************************************************************************************/

INT8U  OSTimeDlyHMSM (INT8U   hours,
                      INT8U   minutes,
                      INT8U   seconds,
                      INT16U  ms);

/**********************************************************************************************************
* Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*              into the CPU's status register.
*
* Arguments  : none
*
* Returns    : cpu_sr
***********************************************************************************************************/
OS_CPU_SR OSCPUSaveSR(void);

/**********************************************************************************************************
* Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
*              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*              into the CPU's status register.
*
* Arguments  : cpu_sr
*
* Returns    : none
***********************************************************************************************************/
void OSCPURestoreSR(OS_CPU_SR cpu_sr);

/**********************************************************************************************************
* Description: This function is used to start the multitasking process which lets uC/OS-II manages the
*              task that you have created.  Before you can call OSStart(), you MUST have called OSInit()
*              and you MUST have created at least one task.
*
* Arguments  : none
*
* Returns    : none
*
* Note       : OSStartHighRdy() MUST:
*                 a) Call OSTaskSwHook() then,
*                 b) Set OSRunning to OS_TRUE.
*                 c) Load the context of the task pointed to by OSTCBHighRdy.
*                 d_ Execute the task.
**********************************************************************************************************/
void OSStart(void);

#endif
