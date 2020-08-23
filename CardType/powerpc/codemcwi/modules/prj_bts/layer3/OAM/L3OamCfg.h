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
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMCFG
#define _INC_L3OAMCFG

#ifdef __WIN32_SIM__
#include <windows.h>
#else
#include <stdio.h>
#endif


#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif

///////////////////////////////////////////////////////////
#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#ifndef _INC_L3OAMCOMMONREQ
#include "L3OamCommonReq.h"
#endif

#ifndef _INC_L3OAMCOMMONRSP
#include "L3OamCommonRsp.h"
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

////////////////////////////////////////////////////
//Configuration Management（MA=0）
#ifndef _INC_L3OAMCFGBTSGENDATAREQ
#include "L3OamCfgBtsGenDataReq.h"  
#endif

#ifndef _INC_L3OAMCFGAIRLINKMISCREQ
#include "L3OamCfgAirLinkMisc.h"       
#endif

#ifndef _INC_L3OAMCFGAIRLINKREQ
#include "L3OamCfgAirLinkReq.h"        
#endif

#ifndef _INC_L3L3L2CFGAIRLINKREQ
#include "L3L3L2CfgAirLinkReq.h"     
#endif

#ifndef _INC_L3OAMCFGBILLINGDATAREQ
#include "L3OamCfgBillingDataReq.h"    
#endif

#ifndef _INC_L3OAMCFGBTSNEIBLISTREQ
#include "L3OamCfgBtsNeibListReq.h"    
#endif

#ifndef _INC_L3OAMCFGBTSRSTREQ
#include "L3OamCfgBtsRstReq.h"         
#endif

#ifndef _INC_L3OAMCFGCALGENDATAREQ
#include "L3OamCfgCalGenDataReq.h"   
#endif

#ifndef _INC_L3OAMCFGCALDATAREQ
#include "L3OamCfgCalDataReq.h"   
#endif

#ifndef _INC_L3OAMCFGCALRSTGENNOTIFY
#include "L3OamCfgCalRstGenNotify.h"   
#endif

#ifndef _INC_L3OAMCFGCALRSTNOTIFY
#include "L3OamCfgCalRstNotify.h"      
#endif


#ifndef _INC_L3OAMCFGCFGCALREQ
#include "L3OamCfgCfgCalReq.h"      
#endif

#ifndef _INC_L3OAMCFGCALACTIONREQ
#include "L3OamCfgCalActionReq.h"      
#endif

#ifndef _INC_L3OAMCFGCALIBRATIONTIMER
#include "L3OamCfgCalibrationTimer.h"
#endif

#ifndef _INC_L3OAMCFGCPESERVICEREQ
#include "L3OamCfgCpeServiceReq.h"     
#endif

#ifndef _INC_L3OAMCFGDATASERVICEREQ
#include "L3OamCfgDataServiceReq.h"    
#endif

#ifndef _INC_L3OAMCFGGPSDATAREQ
#include "L3OamCfgGpsDataReq.h"        
#endif

#ifndef _INC_L3OAMCFGL1GENDATAREQ
#include "L3OamCfgL1GenDataReq.h"      
#endif

#ifndef _INC_L3OAMCFGPERFLOGREQ
#include "L3OamCfgPerfLogReq.h"        
#endif

#ifndef _INC_L3OAMCFGRMPOLICYREQ
#include "L3OamCfgRMPolicy.h"          
#endif

#ifndef _INC_L3OAMCFGTEMPALARMREQ
#include "L3OamCfgTempAlarmReq.h"      
#endif

#ifndef _INC_L3OAMCFGTOSREQ
#include "L3OamCfgTosReq.h"            
#endif


#ifndef _INC_TIMEOUTNOTIFY
#include "TimeOutNotify.h"
#endif

#ifndef _INC_L3OAMCFGRFREQ
#include "L3OamCfgRfReq.h"    
#endif

#ifndef _INC_L3OOModuleInitNOTIFY
#include "L3OOModuleInitNotify.h"
#endif



#ifndef _INC_L3OAMCFGINITFAILNOTIFY
#include "L3OamCfgInitFailNotify.h"     
#endif

#ifndef _INC_L3OAMCFGARPDATASERVICEREQ
#include "L3OamCfgArpDataServiceReq.h" 
#endif

#ifndef _INC_L3OAMCFGDMDATASERVICEREQ
#include "L3OamCfgDmDataServiceReq.h" 
#endif

#ifndef _INC_L3OAMCFGEBDATASERVICEREQ
#include "L3OamCfgEbDataServiceReq.h" 
#endif

