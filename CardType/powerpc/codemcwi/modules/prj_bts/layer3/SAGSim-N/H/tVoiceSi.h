/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    tVoiceSignal.h
*
* DESCRIPTION: 
*		ģ��SAG��tVoiceSignal������,������պͷ���SAbis+����
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-12-06   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__TVOICESIGNAL_H
#define	__TVOICESIGNAL_H

#include "Task.h"

#include "SAGSignal.h"

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





#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

//�����������
#define M_TASK_NAME_LEN				(20)
#define M_TASK_TVOICESIGNAL_TASKNAME      "tVoiceSignal"
#define M_TASK_TVOICESIGNAL_PRIORITY      (95)
#ifdef __WIN32_SIM__
#define M_TASK_TVOICESIGNAL_OPTION        (0x0008)
#define M_TASK_TVOICESIGNAL_MSGOPTION     (0x02)
#elif __NUCLEUS__
#define M_TASK_TVOICESIGNAL_OPTION       (NULL)	//not use
#define M_TASK_TVOICESIGNAL_MSGOPTION     (NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_TVOICESIGNAL_OPTION        (VX_FP_TASK)
#define M_TASK_TVOICESIGNAL_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TVOICESIGNAL_STACKSIZE     (204800)
#define M_TASK_TVOICESIGNAL_MAXMSG        (1024)

#define M_SAG_SIGNAL_PORT				(2001)	//SAG�������TCP�˿�
#define M_SAG_MEDIA_PORT				(9876)	//��������UDP�˿�

class CTask_VoiceSignal:public CTask
{
public:
	static CTask_VoiceSignal* GetInstance();
	bool SendASignal(UINT fdSocket, CSAbisSignal& signal);
	bool ProcessMedia();
	bool initMediaServer();
protected:
	virtual void MainLoop();
private:
	
	bool PostMessage(CComMessage*, SINT32, bool){return true;}

	CTask_VoiceSignal();
	~CTask_VoiceSignal(){};
	bool Initialize();
	inline TID GetEntityId() const { return M_TID_VCR; }

	bool StartTcpServer();
	bool StopTcpServer();
	
	int BytesAvailable(int fdSocket);
	void OutputSocketErrCode(char*p);

	int CTask_VoiceSignal::recvOneSignal(int m_fdTcpSag, char *pBuf);
	
	static CTask_VoiceSignal* s_pTaskVoiceSignal;
	int m_fdListen;		//TCP����Socket
	int m_fdVoiceData;	//UDP���պͷ����������ݰ��Ķ˿�
	FD_SET m_Clients;	//�����Ѿ��������ӵ�TCP socket

};
#endif /* __TVOICESIGNAL_H */