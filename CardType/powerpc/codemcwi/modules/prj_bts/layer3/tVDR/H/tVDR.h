/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    tVDR.h
*
* DESCRIPTION: 
*		BTS上的tVDR任务类
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-9-20   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__TVDR_H
#define	__TVDR_H

#include "BizTask.h"
#include "ComMessage.h"
#include "taskdef.h"
#include "voiceCommon.h"
#include "voicecfg.h"
#include "NatpiApp.h"

#ifndef __WIN32_SIM__
#ifdef __VXWORKS__
#include "taskLib.h"
#include "msgQLib.h"
#include "selectLib.h"
#include "sockLib.h"
#include "ioLib.h"
#include "pipeDrv.h"
#include "inetLib.h" 
#include "errnoLib.h"
#include "netinet/ip.h"
#endif
typedef struct fd_set FD_SET;
#ifdef __USE_LWIP__
#include "datatype.h"
#include "util.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#endif
#endif

//任务参数定义
#define M_TASK_NAME_LEN				(20)
#define M_TASK_TVDR_TASKNAME      "tVDR"
#define M_TASK_TVDR1_TASKNAME      "tVDR1"
#define M_TASK_TVDR_PRIORITY      (95)
#ifdef __WIN32_SIM__
#define M_TASK_TVDR_OPTION        (0x0008)
#define M_TASK_TVDR_MSGOPTION     (0x02)
#elif __NUCLEUS__
#define M_TASK_TVDR_OPTION       (NULL)	//not use
#define M_TASK_TVDR_MSGOPTION     (NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_TVDR_OPTION        (VX_FP_TASK)
#define M_TASK_TVDR_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TVDR_STACKSIZE     (20480)
#define M_TASK_TVDR_MAXMSG        (1024)

#define M_VDR_PIPENAME			"/dev/VDRPipe"
#define M_VDR1_PIPENAME			"/dev/VDRPipe1"
#define M_WIN32_VDR_PIPEPORT		(3456)
#define M_WIN32_VDR1_PIPEPORT		(3457)

#define M_MAX_VDR_PDU			(1500)
#ifdef DSP_BIOS
#define TVDR_MAX_BLOCKED_TIME_IN_10ms_TICK	(200)//(2000)//20seconds
#else
#define TVDR_MAX_BLOCKED_TIME_IN_10ms_TICK	(2000)//20seconds
#endif
#ifdef DSP_BIOS
class CVDR:public CBizTask
#else
class CVDR:public CTask
#endif
{
public:
	static CVDR* GetInstance();
	static CVDR* GetBakInstance();	
	void setTaskOptions();
	bool isMasterInstance(){return M_TID_VDR==m_selfTID;};
	bool isBackupInstance(){return M_TID_VDR1==m_selfTID;};
	UINT32 GetBtsSagIpAddr();
	CVDR();
	~CVDR();
	bool Initialize();
	void setTosVal(UINT8 tos);
protected:
#ifndef DSP_BIOS
	virtual void MainLoop();	
#endif
private:

	static CVDR* s_ptaskTVDR;
	static CVDR* s_ptaskTVDR1;
	
	void SetEntityId(TID tid){m_selfTID=tid;};
	TID GetEntityId() const; //{ return M_TID_VDR; }
#ifdef DSP_BIOS
	//lijinan 20091118 add-----------
    inline bool IsNeedTransaction() const
    {
    	return false;
    }
	bool ProcessComMessage(CComMessage* pComMsg);
	//---------------------------------
#endif
	bool PostMessage(CComMessage* pMsg,	SINT32 timeout, bool isUrgent=false);
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return TVDR_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

	bool IsNetCfgChanged();
	void reloadVdrCfg();

	bool CreateSocket();
	void CloseSocket();

	bool CreatePipe();
	bool DeletePipe();
	bool OpenPipe();
	bool ClosePipe();
#ifdef DSP_BIOS
	static STATUS RunVdrSocketRx(CVDR *);
	static STATUS RunVdr1SocketRx(CVDR *);
	void SocketRxMainLoop();
	int RecvAndHandleVoiceDataFromVoice(CComMessage *pComMsg);	//接收并处理从tVoice任务收到的语音包
#else
	int RecvAndHandleVoiceDataFromVoice();	//接收并处理从tVoice任务收到的语音包
#endif
	bool RecvAndHandleVoiceDataFromSAG();	//接收并处理从SAG收到的语音包

	int	m_UdpSocket;	//UDP socket to receive voice data
	int m_fdPipe;		//fd for pipe, for inter-task communication

	char m_MediaServerIPAddr[16];	//媒体服务器的IP地址
	UINT16 m_MediaPort;				//媒体服务器的端口
	UINT16 m_LocalPort;				//本端语音UDP端口
	UINT32 m_BTSID;					//本端BTSID
	bool m_blEffect;				//是否使用语音业务
	bool m_blNetCfgChanged;			//if NetCfg Changed
	bool m_blNatApSessionNeedRestart;//if need restart natap session
#ifdef DSP_BIOS
	SEM_Handle semHandle ;
#endif
	struct sockaddr_in m_RemoteAddr;

	TID m_selfTID;
	char m_szPipeName[20];
	NatPiSession* m_pNatpiSession;
	T_BtsSagLinkCfg* m_pBtsSagLinkCfg;
};
#endif /* __TVDR_H */
