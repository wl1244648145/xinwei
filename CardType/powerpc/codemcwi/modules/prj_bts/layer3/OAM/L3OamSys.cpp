/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3OamSystem.cpp
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifdef __WIN32_SIM__
    #ifndef _WINSOCK2API_
    #include <winsock2.h>
    #endif
    
    #ifndef _WINDOWS_
    #include <windows.h>
    #endif
    #ifndef _INC_STRING
    #endif
#else //Vxworks
    #include <vxworks.h>
    #include <sockLib.h>
    #include <inetLib.h>
    #include <hostLib.h>
    #include <ioLib.h>
    #include <fioLib.h>
    #include <rebootLib.h> 
    #include <stdio.h>
    #include "mcWill_bts.h"
#endif

#include <stdio.h>
#include <string.h>

#ifndef _INC_MSGQUEUE
#include "MsgQueue.h"
#endif

#ifndef _INC_L3OAMTEST
//#include "L3OamTest.h"
#endif


#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_ERRORCODEDEF
#include "ErrorCodeDef.h"
#endif

#ifndef _INC_L3OAMSYSTEM
#include "L3OamSystem.h"
#endif

#ifndef _INC_L3OAMALMINFO
#include "L3OamAlmInfo.h"
#endif

#ifndef INC_OAML3ALM
#include "L3OamAlm.h"
#endif

#ifndef INC_OAML3GPS
#include "L3OamGps.h"
#endif

#ifndef  _INC_L3OAMCFG
#include "L3OamCfg.h"
#endif

#ifndef _INC_L3OAMFILE
#include "L3OamFile.h"
#endif

#ifndef _INC_L3OAMFTPCLIENT
#include "L3OamFtpClient.h"
#endif

#ifndef _INC_OAML3CPEM
#include "L3OamCpeM.h"
#endif

#ifndef LOAD_VERSION_H
#ifndef WBBU_CODE
#include "loadVersion.h"
#else
#include "loadVersion_WBBU.h"
#endif
#endif

#ifndef __WIN32_SIM__      //__WIN32_SIM__
#ifndef _INC_L3_BTS_PM
#include "L3_BTS_PM.h"
#endif
#endif //__WIN32_SIM__

#ifndef _INC_L3OAMDIAG
#include "L3OamDiag.h"
#endif


////#include "L3L3L2BtsConfigSYNPower.h"
#include "sysBtsConfigData.h"

#include "L3DataNotifyBTSPubIP.h"
#include "L3DataSocket.h"
#ifdef WBBU_CODE
#include "L3WRRU.h"
#endif
//////////////////////
#ifdef __WIN32_SIM__
FILE *pgL3oamLogFile = 0;
#else
SINT32 pgL3oamLogFile = 0;
#endif
//sag故障弱化功能添加
#define    SM_USR_BOOT_DIR    "/ata0a/usr/"  
LOCAL char userACL[32] = "/ata0a/usr/userACL.csv";
LOCAL char userInfo[32] = "/ata0a/usr/userInfo.csv";
LOCAL char GrpInfo[32] = "/ata0a/usr/GrpInfo.csv";
LOCAL char GrpUserInfo[32] = "/ata0a/usr/GrpUserInfo.csv";
T_UserFileRecord userFileRecord;

extern bool AlarmReport(UINT8   Flag, UINT16  EntityType,
                 UINT16  EntityIndex, UINT16  AlarmCode,      
                 UINT8   Severity, const char format[],...);

extern T_NvRamData *NvRamDataAddr;
extern bool bgRedirectLog;
extern T_DiagToolRecs  gDiagToolRecs;
extern UINT8  gBtsVisbleSatcnt;
extern UINT8  gBtstrackSatcnt;
extern "C" int bspGetMainEmsIpAddr();
extern "C" int bspGetMainEmsUDPRcvPort();
extern "C" int bspGetBakEmsIpAddr();
extern "C" int bspGetBakEmsUDPRcvPort();
#ifdef WBBU_CODE
extern "C"  unsigned short  Read_Fpga_Alarm();
extern "C" unsigned int Judge_Aif_Status(unsigned char index);
// extern "C" void Reset_Dsp(unsigned char index,unsigned char flag);
 extern "C" unsigned short  ReadTemp(unsigned char port,unsigned char offset);
 extern "C" void WrruRFC(unsigned char antennamask,unsigned char flag,unsigned char flag1);
  extern "C" void L3_APP_Run();
 extern "C" void sendAnntenaMsk(unsigned short  antennamask,unsigned char flag,unsigned char flag1);
extern unsigned char  L2_BOOT_FLAG;
volatile UINT16 FGPA_Alarm_Type = 0x1ff;/*****初始状态为正常*****/
bool notifyAllCPEtoRegister();
extern "C" unsigned char  Reset_Dsp(unsigned char index,unsigned char flag);
extern "C" void ClearResetReasonSet();//wangwenhua add 20110429
extern "C" void ReadRRUFiberInfo(unsigned char flag);
extern "C" void ReadBBUFiberInfo(unsigned char fiber_no);
extern"C"  void ShowBBUFiberInfo(unsigned char flag);
extern "C" void    ResetAifSerdies();
#endif

extern "C" void rtcDateSet(int year, int month,  int dayOfMonth, int dayOfWeek);
extern "C" void rtcTimeSet(int hour, int minute, int second);

#ifdef WBBU_CODE
extern "C" void bspSetTime(char*pbuf);
#endif

#ifndef WBBU_CODE
extern "C" int startUiTask();
extern "C" void callMulticast();//wangwenhua add 20110510

#endif
#ifdef M_TGT_WANIF
extern "C" void SetLocalEms(unsigned char  flag);
extern UINT16   Wanif_Switch ;
#endif
CTaskSystem* CTaskSystem:: m_Instance = NULL;
T_BtsSysStatusData gl3oamBtsSysStatusData;

#ifdef WBBU_CODE

extern UINT8     bGpsStatus_RF;
extern "C" void FanControl(unsigned char flag);
extern unsigned char  Hard_VerSion ;
#endif
/*
 *gVx_System_Clock维护系统启动持续运行的秒数。
 *不是很精确的
 */
UINT32 gVx_System_Clock = 0;    /*系统持续运行的时间(秒)*/

/*
 *EMS采用主备用，主用EMS优先使用的方式
 */
volatile UINT32 gActiveEMSip    = 0;    //目前使用的ems.可能是主用，可能是备用，可能是0
volatile UINT16 gActiveEMSport  = 0;

const UINT8 strCPUType[7][10] = {"L2","AUX","MCP","FEP","L1","BTS","FPGA"};

//wangwenhua add 2012-4-26  
//针对加蓬反馈问题，ARP表没有free ，导致ARP增加不入，用户不能上网，投诉

//加强统计和审核，当出现问题时，产生告警，给EMS
extern unsigned int g_eb_no_ft_freelist ;
extern unsigned int g_eb_no_cdr_freelist ;
extern unsigned int g_socket_no_ft_freelist ;
extern unsigned int g_socket_no_CB_freelist ;
extern unsigned int g_snoop_no_freelist ;
 extern unsigned int g_arp_no_freelist ;
 extern  unsigned int g_dm_no_freelist ;
 extern unsigned int g_cdr_btree_err ;
void l3oamprintbtssysstatus()
{
    /////////////////////////////////////////////////
    gl3oamBtsSysStatusData.SatellitesVisible = gBtsVisbleSatcnt;
    gl3oamBtsSysStatusData.StatellitesTrack  = gBtstrackSatcnt;
    gl3oamBtsSysStatusData.BootSrc           = bspGetBootupSource() - 30;
    gl3oamBtsSysStatusData.RfMask            = NvRamDataAddr->L1GenCfgEle.AntennaMask;
    //gBtsSysStatusData.SyncCardTemperature = gSyncCardTemperature; //      当前频踪板温度
    gl3oamBtsSysStatusData.DigitalBoardTemperature = bspGetBTSTemperature();//当前主用板温度
////UINT32 temp = GetSystemTime();
    gl3oamBtsSysStatusData.TimePassedSinceBtsBoot = gVx_System_Clock/*temp - gbtsBootSecond*/;

    printf( "SatellitesVisible       = %d\r\n"  
            "StatellitesTrack        = %d\r\n"  
            "BootSrc                 = %d\r\n"
            "RfMask                  = 0X%04X\r\n"
            "SyncCardTemperature     = %d\r\n"
            "DigitalBoardTemperature = %d\r\n"
            "TimePassedSinceBtsBoot  = %d seconds\r\n", 
            gl3oamBtsSysStatusData.SatellitesVisible,   
            gl3oamBtsSysStatusData.StatellitesTrack,    
            gl3oamBtsSysStatusData.BootSrc,
            gl3oamBtsSysStatusData.RfMask,
            gl3oamBtsSysStatusData.SyncCardTemperature,
            gl3oamBtsSysStatusData.DigitalBoardTemperature,
            gl3oamBtsSysStatusData.TimePassedSinceBtsBoot
        );
}


UINT32 l3oamgetactiveversion()
{
    char version1[40];
    memset(version1, 0, sizeof(version1));
    memcpy(version1, VERSION, strlen(VERSION));
    return inet_addr(&(version1[7]));
}

int bspGetSMFlag()
{
    return NvRamDataAddr->BTSCommonDataEle.BtsBootPlane;
}

#if 0
void l3oamprintswversion()
{
    OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS software version: %s", (UINT32)VERSION);
    //////////////////////////////////////////////////////////////////
    if(0 == bspGetWorkFlag())  //试运行
    {
        int setboot = bspGetSMFlag();
        int boot    = bspGetBootPlane (); 
        if(setboot == boot)  //启动平面同设置启动平面一致，启动成功
        {
            /* 软件试运行成功, 清除试运行和试运行记录标志位*/
            OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS boot from plane[%d], BTS upgrade succeed!", setboot);
        }
        else
        {
            OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS boot from plane[%d], BTS upgrade failed!", setboot);
        }
    }
    else // 不是软件试运行引导
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS boot normal!");
    }
    ////////////////////////////////////////////////////////////////////
}

#endif 
//for telnet output -- sunshanggu 080619
void l3oamprintswversion()
{
	char tmpStr[50] = "BWA ";
	char tmpMidStr[50];
	strcpy( tmpMidStr, VERSION );
	strcat( tmpStr, tmpMidStr+7 );
    printf("[tSys] BTS software version: %s", tmpStr);
    //////////////////////////////////////////////////////////////////
    if(0 == bspGetWorkFlag())  //试运行
    {
        int setboot = bspGetSMFlag();
        int boot    = bspGetBootPlane (); 
        if(setboot == boot)  //启动平面同设置启动平面一致，启动成功
        {
            /* 软件试运行成功, 清除试运行和试运行记录标志位*/
            printf("[tSys] BTS boot from plane[%d], BTS upgrade succeed!\n", setboot);
        }
        else
        {
            printf("[tSys] BTS boot from plane[%d], BTS upgrade failed!\n", setboot);
        }
    }
    else // 不是软件试运行引导
    {
        printf("[tSys] BTS boot normal!\n");
    }
    ////////////////////////////////////////////////////////////////////
}

void CTaskSystem :: BtsSwUpdateCheck()
{
    if(LOAD_STATUS_TEST_RUN == bspGetWorkFlag())  //试运行
    {
        int setboot = bspGetSMFlag();
        int boot    = bspGetBootPlane (); 
        if(setboot == boot)  //启动平面同设置启动平面一致，启动成功
        {
            // 软件试运行成功, 清除试运行和试运行记录标志位
            bspSetWorkFlag(LOAD_STATUS_VERIFIED);
            bspSetResetNum(0);
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_L3PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_BTS_UPGRADE_FAIL,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);
        }
        else
        {
            //告警版本升级失败
            bspSetWorkFlag(LOAD_STATUS_VERIFIED);
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_L3PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_BTS_UPGRADE_FAIL,
                                   ALM_CLASS_CRITICAL,
                                   STR_BTS_BKVERFAIL);
        }
    }
    return;
}



 
CTaskSystem :: CTaskSystem()
{
    strcpy(m_szName, "tSys");
    m_uPriority   = M_TP_L3SYS;
    m_uOptions    = 0;
    m_uStackSize  = 20000;//SIZE_KBYTE * 500;
    m_iMsgQMax    = 1000; 
    m_iMsgQOption = 0;
    
    pL3L2ProbeTimer         = NULL;
    pBootupMonTimer         = NULL;
    pBtsRegNotifyTimer      = NULL;
    pBtsPeriodicRegTimer    = NULL;  
    pEmsLinkTimer           = NULL;
    pBtsDataInitTimer       = NULL;
    pEmsDataInitTimer       = NULL;
    pPeriodSetCpeTimeTimer  = NULL;
    pNotifyEmsDlDataTimer   = NULL; 
    m_pSystemClockTimer     = NULL;//系统状态初始化
    pDigitalBoardTemperatureMonTimer = NULL;
    memset(&m_BtsCpuWorkStatus, 0, sizeof(T_BtsCpuWorkStatus));
    memset(&m_CpuIsWorking,     0, sizeof(  T_CpuIsWorking));
    memset(&m_lastMCPStateInfo, 0, sizeof(m_lastMCPStateInfo));
    memset(&m_lastAUXStateInfo, 0, sizeof(m_lastAUXStateInfo));
    memset(&m_lastRFStateInfo,  0, sizeof(m_lastRFStateInfo));

    m_SysStatus.IsWork      = 0; //假设有10个模块必须发送开工指示消息。在现阶段本字段可以不用，
    m_SysStatus.BootLodeOk  = 0; //0 - 未收到    1- 已收到; 表示已经收到bootload任务的代码加载完成消息,在收到注册完成消息后可以进行数据配置了
    m_SysStatus.RegOk       = 0; //0 - 未完成    1- 完成;   表示注册已经完成,在收到 bootload任务的代码加载完成消息后可以进行数据配置了                  
    m_SysStatus.iSPeriodReg = 0; //0 - 系统未进行周期注册; 1 - 系统正进行周期注册
    m_SysStatus.IsEmsLinkOk = 1; //0 - 链路断    1- 链路通 ;当同ems链路断时要重新进行注册.
    m_SysStatus.NeverInitFromEMS = 1;    //未初始化数据
    m_SysStatus.SendWorkToEMS    = 0;

    pGetBtsTimerFromEms = NULL;
}


bool CTaskSystem :: Initialize()
{
    //初始化NVRAM
    if ((0x20080505 != NvRamDataAddr->init_code_high)
    || (0x0BAD0BAD  != NvRamDataAddr->init_code_low))
        {
        //nvram not initialized.
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Nvram is not initialized, initializing...");
        T_BTSCommonDataEle tmp; 
        if(0x20080505 == NvRamDataAddr->init_code_high)
            {
            //保留旧值
            memcpy(&tmp, &(NvRamDataAddr->BTSCommonDataEle), sizeof(tmp));
            }
        else
            {
            //
            memset(&tmp, 0, sizeof(tmp));
            }
        if(bspEnableNvRamWrite((char*)NvRamDataAddr, sizeof(T_NvRamData))==TRUE)
    	 {
            memset((char*)NvRamDataAddr, 0, sizeof(T_NvRamData) );
            memcpy(&(NvRamDataAddr->BTSCommonDataEle), &tmp, sizeof(tmp));   //恢复旧值
            NvRamDataAddr->init_code_high = 0x20080505;
            NvRamDataAddr->init_code_low  = 0x0BAD0BAD;
            bspDisableNvRamWrite((char*)NvRamDataAddr, sizeof(T_NvRamData));
    	 }
	
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Nvram is cleared.........");
	 #ifdef WBBU_CODE
        bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
        #endif
        }
    if(NvRamDataAddr->PayloadCfg.usFlag>1||(0x20110331!= NvRamDataAddr->pay_load_crc))
    {
       if(bspEnableNvRamWrite((char*)&NvRamDataAddr->PayloadCfg.usFlag, (sizeof(T_PayloadBalanceCfg)+sizeof(UINT32)))==TRUE)
     	{
		 NvRamDataAddr->PayloadCfg.usFlag = 1;
		 NvRamDataAddr->PayloadCfg.usLi = 50;
		 NvRamDataAddr->PayloadCfg.usPeriod = 20;
		 NvRamDataAddr->PayloadCfg.usLd = 30;
		 NvRamDataAddr->PayloadCfg.usCount = 6;
		 NvRamDataAddr->PayloadCfg.usSignal = 2;
		 NvRamDataAddr->PayloadCfg.usLdPeriod = 600;
		 NvRamDataAddr->PayloadCfg.usParam = 128;
		 NvRamDataAddr->pay_load_crc = 0x20110331;
		 bspDisableNvRamWrite((char*)&NvRamDataAddr->PayloadCfg.usFlag, (sizeof(T_PayloadBalanceCfg)+sizeof(UINT32)));
     	}
    }
    SYS_setActiveEMS(bspGetMainEmsIpAddr(), bspGetMainEmsUDPRcvPort());

    CBizTask :: Initialize();
    l3oamprintswversion();
    #ifdef WBBU_CODE
    ClearResetReasonSet();//wangwenhua add 20110429
      if((Hard_VerSion>5)&&(Hard_VerSion<15))
      	{
      		int    temperature = (ReadTemp(0,0)+1)/2;
    
	       if(temperature>= 20)
	       {
	                 //打开风扇
	       	 FanControl(0);
		  	OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Open BBU Fan");      
	       }
	       if(temperature<20)
	       {
	             //关闭风扇
	             	FanControl(1);     
		    	 OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Close BBU Fan");
		            
	            
	       }
    	}
    #endif
    return InitSysApp();
}
void CTaskSystem ::SYS_getLocalCfgFromCF()
{
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "SYS_getLocalCfgFromCF!");
    FILE *file;
    UINT32 count = 0;
    UINT8 *ptr;

     DIR* pdir = opendir( SM_USR_BOOT_DIR );
    if ( NULL == pdir )
    {
        userFileRecord.User_ACL_List.flag = 0;
	 userFileRecord.User_ACL_List.fileData = NULL;
	 userFileRecord.User_ACL_List.userFileLen = 0;
	 userFileRecord.User_ACL_List.userFileName[0] = 0;//set filename 0
	 userFileRecord.User_Voice_List.flag = 0;
	 userFileRecord.User_Voice_List.fileData = NULL;
	 userFileRecord.User_Voice_List.userFileLen = 0;
	 userFileRecord.User_Voice_List.userFileName[0] = 0;//set filename 0
	 userFileRecord.Trunk_Group_list.flag = 0;
	 userFileRecord.Trunk_Group_list.fileData = NULL;
	 userFileRecord.Trunk_Group_list.userFileLen = 0;
	 userFileRecord.Trunk_Group_list.userFileName[0] = 0;//set filename 0
	 userFileRecord.Trunk_Group_User_List.flag = 0;
	 userFileRecord.Trunk_Group_User_List.fileData = NULL;
	 userFileRecord.Trunk_Group_User_List.userFileLen = 0;
	 userFileRecord.Trunk_Group_User_List.userFileName[0] = 0;//set filename 0
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "SYS_getLocalCfgFromCF, can't use CF card, pls check!");
        return;
    }
    else
        closedir( pdir );
    if ((file = fopen (userACL, "a+")) == (FILE *)ERROR)/*如果打不开就不用cf机制*/
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "can't open file userACL.csv");
	 userFileRecord.User_ACL_List.flag = 0;
	 userFileRecord.User_ACL_List.fileData = NULL;
	 userFileRecord.User_ACL_List.userFileLen = 0;
	 userFileRecord.User_ACL_List.userFileName[0] = 0;//set filename 0
    }
    else
    {
        rewind(file);
        userFileRecord.User_ACL_List.fileData = new UINT8[140000];
	 ptr = userFileRecord.User_ACL_List.fileData;
        while ( !feof(file) )
        {
		UINT32 nRead = fread(ptr, 1/*size*/, 4096, file);
		count += nRead;
		ptr+=nRead;
        }
	 *ptr = 0;
	 userFileRecord.User_ACL_List.userFileLen = count+1;
	 userFileRecord.User_ACL_List.flag = 0x6789;
	 strcpy((char*)userFileRecord.User_ACL_List.userFileName, userACL);
	 fclose(file);
    }
    
    if ((file = fopen (userInfo, "a+")) == (FILE *)ERROR)/*如果打不开就不用cf机制*/
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "can't open file userInfo.csv");
	 userFileRecord.User_Voice_List.flag = 0;
	 userFileRecord.User_Voice_List.fileData = NULL;
	 userFileRecord.User_Voice_List.userFileLen = 0;
	 userFileRecord.User_Voice_List.userFileName[0] = 0;//set filename 0
    }
    else
    {
        rewind(file);
        userFileRecord.User_Voice_List.fileData = new UINT8[220000];
	 ptr = userFileRecord.User_Voice_List.fileData;
        while ( !feof(file) )
        {
		UINT32 nRead = fread(ptr, 1/*size*/, 4096, file);
		count += nRead;
		ptr+=nRead;
        }
	 *ptr = 0;
	 userFileRecord.User_Voice_List.userFileLen = count+1;
	 userFileRecord.User_Voice_List.flag = 0x6789;
	 strcpy((char*)userFileRecord.User_Voice_List.userFileName, userInfo);
	 fclose(file);
    }

    if ((file = fopen (GrpInfo, "a+")) == (FILE *)ERROR)/*如果打不开就不用cf机制*/
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "can't open file GrpInfo.csv");
	 userFileRecord.Trunk_Group_list.flag = 0;
	 userFileRecord.Trunk_Group_list.fileData = NULL;
	 userFileRecord.Trunk_Group_list.userFileLen = 0;
	 userFileRecord.Trunk_Group_list.userFileName[0] = 0;//set filename 0
    }
    else
    {
        rewind(file);
        userFileRecord.Trunk_Group_list.fileData = new UINT8[2000];
	 ptr = userFileRecord.Trunk_Group_list.fileData;
        while ( !feof(file) )
        {
		UINT32 nRead = fread(ptr, 1/*size*/, 4096, file);
		count += nRead;
		ptr+=nRead;
        }
	 *ptr = 0;
	 userFileRecord.Trunk_Group_list.userFileLen = count+1;
	 userFileRecord.Trunk_Group_list.flag = 0x6789;
	 strcpy((char*)userFileRecord.Trunk_Group_list.userFileName, GrpInfo);
	 fclose(file);
    }
    
    if ((file = fopen (GrpUserInfo, "a+")) == (FILE *)ERROR)/*如果打不开就不用cf机制*/
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "can't open file GrpUserInfo.csv");
	 userFileRecord.Trunk_Group_User_List.flag = 0;
	 userFileRecord.Trunk_Group_User_List.fileData = NULL;
	 userFileRecord.Trunk_Group_User_List.userFileLen = 0;
	 userFileRecord.Trunk_Group_User_List.userFileName[0] = 0;//set filename 0
    }
    else
    {
        rewind(file);
        userFileRecord.Trunk_Group_User_List.fileData = new UINT8[2048000];
	 ptr = userFileRecord.Trunk_Group_User_List.fileData;
        while ( !feof(file) )
        {
		UINT32 nRead = fread(ptr, 1/*size*/, 4096, file);
		count += nRead;
		ptr+=nRead;
        }
	 *ptr = 0;
	 userFileRecord.Trunk_Group_User_List.userFileLen = count+1;
	 userFileRecord.Trunk_Group_User_List.flag = 0x6789;
	 strcpy((char*)userFileRecord.Trunk_Group_User_List.userFileName, GrpUserInfo);
	 fclose(file);
    }
    
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "SYS_getLocalCfgFromCF OK!");
}
bool CTaskSystem :: InitSysApp() 
{
    gVx_System_Clock = 0;
//    bspCpldLED(LED_RUN, LED_ONEHZ);   //快闪表示初始化

    //修改bts复位记录，将复位计数加1
    SYS_AddBtsRstCnt();
    bool Rst = false;    
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tSys create ok!");

    CL3OamDiagEMSL3L2 *taskDiagL3 = CL3OamDiagEMSL3L2::GetInstance();
    if(NULL != taskDiagL3) Rst = taskDiagL3->Begin();
    else  rebootBTS(BOOT_CLEAR ); 
    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tDiagEmsL3L2 create ok!");
        CL3OamDiagL2L3EMS *pTaskDiagEMSL2L3 =  new CL3OamDiagL2L3EMS;
        if(NULL != pTaskDiagEMSL2L3) Rst = pTaskDiagEMSL2L3->Begin();
        else  rebootBTS(BOOT_CLEAR );
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tDiagL2L3Ems create ok!");
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tDiagEmsL3L2 create failed!");
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tDiagL2L3Ems create failed!");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }
#ifdef WBBU_CODE
  CWRRU   *taskWrru=CWRRU::GetInstance();
  if(NULL!=taskWrru)
  	{
  	   	Rst = taskWrru->Begin();
  	}
  else
  	{
  	      OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "taskWrru create fail !");
  	}
  OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "taskWrru create rst:%x!\n",Rst);
#endif
    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tDiagL2L3Ems create ok!");
        CSOCKET *taskSocket = CSOCKET::GetInstance();
        if(NULL != taskSocket) 
    {
	Rst = taskSocket->Begin();
#ifndef WBBU_CODE
	startUiTask();
#endif
    }
        else  
            rebootBTS(BOOT_CLEAR ); 
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tDiagL2L3Ems create FAIL");
        }

    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tSocket create ok!");
        CTaskAlm *pTaskAlarm = CTaskAlm::GetInstance();
        if(NULL != pTaskAlarm) Rst = pTaskAlarm->Begin();
        else  rebootBTS(BOOT_CLEAR ); 
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tSocket create FAIL.");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }

    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tAlarm create ok!");
        CTaskCfg *pTaskCfg = CTaskCfg::GetInstance();
        Rst = pTaskCfg->Begin();
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tAlarm create fail! now begin reboot bts,please wait......");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }

    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "tCfg create ok!");
        CTaskFileMag *pTaskFileM = CTaskFileMag::GetInstance();
        Rst = pTaskFileM->Begin();
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tCfg create fail! now begin reboot bts,please wait......");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }

    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "tFileM create ok!");
        CTaskFfpClient *pTaskFtpClient = CTaskFfpClient::GetInstance();
        Rst = pTaskFtpClient->Begin();
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tFileM create fail! now begin reboot bts,please wait......");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }

    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "tFtpC create ok!");
        CTaskCpeM *pTaskCpe = CTaskCpeM :: GetInstance();
        Rst = pTaskCpe->Begin();
        }

    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tFtpC create fail! now begin reboot bts,please wait......");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }
    
    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "tCpeM create ok!.");
        CTaskGPS* pTaskGps = CTaskGPS::GetInstance();
        Rst = pTaskGps->Begin();
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tCpeM create fail! now begin reboot bts,please wait......");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }
#ifndef __WIN32_SIM__      //__WIN32_SIM__
    if(true == Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "tGps create ok!");
        CTaskPM * pTaskPM = CTaskPM::GetInstance();
        Rst = pTaskPM->Begin();
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tGps create fail!");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }
#endif   
    if(true != Rst)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "tPM create fail! now begin reboot bts,please wait......");
#ifndef __WIN32_SIM__
        taskDelay(500);
        rebootBTS(BOOT_CLEAR );
#endif
        }
    else
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "tPM create ok!");
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] All OAM tasks create ok!");
        }
//    bspCpldLED(LED_RUN, LED_HALFHZ);    //慢闪表示初始化完成

    //启动监视系统代码加载的定时器
    pBootupMonTimer = SYS_Createtimer(M_OAM_BTS_BOOT_UP_TIMER, M_TIMER_PERIOD_NOT, SYS_CODE_LODE_PERIOD);
    if(NULL != pBootupMonTimer)
        {
        pBootupMonTimer->Start();
        }

    //创建周期注册定时器,因为此定时器会经常使用,故先创建出来,使用时直接
    //启动即可,永不用删除,只要停止即可.
    pBtsPeriodicRegTimer = SYS_Createtimer(M_OAM_BTS_PERIODREG_NOTIFY_TIMER, M_TIMER_PERIOD_YES, SYS_BTS_REG_PERIOD);
    if (NULL == pBtsPeriodicRegTimer)
        {
        return false;
        }

    //创建ems-bts心跳监视定时器。现在不启动。收到ems心跳请求时启动监视定时器。
    pEmsLinkTimer = SYS_Createtimer(M_EMS_BTS_LINK_TIMER, M_TIMER_PERIOD_NOT, SYS_BTS_EMSLINK_PERIOD);
    if(NULL == pEmsLinkTimer) 
        {
        return false;
        }

    pBtsRegNotifyTimer = SYS_Createtimer(M_OAM_BTSREG_NOTIFY_TIMER, M_TIMER_PERIOD_YES, SYS_BTS_REG_RSP_TIME);
    if(NULL == pBtsRegNotifyTimer)
        {
        return false;
        }

    m_pSystemClockTimer = SYS_Createtimer(M_OAM_BTS_CLOCK_TIMER, M_TIMER_PERIOD_YES, SYS_BTS_CLOCK_TIMER);
    if(NULL == m_pSystemClockTimer)
        {
        return false;
        }
    else
        {
        m_pSystemClockTimer->Start();
        }
