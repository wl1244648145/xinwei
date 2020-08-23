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
	u8  u8ChnSetupReason;   //ͨ������ԭ�� a	
	                        //0��BBU�෢������������
                            //1��RRU�෢������������
                            //2���ϵ�
                            //���ౣ��
	u32 u32Warncode;        //����RRU�෢������������ĸ澯��
	/**********��Ӳ���汾��Ϣ**********/
	u8  au8pkg_ver[4];      //�������汾
	u8  au8os_ver[4];       //����ϵͳ����汾
	u8  au8dts_ver[4];      //DTS�ļ�
	u8  au8uboot_ver[4];    //Uboot�汾
	u8  au8app_ver[4];      //�ļ�ϵͳ+Ӧ������汾
	u8  au8fpga_ver[4];     //FPGA����汾
	u32 u32masterboard_ver; //Ӳ������汾��
	u32 u32slaveboard_ver;  //Ӳ���Ӱ�汾��
	u32 u32pa_masterboard_ver;  //Ӳ����������汾��
	u32 u32pa_slaveboard_ver;   //Ӳ�����ŴӰ�汾��
	u8  au8masterboard_sn[32];  //Ӳ���������к�
	u8  au8slaveboard_sn[32];   //Ӳ���Ӱ����к�
	u8  au8pa_masterboard_sn[32];   //Ӳ�������������к�
	u8  au8pa_slaveboard_sn[32];    //Ӳ�����ŴӰ����к�
	u16 u16tx_max_pow;      //ͨ������书�ʣ���λ:0.1dBm
	s16 s16aiq_tx_nom;      //���ж���ֵ
	u16 u16rx_max_pow;      //ͨ�������չ��ʣ���λ:0.1dBm
	s16 s16aiq_rx_nom;      //���ж���ֵ
	u16 u16FreqBand;        //Ƶ����Ϣ��ö��ֵ:ENUM_RRU_FREQBAND_INFO
	u16 u16AntNum;          //�����շ�״̬���ϱ����߿����������BBU
	u16 u16ProductType;     //RRU��Ʒ���ͣ�ö��ֵ:ENUM_RRU_PRODUCT_TYPE
}__attribute__((packed))chnSetupT;/*_RRU_NEW_INTERFACE_*/


/*Ӳ��������ѯ��Ӧ*/
typedef struct _HWParamRsp {
	/*===============��EEPROM����Ϣ=================*/
	u8  u8Freq_IO;          //�ߵͱ���1-�߱���2-�ͱ���
	u16 u16Freq_Band_Val;   //Ƶ��ֵ������ֵ����λ: MHz
	u32 u32Freq_min;        //Ƶ����Сֵ����λ: KHz
	u32 u32Freq_max;        //Ƶ�����ֵ����λ: KHz
	u8  u8RF_total_chn;     //��Ƶͨ��������ȡֵ: 4��8
	u8  u8RF_Freq_K;        //Ƶ������������
    u8  u8RF_Freq_X;        //��Ƶ��ϵ����hmc1044
    u8  au8RRU_TYPE[16];             /*��Ʒ�ͺţ�XWR6404-18*/
    u8  au8RRU_ProductTYPE[16];      /*��Ʒ���ͣ�RRU4E30_CF*/
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

	unsigned char SCG_Mask[2];      /*δ�õ�*/
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
	
//ϵͳʱ������
	unsigned char second;//��	0-59	Unsigned Char	1 BYTE
	unsigned char minute;//��	0-60	Unsigned Char	1 BYTE
	unsigned char hour;//ʱ	0-23	Unsigned Char	1 BYTE
	unsigned char day;//��	1-31	Unsigned Char	1 BYTE
	unsigned char month;//��	1-12	Unsigned Char	1 BYTE
	unsigned char year[2];//��	1997-2099	Unsigned Short	2 BYTE
//����ģʽ
	unsigned char workmode;//	RRU������ʽ	0��LTE 1��McWill	Unsigned char	1 BYTE
	unsigned char rrumode;//RRU����ģʽ	0��RRU��������1������״̬	Unsigned char	1 BYTE
	unsigned char irmode;//Ir�ӿڹ���ģʽ	1����ͨģʽ2������ģʽ	Unsigned char	1 BYTE
#ifdef _RRU_NEW_INTERFACE_
    unsigned char au8Reserved[2];   /*��ʱ϶��������ֶα�Ϊ����*/
#else
	unsigned char tssum;//��ʱ϶����	Mcwill��LTE����Ҫ	unsigned char	1 BYTE
	unsigned char dlts;//����ʱ϶��	Mcwill��LTE����Ҫ	unsigned char	1 BYTE
#endif /*_RRU_NEW_INTERFACE_*/
//FTP��Ϣ
	unsigned char bbuip[4];//BBU��FTP������IP��ַa 		Unsigned Char����	4 BTYE
	char ftpusername[20];//FTP�û���		�ַ���	20 BYTE
	char ftppassword[20];//FTP����		�ַ���	20 BYTE
	char filename[20];
	unsigned char rsv0[80];//reserved			100 BYTE
//����汾�˶Խ��
	unsigned char ver_comp_res;//�������汾	0���汾��ͬ1���汾��ͬ	unsigned char	1BYTE
	unsigned char rsv1[5];//Reserved	Ԥ������С�汾�˶Խ��		5BYTE
//�澯��������
	char board_temp_threshold;//�����Ӱ��¶�����	ȡ��	signed char	  1
	char rfchn_temp_threshold;//��Ƶͨ���¶�����	ȡ��	signed char	  1
	char vswr[2];//פ��������	��λ�� 0.1	unsigned short	2 BYTE
	char rsv2[6];//reserved			6 BYTE
#ifndef _RRU_NEW_INTERFACE_
//��Ƶ����
	unsigned char N_Cell;
	unsigned char rsv3[3];
	RF_CfgT rf_cfg[4];
#endif /*_RRU_NEW_INTERFACE_*/
#if 0
	unsigned char scgmask[4];//20M_SCG_Mask	20M SCG ���룬Ϊÿ��5M��SCG���룬1Ϊ�򿪣�0Ϊ��ֹ		4 BYTE
	unsigned char freq[4];//20MFreqIndex	20M ����Ƶ��//��λ50K;//start Freq Index use base of band ID passing from AUX,//After AUX receive this, it will do floor((start Freq index-PLLmin)/PLLstep), and look up the PLL table to do the configuration		4 BYTE
	unsigned char pw[2];//20M AntennaPower	20M��վ����ʵ���书�ʡ�		2 BYTE
#endif
}chnSetupCfgReqT;

