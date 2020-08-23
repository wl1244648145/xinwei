/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTDR.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
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

#include "L3DataCommon.h"
#include "L3DataMsgId.h"
#include "L3DataTDR.h"
#include "L3DataTDRErrCode.h"



//任务实例指针的初始化
CTaskTDR* CTaskTDR::s_ptaskTDR = NULL;


/*============================================================
MEMBER FUNCTION:
    CTaskTDR::CTaskTDR

DESCRIPTION:
    CTaskTDR构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTaskTDR::CTaskTDR()
{
    LOG( LOG_DEBUG3, 0, "CTaskTDR::CTaskTDR()" );


    memset( m_szName, 0, sizeof( m_szName ) );
    memcpy( m_szName, M_TASK_TDR_TASKNAME, strlen( M_TASK_TDR_TASKNAME ) );
    m_uPriority     = M_TP_L3TDR;
    m_uOptions      = M_TASK_TDR_OPTION;
    m_uStackSize    = M_TASK_TDR_STACKSIZE;

    m_EtherIpSocket = 0;
    memset( m_aulMeasure, 0, sizeof( m_aulMeasure ) );

    //初始化缓冲池.
    memset( (void*)m_aucBufferPool, 0, sizeof( m_aucBufferPool ) );
    //初始化链表
    InitFreeBufferList();

    m_usUsedBuffer    = 0;
}



/*============================================================
MEMBER FUNCTION:
    CTaskTDR::~CTaskTDR

DESCRIPTION:
    CTaskTDR析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTaskTDR::~CTaskTDR()
{
    LOG( LOG_DEBUG3, 0, "CTaskTDR::~CTaskTDR" );

}


/*============================================================
MEMBER FUNCTION:
    CTaskTDR::GetInstance

DESCRIPTION:
    Get TDR Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTaskTDR* 

SIDE EFFECTS:
    none
==============================================================*/
CTaskTDR* CTaskTDR::GetInstance()
{
    if ( NULL == s_ptaskTDR )
        {
        s_ptaskTDR = new CTaskTDR;
        }
    return s_ptaskTDR;
}


/*============================================================
MEMBER FUNCTION:
    CTaskTDR::showStatus

DESCRIPTION:
    打印TaskTDR任务转发表索引数和空闲转发表信息

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTaskTDR::showStatus()
{
#ifdef __WIN32_SIM__
    //等showStatus信号量
    WAIT();
#endif
    printf( "\r\n*************************" );
    printf( "\r\n*TDR  Task  Attributes  *" );
    printf( "\r\n*************************" );
    printf( "\r\n%-20s: %d", "Task   stack   size",  M_TASK_TDR_STACKSIZE );
    printf( "\r\n" );

    //Free Buffer
    printf( "\r\n%-20s: %-5d", "Total Buffers", M_TDR_BUFFER_NUM );
    printf( "\r\n%-20s: %-5d", "Used Buffers",  m_usUsedBuffer );
    printf( "\r\n%-20s: %-5d", "Free Buffers",  m_listFreeBuffer.size() );

    printf( "\r\n" );
    printf( "\r\n***************************************" );
    printf( "\r\n*Performance Measurement Status       *" );
    printf( "\r\n***************************************" );
    for ( UINT8 type = MEASURE_TDR_SOCK_ERR; type < MEASURE_TDR_MAX; ++type )
        {
        printf( "\r\n%-20s: %d", strTDRMeasureType[ type ], m_aulMeasure[ type ] );
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
    CTaskTDR::Initialize

DESCRIPTION:
    创建Socket，设置选项，绑定Socket.

ARGUMENTS:
    NULL

RETURN VALUE:
    true of false 

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskTDR::Initialize()
{
    LOG( LOG_DEBUG3, 0, "CTaskTDR::Initialize" );

#ifdef __WIN32_SIM__
    ::WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
#endif
    //create socket
    m_EtherIpSocket = ::socket( AF_INET, SOCK_RAW, 0 );     /*IPPROTO_RAW只能用于send*/
    if ( M_DATA_INVALID_SOCKET == m_EtherIpSocket )
        {
#ifdef __WIN32_SIM__
        ::WSACleanup();
#endif
        LOG( LOG_SEVERE, 0, "TDR create EtherIP socket err." );
        return false;
        }

    struct sockaddr_in myaddr;
#ifdef __WIN32_SIM__
    memset( (char*)&myaddr, 0, sizeof( struct sockaddr_in ) );
    myaddr.sin_addr.S_un.S_addr = htonl( GetHostIpAddr() );
