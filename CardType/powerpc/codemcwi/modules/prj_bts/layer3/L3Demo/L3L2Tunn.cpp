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

#include "MsgQueue.h"
#include "Object.h"
#include "LogArea.h"
#include "log.h"
#include "l3datacommon.h"

#include "L3L2Tunnel.h"
#include "l3l2tcr.h"

#ifdef __WIN32_SIM__
extern int g_bts_card_id;
extern int CPEcardIp[3];
extern int AdapterNum;
UINT32 GetBTSIpAddr()
{
	for ( int i = 0; i < AdapterNum; i++ )
	{
		if ( g_bts_card_id == i ) 
		{
			return CPEcardIp[i];
		}
	}
}
#endif

//任务实例指针的初始化
CTaskL3L2Tunnel* CTaskL3L2Tunnel::s_ptaskL3L2Tunnel = NULL;
#ifdef M_TGT_L3
const TID CTaskL3L2Tunnel::ProxyTIDs[2] = { 
                           M_TID_L2MAIN,      // 50  
                           M_TID_DM      // 18
                        };
#elif M_TGT_L2
const TID CTaskL3L2Tunnel::ProxyTIDs[2] = { 
                           M_TID_L2MAIN ,     // 57 
                           M_TID_UTDM
                        };
		
#endif


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

CTaskL3L2Tunnel::CTaskL3L2Tunnel()
{
    //LOG( LOG_DEBUG3, LOGNO(TCR,ERR_TCR_NORMAL), "CTaskTCR::CTaskTCR()" );
    memset( m_szName, 0 ,M_TASK_L3L2TUNNEL_LEN);
    memcpy( m_szName, M_TASK_L3L2TUNNEL_TASKNAME, strlen( M_TASK_L3L2TUNNEL_TASKNAME ) );
    m_uPriority     = 100;
    m_uOptions      = M_TASK_L3L2TUNNEL_OPTION;
    m_uStackSize    = M_TASK_L3L2TUNNEL_STACKSIZE;

    m_iMsgQMax      = 1024;
    m_iMsgQOption   = M_TASK_L3L2TUNNEL_MSGOPTION;

	m_ulEID         = 0;
    CurrentTid = ProxyTIDs[0];

	for (int i=0; i<SIZEOF(ProxyTIDs); i++)
    {
        CurrentTid = ProxyTIDs[i];
        RegisterEntity(false);
    }

#ifdef __WIN32_SIM__
    ::WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
#endif

}

