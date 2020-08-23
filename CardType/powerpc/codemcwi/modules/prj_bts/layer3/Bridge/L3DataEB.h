/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataEB.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   06/06/07   xin wang      ���� MAC Filter
 *   03/28/06   xiao weifang  FixIp�ĳɷ������Ե��û�
 *   07/28/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_BRIDGE_H__
#define __DATA_BRIDGE_H__

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
#include "LogArea.h"

#include "L3DataTypes.h"
#include "L3DataMacAddress.h"
#include "L3DataMessages.h"
#include "L3DataConfig.h"
#include "L3DataFTEntryExpire.h"
#include "L3DataFTAddEntry.h"
#include "L3DataFTDelEntry.h"
#include "L3DataMFTAddEntry.h"
#include "L3DataMFTDelEntry.h"
#include "L3DataMFTEntryExpire.h"
#include "L3DataTosSFID.h"
#include "log.h"
#include "L3DataEBMeasure.h"
#include "L3DataEBErrCode.h"
#include "L3DataAssert.h"
#include "L3DataNotifyBTSPubIP.h"
#include "L3oamCfgACLReq.h"

//lijinan 20081208 �Ʒ�ϵͳ����
#include "sysBtsConfigData.h"

#ifdef BUFFER_EN
#include "L3DataEBBuffer.h"
#endif

//Ether Bridge�����������
#define M_TASK_BRIDGE_TASKNAME      "tEB"
#ifdef __WIN32_SIM__
#define M_TASK_BRIDGE_OPTION        (0x0008)
#define M_TASK_BRIDGE_MSGOPTION     (0x02)
#else
#define M_TASK_BRIDGE_OPTION        ( VX_FP_TASK )
#define M_TASK_BRIDGE_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#ifndef WBBU_CODE
#define M_TASK_BRIDGE_STACKSIZE     (20480)
#else
#define M_TASK_BRIDGE_STACKSIZE     (60000/*40960*/)
#endif
#define M_TASK_BRIDGE_MAXMSG        (30000)

//Clean Up�����������
#define M_TASK_CLEANUP_TASKNAME     "tCleanUp"
#define M_TASK_CLEANUP_PRIORITY     (190)
#ifdef __WIN32_SIM__
#define M_TASK_CLEANUP_OPTION       (0x0008)
#else
#define M_TASK_CLEANUP_OPTION       ( VX_FP_TASK )
#endif
#define M_TASK_CLEANUP_STACKSIZE    (20480)

/***************************************
 *M_CREATOR_DRV   : ���������payload
 *M_CREATOR_MYSELF: EB�Լ������payload.
 */
#define M_CREATOR_DRV               (0xFFFFFFFF)
//#define M_CREATOR_MYSELF            (0xEFEFEFEF)

/***************************************
 *M_COUNT_USERTYPE_ADD:�����û����ͼ���
 *M_COUNT_USERTYPE_DEL:�����û����ͼ���
 */
#define M_COUNT_USERTYPE_ADD        (false)
#define M_COUNT_USERTYPE_DEL        (true)


//���ش���
#define EB_FLOWCTRL_CNT             (100)
//ComMessage����ĳ���
#define M_MAX_LIST_SIZE             (60000)

#define EB_MAX_BLOCKED_TIME_IN_10ms_TICK  (100)
#define EB_CLEANUP_MAX_BLOCKED_TIME_IN_10ms_ICK (500)


//lijinan 20081205 �Ʒ�ϵͳ����
#include <dirent.h>
#ifndef WBBU_CODE
#define    CDR_FILE_DIR     "/RAMD/cdr/"
#else

#define    CDR_FILE_DIR     "/RAMD:0/cdr/"
#endif
#define    CDR_UPLOAD_DIR      "system/cdr/"
#define    CF_CDR_DIR 	"/ata0a/cdr/"
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#define FLAG_YES                     1
#define FLAG_NO                       0

#define M_CDR_NVRAM_INITIALIZED (0x20081218)
#define MSGID_BWINFO_DEL_REQ		(0x6206)
const UINT16 CDR_ABSTRACT_FILENAME_LEN = 100;
    enum FILE_TYPE
    {
        RAM_FILE = 0,
        CF_FILE
    };
    enum CDR_TYPE
    {
        From15MinTimer = 0,
        From1MinInterval,
        FromDelEid,
        FromSendCfToFtp,
        //#ifdef LJF_BILLING_VOICE
		FromVoiceInfo,
		FromMoveAway,
		FromMax
    };
     enum WRITE_FILE_TYPE
    {
        CDR_UP = 0,
        CDR_DOWN
    };
     enum STAT_TIME_TYPE
    {
        FromPKT = 0,
        FromTimer
    };
    typedef enum CB3000_MSG
    {
        WIFI_USER_ACC_REQ = 1,
        WIFI_USER_ACC_RSP,
        CB3000_BTS_HEART_REQ,
        BTS_CB3000_HEART_RSP,
	 WIFI_USER_LOG_OUT_REQ,
	 WIFI_USER_LOG_OUT_RSP,
	 RT_CHARGE_REQ,
	 RT_CHARGE_RSP,
	 RT_CHARGE_REPT_REQ,
	 RT_CHARGE_REPT_RSP,
	 RT_CHARGE_INFORM,
	 RT_CHARGE_INFORM_RSP,
	 RT_CHARGE_RPT_PERIOD,
	 RT_CHARGE_RPT_PERIOD_ACK,
	 CB3000_MSG_MAX
    }BTS_CB3000_MSG;
     typedef enum RT_RPT_MSG
    {
    	 RPT_PERIOD = 0,
        RPT_NO_FLOW,
	 RPT_MOVE_AWAY,	
	 RPT_MSG_MAX
    }RT_RPT_MSG_TYPE;
