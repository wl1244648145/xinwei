/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   define the class for CComEntity -- application modules that 
 *                 communicate with each other through CComMessage
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   5/11/2005    Qun Liu      Initial file creation.
 *
 *   3/22/2006   Yushu Shi     Add message log enable feature
 *---------------------------------------------------------------------------*/

#ifndef _INC_COMENTITY
#include "ComEntity.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_APASSERT
#include "APASSERT.h"
#endif

#include "FrameworkErrorCode.h"
#include "L3L2MessageId.h"

#ifdef __NUCLEUS__
#include <string.h>
#endif

#ifndef __WIN32_SIM__
#ifndef __NUCLEUS__
#ifndef __INCstringh
#include <string.h>
#endif
//#include <intLib.h>//delete by huangjl
#ifndef COMIP
//#include <taskLib.h>//delete by huangjl
#include "Vxw_hdrs.h"       //add by huangjl
#include "vxwk2pthread.h"   //add by huangjl
#endif
#endif
#endif
#ifndef WBBU_CODE  
#if (M_TARGET==M_TGT_L3)
extern unsigned int task_suspend_id[40];
extern unsigned int task_suspend_time[40] ;
extern unsigned int task_id_index ;
#endif
#endif
#ifdef __WIN32_SIM__
CComEntity::EntityEntry CComEntity::s_EntityTable[M_TID_MAX+1];
CMutex * CComEntity::s_pmutexlist=NULL;
/************************
 *多任务的showStatus()总是
 *交互着打印，影响使用
 *增加一个互斥信号量
 *-by xiaoweifang
 */
CMutex CComEntity::s_mutexShowStatus;
#else
CComEntity::EntityEntry CComEntity::s_EntityTable[M_TID_MAX];
#endif

#ifndef __NUCLEUS__
UINT8 EntityMsgLogEnableTable[M_TID_MAX][M_TID_MAX] = {0};
char *EntityNameStr[M_TID_MAX] = {"UNKNOWN"};
void InitEntityNameStr();
#endif

bool CComEntity::s_bClassInited=false;
UINT32 CComEntity::PoolSize = 0;
UINT32 CComEntity::CurrentAllocatedCnt = 0;
UINT32 CComEntity::MaxAllocatedCnt = 0;
CComMessage *CComEntity::s_pFreeComMsg = NULL;

#ifdef __NUCLEUS__
UINT32 MaxComMsgPoolSize = 0;
#endif

#ifdef __NUCLEUS__
#ifdef BF_NU_L2
#pragma section("p_init")
#else
#pragma CODE_SECTION("p_init")
#endif
#endif
bool CComEntity::InitComEntityClass()
{
    if (s_bClassInited)
        return true;
#ifdef __WIN32_SIM__
    s_pmutexlist = new CMutex;
    if (s_pmutexlist==NULL)
        return false;
#endif

#ifdef M_MONITOR_RESOURCE
	memset(&g_ResMonitor, 0, sizeof(g_ResMonitor));
	updateResPoolSize(RES_MONITOR_COMMSG, M_COMMSG_POOL_INIT_SIZE);
#endif

    s_pFreeComMsg = (CComMessage*) new UINT8[M_COMMSG_POOL_INIT_SIZE*sizeof(CComMessage)];
    if (s_pFreeComMsg==NULL)
    {
        return false;
    }
	CComMessage* pComMsg = s_pFreeComMsg;
    for (int i= 0; i<M_COMMSG_POOL_INIT_SIZE -1; i++)
    {
        pComMsg->m_pNextComMsg = pComMsg+1;
        #ifdef __NUCLEUS__
        pComMsg->m_uBlockSize = M_COMMSG_POOL_INIT_SIZE;
        #endif
        ++pComMsg;
    }
	pComMsg->m_pNextComMsg = NULL;
    #ifdef __NUCLEUS__
    pComMsg->m_uBlockSize = M_COMMSG_POOL_INIT_SIZE;
    #endif
//    s_ulstComMsgSize = M_COMMSG_POOL_INIT_SIZE;
    PoolSize = M_COMMSG_POOL_INIT_SIZE;

#ifndef __NUCLEUS__
    InitEntityNameStr();
#endif

    s_bClassInited = true;
    return s_bClassInited;
}

#if (M_TARGET==M_TGT_L3)
UINT32 debug_eid = 0;
#endif
bool CComEntity::PostEntityMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComEntity::PostEntityMessage(timeout)");

    if (!ASSERT_VALID(pMsg))
        return false;

    LOGMSG(LOG_CRITICAL,0,pMsg,"CComMessage sent:");
#endif
  
//    ApAssertRtnV(pMsg, LOG_SEVERE, FMK_ERROR_POST_NULL_MSG, "Post Empty ComMessage" , ; , 
// false);//delete by huangjl

    TID tid = pMsg->GetDstTid();
    //ApAssertRtnV( (tid<M_TID_MAX), LOG_CRITICAL, FMK_ERROR_POST_MSG_FAIL,"Invalid Destination TID." , ; , false);
    if (tid>=M_TID_MAX)
        { 
        LOG2(LOG_CRITICAL, FMK_ERROR_POST_MSG_FAIL, "Invalid Destination TID[%d], messageID[0x%x].", pMsg->GetDstTid(), pMsg->GetMessageId()); 
        return false;
        }

    if (s_EntityTable[tid].pEntity==NULL) // || s_EntityTable[tid].idsys==0xFFFFFFFF)
    {
        LOGMSG(LOG_SEVERE,0,pMsg,"Destination Entity not registered.");
        return false;
    }
    TID srcTid = pMsg->GetSrcTid();
    if( srcTid>=M_TID_MAX)
    {
        LOG1(LOG_SEVERE, FMK_ERROR_POST_MSG_FAIL, "Invlaid Source Task ID %d", srcTid);
        LOGMSG(LOG_SEVERE,0,pMsg,"Invalid Src TID:"); //delete by huangjl
        return false;
    }
    else//add by huangjl
        LOG1(LOG_SEVERE, FMK_ERROR_POST_MSG_FAIL, "valid Source Task ID %d", srcTid);
    
