/********************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
*********************************************************************************
* 源文件名:           bbu_config.h
* 功能:
* 版本:
* 编制日期:
* 作者:
*********************************************************************************/

/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

/**************************** 私用头文件* **********************************/
#include "../../../com_inc/bsp_types.h"
#include "../../../com_inc/bsp_dsp_ext.h"
#include "../../../com_inc/bsp_epld_ext.h"
#include "../../../com_inc/bsp_ethsw_ext.h"
#include "../../../com_inc/bsp_shmem_ext.h"
#include "../../../com_inc/bsp_fpga_ext.h"
#include "../../../com_inc/bsp_gpio_ext.h"
#include "../../../com_inc/bsp_gps_ext.h"
#include "../../../com_inc/bsp_pll_ext.h"
#include "../../../com_inc/bsp_usdpaa_ext.h"
#include "../../../com_inc/bsp_ushell_ext.h"
#include "../../../com_inc/bsp_i2c_ext.h"
#include "../../../com_inc/bsp_conkers_ext.h"

#include "../inc/fsl_p2041.h"
#include "../inc/bsp_comapi.h"
#include "../../../modules/bbp_mcta/bsp_msg_proc.h"
#include "../../../modules/hmi/inc/hmi.h"

/******************************* 局部宏定义 *********************************/
#define SCRIPT_FILENAME		"/mnt/btsa/cfg.txt"
#define MCT_SLAVE	0
#define MCT_MASTER	1


/*********************** 全局变量定义/初始化 **************************/
u8 *g_u8ccsbar = 0;
u8 *g_u8fpgabase = 0;
u8 *g_u8epldbase = 0;
u8 *g_u8tmp = 0;
u8 *g_u8tmp1 = 0;
ULONG g_ulIsBspInit = 0;
u32 g_subboard_reset_over =0;


unsigned char g_eth0auMacAddr[6] = {0};
unsigned char g_eth1auMacAddr[6] = {0};
unsigned char g_eth2auMacAddr[6] = {0};
unsigned char g_eth3auMacAddr[6] = {0};

/************************** 局部常数和类型定义 ************************/

void bsp_sys_hw_init(void);

extern UINT32 BspIntcInit(void);
extern UINT32 InteruptInitExt(void);
extern void BspExtIrqInstall(void);
extern void BspEnableInterrupt(void);
extern UINT32 bsp_sharemem_init();
extern void bsp_sys_msdelay(ULONG dwTimeOut);
extern void bsp_sys_usdelay(ULONG dwTimeOut);
extern void bsp_ushell_init(UCHAR * ucpsymbname, UCHAR * ucpboardname);

extern void  *BspShmemVirtMalloc(unsigned long  align, unsigned long  size, const char *name, unsigned long index,int *pdwphyaddr);
extern unsigned long BspAlignSize(unsigned long  align, unsigned long  size);
/*************************** 局部函数原型声明 **************************/

/************************************ 函数实现 ************************* ****/
/********************************************************************************
* 函数名称: bsp_p2041_init
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称		   类型					输入/输出 		描述

* 返回值: 0 表示成功；其它值表示失败。
* 说   明:
*********************************************************************************/
UINT32  BspMallocAddr(int len)
{
    unsigned long tmp;
    unsigned long  dwphybase = 0;
    if (len<0)
        return 0;
    BspShmemVirtMalloc(len, BspAlignSize(len, len), "test1", 0,(int *)&dwphybase);
    return dwphybase;
}

s32 bsp_set_system_time(void)
{
    s8  s8ptemp[100];
    u8 pbuffer[20];	

#if defined(__CPU_LTE_CENTERSTATION__) || defined(__CPU_LTE_CARDTYPE__)
    if(2 == g_GNSS_flag){
    	bd_NAV_TimeBD(gps_GMT_Offset,gLeapSecond);
    }else if (0 == g_GNSS_flag){
    	gps_NAV_TimeGPS(gps_GMT_Offset,gLeapSecond);
    }
    pbuffer[0] = gUbloxGpsAllData.Second;//&0x7f;  /******s********/
    pbuffer[1] = gUbloxGpsAllData.Minute;//&0x7f;  /******min*******/
    pbuffer[2] = gUbloxGpsAllData.Hour;//&0x3f;  /*********hour*******/
    pbuffer[3] = gUbloxGpsAllData.Day;//&0x3f; /*******day********/
    pbuffer[4] = 5;//&0x7; /****week day*****/
    pbuffer[5] = gUbloxGpsAllData.Month;//&0x1f; /*******month*******/
    pbuffer[6] = (gUbloxGpsAllData.Year-2000);//&0xff; /**year***/
#else	
	if(BSP_OK != bsp_get_rtc(pbuffer))
	{
		printf("get time is wrong!\n");
		return BSP_ERROR;
	}
#endif	
	sprintf(s8ptemp,"date -s 20%d.%d.%d-%d:%d:%d",pbuffer[6],pbuffer[5],pbuffer[3],pbuffer[2],pbuffer[1],pbuffer[0]);
	//printf("%s",s8ptemp);
	if(system((char *)s8ptemp) < 0)
	{
		printf("system call wrong!\n");
		return BSP_ERROR;
	}

	return BSP_OK;
}


