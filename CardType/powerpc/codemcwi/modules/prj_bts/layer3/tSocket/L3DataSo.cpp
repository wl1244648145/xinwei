/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    tSOCKET.cpp
 *
 * DESCRIPTION: 
 *   Socket task, 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        -----------------------------------------------
 *   11/01/06   xinwang       initialization.
 *
 *---------------------------------------------------------------------------*/

#ifdef __WIN32_SIM__

    #ifndef _WINSOCK2API_
        #include <winsock2.h>
    #endif
    #ifndef _WINDOWS_
        #include <windows.h>
    #endif
    #ifndef _INC_STRING
        #include <string.h>
    #endif

#else //Vxworks
    #include <vxworks.h>
    #include <sockLib.h>
    #include <inetLib.h>
    #include <string.h>
    #include <hostLib.h>
    #include <ioLib.h>
    #define INVALID_SOCKET ERROR
    #define SOCKET_ERROR   ERROR
    #define closesocket    close
#endif

//////////////////////////////////////////////////////////////////////////
//winsock2.h

#include "ComMessage.h"
#include "L3DataAPI.h"
#ifndef _INC_TSOCKET
    #include "L3DataSocket.h"
#endif

#ifndef __WIN32_SIM__
    #include "L3DataNotifyBTSPubIP.h"
    #include "Mcwill_bts.h"
#endif

#ifndef __L3_DATAREFRESH_JAMMINGNEGHBOR_H__
#include "L3DataNotifyRefreshJammingNeighbor.h"
#endif

#include "L3OamCfgCommon.h"
#include "L3OAMMessageId.h"
#include "L3OAMsystem.h"

#ifndef WBBU_CODE
#include "ui.h"
#endif
//using namespace std;
typedef map<UINT32, UINT16>::value_type ValType;

typedef struct _tag_JammingNeighborConfigNotify
{
    UINT16 TransId;
    UINT16 NeighborNum;//same frequency different sequence
    T_BtsInfoIE BtsInfoIe[M_TSOCKET_JAMMINGNEIGHBOR_NUM];
}JammingNeighborConfigNotify;
//////////////////////////////////////////////////////////////////////////
//bsp functions
extern "C" int bspGetBtsUDPRcvPort();
extern "C" int bspGetBtsID();
extern "C" int bspGetMainEmsIpAddr();
extern "C" int bspGetMainEmsUDPRcvPort();
extern "C" int bspGetBakEmsIpAddr();
extern "C" int bspGetBakEmsUDPRcvPort();
extern "C" int bspEnableNvRamWrite(char *startAddr, UINT32 size);
extern "C" int bspDisableNvRamWrite(char *startAddr, UINT32 size);
#ifndef __WIN32_SIM__
T_NvRamData *NvRamData = (T_NvRamData*)NVRAM_BASE_ADDR_OAM;
#else
extern T_NvRamData *NvRamData;
#endif
extern UINT32 gActiveEMSip;
extern UINT16 gActiveEMSport;
extern UINT32 IP_CB3000;
#define IP_CB3000_PORT  11002

//任务实例指针的初始化
CSOCKET* CSOCKET::s_ptaskTSOCKET = NULL;

UINT8  PostBtsIpNotifyBuf[M_TSOCKET_BUFFER_LENGTH];
UINT32 JammingTable[M_TSOCKET_JAMMINGNEIGHBOR_NUM];

//lijinan 090105 for jamming rpt flow ctl
UINT32 JammingMaxNumFromL2 = 20;
UINT32 JammingMaxNumFromBts = 50;
extern  bool sysStateIsRun();
unsigned int g_socket_no_ft_freelist =0;
unsigned int g_socket_no_CB_freelist = 0;
//NVRAM初始化
stNVRamFCB * const gpNVRamFCBTable  = (stNVRamFCB*)( (UINT8*)( NVRAM_TASK_SOCKET_DATA_BASE ) + sizeof( stDataSocketNVRAMhdr ) );
const UINT32 NVRamFCBTableLength = (sizeof(stNVRamFCB) * M_TSOCKET_FTENTRYNUM_MAX);


CSOCKET::CSOCKET()
:m_fdtSocket(INVALID_SOCKET),
m_fdPipePut(INVALID_SOCKET),m_fdPipe(INVALID_SOCKET),
m_nChar(0)
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->CSOCKET()" );
#ifndef NDEBUG
    LOG( LOG_DEBUG3, LOGNO( EMSA, EC_SOCKET_NORMAL ), "CTaskSOCKET::CTaskSOCKET()" );
    //use task EMSAgentTx instead
    if ( !Construct( CObject::M_OID_EMSAGENTTX ) )
    {
        LOG( LOG_SEVERE, LOGNO( EMSA, EC_SOCKET_SYS_ERR ), "ERROR!!!CTaskSOCKET::CTaskSOCKET()% Construct failed." );
    }
#endif

    memset( m_szName, 0, sizeof( m_szName ) );
    memcpy( m_szName, M_TSOCKET_TASKNAME, strlen( M_TSOCKET_TASKNAME ) );
    m_uPriority     = M_TP_L3EMSAGENTTX;
    m_uOptions      = M_TSOCKET_OPTION;
    m_uStackSize    = M_TSOCKET_STACKSIZE;
    m_lMaxMsgs      = M_TSOCKET_MAXMSG;
    m_lMsgQOption   = M_TSOCKET_MSGOPTION;
    m_plistFreeComMessage = NULL;
    FreeComMsgBufferCount = 0;
}

/*============================================================
MEMBER FUNCTION:
    CSOCKET::~CSOCKET

DESCRIPTION:
    CSOCKET析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CSOCKET::~CSOCKET()
{
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->~CSOCKET()" );
#ifndef NDEBUG
    LOG( LOG_DEBUG3, LOGNO( EMSA, EC_SOCKET_NORMAL ), "CTaskSOCKET::~CTaskSOCKET()" );
    if ( !Destruct( CObject::M_OID_EMSAGENTTX ) )
    {
        LOG( LOG_SEVERE, LOGNO( EMSA, EC_SOCKET_SYS_ERR ), "ERROR!!!CTaskSOCKET::~CTaskSOCKET failed." );
    }
#endif
}

CSOCKET* CSOCKET::GetInstance()
{
    if ( NULL == s_ptaskTSOCKET )
    {
        s_ptaskTSOCKET = new CSOCKET;
    }
    return s_ptaskTSOCKET;
}

/*============================================================
MEMBER FUNCTION:
    CSOCKET::Initialize

DESCRIPTION:
    创建Socket，设置选项，绑定Socket.

ARGUMENTS:
    NULL

RETURN VALUE:
    true of false 

SIDE EFFECTS:
    none
==============================================================*/
bool CSOCKET::Initialize()
{
   bspEnableNvRamWrite( (char*)NVRAM_TASK_SOCKET_DATA_BASE,  NVRAM_TASK_SOCKET_DATA_SIZE);
   // uiInit();
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->Initialize()" );
    //create pipe
    if (!CreatePipe())
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_SYS_ERR), "CreatePipe() failed.");
        return false;
    }
    //open pipe
    if (!OpenPipe())
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_SYS_ERR), "OpenPipe() failed.");
        DeletePipe();
        return false;
    }
    //set pipe unblock mode
    UINT32 on = TRUE;
#ifdef __WIN32_SIM__
    if ( ioctlsocket(m_fdPipe, FIONBIO, (u_long*)&on) < 0 ) // make socket non-block 
#else   //vxworks
    //if ( ioctl(m_fdPipe, FIONBIO, (int)&on) < 0 ) // make handle non-block 
    if (0)
#endif
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_SYS_ERR), "make pipe unblock failed!!!");
        return false;
    }

    //create socket
    #ifdef WBBU_CODE
    if (!CreateSocket())
    {
        return false;
    }
    #endif
    #ifndef WBBU_CODE 
     m_fdtSocket =  uiOpen(dwIP, htons( /*1024*/::bspGetBtsUDPRcvPort()), UDP);
     if (!m_fdtSocket)
    {
    	 printf(" open m_fdtSocket err!!!...\n ");
        return false;
    }
     uiCallOn(m_fdtSocket, UDPReceive, NULL);
	#endif
    if ((m_ptmFTExpire = InitTimer(true, M_TSOCKET_FT_EXPIRE_TIMEOUT, M_TSOCKET_TIMER_FT_EXPIRE)) == NULL )
    {
        return false;
    }

    InitMapEntry();

    InitFreeComMsgList();

    //初始化转发表
    memset(m_FT, 0, sizeof(m_FT));
    RecoverFTfromFCB();

    //BtsIp消息重发控制快
    memset(m_CB, 0, sizeof(m_CB));
    InitFreeCBList();

    InitialJNT();

    CTaskSystem::GetInstance()->SYS_setActiveEMS(bspGetMainEmsIpAddr(), bspGetMainEmsUDPRcvPort());

    //lijinan 090105 for jamming rpt flow ctl
    initJmFlowCtl();
ClearMeasure();
    return true;
}

void CSOCKET::initJmFlowCtl()
{
	T_NVRAM_BTS_CONFIG_PARA params1;
	
	 //lijinan 090105 for jamming rpt flow ctl
	    bspNvRamRead((char *)&params1, (char*)(NVRAM_BASE_ADDR_PARA_PARAMS), sizeof(params1));
	    if(params1.jammingPatter != NVRAM_VALID_PATTERN)
	    {
			JammingMaxNumFromL2 = 20;
			JammingMaxNumFromBts = 50;
		 params1.jammingPatter = NVRAM_VALID_PATTERN;
		 params1.jammingRptFromL2Num = 20;
		 params1.jammingRptFromBtsNum = 50;
   		 if(bspEnableNvRamWrite( (char*)NVRAM_BASE_ADDR_PARA_PARAMS, sizeof(params1))==TRUE)
	 	 {
		     memcpy((char*)NVRAM_BASE_ADDR_PARA_PARAMS,(char *)&params1,sizeof(params1));
	            bspDisableNvRamWrite( (char*)NVRAM_BASE_ADDR_PARA_PARAMS, sizeof(params1));
	        }
	    }
	    else
	    {
			JammingMaxNumFromL2 = params1.jammingRptFromL2Num;
			JammingMaxNumFromBts = params1.jammingRptFromBtsNum;
	     }

	    memset(&stJammingRptCtl, 0, sizeof(stJammingRptCtl));
	    stJammingRptCtl.pFromBtsTimer = NULL;
	    stJammingRptCtl.pFromL2Timer = NULL;
		
	    CComMessage* pMsg = NULL;
	    pMsg = new (this, 0) CComMessage;
	    pMsg->SetDstTid(M_TID_EMSAGENTTX);
	    pMsg->SetSrcTid(M_TID_EMSAGENTTX);
	    pMsg->SetMessageId(M_TSOCKET_TIMER_JAMMINGL2_EXPIRE);
	    pMsg->SetDataLength(0);
	    
	    stJammingRptCtl.pFromL2Timer  = new CTimer( false, 1000, pMsg );
	    if(stJammingRptCtl.pFromL2Timer==NULL)
	    	pMsg->Destroy();	
		
	    pMsg = new (this, 0) CComMessage;
	    pMsg->SetDstTid(M_TID_EMSAGENTTX);
	    pMsg->SetSrcTid(M_TID_EMSAGENTTX);
	    pMsg->SetMessageId(M_TSOCKET_TIMER_JAMMINGBTS_EXPIRE);
	    pMsg->SetDataLength(0);
	    
	    stJammingRptCtl.pFromBtsTimer  = new CTimer( false, 1000, pMsg );
	    if(stJammingRptCtl.pFromBtsTimer==NULL)
	    	pMsg->Destroy();	

		

}


/*============================================================
MEMBER FUNCTION:
    CSOCKET::MainLoop

DESCRIPTION:
    重载mainloop,使用select处理消息，分别处理来自UDP端口的消息，然后根据消息类型分发到相应的内部任务类中；
    处理来自Pipe的消息，动态建立socket,转发到外网

ARGUMENTS:
    NULL

RETURN VALUE:
    true of false 

SIDE EFFECTS:
    none
==============================================================*/
void CSOCKET::MainLoop()
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->MainLoop()" );
    FD_SET readfds;
    int ret;
    SOCKET biggerFd;

    struct timeval timer = { TSOCKET_MAX_BLOCKED_TIME_IN_10ms_TICK/SecondsToTicks(1), 0} ;

    while (1)
    {
        FD_ZERO(&readfds);
#ifdef WBBU_CODE
      FD_SET(m_fdtSocket, &readfds);      
#endif
        FD_SET(m_fdPipe, &readfds);
#ifdef WBBU_CODE
       biggerFd = (m_fdtSocket > m_fdPipe)? m_fdtSocket:m_fdPipe;
#endif
#ifndef WBBU_CODE
        biggerFd = m_fdPipe;
#endif
        ret = select(biggerFd+1, &readfds, NULL, NULL, &timer);
        if (ret == SOCKET_ERROR)
        {
            OutputSocketErrCode("ret = select(m_fdtSocket+1, &readfds, NULL, NULL, NULL)");
#ifdef WBBU_CODE
            CloseSocket();
             CreateSocket();
#endif
            taskDelay(10);
            continue;
        }
#ifdef WBBU_CODE	
        if (FD_ISSET(m_fdtSocket, &readfds))
        {
            ProcessRecvMsg();
        }
#endif        
        if (FD_ISSET(m_fdPipe, &readfds))
        {
            CComMessage* pComMsg = GetOneMsgFromPipe();
            if (NULL == pComMsg)
            {
                LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_SYS_ERR), "Get commessage from pipe failed.");
                continue;
            }
            ProcessSendMsg(pComMsg);
            pComMsg->Destroy();
        }
#ifdef __WIN32_SIM__
        ShowStatus();
#endif
    }
}
#ifndef WBBU_CODE
extern "C" void cSocket_ProcessRecvMsg(UINT32*pComMsg,int recLen, struct sockaddr_in src_ip)
{
	CSOCKET *taskSocket = CSOCKET::GetInstance();
	taskSocket->ProcessRecvMsg((CComMessage*)pComMsg,recLen,src_ip);
}

extern "C" void cSocket_GetFreeBuf(UINT8** pRecBuf,UINT32**pComMsg )
{
    CSOCKET *taskSocket = CSOCKET::GetInstance();
    *pComMsg = (UINT32*)(taskSocket->GetFreeComMsg());
    if (NULL == *pComMsg)
    {
         taskSocket->IncreasePerf( PERF_TSOCKET_NO_BUFFER );
	  *pRecBuf = NULL;
        return ;
    }
     *pRecBuf = (UINT8*)(((CComMessage*)*pComMsg)->GetBufferPtr());

     (*pRecBuf) += M_DEFAULT_RESERVED-sizeof(SocketMsgArea);

     // printf("Socket get free Buf...0x%x,0x%x\n",*pRecBuf,*pComMsg);
     

}
void CSOCKET::ProcessRecvMsg(CComMessage* pComMsg,int ret, sockaddr_in src_ip)
{
    //LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessRecvMsg()" );
    //int ret = 0;
    int socketRxAddrLen = sizeof(m_client);
    m_client = src_ip;
    
/*
    CComMessage* pComMsg = GetFreeComMsg();*/
    if (NULL == pComMsg)
    {
        IncreasePerf( PERF_TSOCKET_NO_BUFFER );
        return;
    }
    UINT8* pRecvBuf = (UINT8*)pComMsg->GetBufferPtr();

    //receive from bound port
   // ret = ::recvfrom(m_fdtSocket, (char*)((UINT8*)pRecvBuf+M_DEFAULT_RESERVED-sizeof(SocketMsgArea)), M_TSOCKET_MAX_MSGLEN, 0, (sockaddr*)&m_client, &socketRxAddrLen);
   /*
    if (ret == SOCKET_ERROR)
    {
        ReclaimFreeComMsg(pComMsg);
        IncreasePerf( PERF_TSOCKET_SOCK_ERR );
        OutputSocketErrCode("recvfrom(m_fdtSocket, pRecvBuf, sizeof(pRecvBuf), 0, (sockaddr*)&m_client, &socketRxAddrLen)");
        return;
    }
    if (ret > M_TSOCKET_MAX_MSGLEN)
    {
        ReclaimFreeComMsg(pComMsg);
        IncreasePerf( PERF_TSOCKET_SOCK_ERR );
        OutputSocketErrCode("receive message from socket > M_TSOCKET_MAX_MSGLEN(4000B)");
        return;
    }
    if (ret < sizeof(SocketMsgArea))
    {
        ReclaimFreeComMsg(pComMsg);
        IncreasePerf( PERF_TSOCKET_SOCK_ERR );
        OutputSocketErrCode("receive message from socket < sizeof(SocketMsgArea)");
        return;
    }*/


#ifdef __WIN32_SIM__
    UINT32 btsip = ntohl(m_client.sin_addr.S_un.S_addr);
#else
    UINT32 btsip = ntohl(m_client.sin_addr.s_addr);
#endif
   
    UINT16 btsport = ntohs(m_client.sin_port);
    if(btsip==IP_CB3000&&btsport==IP_CB3000_PORT)
    {
	    pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr() + M_DEFAULT_RESERVED-sizeof(SocketMsgArea)));
	    pComMsg->SetDataLength(ret);
	    pComMsg->SetDstTid(M_TID_EB);
           pComMsg->SetMessageId(MSG_BTS_TO_CB3000);
       
	    if(!CComEntity::PostEntityMessage(pComMsg))
	    {
	        LOG( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "forwarding this msg(0x%X) to eb failed!!! ");
	        return ;
	    }
		return;
    }
    pComMsg->SetBtsAddr(btsip);
    pComMsg->SetBtsPort(btsport);

    SocketMsgArea* pMsgArea = (SocketMsgArea*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED-sizeof(SocketMsgArea));
    switch (pMsgArea->MsgArea)
    {
        case MSGAREA_EMS:
            ret = ProcessEmsMsg(pComMsg, ret);
            break;
        case MSGAREA_TUNNEL_MANAGEMENT:
            ret = ProcessTunnelManagementMsg(pComMsg, ret);
            break;
        case MSGAREA_TUNNEL_DATA:
            ret = ProcessTunnelDataMsg(pComMsg, ret);
            break;
        case MSGAREA_LOADING_INFO:
            ret = ProcessLoadingInfoMsg(pComMsg, ret);
            break;
        case MSGAREA_DIAG:
            ret = ProcessDiagMsg(pComMsg, ret);
            break;
        case MSGAREA_JAMMING_REPORT:
        case MSGAREA_JAMMING_REPORT_RSP:
        case MSGAREA_PAIREDCPE_PROF:
        case MSGAREA_GROUP_RESOURCE_RPT_RSP:
            ret = ProcessJammingFromBts(pComMsg, ret);
            break;        
	case MSGAREA_GRPSRV_HORESREQ:
	case MSGAREA_GRPSRV_HORESRSP:
		ret = ProcessGrpSrvHOMsgFromOtherBTS(pComMsg, ret);
		break;
        default:
            IncreasePerf(PERF_TSOCKET_INVALID_RX);
            LOG2(LOG_WARN, LOGNO(EMSA, EC_SOCKET_NORMAL), "Invalid MsgArea 0x%x from IP 0x%x.",pMsgArea->MsgArea,btsip);
            ret = 0;
            break;
    }
    if (ret != 1)
    {
        pComMsg->Destroy();
    }
}
#else
  static char buf_Recv[M_TSOCKET_MAX_MSGLEN];
