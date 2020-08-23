/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: L3OamDiag.cpp
 *
 * DESCRIPTION:
 *     Implementation of the receiving task of the Diag module.
 *
 * HISTORY:
 * Date        Author       Description
 * ----------  ----------   ----------------------------------------------------
 * 07/04/2006  Tian JingWei Initial file creation.
 * 01/08/2006  Xin Wang     Nat.
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
#include<taskLib.h>
#include <vxworks.h>
#include <sockLib.h>
#include <inetLib.h>
#include <string.h>
#include <hostLib.h>
#include <ioLib.h>
#include <bootLib.h>
#include "mcwill_bts.h"
#define INVALID_SOCKET ERROR
#define SOCKET_ERROR   ERROR
#define closesocket    close
#endif

////////////////////////////////////////////////////////////////////////
#include "stdio.h"
#include "BizTask.h"

#ifndef _INC_L3OAMDIAG
#include "L3OamDiag.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif
//////////////////////////////////////////////////////////////////////////////////////////////
#define M_EMS_MAX_MSGLEN  1500
#define DIAG_RCVEMS_PORT  8888
#define DIAG_SENDEMS_PORT   8887

#define DIAGTOOLREQ_TYPE   0x80
#define DIAGTOOL_CFG       0x81
#define DIAGTOOL_BCUTMONITOR_REQ 0x86
#define DIAGTOOL_BCUTMONITOR_RSP 0x66
#define DIAGTOOL_BCUTMONITOR_GLOBALINFO 0x60
T_DiagToolReg  gDiagToolReg; 

struct in_addr L3BackplaneIpAddr; 
struct in_addr L2BackplaneIpAddr; 
#ifdef WBBU_CODE
unsigned int Recv_num = 0;
unsigned int Send_num = 0;
unsigned int Err_num = 0;
unsigned int Direct_num = 0;
#endif


#ifdef WBBU_CODE
extern unsigned char L2_packet_Test;
extern "C" void send_2_diag_tools(unsigned char *pBuf,unsigned int Datalen);
#endif
extern UINT32 gVx_System_Clock;    /*系统持续运行的时间(秒)*/
T_DiagToolRecs gDiagToolRecs;

//任务实例指针的初始化
CL3OamDiagEMSL3L2* CL3OamDiagEMSL3L2::s_ptaskDiagL3 = NULL;
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
CL3OamDiagEMSL3L2::CL3OamDiagEMSL3L2()
:m_sfdL3L2(INVALID_SOCKET)
{
    ::strcpy(m_szName, "tDiagL3L2");
    m_uPriority   = M_TP_L3EMSAGENTRX;
    m_uStackSize  = 20000;
    m_iMsgQMax    = 100;
#ifndef __WIN32_SIM__
    m_uOptions    = VX_FP_TASK;
    m_iMsgQOption = (MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY);  
#endif
}

TID CL3OamDiagEMSL3L2::GetEntityId() const
{
    return M_TID_L3OAMDIAGEMSL3L2;
}

bool CL3OamDiagEMSL3L2::Initialize()
{
    BOOT_PARAMS       bootParams;
    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
    L3BackplaneIpAddr.s_addr = inet_addr(bootParams.bad);
    L2BackplaneIpAddr.s_addr = L3BackplaneIpAddr.s_addr + 1;  // L2 backplace address is based on L3 address
    LOG2(LOG_CRITICAL, LOGNO(DIAG,0), "[DIAG L3->L2][L3BackplaneIpAddr:0x%x,L2BackplaneIpAddr:%x]",    L3BackplaneIpAddr.s_addr, L2BackplaneIpAddr.s_addr);
    //Create receiving socket
    if (!CreateSocket())
    {
        LOG(LOG_SEVERE, LOGNO(DIAG,0), "Create socket failed.task crashed.");
        return false;
    }
    memset(&gDiagToolRecs, 0, sizeof(gDiagToolRecs));
    if ( false == CBizTask::Initialize() )
    {
        CloseSocket();
        LOG(LOG_SEVERE, LOGNO(DIAG,0), "CBizTask::Initialize failed.task crashed.");
        return false;
    }

    return true;
}

CL3OamDiagEMSL3L2* CL3OamDiagEMSL3L2::GetInstance()
{
    if ( NULL == s_ptaskDiagL3 )
    {
        s_ptaskDiagL3 = new CL3OamDiagEMSL3L2;
    }
    return s_ptaskDiagL3;
}