#if (M_TARGET==M_TGT_L3)
    UINT32 eid = pMsg->GetEID();
#endif
#ifndef __NUCLEUS__
    if( EntityMsgLogEnableTable[srcTid][tid] )
    {        
        if((srcTid==M_TID_EB)||(tid==M_TID_EB))
        {
     #if (M_TARGET==M_TGT_L3)
             if(((debug_eid==eid)&&(eid!=0))||(debug_eid==0x12345678))
            {
                LOGMSG(LOG_SEVERE,0,pMsg," EB CComMessage Sent:");      
            }
     #endif
        }
        else
        {
            LOGMSG(LOG_SEVERE,0,pMsg,"CComMessage Sent:");
        }        
    }
#endif

    return s_EntityTable[tid].pEntity->PostMessage(pMsg,timeout, isUrgent);
}

#ifdef __WIN32_SIM__
TID CComEntity::LookupTid(UINT32 idsys)
{
    for (int i=0;i<M_TID_MAX;i++)
    {
        if (s_EntityTable[i].idsys == idsys)
            return (TID)i;
    }
    return M_TID_MAX;
}
#endif

bool CComEntity::RegisterEntity(bool bIdOnly)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComEntity::RegisterEntity");

    if (!ASSERT_VALID(this))
        return false;
#endif

    TID tid = GetEntityId();
//    printf("he1\n");

#ifndef __WIN32_SIM__
    if (tid==M_TID_MAX)
        return false;
#endif

    #ifndef __NUCLEUS__
	s_EntityTable[tid].idsys = m_idsys;
	
	#endif
	
    if (bIdOnly)
        return true;

    if (s_EntityTable[tid].pEntity==this)
    {
#ifndef NDEBUG
        LOG1(LOG_MINOR,0,"Reregister Entity ID=%d",tid);
#endif        
        return true;
    }
//  printf("he2\n");

    if (s_EntityTable[tid].pEntity!=NULL)
    {
#ifndef NDEBUG
        LOG1(LOG_CRITICAL,0,"Reregister tid=%d with different Entity Object.",tid);
#endif  
#ifdef WBBU_CODE
  LOG1(LOG_CRITICAL,0,"Reregister tid=%d with different Entity Object.",tid);
     if((tid!=M_TID_PM)&&(tid!=M_TID_FTPCLIENT))
     	{
     		 return false;
     	}
#else
        return false;
#endif
    }

    s_EntityTable[tid].pEntity = this;
    return true;
}

CComEntity::CComEntity()
:m_idsys(0xFFFFFFFF)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComEntity::CComEntity");

    if (!Construct(M_OID_COMENTITY))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif
}

//Low Level
void* CComEntity::Allocate(size_t &size)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComEntity::Allocate");

    if (!ASSERT_VALID(this))
        return NULL;
    if (size==0)
        return NULL;
#endif
    return (void*)::new UINT8[size];
}

#ifndef  __NUCLEUS__
CComEntity* CComEntity::getEntity( TID tid )
{
	return s_EntityTable[tid].pEntity;
}
#endif
/**********************************************************************
*
*  NAME:          setEntityNull
*  FUNTION:     设置注册队列为空
*  INPUT:          tid
*  OUTPUT:        无
  OTHERS:        jiaying20100820
*******************************************************************/

void CComEntity::setEntityNull(TID tid)
{
    s_EntityTable[tid].pEntity = NULL;
}
bool CComEntity::Deallocate(void* p, size_t size)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComEntity::Deallocate");

    if (!ASSERT_VALID(this))
        return false;
    if (p==NULL)
    {
        LOG(LOG_WARN,0,"p==NULL.");
    }
#endif
    delete [] (UINT8*)p;
    return true;
}

//#define NO_LIMIT_UPLINK_COMMSG
//High Level
#ifdef  __NUCLEUS__
#if !defined COMIP || !defined BF_NU_L2 
extern NU_MEMORY_POOL  ETH_SDRAM_MemoryPool;
#endif
void* CComEntity::AllocateComMessage(size_t &size, size_t &DataSize, bool canBeDiscarded)
#else
void* CComEntity::AllocateComMessage(size_t &size, size_t &DataSize)
#endif
	{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComEntity::AllocateComMessage(size_t,size_t)");
    if (!ASSERT_VALID(this))
        return NULL;

    if (!s_bClassInited)
    {
        LOG(LOG_CRITICAL,0,"ComEntity class not initialized.");
        return NULL;
    }
#endif

    CComMessage* pComMsg;

#ifdef __WIN32_SIM__
    if (!s_pmutexlist->Wait())
    {
        LOG(LOG_CRITICAL,0,"Wait s_pmutexlist failed.");
        return NULL;
    }
#elif __NUCLEUS__
    UINT16 oldlevel;
    oldlevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);