void CSOCKET::ProcessRecvMsg()
{
   LOG( (LOG_DEBUG3), LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessRecvMsg()" );
    int ret = 0;
    int socketRxAddrLen = sizeof(m_client);
   // m_client = src_ip;
    

    CComMessage* pComMsg = GetFreeComMsg();
    if (NULL == pComMsg)
    {
        
        IncreasePerf( PERF_TSOCKET_NO_BUFFER );
	 ::recvfrom(m_fdtSocket, buf_Recv, M_TSOCKET_MAX_MSGLEN, 0, (sockaddr*)&m_client, &socketRxAddrLen);
        LOG( LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "ProcessRecvMsg GetFreeComMsg failed!!! ");
        return;
    }
    UINT8* pRecvBuf = (UINT8*)pComMsg->GetBufferPtr();

    //receive from bound port
    ret = ::recvfrom(m_fdtSocket, (char*)((UINT8*)pRecvBuf+M_DEFAULT_RESERVED-sizeof(SocketMsgArea)), M_TSOCKET_MAX_MSGLEN, 0, (sockaddr*)&m_client, &socketRxAddrLen);
 //  printf("here\n");
    if (ret == SOCKET_ERROR)
    {
        ReclaimFreeComMsg(pComMsg);
        IncreasePerf( PERF_TSOCKET_SOCK_ERR );
        OutputSocketErrCode("recvfrom(m_fdtSocket, pRecvBuf, sizeof(pRecvBuf), 0, (sockaddr*)&m_client, &socketRxAddrLen)");
        return;
    }
    if (ret > M_TSOCKET_MAX_MSGLEN)
    {
        ReclaimFreeComMsg(pComMsg);
        IncreasePerf( PERF_TSOCKET_SOCK_ERR );
        OutputSocketErrCode("receive message from socket > M_TSOCKET_MAX_MSGLEN(4000B)");
        return;
    }
    if (ret < sizeof(SocketMsgArea))
    {
        ReclaimFreeComMsg(pComMsg);
        IncreasePerf( PERF_TSOCKET_SOCK_ERR );
        OutputSocketErrCode("receive message from socket < sizeof(SocketMsgArea)");
        return;
    }
    #if 0
   if(ret>1000)
   	{
   	   LOG1(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_NORMAL), "cal data:%d\n",ret);
   	   return;
   	// OutputSocketErrCode("receive message from socket < sizeof(SocketMsgArea)");
   	}
#endif
#ifdef __WIN32_SIM__
    UINT32 btsip = ntohl(m_client.sin_addr.S_un.S_addr);
#else
    UINT32 btsip = ntohl(m_client.sin_addr.s_addr);
#endif
    UINT16 btsport = ntohs(m_client.sin_port);
    if(btsip==IP_CB3000&&btsport==IP_CB3000_PORT)
    {
	    pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr() + M_DEFAULT_RESERVED-sizeof(SocketMsgArea)));
	    pComMsg->SetDataLength(ret);
	    pComMsg->SetDstTid(M_TID_EB);
           pComMsg->SetMessageId(MSG_BTS_TO_CB3000);
       
	    if(!CComEntity::PostEntityMessage(pComMsg))
	    {
	        LOG( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "forwarding this msg(0x%X) to eb failed!!! ");
	        return ;
	    }
		return;
    }
    pComMsg->SetBtsAddr(btsip);
    pComMsg->SetBtsPort(btsport);

    SocketMsgArea* pMsgArea = (SocketMsgArea*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED-sizeof(SocketMsgArea));
    switch (pMsgArea->MsgArea)
    {
        case MSGAREA_EMS:
            ret = ProcessEmsMsg(pComMsg, ret);
            break;
        case MSGAREA_TUNNEL_MANAGEMENT:
            ret = ProcessTunnelManagementMsg(pComMsg, ret);
            break;
        case MSGAREA_TUNNEL_DATA:
            ret = ProcessTunnelDataMsg(pComMsg, ret);
            break;
        case MSGAREA_LOADING_INFO:
            ret = ProcessLoadingInfoMsg(pComMsg, ret);
            break;
        case MSGAREA_DIAG:
            ret = ProcessDiagMsg(pComMsg, ret);
            break;
        case MSGAREA_JAMMING_REPORT:
        case MSGAREA_JAMMING_REPORT_RSP:
        case MSGAREA_PAIREDCPE_PROF:
        case MSGAREA_GROUP_RESOURCE_RPT_RSP:
            ret = ProcessJammingFromBts(pComMsg, ret);
            break;        
	case MSGAREA_GRPSRV_HORESREQ:
	case MSGAREA_GRPSRV_HORESRSP:
		ret = ProcessGrpSrvHOMsgFromOtherBTS(pComMsg, ret);
		break;
        #ifdef WBBU_CODE
        case MSGAREA_GPS:    //zengjihan 20120801 for GPSSYNC
            ret = ProcessGpsMsg(pComMsg, ret);
            break;
        #endif
        default:
            IncreasePerf(PERF_TSOCKET_INVALID_RX);
            LOG2(LOG_WARN, LOGNO(EMSA, EC_SOCKET_NORMAL), "Invalid MsgArea 0x%x from IP 0x%x.",pMsgArea->MsgArea,btsip);
            ret = 0;
            break;
    }
    if (ret != 1)
    {
        pComMsg->Destroy();
    }
}

#endif
void CSOCKET::ProcessSendMsg(CComMessage* pComMsg)
{
    //LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessSendMsg()" );
    TID tid = pComMsg->GetSrcTid();
    UINT16 msgid = pComMsg->GetMessageId();

    switch (tid)
    {
        case M_TID_EMSAGENTTX:
            if (M_TSOCKET_TIMER_FT_EXPIRE == msgid)
            {
                FTBPtreeExpire();

            }
            else if (M_TSOCKET_TIMER_CB_EXPIRE == msgid)
            {
                BtsIpNotifyTimeout(pComMsg);
            }

	     //lijinan 090105 for jamming rpt flow ctl
	     else if(M_TSOCKET_TIMER_JAMMINGL2_EXPIRE == msgid)
	     {
	     	  stJammingRptCtl.jammingFromL2Cnt = 0;
		  stJammingRptCtl.pFromL2Timer->Stop();
		  stJammingRptCtl.L2TimerIsStart = FLAG_NO;
	     }
	     else if(M_TSOCKET_TIMER_JAMMINGBTS_EXPIRE == msgid)
	     {
		   stJammingRptCtl.jammingFromBtsCnt = 0;
		   stJammingRptCtl.pFromBtsTimer->Stop();
		   stJammingRptCtl.BtsTimerIsStart = FLAG_NO;
	     }
		 
            break;
        case M_TID_TUNNEL:
            if (MSGID_IPLIST_PUBIP_MODIFY == msgid)
            {
                ProcessUpdateBtsPubIp(pComMsg);
            }
            else
            {
                if (true == PostMsgToBts(pComMsg, MSGAREA_TUNNEL_MANAGEMENT))
                    IncreasePerf(PERF_TSOCKET_TCR_PACKETS_TX);
            }
            break;
        case M_TID_CM:
            if (M_SOCKET_JNT_REFRESH == msgid)
            {
                ProcessJNTRefreshNotify(pComMsg);
            }            
            else
            {
                PostMsgToEms(pComMsg);
            }
            break;
            
        case M_TID_L2MAIN:
            if((BTS_Jamming_Rpt == msgid)||(BTS_Jamming_Rpt_Rsp == msgid)||(BTS_PairedCpe_Prof_Msg == msgid)
                ||(BTS_GROUP_RESOURCE_Rpt_Rsp == msgid))
            {
                ProcessJammingFromL2(pComMsg);
            }
            break;
    case M_TID_VOICE:
        if(MSGID_GRP_HO_RES_REQ==msgid || MSGID_GRP_HO_RES_RSP==msgid)
        {
            ProcessGrpSrvHOMsg2OtherBTS(pComMsg);
        }
        break;
    case M_TID_GM:
        if (M_OAM_CFG_HLR_TIME_REQ == msgid)    
            ProcessSTime2HLR(pComMsg);
        #ifdef WBBU_CODE
        else if(M_OAM_GPS_SYNC_NOTIFY == msgid)    //zengjihan 20120801 for GPSSYNC
            PostMsgToGps(pComMsg,MSGAREA_GPS);
        #endif
        else
            PostMsgToEms(pComMsg);
        break;
    case M_TID_EB:
        if(msgid==MSG_BTS_TO_CB3000)
            ProcessMsg2CB3000(pComMsg);
        break;
    case M_TID_EMSAGENTRX://新加的消息，用来发送获得基站ip消息，给eb，snoop使用
        if(msgid == MSGID_OAM_NOTIFY_GET_BTSPUBIP)   
        {
            UINT32 btsid = pComMsg->GetBTS();
            PostBtsIpNotification(btsid);
        }
        break;
        default:
            if (M_SOCKET_LOADINGINFO_TX == msgid)
            {
                if(true == PostMsgToBts(pComMsg, MSGAREA_LOADING_INFO))
                    IncreasePerf(PERF_TSOCKET_LOADING_PACKETS_TX);
            }
            else
            {
                PostMsgToEms(pComMsg);
            }
            break;
    }
}


void CSOCKET::ProcessSTime2HLR(CComMessage* pComMsg)
{
    if (!ASSERT_VALID(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
        return;
    }
    static UINT16 trans_index = 0x8000;
    char buf[32];
    UINT32 ems = NvRamData->sTimePara.T_server_ip;
    UINT16 port= NvRamData->sTimePara.T_server_port;
        SocketMsgArea* pArea = (SocketMsgArea*)buf;

        pArea->MsgArea = 0x08;
        HlrMsgHeader* pHead = (HlrMsgHeader*)((UINT8*)pArea+2);
        pHead->ma = htons(0x07);
        pHead->moc = htons(0x0752);
        pHead->action = htons(0xff00);
        pHead->btsid = htonl(bspGetBtsID());
	 pHead->transId =  htons(trans_index);
	 UINT32 period = NvRamData->sTimePara.STime_period;
	 pHead->period =  htonl(period);
	//T_TimeDate time = bspGetDateTime();
	 char *p = buf+2+sizeof(HlrMsgHeader);
	memcpy(p,(char*)pComMsg->GetDataPtr(),11);
	/* memcpy(p,(char*)&htons(time.year),2);
	 p+=2;
	 *p = time.month;
	 p+=1;
	 *p = time.day;
	 p+=1;
	 *p = time.hour;
	 p+=1;
	 *p = time.minute;
	  p+=1;
	 *p = time.second;
	  p+=1;
	  *p = 0;*/
	  
	  int len = sizeof(HlrMsgHeader)+2+11;
        if (!SendBySocket(ems, port, buf, len))
        {
            IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
            LOG2(LOG_WARN, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket() to EMS(0x%X:%d) error!!!", ems, port);
            return;
        }
	trans_index++;
}

void CSOCKET::ProcessMsg2CB3000(CComMessage* pComMsg)
{
    if (!ASSERT_VALID(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
        return;
    }
    char* buf =(char*)pComMsg->GetDataPtr() -6;
    UINT32 bs_id = htonl(bspGetBtsID());
    memcpy(buf,(char*)&bs_id,4);
    UINT16 msg_len = pComMsg->GetDataLength();
    memcpy(&buf[4],(char*)&msg_len,2);
    //memcpy(&buf[6],(char*)pComMsg->GetDataPtr(),msg_len);
    UINT16 port = IP_CB3000_PORT;
     int len = msg_len+6;
        if (!SendBySocket(IP_CB3000, port, buf, len))
        {
            IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
            LOG2(LOG_WARN, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket() to CB3000(0x%X:%d) error!!!", IP_CB3000, port);
            return;
        }
}


bool CSOCKET::ProcessEmsMsg(CComMessage* pComMsg, UINT16 nChar)
{
    //LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessEmsMsg()" );
    if ((nChar - sizeof(SocketMsgArea) - sizeof(SocketEmsMsgHeader)) < 0)
    {
        LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_NORMAL), "EMS message error");
        IncreasePerf(PERF_TSOCKET_EMSA_ERR);
        return false;
    }
    SocketEmsMsgHeader* pHead = (SocketEmsMsgHeader*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED);
    UINT32 btsid  = pHead->btsid;
    if (btsid == bspGetBtsID())
    {
        //search map
        int mapIndex = SearchMap(ntohs(pHead->ma), ntohs(pHead->moc), ntohs(pHead->action));
        if (mapIndex!=-1)
        {
            pComMsg->SetMessageId(g_EmsMessageMap[mapIndex].msgid);
            if (M_TSOCKET_BTSIP_REQ == pComMsg->GetMessageId())
            {
                ProcessBtsIpRequest(pComMsg);
                pComMsg->Destroy();
                return true;
            }
//test start
//printf("\r\nM_TSOCKET_BTSIP_REQ != pComMsg->GetMessageId()\r\n" );
//test stop             
            pComMsg->SetDstTid(g_EmsMessageMap[mapIndex].tid);
            pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED+sizeof(SocketEmsMsgHeader)));
            pComMsg->SetDataLength(nChar-sizeof(SocketMsgArea)-sizeof(SocketEmsMsgHeader));
            if (!CComEntity::PostEntityMessage(pComMsg))
            {
                IncreasePerf(PERF_TSOCKET_EMSA_ERR);
                return false;
            }
            else
            {
//test start
//printf("\r\nProcessEmsMsg--->true == CComEntity::PostEntityMessage\r\n" );
//test stop             
            }
        }
        else
        {         
    /***********************************************
        *  pComMsg buffer                              *
        *-----------64 rsv---------|                   *
        *  n  rsv|msgarea|emsheader|message            *
        ************************************************/
        UINT32 ems = gActiveEMSip;
        UINT16 port= gActiveEMSport, ustemp;
        CComMessage *RspMsg = new ( this, 6 ) CComMessage;
        SocketMsgArea* pArea = (SocketMsgArea*)((UINT8*)RspMsg->GetDataPtr()-sizeof(SocketMsgArea)-sizeof(SocketEmsMsgHeader));
        //增加Header的时候，超过了Buffer预留的64 rsv
        if (pArea < RspMsg->GetBufferPtr())
        {
            IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
            LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), "Commmessage buffer reserve 64 is too low, error!!!");
            return false;
        }
        pArea->MsgArea = MSGAREA_EMS;
        SocketEmsMsgHeader* pHeadRsp = (SocketEmsMsgHeader*)((UINT8*)pArea+sizeof(SocketMsgArea));
        pHeadRsp->ma = htons(pHead->ma);
	 ustemp = 0x0800;
        pHeadRsp->moc = htons(ustemp);
        pHeadRsp->action = htons(pHead->action);
        pHeadRsp->btsid = htonl(bspGetBtsID());
	 UINT16 *pus = (UINT16*)RspMsg->GetDataPtr();
	    *pus = *(UINT16*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED+sizeof(SocketEmsMsgHeader));
        pus+=1;
	    *pus = 4; 
        pus+=1;
	 *pus = pHead->moc;
	 
        if (!SendBySocket(ems, port, pArea, sizeof(SocketMsgArea)+sizeof(SocketEmsMsgHeader)+RspMsg->GetDataLength()))
        {
            IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
            LOG2(LOG_WARN, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket() to EMS(0x%X:%d) error!!!", ems, port);
            return false;
        }       
    
            IncreasePerf(PERF_TSOCKET_EMSA_ERR);
            LOG4(LOG_SEVERE, LOGNO(EMSA, EC_SOCKET_NORMAL), "EMS map serch error. illegal command,msgid:0x%X,(0x%x,0x%x,0x%x)",pComMsg->GetMessageId(), ntohs(pHead->ma), ntohs(pHead->moc), ntohs(pHead->action));
            return false;
        }
    }
    IncreasePerf(PERF_TSOCKET_EMSA_PACKETS_RX);
    return true;
}



