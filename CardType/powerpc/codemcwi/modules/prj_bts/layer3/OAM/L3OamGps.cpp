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
 *   10/31/2006   石昱舒/肖卫方修改
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/
#pragma warning (disable : 4786)
#ifdef __WIN32_SIM__
#include <windows.h>
#else
#ifndef __INCtaskLibh
#include <vxWorks.h>
#include <time.h>
#include <selectLib.h>
#include <ctype.h>
#endif
#include <ioLib.h>
#endif
#include <stdio.h>
#include <string.h>

#ifndef __WIN32_SIM__
#include "mcWill_bts.h"
#endif

#include "L3OamGps.h"
#include "MsgQueue.h"
#include "Log.h"
#include "ErrorCodeDef.h"
//#include "L3OamTest.h"
#include "L3EmsMessageId.h"
#include "L3OamCfgCommon.h"
#include "L3OamCfgGpsDataReq.h"     
#include "L3OamAlmInfo.h"
#include "L3OamAlmNotifyOam.h"
#include "L3OamCommonReq.h"
#include "L3OamCfgGpsLocNotify.h"     
#include "Transaction.h"
#include "L3OamCommonRsp.h"
#include "L3L2MessageId.h"
#include "L3OamCfgL1GenDataReq.h"
#include "L3OamAlm.h"
#include "L3OamCfg.h"
#ifdef WBBU_CODE
#include "sysBtsConfigData.h"
#endif
#include <taskLib.h>
/////////////////////////////////////////////////////////////////////////////////////
#define GPS_HEARTBEAT_PERIOD            (30 * 1000)     /*30seconds */
#define RTC_UPDATE_PERIOD               (10*60*1000)    /*10 minutes*/
#define GPS_MAX_CMD_LENGTH              (25)
#define GPS_MAX_RESP_LENGTH             (300)  /*max response length, excluding receiver ID message*/
#define GPS_SEND_RECEIVE_RETRY_COUNT    (2)
#define GPS_READ_DATA_TIMEOUT_PERIOD    (2)
#define GPS_HEADER_LENGTH               (2)
#define GPS_COMMAND_LENGTH              (2)
#define CR                              (0x0D)
#define LF                              (0x0A)

#define GPS_LOG_CODE        LOGNO(GPS, 0)
#define ERR_GPS_TIMED_OUT   LOGNO(GPS, 1)
/////////////////////////////////////////////////////////////////////////////////////


extern T_NvRamData *NvRamDataAddr;
////extern UINT16 gRFMaskCfg;
CTaskGPS*   CTaskGPS::m_Instance = NULL;
bool    bGpsStatus = false; 
bool bGpsInitTime = false;
UINT8   gBtsVisbleSatcnt = 0;
UINT8   gBtstrackSatcnt = 0;
extern UINT8  Cal_Data_Cfg_Return_Ok ;
extern UINT8  GPS_Cal_OK_Send_Cmd ;
int gpsModel = 0xff;
#ifdef WBBU_CODE
 UINT8     bGpsStatus_RF= 1;
extern unsigned char  Hard_VerSion ;
T_GpsAllData gUbloxGpsAllData;
UBLOX_SData gUbloxSData;
bool isFirstMsg = true;//第一条判断是否为ublox消息
UINT32 g_close_RF_dueto_Slave = 0;
#endif
extern void l3oamactcalibration();
extern "C"
{
void rtcDateSet(int year, int month,  int dayOfMonth, int dayOfWeek);
void rtcTimeSet(int hour, int minute, int second);
int bspNvRamWrite(char * TargAddr, char *ScrBuff, int size);
#ifdef WBBU_CODE
void Set_Fpga_Clk(unsigned char type);
void WrruRFC(unsigned char antennamask,unsigned char flag,unsigned char flag1);
void bspSetTime(char*pbuf);
extern unsigned char	Calibration_Antenna ;
//zengjihan 20120801 for GPSSYNC
unsigned int  bspGetslaveBTSID();
#endif
}

extern bool AlarmReport(UINT8   Flag, UINT16  EntityType,
                 UINT16  EntityIndex, UINT16  AlarmCode,      
                 UINT8   Severity, const char format[],...);

static const struct {
    char code[2];               // command/response code
    int  cmd_length;            // number of bytes in command (to GPS)
    int  resp_length;           // number of bytes in response (from GPS)
}

GPS_COMMAND_TABLE[/* GPS_COMMANDS */] = 
{
    /* these must match the order of GPS_COMMANDS exactly */
    { { 'A','a' },  10,  10 },    // CMD_TIME_OF_DAY
    { { 'A','b' },  10,  10 },    // CMD_GMT_CORRECTION
    { { 'A','c' },  11,  11 },    // CMD_DATE
    { { 'A','g' },  8,  8 },    // CMD_SATELLITE_MASK_ANGLE
    { { 'A','P' },  8,  8 },    // CMD_PULSE_MODE
    { { 'A','s' }, 20, 20 },    // CMD_POSITION_HOLD_POSITION
    { { 'A','w' },  8,  8 },    // CMD_TIME_MODE
    { { 'A','z' }, 11, 11 },    // CMD_CABLE_DELAY
    { { 'C','f' },  7,  7 },    // CMD_SET_TO_DEFAULTS
    { { 'C','j' },  7,294 },    // CMD_RECEIVER_ID
    { { 'G','a' }, 20, 20 },    // CMD_M12_POSITION
    { { 'G','b' }, 17, 17 },    // CMD_M12_TIME
    { { 'G','c' },  8,  8 },    // CMD_M12_PPS_CONTROL
    { { 'G','d' },  8,  8 },    // CMD_M12_POSITION_HOLD_MODE
    { { 'G','e' },  8,  8 },    // CMD_M12_TIME_RAIM_ALGORITHM
    { { 'G','f' },  9,  9 },    // CMD_M12_TIME_RAIM_ALARM    
    { { 'H','a' },  8,154 },    // CMD_M12_POSITION_STATUS_DATA
    { { 'H','n' },  8, 78 },    // CMD_M12_TIME_RAIM_STATUS
    { { 'I','a' },  7, 10 },    // CMD_M12_SELF_TEST
    { { 'S','z' },  0,  8 },    // CMD_POWER_ON_FAILURE
                                // 
    { { '?','?' },  0,  0 }     // CMD_UNKNOWN
};

CTaskGPS::CTaskGPS()
{
    strcpy(m_szName, "tGPS");
    m_uPriority   = M_TP_L3GM;
    m_uOptions    = 0;
    m_uStackSize  = SIZE_KBYTE * 50;
    m_iMsgQMax    = 100;
    m_iMsgQOption = 0;

    m_gpssfd            = 0;
    m_pGPSHeartBeatTimer= 0;
    m_RtcUpdateInterval = 0;
    m_gpsSemId = NULL;
    m_bInitSIO          = false;
    m_gpsWorkDownNum = 0;
    m_gpsRcd = GPS_OPR_INIT;
}

bool CTaskGPS::Initialize()
{
    T_NVRAM_BTS_OTHER_PARA *para = (T_NVRAM_BTS_OTHER_PARA*)NVRAM_BASE_ADDR_OTHER_PARAS;
    CBizTask :: Initialize();

    m_gpssfd = open("/tyCo/1", O_RDWR, 0);
    if(ERROR == m_gpssfd)
        {
        printf("open gps error:%x\n",m_gpssfd);
        return false;
        }
#ifdef WBBU_CODE
 if(ERROR == ioctl(m_gpssfd, FIOBAUDRATE, 9504 ))//wbbu实际频率是9.61k 
#else

    if(ERROR == ioctl(m_gpssfd, FIOBAUDRATE, 4800 ))
#endif
        {
        return false;
        }
    #ifdef WBBU_CODE
    gpsModel = gpsJudgeModel();    
    if(gpsModel == GPS_MODEL_M12)
        printf("GPS is M12!!\n");
    else if(gpsModel == GPS_MODEL_UBLOX)
        printf("GPS is UBLOX!!\n");
    else
        printf("GPS is Unknown!!\n");   
    if(gpsModel ==GPS_MODEL_M12)
    {
        m_bInitSIO = gpsInitSIO();  //
    }
    else
    {        
        m_bInitSIO = gpsInitSIO_UBlox();        
        memset(&gUbloxGpsAllData, 0, sizeof(T_GpsAllData));
        memset(&gUbloxSData, 0, sizeof(UBLOX_SData));
    }
    #else
    m_bInitSIO = gpsInitSIO();  //
    #endif
    //启动心跳定时器
    m_pGPSHeartBeatTimer = gpsCreatetimer(M_OAM_GPS_HEARTBEAT_TIMER, true, GPS_HEARTBEAT_PERIOD);
    if(NULL != m_pGPSHeartBeatTimer)
    {
        m_pGPSHeartBeatTimer->Start();
    }
    else
    {
        return false;
    }

    m_gpsSemId = semMCreate(SEM_Q_PRIORITY |SEM_INVERSION_SAFE | SEM_DELETE_SAFE );
    if (NULL == m_gpsSemId)
    {
        return false;
    }
    if(para->gpsSetFlag == 0x5a5a)
        gpsSetCloseTimer(para->gpscloseRFtimer * 2);
    else
	gpsSetCloseTimer(40);
    //如果同步需要的最小卫星数为0则默认修改为3
    if(NvRamDataAddr->GpsDataCfgEle.SatelliteCnt<3)
    {
        T_GpsDataCfgEle tmp;
        memcpy((char*)&tmp, (char*)&(NvRamDataAddr->GpsDataCfgEle), sizeof(T_GpsDataCfgEle));
        tmp.SatelliteCnt = 3;
        CTaskCfg::l3oambspNvRamWrite((char*)&(NvRamDataAddr->GpsDataCfgEle), (char*)&tmp, sizeof(T_GpsDataCfgEle));
    }
    return true;
}


int CTaskGPS::gpsSetGpsTime()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;

    //T_TimeDate timeData = bspGetDateTime();
    //@@Gb
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME].code[1];
    cmd[cmdlen++] = 0;//timeData.month;
    cmd[cmdlen++] = 0;//timeData.day;
    cmd[cmdlen++] = 0;//timeData.year>>8;
    cmd[cmdlen++] = 0;//timeData.year&0xff;
    cmd[cmdlen++] = 0;//timeData.hour;
    cmd[cmdlen++] = 0;//timeData.minute;
    cmd[cmdlen++] = 0;//timeData.second;
    
    int Offset = NvRamDataAddr->GpsDataCfgEle.GMTOffset;
    OAM_LOGSTR2(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] set GPS time with OFFSET[%d Hour, %d Minutes]", (char)(Offset/60), (char)(Offset%60));
    if (Offset < 0)
        {
        cmd[cmdlen++] = 0xff;   //signed byte of GMT offset; ff:negative
        Offset = -Offset;
        } 
    else 
        {
        cmd[cmdlen++] = 0;
        }
    cmd[cmdlen++] = (char) (Offset / 60);   //hour of GMT offset
    cmd[cmdlen++] = (char) (Offset % 60);   //minutes of GMT offset

    status = gpsExecCommand(cmd, cmdlen, 3);
    if (OK != status)
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! set GPS time with configured time failed");
        }
    return status;
}


int CTaskGPS::gpsSetGpsLocation()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;

    T_GpsDataCfgEle *pGpsCfgPostion = &(NvRamDataAddr->GpsDataCfgEle);
    if (  (0 == pGpsCfgPostion->Latitude)
        &&(0 == pGpsCfgPostion->Longitude)
        &&(0 == pGpsCfgPostion->Height))
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] Position data{0,0,0}"); 
////    return ERROR;
        }

    //@@Ga:change current position message.
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION].code[1];

    *((SINT32*)(cmd + cmdlen)) = pGpsCfgPostion->Latitude;   //latitude in mas.
    cmdlen += sizeof(SINT32);
    *((SINT32*)(cmd + cmdlen)) = pGpsCfgPostion->Longitude;   //Longitude in mas.
    cmdlen += sizeof(SINT32);
    *((SINT32*)(cmd + cmdlen)) = pGpsCfgPostion->Height;   //Height in mas.
    cmdlen += sizeof(SINT32);
    cmd[cmdlen++] = 0x0;   // 0 = GPS    1 = MSL

    status = gpsExecCommand(cmd, cmdlen, 3);
    if (OK != status)
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! set GPS position with configured position failed");
        }
    return status;
}

