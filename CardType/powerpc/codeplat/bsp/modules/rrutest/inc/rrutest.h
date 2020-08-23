#ifndef	__RRUTEST_H
#define	__RRUTEST_H

#include "bsp_types.h"
#include "bsp_boardtest_ext.h"

#define _RRU_NEW_INTERFACE_
#define RF_CHN_COUNT 8
#define FIBER_PORT_COUNT 2
#define RFCHANNEL 8

cmd_t rrutest_version_query(cmd_t cmd);
cmd_t rrutest_rfstatus_query(cmd_t cmd);
cmd_t rrutest_rrustatus_query(cmd_t cmd);
cmd_t rrutest_fiberstatus_query(cmd_t cmd);
cmd_t rrutest_para_query(cmd_t cmd);
cmd_t rrutest_systime_config(cmd_t cmd);
void rrutest_threshold_config(u16 vswr, s8 board, s8 rfchn);
cmd_t rrutest_antenna_config(cmd_t cmd);
cmd_t rrutest_power_cal(cmd_t cmd);
cmd_t rrutest_cell_config(cmd_t cmd);
cmd_t rrutest_reset(cmd_t cmd);
cmd_t rrutest_hwparam_query(cmd_t cmd);
cmd_t rrutest_verdownload_req(cmd_t cmd);
cmd_t rrutest_veractivate_req(cmd_t cmd);
cmd_t rrutest_fiberdelay_config(cmd_t cmd);
cmd_t rrutest_serialclose_req(cmd_t cmd);
cmd_t rrutest_heartbeat_to_pc(cmd_t cmd);
cmd_t rrutest_almquery_req(cmd_t cmd);

typedef struct {
	u8	rruID;
	u8	bbuID;
	u8	type[4];
	u8	len[4];
	u8	rsv[2];
	u8	transid[2];
	u8	data[1024];
	}rru_test_msg;


typedef struct _RRUinfoT{
	u8  rruID;
	u8  u8ChnSetupReason;  
	u32 u32Warncode;     
	u8  au8pkg_ver[4];
	u8  au8app_ver[4];
	u8  au8fpga_ver[4];
	u32 u32masterboard_ver; 
	u32 u32slaveboard_ver;  
	u32 u32pa_masterboard_ver;
	u32 u32pa_slaveboard_ver; 
	u8  au8masterboard_sn[32];
	u8  au8slaveboard_sn[32];  
	u8  au8pa_masterboard_sn[32]; 
	u8  au8pa_slaveboard_sn[32]; 
	u16 u16tx_max_pow;  
	u16 u16rx_max_pow; 
	s16 s16aiq_rx_nom; 
	s16 s16aiq_tx_nom;  
	u16 u16FreqBand;  
	u16 u16AntNum;   
	u16 u16ProductType; 
}__attribute__((packed))RruInformationT;


typedef struct _ftpReqT {
	unsigned int type;
	unsigned int optype;
	char filename[200];
	char lpath[200];
	char rpath[200];
}ftpReqT;

typedef struct _ftpResultT {
	unsigned int type;
	unsigned int optype;
	char filename[200];
	char lpath[200];
	char rpath[200];
	int success;
}ftpResultT;


typedef struct _ChnSetupT {
	u8  u8ChnSetupReason;   //通道建立原因 a	
	                        //0：BBU侧发起的软件重启动
                            //1：RRU侧发起的软件重启动
                            //2：上电
                            //其余保留
	u32 u32Warncode;        //导致RRU侧发起软件重启动的告警码
	/**********软、硬件版本信息**********/
	u8  au8pkg_ver[4];      //软件包大版本
	u8  au8os_ver[4];       //操作系统软件版本
	u8  au8dts_ver[4];      //DTS文件
	u8  au8uboot_ver[4];    //Uboot版本
	u8  au8app_ver[4];      //文件系统+应用软件版本
	u8  au8fpga_ver[4];     //FPGA软件版本
	u32 u32masterboard_ver; //硬件主板版本号
	u32 u32slaveboard_ver;  //硬件从板版本号
	u32 u32pa_masterboard_ver;  //硬件功放主板版本号
	u32 u32pa_slaveboard_ver;   //硬件功放从板版本号
	u8  au8masterboard_sn[32];  //硬件主板序列号
	u8  au8slaveboard_sn[32];   //硬件从板序列号
	u8  au8pa_masterboard_sn[32];   //硬件功放主板序列号
	u8  au8pa_slaveboard_sn[32];    //硬件功放从板序列号
	u16 u16tx_max_pow;      //通道最大发射功率，单位:0.1dBm
	s16 s16aiq_tx_nom;      //下行定标值
	u16 u16rx_max_pow;      //通道最大接收功率，单位:0.1dBm
	s16 s16aiq_rx_nom;      //上行定标值
	u16 u16FreqBand;        //频段信息，枚举值:ENUM_RRU_FREQBAND_INFO
	u16 u16AntNum;          //天线收发状态，上报天线可配置掩码给BBU
	u16 u16ProductType;     //RRU产品类型，枚举值:ENUM_RRU_PRODUCT_TYPE
}__attribute__((packed))chnSetupT;/*_RRU_NEW_INTERFACE_*/


