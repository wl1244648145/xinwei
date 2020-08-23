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
01)��æ����ʱ
02)�Ų�������ʱ
03)�Ż�������ʱ
04)���еȴ�UT��setup��ʱ
05)���еȴ�UT���ų�ʱ
06)���еȴ�alerting��ʱ
07)���еȴ�connect��ʱ
08)���еȴ�UT��connectAck��ʱ
09)���еȴ�UT��setupAck��ʱ
10)���еȴ�UT��alerting��ʱ
11)���еȴ�UT��connect��ʱ
12)���еȴ�connectAck��ʱ
13)�ȴ�UT�һ���ʱ
14)�ȴ�����release complete��ʱ
15)���еȴ�btsѰ����Ӧ��ʱ
16)���еȴ�bts��assign transport resource Req��ʱ
17)����
18)����
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
	TIMERID_SAG_GRP_PTTCONNECT,//���������ʱ
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


