/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    voiceFSM.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   2006-10-13 fengbing  保证呼叫没有建立的情况下不发生异常情况的切换
 *   2006-09-14 fengbing  支持异常情况下切换
 *   2006-04-10 fengbing  增加测试函数showBTree，修改AllocCCB函数使得AllocCCB时使得CCB的状态为Idle
 *   2006-04-09 fengbing  add sendVACSetupRsp member function for VoiceCCB
 *   2006-04-09 fengbing  delete VOICE_IDLE_DELAPAGING from VOICE_TRANS
 *   2006-04-09 fengbing  修改BTree删除操作
 *   2006-03-27 fengbing  增加被叫时回应PagingRsp后收到Error Notification Req的处理
 *   2006-03-26 fengbing  add getSAGCongestLevel member function to VoiceFSM class	
 *   2006-03-22 fengbing  modify FSM, add Trans_Testab_DeLAPaging routine;
 *   2005-09-13 fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/
 
#ifndef __VOICE_FSM_H
#define __VOICE_FSM_H
#include <stdio.h>
//#include "typeinfo"

#include "log.h"
#include "fsm.h"
#include "timer.h"
#include "voiceCommon.h"
#include "VAC_session_interface.h"
#include "timeoutVoiceMsg.h"
#include "Voice_msgs_struct.h"


#include <map>
#include <list>
using namespace std;

#define M_VIDEOUT_GRPRES_OPTIMIZE	

typedef enum {
	VOICE_IDLE_STATE = 0,
	VOICE_O_ESTABLISH_STATE,
	VOICE_T_ESTABLISH_STATE,
	VOICE_TRANSPARENT_STATE,
	VOICE_RELEASE_STATE,
	VOICE_PAGING_STATE,
	VOICE_PROBE_STATE,
	VOICE_WAITSYNC_STATE,
	MAX_VOICE_STATE
}VOICE_STATE;

typedef enum {

	VOICE_IDLE_VACSETUP = 0,
	VOICE_IDLE_LAPAGING,
	
	VOICE_OESTAB_ASSGN_RES_RSP,
	VOICE_OESTAB_VAC_REL,
	VOICE_OESTAB_TMOUT_ASSGN_RES_RSP,


	VOICE_TESTAB_ASSGN_RES_RSP,//VOICE_TESTAB_ASSGN_RES_REQ,
	VOICE_TESTAB_TMOUT_ASSGN_RES_RSP,
	VOICE_TESTAB_VAC_REL,
	VOICE_TESTAB_DELAPAGING,
	VOICE_TESTAB_ERR_REQ,

	VOICE_TRANSPARENT_REL_RES_REQ,
	VOICE_TRANSPARENT_VAC_REL,
	VOICE_TRANSPARENT_MODI_VAC_NOTIFY,

	VOICE_RELEASE_TMOUT_DELAY_RELVAC,
	VOICE_RELEASE_ERR_RSP,
	VOICE_RELEASE_TMOUT_ERR_RSP,
	VOICE_RELEASE_REL_RES_REQ,
	VOICE_RELEASE_TMOUT_REL_RES,

	VOICE_PAGING_VAC_SETUP_RSP,
	VOICE_PAGING_TMOUT_VACSETUP_RSP,
	VOICE_PAGING_VAC_REL,
	VOICE_PAGING_DELAPAGING,

	VOICE_PROBE_PROBERSP,
	VOICE_PROBE_TMOUT_PROBERSP,

	VOICE_WAITSYNC_TMOUT_WAITSYNC,
	VOICE_WAITSYNC_VAC_REL,
	VOICE_WAITSYNC_VACSETUP,
	VOICE_WAITSYNC_REL_RES_REQ,

	MAX_VOICE_TRANS

}VOICE_TRANS;
	

//20090531 fengbing bts inner switch for Voice Data begin
#define M_VDATA_BTS_INNER_SWITCH
//20090531 fengbing bts inner switch for Voice Data end

struct VoiceTupleStruct;
typedef struct VoiceTupleStruct VoiceTuple;
struct VoiceTupleStruct
{
	bool operator > (const VoiceTuple& b) const
	{
		return (Eid > b.Eid || (Eid==b.Eid && Cid > b.Cid) );
	}
	
	bool operator == (const VoiceTuple& b) const
	{
		return (Eid== b.Eid && Cid==b.Cid);
	}
	
	bool operator < (const VoiceTuple& b) const
	{
		return (Eid<b.Eid || (Eid==b.Eid && Cid<b.Cid));
	}
	UINT32 Eid;
	UINT16 Cid;
};
/*===========================================================================*/
// VoiceCCB
class VoiceCCBTable;
class VoiceCCB: public CCBBase
{
public:
	VoiceCCB():CCBBase(VOICE_IDLE_STATE){ 
		pTimer = NULL; 
		pTimerMoveAwayDisable = NULL;
		pTimerWaitSYNC = NULL;
		m_vDataIdx = 0xffff;
//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
		initInnerSwitch();
#endif
//20090531 fengbing bts inner switch for Voice Data end		
		CCBClean(); 
		};

	inline void setConnectedFlag() {m_blConnected=true;}
	inline void clearConnectedFlag(){m_blConnected=false;}
	inline bool isConnected(){return m_blConnected;}

	inline const VoiceTuple& getVoiceTuple() const { return m_tuple;}
	inline void setVoiceTuple(VoiceTuple& tuple){ m_tuple = tuple;}
	inline UINT32 getUID() const {return m_UID;}
	inline void setUID(UINT32 UID) {m_UID = UID;}
	inline UINT32 getL3addr() const {return m_L3Addr;}
	inline void setL3addr(UINT32 L3addr) {m_L3Addr = L3addr;}

	inline void setCPEZFlag(bool blIsCPEZ){m_blCPEZ=blIsCPEZ;}
	inline bool isCPEZ(){return m_blCPEZ;}

