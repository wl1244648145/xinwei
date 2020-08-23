/*******************************************************************************
* Copyright (c) 2005 by Arrowping Networks, Inc.   All Rights Reserved   
* File Name      : tVoice.cpp
* Create Date    : 18-Aug-2005
* programmer     :fengbing
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "localSagTask.h"
#include "localSagCfg.h"
#include "dataGrpLink.h"

//#include "Object.h"
#include "MsgQueue.h"
#include "Message.h"
#include "voiceFSM.h"

#include "log.h"

#include "tvoice.h"
#include "tVDR.h"
#include "tVCR.h"
#include "tvoice.h"

//任务实例指针的初始化
CTVoice* CTVoice::s_ptaskTVoice = NULL;

CTVoice::CTVoice()
{
	LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CTVoice::CTVoice()");
#ifndef NDEBUG
	if (!Construct(CObject::M_OID_VOICE))
	{
       	LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "ERROR!!!CTVoice::CTVoice()% Construct failed.");
	}
#endif
	
	memcpy(m_szName, M_TASK_TVOICE_TASKNAME, strlen( M_TASK_TVOICE_TASKNAME ) );
	m_szName[strlen( M_TASK_TVOICE_TASKNAME )] = 0;

	m_uPriority = M_TP_L3VOICE;
#ifndef DSP_BIOS
	m_uOptions = M_TASK_TVOICE_OPTION;
#endif
	m_uStackSize = M_TASK_TVOICE_STACKSIZE;//fengbing 20091229 stack==64K

	m_lMaxMsgs = M_TASK_TVOICE_MAXMSG;
#ifndef DSP_BIOS
	m_lMsgQOption = M_TASK_TVOICE_MSGOPTION;
#endif

	m_iMsgQMax = M_TASK_TVOICE_MAXMSG;
#ifndef DSP_BIOS
	m_iMsgQOption = M_TASK_TVOICE_MSGOPTION;
#endif
}

CTVoice::~CTVoice()
{
	LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CTVoice::~CTVoice");
#ifndef NDEBUG
	if (!Destruct(CObject::M_OID_VOICE))
	{
       	LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_NORMAL), "ERROR!!!CTVoice::~CTVoice failed.");
	}
#endif
}

TID CTVoice::GetEntityId() const
{ 
	return M_TID_VOICE; 
}

CTVoice* CTVoice::GetInstance()
{
	if ( NULL == s_ptaskTVoice)
	{
       	 s_ptaskTVoice = new CTVoice;
 	}
    	return s_ptaskTVoice;
}

bool CTVoice::Initialize()
{
	LOG(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), "CTVoice::Initialize");

	//create message queue
//	m_pMsgQueue = new CMsgQueue(m_lMaxMsgs, m_lMsgQOption);
//	if (NULL == m_pMsgQueue)
//	{
//		return false;
//	}


	if(!m_fsm.init())
	{
		return false;
	}
	UINT8 ucInit = CBizTask::Initialize();
	if ( false == ucInit)
	{
		//Delete MsgQueue
		delete m_pMsgQueue;
	       return false;
	}
	return true;
}

bool CTVoice::ProcessMessage(CMessage& msg)
{
	m_fsm.Parse_Handle_Event(msg);
	return true;
}




bool InitL3VoiceSvc()
{
#ifndef __WIN32_SIM__

	updateVoiceCfgs();	//保证任务运行前配置生效
	updateLocalSagCfg(true);//localSag任务配置生效
	updateDcsCfg();
	
	CTVoice * pTVoice = CTVoice::GetInstance();
	pTVoice->Begin();
//master SAG		 
	CVCR * pTVCR = CVCR::GetInstance();
	pTVCR->Begin();
		 
	CVDR * pTVDR = CVDR::GetInstance();
	pTVDR->Begin();
//backup SAG
	CVCR * pTVCR1 = CVCR::GetBakInstance();
	pTVCR1->Begin();
		 
	CVDR * pTVDR1 = CVDR::GetBakInstance();
	pTVDR1->Begin();
//local SAG
	CTask_SAG *pSagTask = CTask_SAG::GetInstance();
	if(pSagTask!=NULL)
	{
		pSagTask->Begin();
	}
//DCS
	CDGrpLink *pDcsLinkTask = CDGrpLink::GetInstance();
	if(pDcsLinkTask!=NULL)
	{
		pDcsLinkTask->Begin();
	}

#endif

     return true;
}

///////////////////////////////////////////////////////////////////////////////










































