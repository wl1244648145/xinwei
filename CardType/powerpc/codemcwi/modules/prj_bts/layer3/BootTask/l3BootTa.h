/*****l3BootTask.h***********
*Author: dingfojin
*date: 2005.12.2
****************************/

#ifndef _L3BOOTTASK_H
#define _L3BOOTTASK_H

#include "vxWorks.h"
#include "iv.h"
#include "memLib.h"
#include "intLib.h"
#include "stdio.h"
#include "stdlib.h"
#include "netLib.h"

#include "net/protosw.h"
#include "sys/socket.h"
#include "errno.h"
#include "net/if.h"
#include "net/route.h"
#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"
#include "netinet/ip.h"
#ifndef WBBU_CODE
#include "netinet/if_ether.h"
#include "net/if_subr.h"
#endif
#include "m2Lib.h"


#include "mcWill_bts.h"

#ifndef WBBU_CODE
#include "mv64360.h"
#endif
#include "config.h"
#include "BizTask.h"
#include "Timer.h"
#include "btsConfig.h"

#define MAC_ADRS_L2_OFFSET	(0x40000 - 0x110)  /** point last SRAM **/

/***cpld register*****/
#define CPLD_BASE 0x1C000000
//#define CPLD_WRITE_REG(reg, value)  {*(int *)(CPLD_BASE + reg) = value ; }
//#define CPLD_READ_REG(reg, value)	{ *value = *(int *)(CPLD_BASE + reg); }
#define CPLD_WRITE_REG(reg, temp)  {unsigned int  val;  val = /*LONGSWAP*/ (temp);	*(int *)(CPLD_BASE + reg) = val ; }
#define CPLD_READ_REG(reg, temp)	{unsigned int  val; val = *(int *)(CPLD_BASE + reg);  *temp = /*LONGSWAP */(val); }


#define RESET_REG		0x0C   
#define FPGA_REG		0x08   
#define FPGA_STATE_REG  0x04   
#define MANUFA_DATE_REG 0x14   /**manufacture date****/
#define CONFIG_DATASTATE_REG 0x18   /**manufacture date****/
#define CPLD_REV_REG	0x10   
#define SYSTEM_CMD_REG	0x00   

//lijinan 20090224--------------------------
#define  L2_PRE_FINISH	  		0xf0000001	//preloader执行完毕，准备跳转

#define  L2_APP_START			0xf0000002

#define  L2_PCI_BUFFER_ERR		0xf0001000
#define  L2_PCI_NETPOOL_ERR		0xf0001001
#define  L2_PCI_END_ERR		0xf0001002
#define  L2_PCISIO_PTY_ERR		0xf0001003
#define  L2_PCISIO_TASK_ERR		0xf0001004
#define  L2_BOOT_FTP_CREATE_ERR	0xf0001005
#define  L2_BIZ_TASK_ERR		0xf0001006
#define  L2_BOOT_END			0xf0002000

//------------------------------------

typedef enum
{
    DATA_SOURCE_CF = 0,
    DATA_SOURCE_MV
}T_BOOT_SOURCE;

#ifndef WBBU_CODE
/**filename**/
#define FTP_DEVICE_NAME		"/FTPSERVER/"
#define RAMDISK_DOWNLOAD_PATH	"load/"
#define RAMDISK_CPE_DIR_NAME  "/RAMD/cpe/"   /*  "/RAMDISK/"   */
#define RAMDISK_CALIBRATION_DIR  "/RAMD/cal/"	/*--by xiaoweifang*/
#define RAMDISK_TEMP             "/RAMD/temp/"
#else
/**filename**/
#define FTP_DEVICE_NAME		"/FTPSERVER/"
#define RAMDISK_DOWNLOAD_PATH	"load/"
#define RAMDISK_CPE_DIR_NAME  "/RAMD:0/cpe/"   /*  "/RAMDISK/"   */
#define RAMDISK_CALIBRATION_DIR  "/RAMD:0/cal/"	/*--by xiaoweifang*/
#define RAMDISK_CALIBRATION_PATH "/RAMD:0/cal/"
#define RAMDISK_CAL_PATH  "/RAMD:0/cal/"
#define RAMDISK_TEMP             "/RAMD:0/temp/"
#endif
#define FTP_SERVER_PATH		   "" /* "download/"  */
#define FILE_PATH_CF_A		"/ata0a/btsA/"
#define FILE_PATH_CF_B		"/ata0a/btsB/"
/*#define COPY_FILE_FROM_FTPSERVER	1*/

#define FILENAME_LEN  40  /* 40*/


/**element index***/
#define L2		0x00
#define AUX		0x01
#define MCP		0x02
#define FEP		0x03
#define L1		0x04
#define BTS     0x05
#define FPGA    0x06
#define L2_PRELOADER  0x7

#define ELEMENT_ALL	0x7F

#define MCP0  0x0
#define MCP1  0x1
#define MCP2  0x2
#define MCP3  0x3
#define MCP4  0x4
#define MCP5  0x5
#define MCP6  0x6
#define MCP7  0x7

#define FEP0  0x0
#define FEP1  0x1


/**message id***/
#define L2_BOOTROM_RUNING	0x0B00
#define L2_PRELOADER_RUNING	0xaaaa/* 0x0B01 */
#define L2_REQUEST_RESET    0x9999
#define L2_RUNING			0x0B02
#define AUX_RESET			0x0B03
#define AUX_RUNING			0x0B04
#define MCP_RESET			0x0B05
#define MCP_RUNING			0x0B06
#define FEP_RESET			0x0B07
#define FEP_RUNING			0x0B08
#define BOOT_TIMER_EXPIRES	0x0B09
#ifdef WBBU_CODE
#define DSP_BOOT                    0x0b0a
#endif
#define MSGID_DIAG_5MIN_TIME_OUT         0x3901  //lijinan 20090202

