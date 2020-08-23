/*******************************************************************************
* Copyright (c) 2009 by Beijing Jiaxun Feihong Electrical Co.Ltd.All Rights Reserved   
* File Name      : localSagTimer.h
* Create Date    : 9-Oct-2009
* programmer     :
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__LOCALSAGTIMER_H
#define	__LOCALSAGTIMER_H

#include "dataType.h"
#include "timer.h"
#include "localSagCommon.h"

//All timers
/*
01)放忙音超时
02)放拨号音超时
03)放回铃音超时
04)主叫等待UT的setup超时
05)主叫等待UT拨号超时
06)主叫等待alerting超时
07)主叫等待connect超时
08)主叫等待UT的connectAck超时
09)被叫等待UT的setupAck超时
10)被叫等待UT的alerting超时
11)被叫等待UT的connect超时
12)被叫等待connectAck超时
13)等待UT挂机超时
14)等待信令release complete超时
15)被叫等待bts寻呼响应超时
16)被叫等待bts的assign transport resource Req超时
17)暂无
18)暂无
*/
enum
{
	TIMERID_SAG_MIN=0,
		
	//begin
	TIMERID_SAG_O_SETUP=0,
	TIMERID_SAG_O_DIALNUMBER,
	TIMERID_SAG_O_ALERTING,
	TIMERID_SAG_O_CONNECT,
	TIMERID_SAG_O_CONNECTACK,
	TIMERID_SAG_T_SETUPACK,
	TIMERID_SAG_T_ALERTING,
	TIMERID_SAG_T_CONNECT,
	TIMERID_SAG_T_CONNECTACK,
	TIMERID_SAG_DISCONNECT,
	TIMERID_SAG_RELEASE_COMPLETE,
	TIMERID_SAG_T_LAPAGING,
	TIMERID_SAG_T_ASSIGN_TRANS_RES,
	TIMERID_SAG_GRP_PTTCONNECT,//组呼建立超时
	TIMERID_SAG_GRP_LAGRPPAGING,
	TIMERID_SAG_GRP_ASSIGNRESREQ,
	TIMERID_SAG_GRP_PRESSINFO,
	TIMERID_SAG_GRP_MAX_IDLE_TIME,
	TIMERID_SAG_GRP_TTL,
	TIMERID_SAG_GRP_MAX_TALKING_TIME,
	
	//new timer add here
	//end
	
	TIMERID_SAG_COUNT
};

#define M_INVALID_SAG_TIMERTYPE (0xffff)

typedef struct __SagTimerStructT
{
	UINT16 type;
	UINT32 uid;
	UINT32 l3Addr;
	UINT16 gid;
	UINT32 grpL3Addr;
}SagTimerStructT, *PSagTimerStructT;

typedef struct __SagTimerCfgT
{
	UINT32 timeoutVal;
	bool blPeriodic;
//	TID srcTID;
//	TID dstTID;
}SagTimerCfgT;

#define SAG_ONE_SECOND (1000)
#define SAG_ONE_MINUTE (60*SAG_ONE_SECOND)
#define SAG_ONE_HOUR (60*SAG_ONE_MINUTE)

#ifdef __cplusplus
extern "C" {
#endif

extern SagTimerCfgT sagTimerCfgTbl[TIMERID_SAG_COUNT];
extern char sagTimerName[TIMERID_SAG_COUNT][40];
bool startSagTimer(UINT16 timerID, CTimer**ppTimer, 
	UINT32 uid, UINT32 l3Addr, 
	UINT16 gid=M_INVALID_GID, UINT32 grpL3Addr=M_INVALID_GRPL3ADDR);
bool stopSagTimer(CTimer**ppTimer);
bool deleteSagTimer(CTimer**ppTimer);

#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAGTIMER_H */


