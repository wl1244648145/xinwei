/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    UTBridge.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   02/07/06   Yushu Shi    initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <intLib.h>
#include <logLib.h>

#include "UTBridge.h"
#include "biztask.h"
#include "L3L2MessageId.h"
#include "log.h"

extern "C" {
    typedef void (*pfEBFreeCallBack) (UINT32 param);
    typedef void (*pfEBRxCallBack)(char *, UINT16, char *);
    BOOL   mv643xxRecvMsgFromEB(char *,UINT16,pfEBFreeCallBack,UINT32);
    void   Drv_Reclaim(void *);
    void   Drv_RegisterEB(pfEBRxCallBack);
}


//����ʵ��ָ��ĳ�ʼ��
CUTBridge* CUTBridge::Instance = NULL;
extern UINT16 BridgeBtsMacAddr[3]; 

/*============================================================
MEMBER FUNCTION:
    CUTBridge::ProcessComMessage

DESCRIPTION:
    Bridge������Ϣ������

ARGUMENTS:
    *pComMsg: ��Ϣ

RETURN VALUE:
    bool:true or false,FrameWork���ݷ���ֵ�����Ƿ���PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
bool CUTBridge::ProcessComMessage(CComMessage *msg)
{
    T_SecondHeader *header;
    switch ( msg->GetDstTid() )
    {
        case M_TID_L2MAIN:   // from BTS to UT
        case M_TID_DAC:
        case M_TID_UTVAC:
LOGMSG(LOG_CRITICAL,0,msg,"UtBridge Get From L3:");
            if ( msg->GetBufferLength()- msg->GetDataLength() < sizeof(T_SecondHeader))
            {
                msg->Destroy();
                return false;  // can not relay messages without enough leading space
            }

            // move the data pointer forward
            msg->SetDataPtr((UINT8*)msg->GetDataPtr() - sizeof(T_SecondHeader));

            header = (T_SecondHeader*)msg->GetDataPtr();

            // compose the second header and change the ComMessage fields
            header->BtsMacAddr[0] = BridgeBtsMacAddr[0];
            header->BtsMacAddr[1] = BridgeBtsMacAddr[1];
            header->BtsMacAddr[2] = BridgeBtsMacAddr[2];
            header->CPEMacAddr[0] = 0;
            header->CPEMacAddr[1] = msg->GetEID()>>16;
            header->CPEMacAddr[2] = (UINT16)msg->GetEID()&0xffff;
            header->Protocol = 0x9998;
            header->SrcTID = msg->GetSrcTid();
            header->MsgID = msg->GetMessageId();
            header->Length = msg->GetDataLength();

            switch (msg->GetMessageId())
            {
                case MSGID_HI_PRIORITY_UNICAST_OAM:
                case MSGID_LOW_PRIORITY_UNICAST_OAM:
                    header->DstTID = M_TID_BTSAGENT;
                    break;
                case MSGID_HIGH_PRIORITY_TRAFFIC:
                case MSGID_LOW_PRIORITY_TRAFFIC:
                    header->DstTID = M_TID_UTDM;
                    break;
                default:  // to UT VAC
                    if (msg->GetDstTid() != M_TID_UTVAC)
                    {
                        msg->Destroy();
                        return false;
                    }
                    header->DstTID = msg->GetDstTid();
                    break;
            }

            msg->SetDataLength(msg->GetDataLength() + sizeof(T_SecondHeader));

            if (! mv643xxRecvMsgFromEB ( (char*)msg->GetDataPtr(),       //Data to send
                                         (UINT16)msg->GetDataLength(),   //Data length
                                         CUTBridge::EBFreeMsgCallBack,        //function.
                                         (UINT32)msg                     //ComMessage ptr.
                                       ))
            {
                msg->Destroy();
                return false;
            }

            return true;

        case M_TID_UTBRIDGE:  // from UT, from Ethernet driver
LOGMSG(LOG_CRITICAL,0,msg,"UtBridge Get From UT:");

            header = (T_SecondHeader *)msg->GetDataPtr();

            // strip off the second header in payload and change the ComMessage 
            msg->SetDstTid((TID)header->DstTID);
            msg->SetSrcTid((TID)header->SrcTID);
            msg->SetMessageId(header->MsgID);
            msg->SetEID( (header->CPEMacAddr[4]<<16 ) | header->CPEMacAddr[5]);

            msg->SetDataLength( header->Length );
            msg->SetDataPtr( (UINT8*)header +  sizeof(T_SecondHeader));

LOGMSG(LOG_CRITICAL,0,msg,"UtBridge Sent:");
            if ( ! CComEntity::PostEntityMessage(msg))
            {
                msg->Destroy();
                return false;
            }
            break;
        default:
            printf("Invalide UTBridge Packet\n");
            //msg->Destroy();
            break;
    }
    msg->Destroy();
    return true;
}



/*============================================================
MEMBER FUNCTION:
    CUTBridge::CUTBridge

DESCRIPTION:
    CUTBridge���캯��

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CUTBridge::CUTBridge()      
{
    memset( m_szName, 0 , strlen(M_TASK_UTBRIDGE_TASKNAME ));
    memcpy( m_szName, M_TASK_UTBRIDGE_TASKNAME, strlen( M_TASK_UTBRIDGE_TASKNAME ) );
    m_uPriority     = M_TP_L3EB;
    m_uOptions      = M_TASK_UTBRIDGE_OPTION;
    m_uStackSize    = M_TASK_UTBRIDGE_STACKSIZE;

    m_iMsgQMax      = M_TASK_UTBRIDGE_MAXMSG;
    m_iMsgQOption   = M_TASK_UTBRIDGE_MSGOPTION;


    //��ʼ��ComMessage����
    lstInit( &m_listFreeNode );
    lstInit( &m_listComMessage );

    ProxyTIDs[0] = M_TID_L2MAIN;
    ProxyTIDs[1] = M_TID_UTVAC;
    ProxyTIDs[2] = M_TID_UTBRIDGE;
    ProxyTIDs[3] = M_TID_DAC;


    for (int i=0; i<(SIZEOF(ProxyTIDs)); i++)
    {
        CurrentTid = ProxyTIDs[i];
        RegisterEntity(false);
    }

    CurrentTid = M_TID_UTBRIDGE;
}



/*============================================================
MEMBER FUNCTION:
    CUTBridge::~CUTBridge

DESCRIPTION:
    CUTBridge��������

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CUTBridge::~CUTBridge()      
{
}


/*============================================================
MEMBER FUNCTION:
    CUTBridge::GetInstance

DESCRIPTION:
    Get CUTBridge Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CUTBridge* 

SIDE EFFECTS:
    none
==============================================================*/
CUTBridge* CUTBridge::GetInstance(){
    if ( NULL == Instance )
    {
        Instance = new CUTBridge;
    }
    return Instance;
}



