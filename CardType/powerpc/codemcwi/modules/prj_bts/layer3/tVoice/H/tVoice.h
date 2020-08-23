/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    tVoice.h
*
* DESCRIPTION: 
*		BTS上的Voice功能L3任务类
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2006-04-09 fengbing  add getVoiceFSM member function;
*   2006-03-25 fengbing  add getSAGCongestLevel member function;
*   2005-08-18 fengbing  initialization. 
*
*---------------------------------------------------------------------------*/

#ifndef	__TVOICE_H
#define	__TVOICE_H

#include "BizTask.h"
#include "ComMessage.h"
#include "voiceFsm.h"

//任务参数定义
#define M_TASK_NAME_LEN				(20)
#define M_TASK_TVOICE_TASKNAME      "tVoice"
#define M_TASK_TVOICE_PRIORITY      (95)
#ifdef __WIN32_SIM__
#define M_TASK_TVOICE_OPTION        (0x0008)
#define M_TASK_TVOICE_MSGOPTION     (0x02)
#elif __NUCLEUS__
#define M_TASK_TVOICE_OPTION       (NULL)	//not use
#define M_TASK_TVOICE_MSGOPTION     (NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_TVOICE_OPTION        (VX_FP_TASK)
#define M_TASK_TVOICE_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TVOICE_STACKSIZE     (65535)//fengbing 20091229 stack==64K

#ifdef DSP_BIOS
#undef M_TASK_TVOICE_STACKSIZE
#define M_TASK_TVOICE_STACKSIZE     (65532)
#endif

#define M_TASK_TVOICE_MAXMSG        (1024)


class CTVoice:public CBizTask
{
public:
	static CTVoice* GetInstance();
	UINT8 getSAGCongestLevel(){return m_fsm.getSAGCongestLevel();}
	VoiceFSM* getVoiceFSM(){return &m_fsm;}
private:
    SINT32 m_lMaxMsgs;
    SINT32 m_lMsgQOption;

	static CTVoice* s_ptaskTVoice;
	CTVoice();
	~CTVoice();
	bool Initialize();
	bool ProcessMessage(CMessage& msg);
	TID GetEntityId() const; //{ return M_TID_VOICE; }

    #define VOICE_MAX_BLOCKED_TIME_IN_10ms_TICK (200)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return VOICE_MAX_BLOCKED_TIME_IN_10ms_TICK ;};
    
	VoiceFSM m_fsm;
};

#endif /* __TVOICE_H */











