#ifndef  WBBU_CODE
    pDigitalBoardTemperatureMonTimer = SYS_Createtimer(M_OAM_BTS_TEMPERATURE_MON_TIMER, M_TIMER_PERIOD_YES, SYS_BTS_TEMP_MON_PERIOD);
#else
	pDigitalBoardTemperatureMonTimer = SYS_Createtimer(M_OAM_BTS_TEMPERATURE_MON_TIMER, M_TIMER_PERIOD_YES, 5*60*1000);

#endif
    if(NULL == pDigitalBoardTemperatureMonTimer)
        {
        return false;
        }
    else
    {
        pDigitalBoardTemperatureMonTimer->Start();
    }

    //EMS状态初始化为主用ems正在注册，备用ems异常
    m_emsState = M_MAIN_EMS_REGISTER|M_BACKUP_EMS_DOWN;
    //发送BTS 注册指示
    SYS_BtsRegNotify(true);
    m_bWcpeBootModelOK =false;
	 m_bWcpeBootModel =false;
//#ifdef LJF_WCPE_BOOT
		if( (0x5a5a == NvRamDataAddr->Wcpe_Switch) && (BTS_BOOT_DATA_SOURCE_EMS==bspGetBootupSource()) )
		{
	        OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_SYS_DATA_INIT_FAIL, "[tSys] NvRamDataAddr->Wcpe_Switch[%04x]", NvRamDataAddr->Wcpe_Switch );
			bspSetBootupSource( BTS_BOOT_DATA_SOURCE_BTS);
			m_bWcpeBootModel = true;
		}

    return true;
}

CTimer* CTaskSystem :: SYS_Createtimer(UINT16 MsgId, UINT8 IsPeriod, UINT32 TimerPeriod)
{
    CL3OamCommonReq TimerMsg;
    if (false == TimerMsg.CreateMessage(*this))
        return NULL;
    TimerMsg.SetDstTid(M_TID_SYS);
    TimerMsg.SetSrcTid(M_TID_SYS);
    TimerMsg.SetMessageId(MsgId);
    return new CTimer(IsPeriod, TimerPeriod, TimerMsg);
}

bool CTaskSystem :: SYS_AddBtsRstCnt()
{
    UINT16 Cnt = NvRamDataAddr->BTSCommonDataEle.BtsRstCnt;
    Cnt = Cnt + 1;
    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->BTSCommonDataEle.BtsRstCnt), (SINT8*)&Cnt, 2);
    return true; 
}

bool CTaskSystem :: SYS_AddBtsRegCnt()
{
    UINT32 Cnt = NvRamDataAddr->BTSCommonDataEle.BtsRegCnt;
    Cnt = Cnt + 1;
    CTaskCfg::l3oambspNvRamWrite((SINT8*)&(NvRamDataAddr->BTSCommonDataEle.BtsRegCnt), (SINT8*)&Cnt, 4);
    return true; 
}

bool CTaskSystem :: ProcessMessage(CMessage &rMsg)
{
    UINT16 MsgId = rMsg.GetMessageId();
   OAM_LOGSTR1(LOG_DEBUG3, L3SYS_ERROR_REV_MSG, "[tSys] receive msg[0x%04x]", MsgId);  
    switch(MsgId)
    {
        case M_EMS_BTS_GET_BTS_STATUS_REQ:
        {
            UINT16 *tem = (UINT16 *)((UINT8*)rMsg.GetDataPtr());
            SYS_GetBtsSysStatusReq(*tem);
            break;
        }

        case M_OAM_BTS_CLOCK_TIMER:
            SYS_SystemClock();
            break;
        case M_EMS_BTS_REG_REQ:
        {
            CBtsRegReq BtsRegReq(rMsg);
            SYS_BtsRegReq(BtsRegReq);
            break;
        }

        case M_EMS_BTS_SESSIONID_UPDATE_REQ:
        {
             CSessionIdUpdateReq Req(rMsg);
             SYS_SessionIdUpdateReq(Req);
             break;
        }
        case M_EMS_BTS_HEARTBEAT_REQ:
        {
             CL3OamCommonReq Req(rMsg);
             SYS_EmsBtsHeartbeatReq(Req);
             break;
        }
        case M_EMS_BTS_DL_FINISH_REQ:
        {
            CL3OamCommonReq Notify(rMsg);
            SYS_BtsDlFinReq(Notify);
            break;
        }

        case M_OAM_DATA_CFG_FINISH_NOTIFY:
        {
            CL3OamCommonReq DataCfgFinishNotify(rMsg); 
            SYS_DataCfgFinishNotify(DataCfgFinishNotify);
            break;
        }

  
        case M_OAM_CPU_WORKING_NOTIFY:
        {
            CL3OamCpuWorkingNotify CpuWorkingNotify(rMsg); 
            SYS_CpuWorkingNotify(CpuWorkingNotify);
            break;
        }
        
        case M_OAM_SYSTEM_RUNNING_NOTIFY:  //BTS 成功启动完成通知
        {
            CModuleBootupNotify ModuleBootupNotify(rMsg);
            SYS_BtsBootUp(ModuleBootupNotify);
            break;
        }
        
        case M_OAM_BTS_BOOT_UP_TIMER:
        {
////////////CL3OamCommonReq SystemBootupTimer(rMsg);
            SYS_BtsBootUpTimer();
            break;
        }

        case M_OAM_BTSREG_NOTIFY_TIMER:
        {
            CL3OamCommonReq RegNotifyTimer(rMsg); 
            SYS_BtsRegNotifyTimer(RegNotifyTimer);
            break;
        }
        
        case M_OAM_BTS_PERIODREG_NOTIFY_TIMER:
        {
            CL3OamCommonReq RegNotifyTimer(rMsg); 
            SYS_BtsPeriodRegNotifyTimer(RegNotifyTimer);
            break;
        }
        
        case M_EMS_BTS_LINK_TIMER:
        {
             CL3OamCommonReq TimerMsg(rMsg);
             SYS_EmsLinkTimer(TimerMsg);
             break;
        }
        
        case M_OAM_BTS_RESET_NOTIFY:
        {
             CBtsRstNotify NotifyMsg(rMsg);
             SYS_OamBtsRstNotify(NotifyMsg);
             break;
        }

        case M_EMS_BTS_RESET_CNT_INCREASE_REQ:
        {
            CL3OamCommonReq ReqMsg(rMsg);
            SYS_BtsResetCntIncReq(ReqMsg.GetTransactionId());
            break;
        }

        case M_OAM_BTS_INIT_FROM_NVRAM_TIMER: 
        {
            SYS_BtsDataInitTimer();
            break;            
        }
        
        case M_OAM_PROBING_L2SYS_TIMER:
        {
             SYS_L3L2SysProbeTimer();
             break;
        }

        case M_OAM_SETCPETIME_TIMER:
        {
            SYS_BtsSynCpeTimer();
            break;
        }

        case M_OAM_BTS_TEMPERATURE_MON_TIMER:
        {
            SYS_BtsDigitalBoardTemperature();
            break;
        }

        case M_L2_L3_SYSPROBING_RSP:
        {
             CL3OamCommonRsp ProbeRsp(rMsg);
             SYS_L3L2SysProbeRsp(ProbeRsp);
             break;
        }
#ifdef WBBU_CODE
        case M_L3_L2_SYSPROBING_REQ_core1_rsp:
        {
        	CL3OamCommonRsp ProbeRsp(rMsg);
              SYS_L3L2SysProbeCore1Rsp(ProbeRsp);
            
        	 break;
        }
        case M_OAM_PROBING_L2SYS_FAIL_core1:
        {
        	  SYS_L3L2SysProbeCore1Fail();
        	  break;
        }
			case M_L2_CORE9_STATUS_NOTIFY://3409
			{
			   CL3L2Core9StateNoitfy NoitfyMsg(rMsg);  
			   SYS_L2L3Core9StateNotify(NoitfyMsg); 
				break;
			}
			case M_L2_AIF_STATAS_NOTIFY:
			{
					    CL3L2AifStateNoitfy NoitfyMsg(rMsg);  
			                 SYS_L2L3AifInfoStateNotify(NoitfyMsg); 
			                 break;
			}

#endif
        case M_OAM_PROBING_L2SYS_FAIL:
        {
	      UINT16 transId = rMsg.GetTransactionId();
	      if(transId!=0)
	      {
	          CTransaction * pTransaction = FindTransact(transId);
		   if(pTransaction)
                 {
                     pTransaction->EndTransact();
                     delete pTransaction;
                 }		    
	      }
             SYS_L3L2SysProbeFail();
             break;
        }
        
        case M_OAM_BTS_INITFROMEMS_TIMER:
        {
            SYS_InitFromEmsFail();
            break;
        }
        
        // 发送数据下载通知到ems后，没收到ems的配置消息，要重新发送数据下载通知消息
        case M_OAM_EMS_DL_NOTIFY_DATA_TIMER:  
        {
            SYS_SendDataDLNotify();
            break;
        }

        case M_OAM_CFG_SYS_REV_FIRST_CFGMSG:
        {
            if(pNotifyEmsDlDataTimer)
            {
                pNotifyEmsDlDataTimer->Stop();
                delete pNotifyEmsDlDataTimer;
                pNotifyEmsDlDataTimer = NULL;
            }
            break;
        }

#if 0
        //在告警处理中完成
        case M_OAM_BTS_TXCOCALIBRATION_TIMER:
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] TCXO err timer expire, reset FEP1");
            SendCpuResetMsg(BTS_CPU_TYPE_FEP, BTS_CPU_INDEX_FEP1);
            pTXCOCalibrationErrTimer->Stop();   //不停止定时器
        }
        case M_L2_L3_CFG_SYN_POWER_RSP:
        {
            //to do.
        }
#endif
        case M_OAM_CPU_RESET_NOTIFY:
            {
                SINT8* pdata =  ((SINT8*) rMsg.GetDataPtr()) + 2;
                UINT8 CpuType  = pdata[0];
                UINT8 CpuIndex = pdata[1];
                OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Receive CPU Reset Notify message.");
                SendCpuResetMsg(CpuType, CpuIndex); 
                break;
            } 
		
       case M_OAM_BTS_GET_BTS_TIME_FROM_EMS_TIMER:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] send M_BTS_EMS_SYSTEM_TIME_REQ ...");
            //基站向EMS请求系统时间,如果同步源为485的情况下才发
            if(BTS_SYNC_SRC_485==NvRamDataAddr->L1GenCfgEle.SyncSrc)
            {
		            CL3OamCommonReq Notify1;
		            if (false == Notify1.CreateMessage(*this))
		            return false;
		            Notify1.SetDstTid(M_TID_EMSAGENTTX);
		            Notify1.SetSrcTid(M_TID_SYS);
		            Notify1.SetMessageId(M_BTS_EMS_SYSTEM_TIME_REQ);
		            Notify1.SetTransactionId(OAM_DEFAUIT_TRANSID);
		            if(true != Notify1.Post())
		            {
		                OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] send M_BTS_EMS_SYSTEM_TIME_REQ to EMS FAIL...");
		                Notify1.DeleteMessage();
		                return false;
		            }
            }
            break;
        }
	 case M_EMS_BTS_SYSTEM_TIME_RSP:
        {
            OAM_LOGSTR(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] recv M_EMS_BTS_SYSTEM_TIME_RSP ...");
	     if(BTS_SYNC_SRC_485==NvRamDataAddr->L1GenCfgEle.SyncSrc)
	     {
				            UINT8 *puc = (UINT8*)rMsg.GetDataPtr()+4;            
				            struct tm time_s;
				            T_TimeDate timeDate;            
				            
				            //同步系统时间
				            timeDate.year = puc[0]*0x100+puc[1];            
				            timeDate.month = puc[2];           
				            timeDate.day = puc[3];            
				            timeDate.hour = puc[4];            
				            timeDate.minute = puc[5];            
				            timeDate.second = puc[6];
							
				            LOG6(LOG_SEVERE,0,"ems time: %d, %d, %d, %d, %d, %d\n", timeDate.year, timeDate.month, timeDate.day, timeDate.hour, timeDate.minute, timeDate.second);


				#ifndef WBBU_CODE
			        rtcDateSet(timeDate.year, timeDate.month, timeDate.day, 0);
			        rtcTimeSet(timeDate.hour, timeDate.minute, timeDate.second);
			       #else
			        if((Hard_VerSion<5)||(Hard_VerSion>15))
			        	{
			       		 rtcDateSet(timeDate.year, timeDate.month, timeDate.day, 0);
			        		rtcTimeSet(timeDate.hour, timeDate.minute, timeDate.second);
			        	}
			              else
			              {
			                    char pbuffer[7];
			                    pbuffer[0] = timeDate.second;//&0x7f;  /******s********/
			                    pbuffer[1] = timeDate.minute;//&0x7f;  /******min*******/
			                    pbuffer[2] = timeDate.hour;//&0x3f;  /*********hour*******/
			                    pbuffer[3] = timeDate.day;//&0x3f; /*******day********/
			                    pbuffer[4] = 5;//&0x7; /****week day*****/
			                    pbuffer[5] = timeDate.month;//&0x1f; /*******month*******/
			                    pbuffer[6] = (timeDate.year-2000);//&0xff; /**year***/
			                    bspSetTime(pbuffer);
			                    
			              }
			           #endif
			        ////set system time;
			            {
			           // struct tm time_s;
			            //T_TimeDate timeDate;
			            struct timespec timeSpec;
			            unsigned int secCount;
			           #ifndef WBBU_CODE
			            //timeDate = bspGetDateTime();
			            time_s.tm_sec = timeDate.second;
			            time_s.tm_min = timeDate.minute;
			            time_s.tm_hour = timeDate.hour;

			            time_s.tm_mday = timeDate.day;
			            time_s.tm_mon  = timeDate.month - 1;
			            time_s.tm_year = timeDate.year - 1900;
			           #else
			            time_s.tm_sec = timeDate.second;
			            time_s.tm_min = timeDate.minute;
			            time_s.tm_hour = timeDate.hour;

			            time_s.tm_mday = timeDate.day;
			            time_s.tm_mon  = timeDate.month- 1;
			           
			            time_s.tm_year = timeDate.year- 1900;
			           #endif
			            time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

			            secCount = mktime(&time_s);
			            timeSpec.tv_sec = secCount;
			            timeSpec.tv_nsec = 0;
			            clock_settime(CLOCK_REALTIME, &timeSpec);
				     }
			            break;
	}
	 	}

        default:
            if(1 == m_SysStatus.IsWork)
                {
                switch(MsgId)
                    {
                    //L3 tBOOT发送的告警消息
                    case M_OAM_CPU_ALARM_NOTIFY:
                        {
                        CL3CpuAlarmNofity  NoitfyMsg(rMsg);  
                        SYS_L3CpuAlarmNofity(NoitfyMsg);
                        break;
                        }

                    //L2发送的告警消息
                    case M_L2_L3_AUXSTATE_NOTIFY:
                        { 
                        CL3L2AuxStateNoitfy NoitfyMsg(rMsg);  
                        SYS_L2L3AuxStateNotify(NoitfyMsg);
                        break;
                        }

                    case M_L2_L3_MCPSTATE_NOTIFY: 
                        {
                        CL3L2McpStateNoitfy NoitfyMsg(rMsg);  
                        SYS_L2L3McpStateNotify(NoitfyMsg);    
                        break;
                        }

                    case M_L2_L3_RFSTATE_NOTIFY:
                        {
                        CL3L2RFStateNoitfy NoitfyMsg(rMsg);  
                        SYS_L2L3RFStateNotify(NoitfyMsg);
                        break;
                        }

                    case M_OAM_VOICE_SAG_LINK_ALM:
                        {
                        SYS_SAGLinkAlm(rMsg); 
                        break;
                        }
                    }
                }
       break;
    }

    return true;
}

TID CTaskSystem :: GetEntityId()  const
{
   return M_TID_SYS;
}

CTaskSystem* CTaskSystem :: GetInstance()
{
    if ( NULL == m_Instance )
    {
        m_Instance = new CTaskSystem;
    }

    return m_Instance;
}

extern T_BTSLoadInfo gL3OamBTSLoadInfo;
#ifdef WBBU_CODE
extern  int  WRRU_Temperature;
extern unsigned char rru_status ;
extern  UINT8   g_Close_RF_flag ;
extern UINT32 g_close_RF_dueto_Slave;
#endif
void CTaskSystem ::  SYS_GetBtsSysStatusReq(UINT16 transid)
{
    //BTS Load Info (BTS->EMS)
     short temp;
#pragma pack (1)
struct T_BTSLoadInfo2EMS
    {
    UINT16 CurrentUserNumber;
    UINT16 AverageFreeDLchannel;
    UINT16 AverageFreeULchannel;
    UINT16 AverageFreePowerUsage;
    };
#pragma pack ()

    /////////////////////////////////////////////////
    gl3oamBtsSysStatusData.SatellitesVisible = gBtsVisbleSatcnt;
    gl3oamBtsSysStatusData.StatellitesTrack  = gBtstrackSatcnt;
    gl3oamBtsSysStatusData.BootSrc           = bspGetBootupSource() - 30;
    #ifndef WBBU_CODE
    gl3oamBtsSysStatusData.RfMask            = CTaskAlm::GetInstance()->getDisplayRFMask();    //current enabled RF.
   #else
      gl3oamBtsSysStatusData.RfMask   = CWRRU::GetInstance()->RRU_Get_Anntena_MASK();
   #endif
#ifdef WBBU_CODE

     if((rru_status==0)||(g_Close_RF_flag==1))
     	{
     	      gl3oamBtsSysStatusData.RfMask    = 0;
     	}
    gl3oamBtsSysStatusData.SyncCardTemperature = WRRU_Temperature/*gSyncCardTemperature*/; //      当前频踪板温度
    temp = ReadTemp(0,0);
    temp = (temp+1)/2;
   // temp = temp>>3;
   // if(temp%2)
    gl3oamBtsSysStatusData.DigitalBoardTemperature = temp;//bspGetBTSTemperature();//当前主用板温度
#else
    gl3oamBtsSysStatusData.DigitalBoardTemperature = bspGetBTSTemperature();//当前主用板温度
#endif
////UINT32 temp = GetSystemTime();
    gl3oamBtsSysStatusData.TimePassedSinceBtsBoot = gVx_System_Clock/*temp - gbtsBootSecond*/;
    /////////////////////////////////////////////////

    CL3OamCommonRsp CommonRsp;
    bool b = CommonRsp.CreateMessage(*this, sizeof(T_BtsSysStatusData)
        + sizeof(T_BTSLoadInfo2EMS)
        + 4);   // 4 = transid + result
    if (false == b)
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] System encouter exception, create message fail.");
        return;
        }

    CommonRsp.SetMessageId(M_BTS_EMS_GET_BTS_STATUS_RSP);
    CommonRsp.SetDstTid(M_TID_EMSAGENTTX);
    CommonRsp.SetTransactionId(transid);
    CommonRsp.SetResult(OAM_SUCCESS);
    UINT8 *pdstdata = (UINT8*)CommonRsp.GetDataPtr() + 4;
    memcpy(pdstdata, &gl3oamBtsSysStatusData, sizeof(T_BtsSysStatusData));
    pdstdata += sizeof(T_BtsSysStatusData);
    memcpy(pdstdata, &gL3OamBTSLoadInfo, sizeof(T_BTSLoadInfo2EMS));
    if(true != CommonRsp.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post Get BTS system status response fail...");
        CommonRsp.DeleteMessage();
    }
}
/*
extern "C" STATUS testReg()
{

    CTaskSystem *taskSys = CTaskSystem::GetInstance();
    taskSys->SYS_BtsRegNotify();
    return OK;
}
*/

extern "C" UINT32 GetBtsIpAddr();
bool CTaskSystem ::  SYS_BtsRegNotify(bool bRegToCurrentEMS)
{
    //启动定时器等待ems的bts注册请求消息
    pBtsRegNotifyTimer->Stop();
    pBtsRegNotifyTimer->Start();

    UINT32 natIp = htonl(GetBtsIpAddr());

    SYS_AddBtsRegCnt();
    CBtsRegNotify RegNotify;
    if (false == RegNotify.CreateMessage(*this))
    {
        return false;
    }
    RegNotify.SetDstTid(M_TID_EMSAGENTTX);
    RegNotify.SetSrcTid(M_TID_SYS);
    RegNotify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    UINT32 Version = bspGetBtsHWVersion();
    RegNotify.SetBtsHWVersion(Version);
    Version = l3oamgetactiveversion();//bspGetActiveVersion();
    RegNotify.SetBtsSWVersion(Version);

    SINT8 IDInfo[ENCRYPED_BTSID_LEN];
    memset(IDInfo, 0, sizeof(IDInfo));
    RegNotify.SetBtsID(IDInfo, ENCRYPED_BTSID_LEN);
    RegNotify.SetBtsIp(natIp);
    UINT16 Port = bspGetBtsUDPRcvPort();
    RegNotify.SetBtsRcvPort(htons(Port));
    if (false ==bRegToCurrentEMS)
        {
        //往备用ems注册
        OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS try to register to %s EMS", (int)((gActiveEMSip == bspGetMainEmsIpAddr())?"backup":"main"));
        SYS_WrapToBakEMS(RegNotify);
        }
    else
        {
        OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS try to register to %s EMS", (int)((gActiveEMSip == bspGetMainEmsIpAddr())?"main":"backup"));
        }
    if(true != RegNotify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Send Register Notify fail");
        RegNotify.DeleteMessage();
        return false;
    }

    struct in_addr IpAddr;
    UINT32 ems = gActiveEMSip;
    UINT16 port= gActiveEMSport;
    if (false  ==bRegToCurrentEMS)
        {
        ems = (bspGetMainEmsIpAddr() == gActiveEMSip)?bspGetBakEmsIpAddr():bspGetMainEmsIpAddr();
        port= (bspGetMainEmsIpAddr() == gActiveEMSip)?bspGetBakEmsUDPRcvPort():bspGetMainEmsUDPRcvPort();
        }
    IpAddr.s_addr = htonl(ems);
    SINT8 strIpAddr[ INET_ADDR_LEN ] = {0};
    inet_ntoa_b( IpAddr, strIpAddr );

    char disp[80];
    sprintf(disp, "[tSys] Send Register Notify to EMS[%s:%d] @state[%d]", (int)strIpAddr, port, m_SysStatus.IsWork);
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, disp);

    return true;
}

bool CTaskSystem :: SYS_BtsResetNotify()
{
    CBtsRstNotify RstNotify;
    if (false == RstNotify.CreateMessage(*this))
        return false;
    RstNotify.SetDstTid(M_TID_EMSAGENTTX);
    RstNotify.SetSrcTid(M_TID_SYS);
    RstNotify.SetTransactionId(OAM_DEFAUIT_TRANSID);

    UINT32 Version = l3oamgetactiveversion();//bspGetActiveVersion();
    RstNotify.SetActiveVersion(Version);
    Version = bspGetStandbyVersion();
    OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS standby version: %x", Version);
    RstNotify.SetStandbyVersion(Version);
    
    UINT16 Temp = bspGetBtsHWType();
    RstNotify.SetBtsHWType(Temp);
    Temp = bspGetBootupSource();
    RstNotify.SetBootupSource(Temp);
    Temp = bspGetBtsResetReason();
    RstNotify.SetResetReason(Temp);
    if(true != RstNotify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post BTS Reset Notify to tEmsAgentTx fail.");
        RstNotify.DeleteMessage();
        return false;
    }

    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Send BTS Reset Notify to EMS.");
    return true;
}

bool CTaskSystem :: SYS_BtsRegRsp(UINT16 State, UINT16 TransId, bool bRespToCurrentEMS)
{
    CL3OamBtsRegRsp Rsp;
    if (false == Rsp.CreateMessage(*this))
        return false;
    Rsp.SetDstTid(M_TID_EMSAGENTTX);
    Rsp.SetSrcTid(M_TID_SYS);
    Rsp.SetTransactionId(TransId);
    Rsp.SetResult(OAM_SUCCESS); //此处设置的值要进行重新定义
    Rsp.SetBtsRunState(State);

	//#ifdef LJF_WCPE_BOOT
//	if( 0x5a5a == NvRamDataAddr->Wcpe_Switch )
//	    Rsp.SetBootupSource(BTS_BOOT_DATA_SOURCE_EMS- 30);
//	else
//	    Rsp.SetBootupSource(bspGetBootupSource() - 30);
	if( m_bWcpeBootModel && (!m_bWcpeBootModelOK) )
	    Rsp.SetBootupSource(BTS_BOOT_DATA_SOURCE_EMS- 30);
	else if( m_bWcpeBootModel && (m_bWcpeBootModelOK) )
	    Rsp.SetBootupSource(BTS_BOOT_DATA_SOURCE_BTS- 30);
	else
	    Rsp.SetBootupSource(bspGetBootupSource() - 30);

    if (false == bRespToCurrentEMS)
        {
        SYS_WrapToBakEMS(Rsp);
        UINT32 ems = (bspGetMainEmsIpAddr() == gActiveEMSip)?bspGetBakEmsIpAddr():bspGetMainEmsIpAddr();
        UINT32 port= (bspGetMainEmsIpAddr() == gActiveEMSip)?bspGetBakEmsUDPRcvPort():bspGetMainEmsUDPRcvPort();
        OAM_LOGSTR2(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] send Register response to EMS:0x%x:%d",ems,port);
        }
    else
        OAM_LOGSTR2(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] send Register response to current EMS:0x%x:%d", gActiveEMSip, gActiveEMSport);

    if(true != Rsp.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post BTS Register Response fail");
        Rsp.DeleteMessage();
    }

    OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS send to EMS register response, state[%d]", State);
    return true;
}

