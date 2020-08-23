#ifdef WIN32
#pragma warning (disable : 4786)
#pragma warning (disable : 4100)
#endif

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_MSGQUEUE
#include "MsgQueue.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

/////////////////////////////////////
#ifndef __NUCLEUS__

#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif

#ifndef _INC_TRANSACTIONMANAGER
#include "TransactionManager.h"
#endif

#ifndef _INC_TIMEOUTNOTIFY
#include "TimeOutNotify.h"
#endif

#endif

/////////////////////////////////////
#ifdef __WIN32_SIM__

#define MSG_Q_FIFO 0

#elif __NUCLEUS__

#ifndef NUCLEUS
#include "NUCLEUS.h"
#endif

#define MSG_Q_FIFO NU_FIFO

#else //VxWorks

#ifndef __INCmsgQLibh
//#include <msgQLib.h>//delete by huangjl
#endif

#endif

////////////////////////////////////
#ifndef _INC_LOG
#include "Log.h"
#endif

#ifdef WBBU_CODE

#define RESET_RECORD_NUM  4
#define NVRAM_BASE_ADDR         (0xF0000000)

#define NV_RAM_ADRS             NVRAM_BASE_ADDR            /* this is an offset, base addr*/



#define BOOT_LINE_ADRS_L3         (NVRAM_BASE_ADDR + /*0*/0x4)/***wangwenhua modify 20091126***/
#define BOOT_LINE_ADRS_L2         (NVRAM_BASE_ADDR + 0x100)  /** point to nvram (BOOT_LINE_ADRS + BOOT_LINE_SIZE) **/


#define NVRAM_BASE_ADDR_BOOT_STATE     (NVRAM_BASE_ADDR + 0x200)
#define NVRAM_BASE_ADDR_APP_PARAMS     (NVRAM_BASE_ADDR + 0x300)
#define NVRAM_BASE_ADDR_PARA_PARAMS (NVRAM_BASE_ADDR+0x400)/*用来保存新加的参数判断*/
#define NVRAM_BASE_ADDR_NETWORK_PARAS (NVRAM_BASE_ADDR+0x500)/***用来保存网口配置参数20090224**********/
#define NVRAM_BASE_ADDR_OTHER_PARAS (NVRAM_BASE_ADDR+0x600)/*保存参数在nvram中*/
#define NVRAM_BASE_ADDR_OAM            (NVRAM_BASE_ADDR + 0x1000)
#define NVRAM_OAM_MIB_SIZE             (100*1024)
#define NVRAM_BASE_ADDR_DATA           (NVRAM_BASE_ADDR_OAM + NVRAM_OAM_MIB_SIZE + 4096)	/*+4096:是为了保证和OAM不在同一NVRAM物理页上*/
#define NVRAM_DATA_SERVICE_SIZE        (204*1024)
#define NVRAM_TASK_SOCKET_DATA_BASE    (NVRAM_BASE_ADDR_DATA + NVRAM_DATA_SERVICE_SIZE)
#define NVRAM_TASK_SOCKET_DATA_SIZE    (12*1024)
#define NVRAM_CSI_BASE                 (NVRAM_TASK_SOCKET_DATA_BASE + NVRAM_TASK_SOCKET_DATA_SIZE)
#define NVRAM_CSI_SIZE                 (120*1024)
/*lijinan 20081217 计费系统增加*/
#define NVRAM_CDR_BASE    (NVRAM_CSI_BASE + NVRAM_CSI_SIZE + 4096)
#define NVRAM_CDR_SIZE    (4*1024)

#define SDRAM_TSK_INFORMATION (NVRAM_CDR_BASE + NVRAM_CDR_SIZE+ 4096)
typedef struct
{
    UINT32  tskInTick;
    UINT32  tskOutTick;	
    int  tskStatus;	
    char data[24];
}T_TSK_INFOR;

typedef struct _ST_RESET_RECORD
{
	int lastTskSwitchTick;
	T_TSK_INFOR stTskInfor[40];
	
} ST_RESET_RECORD;



typedef struct _ST_TSK_INFOR
{
       int chknum1;
	int resetNum;
	int resetCnt;
	ST_RESET_RECORD record[RESET_RECORD_NUM];
	int chknum2;
} ST_TskINFOR;
extern "C" int bspNvRamWrite(char * TargAddr, char *ScrBuff, int size);
extern "C" STATUS bspNvRamRead(char * TargAddr, char *SrcBuff, int size);
//ST_TskINFOR  TemppstTskInfor;


