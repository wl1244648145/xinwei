/*******************************************************************************
* Copyright (c) 2009 by Beijing  Arrowping Communication Co.Ltd.All Rights Reserved   
* File Name      : VoiceCfg.cpp
* Create Date    : 26-Jun-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "VoiceCfg.h"
#include "VoiceToolFunc.h"
#include "L3OamCfgCommon.h"
#include "sysBtsConfigData.h"

extern T_NvRamData *NvRamDataAddr;
extern void initRsvVoiceCfgFromNvRam();

T_BtsDcsLinkCfg	g_vDcsBtsLinkCfg={0,0,0,1};	//DCS-BTS
T_BtsSagLinkCfg	g_vSagBtsLinkCfg1;	//master SAG
T_BtsSagLinkCfg	g_vSagBtsLinkCfg2;	//backup SAG
T_BTSVoiceSrvCfg g_vSrvCfg;			//BTS voice serverice options

void updateVoiceCfgs()
{	
	//bts非网管配置项初始化
	initRsvVoiceCfgFromNvRam();

//master SAG
#if 1
	g_vSagBtsLinkCfg1.BtsIPAddr = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.BtsIPAddr);
	g_vSagBtsLinkCfg1.SAGID = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGID);
	g_vSagBtsLinkCfg1.SAGVoiceIP = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGVoiceIP);
	g_vSagBtsLinkCfg1.SAGSignalIP = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGSignalIP);
	g_vSagBtsLinkCfg1.SAGRxPortV = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGRxPortV);
	g_vSagBtsLinkCfg1.SAGTxPortV = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGTxPortV);
	g_vSagBtsLinkCfg1.SAGRxPortS = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGRxPortS);
	g_vSagBtsLinkCfg1.SAGTxPortS = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGTxPortS);
	g_vSagBtsLinkCfg1.SAGSPC = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGSPC);
	g_vSagBtsLinkCfg1.BTSSPC = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.BTSSPC);
	g_vSagBtsLinkCfg1.NatAPKey = NvRamDataAddr->BtsGDataCfgEle.NatAPKey;
	g_vSagBtsLinkCfg1.SAGVlanUsage = NvRamDataAddr->BtsGDataCfgEle.SAGVlanUsage;	
#endif	
//backup SAG
#if 1
	g_vSagBtsLinkCfg2.BtsIPAddr = g_vSagBtsLinkCfg1.BtsIPAddr; //VGetU32BitVal((UINT8*)&NvRamDataAddr->SagBkp.BtsIPAddr);
	g_vSagBtsLinkCfg2.SAGID = VGetU32BitVal((UINT8*)&NvRamDataAddr->SagBkp.ulSagID);
	g_vSagBtsLinkCfg2.SAGVoiceIP = VGetU32BitVal((UINT8*)&NvRamDataAddr->SagBkp.ulSagVoiceAddr);
	g_vSagBtsLinkCfg2.SAGSignalIP = VGetU32BitVal((UINT8*)&NvRamDataAddr->SagBkp.ulSagSignalAddr);
	g_vSagBtsLinkCfg2.SAGRxPortV = VGetU16BitVal((UINT8*)&NvRamDataAddr->SagBkp.usVoiceRxPort);
	g_vSagBtsLinkCfg2.SAGTxPortV = VGetU16BitVal((UINT8*)&NvRamDataAddr->SagBkp.usVoiceTxPort);
	g_vSagBtsLinkCfg2.SAGRxPortS = VGetU16BitVal((UINT8*)&NvRamDataAddr->SagBkp.usSignalRxPort);
	g_vSagBtsLinkCfg2.SAGTxPortS = VGetU16BitVal((UINT8*)&NvRamDataAddr->SagBkp.usSignalTxPort);
	g_vSagBtsLinkCfg2.SAGSPC = VGetU16BitVal((UINT8*)&NvRamDataAddr->SagBkp.ulSagSignalPointCode);
	g_vSagBtsLinkCfg2.BTSSPC = VGetU16BitVal((UINT8*)&NvRamDataAddr->SagBkp.ulBtsSignalPointCode);
	g_vSagBtsLinkCfg2.NatAPKey = NvRamDataAddr->SagBkp.NatAPKey;
	g_vSagBtsLinkCfg2.SAGVlanUsage = g_vSagBtsLinkCfg1.SAGVlanUsage; //NvRamDataAddr->SagBkp.SAGVlanUsage;
#endif	

//BTS voice serverice options
#if 1	
	g_vSrvCfg.nTosVal = NvRamDataAddr->SagTos.ucSagTosVoice;
	g_vSrvCfg.blForceUseJBuf = VGetU16BitVal((UINT8*)&NvRamDataAddr->JitterBuf.usJtrBufEnable);
	g_vSrvCfg.blCpeZUseJBuf = VGetU16BitVal((UINT8*)&NvRamDataAddr->JitterBuf.usJtrBufZEnable);
	g_vSrvCfg.JBufSize = VGetU16BitVal((UINT8*)&NvRamDataAddr->JitterBuf.usJtrBufLength);
	g_vSrvCfg.nFramesToStartTx = VGetU16BitVal((UINT8*)&NvRamDataAddr->JitterBuf.usJtrBufPackMax);
#endif	
//for test
#if 0
	T_BtsSagLinkCfg *pCfg;

	pCfg = &g_vSagBtsLinkCfg1;
	
	pCfg->BtsIPAddr = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.BtsIPAddr);
	pCfg->SAGID = 101;
	pCfg->SAGVoiceIP = 0xAC100842;
	pCfg->SAGSignalIP = 0xAC100842;
	pCfg->SAGRxPortV = 9876;
	pCfg->SAGTxPortV = 9876;
	pCfg->SAGRxPortS = 6893;
	pCfg->SAGTxPortS = 6893;
	pCfg->SAGSPC = 101;
	pCfg->BTSSPC = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.BTSSPC);
	pCfg->NatAPKey = 1;
	pCfg->SAGVlanUsage = 0;
	
	pCfg = &g_vSagBtsLinkCfg2;
	
	pCfg->BtsIPAddr = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.BtsIPAddr);
	pCfg->SAGID = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGID);
	pCfg->SAGVoiceIP = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGVoiceIP);
	pCfg->SAGSignalIP = VGetU32BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGSignalIP);
	pCfg->SAGRxPortV = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGRxPortV);
	pCfg->SAGTxPortV = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGTxPortV);
	pCfg->SAGRxPortS = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGRxPortS);
	pCfg->SAGTxPortS = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGTxPortS);
	pCfg->SAGSPC = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.SAGSPC);
	pCfg->BTSSPC = VGetU16BitVal((UINT8*)&NvRamDataAddr->BtsGDataCfgEle.BTSSPC);
	pCfg->NatAPKey = NvRamDataAddr->BtsGDataCfgEle.NatAPKey;
	pCfg->SAGVlanUsage = NvRamDataAddr->BtsGDataCfgEle.SAGVlanUsage;
#endif
}

void updateDcsCfg()
{
	if(M_DCS_CFG_VALID_FLAG==VGetU32BitVal(NvRamDataAddr->dcsCfgBuffer.validFlag))
	{
		g_vDcsBtsLinkCfg.DCS_IP = VGetU32BitVal(NvRamDataAddr->dcsCfgBuffer.dcsCfg.DCS_IP);
		g_vDcsBtsLinkCfg.DCS_Port = VGetU16BitVal(NvRamDataAddr->dcsCfgBuffer.dcsCfg.DCS_Port);
		g_vDcsBtsLinkCfg.BTS_Port = VGetU16BitVal(NvRamDataAddr->dcsCfgBuffer.dcsCfg.BTS_Port);
		g_vDcsBtsLinkCfg.NatApKey = NvRamDataAddr->dcsCfgBuffer.dcsCfg.NatApKey;
	}
}

