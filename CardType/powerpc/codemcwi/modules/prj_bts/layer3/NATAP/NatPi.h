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
#include <string.h>
#include "ComMessage.h"
#include "Timer.h"
#include "VoiceToolFunc.h"
////////////////////////////////////////////////////////////////////////////////
#define NATAP_FUNCTION

//����NATAP session ��ص�û�л�Ĵ�������������Ϊsession��������Ҫ����session
#define M_MAX_NATAP_NOACTION_TIMES	(5)

#define M_NAT_INVALID_LINKID	(0xffff)
//ע�ᳬʱ���´�ע����,5s
#define M_TIMEOUT_NAT_REGLOOP		(5000)
//ע�ᳬʱʱ�䣬4s
#define M_TIMEOUT_NAT_REGRSP		(4000)
//���ּ��Ths����С��NAT���ص��ϻ�ʱ��,5s	
#define M_TIMEOUT_NAT_HANDSHAKE		(5000)
//����Ӧ��ȴ�ʱ��Ths_ack����С��NAT���ص��ϻ�ʱ��,4s	
#define M_TIMEOUT_NAT_HANDSHAKERSP	(4000)	
//��·ʧ����ʱ����Nhs
#define M_MAX_HANDSHAKE_UNREPLY		(3)		
//ע�������0xFFFFΪ��������
#define M_NAT_REG_RETRYFOREVER		(0xffff)

#define MSGID_TMOUT_REG_LOOP		(0xAAA0)
#define MSGID_TMOUT_REG_RSP			(0xAAA1)
#define MSGID_TMOUT_HANDSHAKE		(0xAAA2)
#define MSGID_TMOUT_HANDSHAKERSP	(0xAAA3)

#define MSGID_NATAP_RESTART		(0xAAAF)

#ifdef DSP_BIOS
#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif
#endif
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
	APP_PACKET=1,		//͸��Э���
	REG_MSG,
	REG_RSP_MSG,
	HANDSHAKE_MSG,
	HANDSHAKE_RSP_MSG,
	CLOSE_MSG
}NATAP_PKT_TYPE;

#define M_MAX_NATAP_PKTTYPE (CLOSE_MSG+1)

#define M_PROTOCOL_ID_NATAP	(7)
////////////////////////////////////////////////////////////////////////////////

typedef  UINT16 NATAP_LinkIDT;
typedef struct _AH_T
{
	UINT8 protocolID:3;	//7����NatApЭ��
	UINT8 SPI:5;
	UINT8 nLen[2];	//NAT msg length,include self len
	UINT8 msgType;
}AH_T;	//��Ȩͷ

//4.2.3.5.11   REGISTER
//��Ԫ	����	����	˵��
//TRANS ID	M	2 byte	����ţ�����ƥ��Ӧ��
//LOCAL IP	M	4 byte	���أ�˽����IP��ַ
//LOCAL PORT	M	2 byte	���أ�˽����PORT

typedef struct _NAT_Reg_Data_T
{
	UINT8 transID[2];
	UINT8 localIP[4];
	UINT8 localPort[2];
}NAT_Reg_Data_T;

//4.2.3.5.12   REGISTER_ACK
//��Ԫ	����	����	˵��
//TRANS ID	M	2 byte	����ţ�����ƥ������
//RESULT	M	1 byte	ע������
//0�ɹ����п�ѡ�ֶ�
//1�Ƿ���Ϣ
//2����δ����
//LINK ID	O	2 byte 	NatAp����·��ʶ
//PUB IP	O	4 byte	NATת�󣨹�����IP��ַ
//PUB PORT	O	2 byte	NATת�󣨹�����PORT
typedef struct _NAT_RegRsp_Data_T
{
	UINT8 transID[2];
	UINT8 result;
	UINT8 linkID[2];
	UINT8 pubIP[4];
	UINT8 pubPort[2];
}NAT_RegRsp_Data_T;