bool CSOCKET::ProcessTunnelManagementMsg(CComMessage* pComMsg, UINT16 nChar)
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessTunnelManagementMsg()" );
    pComMsg->SetDstTid(M_TID_TUNNEL);
    pComMsg->SetMessageId(MSGID_TUNNEL_MSGFROM_TCR);
    pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED));
    pComMsg->SetDataLength(nChar-sizeof(SocketMsgArea));
    if (!CComEntity::PostEntityMessage(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_TCR_ERR);
        return false;
    }
    IncreasePerf(PERF_TSOCKET_TCR_PACKETS_RX);
    return true;
}


bool CSOCKET::ProcessTunnelDataMsg(CComMessage* pComMsg, UINT16 nChar)
{
    //LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessTunnelDataMsg()" );
    pComMsg->SetMessageId(MSGID_TRAFFIC_ETHERIP);
    pComMsg->SetDstTid(M_TID_EB);
    pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED));
    pComMsg->SetDataLength(nChar-sizeof(SocketMsgArea));
    if (!CComEntity::PostEntityMessage(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_TDR_ERR);
        return false;
    }
    IncreasePerf(PERF_TSOCKET_TDR_PACKETS);
    return true;
}

#include "L3OamCfgCommon.h"
bool CSOCKET::ProcessLoadingInfoMsg(CComMessage* pComMsg, UINT16 nChar)
{
    //LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessLoadingInfoMsg()" );
    pComMsg->SetDstTid(M_TID_UM);
    pComMsg->SetMessageId(M_EMS_BTS_NEIGHBOUR_BTS_LOADINFO_CFG_REQ);

    pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr() + M_DEFAULT_RESERVED));
    pComMsg->SetDataLength(nChar-sizeof(SocketMsgArea));

    // check if the IP address of the sender BTS got changed 
    T_NeighbotBTSLoadInfo *pRxLoadInfo = (T_NeighbotBTSLoadInfo *)pComMsg->GetDataPtr();
    UINT32 btsId = pRxLoadInfo->BTSID;
    BtsAddr addr;
    addr.IP = pComMsg->GetBtsAddr();
    addr.Port = pComMsg->GetBtsPort();
    FTAddEntry(btsId, addr);


    if (!CComEntity::PostEntityMessage(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_LOADING_ERR);
        return false;
    }
    IncreasePerf(PERF_TSOCKET_LOADING_PACKETS_RX);

    return true;
}

bool CSOCKET::ProcessDiagMsg(CComMessage* pComMsg, UINT16 nChar)
{
    //LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessDiagMsg()" );
    pComMsg->SetDstTid(M_TID_L3OAMDIAGEMSL3L2);

    pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr() + M_DEFAULT_RESERVED));
    pComMsg->SetDataLength(nChar-sizeof(SocketMsgArea));
    if (!CComEntity::PostEntityMessage(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_DIAG_ERR);
        return false;
    }
    IncreasePerf(PERF_TSOCKET_DIAG_PACKETS);
    return true;
}

//zengjihan 20120801 for GPSSYNC
#ifdef WBBU_CODE
bool CSOCKET::ProcessGpsMsg(CComMessage* pComMsg, UINT16 nChar)
{
    LOG(LOG_DEBUG3, LOGNO(EMSA,0), "CSOCKET::ProcessGpsMsg");
    if ((nChar - sizeof(SocketMsgArea) - sizeof(SocketEmsMsgHeader)) < 0)
    {
        LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_NORMAL), "EMS message error");
        IncreasePerf(PERF_TSOCKET_EMSA_ERR);
        return false;
    }
    SocketEmsMsgHeader* pHead = (SocketEmsMsgHeader*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED);
    UINT32 btsid  = pHead->btsid;
    if (btsid == bspGetBtsID())
    {
        //search map
        int mapIndex = SearchMap(ntohs(pHead->ma), ntohs(pHead->moc), ntohs(pHead->action));
        if (mapIndex!=-1)
        {
            pComMsg->SetMessageId(g_EmsMessageMap[mapIndex].msgid);
            pComMsg->SetDstTid(g_EmsMessageMap[mapIndex].tid);
            pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED+sizeof(SocketEmsMsgHeader)));
            pComMsg->SetDataLength(nChar-sizeof(SocketMsgArea)-sizeof(SocketEmsMsgHeader));
            if (!CComEntity::PostEntityMessage(pComMsg))
            {
                IncreasePerf(PERF_TSOCKET_EMSA_ERR);
                return false;
            }
        }
    }
    IncreasePerf(PERF_TSOCKET_EMSA_PACKETS_RX);
    return true;
}
#endif

void CSOCKET::PostMsgToEms(CComMessage* pComMsg)
{
    UINT16 usMsgId = pComMsg->GetMessageId();
    LOG2( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->PostMsgToEms(taskid=%d,msgid=0x%X)",UINT16(pComMsg->GetSrcTid()), usMsgId);
    if (!ASSERT_VALID(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
        return;
    }
    UINT32 ems = gActiveEMSip;
    UINT16 port= gActiveEMSport;
    if (M_SYS_SOCKET_BAK_EMS == usMsgId)
    {
        /*
         *此消息需发往非目前正用的ems;恢复原消息
         */
        T_WrapToEMS *ptr = (T_WrapToEMS *)(pComMsg->GetDataPtr());
        usMsgId = ptr->usMsgCode;
        if (M_SENDTO_MAIN_EMS == ptr->ucToMainEMS)
            {
            //往主用ems
            ems  = bspGetMainEmsIpAddr();
            port = bspGetMainEmsUDPRcvPort();
            }
        else
            {
            //往备用ems
            ems  = bspGetBakEmsIpAddr();
            port = bspGetBakEmsUDPRcvPort();
            }
        ptr += 1;
        pComMsg->SetMessageId(usMsgId);
        pComMsg->SetDataPtr(ptr);
        pComMsg->SetDataLength(pComMsg->GetDataLength() - sizeof(T_WrapToEMS));
        //决定ems.
        //LOG2(LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_NORMAL), "send to EMS:0x%x,port:%d", ems, port);
    }

    int msgIndex = SearchMap(pComMsg->GetSrcTid(), usMsgId);
    if (msgIndex!=-1)
    {
        /***********************************************
        *  pComMsg buffer                              *
        *-----------64 rsv---------|                   *
        *  n  rsv|msgarea|emsheader|message            *
        ************************************************/
        SocketMsgArea* pArea = (SocketMsgArea*)((UINT8*)pComMsg->GetDataPtr()-sizeof(SocketMsgArea)-sizeof(SocketEmsMsgHeader));
        //增加Header的时候，超过了Buffer预留的64 rsv
        if (pArea < pComMsg->GetBufferPtr())
        {
            IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
            LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), "Commmessage buffer reserve 64 is too low, error!!!");
            return;
        }
        pArea->MsgArea = MSGAREA_EMS;
        SocketEmsMsgHeader* pHead = (SocketEmsMsgHeader*)((UINT8*)pArea+sizeof(SocketMsgArea));
        pHead->ma = htons(g_EmsMessageMap[msgIndex].ma);
        pHead->moc = htons(g_EmsMessageMap[msgIndex].moc);
        pHead->action = htons(g_EmsMessageMap[msgIndex].action);
        pHead->btsid = htonl(bspGetBtsID());
        if (!SendBySocket(ems, port, pArea, sizeof(SocketMsgArea)+sizeof(SocketEmsMsgHeader)+pComMsg->GetDataLength()))
        {
            IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
            LOG2(LOG_WARN, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket() to EMS(0x%X:%d) error!!!", ems, port);
            return;
        }
        IncreasePerf(PERF_TSOCKET_EMSA_PACKETS_TX);
        return;
    }
    IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
    return;
}

//zengjihan 20120801 for GPSSYNC
#ifdef WBBU_CODE
bool CSOCKET::PostMsgToGps(CComMessage* pComMsg, UINT16 msgArea)
{
    LOG(LOG_DEBUG3, LOGNO(EMSA,0), "CSOCKET::PostMsgToGps");
    UINT16 usMsgId = pComMsg->GetMessageId();
    LOG2( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->PostMsgToGps(taskid=%d,msgid=0x%X)",UINT16(pComMsg->GetSrcTid()), usMsgId);
    if (!ASSERT_VALID(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
        return false;
    }
    UINT32 btsid = pComMsg->GetBTS();
    LOG1(LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "[SOCKET]Tx message to BTS[%d]", btsid);
    BtsAddr addr;
    if (!GetBtsPubAddr(btsid, &addr))
    {
        return false;
    }
    
    int msgIndex = SearchMap(pComMsg->GetSrcTid(), usMsgId);
    if (msgIndex!=-1)
    {
        /************************************************
        *  pComMsg buffer                                                         *
        *-----------64 rsv--------|                                      *
        *  n  rsv|msgarea|emsheader|message                         *
        ************************************************/
        SocketMsgArea* pArea = (SocketMsgArea*)((UINT8*)pComMsg->GetDataPtr()-sizeof(SocketMsgArea)-sizeof(SocketEmsMsgHeader));
        //增加Header的时候，超过了Buffer预留的64 rsv
        if (pArea < pComMsg->GetBufferPtr())
        {
            IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
            LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), "Commmessage buffer reserve 64 is too low, error!!!");
            return false;
        }
        pArea->MsgArea = MSGAREA_GPS;
        SocketEmsMsgHeader* pHead = (SocketEmsMsgHeader*)((UINT8*)pArea+sizeof(SocketMsgArea));
        pHead->ma = htons(g_EmsMessageMap[msgIndex].ma);
        pHead->moc = htons(g_EmsMessageMap[msgIndex].moc);
        pHead->action = htons(g_EmsMessageMap[msgIndex].action);
        pHead->btsid = htonl(btsid);
        if (!SendBySocket(addr.IP,addr.Port, pArea, sizeof(SocketMsgArea)+sizeof(SocketEmsMsgHeader)+pComMsg->GetDataLength()))
        {
            IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
            LOG2(LOG_WARN, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket() to BTS(0x%X:%d) error!!!", addr.IP,addr.Port);
            return false;
        }
        IncreasePerf(PERF_TSOCKET_EMSA_PACKETS_TX);
        return true;
    }
    IncreasePerf(PERF_TSOCKET_EMSA_TX_FAIL);
    return false;
}
#endif

bool CSOCKET::PostMsgToBts(CComMessage* pComMsg, UINT16 msgArea)
{
    if (!ASSERT_VALID(pComMsg))
    {
        return false;
    }
    UINT32 btsid = pComMsg->GetBTS();
    LOG1(LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "[SOCKET]Tx message to BTS[%d]", btsid);
    BtsAddr addr;
    if (!GetBtsPubAddr(btsid, &addr))
    {
        return false;
    }

    SocketMsgArea* pArea = (SocketMsgArea*)((UINT8*)pComMsg->GetDataPtr()-sizeof(SocketMsgArea));
    //增加Header的时候，超过了Buffer预留的64 rsv
    if (pArea < pComMsg->GetBufferPtr())
    {
        LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), "Commmessage dataptr exceed bufferptr, error!!!");
        return false;
    }
    pArea->MsgArea = msgArea;
    if (!SendBySocket(addr.IP,addr.Port,pArea,sizeof(SocketMsgArea)+pComMsg->GetDataLength()))
    {
        LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket(), error!!!");
        return false;
    }
    return true;
}

//创建Pipe
bool CSOCKET::CreatePipe()
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->CreatePipe()" );
#ifdef __WIN32_SIM__
    m_fdPipePut = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET==m_fdPipePut)
    {
        OutputSocketErrCode("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)");
        return false;
    }
    m_fdPipe = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (INVALID_SOCKET==m_fdPipe)
    {
        OutputSocketErrCode("socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)");
        return false;
    }
    sockaddr_in pipe;
    pipe.sin_family = AF_INET;
    pipe.sin_addr.s_addr = htonl(INADDR_ANY);
    pipe.sin_port = htons(M_TSOCKET_PIPE_PORT);
    if (::bind(m_fdPipe, (sockaddr*)&pipe, sizeof(pipe))==SOCKET_ERROR)
    {
        OutputSocketErrCode("bind(m_fdPipe, (sockaddr*)&pipe, sizeof(pipe))==SOCKET_ERROR");
        return false;
    }
    return true;
#else//vxworks
    return pipeDevCreate(M_TSOCKET_PIPENAME, m_lMaxMsgs, sizeof(int))==OK ? true:false;
#endif  
}

//打开Pipe
bool CSOCKET::OpenPipe()
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->OpenPipe()" );
#ifdef __WIN32_SIM__
    return true;
#else//vxworks
    m_fdPipe = open(M_TSOCKET_PIPENAME, O_RDWR, 0644);
    return(m_fdPipe == ERROR)?false:true;
#endif  
}

//删除Pipe
bool CSOCKET::DeletePipe()
{
#ifdef __WIN32_SIM__
    closesocket(m_fdPipePut);
    closesocket(m_fdPipe);
    return true;
#else//vxworks
    return pipeDevDelete(M_TSOCKET_PIPENAME, true)==OK;
#endif
}

//关闭管道
bool CSOCKET::ClosePipe()
{
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ClosePipe()" );
#ifdef __WIN32_SIM__
    return true;
#else//vxworks
    return close(m_fdPipe)!=ERROR;
#endif  
}

void CSOCKET::OutputSocketErrCode(char*p)
{
#ifdef __WIN32_SIM__
    int err_code = WSAGetLastError();
#else//vxworks
    int err_code = errnoGet();
#endif
    if (NULL!=p)
    {
        LOG2(LOG_SEVERE, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "%s failed.errno=[%d]", (int)p, err_code);
    }
    else
    {
        LOG2(LOG_SEVERE, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "%s failed.errno=[%d]", (int)"socket operation", err_code);
    }
}

bool CSOCKET::CreateSocket(void)
{
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->CreateSocket()" );
    //create receive socket and bind it to localhost port 
    m_fdtSocket = ::socket(AF_INET,SOCK_DGRAM, IPPROTO_UDP);     
    if ( INVALID_SOCKET == m_fdtSocket)
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_NORMAL), "Create SocketRx failed.");
#ifdef __WIN32_SIM__
        ::WSACleanup();
#else
        ::close( m_fdtSocket );
#endif
        return false;
    }
    sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr =htonl(INADDR_ANY);
    server.sin_port =htons( /*1024*/::bspGetBtsUDPRcvPort());//M_TSOCKET_DEFAULT_PORT
//make socket unblock
   UINT32 on=1;
    if(ioctl(m_fdtSocket, FIONBIO, (int)&on) < 0) 
    { 
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "make socket unblock fail.");
        OutputSocketErrCode("ioctl(m_fdtSocket, FIONBIO, on)");
    }   

    if (::bind(m_fdtSocket,(sockaddr*)&server,sizeof(server)))
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_NORMAL), "Bind SocketRx failed.");
#ifdef __WIN32_SIM__
        ::closesocket( m_fdtSocket );
        ::WSACleanup();
#else
        ::close( m_fdtSocket );
#endif
        return false;
    }

    return true;
}

bool CSOCKET::CloseSocket()
{
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->CloseSocket()" );
#ifdef __WIN32_SIM__
    closesocket(m_fdtSocket);
#else   //vxworks
    close(m_fdtSocket);
#endif
    m_fdtSocket = INVALID_SOCKET;
    return true;
}
bool CSOCKET::PostMessage(CComMessage* pMsg, SINT32 timeout, bool isUrgent)
{
    if (NULL==pMsg)
        return false;

    LOG2( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->PostMessage()--tid=%d,msgid=0x%X",UINT16(pMsg->GetSrcTid()),pMsg->GetMessageId() );
    
    if (!pMsg->AddRef())
    {
        LOG(LOG_CRITICAL,LOGNO(EMSA, EC_SOCKET_NORMAL),"Add Reference failed.");
        return false;
    }

    //往管道中发送pMsg
#ifdef __WIN32_SIM__
    struct sockaddr_in pipe;
    pipe.sin_family = AF_INET;
    pipe.sin_port= htons(M_TSOCKET_PIPE_PORT);
    pipe.sin_addr.s_addr=inet_addr("127.0.0.1");
    int ret = sendto (m_fdPipePut, (char *)&pMsg, sizeof(pMsg), 0, (struct sockaddr *)&pipe, sizeof(struct sockaddr));
#else
    int ret = write(m_fdPipe, (char*)&pMsg, sizeof(pMsg));
#endif
    if (ret==sizeof(pMsg))
    {
        return true;
    }
    else
    {
        return false;
    }

}

CComMessage* CSOCKET::GetOneMsgFromPipe(void)
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetOneMsgFromPipe()" );
    CComMessage* pComMsg = NULL;
    int ret;
#ifdef __WIN32_SIM__
    ret = recvfrom(m_fdPipe, (char*)&pComMsg, 4, 0, NULL, NULL);
#else   //vxworks
    ret = read(m_fdPipe, (char*)&pComMsg, 4);
#endif  
    if (ret<0)
    {
        LOG(LOG_SEVERE, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), "Read from Pipe error!!!");
        pComMsg = NULL;
    }
    else
    {
        if (4!=ret)
        {
            LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), "Read less than 4 bytes from Pipe, error!!!");
            pComMsg = NULL;
        }
    }
    return pComMsg; 
}