bool CTaskGPS::gpsInitSIO()
{
#ifndef __WIN32_SIM__
#if 0   // moved to initialize, only open once
    m_gpssfd = open("/tyCo/1", O_RDWR, 0);
    if(ERROR == m_gpssfd)
        {
        return false;
        }
    if(ERROR == ioctl(m_gpssfd, FIOBAUDRATE, 4800 ))
        {
        return false;
        }
#endif
    UINT16 tSyncSrc =  NvRamDataAddr->L1GenCfgEle.SyncSrc;
    /**0***/
    //100HZ PPS Enable.
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
    //@@AP
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_PULSE_MODE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_PULSE_MODE].code[1];
    cmd[cmdlen++] = 0x1;
    status = gpsExecCommand(cmd, cmdlen, 3);
    if (OK != status)
    {
        if(BTS_SYNC_SRC_GPS == tSyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! set GPS 100HZ PPS(enable) failed");
        return false;
    }

    //1PPS control. 0: disable, 1:continuously, 
    cmdlen = 0;
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_PPS_CONTROL].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_PPS_CONTROL].code[1];
#ifndef WBBU_CODE
    cmd[cmdlen++] = 0x1;/*disable*/
#else
    cmd[cmdlen++] = 2/*0x1*/;/*disable wangwenhua modify 20091024*/