#ifndef _INC_L3OAMCFGSNOOPDATASERVICEREQ
#include "L3OamCfgSnoopDataServiceReq.h" 
#endif


#ifndef _INC_L3OAMBTSRSTNOTIFY
#include "L3OamBtsRstNotify.h"
#endif

#ifndef _INC_L3OAMCFGACLREQ
#include "L3OamCfgACLReq.h"    
#endif

#ifndef _INC_L3OAMCFGDELACLREQ
#include "L3OamCfgDelACLReq.h"
#endif

#ifndef _INC_L3OAMCFGDELACLREQ
#include "L3OamCfgDelACLReq.h"
#endif

#ifndef _INC_L3OAMCFGSFIDREQ
#include "L3OamCfgSFIDReq.h"    
#endif

#ifndef _INC_L3OAMCFGNEIGHBOURBTSLOADINFOREQ
#include "L3OamCfgNeighbourBtsLoadInfo.h"
#endif
#ifndef _INC_L3OAMCFGBTSREPEATERREQ
#include "L3OamCfgBtsRepeaterReq.h"
#endif

#ifdef NUCLEAR_CODE
#define NUCLEAR_PRJ_BC_TIMER_MSGID  (0x2000)
#define NUCLEAR_PRJ_BC_TIMER_LENGTH (10*1000)
#endif
//#ifndef WBBU_CODE
struct T_NVRAMBAKINFO
{
    bool hasBakFile1;
    bool hasBakFile2;
    bool cfgDataChg;//配置数据是否改变，如果已经同步到备份文件2，则清为false
    bool bakFileAllBad;
    int bootUseFile; //目前启动使用的文件，0-nvram, 1-bakfile1, 2-bakfile2
    CTimer *pBakDataTimer;//每2个小时，如果配置数据发生更改，则同步到文件2
    UINT8 *pBakFile1Data;//开机时如果文件1写入内存，如果校验和不通过，则释放空间
    UINT8 *pBakFile2Data;//如果文件1不正确，则将文件2写入内存，如果校验和不通过，则释放空间
    UINT8 bakFile[200000];
};
struct T_NVRAMCRCINFO
{
    UINT32 crcFlag;
    UINT32 crcResult;
};
//#endif
class CTaskCfg : public CBizTask
{
public:
    CTaskCfg();
    enum OAM_CFG_STATUS
    {
        OAM_CFG_IDLE = 0,
        OAM_CFG_INIT_FROM_NVRAM,  //boot from bts
        OAM_CFG_INIT_FROM_EMS,
        OAM_CFG_INIT_FAIL,
        OAM_CFG_NORMAL	 
    };
    bool CM_L2AirLinkCfg(); //by xiaoweifang.
    bool CM_L2AirLinkCfgRandomBCH(); //by xiaoweifang.
    static CTaskCfg* GetInstance();    
    static int l3oambspNvRamWrite(char *, char *, int);
   // #ifndef WBBU_CODE
    static int l3oambspNvRamUpdateinCF(char *, int);  
    static UINT32  CalcNvramCheckSum(UINT8 *pData);   
    
