/****L3WRRU.h***********
*Author: liulihan
*date: 2009.060.2
****************************/

#ifndef _L3WRRU_H
#define _L3WRRU_H

#include "vxWorks.h"
#include "iv.h"
#include "memLib.h"
#include "intLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
//#include "l3OamWRRUReq.h"
/*
#include "netLib.h"

#include "net/protosw.h"
#include "sys/socket.h"

#include "net/if.h"
#include "net/route.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"
#include "netinet/ip.h"
#include "netinet/if_ether.h"
#include "net/if_subr.h"
#include "m2Lib.h"
*/

//#include "mcWill_bts.h"

//#include "mv64360.h"
//#include "config.h"
#include "BizTask.h"
#include "Timer.h"
//#include "btsConfig.h"


#define fail 0x00
#define succ 0x01

/**filename**/
/*#define COPY_FILE_FROM_FTPSERVER	1*/



/**element index***/
#define BBU_No 0x10
#define RRU_No 0x01
#define Max_Calibration_HW_Bag 3

/**message id***/
#define Type_L2_WRRU 0x4001
#define Type_L3_WRRU 0x4000
#define M_L1_Calibration_HW 0x4002//Calibration hardware for aux,used as wrru reset
#define  M_OTHER2_WRRU   0x4003

#define Type_WRRU_No 0x02
#define Type_Config_WRRU 0x03
#define Type_Config_WRRU_Ack 0x03
#define Type_Query_WRRU_Calibration_HW 0x04
#define Type_Query_WRRU_Calibration_HW_Ack 0x04
#define Type_Get_WRRU_Calibration_HW 0x05
#define Type_Poll_Mcu 0x07
#define Type_Poll_Mcu_Ack 0x07

//Type of Config & Query
#define Type_Query_WRRU_Version 0x10
#define Type_Query_WRRU_Ver_Ack 0x10
#define Type_Config_WRRU_PLL 0x11
#define Type_Config_WRRU_PLL_Ack 0x11
#define Type_Config_WRRU_CalibrationData 0x12
#define Type_Config_WRRU_CalibrationData_Ack 0x12
#define Type_Config_WRRU_Timeslot 0x13
#define Type_Config_WRRU_Timeslot_Ack 0x13
#define Type_Config_WRRU_General 0x14
#define Type_Config_WRRU_General_Ack 0x14
#define Type_Config_WRRU_CircumsPara 0x15
#define Type_Config_WRRU_CircumsPara_Ack 0x15
#define Type_Config_WRRU_DbgSwh 0x16
#define Type_Config_WRRU_DbgSwh_Ack 0x16
#define Type_Reset_WRRU 0x17
#define Type_MCU_DbgMsg_Report 0x18
#define Type_Query_WRRU_RF1to4_Version	0x19
#define Type_Query_WRRU_RF1to4_Ver_Ack	0x19
#define Type_Query_WRRU_RF5to8_Version	0x1a
#define Type_Query_WRRU_RF5to8_Ver_Ack	0x1a
#define Type_Query_WRRU_SPF_Version	0x1b
#define Type_Query_WRRU_SPF_Ver_Ack	0x1b

//Type of Code Download
#define Type_MCUCode_Download	0x20
#define Type_MCUCode_Download_Ack 0x20
#define Type_MCUCode 0x21
#define Type_MCUCode_Ack 0x21
#define Type_MCUCode_End 0x22
#define Type_MCUCode_End_Ack 0x22
#define Type_FPGACode_Download 0x23
#define Type_FPGACode_Download_Ack 0x23
#define Type_FPGACode 0x24
#define Type_FPGACode_Ack 0x24
#define Type_FPGACode_End 0x25
#define Type_FPGACode_End_Ack 0x25

//Type of Alarm
#define Alarm_WRRU_Reset 0x40
#define Alarm_WRRU_RFPwOff 0x41
#define Alarm_WRRU_SYNPwOff 0x42
#define Alarm_WRRU_Temp 0x43
#define Alarm_WRRU_Current 0x44
#define Alarm_WRRU_TRB_VSWR 0x45
#define Alarm_WRRU_10ms 0x46
#define Alarm_WRRU_LightLink 0x47
#define Alarm_WRRU_FNWrong 0x48
#define Alarm_WRRU_recv_nopoll 0x49
#define Alarm_Sync_PLL_Lost 0x4a
#define Alarm_WRRU_Download_MCU 0x4b
#define Alarm_WRRU_Download_FPGA 0x4c
#define Alarm_WRRU_RF_LOS  0x4d