ST_TskINFOR *pstTskInfor = (ST_TskINFOR *)SDRAM_TSK_INFORMATION;
int startRecordFlag = false;
int g_recordNum;
//extern "C" int tickGet ();//modify by huangjl
int tickGet ()          //add by huangjl
{
    return 1024;
}
extern "C" int taskLock ();
extern "C" int taskUnlock ();
void mySwitchFxn(int tid,unsigned int inOut,void* pMsg)
{


	CComMessage* pComMsg =NULL;
	T_TSK_INFOR * taskInfor;
       int tick ;
	if(startRecordFlag!=true)
		return;
	::taskLock();
    if((tid>=40))//||(inOut>1))
       {
       	::taskUnlock();
        	return;
    	}

       tick = (int)tickGet();
	pstTskInfor->record[g_recordNum].lastTskSwitchTick = tick;
 
	taskInfor = &pstTskInfor->record[g_recordNum].stTskInfor[tid];
        if(inOut==0)
        {
            taskInfor->tskOutTick = tick;
        }
        else
        {

            taskInfor->tskInTick = tick;
        }
		taskInfor->tskStatus = inOut;
	pComMsg = (CComMessage*)pMsg;
	if(pComMsg!=NULL)
	{
		UINT16 temp = pComMsg->GetSrcTid();
		taskInfor->data[1] = tid;
		memcpy(taskInfor->data+2, (char*)&temp,2);
		temp = pComMsg->GetMessageId();
		memcpy(taskInfor->data+4, (char*)&temp,2);
		temp = pComMsg->GetDataLength();
		memcpy(taskInfor->data+6, (char*)&temp,2);
		UINT32 eid = pComMsg->GetEID();
		memcpy(taskInfor->data+8, (char*)&eid,4);
		UINT16 len = pComMsg->GetDataLength();
		if(len!=0)
		{
			char *pData = (char*)pComMsg->GetDataPtr();
			for(int i = 0;i<len;i++)
			{
				if((12+i)<24)
					taskInfor->data[12+i] =  pData[i];
				else
					break;
			}

		}
	}
	::taskUnlock();

}


void startTaskInforRecord()
{
	
	if((pstTskInfor->chknum1!=0x5a5a5a5a)||(pstTskInfor->chknum2!=0xa5a5a5a5))
	{

		memset((char*)pstTskInfor,0,sizeof(ST_TskINFOR));
		pstTskInfor->chknum1 =0x5a5a5a5a;
		pstTskInfor->chknum2 =0xa5a5a5a5;
		pstTskInfor->resetCnt = RESET_RECORD_NUM -1;
		pstTskInfor->resetNum = 0;
		printf("csi msg nvram err :%x,%x!!!\n",pstTskInfor->chknum1,pstTskInfor->chknum2);
	}
	pstTskInfor->resetCnt++; 
       pstTskInfor->resetNum++;
	if(pstTskInfor->resetCnt>=RESET_RECORD_NUM)
		pstTskInfor->resetCnt = 0;
	g_recordNum = pstTskInfor->resetCnt;
	memset((char*)&pstTskInfor->record[g_recordNum],0,sizeof(ST_RESET_RECORD));
	startRecordFlag = true;
}

void csiShowMsg(unsigned int num)
{
	int displayNum = g_recordNum;
	int i = 0;
	//int showtimes = 0;
	ST_RESET_RECORD* pstRecode; 
	int maxtimes = RESET_RECORD_NUM;
	if(pstTskInfor->resetNum==0)
	{
		printf("\n NO record before!!!\n");
		return;
	}
	if((num>RESET_RECORD_NUM)||(num==0))
	{
		printf("bts record max times is %d,please inter para < %x and >0 \n",maxtimes,(maxtimes+1));
		return;
	}
	if(num<=g_recordNum)
		displayNum = g_recordNum -num;
	else
		displayNum = g_recordNum+RESET_RECORD_NUM -num;

		pstRecode = &(pstTskInfor->record[displayNum]);
		printf("/-------the last work information,have reset num:%d -------------------\n",pstTskInfor->resetNum);
		printf("last time :%d tick,1 tick = 10ms\n",pstRecode->lastTskSwitchTick);

		printf("   %4s %10s %10s %10s\n","tid","intick","outtick","working");
		printf("--------------------------------------------------------------\n");
		for( i = 0;i<40;i++)
		{
			if((pstRecode->stTskInfor[i].tskOutTick!=0)||(pstRecode->stTskInfor[i].tskInTick!=0))
			{
		        printf("   %4d %10d %10d %10d\n",i,pstRecode->stTskInfor[i].tskInTick,pstRecode->stTskInfor[i].tskOutTick,pstRecode->stTskInfor[i].tskStatus);
			}
			
		}
		for( i = 0;i<40;i++)
		{
			
			char* pData = pstRecode->stTskInfor[i].data;
			if(!(pData[0]==0&&pData[1]==0&&pData[2]==0&&pData[3]==0))
			{
				UINT16 len = pData[6]*256+pData[7];
				printf("tsk id:%d last rec data:\n",i);
				len+= 12;
				for(int j =0;j<len;j++)
				{
					if(j>=24)
					{
						break;
					}
					printf(" %x",(UINT8)pData[j]);
				}
				printf("\n");
			}

		}
		printf("---------------------------------------------------------------\n");

}

