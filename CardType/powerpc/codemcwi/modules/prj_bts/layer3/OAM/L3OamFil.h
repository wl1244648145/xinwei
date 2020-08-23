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

#pragma warning (disable : 4786)
#ifndef _INC_L3OAMFILE
#define _INC_L3OAMFILE

#ifdef __WIN32_SIM__
#include <windows.h>
#include <list>
using namespace std;
#else
#include <list>
#include <stdio.h>
#endif

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_L3OAMUPDATECPESWRARENOTIFY
#include "L3OamUpdateCpeSWRateNotify.h"
#endif

#ifndef _INC_L3OAMUPDATECPESWRESULTNOTIFY        
#include "L3OamUpdateCpeSWResultNotify.h"
#endif

#ifndef _INC_L3OAMUPDATECPESWREQ
#include "L3OamUpdateCpeSWReq.h"
#endif

#ifndef _INC_L3OAMCPESWDLOADREQ
#include "L3OamCPESWDLoadReq.h"
#endif

#ifndef _INC_L3OAMBTSSWUPDATEREQ
#include "L3OamBtsSWUpdateReq.h"
#endif

#ifndef _INC_L3OAMBTSSWDLOADREQ
#include "L3OamBtsSWDLoadReq.h"
#endif

#ifndef _INC_L3OAMBCCPESWRSP
#include "L3OamBCCPESWRsp.h"
#endif

#ifndef _INC_L3OAMBCCPESWREQ
#include "L3OamBCCPESWReq.h"
#endif

#ifndef _INC_L3OAMBCCPESWRATENOTIFY
#include "L3OamBCCPESWRateNotify.h"
#endif

#ifndef _INC_TIMEOUTNOTIFY
#include "TimeOutNotify.h"
#endif

#ifndef   _INC_L3L3CPESWDLREQ
#include "L3L3CpeSWDLReq.h"
#endif

#ifndef   _INC_L3L3CPESWDLPACKREQ
#include "L3L3CpeSWDLPackReq.h"
#endif

#ifndef _INC_L3L3CPESWDLPACKRSP
#include "L3L3CpeSWDLPackRsp.h"
#endif

#ifndef _INC_L3L3CPESWDLRESULTNOTIFY
#include "L3L3CpeSWDLResultNotify.h"
#endif

#ifndef _INC_L3L3BROADCASTUTSWTIMER
#include "L3L3BroadcastUTSWTimer.h"
#endif

#ifndef _INC_L3OAMUNICASTUTSWREQFAIL
#include "L3OamUnicastUTSWReqFail.h"
#endif

#ifndef _INC_L3OAMBTSRSTNOTIFY
#include "L3OamBtsRstNotify.h"
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

#ifndef _INC_L3OAMCOMMONRSPFROMCPE
#include "L3OamCommonRspFromCpe.h"
#endif

extern "C"
{
UINT32 bspGetActiveVersion();
UINT32 bspGetStandbyVersion();
}


#define CALFTPCLIENT_TIMER 0xcccc
#define DLFTPCLIENT_TIMER 0xddcc
#define UPDATE_TIMER 0xDDDD //jy 081114
#define FTPPM_TIMER 0xccdd //jy001121
#define PMFAIL_TIMER 0xcdcd
#define FTPCFAIL_TIMER 0xdcdc
#define FTPCLIENT_SUCCESS_FLAG 0xEEEE
#define DEL_INFO_TIMER 0xbbbb

//#define _INC_SETTYPE_
//#ifdef _INC_SETTYPE_
#define SETTYPE_WORKING    0xC0AB
#define SETTYPE_COMIPAPP  0xCDAB
//#endif
#pragma pack(1)
typedef struct
{
    UINT16 TransId;    
    UINT32 EID;
    UINT16 EmsTransId;
    UINT8 UT_Type;
    UINT8 File_name_length;
    UINT8 File_name[FILE_NAME_LEN];
    
}T_UtDlGuageNotify;

typedef struct
{
    UINT16 Seq;
    UINT8 guage;
    UINT32 newBtsId;
}T_UtDlGuageNotify_tail;

typedef struct
{
    UINT16 TransId;    
    UINT32 EID;
    UINT16 flag;
    UINT32 BtsId;
}T_UtMoveaway;
typedef struct
{
    UINT16 TransId;    
    UINT32 EID;
    UINT16 EmsTransId; 
    UINT8 File_name_length;
    UINT8 File_name[FILE_NAME_LEN];
}T_UtRetranHead;
typedef struct
{
    UINT16 Seq;
    UINT8 guage;
    UINT8 flag;
}T_UtRetranTail;