#else //VxWorks
    ::taskLock();
//    UINT32 oldlevel = ::intLock();//delete by huangjl
#endif

    if ( NULL == s_pFreeComMsg )
    {
        int grow = M_COMMSG_POOL_GROW_SIZE;
        #if (defined  __NUCLEUS__) && ((!defined COMIP)&&(!defined BF_NU_L2))
        if (PoolSize >= M_COMMSG_POOL_MAX_NUM)
        {
            #ifndef NO_LIMIT_UPLINK_COMMSG
            if (canBeDiscarded)
            {
                ::NU_Control_Interrupts(oldlevel);	//restore CPSR(restore global interrput)
                return NULL;
            }
			else
            #endif
			{
			    grow = 1;   // allocate only one node, to be deleted later
			    #ifndef NO_SDRAM
                NU_Allocate_Memory(&ETH_SDRAM_MemoryPool, (void**)&pComMsg, 
                                   sizeof(CComMessage), NU_NO_SUSPEND);    

				#else
				pComMsg = (CComMessage*) new UINT8[grow*sizeof(CComMessage)];
				#endif
		    }
        }
		else
        #endif

        {
        pComMsg = (CComMessage*) new UINT8[grow*sizeof(CComMessage)];
        }

		if (pComMsg==NULL)
        {
            #ifdef __WIN32_SIM__
            if (!s_pmutexlist->Release())
            {
                LOG(LOG_CRITICAL,0,"Release s_pmutexlist failed.");
            }
            #elif __NUCLEUS__ 
            ::NU_Control_Interrupts(oldlevel);	//restore CPSR(restore global interrput)
            #else //VxWorks
//            ::intUnlock(oldlevel);//delete by huangjl
		::taskUnlock();
            #endif
            
            return NULL;
        }

		for (int i=0;i<grow;++i)
        {
            #ifdef __NUCLEUS__
            pComMsg->m_uBlockSize = grow;
			#endif
			
            pComMsg->m_pNextComMsg = s_pFreeComMsg;
			s_pFreeComMsg = pComMsg;

            ++pComMsg;
        }

#ifdef M_MONITOR_RESOURCE
		//POOL Added
		addResPool(RES_MONITOR_COMMSG, grow);
#endif
		
//        s_ulstComMsgSize += grow;
        PoolSize += grow;
        #ifdef __NUCLEUS__
		if ( PoolSize > MaxComMsgPoolSize )
		{
		    MaxComMsgPoolSize = PoolSize;
	    }
		#endif
			
    }

	// must have a free ComMsg in the free list at this point
	pComMsg = s_pFreeComMsg;
	if (s_pFreeComMsg)
	{
        s_pFreeComMsg = s_pFreeComMsg->m_pNextComMsg;
	}

    CurrentAllocatedCnt ++;
    if (CurrentAllocatedCnt > MaxAllocatedCnt)
    {
        MaxAllocatedCnt = CurrentAllocatedCnt;
    }

#ifdef M_MONITOR_RESOURCE
	//ComMessage alloc
	TID tmp = this->GetEntityId();//getMsgCreatorTID(pComMsg);
	if(M_TID_MAX>tmp)
	{
		updateResStat(RES_MONITOR_COMMSG, RES_OP_ALLOC, 1, tmp);
	}
#endif

#ifdef __WIN32_SIM__
    if (!s_pmutexlist->Release())
    {
        LOG(LOG_CRITICAL,0,"Release s_pmutexlist failed.");
    }
#elif __NUCLEUS__ 
    ::NU_Control_Interrupts(oldlevel);	//restore CPSR(restore global interrput)
#else //VxWorks
 //   ::intUnlock(oldlevel);//delete by huangjl
		::taskUnlock();
#endif

    if (pComMsg==NULL)
        return NULL;

    pComMsg->m_pCreator = this;

    pComMsg->m_pBuf = NULL;
    pComMsg->m_uBufLen = 0;
    pComMsg->m_pData = NULL;
    pComMsg->m_uDataLen = 0;
#ifdef WBBU_CODE
   pComMsg->m_module = 0;
#endif
    if (DataSize>0)
    {
        size_t uBufLen = M_DEFAULT_RESERVED + DataSize;
        UINT8* pBuf = (UINT8*)Allocate(uBufLen);
        if (pBuf == NULL)
        {
            DeallocateComMessage(pComMsg);
            return NULL;
        }
        pComMsg->m_pBuf = pBuf;
        pComMsg->m_uBufLen = uBufLen;
        pComMsg->m_pData = (pBuf + M_DEFAULT_RESERVED);
        pComMsg->m_uDataLen = DataSize;
    }
	
    return pComMsg;
}

//****add by huangjl********************
char * sysMemTop (void)
{
    return (char*)0xF0000000;
}
//*************************************/

