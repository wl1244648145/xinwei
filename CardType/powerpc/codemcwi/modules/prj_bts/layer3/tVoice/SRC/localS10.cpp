/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSagTask.cpp
* Create Date    : 16-Sep-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "string.h"

#ifdef __VXWORKS__
#include "vxworks.h"
#endif

#include "Log.h"
#include "MsgQueue.h"

#include "localSagTask.h"
#include "voiceCommon.h"


CTask_SAG* CTask_SAG::s_pTaskSAG = NULL;
CSAG * CTask_SAG::m_pSAG = NULL;
CTask_SAG::CTask_SAG()
{
	LOG(LOG_DEBUG, LOGNO(SAG, EC_L3VOICE_NORMAL), "CTask_SAG::CTask_SAG()");
#ifndef NDEBUG
	if (!Construct(CObject::M_OID_SAG))
	{
       	LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "ERROR!!!CTask_SAG::CTask_SAG()% Construct failed.");
	}
#endif
	
	memcpy(m_szName, M_TASK_TLOCALSAG_TASKNAME, strlen( M_TASK_TLOCALSAG_TASKNAME ) );
	m_szName[strlen( M_TASK_TLOCALSAG_TASKNAME )] = 0;

	m_uPriority = M_TP_L3VOICE;
#ifndef DSP_BIOS
	m_uOptions = M_TASK_TLOCALSAG_OPTION;
#endif
	m_uStackSize = M_TASK_TLOCALSAG_STACKSIZE;

	m_iMsgQMax = M_TASK_TLOCALSAG_MAXMSG;
#ifndef DSP_BIOS
	m_iMsgQOption = M_TASK_TLOCALSAG_MSGOPTION;
#endif	
}
CTask_SAG::~CTask_SAG()
{
	LOG(LOG_DEBUG, LOGNO(SAG, EC_L3VOICE_NORMAL), "CTask_SAG::~CTask_SAG");
#ifndef NDEBUG
	if (!Destruct(CObject::M_OID_VOICE))
	{
       	LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_NORMAL), "ERROR!!!CTask_SAG::~CTask_SAG failed.");
	}
#endif	
}
bool CTask_SAG::Initialize()
{
	LOG(LOG_DEBUG, LOGNO(SAG, EC_L3VOICE_NORMAL), "CTask_SAG::Initialize");
#ifdef M_SAG	
	m_pSAG = CSAG::getSagInstance();
	if(m_pSAG!=NULL)
	{
		if(!m_pSAG->Init())
		{
			return false;
		}
	}
	else
	{
		return false;
	}
#endif	
	UINT8 ucInit = CBizTask::Initialize();
	if ( false == ucInit)
	{
		//Delete MsgQueue
		delete m_pMsgQueue;
	       return false;
	}
	return true;	
}
bool CTask_SAG::ProcessMessage(CMessage& msg)
{
#ifdef M_SAG		
	m_pSAG->parseAndHandleMsg(msg);
	return true;
#endif
}

CTask_SAG* CTask_SAG::GetInstance()
{
	if ( NULL == s_pTaskSAG)
	{
		s_pTaskSAG = new CTask_SAG;
#ifdef M_SAG
		m_pSAG = CSAG::getSagInstance();
		if(m_pSAG)
		{
			m_pSAG->initSignalHandlers();
			m_pSAG->initTimeoutHandlers();
		}
#endif
 	}
    	return s_pTaskSAG;
}