#define M_FILE_READY_FLAG (0x6789)
typedef struct
{
    UINT8 userFileName[32];
    UINT32 userFileLen;
    UINT8 *fileData;
    UINT16 flag;
}T_UserFileHead;
typedef struct
{
    T_UserFileHead User_ACL_List;
    T_UserFileHead User_Voice_List;
    T_UserFileHead Trunk_Group_list;
    T_UserFileHead Trunk_Group_User_List;
}T_UserFileRecord;
#ifdef WBBU_CODE

typedef struct
{
    UINT16 TransId;    
    UINT16  LoadType;//01 -mcu ;02-fpga
}T_WrruLoad;

#endif
extern T_UserFileRecord userFileRecord;

#pragma pack()

//为支持多播升级，对每次文件读取操作后的指针保存到CpeUpgradeList的对应记录。
//收到CPE应答后通过列表CpeUpgradeList检索到正确的文件指针【pFile】。
class CTaskFileMag : public CBizTask
{
public:
    CTaskFileMag();
    typedef struct
    {
        UINT16  EmsTransId;
        UINT32  CPEID;
        UINT16  DLReqSeqNum;
        UINT8   Progress;
        UINT32  FileSize;
        UINT16  TotalPackNum;
        UINT16  PackSize;
        UINT16  CurPackNum;
        char    FileDirName[FILE_DIRECTORY_LEN + FILE_NAME_LEN];
        FILE *  FileCurPtr;      //记录文件指针的位置     
        bool    bZFlag;//是否为Z模块应用软件下载
        UINT32  UID;
        UINT32 RetranNum;
        CTimer *delInfoCTimer;
        UINT32   ulZBoxPID;
    }T_CpeUpgradeRecord;
    static CTaskFileMag* GetInstance();    
private:
    static CTaskFileMag * m_Instance;    
private:
    enum
    {
        OAM_FILE_IDLE = 0,
        OAM_FILE_DL_SW,
        OAM_FILE_UPGRADE_CPE_SW,
        OAM_FILE_BC_UPGRADE_CPE_SW,
    };

    #define    USER_ROOT_DIR       "/ata0a/"
    #define    SM_BTSA_BOOT_DIR    "/ata0a/btsA/"     // BTS软件目录, 启动平面 0
    #define    SM_BTSB_BOOT_DIR    "/ata0a/btsB/"     // BTS软件目录, 启动平面 1
    #define    SM_CPE_BOOT_DIR     "/ata0a/cpe/"      // CPE软件目录
    //#define    SM_BTS_TEMP_DIR     "/ata0a/temp/"     // 临时目录
#ifndef WBBU_CODE
    #define    SM_BTS_TEMP_DIR      "/RAMD/temp/"
#else
    #define    SM_BTS_TEMP_DIR      "/RAMD:0/temp/"

#endif
    #define    SM_BTS_WORK_DIR     "/ata0a/work/"     // 工作目录
    #define    SM_CPE_WORK_DIR     "/ata0a/cpe/"
    #define    SM_BTS_ACTVER_FILE      "/ata0a/work/btsactive.txt"
    #define    SM_BTS_STANDBY_FILE     "/ata0a/work/btsstandby.txt"
    
#if 0
    #define    USER_ROOT_DIR       "D:\\filetest\\ata0a\\"
    #define    SM_BTSA_BOOT_DIR    "D:\\filetest\\ata0a\\btsA\\"     // BTS软件目录, 启动平面 0
    #define    SM_BTSB_BOOT_DIR    "D:\\filetest\\ata0a\\btsB\\"     // BTS软件目录, 启动平面 1
    #define    SM_CPE_BOOT_DIR     "D:\\filetest\\ata0a\\cpe\\"      // CPE软件目录
    #define    SM_BTS_TEMP_DIR     "D:\\filetest\\ata0a\\temp\\"     // 临时目录
    #define    SM_BTS_WORK_DIR     "D:\\filetest\\ata0a\\work\\"     // 工作目录
    #define    SM_CPE_WORK_DIR     "D:\\filetest\\ata0a\\cpe\\"
/////////////////////////////////////////////////////////////////////////
    #define    USER_ROOT_DIR       "/RAMD/"
    #define    SM_BACKUP_SW_DIR    "/RAMD/BACKUP/"       /* 备份软件目录 */
    #define    SM_CPE_BOOT_DIR     "/RAMD/cpe/"          /* 工作目录 */
    #define    SM_BTSA_BOOT_DIR    "/RAMD/btsA/"     // BTS软件目录, 启动平面 0
    #define    SM_BTSB_BOOT_DIR    "/RAMD/btsB/"     // BTS软件目录, 启动平面 1
    #define    SM_BTS_TEMP_DIR     "/RAMD/temp/"     // 临时目录
    #define    SM_BTS_WORK_DIR     "/RAMD/work/"     // 工作目录
    #define    SM_CPE_WORK_DIR     "/RAMD/cpe/"
    #define    SM_BTS_ACTVER_FILE  "/RAMD/work/btsactive.txt"
    #define    SM_BTS_STANDBY_FILE "/RAMD/work/btsstandby.txt"
#endif




