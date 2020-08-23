/*******************************************************************************
* Copyright (c) 2009 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagFsmCfg.cpp
* Create Date    : 13-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "localSagMsgID.h"
#include "localSagFsm.h"
#include "localSagFsmCfg.h"
#include "voiceCommon.h"

#include "BtsVMsgId.h"

//SAG_IDLE_STATE,		//waiting fro assignResReq(OCall) or InnerSetup(内部生成)
const EventActionItemT	IDLE_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	AssignResReq_MSG,	M_INVALID_SAG_TIMERTYPE,	IDLE__ULSignal__AssignResReq},
	{MSGID_SAG_INNER_SIGNAL,	SAG_INNER_CallArrive_Msg,	M_INVALID_SAG_TIMERTYPE,	IDLE__INSignal__Setup},
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_O_SETUP_STATE,	//waiting for setup
const EventActionItemT	O_Setup_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	PTT_SetupReq_MSG,	M_INVALID_SAG_TIMERTYPE,	O_SETUP__ULSignal__PttSetupReq},
	{MSGID_VOICE_VCR_SIGNAL,	Setup_MSG,	M_INVALID_SAG_TIMERTYPE,	O_SETUP__ULSignal__Setup},
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_O_SETUP,	O_SETUP__Timeout__OSetup},
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	O_SETUP__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	O_SETUP__INSignal__Disconnect},//inner release call
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_DIAL_STATE,		//dial number
const EventActionItemT	O_Dial_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	Information_MSG,	M_INVALID_SAG_TIMERTYPE,	DIAL__ULSignal__DialNumber},
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_O_DIALNUMBER,	DIAL__Timeout__DialNumber},
	{MSGID_VOICE_VCR_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	DIAL__ULSignal__Disconnect},//disconnect(ut onhook)
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	DIAL__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	DIAL__INSignal__Disconnect},//inner release call
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_O_ALERTING_STATE,//waiting for alerting
const EventActionItemT	O_Alerting_EventActionTbl[] = 
{
	{MSGID_SAG_INNER_SIGNAL,	Alerting_MSG,	M_INVALID_SAG_TIMERTYPE,		O_ALERTING__INSignal__Alerting},
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_O_ALERTING,		O_ALERTING__Timeout__Alerting},		
	{MSGID_VOICE_VCR_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	O_ALERTING__ULSignal__Disconnect},//disconnect(ut onhook)
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	O_ALERTING__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	O_ALERTING__INSignal__Disconnect},//inner release call
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_RINGBACK_STATE,//waiting for connect
const EventActionItemT	Ringback_EventActionTbl[] = 
{
	{MSGID_SAG_INNER_SIGNAL,	Connect_MSG,	M_INVALID_SAG_TIMERTYPE,	RINGBACK__INSignal__Connect},//connect
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_O_CONNECT,		RINGBACK__Timeout__Connect},//timeout connect
	{MSGID_VOICE_VCR_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	RINGBACK__ULSignal__Disconnect},//disconnect from caller(ut onhook)
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	RINGBACK__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	RINGBACK__INSignal__Disconnect},//inner signal(disconnect), call rejected //inner release call
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_PAGING_STATE,	//waiting for LapagingRsp
const EventActionItemT Paging_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	LAPagingRsp_MSG,	M_INVALID_SAG_TIMERTYPE,	PAGING__ULSignal__LapagingRsp},//lapagingRsp
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_T_LAPAGING,		PAGING__Timeout__LapagingRsp},//timeout lapagingRsp
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	PAGING__INSignal__Disconnect},//inner signal(disconnect), call canceled(caller onhook) //peer errornotify	//inner release call
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_T_ASSIGNRES_STATE,//waiting for assignResReq
const EventActionItemT	T_AssignRes_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	AssignResReq_MSG,	M_INVALID_SAG_TIMERTYPE,	T_ASSIGNRES__ULSignal__AssignResReq},//assignResReq
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_T_ASSIGN_TRANS_RES,		T_ASSIGNRES__Timeout__AssignResReq},//timeout assignResReq
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	T_ASSIGNRES__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	T_ASSIGNRES__INSignal__Disconnect},//inner signal(disconnect), call canceled(caller onhook) //peer errornotify	//inner release call
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_T_SETUP_STATE,	//waiting for setupAck
const EventActionItemT	T_Setup_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	CallProc_MSG,	M_INVALID_SAG_TIMERTYPE,	T_SETUP__ULSignal__SetupAck},//setupAck
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_T_SETUPACK,	T_SETUP__Timeout__SetupAck},//timeout setupAck
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	T_SETUP__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	T_SETUP__INSignal__Disconnect},//inner signal(disconnect), call canceled(caller onhook) //peer errornotify	//inner release call
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_T_ALERTING_STATE,//waiting for alerting
const EventActionItemT	T_Alerting_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	Alerting_MSG,	M_INVALID_SAG_TIMERTYPE,	T_ALERTING__ULSignal__Alerting},//alerting
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_T_ALERTING,	T_ALERTING__Timeout__Alerting},//timeout alerting
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	T_ALERTING__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	T_ALERTING__INSignal__Disconnect},//inner signal(disconnect), call canceled(caller onhook) //peer errornotify //inner release call
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_RING_STATE,		//ringing,waiting for connect
const EventActionItemT	Ring_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	Connect_MSG,	M_INVALID_SAG_TIMERTYPE,	RING__ULSignal__Connect},//connect
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_T_CONNECT,	RING__Timeout__Connect},//timeout connect
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	RING__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	RING__INSignal__Disconnect},//inner signal(disconnect), call canceled(caller onhook) //peer errornotify //inner release call
	{MSGID_VOICE_VCR_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	RING__ULSignal__Disconnect},//disconnect,用户拒接
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_CONNECT_STATE,	//connected
const EventActionItemT	CONNECT_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	CONNECT__ULSignal__Disconnect},//disconnect
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	CONNECT__ULSignal__ErrNotiReq},//error notify
	{MSGID_SAG_INNER_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	CONNECT__INSignal__Disconnect},//inner signal(disconnect), peer disconnect	//inner release call
	{MSGID_SAG_INNER_SIGNAL,	ConnectAck_MSG,	M_INVALID_SAG_TIMERTYPE,	CONNECT__INSignal__TConnectAck},//TConnectAck
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_T_CONNECTACK,	CONNECT__Timeout__TConnectAck},//Timeout TConnectAck
	{MSGID_VOICE_VCR_SIGNAL,	ConnectAck_MSG,	M_INVALID_SAG_TIMERTYPE,	CONNECT__ULSignal__OConnectAck},//OConnectAck
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_O_CONNECTACK,	CONNECT__Timeout__OConnectAck},//Timeout OConnectAck
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_WAIT_ONHOOK_STATE,//waiting fro disconnect(on hook)
const EventActionItemT	WAITONHOOK_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	WAITONHOOK__ULSignal__Disconnect},//disconnect
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_DISCONNECT,	WAITONHOOK__Timeout__Disconnect},//timeout disconnect
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	WAITONHOOK__ULSignal__ErrNotiReq},//error notify
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};
//SAG_RELEASE_STATE		//release sent,waiting release complete to release transport res
const EventActionItemT	RELEASE_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	ReleaseComplete_MSG,	M_INVALID_SAG_TIMERTYPE,	RELEASE__ULSignal__ReleaseComplete},//realease complete
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_RELEASE_COMPLETE,	RELEASE__Timeout__ReleaseComplete},//timeout release complete
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	RELEASE__ULSignal__ErrNotiReq},//error notify
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};


//SAG_GRP_CALLSETUP_STATE,	//GrpSetup received,waiting for listenners to join
const EventActionItemT	GRP_CALLSETUP_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	Grp_Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_CALLSETUP__ULSignal__GrpDisconnect},//GRP_CALLSETUP__ULSignal__GrpDisconnect,
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_CALLSETUP__ULSignal__ErrNotiReq},//GRP_CALLSETUP__ULSignal__ErrNotiReq,
	{MSGID_TIMEOUT_LOCALSAG,	InvalidSignal_MSG,	TIMERID_SAG_GRP_PTTCONNECT,	GRP_CALLSETUP__Timeout__PttConnect},//GRP_CALLSETUP__Timeout__PttConnect,
	{MSGID_SAG_INNER_SIGNAL,	PTT_Connect_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_CALLSETUP__INSignal__PttConnect},//GRP_CALLSETUP__INSignal__PttConnect,
	{MSGID_SAG_INNER_SIGNAL,	Group_CallingRls_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_CALLSETUP__INSignal__GrpCallingRelease},//GRP_CALLSETUP__INSignal__GrpCallingRelease,//-
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};

//SAG_GRP_TALKING_STATE,//talking
const EventActionItemT	GRP_TALKING_EventActionTbl[] = 
{
	{MSGID_VOICE_VCR_SIGNAL,	Grp_Disconnect_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_TALKING__ULSignal__GrpDisconnect},//GRP_TALKING__ULSignal__GrpDisconnect,
	{MSGID_VOICE_VCR_SIGNAL,	PTT_Rls_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_TALKING__ULSignal__PttRls},//GRP_TALKING__ULSignal__PttRls,
	{MSGID_VOICE_VCR_SIGNAL,	ErrNotifyReq_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_TALKING__ULSignal__ErrNotiReq},//GRP_TALKING__ULSignal__ErrNotiReq,
	{MSGID_SAG_INNER_SIGNAL,	Group_CallingRls_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_TALKING__INSignal__GrpCallingRelease},//GRP_TALKING__INSignal__GrpCallingRelease,//最长讲话时间到//最长组呼存在时间到等原因释放组呼
	{MSGID_SAG_INNER_SIGNAL,	PTT_Interrupt_MSG,	M_INVALID_SAG_TIMERTYPE,	GRP_TALKING__INSignal__PttInterrupt},//GRP_TALKING__INSignal__PttInterrupt,
	{M_INVALID_MESSAGE_ID, InvalidSignal_MSG, M_INVALID_SAG_TIMERTYPE, SAG_CALL_FSM_END}
};


//////////////////////////////////////////////////////////////////////////

const EventActionItemT*	pEventAction[] = 
{
	(EventActionItemT*)IDLE_EventActionTbl,
	(EventActionItemT*)O_Setup_EventActionTbl,
	(EventActionItemT*)O_Dial_EventActionTbl,
	(EventActionItemT*)O_Alerting_EventActionTbl,
	(EventActionItemT*)Ringback_EventActionTbl,
	(EventActionItemT*)Paging_EventActionTbl,
	(EventActionItemT*)T_AssignRes_EventActionTbl,
	(EventActionItemT*)T_Setup_EventActionTbl,
	(EventActionItemT*)T_Alerting_EventActionTbl,
	(EventActionItemT*)Ring_EventActionTbl,
	(EventActionItemT*)CONNECT_EventActionTbl,
	(EventActionItemT*)WAITONHOOK_EventActionTbl,
	(EventActionItemT*)RELEASE_EventActionTbl,
	(EventActionItemT*)GRP_CALLSETUP_EventActionTbl,
	(EventActionItemT*)GRP_TALKING_EventActionTbl
};

sagCallFsmProcPtr pCallFsmFunc[SAG_CALL_FSM_COUNT]=
{
	IDLE__ULSignal__AssignResReq__Proc,
	IDLE__INSignal__Setup__Proc,
	O_SETUP__ULSignal__PttSetupReq__Proc,
	O_SETUP__ULSignal__Setup__Proc,
	O_SETUP__Timeout__OSetup__Proc,
	O_SETUP__ULSignal__ErrNotiReq__Proc,
	O_SETUP__INSignal__Disconnect__Proc,
	DIAL__ULSignal__DialNumber__Proc,
	DIAL__Timeout__DialNumber__Proc,
	DIAL__ULSignal__Disconnect__Proc,
	DIAL__ULSignal__ErrNotiReq__Proc,
	DIAL__INSignal__Disconnect__Proc,
	O_ALERTING__INSignal__Alerting__Proc,
	O_ALERTING__Timeout__Alerting__Proc,
	O_ALERTING__ULSignal__Disconnect__Proc,
	O_ALERTING__ULSignal__ErrNotiReq__Proc,
	O_ALERTING__INSignal__Disconnect__Proc,
	RINGBACK__INSignal__Connect__Proc,
	RINGBACK__Timeout__Connect__Proc,
	RINGBACK__ULSignal__Disconnect__Proc,
	RINGBACK__ULSignal__ErrNotiReq__Proc,
	RINGBACK__INSignal__Disconnect__Proc,
	PAGING__ULSignal__LapagingRsp__Proc,
	PAGING__Timeout__LapagingRsp__Proc,
	PAGING__INSignal__Disconnect__Proc,
	T_ASSIGNRES__ULSignal__AssignResReq__Proc,
	T_ASSIGNRES__Timeout__AssignResReq__Proc,
	T_ASSIGNRES__ULSignal__ErrNotiReq__Proc,
	T_ASSIGNRES__INSignal__Disconnect__Proc,
	T_SETUP__ULSignal__SetupAck__Proc,
	T_SETUP__Timeout__SetupAck__Proc,
	T_SETUP__ULSignal__ErrNotiReq__Proc,
	T_SETUP__INSignal__Disconnect__Proc,
	T_ALERTING__ULSignal__Alerting__Proc,
	T_ALERTING__Timeout__Alerting__Proc,
	T_ALERTING__ULSignal__ErrNotiReq__Proc,
	T_ALERTING__INSignal__Disconnect__Proc,
	RING__ULSignal__Connect__Proc,
	RING__Timeout__Connect__Proc,
	RING__ULSignal__ErrNotiReq__Proc,
	RING__INSignal__Disconnect__Proc,
	RING__ULSignal__Disconnect__Proc,
	CONNECT__ULSignal__Disconnect__Proc,
	CONNECT__ULSignal__ErrNotiReq__Proc,
	CONNECT__INSignal__Disconnect__Proc,
	CONNECT__INSignal__TConnectAck__Proc,
	CONNECT__Timeout__TConnectAck__Proc,
	CONNECT__ULSignal__OConnectAck__Proc,
	CONNECT__Timeout__OConnectAck__Proc,	
	WAITONHOOK__ULSignal__Disconnect__Proc,
	WAITONHOOK__Timeout__Disconnect__Proc,
	WAITONHOOK__ULSignal__ErrNotiReq__Proc,
	RELEASE__ULSignal__ReleaseComplete__Proc,
	RELEASE__Timeout__ReleaseComplete__Proc,
	RELEASE__ULSignal__ErrNotiReq__Proc,
	GRP_CALLSETUP__ULSignal__GrpDisconnect__Proc,
	GRP_CALLSETUP__ULSignal__ErrNotiReq__Proc,
	GRP_CALLSETUP__Timeout__PttConnect__Proc,
	GRP_CALLSETUP__INSignal__PttConnect__Proc,
	GRP_CALLSETUP__INSignal__GrpCallingRelease__Proc,
	GRP_TALKING__ULSignal__GrpDisconnect__Proc,
	GRP_TALKING__ULSignal__PttRls__Proc,
	GRP_TALKING__ULSignal__ErrNotiReq__Proc,
	GRP_TALKING__INSignal__GrpCallingRelease__Proc,
	GRP_TALKING__INSignal__PttInterrupt__Proc
};

char callFsmFuncName[SAG_CALL_FSM_COUNT][50]=
{
	"IDLE__ULSignal__AssignResReq",
	"IDLE__INSignal__Setup",
	"O_SETUP__ULSignal__PttSetupReq",
	"O_SETUP__ULSignal__Setup",
	"O_SETUP__Timeout__OSetup",
	"O_SETUP__ULSignal__ErrNotiReq",
	"O_SETUP__INSignal__Disconnect",
	"DIAL__ULSignal__DialNumber",
	"DIAL__Timeout__DialNumber",
	"DIAL__ULSignal__Disconnect",
	"DIAL__ULSignal__ErrNotiReq",
	"DIAL__INSignal__Disconnect",
	"O_ALERTING__INSignal__Alerting",
	"O_ALERTING__Timeout__Alerting",
	"O_ALERTING__ULSignal__Disconnect",
	"O_ALERTING__ULSignal__ErrNotiReq",
	"O_ALERTING__INSignal__Disconnect",
	"RINGBACK__INSignal__Connect",
	"RINGBACK__Timeout__Connect",
	"RINGBACK__ULSignal__Disconnect",
	"RINGBACK__ULSignal__ErrNotiReq",
	"RINGBACK__INSignal__Disconnect",
	"PAGING__ULSignal__LapagingRsp",
	"PAGING__Timeout__LapagingRsp",
	"PAGING__INSignal__Disconnect",
	"T_ASSIGNRES__ULSignal__AssignResReq",
	"T_ASSIGNRES__Timeout__AssignResReq",
	"T_ASSIGNRES__ULSignal__ErrNotiReq",
	"T_ASSIGNRES__INSignal__Disconnect",
	"T_SETUP__ULSignal__SetupAck",
	"T_SETUP__Timeout__SetupAck",
	"T_SETUP__ULSignal__ErrNotiReq",
	"T_SETUP__INSignal__Disconnect",
	"T_ALERTING__ULSignal__Alerting",
	"T_ALERTING__Timeout__Alerting",
	"T_ALERTING__ULSignal__ErrNotiReq",
	"T_ALERTING__INSignal__Disconnect",
	"RING__ULSignal__Connect",
	"RING__Timeout__Connect",
	"RING__ULSignal__ErrNotiReq",
	"RING__INSignal__Disconnect",
	"RING__ULSignal__Disconnect",
	"CONNECT__ULSignal__Disconnect",
	"CONNECT__ULSignal__ErrNotiReq",
	"CONNECT__INSignal__Disconnect",
	"CONNECT__INSignal__TConnectAck",
	"CONNECT__Timeout__TConnectAck",
	"CONNECT__ULSignal__OConnectAck",
	"CONNECT__Timeout__OConnectAck",	
	"WAITONHOOK__ULSignal__Disconnect",
	"WAITONHOOK__Timeout__Disconnect",
	"WAITONHOOK__ULSignal__ErrNotiReq",
	"RELEASE__ULSignal__ReleaseComplete",
	"RELEASE__Timeout__ReleaseComplete",
	"RELEASE__ULSignal__ErrNotiReq",
	"GRP_CALLSETUP__ULSignal__GrpDisconnect",
	"GRP_CALLSETUP__ULSignal__ErrNotiReq",
	"GRP_CALLSETUP__Timeout__PttConnect",
	"GRP_CALLSETUP__INSignal__PttConnect",
	"GRP_CALLSETUP__INSignal__GrpCallingRelease",//-
	"GRP_TALKING__ULSignal__GrpDisconnect",
	"GRP_TALKING__ULSignal__PttRls",
	"GRP_TALKING__ULSignal__ErrNotiReq",
	"GRP_TALKING__INSignal__GrpCallingRelease",//最长讲话时间到//最长组呼存在时间到等原因释放组呼
	"GRP_TALKING__INSignal__PttInterrupt"	
};

char callFsmStateName[SAG_STATE_COUNT][40]=
{
	"IDLE_STATE",		//waiting fro assignResReq(OCall) or InnerSetup(内部生成)
	"O_SETUP_STATE",	//waiting for setup
	"DIAL_STATE",		//dial number
	"O_ALERTING_STATE",//waiting for alerting
	"RINGBACK_STATE",//waiting for connect
	"PAGING_STATE",	//waiting for LapagingRsp
	"T_ASSIGNRES_STATE",//waiting for assignResReq
	"T_SETUP_STATE",	//waiting for setupAck
	"T_ALERTING_STATE",//waiting for alerting
	"RING_STATE",		//ringing,waiting for connect
	"CONNECT_STATE",	//connected
	"WAIT_ONHOOK_STATE",//waiting fro disconnect(on hook)
	"RELEASE_STATE",		//release sent,waiting release complete to release transport res	
//GrpVoiceCall Srv begin	
	"GRP_CALLSETUP_STATE",	//GrpSetup received,waiting for listenners to join
	"GRP_TALKING_STATE"//talking
//GrpVoiceCall Srv end
	
};