#else
    bzero( (char*)&myaddr, sizeof( struct sockaddr_in ) ); 
    myaddr.sin_addr.s_addr = htonl( GetHostIpAddr() );
    //长度?
#endif
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons( 0 );

    if ( M_DATA_SOCKET_ERR == ::bind( m_EtherIpSocket, (struct sockaddr*)&myaddr, sizeof( struct sockaddr ) ) )
        {
#ifdef __WIN32_SIM__
        ::closesocket( m_EtherIpSocket );
        ::WSACleanup();
#else
        ::close( m_EtherIpSocket );
#endif
        LOG( LOG_SEVERE, 0, "TDR bind EtherIP socket err." );
        return false;
        }
#ifdef __WIN32_SIM__
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)
    UINT32 ulValue = 1;
    ioctlsocket( m_EtherIpSocket, SIO_RCVALL, (u_long*)&ulValue ); 
#endif

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTaskTDR::DeallocateComMessage

DESCRIPTION:
    释放ComMessage.

ARGUMENTS:
    pComMsg

RETURN VALUE:
    true of false

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskTDR::DeallocateComMessage(CComMessage *pComMsg)
{
    LOG(LOG_DEBUG3,0,"CTaskTDR::DeallocateComMessage(CComMessage*)");

    UINT8 *pBuf = (UINT8*)pComMsg->GetBufferPtr();

    //回收缓冲区
    ReclaimFreeBuffer( pBuf );
    pComMsg->DeleteBuffer();

    //其他释放操作
    return CComEntity::DeallocateComMessage( pComMsg );
}


/*============================================================
MEMBER FUNCTION:
    CTaskTDR::MainLoop

DESCRIPTION:
    重载主任务

ARGUMENTS:
    void

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTaskTDR::MainLoop()
{
    struct sockaddr_in from;    /* connector's address information */ 
    SINT32 lFromLen = sizeof( struct sockaddr );
    bool   bBufferUsedUp = false;
    //接收到的Ip头和EtherIp头长度
    UINT8  ucRsvHeaderLen = sizeof( IpHdr ) + sizeof( EtherIpHdr );
    //预留64字节，还要去Ip头和EtherIp头
    UINT8  ucOffset = M_DEFAULT_RESERVED - ucRsvHeaderLen;

    for(;;)
        {
        UINT8 *pBuf = GetBufferFromPool();
        if ( NULL == pBuf )
            {
            //没有空闲缓冲池
            if ( false == bBufferUsedUp )
                {
                ////循环取的时候，只记一次
                bBufferUsedUp = true;
                IncreaseMeasureByOne( MEASURE_TDR_NO_BUFFER );
                }

#ifdef __WIN32_SIM__
//Win32:
            ::Sleep( 100 );//释放CPU
#else
//VxWorks:
            ::taskDelay( 1 );
#endif
            continue;
            }

        //恢复标志
        bBufferUsedUp = false;

        //pBuf指针前移，预留空间
        SINT8 *pDataPtr = (SINT8*)pBuf + ucOffset;     //预留64字节
        UINT32 ulRecvBytes = ::recvfrom( m_EtherIpSocket, 
            pDataPtr, M_TDR_BUFFER_LENGTH - ucOffset,   //长度-42
            0, (struct sockaddr*)&from, &lFromLen );

        if ( M_DATA_SOCKET_ERR == ulRecvBytes )
            {
            //Error.
            IncreaseMeasureByOne( MEASURE_TDR_SOCK_ERR );
            //回收缓冲区
            ReclaimFreeBuffer( pBuf );
            LOG( LOG_WARN,0, "Receive raw socket data err." );
            continue;
            }

        if ( ulRecvBytes < ucRsvHeaderLen )
            {
            //接收的数据不够IP头和EtherIp头的长度
            //不是EtherIp数据，则不处理，继续接收其他报文。
            //回收缓冲区
            ReclaimFreeBuffer( pBuf );
            continue;
            }

        IpHdr *pIp = (IpHdr*)pDataPtr;
       /* if ( M_PROTOCOL_TYPE_ETHERIP != pIp->ucProto )
            {
            //不是EtherIp
            ReclaimFreeBuffer( pBuf );
            continue;
            }*/

        ////收到EtherIp报文，
        //接收到的EtherIp报文统计
        IncreaseMeasureByOne( MEASURE_TDR_ETHERIP_PACKETS );

        UINT32 ulFromBts;
#ifdef __WIN32_SIM__
        ulFromBts = ntohl( from.sin_addr.S_un.S_addr );
#else
        ulFromBts = ntohl( from.sin_addr.s_addr );
        //长度?
#endif
        //实际数据长度必须去Ip头和EtherIp头
        ForwardToEB( pBuf, ulRecvBytes - ucRsvHeaderLen, ulFromBts );

#ifdef __WIN32_SIM__
        showStatus();
#endif
        }
}