bool CTaskSystem :: SYS_BtsRegReq(CBtsRegReq &rMsg)
{
   OAM_LOGSTR1(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "->SYS_BtsRegReq:%x",m_emsState);
    UINT16 State;

    //发送bts注册应答消息
    if((1 == m_SysStatus.iSPeriodReg)&&(0 == m_SysStatus.RegOk))
    {
        //系统进行周期注册，设置bts运行状态为初始化
        m_SysStatus.iSPeriodReg = 0;
    }

    State = m_SysStatus.IsWork;
    UINT32 ulFromEMS = rMsg.GetBtsAddr();
    OAM_LOGSTR1(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "Sockt rx EMS message from IP:0x%x", ulFromEMS);
    bool bFromCurrentEMS = (ulFromEMS == gActiveEMSip);
////必须保证tSocket优先级高于tSys.
    SYS_BtsRegRsp(State, rMsg.GetTransactionId(), bFromCurrentEMS);  

    //bts ip&port changed.
    if((bspGetBtsPubIp() != rMsg.GetBtsPublicIp()) || (bspGetBtsPubPort() != rMsg.GetBtsPublicPort()))
    {
        bspSetBtsPubIp(rMsg.GetBtsPublicIp());
        bspSetBtsPubPort(rMsg.GetBtsPublicPort());  
        
        CDataNotifyBtsIP Notify;
        bool flag = Notify.CreateMessage(*this);
        if (false == flag)
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] CDataNotifyBtsIP, create message fail.");
            return false;
        }

        Notify.SetDstTid(M_TID_EB);
        if(true != Notify.Post())
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post CDataNotifyBtsIP fail...");
            Notify.DeleteMessage();
        }
    }

    UINT16 usRegisterResult = rMsg.GetResult();
    switch(m_emsState)
        {
        case M_MAIN_EMS_REGISTER|M_BACKUP_EMS_DOWN:
            //主用ems注册,备用ems未工作,属于系统第一次往主用ems注册的情况
            //如果主用EMS注册成功：state->主用ems正常；备用ems down，关闭注册定时器；设置gActiveEMSip;,关闭周期注册定时器
            //如果主用EMS注册失败：state->主用ems down.设置gActiveEMSip;重启注册定时器；state->备用ems注册;改往备用EMS注册 {如果bts下载配置完成，启动主用EMS注册定时器；}
            if (ulFromEMS != bspGetMainEmsIpAddr())
                {
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Rx Register response NOT from main EMS @ EMS state:0x%x", m_emsState);
                return false;
                }
            pBtsRegNotifyTimer->Stop();
            if (OAM_SUCCESS == usRegisterResult)
                {   
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS Register to main EMS success @ EMS state:0x%x", m_emsState);
                m_emsState      = M_MAIN_EMS_WORK|M_BACKUP_EMS_DOWN;
                SYS_setActiveEMS(bspGetMainEmsIpAddr(), bspGetMainEmsUDPRcvPort());
                }
            else
                {
                //系统第一次往主用ems注册,失败.改往备用ems注册
                if (0 == bspGetBakEmsIpAddr())
                    {
                    OAM_LOGSTR(LOG_WARN, 0, "[tSys] BTS register to main EMS FAIL, no backup EMS configuration, now start PERIOD register.");
                    m_emsState = M_MAIN_EMS_DOWN|M_BACKUP_EMS_DOWN;
                    pBtsPeriodicRegTimer->Stop();
                    pBtsPeriodicRegTimer->Start();
                    m_SysStatus.iSPeriodReg = 1;
                    }
                else
                    {
                    OAM_LOGSTR(LOG_WARN, 0, "[tSys] BTS register to main EMS FAIL. now try the backup EMS.");
                    m_emsState = M_MAIN_EMS_DOWN|M_BACKUP_EMS_REGISTER;
                    SYS_BtsRegNotify(gActiveEMSip == bspGetBakEmsIpAddr());
                    }
                return true;
                }
            break;

        case M_MAIN_EMS_DOWN|M_BACKUP_EMS_REGISTER:
            //主用ems异常,往备用ems注册的情况
            //注册成功，state->备用ems工作；关闭注册定时器；设置gActiveEMSip;关闭周期注册定时器，如果bts下载配置完成，state->主用ems注册；启动主用EMS注册定时器；
            //注册失败，state->备用ems down；关闭注册定时器，关闭主用EMS注册定时器；启用周期注册定时器；设置gActiveEMSip=0;
            if (ulFromEMS != bspGetBakEmsIpAddr())
                {
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Rx Register response NOT from backup EMS @ EMS state:0x%x", m_emsState);
                return false;
                }
            pBtsRegNotifyTimer->Stop();
            if (OAM_SUCCESS == usRegisterResult)
                {
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS Register to backup EMS success @ EMS state:0x%x", m_emsState);
                m_emsState = M_MAIN_EMS_DOWN|M_BACKUP_EMS_WORK;
                SYS_setActiveEMS(bspGetBakEmsIpAddr(), bspGetBakEmsUDPRcvPort());
                if (1 == m_SysStatus.SendWorkToEMS)
                    {
                    //配置已经下载完成,属于bts工作后,由于主用ems心跳断引起的往备用ems注册
                    //需要启动主用ems的注册
                    m_emsState = M_MAIN_EMS_REGISTER|M_BACKUP_EMS_WORK;
                    SYS_BtsRegNotify(gActiveEMSip == bspGetMainEmsIpAddr());
                    }
                }
            else
                {
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS Register to backup EMS FAIL @ EMS state:0x%x", m_emsState);
                m_emsState = M_MAIN_EMS_DOWN|M_BACKUP_EMS_DOWN;
                pBtsPeriodicRegTimer->Stop();
                pBtsPeriodicRegTimer->Start();
                m_SysStatus.iSPeriodReg = 1;
                return true;
                }
            break;

        case M_MAIN_EMS_REGISTER|M_BACKUP_EMS_WORK:
            //主用EMS注册成功，state->主用ems工作；备用ems down，关闭注册定时器；设置gActiveEMSip;；关闭周期注册定时器
            if (ulFromEMS != bspGetMainEmsIpAddr())
                {
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Rx Register response NOT from main EMS @ EMS state:0x%x", m_emsState);
                return false;
                }
            pBtsRegNotifyTimer->Stop();
            if (OAM_SUCCESS == usRegisterResult)
                {
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS Register to main EMS success @ EMS state:0x%x", m_emsState);
                m_emsState = M_MAIN_EMS_WORK|M_BACKUP_EMS_DOWN;
                SYS_setActiveEMS(bspGetMainEmsIpAddr(), bspGetMainEmsUDPRcvPort());
                }
            else
                {
                //继续往主用ems注册
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS Register to main EMS FAIL @ EMS state:0x%x", m_emsState);
                SYS_BtsRegNotify(gActiveEMSip == bspGetMainEmsIpAddr());
                return true;
                }
            break;

        default:
            OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Rx Register response from EMS @ unexpected EMS[0x%X] state:0x%x", ulFromEMS, m_emsState);
            pBtsRegNotifyTimer->Stop();
            return false;
        }

    if(OAM_SUCCESS == usRegisterResult)
        {
        pBtsPeriodicRegTimer->Stop();
        m_SysStatus.RegOk = 1;
        OAM_LOGSTR1(LOG_SEVERE, 0, "[tSys] BTS register SUCCEED,m_SysStatus.BootLodeOk[%02d].", m_SysStatus.BootLodeOk);
//#ifdef LJF_WCPE_BOOT
		//if( ! m_bWcpeBootModelOK )
		//{
		//}
		if( (1==m_SysStatus.BootLodeOk) && (m_bWcpeBootModel) )
		{
			CCfgModuleInitNotify  InitNotify; 
			if (false == InitNotify.CreateMessage(*this))
		    {
			    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] System encounter exception, create message fail.");
			    return false;
		    }
			InitNotify.SetDstTid(M_TID_CM);
			InitNotify.SetSrcTid(M_TID_SYS);
			InitNotify.SetTransactionId(OAM_DEFAUIT_TRANSID);
		    InitNotify.SetType(TASK_INIT_FROM_EMS);
			
			if(true != InitNotify.Post())
		    {
		        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_SYS_DATA_INIT_FAIL, "[tSys] post tCM Init Notify fail...");
		        InitNotify.DeleteMessage();
		        return false;
		    }
			else
			{
		        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_SYS_DATA_INIT_FAIL, "[tSys] post tCM Init frome EMS for WCPE");
			}
		}
        if((m_SysStatus.NeverInitFromEMS == 1)&&
           (BTS_BOOT_DATA_SOURCE_EMS == bspGetBootupSource())&&
           (m_SysStatus.BootLodeOk == 1))
            {
             m_SysStatus.NeverInitFromEMS = 0;
             SYS_EMSDLHandler();
            }

        if ((m_SysStatus.IsWork == 1) && (m_SysStatus.SendWorkToEMS == 0))
            {
            SYS_SendBtsWorkingNotify();
            //通知CPE重新注册.
            //bts和ems链路断开期间，有cpe会登陆到bts。链路通后，需要cpe重新注册一下
           // notifyAllCPEtoRegister();//wangwenhua mark 2012-12-18
            CTaskPM *tpmt = CTaskPM::GetInstance();			
            tpmt->InitRTMonitor();
            tpmt->PM_StartRTMonitor();
            }
        OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] start HeartBeat timer.");
        pEmsLinkTimer->Stop();
        pEmsLinkTimer->Start();
        }
#if 0
    else//register 不成功
    {
        OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] BTS register failed!!! Begin period register...");
        if(NULL == pBtsPeriodicRegTimer)
        {
            pBtsPeriodicRegTimer = SYS_Createtimer(M_OAM_BTS_PERIODREG_NOTIFY_TIMER, M_TIMER_PERIOD_YES, SYS_BTS_REG_PERIOD);
        }
        if(NULL != pBtsPeriodicRegTimer)
        {
            pBtsPeriodicRegTimer->Start();
        }
    }
#endif
    return true;
}


bool CTaskSystem :: SYS_EMSDLHandler()
{
    //Download from EMS.
    SYS_SendDataDLNotify();

    //kill and Start timer.
    if (NULL == pNotifyEmsDlDataTimer)
        pNotifyEmsDlDataTimer = SYS_Createtimer(M_OAM_EMS_DL_NOTIFY_DATA_TIMER, M_TIMER_PERIOD_YES, OAM_REQ_RSP_PERIOD);
    else
        pNotifyEmsDlDataTimer->Stop();
        
    if (NULL != pNotifyEmsDlDataTimer)
        pNotifyEmsDlDataTimer->Start();

    if(NULL == pEmsDataInitTimer)
        pEmsDataInitTimer = SYS_Createtimer(M_OAM_BTS_INITFROMEMS_TIMER, M_TIMER_PERIOD_NOT, SYS_BTS_INITFROMEMS_PERRIOD);
    else
        pEmsDataInitTimer->Stop();
    if (NULL != pEmsDataInitTimer)
        pEmsDataInitTimer->Start();

    return true;
}

//tboot的代码加载完成指示消息
bool CTaskSystem :: SYS_BtsBootUp(CModuleBootupNotify &)//rMsg
{
    //创建复位上报消息
#ifdef WBBU_CODE
     if(m_SysStatus.BootLodeOk==1)
     	{
     	    return true;
     	}
#endif
    SYS_BtsResetNotify();

#ifndef __WIN32_SIM__
    taskDelay(10);
#endif

    if(pBootupMonTimer != NULL)
        {
        pBootupMonTimer->Stop();
        delete pBootupMonTimer;
        pBootupMonTimer = NULL;
        }

    CCfgModuleInitNotify  InitNotify; 
    if (false == InitNotify.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] System encounter exception, create message fail.");
        return false;
        }
    InitNotify.SetDstTid(M_TID_CM);
    InitNotify.SetSrcTid(M_TID_SYS);
    InitNotify.SetTransactionId(OAM_DEFAUIT_TRANSID);

    if (BTS_BOOT_DATA_SOURCE_BTS == bspGetBootupSource())    
    {
        OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] BTS boot from NvRam.");
        InitNotify.SetType(TASK_INIT_FROM_NVRAM);
        if(true != InitNotify.Post())
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_SYS_DATA_INIT_FAIL, "[tSys] post tCM Init Notify fail...");
            InitNotify.DeleteMessage();
            return false;
        }

        pBtsDataInitTimer = SYS_Createtimer(M_OAM_BTS_INIT_FROM_NVRAM_TIMER, M_TIMER_PERIOD_NOT, SYS_BTS_INITFROMNVRAM_PERRIOD);
        if(NULL != pBtsDataInitTimer)
        {
            pBtsDataInitTimer->Start();
        }
    }
    else if(BTS_BOOT_DATA_SOURCE_EMS == bspGetBootupSource())       // 从 ems 启动
    {
        OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] BTS boot from EMS.");
        InitNotify.SetType(TASK_INIT_FROM_EMS);
        if(true != InitNotify.Post())
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_SYS_DATA_INIT_FAIL, "[tSys] post tCM Init Notify fail...");
            InitNotify.DeleteMessage();
            return false;
        }

        if(m_SysStatus.RegOk)
        {
            OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] BTS register state is OK.");
            SYS_EMSDLHandler();
        }
        else
        {
           SYS_EMSDLHandler();//wangwenhua add 20090805
            OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] BTS register state is NOT OK!");
            OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] BTS dont send Data Download Notify. now waiting for EMS's register response...");
        }
    }

    m_SysStatus.BootLodeOk = 1;
    SYS_SendBtsResetCntsNotify();
    
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_BTS_BOOT_SUCCESS ,"[tSys] BTS boot up SUCCESS.");
    
    return true;
}

bool CTaskSystem :: SYS_SendDataDLNotify()
{
    //构造发往ems的数据下载指示消息,将消息发往ems
    OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO,"[tSys] send Data Download Notify to EMS[0x%X:%d]....", gActiveEMSip, gActiveEMSport);
    CBtsDataDLoadNotify Notify;
    if (false == Notify.CreateMessage(*this))
        {
        return false;
        }
    Notify.SetDstTid(M_TID_EMSAGENTTX);
    Notify.SetSrcTid(M_TID_SYS);
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);

    Notify.SetActiveVerion(bspGetActiveVersion());
    Notify.SetStandbyVersion(bspGetStandbyVersion());
    Notify.SetBtsHWType(bspGetBtsHWType());
    if(true != Notify.Post())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO,"[tSys] send Data Download Notify to EMS[0x%X:%d] FAIL", gActiveEMSip, gActiveEMSport);
        Notify.DeleteMessage();
        return false;
    }

    return true;
}

bool CTaskSystem :: SYS_InitFromEmsFail()
{
    if(NULL == pEmsDataInitTimer)
    {
        return false;
    }
    else
    {
        pEmsDataInitTimer->Stop();
    }
#ifdef WBBU_CODE
        m_SysStatus.IsWork = 1;
#endif
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "[tSys] Boot from EMS FAIL! Reboot BTS......");
    bspSetBootupSource(BTS_BOOT_DATA_SOURCE_BTS);//wangwenhua modify if ems not ok,then from nvram
    #ifndef __WIN32_SIM__
    taskDelay(500);
    
    bspSetBtsResetReason(RESET_REASON_SW_INIT_EMSTIMEOUT_FAIL/*RESET_REASON_SW_INIT_FAIL*/);
    rebootBTS(BOOT_CLEAR );
    #endif
  
    return true;
}

bool CTaskSystem :: SYS_BtsDlFinReq(CL3OamCommonReq &rMsg)
{
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Receive Dada Download Finish Notify");
    m_SysStatus.IsWork = 1;
    CL3OamCommonRsp RspMsg;
    if (false == RspMsg.CreateMessage(*this))
        {
        return false;
        }
    RspMsg.SetDstTid(M_TID_EMSAGENTTX);
    RspMsg.SetSrcTid(M_TID_SYS);
    RspMsg.SetMessageId(M_BTS_EMS_DL_FINISH_RSP);
    RspMsg.SetTransactionId(rMsg.GetTransactionId());
    if(true != RspMsg.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post Data Download Finish Response fail...");
        RspMsg.DeleteMessage();
    }
    
    if(NULL != pEmsDataInitTimer)
    {
        pEmsDataInitTimer->Stop();
        delete pEmsDataInitTimer;
        pEmsDataInitTimer = NULL;
    }

    SYS_SendNotifyMsg(M_TID_CM, M_OAM_SYS_CFG_INIT_FROM_EMS_OK);

    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Boot from EMS SUCCESS!");

    //向ems发送bts开工指示消息
    SYS_SendBtsWorkingNotify();
#if 0
    OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] start HeartBeat timer.");
    pEmsLinkTimer->Stop();
    pEmsLinkTimer->Start();
#endif
    SYS_StartSetL2TimeDayTimer();
    
    //设置bts启动标志，下次从bts nvram启动
    bspSetBootupSource(BTS_BOOT_DATA_SOURCE_BTS);

    //从备用ems下载配置数据的过程中,不向主用ems注册
    //等待下载完成,再启动注册定时器.
    if (m_emsState&M_MAIN_EMS_DOWN)
        {
        //主用ems不工作，持续检查
        m_emsState = M_MAIN_EMS_REGISTER|M_BACKUP_EMS_WORK;
        SYS_BtsRegNotify(gActiveEMSip == bspGetMainEmsIpAddr());
        }

    return true;
}

//当从bts启动时,收到的从cm发来的初始化结束指示消息
bool CTaskSystem :: SYS_DataCfgFinishNotify(CL3OamCommonReq &)//rMsg
{
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Receive Data Init Finish Notify from tCfg");
    //设置系统状态为开工状态
    m_SysStatus.IsWork = 1;

    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Boot from BTS Nvram SUCCESS!");
    //向ems发送bts开工指示
    SYS_SendBtsWorkingNotify();
#if 0
    OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] start HeartBeat timer.");
    pEmsLinkTimer->Stop();
    pEmsLinkTimer->Start();
#endif
    SYS_StartSetL2TimeDayTimer();

    if(pBtsDataInitTimer != NULL)
    {
        pBtsDataInitTimer->Stop();
        delete pBtsDataInitTimer;
        pBtsDataInitTimer = NULL;
    }

    //等待配置完成,再启动注册定时器.
    if (m_emsState&M_MAIN_EMS_DOWN)
        {
        //主用ems不工作，持续检查
        m_emsState = M_MAIN_EMS_REGISTER|M_BACKUP_EMS_WORK;
        SYS_BtsRegNotify(gActiveEMSip == bspGetMainEmsIpAddr());
        }

    return true;
}

bool CTaskSystem :: SYS_StartL3L2MoniterTimer()
{
    //启动L3L2链路PROBE检测定时器
    if (NULL != pL3L2ProbeTimer)
        {
        pL3L2ProbeTimer->Stop();
        delete pL3L2ProbeTimer;
        }
    OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] Now start L2 probe timer...");
    pL3L2ProbeTimer = SYS_Createtimer(M_OAM_PROBING_L2SYS_TIMER, M_TIMER_PERIOD_YES, SYS_L3L2SYSPROBE_PERIOD);
    if(NULL != pL3L2ProbeTimer)
    {
        pL3L2ProbeTimer->Start();
    }
    return true;
}


bool CTaskSystem :: SYS_StartSetL2TimeDayTimer()
{
    pPeriodSetCpeTimeTimer = SYS_Createtimer(M_OAM_SETCPETIME_TIMER, M_TIMER_PERIOD_YES, SYS_BTS_TIMEDAY_PERIOD);
    if(NULL != pPeriodSetCpeTimeTimer)
    {
        pPeriodSetCpeTimeTimer->Start();
    }
    return true;
}

extern  void sendCdrForReboot();/*lijinan 20100916 for bill*/
//代码加载超时,重新复位bts
bool CTaskSystem :: SYS_BtsBootUpTimer()
{
    if(pBootupMonTimer != NULL)
    {
        pBootupMonTimer->Stop();
        delete pBootupMonTimer;
        pBootupMonTimer = NULL;
    }

    OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] BTS boot up FAIL! Reboot BTS now.");
#ifndef __WIN32_SIM__
    bspSetBtsResetReason(RESET_REASON_SW_INIT_LOADCODETIMEOUT_FAIL/*RESET_REASON_SW_INIT_FAIL*/);
    sendCdrForReboot();
    taskDelay(500);
    rebootBTS(BOOT_CLEAR );
#endif

    return true;
}

//获取bts时间构造指示消息发给L2    
bool CTaskSystem :: SYS_BtsSynCpeTimer()
{
    T_TimeDate TD;
    TD = bspGetDateTime();

    CSetCpeTimeNotify Notify;
    if (false == Notify.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] System encounter exceptions, create message fail.");
        return false;
        }
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetDstTid(M_TID_L2MAIN);
    Notify.SetSrcTid(M_TID_SYS);
    Notify.SetYear(TD.year);
    Notify.SetMonth(TD.month);
    Notify.SetDay(TD.day);
    Notify.SetHour(TD.hour);
    Notify.SetMinute(TD.minute);
    Notify.SetSecond(TD.second);
    if(true != Notify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post BTS sync CPE time to L2 fail");
        Notify.DeleteMessage();
        return false;
    }

    return true;
}


void CTaskSystem::SYS_BtsDigitalBoardTemperature()
{
#define M_TEMP_COLD     (NvRamDataAddr->TempAlarmCfgEle.ShutdownL)
#define M_TEMP_LOW      (NvRamDataAddr->TempAlarmCfgEle.AlarmL)
#define M_TEMP_HIGH     (NvRamDataAddr->TempAlarmCfgEle.AlarmH)
#define M_TEMP_HOT      (NvRamDataAddr->TempAlarmCfgEle.ShutdownH)
#ifndef WBBU_CODE
    int    temperature = bspGetBTSTemperature();
#else
    int    temperature = (ReadTemp(0,0)+1)/2;
#endif
#ifdef WBBU_CODE
    if((Hard_VerSion>5)&&(Hard_VerSion<15))
    	{
    	    static unsigned char   FanStatus  =0;
	       if(temperature>= 40)
	       {
	                 //打开风扇
	                  FanControl(0);
	                 if(FanStatus!=1)
	                 {
		                
		                 OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Open BBU Fan");
		                 FanStatus = 1;
	                 }
	                 
	       }
	       if(temperature<=0)
	       {
	             //关闭风扇
	             FanControl(1);
	             if(FanStatus!=2)
	             	{
		             
		             OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Close BBU Fan");
		              FanStatus = 2;
	             	}
	       }
    	}
    #endif
    UINT8  ucAlmArea = 0;
    static bool  bAlm = false;
    static UINT8 ucLastAlmArea = 0;
    if (temperature < M_TEMP_COLD)
        {
        ////温度太低
        ucAlmArea = 1;
        }
    else if (temperature < M_TEMP_LOW)
        {
        ////温度较低
        ucAlmArea = 2;
        }
    else if (temperature > M_TEMP_HOT)
        {
        ////温度太高
        ucAlmArea = 3;
        }
    else if (temperature > M_TEMP_HIGH)
        {
        ////温度较高
        ucAlmArea = 4;
        }
    else
        {
        ////温度正常
        if (true == bAlm)
            {
            //如果之前有告警，则恢复
            bAlm = false;
            AlarmReport(ALM_FLAG_CLEAR,
                           ALM_ENT_ENV,
                           ALM_ENT_INDEX0,
                           ALM_ID_ENV_DIGITAL_BOARD_TEMPERATURE,
                           ALM_CLASS_CRITICAL,
                           STR_CLEAR_ALARM);
            ucLastAlmArea = ucAlmArea = 0;
            }
        return;
        }
////温度异常
    if (ucAlmArea != ucLastAlmArea)
        {
        bAlm = true;
        AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_ENV,
                       ALM_ENT_INDEX0,
                       ALM_ID_ENV_DIGITAL_BOARD_TEMPERATURE,
                       ALM_CLASS_CRITICAL,
                       STR_DIGITAL_BORD_TEMPERATURE, temperature);
        ucLastAlmArea = ucAlmArea;
        }
    return;
}

bool CTaskSystem :: SYS_BtsRegNotifyTimer(CL3OamCommonReq &)//rMsg
{
    switch(m_emsState)
        {
        case M_MAIN_EMS_REGISTER|M_BACKUP_EMS_DOWN:
            //state->主用ems down；state->备用ems注册，改往备用EMS注册
            //属于bts启动后第一次注册,主用ems超时不回应的情况
            if (0 == bspGetBakEmsIpAddr())
                {
                OAM_LOGSTR(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS register to main EMS time out, no backup EMS configuration, now start PERIOD register.");
                m_emsState = M_MAIN_EMS_DOWN|M_BACKUP_EMS_DOWN;
                pBtsRegNotifyTimer->Stop();
                pBtsPeriodicRegTimer->Stop();
                pBtsPeriodicRegTimer->Start();
                m_SysStatus.iSPeriodReg = 1;
                }
            else
                {
                OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Register to main EMS time out @ EMS state:0x%x", m_emsState);
                m_emsState = M_MAIN_EMS_DOWN|M_BACKUP_EMS_REGISTER;
                SYS_BtsRegNotify(gActiveEMSip == bspGetBakEmsIpAddr());
                }
            break;
        case M_MAIN_EMS_DOWN|M_BACKUP_EMS_REGISTER:
            OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Register to backup EMS time out @ EMS state:0x%x", m_emsState);
            m_emsState = M_MAIN_EMS_DOWN|M_BACKUP_EMS_DOWN;
            pBtsRegNotifyTimer->Stop();
            //启动周期注册定时器
            pBtsPeriodicRegTimer->Stop();
            pBtsPeriodicRegTimer->Start();
            m_SysStatus.iSPeriodReg = 1;
            break;
        case M_MAIN_EMS_REGISTER|M_BACKUP_EMS_WORK:
            //属于备用ems工作情况下,持续检查主用ems是否能注册的情况
            //继续检查
            OAM_LOGSTR1(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BTS register to main EMS time out @ EMS state:0x%x, continue to register", m_emsState);
            SYS_BtsRegNotify(gActiveEMSip == bspGetMainEmsIpAddr());
            break;
	 case M_MAIN_EMS_DOWN|M_BACKUP_EMS_DOWN:
	     OAM_LOGSTR(LOG_WARN, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] EMS state:M_MAIN_EMS_DOWN|M_BACKUP_EMS_DOWN");
	     pBtsRegNotifyTimer->Stop();
            //启动周期注册定时器
            pBtsPeriodicRegTimer->Stop();
            pBtsPeriodicRegTimer->Start();
            m_SysStatus.iSPeriodReg = 1;
	     break;
	 	
        default:
            pBtsRegNotifyTimer->Stop();
            OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Register to EMS time out @ unexpected state:0x%x", m_emsState);
            break;
        }

    return true;
}

bool CTaskSystem :: SYS_BtsPeriodRegNotifyTimer(CL3OamCommonReq &)//rMsg
{
    m_emsState = M_MAIN_EMS_REGISTER|M_BACKUP_EMS_DOWN;
    //往主用ems注册
    gActiveEMSip = bspGetMainEmsIpAddr();
    gActiveEMSport = bspGetMainEmsUDPRcvPort();
    return SYS_BtsRegNotify(true);
}

///////////////////////////////////////////////////////////
bool CTaskSystem :: SYS_SessionIdUpdateReq(CSessionIdUpdateReq &rMsg)
{
    CL3OamCommonRsp RspMsg;
    if (false == RspMsg.CreateMessage(*this))
        return false;
    RspMsg.SetTransactionId(rMsg.GetTransactionId());
    RspMsg.SetDstTid(M_TID_EMSAGENTTX);
    RspMsg.SetSrcTid(M_TID_SYS);
    RspMsg.SetMessageId(M_BTS_EMS_SESSIONID_UPDATE_RSP);
    if(true != RspMsg.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post to EMS Session ID update response fail...");
        RspMsg.DeleteMessage();
        return false;
    }

    //其余处理未考虑
    return true;
}

//在收到ems心跳监测请求消息后,向ems发送应答消息,然后启动定时器
//等待下一个请求消息,若超时未收到请求消息则认为同ems链路断,重新
//向ems注册
bool CTaskSystem :: SYS_EmsBtsHeartbeatReq(CL3OamCommonReq &rMsg)
{
    UINT32 ulFromEMS = rMsg.GetBtsAddr();
    if (ulFromEMS != gActiveEMSip)
        {
        OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Rx heartbeat request from other EMS[0x%X].", ulFromEMS);
        return false;
        }
    CL3OamCommonRsp Rsp;
    if (false == Rsp.CreateMessage(*this))
        return false;
    Rsp.SetTransactionId(rMsg.GetTransactionId());
    Rsp.SetDstTid(M_TID_EMSAGENTTX);
    Rsp.SetSrcTid(M_TID_SYS);
    Rsp.SetMessageId(M_BTS_EMS_HEARTBEAT_RSP);
    Rsp.SetResult(OAM_SUCCESS);
    if(true != Rsp.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post Heartbeat response fail");
        Rsp.DeleteMessage();
    }

    OAM_LOGSTR1(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] Rx heartbeat request from EMS[0x%X].", ulFromEMS);
    pEmsLinkTimer->Stop();
    pEmsLinkTimer->Start();
    return true;
}

///////////////////////////////////////////////////////////////////////
//心跳定时器超时
bool CTaskSystem :: SYS_EmsLinkTimer(CL3OamCommonReq &)//rMsg
{
    OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] Can't receive EMS heartbeat, EMS link down!");
    if(m_emsState & M_MAIN_EMS_WORK)
        {
        //如果原来是主用ems，仍然试图往主用ems注册
        m_emsState = M_MAIN_EMS_REGISTER|M_BACKUP_EMS_DOWN;
        }
    else if(m_emsState & M_BACKUP_EMS_WORK)
        {
        //如果原来是备用ems，仍然试图往备用ems注册
        m_emsState = M_MAIN_EMS_DOWN|M_BACKUP_EMS_REGISTER;
        }
    return SYS_BtsRegNotify(true);
}
//#ifdef LJF_RPT_ALTER_2_NVRAMLIST
bool CTaskSystem :: SYS_IsEmsConnectOK()//rMsg
{
	UINT32 ulmain= 0;
	UINT32 ulbackup = 0;
	ulmain = m_emsState & M_MAIN_EMS_WORK;
	ulbackup = m_emsState & M_BACKUP_EMS_WORK;
    OAM_LOGSTR3(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] m_emsState[%08x] main[%08x] backup[%08x]", m_emsState, ulmain, ulbackup );
    if( (ulmain>0) || (ulbackup>0) )
    	return true;
	else
		return false;
}

bool CTaskSystem :: SYS_OamBtsRstNotify(CBtsRstNotify &rMsg)
{
    bspSetBtsResetReason(RESET_REASON_EMS);
    OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] L3 reboot BTS, please wait...%d",RESET_REASON_EMS);

#ifndef __WIN32_SIM__
    sendCdrForReboot();
    taskDelay(500);
    rebootBTS(BOOT_CLEAR );
#endif

    return true;
}

