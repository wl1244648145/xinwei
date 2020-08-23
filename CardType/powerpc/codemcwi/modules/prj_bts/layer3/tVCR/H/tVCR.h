/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    tVCR.h
*
* DESCRIPTION: 
*		BTS上的tVCR任务类
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-9-19   fengbing  initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef	__TVCR_H
#define	__TVCR_H

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
#define TVCR_MAX_BLOCKED_TIME_IN_10ms_TICK	(200)//(2000)	//20seconds
#else
#define TVCR_MAX_BLOCKED_TIME_IN_10ms_TICK	(2000)	//20seconds
#endif

//VCR与SAG的TCP接口定义
#define M_TCP_PKT_BEGIN_FLAG			(0x7ea5)	//HEAD FALG取值为0x7ea5
#define M_TCP_PKT_END_FLAG				(0x7e0d)	//END FLAG取值为0x7e0d
#define M_TCP_PKT_SABIS1_USERTYPE		(0x1106)	//用户类型，SAbis1接口固定为0x1106
#define M_TCP_PKT_TEST_USERTYPE			(0x1110)	//TCP封装功能模块测试消息：0x1110

#define M_TCP_PKT_TEST_AUTH_REQ			(0x02)		//TCP连接认证请求
#define M_TCP_PKT_TEST_AUTH_RSP			(0x03)		//TCP连接认证应答

#define M_TCP_PKT_TEST_HEARTBEAT_REQ	(0x00)		//TCP连接握手请求
#define M_TCP_PKT_TEST_HEARTBEAT_RSP	(0x01)		//TCP连接握手应答

#define M_TCP_PKT_DEFAULT_TTL			(32)		//路由计数器：为防止TCP封装层用户路由错误出现循环路由，此计数器每经过一个节点减一，若为0则此包将被丢弃。此计数器的默认值为32。
#ifdef DSP_BIOS
#define localtime_r( _clock, _result ) \
	( *(_result) = *localtime( (_clock) ), \
	  (_result) )
#endif

extern bool sagStatusFlag;

//任务参数定义
#define M_TASK_NAME_LEN			(20)
#define M_TASK_TVCR_TASKNAME      	"tVCR"
#define M_TASK_TVCR1_TASKNAME      	"tVCR1"
#define M_TASK_TVCR_PRIORITY      	(95)
#ifdef __WIN32_SIM__
#define M_TASK_TVCR_OPTION        	(0x0008)
#define M_TASK_TVCR_MSGOPTION     	(0x02)
#elif __NUCLEUS__
#define M_TASK_TVCR_OPTION       	(NULL)	//not use
#define M_TASK_TVCR_MSGOPTION     	(NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_TVCR_OPTION        	(VX_FP_TASK)
#define M_TASK_TVCR_MSGOPTION     	( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_TVCR_STACKSIZE     	(20480)
#define M_TASK_TVCR_MAXMSG        	(1024)

#define M_VCR_PIPENAME				"/dev/VCRPipe"
#define M_VCR1_PIPENAME			"/dev/VCRPipe1"
#define M_WIN32_VCR_PIPEPORT		(6666)
#define M_WIN32_VCR1_PIPEPORT		(6667)

#ifdef DSP_BIOS
class CVCR:public CBizTask
#else
class CVCR:public CTask
#endif
{
public:
	static CVCR* GetInstance();
	static CVCR* GetBakInstance();
	void setTaskOptions();
	bool isMasterInstance(){return M_TID_VCR==m_selfTID;};
	bool isBackupInstance(){return M_TID_VCR1==m_selfTID;};	
	UINT32 getSAGID();
	void SendResetMsgToSAG();
	bool IsSAGConnected();
	UINT32 GetBtsSagIpAddr();
	void setTosVal(UINT8 tos);
	TcpEWithNatApRcvBuf m_RcvBuf;
protected:
	virtual void MainLoop();
private:
    SINT32 m_lMaxMsgs;
    SINT32 m_lMsgQOption;
	int m_fdTcpSag;		//fd for tcp socket, connected to SAG
	int m_fdPipe;		//fd for pipe, for inter-task communication
	bool m_connected;	//当前是否与SAG建立了连接
	CMutex m_lock;

	UINT16 m_SPCCode;	//本端信令点编码
	UINT16 m_DPCCode;	//远端信令点编码
	char m_SagIPAddr[16];	//IP address of SAG
	UINT16 m_SagPort;	//TCP port of SAG
	UINT16 m_LocalPort;	//TCP local port
	
	static CVCR* s_ptaskTVCR;
	static CVCR* s_ptaskTVCR1;

	ENUM_ResetCauseT m_ResetReason;
	UINT8	m_AlarmExist;
	bool m_blNetCfgChanged;
	bool m_blNeedResetLink;

	TID m_selfTID;
	char m_szPipeName[20];
	NatPiSession* m_pNatpiSession;
	T_BtsSagLinkCfg* m_pBtsSagLinkCfg;

#ifdef __WIN32_SIM__
	int m_fdUdpSndToPipe;
#endif

	CVCR();
	~CVCR();
	bool Initialize();
	void SetEntityId(TID tid){m_selfTID=tid;};
	TID GetEntityId() const;// { return M_TID_VCR; }
	bool PostMessage(CComMessage* pMsg,	SINT32 timeout, bool isUrgent=false);

	bool IsMonitoredForDeadlock()  { return true; };
	int  GetMaxBlockedTime() { return TVCR_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

	bool CreateSocket();
	bool ConnectSAG();
	bool DisconnectSAG();

	bool CreatePipe();
	bool DeletePipe();
	bool OpenPipe();
	bool ClosePipe();
	bool FlushPipe();

	void SetSAGConnected(bool connected);
	void handleTcpError();

	void processOneMsgFromPeer(CComMessage* pMsg);
	void sendOneSignalToTVoice(CComMessage* pComMsg);
	void sendOneSignalToUm(CComMessage* pComMsg);
	void processOneSignalFromSAG(CComMessage* pComMsg);
	int sendOneSignalToSAG(CComMessage* pComMsg);
	bool send_TcpFuncTestPkt_Rsp(UINT8 msgType, UINT32 RAND);
	void SendSAGAlarmToOAM(UINT8 SetAlarm);
	bool IsNetCfgChanged();
	void reloadVcrCfg();
#ifdef DSP_BIOS
	static STATUS RunVcrSocketRx(CVCR *);
	static STATUS RunVcr1SocketRx(CVCR *);
	void SocketRxMainLoop();
	void tryToConnectToSAG();	
	//lijinan 20091118 add-----------
    inline bool IsNeedTransaction() const
    {
    	return false;
    }
	//---------------------------------
#else
	CComMessage* readOneSignalFromPipe();
#endif
};
#endif /* __TVCR_H */