//////////////////////////////////////////////////////////////////////////
//Ems message map
//初始化mapMsgTx & mapMsgRx
void CSOCKET::InitMapEntry()
{
    LOG( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->InitMapEntry()" );
#ifndef NDEBUG
    LOG(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_NORMAL), "CEmsAgentRx::Initialize");
#endif

    for (int i=0;;i++)
        {
        if (g_EmsMessageMap[i].bIsSend==0xFF)
            break;

        if (g_EmsMessageMap[i].bIsSend==0)
            {
            m_mapRx.insert(map<UINT16, UINT32>::value_type(g_EmsMessageMap[i].moc, i));
            }

        if (g_EmsMessageMap[i].bIsSend==1)
            {
            m_mapTx.insert(map<UINT16, UINT32>::value_type(g_EmsMessageMap[i].msgid, i));
            }
        }
}

//Rx map
int CSOCKET::SearchMap(UINT16 ma, UINT16 moc, UINT16 action) const
{
    LOG3( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->SearchMap(ma=0x%X, moc=0x%X, action=0x%X):Rx",ma, moc, action );

    map<UINT16, UINT32>::const_iterator it = m_mapRx.find(moc);
    
    if (it != m_mapRx.end())
        {
        //返回Index.
        return it->second;
        }
    return -1;
}

//Tx map
int CSOCKET::SearchMap(TID tid, UINT16 msgid) const
{
    LOG2( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->SearchMap(tid=0x%X,msgid=0x%X):Tx",UINT16(tid),msgid );

    map<UINT16, UINT32>::const_iterator it = m_mapTx.find(msgid);

    if (it != m_mapTx.end())
        {
        //返回Index.
        return it->second;
        }
    return -1;
}


/*============================================================
MEMBER FUNCTION:
CSOCKET:InitFreeComMsgList

DESCRIPTION:
初始化空闲Buffer链表m_plistFreeComMessage

ARGUMENTS:
NULL

RETURN VALUE:
void

SIDE EFFECTS:
none
==============================================================*/
void CSOCKET::InitFreeComMsgList()
{

    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->InitFreeComMsgList()" );

    m_plistFreeComMessage = NULL;
    FreeComMsgBufferCount = 0;

    for ( UINT16 usIdx = 0; usIdx < M_TSOCKET_BUFFER_NUM; ++usIdx )
    {
        CComMessage  *pComMsg = new ( this, 0 )CComMessage;
        if ( NULL == pComMsg )
        {
            LOG(LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_NORMAL),"TSocket Initialize ComMsg List fail");
            return;
        }

        pComMsg->SetSrcTid( M_TID_EMSAGENTTX ); 
        pComMsg->SetFlag (MSG_BUFFER_FROM_POOL);
        pComMsg->SetBuffer(&m_aucBufferPool[ usIdx ], M_TSOCKET_BUFFER_LENGTH);

        ::taskLock();
        pComMsg->setNext(m_plistFreeComMessage);
        m_plistFreeComMessage = pComMsg;
        FreeComMsgBufferCount ++;
        ::taskUnlock();
    }
}


/*============================================================
MEMBER FUNCTION:
CSOCKET::GetFreeComMsg

DESCRIPTION:
从空闲Buffer链表m_listFreeBuffer取空闲Buffer;(从链表头部取)

ARGUMENTS:
NULL

RETURN VALUE:
UINT8*: 缓冲区指针

SIDE EFFECTS:
none
==============================================================*/
CComMessage *CSOCKET::GetFreeComMsg()
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetFreeComMsg()" );

    ::taskLock();
    if (NULL == m_plistFreeComMessage)
    {
        ::taskUnlock();
        return NULL;
    }

    CComMessage *pComMsg = m_plistFreeComMessage;
    m_plistFreeComMessage    = m_plistFreeComMessage->getNext();
    FreeComMsgBufferCount--;
    ::taskUnlock();

    return pComMsg;
}


/*============================================================
MEMBER FUNCTION:
CSOCKET::ReclaimFreeComMsg

DESCRIPTION:
回收缓冲区，插入回空闲缓冲区链表m_listFreeBuffer

ARGUMENTS:
NULL

RETURN VALUE:
UINT8*: 缓冲区指针

SIDE EFFECTS:
none
==============================================================*/
void CSOCKET::ReclaimFreeComMsg(CComMessage *pComMsg)
{
	  if(m_plistFreeComMessage==pComMsg)
	  	return;
    pComMsg->SetSrcTid(M_TID_EMSAGENTTX);
    pComMsg->SetDstTid(M_TID_MAX);
    pComMsg->SetMessageId(0);
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ReclaimFreeComMsg(ComMsg=0x%X)",UINT32(pComMsg) );
    ::taskLock();
    pComMsg->setNext(m_plistFreeComMessage);
    m_plistFreeComMessage = pComMsg;
    FreeComMsgBufferCount++;
    ::taskUnlock();
}



/*============================================================
MEMBER FUNCTION:
CSOCKET::DeallocateComMessage

DESCRIPTION:
释放ComMessage.

ARGUMENTS:
pComMsg

RETURN VALUE:
true of false

SIDE EFFECTS:
none
==============================================================*/
bool CSOCKET::DeallocateComMessage(CComMessage *pComMsg)
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->DeallocateComMessage()" );

    if ( MSG_BUFFER_FROM_POOL == pComMsg->GetFlag())
    {
        ReclaimFreeComMsg(pComMsg);
        return true;
    }
    else
    {
        //其他释放操作
        return CComEntity::DeallocateComMessage( pComMsg );
    }
}

void CSOCKET::ShowStatus()
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ShowStatus()" );
#ifdef __WIN32_SIM__
    //等ShowStatus信号量
    WAIT();
#endif
    printf( "\r\n**************************************************" );
    printf( "\r\n*TSOCKET Task Attributes                         *" );
    printf( "\r\n**************************************************" );
    printf( "\r\n%-20s: %d", "Task   stack   size",  M_TSOCKET_STACKSIZE);
    printf( "\r\n" );
    printf( "\r\n%-20s: %-5d", "Total Buffers", M_TSOCKET_BUFFER_NUM);
    printf( "\r\n%-20s: %-5d", "Used  Buffers", M_TSOCKET_BUFFER_NUM - FreeComMsgBufferCount);
    printf( "\r\n%-20s: %-5d", "Free  Buffers", FreeComMsgBufferCount);
    printf( "\r\n%-20s: %-5d", "Total CB Entries", M_TSOCKET_BTSADDRNOTIFY_CB_NUM);
    printf( "\r\n%-20s: %-5d", "Used  CB Entries", M_TSOCKET_BTSADDRNOTIFY_CB_NUM-m_listFreeCB.size());
    printf( "\r\n%-20s: %-5d", "Free  CB Entries", m_listFreeCB.size());
    printf( "\r\n%-20s: %-5d", "Total JNT Entries", M_TSOCKET_JAMMINGNEIGHBOR_NUM);
    printf( "\r\n**************************************************" );
    printf( "\r\n*Performance Measurement Status                  *" );
    printf( "\r\n**************************************************" );

    for ( UINT8 type = PERF_TSOCKET_UDP_TOTAL_TX; type < PERF_TSOCKET_MAX; ++type )
    {
        printf( "\r\n%-20s: %d", strPerfType[ type ], m_perf[ type ] );
    }
    printf( "\r\n");
    printf( "\r\n***************************************************" );
    printf( "\r\n*BTS Forwarding Table                             *" );
    printf( "\r\n***************************************************" );
    printf( "\r\n%-20s:: %-5d", "Total FT Entries", M_TSOCKET_FTENTRYNUM_MAX);
    //Free CCB Entries
    printf( "\r\n%-20s:: %-5d entries", "Free    CCB Entries", m_listFreeFT.size() );
    //BPtree
    if ( true == m_FTBptree.empty() )
    {
        printf( "\r\n%-20s:: %s", "Indexed CCB Entries", "0     entries" );
    }
    else
    {
        printf( "\r\n%-20s:: %-5d entries", "Indexed CCB Entries", m_FTBptree.size() );
    }
    printf( "\r\n" );
    printf( "\r\n|%-5s|%-10s|%-15s|%-10s|%-5s|" ,"index","BTSID","IP","PORT","TTL" );

    map<UINT32/*BtsId*/, UINT16>::iterator it = m_FTBptree.begin();
    struct in_addr IpAddr;
    for (;it != m_FTBptree.end();it++)
    {
#ifdef __WIN32_SIM__
        IpAddr.S_un.S_addr = htonl(m_FT[(it->second)].IP);
        printf( "\r\n|%-5d|%-10d|%-15s|%-10d|%-5d|", it->second, it->first, inet_ntoa(IpAddr), m_FT[(it->second)].Port, m_FT[(it->second)].TTL);
#else
        IpAddr.s_addr = htonl(m_FT[(it->second)].IP);
        SINT8 strIpAddr[ INET_ADDR_LEN ] = {0};
        inet_ntoa_b( IpAddr, strIpAddr );
        printf( "\r\n|%-5d|%-8x|%-15s|%-10d|%-5d|",it->second, it->first, strIpAddr, m_FT[(it->second)].Port, m_FT[(it->second)].TTL);
#endif
    }


#ifndef __WIN32_SIM__
    //Show NVRAM FCB
    printf( "\r\n" );
    printf( "\r\n***************************************************" );
    printf( "\r\n*NVRAM FCB table Attributes                       *" );
    printf( "\r\n***************************************************" );
    printf( "\r\n|%-5s|%-10s|%-15s|%-16s|" ,"index","BTSID","IP","PORT" );
    for (int index = 0; index<M_TSOCKET_FTENTRYNUM_MAX; index++)
    {
        if (true==gpNVRamFCBTable[index].IsOccupied)//wangwenhua modify 20090113        
        {
            struct in_addr IpAddr;
            IpAddr.s_addr = htonl(gpNVRamFCBTable[index].IP);
            SINT8 strIpAddr[ INET_ADDR_LEN ] = {0};
            inet_ntoa_b( IpAddr, strIpAddr );

            printf( "\r\n|%-5d|%-8x|%-15s|%-16d|", index, gpNVRamFCBTable[index].BtsId,
                    strIpAddr, gpNVRamFCBTable[index].Port);
        }
    }
    printf("\n");
#endif

    printf( "\r\n");
    printf( "\r\nCurrent BTS ID:%08x",m_curBtsId);
    printf( "\r\nCurrent BTS Frequence:%d",m_curBtsFreq);
    printf( "\r\nCurrent BTS Sequence:%d",m_curBtsSeq);
    printf( "\r\n****************************" );
    printf( "\r\n*BTS Jamming Neighbor Table*" );
    printf( "\r\n****************************" );
    printf( "\r\n|%-10s|%-15s|" ,"SEQ","BTSID" );
    for(int i=0;i<M_TSOCKET_JAMMINGNEIGHBOR_NUM;i++)
    {
        if(0 != JammingTable[i])
        {
#ifdef __WIN32_SIM__
        printf( "\r\n|%-10d|%-15d|", i,JammingTable[i]);
#else
        printf( "\r\n|%-10d|%-15d|", i,JammingTable[i]);
#endif
        }
    }
    printf( "\r\n");

#ifdef __WIN32_SIM__
    //释放showStatus信号量
    RELEASE();
#endif
}

