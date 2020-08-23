/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 文件名称:  bsp_afc.h
* 功    能:  为bsp_afc.c提供头文件
* 版    本:  V0.1
* 编写日期:  2014/1/16
* 说    明:  无
* 修改历史:
* 修改日期           修改人  BugID/CRID     修改内容
*------------------------------------------------------------------------------
*                                                         创建文件
*
*
*******************************************************************************/
/******************************** 头文件保护开头 ******************************/
#ifndef DD_AFC_H
#define DD_AFC_H

/******************************** 包含文件声明 ********************************/

/***************************************/
/******************************** 宏和常量定义 ********************************/
#define AFC_DD_INITED                                    1
#define AFC_DD_NOT_INITED                 				 0
#define COUNTER_61M44                                    61440000
//#define COUNTER_61M44                                  122880000

#define MAX_PHASE_DIF_LEN                        		 128
#define MAX_NS_FREQ_DIF_LEN                      		 10
#define NS_PREQ_LEN                                      6
#define NS_PHASE_DIF_LEN                            	 30 
#define PHASE_DIF_FILTER_VALUE                 			 200
#define AFC_COMP_TIME_LENGTH    						 2880
#define AFC_COMP_UPDATE_LENGTH   						 60

#define MIN_DAC_VALUE                            		 0x0
#define MAX_DAC_VALUE                           		 0xffff
#define DEFAULT_DAC_VALUE                   			 0x7fff

#define DAC_VALUE_REF_LOW                  				 0x3fff 
#define DAC_VALUE_REF_HIGH                 				 0xbfff

//#define AFC_LOCK_FREDIF_AVG_MIN          				 0.05
#define AFC_LOCK_FREDIF_AVG_MIN          				 0.1
#define AFC_LOCK_FREDIF_REF                     		 3
#define AFC_FREQ_DIF_REF                           		 3
#define AFC_FREQ_DIF_AVG_REF                  			 0.4
#define AFC_LOCK_FREDIF_AVG_REF          				 0.2
#define AFC_LOCK_PHASEDIF_REF               			 25
#define AFC_LOCK_PHASEDIF_AVG_REF      					 20
#define AFC_PHASE_DIF_REF                       		 90
#define FREQ_MODU_FRQDIF_REF                       		 12
#define FREQ_MODU_PHASEDIF_REF                           15 
#define FREQ_MODU_PHASEDIF_AVG_REF                       15
#define PHASE_MODU_PHASEDIF_REF                          30
#define PHASE_MODU_PHASEDIF_AVG_REF                      30

#define FREQUENCE_MODULATION_ENABLE                      1   
#define FREQUENCE_MODULATION_DISABLE                     0   

#define PHASE_MODULATION_ENABLE                          1   
#define PHASE_MODULATION_DISABLE                         0   

#define POWER_ON_RESET_MASK                              0x80
#define WATCH_DOG_RESET_MASK                             0x40
#define GLOBAL_BUTTON_RESET_MASK                         0x20
#define SF_CMD_RESET_MASK                                0x10

#define AFC_STARTUP_GPS_AVAILABLE_TIMES_MIN              5 
#define AFC_WARMUP_GPS_AVAILABLE_TIMES_MIN               5 
#define AFC_FAST_GPS_NOT_AVAILABLE_TIMES_MAX             30
#define AFC_FAST_GPS_AVAILABLE_TIMES_MIN                 30 
#define AFC_FAST_NOT_LOCK_TIMES_MAX                      900
#define AFC_HOLDOVER_GPS_AVAILABLE_TIMES_MIN             5
#define AFC_HOLDOVER_GPS_AVAILABLE_LOCK_TIMES_MIN        30
#define AFC_HOLDOVER_NOT_LOCK_TIMES_MAX                  900

#define AFC_COMP_INTERVAL                                30  
#define AFC_HOLDOVER_TIME_MAX                            28800
#define CNT_WINDOWS                                      50


#define AFC_STARTUP                                       0
#define AFC_WARMUP                                        1
#define AFC_FAST                                          2
#define AFC_LOCK                                          3
#define AFC_HOLDOVER                                      4
#define AFC_HOLDOVER_TIMEOUT                              5
#define AFC_ABNORMAL                                      6
#define AFC_LOCAL                                         7
#define AFC_STATE_MAXVALUE		                          8


#define AFC_PRINT_LEVEL_NORMAL          				  0
#define AFC_PRINT_LEVEL_DEBUG            				  1
#define AFC_PRINT_LEVEL_ALL                     		  2

