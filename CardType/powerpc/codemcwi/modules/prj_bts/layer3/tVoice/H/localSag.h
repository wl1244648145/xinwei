/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSag.h
* Create Date    : 16-Sep-2009
* programmer     :fb
* description    :
* functions      : 
	本SAG只考虑本基站下的CPE与CPE之间的呼叫，
	不考虑CPE与其他网络和终端的互通
* Modify History :
*******************************************************************************/

#ifndef	__LOCALSAG_H
#define	__LOCALSAG_H

#include <string>
#include <map>
#include <list>
#include <vector>
using namespace std;

#include "log.h"

#include "localSagCommon.h"
#include "localSagFsm.h"
#include "localSAGTimer.h"
#include "localSagGrpCCB.h"

#define SAG_LOG_UT_ACTION LOG_DEBUG1
#define SAG_LOG_DL_SIGNAL  LOG_DEBUG2
#define SAG_LOG_INNER_SIGNAL  LOG_DEBUG2
#define SAG_LOG_TIMER LOG_DEBUG2
#define SAG_LOG_MSG LOG_DEBUG2
#define SAG_LOG_UL_SIGNAL LOG_DEBUG2
#define SAG_LOG_PLAYTONE LOG_DEBUG3
#define SAG_LOG_VOICE_GRP_INFO1 LOG_DEBUG1
#define SAG_LOG_VOICE_GRP_INFO2 LOG_DEBUG2
#define SAG_LOG_VOICE_GRP_INFO3 LOG_DEBUG3

typedef map<UINT32,CCCB*,less<UINT32> > UID_INTDEX_TABLE;
typedef map<UINT32,CCCB*,less<UINT32> > L3ADDR_INDEX_TABLE;	
typedef map<string,CCCB*,less<string> > PHONE_ADDRESS_BOOK;

class grpUserInfo
{
public:
	grpUserInfo(){};
	grpUserInfo(UINT16 _gid, UINT8 _prio):gid(_gid),prio(_prio){};
	~grpUserInfo(){};
	UINT16 gid;
	UINT8 prio;
};

class userInfo
{
public:
	userInfo(UINT32 _uid, UINT32 _pid, UINT8 _prio, char*_telNO)
		:uid(_uid),pid(_pid),prio(_prio),telNO(_telNO){};
	//userInfo(const userInfo& rh):uid(rh.uid),pid(rh.pid),prio(rh.prio),telNO(rh.telNO),grpUserInfoTbl(rh.grpUserInfoTbl){};
	userInfo(){};
	~userInfo(){grpUserInfoTbl.clear();};

	UINT32 uid;
	UINT32 pid;
	UINT8 prio;
	string telNO;
	map<UINT16,grpUserInfo,less<UINT16> > grpUserInfoTbl;
};
class grpInfo
{
	public:
	grpInfo(UINT16 _gid, UINT8 _prio, char* _grpName):
		gid(_gid),grpPrio(_prio),grpName(_grpName){};
	//grpInfo(const grpInfo& rh):gid(rh.gid),grpPrio(rh.grpPrio),grpName(rh.grpName){};
	grpInfo(){};
	~grpInfo(){};

	UINT16 gid;
	UINT8 grpPrio;
	string grpName;
};


class CSAG;
typedef void (CSAG::*SignalProcFuncPtr)(CSAbisSignal&);
typedef void (CSAG::*timeoutProcFuncPtr)(CMessage&);

class CSAG
{
public:
	static CSAG* getSagInstance();
	bool Init();
	void initSignalHandlers();
	void initTimeoutHandlers();
	UINT8 AllocateGrpTransID();
	UINT32 AllocateL3Addr();
	UINT32 AllocateGrpL3Addr(){return AllocateL3Addr();};

	SxcGrpCCB* AllocGrpCCB(UINT16 gid);
	void DeAllocGrpCCB(UINT16 TabIndex);
	SxcGrpCCB* FindGrpCCBByGID(UINT16 gid);
	SxcGrpCCB* FindGrpCCBByGrpL3Addr(UINT32 grpL3Addr);
	void AddGIDIndexTable(UINT16 gid, SxcGrpCCB* pGrpCCB);
	void DelGIDIndexTable(UINT32 gid);
	void AddGrpL3AddrIndexTable(UINT32 grpL3Addr, SxcGrpCCB* pGrpCCB);
	void DelGrpL3AddrIndexTable(UINT32 grpL3Addr);	

	CCCB* AllocCCB(UINT32 uid);
	void DeAllocCCB(UINT16 TabIndex);