/*硬件参数查询响应*/
typedef struct _HWParamRsp {
	/*===============新EEPROM表信息=================*/
	u8  u8Freq_IO;          //高低本振。1-高本振；2-低本振
	u16 u16Freq_Band_Val;   //频段值，是数值，单位: MHz
	u32 u32Freq_min;        //频点最小值，单位: KHz
	u32 u32Freq_max;        //频点最大值，单位: KHz
	u8  u8RF_total_chn;     //射频通道总数，取值: 4或8
	u8  u8RF_Freq_K;        //频点配置字增益
    u8  u8RF_Freq_X;        //分频比系数，hmc1044
    u8  au8RRU_TYPE[16];             /*产品型号，XWR6404-18*/
    u8  au8RRU_ProductTYPE[16];      /*产品类型，RRU4E30_CF*/
}__attribute__((packed))HWParamRsp;

typedef struct _PS_NormT
{
	unsigned char PS_Norm_X[2];//PS_Norm_X[i]	2
	unsigned char PS_Norm_Y[2];//PS_Norm_Y[i]	2
}PS_NormT;

typedef struct _RF_CfgT
{
	unsigned char Start_Freq_Index[4];
	unsigned char AntennaPower[2];
	unsigned char RxSensitivity[2];
	unsigned char Cable_Loss[2];
	unsigned char PS_Loss[2];

	PS_NormT ps_norm[8];

	unsigned char SCG_Mask[2];      /*未用到*/
	unsigned char Calibration_SCGMask[2];   /**/
	unsigned char BBU1303_FreqIndex[4];
	unsigned char BBU1303AntennaPower[2];
	unsigned char rsv[2];
}RF_CfgT;

typedef struct _RF_CFG_Rsp
{
    unsigned char result[2];
}RF_CFG_Rsp;

typedef struct _chnSetupCfgReqT {
	
//系统时间配置
	unsigned char second;//秒	0-59	Unsigned Char	1 BYTE
	unsigned char minute;//分	0-60	Unsigned Char	1 BYTE
	unsigned char hour;//时	0-23	Unsigned Char	1 BYTE
	unsigned char day;//日	1-31	Unsigned Char	1 BYTE
	unsigned char month;//月	1-12	Unsigned Char	1 BYTE
	unsigned char year[2];//年	1997-2099	Unsigned Short	2 BYTE
//运行模式
	unsigned char workmode;//	RRU工作制式	0：LTE 1：McWill	Unsigned char	1 BYTE
	unsigned char rrumode;//RRU操作模式	0：RRU正常运行1：测试状态	Unsigned char	1 BYTE
	unsigned char irmode;//Ir接口工作模式	1：普通模式2：级连模式	Unsigned char	1 BYTE
#ifdef _RRU_NEW_INTERFACE_
    unsigned char au8Reserved[2];   /*将时隙配置相关字段变为保留*/
#else
	unsigned char tssum;//总时隙个数	Mcwill和LTE都需要	unsigned char	1 BYTE
	unsigned char dlts;//下行时隙比	Mcwill和LTE都需要	unsigned char	1 BYTE
#endif /*_RRU_NEW_INTERFACE_*/
//FTP信息
	unsigned char bbuip[4];//BBU侧FTP服务器IP地址a 		Unsigned Char数组	4 BTYE
	char ftpusername[20];//FTP用户名		字符串	20 BYTE
	char ftppassword[20];//FTP密码		字符串	20 BYTE
	char filename[20];
	unsigned char rsv0[80];//reserved			100 BYTE
//软件版本核对结果
	unsigned char ver_comp_res;//软件包大版本	0：版本相同1：版本不同	unsigned char	1BYTE
	unsigned char rsv1[5];//Reserved	预留用作小版本核对结果		5BYTE
//告警门限配置
	char board_temp_threshold;//主、从板温度门限	取整	signed char	  1
	char rfchn_temp_threshold;//射频通道温度门限	取整	signed char	  1
	char vswr[2];//驻波比门限	单位： 0.1	unsigned short	2 BYTE
	char rsv2[6];//reserved			6 BYTE
#ifndef _RRU_NEW_INTERFACE_
//射频配置
	unsigned char N_Cell;
	unsigned char rsv3[3];
	RF_CfgT rf_cfg[4];
#endif /*_RRU_NEW_INTERFACE_*/
#if 0
	unsigned char scgmask[4];//20M_SCG_Mask	20M SCG 掩码，为每个5M的SCG掩码，1为打开，0为禁止		4 BYTE
	unsigned char freq[4];//20MFreqIndex	20M 中心频点//单位50K;//start Freq Index use base of band ID passing from AUX,//After AUX receive this, it will do floor((start Freq index-PLLmin)/PLLstep), and look up the PLL table to do the configuration		4 BYTE
	unsigned char pw[2];//20M AntennaPower	20M基站的真实发射功率。		2 BYTE
#endif
}chnSetupCfgReqT;