typedef struct _chnSetupCfgRspT {
	
	unsigned char result[2];//Result	0 succ  1 fail		2BYTE
	unsigned char allresults[4];//���ؽ��	ÿ���ر�ʾһ�����õķ��ؽ��//0���ɹ�//1��ʧ��//bit0��RRU����ģʽbit1��Ir�ӿ�ģʽbit2����������bit3��Ƶ������bit4�����߹�������	Unsigned long	4 BYTE

}chnSetupCfgRspT;

typedef struct _SWUpdateResultIndT {
	unsigned char result;//��汾	0���ɹ�1��ʧ��	unsigned char	1 BYTE
}SWUpdateResultIndT;

typedef struct _SWQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char bigPkgVer[4];//��汾		Unsigned long	4 BYTE
	unsigned char osver[4];//����ϵͳ����汾		Unsigned long	4 BYTE
	unsigned char dtsver[4];//DTS�ļ�		Unsigned long	4 BYTE
	unsigned char ubootver[4];//Uboot�汾		Unsigned long	4 BYTE
	unsigned char appver[4];//�ļ�ϵͳ+Ӧ������汾		Unsigned long	4 BYTE
	unsigned char fpgaver[4];//FPGA����汾		Unsigned long	 4 BYTE
}SWQryRspT;

typedef struct _HWQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char mainboardver[4];//	Ӳ������汾��		unsigned short	 4 BYTE
	unsigned char slaveboardver[4];//	Ӳ���Ӱ�汾��		unsigned short	 4 BYTE
	unsigned char pamainboardver[4];//	Ӳ����������汾��		unsigned short	 4 BYTE
	unsigned char paslaveboardver[4];//	Ӳ�����ŴӰ�汾��		unsigned short	 4 BYTE
	unsigned char mainboardsn[32];//	Ӳ���������к�		�ַ���	32 BYTE
	unsigned char slaveboardsn[32];//	Ӳ���Ӱ����к�		�ַ���	32 BYTE
	unsigned char pamainboardsn[32];//	Ӳ�������������к�		�ַ���	32 BYTE
	unsigned char paslaveboardsn[32];//	Ӳ�����ŴӰ����к�		�ַ���	32 BYTE
}HWQryRspT;

typedef struct _VersionDownloadReqT {
	char filename[20];//	������ļ���		Unsigned char����	20BYTE
}VersionDownloadReqT;

