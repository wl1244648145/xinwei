/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_dsp.c 
* 功能:             DSP 镜像文件加载     
* 版本:                                                                  
* 编制日期:                              
* 作者:                    hjf                         
*******************************************************************************/
/************************** 包含文件声明 **********************************/
/**************************** 共用头文件* **********************************/
#include <stdio.h>
#include <asm/types.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <linux/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/ioctl.h>  
#include <sys/stat.h>
#include <netinet/in.h>  
#include <net/if.h>  
#include <net/if_arp.h>  
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/if_ether.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdarg.h> //va_list
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <sys/syscall.h>

/**************************** 私用头文件* **********************************/
#include "../../../com_inc/bsp_types.h"
#include "../inc/bsp_dsp.h"
#include "../../../com_inc/bsp_fpga_ext.h"
#include "../../../com_inc/fsl_p2041_ext.h"
/******************************* 局部宏定义 *********************************/


/*********************** 全局变量定义/初始化 ********************************/
pthread_mutex_t  g_mp_boot_dsp = PTHREAD_MUTEX_INITIALIZER;  /* 互斥 dsp 加载访问*/

s32 g_s32SocketUdp;
struct stDspCmdContext
{
    u32 u32CmdHeader;
    u8 u8DspId;
    u8 u8Respond[3];
};

struct sockaddr_in struBaseBandAddr = {0};    /* system socket addr struct */
struct sockaddr_in struHostAddr;    /* system socket addr struct */

/************************** 局部常数和类型定义 ************************/
extern void bsp_sys_msdelay(ul32 dwTimeOut);// ---延迟单位为毫秒
extern void bsp_sys_usdelay(ul32 dwTimeOut); //---延迟单位为微妙

/************************* 局部函数原型声明 **************************/

/*************************  函数实现    ***********************************/


/******************************************************************************
* 函数名: bsp_reset_fpga
* 描  述:  整板复位  
* 相关文档:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/

/******************************************************************************
* 函数名: asciiByte
* 功  能: 文件解析
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
Private s32 asciiByte (u8 u8Character)
{
	if ((u8Character >= '0') && (u8Character <= '9'))
		return (1);

	if ((u8Character >= 'A') && (u8Character <= 'F'))
		return (1);

	return (0);
}

/******************************************************************************
* 函数名: toNum
* 功  能: 文件解析
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
Private s32 toNum (u8 u8Character)
{
	if ((u8Character >= '0') && (u8Character <= '9'))
	{
		return (u8Character - '0');
	}

	return (u8Character - 'A' + 10);
}

/******************************************************************************
* 函数名: stripLine
* 功  能: 文件解析
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
Private void  stripLine (FILE *File_stream)
{
	s8 s8iline[132];
	fgets ((char *)s8iline, 131, File_stream);
}

/******************************************************************************
* 函数名: getFileSize
* 功  能: 文件解析
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明: Read the file size in bytes
******************************************************************************/
Private s32 getFileSize (FILE *File_stream)
{
	u8 u8x, u8y;
	u32 u32byteCount = 0;       

	/* Strip the 1st two lines */
	stripLine (File_stream);
	stripLine (File_stream);

	for (;;) 
	{
		/* read the 1st ascii char */
		do  
		{
			u8x = fgetc (File_stream);
			if (u8x == (u8)EOF)
			{
				return (u32byteCount);
		  	}
        }
        while (!asciiByte(u8x));

        /* Read the next ascii char */
        u8y = fgetc(File_stream);
        if (u8y == (u8)EOF)
        {
            return (u32byteCount);
        }

		if (asciiByte(u8y)) 
			u32byteCount++;
	}    
}


/* 
  Input file format: boot table in big endian format

  Output file format: 

	1st line: CCS data format
	2nd line: 0x0000
	3rd line: length of first packet in bytes, length counts not include itself
	.......
	first packet 
	.......

	0xEA00
	0x0000
	length of the second packet in bytes, length counts not include itself
	.......
	second packet
	.......
	

  	0xEA00
	0x0000
	length of the other packet in bytes, length counts not include itself
	.......
	other packets
	.......

	0xEA00 
	0X0000
	0X0000: end of the file
*/
/******************************************************************************
* 函数名: getDataIn
* 功  能: 文件解析
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
Private s32 getDataIn(FILE *File_stream, s32 s32size_bytes, u16 *u16output)
{
	s32 s32i;
	u8 u8x, u8y;
	u8 u8c[2];
    
	for(s32i= 0; s32i< s32size_bytes/2; s32i++) 
	{
		u8c[0] = 0;
		u8c[1] = 0;

        /* read the 1st ascii char */
        do
        {
            u8x = fgetc (File_stream);
            if (u8x == (u8)EOF)
            {
                printf("file parsing error\n");
                return BSP_ERROR;
            }
        }
        while (!asciiByte(u8x));


        /* Read the next ascii char */
        u8y = fgetc (File_stream);
        if (u8y == (u8)EOF)
        {
            printf("file parsing error\n");
            return BSP_ERROR;
        }

        if (asciiByte(u8y))
        {
            u8c[0] =  (toNum(u8x) << 4) | toNum (u8y);
        }

        do
        {
            u8x = fgetc(File_stream);
            if (u8x == (u8)EOF)
            {
                printf("file parsing error\n");
                return BSP_ERROR;
            }
        }
        while (!asciiByte(u8x));

        /* Read the next ascii char */
        u8y = fgetc (File_stream);
        if (u8y == (u8)EOF)
        {
            printf("file parsing error\n");
            return BSP_ERROR;
        }

        if (asciiByte(u8y))
        {
            u8c[1] =  (toNum(u8x) << 4) | toNum (u8y);
        }

        *u16output++ = (u8c[0] << 8) | u8c[1];
    }
    return BSP_OK;
}

/******************************************************************************
* 函数名: send_packet
* 功  能: 发数据包
* 相关文档:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
Private s32 send_packet(u8 * pbuf,u32 u32Len,struct sockaddr_in struOppositeAddr)
{
    s32 u32i,u32tmp1,u32tmp2;

    //printf("send_packet in !\n");
    /* send the packet via ethernet */
    if (0 > sendto(g_s32SocketFd, (u8*)pbuf, (u32)u32Len,0,(struct sockaddr*)&struOppositeAddr,sizeof(struct sockaddr)))
    {
        printf("[fdd_dsp_download]:failed! failed to send packet to ti dsp!\n");
        //close(g_s32SocketFdBootp);
        //close(g_s32SocketFd);
        return BSP_ERROR;
    }

    /* for delay */
    for (u32i = 0; u32i < BBU_SOCKET_SEND_DELAY*11; u32i ++)
    {
        u32tmp1 = u32tmp2;
        u32tmp2 = u32tmp1;
    }
    return BSP_OK;
}

