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

//禁用Winsock 1
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

//任务实例指针的初始化
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
    Tunnel任务消息处理函数

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    bool:true or false,FrameWork根据返回值决定是否做PostProcess()

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
                //收到SNOOP发送的隧道创建请求
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
                //收到SNOOP发送的隧道拆除请求
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
                //收到SNOOP发送的隧道同步请求
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
                //收到SNOOP发送的隧道改变Anchor请求
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
                //收到SNOOP发送的心跳应答
                CTunnelResponseBase respMsg(msg);
                respMsg.SetBTS(respMsg.GetDstBtsID());
                ForwardToSOCKET( respMsg );
                }
            break;

        case MSGID_TUNNEL_ESTABLISH_RESP:
                {
                //收到SNOOP发送的隧道创建应答
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
                //收到SNOOP发送的隧道终结应答
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
                //收到SNOOP发送的隧道同步应答
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
                //收到SNOOP发送的隧道改变Anchor应答
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
                //收到TCR接收到的隧道消息
                ReceiveFromTCR( msg );
            break;

        case MSGID_TIMER_TUNNEL:
                //Transaction超时
                ProcTimeOut( msg );
            break;
        case MSGID_FT_MODIFY_BTSPUBIP:
               {
                //发送PubIP&port更新消息
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
    CTunnel构造函数

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
    CTunnel析够函数

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
    打印Tunnel任务转发表索引数和空闲转发表信息

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
    //等showStatus信号量
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
    //释放showStatus信号量
    RELEASE();
#endif
    return ;
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::Initialize

DESCRIPTION:
    创建Message Queue;创建Socket

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
    初始化空闲控制块链表m_listFreeCB

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
    插入空闲控制块下标到链表m_listFreeCB尾部

ARGUMENTS:
    usIdx:表项下标

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
    从空闲链表m_listFreeCB取空闲控制块下标;(从链表头部取)

ARGUMENTS:
    NULL

RETURN VALUE:
    usIdx:表项下标

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
        //下标错误
        LOG1( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_CB_INDEX_ERR ), "Err! Free Tunnel CB Index[%d] Err. ", usIdx );
        return M_DATA_INDEX_ERR;
        }

    return usIdx;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::BPtreeAdd

DESCRIPTION:
    控制块索引树插入节点；控制块索引树以Mac地址作键值，把控制块
    表项下标插入到索引树

ARGUMENTS:
    Mac:Mac地址
    usIdx:表项下标

RETURN VALUE:
    bool:插入成功/失败

SIDE EFFECTS:
    如果存在重复项，将会返回失败，所以必须在BPtreeAdd之前保证
    没有重复。即首先 BPtreeFind 一下
==============================================================*/
bool CTunnel::BPtreeAdd(CMac& Mac, UINT16 usIdx)
{
    pair<map<CMac, UINT16>::iterator, bool> stPair;

    stPair = m_CBptree.insert( ValType( Mac, usIdx ) );
    /*
     *必须保证不存在重复项，否则将会返回失败
     */
    return stPair.second;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::BPtreeDel

DESCRIPTION:
    控制块索引树删除Mac对应的节点；

ARGUMENTS:
    Mac:Mac地址

RETURN VALUE:
    bool:删除成功/失败

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
    从BPtree搜索Mac地址对应的控制块下标

ARGUMENTS:
    Mac:Mac地址

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTunnel::BPtreeFind(CMac& Mac)
{
    map<CMac, UINT16>::iterator it = m_CBptree.find( Mac );
    
    if ( it != m_CBptree.end() )
        {
        //返回Index.
        return it->second;
        }
    return M_DATA_INDEX_ERR;
}



/*============================================================
MEMBER FUNCTION:
    CTunnel::ProcessTunnelRequest

DESCRIPTION:
    处理SNOOP发送的隧道请求

ARGUMENTS:
    msg:请求消息

RETURN VALUE:
    bool

SIDE EFFECTS:
    隐患:如果网络状态不是很好的情况下,有可能出现以下状况:
    tunnel定时器重发了两次request1，它会陆续收到两个response1.
    收到第一个response1的时候，tTunnel会发送等待发送的请求request2.
    这时候再次收到response1,tTunnel会当作response2处理。
==============================================================*/
bool CTunnel::SendTunnelRequest(CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->SendTunnelRequest()" );
    CMac Mac( ( (CTunnelRequestBase&)msg ).GetMac() );
    UINT16 usIdx = BPtreeFind( Mac );
    if ( M_DATA_INDEX_ERR != usIdx )
        {
        //用户前一个隧道请求还没有处理完
        LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_CB_BUSY ), "Tunnel is busy, waiting..." );
        //把需要发送的tunnel request插到待处理链表的尾部
        TMReqCB *pCB = GetCBbyIdx( usIdx );
        if ( (NULL != pCB) && (NULL != pCB->pComMsgList) )
            {
            LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_CB_BUSY ), "Insert message to the end of the commessage list." );
            CComMessage *p = pCB->pComMsgList;
            while (NULL != p->getNext()) 
                p = p->getNext();
            // NULL == p->getNext();    
            //找到了，最后一个request.
            CComMessage *pComMsg = msg.GetpComMsg();
            pComMsg->AddRef();
            pComMsg->setNext(NULL);
            //插在后面
            p->setNext(pComMsg);
            }
        return true;
        }

    usIdx = GetFreeCBIndexFromList();
    //新建转发表表项
    TMReqCB *pCB = GetCBbyIdx( usIdx );
    if ( NULL == pCB )
        {
        //控制块用光
        LOG( LOG_DEBUG, LOGNO( TUNNEL, EC_TUNNEL_CB_USEDUP ), "No free tunnel CB!" );
        return false;
        }

    ////已经申请到CB.
    BPtreeAdd( Mac, usIdx );

    pCB->Mac=Mac;
    pCB->ucCount = 1;
    pCB->pComMsgList = msg.GetpComMsg();
    pCB->pComMsgList->AddRef();
    //启动定时器
    CTimer *pTimer = StartTunnelTimer( Mac, M_TUNNEL_TIMER_INTERVAL );
    if ( NULL == pTimer )
        {
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Start Tunnel timer fail." );
        BPtreeDel( Mac );
        pCB->Reclaim();
        //插入回空闲链表
        InsertFreeCB( usIdx );

        return false;
        }
    pCB->pTimer = pTimer;
    
    //Socket发送
    return ForwardToSOCKET( msg );
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ForwardToSOCKET

