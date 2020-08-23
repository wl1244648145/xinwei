/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_usb.c 
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
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include "bsp_types.h"
#include "bsp_usb.h"
#define USB_PATH    "/mnt/usb1/"
#define RW_FLAG      0
#define RW_FLAG_PLUS  1
#define PORTSC   0x184
#define USBBASEADDR(x)  (0x210000+((x-1)*0x1000))
#define CCS (0x1<<30)
#define OCA (0x1<<3)
extern u8 *g_u8ccsbar;
#if 0
#define g_s32UsbDevOnFlag(Id) g_s32Usb##Id##DevOnFlag
s32 g_s32UsbDevOnFlag(1) = 0;
s32 g_s32UsbDevOnFlag(2) = 0;
#define USBID(x) x
#else
s32 g_s32Usb1DevOnFlag = 0;
#endif
void bsp_create_usb(u8 u8Id)
{
    char *buf[100];
    char *filename[50];
    sprintf(buf, "mknod /dev/sda%d b 180 0",u8Id);
    system(buf);
    sprintf(filename, "/mnt/usb%d",u8Id);
    if(!bsp_check_dir(filename))
        bsp_create_dir(filename);
    sprintf(buf, "mount /dev/sda%d /mnt/usb%d",u8Id,u8Id);
    system(buf);
}
void bsp_remove_usb(u8 u8Id)
{
    char *buf[100];
    sprintf(buf, "umount /dev/sda%d",u8Id);
    system(buf);
    sprintf(buf, "rm /dev/sda%d",u8Id);
    system(buf);
}
unsigned int read_portsc(u8Id)
{
    return in_be32(g_u8ccsbar + USBBASEADDR(u8Id) + PORTSC);
}
#if 0
s32 bsp_usb_fun(u8 u8Id)
{
    u32 u32PortScAddr;
    u32 u32PortScVal;
    if(u8Id != 1 || u8Id !=2 )
        return BSP_ERROR;
    u32PortScAddr = g_u8ccsbar + USBBASEADDR(u8Id) + PORTSC;
    while(1)
    {
        u32PortScVal = in_be32(u32PortScAddr);
        printf("u32PortScVal = 0x%x.\n",u32PortScVal);
        if(g_s32UsbDevOnFlag(1) == 1)
        {
            if(u32PortScVal&OCA != OCA)
            {
                g_s32UsbDevOnFlag(1) = 0;
                bsp_decreate_usb(1);
                printf("not detect usb device on usb%d.\n",u8Id);
            }
        }
        if(u32PortScVal&OCA == OCA)
        {
            g_s32UsbDevOnFlag(1) = 1;
            bsp_create_usb(u8Id);
            printf("detect usb device on usb%d.\n",u8Id);
        }
        sleep(1);
    }
    return BSP_OK;
}
s32 bsp_usb_init(u8 u8Id)
{
    pthread_t ptid;
    pthread_t       a_thread;
    pthread_attr_t  attr;
    struct sched_param parm;
    int res;
    pthread_attr_init(&attr); 
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 20; 
    pthread_attr_setschedparam(&attr, &parm);
    res = pthread_create(&ptid, &attr, (FUNCPTR)bsp_usb_fun,u8Id);
    pthread_attr_destroy(&attr);
    if (-1 == res)
    {
        perror("create gps thread error!\n");
    }
    printf("init usb pthread\n"); 
    return BSP_OK;
}
#endif
/*usb是否在位*/
s32 bsp_check_usb()
{
    if(g_s32Usb1DevOnFlag == 1)
    {
        return BSP_OK;
    }
    else if(g_s32Usb1DevOnFlag == 0)
    {
        return BSP_ERROR;
    }
}

