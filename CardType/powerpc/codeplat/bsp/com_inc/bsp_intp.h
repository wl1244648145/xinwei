/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_intpro_ext.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_INTPRO_EXT_H
#define BSP_INTPRO_EXT_H
#ifdef __CPU_LTE_MACRO__
extern int interpro_socket_init(void);
extern int  interpro_send_msg(char *MsgContent, UINT32 u32MsgLen );
//extern void  interpro_send_gps_info(void)


#define FILEVER_STR_LEN      16 // bts."111.222.333.444".bin
#define USER_NAME_LEN        10
#define USER_PASSWORD_LEN      10
#define FILE_DIRECTORY_LEN      50
#define FILE_NAME_LEN  60

/*BEGIN: added by zs 20131106,  PN: BTS1303 公共模块同业务模块间的消息ID定义*/
#define M_L3_PLAT_GENERAL_SETTING_REQ  0x5000
#define M_PLAT_L3_GENERAL_SETTING_RSP  0x5001
#define M_L3_PLAT_GPS_DATA_CFG_REQ   0x5002
#define M_PLAT_L3_GPS_DATA_CFG_RSP   0x5003
#define M_PLAT_L3_SYNSRC_CHANGE_NOTIFY 0x5004;
#define M_PLAT_L3_SYNSRC_STATUS_CHANGE_NOTIFY 0x5005;
#define M_L3_PLAT_GPS_INFO_QUERY_REQ   0x5006
#define M_PLAT_L3_GPS_INFO_QUERY_RSP   0x5007

#define M_L3_PLAT_DL_BTS_SW_REQ  0x5009
#define M_PLAT_L3_DL_BTS_SW_RSP   0x500a
#define M_PLAT_L3_DL_BTS_SW_RESULT_NOTIFY   0x500b
#define M_L3_PLAT_SWITCH_BTS_SW_REQ   0x500c

#define M_L3_PLAT_RESET_SINGLE_DSP   0x500d
#define M_PLAT_L3_RESET_SINGLE_DSP_RSP   0x500e
#define M_L3_PLAT_RESET_ALL_DSP   0x500f
#define M_PLAT_L3_RESET_ALL_DSP_RSP  0X5010

#define M_L3_PLAT_CPU_WORK_STATUS_NOTIFY   0x5011
#define M_PLAT_L3_CPU_WORK_STATUS_RSP   0x5012
#define M_L3_PLAT_HEARTBEAT   0x5008


struct T_MsgHeader
{   
    UINT16  MsgId;     //00：BTS本地同步,01：GPS同步,02：E1同步,03：光口同步,04：1588同步,05：485同步
    UINT16  transID;   //         
 };

//3.1.1.1   同步源配置请求
struct T_L1GenCfgEle
{   
    UINT16  SyncSrc;     //00：BTS本地同步,01：GPS同步,02：E1同步,03：光口同步,04：1588同步,05：485同步
    UINT16  GpsOffset;   //         
    UINT16  AntennaMask; //  0-disable, 1-enable       LSB is antenna 0
};

//3.1.2.1   GPS数据配置请求
struct T_GpsDataCfgEle
{
    SINT32  Latitude;       
    SINT32  Longitude;      
    SINT32  Height;     
    SINT32  GMTOffset;      
    UINT8   SatelliteCnt;  //Minimum Tracking satellite #   1       Default = 3
};

//3.1.3.1   GPS位置变化通知
struct T_GpsLocationInfo
{
    SINT32  Latitude;       
    SINT32  Longitude;      
    SINT32  Height;     
    SINT32  GMTOffset;      
    UINT8   TrackedSatellites;  //Minimum Tracking satellite #   1       Default = 3
    UINT8   VisibleSatellites;
};

//3.1.4.2   GPS信息查询应答
struct T_GpsInfo
{
	UINT32 Latitude;
	UINT32 Longitude;
	UINT32 Height;
	UINT8 TrackedSatellites;
	UINT8 VisibleSatellites;
	UINT8 Month;
	UINT8 Day;
	UINT16 Year;
	UINT8 Hour;
	UINT8 Minute;
	UINT16 Second;
};