/**from oam message*****/
#define BOOT_ALARM			0x2070
#define BOOT_WORKING		0x2071
#define BOOT_SYSTEM_RUNING  0x2072
#define BOOT_SYSTEM_RESET   0x2073


#define MCP_DOWN			0x0B13
#define AUX_DOWN			0x0B14
#define FEP_DOWN			0x0B15
#define L3TOMCP_RESET		0x0B17


/**timer expires***/
#define L2_BOOT_EXPIRES  	0x300000  //0x30000
#define L2_BOOT_IMAGE_TIMER_LEN    30000    // 30 seconds
#ifndef WBBU_CODE
#define L2_PRELOADER_BOOT_TIMER_LEN 10000 //10 sec //3000   // 3 seconds
#else
#define L2_PRELOADER_BOOT_TIMER_LEN  3000   // 3 seconds
#endif
#define MCP_BOOT_EXPIRES  	10000    //0x10000
#define AUX_BOOT_EXPIRES  	10000    //0x10000
#define FEP_BOOT_EXPIRES  	15000    //0x10000

/***alarm type*******/	
#define ALARM_RESET			0x0
#define ALARM_EXPIRES		0x1

#define REBOOT_DISABLE		0x55
#define REBOOT_ENABLE		0x88


class L3BootTask : public CBizTask{
	typedef enum{   
		RESET,
		BOOTING,
		RUNNING,
		TIME_OUT_RESET
	}T_RunningState;

	/***down message*****/
	struct down_message{
		char element;
		char index;
		char filename[FILENAME_LEN];  
	};

	struct alarm_message{
		short tranId;
		char element;
		char index;
		short type;  
	};

	struct boot_config{
		int emsip;
		int btsId;
		int vlanId;
		int dataSource;  
		short btsRstReason;
	};

	public:
		/****test**************/
		char L2_apppName[20];		

	private:
		static L3BootTask *instance;
		T_RunningState state_MCP[M_NumberOfMCP];   // initial state = BOOTING
		T_RunningState state_FEP[M_NumberOfFEP];    // initial state = BOOTING
		T_RunningState state_AUX;   // initial state = BOOTING
		T_RunningState state_L2;   // initial state = BOOTING
		T_RunningState state_System;   // initial state = BOOTING
		CTimer  *timer_MCP[M_NumberOfMCP];
		CTimer  *timer_FEP[M_NumberOfFEP];
		CTimer  *timer_AUX;
		CTimer  *timer_L2;
#ifdef WBBU_CODE
		CTimer * Vertimer ;
             CTimer  *ActiveUsertime;
#endif
        CTimer  *timer_L2Preloader;
		int BootSource;
		/*	struct boot_config *config_par;*/



	public:		  
		L3BootTask ();
		TID GetEntityId() const;
		bool Initialize();
		bool ProcessComMessage(CComMessage* pComMsg);
		void system_reset(char element, char index);
		~L3BootTask ();
		static L3BootTask *GetInstance();
#ifdef WBBU_CODE
		 static void BootPCallBack(char *, UINT16);
#endif
		virtual bool IsNeedTransaction() const;
		int copy_file_to_ramdisk(const char *filename);
        void ShowBootState();
		
	 //lijinan 090105 for jamming rpt flow ctl	
        bool getState_System(){if(state_System==RUNNING)
        						return 1;
	 							else return 0;};
#ifdef WBBU_CODE
	  void down_fpga_wbbu();
#endif
			/*lijinan 20090202*/
		void L3BOOT_5MinTIMER_Expires();

		 void resetBtsExcepL3();
		
	private:

        #define BOOT_MAX_BLOCKED_TIME_IN_10ms_TICK (60000)   /* 10 minutes */
        bool IsMonitoredForDeadlock()  { return false; };  /* can't enable, FTP server maybe too slow */
        int GetMaxBlockedTime() { return WAIT_FOREVER ;};

	 


		/***message process function*****/
		bool L3BOOT_L2PreLoaderRuning();
		bool L3BOOT_L2AppRuning();
        bool L3BOOT_L2RequestReboot();
		bool L3BOOT_AUX_Rest();
		bool L3BOOT_AUX_Runing();
		bool L3BOOT_MCP_Rest(CComMessage* pComMsg);
		bool L3BOOT_MCP_Runing(CComMessage* pComMsg);
		bool L3BOOT_FEP_Rest(CComMessage* pComMsg);
		bool L3BOOT_FEP_Runing(CComMessage* pComMsg);
		bool L3BOOT_TIMER_Expires(CComMessage* pComMsg);
		bool L3BOOT_System_Rest(CComMessage* pComMsg);


		
		/***other function***/		
		void send_working_message(char element, char index);
		void send_system_runing();
		
		void down_fpga();
        CTimer *CreateBootTimer(int cpuType, int index, int timerLen);
        void SendCpuResetAlarm(int cpuType, int index);
};

extern "C" int rebootMCP(char index);
#ifdef WBBU_CODE
extern "C" void	set_DCLK(UINT8 setting);
extern "C" void	set_DATA0(UINT8 setting);
//extern "C" UINT8    read_nSTATUS(void);
extern "C" UINT8	 read_CONF_DONE(void);
//extern "C"  void      fpga_reset(void);
extern "C" UINT8	read_nINT();
extern "C" void	set_nPROGRAM(UINT8 setting);
#endif
#endif /* _L3BOOTTASK_H */

