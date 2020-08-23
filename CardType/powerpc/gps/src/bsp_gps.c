/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_gps.c 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>  
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/inotify.h>

/**************************** 私用头文件* **********************************/
#include "bsp_types.h"
#include "bsp_epld_ext.h"
#include "bsp_fpga_ext.h"
#include "../inc/bsp_gps.h"
#include "../../afc/inc/bsp_afc.h"
#include "bsp_gps_ext.h"
#include "bsp_intpro_ext.h"
#include "fsl_p2041_ext.h"
#include "bsp_external.h"

/******************************* 局部宏定义 *********************************/
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>
 #include <sys/time.h>

/*********************** 全局变量定义/初始化 **************************/
STRU_GPS_FLAG  g_struGpsFlag;/*相关标志位*/
/************************** 局部常数和类型定义 ************************/
pthread_t  t_gps_connet_thread; 
#define BSP_ALM_TYPE_FPGA             0X0a
#define BSP_ALM_TYPE_CPLD             0x12
#define BSP_ALM_TYPE_CPU_PF           0x13
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

#define BSP_ALM_CLASS_CRITICAL       1
#define BSP_ALM_CLASS_MAJOR          2
#define BSP_ALM_CLASS_MINOR          3
#define BSP_ALM_CLASS_INFO           4

#define BSP_DATARSP_BUF_LEN         (1500)
sem_t g_sem_tod_sec_exe;
sem_t g_sem_sched_regulate ;
int g_gps_led_flicker=0;
pthread_mutex_t g_mp_gpsvar = PTHREAD_MUTEX_INITIALIZER;  
pthread_mutex_t g_mp_rpt_msg = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t g_mp_gps = PTHREAD_MUTEX_INITIALIZER;  
pthread_mutex_t g_uart_mutex = PTHREAD_MUTEX_INITIALIZER;  

u32 g_IntGpsUartFd;

static void (*gps_pos_get_callback)();

u32 g_IntGpsUartFd;


BOOL isFirstMsg = TRUE;//第一条判断是否为ublox消息
T_GpsAllData gUbloxGpsAllData;
UBLOX_SData gUbloxSData;
Clock_Set_Tc_Type g_Clock_Set_AllDate;
extern u16 bts_synsrc_type;
s32 gps_GMT_Offset = 480;
u32 gLeapSecond = 17;
s8 g_UbloxOn = 1;
s32 gps_update_rtc_interval;
s32 gps_update_rtc_valid;
s32 gps_update_rtc_oldvalid = 0;
s32 g_gps_alarm_flag = 0;

u8 g_ublox_gps_type =0;
u8 g_GNSS_flag = 0;  /* 0 ---gps;  1----BEIDOU*; 2---local clock*/
u8 g_type_Flag = 0;
u8 g_SynWay_Flag = 0;
u8 g_CascadeCfg_Flag = 0;
unsigned int g_NtpdBbuConfig = 0;
unsigned char *gps_type_file = "/mnt/csi/gps.txt";


unsigned char *gps_file = "/tmp/gps_data";
struct Mesh_Lte_Com_Data{
	T_GpsAllData UbloxGpsAllData;
	STRU_GPS_FLAG struGpsFlag;
};
#define GPS_DELAY(a)         {vu32 _i_;  for (_i_ = 0; _i_ < 4 * (a); _i_++) {__asm(" sync");}}

enum
{
    /*gps*/
    BSP_TC_TYPE_GPS,
    /*local oscillator*/
    BSP_TC_TYPE_LO,
	/*beidou*/
	BSP_TC_TYPE_BEIDOU,
	/*cascade*/
	BSP_TC_TYPE_CASCADE,
	BSP_TC_TYPE_1588,
	BSP_TC_TYPE_1PPS_TOD,
	BSP_TC_TYPE_ES,
};

//#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
#define SYNSRC_MAX_NUM 					(32)

//typedef void (*SynSrcAlarmFuncPtr)(u16 u16EventId, T_SynSrcAlmInfo *pt_info);
typedef void(*pfSynSrcSwitch)(void);
typedef enum {SYNSRC_SWITCH_AUTO = 0, SYNSRC_SWITCH_CONFIG_SINGLE_SYNSRC = 1, SYNSRC_SWITCH_CONFIG_MULTI_SYNSRC = 2} SynSrcSwithPattern;
typedef struct {
	s32 SynSrcType;
	SynSrcSwithPattern SynSrcSwitchPat;
	pfSynSrcSwitch pReceiveReturn;
}SynSrcSwitchInfo;

SynSrcAlarmFuncPtr pfSynSrcAlmFunc = NULL;
SynSrcSwitchInfo syn_src_switch_info[SYNSRC_MAX_NUM];
pthread_mutex_t g_mp_synsrc_switch = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond_synsrc_switch;
//#endif

/*************************** 局部函数原型声明 **************************/
s32 ubx_init(void);
s32 gps_uart_set(void);
//s32 gps_Cfg_Tp1(void);
s32 gps_Cfg_Tp1(u8 u8Flag);
s32 gps_Cfg_Tp2(void);
s32 gps_Cfg_Tp1BD(u8 u8Flag);
s32 gps_Cfg_Tp2BD(void);
s32 gps_Cfg_Tp1b(void); 
s32 gps_Cfg_Tp2b(void); 

s32 gps_NAV_PosLLH(void);
void gps_check(void);


s32 gps_NAV_TimeUTC(s32 gps_gmt_offset);
s32 gps_NAV_SVInfo(void);
s32 t_gps_func(void);

u8 set_gps_data(void);
u8 get_gps_data(void);
s32 gps_HW_VER(void);
s32 ublox_comp_init(void);
s32 config_to_beidou(void);
s32 config_to_gps(void);
u8 bsp_init_get_gps_tc_type(u8 Value);
u8 bsp_switch_synsrc(u16 synsrc);
u32 bsp_update_rtc_check(void);
s32 SynSrcAlmFunc(u16 u16EventId, T_SynSrcAlmInfo *pt_info);

/************************************ 函数实现 ************************* ****/

/**************************************************************
* void cal_cs(u8 *pu8Date, u16 u16Len)
*
*
*
****************************************************************/
u8 calc_buffer[56]={0x06, 0x3E, 0x34, 0x00, 0x0, 0x20, 0x20, 0x06, 
	                  0x0, 0x8, 0x10, 0x0, 0x1,0x0, 0x1, 0x1, 
	                  0x1, 0x1, 0x3, 0x0, 0x0, 0x0, 0x1, 0x1,
	                  0x3, 0x8, 0x10,0x0, 0x1, 0x0, 0x1, 0x1,
	                  0x4, 0x0, 0x8, 0x0, 0x0, 0x0, 0x1, 0x3,
	                  0x5, 0x0, 0x3, 0x0, 0x0, 0x0, 0x1, 0x5,
	                  0x6, 0x8, 0xE, 0x0, 0x0, 0x0, 0x1, 0x1};
void cal_cs(u8 *pu8Date, u16 u16Len)
{
	 int  CK_A = 0, CK_B = 0;
	 int i;
 
	 for(i = 0; i < u16Len; i++)
	 {
	     CK_A = CK_A + pu8Date[i];
	     CK_B = CK_B + CK_A;
	 }
	 //printf("ck_a=0x%x,ck_b=0x%x\n", CK_A&0xff, CK_B&0xff);

     pu8Date[u16Len] = CK_A&0xff;
     pu8Date[u16Len + 1] = CK_B&0xff;
}

/*******************************************************************************
* 函数名称: bsp_gps_gpsvar_mutex_init						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 bsp_gps_gpsvar_mutex_init(void)
{
	s32 s32ret = 0;
	s32ret =pthread_mutex_init(&g_mp_gpsvar, NULL);
	return s32ret;
}

/*******************************************************************************
* 函数名称: bsp_gps_gpsvar_mutex_init						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 bsp_gps_rpt_mutex_init(void)
{
	s32 s32ret = 0;
	s32ret =pthread_mutex_init(&g_mp_rpt_msg, NULL);
	return s32ret;
}

/*******************************************************************************
* 函数名称: bsp_gps_mutex_init						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 bsp_gps_mutex_init(void)
{
	s32 s32ret = 0;
	s32ret |=pthread_mutex_init(&g_mp_gps, NULL);
	return s32ret;
}

s32 bsp_gps_mutexalldata_init(void)
{
	s32 s32ret = 0;
	s32ret |=pthread_mutex_init(&(gUbloxGpsAllData.m_mutex), NULL);
	return s32ret;
}

s32 bsp_gps_mutexubloxdata_init(void)
{
	s32 s32ret = 0;
	s32ret |=pthread_mutex_init(&(gUbloxSData.m_mutex), NULL);
	return s32ret;
}

/*******************************************************************************
* 函数名称: bsp_gps_update_rtc						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 bsp_gps_update_rtc(void)
{
	char pbuffer[7];
#ifdef __CPU_LTE_MACRO__
	if(2 == g_GNSS_flag){
		bd_NAV_TimeBD(gps_GMT_Offset,gLeapSecond);;
	}else if (0 == g_GNSS_flag){
		gps_NAV_TimeGPS(gps_GMT_Offset,gLeapSecond);
	}
	
	pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
	pbuffer[0] = gUbloxGpsAllData.Second;//&0x7f;  /******s********/
	pbuffer[1] = gUbloxGpsAllData.Minute;//&0x7f;  /******min*******/
	pbuffer[2] = gUbloxGpsAllData.Hour;//&0x3f;  /*********hour*******/
	pbuffer[3] = gUbloxGpsAllData.Day;//&0x3f; /*******day********/
	pbuffer[4] = 5;//&0x7; /****week day*****/
	pbuffer[5] = gUbloxGpsAllData.Month;//&0x1f; /*******month*******/
	pbuffer[6] = (gUbloxGpsAllData.Year-2000);//&0xff; /**year***/
	pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));

	if(bsp_set_rtc(pbuffer)!= BSP_OK)
	{
		printf("bsp_set_rtc error!");
		return BSP_ERROR;
	}
#endif
	if(bsp_set_system_time()!=BSP_OK)
	{
	    printf("bsp_set_system_time error!");
		return BSP_ERROR;
	}
	return BSP_OK;
}
#if 0
unsigned int BspGpsPosGetCallBackReg(GPS_POS_GET pCallBack)
{
    gps_pos_get_callback = pCallBack;
    return 0;
}
#endif

#if 0
void bsp_update_gpspos()
{
    if(afc_is_lock())
    {
        if(gps_NAV_PosLLH() != OK)
            printf("get gps pos error.\n");
        else
        {
	#if 0
            if(gps_pos_get_callback != NULL)
                (gps_pos_get_callback)();
	#endif
        }
    }
}
#endif

/*******************************************************************************
* 函数名称: bsp_gps_init						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 bsp_gps_init(void)
{
	s32 s32ret;
	u32 u32i;
    int res=0;
	int fd;
    pthread_t ptid;
    pthread_t       a_thread;
    pthread_attr_t  attr;
    struct sched_param parm;
	unsigned char touchFileCmd[100] = "";
	

    bsp_gps_rpt_mutex_init();
	bsp_gps_mutex_init();
	bsp_gps_mutexalldata_init();
	bsp_gps_mutexubloxdata_init();
	
	printf("\nGPS INIT lte.......\n");	
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	sprintf(touchFileCmd, "touch %s", gps_file);
	system(touchFileCmd);
#endif	
	fd = open(gps_type_file, O_RDONLY);
	if(fd >= 0)
	{
		
		read(fd, &g_Clock_Set_AllDate, sizeof(g_Clock_Set_AllDate));
		close(fd);

	}else 
	{
		g_Clock_Set_AllDate.gps_Type = 0;
		g_Clock_Set_AllDate.gps_ClockSynWay = 0;
		g_Clock_Set_AllDate.gps_CascadeCfg = 0xFF;
		fd = open(gps_type_file, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd < 0)
		{
			printf("creat gps.txt error\n");
		}
		write(fd, &g_Clock_Set_AllDate, sizeof(g_Clock_Set_AllDate));
		printf("the first write gps.txt\n");
		close(fd);
	}
	printf("SynSrc is %d.\n", g_Clock_Set_AllDate.gps_Type);
	g_GNSS_flag = g_Clock_Set_AllDate.gps_Type;		

	printf("[%s]:before gps_uart_set func!\n",__func__);
	if(GPS_OK != gps_uart_set())
	{
		bsp_dbg("gps_uart_set error!\n");
	}
	
	printf("[%s]:before ubx_init func!\n",__func__);
	if(GPS_OK != ubx_init())
	{
		bsp_dbg("\nubx_init error!\n");
	}
	printf("[%s]:after ubx_init func!\n",__func__);
	for(u32i = 0; u32i<2;u32i++)
	{
		if(2 == g_GNSS_flag){
			bd_NAV_TimeBD(gps_GMT_Offset,gLeapSecond);;
		}else if (0 == g_GNSS_flag){
			gps_NAV_TimeGPS(gps_GMT_Offset,gLeapSecond);
		}
	    (void)gps_NAV_SVInfo();
	}
	printf("[%s]:before bsp_set_mct_system_time func!\n",__func__);
	if(bsp_set_system_time()!=BSP_OK)
	{
	    printf("the first set system time error!\n");
		
	}
#ifdef __CPU_LTE_MACRO__	
#if 1
		// 级联从
	if ((g_Clock_Set_AllDate.gps_Type == 0xFF) && (g_Clock_Set_AllDate.gps_ClockSynWay == 1) && (g_Clock_Set_AllDate.gps_CascadeCfg == 1))
	{
		bsp_fpga_write_reg(401,0x1);
		printf("cascade slave\n");
		printf("FPGA_SERIAL_CTRL_REG(401) = %x\n", bsp_fpga_read_reg(FPGA_SERIAL_CTRL_REG));
		printf("CPLD_AFC_CTRL_REG = %x\n", bsp_cpld_read_reg(CPLD_AFC_CTRL_REG));
	}	
	// gps beidou local
	if ((g_Clock_Set_AllDate.gps_Type != 0xFF) && (g_Clock_Set_AllDate.gps_ClockSynWay == 1) && (g_Clock_Set_AllDate.gps_CascadeCfg == 0))
	{
		bsp_fpga_write_reg(FPGA_SERIAL_CTRL_REG,0x0);
		printf("gps_beidou cascade master\n");
		printf("FPGA_SERIAL_CTRL_REG = %x\n", bsp_fpga_read_reg(FPGA_SERIAL_CTRL_REG));
		printf("CPLD_AFC_CTRL_REG = %x\n", bsp_cpld_read_reg(CPLD_AFC_CTRL_REG));
	}

	if (((g_Clock_Set_AllDate.gps_Type == 0) || (g_Clock_Set_AllDate.gps_Type == 2)) && (g_Clock_Set_AllDate.gps_ClockSynWay == 0) && (g_Clock_Set_AllDate.gps_CascadeCfg == 0xFF))
	{
		bsp_fpga_write_reg(FPGA_SERIAL_CTRL_REG,0x0);
	}
#endif
#endif
    #if 0
	s32ret = pthread_create(&t_gps_connet_thread, 0, (void *)t_gps_func, 0);
	if (s32ret < 0)
	{
		bsp_dbg( "t_afc_func task create failed!\n");
		return GPS_ERROR;
	}
    #else
    pthread_attr_init(&attr); 
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 30; 
    pthread_attr_setschedparam(&attr, &parm);
    res = pthread_create(&ptid, &attr, (FUNCPTR)t_gps_func,NULL);
    pthread_attr_destroy(&attr);
    if (-1 == res)
    {
        perror("create gps thread error!\n");
    }

    #endif

#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	if(g_Clock_Set_AllDate.gps_Type == SYNC_SOURCE_1PPS_TOD)
	{
		syn_ch_to_1pps_tod(); 
	}
	else if(g_Clock_Set_AllDate.gps_Type == SYNC_SOURCE_1588)
	{
		//set_1588_running_flag(TRUE); 
	}
#endif

	bsp_dbg("bsp_gps_init success!\n");

	return GPS_OK;

}

pthread_mutex_t local_time_mutex = PTHREAD_MUTEX_INITIALIZER;
typedef struct
{
    UINT16  year;		
    UCHAR   month;
    UCHAR   day;
    UCHAR   hour;		
    UCHAR   minute;		
    UCHAR   second;		
}T_TimeDate;

void taskLocalTime(const time_t *timep,struct tm *result)
{
    if ( NULL != timep && NULL != result )
    {
        pthread_mutex_lock(&local_time_mutex);
        (void)localtime_r(timep,result);
        pthread_mutex_unlock(&local_time_mutex);
    }
    else
    {
        printf("taskLocalTime:the par in is NULL!\n");
    }
    return;
}

T_TimeDate bsp_get_time()
{
    T_TimeDate pDateTime; 
    struct tm result;
    time_t timep; 
    struct tm *p; 
    time(&timep); 
    
    taskLocalTime(&timep, &result);
    pDateTime.year = 1900+result.tm_year;		
    pDateTime.month =  1+result.tm_mon;
    pDateTime.day = result.tm_mday;
    pDateTime.hour = result.tm_hour;		
    pDateTime.minute = result.tm_min;		
    pDateTime.second = result.tm_sec;	
    return pDateTime;	
}

/*******************************************************************************
* 函数名称: t_gps_func
* 功    能: 
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
unsigned int g_UpDateRtc = 0;
unsigned int g_IntUpDateRtc = 0;
unsigned int g_UpLongitudeLatitude = 0;
unsigned int cnt=0;
s32 t_gps_func(void)
{
    u32 u32i;
    T_TimeDate pDateTime;
	u32 u32UpDateRtcTimes = 0;
	u32 u32Ret = 0;
	static u32 LongitudeLatitudeCnt = 0;
    bsp_print_reg_info(__func__, __FILE__, __LINE__);
    for(;;)
    {
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)

    	if(0 != g_Clock_Set_AllDate.gps_Type &&  2 != g_Clock_Set_AllDate.gps_Type) 
    	{
    	}
		else /* gps or beidou */
		{
#endif
        interpro_send_gps_info();
        if(gps_update_rtc_valid)
        {
            gps_update_rtc_interval++;
        }
        pDateTime =  bsp_get_time();
        if((gps_update_rtc_interval >= 20)||(gps_update_rtc_oldvalid == 0)||(pDateTime.year != gUbloxGpsAllData.Year))
        {
			if (gUbloxGpsAllData.TrackedSatellites >= 3)
            {
                gps_update_rtc_interval = 0;
                
				if (g_IntUpDateRtc == 0)
				{
					printf("IntUpDateRtc...\n");					
					while(u32UpDateRtcTimes < 3)
					{
						printf("u32UpDateRtcTimes : %d\n", u32UpDateRtcTimes);
						bsp_gps_update_rtc();
						u32Ret = bsp_update_rtc_check();
						if (u32Ret != BSP_OK)
						{
							u32UpDateRtcTimes++;
							sleep(1);	
						}else
						{
							break;
						}
					}
					g_IntUpDateRtc = 1;		
				}else if (g_NtpdBbuConfig == 1)
				{
					g_UpDateRtc++;
					if(bsp_gps_update_rtc()!= BSP_OK)
					{
						bsp_dbg("bsp_gps_update_rtc error!\n");
					}
					//printf("NTP:  UpDateRtc...\n");
				}else 
				{
					//printf("NTP:  Don't UpDateRtc...\n");
				}				
				if ((LongitudeLatitudeCnt++ % 5) == 0)
				{
					g_UpLongitudeLatitude++;
					gps_NAV_PosLLH();
				}
            }else{
				while (cnt < 10)
				{	
					sleep(3);
					if (gUbloxGpsAllData.TrackedSatellites >= 3)
					{
						break;
					}else{
						cnt++;
					}				
				}			
			}          
        }
        gps_update_rtc_oldvalid = gps_update_rtc_valid;
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
    	}/* gps or beidou */
