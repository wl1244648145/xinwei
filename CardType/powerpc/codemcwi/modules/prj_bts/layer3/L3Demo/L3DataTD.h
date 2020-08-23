/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTDR.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   11/22/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_TDR_H__
#define __DATA_TDR_H__

//socket:
#ifdef __WIN32_SIM__
#include <winsock2.h>
#else   //VxWorks:
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

#include "L3DataTypes.h"
#include "L3DataTDRMeasure.h"

//TDR任务参数定义
#define M_TASK_TDR_TASKNAME      "tTDR"
#ifdef __WIN32_SIM__
#define M_TASK_TDR_OPTION        (0x0008)
#define M_TASK_TDR_MSGOPTION     (0x02)
#else
#define M_TASK_TDR_OPTION        ( VX_FP_TASK )
#define M_TASK_TDR_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TDR_STACKSIZE     (20480)
#define M_TASK_TDR_MAXMSG        (1024)


/******************************************
 *M_TDR_BUFFER_LENGTH   : 最大的RawSocket
 *接收缓冲区大小 (Byte)
 *M_TDR_BUFFER_NUM      : 用于RawSocket接收
 *的缓冲区个数
 ******************************************/
#define M_TDR_BUFFER_LENGTH     (2048)
#define M_TDR_BUFFER_NUM        (500)

//TDR task
class CTaskTDR : public CTask
{
public:
    static CTaskTDR* GetInstance();
    void showStatus();

private:
    CTaskTDR();
    ~CTaskTDR();

    inline TID GetEntityId() const 
        {
        return M_TID_TDR;
        }
    bool Initialize();
    void MainLoop();
    bool DeallocateComMessage(CComMessage*);

    void ForwardToEB(UINT8*, UINT32, UINT32);

    //--------------
    //设置统计值 +1
    //--------------
    inline UINT32 IncreaseMeasureByOne( TDR_MEASURE_TYPE type )
        {
        return m_aulMeasure[ type ] += 1;
        }

    void   InitFreeBufferList();
    UINT8* GetBufferFromPool();
    void   ReclaimFreeBuffer(UINT8*);
	UINT32 GetHostIpAddr();

private:

    UINT8  m_aucBufferPool[ M_TDR_BUFFER_NUM ][ M_TDR_BUFFER_LENGTH ];
    list<UINT8*> m_listFreeBuffer;      //空闲Buffer的链表
    UINT16 m_usUsedBuffer;

#ifdef __WIN32_SIM__
    SOCKET m_EtherIpSocket;     //Windows:
    WSADATA m_wsaData;
#else
    UINT32 m_EtherIpSocket;     //VxWorks:
#endif

    /*************************************************************
     *performance Measurement:
     *m_aulInMeasure[ MEASURE_TDR_MAX ]记录TDR接收到的
     *不同数据类型的统计值
     ************************************************************/
    UINT32 m_aulMeasure[ MEASURE_TDR_MAX ];

    //static members:
    static CTaskTDR* s_ptaskTDR;
};

#endif /*__DATA_TDR_H__*/