#endif
    status = gpsExecCommand(cmd, cmdlen, 3);
    if (OK != status)
    {
        if(BTS_SYNC_SRC_GPS == tSyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! set GPS 1PPS control(disable) failed");
        return false;
    }
    
    /**3***/    
    //set current UTC time correction
    cmdlen = 0;
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_TIME_MODE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_TIME_MODE].code[1];
    cmd[cmdlen++] = 1;    // set time mode to GPS mode
    status = gpsExecCommand(cmd, cmdlen, 3);
    if (OK != status)
    {
        if(BTS_SYNC_SRC_GPS == tSyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! set GPS current UTC time correction failed");
        return false;
    }

    if (OK != gpsSetPositionControl(0)) // disable position hold
    {
        if(BTS_SYNC_SRC_GPS == tSyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! set GPS control mode(0) failed");
        return false;
    }
    #if 0
    if (OK != gpsSetGpsTime())
    {
        if(BTS_SYNC_SRC_GPS == tSyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! Init GPS time from NVRAM failed.");
        return false;
    }
    #endif
    if (OK != gpsSetGpsLocation())
    {
        if(BTS_SYNC_SRC_GPS == tSyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! Init GPS position from NVRAM failed.");
        return false;
    }
#endif
    return true; 
}

bool CTaskGPS::ProcessMessage(CMessage &rMsg)
{
    switch(rMsg.GetMessageId())
    {
        case M_EMS_BTS_GPS_DATA_CFG_REQ: 
        {
            OAM_LOGSTR(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] Receive from EMS GPS data config request.");
            gpsConfig();
            break;
        }

        case M_OAM_GPS_HEARTBEAT_TIMER:
        {
            #ifdef WBBU_CODE
            if(gpsModel ==GPS_MODEL_UBLOX)
                gpsHeartbeatTimeOut_UBlox();
            else
                gpsHeartbeatTimeOut();
            #else
            gpsHeartbeatTimeOut();
            #endif
            break; 
        }
        
        case M_L2_L3_L1_GENERAL_SETTING_RSP:
        {
            gpsGenRspHandle(rMsg);      
            break;  
        }
        case SEND_L1_MSG_TIMEOUT://add timeout process jiaying20100817
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
	     break;
        }
        case M_OAM_CFG_HLR_TIME_REQ:
        {
            gpsReturnHlrReq();
            break;
        }
        //zengjihan 20120801 for GPSSYNC
        #ifdef WBBU_CODE
        case M_OAM_GPS_SYNC_NOTIFY:
        {
            gpsSyncNotify(rMsg);
            break;
        }
        #endif     
     
        default:
        {
            OAM_LOGSTR1(LOG_DEBUG, L3GPS_ERROR_REV_ERR_MSG, "[tGPS] ERROR!!! receive error msg[0x%04x]", rMsg.GetMessageId());
            break;
        }
    }
  
    return true;
}


TID CTaskGPS::GetEntityId() const
{
   return M_TID_GM;
}

CTaskGPS* CTaskGPS::GetInstance()
{
    if ( NULL == m_Instance )
    {
        m_Instance = new CTaskGPS;
    }
    return m_Instance;
}


bool CTaskGPS::gpsGenRspHandle(CMessage &rMsg)
{
    CL3OamCommonRsp Rsp(rMsg);
    UINT16 TransId = Rsp.GetTransactionId();
    CTransaction * pTransaction = FindTransact(TransId);
    if(pTransaction)
    {
        pTransaction->EndTransact();
        delete pTransaction;
    }

    return true;
}

//zengjihan 20120801 for GPSSYNC
#ifdef WBBU_CODE
void CTaskGPS::gpsSyncNotify(CMessage &rMsg)
{
    LOG(LOG_DEBUG3, LOGNO(GPS,0), "CTaskGPS::gpsSyncNotify");
    char *pDataPtr = (char*)(rMsg.GetDataPtr());
    T_GpsAllData GpsAllData;
    if(0x55 == pDataPtr[11])
    {
        g_close_RF_dueto_Slave = 0;
        GpsAllData.Year    = (pDataPtr[0] << 8) + pDataPtr[1];
        GpsAllData.Month   = pDataPtr[2];
        GpsAllData.Day     = pDataPtr[3];
        GpsAllData.Hour    = pDataPtr[4];
        GpsAllData.Minute  = pDataPtr[5];
        GpsAllData.Second  = pDataPtr[6];
        /*****************************************************
        printf("GpsAllData.Year=%d\n",GpsAllData.Year);
        printf("GpsAllData.Month=%d\n",GpsAllData.Month);
        printf("GpsAllData.Day=%d\n",GpsAllData.Day);
        printf("GpsAllData.Hour=%d\n",GpsAllData.Hour);
        printf("GpsAllData.Minute=%d\n",GpsAllData.Minute);
        printf("GpsAllData.Second=%d\n",GpsAllData.Second);
        ******************************************************/
        if(BTS_SYNC_SRC_485!=NvRamDataAddr->L1GenCfgEle.SyncSrc)
	 {
#ifndef WBBU_CODE
		        rtcDateSet(GpsAllData.Year, GpsAllData.Month, GpsAllData.Day, 0);
		        rtcTimeSet(GpsAllData.Hour, GpsAllData.Minute, GpsAllData.Second);
#else
		        if((Hard_VerSion<5)||(Hard_VerSion>15))
		        {
		            rtcDateSet(GpsAllData.Year, GpsAllData.Month, GpsAllData.Day, 0);
		            rtcTimeSet(GpsAllData.Hour, GpsAllData.Minute, GpsAllData.Second);
		        }
		        else
		        {
		            char pbuffer[7];
		            pbuffer[0] = GpsAllData.Second;       //&0x7f;  /******s********/
		            pbuffer[1] = GpsAllData.Minute;        //&0x7f;  /******min*******/
		            pbuffer[2] = GpsAllData.Hour;           //&0x3f;  /*********hour*******/
		            pbuffer[3] = GpsAllData.Day;            //&0x3f; /*******day********/
		            pbuffer[4] = 5;                                 //&0x7; /****week day*****/
		            pbuffer[5] = GpsAllData.Month;         //&0x1f; /*******month*******/
		            pbuffer[6] = (GpsAllData.Year-2000);//&0xff; /**year***/
		            bspSetTime(pbuffer);
		        }
#endif

		        struct tm time_s;
		        T_TimeDate timeDate;
		        struct timespec timeSpec;
		        unsigned int secCount;
#ifndef WBBU_CODE
		        timeDate = bspGetDateTime();
		        time_s.tm_sec = timeDate.second;
		        time_s.tm_min = timeDate.minute;
		        time_s.tm_hour = timeDate.hour;

		        time_s.tm_mday = timeDate.day;
		        time_s.tm_mon  = timeDate.month - 1;
		        time_s.tm_year = timeDate.year - 1900;
#else
		        time_s.tm_sec = GpsAllData.Second;
		        time_s.tm_min = GpsAllData.Minute;
		        time_s.tm_hour = GpsAllData.Hour;

		        time_s.tm_mday = GpsAllData.Day;
		        time_s.tm_mon  = GpsAllData.Month - 1;

		        time_s.tm_year = GpsAllData.Year - 1900;
#endif
		        time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

		        secCount = mktime(&time_s);
		        timeSpec.tv_sec = secCount;
		        timeSpec.tv_nsec = 0;
		        clock_settime(CLOCK_REALTIME, &timeSpec);
        }

        if(GPS_OPR_GPS_OK != m_gpsRcd)
        {
            gpsConfigL1(NvRamDataAddr->L1GenCfgEle.SyncSrc, false);
            AlarmReport(ALM_FLAG_CLEAR, ALM_ENT_GPS, ALM_ENT_INDEX0,
                                      ALM_ID_GPS_LOST, ALM_CLASS_CRITICAL, STR_CLEAR_ALARM);
            m_gpsRcd = GPS_OPR_GPS_OK;
            OAM_LOGSTR(LOG_WARN, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS_OPR_GPS_OK.");
        }    
    }
    else if(0xaa == pDataPtr[11])
    {
        if(GPS_OPR_SEND_CLOSE_RF != m_gpsRcd)
        {
            gpsConfigL1(NvRamDataAddr->L1GenCfgEle.SyncSrc, true);
            /*gpsConfigRFMask();*/
            m_gpsRcd = GPS_OPR_SEND_CLOSE_RF;
            OAM_LOGSTR(LOG_WARN, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS_OPR_SEND_CLOSE_RF.");
        }
        g_close_RF_dueto_Slave = 1;
    }
    else
    {
        OAM_LOGSTR(LOG_WARN, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!Should not enter the branch.");
    }    
}
#endif

CTimer* CTaskGPS::gpsCreatetimer(UINT16 MsgId, bool IsPeriod, UINT32 TimerPeriod)
{
    CL3OamCommonReq TimerMsg;
    if (false == TimerMsg.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! System encounter exception, createMessage failed.");
        return NULL;
        }
    TimerMsg.SetDstTid(M_TID_GM);
    TimerMsg.SetSrcTid(M_TID_GM);
    TimerMsg.SetMessageId(MsgId);
    CTimer *p = new CTimer(IsPeriod, TimerPeriod, TimerMsg);
    if (NULL == p)
        {
        TimerMsg.DeleteMessage();
        }
    return p;
}

bool  CTaskGPS::gpsConfig()
{
    if(gpsModel == GPS_MODEL_UBLOX)//ublox直接将时区写入系统时间
    {    
        //设置重写RTC和系统时间
        m_RtcUpdateInterval = RTC_UPDATE_PERIOD/GPS_HEARTBEAT_PERIOD;        
        return true;
    }
    //配置数据已经在tCFG任务写入NVRAM.
    if (OK != gpsSetGpsTime())
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! Config GPS time from EMS failed.");
        }

    //可能GMToffset会改变，等待GPS心跳到来后，立即更改时间
    m_RtcUpdateInterval = RTC_UPDATE_PERIOD/GPS_HEARTBEAT_PERIOD;

    //配置数据中只有纬度、经度和高度可设置。GPS Offset 
    //和可视卫星数只可获取
    //命令Ga用来设置GPS Combined Position
    if (OK != gpsSetGpsLocation())
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! Config GPS position from EMS failed.");
        }

    return true;
}

bool CTaskGPS:: gpsReturnHlrReq()
{
    #ifndef WBBU_CODE
    if (false == m_bInitSIO)
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] Initialize GPS!");
        m_bInitSIO = gpsInitSIO();  
        if (false == m_bInitSIO)
            {
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR...Initialize GPS FAIL!");
            return false;
            }
        }
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;

    //@@Ha
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[1];
    // Enable the RAIM algorithm if the alarm limit is non-zero
    cmd[cmdlen++] = 0;

    char response[GPS_MAX_RESP_LENGTH] = {0};
    status = gpsExecCommand(cmd, cmdlen, 3, response);
    if (OK == status)
        {
        OAM_LOGSTR(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS module communication OK.");
        T_GpsAllData *pGpsData = (T_GpsAllData*)response;
        if (pGpsData->TrackedSatellites >= NvRamDataAddr->GpsDataCfgEle.SatelliteCnt)
        {            
            //GPS锁定.同步时间和位置信息
	      CComMessage *pComMsg = new(this, 16)CComMessage;
		if(NULL != pComMsg)
		{
			pComMsg->SetMessageId(M_OAM_CFG_HLR_TIME_REQ);
			pComMsg->SetSrcTid(M_TID_GM);
			pComMsg->SetDstTid(M_TID_EMSAGENTTX);	
			char *p = (char*)pComMsg->GetDataPtr();
			p[0] = (pGpsData->Year>>8)&0xff;
			p[1] = pGpsData->Year&0xff;
			p[2] = pGpsData->Month;
			p[3] = pGpsData->Day;
			p[4] = pGpsData->Hour;
			p[5] = pGpsData->Minute;
			p[6] = pGpsData->Second;
			memcpy(&p[7],&pGpsData->MinSec,4);
			if ( false == CComEntity::PostEntityMessage(pComMsg) )
			{
				pComMsg->Destroy();				
			}
		}
            return true;
            }
        }
    else
        {
        //GPS通信错误；
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS module communication fail, please check GPS hardware.");
        }
 
    return false;
    #else
    if (false == m_bInitSIO)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] Initialize GPS!");
        if(gpsModel ==GPS_MODEL_M12)
        {
            m_bInitSIO = gpsInitSIO();  //
        }
        else
        {        
            m_bInitSIO = gpsInitSIO_UBlox(); 
        }
        //m_bInitSIO = gpsInitSIO();  
        if (false == m_bInitSIO)
        {
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR...Initialize GPS FAIL!");
            return false;
        }
    }
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
    if(gpsModel ==GPS_MODEL_M12)
    {
        //@@Ha
        cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[0];
        cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[1];
        // Enable the RAIM algorithm if the alarm limit is non-zero
        cmd[cmdlen++] = 0;
        
        char response[GPS_MAX_RESP_LENGTH] = {0};
        status = gpsExecCommand(cmd, cmdlen, 3, response);
        if (OK == status)
        {
            OAM_LOGSTR(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS module communication OK.");
            T_GpsAllData *pGpsData = (T_GpsAllData*)response;
            if (pGpsData->TrackedSatellites >= NvRamDataAddr->GpsDataCfgEle.SatelliteCnt)
            {            
                //GPS锁定.同步时间和位置信息
                CComMessage *pComMsg = new(this, 16)CComMessage;
                if(NULL != pComMsg)
                {
                    pComMsg->SetMessageId(M_OAM_CFG_HLR_TIME_REQ);
                    pComMsg->SetSrcTid(M_TID_GM);
                    pComMsg->SetDstTid(M_TID_EMSAGENTTX);	
                    char *p = (char*)pComMsg->GetDataPtr();
                    p[0] = (pGpsData->Year>>8)&0xff;
                    p[1] = pGpsData->Year&0xff;
                    p[2] = pGpsData->Month;
                    p[3] = pGpsData->Day;
                    p[4] = pGpsData->Hour;
                    p[5] = pGpsData->Minute;
                    p[6] = pGpsData->Second;
                    memcpy(&p[7],&pGpsData->MinSec,4);
                    if ( false == CComEntity::PostEntityMessage(pComMsg) )
                    {
                    pComMsg->Destroy();				
                    }
                }
                return true;
            }
        }
        else
        {
            //GPS通信错误；
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS module communication fail, please check GPS hardware.");
        }
    }
    else
    {
        status = gps_NAV_SVInfo();
        if((status == OK)&& (gUbloxGpsAllData.TrackedSatellites >= NvRamDataAddr->GpsDataCfgEle.SatelliteCnt)  )
        {
            status = gps_NAV_TimeUTC();
            if(status == OK)
            {
                //GPS锁定.同步时间和位置信息
                CComMessage *pComMsg = new(this, 16)CComMessage;
                if(NULL != pComMsg)
                {
                    pComMsg->SetMessageId(M_OAM_CFG_HLR_TIME_REQ);
                    pComMsg->SetSrcTid(M_TID_GM);
                    pComMsg->SetDstTid(M_TID_EMSAGENTTX);	
                    char *p = (char*)pComMsg->GetDataPtr();
                    p[0] = (gUbloxGpsAllData.Year>>8)&0xff;
                    p[1] = gUbloxGpsAllData.Year&0xff;
                    p[2] = gUbloxGpsAllData.Month;
                    p[3] = gUbloxGpsAllData.Day;
                    p[4] = gUbloxGpsAllData.Hour;
                    p[5] = gUbloxGpsAllData.Minute;
                    p[6] = gUbloxGpsAllData.Second;
                    memset(&p[7],0,4);//没有这个数值
                    if ( false == CComEntity::PostEntityMessage(pComMsg) )
                    {
                        pComMsg->Destroy();				
                    }
                }
                return true;
            }
        }
        
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gpsReturnHlrReq, UBLOX return error.");
    }
    return false;
    #endif
}
bool  CTaskGPS::gpsHeartbeatTimeOut()
{
    if (false == m_bInitSIO)
        {
        if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] Initialize GPS!");
        m_bInitSIO = gpsInitSIO();  
        if (false == m_bInitSIO)
            {
            if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR...Initialize GPS FAIL!");
            return false;
            }
        }
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;

    //@@Ha
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[1];
    // Enable the RAIM algorithm if the alarm limit is non-zero
    cmd[cmdlen++] = 0;

    char response[GPS_MAX_RESP_LENGTH] = {0};
    status = gpsExecCommand(cmd, cmdlen, 3, response);
    if (OK == status)
        {
        OAM_LOGSTR(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS module communication OK.");
        T_GpsAllData *pGpsData = (T_GpsAllData*)response;
        gBtsVisbleSatcnt = pGpsData->VisibleSatellites;
        gBtstrackSatcnt  = pGpsData->TrackedSatellites;
        if (pGpsData->TrackedSatellites >= NvRamDataAddr->GpsDataCfgEle.SatelliteCnt)
        {      
            //GPS锁定.同步时间和位置信息
            if(true == bGpsInitTime)
                gpsSynchronize(*pGpsData);           
            
#ifdef WBBU_CODE
            bGpsStatus_RF = 0;
#endif
            if ((false == bGpsStatus) && (true == gpsConfigL1(BTS_SYNC_SRC_GPS, false)))
            {
                m_gpsWorkDownNum=0;
	         m_gpsRcd = GPS_OPR_GPS_OK;/*已经同步过了*/
                bGpsStatus = true;  //L1配置成功才回复状态
                //告警恢复
                //if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                {
                    OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS locked!");
                    AlarmReport(ALM_FLAG_CLEAR, ALM_ENT_GPS, ALM_ENT_INDEX0,
                                  ALM_ID_GPS_LOST, ALM_CLASS_CRITICAL, STR_CLEAR_ALARM);
                    if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                        AlarmReport(ALM_FLAG_CLEAR, ALM_ENT_GPS, ALM_ENT_INDEX0,
                                  ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_CLEAR_ALARM);
                }
            }
            if(false == bGpsInitTime)
            {
                if (OK != gpsSetGpsTime())
                {
                    if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! Init GPS time from NVRAM failed.");
                    
                }
                else
                {
                    bGpsInitTime = true;
                    m_RtcUpdateInterval = RTC_UPDATE_PERIOD/GPS_HEARTBEAT_PERIOD;
                    status = gpsExecCommand(cmd, cmdlen, 3, response);
                    if (OK == status)
                    {
                        pGpsData = (T_GpsAllData*)response;
                        if (pGpsData->TrackedSatellites >= NvRamDataAddr->GpsDataCfgEle.SatelliteCnt)
                            gpsSynchronize(*pGpsData);  
                    }
                }
            }
            return true;
            }
        }
    else
        {
        //GPS通信错误；
        if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS module communication fail, please check GPS hardware.");
        }
     /*如果已经同步过,并且连续6次失步,则关射频*/
     if((false == bGpsStatus)&&(m_gpsRcd==GPS_OPR_GPS_OK)&&(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc))
     {
         m_gpsWorkDownNum++;
	  if(m_gpsWorkDownNum>=gpscloseRFtimer)
  	  {
  	      if(m_gpsRcd!=GPS_OPR_SEND_CLOSE_RF)
  	      {
  	          OAM_LOGSTR1(LOG_WARN, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS can't sync after %dmin, closed RF!!", gpscloseRFtimer/2);
	  	   gpsConfigL1(BTS_SYNC_SRC_BTS, true);
		   gpsConfigRFMask();
		   m_gpsRcd = GPS_OPR_SEND_CLOSE_RF;
#ifdef WBBU_CODE
		   bGpsStatus_RF = 1;
                //zengjihan 20120814 for GPSSYNC            
                if(0 < bspGetslaveBTSID())
                {
                    LOG(LOG_DEBUG3, LOGNO(GPS,0), "//GPS锁定.主失步时间信息");
                    //GPS锁定.主失步时间信息
                    CComMessage *pComMsg = new(this, 16)CComMessage;
                    if(NULL != pComMsg)
                    {
                        pComMsg->SetMessageId(M_OAM_GPS_SYNC_NOTIFY);
                        pComMsg->SetSrcTid(M_TID_GM);
                        pComMsg->SetDstTid(M_TID_EMSAGENTTX);
                        pComMsg->SetBTS(bspGetslaveBTSID());
                        char *p = (char*)pComMsg->GetDataPtr();
                        p[0] = 0;
                        p[1] = 0;
                        p[2] = 0;
                        p[3] = 0;
                        p[4] = 0;
                        p[5] = 0;
                        p[6] = 0;
                        memset(&p[7],0,4);
                        p[11] = 0xaa;/**表示失步*/
                        if ( false == CComEntity::PostEntityMessage(pComMsg) )
                        {
                            pComMsg->Destroy();
                        }
                    }
                }            
#endif
	      }	  	
	      m_gpsWorkDownNum=0;
  	   }
     }
     
	 	
	
    //status = ERROR 或者 锁定的卫星数太少；都认为是GPS失步
    if ((true == bGpsStatus)&&(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc))
        {
        //GPS告警
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] WARNING...GPS lose lock, maybe GPS link error, or GPS module error, or Config L1 General setting error.");
        AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                      ALM_ID_GPS_LOST, ALM_CLASS_CRITICAL, STR_GPS_LOST);
        //gpsConfigL1(BTS_SYNC_SRC_BTS);
        }
    bGpsStatus = false;

    return false;
}