#endif
        sleep(27);
    }
}

/*******************************************************************************
* 函数名称: t_gps_func
* 功    能: 
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称          类型        输入/输出       描述
*******************************************************************************/
s32 gps_uart_set(void)
{
   	struct termios Opt;
    #ifdef NORMAL_MODE
	g_IntGpsUartFd = open( "/dev/ttyS0", O_RDWR |O_NOCTTY | O_NDELAY);
	#else
	g_IntGpsUartFd = open( "/dev/ttyS0", O_RDWR |O_NOCTTY | O_NONBLOCK/*O_NDELAY*/);
	#endif
	if (ERROR == g_IntGpsUartFd)
	{
		bsp_dbg( "Open  g_IntGpsUartFd failed!\n");
		return GPS_ERROR;
	}

	if(tcgetattr(g_IntGpsUartFd, &Opt) != 0)
	{
		perror("tcgetattr fd");
		return GPS_ERROR;
	}

	Opt.c_cflag &= ~CSIZE;
	Opt.c_cflag |= CS8;

	Opt.c_cflag |= (CLOCAL | CREAD);

	Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	Opt.c_oflag &= ~OPOST;
	Opt.c_oflag &= ~(ONLCR | OCRNL); 

	Opt.c_iflag &= ~(ICRNL | INLCR);
	Opt.c_iflag &= ~(IXON | IXOFF | IXANY); 

	tcflush(g_IntGpsUartFd, TCIFLUSH);
	Opt.c_cc[VTIME] = 0; 
	Opt.c_cc[VMIN] = 0; 

	if(tcsetattr(g_IntGpsUartFd, TCSANOW, &Opt) != 0)
	{
		perror("tcsetattr g_IntGpsUartFd");
		return GPS_ERROR;
	}
	
	return GPS_OK;
	
}

s32 gps_uart_mode(int isCharMode)
{
   	struct termios Opt;
	static int uartMode = 0;
	if(uartMode==isCharMode)
		return BSP_OK;
		
	pthread_mutex_lock(&g_uart_mutex);
	uartMode = isCharMode;
	
	if(tcgetattr(g_IntGpsUartFd, &Opt) != 0)
	{
		perror("tcgetattr fd");
        pthread_mutex_unlock(&g_uart_mutex);
		return GPS_ERROR;
	}
	
	Opt.c_cflag &= ~CSIZE;
	Opt.c_cflag |= CS8;
	Opt.c_cflag |= (CLOCAL | CREAD);
	Opt.c_lflag &= ~(ECHO | ECHOE | ISIG);
	
	if(isCharMode==1){
		Opt.c_lflag |= ICANON;
	}else{
		Opt.c_lflag &= ~ICANON;
	}
	Opt.c_oflag &= ~OPOST;
	Opt.c_oflag &= ~(ONLCR | OCRNL); 

	Opt.c_iflag &= ~(ICRNL | INLCR);
	Opt.c_iflag &= ~(IXON | IXOFF | IXANY); 

	tcflush(g_IntGpsUartFd, TCIFLUSH);
	Opt.c_cc[VTIME] = 0; 
	Opt.c_cc[VMIN] = 0; 
	if(tcsetattr(g_IntGpsUartFd, TCSANOW, &Opt) != 0)
	{
		perror("tcsetattr g_IntGpsUartFd");
        pthread_mutex_unlock(&g_uart_mutex);
		return GPS_ERROR;
	}
	
	pthread_mutex_unlock(&g_uart_mutex);	
	return GPS_OK;
}

/*******************************************************************************
* 函数名称: ubx_disable_sbas
* 功    能:设置UBX CFG-SBAS   禁用SBAS功能
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
* 返回值:
* BSP_OK:成功
* 说   明:
*******************************************************************************/
s32 ubx_disable_sbas(void)
{
	u8 u8WriteLen;
	u8 u8UBX_disable_sbas[16];
	u8 u8ck1 = 0;
	u8 u8ck2 = 0;
	u8 u8i;

	u8UBX_disable_sbas[0] = 0xB5;
	u8UBX_disable_sbas[1] = 0x62;
	u8UBX_disable_sbas[2] = 0x06;
	u8UBX_disable_sbas[3] = 0x16;
	u8UBX_disable_sbas[4] = 0x08;
	u8UBX_disable_sbas[5] = 0;

	u8UBX_disable_sbas[6] = 0;
	u8UBX_disable_sbas[7] = 1;
	u8UBX_disable_sbas[8] = 1;
	u8UBX_disable_sbas[9] = 0;
	u8UBX_disable_sbas[10] = 0;
	u8UBX_disable_sbas[11] = 0;
	u8UBX_disable_sbas[12] = 0;
	u8UBX_disable_sbas[13] = 0;

	for (u8i=2;u8i<14;u8i++)
	{
		u8ck1 = u8ck1 + u8UBX_disable_sbas[u8i];
		u8ck2 = u8ck2 + u8ck1;
	}
	u8UBX_disable_sbas[14] = u8ck1;
	u8UBX_disable_sbas[15] = u8ck2;

	pthread_mutex_lock(&g_mp_gps);
	u8WriteLen = (u8)write(g_IntGpsUartFd, (char *)u8UBX_disable_sbas, 16);
	pthread_mutex_unlock(&g_mp_gps);

	if (16 != u8WriteLen)
	{
	bsp_dbg("write error!%s,%d\n",__FUNCTION__,__LINE__);
		return E_GPS;
	}
	else
	{
		return GPS_OK;
	}
}

/*******************************************************************************
* 函数名称: ubx_close_nmea
* 功    能:设置UBX CFG-MSG   关闭NEMA输出
* 相关文档:
* 函数类型: extern
* 参    数:
* 参数名称          类型        输入/输出       描述
* 返回值:
* BSP_OK:成功
* 说   明:
*******************************************************************************/
s32 ubx_close_nmea(void)
{
	u8 u8WriteLen;
	u8 close_nmea_gga[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0, 0xF0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0x23};
	u8 close_nmea_gll[16] =   {0xB5, 0x62, 0x06, 0x01, 0x08, 0, 0xF0, 1, 0, 0, 0, 0, 0, 0, 0, 0x2A};
	u8 close_nmea_gsa[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0, 0xF0, 2, 0, 0, 0, 0, 0, 0, 1, 0x31};
	u8 close_nmea_gsv[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0, 0xF0, 3, 0, 0, 0, 0, 0, 0, 2, 0x38};
	u8 close_nmea_rmc[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0, 0xF0, 4, 0, 0, 0, 0, 0, 0, 3, 0x3F};
	u8 close_nmea_vtg[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0, 0xF0, 5, 0, 0, 0, 0, 0, 0, 4, 0x46};
	u8 close_nmea_zda[16] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0, 0xF0, 8, 0, 0, 0, 0, 0, 0, 7, 0x5B};
	
	pthread_mutex_lock(&g_mp_gps);
	u8WriteLen = (u8)write(g_IntGpsUartFd, (char *)close_nmea_gga, 16);
	if (16 != u8WriteLen)
	{
		bsp_dbg("write error!%s,%d\n",__FUNCTION__,__LINE__);
		pthread_mutex_unlock(&g_mp_gps);
		return GPS_ERROR;
	}
	
      GPS_DELAY(10);

	u8WriteLen = (u8)write(g_IntGpsUartFd, (char *)close_nmea_gll, 16);
	if (16 != u8WriteLen)
	{
		bsp_dbg("write error!%s,%d\n",__FUNCTION__,__LINE__);
	       pthread_mutex_unlock(&g_mp_gps);
		return GPS_ERROR;
	}
      GPS_DELAY(10);

	u8WriteLen = (u8)write(g_IntGpsUartFd, (char *)close_nmea_gsa, 16);
	if (16 != u8WriteLen)
	{
		bsp_dbg("write error!%s,%d\n",__FUNCTION__,__LINE__);
	       pthread_mutex_unlock(&g_mp_gps);
		return GPS_ERROR;
	}
      GPS_DELAY(10);

	u8WriteLen = (u8)write(g_IntGpsUartFd, (char *)close_nmea_gsv, 16);
	if (16 != u8WriteLen)
	{
		bsp_dbg("write error!%s,%d\n",__FUNCTION__,__LINE__);
	       pthread_mutex_unlock(&g_mp_gps);
		return GPS_ERROR;
	}
      GPS_DELAY(10);

	u8WriteLen = (u8)write(g_IntGpsUartFd, (char *)close_nmea_rmc, 16);
	if (16 != u8WriteLen)
	{
		bsp_dbg("write error!%s,%d\n",__FUNCTION__,__LINE__);
	       pthread_mutex_unlock(&g_mp_gps);
		return GPS_ERROR;
	}
      GPS_DELAY(10);

	u8WriteLen = (u8)write(g_IntGpsUartFd, (char *)close_nmea_vtg, 16);
	if (16 != u8WriteLen)
	{
		bsp_dbg("write error!%s,%d\n",__FUNCTION__,__LINE__);
	       pthread_mutex_unlock(&g_mp_gps);
		return GPS_ERROR;
	}
      GPS_DELAY(10);

	u8WriteLen = (u8)write(g_IntGpsUartFd, (char *)close_nmea_zda, 16);
	if (16 != u8WriteLen)
	{
		bsp_dbg("write error!%s,%d\n",__FUNCTION__,__LINE__);
	       pthread_mutex_unlock(&g_mp_gps);
		return GPS_ERROR;
	}
	pthread_mutex_unlock(&g_mp_gps);

      GPS_DELAY(10);  
	return GPS_OK;
}

/*******************************************************************************
* 函数名称: ubx_init						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 ubx_init(void)
{
	s32 s32ret;
	s32 s32i_tmp = 0;
	
	s32ret = ubx_close_nmea();
	if(s32ret != GPS_OK)
	{
		bsp_dbg("ubx_close_nmea error!\n");
		return GPS_ERROR;
	}
	
	printf("[%s]:before sleep 1 s\n",__func__);
	//sleep(1);
	for(s32i_tmp = 0;s32i_tmp < 100;s32i_tmp++)
	{
		bsp_delay_100ms();
	}
	printf("[%s]:after sleep 1 s\n",__func__);
	
	s32ret = ubx_disable_sbas();/*关闭SBAS功能*/
	if(s32ret != GPS_OK)
	{
		bsp_dbg("ubx_disable_sbas error!\n");
		return GPS_ERROR;
	}
	
	printf("[%s]:before gps_HW_VER func!\n",__func__);
	if(gps_HW_VER()==1)
	{
		g_ublox_gps_type = 1;		
		ublox_comp_init();
		
		s32ret = gps_Cfg_ANT();		
		if(s32ret != GPS_OK)
		{
			bsp_dbg("gps_Cfg_ANT error!\n");
			return GPS_ERROR;
		}
		if(2 == g_GNSS_flag)
        {
            s32ret = config_to_beidou();
			if(s32ret != GPS_OK)
			{
				bsp_dbg("config_to_gps error!\n");
				return GPS_ERROR;
			}
		}
		else
		{	
			s32ret = config_to_gps();
			if(s32ret != GPS_OK)
			{
				bsp_dbg("config_to_gps error!\n");
				return GPS_ERROR;
			}
		}
	}
	else
	{
	    g_ublox_gps_type = 0;
		s32ret = gps_Cfg_ANT_6T();
		if(s32ret != GPS_OK)
		{
			bsp_dbg("gps_Cfg_ANT_6T error!\n");
			return GPS_ERROR;
		}
		s32ret = config_to_gps();
		if(s32ret != GPS_OK)
    	{
		    bsp_dbg("config_to_gps error!\n");
    		return GPS_ERROR;
	    }
    }

	return GPS_OK;
}

/*******************************************************************************
* 函数名称: gps_check_clock
* 功    能: 该函数用来检查时钟是否可用
* 相关文档:
* 函数类型: SYS_STATUS
* 参    数:
* 参数名称          类型        输入/输出       描述
* 无
* 返回值: 状态GPS_OK,或者NB_ERROR
* 说   明:
*******************************************************************************/
s32  gps_check_clock(void)
{
#if !defined(__CPU_LTE_CENTERSTATION__)&&!defined(__CPU_LTE_CARDTYPE__)
	u16 ext_1pps_in_error =0;
	ext_1pps_in_error = bsp_fpga_read_reg(402);
#endif

#if 0
	if (GPS_AVAILABLE == g_struGpsFlag.GpsAvailable
        && gUbloxGpsAllData.TrackedSatellites >= 3)
#else
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	if ((SYNC_SOURCE_GPS == g_Clock_Set_AllDate.gps_Type ||
		SYNC_SOURCE_BD == g_Clock_Set_AllDate.gps_Type)  && 
		(gUbloxGpsAllData.TrackedSatellites >= 3) && (GPS_AVAILABLE == g_struGpsFlag.GpsAvailable))
#elif defined __CPU_LTE_MACRO__
	if((gUbloxGpsAllData.TrackedSatellites >= 3) && (GPS_AVAILABLE == g_struGpsFlag.GpsAvailable))
#endif
#endif
	{
		return GPS_OK;
	}
#if !defined(__CPU_LTE_CENTERSTATION__)&&!defined(__CPU_LTE_CARDTYPE__)
	else if (g_Clock_Set_AllDate.gps_Type == 0xFF)
	{
		if (!(ext_1pps_in_error&0x10))
		{
			return GPS_OK;
		}
		else
		{
			return GPS_ERROR;
		}
	}
#endif

#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	else if(g_Clock_Set_AllDate.gps_Type == SYNC_SOURCE_1PPS_TOD)
	{
		//if((bsp_cpld_read_reg(13) & 0x02) \
        //    && bsp_get_tod_secstatus() != 2 \
        //    && (bsp_get_tod_srcstatus() == 3 || bsp_get_tod_srcstatus() == 5) \
        //    )
		//	return GPS_OK;
		//else
			return GPS_ERROR;
	}
    else if( g_Clock_Set_AllDate.gps_Type == SYNC_SOURCE_1588)
    {
        //if(checkoffset())
        //    return GPS_OK;
        //else
            return GPS_ERROR;
    }
	else if( g_Clock_Set_AllDate.gps_Type == SYNC_SOURCE_ES)
    {
        if((bsp_cpld_read_reg(124) & 0x11) == 0x11)
            return GPS_OK;
        else
            return GPS_ERROR;
    }
#endif
	else
	{
		return GPS_ERROR;
	}
}

/*******************************************************************************
* 函数名称: gps_unlock_test
* 功    能: 
* 相关文档:
* 函数类型: SYS_STATUS
* 参    数:
* 参数名称          类型        输入/输出       描述

*******************************************************************************/
void  gps_unlock_test(void)
{
	g_struGpsFlag.GpsAvailable  = 0;
	set_gps_data();
}

/*******************************************************************************
* 函数名称: gps_lock_test
* 功    能: 
* 相关文档:
* 函数类型: SYS_STATUS
* 参    数:
* 参数名称          类型        输入/输出       描述

*******************************************************************************/
void gps_lock_test(void)
{
	g_struGpsFlag.GpsAvailable  = 1;
	set_gps_data();
}

#define TIMEOUT_USEC (0)
#define TIMEOUT_SEC(buflen,baud) (buflen*20/baud+2)