typedef struct _chnSetupCfgRspT {
	
	unsigned char result[2];//Result	0 succ  1 fail		2BYTE
	unsigned char allresults[4];//返回结果	每比特表示一种配置的返回结果//0：成功//1：失败//bit0：RRU操作模式bit1：Ir接口模式bit2：门限配置bit3：频点配置bit4：天线功率配置	Unsigned long	4 BYTE

}chnSetupCfgRspT;

typedef struct _SWUpdateResultIndT {
	unsigned char result;//大版本	0：成功1：失败	unsigned char	1 BYTE
}SWUpdateResultIndT;

typedef struct _SWQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char bigPkgVer[4];//大版本		Unsigned long	4 BYTE
	unsigned char osver[4];//操作系统软件版本		Unsigned long	4 BYTE
	unsigned char dtsver[4];//DTS文件		Unsigned long	4 BYTE
	unsigned char ubootver[4];//Uboot版本		Unsigned long	4 BYTE
	unsigned char appver[4];//文件系统+应用软件版本		Unsigned long	4 BYTE
	unsigned char fpgaver[4];//FPGA软件版本		Unsigned long	 4 BYTE
}SWQryRspT;

typedef struct _HWQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char mainboardver[4];//	硬件主板版本号		unsigned short	 4 BYTE
	unsigned char slaveboardver[4];//	硬件从板版本号		unsigned short	 4 BYTE
	unsigned char pamainboardver[4];//	硬件功放主板版本号		unsigned short	 4 BYTE
	unsigned char paslaveboardver[4];//	硬件功放从板版本号		unsigned short	 4 BYTE
	unsigned char mainboardsn[32];//	硬件主板序列号		字符串	32 BYTE
	unsigned char slaveboardsn[32];//	硬件从板序列号		字符串	32 BYTE
	unsigned char pamainboardsn[32];//	硬件功放主板序列号		字符串	32 BYTE
	unsigned char paslaveboardsn[32];//	硬件功放从板序列号		字符串	32 BYTE
}HWQryRspT;

typedef struct _VersionDownloadReqT {
	char filename[20];//	软件包文件名		Unsigned char数组	20BYTE
}VersionDownloadReqT;

typedef struct _VersionDownloadRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
}VersionDownloadRspT;

typedef struct _VersionDownloadResIndT {
	unsigned char result;//	大版本	0：成功1：失败	unsigned char	1 BYTE
}VersionDownloadResIndT;

typedef struct _VersionActivateRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char reason[2];//	失败原因	1：要激活的版本不存在2：文件被破坏3：软件版本与硬件版本不匹配4：激活不成功5：其他原因	Unsigned short	2 BYTE
}VersionActivateRspT;

typedef struct _SwrStateQryReqT {
	char rfchn;//射频通道号	0~射频通道总数-1： 表示查询指定单通道；射频通道总数：查询所有通道	unsigned char	1 BYTE
}SwrStateQryReqT;

typedef struct _SwrStateQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char rfchn;//射频通道号	0~射频通道总数-1： 表示查询指定单通道；射频通道总数：查询所有通道	unsigned char	1 BYTE
	unsigned char swrval[RF_CHN_COUNT][2];//通道驻波比	单位： 0.1	signed short	2 BYTE
} SwrStateQryRspT;

typedef struct _AlarmThresholdQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char swr[2];//驻波比门限	单位：0.1	unsigned short	2 BYTE
	char board_temp_threshold;//主、从板温度门限	取整	signed char	1 BYTE
	char rfchn_temp_threshold;//射频通道温度门限	取整	signed char	1 BYTE
	//reserved			2 BYTE
	//reserved			4 BYTE
	char rsv2[6];
}AlarmThresholdQryRspT;

/*参数查询请求接口  _RRU_NEW_INTERFACE_*/
typedef struct _ParameterQryRspT {
	u16 u16result;          //Result	0-succ 1-fail
	u16 u16vswrThreshold;   //驻波比门限	单位：0.1
	s8  s8board_temp_threshold; //主、从板温度门限	取整
	s8  s8rfchn_temp_threshold; //射频通道温度门限	取整
	u8  au8Reserved[6];        //
	/********* McWiLL 时隙配比 *********/
    u8  u8TsTotalNum;       //总时隙数，默认为:8
    u8  u8DLTsNum;          //下行时隙比
}ParameterQryRspT;

typedef struct _RFStateQryReqT {
	unsigned char rfchn;//	射频通道号	0~射频通道总数-1： 表示查询指定单通道；射频通道总数：查询所有通道	unsigned char	1 BYTE
}RFStateQryReqT;