bool CComEntity::DeallocateComMessage(CComMessage *pComMsg)
{    
    unsigned char flag = 0;
    int i;
    unsigned int taskid;
#ifdef M_MONITOR_RESOURCE
	TID tmp = M_TID_MAX;
#endif
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComEntity::DeallocateComMessage(CComMessage*)");

    if (!ASSERT_VALID(this))
        return false;
    if (!ASSERT_VALID(pComMsg))
        return false;
#endif
 /* delete by huangjl
    ApAssertRtnV(pComMsg, LOG_SEVERE, FMK_ERROR_POST_NULL_MSG, "Deallocate Empty ComMessage" , ; , false);
    ApAssertRtnV(pComMsg->m_pCreator, LOG_SEVERE, FMK_ERROR_POST_NULL_MSG, 
                     "Deallocate ComMessage with Null",;,false); 
*/
    #ifndef __NUCLEUS__
    #ifndef ____WIN32_SIM__
    if (pComMsg->m_pBuf >= sysMemTop())
    {
        LOGMSG(LOG_CRITICAL, 0, pComMsg, "Free a ComMsg with invalid buffer Addr ");
        
        //   taskSuspend(0);   // suspend the calling task 
        /* unsigned char flag;
         int i;
          unsigned int taskid;*/
       #ifndef WBBU_CODE 

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
           return false;
#else
        taskSuspend(0); 
#endif
#endif
    }
    #endif
    #endif

    if (pComMsg->m_pBuf!=NULL && pComMsg->m_uBufLen>0)
    {
        if (!Deallocate(pComMsg->m_pBuf, pComMsg->m_uBufLen))
        {
            LOG(LOG_SEVERE,0,"Deallocate ComMessage buffer failed.Memory Leak.");
        }
    }


    UINT32 oldlevel;
    #ifdef __NUCLEUS__
	if ( 1 == pComMsg->m_uBlockSize )
	{   
	    #if (!defined NO_SDRAM) && (!defined COMIP)
        NU_Deallocate_Memory((void *)pComMsg);
		#else
	    delete (void *)pComMsg;
		#endif
        oldlevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);//disable global interrupt
		PoolSize --;
#ifdef M_MONITOR_RESOURCE
		//pool --
		addResPool(RES_MONITOR_COMMSG, -1);
		//commessage delete
		tmp = getMsgCreatorTID(pComMsg);
#endif
	}
	else
	#endif
	{

#ifdef M_MONITOR_RESOURCE
		//commessage delete
		tmp = getMsgCreatorTID(pComMsg);
#endif

        pComMsg->m_pBuf = NULL;
        pComMsg->m_uBufLen = 0;
        pComMsg->m_pData = NULL;
        pComMsg->m_uDataLen = 0;
        pComMsg->m_pCreator = NULL;

        #ifdef __WIN32_SIM__
        if (!s_pmutexlist->Wait())
        {
            LOG(LOG_CRITICAL,0,"Wait s_pmutexlist failed.");
            return false;
        }
        #elif __NUCLEUS__ 
        oldlevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);//disable global interrupt
        #else //VxWorks
		//::taskLock(); wangwenhua
        //oldlevel = ::intLock();
        #endif
	     
        pComMsg->m_pNextComMsg = s_pFreeComMsg;
		s_pFreeComMsg = pComMsg;

    }
   if(CurrentAllocatedCnt>0)
   	{
    CurrentAllocatedCnt --;
   	}
#ifdef M_MONITOR_RESOURCE
	//commessage delete
	if(M_TID_MAX>tmp)
	{		
		updateResStat(RES_MONITOR_COMMSG, RES_OP_FREE, -1, tmp);
	}
#endif

    #ifdef __WIN32_SIM__
    if (!s_pmutexlist->Release())
    {
        LOG(LOG_CRITICAL,0,"Release s_pmutexlist failed.");
    }
    #elif __NUCLEUS__ 
    ::NU_Control_Interrupts(oldlevel);//restore CPSR(restore global interrput)
    #else //VxWorks
    //::intUnlock(oldlevel);
    //::taskUnlock();
    #endif

    return true;
}

#ifndef NDEBUG
bool CComEntity::AssertValid(const char* lpszFileName, UINT32 nLine) const
{
#ifdef __WIN32_SIM__
    if (::IsBadReadPtr(this,sizeof(CComEntity)) || ::IsBadWritePtr((void*)this,sizeof(CComEntity)) )
    {
        CLog::LogAdd(lpszFileName,nLine,M_LL_CRITICAL,0,"Invalid CComEntity pointer.");
        return false;
    }
#endif

    if (GetEntityId() > M_TID_MAX)
    {
        LOG1(LOG_CRITICAL,0,"Invalid Entity ID=%d",GetEntityId());
        return false;
    }
    return true;
}
#endif

/*
CComEntity::~CComEntity()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComEntity::~CComEntity");

    if (!Destruct(M_OID_COMENTITY))
        LOG(LOG_SEVERE,0,"Destruct failed.");
#endif
}
*/

#ifndef __NUCLEUS__
#include <stdio.h>
void CComEntity::StatShow()
{
    printf(" ComMessage Pool size = %d\n", PoolSize);
    printf(" Current Allocated ComMessage Count = %d\n", CurrentAllocatedCnt);
    printf(" Maximum Allocated ComMessage Count = %d\n", MaxAllocatedCnt);
    printf(" Current Free ComMessage Count = %d\n", PoolSize - CurrentAllocatedCnt);
}
#endif



#ifndef __NUCLEUS__