#if 1//def LJF_BILLING_VOICE
#define MAX_SAVE_CDR_NUM 48    
#else
#define MAX_SAVE_CDR_NUM 96    
#endif

#pragma pack (1)
//new stat
typedef struct
{
    UINT32 plusTimeHigh;//��Ʒ�ͳ�Ƶ�ƫ���Ҫ����
    UINT32 plusTimeLow;
    UINT32 timelongHigh;
    UINT32 timelongLow;
    UINT32 dataflowHigh;
    UINT32 dataflowLow;
}stBilling_newStat;
typedef enum
{
    STAT_PLUS = 0,
    STAT_DATA_FLOW,
    STAT_TIMELONG,
    STAT_MAX
}STAT_DATA;

//#ifdef LJF_BILLING_VOICE
typedef struct 
{
	UINT8 type;
	UINT8* pucAdd;
	UINT32 ulLen;
}stVoiceBilling;

/*
typedef struct 
{
	UINT32 Initialized;
	UINT32 LocalBtsId;
	UINT8 cdrOnOff;
	UINT8 cdrPeriod;
	UINT32 IP_CB3000;
	UINT16 intval;
}stCdrNVRAMhdr;*/

typedef struct 
{
	char name[48];//lijinan 20101215 30 ->48
}stCdrNameNVRAM;


typedef struct
{
    UCHAR  year[2];		
    UCHAR   month;
    UCHAR   day;
    UCHAR   hour;		
    UCHAR   minute;		
    UCHAR   second;		
}CdrTime;

typedef struct 
{
	UINT16  Len;    //������¼����
	UINT16  Type;   //����ļ��еļ�¼����
	UINT32  Id;     // CDR���б�ʶ
	UINT32      ulEid;	//PID
	UINT32      ulUid;	//UID
	UINT8       User_type;   // ��Ϊ�ƷѶ�����ƶ��û�����
	UINT32      Home_prov;  // �Ʒ��û��������ʡ��ʡ����
		UINT32      Roam_prov;		//�Ʒ��û���������ʡ��ʡ����
	UINT16	     Auth_type;	 //�û���֤ʹ�õķ�ʽ
      	CdrTime	stStartTime; //������ʼʱ��
	CdrTime	stStopTime; //���ӽ���ʱ��
	UINT32  Duration;   //�������ӵĳ���ʱ�����룩
	UINT32  Data_flowup[2];  //������������
	UINT32  Data_flowdn[2];  //������������
	UINT32  BS_ID;   // BSID
	UINT16  Cause_close;   //����ԭ��
	UINT8   bw[8];    //�����д���
	UINT32 wifi_UID;
	UINT32 wifi_IP;
	UINT8 MAC[M_MAC_ADDRLEN];
	UINT16 Id_little;
	UINT8 resv1[8];
 
}_CDR;
//#ifdef LJF_BILLING_VOICE
typedef struct 
{
	UINT8 CALLING[20];
	UINT8 DIALED[24];
	UINT8 CALLED[24];
	UINT8 CALLING_TY[12];
	UINT8 CALLED_TYP[12];
	UINT8 CALLING_DE[12];
	UINT8 CALLED_DEV[12];
	UINT8 CALL_TYPE[12];
	UINT8 START_DATE[8] ;
	UINT8 START_TIME[6] ;
	UINT8 ANSWER_DA[8] ;
	UINT8 ANSWER_TM[6] ;
	UINT8 END_DATE[8] ;
	UINT8 END_TIME[6] ;
	UINT8 DURATION_S[12];
	UINT8 DURATION_T[12];
	UINT8 CALL_STATU[10];
	UINT8 CALLING_LO[10];
	UINT8 CALLED_LOC[10];
}_VCDR;

typedef struct  _CDR_{
	
	_CDR cdr;

	//��һ���������ʱ��,��tickΪ��λ
	UINT32    lastPktTick;
	
	//�ϴ�дcdr tick  
	UINT32    laskCdrTick;

	//�Ƿ�Ƿ��flag
	UINT8 noPayflag;
	UINT8 intvlOneMin;
	
	T_TimeDate lastPtTime;

	UINT8 ucMacNum;
	UINT16 cdr_id_little;
	UINT32 wifi_eid_flag;
	UINT32 wifi_eid_sec;
	UINT16 req_remain_flag;
	UINT16 time_remain;
	UINT32 data_flow_remain;
	int flag_limit;
	int flag_final;
	int flag_time;
	int flag_traffic;
	int flag_over;
	
	}CDR;





#pragma pack ()

