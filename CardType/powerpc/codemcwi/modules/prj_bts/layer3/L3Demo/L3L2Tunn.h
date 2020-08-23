/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3l2Tunnel.h
 *
 * DESCRIPTION: 
 *   details of Task TCR
 *  the main function of TCR is,create a readsocket with UDP port 22222,then send Recv
 *  to Tunnul Task
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   11/08/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __L3L2_Tunnel_H__
#define __L3L2_Tunnel_H__

#ifndef __WIN32_SIM__
#include <taskLib.h>
#endif
#ifdef __WIN32_SIM__
#include <winsock2.h>
#else	//VxWorks:
#include "vxWorks.h" 
#include "sockLib.h" 
#include "inetLib.h" 
#include "stdioLib.h" 
#include "strLib.h" 
#include "hostLib.h" 
#include "ioLib.h" 
#endif

#include "datatype.h"
#include "taskdef.h"

#include "BizTask.h"
#include "ComMessage.h"

#define DEFAULT_PORT            (11111)
#define DEFAULT_LENTH   (2048)

#define DEFAULT_DATA_LENTH (64)


//ARP 任务参数定义
#define M_TASK_L3L2TUNNEL_LEN          (20)
#define M_TASK_L3L2TUNNEL_TASKNAME     "tTunnel"
#ifdef __WIN32_SIM__
#define M_TASK_L3L2TUNNEL_OPTION       (0x0008)
#define M_TASK_L3L2TUNNEL_MSGOPTION    (0x02)
#else
#define M_TASK_L3L2TUNNEL_OPTION       ( VX_FP_TASK )
#define M_TASK_L3L2TUNNEL_MSGOPTION    ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_L3L2TUNNEL_STACKSIZE    (20480)


//TUNNEL错误码定义
const UINT16 ERR_TCR_NORMAL          = 0x0000;  //正常流程
const UINT16 ERR_TCR_DATAL           = 0x0001;  //非法Message ID
const UINT16 ERR_TCR_SYS             = 0x0002;  //系统错误
const UINT16 ERR_TCR_SOCKET          = 0x0003;  //Socket错误


class CTaskL3L2Tunnel : public CBizTask
{
public:
    static CTaskL3L2Tunnel* GetInstance();

private:
    CTaskL3L2Tunnel();
    ~CTaskL3L2Tunnel();

    bool Initialize();
	bool ProcessComMessage(CComMessage * pComMsg);
	bool Sendsocketdata(UINT32 , void* , UINT32 );
	virtual TID GetEntityId() const { return CurrentTid;};

	TID CurrentTid;
	UINT32 m_ulEID;
	
#ifdef M_TGT_L3
    static const TID ProxyTIDs[2]; 
	void   swap16(UINT8 *, UINT32 );
#elif M_TGT_L2
    static const TID ProxyTIDs[2]; 
#endif

    UINT32 GetDestIpAddr();

#ifdef __WIN32_SIM__
     SOCKET m_UDPSocket;//Windows:
     WSADATA m_wsaData;
#else
     UINT32 m_UDPSocket;//VxWorks:
#endif
     static CTaskL3L2Tunnel* s_ptaskL3L2Tunnel;
#pragma pack (1)

	typedef struct T_InterComm
	{
    UINT8 reserver[50];
	TID m_tidDst;
    TID m_tidSrc;
    UINT16 m_uMsgId;
    UINT32 m_uEID;
	UINT8  databuf[DEFAULT_LENTH];

	}T_InterComm;
#pragma pack ()

};



#endif