    void NvramDataCheckandInitInfo();
	 #ifndef WBBU_CODE
    static bool CheckNvramCfBakFile(char *filename, int fileNo);
    bool getNvramBakData();
    #endif
private:
    static CTaskCfg *m_Instance;    
    static SEM_ID    s_semWriteNvram;
private:
    bool CM_BtsDataServiceCfgReq(CMessage &rMsg); //M_EMS_BTS_DATA_SERVICE_CFG_REQ
    bool CM_BtsDataServiceCfgRsp(CMessage &rMsg); //M_BTS_EMS_DATA_SERVICE_CFG_RSP
    bool CM_BtsResetReq(CMessage &rMsg);          //M_EMS_BTS_BTS_RESET_REQ
    bool CM_SendBtsResetMsg(UINT16);
    bool CM_CarrierDataCfgReq(CMessage &rMsg);    //M_EMS_BTS_CARRIER_DATA_CFG_REQ
    bool CM_CalibResultGenNotify(CMessage &rMsg); //M_BTS_EMS_CAL_GENDATA_NOTIFY
    bool CM_CalibResultNotify(CMessage &rMsg);    //M_BTS_EMS_CAL_DATA_NOTIFY
    bool getFileName(UINT8 *);
    UINT8* getFullPathFileName(UINT8 *, UINT8 *);
    bool Write2FileAndNotifyToFTP(char*);
    bool NotifyToFTP(UINT8 *);
    bool CM_CalibrationSetReq(CMessage &rMsg);    //M_EMS_BTS_CALIBRAT_CFG_DATA_REQ
    bool CM_CalibrationTimer(CMessage &rMsg);     //周期校准定时器消息处理函数
    bool CM_CommonReqHandle(CMessage &, TID, UINT16, UINT16);
    bool CM_CommonReqNoRspHandle(CMessage&, UINT16, SINT8*, UINT16, UINT16 DataOff = SIZEOF_TRANSID);
    bool CM_CommonRspHandle(CMessage&, UINT16, SINT8*, UINT16, UINT16 DataOff = SIZEOF_TRANSID);
////bool CM_CommonRspNoSaveHandle(CMessage& rMsg, UINT16 MsgId);
////bool CM_SendAlmNotifyToOam (UINT8, UINT16, UINT16, UINT16, UINT16, UINT8, SINT8*);
    bool CM_InitSendCfgReq(CMessage &);
	bool CM_SendBtsWorkingMsg();
    bool CM_SendCalibrationActMsg();
	bool CM_CarrierDataCfgRsp(CMessage& rMsg);
	bool CM_InitAUXData();
	bool CM_InitFEPData();
    bool CM_InitL2Data();
	bool CM_SendNotifyMsg( TID tid, UINT16 msgid);
    CTimer* CM_Createtimer(UINT16 MsgId, bool IsPeriod, UINT32 TimerPeriod);
    #if  0  
    void CM_UNITTEST_L2SendRsp(TID, UINT16, UINT16);
////void CM_UNITTEST_L3RspEms(UINT16 MsgId, UINT16 TransId);
    #endif
    void  CM_PostCommonRsp(TID tid, UINT16 transid, UINT16 msgid, UINT16 result);
    void  CM_GetMsgHandle(SINT8* pdata, UINT32 msglen, UINT16 rspmsgid, UINT16 transid);
    //bool CM_BtsNeighborBtsLoadInfoCfgReq(CMessage &rMsg); //M_EMS_BTS_NEIGHBOUR_BTS_LOADINFO_CFG_REQ
    //bool CM_BtsNeighborBtsLoadInfoGetReq(CMessage &rMsg); //M_EMS_BTS_NEIGHBOUR_BTS_LOADINFO_GET_REQ
    bool CM_BtsNeighborListHandoffCfgReq(CMessage &rMsg); //M_EMS_BTS_NEIGHBOR_LIST_HANDOFF_CFG_REQ
    bool CM_BtsNeighborListHandoffGetReq(CMessage &rMsg); //M_EMS_BTS_NEIGHBOR_LIST_HANDOFF_GET_REQ
    bool CM_BtsNeighborListCommonCfgReq(CMessage &rMsg); //M_EMS_BTS_NEIGHBOR_LIST_COMMON_CFG_REQ
    bool CM_BtsRepeaterCfgReq(CMessage &rMsg);
    bool CM_BtsRepeaterGetReq(CMessage &rMsg);
    void CM_BtsTelnetUserCfgReq(CMessage &rMsg);
    bool CM_BtsTelnetUserGetReq(CMessage &rMsg);


    bool CM_L2BillDataAndDiagUserCfg();
	bool CM_CreateCalibrationTimer();
    void CM_HandoverParaCfg(CMessage& rMsg);
    void CM_HandoverParaGet(CMessage&rMsg);
     void CM_IfPermitUseCfg(CMessage& rMsg);
    void CM_IfPermitUseGet(CMessage&rMsg);
	void CM_CdrParaCfgPro(CMessage& rMsg);
	void CM_CdrParaGet(CMessage& rMsg);
	void CM_STimeParaCfgPro(CMessage& rMsg);
	void CM_StimeParaGet(CMessage& rMsg);
	void CM_RFOPParaCfgPro(CMessage& rMsg);//网络开关RF配置
	void CM_RFOPParaGet(CMessage& rMsg);//网络开关RF查询
    void CM_sendMsgToRpc();

public:  //FIXME -- remove public when init from EMS
	#ifdef M_TGT_WANIF
	bool CM_ConfigWanCPE();
    bool CM_ConfigWanCpeFromShell(UINT32 eid,UINT32 bakeid );
	bool CM_WCPE_Cfg(CMessage& rMsg);
	void CM_WCPE_Get(CMessage& rMsg);
       bool CM_RCPE_Cfg(CMessage& rMsg);
	void CM_RCPE_Get(CMessage& rMsg);
	#endif
	bool CM_VacPrefScg();
	#ifdef WBBU_CODE
   void CM_SendPerfCfg();//将ftp 配置数据发送竝m任务?
      unsigned char     m_send_cal_alarm ;
	#endif

public:
	void  testSagBackup();
	void  testSagBackupShow();
private:
    bool Initialize();
    bool ProcessMessage(CMessage &Msg);
    TID  GetEntityId()  const; 