/*******************************************************************************
* 函数名称: TimedRead						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
int TimedRead (int fd, char *buffer, const int maxlength, int timeout)
{
	int count, tcount;
	
	fd_set readFds;
	#ifdef NORMAL_MODE
	struct timeval timer = { timeout, 0} ;
    #else
	struct timeval timer ;//= { timeout, 0} ;
	#endif
	FD_ZERO (&readFds);
	FD_SET (fd, &readFds);
    
    #ifndef NORMAL_MODE
	timer.tv_sec = TIMEOUT_SEC (maxlength, 9600);
	timer.tv_usec = 0;
	#endif
	//bsp_delay_1ms(500);
	for (count = 0; count < maxlength; )
	{
		tcount = select (fd+1, &readFds, NULL, NULL, &timer);
		if (tcount <= 0 || !(FD_ISSET (fd, &readFds)))
		{
			if (tcount == 0) 
			{
				//bsp_dbg ( "TimedRead timed out, count %d \n", count);
			} 
			else 
			{
				bsp_dbg ( "TimedRead had error 0x%x\n", errno);
			}
			break;
		}
		//bsp_sys_msdelay(500);
		
		count += read (fd, &buffer[count], maxlength);
        /*BD*/
		if((count==28)&&(buffer[2]==0x01)&&(buffer[3]==0x24)&&(buffer[0]==0xb5)&&(buffer[1]==0x62))
        {
            break;
        }
        /*UTC*/
        if((count==28)&&(buffer[2]==0x01)&&(buffer[3]==0x21)&&(buffer[0]==0xb5)&&(buffer[1]==0x62))
        {
            break;
        }
        /*POSLLH*/
        if((count==36)&&(buffer[2]==0x01)&&(buffer[3]==0x2)&&(buffer[0]==0xb5)&&(buffer[1]==0x62))
        {
            break;
        }
        /*GPSTime*/
        if((count==24)&&(buffer[2]==0x01)&&(buffer[3]==0x20)&&(buffer[0]==0xb5)&&(buffer[1]==0x62))
        {
            break;
        }
        /*3D fix*/
        if((count==60)&&(buffer[2]==0x01)&&(buffer[3]==0x06)&&(buffer[0]==0xb5)&&(buffer[1]==0x62))
        {
            break;
        }
        /*MON*/
        if((count==68)&&(buffer[2]==0x0A)&&(buffer[3]==0x09)&&(buffer[0]==0xb5)&&(buffer[1]==0x62))
        {
            break;
        }
		/*SVINFO*/
		if((buffer[2]==0x01)&&(buffer[3]==0x30)&&(buffer[0]==0xb5)&&(buffer[1]==0x62))
		{
			if(count > 16)
			{
				if((buffer[10]*12+16) == count)
				{	
					break;
				}	
			}
	    }
	}
	
	return count;
}

/*******************************************************************************
* 函数名称: gpsExecCommand_UBlox						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
static struct timeval tv_timeout;
//#define TIMEOUT_USEC (0)
//#define TIMEOUT_SEC(buflen,baud) (buflen*20/baud+2)

unsigned int g_ReturnStatus[10] = {0};
int gpsExecCommand_UBlox(char *msg, const u8 u8length, char *pOutPutResponse, u8 resplen)
{  
	char response[BSP_DATARSP_BUF_LEN];       
	int i, m, len, count;    
	u8 done;
	int retry  = 5;
	s32 status = 0;
  #ifndef NORMAL_MODE
	static fd_set  fs_write;
	int retval;
	//pthread_mutex_lock(&g_mp_gps);
	#endif
  
	pthread_mutex_lock(&g_mp_gps);
	done = 0;
	do 
	{
        /*填写checksum*/
        cal_cs(msg + 2, u8length - 4);
		
		status = 0;	
		#ifndef NORMAL_MODE
        FD_ZERO (&fs_write);
        FD_SET (g_IntGpsUartFd, &fs_write);
	    tv_timeout.tv_sec = TIMEOUT_SEC (u8length, 9600);
        tv_timeout.tv_usec = TIMEOUT_USEC;
		retval = select (g_IntGpsUartFd + 1, NULL, &fs_write, NULL, &tv_timeout);
        if (retval) 
        {
		#endif
		count = write(g_IntGpsUartFd, msg, u8length);
        #ifndef NORMAL_MODE
		}
		#endif
		if(count!=u8length)
		{
			bsp_dbg("writing u8length error!..............count=%d, u8length=%d.\n", count, u8length);
			g_ReturnStatus[0]++;
			status = -1;
		
			break;
		}        
		
		if(resplen==0)//不需要等返回值
			done = 1;

		// Wait for the response
		while (0 == done)
		{
			bsp_delay_1ms(501);
			count = TimedRead (g_IntGpsUartFd, &response[0], 300, 2);
			if (count == 0) 
			{
				bsp_dbg("%s %d\n",__FUNCTION__,__LINE__);
				g_ReturnStatus[1]++;
				status = -1;
		
				break;
			}
			
			for(i=0; i<count; i++)//找到应答命令头
			{
				if(response[i]==0xb5)
				break;
			}
			
			if((i==count)||((count-i)<= 8))//没有找到或长度不对，丢弃
			{
				status = -1;
				g_ReturnStatus[2]++;
				break;
			} 

			for(m=i; m<4+i; m++)
			{
				if(response[m]!=msg[m-i])
				break;
			}
			
			if (m==(i+4))
			{
				len = response[5+i] * 0x100 + response[4+i];
				if((count-i)<len)
				{
					status = -1;
					g_ReturnStatus[3]++;
					break;
				}
				
				done   = 1;
				status = 0;	
							
				if (NULL != pOutPutResponse)
				{    
				
					if (len < (BSP_DATARSP_BUF_LEN - 8))
					{
						memcpy((unsigned char *)pOutPutResponse, (unsigned char *)&response[i], len+8);
					}
				}                
			}
			else
			{
				status = -1;    
				g_ReturnStatus[4]++;
				break;
			} 
			
		} // !done getting response
		
	} while ((0 == done) && (status == -1) && retry--);
	
	pthread_mutex_unlock(&g_mp_gps);
	
	return (status);
	
}
 /*******************************************************************************
* 函数名称: gps_disable_tp						
* 功    能: 关闭GPS TIMEPULSE输出                              
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_disable_tp(void) 
{    
	char data0[]={0xB5,0x62,0x06,0x31,0x20,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x57,0xCB};
	char data1[]={0xB5,0x62,0x06,0x31,0x20,0x00,0x01,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
				 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x58,0xEB};
				 
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};
	int status;
	
	status = gpsExecCommand_UBlox(data0, 40, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_disable_tp0, return failure!, try again");         
	}
	
	status = gpsExecCommand_UBlox(data1, 40, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_disable_tp1, return failure!, try again");         
	}
	return status;
}
 /*******************************************************************************
* 函数名称: gps_Cfg_reset						
* 功    能: Reset ColdStart                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Cfg_reset(void) 
{    
	char data[]={0xB5,0x62,0x06,0x04,0x04,0x00,0xFF,0x07,0x01,0x00,0x15,0x77};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;
	
	status = gpsExecCommand_UBlox(data, 12, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_Cfg_reset, return failure!, try again");         
	}
	
	return status;
}
/*******************************************************************************
* 函数名称: gps_Cfg_ANT						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Cfg_ANT(void) 
{    
	char data[]={ 0xB5, 0x62, 0x06, 0x13, 0x04, 0x00, 0x1F, 0x00, 0xF0, 0xB9, 0xE5, 0xE2};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};  
	int status;
	
	status = gpsExecCommand_UBlox(data, 12, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_Cfg_ANT, return failure!, try again");         
	}
	
	return status;
}

s32 gps_Cfg_ANT_6T(void) 
{    
	//char data[]={ 0xB5, 0x62, 0x06, 0x13, 0x04, 0x00, 0x1F, 0x00, 0x8B, 0xA9, 0x70, 0x08};/*10,12,11*/
    char data[]={ 0xB5, 0x62, 0x06, 0x13, 0x04, 0x00, 0x1F, 0x00, 0x2d, 0xa2, 0x0b, 0x45};/*8,17,13*/
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status = OK;
	
	status = gpsExecCommand_UBlox(data, sizeof(data), dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_Cfg_ANT, return failure!, try again");         
	}

    gps_Cfg_ANT_Poll();
    
	return status;
}
/*******************************************************************************
* 函数名称: gps_Cfg_Tp1						
* 功    能: 配置10ms                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
char TP1_GPS_10ms[] = {0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 
        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
        0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x07, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x00, 0x00, 0x00, 
        0x9D, 0xF2};
char TP1_GPS_1PPS[] = {0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 
        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0F, 0x00, 
        0x40, 0x42, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x17,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x00, 0x00, 0x00, 
        0x01, 0x28};
/*u8Flag: 0-1PPS; 1-10ms*/
#if 1
s32 gps_Cfg_Tp1(u8 u8Flag) 
{   
    char *pData = NULL;
    
    if (0 == u8Flag)
    {
        pData = TP1_GPS_1PPS;
    }
    else
    {
        pData = TP1_GPS_10ms;
    }
	printf("gps_Cfg_Tp1\n");
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(pData, sizeof(TP1_GPS_1PPS), dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("\n[tGPS] gps_Cfg_Tp1, return failure!\n"); 
		return GPS_ERROR;
	}

	return GPS_OK;
    
}
#endif

/*******************************************************************************
* 函数名称: gps_Cfg_Tp2						
* 功    能:    cfg time pulse 2,配置1s                                 
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Cfg_Tp2(void) 
{   
	char data[] ={0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x01, 0x00, 
        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0F, 0x00, 
        0x40, 0x42, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x17,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x00, 0x00, 0x00, 
        0x02, 0x48};

	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;
	printf("gps_Cfg_Tp2\n");
	status = gpsExecCommand_UBlox(data, 40, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_Cfg_Tp2, return failure!\n"); 
		return GPS_ERROR;
	}

	return GPS_OK;

}
/*******************************************************************************
* 函数名称: gps_test_close						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
void gps_test_close(void)
{
       close(g_IntGpsUartFd);

}

/*******************************************************************************
* 函数名称: gps_Close_GGA						
* 功    能:      关闭NMEA的GGA命?                              
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Close_GGA(void)
{
	char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x23};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("\n[tGPS] gps_Close_GGA, return failure!\n");         
	}

	return status; 
}
/*******************************************************************************
* 函数名称: gps_Close_GLL						
* 功    能:        关闭NMEA的GLL命令                             
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Close_GLL(void)
{
	char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg( "\n[tGPS] gps_Close_GLL, return failure!\n");         
	}
	
	return status; 
}

/*******************************************************************************
* 函数名称: gps_Close_GSA						
* 功    能:         关闭NMEA的GSA命令s                            
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Close_GSA(void)
{
	char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31 };
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg( "\n[tGPS] gps_Close_GSAL, return failure!\n");         
	}
	
	return status; 
}
/*******************************************************************************
* 函数名称: gps_Close_GSV						
* 功    能:          关闭NMEA的GSV命令                           
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
**********************************s*********************************************/
s32 gps_Close_GSV(void)
{
	char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg( "\n[tGPS] gps_Close_GSV, return failure!\n");         
	}
	
	return status; 
}
/*******************************************************************************
* 函数名称: gps_Close_RMC						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Close_RMC(void)
{
	char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg( "\n[tGPS] gps_Close_RMC, return failure!\n");         
	}
	
	return status; 
}
/*******************************************************************************
* 函数名称: gps_Close_VTG						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Close_VTG(void)
{
    char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46};
    char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
    int status;

    status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
    if(status!=OK)//failure, try again
    {
        bsp_dbg( "\n[tGPS] gps_Close_UTG, return failure!\n");         
    }
    return status; 
}
/*******************************************************************************
* 函数名称: gps_Close_ZDA						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Close_ZDA(void)
{
	char data[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5B};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("\n[tGPS] gps_Close_ZDA, return failure!\n");         
	}
	
	return status; 
}
/*******************************************************************************
* 函数名称: gps_Close_TXT						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Close_TXT(void)
{
	u8 u8i,u8ck1,u8ck2;
	char data[] = {0xB5, 0x62, 0x06, 0x02, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,0x00,0x00,0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,0x00};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	for (u8i=2;u8i<26;u8i++)
	{
		u8ck1 = u8ck1 + data[u8i];
		u8ck2 = u8ck2 + u8ck1;
	}
	
	data[26] = u8ck1;
	data[27] = u8ck2;
	
	status = gpsExecCommand_UBlox(data, 28, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("\n[tGPS] gps_Close_TXT, return failure!\n");         
	}
	
	return status; 
}
/*******************************************************************************
* 函数名称: gps_Close_SABS						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Close_SABS(void)
{
	char data[] = {0xB5, 0x62, 0x06, 0x16, 0x08, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x97};
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(data, 16, dataRsp, 0);
	if(status!=OK)//failure, try again
	{
		bsp_dbg("\n[tGPS] gps_Close_SABS, return failure!\n");         
	}
	
	return status; 
}


void gps_close_all()
{
	int i;
	for(i=0;i<6;i++)
	{
		gps_Close_GGA();
		GPS_DELAY(10);
		gps_Close_GLL();
		GPS_DELAY(10);
		gps_Close_GSA();
		GPS_DELAY(10);
		gps_Close_GSV();
		GPS_DELAY(10);
		gps_Close_RMC();
		GPS_DELAY(10);
		gps_Close_VTG();
		GPS_DELAY(10);
		gps_Close_ZDA();
		GPS_DELAY(10);
		gps_Close_TXT();
		GPS_DELAY(10);
		gps_Close_SABS();
		GPS_DELAY(10);
	}

}
unsigned char gps_Get_Lockinfo_fix(void)
{
	char data[] = {0xB5, 0x62, 0x01, 0x06, 0, 0, 0x07, 0x16};
	char dataRsp[BSP_DATARSP_BUF_LEN]={0};    
    int status;
    u8    GpsAvailable;

	status = gpsExecCommand_UBlox(data, 8, dataRsp, 1);
	if(status==OK)
	{
		char *pInfo = &dataRsp[6];
		GpsAvailable = pInfo[10];
        if(GpsAvailable == 0x3 || GpsAvailable == 0x5)
        {
		    g_struGpsFlag.GpsAvailable =  GPS_AVAILABLE;
		}
		else
		{
		    g_struGpsFlag.GpsAvailable =  GPS_NOT_AVAILABLE;
		}

	}	
	return GpsAvailable; 
}

typedef struct{
	time_t time;
	double sec;
}gtime_t;
typedef struct
{
	u16 GpsWeeks;
	sl32 GpsMs;
	sl32 GpsSecned;
	u8  GpsLeaps;
}T_GpsTimeDate;
T_GpsTimeDate gUbloxGpsTime;
double gpst0[] = {1980, 1, 6, 0, 0, 0};
gtime_t epoch2time(const double * ep)
{
	const int doy[]={1,32,60,91,121,152,182,213,244,274,305,335};
	gtime_t time={0};
	int days, sec, year=(int)ep[0],mon=(int)ep[1],day=(int)ep[2];
	
	if (year < 1970 || 2099 < year || mon < 1 || 12 < mon)
	return time;
	/*leap year if year % 4 == 0 in 1901-2099*/
	days = (year - 1970) * 365 + (year - 1969)/4 + doy[mon-1] + day-2+(year%4==0&&mon>=3?1:0);
	sec = (int) floor(ep[5]);
	time.time = (time_t)days*86400+(int)ep[3]*3600+(int)ep[4]*60+sec;
	time.sec = ep[5]-sec;
	return time;
}

gtime_t gpst2time(int week, double sec)
{
	gtime_t t=epoch2time(gpst0);
	
	if (sec < -1E9||1E9 < sec)
	sec = 0.0;
	t.time+= 86400*7*week+(int)sec;
	t.sec=sec-(int)sec;
	
	return t;
}

s32 bd_NAV_TimeBD(s32 gps_gmt_offset,u32 leap_second)
{
	char data[] = {0xB5, 0x62, 0x01, 0x24, 0x00, 0x00, 0x21, 0x64};
	char dataRsp[BSP_DATARSP_BUF_LEN]={0};
    struct tm result;
	struct tm mytime;
	struct tm *pmytime;
	time_t  t_tick;
	s32 gpsOffset;
	s32 status;
	gtime_t GPSTime;
	gtime_t UTCTime;
	struct tm * gpstimeinfo;
	if((gps_gmt_offset > 720)||(gps_gmt_offset<-720))
	{
            printf("Input par is error! gps_gmt_offset = %d",gps_gmt_offset);
	     return -1;
	}
	gpsOffset = gps_gmt_offset;
	memset(dataRsp,0,BSP_DATARSP_BUF_LEN);
	status = gpsExecCommand_UBlox(data, 8, dataRsp, 20+6);

	if(status==OK)
	{        
		//解析时间，加时区
		char *pchar = &dataRsp[6];//数据区
		//printf("pchar[11] = %d\n", pchar[11]);
		if((pchar[15]&0x03) == 0x03)//GPSTime valid
		{
			pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));		
			gUbloxGpsTime.GpsMs    = pchar[3]*0x1000000 + pchar[2]*0x10000 + pchar[1]*0x100 + pchar[0];
			gUbloxGpsTime.GpsWeeks = pchar[13]*0x100 + pchar[12] + 1356; //小端模式     1356是和UTC差的星期数
			gUbloxGpsTime.GpsLeaps  = pchar[14];
			gUbloxGpsTime.GpsSecned = gUbloxGpsTime.GpsMs / 1000 - leap_second; 
			GPSTime = gpst2time(gUbloxGpsTime.GpsWeeks,gUbloxGpsTime.GpsSecned);
			//printf("GpsWeeks:%d\n",gUbloxGpsTime.GpsWeeks);
			//printf("GpsLeaps:%d\n",gUbloxGpsTime.GpsLeaps);
			//printf("GpsSecned:%d\n",gUbloxGpsTime.GpsSecned);
			gpstimeinfo = localtime(&GPSTime.time);
			//printf("GPSTime %s\n", asctime(gpstimeinfo));
			#if 1
			gUbloxGpsAllData.Year = gpstimeinfo->tm_year+1900;
			gUbloxGpsAllData.Month = gpstimeinfo->tm_mon+1;
			gUbloxGpsAllData.Day = gpstimeinfo->tm_mday;
			gUbloxGpsAllData.Hour = gpstimeinfo->tm_hour;
			gUbloxGpsAllData.Minute = gpstimeinfo->tm_min;
			gUbloxGpsAllData.Second = gpstimeinfo->tm_sec;
			#endif
			pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));
		}
		else
		{
			//bsp_dbg("[tGPS] gps_NAV_TimeGPS, msg is invalid\n"); 
			return -1;
		}

	}
	else
	{
		return -1;
	}
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	set_gps_data();
#endif	
	
	return status;   
}