/*读usb文件*/
s32 bsp_read_usb(s8 *ps8FileName, u8 *pu8Buf, u32 u32Len, s32 s32Flag)
{
    FILE *pFile = NULL;
    u32 u32Cnt;
    s8 s8Name[128] = USB_PATH;
    s8 *ps8Flag;
    if(bsp_check_usb() != BSP_OK)
    {
        printf("usb is not usable.\n");
        return BSP_ERROR;
    }
    if(ps8FileName == NULL || pu8Buf == NULL)
    {
        printf("read usb error, param is NULL");
        return BSP_ERROR;
    }
    if(u32Len == 0)
        return BSP_OK;
    if(s32Flag == RW_FLAG)
        ps8Flag = "r";
    else if(s32Flag == RW_FLAG_PLUS)
        ps8Flag = "r+";
    else
    {
        printf("flag error.\n");
        return BSP_ERROR;
    }

    strcat(s8Name, ps8FileName);
    /*检查文件是否存在*/
    pFile = fopen(s8Name, ps8Flag);

    if(pFile == NULL)
    {
        printf("file %s  not exsit.\n", s8Name);
        return BSP_ERROR;
    }

    u32Cnt = fread((u8*)pu8Buf, 1, u32Len, pFile);
    if(u32Cnt != u32Len)
    {
        printf("read usb error.\n");
        fclose(pFile);
    }
    else
    {
        printf("read usb ok.\n");
    }
    fclose(pFile);

    return BSP_OK;
}

/*写usb文件*/
s32 bsp_write_usb(s8 *ps8FileName, u8 *pu8Buf, u32 u32Len, s32 s32Flag)
{
    FILE *pFile = NULL;
    u32 u32Cnt;
    s8 s8Name[128] = USB_PATH;
    s8 *ps8Flag;
    s8 *buf[100];
    if(bsp_check_usb() != BSP_OK)
    {
        printf("usb is not usable.\n");
        return BSP_ERROR;
    }
    if(ps8FileName == NULL || pu8Buf == NULL)
    {
        printf("read usb error, param is NULL");
        return BSP_ERROR;
    }

    if(u32Len == 0)
        return BSP_OK;
    if(s32Flag == RW_FLAG)
        ps8Flag = "w";
    else if(s32Flag == RW_FLAG_PLUS)
        ps8Flag = "w+";
    else
    {
        printf("flag error.\n");
        return BSP_ERROR;
    }

    strcat(s8Name, ps8FileName);
    /*检查文件是否存在*/
    pFile = fopen(s8Name, ps8Flag);
    if(pFile == NULL)
    {
        printf("file %s  not exsit create it.\n", s8Name);
        sprintf(buf,"touch %s",s8Name);
        system(buf);
        pFile = fopen(s8Name, ps8Flag);
        if(pFile == NULL)
        {
            printf("file %s  create fail.\n", s8Name);
        return BSP_ERROR;
        }
    }

    u32Cnt = fwrite((u8*)pu8Buf, 1, u32Len, pFile);
    if(u32Cnt != u32Len)
    {
        printf("write usb error.\n");
        fclose(pFile);
    }
    else
    {
        printf("write usb ok.\n");
    }
    fclose(pFile);

    return BSP_OK;
}

s32 bsp_usb_rw_test(s8 *ps8FileName, u8 *pu8WriteBuff, u8 *pu8ReadBuff, s32 s32Len)
{
    if(bsp_write_usb(ps8FileName, pu8WriteBuff, s32Len, RW_FLAG) != BSP_OK)
    {
        printf("usb write error.\r\n");
        return BSP_ERROR;
    }
    if(bsp_read_usb(ps8FileName, pu8ReadBuff, s32Len, RW_FLAG) != BSP_OK)
    {
        printf("usb read error.\r\n");
        return BSP_ERROR;
    }
    if(memcmp(pu8WriteBuff, pu8ReadBuff, s32Len) != 0)
    {
        printf("usb write data is not equal to read data.\r\n");
        return BSP_ERROR;
    }
    return BSP_OK;
}

void test_usb()
{
    char *name="whp";
    char *buf = "asdfghjkl";
    char readbuf[100];
    bsp_usb_rw_test(name,buf,readbuf,strlen(buf)+1);
}