/*============================================================
MEMBER FUNCTION:
    CUTBridge::Initialize

DESCRIPTION:
    ����Message Queue;����Socket

ARGUMENTS:
    NULL

RETURN VALUE:
    true of false 

SIDE EFFECTS:
    none
==============================================================*/
bool CUTBridge::Initialize(){

    /*============================================================
        ��ʼ��ComMessage������ʼ��=2000����
        �������ֻ�������������ʹ��
    ==============================================================*/
    for ( UINT16 usIdx = 0; usIdx < M_MAX_LIST_SIZE; ++usIdx )
    {
        stComMessageNode *pComMsgNode = (stComMessageNode *)new stComMessageNode;
        CComMessage *pComMsg = new ( this, 0 )CComMessage;
        if ( ( NULL == pComMsg ) || ( NULL == pComMsgNode ) )
        {
            return false;
        }
        pComMsgNode->pstComMsg = pComMsg;
        pComMsg->SetFlag( 0xffffffff );
        //ֻ������������EB��������Ϣ
        pComMsg->SetDstTid( M_TID_UTBRIDGE ); 
        pComMsg->SetSrcTid( M_TID_UTBRIDGE ); 
        pComMsg->SetMessageId( 0x5555 );
        UINT32 intKey = ::intLock();
        lstAdd( &m_listComMessage, &( pComMsgNode->lstHdr ) );
        ::intUnlock( intKey );
    }


    UINT8 ucInit = CBizTask::Initialize();
    if ( false == ucInit )
    {
        return false;
    }

    //������ע�ᱨ�Ľ��պ���
    Drv_RegisterEB( CUTBridge::RxDriverPacketCallBack );

    return true;
}



