/*****************************************************************************
 * kernelLib.c - defines the functions and data structures needed
 *               to initialize a VxWorks virtual machine in a POSIX Threads
 *               environment.
 *  
 * Copyright (C) 2000  Monta Vista Software Inc.
 *
 * Author : Gary S. Robertson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 ****************************************************************************/

#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include "vxwk2pthread.h"
#include "vxwkdefs.h"

#undef DIAG_PRINTFS

/*
**  user_sysinit is a user-defined function.  It contains all initialization
**               calls to create any tasks and other objects reqired for
**               startup of the user's RTOS system environment.  It is called
**               from (and runs in) the system root task context.
*/
extern void user_sysinit( void );

/*
**  user_syskill is a user-defined function.  It is called from the main
**               process context of the VxWorks virtual machine.
**               At its simplest it is an unconditional infinite loop.
**               It may optionally wait for some condition, shut down the
**               user's RTOS system environment, clean up the resources used
**               by the various RTOS objects, and then return to the main
**               process of the VxWorks virtual machine.
**               The VxWorks virtual machine terminates upon its return.
*/
extern void user_syskill( void );

/*
**  process_timer_list is a system function used to service watchdog timers
**                     when a system clock tick expires.  It is called from
**                     the system exception task once per clock tick.
*/
extern void
   process_timer_list( void );

extern void
   taskLock( void );
extern void
   taskUnlock( void );
extern STATUS
   taskDelay( int interval );
extern STATUS
    taskInit( vxwk2pthread_cb_t *tcb, char *name, int pri, int opts,
              char *pstack, int stksize,
              int (*funcptr)( int,int,int,int,int,int,int,int,int,int ),
              int arg1, int arg2, int arg3, int arg4, int arg5,
              int arg6, int arg7, int arg8, int arg9, int arg10 );
extern STATUS
    taskActivate( int tid );

/*****************************************************************************
**  VxWorks-to-pthread Global Data Structures
*****************************************************************************/
/*
**  Task control blocks for the VxWorks system tasks.
*/


static vxwk2pthread_cb_t
    excp_tcb;
/*
**  task_list is a linked list of pthread task control blocks.
**            It is used to perform en-masse operations on all VxWorks pthread
**            tasks at once.
*/
extern vxwk2pthread_cb_t *
    task_list;

/*
**  task_list_lock is a mutex used to serialize access to the task list
*/
extern pthread_mutex_t
    task_list_lock;

/*
**  round_robin_enabled is a system-wide mode flag indicating whether the
**                      VxWorks scheduler is to use FIFO or Round Robin
**                      scheduling.
*/
static unsigned char
    round_robin_enabled = 0;

/*****************************************************************************
** round-robin control 
*****************************************************************************/
void disableRoundRobin( void )
{
    round_robin_enabled = 0;
}

void enableRoundRobin( void )
{
    round_robin_enabled = 1;
}

BOOL
   roundRobinIsEnabled( void )
{
    return( (BOOL)round_robin_enabled );
}

/*****************************************************************************
** kernelTimeSlice - turns Round-Robin Timeslicing on or off in the scheduler
*****************************************************************************/
STATUS
    kernelTimeSlice( int ticks_per_quantum )
{
    vxwk2pthread_cb_t *tcb;
    int sched_policy;
    struct sched_param sch_param;
	
    taskLock();

    /*
    **  Linux doesn't allow the round-robin quantum to be changed, so
    **  we only use the ticks_per_quantum as an on/off value for
    **  round-robin scheduling.
    */
    if ( ticks_per_quantum == 0 )
    {
        /*
        **  Ensure Round-Robin Timeslicing is OFF for all tasks, both
        **  existing and yet to be created.
        */
        round_robin_enabled = 0;
        sched_policy = SCHED_FIFO;
    }
    else
    {
        /*
        **  Ensure Round-Robin Timeslicing is ON for all tasks, both
        **  existing and yet to be created.
        */
        round_robin_enabled = 1;
        sched_policy = SCHED_RR;
    }

    if ( task_list != (vxwk2pthread_cb_t *)NULL )
    {
        /*
        **  Change the scheduling policy for all tasks in the task list.
        */
        for ( tcb = task_list; tcb != (vxwk2pthread_cb_t *)NULL;
              tcb = tcb->nxt_task )
        {
            /*
            **  First set the new scheduling policy attribute.  Since the
            **  max priorities are identical under Linux for both real time
            **  scheduling policies, we know we don't have to change priority.
            */
            pthread_attr_setschedpolicy( &(tcb->attr), sched_policy );

            /*
            **  Activate the new scheduling policy
            */

			/*modify by huangjl*/
			pthread_attr_getschedparam(&(tcb->attr),&sch_param);
            pthread_setschedparam( tcb->pthrid, sched_policy, (struct sched_param *)(&sch_param) );
        }
    }

    taskUnlock();

    return( OK );
}