#define SYNSRC_GPS                             			        0
#define SYNSRC_LOCAL                            		        1
#define SYNSRC_BD                          			        2
#define SYNSRC_FIBER                           			  3
#define SYNSRC_1588                            			  4
#define SYNSRC_485                                                    5
#define SYNSRC_ES                              			         6
#define SYNSRC_MIX_GPS                         			  7
#define SYNSRC_MIX_BD                          			  8
#define SYNSRC_1PPS                            			  9
#define SYNSRC_1PPS_TOD                        			  10
#define SYNSRC_CASCADE                                    255


#define E_AFC                                  			  -1
#define AFC_OK                                  		  0
#define AFC_ERROR                              			  -1


#if 0
#undef AFC_DBG_ON
#else
#define AFC_DBG_ON
#endif
#if defined(AFC_DBG_ON)
	#define afc_dbg(format, arg...) printf("AFC_DEBUG[%s:%s():%u]- " format,	__FILE__,__func__, __LINE__,  ##arg)
#else
#define afc_dbg(arg...)
#endif

#define interpro_syn_status_change_notify /\
/interpro_syn_status_change_notify
#define AFC_LOG_LENTH                   40000//1801

/******************************** 类型定义 ************************************/
typedef struct
{
	u32 FastUnlockCnt;
	u32 HoldoverCnt;
    u32 HoldoverTimeoutCnt;
	u32 LockTimesCnt;
	u32 HoldoverTimesCnt ;
    u32 HoldoverTimeoutTimesCnt ;
}STRU_AFC_COUNTER;

typedef struct
{
	u16 DacValue ;
	u16 DacLastTime;
}STRU_AFC_DAC;

typedef struct
{
	u16   SpiInit;
	u16   PhaseLockStage;
	u32   SimuTime;
}STRU_AFC_STATE;

typedef struct
{
	u8  FrqAdjFlag;
	u8  Cnt20sChangeFlag;
	u8   Cnt20sHis;
	u8   Cnt20sHisLen;
	s32 CntDif1s;
	u32 Cnt1s;
	u32 Cnt20s;
	u32 Cnt20sGrp[MAX_NS_FREQ_DIF_LEN];
	float OCCoef;
	double CntDifAvg;
}STRU_AFC_FREQUENCE;

typedef struct
{
	u16 PhaseAdjFlag;
	u16  PhaseDifHis;
	s32 PhaseDif;
	s32 PhaseDifGrp[MAX_PHASE_DIF_LEN];
	double PhaseDifAvg;
}STRU_AFC_PHASE;



typedef struct 
{  
	s16 s16TempValue[1];
	u8 u8TempNum;
}SSP_STRU_TEMP;


typedef struct 
{  
	u8 u8ResetCause;
}STRU_RESET_CAUSE;


typedef struct
{
	u32  LenthFlag;
	u32  CalCnt;
	u32 CompCnt;
	 SSP_STRU_TEMP  Temperature;
	double  DACGrp[AFC_COMP_TIME_LENGTH];
	double  TemperatureGrp[AFC_COMP_TIME_LENGTH];
	double DACCoef;
	double DACCoef_adjust;
	double TimeCoef;
	double TemperatureCoef;
}STRU_AFC_COMP;

typedef struct MSG_GPS_AFC_STATE_STRU
{
	s32 s32PhaseDif;
	u32 u32DacValue;
	u32 GpsAvailable;
	u32 u32GpsCnt25M;
	u32 u32LocalCnt25M;
	u32 u32GpsCnt6144M;
	u32 u32LocalCnt6144M;
	u32 u32LMK3000UnLockCnt;
	u16 u16AheadData;
	u16 u16LagData;
}MSG_GPS_AFC_STATE;

typedef struct
{
	u16 DacValue ;
	float OCCoef;
}STRU_AFC_DAC_SAVE;
/******************************** 全局变量声明 ********************************/
/******************************** 外部函数原形声明 ****************************/
extern s32 afc_sim_switch(u32 u32Arg);
extern u8 ssd_get_hw_reset_cuase(void);
extern s32  gps_lock_status_modify(u8 u8Status);
extern s32  gps_check_clock(void);
extern s32 GPS_PRINT_VAR(void);

/******************************** 头文件保护结尾 ******************************/
#endif /*DD_AFC_H*/
/******************************** 头文件结束 **********************************/

