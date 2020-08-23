/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSagTask.h
* Create Date    : 16-Sep-2009
* programmer     :fb
* description    :sag task
* functions      : 
* Modify History :
*******************************************************************************/

#ifndef	__LOCALSAGTASK_H
#define	__LOCALSAGTASK_H

#define M_SAG
#ifdef __VXWORKS__
#include "taskLib.h"
#endif

#include "BizTask.h"

#ifdef M_SAG	
#include "localSag.h"
#endif

//任务参数定义
#define M_TASK_NAME_LEN				(20)
#define M_TASK_TLOCALSAG_TASKNAME      "tSag"
#define M_TASK_TLOCALSAG_PRIORITY		(95)
#ifdef __WIN32_SIM__
#define M_TASK_TLOCALSAG_OPTION		(0x0008)
#define M_TASK_TLOCALSAG_MSGOPTION	(0x02)
#elif __NUCLEUS__
#define M_TASK_TLOCALSAG_OPTION		(NULL)	//not use
#define M_TASK_TLOCALSAG_MSGOPTION	(NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_TLOCALSAG_OPTION		(VX_FP_TASK)
#define M_TASK_TLOCALSAG_MSGOPTION	( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TLOCALSAG_STACKSIZE     (65535)
#define M_TASK_TLOCALSAG_MAXMSG		(1024)

class CTask_SAG:public CBizTask
{
public:
	static CTask_SAG* GetInstance();
private:
#ifdef M_SAG	
	static CSAG *m_pSAG;
#endif	
	static CTask_SAG* s_pTaskSAG;
	CTask_SAG();
	~CTask_SAG();
	bool Initialize();
	inline TID GetEntityId() const { return M_TID_SAG; }
	bool ProcessMessage(CMessage& msg);	

	bool IsMonitoredForDeadlock()  { return false; };
	#ifdef DSP_BIOS
	int  GetMaxBlockedTime() { return 200;};//-1 ;};
	#else
	int  GetMaxBlockedTime() { return -1 ;};
	#endif
};

#endif /* __LOCALSAGTASK_H */