class CCDR
{
public:
    void InitFreeCDRList();
    void InsertFreeCDRList(UINT16 usIdx );
    UINT16 GetFreeCDRIdxFromList();
    bool BPtreeAdd(UINT32 &Eid, UINT16 usIdx);
    bool BPtreeAdd(CMac &, UINT16 ,UINT32);
    UINT16 BPtreeFindbyUid(UINT32 &Uid);
    void cdrMacAdd(UINT32 & uiEid);
    bool BPtreeDel(UINT32 &Eid);
    bool BPtreeDel(CMac &);
    bool BPtreeDelbyUid(UINT32 &Uid);
    UINT16 BPtreeFind(UINT32 &Eid);
    UINT16 BPtreeFind(CMac &);
    bool RemoveFile( char* chFileName );
    void StatTimeLen(STAT_TIME_TYPE type,UINT16 usIdx);
    void StatDataFlow(WRITE_FILE_TYPE type, UINT16 usIdx,UINT16 dataLen);
    void  GetFileTime( char * chVal );
    bool  MakeCdrFileInfo();
//#ifdef LJF_BILLING_VOICE
	bool WriteVoiceToFile( UINT8*pd, UINT16 usLen );  
	void sendVoiceMsgToWriteFileTask( CDR_TYPE type, UINT8*pd, UINT16 usLen );
	void billing(  UINT8 uc );  
    bool  WriteToFile(      _CDR* pstCdr,UINT32 index);
    bool  WriteToFile(char*);
    bool  WriteFileHead( );
    void  CdrDataFileUploadTimer();
    void bs_reboot_send_cdr();
    void deleteCdrInform(UINT16);
#ifdef WBBU_CODE
    bool SendCdrFile( char* chFileName,int flag );
#else
    bool SendCdrFile( char* chFileName);
#endif
    bool   DelEid(UINT32 &Eid,int flag);
    bool   DelWifiMac(CMac &);
    STATUS writeFileMainloop();
    void sendMsgToWriteFileTask(CDR_TYPE type,UINT16 index);
    void countDurationAndStoptime(T_TimeDate*pLastPktTime,
	CdrTime*pCdrStartTime,UINT16 &index,UINT16 dltSec);
    void showCdr(UINT32 ulEID);
   void restartTsk();
   bool writeFileToCF( char* chFileName );
   bool SendCFCdrFile( char* chFileName );
   bool RemoveCfFile( char* chFileName );
   void checkNvramCdrFile();
   void sendCfFileToFtp();
   UINT32 getWriteFileTaskId(){return writeFileTid;}; 
   void setWriteFileTaskId(UINT32 tid){writeFileTid = tid;};
   void spyCdrMacTbl();
   void func_moveAway_Rt_cdr(UINT16);
   void delWifiUser();
   void func_heart_pro(char *);
    //---------------------------------------------
    //����ָ���±��CDR
    //---------------------------------------------
    CDR* GetCDREntryByIdx(UINT16 usIdx)
        {
        if ( !( usIdx < M_MAX_UT_PER_BTS ) )
            {
            return NULL;
            }
        return &( cdrTbl [ usIdx ] );
        }
    char* GetAbstractFile( char* chFileName, char* chFullName )
    {
        memset( chFullName, 0, CDR_ABSTRACT_FILENAME_LEN );
        strcpy( chFullName, CDR_FILE_DIR );
        //strcat( chFullName, "/" );
        strcat( chFullName, chFileName );
        return chFullName;
    }
    char* GetCFAbstractFile( char* chFileName, char* chFullName )
    {
        memset( chFullName, 0, CDR_ABSTRACT_FILENAME_LEN );
        strcpy( chFullName, CF_CDR_DIR );
        //strcat( chFullName, "/" );
        strcat( chFullName, chFileName );
        return chFullName;
    }

	bool init();
	SEM_ID GetCdrSemId() const {return cdrSemId;};
	bool GetNoPayFlag(UINT16 usIdx) {return cdrTbl[usIdx].noPayflag;};
	void SetNoPayFlag(UINT16 usIdx,UINT8 AdminStatus){cdrTbl[usIdx].noPayflag = AdminStatus ;};
	UINT32 GetWifiEidFlag(UINT16 usIdx) {return cdrTbl[usIdx].wifi_eid_flag;};
	void SetWifiEidFlag(UINT16 usIdx,UINT8 flag){cdrTbl[usIdx].wifi_eid_flag = flag ;};
	
