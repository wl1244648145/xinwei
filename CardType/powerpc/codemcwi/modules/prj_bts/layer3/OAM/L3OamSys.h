/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/


#ifndef _INC_L3OAMSYSTEM
#define _INC_L3OAMSYSTEM

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif


#ifndef _INC_L3OAMSESSIONIDUPDATEREQ
#include "L3OamSessionIdUpdateReq.h"
#endif

#ifndef _INC_L3OAMMODULEBOOTUPNOTIFY
#include "L3OamModuleBootupNotify.h"
#endif

#ifndef _INC_L3OAMBTSRSTNOTIFY
#include "L3OamBtsRstNotify.h"
#endif

#ifndef _INC_L3OAMBTSREGREQ
#include "L3OamBtsRegReq.h"
#endif

#ifndef _INC_L3OAMBTSREGRSP
#include "L3OamBtsRegRsp.h"
#endif

#ifndef _INC_L3OAMBTSREGNOTIFY
#include "L3OamBtsRegNotify.h"
#endif


#ifndef _INC_L3OAMBTSDATADLOADNOTIFY
#include "L3OamBtsDataDLoadNotify.h"
#endif


#ifndef _INC_L3OAMSETCPETIMENOTIFY
#include "L3OamSetCpeTimeNotify.h"
#endif


#ifndef _INC_L3OOModuleInitNOTIFY
#include "L3OOModuleInitNotify.h"
#endif

#ifndef _INC_L3OAMSESSIONIDUPDATEREQ
#include "L3OamSessionIdUpdateReq.h"
#endif

#ifndef _INC_TIMEOUTNOTIFY
#include "TimeOutNotify.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

#ifndef _INC_L3OAMALMNOTIFYOAM
#include "L3OamAlmNotifyOam.h"
#endif

#ifndef _INC_L3L2BTSRSTCNTCHANGENOTIFY
#include "L3L3L2BtsRstCntChangeNotify.h"
#endif

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#ifndef _INC_L3OAMCOMMONREQ
#include "L3OamCommonReq.h"
#endif

#ifndef _INC_L3OAMCOMMONRSP
#include "L3OamCommonRsp.h"
#endif

#ifndef _INC_L3L3L2AUXSTATENOTIFY
#include "L3L3L2AuxStateNotify.h"
#endif

#ifndef _INC_L3L3L2MCPSTATENOTIFY
#include "L3L3L2McpStateNotify.h"
#endif

#ifndef _INC_L3L3L2RFSTATENOTIFY
#include "L3L3L2RFStateNotify.h"
#endif

#ifndef _INC_L3OAMCPUWORKINGNOTIFY
#include "L3OamCpuWorkingNotify.h"
#endif

#ifndef _INC_L3OACPUALARMNOTIFY
#include "L3OAMCpuAlarmNofity.h"
#endif

#pragma pack(1)
struct T_BtsSysStatusData
{
    UINT8   SatellitesVisible;      //0--12 
    UINT8   StatellitesTrack;       //0--12 
    UINT16  BootSrc;
    UINT16  RfMask;
    SINT16  SyncCardTemperature;    //      当前频踪板温度
    SINT16  DigitalBoardTemperature;//当前主用板温度
    UINT32  TimePassedSinceBtsBoot;     //ms    
};

struct T_WrapToEMS
{
#define M_SENDTO_MAIN_EMS (0x55)
#define M_SENDTO_BAK_EMS  (0xaa)
    UINT16 usMsgCode;
    UINT8  ucToMainEMS;
};

#pragma pack()


/*
 *EMS链路状态,占用UINT8的bit位
 */
#define M_MAIN_EMS_REGISTER     (0x01)
#define M_MAIN_EMS_WORK         (0x02)
#define M_MAIN_EMS_DOWN         (0x04)
#define M_BACKUP_EMS_REGISTER   (0x10)
#define M_BACKUP_EMS_WORK       (0x20)
#define M_BACKUP_EMS_DOWN       (0x40)

extern "C" bool notifyAllCPEtoRegister();
extern "C" bool notifyOneCpeToRegister(UINT32 EID,bool blStopDataSrv);

class CTaskSystem : public CBizTask
{
friend void l3oamsendbtsreg();
friend bool notifyAllCPEtoRegister();
public:
    CTaskSystem();
    static CTaskSystem* GetInstance();
    void   showSysStatus();
    void   SYS_setActiveEMS(UINT32, UINT32);
     void SYS_getLocalCfgFromCF();
private:
    static CTaskSystem * m_Instance;    
private:
    enum SYS_STATE
    {
        SYS_INIT = 0,
        SYS_WORK = 1   
    };