bool CTaskGPS::gpsSynchronize(T_GpsAllData &GpsAllData)
{
#if 0
    //send synchronize GPSFn to L2
    CGPSSynchReq CGPSReq;
    if(CGPSReq.CreateMessage(*this))
        {
        CGPSReq.SetTransactionId(OAM_DEFAUIT_TRANSID);
        CGPSReq.SetDstTid(M_TID_L2MAIN);
        CGPSReq.SetHour(GpsAllData.Hour);
        CGPSReq.SetMinute(GpsAllData.Minute);
        CGPSReq.SetSecond(GpsAllData.Second);
        CGPSReq.SetMinSecond(GpsAllData.MinSec);
        if ( !CGPSReq.Post() )
            {
            OAM_LOGSTR(LOG_WARN, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] Post GPS Synchronize failed."); 
            CGPSReq.DeleteMessage();
            }
        }
#endif
    bool bSetRTC = false;
    if (false == bGpsStatus)
        {
        //重新锁定,则写RTC.
        bSetRTC             = true;
        m_RtcUpdateInterval = 0;
        }
    else
        {
        if (++m_RtcUpdateInterval >= RTC_UPDATE_PERIOD/GPS_HEARTBEAT_PERIOD)
            {
            //定期写RTC.
            bSetRTC             = true;
            m_RtcUpdateInterval = 0;
            }
        }
    T_TimeDate timeDate1;            
    timeDate1 = bspGetDateTime();
    if(timeDate1.year!=GpsAllData.Year)//如果gps时间和本地的年不一致，立即修改
    {
        bSetRTC = true;
        m_RtcUpdateInterval = 0;
    }
    if ( true == bSetRTC )
        {
        //设置bts系统时间
        OAM_LOGSTR3(LOG_DEBUG2, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] set RTC Date %d/%d/%d", GpsAllData.Year, GpsAllData.Month, GpsAllData.Day); 
        OAM_LOGSTR3(LOG_DEBUG2, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] set RTC time %d:%d:%d", GpsAllData.Hour, GpsAllData.Minute, GpsAllData.Second); 
        OAM_LOGSTR2(LOG_DEBUG2, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GMT offset: %d hour, %d minute", GpsAllData.HourGMTOffset, GpsAllData.MinuteGMTOffset); 

        //zengjihan 20120801 for GPSSYNC
        #ifdef WBBU_CODE
        if(0 < bspGetslaveBTSID())
        {
            LOG(LOG_DEBUG3, LOGNO(GPS,0), "//GPS锁定.主同步时间信息");
            //GPS锁定.主同步时间信息
            CComMessage *pComMsg = new(this, 16)CComMessage;
            if(NULL != pComMsg)
            {
                pComMsg->SetMessageId(M_OAM_GPS_SYNC_NOTIFY);
                pComMsg->SetSrcTid(M_TID_GM);
                pComMsg->SetDstTid(M_TID_EMSAGENTTX);
                pComMsg->SetBTS(bspGetslaveBTSID());
                char *p = (char*)pComMsg->GetDataPtr();
                p[0] = (GpsAllData.Year>>8)&0xff;
                p[1] = GpsAllData.Year&0xff;
                p[2] = GpsAllData.Month;
                p[3] = GpsAllData.Day;
                p[4] = GpsAllData.Hour;
                p[5] = GpsAllData.Minute;
                p[6] = GpsAllData.Second;
                memcpy(&p[7],&GpsAllData.MinSec,4);
                p[11] = 0x55;/**表示同步*/
                if ( false == CComEntity::PostEntityMessage(pComMsg) )
                {
                    pComMsg->Destroy();
                }
            }
        }
        #endif
	if(BTS_SYNC_SRC_485!=NvRamDataAddr->L1GenCfgEle.SyncSrc)
	{
	       #ifndef WBBU_CODE
	        rtcDateSet(GpsAllData.Year, GpsAllData.Month, GpsAllData.Day, 0);
	        rtcTimeSet(GpsAllData.Hour, GpsAllData.Minute, GpsAllData.Second);
	       #else
	        if((Hard_VerSion<5)||(Hard_VerSion>15))
	        	{
	       		 rtcDateSet(GpsAllData.Year, GpsAllData.Month, GpsAllData.Day, 0);
	        		rtcTimeSet(GpsAllData.Hour, GpsAllData.Minute, GpsAllData.Second);
	        	}
	              else
	              {
	                    char pbuffer[7];
	                    pbuffer[0] = GpsAllData.Second;//&0x7f;  /******s********/
	                    pbuffer[1] = GpsAllData.Minute;//&0x7f;  /******min*******/
	                    pbuffer[2] = GpsAllData.Hour;//&0x3f;  /*********hour*******/
	                    pbuffer[3] = GpsAllData.Day;//&0x3f; /*******day********/
	                    pbuffer[4] = 5;//&0x7; /****week day*****/
	                    pbuffer[5] = GpsAllData.Month;//&0x1f; /*******month*******/
	                    pbuffer[6] = (GpsAllData.Year-2000);//&0xff; /**year***/
	                    bspSetTime(pbuffer);
	                    
	              }
	           #endif
	        ////set system time;
	            {
	            struct tm time_s;
	            T_TimeDate timeDate;
	            struct timespec timeSpec;
	            unsigned int secCount;
	           #ifndef WBBU_CODE
	            timeDate = bspGetDateTime();
	            time_s.tm_sec = timeDate.second;
	            time_s.tm_min = timeDate.minute;
	            time_s.tm_hour = timeDate.hour;

	            time_s.tm_mday = timeDate.day;
	            time_s.tm_mon  = timeDate.month - 1;
	            time_s.tm_year = timeDate.year - 1900;
	           #else
	            time_s.tm_sec = GpsAllData.Second;
	            time_s.tm_min = GpsAllData.Minute;
	            time_s.tm_hour = GpsAllData.Hour;

	            time_s.tm_mday = GpsAllData.Day;
	            time_s.tm_mon  = GpsAllData.Month - 1;
	           
	            time_s.tm_year = GpsAllData.Year - 1900;
	           #endif
	            time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

	            secCount = mktime(&time_s);
	            timeSpec.tv_sec = secCount;
	            timeSpec.tv_nsec = 0;
	            clock_settime(CLOCK_REALTIME, &timeSpec);
	            }
		}

        }

    if (GpsAllData.Latitude && GpsAllData.Longitude && GpsAllData.GPSHigh)
        {
        if ((GpsAllData.Latitude  != NvRamDataAddr->GpsDataCfgEle.Latitude)||
            (GpsAllData.Longitude != NvRamDataAddr->GpsDataCfgEle.Longitude)||
            (GpsAllData.GPSHigh   != NvRamDataAddr->GpsDataCfgEle.Height))
            {
            CTaskCfg::l3oambspNvRamWrite((char*)&(NvRamDataAddr->GpsDataCfgEle), (char*)&(GpsAllData.Latitude), 12);
            gpsLocationModifyToEMS(NvRamDataAddr->GpsDataCfgEle);
            }
        }

    return true;
}


bool CTaskGPS::gpsLocationModifyToEMS(T_GpsDataCfgEle &GpsDataCfgEle)
{
    CCfgGpsLocNotify Notify;
    if (false == Notify.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! System encounter exception, createMessage failed.");
        return false;
        }
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetDstTid(M_TID_EMSAGENTTX);
    Notify.SetLongitude(GpsDataCfgEle.Longitude);
    Notify.SetLatitude(GpsDataCfgEle.Latitude);
    Notify.SetHeight(GpsDataCfgEle.Height);
    if(true != Notify.Post())
        {
        OAM_LOGSTR1(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! send GPS Location Notify to EMS fail!", Notify.GetMessageId());
        Notify.DeleteMessage();
        }

    OAM_LOGSTR3(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] send GPS Location Notify to EMS!lat[%d] long[%d] height[%d]", GpsDataCfgEle.Latitude, GpsDataCfgEle.Longitude, GpsDataCfgEle.Height); 
    return true;
}

bool CTaskGPS::gpsConfigL1(UINT16 SyncSrc, BOOL closeRF)
{
    UINT16 usRFMask;
    T_L1GenCfgEle *pCfgEle = (T_L1GenCfgEle *)&(NvRamDataAddr->L1GenCfgEle);
    CCfgL1GenDataReq CfgReq;
    if (false == CfgReq.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! System encounter exception, createMessage failed.");
        return false;
        }
    CfgReq.SetMessageId(M_L3_L2_L1_GENERAL_SETTING_REQ);
    CfgReq.SetDstTid(M_TID_L2MAIN);  
    CfgReq.SetSrcTid(M_TID_GM);
    CfgReq.SetSyncSrc(SyncSrc);
    CfgReq.SetGPSOffset(pCfgEle->GpsOffset);
    if(closeRF==false)//open
    	{
    	  if(m_gpsRcd!=GPS_OPR_SEND_CLOSE_RF)//其它告警关闭的
	         usRFMask = CTaskAlm::GetInstance()->getCurrentRFMask();
	  else //是gps3分钟失败关闭的
	  	//当前的天线掩码 = (配置值) 
               usRFMask  = NvRamDataAddr->L1GenCfgEle.AntennaMask;
    	}
     else/*如果是关射频,则直接设置为0*/
	  usRFMask = 0;
#ifdef WBBU_CODE
	 usRFMask= usRFMask&(~Calibration_Antenna);
#endif
    CfgReq.SetAntennaMask(usRFMask);

    //构造配置失败消息       
    CL3OamCommonRsp FailNotify;
    if (false == FailNotify.CreateMessage(*this))
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! System encounter exception, createMessage failed.");
        CfgReq.DeleteMessage();
        return false;
    }
    FailNotify.SetDstTid(M_TID_GM);
    FailNotify.SetMessageId(SEND_L1_MSG_TIMEOUT);
    //创建配置数据transaction,发配置请求消息
    CTransaction* pCfgTransaction = CreateTransact(CfgReq, FailNotify, OAM_REQ_RESEND_CNT3, OAM_REQ_RSP_PERIOD);
    if (NULL == pCfgTransaction)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! System encounter exception, create transaction failed.");
        CfgReq.DeleteMessage();
        FailNotify.DeleteMessage();
        return false;
    }
    UINT16 TransID = pCfgTransaction->GetId();
    CfgReq.SetTransactionId(TransID);
    FailNotify.SetTransactionId(TransID);
    FailNotify.SetResult((UINT16)L3CM_ERROR_TIMER_OUT);
    if(false == pCfgTransaction->BeginTransact())
    {
        //OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! System encounter exception, Begin transaction failed.");
        pCfgTransaction->EndTransact();
        delete pCfgTransaction;
        return false;
    }
#ifdef WBBU_CODE
  WrruRFC(usRFMask,1,9);//wangwenhua add 20091116
#endif
    return true;
}

//--------------------------------------------------------------------------
//  Method: gpsGetCommandCode
//
//  Description: Searches the command table for the specified command.
//
//  Inputs: Pointer to first byte of command in buffer.
//
//  Return: Command code found in table, or CMD_UNKNOWN.
//
//--------------------------------------------------------------------------
GPS_COMMANDS CTaskGPS::gpsGetCommandCode (const char *cmd)
{
    int i;
    for (i = GPS_COMMANDS(0); i < CMD_UNKNOWN; i++)
    {
        if (cmd[0] == GPS_COMMAND_TABLE[i].code[0] &&
            cmd[1] == GPS_COMMAND_TABLE[i].code[1])
            break;
    }
    return ((GPS_COMMANDS) i);
}


void CTaskGPS::GPS_TRACE(const char *prefix, const char *msg, const int msglen, bool isM12)
{
    char line[200];

    line[0] = 0;

    sprintf(&line[strlen(line)], prefix);
    if(isM12)
    {
        for (int i=0; i<4 && i<msglen; i++)
        {
            sprintf (&line[strlen(line)], "%c", msg[i]);
        }
    
        for (int i = 4; i < msglen; i++) 
        {
            sprintf (&line[strlen(line)], "|%02x", (UINT8)msg[i]);
            if ( strlen(line) >= 100)
            {
                LOG_STR(LOG_DEBUG2, GPS_LOG_CODE, line);
                line[0]=  0;
            }
        }
    }
    else//ublox
    {
        for (int i = 0; i < msglen; i++) 
        {
            sprintf (&line[strlen(line)], " %.2x", (UINT8)msg[i]);
            if ( strlen(line) >= 100)
            {
                LOG_STR(LOG_DEBUG2, GPS_LOG_CODE, line);
                line[0]=  0;
            }
        }
    }
    LOG_STR(LOG_DEBUG2, GPS_LOG_CODE, line);

}



