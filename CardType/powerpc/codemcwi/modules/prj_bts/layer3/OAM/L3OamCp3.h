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

#ifndef _INC_L3OAMCPESTRANS
#define _INC_L3OAMCPESTRANS

#include "L3OamCpeState.h"
#include "L3L3L2CpeRegNotify.h"
#include "L3L3CpeProfUpdateReq.h"
#include "L3oamCpeRegNotify.h"
#include "L3oamCfgCommon.h"


class CPETrans: public FSMTransition
{
public:
    CPETrans(FSMStateIndex target): FSMTransition(target) {};
    virtual FSMStateIndex Action(CPECCB& ccb, CMessage &msg) = 0;
    virtual FSMStateIndex Action(CCBBase &ccb, CMessage &msg) 
    {
        return Action((CPECCB&)ccb, msg);
    }
    static bool SendDisableToDM_Voice(CPECCB &,UINT16 usMsgId, UINT32 eid, UINT16 = 0);
    static void CPE_StopTransaction(CPECCB &rCCB);
protected:
    bool SendProfDatatoOther(const CPECCB &, UINT8 = 0);
    bool SendEidMoneyStatustoEbCdr(UINT32 ,UINT8 );
    bool SendMoveAwaytoEbCdr(UINT16,UINT32);
    bool GetTosInfo(T_CpeToSCfgEle*, UINT16&, UINT8);    
    bool CreateNoPrifileIEProfUpdateReqToCpe(CL3CpeProfUpdateReq &UpdateReq, UINT8 Ut_type, UINT32 );
    bool CreateProfUpdateReqToCpe(CL3CpeProfUpdateReq &, const CPECCB&);
    bool CreateDefaultProfUpdateReqToCpe(CL3CpeProfUpdateReq &UpdateReq,UINT8 ut_type, UINT32, UINT8 AdminState, UINT8=0);
    UINT16 getBtsIDs(UINT32 *, UINT16 &);
    bool CreateCpeNotifyToEms(CCpeRegistNotify&, CL3L2CpeRegistNotify&);
    bool ProfUpdateRspToEms(UINT16);
    bool stopUpdUTProfileTransaction(CPECCB &,UINT8 flag );
    bool stopBWInfoTransaction(CPECCB &);
    bool BeginProfUpdateReqToCpeTrasaction(CL3CpeProfUpdateReq &, CPECCB &rCCB, UINT8 = OAM_REQ_RESEND_CNT3);
    bool BeginCpeRegNotifyToEmsTrasaction(CCpeRegistNotify &);
////bool IsProfDataSame(SINT8 *ccbData, SINT8 *MsgDate, UINT16);//判断请求消息重的profile是否同ccb保存的相同    
    FSMStateIndex RegisterProcedure(CPECCB&, CL3L2CpeRegistNotify&);
    CTimer* createRegisterTimer(UINT32 eid);
    FSMStateIndex RegisterReq(CPECCB &, const CUTRegisterReq &);
    bool sendBWInfoReq(CPECCB &);
    bool sendModifyBWInfoRsp(const CPECCB &);
    void SendHandoverParaToCpe(UINT32 eid);
    void SendValidFreqsParaToCpe(UINT32 eid);
    void sendDeleteMsgtoUT(UINT32 eid);
    void sendDeleteMsgtoZ( char* p );
    UINT8 m_aucMZDelMsg[18];//发送到CPEZ的删除消息
//#ifdef RPT_FOR_V6
	void sendRptReqToEms(const T_RegisterReq *const msgUTRegisterReq);
};

