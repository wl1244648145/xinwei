/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bbu_config.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_GPS_EXT_H
#define BSP_GPS_EXT_H

#define E_GPS               (-1)
#define GPSNUMCHINFO        (30)
#define GPS_OK                                 0
#define GPS_ERROR                        -1

#define  GPS_AVAILABLE             1
#define  GPS_NOT_AVAILABLE     0

typedef struct
{
    u8    GpsInitFlag;
    u8    GpsAvailable;
}STRU_GPS_FLAG;


typedef struct
{
    char  command[4];
    char  Month;  
    char  Day;
    unsigned short  Year;
    char  Hour;
    char  Minute;
    char  Second;
    int   MinSec;
    pthread_mutex_t m_mutex;
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
}T_GpsAllData;   // Message Length:154 Bytes
typedef struct
{
    UINT8 chn;
    UINT8 svid;
    UINT8 flags;
    UINT8 quality;
    UINT8 cno_db;
    char elev;
    short azim;
    int prRes;
}UBLOX_SatelliteInfo;

typedef struct
{
    UINT32 iTow;
    UINT8 numCh;
    UINT8 globalFlags;
    UINT16 res2;
    UBLOX_SatelliteInfo sInfo[GPSNUMCHINFO];
}UBLOX_SVInfo;

typedef struct
{
    UINT8 aStatus;//Status of the Antenna Supervisor State Machine(0=INIT, 1=DONTKNOW, 2=OK, 3=SHORT,4=OPEN)
    UINT8 aPower;//Current PowerStatus of Antenna (0=OFF, 1=ON,2=DONTKNOW)
}UBLOX_Antenna;

typedef struct
{
    UBLOX_SVInfo svInfo;
    UBLOX_Antenna antennaInfo;
	pthread_mutex_t m_mutex;
}UBLOX_SData;

typedef struct
{
	UINT8 gps_Type;
	UINT8 gps_ClockSynWay;
	UINT8 gps_CascadeCfg;
}Clock_Set_Tc_Type;


Export u8 g_GNSS_flag;
Export u32 gLeapSecond; 
Export s32 gps_GMT_Offset;//时区
Export T_GpsAllData gUbloxGpsAllData;
Export s32 bsp_gps_init(void);
Export s32 gps_NAV_TimeUTC(s32 gps_gmt_offset);
Export s32 gps_NAV_SVInfo(void);
Export s32 gps_NAV_PosLLH(void);
Export s32 gps_MON_HW(void);
Export s32 bsp_gps_update_rtc(void);
Export s32 interpro_send_gps_info(void);
Export u8 bsp_gps_TrackedSatellites(void);
Export u8 bsp_gps_VisibleSatellites(void);
Export void gps_get_position(sl32 *psl32Latitude,sl32 *psl32Longitude);

#endif
/******************************* 头文件结束 ********************************/