typedef struct _RFChnStateT {
#ifdef _RRU_NEW_INTERFACE_
    u8  u8dl_antenna_state;     //天线状态	0：关闭1：开启
	u8  u8ul_antenna_state;     //天线状态	0：关闭1：开启
	s8  s8chn_temp;             //通道温度	取整
	s16 s16txpw;                //发射功率	单位：0.1dBm
	u8  u8txgain;               //发射增益	单位：0.1dB
	u8  u8rxgain;               //接收增益	单位：0.1dB
	s16 s16rxpw;                //接收功率  单位: 0.1dBm
	s16 s16vswr;                //驻波比    单位: 0.1
	u8  u8ResultGetTxpw;        //下行功率读取结果: 0-SUCC;1-FAIL
	u8  u8ResultGetRxpw;        //上行功率读取结果: 0-SUCC;1-FAIL
	u8  u8ResultGetVswr;        //驻波比读取结果:   0-SUCC;1-FAIL	
	u8  u8Reserved;             //Reserved
#else
	unsigned char dl_antenna_state;//天线状态	0：关闭1：开启	unsigned char	1 BYTE
	unsigned char ul_antenna_state;//天线状态	0：关闭1：开启	unsigned char	1 BYTE	
	char chn_temp;//通道温度	取整	Signed char	1 BYTE
	unsigned char txpw[2];//发射功率	单位：0.1dBm	Signed short	2 BYTE
	unsigned char txgain;//发射增益	dB	unsigned char	1 BYTE
	unsigned char rxgain;//接收增益	dB	unsigned char	1 BYTE
	unsigned char rsv[8];//Reserveda		Unsigned char	8 BYTE
#endif /*_RRU_NEW_INTERFACE_*/
#ifdef _RRU_NEW_INTERFACE_
}__attribute__((packed))RFChnStateT;
#else
}RFChnStateT;
#endif  /*_RRU_NEW_INTERFACE_*/

typedef struct _RFStateQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char rfchn;
	unsigned char rsv;
	RFChnStateT rfchnstate[RF_CHN_COUNT];
}RFStateQryRspT;

typedef struct _RruStateQryRspT {
	unsigned char result[2];    //	Result	0 succ1 fail		2BYTE
	unsigned char rflofreq[4];  //	射频本振频率，根据实际频点变化	单位：100KHz	unsigned long	4 BYTE
	unsigned char rflostate;    //	射频本振状态	0：锁定1：失锁	unsigned char	1 BYTE
	unsigned char clkstate;     //	时钟状态	0：同步1：失步	unsigned char	1 BYTE
	unsigned char irifmode;     //	Ir接口工作模式	1：普通模式2：级连模式	unsigned char	1 BYTE
#ifndef _RRU_NEW_INTERFACE_ /*该字段已经被删除*/
	unsigned char totalts;      //	总时隙个数		unsigned char	1 BYTE
	unsigned char dlts;         //	下行时隙比		unsigned char	1 BYTE
#endif  /*_RRU_NEW_INTERFACE_*/
	unsigned char rsv[2];
	unsigned char workmode;     //	RRU运行状态a	0：未运营1：测试中2：运营中3：版本升级中4：故障	unsigned char	1 BYTE
	//温度
	char mainboard_temp;        //	主板温度	取整	signed char	1 BYTE
	char slaveboard_temp;       //	从板温度	取整	signed char	1 BYTE
	//系统时间
	unsigned char sec;          //	秒	0-59	Unsigned Char	1 BYTE
	unsigned char min;          //	分	0-60	Unsigned Char	1 BYTE
	unsigned char hour;         //	时	0-23	Unsigned Char	1 BYTE
	unsigned char day;          //	日	1-31	Unsigned Char	1 BYTE
	unsigned char mon;          //	月	1-12	Unsigned Char	1 BYTE
	unsigned char reserved[3];
	unsigned char year[2];      //	年	1997-2099	Unsigned Short	2 BYTE
	unsigned char workingtime[4];//	rru运行时间	秒	unsigned long	4 BYTE
}RruStateQryRspT;

typedef struct _FiberPortStateQryReqT {
unsigned char fiberport;//	光口号	0~射频通道总数-1： 表示查询指定光口；光口总数：查询所有光口	unsigned char	1 BYTE
}FiberPortStateQryReqT;

typedef struct _FiberPortStateT {
	unsigned char rxpw[2];//	收功率	单位0.1uW	Unsigned short	2BYTE
	unsigned char txpw[2];//	发功率	单位0.1 uW	Unsigned short	2BYTE
	unsigned char ineffect;//	在位信息	1：在位//	0：不在位	Unsigned char	1BYTE
	unsigned char rev1;
	char vendor[16];//	光模块厂商	ASCII	字符串	16BYTE
	char rate[2];//	光模块传输bit速率	单位 Mbit/s Unsigned short	2BYTE
	char temperature;//	温度	单位 C摄氏度	Signed char 1BYTE
	unsigned char rev2;
	unsigned char voltage[2];//	电压	单位 mV Unsigned short	2BYTE
	unsigned char current[2];//	电流	单位 mA Unsigned short	2BYTE
	unsigned char rsv[4];
}FiberPortStateT;

typedef struct _FiberPortStateQryRspT {
	unsigned char result[2];
	unsigned char fiberport;//	光口号	0~射频通道总数-1： 表示查询指定光口；光口总数：查询所有光口	unsigned char	1 BYTE
	unsigned char rsv;
	FiberPortStateT fpstat[FIBER_PORT_COUNT];
}FiberPortStateQryRspT;

