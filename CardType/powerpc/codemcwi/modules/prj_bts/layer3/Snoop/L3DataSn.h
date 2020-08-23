/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoop.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   03/30/06   xiao weifang  NVRAM�ָ��û���Ϣ. 
 *   09/02/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_SNOOP_H__
#define __DATA_SNOOP_H__

#include "BizTask.h"
#include "ComMessage.h"

#include "L3DataTypes.h"
#include "L3DataMacAddress.h"
#include "L3DataMessages.h"
#include "log.h"

#include "L3DataRaid.h"
#include "L3DataSnoopFSM.h"
#include "L3DataSnoopMeasure.h"
#include "L3DataFTCheckVlan.h"

//Snoop�����������
#define M_TASK_SNOOP_TASKNAME      "tSNOOP"
#ifdef __WIN32_SIM__
#define M_TASK_SNOOP_OPTION        (0x0008)
#define M_TASK_SNOOP_MSGOPTION     (0x02)
#else
#define M_TASK_SNOOP_OPTION        ( VX_FP_TASK )
#define M_TASK_SNOOP_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_SNOOP_STACKSIZE     (20480)
#define M_TASK_SNOOP_MAXMSG        (1024)

//���ش���
#define SNOOP_FLOWCTRL_CNT          (100)


class CTSnoop : public CBizTask
{
public:
    static CTSnoop* GetInstance();
    void showStatus();
    UINT32 getIpByMac(UINT8*);
    void showCCBinMem(UINT32);
    void showCCBinNVRAM(UINT32);
    void showPerf();
    void showCCBbyMac(CMac &Mac);

    UINT32 GetRouterAreaId()          const { return m_ulRouterAreaId; }
    bool   GetRelayOptionEnable()     const { return m_bAgentCircuitId || m_bRemoteCircuitId ; }
    bool   GetAgentCircuitIdEnable()  const { return m_bAgentCircuitId; }
    bool   GetRemoteCircuitIdEnable() const { return m_bRemoteCircuitId ; }
    bool   GetPPPoERemoteIdEnable() const { return m_bPPPoERemoteID ; }
    void   SetPPPoERemoteIdEnable(bool blva)  {  m_bPPPoERemoteID =blva; }
    //������ȷ��pData�ڴ��㹻��
    void   GetPerfData(UINT8*);
    void   ClearMeasure();
    bool   isLeaseUpdated(const UINT32, const UINT32, const CMac&);

#ifdef UNITEST
/*UNITESTʱ,���еĳ�Ա��������Public*/
public:
#else
private:
#endif
    CTSnoop();
    ~CTSnoop();

    //bool Initialize();
    bool ProcessMessage(CMessage&);
    inline TID GetEntityId() const { return M_TID_SNOOP; }
    inline bool IsNeedTransaction(){ return false; }
#ifndef _NO_NVRAM_RECOVER_
    void MainLoop();
#endif

    #define SNOOP_MAX_BLOCKED_TIME_IN_10ms_TICK (200)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return SNOOP_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    bool SnoopConfig(const CRAIDConfig&);
    bool SnoopCheckGroup(const CFTCheckVLAN&);
    bool clearCPEData(UINT32);

    //---------------------------------------------
    //��ȡָ��ת������(Ingress/Egress/TDR)�İ�ͳ��ֵ
    //---------------------------------------------
    UINT32 GetMeasure( IN_TYPE type )
        {
        DATA_assert( type < IN_TYPE_MAX );
        return m_aulInMeasure[ type ];
        }

    //---------------------------------------------
    //����ָ�����Ͱ�ͳ��ֵ +1
    //---------------------------------------------
    UINT32 IncreaseMeasureByOne( IN_TYPE type )
        {
        DATA_assert( type < IN_TYPE_MAX );
        return m_aulInMeasure[ type ] += 1;
        }

#ifdef UNITEST
    static void DeleteInstance();
    CSnoopFSM&  GetFSM() { return m_FSM; }
#endif

private:
    //members.
    UINT32 m_ulRouterAreaId;
    bool   m_bAgentCircuitId;
    bool   m_bRemoteCircuitId;
    bool   m_bPPPoERemoteID;	
    //״̬��
    CSnoopFSM m_FSM;

    /*************************************************************
     *performance Measurement:
     *m_aulInMeasure[ IN_TYPE_MAX ]��¼Snoop���յ���
     *��ͬ�������͵�ͳ��ֵ
     ************************************************************/
    UINT32 m_aulInMeasure[ IN_TYPE_MAX ];

    //static members:
    static CTSnoop *s_ptaskSnoop;
};

#endif /*__DATA_SNOOP_H__*/