bool CTaskSystem :: SYS_BtsResetCntIncReq(UINT16 transID)
{
    if (OAM_DEFAUIT_TRANSID != transID)
        {
        //EMS下发的命令，需要回复给EMS.
        CL3OamCommonRsp RspMsg;
        if (false == RspMsg.CreateMessage(*this))
            return false;
        RspMsg.SetTransactionId(transID);
        RspMsg.SetDstTid(M_TID_EMSAGENTTX);
        RspMsg.SetSrcTid(M_TID_SYS);
        RspMsg.SetMessageId(M_BTS_EMS_RESET_CNT_INCREASE_RSP);
	RspMsg.SetResult(0);/***wangwenhua add 20100705***/
        if(true != RspMsg.Post())
            {
            RspMsg.DeleteMessage();
            return false;
            }
        }

    SYS_AddBtsRstCnt();
    SYS_SendBtsResetCntsNotify();

    return true;
} 

bool CTaskSystem :: SYS_BtsDataInitTimer()//rMsg
{
    bspSetBtsResetReason(RESET_REASON_SW_INIT_NVRAMBOOT_FAIL/*RESET_REASON_SW_INIT_FAIL*/);
    OAM_LOGSTR(LOG_SEVERE, 0, "[tSys] Init BTS from NvRam FAIL! Reboot BTS now.");
    bspSetBootupSource(BTS_BOOT_DATA_SOURCE_EMS);
#ifndef __WIN32_SIM__
    taskDelay(500);
    rebootBTS(BOOT_CLEAR );
#endif
    return true;
}

//对L3-L2的链路监测由L3 OAM发起. L3 OAM以周期 SYS_L3L2SYSPROBE_PERIOD
//发起probe req
bool CTaskSystem :: SYS_L3L2SysProbeTimer()//rMsg
{  
    CL3OamCommonReq L3L2SysProbeReq; 
    if (false == L3L2SysProbeReq.CreateMessage(*this))
        return false;
    L3L2SysProbeReq.SetDstTid(M_TID_L2MAIN);  
    L3L2SysProbeReq.SetSrcTid(M_TID_SYS);
    L3L2SysProbeReq.SetMessageId(M_L3_L2_SYSPROBING_REQ);

    CL3OamCommonReq L3L2ProbeReqFail;
    if (false == L3L2ProbeReqFail.CreateMessage(*this))
    {
        L3L2SysProbeReq.DeleteMessage();
        return false;
    }
    L3L2ProbeReqFail.SetDstTid(M_TID_SYS);
    L3L2ProbeReqFail.SetSrcTid(M_TID_SYS);
    L3L2ProbeReqFail.SetMessageId(M_OAM_PROBING_L2SYS_FAIL);

    CTransaction *pL3L2ProbeTrans = CreateTransact(L3L2SysProbeReq, L3L2ProbeReqFail, OAM_REQ_RESEND_L2_L3_CNT40, 5000);
    if (NULL == pL3L2ProbeTrans)
    {
        L3L2SysProbeReq.DeleteMessage();
        L3L2ProbeReqFail.DeleteMessage();
        return false;
    }
    L3L2ProbeReqFail.SetTransactionId(pL3L2ProbeTrans->GetId());
    L3L2SysProbeReq.SetTransactionId(pL3L2ProbeTrans->GetId());

    if ( !pL3L2ProbeTrans->BeginTransact())
    {            
        pL3L2ProbeTrans->EndTransact();
        delete pL3L2ProbeTrans;
        return false;
    }
#ifdef WBBU_CODE
      CL3OamCommonReq L3L2SysProbeReqcore1; 
    if (false == L3L2SysProbeReqcore1.CreateMessage(*this))
        return false;
    L3L2SysProbeReqcore1.SetDstTid(M_TID_L2MAIN);  
    L3L2SysProbeReqcore1.SetSrcTid(M_TID_SYS);
    L3L2SysProbeReqcore1.SetMessageId(M_L3_L2_SYSPROBING_REQ_core1);
    L3L2SysProbeReqcore1.SetModule(1);
    CL3OamCommonReq L3L2ProbeReqFailcore1;
    if (false == L3L2ProbeReqFailcore1.CreateMessage(*this))
        {
        L3L2SysProbeReqcore1.DeleteMessage();
        return false;
        }
    L3L2ProbeReqFailcore1.SetDstTid(M_TID_SYS);
    L3L2ProbeReqFailcore1.SetSrcTid(M_TID_SYS);
    L3L2ProbeReqFailcore1.SetMessageId(M_OAM_PROBING_L2SYS_FAIL_core1);

    CTransaction *pL3L2ProbeTranscore1 = CreateTransact(L3L2SysProbeReqcore1, L3L2ProbeReqFailcore1, OAM_REQ_RESEND_L2_L3_CNT40, 5000);
    if (NULL == pL3L2ProbeTranscore1)
        {
        L3L2SysProbeReqcore1.DeleteMessage();
        L3L2ProbeReqFailcore1.DeleteMessage();
        return false;
        }
    L3L2ProbeReqFailcore1.SetTransactionId(pL3L2ProbeTranscore1->GetId());
    L3L2SysProbeReqcore1.SetTransactionId(pL3L2ProbeTranscore1->GetId());
    if(!pL3L2ProbeTranscore1->BeginTransact())
{
       pL3L2ProbeTranscore1->EndTransact();
		delete pL3L2ProbeTranscore1;
        return false;
}
#endif
    return true;
}


//若在周期SYS_BTS_REG_PERIOD内收到L2的应答信号,则认为l2l3链路没断.
//将链路状态上报到告警管理模块;
//      0-  clear   1-  set
bool CTaskSystem :: SYS_L3L2SysProbeRsp(CL3OamCommonRsp &rMsg)
{
    CTransaction *pTransaction = FindTransact(rMsg.GetTransactionId());
    if(!pTransaction)
    {
       // return false;  
    }
   else
   {
        pTransaction->EndTransact();
        delete pTransaction;
    }
    AlarmReport(ALM_FLAG_CLEAR,
                   ALM_ENT_L2PPC, 
                   ALM_ENT_INDEX0, 
                   ALM_ID_L2PPC_COMMFAIL,
                   ALM_CLASS_MAJOR,
                   STR_CLEAR_ALARM);

    return true;
}
#ifdef WBBU_CODE
//若在周期SYS_BTS_REG_PERIOD内收到L2的应答信号,则认为l2l3链路没断.
//将链路状态上报到告警管理模块;
//      0-  clear   1-  set
bool CTaskSystem :: SYS_L3L2SysProbeCore1Rsp(CL3OamCommonRsp &rMsg)
{
    CTransaction *pTransaction = FindTransact(rMsg.GetTransactionId());
    if(!pTransaction)
    {
       // return false;  
    }
   {
        pTransaction->EndTransact();
        delete pTransaction;
    }
    AlarmReport(ALM_FLAG_CLEAR,
                   ALM_ENT_L2PPC, 
                   ALM_ENT_INDEX0, 
                   ALM_ID_L2PPC_COMMFAIL,
                   ALM_CLASS_MAJOR,
                   STR_CLEAR_ALARM);

    return true;
}
 bool CTaskSystem :: SYS_L3L2SysProbeCore1Fail()
 {
          if (CPU_BOOT_SUCCESS == m_BtsCpuWorkStatus.OAM_L2PPC)
        {
        //layer 2启动后才开始报告警
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, STR_L3L2_Core1_LINKFAIL);
        AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L2PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_L2PPC_COMMFAIL,
                       ALM_CLASS_MAJOR,
                       STR_L3L2_Core1_LINKFAIL);    
                    Reset_Dsp(5,2);
        }
    return true;
 }

#endif
bool CTaskSystem :: SYS_L3L2SysProbeFail()
{
    if (CPU_BOOT_SUCCESS == m_BtsCpuWorkStatus.OAM_L2PPC)
        {
        //layer 2启动后才开始报告警
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, STR_L3L2LINKFAIL);
        AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L2PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_L2PPC_COMMFAIL,
                       ALM_CLASS_MAJOR,
                       STR_L3L2LINKFAIL);    
        }
    return true;
}


bool CTaskSystem :: SYS_L2L3AuxStateNotify(CL3L2AuxStateNoitfy& NoitfyMsg)
{
    const T_AUXStateInfo* cpInfo    = NoitfyMsg.GetAUXStateInfo(); 
#ifndef WBBU_CODE
    SINT8 aBuff[ALM_INFO_LEN]       = {0};
    SINT8 strAlmInfo[ALM_INFO_LEN]  = {0};
    bool  bExistSetAlarm            = false;
////GPS alarm: GSP信号告警
    if(cpInfo->GPS_MNT_FAILURE_CNT > 0)
        {
        if (0 == m_lastAUXStateInfo.GPS_MNT_FAILURE_CNT)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_GPS,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_GPS_SIGNAL,
                                   ALM_CLASS_CRITICAL,
                                   STR_GPS_MNT_FAILURE_CNT);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.GPS_MNT_FAILURE_CNT > 0)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_GPS,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_GPS_SIGNAL,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);
            }
        }

////本地时钟告警
    if(cpInfo->GPS_TRACKIN_FAILURE_CNT > 0)
        {
        if (0 == m_lastAUXStateInfo.GPS_TRACKIN_FAILURE_CNT)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_GPS,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_GPS_LOC_CLOCK,
                                   ALM_CLASS_CRITICAL,
                                   STR_GPS_TRACKIN_FAILURE_CNT);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.GPS_TRACKIN_FAILURE_CNT > 0)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_GPS,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_GPS_LOC_CLOCK,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);
            }
        }

////SYNC温度过高/低 关闭SYNC板
    if ((cpInfo->TemperatureML == 0)&&
        (cpInfo->TemperatureMH == 0))
        {
        /*温度恢复正常*/
        if ((m_lastAUXStateInfo.TemperatureML > 0)||
            (m_lastAUXStateInfo.TemperatureMH > 0))
            {
            /*上一次的温度异常*/
            //发恢复告警
            AlarmReport(ALM_FLAG_CLEAR,
                                    ALM_ENT_ENV,
                                    ALM_ENT_INDEX0,
                                    ALM_ID_ENV_SYNC_SHUTDOWN,
                                    ALM_CLASS_CRITICAL,
                                    STR_CLEAR_ALARM);
            }
        }
    else
        {
        if (((cpInfo->TemperatureML > 0) && (m_lastAUXStateInfo.TemperatureML == 0))||
            ((cpInfo->TemperatureMH > 0) && (m_lastAUXStateInfo.TemperatureMH == 0)))
            {
            //与上一次的温度异常情况不一样，仍然发告警
            AlarmReport(ALM_FLAG_SET,
                                    ALM_ENT_ENV,
                                    ALM_ENT_INDEX0,
                                    ALM_ID_ENV_SYNC_SHUTDOWN,
                                    ALM_CLASS_CRITICAL,
                                    STR_SYNC_SHUTDOWN,
                                    cpInfo->TemperatureML,
                                    cpInfo->TemperatureL, 
                                    cpInfo->TemperatureH,
                                    cpInfo->TemperatureMH);
            }
        }

////SYNC温度告警
    if ((cpInfo->TemperatureL  == 0)&&
        (cpInfo->TemperatureH  == 0))
        {
        /*温度恢复正常*/
        if ((m_lastAUXStateInfo.TemperatureL  > 0)||
            (m_lastAUXStateInfo.TemperatureH  > 0))
            {
            /*上一次的温度异常*/
            //发恢复告警
            AlarmReport(ALM_FLAG_CLEAR,
                                    ALM_ENT_ENV,
                                    ALM_ENT_INDEX0,
                                    ALM_ID_ENV_SYNC_TEMPERATURE,
                                    ALM_CLASS_CRITICAL,
                                    STR_CLEAR_ALARM);
            }
        }
    else
        {
        if (((cpInfo->TemperatureL  > 0) && (m_lastAUXStateInfo.TemperatureL  == 0))||
            ((cpInfo->TemperatureH  > 0) && (m_lastAUXStateInfo.TemperatureH  == 0)))
            {
            //与上一次的温度异常情况不一样，仍然发告警
            AlarmReport(ALM_FLAG_SET,
                                    ALM_ENT_ENV,
                                    ALM_ENT_INDEX0,
                                    ALM_ID_ENV_SYNC_TEMPERATURE,
                                    ALM_CLASS_CRITICAL,
                                    STR_SYNC_TEMPERATURE,
                                    cpInfo->TemperatureML,
                                    cpInfo->TemperatureL, 
                                    cpInfo->TemperatureH,
                                    cpInfo->TemperatureMH);
            }
        }

///////////////////////////////////////////////////////////
////AUX通信告警
#define M_AUX_ERROR     (50)    /*一秒钟内错误数超过M_AUX_ERROR则认为该告警*/
    if(cpInfo->AUX_To_L2_Control_Fail > M_AUX_ERROR)
        {
        if (M_AUX_ERROR >= m_lastAUXStateInfo.AUX_To_L2_Control_Fail)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_TO_L2_CONTROL_FAIL,
                                   ALM_CLASS_MAJOR,
                                   STR_AUX_TO_L2_CONTROL_FAIL);
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_AUX_TO_L2_CONTROL_FAIL);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.AUX_To_L2_Control_Fail > M_AUX_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_TO_L2_CONTROL_FAIL,
                                   ALM_CLASS_MAJOR,
                                   STR_CLEAR_ALARM);
            }
        }

////
    if(cpInfo->AUX_To_L2_Ranging_Buf_Not_Empty > M_AUX_ERROR)
        {
        if (M_AUX_ERROR >= m_lastAUXStateInfo.AUX_To_L2_Ranging_Buf_Not_Empty)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_TO_L2_RANGING_BUF_NOT_EMPTY,
                                   ALM_CLASS_MAJOR,
                                   STR_AUX_TO_L2_RANGING_BUF_NOT_EMPTY);
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_AUX_TO_L2_RANGING_BUF_NOT_EMPTY);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.AUX_To_L2_Ranging_Buf_Not_Empty > M_AUX_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_TO_L2_RANGING_BUF_NOT_EMPTY,
                                   ALM_CLASS_MAJOR,
                                   STR_CLEAR_ALARM);
            }
        }

////
    if(cpInfo->L2_To_Aux_Control_Fail > M_AUX_ERROR)
        {
        if (M_AUX_ERROR >= m_lastAUXStateInfo.L2_To_Aux_Control_Fail)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_L2_TO_AUX_CONTROL_FAIL,
                                   ALM_CLASS_MAJOR,
                                   STR_L2_TO_AUX_CONTROL_FAIL);
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_TO_AUX_CONTROL_FAIL);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.L2_To_Aux_Control_Fail > M_AUX_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_L2_TO_AUX_CONTROL_FAIL,
                                   ALM_CLASS_MAJOR,
                                   STR_CLEAR_ALARM);
            }
        }

    for (UINT8 ucTimeSlot = 0; ucTimeSlot < DL_TIME_SLOT_NUM; ++ucTimeSlot)
        {
        ////////
        if(cpInfo->L2AUXStateInfo[ucTimeSlot].L2_To_AUX_Weight_Not_Empty > M_AUX_ERROR)
            {
            if (M_AUX_ERROR >= m_lastAUXStateInfo.L2AUXStateInfo[ucTimeSlot].L2_To_AUX_Weight_Not_Empty)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_L2_TO_AUX_WEIGHT_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_L2_TO_AUX_WEIGHT_NOT_EMPTY, ucTimeSlot);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_TO_AUX_WEIGHT_NOT_EMPTY, ucTimeSlot);
                }
            }
        else
            {
            if (m_lastAUXStateInfo.L2AUXStateInfo[ucTimeSlot].L2_To_AUX_Weight_Not_Empty > M_AUX_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_L2_TO_AUX_WEIGHT_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        ////////
        if(cpInfo->L2AUXStateInfo[ucTimeSlot].L2_To_AUX_Config_Not_Empty > M_AUX_ERROR)
            {
            if (M_AUX_ERROR >= m_lastAUXStateInfo.L2AUXStateInfo[ucTimeSlot].L2_To_AUX_Config_Not_Empty)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_L2_TO_AUX_CONFIG_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_L2_TO_AUX_CONFIG_NOT_EMPTY, ucTimeSlot);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_TO_AUX_CONFIG_NOT_EMPTY, ucTimeSlot);
                }
            }
        else
            {
            if (m_lastAUXStateInfo.L2AUXStateInfo[ucTimeSlot].L2_To_AUX_Config_Not_Empty > M_AUX_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_L2_TO_AUX_CONFIG_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        ////////
        if(cpInfo->L2AUXStateInfo[ucTimeSlot].L2_From_AUX_Ranging_Not_Full > M_AUX_ERROR)
            {
            if (M_AUX_ERROR >= m_lastAUXStateInfo.L2AUXStateInfo[ucTimeSlot].L2_From_AUX_Ranging_Not_Full)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_L2_FROM_AUX_RANGING_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_L2_FROM_AUX_RANGING_NOT_FULL, ucTimeSlot);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_FROM_AUX_RANGING_NOT_FULL, ucTimeSlot);
                }
            }
        else
            {
            if (m_lastAUXStateInfo.L2AUXStateInfo[ucTimeSlot].L2_From_AUX_Ranging_Not_Full > M_AUX_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_L2_FROM_AUX_RANGING_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        ////////
        if(cpInfo->L2AUXStateInfo[ucTimeSlot].L2_From_AUX_Ranging_CHKSUM > M_AUX_ERROR)
            {
            if (M_AUX_ERROR >= m_lastAUXStateInfo.L2AUXStateInfo[ucTimeSlot].L2_From_AUX_Ranging_CHKSUM)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_L2_FROM_AUX_RANGING_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_L2_FROM_AUX_RANGING_CHKSUM, ucTimeSlot);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_FROM_AUX_RANGING_CHKSUM, ucTimeSlot);
                }
            }
        else
            {
            if (m_lastAUXStateInfo.L2AUXStateInfo[ucTimeSlot].L2_From_AUX_Ranging_CHKSUM > M_AUX_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_L2_FROM_AUX_RANGING_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        ////////
        if(cpInfo->AUXL2StateInfo[ucTimeSlot].AUX_From_L2_Buf_Not_Full > M_AUX_ERROR)
            {
            if (M_AUX_ERROR >= m_lastAUXStateInfo.AUXL2StateInfo[ucTimeSlot].AUX_From_L2_Buf_Not_Full)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_AUX_FROM_L2_BUF_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_AUX_FROM_L2_BUF_NOT_FULL, ucTimeSlot);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_AUX_FROM_L2_BUF_NOT_FULL, ucTimeSlot);
                }
            }
        else
            {
            if (m_lastAUXStateInfo.AUXL2StateInfo[ucTimeSlot].AUX_From_L2_Buf_Not_Full > M_AUX_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_AUX_FROM_L2_BUF_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        ////////
        if(cpInfo->AUXL2StateInfo[ucTimeSlot].AUX_From_L2_CHKSUM > M_AUX_ERROR)
            {
            if (M_AUX_ERROR >= m_lastAUXStateInfo.AUXL2StateInfo[ucTimeSlot].AUX_From_L2_CHKSUM)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_AUX_FROM_L2_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_AUX_FROM_L2_CHKSUM, ucTimeSlot);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_AUX_FROM_L2_CHKSUM, ucTimeSlot);
                }
            }
        else
            {
            if (m_lastAUXStateInfo.AUXL2StateInfo[ucTimeSlot].AUX_From_L2_CHKSUM > M_AUX_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_AUX_FROM_L2_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

////////AUX to FEP Buffer not Empty!
        memset(aBuff,      0, sizeof(aBuff));
        memset(strAlmInfo, 0, sizeof(strAlmInfo));
        bExistSetAlarm = false;

        //查是否有新产生的告警.
        for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
            {
            if(cpInfo->AuxFepStateInfo[ucTimeSlot][ucFepNum].AUX_To_FEP_Buf_Not_Empty > M_AUX_ERROR)
                {
                bExistSetAlarm = true;
                sprintf(aBuff, STR_AUX_TO_FEP_BUF_NOT_EMPTY, ucFepNum, ucTimeSlot);
                if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                    strcat(strAlmInfo, aBuff);        
                else
                    break;
                }
            }
        if (false == bExistSetAlarm)
            {
            //没有错误,查前次的状态，看是否需要发恢复告警
            bool bMustSendClearAlarm = false;
            for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
                {
                if(m_lastAUXStateInfo.AuxFepStateInfo[ucTimeSlot][ucFepNum].AUX_To_FEP_Buf_Not_Empty > M_AUX_ERROR)
                    {
                    bMustSendClearAlarm = true;
                    break;
                    }
                }
            if (true == bMustSendClearAlarm)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_AUX_TO_FEP_BUF_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }
        else
            {
            //有错误发生,查是否和前次的错误一样，不一样则发告警
            bool bNoChange = true;
            for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
                {
                if (((M_AUX_ERROR <  cpInfo->AuxFepStateInfo[ucTimeSlot][ucFepNum].AUX_To_FEP_Buf_Not_Empty)
                   &&(M_AUX_ERROR >= m_lastAUXStateInfo.AuxFepStateInfo[ucTimeSlot][ucFepNum].AUX_To_FEP_Buf_Not_Empty))
                  ||((M_AUX_ERROR >= cpInfo->AuxFepStateInfo[ucTimeSlot][ucFepNum].AUX_To_FEP_Buf_Not_Empty)
                   &&(M_AUX_ERROR <  m_lastAUXStateInfo.AuxFepStateInfo[ucTimeSlot][ucFepNum].AUX_To_FEP_Buf_Not_Empty)))
                    {
                    bNoChange = false;
                    break;
                    }
                }
            if (false == bNoChange)
                {
                //send alarm.
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_AUX_TO_FEP_BUF_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       strAlmInfo);
                OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, strAlmInfo);
                }
            }

////////FEP from AUX Buffer and Checksum error.
        memset(aBuff,      0, sizeof(aBuff));
        memset(strAlmInfo, 0, sizeof(strAlmInfo));
        bExistSetAlarm = false;

        //查是否有新产生的告警.
        for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
            {
            if(cpInfo->FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_Buf_Not_Full > M_AUX_ERROR)
                {
                bExistSetAlarm = true;
                sprintf(aBuff, STR_FEP_FROM_AUX_BUF_NOT_FULL, ucFepNum, ucTimeSlot);
                if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                    strcat(strAlmInfo, aBuff);        
                else
                    break;
                }

            if(cpInfo->FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_CHKSUM > M_AUX_ERROR)
                {
                bExistSetAlarm = true;
                sprintf(aBuff, STR_FEP_FROM_AUX_CHKSUM, ucFepNum, ucTimeSlot);
                if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                    strcat(strAlmInfo, aBuff);        
                else
                    break;
                }
            }
        if (false == bExistSetAlarm)
            {
            //没有错误,查前次的状态，看是否需要发恢复告警
            bool bMustSendClearAlarm = false;
            for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
                {
                if((m_lastAUXStateInfo.FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_Buf_Not_Full > M_AUX_ERROR)
                    ||(m_lastAUXStateInfo.FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_CHKSUM > M_AUX_ERROR))
                    {
                    bMustSendClearAlarm = true;
                    break;
                    }
                }
            if (true == bMustSendClearAlarm)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_FEP_FROM_AUX_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }
        else
            {
            //有错误发生,查是否和前次的错误一样，不一样则发告警
            bool bNoChange = true;
            for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
                {
                if (((M_AUX_ERROR <  cpInfo->FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_Buf_Not_Full)
                  && (M_AUX_ERROR >= m_lastAUXStateInfo.FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_Buf_Not_Full))
                  ||((M_AUX_ERROR >= cpInfo->FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_Buf_Not_Full)
                  && (M_AUX_ERROR <  m_lastAUXStateInfo.FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_Buf_Not_Full)))
                    {
                    bNoChange = false;
                    break;
                    }

                if (((M_AUX_ERROR <  cpInfo->FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_CHKSUM)
                  && (M_AUX_ERROR >= m_lastAUXStateInfo.FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_CHKSUM))
                  ||((M_AUX_ERROR >= cpInfo->FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_CHKSUM)
                  && (M_AUX_ERROR <  m_lastAUXStateInfo.FepAuxStateInfo[ucTimeSlot][ucFepNum].FEP_From_AUX_CHKSUM)))
                    {
                    bNoChange = false;
                    break;
                    }
                }
            if (false == bNoChange)
                {
                //send alarm.
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_AUX,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_AUX_FEP_FROM_AUX_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       strAlmInfo);
                OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, strAlmInfo);
                }
            }
        }

////////FEP from AUX Buffer and Checksum error.
    memset(aBuff,      0, sizeof(aBuff));
    memset(strAlmInfo, 0, sizeof(strAlmInfo));
    bExistSetAlarm = false;

    //查是否有新产生的告警.
    for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
        {
        if(cpInfo->AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_Buf_Not_Full > M_AUX_ERROR)
            {
            bExistSetAlarm = true;
            sprintf(aBuff, STR_AUX_FROM_FEP_BUF_NOT_FULL, ucFepNum);
            if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                strcat(strAlmInfo, aBuff);        
            else
                break;
            }

        if(cpInfo->AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_CHKSUM > M_AUX_ERROR)
            {
            bExistSetAlarm = true;
            sprintf(aBuff, STR_AUX_FROM_FEP_CHKSUM, ucFepNum);
            if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                strcat(strAlmInfo, aBuff);        
            else
                break;
            }

        if(cpInfo->AuxFromFepStateInfo[ucFepNum].FEP_To_AUX_Buf_Not_Empty > M_AUX_ERROR)
            {
            bExistSetAlarm = true;
            sprintf(aBuff, STR_FEP_TO_AUX_BUF_NOT_EMPTY, ucFepNum);
            if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                strcat(strAlmInfo, aBuff);        
            else
                break;
            }
        }
    if (false == bExistSetAlarm)
        {
        //没有错误,查前次的状态，看是否需要发恢复告警
        bool bMustSendClearAlarm = false;
        for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
            {
            if((m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_Buf_Not_Full > M_AUX_ERROR)
                ||(m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_CHKSUM > M_AUX_ERROR)
                ||(m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].FEP_To_AUX_Buf_Not_Empty > M_AUX_ERROR))
                {
                bMustSendClearAlarm = true;
                break;
                }
            }
        if (true == bMustSendClearAlarm)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM,
                                   ALM_CLASS_MAJOR,
                                   STR_CLEAR_ALARM);
            }
        }
    else
        {
        //有错误发生,查是否和前次的错误一样，不一样则发告警
        bool bNoChange = true;
        for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
            {
            if (((M_AUX_ERROR < cpInfo->AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_Buf_Not_Full)
              && (M_AUX_ERROR >= m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_Buf_Not_Full))
              ||((M_AUX_ERROR >= cpInfo->AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_Buf_Not_Full)
              && (M_AUX_ERROR <  m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_Buf_Not_Full)))
                {
                bNoChange = false;
                break;
                }
            if (((M_AUX_ERROR < cpInfo->AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_CHKSUM)
              && (M_AUX_ERROR >= m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_CHKSUM))
              ||((M_AUX_ERROR >= cpInfo->AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_CHKSUM)
              && (M_AUX_ERROR <  m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].AUX_From_FEP_CHKSUM)))
                {
                bNoChange = false;
                break;
                }
            if (((M_AUX_ERROR < cpInfo->AuxFromFepStateInfo[ucFepNum].FEP_To_AUX_Buf_Not_Empty)
              && (M_AUX_ERROR >= m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].FEP_To_AUX_Buf_Not_Empty))
              ||((M_AUX_ERROR >= cpInfo->AuxFromFepStateInfo[ucFepNum].FEP_To_AUX_Buf_Not_Empty)
              && (M_AUX_ERROR <  m_lastAUXStateInfo.AuxFromFepStateInfo[ucFepNum].FEP_To_AUX_Buf_Not_Empty)))
                {
                bNoChange = false;
                break;
                }
            }
        if (false == bNoChange)
            {
            //send alarm.
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM,
                                   ALM_CLASS_MAJOR,
                                   strAlmInfo);
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, strAlmInfo);
            }
        }