/*该消息已经被删除 _RRU_NEW_INTERFACE_ */
typedef struct _TimeSlotCfgReqT {
	unsigned char tssum;//总时隙个数		unsigned char	1 BYTE
	unsigned char dlts;//下行时隙比		unsigned char	1 BYTE
}TimeSlotCfgReqT;

typedef struct _TimeSlotCfgRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char ret;//	返回结果	0：成功1：失败	unsigned char	1 BYTE
}TimeSlotCfgRspT;

typedef struct _TimeCfgReqT {
	unsigned char second;//秒	0-59	Unsigned Char	1 BYTE
	unsigned char minute;//分	0-60	Unsigned Char	1 BYTE
	unsigned char hour;//时	0-23	Unsigned Char	1 BYTE
	unsigned char day;//日	1-31	Unsigned Char	1 BYTE
	unsigned char month;//月	1-12	Unsigned Char	1 BYTE
	unsigned char year[2];//年	1997-2099	Unsigned Short	2 BYTE
}TimeCfgReqT;

typedef struct _TimeCfgRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char ret;//	返回结果	0：成功1：失败	unsigned char	1 BYTE
}TimeCfgRspT;

typedef struct _AlarmThresholdCfgReqT {
	char swr[2];//驻波比门限	单位： 0.1	unsigned short	2 BYTE
	char board_temp_threshold;//主、从板温度门限	取整	signed char	  1
	char rfchn_temp_threshold;//射频通道温度门限	取整	signed char	  1
	char rsv2[6];//reserved			6 BYTE
}AlarmThresholdCfgReqT;

typedef struct _AlarmThresholdCfgRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char ret;//	返回结果	0：成功1：失败	unsigned char	1 BYTE	bit0：驻波比门限	bit1：主、从板温度门限	bit2：射频通道温度门限
}AlarmThresholdCfgRspT;

typedef struct _AntennaStateCfgReqT {
	unsigned char dlantennamask;//天线掩码状态	0：关闭天线	1：开启天线	bit7：天线编号7	...	bit0：天线编号0 unsigned char	1 BYTE
	unsigned char ulantennamask;//天线掩码状态	0：关闭天线	1：开启天线	bit7：天线编号7	...	bit0：天线编号0 unsigned char	1 BYTE
	char rsv[3];	//reserved			3 BYTE
}AntennaStateCfgReqT;

typedef struct _AntennaStateCfgRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char ret;//	返回结果	0：成功1：失败	unsigned char	1 BYTE
}AntennaStateCfgRspT;

typedef struct _ResetReqT {
	unsigned char resettype;//	复位类型	0：本级RRU复位	unsigned char	1 BYTE
}ResetReqT;
typedef struct _ResetRspT {
	unsigned char result[2];//	Result	0 succ	1 fail	Unsigned short	2BYTE
}ResetRspT;

typedef struct _AlarmRptT {
	unsigned char sn[4];//Alarm Sequence Number	4	M	0xffffffff	告警序列号
	unsigned char flag;//Flag	1	M	0 告警恢复1 告警产生	告警标示
	unsigned char year[2];//Timestamp - year	2	M		年
	unsigned char mon;//Timestamp - month	1	M		月
	unsigned char day;//Timestamp - day	1	M		日
	unsigned char hour;//Timestamp - hour	1	M		时
	unsigned char minute;//Timestamp - minute	1	M		分
	unsigned char second;//Timestamp - second	1	M		秒
	unsigned char type[2];//Alarm Entity Type	2	M	L3告警0x01 L2告警0x02 MCP告警0x03 AUX告警0x04 ENV告警0x05 PLL告警0x06 RF告警0x07 GPS告警 0x08 L1	 0x09 FPGA  0x0a FEP 0x0b Core9 0x0c RRU 0x0d AIF 0x0e 电池     0x0f	告警实体类型
	unsigned char index[2];//Alarm Entity Indexb	2	M	ALARM_ENTIFY_INDEX0    = 0X0;（默认值）ALARM_ENTIFY_INDEX1    = 0X1;ALARM_ENTIFY_INDEX2    = 0X2;ALARM_ENTIFY_INDEX3    = 0X3;ALARM_ENTIFY_INDEX4    = 0X4;ALARM_ENTIFY_INDEX5    = 0X5;ALARM_ENTIFY_INDEX6    = 0X6;ALARM_ENTIFY_INDEX7    = 0X7;	告警实体索引号
	unsigned char codeidx[2];//Alarm Code Index	2	M	 对应于告警列表中的事件号	告警ID
	unsigned char level;//Alarm Severity	1	M	ALARM_CLASS_CRITICAL   = 0X1;ALARM_CLASS_MAJOR     = 0X2;ALARM_CLASS_MINOR     = 0X3;ALARM_CLASS_INFO       = 0X4..	告警等级
	unsigned char infolen[2];//Alarm Info Length	2	M		告警内容长度
	char info[200];//Alarm Info	V	O	Text to explain the alarm	告警内容
}AlarmRptT;