/*============================================================
MEMBER FUNCTION:
    CTaskTDR::InitFreeFTList

DESCRIPTION:
    初始化空闲Buffer链表m_listFreeBuffer

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTaskTDR::InitFreeBufferList()
{
    m_listFreeBuffer.clear();
    for ( UINT16 usIdx = 0; usIdx < M_TDR_BUFFER_NUM; ++usIdx )
        {
        m_listFreeBuffer.push_back( m_aucBufferPool[ usIdx ] );
        }
}


/*============================================================
MEMBER FUNCTION:
    CTaskTDR::GetBufferFromPool

DESCRIPTION:
    从空闲Buffer链表m_listFreeBuffer取空闲Buffer;(从链表头部取)

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT8*: 缓冲区指针

SIDE EFFECTS:
    none
==============================================================*/
UINT8* CTaskTDR::GetBufferFromPool()
{
    if ( true == m_listFreeBuffer.empty() )
        {
        return NULL;
        }

    UINT8 *pBuf = *m_listFreeBuffer.begin();
    m_listFreeBuffer.pop_front();

    ++m_usUsedBuffer;

    return pBuf;
}


/*============================================================
MEMBER FUNCTION:
    CTaskTDR::ReclaimFreeBuffer

DESCRIPTION:
    回收缓冲区，插入回空闲缓冲区链表m_listFreeBuffer

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT8*: 缓冲区指针

SIDE EFFECTS:
    none
==============================================================*/
void CTaskTDR::ReclaimFreeBuffer(UINT8 *pBuf)
{
    if ( NULL == pBuf )
        {
        return;
        }

    m_listFreeBuffer.push_back( pBuf );
    m_usUsedBuffer--;
}


/*============================================================
MEMBER FUNCTION:
    CTaskTDR::ForwardToEB

DESCRIPTION:
    往Ether Bridge任务转发消息

ARGUMENTS:
    pBuf:   ComMessage的Buffer指针，包括预留字节 
    ulDataLen: 消息净荷的长度,(已经去Ip头和EtherIp头)
    ulFromBtsAddr:对端的BTS地址

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTaskTDR::ForwardToEB(UINT8 *pBuf, UINT32 ulDataLen, UINT32 ulFromBtsAddr )
{
    LOG( LOG_DEBUG3, 0, "->ForwardToEB()" );
    CComMessage *pComMsg = new( this, 0 )CComMessage;
    if ( NULL == pComMsg )
        {
        //接收到的EtherIp报文统计
        IncreaseMeasureByOne( MEASURE_TDR_NO_BUFFER );
        //回收缓冲区
        ReclaimFreeBuffer( pBuf );
        LOG( LOG_WARN, 0, "Create ComMessage failed." );
        return;
        }

    pComMsg->SetBuffer( (void*)pBuf, M_TDR_BUFFER_LENGTH );
    pComMsg->SetDataPtr( (void*)( pBuf + M_DEFAULT_RESERVED ) );
    pComMsg->SetDataLength( ulDataLen );

    //pComMsg->SetBtsAddr( ulFromBtsAddr );
    pComMsg->SetMessageId( MSGID_TRAFFIC_INGRESS );
    pComMsg->SetDstTid( M_TID_UTDM );
    pComMsg->SetSrcTid( M_TID_TDR );

    if ( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        //释放消息；回收缓冲区
        pComMsg->Destroy();
        LOG( LOG_WARN, 0, "TDR fail to forward message to Ether Bridge!" );
        }

    return;
}

UINT32 CTaskTDR::GetHostIpAddr()
{
    printf("\r\n*****STUB::GetBtsIpAddr....return 192.168.2.209*****\r\n");
    //返回主机序
    return 0xc0a802d1;

}
/*============================================================
MEMBER FUNCTION:
    TDRShow

DESCRIPTION:
    用于Tornado Shell上调用执行

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
extern "C" void TDRShow()
{
    CTaskTDR *taskTDR = CTaskTDR::GetInstance();
    taskTDR->showStatus();
}