////BTS fan alarm: BTS风扇告警
////FAN_status的bit0~bit2分别表示对应风扇的状态，
////0:风扇正常运转，1:风扇停止
    UINT32 mask = 0xE0000000;
    if(cpInfo->FAN_status & mask > 0)
        {
        if (cpInfo->FAN_status & mask != m_lastAUXStateInfo.FAN_status & mask)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_ENV,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_ENV_FAN_STOP,
                                   ALM_CLASS_CRITICAL,
                                   STR_ENV_FAN_STOP, cpInfo->FAN_status & mask);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.FAN_status & mask > 0)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_ENV,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_ENV_FAN_STOP,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);
            }
        }


    memcpy(&m_lastAUXStateInfo, cpInfo, sizeof(m_lastAUXStateInfo));
	#else
    int fepindex;
       #define M_MCP_ERROR     (50)    /*一秒钟内错误数超过M_MCP_ERROR则认为该告警*/
	   if(cpInfo->Core9_From_AUX_Message_Lost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAUXStateInfo.Core9_From_AUX_Message_Lost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_FROM_AUX_RECVMSG_LOST,
                                   ALM_CLASS_INFO,
                                   STR_Core9_From_AUX_Message_Lost);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.Core9_From_AUX_Message_Lost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_FROM_AUX_RECVMSG_LOST,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

////本地时钟告警
    if(cpInfo->Core9_From_AUX_Message_CHKSUM > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >=  m_lastAUXStateInfo.Core9_From_AUX_Message_CHKSUM)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_FROM_AUX_CHECKSUM_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_Core9_From_AUX_Message_CHKSUM);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.Core9_From_AUX_Message_CHKSUM > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_FROM_AUX_CHECKSUM_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

////SYNC温度过高/低 关闭SYNC板
    if ((cpInfo->AUX_From_Core9_Config_Lost >M_MCP_ERROR))
    	{
         if ((M_MCP_ERROR >=  m_lastAUXStateInfo.AUX_From_Core9_Config_Lost))
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_CORE9_DOWN_DATA_ALM,
                                   ALM_CLASS_INFO,
                                   STR_AUX_From_Core9_Config_Lost);
            }
        }
    else
        {
        if((m_lastAUXStateInfo.AUX_From_Core9_Config_Lost > M_MCP_ERROR))
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_CORE9_DOWN_DATA_ALM,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

        if ((cpInfo->AUX_From_Core9_Config_CHKSUM > M_MCP_ERROR))
    	{
         if ((M_MCP_ERROR >=  m_lastAUXStateInfo.AUX_From_Core9_Config_CHKSUM))
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_CORE9_DOWN_DATA_ALM,
                                   ALM_CLASS_INFO,
                                   STR_AUX_From_Core9_Config_CHKSUM);
            }
        }
    else
        {
        if((m_lastAUXStateInfo.AUX_From_Core9_Config_CHKSUM > M_MCP_ERROR))
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AUX,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_CORE9_DOWN_DATA_ALM,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
     
       if(cpInfo->AUX_From_FPGA_Message_Lost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >=  m_lastAUXStateInfo.AUX_From_FPGA_Message_Lost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_FPGA_RecMSG_ALM,
                                   ALM_CLASS_INFO,
                                   STR_AUX_From_FPGA_Message_Lost);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.AUX_From_FPGA_Message_Lost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_FPGA_RecMSG_ALM,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


	   if(cpInfo->AUX_From_FPGA_Message_CHKSUM > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >=  m_lastAUXStateInfo.AUX_From_FPGA_Message_CHKSUM)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_FPGA_CHECKSUM_ALM,
                                   ALM_CLASS_INFO,
                                   STR_AUX_From_FPGA_Message_CHKSUM);
            }
        }
    else
        {
        if (m_lastAUXStateInfo.AUX_From_FPGA_Message_CHKSUM > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_AUX_AUX_FROM_FPGA_CHECKSUM_ALM,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
       for(fepindex= 0; fepindex< 2;fepindex++)
       {

	 if(cpInfo->FEP_From_AUX_Message_CHKSUM[fepindex] > M_MCP_ERROR)
        {
	        if (M_MCP_ERROR >=  m_lastAUXStateInfo.FEP_From_AUX_Message_CHKSUM[fepindex])
	            {
	            AlarmReport(ALM_FLAG_SET,
	                                   ALM_ENT_FEP,
	                                   fepindex,
	                                   ALM_ID_FEP_AUX_CFG_MSG_ERROR,
	                                   ALM_CLASS_INFO,
	                                   STR_FEP_From_AUX_Message_CHKSUM,fepindex);
	            }
        }
    else
        {
        if (m_lastAUXStateInfo.FEP_From_AUX_Message_CHKSUM[fepindex] > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_FEP_AUX_CFG_MSG_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }



	  if(cpInfo->AUX_From_FEP_Message_CHKSUM[fepindex] > M_MCP_ERROR)
        {
	        if (M_MCP_ERROR >=  m_lastAUXStateInfo.AUX_From_FEP_Message_CHKSUM[fepindex])
	            {
	            AlarmReport(ALM_FLAG_SET,
	                                   ALM_ENT_AUX,
	                                   ALM_ENT_INDEX0,
	                                   ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM,
	                                   ALM_CLASS_INFO,
	                                   STR_AUX_From_FEP_Message_CHKSUM,fepindex);
	            }
        }
    else
        {
	        if (m_lastAUXStateInfo.AUX_From_FEP_Message_CHKSUM[fepindex] > M_MCP_ERROR)
	            {
	            //clear alarm.
	            AlarmReport(ALM_FLAG_CLEAR,
	                                   ALM_ENT_AUX,
	                                   ALM_ENT_INDEX0,
	                                   ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM,
	                                   ALM_CLASS_INFO,
	                                   STR_CLEAR_ALARM);
	            }
        }
       }

	


    memcpy(&m_lastAUXStateInfo, cpInfo, sizeof(m_lastAUXStateInfo));
#endif
    return true;
}
#ifdef WBBU_CODE




bool CTaskSystem ::SYS_L2L3Core9StateNotify(CL3L2Core9StateNoitfy& NoitfyMsg)
{



      const T_Core9StateInfo* cpInfo = NoitfyMsg.GetCORE9StateInfo(); 
    
//    const T_Core9StateInfo *pLastMCPStateInfo = &m_lastCore9StateInfo;
	#define M_MCP_ERROR     (50)    /*一秒钟内错误数超过M_MCP_ERROR则认为该告警*/
	//int fepindex;
    //开始构造告警信息
	   if(cpInfo->L2_From_Core9_UplinkData_Lost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastCore9StateInfo.L2_From_Core9_UplinkData_Lost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_L2PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_L2PPC_CORE9_UPLINK_DATA_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_L2_From_Core9_UplinkData_Lost);
            }
        }
    else
        {
        if (m_lastCore9StateInfo.L2_From_Core9_UplinkData_Lost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_L2PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_L2PPC_CORE9_UPLINK_DATA_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

////本地时钟告警
    if(cpInfo->L2_From_Core9_UplinkData_CHKSUM > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >=  m_lastCore9StateInfo.L2_From_Core9_UplinkData_CHKSUM)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_L2PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_L2PPC_CORE9_UPLINK_DATA_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_L2_From_Core9_UplinkData_CHKSUM);
            }
        }
    else
        {
        if (m_lastCore9StateInfo.L2_From_Core9_UplinkData_CHKSUM > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_L2PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_L2PPC_CORE9_UPLINK_DATA_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


    if ((cpInfo->L2_From_Core9_Message_Lost >M_MCP_ERROR))
    	{
         if ((M_MCP_ERROR >=  m_lastCore9StateInfo.L2_From_Core9_Message_Lost))
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_L2PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_L2PPC_CORE9_MSG_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_L2_From_Core9_Message_Lost);
            }
        }
    else
        {
        if((m_lastCore9StateInfo.L2_From_Core9_Message_Lost > M_MCP_ERROR))
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_L2PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_L2PPC_CORE9_MSG_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


         if ((cpInfo->L2_From_Core9_Message_CHKSUM > M_MCP_ERROR))
    	{
         if ((M_MCP_ERROR >=  m_lastCore9StateInfo.L2_From_Core9_Message_CHKSUM))
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_L2PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_L2PPC_CORE9_MSG_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_L2_From_Core9_Message_CHKSUM);
            }
        }
    else
        {
        if((m_lastCore9StateInfo.L2_From_Core9_Message_CHKSUM > M_MCP_ERROR))
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_L2PPC,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_L2PPC_CORE9_MSG_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

    
      if ((cpInfo->Core9_From_L2_UpProf_CHKSUM > M_MCP_ERROR))
    	{
         if ((M_MCP_ERROR >=  m_lastCore9StateInfo.Core9_From_L2_UpProf_CHKSUM))
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_L2_UP_PROFILE_WARN,
                                   ALM_CLASS_INFO,
                                  STR_Core9_From_L2_UpProf_CHKSUM);
            }
        }
    else
        {
        if((m_lastCore9StateInfo.Core9_From_L2_UpProf_CHKSUM> M_MCP_ERROR))
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_L2_UP_PROFILE_WARN,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

	if (
        (cpInfo->Core9_From_L2_DownlinkData_CHKSUM > M_MCP_ERROR))
    	{
         if ((M_MCP_ERROR >=  m_lastCore9StateInfo.Core9_From_L2_DownlinkData_CHKSUM))
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_L2_DOWN_DATA_WARN,
                                   ALM_CLASS_INFO,
                                 STR_Core9_From_L2_DownlinkData_CHKSUM);
            }
        }
    else
        {
        if((m_lastCore9StateInfo.Core9_From_L2_UpProf_CHKSUM> M_MCP_ERROR))
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_L2_DOWN_DATA_WARN,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }




if ((cpInfo->Core9_From_L2_Config_CHKSUM> M_MCP_ERROR))
    	{
         if ((M_MCP_ERROR >=  m_lastCore9StateInfo.Core9_From_L2_Config_CHKSUM))
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_L2_CFG_MSG_WARN,
                                   ALM_CLASS_INFO,
                                   STR_Core9_From_L2_Config_CHKSUM);
            }
        }
    else
        {
        if((m_lastCore9StateInfo.Core9_From_L2_Config_CHKSUM> M_MCP_ERROR))
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_L2_CFG_MSG_WARN,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


	
       if(cpInfo->Core9_From_L2_Timeout > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >=  m_lastCore9StateInfo.Core9_From_L2_Timeout)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_L2_RSP_TIMEOUT,
                                   ALM_CLASS_INFO,
                                   STR_Core9_From_L2_Timeout);
            }
        }
    else
        {
        if (m_lastCore9StateInfo.Core9_From_L2_Timeout > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_CORE9,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_CORE9_L2_RSP_TIMEOUT,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


	 
       for(int fepindex= 0; fepindex<2;fepindex++)
       {

		 if((cpInfo->fepfromL2Weight[fepindex].FEP_From_L2_Weight_Lost> M_MCP_ERROR))
	        {
	        if ((M_MCP_ERROR >=  m_lastCore9StateInfo.fepfromL2Weight[fepindex].FEP_From_L2_Weight_Lost))
	            {
	            AlarmReport(ALM_FLAG_SET,
	                                   ALM_ENT_FEP,
	                                   fepindex,
	                                   ALM_ID_FEP_MCP_DOWN_WEIGHT_ERROR,
	                                   ALM_CLASS_INFO,
	                                   STR_FEP_From_L2_Weight_Lost,fepindex);
	            }
	        }
	    else
	        {
	        if ((m_lastCore9StateInfo.fepfromL2Weight[fepindex].FEP_From_L2_Weight_Lost > M_MCP_ERROR))
	            {
	            //clear alarm.
	            AlarmReport(ALM_FLAG_CLEAR,
	                                   ALM_ENT_FEP,
	                                   fepindex,
	                                   ALM_ID_FEP_MCP_DOWN_WEIGHT_ERROR,
	                                   ALM_CLASS_INFO,
	                                   STR_CLEAR_ALARM);
	            }
	        }

           if((cpInfo->fepfromL2Weight[fepindex].FEP_From_L2_Weight_CHKSUM> M_MCP_ERROR))
	        {
	        if ((M_MCP_ERROR >= m_lastCore9StateInfo.fepfromL2Weight[fepindex].FEP_From_L2_Weight_CHKSUM))
	            {
	            AlarmReport(ALM_FLAG_SET,
	                                   ALM_ENT_FEP,
	                                   fepindex,
	                                   ALM_ID_FEP_MCP_DOWN_WEIGHT_ERROR,
	                                   ALM_CLASS_INFO,
	                                   STR_FEP_From_L2_Weight_CHKSUM,fepindex);
	            }
	        }
	    else
	        {
	        if ((m_lastCore9StateInfo.fepfromL2Weight[fepindex].FEP_From_L2_Weight_CHKSUM> M_MCP_ERROR))
	            {
	            //clear alarm.
	            AlarmReport(ALM_FLAG_CLEAR,
	                                   ALM_ENT_FEP,
	                                   fepindex,
	                                   ALM_ID_FEP_MCP_DOWN_WEIGHT_ERROR,
	                                   ALM_CLASS_INFO,
	                                   STR_CLEAR_ALARM);
	            }
	        }


	
       }
    memcpy(&m_lastCore9StateInfo, cpInfo, sizeof(T_Core9StateInfo));
     return true;
}

bool CTaskSystem :: SYS_L2L3AifInfoStateNotify(CL3L2AifStateNoitfy& NotifyMsg)
{
        
	static  unsigned int  link_err_time = 0;
      const T_AifStateInfo* cpInfo = NotifyMsg.GetAifStateInfo(); 
    
   //   const T_AifStateInfo  *pLastAifInfo = &m_lastAifInfo;
	#define M_MCP_ERROR     (50)    /*一秒钟内错误数超过M_MCP_ERROR则认为该告警*/
	//int fepindex;

	   if(cpInfo->Dsp2_To_AUX_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp2_To_AUX_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                    Alarm_ID_AIF_Dsp2_To_AUX_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp2_To_AUX_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp2_To_AUX_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp2_To_AUX_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

    //Dsp2_To_AUX_HeaderLost
	 if(cpInfo->Dsp2_To_AUX_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp2_To_AUX_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp2_To_AUX_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp2_To_AUX_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp2_To_AUX_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                    Alarm_ID_AIF_Dsp2_To_AUX_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

//  UINT32 Dsp3_To_AUX_Linkerror;
      	   if(cpInfo->Dsp3_To_AUX_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp3_To_AUX_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_To_AUX_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp3_To_AUX_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp3_To_AUX_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_To_AUX_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//  UINT32 Dsp3_To_AUX_HeaderLost;
    
    	   if(cpInfo->Dsp3_To_AUX_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp3_To_AUX_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_To_AUX_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp3_To_AUX_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp3_To_AUX_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_To_AUX_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

  //UINT32 Dsp4_To_AUX_Linkerror;
    	   if(cpInfo->Dsp4_To_AUX_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp4_To_AUX_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_AUX_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp4_To_AUX_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp4_To_AUX_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_AUX_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

    //  UINT32 Dsp4_To_AUX_HeaderLost;
    	   if(cpInfo->Dsp4_To_AUX_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp4_To_AUX_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_AUX_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp4_To_AUX_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp4_To_AUX_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_AUX_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


//  UINT32 FPGA_To_AUX_Linkerror;
    
    	   if(cpInfo->FPGA_To_AUX_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.FPGA_To_AUX_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FPGA_To_AUX_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_FPGA_To_AUX_Linkerror);
            }
		link_err_time++;
		if(link_err_time%5==0)//wangwenhua add 2012-5-30 增加保护，当连续5s出现link err时，复位Serdies
		{
		    OAM_LOGSTR(M_LL_DEBUG0, 0, "[tSys] FPGA to Aux link err over 5s ResetAifSerdies!");
		    ResetAifSerdies();
		}
        }
    else
        {
        if (m_lastAifInfo.FPGA_To_AUX_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FPGA_To_AUX_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

    //  UINT32 FPGA_To_AUX_Timeout;
    	   if(cpInfo->FPGA_To_AUX_Timeout > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.FPGA_To_AUX_Timeout)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FPGA_To_AUX_Timeout,
                                   ALM_CLASS_INFO,
                                   STR_AIF_FPGA_To_AUX_Timeout);
            }
        }
    else
        {
        if (m_lastAifInfo.FPGA_To_AUX_Timeout > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FPGA_To_AUX_Timeout,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//AUX_AIF_Reconfig
  
    	   if(cpInfo->AUX_AIF_Reconfig > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.AUX_AIF_Reconfig)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_AUX_AIF_Reconfig,
                                   ALM_CLASS_INFO,
                                   STR_AIF_AUX_AIF_Reconfig);
            }
        }
    else
        {
        if (m_lastAifInfo.AUX_AIF_Reconfig > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_AUX_AIF_Reconfig,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//Dsp4_To_Dsp2_Linkerror
    
    	   if(cpInfo->Dsp4_To_Dsp2_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp4_To_Dsp2_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_Dsp2_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp4_To_Dsp2_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp4_To_Dsp2_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_Dsp2_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

    //Dsp4_To_Dsp2_HeaderLost
    	   if(cpInfo->Dsp4_To_Dsp2_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp4_To_Dsp2_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_Dsp2_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp4_To_Dsp2_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp4_To_Dsp2_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_Dsp2_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


    //FEP_To_Dsp2_Linkerror
    	   if(cpInfo->FEP_To_Dsp2_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.FEP_To_Dsp2_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp2_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_FEP_To_Dsp2_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.FEP_To_Dsp2_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp2_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//FEP_To_Dsp2_HeaderLost
    
    	   if(cpInfo->FEP_To_Dsp2_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.FEP_To_Dsp2_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp2_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_FEP_To_Dsp2_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.FEP_To_Dsp2_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp2_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

//Dsp2_AIF_Reconfig
    	   if(cpInfo->Dsp2_AIF_Reconfig > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp2_AIF_Reconfig)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp2_AIF_Reconfig,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp2_AIF_Reconfig);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp2_AIF_Reconfig > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp2_AIF_Reconfig,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

//Dsp4_To_Dsp3_Linkerror
    
    	   if(cpInfo->Dsp4_To_Dsp3_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp4_To_Dsp3_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_Dsp3_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp4_To_Dsp3_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp4_To_Dsp3_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_Dsp3_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//Dsp4_To_Dsp3_HeaderLost
    
    	   if(cpInfo->Dsp4_To_Dsp3_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp4_To_Dsp3_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_Dsp3_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp4_To_Dsp3_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp4_To_Dsp3_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp4_To_Dsp3_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

//FEP_To_Dsp3_Linkerror
    
    	   if(cpInfo->FEP_To_Dsp3_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.FEP_To_Dsp3_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp3_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_FEP_To_Dsp3_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.FEP_To_Dsp3_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp3_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//FEP_To_Dsp3_HeaderLost
    
    	   if(cpInfo->FEP_To_Dsp3_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.FEP_To_Dsp3_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp3_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_FEP_To_Dsp3_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.FEP_To_Dsp3_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp3_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


//Dsp3_AIF_Reconfig
    
    	   if(cpInfo->Dsp3_AIF_Reconfig > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp3_AIF_Reconfig)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_AIF_Reconfig,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp3_AIF_Reconfig);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp3_AIF_Reconfig > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_AIF_Reconfig,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

   // FEP_To_Dsp4_Linkerror
    	   if(cpInfo->FEP_To_Dsp4_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.FEP_To_Dsp4_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp4_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_FEP_To_Dsp4_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.FEP_To_Dsp4_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp4_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

    //FEP_To_Dsp4_HeaderLost
    	   if(cpInfo->FEP_To_Dsp4_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.FEP_To_Dsp4_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp4_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_FEP_To_Dsp4_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.FEP_To_Dsp4_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_FEP_To_Dsp4_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }


//AUX_To_Core9_Linkerror
    	   if(cpInfo->AUX_To_Core9_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.AUX_To_Core9_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_AUX_To_Core9_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_AUX_To_Core9_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.AUX_To_Core9_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_AUX_To_Core9_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//AUX_To_Core9_HeaderLost
    
    	   if(cpInfo->AUX_To_Core9_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.AUX_To_Core9_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_AUX_To_Core9_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_AUX_To_Core9_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.AUX_To_Core9_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_AUX_To_Core9_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

//Dsp2_To_Core9_Linkerror
    
    	   if(cpInfo->Dsp2_To_Core9_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp2_To_Core9_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp2_To_Core9_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp2_To_Core9_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp2_To_Core9_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp2_To_Core9_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

//Dsp2_To_Core9_HeaderLost
    
    	   if(cpInfo->Dsp2_To_Core9_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp2_To_Core9_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp2_To_Core9_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp2_To_Core9_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp2_To_Core9_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp2_To_Core9_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

//Dsp3_To_Core9_Linkerror

    	   if(cpInfo->Dsp3_To_Core9_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp3_To_Core9_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_To_Core9_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp3_To_Core9_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp3_To_Core9_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_To_Core9_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }

    //Dsp3_To_Core9_HeaderLost
    	   if(cpInfo->Dsp3_To_Core9_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp3_To_Core9_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_To_Core9_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp3_To_Core9_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp3_To_Core9_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp3_To_Core9_HeaderLost,
                                   ALM_CLASS_INFO,
            STR_CLEAR_ALARM);
            }
        }

    //Dsp5_To_Core9_Linkerror
    	   if(cpInfo->Dsp5_To_Core9_Linkerror > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp5_To_Core9_Linkerror)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp5_To_Core9_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp5_To_Core9_Linkerror);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp5_To_Core9_Linkerror > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp5_To_Core9_Linkerror,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//Dsp5_To_Core9_HeaderLost
    
    	   if(cpInfo->Dsp5_To_Core9_HeaderLost > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Dsp5_To_Core9_HeaderLost)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp5_To_Core9_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_AIF_Dsp5_To_Core9_HeaderLost);
            }
        }
    else
        {
        if (m_lastAifInfo.Dsp5_To_Core9_HeaderLost > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Dsp5_To_Core9_HeaderLost,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//Core9_To_Dsp5_Handshake_Timeout
    
    	   if(cpInfo->Core9_To_Dsp5_Handshake_Timeout > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.Core9_To_Dsp5_Handshake_Timeout)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Core9_To_Dsp5_Handshake_Timeout,
                                   ALM_CLASS_CRITICAL,
                                   STR_AIF_Core9_To_Dsp5_Handshake_Timeout);
            }
        }
    else
        {
        if (m_lastAifInfo.Core9_To_Dsp5_Handshake_Timeout > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_Core9_To_Dsp5_Handshake_Timeout,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
//    AIF_Reconfig
    	   if(cpInfo->AIF_Reconfig > M_MCP_ERROR)
        {
        if (M_MCP_ERROR >= m_lastAifInfo.AIF_Reconfig)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_AIF_Reconfig,
                                   ALM_CLASS_INFO,
                                   STR_AIF_AIF_Reconfig);
            }
        }
    else
        {
        if (m_lastAifInfo.AIF_Reconfig > M_MCP_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_AIF,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_AIF_AIF_Reconfig,
                                   ALM_CLASS_INFO,
                                   STR_CLEAR_ALARM);
            }
        }
    
    memcpy(&m_lastAifInfo, cpInfo, sizeof(m_lastAifInfo));
     return true;
}
#endif
bool CTaskSystem :: SYS_L2L3McpStateNotify(CL3L2McpStateNoitfy& NoitfyMsg)  
{
    const T_MCPStateInfo* cpInfo = NoitfyMsg.GetMCPStateInfo(); 

    const T_MCPStateInfo *pLastMCPStateInfo = &m_lastMCPStateInfo;
	#define M_MCP_ERROR     (50)    /*一秒钟内错误数超过M_MCP_ERROR则认为该告警*/
	int fepindex;
#ifndef WBBU_CODE
    UINT8  AlmFlg   = ALM_FLAG_CLEAR;
#define M_MCP_ERROR     (50)    /*一秒钟内错误数超过M_MCP_ERROR则认为该告警*/

    //开始构造告警信息
    for(UINT8 ucMcpIdx = 0; ucMcpIdx < MCP_NUM; ++ucMcpIdx)
        {
        //MCP alarm: L2 to MCP downlink data not empty.
        if(cpInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_DownlinkData_Not_Empty > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_DownlinkData_Not_Empty)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_DownlinkData_Not_Empty > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: L2 to MCP UplinkProf not_empty.
        if(cpInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_UplinkProf_Not_Empty > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_UplinkProf_Not_Empty)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_TO_MCP_UPLINKPROF_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_L2_TO_MCP_UPLINKPROF_NOT_EMPTY, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_TO_MCP_UPLINKPROF_NOT_EMPTY, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_UplinkProf_Not_Empty > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_TO_MCP_UPLINKPROF_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: L2 to MCP config_not empty
        if(cpInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_Config_Not_Empty > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_Config_Not_Empty)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_TO_MCP_CONFIG_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_L2_TO_MCP_CONFIG_NOT_EMPTY, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_TO_MCP_CONFIG_NOT_EMPTY, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_To_MCP_Config_Not_Empty > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_TO_MCP_CONFIG_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: L2 from MCP uplink data_not full.
        if(cpInfo->L2McpStateInfo[ucMcpIdx].L2_From_MCP_UplinkData_Not_Full > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_From_MCP_UplinkData_Not_Full)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_L2_FROM_MCP_UPLINKDATA_NOT_FULL, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_FROM_MCP_UPLINKDATA_NOT_FULL, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_From_MCP_UplinkData_Not_Full > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: L2 from MCP uplink data checksum
        if (cpInfo->L2McpStateInfo[ucMcpIdx].L2_From_MCP_UplinkData_CHKSUM > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_From_MCP_UplinkData_CHKSUM)
                {
                AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_MCP,
                                   ucMcpIdx,
                                   ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_CHKSUM,
                                   ALM_CLASS_MAJOR,
                                   STR_L2_FROM_MCP_UPLINKDATA_CHKSUM, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_L2_FROM_MCP_UPLINKDATA_CHKSUM, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->L2McpStateInfo[ucMcpIdx].L2_From_MCP_UplinkData_CHKSUM > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: MCP to L2 uplink not empty.
        if(cpInfo->McpL2StateInfo[ucMcpIdx].MCP_To_L2_Uplink_Not_Empty > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_To_L2_Uplink_Not_Empty)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_TO_L2_UPLINK_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_MCP_TO_L2_UPLINK_NOT_EMPTY, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_TO_L2_UPLINK_NOT_EMPTY, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_To_L2_Uplink_Not_Empty > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_TO_L2_UPLINK_NOT_EMPTY,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: MCP from L2 downlink not full.
        if(cpInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Downlink_Not_Full > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Downlink_Not_Full)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_MCP_FROM_L2_DOWNLINK_NOT_FULL, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_FROM_L2_DOWNLINK_NOT_FULL, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Downlink_Not_Full > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: MCP from L2 upprof not full.
        if(cpInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_UpProf_Not_Full > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_UpProf_Not_Full)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_UPPROF_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_MCP_FROM_L2_UPPROF_NOT_FULL, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_FROM_L2_UPPROF_NOT_FULL, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_UpProf_Not_Full > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_UPPROF_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: MCP from L2 config not full.
        if(cpInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Config_Not_Full > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Config_Not_Full)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_CONFIG_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_MCP_FROM_L2_CONFIG_NOT_FULL, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_FROM_L2_CONFIG_NOT_FULL, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Config_Not_Full > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_CONFIG_NOT_FULL,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: MCP from L2 downlink CHKSUM.
        if(cpInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Downlink_CHKSUM > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Downlink_CHKSUM)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_MCP_FROM_L2_DOWNLINK_CHKSUM, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_FROM_L2_DOWNLINK_CHKSUM, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Downlink_CHKSUM > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: MCP from L2 upProf CHKSUM.
        if(cpInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_UpProf_CHKSUM > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_UpProf_CHKSUM)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_UPPROF_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_MCP_FROM_L2_UPPROF_CHKSUM, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_FROM_L2_UPPROF_CHKSUM, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_UpProf_CHKSUM > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_UPPROF_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: MCP from L2 config CHKSUM.
        if(cpInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Config_CHKSUM > M_MCP_ERROR)
            {
            if (M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Config_CHKSUM)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_CONFIG_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_MCP_FROM_L2_CONFIG_CHKSUM, ucMcpIdx);
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_FROM_L2_CONFIG_CHKSUM, ucMcpIdx);
                }
            }
        else
            {
            if (pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].MCP_From_L2_Config_CHKSUM > M_MCP_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FROM_L2_CONFIG_CHKSUM,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }

        SINT8 aBuff[ALM_INFO_LEN]       = {0};
        SINT8 strAlmInfo[ALM_INFO_LEN]  = {0};
        bool  bExistSetAlarm            = false;

        //查是否有新产生的告警.
        for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
            {
            if(cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_To_FEP_Not_Empty > M_MCP_ERROR)
                {
                bExistSetAlarm = true;
                sprintf(aBuff, STR_MCP_TO_FEP_NOT_EMPTY, ucMcpIdx, ucFepNum);
                if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                    strcat(strAlmInfo, aBuff);        
                else
                    break;
                }

            if(cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_Not_Full > M_MCP_ERROR)
                {
                bExistSetAlarm = true;
                sprintf(aBuff, STR_MCP_FROM_FEP_NOT_FULL, ucMcpIdx, ucFepNum);
                if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                    strcat(strAlmInfo, aBuff);        
                else
                    break;
                }

            if(cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_CHKSUM > M_MCP_ERROR)
                {
                bExistSetAlarm = true;
                sprintf(aBuff, STR_MCP_FROM_FEP_CHKSUM, ucMcpIdx, ucFepNum);
                if(strlen(strAlmInfo) + strlen(aBuff) < ALM_INFO_LEN)
                    strcat(strAlmInfo, aBuff);        
                else
                    break;
                }
            }
        if (false == bExistSetAlarm)
            {
            //没有错误,查前次的状态，看是否需要发恢复告警
            bool bMustSendClearAlarm = false;
            for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
                {
                if(pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_To_FEP_Not_Empty > M_MCP_ERROR)
                    {
                    bMustSendClearAlarm = true;
                    break;
                    }

                if(pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_Not_Full > M_MCP_ERROR)
                    {
                    bMustSendClearAlarm = true;
                    break;
                    }

                if(pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_CHKSUM > M_MCP_ERROR)
                    {
                    bMustSendClearAlarm = true;
                    break;
                    }
                }
            if (true == bMustSendClearAlarm)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FEP_ALARM,
                                       ALM_CLASS_MAJOR,
                                       STR_CLEAR_ALARM);
                }
            }
        else
            {
            //有错误发生,查是否和前次的错误一样，不一样则发告警
            bool bNoChange = true;
            for(UINT8 ucFepNum = 0; ucFepNum < FEP_NUM; ++ucFepNum)
                {
                if (((M_MCP_ERROR >= cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_To_FEP_Not_Empty)
                   &&(M_MCP_ERROR <  pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_To_FEP_Not_Empty))
                  ||((M_MCP_ERROR <  cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_To_FEP_Not_Empty)
                   &&(M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_To_FEP_Not_Empty)))
                    {
                    bNoChange = false;
                    break;
                    }

                if (((M_MCP_ERROR >= cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_Not_Full)
                   &&(M_MCP_ERROR <  pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_Not_Full))
                  ||((M_MCP_ERROR <  cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_Not_Full)
                   &&(M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_Not_Full)))

                    {
                    bNoChange = false;
                    break;
                    }

                if (((M_MCP_ERROR >= cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_CHKSUM)
                   &&(M_MCP_ERROR <  pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_CHKSUM))
                  ||((M_MCP_ERROR <  cpInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_CHKSUM)
                   &&(M_MCP_ERROR >= pLastMCPStateInfo->McpL2StateInfo[ucMcpIdx].McpFepStateInfo[ucFepNum].MCP_From_FEP_CHKSUM)))
                    {
                    bNoChange = false;
                    break;
                    }
                }
            if (false == bNoChange)
                {
                //send alarm.
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_MCP_FEP_ALARM,
                                       ALM_CLASS_MAJOR,
                                       strAlmInfo);
                OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, strAlmInfo);
                }
            }
        }

    memcpy(&m_lastMCPStateInfo, cpInfo, sizeof(T_MCPStateInfo));
#else

    //开始构造告警信息
    for(UINT8 ucMcpIdx = 0; ucMcpIdx < MCP_NUM; ++ucMcpIdx)
        {
        //MCP alarm: L2 to MCP downlink data not empty.
        if(( cpInfo->Core9MCPInfo[ucMcpIdx].Core9_From_MCP_UplinkData_CHKSUM>M_MCP_ERROR ))
            {
            if ((M_MCP_ERROR >= pLastMCPStateInfo->Core9MCPInfo[ucMcpIdx].Core9_From_MCP_UplinkData_CHKSUM))		
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_CORE9,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_CORE9_MCP_UPLINK_DATA_LOST,
                                       ALM_CLASS_INFO,
                                       STR_Core9_From_MCP_UplinkData_CHKSUM, ucMcpIdx);
                OAM_LOGSTR1(LOG_DEBUG3, L3FM_ERROR_PRINT_SW_INFO, STR_Core9_From_MCP_UplinkData_CHKSUM, ucMcpIdx);
                }
            }
        else
            {
            if ((pLastMCPStateInfo->Core9MCPInfo[ucMcpIdx].Core9_From_MCP_UplinkData_CHKSUM> M_MCP_ERROR))
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_CORE9,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_CORE9_MCP_UPLINK_DATA_LOST,
                                       ALM_CLASS_INFO,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: L2 to MCP UplinkProf not_empty.
        if((cpInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_Downlink_CHKSUM> M_MCP_ERROR))
            {
            if( (M_MCP_ERROR >= pLastMCPStateInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_Downlink_CHKSUM))
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_CORE9_DOWN_DATA_LOST,
                                       ALM_CLASS_INFO,
                                       STR_MCP_From_Core9_Downlink_CHKSUM, ucMcpIdx);
                OAM_LOGSTR1(LOG_DEBUG3, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_From_Core9_Downlink_CHKSUM, ucMcpIdx);
                }
            }
        else
            {
            if ((pLastMCPStateInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_Downlink_CHKSUM> M_MCP_ERROR))
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_CORE9_DOWN_DATA_LOST,
                                       ALM_CLASS_INFO,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: L2 to MCP config_not empty
        if((cpInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_UpProf_CHKSUM > M_MCP_ERROR))
            {
            if ((M_MCP_ERROR >= pLastMCPStateInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_UpProf_CHKSUM))
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_CORE9_UPLINK_Prof_LOST,
                                       ALM_CLASS_INFO,
                                       STR_MCP_From_Core9_UpProf_CHKSUM, ucMcpIdx);
                OAM_LOGSTR1(LOG_DEBUG3, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_From_Core9_UpProf_CHKSUM, ucMcpIdx);
                }
            }
        else
            {
            if ((pLastMCPStateInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_UpProf_CHKSUM> M_MCP_ERROR))
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_CORE9_UPLINK_Prof_LOST,
                                       ALM_CLASS_INFO,
                                       STR_CLEAR_ALARM);
                }
            }

        //MCP alarm: L2 from MCP uplink data_not full.MCP_From_Core9_Config_CHKSUM
        if((cpInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_Config_CHKSUM> M_MCP_ERROR))
            {
            if( (M_MCP_ERROR >= pLastMCPStateInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_Config_CHKSUM))
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_CORE9_CFG_MSG_LOST,
                                       ALM_CLASS_INFO,
                                       STR_MCP_From_Core9_Config_CHKSUM, ucMcpIdx);
                OAM_LOGSTR1(LOG_DEBUG3, L3FM_ERROR_PRINT_SW_INFO, STR_MCP_From_Core9_Config_CHKSUM, ucMcpIdx);
                }
            }
        else
            {
            if ((pLastMCPStateInfo->MCPCore9Info[ucMcpIdx].MCP_From_Core9_Config_CHKSUM > M_MCP_ERROR))
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_CORE9_CFG_MSG_LOST,
                                       ALM_CLASS_INFO,
                                       STR_CLEAR_ALARM);
                }
            }



         for( fepindex=0; fepindex<2;fepindex++)
         {
        //MCP alarm: L2 from MCP uplink data checksum
        if ((cpInfo->FEPFromMCPInfo[ucMcpIdx][fepindex].FEP_From_MCP_DownlinkData_Lost > M_MCP_ERROR))
            {
            if( (M_MCP_ERROR >= pLastMCPStateInfo->FEPFromMCPInfo[ucMcpIdx][fepindex].FEP_From_MCP_DownlinkData_Lost))
                {
                AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_FEP,
                                   fepindex,
                                   ALM_ID_FEP_MCP_DOWN_DATA_LOST,
                                   ALM_CLASS_INFO,
                                   STR_FEP_From_MCP_DownlinkData_Lost, fepindex,ucMcpIdx);
                OAM_LOGSTR2(LOG_DEBUG3, L3FM_ERROR_PRINT_SW_INFO, STR_FEP_From_MCP_DownlinkData_Lost ,fepindex,ucMcpIdx);
                }
            }
        else
            {
            if ((pLastMCPStateInfo->FEPFromMCPInfo[ucMcpIdx][fepindex].FEP_From_MCP_DownlinkData_Lost> M_MCP_ERROR))
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_FEP,
                                       fepindex,
                                       ALM_ID_FEP_MCP_DOWN_DATA_LOST,
                                       ALM_CLASS_INFO,
                                       STR_CLEAR_ALARM);
                }
            }


        if ((cpInfo->FEPFromMCPInfo[ucMcpIdx][fepindex].FEP_From_MCP_DownlinkData_CHKSUM> M_MCP_ERROR))
            {
            if( (M_MCP_ERROR >= pLastMCPStateInfo->FEPFromMCPInfo[ucMcpIdx][fepindex].FEP_From_MCP_DownlinkData_CHKSUM))
                {
                AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_FEP,
                                   fepindex,
                                   ALM_ID_FEP_MCP_DOWN_DATA_LOST,
                                   ALM_CLASS_INFO,
                                   STR_FEP_From_MCP_DownlinkData_CHKSUM, fepindex,ucMcpIdx);
                OAM_LOGSTR2(LOG_DEBUG3, L3FM_ERROR_PRINT_SW_INFO, STR_FEP_From_MCP_DownlinkData_CHKSUM ,fepindex,ucMcpIdx);
                }
            }
        else
            {
            if ((pLastMCPStateInfo->FEPFromMCPInfo[ucMcpIdx][fepindex].FEP_From_MCP_DownlinkData_CHKSUM> M_MCP_ERROR))
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_FEP,
                                       fepindex,
                                       ALM_ID_FEP_MCP_DOWN_DATA_LOST,
                                       ALM_CLASS_INFO,
                                       STR_CLEAR_ALARM);
                }
            }
        }

       for( fepindex=0; fepindex<2;fepindex++)
         {
       
        if ((cpInfo->MCPFromFEPInfo[ucMcpIdx][fepindex].MCP_From_FEP_CHKSUM> M_MCP_ERROR))
            {
            if( (M_MCP_ERROR >= pLastMCPStateInfo->MCPFromFEPInfo[ucMcpIdx][fepindex].MCP_From_FEP_CHKSUM))
                {
                AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_MCP,
                                   ucMcpIdx,
                                   ALM_ID_MCP_FEP_UPLINK_DATA_ERROR,
                                   ALM_CLASS_INFO,
                                   STR_MCP_From_FEP_CHKSUM, ucMcpIdx,fepindex);
                OAM_LOGSTR2(LOG_DEBUG3, L3FM_ERROR_PRINT_SW_INFO, STR_FEP_From_MCP_DownlinkData_Lost, ucMcpIdx,fepindex);
                }
            }
        else
            {
            if ((pLastMCPStateInfo->MCPFromFEPInfo[ucMcpIdx][fepindex].MCP_From_FEP_CHKSUM> M_MCP_ERROR))
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_MCP,
                                       ucMcpIdx,
                                       ALM_ID_MCP_FEP_UPLINK_DATA_ERROR,
                                       ALM_CLASS_INFO,
                                       STR_CLEAR_ALARM);
                }
            }
        }

	}
        

    memcpy(&m_lastMCPStateInfo, cpInfo, sizeof(T_MCPStateInfo));