s32 send_ddr_cfg_packet(const s8 *pFileTem,u32 download_len,struct sockaddr_in struOppositeAddr)
{


}
void write_slot_id(char *buf, int len, int slot)
{
	int i = 0;
	//unsigned char temp[100] = {0x00, 0x00, 0x00, 0x04, 0xBF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xFF};
	unsigned int *ptls = (unsigned int *)buf;
	
    for(i=0; i<(len/4); i++)
    {
		//printf("[%s]len=%d, i=%d\r\n", __func__, len, i);
		if( ptls[i+0]==0x00000004 && ptls[i+1]==0xBFFFFF80 && ptls[i+2]==0x000000FF)
		{
			ptls[i+2] = 0x000000FF & slot;
			break;
		}
	}
}
#if 0
void write_MCT_slot_id(char *buf, int len, int slot)
{
	//slot = 1;
	int i = 0;
	//unsigned char temp[100] = {0x00, 0x00, 0x00, 0x04, 0xBF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xFF};
	unsigned int *ptls = (unsigned int *)buf;

	for(i=0;i<(len/4);i++){
		//printf("[%s]len=%d, i=%d\r\n", __func__, len, i);
		if( ptls[i+0]==0x00000004 && ptls[i+1]==0xBFFFFF84 && ptls[i+2]==0x000000FF){
			ptls[i+2] = 0x000000FF & slot;
			break;
		}
	}
	
}
#endif
#if 1
/******************************************************************************
* 函数名: Send_DSPS_Image
* 功  能: 发送dsp 镜像文件
* 相关文档:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
Private s32  send_dsp_Image(int boardid, const s8 *pFileTem,struct sockaddr_in struOppositeAddr)
{
	FILE *file_strin_size;
	FILE *file_strin;
	s32 counter,indx;
	s32 flag = 0;
	u8 *u8pbuf = NULL;
	u32 dsps_image_pkt_send_head = 0x544b0100;
	u32 u32DownloadLen = MAX_BOOTTBL_LEN;   /* record the downloadlen in a file */
	//u32 u32DownloadLen1 = MAX_BOOTTBL_LEN/2;
	u32 u32LenTem = 0;
	u32 u32ddr_cfg_len = 712;
	int MCT_slotid;
    
    u8pbuf = (u8 *)malloc(2000);
    if(u8pbuf==NULL)
    {
		printf("[%s]:malloc error!\r\n", __func__);
		return BSP_ERROR;
	}

    memset(u8pbuf,0,2000);
	//printf("Send_DSPS_Image in !\n");

	printf("%s\n",pFileTem);

	file_strin_size = fopen((const s8 *)pFileTem,"rb");
	if(file_strin_size == NULL)
	{
		printf("open %s error!please check it! \n",pFileTem);
		free(u8pbuf);
		return BSP_ERROR;
	}
	//printf("2222222\n");

	u32LenTem = getFileSize(file_strin_size);
	fclose(file_strin_size);
	
	//printf("333333\n");
	u32LenTem = u32LenTem - u32ddr_cfg_len;
	counter = u32LenTem / u32DownloadLen;  // calculate how many packets
	printf("counter = %d,u32LenTem = 0x%x,u32ddr_cfg_len = %d\n",counter,u32LenTem,u32ddr_cfg_len);
	//printf("counter = %d,u32LenTem = 0x%x,u32DownloadLen = %d\n",counter,u32LenTem,u32DownloadLen);

	if((u32LenTem % u32DownloadLen) != 0)
	{
		flag = 1;
	}

	#if 1
	u32 ddr_cfg_packet_len = 0;
	file_strin = fopen((const char *)pFileTem,"rb");
	if(file_strin == NULL)
	{
		printf("open %s error!please check it! \n",pFileTem);
		free(u8pbuf);
		return BSP_ERROR;
	}

	/* Strip the 1st two lines */
	stripLine (file_strin);
	stripLine (file_strin);
	
	if(1)   
    {
        ddr_cfg_packet_len = 88;
        if(getDataIn(file_strin,ddr_cfg_packet_len,(u16 *)((u8 *)u8pbuf+4))!=0)
        {
            printf("getDataIn error0!\n");
            free(u8pbuf);
            u8pbuf = NULL;
            fclose(file_strin);
            return BSP_ERROR;
        }
        else
        {

            *(u32 *)u8pbuf = dsps_image_pkt_send_head;
            int i=0;
            for(i = 0; i < ddr_cfg_packet_len+4; i++)
            {
                printf("%2X ",u8pbuf[i]);
                if(i!=0)
                {
                    if((i)%12==0)
                        printf("\n");
                }
            }
            printf("\n");
            if(send_packet((u8 *)u8pbuf,ddr_cfg_packet_len+4,struOppositeAddr) == -1)
            {
                printf("send packet error2!\n");
                free(u8pbuf);
                u8pbuf = NULL;
                fclose(file_strin);
                return BSP_ERROR;
            }
            bsp_sys_msdelay(1);
            dsps_image_pkt_send_head++;
            if(dsps_image_pkt_send_head >= 0x544b0200)
            {
                dsps_image_pkt_send_head = 0x544b0100;
            }
        }
        printf("packet1:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",u8pbuf[ddr_cfg_packet_len-8],u8pbuf[ddr_cfg_packet_len-7],u8pbuf[ddr_cfg_packet_len-6],u8pbuf[ddr_cfg_packet_len-5],\
               u8pbuf[ddr_cfg_packet_len-4],u8pbuf[ddr_cfg_packet_len-3],u8pbuf[ddr_cfg_packet_len-2],u8pbuf[ddr_cfg_packet_len-1],\
               u8pbuf[ddr_cfg_packet_len],u8pbuf[ddr_cfg_packet_len+1],u8pbuf[ddr_cfg_packet_len+2],u8pbuf[ddr_cfg_packet_len+3]);
    }

    if(2)
    {
        ddr_cfg_packet_len =12;
        if(getDataIn(file_strin,ddr_cfg_packet_len,(u16 *)((u8 *)u8pbuf+4))!=0)
        {
            printf("getDataIn error0!\n");
            free(u8pbuf);
            u8pbuf = NULL;
            fclose(file_strin);
            return BSP_ERROR;
        }
        else
        {
            *(u32 *)u8pbuf = dsps_image_pkt_send_head;
            if(send_packet((u8 *)u8pbuf,ddr_cfg_packet_len+4,struOppositeAddr) == -1)
            {
                printf("send packet error2!\n");
                free(u8pbuf);
                u8pbuf = NULL;
                fclose(file_strin);
                return BSP_ERROR;
            }
            bsp_sys_msdelay(1);
            dsps_image_pkt_send_head++;
            if(dsps_image_pkt_send_head >= 0x544b0200)
            {
                dsps_image_pkt_send_head = 0x544b0100;
            }
        }
        printf("packet2:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",u8pbuf[ddr_cfg_packet_len-8],u8pbuf[ddr_cfg_packet_len-7],u8pbuf[ddr_cfg_packet_len-6],u8pbuf[ddr_cfg_packet_len-5],\
               u8pbuf[ddr_cfg_packet_len-4],u8pbuf[ddr_cfg_packet_len-3],u8pbuf[ddr_cfg_packet_len-2],u8pbuf[ddr_cfg_packet_len-1],\
               u8pbuf[ddr_cfg_packet_len],u8pbuf[ddr_cfg_packet_len+1],u8pbuf[ddr_cfg_packet_len+2],u8pbuf[ddr_cfg_packet_len+3]);

    }

    if(3)
    {
        ddr_cfg_packet_len = 504;
        if(getDataIn(file_strin,ddr_cfg_packet_len,(u16 *)((u8 *)u8pbuf+4))!=0)
        {
            printf("getDataIn error0!\n");
            free(u8pbuf);
            u8pbuf = NULL;
            fclose(file_strin);
            return BSP_ERROR;
        }
        else
        {
            *(u32 *)u8pbuf = dsps_image_pkt_send_head;
            if(send_packet((u8 *)u8pbuf,ddr_cfg_packet_len+4,struOppositeAddr) == -1)
            {
                printf("send packet error2!\n");
                free(u8pbuf);
                u8pbuf = NULL;
                fclose(file_strin);
                return BSP_ERROR;
            }
            bsp_sys_msdelay(1);
            dsps_image_pkt_send_head++;
            if(dsps_image_pkt_send_head >= 0x544b0200)
            {
                dsps_image_pkt_send_head = 0x544b0100;
            }
        }
        printf("packet3:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",u8pbuf[ddr_cfg_packet_len-8],u8pbuf[ddr_cfg_packet_len-7],u8pbuf[ddr_cfg_packet_len-6],u8pbuf[ddr_cfg_packet_len-5],\
               u8pbuf[ddr_cfg_packet_len-4],u8pbuf[ddr_cfg_packet_len-3],u8pbuf[ddr_cfg_packet_len-2],u8pbuf[ddr_cfg_packet_len-1],\
               u8pbuf[ddr_cfg_packet_len],u8pbuf[ddr_cfg_packet_len+1],u8pbuf[ddr_cfg_packet_len+2],u8pbuf[ddr_cfg_packet_len+3]);

	}

	if(4)   
	{
		ddr_cfg_packet_len=36;
		if(getDataIn(file_strin,ddr_cfg_packet_len,(u16 *)((u8 *)u8pbuf+4))!=0) 
		{
		    printf("getDataIn error0!\n");
			free(u8pbuf);
		    u8pbuf = NULL;
			fclose(file_strin);
			return BSP_ERROR;
		}
		else
		{
		    *(u32 *)u8pbuf = dsps_image_pkt_send_head;
			if(send_packet((u8 *)u8pbuf,ddr_cfg_packet_len+4,struOppositeAddr) == -1)
			{
				printf("send packet error2!\n");
				free(u8pbuf);
				u8pbuf = NULL;
				fclose(file_strin);
				return BSP_ERROR;
			}
			bsp_sys_msdelay(12);
		   dsps_image_pkt_send_head++;
		   if(dsps_image_pkt_send_head >= 0x544b0200)
		   {
				dsps_image_pkt_send_head = 0x544b0100;
		   }	
		}
		printf("packet4:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",u8pbuf[ddr_cfg_packet_len-8],u8pbuf[ddr_cfg_packet_len-7],u8pbuf[ddr_cfg_packet_len-6],u8pbuf[ddr_cfg_packet_len-5],\
		u8pbuf[ddr_cfg_packet_len-4],u8pbuf[ddr_cfg_packet_len-3],u8pbuf[ddr_cfg_packet_len-2],u8pbuf[ddr_cfg_packet_len-1],\
		u8pbuf[ddr_cfg_packet_len],u8pbuf[ddr_cfg_packet_len+1],u8pbuf[ddr_cfg_packet_len+2],u8pbuf[ddr_cfg_packet_len+3]);
	}

	if(5)   
	{
		ddr_cfg_packet_len = 36;
		if(getDataIn(file_strin,ddr_cfg_packet_len,(u16 *)((u8 *)u8pbuf+4))!=0) 
		{
		    printf("getDataIn error0!\n");
			free(u8pbuf);
		    u8pbuf = NULL;
			fclose(file_strin);
			return BSP_ERROR;
		}
		else
		{
		    *(u32 *)u8pbuf = dsps_image_pkt_send_head;
			if(send_packet((u8 *)u8pbuf,ddr_cfg_packet_len+4,struOppositeAddr) == -1)
			{
				printf("send packet error2!\n");
				free(u8pbuf);
				u8pbuf = NULL;
				fclose(file_strin);
				return BSP_ERROR;
			}
			bsp_sys_msdelay(1000);
			dsps_image_pkt_send_head++;
			if(dsps_image_pkt_send_head >= 0x544b0200)
			{
				dsps_image_pkt_send_head = 0x544b0100;
			}	
		}
		printf("packet5:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",u8pbuf[ddr_cfg_packet_len-8],u8pbuf[ddr_cfg_packet_len-7],u8pbuf[ddr_cfg_packet_len-6],u8pbuf[ddr_cfg_packet_len-5],\
		u8pbuf[ddr_cfg_packet_len-4],u8pbuf[ddr_cfg_packet_len-3],u8pbuf[ddr_cfg_packet_len-2],u8pbuf[ddr_cfg_packet_len-1],\
		u8pbuf[ddr_cfg_packet_len],u8pbuf[ddr_cfg_packet_len+1],u8pbuf[ddr_cfg_packet_len+2],u8pbuf[ddr_cfg_packet_len+3]);
	}

	if(6)   
	{
		ddr_cfg_packet_len = 36;
		if(getDataIn(file_strin,ddr_cfg_packet_len,(u16 *)((u8 *)u8pbuf+4))!=0) 
		{
		    printf("getDataIn error0!\n");
			free(u8pbuf);
		    u8pbuf = NULL;
			fclose(file_strin);
			return BSP_ERROR;
		}
		else
		{
		    *(u32 *)u8pbuf = dsps_image_pkt_send_head;
			if(send_packet((u8 *)u8pbuf,ddr_cfg_packet_len+4,struOppositeAddr) == -1)
			{
				printf("send packet error2!\n");
				free(u8pbuf);
				u8pbuf = NULL;
				fclose(file_strin);
				return BSP_ERROR;
			}
			dsps_image_pkt_send_head++;
			if(dsps_image_pkt_send_head >= 0x544b0200)
			{
				dsps_image_pkt_send_head = 0x544b0100;
			}	
		}
		printf("packet6:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",u8pbuf[ddr_cfg_packet_len-8],u8pbuf[ddr_cfg_packet_len-7],u8pbuf[ddr_cfg_packet_len-6],u8pbuf[ddr_cfg_packet_len-5],\
		u8pbuf[ddr_cfg_packet_len-4],u8pbuf[ddr_cfg_packet_len-3],u8pbuf[ddr_cfg_packet_len-2],u8pbuf[ddr_cfg_packet_len-1],\
		u8pbuf[ddr_cfg_packet_len],u8pbuf[ddr_cfg_packet_len+1],u8pbuf[ddr_cfg_packet_len+2],u8pbuf[ddr_cfg_packet_len+3]);
	}
	
	 
	if(7)   
	{
		unsigned char temp[12] = {0x00, 0x00, 0x00, 0x04, 0xBF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0xFF};		
		temp[11] = 0xFF & boardid;
		
		ddr_cfg_packet_len = 12;
		*(u32 *)u8pbuf = dsps_image_pkt_send_head;
		memcpy(u8pbuf+4, temp, 12);
		if(send_packet((u8 *)u8pbuf,ddr_cfg_packet_len+4,struOppositeAddr) == -1)
		{
			printf("send packet error2!\n");
			free(u8pbuf);
			u8pbuf = NULL;
			fclose(file_strin);
			return BSP_ERROR;
		}
		dsps_image_pkt_send_head++;
		if(dsps_image_pkt_send_head >= 0x544b0200)
		{
			dsps_image_pkt_send_head = 0x544b0100;
		}	
		printf("packet7:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",u8pbuf[ddr_cfg_packet_len-8],u8pbuf[ddr_cfg_packet_len-7],u8pbuf[ddr_cfg_packet_len-6],u8pbuf[ddr_cfg_packet_len-5],\
		u8pbuf[ddr_cfg_packet_len-4],u8pbuf[ddr_cfg_packet_len-3],u8pbuf[ddr_cfg_packet_len-2],u8pbuf[ddr_cfg_packet_len-1],\
		u8pbuf[ddr_cfg_packet_len],u8pbuf[ddr_cfg_packet_len+1],u8pbuf[ddr_cfg_packet_len+2],u8pbuf[ddr_cfg_packet_len+3]);
	}
	#if 0
	if(8)
	{
		unsigned char temp[12] = {0x00, 0x00, 0x00, 0x04, 0xBF, 0xFF, 0xFF, 0x84, 0x00, 0x00, 0x00, 0xFF};
		temp[11] = 0xFF & bsp_get_slot_id();

		ddr_cfg_packet_len = 12;
		*(u32 *)u8pbuf = dsps_image_pkt_send_head;
		memcpy(u8pbuf+4, temp, 12);
		if(send_packet((u8 *)u8pbuf,ddr_cfg_packet_len+4,struOppositeAddr) == -1)
		{
			printf("send packet error2!\n");
			free(u8pbuf);
			u8pbuf = NULL;
			fclose(file_strin);
			return BSP_ERROR;
		}
		dsps_image_pkt_send_head++;
		if(dsps_image_pkt_send_head >= 0x544b0200)
		{
			dsps_image_pkt_send_head = 0x544b0100;
		}
		printf("packet8:%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x\n",u8pbuf[ddr_cfg_packet_len-8],u8pbuf[ddr_cfg_packet_len-7],u8pbuf[ddr_cfg_packet_len-6],u8pbuf[ddr_cfg_packet_len-5],\
		u8pbuf[ddr_cfg_packet_len-4],u8pbuf[ddr_cfg_packet_len-3],u8pbuf[ddr_cfg_packet_len-2],u8pbuf[ddr_cfg_packet_len-1],\
		u8pbuf[ddr_cfg_packet_len],u8pbuf[ddr_cfg_packet_len+1],u8pbuf[ddr_cfg_packet_len+2],u8pbuf[ddr_cfg_packet_len+3]);
	}
	#endif
	
   /* 
    free(u8pbuf);
	u8pbuf = NULL;
	fclose(file_strin);
	return  BSP_OK;
	*/	

	#endif

	if((counter < 1)&&(u32LenTem < u32DownloadLen))   
	{
		if(getDataIn(file_strin,u32LenTem,(u16 *)((u8 *)u8pbuf+4))!=0) 
		{
		    printf("getDataIn error0!\n");
			free(u8pbuf);
		    u8pbuf = NULL;
			fclose(file_strin);
			return BSP_ERROR;
		}
		else
		{
			write_slot_id(u8pbuf, u32LenTem, boardid);
			#if 0
			MCT_slotid = bsp_get_slot_id();
			write_MCT_slot_id(u8pbuf, u32LenTem, MCT_slotid);
			#endif			
		    *(u32 *)u8pbuf = dsps_image_pkt_send_head;
			if(send_packet((u8 *)u8pbuf,u32LenTem+4,struOppositeAddr) == -1)
			{
				printf("send packet error2!\n");
				free(u8pbuf);
				u8pbuf = NULL;
				fclose(file_strin);
				return BSP_ERROR;
			}
		    free(u8pbuf);
		    u8pbuf = NULL;
		    fclose(file_strin);
		    return  BSP_OK;
		}
	}
    else
	{     
	   for(indx = 0;indx < counter;indx++)
	   {
	       if(getDataIn(file_strin,u32DownloadLen,(u16 *)((u8 *)u8pbuf+4))!= BSP_OK) 
	       {
	       	   printf("getDataIn error1!\n");
	           free(u8pbuf);
			   u8pbuf = NULL;
			   fclose(file_strin);
			   return BSP_ERROR;
	       }
		   else
		   {	   
				write_slot_id(u8pbuf, u32DownloadLen, boardid);
				#if 0
				MCT_slotid = bsp_get_slot_id();
				write_MCT_slot_id(u8pbuf, u32DownloadLen, MCT_slotid);
				#endif				
				*(u32 *)u8pbuf = dsps_image_pkt_send_head;
			    if(send_packet((u8 *)u8pbuf,u32DownloadLen+4,struOppositeAddr) != BSP_OK)
			    {
			       	  printf("send packet error1!\n");
		              free(u8pbuf);
					  u8pbuf = NULL;
					  fclose(file_strin);
					  return BSP_ERROR;
	       		}			
		   }
		   dsps_image_pkt_send_head++;
		   if(dsps_image_pkt_send_head >= 0x544b0200)
		   {
				dsps_image_pkt_send_head = 0x544b0100;
		   }	
	   }
	   
	   if(flag == 1)   //the last one 
	   {
	       u32LenTem = u32LenTem - u32DownloadLen*counter;
		   printf("indx = %d,%d \n",indx,u32LenTem);	
		   if(getDataIn(file_strin, u32LenTem, (u16 *)((u8 *)u8pbuf+4))!= BSP_OK) 
	       {
	            printf("getDataIn error3!\n");
	            free(u8pbuf);
			    u8pbuf = NULL;
				fclose(file_strin);
				return BSP_ERROR;
	       }
		   else
		   {	
			   *(u32 *)u8pbuf = dsps_image_pkt_send_head;
			   if(send_packet((u8 *)u8pbuf,u32LenTem+4,struOppositeAddr)!= BSP_OK)				    
			   	{
			       	  printf("error4\n");
		              free(u8pbuf);
					  u8pbuf = NULL;
					  fclose(file_strin);
					  return BSP_ERROR;
	       		}
		   }
	   }	   
	   free(u8pbuf);
	   u8pbuf = NULL;
	   fclose(file_strin);  
	   return BSP_OK;
	}	   
}
#endif

