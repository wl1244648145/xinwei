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
#ifndef _INC_OAML3CPEM
#define _INC_OAML3CPEM

#ifdef __WIN32_SIM__
#include <windows.h>
#include <map>
#include <list>
using namespace std;
#else
#include <map>
#include <stdio.h>
#include <list>
#endif

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_TIMEOUTNOTIFY
#include "TimeOutNotify.h"
#endif

#ifndef _INC_L3OAMCPEREGNOTIFY
#include "L3oamCpeRegNotify.h"
#endif

#ifndef _INC_L3OAMCPELOCATIONNOTIFY
#include "L3OamCpeLocationNotify.h"     
#endif

#ifndef _INC_L3OAMPROFUPDATEREQ
#include "L3OamCpeProfUpdateReq.h"
#endif


#ifndef _INC_L3OAMCPECOMMONREQ
#include "L3OamCpeCommonReq.h"        
#endif

#ifndef _INC_L3OAMCPESWITCHOFFNOTIFY
#include "L3OamCpeSwitchOffNotify.h"    
#endif

#ifndef _INC_L3OAMCPEREQ
#include "L3OamCpeReq.h"
#endif

#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif

#ifndef _INC_L3OAMCPEFSM
#include "L3OamCpeFSM.h"
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

#ifndef _INC_ERRORCODEDEF
#include "ErrorCodeDef.h"
#endif

#ifndef _INC_L3OAMRPTHEADER
#include "L3OamRpt.h"
#endif
enum CPE_NETIF_CFG_TYPE //sunshanggu --080708
{
	RST_CPE_NETIF = 0,
	READ_CPE_NETIF_REG,
	READ_CPE_RTL8150_REG,
	WRITE_CPE_RTL8150_REG,
	READ_CPE_NETIF_STATUS,
	CFG_CPE_NETIF_ADA,
	CFG_CPE_NETIF_SPEED_DUPLEX,
	READ_CPE_NETIF_SPEED_DUPLEX,
	CPE_NETIF_CFG_MAX_TYPE
};

enum CPE_NETIF_ADA_STATUS
{
	CPE_NETIF_ADA_INIT = 0,
	CPE_NETIF_ADA_AUTO,
	CPE_NETIF_ADA_NONEAUTO,
	CPE_NETIF_ADA_MAX
};

const UINT32 CHECK_CPE_TO_DELETE_PERIOD     = (1000 * 60); 
const UINT32 BROADCAST_BTSLOADINFO_PERIOD   = (5 * 1000 * 60); 
#ifdef M_TGT_WANIF
const UINT32 Probe_WanCPE_TO_DELETE_PERIOD = (1000*5/*20*/);// from 20s to 5s
#endif
#ifdef __WIN32_SIM__
#define M_DATA_INVALID_SOCKET   (INVALID_SOCKET)
#define M_DATA_SOCKET_ERR       (SOCKET_ERROR)
#else
#define M_DATA_INVALID_SOCKET   (ERROR)
#define M_DATA_SOCKET_ERR       (ERROR)
#endif

struct T_ToBeDeleteCpe
{
    UINT32  CPEID;
    UINT32  RemanentTime;
};

UINT32   l3oamGetEIDByUID(UINT32 UID);//wangwenhua add 20081208
UINT32  l3oamGetUIDByEID(UINT32 EID);//通过EID得到UID

#if 1//def PAYLOAD_BALABCE
typedef enum{ PAYLOAD_UNKNOW=0, PAYLOAD_A, PAYLOAD_NA }NBtsPayload; 
#endif

#ifdef LOCATION_2ND
	typedef struct hash_node_t {
		struct hash_node_t *link;
	}hash_node_t;

	typedef struct  {
		unsigned long (*hash)(const void *);  
		int (*compare)(const void *, const void *); 
		const void  *(*keyof)(const hash_node_t *);
		unsigned int nbuckets;
		unsigned int count;
		hash_node_t **table;
	} hashtab_t;
	/** Hash Tables *************************************************************/
	void    HashInit( 
		hashtab_t *, unsigned int, unsigned long (*)(const void *), 
		int (*)(const void *, const void *), const void *(*)(const hash_node_t *), hash_node_t ** );
	void    HashInsert(hashtab_t *, hash_node_t *);
	void    HashDelete(hashtab_t *, hash_node_t *);
	unsigned int    HashCount(const hashtab_t *);