#endif
    return true;
}


bool CTaskSystem :: SYS_L2L3RFStateNotify(CL3L2RFStateNoitfy& NoitfyMsg)
{
    const T_RFStateInfo *cpInfo = NoitfyMsg.GetRFStateInfo(); 
    gl3oamBtsSysStatusData.SyncCardTemperature = cpInfo->temperature;

#define M_RF_ERROR      (10)    /*一秒钟内错误数超过M_RF_ERROR 则认为该告警*/
#define M_PLL_ERROR     (10)    /*一秒钟内错误数超过M_PLL_ERROR则认为该告警*/

    for(UINT8 ucAntennaIdx = 0; ucAntennaIdx < ANTENNA_NUM; ++ucAntennaIdx)
        {
//////////alarm: board voltage minor
        if(cpInfo->RFChStateInfo[ucAntennaIdx].BoardVoltminorAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].BoardVoltminorAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_VOLTAGE_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_BOARD_VOLTAGE_OUTOFRANGE_MINOR, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].BoardVoltminorAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_VOLTAGE_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: board voltage serious
        if(cpInfo->RFChStateInfo[ucAntennaIdx].BoardVoltseriousAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].BoardVoltseriousAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_VOLTAGE_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_BOARD_VOLTAGE_OUTOFRANGE_SERIOUS, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].BoardVoltseriousAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_VOLTAGE_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: board current minor
        if(cpInfo->RFChStateInfo[ucAntennaIdx].BoardCurrminorAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].BoardCurrminorAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_CURRENT_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_BOARD_CURRENT_OUTOFRANGE_MINOR, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].BoardCurrminorAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_CURRENT_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: board current serious
        if(cpInfo->RFChStateInfo[ucAntennaIdx].BoardCurrseriousAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].BoardCurrseriousAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_CURRENT_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_BOARD_CURRENT_OUTOFRANGE_SERIOUS, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].BoardCurrseriousAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_CURRENT_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: TTS voltage minor
        if(cpInfo->RFChStateInfo[ucAntennaIdx].TTAVoltminorAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TTAVoltminorAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TTA_VOLTAGE_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_TTA_VOLTAGE_OUTOFRANGE_MINOR, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TTAVoltminorAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TTA_VOLTAGE_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: TTA voltage serious
        if(cpInfo->RFChStateInfo[ucAntennaIdx].TTAVoltserirousAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TTAVoltserirousAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TTA_VOLTAGE_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_TTA_VOLTAGE_OUTOFRANGE_SERIOUS, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TTAVoltserirousAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TTA_VOLTAGE_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: TTA current minor
        if(cpInfo->RFChStateInfo[ucAntennaIdx].TTACurrminorAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TTACurrminorAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TTA_CURRENT_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_TTA_CURRENT_OUTOFRANGE_MINOR, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TTACurrminorAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TTA_CURRENT_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: TTA current serious
        if(cpInfo->RFChStateInfo[ucAntennaIdx].TTACurrSeriousAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TTACurrSeriousAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TTA_CURRENT_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_TTA_CURRENT_OUTOFRANGE_SERIOUS, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TTACurrSeriousAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TTA_CURRENT_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: TX power minor
        if(cpInfo->RFChStateInfo[ucAntennaIdx].TxPowerminorAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TxPowerminorAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TX_POWER_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_TX_POWER_OUTOFRANGE_MINOR, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TxPowerminorAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TX_POWER_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: TX power serious
        if(cpInfo->RFChStateInfo[ucAntennaIdx].TxPowerSeriousAlarm > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TxPowerSeriousAlarm)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TX_POWER_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_TX_POWER_OUTOFRANGE_SERIOUS, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].TxPowerSeriousAlarm > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_TX_POWER_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: RF disabled.
        if(cpInfo->RFChStateInfo[ucAntennaIdx].RFDisable > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].RFDisable)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_RF_DISABLED,
                                       ALM_CLASS_CRITICAL,
                                       STR_RF_DISABLED, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].RFDisable > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_RF_DISABLED,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: RF SSP checksum error.
        if(cpInfo->RFChStateInfo[ucAntennaIdx].SSPChkErr > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].SSPChkErr)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_SSP_CHKSUM_ERROR,
                                       ALM_CLASS_CRITICAL,
                                       STR_BOARD_SSP_CHKSUM_ERROR, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].SSPChkErr > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_SSP_CHKSUM_ERROR,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: RF checksum error
        if(cpInfo->RFChStateInfo[ucAntennaIdx].RFChkErr > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].RFChkErr)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_RF_CHKSUM_ERROR,
                                       ALM_CLASS_CRITICAL,
                                       STR_BOARD_RF_CHKSUM_ERROR, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].RFChkErr > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_RF_CHKSUM_ERROR,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

//////////alarm: board RF no response
        if(cpInfo->RFChStateInfo[ucAntennaIdx].RFNoRsp > M_RF_ERROR)
            {
            if (M_RF_ERROR >= m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].RFNoRsp)
                {
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_RF_NORESPONSE,
                                       ALM_CLASS_CRITICAL,
                                       STR_BOARD_RF_NORESPONSE, ucAntennaIdx + 1);
                }
            }
        else
            {
            if (m_lastRFStateInfo.RFChStateInfo[ucAntennaIdx].RFNoRsp > M_RF_ERROR)
                {
                //clear alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       ucAntennaIdx,
                                       ALM_ID_RF_BOARD_RF_NORESPONSE,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                }
            }

        }


///////////////////////////////////////////////////////////
//TCXO calibration告警
#define M_TXCO_NO_ERR   (0)
#define M_TXCO_ERR      (1)
    static UINT8 tcxo_err_state = M_TXCO_NO_ERR;  //是否因为TCXO，重起过FEP.
    static bool  bAlarmed = false;  //重起FEP后，是否已经发过告警?
    static UINT8 counter = 0;
    UINT32 offset = cpInfo->TCXOFrenOffset;
    if (M_TXCO_ERR != tcxo_err_state)
        {
        //no TCXO error occurs before ARM reseting.
        if (offset > 0)     //offset为偏移>2.5的次数
            {
            if (++counter > 5)//连续5秒偏移则重启
                {
                counter = 0;
                tcxo_err_state = M_TXCO_ERR;

                OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] TCXO err, reset FEP1");
                SendCpuResetMsg(BTS_CPU_TYPE_FEP, BTS_CPU_INDEX_FEP1);
                //置状态0
                SYS_SetCpuWorkingStatus(BTS_CPU_TYPE_FEP, BTS_CPU_INDEX_FEP1, 0); 
                }
            }
        else
            {
            counter = 0;
            }
        }
    else
        {
        //TCXO error occurs before ARM reseting.
        if ((offset > 0)     //offset为偏移>2.5的次数
            && (false == bAlarmed)
            && (1 == m_BtsCpuWorkStatus.OAM_FEP1)/*FEP1启动成功后*/)
            {
            //send TCXO Calibration alarm.
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_TCXO_FREQOFF,
                                   ALM_CLASS_CRITICAL,
                                   STR_TCXOFRENOFFSET);

#if 0
            //start 1-min timer tTXCOerr..// 可能会重复启动定时器
            pTXCOCalibrationErrTimer->Start();
#endif
            bAlarmed = true;
            }
        }

    if ((offset == 0)        //offset为偏移>2.5的次数
        && (true == bAlarmed))
        {
#if 0
        //stop timer tTXCOerr.
        pTXCOCalibrationErrTimer->Stop();
#endif
        //clear alarm.
        AlarmReport(ALM_FLAG_CLEAR,
                               ALM_ENT_PLL,
                               ALM_ENT_INDEX0,
                               ALM_ID_PLL_TCXO_FREQOFF,
                               ALM_CLASS_CRITICAL,
                               STR_CLEAR_ALARM);

        tcxo_err_state = M_TXCO_NO_ERR;
        bAlarmed = false;
        }


///////////////////////////////////////////////////////////
////PLLLose Lock告警
    UINT32 lock_percent          = cpInfo->PLLLoseLock;
    static UINT8 ucPLLLoseLockSeriously = 0;
    if (1 != ucPLLLoseLockSeriously)
        {
        //重起BTS前没有该严重告警, 
        if ((0 == lock_percent) && (m_lastRFStateInfo.PLLLoseLock > 0))
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_PLLLOSELOCK_MINOR,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);

            }
        if (((lock_percent > 0) && (lock_percent < 10))
            && (0 == m_lastRFStateInfo.PLLLoseLock))
            {
            //发送minor告警
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_PLLLOSELOCK_MINOR,
                                   ALM_CLASS_CRITICAL,
                                   STR_PLLLOSELOCK_MINOR);
            }
        if ((lock_percent >= 10)
            /*&& (10 > m_lastRFStateInfo.PLLLoseLock)*/)
            {
            ucPLLLoseLockSeriously = 1;
            //reboot FEP,AUX;
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] PLL lose lock, reset AUX, FEP0, and FEP1");
            SendCpuResetMsg(BTS_CPU_TYPE_AUX, 0);
            SendCpuResetMsg(BTS_CPU_TYPE_FEP, BTS_CPU_INDEX_FEP0);
            SendCpuResetMsg(BTS_CPU_TYPE_FEP, BTS_CPU_INDEX_FEP1);
            }
        }
    else
        {
        //重起前有严重告警
        if ((1 == m_BtsCpuWorkStatus.OAM_AUX)
            && (1 == m_BtsCpuWorkStatus.OAM_FEP0)
            && (1 == m_BtsCpuWorkStatus.OAM_FEP1))
            {
            //After reboot AUX,FEP
            if (0 == lock_percent)
                {
                //clear minor alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_PLL,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_PLL_PLLLOSELOCK_MINOR,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);

                //clear serious alarm.
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_PLL,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_PLL_PLLLOSELOCK_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                ucPLLLoseLockSeriously = 0;
                }

            if ( (lock_percent > 0)&&(lock_percent < 10) )
                {
                //clear serious alarm
                AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_PLL,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_PLL_PLLLOSELOCK_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);

                ucPLLLoseLockSeriously = 0;
                }

            if (lock_percent >= 10)
                {
                //发送serious告警
                AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_PLL,
                                       ALM_ENT_INDEX0,
                                       ALM_ID_PLL_PLLLOSELOCK_SERIOUS,
                                       ALM_CLASS_CRITICAL,
                                       STR_PLLLOSELOCK_SERIOUS);
                //Alarm异常处理会重起BTS.
                }
            }
        else
            {
            //AUX,FEP成功重起前,不进行告警分析
            }
        }


//////////alarm: FS Bad Factory Data告警
    if(cpInfo->FlashErr > 0)
        {
        if (0 == m_lastRFStateInfo.FlashErr)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_FACTORY_DATA,
                                   ALM_CLASS_CRITICAL,
                                   STR_FSC_FACTORY_DATA);
            }
        }
    else
        {
        if (m_lastRFStateInfo.FlashErr > 0)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_FACTORY_DATA,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);
            }
        }


///////////////////////////////////////////////////////////
//////////alarm: FSC SSP Checksum Error告警
    if(cpInfo->SSPChkErr > M_PLL_ERROR)
        {
        if (M_PLL_ERROR >= m_lastRFStateInfo.SSPChkErr)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_SSP_CHECKSUM_ERROR,
                                   ALM_CLASS_CRITICAL,
                                   STR_FSC_SSP_CHECKSUM_ERROR);
            }
        }
    else
        {
        if (m_lastRFStateInfo.SSPChkErr > M_PLL_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_SSP_CHECKSUM_ERROR,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);
            }
        }


///////////////////////////////////////////////////////////
//////////alarm: AUX2SYNC CHECKSUM Error告警
    if(cpInfo->AUX2SYNchecksumErr > M_PLL_ERROR)
        {
        if (M_PLL_ERROR >= m_lastRFStateInfo.AUX2SYNchecksumErr)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_AUX2SYNC_CHECKSUM_ERROR,
                                   ALM_CLASS_CRITICAL,
                                   STR_FSC_AUX2SYNC_CHECKSUM_ERROR);
            }
        }
    else
        {
        if (m_lastRFStateInfo.AUX2SYNchecksumErr > M_PLL_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_AUX2SYNC_CHECKSUM_ERROR,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);
            }
        }


///////////////////////////////////////////////////////////
//////////alarm: SYNC no response告警
    if(cpInfo->SynNoRsp > M_PLL_ERROR)
        {
        if (M_PLL_ERROR >= m_lastRFStateInfo.SynNoRsp)
            {
            AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_SYNC_NORESP_ERROR,
                                   ALM_CLASS_CRITICAL,
                                   STR_FSC_SYNC_NORESP_ERROR);
            }
        }
    else
        {
        if (m_lastRFStateInfo.SynNoRsp > M_PLL_ERROR)
            {
            //clear alarm.
            AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_PLL,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_SYNC_NORESP_ERROR,
                                   ALM_CLASS_CRITICAL,
                                   STR_CLEAR_ALARM);
            }
        }

    memcpy(&m_lastRFStateInfo, cpInfo, sizeof(m_lastRFStateInfo));
    return true;
}

void CTaskSystem :: SYS_SetCpuWorkingStatus(UINT8 CpuType, UINT8 CpuIndex, UINT8 state)
{
    switch (CpuType)
    {
        case BTS_CPU_TYPE_L2PPC:  
            m_BtsCpuWorkStatus.OAM_L2PPC = state;
            if(state == 1)
                {
                OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_L2PPC_BOOT_SUCCESS ,"[tSys] BTS L2 boot up SUCCESS");
                //启动L3L2链路PROBE检测定时器
                SYS_StartL3L2MoniterTimer();
                }
            break;

        case BTS_CPU_TYPE_AUX:
            m_BtsCpuWorkStatus.OAM_AUX= state;
            if(state == 1)OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_AUX_BOOT_SUCCESS ,"[tSys] BTS AUX boot up SUCCESS");
            break;

        case BTS_CPU_TYPE_MCP:
            if(BTS_CPU_INDEX_MCP0 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_MCP0 = state;
            }
            else if(BTS_CPU_INDEX_MCP1 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_MCP1 = state;
            }
            else if(BTS_CPU_INDEX_MCP2 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_MCP2 = state;
            }
            else if(BTS_CPU_INDEX_MCP3 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_MCP3 = state;
            }
            else if(BTS_CPU_INDEX_MCP4 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_MCP4 = state;
            }
            else if(BTS_CPU_INDEX_MCP5 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_MCP5 = state;
            }
            else if(BTS_CPU_INDEX_MCP6 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_MCP6 = state;
            }
            else if(BTS_CPU_INDEX_MCP7 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_MCP7 = state;
            }
            if(state == 1)OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_MCP_BOOT_SUCCESS, "[tSys] BTS MCP[%d] boot up SUCCESS", CpuIndex);
                break;

        case BTS_CPU_TYPE_FEP:  // 0x03
            if(BTS_CPU_INDEX_FEP0 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_FEP0 = state;
            }
            else if(BTS_CPU_INDEX_FEP1 == CpuIndex)
            {
                m_BtsCpuWorkStatus.OAM_FEP1 = state;
            }
            if(state == 1)OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_FEP_BOOT_SUCCESS ,"[tSys] BTS FEP[%d] boot up SUCCESS", CpuIndex);
                break;
        default:
            break;
    }
    if (0 == state)
        {
        //send alarm to alarm management task
        AlarmReport(ALM_FLAG_SET,
                           CpuType,
                           CpuIndex,
                           ALM_ID_L2PPC_RESET,
                           ALM_CLASS_MAJOR,
                           STR_L2PPCRESET, strCPUType[CpuType], CpuIndex);
        OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, STR_L2PPCRESET, (int)strCPUType[CpuType], CpuIndex);
        }
    else
        {
        //send alarm to alarm management task
        AlarmReport(ALM_FLAG_CLEAR,
                           CpuType,
                           CpuIndex,
                           ALM_ID_L2PPC_RESET,
                           ALM_CLASS_MAJOR,
                           STR_CLEAR_ALARM);
        }
}