//--------------------------------------------------------------------------
//  Method: TimedRead
//
//  Description: Performs a read with a timeout.
//
//  Inputs: fd        - file descriptor from which to get data
//          buffer    - pointer to output buffer to receive the data
//          maxlength - maximum number of bytes to read from file
//          timeout   - maximum number of seconds to wait for data
//
//  Return: Number of bytes read, or zero if timeout or error occurs.
//
//--------------------------------------------------------------------------
int CTaskGPS::TimedRead (int fd, char *buffer, const int maxlength, int timeout)
{
    int count, tcount;
    fd_set readFds;
    struct timeval timer = { timeout, 0} ;

    // Initialize the file descriptor set for this one file
    FD_ZERO (&readFds);
    FD_SET (fd, &readFds);

    // Fill the data buffer
    for (count = 0; count < maxlength; )
    {
        // Wait for the data or a timeout, which ever comes first
        tcount = select (fd+1, &readFds, NULL, NULL, &timer);

        // Quit if a timeout occurred or there's no data for this file descr.
        if (tcount <= 0 || !(FD_ISSET (fd, &readFds)))
        {
            if (tcount == 0) {
                LOG_STR1 (LOG_DEBUG1, GPS_LOG_CODE, "TimedRead timed out w/count %d ", count);
            } else {
                LOG_STR1 (LOG_DEBUG1, GPS_LOG_CODE, "TimedRead had error 0x%x", errno);
            }
            break;
        }

        // Get the data that is available
        count += read (fd, &buffer[count], maxlength);
        //ublox get utc
        if(gpsModel == GPS_MODEL_UBLOX)
        {
            if((count==28)&&(buffer[2]==0x01)&&(buffer[3]==0x21)&&(buffer[0]==0xb5)&&(buffer[1]==0x62))
            {
                break;
            }
        }
        
    }

    return count;
}

//--------------------------------------------------------------------------
//  Method: SendMsg
//
//  Description: This method will construct the cmd and send to interrupt
//               transmit routine to send the cmd to the hardware.
//
//  Inputs: msg     - message to send, excluding header, trailer and checksum,
//          length  - number of bytes in message,
//          timeout - number of seconds to wait for a response (optional).
//
//  Return: OK if successful, otherwise ERROR.
//
//--------------------------------------------------------------------------
int CTaskGPS::gpsExecCommand(const char *msg, const UINT8 length, const int timeout, char *pOutPutResponse)
{
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
    GPS_COMMANDS cmdcode;

    char response[GPS_MAX_RESP_LENGTH];
    int  resplen;
    GPS_COMMANDS respcode;

    int i, count;
    unsigned char checksum;
    bool done;
    int retry  = GPS_SEND_RECEIVE_RETRY_COUNT;
    int status = OK;

    // Add a header to the command buffer
    cmd[cmdlen++] = '@';
    cmd[cmdlen++] = '@';

    // Copy the message to the command buffer and calculate the checksum
    checksum = 0;
    for (i = 0; i < length; i++)
    {
        checksum ^= cmd[cmdlen++] = *(msg+i);
    }

    // Fill in the checksum
    cmd[cmdlen++] = checksum;

    // Add <CR><LF> to complete the command
    cmd[cmdlen++] = CR;
    cmd[cmdlen++] = LF;

    // Validate the command
    cmdcode = gpsGetCommandCode (msg);

    if (cmdcode == CMD_UNKNOWN)
    {
        if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
            OAM_LOGSTR(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! GPS internal error: unknown command sent"); 
        return ERROR;
    }

    if (cmdlen != GPS_COMMAND_TABLE[cmdcode].cmd_length)
    {
        OAM_LOGSTR(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR!!! GPS try to send msg with invalid length\n"); 
        return ERROR;
    }

    // Take exclusive access of the GPS device
    semTake (m_gpsSemId, WAIT_FOREVER);

    // Take exclusive access of the GPS device
    // Send a message and get it's response (may need to re-send)
    done = false;
    do {
        status = OK;

        // Send the command to the GPS
        GPS_TRACE ("Writing: ", cmd, cmdlen, true);
        count = write(m_gpssfd, cmd, cmdlen);

        // Wait for the response
        while (!done)
        {
            // Look for the beginning of a message (ie, "@@...")
            for (i = 0; i < GPS_HEADER_LENGTH; (response[i] == '@') ? i++ : i=0)
            {
                // Read the next character on the serial port
                count = TimedRead (m_gpssfd, &response[i], 1,
                            timeout ? timeout : GPS_READ_DATA_TIMEOUT_PERIOD);

                // If the read timed-out and no data was read, then quit
                if (count == 0) {
                    status = ERR_GPS_TIMED_OUT;
                    break;
                }

                // (debug output)
                GPS_TRACE ("Read: ", response, i+1, true);
            }
            if (status == ERR_GPS_TIMED_OUT) break;
            count = i;

            // Read the response command code
            count += TimedRead (m_gpssfd, &response[count],
                                GPS_COMMAND_LENGTH, 1);
            GPS_TRACE ("Read: ", response, count, true);

            if (count < GPS_HEADER_LENGTH + GPS_COMMAND_LENGTH) {
                status = ERR_GPS_TIMED_OUT;
                break;
            }

            // Get the rest of the message from the GPS
            respcode = gpsGetCommandCode (&response[GPS_HEADER_LENGTH]);
            if (respcode == CMD_UNKNOWN)
                continue;
            resplen = GPS_COMMAND_TABLE[respcode].resp_length;

            count += TimedRead (m_gpssfd, &response[count], resplen-count, 1);
            GPS_TRACE ("Read: ", response, count, true);

            if (count < resplen) {
                status = ERR_GPS_TIMED_OUT;
                break;
            }

            // Parse the response message into this GPS object's data members
//            status = ProcessMsg (response, count);
//            if (status == ERROR)
//                continue;

            // We're done if this is the response to the command     
            if (respcode == cmdcode)
                {
                done   = true;
                status = OK;
                if (NULL != pOutPutResponse)
                    {
                    memcpy(pOutPutResponse, response, resplen);
                    }
                }
        } // !done getting response

    // Send the command again if we timed-out
    } while ((!done) && (status == ERR_GPS_TIMED_OUT) && retry--);

    // Release control of the GPS device
    semGive (m_gpsSemId);

    // If the read timed-out, report it and return an error.
    // Return the completion status
    return (status);
}
void CTaskGPS::gpsConfigRFMask(UINT8 flag)
{
	CComMessage* pComMsg = new (this, 2) CComMessage;
	if (pComMsg==NULL)
	{
	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in gpsConfigRFMask.");
	return ;
	}
	pComMsg->SetDstTid(M_TID_FM);
	pComMsg->SetSrcTid(M_TID_GM);    
	pComMsg->SetMessageId(M_GPS_ALM_CHG_RF_MASK_REQ);
	#ifdef WBBU_CODE
    UINT8 *puc = (UINT8*)pComMsg->GetDataPtr();
    *puc = flag;
    #endif
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
}
int CTaskGPS::gpsSetPositionControl(int mode)
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
 //   int i;

    OAM_LOGSTR1(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] Position hold mode: %d", mode); 
    OAM_LOGSTR (LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, " mode 0: no hold; 1: enable position hold; 2:enable altitude hold; 3:autosite survey\n"); 
    if (mode > 3)
        {
        return ERROR;
        }
    //@@Gd
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_HOLD_MODE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_HOLD_MODE].code[1];
    
    cmd[cmdlen++] = mode;
    status = gpsExecCommand(cmd, cmdlen, 3);
    return status;
}
#ifdef WBBU_CODE
/*
判断GPS型号,先使用m12命令格式判断，如果返回不对认为不是m12
*/
int CTaskGPS::gpsJudgeModel()
{
    int status;    
    int cmdlen = 0;   
   #if 0
    gps_Close_GGA();
    ::taskDelay( 10 );
    gps_Close_GLL();
    ::taskDelay( 10 );
    gps_Close_GSA();
    ::taskDelay( 10 );
    gps_Close_GSV();
    ::taskDelay( 10 );
    gps_Close_RMC();
    ::taskDelay( 10 );
    gps_Close_VTG();
    ::taskDelay( 10 );
    gps_Close_ZDA();
    ::taskDelay( 10 );
    #endif
    status = gps_NAV_TimeUTC();
    if(status == OK)
    {            
        return GPS_MODEL_UBLOX;
    }
    return GPS_MODEL_M12;
    
}
//配置10ms
STATUS CTaskGPS::gps_Cfg_Tp1() 
{   
    
    char data[] = {
    0xb5, 0x62, 0x06, 0x31, 0x20, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 
    0xe8, 0x03, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
    0xb8, 0x0b, 0x00, 0x00, 0xb8, 0x0b, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xf7, 0x00, 
    0x00, 0x00, 0x00, 0x27};
    
    /*char data[45] = {0xb5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0f, 0x00, 0x40, 
    0x42, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa0, 0x86, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xf7, 0x00, 0x00, 0x00, 0xc9, 0x97 };*/
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;
    
    status = gpsExecCommand_UBlox(data, 40, dataRsp, 10);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Cfg_Tp1, return failure!"); 
        return false;
    }
    else if((dataRsp[2]==5)&&(dataRsp[3]==1))//ACK
    {
        return true;
    }
    else
        return false;
    
}		

