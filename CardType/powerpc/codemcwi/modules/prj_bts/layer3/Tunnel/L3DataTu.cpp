/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnel.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   06/26/07   xiao weifang  fix the problem of tunnel busy.
 *   11/17/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

//����Winsock 1
#define _WINSOCKAPI_

#include <string.h>

#include "Object.h"
#include "biztask.h"
#include "MsgQueue.h"
#include "Message.h"
#include "taskDef.h"
#include "LogArea.h"
#include "Timer.h"
#include <taskLib.h>
#include "L3DataCommon.h"
#include "L3DataMsgId.h"
#include "L3DataTunnel.h"
#include "L3DataTunnelErrCode.h"
#include "L3DataTunnelRequestBase.h"
#include "L3DataTunnelResponseBase.h"
#include "L3DataAssert.h"
#include "L3DataNotifyBTSPubIP.h"


typedef map<CMac, UINT16>::value_type ValType;

//����ʵ��ָ��ĳ�ʼ��
CTunnel* CTunnel::s_ptaskTunnel = NULL;

extern "C" 
{
int  bspGetBtsPubIp();
int  bspGetBtsID();
int  bspGetBtsPubPort();
void bspSetBtsPubIp(int);
void bspSetBtsPubPort(int);
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ProcessMessage

DESCRIPTION:
    Tunnel������Ϣ������

ARGUMENTS:
    CMessage: ��Ϣ

RETURN VALUE:
    bool:true or false,FrameWork���ݷ���ֵ�����Ƿ���PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
bool CTunnel::ProcessMessage(CMessage &msg)
{
    UINT16 usMsgId = msg.GetMessageId();
    switch ( usMsgId )
        {
        /////////TUNNEL REQUEST/////////
        case MSGID_TUNNEL_HEARTBEAT:
                {
                CTunnelRequestBase reqMsg(msg);
                reqMsg.SetBTS(reqMsg.GetDstBtsID());
                SendTunnelRequest( reqMsg );
                }
            break;

        case MSGID_TUNNEL_ESTABLISH_REQ:
                {
                //�յ�SNOOP���͵������������
                CTunnelRequestBase reqMsg(msg);
                reqMsg.SetBTS(reqMsg.GetDstBtsID());
                if ( true == SendTunnelRequest( reqMsg ) )
                    {
                    IncreaseMeasureByOne( MEASURE_TX_ESTABLISH_REQ );
                    }
                }
            break;

        case MSGID_TUNNEL_TERMINATE_REQ:
                {
                //�յ�SNOOP���͵�����������
                CTunnelRequestBase reqMsg(msg);
                reqMsg.SetBTS(reqMsg.GetDstBtsID());
                    //wangwenhua add 20080605
                CMac Mac( ( (CTunnelRequestBase&)reqMsg ).GetMac() );
                UINT16 usIdx = BPtreeFind( Mac );
                TMReqCB *pCB = GetCBbyIdx( usIdx );        
                if(NULL!=pCB)
                {
                 BPtreeDel( Mac );        
                 pCB->Reclaim();       
                 InsertFreeCB( usIdx );
                 }
                if ( true == SendTunnelRequest( reqMsg ) )
                    {
                    IncreaseMeasureByOne( MEASURE_TX_TERMINATE_REQ );
                    }
                }
            break;
             case MSGID_TUNNEL_DELETE_TIMER:
            //wangwenhua add 20080605
            {
           
            LOG( LOG_DEBUG3, LOGNO( TUNNEL, MSGID_TUNNEL_DELETE_TIMER ), 
                   "TMSGID_TUNNEL_DELETE_TIMER." );
            CTunnelRequestBase reqMsg(msg);
     
            CMac Mac(((CTunnelRequestBase&)reqMsg).GetMac());
            UINT16 usIdx = BPtreeFind( Mac );
            TMReqCB *pCB = GetCBbyIdx( usIdx );        
            if(NULL!= pCB)
            {
                BPtreeDel( Mac );        
                pCB->Reclaim();       
                InsertFreeCB( usIdx );
            }
        
            }
            break;

        case MSGID_TUNNEL_SYNC_REQ:
                {
                //�յ�SNOOP���͵����ͬ������
                CTunnelRequestBase reqMsg(msg);
                reqMsg.SetBTS(reqMsg.GetDstBtsID());
                if ( true == SendTunnelRequest( reqMsg ) )
                    {
                    IncreaseMeasureByOne( MEASURE_TX_SYNC_REQ );
                    }
                }
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_REQ:
                {
                //�յ�SNOOP���͵�����ı�Anchor����
                CTunnelRequestBase reqMsg(msg);
                reqMsg.SetBTS(reqMsg.GetDstBtsID());
                if ( true == SendTunnelRequest( reqMsg ) )
                    {
                    IncreaseMeasureByOne( MEASURE_TX_CHANGE_ANCHOR_REQ );
                    }
                }
            break;

        /////////TUNNEL RESPONSE/////////
        case MSGID_TUNNEL_HEARTBEAT_RESP:
                {
                //�յ�SNOOP���͵�����Ӧ��
                CTunnelResponseBase respMsg(msg);
                respMsg.SetBTS(respMsg.GetDstBtsID());
                ForwardToSOCKET( respMsg );
                }
            break;

        case MSGID_TUNNEL_ESTABLISH_RESP:
                {
                //�յ�SNOOP���͵��������Ӧ��
                CTunnelResponseBase respMsg(msg);
                respMsg.SetBTS(respMsg.GetDstBtsID());
                if ( true == ForwardToSOCKET( respMsg ) )
                    {
                    IncreaseMeasureByOne( MEASURE_TX_ESTABLISH_RESPONSE );
                    }
                }
            break;

        case MSGID_TUNNEL_TERMINATE_RESP:
                {
                //�յ�SNOOP���͵�����ս�Ӧ��
                CTunnelResponseBase respMsg(msg);
                respMsg.SetBTS(respMsg.GetDstBtsID());
                //wangwenhua add 20080606
                CMac Mac( ( (CTunnelResponseBase&)respMsg ).GetMac() );
                UINT16 usIdx = BPtreeFind( Mac );
                TMReqCB *pCB = GetCBbyIdx( usIdx );   
                if(NULL!=pCB)
                {
                    BPtreeDel( Mac );        
                    pCB->Reclaim();       
                    InsertFreeCB( usIdx );
                 }
                if ( true == ForwardToSOCKET( respMsg ) )
                    {
                    IncreaseMeasureByOne( MEASURE_TX_TERMINATE_RESPONSE );
                    }
                }
            break;

        case MSGID_TUNNEL_SYNC_RESP:
                {
                //�յ�SNOOP���͵����ͬ��Ӧ��
                CTunnelResponseBase respMsg(msg);
                respMsg.SetBTS(respMsg.GetDstBtsID());
                
                if ( true == ForwardToSOCKET( respMsg ) )
                    {
                    IncreaseMeasureByOne( MEASURE_TX_SYNC_RESPONSE );
                    }
                }
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_RESP:
                {
                //�յ�SNOOP���͵�����ı�AnchorӦ��
                CTunnelResponseBase respMsg(msg);
                respMsg.SetBTS(respMsg.GetDstBtsID());
                
                if ( true == ForwardToSOCKET( respMsg ) )
                    {
                    IncreaseMeasureByOne( MEASURE_TX_CHANGE_ANCHOR_RESPONSE );
                    }
                }
            break;

        //////////////////////////////
        case MSGID_TUNNEL_MSGFROM_TCR:
                //�յ�TCR���յ��������Ϣ
                ReceiveFromTCR( msg );
            break;

        case MSGID_TIMER_TUNNEL:
                //Transaction��ʱ
                ProcTimeOut( msg );
            break;
        case MSGID_FT_MODIFY_BTSPUBIP:
               {
                //����PubIP&port������Ϣ
                LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), 
                   "TUNNEL Send Modify PubIP&Port message." );
                ForwardToSOCKET( msg );
               }
            break;

        default:
                LOG1( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_UNEXPECTED_MSGID ), 
                   "TUNNEL receive unexpected message[Id: 0x%x].", usMsgId );
            break;
        }

    //destroy message.
    //pComMsg->Destroy();