s32 gps_NAV_TimeGPS(s32 gps_gmt_offset,u32 leap_second)
{
	char data[] = {0xB5, 0x62, 0x01, 0x20, 0x00, 0x00, 0x21, 0x64};
	char dataRsp[BSP_DATARSP_BUF_LEN]={0};
    struct tm result;
	struct tm mytime;
	struct tm *pmytime;
	time_t  t_tick;
	s32 gpsOffset;
	s32 status;
	gtime_t GPSTime;
	gtime_t UTCTime;
	struct tm * gpstimeinfo;
	if((gps_gmt_offset > 720)||(gps_gmt_offset<-720))
	{
            printf("Input par is error! gps_gmt_offset = %d",gps_gmt_offset);
	     return -1;
	}
	gpsOffset = gps_gmt_offset;
	memset(dataRsp,0,BSP_DATARSP_BUF_LEN);
	status = gpsExecCommand_UBlox(data, 8, dataRsp, 20+6);

	if(status==OK)
	{        
		//解析时间，加时区
		char *pchar = &dataRsp[6];//数据区
		//printf("pchar[11] = %d\n", pchar[11]);
		if((pchar[11]&0x03) == 0x03)//GPSTime valid
		{
			pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));		
			gUbloxGpsTime.GpsMs    = pchar[3]*0x1000000 + pchar[2]*0x10000 + pchar[1]*0x100 + pchar[0];
			gUbloxGpsTime.GpsWeeks = pchar[9]*0x100 + pchar[8]; //小端模式
			gUbloxGpsTime.GpsLeaps  = pchar[11];
			gUbloxGpsTime.GpsSecned = gUbloxGpsTime.GpsMs / 1000 - leap_second; 
			GPSTime = gpst2time(gUbloxGpsTime.GpsWeeks,gUbloxGpsTime.GpsSecned);
			//printf("GpsWeeks:%d\n",gUbloxGpsTime.GpsWeeks);
			//printf("GpsLeaps:%d\n",gUbloxGpsTime.GpsLeaps);
			//printf("GpsSecned:%d\n",gUbloxGpsTime.GpsSecned);
			gpstimeinfo = localtime(&GPSTime.time);
			//printf("GPSTime %s\n", asctime(gpstimeinfo));
			#if 1
			gUbloxGpsAllData.Year = gpstimeinfo->tm_year+1900;
			gUbloxGpsAllData.Month = gpstimeinfo->tm_mon+1;
			gUbloxGpsAllData.Day = gpstimeinfo->tm_mday;
			gUbloxGpsAllData.Hour = gpstimeinfo->tm_hour;
			gUbloxGpsAllData.Minute = gpstimeinfo->tm_min;
			gUbloxGpsAllData.Second = gpstimeinfo->tm_sec;
			#endif
			pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));
		}
		else
		{
			//bsp_dbg("[tGPS] gps_NAV_TimeGPS, msg is invalid\n"); 
			return -1;
		}

	}
	else
	{
		return -1;
	}
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	set_gps_data();
#endif	
	
	return status;   
}
extern int g_gps_print_level;
/*******************************************************************************
* 函数名称: gps_NAV_TimeUTC						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_NAV_TimeUTC(s32 gps_gmt_offset)
{
	char data[] = {0xB5, 0x62, 0x01, 0x21, 0x0, 0x0, 0x22, 0x67};
	char dataRsp[BSP_DATARSP_BUF_LEN]={0};
    struct tm result;
	struct tm mytime;
	struct tm *pmytime;
	time_t  t_tick;
	s32 gpsOffset;
	s32 status;
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	if (g_Clock_Set_AllDate.gps_Type != 0 && g_Clock_Set_AllDate.gps_Type != 2)
#else
	if (g_Clock_Set_AllDate.gps_Type == 1 || g_Clock_Set_AllDate.gps_Type == 0xFF)
#endif
	{
		return BSP_OK;
	}
	
	if((gps_gmt_offset > 720)||(gps_gmt_offset<-720))
	{
            printf("Input par is error! gps_gmt_offset = %d",gps_gmt_offset);
	     return -1;
	}
	gpsOffset = gps_gmt_offset;
	memset(dataRsp,0,BSP_DATARSP_BUF_LEN);
	
	status = gpsExecCommand_UBlox(data, 8, dataRsp, 20+6);

	if(status==OK)
	{        
		//解析时间，加时区
		char *pchar = &dataRsp[6];//数据区
		
		if(pchar[19]&0x07 == 0x07)//utc valid
		{
			pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
			gUbloxGpsAllData.Year = pchar[13]*0x100+pchar[12];//小端模式
			gUbloxGpsAllData.Month = pchar[14];
			gUbloxGpsAllData.Day = pchar[15];
			gUbloxGpsAllData.Hour = pchar[16];
			gUbloxGpsAllData.Minute = pchar[17];
			gUbloxGpsAllData.Second = pchar[18];
#if 1
            if(g_gps_print_level == 1){
    			printf("GpsAllData.Year=%d\n",gUbloxGpsAllData.Year);
    			printf("GpsAllData.Month=%d\n",gUbloxGpsAllData.Month);
    			printf("GpsAllData.Day=%d\n",gUbloxGpsAllData.Day);
    			/*东八区时间，和标准的北京时间差8小时*/
    			printf("GpsAllData.Hour=%d\n",gUbloxGpsAllData.Hour+8);
    			printf("GpsAllData.Minute=%d\n",gUbloxGpsAllData.Minute);
    			printf("GpsAllData.Second=%d\n",gUbloxGpsAllData.Second);
            }
#endif


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

			pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));
		}
		else
		{
			//bsp_dbg("[tGPS] gps_NAV_TimeUTC, msg is invalid\n"); 
			return -1;
		}

	}
	else
	{
		return -1;
	}
	set_gps_data();
	return status;   
}
unsigned int g_GpsSvinfoDebug = 0;

/*******************************************************************************
* 函数名称: gps_NAV_SVInfo						
* 功    能:       查询卫星状况                              
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_NAV_SVInfo(void)
{
	char data[10] = {0xB5, 0x62, 0x01, 0x30, 0x00, 0x00, 0x31, 0x94};
	char dataRsp[BSP_DATARSP_BUF_LEN]={0};    
	int status;
	char numCh;
	int visiableS, trackS,i;

	visiableS = 0;
	trackS = 0;
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	if (g_Clock_Set_AllDate.gps_Type != 0 && g_Clock_Set_AllDate.gps_Type != 2)
#else
	if (g_Clock_Set_AllDate.gps_Type == 1 || g_Clock_Set_AllDate.gps_Type == 0xFF)
#endif
	{
		
		memset(&gUbloxGpsAllData, 0, sizeof(gUbloxGpsAllData));
		return BSP_OK;
	}
	memset(g_ReturnStatus, 0 , sizeof(g_ReturnStatus));
	status = gpsExecCommand_UBlox(data, 8, dataRsp, 200);
	pthread_mutex_lock(&(gUbloxSData.m_mutex));
	
	if(status==0)
	{        
		UINT8*pInfo = (UINT8*)&dataRsp[6];//小端模式
		int len = 0;

		memset((void *)&(gUbloxSData.svInfo), 0x0, sizeof(gUbloxSData.svInfo));
		gUbloxSData.svInfo.iTow = pInfo[len+3]*0x1000000 + pInfo[len+2]*0x10000 + pInfo[len+1]*0x100 +pInfo[len];
		len += 4;
		
		gUbloxSData.svInfo.numCh = pInfo[len];
		len += 1;
		
		gUbloxSData.svInfo.globalFlags = pInfo[len];
		len += 1;
		
		gUbloxSData.svInfo.res2 = pInfo[len+1]*0x100 +pInfo[len];
		len += 2;
		
		if (gUbloxSData.svInfo.numCh < (GPSNUMCHINFO - 1))
		{
			for(i=0; i<gUbloxSData.svInfo.numCh; i++)
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
		    	
		    	gUbloxSData.svInfo.sInfo[i].prRes = pInfo[len+3]*0x1000000 + pInfo[len+2]*0x10000 + pInfo[len+1]*0x100 +pInfo[len];
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
		}
		//pthread_mutex_unlock(&(gUbloxSData.m_mutex));
		pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
		gUbloxGpsAllData.VisibleSatellites = visiableS;
		gUbloxGpsAllData.TrackedSatellites = trackS;
		pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));
	}
	else
	{
		pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
		gUbloxGpsAllData.VisibleSatellites = visiableS;
		gUbloxGpsAllData.TrackedSatellites = trackS;
		pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));
	}
	pthread_mutex_unlock(&(gUbloxSData.m_mutex));
	set_gps_data();

/*******************************GPS SVINFO DEBUG CODE START************************************************/	
	if(g_GpsSvinfoDebug == 1)
	{
	   FILE *fp;
	   int k;
	   time_t timer = time(NULL);
	   struct tm *tm;
	   tm = localtime(&timer);
	   unsigned char fix_value = 0;
	   if(gUbloxGpsAllData.TrackedSatellites < 3) 
	   {	
	   	if((fp = fopen("/mnt/btsa/bsp_gps_svinfo_log.txt","a+")) == NULL)
	   	{
	   		printf("can not open bsp_gps_svinfo_writelog.txt!\n");
	   		return -1;
	   	}
	   	fix_value = gps_Get_Lockinfo_fix();
	   	fprintf(fp,"%d/%d/%d/%d/%d/%d\n",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
	   	fprintf(fp,"status = %d\n", status);
	   	fprintf(fp,"g_Re[0]=%d g_Re[1]=%d g_Re[2]=%d g_Re[3]=%d g_Re[4]=%d\n", 
	   	g_ReturnStatus[0], g_ReturnStatus[1], g_ReturnStatus[2], g_ReturnStatus[3], g_ReturnStatus[4]);
	   	fprintf(fp,"gUbloxSData.svInfo.numCh = %d\n", gUbloxSData.svInfo.numCh);
	   	fprintf(fp,"fix_value = 0x%x\n", fix_value);
	   	for (k = 0; k < gUbloxSData.svInfo.numCh; k++)
	   	{
	   	
	   		fprintf(fp,"%d/", gUbloxSData.svInfo.sInfo[k].quality);
	   		
	   	}
	   	fprintf(fp,"\n");
	   	fprintf(fp,"trackS = %d\n", trackS);
	   	fprintf(fp,"visiableS = %d\n", visiableS);
	   	fclose(fp);	
	   }
	}
/**************************************GPS SVINFO DEBUG CODE START*****************************************/
	return status; 
	
}

/*******************************************************************************
* 函数名称: gps_NAV_PosLLH						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_NAV_PosLLH(void)
{
    char data[] = {0xB5, 0x62, 0x01, 0x02, 0x00, 0x00, 0x03, 0x0A};
    char dataRsp[BSP_DATARSP_BUF_LEN]={0};     
	int status;
	sl32 lon, lat, hei, deg, min, sec;
	
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	if (g_Clock_Set_AllDate.gps_Type != 0 && g_Clock_Set_AllDate.gps_Type != 2)
#else
	if (g_Clock_Set_AllDate.gps_Type == 1 || g_Clock_Set_AllDate.gps_Type == 0xFF)
#endif
	{
		return BSP_OK;
	}

    status = gpsExecCommand_UBlox(data, 8, dataRsp, 28+6);
    if(status==OK)
    {        
        char *pchar = &dataRsp[6];//数据区
        //经纬度返回值是1e-7单位，转换为度，分，再转为上报值
        lon = pchar[7]*0x1000000 + pchar[6]*0x10000 + pchar[5]*0x100 + pchar[4];
        deg = lon / 10000;//000;
        min = (lon % 10000000) * 60 / 10000;//000;
        sec = ((lon % 10000000) * 3600 / 10000)%60;    //000)%60;        
	    pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
        gUbloxGpsAllData.Longitude =  lon;//lon*1.0/25*9;//lon*3600*1000/10000000 //(deg * 3600 +min * 60 + sec);//*1000;
        
        lat = pchar[11]*0x1000000 + pchar[10]*0x10000 + pchar[9]*0x100 + pchar[8];  
        deg = lat / 10000;//000;
        min = (lat % 10000000) * 60 / 10000;//000;
        sec = ((lat % 10000000) * 3600 / 10000/*000*/)%60;        
        gUbloxGpsAllData.Latitude = lat;//lat*1.0/25*9;//(deg * 3600 +min* 60 + sec);//*1000;保留精度
       
        //高度单位为mm,转为cm
        hei = (pchar[19]*0x1000000 + pchar[18]*0x10000 + pchar[17]*0x100 + pchar[16]);        
         gUbloxGpsAllData.GPSHigh = hei/10;//mm->cm    
         pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));
    }
    else
    {
    	  pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
         gUbloxGpsAllData.Longitude =  0;
         gUbloxGpsAllData.Latitude = 0;
         gUbloxGpsAllData.GPSHigh =0;   
	  	 pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));

    }
	set_gps_data();
    return status; 
}

s32 gps_Get_Lockinfo(void)
{
    char data[] = {0xB5, 0x62, 0x01, 0x06, 0, 0, 0x07, 0x16};
	char dataRsp[BSP_DATARSP_BUF_LEN]={0};    
    int status;
    u8    GpsAvailable;
	
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	if (g_Clock_Set_AllDate.gps_Type != 0 && g_Clock_Set_AllDate.gps_Type != 2)
#else
	if (g_Clock_Set_AllDate.gps_Type == 1 || g_Clock_Set_AllDate.gps_Type == 0xFF)
#endif
	{
		return BSP_OK;
	}

	status = gpsExecCommand_UBlox(data, 8, dataRsp, 1);
	if(status==OK)
	{
		char *pInfo = &dataRsp[6];
		GpsAvailable = pInfo[10];
		//printf("GpsAvailable: %d\n", GpsAvailable);
        if(GpsAvailable == 0x3 /*|| GpsAvailable == 0x5*/)
        {
		    g_struGpsFlag.GpsAvailable =  GPS_AVAILABLE;
		}
		else
		{
		    g_struGpsFlag.GpsAvailable =  GPS_NOT_AVAILABLE;
		}

	}
	if (g_gps_print_level == 1){
		printf("g_struGpsFlag.GpsAvailable=%d\n", g_struGpsFlag.GpsAvailable);
	}
	return status; 
}

void gps_get_position(sl32 *psl32Latitude,sl32 *psl32Longitude)
{
    *psl32Latitude = gUbloxGpsAllData.Latitude;
    *psl32Longitude = gUbloxGpsAllData.Longitude;
}
#if 0
s32 bsp_get_tc_type()
{
#if 0
    if(GPS_NOT_AVAILABLE == g_struGpsFlag.GpsAvailable)
        return BSP_TC_TYPE_LO;
    else
        return BSP_TC_TYPE_GPS;
#else
    if(gUbloxGpsAllData.TrackedSatellites < 3)
        return BSP_TC_TYPE_LO;
    else
        return BSP_TC_TYPE_GPS;
#endif
}

