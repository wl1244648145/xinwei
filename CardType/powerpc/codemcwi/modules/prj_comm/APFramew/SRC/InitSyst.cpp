#ifndef _INC_AFWINIT
#include "AfwInit.h"
#endif

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

#ifdef __NUCLEUS__

#include "nucleus.h"
#include <string.h>
#include <stdio.h> //for tesing
extern NU_MEMORY_POOL  System_Memory;
//#define SYSTEM_MEMORY_SIZE 6400000

extern "C" void InitBoard(void *first_available_memory);

#ifdef M_TGT_L2
extern "C" void PreAfwInit(void);
#endif

NU_TASK	*gp_InitTaskTCB;
bool AfwInitFinished = false;

static VOID AfwInitTask(UNSIGNED argc, VOID *pParam)
{
#ifdef M_TGT_L2
    PreAfwInit();
#endif
	AfwInit();
    AfwInitFinished = true;
    while(1)
    {
       NU_Sleep(10000);
    }
}

#define INIT_TASK_STACK_SIZE  1000
VOID *initTaskStackPtr = NULL;

VOID Application_Initialize(VOID *first_available_memory)
{
	InitBoard(first_available_memory);
    initTaskStackPtr = new UINT8[INIT_TASK_STACK_SIZE];
#ifndef COMIP
    gp_InitTaskTCB = new NU_TASK;
    // allocate mem for task

	// create task
    ::memset(gp_InitTaskTCB, 0, sizeof(UNSIGNED)*NU_TASK_SIZE);
	if(NU_SUCCESS!=NU_Create_Task(gp_InitTaskTCB, "AfwInit", AfwInitTask, 0, NULL,
                                  initTaskStackPtr, INIT_TASK_STACK_SIZE, (OPTION)5,
                                  1, NU_PREEMPT, NU_START))
    {
//        printf("NU_Create_Task failed.\n");
    }
#endif
#if 0
    char name[20];
    DATA_ELEMENT task_status;
    UNSIGNED scheduled_count;
    OPTION priority;
    OPTION preempt;
    UNSIGNED time_slice;
    VOID* stack_base;
    UNSIGNED stack_size;
    UNSIGNED minimum_stack;
    for(;;)
    {
        if (NU_SUCCESS==NU_Task_Information(&g_InitTaskTCB, name, &task_status,
                                           &scheduled_count, &priority, &preempt,
                                           &time_slice, &stack_base, &stack_size,
                                           &minimum_stack) &&
            NU_FINISHED==task_status
           )
        {
            break;
        }
    }

	//NU_Deallocate_Memory(stackptr);
	delete initTaskStackPtr;
#endif
}

#else

#ifdef __WIN32_SIM__
#include <windows.h>
#endif

#ifdef M_TGT_L2
extern "C" void PreAfwInit();
#endif

int main(void)
{
#ifdef __WIN32_SIM__
    HANDLE hbufNew = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (hbufNew!=INVALID_HANDLE_VALUE)
    {
        COORD coord;
        coord.X = 120;
        coord.Y = 3000;
        ::SetConsoleScreenBufferSize(hbufNew, coord);
    }
#endif

#ifdef M_TGT_L2
    PreAfwInit();
#endif

    int iRet = !AfwInit();

#ifdef __WIN32_SIM__
    ::Sleep(WAIT_FOREVER);
#endif

    return iRet;
}

#endif