//Type of Calibration
#define Type_Config_WRRU_Calibration_Action 0x50
#define Type_WRRU_Calibration_Ack 0x51
#define Type_Config_WRRU_Calibration_end 0x52
#define Type_WRRU_Calibration_end_ack 0x53
#define Type_Config_WRRU_Gain_SYN_Tx 0x54
#define Type_Config_WRRU_Gain_TR_Rx	0x55
#define Type_Config_WRRU_Gain_up 0x56
#define Type_Config_WRRU_Cal_up 0x57
#define Type_Config_WRRU_Cal_dn 0x58
#define Type_Syn_Voltage_report 0x59
#define Type_WRRU_Result_Commun_dn 0x5a
#define Type_WRRU_TEmp_Current   0x5b
#define Type_WRRU_SYN_DSB_Ver   0x5c
#define Type_Fpga_Register  0x5d
#define Type_CSI_INFO_Read 0x60//wangwenhua add 20110930
#define Type_Read_RRU_Fiber_INfo  0x61

#define Type_Control_RRU_Power   0x62
#define  BTS_WRRU_LINK_PERIOD   (4500/*5 * 1000*/ + 1)
#define  BTS_WRRU_CFG_PERIOD   (2 * 1000+2)
#define  BTS_WRRU_CFG_PERIOD_OTHER   (1 * 1000+2)
#define BTS_WRRU_LOAD_CODE_PERIOD   (1*1000 +30)
#define M_BTS_WRRU_LINK_TIMER_Report 0x00b1
#define M_BBU_RRU_RUNNING             0x00b2
#define M_BTS_WRRU_LINK_TIMER 0x00bb
#define M_BTS_WRRU_CFG_TIMER  0x00cc
#define M_BTS_WRRU_CFG_TIMER_OTHER   0x00dd
#define M_BTS_WRRU_LoadCode_TIMER      0x00ee
#define M_BTS_WRRU_RESET_TIMER      0x00b3

/**from oam message*****/
	typedef enum{ 
		IDLE,
		DOWNLOAD,
		RESET,
		BOOTING,
		RUNNING,
		ALARMING
	}T_RunningState;
#pragma pack(1)	
       struct WRRU_message{
	   	unsigned char des;
		unsigned char src;
		unsigned char type;
		unsigned char length;

	};
	struct WRRU_Data{
		T_RunningState WRRU_State;
		unsigned char WRRU_Num;//Get WRRU_No.
		unsigned short DbgCtl;
		unsigned int FPGA_Version;
		unsigned short TDD10ms_Sel; //10ms source select
		///////////////alarm and error statistics	
		unsigned short Alarm;	//各种错误
		unsigned short Frame_CRC; //10ms内光纤帧CRC错误个数
		/////////////////only for test
		unsigned short Test_Mode;//FPGA测试模式，只在测试时候用
	};

struct T_PLL_Msg{
	unsigned short transID;
	unsigned int PLL;
};


struct POLL_Msg{
       unsigned short Frame_No;
       unsigned short CRC;
};

struct T_ConfigWRRU_msg{
	unsigned int StartFreqIndex;
	unsigned char TimeSlotNum;
	unsigned char DLTSNum;
	unsigned short Syn_TxGain;
	unsigned short Syn_RxGain;
	unsigned char RF_TxGain[8];
	unsigned char RF_RxGain[8];
	unsigned char RFPOWERONOFF;
	unsigned char SYNPOWERONOFF;
	unsigned char Max_Temp;
	unsigned char Min_Temp;
	unsigned int Max_Current;
	unsigned int Min_Current;
	unsigned short TDDSRC;
	unsigned short DbgCtrl;
	unsigned short CRC;
};

struct T_RSV_Msg{
	unsigned short transID;
	unsigned short Rsv;
//	unsigned short CRC;
};
	
	
struct GetCalibrationHw_Msg{
	unsigned char Result;
	unsigned char BAG_No;
	unsigned short CRC;
};

struct T_ConfigCalibrationData_Msg{
	unsigned short transID;
	unsigned char RF_RxGain[8];
	unsigned char RF_TxGain[8];
	unsigned short SYN_TxGain;
	unsigned short SYN_RxGain;
};
	