    #define    UT_HWTYPE_CNT           /*13*/255
    #define    MAX_REQ_SEND_CNT        10
    //#ifdef _LJFDEBUG
    #define    BC_UT_DATAPACK_PERIOD   1500
    //#else
    //#define    BC_UT_DATAPACK_PERIOD   500
    //#endif

    enum
    {   SWDLTYPE_UNICAST   = 0,   
        SWDLTYPE_BROADCAST = 1,   
        SWDLTYPE_BOOTLOADER = 2        
    };

    typedef struct
    {
        char    chFileName[FILE_DIRECTORY_LEN + FILE_NAME_LEN];
        UINT16  EmsTransId;
        UINT8   SendReqCnt;      //标记是否已经将代码下载请求消息下发过N(=3)遍,
                                 //若是则开始下载数据部分
        UINT16  DLReqSeqNum;
        UINT32  FileSize;
        UINT32  SWVer;
        UINT16  InterType;
        
        UINT32  TotalPackNum;
        UINT32  PackSize;
        UINT32  CurPackNum;
        FILE *  FileCurPtr;      //记录文件指针的位置     
        UINT8   Progress;        //
        UINT8   CurRetryTimes;   //当前是第几次下载  
                                 // 0 -- 未曾进行下载,  1 -- 第一遍下载
                                 // 2  -- 第二遍下载
        UINT8   RetryTimes;      //总共需要下载的次数
        CTimer *pBcSwTimer;
    }T_BCUGRcd;

	typedef struct {
		UINT16   version_major_minor;
		UINT16   version_maint;             
		UINT16   year;
		UINT16   month_day;
		UINT16   hour_minute;
		UINT16   checksum;
		UINT32   BootLoaderSize;   
		UINT32   ArmSize;
		UINT32   DspSize;
		UINT32   FpgaSize;
		UINT32   reserve1;
		UINT32   reserve2;
		UINT32   reserve3;
		UINT32   reserve4;
		UINT16   Reserved[232];
        UINT16   SetType;
		UINT16   HardwareType;
	}CPE_BIN_HEADER;
public: 
    void   showDL();
    void setFtpUsingFlag(UINT32 tid, bool flag);
    void StartPMCreateTimer();
    void StartFtpCCreateTimer();
    void setTaskNull(TID tid);//删除任务jiaying20100901
private:
    bool   SM_FileLoadReq (CMessage & rMsg);
	bool   SM_FileLoadReqNew (CMessage& rMsg);
    bool   SM_FileLoadResultNotify(CL3OamCommonRsp& rMsg);    //M_OAM_DL_FILE_RESULT_NOTIFY处理ftp client 返回的文件下载通知

    bool   SM_BtsUpgradeReq(CBtsSWUpdateReq &rMsg);       //M_EMS_BTS_UPGRADE_BTS_SW_REQ
    bool   SM_CpeUpgradeReq(CMessage& rMsg);       //M_EMS_BTS_UPGRADE_UT_SW_REQ
    bool   SM_CpeUpgradeReqNew(CMessage& rMsg);       //M_EMS_BTS_UPGRADE_UT_SW_REQ_NEW
    bool   DealCpeUpdateReq( CUpdateCpeSWReq& rMsg, T_CpeUpgradeRecord& CpeInfo );
	bool   DealCpeUpdateReqNew( CUpdateCpeSWReqNew& rMsg, T_CpeUpgradeRecord& CpeInfo );
//    bool   DealZUpdateReq( CUpdateZSWReq& rMsg, T_CpeUpgradeRecord& CpeInfo );
    bool   SM_CpeZUpgradeRsp(CL3OamCommonRspFromCpe &rMsg, UINT16 MsgId);       //M_BTS_EMS_UPGRADE_UT_SW_RSP
#ifndef WBBU_CODE
    bool   SendZPack( CMessage& rFailMsg, list<CTaskFileMag :: T_CpeUpgradeRecord> :: iterator pRecord, SINT8* Buff, UINT16 usLen, UINT16 usTransId );
    bool   SendCpePack( CMessage& rFailMsg, list<CTaskFileMag :: T_CpeUpgradeRecord> :: iterator pRecord, SINT8* Buff, UINT16 usLen, UINT16 usTransId );
#else