	void SetWifiUid(UINT16 usIdx,UINT32 uid){cdrTbl[usIdx].cdr.wifi_UID= uid ;};
	UINT32 getWifiUid(UINT16 usIdx){return cdrTbl[usIdx].cdr.wifi_UID;};
	void SetWifiIp(UINT16 usIdx,UINT32 wifi_ip){cdrTbl[usIdx].cdr.wifi_IP = wifi_ip;};
	void setCdrUid(UINT16 usIdx,UINT32 uid){cdrTbl[usIdx].cdr.ulUid = uid;};//lijinan 20091103 add
	UINT32 getCdrUid(UINT16 usIdx){return cdrTbl[usIdx].cdr.ulUid;};
	bool get_cdr_profile(char* pData, UINT16 usIdx);
	void setUtType(UINT16 usIdx,UINT8 utType){cdrTbl[usIdx].cdr.User_type = utType;};
	void setUtUpDownBW(UINT16 usIdx,char* bw){memcpy(cdrTbl[usIdx].cdr.bw,bw,8);};
	UINT16 GetReqRemainFlag(UINT16 usIdx){return cdrTbl[usIdx].req_remain_flag;};
	int GetOverFlag(UINT16 usIdx){return cdrTbl[usIdx].flag_over;};
	_CDR* setRtChargeRptBuf(char* buf,UINT16 index,RT_RPT_MSG_TYPE flag);
	void  SetReqRemainFlag(UINT16 usIdx){  cdrTbl[usIdx].req_remain_flag = 1;};
	void uidChange(UINT32 new_uid, UINT16 index);
	void func_rtCharge_rsp(UINT8*,int);
	void func_clear_Rt_data(UINT16 usIdx);
private:
    UINT32  cdrId;     // CDR���б�ʶ
       //#ifdef LJF_BILLING_VOICE
    UINT32  vcdrId;     // CDR���б�ʶ
    CDR cdrTbl[M_MAX_UT_PER_BTS];	
    list<UINT16> FreeCdrList;
    map<UINT32, UINT16> BTreeByEID;		// EID������
    map<CMac, UINT16> BTreeByMAC;  // mac ����
    map<UINT32, UINT16> BTreeByUID;		// UID������
    struct    {
        bool isCdrFileExist;        //�Ƿ�����ļ�����һ��д������ʱ��true,�����ļ���ʱ��false
#if 1//def LJF_BILLING_VOICE
        char CdrFile[48];
        char RsvCdrFile[40];    //FTP���ϱ������ļ�
        char VcdrFile[40];
        char RsvVcdrFile[40];    //FTP���ϱ������ļ�
#else
        char CdrFile[30];
        char RsvCdrFile[30];    //FTP���ϱ������ļ�
#endif
    } m_tFileInfo;

    //�ļ�ͷ��	
    struct    _tagRECORD_HEAD{
    //CDRͷ�� 20�ֽ�
        UINT32 fileLen;
        UINT32 version;
        UINT16 cdrNum;
        UINT16 cdrIndex;
	 UINT32 rsv[2];

   //CDR���� 12�ֽ�ȫ����0
        UINT16 cdrSon[6];
    } m_tfileHead;
	SEM_ID cdrSemId;
	MSG_Q_ID cdrMsgQ;
	UINT32 writeFileTid;
public:
    stBilling_newStat billingNewStat;
    void newStatBegin();
    void stat_countTimeLeft(STAT_DATA tt);
    void stat_addData(UINT32 data, STAT_DATA tt);
    void stat_addDataFlow(UINT32 data);
    void stat_addTimeLong(UINT32 data);
    void newStatEnd(UINT8 *pdata, UINT8 *ptimelong);

};
#include "L3_BTS_PM.h"
extern T_BTSPM_FTPINFO g_tFTPInfo;

//lijinan 20081205 �Ʒ�ϵͳ����





//////////////////////////////////////////////////////////////////////////
//MacFilter��ṹ
//////////////////////////////////////////////////////////////////////////
#pragma pack (1)
typedef struct _tag_MACFilterEntry
{
    bool            IsOccupied;
    UINT8           MAC[M_MAC_ADDRLEN];
#if 0	
    UINT8           TYPE;
#endif
    UINT16          TTL;
    UINT16          Elapsed;
}MACFilterEntry;
#pragma pack ()

/*********************
 *ת����ṹ
 *********************/
#pragma pack (1)
typedef struct _tag_FTEntry
{
    bool        bIsOccupied; 
    UINT32      ulEid;
    UINT16      usGroupId;   /*0: not in any group.add by xiaoweifang to support vlan-group*/
    UINT8       aucMAC[ M_MAC_ADDRLEN ];
    bool        bIsServing;
    bool        bIsTunnel;
    UINT32      ulPeerBtsID;             //add by yanghuawei,to save public IP&port
    UINT32      ulPeerBtsAddr;
    UINT16      usPeerBtsPort;
    UINT16      usTTL;  /*��ʼ��TTL�ܳ�*/
    UINT16      usElapsed;
    bool        bIsAuthed;  /*has been authenticated? */
    UINT32      aulTrafficMeasure[ EB_FROM_MAX ][ EB_TO_MAX ];/* [from][to] */
    bool          bIsRcpe;
    UINT16      usVlanID;
    UINT32  video_ip[2];
	UINT16  DM_Sync_Flag;//wangwenhua add for 106 20110918
#ifdef BUFFER_EN
    CBufferList *pBufList;
#endif
} FTEntry;
#pragma pack ()

class CfilterMAC
{
public:
    CfilterMAC(){}
    ~CfilterMAC(){}
    void Add2Filter(const UINT8*);
    bool find(const CMac &) const;
    bool find(const UINT8*) const;
private:
    list<CMac> m_listToBeFilteredMAC;
};