class CPEanyStateRegisterNotifyTrans: public CPETrans
{
public:
    CPEanyStateRegisterNotifyTrans(FSMStateIndex target): CPETrans  (target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

/*
 *新注册消息的处理
 */
class CPEanyStateRegisterReqTrans: public CPETrans
{
public:
    CPEanyStateRegisterReqTrans(FSMStateIndex target): CPETrans  (target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPENorecordProfileUpdateReqTrans: public CPETrans
{
public:
    CPENorecordProfileUpdateReqTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPENorecordModifyBWInfoReqTrans: public CPETrans
{
public:
    CPENorecordModifyBWInfoReqTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

#if 0
class CPEServingRegisterNotifyTrans: public CPETrans
{
public:
    CPEServingRegisterNotifyTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};
#endif
class CPEServingProfileUpdateReqTrans: public CPETrans
{
public:
    CPEServingProfileUpdateReqTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPEServingModifyBWInfoReqTrans: public CPETrans
{
public:
    CPEServingModifyBWInfoReqTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPEServingMovedawayNotifyTrans: public CPETrans
{
public:
    CPEServingMovedawayNotifyTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPEServingCpeProfileUpdateNotifyTrans: public CPETrans
{
public:
    CPEServingCpeProfileUpdateNotifyTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};
//#ifdef RPT_FOR_V6
class CPEServingRptBWInfoRspTrans: public CPETrans
{                                                                                   
public:                                                                             
    CPEServingRptBWInfoRspTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);                       
};   
class CPEServingRptRfOnOffReqTrans: public CPETrans
{                                                                                   
public:                                                                             
    CPEServingRptRfOnOffReqTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);                       
};    
class CPEServingRptCfgReqTrans: public CPETrans
{                                                                                   
public:                                                                             
    CPEServingRptCfgReqTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);                       
};    
class CPEServingRptGetReqTrans: public CPETrans                                                                                       
{                                                                                   
public:                                                                             
    CPEServingRptGetReqTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);                       
};    
class CPEServingRptRfOnOffRspTrans: public CPETrans                                                                                       
{                                                                                   
public:                                                                             
    CPEServingRptRfOnOffRspTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);                       
}; 
class CPEServingRptCfgRspTrans: public CPETrans                                                                                       
{                                                                                   
public:                                                                             
    CPEServingRptCfgRspTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);                       
};       
class CPEServingRptGetRspTrans: public CPETrans                                                                                       
{                                                                                   
public:                                                                             
    CPEServingRptGetRspTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);                       
}; 
class CPEServingRptTimeoutTrans: public CPETrans                                                                                       
{                                                                                   
public:                                                                             
    CPEServingRptTimeoutTrans (FSMStateIndex target):CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);                       
};    

//Add for Z_Module Register.
class CPE_Z_RegisterNotifyTrans: public CPETrans
{
public:
    CPE_Z_RegisterNotifyTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

#if 0
class CPEWaitforemsRegisterNotifyTrans: public CPETrans
{
public:
    CPEWaitforemsRegisterNotifyTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};
#endif

class CPEWaitforemsProfileUpdateReqTrans: public CPETrans
{
public:
    CPEWaitforemsProfileUpdateReqTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPEWaitforemsModifyBWInfoReqTrans: public CPETrans
{
public:
    CPEWaitforemsModifyBWInfoReqTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPERegisterTimeOutTrans: public CPETrans
{
public:
    CPERegisterTimeOutTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

#if 0
class CPEMovedawayRegisterNotifyTrans: public CPETrans
{
public:
    CPEMovedawayRegisterNotifyTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};
#endif
class CPEMovedawayRecordKeepTimerTrans: public CPETrans
{
public:
    CPEMovedawayRecordKeepTimerTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

#if 0
class CPESwitchoffRegisterNotifyTrans: public CPETrans
{
public:
    CPESwitchoffRegisterNotifyTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};
#endif


class CPESwitchoffProfileUpdateReqTrans: public CPETrans
{
public:
    CPESwitchoffProfileUpdateReqTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPESwitchoffModifyBWInfoReqTrans: public CPETrans
{
public:
    CPESwitchoffModifyBWInfoReqTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPESwitchoffMovedawayNotifyTrans: public CPETrans
{
public:
    CPESwitchoffMovedawayNotifyTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPESwitchoffRecordKeepTimerTrans: public CPETrans
{
public:
    CPESwitchoffRecordKeepTimerTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};

class CPEAnyStateSwitchOffTrans: public CPETrans
{
public:
    CPEAnyStateSwitchOffTrans (FSMStateIndex target): CPETrans(target) {};
    virtual FSMStateIndex Action(CPECCB &ccb, CMessage &msg);
};
#endif
