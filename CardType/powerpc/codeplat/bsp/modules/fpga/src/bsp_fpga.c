/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_fpga.h 
* 功能:             FPGA 寄存器读写，FPGA镜像文件加载       
* 版本:                                                                  
* 编制日期:                              
* 作者:              hjf                                        
*****************************************************************************/
/************************** 包含文件声明 ************************************/
/**************************** 共用头文件* ***********************************/
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include "fcntl.h"
#include "unistd.h"

/**************************** 私用头文件* *************************************/
#include "../../../com_inc/bsp_types.h"
#include "../inc/bsp_fpga.h"
#include "../../../com_inc/fsl_p2041_ext.h"
#include "../../hmi/inc/hmi.h"
extern pthread_mutex_t g_semFpgaWrite ;
extern HMI_WRITE_FPGA_REG   g_struWriteFpgaQueueBuf[WRITE_FPGA_BUF_NUM];
extern HMI_READ_FPGA_REG    g_struReadFpgaQueueBuf[READ_FPGA_BUF_NUM];
#define PRODUCT_CAR_STATION
extern sem_t g_ipmb_writefpga;
/******************************* 局部宏定义 ***********************************/


/*********************** 全局变量定义/初始化 **********************************/

/************************** 局部常数和类型定义 ********************************/



/*************************** 局部函数原型声明 *********************************/

/***************************函数实现 ******************************************/

/*******************************************************************************
* 函数名称: bsp_read_reg							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 正常， 其他 错误	
* 说   明: 
*******************************************************************************/
Private u16 bsp_read_16reg(u8 *u8pAddr)
{
	u16 ret;
	ret = *(u16 *)u8pAddr;
	return ret;
}

/*******************************************************************************
* 函数名称: bsp_write_reg							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 正常， 其他 错误	
* 说   明: 
*******************************************************************************/
Private void bsp_write_16reg(u16 *u16pAddr, u16 u16dwVal)
{
    *u16pAddr = u16dwVal;
}

/******************************************************************************
** 函数名称:       bsp_fpga_read_addr
** 功    能:     
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* u16RegAddr     u16        input           fpga寄存器偏移地址
* pu16Dat        u16 *      point         数据
* 返回值:  0 正常，其他错误
*******************************************************************************/
u16 bsp_fpga_read_addr(u16 u16Reg_offset)
{ 
    u16 reg_data;
	reg_data = bsp_read_16reg(g_u8fpgabase+u16Reg_offset);
	return reg_data;
}

/*****************************************************************************
*函数名称:   bsp_fpga_write_reg
*功    能:          
*函数存储类型:
*参数:
*参数名        类型        输入/输出       描述
*u16RegAddr     u16        input           fpga寄存器偏移地址
*u16Dat         u16                        设置数据
*返回值:  0 正常，其他错误
*******************************************************************************/
void bsp_fpga_write_addr(u16 u16Reg_offset,u16 u16Dat)
{
    #if 0
    UINT8 u8Index = 0;
    #ifndef PRODUCT_CAR_STATION
	bsp_write_16reg(g_u8fpgabase+u16Reg_offset,u16Dat);
    #else
	pthread_mutex_lock(&g_semFpgaWrite);
	for(u8Index = 0; u8Index < WRITE_FPGA_BUF_NUM; u8Index++)
    {
	    if(!g_struWriteFpgaQueueBuf[u8Index].u8Used)
		    break;
    }
	if(u8Index < WRITE_FPGA_BUF_NUM)
    {
	    g_struWriteFpgaQueueBuf[u8Index].u8Used = 1;
        g_struWriteFpgaQueueBuf[u8Index].dwReg = u16Reg_offset;
		g_struWriteFpgaQueueBuf[u8Index].dwVal = u16Dat;
		sem_post(&g_ipmb_writefpga);
    }
	pthread_mutex_unlock(&g_semFpgaWrite);
	#endif
	#endif
}

/******************************************************************************
** 函数名称:       bsp_fpga_read_reg
** 功    能:     
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* u16RegAddr     u16        input           fpga寄存器偏移地址
* pu16Dat        u16 *      point         数据
* 返回值:  0 正常，其他错误
*******************************************************************************/
u16 bsp_fpga_read_reg(u16 u16Reg_offset)
{ 
#ifndef PRODUCT_CAR_STATION
    u16 reg_data;
	reg_data = bsp_read_16reg(g_u8fpgabase+u16Reg_offset*2);
	return reg_data;
#else
    #if 0
	UINT8 u8Index = 0;
    u32 reg_data  = 0xdeadbeef;
	bsp_ipmb_ioctl(IPMB_CMD_BASEBANDCTL_READ_FPGA, u16Reg_offset, 0);
	for(u8Index = 0; u8Index < READ_FPGA_BUF_NUM; u8Index++)
    {
	    if( u16Reg_offset == (UINT16)g_struReadFpgaQueueBuf[u8Index].dwReg)
	    {
	        reg_data = (UINT32) g_struReadFpgaQueueBuf[u8Index].dwVal;
            break;
		}
    }	
	#else
    u16 reg_data = 0xFFFF;
	bsp_bbp_fpga_read(u16Reg_offset,&reg_data);
	return reg_data;	
	#endif
#endif
}

