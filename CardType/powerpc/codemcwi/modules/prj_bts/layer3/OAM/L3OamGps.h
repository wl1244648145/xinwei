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
 *   08/03/2005   ╠я╛▓╬░       Initial file creation.
 *---------------------------------------------------------------------------*/
#pragma warning (disable : 4786)
#ifndef INC_OAML3GPS
#define INC_OAML3GPS

#ifdef __WIN32_SIM__
#include <windows.h>
#include <list>
using namespace std;
#else
#include <list>
#include <stdio.h>
#endif

#include "BizTask.h"
#include "Timer.h"
#include "L3OamMessageId.h"
#include "L3OamCommon.h"
#include "L3OamCfgCommon.h"

#define MAX_M12T_GPS_CHAN 12
#define SEND_L1_MSG_TIMEOUT 0x1111
typedef enum _GPS_MODEL
{
	GPS_MODEL_M12=0,
	GPS_MODEL_UBLOX,
	GPS_MODEL_MAX
}GpsModel;

#pragma pack(1)
#ifdef WBBU_CODE
struct UBLOX_SatelliteInfo
{
    UINT8 chn;
    UINT8 svid;
    UINT8 flags;
    UINT8 quality;
    UINT8 cno_db;
    char elev;
    short azim;
    int prRes;
};

struct UBLOX_SVInfo
{
    UINT32 iTow;
    UINT8 numCh;
    UINT8 globalFlags;
    UINT16 res2;
    UBLOX_SatelliteInfo sInfo[16];
};
struct UBLOX_Antenna
{
    UINT8 aStatus;//Status of the Antenna Supervisor State Machine(0=INIT, 1=DONTKNOW, 2=OK, 3=SHORT,4=OPEN)
    UINT8 aPower;//Current PowerStatus of Antenna (0=OFF, 1=ON,2=DONTKNOW)
};
struct UBLOX_SData
{
    UBLOX_SVInfo svInfo;
    UBLOX_Antenna antennaInfo;
};
#endif
struct T_ChannelData
{
    char  StatelliteNo;
    char  Model;
    char  SNR;
    char  IODE;
    unsigned short  Status;
};

struct T_GpsAllData
{
    char  command[4];
    char  Month;  
    char  Day;
    unsigned short  Year;
    char  Hour;
    char  Minute;
    char  Second;
    int   MinSec;
    unsigned int  Latitude;
    unsigned int  Longitude;
    unsigned int  GPSHigh;
    unsigned int  MSLHigh;
    unsigned int  Latitude1;
    unsigned int  Longitude1;
    unsigned int  GPSHigh1;
    unsigned int  MSLHigh1;
    unsigned short  D3Speed;
    unsigned short  D2Speed;
    unsigned short  D2SpeedDirect;
    unsigned short  DOPValue;
    unsigned char  VisibleSatellites;
    unsigned char  TrackedSatellites;
    unsigned int aaaa[18];
    unsigned short  StatusMsb        : 3;
    unsigned short  StatusRev0        : 1;
    unsigned short  StatusRev1        : 1;
    unsigned short  StatusRev2        : 1;
    unsigned short  StatusFastAcqPos : 1;
    unsigned short  StatusFilter     : 1;
    unsigned short  StatusCold      : 1;
    unsigned short  StatusDiffFix    : 1;
    unsigned short  StatusPosLoc     : 1;
    unsigned short  StatusAutoSurvey: 1;
    unsigned short  StatusInsufficientSatellite : 1;
    unsigned short  StatusAntennaSense: 2;
    unsigned short  CodeLocation:1;
    unsigned short  Resv;
    unsigned short  ClockBias;
    unsigned int  OscillatorOffset;
    unsigned short  Temperature;
    unsigned char UTCPara;
    char  SignedGMTOffset;
    char  HourGMTOffset;
    char  MinuteGMTOffset;
    char  IDTag[6];
    unsigned char CheckSum; 
    char  CharCR;
    char  CharLF;  
};   // Message Length:154 Bytes