bool CSOCKET::FTBPtreeAdd(UINT32 &btsid, UINT16 index)
{
    LOG2( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->FTBPtreeAdd(btsid=%d, index=%d)",btsid,index );
    pair<map<UINT32, UINT16>::iterator, bool> stPair;
    stPair = m_FTBptree.insert( ValType( btsid, index ) );
    return stPair.second;
}

bool CSOCKET::FTBPtreeDel(UINT32 &btsid)
{
    LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->FTBPtreeDel(btsid=%d)",btsid );
    map<UINT32, UINT16>::iterator it;
    if ((it = m_FTBptree.find( btsid ) ) != m_FTBptree.end())
    {
        m_FTBptree.erase( it );
    }
    return true;
}

UINT16 CSOCKET::FTBPtreeFind(UINT32 &btsid)
{
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->FTBPtreeFind(btsid=%d)",btsid );
    map<UINT32, UINT16>::iterator it = m_FTBptree.find( btsid );
    if ( it != m_FTBptree.end() )
    {
        return it->second;
    }
    return M_TSOCKET_FT_INDEX_ERR;
}

void CSOCKET::FTBPtreeExpire()
{
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->BPtreeExpire()" );
    map<UINT32/*BtsId*/, UINT16>::iterator it = m_FTBptree.begin();
    UINT32 btsIdList[M_TSOCKET_FTENTRYNUM_MAX];
    UINT32 timeOutBtsNum=0;
    list<UINT32> m_listTrash;
    UINT32 btsid = 0;
    while (it!=m_FTBptree.end())
    {
        if (++(m_FT[it->second].TTL) > M_TSOCKET_FTENTRY_EXPIRE)
        {
            btsid = it->first;
            btsIdList[timeOutBtsNum++] = btsid;
        }
	    if(((m_FT[it->second].TTL)%5)==0)//wangwenhua add 20081119
	     {
	        PostBtsIpNotification(btsid);
	     }
        it++;
    }
    for (int i=0; i<timeOutBtsNum; i++ )
    {
        FTDelEntry(btsIdList[i]);
    }
}



UINT16 CSOCKET::GetFreeFTEntryIdxFromList()
{
    ///LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetFreeFTEntryIdxFromList()" );
    if (true == m_listFreeFT.empty())
    {
        g_socket_no_ft_freelist++;
        return M_TSOCKET_FT_INDEX_ERR;
    }
    UINT16 index = *m_listFreeFT.begin();
    m_listFreeFT.pop_front();
    if ( M_TSOCKET_FTENTRYNUM_MAX <= index )
    {
         g_socket_no_ft_freelist++;
        return M_TSOCKET_FT_INDEX_ERR;
    }
	g_socket_no_ft_freelist = 0;
    return index;
}

BtsAddr* CSOCKET::GetFTEntryByIdx(UINT16 index)
{
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetFTEntryByIdx(index=%d)",index );
    if (index >= M_TSOCKET_FTENTRYNUM_MAX)
    {
        return NULL;
    }
    return &( m_FT[ index ] );
}

bool CSOCKET::FTAddEntry(UINT32& btsid, const BtsAddr &addr)
{
    UINT16 index = FTBPtreeFind( btsid );
    BtsAddr *pFT = GetFTEntryByIdx( index );
    if (NULL == pFT)
    {
        //新建转发表表项
        index  = GetFreeFTEntryIdxFromList();
        pFT = GetFTEntryByIdx( index );
        if (NULL == pFT)
        {
            LOG3( LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_NORMAL), "->FTAddEntry(btsid=%d, addr.ip=0x%X, addr.port=0x%X FAILED no free Buffer)",btsid,addr.IP,addr.Port );
            IncreasePerf(PERF_TSOCKET_FT_NO_BUFFER);
            return false;
        }
        FTBPtreeAdd(btsid, index);
    }
    else
    {
        if (pFT->IP == addr.IP && pFT->Port == addr.Port)
        {
            return true;   // do nothing if the address of the BTS is not changed
        }
    }

    LOG3( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->FTAddEntry(btsid=%d, addr.ip=0x%X, addr.port=0x%X)",btsid,addr.IP,addr.Port );
    memcpy(pFT, (void*)&addr, sizeof(BtsAddr)) ;
    pFT->TTL = 0;

    gpNVRamFCBTable[index].IsOccupied = true;
    gpNVRamFCBTable[index].BtsId      = btsid;
    gpNVRamFCBTable[index].IP         = addr.IP;
    gpNVRamFCBTable[index].Port       = addr.Port;

   
    //写入NVRAM
    LOG3( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->AddToNVRAM(btsid:%d,IP:0x%X,PORT:%d)",btsid,addr.IP,addr.Port );
    //打开NVRAM可写开关
    LOG2( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->AddToNVRAM(index%d, addr 0x%x)",index, (UINT32)&gpNVRamFCBTable[index]);
	 #if 0
    stNVRamFCB tmpFcb;
    tmpFcb.IsOccupied = true;    
    tmpFcb.BtsId      = btsid;
    tmpFcb.IP         = addr.IP;
    tmpFcb.Port       = addr.Port;    
    bspNvRamWrite((char*)((char*)gpNVRamFCBTable+index*sizeof(stNVRamFCB)), (char*)&tmpFcb, sizeof(stNVRamFCB));    
    #endif
    m_ptmFTExpire->Start();

    return true;
}

bool CSOCKET::FTDelEntry(UINT32& btsid)
{
    LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->FTDelEntry(btsid=%d)",btsid );
    UINT16 index = FTBPtreeFind(btsid);
    BtsAddr *pFTEntry = GetFTEntryByIdx(index);
    if ( NULL != pFTEntry )
    {
        FTBPtreeDel(btsid);
        memset(pFTEntry, 0, sizeof(BtsAddr));
        m_listFreeFT.push_back( index );   // push the entry back to the free list
        gpNVRamFCBTable[index].IsOccupied = false;

        //从NVRAM中删除
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->DelFromNVRAM(btsid:%d)",btsid);
	  #if 0
	 stNVRamFCB tmpFcb;
	 bspNvRamRead((char *)&tmpFcb, (char*)((char*)gpNVRamFCBTable+index*sizeof(stNVRamFCB)), sizeof(stNVRamFCB));
	 tmpFcb.IsOccupied = false;
	 bspNvRamWrite((char*)((char*)gpNVRamFCBTable+index*sizeof(stNVRamFCB)), (char*)&tmpFcb, sizeof(stNVRamFCB));    
	 #endif
    }
    if (m_FTBptree.size() == 0)
    {
        m_ptmFTExpire->Stop();
    }
    return true;
}

CTimer* CSOCKET::InitTimer(bool bPeriodic, UINT32 timeout, UINT16 msgId)
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->InitTimer()" );
    CComMessage* pComMsg = new (this, 0) CComMessage;
    if (pComMsg==NULL)
        return NULL;

    pComMsg->SetMessageId(msgId);
    pComMsg->SetSrcTid(M_TID_EMSAGENTTX);
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetFlag(MSG_BUFFER_FROM_HEAP);

    CTimer* pTimer = new CTimer(bPeriodic, timeout, pComMsg);
    if (pTimer == NULL)
    {
        pComMsg->Destroy();
        return NULL;
    }
    return pTimer;
}


//UINT8 PostBtsIpNotifyBuf[M_TSOCKET_BUFFER_LENGTH];
void CSOCKET::PostBtsIpNotification(UINT32& btsid)
{
    LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->PostBtsIpNotification(btsid=%x)",btsid );
    //若CB中已经存在该BTS则无须再发送，等超时或者得到回复之后再处理
    if (M_TSOCKET_CB_INDEX_ERR != CBBPtreeFind(btsid))
    {
        return;
    }
    UINT32 msgLen = sizeof(SocketMsgArea)+sizeof(SocketEmsMsgHeader)+sizeof(SocketBtsIpNotification);
    /***********************************************
    *  pComMsg buffer                              *
    *  64b rsv|msgarea|emsheader|btsipnotification *
    ************************************************/

    int mapIndex = SearchMap(M_TID_EMSAGENTTX, M_TSOCKET_BTSIP_NOTIFY);
    if (mapIndex!=-1)
    {
        SocketMsgArea* pArea = (SocketMsgArea*)PostBtsIpNotifyBuf;
        pArea->MsgArea = MSGAREA_EMS;
        SocketEmsMsgHeader* pHead = (SocketEmsMsgHeader*)((UINT8*)pArea+sizeof(SocketMsgArea));
        pHead->ma = htons(g_EmsMessageMap[mapIndex].ma);
        pHead->moc = htons(g_EmsMessageMap[mapIndex].moc);
        pHead->action = htons(g_EmsMessageMap[mapIndex].action);
        pHead->btsid = htonl(bspGetBtsID());
        SocketBtsIpNotification* pNotify = (SocketBtsIpNotification*)((UINT8*)pHead+sizeof(SocketEmsMsgHeader));
        pNotify->BtsId = htonl(btsid); 
        pNotify->TransId = DEFAULT_TRANS_ID;   

        //Control Block
        UINT16 index = GetFreeCBIndexFromList();
//test start
//printf("\r\nAllocate block from CB :btsid=%d,index=%d\r\n",btsid,index);
//test end
        //新建转发表表项
        BtsAddrNotifyCB *pCB = GetCBbyIdx( index );
        if ( NULL == pCB )
        {
            //控制块用光
            IncreasePerf( PERF_TSOCKET_CB_NO_BUFFER );
            LOG( LOG_SEVERE, LOGNO( EMSA, EC_SOCKET_CB_USEDUP ), "No free tSocket CB!" );
//test start
//printf("\r\n task tSOCKET call PostBtsIpNotification --- pComMsg->Destroy(2);\r\n");
//test end
            return;
        }
        //已经申请到CB.
        CBBPtreeAdd( btsid, index );

        //初始化控制块
        pCB->Btsid = btsid;
        pCB->Count = 1;
        pCB->DestIpAddr = gActiveEMSip/*bspGetEmsIpAddr()*/;
        pCB->DestPort = gActiveEMSport/*bspGetEmsUDPRcvPort()*/;
        pCB->Length = msgLen;
        memcpy(pCB->Buf, PostBtsIpNotifyBuf, msgLen);

        //启动定时器
        CTimer *pTimer = StartBtsIpNotifyTimer( btsid, M_TSOCKET_CB_EXPIRE_TIMEOUT );
        if ( NULL == pTimer )
        {
            LOG( LOG_WARN, LOGNO( EMSA, EC_SOCKET_SYS_ERR ), "Start Socket timer fail." );
            CBBPtreeDel( btsid );
            //控制块清0
            memset( pCB, 0, sizeof( BtsAddrNotifyCB ) );
            //插入回空闲链表
            InsertFreeCB( index );
//test start
//printf("\r\nReclaim block to CB :btsid=%d,index=%d\r\n",btsid,index);
//test end
//test start
//printf("\r\n task tSOCKET call PostBtsIpNotification --- pComMsg->Destroy(3);\r\n");
//test end
            return;
        }
        pCB->pTimer = pTimer;

        //Socket发送
        if (!SendBySocket(gActiveEMSip/*bspGetEmsIpAddr()*/,gActiveEMSport/*bspGetEmsUDPRcvPort()*/,PostBtsIpNotifyBuf,msgLen))
        {
            LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket(), error!!!");
        }
    }
//test start
//printf("\r\n task tSOCKET call PostBtsIpNotification --- pComMsg->Destroy(4);\r\n");
//test end
    return;
}
//test start
//int cnt=0;
//test end
void CSOCKET::ProcessBtsIpRequest(CComMessage* pComMsg)
{
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessBtsIpRequest()" );
    if ((m_nChar - sizeof(SocketMsgArea) - sizeof(SocketEmsMsgHeader) - sizeof(SocketBtsIpRequest)) < 0)
    {
        LOG(LOG_WARN, 0, "EMS message error in ProcessBtsIpRequest()");
        return;
    }
   UINT32 btsid;
    SocketBtsIpRequest* pHead = (SocketBtsIpRequest*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED+sizeof(SocketEmsMsgHeader));
    if ((0 == pHead->IP) && (0 == pHead->Port))//如果请求到的IP地址或者端口为0的话，也停止请求 wangwenhua modify 20081215
    {
        PostBtsIpResponse();
        btsid = ntohl(pHead->BtsId);
    }
  else
  {
//test start
//cnt++;
//printf("\r\n tSOCKET get BTSIPADDR from EMS (count=%d)\r\n",cnt);
//test end
    btsid = ntohl(pHead->BtsId);
    BtsAddr addr;
    addr.IP = ntohl(pHead->IP);
    addr.Port = ntohs(pHead->Port);
    if (!FTAddEntry(btsid, addr))
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_NORMAL), "FTAddEntry() failed in ProcessBtsIpRequest()");
    }
    PostBtsIpResponse();
  }
    //停止发送该BTS获取IP和Port的请求
    UINT16 index = CBBPtreeFind( btsid );
    BtsAddrNotifyCB *pCB = GetCBbyIdx( index );
    if ( NULL == pCB )
    {
        return;
    }
    CTimer *pTimer = pCB->pTimer;
    if ( NULL != pTimer )
    {
        pTimer->Stop();
        delete pTimer;
    }
    CBBPtreeDel(btsid);
    memset( pCB, 0, sizeof( BtsAddrNotifyCB ) );
    InsertFreeCB(index);
//test start
//printf("\r\nReclaim block to CB :btsid=%d,index=%d\r\n",btsid,index);
//test end
//test start
//printf("\r\n task tSOCKET call ProcessBtsIpRequest --- pComMsg->Destroy(1);\r\n");
//test end
    return;
}

void CSOCKET::PostBtsIpResponse()
{
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->PostBtsIpResponse()" );

    int index = SearchMap(M_TID_EMSAGENTTX, M_TSOCKET_BTSIP_RSP);
    if (index!=-1)
    {
        SocketMsgArea* pArea = (SocketMsgArea*)PostBtsIpNotifyBuf;
        pArea->MsgArea = MSGAREA_EMS;
        SocketEmsMsgHeader* pHead = (SocketEmsMsgHeader*)((UINT8*)pArea+sizeof(SocketMsgArea));
        pHead->ma = htons(g_EmsMessageMap[index].ma);
        pHead->moc = htons(g_EmsMessageMap[index].moc);
        pHead->action = htons(g_EmsMessageMap[index].action);
        pHead->btsid = htonl(bspGetBtsID());
        SocketBtsIpResponse* pRsp = (SocketBtsIpResponse*)((UINT8*)pHead+sizeof(SocketEmsMsgHeader));
        pRsp->TransId = DEFAULT_TRANS_ID;
        pRsp->Result = 0;
        if (!SendBySocket(gActiveEMSip/*bspGetEmsIpAddr()*/,
                          gActiveEMSport/*bspGetEmsUDPRcvPort()*/,
                          PostBtsIpNotifyBuf,
                          sizeof(SocketMsgArea)+sizeof(SocketEmsMsgHeader)+sizeof(SocketBtsIpResponse)))
        {
            LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket(), error!!!");
        }
    }
//test start
//printf("\r\n task tSOCKET call PostBtsIpResponse --- pComMsg->Destroy(1);\r\n");
//test end
    return;
}

//其他任务用来取得IP地址
bool CSOCKET::GetBtsPubAddr(UINT32 btsid, BtsAddr* addr)
{
    LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetBtsPubAddr(btsid=%x)",btsid );
    UINT16 index = FTBPtreeFind(btsid);
    if (M_TSOCKET_FT_INDEX_ERR == index)
    {
        PostBtsIpNotification(btsid);
        return false;
    }
    BtsAddr *pFT = GetFTEntryByIdx(index);
    if (NULL == pFT)
    {
        return false;
    }
    memcpy(addr, pFT, sizeof(BtsAddr));
    return true;
}
/*
*重写一个接口给EB和SNOOP使用,获得基站ip，port
*/
bool CSOCKET::GetBtsPubAddrByData(UINT32 btsid, BtsAddr* addr)
{
    LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetBtsPubAddrByData(btsid=%x)",btsid );
    UINT16 index = FTBPtreeFind(btsid);
    if (M_TSOCKET_FT_INDEX_ERR == index)
    {
        PostBtsIpReqMsg(btsid);
        return false;
    }
    BtsAddr *pFT = GetFTEntryByIdx(index);
    if (NULL == pFT)
    {
        return false;
    }
    memcpy(addr, pFT, sizeof(BtsAddr));
    return true;
} 
/*
*给socket任务发送消息，请求bts ip
*/
bool CSOCKET::PostBtsIpReqMsg(UINT32 btsid)
{
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->PostBtsIpReqMsg(btsid=%d)",btsid );
    CComMessage* pComMsg = new (this, 0) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_NORMAL), "Allocate empty commessage object failed.");
        return false;
    }
    pComMsg->SetSrcTid(M_TID_EMSAGENTRX);
    pComMsg->SetDstTid(M_TID_EMSAGENTRX);
    pComMsg->SetBTS(btsid);
    pComMsg->SetMessageId(MSGID_OAM_NOTIFY_GET_BTSPUBIP);
    pComMsg->SetFlag(MSG_BUFFER_FROM_HEAP);
    if(!CComEntity::PostEntityMessage(pComMsg))
    {        
        LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "forwarding this msg(0x%X) to socket task failed!!! ",pComMsg->GetMessageId());
        pComMsg->Destroy();
        return false;
    }
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "forwarding this msg(0x%X) to socket task succ!!! ",pComMsg->GetMessageId());
    return true;
} 

/*============================================================
MEMBER FUNCTION:
CSOCKET::InitFreeCBList

DESCRIPTION:
初始化空闲控制块链表m_listFreeCB

ARGUMENTS:
NULL

RETURN VALUE:
bool

SIDE EFFECTS:
none
==============================================================*/
bool CSOCKET::InitFreeCBList()
{
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->InitFreeCBList()" );
    m_listFreeCB.clear();
    for ( UINT16 index = 0; index < M_TSOCKET_BTSADDRNOTIFY_CB_NUM; ++index )
    {
        m_listFreeCB.push_back( index );
    }
    return true;
}

/*============================================================
MEMBER FUNCTION:
CSOCKET::InsertFreeCB

DESCRIPTION:
插入空闲控制块下标到链表m_listFreeCB尾部

ARGUMENTS:
index:表项下标

RETURN VALUE:
bool

SIDE EFFECTS:
none
==============================================================*/
bool CSOCKET::InsertFreeCB(UINT16 index )
{
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->InsertFreeCB(index=%d)",index );
    if( index < M_TSOCKET_BTSADDRNOTIFY_CB_NUM )
    {
        m_listFreeCB.push_back( index );
    }
    else
    {
        return false;
    }
    return true;
}

/*============================================================
MEMBER FUNCTION:
CSOCKET::GetFreeCBIndexFromList

DESCRIPTION:
从空闲链表m_listFreeCB取空闲控制块下标;(从链表头部取)

ARGUMENTS:
NULL

RETURN VALUE:
index:表项下标

SIDE EFFECTS:
none
==============================================================*/
UINT16 CSOCKET::GetFreeCBIndexFromList()
{
    //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetFreeCBIndexFromList()" );
    if ( true == m_listFreeCB.empty() )
    {
      g_socket_no_CB_freelist++;
        return M_TSOCKET_CB_INDEX_ERR;
    }

    UINT16 index = *m_listFreeCB.begin();
    m_listFreeCB.pop_front();

    if ( M_TSOCKET_BTSADDRNOTIFY_CB_NUM <= index )
    {
        //下标错误
        g_socket_no_CB_freelist++;
        LOG1( LOG_WARN, LOGNO( EMSA, EC_SOCKET_CB_INDEX_ERR ), "Err! Free Socket CB Index[%d] Err. ", index );
        return M_TSOCKET_CB_INDEX_ERR;
    }
g_socket_no_CB_freelist = 0;
    return index;
}

BtsAddrNotifyCB* CSOCKET::GetCBbyIdx(UINT16 index)
{
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetCBbyIdx(index=%d)",index );
    if ( index >= M_TSOCKET_BTSADDRNOTIFY_CB_NUM )
    {
        return NULL;
    }
    return &( m_CB[ index ] );
}