//Ether Bridge task
class CTBridge : public CBizTask
{
friend class CTaskARP;
friend void EBShow(UINT8);
friend void eidShow();//jy 080626
friend void macShow();//jy 080626

public:
     static   UINT16 m_ping_seq;//wangwenhua add 20080617 
     static   UINT16 m_ping_ack_seq;
    static CTBridge* GetInstance();
#ifndef WBBU_CODE
    static void RxDriverPacketCallBack(char *, UINT16, char *);

#else
    static void RxDriverPacketCallBack(void *,char *, UINT16, char *);
#endif
    static void RxBTSIPstackCallBack(char *, UINT16, char *);
    static void EBFreeMsgCallBack (UINT32 param);
    static void IsCPEVisitBTSMacCallBack(char *mac,char *result);
#ifdef WBBU_CODE
     void send_packet_2_wan(unsigned char *pdata,unsigned short len);
#endif
    //////////////////////////////////////////////////////////////////////////
    //�����ǲ��Դ��룬����ģ��DHCP��PPPOE PADI���ݰ�,����һ�ִ������ݰ�,����
    //ע�͵�,ʹ��ʱ��
    //////////////////////////////////////////////////////////////////////////
    //void sendDHCPPkt();
    //void sendPPPOEPkt();
    //void sendIlligalMAC();


    //������ȷ��pData�ڴ��㹻��
    void GetPerfData(UINT8 *pData)
        {
        UINT32 *ptr = (UINT32 *)pData;
        *ptr++ = m_usNotServing;
        *ptr++ = m_usNowServing;
        *ptr++ = m_usAnchorAndServing;
        *ptr++ = m_usServingNotAnchor;
        memcpy( (UINT8*)ptr, m_aulDirTrafficMeasure, sizeof( m_aulDirTrafficMeasure ) );
        }
    //new stat
    void GetPerfDataNew(UINT32 *pData)
    {
        UINT32 *ptr = pData;
        *ptr++ = m_dataToWan;
        *ptr++ = m_dataFromWan;
        *ptr++ = m_dataToTDR;
        *ptr++ = m_dataFromTDR;         
    }
    void ClearPerfDataNew()
    {
        //clear data
        m_dataToWan = 0;
        m_dataFromWan = 0;
        m_dataToTDR = 0;
        m_dataFromTDR = 0;
    }
    UINT16 getVlanID(UINT16 group) const
        {
        return m_group[group].usVlanID;
        }

    void ClearMeasure()
        {
        memset( m_aulDirTrafficMeasure, 0, sizeof( m_aulDirTrafficMeasure ) );
        }

	void ShowCdr(UINT32 eid)
	{
		cCdrObj.showCdr(eid);
	}
	 void restartcdrTsk()
	{
		cCdrObj.restartTsk();
	}
	void  send_cdr_for_reboot()
	{
		cCdrObj.bs_reboot_send_cdr();
	}
	void func_RTCharge_rpt(UINT16 index,RT_RPT_MSG_TYPE type);
       void func_RTCharge_rpt(char* buf,int len,BTS_CB3000_MSG type,char*buf_fail,int fail_buf_len);

#ifndef __WIN32_SIM__
    CComMessage* GetComMessage();
#endif

#ifdef UNITEST
    /*UNITESTʱ,���еĳ�Ա��������Public*/
public:
#else
private:
#endif
    CTBridge();
    ~CTBridge();

    bool Initialize();
    //bool ProcessComMessage(CComMessage*);
    bool ProcessMessage(CMessage &);
    
    inline TID GetEntityId() const 
        {
        return M_TID_EB;
        }
    inline bool IsNeedTransaction()
        {
        //return false;
        return true;//lijinan 20110524 for realtimebill
        }
    virtual bool DeallocateComMessage(CComMessage*);
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return  EB_MAX_BLOCKED_TIME_IN_10ms_TICK ;};


#ifdef __WIN32_SIM__
    static DWORD WINAPI RunCleanUp(void *);
#else
    static STATUS RunCleanUp(CTBridge *);
#endif
    void doMonitor();

    static STATUS RunWriteCdrFile(CTBridge *pTask);

    // Free FT Records List methods
    void   InitFreeFTList();
    void   InsertFreeFTList(UINT16);
    UINT16 GetFreeFTEntryIdxFromList();

    // BPtree methods
    bool   BPtreeAdd(CMac&, UINT16);
    bool   BPtreeDel(CMac&);
    UINT16 BPtreeFind(CMac&);

    // Free MacFilterTable Records List methods
    void   InitFreeMFTList();
    void   InsertFreeMFTList(UINT16);
    UINT16 GetFreeMFTEntryIdxFromList();

    // MacFilterTable BPtree methods
    bool   MFTBPtreeAdd(CMac&, UINT16);
    bool   MFTBPtreeDel(CMac&);
    UINT16 MFTBPtreeFind(CMac&);
    bool sendMsg2CB3000(char*,UINT16 );
    void func_user_RTCharge_req(UINT16 );
    MACFilterEntry* GetMFTEntryByIdx(UINT16 index)
    {
        if ( !( index < M_MAX_MACFILTER_TABLE_NUM ) )
        {
            return NULL;
        }
        return &( m_MFT[ index ] );
    }

    void   InitRouterMAC();
    void   ResolveRouterMAC();
    void   FilterMac(const CComMessage *, const UINT8);

    void   InitFilterMAC();

private:

#ifndef __WIN32_SIM__
    bool   InitComMessageList();