s32 bsp_set_mct_system_time(void)
{
    char pbuffer[7];
    s8  s8ptemp[100];
    gps_NAV_TimeUTC(gps_GMT_Offset);
    pbuffer[0] = gUbloxGpsAllData.Second;//&0x7f;  /******s********/
    pbuffer[1] = gUbloxGpsAllData.Minute;//&0x7f;  /******min*******/
    pbuffer[2] = gUbloxGpsAllData.Hour;//&0x3f;  /*********hour*******/
    pbuffer[3] = gUbloxGpsAllData.Day;//&0x3f; /*******day********/
    pbuffer[4] = 5;//&0x7; /****week day*****/
    pbuffer[5] = gUbloxGpsAllData.Month;//&0x1f; /*******month*******/
    pbuffer[6] = (gUbloxGpsAllData.Year-2000);//&0xff; /**year***/
    sprintf(s8ptemp,"date -s 20%d.%d.%d-%d:%d:%d",pbuffer[6],pbuffer[5],pbuffer[3],pbuffer[2],pbuffer[1],pbuffer[0]);
    if(system((char *)s8ptemp) < 0)
    {
        printf("system call wrong!\n");
        return BSP_ERROR;
    }
}
void bsp_reset_all_chip(void)
{
    unsigned char u8tmp=0;
    u8tmp = *(unsigned char *)(g_u8epldbase+0xa);
    u8tmp |= ~(0x3);
    *(unsigned char *)(g_u8epldbase+0xa) = u8tmp;
    u8tmp &= 0x3;
    usleep(100);
    *(unsigned char *)(g_u8epldbase+0xa) = u8tmp;

}

#define CPLD_SLOT_ID    (52)
UINT8 bsp_get_slot_id(void)
{
    return *(unsigned char *)(g_u8epldbase+CPLD_SLOT_ID);
}

s32 bsp_check_dir(char *pfilename)
{
    FILE *fp = NULL;
    fp = fopen(pfilename,"r");
    if (NULL == fp)
    {
        return 0;
    }
    else
    {
        fclose(fp);
        return 1;
    }
}

void bsp_create_dir(char *dirname)
{
    char cmd[128];

    sprintf(cmd, "mkdir -p %s", dirname);
    system(cmd);
}


#define DSP_DUMP_LEN  (34*1024)

#define FPGA_DUMP_START             (256)
#define FPGA_DUMP_END               (383)
#define FPGA_START_FLAG             (34)
#define FPGA_DUMP_DDR_BASE_LOW      (64)
#define FPGA_DUMP_DDR_BASE_HIGH     (63)
#define FPGA_DUMP_DATA_READY_FLAGA  (72)
#define DSP_SRIO_BASE_ADDR          (0xbf7fff00)

#define DSP_SRIO_STATE_REG		52
#define DSP_DEST_DEVICE_ID_REG  66
#define FPGA_WRITE_LEN_REG      63
#define FPGA_DEVICE_ID_REG      67

#define FPGA_DEVICE_ID      0x5555
#define FPGA_WRITE_LEN      0xff

#define FILE_SAVE_LEVEL		3
#define DSP1_CORE_NUM		4
#define DSP2_CORE_NUM		4
#define DSP3_CORE_NUM		4
#define DSP4_CORE_NUM		8
#define DSP_CORE_NUM_MAX	(DSP1_CORE_NUM + DSP2_CORE_NUM + DSP3_CORE_NUM + DSP4_CORE_NUM)

/********************************************************************************
* 函数名称: is_empty_dir
* 功    能: 判断目录是否为空目录
* 参    数: char *path 路径名称
* 返回值: 0 不为空；1 为空；-1 失败
* 说   明:
*********************************************************************************/
int is_empty_dir(char *path)
{
    DIR *dir = NULL;
    int num = 0;
    if(path==NULL)
    {
        return -1;
    }
    dir = opendir(path);
    if(dir==NULL)
    {
        return -1;
    }
    while(readdir(dir)!=NULL)
    {
        num++;
    }
    closedir(dir);
    return (num>2)?0:1;
}

