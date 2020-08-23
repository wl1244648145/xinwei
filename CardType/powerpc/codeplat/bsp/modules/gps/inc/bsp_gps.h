/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_gps.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_GPS_H
#define BSP_GPS_H
#if 0
#define E_GPS               (-1)

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
    UBLOX_SatelliteInfo sInfo[16];
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
}UBLOX_SData;
s32 gps_disable_tp(void);
#endif
#endif
/******************************* 头文件结束 ********************************/
