#include "tVoiceSignal.h"
#include "log.h"
#include "stdio.h"
#include "SAGSignal.h"

#ifdef __WIN32_SIM__
//#include "winsock2.h"
#endif

#define M_TASK_NAME_LEN				(20)
#define M_TASK_TVCR_TASKNAME      "tVCR"
#define M_TASK_TVCR_PRIORITY      (95)
#ifdef __WIN32_SIM__
#define M_TASK_TVCR_OPTION        (0x0008)
#define M_TASK_TVCR_MSGOPTION     (0x02)
#elif __NUCLEUS__
#define M_TASK_TVCR_OPTION       (NULL)	//not use
#define M_TASK_TVCR_MSGOPTION     (NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_TVCR_OPTION        (VX_FP_TASK)
#define M_TASK_TVCR_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TVCR_STACKSIZE     (20480)
#define M_TASK_TVCR_MAXMSG        (1024)

CTask_VoiceSignal* CTask_VoiceSignal::s_pTaskVoiceSignal = NULL;

CTask_VoiceSignal::CTask_VoiceSignal()
{
	memcpy(m_szName, "tSAG", strlen( "tSAG" ) );
	m_szName[strlen( "tSAG" )] = 0;
	m_uPriority = 95;
	m_uOptions = M_TASK_TVCR_OPTION;
	m_uStackSize = M_TASK_TVCR_STACKSIZE;
	
	//m_lMaxMsgs = M_TASK_TVCR_MAXMSG;
	//m_lMsgQOption = M_TASK_TVCR_MSGOPTIO
}


CTask_VoiceSignal* CTask_VoiceSignal::GetInstance()
{
	if(s_pTaskVoiceSignal==NULL)
		s_pTaskVoiceSignal = new CTask_VoiceSignal;
	return s_pTaskVoiceSignal;
}
//////////////////////////////////////////////////////////////////////////
	
bool CTask_VoiceSignal::StartTcpServer()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CVCR::CreateSocket");
#endif
    sockaddr_in server;
	
    m_fdListen = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_fdListen<0)
    {
        LOG(LOG_CRITICAL, 0, "Create socket failed.");
        return false;
    }
	
	struct linger Linger;
	int yes = 1;	
	UINT32 on = 1;
	setsockopt(m_fdListen, SOL_SOCKET, SO_LINGER, (char *)&Linger, sizeof(Linger));
	//tcp no delay option
	setsockopt(m_fdListen, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof(yes));
	//keep alive option
	setsockopt(m_fdListen, SOL_SOCKET, SO_KEEPALIVE, (char *)&yes, sizeof(yes));
	
	//#ifdef __WIN32_SIM__
	//	if(ioctlsocket(m_fdListen, FIONBIO, (u_long*)&on) < 0) 
	//#else
	//	if(ioctl(m_fdListen, FIONBIO, (u_long*)&on) < 0) 
	//#endif
	//	{ 
	//		LOG(LOG_CRITICAL, 0, "make socket unblock fail.");
	//		OutputSocketErrCode("ioctl(m_fdListen, FIONBIO, (u_long*)&on)");
	//	} 
	
	//addr reuse
	setsockopt (m_fdListen, SOL_SOCKET, SO_REUSEADDR, (char *)0, 0); 	
	
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(M_SAG_SIGNAL_PORT);
    if (::bind(m_fdListen, (sockaddr*)&server, sizeof(server))<0)
    {
        LOG(LOG_CRITICAL, 0, "bind socket failed.");
		OutputSocketErrCode("bind(m_fdListen, (sockaddr*)&server, sizeof(server))");
        StopTcpServer();
        return false;
    }
	if(listen(m_fdListen, 5)<0)
	{
        LOG(LOG_CRITICAL, 0, "listen failed.");
		OutputSocketErrCode("listen(m_fdListen, 5)");
        StopTcpServer();
        return false;
	}

	FD_ZERO(&m_Clients);

    return true;
}
//////////////////////////////////////////////////////////////////////////

bool CTask_VoiceSignal::StopTcpServer()
{
	closesocket(m_fdListen);
	m_fdListen = -1;
	return true;
}

int CTask_VoiceSignal::BytesAvailable(int fdSocket)
{
	int ret;
#ifdef __WIN32_SIM__
	u_long bytesAvailable;
	ret = ioctlsocket(fdSocket, FIONREAD, &bytesAvailable);
#else	//vxworks
	int bytesAvailable;
	ret = ioctl(fdSocket, FIONREAD, (int)&bytesAvailable);
#endif
	if(ret<0)	//error
	{
		LOG(LOG_DEBUG3, 0, "ioctl(fdSocket, FIONREAD, &bytesAvailable) error!!!");
		OutputSocketErrCode("ioctl(fdSocket, FIONREAD, &bytesAvailable)");
		return -1;
	}
	return bytesAvailable;
}


//////////////////////////////////////////////////////////////////////////

