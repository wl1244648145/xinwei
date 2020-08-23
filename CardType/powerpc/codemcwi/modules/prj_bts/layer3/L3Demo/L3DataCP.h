/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataCPESM.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   1/12/05    xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_CPESM_H__
#define __DATA_CPESM_H__

#include "Task.h"
#include "MsgQueue.h"
#include "ComMessage.h"
#include "log.h"

#include "L3DataTypes.h"

//CPESM任务参数定义
#define M_TASK_CPESM_TASKNAME      "tCPESM"
#ifdef __WIN32_SIM__
#define M_TASK_CPESM_OPTION        (0x0008)
#define M_TASK_CPESM_MSGOPTION     (0x02)
#else
#define M_TASK_CPESM_OPTION        ( VX_FP_TASK )
#define M_TASK_CPESM_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_CPESM_STACKSIZE     (20480)
#define M_TASK_CPESM_MAXMSG        (1024)

#define M_EVENT_MSGQ               ("MsgQEvt")
#define M_EVENT_TRAFFIC            ("TrafficQEvt")


//CPESM task
class CTaskCPESM : public CTask
{
public:
    static CTaskCPESM* GetInstance();
    static void SendToUTDM(int len,UINT8 *pkt_data);
#ifdef TEST_UTDM
	static void Egress(int len,UINT8 *pkt_data);
#endif
#ifdef UNITEST
    /*UNITEST时,所有的成员函数都是Public*/
public:
#else
private:
#endif
    CTaskCPESM();
    ~CTaskCPESM(){}

    bool ProcessMessage(CMessage&);
    inline TID GetEntityId() const 
        {
        return M_TID_CPESM;
        }
    virtual void MainLoop();
    virtual bool PostMessage(CComMessage *, SINT32, bool = false);
    bool Initialize();

    void InBoundMsgProcess();
    void CaptureTraffic();
	void swap16(UINT8 *pData, UINT32 ulLen);
private:

    HANDLE m_hMsgQEvent;
    HANDLE m_hTrafficQEvent;
    HANDLE m_arrHandle[ 2 ];
    CMsgQueue *m_pMsgQ;
    SINT32 m_lMaxMsgs;
    SINT32 m_lMsgQOption;

    //static members:
    static CTaskCPESM* s_ptaskCPESM;
};

#endif /*__DATA_DAC_H__*/