DESCRIPTION:
    往SNOOP任务转发消息

ARGUMENTS:
    msg: TCR收到的Tunnel回应消息
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
        //不用DeleteMessage.
        return false;
        }

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ForwardToSOCKET

DESCRIPTION:
    往SNOOP任务转发消息

ARGUMENTS:
    msg: TCR收到的Tunnel回应消息
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
    Tunnel启动定时器

ARGUMENTS:
    CMac:
    ulMillSecs: 定时器时长(毫秒):

RETURN VALUE:
    CTimer*:返回定时器指针

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
    //创建循环定时器
    CTimer *pTimer = new CTimer( true, ulMillSecs, msgTimerExpire );
    if ( NULL == pTimer )
        {
        ////System Exception!
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Tunnel create timer failed." );

        //删除消息；
        msgTimerExpire.DeleteMessage();
        return NULL;
        }
    if ( false == pTimer->Start() )
        {
        ////System Exception!
        LOG( LOG_WARN, LOGNO( TUNNEL, EC_TUNNEL_SYS_ERR ), "Timer start err." );

        delete pTimer;

        //删除消息；
        msgTimerExpire.DeleteMessage();
        return NULL;
        }
    return pTimer;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ReceiveFromTCR

DESCRIPTION:
    处理TCR任务收到的数据