bool CTaskSystem :: SYS_CpuWorkingNotify(CL3OamCpuWorkingNotify &NotifyNsg)
{
    UINT8 CpuType  = NotifyNsg.GetCpuType();
    UINT8 CpuIndex = NotifyNsg.GetCpuIndex();

    SYS_SetCpuWorkingStatus(CpuType, CpuIndex, 1); //设置开工状态

    if(1 == m_SysStatus.IsWork)
    {
        UINT16 MsgId;
        if(BTS_CPU_TYPE_L2PPC == CpuType)
        {
            MsgId = M_OAMSYS_CFG_INIT_L2DATA_NOTIFY; 
        }
        else if(BTS_CPU_TYPE_AUX == CpuType)
        {
            MsgId = M_OAMSYS_CFG_INIT_AUXDATA_NOTIFY; 
        }
        else if(BTS_CPU_TYPE_FEP == CpuType)
        {
            MsgId = M_OAMSYS_CFG_INIT_FEPDATA_NOTIFY; 
        }
        else
        {
            return true;
        }

        CL3OamCommonReq Notify;
        if (false == Notify.CreateMessage(*this))
            return false;
        Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
        Notify.SetDstTid(M_TID_CM);
        Notify.SetMessageId(MsgId);
        if(true != Notify.Post())
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_REV_MSG, "[tSys] post CPU working Notify to tCM fail.");
            Notify.DeleteMessage();
            return false;
        }
    }

    return true;
}

bool CTaskSystem ::  SendCpuResetMsg(UINT8 CpuType, UINT8 CpuIndex)
{
    OAM_LOGSTR2(LOG_DEBUG3, L3SYS_ERROR_RESET_CPU, 
        "[tSys] Reset CPU:%s, Index = %d", (int)strCPUType[CpuType], CpuIndex);

    CL3OamCpuWorkingNotify  Notify;
    if (false == Notify.CreateMessage(*this))
        return false;
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetDstTid(M_TID_BM);
    Notify.SetSrcTid(M_TID_SYS);
    Notify.SetMessageId(M_OAM_CPU_RESET_NOTIFY);    //更改消息ID
    Notify.SetCpuType(CpuType);
    Notify.SetCpuIndex(CpuIndex);
    if(true != Notify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post tBM CPU reset Notify fail.");
        Notify.DeleteMessage();
        return false;
    }

    return true;
}

//目前包括两种类型，code 为0表示cpu已经复位，为1表示cpu启动超时//
//发送M_OAM_CPU_RESET_NOTIFY消息到TBOOT,
//指示复位该cpu，然后向告警模块发送告警信息
void CTaskSystem :: SYS_L3CpuAlarmNofity(CL3CpuAlarmNofity &NotifyMsg)
{
#define ALARM_RESET     0x0
#define ALARM_EXPIRES   0x1
    const T_CpuAlarmNofity *CpuAlarmInfo = NotifyMsg.GetCpuAlarmNofity();
    UINT8 CpuType   = CpuAlarmInfo->CpuType;
    UINT8 CpuIndex  = CpuAlarmInfo->CpuIndex;
    UINT8 type      = CpuAlarmInfo->AlmCode;
    SYS_SetCpuWorkingStatus(CpuType, CpuIndex, 0); //设置开工状态
    if(ALARM_EXPIRES == type)   //启动超时
    {
        OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_BTS_BOOT_FAIL ,"[tSys] BTS CPU type[%d] index[%d] boot up FAIL", CpuType, CpuIndex);
    }

    return;
}

bool CTaskSystem :: SYS_SendBtsWorkingNotify()
{
    CL3OamCommonRsp Notify;
    if (false == Notify.CreateMessage(*this))
        return false;
    Notify.SetDstTid(M_TID_EMSAGENTTX);
    Notify.SetSrcTid(M_TID_SYS);
    Notify.SetMessageId(M_BTS_EMS_BOOTUP_NOTIFY);
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    if(true != Notify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] send BTS working response to EMS FAIL...");
        Notify.DeleteMessage();
        return false;
    }
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "=====================");
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "BTS IS WORKING STATUS");
    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "=====================");

    m_SysStatus.SendWorkToEMS = 1;
    BtsSwUpdateCheck();
#ifndef WBBU_CODE
    callMulticast();//wangwenhua add 20110510
 #endif
#ifdef M_TGT_WANIF
	if(Wanif_Switch==0x5a5a)
	{
	    SetLocalEms(0);
	}
#endif

    //基站向EMS请求系统时间
    CL3OamCommonReq Notify1;
    if (false == Notify1.CreateMessage(*this))
        return false;
    Notify1.SetDstTid(M_TID_EMSAGENTTX);
    Notify1.SetSrcTid(M_TID_SYS);
    Notify1.SetMessageId(M_BTS_EMS_SYSTEM_TIME_REQ);
    Notify1.SetTransactionId(OAM_DEFAUIT_TRANSID);
    if(true != Notify1.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] send M_BTS_EMS_SYSTEM_TIME_REQ to EMS FAIL...");
        Notify1.DeleteMessage();
        return false;
    }
    pGetBtsTimerFromEms = SYS_Createtimer(M_OAM_BTS_GET_BTS_TIME_FROM_EMS_TIMER, M_TIMER_PERIOD_YES, 60*1000);//one min
    if(NULL != pGetBtsTimerFromEms)
    {
        pGetBtsTimerFromEms->Start();
    }


    return true;
}
#ifdef WBBU_CODE
unsigned int  g_10ms_warn= 0;
unsigned int g_10ms_warn_count = 0;
unsigned int g_10ms_warn_count_recov = 0;

  void CTaskSystem::FPGAConfigRFMask(unsigned char value)

{
     unsigned char* pdata;
	CComMessage* pComMsg = new (this, 2) CComMessage;
	if (pComMsg==NULL)
	{
	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in gpsConfigRFMask.");
	return ;
	}
	pComMsg->SetDstTid(M_TID_FM);
	pComMsg->SetSrcTid(M_TID_SYS);    
	pComMsg->SetMessageId(M_FPGA_ALM_CHG_RF_MASK_REQ);
	pdata = (unsigned char*)pComMsg->GetDataPtr();
	pdata[0] = 0;//表示tsys任务，1-表示来自RRU的温度电流等告警
	pdata[1] = value;//表示全部关闭,0xff表示全部打开
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
}
 #endif
 #ifdef WBBU_CODE
extern  unsigned char   Calibration_Antenna;
 #endif
    #if 0
   告警类型：
Warning_Type(0)：gps_warning；
Warning_Type(1)：tdd_fep_rx_warning；
Warning_Type(2)：frame_sync_warning；//需要告警
Warning_Type(3)：sfp_los_warning；
Warning_Type(4)：fan0_monitor_warning;(J29)//需要告警
Warning_Type(5)：fan1_monitor_warning;(J28)
Warning_Type(6)：fan2_monitor_warning;(J30)
Warning_Type(7)：fan3_monitor_warning;(J26)
Warning_Type(8)：fan4_monitor_warning;(J27)
Warning_Type(9)：gps_flag；维持状态；
Warning_Type(10)：gps_loss；维持超过30分钟?//需要告警
各比特独立（可能同时发生），为'1'时正常，为'0'时告警；
2、为了可靠的，建议cpu增加控制机制：每5s检测一次aif状态寄存器的值（aif0对应0xd1000016,aif1对应0xd100
2、为了可靠的，建议cpu增加控制机制：每5s检测一次aif状态寄存器的值（aif0对应0xd1000016,aif1对应0xd100001a，正常时为8200～8203)，出错时复位aif serdes；
  #endif
  #ifdef WBBU_CODE
 unsigned char g_bbu_afc4001_status = 0;

  #endif
void CTaskSystem :: SYS_SystemClock()
{
    ++gVx_System_Clock;
    	static  unsigned char  m_flag0 =0;
    	static  unsigned char  m_flag1 =0;
	static  unsigned char  m_flag2 =0;
	static  unsigned char  m_flag3 =0;
	static  unsigned char  m_flag4 =0;
	static  unsigned char  m_flag5 =0;
	static  unsigned char  m_flag6 =0;
	static  unsigned char  m_flag8 =0;
	unsigned char  m_flag7 = 0;
#ifdef WBBU_CODE
//	static  unsigned int count_begin = 0;
	static  unsigned short m_last_sync = 0;
   // #ifdef FPGA_ALARM
  // L3_APP_Run();
       unsigned short aif0,aif1;
    if(m_SysStatus.IsWork== 1)
   	{
    unsigned short  Alarm = Read_Fpga_Alarm()&0xfff;
    unsigned char firstbit,secbit,thirdbit,fourbit,fan_stat,elevenbit,twelvebit;
    unsigned char firstbit1,secbit1,thirdbit1,fourbit1,fan_stat1,elevenbit1,twelvebit1;

    unsigned char fan[5];
	int j;
   for(j  = 0; j < 5; j++)
   	{
   	   fan[j] =0;
   	}

  
  if((FGPA_Alarm_Type==Alarm)&&(m_last_sync==NvRamDataAddr->L1GenCfgEle.SyncSrc))/****如果相同的话，则不产生告警****/
  {
  }
  else
  {
     //  OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] FPGA AlM:last:%x,now:%x.",FGPA_Alarm_Type,Alarm);
        firstbit = Alarm&0x1;
        secbit = (Alarm>>1)&0x1;
        thirdbit = (Alarm>>2)&0x1;
        fourbit = (Alarm>>3)&0x1;
        fan_stat = (Alarm>>4)&0x1f;//5 bit
        elevenbit = (Alarm>>10)&0x1;//10ms 
        twelvebit = (Alarm>>11)&0x1;//ADF4001_LOCK
        firstbit1 = FGPA_Alarm_Type&0x1;
        secbit1 = (FGPA_Alarm_Type>>1)&0x1;
        thirdbit1 = (FGPA_Alarm_Type>>2)&0x1;
        fourbit1 = (FGPA_Alarm_Type>>3)&0x1;
        fan_stat1 =( FGPA_Alarm_Type>> 4)&0x1f;
	  elevenbit1 = ( FGPA_Alarm_Type>> 10)&0x1;
	 
	  twelvebit1 = ( FGPA_Alarm_Type>> 11)&0x1;
         if((thirdbit!=thirdbit1)||(elevenbit!=elevenbit1)||(twelvebit!=twelvebit1)||(fourbit!=fourbit1))
        {
             OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] FPGA AlM:last:%x,now:%x.",FGPA_Alarm_Type,Alarm);
        }
         if(thirdbit!=thirdbit1) //fiber warn
         {
               if(thirdbit)
               {
                     AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_FPGA, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_frame_sync_warning,
                       ALM_CLASS_MAJOR,
                       STR_frame_sync_warning); 
               }
               else
               {
               	       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_FPGA, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_frame_sync_warning,
                       ALM_CLASS_MAJOR,
                       STR_frame_sync_warning); 
               	    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BBU  FPGA frame_sync_warning");
               }
         }
         if(fourbit!=fourbit1)
         {
            
               if(fourbit)
               {
                     AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_FPGA, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_sfp_los_warning,
                       ALM_CLASS_MAJOR,
                       STR_sfp_los_warning); 
               }
               else
               {
               	       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_FPGA, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_sfp_los_warning,
                       ALM_CLASS_MAJOR,
                       STR_sfp_los_warning); 
               	    OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BBU  FPGA sfp_los_warning");
               	  
               }
	
         }
            if(fan_stat!=fan_stat1)
         {
           
               if(fan_stat==0x1f)
               {
                    fan[0] = 1;
                     fan[1] = 1;
                      fan[2] = 1;
                       fan[3] = 1;
                        fan[4] = 1;
                     AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_ENV, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_ENV_FAN_STOP,
                       ALM_CLASS_MAJOR,
                       STR_sfp_fan_warning,fan[0],fan[1],fan[2],fan[3],fan[4]); 
               }
               else if(fan_stat==0)
               {
                         AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_ENV, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_ENV_FAN_STOP,
                       ALM_CLASS_MAJOR,
                       STR_sfp_fan_warning,fan[0],fan[1],fan[2],fan[3],fan[4]); 
               }
               else
               {
                    if(fan_stat&0x1)
                    {
                       fan[0] = 1;
                    }
			if(fan_stat&0x2)
			{
			     fan[1] = 1;
			}
			if(fan_stat&0x4)
			{
			     fan[2] = 1;
			}
			if(fan_stat&0x8)
			{
			     fan[3] = 1;
			}
			if(fan_stat&0x10)
			{
			     fan[4] = 1;
			}
			       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_ENV, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_ENV_FAN_STOP,
                       ALM_CLASS_MAJOR,
                       STR_sfp_fan_warning,fan[0],fan[1],fan[2],fan[3],fan[4]); 
               }     
         }
	  if((NvRamDataAddr->L1GenCfgEle.SyncSrc)==1)//只有在GPS作为同步源的情况下去检测该bit
	  {
	   if((elevenbit!=elevenbit1)||(m_last_sync!=NvRamDataAddr->L1GenCfgEle.SyncSrc))
         {
         	g_10ms_warn++;

               if((elevenbit)/*&&(bGpsStatus_RF==0)*/)
               {
               	    g_10ms_warn_count_recov++;

                     AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_ENV, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_10ms_warning,
                       ALM_CLASS_MAJOR,
                       STR_tdd_10ms_warning); 
			//	UINT16 mask = NvRamDataAddr->L1GenCfgEle.AntennaMask;
			//	mask = (mask)&(~Calibration_Antenna);
				//WrruRFC((unsigned char)mask,1);//打开RF
			//	FPGAConfigRFMask(mask/*0xff*/);

			//	sendAnntenaMsk(mask,1,7);
               }
               else
               {
               	        g_10ms_warn_count++;

               	       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_ENV, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_10ms_warning,
                       ALM_CLASS_MAJOR,
                       STR_tdd_10ms_warning); 
			//	FPGAConfigRFMask(0);
				//WrruRFC(0,1);//关闭RF
				//sendAnntenaMsk(0);
               }
         }
	 }
	  else // 不管什么情况都打开S?
	  	{
	  	           AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_ENV, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_10ms_warning,
                       ALM_CLASS_MAJOR,
                       STR_tdd_10ms_warning); 
                if((g_Close_RF_flag!=1)&&(g_close_RF_dueto_Slave==0))
                {
    				UINT16 mask = NvRamDataAddr->L1GenCfgEle.AntennaMask;
    				mask = (mask)&(~Calibration_Antenna);
    				FPGAConfigRFMask(mask/*0xff*/);

    				sendAnntenaMsk(mask,1,8);
                }
	  	}
	 if(twelvebit!=twelvebit1)
	  {
	     //    OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] FPGA AlM:last:%x,now:%x.",FGPA_Alarm_Type,Alarm);
	         if(twelvebit)
               {
                   
                     AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_ENV, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_AD4001_warning,
                       ALM_CLASS_MAJOR,
                       STR_AD4001_warning); 
                     g_bbu_afc4001_status = 0;
			
               }
               else
               {
                      
               	       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_ENV, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_AD4001_warning,
                       ALM_CLASS_MAJOR,
                       STR_AD4001_warning); 
               	   OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] BBU FPGA AD4001 LOCK FAI");
               	   g_bbu_afc4001_status = 1;

               }
	  }

  }
  m_last_sync =  NvRamDataAddr->L1GenCfgEle.SyncSrc;
  FGPA_Alarm_Type=Alarm ;
	  if(gVx_System_Clock%122==0)
	  {
	          ReadRRUFiberInfo(1);
	  //     ReadBBUFiberInfo(0);
	  }
	  if(gVx_System_Clock%61==0)
	  {
	  //	  ShowBBUFiberInfo(0);
	  }
	  if(gVx_System_Clock%5==0)
	  {
	  	       aif0 = *(unsigned short*)0xd1000016 ;
			aif1 = *(unsigned short*)0xd100001a ;
			if(((aif0<0x8200)||(aif0>0x8203))||((aif1<0x8200)||(aif1>0x8203)))
			{
			      ResetAifSerdies();
			     OAM_LOGSTR2(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] ResetAifSerdies due to aif0 or aif1 not normal [%x][%x]",aif0,aif1);
			}
	  }
   	}
#endif
/*  extern unsigned int g_eb_no_ft_freelist ;
extern unsigned int g_eb_no_cdr_freelist ;
extern unsigned int g_socket_no_ft_freelist ;
extern unsigned int g_socket_no_CB_freelist ;
extern unsigned int g_snoop_no_freelist ;
 extern unsigned int g_arp_no_freelist ;
 extern  unsigned int g_dm_no_freelist ;
const UINT16 ALM_ID_eb_no_ft_freelist   = 0x0110;
const UINT16 ALM_ID_eb_no_cdr_freelist = 0x0111;
const UINT16 ALM_ID_socket_no_ft_freelist  = 0x0112;
const UINT16 ALM_ID_socket_no_CB_freelist  = 0x0113;
const UINT16 ALM_ID_snoop_no_freelist   = 0x0114;
const UINT16 ALM_ID_arp_no_freelist  = 0x0115;
const UINT16 ALM_ID_dm_no_freelist  = 0x0116;

 */
 //wangwenhua add 2012-4-26 for audit the free list 
 if((gVx_System_Clock%180)==0)
 {
         if(g_eb_no_ft_freelist>10)
        {
           m_flag7 =1;
            if(m_flag0==0)
            	{
         	          AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_eb_no_ft_freelist,
                       ALM_CLASS_MINOR,
                       "\r\n L3 EB no free FT:%d",g_eb_no_ft_freelist); 
			  m_flag0 = 1;
			  
            	}
         }
	 else
	 {
	          if(m_flag0==1)
	          	{
	 	         AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_eb_no_ft_freelist,
                       ALM_CLASS_MINOR,
                       "\r\n L3 EB no free FT:%d",g_eb_no_ft_freelist); 
			   m_flag0 = 0;
	          	}
	 }
		 
	   
	if(g_eb_no_cdr_freelist>10)
	{
		m_flag7 =1;
	       if(m_flag1==0)
	       	{
	          AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_eb_no_cdr_freelist,
                       ALM_CLASS_MINOR,
                        "\r\n L3 EB CDR no free list:%d",g_eb_no_cdr_freelist); 
			  m_flag1 = 1;
			   
	       	}
	}
	 else
	 {
                      if(m_flag1==1)
                      	{
                           AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_eb_no_cdr_freelist,
                       ALM_CLASS_MINOR,
                        "\r\n L3 EB CDR no free list:%d",g_eb_no_cdr_freelist); 
				 m_flag1 = 0;
                      	}
		
	 }
		 
	if(g_socket_no_ft_freelist>10)
	{
		 m_flag7 =1;
	          if(m_flag2==0)
	          	{
	           AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_socket_no_ft_freelist,
                       ALM_CLASS_MINOR,
                       "\r\n L3 Socket  no free FT:%d",g_socket_no_ft_freelist); 
			   m_flag2 = 1;
			   
	          	}
	}
	 else
	 {
	       if(m_flag2==1)
	       	{

	           AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_socket_no_ft_freelist,
                       ALM_CLASS_MINOR,
                       "\r\n L3 Socket  no free FT:%d",g_socket_no_ft_freelist); 
			   m_flag2 = 0;
	       	}
	 }
		 
	if(g_socket_no_CB_freelist>10)
	{
	     m_flag7 =1;
	    if(m_flag3==0)
	    	{
	         AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_socket_no_CB_freelist,
                       ALM_CLASS_MINOR,
                        "\r\n L3 Socket  no free CB list :%d",g_socket_no_CB_freelist); 
			 m_flag3 = 1;
			 
	    	}
	}
	 else
	 {
	 	  if(m_flag3==1)   
	 	  	{
	         AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_socket_no_CB_freelist,
                       ALM_CLASS_MINOR,
                        "\r\n L3 Socket  no free CB list :%d",g_socket_no_CB_freelist); 
			 m_flag3 = 0;
	 	  	}
	 }
		 
	if(g_snoop_no_freelist>10)
	{
	      m_flag7 =1;
	    if(m_flag4==0)
	    	{
	          AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_snoop_no_freelist,
                       ALM_CLASS_MINOR,
                        "\r\n L3 SNOOP  no free list:%d",g_snoop_no_freelist); 
			  m_flag4 = 1;
			 
	    	}
	}
	 else
	 {
	       if(m_flag4==1)
	       	{
	 	     AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_snoop_no_freelist,
                       ALM_CLASS_MINOR,
                        "\r\n L3 SNOOP  no free list:%d",g_snoop_no_freelist); 
			 m_flag4 = 0;
	       	}
	 }
		 
	if(g_arp_no_freelist>10)
	{
		  m_flag7 =1;
	       if(m_flag5==0)
	       	{
	         AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_arp_no_freelist,
                       ALM_CLASS_MINOR,
                       "\r\n L3 ARP no free arp Table :%d",g_arp_no_freelist); 
			 m_flag5 = 1;
			
	       	}
	}
	 else
	 {
	           if(m_flag5==1)
	           	{
	 	          AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_arp_no_freelist,
                       ALM_CLASS_MINOR,
                       "\r\n L3 ARP no free arp Table :%d",g_arp_no_freelist); 
				  m_flag5 = 0;
	           	}
	 }
		 
	if(g_dm_no_freelist>10)
	{
	    m_flag7 =1;
	   if(m_flag6==0)
	   	{
	         AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_dm_no_freelist,
                       ALM_CLASS_MINOR,
                        "\r\n L3 DM no free dm list:%d",g_dm_no_freelist); 
			 m_flag6 = 1;
			 
	   	}
	}
	 else
	 {
	       if(m_flag6==1)
	       	{
	 	       AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_dm_no_freelist,
                       ALM_CLASS_MINOR,
                        "\r\n L3 DM no free dm list:%d",g_dm_no_freelist); 
			   m_flag6 = 0;
	       	}
	 }

       if(g_cdr_btree_err>10)
	{
	    m_flag7 =1;
	   if(m_flag8==0)
	   	{
	         AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_cdr_err,
                       ALM_CLASS_MINOR,
                        "\r\n L3 cdr err count:%d",g_cdr_btree_err); 
			 m_flag8 = 1;
			 
	   	}
	}
	 else
	 {
	       if(m_flag8==1)
	       {
	 	       AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_cdr_err,
                       ALM_CLASS_MINOR,
                        "\r\n L3 cdr err count:%d",g_cdr_btree_err); 
			   m_flag8 = 0;
	       }
	 }
	 
if( m_flag7 ==1)//只有错误才打印，正确时不打印
{
OAM_LOGSTR4(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] no Freelist Count:eb(%d),cdrlist(%d),socket ft(%d),socket cb(%d)\n",g_eb_no_ft_freelist,g_eb_no_cdr_freelist,g_socket_no_ft_freelist,g_socket_no_CB_freelist);
OAM_LOGSTR4(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] no Freelist Count:snoop(%d),arp(%d),dm(%d),cdr_err(%d)\n",g_snoop_no_freelist,g_arp_no_freelist,g_dm_no_freelist,g_cdr_btree_err);
}		 
 }
    return;
}
#ifdef WBBU_CODE
extern "C" void print_10ms_count(unsigned int flag)
{
    if(flag ==0)
    	{
    	     g_10ms_warn = 0;
		g_10ms_warn_count = 0;
		g_10ms_warn_count_recov = 0;
    	}
	else
	{
	    printf("10ms:%d,%d,%d\n",g_10ms_warn,g_10ms_warn_count,g_10ms_warn_count_recov);
	}
}
#endif
bool CTaskSystem :: SYS_SendBtsResetCntsNotify()
{
    UINT16 Cnt = NvRamDataAddr->BTSCommonDataEle.BtsRstCnt;
    OAM_LOGSTR1(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post BTS Reset count(%d) change Notify to L2.", Cnt);
    CL3L2BtsRstCntChangeNotify Notify;
    if (false == Notify.CreateMessage(*this))
        return false;
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetDstTid(M_TID_L2MAIN);
    Notify.SetSrcTid(M_TID_SYS);
    Notify.SetRstCnt(Cnt);
    if(true != Notify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post BTS Reset count change Notify to L2 fail.");
        Notify.DeleteMessage();
        return false;
    }

    return true;
}


/*
 *需要发送到备用ems的消息，都必须经过这个函数的封装
 *socket检查到这个消息后，从payload恢复原来的消息id;和消息长度
 */
void CTaskSystem::SYS_WrapToBakEMS(CMessage &msg)
{
    UINT16 usMsgId = msg.GetMessageId();
    //modify msg id;
    msg.SetMessageId(M_SYS_SOCKET_BAK_EMS);
    //save msg id;
    UINT8 *ptr = (UINT8*)(msg.GetDataPtr());
    ptr -= sizeof(T_WrapToEMS);
    //
    T_WrapToEMS *pWrap = (T_WrapToEMS*)ptr;
    pWrap->usMsgCode   = usMsgId;
    pWrap->ucToMainEMS = (gActiveEMSip == bspGetMainEmsIpAddr())?M_SENDTO_BAK_EMS:M_SENDTO_MAIN_EMS;
    //modify msg length.
    msg.SetDataLength(msg.GetDataLength() + sizeof(T_WrapToEMS));
    //modify pointer.
    msg.SetDataPtr(ptr);
}


void CTaskSystem::SYS_setActiveEMS(UINT32 ip, UINT32 port)
{
    taskLock();
    gActiveEMSip    = ip;
    gActiveEMSport  = (UINT16)port;
    taskUnlock();
}


void CTaskSystem::showSysStatus()
{
    printf("\r\nRegister state       :");
    printf("\r\nBTS boot up          : %s",(int)(m_SysStatus.BootLodeOk?"true":"false"));
    printf("\r\nBTS register to EMS  : %s",(int)(m_SysStatus.RegOk?"true":"false"));
    printf("\r\nEMS link is OK       : %s",(int)(m_SysStatus.IsEmsLinkOk?"true":"false"));
    printf("\r\nNever init from EMS  : %s",(int)(m_SysStatus.NeverInitFromEMS?"true":"false"));
    printf("\r\nSend working to EMS  : %s",(int)(m_SysStatus.SendWorkToEMS?"true":"false"));
    printf("\r\nPeriod registering   : %s",(int)(m_SysStatus.iSPeriodReg?"true":"false"));
    printf("\r\nCurrent EMS state    : 0x%X (0xBM 4:down,2:work,1:reg)",m_emsState);
    char IPstr[20];
    struct in_addr emsIp; 
    emsIp.s_addr = bspGetMainEmsIpAddr();
    inet_ntoa_b(emsIp, IPstr);
    printf("\r\nMain    EMS:%s", IPstr);
    emsIp.s_addr = bspGetBakEmsIpAddr();
    if(0 != emsIp.s_addr)
        {
        inet_ntoa_b(emsIp, IPstr);
        printf("\r\nBack up EMS:%s", IPstr);
        }
    else
        {
        printf("\r\nBack up EMS: not configured");
        }
    emsIp.s_addr = gActiveEMSip;
    inet_ntoa_b(emsIp, IPstr);
    printf("\r\nCurrent EMS:%s", IPstr);
}


void GetOAMPerfData(UINT8 *pData)
{
#pragma pack()
    struct T_L3OAMPerfData
    {
        UINT32  BtsResetCnt; // BTS 复位计数
        UINT32  BtsRegCnt;   // BTS 注册计数
    };
#pragma pack()

    ((T_L3OAMPerfData*)pData)->BtsResetCnt = NvRamDataAddr->BTSCommonDataEle.BtsRstCnt;
    ((T_L3OAMPerfData*)pData)->BtsRegCnt   = NvRamDataAddr->BTSCommonDataEle.BtsRegCnt;
}



/*************************************
 *全局函数:
 *系统重要配置有修改，需要CPE重新注册.
 *************************************/
bool notifyAllCPEtoRegister()
{
    return CTaskSystem::GetInstance()->SYS_BtsResetCntIncReq(OAM_DEFAUIT_TRANSID);
}

bool notifyOneCpeToRegister(UINT32 EID,bool blStopDataSrv)
{
	typedef struct _FroceCpeReg
	{
		UINT16 blStopDataSrv;
	}FroceCpeRegT;
	bool ret = false;
	//UINT8 *pData;
	CComMessage *pMsg = new (CTaskSystem::GetInstance(), sizeof(FroceCpeRegT)) CComMessage;
	if(pMsg!=NULL)
	{
		pMsg->SetMessageId(M_L3_BTS_CPE_FORCE_REGISTER);
		pMsg->SetSrcTid(M_TID_UM);
		pMsg->SetDstTid(M_TID_CPECM);
		pMsg->SetDataLength(sizeof(FroceCpeRegT));
		pMsg->SetEID(EID);
		
		FroceCpeRegT *pData = (FroceCpeRegT*)pMsg->GetDataPtr();
		pData->blStopDataSrv = blStopDataSrv;

		ret = CComEntity::PostEntityMessage(pMsg);
		if(!ret)
		{
			pMsg->Destroy();
			OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, 
				"force CPE-EID[0x%08X] register fail!!!", EID);
		}
		else
		{
			OAM_LOGSTR1(LOG_DEBUG1, L3SYS_ERROR_PRINT_SYS_INFO, 
				"force CPE-EID[0x%08X] register ", EID);
		}
	}
	else
	{
		OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "new commessage fail!!!");
	}	
	return ret;
}