//cfg time pulse 2,配置1s
STATUS CTaskGPS::gps_Cfg_Tp2() 
{
   
    char data[] ={
    0xb5, 0x62, 0x06, 0x31, 0x20, 0x00, 
    0x01, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0f, 0x00, 0x40, 0x42, 0x0f, 0x00, 
    0x4c, 0x1d, 0x00, 0x00, 0x4c, 0x1d, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 0xf7, 0x00, 0x00, 0x00, 0xcb,
    0x5f};
   
    /*char data[]={0xB5, 0x62, 0x06, 0x07, 0x14, 0x00, 0x40, 0x42, 0x0F, 0x00, 0x58, 0x1B, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
    0x34, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5E, 0xBE};*/
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;
    status = gpsExecCommand_UBlox(data, 40, dataRsp, 10);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Cfg_Tp2, return failure!, try again"); 
        return false;
    }
    else if((dataRsp[2]==5)&&(dataRsp[3]==1))//ACK
    {
        return true;
    }
    else
        return false;

}
STATUS CTaskGPS::gps_Cfg_ANT() 
{    
    char data[]={ 0xB5, 0x62, 0x06, 0x13, 0x04, 0x00, 0x1F, 0x00, 0x0F, 0x64, 0xAF, 0xCB};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;
    status = gpsExecCommand_UBlox(data, 12, dataRsp, 10);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Cfg_ANT, return failure!, try again");         
    }
    return status;

}
int CTaskGPS::gpsExecCommand_UBlox(char *msg, const UINT8 length, char *pOutPutResponse, UINT8 resplen)
{  
    char response[GPS_MAX_RESP_LENGTH];       
    int i, m, len, count;    
    bool done;
    int retry  = GPS_SEND_RECEIVE_RETRY_COUNT;
    int status = OK;
   
    // Take exclusive access of the GPS device
    semTake (m_gpsSemId, WAIT_FOREVER);

    // Take exclusive access of the GPS device
    // Send a message and get it's response (may need to re-send)
    done = false;
    do {
        status = OK;

        // Send the command to the GPS
        GPS_TRACE ("Writing: ", msg, length, false);
        count = write(m_gpssfd, msg, length);
        if(count!=length)
        {
            printf("gps writing len error!..............count=%d, len=%d.\n", count, length);
            status = ERR_GPS_TIMED_OUT;
            break;
        }        
        if(resplen==0)//不需要等返回值
            done = true;

        // Wait for the response
        while (!done)
        {
           count = TimedRead (m_gpssfd, &response[0], GPS_MAX_RESP_LENGTH, 3);
           GPS_TRACE ("Reading: ", response, count, false);
           if (count == 0) 
           {
               status = ERR_GPS_TIMED_OUT;
               break;
           }
           if(isFirstMsg)//第一条判断是否为ublox消息
            {
                for(i=0; i<count; i++)
                {
                    if(((response[i]=='$')&&(response[i+1]=='G')&&(response[i+2]=='P'))\
                        ||((response[i]==0xb5)&&(response[i+1]==0x62)))//is ublox
                    {                        
                        break;
                    }                   
                }                
                if(i<count)//find it
                {
                    status = OK;
                    done   = true;
                    break;
                }
                else
                {
                    status = ERR_GPS_TIMED_OUT;
                    done   = true;
                    break;
                }
            }
           //GPS_TRACE ("Read: ", response, count, false);
            for(i=0; i<count; i++)//找到应答命令头
            {
                if(response[i]==0xb5)
                    break;
            }
            if((i==count)||((count-i)<= 8))//没有找到或长度不对，丢弃
            {
               status = ERR_GPS_TIMED_OUT;
               break;
            } 
            
            for(m=i; m<4; m++)
            {
                if(response[m]!=msg[m])
                    break;
            }
            if ((m==4)||((response[i]==0xb5)&&(response[i+1]==0x62)&&(response[i+2]==0x05)&&(response[i+3]==0x01)))//是查询应答
            {
                len = response[5+i] * 0x100 + response[4+i];
                if((count-i)<len)//没有取到全部消息，丢弃
                {
                    status = ERR_GPS_TIMED_OUT;
                    break;
                }
                done   = true;
                status = OK;
                if (NULL != pOutPutResponse)
                {                    
                    memcpy(pOutPutResponse, &response[i], len+8);
                }                
            }
            else
            {
                status = ERR_GPS_TIMED_OUT;    
                break;
            }           
            // Parse the response message into this GPS object's data members
//            status = ProcessMsg (response, count);
//            if (status == ERROR)
//                continue;

            // We're done if this is the response to the command     
            
        } // !done getting response

    // Send the command again if we timed-out
    } while ((!done) && (status == ERR_GPS_TIMED_OUT) && retry--);

    // Release control of the GPS device
    semGive (m_gpsSemId);

    // If the read timed-out, report it and return an error.
    // Return the completion status
    return (status);
}
/*
查询时间
*/
STATUS CTaskGPS::gps_NAV_TimeUTC()
{
    char data[] = {0xB5, 0x62, 0x01, 0x21, 0, 0, 0x22, 0x67};
    char dataRsp[GPS_MAX_RESP_LENGTH];
    struct tm mytime;
    struct tm *pmytime;
    time_t  t_tick;
    int status;

    memset(dataRsp, 0, GPS_MAX_RESP_LENGTH);
    status = gpsExecCommand_UBlox(data, 8, dataRsp, 20+6);
    if(isFirstMsg)
    {
        isFirstMsg = false;
        return status;
    }
    if(status==OK)
    {        
        //解析时间，加时区
        char *pchar = &dataRsp[6];//数据区
        if(pchar[19]&0x07 == 0x07)//utc valid
        {
            gUbloxGpsAllData.Year = pchar[13]*0x100+pchar[12];//小端模式
            gUbloxGpsAllData.Month = pchar[14];
            gUbloxGpsAllData.Day = pchar[15];
            gUbloxGpsAllData.Hour = pchar[16];
            gUbloxGpsAllData.Minute = pchar[17];
            gUbloxGpsAllData.Second = pchar[18];
            
            SINT32 gpsOffset = NvRamDataAddr->GpsDataCfgEle.GMTOffset;
            
            if(gpsOffset!=0)
            {
                   memset(&mytime, 0, sizeof(mytime));
                   mytime.tm_year =  gUbloxGpsAllData.Year - 1900;//from 1900
                   mytime.tm_mon  = gUbloxGpsAllData.Month - 1;//0-11
                   mytime.tm_mday = gUbloxGpsAllData.Day;
                   mytime.tm_hour = gUbloxGpsAllData.Hour ;
                   mytime.tm_min = gUbloxGpsAllData.Minute;
                   t_tick = mktime(&mytime) + gpsOffset*60;
                   pmytime = localtime(&t_tick);
                   gUbloxGpsAllData.Year = pmytime->tm_year +1900;
                   gUbloxGpsAllData.Month = pmytime->tm_mon  +1;
                   gUbloxGpsAllData.Day   = pmytime->tm_mday ;
                   gUbloxGpsAllData.Hour = pmytime->tm_hour ;
                   gUbloxGpsAllData.Minute = pmytime->tm_min ;            
            }            
        }
        else
        {
            OAM_LOGSTR(LOG_DEBUG2, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_NAV_TimeUTC, msg is invalid"); 
        }
        
    }
    
    return status;   
    
}
/*
查询经纬度
*/
STATUS CTaskGPS::gps_NAV_PosLLH()
{
    char data[] = {0xB5, 0x62, 0x01, 0x02, 0x00, 0x00, 0x03, 0x0A};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;
    UINT32 lon, lat, hei, deg, min, sec;

    status = gpsExecCommand_UBlox(data, 8, dataRsp, 28+6);
    if(status==OK)
    {        
        char *pchar = &dataRsp[6];//数据区
        //经纬度返回值是1e-7单位，转换为度，分，再转为上报值
        lon = pchar[7]*0x1000000 + pchar[6]*0x10000 + pchar[5]*0x100 + pchar[4];
        deg = lon / 10000000;
        min = (lon % 10000000) * 60 / 10000000;
        sec = ((lon % 10000000) * 3600 / 10000000)%60;        
        gUbloxGpsAllData.Longitude = (deg * 3600 +min * 60 + sec)*1000;
        
        lat = pchar[11]*0x1000000 + pchar[10]*0x10000 + pchar[9]*0x100 + pchar[8];  
        deg = lat / 10000000;
        min = (lat % 10000000) * 60 / 10000000;
        sec = ((lat % 10000000) * 3600 / 10000000)%60;        
        gUbloxGpsAllData.Latitude = (deg * 3600 +min* 60 + sec)*1000;
       
        //高度单位为mm,转为cm
        hei = (pchar[19]*0x1000000 + pchar[18]*0x10000 + pchar[17]*0x100 + pchar[16]);        
         gUbloxGpsAllData.GPSHigh = hei/10;//mm->cm    
    }
    return status; 
}
/*
查询GPS硬件信息
*/
STATUS CTaskGPS::gps_MON_HW()
{
    char data[] = {0xB5, 0x62, 0x0A, 0x09, 0x00, 0x00, 0x13, 0x43 };
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 8, dataRsp, 68+6);
    if(status==OK)
    {
        char *pInfo = &dataRsp[6];
        gUbloxSData.antennaInfo.aStatus = pInfo[20];
        gUbloxSData.antennaInfo.aPower = pInfo[21];        
    }
    return status; 
}
/*
查询卫星状况
*/
STATUS CTaskGPS::gps_NAV_SVInfo()
{
    char data[10] = {0xB5, 0x62, 0x01, 0x30, 0x00, 0x00, 0x31, 0x94};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;
    char numCh;
    int visiableS, trackS;

    visiableS = 0;
    trackS = 0;
    status = gpsExecCommand_UBlox(data, 8, dataRsp, 200);
    if(status==OK)
    {        
        UINT8*pInfo = (UINT8*)&dataRsp[6];//小端模式
        int len = 0;
        
        gUbloxSData.svInfo.iTow = pInfo[len+3]*0x1000000 + pInfo[len+2]*0x10000 + \
            pInfo[len+1]*0x100 +pInfo[len];
        len += 4;
        gUbloxSData.svInfo.numCh = pInfo[len];
        len += 1;
        gUbloxSData.svInfo.globalFlags = pInfo[len];
        len += 1;
        gUbloxSData.svInfo.res2 = pInfo[len+1]*0x100 +pInfo[len];
        len += 2;
        for(int i=0; i<gUbloxSData.svInfo.numCh; i++)
        {
            gUbloxSData.svInfo.sInfo[i].chn = pInfo[len];
            len += 1;
            gUbloxSData.svInfo.sInfo[i].svid = pInfo[len];
            len += 1;
            gUbloxSData.svInfo.sInfo[i].flags = pInfo[len];
            len += 1;
            gUbloxSData.svInfo.sInfo[i].quality = pInfo[len];
            len += 1;
            gUbloxSData.svInfo.sInfo[i].cno_db = pInfo[len];
            len += 1;
            gUbloxSData.svInfo.sInfo[i].elev = pInfo[len];
            len += 1;
            gUbloxSData.svInfo.sInfo[i].azim = pInfo[len] * 0x100 + pInfo[len] ;
            len += 2;
            gUbloxSData.svInfo.sInfo[i].prRes = pInfo[len+3]*0x1000000 + pInfo[len+2]*0x10000 + \
            pInfo[len+1]*0x100 +pInfo[len];
            len += 4;
            if(gUbloxSData.svInfo.sInfo[i].quality>=3)//Signal detected but unusable, track
            {
                visiableS++;
                if(gUbloxSData.svInfo.sInfo[i].quality>=4)//lock
                {
                    trackS++;
                }
            }
            
        }
        gUbloxGpsAllData.VisibleSatellites = visiableS;
        gUbloxGpsAllData.TrackedSatellites = trackS;
    }
    return status; 
}

/*
*关闭NMEA的GGA命令
*/
STATUS CTaskGPS::gps_Close_GGA()
{
    char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x23};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Close_GGA, return failure!");         
    }
    return status; 
}

/*
*关闭NMEA的GLL命令
*/
STATUS CTaskGPS::gps_Close_GLL()
{
    char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Close_GLL, return failure!");         
    }
    return status; 
}

/*
*关闭NMEA的GSA命令
*/
STATUS CTaskGPS::gps_Close_GSA()
{
    char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31 };
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Close_GSAL, return failure!");         
    }
    return status; 
}

/*
*关闭NMEA的GSV命令
*/
STATUS CTaskGPS::gps_Close_GSV()
{
    char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Close_GSV, return failure!");         
    }
    return status; 
}

/*
*关闭NMEA的RMC命令
*/
STATUS CTaskGPS::gps_Close_RMC()
{
    char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Close_RMC, return failure!");         
    }
    return status; 
}

/*
*关闭NMEA的VTG命令
*/
STATUS CTaskGPS::gps_Close_VTG()
{
    char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
    if(status!=OK)//failure, try again
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Close_UTG, return failure!");         
    }
    return status; 
}