#endif
/*******************************************************************************
* 函数名称: bsp_gps_gpsvar_mutex_init						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_MON_HW(void)
{
	char data[] = {0xB5, 0x62, 0x0A, 0x09, 0x00, 0x00, 0x13, 0x43 };
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;
	
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
	if (g_Clock_Set_AllDate.gps_Type != 0 && g_Clock_Set_AllDate.gps_Type != 2)
#else
	if (g_Clock_Set_AllDate.gps_Type == 1 || g_Clock_Set_AllDate.gps_Type == 0xFF)
#endif
	{
		return BSP_OK;
	}

	status = gpsExecCommand_UBlox(data, 8, dataRsp, 68+6);
	if(status==OK)
	{
		char *pInfo = &dataRsp[6];
		pthread_mutex_lock(&(gUbloxSData.m_mutex));
		gUbloxSData.antennaInfo.aStatus = pInfo[20];
		gUbloxSData.antennaInfo.aPower = pInfo[21];   
		pthread_mutex_unlock(&(gUbloxSData.m_mutex));     
	}	
	return status; 
}

/*******************************************************************************
* 函数名称: GPS_VAR_PRINT						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 GPS_VAR_PRINT(void)
{
	int i;
	
	if (g_Clock_Set_AllDate.gps_Type == 1 || g_Clock_Set_AllDate.gps_Type == 0xFF)
	{
		printf("GPS is NOT exit!\n");
		return BSP_OK;
	}
   if(g_Clock_Set_AllDate.gps_Type == 0)
        printf("GNSS is GPS.\n");
   if(g_Clock_Set_AllDate.gps_Type == 2)
        printf("GNSS is BEIDOU.\n");
    gps_MON_HW();
	pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
	printf("GPS time: %d/%d/%d[%d:%d:%d]\n", gUbloxGpsAllData.Year, gUbloxGpsAllData.Month, gUbloxGpsAllData.Day ,
	gUbloxGpsAllData.Hour ,gUbloxGpsAllData.Minute, gUbloxGpsAllData.Second);
	
	printf("Gps Position:\n\tLati (%d)\n\tLong(%d)\n\tHeight (%d)\n", gUbloxGpsAllData.Latitude,gUbloxGpsAllData.Longitude,gUbloxGpsAllData.GPSHigh);    
	printf("visable satellite:%d\n tracked_satellite:%d\n lock_info:%d\n ", gUbloxGpsAllData.VisibleSatellites, gUbloxGpsAllData.TrackedSatellites, g_struGpsFlag.GpsAvailable);	
	pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));

	pthread_mutex_lock(&(gUbloxSData.m_mutex));
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
	if (gUbloxSData.svInfo.numCh < (GPSNUMCHINFO - 1))
	{
		for(i=0; i<gUbloxSData.svInfo.numCh; i++)
		{
			printf("\n svid:%d, ", gUbloxSData.svInfo.sInfo[i].svid);
			printf("quality: %d, ", gUbloxSData.svInfo.sInfo[i].quality);
			printf("cno_db:%d ", gUbloxSData.svInfo.sInfo[i].cno_db);
		}
	}
	pthread_mutex_unlock(&(gUbloxSData.m_mutex));
	printf("\n");
	return 0;
}
/*******************************************************************************
* 函数名称: bsp_get_gps_status						
* 功    能:                                     
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 
* 说   明:
*******************************************************************************/
s32 bsp_get_gps_status(u8 *u8GpsStatus)
{
    s32 status = 0;
    
    if (g_Clock_Set_AllDate.gps_Type == 1 || g_Clock_Set_AllDate.gps_Type == 0xFF)
    {
        //printf("GPS is NOT exit!\n");
        return BSP_ERROR;
    }       
    *u8GpsStatus = gUbloxSData.antennaInfo.aStatus;
    return BSP_OK;
}
/*******************************************************************************
* 函数名称: gps_syn_time						
* 功    能: 长帧号，年时间转换为ms                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
u32 gps_syn_time(void)
{
	unsigned int secCount, secCount1,secCount2;
	struct tm time_s;
	struct timeval tTimeval;
	unsigned long long int u64temp = 0;
	
	if(2 == g_GNSS_flag){
		//gps_NAV_TimeUTC(gps_GMT_Offset);
		bd_NAV_TimeBD(gps_GMT_Offset,gLeapSecond);
	}else if (0 == g_GNSS_flag){
		gps_NAV_TimeGPS(gps_GMT_Offset,gLeapSecond);
	}
	pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
	time_s.tm_sec = gUbloxGpsAllData.Second;
	time_s.tm_min = gUbloxGpsAllData.Minute;
	time_s.tm_hour = gUbloxGpsAllData.Hour;
	time_s.tm_mday = gUbloxGpsAllData.Day;
	time_s.tm_mon  = gUbloxGpsAllData.Month - 1;       
	time_s.tm_year = gUbloxGpsAllData.Year-1900; 
    pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));
	time_s.tm_isdst = 0;                          /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */
	secCount1 = mktime(&time_s);
#if 0
	time_s.tm_sec = 0;
	time_s.tm_min = 0;
	time_s.tm_hour = 0;
	time_s.tm_mday = 1;
	time_s.tm_mon  = 1;       
	time_s.tm_year = gUbloxGpsAllData.Year-1900; 
	time_s.tm_isdst = 0;                          /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */
	secCount2 = mktime(&time_s);
#endif
	secCount = (secCount1) * 100;//转换为10毫秒
	//gettimeofday(&tTimeval, NULL);
	//printf("secCount = %u %u %llu  sys sec = %u\n", secCount, secCount1, (u64temp & 0x0fffffff), tTimeval.tv_sec);


	return secCount;
}

u32 sys_syn_time(struct tm *tm_s)
{

	unsigned int secCount, secCount1,secCount2;
	struct tm time_s;
	time_s.tm_sec = tm_s->tm_sec;
	time_s.tm_min = tm_s->tm_min;
	time_s.tm_hour = tm_s->tm_hour;
	time_s.tm_mday = tm_s->tm_mday;     
	time_s.tm_mon  = tm_s->tm_mon;
	time_s.tm_year = tm_s->tm_year;
	time_s.tm_isdst = 0;                          /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */
	secCount1 = mktime(&time_s);
#if 0
	time_s.tm_sec = 0;
	time_s.tm_min = 0;
	time_s.tm_hour = 0;
	time_s.tm_mday = 1;
	time_s.tm_mon  = 1;       
	time_s.tm_year = tm_s->tm_year; 
	time_s.tm_isdst = 0;                         /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */
	secCount2 = mktime(&time_s);
#endif
	secCount = (secCount1)*100;//转换为10毫秒
	return secCount;
}
u32 bsp_update_rtc_check(void)
{
	struct tm time_s;
	struct tm *tm;
	time_t timer = time(NULL);
	
	tm = localtime(&timer);
	
	
	if(2 == g_GNSS_flag){
		bd_NAV_TimeBD(gps_GMT_Offset,gLeapSecond);
	}else if (0 == g_GNSS_flag){
		gps_NAV_TimeGPS(gps_GMT_Offset,gLeapSecond);
	}
    pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));
	time_s.tm_sec = gUbloxGpsAllData.Second;
	time_s.tm_min = gUbloxGpsAllData.Minute;
	time_s.tm_hour = gUbloxGpsAllData.Hour;
	time_s.tm_mday = gUbloxGpsAllData.Day;
	time_s.tm_mon  = gUbloxGpsAllData.Month;       
	time_s.tm_year = gUbloxGpsAllData.Year-1900; 
    pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));
	//printf("system:%d/%d/%d/%d/%d/%d\n",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
	//printf("gps:%d/%d/%d/%d/%d/%d\n",time_s.tm_year+1900,time_s.tm_mon,time_s.tm_mday,time_s.tm_hour,time_s.tm_min,time_s.tm_sec);
	if (((tm->tm_year+1900) == (time_s.tm_year+1900)) && (time_s.tm_mon == tm->tm_mon+1) && (time_s.tm_mday == tm->tm_mday))
	{
		return BSP_OK;
	}else
	{
		return BSP_ERROR;
	}
	return BSP_OK;
}
/*******************************************************************************
* 函数名称: interpro_send_gps_info						
* 功    能: 上报gps信息                                   
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32  interpro_send_gps_info(void)
{
	char snd_buf[100];
	struct T_MsgHeader *strMsgHeader;
	struct T_GpsInfo *str_gps_info;
	int ret;
	int i;
	s32 s32ret;
	s8 alarm_gps[]= "GPS lost alarm";
 	#if 0   
	if(GPS_OK != gps_Get_Lockinfo())
	{
		bsp_dbg("get GPS Lockinfo error!!\n");
	}

	if(GPS_OK != gps_NAV_SVInfo())
	{
		bsp_dbg("get GPS visible&tracked satellites error!!\n");
	}	
	#endif
#if 0  
	if (g_struGpsFlag.GpsAvailable ==  GPS_AVAILABLE)
#else
	if (gUbloxGpsAllData.TrackedSatellites >= 3)
#endif
	{
        gps_update_rtc_valid = 1;
		/*GPS同步，面板灯闪烁标志清零，面板灯长亮*/
		g_gps_led_flicker = 0;
		//bsp_led_gps_on();
		if(g_gps_alarm_flag == 1)
		{
			//bsp_interpro_clear_alarm(BSP_ALM_TYPE_CPU_PF,BSP_ALM_GPS_LOST_ALARM,BSP_ALM_CLASS_MAJOR,alarm_gps,sizeof(alarm_gps),0);
			g_gps_alarm_flag = 0;
		}
	}
	else
	{
        gps_update_rtc_valid = 0;
		/*GPS失步，面板灯闪烁标志置位*/
		g_gps_led_flicker = 1;
		if(g_gps_alarm_flag == 0)
		{
			//bsp_interpro_send_alarm_msg(BSP_ALM_TYPE_CPU_PF,BSP_ALM_GPS_LOST_ALARM,BSP_ALM_CLASS_MAJOR,alarm_gps,sizeof(alarm_gps),0);
			g_gps_alarm_flag = 1;
		}
	}
#if 0
	gps_NAV_TimeUTC(gps_GMT_Offset);
	gps_NAV_PosLLH();

	memset(snd_buf,0,100);
	
	strMsgHeader = (struct T_MsgHeader *)snd_buf;	
	str_gps_info = (struct T_GpsInfo *)(snd_buf + 4);	

	strMsgHeader->MsgId = M_PLAT_L3_GPS_INFO_QUERY_RSP;
	pthread_mutex_lock(&(gUbloxGpsAllData.m_mutex));

	str_gps_info->Latitude = gUbloxGpsAllData.Latitude;
	str_gps_info->Longitude = gUbloxGpsAllData.Longitude;
	str_gps_info->Height = gUbloxGpsAllData.GPSHigh;

	str_gps_info->TrackedSatellites = gUbloxGpsAllData.TrackedSatellites;
	str_gps_info->VisibleSatellites = gUbloxGpsAllData.VisibleSatellites;

	str_gps_info->Month =gUbloxGpsAllData.Month;
	str_gps_info->Day = gUbloxGpsAllData.Day;
	str_gps_info->Hour = gUbloxGpsAllData.Hour;
	str_gps_info->Year = gUbloxGpsAllData.Year;
	str_gps_info->Minute = gUbloxGpsAllData.Minute;
	str_gps_info->Second = gUbloxGpsAllData.Second;
	pthread_mutex_unlock(&(gUbloxGpsAllData.m_mutex));

	if(gUbloxGpsAllData.TrackedSatellites > 3)
	{
		gps_update_rtc_valid = 1;		
	}
	else
	{
		gps_update_rtc_valid = 0;		
	}

  	if (gUbloxGpsAllData.VisibleSatellites > 3)
	{
		//ret=interpro_send_msg((char *)snd_buf, 100);
		//if (ret != 0)
		//{
			//printf("interpro_send_gps_info msg error!\n");	
			return BSP_ERROR;
		//}
	}		
#endif
}

u8 bsp_gps_TrackedSatellites(void)
{
	u8 u8TrackedSatellites = 0;
	if ((g_ublox_gps_type == 0) && (g_GNSS_flag == 2))
	{
		return 0;
	}
	if (GPS_AVAILABLE == g_struGpsFlag.GpsAvailable)
	{
		u8TrackedSatellites = gUbloxGpsAllData.TrackedSatellites;
	}else{
		u8TrackedSatellites = 0;
	}
	return u8TrackedSatellites;  
}

u8 bsp_gps_VisibleSatellites(void)
{	
	u8 u8VisibleSatellites = 0;
	if ((g_ublox_gps_type == 0) && (g_GNSS_flag == 2))
	{
		return 0;
	}
	if (GPS_AVAILABLE == g_struGpsFlag.GpsAvailable)
	{
		u8VisibleSatellites = gUbloxGpsAllData.TrackedSatellites;
	}else{
		u8VisibleSatellites = 0;
	}
    return u8VisibleSatellites;
}

u8 set_gps_data(void){
	int fd = 0;	
	struct Mesh_Lte_Com_Data data = {0};
	
	fd = open(gps_file, O_WRONLY);	
	if(fd<0){
	//	perror("gps set open");
		return BSP_ERROR;
	}	
	data.UbloxGpsAllData = gUbloxGpsAllData;
	data.struGpsFlag = g_struGpsFlag;	
	write(fd, &data, sizeof(data));
	close(fd);
	return BSP_OK;
}

u8 get_gps_data(void){
	int fd = 0;
	int size = 0;
	struct Mesh_Lte_Com_Data data = {0};
	//T_GpsAllData data;
	fd = open(gps_file, O_RDONLY);
	if(fd<0){
		//perror("gps get open");
		return BSP_ERROR;
	}
	size = read(fd, &data, sizeof(data));
	if(size==sizeof(data)){
		gUbloxGpsAllData = data.UbloxGpsAllData;
		g_struGpsFlag = data.struGpsFlag;
	}
	close(fd);
	return BSP_OK;
}

void *MonitorFileThread(void *arg){
	int fd = 0;	
	int wd = 0;
	FILE *fp = NULL;
	fd = inotify_init(); 
	if(fd<0){
		perror("gps file inotify");
		return ;
	}
	
	wd = inotify_add_watch(fd, gps_file, IN_MODIFY);
	if(wd<0){
		perror("gps file add_watch");
		return ;
	}
	
	fp = fdopen(fd, "r");
	
	while(1){
		struct inotify_event event;
		if(fread(&event, sizeof(event), 1, fp)<0){
			perror("gps file fread");
			return;
		}		
		get_gps_data();
		//bsp_update_gpspos();
	}
}
void StartFileMonitor(void)
{
	pthread_t ctid;	
	pthread_create(&ctid, NULL, MonitorFileThread, NULL);
	pthread_detach(ctid);
}

/*******************************************************************************
* 函数名称: gps_HW_VER						
* 功    能: 返回UBLOX硬件版本(6T/8T)                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值:	 0 :6T ,1:8T
* 说   明:
*******************************************************************************/
s32 gps_HW_VER(void)
{
	char data[] = {0xB5, 0x62, 0x0A, 0x04, 0x00, 0x00, 0x0E, 0x34 };
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status = 0;
	int gps_type = 0;
	
	printf("[%s]:before gpdExecCommand_UBlox func!\n",__func__);
	status = gpsExecCommand_UBlox(data, 8, dataRsp, 1);
	printf("[%s]:after gpdExecCommand_UBlox func!\n",__func__);
	
	if(status==OK)
	{
		char *pInfo = &dataRsp[6];

		//printf("hwver is %s \n",&pInfo[30]);

		if(pInfo[33]==0x38)
		{
           printf("UBLOX  8T\n");
		   gps_type=1;		  
			return gps_type;                  //zhichi beidou  
		}

		if(pInfo[33]==0x34)
		{
           printf("UBLOX  6T\n");
		   gps_type = 0;
		}
		return  gps_type; 
	}
	return status;	
}
s32 ublox_comp_init(void)
{
    s32 s32ret;

	s32ret = gps_config_gps_beidou();
	if(s32ret != GPS_OK)
	{
	     bsp_dbg("gps_config_compatible mode error!\n");
	     return GPS_ERROR;
	}

	s32ret = gps_Cfg_Tp1(0);  /* 使能PP1S中断*/
	if(s32ret != GPS_OK)
	{
		bsp_dbg("gps_Cfg_Tp1 error!\n");
		return GPS_ERROR;
	}
	return GPS_OK;
}
int gps_config_gps_beidou(void)
{
   int i = 0;
    char data[] = {0xB5, 0x62, 0x06, 0x3E, 0x34, 0x00, 0x0, 0x20, 0x20, 0x06, 
	                  0x0, 0x8, 0x10, 0x0, 0x1,0x0, 0x1, 0x1, 
	                  0x1, 0x1, 0x3, 0x0, 0x0, 0x0, 0x1, 0x1,
	                  0x3, 0x8, 0x10,0x0, 0x1, 0x0, 0x1, 0x1,
	                  0x4, 0x0, 0x8, 0x0, 0x0, 0x0, 0x1, 0x3,
	                  0x5, 0x0, 0x3, 0x0, 0x0, 0x0, 0x1, 0x5,
	                  0x6, 0x8, 0xE, 0x0, 0x0, 0x0, 0x1, 0x1,0x3a,0x87};

   char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
   int status;
	printf("gps_config_gps_beidou\n");
    for(i = 0;i < 3; i++)
    {
		status = gpsExecCommand_UBlox(data, 60, dataRsp, 0);
		if(status==OK)
		{
		    return status;
		}
		else
		{
		}
    }
    return -1;
}


s32 config_to_beidou(void)
{
	s32 s32ret;

	g_GNSS_flag = 2;

	printf("config_to_beidou\n");
	s32ret = gps_config_beidou();
	if(s32ret != GPS_OK)
	{
	     bsp_dbg("gps_config_beidou error!\n");
	     return GPS_ERROR;
	}
	#if 1
	s32ret = gps_Cfg_Tp1BD(0);  /* 使能BEIDOU PP1S中断*/
	if(s32ret != GPS_OK)
	{
		bsp_dbg("gps_Cfg_Tp1BD error!\n");
		return GPS_ERROR;
	}
	#endif
    s32ret = gps_Cfg_Tp2BD();  /* 使能PP1S中断*/
	if(s32ret != GPS_OK)
	{
		bsp_dbg("gps_Cfg_Tp2BD error!\n");
		return GPS_ERROR;
	}
#if 0   
   s32ret = gps_config_beidou();
    if(s32ret != GPS_OK)
    {
    	bsp_dbg("gps_config_beidou error!\n");
    	return GPS_ERROR;
    }

	#if 1
	s32ret = gps_Cfg_Tp1b();  /* 使能BEIDOU PP1S中断*/
	if(s32ret != GPS_OK)
	{
		bsp_dbg("gps_Cfg_Tp1 error!\n");
		return GPS_ERROR;
	}
	#endif
    s32ret = gps_Cfg_Tp2b();  /* 使能PP1S中断*/
	if(s32ret != GPS_OK)
	{
		bsp_dbg("gps_Cfg_Tp2 error!\n");
		return GPS_ERROR;
	}	
#endif 
}

