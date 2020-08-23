#include <msgQLib.h>
#include <string.h>
#include <stdio.h>
#include <taskLib.h>
#include <intLib.h>
#include <logLib.h>

#include "WanCpeIf.h"
#include "log.h"
#include "L3L2MessageId.h"


extern "C" {
typedef void (*pfWanIfFreeCallBack) (UINT32 param);
typedef void (*pfWanIfRxCallBack)(char *, UINT16, char *);
BOOL   mv643xxRecvMsgFromEB(char *,UINT16,pfWanIfFreeCallBack,UINT32);
void   Drv_Reclaim(void *);
void   Drv_RegisterEB(pfWanIfRxCallBack);
}

WanCpeIF* WanCpeIF::instance = NULL;
UINT32 gWAN_CPE_EID = 0;
UINT32 g_ulOccupiedCounter = 0;
UINT32 MsgCountFreeFromWCPEToWan;


WanCpeIF::WanCpeIF()
{
    strcpy(m_szName, "tWanIf");
    m_uPriority   = 115;
    m_uOptions    = 0;
    m_uStackSize  = 1024 * 10;

    m_iMsgQMax      = M_EGRESS_Q_SIZE;
    m_iMsgQOption   = MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY;

    m_plistComMessage = NULL;

	MsgCountFromWANToWCPE = 0;
	MsgCountFromWCPEToWAN = 0;
	MsgCountFreeFromWCPEToWan = 0;

}

TID WanCpeIF::GetEntityId() const 
{
    return M_TID_WANIF;
}


 
WanCpeIF* WanCpeIF::GetInstance()
{
    if ( instance == NULL)
    {
        instance = new WanCpeIF;
    }
    return instance;
}

bool WanCpeIF::Initialize()
{
    if( false == InitComMessageList() )
    {
        LOG( LOG_SEVERE, 0, "Fail to initialize WAN IF message pool!" );
        return false;
    }

    return CBizTask::Initialize();
}

#include "L3L2CfgWanIfCpeEidReq.h"
#include "L3OamCommonRsp.h"
UINT32 RxMsgCount = 0;
bool WanCpeIF::ProcessComMessage(CComMessage* pMsg)
{
    switch (pMsg->GetSrcTid())
    {
        case M_TID_CM:
            {
                CL3OamCfgWanIfCpeEidReq reqMsg(pMsg);

                if (0 == gWAN_CPE_EID)
                {  // config the first time
                    Drv_RegisterEB( WanCpeIF::RxDriverPacketCallBack );
                }

                gWAN_CPE_EID = reqMsg.GetWanCpeEid();
                LOG1( LOG_SEVERE, 0, "gWAN_CPE_EID = 0x%x",gWAN_CPE_EID);

                CL3OamCommonRsp L3OamCommonRsp;
                L3OamCommonRsp.CreateMessage(*this);
                L3OamCommonRsp.SetSrcTid(M_TID_WANIF);  
                L3OamCommonRsp.SetDstTid(M_TID_CM);  
                L3OamCommonRsp.SetMessageId(M_L2_L3_CFG_WANIF_CPE_RSP);
                L3OamCommonRsp.SetTransactionId(reqMsg.GetTransactionId());
                L3OamCommonRsp.SetResult(0);
                L3OamCommonRsp.Post();
            }
            break;

        default:
//            logMsg("Got a message from WCPE to Ethernet IF\n",0,0,0,0,0,0);
               SendToWAN(pMsg);
            break;
    }
    pMsg->Destroy();

}



bool WanCpeIF::InitComMessageList()
{
    for( UINT16 usIdx = 0; usIdx < M_BUFFERED_INGRESS_MSG_NUM; ++usIdx )
    {
        CComMessage      *pComMsg     = new ( this, 0 )CComMessage;
        if ( NULL == pComMsg )
        {
            //delete pComMsgNode;
            return false;
        }

        //pComMsgNode->pstComMsg = pComMsg;
        //只是网卡驱动给EB任务发送消息
        pComMsg->SetDstTid( M_TID_L2MAIN ); 
        pComMsg->SetSrcTid( M_TID_WANIF );
        pComMsg->SetMessageId( MSGID_LOW_PRIORITY_TRAFFIC );
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_plistComMessage);
        m_plistComMessage = pComMsg;
        ::intUnlock( intKey );
        ::taskUnlock();
    }

    return true;
}