bool CL3OamDiagEMSL3L2::ProcessComMessage(CComMessage* pComMsg)
{
    UINT32 IP   = pComMsg->GetBtsAddr();
    UINT16 Port = pComMsg->GetBtsPort();
    UINT16 srcTid = pComMsg->GetSrcTid();
    LOG2(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2]Rx packet from DiagTool(bts)[IP:0x%x,Port:%d]",IP,Port);
    
    if(M_TID_EMSAGENTTX == srcTid)
    {
        CheckValid();
        if (pComMsg->GetDataLength() < M_EMS_MAX_MSGLEN+1) //within the limit
        {
            T_DiagToolUtMonitorReq * pDiagToolUtMonitorReg = (T_DiagToolUtMonitorReq *)pComMsg->GetDataPtr();

            if(DIAGTOOL_BCUTMONITOR_REQ == pDiagToolUtMonitorReg->header.type)
	        {
                OAM_LOGSTR(LOG_SEVERE, LOGNO(DIAG,0), "[DIAG L3->L2] ProcessComMessage::DIAGTOOL_BCUTMONITOR_REQ");
                for(int index = 0; index < MAX_DIAGTOOL_REGCNT; index++)
                {
                    if ((gDiagToolRecs.DiagToolRec[index].IP == IP) && (gDiagToolRecs.DiagToolRec[index].Port == Port))
                    { 
                        gDiagToolRecs.DiagToolRec[index].Flag = DIAG_TOOLREG_REC_USEED;
                        gDiagToolRecs.DiagToolRec[index].TTL  = gVx_System_Clock + pDiagToolUtMonitorReg->Period;
                        LOG3(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2]receive config from DiagTool(bts) (IP:0x%x,Port:%d) exists,update TTL:%d",IP,Port,gDiagToolRecs.DiagToolRec[index].TTL);
                        break;
                    }
                    
                    if (gDiagToolRecs.CurrentCnt >= MAX_DIAGTOOL_REGCNT)
                    {
                        //返回不能注册
                        LOG1(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2] gDiagToolRecs.CurrentCnt:%d >= MAX_DIAGTOOL_REGCNT can not register",gDiagToolRecs.CurrentCnt);
                        break;
                    }
                    
                    if (gDiagToolRecs.DiagToolRec[index].Flag == DIAG_TOOLREG_REC_NOTUSEED)
                    { 
                        gDiagToolRecs.DiagToolRec[index].Flag = DIAG_TOOLREG_REC_USEED;
                        gDiagToolRecs.DiagToolRec[index].TTL  = gVx_System_Clock + pDiagToolUtMonitorReg->Period;
                        gDiagToolRecs.DiagToolRec[index].IP   = IP;
                        gDiagToolRecs.DiagToolRec[index].Port = Port;
                        gDiagToolRecs.CurrentCnt++;
                        LOG2(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2]DiagTool(bts) [IP:0x%x,Port:%d] register successful!!!",IP,Port);
                        LOG1(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2]the number of DiagTool(bts) users: %d",gDiagToolRecs.CurrentCnt);
                        break;
                    }
                }
                pDiagToolUtMonitorReg->IP = L3BackplaneIpAddr.s_addr;
                pDiagToolUtMonitorReg->Port = DIAG_SENDEMS_PORT;
                LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2]tDiagL3 pDiagToolReg->Username = %s",(int)pDiagToolUtMonitorReg->Username);
                LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2]tDiagL3 pDiagToolReg->Password = %s",(int)pDiagToolUtMonitorReg->Password);
            }
            T_DiagToolReg * pDiagToolReg = (T_DiagToolReg *)pComMsg->GetDataPtr();
            if(DIAGTOOL_CFG == pDiagToolReg->header.type)
            {
                for(int index = 0; index < MAX_DIAGTOOL_REGCNT; index++)
                {   
                    if ((gDiagToolRecs.DiagToolRec[index].IP == IP) && (gDiagToolRecs.DiagToolRec[index].Port == Port))
                    { 
                        gDiagToolRecs.DiagToolRec[index].Flag = DIAG_TOOLREG_REC_USEED;
                        gDiagToolRecs.DiagToolRec[index].TTL = gVx_System_Clock;
                        LOG3(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2]receive config from DiagTool(bts)[IP:0x%x,Port:%d] exists,update TTL:%d",IP,Port,gDiagToolRecs.DiagToolRec[index].TTL);
                        LOG2(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2]eid:0x%x flag:0x%x",*(UINT32*)((UINT8*)pDiagToolReg+ sizeof(BtsDiagHeader) +2),
                                 *(UINT16*)((UINT8*)pDiagToolReg+ sizeof(BtsDiagHeader) +6));
                        break;
                    }                       
                }       
            }
            if(DIAGTOOLREQ_TYPE == pDiagToolReg->header.type)
            {
                for(int index = 0; index < MAX_DIAGTOOL_REGCNT; index++)
                {
                    if ((gDiagToolRecs.DiagToolRec[index].IP == IP) && (gDiagToolRecs.DiagToolRec[index].Port == Port))
                    { 
                        gDiagToolRecs.DiagToolRec[index].Flag = DIAG_TOOLREG_REC_USEED;
                        gDiagToolRecs.DiagToolRec[index].TTL  = gVx_System_Clock;
                        LOG3(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2]receive config from DiagTool(bts) (IP:0x%x,Port:%d) exists,update TTL:%d",IP,Port,gDiagToolRecs.DiagToolRec[index].TTL);
                        break;
                    }
                    
                    if (gDiagToolRecs.CurrentCnt >= MAX_DIAGTOOL_REGCNT)
                    {
                        //返回不能注册
                        LOG1(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2] gDiagToolRecs.CurrentCnt:%d >= MAX_DIAGTOOL_REGCNT can not register",gDiagToolRecs.CurrentCnt);
                        break;
                    }
                    
                    if (gDiagToolRecs.DiagToolRec[index].Flag == DIAG_TOOLREG_REC_NOTUSEED)
                    { 
                        gDiagToolRecs.DiagToolRec[index].Flag = DIAG_TOOLREG_REC_USEED;
                        gDiagToolRecs.DiagToolRec[index].TTL  = gVx_System_Clock;
                        gDiagToolRecs.DiagToolRec[index].IP   = IP;
                        gDiagToolRecs.DiagToolRec[index].Port = Port;
                        gDiagToolRecs.CurrentCnt++;
                        LOG2(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2]DiagTool(bts) [IP:0x%x,Port:%d] register successful!!!",IP,Port);
                        LOG1(LOG_DEBUG2, LOGNO(DIAG,0), "[DIAG L3->L2]the number of DiagTool(bts) users: %d",gDiagToolRecs.CurrentCnt);
                        break;
                    }
                }
                pDiagToolReg->IP = L3BackplaneIpAddr.s_addr;
                pDiagToolReg->Port = DIAG_SENDEMS_PORT;
                LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2]tDiagL3 pDiagToolReg->Username = %s",(int)pDiagToolReg->Username);
                LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2]tDiagL3 pDiagToolReg->Password = %s",(int)pDiagToolReg->Password);
            }

            //将消息发到l2oam
            sockaddr_in target;
            int nChars;
            target.sin_family = AF_INET;
            target.sin_addr.s_addr = L2BackplaneIpAddr.s_addr;
            target.sin_port = htons(DIAG_RCVEMS_PORT);
            nChars = ::sendto(m_sfdL3L2, (char*)pComMsg->GetDataPtr(), (int)pComMsg->GetDataLength(), 0, (sockaddr*)&target, sizeof(target));
            if (nChars==SOCKET_ERROR)
                {
                LOG2(LOG_DEBUG1, LOGNO(DIAG,0), "[DIAG L3->L2] send packet to L2:0x%x : %d FAIL",target.sin_addr.s_addr,DIAG_RCVEMS_PORT);
                pComMsg->Destroy();
                return true;
                }
        }
        else
        {
            LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2]Diag tool packet length[%d] error", pComMsg->GetDataLength());
        }
    }
    else
    {
        LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2]Rx diag tool packet from error task[%d]", srcTid);
    }
    pComMsg->Destroy();
    return true;

}