void InitEntityNameStr()
{
    EntityNameStr[M_TID_CM] = "tCM"; 
    EntityNameStr[M_TID_FM] = "tALM"; 
    EntityNameStr[M_TID_PM] = "tPM"; 
    EntityNameStr[M_TID_UM] = "tUM"; 
    EntityNameStr[M_TID_SM] = "tSM"; 
    EntityNameStr[M_TID_DIAGM] = "tDiag";
    EntityNameStr[M_TID_BM] = "tL3Boot";  
    EntityNameStr[M_TID_GM] = "tGpsM";  
    EntityNameStr[M_TID_SYS] = "tSys"; 
    EntityNameStr[M_TID_FTPCLIENT] = "tFtp";
    EntityNameStr[M_TID_EMSAGENTTX] = "tEmsTx";
    EntityNameStr[M_TID_L2IF] = "tL2If";
    EntityNameStr[M_TID_LOG] = "tLog";
    EntityNameStr[M_TID_EB] = "tEB";
    EntityNameStr[M_TID_SNOOP] = "tSnoop";
    EntityNameStr[M_TID_TUNNEL] = "tTunnel";
    EntityNameStr[M_TID_ARP] = "tArp";
    EntityNameStr[M_TID_CLEANUP] = "tCleanUp";
    EntityNameStr[M_TID_DM] = "tDM";
    EntityNameStr[M_TID_TCR] = "tTCR";
    EntityNameStr[M_TID_TDR] = "tTDR";
    EntityNameStr[M_TID_VOICE] = "tVOICE";
	EntityNameStr[M_TID_VCR] = "tVCR";
	EntityNameStr[M_TID_VDR] = "tVDR";
#ifdef WBBU_CODE
     EntityNameStr[M_TID_WRRU] = "tWRRU";//for wrru
#endif
//fengbing 20091121 begin
	EntityNameStr[M_TID_VCR1] = "tVCR1";
	EntityNameStr[M_TID_VDR1] = "tVDR1";
	EntityNameStr[M_TID_SAG] = "tSag";
	EntityNameStr[M_TID_DGRV_LINK] = "tDGrpLink";
//fengbing 20091121 end
////EntityNameStr[M_TID_EMSAGENTRX] = "tEmsRx";
    EntityNameStr[M_TID_UTAGENT] = "tUtAgent";  

    EntityNameStr[M_TID_DAC] = "tDAC";
    EntityNameStr[M_TID_VAC] = "tVAC";
    EntityNameStr[M_TID_DIAG] = "tL2Diag";
    EntityNameStr[M_TID_L2OAM] = "tL2Oam";
    EntityNameStr[M_TID_L1TDDIF] = "tL1If";
    EntityNameStr[M_TID_AUXCTRLIF] = "tAuxIf";
    EntityNameStr[M_TID_L2BOOT] = "tL2Boot";
    EntityNameStr[M_TID_L2MAIN] = "tL2Main";
    EntityNameStr[M_TID_L3IF] = "tL3If";

    #ifdef M_TGT_WANIF
    EntityNameStr[M_TID_WANIF] = "tWanIf";
    #endif
    

    EntityNameStr[M_TID_CPECM] = "tUTCM";
    EntityNameStr[M_TID_CPESM] = "tUTSM";
	EntityNameStr[M_TID_UTV] = "tUTV";
    EntityNameStr[M_TID_UTDM] = "tUTDM";
	EntityNameStr[M_TID_CPEPM] = "tUTPM";
}