#define HashEmpty(L)    ((L)->count == 0)
	hash_node_t *HashFind(const hashtab_t *, const void *);
	hash_node_t *HashFirst( const hashtab_t *H );
	hash_node_t *HashNext( const hashtab_t *H, const hash_node_t *N );

#define LOCATION_GPS_MAX (100)
#define LOCATION_DOA_MAX (100)
enum { LCT_OK, LCT_SGL_OK, LCT_CPE_FAIL, LCT_TIMEOUT, LCT_CPE_NOT_EXIST, LCT_BTS_FAIL };
typedef struct T_LOCATION_RECORD {
    //T_LOCATION_RECORD();
    T_LOCATION_RECORD();
    ~T_LOCATION_RECORD();
	hash_node_t node;
	UINT32 ulEID;
	UINT16 usTransID;
    bool bGps;//是否收到GPS数据
    bool bGpsReq;//是否需要GPS数据
    bool bDoa;//是否收到DOA 数据
    UINT8 ucGpsLen;
    UINT8 ucDoaLen;
    CTimer* pLctTimer;
    CComMessage* pComMsg;
    BOOL bTmOutMsgFlag;//标示超时情况下消息是否发送
//	CTimer* pTm;
//    bool bCreateFlag;
}stLocationRcd;

typedef hashtab_t hash_table_t;
#define LOCATION_RECORD_ENTRY_NUM 100
#define M_LOCATION_TIMER_LENGTH   3*1000
#define MSGID_LOCATION_TIMER_OUT  0x4567      
struct T_LocationRecordTable {
	hash_table_t htab; 
	hash_node_t *hentry[LOCATION_RECORD_ENTRY_NUM];
	T_LocationRecordTable();
	~T_LocationRecordTable(){};
	bool Insert(stLocationRcd *prec);
	bool Delete(const UINT8 *addr);//now is not used
	stLocationRcd *Find(const UINT8 *addr);
	stLocationRcd *First();
	stLocationRcd *Next(stLocationRcd *p);
	//void List();
};
#endif//LOCATION_2ND
class CTaskCpeM : public CBizTask
{
friend bool GetCPEConfig(UINT32, UINT8*);
friend void l3oamprintcpedata(UINT32 uleid);
friend void l3oamshowcpe();
friend UINT32   l3oamGetEIDByUID(UINT32 UID);//wangwenhua add 20081208
friend UINT32  l3oamGetUIDByEID(UINT32 EID);//通过EID得到UID

public:
    CTaskCpeM();
    static CTaskCpeM * GetInstance();    
#ifndef WBBU_CODE
    static bool CPE_AddDeleteCpeInfo(UINT32, UINT32);
#else
     bool CPE_AddDeleteCpeInfo_function(UINT32, UINT32);
#endif
	 void CPE_NetIf_Cfg(UINT32, CPE_NETIF_CFG_TYPE, UINT8*); //sunshanggu -- 080708
	 void CPE_Debug_Cfg_OK(UINT32 eid,UINT16 type);//wangwenhua add 20081211
	 void ProbeWanifCpe(UINT32 eid);
	 void ResetWanifCpe(UINT32 eid);
	 CPE_NETIF_ADA_STATUS Get_TempNetIfAdaStatus(){return CTaskCpeM::TempNetifAdaStatus;};
	 void Set_TempNetIfAdaStatus(CPE_NETIF_ADA_STATUS Status){CTaskCpeM::TempNetifAdaStatus = Status;};
	 CPE_NETIF_ADA_STATUS Get_NetIfAdaStatus(){return CTaskCpeM::NetifAdaStatus;};
	 void Set_NetIfAdaStatus(CPE_NETIF_ADA_STATUS Status){CTaskCpeM::NetifAdaStatus = Status;};
        void CPE_SetDlFlag(UINT32 eid, UINT8 flag);
       // void CPE_SetSagStatus(UINT32 flag) {CTaskCpeM::m_SagStatus = flag;};
            //lijinan 20081208 计费系统增加
    UINT32 FindUidViaEid(UINT32 eid,UINT8*,UINT8*,char*);
    bool getWifiFlag(UINT32);
   void CPE_DeleteCpeData(UINT32 eid);
    //lijinan 20081208 计费系统增加

private:
    bool Initialize();
    bool ProcessMessage(CMessage&);
    TID GetEntityId() const;