bool CL3OamDiagEMSL3L2::CreateSocket()
{
    m_sfdL3L2 = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_sfdL3L2 ==INVALID_SOCKET)
        return false;
#ifndef __WIN32_SIM__
    OAM_LOGSTR1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2] create m_sfdl3l2[%d] ok!", m_sfdL3L2);
#endif
    return true;
}

void CL3OamDiagEMSL3L2::CloseSocket()
{
    
#ifdef __WIN32_SIM__
    ::closesocket( m_sfdL3L2 );
    ::WSACleanup();
#else
    OAM_LOGSTR1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2] close socket m_sfdl3l2[%d] !", m_sfdL3L2);
    ::close( m_sfdL3L2 );
#endif
//#ifndef __WIN32_SIM__
 //   OAM_LOGSTR1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L3->L2] close socket m_sfdl3l2[%d] !", m_sfdL3L2);
//#endif
}

#define M_DIAG_REC_AGEOUT   (/*9 * 60*/30)    /*秒*/
void CL3OamDiagEMSL3L2::CheckValid()
{
    for(int index = 0; index < MAX_DIAGTOOL_REGCNT; index++)
    {
        if(gDiagToolRecs.DiagToolRec[index].Flag == DIAG_TOOLREG_REC_USEED)
        { 
            if( gVx_System_Clock <= gDiagToolRecs.DiagToolRec[index].TTL )
                continue; 
            if(gVx_System_Clock - gDiagToolRecs.DiagToolRec[index].TTL > M_DIAG_REC_AGEOUT)
            {
                LOG2(LOG_DEBUG2, LOGNO(DIAG,0), "DiagTool(bts)[IP:0x%x,Port:%d] timeout",
                    gDiagToolRecs.DiagToolRec[index].IP,
                    gDiagToolRecs.DiagToolRec[index].Port);

                memset((UINT8*)&(gDiagToolRecs.DiagToolRec[index]),0,sizeof(T_DiagToolRec));
                if(gDiagToolRecs.CurrentCnt > 0)
                {
                    gDiagToolRecs.CurrentCnt --;
                }
            }
        }
    }
    return;
}