//3.2.1   下载基站软件请求
struct T_DLBtsSWReq
{
    SINT32  m_FTPserverIP;
    UINT16  m_FTPserverPort;  
    UINT8   m_UserNameLen;
    SINT8   m_UserName[USER_NAME_LEN];
    UINT8   m_FTPPasswordLen;
    SINT8   m_FTPPassword[USER_PASSWORD_LEN];
    UINT8   m_FtpDirLen;
    SINT8   m_FtpDir[FILE_DIRECTORY_LEN];
    UINT8   m_SWType;   //软件类型; 0;  1 -- b;  2 -- c;
    SINT8   m_SWVer[FILEVER_STR_LEN];    //软件版本; 同软件类型的组合构造出bts软件文件名
                                            //文件名如 FILENAME = BTS.1.12.1.1.BIN
};

  //3.3.5   版本加载结果，失败后带原因
struct T_DLBtsRusult  //每个CPU的状态
{
    UINT16  Result;   
    UINT16  ErrorCause;     
};

 //3.3.5   基站开工通知   
struct T_BtsCpuWorkStatus  //每个CPU的状态
{
    //lijie add for 1303   
    UINT8 OAM_DSP1;    //0 BOOT FAIL        1 - BOOT SUCCESS
    UINT8 OAM_DSP2;    //0 BOOT FAIL        1 - BOOT SUCCESS
    UINT8 OAM_DSP3;    //0 BOOT FAIL        1 - BOOT SUCCESS
    UINT8 OAM_DSP4;    //0 BOOT FAIL        1 - BOOT SUCCESS
    UINT8 OAM_FPGA;    //0 BOOT FAIL        1 - BOOT SUCCESS
};

 //3.3.5   单个DSP复位   
struct T_BtsDspReset  //每个CPU的状态
{
    UINT16  DspNo;   
    UINT16  ResetMode;     
};
  //3.3.5   单个DSP复位结果   
struct T_BtsDspResetEResult  //每个CPU的状态
{
    UINT16  DspNo;   
    UINT16  Result;     
};
#endif

#if  defined(__CPU_LTE_SMALLCELL__)||defined(__CPU_LTE_SMALLCELL_PACK__)
extern int interpro_socket_init(void);
extern int  interpro_send_msg(char *MsgContent, UINT32 u32MsgLen );
extern int bsp_fpga_alarm_init(void);
//extern s32 bsp_interpro_send_alarm_msg(u16 alarm_type,u16 alarm_code_index,u8 alarm_severitys,s8 *msg_alarm, u32 msg_alarm_Len,u16 entity_index);
//extern s32 bsp_interpro_clear_alarm(u16 alarm_type,u16 alarm_code_index,u8 alarm_severitys,s8 *msg_alarm, u32 msg_alarm_Len,u16 entity_index);
//extern void  interpro_send_gps_info(void)


#define FILEVER_STR_LEN      16 // bts."111.222.333.444".bin
#define USER_NAME_LEN        10
#define USER_PASSWORD_LEN      10
#define FILE_DIRECTORY_LEN      50
#define FILE_NAME_LEN  60

/*BEGIN: added by zs 20131106,  PN: BTS1303 公共模块同业务模块间的消息ID定义*/
#define M_L3_PLAT_GENERAL_SETTING_REQ  0x5000
#define M_PLAT_L3_GENERAL_SETTING_RSP  0x5001

#define M_L3_PLAT_GPS_DATA_CFG_REQ   0x5002
#define M_PLAT_L3_GPS_DATA_CFG_RSP   0x5003
#define M_PLAT_L3_SYNSRC_CHANGE_NOTIFY 0x5004;
#define M_PLAT_L3_SYNSRC_STATUS_CHANGE_NOTIFY 0x5005;
#define M_L3_PLAT_GPS_INFO_QUERY_REQ   0x5006
#define M_PLAT_L3_GPS_INFO_QUERY_RSP   0x5007

#define M_L3_PLAT_HEARTBEAT   0x5008

#define M_L3_PLAT_DL_BTS_SW_REQ  0x5009
#define M_PLAT_L3_DL_BTS_SW_RSP   0x500a
#define M_PLAT_L3_DL_BTS_SW_RESULT_NOTIFY   0x500b
#define M_L3_PLAT_SWITCH_BTS_SW_REQ   0x500c