struct T_ConfigTimeSlot_Msg{
	unsigned short transID;
	unsigned char TimeSlotNum;
	unsigned char DLTSNum;
};

struct T_ConfigGeneral_Msg{
	unsigned short transID;
	unsigned short SYNSRC;
	unsigned char RFPOWERONOFF;
	unsigned char SYNPOWERONOFF;
};

struct T_ConfigSYNSwitchEms_Msg{
	unsigned short transID;
	unsigned char SYNPOWERONOFF;
	unsigned char RFPOWERONOFF;
	unsigned short SYNSRC;
};
struct Code_Msg{
	unsigned short Seq_No;
	unsigned char Down_Code[102];
	unsigned short CRC;
};

struct T_ConfigCirCumsPara_Msg{
	unsigned short transID;
	unsigned char Max_Temp;
	unsigned char Min_Temp;
	unsigned char Max_Current;
	unsigned char  Min_Current;
//	unsigned short CRC;
};

struct T_Status_Ems_Msg{
	unsigned short transID;
	unsigned char RSV1;
	unsigned char RSV2;
	unsigned char status;
};

struct T_Carrier_Ems_Msg{
	unsigned short transID;
	unsigned char sequesID;
	unsigned char TotalSlotN;
	unsigned char DownSlotN;
};
struct T_WRRU_Ver{
	unsigned short transID;
	unsigned short  DSB_HW_version;
	unsigned int MCU_SW_version;
	unsigned int FPGA_SW_version;
	
};

struct T_WRRU_HW_STATUS{
	unsigned short transID;
	unsigned short  DSB_HW_Temp;
	unsigned short DSB_HW_Current;
	unsigned short Fiber_Delay;
	unsigned short  Freq_Offset;//wangwenhua modify 20111207
	unsigned short  RSV;
};

struct T_Calibrat_TR{
	char PRED_H[22];
	unsigned short antennaCalibrationResult;
	unsigned short TxGain;
	unsigned short RxGain;
};

struct T_Calibrat_Ems_Msg{
	unsigned short transID;
	T_Calibrat_TR TR_Cali[8];
	unsigned short SYN_TxGain;
	unsigned short SYN_RxGain;
};

struct T_SPF_Ver_Msg{
	#if 1
	unsigned short transID;
	unsigned char  Value[96];
	/*unsigned char physical_device;
	unsigned char function_defined_by_serial_ID;
	unsigned char LC_optical_connecter;
	unsigned char sonet_reach_specifier;
	unsigned char normal_bit_rate;
	unsigned char link_length_km;
	unsigned char OUI[3];
	unsigned char SPF_identifier[13];
	unsigned short serial_No[8];*/
	#endif
	
};

#pragma pack()


/**timer expires***/

/***alarm type*******/	

class CWRRU:public CBizTask
{
	public:
		/****test**************/
		unsigned short CheckSum(unsigned short * msg, unsigned short length);
		

	private:
		#define WRRU_MAX_BLOCKED_TIME_IN_10ms_TICK (500)

		static CWRRU *instance;
              WRRU_Data data_WRRU;

	       char Calibration_HW_Data[1200*2];
		unsigned char flag_rf_ver_get;
		unsigned short rf_ver[44];
		unsigned short rf_ver2[44];
		unsigned char  DSB_SYN_ver[44];
		unsigned short m_transid;
		//unsigned short SYN_Ver[22];
              static SEM_ID    s_semWriteNvram;

		 int  GetMaxBlockedTime() { return WRRU_MAX_BLOCKED_TIME_IN_10ms_TICK ;};
              void RRU_ProceedCirRRU(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth);
              void RRU_ProceedResetRRU(unsigned char MsgId,unsigned char Lenth);
              void RRU_ProceedSYNSwithRRU(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth);
              void RRU_ProcessRFCFGEmsMsg(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth);
              void RRU_ProcessCarrierEmsMsg(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth);
              void RRU_ProcessCalibratEmsMsg(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth);
              void RRU_ProceedConfigRRUToRRU(CComMessage *pComMsg,unsigned char MsgId,unsigned char Lenth);
              void RRU_ProceedRsvRRU(CComMessage *pComMsg,unsigned char MsgId,unsigned char Lenth);
              void RRU_ProcessCaliHW(CComMessage *pComMsg);
              void RRU_ProceedQueryVer(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth);
		 void RRU_ProcessQueryHwStatus(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth);
		void RRU_RRUSPFToEMS(T_SPF_Ver_Msg*result_msg,UINT16 MsgID);
		void RRU_RRUVERToEMS(T_WRRU_Ver *result_msg,UINT16 MsgID);
		void RRU_HwStatusToEMS(CComMessage *InMsg/*T_WRRU_HW_STATUS *result_msg*/,UINT16 MsgID);
              void RRU_RRURFVerToEMS(unsigned char flag,UINT16 *result_msg,UINT16 MsgID);
		   void RRU_CFG_AntennaMask(unsigned char anntenamask);
			 void RRU_Read_CSI_Info(CComMessage *pComMsg);//wangwenhua add 20110930 read rru 的csi Info
		   void RRU_Read_Fiber_Info(CComMessage *pComMsg);
 		//void RRU_CFG_RFMask(unsigned char anntenaidx);
              