bool CSOCKET::CBBPtreeAdd(UINT32& btsid, UINT16 index)
{
    LOG2( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->CBBPtreeAdd(btsid=%d, index=%d)",btsid,index );
    pair<map<UINT32, UINT16>::iterator, bool> stPair;
    stPair = m_CBptree.insert( ValType( btsid, index ) );
    return stPair.second;
}

bool CSOCKET::CBBPtreeDel(UINT32& btsid)
{
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->CBBPtreeDel(btsid=%d)",btsid );
    map<UINT32, UINT16>::iterator it;
    if ( ( it = m_CBptree.find( btsid ) ) != m_CBptree.end() )
    {
        m_CBptree.erase( it );
    }
    return true;
}

UINT16 CSOCKET::CBBPtreeFind(UINT32& btsid)
{
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->CBBPtreeFind(btsid=%d)",btsid );
    map<UINT32, UINT16>::iterator it = m_CBptree.find( btsid );
    if ( it != m_CBptree.end() )
    {
        return it->second;
    }
    return M_TSOCKET_CB_INDEX_ERR;
}

/*============================================================
MEMBER FUNCTION:
CSOCKET::SendBySocket

DESCRIPTION:
通过Socket发送数据

ARGUMENTS:
ip:对端Ip地址
port:对端Port
len:数据长度
*pData:发送数据

RETURN VALUE:
bool

SIDE EFFECTS:
none
==============================================================*/
void CSOCKET::testsocket(UINT32 ip,UINT32 len)
{
     int i,j;
     UINT8 array[100];
    
     for(i = 0; i < 100; i++)
     {
          array[i] = i;
     }
	for(j = 0; j<50;j++)
	{
	SendBySocket( ip, 8002, (void *)array,  len) ;
	}
	
}

void testmbuf(UINT32 ip,UINT32 len)
{
    CSOCKET *p = CSOCKET::GetInstance();
    p ->testsocket( ip, len);
}

UINT8 badMac1[M_MAC_ADDRLEN] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
#ifdef WBBU_CODE
extern   UINT32 dwIP;
#endif
bool CSOCKET::SendBySocket(UINT32 ip, UINT16 port, void *pData, UINT32 len)
{
    if(dwIP==ip)
    	{
    	     LOG2( LOG_WARN, LOGNO( EMSA, EC_SOCKET_SOCKET_ERR ), "tSocket send to self:%x,%x\n", dwIP,ip);
	       return false;
    	}

    LOG3( LOG_DEBUG3, LOGNO( EMSA, EC_SOCKET_NORMAL ), "->SendBySocket(IP:0x%x, PORT:%d, LENGTH:%d)", ip, port, len);
#ifndef WBBU_CODE    
/*
    struct sockaddr_in to;

#ifdef __WIN32_SIM__
    memset( (char*)&to, 0, sizeof( struct sockaddr_in ) );
    to.sin_addr.S_un.S_addr = htonl( ip );
#else
    bzero ( (char*)&to, sizeof( struct sockaddr_in ) ); 
    to.sin_addr.s_addr = htonl( ip );
#endif
    to.sin_family = AF_INET;
    to.sin_port = htons(port);*/

#if 1
    if (SOCKET_ERROR == uiUdpSend(m_fdtSocket,(UINT8*)pData,len,badMac1,htonl( ip ),htons(port)))
    {
#ifdef __WIN32_SIM__
        LOG( LOG_CRITICAL, LOGNO( EMSA, EC_SOCKET_SOCKET_ERR ), "tSocket send packet fails! Err: %d .", WSAGetLastError() );
#else
        LOG4( LOG_DEBUG3, LOGNO( EMSA, EC_SOCKET_SOCKET_ERR ), "tSocket send packet fails(IP:0x%x, PORT:%d, LENGTH:%d, err:%d)!", ip, port, len, errnoGet() );
#endif
        IncreasePerf(PERF_TSOCKET_SOCK_ERR);
        return false;
    }
#endif
#else
    struct sockaddr_in to;

#ifdef __WIN32_SIM__
    memset( (char*)&to, 0, sizeof( struct sockaddr_in ) );
    to.sin_addr.S_un.S_addr = htonl( ip );
#else
    bzero ( (char*)&to, sizeof( struct sockaddr_in ) ); 
    to.sin_addr.s_addr = htonl( ip );
#endif
    to.sin_family = AF_INET;
    to.sin_port = htons(port);

    if (SOCKET_ERROR == ::sendto(m_fdtSocket,(char*)pData,len,0,(struct sockaddr*)&to,sizeof(to)))
    {
#ifdef __WIN32_SIM__
        LOG( LOG_CRITICAL, LOGNO( EMSA, EC_SOCKET_SOCKET_ERR ), "tSocket send packet fails! Err: %d .", WSAGetLastError() );
#else
        LOG4( LOG_WARN, LOGNO( EMSA, EC_SOCKET_SOCKET_ERR ), "tSocket send packet fails(IP:0x%x, PORT:%d, LENGTH:%d, err:%d)!", ip, port, len, errnoGet() );
#endif
        IncreasePerf(PERF_TSOCKET_SOCK_ERR);
        return false;
    }
#endif
//  IncreasePerf(PERF_TSOCKET_UDP_PACKETS);
    return true;
}

/*============================================================
MEMBER FUNCTION:
CSOCKET::StartBtsIpNotifyTimer

DESCRIPTION:
启动定时器

ARGUMENTS:
btsid
millsecs: 定时器时长(毫秒):

RETURN VALUE:
CTimer*:返回定时器指针

SIDE EFFECTS:
none
==============================================================*/
CTimer* CSOCKET::StartBtsIpNotifyTimer( UINT32 &btsid, UINT32 millsecs )
{
    LOG2( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->StartBtsIpNotifyTimer(btsid=%d,millsecs=%d)",btsid,millsecs );

    CComMessage* pComMsg = new (this, 0) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_NORMAL), "Allocate empty commessage object failed.");
        return NULL;
    }
    pComMsg->SetSrcTid(this->GetEntityId());
    pComMsg->SetDstTid(this->GetEntityId());
    pComMsg->SetBTS(btsid);
    pComMsg->SetMessageId(M_TSOCKET_TIMER_CB_EXPIRE);
    pComMsg->SetFlag(MSG_BUFFER_FROM_HEAP);

    CTimer *pTimer = new CTimer( false, millsecs, pComMsg );
    if ( NULL == pTimer )
    {
        LOG( LOG_WARN, LOGNO( EMSA, EC_SOCKET_SYS_ERR ), "Socket create timer failed." );
        pComMsg->Destroy();
        return NULL;
    }
    if ( false == pTimer->Start() )
    {
        LOG( LOG_WARN, LOGNO( EMSA, EC_SOCKET_SYS_ERR ), "Timer start err." );
        delete pTimer;
        pComMsg->Destroy();
        return NULL;
    }
    return pTimer;
}

/*============================================================
MEMBER FUNCTION:
CSOCKET::BtsIpNotifyTimeout

DESCRIPTION:
处理CSOCKET任务的定时器超时消息

ARGUMENTS:
msgTimerExpire: 超时消息

RETURN VALUE:
void 

SIDE EFFECTS:
none
==============================================================*/
void CSOCKET::BtsIpNotifyTimeout(const CComMessage* pComMsg)
{
    LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->BtsIpNotifyTimeout()" );
    UINT32 btsid = pComMsg->GetBTS();
    UINT16 index = CBBPtreeFind( btsid );
    BtsAddrNotifyCB *pCB = GetCBbyIdx( index );
    if ( NULL == pCB )
    {
        return;
    }

    UINT8 count = pCB->Count;
    if ( count < M_TSOCKET_MAX_RETRY_COUNT )
    {
        //重发消息；
        SendBySocket(pCB->DestIpAddr, pCB->DestPort, pCB->Buf, pCB->Length);
        pCB->Count++;
        //重启定时器
        if ( NULL != pCB->pTimer)
        {
            pCB->pTimer->Stop();
            delete pCB->pTimer;
        }
        CTimer *pTimer = StartBtsIpNotifyTimer( btsid, M_TSOCKET_CB_EXPIRE_TIMEOUT );
        if ( NULL == pTimer )
        {
            LOG( LOG_WARN, LOGNO( EMSA, EC_SOCKET_SYS_ERR ), "Start Socket timer fail." );
            CBBPtreeDel( btsid );
            //控制块清0
            memset( pCB, 0, sizeof( BtsAddrNotifyCB ) );
            //插入回空闲链表
            InsertFreeCB( index );
            return;
        }
        pCB->pTimer = pTimer;
    }
    else
    {
        //重发结束并删除控制块
        CTimer *pTimer = pCB->pTimer;
        if ( NULL != pTimer )
        {
            pTimer->Stop();
            delete pTimer;
        }
        //删控制块
        CBBPtreeDel(btsid);
        //控制块清0
        memset( pCB, 0, sizeof( BtsAddrNotifyCB ) );
        //插入回空闲链表
        InsertFreeCB(index);
//test start
//printf("\r\nReclaim block to CB :btsid=%d,index=%d\r\n",btsid,index);
//test end
    }
    return;
}


//更新BTSID的PubIP&Port
void CSOCKET::ProcessUpdateBtsPubIp(CComMessage* pComMsg)
{
#ifndef __WIN32_SIM__
    CMessage msg(pComMsg);
    CUpdateBTSPubIp mUpdateIp(msg);
    UINT32 btsid = mUpdateIp.GetBTSID();
    LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessUpdateBtsPubIp(BTSID=%d)",btsid );
    BtsAddr addr;
    addr.IP = mUpdateIp.GetBTSPubIP();
    addr.Port = mUpdateIp.GetBTSPubPort();
    if (!FTAddEntry(btsid, addr))
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_NORMAL), "FTAddEntry() failed in ProcessUpdateBtsPubIp()");
    }
#else
    return;
#endif
}
//clear FT
void CSOCKET::clearFT()
{
    LOG(LOG_CRITICAL, 0, "Socket Task BTS Forward Table is Reinitialized");
    if (m_ptmFTExpire != NULL)
    {
        m_ptmFTExpire->Stop();
    }
    //初始化转发表
    memset(m_FT, 0, sizeof(m_FT));
    m_FTBptree.clear();
    m_listFreeFT.clear();
    for ( UINT16 index = 0; index < M_TSOCKET_FTENTRYNUM_MAX; ++index )
    {
        m_listFreeFT.push_back( index );
    }

    LOG(LOG_CRITICAL, 0, "Socket Task NVRAM Forward Table is Reinitialized");    
    memset( (char*)gpNVRamFCBTable, 0, NVRamFCBTableLength );
	#if 0
    if(bspEnableNvRamWrite( (char*)gpNVRamFCBTable,  NVRamFCBTableLength)==TRUE)
    {
        memset( (char*)gpNVRamFCBTable, 0, NVRamFCBTableLength );
        bspDisableNvRamWrite( (char*)gpNVRamFCBTable,  NVRamFCBTableLength);
    }
	#endif
}


/*============================================================
MEMBER FUNCTION:
SShow

DESCRIPTION:
用于Tornado Shell上调用执行

ARGUMENTS:
NULL

RETURN VALUE:
void

SIDE EFFECTS:
none
==============================================================*/
extern "C" STATUS SShow()
{

    CSOCKET *taskSocket = CSOCKET::GetInstance();
    taskSocket->ShowStatus();
    return OK;
}

extern "C" STATUS SSClear(/*UINT8 type*/)
{
    CSOCKET *taskSocket = CSOCKET::GetInstance();
    taskSocket->clearFT();
    return OK;
}




/*============================================================
MEMBER FUNCTION:
CSOCKET::RecoverFTfromFCB

DESCRIPTION:
从NVRAM删除已经存在的FCB.

ARGUMENTS:

RETURN VALUE:
void

SIDE EFFECTS:
none
==============================================================*/
void CSOCKET::RecoverFTfromFCB()
{
    LOG( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_NORMAL), "->RecoverFTfromFCB");
    stDataSocketNVRAMhdr *nvramHeader = (stDataSocketNVRAMhdr*)NVRAM_TASK_SOCKET_DATA_BASE;
    if (    M_TSOCKET_NVRAM_INITIALIZED != nvramHeader->Initialized 
         || bspGetBtsID() != nvramHeader->LocalBtsId)
    {     //第一次启动或启动前后配置的BTSID不一致，NVRAM中的转发表不恢复
        clearFT();
	 nvramHeader->Initialized = M_TSOCKET_NVRAM_INITIALIZED;
        nvramHeader->LocalBtsId = bspGetBtsID();
	  #if 0
        if(bspEnableNvRamWrite( (char*)nvramHeader,  sizeof(stDataSocketNVRAMhdr))==TRUE)
    	 {
            nvramHeader->Initialized = M_TSOCKET_NVRAM_INITIALIZED;
            nvramHeader->LocalBtsId = bspGetBtsID();
            bspDisableNvRamWrite( (char*)gpNVRamFCBTable,  NVRamFCBTableLength);
    	 }
	 #endif
        return;
    }

    m_FTBptree.clear();
    m_listFreeFT.clear();

    //逐项查询NVRAM
    for ( UINT16 index = 0; index < M_TSOCKET_FTENTRYNUM_MAX; index++ )
    {
        if ( true == gpNVRamFCBTable[index].IsOccupied )
        {
            //表项需要恢复到内存, use the same index as in NVRAM table
            UINT32 btsid =  gpNVRamFCBTable[index].BtsId;
            BtsAddr *pFT = GetFTEntryByIdx( index );
            pFT->IP = gpNVRamFCBTable[index].IP;
            pFT->Port = gpNVRamFCBTable[index].Port;
            pFT->TTL = 0;

            FTBPtreeAdd(btsid, index);
        }
        else
        {   // push the entry to free list
            m_listFreeFT.push_back( index );
        }
    }

    if (m_FTBptree.size() > 0)
    {
        m_ptmFTExpire->Start();
    }
    return;
}
void CSOCKET::GetPerfData(UINT8 *pData,UINT8 ucType)
{
    if (ucType==BTS_PERF_TYPE_TCR)
    {
        //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetPerfData,PerfData Type is[TCR]");
        memcpy( pData, &m_perf[PERF_TSOCKET_TCR_ERR], 2*sizeof( UINT32 ));
    }
    else
    {
        //LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->GetPerfData,PerfData Type is[TDR]");
        UINT32 ulPerf[3];
        ulPerf[0]= m_perf[PERF_TSOCKET_TDR_ERR];
        ulPerf[1]= m_perf[PERF_TSOCKET_TDR_PACKETS];
        ulPerf[2]= m_perf[PERF_TSOCKET_NO_BUFFER];
        memcpy( pData, ulPerf, sizeof( ulPerf ));
    }
}

void CSOCKET::ProcessJNTRefreshNotify(CComMessage* pComMsg)
{
    LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessJNTRefreshNotify()" );
    UINT16 NeigBtsNum = NvRamData->BtsNeighborCommCfgEle.NeighborBtsNum;
    if(NeigBtsNum > NEIGHBOR_BTS_NUM)
    {
        return;
    }

    //TODO:收到这条消息就读取NVRAM将Bts的当前频率和JNT更新
////CMessage msg(pComMsg);
////CDataRefreshJammingNeighbor Notify(msg);

    m_curBtsId = bspGetBtsID();
    m_curBtsSeq = NvRamData->AirLinkCfgEle.SequenceID;
    m_curBtsFreq = NvRamData->RfCfgEle.StartFreqIndex;
    LOG3( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), " Get m_curBtsId(%d),m_curBtsSeq(%d),GetBtsFreq(%d) from NVRAM",m_curBtsId,m_curBtsSeq,m_curBtsFreq );

    //首先清除JammingNeighborList
    memset(JammingTable, 0, sizeof(JammingTable));
    LOG1( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), " Get neighborNum(%d) from NVRAM",NeigBtsNum);
   // UINT16 RepeaterNum = 0, repeaterLen = 0;
    char *pdata = (char*)(NvRamData->BtsNeighborCommCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
    for(int i = 0; i < NeigBtsNum; ++i)
    {
        T_BtsNeighborCfgData *pNeighborDataElement = (T_BtsNeighborCfgData*)pdata;

        if( (M_TSOCKET_JAMMINGNEIGHBOR_NUM > pNeighborDataElement->BtsInfoIE.SequenceID)
            &&(abs(m_curBtsFreq - pNeighborDataElement->BtsInfoIE.FrequencyIndex)<100) 
            &&(m_curBtsSeq  != pNeighborDataElement->BtsInfoIE.SequenceID))
        {
            JammingTable[pNeighborDataElement->BtsInfoIE.SequenceID]
                = pNeighborDataElement->BtsInfoIE.BTSID;
        }
        else
        {
            LOG2( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "Please make sure new entry 's freq =%d and seq != %d ",m_curBtsFreq,m_curBtsSeq);
        }
        //跳到下一个neighbor BTS
        UINT16 usElementLen = pNeighborDataElement->length();
        if (0xFFFF == usElementLen)
        {
            return;
        }
        pdata += usElementLen;
    }

    PostJammingBtsInfoIeToL2();
    return;
}

void CSOCKET::PostJammingBtsInfoIeToL2()
{
    OAM_LOGSTR(LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "Update L2 Jamming BTS info to L2.");

    CComMessage* pComMsg = new (this, M_TSOCKET_BUFFER_LENGTH) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, LOGNO(EMSA, EC_SOCKET_NORMAL), "Allocate empty commessage object failed.");
        return ;
    }
    pComMsg->SetSrcTid(this->GetEntityId());
    pComMsg->SetDstTid(M_TID_L2MAIN);
    pComMsg->SetBTS(m_curBtsId);
    pComMsg->SetMessageId(M_SOCKET_JAMMING_NEIGHBOR_NOTIFY);
    pComMsg->SetFlag(MSG_BUFFER_FROM_HEAP);
    //pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr() + M_DEFAULT_RESERVED));
    //pComMsg->SetDataLength(M_TSOCKET_BUFFER_LENGTH-M_DEFAULT_RESERVED);

    //首先清除JammingNeighborList
    //memset(pComMsg->GetDataPtr(), 0, sizeof(JammingNeighborConfigNotify));
    
    UINT16 NeigBtsNum = NvRamData->BtsNeighborCommCfgEle.NeighborBtsNum;
    if(NeigBtsNum > NEIGHBOR_BTS_NUM)
    {
        return;
    }

    JammingNeighborConfigNotify* pNotify = (JammingNeighborConfigNotify*)pComMsg->GetDataPtr();
    pNotify->TransId = DEFAULT_TRANS_ID;
    pNotify->NeighborNum = 0;
    UINT32 jammingNeighborNum = 0;  
 //   UINT16 RepeaterNum = 0, repeaterLen = 0;
    char *pdata = (char*)(NvRamData->BtsNeighborCommCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
    for(int i = 0; i < NeigBtsNum; ++i)
    {
        T_BtsNeighborCfgData *pNeighborDataElement = (T_BtsNeighborCfgData*)pdata;

        if( (M_TSOCKET_JAMMINGNEIGHBOR_NUM > pNeighborDataElement->BtsInfoIE.SequenceID)
            &&(abs(m_curBtsFreq - pNeighborDataElement->BtsInfoIE.FrequencyIndex) < 100) 
            &&(m_curBtsSeq  != pNeighborDataElement->BtsInfoIE.SequenceID))
        {
            if(jammingNeighborNum>=M_TSOCKET_JAMMINGNEIGHBOR_NUM)
            {
                pComMsg->Destroy();
                return;
            }
            memcpy((UINT8*)&(pNotify->BtsInfoIe[jammingNeighborNum]),
                   (UINT8*)&(pNeighborDataElement->BtsInfoIE),
                   sizeof(T_BtsInfoIE));
            jammingNeighborNum++;
        }

        //跳到下一个neighbor BTS
        UINT16 usElementLen = pNeighborDataElement->length();
        if (0xFFFF == usElementLen)
        {
            pComMsg->Destroy();
            return;
        }
        pdata += usElementLen;
        
    }
    pNotify->NeighborNum = jammingNeighborNum;
    
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_JAMMING_BTS_ERR);
        LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "forwarding this msg(0x%X) to tL2Main failed!!! ",pComMsg->GetMessageId() );

        pComMsg->Destroy();

        return;
    }
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "forwarding this msg(0x%X) to tL2Main successed!!! ",pComMsg->GetMessageId() );
    IncreasePerf(PERF_TSOCKET_JAMMING_BTS_PACKETS);
    return;
}

