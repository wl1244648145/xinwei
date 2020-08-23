/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnel.h
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

#ifndef __DATA_TUNNEL_H__
#define __DATA_TUNNEL_H__

//socket:
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

#include <map>
using namespace std;
#include <list>

#include "BizTask.h"
#include "ComMessage.h"
#include "log.h"
#include "timer.h"

#include "L3DataTypes.h"
#include "L3DataMacAddress.h"
#include "L3DataMessages.h"
#include "L3DataTunnelTimer.h"
#include "L3DataTunnelMeasure.h"
#include "L3DataAssert.h"

//Tunnel任务参数定义
#define M_TASK_TUNNEL_TASKNAME      "tTunnel"
#ifdef __WIN32_SIM__
#define M_TASK_TUNNEL_OPTION        (0x0008)
#define M_TASK_TUNNEL_MSGOPTION     (0x02)
#else
#define M_TASK_TUNNEL_OPTION        ( VX_FP_TASK )
#define M_TASK_TUNNEL_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TUNNEL_STACKSIZE     (20480)
#define M_TASK_TUNNEL_MAXMSG        (1024)


/******************************************
 *M_TUNNEL_REQUEST_CB_NUM   : 最大控制块个数
 *M_TUNNEL_SOCKET_PORT      : 端口号
 ******************************************/
#define M_TUNNEL_REQUEST_CB_NUM     (1024)
#define M_TUNNEL_SOCKET_PORT        (22222)


/***************************
 *Tunnel Management Request
 *Control Block
 **************************/
#pragma pack (1)
typedef struct _tag_TMReqCB
{
    CMac    Mac;
    UINT8   ucCount;
    CTimer *pTimer;
    CComMessage *pComMsgList;

////member function.
    void Reclaim()
    {
        //回收控制块
        if ( NULL != pTimer )
            {
            pTimer->Stop();
            delete pTimer;
            pTimer = NULL;
            }

        //删除消息链表
        CComMessage *p = pComMsgList;
        while(NULL != p)
            {
            pComMsgList = p->getNext();
            p->Destroy();
            p = pComMsgList;
            }

        //控制块清0
        memset(&Mac, 0, sizeof(CMac));
        ucCount   = 0;
    }
} TMReqCB;
#pragma pack ()



//TUNNEL task
class CTunnel : public CBizTask
{
public:
    static CTunnel* GetInstance();
    void   showStatus();
    void   GetPerfData(UINT8 *pData)    { memcpy( pData, m_aulMeasure, sizeof( m_aulMeasure ) ); }
    void   ClearMeasure()               { memset( m_aulMeasure, 0, sizeof( m_aulMeasure ) ); }

#ifdef UNITEST
    /*UNITEST时,所有的成员函数都是Public*/
public:
#else
private:
#endif
    CTunnel();
    ~CTunnel();

    bool Initialize();
    bool ProcessMessage(CMessage&);
    inline TID GetEntityId() const 
        {
        return M_TID_TUNNEL;
        }
    inline bool IsNeedTransaction()
        {
        return false;
        }

    #define TUNNEL_MAX_BLOCKED_TIME_IN_10ms_TICK (200)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return TUNNEL_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    // Free FT Records List methods
    void   InitFreeCBList();
    void   InsertFreeCB(UINT16);
    UINT16 GetFreeCBIndexFromList();

    // BPtree methods
    bool   BPtreeAdd(CMac&, UINT16);
    bool   BPtreeDel(CMac&);
    UINT16 BPtreeFind(CMac&);

    //---------------------------------------------
    //返回指定下标的控制块
    //---------------------------------------------
    TMReqCB* GetCBbyIdx(UINT16 usIdx)
        {
        if ( usIdx >= M_TUNNEL_REQUEST_CB_NUM )
            {
            return NULL;
            }
        return &( m_CB[ usIdx ] );
        }

    bool    SendTunnelRequest (CMessage&);
    bool    ForwardToSOCKET(CMessage&);
    bool    ForwardToSOCKET(CComMessage*);

    void    ReceiveFromTCR(CMessage&);
    void    ReceiveTunnelResponse(CMessage&, UINT16);
    bool    SendSuspendTunnelRequest(TMReqCB*);
    void    ForwardToSnoop(CMessage&, UINT16);
    void    ProcTimeOut(const CTunnelTimerExpire&);
    CTimer* StartTunnelTimer(CMac&, UINT32);

    //--------------
    //设置统计值 +1
    //--------------
    inline UINT32 IncreaseMeasureByOne( TUNNEL_MEASURE_TYPE type )
        {
        return m_aulMeasure[ type ] += 1;
        }

private:

    TMReqCB m_CB[ M_TUNNEL_REQUEST_CB_NUM ];
    list<UINT16> m_listFreeCB;      //空闲控制块的链表
    map<CMac, UINT16> m_CBptree;    //控制块索引树

    /*************************************************************
     *performance Measurement:
     *m_aulInMeasure[ MEASURE_TUNNEL_MAX ]记录TUNNEL接收到的
     *不同数据类型的统计值
     ************************************************************/
    UINT32 m_aulMeasure[ MEASURE_TUNNEL_MAX ];

    //static members:
    static CTunnel* s_ptaskTunnel;
};

#endif /*__DATA_TUNNEL_H__*/