              void Process_Link_msg(void);
              unsigned short Link_Frame_No;
              unsigned char send_comm_alm_flag;
		unsigned char  send_temp_alm_flag;
		unsigned char  send_current_alm_flag;
		unsigned char  send_syn_alm_flag;
		unsigned char  send_Ad401_alm_flag;
		CTimer *pWRRULinkTimer;
		CTimer *pWRRULinkTimer_Report;//wangwenhua add 20110525
		CTimer  *pWRRUCfgTimer; //WRRU消息timer
		CTimer  *pWRRUCfgTimer_other; //WRRU消息timer
		CTimer  *pWrruLoadCodeTimer;
		CTimer  *pWBBURRU_Run_Timer;//如果3分钟内RRU没有起来，则复位BBU作为保护
		CTimer  *pWBBURRU_REST_Timer;//如果40s内没有收到复位的信息，则复位RRU
		CTimer* SYS_Createtimer(UINT16 MsgId, UINT8 IsPeriod, UINT32 TimerPeriod);
		void RRU_ProcessLinkMsg(CComMessage* pComMsg);
	public:	
		unsigned short link_packet_flag;
		unsigned short link_flag; 
		unsigned char  m_cfg_times;
		unsigned char  m_cfg_times_other ;
		unsigned char  m_antennaMask;
		unsigned char m_send_anntennaMask;//表示是否已经发送了RF开关.
		unsigned char m_rru_state ;
		char    m_temp;
		int m_current;
		unsigned char m_bitmap;//记录发送数据
		
              /*****************************************************

                7 6 5 4 3 2                                                                      1            0 bit
                                                                                                                
                               5                                                                       4           3 msg id


              ********************************************************/
		  
		CWRRU ();
		TID GetEntityId() const;
		bool Initialize();
		bool ProcessComMessage(CComMessage* pComMsg);
		~CWRRU ();
		static CWRRU *GetInstance();
       	void	Process_EMS_msg(CComMessage* pComMsg);
		void Process_WRRU_msg(CComMessage* pComMsg);
		void RRU_RRUResToEMS(T_RSV_Msg *result_msg,UINT16 MsgID);
		void RRU_NvramCirBackEms(UINT16 *tranID,UINT16 MsgId);
		void RRU_StatusEms(UINT16 *tranID,UINT16 MsgId);
		void RRU_NvramSYNBackEms(UINT16 *tranID,UINT16 MsgId);
		void l3rrubspNvRamWrite(char * TargAddr, char *ScrBuff, int size);
		void change_bytes(unsigned char *p);
		void RRU_SendCaliHWToAUX(unsigned char flag);
		void RRU_CFG_RFMask(unsigned char anntenaidx,unsigned char flag);
		void RRU_Start_CFG_Timer();//启动配置定时器
		void RRU_Start_CFG_Timer_other();//启动配置定时器
		void print_wrru_state();
		void RRU_FiberInfoToEMS(UINT16 *tranID,UINT16 MsgId);
		void RRU_ReadFiberInfo(unsigned short *transid) ;
		unsigned char RRU_Get_Anntena_MASK(){return m_antennaMask;};
		FILE *file_mcu, *file_fpga;
		unsigned short   m_last_mcu;
		unsigned short   m_last_fpga;
		unsigned char mcu_end_flag;
		unsigned char fpga_end_flag;
		 unsigned char mcu_code[8][112];
		  unsigned char fpga_code[8][112];

};


#endif /* _L3WRRU_H */


