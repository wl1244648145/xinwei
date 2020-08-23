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
	UINT8 CALLING[20];//	Character	20	���к���	ascii	��д
	UINT8 DIALED[24];//	Character	24	����		��
	UINT8 CALLED[24];//	Character	24	���к���	ascii	��д
	UINT8 CALLING_TY	[12];//Character	12	��������		��
	UINT8 CALLED_TYP[12];//	Character	12	��������		��
	UINT8 CALLING_DE[12];//	Character	12	�����豸		��
	UINT8 CALLED_DEV[12];//	Character	12	�����豸		��
	UINT8 CALL_TYPE[12];//	Character	12	��������		��
	UINT8 START_DATE[8];//	Character	8	��ʼ����	Ascii,yyyymmdd	��д
	UINT8 START_TIME[6];//	Character	6	��ʼʱ��	Ascii,hhmmss	��д
	UINT8 ANSWER_DA[8];//	Character	8	��Ӧ����	Ascii,yyyymmdd	��д
	UINT8 ANSWER_TM[6];//	Character	6	��Ӧʱ��	Ascii,hhmmss	��д
	UINT8 END_DATE[8];//	Character	8	��������	Ascii,yyyymmdd	��д
	UINT8 END_TIME[6];//	Character	6	����ʱ��	Ascii,hhmmss	��д
	UINT8 DURATION_S[12];//	Character	12	ռ��ʱ��		��
	UINT8 DURATION_T[12];//	Character	12	ͨ��ʱ��	��λ�룬int	��д
	UINT8 CALL_STATU[10];//	Character	10	ͨ��״̬	connected	�Ƿ�ɹ�
	UINT8 CALLING_LO[10];//	Character	10	��������	��վ�ı�������	
	UINT8 CALLED_LOC[10];//	Character	10	��������	��վ�ı�������	
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
	void clearCCBCallInfo();	//�ͷź��к����
	void ClearCCBInfo();		//�û�ע�����ͷ�CCBʱ����
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

	//bool sendDLSignalErrNotiRsp(CMessage& errNotiReq);�յ����Զ���Ӧ
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
	char m_OwnNumber[M_MAX_PHONE_NUMBER_LEN+1];		//���û��绰����
	char m_DialedNumber[M_MAX_PHONE_NUMBER_LEN+1];	//���û�����ĺ���
	UINT8 m_DialedNumberLen;		//�������λ��
	CTimer *m_pTimer;
	UINT16 m_timerID;
	UINT8 m_State;					//��ǰ״̬
	bool m_blOrigCall;				//�Ƿ�����
	bool m_blDialToneStopped;
	UINT16 m_GID;	//��ǰ��֮������GID
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


