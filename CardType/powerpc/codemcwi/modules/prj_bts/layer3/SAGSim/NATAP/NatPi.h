/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    NatPi.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   2006-11-27 fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/
 

#ifndef __INC_NATPI_H
#define __INC_NATPI_H

/* includes */
#include "ComMessage.h"
#include "Timer.h"

#ifdef __WIN32_SIM__

//#include "winsock2.h"

#else

#include "inetLib.h"

#endif
////////////////////////////////////////////////////////////////////////////////
#define NATAP_FUNCTION

#define M_NAT_INVALID_LINKID	(0xffff)
//注册超时后下次注册间隔,10s
#define M_TIMEOUT_NAT_REGLOOP		(10000)
//注册超时时间，8s
#define M_TIMEOUT_NAT_REGRSP		(8000)
//握手间隔Ths，需小于NAT网关的老化时间,10s	
#define M_TIMEOUT_NAT_HANDSHAKE		(10000)
//握手应答等待时间Ths_ack，需小于NAT网关的老化时间,8s	
#define M_TIMEOUT_NAT_HANDSHAKERSP	(8000)	
//链路失连超时次数Nhs
#define M_MAX_HANDSHAKE_UNREPLY		(3)		
//注册次数，0xFFFF为反复尝试
#define M_NAT_REG_RETRYFOREVER		(0xffff)

#define MSGID_TMOUT_REG_LOOP		(0xAAA0)
#define MSGID_TMOUT_REG_RSP			(0xAAA1)
#define MSGID_TMOUT_HANDSHAKE		(0xAAA2)
#define MSGID_TMOUT_HANDSHAKERSP	(0xAAA3)

enum
{
	NATAP_REG_SUCCESS=0,
	NATAP_REG_INVALID,
	NATAP_REG_NOCFG
};

typedef enum _enum_NATTimerID
{
	TIMER_NAT_REGLOOP=0,
	TIMER_NAT_REGRSP,
	TIMER_NAT_HANDSHAKE,
	TIMER_NAT_HANDSHAKERSP,
	TIMER_NAT_COUNT
}NATTimerID;

typedef struct _TimerCfgItemT
{
	NATTimerID timerID;
	UINT16 msgID;
	UINT32 timeoutVal;
	CComMessage* pTimeoutMsg;	
}TimerCfgItemT;

typedef enum
{
	NAT_APP_UDP,
	NAT_APP_TCP
}NAT_APP_TYPE;

typedef enum
{
	INVALID_NATAP_PKT=0,
	APP_PACKET=1,		//透传协议包
	REG_MSG,
	REG_RSP_MSG,
	HANDSHAKE_MSG,
	HANDSHAKE_RSP_MSG,
	CLOSE_MSG
}NATAP_PKT_TYPE;

#define M_MAX_NATAP_PKTTYPE (CLOSE_MSG+1)

#define M_PROTOCOL_ID_NATAP	(7)
////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif
typedef  UINT16 NATAP_LinkIDT;
typedef struct _AH_T
{
	UINT8 protocolID:3;	//8代表NatAp协议
	UINT8 SPI:5;
	UINT16 nLen;	//NAT msg length,include self len
	UINT8 msgType;		
}AH_T;	//鉴权头

//4.2.3.5.11   REGISTER
//信元	类型	长度	说明
//TRANS ID	M	2 byte	事务号，用以匹配应答
//LOCAL IP	M	4 byte	本地（私网）IP地址
//LOCAL PORT	M	2 byte	本地（私网）PORT

typedef struct _NAT_Reg_Data_T
{
	UINT16 transID;
	UINT32 localIP;
	UINT16 localPort;
}NAT_Reg_Data_T;

//4.2.3.5.12   REGISTER_ACK
//信元	类型	长度	说明
//TRANS ID	M	2 byte	事务号，用以匹配请求
//RESULT	M	1 byte	注册结果，
//0成功，有可选字段
//1非法消息
//2网管未配置
//LINK ID	O	2 byte 	NatAp的链路标识
//PUB IP	O	4 byte	NAT转后（公网）IP地址
//PUB PORT	O	2 byte	NAT转后（公网）PORT
typedef struct _NAT_RegRsp_Data_T
{
	UINT16 transID;
	UINT8 result;
	NATAP_LinkIDT linkID;
	UINT32 pubIP;
	UINT16 pubPort;
}NAT_RegRsp_Data_T;

//4.2.3.5.13   HANDSHAKE
//信元	类型	长度	说明
//SEQUENCE	M	4 byte	握手序号，用以匹配应答
//LINK ID	M	2 byte	链路标识
typedef struct _NAT_HandShake_Data_T
{
	UINT32 SN;
	NATAP_LinkIDT linkID;
}NAT_HandShake_Data_T;