#if 0
bool CTaskSystem :: SYS_SendAlmNotifyToEMS ( UINT32  SeqID,
                                         UINT8   Flag,
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
    Notify.SetDstTid(M_TID_EMSAGENTTX);
    Notify.SetSrcTid(M_TID_FM);
    Notify.SetSequenceNum(SeqID);
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
        OAM_LOGSTR1(LOG_DEBUG, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] POST MSG[0X%04X] FAIL", 
                    Notify.GetMessageId());
        Notify.DeleteMessage();
    }


    return true;
}
#endif


extern "C" int tickGet ();
void CTaskSystem::SYS_SAGLinkAlm(CMessage &rMsg)
{
    static UINT32 clear_time = 0;
    static bool   bSet     = false;
    UINT8  Flag = *(((UINT8*)rMsg.GetDataPtr()) + 2);
    if ((ALM_FLAG_CLEAR == Flag)&&(true == bSet))
        {
        clear_time = tickGet();
        bSet       = false;
        AlarmReport(Flag,
                           ALM_ENT_L3PPC,
                           ALM_ENT_INDEX0,
                           ALM_ID_BTS_SAG_LINK,
                           ALM_CLASS_CRITICAL,
                           STR_CLEAR_ALARM);
        return;
        }
    if ((ALM_FLAG_SET == Flag)&&(false == bSet)&&(tickGet() - clear_time > 300))
        {
        bSet     = true;
        AlarmReport(Flag,
                           ALM_ENT_L3PPC,
                           ALM_ENT_INDEX0,
                           ALM_ID_BTS_SAG_LINK,
                           ALM_CLASS_CRITICAL,
                           STR_BTS_SAG_LINK);
        return;
        }
}


bool CTaskSystem :: SYS_SendNotifyMsg( TID tid, UINT16 msgid)
{
    CL3OamCommonReq Notify;
    if (false == Notify.CreateMessage(*this))
        return false;
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetDstTid(tid);
    Notify.SetSrcTid(M_TID_SYS);
    Notify.SetMessageId(msgid);
    if(true != Notify.Post())
    {
        OAM_LOGSTR1(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post msg[0x%04x] fail", msgid);
        Notify.DeleteMessage();
    }

    return true;
}

#if 0
void l3oamsendbtsreg()
{
    CTaskSystem* ptask = CTaskSystem :: GetInstance();
    CBtsRegNotify RegNotify;
    if (false == RegNotify.CreateMessage(*ptask))
        return;
    RegNotify.SetDstTid(M_TID_EMSAGENTTX);
    RegNotify.SetSrcTid(M_TID_SYS);
    RegNotify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    UINT32 Version = bspGetBtsHWVersion();
    RegNotify.SetBtsHWVersion(Version);
    Version = bspGetActiveVersion();
    RegNotify.SetBtsSWVersion(Version);
    
    SINT8 IDInfo[ENCRYPED_BTSID_LEN];
    memset(IDInfo, 0, sizeof(IDInfo));
    RegNotify.SetBtsID(IDInfo, ENCRYPED_BTSID_LEN);;
    UINT16 Port = bspGetBtsUDPRcvPort();
    RegNotify.SetBtsRcvPort(Port);
    if(true != RegNotify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_PRINT_SYS_INFO, "[tSys] post BTS register message Fail");
        RegNotify.DeleteMessage();
        return;
    }

    printf("\r\n[tSys] Send BTS Register TO EMS");
}
#endif

void l3oamnvramaddr()
{
    printf("\r\nOAM NvRam Address:      %p, total len:%d", NvRamDataAddr, sizeof(T_NvRamData));
    printf("\r\n");
    printf("\r\nBTS Common Data:        %p, len:%d", &(NvRamDataAddr->BTSCommonDataEle), sizeof(T_BTSCommonDataEle));
    printf("\r\nBTS Voice cfg:          %p, len:%d", &(NvRamDataAddr->BtsGDataCfgEle), sizeof(T_BtsGDataCfgEle));
    printf("\r\nBTS Data services cfg:  %p, len:%d", &(NvRamDataAddr->DataServiceCfgEle), sizeof(T_DataServiceCfgEle));
    printf("\r\nUTSD cfg:               %p, len:%d", &(NvRamDataAddr->UTSDCfgEle), sizeof(T_UTSDCfgEle));
    printf("\r\nTOS  cfg:               %p, len:%d",   NvRamDataAddr->ToSCfgEle, sizeof(NvRamDataAddr->ToSCfgEle));
    printf("\r\nPerf Log  cfg:          %p, len:%d", &(NvRamDataAddr->PerfLogCfgEle), sizeof(T_PerfLogCfgEle));
    printf("\r\nTemperature alarm cfg:  %p, len:%d", &(NvRamDataAddr->TempAlarmCfgEle), sizeof(T_TempAlarmCfgEle));
    printf("\r\nGps Data cfg:           %p, len:%d", &(NvRamDataAddr->GpsDataCfgEle), sizeof(T_GpsDataCfgEle));
    printf("\r\nBTS Boot_from cfg:      %p, len:%d", &(NvRamDataAddr->BtsRstEle), sizeof(T_BtsRstEle));
    printf("\r\nAir link cfg:           %p, len:%d", &(NvRamDataAddr->AirLinkCfgEle), sizeof(T_AirLinkCfgEle));
    printf("\r\nRM policy cfg:          %p, len:%d", &(NvRamDataAddr->RMPoliceEle), sizeof(T_RMPoliceEle));
    printf("\r\nAir link Mis cfg:       %p, len:%d", &(NvRamDataAddr->AirLinkMisCfgEle), sizeof(T_AirLinkMisCfgEle));
    printf("\r\nBill Data cfg:          %p, len:%d", &(NvRamDataAddr->BillDataCfgEle), sizeof(T_BillDataCfgEle));
    printf("\r\nL1 general cfg:         %p, len:%d", &(NvRamDataAddr->L1GenCfgEle), sizeof(T_L1GenCfgEle));
    printf("\r\nRF cfg:                 %p, len:%d", &(NvRamDataAddr->RfCfgEle), sizeof(T_RfCfgEle));
    printf("\r\nCalibration cfg:        %p, len:%d", &(NvRamDataAddr->CalCfgEle), sizeof(T_CalCfgEle));
    printf("\r\nCalibration general:    %p, len:%d", &(NvRamDataAddr->CaliGenCfgEle), sizeof(T_CaliGenCfgEle));
    printf("\r\nCalibration data:       %p, len:%d",   NvRamDataAddr->CaliDataEle, sizeof(NvRamDataAddr->CaliDataEle));
    printf("\r\nACL cfg:                %p, len:%d", &(NvRamDataAddr->ACLCfgEle), sizeof(T_ACLCfgEle));
    printf("\r\nSFID cfg:               %p, len:%d", &(NvRamDataAddr->SFIDCfgEle), sizeof(T_SFIDCfgEle));
    printf("\r\nBTS neighbor list cfg:  %p, len:%d", &(NvRamDataAddr->BtsNeighborCfgEle), sizeof(T_BtsNeighborCfgEle));
    //printf("\r\nBTS neighbor LoadInfo:  %p, len:%d", &(NvRamDataAddr->NeighbotBTSLoadInfoEle), sizeof(T_NeighbotBTSLoadInfoEle));
    printf("\r\nBTS repeater cfg:       %p, len:%d", &(NvRamDataAddr->BTSRepeaterEle), sizeof(T_BTSRepeaterEle));
    printf("\r\nBTS telnet user cfg:    %p, len:%d", &(NvRamDataAddr->BTSUserCfgEle), sizeof(T_BTSUserCfgEle));
    printf("\r\nBTS VLAN group  cfg:    %p, len:%d", &(NvRamDataAddr->VlanGroupCfgEle), sizeof(T_VlanGroupCfgEle));
    printf("\r\nN=1 parameter   cfg:    %p, len:%d", &(NvRamDataAddr->N_parameter), sizeof(T_N_Parameter));
    printf("\r\nBTS neighbor list for common cfg:  %p, len:%d", &(NvRamDataAddr->BtsNeighborCommCfgEle), sizeof(T_BtsNeighborCfgEle));
       printf("\r\n T_UtHandoverPara  cfg:%p,len:%d", &(NvRamDataAddr->UtHandoverPara),sizeof(T_UtHandoverPara));
  #if 1
   printf("\r\nT_RangingPara RangingPara cfg:%p,len:%d", &(NvRamDataAddr->RangingPara),sizeof(T_RangingPara));//远距离Ranging参数
#ifdef WBBU_CODE
       printf("\r\nT_WRRUDataEle cfg:%p,len:%d",  &(NvRamDataAddr->WRRUCfgEle),sizeof(T_WRRUDataEle));
#endif       
	printf("\r\nT_CLUSTER_PARA cfg:%p,len:%d",  &(NvRamDataAddr->ClusterPara),sizeof(T_CLUSTER_PARA));//集群功能及参数配置
#ifdef WBBU_CODE
	printf("\r\nT_CalHardWare_Para cfg:%p,len:%d",  &(NvRamDataAddr->Cal_Para),sizeof(T_CalHardWare_Para));//wangwenhua add 20091030
#endif

    printf("\r\nT_SagBkp cfg:%p,len:%d",  &(NvRamDataAddr->SagBkp),sizeof(T_SagBkp)); 
    printf("\r\nT_JitterBuf cfg:%p,len:%d", &(NvRamDataAddr->JitterBuf),sizeof(T_JitterBuf)); 
    printf("\r\nT_SagTos cfg:%p,len:%d",  &(NvRamDataAddr->SagTos),sizeof(T_SagTos)); 

    printf("\r\nT_SavePwrCfg cfg:%p,len:%d", &(NvRamDataAddr->SavePwr),sizeof(T_SavePwrCfg)); 
	printf("\r\nT_Qam64Cfg cfg:%p,len:%d", &(NvRamDataAddr->Qam64Cfg),sizeof(T_Qam64Cfg));
    printf("\r\nT_AlarmFlag cfg:%p,len:%d", &(NvRamDataAddr->AlarmFlag),sizeof(T_AlarmFlag));
#endif
    printf("\r\n");
}

extern "C" void sysBootStateShow();
void sysShow()
{
    CTaskSystem::GetInstance()->showSysStatus();
    printf("\r\n");
    sysBootStateShow();
}

extern "C" void saveSrvCfgInfoToFile();
extern "C" void writesaccfg()
{    
    FILE *fd;
	saveSrvCfgInfoToFile();
       DIR* pdir = opendir( SM_USR_BOOT_DIR );
        if( NULL == pdir )
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "[tSys] opendir( /ata0a/usr/ ) error");
            if( OK != mkdir( SM_USR_BOOT_DIR ) )
            {
                OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "[tSys] mkdir( /ata0a/usr/ ) error");
                return ;//创建路径失败
            }
        }
        else
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "[tSys] opendir( /ata0a/usr/ ) success");
            if(OK != closedir( pdir ))
            {
                OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "[tSys] closedir() error");
                return;
            }
        }
        
        if(OK != chdir( SM_USR_BOOT_DIR ))
        {
            OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "[tSys] chdir( /ata0a/usr/ ) error");
            return;
        }
        OAM_LOGSTR(LOG_SEVERE, L3SYS_ERROR_RESET_BTS, "[tSys] chdir( /ata0a/usr/ ) success");
		
    if((userFileRecord.User_ACL_List.flag==0x6789)&&(userFileRecord.User_ACL_List.fileData!=NULL))
    {
        if((fd = fopen ((const char*)userFileRecord.User_ACL_List.userFileName, "w+")) == (FILE *)ERROR)
        {
            printf("write %s to CF Card fail, can't open file\n", userFileRecord.User_ACL_List.userFileName);
        }
	 else
	 {
	     fwrite( (const void *)userFileRecord.User_ACL_List.fileData, 1,userFileRecord.User_ACL_List.userFileLen, fd);	     
	     fclose(fd);
	     printf("write %s to CF Card succ\n", userFileRecord.User_ACL_List.userFileName);
	 }
    }
    else
    {
        printf("write %s to CF Card fail, flag error or data is null \n", userFileRecord.User_ACL_List.userFileName);
    }
    if((userFileRecord.User_Voice_List.flag==0x6789)&&(userFileRecord.User_Voice_List.fileData!=NULL))
    {
        if((fd = fopen ((const char*)userFileRecord.User_Voice_List.userFileName, "w+")) == (FILE *)ERROR)
        {
            printf("write %s to CF Card fail, can't open file\n", userFileRecord.User_Voice_List.userFileName);
        }
	 else
	 {
	     fwrite( (const void *)userFileRecord.User_Voice_List.fileData, 1,userFileRecord.User_Voice_List.userFileLen, fd);
	     fclose(fd);
	     printf("write %s to CF Card succ\n", userFileRecord.User_Voice_List.userFileName);
	 }
    }
    else
    {
        printf("write %s to CF Card fail, flag error or data is null \n", userFileRecord.User_Voice_List.userFileName);
    }
    if((userFileRecord.Trunk_Group_list.flag==0x6789)&&(userFileRecord.Trunk_Group_list.fileData!=NULL))
    {
        if((fd = fopen ((const char*)userFileRecord.Trunk_Group_list.userFileName, "w+")) == (FILE *)ERROR)
        {
            printf("write %s to CF Card fail, can't open file\n", userFileRecord.Trunk_Group_list.userFileName);
        }
	 else
	 {
	     fwrite( (const void *)userFileRecord.Trunk_Group_list.fileData, 1,userFileRecord.Trunk_Group_list.userFileLen, fd);
	     fclose(fd);
	     printf("write %s to CF Card succ\n", userFileRecord.Trunk_Group_list.userFileName);
	 }
    }
    else
    {
        printf("write %s to CF Card fail, flag error or data is null \n", userFileRecord.Trunk_Group_list.userFileName);
    }
    if((userFileRecord.Trunk_Group_User_List.flag==0x6789)&&(userFileRecord.Trunk_Group_User_List.fileData!=NULL))
    {
        if((fd = fopen ((const char*)userFileRecord.Trunk_Group_User_List.userFileName, "w+")) == (FILE *)ERROR)
        {
            printf("write %s to CF Card fail, can't open file\n", userFileRecord.Trunk_Group_User_List.userFileName);
        }
	 else
	 {
	     fwrite( (const void *)userFileRecord.Trunk_Group_User_List.fileData, 1,userFileRecord.Trunk_Group_User_List.userFileLen, fd);
	     fclose(fd);
	     printf("write %s to CF Card succ\n", userFileRecord.Trunk_Group_User_List.userFileName);
	 }
    }
    else
    {
        printf("write %s to CF Card fail, flag error or data is null \n", userFileRecord.Trunk_Group_User_List.userFileName);
    }
}

extern "C" void showsaccfg()
{
	printf("\r\nfile %s ---->\n", userFileRecord.User_ACL_List.userFileName);
	printf("\r\nfile len %d\n", userFileRecord.User_ACL_List.userFileLen);
	printf("\r\nfile data addr:%x\n", userFileRecord.User_ACL_List.fileData);
	printf("\r\n file flag :%x", userFileRecord.User_ACL_List.flag);

	printf("\r\nfile %s ---->\n", userFileRecord.User_Voice_List.userFileName);
	printf("\r\nfile len %d\n", userFileRecord.User_Voice_List.userFileLen);
	printf("\r\nfile data addr:%x\n", userFileRecord.User_Voice_List.fileData);
       printf("\r\n file flag :%x", userFileRecord.User_Voice_List.flag);
	   
	printf("\r\nfile %s ---->\n", userFileRecord.Trunk_Group_list.userFileName);
	printf("\r\nfile len %d\n", userFileRecord.Trunk_Group_list.userFileLen);
	printf("\r\nfile data addr:%x\n", userFileRecord.Trunk_Group_list.fileData);
	printf("\r\n file flag :%x", userFileRecord.Trunk_Group_list.flag);

	printf("\r\nfile %s ---->\n", userFileRecord.Trunk_Group_User_List.userFileName);
	printf("\r\nfile len %d\n", userFileRecord.Trunk_Group_User_List.userFileLen);
	printf("\r\nfile data addr:%x\n", userFileRecord.Trunk_Group_User_List.fileData);
	printf("\r\n file flag :%x", userFileRecord.Trunk_Group_User_List.flag);
}



/****************below is add by caoweiyi for rrm log trans*****************/
//#include "TraceTrans.h"

#define VXWORKS

#ifdef WIN32
#ifndef LCC
#include <windows.h>
#include <winsock.h>
#endif
#endif//WIN32

#ifdef VXWORKS

#ifndef __stdcall
#define __stdcall
#endif 

#include <vxWorks.h>
#include <msgQLib.h>
#include <ioLib.h>
#include <bootLib.h>
#include <taskLib.h>

/* vxworks includes */
#include <errno.h>
#include <ioctl.h>
#include <time.h>
#include <sys/times.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<signal.h>

/*
 * Networking support
 */
#include <socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
#ifndef WBBU_CODE
#include <net/inet.h>
#endif
#include <net/mbuf.h>
#endif//VXWORKS



#ifdef VXWORKS
#define REMOTE_TRACE
#endif

#ifdef PRJ_RRM_TEST
#undef REMOTE_TRACE
#endif
#if !defined VXWORKS && !defined WIN32
#ifdef REMOTE_TRACE
#undef REMOTE_TRACE
#endif
#endif//#if !defined VXWORKS && !defined WIN32

#if defined REMOTE_TRACE

/*#define TRACETEST*/

static int gTraceTransTid = 0;/*the trans task id*/

/*the socket and the port of l3*/
static int g_trace_server_port_l3 = 7000;
static int g_sock_trace_l3 = -1;
static int g_sock_listen_l3 = -1;

/*the sock and the host,port of l2*/
static char g_trace_server_host_l2[20];
static int g_trace_server_port_l2 = 0;
static int g_sock_trace_l2 = -1;


#ifndef WIN32
#define closesocket close

#define L2_SYS_CLK_RATE 100

extern "C"
void SleepL3(int ms)
{
    taskDelay(ms*L2_SYS_CLK_RATE/1000);
}
#endif

extern "C"
{

typedef struct 
{
    int l_onoff;
    int l_linger;
}sockopt_val;


int connectTCPL3(char *host,u_short port)
{
    struct sockaddr_in sin;
    int sfd,rv;

    sin.sin_family=AF_INET;
    sin.sin_port=htons(port);
    sin.sin_addr.s_addr=inet_addr(host);
    memset(&(sin.sin_zero),0,8);

    sfd=socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1)
    {
        perror("socket");
        return -1;
    }
	
    rv=connect(sfd,(struct sockaddr *)&sin,sizeof(struct sockaddr));
    if(rv==-1)
    {
        closesocket(sfd);
        return -1;
    }
    return sfd;
}


int passiveTCPL3(short port)
{
    struct sockaddr_in loc_addr;
    int sfd;
    int rv;
	//sockopt_val OptionValue;/**/
	int opt = 1;	

    sfd=socket(AF_INET,SOCK_STREAM,0);

    if(sfd==-1)
    {
        perror("socket");
        return -1;
    }

	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int)) != 0)
	{
		closesocket(sfd);
		return -1;
	}

    loc_addr.sin_family=AF_INET;
    loc_addr.sin_port=htons(port);
    loc_addr.sin_addr.s_addr=INADDR_ANY;
    memset(&(loc_addr.sin_zero),0,8);
    rv=bind(sfd,(struct sockaddr*)&loc_addr,sizeof(struct sockaddr));
    if(rv==-1)
    {
        perror("passiveTCPL3, bind");
        closesocket(sfd);
        return -1;
    }
	
    rv=listen(sfd,5);
    if(rv==-1)
    {
        perror("listen");
        closesocket(sfd);
        return -1;
    }
    return sfd;

}


/*set the layer3 port*/
void SetTraceTransPortL3(int port)
{
	g_trace_server_port_l3 = port;
	
	if(-1 != g_sock_listen_l3){
	    closesocket(g_sock_listen_l3);
	    g_sock_listen_l3 = -1;
	}
	
	if(-1 != g_sock_trace_l3){
	    closesocket(g_sock_trace_l3);
	    g_sock_trace_l3 = -1;
	}
	
	do{
		g_sock_listen_l3 = passiveTCPL3(g_trace_server_port_l3);
		if(g_sock_listen_l3>0)
			break;
		SleepL3(1000);
	}while(1);
	
#ifdef WIN32	
	printf("g_sock_listen_l3 = %d \n",g_sock_listen_l3);
#else
	LOG1(LOG_CRITICAL, 0, "g_sock_listen_l3 = %d ",g_sock_listen_l3);
#endif

}


#ifdef WIN32
ULONG __stdcall __trace_trans_task(void *arg)
#elif defined VXWORKS
int __trace_trans_task()
#endif
{
	static char buf1[1000];
	static char *cur = buf1;	
	char *pc;
	int r,i;
	
	fd_set rfds_l3,rfds_l2;
	struct timeval tv;
	int n;
	char temp[200];

	/*int last_heart_beat_time=Now();*/

#ifdef WIN32
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_IDLE);
#endif

	SetTraceTransPortL3(7000);

	for(;;){
		FD_ZERO(&rfds_l3);
		if(g_sock_listen_l3 != -1)
			FD_SET(g_sock_listen_l3, &rfds_l3);
		if(g_sock_trace_l3 != -1){
			FD_SET(g_sock_trace_l3, &rfds_l3);
			if(-1 == g_sock_trace_l2){
#ifdef WIN32
				printf("connectTCP to L2: g_trace_server_host_l2 %s g_trace_server_port_l2 %d \n",
					g_trace_server_host_l2,g_trace_server_port_l2);
#else
				sprintf(temp, "connectTCP to L2: g_trace_server_host_l2 %s g_trace_server_port_l2 %d ",
					g_trace_server_host_l2,g_trace_server_port_l2);
				LOG(LOG_CRITICAL, 0, temp);
#endif
				g_sock_trace_l2 = connectTCPL3(g_trace_server_host_l2, g_trace_server_port_l2);
				if(g_sock_trace_l2 == -1){
			        SleepL3(100);
					continue;
				}
				else{
#ifdef WIN32							
					printf("g_sock_trace_l2 = %d \n",g_sock_trace_l2);
#else
					LOG1(LOG_CRITICAL, 0, "g_sock_trace_l2 = %d ",g_sock_trace_l2);
#endif
				}
			}
		}

		tv.tv_sec=0;
		tv.tv_usec=1000;
		n=select(FD_SETSIZE-1,&rfds_l3,0,0,&tv);

		if(n>0){
			if(FD_ISSET(g_sock_listen_l3, &rfds_l3)){
				struct sockaddr_in fsin;
				int alen = sizeof(fsin);
				if(g_sock_trace_l3 != -1){
					close(g_sock_trace_l3);
					g_sock_trace_l3 = -1;
				}
				g_sock_trace_l3 = accept(g_sock_listen_l3, (struct sockaddr*)&fsin, &alen);
				if(g_sock_trace_l3 > 0){
#ifdef WIN32                
                printf("accept from pc 0x%x ... \n",fsin.sin_addr.s_addr);
  
#else
				sprintf(temp, "accept from pc 0x%x ... ",fsin.sin_addr.s_addr);
				LOG(LOG_CRITICAL, 0, temp);
#endif

				}
			}

			/*recv and retrans cmd to the L2*/
			if(FD_ISSET(g_sock_trace_l3, &rfds_l3)){
				static char buf[400];
				r=recv(g_sock_trace_l3, (char*)buf, sizeof(buf), 0);
				if(r > 0){
					/*send the cmd to the L2*/
					if(-1 == g_sock_trace_l2){
#ifdef WIN32
						printf("connectTCP to L2: g_trace_server_host_l2 %s g_trace_server_port_l2 %d \n",
							g_trace_server_host_l2,g_trace_server_port_l2);	
#else
						sprintf(temp, "connectTCP to L2: g_trace_server_host_l2 %s g_trace_server_port_l2 %d ",
							g_trace_server_host_l2,g_trace_server_port_l2);
						LOG(LOG_CRITICAL, 0, temp);
#endif						
						g_sock_trace_l2 = connectTCPL3(g_trace_server_host_l2, g_trace_server_port_l2);
						if(g_sock_trace_l2 == -1){
			                SleepL3(100);
							continue;
						}
						else{
#ifdef WIN32							
							printf("g_sock_trace_l2 = %d \n",g_sock_trace_l2);
#else
							LOG1(LOG_CRITICAL, 0, "g_sock_trace_l2 = %d ",g_sock_trace_l2);
#endif
						}
					}
					r=send(g_sock_trace_l2,(char *)buf,r,0);			
					if(r == -1){
						close(g_sock_trace_l2);
						g_sock_trace_l2=-1;
					}
				}
				else if(r == -1){
					close(g_sock_trace_l3);
					g_sock_trace_l3=-1;					
				}
			}
		}

		FD_ZERO(&rfds_l2);
		if(g_sock_trace_l2 != -1){
			FD_SET(g_sock_trace_l2, &rfds_l2);
		}
		tv.tv_sec = 0;
		tv.tv_usec = 10000;/*10 ms*/
		n = select(FD_SETSIZE-1, &rfds_l2, 0, 0, &tv);
		if(n <= 0){
			SleepL3(10);
			continue; 
		}

		if(FD_ISSET(g_sock_trace_l2, &rfds_l2)){
			r = recv(g_sock_trace_l2, (char*)buf1, sizeof(buf1), 0);
			if(r > 0){
				/*send the trace msg to the client*/
				if(-1 != g_sock_trace_l3){					
					r = send(g_sock_trace_l3,buf1,r,0);
					if(r == -1){
						closesocket(g_sock_trace_l3);
						g_sock_trace_l3 = -1;
					}
				}
				
			}
			else if(r == -1){
				/*L2Log("Ln%d: recv() == -1, close g_sock_trace_l2 ... \n",buf1);*/
				closesocket(g_sock_trace_l2);
				g_sock_trace_l2 = -1;
			}
			else{
				SleepL3(10);
			}
		}	
		
		/*SleepL3(10);*/
		
	}	

}

void start_tarce_trans_task()
{
#ifdef WIN32
	ULONG thid;
	CreateThread(0,0,__trace_trans_task,0,0,&thid);
#endif

#ifdef VXWORKS
	LOG(LOG_CRITICAL, 0, "__trace_trans_task taskSpawn...............");
	gTraceTransTid = taskSpawn("__trace_trans_task", 251, 0, 20000, (FUNCPTR)__trace_trans_task, 1, 2, 3, 4, 5, 0, 0, 0, 0, 0);
#endif
}

static int __remote_trace_trans_init()
{

	strcpy(g_trace_server_host_l2,"10.0.0.2");
	g_trace_server_port_l2=7000;	
		
	start_tarce_trans_task();	
	SleepL3(-1);		
	return 0;
}

#endif//REMOTE_TRACE

}//extern "C"



extern "C"
{

int TransBegin()
{

#ifdef REMOTE_TRACE

	return __remote_trace_trans_init();
#endif

    return 0;
}


void TransEnd()
{
#ifdef VXWORKS

	if(-1 != g_sock_trace_l2){
	    closesocket(g_sock_trace_l2);
	    g_sock_trace_l2 = -1;
	}
	
	if(-1 != g_sock_trace_l3){
	    closesocket(g_sock_trace_l3);
	    g_sock_trace_l3 = -1;
	}
	
	if(-1 != g_sock_listen_l3){
	    closesocket(g_sock_listen_l3);
	    g_sock_listen_l3 = -1;
	}

	LOG(LOG_CRITICAL, 0, "Task __trace_trans_task is deleted");
	//LOG(LOG_CRITICAL, 0, "U'd better don't spawn this task in 2 minutes!!!!!!!!!!!");
	
	taskDelete(gTraceTransTid);

#endif
}

}//extern "C"