void CSOCKET::ProcessJammingFromL2(CComMessage* pComMsg)
{
    LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessJammingFromL2()" );
    BtsAddr addr;
    UINT32 btsid;
    UINT16 seqid;
//    UINT8* dataptr = NULL;
   // UINT32 count=0;
 //   UINT32 datalen = 0;
    SocketMsgArea* pArea = (SocketMsgArea*)((UINT8*)pComMsg->GetDataPtr()-sizeof(SocketMsgArea));
    //增加Header的时候，超过了Buffer预留的64 rsv
    if (pArea < pComMsg->GetBufferPtr())
    {
        LOG(LOG_WARN, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), "Commmessage dataptr exceed bufferptr, error!!!");
        return;
    }
    
    UINT16 msgid = pComMsg->GetMessageId();
    if(BTS_Jamming_Rpt == msgid)
    {
        LOG1(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), " tSOCKET send BTS_Jamming_Rpt msg(addr:0x%X, from tL2Main)to WAN ",(int)pComMsg->GetDataPtr());

	IncreasePerf(PERF_TSOCKET_JAMMING_L2_PACKETS);
	//lijinan 0090105 for jamming rpt flow ctl
	if(stJammingRptCtl.L2TimerIsStart==FLAG_NO)
	{
			if(stJammingRptCtl.pFromL2Timer==NULL)
			{
				CComMessage* pMsg = NULL;
			       pMsg = new (this, 0) CComMessage;
				pMsg->SetDstTid(M_TID_EMSAGENTTX);
				pMsg->SetSrcTid(M_TID_EMSAGENTTX);
				pMsg->SetMessageId(M_TSOCKET_TIMER_JAMMINGL2_EXPIRE);
				pMsg->SetDataLength(0);

				stJammingRptCtl.pFromL2Timer  = new CTimer( false, 1000, pMsg );
				if(stJammingRptCtl.pFromL2Timer==NULL)
					pMsg->Destroy();
				else
				{
					stJammingRptCtl.pFromL2Timer->Start();	
					stJammingRptCtl.L2TimerIsStart = FLAG_YES;
				}
			}
			else
			{
				stJammingRptCtl.pFromL2Timer->Start();	
				stJammingRptCtl.L2TimerIsStart = FLAG_YES;
			}
			stJammingRptCtl.jammingFromL2Cnt++;
	
	}
	else
	{
		stJammingRptCtl.jammingFromL2Cnt++;
		if(stJammingRptCtl.jammingFromL2Cnt>JammingMaxNumFromL2)
		{
			IncreasePerf(PERF_TSOCKET_JAMMING_L2_ERR);
		 	LOG1(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), " tSOCKET send BTS_Jamming_Rpt msg(addr:0x%X, from tL2Main)to WAN,but the jamming rpt cnt over MaxNum from L2",\
				(int)pComMsg->GetDataPtr());
			return;
		}

		
	}
        //test start
        //printf("\r\n tSOCKET send BTS_Jamming_Rpt msg(addr:0x%X, from tL2Main)to WAN \r\n",pComMsg->GetDataPtr());
        //test stop
        //PrintMsgData(pComMsg);
        pArea->MsgArea = MSGAREA_JAMMING_REPORT;
        
        //TODO:这里要解析该消息并转发到相应BTS
        stBtsJammingRptL3* pRpt = (stBtsJammingRptL3*)((UINT8*)pComMsg->GetDataPtr());

        //lrc 100121
        if(pRpt->cpeJammingRptInfo.OBSInfo.Rsv1)//0-old mode , 1-new mode
        {
            stOBSNew* pOBSNew = (stOBSNew*)&pRpt->cpeJammingRptInfo.OBSInfo;
            
            UINT16 seqIdMask = (pRpt->cpeJammingRptInfo.macCtrlInfo.RSV<<8) + pOBSNew->SeqIdMaskLow;
            
            for(int i=0; i<M_TSOCKET_JAMMINGNEIGHBOR_NUM; i++)
            {
                if(seqIdMask&(1<<i))
                {
                    ForwardingJammingRpt(i,pArea,sizeof(SocketMsgArea)+pComMsg->GetDataLength());
                    
                }
            }
        }
        else
        {
            seqid = pRpt->cpeJammingRptInfo.OBSInfo.OBS0;

            if(seqid<M_TSOCKET_JAMMINGNEIGHBOR_NUM)
            {
                ForwardingJammingRpt(seqid,pArea,sizeof(SocketMsgArea)+pComMsg->GetDataLength());
                
            }
            seqid = pRpt->cpeJammingRptInfo.OBSInfo.OBS1;

            if(seqid<M_TSOCKET_JAMMINGNEIGHBOR_NUM)
            {
                ForwardingJammingRpt(seqid,pArea,sizeof(SocketMsgArea)+pComMsg->GetDataLength());
                
            }
        }
		/* //maqiang 080512
        seqid = pRpt->cpeJammingRptInfo.OBSInfo.OBS2;
        if(seqid<M_TSOCKET_JAMMINGNEIGHBOR_NUM)
        {
            ForwardingJammingRpt(seqid,pArea,sizeof(SocketMsgArea)+pComMsg->GetDataLength());
        }*/

        return;
    }
    else if(BTS_Jamming_Rpt_Rsp == msgid)
    {
        LOG1(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_NORMAL), " tSOCKET send BTS_Jamming_Rpt_Rsp msg(addr:0x%X,from tL2Main) to WAN  ",(int)pComMsg->GetDataPtr());
        pArea->MsgArea = MSGAREA_JAMMING_REPORT_RSP;

        stBtsJammingRptRspL3* pRpt = (stBtsJammingRptRspL3*)((UINT8*)pComMsg->GetDataPtr());
        btsid  = pRpt->dstBtsId;
        if(!GetBtsPubAddr(btsid, &addr))
        {
            LOG1(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_NORMAL), " tSOCKET :when forwarding BTS_Jamming_Rpt_Rsp to wan,can not get btsid(%d) 's ip&port ",btsid);
            return;
        }
        if(!SendBySocket(addr.IP,addr.Port,pArea,sizeof(SocketMsgArea)+pComMsg->GetDataLength()))
        {
            LOG(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket(), error!!!");
            return;
        }
    }
    else if(BTS_PairedCpe_Prof_Msg == msgid)
    {
        LOG1(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_NORMAL), " tSOCKET send BTS_PairedCpe_Prof_Msg msg(addr:0x%X,from tL2Main) to WAN  ",(int)pComMsg->GetDataPtr());
        pArea->MsgArea = MSGAREA_PAIREDCPE_PROF;

        stBtsPairedCpeProfMsgL3* pProf = (stBtsPairedCpeProfMsgL3*)((UINT8*)pComMsg->GetDataPtr());
        btsid  = pProf->dstBtsId;
        if(!GetBtsPubAddr(btsid, &addr))
        {
            LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_SYS_ERR), " tSOCKET :when forwarding BTS_Jamming_Rpt_Rsp to wan,can not get btsid(%d) 's ip&port ",btsid );
            return;
        }
        if(!SendBySocket(addr.IP,addr.Port,pArea,sizeof(SocketMsgArea)+pComMsg->GetDataLength()))
        {
            LOG(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket(), error!!!");
            return;
        }
    }
#if 1//def M_CLUSTER_SAME_F
    else if(BTS_GROUP_RESOURCE_Rpt_Rsp == msgid)
    {
        LOG1(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), " tSOCKET send BTS_GROUP_RESOURCE_Rpt_Rsp msg(addr:0x%X, from tL2Main)to WAN ",(int)pComMsg->GetDataPtr());
        pArea->MsgArea = MSGAREA_GROUP_RESOURCE_RPT_RSP;
        
        //TODO:这里要解析该消息并转发到相应BTS
        stBtsGroupResourceRptRsp* pRpt = (stBtsGroupResourceRptRsp*)((UINT8*)pComMsg->GetDataPtr());
        pRpt->OBSNum = pRpt->OBSNum < M_TSOCKET_JAMMINGNEIGHBOR_NUM ? pRpt->OBSNum : (M_TSOCKET_JAMMINGNEIGHBOR_NUM-1);

        for(int i=0; i<pRpt->OBSNum; i++)
        {
            seqid = pRpt->OBSSeqId[i];
            if(seqid<M_TSOCKET_JAMMINGNEIGHBOR_NUM)
            {                
                ForwardingJammingRpt(seqid,pArea,sizeof(SocketMsgArea)+pComMsg->GetDataLength());
            }
            else
            {
                LOG1(LOG_DEBUG1, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), " tSOCKET send BTS_GROUP_RESOURCE_Rpt_Rsp to WAN,seqID[%d] Err",seqid);
            }
        }
        return;
    }
#endif
    else
    {
        LOG1(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "Message(msgId : 0x%X) From L2 , error!!!",msgid);
    }

    return;
}
 bool CSOCKET::ProcessJammingFromBts(CComMessage* pComMsg,UINT16 nChar)
{
    LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessJammingFromBts()" );

    pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr() + M_DEFAULT_RESERVED));
    pComMsg->SetDataLength(nChar-sizeof(SocketMsgArea));
    pComMsg->SetDstTid(M_TID_L2MAIN);

    SocketMsgArea* pMsgArea = (SocketMsgArea*)((UINT8*)pComMsg->GetBufferPtr()+M_DEFAULT_RESERVED-sizeof(SocketMsgArea));
    switch(pMsgArea->MsgArea)
    {
    case MSGAREA_JAMMING_REPORT:
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "received MSGAREA_JAMMING_REPORT msg(0x%X) from WAN ",pComMsg->GetMessageId() );
        //PrintMsgData(pComMsg);
        pComMsg->SetMessageId(BTS_Jamming_Rpt);
        break;
    case MSGAREA_JAMMING_REPORT_RSP:
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "received MSGAREA_JAMMING_REPORT_RSP msg(0x%X) from WAN ",pComMsg->GetMessageId() );
        //PrintMsgData(pComMsg);
        pComMsg->SetMessageId(BTS_Jamming_Rpt_Rsp);
        break;
    case MSGAREA_PAIREDCPE_PROF:
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "received MSGAREA_PAIREDCPE_PROF msg(0x%X) from WAN ",pComMsg->GetMessageId() );
        //PrintMsgData(pComMsg);
        pComMsg->SetMessageId(BTS_PairedCpe_Prof_Msg);
        break;
#if 1//def M_CLUSTER_SAME_F
    case MSGAREA_GROUP_RESOURCE_RPT_RSP:
        LOG1( LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_MSG_EXCEPTION), "received MSGAREA_GROUP_RESOURCE_RPT_RSP msg(0x%X) from WAN ",pComMsg->GetMessageId() );
        pComMsg->SetMessageId(BTS_GROUP_RESOURCE_Rpt_Rsp);
		break;
#endif
    default:
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "received msg(0x%X) from WAN, error MsgArea!!,it will be Destroy ",pComMsg->GetMessageId() );
        //PrintMsgData(pComMsg);
        return false;
    }

   //lijinan 090105 for jamming rpt flow ctl
   if(pMsgArea->MsgArea==MSGAREA_JAMMING_REPORT)
   {
   	if(sysStateIsRun()!=1)
   		{
	   		IncreasePerf(PERF_TSOCKET_JAMMING_BTS_ERR);
			return false;
		}
	//lijinan 0090105 for jamming rpt flow ctl
	if(stJammingRptCtl.BtsTimerIsStart==FLAG_NO)
	{
		if(stJammingRptCtl.pFromBtsTimer==NULL)
		{
			CComMessage* pMsg = NULL;
		       pMsg = new (this, 0) CComMessage;
			pMsg->SetDstTid(M_TID_EMSAGENTTX);
			pMsg->SetSrcTid(M_TID_EMSAGENTTX);
			pMsg->SetMessageId(M_TSOCKET_TIMER_JAMMINGBTS_EXPIRE);
			pMsg->SetDataLength(0);

			stJammingRptCtl.pFromBtsTimer  = new CTimer( false, 1000, pMsg );
			if(stJammingRptCtl.pFromBtsTimer==NULL)
				pMsg->Destroy();
			else
			{
				stJammingRptCtl.pFromBtsTimer->Start();	
				stJammingRptCtl.BtsTimerIsStart = FLAG_YES;
			}
		}
		else
		{
				stJammingRptCtl.pFromBtsTimer->Start();	
				stJammingRptCtl.BtsTimerIsStart = FLAG_YES;
		}
		stJammingRptCtl.jammingFromBtsCnt++;
	
	}
	else
	{
		stJammingRptCtl.jammingFromBtsCnt++;
		if(stJammingRptCtl.jammingFromBtsCnt>JammingMaxNumFromBts)
		{
			IncreasePerf(PERF_TSOCKET_JAMMING_BTS_ERR);
		 	LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "received MSGAREA_JAMMING_REPORT msg(0x%X) from WAN,but jamming num over Maxnum from bts ",pComMsg->GetMessageId() );
			return false;
		}

		
	}
   }
   
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_JAMMING_BTS_ERR);
        LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "forwarding this msg(0x%X) to tL2Main failed!!! ",pComMsg->GetMessageId() );
        return false;
    }
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "forwarding this msg(0x%X) to tL2Main successed!!! ",pComMsg->GetMessageId() );
    IncreasePerf(PERF_TSOCKET_JAMMING_BTS_PACKETS);
    return true;
}

void CSOCKET::ForwardingJammingRpt(UINT16 seqid,SocketMsgArea* pArea,UINT32 dataLen)
{
    LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ForwardingJammingRpt()" );
    /***********************************************
    *  pComMsg buffer                              *
    *  64b rsv|msgarea|emsheader|btsipnotification *
    ************************************************/
    BtsAddr addr;
    if(!GetBtsPubAddr(JammingTable[seqid], &addr))
    {
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "when forwarding ForwardingJammingRpt to wan,can not get btsid(%d) 's ip&port ",JammingTable[seqid]);
        return;
    }
    if(!SendBySocket(addr.IP,addr.Port,pArea,dataLen))
    {
        LOG(LOG_DEBUG3, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), "SendBySocket(), error!!!");
        return;
    }   
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "ForwardingJammingRpt successed!!! ");
}

void CSOCKET::InitialJNT()
{
//debug start
//printf("\r\n direction ----<8>\r\n");
//debug stop
    LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), "->InitialJNT()");
    UINT16 NeigBtsNum = NvRamData->BtsNeighborCommCfgEle.NeighborBtsNum;
    if(NeigBtsNum > NEIGHBOR_BTS_NUM)
    {
        return;
    }
    memset(JammingTable, 0, sizeof(JammingTable));
    m_curBtsId = bspGetBtsID();
    m_curBtsSeq = NvRamData->AirLinkCfgEle.SequenceID;
    m_curBtsFreq = NvRamData->RfCfgEle.StartFreqIndex;

    LOG1( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_SYS_ERR), " Get neighborNum(%d) from NVRAM ",NeigBtsNum);
 //   UINT16 RepeaterNum = 0, repeaterLen = 0;
    char *pdata = (char*)(NvRamData->BtsNeighborCommCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
    for(int i = 0; i < NeigBtsNum; ++i)
    {
        T_BtsNeighborCfgData *pNeighborDataElement = (T_BtsNeighborCfgData*)pdata;
        if( (M_TSOCKET_JAMMINGNEIGHBOR_NUM > pNeighborDataElement->BtsInfoIE.SequenceID)
            &&(abs(m_curBtsFreq - pNeighborDataElement->BtsInfoIE.FrequencyIndex) < 100) 
            &&(m_curBtsSeq  != pNeighborDataElement->BtsInfoIE.SequenceID))
        {
            JammingTable[pNeighborDataElement->BtsInfoIE.SequenceID]
                =pNeighborDataElement->BtsInfoIE.BTSID;
        }
        else
        {
            LOG2( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "Please make sure new entry 's freq =%d and seq != %d ",m_curBtsFreq,m_curBtsSeq);
        }
        //跳到下一个neighbor BTS
        UINT16 usElementLen = pNeighborDataElement->length();
        if (0xFFFF == usElementLen)
        {
            return;
        }
        pdata += usElementLen;
    }
    return;
}