int gps_config_beidou(void)
{
    int i = 0;
      char data[] = {0xB5, 0x62, 0x06, 0x3E, 0x34, 0x00, 0x0, 0x20, 0x20, 0x06, 
   	                  0x0, 0x8, 0x10, 0x0, 0x0, 0x0, 0x1, 0x1, 
   	                  0x1, 0x1, 0x3, 0x0, 0x0, 0x0, 0x1, 0x1,
   	                  0x3, 0x8, 0x10, 0x0, 0x1,0x0, 0x1, 0x1,
   	                  0x4, 0x0, 0x8, 0x0, 0x0, 0x0, 0x1, 0x3,
   	                  0x5, 0x0, 0x3, 0x0, 0x0, 0x0, 0x1, 0x5,
   	                  0x6, 0x8, 0xE, 0x0, 0x0, 0x0, 0x1, 0x1,0x39,0x5b};

   char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
   int status;

	printf("gps_config_beidou\n");	
    for(i = 0;i < 3; i++)
    {
	status = gpsExecCommand_UBlox(data, 60, dataRsp, 0);
	if(status==OK)
	{
	    return status;
	}
	else
	{
	
	}
    }
    return -1;
}
s32 config_to_gps(void)
{
	s32 s32ret;
	printf("config_to_gps\n");
	if (!(g_ublox_gps_type == 0 && g_GNSS_flag == 2))
	{
		g_GNSS_flag = 0;
	}

    s32ret = gps_config_gps();
    if(s32ret != GPS_OK)
    {
    	bsp_dbg("config_to_gps error!\n");
    	return GPS_ERROR;
    }
#if 1
	s32ret = gps_Cfg_Tp1(0);  /* 使能GPS PP1S中断*/
	if(s32ret != GPS_OK)
	{
		bsp_dbg("gps_Cfg_Tp1 error!\n");
		return GPS_ERROR;
	}
#endif
   	s32ret = gps_Cfg_Tp2();  /* 使能PP1S中断*/
	if(s32ret != GPS_OK)
	{
		bsp_dbg("gps_Cfg_Tp2 error!\n");
		return GPS_ERROR;
	}

}
int gps_config_gps(void)
{
    int i = 0;

      char data[] = {0xB5, 0x62, 0x06, 0x3E, 0x34, 0x00, 0x0, 0x20, 0x20, 0x06, 
   	                  0x0, 0x8, 0x10, 0x0, 0x1, 0x0, 0x1, 0x1, 
   	                  0x1, 0x1, 0x3, 0x0, 0x0, 0x0, 0x1, 0x1,
   	                  0x3, 0x8, 0x10, 0x0, 0x0,0x0, 0x1, 0x1,
   	                  0x4, 0x0, 0x8, 0x0, 0x0, 0x0, 0x1, 0x3,
   	                  0x5, 0x0, 0x3, 0x0, 0x0, 0x0, 0x1, 0x5,
   	                  0x6, 0x8, 0xE, 0x0, 0x0, 0x0, 0x1, 0x1,0x39,0x6b};

   char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
   int status;
	printf("gps_config_gps\n");
    for(i = 0;i < 3; i++)
    {
	status = gpsExecCommand_UBlox(data, 60, dataRsp, 0);
	if(status==OK)
	{
	    return status;
	}
	else
	{
	}
    }
    return -1;
}
/*******************************************************************************
* 函数名称: gps_Cfg_Tp1						
* 功    能: 配置10ms                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
char TP1_BD_10ms[] = {0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 
	        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
	        0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x07, 
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x01, 0x00, 0x00, 
	        0x9E, 0xF5};
char TP1_BD_1PPS[] = {0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 
	        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0F, 0x00,
	        0x40, 0x42, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x17, 
	        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x01, 0x00, 0x00, 
	        0x03, 0x4A}; 
/* u8Flag: 0-1PPS; 1-10ms*/
s32 gps_Cfg_Tp1BD(u8 u8Flag) 
{  
    char *pData = NULL;
    printf("gps_Cfg_Tp1BD\n");
    if (0 == u8Flag)
    {
        pData = TP1_BD_1PPS;
    }
    else
    {
        pData = TP1_BD_10ms;
    }

	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;

	status = gpsExecCommand_UBlox(pData, sizeof(TP1_BD_1PPS), dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("\n[tGPS] gps_Cfg_Tp1BD, return failure!\n"); 
		return GPS_ERROR;
	}

	return GPS_OK;
    
}
/*******************************************************************************
* 函数名称: gps_Cfg_Tp2						
* 功    能:    cfg time pulse 2,配置1s                                 
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Cfg_Tp2BD(void) 
{   

	char data[] ={0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x01, 0x01, 
        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0F, 0x00, 
        0x40, 0x42, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x17,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x01, 0x00, 0x00, 
        0x03, 0x4B};

	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;
	printf("gps_Cfg_Tp2BD\n");
	status = gpsExecCommand_UBlox(data, 40, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_Cfg_Tp2BD, return failure!\n"); 
		return GPS_ERROR;
	}

	return GPS_OK;

}
void gps_check(void)
{
   int i = 0;
   char data[] = {0xB5, 0x62, 0x0A, 0x09, 0x00, 0x00, 0x13, 0x43 };
   char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
   int status;

    for(i = 0;i < 3; i++)
    {
		status = gpsExecCommand_UBlox(data, 8, dataRsp, 68+6);
		if(status==OK)
		{
			g_UbloxOn = 1;
			return;
		}
		else
		{
			g_UbloxOn = 0;/*ublox is not exit*/
		}
    }
    return;
}

s32 bsp_get_current_clocksource(void)
{
	if(gUbloxGpsAllData.TrackedSatellites < 3)
	{
#ifdef __CPU_LTE_MACRO__
		if (g_Clock_Set_AllDate.gps_ClockSynWay == 1)	
		{
			if (g_Clock_Set_AllDate.gps_CascadeCfg == 0)
			{
				return BSP_TC_TYPE_LO;
			}
			else
			{
				u16 ext_1pps_in_error =0;
				ext_1pps_in_error = bsp_fpga_read_reg(402);
				if (!(ext_1pps_in_error&0x10))
				{
					return BSP_TC_TYPE_CASCADE;
				}
				else                                   
				{
					return BSP_TC_TYPE_LO;
				}
			}
	
		}
#endif
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
		if(g_Clock_Set_AllDate.gps_Type == BSP_TC_TYPE_1PPS_TOD)
		{
			//if((bsp_cpld_read_reg(13) & 0x02) \
	        //    && bsp_get_tod_secstatus() != 2 \
	        //    && (bsp_get_tod_srcstatus() == 3 || bsp_get_tod_srcstatus() == 5) \
	        //    )
				return BSP_TC_TYPE_1PPS_TOD;
		}
		else if(g_Clock_Set_AllDate.gps_Type == BSP_TC_TYPE_1588)
		{
			//if(checkoffset())
            	return BSP_TC_TYPE_1588;
		}
		else if(g_Clock_Set_AllDate.gps_Type == BSP_TC_TYPE_ES)
		{
			if((bsp_cpld_read_reg(124) & 0x11) == 0x11)
            	return BSP_TC_TYPE_ES;
		}
#endif
		else 	
		{
			return BSP_TC_TYPE_LO;
		}
	}
    else
	{
		if(g_struGpsFlag.GpsAvailable ==  GPS_AVAILABLE)
		{
	        if (g_GNSS_flag == 2)
			{
				if (g_ublox_gps_type == 1)
				{
					return BSP_TC_TYPE_BEIDOU;
				}
				else
				{
					return BSP_TC_TYPE_LO;
				}
			}
			else
			{
				return BSP_TC_TYPE_GPS;
			}
		}else{
			return BSP_TC_TYPE_LO;
		}
	}
	return BSP_TC_TYPE_LO;
}

s32 bsp_gps_get_type(void)
{
	if(afc_lock_status_get() == 3/*AFC_LOCK*//* || afc_lock_status_get() == 4*//*AFC_HOLDOVER*/)
		return bsp_get_current_clocksource();
	else
		return BSP_TC_TYPE_LO;
}

s32 bsp_get_clocksource(void)
{
	return bsp_gps_get_type();
}

/*******************************************************************************
* 函数名称: bsp_set_tc_type(s32 Type, u8 ClockSynWay, u8 CascadeCfg)						
* 功    能: 设置获取时钟源的类型                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称     类型			
* 说   明:   Type  	           /0 gps;          1 local clock;   2 beidou   3 cascade  /
*            ClockSynWay       /0 non cascade;  1 cascade            /
*            CascadeCfg        /0 master;       1 slave              /
* 作   者：贾雄
* 时   间：2015-12-28
*******************************************************************************/
s32 bsp_set_tc_type(u8 Type, u8 ClockSynWay, u8 CascadeCfg)
{	
	int fd = 0;
	s32 s32ret;
	if ((Type != 0) && (Type != 1) && (Type != 2) && (Type != 0xFF) && (Type != 3))
	{
		printf("Type invalid\n");
		return BSP_ERROR;
	}
	if ((ClockSynWay != 0) && (ClockSynWay != 1))
	{
		printf("ClockSynWay invalid\n");
		return BSP_ERROR;
	}
	if ((CascadeCfg != 0) && (CascadeCfg != 1) && (CascadeCfg != 0xFF))
	{
		printf("ClockSynWay invalid\n");
		return BSP_ERROR;
	}
	g_Clock_Set_AllDate.gps_Type = Type;
	g_Clock_Set_AllDate.gps_ClockSynWay = ClockSynWay;
	g_Clock_Set_AllDate.gps_CascadeCfg = CascadeCfg;
	g_GNSS_flag = g_Clock_Set_AllDate.gps_Type;	
	fd = open(gps_type_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if(fd<0){
		perror("gps_type set open\n");
		return BSP_ERROR;
	}
	write(fd, &g_Clock_Set_AllDate, sizeof(g_Clock_Set_AllDate));
	close(fd);
	if ((g_ublox_gps_type == 0) && (g_GNSS_flag == 2))
	{
		printf("ublox6T nonsupport beidou...\n");
	}
	else
	{
		bsp_switch_synsrc(g_Clock_Set_AllDate.gps_Type);
	}
	return BSP_OK;

}

#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
u32 get_synsrc_mask(void); 
static inline void syn_src_clear_alm(u8 u8SyncSrc); 

s32 bsp_set_clocksource(u32 u32SynSrc,  u8 ClockSynWay, u8 CascadeCfg)
{
	u8 u8flag_1st_config = TRUE;
	int index;
	s32 retval = BSP_ERROR;
	s32 SynSrc;
	u32 SynSrcMask, u32SynSrcMdf;
	T_SynSrcAlmInfo t_info;

	pthread_mutex_lock(&g_mp_synsrc_switch);
	for(index = 0; index < SYNSRC_MAX_NUM; index++)
	{
		syn_src_switch_info[index].SynSrcSwitchPat = SYNSRC_SWITCH_AUTO;
	}

	SynSrcMask = get_synsrc_mask(); 
	u32SynSrcMdf = u32SynSrc & SynSrcMask;
	if(u32SynSrc != u32SynSrcMdf)
	{
		printf("unsupported SyncSrc is configured! u32SynSrc=0x%08x SynSrcMask=0x%08x u32SynSrcMdf=0x%08x\n", 
				u32SynSrc, SynSrcMask, u32SynSrcMdf);
		SynSrcAlmFunc(BSP_ALM_SYN_SRC_INVALID_CONFIGURATION, &t_info);
	}

	for(index = 0; u32SynSrcMdf > 0; u32SynSrcMdf /= 2, index++)
	{
		if((u32SynSrcMdf & 1) != 0)
		{
			SynSrc = syn_src_switch_info[index].SynSrcType;
			printf("Configured SyncSrc: %d\n", SynSrc);

			if(TRUE == u8flag_1st_config)
			{
				u8flag_1st_config = FALSE;

				if((u32SynSrcMdf & 0xfffffffe) != 0)/*multi sync source*/
				{
					syn_src_switch_info[index].SynSrcSwitchPat = SYNSRC_SWITCH_CONFIG_MULTI_SYNSRC;
				}
				else/*single sync source*/
				{
					syn_src_switch_info[index].SynSrcSwitchPat = SYNSRC_SWITCH_CONFIG_SINGLE_SYNSRC;
				}
				
				if(SynSrc == g_Clock_Set_AllDate.gps_Type)
				{
					retval = (s32)SynSrc;
					printf("configured sync source is identical with current synsrc:%d\n", SynSrc);
					t_info.currentSynSrc = SynSrc;
					SynSrcAlmFunc(BSP_ALM_SYN_SRC_REMAIN_UNCHANGED, &t_info);

				}
				else
				{
					if(NULL != syn_src_switch_info[index].pReceiveReturn)
					{
						pthread_cond_signal(&g_cond_synsrc_switch);
						t_info.t_switchInfo.oldSynSrc = g_Clock_Set_AllDate.gps_Type;
						t_info.t_switchInfo.newSynSrc = SynSrc;
						syn_src_switch_info[index].pReceiveReturn();
						sleep(1);
						if(GPS_OK ==  gps_check_clock())
							afc_cpld_clk_rst_oc();
						SynSrcAlmFunc(BSP_ALM_SYN_SRC_SWITCH, &t_info);
						syn_src_clear_alm(t_info.t_switchInfo.oldSynSrc);
						retval = (s32)SynSrc;						
					}
					else
					{
						printf("pReceiveReturn for SyncSrc%d == NULL!\n", SynSrc);
					}
				}
			}
			else
			{
				if(NULL != syn_src_switch_info[index].pReceiveReturn)
					syn_src_switch_info[index].SynSrcSwitchPat = SYNSRC_SWITCH_CONFIG_MULTI_SYNSRC;
				else
					printf("pReceiveReturn for SyncSrc%d == NULL!\n", SynSrc);
			}
		}
	}
	pthread_mutex_unlock(&g_mp_synsrc_switch);
	return retval;
}
#endif/*__CPU_LTE_CENTERSTATION__*/

#ifdef __CPU_LTE_MACRO__
s32 bsp_set_clocksource(u32 u32SynSrc,  u8 ClockSynWay, u8 CascadeCfg)
{
	u8 u8Type;
	s32 retval;
	if(u32SynSrc & SYNSRC_GPS_MASK)
	{
		u8Type = 0;
	}
	else if(u32SynSrc & SYNSRC_BD_MASK)
	{
		u8Type = 2;
	}
	else if(u32SynSrc& SYNSRC_CASCADE_MASK)
	{
		u8Type = 255;
	}
	else if(u32SynSrc & SYNSRC_LOCAL_MASK)
	{
		u8Type = 1;
	}
	else
	{
		printf("[%s] unsupported sync source:%#x!\n", __func__, u32SynSrc);
		return BSP_ERROR;
	}
		
	retval = bsp_set_tc_type(u8Type, ClockSynWay, CascadeCfg);
	if(retval == BSP_ERROR)
		return BSP_ERROR;
	else
		return (s32)g_Clock_Set_AllDate.gps_Type;
}

#endif/*__CPU_LTE_MACRO__*/

void config_to_cascade(void)
{
#ifdef __CPU_LTE_MACRO__	
	//cascade slave
	if ((g_Clock_Set_AllDate.gps_Type == 0xFF) && (g_Clock_Set_AllDate.gps_ClockSynWay == 1) && (g_Clock_Set_AllDate.gps_CascadeCfg == 1))
	{
		bsp_fpga_write_reg(FPGA_SERIAL_CTRL_REG,1);
		afc_cpld_clk_rst_oc();
		printf("cascade slave\n");
	}
	
	// cascade master
	if ((g_Clock_Set_AllDate.gps_Type != 0xFF) && (g_Clock_Set_AllDate.gps_ClockSynWay == 1) && (g_Clock_Set_AllDate.gps_CascadeCfg == 0))
	{
		bsp_fpga_write_reg(FPGA_SERIAL_CTRL_REG,0x0);
		afc_cpld_clk_rst_oc();
		printf("cascade master\n");
	}
	// no cascade 
	if ((g_Clock_Set_AllDate.gps_Type != 0xFF) && (g_Clock_Set_AllDate.gps_ClockSynWay == 0) && (g_Clock_Set_AllDate.gps_CascadeCfg == 0xFF))
	{
		bsp_fpga_write_reg(FPGA_SERIAL_CTRL_REG,0x0);
		afc_cpld_clk_rst_oc();
		printf("no cascade\n");
	}
#endif	
}
/*******************************************************************************
* 函数名称: gps_Cfg_Tp1						
* 功    能: 配置10ms                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Cfg_Tp1b(void) 
{   
	/*char data[] = {0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 
        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 
        0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x07, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x01, 0x00, 0x00, 
        0x9f, 0x14 };*/
    char data[] = {0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x00, 0x01, 
        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0F, 0x00,
        0x40, 0x42, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x17, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x01, 0x00, 0x00, 
        0x03, 0x4a}; 

	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;
	printf("gps_Cfg_Tp1b\n");
	status = gpsExecCommand_UBlox(data, 40, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("\n[tGPS] gps_Cfg_Tp1b, return failure!\n"); 
		return GPS_ERROR;
	}

	return GPS_OK;
    
}
/*******************************************************************************
* 函数名称: gps_Cfg_Tp2						
* 功    能:    cfg time pulse 2,配置1s                                 
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Cfg_Tp2b(void) 
{   

	char data[] ={0xB5, 0x62, 0x06, 0x31, 0x20, 0x00, 0x01, 0x01, 
        0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x40, 0x42, 0x0F, 0x00, 
        0x40, 0x42, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x17,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF7, 0x01, 0x00, 0x00, 
        0x04, 0x6a};

	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;
	printf("gps_Cfg_Tp2b\n");
	status = gpsExecCommand_UBlox(data, 40, dataRsp, 0);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_Cfg_Tp2b, return failure!\n"); 
		return GPS_ERROR;
	}

	return GPS_OK;

}
/*******************************************************************************
* 函数名称: gps_Cfg_ANT_Poll						
* 功    能: 天线状态查询                                    
* 相关文档:                    
* 函数类型:	s32								
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值:	 0 正常
* 说   明:
*******************************************************************************/
s32 gps_Cfg_ANT_Poll(void) 
{    

	char data[]={ 0xB5, 0x62, 0x06, 0x13, 0x00, 0x00, 0x19, 0x51};
	
	char dataRsp[BSP_DATARSP_BUF_LEN] = {0};    
	int status;
	
	status = gpsExecCommand_UBlox(data, sizeof(data), dataRsp, 1);
	if(status!=OK)
	{
		bsp_dbg("[tGPS] gps_Cfg_ANT_Poll, return failure!, try again");
        return 1;
	}

    short flags = 0;
    flags = dataRsp[6];
    //memcpy(&flags, &dataRsp[6], sizeof(short));
    printf(" \nflags:[%#x, %#x]\n svcs[%s]\n scd[%s]\n ocd[%s]\n pdwnOnSCD[%s]\n recovery[%s]\n",
        dataRsp[6], dataRsp[7], 
        (0x1&flags) ? "Enable" : "Disable", 
        (0x2&flags) ? "Enable" : "Disable", 
        (0x4&flags) ? "Enable" : "Disable", 
        (0x8&flags) ? "Enable" : "Disable", 
        (0x10&flags) ? "Enable" : "Disable");

    printf(" \npins:[%#x, %#x]\n", 
        dataRsp[8], dataRsp[9]);
    
	return status;
}