#endif
#ifdef ENABLE_UT_CSI_SERVICE   
extern void Csi_ContextSwitchRecord(UINT32 Tid);
extern void Csi_TimerReport(void);
#endif
////////////////////////////////////
CBizTask::CBizTask()
:m_iMsgQMax(0), m_pMsgQueue(NULL)
, m_iMsgQOption(MSG_Q_FIFO)
#ifndef __NUCLEUS__
, m_pTransactManager(NULL)
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CBizTask::CBizTask");

    if (!Construct(CObject::M_OID_BIZTASK))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif
}

/*
CBizTask::~CBizTask()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CBizTask::~CBizTask");

    if (!Destruct(CObject::M_OID_BIZTASK))
        LOG(LOG_SEVERE,0,"Destruct failed.");
#endif
}
*/

bool CBizTask::PostMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CBizTask::PostMessage");

    if (!ASSERT_VALID(this))
        return false;

    if (m_pMsgQueue == NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsgQueue==NULL.");
        return false;
    }
#endif

    return m_pMsgQueue->PostMessage(pMsg, timeout, isUrgent);
}

#ifndef __NUCLEUS__
bool CBizTask::IsNeedTransaction() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CBizTask::IsNeedTransaction");

    if (!ASSERT_VALID(this))
        return false;
#endif

    return true;
}
#endif

bool CBizTask::Initialize()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CBizTask::Initialize");

    if (!ASSERT_VALID(this))
        return false;
#endif

    if (m_iMsgQMax > 0)
    {
        if (!CreateMsgQueue())
        {
            LOG(LOG_CRITICAL, 0, "Create MsgQueue failed.");
            return false;
        }
    }
    else
    {
        LOG(LOG_DEBUG, 0, "Initializing BizTask without MsgQueue.");
        return false;
    }

#ifndef __NUCLEUS__
    if (IsNeedTransaction())
    {
        m_pTransactManager = new CTransactionManager;
        if (m_pTransactManager==NULL)
        {
            LOG(LOG_SEVERE,0,"Create TransactionManager failed.");
            return false;
        }
    }
#endif
    return true;
}

CComMessage* CBizTask::GetMessage()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CBizTask::GetMessage");

    if (!ASSERT_VALID(this))
        return NULL;

    if (m_pMsgQueue==NULL)
    {
        LOG(LOG_SEVERE,0,"Invalid CMsgQueue pointer.");
        return NULL;
    }
#endif
#ifndef __NUCLEUS__
    if (m_bNeedDeadlockMonitor)
    {
        return m_pMsgQueue->GetMessage(m_uMaxBlockedTime);
    }
    else
    {
        return m_pMsgQueue->GetMessage(WAIT_FOREVER);
    }
#else
    return m_pMsgQueue->GetMessage(WAIT_FOREVER);
#endif

}

SINT32 CBizTask::GetMsgCount()
{
    if (m_pMsgQueue)
    {
        return m_pMsgQueue->GetCount();
    }
    else
    {
        return 0;
    }
}

#ifndef __NUCLEUS__
CTransaction* CBizTask::CreateTransact(CMessage& MsgRequest,
                                       CMessage& MsgTimeoutResp,
                                       SINT32 iCount,
                                       SINT32 iTimeout)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CBizTask::CreateTransact");

    if (!ASSERT_VALID(this))
        return NULL;