/* dsp 一次boot 的情况 */
#if 1
/******************************************************************************
* 函数名: fdd_dsp_download
* 1, 建立socket ，复位第DSP(1-5), 接收广播bootp包。
  2，调用发送函数发代码到DSP(1-5)
  3，收到dsp(1-5)已经启动成功包。
* 相关文档:
* 函数存储类型:
* 参数:
* 参数名        类型        输入/输出       描述

* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
s32 bsp_dsp_download(int boardid, u32 u32DspId, const s8* pFileTem)
{
    u32 u32ErrorC;   /* error counter for dsp reset or other */
    u32 u32ErrorC1;
    struct sockaddr_in struMyAddr;          /* system socket addr struct */
    struct sockaddr_in struOppositeAddr;    /* system socket addr struct */
    struct sockaddr_in struInaddr;
    s32 s32NumBytes;                        /* real number fo byte for download */
    struct bootp_msg  *pstruBootP;          /* point to bootp message that received */

    /* add for bootp chaddr convert */
    s8 s8MacAddr[6] = {0} ;                       /* record the mac address for secondary download */
    u8 u8Chaddr[30] = {0} ;                       /* format string used for arpAdd */

    s32 s32Ret;                             /* reture value */
    s32 s32Ret1;                             /* reture value1 */
    u32 u32Flag;                            /* loop  flag    */
    u32 u32Flag1;                            /* loop  flag1    */
    s8  s8Buf[BBU_DSP_MAX_BUF_LENGTH] = {0};      /* download buffer */
    s8  s8Buf1[BBU_DSP_MAX_BUF_LENGTH] = {0};     /* download buffer1 */

    u8  u8BufIfAddr[30] = {0};

    struct timeval  struTv;
    fd_set struReadfds;
    u32   u32i1;
    u32   u32i2;
    s8  s8ptemp[100];
    s32 s32socketlen;
    int on = 1;

    char ifr[] = "eth0";

    /* SOCKET for bootp*/
    if (SOCKET_ERROR == (g_s32SocketFdBootp = socket(AF_INET,SOCK_DGRAM,0)))
    {
    	printf("[fdd_dsp_download]:failed! faild to creat the socket for bootp!\n");
    	return BSP_ERROR;
    }

    if (SOCKET_ERROR == (inet_aton((char *)u8BufIfAddr, (struct in_addr *) &(struInaddr))))
    {
    	close(g_s32SocketFdBootp);
    	printf("error in inet_aton");
    	return BSP_ERROR;
    }

    if (setsockopt(g_s32SocketFdBootp, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        perror("setsockopt g_s32SocketFdBootp");
        return -1;
    }

    if(setsockopt(g_s32SocketFdBootp, SOL_SOCKET, SO_BINDTODEVICE, (void *)ifr, sizeof(ifr))<0)
    {
        perror("setsockopt SO_BINDTODEVICE");
    }

    struMyAddr.sin_family = AF_INET;
    struMyAddr.sin_port = htons(BBU_SOCKET_PORT_DSP_BOOTP) ;
    struMyAddr.sin_addr.s_addr = INADDR_ANY;
    //struMyAddr.sin_addr.s_addr = inet_addr((char *)pMPC_to_dsp_Ip);
    bzero((s8*)(struMyAddr.sin_zero), sizeof(struMyAddr.sin_zero));

	if (SOCKET_ERROR == bind(g_s32SocketFdBootp, (struct sockaddr *)&struMyAddr, sizeof(struct sockaddr)))
	{
		printf("[fdd_dsp_download]:failed! failded to bind socket for bootp!\n");
		perror("bind bootp:");
		close(g_s32SocketFdBootp);
		return BSP_ERROR;
	}

	/*SOCKET for data send*/
	if (-1 == (g_s32SocketFd = socket(AF_INET,SOCK_DGRAM,0)))
	{
		printf("[fdd_dsp_download]:failed! faild to creat the socket for download!\n");
		close(g_s32SocketFdBootp);
		return BSP_ERROR;
	}
	
	on = 1;

    if (setsockopt(g_s32SocketFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0 )
    {
        perror("setsockopt g_s32SocketFd");
        return -1;
    }

	struMyAddr.sin_family = AF_INET;
	struMyAddr.sin_port = htons(BBU_SOCKET_PORT_DSP_DOWNLOAD) ;
	struMyAddr.sin_addr.s_addr = inet_addr((char *)pMPC_to_dsp_Ip);
	bzero((s8*)(struMyAddr.sin_zero), sizeof(struMyAddr.sin_zero));	
	/* socket bind */
	if (SOCKET_ERROR == bind(g_s32SocketFd,(struct sockaddr*)&struMyAddr,sizeof(struct sockaddr)))
	{
		printf("[fdd_dsp_download]:failed! faild to bind socket for download!\n");
		close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		return BSP_ERROR;
	}

	if (1 == g_s32DspDownloadDebugSwitch)
	{
		printf("[fdd_dsp_download:%d] coreid %d reseted\n",boardid, u32DspId);
		printf("[fdd_dsp_download] waiting for bootp \n");
	}

#if 1
	/* reset the right dsp and wait for bootp*/
	u32ErrorC = 0;

	switch (u32DspId)
	{
	case  BBU_DSP_ID_1:
		if (BBU_DSP_DOWNLOAD_DO == g_u8DspDownloadDo1)
		{
		    printf("boot dsp1! \n");
			s32Ret = bsp_bbp_dsp_reset(boardid, 1<<(u32DspId-1));//bsp_reset_dsp(u32DspId);
			if (BSP_OK != s32Ret)
			{
				printf("dsp1 reset failed in dsp download!\n");
				u32ErrorC ++;
			}
		}
		else
		{
			printf("dsp1 is not determined for download!\n");
			u32ErrorC ++;
		}
		break;
		
	case  BBU_DSP_ID_2:
		if (BBU_DSP_DOWNLOAD_DO == g_u8DspDownloadDo2)
		{
		    printf("boot dsp2! \n");
			s32Ret= bsp_bbp_dsp_reset(boardid, 1<<(u32DspId-1));//bsp_reset_dsp(u32DspId);
			if (BSP_OK != s32Ret)
			{
				printf("dsp2 reset faild in dsp download!\n");
				u32ErrorC ++;
			}
		}
		else
		{
			printf("dsp2 is not determined for download!\n");
			u32ErrorC ++;
		}
		break;

	case  BBU_DSP_ID_3:
		if (BBU_DSP_DOWNLOAD_DO == g_u8DspDownloadDo3)
		{
		    printf("boot dsp3! \n");
			s32Ret = bsp_bbp_dsp_reset(boardid, 1<<(u32DspId-1));//bsp_reset_dsp(u32DspId);
			if (BSP_OK != s32Ret)
			{
				printf("dsp3 reset failed in dsp download!\n");
				u32ErrorC ++;
			}
		}
		else
		{
			printf("dsp3 is not determined for download!\n");
			u32ErrorC ++;
		}
		break;

	case  BBU_DSP_ID_4:
		if (BBU_DSP_DOWNLOAD_DO == g_u8DspDownloadDo4)
		{
		    printf("boot dsp4! \n");
			s32Ret = bsp_bbp_dsp_reset(boardid, 1<<(u32DspId-1));//bsp_reset_dsp(u32DspId);
			if (BSP_OK != s32Ret)
			{
				printf("dsp4 reset failed in dsp download!\n");
				u32ErrorC ++;
			}
		}
		else
		{
			printf("dsp4 is not determined for download!\n");
			u32ErrorC ++;
		}
		break;

	default:
		printf("mistake dsp id in ti dsp download!\n");
		u32ErrorC ++;
	}
	
	if (0 < u32ErrorC )
	{
		close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		return BSP_ERROR;
	}

    //收到bootp包，获取包信息，获得对方的mac，ip,  第一步测试
	//打开文件，获取文件大小
	//传文件到DSP，需要拆包吗?
	//起来后再次收到DSP发来的单播包，解析包标志位看是否DSP成功启动

	/*initialize*/
	struTv.tv_sec = 0;
	struTv.tv_usec = 50000;

	u32Flag = 1;
	while (u32Flag)
	{
		for (u32i1 = 0; u32i1 <4; u32i1 ++)
		{
			FD_ZERO(&struReadfds);
			FD_SET((u32)g_s32SocketFdBootp, &struReadfds);

			(void)select(g_s32SocketFdBootp+1, &struReadfds, NULLPTR, NULLPTR, &struTv);
			if (FD_ISSET((u32)g_s32SocketFdBootp, &struReadfds))
			{
				s32NumBytes = 0;
				s32socketlen = sizeof(struct sockaddr);
				if (SOCKET_ERROR == (s32NumBytes = recvfrom(g_s32SocketFdBootp,s8Buf,(s32)BBU_DSP_MAX_BUF_LENGTH,0,(struct sockaddr*)&struOppositeAddr,(int *)&s32socketlen)))
				{
					s32NumBytes = s32NumBytes;
					printf("faild to receive the bootp packet!\n");
					continue;
				}
				printf("s32NumBytes = %d\n",s32NumBytes);

			   //if (inet_addr((char *)pMPC_to_dsp_Ip) == struOppositeAddr.sin_addr.s_addr)
				//if (INADDR_ANY == struOppositeAddr.sin_addr.s_addr)
               if(s32NumBytes>50)
			   {
					u32Flag = 0;
					break;
				}
			}
		}

		if (4 <= u32i1)
		{
			u32Flag ++;
		}
		if (1000000 <= u32Flag)
		{
			close(g_s32SocketFdBootp);
			close(g_s32SocketFd);
			printf("failed at waiting bootp!\n");
			return BSP_ERROR;
		}
	}
	int i;
	for(i= 0;i<50;i++)
	{
		printf("%x,",(u8)s8Buf[i]);
	}
   	printf("\n\n");
	//pstruBootP = (struct bootp_msg*)s8Buf;
	//memcpy((void*)s8MacAddr, (const void *)pstruBootP->bp_chaddr,6 );
	//printf("%d,%d,%d\n",pstruBootP->bp_op,pstruBootP->bp_htype,pstruBootP->bp_hlen);

	printf("%d,%d,%d\n",s8Buf[0],s8Buf[1],s8Buf[2]);

	/*检验是否真的是bootp请求包*/
	if(((u8)s8Buf[0] != 1)||((u8)s8Buf[1] != 1)||((u8)s8Buf[2] != 6))
	{
		close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		printf("the packet is not bootp,please try to boot dsp again!\n");
		return BSP_ERROR;
	}

	if(((u8)s8Buf[4] != 0x12)||((u8)s8Buf[5] != 0x34)||((u8)s8Buf[6] != 0x56)||((u8)s8Buf[7] != 0x78))
	{
		close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		printf("the packet is not TI DSP bootp,please try to boot dsp again!\n");
		return BSP_ERROR;
	}

	
#endif
	//memcpy((void*)s8MacAddr, (const void *)&s8Buf[28],6);

/************** 需要两次复位DSP 以便进行验证MAC地址是否一致****************/
	//bsp_sys_msdelay(1000);
#if 1
	/* reset the right dsp and wait for bootp*/
	printf("reset dsp%d again，check the MAC !\n",u32DspId);
	u32ErrorC1 = 0;

	switch (u32DspId)
	{
	case  BBU_DSP_ID_1:
		if (BBU_DSP_DOWNLOAD_DO == g_u8DspDownloadDo1)
		{
		    printf("now reset dsp1 again! \n");
			s32Ret1 = bsp_bbp_dsp_reset(boardid, 1<<(u32DspId-1));//bsp_reset_dsp(u32DspId);
			if (BSP_OK != s32Ret1)
			{
				printf("dsp1 reset failed in dsp download!\n");
				u32ErrorC1 ++;
			}
		}
		else
		{
			printf("dsp1 is not determined for download!\n");
			u32ErrorC1 ++;
		}
		break;
		
	case  BBU_DSP_ID_2:
		if (BBU_DSP_DOWNLOAD_DO == g_u8DspDownloadDo2)
		{
		    printf("now reset dsp2 again! \n");
			s32Ret1= bsp_bbp_dsp_reset(boardid, 1<<(u32DspId-1));//bsp_reset_dsp(u32DspId);
			if (BSP_OK != s32Ret1)
			{
				printf("dsp2 reset faild in dsp download!\n");
				u32ErrorC1 ++;
			}
		}
		else
		{
			printf("dsp2 is not determined for download!\n");
			u32ErrorC1 ++;
		}
		break;

	case  BBU_DSP_ID_3:
		if (BBU_DSP_DOWNLOAD_DO == g_u8DspDownloadDo3)
		{
		    printf("now reset dsp3 again!\n");
			s32Ret1 = bsp_bbp_dsp_reset(boardid, 1<<(u32DspId-1));//bsp_reset_dsp(u32DspId);
			if (BSP_OK != s32Ret1)
			{
				printf("dsp3 reset failed in dsp download!\n");
				u32ErrorC1 ++;
			}
		}
		else
		{
			printf("dsp3 is not determined for download!\n");
			u32ErrorC1 ++;
		}
		break;

	case  BBU_DSP_ID_4:
		if (BBU_DSP_DOWNLOAD_DO == g_u8DspDownloadDo4)
		{
		    printf("now reset dsp4 again!\n");
			s32Ret1 = bsp_bbp_dsp_reset(boardid, 1<<(u32DspId-1));//bsp_reset_dsp(u32DspId);
			if (BSP_OK != s32Ret1)
			{
				printf("dsp4 reset failed in dsp download!\n");
				u32ErrorC1 ++;
			}
		}
		else
		{
			printf("dsp4 is not determined for download!\n");
			u32ErrorC1 ++;
		}
		break;

	default:
		printf("mistake dsp id in ti dsp download!\n");
		u32ErrorC1 ++;
	}
	
	if (0 < u32ErrorC1 )
	{
		close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		return BSP_ERROR;
	}

	/*initialize*/
	struTv.tv_sec = 0;
	struTv.tv_usec = 50000;

	u32Flag1 = 1;
	while (u32Flag1)
	{
		for (u32i2 = 0; u32i2 <4; u32i2++)
		{
			FD_ZERO(&struReadfds);
			FD_SET((u32)g_s32SocketFdBootp, &struReadfds);

			(void)select(g_s32SocketFdBootp+1, &struReadfds, NULLPTR, NULLPTR, &struTv);
			if (FD_ISSET((u32)g_s32SocketFdBootp, &struReadfds))
			{
				s32NumBytes = 0;
				s32socketlen = sizeof(struct sockaddr);
				if (SOCKET_ERROR == (s32NumBytes = recvfrom(g_s32SocketFdBootp,s8Buf1,(s32)BBU_DSP_MAX_BUF_LENGTH,0,(struct sockaddr*)&struOppositeAddr,(int *)&s32socketlen)))
				{
					s32NumBytes = s32NumBytes;
					printf("faild to receive the bootp packet!\n");
					continue;
				}

			   //if (inet_addr((char *)pMPC_to_dsp_Ip) == struOppositeAddr.sin_addr.s_addr)
				if (INADDR_ANY == struOppositeAddr.sin_addr.s_addr)
				{
					u32Flag1 = 0;
					break;
				}
			}
		}

		if (4 <= u32i2)
		{
			u32Flag1 ++;
		}
		if (1000000 <= u32Flag1)
		{
			close(g_s32SocketFdBootp);
			close(g_s32SocketFd);
			printf("failed at waiting bootp!\n");
			return BSP_ERROR;
		}
	}
	int j;
	for(j= 0;j<50;j++)
	{
		printf("%x,",(u8)s8Buf1[j]);
	}
   	printf("\n\n");
	/*pstruBootP = (struct bootp_msg*)s8Buf;
	memcpy((void*)s8MacAddr, (const void *)pstruBootP->bp_chaddr,6 );
	printf("%d,%d,%d\n",pstruBootP->bp_op,pstruBootP->bp_htype,pstruBootP->bp_hlen);*/

	printf("%d,%d,%d\n",s8Buf1[0],s8Buf1[1],s8Buf1[2]);

	/*检验是否真的是TI DSP 的bootp请求包*/
	if(((u8)s8Buf1[0] != 1)||((u8)s8Buf1[1] != 1)||((u8)s8Buf1[2] != 6))
	{
		close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		printf("the packet is not bootp,please try to boot dsp again!\n");
		return BSP_ERROR;
	}

	if(((u8)s8Buf1[4] != 0x12)||((u8)s8Buf1[5] != 0x34)||((u8)s8Buf1[6] != 0x56)||((u8)s8Buf1[7] != 0x78))
	{
		close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		printf("the packet is not TI DSP bootp,please try to boot dsp%d again!\n",u32DspId);
		return BSP_ERROR;
	}	
#endif

	/*比较两次的mac地址是否一致，不一致的话返回再次加载*/
	if(memcmp(&s8Buf[28],&s8Buf1[28],6) != 0)
	{
		close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		printf("the MAC is not equal to last,try to boot dsp%d again!\n",u32DspId);
		return BSP_ERROR;
	}
	else
	{
		printf("the dsp%d MAC is right!\n",u32DspId);
	}
	/*arp add for certain way to download DATA*/
	//sprintf((char*)u8Chaddr,"%x:%x:%x:%x:%x:%x",pstruBootP->bp_chaddr[0],pstruBootP->bp_chaddr[1],pstruBootP->bp_chaddr[2],pstruBootP->bp_chaddr[3],pstruBootP->bp_chaddr[4],pstruBootP->bp_chaddr[5]);
	sprintf((char*)u8Chaddr,"%x:%x:%x:%x:%x:%x",(u8)s8Buf[28],(u8)s8Buf[29],(u8)s8Buf[30],(u8)s8Buf[31],(u8)s8Buf[32],(u8)s8Buf[33]);
	printf("%s\n",u8Chaddr);

    switch(u32DspId)
	{
		case  BBU_DSP_ID_1:
			strcpy(pDownloadIp,dsp1_download_ip);
			break;
			
		case  BBU_DSP_ID_2:
			strcpy(pDownloadIp,dsp2_download_ip);
			break;

		case  BBU_DSP_ID_3:
			strcpy(pDownloadIp,dsp3_download_ip);
			break;

		case  BBU_DSP_ID_4:
			strcpy(pDownloadIp,dsp4_download_ip);		
			break;
			
		default:
			printf("IP ID ERROR!\n");
	}

	/*添加arp表*/
	switch(u32DspId)
	{
		case  BBU_DSP_ID_1:
			sprintf(s8ptemp,"arp -s %s %s","10.0.0.54",u8Chaddr);
			break;
			
		case  BBU_DSP_ID_2:
			sprintf(s8ptemp,"arp -s %s %s","10.0.0.55",u8Chaddr);
			break;

		case  BBU_DSP_ID_3:
			sprintf(s8ptemp,"arp -s %s %s","10.0.0.56",u8Chaddr);
			break;

		case  BBU_DSP_ID_4:
			sprintf(s8ptemp,"arp -s %s %s","10.0.0.57",u8Chaddr);	
			break;
			
		default:
			printf("IP ID ERROR!\n");
	}

    printf("%s\n",s8ptemp);
    if(system((char *)s8ptemp) < 0)
    {
    	close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
		printf("system call wrong!\n");
		return BSP_ERROR;
	}
		
	if (1 == g_s32DspDownloadDebugSwitch)
	{
		printf("download imagine...\n");
	}

	struOppositeAddr.sin_family = AF_INET;
	struOppositeAddr.sin_port = htons(0x9);
	struOppositeAddr.sin_addr.s_addr =inet_addr((char *)pDownloadIp);
	bzero((s8*)struOppositeAddr.sin_zero, sizeof(struOppositeAddr.sin_zero));

	if(send_dsp_Image(boardid, pFileTem,struOppositeAddr) == BSP_ERROR)
    {
	    close(g_s32SocketFdBootp);
		close(g_s32SocketFd);
        return BSP_ERROR;
	}

    if (1 == g_s32DspDownloadDebugSwitch)
	{
		printf("DSP%d download ok!\n",u32DspId);
	}

    close(g_s32SocketFdBootp);
    close(g_s32SocketFd);
	
	/* set the flag for sunsequent*/
	switch (u32DspId)
	{
		case  BBU_DSP_ID_1:
			g_u8Dsp1DlFlag = BBU_DSP_IS_DOWNLOADED;
			break;
			
		case  BBU_DSP_ID_2:
			g_u8Dsp2DlFlag = BBU_DSP_IS_DOWNLOADED;
			break;

		case  BBU_DSP_ID_3:
			g_u8Dsp3DlFlag = BBU_DSP_IS_DOWNLOADED;
			break;

		case  BBU_DSP_ID_4:
			g_u8Dsp4DlFlag = BBU_DSP_IS_DOWNLOADED;
			break;

		default:
			
			return BSP_ERROR;
	}
	
    return BSP_OK;	   
}

/******************************************************************************
* 函数名: bsp_boot_dsp
* 描  述:  BOOT DSP
* 相关文档:
* 函数存储类型:
* 参数:
* 参数名    类型        输入/输出       描述
		    dspid: 1，2，3，4
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
s32 bsp_boot_dsp_alone(int boardid, u8 u8dspid)
{  
	u8 u8i = u8dspid; 
    switch(u8i)
    {
	    case 1:
		    printf("boot %s start! \n",dsp1_image_name);
		    if(bsp_dsp_download(boardid, BBU_DSP_ID_1,dsp1_image_name)== -1)
			{
				printf("fdd_dsp_download %s error!! \n",dsp1_image_name);
				return BSP_ERROR;		
			}
		break;

		case 2:
			printf("boot %s start! \n",dsp2_image_name);
		    if(bsp_dsp_download(boardid, BBU_DSP_ID_2,dsp2_image_name)== -1)
			{
				printf("fdd_dsp_download %s error!! \n",dsp2_image_name);
				return BSP_ERROR;		
			}
		break;
		
	   case 3: 
	   	    printf("boot %s start! \n",dsp3_image_name);
			if(bsp_dsp_download(boardid, BBU_DSP_ID_3,dsp3_image_name)== -1)
			{
				printf("fdd_dsp_download %s error!! \n",dsp3_image_name);
				return BSP_ERROR;		
			}	
	   	break;
		
	   	case 4:
			printf("boot %s start! \n",dsp4_image_name);
			if(bsp_dsp_download(boardid, BBU_DSP_ID_4,dsp4_image_name)== -1)
			{
				printf("fdd_dsp_download %s error!! \n",dsp4_image_name);
				return BSP_ERROR;		
			}
		break;

		default:
		printf("arguments wrong!\n");
		
    }
	return BSP_OK;
}

/******************************************************************************
* 函数名: bsp_boot_dsp
* 描  述:  
* 相关文档:
* 函数存储类型:
* 参数:
* 参数名    类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
s32 bsp_boot_dsp(int boardid, u8 u8dspid)
{
	//bsp_dsp_mutex_init();
	pthread_mutex_lock(&g_mp_boot_dsp);
    /*如果加载不成功，拉死dsp，避免switch 的多个dsp 的bootp包*/
	if(bsp_boot_dsp_alone(boardid, u8dspid) == BSP_ERROR)
	{
	    //bsp_close_dsp(u8dspid);
		bsp_bbp_dsp_close(boardid, 1<<(u8dspid-1));
		pthread_mutex_unlock(&g_mp_boot_dsp);
		return BSP_ERROR;
	}
	pthread_mutex_unlock(&g_mp_boot_dsp);
}