u32 bsp_get_synsrc_configuration(u8 Value)
{
	int fd = 0;
	u32 u32SynSrc = 0, index;
	
	if (Value > 2 && Value < 0)
	{
		printf("Value invalid \n");
	}
	fd = open(gps_type_file, O_RDONLY);
	if(fd<0)
	{
		perror("gps get open");
		//return BSP_ERROR;
	}
	read(fd, &g_Clock_Set_AllDate, sizeof(g_Clock_Set_AllDate));
	close(fd);
	if (Value == 0)
	{
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
		pthread_mutex_lock(&g_mp_synsrc_switch);

		switch(g_Clock_Set_AllDate.gps_Type)
		{
			case SYNC_SOURCE_GPS: u32SynSrc |= SYNSRC_GPS_MASK; break;
			case SYNC_SOURCE_LOCAL: u32SynSrc |= SYNSRC_LOCAL_MASK; break;
			case SYNC_SOURCE_BD: u32SynSrc |= SYNSRC_BD_MASK; break;
			case SYNC_SOURCE_1PPS_TOD: u32SynSrc |= SYNSRC_1PPS_TOD_MASK; break;
			case SYNC_SOURCE_1588: u32SynSrc |= SYNSRC_1588_MASK; break;
			default: printf("invalid SynSrc:%d\n", g_Clock_Set_AllDate.gps_Type);
		}

		for(index = 0; index < SYNSRC_MAX_NUM; index++)
		{
			if(syn_src_switch_info[index].SynSrcSwitchPat == SYNSRC_SWITCH_CONFIG_MULTI_SYNSRC)
			{
				u32SynSrc |= (1 << index);
			}
		}

		printf("Sync source configuration: 0x%08x\n", u32SynSrc);
		pthread_mutex_unlock(&g_mp_synsrc_switch);
#endif
#ifdef __CPU_LTE_MACRO__
		switch(g_Clock_Set_AllDate.gps_Type)
		{
			case SYNC_SOURCE_GPS: u32SynSrc |= SYNSRC_GPS_MASK; break;
			case SYNC_SOURCE_LOCAL: u32SynSrc |= SYNSRC_LOCAL_MASK; break;
			case SYNC_SOURCE_BD: u32SynSrc |= SYNSRC_BD_MASK; break;
			case SYNC_SOURCE_CASCADE: u32SynSrc |= SYNSRC_CASCADE_MASK; break; //???
			default: printf("invalid SynSrc:%d\n", g_Clock_Set_AllDate.gps_Type);
		}
#endif
		return u32SynSrc;
	}
	if (1 == Value)
	{
		if (g_Clock_Set_AllDate.gps_ClockSynWay == 0)
		{
			printf("Init gps ClockSynWay: non cascade...\n");
		}
		if (g_Clock_Set_AllDate.gps_ClockSynWay == 1)
		{
			printf("Init gps ClockSynWay: cascade...\n");
		}
		return (u32)g_Clock_Set_AllDate.gps_ClockSynWay;
	}
	if (2 == Value)
	{
		if (g_Clock_Set_AllDate.gps_CascadeCfg == 0)
		{
			printf("Init gps cascadecfg: master ...\n");
		}
		if (g_Clock_Set_AllDate.gps_CascadeCfg == 1)
		{
			printf("Init gps cascadecfg: slave...\n");
		}
		return (u32)g_Clock_Set_AllDate.gps_CascadeCfg;
	}
	return BSP_ERROR;
}

/*******************************************************************************
* 函数名称: bsp_init_get_gps_tc_type(u8 Value)						
* 功    能: 获取时钟配置文件中的配置参数                                    
* 相关文档:                    
* 函数类型:	u8								
* 参    数: 获取类型					     			
* 参数名称     类型			
* 说   明: Value 0  获取gps_Type  	          /0 gps;          1 local clock;  2 beidou;  3 cascade  
*          Value 1  获取gps_ClockSynWay       /0 non cascade;  1 cascade            
*          Value 2  获取gps_CascadeCfg        /0 master;       1 slave              
* 作   者：贾雄 
* 时   间：2015-12-28
*******************************************************************************/
u8 bsp_init_get_gps_tc_type(u8 Value)
{
	int fd = 0;
	
	if (Value > 2 && Value < 0)
	{
		printf("Value invalid \n");
	}
	fd = open(gps_type_file, O_RDONLY);
	if(fd<0)
	{
		perror("gps get open");
		//return BSP_ERROR;
	}
	read(fd, &g_Clock_Set_AllDate, sizeof(g_Clock_Set_AllDate));
	close(fd);
	if (Value == 0)
	{
		if (g_Clock_Set_AllDate.gps_Type == 0)
		{
			printf("Init gps type: gps...\n");
		}
		if (g_Clock_Set_AllDate.gps_Type == 2)
		{
			printf("Init gps type: beidou...\n");
		}
		if (g_Clock_Set_AllDate.gps_Type == 1)
		{
			printf("Init gps type: local...\n");
		}
		return g_Clock_Set_AllDate.gps_Type; 
	}
	if (1 == Value)
	{
		if (g_Clock_Set_AllDate.gps_ClockSynWay == 0)
		{
			printf("Init gps ClockSynWay: non cascade...\n");
		}
		if (g_Clock_Set_AllDate.gps_ClockSynWay == 1)
		{
			printf("Init gps ClockSynWay: cascade...\n");
		}
		return g_Clock_Set_AllDate.gps_ClockSynWay;
	}
	if (2 == Value)
	{
		if (g_Clock_Set_AllDate.gps_CascadeCfg == 0)
		{
			printf("Init gps cascadecfg: master ...\n");
		}
		if (g_Clock_Set_AllDate.gps_CascadeCfg == 1)
		{
			printf("Init gps cascadecfg: slave...\n");
		}
		return g_Clock_Set_AllDate.gps_CascadeCfg;
	}
	return BSP_ERROR;
}

/*******************************************************************************
* 函数名称: bsp_gps_support_type(void)						
* 功    能: 获取gps支持的类型                                    
* 相关文档:                    
* 函数类型:	u8								
* 参    数: 获取类型					     			
* 参数名称     类型
* 说   明：ublox6T： 支持gps     ublox8T： 支持gps和beidou 			              
* 作   者：贾雄 
* 时   间：2015-12-28
*******************************************************************************/
u8 bsp_gps_support_type(void)
{
	u8 gps_support;
	if (g_ublox_gps_type == 0)
	{
		gps_support = 0x01;
		return gps_support;
	} 
	if (g_ublox_gps_type == 1)
	{
		gps_support = 0x03;
		return gps_support;
	}
	printf("get ublox type error\n");
	return BSP_ERROR;
}
/*******************************************************************************
* 函数名称: bsp_gps_get_clocksynway(void)						
* 功    能: 获取ClockSynWay类型                                    
* 相关文档:                    
* 函数类型:	u8								
* 参    数: 获取类型					     			
* 参数名称     类型
* 说   明：	              
* 作   者：贾雄 
* 时   间：2015-12-28
*******************************************************************************/
u8 bsp_gps_get_clocksynway(void)
{
	return g_Clock_Set_AllDate.gps_ClockSynWay;
}
/*******************************************************************************
* 函数名称: bsp_gps_get_cascadecfg(void)						
* 功    能: 获取CascadeCfg类型                                    
* 相关文档:                    
* 函数类型:	u8								
* 参    数: 获取类型					     			
* 参数名称     类型
* 说   明：		              
* 作   者：贾雄 
* 时   间：2015-12-28
*******************************************************************************/
u8 bsp_gps_get_cascadecfg(void)
{
	return g_Clock_Set_AllDate.gps_CascadeCfg;
}

u8 bsp_switch_synsrc(u16 synsrc) 
{
	if((synsrc != SYNSRC_LOCAL) && (synsrc != SYNSRC_GPS) && (synsrc != SYNSRC_BD) && (synsrc != SYNSRC_CASCADE))
	{
		printf("invalid SynSrc! SynSrc = %d\n", synsrc);
		return GPS_ERROR;
	} 
    if(synsrc == 1)
    {
    	//afc_sim_switch(1);
    }
	else
	{
    	//afc_sim_switch(0);
	}
	//afc_syn_src();
//	if(bts_synsrc_type != synsrc)
//	{
//		bts_synsrc_type = synsrc;
		switch(synsrc)
		{
			case SYNSRC_GPS:
                printf("synsrc change to GPS!\n");
                config_to_gps();
				config_to_cascade();
                break;
			case SYNSRC_LOCAL:
				printf("synsrc change to LOCAL!\n");
				config_to_cascade();
				break;
			case SYNSRC_BD:
                printf("synsrc change to BD!\n");
                config_to_beidou();
				config_to_cascade();
                break;
			case SYNSRC_CASCADE:        //cascade slave
				printf("synsrc change to CASCADE!\n");
				config_to_cascade();
				break;
			default:
				printf("synsrc is error!\n");
				printf("synsrc=%d\n", synsrc);
				break;
		}
	//}
	
	return GPS_OK;
}