/*============================================================
MEMBER FUNCTION:
    CUTBridge::DeallocateComMessage

DESCRIPTION:
    �ͷ�ComMessage�ڴ�

ARGUMENTS:
    *pComMsg

RETURN VALUE:
    bool: true,�ɹ���false,ʧ��

SIDE EFFECTS:
    none
==============================================================*/
bool CUTBridge::DeallocateComMessage(CComMessage *pComMsg){

    UINT32 ulFlag = pComMsg->GetFlag();
    if ( 0xffffffff == ulFlag )
    {
        ::Drv_Reclaim( pComMsg->GetBufferPtr() );
        pComMsg->DeleteBuffer();
        pComMsg->SetDataLength( 0 );
        pComMsg->SetDataPtr( NULL );
        pComMsg->SetDstTid( M_TID_UTBRIDGE ); 
        pComMsg->SetSrcTid( M_TID_UTBRIDGE ); 
        pComMsg->SetMessageId( 0x5555 );

        //�����ComMessage����
        UINT32 intKey = ::intLock();
        //��ͷ�ӿ�������ȡ
        stComMessageNode *pComMsgNode = (stComMessageNode *)lstGet( &m_listFreeNode );
        if ( NULL == pComMsgNode )
        {
            ::intUnlock( intKey );
            return false;
        }
        //Add to the end
        lstAdd( &m_listComMessage, &( pComMsgNode->lstHdr ) );
        ::intUnlock( intKey );
        pComMsgNode->pstComMsg = pComMsg;
    }
    else
    {
        CComEntity::DeallocateComMessage( pComMsg );
    }

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CUTBridge::GetComMessage

DESCRIPTION:
    ��ComMessage�����л�ȡComMessage.

ARGUMENTS:
    void

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
CComMessage* CUTBridge::GetComMessage()
{

    //Get first Node.
    UINT32 intKey = ::intLock();
    stComMessageNode *pNode = (stComMessageNode *)lstGet( &m_listComMessage );
    if ( NULL == pNode )
    {
        ::intUnlock( intKey );
        return NULL;
    }

    //���б�ͷ���뵽���б�ͷ����
    lstAdd( &m_listFreeNode, &( pNode->lstHdr ) );
    ::intUnlock( intKey );
    CComMessage *pComMsg = pNode->pstComMsg;
    //delete pNode;
    return pComMsg;
}


/*============================================================
MEMBER FUNCTION:
    CUTBridge::RxDriverPacketCallBack

DESCRIPTION:
    �����������͵ı���

ARGUMENTS:
    char* : data to be sent.
    UINT16: data length
    char* : buffer ptr.

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CUTBridge::RxDriverPacketCallBack(char *pData, UINT16 usDataLength, char *pBuf)
{
logMsg("Got bridge message in utBridge %x %x %x %x %x %x\n", pData[0], 
       pData[1], pData[2], pData[11],pData[12], pData[13]);
    usDataLength -= 6;/*a driver error.*/

    CComMessage *pComMsg = CUTBridge::GetInstance()->GetComMessage();

    if (pComMsg == NULL)
    {
        //�ͷ�RDR.
        ::Drv_Reclaim( pBuf );
        return;
    }

    pComMsg->SetBuffer( pBuf, usDataLength + M_DEFAULT_RESERVED );
    pComMsg->SetDataPtr( pData );
    pComMsg->SetDataLength( usDataLength );

    if ( false == CComEntity::PostEntityMessage( pComMsg ) )
    {
logMsg("failed to post message in UTBridge\n",0,0,0,0,0,0);
        pComMsg->Destroy();  // both ComMessage and pBuf will be deallocated
    }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CUTBridge::EBFreeMsgCallBack

DESCRIPTION:
    Driver�ͷ�ComMessage�Ļص�����

ARGUMENTS:
    UINT32 :ComMessage ptr.

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CUTBridge::EBFreeMsgCallBack (UINT32 param)
{
logMsg("Free ComMsg from Ethernet Driver\n",0,0,0,0,0,0);
    ((CComMessage*)param)->Destroy();
    return;
}