/*****************************************************************************
**  system exception task
**
**  In the VxWorks-to-pthreads environment, the exception task serves only to
**  handle watchdog timer functions and to allow self-restarting of other
**  VxWorks tasks.
*****************************************************************************/
int exception_task( int dummy0, int dummy1, int dummy2, int dummy3,
                    int dummy4, int dummy5, int dummy6, int dummy7,
                    int dummy8, int dummy9 )
{

    while ( 1 )
    {
        /*
        **  Process system watchdog timers (if any are defined).
        **  NOTE that since ALL timers must be handled during a single
        **  10 msec system clock tick, timers should be used sparingly.
        **  In addition, the timeout functions called by watchdog timers
        **  should be "short and sweet".
        */
        process_timer_list();

        /*
        **  Delay for one timer tick.  Since this is the highest-priority
        **  task in the VxWorks virtual machine (except for the root task,
        **  which stays blocked almost all the time), any processing done
        **  in this task can impose a heavy load on the remaining tasks.
        **  For this reason, this task and all watchdog timeout functions
        **  should be kept as brief as possible.
        */
        taskDelay( 1 );
    }

    return( 0 );
}

/*****************************************************************************
**  system root task
**
**  In the VxWorks-to-pthreads environment, the root task serves only to
**  start the system exception task and to provide a context in which the
**  user_sysinit function may call any supported VxWorks function, even
**  though that function might block.
*****************************************************************************/
int root_task( int dummy0, int dummy1, int dummy2, int dummy3, int dummy4,
               int dummy5, int dummy6, int dummy7, int dummy8, int dummy9 )
{
    int max_priority;

    /*
    **  Set up a VxWorks task and TCB for the system exception task.
    */
    printf( "\r\nStarting System Exception Task" );
    taskInit( &excp_tcb, "tExcTask", 0, 0, 0, 0, exception_task,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    /*
    **  Get the maximum permissible priority level for a VxWorks pthread
    **  and make that the pthreads priority for the exception task.
    */
    max_priority = sched_get_priority_max( SCHED_FIFO );
    (excp_tcb.prv_priority).sched_priority = (max_priority - 1);
    pthread_attr_setschedparam( &(excp_tcb.attr), &(excp_tcb.prv_priority) );

    taskActivate( excp_tcb.taskid );

 //   user_sysinit();//delete by huangjl

    while ( 1 )
        taskDelay( 500 );

    return( 0 );
}

/*****************************************************************************
**  system initialization pthread
*****************************************************************************/
void *init_system( void *dummy )
{
    return( (void *)NULL );
}


  pthread_t 	getLogpid()
{
	int logid =  1;
	vxwk2pthread_cb_t *tcblog = taskTcb(logid);
	return tcblog->pthrid;

}

#if 0
/*****************************************************************************
**  vxwk2pthread main program
**
**  This function serves as the entry point to the pthreads VxWorks emulation
**  environment.  It serves as the parent process to all VxWorks task pthreads.
**  This process creates an initialization thread and sets the priority of
**  that thread to the highest allowable value.  This allows the initialization
**  thread to complete its work without being preempted by any of the task
**  threads it creates.
*****************************************************************************/
int main( int argc, char **argv )
{
    int max_priority;

    /*
    **  Lock all memory pages associated with this process to prevent delays
    **  due to process (or thread) memory being swapped out to disk and back.
    */
    mlockall( (MCL_CURRENT | MCL_FUTURE) );

    /*
    **  Set up a VxWorks task and TCB for the system root task.
    */
    printf( "\r\nStarting System Root Task" );
    taskInit( &root_tcb, "tUsrRoot", 0, 0, 0, 0, root_task,
              0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );

    /*
    **  Get the maximum permissible priority level for the current OS
    **  and make that the pthreads priority for the root task.
    */
    max_priority = sched_get_priority_max( SCHED_FIFO );
    (root_tcb.prv_priority).sched_priority = max_priority;
    pthread_attr_setschedparam( &(root_tcb.attr), &(root_tcb.prv_priority) );

    taskActivate( root_tcb.taskid );

    errno = 0;

    /*
    **  Wait for the return.
    */    
    user_syskill();

    exit( 0 );
}
#endif
