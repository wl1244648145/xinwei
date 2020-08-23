/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: MemOpReload.cpp
 *
 * DESCRIPTION:  functions that reload the global new and delete operators
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   04/19/2006   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifdef __WIN32_SIM__ 
#include <stdlib.h>
#include <malloc.h>
#else
#include <taskLib.h>
#endif

#include <stdio.h>
#include <loglib.h>

#ifndef __WIN32_SIM__ 
#define RELOAD_MEM_OPERATOR
#endif

#define MEM_DEBUG
#ifndef WBBU_CODE
#ifdef MEM_DEBUG
#define MEM_LEAK_DEBUG	// for L2TxInL3

extern "C"
void dec_mem_tag_count(int tag);

#endif
#endif
	
#ifndef WBBU_CODE
#undef RELOAD_MEM_OPERATOR
#endif //LARGE_BTS

#ifdef RELOAD_MEM_OPERATOR
int g_new_cnt=0;
int g_del_cnt=0;
int g_del_invalid_addr_cnt = 0;
int g_del_overwrite_cnt = 0;

#ifndef COMIP
extern "C" char * sysMemTop (void);
#include <taskLib.h>

#ifdef MEM_DEBUG
#include "MemOpReload.h"
#ifndef WBBU_CODE
extern "C" usrMemPart g_mem_by_task[M_MAX_TASK_NUM];
#else
usrMemPart g_mem_by_task[M_MAX_TASK_NUM];
#endif

void updateMemoryUsage(int taskID, int size, int flag);
#endif
#ifndef WBBU_CODE 
#if (M_TARGET==M_TGT_L3)
extern unsigned int task_suspend_id[40];
extern unsigned int task_suspend_time[40] ;
extern unsigned int task_id_index ;
#endif
#endif

void * glob_new(size_t sz)
{
    char *rcPtr = 0;

    g_new_cnt++;
	sz= (sz+3)&0xfffffffc;   // to make the tail pointer 4 byte aligned
    
#ifdef MEM_DEBUG
    #ifndef __WIN32_SIM__
    //mem blk format: {realAddress(4),tag(4),sz(4), zero(4) data(sz),realAddress(4),tail(4)}
    int *headPtr, *tailPtr;

    headPtr=(int*)malloc(6*sizeof(int)+sz);
    if (headPtr==NULL)
        return NULL;
    headPtr[0] = (int)headPtr;
    headPtr[1] = -1;	//taskIdSelf();	// for tag
    headPtr[2] = sz;
    headPtr[3] = taskIdSelf();//0x12345678;

    rcPtr = (char*)headPtr;
    rcPtr += 4*sizeof(int);
    tailPtr = (int*)(rcPtr + sz );
    *tailPtr = (int)headPtr;
    tailPtr ++;
    *tailPtr = 0x5555aaaa;
#ifndef COMIP
    #ifdef M_TGT_L3
    updateMemoryUsage(taskIdSelf(), sz, M_OP_NEW);
    #endif
    #endif
    #endif
#else
    //normal
    rcPtr = (char*)malloc(sz);
#endif//MEM_DEBUG

#ifndef COMIP
    if (rcPtr >= sysMemTop())
    {
        logMsg("Get an invalid memory at addr 0x%x \n", (UINT32)rcPtr, 0, 0, 0, 0, 0);
        return NULL;
    }
#endif
	return (void *)rcPtr;
}