typedef struct _VersionDownloadRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
}VersionDownloadRspT;

typedef struct _VersionDownloadResIndT {
	unsigned char result;//	��汾	0���ɹ�1��ʧ��	unsigned char	1 BYTE
}VersionDownloadResIndT;

typedef struct _VersionActivateRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char reason[2];//	ʧ��ԭ��	1��Ҫ����İ汾������2���ļ����ƻ�3������汾��Ӳ���汾��ƥ��4������ɹ�5������ԭ��	Unsigned short	2 BYTE
}VersionActivateRspT;

typedef struct _SwrStateQryReqT {
	char rfchn;//��Ƶͨ����	0~��Ƶͨ������-1�� ��ʾ��ѯָ����ͨ������Ƶͨ����������ѯ����ͨ��	unsigned char	1 BYTE
}SwrStateQryReqT;

typedef struct _SwrStateQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char rfchn;//��Ƶͨ����	0~��Ƶͨ������-1�� ��ʾ��ѯָ����ͨ������Ƶͨ����������ѯ����ͨ��	unsigned char	1 BYTE
	unsigned char swrval[RF_CHN_COUNT][2];//ͨ��פ����	��λ�� 0.1	signed short	2 BYTE
} SwrStateQryRspT;

typedef struct _AlarmThresholdQryRspT {
	unsigned char result[2];//Result	0 succ 1 fail		2BYTE
	unsigned char swr[2];//פ��������	��λ��0.1	unsigned short	2 BYTE
	char board_temp_threshold;//�����Ӱ��¶�����	ȡ��	signed char	1 BYTE
	char rfchn_temp_threshold;//��Ƶͨ���¶�����	ȡ��	signed char	1 BYTE
	//reserved			2 BYTE
	//reserved			4 BYTE
	char rsv2[6];
}AlarmThresholdQryRspT;

/*������ѯ����ӿ�  _RRU_NEW_INTERFACE_*/
typedef struct _ParameterQryRspT {
	u16 u16result;          //Result	0-succ 1-fail
	u16 u16vswrThreshold;   //פ��������	��λ��0.1
	s8  s8board_temp_threshold; //�����Ӱ��¶�����	ȡ��
	s8  s8rfchn_temp_threshold; //��Ƶͨ���¶�����	ȡ��
	u8  au8Reserved[6];        //
	/********* McWiLL ʱ϶��� *********/
    u8  u8TsTotalNum;       //��ʱ϶����Ĭ��Ϊ:8
    u8  u8DLTsNum;          //����ʱ϶��
}ParameterQryRspT;

typedef struct _RFStateQryReqT {
	unsigned char rfchn;//	��Ƶͨ����	0~��Ƶͨ������-1�� ��ʾ��ѯָ����ͨ������Ƶͨ����������ѯ����ͨ��	unsigned char	1 BYTE
}RFStateQryReqT;