    enum CONST_DATA
    {
        SYS_BTS_REG_RSP_TIME     = (5 * 1000), 
        #ifndef WBBU_CODE
        SYS_L3L2SYSPROBE_PERIOD  = (200 * 1000),
        #else
	 SYS_L3L2SYSPROBE_PERIOD  = (60 * 1000),
	 #endif
        SYS_BTS_REG_PERIOD       = (10 * 1000),  
        SYS_BTS_TEMP_MON_PERIOD  = (60 * 60 * 1000),
        SYS_BTS_EMSLINK_PERIOD   = (30 * 1000),
        SYS_BTS_TIMEDAY_PERIOD	 = (1 * 1000),
        SYS_BTS_CLOCK_TIMER      = (1 * 1000),  
//        SYS_BTS_TCXO_ERR_PERIOD  = (1 * 1000 * 60),  
        SYS_DATACFG_PERIOD       = (5  * 60 * 1000),  
        SYS_CODE_LODE_PERIOD     = (5  * 60 * 1000),
#ifdef OAM_DEBUG
        SYS_BTS_INITFROMEMS_PERRIOD = (50 * 60 * 1000),
        SYS_BTS_INITFROMNVRAM_PERRIOD = (50 * 60 * 1000)
#else
        SYS_BTS_INITFROMEMS_PERRIOD = (5 * 60 * 1000),
        SYS_BTS_INITFROMNVRAM_PERRIOD = (5 * 60 * 1000)
#endif
    };

    struct T_SysStatus
    {
        UINT16 IsWork;       //假设有10个模块必须发送开工指示消息。在现阶段本字段可以不用，
        UINT8 BootLodeOk;   //0 - 未收到    1- 已收到; 表示已经收到bootload任务的代码加载完成消息,在收到注册完成消息后可以进行数据配置了
        UINT8 RegOk;        //0 - 未完成    1- 完成;   表示注册已经完成,在收到 bootload任务的代码加载完成消息后可以进行数据配置了                  
        UINT8 iSPeriodReg;  //0 - 系统未进行周期注册; 1 - 系统正进行周期注册
        UINT8 IsEmsLinkOk;  //0 - 链路断    1- 链路通 ;当同ems链路断时要重新进行注册.
        UINT8 NeverInitFromEMS;  // 1- 从未初始化     0 - 已经初始化、     
        UINT8 SendWorkToEMS;  // 0- 未发送开工指示消息     1 - 已发送开工指示消息 
    };
#define CPU_BOOT_SUCCESS    0x1
#define CPU_BOOT_FAIL       0x0
    struct T_BtsCpuWorkStatus  //每个CPU的状态
    {
        UINT8 OAM_L2PPC;    //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_AUX;      //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_MCP0;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_MCP1;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_MCP2;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_MCP3;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_MCP4;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_MCP5;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_MCP6;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_MCP7;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_FEP0;     //0 BOOT FAIL        1 - BOOT SUCCESS
        UINT8 OAM_FEP1;     //0 BOOT FAIL        1 - BOOT SUCCESS
#ifdef WBBU_CODE
        UINT8 OAM_CORE9;    //0 BOOT FAIL        1 - BOOT SUCCESS
#endif
    };

