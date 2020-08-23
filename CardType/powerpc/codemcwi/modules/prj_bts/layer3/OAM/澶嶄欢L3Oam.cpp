/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifdef __WIN32_SIM__
#include <windows.h>
#else
#ifndef __INCtaskLibh
#include <taskLib.h>
#endif
#include <sysLib.h>
#endif

#include <iostream>
#include <iomanip>
#include <string.h>
#include <loginLib.h>

#if !defined(__WIN32_SIM__)&&!defined(__NUCLEUS__)
#include <ioLib.h>
#include <stat.h>
#include <netDrv.h>
#include <fiolib.h>
#include <ramDrv.h>
#include <dosFsLib.h>
#include <remLib.h>
#include <taskLib.h>
#include <inetLib.h>
#include <vmLib.h>
#include <dirent.h>
#include <stat.h>
#endif


#ifndef _INC_MSGQUEUE
#include "MsgQueue.h"
#endif

#ifndef _INC_L3OAMTEST
//#include "L3OamTest.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMCFG
#include "L3OamCfg.h"
#endif

#ifndef _INC_ERRORCODEDEF
#include "ErrorCodeDef.h"
#endif

#ifndef _INC_L3OAMALMHANDLENOTIFY
#include "L3OamAlarmHandleNotify.h"
#endif

#ifndef _INC_L3OAMALMNOTIFYOAM
#include "L3OamAlmNotifyOam.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

#ifndef _INC_L3OAMGETBOARDSSTATERSP
#include "L3OamGetBoardsStateRsp.h"
#endif

#ifndef __L3_DATAREFRESH_JAMMINGNEGHBOR_H__
#include "L3DataNotifyRefreshJammingNeighbor.h"
#endif

#ifdef M_TGT_WANIF
#ifndef _INC_L3OAM_CFG_WANIF_CPE_REQ
#include "L3L2CfgWanIfCpeEidReq.h"
#endif
#endif

#ifndef _INC_L3_BTS_PM
#include "L3_BTS_PM.h"
#endif

#include "L3BootTask.h"
#include "L3OamCfgGetRFData.h"
#include "L3VoiceMsgID.h"
#include "l3datasockettable.h"

#define GPS_CFG_DATA_ERR_ALM_PERIOD    (10*60 * 1000)  //10分钟告警一次
#define SAG_BACKUP_FLAG 0x88

#ifndef BTS_CONFIG_DATA_H__
#include "sysBtsConfigData.h"
#endif
#ifndef WBBU_CODE
#define SAVE_POWER_L2_L3_TRANSID 0xEEEE
#define SAVE_POWER_CPU_ENABLE 0x00415080
#define SAVE_POWER_CPU_DISABLE 0x00015080
#define SAVE_POWER_CPU_ENABLE_RESULT 0x00416080
#define L2_L3_SAVE_POWER_ENABLE 1
#define L2_L3_SAVE_POWER_DISABLE 0
#define L2_L3_SAVE_POWER_SUCCESS 0
#define L2_L3_SAVE_POWER_FAIL 1
#define SAVE_POWER_FAN_PERIOD    (5 * 60 * 1000)  
#define SAVE_POWER_HW_NOT_SUPPORT  0x0002
#define SAVE_POWER_SW_NOT_SUPPORT  0x0003
//IMPORT UINT32 vxHid1Set (UINT32);
//IMPORT UINT32 vxHid1Get (void);
#include <vxWorks.h>
#include "vxPpcLib.h"
#endif
#ifndef __WIN32_SIM__
#include "mcWill_bts.h"
#endif
//#ifndef WBBU_CODE
//将nvram配置数据保存的nvram备份机制添加
/*nvram数据备份到cf卡的文件夹jiaying20110217*/
#define SM_NVRAM_CFG_DATA_BAK_DIR "/ata0a/nvdatabak/" 
/*nvram数据备份到cf卡的文件1 jiaying20100820*/
#define SM_NVRAM_CFG_DATA_FILENAME1 "nvdatabak1" 
/*nvram数据备份到cf卡的文件2 jiaying20100820*/
#define SM_NVRAM_CFG_DATA_FILENAME2 "nvdatabak2" 
#define SM_NVRAM_CFG_DATA_FILENAME3 "nvdatabak3" 
//#endif
#ifndef __WIN32_SIM__
T_NvRamData *NvRamDataAddr = (T_NvRamData*)NVRAM_BASE_ADDR_OAM;
#else
extern T_NvRamData *NvRamDataAddr;
#endif

extern "C" UINT32 g_SAG_VLAN_USAGE;
extern "C" UINT32 g_SAG_VLAN_BTSIP;
extern "C" UINT32 g_SAG_VLAN_ID;
extern "C" UINT32 g_SAG_VLAN_GATEWAY;
extern "C" UINT32 g_SAG_VLAN_SUBNETMASK;
extern "C" STATUS StartVlanEnd(/*NET_POOL_ID pNetPool*/);
extern "C" int getSTDNum(int *pValue);
#ifdef WBBU_CODE
extern "C" void WrruRFC(unsigned char antennamask,unsigned char flag,unsigned char flag1);
extern "C"  void Cfg_Annntena_mask(unsigned char anntenamask,unsigned char flag);
extern "C" void SendHardWarePara();
extern "C" void Send_Temp_2Aux();
extern "C" void sendAnntenaMsk(unsigned short  antennamask,unsigned char flag,unsigned char flag1);
extern "C" void  bspGetBPBSerial( char *ptr);
extern "C"  unsigned char  bspGetDeviceID(unsigned char *ptr);
extern "C"  void SetTsMode(unsigned char flag,unsigned char rru_no,unsigned short  offset);
extern "C" void Set_Fpga_Clk(unsigned char type);
extern "C" void SetOffset(unsigned short offset);
#include "l3Wrru.h"

#endif
extern bool AlarmReport(UINT8   Flag, UINT16  EntityType,
                 UINT16  EntityIndex, UINT16  AlarmCode,      
                 UINT8   Severity, const char format[],...);
extern "C" void updateLocalSagCfg(bool blUpdateVSrvInfoFromCfgFile);
extern "C" void updateDcsCfg();
//#ifndef WBBU_CODE
extern "C" bool writeNvramToBakfile(char *filename, char *srcData, UINT32 len,  int fileNo);
//#endif
extern bool bGpsStatus;  
CTaskCfg* CTaskCfg:: m_Instance = NULL;
SEM_ID CTaskCfg::s_semWriteNvram    = NULL;

static SINT8 s_aucEncryptedPwd[81] = {0};
UINT32  RelayWanifCpeEid[20] ;
UINT32 Enable_Wcpe_Flag_Judge = 0xeabdeabd;
//#ifndef WBBU_CODE
T_NVRAMBAKINFO nvramBakFileInfo;//标示nvram备份数据信息
//#endif
////UINT16 gRFMaskCfg;
void l3oamprintl2airlinknvromdata();
void init_Relay_WcpeEid();
#ifndef WBBU_CODE
extern "C" void DeleteVLANEnd(unsigned int ipaddr);
#else
extern "C"    void DeleteVLANEnd(/*unsigned int ipaddr*/);
#endif
//void l3oamprintl2arilinkcfgdata(T_AirLinkCfgEle *pdata);
#ifdef WBBU_CODE

 unsigned char   Calibration_Antenna = 0;
 unsigned char antennaMaskAlmInfo = 0;//bit为0表示无告警,为1表示有告警,bit0为低位
extern "C"  unsigned int  bspGetRRUChannelNum();
extern UINT32 g_close_RF_dueto_Slave;
#endif
UINT16 bspGetNetworkID()
{
    return NvRamDataAddr->BtsGDataCfgEle.NetworkID;
}
#if 0
void bspSetNetworkID(UINT16 id)
{
    NvRamDataAddr->BtsGDataCfgEle.NetworkID = id;
}
#endif

UINT8  Cal_Data_Cfg_Return_Ok = 0;
UINT8  GPS_Cal_OK_Send_Cmd = 0;
int CTaskCfg::l3oambspNvRamWrite(char * TargAddr, char *ScrBuff, int size)
{
   if((ScrBuff==NULL)||(size==0)||(TargAddr==NULL))
   	{
   	       return 0;
   	}   
  // #ifndef WBBU_CODE
   bool isChg = false;  
   static int caliFlag = 0;
   //判断是否是需要备份的地址，并且与nvram不一致
    if((TargAddr>(char*)NVRAM_BASE_ADDR_OAM+sizeof(T_BTSCommonDataEle))\
		&&(TargAddr!=(char*)&(NvRamDataAddr->GpsDataCfgEle))\
		&&(memcmp(TargAddr, ScrBuff, size)!=0))//如果内存进行了修改
    {
        isChg = true;
    }    
  //  #endif
    semTake(s_semWriteNvram, WAIT_FOREVER);
    bspNvRamWrite(TargAddr, ScrBuff, size);
    semGive(s_semWriteNvram);
  //  #ifndef WBBU_CODE
    //记录校准数据是否发生改变
    if((isChg==true)&&((TargAddr>=(char*)&(NvRamDataAddr->CaliDataEle[0])) \
        &&(TargAddr<=(char*)&(NvRamDataAddr->CaliDataEle[CALIBRAT_DATA_ELE_NUM]))))
    {
        caliFlag = 1;
    }
    //如果有变化才写入cf卡    ,校准数据最后写
    if((isChg==true)&&!((TargAddr>=(char*)&(NvRamDataAddr->CaliDataEle[0])) \
        &&(TargAddr<=(char*)&(NvRamDataAddr->CaliDataEle[CALIBRAT_DATA_ELE_NUM]))))
    {        
        memcpy(&nvramBakFileInfo.bakFile[4], (UINT8*)NVRAM_BASE_ADDR_OAM, sizeof(T_NvRamData));
        UINT32 nvramCrc = CalcNvramCheckSum(&nvramBakFileInfo.bakFile[4]);
	 memcpy(nvramBakFileInfo.bakFile, &nvramCrc, 4);
        writeNvramToBakfile((char*)SM_NVRAM_CFG_DATA_FILENAME1, (char*)nvramBakFileInfo.bakFile, sizeof(T_NvRamData)+4, 1);
        T_NVRAMCRCINFO nvramCrcInfo;
        nvramCrcInfo.crcFlag = 0xaaaabbbb;
        nvramCrcInfo.crcResult = nvramCrc;
        bspNvRamWrite((char *)NVRAM_BASE_ADDR_OAM_DATA_CRC, (char *)&nvramCrcInfo, sizeof(T_NVRAMCRCINFO));
        nvramBakFileInfo.cfgDataChg = true;
    } 
    //校准数据发生改变,写一次
    if((caliFlag==1)&&(TargAddr==(char*)&(NvRamDataAddr->CaliDataEle[CALIBRAT_DATA_ELE_NUM])))
    {        
        memcpy(&nvramBakFileInfo.bakFile[4], (UINT8*)NVRAM_BASE_ADDR_OAM, sizeof(T_NvRamData));
        UINT32 nvramCrc = CalcNvramCheckSum(&nvramBakFileInfo.bakFile[4]);
	 memcpy(nvramBakFileInfo.bakFile, &nvramCrc, 4);
        writeNvramToBakfile((char*)SM_NVRAM_CFG_DATA_FILENAME1, (char*)nvramBakFileInfo.bakFile, sizeof(T_NvRamData)+4, 1);
        T_NVRAMCRCINFO nvramCrcInfo;
        nvramCrcInfo.crcFlag = 0xaaaabbbb;
        nvramCrcInfo.crcResult = nvramCrc;
        bspNvRamWrite((char *)NVRAM_BASE_ADDR_OAM_DATA_CRC, (char *)&nvramCrcInfo, sizeof(T_NVRAMCRCINFO));
        nvramBakFileInfo.cfgDataChg = true;
        caliFlag = 0;
    }
    if(TargAddr==(char*)&(NvRamDataAddr->GpsDataCfgEle))//gps数据定时更新,将crc写入nvram中，不立即写入备份文件
    {
        UINT32 nvramCrc = CalcNvramCheckSum((UINT8*)NVRAM_BASE_ADDR_OAM);	 
        T_NVRAMCRCINFO nvramCrcInfo;
        nvramCrcInfo.crcFlag = 0xaaaabbbb;
        nvramCrcInfo.crcResult = nvramCrc;
        bspNvRamWrite((char *)NVRAM_BASE_ADDR_OAM_DATA_CRC, (char *)&nvramCrcInfo, sizeof(T_NVRAMCRCINFO));
	 nvramBakFileInfo.cfgDataChg = true;
    }
   // #endif
  return 1;
}




CTaskCfg :: CTaskCfg()
{
    strcpy(m_szName, "tCfg");
    m_uPriority   = M_TP_L3CM;
    m_uOptions    = 0;
#ifndef WBBU_CODE
    m_uStackSize  = SIZE_KBYTE * 50;
#else
 m_uStackSize  = SIZE_KBYTE * 60;
m_send_cal_alarm = 0;
#endif
    m_iMsgQMax    = 100; 
    m_iMsgQOption = 0;
    bGPSCfgDataErrorAlm = false;

    pCalibTimer              = NULL;
    pCalRstDataRcd           = NULL;
    pDataAirLinkCfg          = NULL;
    pDataAirLinkCfgRandom    = NULL;
    pDataServiceCfg          = NULL;
    pGPSCfgDataErrorAlmTimer = NULL;
    pHLRStimer = NULL;
}

T_BTSUserCfgEle gstNvUser;
bool CTaskCfg :: Initialize()
{
    m_SysStatus = OAM_CFG_IDLE;

    CBizTask :: Initialize();
   // #ifndef WBBU_CODE
    //NVRAM备份机制初始化
    NvramDataCheckandInitInfo();
  //  #endif
	
    pDataAirLinkCfg = new T_AirLinkCfgEle; 
    pDataAirLinkCfgRandom = new T_AirLinkCfgEle; 
    if(pDataAirLinkCfg == NULL)
    { return false;}
    memcpy(pDataAirLinkCfg, &(NvRamDataAddr->AirLinkCfgEle), sizeof(T_AirLinkCfgEle));
    if(pDataAirLinkCfgRandom == NULL)
    { return false;}
    memcpy(pDataAirLinkCfgRandom, &(NvRamDataAddr->AirLinkCfgEle), sizeof(T_AirLinkCfgEle));

    pCalRstDataRcd = new T_CaliNotifyDataRcd; 
    if(pCalRstDataRcd == NULL)
    { return false;}
    memset(pCalRstDataRcd, 0, sizeof(T_CaliNotifyDataRcd));
    pCalRstDataRcd->IsDataCorrect  = true;
    //pCalRstDataRcd->CalibrateOK    = true;

    pGPSCfgDataErrorAlmTimer = CM_Createtimer(M_OAM_CFG_GPS_DATA_ERR_ALM_TIMER, true, GPS_CFG_DATA_ERR_ALM_PERIOD);
    if(NULL == pGPSCfgDataErrorAlmTimer)
    {
        return false;
    }

    s_semWriteNvram = semBCreate(SEM_Q_FIFO, SEM_FULL);   // initial value of the semaphore

    //lijinan 20080904
    memcpy(&gstNvUser, &(NvRamDataAddr->BTSUserCfgEle), sizeof(T_BTSUserCfgEle));
      #ifdef WBBU_CODE
   Calibration_Antenna = NvRamDataAddr->Last_Calibration_Antenna;
    OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_ERR_MSG, "[tCfg] Last Calibration State = 0x%02x \n",  Calibration_Antenna);
    #endif
   // gstNvUser = &(NvRamDataAddr->BTSUserCfgEle);
   //----------------
   
    //恢复配置过的用户及密码信息
    T_BTSUserCfgEle *pNvUser = &(NvRamDataAddr->BTSUserCfgEle);
    SINT8 *pExistUser        = pNvUser->user;
    if (0 != strlen(pExistUser))
        {
        //采用默认的加密算法
        if (OK == loginDefaultEncrypt(pNvUser->password, s_aucEncryptedPwd))
            {
            //加入到用户表
            loginUserAdd(pExistUser, s_aucEncryptedPwd);
            }
        }


//	m_bReset = false;  //liujianfeng
#ifdef NUCLEAR_CODE
	m_pucLimitedNeiBts = NULL;
	m_usLimitedNeiBtsLen = 0;
	m_ptmLimitedBC = NULL;
	m_ucLimitFlag = NvRamDataAddr->BtsGDataCfgEle.bLimited;
	if( 0x5a5a != NvRamDataAddr->stUtHandoverPara2.write_flag )
	{
		UINT8 ucw[6];
		ucw[0] = 12;
		ucw[1] = 3;
		ucw[2] = 3;
		ucw[3] = 0;
		ucw[4] = 0x5a;
		ucw[5] = 0x5a;
	    l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->stUtHandoverPara2), 
                           (SINT8*)&ucw, 
                           sizeof(T_UtHandoverPara2)
                           );		
	}
#endif

	if( SAG_BACKUP_FLAG != NvRamDataAddr->ucSagFlag )
	{
		char uc=SAG_BACKUP_FLAG;
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->ucSagFlag, (char*)&uc, 1 );
		
		T_SagBkp tmpSagBkp;
		memset( (UINT8*)&tmpSagBkp, 0, sizeof(T_SagBkp) );
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->SagBkp.ulSagID, (char*)&tmpSagBkp, sizeof(T_SagBkp) );

		T_JitterBuf tmpJtrBuf;
		memset( (UINT8*)&tmpJtrBuf, 0, sizeof(T_JitterBuf) );
		tmpJtrBuf.usJtrBufLength      =0x20;
		tmpJtrBuf.usJtrBufPackMax = 4;//20091227 fengbing, default value should be 4
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->JitterBuf.usJtrBufEnable, (char*)&tmpJtrBuf, sizeof(T_JitterBuf) );

		UINT8 ucTmpSagTosVoice      =0x00;//20091227 fengbing, default value should be 0
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->SagTos.ucSagTosVoice, (char*)&ucTmpSagTosVoice, sizeof(T_SagTos) );

		T_SavePwrCfg tmpSavePwrCfg;
		//memset( (UINT8*)&tmpSavePwrCfg, 0, sizeof(T_SavePwrCfg) );
		tmpSavePwrCfg.SavePwrFlag = 0;
		tmpSavePwrCfg.TS1Channel  = 50;
		tmpSavePwrCfg.TS1User     = 6 ;
		tmpSavePwrCfg.TS2Channel  = 110;
		tmpSavePwrCfg.TS2User     = 12;
		tmpSavePwrCfg.Fan1        = 40;
		tmpSavePwrCfg.Fan2        = 50;
		tmpSavePwrCfg.Fan3        = 55;
		tmpSavePwrCfg.Fan4        = 60;
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->SavePwr.SavePwrFlag, (char*)&tmpSavePwrCfg, sizeof(T_SavePwrCfg) );

		T_Qam64Cfg tmpQam64Cfg;
		tmpQam64Cfg.Qam64Ppc0 = 1;
		tmpQam64Cfg.Qam4ForQam64 = 0;
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->Qam64Cfg.Qam64Ppc0, (char*)&tmpQam64Cfg, sizeof(T_Qam64Cfg) );
	}
	#ifndef WBBU_CODE
	m_bSavePwrSupport = CM_SavePwrSupport();
	m_bSavePwrInitL2Req = false;
	if( m_bSavePwrSupport )
	{
		if( L2_L3_SAVE_POWER_ENABLE == NvRamDataAddr->SavePwr.SavePwrFlag )
			m_bSavePowerModel = true;
		else
			m_bSavePowerModel = false;
		m_ucFanCnt = 0;
		m_pFanTimer = CM_Createtimer( M_OAM_CFG_SAVE_POWER_FAN_REQ, true, SAVE_POWER_FAN_PERIOD );
		if( m_bSavePowerModel )
			m_pFanTimer->Start();
	}
	else
		m_bSavePowerModel = false;
	#endif
       #ifdef M_TGT_WANIF
	init_Relay_WcpeEid();
	#endif

	//#ifdef LJF_WCPE_BOOT
	m_bWcpeBootModel = false;
	m_bBootFromNV = false;
	m_bNVBootFail = false;
	m_bWcpeBootModelOK = false;
//#ifdef LJF_WAKEUPCONFIG
	if( M_WAKEUP_CONFIG_FLAG != NvRamDataAddr->stWakeupConf.usFlag )
	{
		T_WAKEUP_CONFIG stWkupConf;
		stWkupConf.usFlag = M_WAKEUP_CONFIG_FLAG;
		stWkupConf.usSwitch = 0;
		stWkupConf.ulPeriod = 10000000;
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->stWakeupConf.usFlag, (char*)&stWkupConf, sizeof(T_WAKEUP_CONFIG) );
	}
//	#ifdef LJF_RPT_ALTER_2_NVRAMLIST
	if( M_RPT_LIST_FLAG != NvRamDataAddr->ulRptListFlag )
	{
		UINT32 ulpid[11];
		ulpid[0] = M_RPT_LIST_FLAG;
		memset( (UINT8*)&ulpid[1], 0, sizeof(UINT32)*M_RPT_LIST_MAX );
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->ulRptListFlag, (char*)ulpid, sizeof(UINT32)*(1+M_RPT_LIST_MAX) );
	}
    if(NvRamDataAddr->sTimePara.valid == 0x20110401)
    {
	 pHLRStimer = CM_Createtimer( M_OAM_CFG_HLR_TIME_REQ, true, NvRamDataAddr->sTimePara.STime_period*1000 );
	 if( pHLRStimer )
		pHLRStimer->Start();
    }
	#ifdef RCPE_SWITCH
	if( M_TRUNK_MRCPE_FLAG != NvRamDataAddr->stTrunkMRCpe.usflag )
	{
		memset( (UINT8*)&m_stTrnukMRCpe, 0, sizeof(TrunkMRCpeRcd) );
		m_stTrnukMRCpe.usflag = M_TRUNK_MRCPE_FLAG;
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->stTrunkMRCpe, (char*)&m_stTrnukMRCpe, sizeof(TrunkMRCpeRcd) );
	}
	else
		memcpy( (UINT8*)&m_stTrnukMRCpe, (char*)&NvRamDataAddr->stTrunkMRCpe, sizeof(TrunkMRCpeRcd) );
	#endif

    #ifdef PAYLOAD_BALANCE_2ND
    if( 0x20110331 != NvRamDataAddr->pay_load_crc )
    {
        m_stPldBlnCfg.usFlag = 1;
        m_stPldBlnCfg.usLi = 50;
        m_stPldBlnCfg.usPeriod = 20;
        m_stPldBlnCfg.usLd = 30;
        m_stPldBlnCfg.usCount = 6;
        m_stPldBlnCfg.usSignal = 2;
        m_stPldBlnCfg.usLdPeriod = 600;
        m_stPldBlnCfg.usParam = 128;
        m_stPldBlnCfg.usBandSwitch = 0;

        UINT32 pay_load_crc;
        pay_load_crc = 0x20110331;
        l3oambspNvRamWrite( (char*)&NvRamDataAddr->PayloadCfg.usFlag, (char*)&m_stPldBlnCfg, sizeof(T_PayloadBalanceCfg) );
        l3oambspNvRamWrite( (char*)&NvRamDataAddr->pay_load_crc, (char*)&pay_load_crc, sizeof(UINT32) );

        UINT16 us[2];
        us[0] = BANDSWITCHFLAG;
        us[1] = m_stPldBlnCfg.usBandSwitch;            
        l3oambspNvRamWrite( (char*)&NvRamDataAddr->usBandSwitchFlag, (char*)&us, sizeof(16) * 2 );
    }
    #endif
    return true;
}

CTaskCfg* CTaskCfg :: GetInstance()
{
    if ( NULL == m_Instance )
    {
        m_Instance = new CTaskCfg;
    }

    return m_Instance;
}

CTimer* CTaskCfg :: CM_Createtimer(UINT16 MsgId, bool IsPeriod, UINT32 TimerPeriod)
{
    CL3OamCommonReq TimerMsg;
    TimerMsg.CreateMessage(*this);
    TimerMsg.SetDstTid(M_TID_CM);
    TimerMsg.SetSrcTid(M_TID_CM);
    TimerMsg.SetMessageId(MsgId);
    return new CTimer(IsPeriod, TimerPeriod, TimerMsg);
}
#ifdef WBBU_CODE
unsigned char cal_flag = 1;
extern "C"
void setcalflag(unsigned char flag)
{
   cal_flag = flag;
}
unsigned int aaaa = 0;
unsigned int bbbb = 0;
extern "C"  void printaabb(unsigned char flag)
{
	if(flag==0)
	{
	printf("aaaa:%d,bbbb:%d\n",aaaa,bbbb);
	}
	else
	{
		aaaa = 0;
		bbbb = 0;
	}
}
#endif
typedef struct _tmpWriteNvRamInfo
{
	char* dstAddr;
	char* srcAddr;
	int len;
}tmpWriteNvRamInfoT;
int CTaskCfg::CM_WrtieNvRam4OtherTask(CMessage& msg)
{
	tmpWriteNvRamInfoT* pData = (tmpWriteNvRamInfoT*)msg.GetDataPtr();
	return l3oambspNvRamWrite(pData->dstAddr, pData->srcAddr, pData->len);
}

//#ifdef LJF_WCPE_BOOT
#ifndef _INC_L3OAMSYSTEM
#include "L3OamSystem.h"
#endif
extern UINT8   g_Close_RF_flag;
bool CTaskCfg:: ProcessMessage(CMessage &rMsg)
{
    UINT16 MsgId;
	UINT16 usComRsp[2];
    MsgId = rMsg.GetMessageId();
    CTaskPM *tpmt = CTaskPM::GetInstance();
#ifdef WBBU_CODE
    unsigned char *p = (unsigned char*)rMsg.GetDataPtr();

 //unsigned short id =p[0]*0x100+p[1];
  OAM_LOGSTR3(LOG_DEBUG3, L3CM_ERROR_REV_MSG, "[tCfg] receive msg = 0x%04x,%x,%x", MsgId,p[0],p[1]);

  if(rMsg.GetSrcTid()>M_TID_MAX)
  	{
  	    return false;
  	}
//  static int caldata_times = 0;
#endif
    if(OAM_CFG_IDLE == m_SysStatus)   
    {
        if(M_OAM_DATA_CFG_INIT_NOTIFY == MsgId)
        {
            CCfgModuleInitNotify  InitNotify(rMsg);

//#ifdef LJF_WCPE_BOOT
            OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCFG]NvRamDataAddr->Wcpe_Switch[%04x]", NvRamDataAddr->Wcpe_Switch );
			if( 0x5a5a == NvRamDataAddr->Wcpe_Switch )
			{
				m_bWcpeBootModel = CTaskSystem::GetInstance()->m_bWcpeBootModel;
				if( m_bWcpeBootModel && (!m_bBootFromNV) )
				{
					InitNotify.SetType( TASK_INIT_FROM_NVRAM );
					m_bBootFromNV = true;
                    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]Boot from NvRam! please wait...m_bBootFromNV = true");
				}
			}
			
            if (TASK_INIT_FROM_NVRAM == InitNotify.GetType())
            {
                //启动周期校准定时器
                CM_CreateCalibrationTimer();
                InitFirstDataFromNvram();
                SetSysStatus(OAM_CFG_INIT_FROM_NVRAM );
                OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]Boot from NvRam! please wait...");
            }
            else
            {
                OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]Boot from EMS! please wait...");
                SetSysStatus(OAM_CFG_INIT_FROM_EMS);
				
				if( 0==NvRamDataAddr->ucVacPrefScg || 1==NvRamDataAddr->ucVacPrefScg )
					CM_VacPrefScg();
		#ifndef WBBU_CODE		
				CM_SavePwrCfg();
		#endif
				CM_Qam64();
				CM_LoadBlnInfo();
			/*	if( m_bPldBlnCfgOK )
                {
            		CM_SendComMsg( M_TID_L2OAM, MSGID_L3_L2_PAYLOAD_BALANCE_CFG, (UINT8*)&m_stPldBlnCfg, sizeof(m_stPldBlnCfg) );
                    OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]m_bPldBlnCfgOK=true");
                }
			   else
                    OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]m_bPldBlnCfgOK=false");*/
                tpmt->InitRTMonitor();
                tpmt->PM_StartRTMonitor();
            }
        }
        else
        {
            OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_REV_ERR_MSG, "[tCfg] receive error msg = 0x%04x from Task:%d @idle state", MsgId, rMsg.GetSrcTid());
        }

        return true;
    }

    if(OAM_CFG_INIT_FROM_NVRAM == m_SysStatus)
    {
        InitOtherDataFromNvram(rMsg);
        return true;
    }
 
    switch(MsgId)
    {
#ifdef WBBU_CODE
        case  0xaaaa:
        {
        	aaaa++;
            break;
        }
        case  0xbbbb:
        {
        	bbbb++;
        	break;
        }
#endif
    //#ifdef LJF_WCPE_BOOT
		case M_OAM_DATA_CFG_INIT_NOTIFY:
       		OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage::M_OAM_DATA_CFG_INIT_NOTIFY GetSysStatus([%02d]", GetSysStatus() );
#if 0
			if( (OAM_CFG_NORMAL==GetSysStatus()) && (m_bBootFromNV) )
			{
				CTaskSystem::GetInstance()->SYS_SendDataDLNotify();
				SetSysStatus( OAM_CFG_INIT_FROM_EMS );
				m_bBootFromNV = false;		
			}
			else if( (OAM_CFG_INIT_FROM_EMS==GetSysStatus()) && (m_bBootFromNV) )
			{
				CTaskSystem::GetInstance()->SYS_SendDataDLNotify();
				m_bBootFromNV = false;		
	       		OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] WCPE nvram boot fail, start ems boot" );				
			}
			else
           		OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] RECV M_OAM_DATA_CFG_INIT_NOTIFY before m_sysStatus==4");
			break;
#else
			if( m_bWcpeBootModel )//是否WCPEBOOT
			{
				if ( m_bWcpeBootModelOK )//已经注册成功，但是bts心跳失连，再次注册，直接回复
				{
					CM_SendBtsWorkingMsg();
					break;
				}
				if( m_bBootFromNV )//非OAM_CFG_INIT_FROM_NV状态进入
				{
					if( ! m_bNVBootFail )//NVram启动失败
						SetSysStatus( OAM_CFG_INIT_FROM_EMS );
					CTaskSystem::GetInstance()->SYS_SendDataDLNotify();
					m_bBootFromNV = false;		
		       		OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] (!m_bNVBootFail) && (m_bBootFromNV), start ems boot" );				
					break;
				}
			}
			break;
#endif
		case M_EMS_BTS_PAYLOAD_BALANCE_CFG_REQ:
#ifdef PAYLOAD_BALANCE_2ND
            if( (sizeof(T_PayloadBalanceCfg2nd)+2) == rMsg.GetDataLength() )
    			memcpy( (UINT8*)&m_stPldBlnCfg, (UINT8*)rMsg.GetDataPtr()+2, sizeof(T_PayloadBalanceCfg2nd) );
            else
            {
    			memcpy( (UINT8*)&m_stPldBlnCfg, (UINT8*)rMsg.GetDataPtr()+2, sizeof(T_PayloadBalanceCfg) );
                m_stPldBlnCfg.usBandSwitch = 0;
            }
			UINT32 pay_load_crc;
            pay_load_crc = 0x20110331;
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->PayloadCfg.usFlag, (char*)&m_stPldBlnCfg, sizeof(T_PayloadBalanceCfg) );
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->pay_load_crc, (char*)&pay_load_crc, sizeof(UINT32) );
            usComRsp[0] = BANDSWITCHFLAG;
            usComRsp[1] = m_stPldBlnCfg.usBandSwitch;            
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->usBandSwitchFlag, (char*)&usComRsp, sizeof(16) * 2 );
			m_bPldBlnCfgOK = true;
			CM_SendComMsg( M_TID_L2OAM, MSGID_L3_L2_PAYLOAD_BALANCE_CFG, (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength()/*l2不需要usBandSwitch*/ );
			OAM_LOGSTR3(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] M_EMS_BTS_PAYLOAD_BALANCE_CFG_REQ nvram len[%d],%d,0x%x", sizeof(T_NvRamData),NvRamDataAddr->PayloadCfg.usFlag,NvRamDataAddr->pay_load_crc);
#else
			memcpy( (UINT8*)&m_stPldBlnCfg, (UINT8*)rMsg.GetDataPtr()+2, sizeof(T_PayloadBalanceCfg) );
			UINT32 pay_load_crc;
            pay_load_crc = 0x20110331;
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->PayloadCfg.usFlag, (char*)&m_stPldBlnCfg, sizeof(T_PayloadBalanceCfg) );
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->pay_load_crc, (char*)&pay_load_crc, sizeof(UINT32) );
			m_bPldBlnCfgOK = true;
			CM_SendComMsg( M_TID_L2OAM, MSGID_L3_L2_PAYLOAD_BALANCE_CFG, (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength() );
			OAM_LOGSTR3(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] M_EMS_BTS_PAYLOAD_BALANCE_CFG_REQ nvram len[%d],%d,0x%x", sizeof(T_NvRamData),NvRamDataAddr->PayloadCfg.usFlag,NvRamDataAddr->pay_load_crc);
#endif
			break;
		case MSGID_L2_L3_PAYLOAD_BALANCE_RSP:
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_PAYLOAD_BALANCE_CFG_RSP, (UINT8*)rMsg.GetDataPtr(), 4 );
			break;
		case M_EMS_BTS_PAYLOAD_BALANCE_GET_REQ:
#ifdef PAYLOAD_BALANCE_2ND
			UINT8 ucArrRsp[2+2+sizeof(T_PayloadBalanceCfg2nd)];
			memcpy( ucArrRsp, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( ucArrRsp+2, 0, 2);
			memcpy( ucArrRsp+4, (UINT8*)&m_stPldBlnCfg, sizeof(T_PayloadBalanceCfg2nd) );
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_PAYLOAD_BALANCE_GET_RSP, (UINT8*)ucArrRsp, 2+2+sizeof(T_PayloadBalanceCfg2nd) );
#else
			UINT8 ucArrRsp[2+2+sizeof(T_PayloadBalanceCfg)];
			memcpy( ucArrRsp, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( ucArrRsp+2, 0, 2);
			memcpy( ucArrRsp+4, (UINT8*)&m_stPldBlnCfg, sizeof(T_PayloadBalanceCfg) );
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_PAYLOAD_BALANCE_GET_RSP, (UINT8*)ucArrRsp, 2+2+sizeof(T_PayloadBalanceCfg) );
#endif
			break;

 //fengbing 20091015 begin   
 //other task can use this message to write nvram
    	case MSGID_OAM_WRITE_NVRAM:
			CM_WrtieNvRam4OtherTask(rMsg);
			break;
 //fengbing 20091015 end			
#ifdef NUCLEAR_CODE
		case M_EMS_BTS_NUCLEAR_CFG_REQ:
			//#ifdef LJF_CODE_TEST
			//if( M_TID_EMSAGENTRX == rMsg.GetSrcTid() || M_TID_EMSAGENTTX == rMsg.GetSrcTid() )
			//break;
			//#endif
			if( NULL != m_pucLimitedNeiBts )
			{
				delete [] m_pucLimitedNeiBts;
				m_pucLimitedNeiBts = NULL;
				m_usLimitedNeiBtsLen = 0;
			}
			m_usLimitedNeiBtsLen = rMsg.GetDataLength();
			m_pucLimitedNeiBts = new UINT8[m_usLimitedNeiBtsLen];
	        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] M_EMS_BTS_NUCLEAR_CFG_REQ m_usLimitedNeiBtsLen[%d]", m_usLimitedNeiBtsLen);
			if( NULL != m_pucLimitedNeiBts)
			{
				memset( m_pucLimitedNeiBts, 0, m_usLimitedNeiBtsLen );
				memcpy( m_pucLimitedNeiBts, (UINT8*)rMsg.GetDataPtr(), 2 );
				CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_NUCLEAR_CFG_RSP, m_pucLimitedNeiBts, 4 );
				memcpy( m_pucLimitedNeiBts+2, (UINT8*)rMsg.GetDataPtr()+2, m_usLimitedNeiBtsLen-2 );
	        	OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] M_EMS_BTS_NUCLEAR_CFG_REQ m_usLimitedNeiBtsLen[%d]", m_usLimitedNeiBtsLen);
			}
			else
	            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] M_EMS_BTS_NUCLEAR_CFG_REQ new fail!!!");
			break;
		case M_EMS_BTS_NUCLEAR_GET_REQ:
			*(UINT16*)m_pucLimitedNeiBts = *(UINT16*)rMsg.GetDataPtr();
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_NUCLEAR_GET_RSP, m_pucLimitedNeiBts, m_usLimitedNeiBtsLen );
			break;

		case NUCLEAR_PRJ_BC_TIMER_MSGID:
			CM_NuclearSendLimitData( true, 0xFFFFFFFE );
			break;
		case MSGID_LIMITAREA_L2L3_CFG_RSP:
			break;
#endif

 #ifndef WBBU_CODE
		case M_EMS_BTS_SAVE_PWR_CFG_REQ:
			if( ! m_bSavePwrSupport )
			{
           		OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_SAVE_PWR_CFG_REQ bts save power support is false");
				*((UINT16*)rMsg.GetDataPtr()+1) = L2_L3_SAVE_POWER_DISABLE;
				l3oambspNvRamWrite( (char*)&NvRamDataAddr->SavePwr.SavePwrFlag, ((char*)rMsg.GetDataPtr())+2, sizeof(T_SavePwrCfg));
				*((UINT16*)rMsg.GetDataPtr()+1) = SAVE_POWER_HW_NOT_SUPPORT;
				CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_SAVE_PWR_GET_RSP,(UINT8*)rMsg.GetDataPtr(), 4 );
				break;
			}
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->SavePwr.SavePwrFlag, ((char*)rMsg.GetDataPtr())+2, sizeof(T_SavePwrCfg));
			CM_SendComMsg( M_TID_L2MAIN, MSGID_L3_L2_SAVEPWR_CFG_REQ, (UINT8*)rMsg.GetDataPtr(), 2+sizeof(T_SavePwrCfg));
			if( L2_L3_SAVE_POWER_ENABLE == *((UINT16*)rMsg.GetDataPtr()+1) )
			{
				m_pFanTimer->Start();
				m_bSavePowerModel = true;
           		OAM_LOGSTR(RPT_LOG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_SAVE_PWR_CFG_REQ enter savepwr modle");
			}
			else
			{
				CM_SavePwrExitModel();
			}
			break;
		case M_EMS_BTS_SAVE_PWR_GET_REQ  : 
			UINT8 us6[4+sizeof(T_SavePwrCfg)];
			memcpy( us6, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( us6+2, 0, 2);
			memcpy( us6+4, (UINT8*)&NvRamDataAddr->SavePwr, sizeof(T_SavePwrCfg) );
            OAM_LOGSTR(RPT_LOG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_SAVE_PWR_GET_REQ");
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_SAVE_PWR_GET_RSP,us6, 4+sizeof(T_SavePwrCfg));
			break;
		case MSGID_L2_L3_SAVEPWR_CFG_RSP:
			OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:MSGID_L2_L3_SAVEPWR_CFG_RSP");
			if( m_bSavePwrInitL2Req )
			{
				m_bSavePwrInitL2Req = false;
				CM_CommonRspHandle(rMsg, MSGID_L2_L3_SAVEPWR_CFG_RSP, NULL, 0);
			}
			else
			{
				CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_SAVE_PWR_GET_RSP,(UINT8*)rMsg.GetDataPtr(), 4);
				if( SAVE_POWER_HW_NOT_SUPPORT == *((UINT16*)rMsg.GetDataPtr()+1) ||
						SAVE_POWER_SW_NOT_SUPPORT == *((UINT16*)rMsg.GetDataPtr()+1))
					CM_SavePwrExitModel();
			}
			break;
		case M_L2_L3_SAVEPOWER_FAN_RSP:
			if( L2_L3_SAVE_POWER_SUCCESS != *((UINT16*)rMsg.GetDataPtr()+1) )
            	OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_L2_L3_SAVEPOWER_FAN_RSP open fan[%02d] FAIL!", m_ucFanCnt );
			else
            	OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_L2_L3_SAVEPOWER_FAN_RSP open fan[%02d] SUCCESS!", m_ucFanCnt );
			break;
		case M_OAM_CFG_SAVE_POWER_FAN_REQ:
			if( m_bSavePowerModel )
				CM_SavePwrFanCtrl();
			break;
		case M_L2_L3_SAVEPOWER_CPU_REQ:
			CM_SavePwrCpuCtrl(rMsg);
			break;
	#endif
		case M_EMS_BTS_SAG_CFG_REQ      :
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->SagBkp.ulSagID, ((char*)rMsg.GetDataPtr())+2, sizeof(T_SagBkp) );
			UINT8 us1[4];
			memcpy( us1, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( us1+2, 0, 2);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_SAG_CFG_REQ");
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_SAG_CFG_RSP,us1, 4);
			CM_SendComMsg( M_TID_VOICE, MSGID_VOICE_SET_CFG, NULL, 0 );
			break;
		case M_EMS_BTS_JITTERBUF_CFG_REQ:
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->JitterBuf.usJtrBufEnable, ((char*)rMsg.GetDataPtr())+2, sizeof(T_JitterBuf));
			UINT8 us2[4];
			memcpy( us2, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( us2+2, 0, 2);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_JITTERBUF_CFG_REQ");
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_JITTERBUF_CFG_RSP,us2, 4);
			CM_SendComMsg( M_TID_VOICE, MSGID_VOICE_SET_CFG, NULL, 0 );
			break;
		case M_EMS_BTS_SAG_TOS_CFG_REQ  :
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->SagTos.ucSagTosVoice, ((char*)rMsg.GetDataPtr())+2, sizeof(T_SagTos) );
			UINT8 us3[4];
			memcpy( us3, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( us3+2, 0, 2);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_SAG_TOS_CFG_REQ");
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_SAG_TOS_CFG_RSP,us3, 4);
			CM_SendComMsg( M_TID_VOICE, MSGID_VOICE_SET_CFG, NULL, 0 );
			break;
		case M_EMS_BTS_SAG_GET_REQ      :
			UINT8 us4[4+sizeof(T_SagBkp)];
			memcpy( us4, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( us4+2, 0, 2);
			memcpy( us4+4, (UINT8*)&NvRamDataAddr->SagBkp.ulSagID, sizeof(T_SagBkp) );
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_SAG_GET_REQ");
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_SAG_GET_RSP,us4, 4+sizeof(T_SagBkp) );
			break;
		case M_EMS_BTS_JITTERBUF_GET_REQ:
			UINT8 us5[4+sizeof(T_JitterBuf)];
			memcpy( us5, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( us5+2, 0, 2);
			memcpy( us5+4, (UINT8*)&NvRamDataAddr->JitterBuf.usJtrBufEnable, sizeof(T_JitterBuf) );
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_JITTERBUF_GET_REQ");
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_JITTERBUF_GET_RSP,us5, 4+sizeof(T_JitterBuf));
			break;
		case M_EMS_BTS_SAG_TOS_GET_REQ  : 
			UINT8 us7[4+sizeof(T_SagTos)];
			memcpy( us7, (UINT8*)rMsg.GetDataPtr(), 2);
			memset( us7+2, 0, 2);
			memcpy( us7+4, (UINT8*)&NvRamDataAddr->SagTos.ucSagTosVoice, sizeof(T_SagTos) );
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_SAG_TOS_GET_REQ");
			CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_SAG_TOS_GET_RSP,us7, 4+sizeof(T_SagTos));
			break;
        case M_EMS_BTS_GEN_DATA_CFG_REQ:
        {
        	   if(rMsg.GetDataLength()==0)
        	   	{
        	   	    return false;
        	   	}
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS general data config...OK");
            if(OAM_CFG_INIT_FROM_EMS == m_SysStatus)
            {
                CM_SendNotifyMsg(M_TID_SYS, M_OAM_CFG_SYS_REV_FIRST_CFGMSG);
            }

            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->BtsGDataCfgEle);
            CCfgBtsGenDataReq ReqMsg(rMsg); 
		unsigned int temp_g_SAG_VLAN_USAGE = 0;
		  temp_g_SAG_VLAN_USAGE = g_SAG_VLAN_USAGE;
            CM_CommonReqNoRspHandle(ReqMsg, 
                                    M_BTS_EMS_GEN_DATA_CFG_RSP,
                                    DstAddr, sizeof(T_BtsGDataCfgEle));

         //对是否使用SAG VLAN  的配置进行全局变量更新
           if((g_SAG_VLAN_USAGE    != NvRamDataAddr->BtsGDataCfgEle.SAGVlanUsage)|| 
            (g_SAG_VLAN_ID         != NvRamDataAddr->BtsGDataCfgEle.SAGVlanID) ||
            (g_SAG_VLAN_BTSIP      != NvRamDataAddr->BtsGDataCfgEle.BtsIPAddr)  ||
            (g_SAG_VLAN_SUBNETMASK != NvRamDataAddr->BtsGDataCfgEle.SubnetMask) ||
            (g_SAG_VLAN_GATEWAY    != NvRamDataAddr->BtsGDataCfgEle.DefGateway))
                {
                g_SAG_VLAN_USAGE      = NvRamDataAddr->BtsGDataCfgEle.SAGVlanUsage;
                g_SAG_VLAN_ID         = NvRamDataAddr->BtsGDataCfgEle.SAGVlanID;          
                g_SAG_VLAN_BTSIP      = NvRamDataAddr->BtsGDataCfgEle.BtsIPAddr;
                g_SAG_VLAN_SUBNETMASK = NvRamDataAddr->BtsGDataCfgEle.SubnetMask;     
                g_SAG_VLAN_GATEWAY    = NvRamDataAddr->BtsGDataCfgEle.DefGateway;
		  if(g_SAG_VLAN_USAGE ==1)
		 {
                	StartVlanEnd();     
		  }
		  else
		  {
		  	 if(temp_g_SAG_VLAN_USAGE==1)
		  	 {
		  	        OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] DeleteVLANEnd");
#ifndef WBBU_CODE
		  	 	    DeleteVLANEnd(g_SAG_VLAN_BTSIP);
#else
                      DeleteVLANEnd(/*g_SAG_VLAN_BTSIP*/);
#endif
		  	 }
		  }
                }
                
            OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS SAG vlan: %s", (int)(g_SAG_VLAN_USAGE?"true":"false"));
            OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS SAG vlan ID: %d", g_SAG_VLAN_ID);
            OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS IP in SAG vlan: 0x%x", g_SAG_VLAN_BTSIP);
            OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS IP mask in SAG vlan: 0x%x", g_SAG_VLAN_SUBNETMASK);
            OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS vlan gateway: 0x%x", g_SAG_VLAN_GATEWAY);

            //voice有关的配置修改,通知voice
            CComMessage *pComMsg = new(this,0)CComMessage;
            if(NULL != pComMsg)
                {
                pComMsg->SetMessageId(MSGID_VOICE_SET_CFG);
                pComMsg->SetSrcTid(M_TID_CM);
                pComMsg->SetDstTid(M_TID_VOICE);
                CComEntity::PostEntityMessage(pComMsg);
                }

            //by xiaoweifang.EMS修改NetworkID后，没有通知2层
            if(OAM_CFG_INIT_FROM_EMS != m_SysStatus)
            {// should only send data to L2 if not booting from EMS,make sure the other 
            // L2 data is valid
                CM_L2AirLinkCfg();
            }
#ifdef NUCLEAR_CODE
			m_ucLimitFlag = ReqMsg.GetLimited();
//#ifdef LJF_CODE_TEST
//			m_ucLimitFlag = 1;
//#endif
            OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] m_ucLimitFlag[%d],ReqMsg.GetLimited()[%d]", m_ucLimitFlag, ReqMsg.GetLimited());
			if( 1 == m_ucLimitFlag )
			{
                if( m_ptmLimitedBC )
                {
					m_ptmLimitedBC->Start();
                }
                else
                {
    				m_ptmLimitedBC = InitTimer( TRUE, NUCLEAR_PRJ_BC_TIMER_MSGID, NUCLEAR_PRJ_BC_TIMER_LENGTH);
    				if( m_ptmLimitedBC)
    					m_ptmLimitedBC->Start();
    				else
    		            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] create timer m_ptmLimitedBC failed");
                }
			}
            else
            {
                if( m_ptmLimitedBC )
                    m_ptmLimitedBC->Stop();
            }
			UINT16 us[2];
			us[0]= 0;
			us[1]= m_ucLimitFlag;
			CM_SendComMsg( M_TID_L2MAIN, MSGID_LIMITAREA_L2L3_CFG_REQ, (UINT8*)us, 4 );
#endif
            break;
        }

        case M_EMS_BTS_BTS_RESET_REQ:
            CM_BtsResetReq(rMsg);
            break;

        case M_EMS_BTS_DATA_SERVICE_CFG_REQ:
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Data Service config...");
            CM_BtsDataServiceCfgReq(rMsg);
            break;

        case M_ARP_CFG_DATA_SERVICE_CFG_RSP:
        case M_DM_CFG_DATA_SERVICE_CFG_RSP:
        case M_EB_CFG_DATA_SERVICE_CFG_RSP:
        case M_SNOOP_CFG_DATA_SERVICE_CFG_RSP:
            CM_BtsDataServiceCfgRsp(rMsg);
            break;
        case M_EMS_BTS_QOS_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS Qos config...");
            CCfgTosReq CfgReq(rMsg); 
            CM_CommonReqHandle(CfgReq, M_TID_EB, M_EMS_BTS_QOS_CFG_REQ, M_BTS_EMS_QOS_CFG_RSP);
            break;
        }
        
        case M_BTS_EMS_QOS_CFG_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS Qos config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->ToSCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_QOS_CFG_RSP, 
                               DstAddr, sizeof(T_ToSCfgEle) * MAX_TOS_ELE_NUM);
            break;
        }
             
        case M_EMS_BTS_CFG_ACL_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS ACL config...");
            CCfgACLReq CfgReq(rMsg); 
            CM_CommonReqHandle(CfgReq, 
                               M_TID_DM, 
                               M_EMS_BTS_CFG_ACL_REQ,
                               M_BTS_EMS_CFG_ACL_RSP);
            break;
        }
        
        case M_BTS_EMS_CFG_ACL_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS ACL config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->ACLCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_CFG_ACL_RSP, 
                               DstAddr, sizeof(T_ACLCfgEle));
            break;
        }
        
        case M_EMS_BTS_PERF_LOGGING_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS Performance config...");
            CCfgPerfLogReq CfgReq(rMsg);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_PM, 
                               M_EMS_BTS_PERF_LOGGING_CFG_REQ,
                               M_BTS_EMS_PERF_LOGGING_CFG_RSP);
            break;
        }

        case M_BTS_EMS_PERF_LOGGING_CFG_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS Performance config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->PerfLogCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_PERF_LOGGING_CFG_RSP, 
                               DstAddr, sizeof(T_PerfLogCfgEle));
            break;
        }

        case M_EMS_BTS_GPS_DATA_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] GPS data config ....");
            T_GpsDataCfgEle tmp;
            UINT16 result = 0;//succ
            memcpy((char*)&tmp, (char*)((char*)rMsg.GetDataPtr()+2), sizeof(T_GpsDataCfgEle));
            if(tmp.SatelliteCnt<3)//不允许配置卫星数小于3
            {
                result = 1;//fail
                UINT8 us4[4];
                memcpy( us4, (UINT8*)rMsg.GetDataPtr(), 2);
                memcpy( us4+2, &result, 2);			
                CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_GPS_DATA_CFG_RSP,us4, 4);
            }
            else
            {
                SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->GpsDataCfgEle);
                CCfgGpsDataReq ReqMsg(rMsg); 
                CM_CommonReqNoRspHandle(ReqMsg, 
                                        M_BTS_EMS_GPS_DATA_CFG_RSP, 
                                        DstAddr, sizeof(T_GpsDataCfgEle));
    
                CM_PostCommonRsp(M_TID_GM, OAM_DEFAUIT_TRANSID, M_EMS_BTS_GPS_DATA_CFG_REQ, OAM_SUCCESS);    
            }
            break;
        }

#if 0
        case M_BTS_EMS_GPS_DATA_CFG_RSP:
        {
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_BTS_INIT_FAIL, "[tCFG]ERROR!!!Rx M_BTS_EMS_GPS_DATA_CFG_RSP");
#if 0
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->GpsDataCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_GPS_DATA_CFG_RSP, 
                               DstAddr, sizeof(T_GpsDataCfgEle));
#endif
            break;
        }
#endif


        case M_EMS_BTS_UT_SERVICE_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] UT Service descriptor config...");
            CCfgBtsNeibListReq CfgReq(rMsg);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_UT_DEFAULT_SER_DISCR_REQ,
                               M_BTS_EMS_UT_SERVICE_CFG_RSP);
            break;
        }

        case M_L2_L3_UT_DEFAULT_SER_DISCR_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] UT Service descriptor config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->UTSDCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_UT_SERVICE_CFG_RSP, 
                               DstAddr, sizeof(T_UTSDCfgEle));
            break;
        }
        
        case M_EMS_BTS_TEMP_MON_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS temprature monitor data config...");
            CCfgTempAlarmReq CfgReq(rMsg);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_TEMP_MONITOR_REQ,
                               M_BTS_EMS_TEMP_MON_CFG_RSP);
            break;
        } 
        
        case M_L2_L3_TEMP_MONITOR_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS temprature monitor data config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->TempAlarmCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_TEMP_MON_CFG_RSP, 
                               DstAddr, sizeof(T_TempAlarmCfgEle));
            break;
        }

        case M_EMS_BTS_GET_BOARDS_STATE_REQ:
        {
            CL3OamCommonReq CfgReq(rMsg);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_GET_BOARDS_STATE_REQ,
                               M_BTS_EMS_GET_BOARDS_STATE_RSP);
            break;
        }

        case M_L2_L3_GET_BOARDS_STATE_RSP:
        {
            CL3GetBoardsStatesRsp RspMsg(rMsg);
            RspMsg.SetMessageId(M_BTS_EMS_GET_BOARDS_STATE_RSP);
            RspMsg.SetDstTid(M_TID_EMSAGENTTX);
            RspMsg.SetSrcTid(M_TID_CM);  
 
            CTransaction * pTransaction = FindTransact(RspMsg.GetTransactionId());
            if(!pTransaction)
            { 
                OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post response fail!  can't find TransId! srcTID[%d] MsgId[0x%04x]", rMsg.GetSrcTid(), M_L2_L3_GET_BOARDS_STATE_RSP);
                return false;
            }
            else
            {
                RspMsg.SetTransactionId(pTransaction->GetRequestTransId());
                if(true != RspMsg.Post())
                {
                    OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", RspMsg.GetMessageId());
                }

                pTransaction->EndTransact();
                delete pTransaction;
            }

            break;
        }
        case M_EMS_BTS_HANDOVER_PARA_CFG_REQ:
              CM_HandoverParaCfg(rMsg);
		break;
		
	 case M_EMS_BTS_IF_PERMIT_USE_CFG_REQ:
              CM_IfPermitUseCfg(rMsg);
		break;
	case M_EMS_BTS_VOICE_PARA_IF_SAG_DOWN_CFG:
	{
              OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS voice para if sag is down cfg OK");
		char* DstAddr = (char*)&(NvRamDataAddr->localSagCfg);
		T_localSagCfg localSagCfg;
		memcpy((char*)&localSagCfg, ((char*)rMsg.GetDataPtr()+2), sizeof(T_localSagCfg)-4 );
		UINT32 uiTemp = M_LOCALSAG_CFG_VALID_FLAG;		 
		memcpy((UINT8*)&localSagCfg.validFlag, (UINT8*)&uiTemp, 4);
		l3oambspNvRamWrite(DstAddr, (char*)&localSagCfg, sizeof(T_localSagCfg));	       
		UINT16 tTranid = *(UINT16*)((UINT8*)rMsg.GetDataPtr());
		CM_PostCommonRsp(M_TID_EMSAGENTTX, tTranid, M_BTS_EMS_VOICE_PARA_IF_SAG_DOWN_RSP, OAM_SUCCESS);  
		//根据配置参数更新配置add by fengbing 20100126
		updateLocalSagCfg(false);
		break;
	}	
        case M_EMS_BTS_DCS_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS DCS config...");
            char* DstAddr = (char*)(&NvRamDataAddr->dcsCfgBuffer.dcsCfg);
            l3oambspNvRamWrite(DstAddr, ((char*)rMsg.GetDataPtr()+2), sizeof(T_DcsCfg));   
            UINT32 uiTemp = M_DCS_CFG_VALID_FLAG;
            l3oambspNvRamWrite((char*)NvRamDataAddr->dcsCfgBuffer.validFlag, (char*)&uiTemp, 4);
            UINT16 tTranid = *(UINT16*)((UINT8*)rMsg.GetDataPtr());
            CM_PostCommonRsp(M_TID_EMSAGENTTX, tTranid, M_BTS_EMS_DCS_CFG_RSP, OAM_SUCCESS);  
            updateDcsCfg();
            CM_SendComMsg( M_TID_DGRV_LINK, MSGID_VOICE_SET_CFG, NULL, 0 );
            break;
        }			
        case M_EMS_BTS_DCS_GET_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS DCS config response");
            CComMessage *pDcsCfgMsg = new(this, sizeof(T_DcsCfg)+4)CComMessage;
            if(NULL != pDcsCfgMsg)
            {
                pDcsCfgMsg->SetMessageId(M_BTS_EMS_DCS_GET_RSP);
                pDcsCfgMsg->SetSrcTid(M_TID_CM);
                pDcsCfgMsg->SetDstTid(M_TID_EMSAGENTTX);
                *(UINT16*)(pDcsCfgMsg->GetDataPtr()) = *(UINT16*)(rMsg.GetDataPtr());//tranid
                *(UINT16*)((UINT8*)pDcsCfgMsg->GetDataPtr()+2) = 0;//result			
                memcpy((UINT8*)pDcsCfgMsg->GetDataPtr()+4, (SINT8*)&(NvRamDataAddr->dcsCfgBuffer.dcsCfg), sizeof(T_DcsCfg));		       
                if ( false == CComEntity::PostEntityMessage(pDcsCfgMsg) )
                {
                    pDcsCfgMsg->Destroy();				
                }
            }
            break;
        }
        //L2                
        case M_EMS_BTS_CARRIER_DATA_CFG_REQ:
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Carrier data config...");
            CM_CarrierDataCfgReq(rMsg);
#ifdef WBBU_CODE
              l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->AirLinkCfgEle), (SINT8*)pDataAirLinkCfg, sizeof(T_AirLinkCfgEle));
#endif
            break;
        
        case M_L2_L3_AIRLINK_DATA_CFG_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Carrier data config response");
            CM_CarrierDataCfgRsp(rMsg);
            break;
        }
        
        case M_EMS_BTS_AIR_LINK_MISC_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Airlink Misc data config...");
            CCfgAirLinkMiscReq CfgReq(rMsg);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_AIR_LINK_MISC_CFG_REQ,
                               M_BTS_EMS_AIR_LINK_MISC_CFG_RSP);
            break;
        }

        case M_L2_L3_AIR_LINK_MISC_CFG_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Airlink Misc data config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->AirLinkMisCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_AIR_LINK_MISC_CFG_RSP, 
                               DstAddr, sizeof(T_AirLinkMisCfgEle));
            break;
        }

        case M_EMS_BTS_N1_PARAMETER_CFG_REQ:
            {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] N=1 parameter config...");            
            CL3OamCommonReq ReqMsg(rMsg);            
            CM_CommonReqHandle(ReqMsg, 
                               M_TID_L2MAIN, 
                               M_L3_L2_N1_PARAMETER_REQ,
                               M_BTS_EMS_N1_PARAMETER_CFG_RSP);
            break;
            }
        case M_L2_L3_N1_PARAMETER_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] N1_PARAMETER config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->N_parameter);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_N1_PARAMETER_CFG_RSP, 
                               DstAddr, sizeof(T_N_Parameter));
            break;
        }
        case M_EMS_BTS_RES_MANAGE_POLICY_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS RM policy config...");
            CCfgRMPolicyReq CfgReq(rMsg);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_RES_MANAGE_POLICY_REQ,
                               M_BTS_EMS_RES_MANAGE_POLICY_RSP);
            break;
        }
        
        case M_L2_L3_RES_MANAGE_POLICY_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS RM policy config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->RMPoliceEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_RES_MANAGE_POLICY_RSP, 
                               DstAddr, sizeof(T_RMPoliceEle));
            break;
        }

        case M_EMS_BTS_BILLING_DATA_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Billing data config...");
            CCfgBillingDataReq CfgReq(rMsg);
            CfgReq.setUserPassword(NvRamDataAddr->BTSUserCfgEle.user, NvRamDataAddr->BTSUserCfgEle.password);
			CfgReq.SetDataLength(CfgReq.GetDataLength() + sizeof(NvRamDataAddr->BTSUserCfgEle));
            CM_CommonReqHandle(CfgReq,
                               M_TID_L2MAIN, 
                               M_L3_L2_BILLING_DATA_CFG_REQ,
                               M_BTS_EMS_BILLING_DATA_CFG_RSP);
            break;
        }

        case M_L2_L3_BILLING_DATA_CFG_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Billing data config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->BillDataCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_BILLING_DATA_CFG_RSP, 
                               DstAddr, sizeof(T_BillDataCfgEle));
            break;
        }

	case M_EMS_BTS_CLUSTER_PARA_CFG:
	{
	     OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Cluser para config...");            
            CCfgClusterParaReq ReqMsg(rMsg);            
            CM_CommonReqHandle(ReqMsg, 
                               M_TID_L2MAIN, 
                               MSGID_CLUSTER_L2L3_PARA_CFG,
                               M_BTS_EMS_CLUSTER_PARA_RSP);
           break;
	}
		   
       case MSGID_CLUSTER_L2L3_PARA_CFG_RSP:
	{
	     OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS Cluser para config response");
            SINT8* DstAddr1 = (SINT8*)&(NvRamDataAddr->ClusterPara);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_CLUSTER_PARA_RSP, 
                               DstAddr1, sizeof(T_CLUSTER_PARA)-2);//去掉末尾写标志
	   	break;
   	}

	//ems ranging cfg
	case M_EMS_BTS_REMOTE_RANGE_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] remote range para config...");            
            CL3OamCommonReq ReqMsg(rMsg);            
            CM_CommonReqHandle(ReqMsg, 
                               M_TID_L2MAIN, 
                               MSGID_L3_L2_REMOTE_RANGE_CFG,
                               M_BTS_EMS_REMOTE_RANGE_CFG_RSP);
            break;
            }
	
	case MSGID_L2_L3_REMOTE_RANGE_RSP:
	{
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] remote range para config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->RangingPara);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_REMOTE_RANGE_CFG_RSP, 
                               DstAddr, sizeof(T_RangingPara));
	        
            UINT16 usResult = *(UINT16*)((UINT8*)rMsg.GetDataPtr()+2);
            if(OAM_SUCCESS == usResult)
                CM_sendMsgToRpc();
            break;
        }
	case MSGID_L2_L3_VACPREFSCG_RSP:
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:MSGID_L3_L2_VACPREFSCG_CFG");
            CM_CommonRspHandle(rMsg, 
                               MSGID_L2_L3_VACPREFSCG_RSP, 
                               NULL, 0);
			break;

		case MSGID_L2_L3_QAM64_RSP:
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:MSGID_L2_L3_QAM64_RSP");
            CM_CommonRspHandle(rMsg, 
                               MSGID_L2_L3_VACPREFSCG_RSP, 
                               NULL, 0);
			break;
	

        //L1
        case M_EMS_BTS_RF_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] RF data config...");
            CCfgRfReq CfgReq(rMsg);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_RF_CFG_REQ,
                               M_BTS_EMS_RF_CFG_RSP);
            neighborListRefresh();
            break;
        }

        case M_L2_L3_RF_CFG_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] RF data config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->RfCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_RF_CFG_RSP, 
                               DstAddr, sizeof(T_RfCfgEle));
            break;
        }
	 //telnet配置命令消息，这里来删除transaction
        case MSGID_L2_L3_GRP_LIMIT_CFG_RSP:
        {
	      CL3OamCommonRsp Rsp(rMsg);    
   
             CTransaction * pTransaction = FindTransact(Rsp.GetTransactionId());
             if(!pTransaction)
             { 
                 OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post response fail!  can't find TransId! srcTID[%d] MsgId[0x%04x]", rMsg.GetSrcTid(), M_L2_L3_GET_BOARDS_STATE_RSP);
                 return false;
             }
             else
             { 
                 pTransaction->EndTransact();
                 delete pTransaction;
		   pTransaction = NULL;
             }
             break;
        }
        case M_EMS_BTS_L1_GENERAL_SETTING_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] L1 general setting config...");
            CCfgL1GenDataReq CfgReq(rMsg);
            m_gL1Cfg.AntennaMask = CfgReq.GetAntennaMask();
            m_gL1Cfg.SyncSrc     = CfgReq.GetSyncSrc();
            m_gL1Cfg.GpsOffset   = CfgReq.GetGPSOffset();
#ifndef WBBU_CODE
            OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] SyncSrc[%d], offset[%d] Antanna Mask[%04x]", m_gL1Cfg.SyncSrc, m_gL1Cfg.GpsOffset, m_gL1Cfg.AntennaMask);

            if(BTS_SYNC_SRC_GPS == CfgReq.GetSyncSrc())
            {
                if(bGPSCfgDataErrorAlm == true)
                {
                    bGPSCfgDataErrorAlm = false;                    
                    pGPSCfgDataErrorAlmTimer->Stop();
                }

                if (false == bGpsStatus)
                {
                    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] GPS not ready, disable all RFs, and set SYNC source: BTS");
                    CfgReq.SetAntennaMask(0X0000);
                    CfgReq.SetSyncSrc(BTS_SYNC_SRC_BTS);
                    AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                                  ALM_ID_GPS_LOST, ALM_CLASS_CRITICAL, STR_GPS_LOST);
                }
                else
                {
                    AlarmReport(ALM_FLAG_CLEAR, ALM_ENT_GPS, ALM_ENT_INDEX0,
                          ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_CLEAR_ALARM);
                }
            }

            if(BTS_SYNC_SRC_BTS == CfgReq.GetSyncSrc())
            {
                AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                          ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_GPS_CFG_ERROR);    
                if(bGPSCfgDataErrorAlm == false)
                {
                    bGPSCfgDataErrorAlm = true;                    
                    pGPSCfgDataErrorAlmTimer->Start();
                }
                //给alm模块发送消息，清除gps长时间不同步造成射频关闭告警,使得oam状态显示正确
                if(m_gL1Cfg.SyncSrc == BTS_SYNC_SRC_BTS)
                    CM_SendComMsg( M_TID_FM, M_OAM_CFG_ALM_CLR_RF_ALM_REQ, NULL, 0 );
            }
            #ifdef NUCLEAR_CODE
            if(BTS_SYNC_SRC_485 == CfgReq.GetSyncSrc())//核电项目增加485配置,但告诉一层为gps
            {
                OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Sync Src is not GPS, is 485.");
                AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                          ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_GPS_CFG_ERROR_485);
                CfgReq.SetSyncSrc(BTS_SYNC_SRC_GPS);
                if(bGPSCfgDataErrorAlm == true)
                {
                    bGPSCfgDataErrorAlm = false;                    
                    pGPSCfgDataErrorAlmTimer->Stop();
                }
            }
            #endif
#else
             //UINT8 uctemp1, uctemp2;
             int sycRF = 0;
             #if 0
             //解析mask每一位,判断是否要告警
             for(int antcount=0; antcount<8; antcount++)
            {
                uctemp1 = (m_gL1Cfg.AntennaMask&(1<<antcount))>>antcount;//取出每一个bit
                uctemp2 = (antennaMaskAlmInfo&(1<<antcount))>>antcount;
                if((uctemp1 == 0)&&(uctemp2 == 0))//关闭射频,此前无告警
                {
                    antennaMaskAlmInfo = antennaMaskAlmInfo|(1<<antcount);//添加告警
                    AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       antcount,
                                       ALM_ID_RF_RF_DISABLED,
                                       ALM_CLASS_CRITICAL,
                                       STR_RF_DISABLED, antcount + 1);
                }
                else if((uctemp1 == 1)&&(uctemp2 == 1))
                {
                    antennaMaskAlmInfo = antennaMaskAlmInfo&(~(1<<antcount));//清除告警
                    AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       antcount,
                                       ALM_ID_RF_RF_DISABLED,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
                //其他情况不变
            }
            #endif
              if(BTS_SYNC_SRC_GPS == CfgReq.GetSyncSrc())
            {
                if(bGPSCfgDataErrorAlm == true)
                {
                    bGPSCfgDataErrorAlm = false;                    
                    pGPSCfgDataErrorAlmTimer->Stop();
                }
                if (false == bGpsStatus)
                {
                    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] GPS not ready, disable all RFs");                  
                    AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                                  ALM_ID_GPS_LOST, ALM_CLASS_CRITICAL, STR_GPS_LOST);
                    //m_gL1Cfg.AntennaMask = 0;
                    WrruRFC(0x0000,1,11 );//关闭射频
                    sycRF = 1;
                }
                
                AlarmReport(ALM_FLAG_CLEAR, ALM_ENT_GPS, ALM_ENT_INDEX0,
                          ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_CLEAR_ALARM);
                
             
            }

            if(BTS_SYNC_SRC_BTS == CfgReq.GetSyncSrc())
            {
                AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                          ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_GPS_CFG_ERROR);    
                if(bGPSCfgDataErrorAlm == false)
                {
                    bGPSCfgDataErrorAlm = true;                    
                    pGPSCfgDataErrorAlmTimer->Start();//如果不是GPS作为同步方式，则进行告警 
                }
                //给alm模块发送消息，清除gps长时间不同步造成射频关闭告警,使得oam状态显示正确
                if(m_gL1Cfg.SyncSrc == BTS_SYNC_SRC_BTS)
                    CM_SendComMsg( M_TID_FM, M_OAM_CFG_ALM_CLR_RF_ALM_REQ, NULL, 0 );
            }

            
            if(m_gL1Cfg.SyncSrc == 1)
            	{
            	   Set_Fpga_Clk(2);
            	}
            else if(m_gL1Cfg.SyncSrc==0)
            	{
            	    Set_Fpga_Clk(0);
            	}
            else
            	{
		       Set_Fpga_Clk(3);
            	}

           SetOffset(m_gL1Cfg.GpsOffset );            
            if(Calibration_Antenna!=0)//同步校准状态
            {
                OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] last calibration(%x) is fail, disable some RF", Calibration_Antenna);
                UINT16 usRFMask = 0xffff;
                usRFMask = (m_gL1Cfg.AntennaMask)&(~Calibration_Antenna);
                WrruRFC(usRFMask,1,11 );//关闭射频
                sycRF = 1;  
		if(m_send_cal_alarm ==1)
			{
                AlarmReport(ALM_FLAG_SET,
                   ALM_ENT_AUX, 
                   ALM_ENT_INDEX0, 
                   ALM_ID_AUX_AUX_Calibration_Err,
                   ALM_CLASS_MAJOR,
                   STR_AUX_CAL_ERR_0);
				m_send_cal_alarm = 1;
			}
            }
           if(sycRF == 0)// 如果没有同步rf，则执行
            {
                WrruRFC(m_gL1Cfg.AntennaMask ,0,12);
                OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] SyncSrc[%d], offset[%d] Antanna Mask[%04x]", m_gL1Cfg.SyncSrc, m_gL1Cfg.GpsOffset, m_gL1Cfg.AntennaMask);
            }
#endif
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_L1_GENERAL_SETTING_REQ,
                               M_BTS_EMS_L1_GENERAL_SETTING_RSP);
            break;
        }
        
        case M_L2_L3_L1_GENERAL_SETTING_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] L1 general setting config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->L1GenCfgEle);
            if(true==CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_L1_GENERAL_SETTING_RSP, 
                               DstAddr, sizeof(T_L1GenCfgEle)))
            	{
#ifdef WBBU_CODE
		                 //WrruRFC(m_gL1Cfg.AntennaMask ,0,12);//wangwenhua add 20091116
#endif
            	}
            break;
        }

        #ifdef M_TGT_WANIF
        case M_L2_L3_CFG_WANIF_CPE_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] WAN IF CPE EID setting config response");
            CL3OamCommonRsp RspMsg(rMsg);
            CTransaction * pTransaction = FindTransact(RspMsg.GetTransactionId());
            if(pTransaction)
            {
                pTransaction->EndTransact();
                delete pTransaction;
            }

            #if 0
            CM_CommonRspHandle(rMsg, 
                               M_L2_L3_CFG_WANIF_CPE_RSP, 
                               NULL, 0);
            #endif
            break;
        }
        #endif

        //5.4.3 Calibration General (EMS)
        case M_EMS_BTS_CALIBRAT_CFG_GENDATA_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS calibration general config...");
            CCfgCalGenDataReq  CfgReq(rMsg);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_CALIBRAT_CFG_GENDATA_REQ,
                               M_BTS_EMS_CALIBRAT_CFG_GENDATA_RSP);
            break;
        }
        
        case M_L2_L3_CALIBRAT_CFG_GENDATA_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS calibration general config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->CaliGenCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_CALIBRAT_CFG_GENDATA_RSP, 
                               DstAddr, sizeof(T_CaliGenCfgEle));
            break;
        }

        //5.4.4 Calibration CalData (EMS)
        case M_EMS_BTS_CALIBRAT_CFG_DATA_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS calibration data config...");
            CCfgCalDataReq  CfgReq(rMsg);
	   #ifdef WBBU_CODE
          Calibration_Antenna = 0;//如果从EMS主动加载校准数据，则认为是好的
	   #endif
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_CALIBRAT_CFG_DATA_REQ,
                               M_BTS_EMS_CALIBRAT_CFG_DATA_RSP);
            break;
        } 
        
        case M_L2_L3_CALIBRAT_CFG_DATA_RSP:
        {   //配置的数据不保存，对正确的结果才保存
            //CM_CommonRspNoSaveHandle(rMsg, M_BTS_EMS_CALIBRAT_CFG_DATA_RSP);
            static int i = 0;
            //static bool first = true;
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->CaliDataEle[i]);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_CALIBRAT_CFG_DATA_RSP, 
                               DstAddr, sizeof(T_CaliDataEle));
            i++;
            if(32 == i)
				{
				i = 0;
	            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS calibration data config response[32 times]");
				}

            break;
        }
        
        //5.4.6 Calibration Result Notification
        //5.4.6.1   Calibration Result General Notification（BTS）
        case M_L2_L3_CAL_GENDATA_NOTIFY:
            CM_CalibResultGenNotify(rMsg);
            break;

        //5.4.6.2   Calibration Result CalData Notification（BTS）
        case M_L2_L3_CAL_DATA_NOTIFY:
            CM_CalibResultNotify(rMsg);
            break;
        
        //5.4.5 Calibration Action Request（EMS）
        case M_EMS_BTS_INSTANT_CALIBRATION_REQ://一次性校准请求
        {
	   UINT16 usRFMask = NvRamDataAddr->L1GenCfgEle.AntennaMask;
#ifdef WBBU_CODE
	    if(g_Close_RF_flag!=1)
	    	{
	     		 sendAnntenaMsk(usRFMask,0,5);
	    	}
		else
		{
		   	sendAnntenaMsk(0,0,5);
		}
	        SendHardWarePara();
		Send_Temp_2Aux();//wangwenhua add 20100628 
	        taskDelay(2);
#endif
	      OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]Send Calibration Action Request to L2Main,antennmak:%x",usRFMask);
            CCfgCalActionReq CfgReq(rMsg);
            pCalRstDataRcd->ucCalTrigger = 0;
            CfgReq.SetCalTrigger(pCalRstDataRcd->ucCalTrigger);
            CM_CommonReqHandle(CfgReq, 
                               M_TID_L2MAIN, 
                               M_L3_L2_INSTANT_CALIBRATION_REQ,
                               M_BTS_EMS_INSTANT_CALIBRATION_RSP);
            break;
        }
        
        case M_L2_L3_INSTANT_CALIBRATION_RSP:
            CM_CommonRspHandle(rMsg, M_BTS_EMS_INSTANT_CALIBRATION_RSP,NULL,0,0);
            break;

        //5.4.7 Calibration Setting Request（EMS）            
        case M_EMS_BTS_CALIBRAT_CFG_REQ:
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS calibration periodic config OK");
            CM_CalibrationSetReq(rMsg);
            break;

        case M_OAM_CALIBRATION_TIMER:
#ifdef WBBU_CODE
            if((g_Close_RF_flag!=1)&&(g_close_RF_dueto_Slave==0))
            	{
	      sendAnntenaMsk(NvRamDataAddr->L1GenCfgEle.AntennaMask,0,6);
            	}
		else
		{
		 // sendAnntenaMsk(0,0,6);
		 break;//如果此时处于关RF状态的话，则不响应自动校准
		}
	        SendHardWarePara();
		Send_Temp_2Aux();//wangwenhua add 20100710
	        taskDelay(2);
#endif
	            CM_CalibrationTimer(rMsg);
            break;

        case M_OAMSYS_CFG_INIT_L2DATA_NOTIFY:
            CM_InitL2Data();
#ifdef WBBU_CODE
		OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage::MSGID_L2_L3_LOAD_CFG:%x\n ",MsgId );
#endif
            break;
#ifndef WBBU_CODE            
        case M_OAMSYS_CFG_INIT_AUXDATA_NOTIFY:  
            CM_InitAUXData();          
            break;
        case M_OAMSYS_CFG_INIT_FEPDATA_NOTIFY:  
            CM_InitFEPData();          
            break;
#else             
        case M_OAMSYS_CFG_INIT_AUXDATA_NOTIFY:  
        case M_OAMSYS_CFG_INIT_FEPDATA_NOTIFY:  
            CM_InitAUXData();          
        //    break;
      //  case M_OAMSYS_CFG_INIT_FEPDATA_NOTIFY:  
            CM_InitFEPData();          
            break;

#endif
        case M_OAM_SYS_CFG_INIT_FROM_EMS_OK:
            SetSysStatus(OAM_CFG_NORMAL);
//LJF_WCPE_BOOT
			if( m_bWcpeBootModel )
			{
				if( m_bNVBootFail )//Nv启动失败，发送一次开工指示，停止sys中inittimer
					CM_SendBtsWorkingMsg();
				m_bWcpeBootModelOK = true;
				CTaskSystem::GetInstance()->m_bWcpeBootModelOK = true;
			}
           // #ifndef WBBU_CODE
            //查看是否有nvram配置备份文件，如果没有则备份            
            if(nvramBakFileInfo.hasBakFile1==false)
            {
                memcpy(&nvramBakFileInfo.bakFile[4], (UINT8*)NVRAM_BASE_ADDR_OAM, sizeof(T_NvRamData));
                UINT32 nvramCrc = CalcNvramCheckSum(&nvramBakFileInfo.bakFile[4]);
                writeNvramToBakfile((char*)SM_NVRAM_CFG_DATA_FILENAME1, (char*)nvramBakFileInfo.bakFile, sizeof(T_NvRamData)+4, 1);
            }
            if(nvramBakFileInfo.hasBakFile2==false)
            {          
                memcpy(&nvramBakFileInfo.bakFile[4], (UINT8*)NVRAM_BASE_ADDR_OAM, sizeof(T_NvRamData));
                UINT32 nvramCrc = CalcNvramCheckSum(&nvramBakFileInfo.bakFile[4]);
                memcpy(nvramBakFileInfo.bakFile, &nvramCrc, 4);
                writeNvramToBakfile((char*)SM_NVRAM_CFG_DATA_FILENAME2, (char*)nvramBakFileInfo.bakFile, sizeof(T_NvRamData)+4, 2);
            }
            if(NULL == nvramBakFileInfo.pBakDataTimer) 
            {
                //启动2小时周期定时器，如果出现配置改变，就同步到备份文件2中
                nvramBakFileInfo.pBakDataTimer = CM_Createtimer(M_OAM_PERIOD_BAK_NVRAMDATA_TIMER, true,  2*60*60 * 1000); // 2hours
                
            }
	     if(NULL != nvramBakFileInfo.pBakDataTimer) 
            {
                nvramBakFileInfo.pBakDataTimer->Stop();
                nvramBakFileInfo.pBakDataTimer->Start();
            }
        // #endif
            break;

        case M_OAM_CFG_GPS_DATA_ERR_ALM_TIMER:
            if(BTS_SYNC_SRC_GPS != NvRamDataAddr->L1GenCfgEle.SyncSrc)
            {
                AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                          ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_GPS_CFG_ERROR);
            }
            break;        
        //查询类型的消息处理
        case  M_EMS_BTS_GEN_DATA_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->BtsGDataCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_BtsGDataCfgEle), M_EMS_BTS_GEN_DATA_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_DATA_SERVICE_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->DataServiceCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_DataServiceCfgEle), M_BTS_EMS_DATA_SERVICE_GET_RSP, Transid);
    
            break;          
        }

        case  M_EMS_BTS_UT_SERVICE_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->UTSDCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_UTSDCfgEle), M_BTS_EMS_UT_SERVICE_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_QOS_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->ToSCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_ToSCfgEle), M_BTS_EMS_QOS_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_PERF_LOGGING_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->PerfLogCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_PerfLogCfgEle), M_BTS_EMS_PERF_LOGGING_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_TEMP_MON_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->TempAlarmCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_TempAlarmCfgEle), M_BTS_EMS_TEMP_MON_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_GPS_DATA_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->GpsDataCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_GpsDataCfgEle), M_BTS_EMS_GPS_DATA_GET_RSP, Transid);
    
            break;         
        }
        case  M_EMS_BTS_GET_ACL_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->ACLCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_ACLCfgEle), M_BTS_EMS_GET_ACL_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_GET_SFID_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->SFIDCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_SFIDCfgEle), M_BTS_EMS_GET_SFID_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_CARRIER_DATA_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->AirLinkCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_AirLinkCfgEle), M_EMS_BTS_CARRIER_DATA_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_RES_MANAGE_POLICY_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->RMPoliceEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_RMPoliceEle), M_EMS_BTS_RES_MANAGE_POLICY_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_BILLING_DATA_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->BillDataCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_BillDataCfgEle), M_EMS_BTS_BILLING_DATA_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_AIR_LINK_MISC_GET_REQ: 
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->AirLinkMisCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_AirLinkMisCfgEle), M_BTS_EMS_AIR_LINK_MISC_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_N1_PARAMETER_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->N_parameter);
            CM_GetMsgHandle(srcAddr, sizeof(T_N_Parameter), M_BTS_EMS_N1_PARAMETER_GET_RSP, Transid);
            break;          
        }
        case  M_BTS_EMS_L1_GENERAL_SETTING_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->L1GenCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_L1GenCfgEle), M_BTS_EMS_L1_GENERAL_SETTING_GET_RSP, Transid);

            break;          
        }
        case  M_EMS_BTS_RF_CFG_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->RfCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_RfCfgEle), M_BTS_EMS_RF_CFG_GET_RSP, Transid);

            break;          
        }
        case  M_EMS_BTS_CALIBRAT_GET_GENDATA_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->CaliGenCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_CaliGenCfgEle), M_BTS_EMS_CALIBRAT_GET_GENDATA_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_CALIBRAT_GET_DATA_REQ:
        {
            UINT8* pdata   = (UINT8*)(rMsg.GetDataPtr());
            UINT16 Transid = *(UINT16 *)pdata; 
            UINT8  Index   = *(pdata + 2);
            if(Index < CALIBRATION_DATA_NUM)
            {
                SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->CaliDataEle[Index]);
                CM_GetMsgHandle(srcAddr, sizeof(T_CalCfgEle), M_BTS_EMS_CALIBRAT_GET_DATA_RSP, Transid);
            }
            else
            {
                CM_PostCommonRsp(M_TID_EMSAGENTTX, Transid, M_BTS_EMS_CALIBRAT_GET_DATA_RSP, OAM_FAILURE);    
            }
            
            break;          
        }
        case  M_EMS_BTS_CALIBRAT_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->CalCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_CalCfgEle), M_BTS_EMS_CALIBRAT_GET_RSP, Transid);
    
            break;          
        }
        case  M_EMS_BTS_RF_DATA_GET_REQ:
        {
            CCfgCalGetRFDataReq msgGetRFData(rMsg);
            CM_CommonReqHandle(msgGetRFData, 
                               M_TID_L2MAIN, 
                               M_L3_L2_GET_RF_DATA_REQ,
                               M_BTS_EMS_RF_DATA_GET_RSP);
            break;
        }
        case  M_L2_L3_GET_RF_DATA_RSP:
        {
            CL3OamCommonRsp RspMsg(rMsg);

            CTransaction * pTransaction = FindTransact(RspMsg.GetTransactionId());
            if(!pTransaction)
            { 
                OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Unexpected L2->L3 Get RF Data response message.");
            }
            else
            {
                CComMessage *pComMsg = new (this, rMsg.GetDataLength() + 2)CComMessage;
              if(pComMsg!=NULL)
              	{
                pComMsg->SetDstTid(M_TID_EMSAGENTTX);
                pComMsg->SetSrcTid(M_TID_CM);
                *(UINT16*)(pComMsg->GetDataPtr()) = pTransaction->GetRequestTransId();
                *((UINT16*)(pComMsg->GetDataPtr()) + 1) = ERR_SUCCESS;
                memcpy((UINT16*)(pComMsg->GetDataPtr()) + 2, ((UINT8*)(rMsg.GetDataPtr())) + 2, rMsg.GetDataLength()-2);
                pComMsg->SetMessageId(M_BTS_EMS_RF_DATA_GET_RSP);
                if(false == CComEntity::PostEntityMessage(pComMsg))
                {
                    pComMsg->Destroy();
                }
                }
                pTransaction->EndTransact();
                delete pTransaction;
            }

            break;
        }

        case M_EMS_BTS_TELNET_USER_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS telnet user config...response");
            CM_BtsTelnetUserCfgReq(rMsg);
            break;
        }
        case M_EMS_BTS_TELNET_USER_GET_REQ:
        {
            CM_BtsTelnetUserGetReq(rMsg);
            break;
        }

        case M_EMS_BTS_NEIGHBOR_LIST_HANDOFF_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS neighbor list config OK");
            CM_BtsNeighborListHandoffCfgReq(rMsg);
            break;
        }
        case M_EMS_BTS_NEIGHBOR_LIST_HANDOFF_GET_REQ:
        {
            CM_BtsNeighborListHandoffGetReq(rMsg);
            break;
        }

        case M_EMS_BTS_BTS_REPEATER_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS repeater config OK");
            CM_BtsRepeaterCfgReq(rMsg);
            break;
        }
        case M_EMS_BTS_BTS_REPEATER_GET_REQ:
        {
            CM_BtsRepeaterGetReq(rMsg);
            break;
        }

        case M_EMS_BTS_VLAN_GROUP_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] VLAN group config...");
            CL3OamCommonReq msgReq(rMsg);
            CM_CommonReqHandle(msgReq, M_TID_EB, M_EMS_BTS_VLAN_GROUP_CFG_REQ, M_BTS_EMS_VLAN_GROUP_CFG_RSP);
            break;
        }
        case M_BTS_EMS_VLAN_GROUP_CFG_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] VLAN group config response");
            SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->VlanGroupCfgEle);
            CM_CommonRspHandle(rMsg, 
                               M_BTS_EMS_VLAN_GROUP_CFG_RSP, 
                               DstAddr, sizeof(T_VlanGroupCfgEle));
            break;
        }
        case M_EMS_BTS_VLAN_GROUP_GET_REQ:
        {
            UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
            SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->VlanGroupCfgEle);
            CM_GetMsgHandle(srcAddr, sizeof(T_VlanGroupCfgEle), M_BTS_EMS_VLAN_GROUP_GET_RSP, Transid);
            break;          
        }
        case M_EMS_BTS_SERIAL_NUM_GET_REQ://查询序列号消息
 	 {
	 #ifndef WBBU_CODE
	 	CComMessage* pComMsg = new (this, 6) CComMessage;
	    if (pComMsg==NULL)
	    {
	        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed.");
	        return false;
	    }
	    pComMsg->SetDstTid(M_TID_DIAGM);
	    pComMsg->SetSrcTid(M_TID_CM); 
	    pComMsg->SetMessageId(M_EMS_BTS_SERIAL_NUM_GET_REQ);
	    memcpy((pComMsg->GetDataPtr()), ((UINT8*)(rMsg.GetDataPtr())), rMsg.GetDataLength());
	    if(!CComEntity::PostEntityMessage(pComMsg))
	    {
	        pComMsg->Destroy();
	        pComMsg = NULL;
	    }           
	#else
	         CComMessage *pMsg = NULL;
     	     pMsg = new (this, 4+20*10) CComMessage;
           if(pMsg!=NULL)
           	{
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_CM);
	     pMsg->SetMessageId(M_BTS_EMS_SERIAL_NUM_GET_RSP);
	      char *ptrtemp  =(char *) pMsg->GetDataPtr();
	      unsigned char *ptr1 = (unsigned char *) (rMsg.GetDataPtr());
		 for(int j =0 ; j < 204;j++)
		 {
		 	    ptrtemp[j ] = 0;
		 }
             ptrtemp[ 0] = ptr1[0];
		 ptrtemp[ 1] = ptr1[1];
		 ptrtemp[ 2] = 0;
		 ptrtemp[ 3] = 0;
         //   memcpy((pMsg->GetDataPtr() +),result_msg, sizeof(T_RSV_Msg));
           //  bspGetBPBSerial((ptrtemp+4));
		 if(bspGetDeviceID((unsigned char*)(ptrtemp+4))==1)
		 {
		 	ptrtemp[23] = 0;
		 }
		 else
		 {
		 	ptrtemp[23] = 0xff;
		 }
	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }
            }
		 #endif
	    break;	
 	 }
	case M_BTS_EMS_SERIAL_NUM_GET_RSP://diagM response
	{
		CComMessage *pComMsg1 = new(this,rMsg.GetDataLength())CComMessage;
		if(NULL != pComMsg1)
		{
			pComMsg1->SetMessageId(M_BTS_EMS_SERIAL_NUM_GET_RSP);
			pComMsg1->SetSrcTid(M_TID_CM);
			pComMsg1->SetDstTid(M_TID_EMSAGENTTX);			
			memcpy((UINT8*)pComMsg1->GetDataPtr(), (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength());		       
			if ( false == CComEntity::PostEntityMessage(pComMsg1) )
			{
				pComMsg1->Destroy();				
			}
		}
		break;
	}
	 case M_EMS_BTS_NEIGHBOR_LIST_COMMON_CFG_REQ:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS neighbor common config OK");
            CM_BtsNeighborListCommonCfgReq(rMsg);
            break;
        }       
        case M_EMS_BTS_HANDOVER_PARA_GET_REQ:
              CM_HandoverParaGet(rMsg);
        break;
       case M_EMS_BTS_CLUSTER_PARA_GET_REQ:
	{
	   	CComMessage *pComMsg3 = new(this, sizeof(T_CLUSTER_PARA)+2)CComMessage;
		if(NULL != pComMsg3)
		{
			pComMsg3->SetMessageId(M_BTS_EMS_CLUSTER_PARA_GET_RSP);
			pComMsg3->SetSrcTid(M_TID_CM);
			pComMsg3->SetDstTid(M_TID_EMSAGENTTX);		
			*(UINT16*)(pComMsg3->GetDataPtr()) = *(UINT16*)(rMsg.GetDataPtr());//tranid
			*(UINT16*)((UINT8*)pComMsg3->GetDataPtr()+2) = 0;//result			
			memcpy((UINT8*)pComMsg3->GetDataPtr()+4, (SINT8*)&(NvRamDataAddr->ClusterPara), sizeof(T_CLUSTER_PARA)-2);		       
			if ( false == CComEntity::PostEntityMessage(pComMsg3) )
			{
				pComMsg3->Destroy();				
			}
		}
       	
		break;
	}	
       case M_EMS_BTS_IF_PERMIT_USE_CFG_GET:
	   	CM_IfPermitUseGet(rMsg);
		break;
	case M_EMS_BTS_CFG_CDR_REQ:
		CM_CdrParaCfgPro(rMsg);
		break;
	case M_EMS_BTS_GET_CDR_REQ:
		CM_CdrParaGet(rMsg);
		break;

		//ems ranging cfg
       case M_BTS_EMS_REMOTE_RANGE_GET_REQ:
	   {
              UINT16 srcTransid = *(UINT16*)(rMsg.GetDataPtr());
              SINT8* srcNvAddr = (SINT8*)&(NvRamDataAddr->RangingPara);
              CM_GetMsgHandle(srcNvAddr, sizeof(T_RangingPara), M_BTS_EMS_REMOTE_RANGE_GET_RSP, srcTransid);
	   	break;
       }
#ifdef M_TGT_WANIF
        case M_EMS_BTS_WCPE_CFG_REQ ://      = 0x073f;
            CM_WCPE_Cfg(rMsg);
            break;
        case M_EMS_BTS_WCPE_Get_REQ:           // =0x0741;
            CM_WCPE_Get(rMsg);
            break;	 
       case M_EMS_BTS_RCPE_NEW_CFG_REQ ://      = 0x073f;
            CM_RCPE_Cfg(rMsg);
            break;
        case M_EMS_BTS_RCPE_NEW_GET_REQ:           // =0x0741;
            CM_RCPE_Get(rMsg);
            break;	
#endif
#ifdef RCPE_SWITCH
		case M_EMS_BTS_TRUNK_RCPE_CFG_REQ:
			if( NvRamDataAddr->Relay_WcpeEid_falg != 0xa5a5a5a5 )
			{
	            OAM_LOGSTR(LOG_WARN, L3CM_ERROR_REV_ERR_MSG, "[tCfg] rcpe flag is disable, please enable it first!!!" );
				memset( (char*)rMsg.GetDataPtr()+2, 0, 2 );
				CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_TRUNK_RCPE_CFG_RSP, (UINT8*)rMsg.GetDataPtr(), 4 );
			    break;
			}
			CM_RcpeSwitchCfg(rMsg);
			break;
		case M_EMS_BTS_TRUNK_RCPE_GET_REQ:
			memcpy( (char*)&m_stTrnukMRCpe.usflag, (char*)rMsg.GetDataPtr(), 2 );
    		CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_TRUNK_RCPE_GET_RSP, (UINT8*)&m_stTrnukMRCpe, sizeof(TrunkMRCpeRcd) );
			break;
#endif
       case M_EMS_BTS_VOICE_PARA_IF_SAG_DOWN_GET_REQ:
	{
              CComMessage *pComMsg3 = new(this, sizeof(T_localSagCfg)+4)CComMessage;
		if(NULL != pComMsg3)
		{
			pComMsg3->SetMessageId(M_BTS_EMS_VOICE_PARA_IF_SAG_DOWN_GET_RSP);
			pComMsg3->SetSrcTid(M_TID_CM);
			pComMsg3->SetDstTid(M_TID_EMSAGENTTX);		
			*(UINT16*)(pComMsg3->GetDataPtr()) = *(UINT16*)(rMsg.GetDataPtr());//tranid
			*(UINT16*)((UINT8*)pComMsg3->GetDataPtr()+2) = 0;//result			
			memcpy((UINT8*)pComMsg3->GetDataPtr()+4, (SINT8*)&(NvRamDataAddr->localSagCfg), sizeof(T_localSagCfg)-4);		       
			if ( false == CComEntity::PostEntityMessage(pComMsg3) )
			{
				pComMsg3->Destroy();				
			}
		}
	       break;
	}	
      // #ifndef WBBU_CODE
       case M_OAM_PERIOD_BAK_NVRAMDATA_TIMER:// 每隔2小时备份一次nvram数据
        {
	    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_ERR_MSG, "[tCfg] M_OAM_PERIOD_BAK_NVRAMDATA_TIMER, time out");	
            nvramBakFileInfo.pBakDataTimer->Stop();
            nvramBakFileInfo.pBakDataTimer->Start();
            if(nvramBakFileInfo.cfgDataChg == true)//改变了再写入
            {
                memcpy(&nvramBakFileInfo.bakFile[4], (UINT8*)NVRAM_BASE_ADDR_OAM, sizeof(T_NvRamData));
                UINT32 nvramCrc = CalcNvramCheckSum(&nvramBakFileInfo.bakFile[4]);
                memcpy(nvramBakFileInfo.bakFile, &nvramCrc, 4);
                writeNvramToBakfile((char*)SM_NVRAM_CFG_DATA_FILENAME2, (char*)nvramBakFileInfo.bakFile, sizeof(T_NvRamData)+4, 2);
                nvramBakFileInfo.cfgDataChg = false;
            }
            break;
        }
      // #endif
       case M_EMS_BTS_CFG_STIME_REQ:
		CM_STimeParaCfgPro(rMsg);
		break;
	case M_EMS_BTS_GET_STIME_REQ:
		CM_StimeParaGet(rMsg);
		break;
       case M_OAM_CFG_HLR_TIME_REQ:
	   	{
			 CComMessage *pComMsg = new(this, 0)CComMessage;
			if(NULL != pComMsg)
			{
				pComMsg->SetMessageId(M_OAM_CFG_HLR_TIME_REQ);
				pComMsg->SetSrcTid(M_TID_CM);
				pComMsg->SetDstTid(M_TID_GM);			       
				if ( false == CComEntity::PostEntityMessage(pComMsg) )
				{
					pComMsg->Destroy();				
				}
			}

	       }
	   	break;
	case M_EMS_RF_OPERA_CFG_REQ:
		CM_RFOPParaCfgPro(rMsg);
		break;
	case M_EMS_RF_OPERA_GET_REQ:
		CM_RFOPParaGet(rMsg);
		break;
    case M_EMS_BTS_VALID_FRECS_CFG_REQ:
    {
        VOLID_FREQS_PARA tempPara;
        memset(&tempPara, 0, sizeof(VOLID_FREQS_PARA));
        memcpy((char*)&tempPara.validFreqsInd, ((char*)rMsg.GetDataPtr())+2, 2);//get ind and num
        if(tempPara.validFreqsNum>20)
        {
           UINT8 us1[4];
           memcpy( us1, (UINT8*)rMsg.GetDataPtr(), 2);
           memset( us1+2, 0, 2);
           us1[3] = 1;           
           CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_VALID_FRECS_CFG_RSP,us1, 4); 
           OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, \
               "[tCfg] ProcessMessage:M_EMS_BTS_VALID_FRECS_CFG_REQ, para err, return fail!!");           
        }
        else
        {
            memcpy((char*)tempPara.validFreqs, ((char*)rMsg.GetDataPtr())+4, tempPara.validFreqsNum*2);
            tempPara.validFlag = 0x5a5a;
            l3oambspNvRamWrite((char*)&NvRamDataAddr->volidFreqPara.validFreqsInd, \
                     (char*)&tempPara, sizeof(VOLID_FREQS_PARA));            
            UINT8 us1[4];
            memcpy( us1, (UINT8*)rMsg.GetDataPtr(), 2);
            memset( us1+2, 0, 2);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_EMS_BTS_VALID_FRECS_CFG_REQ");
            CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_VALID_FRECS_CFG_RSP,us1, 4);
        }
        break;
    }
    case M_EMS_BTS_VALID_FRECS_GET_REQ:
    {
        UINT8 ucArrRsp[2+2+sizeof(VOLID_FREQS_PARA)-2];
		memcpy( ucArrRsp, (UINT8*)rMsg.GetDataPtr(), 2);
		memset( ucArrRsp+2, 0, 2);
		memcpy( ucArrRsp+4, (UINT8*)&NvRamDataAddr->volidFreqPara.validFreqsInd, \
             NvRamDataAddr->volidFreqPara.validFreqsNum*2+2);
		CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_VALID_FRECS_GET_RSP, (UINT8*)ucArrRsp,\
             2+2+NvRamDataAddr->volidFreqPara.validFreqsNum*2+2);
        break;
    }
    default:
       {
           OAM_LOGSTR2(LOG_DEBUG3, L3CM_ERROR_REV_ERR_MSG, "[tCfg] Receive error msg[0x%04x] from task:%d @ normal status", MsgId, rMsg.GetSrcTid());
           break;
       } 
    }
  
    return true;
}

extern UINT32 gcdrSendPeriod ;
extern UINT32 IP_CB3000;
extern UINT32 uiCdrSwitch;
extern UINT32 cdr_para_err_flag;
extern   RF_Operation_Para    g_rf_openation;
/*
#pragma pack (1)


typedef struct 
{
	UINT32 Initialized;
	UINT32 LocalBtsId;
	UINT8 cdrOnOff;
	UINT8 cdrPeriod;
	UINT32 IP_CB3000;
	UINT16 intval;
}stCdrNVRAMhdr;
#pragma pack ()
*/
void CTaskCfg::CM_CdrParaCfgPro(CMessage& rMsg)
{
     UINT8 onoff = *((UINT8*)rMsg.GetDataPtr()+2);
     UINT8 period = *((UINT8*)rMsg.GetDataPtr()+3);
     UINT32 Ip = *((UINT32 *)((UINT8*)rMsg.GetDataPtr()+4));
     UINT16 intval = *((UINT16 *)((UINT8*)rMsg.GetDataPtr()+8));
     UINT8 result = 0;
     if((onoff>1)||(period<1))
     {
		LOG2(LOG_CRITICAL, 0, "\n CM_CdrParaCfgPro para err,onoff:%x,period:%x\n",onoff,period);
		result = 1;
		
     }
	 	
    CComMessage* pComMsg = new (this, 4) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_HandoverParaCfg.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_CFG_CDR_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = result;
    /*if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }*/

    if(result==0)
    	{

	     stCdrNVRAMhdr *nvramHeader = (stCdrNVRAMhdr*)NVRAM_CDR_BASE;
	     if(bspEnableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)))==TRUE)
	  	{

		     nvramHeader->cdrOnOff = onoff;
		     nvramHeader->cdrPeriod = period;
		     nvramHeader->IP_CB3000 = Ip;
		     nvramHeader->intval = intval;
		     nvramHeader->Initialized = 0x20081218;//M_CDR_NVRAM_INITIALIZED;
            	     nvramHeader->LocalBtsId = bspGetBtsID();
		     
		        bspDisableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)));
	  	}
	      	{
			l3oambspNvRamWrite((char*)&NvRamDataAddr->stCdr_para.Initialized,(char*)&nvramHeader->Initialized,sizeof(stCdrNVRAMhdr));
		}
		uiCdrSwitch = onoff;
		gcdrSendPeriod = period*60;
		IP_CB3000 = Ip;
		cdr_para_err_flag = 0;
    	}
    /*Nvram写成功后再发响应*/	
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
    
}


void CTaskCfg::CM_CdrParaGet(CMessage& rMsg)
{
	CComMessage* pComMsg = new (this, 12) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_HandoverParaGet.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_GET_CDR_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;//result
     stCdrNVRAMhdr *nvramHeader = (stCdrNVRAMhdr*)NVRAM_CDR_BASE;
    *((UINT8*)pComMsg->GetDataPtr()+4) = nvramHeader->cdrOnOff;
    *((UINT8*)pComMsg->GetDataPtr()+5) = nvramHeader->cdrPeriod;
    *((UINT32*)((UINT8*)pComMsg->GetDataPtr()+6)) = nvramHeader->IP_CB3000;
    *((UINT16*)((UINT8*)pComMsg->GetDataPtr()+10)) = nvramHeader->intval;
    
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
}


void CTaskCfg::CM_STimeParaCfgPro(CMessage& rMsg)
{
    if (false == bGpsStatus)
    {
        UINT8 us3[4];
        memcpy( us3, (UINT8*)rMsg.GetDataPtr(), 2);
        us3[2] = 0;
        us3[3] = 1;//fail
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] CM_STimeParaCfgPro, gps isn't OK, return fail to EMS");
        CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_CFG_STIME_RSP,us3, 4); 
        return;
    }
     UINT32 ip = *((UINT32 *)((UINT8*)rMsg.GetDataPtr()+2));
 //    UINT16 port = *((UINT16 *)((UINT8*)rMsg.GetDataPtr()+6));
   //  UINT16 period = *((UINT16 *)((UINT8*)rMsg.GetDataPtr()+8));
     UINT32 valid = 0x20110401;
    
     l3oambspNvRamWrite( (char*)&NvRamDataAddr->sTimePara.valid, (char*)&valid, sizeof(UINT32)); 
     l3oambspNvRamWrite( (char*)&NvRamDataAddr->sTimePara.T_server_ip, ((char*)rMsg.GetDataPtr())+2, (sizeof(T_STime_CFG)-sizeof(UINT32)));
     UINT8 us3[4];
     memcpy( us3, (UINT8*)rMsg.GetDataPtr(), 2);
     memset( us3+2, 0, 2);
     OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_STimeParaCfgPro");
     CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_CFG_STIME_RSP,us3, 4);  
     if(ip!=0)
     {
     
		if(pHLRStimer!=NULL)
		{
			pHLRStimer->Stop();
			delete pHLRStimer;
        		pHLRStimer = NULL;
		}
	       pHLRStimer = CM_Createtimer( M_OAM_CFG_HLR_TIME_REQ, true, NvRamDataAddr->sTimePara.STime_period*1000 );
		if( pHLRStimer )
			pHLRStimer->Start();
     }
}


void CTaskCfg::CM_StimeParaGet(CMessage & rMsg)
{
	CComMessage* pComMsg = new (this, 12) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_HandoverParaGet.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_GET_STIME_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;//result
    memcpy(((UINT8*)pComMsg->GetDataPtr()+4),(char*)&NvRamDataAddr->sTimePara.T_server_ip,(sizeof(T_STime_CFG)-sizeof(UINT32)));
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
}

void CTaskCfg::CM_RFOPParaCfgPro(CMessage& rMsg)//网络开关RF配置
{

     UINT32 valid = 0x55aa55aa;
    RF_Operation temp_rf;
	UINT8 us3[4];
     memcpy( us3, (UINT8*)rMsg.GetDataPtr(), 2);
     memset( us3+2, 0, 2);
    
 
     if(NvRamDataAddr->Wcpe_Switch==0x5a5a)
	  {
	            us3[3] = 1;//回失败
		  g_rf_openation.type = 0;
			 
		OAM_LOGSTR(LOG_CRITICAL, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_RFOPParaCfgPro:curent mode is wcpe the RF CFG disabled");
             CM_SendComMsg( M_TID_EMSAGENTTX, M_EMS_RF_OPERA_CFG_RSP,us3, 4);  
		return;
	 }
     l3oambspNvRamWrite( (char*)&NvRamDataAddr->rf_operation.flag, (char*)&valid, sizeof(UINT32)); 
     l3oambspNvRamWrite( (char*)&NvRamDataAddr->rf_operation.type, ((char*)rMsg.GetDataPtr())+2, (sizeof(RF_Operation)-sizeof(UINT32)));
  //  temp_rf.flag = 0x55aa55aa;
  //  temp_rf.type = 
   

    if(NvRamDataAddr->rf_operation.type>4)
    	{
    	     g_rf_openation.type = 0;
    	}
    else
    	{
    	    
     		g_rf_openation.type = NvRamDataAddr->rf_operation.type;
		
    	}
    if( NvRamDataAddr->rf_operation.Close_RF_Time_Len<60)
    	{
    	     g_rf_openation.Close_RF_Time_Len = 60;
    	}
    else
    	{
    	
         g_rf_openation.Close_RF_Time_Len = NvRamDataAddr->rf_operation.Close_RF_Time_Len;
    	}
    if(NvRamDataAddr->rf_operation.Open_RF_Time_Len<30)
    	{
    	     g_rf_openation.Open_RF_Time_Len = 30;
    	}
    else
    	{
  		 g_rf_openation.Open_RF_Time_Len = NvRamDataAddr->rf_operation.Open_RF_Time_Len;
    	}

	if(g_rf_openation.GateWayIP1!= NvRamDataAddr->rf_operation.GateWayIP1)
	{
	     g_rf_openation.GateWay1_valid = 0;
	    memset(g_rf_openation.GateWay1_MAC,0,6);
	    g_rf_openation.Gateway1_bad_time =0;
	     g_rf_openation.GateWay1_UP= 0;
           g_rf_openation.GateWay1_down = 0;
		
	}
	if(g_rf_openation.GateWayIP2!= NvRamDataAddr->rf_operation.GateWayIP2)
	{
	     g_rf_openation.GateWay2_valid = 0;
	    memset(g_rf_openation.GateWay2_MAC,0,6);
	     g_rf_openation.Gateway2_bad_time =0;
	      g_rf_openation.GateWay2_UP= 0;
           g_rf_openation.GateWay2_down = 0;
	}
    g_rf_openation.GateWayIP1 = NvRamDataAddr->rf_operation.GateWayIP1;
   g_rf_openation.GateWayIP2 = NvRamDataAddr->rf_operation.GateWayIP2;
	 g_rf_openation.Voice_Bad_Time = 0;
     
     OAM_LOGSTR4(LOG_CRITICAL, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_RFOPParaCfgPro:%x,%x,%x,%x", NvRamDataAddr->rf_operation.type, NvRamDataAddr->rf_operation.Close_RF_Time_Len,NvRamDataAddr->rf_operation.Open_RF_Time_Len, NvRamDataAddr->rf_operation.GateWayIP1);
     CM_SendComMsg( M_TID_EMSAGENTTX, M_EMS_RF_OPERA_CFG_RSP,us3, 4);  



    //printf("%x,%x,%x,%x,%x,%x\n",g_rf_openation.type,g_rf_openation.Close_RF_Time_Len,g_rf_openation.Open_RF_Time_Len ,  g_rf_openation.GateWayIP1,g_rf_openation.GateWayIP2 ,0);

}
void CTaskCfg::CM_RFOPParaGet(CMessage& rMsg)//网络开关RF查询
{
    	CComMessage* pComMsg = new (this, 14+2+2) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_HandoverParaGet.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_EMS_RF_OPERA_GET_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;//result
    memcpy(((UINT8*)pComMsg->GetDataPtr()+4),(char*)&NvRamDataAddr->rf_operation.type,(sizeof(RF_Operation)-sizeof(UINT32)));
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
}

TID CTaskCfg :: GetEntityId() const
{
    return M_TID_CM;
}

void CTaskCfg :: SetSysStatus(CTaskCfg :: OAM_CFG_STATUS Status )
{
        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]SetSysStatus() Status[%02d]", Status);
#ifdef NUCLEAR_CODE
	if( OAM_CFG_NORMAL == Status )
	{
		UINT16 us[2];
		us[0]= 0;
		us[1]= m_ucLimitFlag;
		CM_SendComMsg( M_TID_L2MAIN, MSGID_LIMITAREA_L2L3_CFG_REQ, (UINT8*)us, 4 );
	}
#endif
    m_SysStatus = Status;
}

CTaskCfg :: OAM_CFG_STATUS CTaskCfg :: GetSysStatus() 
{
    return  m_SysStatus;
}

void CTaskCfg :: InitFirstDataFromNvram()
{
    //从NVram读取配置数据
    T_DataServiceCfgEle *pCfgEle = (T_DataServiceCfgEle *)&(NvRamDataAddr->DataServiceCfgEle);
    CCfgArpDataServiceReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetDstTid(M_TID_ARP);  
    CfgReq.SetP2PBridging(pCfgEle->P2PBridging);
    CfgReq.SetEgrARPRroxy(pCfgEle->EgrARPRroxy);
    CM_InitSendCfgReq(CfgReq);
    OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config ARP...");
    return;
}
#ifdef WBBU_CODE
void CTaskCfg ::CM_SendPerfCfg()
{
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->PerfLogCfgEle);
            CCfgPerfLogReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetDstTid(M_TID_PM);  
            CfgReq.SetEle(pCfgEle, sizeof(T_PerfLogCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config VLAN Group OK, now config PerfLog FTP server...");
}
#endif
void CTaskCfg :: InitOtherDataFromNvram(CMessage &rMsg)
{
  static UINT8 EnterCnt = 1;
    CL3OamCommonRsp Rsp(rMsg);
    UINT16 Rst = Rsp.GetResult();
    UINT16 MsgId = rMsg.GetMessageId();
    //停掉 transaction 
    UINT16 TransId = Rsp.GetTransactionId();
    CTransaction * pTransaction = FindTransact(TransId);
    CTaskPM *tpmt = CTaskPM::GetInstance();
	if(M_OAM_CFG_HLR_TIME_REQ== MsgId)//wangwenhua add 2012-11-9
	{
	     return;
	}
#ifndef WBBU_CODE   
    if( MSGID_L2_L3_SAVEPWR_CFG_RSP == MsgId )
	{
		if( SAVE_POWER_HW_NOT_SUPPORT == Rst || SAVE_POWER_SW_NOT_SUPPORT == Rst )
		{
	        OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_BTS_INIT_FAIL, "[tCfg] L2 savepwr fail {2-HW:3-SW}[%d].", Rst );
			Rst = 0;
			CM_SavePwrExitModel();
		}
	}
#endif
    if(0 != Rst)
    {
        //#ifndef WBBU_CODE
        //如果nvram启动失败了，首先看看备份数据能否使用
        if(getNvramBakData()==true)
        {
            //重新发起配置
            InitFirstDataFromNvram();            
            return;
        }
      //  #endif
		//#ifdef LJF_WCPE_BOOT
		if( m_bWcpeBootModel && m_bBootFromNV )
		{
			CTaskSystem::GetInstance()->SYS_SendDataDLNotify();
			SetSysStatus( OAM_CFG_INIT_FROM_EMS );
			//m_bBootFromNV = false;
			m_bNVBootFail = true;
		    OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_BTS_INIT_FAIL, "[tCfg] Init from NVRAM fail! wcpe=true Config task[%d], Msg[0x%04x], Result[0x%04x] fail.", rMsg.GetSrcTid(), MsgId, Rst);
		}
		else
		{
		    OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_BTS_INIT_FAIL, "[tCfg] Init from NVRAM fail! wcpe=false Config task[%d], Msg[0x%04x], Result[0x%04x] fail.", rMsg.GetSrcTid(), MsgId, Rst);
		    bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
		    CM_SendBtsResetMsg(L3SYS_ERROR_SYS_DATA_INIT_FAIL);
		    return;
		}
    }
    if(pTransaction)
    {
        pTransaction->EndTransact();
        delete pTransaction;
    }
    else
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_BTS_INIT_FAIL, "[tCfg] find pTransaction fail!  Config task[%d], Msg[0x%04x] fail.", rMsg.GetSrcTid(), MsgId);
#ifdef WBBU_CODE
	 if(MsgId==0x3012)
	  {
	     // if((EnterCnt>1)&&(EnterCnt<L1_CAL_DATA_CNTS))
	      	{
	      	   // MsgId = 0x301b;
	      	   return;
	      	}
	  }
#endif
    }
    
   
    switch(MsgId)
    {
    //#ifdef LJF_WCPE_BOOT
		case M_OAM_DATA_CFG_INIT_NOTIFY:
		{
           	OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] InitOtherDataFromNvram RECV M_OAM_DATA_CFG_INIT_NOTIFY before m_sysStatus==1");
			break;
		}
        case M_ARP_CFG_DATA_SERVICE_CFG_RSP:
        {  
            T_DataServiceCfgEle *pCfgEle = (T_DataServiceCfgEle *)&(NvRamDataAddr->DataServiceCfgEle);
            CCfgDmDataServiceReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetDstTid(M_TID_DM);  
            CfgReq.SetMobility(pCfgEle->Mobility);
            CfgReq.SetAccessControl(pCfgEle->AccessControl);
            CfgReq.SetLBATimerLen(pCfgEle->LBATimerLen);
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config ARP OK, now config DM...");
            break;
        }    
        case M_DM_CFG_DATA_SERVICE_CFG_RSP:
        {   
            T_DataServiceCfgEle *pCfgEle = (T_DataServiceCfgEle *)&(NvRamDataAddr->DataServiceCfgEle);
            CCfgSnoopDataServiceReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetDstTid(M_TID_SNOOP);  
            CfgReq.SetRoutingAreaID(pCfgEle->RoutingAreaID);
            CfgReq.SetTargetBtsID(pCfgEle->TargetBTSID);
            CfgReq.SetTargetEID(pCfgEle->TargetEID);
            CfgReq.SetTargetPPPoEEID(pCfgEle->TargetPPPoEEID);
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config DM OK, now config SNOOP...");
            break;
        }
        
        case M_SNOOP_CFG_DATA_SERVICE_CFG_RSP:
        { 
            T_DataServiceCfgEle *pCfgEle = (T_DataServiceCfgEle *)&(NvRamDataAddr->DataServiceCfgEle);
            CCfgEbDataServiceReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetDstTid(M_TID_EB);  
            CfgReq.SetEgrBCFilter(pCfgEle->EgrBCFilter);
            CfgReq.SetPPPSessionLen(pCfgEle->PPPSessionLen);
            CfgReq.SetLBATimerLen(pCfgEle->LBATimerLen);
            CfgReq.SetAccessControl(pCfgEle->AccessControl);
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "Config SNOOP OK, now config EB...");
            break;
        }    
        
        case M_EB_CFG_DATA_SERVICE_CFG_RSP:
        {   
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->ToSCfgEle);
            CCfgTosReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetDstTid(M_TID_EB);  
            CfgReq.SetEle(pCfgEle, sizeof(T_ToSCfgEle)*MAX_TOS_ELE_NUM);
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config EB OK, now config QoS...");
            break;
        }

        case M_BTS_EMS_QOS_CFG_RSP:
        {  
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->ACLCfgEle);
            CCfgACLReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetDstTid(M_TID_DM);  
            CfgReq.SetEle(pCfgEle, sizeof(T_ACLCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config Qos OK, now config ACL...");
            break;
        }    
        case M_BTS_EMS_CFG_ACL_RSP:
        {
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->VlanGroupCfgEle);
            CCfgVlanGroupReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetDstTid(M_TID_EB);  
            CfgReq.SetEle(pCfgEle, sizeof(T_VlanGroupCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config ACL OK, now config VLAN Group...");
            break;
        }
        case M_BTS_EMS_VLAN_GROUP_CFG_RSP:
        {  
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->PerfLogCfgEle);
            CCfgPerfLogReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetDstTid(M_TID_PM);  
            CfgReq.SetEle(pCfgEle, sizeof(T_PerfLogCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config VLAN Group OK, now config PerfLog FTP server...");
            break;
        }
        
        case M_BTS_EMS_PERF_LOGGING_CFG_RSP:
        {
	     sendGrpLmtPara();
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config PerfLog FTP server OK, now config group limit para...");
	     break;
        }
	 case MSGID_L2_L3_GRP_LIMIT_CFG_RSP:
	{
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->N_parameter);
            CL3L2CfgN1ParameterReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_N1_PARAMETER_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);
            CfgReq.SetEle(pCfgEle);
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config group limit para OK, now config N1 PARAMETE...");
            break;
        }
        
        case M_L2_L3_N1_PARAMETER_RSP:
        {  
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->TempAlarmCfgEle);
            CCfgTempAlarmReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_TEMP_MONITOR_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);
            CfgReq.SetEle(pCfgEle, sizeof(T_TempAlarmCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config N1 PARAMETE OK, now config Temperature Alarm...");
            break;
        }    
        case M_L2_L3_TEMP_MONITOR_RSP://集群功能及参数配置请求
        {  
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->ClusterPara);
	     if(NvRamDataAddr->ClusterPara.cfg_flag!=0x5a5a)
	     {
	         T_CLUSTER_PARA cluster_para_temp;
		  cluster_para_temp.flag = 1;
		  cluster_para_temp.sleep_period = 32;
		  cluster_para_temp.Rsv_Ch_Resourse_Num = 2;
		  cluster_para_temp.rsv1 = 0;
		  cluster_para_temp.rsv2 = 0;
		  cluster_para_temp.rsv3 = 0;
		  cluster_para_temp.cfg_flag = 0x5a5a;
		  l3oambspNvRamWrite(pCfgEle, (SINT8*)&cluster_para_temp, sizeof(T_CLUSTER_PARA));
	     }	
	     CCfgClusterParaReq CfgReq;	     
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(MSGID_CLUSTER_L2L3_PARA_CFG);
            CfgReq.SetDstTid(M_TID_L2MAIN);
            CfgReq.SetEle(pCfgEle, sizeof(T_CLUSTER_PARA)-2);
	     CM_InitSendCfgReq(CfgReq);     
            //printf("\n now config cluster para....... \n");
	     OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] config Temperature Alarm OK, now config cluster para...");
            break;
        }    
        case MSGID_CLUSTER_L2L3_PARA_CFG_RSP:
        {   
#if 0
            l3oamprintl2airlinknvromdata();
#endif
            //构造下一条配置数据 ,配置5.3.1 Airlink Configuration Request（EMS）
            //从NVram读取配置数据
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->AirLinkCfgEle);
#ifdef WBBU_CODE
             OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] NVRAM CM_CarrierDataCfgReq ts:%d\n",pDataAirLinkCfg->DLTSNum);
		if((NvRamDataAddr->AirLinkCfgEle.DLTSNum==1)||(NvRamDataAddr->AirLinkCfgEle.DLTSNum==7))
		{
		    OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] NVRAM CM_CarrierDataCfgReq ts ERR :%d,not support 1:7 or 7:1 mode\n",pDataAirLinkCfg->DLTSNum);
		}
		else
		{
             OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] NVRAM CM_CarrierDataCfgReq ts:%d\n",pDataAirLinkCfg->DLTSNum);
		}
		    SetTsMode(NvRamDataAddr->AirLinkCfgEle.DLTSNum,1,0);//add 20100128
#endif
            CM_L2AirLinkCfg();
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] config Cluster para OK, now config L2AirLink...");
            break;
        }
            
        case M_L2_L3_AIRLINK_DATA_CFG_RSP:
        {   
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->RMPoliceEle);
            CCfgRMPolicyReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_RES_MANAGE_POLICY_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);  
            CfgReq.SetEle(pCfgEle, sizeof(T_RMPoliceEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config L2AirLink OK, now config RM policy...");
            break;
        }
            
        case M_L2_L3_RES_MANAGE_POLICY_RSP:
        {  
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->AirLinkMisCfgEle);
            CCfgAirLinkMiscReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_AIR_LINK_MISC_CFG_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);  
            CfgReq.SetEle(pCfgEle, sizeof(T_AirLinkMisCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config RM policy OK, now config AirLinkMis...");
            break;
        }
        case M_L2_L3_AIR_LINK_MISC_CFG_RSP:
        {   
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config AirLinkMis OK, now config Billing Data and Diag user...");
            CM_L2BillDataAndDiagUserCfg();
            break;
        }   

        case M_L2_L3_BILLING_DATA_CFG_RSP:
        {
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->RfCfgEle);
            CCfgRfReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_RF_CFG_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);  
            CfgReq.SetEle(pCfgEle, sizeof(T_RfCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config Billing Data OK, now config RF...");
            break;
        }  
        
        case M_L2_L3_RF_CFG_RSP:
	    {
			//ems ranging cfg
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->RangingPara);
            if(NvRamDataAddr->RangingPara.cfg_flag!=0x5a5a)
	        {
                T_RangingPara ranging_para_temp;
                ranging_para_temp.Ranging_Switch = 2;
                ranging_para_temp.Enable_Shreashold = 40;
                ranging_para_temp.Disable_Shreahold = 35;
                ranging_para_temp.Ratio_Shreahold = 80;
                ranging_para_temp.SNR_Shreahold = 10;
                ranging_para_temp.Ranging_Offset_Shreahold = 46;
                ranging_para_temp.cfg_flag = 0x5a5a;
                l3oambspNvRamWrite(pCfgEle, (SINT8*)&ranging_para_temp, sizeof(T_RangingPara));
	        }	
            CCfgRRangingReq CfgReq;	     
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(MSGID_L3_L2_REMOTE_RANGE_CFG);
            CfgReq.SetDstTid(M_TID_L2MAIN);
            CfgReq.SetEle(pCfgEle, sizeof(T_RangingPara)-2);
            CM_InitSendCfgReq(CfgReq);     
            //printf("\n now config remote ranging para....... \n");
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config RF OK, now config Remote Ranging...");
            break;
        
        }
	case MSGID_L2_L3_REMOTE_RANGE_RSP:
        {
	     //应答回来后给rpc发送消息，然后再转入下一条消息处理
            CM_sendMsgToRpc();
		 
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->CaliGenCfgEle);
            CCfgCalGenDataReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_CALIBRAT_CFG_GENDATA_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);  
            CfgReq.SetEle(pCfgEle, sizeof(T_CaliGenCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config RF OK, now config Calibration General...");
            break;
        }

        case M_L2_L3_CALIBRAT_CFG_GENDATA_RSP:
        {
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->CaliDataEle[0]);
            CCfgCalDataReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_CALIBRAT_CFG_DATA_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);  
            CfgReq.SetEle(pCfgEle, sizeof(T_CaliDataEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config Calibration General OK, now config Calibration Data[0]...");
            break;
        }    
        
        case M_L2_L3_CALIBRAT_CFG_DATA_RSP:
        {   
            static UINT8 EnterCnt = 1;
            if(EnterCnt < L1_CAL_DATA_CNTS)
            {
                SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->CaliDataEle[EnterCnt]);
                CCfgCalDataReq CfgReq;
                CfgReq.CreateMessage(*this);
                CfgReq.SetMessageId(M_L3_L2_CALIBRAT_CFG_DATA_REQ);
                CfgReq.SetDstTid(M_TID_L2MAIN);  
                CfgReq.SetEle(pCfgEle, sizeof(T_CaliDataEle));
                CM_InitSendCfgReq(CfgReq);
                OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config Calibration General OK, now config Calibration Data[%d]...", EnterCnt);
            }
            else if(L1_CAL_DATA_CNTS == EnterCnt)
            {
                T_L1GenCfgEle *pCfgEle = (T_L1GenCfgEle *)&(NvRamDataAddr->L1GenCfgEle);
                m_gL1Cfg.AntennaMask   = pCfgEle->AntennaMask;
                m_gL1Cfg.SyncSrc       = pCfgEle->SyncSrc;
                m_gL1Cfg.GpsOffset     = pCfgEle->GpsOffset;
                CCfgL1GenDataReq CfgReq;
                CfgReq.CreateMessage(*this);
                CfgReq.SetMessageId(M_L3_L2_L1_GENERAL_SETTING_REQ);
                CfgReq.SetDstTid(M_TID_L2MAIN);  
                CfgReq.SetSyncSrc(pCfgEle->SyncSrc);
                CfgReq.SetGPSOffset(pCfgEle->GpsOffset);
                CfgReq.SetAntennaMask(pCfgEle->AntennaMask);
		  Cal_Data_Cfg_Return_Ok = 1;
#ifdef WBBU_CODE
              #if 0
              UINT8 uctemp1, uctemp2;
             //解析mask每一位,判断是否要告警
             for(int antcount=0; antcount<8; antcount++)
            {
                uctemp1 = (m_gL1Cfg.AntennaMask&(1<<antcount))>>antcount;//取出每一个bit
                uctemp2 = (antennaMaskAlmInfo&(1<<antcount))>>antcount;
                if((uctemp1 == 0)&&(uctemp2 == 0))//关闭射频,此前无告警
                {
                    antennaMaskAlmInfo = antennaMaskAlmInfo|(1<<antcount);//添加告警
                    AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       antcount,
                                       ALM_ID_RF_RF_DISABLED,
                                       ALM_CLASS_CRITICAL,
                                       STR_RF_DISABLED, antcount + 1);
                }
                else if((uctemp1 == 1)&&(uctemp2 == 1))
                {
                    antennaMaskAlmInfo = antennaMaskAlmInfo&(~(1<<antcount));//清除告警
                    AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       antcount,
                                       ALM_ID_RF_RF_DISABLED,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
                //其他情况不变
            }
            #endif
              if(pCfgEle->SyncSrc == 1)
            	{
            	   Set_Fpga_Clk(2);
            	   SetOffset(pCfgEle->GpsOffset);//add 20100202
            	}
            else  if(m_gL1Cfg.SyncSrc==0)
            	{
            	    Set_Fpga_Clk(0);
            	}
		else
		{
		    Set_Fpga_Clk(3);
		}
#endif
                if ((true != bGpsStatus)&&(BTS_SYNC_SRC_GPS == pCfgEle->SyncSrc))//如果gps还未正常工作，且配置的是本地时钟，则关掉所有RF
                {
                    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] GPS not ready, config Antenna Mask with 0x0000");
                    CfgReq.SetAntennaMask(0X0000);
                    AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                                  ALM_ID_GPS_LOST, ALM_CLASS_CRITICAL, STR_GPS_LOST);
                }  
                #ifndef WBBU_CODE
                #ifdef NUCLEAR_CODE
                if(BTS_SYNC_SRC_485 == pCfgEle->SyncSrc)//核电项目增加485配置,但告诉一层为gps
                {
                    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Sync Src is not GPS, is 485.");
                    AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                              ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_GPS_CFG_ERROR_485);
                    CfgReq.SetSyncSrc(BTS_SYNC_SRC_GPS);
                }
                #endif
                #endif
                CM_InitSendCfgReq(CfgReq);
                OAM_LOGSTR (LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config Calibration Data OK, now config L1 General setting...");
                if(BTS_SYNC_SRC_BTS == pCfgEle->SyncSrc)
                {
                    AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                          ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_GPS_CFG_ERROR); 
                    bGPSCfgDataErrorAlm = true;
                    pGPSCfgDataErrorAlmTimer->Start();
                    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] WARNING!!!Sync Src is not GPS");
                }
#ifdef WBBU_CODE
		  if ((true != bGpsStatus)&&(BTS_SYNC_SRC_GPS == pCfgEle->SyncSrc))
		  	{
		  	    WrruRFC(0x0000,1,11 );//wangwenhua add 20091116
		  	}
		  else
		  	{
                WrruRFC((m_gL1Cfg.AntennaMask&(~Calibration_Antenna)),1,10 );//wangwenhua add 20091116
		  	}
#endif
            }
            EnterCnt++;  //增加接收应答记数
            break;                                
        }
#ifndef WBBU_CODE        
        case M_L2_L3_L1_GENERAL_SETTING_RSP:
        {
            neighborListRefresh();
#ifdef M_TGT_WANIF
            CM_ConfigWanCPE();
#endif
			CM_SavePwrCfg();

            break;
        }     

#if 0
        case M_L2_L3_CFG_WANIF_CPE_RSP:
			if( NvRamDataAddr->ucVacPrefScg==0 || NvRamDataAddr->ucVacPrefScg==1 )
				CM_VacPrefScg();
			else
			{
	            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_BTS_INIT_SUCCESS, "[tCfg] All Configuration is OK. Boot from NvRam success!");
	            SetSysStatus(OAM_CFG_NORMAL);
	            CM_SendBtsWorkingMsg();
			}
			break;
#endif
		case MSGID_L2_L3_SAVEPWR_CFG_RSP:
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] InitOtherDataFromNvram:MSGID_L2_L3_SAVEPWR_CFG_RSP");
			if( NvRamDataAddr->ucVacPrefScg==0 || NvRamDataAddr->ucVacPrefScg==1 )
				CM_VacPrefScg();
			else
			{
	            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_BTS_INIT_SUCCESS, "[tCfg] All Configuration is OK. Boot from NvRam success!");
	            SetSysStatus(OAM_CFG_NORMAL);
	            CM_SendBtsWorkingMsg();
			}
			break;
		case MSGID_L2_L3_VACPREFSCG_RSP:
            CM_Qam64();
	     CM_LoadBlnInfo();
            break;
		case MSGID_L2_L3_QAM64_RSP:
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] InitOtherDataFromNvram:MSGID_L3_L2_VACPREFSCG_CFG");
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_BTS_INIT_SUCCESS, "[tCfg] All Configuration is OK. Boot from NvRam success!");
            SetSysStatus(OAM_CFG_NORMAL);
            CM_SendBtsWorkingMsg();
            tpmt->InitRTMonitor();
            tpmt->PM_StartRTMonitor();
            break;
#else
        case M_L2_L3_L1_GENERAL_SETTING_RSP:
        {
            neighborListRefresh();
#ifdef M_TGT_WANIF
            CM_ConfigWanCPE();
//#else
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_BTS_INIT_SUCCESS, "[tCfg] All Configuration is OK. Boot from NvRam success!");
            SetSysStatus(OAM_CFG_NORMAL);
            CM_SendBtsWorkingMsg();
#endif
			if( NvRamDataAddr->ucVacPrefScg==0 || NvRamDataAddr->ucVacPrefScg==1 )
				CM_VacPrefScg();
                   CM_Qam64();
		     CM_LoadBlnInfo();
            break;
        }     
        
#endif
        case M_OAM_CFGDATAINIT_FAIL_NOTIFY:    //配置数据无应答
        {
           // #ifndef WBBU_CODE
            if(getNvramBakData()==true)
            {
               //重新发起配置
               InitFirstDataFromNvram();
               break;
            }
         //  #endif
			//#ifdef LJF_WCPE_BOOT
			if( m_bWcpeBootModel && m_bBootFromNV )
			{
				CTaskSystem::GetInstance()->SYS_SendDataDLNotify();
				SetSysStatus( OAM_CFG_INIT_FROM_EMS );
				//m_bBootFromNV = false;		
				m_bNVBootFail = true;
	            OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] M_OAM_CFGDATAINIT_FAIL_NOTIFY true Config message[0x%04x] timeout, wcpe true", *((UINT16*)rMsg.GetDataPtr()+2));
			}
			else
			{
	            OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] M_OAM_CFGDATAINIT_FAIL_NOTIFY false Config message[0x%04x] timeout, wcpe false", *((UINT16*)rMsg.GetDataPtr()+2));
	            CM_SendBtsResetMsg(L3SYS_ERROR_SYS_DATA_INIT_FAIL);
			}
			break;
           
        }        
        
        default:
            break;
    }
}


bool CTaskCfg :: CM_SendBtsResetMsg(UINT16 Reason) //构造bts复位请求消息发往bts sys manage task ;
{
    CBtsRstNotify  NotifyMsg;
    if (false == NotifyMsg.CreateMessage(*this))
        return false;
    NotifyMsg.SetDstTid(M_TID_SYS);
    NotifyMsg.SetSrcTid(M_TID_CM);
    NotifyMsg.SetMessageId(M_OAM_BTS_RESET_NOTIFY);
    NotifyMsg.SetResetReason(Reason);
    if(true != NotifyMsg.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", NotifyMsg.GetMessageId());
        NotifyMsg.DeleteMessage();
    }

    return true;
}

bool CTaskCfg :: CM_SendBtsWorkingMsg() 
{
    //#ifndef WBBU_CODE
    //查看是否有nvram配置备份文件，如果没有则备份
    UINT32 nvramCrc = 0;
    if((nvramBakFileInfo.hasBakFile1==false)||(nvramBakFileInfo.hasBakFile2==false))
    {
        memcpy(&nvramBakFileInfo.bakFile[4], (UINT8*)NVRAM_BASE_ADDR_OAM, sizeof(T_NvRamData));
        UINT32 nvramCrc = CalcNvramCheckSum(&nvramBakFileInfo.bakFile[4]);
        memcpy(nvramBakFileInfo.bakFile, &nvramCrc, 4);
    }
    if(nvramBakFileInfo.hasBakFile1==false)
    {
        writeNvramToBakfile((char*)SM_NVRAM_CFG_DATA_FILENAME1, (char*)nvramBakFileInfo.bakFile, sizeof(T_NvRamData)+4, 1);
    }
    if(nvramBakFileInfo.hasBakFile2==false)
    {                
        writeNvramToBakfile((char*)SM_NVRAM_CFG_DATA_FILENAME2, (char*)nvramBakFileInfo.bakFile, sizeof(T_NvRamData)+4, 2);
    }
    if(NULL == nvramBakFileInfo.pBakDataTimer) 
    {
        //启动2小时周期定时器，如果出现配置改变，就同步到备份文件2中
        nvramBakFileInfo.pBakDataTimer = CM_Createtimer(M_OAM_PERIOD_BAK_NVRAMDATA_TIMER, true,  2*60*60 * 1000); // 2hours
        
    }
    if(NULL != nvramBakFileInfo.pBakDataTimer) 
    {
        nvramBakFileInfo.pBakDataTimer->Stop();
        nvramBakFileInfo.pBakDataTimer->Start();
    }
  //  #endif
//#ifdef LJF_WCPE_BOOT
	if( m_bWcpeBootModel && (!m_bWcpeBootModelOK) )
	{
		if( (!m_bBootFromNV) )//m_bWcpeBootModel 并且 init from ems
		{
			CTaskSystem::GetInstance()->SYS_SendBtsWorkingNotify();
			return true;
		}
	}

    CL3OamCommonReq NotifyMsg;
    NotifyMsg.CreateMessage(*this);
    NotifyMsg.SetDstTid(M_TID_SYS);
    NotifyMsg.SetSrcTid(M_TID_CM);
    NotifyMsg.SetMessageId(M_OAM_DATA_CFG_FINISH_NOTIFY);
    
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] send Data Init Finish Notify to tSys");
    if(true != NotifyMsg.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", NotifyMsg.GetMessageId());
        NotifyMsg.DeleteMessage();
    }
    return true;
}

bool CTaskCfg :: CM_BtsResetReq(CMessage& rMsg)
{
    //保存配置数据
    CCfgBtsRstReq ReqMsg(rMsg);

    if ( 0 == ReqMsg.GetDataSource() )
    {
        bspSetBootupSource(BTS_BOOT_DATA_SOURCE_BTS);
    }
    else
    {
        bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
    }
    
    //向ems返回应答消息
    CM_PostCommonRsp(M_TID_EMSAGENTTX, ReqMsg.GetTransactionId(), M_BTS_EMS_BTS_RESET_RSP, OAM_SUCCESS);    

    //向系统管理模块发送复位消息
    CM_SendBtsResetMsg(L3SYS_ERROR_BTS_RESET_OMCREQ);

    return true;
}


//下面的消息是发送到各任务部分的
bool CTaskCfg :: CM_BtsDataServiceCfgReq(CMessage& rMsg)
{
//暂时保存配置内容
    pDataServiceCfg = new T_DataServiceCfgEle;
    if(NULL == pDataServiceCfg)
    { return false;}
    memcpy(pDataServiceCfg, 
           (SINT8*)(rMsg.GetDataPtr()) + SIZEOF_TRANSID, 
           sizeof(T_DataServiceCfgEle));
//构造请求消息
    CCfgDataServiceReq ReqMsg(rMsg);

//配置arp任务
    CCfgArpDataServiceReq ArpCfgReq;
    ArpCfgReq.CreateMessage(*this);
    ArpCfgReq.SetDstTid(M_TID_ARP);  
    ArpCfgReq.SetTransactionId(ReqMsg.GetTransactionId());
    ArpCfgReq.SetP2PBridging(ReqMsg.GetP2PBridging());
    ArpCfgReq.SetEgrARPRroxy(ReqMsg.GetEgrARPRroxy());
    
    //构造配置失败消息       
    CL3OamCommonRsp ArpRspMsg;
    ArpRspMsg.CreateMessage(*this);
    ArpRspMsg.SetDstTid(M_TID_CM);
    ArpRspMsg.SetMessageId(M_ARP_CFG_DATA_SERVICE_CFG_RSP);
    ArpRspMsg.SetResult(L3CM_ERROR_TIMER_OUT);
    //创建配置数据transaction,发配置请求消息
    CTransaction* pCfgTransaction = CreateTransact(ArpCfgReq, ArpRspMsg, OAM_REQ_RESEND_CNT3, OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        ArpCfgReq.DeleteMessage();
        ArpRspMsg.DeleteMessage();
        return false;
    }
    ArpRspMsg.SetTransactionId(ReqMsg.GetTransactionId());
    ArpCfgReq.SetTransactionId(pCfgTransaction->GetId());
    if(false == pCfgTransaction->BeginTransact())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] send msg = 0x%04x to task %d failed! can not creat transaction!", ArpCfgReq.GetMessageId(), ArpCfgReq.GetDstTid());
        pCfgTransaction->EndTransact();//如果失败释放处理jiaying20100811
        delete pCfgTransaction;
        return false;
    }

//配置dm任务
    CCfgDmDataServiceReq DmCfgReq;
    DmCfgReq.CreateMessage(*this);
    DmCfgReq.SetDstTid(M_TID_DM);  
    DmCfgReq.SetTransactionId(ReqMsg.GetTransactionId());
    
    DmCfgReq.SetMobility(ReqMsg.GetMobility());
    DmCfgReq.SetAccessControl(ReqMsg.GetAccessControl());
    DmCfgReq.SetLBATimerLen(ReqMsg.GetLBATimerLen());
    //构造配置失败消息       
    CL3OamCommonRsp DmRspMsg;
    DmRspMsg.CreateMessage(*this);
    DmRspMsg.SetDstTid(M_TID_CM);
    DmRspMsg.SetMessageId(M_DM_CFG_DATA_SERVICE_CFG_RSP);
    DmRspMsg.SetResult(L3CM_ERROR_TIMER_OUT);
    //创建配置数据transaction
    pCfgTransaction = CreateTransact(DmCfgReq, DmRspMsg, OAM_REQ_RESEND_CNT3, OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        DmCfgReq.DeleteMessage();
        DmRspMsg.DeleteMessage();
        return false;
    }
    DmRspMsg.SetTransactionId(ReqMsg.GetTransactionId());
    DmCfgReq.SetTransactionId(pCfgTransaction->GetId());
    if(false == pCfgTransaction->BeginTransact())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] send msg = 0x%04x to task %d failed! can not creat transaction!", DmCfgReq.GetMessageId(), DmCfgReq.GetDstTid());
        pCfgTransaction->EndTransact();//如果失败释放处理jiaying20100811
        delete pCfgTransaction;
        return false;
    }

//配置snoop任务
    CCfgSnoopDataServiceReq SnoopCfgReq;
    SnoopCfgReq.CreateMessage(*this);
    SnoopCfgReq.SetDstTid(M_TID_SNOOP);  
    SnoopCfgReq.SetTransactionId(ReqMsg.GetTransactionId());
    SnoopCfgReq.SetRoutingAreaID(ReqMsg.GetRoutingAreaID());
    SnoopCfgReq.SetTargetBtsID(ReqMsg.GetTargetBtsID());
    SnoopCfgReq.SetTargetEID(ReqMsg.GetTargetEID());
    SnoopCfgReq.SetTargetPPPoEEID(ReqMsg.GetTargetPPPoEEID());

    //构造配置失败消息       
    CL3OamCommonRsp SnoopRspMsg;
    SnoopRspMsg.CreateMessage(*this);
    SnoopRspMsg.SetDstTid(M_TID_CM);
    SnoopRspMsg.SetMessageId(M_SNOOP_CFG_DATA_SERVICE_CFG_RSP);
    SnoopRspMsg.SetResult(L3CM_ERROR_TIMER_OUT);
    //创建配置数据transaction
    pCfgTransaction = CreateTransact(SnoopCfgReq, SnoopRspMsg, OAM_REQ_RESEND_CNT3, OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        SnoopCfgReq.DeleteMessage();
        SnoopRspMsg.DeleteMessage();
        return false;
    }
    SnoopRspMsg.SetTransactionId(ReqMsg.GetTransactionId());
    SnoopCfgReq.SetTransactionId(pCfgTransaction->GetId());
    if(false == pCfgTransaction->BeginTransact())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] send msg = 0x%04x to task %d failed! can not creat transaction!", SnoopCfgReq.GetMessageId(), SnoopCfgReq.GetDstTid());
        pCfgTransaction->EndTransact();//如果失败释放处理jiaying20100811
        delete pCfgTransaction;
        return false;
    }

//配置eb任务
    CCfgEbDataServiceReq EbCfgReq;
    EbCfgReq.CreateMessage(*this);
    EbCfgReq.SetDstTid(M_TID_EB);  
    EbCfgReq.SetTransactionId(ReqMsg.GetTransactionId());
    
    EbCfgReq.SetEgrBCFilter(ReqMsg.GetEgrBCFilter());
    EbCfgReq.SetPPPSessionLen(ReqMsg.GetPPPSessionLen());
    EbCfgReq.SetLBATimerLen(ReqMsg.GetLBATimerLen());
    EbCfgReq.SetAccessControl(ReqMsg.GetAccessControl());
    
    //构造配置失败消息       
    CL3OamCommonRsp EbRspMsg;
    EbRspMsg.CreateMessage(*this);
    EbRspMsg.SetDstTid(M_TID_CM);
    EbRspMsg.SetMessageId(M_EB_CFG_DATA_SERVICE_CFG_RSP);
    EbRspMsg.SetResult(L3CM_ERROR_TIMER_OUT);
    //创建配置数据transaction
    pCfgTransaction = CreateTransact(EbCfgReq, EbRspMsg, OAM_REQ_RESEND_CNT3, OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        EbCfgReq.DeleteMessage();
        EbRspMsg.DeleteMessage();
        return false;
    }
    EbRspMsg.SetTransactionId(ReqMsg.GetTransactionId());
    EbCfgReq.SetTransactionId(pCfgTransaction->GetId());
    if(false == pCfgTransaction->BeginTransact())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] send msg = 0x%04x to task %d failed! can not creat transaction!", EbCfgReq.GetMessageId(), EbCfgReq.GetDstTid());
        pCfgTransaction->EndTransact();//如果失败释放处理jiaying20100811
        delete pCfgTransaction;
        return false;
    }

    return true;
}

bool CTaskCfg :: CM_BtsDataServiceCfgRsp(CMessage& rMsg)
{
    static bool ArpRsp = false, DmRsp = false, EbRsp = false, SnoopRsp = false;


    //停掉 transaction 
    CL3OamCommonRsp RspMsg(rMsg);

    if(OAM_SUCCESS != RspMsg.GetResult())
    {
        if(OAM_CFG_INIT_FROM_EMS == m_SysStatus)
        {
        OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Data Service config fail! srctid[%d] msgid = 0x%04x, result = 0x%04x", rMsg.GetSrcTid(), rMsg.GetMessageId(), RspMsg.GetResult());
        //SetSysStatus(OAM_CFG_INIT_FAIL); 
        }
    }
    
    CL3OamCommonRsp CommonRsp;
    CTransaction * pTransaction = FindTransact(RspMsg.GetTransactionId());
    if(!pTransaction)
    { 
        OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Data Service config fail!  can not find transid! srctid[%d] msgid = 0x%04x, result = 0x%04x", rMsg.GetSrcTid(), rMsg.GetMessageId(), RspMsg.GetResult());
        return false;
    }
    
    pTransaction->EndTransact();
    switch(rMsg.GetMessageId())   
    {
        case M_ARP_CFG_DATA_SERVICE_CFG_RSP:
			OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ARP config OK");
            ArpRsp = true;
            break;
        case M_DM_CFG_DATA_SERVICE_CFG_RSP:
			OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] DM config OK");
            DmRsp = true;
            break;
        case M_EB_CFG_DATA_SERVICE_CFG_RSP:
			OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] EB config OK");
            EbRsp = true;
            break;
        case M_SNOOP_CFG_DATA_SERVICE_CFG_RSP:
			OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] SNOOP config OK");
            SnoopRsp = true;
            break;
        default:
            break;
    }
    
    if((true == ArpRsp)&&
       (true == DmRsp)&&
       (true == EbRsp)&&
       (true == SnoopRsp))
    {
        OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Data Service config OK");
        //向ems返回应答消息
        CM_PostCommonRsp(M_TID_EMSAGENTTX, pTransaction->GetRequestTransId(), M_BTS_EMS_DATA_SERVICE_CFG_RSP, OAM_SUCCESS);    

        //配置成功,保存数据
        l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->DataServiceCfgEle), (SINT8*)pDataServiceCfg, sizeof(T_DataServiceCfgEle));

        ArpRsp   = false; //恢复状态标志以备下次配置命令使用
        DmRsp    = false;
        EbRsp    = false;
        SnoopRsp = false;
        
        delete pDataServiceCfg;
        delete pTransaction;

        return true;
    }
    else
    {
        delete pTransaction;
        
        return false;
    }
}

bool CTaskCfg :: CM_CarrierDataCfgReq(CMessage& rMsg)
{
    //构造请求消息
    CCfgAirLinkReq Req(rMsg);
    memcpy(pDataAirLinkCfg, Req.GetEle(), sizeof(T_AirLinkCfgEle));
#ifdef WBBU_CODE
    OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] CM_CarrierDataCfgReq ts:%d\n",pDataAirLinkCfg->DLTSNum);
	 if((pDataAirLinkCfg->DLTSNum==1)||(pDataAirLinkCfg->DLTSNum==7))
   	{
	   	CL3OamCommonRsp CommonRsp;
	    CommonRsp.CreateMessage(*this);   // 4 = transid + result
	    CommonRsp.SetMessageId(M_BTS_EMS_CARRIER_DATA_CFG_RSP);
	    CommonRsp.SetDstTid(M_TID_EMSAGENTTX);
	    CommonRsp.SetTransactionId(rMsg.GetTransactionId());
	    CommonRsp.SetResult(1);
	    

	    if(true != CommonRsp.Post())
	    {
	       // OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", CommonRsp.GetMessageId());
	        CommonRsp.DeleteMessage();
	    }
	   	   OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] CM_CarrierDataCfgReq ts:%d ERR not support 1:7 or 7:1\n",pDataAirLinkCfg->DLTSNum);
		   return false;
   }
    SetTsMode(pDataAirLinkCfg->DLTSNum,1,0);//add 20100128
#endif
#if 0
    {
    printf("\r\nL2 AIR LINK DATA FROM EMS...........................\r\n");
    l3oamprintl2arilinkcfgdata(pDataAirLinkCfg);
    }
#endif

    CL3L2CfgAirLinkReq ReqMsg; 

    ReqMsg.CreateMessage(*this);
    ReqMsg.SetDstTid(M_TID_L2MAIN);
    ReqMsg.SetMessageId(M_L3_L2_AIRLINK_DATA_CFG_REQ);
    ReqMsg.SetTransactionId(Req.GetTransactionId());
    ReqMsg.SetBtsID(bspGetBtsID());
    ReqMsg.SetNetworkID(bspGetNetworkID());
    UINT32 BtsRstCnt = NvRamDataAddr->BTSCommonDataEle.BtsRstCnt;
    ReqMsg.SetResetCnt(BtsRstCnt);
    ReqMsg.SetEle(Req.GetEle());   

    //构造配置失败消息       
    CL3OamCommonRsp CfgFailRsp;
    CfgFailRsp.CreateMessage(*this);
    CfgFailRsp.SetMessageId(M_BTS_EMS_QOS_CFG_RSP);
    CfgFailRsp.SetDstTid(M_TID_EMSAGENTTX);
    CfgFailRsp.SetResult(OAM_TIMEOUT_ERR);

    //创建配置数据transaction
    CTransaction* pCfgTransaction = CreateTransact(ReqMsg, 
                                    CfgFailRsp, 
                                    OAM_REQ_RESEND_CNT3, 
                                    OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        ReqMsg.DeleteMessage();
        CfgFailRsp.DeleteMessage();
        return false;
    }
    ReqMsg.SetTransactionId(pCfgTransaction->GetId());
    CfgFailRsp.SetTransactionId(Req.GetTransactionId());
    if(false == pCfgTransaction->BeginTransact())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] send msg = 0x%04x to task %d failed! can not creat transaction!", ReqMsg.GetMessageId(), ReqMsg.GetDstTid());
        pCfgTransaction->EndTransact();//如果失败释放处理jiaying20100811
        delete pCfgTransaction;
        return false;
    }
    return true;
}

bool CTaskCfg :: CM_CarrierDataCfgRsp(CMessage& rMsg)
{
    //停掉 transaction 
    CL3OamCommonRsp RspMsg(rMsg);
////CL3OamCommonRsp CommonRsp;

    if(OAM_SUCCESS != RspMsg.GetResult())
    {
        if(OAM_CFG_INIT_FROM_EMS == m_SysStatus)
        {
        OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Carrier Data config fail! srctid[%d] msgid = 0x%04x, result = 0x%04x", rMsg.GetSrcTid(), rMsg.GetMessageId(), RspMsg.GetResult());
        }
    }
    else
    {
        //配置成功,保存数据
#if 0
        {
        printf("\r\nL2 AIR LINK DATA SAVED...........................\r\n");
        l3oamprintl2arilinkcfgdata(pDataAirLinkCfg);
        }
#endif
        l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->AirLinkCfgEle), (SINT8*)pDataAirLinkCfg, sizeof(T_AirLinkCfgEle));
    }
    
    CTransaction * pTransaction = FindTransact(RspMsg.GetTransactionId());
    if(!pTransaction)
    { 
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Carrier Data config fail!  can not find transid! srctid[%d] msgid = 0x%04x, result = 0x%04x", rMsg.GetSrcTid(), rMsg.GetMessageId());
        return false;
    }
    
    pTransaction->EndTransact();

    //向ems返回应答消息
    CM_PostCommonRsp(M_TID_EMSAGENTTX, pTransaction->GetRequestTransId(), M_BTS_EMS_DATA_SERVICE_CFG_RSP, OAM_SUCCESS);    

    delete pTransaction;
    
    return true;

}

bool CTaskCfg :: CM_CalibrationSetReq(CMessage& rMsg)
{
    //保存配置数据
    CCfgCalReq ReqMsg(rMsg);

    //设置nvram
    l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->CalCfgEle), ReqMsg.GetEle(), sizeof(T_CalCfgEle));
    
    //向ems返回应答消息
    CM_PostCommonRsp(M_TID_EMSAGENTTX, ReqMsg.GetTransactionId(), M_BTS_EMS_CALIBRAT_CFG_RSP, OAM_SUCCESS);    

    CM_CreateCalibrationTimer();

    return true;
}


//根据配置消息内容和当前系统状态决定是否启动周期定时器或
//UINT16  CalIner  ---- interval    0, 30~ 1440     In minutes, 0 is to disable periodic calibration
//UINT16  CalType  ---- type        0 - online 1 -- full    
#ifdef WBBU_CODE
unsigned int cal_test = 0;
unsigned int cal_long = 0;
extern "C" void Set_Cal_Time(unsigned int flag ,unsigned int len)
{
      //NvRamDataAddr->CalCfgEle.CalIner  = len;
      cal_test = flag;
      cal_long = len;
}
#endif 
bool CTaskCfg::CM_CreateCalibrationTimer()
{
#ifndef WBBU_CODE
    UINT32  CalIner = NvRamDataAddr->CalCfgEle.CalIner * 60 * 1000;
#else
 UINT32  CalIner;
  if(cal_test ==0)
  	{
     		CalIner = NvRamDataAddr->CalCfgEle.CalIner * 60 * 1000;
  	}
    else
  	{
  	   if(cal_long==0)
  	   	{
  	    CalIner = 5*60*1000;
  	   	}
  	       else
  	       {
  	            CalIner = cal_long*1000;
  	       }
  	}
#endif
    if(pCalibTimer)
        {
        //停止周期定时器    
        pCalibTimer->Stop();
        delete pCalibTimer;
        pCalibTimer = NULL;
        }

    if(0 != CalIner)
        {
        //启动周期校准TIMER
        CL3CalibrationTimer TimerMsg;
        if (false == TimerMsg.CreateMessage(*this))
            return false;
        TimerMsg.SetDstTid(M_TID_CM);
        pCalibTimer = new CTimer(M_TIMER_PERIOD_YES, CalIner, TimerMsg);
        if(pCalibTimer == NULL)
            {
            TimerMsg.DeleteMessage();
            return false;
            }
        else
            {
            pCalibTimer->Start();
            }
        }
    return true;
}

bool CTaskCfg :: CM_CalibrationTimer(CMessage &)
{
    pCalRstDataRcd->ucCalTrigger = 1;
    CM_SendCalibrationActMsg();
    return true;
}

bool CTaskCfg :: CM_SendCalibrationActMsg()
{
    CCfgCalActionReq ReqMsg;
    ReqMsg.CreateMessage(*this);
    ReqMsg.SetMessageId(M_L3_L2_INSTANT_CALIBRATION_REQ);
    ReqMsg.SetTransactionId(OAM_DEFAUIT_TRANSID);
    ReqMsg.SetDstTid(M_TID_L2MAIN);
    ReqMsg.SetCalType(NvRamDataAddr->CalCfgEle.CalType);
    ReqMsg.SetCalTrigger(pCalRstDataRcd->ucCalTrigger);    //0 - manual.
    if(true != ReqMsg.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post Calibration Action Req msg to L2Main fail");
        ReqMsg.DeleteMessage();
    }

    return true;
}
bool CTaskCfg :: CM_CalibResultGenNotify(CMessage &rMsg)
{
    //5.4.6.1  Calibration Result General Notification（BTS） M_BTS_EMS_CAL_GENDATA_NOTIFY
    CCfgCalRstGenNotify Notify(rMsg);
    Notify.SetMessageId(M_BTS_EMS_CAL_GENDATA_NOTIFY);
    Notify.SetSrcTid(M_TID_CM);
    Notify.SetDstTid(M_TID_EMSAGENTTX);  
    Notify.Post();    
#ifdef WBBU_CODE
    unsigned short  cal_result = Notify.IsCalibrationErr();
    unsigned char  channel_warm= cal_result>>8;
#endif
    //将calibration notification 上报ems
#ifndef WBBU_CODE
#if 0
    if(Notify.IsAntennaErr() || 
       Notify.IsPreCalibrationErr()||
       Notify.IsPreDistortErr()||
       Notify.IsSynErr())
#else
    if(false == Notify.IsCalibrationErr())
#endif
    {
        pCalRstDataRcd->IsDataCorrect = false;
        //pCalRstDataRcd->CalibrateOK   = false;
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] calibration result error");
    }
    else
    {
        pCalRstDataRcd->EntryCnt = 0; 
        pCalRstDataRcd->IsDataCorrect = true;
        //pCalRstDataRcd->IsCalibrating = false;
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] calibration result correct");
    }
#else
    //将calibration notification 上报ems
#if 0
    if(Notify.IsAntennaErr() || 
       Notify.IsPreCalibrationErr()||
       Notify.IsPreDistortErr()||
       Notify.IsSynErr())
#else
    if( /*Notify.IsCalibrationErr()*/(cal_result&0xff)!=1)
#endif
    {
        pCalRstDataRcd->IsDataCorrect = false;
        if(channel_warm!=0)//如果等于0，则可能是定时校准
        {
        Cfg_Annntena_mask(channel_warm,0);
        //pCalRstDataRcd->CalibrateOK   = false;
        #ifdef WBBU_CODE
            Calibration_Antenna = channel_warm;
      //  Last_Calibration_Antenna;
            l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Last_Calibration_Antenna), 
                               (SINT8*)&(channel_warm), 1);
      #if 0
           UINT8 uctemp1, uctemp2;
            //解析mask每一位,判断是否要告警
           for(int antcount=0; antcount<8; antcount++)
           {
               uctemp1 = (channel_warm&(1<<antcount))>>antcount;//取出每一个bit
               uctemp2 = (antennaMaskAlmInfo&(1<<antcount))>>antcount;
               if((uctemp1 == 1)&&(uctemp2 == 0))//关闭射频,此前无告警
               {
                   antennaMaskAlmInfo = antennaMaskAlmInfo|(1<<antcount);//添加告警
                   AlarmReport(ALM_FLAG_SET,
                                      ALM_ENT_RF,
                                      antcount,
                                      ALM_ID_RF_RF_DISABLED,
                                      ALM_CLASS_MAJOR,
                                      STR_RF_DISABLED, antcount + 1);
               }
               else if((uctemp1 == 0)&&(uctemp2 == 1))//射频打开,清除告警
               {
                   antennaMaskAlmInfo = antennaMaskAlmInfo&(~(1<<antcount));//清除告警
                   AlarmReport(ALM_FLAG_CLEAR,
                                      ALM_ENT_RF,
                                      antcount,
                                      ALM_ID_RF_RF_DISABLED,
                                      ALM_CLASS_MAJOR,
                                      STR_CLEAR_ALARM);
               }
               //其他情况不变
           }
           #endif
        #endif
        }
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] calibration result error:%x,channel:%x\n",Notify.IsCalibrationErr(),channel_warm);

//        4（校准过程中上行消息包校验和错）；
//      5（校准过程中下行消息包校验和错）；
//      6（WRRU未能正确进入校准，WBBU未执行校准流程）。
       if((cal_result&0xff)==4)
       {
	         AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_AUX, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_AUX_AUX_Calibration_Err,
                       ALM_CLASS_MAJOR,
                       STR_AUX_CAL_ERR_4); 
       }
	   else if((cal_result&0xff)==5)
	   {
	   	     AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_AUX, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_AUX_AUX_Calibration_Err,
                       ALM_CLASS_MAJOR,
                       STR_AUX_CAL_ERR_5); 
	   }
	   else if((cal_result&0xff)==6)
	   {
	   	    AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_AUX, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_AUX_AUX_Calibration_Err,
                       ALM_CLASS_MAJOR,
                       STR_AUX_CAL_ERR_6); 
	   }
    }
    else
    {
       Cfg_Annntena_mask(0,0);
        pCalRstDataRcd->EntryCnt = 0; 
        pCalRstDataRcd->IsDataCorrect = true;
          #ifdef WBBU_CODE
            Calibration_Antenna = 0;
              l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Last_Calibration_Antenna), 
                               (SINT8*)&(channel_warm), 1);
        #endif
        //pCalRstDataRcd->IsCalibrating = false;
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] calibration result correct");
	AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_AUX, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_AUX_AUX_Calibration_Err,
                       ALM_CLASS_MAJOR,
                       STR_AUX_CAL_Right); 
	m_send_cal_alarm = 0;
    #if 0
	 //wangwenhua modify 20111111 
	 /****************************************************
        重庆反馈当从全部校准不通过到全部校准通过时状态不对ems上天线掩码显示还是关闭的。
        但RF是打开的

	 ******************************************************/
          UINT8  uctemp3;
	  UINT8  uctemp4 = 0;
	  for(int antcount=0; antcount<8; antcount++)
           {
              
               uctemp3 = (antennaMaskAlmInfo&(1<<antcount))>>antcount;
		  uctemp4 = (NvRamDataAddr->L1GenCfgEle.AntennaMask)&0xff;
             if((uctemp3 == 1))//射频打开,清除告警
               {
                  if(uctemp4&(1<<antcount))
                  	{
	                   antennaMaskAlmInfo = antennaMaskAlmInfo&(~(1<<antcount));//清除告警
	                   AlarmReport(ALM_FLAG_CLEAR,
	                                      ALM_ENT_RF,
	                                      antcount,
	                                      ALM_ID_RF_RF_DISABLED,
	                                      ALM_CLASS_MAJOR,
	                                      STR_CLEAR_ALARM);
                  	}
               }
	  }
      #endif
    }
#endif
    memcpy(&(pCalRstDataRcd->GenResultNotify), Notify.GetEle(), sizeof(T_CaliResultGenNotifyEle));

    return true;
}

bool CTaskCfg :: CM_CalibResultNotify(CMessage &rMsg)
{
    static UINT16 RevIndex= 0;  //标识是否是顺序接收的数据
  int i;
    CCfgCalRstNotify Notify(rMsg);
#if 0
    Notify.SetDstTid(M_TID_EMSAGENTTX);  
    Notify.SetSrcTid(M_TID_CM);  
    Notify.SetMessageId(M_BTS_EMS_CAL_DATA_NOTIFY);
    Notify.Post();  //将calibration notification 上报ems
#endif
    memcpy(&(pCalRstDataRcd->NotifyData[RevIndex]), Notify.GetEle(), sizeof(T_CaliDataEle));
    RevIndex++;

    bool bFTPFile = false;

    if(L1_CAL_DATA_CNTS == RevIndex) //接收数据完成,根据是否保存标识决定是否保存到nvram
    {
        if(false == pCalRstDataRcd->IsDataCorrect)
        {
 // #ifndef WBBU_CODE
            if(++ (pCalRstDataRcd->EntryCnt) < 3)
            {
            #ifdef WBBU_CODE   //the same with the large BTS //wangwenhua modify 20110503
	     if(g_Close_RF_flag!=1)
	     	{
            		 sendAnntenaMsk(NvRamDataAddr->L1GenCfgEle.AntennaMask,0,6);
	     	}
		 else
		 {
		     	sendAnntenaMsk(0,0,6); 
		 }
	        taskDelay(2);
		#endif
                CM_SendCalibrationActMsg();
                OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] calibration result error, %d times", pCalRstDataRcd->EntryCnt);
            }
            else
	//#endif
            {
                bFTPFile = true;
                pCalRstDataRcd->EntryCnt = 0;
				#ifdef WBBU_CODE


                if(4== bspGetRRUChannelNum())//如果是4通道RRU的话,只要校准出了4跟，也认为正确
				{
				   if(Calibration_Antenna== 0xf0)//4跟天线校准正确
				   	{
				   	     l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->CaliDataEle), 
                               (SINT8*)&(pCalRstDataRcd->NotifyData), 
                               sizeof(pCalRstDataRcd->NotifyData)
                               );
            			l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->CaliGenCfgEle), 
                               (SINT8*)&(pCalRstDataRcd->GenResultNotify.GenCfgEle), 
                               sizeof(NvRamDataAddr->CaliGenCfgEle)
                               );

						T_ConfigCalibrationData_Msg    temp_Calibra;
						temp_Calibra.SYN_RxGain = (pCalRstDataRcd->GenResultNotify.GenCfgEle.SynRxGain);
						temp_Calibra.SYN_TxGain = (pCalRstDataRcd->GenResultNotify.GenCfgEle.SynTxGain);
						for( i = 0; i< ANTENNA_NUM;i++)
						{
							temp_Calibra.RF_RxGain[i] = pCalRstDataRcd->GenResultNotify.GenCfgEle.AnnetaInfo[i].RX_GAIN;
							temp_Calibra.RF_TxGain[i] = pCalRstDataRcd->GenResultNotify.GenCfgEle.AnnetaInfo[i].TX_GAIN;
						}
  
				      l3oambspNvRamWrite((char*)&(NvRamDataAddr->WRRUCfgEle.RXGAIN_RFB) ,(char *)(temp_Calibra.RF_RxGain),8);
			             l3oambspNvRamWrite((char*)&(NvRamDataAddr->WRRUCfgEle.TXGAIN_RFB) ,(char *)(temp_Calibra.RF_TxGain),8);
			            l3oambspNvRamWrite((char*)&(NvRamDataAddr->WRRUCfgEle.RXGAIN_SYN) ,(char *)(&(temp_Calibra.SYN_RxGain)),2);
			             l3oambspNvRamWrite((char*)&(NvRamDataAddr->WRRUCfgEle.TXGAIN_SYN) ,(char *)(&(temp_Calibra.SYN_TxGain)),2);
						OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] calibration result saved 4!");
				   	}
				}

				#endif
                //pCalRstDataRcd->IsDataCorrect = false;
#if 0
                l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->CaliDataEle), 
                                   (SINT8*)&(pCalRstDataRcd->NotifyData), 
                                   sizeof(pCalRstDataRcd->NotifyData)
                                   );
                l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->CaliGenCfgEle), 
                                   (SINT8*)&(pCalRstDataRcd->GenResultNotify.GenCfgEle), 
                                   sizeof(NvRamDataAddr->CaliGenCfgEle)
                                   );
                OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] calibration result saved!");
#endif
            }
        }
        else
        {
            bFTPFile = true;
            l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->CaliDataEle), 
                               (SINT8*)&(pCalRstDataRcd->NotifyData), 
                               sizeof(pCalRstDataRcd->NotifyData)
                               );
            l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->CaliGenCfgEle), 
                               (SINT8*)&(pCalRstDataRcd->GenResultNotify.GenCfgEle), 
                               sizeof(NvRamDataAddr->CaliGenCfgEle)
                               );
#ifdef WBBU_CODE
		T_ConfigCalibrationData_Msg    temp_Calibra;
		temp_Calibra.SYN_RxGain = (pCalRstDataRcd->GenResultNotify.GenCfgEle.SynRxGain);
		temp_Calibra.SYN_TxGain = (pCalRstDataRcd->GenResultNotify.GenCfgEle.SynTxGain);
		for( i = 0; i< ANTENNA_NUM;i++)
		{
			temp_Calibra.RF_RxGain[i] = pCalRstDataRcd->GenResultNotify.GenCfgEle.AnnetaInfo[i].RX_GAIN;
			temp_Calibra.RF_TxGain[i] = pCalRstDataRcd->GenResultNotify.GenCfgEle.AnnetaInfo[i].TX_GAIN;
		}
  
	      l3oambspNvRamWrite((char*)&(NvRamDataAddr->WRRUCfgEle.RXGAIN_RFB) ,(char *)(temp_Calibra.RF_RxGain),8);
             l3oambspNvRamWrite((char*)&(NvRamDataAddr->WRRUCfgEle.TXGAIN_RFB) ,(char *)(temp_Calibra.RF_TxGain),8);
            l3oambspNvRamWrite((char*)&(NvRamDataAddr->WRRUCfgEle.RXGAIN_SYN) ,(char *)(&(temp_Calibra.SYN_RxGain)),2);
             l3oambspNvRamWrite((char*)&(NvRamDataAddr->WRRUCfgEle.TXGAIN_SYN) ,(char *)(&(temp_Calibra.SYN_TxGain)),2);
#endif
            pCalRstDataRcd->EntryCnt = 0;
            //pCalRstDataRcd->IsDataCorrect = true;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Save calibration result to NVRAM!");
        }

        RevIndex = 0; //恢复记录标识
    }

    if (true == bFTPFile)
        {
        //write to a file and FTP to EMS
        UINT8 filename[64];
        getFileName(filename);
        Write2FileAndNotifyToFTP((char *)filename);
        }

    return true;
}


extern UINT32 GetSystemTime();
bool CTaskCfg::getFileName(UINT8 *pfilename)
{
    UINT32 ulBtsID = bspGetBtsID();
    time_t tm = GetSystemTime();
    struct tm *time = ::localtime(&tm);
    pfilename += sprintf((char*)pfilename,"%d_", ulBtsID);
    ::strftime((char*)pfilename, 200, "%Y%m%d%H%M%S", time);
    return true;
}


UINT8* CTaskCfg::getFullPathFileName(UINT8 *pFullName, UINT8 *filename)
{
    sprintf((char*)pFullName, "%s%s", RAMDISK_CALIBRATION_DIR, filename);
    return (pFullName);
}


#include "L3DataSocketMsgs.h"
bool CTaskCfg::Write2FileAndNotifyToFTP(char *filename)
{
#pragma pack(1)
typedef struct _tag_msgHdr
{
    UINT16 msgArea;
    EmsMsgHeader hdr;
    UINT16       usTransID;
} stCalibrationFileHdr;
#pragma pack()

    if (NULL == filename)
        {
        return false;
        }

    FILE *stream;
    DIR  *pDir = opendir(RAMDISK_CALIBRATION_DIR);
    if (NULL == pDir)
        {
         OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]opendir!!!");
        return false;
        }
    closedir(pDir);

    UINT32 ulBtsId = bspGetBtsID();
    stCalibrationFileHdr GeneralHdr;
    GeneralHdr.msgArea      = 0x0001;
    GeneralHdr.hdr.btsid    = htonl(ulBtsId);
    GeneralHdr.hdr.ma       = htons(0x0000);
    GeneralHdr.hdr.moc      = htons(0x0098);
    GeneralHdr.hdr.action   = htons(0xFF00);
    GeneralHdr.usTransID    = 0xFFFF;

    stCalibrationFileHdr DataHdr;
    DataHdr.msgArea         = 0x0001;
    DataHdr.hdr.btsid       = htonl(ulBtsId);
    DataHdr.hdr.ma          = htons(0x0000);
    DataHdr.hdr.moc         = htons(0x0099);
    DataHdr.hdr.action      = htons(0xFF00);
    DataHdr.usTransID       = 0xFFFF;

    UINT32 ulGenLen = sizeof(pCalRstDataRcd->GenResultNotify) - sizeof(pCalRstDataRcd->GenResultNotify.CalibrationError.calCorrFlag);   //不上送校准结果,否则ems不认识校准结果
    UINT32 ulLen = ulGenLen + sizeof(pCalRstDataRcd->NotifyData)
                    + (CALIBRATION_DATA_NUM + 1)*sizeof(stCalibrationFileHdr);

    char *pSrcData = new char[ulLen+1];
    if (NULL == pSrcData)
        {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]Not enough memory!!!");
        return false;
        }

    char *ptr = pSrcData;
    //填充calibration gen hdr.
    memcpy(ptr, &GeneralHdr, sizeof(GeneralHdr));
    ptr += sizeof(GeneralHdr);
    memcpy(ptr, &(pCalRstDataRcd->GenResultNotify), ulGenLen);
    ptr += ulGenLen;
    //填充calibration Data.
    for (UINT8 i = 0; i < CALIBRATION_DATA_NUM; ++i)
        {
        memcpy(ptr, &DataHdr, sizeof(DataHdr));
        ptr += sizeof(DataHdr);
        memcpy(ptr, &(pCalRstDataRcd->NotifyData[i]), sizeof(T_CaliResultNotifyEle));
        ptr += sizeof(T_CaliResultNotifyEle);
        }
#ifndef WBBU_CODE
    UINT8 FullPathFilename[200] = {0};
    getFullPathFileName(FullPathFilename, (UINT8*)filename);
#else
    char  FullPathFilename[200] = {0};
    //strcpy(FullPathFilename, "/RAMD:0/");
    strcat(FullPathFilename, RAMDISK_CAL_PATH);
    strcat(FullPathFilename, filename);
#endif
    stream = fopen((const char*)FullPathFilename, "ab");
    if (NULL == stream)
        {
        delete []pSrcData;
         OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]fopen!!!");
        return false;
        }

    UINT32 num = fwrite((char*)pSrcData, sizeof( UINT8 ), ulLen, stream);
    char disp[200];
    sprintf(disp, "[tCfg] %d Bytes write to calibration file[%s]", num, (int)filename);
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, disp);
    fclose(stream);
    delete []pSrcData;

 if (false == NotifyToFTP((UINT8*)filename))
    {
        sprintf(disp, "[tCfg] WARNING!!!Fail to Notify tFtpC to transfer calibration file %s! Delete file now.", (int)filename);
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, disp);
        //delete file
        if (ERROR == remove((const char*)FullPathFilename))
        {
            sprintf(disp, "[tCfg]WARNING!!!Delete calibration file %s failed...", (int)FullPathFilename);
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, disp);
        }
        return false;
    }
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] FTP calibration file to EMS.");

#if 0
    //start FTP session.
    FTPCalibrationFile(filename);

    //delete file
    if (ERROR == remove((const char*)FullPathFilename))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]WARNING!!!Remove calibration file %s failed...", (UINT8)FullPathFilename);
        }
#endif
    return true;
}

#include "L3oamFile.h"
bool CTaskCfg::NotifyToFTP(UINT8 *filename)
{
    if (NULL == filename)
        {
        return false;
        }

    UINT32 ulDataLen = strlen((char*)filename) + 1;
    CComMessage *pComMsg = new(this, ulDataLen )CComMessage; 
    if (NULL == pComMsg)
        {
        return false;
        }

    pComMsg->SetDstTid(M_TID_FTPCLIENT);
    pComMsg->SetMessageId(M_OAM_CFG_FTP_TRANSFER_FILE_REQ);
    pComMsg->SetSrcTid( this->GetEntityId());
    UINT8 *pDataPtr = (UINT8*)pComMsg->GetDataPtr();
    memcpy( pDataPtr, filename, ulDataLen );
    if (false == CComEntity::PostEntityMessage(pComMsg))
        {
        pComMsg->Destroy();
        return false;
        }
    //启动定时器保护ftp任务jy081121
    CTaskFileMag *taskSM = CTaskFileMag::GetInstance();
    taskSM->setFtpUsingFlag(M_TID_CM, TRUE);
    return true;
}


bool CTaskCfg :: CM_CommonReqHandle(CMessage &ReqMsg, 
                                    TID DstTid, 
                                    UINT16 ReqMsgID,
                                    UINT16 FailRspMsgID
                                    )
{
    ReqMsg.SetDstTid(DstTid);
    ReqMsg.SetSrcTid(M_TID_CM);
    ReqMsg.SetMessageId(ReqMsgID);

    //构造配置失败消息       
    CL3OamCommonRsp CfgFailRsp;
    if (false == CfgFailRsp.CreateMessage(*this))
        return false;
    CfgFailRsp.SetMessageId(FailRspMsgID);
    CfgFailRsp.SetDstTid(M_TID_EMSAGENTTX);
    CfgFailRsp.SetResult(OAM_TIMEOUT_ERR);

    UINT16 TransId = ReqMsg.GetTransactionId();
    CfgFailRsp.SetTransactionId(TransId);
    //创建配置数据transaction
    CTransaction* pCfgTransaction = CreateTransact(ReqMsg, 
                                    CfgFailRsp, 
                                    OAM_REQ_RESEND_CNT3, 
                                    OAM_REQ_RSP_PERIOD);
    if (NULL == pCfgTransaction)
    {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Create transaction failed!");
        ReqMsg.DeleteMessage();//如果失败释放处理jiaying20100811
        CfgFailRsp.DeleteMessage();
        return false;
    }

    UINT16 OamTransId = pCfgTransaction->GetId();
    ReqMsg.SetTransactionId(OamTransId);
    if(false == pCfgTransaction->BeginTransact())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Send messsage[0x%04x] to Task[%d] failed! begin transaction failed!", ReqMsg.GetMessageId(), ReqMsg.GetDstTid());
        pCfgTransaction->EndTransact();//如果失败释放处理jiaying20100811
        delete pCfgTransaction;
        return false;
    }

    return true;
}


bool CTaskCfg :: CM_CommonReqNoRspHandle(CMessage& ReqMsg, 
                                         UINT16 RspMsgId, 
                                         SINT8* DstAddr, 
                                         UINT16 DataLen,
                                         UINT16 DataOff)
{
    CM_PostCommonRsp(M_TID_EMSAGENTTX, ReqMsg.GetTransactionId(), RspMsgId, OAM_SUCCESS);

    //设置nvram
   UINT16 msg_length = ReqMsg.GetDataLength();//wangwenhua add 20090207
   
   if((msg_length==0))
   {
   		return false;
   }
    l3oambspNvRamWrite(DstAddr, (SINT8*)ReqMsg.GetDataPtr() + DataOff, DataLen);

    return true;
}

bool CTaskCfg::CM_CommonRspHandle(CMessage& rMsg, 
                                    UINT16 RspMsgId,// 
                                    SINT8* DstAddr, 
                                    UINT16 DataLen,
                                    UINT16 DataOff)
{
    CL3OamCommonRsp RspMsg(rMsg);
    UINT16 usResult = RspMsg.GetResult();
    if(OAM_SUCCESS != usResult)
    {
        if(OAM_CFG_INIT_FROM_EMS == m_SysStatus)
        {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] WARNING!!!Receive fail response. SrcTid[%d] MsgId[0x%04x]", rMsg.GetSrcTid(), rMsg.GetMessageId());
        //SetSysStatus(OAM_CFG_INIT_FAIL); 
        }
    }
    
    CTransaction * pTransaction = FindTransact(RspMsg.GetTransactionId());
    if(!pTransaction)
    {
        if (M_BTS_EMS_INSTANT_CALIBRATION_RSP != RspMsgId)
            OAM_LOGSTR3(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Unexpected config response message.  can not find its TransId[%x]! SrcTid[%d] MsgId[0x%04x]", RspMsg.GetTransactionId(), rMsg.GetSrcTid(), RspMsgId);
        return false;
    }
    else
    {
    	UINT16 transIDfromEMS = pTransaction->GetRequestTransId();
		if (OAM_DEFAUIT_TRANSID != transIDfromEMS) CM_PostCommonRsp(M_TID_EMSAGENTTX, transIDfromEMS, RspMsgId, usResult);
        pTransaction->EndTransact();
    }

    if (NULL == DstAddr)
        {
        //dont need save to NvRam.
        delete pTransaction;
        return true;
        }

    if(OAM_SUCCESS == usResult) 
    {  
        if(M_L2_L3_L1_GENERAL_SETTING_RSP == rMsg.GetMessageId())
            {
            l3oambspNvRamWrite(DstAddr, (SINT8*)&m_gL1Cfg, sizeof(m_gL1Cfg));
            } 
		//ems ranging cfg
        else if(MSGID_L2_L3_REMOTE_RANGE_RSP == rMsg.GetMessageId())
        {
            T_RangingPara tmpstruct;
            memcpy((SINT8*)&tmpstruct, (SINT8*)((pTransaction->GetRequestMessage()).GetDataPtr()) + DataOff, DataLen);
            tmpstruct.cfg_flag= 0x5a5a;
            l3oambspNvRamWrite((char*) DstAddr, (char*)&tmpstruct, sizeof(T_RangingPara));
        }
	
        else
            {
            l3oambspNvRamWrite(DstAddr, (SINT8*)((pTransaction->GetRequestMessage()).GetDataPtr()) + DataOff, DataLen);
            }
    }
    delete pTransaction;
    return true;
}
#if 0
bool CTaskCfg :: CM_CommonRspNoSaveHandle(CMessage& rMsg, UINT16 RspMsgId)
{
    CL3OamCommonRsp RspMsg(rMsg);
    UINT16 usResult = RspMsg.GetResult();

    if(OAM_SUCCESS != usResult)
    {
        if(OAM_CFG_INIT_FROM_EMS == m_SysStatus)
        {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Receive fail response. SrcTid[%d] MsgId[0x%04x]", rMsg.GetSrcTid(), rMsg.GetMessageId());
        //SetSysStatus(OAM_CFG_INIT_FAIL); 
        }
    }
    
    UINT16 TransID = RspMsg.GetTransactionId();
    CTransaction * pTransaction = FindTransact(TransID);
    if(!pTransaction)
    { 
        if(OAM_DEFAUIT_TRANSID != TransID)
        {   
            OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Unexpected response message.  can not find its TransId! SrcTid[%d] MsgId[0x%04x]", rMsg.GetSrcTid(), RspMsgId);
        }
        return false;
    }
    else
    {
        CM_PostCommonRsp(M_TID_EMSAGENTTX, pTransaction->GetRequestTransId(), RspMsgId, usResult);
        pTransaction->EndTransact();
    }

    delete pTransaction;
                
    return true;
}
#endif

#if 0
bool CTaskCfg :: CM_SendAlmNotifyToOam (UINT8   Flag,
                                        UINT16  EntityType,
                                        UINT16  EntityIndex,        
                                        UINT16  AlarmCode,      
                                        UINT16  Severity, 
                                        UINT8   InfoLen,    
                                        SINT8   *pAlarmInfo)
{
    CAlarmNotifyOam  Notify;
    Notify.CreateMessage(*this);
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetDstTid(M_TID_FM);

    Notify.SetSequenceNum(0XFFFFFFFF);
    T_TimeDate TD = bspGetDateTime();
    Notify.SetFlag(Flag); 
    Notify.SetYear(TD.year);
    Notify.SetMonth(TD.month);
    Notify.SetDay(TD.day);
    Notify.SetHour(TD.hour);
    Notify.SetMinute(TD.minute);
    Notify.SetSecond(TD.second);
    
    //对此告警   0-标识告警链路正常,  1-标识链路通信断
    Notify.SetEntityType(EntityType);
    Notify.SetEntityIndex(EntityIndex);
    Notify.SetAlarmCode(AlarmCode);

    //告警级别等信息可由告警信息表中查询得到
    Notify.SetSeverity(Severity);
    Notify.SetInfoLen(InfoLen);
    Notify.SetAlarmInfo(pAlarmInfo);
    if(true != Notify.Post())
    {
        OAM_LOGSTR(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post Alarm Notify to tFM fail");
        Notify.DeleteMessage();
        return false;
    }

    return true;
}
#endif

bool CTaskCfg :: CM_InitSendCfgReq(CMessage &CfgReq)
{
    //构造配置失败消息       
    CCfgInitFailNotify FailNotify;
    FailNotify.CreateMessage(*this);
    FailNotify.SetDstTid(M_TID_CM);

    //创建配置数据transaction,发配置请求消息
    CTransaction* pCfgTransaction = CreateTransact(CfgReq, FailNotify, OAM_REQ_RESEND_CNT3, OAM_REQ_RSP_PERIOD);
    if ( NULL == pCfgTransaction )//如果失败释放处理jiaying20100811
    {        
        CfgReq.DeleteMessage();
        FailNotify.DeleteMessage();
        return false;
    }   
    UINT16 TransID = pCfgTransaction->GetId();
    CfgReq.SetTransactionId(TransID);
    FailNotify.SetTransactionId(TransID);
    FailNotify.SetResult(L3CM_ERROR_TIMER_OUT);
    FailNotify.SetFailMsgID(CfgReq.GetMessageId());

    if(false == pCfgTransaction->BeginTransact())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]Send msg = 0x%04x to task %d failed! can not create transaction!", CfgReq.GetMessageId(), CfgReq.GetDstTid());
        pCfgTransaction->EndTransact();//如果失败释放处理jiaying20100811
        delete pCfgTransaction;
        return false;
    }

    return true;
}

#if 0
void CTaskCfg :: CM_UNITTEST_L2SendRsp(TID SrcTid, UINT16 MsgId, UINT16 TranId)
{
    CL3OamCommonRsp L3OamCommonRsp;
    L3OamCommonRsp.CreateMessage(*this);
    L3OamCommonRsp.SetSrcTid(SrcTid);  
    L3OamCommonRsp.SetDstTid(M_TID_CM);  
    L3OamCommonRsp.SetMessageId(MsgId);
    L3OamCommonRsp.SetTransactionId(TranId);
    L3OamCommonRsp.SetResult(0);;
    L3OamCommonRsp.Post();
    if(true != L3OamCommonRsp.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", L3OamCommonRsp.GetMessageId());
        L3OamCommonRsp.DeleteMessage();
    }
}

void CTaskCfg :: CM_UNITTEST_L3RspEms(UINT16 MsgId, UINT16 TransId)
{
    CL3OamCommonRsp L3OamCommonRsp;
    L3OamCommonRsp.CreateMessage(*this);
    L3OamCommonRsp.SetSrcTid(M_TID_CM);  
    L3OamCommonRsp.SetDstTid(M_TID_EMSAGENTTX);  
    L3OamCommonRsp.SetMessageId(MsgId);
    L3OamCommonRsp.SetTransactionId(TransId);
    L3OamCommonRsp.SetResult(0);;
    L3OamCommonRsp.Post();
    if(true != L3OamCommonRsp.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", L3OamCommonRsp.GetMessageId());
        L3OamCommonRsp.DeleteMessage();
    }
}
#endif
bool CTaskCfg :: CM_SendNotifyMsg( TID tid, UINT16 msgid)
{
    CL3OamCommonReq Notify;
    Notify.CreateMessage(*this);
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetDstTid(tid);
    Notify.SetSrcTid(M_TID_CM);
    Notify.SetMessageId(msgid);
    if(true != Notify.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", Notify.GetMessageId());
        Notify.DeleteMessage();
    }
    return true;
}

bool CTaskCfg :: CM_InitFEPData()
{
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] Begin init FEP data...");

    {
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->CaliGenCfgEle);
    CCfgCalGenDataReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(M_L3_L2_CALIBRAT_CFG_GENDATA_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);  
    CfgReq.SetEle(pCfgEle, sizeof(T_CaliGenCfgEle));
    CM_InitSendCfgReq(CfgReq);
    #ifndef __WIN32_SIM__
    taskDelay(5);
    #else
    Sleep(5);
    #endif
    }

    {   
    for(int index = 0; index < L1_CAL_DATA_CNTS; index++)
    {
        SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->CaliDataEle[index]);
        CCfgCalDataReq CfgReq;
        CfgReq.CreateMessage(*this);
        CfgReq.SetMessageId(M_L3_L2_CALIBRAT_CFG_DATA_REQ);
        CfgReq.SetDstTid(M_TID_L2MAIN);  
        CfgReq.SetEle(pCfgEle, sizeof(T_CaliDataEle));
        CM_InitSendCfgReq(CfgReq);
        #ifndef __WIN32_SIM__
        taskDelay(5);
        #endif
    }
    }
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] Init FEP data finished.");
    return true;
}

bool CTaskCfg :: CM_InitAUXData()
{
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] Begin init AUX data...");
    {
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->RfCfgEle);
    CCfgRfReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(M_L3_L2_RF_CFG_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);  
    CfgReq.SetEle(pCfgEle, sizeof(T_RfCfgEle));
    CM_InitSendCfgReq(CfgReq);
    }

    {
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->CaliGenCfgEle);
    CCfgCalGenDataReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(M_L3_L2_CALIBRAT_CFG_GENDATA_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);  
    CfgReq.SetEle(pCfgEle, sizeof(T_CaliGenCfgEle));
    CM_InitSendCfgReq(CfgReq);
    #ifndef __WIN32_SIM__
    taskDelay(5);
    #else
    Sleep(5);
    #endif
    }
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] Init AUX data finished.");
    return true;
}

#ifndef _INC_L3_BTS_PM
#include "L3_BTS_PM.h"
#endif
bool CTaskCfg :: CM_InitL2Data()
{
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] Begin init L2PPC data...");
    {
		    UINT16 Cnt = NvRamDataAddr->BTSCommonDataEle.BtsRstCnt;
		    Cnt = Cnt + 1;
		    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->BTSCommonDataEle.BtsRstCnt), (SINT8*)&Cnt, 2);
    		OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] reboot counter=[%d].", Cnt );
    CM_L2AirLinkCfg();
    }
     {
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->N_parameter);
            CL3L2CfgN1ParameterReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_N1_PARAMETER_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);
            CfgReq.SetEle(pCfgEle);
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config PerfLog FTP server OK, now config N1 PARAMETE...");
            
        }        
#ifndef WBBU_CODE       
        {  
            SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->TempAlarmCfgEle);
            CCfgTempAlarmReq CfgReq;
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(M_L3_L2_TEMP_MONITOR_REQ);
            CfgReq.SetDstTid(M_TID_L2MAIN);
            CfgReq.SetEle(pCfgEle, sizeof(T_TempAlarmCfgEle));
            CM_InitSendCfgReq(CfgReq);
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Config N1 PARAMETE OK, now config Temperature Alarm...");
            
        }    
#endif
    {
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->RMPoliceEle);
    CCfgRMPolicyReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(M_L3_L2_RES_MANAGE_POLICY_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);  
    CfgReq.SetEle(pCfgEle, sizeof(T_RMPoliceEle));
    CM_InitSendCfgReq(CfgReq);
    }

    {
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->AirLinkMisCfgEle);
    CCfgAirLinkMiscReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(M_L3_L2_AIR_LINK_MISC_CFG_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);  
    CfgReq.SetEle(pCfgEle, sizeof(T_AirLinkMisCfgEle));
    CM_InitSendCfgReq(CfgReq);
    }

    CM_L2BillDataAndDiagUserCfg();

    {
    T_L1GenCfgEle *pCfgEle = (T_L1GenCfgEle *)&(NvRamDataAddr->L1GenCfgEle);
    CCfgL1GenDataReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(M_L3_L2_L1_GENERAL_SETTING_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);  
    CfgReq.SetSyncSrc(pCfgEle->SyncSrc);
    CfgReq.SetGPSOffset(pCfgEle->GpsOffset);
    CfgReq.SetAntennaMask(pCfgEle->AntennaMask);
    CM_InitSendCfgReq(CfgReq);
    }

    //ems ranging cfg
     {
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->RangingPara);
    if(NvRamDataAddr->RangingPara.cfg_flag!=0x5a5a)
    {
        T_RangingPara ranging_para_temp;
        ranging_para_temp.Ranging_Switch = 2;
        ranging_para_temp.Enable_Shreashold = 40;
        ranging_para_temp.Disable_Shreahold = 35;
        ranging_para_temp.Ratio_Shreahold = 80;
        ranging_para_temp.SNR_Shreahold = 10;
        ranging_para_temp.Ranging_Offset_Shreahold = 46;
        ranging_para_temp.cfg_flag = 0x5a5a;
        l3oambspNvRamWrite(pCfgEle, (SINT8*)&ranging_para_temp, sizeof(T_RangingPara));
    }   
    CCfgRRangingReq CfgReq;      
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(MSGID_L3_L2_REMOTE_RANGE_CFG);
    CfgReq.SetDstTid(M_TID_L2MAIN);
    CfgReq.SetEle(pCfgEle, sizeof(T_RangingPara)-2);
    CM_InitSendCfgReq(CfgReq);     
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "\n now config remote ranging para....... \n");
    
    }
	{
		OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "\n [CFG]CM_InitL2Data SETVACPREFSCG[%04X]\n", NvRamDataAddr->ucVacPrefScg );
		if( 1==NvRamDataAddr->ucVacPrefScg || 0==NvRamDataAddr->ucVacPrefScg )
		{
			CCfgVacPrefScgReq CfgReq;
		    CfgReq.CreateMessage(*this);
		    CfgReq.SetMessageId(MSGID_L3_L2_VACPREFSCG_CFG);
		    CfgReq.SetDstTid(M_TID_L2MAIN);
		    CfgReq.SetValue( NvRamDataAddr->ucVacPrefScg );
		    CM_InitSendCfgReq(CfgReq);     
		    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "\n now send vac prefscg to l2....... \n");
		}
	}
	CM_Qam64();
    //lijinan 20110331 增加负载均衡参数
    m_bPldBlnCfgOK = true;
    memcpy(&m_stPldBlnCfg, &NvRamDataAddr->PayloadCfg.usFlag,sizeof(m_stPldBlnCfg));
    {
		if( m_bPldBlnCfgOK )
        {
        	CCfgPayloadReq CfgReq;
		CfgReq.CreateMessage(*this);
 	       CfgReq.SetMessageId(MSGID_L3_L2_PAYLOAD_BALANCE_CFG);
 	       CfgReq.SetDstTid(M_TID_L2OAM);
 	       CfgReq.SetValue((UINT8*)&m_stPldBlnCfg);
 	       CM_InitSendCfgReq(CfgReq);    
		
    		// CM_SendComMsg( M_TID_L2OAM, MSGID_L3_L2_PAYLOAD_BALANCE_CFG, (UINT8*)&m_stPldBlnCfg, sizeof(m_stPldBlnCfg) );
            OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]m_bSavePowerModel=true");
        }
	   else
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg]m_bSavePowerModel=false");
    }
   
    {
        CTaskPM *tpmt = CTaskPM::GetInstance();
       // if( tpmt->m_bRTMonitorAct )
        {   
             tpmt->InitRTMonitor();
             tpmt->PM_StartRTMonitor();
/*         UINT16 usReq[3];
            usReq[0] = 0x80E9;
            usReq[1] = 0x0;
            usReq[2] = tpmt->m_usRTMonitorTmLen;
            tpmt->InitRTMonitor();
    		CM_SendComMsg( M_TID_PM, M_EMS_BTS_BTS_RTMONITOR_REQ, (UINT8*)usReq, 6 );
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] start pm realtime monitor.");*/
        }
    }
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] Init L2PPC data finished.");

    CM_InitAUXData();           
    CM_InitFEPData();
#ifdef M_TGT_WANIF
    CM_ConfigWanCPE();
#endif

    neighborListRefresh();

    {
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->ClusterPara);
	     if(NvRamDataAddr->ClusterPara.cfg_flag!=0x5a5a)
	     {
	         T_CLUSTER_PARA cluster_para_temp;
		  cluster_para_temp.flag = 1;
		  cluster_para_temp.sleep_period = 32;
		  cluster_para_temp.Rsv_Ch_Resourse_Num = 2;
		  cluster_para_temp.rsv1 = 0;
		  cluster_para_temp.rsv2 = 0;
		  cluster_para_temp.rsv3 = 0;
		  cluster_para_temp.cfg_flag = 0x5a5a;
		  l3oambspNvRamWrite(pCfgEle, (SINT8*)&cluster_para_temp, sizeof(T_CLUSTER_PARA));
	     }	
	     CCfgClusterParaReq CfgReq;	     
            CfgReq.CreateMessage(*this);
            CfgReq.SetMessageId(MSGID_CLUSTER_L2L3_PARA_CFG);
            CfgReq.SetDstTid(M_TID_L2MAIN);
            CfgReq.SetEle(pCfgEle, sizeof(T_CLUSTER_PARA)-2);
	     CM_InitSendCfgReq(CfgReq);   
	     OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] now config cluster para...");    
    }
    sendGrpLmtPara();
    return true;
}

void  CTaskCfg :: CM_PostCommonRsp(TID tid, UINT16 transid, UINT16 msgid, UINT16 result)
{
    CL3OamCommonRsp CommonRsp;
    CommonRsp.CreateMessage(*this);
    CommonRsp.SetDstTid(tid);
    CommonRsp.SetTransactionId(transid);
    CommonRsp.SetMessageId(msgid);
    CommonRsp.SetResult(result);
    if(true != CommonRsp.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", CommonRsp.GetMessageId());
        CommonRsp.DeleteMessage();
    }
}

void CTaskCfg :: CM_GetMsgHandle(SINT8* pdata, UINT32 msglen, UINT16 rspmsgid, UINT16 transid)
{
   if(pdata ==NULL)
   	return;
    CL3OamCommonRsp CommonRsp;
    CommonRsp.CreateMessage(*m_Instance, msglen + 4);   // 4 = transid + result
    CommonRsp.SetMessageId(rspmsgid);
    CommonRsp.SetDstTid(M_TID_EMSAGENTTX);
    CommonRsp.SetTransactionId(transid);
    CommonRsp.SetResult(OAM_SUCCESS);
    UINT8 *pdstdata = (UINT8*)CommonRsp.GetDataPtr() + 4;
    memcpy(pdstdata, pdata, msglen);

    if(true != CommonRsp.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", CommonRsp.GetMessageId());
        CommonRsp.DeleteMessage();
    }
}
#ifndef WBBU_CODE
extern "C" STATUS exitTelnetSession();
#endif
extern "C" STATUS stopL2Shell();

void CTaskCfg::CM_BtsTelnetUserCfgReq(CMessage &rMsg)
{
#pragma pack(1)
    typedef struct
    {
        UINT16          TransId;
        T_BTSUserCfgEle UsrPwd;
    }T_UsrPwd;
#pragma pack()
    T_UsrPwd *pUserCfg  = (T_UsrPwd *)rMsg.GetDataPtr();
    SINT8 *pCfgUser     = pUserCfg->UsrPwd.user;
    SINT8 *pCfgPassword = pUserCfg->UsrPwd.password;

    T_BTSUserCfgEle *pNvUser = &(NvRamDataAddr->BTSUserCfgEle);
    SINT8 *pExistUser   = pNvUser->user;
    SINT8 *pExistPwd    = pNvUser->password;

    if (0 == strlen(pCfgUser))
        {
        if ( 0 != strlen(pExistUser) )
            {
            if (OK == loginUserDelete(pExistUser, pExistPwd))
                {
                OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] The username[%s] delete SUCCESS. ", (int)pExistUser);
                }
            }

        T_BTSUserCfgEle tmp;
        memset(&tmp, 0, sizeof(tmp));
        l3oambspNvRamWrite((char*)pNvUser, (char*)&tmp, sizeof(tmp));

        //通知l2用户名密码已被删除
        CM_L2BillDataAndDiagUserCfg();

        CM_PostCommonRsp(M_TID_EMSAGENTTX, pUserCfg->TransId, M_BTS_EMS_TELNET_USER_CFG_RSP, OAM_SUCCESS);
        return;
        }
    if (strlen(pCfgUser) > 40)
        {
        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Username[%s] is too long(must < 40)", (int)pCfgUser);
        CM_PostCommonRsp(M_TID_EMSAGENTTX, pUserCfg->TransId, M_BTS_EMS_TELNET_USER_CFG_RSP, OAM_FAILURE);
        return;
        }

    UINT32 state = OAM_SUCCESS;
    UINT32 len = strlen(pCfgPassword);
    if ((len < 8) || (len > 40))
        {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Password length error(8 < must < 40)");
        state = OAM_FAILURE;
        }
    //配置了一个不同的用户名
    //把系统原有的用户删除，然后新增配置的用户
    if ((state == OAM_SUCCESS) && (0 != strlen(pExistUser)))
        {
        if (OK == loginUserDelete(pExistUser, pExistPwd))
            {
            OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] The username[%s] delete SUCCESS. ", (int)pExistUser);
            }
        else
            {
//            OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] The username[%s] delete FAIL. ", (int)pExistUser);
//            state = OAM_FAILURE;
            }
        }

    //采用默认的加密算法
    if ((state == OAM_SUCCESS) && (ERROR == loginDefaultEncrypt(pCfgPassword, s_aucEncryptedPwd)))
        {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Password encrypt FAIL.");
        state = OAM_FAILURE;
        }
    //加入到用户表
    if ((state == OAM_SUCCESS) && (ERROR == loginUserAdd(pCfgUser, s_aucEncryptedPwd)))
        {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] Add to login table fail, check if the user is already exist.");
        state = OAM_FAILURE;
        }

    //回应EMS
    CM_PostCommonRsp(M_TID_EMSAGENTTX, pUserCfg->TransId, M_BTS_EMS_TELNET_USER_CFG_RSP, state);
    if (state == OAM_SUCCESS)
        {
        //配置成功,写入NvRAM.
        OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS user[%s] config success.", (int)pCfgUser);
        l3oambspNvRamWrite((char*)pNvUser, (char*)&(pUserCfg->UsrPwd), sizeof(pUserCfg->UsrPwd));

        //lijinan 20080904 add
	  memcpy(&gstNvUser, &(NvRamDataAddr->BTSUserCfgEle), sizeof(T_BTSUserCfgEle));

        //把用户名密码通知l2.
        CM_L2BillDataAndDiagUserCfg();

	 //关闭二层Telnet
	 stopL2Shell();
        }
    else
        {
        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] BTS user[%s] config FAIL.", (int)pCfgUser);
        }

    if ( OAM_CFG_NORMAL == m_SysStatus) 
    {  // stop the telnet session in normal working mode

#ifndef WBBU_CODE
        exitTelnetSession();   
#else
   //     logout();//wangwenhua mark  2012-5-14
#endif
    }
    return;
}
bool CTaskCfg::CM_BtsTelnetUserGetReq(CMessage &rMsg)
{
    UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
    SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->BTSUserCfgEle);
    CM_GetMsgHandle(srcAddr, sizeof(T_BTSUserCfgEle), M_BTS_EMS_TELNET_USER_GET_RSP, Transid);
    return true;
}

bool CTaskCfg :: CM_BtsNeighborListHandoffCfgReq(CMessage &rMsg) //M_EMS_BTS_NEIGHBOR_LIST_HANDOFF_CFG_REQ
{
    CCfgBtsNeibListReq  ReqMsg(rMsg);
    CM_PostCommonRsp(M_TID_EMSAGENTTX, ReqMsg.GetTransactionId(), M_BTS_EMS_NEIGHBOUR_BTS_LOADINFO_CFG_RSP, OAM_SUCCESS);

    UINT16 MsgLen = ReqMsg.GetRealDataLen();
        
    //设置nvram
    l3oambspNvRamWrite((char*)&(NvRamDataAddr->BtsNeighborCfgEle), (SINT8*)ReqMsg.GetDataPtr() + 2, MsgLen -2);
    //neighborListRefresh();rrm取消了给l2下发 jy080716
    return true;
}

bool CTaskCfg :: CM_BtsNeighborListHandoffGetReq(CMessage &rMsg) //M_EMS_BTS_NEIGHBOR_LIST_HANDOFF_GET_REQ
{
    UINT16 BtsNum = NvRamDataAddr->BtsNeighborCfgEle.NeighborBtsNum;
    if(BtsNum > NEIGHBOR_BTS_NUM)
    {
        BtsNum = NEIGHBOR_BTS_NUM;
    }

    CCfgBtsNeibListReq  ReqMsg;
    UINT16 MsgLen = ReqMsg.GetDataLenFromNvram();
    
    UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
    SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->BtsNeighborCfgEle);
    CM_GetMsgHandle(srcAddr, MsgLen, M_BTS_EMS_NEIGHBOR_LIST_HANDOFF_GET_RSP, Transid);

    return true;
}
/* 相关修改说明jy080717
1.增加存储内容 NeighberListForCommon ，格式和旧的 neighberlist一样。
    2.对原来的来自ems的NeighberlistForHandover的处理修改为只存储，不再做“同频”的筛选和转发。
    3.增加来自ems的新消息 NeighberListForCommon 的处理，做“同频”筛选后存储本地(与旧的存储机制相同)，并转发到L2，（到L2的消息不变）
    4.当L2 或者 L3 重启的时候，必须把 NeighberListForCommon(替代以前的NeighberlistForHandover) 的内容发送到 L2，（到L2的消息不变）
*/
bool CTaskCfg :: CM_BtsNeighborListCommonCfgReq(CMessage &rMsg) //M_EMS_BTS_NEIGHBOR_LIST_COMMON_CFG_REQ
{
    CCfgBtsNeibListCommReq  ReqMsg(rMsg);
    CM_PostCommonRsp(M_TID_EMSAGENTTX, ReqMsg.GetTransactionId(), M_BTS_EMS_NEIGHBOR_LIST_COMMON_CFG_RSP, OAM_SUCCESS);

    UINT16 MsgLen = ReqMsg.GetRealDataLen();
        
    //设置nvram
    l3oambspNvRamWrite((char*)&(NvRamDataAddr->BtsNeighborCommCfgEle), (SINT8*)ReqMsg.GetDataPtr() + 2, MsgLen -2);
    neighborListRefresh();
    return true;
}
bool CTaskCfg :: CM_BtsRepeaterCfgReq(CMessage &rMsg)
{
    CL3OamCfgBtsRepeaterReq ReqMsg(rMsg);
    CM_PostCommonRsp(M_TID_EMSAGENTTX, ReqMsg.GetTransactionId(), M_BTS_EMS_BTS_REPEATER_CFG_RSP, OAM_SUCCESS);

    UINT16 MsgLen = ReqMsg.GetRealDataLen();   //消息净荷，不含transid
        
    //设置nvram
    l3oambspNvRamWrite((char*)&(NvRamDataAddr->BTSRepeaterEle), (SINT8*)ReqMsg.GetDataPtr() + 2, MsgLen);

    return true;
}

bool CTaskCfg :: CM_BtsRepeaterGetReq(CMessage &rMsg)
{
    CL3OamCfgBtsRepeaterReq ReqMsg;

    UINT16 MsgLen = ReqMsg.GetDataLenFromNvram();
    UINT16 Transid = *(UINT16*)(rMsg.GetDataPtr());
    SINT8* srcAddr = (SINT8*)&(NvRamDataAddr->BTSRepeaterEle);
    CM_GetMsgHandle(srcAddr, MsgLen, M_BTS_EMS_BTS_REPEATER_GET_RSP, Transid);

    return true;
}


bool CTaskCfg :: CM_L2AirLinkCfg()
{
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->AirLinkCfgEle);
    CL3L2CfgAirLinkReq ReqMsg;
    ReqMsg.CreateMessage(*this);
    ReqMsg.SetDstTid(M_TID_L2MAIN);
    ReqMsg.SetMessageId(M_L3_L2_AIRLINK_DATA_CFG_REQ);
    ReqMsg.SetBtsID(bspGetBtsID());
    ReqMsg.SetNetworkID(bspGetNetworkID());
    UINT32 BtsRstCnt = NvRamDataAddr->BTSCommonDataEle.BtsRstCnt;
    ReqMsg.SetResetCnt(BtsRstCnt);
    ReqMsg.SetEle(pCfgEle);   
    CM_InitSendCfgReq(ReqMsg);
     OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "tNvRam ts: = %d\n",NvRamDataAddr->AirLinkCfgEle.DLTSNum);

    return true;
}

UINT8 roundidx=1;
bool CTaskCfg :: CM_L2AirLinkCfgRandomBCH()
{
    SINT8 *pCfgEle = (SINT8 *)pDataAirLinkCfgRandom;
//	roundidx ++;
//   if (roundidx >=5) 
//     roundidx =0;
#if 0
  UINT8  SequenceID;   // 0~6 For N=1(N equals one)
    UINT8  SubCGrpMask;  //Sub-carrier group Mask   1       
    UINT8  TimeSlotNum;  // 8 for 10ms TDD     4 for 5ms TDD    
    UINT8  DLTSNum;      // 1~Total time slot number    Time slot number used for downlink
    #endif
   pDataAirLinkCfgRandom->DLTSNum = NvRamDataAddr->AirLinkCfgEle.DLTSNum;
    pDataAirLinkCfgRandom->SequenceID = NvRamDataAddr->AirLinkCfgEle.SequenceID;
    pDataAirLinkCfgRandom->TimeSlotNum = NvRamDataAddr->AirLinkCfgEle.TimeSlotNum;
   pDataAirLinkCfgRandom->SubCGrpMask = 0x07;
//   OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "CM_L2AirLinkCfgRandomBCH roundidx: = %d\n",roundidx);
//    j=0;
    
   for(int i=0;  i<8; i++) 
   {
   	
   	/*if((pDataAirLinkCfgRandom-> BCHInfo[i].BCHSCGIndex ==2)&&
        (pDataAirLinkCfgRandom-> BCHInfo[i].BCHTSIndex == 0)&&
        (pDataAirLinkCfgRandom-> RRCHInfor[i].RRCHSCGIndex ==2)&&
        (pDataAirLinkCfgRandom-> RRCHInfor[i].RRCHTSIndex==1)&&
        (pDataAirLinkCfgRandom-> RACHPairInfor[i].RACHPairSCGIndex==2)&&
        (pDataAirLinkCfgRandom-> RACHPairInfor[i].RACHPairTSIndex==2))
        {
   	         return;
   	    }
   	
   	if(pDataAirLinkCfgRandom-> BCHInfo[i].BCHSCGIndex == 0xff)
   	 break;
   	*/
    pDataAirLinkCfgRandom-> BCHInfo[i].BCHSCGIndex = 0xff;
    pDataAirLinkCfgRandom-> BCHInfo[i].BCHTSIndex = 0xff;
    pDataAirLinkCfgRandom-> RRCHInfor[i].RRCHSCGIndex = 0xff;
    pDataAirLinkCfgRandom-> RRCHInfor[i].RRCHTSIndex=0xff;
    pDataAirLinkCfgRandom-> RACHPairInfor[i].RACHPairSCGIndex=0xff;
    pDataAirLinkCfgRandom-> RACHPairInfor[i].RACHPairTSIndex=0xff;
    pDataAirLinkCfgRandom-> CHScale[i].BCHScale=0;
    pDataAirLinkCfgRandom-> CHScale[i].TCHScale=0;
   }
   
    pDataAirLinkCfgRandom-> BCHInfo[0].BCHSCGIndex = 0;
    pDataAirLinkCfgRandom-> BCHInfo[0].BCHTSIndex = 0;
    pDataAirLinkCfgRandom-> RRCHInfor[0].RRCHSCGIndex =0;
    pDataAirLinkCfgRandom-> RRCHInfor[0].RRCHTSIndex=1;
    pDataAirLinkCfgRandom-> RACHPairInfor[0].RACHPairSCGIndex=0;
    pDataAirLinkCfgRandom-> RACHPairInfor[0].RACHPairTSIndex=0;
    pDataAirLinkCfgRandom-> CHScale[0].BCHScale=0x1083;
    pDataAirLinkCfgRandom-> CHScale[0].TCHScale=0x05D6;   
   
   
    pDataAirLinkCfgRandom-> BCHInfo[1].BCHSCGIndex = 2;
    pDataAirLinkCfgRandom-> BCHInfo[1].BCHTSIndex = 0;
    pDataAirLinkCfgRandom-> RRCHInfor[1].RRCHSCGIndex =2;
    pDataAirLinkCfgRandom-> RRCHInfor[1].RRCHTSIndex=1;
    pDataAirLinkCfgRandom-> RACHPairInfor[1].RACHPairSCGIndex=2;
    pDataAirLinkCfgRandom-> RACHPairInfor[1].RACHPairTSIndex=2;
    pDataAirLinkCfgRandom-> CHScale[1].BCHScale=0x169E;
    pDataAirLinkCfgRandom-> CHScale[1].TCHScale=0x0821;
    pDataAirLinkCfgRandom-> CHScale[2].BCHScale=0x169E;
    pDataAirLinkCfgRandom-> CHScale[2].TCHScale=0x0800;
       
    OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] CM_L2AirLinkCfgRandomBCH ts:%d\n",pDataAirLinkCfgRandom->DLTSNum);
        
    CL3L2CfgAirLinkReq ReqMsg;
    ReqMsg.CreateMessage(*this);
    ReqMsg.SetDstTid(M_TID_L2MAIN);
    ReqMsg.SetMessageId(M_L3_L2_AIRLINK_DATA_CFG_REQ);
    ReqMsg.SetBtsID(bspGetBtsID());
    ReqMsg.SetNetworkID(bspGetNetworkID());
    UINT32 BtsRstCnt = NvRamDataAddr->BTSCommonDataEle.BtsRstCnt;
    ReqMsg.SetResetCnt(BtsRstCnt);
    ReqMsg.SetEle(pCfgEle);   
    CM_InitSendCfgReq(ReqMsg);


    return true;
}



 void L2AirLinkCfgRandomBCH()
{
    printf("\nCM_L2AirLinkCfgRandomBCH!!!");
    CTaskCfg::GetInstance()->CM_L2AirLinkCfgRandomBCH();
}

void L2AirLinkCfg()
{
    printf("\nCM_L2AirLinkCfg!!!");
    CTaskCfg::GetInstance()->CM_L2AirLinkCfg();
}
bool CTaskCfg::CM_L2BillDataAndDiagUserCfg()
{
    SINT8 *pCfgEle = (SINT8*)&(NvRamDataAddr->BillDataCfgEle);
    CCfgBillingDataReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(M_L3_L2_BILLING_DATA_CFG_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);  
    CfgReq.setUserPassword(NvRamDataAddr->BTSUserCfgEle.user, NvRamDataAddr->BTSUserCfgEle.password);
    CfgReq.SetEle(pCfgEle, sizeof(T_BillDataCfgEle));
	CfgReq.SetTransactionId(OAM_DEFAUIT_TRANSID);
    CM_InitSendCfgReq(CfgReq);
    return true;
}


void CTaskCfg::neighborListRefresh()
{
    CDataRefreshJammingNeighbor msg;
    if (false == msg.CreateMessage(*this))
        return;
    msg.SetDstTid(M_TID_EMSAGENTTX);
    msg.SetSrcTid(M_TID_CM);
    msg.SetFlag(0); //???
    if(false == msg.Post())
        {
        OAM_LOGSTR(LOG_CRITICAL, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post JNT refreash msg fail");
        msg.DeleteMessage();
        }  
}

void CTaskCfg::CM_HandoverParaCfg(CMessage& rMsg)
{
    CComMessage* pComMsg = new (this, 4) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_HandoverParaCfg.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_HANDOVER_PARA_CFG_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
    T_UtHandoverPara paraTemp;
    memcpy((char*)&paraTemp, (char*)((UINT8*)rMsg.GetDataPtr()+2), sizeof(T_UtHandoverPara)-2);
    paraTemp.write_flag = 0x5a5a;
    l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->UtHandoverPara), 
                               (SINT8*)&paraTemp, 
                               sizeof(T_UtHandoverPara)
                               );
#ifdef NUCLEAR_CODE
	if( rMsg.GetDataLength() > sizeof(T_UtHandoverPara) )
	{
	    T_UtHandoverPara2 paraTemp2;
	    memcpy((char*)&paraTemp2, (char*)((UINT8*)rMsg.GetDataPtr()+sizeof(T_UtHandoverPara)), sizeof(T_UtHandoverPara2)-2);
	    paraTemp2.write_flag = 0x5a5a;
	    l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->stUtHandoverPara2), 
	                               (SINT8*)&paraTemp2, 
 	                               sizeof(T_UtHandoverPara2) );		
	}
#endif
    
}
//切换算法参数配置查询
void CTaskCfg::CM_HandoverParaGet(CMessage&rMsg)
{
    CComMessage* pComMsg = new (this, 4+sizeof(T_UtHandoverPara)-2) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_HandoverParaGet.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_HANDOVER_PARA_GET_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    if(NvRamDataAddr->UtHandoverPara.write_flag == 0x5a5a)
    {
        *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;
        memcpy((UINT8*)((UINT8*)pComMsg->GetDataPtr()+4), (UINT8*)&NvRamDataAddr->UtHandoverPara, sizeof(T_UtHandoverPara)-2);
    }
   else
	*(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 1;
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
}

//ems ranging cfg
void CTaskCfg::CM_sendMsgToRpc()
{
    CComMessage* pComMsg = new (this, 2) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed.");
        return;
    }
    pComMsg->SetDstTid(M_TID_DIAGM);
    pComMsg->SetSrcTid(M_TID_CM); 
    pComMsg->SetMessageId(M_EMS_BTS_REMOTE_RANGE_CFG_REQ);
    SINT8* DstAddr = (SINT8*)&(NvRamDataAddr->RangingPara.SNR_Shreahold);
    memcpy((pComMsg->GetDataPtr()), ((UINT8*)DstAddr), 2);
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }           
}


extern "C" char cfGetCrc(char *pBuf0, UINT16 len);
void CTaskCfg::CM_IfPermitUseCfg(CMessage& rMsg)
{
    CComMessage* pComMsg = new (this, 4) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CM_IfPermitUseCfg.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_IF_PERMIT_USE_CFG_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }    
    T_NVRAM_BTS_CONFIG_PARA params;
    bspNvRamRead((char *)&params, (char*)(NVRAM_BASE_ADDR_PARA_PARAMS), sizeof(params));    
    params.permitUseWhenSagDown = (UINT32)*(UINT16*)((UINT8*)rMsg.GetDataPtr()+2);
    params.permitUseWhenSagDownPattern = NVRAM_VALID_PATTERN;
    bspNvRamWrite((char *)NVRAM_BASE_ADDR_PARA_PARAMS, (char *)&params, sizeof(params));
    /*生成crc，写入 jy080918*/   
    cfGetCrc((char*)NVRAM_BASE_ADDR_PARA_PARAMS, sizeof(params));   
    char FileName1[20] = "/ata0a/btsPara";    
    FILE *fd;  
    if ((fd = fopen (FileName1, "w+")) == (FILE *)ERROR)
    {
        printErr ("\nCannot open when write\"%s\".\n", FileName1);                
    }
    else
    {
        fwrite( (const void * )NVRAM_BASE_ADDR_PARA_PARAMS, 1, (sizeof(params)+6), fd); 
        fflush(fd);
        fclose (fd);
    }
}

void CTaskCfg::CM_IfPermitUseGet(CMessage&rMsg)
{
    CComMessage* pComMsg = new (this, 6) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_HandoverParaGet.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_IF_PERMIT_USE_GET_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;//result
    T_NVRAM_BTS_CONFIG_PARA *param;
    param = (T_NVRAM_BTS_CONFIG_PARA *)NVRAM_BASE_ADDR_PARA_PARAMS;
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+4) = (UINT16)param->permitUseWhenSagDown;
  
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
}
void l3oamprintl1data()
{
    printf("\r\nL1 GEN CFG:####################################");
    printf("\r\nSyncSrc     = %04X"
           "\r\nGpsOffset   = %04X"
           "\r\nAntennaMask = %04X",
           NvRamDataAddr->L1GenCfgEle.SyncSrc,
           NvRamDataAddr->L1GenCfgEle.GpsOffset,
           NvRamDataAddr->L1GenCfgEle.AntennaMask);
    
    printf("\r\n\r\nL1 RF CFG:####################################");
    printf("\r\nStartFreqIndex = %08X"
           "\r\nAntennaPower   = %04X"
           "\r\nRxSensitivity  = %04X"
           "\r\nCableLoss      = %04X"
           "\r\nPSLoss         = %04X",
           
           NvRamDataAddr->RfCfgEle.StartFreqIndex,
           NvRamDataAddr->RfCfgEle.AntennaPower,
           NvRamDataAddr->RfCfgEle.RxSensitivity,
           NvRamDataAddr->RfCfgEle.Cable_Loss,
           NvRamDataAddr->RfCfgEle.PS_Loss);

    printf("\r\nPS_Norm_X  = ");
    for(int i=0;  i<8; i++) printf("%04X ", (UINT16)NvRamDataAddr->RfCfgEle.PS_NormS[i].PS_Norm_X);
    printf("\r\nPS_Norm_Y  = ");
    for(int i=0;  i<8; i++) printf("%04X ", (UINT16)NvRamDataAddr->RfCfgEle.PS_NormS[i].PS_Norm_Y);


    printf("\r\n");
}
#if 0
void l3oamprintl2arilinkcfgdata(T_AirLinkCfgEle *pdata)
{
    printf("\r\nL2 AIR LINK DATA:####################################");
    printf("\r\nbtsid     = %08X"
           "\r\nnetworkid = %08X"
           "\r\nreset cnt = %04X", bspGetBtsID(), bspGetNetworkID(), NvRamDataAddr->BTSCommonDataEle.BtsRstCnt);
    printf("\r\nseqid     = %02X"
           "\r\nsubcgmask = %02X"
           "\r\ntimeslotn = %02X"
           "\r\ndltsnum   = %02X",
           pdata->SequenceID,
           pdata->SubCGrpMask,
           pdata->TimeSlotNum,
           pdata->DLTSNum);

    printf("\r\nBCHSCG    = ");
    for(int i=0;  i<10; i++) printf("%02X ", pdata->BCHInfo[i].BCHSCGIndex);
    printf("\r\nBCHTS     = ");
    for(int i=0;  i<10; i++) printf("%02X ", pdata->BCHInfo[i].BCHTSIndex);

    printf("\r\nRRCHSCG   = ");
    for(int i=0;  i<10; i++) printf("%02X ", pdata->RRCHInfor[i].RRCHSCGIndex);
    printf("\r\nRRCHTS    = ");
    for(int i=0;  i<10; i++) printf("%02X ", pdata->RRCHInfor[i].RRCHTSIndex);

    printf("\r\nRACHSCG   = ");
    for(int i=0;  i<20; i++) printf("%02X ", pdata->RACHPairInfor[i].RACHPairSCGIndex);
    printf("\r\nRACHTS    = ");
    for(int i=0;  i<20; i++) printf("%02X ", pdata->RACHPairInfor[i].RACHPairSCGIndex);

    printf("\r\nmaxscale  = %02X"
           "\r\npremscale = %02X", pdata->MaxScale, pdata->PremScale);

    printf("\r\nBCHSCALE  = ");
    for(int i=0;  i<8; i++) printf("%04X ", pdata->CHScale[i].BCHScale);
    printf("\r\nTCHSCALE  = ");
    for(int i=0;  i<8; i++) printf("%04X ", pdata->CHScale[i].TCHScale);

    printf("\r\nW0I       = ");
    for(int i=0;  i<8; i++) 
    {
        UINT16 temp = pdata->W0Info[i].WI;
        printf("%04X ", temp);
    }
    printf("\r\nW0Q       = ");
    for(int i=0;  i<8; i++)
    {
        UINT16 temp = pdata->W0Info[i].WQ;
        printf("%04X ", temp);
    }
    printf("\r\nForbid_ts_mask = ");
    for (int i=0; i<SCG_NUM; ++i)
    {
        printf("%04X ", pdata->forbid_ts_mask[i]);
    }

}
#endif

void l3oamprintl2airlinkdata()
{
    printf("\r\nL2 AIR LINK DATA:####################################");
    printf("\r\nbtsid     = %08X"
           "\r\nnetworkid = %08X"
           "\r\nreset cnt = %04X", bspGetBtsID(), bspGetNetworkID(), NvRamDataAddr->BTSCommonDataEle.BtsRstCnt);
    printf("\r\nseqid     = %02X"
           "\r\nsubcgmask = %02X"
           "\r\ntimeslotn = %02X"
           "\r\ndltsnum   = %02X",
           (UINT8)NvRamDataAddr->AirLinkCfgEle.SequenceID,
           (UINT8)NvRamDataAddr->AirLinkCfgEle.SubCGrpMask,
           (UINT8)NvRamDataAddr->AirLinkCfgEle.TimeSlotNum,
           (UINT8)NvRamDataAddr->AirLinkCfgEle.DLTSNum);

    printf("\r\nBCHSCG    = ");
    for(int i=0;  i<10; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.BCHInfo[i].BCHSCGIndex);
    printf("\r\nBCHTS     = ");
    for(int i=0;  i<10; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.BCHInfo[i].BCHTSIndex);

    printf("\r\nRRCHSCG   = ");
    for(int i=0;  i<10; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.RRCHInfor[i].RRCHSCGIndex);
    printf("\r\nRRCHTS    = ");
    for(int i=0;  i<10; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.RRCHInfor[i].RRCHTSIndex);

    printf("\r\nRACHSCG   = ");
    for(int i=0;  i<20; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.RACHPairInfor[i].RACHPairSCGIndex);
    printf("\r\nRACHTS    = ");
    for(int i=0;  i<20; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.RACHPairInfor[i].RACHPairSCGIndex);

    printf("\r\nmaxscale  = %02X"
           "\r\npremscale = %02X", NvRamDataAddr->AirLinkCfgEle.MaxScale, NvRamDataAddr->AirLinkCfgEle.PremScale);

    printf("\r\nBCHSCALE  = ");
    for(int i=0;  i<8; i++) printf("%04X ", (UINT16)NvRamDataAddr->AirLinkCfgEle.CHScale[i].BCHScale);
    printf("\r\nTCHSCALE  = ");
    for(int i=0;  i<8; i++) printf("%04X ", (UINT16)NvRamDataAddr->AirLinkCfgEle.CHScale[i].TCHScale);

    printf("\r\nW0I       = ");
    for(int i=0;  i<8; i++) 
    {
        UINT16 temp = (SINT16)NvRamDataAddr->AirLinkCfgEle.W0Info[i].WI;
        printf("%04X ", temp);
    }
    printf("\r\nW0Q       = ");
    for(int i=0;  i<8; i++)
    {
        UINT16 temp = (SINT16)NvRamDataAddr->AirLinkCfgEle.W0Info[i].WQ;
        printf("%04X ", temp);
    }
    printf("\r\nForbid_ts_mask = ");
    for (int i=0; i<SCG_NUM; ++i)
    {
        printf("%04X ", NvRamDataAddr->AirLinkCfgEle.forbid_ts_mask[i]);
    }
    
}


void l3oamprintl2data()
{
    printf("\r\nL2 AIR LINK DATA:####################################");
    printf("\r\nbtsid     = %08X"
           "\r\nnetworkid = %08X"
           "\r\nreset cnt = %04X", bspGetBtsID(), bspGetNetworkID(), NvRamDataAddr->BTSCommonDataEle.BtsRstCnt);
    printf("\r\nseqid     = %02X"
           "\r\nsubcgmask = %02X"
           "\r\ntimeslotn = %02X"
           "\r\ndltsnum   = %02X",
           (UINT8)NvRamDataAddr->AirLinkCfgEle.SequenceID,
           (UINT8)NvRamDataAddr->AirLinkCfgEle.SubCGrpMask,
           (UINT8)NvRamDataAddr->AirLinkCfgEle.TimeSlotNum,
           (UINT8)NvRamDataAddr->AirLinkCfgEle.DLTSNum);

    printf("\r\nBCHSCG    = ");
    for(int i=0;  i<10; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.BCHInfo[i].BCHSCGIndex);
    printf("\r\nBCHTS     = ");
    for(int i=0;  i<10; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.BCHInfo[i].BCHTSIndex);

    printf("\r\nRRCHSCG   = ");
    for(int i=0;  i<10; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.RRCHInfor[i].RRCHSCGIndex);
    printf("\r\nRRCHTS    = ");
    for(int i=0;  i<10; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.RRCHInfor[i].RRCHTSIndex);

    printf("\r\nRACHSCG   = ");
    for(int i=0;  i<20; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.RACHPairInfor[i].RACHPairSCGIndex);
    printf("\r\nRACHTS    = ");
    for(int i=0;  i<20; i++) printf("%02X ", (UINT8)NvRamDataAddr->AirLinkCfgEle.RACHPairInfor[i].RACHPairSCGIndex);

    printf("\r\nmaxscale  = %02X"
           "\r\npremscale = %02X", NvRamDataAddr->AirLinkCfgEle.MaxScale, NvRamDataAddr->AirLinkCfgEle.PremScale);

    printf("\r\nBCHSCALE  = ");
    for(int i=0;  i<8; i++) printf("%04X ", (UINT16)NvRamDataAddr->AirLinkCfgEle.CHScale[i].BCHScale);
    printf("\r\nTCHSCALE  = ");
    for(int i=0;  i<8; i++) printf("%04X ", (UINT16)NvRamDataAddr->AirLinkCfgEle.CHScale[i].TCHScale);

    printf("\r\nW0I       = ");
    for(int i=0;  i<8; i++) 
    {
        UINT16 temp = (SINT16)NvRamDataAddr->AirLinkCfgEle.W0Info[i].WI;
        printf("%04X ", temp);
    }
    printf("\r\nW0Q       = ");
    for(int i=0;  i<8; i++)
    {
        UINT16 temp = (SINT16)NvRamDataAddr->AirLinkCfgEle.W0Info[i].WQ;
        printf("%04X ", temp);
    }
    
    printf("\r\n\r\nL2 RM DATA:####################################");
    SINT16 temp = (SINT16)NvRamDataAddr->RMPoliceEle.MinULSS;
    printf("\r\nBWReqInter= %04X"
           "\r\nSRelThres = %04X"
           "\r\nMinULSS   = %d"
           "\r\nMaxDLPPUs = %04X"
           "\r\nDLBWPerUs = %04X"
           "\r\nULBWPerUsr= %04X"
           "\r\nRsvTCH    = %04X"
           "\r\nRsvPower  = %04X"
           "\r\nPCRange   = %04X"
           "\r\nStepSize  = %04X"
           "\r\nMaxUser   = %04X",
           (UINT16)NvRamDataAddr->RMPoliceEle.BWReqInterval,
           (UINT16)NvRamDataAddr->RMPoliceEle.SRelThreshold,
           temp,
           (UINT16)NvRamDataAddr->RMPoliceEle.MaxDLPPUser,
           (UINT16)NvRamDataAddr->RMPoliceEle.DLBWPerUser,
           (UINT16)NvRamDataAddr->RMPoliceEle.ULBWPerUser,
           (UINT16)NvRamDataAddr->RMPoliceEle.RsvTCH,
           (UINT16)NvRamDataAddr->RMPoliceEle.RsvPower,
           (UINT16)NvRamDataAddr->RMPoliceEle.PCRange,
           (UINT16)NvRamDataAddr->RMPoliceEle.StepSize,
           (UINT16)NvRamDataAddr->RMPoliceEle.MaxUser);

    printf("\r\nBWDistClas= ");
    for(int i=0;  i<8; i++) printf("%02X ", (UINT8)NvRamDataAddr->RMPoliceEle.BWDistClass[i]);
    printf("\r\nULModMask = %02X"
           "\r\nDLModMask = %02X\r\n",  
           NvRamDataAddr->RMPoliceEle.ULModMask,  
           NvRamDataAddr->RMPoliceEle.DLModMask);

    T_N_Parameter *pN1Para = &(NvRamDataAddr->N_parameter);
    printf(
        "\r\nN Algorithm Switch= 0x%x"
        "\r\nCi Jump detection = 0x%x"
        "\r\nUT PowerLock Timer Th = 0x%x"
        "\r\n%.4x %.4x %.4x %.4x %.4x %.4x %.4x ",
        pN1Para->N_Algorithm_Switch,
        pN1Para->Ci_Jump_detection,
        pN1Para->UT_PowerLock_Timer_Th,
        pN1Para->N_para[0],pN1Para->N_para[1],pN1Para->N_para[2],pN1Para->N_para[3],pN1Para->N_para[4],pN1Para->N_para[5],pN1Para->N_para[6]
     );
	//ems ranging cfg
    T_RangingPara *pRangingpara = &(NvRamDataAddr->RangingPara);
    printf("\r\n Ranging_Switch = %d"
		"\r\n Enable_Shreashold = %d"
		"\r\n Disable_Shreahold = %d"
		"\r\n Ratio_Shreahold = %d"
		"\r\n SNR_Shreahold = %d"
		"\r\n Ranging_Offset_Shreahold = %d"
		"\r\n cfg_flag = %x",		
		pRangingpara->Ranging_Switch,
		pRangingpara->Enable_Shreashold,
		pRangingpara->Disable_Shreahold,
		pRangingpara->Ratio_Shreahold,
		pRangingpara->SNR_Shreahold,
		pRangingpara->Ranging_Offset_Shreahold,
		pRangingpara->cfg_flag);
	printf("\r\n");

}


void l3oamprintl3data()
{
    printf("\r\nBTS REPEATER NUM:");
    if(NvRamDataAddr->BTSRepeaterEle.RepeaterFreqNum < NEIGHBOR_BTS_NUM)
    {
        printf("0x%02X", NvRamDataAddr->BTSRepeaterEle.RepeaterFreqNum);
    }
    {
        printf("BTS REPEATER NOT INIT");
    }
    printf("\r\nL3 BTS GEN CFG:####################################");
    printf("\r\nBtsIPAddr  = 0x%08X"
           "\r\nDefGateway = 0x%08X"
           "\r\nSubnetMask = 0x%08X"
           "\r\nSAGID      = 0x%08X"
           "\r\nSAGVoiceIP = 0x%08X"
           "\r\nSAGSignalIP= 0x%08X"
           "\r\nSAGRxPortV = 0x%04X"
           "\r\nSAGTxPortV = 0x%04X"
           "\r\nSAGRxPortS = 0x%04X"
           "\r\nSAGTxPortS = 0x%04X"
           "\r\nLocAreaID  = 0x%08X"
           "\r\nSAGSPC     = 0x%04X"
           "\r\nBTSSPC     = 0x%04X"
           "\r\nEmsIPAddr  = 0x%08X"
           "\r\nNetworkID  = 0x%04X"
           "\r\nBtsBootSrc = 0x%04X",
           // "\r\nNatAPKey = 0x%04X",
           NvRamDataAddr->BtsGDataCfgEle.BtsIPAddr,
           NvRamDataAddr->BtsGDataCfgEle.DefGateway,
           NvRamDataAddr->BtsGDataCfgEle.SubnetMask,
           NvRamDataAddr->BtsGDataCfgEle.SAGID,
           NvRamDataAddr->BtsGDataCfgEle.SAGVoiceIP,
           NvRamDataAddr->BtsGDataCfgEle.SAGSignalIP,
           
           NvRamDataAddr->BtsGDataCfgEle.SAGRxPortV,
           NvRamDataAddr->BtsGDataCfgEle.SAGTxPortV,
           NvRamDataAddr->BtsGDataCfgEle.SAGRxPortS,
           NvRamDataAddr->BtsGDataCfgEle.SAGTxPortS,
           NvRamDataAddr->BtsGDataCfgEle.LocAreaID,
          
           NvRamDataAddr->BtsGDataCfgEle.SAGSPC,
           NvRamDataAddr->BtsGDataCfgEle.BTSSPC,
           NvRamDataAddr->BtsGDataCfgEle.EmsIPAddr,
           NvRamDataAddr->BtsGDataCfgEle.NetworkID,
           NvRamDataAddr->BtsGDataCfgEle.BtsBootSource
          // NvRamDataAddr->BtsGDataCfgEle.NatAPKey
           );

    printf("\r\nNatAPKey = 0x%04X",NvRamDataAddr->BtsGDataCfgEle.NatAPKey);
    
    printf("\r\n\r\nL3 Cluster Para:####################################");
    printf("\r\nClusterPara.cfg_flag            = %d"
           "\r\nClusterPara.flag                = %d"
           "\r\nClusterPara.sleep_period        =%d"
           "\r\nClusterPara.Rsv_Ch_Resourse_Num = %d"
           "\r\nClusterPara.rsv1                = %d"
           "\r\nClusterPara.rsv2                = %d"
           "\r\nClusterPara.rsv3                = %d",
           NvRamDataAddr->ClusterPara.cfg_flag,
           NvRamDataAddr->ClusterPara.flag,
           NvRamDataAddr->ClusterPara.sleep_period,
           NvRamDataAddr->ClusterPara.Rsv_Ch_Resourse_Num, 
           NvRamDataAddr->ClusterPara.rsv1,
           NvRamDataAddr->ClusterPara.rsv2,
           NvRamDataAddr->ClusterPara.rsv3
          );


    
    printf("\r\n\r\nL3 DATA SERVICE CFG:####################################");
    printf("\r\nRoutAreaID  = 0x%08X"
           "\r\nMobility    = 0x%02X"
           "\r\nAccessCtrl  = 0x%02X"
           "\r\nLBATimerLen = 0x%04X"
           "\r\nP2PBridging = 0x%02X"
           "\r\nEgrARPRroxy = 0x%02X"
           "\r\nEgrBCFilter = 0x%02X"
           "\r\nPPPSesLen   = 0x%04X"
           "\r\nTargetBTSID = 0x%02X"
           "\r\nTargetEID   = 0x%02X"
           "\r\nPPPoEEID    = 0x%02X",
           NvRamDataAddr->DataServiceCfgEle.RoutingAreaID,
           NvRamDataAddr->DataServiceCfgEle.Mobility,
           NvRamDataAddr->DataServiceCfgEle.AccessControl,
           NvRamDataAddr->DataServiceCfgEle.LBATimerLen, 
           NvRamDataAddr->DataServiceCfgEle.P2PBridging,
           NvRamDataAddr->DataServiceCfgEle.EgrARPRroxy,
           NvRamDataAddr->DataServiceCfgEle.EgrBCFilter,
           NvRamDataAddr->DataServiceCfgEle.PPPSessionLen,
           NvRamDataAddr->DataServiceCfgEle.TargetBTSID,
           NvRamDataAddr->DataServiceCfgEle.TargetEID,
           NvRamDataAddr->DataServiceCfgEle.TargetPPPoEEID
          );
    //by xiaoweifang.
    printf("\r\n\r\nBTS neighbor list CFG:####################################");
    {
    UINT16 NeigBtsNum = NvRamDataAddr->BtsNeighborCfgEle.NeighborBtsNum;
    printf("\r\nBTS neighbor number[0 ~ %d]: %d", NEIGHBOR_BTS_NUM,NeigBtsNum);
    if((NeigBtsNum <= NEIGHBOR_BTS_NUM)&&(NeigBtsNum>0))
        {
        UINT8 *pData = (UINT8*)(NvRamDataAddr->BtsNeighborCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
        for(UINT16 i = 0; i< NeigBtsNum; i++)
            {
            T_BtsNeighborCfgData *pNeighbor = (T_BtsNeighborCfgData*)pData;
            T_BtsInfoIE *pBtsInfoIE = &(pNeighbor->BtsInfoIE);
            printf(
                "\r\nNo. %d"
                "\r\n\tBTS ID                 = %d"
                "\r\n\tBtsIP                  = 0x%08X"
                "\r\n\tFrequency Index        = 0x%08X"
                "\r\n\tSequence ID            = 0x%08X"
                "\r\n\tSub carrier Group Mask = 0x%08X"
                "\r\n\tRepeater number        = 0x%08X",
                i,
                pBtsInfoIE->BTSID,
                pNeighbor->BtsIP,
                pBtsInfoIE->FrequencyIndex,
                pBtsInfoIE->SequenceID,
                pBtsInfoIE->SubcarrierGroupMask,
                pNeighbor->RepeaterNumber);
            pData = (UINT8*)(pNeighbor->RepeaterStartFreq) + (pNeighbor->RepeaterNumber)*sizeof(UINT16);
            //pNeighbor++;  //变长数据结构
            }
        }
    }
    //new print by jy 080717
    printf("\r\n\r\nBTS neighbor list For Common CFG:####################################");
    {
    UINT16 NeigBtsNum = NvRamDataAddr->BtsNeighborCommCfgEle.NeighborBtsNum;
    printf("\r\nBTS neighbor number[0 ~ %d]: %d", NEIGHBOR_BTS_NUM,NeigBtsNum);
    if((NeigBtsNum <= NEIGHBOR_BTS_NUM)&&(NeigBtsNum>0))
        {
        UINT8 *pData = (UINT8*)(NvRamDataAddr->BtsNeighborCommCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
        for(UINT16 i = 0; i< NeigBtsNum; i++)
            {
            T_BtsNeighborCfgData *pNeighbor = (T_BtsNeighborCfgData*)pData;
            T_BtsInfoIE *pBtsInfoIE = &(pNeighbor->BtsInfoIE);
            printf(
                "\r\nNo. %d"
                "\r\n\tBTS ID                 = %d"
                "\r\n\tBtsIP                  = 0x%08X"
                "\r\n\tFrequency Index        = 0x%08X"
                "\r\n\tSequence ID            = 0x%08X"
                "\r\n\tSub carrier Group Mask = 0x%08X"
                "\r\n\tRepeater number        = 0x%08X",
                i,
                pBtsInfoIE->BTSID,
                pNeighbor->BtsIP,
                pBtsInfoIE->FrequencyIndex,
                pBtsInfoIE->SequenceID,
                pBtsInfoIE->SubcarrierGroupMask,
                pNeighbor->RepeaterNumber);
            pData = (UINT8*)(pNeighbor->RepeaterStartFreq) + (pNeighbor->RepeaterNumber)*sizeof(UINT16);
            //pNeighbor++;  //变长数据结构
            }
        }
    }
    printf("\r\n");
    printf("\r\n\r\n UT Handover parameter CFG########################");
    printf("\r\n M_HO_PWR_THD:%d", NvRamDataAddr->UtHandoverPara.M_HO_PWR_THD);
    printf("\r\n M_HO_PWR_OFFSET1:%d", NvRamDataAddr->UtHandoverPara.M_HO_PWR_OFFSET1);
    printf("\r\n M_HO_PWR_OFFSET2:%d", NvRamDataAddr->UtHandoverPara.M_HO_PWR_OFFSET2);
    printf("\r\n M_CPE_CM_HO_PERIOD:%d", NvRamDataAddr->UtHandoverPara.M_CPE_CM_HO_PERIOD);
    printf("\r\n M_HO_PWR_FILTERCOEF_STAT:%d", NvRamDataAddr->UtHandoverPara.M_HO_PWR_FILTERCOEF_STAT);
    printf("\r\n M_HO_PWR_FILTERCOEF_MOBILE:%d", NvRamDataAddr->UtHandoverPara.M_HO_PWR_FILTERCOEF_MOBILE);
    printf("\r\n TIME_TO_TRIGGER:%d", NvRamDataAddr->UtHandoverPara.TIME_TO_TRIGGER);
    printf("\r\n");
     printf("\r\n");
    printf("\r\n\r\n L1 BASIC General Settint ########################");
    printf("\r\n Antenna Mask:%x", NvRamDataAddr->L1GenCfgEle.AntennaMask);
    printf("\r\n Sync Source:%d", NvRamDataAddr->L1GenCfgEle.SyncSrc);
    printf("\r\n Gps Offset:%d", NvRamDataAddr->L1GenCfgEle.GpsOffset);

   if(NvRamDataAddr->rf_operation.flag==0x55aa55aa)
   {
		printf("\r\n\r\n RF  OPEN/CLOSE CFG ########################"); 
		printf("\r\n Type:%x", NvRamDataAddr->rf_operation.type);
		printf("\r\n Close_RF_Time_Len:%d", NvRamDataAddr->rf_operation.Close_RF_Time_Len);
		printf("\r\n Open_RF_Time_Len:%d", NvRamDataAddr->rf_operation.Open_RF_Time_Len);		      
		printf("\r\n GateWayIP1:%x", NvRamDataAddr->rf_operation.GateWayIP1);
		printf("\r\n GateWayIP2:%x", NvRamDataAddr->rf_operation.GateWayIP2);
   }
    printf("\r\n");
#ifdef WBBU_CODE
       if(NvRamDataAddr->fiber_para.initialized==0x5555aaaa)
   {
		printf("\r\n\r\n Fiber Info  CFG ########################"); 
		printf("\r\n Voltage(uV):%d", NvRamDataAddr->fiber_para.Voltage);
		printf("\r\n Current(uA):%d", NvRamDataAddr->fiber_para.Current);
		printf("\r\n Tx_Power(uW):%d", NvRamDataAddr->fiber_para.Tx_Power);		      
		printf("\r\n Rx_Power(uW):%d", NvRamDataAddr->fiber_para.Rx_Power);
	
   }
    printf("\r\n");
  #endif
    printf("\r\n");
    printf("\r\n CPE Valid Freqs Para###################");
    if(NvRamDataAddr->volidFreqPara.validFlag==0x5a5a)
    {        
        printf("\r\n validFreqsInd:%d", NvRamDataAddr->volidFreqPara.validFreqsInd);
        printf("\r\n validFreqsNum:%d", NvRamDataAddr->volidFreqPara.validFreqsNum);
        printf("\r\n validFreqs: ");
        for(char cr=0; cr<NvRamDataAddr->volidFreqPara.validFreqsNum; cr++)
        {
            printf("%x, ", NvRamDataAddr->volidFreqPara.validFreqs[cr]);
        }
    }
    else
        printf("\r\n validFlag is not valid");
    printf("\r\n");
    
}
void l3oaminitnvram()
{
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] initializing BTS nvram...");
	UINT8 val = 0;
    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->init_code_high), (SINT8*)&val, sizeof(NvRamDataAddr->init_code_high));
    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->init_code_low), (SINT8*)&val,  sizeof(NvRamDataAddr->init_code_low));
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "You need to reboot BTS to take effect");
//    taskDelay(500);
//    rebootBTS(BOOT_CLEAR );
}

void l3oamactcalibration()
{
    CTaskCfg* ptask = CTaskCfg :: GetInstance();
    CCfgCalActionReq ReqMsg;
    ReqMsg.CreateMessage(*ptask);
    ReqMsg.SetMessageId(M_L3_L2_INSTANT_CALIBRATION_REQ);
    ReqMsg.SetTransactionId(OAM_DEFAUIT_TRANSID);
    ReqMsg.SetDstTid(M_TID_L2MAIN);
    ReqMsg.SetCalType(NvRamDataAddr->CalCfgEle.CalType);
    ReqMsg.SetCalTrigger(0);    //0 - manual.
    if(true != ReqMsg.Post())
    {
        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] post msg[0x%04x] fail", ReqMsg.GetMessageId());
        ReqMsg.DeleteMessage();
    }
    
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] send Calibration Action Request msg to L2MAIN");
}

/*
 *强制删除系统已经配置过的Telnet用户
 */
void l3oamdeluser()
{
    T_BTSUserCfgEle *pNvUser = &(NvRamDataAddr->BTSUserCfgEle);
    T_BTSUserCfgEle zero;
    memset(&zero, 0, sizeof(zero));
    CTaskCfg::l3oambspNvRamWrite((char*)pNvUser, (char*)&zero, sizeof(zero));
    return;
}

void l3oamprintgroup()
{
    T_VlanGroupCfgEle *pGroup = &(NvRamDataAddr->VlanGroupCfgEle);
    printf("\r\nBTS Group<-->VLAN: %d configured", pGroup->number);
    for(int i = 0; i < pGroup->number; ++i)
        {
        printf("\r\n\tGroupID:%5d,\tVLAN:%d", pGroup->group[i].usGroupID, pGroup->group[i].usVlanID);
        }
    printf("\r\n");
}
//#ifdef LJF_WAKEUPCONFIG
void setWakeupConf( UINT8 usSwitch, UINT32 ulPeriod )
{
	if( usSwitch!=0 && usSwitch!=1 )
		printf( "the 1st value shoud be [0|1]" );
	CTaskCfg::GetInstance()->CM_setWakeupConf( usSwitch, ulPeriod );
	return;
}
void showWakeupConf()
{
	T_WAKEUP_CONFIG *pst = (T_WAKEUP_CONFIG*)&NvRamDataAddr->stWakeupConf;
	printf( "\r\n*  *  *  *  *  *  *  *  *  *  *  *  *  *  *  *" );
	printf( "\r\nFlag[%04x]  switch[%04d]  period[%08d]", pst->usFlag, pst->usSwitch, pst->ulPeriod );
	return;
}
void CTaskCfg :: CM_setWakeupConf( UINT8 usSwitch, UINT32 ulPeriod )
{
	T_WAKEUP_CONFIG stWkupConf;
	stWkupConf.usFlag = M_WAKEUP_CONFIG_FLAG;
	stWkupConf.usSwitch = usSwitch;
	stWkupConf.ulPeriod = ulPeriod;
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->stWakeupConf.usFlag, (char*)&stWkupConf, sizeof(T_WAKEUP_CONFIG) );
	CM_SendComMsg( M_TID_L2MAIN, MSGID_L3_L2_WAKEUPCONFIG, (UINT8*)&NvRamDataAddr->stWakeupConf.usFlag, sizeof(T_WAKEUP_CONFIG) );
	T_WAKEUP_CONFIG *pst = (T_WAKEUP_CONFIG*)&NvRamDataAddr->stWakeupConf.usFlag;
	printf( "\r\nFlag[%04x]  switch[%04d]  period[%08d]", pst->usFlag, pst->usSwitch, pst->ulPeriod );
	return;
}
//#ifdef LJF_Q64
void setQam64Ppc0( UINT8 uc ) 
{
	if( 0==uc || 1==uc )
	{
		char* puc = (char*)&NvRamDataAddr->Qam64Cfg.Qam64Ppc0;
		CTaskCfg::l3oambspNvRamWrite( (char*)puc, (char*)&uc, 1 );
		CTaskCfg::GetInstance()->CM_Qam64();
	}
	else
	{
		printf( "setQam64Ppc0 [ 0 | 1 ]!" );
	    printf("\r\n");
	}
}
void setQam4ForQam64( UINT8 uc )
{
	if( 0==uc || 1==uc )
	{
		char* puc = (char*)&NvRamDataAddr->Qam64Cfg.Qam4ForQam64;
		CTaskCfg::l3oambspNvRamWrite( (char*)puc, (char*)&uc, 1 );
		CTaskCfg::GetInstance()->CM_Qam64();
	}
	else
	{
		printf( "setQam4ForQam64 [ 0 | 1 ]!" );
	    printf("\r\n");
	}
}
bool CTaskCfg :: CM_Qam64()
{
	CCfgQam64Req CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(MSGID_L3_L2_QAM64_CFG);
    CfgReq.SetDstTid(M_TID_L2MAIN);
    CfgReq.SetValue( (UINT8*)&NvRamDataAddr->Qam64Cfg.Qam64Ppc0);
    OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_REV_MSG, "\n[CFG] send CM_Qam64 CFG to l2....... \n");
//#ifdef LJF_WAKEUPCONFIG
	CM_SendComMsg( M_TID_L2MAIN, MSGID_L3_L2_WAKEUPCONFIG, (UINT8*)&NvRamDataAddr->stWakeupConf.usFlag, sizeof(T_WAKEUP_CONFIG) );
    return CM_InitSendCfgReq(CfgReq);     
}

bool CTaskCfg :: CM_LoadBlnInfo()
{
    //lijinan 20110331 增加负载均衡参数
#ifdef PAYLOAD_BALANCE_2ND
    memcpy(&m_stPldBlnCfg, &NvRamDataAddr->PayloadCfg.usFlag,sizeof(m_stPldBlnCfg)-2);
    memcpy(&m_stPldBlnCfg.usBandSwitch, &NvRamDataAddr->usBandSwitch,sizeof(UINT16));
#else
//    memcpy(&m_stPldBlnCfg, &NvRamDataAddr->PayloadCfg.usFlag,sizeof(m_stPldBlnCfg));
    memcpy(&m_stPldBlnCfg, &NvRamDataAddr->PayloadCfg.usFlag,sizeof(m_stPldBlnCfg));
#endif

	CCfgPayloadReq CfgReq;
	CfgReq.CreateMessage(*this);
	CfgReq.SetMessageId(MSGID_L3_L2_PAYLOAD_BALANCE_CFG);
	CfgReq.SetDstTid(M_TID_L2OAM);
	CfgReq.SetValue((UINT8*)&m_stPldBlnCfg);

	return CM_InitSendCfgReq(CfgReq);     
}
void showNvRamNew()
{
	printf("\r\n SAVE POWER STATE BEGIN  --------------------------------------------------------" );
	printf("\r\n SaveFlag[%04X]", NvRamDataAddr->SavePwr.SavePwrFlag );
	printf("\r\n L2Cfg[%04X][%04X][%04X][%04X]", NvRamDataAddr->SavePwr.TS1Channel, NvRamDataAddr->SavePwr.TS1User, NvRamDataAddr->SavePwr.TS2Channel, NvRamDataAddr->SavePwr.TS2User);
	printf("\r\n FanCfg[%04X][%04X][%04X][%04X]", NvRamDataAddr->SavePwr.Fan1, NvRamDataAddr->SavePwr.Fan2, NvRamDataAddr->SavePwr.Fan3, NvRamDataAddr->SavePwr.Fan4);
	printf("\r\n QAM64 CONFIG BEGIN  --------------------------------------------------------" );
	printf("\r\n Qam64Ppc0[%02d] Qam4ForQam64[%02d]", NvRamDataAddr->Qam64Cfg.Qam64Ppc0, NvRamDataAddr->Qam64Cfg.Qam4ForQam64 );
	printf("\r\n" );

}
void setVacPrefScg( UINT8 uc )
{
	if( 0==uc || 1==uc )
	{
		char* puc = (char*)&NvRamDataAddr->ucVacPrefScg;
		CTaskCfg::l3oambspNvRamWrite( (char*)puc, (char*)&uc, 1 );
		UINT8* pul = (UINT8*)&NvRamDataAddr->init_code_low + sizeof(NvRamDataAddr->init_code_low);
		CTaskCfg::GetInstance()->CM_VacPrefScg();
		printf( "NvRamData Tail Value[%08X][%08X][%08X]", *((UINT32*)pul), *((UINT32*)pul+1), *((UINT32*)pul+2) );
	    printf("\r\n");
		printf( "setVacPrefScg success [%s]", (uc==0)?"FALSE":"TRUE" );
	    printf("\r\n");
	}
	else
	{
		printf( "setVacPrefScg [ 0 | 1 ]!" );
	    printf("\r\n");
	}
}

bool CTaskCfg :: CM_VacPrefScg()
{
	CCfgVacPrefScgReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(MSGID_L3_L2_VACPREFSCG_CFG);
    CfgReq.SetDstTid(M_TID_L2MAIN);
    CfgReq.SetValue( NvRamDataAddr->ucVacPrefScg );
    OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_REV_MSG, "\n[CFG] CM_VacPrefScg send vac prefscg to l2....... \n");
    return CM_InitSendCfgReq(CfgReq);     
}
bool CTaskCfg :: CM_SendComMsg( TID tid, UINT16 usMsgID, UINT8* pd, UINT16 usLen, UINT32 eid )
{
    CComMessage *RspMsg = new ( this, usLen ) CComMessage;
    if( RspMsg != NULL )
    {
        RspMsg->SetDstTid( tid );
        RspMsg->SetMessageId( usMsgID );
        RspMsg->SetEID( eid );
        RspMsg->SetSrcTid( this->GetEntityId());
		if( 0 != usLen )
        	memcpy( (UINT8*)RspMsg->GetDataPtr(), pd, usLen );
        if( ! CComEntity :: PostEntityMessage( RspMsg ) )
        {
            RspMsg->Destroy();
			return false;
        }
    }
    return true;
}
#ifdef NUCLEAR_CODE
CTimer* CTaskCfg :: InitTimer( bool bPeriod, UINT16 usMsgID, UINT16 usPeriod )
{
    CComMessage *pMsgTimer = new ( this, 0 ) CComMessage;
    CMessage *pMsgConTainer = new CMessage( pMsgTimer );
    if (pMsgTimer!=NULL)
    {
        pMsgTimer->SetDstTid( M_TID_CM );
        pMsgTimer->SetSrcTid( M_TID_CM );
        pMsgTimer->SetMessageId( usMsgID );
    }
    CTimer* pTmTmp = new  CTimer( bPeriod, usPeriod, *pMsgConTainer );
    return pTmTmp;
}
void CTaskCfg :: CM_NuclearSendLimitData( bool bBC, UINT32 eid )
{
	if( NULL == m_pucLimitedNeiBts )
		return;
	if( bBC )
	{
		*(UINT16*)m_pucLimitedNeiBts = 0x01;
		CM_SendComMsg( M_TID_CPECM, MSGID_BROADCAST_OAM, m_pucLimitedNeiBts, m_usLimitedNeiBtsLen, eid );
        OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] CM_NuclearSendLimitData[BC] eid[%08x]", eid);
	}
	else
	{
		if( 0 == m_ucLimitFlag )
		{
			CM_SendComMsg( M_TID_CPECM, 0x6016, m_pucLimitedNeiBts+2, m_usLimitedNeiBtsLen-2, eid );
	        OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] CM_NuclearSendLimitData[UC] eid[%08x]", eid);
		}
	}
}
void nutest( UINT8 uc )
{
	CTaskCfg::GetInstance()->nuprint(uc);
}
void CTaskCfg :: nuprint(UINT8 uc)
{
	switch (uc)
	{
	case 0:
		printf("\r\n limitarea[%d], timer[%08x]", m_ucLimitFlag, m_ptmLimitedBC );
		break;
	case 1:
		printf( "\r\nHO new[%02x][%02x][%02x][%04x]",   NvRamDataAddr->stUtHandoverPara2.StrictArea_Pwr_THD, 
														NvRamDataAddr->stUtHandoverPara2.StrictArea_TIME_TO_TRIGGER, 
														NvRamDataAddr->stUtHandoverPara2.StrictArea_HO_PWR_OFFSET, 
														NvRamDataAddr->stUtHandoverPara2.write_flag );
		break;
	case 2:
		printf( "\r\nm_usLimitedNeiBts length[%d]\r\n", m_usLimitedNeiBtsLen );
		if( m_usLimitedNeiBtsLen > 100 )
			break;
		for(UINT8 u=0; u<m_usLimitedNeiBtsLen; u++ )
		{
			printf("%02x", *(m_pucLimitedNeiBts+u));
			if( 20 == u )
				printf("\r\n");
			else
				printf(" ");
		}
		printf("\r\n");
		break;
	case 3:
		UINT8 uc[2+12];
		uc[0] = 0xee;
		uc[1] = 0xee;
		for( UINT8 uct=2; uct<14; uct++ )
			uc[uct] = uct;
		CM_SendComMsg( M_TID_CM, M_EMS_BTS_HANDOVER_PARA_CFG_REQ, uc, 14 );
		break;	
	case 4:
		#pragma pack(1)
		struct{
			UINT16 transid;
			UINT8  u1;
			UINT32 u2;
			UINT16 u3;
			UINT8  u4;
			UINT8  u5;
			UINT8  u6;
			UINT8  u7;
		}uSd;
		#pragma pack()
		uSd.transid = 0xeeee;
		uSd.u1=1;
		uSd.u2=0x01312096;
		uSd.u3=1960;
		uSd.u4=7;
		uSd.u5=1;
		uSd.u6=123;
		uSd.u7=1;
		CM_SendComMsg( M_TID_CM, M_EMS_BTS_NUCLEAR_CFG_REQ, (UINT8*)&uSd, sizeof(uSd) );
		break;	
	default:
		printf( "\r\n0: print LIMITED or NOLIMITED" );
		printf( "\r\n1: print new nvram data" );
		printf( "\r\n2: print m_pucLimitedNeiBts" );
		printf( "\r\n3: send M_EMS_BTS_HANDOVER_PARA_CFG_REQ" );
		printf( "\r\n4: send M_EMS_BTS_NUCLEAR_CFG_REQ" );
		break;			
	}
}
#endif

void testSagBkp( UINT8 uc )
{
	if( 1==uc )
		CTaskCfg::GetInstance()->testSagBackup();
	else if( 0==uc )
		CTaskCfg::GetInstance()->testSagBackupShow();
}
UINT8 gucTest=0;
void CTaskCfg :: testSagBackupShow()
{
	printf( "SAGFalg  [%02X] gucTest[%02X]\n\r", NvRamDataAddr->ucSagFlag, gucTest );
	printf( "SAGAddr  [%08X] [%08X] [%08X] [%04X] [%04X] [%04X] [%04X] [%08X] [%04X] [%04X] [%02X %02X %02X %02X %02X %02X %02X %02X] [%02X]\n\r", 
		NvRamDataAddr->SagBkp.ulSagID, 
		NvRamDataAddr->SagBkp.ulSagVoiceAddr, 
		NvRamDataAddr->SagBkp.ulSagSignalAddr,
		NvRamDataAddr->SagBkp.usVoiceRxPort,
		NvRamDataAddr->SagBkp.usVoiceTxPort,
		NvRamDataAddr->SagBkp.usSignalRxPort,
		NvRamDataAddr->SagBkp.usSignalTxPort,
		NvRamDataAddr->SagBkp.ulLocationAreaID,
		NvRamDataAddr->SagBkp.ulSagSignalPointCode,
		NvRamDataAddr->SagBkp.ulBtsSignalPointCode,
		NvRamDataAddr->SagBkp.ucRsv[0],
		NvRamDataAddr->SagBkp.ucRsv[1],
		NvRamDataAddr->SagBkp.ucRsv[2],
		NvRamDataAddr->SagBkp.ucRsv[3],
		NvRamDataAddr->SagBkp.ucRsv[4],
		NvRamDataAddr->SagBkp.ucRsv[5],
		NvRamDataAddr->SagBkp.ucRsv[6],
		NvRamDataAddr->SagBkp.ucRsv[7],
		NvRamDataAddr->SagBkp.NatAPKey
	);
	printf( "JetterBuf[%04X] [%04X] [%04X] [%04X]\n\r", NvRamDataAddr->JitterBuf.usJtrBufEnable, NvRamDataAddr->JitterBuf.usJtrBufZEnable, NvRamDataAddr->JitterBuf.usJtrBufLength, NvRamDataAddr->JitterBuf.usJtrBufPackMax);
	printf( "SAGTos   [%02X] \n\r", NvRamDataAddr->SagTos.ucSagTosVoice);

}

void CTaskCfg :: testSagBackup()
{
/*	printf( "\n\r SAGFalg  [%02X] gucTest[%02X]", NvRamDataAddr->SagBkp.ucSagFlag, gucTest );
	printf( "\n\r SAGAddr  [%08X] [%08X]", NvRamDataAddr->SagBkp.ulSagVoiceAddr, NvRamDataAddr->SagBkp.ulSagSignalAddr);
	printf( "\n\r JetterBuf[%04X] [%04X] [%04X] [%04X]", NvRamDataAddr->SagBkp.usJtrBufEnable, NvRamDataAddr->SagBkp.usJtrBufZEnable, NvRamDataAddr->SagBkp.usJtrBufLength, NvRamDataAddr->SagBkp.usJtrBufPackMax);
	printf( "\n\r SAGAddr  [%02X] \n\r", NvRamDataAddr->SagBkp.ucSagTosVoice);
    */
    UINT8 ucLen = 2+sizeof(T_SagBkp)+2+sizeof(T_JitterBuf)+2+sizeof(T_SagTos);
	UINT8 us[ucLen];
	for( UINT8 uc=0; uc<ucLen; uc++ )
		us[uc] = gucTest + uc;
	gucTest++;
	if ( 0 == gucTest )
		gucTest++;
    CM_SendComMsg( M_TID_CM, M_EMS_BTS_SAG_CFG_REQ       , (UINT8*)us, 2+sizeof(T_SagBkp) ); 
    CM_SendComMsg( M_TID_CM, M_EMS_BTS_JITTERBUF_CFG_REQ, (UINT8*)us+2+sizeof(T_SagBkp), 2+sizeof(T_JitterBuf) );
    CM_SendComMsg( M_TID_CM, M_EMS_BTS_SAG_TOS_CFG_REQ  , (UINT8*)us+2+sizeof(T_SagBkp)+2+sizeof(T_JitterBuf), 2+sizeof(T_SagTos) );

    CM_SendComMsg( M_TID_CM, M_EMS_BTS_SAG_GET_REQ      , (UINT8*)us, 2 ); 
    CM_SendComMsg( M_TID_CM, M_EMS_BTS_JITTERBUF_GET_REQ, (UINT8*)us, 2 );
    CM_SendComMsg( M_TID_CM, M_EMS_BTS_SAG_TOS_GET_REQ  , (UINT8*)us, 2 );
}


#ifdef M_TGT_WANIF
UINT32  WanIfCpeEid = 0;
UINT32  BakWanIfCpeEid = 0;
//UINT32  RelayWanifCpeEid = 0;
UINT32  WorkingWcpeEid = 0;
extern UINT16   Wanif_Switch;
extern unsigned char  Local_Sac_Mac_Addr[5][6] ;
extern "C" void SetLocalEms(unsigned char  flag);
extern "C" int getMacFromSTD(char*);
// switch 2+ master  4+ copy 4 + relar +4 + mac 6
bool CTaskCfg ::CM_WCPE_Cfg(CMessage& rMsg)
{
    unsigned char * p = (unsigned char *)rMsg.GetDataPtr();
    unsigned short transid =  p[0]*0x100+p[1];
    unsigned short switch_flag = p[2]*0x100+p[3];
    unsigned int  wcpe = p[4]*0x1000000+p[5]*0x10000+p[6]*0x100+p[7]; 
   unsigned int copyeid= p[8]*0x1000000+p[9]*0x10000+p[10]*0x100+p[11];   
   unsigned char mac[5][6];
   unsigned short flag;
   for(int i=0; i<5; i++)
       memcpy(mac[i],&p[12+i*6],6);
   if(switch_flag==1)
   {
        if((wcpe==0)||(wcpe == 0xffffffff))
        	{
        	    CM_PostCommonRsp(M_TID_EMSAGENTTX, transid, M_BTS_EMS_WCPE_CFG_RSP, OAM_FAILURE);
		    OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "\n[CFG] CM_WCPE_Cfg wcpe eid error:%08x....... \n",wcpe);
			return FALSE;
        	}
   }
    l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->WanIfCpeEid), (SINT8*)&wcpe, sizeof(UINT32));
     l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->BakWanIfCpeEid), (SINT8*)&copyeid, sizeof(UINT32));
    l3oambspNvRamWrite((SINT8*)(NvRamDataAddr->Sac_Mac_Addr), (SINT8*)mac, 30);    
     if(switch_flag==1)
     {
	 	flag = 0x5a5a;
		l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Wcpe_Switch), (SINT8*)&flag, 2);
		if( g_rf_openation.type !=0)//wangwenhua add 2012-3-7
		{
		 	g_rf_openation.type = 0;
			 
			OAM_LOGSTR(LOG_CRITICAL, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_WCPE_Cfg:curent mode is wcpe the RF CFG disabled");
		}
	
		
     	}
	 else
	 {
	 	flag = 0;
		l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Wcpe_Switch), (SINT8*)&flag, 2);
		     if(NvRamDataAddr->rf_operation.flag!=0x55aa55aa)
		      	{
		      	     g_rf_openation.type = 0;
			   
		      	}
			else
			{
			   g_rf_openation.type = NvRamDataAddr->rf_operation.type;
			   OAM_LOGSTR1(LOG_CRITICAL, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_WCPE_Cfg:enable RF CFG Type:%d\n",g_rf_openation.type);
			}
	
	 }
	
	 CM_PostCommonRsp(M_TID_EMSAGENTTX, transid, M_BTS_EMS_WCPE_CFG_RSP, OAM_SUCCESS);
	 
	 taskDelay(100);
	  CM_ConfigWanCPE();
   
}
void CTaskCfg :: CM_WCPE_Get(CMessage& rMsg)
{
      CComMessage* pComMsg = new (this, 44) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CM_WCPE_Get.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_WCPE_Get_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;//result
    unsigned char * p =(unsigned char *) pComMsg->GetDataPtr();
	  unsigned short flag ;
    if(NvRamDataAddr->Wcpe_Switch == 0x5a5a)
    {
         flag=1;
    }
   else
  {
        flag = 0;

   }
   p+=4;//transid+result 4 byte
   memcpy(p,(SINT8*)&flag,2);
  p+=2;
  memcpy(p,(SINT8*)&(NvRamDataAddr->WanIfCpeEid),4);
  p+=4;
   memcpy(p,(SINT8*)&(NvRamDataAddr->BakWanIfCpeEid),4);
   p+=4;  
   memcpy(p,(SINT8*)(NvRamDataAddr->Sac_Mac_Addr),30);

    if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
}
bool CTaskCfg ::CM_RCPE_Cfg(CMessage& rMsg)
{
    unsigned char * p = (unsigned char *)rMsg.GetDataPtr();
    unsigned short transid =  p[0]*0x100+p[1];
    unsigned short switch_flag = p[2]*0x100+p[3];
    unsigned char num = p[4];	
    UINT32 reid, i, flagU32, n32;
    
    if(switch_flag == 1)
        flagU32 = 0xa5a5a5a5;
    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid_falg), (SINT8*)&flagU32, sizeof(UINT32));
    if(num>20)
        num = 20;
    n32 = num;
#if 0
    if ( ( n32+m_stTrnukMRCpe.usNum) < 20 ) //liuweiodng add mobile rcpe number
    {
	 n32 = n32 + m_stTrnukMRCpe.usNum;
    }
    else
    {  // liuweidong total is 20,active mobile rcpe should be 20-new rcpe number;
	 m_stTrnukMRCpe.usNum = 20-n32;
 	l3oambspNvRamWrite( (char*)&NvRamDataAddr->stTrunkMRCpe, (char*)&m_stTrnukMRCpe, sizeof(TrunkMRCpeRcd) );
	 n32=20;
    } 
#endif

    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_num), (SINT8*)&n32, sizeof(UINT32));
    //rcpe增加10 个jiaying20100927
    if(num<=10)
    {
        for(i=0; i<num; i++)
        {
            reid = *(UINT32*)(p+5+i*4);
            CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid[i]), (SINT8*)&reid, sizeof(UINT32));
        }
	 //没有占有的添0	
	 for(i=num; i<10; i++)
    	 {
    	     reid = 0;
	     CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid[i]), (SINT8*)&reid, sizeof(UINT32));
      	 }
	 for(i=0; i<10; i++)
    	 {
    	     reid = 0;
	     CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid_New[i]), (SINT8*)&reid, sizeof(UINT32));
      	 }
    }
    else
    {
        for(i=0; i<10; i++)
        {
            reid = *(UINT32*)(p+5+i*4);
            CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid[i]), (SINT8*)&reid, sizeof(UINT32));
        }
	 for(i=0; i<num-10; i++)
        {
            reid = *(UINT32*)(p+5+(i+10)*4);
            CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid_New[i]), (SINT8*)&reid, sizeof(UINT32));
        }
	 for(i=num-10; i<10; i++)
    	 {
    	     reid = 0;
	     CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid_New[i]), (SINT8*)&reid, sizeof(UINT32));
      	 }
	 
    }
    memcpy((UINT8*)RelayWanifCpeEid, (UINT8*)NvRamDataAddr->Relay_WcpeEid, 4*10);
    memcpy((UINT8*)RelayWanifCpeEid+40, (UINT8*)NvRamDataAddr->Relay_WcpeEid_New, 4*10);
    CM_PostCommonRsp(M_TID_EMSAGENTTX, transid, M_BTS_EMS_RCPE_NEW_CFG_RSP, OAM_SUCCESS);
    
}
void CTaskCfg :: CM_RCPE_Get(CMessage& rMsg)
{
    UINT8 num;
    int i;
    CComMessage* pComMsg = new (this, 85) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CM_RCPE_Get.");
        return;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_CM);    
    pComMsg->SetMessageId(M_BTS_EMS_RCPE_NEW_GET_RSP);    	
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()) = *(UINT16*)((UINT8*)rMsg.GetDataPtr());//tranid
    
    *(UINT16*)((UINT8*)pComMsg->GetDataPtr()+2) = 0;//result
    unsigned char * p =(unsigned char *) pComMsg->GetDataPtr();
    unsigned short flag ;
    if(NvRamDataAddr->Relay_WcpeEid_falg == 0xa5a5a5a5)
    {
        flag=1;
    }
    else
    {
        flag = 0;
    }
    p+=4;//transid+result 4 byte
    memcpy(p,(SINT8*)&flag,2);
    p+=2;
    num = NvRamDataAddr->Relay_num;
    if(NvRamDataAddr->Relay_num>20)
        num = 0;
    memcpy(p,(UINT8*)&num,1);
    p+=1;  
    //rcpe增加10 个jiaying20100927
    if(num<=10)
    {
        for(i=0; i<num; i++)
        {
            memcpy(p,(UINT8*)&(NvRamDataAddr->Relay_WcpeEid[i]),4);
            p+=4;  
        }   
    }
    else
    {
        for(i=0; i<10; i++)
        {
            memcpy(p,(UINT8*)&(NvRamDataAddr->Relay_WcpeEid[i]),4);
            p+=4;  
        }  
        for(i=0; i<num-10; i++)
        {
            memcpy(p,(UINT8*)&(NvRamDataAddr->Relay_WcpeEid_New[i]),4);
            p+=4;  
        }    
    }
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }
}
bool CTaskCfg :: CM_ConfigWanCPE()
{
    int i;
    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tCfg] Begin init WAN IF CPE...");
   // CL3OamCfgWanIfCpeEidReq ReqMsg;
    //ReqMsg.CreateMessage(*this);
    //ReqMsg.SetDstTid(M_TID_WANIF);
    //ReqMsg.SetMessageId(M_L3_L2_CFG_WANIF_CPE_REQ);
    //ReqMsg.SetWanCpeEid(NvRamDataAddr->WanIfCpeEid);
    WanIfCpeEid = NvRamDataAddr->WanIfCpeEid;
   BakWanIfCpeEid = NvRamDataAddr->BakWanIfCpeEid;
   WorkingWcpeEid = WanIfCpeEid;
   Wanif_Switch = NvRamDataAddr->Wcpe_Switch;
   memcpy(Local_Sac_Mac_Addr,(NvRamDataAddr->Sac_Mac_Addr),30);
   for(i = 0; i<10; i++)
   {
   	RelayWanifCpeEid[i] = NvRamDataAddr->Relay_WcpeEid[i];
   }   
   for(i = 0; i<10; i++)//rcpe增加10 个jiaying20100927
   {
   	RelayWanifCpeEid[i+10] = NvRamDataAddr->Relay_WcpeEid_New[i];
   }  
   // return CM_InitSendCfgReq(ReqMsg);
   if((NvRamDataAddr->Enable_Wcpe_Flag_Judge!=0xeabd0000)||(NvRamDataAddr->Enable_Wcpe_Flag_Judge!=0xeabdeabd))
   	{
   	    
Enable_Wcpe_Flag_Judge = 0xeabdeabd;
   	}
   else
   	{
   		Enable_Wcpe_Flag_Judge = NvRamDataAddr->Enable_Wcpe_Flag_Judge;
   	}
   return OK;
}

bool CTaskCfg::CM_ConfigWanCpeFromShell(UINT32 eid, UINT32 bakeid )
{
    l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->WanIfCpeEid), (SINT8*)&eid, sizeof(UINT32));
     l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->BakWanIfCpeEid), (SINT8*)&bakeid, sizeof(UINT32));
   CM_ConfigWanCPE();
   return true;
}
#ifdef WBBU_CODE
extern "C"  void Send_Perf_2_PM()
{
     CTaskCfg::GetInstance()->CM_SendPerfCfg();
}
#endif
extern "C" 
STATUS bspSetWanIfCpe(UINT32 eid,UINT32 bakeid)
{
    CTaskCfg::GetInstance()->CM_ConfigWanCpeFromShell(eid,bakeid);
    return OK;
}
extern "C" 
STATUS bspSetWanIfSwitch(unsigned short switch_flag)
{
     unsigned short flag =0;
    
      if(switch_flag==1)
     	{
	 	flag = 0x5a5a;
		CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Wcpe_Switch), (SINT8*)&flag, 2);
		  Wanif_Switch = NvRamDataAddr->Wcpe_Switch;
		//  SetLocalEms(0);
		 printf("wanif switch is opened\n");
		
     	}
	 else
	 {
	 	flag = 0;
		CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Wcpe_Switch), (SINT8*)&flag, 2);
		  Wanif_Switch = NvRamDataAddr->Wcpe_Switch;
		//  SetLocalEms(1);
		  printf("wanif switch is closed\n");
	 }
	 	
    return OK;
}
#ifdef WBBU_CODE
extern "C" 
void Set_WanifSwitch(unsigned short value)
{
  Wanif_Switch =value;
}
#endif
STATUS bspSetCpeWcpeJudge(unsigned short switch_flag)
{
     unsigned int  flag =0;
     
      if(switch_flag==1)
     	{
	 	flag = 0xeabdeabd;
		CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Enable_Wcpe_Flag_Judge), (SINT8*)&flag, 4);
		  Enable_Wcpe_Flag_Judge = NvRamDataAddr->Enable_Wcpe_Flag_Judge;
		//  SetLocalEms(0);
	    printf("WANIF cpe's wcpe_flag judge Is Opened\n");
		
     	}
	 else
	 {
	 	flag = 0xeabd0000;
		CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Enable_Wcpe_Flag_Judge), (SINT8*)&flag, 4);
		  Enable_Wcpe_Flag_Judge = NvRamDataAddr->Enable_Wcpe_Flag_Judge;
		//  SetLocalEms(1);
	    printf("WANIF  cpe's wcpe_flag judge Is Closed\n");
	 }
	 	
    return OK;
}
extern "C" 
STATUS bspSetWanIfSacMacAddr(int num)
{    
    if(num>5)
    {
        printf("mac num must be less 5");
	 return 0;
    }
    printf("\r\n set Local Sac Mac addr.\r\n");    
    UINT8 strMAC[5][ 7 ] = {0};
    for(int i=0; i<num; i++)
    {
        printf("\r\nPlease Enter the MAC[%d] Address[format:xx-xx-xx-xx-xx-xx]", i);
        getMacFromSTD( (char*)strMAC[i] ); 	 
    }
    if(strMAC!=NULL)    
    {
        for(int i=0; i<num; i++)
    	{
            CTaskCfg::l3oambspNvRamWrite((SINT8*)(NvRamDataAddr->Sac_Mac_Addr[i]), (SINT8*)strMAC[i], 6);
            //printf("Local SAC MAC Addr:%02x-%02x-%02x-%02x-%02x-%02x\n",strMAC[0],strMAC[1],strMAC[2],strMAC[3],strMAC[4],strMAC[5]);
            memcpy(Local_Sac_Mac_Addr[i],(NvRamDataAddr->Sac_Mac_Addr[i]),6);
    	}
    }    
    return OK;
}

extern "C"
STATUS bspWanIfCpeShow()
{
    if(NvRamDataAddr->Wcpe_Switch == 0x5a5a)
    {
    	    printf("WANIF Switch Is Opened\n");
    }
	else
    {
	    printf("WANIF Switch Is Closed\n");
	}
    printf("Current Wan If CPE EID is 0x%x\n", NvRamDataAddr->WanIfCpeEid);
    printf("Current bakWan If CPE EID is 0x%x\n", NvRamDataAddr->BakWanIfCpeEid);
    //printf("Current Relay If CPE EID is 0x%x\n", NvRamDataAddr->Relay_WcpeEid);
    printf("Current working Wan If CPE EID is 0x%x\n", WorkingWcpeEid);
   for(int i=0; i<5; i++)
   {
       printf("Local SAC MAC Addr[%d]:%02x-%02x-%02x-%02x-%02x-%02x\n",i, NvRamDataAddr->Sac_Mac_Addr[i][0],NvRamDataAddr->Sac_Mac_Addr[i][1],
   	NvRamDataAddr->Sac_Mac_Addr[i][2],NvRamDataAddr->Sac_Mac_Addr[i][3],\
   	NvRamDataAddr->Sac_Mac_Addr[i][4],NvRamDataAddr->Sac_Mac_Addr[i][5]);
   }


   if(NvRamDataAddr->Enable_Wcpe_Flag_Judge == 0xeabd0000)
    {
    	    printf("WANIF cpe's wcpe_flag judge Is Closed\n");
    }
	else
    {
	    printf("WANIF  cpe's wcpe_flag judge Is Opened\n");
	}

    return OK;
}

extern "C" 
STATUS bspSetRelayWanIfCpe(UINT32 num)
{   
   int a =  0xa5a5a5a5;
   int i;
   UINT32 ulEid = 0;
   //rcpe增加10 个jiaying20100927
   if(num>20)
   {
       printf("RelayWanIfCpe num >20, pls cfg again\n");
	return OK;
   }
   CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_num), (SINT8*)&num, sizeof(UINT32));   
   for(i=0; i<num; i++)
   {
       printf("\r\nPlease Enter EID[%d]:", i);       
       getSTDNum((int*)&ulEid);
	if(i<10)
	    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid[i]), (SINT8*)&ulEid, sizeof(UINT32));
	else
	    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid_New[i-10]), (SINT8*)&ulEid, sizeof(UINT32));
       RelayWanifCpeEid[i] = ulEid;
   }   
   if(num<=10)
    {
        for(i=num; i<10; i++)
    	 {
    	     ulEid = 0;
	     CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid[i]), (SINT8*)&ulEid, sizeof(UINT32));
	     RelayWanifCpeEid[i] = ulEid;
      	 }
	 for(i=0; i<10; i++)
    	 {
    	     ulEid = 0;
	     CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid_New[i]), (SINT8*)&ulEid, sizeof(UINT32));
	     RelayWanifCpeEid[i+10] = ulEid;
      	 }
   }
   else
   {
       for(i=num-10; i<10; i++)
    	 {
    	     ulEid = 0;
	     CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid_New[i]), (SINT8*)&ulEid, sizeof(UINT32));
	     RelayWanifCpeEid[i+10] = ulEid;
      	 }
   }
   CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Relay_WcpeEid_falg), (SINT8*)&a, sizeof(UINT32));
   //NvRamDataAddr->Relay_WcpeEid_falg = 0xa5a5a5a5;

	return OK;
}
extern "C"
STATUS bspRelayCpeShow()
{
   UINT32 num, i;
   if(NvRamDataAddr->Relay_WcpeEid_falg!=0xa5a5a5a5)
   {
       printf("not relay cpe\n");
	return 0;
   }
   if(NvRamDataAddr->Relay_num>20)
   {
       printf("Relay_num: %d,  error!!\n", NvRamDataAddr->Relay_num);
	return 0;
   }   
    printf("There are %d Current Relay CPE,  EID:", NvRamDataAddr->Relay_num);
    num = NvRamDataAddr->Relay_num;
    if(num<=10)
    {
        for(i=0; i<num; i++)
            printf("0x%x, ", NvRamDataAddr->Relay_WcpeEid[i]);
    }
    else
    {
        for(i=0; i<10; i++)
            printf("0x%x, ", NvRamDataAddr->Relay_WcpeEid[i]);
	 for(i=0; i<num-10; i++)
            printf("0x%x, ", NvRamDataAddr->Relay_WcpeEid_New[i]);
    }
    printf("\n");
    
    return OK;
}



void init_Relay_WcpeEid()
{
    int i;
    if(NvRamDataAddr->Relay_WcpeEid_falg == 0xa5a5a5a5)
    {
        for(i = 0;i<10;i++)
        {
            RelayWanifCpeEid[i] = NvRamDataAddr->Relay_WcpeEid[i];
        }
        for(i = 0;i<10;i++)
        {
            RelayWanifCpeEid[i+10] = NvRamDataAddr->Relay_WcpeEid_New[i];
        }
    }
    else
        memset(RelayWanifCpeEid, 0, 20*sizeof(UINT32));
}

#endif
#ifndef WBBU_CODE
void CTaskCfg::CM_SavePwrCpuCtrl(CMessage &rMsg)
{
	UINT16 usRsltOK = L2_L3_SAVE_POWER_SUCCESS;
	UINT16 usRsltFail = L2_L3_SAVE_POWER_FAIL;
	if( ! m_bSavePowerModel )
		return;
	if( L2_L3_SAVE_POWER_ENABLE == (*(UINT16*)rMsg.GetDataPtr()) )
	{
		vxHid1Set(SAVE_POWER_CPU_ENABLE);
		OAM_LOGSTR1(RPT_LOG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_L2_L3_SAVEPOWER_CPU_REQ enable: 0x00416080 ?= [%08X]", vxHid1Get());
		if( SAVE_POWER_CPU_ENABLE_RESULT == vxHid1Get() )
			CM_SendComMsg( M_TID_L2MAIN, M_L3_L2_SAVEPOWER_CPU_RSP, (UINT8*)&usRsltOK, 2);
		else
			CM_SendComMsg( M_TID_L2MAIN, M_L3_L2_SAVEPOWER_CPU_RSP, (UINT8*)&usRsltFail, 2);
	}
	if( L2_L3_SAVE_POWER_DISABLE == (*(UINT16*)rMsg.GetDataPtr()) )
	{
		vxHid1Set(SAVE_POWER_CPU_DISABLE);
		OAM_LOGSTR1(RPT_LOG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:M_L2_L3_SAVEPOWER_CPU_REQ disable: 0x00015080 ?= [%08X]", vxHid1Get());
		if( SAVE_POWER_CPU_DISABLE == vxHid1Get() )
			CM_SendComMsg( M_TID_L2MAIN, M_L3_L2_SAVEPOWER_CPU_RSP, (UINT8*)&usRsltOK, 2);
		else
			CM_SendComMsg( M_TID_L2MAIN, M_L3_L2_SAVEPOWER_CPU_RSP, (UINT8*)&usRsltFail, 2);
	}
}
void CTaskCfg::CM_SavePwrFanCtrl()
{
	int temperature = bspGetBTSTemperature();
	UINT8 ucFanCnt = 0;
	if( temperature >= NvRamDataAddr->SavePwr.Fan4 )
		ucFanCnt = 4;
	else if( temperature >= NvRamDataAddr->SavePwr.Fan3 )
		ucFanCnt = 3;
	else if( temperature >= NvRamDataAddr->SavePwr.Fan2 )
		ucFanCnt = 2;
	else
		ucFanCnt = 1;
	if( m_ucFanCnt == ucFanCnt )
		return;
	m_ucFanCnt = ucFanCnt;
	UINT16 us[2];
	us[0] = SAVE_POWER_L2_L3_TRANSID;
	us[1] = m_ucFanCnt;
	CM_SendComMsg( M_TID_L2MAIN, M_L3_L2_SAVEPOWER_FAN_REQ, (UINT8*)&us, 4 );
	OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] CM_SavePwrFanCtrl Open Fans counter[%04d]", m_ucFanCnt );
/*	switch (m_ucFanCnt)
	{
		case 4:
			OAM_LOGSTR(RPT_LOG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_SavePwrFanCtrl Open Fan[4]");
		case 3:
			OAM_LOGSTR(RPT_LOG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_SavePwrFanCtrl Open Fan[3]");
		case 2:
			OAM_LOGSTR(RPT_LOG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_SavePwrFanCtrl Open Fan[2]");
		case 1: 
			OAM_LOGSTR(RPT_LOG, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] ProcessMessage:CM_SavePwrFanCtrl Open Fan[1]");
	}
*/	
}

bool CTaskCfg :: CM_SavePwrExitModel()
{
	m_bSavePowerModel = false;
	m_pFanTimer->Stop();
	vxHid1Set( SAVE_POWER_CPU_DISABLE );
	m_ucFanCnt = 0;
	m_bSavePwrInitL2Req = false;
	UINT16 us[2];
	us[0] = SAVE_POWER_L2_L3_TRANSID;
	us[1] = 4;
	CM_SendComMsg( M_TID_L2MAIN, M_L3_L2_SAVEPOWER_FAN_REQ, (UINT8*)&us, 4 );
	OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCfg] CM_SavePwrExitModel exit savepwr modle");
}
bool CTaskCfg :: CM_SavePwrCfg()
{
	m_bSavePwrInitL2Req = true;
	CCfgSavePowerReq CfgReq;
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(MSGID_L3_L2_SAVEPWR_CFG_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);
    CfgReq.SetValue( (UINT8*)&NvRamDataAddr->SavePwr.SavePwrFlag );
    OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_REV_MSG, "\n[CFG] CM_SavePwrCfg send save power cfg to l2....... \n");
    return CM_InitSendCfgReq(CfgReq);     
}
bool CTaskCfg :: CM_SavePwrSupport()
{
	UINT32 ul = vxHid1Get();
	if( SAVE_POWER_CPU_ENABLE==ul || SAVE_POWER_CPU_ENABLE_RESULT==ul || SAVE_POWER_CPU_DISABLE==ul )
		return true;
	else
		return false;
}
#endif
//#ifdef LJF_WCPE_BOOT
#ifdef RCPE_SWITCH
bool mrcpe( UINT8 uc ) 
{
	switch (uc)
	{
		case 1:
			CTaskCfg::GetInstance()->CM_RcpeClear();
			break;
		case 4:
			CTaskCfg::GetInstance()->CM_PrintMRcpeInfo();
			break;
		default:
			printf( "\r\n1: clear all rcpe and rcpe+ from nvram");
			printf( "\r\n4: print all mrcpe");
			break;
	}
	return true;
}
bool CTaskCfg :: CM_PrintMRcpeInfo() 
{
	printf( "\r\nMRCPE num[%d]", m_stTrnukMRCpe.usNum );
	printf( "\r\nMRCPE [0:%08x] [1:%08x] [2:%08x] [3:%08x] [4:%08x]", m_stTrnukMRCpe.aulMRCpe[0],m_stTrnukMRCpe.aulMRCpe[1],m_stTrnukMRCpe.aulMRCpe[2],m_stTrnukMRCpe.aulMRCpe[3],m_stTrnukMRCpe.aulMRCpe[4] );
	printf( "\r\nMRCPE [5:%08x] [6:%08x] [7:%08x] [8:%08x] [9:%08x]", m_stTrnukMRCpe.aulMRCpe[5],m_stTrnukMRCpe.aulMRCpe[6],m_stTrnukMRCpe.aulMRCpe[7],m_stTrnukMRCpe.aulMRCpe[8],m_stTrnukMRCpe.aulMRCpe[9] );
	return true;
}
bool CTaskCfg :: CM_RcpeClear() 
{
	UINT32 aul[11];
	memset( (UINT8*)aul, 0, 10*sizeof(UINT32) );
	memset( (UINT8*)&RelayWanifCpeEid, 0, 20*sizeof(UINT32) );
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid_falg, (char*)aul, sizeof(UINT32) );
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_num, (char*)aul, sizeof(UINT32) );
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid, (char*)aul, 10*sizeof(UINT32) );
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid_New, (char*)aul, 10*sizeof(UINT32) );
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->stTrunkMRCpe, (char*)aul, sizeof(TrunkMRCpeRcd) );
	memset( (UINT8*)&m_stTrnukMRCpe, 0, sizeof(TrunkMRCpeRcd) );
	return true;
}
bool CTaskCfg :: CM_RcpeSwitchCfg( CMessage& rMsg ) 
{//需要考虑修改rcpe+的问题
	//ems对rcpe的配置是以两条消息同时发送的方式进行的，故不需要进行修正配置,关闭usRcpeNumOld
	//UINT16 usRcpeNumOld = m_stTrnukMRCpe.usNum;
	TrunkMRCpeRcd* pstMRcpeReq = (TrunkMRCpeRcd*)rMsg.GetDataPtr();
    OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_REV_ERR_MSG, "[tCfg] Receive msg M_EMS_BTS_TRUNK_RCPE_CFG_REQ[0x07C7]" );
    OAM_LOGSTR2(LOG_DEBUG3, L3CM_ERROR_REV_ERR_MSG, "[tCfg] pstMRcpeReq->usNum[%d] NvRamDataAddr->Relay_num[%d]",
		pstMRcpeReq->usNum,
		NvRamDataAddr->Relay_num );
	UINT32 ucRcpeTotal = pstMRcpeReq->usNum + NvRamDataAddr->Relay_num/* - usRcpeNumOld*/;
	UINT8 ucRcpeWriteToNvram;
	if( 20 < ucRcpeTotal )
	{
		ucRcpeWriteToNvram = 20 -NvRamDataAddr->Relay_num/* + usRcpeNumOld*/;
		ucRcpeTotal = 20;
        OAM_LOGSTR(LOG_WARN, L3CM_ERROR_REV_ERR_MSG, "[tCfg] NUM(RCPE + RCPE+)=ucRcpeTotal > 20" );
	}
	else
		ucRcpeWriteToNvram = pstMRcpeReq->usNum;		

	memset( (char*)&m_stTrnukMRCpe, 0, sizeof( TrunkMRCpeRcd ) );
/*	if( 0 == pstMRcpeReq->usNum )
	{
		m_stTrnukMRCpe.usflag = M_TRUNK_MRCPE_FLAG;
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->stTrunkMRCpe, (char*)&m_stTrnukMRCpe, sizeof(TrunkMRCpeRcd) );
		memset( (char*)rMsg.GetDataPtr()+2, 0, 2 );
		CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_TRUNK_RCPE_CFG_RSP, (UINT8*)rMsg.GetDataPtr(), 4 );
		return true;
	}*/
	memcpy( (char*)&m_stTrnukMRCpe, (char*)rMsg.GetDataPtr(), 4 );
	if( 10 < pstMRcpeReq->usNum )
		pstMRcpeReq->usNum = 10;
	memcpy( (char*)&m_stTrnukMRCpe.aulMRCpe, (char*)rMsg.GetDataPtr()+4, pstMRcpeReq->usNum*sizeof(UINT32) );
	m_stTrnukMRCpe.usflag = M_TRUNK_MRCPE_FLAG;
	m_stTrnukMRCpe.usNum = pstMRcpeReq->usNum;
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->stTrunkMRCpe, (char*)&m_stTrnukMRCpe, sizeof(TrunkMRCpeRcd) );
	
	memset( (char*)rMsg.GetDataPtr()+2, 0, 2 );
	CM_SendComMsg( M_TID_EMSAGENTTX, M_BTS_EMS_TRUNK_RCPE_CFG_RSP, (UINT8*)rMsg.GetDataPtr(), 4 );

#if 1
    OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_REV_ERR_MSG, "[tCfg] Receive ucRcpeWriteToNvram[%d] ucRcpeTotal[%d]", ucRcpeWriteToNvram, ucRcpeTotal );
	memcpy( (UINT8*)(RelayWanifCpeEid+NvRamDataAddr->Relay_num/* - usRcpeNumOld*/ ), 
	 		 (UINT8*)m_stTrnukMRCpe.aulMRCpe, 
	 		 ucRcpeWriteToNvram*sizeof(UINT32) );
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_num, (char*)&ucRcpeTotal, sizeof(UINT32) );
	if( ucRcpeTotal > 10 )
 	{
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid, (char*)RelayWanifCpeEid, 10*sizeof(UINT32) );
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid_New, (char*)(RelayWanifCpeEid+10), (ucRcpeTotal-10)*sizeof(UINT32) );
 	}
	else
 	{
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid, (char*)RelayWanifCpeEid, ucRcpeTotal*sizeof(UINT32) );
 	}
#else
	l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_num, (char*)&ucRcpeTotal, sizeof(UINT16) );
	if(NvRamDataAddr->Relay_num >= 10 )
	{
		l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid_New[NvRamDataAddr->Relay_num-10], (char*)m_stTrnukMRCpe.aulMRCpe, ucRcpeWriteToNvram*sizeof(UINT32) );
	}
	else
	{
		if( 10 < ucRcpeTotal )
		{
			UINT8 ucWriteToNew = ucRcpeWriteToNvram + NvRamDataAddr->Relay_num - 10;
			UINT8 ucWriteToOld = 10 - NvRamDataAddr->Relay_num;
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid[NvRamDataAddr->Relay_num], (char*)m_stTrnukMRCpe.aulMRCpe, ucWriteToOld*sizeof(UINT32) );
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid_New, (char*)&m_stTrnukMRCpe.aulMRCpe[ucWriteToOld], ucWriteToNew*sizeof(UINT32) );
		}
		else
			l3oambspNvRamWrite( (char*)&NvRamDataAddr->Relay_WcpeEid[NvRamDataAddr->Relay_num], (char*)m_stTrnukMRCpe.aulMRCpe, ucRcpeWriteToNvram*sizeof(UINT32) );
	}

    memcpy((UINT8*)RelayWanifCpeEid, (UINT8*)NvRamDataAddr->Relay_WcpeEid, 4*10);
    memcpy((UINT8*)RelayWanifCpeEid+40, (UINT8*)NvRamDataAddr->Relay_WcpeEid_New, 4*10);
#endif
	return true;
}
#endif
void btt(UINT8 uc)
{
	UINT16 flag;
	switch (uc)
	{
		case 0:
			printf( "\r\nNvRamDataAddr->Wcpe_Switch[%04x]", NvRamDataAddr->Wcpe_Switch );
			printf( "\r\nGetSysStatus[%d]", CTaskCfg::GetInstance()->GetSysStatus());
			printf( "\r\nm_bWcpeBootModel[%d]", CTaskCfg::GetInstance()->m_bWcpeBootModel);
			printf( "\r\nm_bBootFromNV[%d]", CTaskCfg::GetInstance()->m_bBootFromNV);
			printf( "\r\nm_bNVBootFail[%d]", CTaskCfg::GetInstance()->m_bNVBootFail);
			printf( "\r\nm_bWcpeBootModelOK[%d]", CTaskCfg::GetInstance()->m_bWcpeBootModelOK);
			break;
		case 1:
			flag = 0x5a5a;
			CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Wcpe_Switch), (SINT8*)&flag, 2);
			printf( "\r\nNvRamDataAddr->Wcpe_Switch[%04x]", NvRamDataAddr->Wcpe_Switch );
			break;
		case 2:
			flag = 0;
			CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Wcpe_Switch), (SINT8*)&flag, 2);
			printf( "\r\nNvRamDataAddr->Wcpe_Switch[%04x]", NvRamDataAddr->Wcpe_Switch );
			break;		
		case 3:
		    bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
			break;
		case 4:
		    bspSetBootupSource(BTS_BOOT_DATA_SOURCE_BTS);
			break;
		default:
			printf( "\r\n para 0: print the status" );
			printf( "\r\n para 1: set NvRamDataAddr->Wcpe_Switch = 0x5a5a" );
			printf( "\r\n para 2: set NvRamDataAddr->Wcpe_Switch = 0" );
			printf( "\r\n para 3: set bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);" );
			printf( "\r\n para 4: set bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);" );
			break;
	}
}
extern "C" void EnableVlanEndInterface()
{
	g_SAG_VLAN_USAGE      = NvRamDataAddr->BtsGDataCfgEle.SAGVlanUsage;
	g_SAG_VLAN_ID         = NvRamDataAddr->BtsGDataCfgEle.SAGVlanID;          
	g_SAG_VLAN_BTSIP      = NvRamDataAddr->BtsGDataCfgEle.BtsIPAddr;
	g_SAG_VLAN_SUBNETMASK = NvRamDataAddr->BtsGDataCfgEle.SubnetMask;     
	g_SAG_VLAN_GATEWAY    = NvRamDataAddr->BtsGDataCfgEle.DefGateway; 
	if(g_SAG_VLAN_USAGE==1)
	{
		StartVlanEnd(); 
	}
}
//#ifndef WBBU_CODE
/**********************************************************************
*
*  NAME:          CalcNvramCheckSum
*  FUNTION:     
计算crc，包括打开文件
*  INPUT:          待计算地址
*  OUTPUT:       crc
  OTHERS:        jiaying20110217
*******************************************************************/
UINT32 CTaskCfg :: CalcNvramCheckSum(UINT8 *pData)
{
    UINT32 i;
    UINT32 checksum = 0, result;    
    pData += 4+sizeof(T_BTSCommonDataEle);//去掉前面一些改动大的字段
    for(i = 0; i < sizeof(T_NvRamData)-4-sizeof(T_BTSCommonDataEle); i++)
    {	    
        checksum += *(pData+ i);
    } 
    result = ~checksum + 1;
    OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] welcome to CalcNvramCheckSum, crc: %x!!", result);
    return result;
}
/**********************************************************************
*
*  NAME:          CheckNvramCfBakFile
*  FUNTION:     
检查备份文件的crc，包括打开文件，读出数据，校验crc，crc结果是文件开始四个字节
如果crc正确，修改记录结构体
*  INPUT:          文件名和文件号
*  OUTPUT:        操作是否成功
  OTHERS:        jiaying20110217
*******************************************************************/
bool CTaskCfg ::CheckNvramCfBakFile(char *filename, int fileNo)
{
     FILE *fdhead;

    OAM_LOGSTR1(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] CheckNvramCfBakFile, welcome!! file(%s)", (int)filename);	 
    /*打开文件1*/
    if((fdhead = fopen (filename, "ab+")) == (FILE *)ERROR)
    {
        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] CheckNvramCfBakFile, open ( /ata0a/nvdata/%s) error", (int)filename);	 
        return false;
    }    
    UINT32 nvramFileLen = sizeof(T_NvRamData)+4;
    if(nvramFileLen>=200000)
    {        
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] CheckNvramCfBakFile, file len > 200000, return!!");	 
        fclose(fdhead);
        return false;
    }
    UINT8*pSrcData = nvramBakFileInfo.bakFile;    
    rewind(fdhead);    
    UINT32 count = 0;   
    UINT32 nRead, wnum;    
    while (count < nvramFileLen)
    {
        if(nvramFileLen-count>4096)
            wnum = 4096;
	 else
	     wnum = nvramFileLen-count;
        nRead = fread(pSrcData, 1/*size*/, wnum, fdhead);	
	 if(nRead == 0)
	 	break;
        count += nRead;
        pSrcData+=nRead;
    }
    if(count < nvramFileLen)//数据区加上4字节的校验和
    {
        OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] CheckNvramCfBakFile, read file lenth error, len:%x, read:%x!!", nvramFileLen, count);
	 fclose(fdhead);        
	 return false;
    }
    fclose(fdhead);
    OAM_LOGSTR2(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] CheckNvramCfBakFile, read file succ, len:%x, read:%x!!", nvramFileLen, count);
    UINT32 nvrambakCRC;
    memcpy((UINT8*)&nvrambakCRC, nvramBakFileInfo.bakFile, 4);
    UINT32 countCrc = CalcNvramCheckSum(&nvramBakFileInfo.bakFile[4]);
    if(nvrambakCRC!=countCrc)
    {           
	 OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] CheckNvramCfBakFile, Crc check error,read: %x, count:%x!!", nvrambakCRC, countCrc);
        return false;
    }    
    OAM_LOGSTR(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] CheckNvramCfBakFile, pass check!!");
    if(fileNo == 1)//文件1
    {
        nvramBakFileInfo.hasBakFile1 = true;
        nvramBakFileInfo.pBakFile1Data = nvramBakFileInfo.bakFile;
    }
    else
    {
        nvramBakFileInfo.hasBakFile2 = true;
        nvramBakFileInfo.pBakFile2Data = nvramBakFileInfo.bakFile;
    }   
    
    return true;
    
}
/**********************************************************************
*
*  NAME:          NvramDataCheckandInitInfo
*  FUNTION:     
1)判断cf卡上是否有备份数据，如果有，则校验文件1，如果正确则读入内存，否则
将通过校验后的文件2读入内存。
2)校验nvram配置数据，如果校验和不通过，则从备份文件中获得数据,
3)初始化备份机制的数据结构和定时器
*  INPUT:          无
*  OUTPUT:        无
  OTHERS:        jiaying20110217
*******************************************************************/
void CTaskCfg :: NvramDataCheckandInitInfo()
{
    T_NVRAMCRCINFO nvramCrcInfo;
    int usefileFlag = 1;//file1:1, file2:2
    bool nvramdata = false;
	
    memset(&nvramBakFileInfo, 0, sizeof(T_NVRAMBAKINFO));
    //校验nvram
    memcpy((UINT8*)&nvramCrcInfo, (UINT8*)NVRAM_BASE_ADDR_OAM_DATA_CRC, sizeof(T_NVRAMCRCINFO));
    if(nvramCrcInfo.crcFlag==0xaaaabbbb)//crc flag valid
    {
        OAM_LOGSTR(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, nvram data crc flag is valid ");
        UINT32 nvramcrc = CalcNvramCheckSum((UINT8*)NVRAM_BASE_ADDR_OAM );	
	
	 if(nvramCrcInfo.crcResult == nvramcrc)
        {
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, nvram data crc is valid, boot from nvram");
            nvramBakFileInfo.bootUseFile = 0;//boot from nvram
            nvramdata = true;
        }
	 else
	     OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] nvram data crc is invalid, nvramcrc:%x, count crc:%x", nvramCrcInfo.crcResult, nvramcrc);
    }
    //首先检查cf卡上备份文件是否存在,如果出现异常，不用校验nvram，直接退出
    DIR* pdir = opendir( SM_NVRAM_CFG_DATA_BAK_DIR );   
	
    if( NULL == pdir )//如果打不开文件加，则重建一个
    {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, opendir( /ata0a/nvdatabak/ ) error");
        if( OK != mkdir( SM_NVRAM_CFG_DATA_BAK_DIR ) )
        {
            nvramBakFileInfo.bakFileAllBad = true;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, mkdir( /ata0a/nvdatabak/ ) error");
	     if(!nvramdata)
                bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
            return;//创建路径失败
        }
    }
    else
    {
        OAM_LOGSTR(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, opendir( /ata0a/nvdatabak/ ) success");
        if(OK != closedir( pdir ))
        {
            nvramBakFileInfo.bakFileAllBad = true;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, closedir() error");
            if(!nvramdata)
                bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
            return;
        }
    }
    
    if(OK != chdir( SM_NVRAM_CFG_DATA_BAK_DIR ))//如果不能转到该文件夹下，异常退出
    {
        nvramBakFileInfo.bakFileAllBad = true;
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, chdir( /ata0a/nvdatabak/ ) error");
        if(!nvramdata)
                bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
        return;
    }
    OAM_LOGSTR(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, chdir( /ata0a/nvdatabak/ ) success");
    if(CheckNvramCfBakFile(SM_NVRAM_CFG_DATA_FILENAME1, 1)==false)
    {        
        if(CheckNvramCfBakFile(SM_NVRAM_CFG_DATA_FILENAME2, 2)==false)
        {
            nvramBakFileInfo.bakFileAllBad = true;   
            if(!nvramdata)
                bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
            return;
        }
        usefileFlag = 2;
    }   
    if(nvramdata)//如果nvram数据正确，退出
        return;
    //nvram校验和不通过，将备份文件拷入nvram
    if(usefileFlag == 1)
    {
        if(nvramBakFileInfo.pBakFile1Data!=NULL)
        {
            memcpy((UINT8*)NVRAM_BASE_ADDR_OAM, nvramBakFileInfo.pBakFile1Data+4, sizeof(T_NvRamData));
            nvramBakFileInfo.bootUseFile = 1;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, nvram data crc is invalid, boot from bakfile1");
            return;
        }
	 else
	 {
	     bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
	 }
    }
    else//file2
    {
        if(nvramBakFileInfo.pBakFile2Data!=NULL)
        {
            memcpy((UINT8*)NVRAM_BASE_ADDR_OAM, nvramBakFileInfo.pBakFile2Data+4, sizeof(T_NvRamData));
            nvramBakFileInfo.bootUseFile = 2;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] NvramDataCheckandInitInfo, nvram data crc is invalid, boot from bakfile2");
            return;
        }
	 else
	 {
	     bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
	 }
    }
        
}

/**********************************************************************
*
*  NAME:          getNvramBakData
*  FUNTION:   从备份文件中获得nvram数据，重新覆盖老数据
*  INPUT:          无
*  OUTPUT:        true:获得了数据，false:no
  OTHERS:        jiaying20110217
*******************************************************************/
bool CTaskCfg :: getNvramBakData()
{
    OAM_LOGSTR(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData");
    if(nvramBakFileInfo.bakFileAllBad == true)
    {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, all bak file error!");
        return false;
    }
    if(nvramBakFileInfo.bootUseFile == 2)//boot from flie2
    {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, boot from bak2, result error!");
        return false;
    }
     //首先检查cf卡上备份文件是否存在,如果出现异常，不用校验nvram，直接退出
    DIR* pdir = opendir( SM_NVRAM_CFG_DATA_BAK_DIR );   
	
    if( NULL == pdir )//如果打不开文件加，则重建一个
    {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, opendir( /ata0a/nvdatabak/ ) error");
        return false;
    }
    else
    {
        OAM_LOGSTR(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, opendir( /ata0a/nvdatabak/ ) success");
        if(OK != closedir( pdir ))
        {
            nvramBakFileInfo.bakFileAllBad = true;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, closedir() error");
            return false;
        }
    }
    
    if(OK != chdir( SM_NVRAM_CFG_DATA_BAK_DIR ))//如果不能转到该文件夹下，异常退出
    {
        nvramBakFileInfo.bakFileAllBad = true;
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, chdir( /ata0a/nvdatabak/ ) error");
        return false;
    }
    OAM_LOGSTR(LOG_DEBUG2, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, chdir( /ata0a/nvdatabak/ ) success");
    if(nvramBakFileInfo.bootUseFile == 1)//boot from flie1
    {         
        if(CheckNvramCfBakFile(SM_NVRAM_CFG_DATA_FILENAME2, 2)==false)
        {
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, bakfile2 error!");
            return false;
        }
        else
        {
            memcpy((UINT8*)NVRAM_BASE_ADDR_OAM, nvramBakFileInfo.pBakFile2Data+4, sizeof(T_NvRamData));
            nvramBakFileInfo.bootUseFile = 2;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, get file from bakfile2!");
            return true;
        }
    }
    else if(nvramBakFileInfo.bootUseFile == 0)//boot from nvram
    {
        if(nvramBakFileInfo.pBakFile1Data!=NULL)//bakfile1 valid
        {
            memcpy((UINT8*)NVRAM_BASE_ADDR_OAM, nvramBakFileInfo.pBakFile1Data+4, sizeof(T_NvRamData));
            nvramBakFileInfo.bootUseFile = 1;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, nvram data crc is invalid, boot from bakfile1");
            return true;
        }
        else //bakfile1 invalid
        {        
            if((nvramBakFileInfo.pBakFile2Data==NULL)&&(CheckNvramCfBakFile(SM_NVRAM_CFG_DATA_FILENAME2, 2)==false))
            {
                OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, bakfile2 error!");
                return false;
            }        
            memcpy((UINT8*)NVRAM_BASE_ADDR_OAM, nvramBakFileInfo.pBakFile2Data+4, sizeof(T_NvRamData));
            nvramBakFileInfo.bootUseFile = 2;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] getNvramBakData, get file from bakfile2!");
            return true;        
        }
    }
}

/**********************************************************************
*
*  NAME:          writeNvramToBakfile
*  FUNTION:   将数据写入备份区域，如果是文件2，将备份整个配置数据
*  INPUT:          filename: 
                        srcData
                        len:需要写入文件长度
                        nvramCrc:nvram CRC
                        fileNo:文件1还是2
                        
*  OUTPUT:        true:获得了数据，false:no
  OTHERS:        jiaying20110217
*******************************************************************/
bool writeNvramToBakfile(char *filename, char *srcData, UINT32 len,  int fileNo)
{
    if((filename==NULL)||(srcData==NULL))
    {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, para error!!");
        return false;
    }
    DIR* pdir = opendir( SM_NVRAM_CFG_DATA_BAK_DIR );   
	
    if( NULL == pdir )//如果打不开文件加，则重建一个
    {
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, opendir( /ata0a/nvdatabak/ ) error");
        if( OK != mkdir( SM_NVRAM_CFG_DATA_BAK_DIR ) )
        {
            nvramBakFileInfo.bakFileAllBad = true;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, mkdir( /ata0a/nvdatabak/ ) error");
            return false;//创建路径失败
        }
    }
    else
    {
        OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, opendir( /ata0a/nvdatabak/ ) success");
        if(OK != closedir( pdir ))
        {
            nvramBakFileInfo.bakFileAllBad = true;
            OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, closedir() error");
            return false;
        }
    }
    
    if(OK != chdir( SM_NVRAM_CFG_DATA_BAK_DIR ))//如果不能转到该文件夹下，异常退出
    {
        nvramBakFileInfo.bakFileAllBad = true;
        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, chdir( /ata0a/nvdatabak/ ) error");
        return false;
    }
     FILE *fdhead;
     
    /*打开文件1*/
    if((fdhead = fopen (filename, "wb")) == (FILE *)ERROR)
    {
        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, open ( /ata0a/nvdata/%s) error", (int)filename);	 
        return false;
    }     
    rewind(fdhead); 
       
    int nBytesw, wnum, ncount = 0;
    
    char * ptr = srcData;
    while(ncount<len)
    {   
        if(len-ncount>4096)
            wnum = 4096;
        else
            wnum = len-ncount;
        nBytesw = fwrite(ptr, 1, wnum, fdhead); 	
        if(nBytesw==0)
            break;
        fflush(fdhead);
        ncount += nBytesw;  
        ptr += nBytesw; 
    }    
    fclose(fdhead);	   
    
    if(ncount >= len)
    {
        if(fileNo == 1)
            nvramBakFileInfo.hasBakFile1 = true;
        else
            nvramBakFileInfo.hasBakFile2 = true;
        nvramBakFileInfo.bakFileAllBad = false;
        OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, %s write succ", (int)filename);
        return true;
    }
    else
    {
        OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCM] writeNvramToBakfile, %s write len error, len:%d, write:%d", (int)filename, len, ncount);
        return false;
    }
}

void showNvramInfo()
{
    printf("\nnvramBakFileInfo.hasBakFile1: %d", nvramBakFileInfo.hasBakFile1);
    printf("\nnvramBakFileInfo.hasBakFile2: %d", nvramBakFileInfo.hasBakFile2);
    printf("\nnvramBakFileInfo.cfgDataChg: %d", nvramBakFileInfo.cfgDataChg);
    printf("\nnvramBakFileInfo.bakFileAllBad: %d", nvramBakFileInfo.bakFileAllBad);
    printf("\nnvramBakFileInfo.bootUseFile: %d", nvramBakFileInfo.bootUseFile);    
    printf("\n");
}

void showNvramCrc()
{
    T_NVRAMCRCINFO nvramCrcInfo;
	
    memcpy((UINT8*)&nvramCrcInfo, (UINT8*)NVRAM_BASE_ADDR_OAM_DATA_CRC, sizeof(T_NVRAMCRCINFO));
    printf("\n nvramCrcInfo.crcFlag: %x", nvramCrcInfo.crcFlag);
    printf("\n nvramCrcInfo.crcResult: %x", nvramCrcInfo.crcResult);
    printf("\n");
}

void checkNvramBak()
{
    /////////////////
    CTaskCfg::GetInstance()->CheckNvramCfBakFile(SM_NVRAM_CFG_DATA_FILENAME1, 1);
    CTaskCfg::GetInstance()->CheckNvramCfBakFile(SM_NVRAM_CFG_DATA_FILENAME2, 2);
	///////////////////
}
//#endif
/************
设置组播开关1----打开，0或者其他关闭


**************/

#ifndef WBBU_CODE
extern "C" void open_multicast();
extern "C" void close_multicast();
extern "C"  void bspSetMulticast(unsigned char flag)
{
    T_Multicast_CFG   para;
    if(flag==1)
    {
       para.valid = 0x87654321;
	para.flag = 0x55;
	open_multicast();
    }
   else
   {
       para.valid = 0x87654321;
	para.flag = 0xaa;
	close_multicast();
   }
   CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->MulticastPara), (SINT8*)&para, sizeof(T_Multicast_CFG));
}

extern "C" void callMulticast()
{
     if(NvRamDataAddr->MulticastPara.valid==0x87654321)
     	{
     	    if(NvRamDataAddr->MulticastPara.flag==0x55)
     	    	{
     	    	    open_multicast();
     	    	}
		else
		{
		    close_multicast();
		}
     	}
}
#endif
void CTaskCfg ::sendGrpLmtPara()
{
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->clusterNumLimit);
    if(NvRamDataAddr->clusterNumLimit.cfg_flag!=0x5a5a)
    {
        T_CLUSTERNUMLMT clusterLimit_temp;
        clusterLimit_temp.grpUserNumUpperLimit = 30;
        clusterLimit_temp.grpPagingRspLimit = 30;
        clusterLimit_temp.cfg_flag = 0x5a5a;
        l3oambspNvRamWrite(pCfgEle, (SINT8*)&clusterLimit_temp, sizeof(T_CLUSTERNUMLMT));
    }	
    CCfgClusterNumLmtReq CfgReq;	     
    CfgReq.CreateMessage(*this);
    CfgReq.SetMessageId(MSGID_L3_L2_GRP_LIMIT_CFG);
    CfgReq.SetDstTid(M_TID_L2MAIN);
    CfgReq.SetEle(pCfgEle, sizeof(T_CLUSTERNUMLMT)-2);    
    CM_InitSendCfgReq(CfgReq);  
    
}
extern "C"void setGrpLmtPara(UINT16 grpNumLimit, UINT16 grpPagingRspLimit)
{
    SINT8 *pCfgEle = (SINT8 *)&(NvRamDataAddr->clusterNumLimit);
	     
    T_CLUSTERNUMLMT clusterLimit_temp;
    clusterLimit_temp.grpUserNumUpperLimit = grpNumLimit;
    clusterLimit_temp.grpPagingRspLimit = grpPagingRspLimit;
    clusterLimit_temp.cfg_flag = 0x5a5a;
    CTaskCfg::l3oambspNvRamWrite(pCfgEle, (SINT8*)&clusterLimit_temp, sizeof(T_CLUSTERNUMLMT));
    CTaskCfg::GetInstance()->sendGrpLmtPara();
}
#ifdef WBBU_CODE

extern "C" void SetFpga_Para()
{
          SetTsMode(NvRamDataAddr->AirLinkCfgEle.DLTSNum,1,0);//add 20100128
           T_L1GenCfgEle *pCfgEle = (T_L1GenCfgEle *)&(NvRamDataAddr->L1GenCfgEle);
           
              if(pCfgEle->SyncSrc == 1)
            	{
            	   Set_Fpga_Clk(2);
            	   SetOffset(pCfgEle->GpsOffset);//add 20100202
            	}
            else if(pCfgEle->SyncSrc == 0)
            	{
            	    Set_Fpga_Clk(0);
            	}
		else
		{
		    Set_Fpga_Clk(3);
		}
}


/******************************

当此功能打开时，当检测到DSP不正常时，配置一组默认的值

系统默认打开；当没有配置时

******************/
extern "C"     void bspSetProtectDSP2(unsigned char flag)
{
     unsigned int   tag =0; 
    if(flag==1)
    {
      tag = 0x5a5a5a5a;
	
    }
   else
   {
      tag =0xa5a5a5a5;
   }
   CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->DSP2_Except_Protect), (SINT8*)&tag, sizeof(int));
}
extern "C"  unsigned char  IfDSP2Protect()
{
        if(NvRamDataAddr->DSP2_Except_Protect==0xa5a5a5a5)
        	{
        	    return 0;
				
        	}
		else
		{
		      return 1;
		}
}
/********************************

 DSP增加的一些保存信息的功能是否需要？三层做个接口，默认关闭；
 diag rpc,core9,0x501,1,1  关闭， diag rpc,core9,0x500,1,1打开

***********************************/
extern "C"  void Set_DSP4_info(unsigned int flag);
extern "C"     void bspSetDSP4SaveInfo(unsigned char flag)
{
     unsigned int   tag =0; 
    if(flag==1)
    {
      tag = 0x5a5a5a5a;
	Set_DSP4_info(1);
    }
   else
   {
      tag =0xa5a5a5a5;
	 Set_DSP4_info(0);
   }
   CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->Core9_Info_Save_Flag), (SINT8*)&tag, sizeof(int));
}

extern "C"  void  SetDsp4SaveInfo()
{
  
    if(NvRamDataAddr->Core9_Info_Save_Flag==0x5a5a5a5a)
    	{
    	    Set_DSP4_info(1);
	 /*  printf("SetDsp4SaveInfo called\n");*/
    	}
	else
	{
	    Set_DSP4_info(0);
	   /*  printf("CloseDsp4SaveInfo called\n");*/
	}
}

#endif

/***********************************************************************************************************
为了基站能够灵活使用关闭射频功能，在网管增加基站关闭射频的配置项：
0)	不启用关射频功能；（基站默认不启用关射频功能）
1)	启用数据传输断关射频功能；
2)	启用语音传输断关射频功能；
3)	启用数据/语音同时断关射频功能；
4)	启用数据语音之一断关射频功能。
如果配置为启用关射频功能，还需要配置如下时长：
1)	关射频保护时长（含义：基站检测出传输异常后多长时间才关闭射频）：默认60秒
2)	恢复射频保护时长（含义：基站检测出传输正常后多长时间才打开射频）：默认30秒
3)	数据网关1的IP地址
4)	数据网关2的IP地址
*******************************************************************************************************************/
extern "C" void SetRF(unsigned int type,unsigned int close_time_len,unsigned int open_time_len,unsigned int gatewayip1,unsigned int gatewayip2)
{
    RF_Operation  temp_rf;
	temp_rf.flag = 0x55aa55aa;
     if(type>4)
     {
       	 temp_rf.type = 0;
     }
     else
     	{
     	   	temp_rf.type = type;
     	}
	if(close_time_len<60)
	{
	  	temp_rf.Close_RF_Time_Len = 60;
	}
	else
	{
	      temp_rf.Close_RF_Time_Len = close_time_len;
	}
	if(open_time_len<30)
	{
	     temp_rf.Open_RF_Time_Len = 30;
	}
	else
		{
		   temp_rf.Open_RF_Time_Len = open_time_len;
		}
	temp_rf.GateWayIP1 = gatewayip1;
	temp_rf.GateWayIP2 = gatewayip2;
	if(gatewayip1!= NvRamDataAddr->rf_operation.GateWayIP1)
	{
	     g_rf_openation.GateWay1_valid = 0;
	    memset(g_rf_openation.GateWay1_MAC,0,6);
	    g_rf_openation.Gateway1_bad_time =0;
	     g_rf_openation.GateWay1_UP= 0;
           g_rf_openation.GateWay1_down = 0;

	}
	if(gatewayip2!= NvRamDataAddr->rf_operation.GateWayIP2)
	{
	     g_rf_openation.GateWay2_valid = 0;
	    memset(g_rf_openation.GateWay2_MAC,0,6);
	    	    g_rf_openation.Gateway2_bad_time =0;
	     g_rf_openation.GateWay2_UP= 0;
           g_rf_openation.GateWay2_down = 0;

	}
	   CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->rf_operation), (SINT8*)&temp_rf, sizeof(RF_Operation));
        if(NvRamDataAddr->Wcpe_Switch==0x5a5a)
      	{
      	     g_rf_openation.type = 0;
	    printf("SetRF:curent mode is wcpe the RF CFG disabled\n");
      	}
	else
	{
	   g_rf_openation.type = NvRamDataAddr->rf_operation.type;
	}
	   g_rf_openation.Close_RF_Time_Len = NvRamDataAddr->rf_operation.Close_RF_Time_Len;
	   g_rf_openation.Open_RF_Time_Len = NvRamDataAddr->rf_operation.Open_RF_Time_Len;
	   g_rf_openation.GateWayIP1 = NvRamDataAddr->rf_operation.GateWayIP1;
	   g_rf_openation.GateWayIP2 = NvRamDataAddr->rf_operation.GateWayIP2;

	 //  g_rf_openation.Gateway1_bad_time =0;
	//   g_rf_openation.Gateway2_bad_time =0;
	   g_rf_openation.Voice_Bad_Time = 0;
	    printf("%x,%x,%x,%x,%x,%x\n",g_rf_openation.type,g_rf_openation.Close_RF_Time_Len,g_rf_openation.Open_RF_Time_Len ,  g_rf_openation.GateWayIP1,g_rf_openation.GateWayIP2 ,0);
	   return;
}