/*
*关闭NMEA的ZDA命令
*/
STATUS CTaskGPS::gps_Close_ZDA()
{
    char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5B};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 10);
    if(status!=OK)
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Close_ZDA, return failure!");         
    }
    return status; 
}
/*
*关闭SABS命令
*/
STATUS CTaskGPS::gps_Close_SABS()
{
    char data[] = {0xB5, 0x62, 0x06, 0x16, 0x08, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x97};
    char dataRsp[GPS_MAX_RESP_LENGTH];    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 10);
    if(status!=OK)//failure, try again
    {
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] gps_Close_SABS, return failure!");         
    }
    return status; 
}
/*
判断是否是ublox，如果是，则初始化:关闭NMEA格式输出，配置波特率等
*/
bool CTaskGPS::gpsInitSIO_UBlox()
{    
    bool status1;
    static int count = 0;
    if(count < 2)
    {  
        gps_Close_GGA();
        ::taskDelay( 5 );
        gps_Close_GLL();
        ::taskDelay( 5 );
        gps_Close_GSA();
        ::taskDelay( 5 );
        gps_Close_GSV();
        ::taskDelay( 5 );
        gps_Close_RMC();
        ::taskDelay( 5 );
        gps_Close_VTG();
        ::taskDelay( 5 );
        gps_Close_ZDA();
        ::taskDelay( 5 );
        gps_Close_SABS(); 
        ::taskDelay( 5 );
        count++;
    }
    status1 = gps_Cfg_Tp1();
    if(status1== false )
        return false;
    ::taskDelay( 10 );
    status1 = gps_Cfg_Tp2();
    if(status1== false )
        return false;   
    
    return true;        
}
/*
定时查询gps信息消息处理
*/
bool  CTaskGPS::gpsHeartbeatTimeOut_UBlox()
{  
    int status;    
    if (false == m_bInitSIO)
    {
        if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] Initialize GPS!");
        m_bInitSIO = gpsInitSIO_UBlox();  
        if (false == m_bInitSIO)
        {
            if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] ERROR...Initialize GPS FAIL!");
            return false;
        }
    }
    static int count = 0;
    if(count < 2)
    {
        gps_Close_GGA();
        ::taskDelay( 5 );
        gps_Close_GLL();
        ::taskDelay( 5 );
        gps_Close_GSA();
        ::taskDelay( 5 );
        gps_Close_GSV();
        ::taskDelay( 5 );
        gps_Close_RMC();
        ::taskDelay( 5 );
        gps_Close_VTG();
        ::taskDelay( 5 );
        gps_Close_ZDA();
        ::taskDelay( 5 );        
        gps_Cfg_ANT();
        ::taskDelay( 5 );  
        count++;
    }
    
    status = gps_NAV_SVInfo();
    if (OK == status)
    {
        OAM_LOGSTR(LOG_DEBUG3, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS module communication OK.");
        gps_NAV_PosLLH();
        gps_MON_HW();
        gps_NAV_TimeUTC();
        gBtsVisbleSatcnt = gUbloxGpsAllData.VisibleSatellites;
        gBtstrackSatcnt  = gUbloxGpsAllData.TrackedSatellites;
        if (gUbloxGpsAllData.TrackedSatellites >= NvRamDataAddr->GpsDataCfgEle.SatelliteCnt)        
        {  
            //GPS锁定.同步时间和位置信息         
            gpsSynchronize(gUbloxGpsAllData);
          
            bGpsStatus_RF = 0;
            if ((false == bGpsStatus) && (true == gpsConfigL1(BTS_SYNC_SRC_GPS, false)))
            {
                m_gpsWorkDownNum=0;
                m_gpsRcd = GPS_OPR_GPS_OK;/*已经同步过了*/
                bGpsStatus = true;  //L1配置成功才回复状态
                //告警恢复
                //if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                {
                    OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS locked!");
                    AlarmReport(ALM_FLAG_CLEAR, ALM_ENT_GPS, ALM_ENT_INDEX0,
                    ALM_ID_GPS_LOST, ALM_CLASS_CRITICAL, STR_CLEAR_ALARM);
                    if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                    AlarmReport(ALM_FLAG_CLEAR, ALM_ENT_GPS, ALM_ENT_INDEX0,
                    ALM_ID_GPS_CFG_ERROR, ALM_CLASS_CRITICAL, STR_CLEAR_ALARM);
                }
            }
            return true;
        }
    }
    else
    {
        //GPS通信错误；
        if(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc)
            OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS module communication fail, please check GPS hardware.");
    }
     /*如果已经同步过,并且连续6次失步,则关射频*/
     if((false == bGpsStatus)&&(m_gpsRcd==GPS_OPR_GPS_OK)&&(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc))
     {
         m_gpsWorkDownNum++;
	  if(m_gpsWorkDownNum>=gpscloseRFtimer)
  	  {
  	      if(m_gpsRcd!=GPS_OPR_SEND_CLOSE_RF)
  	      {
  	          OAM_LOGSTR1(LOG_WARN, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] GPS can't sync after %dmin, closed RF!!", gpscloseRFtimer/2);
	  	   gpsConfigL1(BTS_SYNC_SRC_BTS, true);
		   gpsConfigRFMask();
		   m_gpsRcd = GPS_OPR_SEND_CLOSE_RF;
#ifdef WBBU_CODE
           bGpsStatus_RF = 1;
                //zengjihan 20120814 for GPSSYNC            
                if(0 < bspGetslaveBTSID())
                {
                    LOG(LOG_DEBUG3, LOGNO(GPS,0), "//GPS锁定.主失步时间信息");
                    //GPS锁定.主失步时间信息
                    CComMessage *pComMsg = new(this, 16)CComMessage;
                    if(NULL != pComMsg)
                    {
                        pComMsg->SetMessageId(M_OAM_GPS_SYNC_NOTIFY);
                        pComMsg->SetSrcTid(M_TID_GM);
                        pComMsg->SetDstTid(M_TID_EMSAGENTTX);
                        pComMsg->SetBTS(bspGetslaveBTSID());
                        char *p = (char*)pComMsg->GetDataPtr();
                        p[0] = 0;
                        p[1] = 0;
                        p[2] = 0;
                        p[3] = 0;
                        p[4] = 0;
                        p[5] = 0;
                        p[6] = 0;
                        memset(&p[7],0,4);
                        p[11] = 0xaa;/**表示失步*/
                        if ( false == CComEntity::PostEntityMessage(pComMsg) )
                        {
                            pComMsg->Destroy();
                        }
                    }
                }
#endif
	      }	  	
	      m_gpsWorkDownNum=0;
  	   }
     }
     
	 	
	
    //status = ERROR 或者 锁定的卫星数太少；都认为是GPS失步
    if ((true == bGpsStatus)&&(BTS_SYNC_SRC_GPS == NvRamDataAddr->L1GenCfgEle.SyncSrc))
    {
        //GPS告警
        OAM_LOGSTR(LOG_SEVERE, L3GPS_ERROR_GPS_INFO_PRINT, "[tGPS] WARNING...GPS lose lock, maybe GPS link error, or GPS module error, or Config L1 General setting error.");
        AlarmReport(ALM_FLAG_SET, ALM_ENT_GPS, ALM_ENT_INDEX0,
                      ALM_ID_GPS_LOST, ALM_CLASS_CRITICAL, STR_GPS_LOST);
        //gpsConfigL1(BTS_SYNC_SRC_BTS);
    }
    bGpsStatus = false;
    return false;
}
#endif
#ifndef WBBU_CODE
STATUS gpsGetPpsControl()
#else
STATUS gpsGetPpsControl(unsigned char flag)
#endif
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    // @@Gc
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_PPS_CONTROL].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_PPS_CONTROL].code[1];

#ifndef WBBU_CODE
    cmd[cmdlen++] = 0xff;
#else
    cmd[cmdlen++] = flag/*0xff*/; 
#endif

    // Poll the GPS for the current PPS Control information
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 5);
    printf("mode - 0:1pps disable; 1:on continuously; 2:pulse active only when tracking one satellite\n");
    return status;  
}


STATUS gpsSetPpsControl(unsigned int mode)
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Gc
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_PPS_CONTROL].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_PPS_CONTROL].code[1];
    // Specify the PPS control mode
    if (mode <2 || mode == 0xff)
    {
        cmd[cmdlen++] = mode;  // 100pps
        // Poll the GPS for the current PPS Control information
        status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
        return status;
    }
    else
    {
        printf("mode invalid \n");
    }
}


STATUS gpsGetPpsMode()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
  //  int i;

    //@@AP
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_PULSE_MODE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_PULSE_MODE].code[1];
    cmd[cmdlen++] = 0xff;  
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    printf(" mode 0- 1pps; 1-100pps\n");
    return status;
}

STATUS gpsGetPositionControl()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
 //   int i;

    //@@Gd
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_HOLD_MODE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_HOLD_MODE].code[1];
    cmd[cmdlen++] = 0xff;  
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    printf(" mode 0: no hold; 1: enable position hold; 2:enable altitude hold; 3:autosite survey\n");
    return status;
}

STATUS gpsGetTimeMode()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Aw
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_TIME_MODE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_TIME_MODE].code[1];
    cmd[cmdlen++] = 0xff;  
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    printf(" mode 0: GPS; 1: UTC\n");
    return status;
}


STATUS gpsSetTimeMode(int mode)//0:utc, 1:gps
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Aw
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_TIME_MODE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_TIME_MODE].code[1];
    cmd[cmdlen++] = mode;  
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    printf("gpsSetTimeMode, return %d\n", status);
    return status;
}

typedef struct
{
    UINT8 month;
    UINT8 day;
    UINT16 year;
    UINT8  hour;
    UINT8  minute;
    UINT8  second;
    UINT8  GmtOffsetSign;
    UINT8  GmtOffsetHour;
    UINT8  GmtOffsetMinute;
}T_GPS_TIME;

STATUS gpsGetTime()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
  //  int i;

    //@@Gb
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME].code[1];
    for (int i=0; i<10; i++)
    {
        cmd[cmdlen++] = 0xff;  
    }

    char response[GPS_MAX_RESP_LENGTH] = {0};
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3, response);
    if (OK == status )
    {
        T_GPS_TIME *timePtr = (T_GPS_TIME*)&response[4];
        printf("time: %d/%d/%d[%d:%d:%d], GMT offset: %x %dhour %d minute\n",
               timePtr->year,timePtr->month,timePtr->day,timePtr->hour ,
               timePtr->minute,timePtr->second,timePtr->GmtOffsetSign,
               timePtr->GmtOffsetHour, timePtr->GmtOffsetMinute);
    }
    return status;
}


STATUS gpsGetRaimEnable()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Ge
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME_RAIM_ALGORITHM].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME_RAIM_ALGORITHM].code[1];
    // Enable the RAIM algorithm if the alarm limit is non-zero
    cmd[cmdlen++] = (char)0xff;

    char response[GPS_MAX_RESP_LENGTH] = {0};
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3, response);
    if (OK == status )
    {
        printf("time raim select is 0:disabled, 1 :enabled -- %d\n",response[4]);
    }
    return status;
}

typedef struct
{
    UINT8 PulseStatus;
    UINT8 PulseSync;
    UINT8 SlutionStatus;
    UINT8 TimeRaimStatus;
    UINT32 Svid;
    UINT16 TimeSolution;
}T_RaimStatus;

STATUS gpsGetRaimStatus()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Hn
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME_RAIM_STATUS].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME_RAIM_STATUS].code[1];
    // Enable the RAIM algorithm if the alarm limit is non-zero
    cmd[cmdlen++] = 0;

    char response[GPS_MAX_RESP_LENGTH] = {0};
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3, response);
    if (OK == status )
    {
        T_RaimStatus *raimStatus = (T_RaimStatus*)&response[4];
        printf("Pulse Status = %d(0:off,1:on)\n", raimStatus->PulseStatus);
        printf("Pulse Sync   = %d(0:UTC, 1:GPS)\n", raimStatus->PulseSync);
        printf("Time Raim Solution Status = %d (0:OK,1:ALARM,2:Unknown)\n",raimStatus->SlutionStatus);
    }
    return status;
}