//4.2.3.5.13   HANDSHAKE
//��Ԫ	����	����	˵��
//SEQUENCE	M	4 byte	������ţ�����ƥ��Ӧ��
//LINK ID	M	2 byte	��·��ʶ
typedef struct _NAT_HandShake_Data_T
{
	UINT8 SN[4];
	UINT8 linkID[2];
}NAT_HandShake_Data_T;

//4.2.3.5.14   HANDSHAKE_ACK
//��Ԫ	����	����	˵��
//SEQUENCE	M	4 byte	������ţ�����ƥ������
//LINK ID	M	2 byte	��·��ʶ
typedef struct _NAT_HandShakeRsp_Data_T
{
	UINT8 SN[4];
	UINT8 linkID[2];
}NAT_HandShakeRsp_Data_T;

//4.2.3.1.9   CLOSE
//��Ԫ	����	����	˵��
//LINK ID	M	2 byte	��·��ʶ
//REASON	M	1 byte	�ͷ�ԭ��
//1OAM����
//2Nat Ap�쳣
//ע������Ϣһ�����UDP���ء�
typedef struct _NAT_Close_Data_T
{
	UINT8 linkID[2];
	UINT8 reason;
}NAT_Close_Data_T;

//------------------------------------------------

//NAT_Reg_Pkt
typedef struct _NAT_Reg_Pkt_T
{
	AH_T AHHead;
	UINT8 AuthSN[4];
	NAT_Reg_Data_T NatRegData;
	UINT8 AuthResult[100];
}NAT_Reg_Pkt_T;