	inline UINT16 getAppType(){return m_AppType;}
	inline void setAppType(UINT16 appType){m_AppType = appType;}
	inline UINT8 getCodecInfo(){return m_CodecInfo;}
	inline void setCodecInfo(UINT8 codecInfo){m_CodecInfo = codecInfo;}
	inline UINT8 getAppPrio(){return m_AppPrio;}
	inline void setAppPrio(UINT8 appPrio){m_AppPrio = appPrio;}

	inline UINT16 getTabIndex(){ return m_index;}
	inline void setTabIndex(UINT16 index){ m_index = index;}
	
	void startTimer(UINT8 timerType, UINT32 lenTimer, const CMessage &rMsg);
	void stopTimer() ;
	void deleteTimer(); 
	inline void ClearTimer(){pTimer=NULL;}

	void startTimerWaitSYNC();
	void deleteTimerWaitSYNC();
	void startTimerMoveAwayDisable();
	void deleteTimerMoveAwayDisable();
	void clearTimerMoveAwayDisable(){pTimerMoveAwayDisable=NULL;};
	void CCBClean();
	void setCCBTable(VoiceCCBTable* ccbTab){ m_CCBTab = ccbTab; }
	VoiceCCBTable* getCCBTable(){ return m_CCBTab;}

//////////////////////////////////////////////////////////////////////////
	void sendRlsResRsp();
	void sendErrNotifyReqtoSAG(UINT8 errCause);	//向SAG发送Error Notification Req
	void VACRelease();	//VAC session Release
	void sendVACSetupReq(UINT8 reason, UINT8 rate);
	void sendVACSetupRsp(ENUM_VACSetupRspResultT result);
	void releaseActiveCall();

	void setPagingL3Addr(UINT32 PagingL3Addr){m_PagingL3Addr = PagingL3Addr;}
	UINT32 getPagingL3Addr(){return m_PagingL3Addr;}
	void setEntryStateFN(UINT32 curFN){m_EntryStateFN=curFN;};
	UINT32 getEntryStateFN(){return m_EntryStateFN;};
	UINT32 getInStateTime(UINT32 curFN);

//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
	void setBtsInnerSwitchVDataFlag(bool flag){m_blInnerSwitchVDataFlag=flag;};
	bool isBtsInnerSwitchVData(){return m_blInnerSwitchVDataFlag;};
	void setPeerCCB(VoiceCCB* peerpCCB){m_peerCCB=peerpCCB;};
	VoiceCCB* getPeerCCB(){return m_peerCCB;};
	void setPeerL3Addr(UINT32 peerL3Addr){m_peerL3Addr=peerL3Addr;};
	UINT32 getPeerL3Addr(){return m_peerL3Addr;};

	void initInnerSwitch();
	void disableInnerSwitch();
	void enableInnerSwitch(VoiceCCB *ppeerCCB);
#endif
//20090531 fengbing bts inner switch for Voice Data end

	UINT16 m_vDataIdx;

private:
	VoiceCCBTable *m_CCBTab;
	UINT16 m_index;
	
	VoiceTuple m_tuple;
	UINT32 m_UID;
	UINT32 m_L3Addr;
	bool m_blCPEZ;

	CTimer* pTimer;
	CTimer* pTimerMoveAwayDisable;
	CTimer* pTimerWaitSYNC;

	UINT16 m_AppType;
	UINT8 m_AppPrio;
	UINT8 m_CodecInfo;

	UINT32 m_PagingL3Addr;
	UINT32 m_EntryStateFN;
	bool m_blConnected;
	
//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
	bool m_blInnerSwitchVDataFlag;
	VoiceCCB *m_peerCCB;
	UINT32 m_peerL3Addr;
#endif
//20090531 fengbing bts inner switch for Voice Data end
};

/*===========================================================================*/
//VoiceCCBTable
class VoiceCCBTable: public CCBTableBase
{
public:
    VoiceCCBTable(){
        for (int i=0; i<VOICE_CCB_NUM; i++)
        {
			CCBTable[i].setTabIndex(i);
			CCBTable[i].setCCBTable(this);
			CCBTable[i].SetCurrentState(VOICE_IDLE_STATE);
            FreeCCBList.push_back(i);
        }
    };

	inline VoiceCCB* AllocCCB()
	{
		VoiceCCB* ret = NULL;
		if (FreeCCBList.empty()) 
		{
			return ret;
		}
		ret = &CCBTable[*(FreeCCBList.begin())];
		if(ret!=NULL)
		{
			ret->SetCurrentState(VOICE_IDLE_STATE);//默认为Idle
		}
		FreeCCBList.pop_front();
		return ret;
	}
	inline void DeAllocCCB(VoiceCCB *pCCB)
	{ 
		UINT16 index = pCCB->getTabIndex();
		DelBTreeTuple(pCCB->getVoiceTuple());
		DelBTreeUID(pCCB->getUID());
		DelBTreeL3addr(pCCB->getL3addr());
		pCCB->CCBClean();
		FreeCCBList.push_back(index);
	}
	CCBBase *FindCCB(CMessage &msg);