/******************************************************************************
* 函数名: bsp_boot_all_dsp
* 描  述:  BOOT all DSP
* 相关文档:
* 函数存储类型:
* 参数:
* 参数名    类型        输入/输出       描述
* 返回值: 0 正常，其他错误
* 说明:
******************************************************************************/
s32 bsp_boot_all_dsp(int boardid)
{ 
    s32 dsp1_time = 0;
    s32 dsp2_time = 0;
    s32 dsp3_time = 0;
    s32 dsp4_time = 0;
    s32 ret1,ret2,ret3,ret4;
	
    printf("start boot dsp, boardid=%d\n", boardid);

    //bsp_dsp_cmd_init();

    //bsp_close_all_dsp();
    //bsp_bbp_dsp_close(1);
    //bsp_bbp_dsp_close(2);
    //bsp_bbp_dsp_close(4);
    //bsp_bbp_dsp_close(8);
    bsp_bbp_dsp_close(boardid, 0xF);
	
    printf("now boot all dsp,please wait ......!\n");

    while(dsp1_time < 3)
    {
    	ret1 = bsp_boot_dsp(boardid, BBU_DSP_ID_1);
    	if(ret1 < 0)
    	{
    		dsp1_time++;
    		printf("try to boot dsp1 %d time !\n ",dsp1_time);
    	}
    	else
    	{
    		break;
    	}
    }

    while(dsp2_time < 3)
    {
    	ret2 = bsp_boot_dsp(boardid, BBU_DSP_ID_2);
    	if(ret2 < 0)
    	{
    		dsp2_time++;
    		printf("try to boot dsp2 %d time !\n ",dsp2_time);
    	}
    	else
    	{
    		break;
    	}
    }

    while(dsp3_time < 3)
    {
    	ret3 = bsp_boot_dsp(boardid, BBU_DSP_ID_3);
    	if(ret3 < 0)
    	{
    		dsp3_time++;
    		printf("try to boot dsp3 %d time !\n ",dsp3_time);
    	}
    	else
    	{
    		break;
    	}
    }

    while(dsp4_time < 3)
    {
    	ret4 = bsp_boot_dsp(boardid, BBU_DSP_ID_4);
    	if(ret4 < 0)
    	{
    		dsp4_time++;
    		printf("try to boot dsp4 %d time !\n ",dsp4_time);
    	}
    	else
    	{
    		break;
    	}
    }

    if(dsp1_time >= 3||dsp2_time >= 3||dsp3_time >= 3||dsp4_time >= 3)
    {
    	if(dsp1_time >= 3)
    	{
    		   printf("[%d]boot dsp1 failed!!\n", boardid);	
    	}

    	if(dsp2_time >= 3)
    	{
    		   printf("[%d]boot dsp2 failed!!\n", boardid);	
    	}

    	if(dsp3_time >= 3)
    	{
    		   printf("[%d]boot dsp3 failed!!\n", boardid);	
    	}

    	if(dsp4_time >= 3)
    	{
    		   printf("[%d]boot dsp4 failed!!\n", boardid);	
    	}
    	return BSP_ERROR;
    }
    else
    {	    
    	printf("[%d]boot all dsp success!!\n", boardid);
    	return BSP_OK;
    }
}

s32 bsp_dsp_mutex_init(void)
{
	s32 s32ret = 0;
	/*initializea mutext oits default value*/
	s32ret |=pthread_mutex_init(&g_mp_boot_dsp, NULL);
	return s32ret;
}
#endif

/******************************* 源文件结束 ********************************/