#define M_L3_PLAT_RESET_REQ   0x500d
#define M_PLAT_L3_RESET_RSP   0x500e

#define M_L3_PLAT_CPU_WORK_STATUS_NOTIFY   0x5011
#define M_PLAT_L3_CPU_WORK_STATUS_RSP   0x5012

#define M_PLAT_L3_BOOT_TIMES_NOTIFY  0x5013

#define M_L3_PLAT_QUERY_SFP_INFO   0x5014
#define M_PLAT_L3_QUERY_SFP_INFO_RSP   0x5015

#define M_L3_PLAT_CFG_MB_TEMP_THRESHOLD_REQ 0x5016
#define M_PLAT_L3_CFG_MB_TEMP_THRESHOLD_RSP 0x5017

#define M_L3_PLAT_QUERY_MB_TEMP_THRESHOLD_REQ 0X5018
#define M_PLAT_L3_QUERY_MB_TEMP_THRESHOLD_RSP 0X5019

#define M_L3_PLAT_QUERY_PLAT_VER_REQ 0x501a
#define M_PLAT_L3_QUERY_PLAT_VER_RSP 0x501b

#define M_PLAT_L3_ALARM_NOTIFY     0x501c
#define M_L3_PLAT_QUERY_ALARM_REQ  0x501d
#define M_PLAT_L3_QUERY_ALARM_RSP  0x501e
#define M_PLAT_L3_RESET_NOTIFY      0x501f
#define M_L3_PLAT_POLL              0x5020
#define M_L3_PLAT_POLL_ACK          0x5021

#define SYNSRC_GPS                        0
#define SYNSRC_LOCAL                      1
#define SYNSRC_BD                         2
#define SYNSRC_FIBER                      3
#define SYNSRC_1588                       4
#define SYNSRC_485                        5
#define SYNSRC_E1                         6
#define SYNSRC_MIX_GPS                    7
#define SYNSRC_MIX_BD                     8
#define SYNSRC_1PPS                       9
#define SYNSRC_1PPS_TOD                  10

struct T_MsgHeader
{   
    UINT16  MsgId;     //00：BTS本地同步,01：GPS同步,02：E1同步,03：光口同步,04：1588同步,05：485同步
    UINT16  transID;   //         
 };

//3.1.1.1   同步源配置请求
struct T_L1GenCfgEle
{   
    UINT16  SyncSrc;     //00：BTS本地同步,01：GPS同步,02：E1同步,03：光口同步,04：1588同步,05：485同步
    UINT16  GpsOffset;   //         
    UINT16  AntennaMask; //  0-disable, 1-enable       LSB is antenna 0
};

//3.1.2.1   GPS数据配置请求
struct T_GpsDataCfgEle
{
    SINT32  Latitude;       
    SINT32  Longitude;      
    SINT32  Height;     
    SINT32  GMTOffset;      
    UINT8   SatelliteCnt;  //Minimum Tracking satellite #   1       Default = 3
};

//3.1.3.1   GPS位置变化通知
struct T_GpsLocationInfo
{
    SINT32  Latitude;       
    SINT32  Longitude;      
    SINT32  Height;     
    SINT32  GMTOffset;      
    UINT8   TrackedSatellites;  //Minimum Tracking satellite #   1       Default = 3
    UINT8   VisibleSatellites;
};

//3.1.4.2   GPS信息查询应答
struct T_GpsInfo
{
	UINT32 Latitude;
	UINT32 Longitude;
	UINT32 Height;
	UINT8 TrackedSatellites;
	UINT8 VisibleSatellites;
	UINT8 Month;
	UINT8 Day;
	UINT16 Year;
	UINT8 Hour;
	UINT8 Minute;
	UINT16 Second;
};

