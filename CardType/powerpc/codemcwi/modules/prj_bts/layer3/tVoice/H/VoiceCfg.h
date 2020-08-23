/*******************************************************************************
* Copyright (c) 2009 by Beijing Arrowping Communication Co.Ltd.All Rights Reserved   
* File Name      : VoiceCfg.h
* Create Date    : 26-Jun-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__VOICECFG_H
#define	__VOICECFG_H
#include "dataType.h"

struct T_BtsDcsLinkCfg
{
	UINT32 DCS_IP;
	UINT16 DCS_Port;
	UINT16 BTS_Port;
	UINT8 NatApKey;
};

struct T_BtsSagLinkCfg
{  
	UINT32 BtsIPAddr;	
	UINT32 SAGID;
	UINT32 SAGVoiceIP;
	UINT32 SAGSignalIP;
	UINT16 SAGRxPortV;          //UDP port used to communicate with SAG Voice Rx
	UINT16 SAGTxPortV;          //UDP port used to communicate with SAG Voice Tx
	UINT16 SAGRxPortS;          //UDP port used to communicate with SAG Signal  Rx
	UINT16 SAGTxPortS;          //UDP port used to communicate with SAG Signal  Tx
	UINT16 SAGSPC;			//SAGsignal point code    2       
	UINT16 BTSSPC;			//BTS signal point code   2       
	UINT8   NatAPKey;
	UINT8   SAGVlanUsage;    // 0 - no use 1 - usage	
};

struct T_BTSVoiceSrvCfg
{
	UINT8 nTosVal;		//tos value,0-64,default value must be  0
	UINT8 blForceUseJBuf;	//0:not use;i:use;default 0 //强制各种终端都启用下行语音jitter buffer
	UINT8 blCpeZUseJBuf;//0:not use;i:use;default 0 //cpeZ启用下行语音jitter buffer
	UINT16 JBufSize;		//Jitter buffer size(how many 10ms frames),should be in set [16,32,64,128,256]
	UINT8 nFramesToStartTx;//how many frames to buffer before to start downlink TX,should be less than JbufSize/2
};

extern T_BtsDcsLinkCfg	g_vDcsBtsLinkCfg;	//DCS-BTS
extern T_BtsSagLinkCfg	g_vSagBtsLinkCfg1;		//master SAG
extern T_BtsSagLinkCfg	g_vSagBtsLinkCfg2;		//backup SAG
extern T_BTSVoiceSrvCfg g_vSrvCfg;			//BTS voice serverice options

#ifdef __cplusplus
extern "C" {
#endif

void updateVoiceCfgs();
void updateDcsCfg();





#ifdef __cplusplus
}
#endif

#endif /* __VOICECFG_H */