	CCCB* FindCCBByUID(UINT32 uid);
	CCCB* FindCCBByL3Addr(UINT32 l3Addr);
	CCCB* FindCCBByLocalNumber(char* number);

	int checkDialPlan(char *calledNumber, UINT32 UIDcalling);
	void stopVoiceSrv(CCCB* pCCB);
	void releaseCurCall(CCCB* pCCB);
	void AddUIDIndexTable(UINT32 uid, CCCB* pCCB);
	void DelUIDIndexTable(UINT32 uid);
	void AddPhoneAddressBook(char* number, CCCB* pCCB);
	void DelPhoneAddressBook(char* number);
	void AddL3AddrIndexTable(UINT32 l3Addr, CCCB* pCCB);
	void DelL3AddrIndexTable(UINT32 l3Addr);

	bool sendSignalToLocalBTS(CSAbisSignal& signal, TID dstTid=M_TID_VOICE);
	bool sendInnerSignal(CMessage& signal);
	void parseAndHandleSignal(CMessage& msg);
	void parseAndHandleTimeoutMsg(CMessage& msg);
	void parseAndHandleInnerSignal(CMessage& msg);
	void parseAndHandleMsg(CMessage& msg);
	void handleCpeTelNOReg(CMessage& msg);
	void handleUtReg(CMessage& msg);
	void handleUtUnReg(CMessage& msg);
	void handleInnerSignalCallArrive(CMessage& msg);
	void releaseVoiceSrv();

	void handleVoiceDataFromBTS(CMessage& msg);

	void handleSignal_Invalid(CSAbisSignal& signal);		//InvalidSignal_MSG,	
	void handleSignalLAPagingRsp(CSAbisSignal& signal); 
	void handleSignalDeLAPagingRsp(CSAbisSignal& signal); 
	void handleSignalAssignResReq(CSAbisSignal& signal); 
	void handleSignalReleaseResRsp(CSAbisSignal& signal);  

	void handleSignalErrNotiReq(CSAbisSignal& signal);
	void handleSignalErrNotiRsp(CSAbisSignal& signal);

	void handleSignalSetup(CSAbisSignal& signal);
	void handleSignalSetupAck(CSAbisSignal& signal);
	void handleSignalAlerting(CSAbisSignal& signal);	
	void handleSignalConnect(CSAbisSignal& signal);
	void handleSignalConnectAck(CSAbisSignal& signal);
	void handleSignalDisconnect(CSAbisSignal& signal);
	void handleSignalInformation(CSAbisSignal& signal);
	void handleSignalReleaseComplete(CSAbisSignal& signal);
	void handleSignalModifyMediaReq(CSAbisSignal& signal);
	void handleSignalModifyMediaRsp(CSAbisSignal& signal);
		
	void handleSignalHandOverReq(CSAbisSignal& signal);
	void handleSignalHandOverComplete(CSAbisSignal& signal);

	void handleSignalAuthInfoReq(CSAbisSignal & signal);
	void handleSignalSecuriryCardCallParaReq(CSAbisSignal & signal);
	void handleSignalSecuriryCardCallParaRsp(CSAbisSignal & signal);
	void handleSignalOamTransferInfoReq(CSAbisSignal & signal);
	void handleSignalOamTransferInfoRsp(CSAbisSignal & signal);
	void handleSignalLoginReq(CSAbisSignal& signal);
	void handleSignalLogout(CSAbisSignal& signal);
	void handleSignalAuthCmdRsp(CSAbisSignal& signal);

	void handleSignalMOSMSDataReq(CSAbisSignal& signal);
	void handleSignalMTSMSDataRsp(CSAbisSignal& signal);
	void handleSignalSMSMemAvailReq(CSAbisSignal& signal);

	void handleSignalLAGrpPagingRsp(CSAbisSignal& signal); 
	void handleSignalStatusReport(CSAbisSignal& signal); 
	void handleSignalGrpHandoverReq(CSAbisSignal& signal); 
	void handleSignalGrpResReq(CSAbisSignal& signal); 
	void handleSignalPttSetupReq(CSAbisSignal& signal); 
	void handleSignalPttConnectAck(CSAbisSignal& signal); 
	void handleSignalGrpDisconnect(CSAbisSignal& signal); 
	void handleSignalGrpCallingRlsComplete(CSAbisSignal& signal); 
	void handleSignalPttPressReq(CSAbisSignal& signal); 
	void handleSignalPttInterruptAck(CSAbisSignal& signal); 
	void handleSignalPttRls(CSAbisSignal& signal); 
	void handleSignalPttPressCancel(CSAbisSignal& signal); 
	void handleSignalPttPressApplyReq(CSAbisSignal& signal); 	