    bool   SendZPack( CMessage& rFailMsg, std::list<CTaskFileMag :: T_CpeUpgradeRecord> :: iterator pRecord, SINT8* Buff, UINT16 usLen, UINT16 usTransId );
    bool   SendCpePack( CMessage& rFailMsg, std::list<CTaskFileMag :: T_CpeUpgradeRecord> :: iterator pRecord, SINT8* Buff, UINT16 usLen, UINT16 usTransId );
#endif
    bool   SM_CpeUpgradeNotify(CUpdateUTSWResultNotify &rMsg);
    bool   SM_CpeZUpgradePackRsp(CL3CpeSWDLPackRsp &rMsg);   //M_OAM_UPGRADE_UT_SW_PACK_RSP
    bool   SM_CpeUpgradeReqFail(CL3OamUnicastUTSWReqFail&);
    bool   SM_SendUNICASTSWDLReqToCpe(CMessage &ReqMsg, UINT32 FileSize);
	bool   SM_SendUNICASTSWDLReqToCpeNew(CMessage &ReqMsg, UINT32 FileSize);
    void   SM_PostCommonRsp(TID tid, UINT16 transid, UINT16 msgid, UINT16 result);

    bool   SM_SendZSWDLReqToCpeNew(CMessage &ReqMsg, UINT32 FileLen);
    bool   SM_getUtUpgradeStatus(CMessage& rMsg);  
    bool   SM_CpeUpgradeRetran(CMessage& rMsg); 
private:
#ifndef WBBU_CODE
    list <T_CpeUpgradeRecord> m_CpeUpgradeList;
#else
    std::list <T_CpeUpgradeRecord> m_CpeUpgradeList;
#endif
    T_BCUGRcd  m_BCRcdA[UT_HWTYPE_CNT];  //列表的前三项固定用来保存广播升级【0 - Handset  1 - desktop  2 - PCMCIA】信息。
private:
    void   SetSysStatus(UINT32 Status );
    UINT32 GetSysStatus();   
 
    UINT16 GetSysDLReqSeqNum();
    void   AddSysDLReqSeqNum();
    UINT16 AddCpeUpgradeRecord(T_CpeUpgradeRecord &);    
    bool   DelCpeUpgradeRrcord( UINT32 Cpeid); 
#ifndef WBBU_CODE
    list<T_CpeUpgradeRecord> :: iterator FindCpeUpgradeRecord(UINT32 );  //此处CpeInfo是输出参数 
#else
    std::list<T_CpeUpgradeRecord> :: iterator FindCpeUpgradeRecord(UINT32 );  //此处CpeInfo是输出参数 
#endif
    SINT32 GetFileLength(FILE*);
    UINT8  SM_GetBCArrayIndex(UINT8 hwtype);
private:
    bool   Initialize();
    bool   ProcessMessage(CMessage &Msg);
    TID    GetEntityId() const;

    #define FM_MAX_BLOCKED_TIME_IN_10ms_TICK (500)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return FM_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    bool getHardwareInfo(SINT8* filename, UINT8 len, UINT8* modelType, UINT32* VerInfo, UINT8* isComipApp);
	bool getModelTypeByFilename(SINT8* filename, UINT8* modelType);
    
        
private:
    UINT32 m_SysStatus;     // 0 -- 系统空闲     1 -- 系统忙，处于软件下载状态
    UINT16 m_EMSTransID;
    enum{MAX_UPDATE_CPE_NUM = 20};  
    
    UINT32 m_CurUpdateCpeNum;
    UINT16 m_SysDLReqSeqNum;   
//    BOOL m_bBCReqFlag;    //true 发送 BC_Req | false 发送起始数据包

    CTimer *m_pdlFtpCTimer;
    CTimer *m_pcalFtpCTimer;
    CTimer *m_pUpdateCTimer;
    CTimer *m_pFtpPMTimer;//启动对pm任务的监测jy081121
    CTimer *m_pPMTaskTimer;//启动定时器,如果连续3次pm任务都创建失败就复位基站
    CTimer *m_pFtpCTaskTimer;//启动定时器,如果连续3次ftpc任务都创建失败就复位基站
    UINT16 m_usDlType;
    UINT16 m_usTransId;
    int m_iFtpCTid;
    int m_iFtpPMTid;
    UINT32 m_ftpPmTimeoutNum;
    UINT32 m_calFtpCTimeoutNum;
    UINT32 m_dlFtpCTimeoutNum;
    UINT32 m_PmCreateFailNum;
    UINT32 m_ftpCCreateFailNum;
    bool m_ftpPmFlag;
    bool m_dlFtpCFlag;
    bool m_calFtpCFlag;
    void SM_calFtpClientTimeout();
    void SM_dlFtpClientTimeout();
    void StartCalFtpCTimer();
    void StartDlFtpCTimer();
    void SM_UpdateTimeout(CMessage &rMsg);
    void StartUpdateCTimer(UINT32 eid);
    void SM_FtpPMTimeout();//启动对pm任务的监测jy081121
    void StartFtpPMTimer();    
    void SM_PMCreateTimeout();
    void SM_FtpCCreateTimeout();
    void SM_delCpeUpdateInfo(CMessage &rMsg);
};
#endif
