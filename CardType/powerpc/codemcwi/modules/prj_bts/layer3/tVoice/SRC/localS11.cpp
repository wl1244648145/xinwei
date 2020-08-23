/*******************************************************************************
* Copyright (c) 2009 by Beijing Ap Co.Ltd.All Rights Reserved   
* File Name      : localSagTimer.cpp
* Create Date    : 12-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "localSagMsgID.h"
#include "localSagTimer.h"
#include "tVoice.h"

SagTimerCfgT sagTimerCfgTbl[TIMERID_SAG_COUNT]=
{
	{ SAG_ONE_SECOND*5, false },//TIMERID_SAG_O_SETUP,
	{ SAG_ONE_SECOND*20, false },//TIMERID_SAG_O_DIALNUMBER,
	{ SAG_ONE_SECOND*20, false },//TIMERID_SAG_O_ALERTING,
	{ SAG_ONE_SECOND*90, false },//TIMERID_SAG_O_CONNECT,
	{ SAG_ONE_SECOND*5, false },//TIMERID_SAG_O_CONNECTACK,
	{ SAG_ONE_SECOND*5, false },//TIMERID_SAG_T_SETUPACK,
	{ SAG_ONE_SECOND*5, false },//TIMERID_SAG_T_ALERTING,
	{ SAG_ONE_SECOND*90, false },//TIMERID_SAG_T_CONNECT,
	{ SAG_ONE_SECOND*5, false },//TIMERID_SAG_T_CONNECTACK,
	{ SAG_ONE_SECOND*10, false },//TIMERID_SAG_DISCONNECT,
	{ SAG_ONE_SECOND*2, false },//TIMERID_SAG_RELEASE_COMPLETE,
	{ SAG_ONE_SECOND*16, false },//TIMERID_SAG_T_LAPAGING,
	{ SAG_ONE_SECOND*2, false },//TIMERID_SAG_T_ASSIGN_TRANS_RES,
	{ SAG_ONE_SECOND*6, false },//TIMERID_SAG_GRP_PTTCONNECT,//组呼建立超时
	{ 4800, false },//TIMERID_SAG_GRP_LAGRPPAGING,4.8秒,稍早于bts的寻呼超时
	{ SAG_ONE_SECOND*5, false },//TIMERID_SAG_GRP_ASSIGNRESREQ,
	{ SAG_ONE_SECOND*5, false },//TIMERID_SAG_GRP_PRESSINFO,
	{ SAG_ONE_SECOND*5, false },//TIMERID_SAG_GRP_MAX_IDLE_TIME,
	{ SAG_ONE_SECOND*0xffff, false },//TIMERID_SAG_GRP_TTL,
	{ SAG_ONE_SECOND*0xffff, false }//TIMERID_SAG_GRP_MAX_TALKING_TIME,
	
};

char sagTimerName[TIMERID_SAG_COUNT][40]=
{
	"TIMER_O_SETUP",
	"TIMER_DIALNUMBER",
	"TIMER_O_ALERTING",
	"TIMER_O_CONNECT",
	"TIMER_O_CONNECTACK",
	"TIMER_T_SETUPACK",
	"TIMER_T_ALERTING",
	"TIMER_T_CONNECT",
	"TIMER_T_CONNECTACK",
	"TIMER_DISCONNECT",
	"TIMER_RELEASE_COMPLETE",
	"TIMER_LAPAGING",
	"TIMER_T_ASSIGN_TRANS_RES",
	"TIMERID_SAG_GRP_PTTCONNECT",//组呼建立超时
	"TIMERID_SAG_GRP_LAGRPPAGING",
	"TIMERID_SAG_GRP_ASSIGNRESREQ",
	"TIMERID_SAG_GRP_PRESSINFO",
	"TIMERID_SAG_GRP_MAX_IDLE_TIME",
	"TIMERID_SAG_GRP_TTL",
	"TIMERID_SAG_GRP_MAX_TALKING_TIME"	
};

bool startSagTimer(UINT16 timerID, CTimer**ppTimer, 
	UINT32 uid, UINT32 l3Addr,
	UINT16 gid, UINT32 grpL3Addr)
{
	if(NULL==ppTimer)
	{
		LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"startTimer, ppTimer is NULL!!! ");
		return false;
	}
	if(timerID<TIMERID_SAG_COUNT)
	{
		if(*ppTimer!=NULL)
		{
			LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"startTimer, pTimer has not been stopped!!!!!!!!!!!! ");
		}

		CComMessage* pMsg = new (CTVoice::GetInstance(), sizeof(SagTimerStructT)) CComMessage;
		if (pMsg==NULL)
		{
			LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
			return false;
		}
		else
		{
			SagTimerStructT* pDataMsg = (SagTimerStructT*)pMsg->GetDataPtr();
			pDataMsg->type = timerID;
			pDataMsg->uid = uid;
			pDataMsg->l3Addr = l3Addr;
			pDataMsg->gid = gid;
			pDataMsg->grpL3Addr = grpL3Addr;
			
			pMsg->SetMessageId(MSGID_TIMEOUT_LOCALSAG);
			pMsg->SetSrcTid(M_TID_SAG);
			pMsg->SetDstTid(M_TID_SAG);
		}    
		*ppTimer = new CTimer(0!=sagTimerCfgTbl[timerID].blPeriodic, 
			sagTimerCfgTbl[timerID].timeoutVal, pMsg);
		if((*ppTimer)==NULL)
		{
			LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "startTimer, new CTimer failed!!!!");
			pMsg->Destroy();
			return false;
		}
		else
		{
			(*ppTimer)->Start();
			if(sagTimerCfgTbl[timerID].blPeriodic)
			{
				LOG5(LOG_DEBUG3, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"startTimer timer[%s] UID[0x%08X] L3Addr[0x%08X] GID[0x%04X] GrpL3Addr[0x%08X]",
					(int)sagTimerName[timerID], uid, l3Addr, gid, grpL3Addr);
			}
			else
			{
				LOG5(LOG_DEBUG1, LOGNO(SAG, EC_L3VOICE_NORMAL), 
					"startTimer timer[%s] UID[0x%08X] L3Addr[0x%08X] GID[0x%04X] GrpL3Addr[0x%08X]",
					(int)sagTimerName[timerID], uid, l3Addr, gid, grpL3Addr);
			}
		}
		return true;

		
	}
	else
	{
		LOG4(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
				"startTimer timerID[0x%04X] UID[0x%08X] l3Addr[0x%08X], timerID>0x%X!!!",
				timerID, uid, l3Addr, TIMERID_SAG_COUNT);
		return false;
	}
}
bool stopSagTimer(CTimer**ppTimer)
{
	return deleteSagTimer(ppTimer);
}
bool deleteSagTimer(CTimer**ppTimer)
{
	if((*ppTimer)!=NULL)
	{
		(*ppTimer)->Stop();
		delete (*ppTimer);
		*ppTimer = NULL;
		return true;
	}
	else
	{
#if 0		
		LOG(LOG_WARN, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"deleteTimer, pTimer is NULL!!! ");
#endif
	}
	return false;
}