extern "C"
{
#define  TID_SHELL_BASE 2000
UINT32   tcm     =  TID_SHELL_BASE + M_TID_CM ;        // 0  Configuration Management
UINT32   talm    =  TID_SHELL_BASE + M_TID_FM;         // 1  Fault Management
UINT32   tpm     =  TID_SHELL_BASE + M_TID_PM;         // 2  Performance Management
UINT32   tum     =  TID_SHELL_BASE + M_TID_UM;         // 3  Ut Management
UINT32   tsm     =  TID_SHELL_BASE + M_TID_SM;         // 4  Software upgrade & transfer Management
UINT32   tdiag   =  TID_SHELL_BASE + M_TID_DIAGM;      // 5  DIAGnostic Management
UINT32   tbm     =  TID_SHELL_BASE + M_TID_BM;         // 6  Booting Management
UINT32   tgm     =  TID_SHELL_BASE + M_TID_GM;         // 7  Gps Management
UINT32   tsys    =  TID_SHELL_BASE + M_TID_SYS;        // 8  SYStem status management & alarm detection
UINT32   tftp    =  TID_SHELL_BASE + M_TID_FTPCLIENT;  // 9  FtpClient, do both file upload & download
UINT32   temst   =  TID_SHELL_BASE + M_TID_EMSAGENTTX; // 10 udp client on bts which send datagram to ems
UINT32   tl2if   =  TID_SHELL_BASE + M_TID_L2IF;      // 11 interface task on L3 which send packets to L2
UINT32   tlog    =  TID_SHELL_BASE + M_TID_LOG;        // 12 LOGger
UINT32   teb     =  TID_SHELL_BASE + M_TID_EB;                                                        // 
UINT32   tsnoop  =  TID_SHELL_BASE + M_TID_SNOOP;      // 14 Snoop
UINT32   ttunnel =  TID_SHELL_BASE + M_TID_TUNNEL;     // 15 Tunnel
UINT32   tarp    =  TID_SHELL_BASE + M_TID_ARP;        // 16 ARP
UINT32   tdm     =  TID_SHELL_BASE + M_TID_DM;         // 18 DM
UINT32   ttcr    =  TID_SHELL_BASE + M_TID_TCR;        // 19 TCR
UINT32   ttdr    =  TID_SHELL_BASE + M_TID_TDR;
UINT32   tvm     =  TID_SHELL_BASE + M_TID_VOICE;      // 21
UINT32   tvcr    =  TID_SHELL_BASE + M_TID_VCR;        // 22
UINT32   tvdr    =  TID_SHELL_BASE + M_TID_VDR;        // 23
#ifdef WBBU_CODE
UINT32  twrru = TID_SHELL_BASE+M_TID_WRRU ;//for wrru task
#endif
//fengbing 20091121 begin
UINT32 tvcr1 = TID_SHELL_BASE + M_TID_VCR1;//    M_TID_VCR1,	//20090624 btsL3_with_2SAG fengbing 
UINT32 tvdr1 = TID_SHELL_BASE + M_TID_VDR1;//    M_TID_VDR1,	//20090624 btsL3_with_2SAG fengbing 
UINT32 tsag = TID_SHELL_BASE + M_TID_SAG;//    M_TID_SAG,	//20091101 add by fengbing
UINT32 tdgrplink = TID_SHELL_BASE + M_TID_DGRV_LINK;//    M_TID_DGRV_LINK,//20091101 add by fengbing
//fengbing 20091121 end
//UINT32   temsr   =  TID_SHELL_BASE + M_TID_EMSAGENTRX; // 24 udp server on bts which recv datagram from ems
UINT32   tuta    =  TID_SHELL_BASE + M_TID_UTAGENT;  // 25
#ifdef M_TGT_WANIF
UINT32   twanif  =  TID_SHELL_BASE + M_TID_WANIF;
#endif
UINT32   tdac    =  TID_SHELL_BASE + M_TID_DAC;   // 50
UINT32   tvac    =  TID_SHELL_BASE + M_TID_VAC;        // 51
UINT32   tl2diag =  TID_SHELL_BASE + M_TID_DIAG;       // 52
UINT32   tl2oam  =  TID_SHELL_BASE + M_TID_L2OAM;      // 53
UINT32   tl1if   =  TID_SHELL_BASE + M_TID_L1TDDIF;    //54
UINT32   tauxif  =  TID_SHELL_BASE + M_TID_AUXCTRLIF;  //55
UINT32   tl2boot =  TID_SHELL_BASE + M_TID_L2BOOT;     //56
UINT32   tl2main =  TID_SHELL_BASE + M_TID_L2MAIN;     //57
UINT32   tl3if   =  TID_SHELL_BASE + M_TID_L3IF;       //58

UINT32   tutcm   =  TID_SHELL_BASE + M_TID_CPECM;      // 101
UINT32   tutsm   =  TID_SHELL_BASE + M_TID_CPESM;      // 102
UINT32   tutv    =  TID_SHELL_BASE + M_TID_UTV;	      // 107 CPE voice task
UINT32   tutdm   =  TID_SHELL_BASE + M_TID_UTDM;       // 108 CPE DM task
UINT32   tutpm   =  TID_SHELL_BASE + M_TID_CPEPM;	  // 110 CPE Performance Task
UINT32   tmax    =  TID_SHELL_BASE + M_TID_MAX;
}


#define IS_VALID_TID(tid)  (tid < tmax && tid >= tcm/* && tid != teb*/)

#include <stdio.h>
extern "C"
int ChangeUnitMsgLog(UINT16 srcTID, UINT16 dstTID, UINT8 value)
{
    if ( ! IS_VALID_TID(srcTID))
    {
        printf (" Srouce Task ID not valid\r\n");
        return ERROR;
    }
    if ( ! IS_VALID_TID(dstTID))
    {
        printf(" Destination Task ID not valid\r\n");
        return ERROR;
    }

    srcTID -= TID_SHELL_BASE;
    dstTID -= TID_SHELL_BASE;
    EntityMsgLogEnableTable[srcTID][dstTID] = value;
    printf ("Msg Log From %s to %s is " ,EntityNameStr[srcTID], EntityNameStr[dstTID]);
    if (value==1)
        printf("enabled\r\n");
    else
        printf("disabled\r\n");
    return 0;
}

int enableMsgLog(UINT16 srcTID, UINT16 dstTID)
{
    return ChangeUnitMsgLog(srcTID, dstTID, 1);
}

int disableMsgLog(UINT16 srcTID, UINT16 dstTID)
{
    return ChangeUnitMsgLog(srcTID, dstTID, 0);
}

int ChangeMsgLogFrom(UINT16 srcTID, UINT8 value)
{
    if ( ! IS_VALID_TID(srcTID))
    {
        printf(" Srouce Task ID not valid\r\n");
        return -1;
    }
    srcTID -= TID_SHELL_BASE;
    for (int dst=0; dst<M_TID_MAX; dst++)
    {
        if( 1/*dst != M_TID_EB*/)
        {
            EntityMsgLogEnableTable[srcTID][dst] = value;
        }
    }

    printf("Msg Log From %s are all ", EntityNameStr[srcTID]);
    if (value==1)
        printf( "enabled\r\n");
    else
        printf("disabled\r\n");
    return 0;
}

int enableMsgLogFrom(UINT16 srcTID)
{
    return ChangeMsgLogFrom(srcTID, 1);
}

int disableMsgLogFrom(UINT16 srcTID)
{
    return ChangeMsgLogFrom(srcTID, 0);
}


