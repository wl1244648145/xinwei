/*******************************************************************************
* Copyright (c) 2009 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagFsmCfg.h
* Create Date    : 13-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/

#ifndef	__LOCALSAGFSMCFG_H
#define	__LOCALSAGFSMCFG_H

#include "message.h"

#include "localSagCCB.h"


//个呼状态机，暂时不支持集群讲话方

typedef enum
{
	SAG_IDLE_STATE,		//waiting fro assignResReq(OCall) or InnerSetup(内部生成)
	SAG_O_SETUP_STATE,	//waiting for setup
	SAG_DIAL_STATE,		//dial number
	SAG_O_ALERTING_STATE,//waiting for alerting
	SAG_RINGBACK_STATE,//waiting for connect
	SAG_PAGING_STATE,	//waiting for LapagingRsp
	SAG_T_ASSIGNRES_STATE,//waiting for assignResReq
	SAG_T_SETUP_STATE,	//waiting for setupAck
	SAG_T_ALERTING_STATE,//waiting for alerting
	SAG_RING_STATE,		//ringing,waiting for connect
	SAG_CONNECT_STATE,	//connected
	SAG_WAIT_ONHOOK_STATE,//waiting fro disconnect(on hook)
	SAG_RELEASE_STATE	,	//release sent,waiting release complete to release transport res
//GrpVoiceCall Srv begin	
	SAG_GRP_CALLSETUP_STATE,	//GrpSetup received,waiting for listenners to join
	SAG_GRP_TALKING_STATE,//talking
//GrpVoiceCall Srv end
	SAG_STATE_COUNT
}SAG_CPE_STATE;

enum
{
	IDLE__ULSignal__AssignResReq,
	IDLE__INSignal__Setup,
	O_SETUP__ULSignal__PttSetupReq,
	O_SETUP__ULSignal__Setup,
	O_SETUP__Timeout__OSetup,
	O_SETUP__ULSignal__ErrNotiReq,
	O_SETUP__INSignal__Disconnect,
	DIAL__ULSignal__DialNumber,
	DIAL__Timeout__DialNumber,
	DIAL__ULSignal__Disconnect,
	DIAL__ULSignal__ErrNotiReq,
	DIAL__INSignal__Disconnect,
	O_ALERTING__INSignal__Alerting,
	O_ALERTING__Timeout__Alerting,
	O_ALERTING__ULSignal__Disconnect,
	O_ALERTING__ULSignal__ErrNotiReq,
	O_ALERTING__INSignal__Disconnect,
	RINGBACK__INSignal__Connect,
	RINGBACK__Timeout__Connect,
	RINGBACK__ULSignal__Disconnect,
	RINGBACK__ULSignal__ErrNotiReq,
	RINGBACK__INSignal__Disconnect,
	PAGING__ULSignal__LapagingRsp,
	PAGING__Timeout__LapagingRsp,
	PAGING__INSignal__Disconnect,
	T_ASSIGNRES__ULSignal__AssignResReq,
	T_ASSIGNRES__Timeout__AssignResReq,
	T_ASSIGNRES__ULSignal__ErrNotiReq,
	T_ASSIGNRES__INSignal__Disconnect,
	T_SETUP__ULSignal__SetupAck,
	T_SETUP__Timeout__SetupAck,
	T_SETUP__ULSignal__ErrNotiReq,
	T_SETUP__INSignal__Disconnect,
	T_ALERTING__ULSignal__Alerting,
	T_ALERTING__Timeout__Alerting,
	T_ALERTING__ULSignal__ErrNotiReq,
	T_ALERTING__INSignal__Disconnect,
	RING__ULSignal__Connect,
	RING__Timeout__Connect,
	RING__ULSignal__ErrNotiReq,
	RING__INSignal__Disconnect,
	RING__ULSignal__Disconnect,//用户拒接
	CONNECT__ULSignal__Disconnect,
	CONNECT__ULSignal__ErrNotiReq,
	CONNECT__INSignal__Disconnect,
	CONNECT__INSignal__TConnectAck,//被叫
	CONNECT__Timeout__TConnectAck,//被叫
	CONNECT__ULSignal__OConnectAck,//主叫
	CONNECT__Timeout__OConnectAck,//主叫
	WAITONHOOK__ULSignal__Disconnect,
	WAITONHOOK__Timeout__Disconnect,
	WAITONHOOK__ULSignal__ErrNotiReq,
	RELEASE__ULSignal__ReleaseComplete,
	RELEASE__Timeout__ReleaseComplete,
	RELEASE__ULSignal__ErrNotiReq,
	
	GRP_CALLSETUP__ULSignal__GrpDisconnect,
	GRP_CALLSETUP__ULSignal__ErrNotiReq,
	GRP_CALLSETUP__Timeout__PttConnect,
	GRP_CALLSETUP__INSignal__PttConnect,
	GRP_CALLSETUP__INSignal__GrpCallingRelease,//-
	GRP_TALKING__ULSignal__GrpDisconnect,	
	GRP_TALKING__ULSignal__PttRls,
	GRP_TALKING__ULSignal__ErrNotiReq,
	GRP_TALKING__INSignal__GrpCallingRelease,//最长讲话时间到//最长组呼存在时间到等原因释放组呼
	GRP_TALKING__INSignal__PttInterrupt,

	SAG_CALL_FSM_END,
	SAG_CALL_FSM_COUNT=SAG_CALL_FSM_END
};

typedef struct tagEventActionItem
{
	UINT16	messageID;
	UINT16	signalType;
	UINT16	timerType;
	UINT16	TransIndex;
}EventActionItemT;
typedef SAG_CPE_STATE (*sagCallFsmProcPtr)(CMessage& msg, CCCB& ccb);

extern const EventActionItemT*	pEventAction[];
extern sagCallFsmProcPtr pCallFsmFunc[SAG_CALL_FSM_COUNT];
extern char callFsmFuncName[SAG_CALL_FSM_COUNT][50];
extern char callFsmStateName[SAG_STATE_COUNT][40];

SAG_CPE_STATE IDLE__ULSignal__AssignResReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE IDLE__INSignal__Setup__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_SETUP__ULSignal__PttSetupReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_SETUP__ULSignal__Setup__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_SETUP__Timeout__OSetup__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_SETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_SETUP__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE DIAL__ULSignal__DialNumber__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE DIAL__Timeout__DialNumber__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE DIAL__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE DIAL__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE DIAL__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_ALERTING__INSignal__Alerting__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_ALERTING__Timeout__Alerting__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_ALERTING__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_ALERTING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE O_ALERTING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RINGBACK__INSignal__Connect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RINGBACK__Timeout__Connect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RINGBACK__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RINGBACK__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RINGBACK__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE PAGING__ULSignal__LapagingRsp__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE PAGING__Timeout__LapagingRsp__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE PAGING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_ASSIGNRES__ULSignal__AssignResReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_ASSIGNRES__Timeout__AssignResReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_ASSIGNRES__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_ASSIGNRES__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_SETUP__ULSignal__SetupAck__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_SETUP__Timeout__SetupAck__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_SETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_SETUP__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_ALERTING__ULSignal__Alerting__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_ALERTING__Timeout__Alerting__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_ALERTING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE T_ALERTING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RING__ULSignal__Connect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RING__Timeout__Connect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RING__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RING__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE CONNECT__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE CONNECT__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE CONNECT__INSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE CONNECT__INSignal__TConnectAck__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE CONNECT__Timeout__TConnectAck__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE CONNECT__ULSignal__OConnectAck__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE CONNECT__Timeout__OConnectAck__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE WAITONHOOK__ULSignal__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE WAITONHOOK__Timeout__Disconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE WAITONHOOK__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RELEASE__ULSignal__ReleaseComplete__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RELEASE__Timeout__ReleaseComplete__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE RELEASE__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_CALLSETUP__ULSignal__GrpDisconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_CALLSETUP__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_CALLSETUP__Timeout__PttConnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_CALLSETUP__INSignal__PttConnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_CALLSETUP__INSignal__GrpCallingRelease__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_TALKING__ULSignal__GrpDisconnect__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_TALKING__ULSignal__PttRls__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_TALKING__ULSignal__ErrNotiReq__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_TALKING__INSignal__GrpCallingRelease__Proc(CMessage& msg, CCCB& ccb);
SAG_CPE_STATE GRP_TALKING__INSignal__PttInterrupt__Proc(CMessage& msg, CCCB& ccb);

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAGFSMCFG_H */