typedef struct _RFChnStateT {
#ifdef _RRU_NEW_INTERFACE_
    u8  u8dl_antenna_state;     //����״̬	0���ر�1������
	u8  u8ul_antenna_state;     //����״̬	0���ر�1������
	s8  s8chn_temp;             //ͨ���¶�	ȡ��
	s16 s16txpw;                //���书��	��λ��0.1dBm
	u8  u8txgain;               //��������	��λ��0.1dB
	u8  u8rxgain;               //��������	��λ��0.1dB
	s16 s16rxpw;                //���չ���  ��λ: 0.1dBm
	s16 s16vswr;                //פ����    ��λ: 0.1
	u8  u8ResultGetTxpw;        //���й��ʶ�ȡ���: 0-SUCC;1-FAIL
	u8  u8ResultGetRxpw;        //���й��ʶ�ȡ���: 0-SUCC;1-FAIL
	u8  u8ResultGetVswr;        //פ���ȶ�ȡ���:   0-SUCC;1-FAIL	
	u8  u8Reserved;             //Reserved
#else
	unsigned char dl_antenna_state;//����״̬	0���ر�1������	unsigned char	1 BYTE
	unsigned char ul_antenna_state;//����״̬	0���ر�1������	unsigned char	1 BYTE	
	char chn_temp;//ͨ���¶�	ȡ��	Signed char	1 BYTE
	unsigned char txpw[2];//���书��	��λ��0.1dBm	Signed short	2 BYTE
	unsigned char txgain;//��������	dB	unsigned char	1 BYTE
	unsigned char rxgain;//��������	dB	unsigned char	1 BYTE
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
	unsigned char rflofreq[4];  //	��Ƶ����Ƶ�ʣ�����ʵ��Ƶ��仯	��λ��100KHz	unsigned long	4 BYTE
	unsigned char rflostate;    //	��Ƶ����״̬	0������1��ʧ��	unsigned char	1 BYTE
	unsigned char clkstate;     //	ʱ��״̬	0��ͬ��1��ʧ��	unsigned char	1 BYTE
	unsigned char irifmode;     //	Ir�ӿڹ���ģʽ	1����ͨģʽ2������ģʽ	unsigned char	1 BYTE
#ifndef _RRU_NEW_INTERFACE_ /*���ֶ��Ѿ���ɾ��*/
	unsigned char totalts;      //	��ʱ϶����		unsigned char	1 BYTE
	unsigned char dlts;         //	����ʱ϶��		unsigned char	1 BYTE
#endif  /*_RRU_NEW_INTERFACE_*/
	unsigned char rsv[2];
	unsigned char workmode;     //	RRU����״̬a	0��δ��Ӫ1��������2����Ӫ��3���汾������4������	unsigned char	1 BYTE
	//�¶�
	char mainboard_temp;        //	�����¶�	ȡ��	signed char	1 BYTE
	char slaveboard_temp;       //	�Ӱ��¶�	ȡ��	signed char	1 BYTE
	//ϵͳʱ��
	unsigned char sec;          //	��	0-59	Unsigned Char	1 BYTE
	unsigned char min;          //	��	0-60	Unsigned Char	1 BYTE
	unsigned char hour;         //	ʱ	0-23	Unsigned Char	1 BYTE
	unsigned char day;          //	��	1-31	Unsigned Char	1 BYTE
	unsigned char mon;          //	��	1-12	Unsigned Char	1 BYTE
	unsigned char reserved[3];
	unsigned char year[2];      //	��	1997-2099	Unsigned Short	2 BYTE
	unsigned char workingtime[4];//	rru����ʱ��	��	unsigned long	4 BYTE
}RruStateQryRspT;

typedef struct _FiberPortStateQryReqT {
unsigned char fiberport;//	��ں�	0~��Ƶͨ������-1�� ��ʾ��ѯָ����ڣ������������ѯ���й��	unsigned char	1 BYTE
}FiberPortStateQryReqT;

typedef struct _FiberPortStateT {
	unsigned char rxpw[2];//	�չ���	��λ0.1uW	Unsigned short	2BYTE
	unsigned char txpw[2];//	������	��λ0.1 uW	Unsigned short	2BYTE
	unsigned char ineffect;//	��λ��Ϣ	1����λ//	0������λ	Unsigned char	1BYTE
	unsigned char rev1;
	char vendor[16];//	��ģ�鳧��	ASCII	�ַ���	16BYTE
	char rate[2];//	��ģ�鴫��bit����	��λ Mbit/s Unsigned short	2BYTE
	char temperature;//	�¶�	��λ C���϶�	Signed char 1BYTE
	unsigned char rev2;
	unsigned char voltage[2];//	��ѹ	��λ mV Unsigned short	2BYTE
	unsigned char current[2];//	����	��λ mA Unsigned short	2BYTE
	unsigned char rsv[4];
}FiberPortStateT;

typedef struct _FiberPortStateQryRspT {
	unsigned char result[2];
	unsigned char fiberport;//	��ں�	0~��Ƶͨ������-1�� ��ʾ��ѯָ����ڣ������������ѯ���й��	unsigned char	1 BYTE
	unsigned char rsv;
	FiberPortStateT fpstat[FIBER_PORT_COUNT];
}FiberPortStateQryRspT;

/*����Ϣ�Ѿ���ɾ�� _RRU_NEW_INTERFACE_ */
typedef struct _TimeSlotCfgReqT {
	unsigned char tssum;//��ʱ϶����		unsigned char	1 BYTE
	unsigned char dlts;//����ʱ϶��		unsigned char	1 BYTE
}TimeSlotCfgReqT;

typedef struct _TimeSlotCfgRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char ret;//	���ؽ��	0���ɹ�1��ʧ��	unsigned char	1 BYTE
}TimeSlotCfgRspT;

typedef struct _TimeCfgReqT {
	unsigned char second;//��	0-59	Unsigned Char	1 BYTE
	unsigned char minute;//��	0-60	Unsigned Char	1 BYTE
	unsigned char hour;//ʱ	0-23	Unsigned Char	1 BYTE
	unsigned char day;//��	1-31	Unsigned Char	1 BYTE
	unsigned char month;//��	1-12	Unsigned Char	1 BYTE
	unsigned char year[2];//��	1997-2099	Unsigned Short	2 BYTE
}TimeCfgReqT;