int ChangeMsgLogTo(UINT16 dstTID, UINT8 value)
{
    if ( ! IS_VALID_TID(dstTID))
    {
        printf(" Destination Task ID not valid\r\n");
        return -1;
    }
    dstTID -= TID_SHELL_BASE;
    for (int src=0; src<M_TID_MAX; src++)
    {
        if(1/* src != M_TID_EB*/)
        {
            EntityMsgLogEnableTable[src][dstTID] = value;
        }
    }

    printf("Msg Log To %s are all", EntityNameStr[dstTID]);
    if (value==1)
        printf("enabled\r\n");
    else
        printf("disabled\r\n");
    return 0;
}

int enableMsgLogTo(UINT16 dstTID)
{
    return ChangeMsgLogTo(dstTID, 1);
}

int disableMsgLogTo(UINT16 dstTID)
{
    return ChangeMsgLogTo(dstTID, 0);
}

    
int disableMsgLogAll()
{
    for (int src=0; src<M_TID_MAX; src++)
    {
        if( 1/*src != M_TID_EB*/)
        {
            for (int dst=0; dst<M_TID_MAX; dst++)
            {
                if( 1/*dst != M_TID_EB*/)
                {
                    EntityMsgLogEnableTable[src][dst] = 0;
                }
            }
        }
    }
    printf(" Msg Log is disabled for all messages\r\n");
    return 0;
}


int enableMsgLogAll()
{
    for (int src=0; src<M_TID_MAX; src++)
    {
        if( src != M_TID_EB)
        {
            for (int dst=0; dst<M_TID_MAX; dst++)
            {
                if( dst != M_TID_EB)
                {

                    EntityMsgLogEnableTable[src][dst] = 1;
                }
            }
        }
    }
    printf(" Msg Log is enabled for all messages\r\n");
    return 0;
}