	inline void AddBTreeTuple(VoiceTuple &tuple, UINT16 index)
	{ 
		BTreeByTuple.insert(map<VoiceTuple, UINT16>::value_type(tuple,index)); 
	}
	inline void DelBTreeTuple(const VoiceTuple &tuple)
	{
		map<VoiceTuple, UINT16>::iterator it = BTreeByTuple.find(tuple);
		if(it!=BTreeByTuple.end())
			BTreeByTuple.erase(it);
	}
	inline void AddBTreeUID(UINT32 uid, UINT16 index)
	{
		BTreeByUID.insert(map<UINT32, UINT16>::value_type(uid,index)); 
	}
	inline void DelBTreeUID(UINT32 uid)
	{
		map<UINT32, UINT16>::iterator it = BTreeByUID.find(uid);
		if(it!=BTreeByUID.end())
			BTreeByUID.erase(it);
	}
	inline void AddBTreeL3addr(UINT32 L3addr, UINT16 index)
	{
		BTreeByL3addr.insert(map<UINT32, UINT16>::value_type(L3addr,index));
	}
	inline void DelBTreeL3addr(UINT32 L3addr)
	{
		map<UINT32, UINT16>::iterator it = BTreeByL3addr.find(L3addr);
		if(it!=BTreeByL3addr.end())
			BTreeByL3addr.erase(it);
	}	
	inline void AddBTreePlayToneRecord(VoiceTuple tuple, UINT16 toneID)
	{
		BTreePlayToneTbl.insert(map<VoiceTuple, UINT16>::value_type(tuple, toneID));
	}
	inline void DelBTreePlayToneRecord(VoiceTuple tuple)
	{
		map<VoiceTuple, UINT16>::iterator it = BTreePlayToneTbl.find(tuple);
		if(it!=BTreePlayToneTbl.end())
			BTreePlayToneTbl.erase(it);
	}	
	inline CCBBase *FindCCBByEID_CID(VoiceTuple &tuple)
	{
		map<VoiceTuple, UINT16>::iterator it = BTreeByTuple.find(tuple);
		return (it==BTreeByTuple.end()) ? NULL: &CCBTable[(*it).second];
	}

	inline CCBBase *FindCCBByUID(unsigned int UID)
	{
		map<UINT32, UINT16>::iterator it = BTreeByUID.find(UID);
		return (it==BTreeByUID.end()) ? NULL: &CCBTable[(*it).second];
	}

	inline CCBBase *FindCCBByL3addr(unsigned int L3addr)
	{
		map<UINT32, UINT16>::iterator it = BTreeByL3addr.find(L3addr);
		return (it==BTreeByL3addr.end()) ? NULL: &CCBTable[(*it).second];
	}

	void showBTree(bool blDetail=false);
//private:

	VoiceCCB CCBTable[VOICE_CCB_NUM];
	list<UINT16> FreeCCBList;
	map<VoiceTuple, UINT16> BTreeByTuple;
	map<UINT32, UINT16> BTreeByUID;
	map<UINT32, UINT16> BTreeByL3addr;
	map<VoiceTuple, UINT16> BTreePlayToneTbl;
	
};

/*===========================================================================*/
//VoiceTrans
class VoiceTrans: public FSMTransition
{
public:
	unsigned short m_ID;
	static char m_transName[MAX_VOICE_TRANS][40];
	VoiceTrans(FSMStateIndex target, unsigned short id): FSMTransition(target), m_ID(id){};
	virtual FSMStateIndex Action(VoiceCCB& ccb, CMessage &msg) = 0;
#ifdef DSP_BIOS
	virtual FSMStateIndex Action1(CCBBase &ccb, CMessage &msg) 
#else
	virtual FSMStateIndex Action(CCBBase &ccb, CMessage &msg) 
#endif
	{
#if 0		
		//VoiceCCB voiceCCB(ccb);
		const type_info& t = typeid(*this);
#ifdef __WIN32_SIM__		
		LOG2(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "%s::Action, CCB[UID=0x%08X] ", (int)t.name()+6, ((VoiceCCB&)ccb).getUID());
#else
		LOG2(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "%s::Action, CCB[UID=0x%08X] ", (int)t.name()+2, ((VoiceCCB&)ccb).getUID());
#endif
#endif
		if(m_ID<MAX_VOICE_TRANS)
		{
			LOG2(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"%s::Action, CCB[UID=0x%08X] ", 
				(int)m_transName[m_ID], ((VoiceCCB&)ccb).getUID());
		}
		return Action((VoiceCCB&)ccb,msg);
	};

};

/*===========================================================================*/
//all state: idle, O-Establish, T-Establish, Transparent, Release, Paging, Probe

class VoiceStateBase : public FSMState
{
public:
	VoiceStateBase(FSMStateIndex state):m_state(state){};
	//virtual FSMTransIndex  ParseEvent(CMessage& msg);
#ifdef DSP_BIOS
	virtual const UINT8 * GetStateName (void) const { return (UINT8*)&m_stateName[m_state][0]; };
#else
	virtual const UINT8 *const GetStateName (void) const { return (UINT8*)&m_stateName[m_state][0]; };
#endif
	virtual void Entry(CCBBase& ccb);
	virtual void Exit(CCBBase& ccb);

	static char m_stateName[MAX_VOICE_STATE+1][20];
protected:
	FSMStateIndex	m_state;
private:
	
};

class VoiceIdleState : public VoiceStateBase
{
public:
	VoiceIdleState():VoiceStateBase(VOICE_IDLE_STATE) {};
	virtual void				Entry(CCBBase& ccb);  
	FSMTransIndex				DoParseEvent(CMessage& msg);
};

class VoiceOEstablishState : public VoiceStateBase
{
public:
	VoiceOEstablishState():VoiceStateBase(VOICE_O_ESTABLISH_STATE) {};
	FSMTransIndex				DoParseEvent(CMessage& msg);
};

class VoiceTEstablishState : public VoiceStateBase
{
public:
	VoiceTEstablishState():VoiceStateBase(VOICE_T_ESTABLISH_STATE) {};
	FSMTransIndex				DoParseEvent(CMessage& msg);
};

class VoiceTransparentState : public VoiceStateBase
{
public:
	VoiceTransparentState():VoiceStateBase(VOICE_TRANSPARENT_STATE) {};
	FSMTransIndex				DoParseEvent(CMessage& msg);
};

class VoiceReleaseState : public VoiceStateBase
{
public:
	VoiceReleaseState():VoiceStateBase(VOICE_RELEASE_STATE) {};
	FSMTransIndex				DoParseEvent(CMessage& msg);
};

