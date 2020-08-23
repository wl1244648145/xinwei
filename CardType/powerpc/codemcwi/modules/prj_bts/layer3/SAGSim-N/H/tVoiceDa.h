/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    tVoiceData.h
*
* DESCRIPTION: 
*		模拟SAG的tVoiceData任务类,负责接收和转发BTS的语音数据包
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-12-06   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__TVOICEDATA_H
#define	__TVOICEDATA_H

#include "Task.h"
#include "ComMessage.h"

#ifndef __WIN32_SIM__

#include "vxWorks.h" 

#include "taskLib.h"
#include "msgQLib.h"
#include "selectLib.h"
#include "sockLib.h"
#include "ioLib.h"
#include "pipeDrv.h"
#include "inetLib.h" 
#include "errnoLib.h"

typedef struct fd_set FD_SET;

#endif

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif



typedef struct tagDMUXHead
{
	UINT8	FormID:3;
	UINT8	Prio:3;
	UINT8	Reserved1:2;
	UINT8	nFrameNum;
	UINT32	Reserved2;

}DMUXHeadT;

typedef struct tagDMUXVoiDataCommon
{
	UINT32	L3Addr;
	UINT8	SM:4;
	UINT8	Codec:4;
	UINT8	SN;

}DMUXVoiDataCommonT;

typedef struct tagDMUXVoiDataPkt729
{
	DMUXVoiDataCommonT head;
	UINT8	Data[10]; 

}DMUXVoiDataPkt729T;

typedef struct tagDMUXVoiDataPkt711
{
	DMUXVoiDataCommonT head;
	UINT16	Reserved;
	UINT8	Data[80]; 

}DMUXVoiDataPkt711T;


#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

//任务参数定义
#define M_TASK_NAME_LEN				(20)
#define M_TASK_TVOICEDATA_TASKNAME      "tVoiceData"
#define M_TASK_TVOICEDATA_PRIORITY      (95)
#ifdef __WIN32_SIM__
#define M_TASK_TVOICEDATA_OPTION        (0x0008)
#define M_TASK_TVOICEDATA_MSGOPTION     (0x02)
#elif __NUCLEUS__
#define M_TASK_TVOICEDATA_OPTION       (NULL)	//not use
#define M_TASK_TVOICEDATA_MSGOPTION     (NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_TVOICEDATA_OPTION        (VX_FP_TASK)
#define M_TASK_TVOICEDATA_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TVOICEDATA_STACKSIZE     (20480)
#define M_TASK_TVOICEDATA_MAXMSG        (1024)

class CTask_VoiceData:public CTask
{
public:
	static CTask_VoiceData* GetInstance();
protected:
	virtual void MainLoop();
private:
	static CTask_VoiceData* s_ptaskTVOICEDATA;

	CTask_VoiceData(){}
	~CTask_VoiceData(){}
	bool Initialize();
	inline TID GetEntityId() const { return M_TID_VDR; }

	bool CreateSocket();
	bool closesocket();
	void OutputSocketErrCode(char*p);
	
	bool PostMessage(CComMessage*, SINT32, bool){return true;};

};
#endif /* __TVOICEDATA_H */