typedef struct _AlarmQryRspT {
	unsigned char result;// 0无告警1有告警
}AlarmQryRspT;

typedef struct _AntennaOnOffStatT {
	unsigned char DLPrevState;
	unsigned char DLNextState;
	unsigned char ULPrevState;
	unsigned char ULNextState;
}AntennaOnOffStatT;

typedef struct _RFOnOffRptT {
	AntennaOnOffStatT AntennaOnOffStat[8];
}RFOnOffRptT;

typedef struct _CellCfgReqT {
#ifdef _RRU_NEW_INTERFACE_
    u8  u8CfgType;				/*小区配置标识  0-建立; 1-重配; 2-删除*/
	u32 u32CellId; 			    /*本地小区标识*/
	u16 u16pw;				    /*小区功率  单位：0.1dBm*/
	u8  u8AntennaGrpId;			/*天线组号  1－8*/
	u8  u8dn_atennaMask;        /*天线下行开关掩码*/
	u8  u8up_atennaMask;        /*天线上行开关掩码*/
	u8  u8FreqCount;			/*频点数    指当前消息中包含的频点配置IE的数目  default: 1*/
	u8  u8CarrierWaveId; 		/*载波号    暂时未使用*/
	u32 u32CenterFreq; 		    /*中心频率  单位：kHz*/
	u32 u32FreqProperty; 		/*频点的主辅特性    0-主频点; 1-辅频点TD-LTE R10后引入辅频点    暂时未使用*/
	u8  u8tdlte_frm_cfg;		/*TD-LTE上下行子帧比例配置c	bit0-bit3:取值范围0－6*/
	u32 u32EffectSubFrameNo;	/*配置的生效系统子帧号	为10ms帧，取值范围为0－4095	(McWiLL未使用default: 0)*/
	u32 u32CarrierWaveBW;		/*载波带宽  枚举型: ENUM_TYPE_OF_CARRIERWAVEBW*/
	u8  u8SpecialSubFrameCfg;	/*特殊子帧配置		bit0-bit3:取值范围0－8  (McWiLL未使用default: 0)*/
	u8  u8CPLen;				/*循环前缀长度选择	常规CP-0，扩展CP-1  (McWiLL未使用default: 0)*/	
    u16 u16SCGMask[4];          /*SCG掩码   (LTE未使用default: 0x1F)*/
#else
	unsigned char CfgType;				/*小区配置标识		0：建立1：重配2：删除	Unsigned Char	1BYTE*/
	unsigned char CellId[4]; 			/*本地小区标识								Unsigned Long	4BYTE*/
	unsigned char pw[2];				/*小区功率			单位：1/256dbm			Unsigned short	2BYTE*/
	unsigned char AntennaGrpId;			/*天线组号			1－8					Unsigned Char	1BYTE*/
	unsigned char dn_atenna;
	unsigned char up_atenna;
	unsigned char FreqCount;			/*频点数			指当前消息中包含的频点配置IE的数目	Unsigned Char	1BYTE*/
	unsigned char CarrierWaveId; 		/*载波号									Unsigned Char	1BYTE*/
	unsigned char CenterFreq[4]; 		/*中心频率			单位：100kHz			Unsigned Long	4BYTE*/
	unsigned char FreqProperty[4]; 		/*频点的主辅特性	0：主频点1：辅频点TD-LTE R10后引入辅频点	Unsigned Long	4BYTE*/
	unsigned char tdlte_frm_cfg;		/*TD-LTE上下行子帧比例配置c	bit0-bit3:取值范围0－6	Unsigned long	1BYTE*/
	unsigned char EffectSubFrameNo[4];	/*配置的生效系统子帧号	为10ms帧，取值范围为0－4095	Unsigned Long	4BYTE*/
	unsigned char CarrierWaveBW[4];		/*载波带宽			可选择值：5/10/15/20	Unsigned Long	4BYTE*/
	unsigned char SpecialSubFrameCfg;	/*特殊子帧配置		bit0-bit3:取值范围0－8	Unsigned Long	1BYTE*/
	unsigned char CPLen;				/*循环前缀长度选择	常规CP-0，扩展CP-1		Unsigned Char	1BYTE*/	
#endif /*_RRU_NEW_INTERFACE_*/
#ifdef _RRU_NEW_INTERFACE_
}__attribute__((packed))CellCfgReqT;
#else
}CellCfgReqT;
#endif /*_RRU_NEW_INTERFACE_*/

typedef struct _CellCfgRspT {
	unsigned char CellId[4]; 			/*本地小区标识								Unsigned Long	4BYTE*/
	unsigned char result[4];			/*返回结果			0：成功1：失败			Unsigned char	4 BYTE*/
}CellCfgRspT;

/*小区配置查询应答*/
typedef struct tag_STRU_CELL_CFG_QRY_RSP
{
    u16 u16Result;          /*0-SUCC;1-FAIL*/
    CellCfgReqT CellCfgInfo;
}__attribute__((packed))STRU_CELL_CFG_QRY_RSP;