//4.2.3.5.14   HANDSHAKE_ACK
//信元	类型	长度	说明
//SEQUENCE	M	4 byte	握手序号，用以匹配请求
//LINK ID	M	2 byte	链路标识
typedef struct _NAT_HandShakeRsp_Data_T
{
	UINT32 SN;
	NATAP_LinkIDT linkID;
}NAT_HandShakeRsp_Data_T;

//4.2.3.1.9   CLOSE
//信元	类型	长度	说明
//LINK ID	M	2 byte	链路标识
//REASON	M	1 byte	释放原因
//1OAM发起
//2Nat Ap异常
//注：该消息一般针对UDP承载。
typedef struct _NAT_Close_Data_T
{
	NATAP_LinkIDT linkID;
	UINT8 reason;
}NAT_Close_Data_T;

//------------------------------------------------

//NAT_Reg_Pkt
typedef struct _NAT_Reg_Pkt_T
{
	AH_T AHHead;
	UINT32 AuthSN;
	NAT_Reg_Data_T NatRegData;
	UINT8 AuthResult[100];
}NAT_Reg_Pkt_T;

//NAT_RegRsp_Pkt
typedef struct _NAT_RegRsp_Pkt_T
{
	AH_T AHHead;
	UINT32 AuthSN;
	NAT_RegRsp_Data_T NatRegRspData;
	UINT8 AuthResult[100];
}NAT_RegRsp_Pkt_T;

//NAT_HandShake_Pkt
typedef struct _NAT_HandShake_Pkt_T
{
	AH_T AHHead;
	NAT_HandShake_Data_T NatHandShakeData;
}NAT_HandShake_Pkt_T;

//NAT_HandShakeRsp_Pkt
typedef struct _NAT_HandShakeRsp_Pkt_T
{
	AH_T AHHead;
	NAT_HandShakeRsp_Data_T NatHandShakeRspData;
}NAT_HandShakeRsp_Pkt_T;

//NAT_Close_Pkt
typedef struct _NAT_Close_Pkt_T
{
	AH_T AHHead;
	NAT_Close_Data_T NatCloseData;
}NAT_Close_Pkt_T;

//NAT透传应用层报文,对于目前版本，udp的dmux包不做NAT封装，tcp的信令包做NAT封装
typedef struct _NAT_App_Pkt_T
{
	AH_T AHHead;
	NATAP_LinkIDT linkID;
	UINT8 buf[500];
}NAT_App_Pkt_T;

#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

//声明CallBack 类型的函数指针,当NATAP注册成功时的回调函数
typedef int(*RegSuccessCallBack)(void *p);

////////////////////////////////////////////////////////////////////////////////

class NatPiSession
{
public:
	NatPiSession(NAT_APP_TYPE appType, TID dstTaskID, char* name)
		:m_AppType(appType),m_dstTaskID(dstTaskID)
	{
		strncpy(m_name,name, min(strlen(name),sizeof(m_name)-1));
		setEffect(true);
	};
	
	void init(UINT32 localIP,UINT16 localPort,UINT32 peerIP,UINT16 peerPort,
			  RegSuccessCallBack cbFunc=NULL);
	void setSocket(int fdSocket){m_fdSocket=fdSocket;};
	void start();
	void stop();

	void initAllTimers();
	void clearAllTimers();
	void startTimer(NATTimerID timerID);
	void stopTimer(NATTimerID timerID);
	void deleteTimer(NATTimerID timerID);

	void showStatus();

	int sendNatPiPkt(UINT8 *pDataToSend, UINT16 nLenToSend);
	void sendRegMsg();
	void sendHandShakeMsg();
	void sendHandShakeRspMsg(UINT32 peerSN);
	void sendRegRspMsg(UINT16 transID);
	void handleRegMsg(UINT8* pMsg, UINT16 len);
	void handleRegRspMsg(UINT8* pMsg, UINT16 len);
	void handleHandShakeMsg(UINT8* pMsg, UINT16 len);
	void handleHandShakeRspMsg(UINT8* pMsg, UINT16 len);	
	void handleCloseMsg(UINT8* pMsg, UINT16 len);

	NATAP_PKT_TYPE authenticateNATAPMsg(UINT8* pMsg, UINT16 len);
	UINT8 packNATAPMsg4AppPkt(UINT8* pDataApp);//now only for tcp

	void handleTmoutRegRsp();
	void handleTmoutRegLoop();
	void handleTmoutHandShakeRsp();
	void handleTmoutHandShakeLoop();