void CTask_VoiceSignal::OutputSocketErrCode(char*p)
{
	if(NULL==p)
		printf("\nSocket operation error!!! Error Code[%d]", WSAGetLastError());
	else
		printf("\n%s error!!! Error Code[%d]", p, WSAGetLastError());
}
//////////////////////////////////////////////////////////////////////////

bool CTask_VoiceSignal::Initialize()
{
	LOG(LOG_DEBUG, 0, "CTask_VoiceSignal::Initialize");
	/*
	if(!initMediaServer())
		return false;
	*/
	if(!StopTcpServer())
		return false;
	return true;
}
//////////////////////////////////////////////////////////////////////////

bool CTask_VoiceSignal::SendASignal(UINT fdSocket, CSAbisSignal& signal)
{
	if(!FD_ISSET(fdSocket, &m_Clients))
		return false;
	else
	{
		int sent, totalToSend, SigDataLen;
		
		SigDataLen = signal.GetDataLength();
		char *bufToSend = (char*)signal.GetDataPtr();
		totalToSend = SigDataLen;
		do 
		{
			sent = send(fdSocket, bufToSend, totalToSend, 0);
			if(sent<0)
			{
				LOG(LOG_DEBUG3, 0, "CVCR::sendOneSignalToSAG() send error!!!");
				OutputSocketErrCode("send()");
				closesocket(fdSocket);
				FD_CLR(fdSocket, &m_Clients);
				return false;
			}
			bufToSend+=sent;
			totalToSend-=sent;
		} while(0!=totalToSend);
		return true;		
	}
}
//////////////////////////////////////////////////////////////////////////

bool CTask_VoiceSignal::initMediaServer()
{
	LOG(LOG_DEBUG3, 0, "CVCR::initMediaServer()");
	sockaddr_in localAddr;
	m_fdVoiceData = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(m_fdVoiceData<0)
	{
		LOG(LOG_CRITICAL, 0, "socket() failed.");
		return false;
	}
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	localAddr.sin_addr.s_addr = inet_addr("192.168.2.180");
	localAddr.sin_port = htons(M_SAG_MEDIA_PORT);
	if(bind(m_fdVoiceData, (sockaddr*)&localAddr, sizeof(localAddr))<0)
	{
		LOG(LOG_CRITICAL, 0, "bind() failed.");
		return false;
	}

/*
#ifdef __WIN32_SIM__
	int on = 1;
	if(ioctlsocket(m_fdVoiceData, FIONBIO, (u_long*)&on) < 0) 
#else
	if(ioctl(m_fdVoiceData, FIONBIO, (u_long*)&on) < 0) 
#endif
	{ 
		LOG(LOG_CRITICAL, 0, "make socket unblock fail.");
		OutputSocketErrCode("ioctl(m_fdVoiceData, FIONBIO, (u_long*)&on)");
	} 
*/
	return true;
}
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
#include "sagsim.h"
extern CSAG sag;
void CTask_VoiceSignal::MainLoop()
{

	StartTcpServer();
	static int fdAccept = m_fdListen;
	int ret;
	UINT32 i;
	struct sockaddr_in from;
	char bufVoiceData[2000];

	while(1)
	{
	
	FD_SET readFDS = m_Clients;
	int fromLen = sizeof(from);
	
	FD_SET(m_fdListen, &readFDS);
	//FD_SET(m_fdVoiceData, &readFDS);	//UDP voice data socket

	ret = select(0x999999, &readFDS, NULL, NULL, NULL);
	if(ret<0)
	{
		LOG(LOG_CRITICAL, 0, "select failed.");
		OutputSocketErrCode("select(fdAccept+1, &readFDS, NULL, NULL, NULL)");
		StopTcpServer();
		StartTcpServer();
		return;
	}
	//处理语音数据通信的UDP端口
	//+++
	if(FD_ISSET(m_fdVoiceData, &readFDS))
	{
		ret = ::recvfrom(m_fdVoiceData, bufVoiceData, sizeof(bufVoiceData), 0, 0, 0);
		if(ret>0)
		{
			sag.handleVoiceData(bufVoiceData, ret, m_fdVoiceData);
		}
	}		
	
	//处理通信的端口
	for(i=0;i<m_Clients.fd_count;i++)
	{
		if(FD_ISSET(m_Clients.fd_array[i], &readFDS))
		{
			//如果远端关闭了端口或者有错误发生
			if(BytesAvailable(m_Clients.fd_array[i])<=0)
			{
				closesocket(m_Clients.fd_array[i]);			//关闭socket
				FD_CLR(m_Clients.fd_array[i], &m_Clients);	//清除连接socket列表
			}
			//如果有数据接收
			else
			{
				CSAbisSignal signal;
				//ret = recv(m_Clients.fd_array[i], (char*)signal.GetDataPtr(), M_MAX_SIGNAL_LENGTH, 0);
				ret = recvOneSignal(m_Clients.fd_array[i], (char*)signal.GetDataPtr());
				if(ret<0)
				{
					LOG(LOG_CRITICAL, 0, "recv failed.");
					OutputSocketErrCode("socket recv ");
					closesocket(m_Clients.fd_array[i]);
					FD_CLR(m_Clients.fd_array[i], &m_Clients);
					continue;	//处理下一个socket的接收
				}
				if(ret>0)
				{
					signal.SetDataLength(ret);
					//信令的处理
					//合法性检查，如果连接非法，则断开连接
					//++++++
					sag.ParseAndHandleSignal(signal, m_Clients.fd_array[i]);
					
				}
			}
		}
	}
	//处理监听的端口
	if(FD_ISSET(m_fdListen, &readFDS))
	{
		//接受远端的连接
		fdAccept = accept(m_fdListen, (struct sockaddr*)&from, &fromLen);
		if(fdAccept>0)
		{
			LOG(LOG_DEBUG3, 0, "\nAccept tcp connection from IP[%s] Port[%d]", 
				inet_ntoa(from.sin_addr), htons(from.sin_port));
			FD_SET(fdAccept, &m_Clients);	//记录新的socket
		}
		else
		{
			LOG(LOG_CRITICAL, 0, "accept failed.");
			OutputSocketErrCode("socket accept ");
			StopTcpServer();
			StartTcpServer();
			return;
		}
	}

	}
}