/*============================================================
MEMBER FUNCTION:
    CTaskL3L2Tunnel::~CTaskL3L2Tunnel

DESCRIPTION:
    CTaskL3L2Tunnel析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTaskL3L2Tunnel::~CTaskL3L2Tunnel()
{
    
}

/*============================================================
MEMBER FUNCTION:
    CTaskL3L2Tunnel::GetInstance

DESCRIPTION:
    Get CTaskL3L2Tunnel Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTaskDm* 

SIDE EFFECTS:
    none
==============================================================*/
CTaskL3L2Tunnel* CTaskL3L2Tunnel::GetInstance()
{
    if ( NULL == s_ptaskL3L2Tunnel )
        {
        s_ptaskL3L2Tunnel = new CTaskL3L2Tunnel;
        }
    return s_ptaskL3L2Tunnel;
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
bool CTaskL3L2Tunnel::Initialize()
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
	myaddr.sin_addr.s_addr = GetBTSIpAddr() ;//GetHostAddr();
#else
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);//GetHostAddr();htonl(INADDR_ANY);
#endif
    myaddr.sin_family = AF_INET;
  //  myaddr.sin_port =htons( DEFAULT_PORT);
    //绑定到本地地址上
    if (::bind(m_UDPSocket, (sockaddr*)&myaddr,sizeof(myaddr)))
    	{
#ifdef __WIN32_SIM__
        ::closesocket( m_UDPSocket );
        ::WSACleanup();
#else
        ::close( m_UDPSocket );
#endif
        return false;
    	}


	UINT8 ucInit = CBizTask::Initialize();
    if ( false == ucInit )
        {
        //close socket.
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

bool CTaskL3L2Tunnel::ProcessComMessage(CComMessage* pComMsg)
{
	TID srcTid = pComMsg->GetSrcTid();
    if ( M_TID_EB == srcTid )
        {
        pComMsg->SetDstTid( M_TID_UTDM );
        }
#ifdef __WIN32_SIM__
	if ( M_TID_L2MAIN == pComMsg->GetDstTid())
        	{
        	pComMsg->SetDstTid( M_TID_EB );
        	if ( 0 != pComMsg->GetDataLength() % 2 )
	            	{
	            	printf("!!!!!!!!!!!!!!!!!!!!!!!!!");
	            	}
        	}
   	swap16( (UINT8*)pComMsg->GetDataPtr(), pComMsg->GetDataLength() );

	T_InterComm socketdata;
	memset(&socketdata,0,sizeof(T_InterComm));
	socketdata.m_tidDst =(TID) htonl(pComMsg->GetDstTid());
	socketdata.m_tidSrc =(TID) htonl(pComMsg->GetSrcTid());
	socketdata.m_uEID = GetBTSIpAddr();
	socketdata.m_uMsgId = htons(pComMsg->GetMessageId());
	memcpy(socketdata.databuf,pComMsg->GetDataPtr(),pComMsg->GetDataLength());
	UINT32 ulDestBtsIpAddr = GetDestIpAddr();
    //destroy message.
	bool result=Sendsocketdata(ulDestBtsIpAddr,&socketdata,pComMsg->GetDataLength()+DEFAULT_DATA_LENTH);
#else
	T_InterComm socketdata;
	memset(&socketdata,0,sizeof(T_InterComm));
	socketdata.m_tidDst =(TID) htonl(pComMsg->GetDstTid());
	socketdata.m_tidSrc =(TID) htonl(pComMsg->GetSrcTid());
	socketdata.m_uEID = htonl( pComMsg->GetEID() );
	socketdata.m_uMsgId = htons(pComMsg->GetMessageId());
	memcpy(socketdata.databuf,pComMsg->GetDataPtr(),pComMsg->GetDataLength());
	UINT32 ulDestBtsIpAddr = pComMsg->GetEID() ;
    //destroy message.
	bool result=Sendsocketdata(ulDestBtsIpAddr,&socketdata,pComMsg->GetDataLength()+DEFAULT_DATA_LENTH);

#endif

    pComMsg->Destroy();
    return result;
}

/*============================================================
MEMBER FUNCTION:
    CTaskL3L2Tunnel::SendBySocket

DESCRIPTION:
    通过Socket发送数据

ARGUMENTS:
    ulDstBtsAddr:隧道对端BTS的地址
    ulLen:数据长度
    *pData:发送数据

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskL3L2Tunnel::Sendsocketdata(UINT32 ulDstIpAddr, void *pData, UINT32 ulLen)
{

    struct sockaddr_in to;
#ifdef __WIN32_SIM__
    memset( (char*)&to, 0, sizeof( struct sockaddr_in ) );
    to.sin_addr.S_un.S_addr = htonl( ulDstIpAddr );
#else
    bzero ( (char*)&to, sizeof( struct sockaddr_in ) ); 
    to.sin_addr.s_addr = htonl( ulDstIpAddr );
    //长度?
#endif
    to.sin_family = AF_INET;
    to.sin_port = htons( DEFAULT_PORT );

    if ( M_DATA_SOCKET_ERR == ::sendto( m_UDPSocket, (char*)pData, ulLen, 0, \
        (struct sockaddr*)&to, sizeof( struct sockaddr ) ) )
        {
        return false;
        }

    return true;
}




UINT32 CTaskL3L2Tunnel::GetDestIpAddr()
{
#ifdef M_TGT_L3
	printf("\r\n*****STUB::DestIpAddr....return 192.168.2.242*****\r\n");
    //返回主机序
    return 0xc0a802f2;//0xc0a8021a;
#elif  M_TGT_L2
	printf("\r\n*****STUB::DestIpAddr....return Ip from Tcr*****\r\n");
    //返回主机序
    return CTaskL3L2Tcr::GetInstance()->GetSrcIpAddr();//0xc0a802db;
#endif
}

#ifdef M_TGT_L3

void CTaskL3L2Tunnel::swap16(UINT8 *pData, UINT32 ulLen)
{
    for( UINT32 ulIdx = 0; ulIdx < ulLen / 2; ++ulIdx )
        {
        UINT32 ulOffset = ulIdx * 2;
        UINT8 temp      = pData[ ulOffset ];
        pData[ ulOffset ]       = pData[ ulOffset + 1 ];
        pData[ ulOffset + 1 ]   = temp;
        }
}
#endif