    #define CPEM_MAX_BLOCKED_TIME_IN_10ms_TICK (100)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return CPEM_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

private:    
    bool CPE_ProfileUpateRsp(CMessage&);
	bool CPE_ProfileUpateFail(CMessage&);
    bool CPE_L2ProfileUpateNotify(CMessage&);
////bool CPE_SwitchoffNotify(CMessage&);
	bool CPE_ProbeCPE(CMessage&, UINT16 FailRspMsgID);
      bool CPE_HeartBeatMsg(CMessage &rMsg);//wangwenhua add 20090213
//wangwenhua add 20081119
       bool   CPE_CfgNetWorkCPE(CMessage &rMsg);
      bool    CPE_ReqNetWorkCPE(CMessage &rMsg);
	  //wangwenhua add end 20081119
	bool CPE_ProbeUTResponse(CMessage&);
    bool CPE_ResetCPE(CMessage&, UINT16);
	bool CPE_CheckToDelete(CMessage&);
    void UM_PostCommonRsp(TID tid, UINT16 transid, UINT16 msgid, UINT16 result);
////void CPE_DeleteCpeData(CMessage&);
////void CPE_SendCpeRegNotifySimMsg(); 
	void CPE_ProbeCpeFail(CMessage&);
    CTimer* CPE_Createtimer(UINT16, UINT8, UINT32);
	void CPE_L2BtsLoadInfoNotify(CMessage&);
    void CPE_SyncBTSLoadInfo( bool bNotify=false );
    void SendBySocket(UINT32, void*, UINT32);
    void CPE_CpeGetNeighborListReq(CMessage&);
	void CPE_BroadcastBtsLoadInfoToCpe(CMessage&);
    bool CPE_BtsNeighborBtsLoadInfoSyncReq(CMessage&);
	bool SendMsgTotSocket(UINT32 btsid,UINT8* pData, UINT32 len);
    bool CPE_AccessReq(const CUTAccessReq&);
    bool CPE_sabisAuthCMD(const CsabisAuthenticationCMD &);
    bool CPE_UTAuthRsp(const CUTAuthenticationRsp &);
    bool CPE_sabisAuthResult(const CsabisAuthenticationResult &);
    bool CPE_AccountLogin_req(const CUTAccountLoginReq &);
    bool sendAuthResult(UINT32, UINT32, UINT8);
    bool sendAuthResult(UINT32 ulUID, UINT32 ulPID, UINT8 auth_result, UINT8 ind, UINT8 result);
    bool sendAuthResult(UINT32,UINT32,UINT8,UINT8,UINT8,const UINT8* SID,const UINT8* CID);//wangwenhua add 20080804
    bool CpeHWtypeRequest(CMessage &);
    bool CpeHWtypeResponse(CMessage &);
	void CPE_ClearHist(CMessage&);//liujianfeng 20080515 CPE clear HIST
	 bool CPE_Netif_Rsp(CMessage&); //sunshanggu  -- 080708
	 bool CPE_Debug_MSg(CMessage&);//wangwenhua add 20081211
	void CPE_BtsUtStatusDataReq(CMessage&);
	void CPE_BtsUtStatusDataRsp(CMessage&);
    void CPE_EmsBtsUtMemInfoReportReq(CMessage&);//zengjihan 20120503 for MEM
    void CPE_UtBtsEmsMemInfoReportRsp(CMessage&);//zengjihan 20120503 for MEM
    void CPE_GetIplistReq(CMessage&);
    void CPE_SendCommonMsg(UINT16, CMessage&);
    void CPE_RcvCommonMsg(UINT16, CMessage&);
    void CPE_BtsLayer3DataReq(CMessage&);
    void CPE_BtsLayer3DataRsp(CMessage&);
    UINT32 CPE_GetUIDByEID(UINT32 eid);//wangwenhua add 20081208
    bool CPE_SagProfileReqFail(CMessage& );//jy081221
private :
    CPEFSM  m_CpeFsm;
    CTimer  *m_pCheckCPETimer;  
    CTimer  *m_pCheckSAGDefaultTimer;//周期性检查ccb表，如果有终端在sag故障弱化时上来的，现在sag已经好了，则通知终端注册  
  #ifdef  M_TGT_WANIF
     CTimer  *m_pProbeWanCPETimer;  
  #endif
    static  CTaskCpeM  *m_Instance;    
    T_NeighbotBTSLoadInfoEle m_NeighborBTSLoadInfoEle;
#ifndef WBBU_CODE
    static list<T_ToBeDeleteCpe>  m_ToBeDeleteCpeList;
#endif
	static CPE_NETIF_ADA_STATUS NetifAdaStatus;//auto adapation mode of cpe netwrok interface, sunshanggu -- 080718
	static CPE_NETIF_ADA_STATUS TempNetifAdaStatus;
  //bool m_SagStatus;// 0:down, 1:ok
#ifdef __WIN32_SIM__
    SOCKET  m_TxSocket;    //Windows:
    WSADATA m_wsaData;
#else
    UINT32  m_TxSocket;    //VxWorks:
#endif
public:
bool RPT_SendComMsg(UINT32 ulEID, TID tid, UINT16 usMsgID, UINT8* pd, UINT16 usLen );
#ifdef WBBU_CODE
list<T_ToBeDeleteCpe>  m_ToBeDeleteCpeList;
#endif
void CPE_LoadinfoReq(CMessage& rMsg);
UINT16 GetWeightedUserNoByBtsID(UINT32 ulBtsID);
void SetWeightedUserNoByBtsID(UINT32 ulBtsID, UINT16  usdata);
#if 1//def PAYLOAD_BALABCE
NBtsPayload GetNBtsPayloadSupport(UINT32 ulBtsID);
NBtsPayload m_ucNBtsRcdForPld[NEIGHBOR_BTS_NUM];
UINT8 m_ucPayloadSupportNotifyCnt;

public:
void plbPrint(UINT8 uch, UINT8 uc1);

#endif
#ifdef LOCATION_2ND
private:
UINT8 m_ucLocationSwitch[100];
UINT32 m_ulBTSID;
T_LocationRecordTable m_stLocationRcdTbl;
void CPE_Location_EmsReq(CMessage& rMsg);
void CPE_Location_GpsRsp(CMessage& rMsg);
void CPE_Location_DoaRsp(CMessage& rMsg);
void CPE_Location_TmOut(CMessage& rMsg);
void CPE_Location_CreateRsp( stLocationRcd* pRcd, BOOL bTmCall=false );
void CPE_Location_CreateTmRsp( stLocationRcd* pRcd );
public:
void CPE_Location_test( UINT8 uc1, UINT32 eid, UINT8 uc2 );
void CPE_Location_HashShow();
#pragma pack(1)
    struct T_EmsReq
    {        
        UINT16  transId;
        UINT16  rcv;
        UINT32  eid;
    };
    struct T_EmsRsp
    {        
        UINT16  transId;
        UINT16   result;
        UINT32  btsid;
        UINT32  uid;
        UINT16  gpsLen;
    };
    struct T_CpeRsp
    {        
        UINT16  transId;
        UINT16  gpsLen;
    };
    struct T_L2Rsp
    {        
        UINT16  transId;
        UINT16  rcv;
        UINT32  eid;
        UINT16  rcv2;
        UINT16  doaLen;
    };
    struct T_EmsReqNew
    {        
        UINT16  transId;
        UINT16  rcv;
        UINT32  eid;
        UINT16  doaLen;
    };
#pragma pack()
#endif//LOCATION_2ND
//#ifdef LJF_RPT_ALTER_2_NVRAMLIST
void CPE_RptListReq(CMessage& rMsg);
#pragma pack(1)
    struct T_EmsRptListReq
    {        
        UINT16  usTransId;
        UINT16  usCnt;
        UINT32  ulPID[10];
    };
#pragma pack()
	bool isRptInNvram( UINT32 ulpid );
	bool rptIsNotInNvramList( UINT32 ulpid );
	void CPE_ShowRprList();
    list<T_ToBeDeleteCpe>::iterator m_check_SAG_Default_saved;
    void CPE_Check_Sag_Default_Proc();
    
};
#endif