class VoicePagingState : public VoiceStateBase
{
public:
	VoicePagingState():VoiceStateBase(VOICE_PAGING_STATE) {};
	FSMTransIndex				DoParseEvent(CMessage& msg);
};

class VoiceProbeState : public VoiceStateBase
{
public:
	VoiceProbeState():VoiceStateBase(VOICE_PROBE_STATE) {};
	FSMTransIndex				DoParseEvent(CMessage& msg);
};

class VoiceWaitSYNCState : public VoiceStateBase
{
public:
	VoiceWaitSYNCState():VoiceStateBase(VOICE_WAITSYNC_STATE) {};
	FSMTransIndex				DoParseEvent(CMessage& msg);
};
/*===========================================================================*/
//all Transition

//------------------idle transition
class Trans_Idle_VACSetupCmd : public VoiceTrans
{
public:
	Trans_Idle_VACSetupCmd(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Idle_LAPagingReq : public VoiceTrans
{
public:
	Trans_Idle_LAPagingReq(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};


//------------------O-Establish transition
class Trans_OEstab_AssignTransResRsp : public VoiceTrans
{
public:
	Trans_OEstab_AssignTransResRsp(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_OEstab_VACRelNotify : public VoiceTrans
{
public:
	Trans_OEstab_VACRelNotify(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_OEstab_TassignTimeout : public VoiceTrans
{
public:
	Trans_OEstab_TassignTimeout(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

//------------------T-Establish transition
class Trans_TEstab_AssignTransResRsp : public VoiceTrans
{
public:
	Trans_TEstab_AssignTransResRsp(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_TEstab_TassignTimeout : public VoiceTrans
{
public:
	Trans_TEstab_TassignTimeout(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_TEstab_VACRel : public VoiceTrans
{
public:
	Trans_TEstab_VACRel(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_TEstab_DeLAPaging: public VoiceTrans
{
public:
	Trans_TEstab_DeLAPaging(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_TEstab_ErrNotifyReq: public VoiceTrans
{
public:
	Trans_TEstab_ErrNotifyReq(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};


//------------------Transparent transition
class Trans_Transparent_RelTransResReq : public VoiceTrans
{
public:
	Trans_Transparent_RelTransResReq(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Transparent_VACRelNotify : public VoiceTrans
{
public:
	Trans_Transparent_VACRelNotify(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Transparent_ModifyVACNotify : public VoiceTrans
{
public:
	Trans_Transparent_ModifyVACNotify(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

//------------------Release transition

class Trans_Release_TrelvacTimeout : public VoiceTrans
{
public:
	Trans_Release_TrelvacTimeout(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Release_ErrNotifyRsp : public VoiceTrans
{
public:
	Trans_Release_ErrNotifyRsp(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Release_TerrrspTimeout : public VoiceTrans
{
public:
	Trans_Release_TerrrspTimeout(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Release_RlsTransResReq : public VoiceTrans
{
public:
	Trans_Release_RlsTransResReq(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Release_TrelresTimeout : public VoiceTrans
{
public:
	Trans_Release_TrelresTimeout(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

//------------------Paging transition
class Trans_Paging_VACSetupRsp : public VoiceTrans
{
public:
	Trans_Paging_VACSetupRsp(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Paging_TvacTimeout : public VoiceTrans
{
public:
	Trans_Paging_TvacTimeout(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Paging_DeLaPaging : public VoiceTrans
{
public:
	Trans_Paging_DeLaPaging(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Paging_VACRelNotify : public VoiceTrans
{
public:
	Trans_Paging_VACRelNotify(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

//------------------Probe transition
class Trans_Probe_ProbeRsp : public VoiceTrans
{
public:
	Trans_Probe_ProbeRsp(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_Probe_TproberspTimeout : public VoiceTrans
{
public:
	Trans_Probe_TproberspTimeout(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

//------------------waitSYNC transition
class Trans_WaitSYNC_TwaisyncTimeout : public VoiceTrans
{
public:
	Trans_WaitSYNC_TwaisyncTimeout(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_WaitSYNC_VACRelNotify : public VoiceTrans
{
public:
	Trans_WaitSYNC_VACRelNotify(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};
class Trans_WaitSYNC_VACSetupCmd : public VoiceTrans
{
public:
	Trans_WaitSYNC_VACSetupCmd(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

class Trans_WaitSYNC_RelTransResReq : public VoiceTrans
{
public:
	Trans_WaitSYNC_RelTransResReq(FSMStateIndex target, unsigned short id): VoiceTrans(target, id) {};
	virtual FSMStateIndex Action(VoiceCCB &ccb, CMessage &msg);
};

/*===========================================================================*/

enum
{
	GRP_PAGING_STATE,		//组呼寻呼状态
	GRP_WORKING_STATE,	//组呼工作状态
	GRP_RELEASE_STATE	//组呼释放状态
};

enum
{
	ResReason_GrpPaging,		//组呼寻呼
	ResReason_LePaging,		//迟后进入寻呼
	ResReason_GrpHandover,	//组呼切换
	ResReason_GrpCpeRequest	//cpe申请，例如话权切换时
};


class GrpCCB;
class AirResource
{
public:
	AirResT	airRes;						//组广播信道资源
	GrpCCB* pGrpCCB;
	//设置资源分配标记
	void setResReadyFlag(bool flag);
	bool isResReady(){return blResReady;};				//是否分配了资源
	void setResReason(UINT8 reason);
	UINT8 getResReason(){return ResReason;};
	void setGrpDataDetectFlag(bool flag){blGrpDataDetectFlag=flag;};
	bool getGrpDataDetectFlag(){return blGrpDataDetectFlag;};
private:
	bool blResReady;				//是否已经分配了广播信道资源
	UINT8 ResReason;			//资源分配原因
	bool blGrpDataDetectFlag;	//是否检测到集群数据包
};	

typedef struct _statusReportNode
{
	GrpCpeStatusT grpCpeStatus;	
	_statusReportNode* pNext;
} statusReportNodeT;

extern bool g_blUseStatusReport;
extern statusReportNodeT  g_StatusReportPool[STATUSREPORT_POOLSIZE];	
extern statusReportNodeT* g_freeStatusReportLst;
#define M_StatusReport_CountPeriod	(4)
class StatusReport
{
public:
	StatusReport():curCounter(0),cpesCnt(0),grpStatusList(NULL)
		{memset(GrpCpesCounters,0,sizeof(GrpCpesCounters));};
	UINT8	curCounter;				//当前使用的计数器索引
	UINT16	GrpCpesCounters[M_StatusReport_CountPeriod];		//最近4个状态周期的接收方计数
	void clear();						//清除全部计数，重新初始化
	void clearLst();					//清除状态报告列表，每状态报告周期清空一次
	void showInfo(char* txtMark=NULL);
	statusReportNodeT* getFreeNode();
	bool addStatusReportItem(GrpCpeStatusT item);
	UINT16 cpesCnt;					//当前cpe数目
	statusReportNodeT* grpStatusList;	 //基站下组成员状态列表
	GrpCCB* pGrpCCB;
	void sendStatusReport2SXC();
};

class GrpCCBTable;
class GrpCCB: public CCBBase
{
public:
	GrpCCB(CCBBase &) {initMembers();};
	GrpCCB():CCBBase(GRP_RELEASE_STATE){initMembers();};
	//GrpCCB(){initMembers();};
	
	void initMembers();
	bool isGrpEmpty();	
	void markGrpNotEmpty();
	bool ifUseStatusReport(){return blStatusReportFlag;};
	void setUseStatusReportFlag(bool flag){blStatusReportFlag=flag;};
	UINT8 getTransID(){return m_transID;};
	void setTransID(UINT8 transID){m_transID=transID;};
	void incTransID(){++m_transID;};
	UINT16 getGID(){return GID;};
	void setGID(UINT16 gid){GID = gid;};
	UINT32 getGrpL3Addr(){return grpL3Addr;};
	void setGrpL3Addr(UINT32 grpL3){grpL3Addr = grpL3;};
	UINT16 getGrpSize(){return grpSize;};
	void setGrpSize(UINT16 size){grpSize = size;};
	UINT8 getCommType(){return commType;};
	void setCommType(UINT8 val){commType=val;};
	UINT8 getPrioty(){return prioty;};
	void setPrioty(UINT8 val){prioty=val;};
	UINT8 getEncryptFlag(){return encryptFlag;};
	void setEncryptFlag(UINT8 val){encryptFlag=val;};
	inline UINT16 getTabIndex(){ return m_index;}
	inline void setTabIndex(UINT16 index){ m_index = index;}
	void setCCBTable(GrpCCBTable* ccbTab){ m_pGrpTbl = ccbTab; }
	GrpCCBTable* getCCBTable(){ return m_pGrpTbl;}
	void setLePagingStartFlag(bool flag){blLEPagingStarted=flag;}
	bool isLePagingStarted(){return blLEPagingStarted;}
	void setEncryptKey(UINT8 *pKey){if(pKey){memcpy(m_EncryptKey, pKey, sizeof(m_EncryptKey));}};
	UINT8* getEncryptKey(){return m_EncryptKey;};

	void initCCB();	//初始化
	void CCBClean();	//清除操作
	//定时器
	void initAllTimers();	//初始化所有定时器
	void clearAllTimers();	//清除所有定时器
	void startGrpTimer(UINT8 timerID);
	void stopGrpTimer(UINT8 timerID);
	void deleteGrpTimer(UINT8 timerID);

	//发送消息
	//to L2
#ifdef M_SYNC_BROADCAST
	bool sendGrpMBMSGrpResIndication2L2(CMessage& msg);
#endif//M_SYNC_BROADCAST	
	bool sendGrpResReq2L2(ENUM_GrpResOptType optFlag,UINT8 needRspFlag,UINT8 reason,UINT8 transID,
			ENUM_ReportIDFlag statusReportIndexFlag,UINT32 eid,UINT32 btsID,UINT8 cid=DEFAULT_CID);
	bool sendGrpSignal2L2(UINT8 *pPayload,UINT16 len);
	bool sendGrpPagingReq2L2(UINT8 pagingType, UINT8 LEFlag, UINT8 transID);
	//to SXC
	bool sendGrpPagingRsp2SXC(UINT8 Result, UINT32 UID);
	bool sendGrpResReq2SXC(UINT32 UID, UINT32 PID);
	bool sendGrpHandoverReq2SXC(UINT32 UID, UINT32 PID, UINT32 curBTSID);
	bool sendGrpStatusReport2SXC();
	//to other BTS
#if 0 
	bool sendHoResReq2otherBTS(CMessage& HoResReq);
#endif
	bool sendHoResRsp2otherBTS(CMessage& GrpResCfm);
	//to cpe
	bool sendGrpCallInd2CPE(UINT8 type, UINT8 LEFlag, UINT8 transID);
	bool sendHoResRsp2CPE(CMessage& HoResRsp);
	bool sendGrpShareResRsp2CPE(CMessage& GrpResCfm);
	bool sendGrpRelease2CPE(UINT8 reason);
	
	AirResource airRes;		//空口广播资源
	StatusReport statusReport;	//状态报告
	UINT32	utmLenStatusReport;	//状态报告周期ms
	UINT32	utmLenLePagingLoop;	//迟后进入周期ms

	void showCCBInfo();
private:
	bool		blLEPagingStarted;
	UINT16	m_index;
	GrpCCBTable* m_pGrpTbl;
	bool		blStatusReportFlag;		//是否有状态报告
	UINT16	GID;
	UINT16	grpSize;
	UINT32	grpL3Addr;

	UINT8	commType;			//Communication Type	1	M		
	UINT8	prioty;				//Call priority	1	M		呼叫优先权
	UINT8	encryptFlag;			//Encryption Flag	1	M		端到端加密
	UINT8	m_transID;		//transID
	UINT8	m_EncryptKey[M_ENCRYPT_KEY_LENGTH];
	
	CTimer	*pTimerStatus;
	CTimer	*pTimerLePaging;
	CTimer	*pTimerGrpPagingRsp;
	CTimer	*pTimerGrpLePagingRsp;
	CTimer	*pTimerRes;	//延迟释放广播信道资源的定时器,当bts下无用户收到切换申请组呼资源的情况下使用
	CTimer	*pTimerToneDetect;
	CTimer	*pTimerLePagingStart;
	CTimer	*pTimerGrpRls;
};

class GrpCCBTable: public CCBTableBase
{
public:
	GrpCCBTable(){init();};
	inline GrpCCB * AllocGrpCCB();
	inline void DeAllocGrpCCB(GrpCCB *pCCB);

	CCBBase *FindCCB(CMessage &msg){return NULL;};

	// 索引表相关
	inline void AddBTreeGID(UINT16 gid, UINT16 index);
	inline void DelBTreeGID(UINT16 gid);
	inline void AddBTreeGrpL3addr(UINT32 grpL3addr, UINT16 index);
	inline void DelBTreeGrpL3addr(UINT32 grpL3addr);
	inline GrpCCB *FindCCBByGID(UINT16 GID);
	inline GrpCCB *FindCCBByGrpL3addr(unsigned int grpL3addr);

	UINT32 getActiveGrpNum();
	void init();
	void showGrpInfo(bool blDetail=false);
//private:

	GrpCCB grpCCBTbl[GRP_CCB_NUM];	
	list<UINT16> FreeGrpCCBList;
	map<UINT16, UINT16> BTreeByGID;			// GID索引表
	map<UINT32, UINT16> BTreeByGrpL3addr;	// grpL3索引表
};

/*===========================================================================*/

typedef struct _BeartHeartOptions_
{
	TID m_LinkTID;
	UINT8 m_BeartHeartSeq;
	UINT32 m_BeartHeartUnReply;
	bool m_blShouldSendBeartHeart;
}BeartHeartOptionsT;

class BeartHeartCtrl
{
public:
	BeartHeartCtrl();
	~BeartHeartCtrl(){};

	BeartHeartOptionsT m_MasterSAG;
	BeartHeartOptionsT m_BackupSAG;

	bool isFromMasterSag(CMessage& msg);
	bool isFromBackupSag(CMessage& msg);

	void init();	
	void whenRcvOtherSignalMsg(CMessage& msg);
	inline void handleBeatHeartMsg(CMessage& msg){sendBeartHeartAck(msg);};
	void handleBeatHeartAckMsg(CMessage& msg);
	void handleBeatHeartTmoutMsg(CMessage& msg);

	bool sendBeartHeart(BeartHeartOptionsT& opt);
	bool sendBeartHeartAck(CMessage& msg);
	bool resetBtsSagLink(BeartHeartOptionsT& opt);
    //new stat
    void GetPerfDataNew(UINT32 *pData)
    {
        UINT32 *ptr = pData;
        *ptr++ = m_MSAGBeatHeartTimeoutNum;
        if(m_MSAGBeatHeartNum!=0)
            *ptr++ = (m_MSAGBeatHeartDelayTotal / m_MSAGBeatHeartNum) * 10;//ms  
        else
            *ptr++ = 0;
        *ptr++ = m_SSAGBeatHeartTimeoutNum;
        if(m_SSAGBeatHeartNum!=0)
            *ptr++ = (m_SSAGBeatHeartDelayTotal / m_SSAGBeatHeartNum) * 10;//ms       
        else
            *ptr++ = 0;
    }
    void ClearPerfDataNew()
    {
        //clear data
        m_MSAGBeatHeartTimeoutNum = 0;
        m_MSAGBeatHeartDelayTotal = 0;
        m_MSAGBeatHeartNum = 0;
        m_MSAG10msTickSNum = 0;
        m_SSAGBeatHeartTimeoutNum = 0;
        m_SSAGBeatHeartDelayTotal = 0;
        m_SSAGBeatHeartNum = 0;
        m_SSAG10msTickSNum = 0;
        
    }
    //new stat
    UINT32 m_MSAGBeatHeartTimeoutNum;
    UINT32 m_MSAGBeatHeartDelayTotal;
    UINT32 m_MSAGBeatHeartNum;
    UINT32 m_MSAG10msTickSNum;
    UINT32 m_SSAGBeatHeartTimeoutNum;
    UINT32 m_SSAGBeatHeartDelayTotal;
    UINT32 m_SSAGBeatHeartNum;// 记录发送心跳的次数
    UINT32 m_SSAG10msTickSNum;// 记录当前发送心跳的10ms tick
    UINT32 m_MSAGBeatHeartNo;//记录当前发送心跳的序号
    UINT32 m_SSAGBeatHeartNo;//记录当前发送心跳的序号
};

//VoiceFSM
class VoiceFSM: public FSM
{
public:

	VoiceFSM();
	UINT8 GetStateNumber() { return  MAX_VOICE_STATE ; }
	UINT8 GetTransNumber() { return  MAX_VOICE_TRANS; }
	CCBBase *FindCCB(CMessage &msg) { return CCBTable->FindCCB(msg);};
	bool init();
	void Parse_Handle_Event(CMessage &msg);
	UINT8 getSAGCongestLevel(){ return m_RemoteCongestLevel; }
	void showBTree(bool blDetail=false);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	void showVideoUTList();
#endif
	VoiceCCBTable *CCBTable;
	GrpCCBTable *pGrpCCBTbl;
	//GrpCCBTable GrpCCBTbl;
	//new stat
	void GetVoicePerfDataNew(UINT32 *pData){m_BeatHeart_Manager.GetPerfDataNew(pData);}
    void ClearVoicePerfDataNew(){m_BeatHeart_Manager.ClearPerfDataNew();}
private:
	BeartHeartCtrl m_BeatHeart_Manager;

	UINT32	m_SAGID;	//host order
	UINT32	m_BTSID;	//host order
	UINT8	m_LocalCongestLevel;
	UINT8	m_RemoteCongestLevel;

	void sagPlayTone();
	void addPlayToneItem(CMessage& msg);
	void delPlayToneItem(CMessage& msg);
	void handleTelNORegMsg(CMessage& msg);
	void handleSAbis1IFResetMsg(CMessage& msg);
	void handleMakeAllVCpesRegVSrvMsg(CMessage& msg);
	void handleRelVoiceSrvMsg(CMessage& msg);
	void handleVoicePortReg(CMessage& msg);
	void handleVoicePortUnReg(CMessage& msg);
	void handleVoicePortMoveAway(CMessage& msg);
	void handleBeatHeartTimeout(CMessage& msg);
	void handleCongestionTimeout(CMessage& msg);
	void handleVCRCallSignal(CMessage& msg);
	void handleVACCallSignal(CMessage& msg);
	void handleDACSignal(CMessage& msg);
	void checkVacCtrlMsg(CMessage& msg);
	void checkCCBSrvStatus();
	void handleVoiceDataFromVDR(CMessage& msg);
	void handleVoiceDataFromVAC(CMessage& msg);
	void handleFaxDataFromSAG(UINT8 *pData, VoiceCCB *pCCB);
	void handleUplinkFaxData(CMessage& msg);
	void TxUlFaxData(UINT16 i);
	bool sendFaxDataToSag(UINT16 i);
	void sendFaxDataToL2(UINT16 i);
	void handleLAPagingReq(CMessage& msg);
	void handleDELAPagingReq(CMessage& msg);
	void handleReset(CMessage& msg);
	void handleBeatHeart(CMessage& msg);
	void handleCongestReq(CMessage& msg);
	void handleCongestRsp(CMessage& msg);
	void handleVCR_UTSAG_Msg_toVAC(CMessage& msg);
	void handleVCR_UTSAG_Msg_toDAC(CMessage& msg);
	void handleL2CongestionReportMsg(CMessage& msg);
	void handleSetCfgMsg(CMessage& msg);
	void handleVCR_UTSAG_OAM_Msg_toDAC(CMessage& msg);
	void handleULDACCpeOamMsg(CMessage& msg);
	void handleVCR_UT_DL_Relay_Msg(CMessage& msg);
	void handleULCpeRelayMsg(CMessage& msg);
	//yhw
	void handleBroadcast_SM(CMessage& msg);
	void handleSendRejectToSAG(ENUM_REJECTRESN );
	void handleResetReq(CMessage& msg);
	//edit by yhw

//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
	void handle_DVoiceConfigReq(CMessage& msg);
	void handle_RelTransResReq(CMessage& msg);
#endif
//20090531 fengbing bts inner switch for Voice Data end

	void Voice10msProc();
	bool SendVoiceDataToSAG(CComMessage* pComMsg);

	void handleNatApTimerMsg(CMessage& msg);
	void reloadVoiceCfg();
	//集群相关----------------------------------------------------------
	//from L2
	void handle_ResCfm_frmL2(CMessage& msg);
	void handle_StatusReportInd_frmL2(CMessage& msg);
	void handle_GrpPagingRsp_frmL2(CMessage& msg);
	void handle_L2L3BtsPttPressReq(CMessage& msg);
	void handle_L2L3BtsPttPressCancel(CMessage& msg);
	void handle_L2L3BtsPttPressRelease(CMessage& msg);
#ifdef M_SYNC_BROADCAST
	void handle_BtsL2SxcULMsg_frmL2(CMessage& msg);
	void handle_L2L3MBMSMediaDelayRsp(CMessage& msg);
#endif//M_SYNC_BROADCAST		
	//from SXC
	void handle_LAGrpPaging_frmSXC(CMessage& msg);
	void handle_LEGrpPaging_frmSXC(CMessage& msg);
	void handle_DeLEGrpPaging_frmSXC(CMessage& msg);
	void handle_GrpHandoverRsp_frmSXC(CMessage& msg);
	void handle_GrpResRsp_frmSXC(CMessage& msg);
	void handle_ErrorNotify_frmSXC(CMessage& msg);
	void handle_GrpL3Addr_Signal_frmSXC(CMessage& msg);
	void handle_GrpUID_Signal_frmSXC(CMessage& msg);
	void handle_GrpPttPressApplyRsp_frmSXC(CMessage& msg);	
#ifdef M_SYNC_BROADCAST
	void handle_BtsL2SxcDLMsg_frmSXC(CMessage& msg);
	void handle_SyncBroadcastData(CMessage& msg);
#endif//M_SYNC_BROADCAST
	//from CPE
	void handle_GrpL3Addr_signal_frmCPE(CMessage& msg);
	void handle_GrpUID_signal_frmCPE(CMessage& msg);
	void handle_HoResReq_frmCPE(CMessage& msg);
	void handle_GrpResReq_frmCPE(CMessage& msg);
	void handle_GrpDacPttPressReq(CMessage& msg);	
	//from other BTS
	void handle_HoResReq_frmOtherBTS(CMessage& msg);
	void handle_HoResRsp_frmOtherBTS(CMessage& msg);
	//timers
	void handle_Timeout_GrpPagingRsp(CMessage& msg);
	void handle_Timeout_LePagingRsp(CMessage& msg);
	void handle_Timeout_StatusReport(CMessage& msg);
	void handle_Timeout_LePagingLoop(CMessage& msg);
	void handle_Timeout_ResClear(CMessage& msg);
	void handle_Timeout_LePagingStart(CMessage& msg);
	void handle_Timeout_GrpDataDetect(CMessage& msg);
	void handle_Timeout_GrpRls(CMessage& msg);
	//拒绝组呼
	bool refuseGrpPagingRsp2SXC(CMessage& msg);
	//向sxc发送话权申请信令PttPressApplyReq
	bool send_grpPttPressApplyReq2SXC(UINT32 uid, UINT16 gid, UINT8 prio, UINT8 encryptCtrl, UINT16 sessionType);
	//如果组不存在，向终端回应获取资源请求
	//bool sendGrpResRsp2CPE(CMessage& GrpResReq, UINT8 result=1/*0-success;1-fail*/);
	bool sendGrpResRsp2CPE(CMessage& sxcGrpResRsp, bool blUseSXCResult=true, UINT8 result=1);
	bool sendGrpHandoverRsp2otherBTS(CMessage& sxcGrpHandoverRsp, bool blUseSXCResult=true, UINT8 result=1);
	//GrpVersion003 begin
	bool sendGrpResReq2SXC(CMessage& cpeGrpResReq);
	bool sendGrpHandoverReq2SXC(CMessage& cpeHandoverReq);
	//GrpVersion003 end
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	list<UINT32> m_VideoUTList;
	void registerVideoUT(UINT32 uid);
	void unRegisterVideoUT(UINT32 uid);
	bool isVideoUT(UINT32 uid);
#endif
	//集群相关----------------------------------------------------------
	//数据广播、单播相关begin
	void handle_Msg_frmDCS(CMessage& msg);
	void handle_DB_Signal_frmDCS(CMessage& msg);
	void handle_DB_Data_frmDCS(CMessage& msg);
	void handle_DB_UIDMsg_frmDCS(CMessage& msg);
	
	void handle_DB_Signal_frmCPE(CMessage& msg);
	void handle_DB_Data_frmCPE(CMessage& msg);
	//数据广播、单播相关end	
};


/*===========================================================================*/

////////////////////////////////////////////////////////////////////////////////
//spyVData---begin
////////////////////////////////////////////////////////////////////////////////

class VoiceDataSpy
{
public:
	VoiceDataSpy();
	void setDiagUID(UINT32 uid);
	void setDiagGID(UINT16 gid);
	UINT32 getDiagUID(){return diagUID;};
	UINT16 getDiagGID(){return diagGID;};	
	bool isShowPeriodic(){return blShowPeriodic;};
	void setShowPeriodicFlag(bool flag){blShowPeriodic=flag;};
	UINT32 getFrameID(){return uFrameID;};
	void setFrameID(UINT32 uFN){uFrameID=uFN;};
	void incFN(){++uFrameID;};
	
	bool isSpyUidVoiceData(){ return (diagUID!=0 && diagUID!=0xffffffff); };
	bool isSpyGidVoiceData(){ return (diagGID!=0 && diagGID!=0xffff); };

	void clearUidResult();
	void clearGidResult();

	void showDiagResult();

	bool isSpyULUidVDataTS(){return blLogULUidVDataTS;};
	bool isSpyDLUidVDataTS(){return blLogDLUidVDataTS;};
	bool isSpyGidVDataTS(){return blLogGidVDataTS;};
	void logULUidVDataTS(bool flag){blLogULUidVDataTS=flag;};
	void logDLUidVDataTS(bool flag){blLogDLUidVDataTS=flag;};
	void logGidVDataTS(bool flag){blLogGidVDataTS=flag;};

	void logULUidFaxData(bool flag){blLogULUidFaxData=flag;};
	void logDLUidFaxData(bool flag){blLogDLUidFaxData=flag;};
	bool isSpyULUidFaxData(){return blLogULUidFaxData;};
	bool isSpyDLUidFaxData(){return blLogDLUidFaxData;};
	
	//UID
	UINT32 uCntUID10msVDataFromSAG;
	UINT32 uCntUID10msVDataToVAC;

	UINT32 uCntUID10msVDataFromVAC;
	UINT32 uCntUID10msVDataToSAG;		
	//GID
	UINT32 uCntGID10msVDataFromSAG;
	UINT32 uCntGID10msVDataToVAC;
	//729B, now only downlink
	UINT32 uCntUID10ms729BFromSAG;
	UINT32 uCntUID10ms729BToVAC;
	UINT32 uCntUID10ms729BFromVAC;
	UINT32 uCntUID10ms729BToSAG;

	UINT32 uCntGID10ms729BFromSAG;
	UINT32 uCntGID10ms729BToVAC;
	//G.711A
	UINT32 uCntUID10ms711AFromSAG;
	UINT32 uCntUID10ms711AToL2;
	UINT32 uCntUID10ms711AFromL2;
	UINT32 uCntUID10ms711AToSAG;
	UINT32 uCntUID10ms711AFromL2Lost;
	
private:
	UINT32 diagUID;		
	UINT32 diagGID;
	bool blShowPeriodic;
	UINT32 uFrameID;

	bool blLogULUidVDataTS;
	bool blLogDLUidVDataTS;
	bool blLogGidVDataTS;
	bool blLogULUidFaxData;
	bool blLogDLUidFaxData;
};

extern VoiceDataSpy g_SpyVData;

extern "C" void spyVDataSetUid(UINT32 uid);
extern "C" void spyVDataSetGid(UINT16 gid);
extern "C" void spyVDataShow();
extern "C" void spyVDataStart();
extern "C" void spyVDataStop();

extern "C" void logULUidVDataTS(bool flag);
extern "C" void logDLUidVDataTS(bool flag);
extern "C" void logGidVDataTS(bool flag);
extern "C" void logULUidFaxData(bool flag);
extern "C" void logDLUidFaxData(bool flag);

////////////////////////////////////////////////////////////////////////////////
//spyVData---end
////////////////////////////////////////////////////////////////////////////////

#endif //__VOICE_FSM_H