    struct T_CpuIsWorking  //CPU的整体状态
    {
        UINT8 OAM_L2PPC;    //0 NOT WORKING        1 - WORKING
        UINT8 OAM_AUX;      //0 NOT WORKING        1 - WORKING
        UINT8 OAM_MCP;      //0 NOT WORKING        1 - WORKING
        UINT8 OAM_FEP;      //0 NOT WORKING        1 - WORKING
        UINT8 OAM_L1;       //0 NOT WORKING        1 - WORKING
#ifdef WBBU_CODE
        UINT8 OAM_CORE9; //0 NOT WORKING        1 - WORKING
#endif
    };
private:
    //下面的函数是对应的消息处理函数
    void BtsSwUpdateCheck();
    bool SYS_BtsRegNotify(bool);
    bool SYS_BtsResetNotify();
    bool SYS_BtsRegReq(CBtsRegReq &rMsg);                       //M_EMS_BTS_REG_REQ
    bool SYS_SessionIdUpdateReq(CSessionIdUpdateReq &rMsg);
    bool SYS_EmsBtsHeartbeatReq(CL3OamCommonReq &rMsg);
    bool SYS_EmsLinkTimer(CL3OamCommonReq &rMsg);
    bool SYS_BtsDlFinReq(CL3OamCommonReq &rMsg);      //M_EMS_BTS_DL_FINISH_NOTIFY
    bool SYS_DataCfgFinishNotify(CL3OamCommonReq &rMsg);   //M_OAM_DATA_CFG_FINISH_NOTIFY
    bool SYS_BtsBootUp(CModuleBootupNotify &rMsg);  
    bool SYS_CpuWorkingNotify(CL3OamCpuWorkingNotify &);
    void SYS_L3CpuAlarmNofity(CL3CpuAlarmNofity &);
    void SYS_SetCpuWorkingStatus(UINT8, UINT8, UINT8);
//#ifdef LJF_WCPE_BOOT
public:
	bool m_bWcpeBootModelOK;
	bool m_bWcpeBootModel;
    bool SYS_SendDataDLNotify();
    bool SYS_SendBtsWorkingNotify();
   #ifdef WBBU_CODE
	void  FPGAConfigRFMask(unsigned char value);
   #endif
private:
    bool SYS_BtsResetCntIncReq(UINT16);
    bool SYS_BtsBootUpTimer();          //M_OAM_BTS_BOOT_UP_TIMER
    bool SYS_BtsSynCpeTimer();
    bool SYS_BtsRegNotifyTimer(CL3OamCommonReq &rMsg);
    bool SYS_BtsPeriodRegNotifyTimer(CL3OamCommonReq &rMsg);
    bool SYS_OamBtsRstNotify(CBtsRstNotify &);     
////bool SYS_ConfigSYNPower(UINT16);
    bool SYS_BtsDataInitTimer();
    bool SYS_L3L2SysProbeTimer();
    bool SYS_L3L2SysProbeRsp(CL3OamCommonRsp &);
#ifdef WBBU_CODE
    bool  SYS_L3L2SysProbeCore1Rsp(CL3OamCommonRsp &);//core1 probe
     bool SYS_L3L2SysProbeCore1Fail();
    bool SYS_L2L3Core9StateNotify(CL3L2Core9StateNoitfy&);
    bool SYS_L2L3AifInfoStateNotify(CL3L2AifStateNoitfy&);

#endif
    bool SYS_L3L2SysProbeFail();
    bool SYS_L2L3AuxStateNotify(CL3L2AuxStateNoitfy&);
    bool SYS_L2L3McpStateNotify(CL3L2McpStateNoitfy&);    
    bool SYS_L2L3RFStateNotify(CL3L2RFStateNoitfy&);
    bool SYS_SendBtsResetCntsNotify();
    bool SYS_AddBtsRstCnt();
    bool SYS_AddBtsRegCnt();
    bool SYS_StartL3L2MoniterTimer();
	bool SYS_StartSetL2TimeDayTimer();
    bool SYS_BtsRegRsp(UINT16, UINT16, bool);
    CTimer* SYS_Createtimer(UINT16 MsgId, UINT8 IsPeriod, UINT32 TimerPeriod);
#if 0
    bool SYS_SendAlmNotifyToEMS ( UINT32, UINT8, UINT16, UINT16, UINT16, UINT16, UINT8, SINT8 *pAlarmInfo);
#endif
    bool SYS_InitFromEmsFail();
    void SYS_SAGLinkAlm(CMessage &);
    bool SYS_SendNotifyMsg( TID tid, UINT16 msgid);
    void SYS_SystemClock();
    bool SYS_EMSDLHandler();
    void SYS_GetBtsSysStatusReq(UINT16 transid);
    void SYS_BtsDigitalBoardTemperature();
    bool SendCpuResetMsg(UINT8, UINT8);

    bool Initialize();
    bool ProcessMessage(CMessage&);
    TID  GetEntityId()  const; 

    #define SYS_MAX_BLOCKED_TIME_IN_10ms_TICK (1000)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return SYS_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    bool InitSysApp();                                          //初始化应用层逻辑
    void SYS_WrapToBakEMS(CMessage&);
private:
    CTimer *pL3L2ProbeTimer;
    CTimer *pBootupMonTimer;
    CTimer *pBtsRegNotifyTimer;
    CTimer *pBtsPeriodicRegTimer;                               //周期注册过程待定
    CTimer *pEmsLinkTimer;
    CTimer *pBtsDataInitTimer;
    CTimer *pEmsDataInitTimer;
    CTimer *pPeriodSetCpeTimeTimer;
    CTimer *pDigitalBoardTemperatureMonTimer;
    CTimer *pNotifyEmsDlDataTimer;
    CTimer *m_pSystemClockTimer;
////CTimer *pTXCOCalibrationErrTimer;
    T_SysStatus  m_SysStatus;
    T_BtsCpuWorkStatus m_BtsCpuWorkStatus;
    T_CpuIsWorking m_CpuIsWorking;
    T_MCPStateInfo m_lastMCPStateInfo;
    T_AUXStateInfo m_lastAUXStateInfo;
    T_RFStateInfo  m_lastRFStateInfo;
#ifdef WBBU_CODE
    T_Core9StateInfo  m_lastCore9StateInfo;
    T_AifStateInfo  m_lastAifInfo;
#endif
    UINT8          m_emsState;
     CTimer *pGetBtsTimerFromEms;//定时向ems查询时间,周期为1分钟
public:
//	#ifdef LJF_RPT_ALTER_2_NVRAMLIST
	bool SYS_IsEmsConnectOK();
};

void GetOAMPerfData(UINT8 *pData);
#endif 