void WanCpeIF::RxDriverPacketCallBack(char *pRxData, UINT16 usDataLength, char *pRxBuf)
{
//    logMsg("Got a packet from Ethernet IF to WCPE\n",0,0,0,0,0,0);
    usDataLength -= 6;/*a driver error.*/
    static WanCpeIF *pBridgeInstance = WanCpeIF::GetInstance();
    CComMessage *pComMsg = pBridgeInstance->GetComMessage();
    if( NULL == pComMsg )
        {
        //释放RDR.
        //DATA_assert( 0 );
        logMsg("\r\nEB Message pool is used up", 0, 0, 0, 0, 0, 0);
        ::Drv_Reclaim( pRxBuf );
        return;
        }

    pComMsg->SetBuffer( pRxBuf, usDataLength + M_DEFAULT_RESERVED );
    pComMsg->SetDataPtr( pRxData );
    pComMsg->SetDataLength( usDataLength );
    pComMsg->SetTimeStamp( 0 );
    pComMsg->SetFlag( M_CREATOR_DRV );
    pComMsg->SetEID(gWAN_CPE_EID);

    if( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        //DATA_assert( 0 );
        logMsg("\r\nDriver post packets to EB failed.", 0, 0, 0, 0, 0, 0);
        //pComMsg->Destroy();
        pBridgeInstance->DeallocateComMessage( pComMsg );
        }

    return;
}




void WanCpeIF::WanIfFreeMsgCallBack (UINT32 param)
{
MsgCountFreeFromWCPEToWan ++;
    ((CComMessage*)param)->Destroy();
    return;
}



bool WanCpeIF::SendToWAN(CComMessage *pComMsg)
{
    MsgCountFromWCPEToWAN ++;
    //增加计数
    pComMsg->AddRef();
    //发送WAN
/*    UINT8 *dataPtr=(UINT8*)pComMsg->GetDataPtr();
    logMsg("Msg from WCPE is: %x %x %x %x %x %x\n", dataPtr[0],dataPtr[1],
           dataPtr[2],dataPtr[3],dataPtr[4],dataPtr[5]);
*/           
    return mv643xxRecvMsgFromEB
        (
        (char*)pComMsg->GetDataPtr(),       //Data to send
        (UINT16)pComMsg->GetDataLength(),   //Data length
        WanCpeIF::WanIfFreeMsgCallBack,        //function.
        (UINT32)pComMsg                     //ComMessage ptr.
        );
}


CComMessage* WanCpeIF::GetComMessage()
{
    //空闲表头插入到空闲表头链表
    ::taskLock();
    UINT32 intKey = ::intLock();
    if (NULL == m_plistComMessage)
        {
        ::intUnlock( intKey );
        ::taskUnlock();
        return NULL;
        }
    //lstAdd( &m_listFreeNode, &( pNode->lstHdr ) );
    CComMessage *pComMsg = m_plistComMessage;
    m_plistComMessage    = m_plistComMessage->getNext();
    g_ulOccupiedCounter ++;
    MsgCountFromWANToWCPE ++;
    ::intUnlock( intKey );
    ::taskUnlock();
    //CComMessage *pComMsg = pNode->pstComMsg;
    return pComMsg;
}



bool WanCpeIF::DeallocateComMessage(CComMessage *pComMsg)
{
    UINT32 ulFlag = pComMsg->GetFlag();
    if (M_CREATOR_DRV == ulFlag) 
    {
        //驱动提供的缓存，调用驱动的释放函数
        ::Drv_Reclaim( pComMsg->GetBufferPtr() );
        pComMsg->DeleteBuffer();

        //VxWorks.
        pComMsg->SetDstTid( M_TID_L2MAIN ); 
        pComMsg->SetSrcTid( M_TID_WANIF ); 
        pComMsg->SetMessageId( MSGID_LOW_PRIORITY_TRAFFIC );
        pComMsg->SetFlag( 0 );
        pComMsg->SetEID( 0 );

        //空闲的ComMessage插入到链表头
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_plistComMessage);
        m_plistComMessage = pComMsg;
        --g_ulOccupiedCounter;
        ::intUnlock( intKey );
        ::taskUnlock();
    }
    else
    {
        CComEntity::DeallocateComMessage( pComMsg );
    }

    return true;
}

void WanCpeIF::ShowStatus()
{
    printf("=====================  WAN IF STATUS ==========================\n");
    printf("MsgCountFromWANToWCPE = %u\n",MsgCountFromWANToWCPE);
    printf("MsgCountFromWCPEToWAN = %u\n",MsgCountFromWCPEToWAN);
    printf("MsgCountFreeFromWCPEToWan = %u\n",MsgCountFreeFromWCPEToWan);
    printf("g_ulOccupiedCounter = %u\n",g_ulOccupiedCounter);
	printf("WAN IF CPE EID = 0x%x\n",gWAN_CPE_EID);
}

extern "C" 
STATUS wanIfShow()
{
    WanCpeIF::GetInstance()->ShowStatus();
    return OK;
}


extern "C"
STATUS setWanCpe(UINT32 EID)
{
    gWAN_CPE_EID = EID;
}