extern "C" 
#ifndef __WIN32_SIM__
STATUS msgPoolStatShow()
{
    CComEntity::StatShow();
    return OK;
}
#else
int msgPoolStatShow()
{
    CComEntity::StatShow();
    return 0;
}
#endif
#ifndef WBBU_CODE 
#if (M_TARGET==M_TGT_L3)
void suspendShow()
{
    int i;
     printf( "\r\n");
    printf( "\r\n***************************************************" );
    printf( "\r\n*                        task 
suspendShow                             *" );
    printf( "\r\n***************************************************" );
    
    printf( "\r\n" );
    printf( "\r\n|%-10s|%-10s|" ,"suspend_task_id","suspend_time");

     for (i = 0; i < task_id_index; i++)
    {
        printf( "\r\n|%-10x|%-10d|",task_suspend_id[i],task_suspend_time[i]);

    }
    printf("\n");
}
#endif
#endif
#ifndef __NUCLEUS__

#ifndef _INC_L3CPEMESSAGEID
#include "L3CPEMessageId.h"
#endif
#ifndef _INC_MESSAGE
#include "Message.h"
#endif
void clearCpeHist( UINT32 ulEID )
{
	printf( "clearCpeHist->ulEID[%02x]", ulEID );
	CComEntity* pEntity = CComEntity::getEntity(M_TID_UM);
	if( NULL == pEntity )
		printf( "ComEntity->clearCpeHist()-> Task[M_TID_UM] is NA!!!! " );
	#if 1
	CComMessage* pComMsg = new ( pEntity, 4 ) CComMessage;
	if( NULL == pComMsg )
		printf( "ComEntity->clearCpeHist()-> pMsg is NULL!!!! " );
	else
	{
		pComMsg->SetMessageId( M_L3_CPE_CLEAR_HIST_REQ );
		pComMsg->SetDstTid( M_TID_UM );		
		pComMsg->SetSrcTid( M_TID_UM );
		pComMsg->SetEID( ulEID );
		*(UINT32*)pComMsg->GetDataPtr() = ulEID;
		CMessage pMsg( pComMsg );
		if( !pMsg.Post( 300000 ) )
			printf( "ComEntity->clearCpeHist()->Post failed!!!!" );
	}
	#else
		CMessage::CreateMessage(pEntity,4);
		
	#endif
}
#endif
#endif

#ifdef M_MONITOR_RESOURCE

//资源监控计数，暂时只在bts使用,cpe可以自己调整使用
ResStatCntT g_ResMonitor;
static char resName[RES_MONITOR_MAX][20]={"Timer","ComMessage"};
static char opName[RES_OP_MAX][20]={"alloc","free"};
void updateResPoolSize(Enum_RES_TYPE type, UINT32 size)
{
	g_ResMonitor.res_Cnt[type].nPoolSize = size;
}
void addResPool(Enum_RES_TYPE type, int added)
{
	g_ResMonitor.res_Cnt[type].nPoolSize += added;
}
bool updateResStat(Enum_RES_TYPE type, ENUM_RES_OP opType, int count, TID tid)
{
	g_ResMonitor.res_Cnt[type].nInUse+=count;
	if(g_ResMonitor.res_Cnt[type].nInUse>g_ResMonitor.res_Cnt[type].nInUseMAX)
	{
		g_ResMonitor.res_Cnt[type].nInUseMAX = g_ResMonitor.res_Cnt[type].nInUse;
	}
	g_ResMonitor.res_Task_Cnt[type][tid].nCnt[opType] += (count>0?count:(0-count));
}
TID getMsgCreatorTID(CComMessage* pComMsg)
{
	if(NULL==pComMsg)
	{
		return M_TID_MAX;
	}
	else
	{
		CComEntity* pEntity = pComMsg->getCreator();
		if(NULL!=pEntity)
		{
			return pEntity->GetEntityId();
		}
		else
		{
			return M_TID_MAX;
		}
	}
}
void showResStat()
{
	int idx,idtask;
	printf("\n-----------------------------------------------------------\n");
	for (idx=0;idx<RES_MONITOR_MAX;idx++)
	{
		printf("%10s PoolSize[%10d] InUse[%10d] InUseMAX[%10d]\n",
			resName[idx],
			g_ResMonitor.res_Cnt[idx].nPoolSize,
			g_ResMonitor.res_Cnt[idx].nInUse, 
			g_ResMonitor.res_Cnt[idx].nInUseMAX);
		printf("\t\t%10s %10s %10s %10s","[taskName]", "[alloc]", "[free]", "[hold]");
		for (idtask=0;idtask<M_TID_MAX;idtask++)
		{
			if(g_ResMonitor.res_Task_Cnt[idx][idtask].nCnt[RES_OP_ALLOC])
			{
				printf("\n\t%10s[%3d] %10d %10d %10d",
					EntityNameStr[idtask],
					idtask,
					g_ResMonitor.res_Task_Cnt[idx][idtask].nCnt[RES_OP_ALLOC],
					g_ResMonitor.res_Task_Cnt[idx][idtask].nCnt[RES_OP_FREE],
					g_ResMonitor.res_Task_Cnt[idx][idtask].nCnt[RES_OP_ALLOC]-g_ResMonitor.res_Task_Cnt[idx][idtask].nCnt[RES_OP_FREE]
					);
			}
		}
		printf("\n-----------------------------------------------------------\n");		
	}
}

#endif

#ifdef WBBU_CODE
//zengjihan 20120801 for MSGLOG
void msgLogShow()
{
    printf("%s (shell name: %s)\n","tCM      ", "tcm      ");
    printf("%s (shell name: %s)\n","tALM     ", "talm     ");
    printf("%s (shell name: %s)\n","tPM      ", "tpm      ");
    printf("%s (shell name: %s)\n","tUM      ", "tum      ");
    printf("%s (shell name: %s)\n","tSM      ", "tsm      ");
    printf("%s (shell name: %s)\n","tDiag    ", "tdiag    ");
    printf("%s (shell name: %s)\n","tL3Boot  ", "tbm      ");
    printf("%s (shell name: %s)\n","tGpsM    ", "tgm      ");
    printf("%s (shell name: %s)\n","tSys     ", "tsys     ");
    printf("%s (shell name: %s)\n","tFtp     ", "tftp     ");
    printf("%s (shell name: %s)\n","tEmsTx   ", "temst    ");
    printf("%s (shell name: %s)\n","tL2If    ", "tl2if    ");
    printf("%s (shell name: %s)\n","tLog     ", "tlog     ");
    printf("%s (shell name: %s)\n","tEB      ", "teb      ");
    printf("%s (shell name: %s)\n","tSnoop   ", "tsnoop   ");
    printf("%s (shell name: %s)\n","tTunnel  ", "ttunnel  ");
    printf("%s (shell name: %s)\n","tArp     ", "tarp     ");
    printf("%s (shell name: %s)\n","tDM      ", "tdm      ");
    printf("%s (shell name: %s)\n","tTCR     ", "ttcr     ");
    printf("%s (shell name: %s)\n","tTDR     ", "ttdr     ");
    printf("%s (shell name: %s)\n","tVOICE   ", "tvm      ");
    printf("%s (shell name: %s)\n","tVCR     ", "tvcr     ");
    printf("%s (shell name: %s)\n","tVDR     ", "tvdr     ");
    printf("%s (shell name: %s)\n","tWRRU    ", "twrru    ");
    printf("%s (shell name: %s)\n","tVCR1    ", "tvcr1    ");
    printf("%s (shell name: %s)\n","tVDR1    ", "tvdr1    ");
    printf("%s (shell name: %s)\n","tSag     ", "tsag     ");
    printf("%s (shell name: %s)\n","tDGrpLink", "tdgrplink");
    printf("%s (shell name: %s)\n","tUtAgent ", "tuta     ");
    printf("%s (shell name: %s)\n","tDAC     ", "tdac     ");
    printf("%s (shell name: %s)\n","tVAC     ", "tvac     ");
    printf("%s (shell name: %s)\n","tL2Diag  ", "tl2diag  ");
    printf("%s (shell name: %s)\n","tL2Oam   ", "tl2oam   ");
    printf("%s (shell name: %s)\n","tL1If    ", "tl1if    ");
    printf("%s (shell name: %s)\n","tAuxIf   ", "tauxif   ");
    printf("%s (shell name: %s)\n","tL2Boot  ", "tl2boot  ");
    printf("%s (shell name: %s)\n","tL2Main  ", "tl2main  ");
    printf("%s (shell name: %s)\n","tL3If    ", "tl3if    ");
    printf("%s (shell name: %s)\n","tWanIf   ", "twanif   ");
    printf("%s (shell name: %s)\n","tUTCM    ", "tutcm    ");
    printf("%s (shell name: %s)\n","tUTSM    ", "tutsm    ");
    printf("%s (shell name: %s)\n","tUTV     ", "tutv     ");
    printf("%s (shell name: %s)\n","tUTDM    ", "tutdm    ");
    printf("%s (shell name: %s)\n","tUTPM    ", "tutpm    ");
}
#endif

