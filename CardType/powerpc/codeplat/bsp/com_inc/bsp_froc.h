/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_frocktest_ext.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_FROCKTEST_H
#define BSP_FROCKTEST_H

#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "bsp_types.h"
//协议定义
extern unsigned int frocktest_socket_fd;

//命令字
#define	CMD_CONNECT_TARGET	0x0000
#define CMD_FPGA_LOAD		0x0001
#define CMD_DSP_LOAD		0x0002
#define CMD_CPLD_LOAD		0x0003
#define CMD_FPGA_IR			0x0004
#define CMD_FPGA_DDR		0x0005
#define CMD_AIF				0x0006
#define CMD_PHY				0x0007
#define CMD_NET				0x0008
#define CMD_SRIO			0x0009
#define CMD_LBS				0x000A
#define CMD_PPC_DDR			0x000B
#define CMD_NORFLASH		0x000C
#define CMD_NANDFLASH		0x000D
#define CMD_RJ485			0x000E
#define CMD_GPS				0x000F
#define CMD_ETHSW			0x0010
#define CMD_RTC				0x0011
#define CMD_PPC_INTERRUPT	0x0012
#define CMD_FUN				0x0013
#define CMD_TEMPERATURE		0x0014
#define CMD_OCXO			0x0015
#define CMD_LED				0x0016
#define CMD_DSP_DDR			0x0017
#define CMD_DSP_GPIO		0x0018
#define CMD_EEPROM_SET		0x0019
#define CMD_EEPROM_GET		0x001a
#define CMD_SEC				0x001b
#define CMD_MAINNODE		0x001c
#define CMD_CPLD_VERSION	0x001d
#define CMD_SFP				0x001e
#define CMD_BOOT_SELECT		0x001f

#define CMD_TESTALL			0xAAAA
#define CMD_STOP_TESTALL	0xCCCC

#define	CMD_ACK				0xFFFF
#define	CMD_MSG				0xEEEE

#define	CMD_CPUINFO			0xFF01
#define	CMD_MEMINFO			0xFF02
#define	CMD_MTDINFO			0xFF03
#define	CMD_GETETH3IP		0xFF04
#define	CMD_GETETH3MAC		0xFF05
#define	CMD_GETSYSINFO		0xFF06
#define	CMD_GETBOOTCMD		0xFF07

//pkg format:
//    报文头    序号   命令字 状态  数据长度    数据          
// 0xaabbbeef  0x0000 0x0001 0x00 0x00000000 | cmddata

#define HEADER_LENGTH		15

#define OFFSET_CMDNO			4
#define OFFSET_CMDID			6
#define OFFSET_STATUS		(OFFSET_CMDID+2)
#define OFFSET_LENGTH		(OFFSET_STATUS+1)
#define OFFSET_DATA			(OFFSET_LENGTH+4)

#define BIT_SRIO_DSP0_DSP1	0
#define BIT_SRIO_DSP1_DSP2	1
#define BIT_SRIO_DSP2_DSP3	2
#define BIT_SRIO_DSP3_DSP0	3
#define BIT_SRIO_DSP1_DSP3	4
#define BIT_SRIO_DSP0_DSP2	5
#define BIT_SRIO_FPGA_DSP0	6
#define BIT_SRIO_FPGA_DSP1	7
#define BIT_SRIO_FPGA_DSP2	8
#define BIT_SRIO_FPGA_DSP3	9
#define BIT_SRIO_PPC_FPGA	10
//#define PC

#ifdef PC
#define uswap16(x) \
	((((x) & 0xff00) >> 8) | \
	 (((x) & 0x00ff) << 8))
#define uswap32(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))
#else
#define uswap16(x) (x)
#define uswap32(x) (x)
#endif
//数据结构

struct sfp_info{
	int tx_power;
	int rx_power;
};

typedef struct i2c_date
{
    s8  pcb_version[32];	
	u8  product_date[4];
	u8  mac_addr1[6];
	u8  mac_addr2[6];
	u8  device_id[32];
}T_I2C_TABLE;
struct meminfo_t{
	unsigned int addr;
	unsigned int length;
};

struct gpsinfo_t{
	unsigned int longitude;
	unsigned int latitude;
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;
};