CL3OamDiagEMSL3L2::~CL3OamDiagEMSL3L2()
{
}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
CL3OamDiagL2L3EMS::CL3OamDiagL2L3EMS()
:m_sfdL3Ems(INVALID_SOCKET), m_sfdL2L3(INVALID_SOCKET)
{
    ::strcpy(m_szName, "tDiagL2L3");
    m_uPriority  = M_TP_L3EMSAGENTRX;
    m_uOptions   = 0;
    m_uStackSize = 20000;
}

TID CL3OamDiagL2L3EMS::GetEntityId() const
{
    return M_TID_L3OAMDIAGL2L3EMS;
}
#ifdef WBBU_CODE
CL3OamDiagL2L3EMS*   CL3OamDiagL2L3EMS::  s_ptaskDiag = NULL;
#endif
bool CL3OamDiagL2L3EMS::Initialize()
{
    //Create receiving socket
    if (!CreateSocket())
    {
        LOG(LOG_SEVERE, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] Create socket failed. task crashed.");
        return false;
    }
    return true;
}
#ifdef WBBU_CODE
CL3OamDiagL2L3EMS* CL3OamDiagL2L3EMS::GetInstance()
{
    if ( NULL == s_ptaskDiag )
        {
        s_ptaskDiag = new CL3OamDiagL2L3EMS;
        }
    return s_ptaskDiag;
}
SOCKET g_sfdL3Ems = INVALID_SOCKET;
void CL3OamDiagL2L3EMS::send_2_diagtool(unsigned char *pBuf,unsigned int Datalen)
{
           if((pBuf==NULL)||(Datalen==0))
           	{
           	     return;
           	}
		   if(g_sfdL3Ems==INVALID_SOCKET)
		   	{
		    		g_sfdL3Ems= ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		   	}
		   if(g_sfdL3Ems!=INVALID_SOCKET)
		   	{
		   	Direct_num++;
            for (int index = 0; index < MAX_DIAGTOOL_REGCNT; index++)
            {
                if (gDiagToolRecs.DiagToolRec[index].Flag == DIAG_TOOLREG_REC_USEED)
                { 
                    //将消息发到diag tool.
                    int nChars;
                    sockaddr_in target;
                    target.sin_family = AF_INET;
                    target.sin_addr.s_addr = htonl(gDiagToolRecs.DiagToolRec[index].IP);
                    target.sin_port = htons(gDiagToolRecs.DiagToolRec[index].Port);
                    nChars = ::sendto(g_sfdL3Ems, (char*)pBuf, Datalen, 0, (sockaddr*)&target, sizeof(target));
                    if (nChars==SOCKET_ERROR)
                    {
                        LOG4(LOG_SEVERE, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] send packet to DiagTool(bts)[0x%x : %d,%d,%d] FAIL.",target.sin_addr.s_addr,target.sin_port,Datalen,errno);
                    }
                    else
                    {
                        LOG2(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] send packet to DiagTool(bts)[0x%x:%d]",target.sin_addr.s_addr,target.sin_port);
                    }
                }
                else
                {
                    //LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "tDiagL2 gDiagToolRecs.DiagToolRec[%d] unused",index);
                }
            }
		   	}
        
}
#endif
void CL3OamDiagL2L3EMS::MainLoop()
{
    int Datalen;
    sockaddr_in client;
    int sockaddrlen;
    sockaddrlen = sizeof(client);

    for (;;)
    {
        char* pBuf = new char[M_EMS_MAX_MSGLEN+1];
        Datalen = ::recvfrom(m_sfdL2L3, pBuf, M_EMS_MAX_MSGLEN + 1, 0, (sockaddr*)&client, &sockaddrlen);

        if (Datalen==SOCKET_ERROR)
        {
            //On WIN32, the exceed max size error cause recvfrom return SOCKET_ERROR
            //while on Vxworks it return number of bytes received.
            //so we do extra judge here
            LOG(LOG_SEVERE, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] Rx diag packet from L2 FAIL.");
            delete []pBuf;
            taskDelay(1);
            continue;
        }
        LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] received packet from L2 datalen = %d",Datalen);
#ifdef WBBU_CODE
	  Recv_num++;
#endif
        if (Datalen < M_EMS_MAX_MSGLEN+1)
        {
            for (int index = 0; index < MAX_DIAGTOOL_REGCNT; index++)
            {
                if (gDiagToolRecs.DiagToolRec[index].Flag == DIAG_TOOLREG_REC_USEED)
                { 
                    if( gDiagToolRecs.DiagToolRec[index].TTL < gVx_System_Clock &&
                        DIAGTOOL_BCUTMONITOR_GLOBALINFO <= ((BtsDiagHeader*)pBuf)->type &&
                        DIAGTOOL_BCUTMONITOR_RSP >= ((BtsDiagHeader*)pBuf)->type )
                    {
                        continue;
                    }
                    //将消息发到diag tool.
                    int nChars;
                    sockaddr_in target;
                    target.sin_family = AF_INET;
                    target.sin_addr.s_addr = htonl(gDiagToolRecs.DiagToolRec[index].IP);
                    target.sin_port = htons(gDiagToolRecs.DiagToolRec[index].Port);
#ifndef WBBU_CODE
                    nChars = ::sendto(m_sfdL3Ems, pBuf, Datalen, 0, (sockaddr*)&target, sizeof(target));
                    if (nChars==SOCKET_ERROR)
                    {
                        LOG2(LOG_SEVERE, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] send packet to DiagTool(bts)[0x%x : %d] FAIL.",target.sin_addr.s_addr,target.sin_port);
                    }
                    else
                    {
                        LOG2(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] send packet to DiagTool(bts)[0x%x:%d]",target.sin_addr.s_addr,target.sin_port);
                    }
#else
			if(L2_packet_Test==2)
			{
				if(Datalen<200)
				{
		                   nChars = ::sendto(m_sfdL3Ems, pBuf, Datalen, 0, (sockaddr*)&target, sizeof(target));
					Send_num++;
		                    if (nChars==SOCKET_ERROR)
		                    {
		                        LOG2(LOG_SEVERE, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] send packet to DiagTool(bts)[0x%x : %d] FAIL.",target.sin_addr.s_addr,target.sin_port);
						Err_num++;
		                    }
		                    else
		                    {
		                        LOG3(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] send packet to DiagTool(bts)[0x%x:%d,%d]",target.sin_addr.s_addr,target.sin_port,Datalen);
		                    }
				}
			}
			else
			{
				      nChars = ::sendto(m_sfdL3Ems, pBuf, Datalen, 0, (sockaddr*)&target, sizeof(target));
					Send_num++;
		                    if (nChars==SOCKET_ERROR)
		                    {
		                        LOG2(LOG_SEVERE, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] send packet to DiagTool(bts)[0x%x : %d] FAIL.",target.sin_addr.s_addr,target.sin_port);
						Err_num++;
		                    }
		                    else
		                    {
		                        LOG3(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] send packet to DiagTool(bts)[0x%x:%d,%d]",target.sin_addr.s_addr,target.sin_port,Datalen);
		                    }
			}
#endif
                }
                else
                {
                    //LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "tDiagL2 gDiagToolRecs.DiagToolRec[%d] unused",index);
                }
            }
        }
        else
        {
            LOG(LOG_SEVERE, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] packets received from L2 length error");
        }
        delete []pBuf;
    }
}