#endif

    //---------------------------------------------
    //����ָ���±��ת�������
    //---------------------------------------------
    FTEntry* GetFTEntryByIdx(UINT16 usIdx)
        {
        if ( !( usIdx < M_MAX_USER_PER_BTS ) )
            {
            return NULL;
            }
        return &( m_FT[ usIdx ] );
        }

    //work mode methods
    void SetWorkingMode(UINT8);
    inline UINT8 GetWorkingMode()
        {
        return m_ucWorkingMode;
        }

    void SetEgressBCFilter(bool bEgrBCFltr)
        {
        m_bEgressBCFltrEn = bEgrBCFltr;
        }

    inline bool GetEgressBCFilter()
        {
        return m_bEgressBCFltrEn;
        }

    void SetExpireTimeInSeconds(UINT16 usSeconds)
        {
        m_usExpireTimeInSeconds = usSeconds;
        }

    void SetPPPoESessionAliveTime(UINT16 usPPPoESessionAliveTime)
        {
        m_usPPPoESessionAliveTime = \
            ( 0 == usPPPoESessionAliveTime )?M_PPPOE_SESSION_ALIVETIME:usPPPoESessionAliveTime;
        }
    void SetLearnedBridgeAgingTime(UINT16 usLearnedBridgeAgingTime)
        {
        m_usLearnedBridgeAgingTime= \
            ( 0 == usLearnedBridgeAgingTime )?M_LEARNED_BRIDGE_AGINGTIME:usLearnedBridgeAgingTime;
        }

    inline UINT8* GetDstMac(const CComMessage *pComMessge)
        {
        EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
        return pEtherHdr->aucDstMAC;
        }

    inline UINT8* GetSrcMac(CComMessage *pComMessge)
        {
        EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
        return pEtherHdr->aucSrcMAC;
        }
    inline UINT16 GetProtoType(const CComMessage *pComMessge)
    {
           UINT8    *pData = (UINT8*)( pComMessge->GetDataPtr() );
          VLAN_hdr *pVlan = (VLAN_hdr*)( pData + 12 );
    if ( M_ETHER_TYPE_VLAN == ntohs( pVlan->usProto_vlan ) )
        {
                 EtherHdrEX *pEtherHdr1 = (EtherHdrEX*) ( pComMessge->GetDataPtr() );
            //modify by wangx
            if(IS_8023_PACKET(ntohs(pEtherHdr1->usProto)))
            {
                return ntohs(*(UINT16*)((UINT8*)pEtherHdr1 + sizeof(EtherHdrEX)-2 + sizeof(LLCSNAP)));
            }
            else
            {
                return ntohs( pEtherHdr1->usProto );
            }
        }
       else
        {
            EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
            //modify by wangx
            if(IS_8023_PACKET(ntohs(pEtherHdr->usProto)))
            {
                return ntohs(*(UINT16*)((UINT8*)pEtherHdr + sizeof(EtherHdr)-2 + sizeof(LLCSNAP)));
            }
            else
            {
                return ntohs( pEtherHdr->usProto );
            }
        }
    }
	