#ifdef __WIN32_SIM__
    showStatus();
#endif
    return true;
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::CTunnel

DESCRIPTION:
    CTunnel���캯��

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTunnel::CTunnel()
{
    LOG( LOG_DEBUG, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "CTunnel::CTunnel()" );

#ifndef NDEBUG
    if ( !Construct( CObject::M_OID_TUNNEL ) )
        {
        LOG( LOG_SEVERE, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "ERROR!!!CTunnel::CTunnel()% Construct failed." );
        }
#endif

    memset( m_szName, 0, sizeof( m_szName ) );
    memcpy( m_szName, M_TASK_TUNNEL_TASKNAME, strlen( M_TASK_TUNNEL_TASKNAME ) );
    m_uPriority     = M_TP_L3TUNNEL;
    m_uOptions      = M_TASK_TUNNEL_OPTION;
    m_uStackSize    = M_TASK_TUNNEL_STACKSIZE;
    m_iMsgQMax      = M_TASK_TUNNEL_MAXMSG;
    m_iMsgQOption   = M_TASK_TUNNEL_MSGOPTION;

    memset( m_CB, 0, sizeof( m_CB ) );
    InitFreeCBList();
    //m_Socket= 0;
    memset( m_aulMeasure, 0, sizeof( m_aulMeasure ) );
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::~CTunnel

DESCRIPTION:
    CTunnel��������

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTunnel::~CTunnel()
{
    LOG( LOG_DEBUG, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "CTunnel::~CTunnel" );

#ifndef NDEBUG
    if ( !Destruct( CObject::M_OID_TUNNEL ) )
        {
        LOG( LOG_SEVERE, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "ERROR!!!CTunnel::~CTunnel failed." );
        }
#endif
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::GetInstance

DESCRIPTION:
    Get CTunnel Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTunnel* 

SIDE EFFECTS:
    none
==============================================================*/
CTunnel* CTunnel::GetInstance()
{
    if ( NULL == s_ptaskTunnel )
        {
        s_ptaskTunnel = new CTunnel;
        }
    return s_ptaskTunnel;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::showStatus

DESCRIPTION:
    ��ӡTunnel����ת�����������Ϳ���ת������Ϣ

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTunnel::showStatus()
{
#ifdef __WIN32_SIM__
    //��showStatus�ź���
    WAIT();
#endif
    printf( "\r\n*************************" );
    printf( "\r\n*Tunnel Task Attributes *" );
    printf( "\r\n*************************" );
    printf( "\r\n%-20s: %d", "Task   stack   size",  M_TASK_TUNNEL_STACKSIZE );
    printf( "\r\n%-20s: %d", "Task   Max  messages", M_TASK_TUNNEL_MAXMSG );
    printf( "\r\n" );

    //Free CB
    printf( "\r\n%-20s: %-5d", "Free CBs", m_listFreeCB.size() );

    //BPtree
    if ( true == m_CBptree.empty() )
        {
        printf( "\r\n%-20s: %s", "Busy CBs", "0" );
        }
    else
        {
        printf( "\r\n%-20s: %-5d", "Busy CBs", m_CBptree.size() );
        }

    printf( "\r\n" );
    printf( "\r\n***************************************" );
    printf( "\r\n*Performance Measurement Status       *" );
    printf( "\r\n***************************************" );
    for ( UINT8 type = MEASURE_TX_ESTABLISH_REQ; type < MEASURE_TUNNEL_MAX; ++type )
        {
        printf( "\r\n%-20s: %d", strTunnelMeasureType[ type ], m_aulMeasure[ type ] );
        }

    printf( "\r\n" );
#ifdef __WIN32_SIM__
    //�ͷ�showStatus�ź���
    RELEASE();
#endif
    return ;
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::Initialize

DESCRIPTION:
    ����Message Queue;����Socket

ARGUMENTS:
    NULL

RETURN VALUE:
    true of false 

SIDE EFFECTS:
    none
==============================================================*/
bool CTunnel::Initialize()
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "CTunnel::Initialize" );

#ifdef __WIN32_SIM__
    ::WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
#endif
/*
    //create socket
    m_Socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_IP );
    if ( M_DATA_INVALID_SOCKET == m_Socket )
        {
#ifdef __WIN32_SIM__
        ::WSACleanup();
#endif
        LOG( LOG_SEVERE, LOGNO( TUNNEL, EC_TUNNEL_SOCKET_ERR ), "Tunnel socket create err." );

        return false;
        }
*/
    UINT8 ucInit = CBizTask::Initialize();
    if ( false == ucInit )
        {
        /*
#ifdef __WIN32_SIM__
        ::closesocket( m_Socket );
        ::WSACleanup();
#else
        ::close( m_Socket );
#endif
        */ 
        return false;
        }

    return true;
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::InitFreeFTList

DESCRIPTION:
    ��ʼ�����п��ƿ�����m_listFreeCB

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTunnel::InitFreeCBList()
{
    m_listFreeCB.clear();
    for ( UINT16 usIdx = 0; usIdx < M_TUNNEL_REQUEST_CB_NUM; ++usIdx )
        {
        m_listFreeCB.push_back( usIdx );
        }
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::InsertFreeFTList

DESCRIPTION:
    ������п��ƿ��±굽����m_listFreeCBβ��

ARGUMENTS:
    usIdx:�����±�

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTunnel::InsertFreeCB(UINT16 usIdx )
{
    if( usIdx < M_TUNNEL_REQUEST_CB_NUM )
        {
        m_listFreeCB.push_back( usIdx );
        }
    else
        {
        DATA_assert( 0 );
        }
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::GetFreeCBIndexFromList

DESCRIPTION:
    �ӿ�������m_listFreeCBȡ���п��ƿ��±�;(������ͷ��ȡ)

ARGUMENTS:
    NULL

RETURN VALUE:
    usIdx:�����±�

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnel::GetFreeCBIndexFromList()
{
    if ( true == m_listFreeCB.empty() )
        {
        return M_DATA_INDEX_ERR;
        }

    UINT16 usIdx = *m_listFreeCB.begin();
    m_listFreeCB.pop_front();

    if ( M_TUNNEL_REQUEST_CB_NUM <= usIdx )
        {
        //�±����
        LOG1( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_CB_INDEX_ERR ), "Err! Free Tunnel CB Index[%d] Err. ", usIdx );
        return M_DATA_INDEX_ERR;
        }

    return usIdx;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::BPtreeAdd

DESCRIPTION:
    ���ƿ�����������ڵ㣻���ƿ���������Mac��ַ����ֵ���ѿ��ƿ�
    �����±���뵽������

ARGUMENTS:
    Mac:Mac��ַ
    usIdx:�����±�

RETURN VALUE:
    bool:����ɹ�/ʧ��

SIDE EFFECTS:
    ��������ظ�����᷵��ʧ�ܣ����Ա�����BPtreeAdd֮ǰ��֤
    û���ظ��������� BPtreeFind һ��
==============================================================*/
bool CTunnel::BPtreeAdd(CMac& Mac, UINT16 usIdx)
{
    pair<map<CMac, UINT16>::iterator, bool> stPair;

    stPair = m_CBptree.insert( ValType( Mac, usIdx ) );
    /*
     *���뱣֤�������ظ�����򽫻᷵��ʧ��
     */
    return stPair.second;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::BPtreeDel

DESCRIPTION:
    ���ƿ�������ɾ��Mac��Ӧ�Ľڵ㣻

ARGUMENTS:
    Mac:Mac��ַ

RETURN VALUE:
    bool:ɾ���ɹ�/ʧ��

SIDE EFFECTS:
    none
==============================================================*/
bool CTunnel::BPtreeDel(CMac& Mac)
{
    map<CMac, UINT16>::iterator it;
    
    if ( ( it = m_CBptree.find( Mac ) ) != m_CBptree.end() )
        {
        //find, and erase;
        m_CBptree.erase( it );
        }
    //not find
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::BPtreeFind

DESCRIPTION:
    ��BPtree����Mac��ַ��Ӧ�Ŀ��ƿ��±�

ARGUMENTS:
    Mac:Mac��ַ

RETURN VALUE:
    usIdx:�����±�

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnel::BPtreeFind(CMac& Mac)
{
    map<CMac, UINT16>::iterator it = m_CBptree.find( Mac );
    
    if ( it != m_CBptree.end() )
        {
        //����Index.
        return it->second;
        }
    return M_DATA_INDEX_ERR;
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::ProcessTunnelRequest

DESCRIPTION:
    ����SNOOP���͵��������

ARGUMENTS:
    msg:������Ϣ

RETURN VALUE:
    bool

SIDE EFFECTS:
    ����:�������״̬���Ǻܺõ������,�п��ܳ�������״��:
    tunnel��ʱ���ط�������request1������½���յ�����response1.
    �յ���һ��response1��ʱ��tTunnel�ᷢ�͵ȴ����͵�����request2.
    ��ʱ���ٴ��յ�response1,tTunnel�ᵱ��response2����
==============================================================*/
bool CTunnel::SendTunnelRequest(CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->SendTunnelRequest()" );
    CMac Mac( ( (CTunnelRequestBase&)msg ).GetMac() );
    UINT16 usIdx = BPtreeFind( Mac );
    if ( M_DATA_INDEX_ERR != usIdx )
        {
        //�û�ǰһ���������û�д�����
        LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_CB_BUSY ), "Tunnel is busy, waiting..." );
        //����Ҫ���͵�tunnel request�嵽�����������β��
        TMReqCB *pCB = GetCBbyIdx( usIdx );
        if ( (NULL != pCB) && (NULL != pCB->pComMsgList) )
            {
            LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_CB_BUSY ), "Insert message to the end of the commessage list." );
            CComMessage *p = pCB->pComMsgList;
            while (NULL != p->getNext()) 
                p = p->getNext();
            // NULL == p->getNext();    
            //�ҵ��ˣ����һ��request.
            CComMessage *pComMsg = msg.GetpComMsg();
            pComMsg->AddRef();
            pComMsg->setNext(NULL);
            //���ں���
            p->setNext(pComMsg);
            }
        return true;
        }

    usIdx = GetFreeCBIndexFromList();
    //�½�ת�������
    TMReqCB *pCB = GetCBbyIdx( usIdx );
    if ( NULL == pCB )
        {
        //���ƿ��ù�
        LOG( LOG_DEBUG, LOGNO( TUNNEL, EC_TUNNEL_CB_USEDUP ), "No free tunnel CB!" );
        return false;
        }

    ////�Ѿ����뵽CB.
    BPtreeAdd( Mac, usIdx );

    pCB->Mac=Mac;
    pCB->ucCount = 1;
    pCB->pComMsgList = msg.GetpComMsg();
    pCB->pComMsgList->AddRef();
    //������ʱ��
    CTimer *pTimer = StartTunnelTimer( Mac, M_TUNNEL_TIMER_INTERVAL );
    if ( NULL == pTimer )
        {
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Start Tunnel timer fail." );
        BPtreeDel( Mac );
        pCB->Reclaim();
        //����ؿ�������
        InsertFreeCB( usIdx );

        return false;
        }
    pCB->pTimer = pTimer;
    
    //Socket����
    return ForwardToSOCKET( msg );
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ForwardToSOCKET

DESCRIPTION:
    ��SNOOP����ת����Ϣ

ARGUMENTS:
    msg: TCR�յ���Tunnel��Ӧ��Ϣ
    usMsgId:

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
bool CTunnel::ForwardToSOCKET(CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->ForwardToSOCKET()" );
   // msg.SetMessageId( usMsgId );
    msg.SetSrcTid( M_TID_TUNNEL );
    msg.SetDstTid( M_TID_EMSAGENTTX);
    if ( false == msg.Post() )
        {
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Tunnel fail to forward message to SOCKET!" );
        //����DeleteMessage.
        return false;
        }

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ForwardToSOCKET

DESCRIPTION:
    ��SNOOP����ת����Ϣ

ARGUMENTS:
    msg: TCR�յ���Tunnel��Ӧ��Ϣ
    usMsgId:

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
bool CTunnel::ForwardToSOCKET(CComMessage *pComMsg)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->ForwardToSOCKET()" );
   // msg.SetMessageId( usMsgId );
   
    pComMsg->SetSrcTid( M_TID_TUNNEL );
    pComMsg->SetDstTid( M_TID_EMSAGENTTX);
    CComEntity::PostEntityMessage( pComMsg );

    return true;
}

/*============================================================
MEMBER FUNCTION:
    CTunnel::StartTunnelTimer

DESCRIPTION:
    Tunnel������ʱ��

ARGUMENTS:
    CMac:
    ulMillSecs: ��ʱ��ʱ��(����):

RETURN VALUE:
    CTimer*:���ض�ʱ��ָ��

SIDE EFFECTS:
    none
==============================================================*/
CTimer* CTunnel::StartTunnelTimer( CMac &Mac, UINT32 ulMillSecs )
{
    CTunnelTimerExpire msgTimerExpire;
    if ( false == msgTimerExpire.CreateMessage( *this ) )
        {
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Create tunnel timer message failed." );
        return NULL;
        }
    msgTimerExpire.SetMac( Mac );
    msgTimerExpire.SetDstTid( M_TID_TUNNEL );
    //����ѭ����ʱ��
    CTimer *pTimer = new CTimer( true, ulMillSecs, msgTimerExpire );
    if ( NULL == pTimer )
        {
        ////System Exception!
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Tunnel create timer failed." );

        //ɾ����Ϣ��
        msgTimerExpire.DeleteMessage();
        return NULL;
        }
    if ( false == pTimer->Start() )
        {
        ////System Exception!
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Timer start err." );

        delete pTimer;

        //ɾ����Ϣ��
        msgTimerExpire.DeleteMessage();
        return NULL;
        }
    return pTimer;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ReceiveFromTCR

DESCRIPTION:
    ����TCR�����յ�������

ARGUMENTS:
    msg:TCR�յ�����Ϣ

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CTunnel::ReceiveFromTCR(CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->ReceiveFromTCR()" );
    //ȡPayLoad��MsgId;
    UINT16 usMsgId = ntohs( *(UINT16*)msg.GetDataPtr() );
    switch ( usMsgId )
        {
        case MSGID_TUNNEL_HEARTBEAT:
                //�յ�����BTS���������������
                //ת����SNOOP
                ForwardToSnoop( msg, usMsgId );
            break;

        case MSGID_TUNNEL_ESTABLISH_REQ:
            {
                //�յ�����BTS�����������������
                //ת����SNOOP
                /*��ѧϰ��ServingBTS��PublicIP&Port����Ϣ���Դ��Ĳ�һ��ʱ������*/
                CTunnelRequestBase reqMsg(msg);
                UINT32 ulPubIP   = reqMsg.GetBTSPubIP();
                UINT16 usPubPort = reqMsg.GetBTSPubPort();
                if((ulPubIP != reqMsg.GetSenderBtsIP()) || (usPubPort!= reqMsg.GetSenderPort()))
                {
                    reqMsg.SetSenderBtsIP(ulPubIP);
                    reqMsg.SetSenderPort(usPubPort);
                    //֪ͨtSocket �����µ�PubIP&Port
                }

                // post a BTS IP update message to tSocket for every request
                CUpdateBTSPubIp mUpdateIp;
                if ( mUpdateIp.CreateMessage(*this) )
                {
                    mUpdateIp.SetBTSID(reqMsg.GetSenderBtsID());
                    mUpdateIp.SetBTSPubIP(ulPubIP);
                    mUpdateIp.SetBTSPubPort(usPubPort);
                    mUpdateIp.SetDstTid( M_TID_EMSAGENTTX);
                    mUpdateIp.SetSrcTid( M_TID_TUNNEL );
                    if ( !mUpdateIp.Post() )
                    {
                        LOG1( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->SendModifyPubIpRequest To tSocket(BTSID=%d)",reqMsg.GetSenderBtsID() );
                        mUpdateIp.DeleteMessage();
                    }
                }
                IncreaseMeasureByOne( MEASURE_RX_ESTABLISH_REQ );
                ForwardToSnoop( msg, usMsgId );
            }
            break;

        case MSGID_TUNNEL_TERMINATE_REQ:
                //�յ�����BTS����������������
                //ת����SNOOP
                IncreaseMeasureByOne( MEASURE_RX_TERMINATE_REQ );
                ForwardToSnoop( msg, usMsgId );
            break;

        case MSGID_TUNNEL_SYNC_REQ:
                //�յ�����BTS���������ͬ������
                //ת����SNOOP
                IncreaseMeasureByOne( MEASURE_RX_SYNC_REQ );
                ForwardToSnoop( msg, usMsgId );
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_REQ:
                //�յ�����BTS����������ı�Anchor����
                //ת����SNOOP
                IncreaseMeasureByOne( MEASURE_RX_CHANGE_ANCHOR_REQ );
                ForwardToSnoop( msg, usMsgId );
            break;

        case MSGID_TUNNEL_HEARTBEAT_RESP:
                //�յ�����BTS�������������Ӧ��
                ReceiveTunnelResponse( msg, usMsgId );
            break;

        case MSGID_TUNNEL_ESTABLISH_RESP:
            {
                //�յ�����BTS���������������Ӧ��
                /*������ϢЯ����IP&Port,��NVRAM�б���ıȽϣ���һ��ʱ����*/
                CTunnelResponseBase respMsg(msg);
                if(bspGetBtsID() == respMsg.GetDstBtsID())
                    {
                    UINT32 ulPubIP = respMsg.GetDstBtsIP();
                    UINT16 usPubPort = respMsg.GetDstPort();
                    if(ulPubIP!=bspGetBtsPubIp())
                        bspSetBtsPubIp(ulPubIP);
                    if(usPubPort!=bspGetBtsPubPort())
                        bspSetBtsPubPort(usPubPort);
                    }
                IncreaseMeasureByOne( MEASURE_RX_ESTABLISH_RESPONSE );
                ReceiveTunnelResponse( msg, usMsgId );
            }
            break;

        case MSGID_TUNNEL_TERMINATE_RESP:
                //�յ�����BTS��������������Ӧ��
                IncreaseMeasureByOne( MEASURE_RX_TERMINATE_RESPONSE );
                ReceiveTunnelResponse( msg, usMsgId );
            break;

        case MSGID_TUNNEL_SYNC_RESP:
                //�յ�����BTS���������ͬ����Ӧ��
                IncreaseMeasureByOne( MEASURE_RX_SYNC_RESPONSE );
                ReceiveTunnelResponse( msg, usMsgId );
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_RESP:
                //�յ�����BTS����������ı�Anchor��Ӧ��
                IncreaseMeasureByOne( MEASURE_RX_CHANGE_ANCHOR_RESPONSE );
                ReceiveTunnelResponse( msg, usMsgId );
            break;
        case MSGID_FT_MODIFY_BTSPUBIP:
            {
                LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->ForwardToEB()" );
                msg.SetMessageId( MSGID_FT_MODIFY_BTSPUBIP );
                msg.SetSrcTid( M_TID_TUNNEL );
                msg.SetDstTid( M_TID_EB);
                if ( false == msg.Post() )
                   {
                   LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Tunnel fail to forward message to EB!" );
                //����DeleteMessage.
                   }
            }
            break;
        default:
                LOG1( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_UNEXPECTED_MSGID ), "UTDM receive unexpected message[Id: 0x%x].", usMsgId );
            break;
        }
    return;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ReceiveTunnelResponse

DESCRIPTION:
    ����TCR�յ���Tunnel��Ӧ��Ϣ

ARGUMENTS:
    msgTunnelResponse: TCR�յ���Tunnel��Ӧ��Ϣ

RETURN VALUE:
    NULL

SIDE EFFECTS:
    ����:�������״̬���Ǻܺõ������,�п��ܳ�������״��:
    tunnel��ʱ���ط�������request1������½���յ�����response1.
    �յ���һ��response1��ʱ��tTunnel�ᷢ�͵ȴ����͵�����request2.
    ��ʱ���ٴ��յ�response1,tTunnel�ᵱ��response2����
==============================================================*/
void CTunnel::ReceiveTunnelResponse(CMessage &msg, UINT16 usMsgId)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->ReceiveTunnelResponse()" );
    CMac Mac = ( (CTunnelResponseBase&)msg ).GetMac();
    UINT16 usIdx = BPtreeFind( Mac );
    TMReqCB *pCB = GetCBbyIdx( usIdx );
    if ( NULL == pCB )
        {
        ////���ƿ���ܳ�ʱ��ɾ��
        UINT8 strMac[ M_MACADDR_STRLEN ];
        LOG1( LOG_DEBUG, LOGNO( TUNNEL, EC_TUNNEL_NO_CB ), "Find no corresponding CB [MAC:%s]", (int)Mac.str( strMac ) );
        return;
        }

    //�ѻ�Ӧ��Ϣת����SNOOP
    ForwardToSnoop( msg, usMsgId );

    //
    DATA_assert(NULL != pCB->pComMsgList);
    if (NULL != pCB->pComMsgList)
        {
        LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "Release the request message saved in CB" );
        CComMessage *p = pCB->pComMsgList;
        pCB->pComMsgList = p->getNext();
        p->Destroy();
        }

    //���ջ�Ӧ,���û�еȴ����͵�����,��ɾ���ƿ�
    //���򣬼������� �ȴ���������Ϣ��
    if (NULL != pCB->pComMsgList)
        {
        //��������
        LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "Continue to TX request message suspended in CB" );
        if (true == SendSuspendTunnelRequest(pCB))
            return;
        }

    //1.û�еȴ����͵�������Ϣ
    //2.���͵ȴ���������Ϣʧ��
    //ɾ���ƿ�
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "No message suspend, release tunnel CB" );
    BPtreeDel( Mac );
    pCB->Reclaim();
    //����ؿ�������
    InsertFreeCB( usIdx );
    return;
}




/*============================================================
MEMBER FUNCTION:
    CTunnel::SendSuspendTunnelRequest

DESCRIPTION:
    ���ͱ���������������Ϣ

ARGUMENTS:
    msgTunnelResponse: TCR�յ���Tunnel��Ӧ��Ϣ

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
bool CTunnel::SendSuspendTunnelRequest(TMReqCB *pCB)
{
    if (NULL == pCB)
        return false;

    pCB->ucCount = 1;
    //������ʱ��
    CTimer *pTimer = StartTunnelTimer( pCB->Mac, M_TUNNEL_TIMER_INTERVAL );
    if ( NULL == pTimer )
        {
        return false;
        }
    if (NULL != pCB->pTimer)
        {
        pCB->pTimer->Stop();
        delete pCB->pTimer;
        }
    pCB->pTimer = pTimer;
    
    //Socket����
    return ForwardToSOCKET( pCB->pComMsgList);
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ForwardToSnoop

DESCRIPTION:
    ��SNOOP����ת����Ϣ

ARGUMENTS:
    msg: TCR�յ���Tunnel��Ӧ��Ϣ
    usMsgId:

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CTunnel::ForwardToSnoop(CMessage &msg, UINT16 usMsgId)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->ForwardToSnoop()" );
    msg.SetMessageId( usMsgId );
    msg.SetSrcTid( M_TID_TUNNEL );
    msg.SetDstTid( M_TID_SNOOP );
    if ( false == msg.Post() )
        {
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Tunnel fail to forward message to SNOOP!" );
        //����DeleteMessage.
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ProcTimeOut

DESCRIPTION:
    ����Tunnel����Ķ�ʱ����ʱ��Ϣ
    �ȴ����͵�request��Ϣ��������!!

ARGUMENTS:
    msgTimerExpire: ��ʱ��Ϣ

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTunnel::ProcTimeOut(const CTunnelTimerExpire &msgTimerExpire)
{
    CMac Mac = msgTimerExpire.GetMac();
    UINT16 usIdx = BPtreeFind( Mac );
    TMReqCB *pCB = GetCBbyIdx( usIdx );
    if ( NULL == pCB )
        {
        /////
        return;
        }

    UINT8 ucCount = pCB->ucCount;
    if ( ucCount <= M_TUNNEL_TIMER_RETRY_COUNT )
        {
        //�ط���Ϣ��
        pCB->ucCount += 1;
        //�Զ�bts�쳣���ȴ������εȴ���ʱ������20,40,60secs
        CTimer *pTimer = pCB->pTimer;
        if (NULL != pTimer)
            {
            pTimer->Stop();
            pTimer->SetInterval(M_TUNNEL_TIMER_INTERVAL*pCB->ucCount);
            pTimer->Start();
            pCB->pComMsgList->AddRef();
            }
        else
            {
            //�� addref.����Ϣ���ͳ�ȥ���ͷŵ�
            }

        ForwardToSOCKET(pCB->pComMsgList);
        }
    else
        {
        //�ط�����;
        //ɾ���ƿ�
        BPtreeDel( Mac );
        //ɾ�������commessage.
        pCB->Reclaim();
        //����ؿ�������
        InsertFreeCB( usIdx );
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    TunnelShow

DESCRIPTION:
    ����Tornado Shell�ϵ���ִ��

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
extern "C" void TunnelShow()
{
    CTunnel *taskTUNNEL = CTunnel::GetInstance();
    taskTUNNEL->showStatus();
}