	void sigleVoiceCallTimeoutProc(CMessage& msg);
	void handleTimeout_O_Setup(CMessage& msg);
	void handleTimeout_O_DialNumber(CMessage& msg);
	void handleTimeout_O_Alerting(CMessage& msg);
	void handleTimeout_O_Connect(CMessage& msg);
	void handleTimeout_O_ConnectAck(CMessage& msg);
	void handleTimeout_T_SetupAck(CMessage& msg);
	void handleTimeout_T_Alerting(CMessage& msg);
	void handleTimeout_T_Connect(CMessage& msg);
	void handleTimeout_T_ConnectAck(CMessage& msg);
	void handleTimeoutDisconnect(CMessage& msg);
	void handleTimeoutReleaseComplete(CMessage& msg);
	void handleTimeout_T_LaPaging(CMessage& msg);
	void handleTimeout_T_AssignTransRes(CMessage& msg);
	void handleTimeout_Grp_PttConnect(CMessage& msg);
	void handleTimeout_Grp_LAGrpPaging(CMessage& msg);
	void handleTimeout_Grp_AssignResReq(CMessage& msg);
	void handleTimeout_Grp_PressInfo(CMessage& msg);
	void handleTimeout_Grp_MaxIdleTime(CMessage& msg);
	void handleTimeout_Grp_TTL(CMessage& msg);
	void handleTimeout_Grp_TalkingTime(CMessage& msg);
	
	bool sendLoginRspWithUserSrvInfo(CSAbisSignal& signal);
	bool sendSuccessLoginRspWithoutUserSrvInfo(CSAbisSignal& signal);
	bool sendDLSignalGrpHandoverRsp(CSAbisSignal& ReqMsg, UINT8 result, UINT32 grpL3Addr, UINT8 transID);
	bool sendPttPressRsp(UINT16 gid, UINT8 result, UINT32 uid, UINT16 sessionType, UINT8 prio, UINT8 encryptFlag);
	void sendDisconnect(CCCB* pCCB, UINT8 RelCause);
	void sendRelease(CCCB* pCCB, UINT8 RelCause);
	void sendRlsTransResReq(CCCB* pCCB, UINT8 RelCause);
	void setInnerSignalHead(CMessage& msg, UINT16 sigType);
	UINT16 getInnerSignalType(CMessage& msg);
	void setAllHeadTailFields(UINT16 nPayloadLen, SignalType sigType, CSAbisSignal& signal);

	void clearSignalCounters();
	void doTxSignalAcount(CSAbisSignal& signal);
	void doRxSignalAcount(CSAbisSignal& signal);
	void showSignalAccount();
	void showUsers(bool blDeatailFlag);
	void showPhoneBook(bool blDeatailFlag);
	void showActiveCall(bool blDeatailFlag);

//-----------------------------------

void initACLFromFile(char *pBuf, UINT32 size);
void initGrpInfoFromFile(char *pBuf, UINT32 size);
void initUserInfoFromFile(char *pBuf, UINT32 size);
void initGrpUserInfoFromFile(char *pBuf, UINT32 size);
void showACL(UINT32 uid);
void showUserInfo(UINT32 uid);
void showGrpInfo(UINT16 gid);
UINT32 saveACLToFile(char *pBuf, UINT32 *pSize);
UINT32 saveGrpInfoToFile(char *pBuf, UINT32 *pSize);
UINT32 saveUserInfoToFile(char *pBuf, UINT32 *pSize);
UINT32 saveUserGrpInfoToFile(char *pBuf, UINT32 *pSize);
bool ifAllowUserAccess(UINT32 pid, UINT32 uid);
bool isUserInACL(UINT32 uid, UINT32 pid);
bool ifAllowGrpSetup(UINT16 gid, UINT32 uid);
bool isUserInGroup(UINT32 uid, UINT16 gid);
UINT16 formatUserGrpListInfo(char*buf, UINT16& len, UINT8& grpNum, UINT32 uid);
void addACLUser(UINT32 uid, UINT32 pid);
void delACLUser(UINT32 uid);
bool findUserInfoByUID(UINT32 uid, map<UINT32, userInfo, less<UINT32> >::iterator& itFound);
void addUserInfo(UINT32 uid, UINT32 pid, char* telNO, UINT8 prio);
void delUserInfo(UINT32 uid);
void addUserGrpInfo(UINT32 uid, UINT16 gid, UINT8 prioInGrp);
void delUserGrpInfo(UINT32 uid, UINT16 gid);
void clearUserGrpInfo(UINT32 uid);
bool findGrpInfoByGID(UINT16 gid, map<UINT16, grpInfo, less<UINT16> >::iterator& itFound);
void addGrpInfo(UINT16 gid, char* grpName, UINT8 grpPrio);
void delGrpInfo(UINT16 gid);
void clearACL();
void clearAllUserInfo();
void clearAllGrpInfo();

//-----------------------------------


protected:

private:
	CSAG(){};
	static CSAG* s_pSagInst;
	static UINT32 m_L3AddrRes;
	static UINT32 m_GrpTransIDRes;
	CCCB m_CCBTable[M_MAX_CCB_NUM];