//3.2.1   下载基站软件请求
struct T_DLBtsSWReq
{
    SINT32  m_FTPserverIP;
    UINT16  m_FTPserverPort;  
    UINT8   m_UserNameLen;
    SINT8   m_UserName[USER_NAME_LEN];
    UINT8   m_FTPPasswordLen;
    SINT8   m_FTPPassword[USER_PASSWORD_LEN];
    UINT8   m_FtpDirLen;
    SINT8   m_FtpDir[FILE_DIRECTORY_LEN];
    UINT8   m_SWType;   //软件类型; 0;  1 -- b;  2 -- c;
    SINT8   m_SWVer[FILEVER_STR_LEN];    //软件版本; 同软件类型的组合构造出bts软件文件名
                                            //文件名如 FILENAME = BTS.1.12.1.1.BIN
};

  //3.2.2   版本查询
struct T_PfmVerRsp  //每个CPU的状态
{
     SINT8   m_PfmSWVer[FILEVER_STR_LEN];   ;   
};
  //3.3.5   版本加载结果，失败后带原因
struct T_DLBtsRusult  //每个CPU的状态
{
    UINT16  Result;   
    UINT16  ErrorCause;     
};

 //3.3.5   基站开工通知   
struct T_BtsCpuWorkStatus  //每个CPU的状态
{
    //lijie add for 1303   
    UINT8 OAM_DSP1;    //0 BOOT FAIL        1 - BOOT SUCCESS
    UINT8 OAM_DSP2;    //0 BOOT FAIL        1 - BOOT SUCCESS
    UINT8 OAM_DSP3;    //0 BOOT FAIL        1 - BOOT SUCCESS
    UINT8 OAM_DSP4;    //0 BOOT FAIL        1 - BOOT SUCCESS
    UINT8 OAM_FPGA;    //0 BOOT FAIL        1 - BOOT SUCCESS
};

 //3.3.5   单个DSP复位   
struct T_BtsDspReset  //每个CPU的状态
{
    UINT16  Reset_reason;
    UINT16  ResetMode;  
    UINT16  DspNo;     
};
  //3.3.5   单个DSP复位结果   
struct T_BtsDspResetEResult  //每个CPU的状态
{
    UINT16  DspNo;   
    UINT16  Result;     
};

#if 1
//fpga alarm reg
#define  BSP_FPAG_REG_ENV_STATUS           2
#define  BSP_FPAG_REG_LOF_STATUS           4
#define  BSP_FPAG_REG_LGHT_STATUS           28

//PPC_DD
#define  BSP_ALM_SYN_SRC_CHANGE_ALARM           0x1301
#define  BSP_ALM_GPS_LOST_ALARM                  0x1302
#define  BSP_ALM_AFC_STATUS_CHANGE_ALARM        0x1303
#define  BSP_ALM_AFC_HOLDOVER_TIMEOUT_ALARM     0x1304
#define  BSP_ALM_AFC_ABNORMAL_ALARM              0x1305
#define  BSP_ALM_FAN_UNCONTROL_ALARM             0x1306
#define  BSP_ALM_ETHSW_UNCONTROL_ALARM           0x1307
#define  BSP_ALM_RTC_UNCONTROL_ALARM             0x1308
#define  BSP_ALM_E2PROM_UNCONTROL_ALARM          0x1309
#define  BSP_ALM_TEMP_SENSOR_UNCONTROL_ALARM     0x130A
#define  BSP_ALM_FIRMWARE_UPDATE_FAIL_ALARM      0x130B
#define  BSP_ALM_FIRMWARE_UPDATE_ALARM           0x130C
#define  BSP_ALM_MBD_OVER_TEMP_ALARM              0x130E
#define  BSP_ALM_PPC_OVER_TEMP_ALARM             0x130F
#define  BSP_ALM_PAU_OVER_TEMP_ALARM             0x1401
#define  BSP_ALM_POWERSTUS_ABNORMAL_ALARM        0x1312
#define  BSP_ALM_I2C_ACK_ALARM        			0x1315
#define  BSP_ALM_OVER_TEMP_ALARM           		0x1316
#define  BSP_ALM_OCXO_UNCONTROL_ALARM           	0x1317   //ocxo失控
#define  BSP_ALM_OCXO_INVALID_ALARM           	0x1318  //ocxo停震
#define  BSP_ALM_NANDFLASH_FILLED_ALARM          0x1319  //nandflash存满
#define  BSP_ALM_FIRMWARE_BACKUP_ALARM           0x131A  //固件启动源告警