/*****************************************************************************
*函数名称:   bsp_fpga_write_reg
*功    能:          
*函数存储类型:
*参数:
*参数名        类型        输入/输出       描述
*u16RegAddr     u16        input           fpga寄存器偏移地址
*u16Dat         u16                        设置数据
*返回值:  0 正常，其他错误
*******************************************************************************/
void bsp_fpga_write_reg(u16 u16Reg_offset,u16 u16Dat)
{
    UINT8 u8Index = 0;
    #ifndef PRODUCT_CAR_STATION
	    bsp_write_16reg(g_u8fpgabase+u16Reg_offset,u16Dat);
    #else
	pthread_mutex_lock(&g_semFpgaWrite);
	bsp_bbp_fpga_write(u16Reg_offset,u16Dat);
	pthread_mutex_unlock(&g_semFpgaWrite);
	#endif
}

/*******************************************************************************
* 函数名称: set_nPROGRAM				
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 正常， 其他 错误	
* 说   明: //设置CPLD寄存器，eLBC_NCONFIG_n,    //PPC eLBC register wirte
*******************************************************************************/
void set_nPROGRAM(u8 setting)
{
   	if (setting)            //输出 gpio 12
	{
		bsp_cpld_write_reg(0xd,0x40); //Bit6：控制nCONFIG，1标示nCONFIG为低，0标示nCONFIG为高； 
	}
	else
	{
	   bsp_cpld_write_reg(0xd,0);
	}	
}

/*******************************************************************************
* 函数名称: set_DCLK			0xFF1000044				
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 正常， 其他 错误	
* 说   明: 设置CPLD寄存器， eLBC_DCLK,          //PPC eLBC register write
*******************************************************************************/
void set_DCLK(u8 u8setting)
{		
   	if (u8setting)
	{
		bsp_cpld_write_reg(0x27,1);   
	}
	else
	{
		bsp_cpld_write_reg(0x27,0);   
	}	
}

/******************************************************************************
* 函数名称: read_nINT							0xFF100004A	
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		
* 返回值: 0 正常， 其他 错误	
* 说   明: 读CPLD寄存器，output  reg eLBC_NSTATUS_n, //PPC eLBC register read
*******************************************************************************/
u8	read_NSTATUS_n(void)
{
	u8 u8_NSTATUS_n;
	u8_NSTATUS_n =  bsp_cpld_read_reg(0xd);

	if (u8_NSTATUS_n&0x20)
	{
		return (1);
	}
	else
	{
		return (0);
	}	
}

/*******************************************************************************
* 函数名称: read_CONF_DONE			0xFF1000048				
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 正常， 其他 错误	
* 说   明: //读取CPLD寄存器,   eLBC_DONE,        //PPC eLBC register read
*******************************************************************************/
u8	read_CONF_DONE(void)
{
	u8 u8down;
	u8down =  bsp_cpld_read_reg(0xd);

	if (u8down&0x10)
	{
		return (1);
	}
	else
	{
		return (0);
	}	
}