typedef struct 
{
	unsigned char i;/*SVID 0бн37*/
	unsigned char m;/*mode 0бн8*/
	unsigned char s;/*signal strength 0бн255*/
	unsigned char iode;/*IODE 0бн255*/
	unsigned char dd_h;/*channel status (8 bits h)*/
	unsigned char dd_l;/*channel status (8 bits l)*/
}M12T_CHAN_DATA;
typedef struct 
{
	 unsigned char gps_status	:3;/*111 = 3D Fix 
	 							110 = 2D Fix 
	 							101 = Propagate Mode 
	 							100 = Position Hold 
	 							011 = Acquiring Satellites 
	 							010 = Bad Geometry 
	 							001 = Reserved 
	 							000 = Reserved*/
	 unsigned char rsv1		:2;/*Reserved*/
	 unsigned char ntm		:1;/*Narrow track mode (timing only)*/
	 unsigned char fap			:1;/*Fast Acquisition Position*/
	 unsigned char frtrgpss	:1;/*Filter Reset To Raw GPS Solution*/

	 unsigned char cs			:1;/*Cold Start */
	 unsigned char df			:1;/*Differential Fix*/
	 unsigned char pl			:1;/*Position Lock*/
	 unsigned char am			:1;/*Autosurvey Mode*/
	 unsigned char ivs			:1;/*Insufficient Visible Satellites*/
	 unsigned char as			:2;/*Antenna Sense 00 = OK 01 = OC 10 = UC 11 = NV*/
	 unsigned char cl			:1;/*Code Location 0 = EXTERNAL 1 = INTERNAL*/
}SS;/*receiver status*/
typedef struct
{
	unsigned char utc_mode	:1;/*1 = enabled 0 = disabled*/
	unsigned char utc_offset	:1;/*1 = decoded 0 = NOT decoded*/
	unsigned char offset_value	:6;/*Present UTC offset value, 
								range иC32бн+31; 
								from GPS time* (ignore if Bit 6 = 0).
								*/
}UTC_PARA;
typedef struct
{
	unsigned char s;/*signed byte of GMT offset 00 = positive, ff = negative*/
	unsigned char h;/*hour of GMT offset 0бн+*/
	unsigned char m;/*minute of GMT offset 0бн59*/
	unsigned char v[6];/*ID tag 6 characters (0x20 to 0x7e)*/
	unsigned char c;/*checksum*/
}GMT_OFFSET;
typedef struct
{
	unsigned short dd;/*current DOP*/
	unsigned char n;/*number of visible satellites 0бн12*/
	unsigned char t;/*number of tracked satellites 0бн12*/
	M12T_CHAN_DATA chan_data[MAX_M12T_GPS_CHAN];
	SS ss;/*receiver status*/
	unsigned short rr;/*reserved*/
	short cc;/*clock bias -32768бн32767 ns*/
	unsigned int oooo; /*oscillator offset 0бн250000 Hz*/
	short tt;/*temperature -110бн250 half degrees C (-55.0бн+125.0буC)*/
	UTC_PARA u;/*UTC Parameters*/
	GMT_OFFSET gmt_offset;
}M12T_GPS_STATUS_INFO;
#pragma pack()