//lijinan  20101020 for video
 inline UINT32 GetSrcIpAddr(const CComMessage *pComMessge)
    {
           UINT8    *pData = (UINT8*)( pComMessge->GetDataPtr() );
          VLAN_hdr *pVlan = (VLAN_hdr*)( pData + 12 );
    if ( M_ETHER_TYPE_VLAN == ntohs( pVlan->usProto_vlan ) )
        {
                 EtherHdrEX *pEtherHdr1 = (EtherHdrEX*) ( pComMessge->GetDataPtr() );
            //modify by wangx
            if(IS_8023_PACKET(ntohs(pEtherHdr1->usProto)))
            {
                //return ntohs(*(UINT16*)((UINT8*)pEtherHdr1 + sizeof(EtherHdrEX)-2 + sizeof(LLCSNAP)));
		  IpHdr *pIpHead1 = (IpHdr*)((UINT8*)pEtherHdr1 + sizeof(EtherHdrEX) + sizeof(LLCSNAP));
		  return ntohl(pIpHead1->ulSrcIp);
				
            }
            else
            {
                //return ntohs( pEtherHdr1->usProto );
		   IpHdr *pIpHead = (IpHdr*)(pEtherHdr1+1);
	          return ntohl(pIpHead->ulSrcIp);
            }

        }
       else
        {
            EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
            //modify by wangx
            if(IS_8023_PACKET(ntohs(pEtherHdr->usProto)))
            {
                IpHdr *pIpHead1 = (IpHdr*)((UINT8*)pEtherHdr + sizeof(EtherHdr) + sizeof(LLCSNAP));
		  return ntohl(pIpHead1->ulSrcIp);
            }
            else
            {
                //return ntohs( pEtherHdr->usProto );
                 IpHdr *pIpHead = (IpHdr*)(pEtherHdr+1);
	          return ntohl(pIpHead->ulSrcIp);
            }
        }
    }

    UINT16 GetPingPacket_Seq(const CComMessage *pComMessge,UINT8 flag);
     UINT16 GetPingACKPacket_Seq(const CComMessage *pComMessge,UINT8 flag);
    void SetNotifyBTSPubIP(bool bNotify)
        {
        m_blNotifyBTSPubIP = bNotify;
        }

    inline bool GetNotifyBTSPubIP()
        {
        return m_blNotifyBTSPubIP;
        }

    //---------------------------------------------
    //��ȡpComMessage��Ϣ��Tos(�������)��Ӧ��SFIDֵ
    //---------------------------------------------
    UINT8 GetTosSFID(const CComMessage *pComMessge);

    //---------------------------------------------
    //��ȡBridge��SFID��
    //---------------------------------------------
    UINT8* GetTosSFID()     { return m_aSFID; }

    UINT16 GetRelayMsgID(const CComMessage*);

    //---------------------------------------------
    //��ȡָ��ת������(Ingress/Egress/Tunnel)�İ�ͳ��ֵ
    //---------------------------------------------
    inline UINT32* GetDirTrafficMeasure(FROMDIR from)
        {
        return m_aulDirTrafficMeasure[ from ];
        }

    //---------------------------------------------
    //����ָ��ת�������ϵ�ĳ�����Ͱ�ͳ��ֵ +1
    //---------------------------------------------
    inline UINT32 IncreaseDirTrafficMeasureByOne(FROMDIR from, TRAFFICTYPE trafficType)
        {
        return m_aulDirTrafficMeasure[ from ][ trafficType ] += 1;
        }

    //---------------------------------------------
    //��ȡָ��MAC�Ĳ�ͬ�����ϵ�ת����ͳ��ֵ
    //---------------------------------------------
    UINT32** GetMacTrafficMeasure( CMac& );

    //---------------------------------------------
    //����ָ��MAC��ָ��ת������İ�ͳ��ֵ++
    //---------------------------------------------
    UINT32 IncreaseMacTrafficMeasureByOne(FTEntry *pFT, FROMDIR from, TODIR to)
        {
        return pFT->aulTrafficMeasure[ from ][ to ] += 1;
        }

    //�ṩ�����������ת����Ϣ�ӿ�
    void   ForwardTraffic(CComMessage*);

    bool NoMoneyEidIpPro(CComMessage *pComMsg,UINT32 ulEid);

    //
    void   Ingress(CComMessage*);
    void   LearnedUplinkTraffic(CComMessage *pComMsg,UINT8 rcpeflag);
    void   ForwardUplinkPacket(CComMessage*);
    UINT8  IsDhcpPacket(CComMessage*);
    bool   IsMyBTSARP(CComMessage*,UINT32 *);

    bool   SendToTunnel(FTEntry*,UINT32, void*);
    bool   SendToTunnel(UINT32, UINT16,UINT32, void*);
    bool   SendToAI(CComMessage*, UINT32, UINT16,bool);
    bool   SendToWAN(CComMessage*, UINT16, UINT8  flag =0);
    UINT16 GetGroupID(CComMessage*);

    void   Egress(CComMessage*);
    #ifdef M_TGT_WANIF
     void   IPEngress(CComMessage*);
   #endif
////void   LearnedDownlinkTraffic(CComMessage*);
    void   DownlinkPacketDispatcher(CComMessage*, UINT16, CMac&, UINT8);
    void   ForwardDownlinkPacket(CComMessage*, FTEntry*, bool, FROMDIR);

    void   TDR(CComMessage*);

    void   ForwardTrafficToAI(CComMessage*);
    void   ForwardDownlinkTraffic(CComMessage*);
    void   ForwardTunnelTraffic(CComMessage*);
    void   ForwardUplinkTraffic(CComMessage*);

    void   FTAddEntry(const CFTAddEntry&);
    void   FTDelEntry(const CFTDelEntry&);
    void   FTUpdateEntry(const CFTUpdateEntry&);
    bool   EntryAgeOut(const CFTDelEntry&,unsigned char *flag);
    bool   FTEntryExpire(CMessage&);
    void   FTModifyBTSPubIP(const CTunnelNotifyBTSIP&);
    void   EBNotifyBTSIPChange(const CDataNotifyBtsIP&);
    void   EBSendBTSIPChangeReq(const FTEntry& );

    UINT16 TTL_IPTYPE(const UINT8 &, const bool &);

    bool EBConfig(const CDataConfig&);
    bool TosSFIDConfig(const CTosSFIDConfig&);
    bool VlanGroupConfig(const CCfgVlanGroupReq&);

    void INIT_FT(FTEntry*, FTEntry&);
    void UPDATE_FT(FTEntry*, FTEntry&);

#ifdef BUFFER_EN
    void HandleBuffer(FTEntry*, FTEntry&);