typedef struct _TimeCfgRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char ret;//	���ؽ��	0���ɹ�1��ʧ��	unsigned char	1 BYTE
}TimeCfgRspT;

typedef struct _AlarmThresholdCfgReqT {
	char swr[2];//פ��������	��λ�� 0.1	unsigned short	2 BYTE
	char board_temp_threshold;//�����Ӱ��¶�����	ȡ��	signed char	  1
	char rfchn_temp_threshold;//��Ƶͨ���¶�����	ȡ��	signed char	  1
	char rsv2[6];//reserved			6 BYTE
}AlarmThresholdCfgReqT;

typedef struct _AlarmThresholdCfgRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char ret;//	���ؽ��	0���ɹ�1��ʧ��	unsigned char	1 BYTE	bit0��פ��������	bit1�������Ӱ��¶�����	bit2����Ƶͨ���¶�����
}AlarmThresholdCfgRspT;

typedef struct _AntennaStateCfgReqT {
	unsigned char dlantennamask;//��������״̬	0���ر�����	1����������	bit7�����߱��7	...	bit0�����߱��0 unsigned char	1 BYTE
	unsigned char ulantennamask;//��������״̬	0���ر�����	1����������	bit7�����߱��7	...	bit0�����߱��0 unsigned char	1 BYTE
	char rsv[3];	//reserved			3 BYTE
}AntennaStateCfgReqT;

typedef struct _AntennaStateCfgRspT {
	unsigned char result[2];//	Result	0 succ 1 fail		2BYTE
	unsigned char ret;//	���ؽ��	0���ɹ�1��ʧ��	unsigned char	1 BYTE
}AntennaStateCfgRspT;

typedef struct _ResetReqT {
	unsigned char resettype;//	��λ����	0������RRU��λ	unsigned char	1 BYTE
}ResetReqT;
typedef struct _ResetRspT {
	unsigned char result[2];//	Result	0 succ	1 fail	Unsigned short	2BYTE
}ResetRspT;

typedef struct _AlarmRptT {
	unsigned char sn[4];//Alarm Sequence Number	4	M	0xffffffff	�澯���к�
	unsigned char flag;//Flag	1	M	0 �澯�ָ�1 �澯����	�澯��ʾ
	unsigned char year[2];//Timestamp - year	2	M		��
	unsigned char mon;//Timestamp - month	1	M		��
	unsigned char day;//Timestamp - day	1	M		��
	unsigned char hour;//Timestamp - hour	1	M		ʱ
	unsigned char minute;//Timestamp - minute	1	M		��
	unsigned char second;//Timestamp - second	1	M		��
	unsigned char type[2];//Alarm Entity Type	2	M	L3�澯0x01 L2�澯0x02 MCP�澯0x03 AUX�澯0x04 ENV�澯0x05 PLL�澯0x06 RF�澯0x07 GPS�澯 0x08 L1	 0x09 FPGA  0x0a FEP 0x0b Core9 0x0c RRU 0x0d AIF 0x0e ���     0x0f	�澯ʵ������
	unsigned char index[2];//Alarm Entity Indexb	2	M	ALARM_ENTIFY_INDEX0    = 0X0;��Ĭ��ֵ��ALARM_ENTIFY_INDEX1    = 0X1;ALARM_ENTIFY_INDEX2    = 0X2;ALARM_ENTIFY_INDEX3    = 0X3;ALARM_ENTIFY_INDEX4    = 0X4;ALARM_ENTIFY_INDEX5    = 0X5;ALARM_ENTIFY_INDEX6    = 0X6;ALARM_ENTIFY_INDEX7    = 0X7;	�澯ʵ��������
	unsigned char codeidx[2];//Alarm Code Index	2	M	 ��Ӧ�ڸ澯�б��е��¼���	�澯ID
	unsigned char level;//Alarm Severity	1	M	ALARM_CLASS_CRITICAL   = 0X1;ALARM_CLASS_MAJOR     = 0X2;ALARM_CLASS_MINOR     = 0X3;ALARM_CLASS_INFO       = 0X4..	�澯�ȼ�
	unsigned char infolen[2];//Alarm Info Length	2	M		�澯���ݳ���
	char info[200];//Alarm Info	V	O	Text to explain the alarm	�澯����
}AlarmRptT;