struct rtc_t {
	unsigned int year;
	unsigned int month;
	unsigned int day;
	unsigned int hour;
	unsigned int minute;
	unsigned int second;		
};

struct cmd_data{
	unsigned short cmdno;
	unsigned short cmdid;
	unsigned char support_msg;
	unsigned char res;
	unsigned int length;
	struct sockaddr_in fromto;	
	
	union{
		unsigned short st_status;
		unsigned int st_times;
		unsigned int st_ip;		
		unsigned char st_mac[6];
		unsigned char st_data[2048];
		struct sfp_info sfp[3];
		struct meminfo_t st_mem;
		struct gpsinfo_t st_gps;
		unsigned int st_fun_speed;
		struct rtc_t st_rtc;
		char st_temperature;
		T_I2C_TABLE st_eeprom;
	}u_data;
};

#define cm_status u_data.st_status
#define cm_times u_data.st_times
#define cm_sfp u_data.sfp
#define cm_eeprom u_data.st_eeprom
#define cm_ip u_data.st_ip
#define cm_mac u_data.st_mac
#define cm_data u_data.st_data
#define cm_mem u_data.st_mem
#define cm_gps u_data.st_gps
#define cm_fun_speed u_data.st_fun_speed
#define cm_rtc u_data.st_rtc
#define cm_temperature u_data.st_temperature

//接口

/***************************************************
* 函数名： send_msg
*   功能： 发送消息报文
*   参数：
*		struct cmd_data cmddata	测试结果数据结构
*
*   返回：
****************************************************/

extern struct cmd_data cmdall;
extern unsigned int is_alltesting;
extern pthread_mutex_t alltest_lock;

#if 0
#define send_msg(pcmd, fmt...) printf("[FrockTest]:"fmt)
#else
#define send_msg(pcmd, fmt...)\
{\
	unsigned char sendbuf[1024*10]={0};	\
	time_t t;\
	struct tm *p;\
	sendbuf[0] = 0xaa;\
	sendbuf[1] = 0xbb;\
	sendbuf[2] = 0xbe;\
	sendbuf[3] = 0xef;\
	*(unsigned short*)(sendbuf+OFFSET_CMDNO) = 0;\
	*(unsigned short*)(sendbuf+OFFSET_CMDID) = uswap16(CMD_MSG);\
	sendbuf[OFFSET_STATUS] = 0;\
	time(&t);\
	p = localtime(&t);\
	printf("\r[%d/%02d/%02d %02d:%02d:%02d]: ", p->tm_year+1900,p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);\
	printf(fmt);\
	sprintf((unsigned char *)(sendbuf+OFFSET_DATA), fmt);\
	*(unsigned int*)(sendbuf+OFFSET_LENGTH) = uswap32(strlen(sendbuf+OFFSET_DATA));\
	sendbuf[OFFSET_DATA+strlen(sendbuf+OFFSET_DATA)+0] = 0x0;\
	sendbuf[OFFSET_DATA+strlen(sendbuf+OFFSET_DATA)+1] = 0xaa;\
	sendbuf[OFFSET_DATA+strlen(sendbuf+OFFSET_DATA)+2] = 0x55;\
	sendbuf[OFFSET_DATA+strlen(sendbuf+OFFSET_DATA)+3] = 0x55;\
	sendbuf[OFFSET_DATA+strlen(sendbuf+OFFSET_DATA)+3] = 0xaa;\
	if(pcmd!=NULL && pcmd!=&cmdall && pcmd->support_msg){\
		sendto(frocktest_socket_fd, sendbuf, 4+HEADER_LENGTH+strlen(sendbuf+OFFSET_DATA), 0, (struct sockaddr*)&(pcmd->fromto), sizeof(pcmd->fromto));\
	}\
}
#endif
/***************************************************
* 函数名： send_result
*   功能： 发送测试结果
*   参数：
*		struct cmd_data cmddata	测试结果数据结构
*
*   返回：
****************************************************/
int send_result(struct cmd_data *pcmd);


/***************************************************
* 函数名： create_net_server
*   功能： 初始化网络服务
*   参数：
*		struct cmd_data cmddata	测试结果数据结构
*
*   返回：
****************************************************/
int create_net_server(void);


#endif //BSP_FROCKTEST_H
/******************************* 头文件结束 ********************************/