    #define CFG_MAX_BLOCKED_TIME_IN_10ms_TICK (500)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return CFG_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    void InitFirstDataFromNvram();
    void InitOtherDataFromNvram(CMessage &rMsg);

	void neighborListRefresh();
private:
    void SetSysStatus(OAM_CFG_STATUS Status );  
public:
    OAM_CFG_STATUS GetSysStatus();    
private:
    OAM_CFG_STATUS  m_SysStatus;               // OAM_CFG_SATUS
    CTimer          *pCalibTimer;              // 周期校准定时器
    CTimer *pGPSCfgDataErrorAlmTimer;
    CTimer  * pHLRStimer;
    bool    bGPSCfgDataErrorAlm;
#define CALIBRATION_DATA_NUM 32
    struct T_CaliNotifyDataRcd
    {
        bool    IsDataCorrect;        //  消息M_BTS_EMS_CAL_GENDATA_NOTIFY中没有错误指示位
        UINT8   ucCalTrigger;         //  1 -- 自动校准，还是 0--手动校准
        UINT8   EntryCnt;             // 失败时最多做三次calibration
        T_CaliResultGenNotifyEle GenResultNotify;    // Calibration Result General Notification（BTS） 
        T_CaliResultNotifyEle    NotifyData[CALIBRATION_DATA_NUM];//Calibration Result CalData Notification（BTS）
    };
    T_CaliNotifyDataRcd *pCalRstDataRcd;    //校准结果不管正确还是失败,都保存在这个内存变量，而且将该结果发给ems. 如果校准正确,则存入nvram.
    T_DataServiceCfgEle *pDataServiceCfg;
    T_AirLinkCfgEle     *pDataAirLinkCfg;
    T_AirLinkCfgEle     *pDataAirLinkCfgRandom;
    T_L1GenCfgEle       m_gL1Cfg;    //全局,定义在任务内

	bool    m_bSavePowerModel;
	bool    m_bSavePwrInitL2Req; 
	CTimer* m_pFanTimer;
	UINT16   m_ucFanCnt;
	BOOL    m_bSavePwrSupport;
    void CM_SavePwrFanCtrl();
	bool CM_SavePwrCfg();
	void CM_SavePwrCpuCtrl(CMessage &rMsg);
	bool CM_SavePwrExitModel();
	bool CM_SavePwrSupport();
	public:
	bool CM_Qam64();
	bool CM_LoadBlnInfo();
	int CM_WrtieNvRam4OtherTask(CMessage& msg);
#ifdef PAYLOAD_BALANCE_2ND
 	T_PayloadBalanceCfg2nd m_stPldBlnCfg;
#else
 	T_PayloadBalanceCfg m_stPldBlnCfg;
#endif
	bool m_bPldBlnCfgOK;
//#ifdef LJF_WCPE_BOOT
	bool m_bWcpeBootModel;
	bool m_bWcpeBootModelOK;
	bool m_bBootFromNV;
	bool m_bNVBootFail;
//#ifdef LJF_WAKEUPCONFIG
void CM_setWakeupConf( UINT8 usSwitch, UINT32 ulPeriod );
	bool CM_SendComMsg( TID tid, UINT16 usMsgID, UINT8* pd, UINT16 usLen , UINT32 eid=0);
	#ifdef NUCLEAR_CODE
	public:
	void CM_NuclearSendLimitData( bool bBC, UINT32 eid=0 );
	void nuprint(UINT8 uc);
	private:
	//bool CM_SendComMsg( TID tid, UINT16 usMsgID, UINT8* pd, UINT16 usLen , UINT32 eid=0);
	CTimer* InitTimer( bool bPeriod, UINT16 usMsgID, UINT16 usPeriod );
	UINT8* m_pucLimitedNeiBts;
	UINT16 m_usLimitedNeiBtsLen;
	CTimer* m_ptmLimitedBC;
	UINT8  m_ucLimitFlag;
	//#else
	//bool CM_SendComMsg( TID tid, UINT16 usMsgID, UINT8* pd, UINT16 usLen );
	#endif
#ifdef RCPE_SWITCH
public:
	TrunkMRCpeRcd m_stTrnukMRCpe;
	bool CM_RcpeSwitchCfg( CMessage& rMsg ) ;
	bool CM_RcpeClear() ;
    bool CM_PrintMRcpeInfo() ;
#endif
    void sendGrpLmtPara();
};
#endif 