typedef struct _AlarmQryRspT {
	unsigned char result;// 0�޸澯1�и澯
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
    u8  u8CfgType;				/*С�����ñ�ʶ  0-����; 1-����; 2-ɾ��*/
	u32 u32CellId; 			    /*����С����ʶ*/
	u16 u16pw;				    /*С������  ��λ��0.1dBm*/
	u8  u8AntennaGrpId;			/*�������  1��8*/
	u8  u8dn_atennaMask;        /*�������п�������*/
	u8  u8up_atennaMask;        /*�������п�������*/
	u8  u8FreqCount;			/*Ƶ����    ָ��ǰ��Ϣ�а�����Ƶ������IE����Ŀ  default: 1*/
	u8  u8CarrierWaveId; 		/*�ز���    ��ʱδʹ��*/
	u32 u32CenterFreq; 		    /*����Ƶ��  ��λ��kHz*/
	u32 u32FreqProperty; 		/*Ƶ�����������    0-��Ƶ��; 1-��Ƶ��TD-LTE R10�����븨Ƶ��    ��ʱδʹ��*/
	u8  u8tdlte_frm_cfg;		/*TD-LTE��������֡��������c	bit0-bit3:ȡֵ��Χ0��6*/
	u32 u32EffectSubFrameNo;	/*���õ���Чϵͳ��֡��	Ϊ10ms֡��ȡֵ��ΧΪ0��4095	(McWiLLδʹ��default: 0)*/
	u32 u32CarrierWaveBW;		/*�ز�����  ö����: ENUM_TYPE_OF_CARRIERWAVEBW*/
	u8  u8SpecialSubFrameCfg;	/*������֡����		bit0-bit3:ȡֵ��Χ0��8  (McWiLLδʹ��default: 0)*/
	u8  u8CPLen;				/*ѭ��ǰ׺����ѡ��	����CP-0����չCP-1  (McWiLLδʹ��default: 0)*/	
    u16 u16SCGMask[4];          /*SCG����   (LTEδʹ��default: 0x1F)*/
#else
	unsigned char CfgType;				/*С�����ñ�ʶ		0������1������2��ɾ��	Unsigned Char	1BYTE*/
	unsigned char CellId[4]; 			/*����С����ʶ								Unsigned Long	4BYTE*/
	unsigned char pw[2];				/*С������			��λ��1/256dbm			Unsigned short	2BYTE*/
	unsigned char AntennaGrpId;			/*�������			1��8					Unsigned Char	1BYTE*/
	unsigned char dn_atenna;
	unsigned char up_atenna;
	unsigned char FreqCount;			/*Ƶ����			ָ��ǰ��Ϣ�а�����Ƶ������IE����Ŀ	Unsigned Char	1BYTE*/
	unsigned char CarrierWaveId; 		/*�ز���									Unsigned Char	1BYTE*/
	unsigned char CenterFreq[4]; 		/*����Ƶ��			��λ��100kHz			Unsigned Long	4BYTE*/
	unsigned char FreqProperty[4]; 		/*Ƶ�����������	0����Ƶ��1����Ƶ��TD-LTE R10�����븨Ƶ��	Unsigned Long	4BYTE*/
	unsigned char tdlte_frm_cfg;		/*TD-LTE��������֡��������c	bit0-bit3:ȡֵ��Χ0��6	Unsigned long	1BYTE*/
	unsigned char EffectSubFrameNo[4];	/*���õ���Чϵͳ��֡��	Ϊ10ms֡��ȡֵ��ΧΪ0��4095	Unsigned Long	4BYTE*/
	unsigned char CarrierWaveBW[4];		/*�ز�����			��ѡ��ֵ��5/10/15/20	Unsigned Long	4BYTE*/
	unsigned char SpecialSubFrameCfg;	/*������֡����		bit0-bit3:ȡֵ��Χ0��8	Unsigned Long	1BYTE*/
	unsigned char CPLen;				/*ѭ��ǰ׺����ѡ��	����CP-0����չCP-1		Unsigned Char	1BYTE*/	
#endif /*_RRU_NEW_INTERFACE_*/
#ifdef _RRU_NEW_INTERFACE_
}__attribute__((packed))CellCfgReqT;
#else
}CellCfgReqT;
#endif /*_RRU_NEW_INTERFACE_*/

typedef struct _CellCfgRspT {
	unsigned char CellId[4]; 			/*����С����ʶ								Unsigned Long	4BYTE*/
	unsigned char result[4];			/*���ؽ��			0���ɹ�1��ʧ��			Unsigned char	4 BYTE*/
}CellCfgRspT;