//NAT_RegRsp_Pkt
typedef struct _NAT_RegRsp_Pkt_T
{
	AH_T AHHead;
	UINT8 AuthSN[4];
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

//NAT͸��Ӧ�ò㱨��,����Ŀǰ�汾��udp��dmux������NAT��װ��tcp���������NAT��װ
typedef struct _NAT_App_Pkt_T
{
	AH_T AHHead;
	UINT8 linkID[2];
	UINT8 buf[500];
}NAT_App_Pkt_T;


//����CallBack ���͵ĺ���ָ��,��NATAPע��ɹ�ʱ�Ļص�����
typedef int(*RegSuccessCallBack)(void *p);

////////////////////////////////////////////////////////////////////////////////

class NatPiSession
{
public:
	NatPiSession(bool isServer, NAT_APP_TYPE appType, TID dstTaskID, char* name)
		:m_AppType(appType),m_dstTaskID(dstTaskID),m_blIsServer(isServer)
	{
		memset(m_name, 0, sizeof(m_name));
		strncpy(m_name,name, min(strlen(name),sizeof(m_name)-1));
		setEffect(true);
		for(int i=0;i<TIMER_NAT_COUNT;i++)
		{
			m_pTimer[i] = NULL;
		}
	};
	
	void init(UINT32 localIP,UINT16 localPort,UINT32 peerIP,UINT16 peerPort,
			  RegSuccessCallBack cbFunc=NULL);
	void setSPIVal(UINT8 spiV);
	UINT8 getSPIVal(){return m_nSPI;};
	void setSocket(int fdSocket){m_fdSocket=fdSocket;};
	void start();
	void stop();
	bool isSessionAlive();//��Բ����г��ֵ�VDR����û����������Ҳ������NATAP��Ϣ����Ҳû���յ�NATAP�Ķ�ʱ����Ϣ������

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
	void handleRegRspMsg(UINT8* pMsg, UINT16 len);
	void handleHandShakeMsg(UINT8* pMsg, UINT16 len);
	void handleHandShakeRspMsg(UINT8* pMsg, UINT16 len);	
	void handleCloseMsg(UINT8* pMsg, UINT16 len);

	NATAP_PKT_TYPE authenticateNATAPMsg(UINT8* pMsg, UINT16 len);
	UINT8 packNATAPMsg4AppPkt(UINT8* pDataApp, UINT16 len);//now only for tcp

	void handleTmoutRegRsp();
	void handleTmoutRegLoop();
	void handleTmoutHandShakeRsp();
	void handleTmoutHandShakeLoop();

	void handleNATAPTmoutMsg(UINT16 msgID);
	void handleNATAPMsg(UINT8* pMsg, UINT16 len);

	void updateRemoteAddr();

	bool IsServer(){return m_blIsServer;};
	void SetServerFlag(bool isServer){m_blIsServer = isServer;};
	//name
	inline char* getName(){return m_name;};
	//tcp or udp
	inline bool isTcpApp(){return NAT_APP_TCP==m_AppType;};
	inline bool isUdpApp(){return NAT_APP_UDP==m_AppType;};
	//�Ƿ�ע��ɹ�
	inline bool isRegistered(){return m_blRegistered;};
	inline void setRegisteredFlag(bool registered){m_blRegistered=registered;};
	//��·ID
	inline void setLinkID(UINT16 linkID){m_LinkID=linkID;};
	inline UINT16 getLinkID(){return m_LinkID;};
	//������֤���
	inline void setLocalAHSN(UINT32 val){m_localAHSN=val;};
	inline UINT32 getLocalAHSN(){return m_localAHSN;};
	inline void incLocalAHSN(){m_localAHSN++;};
	//Զ����֤���
	inline void setPeerAHSN(UINT32 val){m_peerAHSN=val;};
	inline UINT32 getPeerAHSN(){return m_peerAHSN;};
	//ע�������
	inline void setTransSN(UINT16 val){m_TransSN=val;};
	inline UINT16 getTransSN(){return m_TransSN++;}; 
	//�����������
	inline void setHandShakeSN(UINT32 val){m_HandShakeSN=val;};
	inline UINT32 getHandShakeSN(){return m_HandShakeSN++;};
	//��ǰ����δӦ�����
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
	UINT8 m_nSPI;
	int m_fdSocket;				//socket used
	struct sockaddr_in m_RemoteAddr;	//for udp socket
	CTimer *m_pTimer[TIMER_NAT_COUNT];
	
	NAT_APP_TYPE m_AppType;		//tcp or udp
	TID m_dstTaskID;			//timeout msg dstTaskId
	
	bool m_blRegistered;		//�Ƿ�ע��ɹ�
	UINT16 m_LinkID;			//��·ID
	
	UINT32 m_localAHSN;			//������֤���
	UINT32 m_peerAHSN;			//Զ����֤���
	UINT16 m_TransSN;			//ע�������
	UINT32 m_HandShakeSN;		//�����������
	
	UINT32 m_localIP;
	UINT16 m_localPort;
	UINT32 m_peerIP;
	UINT16 m_peerPort;
	
	int m_nHandShakeLost;	//��ǰ����δӦ�����

	bool m_blIsServer;		//NATAP server or not?
	bool m_blInEffect;		//�Ƿ���NATAP����
	RegSuccessCallBack m_callbackFuncRegSuccess ; //ע��ɹ���ص�����ָ��

	UINT32 m_nRcvNatApPktCounters[M_MAX_NATAP_PKTTYPE];
	UINT32 m_nSndNatApPktCounters[M_MAX_NATAP_PKTTYPE];
	static char m_PktName[M_MAX_NATAP_PKTTYPE][20];
	
	UINT32 m_nRcvNatApPktCounters_bak[M_MAX_NATAP_PKTTYPE];
	UINT32 m_nSndNatApPktCounters_bak[M_MAX_NATAP_PKTTYPE];
	UINT32 m_nNoNatApActionTimes;
	
protected:
	
};


#ifdef __cplusplus
extern "C" {
#endif

void setNatApHandShakeFlag(bool flag);
void showNatApCfg();
void setNatApTimer(NATTimerID timerID, UINT8 nSeconds);
void setMaxHandShakeLostNum(UINT8 nTimes);

#ifdef __cplusplus
}
#endif

#endif /* __INC_NATPI_H */