/*******************************************************************************
* 函数名称: fpga_download							
* 功    能:                                     
* 相关文档:                    
* 函数类型:	
* 参    数: 						     			
* 参数名称		   类型					输入/输出 		描述		

* 返回值: 0 正常， 其他 错误	
* 说   明: 
*******************************************************************************/
s32 fpga_download(void)
{
	FILE *file_fpga;
	u32 delayCount = 0;	
	u8 u8data;
	u16 u16data;
	u32 count;
	s32 i = 0;
	s32 j = 0;
    struct stat *fpga_file_stat;
	u32 fpga_size;

    stat(fpga_bin_name,fpga_file_stat);
	fpga_size = fpga_file_stat->st_size;
	
	file_fpga = fopen(fpga_bin_name,"rb");
	if (file_fpga == NULL)
	{
		printf("%s open fail!! \n",fpga_bin_name);   
		return BSP_ERROR;
	}
    rewind(file_fpga); 
     
	set_nPROGRAM(0);       /**program =0****/
	for (delayCount = 0; delayCount < 100000; delayCount++);	/* tCF2ST0 : max 800 ns */
	set_nPROGRAM(1);       /**program =1****/
    //for (delayCount = 0; delayCount < 1000000; delayCount++);
		
	while (!read_NSTATUS_n())
	{
		i++;
		if(i > 2000000)
		{
		    printf("read n_status error\n");
		    return BSP_ERROR;
		}
	}
	printf("read fpga n_status ok!\n");
	printf("download fpga image ,please wait......\n");

	set_DCLK(1);

	for (delayCount = 0; delayCount < 10000; delayCount++);
	
	for(i=0; i < fpga_size; i++)
	{
		//读取1个字节
		count = fread(&u8data, 1, 1, file_fpga); 
		for(j = 0;j < 8;j++)
		{
			if((u8data >> j)&0x1)
				u16data = 1;
			else
				u16data = 0;				
			bsp_fpga_write_reg(0,u16data);	
		}
	}

	printf("fpga image size  = %d\n",fpga_size);

	for (delayCount = 0; delayCount < 5; delayCount++)
	{
		u16data = 0xff;
		bsp_fpga_write_reg(0,u16data);	
	}

	for (delayCount = 0; delayCount < 10000; delayCount++);

	set_DCLK(0);

	if(read_CONF_DONE())
	{
	    fclose(file_fpga);
		return BSP_OK;
	}
	else
	{
	    fclose(file_fpga);
		return BSP_ERROR;
	}
}

/*******************************************************************************
* 函数名称:  bsp_boot_fpga							
* 功    能:  boot fpga
* 相关文档:                    
* 函数类型:									
* 参    数: 						     			
* 参数名称 	类型			输入/输出 		描述		
* 返回值: 
* 说   明:
*******************************************************************************/
s32 bsp_boot_fpga(void)
{
	s32 ret;
	s32 i = 0;

	printf("fpga boot start......\n");
	while(i < 3)
	{
		/* config FPGA */
		ret = fpga_download();
		if(ret < 0)
		{
			i++;
			printf("try to download fpga %d time !\n ",i);
		}
		else
		{
			printf("[BTS boot] ==> FPGA download succeed!\n");
    		return BSP_OK;
		}
	}

	printf("[BTS boot] ==> FPGA download failed!\n");
	return BSP_ERROR;
}

/************   测试 cpld fpga 寄存器读写,nandflash读写*****************/
void cpld_wr_test(void)
{
    u16 *p_fpga_data;
	u16  cpld_data;	
    cpld_data = bsp_cpld_read_reg(0xa);
    printf("cpld_data =  0x%x\n",cpld_data);

}

void fpga_wr_test(u16 reg)
{
    u16 *p_fpga_data;
	u16  fpga_data;
    bsp_fpga_write_reg(reg,0x100);
	
    fpga_data = bsp_fpga_read_reg(reg);
    printf("fpga_data =  0x%x\n",fpga_data);
}

int writeNvramToFlash(char *srcData, UINT32 len)
{
     FILE *fdhead;
	 int nBytesw, wnum, ncount = 0;
	 char * ptr = srcData;
	 char ptr_open[100]={0};
	 int i;
 
	/*打开文件1*/
	if((fdhead = fopen ("/mnt/nand_test", "wb+")) == NULL)
	{
	    return -1;
	}     
	rewind(fdhead); 
	      
	while(ncount<len)
	{   
	    if(len-ncount>4096)
	        wnum = 4096;
	    else
	        wnum = len-ncount;
	    nBytesw = fwrite(ptr, 1, wnum, fdhead); 	
	    if(nBytesw==0)
	        break;
	    fflush(fdhead);
	    ncount += nBytesw;  
	    ptr += nBytesw; 
	}    

	fclose(fdhead);	  

    if((fdhead = fopen ("/mnt/nand_test", "rb")) == NULL)
	{
	    return -1;
	}
    rewind(fdhead);
	
	for (i=0;i<100;i++)
	{
		nBytesw = fread(ptr_open, 1, 1, fdhead);
		printf("%d", ptr_open[0]);
	}
	fclose(fdhead);	   
    
	return 0;
}

void nand_write_read_test()
{
	FILE *fp;
	int i;
    char name[]= "/mnt/nand_test";
	char s[100]={0};

	for (i=0;i<100;i++)
	    s[i] = i;

	writeNvramToFlash(s,100);
	
	#if 0
	if((fp=fopen(name,"w"))==NULL)
	{
		printf("cannot open file strike any key exit!");
		return -1;
	}	
	
	for(i=0;i<100;i++)
	{   
	    s[i] = i;
		fwrite(&s[i],1,1,fp);
	}	

	#endif
}

/******************************* 源文件结束 ********************************/