ARGUMENTS:
    msg:TCR收到的消息

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CTunnel::ReceiveFromTCR(CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->ReceiveFromTCR()" );
    //取PayLoad的MsgId;
    UINT16 usMsgId = ntohs( *(UINT16*)msg.GetDataPtr() );
    switch ( usMsgId )
        {
        case MSGID_TUNNEL_HEARTBEAT:
                //收到其他BTS发来的隧道心跳；
                //转发给SNOOP
                ForwardToSnoop( msg, usMsgId );
            break;

        case MSGID_TUNNEL_ESTABLISH_REQ:
            {
                //收到其他BTS发来的隧道创建请求；
                //转发给SNOOP
                /*当学习到ServingBTS的PublicIP&Port与消息中自带的不一致时，更新*/
                CTunnelRequestBase reqMsg(msg);
                UINT32 ulPubIP   = reqMsg.GetBTSPubIP();
                UINT16 usPubPort = reqMsg.GetBTSPubPort();
                if((ulPubIP != reqMsg.GetSenderBtsIP()) || (usPubPort!= reqMsg.GetSenderPort()))
                {
                    reqMsg.SetSenderBtsIP(ulPubIP);
                    reqMsg.SetSenderPort(usPubPort);
                    //通知tSocket 增加新的PubIP&Port
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
                //收到其他BTS发来的隧道拆除请求；
                //转发给SNOOP
                IncreaseMeasureByOne( MEASURE_RX_TERMINATE_REQ );
                ForwardToSnoop( msg, usMsgId );
            break;

        case MSGID_TUNNEL_SYNC_REQ:
                //收到其他BTS发来的隧道同步请求；
                //转发给SNOOP
                IncreaseMeasureByOne( MEASURE_RX_SYNC_REQ );
                ForwardToSnoop( msg, usMsgId );
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_REQ:
                //收到其他BTS发来的隧道改变Anchor请求；
                //转发给SNOOP
                IncreaseMeasureByOne( MEASURE_RX_CHANGE_ANCHOR_REQ );
                ForwardToSnoop( msg, usMsgId );
            break;

        case MSGID_TUNNEL_HEARTBEAT_RESP:
                //收到其他BTS发来的隧道心跳应答；
                ReceiveTunnelResponse( msg, usMsgId );
            break;

        case MSGID_TUNNEL_ESTABLISH_RESP:
            {
                //收到其他BTS发来的隧道创建回应；
                /*根据消息携带的IP&Port,与NVRAM中保存的比较，不一致时更新*/
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
                //收到其他BTS发来的隧道拆除回应；
                IncreaseMeasureByOne( MEASURE_RX_TERMINATE_RESPONSE );
                ReceiveTunnelResponse( msg, usMsgId );
            break;

        case MSGID_TUNNEL_SYNC_RESP:
                //收到其他BTS发来的隧道同步回应；
                IncreaseMeasureByOne( MEASURE_RX_SYNC_RESPONSE );
                ReceiveTunnelResponse( msg, usMsgId );
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_RESP:
                //收到其他BTS发来的隧道改变Anchor回应；
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
                //不用DeleteMessage.
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
    处理TCR收到的Tunnel回应消息

ARGUMENTS:
    msgTunnelResponse: TCR收到的Tunnel回应消息

RETURN VALUE:
    NULL

SIDE EFFECTS:
    隐患:如果网络状态不是很好的情况下,有可能出现以下状况:
    tunnel定时器重发了两次request1，它会陆续收到两个response1.
    收到第一个response1的时候，tTunnel会发送等待发送的请求request2.
    这时候再次收到response1,tTunnel会当作response2处理。
==============================================================*/
void CTunnel::ReceiveTunnelResponse(CMessage &msg, UINT16 usMsgId)
{
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "->ReceiveTunnelResponse()" );
    CMac Mac = ( (CTunnelResponseBase&)msg ).GetMac();
    UINT16 usIdx = BPtreeFind( Mac );
    TMReqCB *pCB = GetCBbyIdx( usIdx );
    if ( NULL == pCB )
        {
        ////控制块可能超时被删除
        UINT8 strMac[ M_MACADDR_STRLEN ];
        LOG1( LOG_DEBUG, LOGNO( TUNNEL, EC_TUNNEL_NO_CB ), "Find no corresponding CB [MAC:%s]", (int)Mac.str( strMac ) );
        return;
        }

    //把回应消息转发给SNOOP
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

    //接收回应,如果没有等待发送的请求,则删控制块
    //否则，继续发送 等待的请求消息；
    if (NULL != pCB->pComMsgList)
        {
        //继续发送
        LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "Continue to TX request message suspended in CB" );
        if (true == SendSuspendTunnelRequest(pCB))
            return;
        }

    //1.没有等待发送的请求消息
    //2.发送等待的请求消息失败
    //删控制块
    LOG( LOG_DEBUG3, LOGNO( TUNNEL, EC_TUNNEL_NORMAL ), "No message suspend, release tunnel CB" );
    BPtreeDel( Mac );
    pCB->Reclaim();
    //插入回空闲链表
    InsertFreeCB( usIdx );
    return;
}




/*============================================================
MEMBER FUNCTION:
    CTunnel::SendSuspendTunnelRequest

DESCRIPTION:
    发送被挂起的隧道请求消息

ARGUMENTS:
    msgTunnelResponse: TCR收到的Tunnel回应消息

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
    //启动定时器
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
    
    //Socket发送
    return ForwardToSOCKET( pCB->pComMsgList);
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ForwardToSnoop

DESCRIPTION:
    往SNOOP任务转发消息

ARGUMENTS:
    msg: TCR收到的Tunnel回应消息
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
        //不用DeleteMessage.
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTunnel::ProcTimeOut

DESCRIPTION:
    处理Tunnel任务的定时器超时消息
    等待发送的request消息，不发送!!

ARGUMENTS:
    msgTimerExpire: 超时消息

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
        //重发消息；
        pCB->ucCount += 1;
        //对端bts异常，等待，依次等待的时间增加20,40,60secs
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
            //不 addref.让消息发送出去后释放掉
            }

        ForwardToSOCKET(pCB->pComMsgList);
        }
    else
        {
        //重发结束;
        //删控制块
        BPtreeDel( Mac );
        //删除保存的commessage.
        pCB->Reclaim();
        //插入回空闲链表
        InsertFreeCB( usIdx );
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    TunnelShow

DESCRIPTION:
    用于Tornado Shell上调用执行

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
