/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3l2Tcr.h
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

#ifndef __L3L2_Tcr_H__
#define __L3L2_Tcr_H__

#ifndef __WIN32_SIM__
#include <taskLib.h>
#endif

#include "datatype.h"
#include "taskdef.h"

#include "Task.h"


#define DEFAULT_PORT            (11111)
#define DEFAULT_LENTH   (2048)

#define DEFAULT_DATA_LENTH (64)

//ARP 任务参数定义
#define M_TASK_L3L2TCR_LEN          (20)
#define M_TASK_L3L2TCR_TASKNAME     "tL3L2TCR"
#ifdef __WIN32_SIM__
#define M_TASK_L3L2TCR_OPTION       (0x0008)
#define M_TASK_L3L2TCR_MSGOPTION    (0x02)
#else
#define M_TASK_L3L2TCR_OPTION       ( VX_FP_TASK )
#define M_TASK_L3L2TCR_MSGOPTION    ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_L3L2TCR_STACKSIZE    (20480)



class CTaskL3L2Tcr : public CTask
{
public:
    static CTaskL3L2Tcr* GetInstance();
    void ShowStatus(); 
	UINT32 GetSrcIpAddr();
private:
    CTaskL3L2Tcr();
    ~CTaskL3L2Tcr();

    bool Initialize();
    void MainLoop();

    inline TID GetEntityId() const 
        {
        #ifdef M_TGT_CPE
			return M_TID_UTAGENT;
		#elif M_TGT_L3
        	return M_TID_L2IF;
		#endif
        }
	
     static CTaskL3L2Tcr* Instance;

#ifdef __WIN32_SIM__
     SOCKET m_UDPSocket;//Windows:
     WSADATA m_wsaData;
#else
     UINT32 m_UDPSocket;//VxWorks:
#endif
    UINT32  m_SrcIpAddr;
    UINT32  m_RecvPacketCount;
#pragma pack (1)

	typedef struct _InterComm
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