typedef struct _DelayMeasureReqT {
	unsigned char FiberId[2];//光纤号	0：光纤端口1      1：光纤端口2…	unsigned char	1 BYTE
    unsigned char Y1[2];
    unsigned char Y2[2];
}DelayMeasureReqT;

typedef struct _DelayMeasureRspT {
	unsigned char result[2];//光纤号	0：光纤端口1      1：光纤端口2…	unsigned char	1 BYTE
}DelayMeasureRspT;

typedef struct _DelayCfgReqT {
	unsigned char FiberId;//光纤号	0：光纤端口1   1：光纤端口2	Unsigned char	1BYTE
	unsigned char T12[4];//T12	单位：ns	Unsigned Long	4BYTE
	unsigned char T34[4];//T34	单位：ns	Unsigned Long	4BYTE
	unsigned char DL_Offset[4];//帧定时提前量 (DL Offset) a	单位：Tc	Unsigned Long	4BYTE
	unsigned char DL_CalRRU[4];//RRU下行补偿值(DL CalRRU)	单位：ns	Unsigned Long	4BYTE
	unsigned char UL_CalRRU[4];//RRU上行补偿值(UL CalRRU)	单位：ns	Unsigned Long	4BYTE
}DelayCfgReqT;

typedef struct _DelayCfgRspT {
	unsigned char FiberId;//光纤号	0：光纤端口1   1：光纤端口2…	Unsigned char	1BYTE
	unsigned char result;//返回结果	0：成功1：失败	Unsigned char	1BYTE
}DelayCfgRspT;

typedef struct _PowCal{
#ifdef _RRU_NEW_INTERFACE_
    u16 u16CalType;         /*校准类型  0-online; 1-full*/
    u16 u16CalOpType;       /*校准操作类型  0-只做下行功率校准；1-只做上行功率校准；2-上下行校准都做*/
    u32 u32CalCenterFreq;   /*校准中心频点  单位: kHz*/
#else
    unsigned char cal_type[2];
#endif  /*_RRU_NEW_INTERFACE_*/
#ifdef _RRU_NEW_INTERFACE_
}__attribute__((packed))PowCal;
#else
}PowCal;
#endif  /*_RRU_NEW_INTERFACE_*/

typedef struct _PowCalRsp{
    unsigned char result[2];
}PowCalRsp;

typedef struct _CalChnCell{
#ifdef _RRU_NEW_INTERFACE_
    u8  u8antennapow;   /*通道实际发射功率目标值，范围:30dBm~43dBm*/
    s8  s8txpowoffset;  /*功率校准后，与目标功率的偏差，单位:0.1dB*/
    u16 u16txgain;      /*上行增益值，单位:0.5dB*/
    u16 u16rxgain;      /*下行增益值，单位:0.5dB*/
    //u8  u8antRxPow;     /*通道实际接收功率，单位:0.1dBm*/
    s16 s16antRxPow;    /*通道实际接收功率，单位:0.1dBm*/
#else   
    unsigned char antennapow;
    unsigned char txpowoffset;
    unsigned char txgain[2];
    unsigned char rxgain[2];
#endif  /*_RRU_NEW_INTERFACE_*/
#ifdef _RRU_NEW_INTERFACE_
}__attribute__((packed))CalChnCell;
#else
}CalChnCell;
#endif  /*_RRU_NEW_INTERFACE_*/

typedef struct _calDataCfg{
    unsigned char flag;
    unsigned char txrmsflg;
    unsigned char txdpdflg;
    unsigned char startchn;
    unsigned char totalchn;
    unsigned char rev;
    CalChnCell chncell[RFCHANNEL];
#ifndef _RRU_NEW_INTERFACE_
    unsigned char reserve[11];
#endif  /*_RRU_NEW_INTERFACE_*/
}calDataCfg;

typedef struct _calDataCfgRsp{
    unsigned char result[2];
}calDataCfgRsp;
/******************************************************************************/

/*该消息已经被删除 _RRU_NEW_INTERFACE_*/
typedef struct _RF_Cfg
{
	unsigned char N_Cell;
	unsigned char rev1;
	unsigned char rev2[2];
	RF_CfgT rfcfg[4];
/*	
	unsigned char Start_Freq_Index[4];
	unsigned char AntennaPower[2];
	unsigned char RxSensitivity[2];
	unsigned char Cable_Loss[2];
	unsigned char PS_Loss[2];

	PS_NormT ps_norm[8];

	unsigned char SCG_Mask[2];
	unsigned char Calibration_SCGMask[2];
	unsigned char BBU1303_FreqIndex[4];
	unsigned char BBU1303AntennaPower[2];*/
}__attribute__((packed))RF_Cfg;


enum
{
	type_chnSetup=1,		/* 1	通道建立请求*/
	type_chnSetupCfgReq,	/* 2	通道建立配置*/
	type_chnSetupCfgRsp,	/* 3	通道建立配置应答*/
	type_SWUpdateResInd,	/* 4	版本更新结果指示*/
	type_SWUpdateResRsp,	/* 5	版本更新结果指示应答*/