void old_dump_file_move(void)
{
    char name[50];
    int i;

    /* Check if there is the derictory, if no create it */
    for (i = 0; i < FILE_SAVE_LEVEL; i++)
    {
        sprintf(name, "/mnt/btsa/dspblk%d", i);

        if (bsp_check_dir(name) == 0)
            bsp_create_dir(name);
    }

    system("rm /mnt/btsa/dspblk2/* -rf 2> /dev/null");
    system("mv /mnt/btsa/dspblk1/* /mnt/btsa/dspblk2/ 2> /dev/null");
    system("mv /mnt/btsa/dspblk0/* /mnt/btsa/dspblk1/ 2> /dev/null");
}

void calc_id(int *dev_id, int *core_id, int no)
{
    if (no < DSP1_CORE_NUM)
    {
		*dev_id = 1;
        *core_id = no;
    }
    else if (no < DSP2_CORE_NUM + DSP1_CORE_NUM)
    {
		*dev_id = 2;
        *core_id = no - DSP1_CORE_NUM;
    }
    else if (no < DSP3_CORE_NUM + DSP2_CORE_NUM + DSP1_CORE_NUM)
    {
		*dev_id = 3;
        *core_id = no - DSP1_CORE_NUM - DSP2_CORE_NUM;
    }
    else if (no < DSP4_CORE_NUM + DSP3_CORE_NUM + DSP2_CORE_NUM + DSP1_CORE_NUM)
    {
		*dev_id = 4;
        *core_id = no - DSP1_CORE_NUM - DSP2_CORE_NUM - DSP3_CORE_NUM;
    }
}
int bsp_dsp_black_box(u8 slotid, u32 dsp_mask, int core_sum)
{
    int i, dev_id, core_id;
    int timeout = 100000;
    u32 mask = ~dsp_mask & ((1 << DSP_CORE_NUM_MAX) - 1);

    if (core_sum == 13)
        mask =  ~dsp_mask & ((1 << 13) - 1);

    old_dump_file_move();

    if (timeout == 0)
        return -1;
    for (i = 0; i < DSP_CORE_NUM_MAX; i++)
    {
        if ((1 << i) & mask)
        {
            char name[128]= {0};
            time_t timer = time(NULL);
            struct tm *tm;
            calc_id(&dev_id, &core_id, i);
            //printf("[%s]:i=%d, dev_id=%d, core_id=%d\r\n", __func__, i, dev_id, core_id);
            tm = localtime(&timer);
            sprintf(name,"/mnt/btsa/dspblk0/fpga_dump_D%d_Core%d_%d-%02d-%02d-%02d-%02d-%02d.bin", dev_id, core_id,
                    tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            if(BSP_OK == bsp_get_dsp_dump(slotid, dev_id, core_id, name))
            {
                printf("create dsp dump ok![%s]\r\n", name);
            }
            else
            {
                printf("create dsp dump failed![%s]\r\n", name);
            }
        }
    }

    for (i = 0; i < FILE_SAVE_LEVEL; i++)
    {
        char name[50] = "";
        sprintf(name, "/mnt/btsa/dspblk%d", i);
        if(is_empty_dir(name))
        {
            remove(name);
        }
    }

    return 0;
}

void bsp_sys_hw_init(void)
{
    /*I2C初始化*/
    bsp_i2c_init();
    /*SPI控制器初始化*/
    bsp_spi_init();
    bsp_ethsw_spi_init();
}

u16 data_afc[2]= {0};
u8  fpga_afc_flag = 0;

extern u16 bsp_fpga_read_reg(u16 u16Reg_offset);
s32 fpga_set_dac(void)
{
    u16 data=0;
    data = bsp_fpga_read_reg(300);
    //printf("dac data = 0x%x\n",data);
    spi_dac_write(0,data);
}

s32 cpld_phase_cmd(void)
{
    u8 u8regvalue=0;
    s32 s32i;

    u8regvalue = bsp_cpld_read_reg(0xf);
    bsp_cpld_write_reg(0xf,u8regvalue|(0x20));
    sleep(1);
    bsp_cpld_write_reg(0xf,u8regvalue&(~0x20));
    printf("cpld_phase_cmd\n");
    return BSP_OK;
}

s32 fpga_phase_cmd(void)
{
    u16 data=0;
    bsp_fpga_write_reg(303,1);
    bsp_fpga_write_reg(303,0);
    printf("fpga_phase_cmd\n");
}

void reset_fpga_srio(void)
{
    bsp_fpga_write_reg(107,15);
    bsp_fpga_write_reg(107,0);
}

void reboot_aif_dsp_fpga()
{
    //interpro_send_reset_notify(5);
    //bsp_reset_all_dsp();
    reset_fpga_srio();
    bsp_boot_all_dsp();
}
/*
s32 fpga_set_afc(void)
{
    u16 data=0;
	data_afc[0] =data_afc[1];
	data = bsp_fpga_read_reg(301);
	if(((data>>8)&0x1)&&((data&0xff)<3))
	{
		if(fpga_afc_flag==0)
		{
		    //printf("reg 301 data= 0x%x\n",data);
	    	cpld_phase_cmd();
			fpga_phase_cmd();
			reboot_aif_dsp_fpga();
			sleep(2);
			data_afc[1] = bsp_fpga_read_reg(302);
			fpga_afc_flag=1;
			printf("cmd in frist %d\n",fpga_afc_flag);
			return BSP_OK;
		}
		data_afc[1] = bsp_fpga_read_reg(302);

		if(data_afc[1]!=data_afc[0])
		{
		    printf("the last 302 reg = 0x%x,the 302 reg=%d\n",data_afc[0],data_afc[1]);
			cpld_phase_cmd();
			fpga_phase_cmd();
			reboot_aif_dsp_fpga();
			sleep(2);
			data_afc[1] = bsp_fpga_read_reg(302);
		}
	}
	return BSP_OK;
}

*/
s32 fpga_set_afc(void)
{
    u16 data=0;
    data_afc[0] =data_afc[1];
    data = bsp_fpga_read_reg(301);
    if(((data>>8)&0x1)&&((data&0xff)<3))
    {
        if(fpga_afc_flag==0)
        {
            //printf("reg 301 data= 0x%x\n",data);
            cpld_phase_cmd();
            fpga_phase_cmd();
            reboot_aif_dsp_fpga();
            sleep(2);
            data_afc[1] = bsp_fpga_read_reg(302);
            fpga_afc_flag=1;
            printf("cmd in frist %d\n",fpga_afc_flag);
            return BSP_OK;
        }
        data_afc[1] = bsp_fpga_read_reg(302);

        if(data_afc[1]!=data_afc[0])
        {
            printf("the last 302 reg = 0x%x,the 302 reg=%d\n",data_afc[0],data_afc[1]);
            cpld_phase_cmd();
            fpga_phase_cmd();
            reboot_aif_dsp_fpga();
            sleep(2);
            data_afc[1] = bsp_fpga_read_reg(302);
        }
    }
    return BSP_OK;
}

void *fpga_dac_thread()
{
    bsp_print_reg_info(__func__, __FILE__, __LINE__);
    while(1)
    {
        sleep(1);
        fpga_set_dac();
    }
}

int fpga_dac_thread_init(void)
{
    pthread_t fpga_dac_tid;
    pthread_attr_t  attr;
    int res=0;
    struct sched_param parm;

    printf("start the fpga set dac thread!\n");

    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 27;
    pthread_attr_setschedparam(&attr, &parm);

    res = pthread_create(&fpga_dac_tid, &attr,fpga_dac_thread,NULL);
    pthread_attr_destroy(&attr);
    if (ERROR == res)
    {
        perror("dac_thread_init error!\r\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

void *fpga_afc_thread()
{
    bsp_print_reg_info(__func__, __FILE__, __LINE__);
    while(1)
    {
        sleep(1);
        fpga_set_afc();
    }
}

int fpga_afc_thread_init(void)
{
    pthread_t fpga_afc_tid;
    u16 data=0;
    int res=0;
    pthread_attr_t  attr;
    struct sched_param parm;
    printf("start the fpga_afc_thread!\n");
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 25;
    pthread_attr_setschedparam(&attr, &parm);

    res = pthread_create(&fpga_afc_tid, &attr,fpga_afc_thread, NULL);
    pthread_attr_destroy(&attr);
    if (ERROR == res)
    {
        perror("afc_thread_init error!\r\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

void reset_fpga_ir(u16 ir_no)
{
    u16 ir_data =0;
    ir_data = bsp_fpga_read_reg(0x1c);
    bsp_fpga_write_reg(0x1c,ir_data|(0x1<<ir_no));
    bsp_fpga_write_reg(0x1c,ir_data&(~(0x1<<ir_no)));
}

void fpga_ir_alarm(u16 ir_syn_state, u16 ir_no)
{
    if(!(ir_syn_state&(1<<ir_no)))
    {
        reset_fpga_ir(ir_no);
    }
}

void fpga_alarm(void)
{
    int i = 0;
    u16 syn_state =0;
    u16 LOP_state =0;
    u16 PLL_state =0;
    int index_syn_state = 0;
    int index_ir_lop = 0;
    bsp_fpga_write_reg(11,0xffff);
    bsp_fpga_write_reg(10,0);
    usleep(10*1000);
    syn_state = bsp_fpga_read_reg(11);
    LOP_state = bsp_fpga_read_reg(10);
    for(index_ir_lop = 0; index_ir_lop< 3; index_ir_lop++)
    {
        if(LOP_state&(0x1<<index_ir_lop))
        {

        }
        else
        {
            fpga_ir_alarm(syn_state,index_ir_lop);
        }
    }
}

void *fpga_ir_reset_thread(void *arg)
{
    bsp_print_reg_info(__func__, __FILE__, __LINE__);
    while(1)
    {
        sleep(1);
        fpga_alarm();
    }
}
int fpga_ir_reset_task(void)
{
    u16 data=0;
    int res=0;
    pthread_t tid;
    pthread_attr_t  attr;
    struct sched_param parm;
    printf("start the fpga_ir_reset_thread!\n");
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 15;
    pthread_attr_setschedparam(&attr, &parm);
    res = pthread_create(&tid, &attr,fpga_ir_reset_thread, NULL);
    pthread_attr_destroy(&attr);
    if (ERROR == res)
    {
        perror("afc_thread_init error!\r\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

u8 *g_u8ShareMem;
unsigned long g_sharephyaddr;
unsigned int  g_gspflag=0;
#if 0
#else
void spi_test()
{

    printf("read switch model id....\n");
    u16 swtbuf=0;
    char pstr[4];
    int err;
    err = ethsw_read_reg(0x02, 0x30, (u8*)&swtbuf, 2);
    if(err != 0)
        printf("ethsw read reg error, err = %d.\n",err);
    swtbuf = ((swtbuf>>8)&0x00ff) | ((swtbuf<<8)&0xff00);
    printf("switch model id is 0x%x....\n",swtbuf);

    u32 u32Buf[2] = {0};
    err = ethsw_read_reg(0x04, 0x04, (u8*)u32Buf, 6);
    if(err != 0)
        printf("ethsw read reg error, err = %d.\n",err);
    //swtbuf = ((swtbuf>>8)&0x00ff) | ((swtbuf<<8)&0xff00);
    printf("before BPDU Multicast is 0x%x...0x%x.\n",u32Buf[0], u32Buf[1]);

    u8 u8Wbuf[6]= {0};
    u8Wbuf[0] = 0x12;
    u8Wbuf[1] = 0x34;
    u8Wbuf[2] = 0x56;
    u8Wbuf[3] = 0x78;
    u8Wbuf[4] = 0x90;
    u8Wbuf[5] = 0x12;
    ethsw_write_reg(0x04, 0x04, u8Wbuf, 6);
    err = ethsw_read_reg(0x04, 0x04, (u8*)u32Buf, 6);
    if(err != 0)
        printf("ethsw read reg error, err = %d.\n",err);
    //swtbuf = ((swtbuf>>8)&0x00ff) | ((swtbuf<<8)&0xff00);
    printf("after BPDU Multicast is 0x%x...0x%x.\n",u32Buf[0], u32Buf[1]);
}
void i2c_test()
{
    power_test();
    temp_test();
    printf("read and write eeprom.....\n");
    eeprom_write_test();
    char str[128]= {0};
    int i=0;
    bsp_read_eeprom(str, 128, 0);
    printf("read data:\n");
    for(; i<128; i++)
        printf("%x ",str[i]);
    printf("\n");

    memset(str, 0, 128);
    for(i=0; i<128; i++)
        str[i]=i;
    printf("write data:\n");
    bsp_write_eeprom((u8*)str, sizeof(str), 0);

    memset(str, 0, 128);
    bsp_read_eeprom(str, 128, 0);
    printf("read data:\n");
    for(i=0; i<128; i++)
        printf("%x ",str[i]);
    printf("\n");
}
extern void bsp_cputodsp_callback(unsigned char *pbuf,int len);
extern void bsp_cputodsptest_callback(unsigned char *pbuf,int len);

#define HMI_BUFFER_ENABLE_CHIP1 (47)
#define HMI_BUFFER_ENABLE_CHIP2 (49)
#define HMI_BUFFER_PIN_HIGH     (0x1)
#define HMI_BUFFER_PIN_LOW      (0x0)


/********************************************************************************
* 函数名称: bsp_open_hmi_buffer
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称		   类型					输入/输出 		描述

* 返回值: 0 表示成功；其它值表示失败。
* 说   明:
* 作者:刘刚
*日期:2015-12-15
*********************************************************************************/
UINT8 bsp_open_hmi_buffer(void)
{
    bsp_cpld_write_reg(HMI_BUFFER_ENABLE_CHIP1,HMI_BUFFER_PIN_HIGH);
    bsp_cpld_write_reg(HMI_BUFFER_ENABLE_CHIP2,HMI_BUFFER_PIN_HIGH);
    return TRUE;
}

/********************************************************************************
* 函数名称: bsp_close_hmi_buffer
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称		   类型					输入/输出 		描述

* 返回值: 0 表示成功；其它值表示失败。
* 说   明:
* 作者:刘刚
*日期:2015-12-15
*********************************************************************************/
UINT8 bsp_close_hmi_buffer(void)
{
    bsp_cpld_write_reg(HMI_BUFFER_ENABLE_CHIP1,HMI_BUFFER_PIN_LOW);
    bsp_cpld_write_reg(HMI_BUFFER_ENABLE_CHIP2,HMI_BUFFER_PIN_LOW);
    return TRUE;
}

/********************************************************************************
* 函数名称: bsp_reboot_system
* 功    能:
* 相关文档:
* 函数类型:
* 参    数:
* 参数名称		   类型					输入/输出 		描述

* 返回值: 0 表示成功；其它值表示失败。
* 说   明:
* 作者:刘刚
*日期:2015-12-15
*********************************************************************************/
void bsp_reboot_system(void)
{
    sleep(10);

    printf("now close hmi buffer!\r\n");
    bsp_close_hmi_buffer();

    printf("now reboot system!\r\n");
    system("reboot");
}
void run_cfg_script(void)
{
    if(access(SCRIPT_FILENAME, F_OK)==0)
    {
        int ret = 0;
        char *cmd_dos2unix = "dos2unix " SCRIPT_FILENAME;
        char *cmd_chmod = "chmod 777 " SCRIPT_FILENAME;
        system(cmd_dos2unix);
        system(cmd_chmod);
        system(SCRIPT_FILENAME);
    }
}

int bsp_get_bbp_net_status(void)
{
    UINT32 ret = system("ping 192.168.1.10 -w 1");
    if (WEXITSTATUS(ret) == 0)
    {
        printf("net ok!\r\n");
        return BSP_OK;
    }
    else
    {
        printf("net error!\r\n");
        return BSP_ERROR;
    }
}

s32 bsp_p2041_init(void)
{
    u32 fd;
    UINT32 dwtrytime=0;
    unsigned char auMacAddr[6];
#if 1
    if (system("echo 1 > /proc/sys/net/ipv4/conf/eth0/arp_ignore;\
			echo 1 > /proc/sys/net/ipv4/conf/eth1/arp_ignore;\
			echo 1 > /proc/sys/net/ipv4/conf/eth2/arp_ignore") < 0)
        perror("shutup eth0&eth1&eth2 arp_proxy");

    if (system("/sbin/ifconfig eth0 down; \
		/sbin/ifconfig eth1 down; \
		/sbin/ifconfig eth0 hw ether 00:A0:1E:01:01:01 mtu 9000; \
		/sbin/ifconfig eth1 hw ether 00:A0:1E:01:01:02 mtu 9000; \
		/sbin/ifconfig eth0 10.0.0.1 netmask 255.255.255.0 up; \
		/sbin/ifconfig eth1 30.0.0.1 netmask 255.255.0.0 up") < 0)
        perror("ERROR: Set eth0 eth1 failed ");
    //if(system("ipaddr add 192.168.1.13/16 broadcast + dev eth1") < 0)
    //perror("add eth1 192.168.1.13 failed");

#endif
    //do_boot_flag();
    run_cfg_script();
    printf("open mem device...\n");

    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (BSP_ERROR == fd)
    {
        bsp_dbg("can not open mem!\n");
        return BSP_ERROR;
    }
#ifdef CPU_FSL_SYS_BIT32_WIDTH
    g_u8ccsbar = mmap((void *)0,P2041_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,P2041_BASE);
#else
    g_u8ccsbar = mmap64((void *)0,P2041_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,P2041_BASE);
#endif

    if (BSP_ERROR == (s32)g_u8ccsbar)
    {
        return BSP_ERROR;
    }

    bsp_dbg("g_u8ccsbar vir addr = 0x%x\n",(u32)g_u8ccsbar);

#ifdef CPU_FSL_SYS_BIT32_WIDTH
    g_u8epldbase = mmap((void *)0,EPLD_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,EPLD_BASE);
#else
    g_u8epldbase = mmap64((void *)0,EPLD_LEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,EPLD_BASE);
#endif

    if (BSP_ERROR == (s32)g_u8epldbase)
    {
        return BSP_ERROR;
    }
    if(bsp_get_slot_id() == IPMB_SLOT0)
    {
        if(system("ipaddr add 192.168.1.13/16 broadcast + dev eth1") < 0)
            perror("add eth1 192.168.1.13 failed");
    }
    else if(bsp_get_slot_id() == IPMB_SLOT1)
    {
        if (system("/sbin/ifconfig eth1 down; \
			/sbin/ifconfig eth1 hw ether 00:A0:1E:01:02:02 mtu 9000; \
			/sbin/ifconfig eth1 30.0.0.1 netmask 255.255.0.0 up") < 0)
            perror("ERROR: Set eth1 failed ");

        if(system("ipaddr add 192.168.2.13/16 broadcast + dev eth1") < 0)
            perror("add eth1 192.168.2.13 failed");
    }
    bsp_get_resetcause();
    //bsp_cpld_write_reg(254,0xa5);
    bsp_led_init();
    //bsp_reset_all_chip();
    bsp_start_watchdog();

    printf("init share memory...\n");
    bsp_sharemem_init();
    printf("start hardware init...\n");
    bsp_sys_hw_init();

    printf("start config share memroy...\n");
    bsp_config_sys_shm_table();
    /*dpaa初始化*/

    ip_frag_init(10);
    printf("init dpaa ...\n");
    if(0 != BspDpaaInit())
    {
        printf("BspUsDpaaInit failed\n");
        return 0;
    }
    BspBmanInitExample();
    system("echo 3 > /proc/sys/vm/drop_caches");
    system("echo 100000000 > /proc/sys/net/ipv4/icmp_ratelimit");
    system("arp -s 192.168.2.100 00:01:02:00:02:51");
    system("arp -s 192.168.3.100 00:01:02:00:03:51");
    system("arp -s 192.168.4.100 00:01:02:00:04:51");
    system("arp -s 192.168.5.100 00:01:02:00:05:51");
    system("arp -s 192.168.6.100 00:01:02:00:06:51");
    system("arp -s 192.168.7.100 00:01:02:00:07:51");
    //system("ls -r -l /mnt/btsa/");
    BspGetDataNetMac(auMacAddr);
    if (BSP_OK != bsp_get_netif_mac_addr((CHAR*)"eth0",g_eth0auMacAddr))
    {
        printf("MAC Address eth0 Get Error\n");
        //return BSP_ERROR;
    }
    if (BSP_OK != bsp_get_netif_mac_addr((CHAR*)"eth1",g_eth1auMacAddr))
    {
        printf("MAC Address eth1 Get Error\n");
        //return BSP_ERROR;
    }
    if (BSP_OK != bsp_get_netif_mac_addr((CHAR*)"eth2",g_eth2auMacAddr))
    {
        printf("MAC Address eth2 Get Error\n");
        //return BSP_ERROR;
    }
    if (BSP_OK != bsp_get_netif_mac_addr((CHAR*)"eth3",g_eth3auMacAddr))
    {
        printf("MAC Address eth3 Get Error\n");
        //return BSP_ERROR;
    }
    phy_connect_adapt();

    ethsw_set_jumbo();
    ethsw_arl_set();
    bsp_ethsw_set_port_sgmii_mode(14);

    //bsp_config_vport();
    ethsw_init_vport();
    //ethsw_port_disable(6);

    BspConfigMaxTransConfig();
    printf("init sec module...\n");
    bsp_sec_init();
    printf("init gps and afc module...\n");
    bsp_gps_init();
    bsp_create_gps_thread();
    printf("init time handle task...\n");
    init_time();
    bsp_afc_init();
    printf("init ipmb ...\n");
    bsp_ipmb_init();
    bsp_set_i2c_slave(1);
    bsp_set_i2c_slave(2);
    printf("init mpic interrupt...\n");
    bsp_init_syn_src_switch_info();
#if 1
    BspIntcInit();
    InteruptInitExt();
    BspExtIrqInstall();
    BspEnableInterrupt();
#endif
    //bsp_set_self_available();
    bsp_hardware_check_init();	//硬件设备健康检查
    /*if(BSP_OK != bsp_MCT_available_detection())
    {
    	bsp_set_self_unavailable();
    }*/
    bsp_master_slave_competition();	//主备竞争

    printf("open hmi pin!!!!\r\n");
    bsp_open_hmi_buffer();

    bsp_hmi_config_table_init();

    /*等待从板启动并请求版本*/
    //spi_test();
    //i2c_test();
    printf("init bsp_init_tftp_server!\n");
    init_tftp_server_thread("./");
    printf("init init_msg_proc_thread!\n");
    init_msg_proc_thread();
    init_boardtest();
    BspCpuDspRecvCallBack(bsp_cputodsp_callback);
    BspCpuDspTestRecvCallBack(bsp_cputodsptest_callback);
    init_rrutest_thread();
    if(MCT_MASTER == bsp_get_self_MS_state())	//主备复位从板，备板不做复位
    {
#if 0
        if(BSP_OK!=bsp_hmi_start_send_message())
        {
            if(BSP_OK!=bsp_hmi_start_send_message())
                printf("hmi send start message fail!\n");
        }
#endif
        printf("reboot subboard!\r\n");
        memset(boards, 0, sizeof(boards));
        for (dwtrytime=0; dwtrytime<3; dwtrytime++)
        {
            int bid = 0;
            int is_reset_failed = 0;
            for(bid=2; bid<11; bid++)
            {
                if((boards[bid].mcu_status&0xF0)==0)
                {
                    printf("reboot bbp board[%d]!\r\n",bid);
                    //memset(&boards[bid], 0, sizeof(board_info));
                    bsp_reset_subboard(bid);
                    usleep(100*1000);
                }
            }
            sleep(1);
            for(bid=2; bid<11; bid++)
            {
                if((boards[bid].mcu_status&0xF0)==0)
                {
                    is_reset_failed = 1;
                    break;
                }
            }

            if(is_reset_failed == 0)
            {
                break;
            }
        }
        g_subboard_reset_over=1;
        printf("subboard reset over!\r\n");
    }
    bsp_led_run_blink(1000);
    return TRUE;
}
#endif

s32 bsp_subboard_is_reseted(void)
{
    if(g_subboard_reset_over == 1)
        return BSP_OK;
    return BSP_ERROR;
}
/********************************************************************************
* 函数名称: bsp_board_init
* 功    能:
* 相关文档:
* 函数类型:	int
* 参    数:
* 参数名称		   类型					输入/输出 		描述

* 返回值: 0 表示成功；其它值表示失败。
* 说   明:
*********************************************************************************/
s32 bsp_board_init(void)
{
    s32 s32ret;
    //bsp_ushell_init("XSHELL","P2041");
    s32ret =  bsp_p2041_init();
    if(BSP_ERROR == s32ret)
    {
        bsp_dbg("bsp_p2041_init  error !\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

void bsp_test_demo(void)
{
    u32 fd;
    u8 *g_qmanbar = 0;
    u8 *g_bmanbar = 0;
    fd = open("/dev/usdpaa",O_RDWR|O_SYNC);
    if (BSP_ERROR == fd)
    {
        bsp_dbg("can not open mem!\n");
        return BSP_ERROR;
    }
    g_qmanbar = mmap64((void *)0,0x100,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0xff4200000);
    d4(g_qmanbar,0x100);
}

void bsp_board_test(void)
{
#if 0
    ULONG ulItemNum = 0;
    ULONG bExitFlag = 0;
    printf(" main Test entry!!\n");
    if (!g_ulIsBspInit)
    {
        bsp_board_init();
        g_ulIsBspInit = 1;
    }
    while (TRUE != bExitFlag)
    {
        printf("\n");
        printf("1  : FPGA下载测试\n");
        printf("100: 退出\n");
        printf("\n请选择:--> ");
        ulItemNum  = atoi((char *)readline(""));
        printf("Select is : %d\n",ulItemNum);
        switch (ulItemNum)
        {
        /* FPGA下载测试 */
        case 1:
            printf("1  : FPGA下载测试\n");
            break;
        case 100:
            bExitFlag = TRUE;
            printf("退出\n\n");
            break;
        default:
            printf("\n\n输入有误，请重输入!\n");
            break;
        }/** end of switch(ulItemNum) */
    }
#endif
}

void print_compile_time()
{
    printf("\n---------------compile  info-----------------");
    printf("\n---    date:%s,    time :%s---", __DATE__, __TIME__);
    printf("\n---------------compile  end -----------------\n");
}

#ifdef BSP_DEBUG
/********************************************************************************
* 函数名称: bsp_p2041_init
* 功    能:
* 相关文档:
* 函数类型:	int
* 参    数:
* 参数名称		   类型					输入/输出 		描述

* 返回值: 0 表示成功；其它值表示失败。
* 说   明:
********************************************************************************/
extern int g_sendemacflag;
int main(int argc, char *argv[])
{
    print_compile_time();
    bsp_p2041_init();
    //init_tftp_server_thread("./");
    //init_msg_proc_thread();
    //init_boardtest();
    BspRecvInit();
    g_sendemacflag = 1;
    bsp_ushell_init("bsp_app","p2041");
    return BSP_OK;
}
#endif
/******************************* 源文件结束 ********************************/