int CTask_VoiceSignal::recvOneSignal(int m_fdTcpSag, char *pBuf)
{
	int ret;
	UINT32 i;
#ifdef __WIN32_SIM__
	u_long bytesAvailable;
	ret = ioctlsocket(m_fdTcpSag, FIONREAD, &bytesAvailable);
#else	//vxworks
	int bytesAvailable;
	ret = ioctl(m_fdTcpSag, FIONREAD, (int)&bytesAvailable);
#endif
	if(ret<0)	//error
	{
		OutputSocketErrCode("ioctl(m_fdTcpSag, FIONREAD, &bytesAvailable)");
		return -1;
	}
	if(0==bytesAvailable)
	{
		printf("\nBTS closed tcp link.");
		return -1;
	}

	char *bufHead = pBuf;	//TCP Head Flag, 2bytes
	bufHead[0] = 0;
	bool blHeadFound = false;
	//找到头部
	for(i=0;i<bytesAvailable;i++)
	{
		if(1!=recv(m_fdTcpSag, &bufHead[1], 1, 0))
		{
			OutputSocketErrCode("recv packet header");
			return -1;
		}
		if( M_TCP_PKT_BEGIN_FLAG == ntohs( *( (UINT16*)bufHead ) ) )
		{
			blHeadFound = true;
			break;
		}
		else
		{
			bufHead[0] = bufHead[1];
			continue;
		}
	}
	if(!blHeadFound)
	{
		printf("\nCannot Find packet Header");
		return 0;
	}
	//读出包长度
	UINT16 *pLen = (UINT16*)(pBuf+2);
	UINT16 len;
	ret=recv(m_fdTcpSag, (char*)pLen, sizeof(UINT16), 0);
	if( ret < 0)
	{
		OutputSocketErrCode("recv packet length");
		return -1;
	}
	if(ret < sizeof(UINT16))
	{
		printf("\nrecv packet length error!!!");
		return 0;
	}
	len = ntohs(*pLen);

	UINT8* pData = (UINT8*)(pBuf+4);	//read after tcp pkt len
	//根据长度读出整个信令
	UINT16 readLen = len - sizeof(UINT16) + sizeof(EndFlagT);
	ret = recv(m_fdTcpSag, (char*)pData, readLen, 0);
	if( ret < 0 )
	{
		OutputSocketErrCode("recv packet length");
		return -1;
	}
	if(ret < readLen)
	{
		printf("\nrecv packet payload error!!!");
		return 0;
	}
	//找到尾部确认
	UINT16 *pEndFlag = (UINT16*)( pBuf + sizeof(HeadFlagT) + len );
	if( M_TCP_PKT_END_FLAG != ntohs(*pEndFlag) )
	{
		printf("\npacket END FLAG error!!!");
		return 0;
	}	

	return(sizeof(HeadFlagT)+len+sizeof(EndFlagT));
}

bool CTask_VoiceSignal::ProcessMedia()
{
	char bufVoiceData[2000];
	int ret;
	int errNO;
#if 0
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_fdVoiceData, &fds);	//UDP voice data socket
	ret = select(0x999999, &fds, NULL, NULL, NULL);
#endif
	ret = ::recvfrom(m_fdVoiceData, bufVoiceData, sizeof(bufVoiceData), 0, 0, 0);
	
		if(ret>0)
		{
			sag.handleVoiceData(bufVoiceData, ret, m_fdVoiceData);
		}
		else
		{
			errNO = WSAGetLastError();
		}
	return ret;
}