typedef enum {
    CMD_TIME_OF_DAY,
    CMD_GMT_CORRECTION,
    CMD_DATE,
    CMD_SATELLITE_MASK_ANGLE,
    CMD_PULSE_MODE,
    CMD_POSITION_HOLD_POSITION,
    CMD_TIME_MODE,
    CMD_CABLE_DELAY,
    CMD_SET_TO_DEFAULTS,
    CMD_RECEIVER_ID,
    CMD_M12_POSITION,
    CMD_M12_TIME,
    CMD_M12_PPS_CONTROL,
    CMD_M12_POSITION_HOLD_MODE,
    CMD_M12_TIME_RAIM_ALGORITHM,
    CMD_M12_TIME_RAIM_ALARM,
    CMD_M12_POSITION_STATUS_DATA,
    CMD_M12_TIME_RAIM_STATUS,
    CMD_M12_SELF_TEST,
    CMD_POWER_ON_FAILURE,
    CMD_UNKNOWN,  /* must be last in list */
    CMD_NUM_COMMANDS = CMD_UNKNOWN
} GPS_COMMANDS;
typedef enum
{
    GPS_OPR_INIT,
    GPS_OPR_GPS_OK,
    GPS_OPR_SEND_CLOSE_RF
}GPS_OPR_STATUS;
class CTaskGPS:public CBizTask
{
public:
    CTaskGPS();
    static CTaskGPS* GetInstance();    
	int    gpsExecCommand(const char *msg, const UINT8 length, const int timeout, char* = NULL);
    void gpsSetCloseTimer(int value){ gpscloseRFtimer = value;}
    int getGpsCloseTimer(){return gpscloseRFtimer;}
   bool  gpsConfigL1(UINT16, BOOL);
   void gpsConfigRFMask(UINT8 flag=0);
private:
    bool  Initialize();
    bool  ProcessMessage(CMessage&);
    TID   GetEntityId() const;

    #define GPS_MAX_BLOCKED_TIME_IN_10ms_TICK (500)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return GPS_MAX_BLOCKED_TIME_IN_10ms_TICK ;};


////////
////void  gpsFlushSIO();
    bool  gpsInitSIO();
	bool  gpsConfig();
	bool  gpsHeartbeatTimeOut();
	bool gpsReturnHlrReq();
    CTimer* gpsCreatetimer(UINT16 MsgId, bool IsPeriod, UINT32 TimerPeriod);
    bool  gpsSynchronize(T_GpsAllData &);
	bool  gpsLocationModifyToEMS(T_GpsDataCfgEle&);
   // bool  gpsConfigL1(UINT16, BOOL);
    bool  gpsGenRspHandle(CMessage &rMsg);
    int   gpsSetGpsTime();
    int   gpsSetGpsLocation();
    //zengjihan 20120801 for GPSSYNC
    #ifdef WBBU_CODE
    void gpsSyncNotify(CMessage &);
    #endif
    GPS_COMMANDS gpsGetCommandCode(const char *);
    void  GPS_TRACE(const char *prefix, const char *msg, const int msglen, bool isM12);
    int   TimedRead(int, char *, const int, int);
    int   gpsSetPositionControl(int);
     #ifdef WBBU_CODE
    int gpsJudgeModel();
    STATUS gps_Cfg_Tp1();
    STATUS gps_Cfg_Tp2();    
    STATUS gps_Cfg_ANT();
    int gpsExecCommand_UBlox(char *msg, const UINT8 length, char *pOutPutResponse, UINT8 resplen);
    STATUS gps_NAV_TimeUTC();
    STATUS gps_NAV_PosLLH();
    STATUS gps_MON_HW();
    STATUS gps_NAV_SVInfo();
    STATUS gps_Close_GGA();
    STATUS gps_Close_GLL();
    STATUS gps_Close_GSA();
    STATUS gps_Close_GSV();
    STATUS gps_Close_RMC();
    STATUS gps_Close_VTG();
    STATUS gps_Close_ZDA();
    STATUS gps_Close_SABS();
    bool gpsInitSIO_UBlox();
    bool gpsHeartbeatTimeOut_UBlox();
     #endif
private:
	static CTaskGPS *m_Instance;	
    SINT32  m_gpssfd;
    CTimer *m_pGPSHeartBeatTimer;
    UINT8   m_RtcUpdateInterval;
    SEM_ID  m_gpsSemId;
	bool	m_bInitSIO;
    int m_gpsWorkDownNum;
    int m_gpsRcd;
    int gpscloseRFtimer;
};
#endif