bool CL3OamDiagL2L3EMS::CreateSocket()
{
/////////////socket between l2 l3/////////////////
    sockaddr_in server;
    m_sfdL2L3= ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_sfdL2L3==INVALID_SOCKET)
    {
        LOG(LOG_CRITICAL, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] Create socket failed.");
        return false;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(DIAG_SENDEMS_PORT);
    if (::bind(m_sfdL2L3, (sockaddr*)&server, sizeof(server))==SOCKET_ERROR)
    {
        ::closesocket(m_sfdL2L3);
        return false;
    }
    LOG1(LOG_DEBUG3, LOGNO(DIAG,0), "[DIAG L2->L3->DiagTool] bind to [%d] ok! ",DIAG_SENDEMS_PORT);
#ifndef __WIN32_SIM__
    OAM_LOGSTR1(LOG_DEBUG3, 0, "[DIAG L2->L3->DiagTool] create m_sfdl2l3[%d] ok!", m_sfdL2L3);
#endif

/////////////socket between l3 ems /////////////////
    m_sfdL3Ems= ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_sfdL3Ems ==INVALID_SOCKET)
        return false;
#ifndef __WIN32_SIM__
    OAM_LOGSTR1(LOG_DEBUG3, 0, "[DIAG L2->L3->DiagTool] create m_sfdl3ems[%d] ok!", m_sfdL3Ems);