bool CSOCKET::ProcessGrpSrvHOMsgFromOtherBTS
	(CComMessage* pComMsg,UINT16 nChar)
{
    LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), 
		"->ProcessGrpSrvHOMsgFromOtherBTS()" );

    pComMsg->SetDataPtr((void*)((UINT8*)pComMsg->GetBufferPtr() 
		+ M_DEFAULT_RESERVED));
    pComMsg->SetDataLength(nChar-sizeof(SocketMsgArea));
    pComMsg->SetDstTid(M_TID_VOICE);
    

    SocketMsgArea* pMsgArea = (SocketMsgArea*)((UINT8*)pComMsg->GetBufferPtr()
		+M_DEFAULT_RESERVED-sizeof(SocketMsgArea));
    switch(pMsgArea->MsgArea)
    {
    case MSGAREA_GRPSRV_HORESREQ:
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), 
			"received MSGAREA_GRPSRV_HORESREQ msg(0x%X) from WAN ",
			pMsgArea->MsgArea );
        pComMsg->SetMessageId(MSGID_GRP_HO_RES_REQ);
        break;
    case MSGAREA_GRPSRV_HORESRSP:
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), 
			"received MSGAREA_GRPSRV_HORESRSP msg(0x%X) from WAN ",
			pMsgArea->MsgArea );
        pComMsg->SetMessageId(MSGID_GRP_HO_RES_RSP);
        break;
    default:
        LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), 
			"received msg(0x%X) from WAN, error MsgArea!!,Destroy ",
			pMsgArea->MsgArea );
        return false;
    }

    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        IncreasePerf(PERF_TSOCKET_GRPSRV_HO_ERR);
        LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_SYS_ERR), 
			"forwarding this msg(0x%X) to tVoice failed!!! ",pComMsg->GetMessageId() );
        return false;
    }
    LOG1( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), 
		"forwarding this msg(0x%X) to tVoice successed!!! ",pComMsg->GetMessageId() );
    IncreasePerf(PERF_TSOCKET_GRPSRV_HANDOVER);
    return true;	
}
void CSOCKET::ProcessGrpSrvHOMsg2OtherBTS(CComMessage* pComMsg)
{
	LOG( LOG_DEBUG2, LOGNO(EMSA,EC_SOCKET_NORMAL), "->ProcessGrpSrvHOMsg2OtherBTS()" );
	UINT16 msgID = pComMsg->GetMessageId();
#if 0	
	SocketMsgArea* pMsgArea = (SocketMsgArea*)((UINT8*)pComMsg->GetBufferPtr()
			+M_DEFAULT_RESERVED-sizeof(SocketMsgArea));
#endif
	SocketMsgArea* pMsgArea = (SocketMsgArea*)((UINT8*)pComMsg->GetDataPtr()
				-sizeof(SocketMsgArea));
	switch(msgID)
	{
		case MSGID_GRP_HO_RES_REQ:
			pMsgArea->MsgArea = MSGAREA_GRPSRV_HORESREQ;
			break;
		case MSGID_GRP_HO_RES_RSP:
			pMsgArea->MsgArea = MSGAREA_GRPSRV_HORESRSP;
			break;
		default:
			LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_SYS_ERR), 
				"ProcessGrpSrvHOMsg2OtherBTS ,invalid msgID ",msgID);
			return;
	}
		
	BtsAddr addr;
	UINT32 dstBtsID = pComMsg->GetBTS();
	if(!GetBtsPubAddr(dstBtsID, &addr))
	{
		LOG1( LOG_DEBUG1, LOGNO(EMSA,EC_SOCKET_SYS_ERR), 
			"ProcessGrpSrvHOMsg2OtherBTS ,can not get btsid(%d) 's ip&port ",dstBtsID);
		return;
	}
	if(!SendBySocket(addr.IP,addr.Port,pMsgArea,
		sizeof(SocketMsgArea)+pComMsg->GetDataLength()))
	{
		LOG(LOG_DEBUG1, LOGNO(EMSA, EC_SOCKET_SOCKET_ERR), 
			"SendBySocket(), error!!!");
		return;
	}   
	LOG( LOG_DEBUG3, LOGNO(EMSA,EC_SOCKET_NORMAL), 
		"ProcessGrpSrvHOMsg2OtherBTS successed!!! ");
}
#if 0
extern "C" STATUS testJamming(UINT8 flag)
{

    CSOCKET *taskSocket = CSOCKET::GetInstance();
    taskSocket->testJammingFromL2(flag);
    return OK;
}

void CSOCKET::testJammingFromL2(UINT8 flag)
{
    LOG( LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_NORMAL), " this is a test function, it adds current bts's ip to JNT");
    //test start
    //printf("\r\n this is a test function, it adds current bts's ip to JNT\r\n ") ,
    //test stop
    JammingTable[0]= m_curBtsId;
    CComMessage* pComMsg = GetFreeComMsg();
    if (NULL == pComMsg)
    {
        IncreasePerf( PERF_TSOCKET_NO_BUFFER );
        return;
    }
    UINT8* pRecvBuf = (UINT8*)pComMsg->GetBufferPtr();

    pComMsg->SetBuffer((void*)pRecvBuf, M_TSOCKET_BUFFER_LENGTH);
    pComMsg->SetDataPtr(pRecvBuf + DEFAULT_BUFFER_RSV);
    pComMsg->SetDataLength(100);    
    pComMsg->SetSrcTid(M_TID_L2MAIN);
    pComMsg->SetDstTid(this->GetEntityId());
    if(1 == flag)
    {
        LOG( LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_NORMAL), "testJammingFromL2 post BTS_Jamming_Rpt ");
        //printf("\r\ntestJammingFromL2 post BTS_Jamming_Rpt \r\n");
        pComMsg->SetMessageId(BTS_Jamming_Rpt);
        stBtsJammingRptL3* pRpt = (stBtsJammingRptL3*)((UINT8*)pComMsg->GetDataPtr());
        pRpt->srcBtsId= 157;
        pRpt->cpeId = 0x1234;
        pRpt->cpeJammingRptInfo.OBSInfo.OBS0 = 2;
        pRpt->cpeJammingRptInfo.OBSInfo.OBS1 = 4;
        pRpt->cpeJammingRptInfo.OBSInfo.OBS2 = 6;

    }
    else if(2 == flag)
    {
        LOG( LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_NORMAL), "testJammingFromL2 post BTS_Jamming_Rpt_Rsp ");
        //printf("\r\ntestJammingFromL2 post BTS_Jamming_Rpt_Rsp \r\n");
        pComMsg->SetMessageId(BTS_Jamming_Rpt_Rsp);
        stBtsJammingRptRspL3* pRsp = (stBtsJammingRptRspL3*)((UINT8*)pComMsg->GetDataPtr());
        pRsp->dstBtsId = 155;
        pRsp->srcBtsId = 155;
        pRsp->cpeId = 0x1234;
        pRsp->seqId = 2;    
        pRsp->pairedCpeId = 0x2222;
    }
    else if(3 == flag)
    {
        LOG( LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_NORMAL), "testJammingFromL2 post BTS_PairedCpe_Prof_Msg ");
        //printf("\r\ntestJammingFromL2 post BTS_PairedCpe_Prof_Msg \r\n");
        pComMsg->SetMessageId(BTS_PairedCpe_Prof_Msg);
        stBtsPairedCpeProfMsgL3* pProf = (stBtsPairedCpeProfMsgL3*)((UINT8*)pComMsg->GetDataPtr());
        pProf->dstBtsId = 155;
        pProf->srcBtsId = 155;
        pProf->cpeId = 0x1234;
        pProf->cpeNum = 1;
        pProf->seqId = 2;
    }


    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        LOG( LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_SYS_ERR), "testJammingFromL2 failed");
//test start
//printf("\r\ntestJammingFromL2 failed\r\n");
//test stop
        return;
    }
    LOG( LOG_CRITICAL, LOGNO(EMSA,EC_SOCKET_NORMAL), "testJammingFromL2 successful! "); 
//test start
//printf("\r\ntestJammingFromL2 successful! \r\n");
//test stop
    return;
}
#endif 
#ifndef WBBU_CODE
#include "ui.h"
extern "C" void    initARP();
 extern "C" void   InitFreeARPList();
 extern "C" void   InsertFreeARPList(UINT16 index);
 extern "C" UINT16 GetFreeARPEntryIdxFromList();

   	//ARP的操作
extern "C" 	BOOL   ARPBPtreeAdd(UINT32 ip, UINT16 index);
extern "C" 	BOOL  ARPBPtreeDel(UINT32 ip);
extern "C" 	UINT16 ARPBPtreeFind(UINT32 ip);

extern "C" 	BOOL   ARPAddEntry(UINT32 ip, newarpTableT* p);
extern "C" int getSTDNum(int *);
extern "C" 	BOOL   ARPDelEntry(UINT32 ip);
 extern "C"      BOOL  ARPUpdateEntry(UINT32 ip, newarpTableT* p);
extern "C" 	newarpTableT* GetARPEntryByIdx(UINT16 index);
extern "C" 	void   ARPBPtreeExpire();
   // Free ARP Records List methods
  list<UINT16> m_listFreeARP;  //空闲转发表表项的链表
 newarpTableT  m_ARP[ ARP_TABLE_LEN ];  // 转发表
 map<UINT32, UINT16> m_ARPBptree;  // 转发表索引树
void showMap(int arg)
{
    const CSOCKET * const pTaskSocket = CSOCKET::GetInstance();
    if(0 == arg)
        {
        printf("\r\nshow EMS agent Rx MAP info.");
        printf("\r\nPlease input MOC:");
        UINT32 usMOC = 0;
        getSTDNum((int*)&usMOC);

        UINT32 idx = pTaskSocket->SearchMap((TID)0, (UINT16)usMOC, 0);
        if(-1 != idx)
            {
            printf("mac:%d, moc:0x%X, action:%d, msgid:0x%X, tid:%d", g_EmsMessageMap[idx].ma, g_EmsMessageMap[idx].moc, g_EmsMessageMap[idx].action, g_EmsMessageMap[idx].msgid, g_EmsMessageMap[idx].tid);
            }
        else
            {
            printf("\r\nMOC[%d] not found, try searching Tx MAP. [showMAP 1]", usMOC);
            }
        }
    else
        {
        printf("\r\nshow EMS agent Tx MAP info.");
        printf("\r\nPlease input MSGID:");
        UINT32 usMsgID = 0;
        getSTDNum((int*)&usMsgID);

        UINT32 idx = pTaskSocket->SearchMap((TID)0, (UINT16)usMsgID);
        if(-1 != idx)
            {
            //返回Index.
            printf("mac:%d, moc:0x%X, action:%d, msgid:0x%X, tid:%d", g_EmsMessageMap[idx].ma, g_EmsMessageMap[idx].moc, g_EmsMessageMap[idx].action, g_EmsMessageMap[idx].msgid, g_EmsMessageMap[idx].tid);
            }
        else
            {
            printf("\r\nMOC[%d] not found, try searching Rx MAP. [showMAP 0]", usMsgID);
            }
        }
    printf("\r\n---\r\n");
    return;
}
 void initARP()
 {
     memset( m_ARP, 0, sizeof( m_ARP ) );
 }
  void   InitFreeARPList()
{
      UINT16 usIdx;
        m_listFreeARP.clear();
      for ( usIdx = 0; usIdx < ARP_TABLE_LEN; ++usIdx )
     {
        m_listFreeARP.push_back( usIdx );
      }
 }
 void   InsertFreeARPList(UINT16 index)
{
    if( index < ARP_TABLE_LEN )
        {
        m_listFreeARP.push_back( index );
        }
   
}
 UINT16 GetFreeARPEntryIdxFromList()
 {
         UINT16 usIdx ;
        if ( true == m_listFreeARP.empty() )
        {
        return 0xffff;
        }

    usIdx = *m_listFreeARP.begin();
    m_listFreeARP.pop_front();

    if ( ARP_TABLE_LEN <= usIdx )
      {
        //下标错误
          return 0xffff;
      }

    return usIdx;
 }


   	//ARP的操作
BOOL   ARPBPtreeAdd(UINT32 ip, UINT16 index)
{
         pair<map<UINT32, UINT16>::iterator, bool> stPair;
         stPair = m_ARPBptree.insert( ValType( ip, index ) );
        return stPair.second;
}
	BOOL  ARPBPtreeDel(UINT32 ip)
{

      map<UINT32, UINT16>::iterator it;
    if ((it = m_ARPBptree.find( ip ) ) != m_ARPBptree.end())
    {
        m_ARPBptree.erase( it );
    }
    return true;
}
	UINT16 ARPBPtreeFind(UINT32 ip)
{
        map<UINT32, UINT16>::iterator it = m_ARPBptree.find( ip );
    if ( it != m_ARPBptree.end() )
    {
        return it->second;
    }
    return 0xffff;
}

BOOL   ARPAddEntry(UINT32 ip, newarpTableT* p)
{
    UINT16 index = ARPBPtreeFind( ip );
    newarpTableT *pFT = GetARPEntryByIdx( index );
    if (NULL == pFT)
    {
        //新建转发表表项
        index  = GetFreeARPEntryIdxFromList();
	
        pFT = GetARPEntryByIdx( index );
        if (NULL == pFT)
        {
           
            return false;
        }
        ARPBPtreeAdd(ip, index);
    }
     memcpy(pFT, (void*)p, sizeof(newarpTableT)) ;
  //   printf("ARPAddEntry IP:%x\n",ip);
     return true;
   
}
	BOOL  ARPDelEntry(UINT32 ip)
{
      UINT16 index = ARPBPtreeFind(ip);
    newarpTableT *pFTEntry = GetARPEntryByIdx(index);
    if ( NULL != pFTEntry )
    {
       if(pFTEntry->hold)
       {
             // printf("ARPDelEntry IP:%x,%x,%x,%x\n",ip,pFTEntry->dwIP,pFTEntry->hold,pFTEntry->wTimer);
		netMblkClChainFree(pFTEntry->hold);
       }
        ARPBPtreeDel(ip);
	//printf("ARPDelEntry IP:%x,%x,%x,%x\n",ip,pFTEntry->dwIP,pFTEntry->hold,pFTEntry->wTimer);
        memset(pFTEntry, 0, sizeof(newarpTableT));
        m_listFreeARP.push_back( index );   // push the entry back to the free list
         
    }
	return true;
}
    BOOL  ARPUpdateEntry(UINT32 ip, newarpTableT* p)
{
     UINT16 index = ARPBPtreeFind( ip );
    newarpTableT *pFT = GetARPEntryByIdx( index );
    if (NULL == pFT)
    {
        //新建转发表表项
        index  = GetFreeARPEntryIdxFromList();
        pFT = GetARPEntryByIdx( index );
        if (NULL == pFT)
        {
           
            return false;
        }
        ARPBPtreeAdd(ip, index);
    }
     memcpy(pFT, (void*)p, sizeof(newarpTableT)) ;
   //  printf("ARPUpdateEntry IP:%x\n",ip);
     return true;
}
newarpTableT* GetARPEntryByIdx(UINT16 index)
{
        if (index >= ARP_TABLE_LEN)
    {
        return NULL;
    }
    return &( m_ARP[ index ] );
}
void   ARPBPtreeExpire()
{
     map<UINT32/*ip*/, UINT16>::iterator it = m_ARPBptree.begin();
     UINT32 IPIdList[ARP_TABLE_LEN];
	
    UINT32 timeOutBtsNum=0;
     memset(IPIdList,0,sizeof(IPIdList));
    UINT32 ip = 0;
    while (it!=m_ARPBptree.end())
    {
        if(it->second>=ARP_TABLE_LEN)
        {
          //  printf("it->second:%d\n",it->second);
             continue;
        }
       if((m_ARP[it->second].wTimer)>0)
       {
          m_ARP[it->second].wTimer--;
        if ((m_ARP[it->second].wTimer)==0)
        {
            ip = it->first;
	    
            IPIdList[timeOutBtsNum++] = ip;
	      if(timeOutBtsNum>=ARP_TABLE_LEN)
	      	{
	      //	  printf("timeOutBtsNum:%d\n",timeOutBtsNum);
	      	   break;
	      	}
	 
        }
	//	printf("hehe:%x\n",ip);
       }

        it++;
    }
    for (int i=0; i<timeOutBtsNum; i++ )
    {
        ARPDelEntry(IPIdList[i]);
	
    }


}

/************************************************
Show:
	(1)arpTable
	(2)idleTable count
return:
	checksum
************************************************/
void netShow2(void)
{
         map<UINT32, UINT16>::iterator it = m_ARPBptree.begin();

	newarpTableT *arp;
	int i;
	printf("\ntuiTask ARP table");
	printf("\n\n--IP-----------MAC------------Timer-----HOLD------");
	 for (;it != m_ARPBptree.end();it++)
	{
	        arp = &(m_ARP[(it->second)]);
		printf("\n\r%d.%d.%d.%d  %02x-%02x-%02x-%02x-%02x-%02x  %d    %08x", 
			(int)((arp->dwIP)>>24)&0xff, (int)((arp->dwIP)>>16)&0xff,(int)((arp->dwIP)>>8)&0xff,(int)(arp->dwIP)&0xff,
			arp->bMac[0], arp->bMac[1], arp->bMac[2], arp->bMac[3], arp->bMac[4], arp->bMac[5], 
			arp->wTimer, (unsigned int)arp->hold);
		
	}

}
#endif
//lijinan 090105 for jamming rpt flow ctl
extern "C" STATUS setJmRptFlowCtlPara(UINT32 fromL2,UINT32 fromBts)
{
	T_NVRAM_BTS_CONFIG_PARA params1;
	if((fromL2>10)&&(fromBts>10))
	{
		JammingMaxNumFromL2 = fromL2;
		JammingMaxNumFromBts = fromBts;
		bspNvRamRead((char *)&params1, (char*)(NVRAM_BASE_ADDR_PARA_PARAMS), sizeof(params1));

		 params1.jammingPatter = NVRAM_VALID_PATTERN;
		 params1.jammingRptFromL2Num = fromL2;
		 params1.jammingRptFromBtsNum = fromBts;
		 bspNvRamWrite((char*)NVRAM_BASE_ADDR_PARA_PARAMS, (char *)&params1,sizeof(params1));
		return OK; 
	}
	
	else
		printf("\n para must >10\n");
}