	list < UINT16 > m_freeCCBList;
	UID_INTDEX_TABLE	m_Uid_index_Table;
	L3ADDR_INDEX_TABLE	m_L3Addr_index_Table;
	PHONE_ADDRESS_BOOK  m_Phone_Address_Book;

	SxcGrpCCB m_GrpCCBTable[M_MAX_GRPCCB_NUM];
	list<UINT16> m_freeGrpCCBList;
	map<UINT16,SxcGrpCCB*,less<UINT16> > m_Gid_index_Table;
	map<UINT32,SxcGrpCCB*,less<UINT32> > m_GrpL3Addr_index_Table;	

	//用户接入限制列表(uid,pid)
	map<UINT32, UINT32, less<UINT32> > m_AccessLst;
	//用户业务信息索引表(uid索引)
	map<UINT32, userInfo, less<UINT32> > m_UserInfoTbl;
	//组信息索引表(gid索引)
	map<UINT16, grpInfo, less<UINT16> > m_GrpInfoTbl;
	
	//信令统计表
	static UINT32	RxSignalCounter[InvalidSignal_MSG+1];
	static UINT32	TxSignalCounter[InvalidSignal_MSG+1];
	static SignalProcFuncPtr signalProc[InvalidSignal_MSG+1];
	static timeoutProcFuncPtr timeoutProc[TIMERID_SAG_COUNT];
	
};





#ifdef __cplusplus
extern "C" {
#endif

void showSagSignalCounters();
void clearSagSignalCounters();

void loadSrvCfgInfoFromFile();
void saveSrvCfgInfoToFile();
void initACLFromFile(char *pBuf, UINT32 size);
void initGrpInfoFromFile(char *pBuf, UINT32 size);
void initUserInfoFromFile(char *pBuf, UINT32 size);
void initGrpUserInfoFromFile(char *pBuf, UINT32 size);
void showACL(UINT32 uid);
void showUserInfo(UINT32 uid);
void showGrpInfo(UINT16 gid);
UINT32 saveACLToFile(char *pBuf, UINT32 *pSize);
UINT32 saveGrpInfoToFile(char *pBuf, UINT32 *pSize);
UINT32 saveUserInfoToFile(char *pBuf, UINT32 *pSize);
UINT32 saveUserGrpInfoToFile(char *pBuf, UINT32 *pSize);
bool ifAllowUserAccess(UINT32 pid, UINT32 uid);
bool isUserInACL(UINT32 uid, UINT32 pid);
bool ifAllowGrpSetup(UINT16 gid, UINT32 uid);
bool isUserInGroup(UINT32 uid, UINT16 gid);
UINT16 formatUserGrpListInfo(char*buf, UINT16& len, UINT8& grpNum, UINT32 uid);
void addACLUser(UINT32 uid, UINT32 pid);
void delACLUser(UINT32 uid);
//bool findUserInfoByUID(UINT32 uid, map<UINT32, userInfo, less<UINT32> >::iterator& itFound);
void addUserInfo(UINT32 uid, UINT32 pid, char* telNO, UINT8 prio);
void delUserInfo(UINT32 uid);
void addUserGrpInfo(UINT32 uid, UINT16 gid, UINT8 prioInGrp);
void delUserGrpInfo(UINT32 uid, UINT16 gid);
void clearUserGrpInfo(UINT32 uid);
//bool findGrpInfoByGID(UINT16 gid, map<UINT16, grpInfo, less<UINT16> >::iterator& itFound);
void addGrpInfo(UINT16 gid, char* grpName, UINT8 grpPrio);
void delGrpInfo(UINT16 gid);
void clearACL();
void clearAllUserInfo();
void clearAllGrpInfo();


#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAG_H */



