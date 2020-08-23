/*******************************************************************************
* Copyright (c) 2010 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagGrpCCB.h
* Create Date    : 6-Jan-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__LOCALSAGGRPCCB_H
#define	__LOCALSAGGRPCCB_H

#include "Timer.h"
#include "localSagTimer.h"
#include "localSagCommon.h"
#include "callSignalMsg.h"
#include "voiceToolFunc.h"

#define M_USE_PAGING_CHANNEL USE_PAGING_CHANNEL
#define M_USE_DAC_CHANNEL USE_DAC_CHANNEL
#define M_USE_OLD_PTTREQ_IF (USE_DAC_CHANNEL+1)

#define M_LOCALSAG_MAX_GRP_QUEUE_SIZE (5)//最多允许5个用户排队讲话
typedef struct __GrpQueItemInfoT
{
	UINT32 uid;
	UINT8 prio;
	UINT16 sessionType;
	UINT8 encryptCtrl;
	__GrpQueItemInfoT* pNext;
	__GrpQueItemInfoT* pPrev;
}GrpQueItemInfoT;
typedef struct __PttQueueT
{
	UINT16 size;
	GrpQueItemInfoT QueHead;
}PttQueueT;

class SxcGrpCCB
{
public:
	SxcGrpCCB();
	~SxcGrpCCB(){cleanGrpSrvInfo();};
	bool startGrpTimer(UINT16 timerID, CTimer**ppTimer);
	bool stopGrpTimer(UINT16 timerID, CTimer**ppTimer);
	bool deleteGrpTimer(UINT16 timerID, CTimer**ppTimer);
	void setTableIndex(UINT16 index){m_TableIndex=index;};
	UINT16 getTableIndex(){return m_TableIndex;};
	void setGID(UINT16 gid){m_GID=gid;};
	UINT16 getGID(){return m_GID;};
	void setGrpL3Addr(UINT32 grpL3Addr){m_GrpL3Addr=grpL3Addr;};
	UINT32 getGrpL3Addr(){return m_GrpL3Addr;};
	void setGrpSetupUID(UINT32 uid){m_GrpSetupUID=uid;};
	UINT32 getGrpSetupUID(){return m_GrpSetupUID;};
	bool isGrpFounder(UINT32 uid){return uid==m_GrpSetupUID;};
	void setTalkingUID(UINT32 uid){m_TalkingUID=uid;};
	UINT32 getTalkingUID(){return m_TalkingUID;};
	bool isSomeoneTalking(){return INVALID_UID!=m_TalkingUID;};
	void cleanGrpSrvInfo();
	void initGrpQue();
	void cleanGrpQue();
	bool isPttQueueFull(){return M_LOCALSAG_MAX_GRP_QUEUE_SIZE==m_PttQueue.size;};
	bool isPttQueueEmpty(){return 0==m_PttQueue.size;};
	void grantPttToUser(UINT32 uid);
	void grantPttToUser(UINT32 uid, UINT16 sessionType, UINT8 prio, UINT8 encryptFlag);
	void putOneUserInQueue(UINT32 uid, UINT16 sessionType, UINT8 prio, UINT8 encryptFlag);
	void delUserFromQueue(UINT32 uid);
	GrpQueItemInfoT getOneUserOutofQueue();
	void releaseGrpSrv(UINT8 RelCause);
	void releseGrpSrvByGrpMaker(UINT32 uid, UINT8 RelCause);
	void handlePttPressReq(CMessage& signal, UINT32 UID);//处理排队申请
	void dispatchTalkersInGrpQueue();//排队调度
	void showInfo();
	bool isPagingUserNow(){return m_blPagingUserNow;}
	void setPagingUserFlag(bool flag){m_blPagingUserNow=flag;};
	void setEncryptKey(UINT8 *pKey);
	UINT8* getEncryptKey(){return m_EncryptKey;};
	void setEncryptCtrl(UINT8 EncryptCtrl);
	UINT8 getEncryptCtrl(){return m_EncryptCtrl;};
	void setPrio(UINT8 prio){m_prio=prio;};
	UINT8 getPrio(){return m_prio;};
	void setCommType(UINT8 commType){m_commType=commType;};
	UINT8 getCommType(){return m_commType;};
	void setGrpSize(UINT16 grpSize){m_grpSize=grpSize;};
	UINT16 getGrpSize(){return m_grpSize;};
	void setTransID(UINT8 transID){m_transID=transID;};
	UINT8 getTransID(){return m_transID;	};

	bool sendInnerSignalPttConnect(UINT32 uid);
	bool sendInnerSiganlGrpCallingRls(UINT32 uid, UINT8 RelCause);
	bool sendInnerSignalPttInterrupt(UINT32 uid, UINT8 reason);

	bool sendDLSignalLEPagingStart();
	bool sendDLSignalGrpHandoverRsp(CSAbisSignal& ReqMsg, UINT8 result);
	bool sendDLSignalGrpResRsp(CSAbisSignal& ReqMsg, UINT8 result);
	bool sendDLSignalPttGranted(UINT32 uid);
	bool sendDLSignalLAGrpPaging();
	bool sendDLSignalPressInfo(UINT32 UidInQue);
	bool sendDLSignalGroupRelease(UINT8 RelCause);
	bool sendDLSignalPttPressRsp(UINT8 result, UINT32 uid, UINT16 sessionType, UINT8 prio, UINT8 encryptFlag);
	
	CTimer *m_pTmGrpPaging;//LAGrpPaging timeout
	CTimer *m_pTmGrpAssignResReq;//等待讲话方讲话
	CTimer *m_pTmGrpPressInfo;//periodicly tell who's taling or idle
	CTimer *m_pTmMaxGrpAlive;
	CTimer *m_pTmMaxGrpTalking;
	CTimer *m_pTmMaxGrpIdle;

	ValGuard m_grpVDataGuard;
protected:
private:
	UINT16 m_TableIndex;
	UINT16 m_GID;
	UINT32 m_GrpL3Addr;
	UINT32 m_GrpSetupUID;
	UINT32 m_TalkingUID;
	PttQueueT m_PttQueue;
	bool m_blPagingUserNow;
	UINT8 m_EncryptCtrl;
	UINT8 m_prio;
	UINT8 m_commType;
	UINT16 m_grpSize;
	UINT8 m_transID;
	UINT8 m_EncryptKey[M_ENCRYPT_KEY_LENGTH];
};

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAGGRPCCB_H */


