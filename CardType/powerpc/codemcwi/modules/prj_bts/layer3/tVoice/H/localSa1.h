/*******************************************************************************
* Copyright (c) 2009 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagCCB.h
* Create Date    : 13-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__LOCALSAGCCB_H
#define	__LOCALSAGCCB_H

#include "Timer.h"
#include "localSagTimer.h"
#include "localSagCommon.h"
#include "sysBtsConfigData.h"
#ifdef DSP_BIOS
#include "time.h"
extern "C" T_TimeDate bspGetDateTime();
#endif
typedef struct _LocalSagCdrStructT
{
	UINT8 CALLING[20];//	Character	20	主叫号码	ascii	填写
	UINT8 DIALED[24];//	Character	24	拨号		空
	UINT8 CALLED[24];//	Character	24	被叫号码	ascii	填写
	UINT8 CALLING_TY	[12];//Character	12	主叫类型		空
	UINT8 CALLED_TYP[12];//	Character	12	被叫类型		空
	UINT8 CALLING_DE[12];//	Character	12	主叫设备		空
	UINT8 CALLED_DEV[12];//	Character	12	被叫设备		空
	UINT8 CALL_TYPE[12];//	Character	12	呼叫类型		空
	UINT8 START_DATE[8];//	Character	8	开始日期	Ascii,yyyymmdd	填写
	UINT8 START_TIME[6];//	Character	6	开始时间	Ascii,hhmmss	填写
	UINT8 ANSWER_DA[8];//	Character	8	响应日期	Ascii,yyyymmdd	填写
	UINT8 ANSWER_TM[6];//	Character	6	响应时间	Ascii,hhmmss	填写
	UINT8 END_DATE[8];//	Character	8	结束日期	Ascii,yyyymmdd	填写
	UINT8 END_TIME[6];//	Character	6	结束时间	Ascii,hhmmss	填写
	UINT8 DURATION_S[12];//	Character	12	占用时长		空
	UINT8 DURATION_T[12];//	Character	12	通话时长	单位秒，int	填写
	UINT8 CALL_STATU[10];//	Character	10	通话状态	connected	是否成功
	UINT8 CALLING_LO[10];//	Character	10	主叫区号	基站的本地区号	
	UINT8 CALLED_LOC[10];//	Character	10	被叫区号	基站的本地区号	
}LocalSagCdrStructT;

typedef struct __LocalSagDateTimeT
{
	UINT16 year;
	UINT8 month;
	UINT8 day;
	UINT8 hour;
	UINT8 minute;
	UINT8 second;
}LocalSagDateTimeT;

class LocalSagCdr
{
	public:
		LocalSagCdr(){};
		~LocalSagCdr(){};
		UINT8* getCdrBufPtr(){return (UINT8*)(&m_CdrRecord);};
		UINT16 getCdrLength(){return sizeof(LocalSagCdrStructT);};
		void setStartDateTime(UINT16 year, UINT8 month, UINT8 day, UINT8 hour, UINT8 minute, UINT8 second);
		void setConnectDateTime(UINT16 year, UINT8 month, UINT8 day, UINT8 hour, UINT8 minute, UINT8 second);
		void setEndDateTime(UINT16 year, UINT8 month, UINT8 day, UINT8 hour, UINT8 minute, UINT8 second);
		void finishCdrRecord(char *pCalling, char *pCalled);
		bool reportCdr();
	protected:

	private:
		LocalSagCdrStructT m_CdrRecord;
		LocalSagDateTimeT m_dtStart;
		LocalSagDateTimeT m_dtConnect;
		LocalSagDateTimeT m_dtEnd;
		
		void setCalling(char* calling);
		void setCalled(char* called);
		void setDateTime(LocalSagDateTimeT *pDT, UINT16 year, UINT8 month, UINT8 day, UINT8 hour, UINT8 minute, UINT8 second);
		time_t getTimeFromDateTime(LocalSagDateTimeT *pDT);
		UINT32 computeDuration();
};


class CCCB
{
public:
	CCCB();
	~CCCB(){};
	void clearCCBGrpVoiceCallInfo();
	void clearCCBCallInfo();	//释放呼叫后调用
	void ClearCCBInfo();		//用户注销后，释放CCB时调用
	UINT32 getEID(){return m_Eid;};
	void setEID(UINT32 eid){m_Eid=eid;};
	UINT32 getCID(){return m_Cid;};
	void setCID(UINT8 cid){m_Cid=cid;};
	UINT32 getUID();
	void setUID(UINT32 uid);
	UINT32 getL3Addr();
	void setL3Addr(UINT32 l3Addr);
	CCCB* getPeerCCB();
	void setPeerCCB(CCCB* pCCB);
	char* getOwnNumber();
	void setOwnNumber(char* number);
	void SaveDialedNumber(char Digit);
	char* getDialedNumber();
	void setDialedNumber(char* number);
	UINT16 getCCBTableIndex();
	void setCCBTableIndex(UINT16 TabIndex);
	UINT8 getCodec();
	void setCodec(UINT8 codec);
	UINT8 getDialedNumberLen();
	void setDialedNumberLen(UINT8 len);
	UINT16 getTabIndex();
	void setTabIndex(UINT16 index);
	UINT8 getState();
	void setState(UINT8 state);
	bool isOrigCall();
	void setOrigCall(bool blOrigCall);
	bool isDialToneStopped(){return m_blDialToneStopped;};
	void setDialToneStoppedFlag(bool flag){m_blDialToneStopped=flag;};
	UINT16 getGID(){return m_GID;};
	void setGID(UINT16 gid){m_GID=gid;};
	void setEncryptCtrl(UINT8 EncryptCtrl){m_EncryptCtrl=EncryptCtrl;};
	UINT8 getEncryptCtrl(){return m_EncryptCtrl;};
	void setPrio(UINT8 prio){m_prio=prio;};
	UINT8 getPrio(){return m_prio;};
	void setSetupCause(UINT8 setupCause){m_SetupCause=setupCause;};
	UINT8 getSetupCause(){return m_SetupCause;};

	bool playTone(UINT16 idTone);
	bool stopTone(UINT16 idTone);
	void enterState();
	void exitState();
	bool startTimer(UINT16 timerID);
	bool stopTimer(UINT16 timerID=TIMERID_SAG_COUNT);
	bool deleteTimer(UINT16 timerID=TIMERID_SAG_COUNT);

	//bool sendDLSignalErrNotiRsp(CMessage& errNotiReq);收到后自动回应
	UINT8 getDisconnectReason(CMessage& msg);
	bool sendDLSignalLAPaging(UINT16 appType);
	bool sendDLSignalDeLaPaging();
	bool sendDLSignalSetup();
	bool sendDLSignalSetupAck();
	bool sendDLSignalAlerting();
	bool sendDLSignalConnect();
	bool sendDLSignalConnectAck();
	bool sendDLSignalDisconnect(UINT8 reason);
	bool sendDLSignalRelease(UINT8 reason);
	bool sendDLSignalAssignResRsp(CMessage& assignResReq);
	bool sendDLSignalRlsTransResReq(UINT8 reason);
	bool sendDLSignalErrNotiReq();
	bool sendDLSignalDVoiceReq(UINT8 DVoice);
	
	bool sendDLSignalPttSetupAck(UINT8 EncryptCtrl, UINT8 prio);
	bool sendDLSignalPttConnect(UINT8 GrpOwnerFlag, UINT8 EncryptCtrl, UINT8 prio);
	bool sendDLSignalPttRlsAck();
	UINT8 getGrpCallingRlsReason(CMessage& msg);
	bool sendDLSignalGrpCallingRls(UINT8 relReason);
	UINT8 getPttInterruptReason(CMessage& msg);
	bool sendDLSignalPttInterrupt(UINT8 relReason);
//=======================================================	
	bool sendInnerSignalCallArrive(UINT32 dstUID);
	bool sendInnerSignalAlerting();
	bool sendInnerSignalConnect();
	bool sendInnerSignalConnectAck();
	bool sendInnerSignalDisconnect2Peer(UINT8 reason);
	
	LocalSagCdr m_cdr;
protected:
private:
	UINT16 m_TabIndex;
	UINT32 m_Uid;
	UINT32 m_Eid;
	UINT32 m_Cid;
	UINT32 m_L3Addr;
	UINT32 m_Codec;
	CCCB* m_pPeerCCB;
	char m_OwnNumber[M_MAX_PHONE_NUMBER_LEN+1];		//本用户电话号码
	char m_DialedNumber[M_MAX_PHONE_NUMBER_LEN+1];	//本用户拨打的号码
	UINT8 m_DialedNumberLen;		//拨打号码位数
	CTimer *m_pTimer;
	UINT16 m_timerID;
	UINT8 m_State;					//当前状态
	bool m_blOrigCall;				//是否主叫
	bool m_blDialToneStopped;
	UINT16 m_GID;	//当前与之关联的GID
	UINT8 m_prio;
	UINT8 m_EncryptCtrl;
	UINT8 m_SetupCause;
};


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAGCCB_H */