#endif
    return true;
}

CL3OamDiagL2L3EMS::~CL3OamDiagL2L3EMS()
{
}

void showDiagTool()
{
    printf("\r\n%d", sizeof(T_DiagToolUtMonitorReq) );
    printf("\r\n%-20s%-20s%-20s%-20s%-20s", "IP", "Port", "TTL", "USED", "SYS_CLOCK" );
    for (int idx = 0; idx < MAX_DIAGTOOL_REGCNT; ++idx)
        {
//        if (0 == gDiagToolRecs.DiagToolRec[idx].Flag)
//            continue;
        UINT8 *ucp = (UINT8*)(&gDiagToolRecs.DiagToolRec[idx].IP);
//        printf("\r\n%-20X %-20d %-20d %-20s %-20s", 
            gDiagToolRecs.DiagToolRec[idx].IP,
        printf("\r\n%-3d.%-3d.%-3d.%-3d     %-20d%-20d%-20s%-20d", 
            *ucp, *(ucp+1), *(ucp+2), *(ucp+3),
            gDiagToolRecs.DiagToolRec[idx].Port,
            gDiagToolRecs.DiagToolRec[idx].TTL,
            (gDiagToolRecs.DiagToolRec[idx].Flag)?"TRUE":"FALSE",
            gVx_System_Clock
            );
        }
    printf("\r\n");
}
#ifdef WBBU_CODE
extern "C"  void print_diag(unsigned char flag)
{
      if(flag==0)
      	{
      	   Recv_num = 0;
	   Send_num = 0;
	   Err_num = 0;
	   Direct_num = 0;
      	}
	  else
	  {
	      printf("diag :%d,%d,%d,%d\n",Recv_num,Send_num,Err_num,Direct_num);
	  }
}
 void send_2_diag_tools(unsigned char *pBuf,unsigned int Datalen)
{
           static CL3OamDiagL2L3EMS *pDiadInstance = NULL;
           pDiadInstance = CL3OamDiagL2L3EMS::GetInstance();
           pDiadInstance->send_2_diagtool(pBuf,Datalen);
}

void test_diag()
{
    unsigned char pdata[100];
   for(int i = 0; i< 100; i++)
   	{
   	  pdata[i] = i;
   	}
   send_2_diag_tools(pdata,100);
}	
#endif