/**************************************************ntpd*begin*******************************************************/
unsigned char timezoneinfo[25][20] = {
{"Kwajalein"},                 //west 12   夸贾林岛
{"Midway"},                    //west 11   中途岛
{"Honolulu"},                  //west 10   檀香山
{"Anchorage"},                 //west 9    安克雷奇  
{"Los_Angeles"},               //west 8    洛杉矶
{"Denver"},                    //west 7    丹佛
{"Managua"},                   //west 6    马那瓜
{"Bogota"},                    //west 5    波哥大
{"Caracas"},                   //west 4    加拉加斯
{"Montevideo"},                //west 3    蒙得维的亚
{"Noronha"},                    //west 2    巴西费尔南多
{"Azores"},                    //west 1    亚速尔群岛
{"London"},                    //     0    伦敦
{"Amsterdam"},                 //east 1    阿姆斯特丹
{"Cairo"},                     //east 2    开罗
{"Moscow"},                    //east 3    莫斯科
{"Mauritius"},                 //east 4    毛里求斯 
{"Maldives"},                  //east 5    马尔代夫
{"Colombo"},                   //east 6    科伦坡
{"Bangkok"},                   //east 7    曼谷
{"Shanghai"},                  //east 8    上海
{"Japan"},                     //east 9    东京
{"Melbourne"},                 //east 10   墨尔本
{"Magadan"},                   //east 11   马加丹 
{"Auckland"}                   //east 12   奥克兰
};
int g_ntpd_timezone = 0;
char * g_LoTimezoneInfo;
unsigned char *ntpd_timezone = "/mnt/csi/ntpd.txt";
#define PATH "/usr/share/zoneinfo/"
char cmd[50] = "";
/*******************************************************************************
* 函数名称: bsp_set_ntpd_timezone(void)						
* 功    能: 设置时区                                    
* 相关文档:                    
* 函数类型:	int								
* 参    数: 获取类型					     			
* 参数名称     类型
* 说   明：		              
* 作   者：贾雄 
* 时   间：2016-3-26
*******************************************************************************/
int bsp_set_ntpd_timezone(int Value)
{
	int fd = 0;
	int ret = 0;
	int timezone = 0;
	char pu8system_cmd[50] = "";
	if(Value < 0 || Value > 24){
		printf("[%s]:set error timezone!\r\n",__func__);
		return BSP_ERROR;
	}
	timezone = Value;
	fd = open(ntpd_timezone, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if(fd<0){
		perror("creat ntpd_timezone.txt error\n");
		return BSP_ERROR;
	}
	ret = write(fd, &timezone, sizeof(timezone));
	if(ret<0){
		perror("write ntpd_timezone.txt error\n");
		return BSP_ERROR;
	}
	close(fd);
	
	g_ntpd_timezone = timezone;
	g_LoTimezoneInfo = timezoneinfo[g_ntpd_timezone];
	gps_GMT_Offset = (g_ntpd_timezone - 12) * 60;
	sprintf(pu8system_cmd, "ln -sf %s%s  /etc/localtime", PATH, g_LoTimezoneInfo);
	system(pu8system_cmd);
	return BSP_OK;
}
/*******************************************************************************
* 函数名称: bsp_get_ntpd_timezone(void)						
* 功    能: 获取时区                                    
* 相关文档:                    
* 函数类型:	void								
* 参    数: 获取类型					     			
* 参数名称     类型
* 说   明：		              
* 作   者：贾雄 
* 时   间：2016-3-26
*******************************************************************************/
void bsp_get_ntpd_timezone(void)
{
	g_ntpd_timezone = bsp_get_ntpd_confile();
	// guanxi 
	g_LoTimezoneInfo = timezoneinfo[g_ntpd_timezone];
	gps_GMT_Offset = (g_ntpd_timezone - 12) * 60;
}
/*******************************************************************************
* 函数名称: bsp_get_ntpd_confile(void)						
* 功    能: 获取时区配置文件                                
* 相关文档:                    
* 函数类型:	int								
* 参    数: 获取类型					     			
* 参数名称     类型
* 说   明：		              
* 作   者：贾雄 
* 时   间：2016-3-26
*******************************************************************************/
int bsp_get_ntpd_confile(void)
{
	int fd = 0;
	int timezone = 0;
	fd = open(ntpd_timezone, O_RDONLY);
	if(fd >= 0)
	{
		
		read(fd, &timezone, sizeof(timezone));
		close(fd);

	}else 
	{
		timezone = 20;
		fd = open(ntpd_timezone, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
		if (fd < 0)
		{
			printf("creat ntpd_timezone.txt error\n");
			return timezone;
		}
		write(fd, &timezone, sizeof(timezone));
		printf("the first write ntpd_timezone.txt\n");
		close(fd);
	}
	printf("config ntpd file : %d\n", timezone);
	return timezone;
}
/*******************************************************************************
* 函数名称: bsp_init_timezone(void)						
* 功    能: 上电初始化时区                                   
* 相关文档:                    
* 函数类型:	void								
* 参    数: 获取类型					     			
* 参数名称     类型
* 说   明：		              
* 作   者：贾雄 
* 时   间：2016-3-26
*******************************************************************************/
void bsp_init_timezone(void)
{
	
	bsp_get_ntpd_timezone();
	
	sprintf(cmd, "ln -sf %s%s  /etc/localtime", PATH, g_LoTimezoneInfo);
	#if 1
	system(cmd);
	printf("bsp_int_timezoneinfo : %s\n", g_LoTimezoneInfo);
	#endif
}
/*******************************************************************************
* 函数名称: unsigned int bsp_ntpd_serve_client_config(unsigned int dwvalue)						
* 功    能: BBU是否更新RTC和系统时间                                   
* 相关文档:                    
* 函数类型:	unsigned int							
* 参    数: 获取类型					     			
* 参数名称     类型
* 说   明：	dwvalue 1更新RTC和系统时间   0不更新RTC和系统时间	              
* 作   者：贾雄 
* 时   间：2016-7-14
*******************************************************************************/
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
extern u8 u8TodUpdateRtc;
#endif
unsigned int bsp_ntpd_serve_client_config(unsigned int dwvalue)
{
	if (dwvalue != 1 && dwvalue != 0)
	{
		printf("bsp_ntpd_server_client_config is invalid : %d\n", dwvalue);
		return BSP_ERROR;
	}
	g_NtpdBbuConfig = dwvalue;
	if (g_NtpdBbuConfig == 1)
	{		
		printf("NTP Sever UpDateRtc From Gps Right Now!\n");
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
		if(SYNC_SOURCE_1PPS_TOD == g_Clock_Set_AllDate.gps_Type) // 1pps_tod
			u8TodUpdateRtc = 1;
		else if((SYNC_SOURCE_GPS == g_Clock_Set_AllDate.gps_Type) || (SYNC_SOURCE_BD == g_Clock_Set_AllDate.gps_Type))
		{
#endif
		if(bsp_gps_update_rtc()!= BSP_OK)
        {
			printf("ntpd update rtc error!\n");
		}
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
		}
#endif

	}else
	{
		printf("NTP Client Dont't UpDateRtc From Gps!\n");
	}
	
	return BSP_OK;
}

u32 bsp_get_synsrc(void)
{
	return (u32)g_Clock_Set_AllDate.gps_Type;
}

/************************************************ntpd*end**********************************************************/


/***********************SyncSrc Switch and Alarm*****************************/
#if defined (__CPU_LTE_CENTERSTATION__)||defined(__CPU_LTE_CARDTYPE__)
#include "../../ptpd/inc/ptpd.h"

extern STRU_AFC_STATE g_struAfcStat;
u8 u8AlmSynSrcErr = 0;

u32 bsp_register_synsrc_alm(SynSrcAlarmFuncPtr pfSynSrcAlm)
{
	if(NULL == pfSynSrcAlm)
		return BSP_ERROR;

	pfSynSrcAlmFunc = pfSynSrcAlm;
	return BSP_OK;
}


s32 SynSrcAlmFunc(u16 u16EventId, T_SynSrcAlmInfo *pt_info)
{
	static T_SynSrcAlmInfo t_info;
	if(pfSynSrcAlmFunc == NULL)
		return BSP_ERROR;

	if(pt_info == NULL)
		pt_info = &t_info;
	
	printf("ALARM![id:0x%04x]", u16EventId);
	if(BSP_ALM_SYN_SRC_ERROR == u16EventId || 
	   BSP_ALM_SYN_SRC_OK == u16EventId ||
	   BSP_ALM_SYN_SRC_REMAIN_UNCHANGED == u16EventId)
	   printf("currentSynSrc:%d\n", pt_info->currentSynSrc);
	else if(BSP_ALM_SYN_SRC_SWITCH == u16EventId)
		printf("Switch:%d--->%d\n",\
		pt_info->t_switchInfo.oldSynSrc, pt_info->t_switchInfo.newSynSrc);

	pfSynSrcAlmFunc(u16EventId, pt_info);
	return BSP_OK;
}

static inline void syn_src_clear_alm(u8 u8SyncSrc)
{
	T_SynSrcAlmInfo t_info;

	printf("clear sync src [%d] alarm!\n", u8SyncSrc);
	if(SYNC_SOURCE_1PPS_TOD == u8SyncSrc)
	{
		//tod_clear_alm();
	}
	else if(SYNC_SOURCE_1588 == u8SyncSrc)
	{
	    //set_1588_running_flag(FALSE);
		//ptp_clear_alm();
	}

	/* clear sync source error alarm */
	if(1 == u8AlmSynSrcErr)
	{
		t_info.currentSynSrc = u8SyncSrc;
		SynSrcAlmFunc(BSP_ALM_SYN_SRC_OK, &t_info);
		u8AlmSynSrcErr = 0;
	}

}

void syn_ch_to_485(void)
{
	 u8 syn=0;
     syn = bsp_cpld_read_reg(0xf);
	 bsp_cpld_write_reg(0xf, syn|0x40);
}

void syn_ch_from_485(void)
{
	 u8 syn=0;
     syn = bsp_cpld_read_reg(0xf);
	 bsp_cpld_write_reg(0xf, syn&0xbf);
}

void RE_init_gps()
{
    printf("GPS INIT again.......\n");
	if(GPS_OK != gps_uart_mode(0))
	{
		bsp_dbg("gps_uart_mode error!\n");
	}
#if 0   
	if(GPS_OK != ubx_init())
	{
		bsp_dbg("\nubx_init error!\n");
	}
#endif    
	 printf("done\n");
}

static inline s32 set_gps_type_file(UINT8 gpsType, UINT8 gps_ClockSynWay, UINT8 gps_CascadeCfg)
{	
	int fd;

	g_Clock_Set_AllDate.gps_Type = gpsType;
	g_Clock_Set_AllDate.gps_ClockSynWay = gps_ClockSynWay;
	g_Clock_Set_AllDate.gps_CascadeCfg = gps_CascadeCfg;
	fd = open(gps_type_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if( fd < 0 )
	{
		printf("failed to open gps_type_file: %m\n");
		return BSP_ERROR;
	}
	if(write(fd, &g_Clock_Set_AllDate, sizeof(g_Clock_Set_AllDate)) < 0)
	{
		printf("failed to write gps_type_file: %m\n");
		close(fd);
		return BSP_ERROR;
	}
	
	close(fd);
	return BSP_OK;
}

void syn_ch_to_gps(void)
{
    //afc_sim_switch(0);
	if(SYNC_SOURCE_BD == g_Clock_Set_AllDate.gps_Type)
	{
		config_to_gps();
	}
	else
	{		
    	//set_1588_running_flag(FALSE);
		syn_ch_from_485();
		RE_init_gps();
		config_to_gps();
	}
	set_gps_type_file(SYNC_SOURCE_GPS, 0, 0xff);
	printf("synsrc change to gps...\n");
}

void syn_ch_to_beidou(void)
{
    //afc_sim_switch(0);
	if(SYNC_SOURCE_GPS == g_Clock_Set_AllDate.gps_Type)
	{
		config_to_beidou();
	}
	else
	{		
    	//set_1588_running_flag(FALSE);
		syn_ch_from_485();
		RE_init_gps();
		config_to_beidou();
	}

	set_gps_type_file(SYNC_SOURCE_BD, 0, 0xff);
	printf("synsrc change to beidou...\n");
}
extern s32 tod_uart_set(void);
void syn_ch_to_1pps_tod(void)
{
    //afc_sim_switch(0);
    //set_1588_running_flag(FALSE);
	syn_ch_to_485();
	tod_uart_set();
	set_gps_type_file(SYNC_SOURCE_1PPS_TOD, 0, 0xff);
	printf("synsrc change to 1ppd+tod...\n");
}

void RE_init_1588v2()
{
}

void syn_ch_to_1588v2(void)
{
    //afc_sim_switch(0); //???
    setPtpTimeFromSystem(0);
    //set_1588_running_flag(TRUE);
    set_gps_type_file(SYNC_SOURCE_1588, 0, 0xff);
	printf("synsrc change to 1588...\n");
}

void syn_ch_to_es(void)
{
    //afc_sim_switch(0);
    //set_1588_running_flag(FALSE);
	set_gps_type_file(SYNC_SOURCE_ES, 0, 0xff);
	printf("synsrc change to es...\n");
}

void syn_ch_to_local(void)
{    
	//afc_sim_switch(1);
    //set_1588_running_flag(FALSE);
	set_gps_type_file(SYNC_SOURCE_LOCAL, 0, 0xff);
	printf("synsrc change to local...\n");
}

int enable_1588v2(void)
{
	if(SYNC_SOURCE_1588 == g_Clock_Set_AllDate.gps_Type)
		return TRUE;
	else
		return FALSE;
}

u32 get_synsrc_mask(void)
{
	u32 u32SynSrcMask = 0;

	if(g_ublox_gps_type == 0)
	{
		u32SynSrcMask |= SYNSRC_GPS_MASK;
	}
	else if(g_ublox_gps_type == 1)
	{
		u32SynSrcMask |= (SYNSRC_GPS_MASK| SYNSRC_BD_MASK);
	}

	u32SynSrcMask |= (SYNSRC_1PPS_TOD_MASK| SYNSRC_1588_MASK| SYNSRC_LOCAL_MASK);

	return u32SynSrcMask;
}

static inline u8 swith_to_next_synsrc(void)
{
	int i, index_current_synsrc;
	#if 0
	for(i = 0; i < SYNSRC_MAX_NUM; i++)
	{
		if((SYNSRC_SWITCH_CONFIG_MULTI_SYNSRC== syn_src_switch_info[i].SynSrcSwitchPat) &&
			(NULL != syn_src_switch_info[i].pReceiveReturn))
		{
			syn_src_switch_info[i].pReceiveReturn();
			syn_src_switch_info[i].SynSrcSwitchPat = SYNSRC_SWITCH_AUTO;
			return syn_src_switch_info[i].SynSrcType;
		}
	}

	for(index_current_synsrc = 0; index_current_synsrc < SYNSRC_MAX_NUM; index_current_synsrc++)
	{
		if(syn_src_switch_info[i].SynSrcType == g_Clock_Set_AllDate.gps_Type)
		{
			break;
		}
	}
	for(i = index_current_synsrc + 1; i < SYNSRC_MAX_NUM; i++)
	{
		if(NULL != syn_src_switch_info[i].pReceiveReturn)
		{
			syn_src_switch_info[i].pReceiveReturn();
			return syn_src_switch_info[i].SynSrcType;
		}
	}
	#else
	for(index_current_synsrc = 0; index_current_synsrc < SYNSRC_MAX_NUM; index_current_synsrc++)
	{
		if(syn_src_switch_info[index_current_synsrc].SynSrcType == g_Clock_Set_AllDate.gps_Type)
		{
			break;
		}
	}
	
	bsp_dbg("index_current_synsrc: %d\n", index_current_synsrc);

	if(SYNSRC_SWITCH_CONFIG_MULTI_SYNSRC != syn_src_switch_info[index_current_synsrc].SynSrcSwitchPat)
	{
		return g_Clock_Set_AllDate.gps_Type;
	}
	
	for(i = 0; i < SYNSRC_MAX_NUM; i++)
	{
		index_current_synsrc = (index_current_synsrc + 1) % SYNSRC_MAX_NUM;
		if((SYNSRC_SWITCH_CONFIG_MULTI_SYNSRC == syn_src_switch_info[index_current_synsrc].SynSrcSwitchPat) &&
			(NULL != syn_src_switch_info[index_current_synsrc].pReceiveReturn))
		{
			syn_src_switch_info[index_current_synsrc].pReceiveReturn();
			return syn_src_switch_info[index_current_synsrc].SynSrcType;
		}
	}

	#if 0
	printf("after searching: index_current_synsrc = %d\n", index_current_synsrc);
	for(i = index_current_synsrc + 1; i < SYNSRC_MAX_NUM; i++)
	{
		if(NULL != syn_src_switch_info[i].pReceiveReturn && i != SYNSRC_BD_SHIFT)
		{
			syn_src_switch_info[i].pReceiveReturn();
			return syn_src_switch_info[i].SynSrcType;
		}
	}
	#endif
	#endif
	return g_Clock_Set_AllDate.gps_Type;
}

void bsp_syn_src_auto_switch(void)
{
	u32 u32check_freq = 0;
	struct timespec ts = {0, 0};
	u8 gpsType = g_Clock_Set_AllDate.gps_Type;
	T_SynSrcAlmInfo t_info;
	u8 u8FlagWaitAfcWarmUp = 0;
	
	/* use CLOCK_MONOTONIC */
	pthread_condattr_t attr;
	if(pthread_condattr_init(&attr) != 0)
	{
		perror("[bsp_syn_src_auto_switch]pthread_condattr_init failed");
		return;
	}
	if(pthread_condattr_setclock(&attr, CLOCK_MONOTONIC) != 0)
	{
		printf("[bsp_syn_src_auto_switch]pthread_condattr_setclock failed!\n");
		return;
	}
	if(pthread_cond_init(&g_cond_synsrc_switch, &attr) != 0)
	{
		printf("[g_cond_synsrc_switch]pthread_cond_init failed!\n");
		return;
	}
	//pthread_condattr_destroy(&attr);
	
	pthread_mutex_lock(&g_mp_synsrc_switch);
	while(1)
	{
 		//if( SYNC_SOURCE_GPS == gpsType || SYNC_SOURCE_BD == gpsType || SYNC_SOURCE_1PPS_TOD == gpsType )
 		if(SYNC_SOURCE_LOCAL != gpsType)
		{
			/*REQUEST: start latter than afc thread*/
			while(1)
			{
				clock_gettime(CLOCK_MONOTONIC, &ts);
                if(SYNC_SOURCE_1588 == gpsType)
                    ts.tv_sec += (3600);
                else
                {
				if(AFC_LOCK == g_struAfcStat.PhaseLockStage)
					ts.tv_sec += (300);
				else if((AFC_WARMUP == g_struAfcStat.PhaseLockStage) && (0 == u8FlagWaitAfcWarmUp))
				{
					ts.tv_sec += (240 + 960);
					u8FlagWaitAfcWarmUp = 1;
				}
				else
					ts.tv_sec += (960);
                }

				//printf("[%s][%lu]start waiting...\n", __func__, time(NULL));
				pthread_cond_timedwait(&g_cond_synsrc_switch, &g_mp_synsrc_switch, &ts);
				//printf("[%s][%lu]end waiting...\n", __func__, time(NULL));
				
				if(gpsType == g_Clock_Set_AllDate.gps_Type)
				{
					if(g_struAfcStat.PhaseLockStage != AFC_LOCK /*&& 
						 g_struAfcStat.PhaseLockStage != AFC_HOLDOVER*/) /* current synsrc is useless */
					{
						/*sync source error alarm!*/
						if(0 == u8AlmSynSrcErr)
						{
							t_info.currentSynSrc = gpsType;
							SynSrcAlmFunc(BSP_ALM_SYN_SRC_ERROR, &t_info);
							u8AlmSynSrcErr = 1;
						}
						
						swith_to_next_synsrc();
						/*sync source switch alarm!*/
						if(gpsType != g_Clock_Set_AllDate.gps_Type)
						{
							t_info.t_switchInfo.oldSynSrc = gpsType;
							t_info.t_switchInfo.newSynSrc = g_Clock_Set_AllDate.gps_Type;
							SynSrcAlmFunc(BSP_ALM_SYN_SRC_SWITCH, &t_info);
							syn_src_clear_alm(gpsType);
							gpsType = g_Clock_Set_AllDate.gps_Type;
							u8AlmSynSrcErr = 0;
						}
						break;
					}
					else
					{
						if(1 == u8AlmSynSrcErr)
						{
							t_info.currentSynSrc = gpsType;
							SynSrcAlmFunc(BSP_ALM_SYN_SRC_OK, &t_info);
							u8AlmSynSrcErr = 0;
						}
					}
				}
				else
				{
					gpsType = g_Clock_Set_AllDate.gps_Type;
					break;
				}
			}		
		}
        #if 0
		else if(SYNSRC_1588 == gpsType)
		{
			while(1)
			{
				clock_gettime(CLOCK_MONOTONIC, &ts);
				ts.tv_sec += (3600);
				printf("[%s][%lu]start waiting...\n", __func__, time(NULL));
				pthread_cond_timedwait(&g_cond_synsrc_switch, &g_mp_synsrc_switch, &ts);
				printf("[%s][%lu]end waiting...\n", __func__, time(NULL));
				
				if(gpsType == g_Clock_Set_AllDate.gps_Type)
				{
					if(g_struAfcStat.PhaseLockStage != AFC_LOCK) /* 1588 error */
					{
						/*sync source error alarm!*/
						t_info.currentSynSrc = gpsType;
						SynSrcAlmFunc(BSP_ALM_SYN_SRC_ERROR, &t_info);

						swith_to_next_synsrc();
						/*sync source swithc alarm!*/
						if(gpsType != g_Clock_Set_AllDate.gps_Type)
						{
							t_info.t_switchInfo.oldSynSrc = gpsType;
							t_info.t_switchInfo.newSynSrc = g_Clock_Set_AllDate.gps_Type;
							SynSrcAlmFunc(BSP_ALM_SYN_SRC_SWITCH, &t_info);
							syn_src_clear_alm(gpsType);
							gpsType = g_Clock_Set_AllDate.gps_Type;
						}
						break;
					}
					else
					{
						if(1 == u8AlmSynSrcErr)
						{
							t_info.currentSynSrc = gpsType;
							SynSrcAlmFunc(BSP_ALM_SYN_SRC_OK, &t_info);
							u8AlmSynSrcErr = 0;
						}
					}
				}
				else
				{
					gpsType = g_Clock_Set_AllDate.gps_Type;
					break;
				}
			}
		}
        #endif
		else/*local*/
		{
			pthread_cond_wait(&g_cond_synsrc_switch, &g_mp_synsrc_switch);
			gpsType = g_Clock_Set_AllDate.gps_Type;
		}
	}
	pthread_mutex_unlock(&g_mp_synsrc_switch);
}

void bsp_init_syn_src_switch_info(void)
{
	int i;
	int rtn;
	pthread_t pid;
	pthread_attr_t attr;
	struct sched_param param;

	memset(syn_src_switch_info, 0, sizeof(syn_src_switch_info));
	
	syn_src_switch_info[SYNSRC_GPS_SHIFT].SynSrcType = SYNC_SOURCE_GPS;
	syn_src_switch_info[SYNSRC_BD_SHIFT].SynSrcType = SYNC_SOURCE_BD;
	syn_src_switch_info[SYNSRC_1PPS_TOD_SHIFT].SynSrcType = SYNC_SOURCE_1PPS_TOD;
	syn_src_switch_info[SYNSRC_1588_SHIFT].SynSrcType = SYNC_SOURCE_1588;
	syn_src_switch_info[SYNSRC_ES_SHIFT].SynSrcType = SYNC_SOURCE_ES;
	syn_src_switch_info[SYNSRC_LOCAL_SHIFT].SynSrcType = SYNC_SOURCE_LOCAL;
	
	syn_src_switch_info[SYNSRC_GPS_SHIFT].pReceiveReturn = (pfSynSrcSwitch)syn_ch_to_gps;
	syn_src_switch_info[SYNSRC_BD_SHIFT].pReceiveReturn = (pfSynSrcSwitch)syn_ch_to_beidou;
	syn_src_switch_info[SYNSRC_1PPS_TOD_SHIFT].pReceiveReturn = (pfSynSrcSwitch)syn_ch_to_1pps_tod;
	syn_src_switch_info[SYNSRC_1588_SHIFT].pReceiveReturn = (pfSynSrcSwitch)syn_ch_to_1588v2;
	syn_src_switch_info[SYNSRC_ES_SHIFT].pReceiveReturn = (pfSynSrcSwitch)syn_ch_to_es;
	syn_src_switch_info[SYNSRC_LOCAL_SHIFT].pReceiveReturn = (pfSynSrcSwitch)syn_ch_to_local;
	
	pthread_attr_init(&attr);
	pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    param.sched_priority = 30; 
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedparam(&attr, &param);
    rtn = pthread_create(&pid, &attr, (FUNCPTR)bsp_syn_src_auto_switch,NULL);
    pthread_attr_destroy(&attr);
    if (-1 == rtn)
    {
        perror("create synsrc switch thread error!\n");
    }	
}
#else
s32 bsp_get_ptp_gm_clockIdentity (u8 *pu8GMclockIdentity)
{
	return BSP_OK;
}

s32 bsp_get_ptp_gm_clockClass (u8 *pu8gmClockClass)
{
	return BSP_OK;
}

s32 bsp_get_ptp_gm_clockAccuracy(u8 *pu8gmClockAccuracy)
{
	return BSP_OK;
}

s32 bsp_set_1588_attr(struct st1588Attr *pAttr)
{
	return 0;
}

u32 bsp_register_synsrc_alm(SynSrcAlarmFuncPtr pfSynSrcAlm)
{
	return BSP_OK;
}

#endif/*define __CPU_LTE_CENTERSTATION__*/
/*************************** SyncSrc Switch and Alarm*end**********************************************************/

/******************************* 源文件结束 ********************************/