//CPLD alarm
#define  BSP_ALM_ID_PLL2_ERROR               0x1201
#define  BSP_ALM_ID_1PPS_ERROR               0x1202

//FPGA alarm
#define  BSP_ALM_ID_TDD_10ms_Los      0x0a2d
#define  BSP_ALM_ID_DSPn_AIFm_LOF     0x0a2e
#define  BSP_ALM_ID_DSPn_AIFm_LOS     0x0a2f
#define  BSP_ALM_ID_PLL_LOCKED        0x0a30
#define  BSP_ALM_ID_LITH_PRT          0x0a31
#define  BSP_ALM_ID_OVER_TEMP         0x0a32
#define  BSP_ALM_ID_LO_LOCK           0x0a33

//告警类型
#define BSP_ALM_TYPE_FPGA             0X0a
#define BSP_ALM_TYPE_CPLD             0x12
#define BSP_ALM_TYPE_CPU_PF           0x13

//告警级别
#define BSP_ALM_CLASS_CRITICAL       1
#define BSP_ALM_CLASS_MAJOR          2
#define BSP_ALM_CLASS_MINOR          3
#define BSP_ALM_CLASS_INFO           4

#define  BSP_ALM_FLAG_CLEAR           0    //清除告警
#define  BSP_ALM_FLAG_SET             1    //设置告警
#endif

/*版本下载错误码*/
#define ERR_FTP_GET_BTS_FILE	0X0418	//FTP获取文件失败
#define ERR_BTS_FILE_INFLATE	0X0419	//解压缩失败 
#define ERR_BTS_FLASH_FULL  0X041a	//FLASH已满错误
#define ERR_BTS_CREATE_THREAD	0X041b	//创建加载线程失败


#pragma pack(1)
typedef struct   //每个CPU的状态
{
    u32  seq_number;  //告警序列号
	u8   flag;        //告警标示 0-  clear   1-  set 
	
	u16  year;		//时间
    u8   month;
    u8   day;
    u8   hour;		
    u8   minute;		
    u8   second;
	
	u16  type;   //告警实体类型
	u16  entity_index;//告警实体索引号
	u16  code_index; //告警ID
	u8  severity;  //告警等级 1 - critical   2 - major   3 - minor   4 - informational
    u16  info_length;	//告警内容长度
}t_alarm_messge_head;

#endif


#define    SYNC_SOURCE_GPS      		(0)
#define    SYNC_SOURCE_LOCAL   			(1)
#define    SYNC_SOURCE_BD       		(2)
#define    SYNC_SOURCE_1PPS_TOD 		(5)
#define    SYNC_SOURCE_CASCADE 			(3)
#define    SYNC_SOURCE_1588      		(4)
#define    SYNC_SOURCE_ES        		(6)

#define SYNSRC_GPS_SHIFT				(0)
#define SYNSRC_BD_SHIFT					(1)
#define SYNSRC_ES_SHIFT					(4)
#define SYNSRC_1PPS_TOD_SHIFT			(12)
#define SYNSRC_CASCADE_SHIFT			(18)
#define SYNSRC_1588_SHIFT				(24)
#define SYNSRC_LOCAL_SHIFT				(30)

#define SYNSRC_GPS_MASK					(1 << SYNSRC_GPS_SHIFT)
#define SYNSRC_BD_MASK					(1 << SYNSRC_BD_SHIFT)
#define SYNSRC_1PPS_TOD_MASK			(1 << SYNSRC_1PPS_TOD_SHIFT)
#define SYNSRC_CASCADE_MASK				(1 << SYNSRC_CASCADE_SHIFT)
#define SYNSRC_1588_MASK				(1 << SYNSRC_1588_SHIFT)
#define SYNSRC_LOCAL_MASK				(1 << SYNSRC_LOCAL_SHIFT)
#define SYNSRC_ES_MASK                               (1 << SYNSRC_ES_SHIFT)

#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
extern pthread_mutex_t g_mp_synsrc_switch;
extern pthread_cond_t g_cond_synsrc_switch;
#endif /*ifdef __CPU_LTE_CENTERSTATION__*/
#endif
/******************************* 头文件结束 ********************************/