#endif

    if (IsNeedTransaction())
    {
        if (m_pTransactManager==NULL)
        {
            LOG(LOG_SEVERE,0,"No Transaction Manager");
            return NULL;
        }

        CTimeOutNotify TimeoutNotify;
        if (!TimeoutNotify.CreateMessage(*this))
        {
            LOG(LOG_CRITICAL,0,"Create TimeOutNotify message failed.");
            return NULL;
        }

        TimeoutNotify.SetDstTid(GetEntityId());

        CTransaction* pTransact = new CTransaction(MsgRequest,
                                                   TimeoutNotify,
                                                   MsgTimeoutResp,
                                                   iCount,
                                                   iTimeout);
        if (pTransact==NULL)
        {
            LOG(LOG_SEVERE,0,"Create CTransaction object failed.");
            TimeoutNotify.DeleteMessage();
            return NULL;
        }

        TimeoutNotify.SetTransactionId(pTransact->GetId());

        if (!m_pTransactManager->Insert(pTransact))
        {
            LOG(LOG_SEVERE,0,"Insert into CTransactionManager failed.");
            delete pTransact;
            TimeoutNotify.DeleteMessage();
            return NULL;
        }
        return pTransact;
    }
    return NULL;
}

CTransaction* CBizTask::FindTransact(UINT16 uId)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CBizTask::FindTransact");

    if (!ASSERT_VALID(this))
        return NULL;
#endif

    if (IsNeedTransaction())
    {
        if (m_pTransactManager==NULL)
        {
            LOG(LOG_SEVERE,0,"No Transaction Manager");
            return NULL;
        }
        return m_pTransactManager->Find(uId);
    }
    return NULL;
}
#endif

#ifndef NDEBUG
bool CBizTask::AssertValid(const char* lpszFileName, UINT32 nLine) const
{
#ifdef __WIN32_SIM__
    if (::IsBadReadPtr(this,sizeof(CBizTask)) || ::IsBadWritePtr((void*)this,sizeof(CBizTask)) )
    {
        CLog::LogAdd(lpszFileName,nLine,M_LL_CRITICAL,0,"Invalid CBizTask pointer.");
        return false;
    }
#endif
    return true;
}
#endif

bool CBizTask::CreateMsgQueue()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CBizTask::CreateMsgQueue");
    if (!ASSERT_VALID(this))
        return false;
#endif

    if (m_pMsgQueue!=NULL)
        return true;

    m_pMsgQueue = new CMsgQueue(m_iMsgQMax,m_iMsgQOption);

    return m_pMsgQueue!=NULL;
}

void CBizTask::MainLoop()
{
//printf(" in CBizTask::MainLoop\n");
    for(;;)
    {
    	#ifdef WBBU_CODE
    	 mySwitchFxn(GetEntityId(),0,NULL);
        CComMessage* pComMsg = GetMessage();
        mySwitchFxn(GetEntityId(),1,(void *)pComMsg);
	   #else
        CComMessage* pComMsg = GetMessage();
	   #endif


        if ( NULL == pComMsg )
        {
              continue;
        }

#ifndef NDEBUG
    LOG1(LOG_DEBUG3,0,"Received Message ID=0X%x",pComMsg->GetMessageId());
#endif

#ifdef ENABLE_UT_CSI_SERVICE         
        if(CPE_CPE_CSI_DETECT_ID == pComMsg->GetMessageId())
        {
            Csi_TimerReport();
            pComMsg->Destroy();
            continue;
        }            
#endif

        if (!ProcessComMessage(pComMsg))
        {
            LOG(LOG_WARN,0,"ProcessComMessage return false.");
            PostProcess();
        }
    }
}

#ifndef __NUCLEUS__
bool CBizTask::ProcessComMessage(CComMessage* pComMsg)
{
    #ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CBizTask::ProcessComMessage");
    
    if (!ASSERT_VALID(this))
        return false;
    
    if (!ASSERT_VALID(pComMsg))
        return false;
    #endif
    bool bRet;
    
    CMessage msg(pComMsg);
    
    if (msg.GetMessageId()==M_MSG_TIMEOUT_NOTIFY)
    {
        CTimeOutNotify notify(msg);
        CTransaction* pTransact = FindTransact(notify.GetTransactionId());
        if (pTransact!=NULL)
        {
            if (!pTransact->ReTransmit())
            {
                pTransact->EndTransact();
                delete pTransact;
                pTransact = NULL;//添加保护jiaying20101008
            }
        }
        bRet = true;
    }
    else
        bRet = ProcessMessage(msg);
    
    pComMsg->Destroy();
    return bRet;
}
#endif