	void handleNATAPTmoutMsg(UINT16 msgID);
	void handleNATAPMsg(UINT8* pMsg, UINT16 len);

	void updateRemoteAddr();

	//name
	inline char* getName(){return m_name;};
	//tcp or udp
	inline bool isTcpApp(){return NAT_APP_TCP==m_AppType;};
	inline bool isUdpApp(){return NAT_APP_UDP==m_AppType;};
	//是否注册成功
	inline bool isRegistered(){return m_blRegistered;};
	inline void setRegisteredFlag(bool registered){m_blRegistered=registered;};
	//链路ID
	inline void setLinkID(UINT16 linkID){m_LinkID=linkID;};
	inline UINT16 getLinkID(){return m_LinkID;};
	//本端认证序号
	inline void setLocalAHSN(UINT32 val){m_localAHSN=val;};
	inline UINT32 getLocalAHSN(){return m_localAHSN;};
	inline void incLocalAHSN(){m_localAHSN++;};
	//远端认证序号
	inline void setPeerAHSN(UINT32 val){m_peerAHSN=val;};
	inline UINT32 getPeerAHSN(){return m_peerAHSN;};
	//注册事务号
	inline void setTransSN(UINT16 val){m_TransSN=val;};
	inline UINT16 getTransSN(){return m_TransSN++;}; 
	//本端握手序号
	inline void setHandShakeSN(UINT32 val){m_HandShakeSN=val;};
	inline UINT32 getHandShakeSN(){return m_HandShakeSN++;};
	//当前握手未应答次数
	inline void clearHSLostCounter(){m_nHandShakeLost=0;};
	inline void incHSLostCounter(){m_nHandShakeLost++;};
	inline int getHSLostCounter(){return m_nHandShakeLost;};
	//ip&port,local&peer
	inline void setLocalIP(UINT32 ip){m_localIP=ip;};
	inline UINT32 getLocalIP(){return m_localIP;};
	inline void setLocalPort(UINT16 port){m_localPort=port;};
	inline UINT16 getLocalPort(){return m_localPort;};
	inline void setPeerIP(UINT32 ip){m_peerIP=ip;};
	inline UINT32 getPeerIP(){return m_peerIP;};
	inline void setPeerPort(UINT16 port){m_peerPort=port;};
	inline UINT16 getPeerPort(){return m_peerPort;};
	inline void setEffect(bool effective){m_blInEffect=effective;};
	bool IsInEffect(){return m_blInEffect;};
	void clearCounters();
	
private:
	char m_name[30];
	int m_fdSocket;				//socket used
	struct sockaddr_in m_RemoteAddr;	//for udp socket
	CTimer *m_pTimer[TIMER_NAT_COUNT];
	
	NAT_APP_TYPE m_AppType;		//tcp or udp
	TID m_dstTaskID;			//timeout msg dstTaskId
	
	bool m_blRegistered;		//是否注册成功
	UINT16 m_LinkID;			//链路ID
	
	UINT32 m_localAHSN;			//本端认证序号
	UINT32 m_peerAHSN;			//远端认证序号
	UINT16 m_TransSN;			//注册事务号
	UINT32 m_HandShakeSN;		//本端握手序号
	
	UINT32 m_localIP;
	UINT16 m_localPort;
	UINT32 m_peerIP;
	UINT16 m_peerPort;
	
	int m_nHandShakeLost;	//当前握手未应答次数

	bool m_blInEffect;		//是否开启NATAP功能
	RegSuccessCallBack m_callbackFuncRegSuccess ; //注册成功后回调函数指针

	UINT32 m_nRcvNatApPktCounters[M_MAX_NATAP_PKTTYPE];
	UINT32 m_nSndNatApPktCounters[M_MAX_NATAP_PKTTYPE];
	static char m_PktName[M_MAX_NATAP_PKTTYPE][20];
	
protected:
	
};


//global obj for voice SAbis1 tcp NATAP
extern NatPiSession g_VCR_NatAPc;
//global obj for voice DMUX udp NATAP
extern NatPiSession g_VDR_NatAPc;

#ifdef __cplusplus
extern "C" {
#endif

void showNatApInfo();
void setNatApTimer(NATTimerID timerID, UINT8 nSeconds);
void setMaxHandShakeLostNum(UINT8 nTimes);
void clearNatApPktCounters();
void closeNatApFun();
void closeVDRNatApFunc();
void closeVCRNatApFunc();

#ifdef __cplusplus
}
#endif

#endif /* __INC_NATPI_H */

