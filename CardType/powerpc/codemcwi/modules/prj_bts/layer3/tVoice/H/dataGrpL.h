/*******************************************************************************
* Copyright (c) 2010 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : dataGrpLink.h
* Create Date    : 22-Mar-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__DATAGRPLINK_H
#define	__DATAGRPLINK_H

#include "Task.h"
#include "ComMessage.h"
#include "tVoice.h"
#include "voice_msgs_struct.h"
#include "CallSignalMsg.h"
#include "voicecfg.h"
#include "NatpiApp.h"
#include "tcpE.h"
#include "voiceToolFunc.h"

#ifndef __WIN32_SIM__
#ifdef __VXWORKS__
#include "vxWorks.h" 

#include "taskLib.h"
#include "msgQLib.h"
#include "selectLib.h"
#include "sockLib.h"
#include "ioLib.h"
#include "pipeDrv.h"
#include "inetLib.h" 
#include "errnoLib.h"
#endif
typedef struct fd_set FD_SET;

#endif
#ifdef DSP_BIOS
#define TDGrpLink_MAX_BLOCKED_TIME_IN_10ms_TICK	(200)//(2000)	//20seconds
#else
#define TDGrpLink_MAX_BLOCKED_TIME_IN_10ms_TICK	(2000)	//20seconds
#endif

#ifdef DSP_BIOS
#define localtime_r( _clock, _result ) \
	( *(_result) = *localtime( (_clock) ), \
	  (_result) )
#endif


//任务参数定义
#define M_TASK_NAME_LEN			(20)
#define M_TASK_DGRP_TASKNAME      	"tDGrpLink"
#define M_TASK_DGRP_PRIORITY      	(95)
#ifdef __WIN32_SIM__
#define M_TASK_DGRP_OPTION        	(0x0008)
#define M_TASK_DGRP_MSGOPTION     	(0x02)
#elif __NUCLEUS__
#define M_TASK_DGRP_OPTION       	(NULL)	//not use
#define M_TASK_DGRP_MSGOPTION     	(NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_DGRP_OPTION        	(VX_FP_TASK)
#define M_TASK_DGRP_MSGOPTION     	( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_DGRP_STACKSIZE     	(64000)
#define M_TASK_DGRP_MAXMSG        	(1024)

#define M_DGRP_PIPENAME				"/dev/DGrpPipe"
#define M_WIN32_DGRP_PIPEPORT		(6668)

#define M_MAX_TCPPKT_LEN		(0xffff)
#define M_CONTINUE_RECV (0)
#define M_GET_ONE_MSG (1)
#define M_CONTINUE_PARSE (2)


#ifdef DSP_BIOS
class CDGrpLink:public CBizTask
#else
class CDGrpLink:public CTask
#endif
{
public:
	static CDGrpLink* GetInstance();
	void setTaskOptions();
	bool IsConnected();
	UINT32 GetBtsDcsIpAddr();
	TcpEWithNatApRcvBuf m_DcsRcvBuf;
protected:
	virtual void MainLoop();
private:
	SINT32 m_lMaxMsgs;
	SINT32 m_lMsgQOption;
	int m_fdTcpPeer;		//fd for tcp socket, connected to Peer
	int m_fdPipe;		//fd for pipe, for inter-task communication
	bool m_connected;	//当前是否与SAG建立了连接
	CMutex m_lock;

	UINT16 m_SPCCode;	//本端信令点编码
	UINT16 m_DPCCode;	//远端信令点编码
	char m_PeerIPAddr[16];	//IP address of DCS
	UINT16 m_PeerPort;	//TCP port of DCS
	UINT16 m_LocalPort;	//TCP local port
	
	static CDGrpLink* s_ptaskTDGrpLink;
	UINT8	m_AlarmExist;
	bool m_blNetCfgChanged;
	bool m_blNeedResetLink;

	TID m_selfTID;
	char m_szPipeName[20];
	NatPiSession* m_pNatpiSession;
	T_BtsDcsLinkCfg* m_pBtsDcsLinkCfg;

#ifdef __WIN32_SIM__
	int m_fdUdpSndToPipe;
#endif

	CDGrpLink();
	~CDGrpLink();
	bool Initialize();
	void SetEntityId(TID tid){m_selfTID=tid;};
	TID GetEntityId() const;// { return M_TID_VCR; }
	bool PostMessage(CComMessage* pMsg,	SINT32 timeout, bool isUrgent=false);

	bool IsMonitoredForDeadlock()  { return true; };
	int  GetMaxBlockedTime() { return TDGrpLink_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

	bool CreateSocket();
	bool ConnectPeer();
	bool DisconnectPeer();

	bool CreatePipe();
	bool DeletePipe();
	bool OpenPipe();
	bool ClosePipe();
	bool FlushPipe();

	void SetConnected(bool connected);
	void handleTcpError();

	int processOneMsgFromPeer(CComMessage *pComMsg);

	int sendOneMsgToPeer(CComMessage* pComMsg);
	
	bool send_TcpFuncTestPkt_Rsp(UINT8 msgType, UINT32 RAND);
	void SendDcsAlarmToOAM(UINT8 SetAlarm);
	bool IsNetCfgChanged();
	void reloadDcsCfg();
#ifdef DSP_BIOS
	static STATUS RunDcsSocketRx(CDGrpLink *);
	void SocketRxMainLoop();
	void tryToConnectToPeer();	
	//lijinan 20091118 add-----------
	inline bool IsNeedTransaction() const
	{
		return false;
	}
	//---------------------------------
#else
	CComMessage* readOneMsgFromPipe();
#endif
};


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __DATAGRPLINK_H */


