/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:        sysBtsConfigData.h
 *
 * DESCRIPTION:   define the data structures, declare the functions needed for
 *                 BTS configration data modification
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   12/11/2005   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef BTS_CONFIG_DATA_H__
#define BTS_CONFIG_DATA_H__

#include "sysBootLoad.h"

/***i2c addr*****/
#define TEMP_SENSOR_ADDR   0x49


#define MAX_VLAN_ID   4095
#define DEFAULT_EMS_RCV_PORT   3999
#define DEFAULT_BTS_RCV_PORT   8002


#define VERSION_NUM		0x2
#define SERIAL_NUM		0x4
#define PRODUCT_DATE	0x10

#define VERSION_NUM_LEN		2
#define SERIAL_NUM_LEN		12
#define PRODUCT_DATE_LEN	4



typedef enum
{
    BTS_BOOT_DATA_SOURCE_BTS = 30,
    BTS_BOOT_DATA_SOURCE_EMS = 31
}BTS_BOOT_DATA_SOURCE;


typedef struct 
{
	unsigned int    nvramSafe;   /***for bts config data **0x55aa55aa*********/

    /*RESET_REASON             btsRstReason;*/  /****move to T_BootLoadState *****/
    /*unsigned int             resetFlag; */  /****move to T_BootLoadState *****/

    BTS_BOOT_DATA_SOURCE     dataSource;

	unsigned int    emsIp;
	unsigned int    btsId;
    unsigned int    emsRcvPort;
    unsigned int    btsRcvPort;
	unsigned int    vlanId;
	unsigned int    btsIp;
	unsigned int    btsPort;
    unsigned int    bakemsIP;
    unsigned int    bakemsRcvPort;

    unsigned int    rebootBtsWanIfDiscPattern;
    int             rebootBtsWanIfDisc;

    unsigned int    permitPPPoEPattern;
    int             permitPPPoE;

    unsigned int    filterSTPPattern;
    int             filterSTP;
   
}T_NVRAM_BTS_CONFIG_DATA;

 typedef struct  
{    
    unsigned int    rebootBtsIfFtpDownPattern;
    int             rebootBtsIfFtpDown;
    UINT32 jammingPatter;/*lijinan 090105 for jamming rpt flow ctl*/
    UINT32 jammingRptFromL2Num;
    UINT32 jammingRptFromBtsNum;
    UINT32 permitUseWhenSagDownPattern;
    UINT32  permitUseWhenSagDown;/*0:close, 1:open*/
    UINT32  RRU_Antenna;/****  8   8通道，4-----4通道*****************/
    UINT32 slaveBTSIDFLAG;//zengjihan 20120801 for GPSSYNC
    UINT32 slaveBTSID;//zengjihan 20120801 for GPSSYNC
}     T_NVRAM_BTS_CONFIG_PARA;
typedef struct
{
    unsigned int    BtsIfNETWORKPattern;/**wangwenhua add 20090224**/
    unsigned int              TYPE ;/*0-auto 1-non auto***/
    unsigned int              SPEED;/*0-10M,1-100M****/
    unsigned int              mode   ;/**0-half 1-full******/
}T_NVRAM_BTS_NETWORK_PARA;

typedef struct/*保存需要存在nvram中的参数*/
{
    unsigned int    gpsSetFlag;
    unsigned int    gpscloseRFtimer;/**gps关闭延迟时间**/
    
}T_NVRAM_BTS_OTHER_PARA;
typedef struct
{
    UINT16  year;		
    UCHAR   month;
    UCHAR   day;
    UCHAR   hour;		
    UCHAR   minute;		
    UCHAR   second;		
} T_TimeDate;






#define DEVICE_RAMDISK		"/RAMD"/*"/RAMD/"  */ /*  "/RAMDISK/"   */
#define RAMDISK_BLOCK_SIZE	(1024*64)    /***(1024*32)* 512 = 16M*******/

#ifdef __cplusplus
extern "C"
{
#endif

UINT8 sysNvRead (ULONG offset  /* NVRAM offset to read the byte from */);
void sysNvWrite
    (
    ULONG	offset,	/* NVRAM offset to write the byte to */
    UCHAR	data	/* datum byte */
    );

int bspNvRamWrite(char * TargAddr, char *SrcBuff, int size);
STATUS bspNvRamRead(char * TargAddr, char *SrcBuff, int size);

void bspTimeSet(int hour,int minute,int second);
void bspDateSet(int year,int month,int dayOfMonth,int dayOfWeek);
T_TimeDate bspGetDateTime();

int  bspGetBtsUDPRcvPort();
void bspSetBtsUDPRcvPort(int temp);

int  bspGetBtsPubIp();
void bspSetBtsPubIp(int ipaddr);

int  bspGetBtsPubPort();
void bspSetBtsPubPort(int port);

/*int  bspGetEmsUDPRcvPort();*/
/*void bspSetEmsUDPRcvPort(int temp);*/

int  bspGetNvramSafe();
void bspSetNvramSafe(int temp);

int  bspGetResetNum();
void bspSetResetNum(int temp);

BOOT_PLANE bspGetBootPlane();
void bspSetBootPlane(BOOT_PLANE );

void bspSetWorkFlag(LOAD_STATUS );
void bspSetResetFlag(int);
#if 0
int  bspGetEmsIpAddr();
void bspSetEmsIpAddr(int);
#endif
int bspGetBtsID();
void bspSetBtsID(int );

RESET_REASON bspGetBtsResetReason();			
void bspSetBtsResetReason(RESET_REASON);

int bspGetVlanID();
void bspSetVlanID(int);

BTS_BOOT_DATA_SOURCE bspGetBootupSource();
void bspSetBootupSource(BTS_BOOT_DATA_SOURCE);

LOAD_STATUS bspGetWorkFlag();

int bspGetResetFlag();


unsigned int bspGetCPLDVersion();

int rebootBTS(int type);

unsigned int GetBtsIpAddr();


int bspGetAtaState();


/*********/
void bspEnableCFWrite( BOOL bEnable );

/*********/
STATUS bspGetVersionNum(UINT16* version);
STATUS bspGetPcbVersion(char *buf);
unsigned char  bspGetDeviceID( unsigned char *device);
STATUS bspGetSerialNum(UINT16 *serial);
STATUS bspGetProductionDate(UINT8* month, UINT8* day, UINT16* year);
UINT8  GenerateCRC8for8N(UINT8* DataBuf,UINT16 len);

int bspGetBTSTemperature();  /*bts temperature*/

int bspEnableNvRamWrite(char *startAddr, UINT32 size);
int bspDisableNvRamWrite(char *startAddr, UINT32 size);
void RegisterRebootCallbackFunc( FUNCPTR func);
void ResetBTSViaCPLD();

UINT32 bspGetBtsHWVersion();
UINT16 bspGetBtsHWType();

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif  /*BTS_CONFIG_DATA_H__ */
