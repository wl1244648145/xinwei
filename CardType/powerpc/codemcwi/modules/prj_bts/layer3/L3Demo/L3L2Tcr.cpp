/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    M_TID_L2MAIN.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   09/26/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/
     //socket:
#ifdef __WIN32_SIM__
#include <Winsock2.h>
#include <ws2tcpip.h>
#else	//VxWorks:
#include "vxWorks.h" 
#include "sockLib.h" 
#include "inetLib.h" 
#include "stdioLib.h" 
#include "strLib.h" 
#include "hostLib.h" 
#include "ioLib.h" 
#endif

#include "Object.h"
#include "LogArea.h"
#include "log.h"
#include "l3datacommon.h"
#include "ComMessage.h"

#include "L3L2Tcr.h"
#ifdef __WIN32_SIM__
extern UINT32 GetBTSIpAddr();
#endif
//任务实例指针的初始化
CTaskL3L2Tcr* CTaskL3L2Tcr::Instance = NULL;


/*============================================================
MEMBER FUNCTION:
    CTaskDAC::CTaskDAC

DESCRIPTION:
    CTaskDAC构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/

CTaskL3L2Tcr::CTaskL3L2Tcr()
{

    memset( m_szName, 0 ,M_TASK_L3L2TCR_LEN);
    memcpy( m_szName, M_TASK_L3L2TCR_TASKNAME, strlen( M_TASK_L3L2TCR_TASKNAME ) );
    m_uPriority     = 100;
    m_uOptions      = M_TASK_L3L2TCR_OPTION;
    m_uStackSize    = M_TASK_L3L2TCR_STACKSIZE;
	m_RecvPacketCount=0;
	
#ifdef __WIN32_SIM__
    ::WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
#endif

}

/*============================================================
MEMBER FUNCTION:
    CTaskL3L2Tcr::~CTaskL3L2Tcr

DESCRIPTION:
    CTaskL3L2Tcr析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTaskL3L2Tcr::~CTaskL3L2Tcr()
{
    
}

/*============================================================
MEMBER FUNCTION:
    CTaskL3L2Tcr::GetInstance

DESCRIPTION:
    Get CTaskL3L2Tcr Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTaskDm* 

SIDE EFFECTS:
    none
==============================================================*/
CTaskL3L2Tcr* CTaskL3L2Tcr::GetInstance()
{
    if ( NULL == Instance )
        {
        Instance = new CTaskL3L2Tcr;
        }
    return Instance;
}

/*============================================================
MEMBER FUNCTION:
    Initialize

DESCRIPTION:

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/

bool CTaskL3L2Tcr::Initialize()
{

     //create socket
     m_UDPSocket = ::socket(AF_INET,SOCK_DGRAM, IPPROTO_IP);     
    if ( M_DATA_INVALID_SOCKET == m_UDPSocket )
        {
        
        #ifdef __WIN32_SIM__
        ::WSACleanup();
        #else
        ::close( m_UDPSocket );
        #endif
        return false;
        }

    sockaddr_in myaddr;
#ifdef __WIN32_SIM__
	myaddr.sin_addr.s_addr = GetBTSIpAddr() ;//GetHostAddr();htonl(INADDR_ANY);
#else
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);//GetHostAddr();htonl(INADDR_ANY);
#endif
    myaddr.sin_family = AF_INET;
    myaddr.sin_port =htons( DEFAULT_PORT);
 
    //绑定到本地地址上
    if ( ::bind(m_UDPSocket, (sockaddr*)&myaddr,sizeof(myaddr)))
    	{
#ifdef __WIN32_SIM__
        ::closesocket( m_UDPSocket );
        ::WSACleanup();
#else
        ::close( m_UDPSocket );
#endif
        return false;
    	}

	
    return true;
}


/*============================================================
MEMBER FUNCTION:
    MainLoop

DESCRIPTION:
    实时监测UDP-port,if available,read Buf and send Msg to Tunnul Task

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CTaskL3L2Tcr::MainLoop()
{
    struct sockaddr_in from;    /* connector's address information */ 
    SINT32 lFromLen = sizeof( struct sockaddr );
	SINT8  pBuf[DEFAULT_LENTH];
    //SINT8  pBuf[DEFAULT_LENTH]={0};
    for(;;)
    	{

		
		UINT32 len = ::recvfrom( m_UDPSocket, 
            (char*)pBuf, DEFAULT_LENTH,   //长度-42
            0, (struct sockaddr*)&from, &lFromLen );

        if ( M_DATA_SOCKET_ERR == len )
            {
//            delete[] pBuf;
            continue;
            }
        if(DEFAULT_DATA_LENTH> len)
        	{
//        	delete[] pBuf;
			continue;
        	}
    	CComMessage *pComMsg = new( this, DEFAULT_LENTH ) CComMessage;
		if(NULL == pComMsg)
			{
			continue;
			}
		T_InterComm *pinterCommMsg  = (T_InterComm*)pBuf;
    	pComMsg->SetDstTid( (TID)ntohl(pinterCommMsg->m_tidDst));
		//pComMsg->SetDstTid( M_TID_CPESM );
    	pComMsg->SetSrcTid( (TID)ntohl(pinterCommMsg->m_tidSrc)); 
    	pComMsg->SetEID( ntohl(pinterCommMsg->m_uEID)); 
    	pComMsg->SetMessageId( ntohs(pinterCommMsg->m_uMsgId) );
        
//    	pComMsg->SetBuffer( pBuf, len );

		//memcpy(pComMsg->GetDataPtr(),pBuf,len-DEFAULT_DATA_LENTH);
//    	pComMsg->SetDataPtr( pBuf + DEFAULT_DATA_LENTH );
    	pComMsg->SetDataLength(len-DEFAULT_DATA_LENTH );
		memcpy( pComMsg->GetDataPtr(), pBuf+DEFAULT_DATA_LENTH, len-DEFAULT_DATA_LENTH );

    	if( false == CComEntity::PostEntityMessage( pComMsg ) )
        	{
        	pComMsg->Destroy();
        	}
		m_RecvPacketCount++;
#ifdef __WIN32_SIM__
        m_SrcIpAddr = ntohl( from.sin_addr.S_un.S_addr );
#else
        m_SrcIpAddr = ntohl( from.sin_addr.s_addr );
        //长度?
#endif
		

    	}
}

UINT32 CTaskL3L2Tcr::GetSrcIpAddr()
{
	return m_SrcIpAddr;
}

void CTaskL3L2Tcr::ShowStatus()
{
    printf("\r\nReceive UDPPacket Count = %d\n",m_RecvPacketCount);
}
extern "C" void L3L2TcrShow()
{
    CTaskL3L2Tcr *taskl3l2tcr = CTaskL3L2Tcr::GetInstance();
    taskl3l2tcr->ShowStatus();
}