/*С�����ò�ѯӦ��*/
typedef struct tag_STRU_CELL_CFG_QRY_RSP
{
    u16 u16Result;          /*0-SUCC;1-FAIL*/
    CellCfgReqT CellCfgInfo;
}__attribute__((packed))STRU_CELL_CFG_QRY_RSP;



typedef struct _DelayMeasureReqT {
	unsigned char FiberId[2];//���˺�	0�����˶˿�1      1�����˶˿�2��	unsigned char	1 BYTE
    unsigned char Y1[2];
    unsigned char Y2[2];
}DelayMeasureReqT;

typedef struct _DelayMeasureRspT {
	unsigned char result[2];//���˺�	0�����˶˿�1      1�����˶˿�2��	unsigned char	1 BYTE
}DelayMeasureRspT;

typedef struct _DelayCfgReqT {
	unsigned char FiberId;//���˺�	0�����˶˿�1   1�����˶˿�2	Unsigned char	1BYTE
	unsigned char T12[4];//T12	��λ��ns	Unsigned Long	4BYTE
	unsigned char T34[4];//T34	��λ��ns	Unsigned Long	4BYTE
	unsigned char DL_Offset[4];//֡��ʱ��ǰ�� (DL Offset) a	��λ��Tc	Unsigned Long	4BYTE
	unsigned char DL_CalRRU[4];//RRU���в���ֵ(DL CalRRU)	��λ��ns	Unsigned Long	4BYTE
	unsigned char UL_CalRRU[4];//RRU���в���ֵ(UL CalRRU)	��λ��ns	Unsigned Long	4BYTE
}DelayCfgReqT;

typedef struct _DelayCfgRspT {
	unsigned char FiberId;//���˺�	0�����˶˿�1   1�����˶˿�2��	Unsigned char	1BYTE
	unsigned char result;//���ؽ��	0���ɹ�1��ʧ��	Unsigned char	1BYTE
}DelayCfgRspT;