//TIME_OF_DAY
STATUS gpsSetTimeOfDay(char hour, char min, char sec)
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Aa
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_TIME_OF_DAY].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_TIME_OF_DAY].code[1];
    cmd[cmdlen++] = hour;  
    cmd[cmdlen++] = min;  
    cmd[cmdlen++] = sec;  
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    printf("gpsSetTimeOfDay, return %d\n", status);
    return status;
}
//GMTCorrection
STATUS gpsSetGMTCorrection(UINT8 gmt)
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Ab
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_GMT_CORRECTION].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_GMT_CORRECTION].code[1];
    cmd[cmdlen++] = 0; 
    cmd[cmdlen++] = gmt;
    cmd[cmdlen++] = 0;
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    printf("gpsSetGMTCorrection, return %d\n", status);
    return status;
}
//Date
STATUS gpsSetDate(int year, char month, char day)
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Ac
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_DATE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_DATE].code[1];
    cmd[cmdlen++] = month; 
    cmd[cmdlen++] = day;
    cmd[cmdlen++] = year>>8;
    cmd[cmdlen++] = year&0xff;
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    printf("gpsSetDate, return %d\n", status);
    return status;
}
//SATELLITE_MASK_ANGLE, default 10, if gps is not ok, set 0
STATUS gpsSetSatelliteMaskAngle(char angle)
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Ag
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_SATELLITE_MASK_ANGLE].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_SATELLITE_MASK_ANGLE].code[1];
    cmd[cmdlen++] = angle;     
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    printf("gpsSetSatelliteMaskAngle, return %d\n", status);
    return status;
}
#ifdef WBBU_CODE
STATUS gpsGetStatus_ublox()
{
    printf("GPS time: %d/%d/%d[%d:%d:%d]\n", gUbloxGpsAllData.Year, gUbloxGpsAllData.Month, gUbloxGpsAllData.Day ,
        gUbloxGpsAllData.Hour ,gUbloxGpsAllData.Minute, gUbloxGpsAllData.Second);
    printf("Gps Position: Lati (%d), Long(%d), H (%d)\n", gUbloxGpsAllData.Latitude,gUbloxGpsAllData.Longitude,gUbloxGpsAllData.GPSHigh);    
    printf("visable satellite:0x%x,tracked_satellite:0x%x\n", gUbloxGpsAllData.VisibleSatellites, gUbloxGpsAllData.TrackedSatellites);
    switch(gUbloxSData.antennaInfo.aStatus)
    {
        case 0x00 :
           printf("Status of the Antenna Supervisor State Machine: INIT\n");
           break;
        case 0x01:
            printf("Status of the Antenna Supervisor State Machine: DONTKNOW\n");
            break;
        case 0x02:
            printf("Status of the Antenna Supervisor State Machine: OK\n");
            break;
        case 0x03:
            printf("Status of the Antenna Supervisor State Machine: SHORT\n");
            break;
        case 0x04:
            printf("Status of the Antenna Supervisor State Machine: OPEN\n");
            break;
        default:
            break;
    }
    switch(gUbloxSData.antennaInfo.aPower)
    {
        case 0x00 :
           printf("Current PowerStatus of Antenna: OFF\n");
           break;
        case 0x01:
            printf("Current PowerStatus of Antenna: ON\n");
            break;
        case 0x02:
            printf("Current PowerStatus of Antenna: DONTKNOW\n");
            break;        
        default:
            break;
    }
    printf("Space Vehicle Information, Channel num: %d qualityInd: ", gUbloxSData.svInfo.numCh);
    for(int i=0; i<gUbloxSData.svInfo.numCh; i++)
    {
        printf("%d ", gUbloxSData.svInfo.sInfo[i].quality);
    }
    printf("\n");
}
void showGpsType()
{
    if(gpsModel == GPS_MODEL_M12)
        printf("GPS is M12!!\n");
    else if(gpsModel == GPS_MODEL_UBLOX)
        printf("GPS is UBLOX!!\n");
    else
        printf("GPS is Unknown!!\n");
}
#endif
STATUS gpsGetStatus()
{
    #ifdef WBBU_CODE
    showGpsType();
    if(gpsModel == GPS_MODEL_UBLOX)
    {
        gpsGetStatus_ublox();
        return OK;
    }
    
    #endif
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
    int i;
 

    //@@Ha
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[1];
    // Enable the RAIM algorithm if the alarm limit is non-zero
    cmd[cmdlen++] = 0;

    char response[GPS_MAX_RESP_LENGTH] = {0};
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3, response);
    if ( OK == status )
    {
        T_GpsAllData *gpsData=(T_GpsAllData*)&response[0];
	 M12T_GPS_STATUS_INFO *m12t_gps_info = (M12T_GPS_STATUS_INFO*)(&gpsData->DOPValue);
        printf("GPS time: %d/%d/%d[%d:%d:%d:%d]\n", gpsData->Year, gpsData->Month, gpsData->Day ,
               gpsData->Hour ,gpsData->Minute, gpsData->Second,gpsData->MinSec);
        printf("Gps Position: Lati (%d), Long(%d), H (%d)\n", gpsData->Latitude,gpsData->Longitude,gpsData->GPSHigh);
        
	 printf("visable satellite:0x%x,tracked_satellite:0x%x,dd:0x%x\n",\
			m12t_gps_info->n,m12t_gps_info->t,m12t_gps_info->dd);	
	switch(m12t_gps_info->ss.gps_status)/*111,0,0000*/
	{
		case 0x07:/*111*/
			printf("3D Fix,\n");
			break;
		case 0x06: /*110,0,0000*/
			printf("2D Fix,\n");
			break;
		case 0x05 : /*101,0,0000*/
			printf("Propagate Mode,\n");
			break;
		case 0x04 :  /*100,0,0000*/
			printf("Position Hold,\n");
			break;
		case 0x03 : /*011,0,0000*/
			printf("Acquiring Satellites,\n");
			break;
		case 0x02 : /*010,0,0000*/
			printf("Bad Geometry,\n");
			break;
		default : /*001,0,0000||000,0,0000*/
			printf("Reserved,\n");
			break;
	}	
	printf("Narrow track mode:%x,\n",m12t_gps_info->ss.ntm);
	printf("Fast Acquisition Position:%x,\n",m12t_gps_info->ss.fap);	
	printf("Filter Reset To Raw GPS Solution:%x \n",m12t_gps_info->ss.frtrgpss);	
	printf("Cold Start:%x,\n",m12t_gps_info->ss.cs);
	printf("Differential Fix:%x,\n",m12t_gps_info->ss.df);	
	printf("Position Lock:%x,\n",m12t_gps_info->ss.pl);
	printf("Autosurvey Mode:%x,\n",m12t_gps_info->ss.am);
	printf("Insufficient Visible Satellites:%x,\n",m12t_gps_info->ss.ivs);	
	switch(m12t_gps_info->ss.as)
	{
		case 0x00 :
			printf("Antenna Sense: ok\n");
			break;
		case 0x01:
			printf("Antenna Sense: short circuit\n");
			break;
		case 0x02:
			printf("Antenna Sense:open circuit\n");
			break;
		case 0x03:
			printf("Antenna Sense:status not available\n");
			break;
		default:
			break;
	}
	if(m12t_gps_info->ss.cl !=0 )
		printf("Code Location:INTERNAL \n");
	else
		printf("Code Location:EXTERNAL \n");
	
	
	for(i=0;i<MAX_M12T_GPS_CHAN;i++)	/*for(i=0;i<m12t_gps_info.t;i++)*/
	{
		printf("%02d,SVID:%02d,mode:%d,signal_strength:%03d,IODE:%03d,dd:0x%04x\n",\
			i,
			m12t_gps_info->chan_data[i].i,m12t_gps_info->chan_data[i].m,
			m12t_gps_info->chan_data[i].s,m12t_gps_info->chan_data[i].iode,
			m12t_gps_info->chan_data[i].dd_h*0x100+m12t_gps_info->chan_data[i].dd_l);		
	}
	printf("clock bias:%dns,oscillator offset:%dHz,temperature:%d\n",
		m12t_gps_info->cc,m12t_gps_info->oooo,m12t_gps_info->tt);
	if(m12t_gps_info->u.utc_mode !=0 )
		printf("UTC mode enabled,\n");
	else
		printf("UTC mode disabled,\n");
	if(m12t_gps_info->u.utc_offset !=0 )
		printf("UTC offset decoded,value:%02d\n",
					m12t_gps_info->u.offset_value);
	else
		printf("UTC offset NOT decoded\n");
	if(m12t_gps_info->gmt_offset.s == 0)
		printf("GMT offset positive,\n");
	else
		printf("GMT offset negative,\n");
	printf("hour:%d,minute:%d\n", 	m12t_gps_info->gmt_offset.h,m12t_gps_info->gmt_offset.m);
    }
    return status;
}
/*
 *GPS长时间未用，需要使用"ps12 1 f"命令激活使用
 */
STATUS gpsSetActive()
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;
//    int i;

    //@@Ha
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_POSITION_STATUS_DATA].code[1];

    cmd[cmdlen++] = 1;

    char response[GPS_MAX_RESP_LENGTH] = {0};
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3, response);
    if ( OK == status )
    {
        printf("set GPS to ACTIVE success.\n");
    }
    return status;
}


STATUS gpsSetTime(UINT16 year, UINT8 mon, UINT8 day, UINT8 hour, UINT8 minu, UINT8 sec, UINT8 gmt)
{
    int status;
    char cmd[GPS_MAX_CMD_LENGTH];
    int cmdlen = 0;

    //T_TimeDate timeData = bspGetDateTime();

    //@@Gb
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME].code[0];
    cmd[cmdlen++] = GPS_COMMAND_TABLE[CMD_M12_TIME].code[1];
    cmd[cmdlen++] = mon;//timeData.month;
    cmd[cmdlen++] = day;//timeData.day;
    cmd[cmdlen++] = year>>8;//timeData.year>>8;
    cmd[cmdlen++] = year&0xff;//timeData.year&0xff;
    cmd[cmdlen++] = hour;//timeData.hour;
    cmd[cmdlen++] = minu;//timeData.minute;
    cmd[cmdlen++] = sec;//timeData.second;
    cmd[cmdlen++] = 0;
    cmd[cmdlen++] = gmt;
    cmd[cmdlen++] = 0;
    
    status = CTaskGPS::GetInstance()->gpsExecCommand(cmd, cmdlen, 3);
    if ( OK == status )
        printf("gpsSetTime, return OK\n");
    else
	 printf("gpsSetTime, return err\n");
    return status;
}


//set gps close RF time
void setGPSCloseRFTimer(int period)
{	
       T_NVRAM_BTS_OTHER_PARA para;	
	para.gpscloseRFtimer = period;
	para.gpsSetFlag = 0x5a5a;
	//printf("result = %d\n", gpscloseRFtimer);
	bspNvRamWrite((char *)NVRAM_BASE_ADDR_OTHER_PARAS, (char *)&para, sizeof(para));
       CTaskGPS* pTInstance = CTaskGPS::GetInstance();
       pTInstance->gpsSetCloseTimer(period*2);
}
void GPSCloseRFTimerShow()
{
    T_NVRAM_BTS_OTHER_PARA *para = (T_NVRAM_BTS_OTHER_PARA*)NVRAM_BASE_ADDR_OTHER_PARAS;
    if(para->gpsSetFlag == 0x5a5a)
        printf("\nGPSCloseRFTimer = %d min\n", para->gpscloseRFtimer);
    else
	 printf("\nuse default value: 20 min\n");     
}

void  RF_config( BOOL closeflag)
{
        CTaskGPS* pTInstance = CTaskGPS::GetInstance();
      if(closeflag==true)
      	{
      	    pTInstance->gpsConfigL1(NvRamDataAddr->L1GenCfgEle.SyncSrc,true);
			#ifdef WBBU_CODE
			 pTInstance->gpsConfigRFMask(0);//关闭告警
			#endif
      	}
	 else
	 {
	     pTInstance->gpsConfigL1(NvRamDataAddr->L1GenCfgEle.SyncSrc,false);
         #ifdef WBBU_CODE
		 pTInstance->gpsConfigRFMask(1);//打开告警标志
		 #endif
	 }
  
}

void changeGPSTime(UINT16 year, UINT8 mon, UINT8 day, UINT8 hour, UINT8 minu, UINT8 sec, UINT8 gmt)
{
    gpsSetTimeMode(1);//Aw 1
    gpsSetTime(year, mon, day, hour, minu, sec, gmt); //Gb
    gpsSetGMTCorrection(gmt);//Ab
    gpsSetTimeOfDay(hour, minu, sec);//Aa
    gpsSetDate(year, mon, day);
    
}

#ifdef WBBU_CODE
extern "C" void   TestGPS(unsigned char flag)
{
                  CComMessage *pComMsg = new(CTaskGPS::GetInstance(), 16)CComMessage;
                    if(NULL != pComMsg)
                    {
                        pComMsg->SetMessageId(M_OAM_GPS_SYNC_NOTIFY);
                        pComMsg->SetSrcTid(M_TID_GM);
                        pComMsg->SetDstTid(M_TID_GM);
                        pComMsg->SetBTS(bspGetslaveBTSID());
                        char *p = (char*)pComMsg->GetDataPtr();
                        p[0] = 0;
                        p[1] = 0;
                        p[2] = 0;
                        p[3] = 0;
                        p[4] = 0;
                        p[5] = 0;
                        p[6] = 0;
                        memset(&p[7],0,4);
                        p[11] = flag;/**表示失步*/
                        if ( false == CComEntity::PostEntityMessage(pComMsg) )
                        {
                            pComMsg->Destroy();
                        }
              }
}
#endif