void glob_delete(void *p)
{    
    unsigned char flag;
    int i;
     unsigned int taskid;
#ifndef COMIP
    if ((UINT32) p >= (UINT32)sysMemTop())
    {
        logMsg("Free an invalid memory at addr 0x%x \n", (UINT32)p, 0, 0, 0, 0, 0);
#ifndef WBBU_CODE 
        //      taskSuspend(0);
   #if (M_TARGET==M_TGT_L3)
                  taskid = taskIdSelf();
              for(i =0; i < 40; i++)
              {
                      if(task_suspend_id[i] == taskid)
                          {
                              task_suspend_time[i]++;
                     flag =1;
                      break;
                          }
                  }
              if(flag==0)
              {
                    if(task_id_index<40)
                    {
                     task_suspend_id[task_id_index]=taskid;
                     task_suspend_time[task_id_index]++;
                    }
                     task_id_index++;
              }
    #else
                 taskSuspend(0);
    #endif
    #endif
              return;
        
    }
#endif
#ifdef MEM_DEBUG    
    int *headPtr = 0, *tailPtr = 0;
    //mem blk format: {realAddress(4),tag(4),sz(4), zero(4) data(sz),realAddress(4),tail(4)}
    headPtr = (int*)p - 4;  
    int sz = headPtr[2];
    if (headPtr[0] != (int)headPtr )
    {
        logMsg("Free a buffer not allocated at address %x\n", (UINT32)p, 0, 0, 0, 0, 0);
        g_del_invalid_addr_cnt ++;
        return;
    }
#if 0
    if (headPtr[3] != 0x12345678)
    {
        logMsg("Free a buffer allocated at address 0x%x with wrong tag 0x%x\n", (UINT32)p, headPtr[3], 0, 0, 0, 0);
        g_del_invalid_addr_cnt ++;
        return;
    }
#endif

    tailPtr = headPtr+4;
    tailPtr = (int*)((char *)tailPtr + sz);
    if (*tailPtr != (int)headPtr)
    {
        logMsg("\nwrite out of mem bound! with meory allcoated by Task 0x%x", headPtr[3], 0, 0, 0, 0, 0);
		{
			for(i = 0; i <( sz/4 +6); i++)
				{
				     printf("%08x,",headPtr[i]);
				    // p++;
				}
		}
        g_del_overwrite_cnt ++;
        return;
    }
    tailPtr++;
    if (*tailPtr != 0x5555aaaa)
    {
        logMsg("\nwrite out of mem bound 2! with meory allcoated by Task 0x%x", headPtr[1], 0, 0, 0, 0, 0);
        g_del_overwrite_cnt ++;
        return;
    }
    //headPtr[3] = 0x87654321;
#ifndef COMIP
    #ifdef M_TGT_L3
    updateMemoryUsage(headPtr[3], sz, M_OP_FREE);
    #endif
#endif
//	for L2TxInL3
#ifdef MEM_LEAK_DEBUG
		int tag = headPtr[1];
 		dec_mem_tag_count(tag);
#endif

    free ( headPtr);
#else
    free(p);
#endif//MEM_DEBUG
   g_del_cnt++;
}
#ifdef WBBU_CODE
void *___x_gnu_delaop_o;
void *___x_gnu_delop_o;
void *___x_gnu_newaop_o;
void *___x_gnu_newop_o;
#endif
void *___x_gnu_opnew_o;
void *___x_gnu_opvnew_o;
void *___x_gnu_opdel_o;
void *___x_gnu_opvdel_o;

void * operator new(size_t sz)
{
	return glob_new(sz);
}

void operator delete(void *p)
{
	glob_delete(p);
}

void * operator new [] (size_t sz)
{
	return glob_new(sz);
}

void operator delete [] (void *p)
{
	glob_delete(p);
}
#ifdef WBBU_CODE
int chk_newd_mem_blk(void *p)  
	{
		int 	*ph=(int*)p;
		int *pt;
		//
		ph-=4;
		if(*ph!=(int)ph)
			return -1;
		if(ph[3]!=0x12345678)
			return -1;
		pt=(int*)((char*)(ph+4)+ph[2]);	
		if(pt[0]!=(int)ph)
			return -1;
		if(pt[1]!=0x5555aaaa)
			return -1;
		return 0;
}
#endif
void test_new()
{
	logMsg("\ntest_new() ...", 0, 0, 0, 0, 0, 0);
	char  *p=new char [100];
	delete p;
	printf("\ng_new_cnt %d, g_del_cnt %d\n",g_new_cnt,g_del_cnt);
}


bool BtsL3MemInit()
{
    return true;
}

#ifdef MEM_DEBUG
void updateMemoryUsage(int taskID, int size, int flag)
{
    int idx = 0;
    for (idx = 0; idx < M_MAX_TASK_NUM; ++idx)
        {
        //如果找到该task，或者如果没有对应的task，则返回最后空闲的纪录索引idx.
        if ((g_mem_by_task[idx].taskID == taskID) || (0 == g_mem_by_task[idx].taskID))
            break;
        }
    if (M_MAX_TASK_NUM == idx)
        {
        logMsg("WARNING!!!.........g_mem_by_task is exhausted\n", 0, 0, 0, 0, 0, 0);
        return;
        }
    g_mem_by_task[idx].taskID   = taskID;
    if (M_OP_NEW  == flag)
        g_mem_by_task[idx].allocate += size;
    if (M_OP_FREE == flag)
        g_mem_by_task[idx].free     += size;
}

void memShowByTask()
{
    printf("%10s %15s %10s %10s %10s\n","[taskID]", "[taskName]", "[alloc]", "[free]", "[hold]");
    printf("-----------------------------------------------------------\n");
    for (int idx = 0; idx < M_MAX_TASK_NUM; ++idx)
        {
        int tid = g_mem_by_task[idx].taskID;
        if (0 != tid)
            {
            printf("%10x %15s %10d %10d %10d\n", tid, taskName(tid), 
                g_mem_by_task[idx].allocate, 
                g_mem_by_task[idx].free, 
                g_mem_by_task[idx].allocate - g_mem_by_task[idx].free);
            }
        }
}
#endif//MEM_DEBUG
#endif//COMIP
#endif//RELOAD_MEM_OPERATOR