typedef struct _PowCal{
#ifdef _RRU_NEW_INTERFACE_
    u16 u16CalType;         /*У׼����  0-online; 1-full*/
    u16 u16CalOpType;       /*У׼��������  0-ֻ�����й���У׼��1-ֻ�����й���У׼��2-������У׼����*/
    u32 u32CalCenterFreq;   /*У׼����Ƶ��  ��λ: kHz*/
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
    u8  u8antennapow;   /*ͨ��ʵ�ʷ��书��Ŀ��ֵ����Χ:30dBm~43dBm*/
    s8  s8txpowoffset;  /*����У׼����Ŀ�깦�ʵ�ƫ���λ:0.1dB*/
    u16 u16txgain;      /*��������ֵ����λ:0.5dB*/
    u16 u16rxgain;      /*��������ֵ����λ:0.5dB*/
    //u8  u8antRxPow;     /*ͨ��ʵ�ʽ��չ��ʣ���λ:0.1dBm*/
    s16 s16antRxPow;    /*ͨ��ʵ�ʽ��չ��ʣ���λ:0.1dBm*/
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

/*����Ϣ�Ѿ���ɾ�� _RRU_NEW_INTERFACE_*/
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
	type_chnSetup=1,		/* 1	ͨ����������*/
	type_chnSetupCfgReq,	/* 2	ͨ����������*/
	type_chnSetupCfgRsp,	/* 3	ͨ����������Ӧ��*/
	type_SWUpdateResInd,	/* 4	�汾���½��ָʾ*/
	type_SWUpdateResRsp,	/* 5	�汾���½��ָʾӦ��*/

	type_SWVerQry=15,		/* 15	����汾��ѯ*/
	type_SWVerQryRsp,		/* 16	����汾��ѯӦ��*/
	type_HWVerQry,			/* 17	Ӳ���汾��ѯ*/
	type_HWVerQryRsp,		/* 18	Ӳ���汾��ѯӦ��*/

	type_VerDownReq=21,		/* 21	�汾��������*/
	type_VerDownRsp,		/* 22	�汾����Ӧ��*/
	type_VerDownResInd,		/* 23	�汾���ؽ��ָʾ*/

	type_VerActivateReq=31,	/* 31	�汾����ָʾ����*/
	type_VerActivateRsp,	/* 32	�汾����ָʾӦ��*/

	type_RFStateQryReq=41,	/* 41	��Ƶ״̬��ѯ����*/
	type_RFStateQryRsp,		/* 42	��Ƶ״̬��ѯӦ��*/
	type_RruStateQryReq,	/* 43	RRU����״̬��ѯ*/
	type_RruStateQryRsp,	/* 44	RRU����״̬��ѯӦ��*/
	type_FiberStateQryReq,	/* 45	���״̬��ѯ����*/
	type_FiberStateQryRsp,	/* 46	���״̬��ѯӦ��*/
#ifdef _RRU_NEW_INTERFACE_
    type_ParameterQryReq=51,/* 51	������ѯ����*/
	type_ParameterQryRsp,	/* 52	������ѯӦ��*/
#else
	type_SWRStateQryReq=51,	/* 51	פ����״̬��ѯ����*/
	type_SWRStateQryRsp,	/* 52	פ����״̬��ѯӦ��*/
#endif /*_RRU_NEW_INTERFACE_*/
	type_AlarmThresholdQryReq,	/* 53	�澯���޲�ѯ����*/
	type_AlarmThresholdQryRsp,	/* 54	�澯���޲�ѯӦ��*/

	type_TmSlotCfgReq=61,	/* 61	ʱ϶����������*/
	type_TmSlotCfgRsp,		/* 62	ʱ϶������Ӧ��*/
	type_SysTimeCfgReq,		/* 63	ϵͳʱ����������*/
	type_SysTimeCfgRsp,		/* 64	ϵͳʱ������Ӧ��*/
	type_AlarmThresholdCfgReq,	/* 65	�澯������������*/
	type_AlarmThresholdCfgRsp,	/* 66	�澯��������Ӧ��*/
	type_AntennaStateCfgReq,	/* 67	����״̬��������*/
	type_AntennaStateCfgRsp,	/* 68	����״̬����Ӧ��*/

	type_RFCfgReq=81,		/* 81	��Ƶ��������*/
	type_RFCfgRsp,			/* 82	��Ƶ����Ӧ��*/
#ifdef _RRU_NEW_INTERFACE_
    type_CellCfgQryReq,     /* 83	С�����ò�ѯ����*/
    type_CellCfgQryRsp,     /* 84	С�����ò�ѯӦ��*/
#else	
	type_CaliDataCfgReq,	/* 83	У׼�ۺ�������������*/
	type_CaliDataCfgRsp,	/* 84	У׼�ۺ���������Ӧ��*/
#endif /*_RRU_NEW_INTERFACE_*/

    type_PowerCaliCfgReq,	/* 85	����У׼��������*/
	type_PowerCaliCfgRsp,	/* 86	����У׼����Ӧ��*/
	type_PowerCaliCfgResInd,/* 87	����У׼���֪ͨ*/

	type_Alarm=91,			/* 91	�澯�ϱ�����*/
	type_AlarmQryReq,		/* 92	�澯��ѯ����*/
	type_AlarmQryRsp,		/* 93	�澯��ѯӦ��*/
	type_RFOnOffRpt,		/* 94	��Ƶ����״̬�ϱ�*/
	type_RFOnOffRptRsp,		/* 95	��Ƶ����״̬�ϱ���Ӧ*/


	type_LogUploadReq=101,	/* 101	��־�ϴ�����*/
	type_LogUploadRsp,		/* 102	��־�ϴ�Ӧ��*/
	type_LogUploadResIndi,	/* 103	��־�ϴ����ָʾ*/

	type_HeartBeatReq=110,	/* 110	RRU��λ���*/
	type_HeartBeatRsp,		/* 111	RRU��λ���Ӧ��*/

	type_ResetReq=121,		/* 121	RRU��λ*/
	type_ResetRsp,			/* 122	RRU��λӦ��*/

	type_CellCfgReq=130,	/* 130	С������*/
	type_CellCfgRsp,		/* 131	С������Ӧ��*/

	type_DelayMeasureReq=140,	/* 140	ʱ�Ӳ�������*/
	type_DelayMeasureRsp,		/* 141	ʱ�Ӳ���Ӧ��*/
	type_DelayCfgReq,			/* 142	ʱ����������*/
	type_DelayCfgRsp,			/* 143	ʱ����������Ӧ��*/

    type_PowerResetReq=144,     /* 144  Զ�̵��縴λָʾ*/
    type_PowerResetRsp=145,     /* 145  Զ�̵��縴λָʾ*/    

    type_HWParamReq=146,        /* 146  Ӳ��������ѯ����*/
    type_HWParamRsp=147,        /* 147  Ӳ��������ѯӦ��*/

    type_SerialCloseReq = 251		/* 150	�����ض���ر�����*/
};


#endif /* __MSGSTRUCT_H */