#endif
    void CalcUsrCount(FTEntry *, bool);
    void CheckVLAN(CMessage &);
    bool CheckPeerBtsAddr(FTEntry *);
    
    void ModifyPacketWithVlanID(CComMessage * );
    void DATA_log(const CComMessage *, LOGLEVEL, UINT32, const char text[], ...);

    //MAC Filter Table ����
    bool MFTEntryExpire(const CMFTDelEntry&);
    void MFTAddEntry(const CMFTAddEntry &);
    void MFTUpdateEntry(const CMFTUpdateEntry &);
    void MFTDelEntry(const CMFTDelEntry &);

	//show status;
    void showStatus();
    void showFT(UINT32);
    void showPerf(const UINT8*);
    void showMFT();
	void showQoS();
	void  EidCdrAdminStatus(CComMessage *);
	void updateVideoIp(CComMessage *);//lijinan 20101020 for video
	void proMsgFromCb3000(CComMessage *);
	void func_msg_fail(CComMessage*);
	void func_moveAway_RT_Charge(CComMessage*);

    char getMacType(CComMessage* pComMessge,UINT32* srcIP);
    void recordCPEToBTSIPStack( UINT8 *srcMac,UINT32 eid,UINT32 srcIP);
    void ClearCPEVisitBTSMacFromWan(char *Mac);
    
    bool SendToBTSIPStack(CComMessage *pComMsg);

private:
#ifndef __WIN32_SIM__
//VxWorks.
    CComMessage *m_plistComMessage;
#endif
    list<UINT16> m_listFreeFT;  //����ת������������
    list<UINT16> m_listFreeMFT; //����MACFilter���������
#ifdef __WIN32_SIM__
    SOCKET  m_EtherIpSocket;     //Windows:
    WSADATA m_wsaData;
#else
    UINT32  m_EtherIpSocket;     //VxWorks:
#endif
    UINT8  m_ucWorkingMode;
    UINT16 m_usPPPoESessionAliveTime;
    UINT16 m_usLearnedBridgeAgingTime;
    bool   m_bEgressBCFltrEn; //false: ����㲥��ת����UT; true:��ֹ�㲥��ת����UT

    bool   m_blNotifyBTSPubIP;//true,BTSPubIP���£���Ҫͬ����ServingBTS��

    ////��ͬ�����û���ͳ��ֵ
    UINT16 m_usNotServing;
    UINT16 m_usNowServing;
    UINT16 m_usAnchorAndServing;
    UINT16 m_usServingNotAnchor;

    UINT16  m_usExpireTimeInSeconds; //ת���������ʱ�䳤(��λ: ��)
    FTEntry m_FT[ M_MAX_USER_PER_BTS ];  // ת����
    map<CMac, UINT16> m_FTBptree;  // ת����������

    MACFilterEntry m_MFT[ M_MAX_MACFILTER_TABLE_NUM ];  // MACFilter��
    map<CMac, UINT16> m_MFTBptree;  // MACFilter��������

    /**********************************************************************
     *performance Measurement per Direction:
     *m_aulDirTrafficMeasure[from][type]��¼(from)������(type)���ݵ�ͳ��ֵ
     *type:��enum.
     *from:Ingress,Egress,TDR �� Internal.
     *����DHCP/PPPoE Snoop�������������Ҫ��¼��ͳ��ֵ���Լ�
     *�����������͵İ�����ARP�ȣ�Ϊ��������from = Internal,
     *type = TYPE_TRAFFIC_TO_AI/TDR/WAN
     */
    UINT32 m_aulDirTrafficMeasure[ EB_FROM_MAX ][ TYPE_TRAFFIC_MAX ];

    /*SFID����TOS��ƥ��*/
    UINT8  m_aSFID[ M_TOS_SFID_NO ];

#define M_MAX_GROUP_NUM     (4096)
#pragma pack (1)
    typedef struct _tag_stGroupVlanPAIR
        {
////////UINT16 usGroupID;
        UINT16 usVlanID;
        }stGroupVlanPAIR;
#pragma pack ()
    stGroupVlanPAIR m_group[M_MAX_GROUP_NUM];

    CfilterMAC m_filterMAC;

    //lijinan 20081227  �Ʒ�ϵͳ����
    CCDR cCdrObj;
    
    UINT8 m_btsMac[6];

    //static members:
    static CTBridge* s_ptaskBridge;
//#ifdef LJF_BILLING_VOICE
    public:
		CCDR* GetCdrInstance(){ return &this->cCdrObj; };
#ifdef RCPE_SWITCH
	TrunkMRCpeRcd* m_pstTrnukMRCpe;
	BOOL trunkIsMRcpe( UINT32 ulPID );
#endif
    //new stat
    UINT32 m_dataFromWan;//��վWAN�ڽ��������ʣ�������Э��ջ���ݰ�
    UINT32 m_dataToWan;//��վWAN�ڷ��������ʣ�������Э��ջ���ݰ�
    UINT32 m_dataFromTDR;//��վ����������Ľ���������
    UINT32 m_dataToTDR;//��վ����������ķ���������

};

typedef struct _tag_EBDebugInfo
{
    bool   bEbDebugON;
    bool   bDebugMAC;
    UINT8  MacAddress[M_MAC_ADDRLEN];
    UINT32 DebugEID;
    UINT8  DebugTrafficDir;
    UINT32 dbgLvl;
}stEBDebugInfo;


typedef struct _tag_RegisterInfo
{
   
    UINT32 DebugEID;
	UINT32 TTL;
	
    
}RegisterInfo;

typedef struct _tag_DuPlicate_MAC
{
   
    UINT32 DebugEID[2];
    UINT8  MacAddress[M_MAC_ADDRLEN];
	
    
}DuplicateMac;
#endif /*__DATA_BRIDGE_H__*/