	type_SWVerQry=15,		/* 15	软件版本查询*/
	type_SWVerQryRsp,		/* 16	软件版本查询应答*/
	type_HWVerQry,			/* 17	硬件版本查询*/
	type_HWVerQryRsp,		/* 18	硬件版本查询应答*/

	type_VerDownReq=21,		/* 21	版本下载请求*/
	type_VerDownRsp,		/* 22	版本下载应答*/
	type_VerDownResInd,		/* 23	版本下载结果指示*/

	type_VerActivateReq=31,	/* 31	版本激活指示请求*/
	type_VerActivateRsp,	/* 32	版本激活指示应答*/

	type_RFStateQryReq=41,	/* 41	射频状态查询请求*/
	type_RFStateQryRsp,		/* 42	射频状态查询应答*/
	type_RruStateQryReq,	/* 43	RRU运行状态查询*/
	type_RruStateQryRsp,	/* 44	RRU运行状态查询应答*/
	type_FiberStateQryReq,	/* 45	光口状态查询请求*/
	type_FiberStateQryRsp,	/* 46	光口状态查询应答*/
#ifdef _RRU_NEW_INTERFACE_
    type_ParameterQryReq=51,/* 51	参数查询请求*/
	type_ParameterQryRsp,	/* 52	参数查询应答*/
#else
	type_SWRStateQryReq=51,	/* 51	驻波比状态查询请求*/
	type_SWRStateQryRsp,	/* 52	驻波比状态查询应答*/
#endif /*_RRU_NEW_INTERFACE_*/
	type_AlarmThresholdQryReq,	/* 53	告警门限查询请求*/
	type_AlarmThresholdQryRsp,	/* 54	告警门限查询应答*/

	type_TmSlotCfgReq=61,	/* 61	时隙比配置请求*/
	type_TmSlotCfgRsp,		/* 62	时隙比配置应答*/
	type_SysTimeCfgReq,		/* 63	系统时间配置请求*/
	type_SysTimeCfgRsp,		/* 64	系统时间配置应答*/
	type_AlarmThresholdCfgReq,	/* 65	告警门限配置请求*/
	type_AlarmThresholdCfgRsp,	/* 66	告警门限配置应答*/
	type_AntennaStateCfgReq,	/* 67	天线状态配置请求*/
	type_AntennaStateCfgRsp,	/* 68	天线状态配置应答*/

	type_RFCfgReq=81,		/* 81	射频配置请求*/
	type_RFCfgRsp,			/* 82	射频配置应答*/
#ifdef _RRU_NEW_INTERFACE_
    type_CellCfgQryReq,     /* 83	小区配置查询请求*/
    type_CellCfgQryRsp,     /* 84	小区配置查询应答*/
#else	
	type_CaliDataCfgReq,	/* 83	校准综合数据配置请求*/
	type_CaliDataCfgRsp,	/* 84	校准综合数据配置应答*/
#endif /*_RRU_NEW_INTERFACE_*/

    type_PowerCaliCfgReq,	/* 85	功率校准配置请求*/
	type_PowerCaliCfgRsp,	/* 86	功率校准配置应答*/
	type_PowerCaliCfgResInd,/* 87	功率校准结果通知*/

	type_Alarm=91,			/* 91	告警上报请求*/
	type_AlarmQryReq,		/* 92	告警查询请求*/
	type_AlarmQryRsp,		/* 93	告警查询应答*/
	type_RFOnOffRpt,		/* 94	射频开关状态上报*/
	type_RFOnOffRptRsp,		/* 95	射频开关状态上报响应*/


	type_LogUploadReq=101,	/* 101	日志上传请求*/
	type_LogUploadRsp,		/* 102	日志上传应答*/
	type_LogUploadResIndi,	/* 103	日志上传结果指示*/

	type_HeartBeatReq=110,	/* 110	RRU在位检测*/
	type_HeartBeatRsp,		/* 111	RRU在位检测应答*/

	type_ResetReq=121,		/* 121	RRU复位*/
	type_ResetRsp,			/* 122	RRU复位应答*/

	type_CellCfgReq=130,	/* 130	小区配置*/
	type_CellCfgRsp,		/* 131	小区配置应答*/

	type_DelayMeasureReq=140,	/* 140	时延测量请求*/
	type_DelayMeasureRsp,		/* 141	时延测量应答*/
	type_DelayCfgReq,			/* 142	时延配置命令*/
	type_DelayCfgRsp,			/* 143	时延配置命令应答*/

    type_PowerResetReq=144,     /* 144  远程掉电复位指示*/
    type_PowerResetRsp=145,     /* 145  远程掉电复位指示*/    

    type_HWParamReq=146,        /* 146  硬件参数查询请求*/
    type_HWParamRsp=147,        /* 147  硬件参数查询应答*/

    type_SerialCloseReq = 251		/* 150	串口重定向关闭命令*/
};


#endif /* __MSGSTRUCT_H */


