#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/mman.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/ioctl.h>
#include <mtd/mtd-user.h>

#include "bsp_types.h"
#include "bsp_boardtest.h"
#include "../../bbp_mcta/bsp_bbp_command.h"
#include "bsp_boardtest_ext.h"
#include "bsp_gps_ext.h"
#include "hmi_ext.h"
#include "bsp_i2c_ext.h"
#include "bsp_conkers_ext.h"
#include "../../afc/inc/bsp_afc.h"
#include "../../bbp_mcta/bsp_msg_proc.h"

#include "../../rrutest/inc/rrutest.h"

unsigned char bbp_boot_over = 0;
/* 配置测试slot */
#define MAX_CMD_NUMBER 200
unsigned int test_slot_mask = 0;
unsigned int test_error_times[MAX_CMD_NUMBER][MAX_BOARDS_NUMBER] = {0};

#define  MASK_TESTALL			0x0001
#define  MASK_TEST_DANBAN		0x0002

#define CONFIG_SYS_RAM_BASE		1500*1024*1024
#define RAM_SIZE (2*1024*1024*1024-1500*1024*1024)
unsigned long g_u8ddrbase = 0;

/* 光模块功率门限 */
static const int SFP_MAX = 1120;
static const int SFP_MIN = 140;

int boardtest_fd = -1;

int send_result(cmd_t cmd);

pthread_mutex_t ppcddr_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtd_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bbp_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t peu_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t fan_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t hmi_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t eeprom_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t alltest_lock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t rru_lock = PTHREAD_MUTEX_INITIALIZER; //rrutest add

void bsp_mutex_lock(pthread_mutex_t *lock)
{
    if(lock!=NULL)
        pthread_mutex_lock(lock);
}
void bsp_mutex_unlock(pthread_mutex_t *lock)
{
    if(lock!=NULL)
        pthread_mutex_unlock(lock);
}
typedef cmd_t(*FUNC)(cmd_t cmd);
typedef struct
{
    unsigned short mask;
    unsigned short boardtype;
    unsigned int error_times;
    unsigned short cmd;
    char *cmdname;
    FUNC func;
    pthread_mutex_t *plock;
} testcmd_t;

unsigned int is_alltesting = 0;
testcmd_t cmd_list[];
#define CMD_LIST_SIZE	sizeof(cmd_list)/sizeof(testcmd_t)

#define CHECK_RET(x)	 ((x<0)?"失败":"通过")

extern unsigned char g_BBU_Fiber_INfo[128];
/*****************主控板测试项***************************/
int check_mct_slot(cmd_t cmd)
{
    unsigned int len = cmd.pkg_datalen;
    unsigned char *data = cmd.pkg.u_data.st_data;
    u8 u8Slot;
    if(len != 1)
    {
        printf("mct slot check data length error:len=%d.\n",len);
        return -1;
    }
    if(data[0] != 1 && data[0] != 0)
    {
        printf("mct slot check data  error:data[0]=%d.\n",data[0]);
        return -1;
    }
    u8Slot = bsp_get_slot_id();
    if(u8Slot != data[0])
    {
        printf("mct slot check slot  error:data[0]=%d, slot = %d.\n",data[0], u8Slot);
        return -1;
    }
    return 0;
}
int check_bbp_slot(cmd_t cmd)
{
    unsigned int len = cmd.pkg_datalen;
    unsigned char *data = cmd.pkg.u_data.st_data;
    if(len != 1)
    {
        printf("bbp slot check data length error:len=%d.\n",len);
        return -1;
    }
    if(data[0] <2 || data[0] > 7)
    {
        printf("bbp slot check data  error:data[0]=%d.\n",data[0]);
        return -1;
    }
    return 0;
}
int check_fsa_slot(cmd_t cmd)
{
    unsigned int len = cmd.pkg_datalen;
    unsigned char *data = cmd.pkg.u_data.st_data;
    if(len != 1)
    {
        printf("fsa slot check data length error:len=%d.\n",len);
        return -1;
    }
    if(data[0] <6 || data[0] > 7)
    {
        printf("fsa slot check data  error:data[0]=%d.\n",data[0]);
        return -1;
    }
    return 0;
}
int check_peu_slot(cmd_t cmd)
{
    unsigned int len = cmd.pkg_datalen;
    unsigned char *data = cmd.pkg.u_data.st_data;
    if(len != 1)
    {
        printf("peu slot check data length error:len=%d.\n",len);
        return -1;
    }
    if((data[0] != 8) && (data[0] != 9))
    {
        printf("peu slot check data  error:data[0]=%d.\n",data[0]);
        return -1;
    }
    return 0;
}
int check_fan_slot(cmd_t cmd)
{
    unsigned int len = cmd.pkg_datalen;
    unsigned char *data = cmd.pkg.u_data.st_data;
    if(len != 1)
    {
        printf("fan slot check data length error:len=%d.\n",len);
        return -1;
    }
    if(data[0] != 10)
    {
        printf("fan slot check data  error:data[0]=%d.\n",data[0]);
        return -1;
    }
    return 0;
}
int check_mctbbp_slot(cmd_t cmd)
{
    unsigned int len = cmd.pkg_datalen;
    unsigned char *data = cmd.pkg.u_data.st_data;
    u8 u8Slot;
    if(len != 2)
    {
        printf("mct bbp slot check data length error:len=%d.\n",len);
        return -1;
    }
    if(data[0] != 1 && data[0] != 0)
    {
        printf("mct slot check data  error:data[0]=%d.\n",data[0]);
        return -1;
    }
    if(data[1] <2 || data[1] > 7)
    {
        printf("bbp slot check data  error:data[1]=%d.\n",data[1]);
        return -1;
    }
    u8Slot = bsp_get_slot_id();
    if(u8Slot != data[0])
    {
        printf("mct slot check slot  error:data[0]=%d, slot = %d.\n",data[0], u8Slot);
        return -1;
    }
    return 0;
}
int check_mctfan_slot(cmd_t cmd)
{
    unsigned int len = cmd.pkg_datalen;
    unsigned char *data = cmd.pkg.u_data.st_data;
    u8 u8Slot;
    if(len != 2)
    {
        printf("mct fan slot check data length error:len=%d.\n",len);
        return -1;
    }
    if(data[0] != 1 && data[0] != 0)
    {
        printf("mct slot check data  error:data[0]=%d.\n",data[0]);
        return -1;
    }
    if(data[1] != IPMB_SLOT10)
    {
        printf("fan slot check data  error:data[1]=%d.\n",data[1]);
        return -1;
    }
    u8Slot = bsp_get_slot_id();
    if(u8Slot != data[0])
    {
        printf("mct slot check slot  error:data[0]=%d, slot = %d.\n",data[0], u8Slot);
        return -1;
    }
    return 0;
}
int check_mctpeu_slot(cmd_t cmd)
{
    unsigned int len = cmd.pkg_datalen;
    unsigned char *data = cmd.pkg.u_data.st_data;
    u8 u8Slot;
    if(len != 2)
    {
        printf("mct peu slot check data length error:len=%d.\n",len);
        return -1;
    }
    if(data[0] != 1 && data[0] != 0)
    {
        printf("mct slot check data  error:data[0]=%d.\n",data[0]);
        return -1;
    }
    if((data[1] != IPMB_SLOT8) && (data[1] != IPMB_SLOT9))
    {
        printf("peu slot check data  error:data[1]=%d.\n",data[1]);
        return -1;
    }
    u8Slot = bsp_get_slot_id();
    if(u8Slot != data[0])
    {
        printf("mct slot check slot  error:data[0]=%d, slot = %d.\n",data[0], u8Slot);
        return -1;
    }
    return 0;
}
u8 bsp_get_bbp_slot(cmd_t cmd)
{
    return cmd.pkg.u_data.st_data[0];
}
u8 bsp_get_mct_slot(cmd_t cmd)
{
    return cmd.pkg.u_data.st_data[0];
}
u8 bsp_get_mctbbp_bbp_slot(cmd_t cmd)
{
    return cmd.pkg.u_data.st_data[1];
}
u8 bsp_get_board_type(cmd_t cmd)
{
    return cmd.pkg.board;
}
u8 bsp_get_product_type(cmd_t cmd)
{
    return cmd.pkg.product;
}
void ucharxtostr(unsigned char *u, char *s, int len)
{
    int i;
    for(i=0; i<len; i++)
        sprintf(s+2*i, "%02x", u[i]);
}

s32 stringisnull(char *str, int len)
{
    int i;
    for(i=0; i<len; i++)
    {
        if((u8)str[i]!= 0xff)
            break;
    }
    if(i>=len)
        return BSP_OK;
    else
        return BSP_ERROR;
}

int bsp_runing_time(void)
{
    static int started = 0;
    static time_t t_old, t;
    time(&t);
    if(started == 0)
    {
        started = 1;
        t_old = t;
    }
    //printf("t=%d, told=%d, t-told=%d\r\n", t, t_old, t-t_old);
    return (t - t_old);
}

cmd_t test_mct_temprature(cmd_t cmd)
{
    int times = 0;
    unsigned int datalen = cmd.pkg_datalen;
    unsigned int data = cmd.pkg.u_data.st_data;
    if(check_mct_slot(cmd)!=0)
        return cmd;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times<htonl(cmd.pkg_totaltimes)+1; times++)
    {
        uint8_t i = 0;
        int8_t temp[4] = "";
        int8_t max_temp_id = 0,  min_temp_id = 0;
        uint32_t success = 1;
        //获取温度
        for(i=0; i<4; i++)
        {
            if(bsp_read_temp(0, i+1,&temp[i])!=BSP_OK)
            {
                success = 0;
                send_msg(cmd, "获取温度点%d(1~4)失败!\r\n", (i+1));
            }
            else if(temp[i]<-10)
            {
                success = 0;
                send_msg(cmd, "温度%d(1~4)过低(失败)!\r\n", (i+1));
            }
            else if(temp[i]>85)
            {
                success = 0;
                send_msg(cmd, "温度%d(1~4)过高(失败)!\r\n", (i+1));
            }
        }
        //得到最大温度和最小温度值
        for(i=0; i<4; i++)
        {
            if(temp[i]>temp[max_temp_id])
            {
                max_temp_id = i;
            }
            if(temp[i]<temp[min_temp_id])
            {
                min_temp_id = i;
            }
        }
        //最大温度和最小温度差值
        if((temp[max_temp_id]-temp[min_temp_id])>25)
        {
            success = 0;
            send_msg(cmd, "温差过大！(失败)!\r\n");
        }

        send_msg(cmd, "温度(1~4)=%d,%d,%d,%d\r\n",temp[0],temp[1],temp[2],temp[3]);

        if(success == 1)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, " 温度正常!\r\n");
        }
        else
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}


int test_ram(cmd_t cmd, unsigned long base, uint32_t length)
{
    unsigned int i = 0;
    unsigned int loop = 0;
    for(loop=0; loop<8; loop++)
    {
        //write 0
        for(i=0; i<length-4; i+=4)
        {
            *((unsigned int *)(base+i)) = (0x01010101<<loop);
        }
        for(i=0; i<length-4; i+=4)
        {
            unsigned int data = *(unsigned int*)(base+i);
            if(data != (0x01010101<<loop))
            {
                send_msg(cmd, "PPC DDR写0错误,data[0x%x]<=>0x%x[0x%x]\r\n", base+i, data, (0x01010101<<loop));
                return BSP_ERROR;
            }
        }
        //write 1
        for(i=0; i<length-4; i+=4)
        {
            *((unsigned int *)(base+i)) = ~(0x01010101<<loop);
        }
        for(i=0; i<length-4; i+=4)
        {
            unsigned int data = *(unsigned int*)(base+i);
            if(data != ~(0x01010101<<loop))
            {
                send_msg(cmd, "PPC DDR写1错误,data[0x%x]<=>0x%x[0x%x]\r\n", base+i, data, (0x01010101<<loop));
                return BSP_ERROR;
            }
        }
    }
    //write partten
    for(i=0; i<length-4; i+=4)
    {
        *((unsigned int *)(base+i)) = 0x5A5A5A5A;
    }
    for(i=0; i<length-4; i+=4)
    {
        unsigned int data = *(unsigned int*)(base+i);
        if(data != 0x5A5A5A5A)
        {
            send_msg(cmd, "PPC DDR写5A5A错误,data[0x%x]<=>0x%x[0x%x]\r\n", base+i, data, (0x01010101<<loop));
            return BSP_ERROR;
        }
    }
    //write addr
    for(i=0; i<length-4; i+=4)
    {
        *((unsigned int *)(base+i)) = i;
    }
    for(i=0; i<length-4; i+=4)
    {
        unsigned int data = *(unsigned int*)(base+i);
        if(data != i)
        {
            send_msg(cmd, "PPC DDR写base错误,data[0x%x]<=>0x%x[0x%x]\r\n", base+i, data, (0x01010101<<loop));
            return BSP_ERROR;
        }
    }
    return BSP_OK;
}
void *test_ppc_ddr_thread(void *arg)
{
    int times = 0;
    int is_succes = 1;
    unsigned long base = 0;
    unsigned int length = 40*1024*1024;
    cmd_t cmd = *(cmd_t*)arg;
    if(check_mct_slot(cmd)!=0)
        return NULL;
    bsp_mutex_lock(cmd_list[cmd.index].plock);

    if(htonl(cmd.pkg_datalen) == sizeof(struct meminfo_t))
    {
        base = htonl(cmd.pkg_mem.addr);
        length = htonl(cmd.pkg_mem.length);
    }

    if( (base+length) > 450*1024*1024)
    {
        send_msg(cmd, "PPC DDR测试, 错误的地址或长度addr=0x%x, len=0x%x.\r\n"+1, base, length);
        cmd.pkg_successtimes = 0;
        cmd.pkg_failtimes = cmd.pkg_totaltimes;
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
        bsp_mutex_unlock(cmd_list[cmd.index].plock);
        return NULL;
    }

    base += g_u8ddrbase;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        send_msg(cmd, " PPC DDR开始测试..\r\n");

        if((base%4) != 0)
        {
            send_msg(cmd, " PPC DDR起始地址必须4字节对齐!\r\n");
        }
        else
        {
            if(test_ram(cmd, base, length)!=BSP_OK)
            {
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                send_msg(cmd, " PPC DDR测试失败!\r\n");
            }
            else
            {
                send_msg(cmd, " PPC DDR测试成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_ppc_ddr(cmd_t cmd)
{
    pthread_t tid;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    pthread_create(&tid, NULL, test_ppc_ddr_thread,(void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
int file_fill_and_check(char *filename, unsigned int size)
{
    FILE *fp = NULL;
    fp = fopen(filename, "wb");
    if(fp==NULL)
    {
        perror("fopen_write");
        return -1;
    }
    else
    {
        unsigned int i = 0, j = 0;
        unsigned char buf[1024] = {0};
        for(j=0; j<size/1024; j++)
        {
            for(i=0; i<sizeof(buf); i++)
            {
                buf[i] = j*1024+i;
            }
            fwrite(buf, sizeof(buf), 1, fp);
        }
        fclose(fp);
    }

    fp = fopen(filename, "rb");
    if(fp==NULL)
    {
        perror("fopen_read");
        return -1;
    }
    else
    {
        unsigned int i = 0, j = 0;
        unsigned char buf_read[1024] = {0};
        unsigned char buf_cmp[1024] = {0};
        for(j=0; j<size/1024; j++)
        {
            for(i=0; i<sizeof(buf_cmp); i++)
            {
                buf_cmp[i] = j*1024+i;
            }
            fread(buf_read, sizeof(buf_read), 1, fp);
            if(memcmp(buf_read, buf_cmp, sizeof(buf_read))!=0)
            {
                fclose(fp);
                return -1;
            }
        }
        fclose(fp);
    }
    return 0;
}
int erase_nor_flash(char *devicename, unsigned int start, unsigned int size)
{
    int fd = 0;
    struct erase_info_user erase;
    erase.start = start;
    erase.length = size;
    fd = open(devicename, O_SYNC | O_RDWR);
    if(fd<0)
    {
        perror("open");
        return -1;
    }
    if (ioctl(fd, MEMERASE, &erase) < 0)
    {
        printf ("While erasing blocks from 0x%.8x-0x%.8x on %s: %m\r\n",
                (unsigned int) erase.start,(unsigned int) (erase.start + erase.length));
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
int copy_file(char *src_filename, char *dst_filename, unsigned int size, int is_verify)
{
    FILE *fp_src=NULL, *fp_dst=NULL;
    char buf_src[1024]="";
    char buf_dst[1024]="";
//	int is_ok = 1;
    int retry_times = 3;
    //printf("src:%s, dst:%s\r\n", src_filename, dst_filename);
    fp_src = fopen(src_filename, "rb");
    if(fp_src==NULL)
    {
        return -1;
    }

    fp_dst = fopen(dst_filename, "wb+");
    if(fp_dst==NULL)
    {
        fclose(fp_src);
        return -1;
    }

    while(size>0)
    {
        int ret = 0;
        memset(buf_src, 0, sizeof(buf_src));
        fread(buf_src, sizeof(buf_src), 1, fp_src);
        retry_times = 3;
retry:
        retry_times--;
        ret = fwrite(buf_src, sizeof(buf_src), 1, fp_dst);
        fflush(fp_dst);
        if(is_verify)
        {
            int pos1 = 0, pos2 = 0;;
            memset(buf_dst, 0, sizeof(buf_dst));
            pos1 = ftell(fp_src);
            pos2 = ftell(fp_dst);

            fseek(fp_dst, pos1-sizeof(buf_dst), SEEK_SET);
            fread(buf_dst, sizeof(buf_dst), 1, fp_dst);
            pos2 = ftell(fp_dst);
            if (memcmp(buf_dst, buf_src, sizeof(buf_src))==0)
            {
                fseek(fp_dst, pos1, SEEK_SET);
                size -= sizeof(buf_src);
                continue;
            }
            else if(retry_times>0)
            {
                fseek(fp_dst, 0-sizeof(buf_src), SEEK_CUR);
                if(retry_times==0)
                {
                    fclose(fp_dst);
                    fclose(fp_src);
                    return -1;
                }
                goto retry;
            }
        }
        else
        {
            size -= sizeof(buf_src);
        }
    }


    fclose(fp_dst);
    fclose(fp_src);
    return 0;
}
int test_norflash(cmd_t cmd)
{
    int ret = 0;
    int size = 512*1024;
    char *devname = "/dev/mtd1";
    char *backfile = "/tmp/nor_flash_backup.dat";
    ret = copy_file(devname, backfile, size, 0);
    send_msg(cmd, "    备份nor...%s\r\n", CHECK_RET(ret));;
    if(ret<0)
        return BSP_ERROR;
    ret += erase_nor_flash(devname, 0, size);
    send_msg(cmd, "    擦除nor...%s\r\n", CHECK_RET(ret));
    if(ret<0)
        return BSP_ERROR;
    sleep(1);
    ret = file_fill_and_check(devname, size);
    send_msg(cmd, "    写入and验证...%s\r\n", CHECK_RET(ret));
    if(ret<0)
        return BSP_ERROR;
    ret = erase_nor_flash(devname, 0, size);
    send_msg(cmd, "    擦除...%s\r\n", CHECK_RET(ret));
    if(ret<0)
        return BSP_ERROR;
    sleep(1);
    ret = copy_file(backfile, devname, size, 1);
    send_msg(cmd, "    恢复...%s\r\n", CHECK_RET(ret));
    if(ret<0)
        return BSP_ERROR;

    return BSP_OK;
}
void *test_nor_thread(void *arg)
{
    int times = 0;
    int is_succes = 1;
    int bsp_start_time = 0;
    int ret = 0;
    cmd_t cmd = *(cmd_t*)arg;

    bsp_start_time = bsp_runing_time();

    bsp_mutex_lock(cmd_list[cmd.index].plock);

    send_msg(cmd, "开始测试nor flash...\r\n");

    if(bsp_start_time<0x70)
    {
        sleep(0x70-bsp_start_time);
    }

    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        send_msg(cmd, "NorFlash测试...\r\n");
        ret = test_norflash(cmd);
        if(ret==BSP_OK)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, "NorFlash测试成功\r\n");
        }
        else
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            send_msg(cmd, "NorFlash测试失败\r\n");
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_ppc_norflash(cmd_t cmd)
{
    pthread_t tid;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    pthread_create(&tid, NULL, test_nor_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_nand_thread(void *arg)
{
    cmd_t cmd = *(cmd_t *)arg;
    char *filename = "/mnt/nandtest_file.dat";
    int ret = 0;
    int times = 0;

    bsp_mutex_lock(cmd_list[cmd.index].plock);

    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        send_msg(cmd, " 测试nand flash...\r\n");
        ret = file_fill_and_check(filename, 20*1024*1024);
        if(ret<0)
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            send_msg(cmd, "    Nand测试失败!\r\n");
        }
        else
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, "    Nand测试成功!\r\n");
            remove(filename);
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_nandflash(cmd_t cmd)
{
    pthread_t tid;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    pthread_create(&tid, NULL, test_nand_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_cpldload_thread(void *arg)
{
    cmd_t cmd = *(cmd_t *)arg;
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    cmd.pkg_totaltimes = 1;

    if(BSP_OK!=bsp_update_cpld())
    {
        cmd.pkg_failtimes = htonl(1);
        send_msg(cmd, " CPLD加载失败!\r\n");
        send_result(cmd);
    }
    else
    {
        cmd.pkg_successtimes = htonl(1);
        send_msg(cmd, " CPLD加载成功!\r\n");
        send_result(cmd);
    }
    return NULL;
}
cmd_t test_cpld_load(cmd_t cmd)
{
    pthread_t tid;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    pthread_create(&tid, NULL, test_cpldload_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
cmd_t test_ppc_geswitch(cmd_t cmd)
{
    int times = 0;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times<htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int success=1;
        if(ethsw_write_test()!=BSP_OK)
        {
            success=0;
            send_msg(cmd, "geswitch 读写失败!\r\n");
        }

        if(success == 1)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, " geswitch 读写成功!\r\n");
        }
        else
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}
cmd_t test_cpld_version(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        char version[4] = {0};
        version[0] = bsp_cpld_read_reg(0);
        version[1] = bsp_cpld_read_reg(1);
        version[2] = bsp_cpld_read_reg(2);
        version[3] = bsp_cpld_read_reg(3);

        //cmd.pkg_datalen = htonl(4);
        //cmd.pkg_data[0] = version[0];
        //cmd.pkg_data[1] = version[1];
        //cmd.pkg_data[2] = version[2];
        //cmd.pkg_data[3] = version[3];

        send_msg(cmd, " CPLD Version:%02x%02x%02x%02x\r\n",  version[0], version[1], version[2], version[3]);
        if(version[0]<0x10 || version[0]>0x24)
        {
            send_msg(cmd, "   读CPLD版本失败!(年%x)\r\n", version[0]);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        else
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

struct cpudspres
{
    int slotid;
    int dspid;
    int res;
    int data_len;      /* dsp测试结果长度*/     
    char result[500]; /* dsp测试结果信息 */
};

struct cpudsptestcont
{
    int testid;
    char *name;
    sem_t *sem;
    unsigned int timeout;
};

sem_t  g_cpu_dsp_sem;
sem_t  g_dsp_ddr_sem;
sem_t  g_srio_data_sem;
sem_t  g_srio_effi_sem;
sem_t  g_dsp_aif_sem;
sem_t  g_dsp_up_sem;
sem_t  g_dsp_down_sem;
sem_t  g_sync_start_sem;
sem_t  g_sync_stop_sem;
sem_t  g_ir_delay_sem;
sem_t  g_boards_srio_sem;
sem_t  g_dsp_ver_sem;

#define MAX_DSP_TEST_NUMBER 12
struct cpudsptestcont strCpuDspTestCont[MAX_DSP_TEST_NUMBER] =
{
    {TEST_BBP_DSP_CPU, "CPU-DSP 链路", &g_cpu_dsp_sem, 30},
    {TEST_BBP_DSP_DDR, "DSP-DDR 测试", &g_dsp_ddr_sem, 300},
    {TEST_BBP_SRIO_DATA, "DSP-DSP SRIO 数据", &g_srio_data_sem, 30},
    {TEST_BBP_SRIO_EFFI, "DSP-DSP SRIO 效率", &g_srio_effi_sem, 30},
    {TEST_BBP_AIF, "DSP AIF", &g_dsp_aif_sem, 30},
    {TEST_BBP_UP, "上行链路", &g_dsp_up_sem, 30},
    {TEST_BBP_DOWN, "下行链路", &g_dsp_down_sem, 30},
    {TEST_SYNC_START, "同步开", &g_sync_start_sem, 30},
    {TEST_SYNC_STOP, "同步停", &g_sync_stop_sem, 30},
    {TEST_IR, "光延时测量", &g_ir_delay_sem, 30},
    {TEST_BOARDS_SRIO, "板间SRIO ", &g_boards_srio_sem, 30},
    {TEST_BBP_DSP_VERION, "DSP版本", &g_dsp_ver_sem, 30}
};

extern struct cpudspres cpudspresult[4];
sem_t  g_cpu_dsp_sem;
pthread_mutex_t dsp_lock = PTHREAD_MUTEX_INITIALIZER;

int find_cpudsp_test_item(int id)
{
    int i;

    for(i=0; i<MAX_DSP_TEST_NUMBER; i++)
    {
        if(strCpuDspTestCont[i].testid == id)
            return i;
    }

    return -1;
}

void init_cpu_dsp_test()
{
    sem_init(&g_cpu_dsp_sem,0,0);
    sem_init(&g_dsp_ddr_sem,0,0);
    sem_init(&g_srio_data_sem,0,0);
    sem_init(&g_srio_effi_sem,0,0);
    sem_init(&g_dsp_aif_sem,0,0);
    sem_init(&g_dsp_up_sem,0,0);
    sem_init(&g_dsp_down_sem,0,0);
    sem_init(&g_sync_start_sem,0,0);
    sem_init(&g_sync_stop_sem,0,0);
    sem_init(&g_ir_delay_sem,0,0);
    sem_init(&g_boards_srio_sem,0,0);
    sem_init(&g_dsp_ver_sem,0,0);
}

int test_updown = 1;
void *test_bbp_cpudsp_thread(void *arg)
{
    int times = 0;
    int dspid = 0;
    char *name;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8BbpSlot = 0;
    u8 u8DstBbpSlot = 0;
    u8 u8BoardType = 0;
    int index = 0;

    int testid = cmd.pkg_cmd; //& 0x0fff;

    bsp_mutex_lock(&dsp_lock);
    if(
        (TEST_BBP_UP == testid)
        || (TEST_BBP_DOWN == testid)
        || (TEST_SYNC_START == testid)
        || (TEST_SYNC_STOP == testid)
        || (TEST_IR == testid)
    )
    {
        if(check_mctbbp_slot(cmd)!=0)
        {
            bsp_mutex_unlock(&dsp_lock);
            send_msg(cmd, "error mct or bbp slot id.\r\n");
            return NULL;
        }
        u8BbpSlot = bsp_get_mctbbp_bbp_slot(cmd);
    }
    
    else
    {
        //判断是否是板间SRIO数据测试
        if(TEST_BOARDS_SRIO == testid)
        {
            if(cmd.pkg.datalen != 3)
            {
                bsp_mutex_unlock(&dsp_lock);
                send_msg(cmd, "error datalen.\r\n");
                return NULL;
            }
            if((cmd.pkg_data[1]<2) || (cmd.pkg_data[1]>7))
            {
                bsp_mutex_unlock(&dsp_lock);
                send_msg(cmd, "error src bbp slot.\r\n");
                return NULL;
            }
            if((cmd.pkg_data[2]<2) || (cmd.pkg_data[2]>7))
            {
                bsp_mutex_unlock(&dsp_lock);
                send_msg(cmd, "error dst bbp slot.\r\n");
                return NULL;
            }
            if(cmd.pkg_data[1] == cmd.pkg_data[2])
            {
                bsp_mutex_unlock(&dsp_lock);
                send_msg(cmd, "error src or dst bbp slot(src slot = dst slot).\r\n");
                return NULL;
            }
            u8BbpSlot = cmd.pkg_data[1];
            u8DstBbpSlot = cmd.pkg_data[2];
            if(boards[u8BbpSlot].type != BOARD_TYPE_BBP)
            {
                bsp_mutex_unlock(&dsp_lock);
                send_msg(cmd, "error src bbp boardtype:%d.\r\n");
                send_result(cmd);
                return NULL;
            }
        }
        else
        {
            if(check_bbp_slot(cmd)!=0)
            {
                bsp_mutex_unlock(&dsp_lock);
                send_msg(cmd, "error bbp slot id.\r\n");
                return NULL;
            }
            u8BbpSlot = bsp_get_bbp_slot(cmd);
            //获取板类型
            u8BoardType = boards[u8BbpSlot].type;
            if(BOARD_TYPE_BBP != u8BoardType)
            {
                bsp_mutex_unlock(&dsp_lock);
                send_msg(cmd, "error bbp boardtype:%d.\r\n", u8BoardType);
                send_result(cmd);
                return NULL;
            }
        }
    }

    index = find_cpudsp_test_item(testid);
    if(index < 0)
    {
        bsp_mutex_unlock(&dsp_lock);
        return NULL;
    }
    //初始化信号量
    sem_init(strCpuDspTestCont[index].sem,0,0);
    send_msg(cmd, "开始%s(TESTID:0x%04x)测试!\r\n", strCpuDspTestCont[index].name, testid);

    if((TEST_BBP_UP == testid) || (TEST_BBP_DOWN == testid))
    {
        if(test_updown == 1)
            bsp_close_aifloopback(u8BbpSlot);
        test_updown = 0;
    }

    for(times=1; times<htonl(cmd.pkg_totaltimes)+1; times++)
    {
        for(dspid=0; dspid<4; dspid++)
        {
            cpudspresult[dspid].slotid = u8BbpSlot;
            cpudspresult[dspid].dspid = -1;            
            cpudspresult[dspid].res = -1;
            memset(cpudspresult[dspid].result, 0, 500);
        }        
        int success=1;
        int dspres1=-1;
        int dspres2=-1;
        int dspres3=-1;
        int dspres4=-1;
        struct timespec ts;

        for(dspid=1; dspid<5; dspid++)
        {
            if(((TEST_BBP_AIF == testid) || (TEST_BBP_UP == testid)  || (TEST_BBP_DOWN == testid))
                    && (dspid == 4))
                continue;
            if(bsp_send_ipdata_to_dsp_fortest((testid & 0x0fff),u8BbpSlot,u8DstBbpSlot,dspid,0)!=BSP_OK)
            {
                success=0;
                send_msg(cmd, "SLOT%d CPU DSP--dsp %d 发送失败!\r\n",u8BbpSlot, dspid);
            }
        }
#if 0
        if(TEST_SYNC_START == testid)
        {
            pthread_mutex_unlock(&dsp_lock);
            return NULL;
        }
#endif
        /*wait for dsp test result*/
        /* 根据系统的当前时间赋值信号量的等待时间 */
        if(clock_gettime(CLOCK_REALTIME,&ts) == -1)
            perror("clock_gettime");
        ts.tv_sec += strCpuDspTestCont[index].timeout;
        sem_timedwait(strCpuDspTestCont[index].sem, &ts);

        if(cpudspresult[0].dspid == 1)
        {
            if(cpudspresult[0].res == 1)
                dspres1 = 1;
            else
            {
                send_msg(cmd, "SLOT%d DSP1 %s测试不通过!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
            }          
            send_msg(cmd, "SLOT%d DSP1 %s测试结果%s!\r\n", u8BbpSlot,strCpuDspTestCont[index].name, cpudspresult[0].result);
        }
        else
        {
            send_msg(cmd, "SLOT%d DSP1 %s测试超时!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
        }
        if(cpudspresult[1].dspid == 2)
        {            
            if(cpudspresult[1].res == 1)
                dspres2 = 1;
            else
            {
                send_msg(cmd, "SLOT%d DSP2 %s测试不通过!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
            } 
            send_msg(cmd, "SLOT%d DSP2 %s测试结果%s!\r\n", u8BbpSlot,strCpuDspTestCont[index].name, cpudspresult[1].result);
        }
        else
        {
            send_msg(cmd, "SLOT%d DSP2 %s测试超时!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
        }
        if(cpudspresult[2].dspid == 3)
        {
            if(cpudspresult[2].res == 1)
                dspres3 = 1;
            else
            {
                send_msg(cmd, "SLOT%d DSP3 %s测试不通过!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
            }
            send_msg(cmd, "SLOT%d DSP3 %s测试结果%s!\r\n", u8BbpSlot,strCpuDspTestCont[index].name, cpudspresult[2].result);
        }
        else
        {
            send_msg(cmd, "SLOT%d DSP3 %s测试超时!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
        }
        if((TEST_BBP_AIF != testid) && (TEST_BBP_UP != testid) && (TEST_BBP_DOWN != testid))
        {
            if(cpudspresult[3].dspid == 4)
            {
                if(cpudspresult[3].res == 1)
                    dspres4 = 1;
                else
                {
                    send_msg(cmd, "SLOT%d DSP4 %s测试不通过!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
                }
                send_msg(cmd, "SLOT%d DSP4 %s测试结果%s!\r\n", u8BbpSlot,strCpuDspTestCont[index].name, cpudspresult[3].result);
            }
            else
            {
                send_msg(cmd, "SLOT%d DSP4 %s测试超时!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
            }
        }

        if(TEST_BBP_AIF == testid || TEST_BBP_UP == testid  || TEST_BBP_DOWN == testid)
        {
            if((dspres1==1)&&(dspres2==1)&&(dspres3==1))
            {
                success = 1;
            }
            else
            {
                success = 0;
            }
        }
        else
        {
            if((dspres1==1)&&(dspres2==1)&&(dspres3==1)&&(dspres4==1))
            {
                success = 1;
            }
            else
            {
                success = 0;
            }
        }
        send_msg(cmd, " %s测试--slot%d dsp1 测试%s!\r\n", strCpuDspTestCont[index].name, u8BbpSlot, ((dspres1==1)?"成功":"失败"));
        send_msg(cmd, " %s测试--slot%d dsp2 测试%s!\r\n", strCpuDspTestCont[index].name, u8BbpSlot, ((dspres2==1)?"成功":"失败"));
        send_msg(cmd, " %s测试--slot%d dsp3 测试%s!\r\n", strCpuDspTestCont[index].name, u8BbpSlot, ((dspres3==1)?"成功":"失败"));
        if(TEST_BBP_AIF != testid && TEST_BBP_UP != testid && TEST_BBP_DOWN != testid)
        {
            send_msg(cmd, " %s测试--slot%d dsp4 测试%s!\r\n", strCpuDspTestCont[index].name, u8BbpSlot, ((dspres4==1)?"成功":"失败"));
        }
        if(success == 1)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, " %s测试成功!\r\n", strCpuDspTestCont[index].name);
        }
        else
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            send_msg(cmd, " %s测试失败!\r\n", strCpuDspTestCont[index].name);
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(&dsp_lock);
    return NULL;
}
cmd_t test_bbp_cpudsp(cmd_t cmd)
{
#if 1
    pthread_t tid;

    //sem_init(&g_cpu_dsp_sem,0,0);
    pthread_create(&tid, NULL, test_bbp_cpudsp_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
#else
    test_bbp_cpudsp_thread((void*)&cmd);
#endif
    return cmd;
}

cmd_t test_bbp_dsp_cpu(cmd_t cmd)
{
    cmd_t test_cmd = cmd;
    u8 slot = cmd.pkg_data[0];
    test_cmd.pkg_cmd = TEST_BBP_DSP_CPU;
    test_bbp_cpudsp_thread((void*)&test_cmd);
    //cmd_list[cmd.index].error_times += cmd.pkg_failtimes;
    test_error_times[cmd.index][slot] += cmd.pkg_failtimes;
    send_result(cmd);
    return cmd;
}

cmd_t test_bbp_dsp_ddr(cmd_t cmd)
{
    cmd_t test_cmd = cmd;
    u8 slot = cmd.pkg_data[0];
    test_cmd.pkg_cmd = TEST_BBP_DSP_DDR;
    test_bbp_cpudsp_thread((void*)&test_cmd);
    //cmd_list[cmd.index].error_times += cmd.pkg_failtimes;
    test_error_times[cmd.index][slot] += cmd.pkg_failtimes;
    send_result(cmd);
    return cmd;
}
cmd_t test_bbp_srio_data(cmd_t cmd)
{
    cmd_t test_cmd = cmd;
    u8 slot = cmd.pkg_data[0];
    test_cmd.pkg_cmd = TEST_BBP_SRIO_DATA;
    test_bbp_cpudsp_thread((void*)&test_cmd);
    //cmd_list[cmd.index].error_times += cmd.pkg_failtimes;
    test_error_times[cmd.index][slot] += cmd.pkg_failtimes;
    send_result(cmd);
    return cmd;
}
cmd_t test_bbp_dsp_aif(cmd_t cmd)
{
    cmd_t test_cmd = cmd;
    u8 slot = cmd.pkg_data[0];
    test_cmd.pkg_cmd = TEST_BBP_AIF;
    test_bbp_cpudsp_thread((void*)&test_cmd);
    //cmd_list[cmd.index].error_times += cmd.pkg_failtimes;
    test_error_times[cmd.index][slot] += cmd.pkg_failtimes;
    send_result(cmd);
    return cmd;
}

cmd_t test_ppc_gps(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(gps_NAV_SVInfo()==BSP_OK)
        {
            send_msg(cmd, " gps 测试成功!\r\n");
            send_msg(cmd, "可视星数%d，可追踪星数%d!\r\n", bsp_gps_VisibleSatellites(),bsp_gps_TrackedSatellites());
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " gps 测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}
void *test_ppc_usb_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    unsigned char au8WriteBuff[256] = {0};
    unsigned char au8ReadBuff[256] = {0};
    char as8FileName[32];
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        sprintf(as8FileName, "usbtest%d.txt");
        memset(au8WriteBuff, 0, 256);
        memset(au8ReadBuff, 0, 256);
        int i;
        for(i=0; i<256; i++)
            au8WriteBuff[i] = i;
        if(bsp_usb_rw_test(as8FileName, au8WriteBuff, au8ReadBuff, 256)==BSP_OK)
        {
            send_msg(cmd, " usb读写 测试成功!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " usb读写 测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return ;
}

cmd_t test_ppc_usb(cmd_t cmd)
{
    pthread_t tid;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    pthread_create(&tid, NULL, test_ppc_usb_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

cmd_t test_ppc_sfp(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    fiber_info fiberdata;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_get_fiber_Info_mutex(0, 0, &fiberdata)==BSP_OK)
        {
            send_msg(cmd, " sfp0读 测试成功!\r\n");
            send_msg(cmd, "sfp0 信息:\r\n");
            send_msg(cmd, "电压%6.2f(uV), 电流%6.2f(uA), 发送功率%6.2f(uW), 接收功率%6.2f(uW), 厂家信息%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
            if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
            {
                send_msg(cmd, " sfp0光模块测试成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp0发送功率测试失败!\r\n");
                }
                if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp0接收功率测试失败!\r\n");
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " sfp0读 测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if(bsp_get_fiber_Info_mutex(0, 1, &fiberdata)==BSP_OK)
        {
            send_msg(cmd, " sfp1读 测试成功!\r\n");
            send_msg(cmd, "sfp1 信息:\r\n");
            send_msg(cmd, "电压%6.2f(uV), 电流%6.2f(uA), 发送功率%6.2f(uW), 接收功率%6.2f(uW), 厂家信息%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
            if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
            {
                send_msg(cmd, " sfp1光模块测试成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp1发送功率测试失败!\r\n");
                }
                if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp1接收功率测试失败!\r\n");
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " sfp1读 测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}
cmd_t test_ppc_power(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;

    float power[4];
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_read_power(power)==BSP_OK)
        {
            send_msg(cmd, " 读功率 测试成功!\r\n");
            send_msg(cmd, "电压%f, 电流%f, 功率%f \r\n", power[0], power[1], power[2]);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " 读功率测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_ppc_eeprom(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;

    unsigned char u8Buff[128];
    char as8Str[128];
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        memset(u8Buff, 0, 128);
        memset(as8Str, 0, 128);
        if(bsp_read_eeprom(u8Buff, 128, 0)==BSP_OK)
        {
            send_msg(cmd, " 读eeprom 测试成功!\r\n");
            //ucharxtostr(u8Buff, as8Str, 128);
            //send_msg(cmd, "eeprom 信息:\r\n");
            //send_msg(cmd, "%s \r\n", as8Str);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " 读eeprom 测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_ppc_phy(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_phy_bcm54210s_test(0x08)==BSP_OK)
        {
            send_msg(cmd, " IPRANPHY1测试成功!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " IPRANPHY1测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if(bsp_phy_bcm54210s_test(0x09)==BSP_OK)
        {
            send_msg(cmd, " IPRANPHY2测试成功!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " IPRANPHY2测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if(bsp_phy_bcm54210s_test(0x19)==BSP_OK)
        {
            send_msg(cmd, " TRACEPHY测试成功!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " TRACEPHY测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_ppc_ver(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        bsp_runing_time();
        send_msg(cmd, "Mct Version:bsp_app %s %s\r\n", __DATE__, __TIME__);
        cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}
u32 g_IntTodUartFd = -1;
pthread_mutex_t g_mp_tod = PTHREAD_MUTEX_INITIALIZER;  
s32 tod_uart_set(void)
{
    struct termios Opt;
    if (system("mknod /dev/ttyS1 c 4 65") < 0)
        printf("dev ttyS2 mknod failed!\r\n");
    if(-1 == g_IntTodUartFd)
    {
        #ifdef NORMAL_MODE
        g_IntTodUartFd = open( "/dev/ttyS1", O_RDWR |O_NOCTTY | O_NDELAY);
        #else
        g_IntTodUartFd = open( "/dev/ttyS1", O_RDWR |O_NOCTTY | O_NONBLOCK/*O_NDELAY*/);
        #endif
    }
    
    if (ERROR == g_IntTodUartFd)
    {
        printf( "Open  g_IntTodUartFd failed!\n");
        return BSP_ERROR;
    }
    
    if(tcgetattr(g_IntTodUartFd, &Opt) != 0)
    {
    	printf("tcgetattr fd!\n");
    	return BSP_ERROR;
    }

    Opt.c_cflag &= ~CSIZE;
    Opt.c_cflag |= CS8;

    Opt.c_cflag |= (CLOCAL | CREAD);

    Opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    Opt.c_oflag &= ~OPOST;
    Opt.c_oflag &= ~(ONLCR | OCRNL); 

    Opt.c_iflag &= ~(ICRNL | INLCR);
    Opt.c_iflag &= ~(IXON | IXOFF | IXANY); 

    tcflush(g_IntTodUartFd, TCIFLUSH);
    Opt.c_cc[VTIME] = 0; 
    Opt.c_cc[VMIN] = 0; 

    if(tcsetattr(g_IntTodUartFd, TCSANOW, &Opt) != 0)
    {
    	printf("tcsetattr g_IntTodUartFd!\n");
    	return BSP_ERROR;
    }    
    return BSP_OK;	
}

s32 bsp_get_tod_status(void)
{
    int rtn=0, readlen= 0;
    struct timeval str_tv = {5, 0};
    static fd_set readfds;
    unsigned char tod_test_info[16] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x61,0x62,0x63,0x64,0x65,0x66};
    unsigned char tod_info[16] = {0};
    int i = 0;

    //pthread_mutex_lock(&g_mp_tod);
    FD_ZERO(&readfds);
    FD_SET(g_IntTodUartFd, &readfds);
    /*read tod frame*/
    while(1)
    {
        rtn = select(g_IntTodUartFd + 1, &readfds, NULL, NULL, &str_tv);
        if ((rtn <= 0) || (!(FD_ISSET (g_IntTodUartFd, &readfds))))
        {
            if (rtn == 0) 
            {
                printf("get tod or pp1s status time out!\n");
            } 
            else 
            {
                printf("faild to get tod or pp1s status!\n");
            }
            break;
        }
        //printf("[%s]%d.%d\n", __func__, str_tv.tv_sec, str_tv.tv_usec);
        readlen += read(g_IntTodUartFd, &tod_info[readlen], sizeof(tod_info)-readlen);
        if(sizeof(tod_info) <= readlen)
            break;
    }

    printf("uart2 recv data:");
    for(i=0;i<16;i++)
        printf(" %x", tod_info[i]);
    printf("\r\n");
    
    if(0 != g_IntTodUartFd)
        tcflush(g_IntTodUartFd, TCIFLUSH);

    if(memcmp(tod_info, tod_test_info,16) == BSP_OK)
    {
	//pthread_mutex_unlock(&g_mp_tod);
	return BSP_OK;
    }
    else
    {
        //pthread_mutex_unlock(&g_mp_tod);
	 return BSP_ERROR;
    }

    //pthread_mutex_unlock(&g_mp_tod);
    return BSP_OK;
}

void *test_ppc_extsync_pp1s_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;

    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        /* 配置RJ45 同步源PP1S*/
        printf("change to pp1s...\r\n");
        bsp_cpld_write_reg(102, 1);
        printf("read data from uart2...\r\n");
        if(bsp_get_tod_status()==BSP_OK)
        {
            send_msg(cmd, " 外同步接口pp1s测试成功!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " 外同步接口pp1s测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return ;
}

cmd_t test_ppc_extsync_pp1s(cmd_t cmd)
{
    pthread_t tid;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    pthread_create(&tid, NULL, test_ppc_extsync_pp1s_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_ppc_extsync_tod_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;

    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        /* 配置RJ45 同步源TOD*/
        printf("change to tod...\r\n");
        bsp_cpld_write_reg(102, 0);
        printf("read data from uart2...\r\n");
        if(bsp_get_tod_status()==BSP_OK)
        {
            send_msg(cmd, " 外同步接口tod测试成功!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " 外同步接口tod测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return ;
}

cmd_t test_ppc_extsync_tod(cmd_t cmd)
{
    pthread_t tid;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    pthread_create(&tid, NULL, test_ppc_extsync_tod_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}


/*****************基带板测试项***************************/
cmd_t test_bbp_temprature(cmd_t cmd)
{
    int times = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;

    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times<htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int i;
        int8_t temp[8]={0};
        int max_temp_id = 0,min_temp_id = 0;
        uint32_t success = 1;
        //获取温度
        if(bsp_bbp_read_temp(u8BbpSlot,temp)!=BSP_OK)
        {
            success = 0;
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "获取基带板(slot %d)(1~8)点温度获取失败!\r\n",u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "获取交换板(slot %d)(1~4)点温度获取失败!\r\n",u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "获取同步板(slot %d)温度获取失败!\r\n",u8BbpSlot);
        }
        if(BOARD_TYPE_BBP == u8BoardType)
        {
            //得到最大温度和最小温度值
            for(i = 0; i < 8; i++)
            {
                //找出过低或过高的温度值
                if( (temp[i] < -25) || (temp[i] > 90))
                {
                    success = 0;
                    send_msg(cmd, "基带板(slot %d)温度%d(1~8) %d错误(失败)!\r\n", u8BbpSlot, (i+1), temp[i]);
                }

                if(temp[i]>temp[max_temp_id])
                {
                    max_temp_id = i;
                }
                if(temp[i]<temp[min_temp_id])
                {
                    min_temp_id = i;
                }
            }
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            //得到最大温度和最小温度值
            for(i = 0; i < 4; i++)
            {
                //找出过低或过高的温度值
                if( (temp[i] < -25) || (temp[i] > 90))
                {
                    success = 0;
                    send_msg(cmd, "交换板(slot %d)温度%d(1~8) %d错误(失败)!\r\n", u8BbpSlot, (i+1), temp[i]);
                }

                if(temp[i]>temp[max_temp_id])
                {
                    max_temp_id = i;
                }
                if(temp[i]<temp[min_temp_id])
                {
                    min_temp_id = i;
                }
            }
        }
        if((BOARD_TYPE_BBP == u8BoardType) || (BOARD_TYPE_FSA == u8BoardType))
        {
            //最大温度和最小温度差值
            if((temp[max_temp_id]-temp[min_temp_id])>25)
            {
                success = 0;
                send_msg(cmd, "(slot %d)温差过大！(失败)!\r\n",u8BbpSlot);
            }
        }
        if(BOARD_TYPE_BBP == u8BoardType)
            send_msg(cmd, "基带板(slot %d)温度(1~8)=%d,%d,%d,%d,%d,%d,%d,%d\r\n",u8BbpSlot,temp[0],temp[1],temp[2],temp[3], temp[4],temp[5],temp[6],temp[7]);
        if(BOARD_TYPE_FSA == u8BoardType)
            send_msg(cmd, "交换板(slot %d)温度(1~4)=%d,%d,%d,%d\r\n",u8BbpSlot,temp[0],temp[1],temp[2],temp[3]);
        if(BOARD_TYPE_ES == u8BoardType)
            send_msg(cmd, "同步板(slot %d)温度=%d\r\n",u8BbpSlot, *(int16_t *)(temp));
        if(success == 1)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, "(slot %d)温度正常!\r\n", u8BbpSlot);
        }
        else
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}
cmd_t test_bbp_power_info(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if((BOARD_TYPE_BBP != u8BoardType) && (BOARD_TYPE_FSA != u8BoardType))
    {
        send_msg(cmd, "error bbp or fsa board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        float power[3] = {0.0};
        if(bsp_bbp_powerinfo(u8BbpSlot, power)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " 基带板(slot %d)Power传感器：电压=%f, 电流=%f, 功率=%f\r\n",u8BbpSlot, power[0],power[1],power[2]);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " 交换板(slot %d)Power传感器：电压=%f, 电流=%f, 功率=%f\r\n",u8BbpSlot, power[0],power[1],power[2]);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " 获取基带板(slot %d)Power传感器失败!\r\n",u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " 获取交换板(slot %d)Power传感器失败!\r\n",u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }

    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_bbp_ver(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_bbp_arm_version(u8BbpSlot, boards[u8BbpSlot].arm_version)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "基带板(slot %d)arm version = %s\r\n", u8BbpSlot,boards[u8BbpSlot].arm_version);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "交换板(slot %d)arm version = %s\r\n", u8BbpSlot,boards[u8BbpSlot].arm_version);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "同步板(slot %d)arm version = %s\r\n", u8BbpSlot,boards[u8BbpSlot].arm_version);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "基带板(slot %d)arm version获取失败!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "交换板(slot %d)arm version获取失败!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "同步板(slot %d)arm version获取失败!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_bbp_ir_sync(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;

    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if((BOARD_TYPE_BBP != u8BoardType) && (BOARD_TYPE_FSA != u8BoardType))
    {
        send_msg(cmd, "error bbp or fsa board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        uint16_t  u16IrSyncStat = 0;
        if(BOARD_TYPE_BBP == u8BoardType)
            send_msg(cmd, "基带板(slot %d)光口同步测试开始!\r\n", u8BbpSlot);
        if(BOARD_TYPE_FSA == u8BoardType)
            send_msg(cmd, "交换板(slot %d)光口同步测试开始!\r\n", u8BbpSlot);        
        //设置光口回环
        //bsp_bbp_fpga_write(u8BbpSlot, 205, 1);
        if(BOARD_TYPE_BBP == u8BoardType)
        {
            //清除同步状态
            bsp_bbp_fpga_write(u8BbpSlot, 11, 0xffff);
            //读取同步状态
            bsp_bbp_fpga_read(u8BbpSlot, 11, &u16IrSyncStat);
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            //清除同步状态
            bsp_fsa_fpga_write(u8BbpSlot, FSA_FPGA_325T, 11, 0xffff);
            //读取同步状态
            bsp_fsa_fpga_read(u8BbpSlot, FSA_FPGA_325T, 11, &u16IrSyncStat);
        }
        if(BOARD_TYPE_BBP == u8BoardType)
        {
            if((u16IrSyncStat & 0x07) == 0x07)
            {
                send_msg(cmd, "基带板(slot %d)光口同步测试成功!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {            
                send_msg(cmd, "基带板(slot %d)光口同步状态u16IrSyncStat = 0x%x.\r\n", u8BbpSlot, u16IrSyncStat);
                if((u16IrSyncStat & 0x01) != 0x01)
                {                
                    send_msg(cmd, "基带板(slot %d)光口0同步测试失败.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x02) != 0x02)
                {                
                    send_msg(cmd, "基带板(slot %d)光口1同步测试失败.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x04) != 0x04)
                {                
                    send_msg(cmd, "基带板(slot %d)光口2同步测试失败.\r\n", u8BbpSlot);
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            if((u16IrSyncStat & 0x3F) == 0x3F)
            {
                send_msg(cmd, "交换板(slot %d)光口同步测试成功!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {            
                send_msg(cmd, "交换板(slot %d)光口同步状态u16IrSyncStat = 0x%x.\r\n", u8BbpSlot, u16IrSyncStat);
                if((u16IrSyncStat & 0x01) != 0x01)
                {                
                    send_msg(cmd, "交换板(slot %d)光口0同步测试失败.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x02) != 0x02)
                {                
                    send_msg(cmd, "交换板(slot %d)光口1同步测试失败.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x04) != 0x04)
                {                
                    send_msg(cmd, "交换板(slot %d)光口2同步测试失败.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x08) != 0x08)
                {                
                    send_msg(cmd, "交换板(slot %d)光口3同步测试失败.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x10) != 0x10)
                {                
                    send_msg(cmd, "交换板(slot %d)光口4同步测试失败.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x20) != 0x20)
                {                
                    send_msg(cmd, "交换板(slot %d)光口5同步测试失败.\r\n", u8BbpSlot);
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        //取消光口回环
        //bsp_bbp_fpga_write(u8BbpSlot, 205, 0);
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }

    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_bbp_sfp(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    fiber_info fiberdata;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;

    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_bbp_get_sfpinfo(u8BbpSlot, 0, &fiberdata)==BSP_OK)
        {
            send_msg(cmd, " sfp0读 测试成功!\r\n");
            send_msg(cmd, "sfp0 信息:\r\n");
            send_msg(cmd, "电压%6.2f(uV), 电流%6.2f(uA), 发送功率%6.2f(uW), 接收功率%6.2f(uW), 厂家信息%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
            if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
            {
                send_msg(cmd, " sfp0光模块测试成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp0发送功率测试失败!\r\n");
                }
                if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp0接收功率测试失败!\r\n");
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " sfp0读 测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if(bsp_bbp_get_sfpinfo(u8BbpSlot, 1, &fiberdata)==BSP_OK)
        {
            send_msg(cmd, " sfp1读 测试成功!\r\n");
            send_msg(cmd, "sfp1 信息:\r\n");
            send_msg(cmd, "电压%6.2f(uV), 电流%6.2f(uA), 发送功率%6.2f(uW), 接收功率%6.2f(uW), 厂家信息%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
            if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
            {
                send_msg(cmd, " sfp1光模块测试成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp1发送功率测试失败!\r\n");
                }
                if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp1接收功率测试失败!\r\n");
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " sfp1读 测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if((BOARD_TYPE_BBP == u8BoardType) || (BOARD_TYPE_FSA == u8BoardType))
        {
            if(bsp_bbp_get_sfpinfo(u8BbpSlot, 2, &fiberdata)==BSP_OK)
            {
                send_msg(cmd, " sfp2读 测试成功!\r\n");
                send_msg(cmd, "sfp2 信息:\r\n");
                send_msg(cmd, "电压%6.2f(uV), 电流%6.2f(uA), 发送功率%6.2f(uW), 接收功率%6.2f(uW), 厂家信息%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
                if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                    && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
                {
                    send_msg(cmd, " sfp2光模块测试成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp2发送功率测试失败!\r\n");
                    }
                    if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp2接收功率测试失败!\r\n");
                    }
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " sfp2读 测试失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            if(bsp_bbp_get_sfpinfo(u8BbpSlot, 3, &fiberdata)==BSP_OK)
            {
                send_msg(cmd, " sfp3读 测试成功!\r\n");
                send_msg(cmd, "sfp3 信息:\r\n");
                send_msg(cmd, "电压%6.2f(uV), 电流%6.2f(uA), 发送功率%6.2f(uW), 接收功率%6.2f(uW), 厂家信息%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
                if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                    && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
                {
                    send_msg(cmd, " sfp3光模块测试成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp3发送功率测试失败!\r\n");
                    }
                    if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp3接收功率测试失败!\r\n");
                    }
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " sfp3读 测试失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            if(bsp_bbp_get_sfpinfo(u8BbpSlot, 4, &fiberdata)==BSP_OK)
            {
                send_msg(cmd, " sfp4读 测试成功!\r\n");
                send_msg(cmd, "sfp4 信息:\r\n");
                send_msg(cmd, "电压%6.2f(uV), 电流%6.2f(uA), 发送功率%6.2f(uW), 接收功率%6.2f(uW), 厂家信息%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
                if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                    && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
                {
                    send_msg(cmd, " sfp4光模块测试成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp4发送功率测试失败!\r\n");
                    }
                    if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp4接收功率测试失败!\r\n");
                    }
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " sfp4读 测试失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            if(bsp_bbp_get_sfpinfo(u8BbpSlot, 5, &fiberdata)==BSP_OK)
            {
                send_msg(cmd, " sfp5读 测试成功!\r\n");
                send_msg(cmd, "sfp5 信息:\r\n");
                send_msg(cmd, "电压%6.2f(uV), 电流%6.2f(uA), 发送功率%6.2f(uW), 接收功率%6.2f(uW), 厂家信息%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
                if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                    && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
                {
                    send_msg(cmd, " sfp5光模块测试成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp5发送功率测试失败!\r\n");
                    }
                    if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp5接收功率测试失败!\r\n");
                    }
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " sfp5读 测试失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

void *test_bbp_eeprom_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(BOARD_TYPE_BBP == u8BoardType)
            send_msg(cmd, "基带板(slot %d)EEPROM测试...\r\n", u8BbpSlot);
        if(BOARD_TYPE_FSA == u8BoardType)
            send_msg(cmd, "交换板(slot %d)EEPROM测试...\r\n", u8BbpSlot);
        if(BOARD_TYPE_ES == u8BoardType)
            send_msg(cmd, "同步板(slot %d)EEPROM测试...\r\n", u8BbpSlot);
        if(bsp_bbp_test_eeprom(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "基带板(slot %d)EEPROM测试成功!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "交换板(slot %d)EEPROM测试成功!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "同步板(slot %d)EEPROM测试成功!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "基带板(slot %d)EEPROM测试失败!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "交换板(slot %d)EEPROM测试失败!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "同步板(slot %d)EEPROM测试失败!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }

    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbp_eeprom(cmd_t cmd)
{
    pthread_t tid;
    if(check_bbp_slot(cmd)!=0)
        return cmd;
    pthread_create(&tid, NULL, test_bbp_eeprom_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_bbp_sdram_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(BOARD_TYPE_BBP == u8BoardType)
            send_msg(cmd, "基带板(slot %d)SDRAM测试...!\r\n", u8BbpSlot);
        if(BOARD_TYPE_FSA == u8BoardType)
            send_msg(cmd, "交换板(slot %d)SDRAM测试...!\r\n", u8BbpSlot);
        if(BOARD_TYPE_ES == u8BoardType)
            send_msg(cmd, "同步板(slot %d)SDRAM测试...!\r\n", u8BbpSlot);
        if(bsp_bbp_test_sdram(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "基带板(slot %d)SDRAM测试成功!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "交换板(slot %d)SDRAM测试成功!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "同步板(slot %d)SDRAM测试成功!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "基带板(slot %d)SDRAM测试失败!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "交换板(slot %d)SDRAM测试失败!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "同步板(slot %d)SDRAM测试失败!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbp_sdram(cmd_t cmd)
{
    pthread_t tid;
    if(check_bbp_slot(cmd)!=0)
        return cmd;
    pthread_create(&tid, NULL, test_bbp_sdram_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
cmd_t test_bbp_ethsw(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_bbp_test_ethsw(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " 基带板(slot %d)Ethsw测试成功!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " 交换板(slot %d)Ethsw测试成功!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, " 同步板(slot %d)Ethsw测试成功!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
             if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " 基带板(slot %d)Ethsw测试失败!\r\n", u8BbpSlot);
             if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " 交换板(slot %d)Ethsw测试失败!\r\n", u8BbpSlot);
             if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, " 同步板(slot %d)Ethsw测试失败!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}
cmd_t test_bbp_srioswt(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if((BOARD_TYPE_BBP != u8BoardType) && (BOARD_TYPE_FSA != u8BoardType))
    {
        send_msg(cmd, "error bbp or fsa board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_bbp_test_srioswt(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " 基带板(slot %d)srioswt路由测试成功!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " 交换板(slot %d)srioswt路由测试成功!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " 基带板(slot %d)srioswt路由测试失败!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " 交换板(slot %d)srioswt路由测试失败!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_fsa_phy(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_fsa_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(BOARD_TYPE_FSA != u8BoardType)
    {
        send_msg(cmd, "error fsa board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_fsa_test_phy54210s(u8BbpSlot)==BSP_OK)
        {
            send_msg(cmd, " 交换板(slot %d)phy测试成功!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " 交换板(slot %d)phy测试失败!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_fsa_pll_cfg(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa or es slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if((BOARD_TYPE_FSA != u8BoardType) && (BOARD_TYPE_ES != u8BoardType))
    {
        send_msg(cmd, "error fsa or es board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            if(bsp_test_pll1_config(u8BbpSlot)==BSP_OK)
            {
                send_msg(cmd, " 交换板(slot %d)pll1配置测试成功!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 交换板(slot %d)pll1配置测试失败!\r\n", u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            #if 0
            if(bsp_test_pll2_config(u8BbpSlot)==BSP_OK)
            {
                send_msg(cmd, " 交换板(slot %d)pll2配置测试成功!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 交换板(slot %d)pll2配置测试失败!\r\n", u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            #endif
        }
        if(BOARD_TYPE_ES == u8BoardType)
        {
            if(bsp_test_pll1_config(u8BbpSlot)==BSP_OK)
            {
                send_msg(cmd, " 同步板(slot %d)pll1配置测试成功!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 同步板(slot %d)pll1配置测试失败!\r\n", u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_fsa_fpga_325t_ddr(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    u16 u16ret = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_fsa_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(BOARD_TYPE_FSA != u8BoardType)
    {
        send_msg(cmd, "error fsa board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        //开始测试DDR,写1M数据后读出比较
        bsp_fsa_fpga_write(u8BbpSlot, FSA_FPGA_325T, 13, 1);
        //等待2秒
        sleep(2);
        //读测试结果(1正确,0异常)
        bsp_fsa_fpga_read(u8BbpSlot, FSA_FPGA_325T, 222, &u16ret);
        if(u16ret == 1)
        {
            send_msg(cmd, " 交换板(slot %d)fpga_325t DDR测试成功!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " 交换板(slot %d)fpga_325t DDR测试失败!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        //停止测试并清除DDR测试结果
        bsp_fsa_fpga_write(u8BbpSlot, FSA_FPGA_325T, 13, 0);
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

cmd_t test_fsa_fpga_160t_srio(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    u16 u16status = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_fsa_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(BOARD_TYPE_FSA != u8BoardType)
    {
        send_msg(cmd, "error fsa board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {        
        if(bsp_get_fsa_fpga160t_sync_status(u8BbpSlot, &u16status)==BSP_OK)
        {
            if((u16status&0x07) == 0x07)
            {
                send_msg(cmd, " 交换板(slot %d)fpga_160t高速接口测试成功!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 交换板(slot %d)fpga_160t高速接口同步状态0x%x!\r\n", u8BbpSlot, u16status);
                if((u16status&0x01) != 0x01)                    
                    send_msg(cmd, " 交换板(slot %d)fpga_160t高速接口10G失步!\r\n", u8BbpSlot);
                if((u16status&0x02) != 0x02)                    
                    send_msg(cmd, " 交换板(slot %d)fpga_160t高速接口SRIO1失步!\r\n", u8BbpSlot);
                if((u16status&0x04) != 0x04)                    
                    send_msg(cmd, " 交换板(slot %d)fpga_160t高速接口SRIO0失步!\r\n", u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 交换板(slot %d)fpga_160t高速接口测试失败!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

void * test_es_copper_link_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    int ret = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error es slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }     
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(BOARD_TYPE_ES != u8BoardType)
    {
        send_msg(cmd, "error es board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 u8mcts_tsync = 0;  
        u8 u8timeout = 4;
        u8 u8cnt = 0;
        send_msg(cmd, "同步板(slot %d)电口测试开始\r\n",u8BbpSlot);
        //配置电口2输出1PPS和TOD给主控
        bsp_bbp_fpga_write(u8BbpSlot, 8, 1);
        //配置电口1为输出，2为输入
        bsp_bbp_fpga_write(u8BbpSlot, 6, 5);
        sleep(1);
        for(u8cnt=0; u8cnt<u8timeout;u8cnt++)
        {
            if(0x11 == bsp_cpld_read_reg(124))
                break;
            sleep(1);
        }
        //判断主控板是否收到1PPS和TOD
        u8mcts_tsync = bsp_cpld_read_reg(124);
        send_msg(cmd, "同步板(slot %d)电口同步状态:0x%x\r\n", u8BbpSlot, u8mcts_tsync);
        if((u8mcts_tsync&0x11) == 0x11)
        {
            send_msg(cmd, "同步板(slot %d)电口发送1PPS和TOD成功\r\n",u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if((u8mcts_tsync&0x01) != 0x01)
                send_msg(cmd, "同步板(slot %d)电口发送TOD失败\r\n",u8BbpSlot);
            if((u8mcts_tsync&0x10) != 0x10)
                send_msg(cmd, "同步板(slot %d)电口发送1PPS失败\r\n",u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}

cmd_t test_es_copper_link(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_es_copper_link_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void * test_es_fibber_link_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    int ret = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error es slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }     
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(BOARD_TYPE_ES != u8BoardType)
    {
        send_msg(cmd, "error es board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 u8mcts_tsync = 0;   
        u8 u8timeout = 4;
        u8 u8cnt = 0;
        send_msg(cmd, "同步板(slot %d)光口测试开始\r\n",u8BbpSlot);
        //配置光口2输出1PPS和TOD给主控
        bsp_bbp_fpga_write(u8BbpSlot, 8, 3);
        //输出给主控使能开
        bsp_bbp_fpga_write(u8BbpSlot, 6, 4);
        sleep(1);
        for(u8cnt=0; u8cnt<u8timeout;u8cnt++)
        {
            if(0x11 == bsp_cpld_read_reg(124))
                break;
            sleep(1);
        }        
        //判断主控板是否收到1PPS和TOD
        u8mcts_tsync = bsp_cpld_read_reg(124);
        send_msg(cmd, "同步板(slot %d)光口同步状态:0x%x\r\n", u8BbpSlot, u8mcts_tsync);
        if((u8mcts_tsync&0x11) == 0x11)
        {
            send_msg(cmd, "同步板(slot %d)光口发送1PPS和TOD成功\r\n",u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {            
            if((u8mcts_tsync&0x01) != 0x01)
                send_msg(cmd, "同步板(slot %d)光口发送TOD失败\r\n",u8BbpSlot);
            if((u8mcts_tsync&0x10) != 0x10)
                send_msg(cmd, "同步板(slot %d)光口发送1PPS失败\r\n",u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}

cmd_t test_es_fibber_link(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_es_fibber_link_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

cmd_t test_bbp_cpld_version(cmd_t cmd)
{
    int times = 0;
    int ret = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }     
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(BOARD_TYPE_BBP != u8BoardType)
    {
        send_msg(cmd, "error bbp board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        char version[4] = {0};
        ret = bsp_bbp_cpld_read(u8BbpSlot, 0, version);
        ret += bsp_bbp_cpld_read(u8BbpSlot, 1, version+1);
        ret += bsp_bbp_cpld_read(u8BbpSlot, 2, version+2);
        ret += bsp_bbp_cpld_read(u8BbpSlot, 3, version+3);

        //cmd.pkg_datalen = htonl(4);
        //cmd.pkg_data[0] = version[0];
        //cmd.pkg_data[1] = version[1];
        //cmd.pkg_data[2] = version[2];
        //cmd.pkg_data[3] = version[3];

        send_msg(cmd, "基带板(slot %d)CPLD Version:%02x%02x%02x%02x\r\n",u8BbpSlot, version[0], version[1], version[2], version[3]);
        if(version[0]<0x10 || version[0]>0x24)
        {
            send_msg(cmd, "读基带板(slot %d)CPLD版本失败!(年%x)\r\n",u8BbpSlot, version[0]);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        else
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

void *test_bbp_cpldupdate_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(BOARD_TYPE_BBP != u8BoardType)
    {
        send_msg(cmd, "error bbp board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 200;
        boards[u8BbpSlot].cpld_updated = 0;
        if(bsp_bbp_cpld_update(u8BbpSlot)==BSP_OK)
        {
            while((boards[u8BbpSlot].cpld_updated != 1) && (wait_time-- > 0))
            {
                sleep(1);
            }
            sleep(1);
            if(boards[u8BbpSlot].cpld_updated == 1)
            {
                send_msg(cmd, "基带板(slot %d)CPLD更新成功!\r\n",u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "基带板(slot %d)CPLD更新失败!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "基带板(slot %d)CPLD更新失败!\r\n",u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbp_cpldupdate(cmd_t cmd)
{
    pthread_t tid;
    if(check_bbp_slot(cmd)!=0)
        return cmd;
    pthread_create(&tid, NULL, test_bbp_cpldupdate_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_bbp_fpgaload_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    u8 timewait = 200;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if((BOARD_TYPE_BBP == u8BoardType) || (BOARD_TYPE_ES == u8BoardType))
        {
            boards[u8BbpSlot].fpga_status = DSP_FPGA_STATE_NOLOAD;
            if(bsp_bbp_fpga_load(u8BbpSlot)==BSP_OK)
            {
                while(boards[u8BbpSlot].fpga_status!=  DSP_FPGA_STATE_LOADED && timewait-- >0)
                {
                    sleep(1);
                }
                sleep(1);
                if(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED)
                {
                    if(BOARD_TYPE_BBP == u8BoardType)
                        send_msg(cmd, "基带板(slot %d)FPGA加载成功!\r\n",u8BbpSlot);
                    if(BOARD_TYPE_ES == u8BoardType)
                        send_msg(cmd, "同步板(slot %d)FPGA加载成功!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if(BOARD_TYPE_BBP == u8BoardType)
                        send_msg(cmd, "基带板(slot %d)FPGA加载失败!\r\n",u8BbpSlot);
                    if(BOARD_TYPE_ES == u8BoardType)
                        send_msg(cmd, "同步板(slot %d)FPGA加载失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, "基带板(slot %d)FPGA加载失败!\r\n",u8BbpSlot);
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, "同步板(slot %d)FPGA加载失败!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            boards[u8BbpSlot].fpga_status &= ~FSA_FPGA_325T_LOADED;
            if(bsp_fsa_fpga_load(FSA_FPGA_325T, u8BbpSlot)==BSP_OK)
            {
                while(((boards[u8BbpSlot].fpga_status&FSA_FPGA_325T_LOADED)!= FSA_FPGA_325T_LOADED)&& (timewait-- >0))
                {
                    sleep(1);
                }
                sleep(1);
                if((boards[u8BbpSlot].fpga_status&FSA_FPGA_325T_LOADED)== FSA_FPGA_325T_LOADED)
                {
                    send_msg(cmd, "交换板(slot %d)FPGA_325T加载成功!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, "交换板(slot %d)FPGA_325T加载失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, "交换板(slot %d)FPGA_325T加载失败!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbp_fpgaload(cmd_t cmd)
{
    pthread_t tid;
    if(check_bbp_slot(cmd)!=0)
        return cmd;
    pthread_create(&tid, NULL, test_bbp_fpgaload_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_fsa_fpga160t_load_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    u8 timewait = 200;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_fsa_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(u8BoardType != BOARD_TYPE_FSA)
    {
        printf("error fsa board type(%d)\r\n", u8BoardType);
        send_result(cmd);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        boards[u8BbpSlot].fpga_status &= ~FSA_FPGA_160T_LOADED;
        if(bsp_fsa_fpga_load(FSA_FPGA_160T, u8BbpSlot)==BSP_OK)
        {
            while(((boards[u8BbpSlot].fpga_status&FSA_FPGA_160T_LOADED)!= FSA_FPGA_160T_LOADED)&& (timewait-- >0))
            {
                sleep(1);
            }
            sleep(1);
            if((boards[u8BbpSlot].fpga_status&FSA_FPGA_160T_LOADED)== FSA_FPGA_160T_LOADED)
            {
                send_msg(cmd, "交换板(slot %d)FPGA_160T加载成功!\r\n",u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "交换板(slot %d)FPGA_160T加载失败!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "交换板(slot %d)FPGA_160T加载失败!\r\n",u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_fsa_fpga160t_fpgaload(cmd_t cmd)
{
    pthread_t tid;
    if(check_bbp_slot(cmd)!=0)
        return cmd;
    pthread_create(&tid, NULL, test_fsa_fpga160t_load_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbp_mcuupdate_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 300;
        if(BOARD_TYPE_BBP == u8BoardType)
            bbp_boot_over = 0;
        memset(&boards[u8BbpSlot], 0, sizeof(board_info));
        if(bsp_bbp_mcu_update(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
            {
                send_msg(cmd, "基带板(slot %d)MCU更新...\r\n",u8BbpSlot);
                while((bbp_boot_over!=1) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(1);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
                    &&(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED)
                    &&(boards[u8BbpSlot].dsp_isready == 0x0F))
                {
                    send_msg(cmd, "基带板(slot %d)MCU更新成功!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, "基带板(slot %d)MCU更新失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            if(BOARD_TYPE_FSA == u8BoardType)
            {
                send_msg(cmd, "交换板(slot %d)MCU更新...\r\n",u8BbpSlot);
                while((boards[u8BbpSlot].fpga_status != 0x30) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(1);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)&&(boards[u8BbpSlot].fpga_status == 0x30))
                {
                    send_msg(cmd, "交换板(slot %d)MCU更新成功!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, "交换板(slot %d)MCU更新失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            if(BOARD_TYPE_ES == u8BoardType)
            {
                send_msg(cmd, "同步板(slot %d)MCU更新...\r\n",u8BbpSlot);
                while((boards[u8BbpSlot].fpga_status != DSP_FPGA_STATE_LOADED) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(1);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)&&(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED))
                {
                    send_msg(cmd, "同步板(slot %d)MCU更新成功\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, "同步板(slot %d)MCU更新失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "基带板(slot %d)MCU更新失败!\r\n",u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "交换板(slot %d)MCU更新失败!\r\n",u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "同步板(slot %d)MCU更新失败!\r\n",u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}

cmd_t test_bbp_mcuupdate(cmd_t cmd)
{
    pthread_t tid;
    if(check_bbp_slot(cmd)!=0)
        return cmd;
    pthread_create(&tid, NULL, test_bbp_mcuupdate_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_bbp_reset_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 300;
        if(BOARD_TYPE_BBP == u8BoardType)
            bbp_boot_over = 0;
        memset(&boards[u8BbpSlot], 0, sizeof(board_info));
        if(bsp_bbp_reset(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
            {
                send_msg(cmd, "基带板(slot %d)复位中...\r\n",u8BbpSlot);
                while((bbp_boot_over != 1) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(2);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
                    &&(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED)
                    &&(boards[u8BbpSlot].dsp_isready == 0x0F))
                {
                    send_msg(cmd, "基带板(slot %d)启动正常!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((boards[u8BbpSlot].mcu_status&0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                    {
                        send_msg(cmd, " 基带板(slot %d)bbp启动失败!\r\n",u8BbpSlot);
                    }
                    if(boards[u8BbpSlot].fpga_status != DSP_FPGA_STATE_LOADED)
                    {
                        send_msg(cmd, " 基带板(slot %d)fpga加载失败!\r\n",u8BbpSlot);
                    }
                    if(boards[u8BbpSlot].dsp_isready != 0x0F)
                    {
                        if(boards[u8BbpSlot].dsp_isready != 0x1)
                        {
                            send_msg(cmd, " 基带板(slot %d)DSP1加载失败!\r\n",u8BbpSlot);
                        }
                        if(boards[u8BbpSlot].dsp_isready != 0x2)
                        {
                            send_msg(cmd, " 基带板(slot %d)DSP2加载失败!\r\n",u8BbpSlot);
                        }
                        if(boards[u8BbpSlot].dsp_isready != 0x3)
                        {
                            send_msg(cmd, " 基带板(slot %d)DSP3加载失败!\r\n",u8BbpSlot);
                        }
                        if(boards[u8BbpSlot].dsp_isready != 0x4)
                        {
                            send_msg(cmd, " 基带板(slot %d)DSP4加载失败!\r\n",u8BbpSlot);
                        }
                        send_msg(cmd, " 基带板(slot %d)DSP加载失败!\r\n",u8BbpSlot);
                    }
                    send_msg(cmd, "基带板(slot %d)启动失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            if(BOARD_TYPE_FSA == u8BoardType)
            {
                send_msg(cmd, "交换板(slot %d)复位中...\r\n",u8BbpSlot);
                while((boards[u8BbpSlot].fpga_status != 0x30) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(2);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)&&(boards[u8BbpSlot].fpga_status == 0x30))
                {
                    send_msg(cmd, "交换板(slot %d)启动正常!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((boards[u8BbpSlot].mcu_status&0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                    {
                        send_msg(cmd, " 交换板(slot %d)启动失败!\r\n",u8BbpSlot);
                    }
                    if((boards[u8BbpSlot].fpga_status&FSA_FPGA_325T_LOADED) != FSA_FPGA_325T_LOADED)
                    {
                        send_msg(cmd, " 交换板(slot %d)fpga_325t加载失败!\r\n",u8BbpSlot);
                    }
                    if((boards[u8BbpSlot].fpga_status&FSA_FPGA_160T_LOADED) != FSA_FPGA_160T_LOADED)
                    {
                        send_msg(cmd, " 交换板(slot %d)fpga_160t加载失败!\r\n",u8BbpSlot);
                    }
                    send_msg(cmd, "交换板(slot %d)启动失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            if(BOARD_TYPE_ES == u8BoardType)
            {
                send_msg(cmd, "同步板(slot %d)复位中...\r\n",u8BbpSlot);
                while((boards[u8BbpSlot].fpga_status != DSP_FPGA_STATE_LOADED) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(2);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)&&(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED))
                {
                    send_msg(cmd, "同步板(slot %d)启动正常!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((boards[u8BbpSlot].mcu_status&0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                    {
                        send_msg(cmd, " 同步板(slot %d)启动失败!\r\n",u8BbpSlot);
                    }
                    if(boards[u8BbpSlot].fpga_status != DSP_FPGA_STATE_LOADED)
                    {
                        send_msg(cmd, " 同步板(slot %d)fpga加载失败!\r\n",u8BbpSlot);
                    }
                    send_msg(cmd, "同步板(slot %d)启动失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "基带板(slot %d)复位失败!\r\n",u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "交换板(slot %d)复位失败!\r\n",u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "同步板(slot %d)复位失败!\r\n",u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbp_reset(cmd_t cmd)
{
    pthread_t tid;
    if(check_bbp_slot(cmd)!=0)
        return cmd;
    pthread_create(&tid, NULL, test_bbp_reset_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbp_dsp_load_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;
    
    //获取槽位号
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    } 
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    if(BOARD_TYPE_BBP != u8BoardType)
    {
        send_msg(cmd, "error bbp board type(%d).\r\n", u8BoardType);
        send_result(cmd);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        boards[u8BbpSlot].dsp_isready = 0;
        if(bsp_boot_all_dsp(u8BbpSlot)==BSP_OK)
        {
            sleep(2);
            if(boards[u8BbpSlot].dsp_isready == 0xf)
            {
                send_msg(cmd, " 基带板(slot %d)DSP加载成功!\r\n",u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(boards[u8BbpSlot].dsp_isready != 0x1)
                {
                    send_msg(cmd, " 基带板(slot %d)DSP1加载失败-no ready!\r\n",u8BbpSlot);
                }
                 if(boards[u8BbpSlot].dsp_isready != 0x2)
                {
                    send_msg(cmd, " 基带板(slot %d)DSP2加载失败-no ready!\r\n",u8BbpSlot);
                }
                 if(boards[u8BbpSlot].dsp_isready != 0x3)
                {
                    send_msg(cmd, " 基带板(slot %d)DSP3加载失败-no ready!\r\n",u8BbpSlot);
                }
                 if(boards[u8BbpSlot].dsp_isready != 0x4)
                {
                    send_msg(cmd, " 基带板(slot %d)DSP4加载失败-no ready!\r\n",u8BbpSlot);
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 基带板(slot %d)DSP加载失败!\r\n",u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbp_dsp_load(cmd_t cmd)
{
    pthread_t tid;
    if(check_bbp_slot(cmd)!=0)
        return cmd;
    pthread_create(&tid, NULL, test_bbp_dsp_load_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

/*****************风扇板测试项***************************/
void *fan_reset_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    uint8_t u8Slot = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_fan_slot(cmd) != 0)
    {
        send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 100;
        memset(&boards[u8Slot], 0, sizeof(board_info));
        if(bsp_hmi_board_reboot(BOARD_TYPE_FAN,10)==BSP_OK)
        {
            send_msg(cmd, " 风扇板复位中...\r\n");
            while(((boards[u8Slot].mcu_status & 0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                &&(wait_time-- > 0))
            {
                sleep(1);
            }
            sleep(1);
            if((boards[u8Slot].mcu_status & 0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, " 风扇板启动正常!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 风扇板启动失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 风扇板复位失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_fan_reset(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, fan_reset_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *fan_mcuupdate_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_fan_slot(cmd) != 0)
    {
        send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
        return NULL;
    }
   bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 120;
        memset(&boards[u8Slot], 0, sizeof(board_info));
        if(bsp_hmi_mcu_update(BOARD_TYPE_FAN,10)==BSP_OK)
        {
            send_msg(cmd, "风扇板(slot %d)MCU更新...\r\n",u8Slot);
            while(((boards[u8Slot].mcu_status & 0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                &&(wait_time-- > 0))
            {
                sleep(1);
            }
            sleep(1);
            if((boards[u8Slot].mcu_status & 0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "风扇板(slot %d)MCU更新成功!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "风扇板(slot %d)MCU更新失败!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "风扇板(slot %d)MCU更新失败!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_fan_mcuupdate(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, fan_mcuupdate_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_fan_speed_set_thread(void *arg)
{
    int times = 0;
    int fanchannel;
    int fanspeed;
    cmd_t cmd = *(cmd_t*)arg;
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot != IPMB_SLOT10)
    {
        send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        g_fan_speed_set = 0;
        fanchannel = htonl(cmd.pkg_data[1]);
        fanspeed = htonl(cmd.pkg_data[2]);
        if(bsp_hmi_fan_speed(0, fanchannel, fanspeed)==BSP_OK)
        {
            while((g_fan_speed_set == 0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_fan_speed_set)
            {
                send_msg(cmd, " 风扇板风扇转速控制成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 风扇板风扇转速控制失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 风扇板风扇转速设置失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_fan_speed_set(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_fan_speed_set_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_fan_speed_get_thread(void *arg)
{
    int times = 0;
    int fanchannel;
    cmd_t cmd = *(cmd_t*)arg;
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot != IPMB_SLOT10)
    {
        send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        g_fan_speed_get = 0;
        fanchannel = htonl(cmd.pkg_data[1]);
        if(bsp_hmi_fan_speed(1, fanchannel, 0)==BSP_OK)
        {
            while((g_fan_speed_get == 0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_fan_speed_get)
            {
                if(fanchannel == 0)
                    send_msg(cmd, "转速(channel 0) = %d.\r\n",g_fan1_speed);
                if(fanchannel == 1)
                    send_msg(cmd, "转速(channel 1) = %d.\r\n",g_fan2_speed);
                if(fanchannel == 2)
                    send_msg(cmd, "转速(channel 2) = %d.\r\n",g_fan3_speed);
                if(fanchannel == 3)
                    send_msg(cmd, "转速(channel 0~2) = %d %d %d.\r\n",
                             g_fan1_speed, g_fan2_speed, g_fan3_speed);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 风扇板风扇转速获取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 风扇板风扇转速获取失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_fan_speed_get(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_fan_speed_get_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_fan_eeprom_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_fan_slot(cmd) != 0)
    {
        send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        g_fan_eeprom_test = 0;
        if(bsp_hmi_test_eeprom(BOARD_TYPE_FAN, 10)==BSP_OK)
        {
            while((g_fan_eeprom_test == 0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_fan_eeprom_test)
            {
                send_msg(cmd, "风扇板EEPROM测试成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "风扇板EEPROM测试失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "风扇板EEPROM测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_fan_eeprom(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_fan_eeprom_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_fan_version_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_fan_slot(cmd) != 0)
    {
        send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        memset(&boards[u8Slot].arm_version, 0, 64);
        if(bsp_hmi_get_arm_ver(BOARD_TYPE_FAN, 10)==BSP_OK)
        {
            while((strlen(boards[u8Slot].arm_version) == 0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if((strlen(boards[u8Slot].arm_version) != 0))
            {
                send_msg(cmd, "风扇板arm version = %s\r\n", boards[IPMB_SLOT10].arm_version);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "风扇板arm version获取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "风扇板arm version获取失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_fan_version(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_fan_version_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

#define FAN_SPEED_HIGH 6000
void *test_fan_test_thread(void *arg)
{
    int times = 0;
    int fanchannel;
    int fanspeed;
    cmd_t cmd = *(cmd_t*)arg;

    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        g_fan_speed_set = 0;
        //设置最大转速
        if(bsp_hmi_fan_speed(0, 3, 99)==BSP_OK)
        {
            while((g_fan_speed_set == 0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_fan_speed_set)
            {
                send_msg(cmd, "风扇转速设置为最大转速!\r\n");
            }
            else
            {
                send_msg(cmd, "风扇最大转速设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                goto END;
            }
            /* 等待20s回读转速，判断设置转速是否成功 */
            sleep(20);
            wait_time = 2000;
            g_fan_speed_get = 0;
            if(bsp_hmi_fan_speed(1, 3, 0)==BSP_OK)
            {
                while((g_fan_speed_get == 0)&&(wait_time-- > 0))
                {
                    usleep(1000);
                }
                if(g_fan_speed_get)
                {
                    send_msg(cmd, "获取风扇转速成功!\r\n");
                    send_msg(cmd, "获取通道1最大风扇转速=%d!\r\n",g_fan1_speed);
                    send_msg(cmd, "获取通道2最大风扇转速=%d!\r\n",g_fan2_speed);
                    send_msg(cmd, "获取通道3最大风扇转速=%d!\r\n",g_fan3_speed);
                    if((g_fan1_speed < (FAN_SPEED_HIGH)) ||
                            (g_fan2_speed < (FAN_SPEED_HIGH)) ||
                            (g_fan3_speed < (FAN_SPEED_HIGH)))
                    {
                        cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                        goto END;
                    }
                }
                else
                {
                    send_msg(cmd, "获取风扇转速失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                    goto END;
                }
            }
            else
            {
                send_msg(cmd, "风扇转速获取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                goto END;
            }
        }
        else
        {
            send_msg(cmd, "风扇板风扇转速设置失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            goto END;
        }
        cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
END:
        if(cmd.pkg_failtimes != 0)
        {
            send_msg(cmd, "风扇板测试失败!\r\n");
        }
        /* 设置风扇转速为60% */
        bsp_hmi_fan_speed(0,3,60);
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }

    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_fan_test(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_fan_test_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

/*****************监控板测试项***************************/
void *peu_reset_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8Slot = 0;

     //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_peu_slot(cmd) != 0)
    {
        send_msg(cmd, "error peu slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        memset(&boards[u8Slot], 0, sizeof(board_info));
        if(bsp_hmi_board_reboot(BOARD_TYPE_PEU,u8Slot)==BSP_OK)
        {
            send_msg(cmd, " 监控板(slot %d)复位中...\r\n",u8Slot);
            while(((boards[u8Slot].mcu_status & 0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                &&(wait_time-- > 0))
            {
                usleep(1000);
            }
            sleep(1);
            if((boards[u8Slot].mcu_status & 0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, " 监控板(slot %d)启动正常!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 监控板(slot %d)启动失败!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 监控板(slot %d)复位失败!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_peu_reset(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, peu_reset_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *peu_mcuupdate_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_peu_slot(cmd) != 0)
    {
        send_msg(cmd, "error peu slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 120;
        memset(&boards[u8Slot], 0, sizeof(board_info));
        if(bsp_hmi_mcu_update(BOARD_TYPE_PEU,u8Slot)==BSP_OK)
        {
            send_msg(cmd, "监控板(slot %d)MCU更新...\r\n",u8Slot);
            while(((boards[u8Slot].mcu_status & 0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                &&(wait_time-- > 0))
            {
                sleep(1);
            }
            sleep(1);
            if((boards[u8Slot].mcu_status & 0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "监控板(slot %d)MCU更新成功!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "监控板(slot %d)MCU更新失败!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "监控板(slot %d)MCU更新失败!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_peu_mcuupdate(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, peu_mcuupdate_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *peu_power_down_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8Slot = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_peu_slot(cmd) != 0)
    {
        send_msg(cmd, "error peu slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        g_peu_power_down_flag = 0;
        if(bsp_hmi_board_power_on_off(BOARD_TYPE_PEU,0,u8Slot)==BSP_OK)
        {
            send_msg(cmd, "监控板(slot %d)整机下电中...\r\n",u8Slot);
            while((g_peu_power_down_flag==0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_peu_power_down_flag==1)
            {
                send_msg(cmd, "监控板(slot %d)整机下电成功!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "监控板(slot %d)整机下电失败!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "监控板(slot %d)整机下电失败!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_peu_power_down(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, peu_power_down_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *peu_dryin_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_peu_slot(cmd) != 0)
    {
        send_msg(cmd, "error peu slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        g_peu_get_dryin = 0;
        if(bsp_hmi_get_dryin_state(u8Slot)==BSP_OK)
        {
            while((g_peu_get_dryin==0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_peu_get_dryin==1)
            {
                send_msg(cmd, " 监控板(slot %d)干结点状态dryin0123=0x%x, dryin4567=0x%x!\r\n",
                         u8Slot, g_peu_dryin0123, g_peu_dryin4567);
                if((g_peu_dryin0123 == 0) && (g_peu_dryin4567 == 0))
                {
                    send_msg(cmd, " 监控板(slot %d)干接点测试成功!\r\n", u8Slot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((g_peu_dryin0123 & 0x01) == 0x01)                        
                        send_msg(cmd, " 监控板(slot %d)干接点0测试失败!\r\n", u8Slot);
                    if((g_peu_dryin0123 & 0x02) == 0x02)                        
                        send_msg(cmd, " 监控板(slot %d)干接点1测试失败!\r\n", u8Slot);
                    if((g_peu_dryin0123 & 0x04) == 0x04)                        
                        send_msg(cmd, " 监控板(slot %d)干接点2测试失败!\r\n", u8Slot);
                    if((g_peu_dryin0123 & 0x08) == 0x08)                        
                        send_msg(cmd, " 监控板(slot %d)干接点3测试失败!\r\n", u8Slot);
                    if((g_peu_dryin4567 & 0x01) == 0x01)                        
                        send_msg(cmd, " 监控板(slot %d)干接点4测试失败!\r\n", u8Slot);
                    if((g_peu_dryin4567 & 0x02) == 0x02)                        
                        send_msg(cmd, " 监控板(slot %d)干接点5测试失败!\r\n", u8Slot);
                    if((g_peu_dryin4567 & 0x04) == 0x04)                        
                        send_msg(cmd, " 监控板(slot %d)干接点6测试失败!\r\n", u8Slot);
                    if((g_peu_dryin4567 & 0x08) == 0x08)                        
                        send_msg(cmd, " 监控板(slot %d)干接点7测试失败!\r\n", u8Slot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 监控板(slot %d)干结点状态获取超时!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 监控板(slot %d)干结点状态获取失败!!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_peu_dryin(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, peu_dryin_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *peu_temperature_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_peu_slot(cmd) != 0)
    {
        send_msg(cmd, "error peu slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        g_peu_get_temp = 0;

        if(bsp_hmi_get_temperature(BOARD_TYPE_PEU,u8Slot)==BSP_OK)
        {
            while((g_peu_get_temp==0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_peu_get_temp==1)
            {
                send_msg(cmd, " 监控板(slot %d)温度=%d.\r\n",u8Slot, g_s16PEUTemp);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 监控板(slot %d)温度获取失败!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 监控板(slot %d)温度获取失败!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_peu_temperature(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, peu_temperature_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *peu_rs485_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_peu_slot(cmd) != 0)
    {
        send_msg(cmd, "error peu slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        g_peu_rs485_test = 0;

        if(bsp_hmi_rs485_test(u8Slot)==BSP_OK)
        {
            while((g_peu_rs485_test==0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_peu_rs485_test==1)
            {
                send_msg(cmd, " 监控板(slot %d) RS485测试成功!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 监控板(slot %d) RS485测试失败!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 监控板(slot %d) RS485测试失败!!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_peu_rs485(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, peu_rs485_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_peu_version_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(check_peu_slot(cmd) != 0)
    {
        send_msg(cmd, "error peu slot id(%d).\r\n", u8Slot);
        return NULL;
    }    
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 2000;
        memset(&boards[u8Slot].arm_version, 0, 64);
        if(bsp_hmi_get_arm_ver(BOARD_TYPE_PEU, u8Slot)==BSP_OK)
        {
            while((strlen(boards[u8Slot].arm_version) == 0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(strlen(boards[u8Slot].arm_version) != 0)
            {
                send_msg(cmd, "监控板(slot %d)arm version = %s\r\n", u8Slot,boards[u8Slot].arm_version);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "监控板(slot %d)arm version获取失败!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "监控板(slot %d)arm version获取失败!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_peu_version(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_peu_version_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}


/*****************板间测试项***************************/
void *test_mct_bbp_hmi_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;

    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mctbbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct or bbp slot id.\r\n");
        bsp_mutex_unlock(cmd_list[cmd.index].plock);
        return NULL;
    }
    //获取槽位号
    u8BbpSlot = bsp_get_mctbbp_bbp_slot(cmd);
    //获取板类型
    u8BoardType = boards[u8BbpSlot].type;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 10;
        g_bbp_hmi_test = 0;
        if(BOARD_TYPE_BBP == u8BoardType)
        {
            if(bsp_hmi_board_test(BOARD_TYPE_BBP, u8BbpSlot)==BSP_OK)
            {
                while((g_bbp_hmi_test == 0)&&(wait_time-- > 0))
                {
                    usleep(300000);
                }
                if(g_bbp_hmi_test)
                {
                    send_msg(cmd, " 基带板(slot %d)HMI测试接收成功!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 基带板(slot %d)HMI测试接收失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 基带板(slot %d)HMI测试发送失败!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            if(bsp_hmi_board_test(BOARD_TYPE_FSA, u8BbpSlot)==BSP_OK)
            {
                while((g_bbp_hmi_test == 0)&&(wait_time-- > 0))
                {
                    usleep(300000);
                }
                if(g_bbp_hmi_test)
                {
                    send_msg(cmd, " FSA板(slot %d)HMI测试接收成功!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " FSA板(slot %d)HMI测试接收失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " FSA板(slot %d)HMI测试发送失败!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        if(BOARD_TYPE_ES == u8BoardType)
        {
            if(bsp_hmi_board_test(BOARD_TYPE_ES, u8BbpSlot)==BSP_OK)
            {
                while((g_bbp_hmi_test == 0)&&(wait_time-- > 0))
                {
                    usleep(300000);
                }
                if(g_bbp_hmi_test)
                {
                    send_msg(cmd, " ES板(slot %d)HMI测试接收成功!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ES板(slot %d)HMI测试接收失败!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ES板(slot %d)HMI测试发送失败!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}

cmd_t test_mct_bbp_hmi(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_mct_bbp_hmi_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_mct_ges_hmi_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8Slot;
    u8Slot = bsp_get_mctbbp_bbp_slot(cmd);
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 10;
        g_ges_hmi_test = 0;
        if(bsp_hmi_board_test(BOARD_TYPE_FSA,u8Slot)==BSP_OK)
        {
            while((g_ges_hmi_test == 0)&&(wait_time-- > 0))
            {
                usleep(300000);
            }
            if(g_ges_hmi_test)
            {
                send_msg(cmd, " 交换板HMI测试接收成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 交换板HMI测试接收失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 交换板HMI测试发送失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}

cmd_t test_mct_ges_hmi(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_mct_ges_hmi_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_mct_fan_hmi_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mctfan_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct or fan slot id.\r\n");
        bsp_mutex_unlock(cmd_list[cmd.index].plock);
        return NULL;
    }
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 10;
        g_fan_hmi_test = 0;
        if(bsp_hmi_board_test(BOARD_TYPE_FAN,IPMB_SLOT10)==BSP_OK)
        {
            while((g_fan_hmi_test == 0) && (wait_time-->0))
            {
                usleep(300000);
            }
            if(g_fan_hmi_test)
            {
                send_msg(cmd, " 风扇板HMI测试接收成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 风扇板HMI测试接收失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 风扇板HMI测试发送失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}

cmd_t test_mct_fan_hmi(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_mct_fan_hmi_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_mct_peu_hmi_thread(void *arg)
{
    int times = 0;
    u8 u8Slot = 0;
    cmd_t cmd = *(cmd_t*)arg;
    
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mctpeu_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct or peu slot id.\r\n");
        bsp_mutex_unlock(cmd_list[cmd.index].plock);
        return NULL;
    }
    u8Slot = cmd.pkg_data[1];
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int wait_time = 10;
        g_peu_hmi_test = 0;
        if(bsp_hmi_board_test(BOARD_TYPE_PEU,u8Slot)==BSP_OK)
        {
            while((g_peu_hmi_test == 0) && (wait_time-->0))
            {
                usleep(300000);
            }
            if(g_peu_hmi_test)
            {
                send_msg(cmd, " 监控板(slot %d)HMI测试接收成功!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 监控板(slot %d)HMI测试接收失败!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " 监控板HMI(slot %d)测试发送失败!\r\n",u8Slot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}

cmd_t test_mct_peu_hmi(cmd_t cmd)
{
    pthread_t tid;    
    pthread_create(&tid, NULL, test_mct_peu_hmi_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

const char *afc_status[] =
{
    "自由振荡",
    "自由振荡",
    "自由振荡",
    "锁定",
    "保持",
    "保持超时",
    "异常"
};
cmd_t *test_afc_thread(void *arg)
{
    int times = 0;
    unsigned short afclock = -1;

    cmd_t cmd = *(cmd_t*)arg;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        bsp_mutex_unlock(cmd_list[cmd.index].plock);
        return NULL;
    }
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        afclock = afc_lock_status_get();
        if(afclock < 7)
        {
            send_msg(cmd, " AFC状态获取测试成功!\r\n");
            send_msg(cmd, " AFC状态:%s \r\n",  afc_status[afclock]);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " AFC状态获取测试失败!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_afc(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_afc_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void bsp_check_subboard_readystatus(cmd_t cmd)
{
    int bid = 0;
    for(bid=2; bid<11; bid++)
    {
        if(boards[bid].type == BOARD_TYPE_BBP)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)bbp板卡启动成功!\r\n",bid);
                if(boards[bid].fpga_status== DSP_FPGA_STATE_LOADED)
                {
                    send_msg(cmd, "SLOT(%d) bbp板卡fpga加载成功!\r\n",bid);
                }
                else
                {
                    send_msg(cmd, "SLOT(%d) bbp板卡fpga加载 失败!\r\n",bid);
                }
                if(boards[bid].dsp_isload == DSP_FPGA_STATE_LOADED)
                {
                    send_msg(cmd, "SLOT(%d) bbp板卡dsp加载成功!\r\n",bid);
                }
                else
                {
                    send_msg(cmd, "SLOT(%d) bbp板卡dsp加载 失败!\r\n",bid);
                }
                if(boards[bid].dsp_isready == 0xf)
                {
                    send_msg(cmd, "SLOT(%d) bbp板卡dsp all ready!\r\n",bid);
                }
                else
                {
                    if((boards[bid].dsp_isready & 0x1) == 0x1)
                    {
                        send_msg(cmd, "SLOT(%d) bbp板卡dsp1 ready!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) bbp板卡dsp1 not ready!\r\n",bid);
                    }
                    if((boards[bid].dsp_isready & 0x2) == 0x2)
                    {
                        send_msg(cmd, "SLOT(%d) bbp板卡dsp2 ready!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) bbp板卡dsp2 not ready!\r\n",bid);
                    }
                    if((boards[bid].dsp_isready & 0x4) == 0x4)
                    {
                        send_msg(cmd, "SLOT(%d) bbp板卡dsp3 ready!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) bbp板卡dsp3 not ready!\r\n",bid);
                    }
                    if((boards[bid].dsp_isready & 0x8) == 0x8)
                    {
                        send_msg(cmd, "SLOT(%d) bbp板卡dsp4 ready!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) bbp板卡dsp4 not ready!\r\n",bid);
                    }
                }
            }
            else
            {
                send_msg(cmd, "SLOT(%d)bbp板卡启动失败!\r\n",bid);
            }
        }
        else if(boards[bid].type == BOARD_TYPE_FSA)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)fsa板卡启动成功!\r\n",bid);
                if(boards[bid].fpga_status == 0x30)
                {
                    send_msg(cmd, "SLOT(%d) fsa板卡fpga加载成功!\r\n",bid);
                }
                else
                {
                    if((boards[bid].fpga_status&FSA_FPGA_325T_LOADED) != FSA_FPGA_325T_LOADED)
                    {
                        send_msg(cmd, "SLOT(%d) fsa板卡fpga_325t加载失败!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) fsa板卡fpga_325t加载成功!\r\n",bid);
                    }
                    if((boards[bid].fpga_status&FSA_FPGA_160T_LOADED) != FSA_FPGA_160T_LOADED)
                    {
                        send_msg(cmd, "SLOT(%d) fsa板卡fpga_160t加载失败!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) fsa板卡fpga_160t加载成功!\r\n",bid);
                    }
                }
            }
            else
            {
                send_msg(cmd, "SLOT(%d)fsa板卡启动失败!\r\n",bid);
            }
        }
        else if(boards[bid].type == BOARD_TYPE_ES)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)es板卡启动成功!\r\n",bid);
                if(boards[bid].fpga_status == DSP_FPGA_STATE_LOADED)
                {
                    send_msg(cmd, "SLOT(%d) es板卡fpga加载成功!\r\n",bid);
                }
                else
                {
                    send_msg(cmd, "SLOT(%d) es板卡fpga加载 失败!\r\n",bid);
                }
            }
            else
            {
                send_msg(cmd, "SLOT(%d)fsa板卡启动失败!\r\n",bid);
            }
        }
        else if(boards[bid].type == BOARD_TYPE_FAN)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)fan板卡启动成功!\r\n",bid); 
            }
            else
            {
                send_msg(cmd, "SLOT(%d)fan板卡启动失败!\r\n",bid);
            }
        }
        else if(boards[bid].type == BOARD_TYPE_PEU)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)peu板卡启动成功!\r\n",bid);
            }
            else
            {
                send_msg(cmd, "SLOT(%d)peu板卡启动失败!\r\n",bid);
            }
        }
    }
}

cmd_t *test_board_ready_thread(void *arg)
{
    int times = 0;

    cmd_t cmd = *(cmd_t*)arg;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        bsp_check_subboard_readystatus(cmd);

        send_msg(cmd, "板卡启动查询成功!\r\n");
        cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_board_ready(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_board_ready_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

unsigned int read_long_frame_from_fpga(void)
{
    uint16_t val_frame_h = 0;
    uint16_t val_frame_l = 0;
    bsp_bbp_fpga_read(2, 192, &val_frame_l);
    bsp_bbp_fpga_read(2, 193, &val_frame_h);
    return (val_frame_h<<16) | val_frame_l;
}
unsigned frame_read_delay = 10000;
unsigned frame_write_delay = 0;
void bsp_write_long_frame(unsigned int frame)
{
    unsigned short ustmp0=0;
    unsigned short ustmp1=0;
    bsp_cpld_write_reg(58,0);
    ustmp0 = (frame & 0xffff);
    ustmp1 = (frame & 0x0fff0000)>>16;
    bsp_cpld_write_reg(54,(ustmp1 & 0x0f00)>>8);
    usleep(frame_write_delay);
    bsp_cpld_write_reg(55,(ustmp1 & 0xff));
    usleep(frame_write_delay);
    bsp_cpld_write_reg(56,(ustmp0 & 0xff00) >>8);
    usleep(frame_write_delay);
    bsp_cpld_write_reg(57,(ustmp0 & 0xff));
    usleep(frame_write_delay);
    bsp_cpld_write_reg(58,1);
}
static unsigned int long_frame_number = 0x01234567;
cmd_t test_sync(cmd_t cmd)
{
    int times = 0;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    if(check_mct_slot(cmd)!=0)
    {
        send_msg(cmd, "error mct slot id.\r\n");
        return cmd;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        unsigned int read_frame = 0;
        //	if(AFC_LOCK != afc_lock_status_get()){
        //		send_msg(cmd, " 同步测试失败！AFC状态0x%x!\r\n", afc_lock_status_get());
        //       cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        //	}else
        bsp_bbp_fpga_write(2,192, 0x1);
        long_frame_number = (long_frame_number+0x1111111) & 0xFFFFFFF;
        bsp_write_long_frame(long_frame_number);
        usleep(frame_read_delay);
        read_frame = read_long_frame_from_fpga();
        if(long_frame_number==read_frame)
        {
            send_msg(cmd, " 同步测试成功！\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " 同步测试失败！0x%x<->0x%x\r\n", long_frame_number,read_frame);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

/*****************EEPROM配置/读取测试项***************************/
extern EEPROM_PAR_STRU g_mca_eeprom_par;
extern EEPROM_PAR_STRU g_bbp_eeprom_par;
extern EEPROM_PAR_STRU g_fan_eeprom_set_par;
extern EEPROM_PAR_STRU g_fan_eeprom_get_par;

void *test_bbu_set_crc_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        u16 crc = 0;

        if(len != 2)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        
        crc = (cmd.pkg_data[2]<<8)|cmd.pkg_data[1];
        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_eeprom_set_crc(crc)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM CRC设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM CRC设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_crc(u8Slot, crc)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM CRC设置成功!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM CRC设置成功!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM CRC设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM CRC设置失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM CRC设置失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM CRC设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_set_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            memcpy((u8 *)&g_fan_eeprom_set_par.checkSum, (u8 *)&crc, 2);
            if(bsp_hmi_fan_eeprom_crc(0)==BSP_OK)
            {
                while((g_fan_set_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_set_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM CRC设置成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM CRC设置失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM CRC设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_crc(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_crc_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_device_id_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        s8 deviceid[16] = {0};

        if(len == 0)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        if(len > 16)
        {
            len = 16;
        }
        memcpy(deviceid, (s8 *)(cmd.pkg_data+1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_set_bbu_deviceid(deviceid)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM DeviceID设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM DeviceID设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_deviceid(u8Slot, deviceid)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM DeviceID设置成功!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM DeviceID设置成功!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM DeviceID设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {                
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM DeviceID设置失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM DeviceID设置失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM DeviceID设置失败!\r\n");                
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_set_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            memcpy(g_fan_eeprom_set_par.device_id, deviceid, len);
            if(bsp_hmi_fan_eeprom_device_id(0)==BSP_OK)
            {
                while((g_fan_set_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_set_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM DeviceID设置成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM DeviceID设置失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM DeviceID设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_device_id(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_device_id_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_board_type_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        s8 boardtype[32] = {0};

        if(len == 0)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        if(len > 32)
        {
            len = 32;
        }
        memcpy(boardtype, (s8 *)(cmd.pkg_data + 1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_set_bbu_boardtype(boardtype)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM BoardType设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM BoardType设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_boardtype(u8Slot, boardtype)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)                    
                    send_msg(cmd, " 基带板EEPROM BoardType设置成功!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)                    
                    send_msg(cmd, " 交换板EEPROM BoardType设置成功!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)                    
                    send_msg(cmd, " 增强板EEPROM BoardType设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType) 
                    send_msg(cmd, " 基带板EEPROM BoardType设置失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType) 
                    send_msg(cmd, " 交换板EEPROM BoardType设置失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType) 
                    send_msg(cmd, " 增强板EEPROM BoardType设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_set_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            memcpy(g_fan_eeprom_set_par.board_type, boardtype, len);
            if(bsp_hmi_fan_eeprom_board_type(0)==BSP_OK)
            {
                while((g_fan_set_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_set_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM BoardType设置成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM BoardType设置失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM BoardType设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_board_type(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_board_type_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_mac_addr1_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot != bsp_get_slot_id())
    {
        send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        u8 macaddr1[6] = {0};

        if(len != 6)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        memcpy(macaddr1, (u8 *)(cmd.pkg_data + 1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if(bsp_set_bbu_macaddr1(macaddr1)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM MACADDR1设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM MACADDR1设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_mac_addr1(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_mac_addr1_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_mac_addr2_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot != bsp_get_slot_id())
    {
        send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        u8 macaddr2[6] = {0};

        if(len != 6)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        memcpy(macaddr2, (u8 *)(cmd.pkg_data + 1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if(bsp_set_bbu_macaddr2(macaddr2)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM MACADDR2设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM MACADDR2设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_mac_addr2(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_mac_addr2_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_eth3_addr_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        s8 buf[100] = "";
        struct sockaddr_in addr = {AF_INET};
        addr.sin_addr.s_addr = *(u32 *)cmd.pkg_data;
        sprintf(buf, "ifconfig eth3 %s up", inet_ntoa(addr.sin_addr));
        system(buf);

        sprintf(buf, "echo \"ifconfig eth3 %s up\" > /mnt/btsa/cfg.txt", inet_ntoa(addr.sin_addr));
        system(buf);

        send_msg(cmd, " 主控板ETH3 IP设置成功!\r\n");
        cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;

        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
void test_set_eth3_addr(char *ip)
{
    char buf[20] = "";
    cmd_t cmd = {0};
    cmd.pkg_totaltimes = 1;
    if(ip==NULL)
        return;
    memcpy(cmd.pkg_data, ip, strlen(ip));
    test_bbu_set_eth3_addr_thread((void*)&cmd);
}

cmd_t test_bbu_set_eth3_addr(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_eth3_addr_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}


void *test_bbu_set_product_sn_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        u8 productsn[32] = {0};

        if(len == 0)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        if(len > 32)
        {
            len = 32;
        }
        memcpy(productsn, (s8 *)(cmd.pkg_data + 1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_set_bbu_productsn(productsn)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM ProductSN设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM ProductSN设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_productsn(u8Slot, productsn)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM ProductSN设置成功!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM ProductSN设置成功!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM ProductSN设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM ProductSN设置失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM ProductSN设置失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM ProductSN设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_set_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            memcpy(g_fan_eeprom_set_par.product_sn, productsn, len);
            if(bsp_hmi_fan_eeprom_product_sn(0)==BSP_OK)
            {
                while((g_fan_set_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_set_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM ProductSN设置成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM ProductSN设置失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM ProductSN设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_product_sn(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_product_sn_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_manufacturer_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        s8 manufacturer[12] = {0};

        if(len == 0)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        if(len > 12)
        {
            len = 12;
        }
        memcpy(manufacturer, (s8 *)(cmd.pkg_data + 1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_set_bbu_manufacturer(manufacturer)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM Manufacture设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM Manufacture设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_manufacturer(u8Slot, manufacturer)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM Manufacture设置成功!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM Manufacture设置成功!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM Manufacture设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM Manufacture设置失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM Manufacture设置失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM Manufacture设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_set_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            memcpy(g_fan_eeprom_set_par.manufacturer, manufacturer, len);
            if(bsp_hmi_fan_eeprom_manufacture(0)==BSP_OK)
            {
                while((g_fan_set_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_set_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM Manufacture设置成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM Manufacture设置失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM Manufacture设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_manufacturer(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_manufacturer_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_product_date_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        u8 productdate[4] = {0};
        
        if(len != 4)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        memcpy(productdate, (u8 *)(cmd.pkg_data+1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_set_bbu_productdate(productdate)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM ProductDate设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM ProductDate设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_productdate(u8Slot, productdate)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM ProductDate设置成功!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM ProductDate设置成功!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM ProductDate设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM ProductDate设置失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM ProductDate设置失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM ProductDate设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_set_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            memcpy(g_fan_eeprom_set_par.product_date, productdate, len);
            if(bsp_hmi_fan_eeprom_product_date(0)==BSP_OK)
            {
                while((g_fan_set_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_set_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM ProductDate设置成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM ProductDate设置失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM ProductDate设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_product_date(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_product_date_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_satellite_receiver_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot != bsp_get_slot_id())
    {
        send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        s8 satellitereceiver[12] = {0};

        if(len == 0)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        if(len > 12)
        {
            len = 12;
        }
        memcpy(satellitereceiver, (s8 *)(cmd.pkg_data+1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if(bsp_set_bbu_satellitereceiver(satellitereceiver)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM SatelliteReceiver设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM SatelliteReceiver设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_satellite_receiver(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_satellite_receiver_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_set_fan_init_speed_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot != IPMB_SLOT10)
    {
        send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        u16 fanspeed[3] = {0};

        if(len != 6)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        fanspeed[0] = (cmd.pkg_data[1]<<8)|cmd.pkg_data[2];
        fanspeed[1] = (cmd.pkg_data[3]<<8)|cmd.pkg_data[4];
        fanspeed[2] = (cmd.pkg_data[5]<<8)|cmd.pkg_data[6];
        if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_set_eeprom = 0;
            memcpy(g_fan_eeprom_set_par.fan_initialspeed, fanspeed, sizeof(fanspeed));
            if(bsp_hmi_fan_eeprom_initial_speed(0)==BSP_OK)
            {
                while((g_fan_set_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_set_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM FanInitialSpeed设置成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM FanInitialSpeed设置失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM FanInitialSpeed设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_fan_init_speed(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_fan_init_speed_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_bbu_set_temp_threshold_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;

    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u32 len = cmd.pkg_datalen - 1;
        s8 tempthreshold[2] = {0};

        if(len != 2)
        {
            send_msg(cmd, "error data len(%d) .\r\n", len);
            bsp_mutex_unlock(cmd_list[cmd.index].plock);
            return NULL;
        }
        memcpy(tempthreshold, (s8 *)(cmd.pkg_data+1), len);
        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_set_bbu_temperaturethreshold(tempthreshold)==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM TemperatureThreshold设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM TemperatureThreshold设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_tempthreshold(u8Slot, tempthreshold)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM TemperatureThreshold设置成功!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM TemperatureThreshold设置成功!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM TemperatureThreshold设置成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM TemperatureThreshold设置失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM TemperatureThreshold设置失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM TemperatureThreshold设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_set_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            memcpy(g_fan_eeprom_set_par.temperature_threshold, tempthreshold, len);
            if(bsp_hmi_fan_eeprom_temp_threshold(0)==BSP_OK)
            {
                while((g_fan_set_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_set_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM TemperatureThreshold设置成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM TemperatureThreshold设置失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM TemperatureThreshold设置失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_set_temp_threshold(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_set_temp_threshold_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_crc_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;

        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_eeprom_get_crc()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM CRC读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                send_msg(cmd, "主控板CRC=0x%x\r\n",g_mca_eeprom_par.checkSum);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM CRC读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_crc(u8Slot)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " 基带板EEPROM CRC读取成功!\r\n");
                    send_msg(cmd, " 基带板CRC=0x%x\r\n",g_bbp_eeprom_par.checkSum);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " 交换板EEPROM CRC读取成功!\r\n");
                    send_msg(cmd, " 交换板CRC=0x%x\r\n",g_bbp_eeprom_par.checkSum);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " 增强板EEPROM CRC读取成功!\r\n");
                    send_msg(cmd, " 增强板CRC=0x%x\r\n",g_bbp_eeprom_par.checkSum);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM CRC读取失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM CRC读取失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM CRC读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_get_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_hmi_fan_eeprom_crc(1)==BSP_OK)
            {
                while((g_fan_get_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_get_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM CRC读取成功!\r\n");                    
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    send_msg(cmd, "风扇板CRC=0x%x\r\n",g_fan_eeprom_get_par.checkSum);
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM CRC读取失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM CRC读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_crc(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_crc_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_device_id_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    char *null = "null";
    int i;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;

        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_get_bbu_deviceid()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM DeviceID读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.device_id, 16))
                {
                    memcpy(g_mca_eeprom_par.device_id, null, 5);
                }
                send_msg(cmd, "主控板DeviceID=%s\r\n",g_mca_eeprom_par.device_id);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM DeviceID读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_deviceid(u8Slot)==BSP_OK)
            {
                if(BSP_OK == stringisnull(g_bbp_eeprom_par.device_id, 16))
                {
                    memcpy(g_bbp_eeprom_par.device_id, null, 5);
                }
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " 基带板EEPROM DeviceID读取成功!\r\n");
                    send_msg(cmd, " 基带板DeviceID=%s\r\n",g_bbp_eeprom_par.device_id);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " 交换板EEPROM DeviceID读取成功!\r\n");
                    send_msg(cmd, " 交换板DeviceID=%s\r\n",g_bbp_eeprom_par.device_id);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " 增强板EEPROM DeviceID读取成功!\r\n");
                    send_msg(cmd, " 增强板DeviceID=%s\r\n",g_bbp_eeprom_par.device_id);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM DeviceID读取失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM DeviceID读取失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM DeviceID读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_get_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_hmi_fan_eeprom_device_id(1)==BSP_OK)
            {
                while((g_fan_get_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_get_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM DeviceID读取成功!\r\n");
                    if(BSP_OK == stringisnull(g_fan_eeprom_get_par.device_id, 16))
                    {
                        memcpy(g_fan_eeprom_get_par.device_id, null, 5);
                    }
                    send_msg(cmd, "风扇板DeviceID=%s\r\n",g_fan_eeprom_get_par.device_id);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM DeviceID读取失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM DeviceID读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_device_id(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_device_id_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_board_type_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    char *null = "null";
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;

        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_get_bbu_boardtype()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM BoardType读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.board_type, 32))
                    memcpy(g_mca_eeprom_par.board_type, null, 5);
                send_msg(cmd, "主控板BoardType=%s\r\n",g_mca_eeprom_par.board_type);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM BoardType读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_boardtype(u8Slot)==BSP_OK)
            {
                if(BSP_OK == stringisnull(g_bbp_eeprom_par.board_type, 32))
                    memcpy(g_bbp_eeprom_par.board_type, null, 5);
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " 基带板EEPROM BoardType读取成功!\r\n");
                    send_msg(cmd, " 基带板BoardType=%s\r\n",g_bbp_eeprom_par.board_type);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " 交换板EEPROM BoardType读取成功!\r\n");
                    send_msg(cmd, " 交换板BoardType=%s\r\n",g_bbp_eeprom_par.board_type);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " 增强板EEPROM BoardType读取成功!\r\n");
                    send_msg(cmd, " 增强板BoardType=%s\r\n",g_bbp_eeprom_par.board_type);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM BoardType读取失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM BoardType读取失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM BoardType读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_get_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_hmi_fan_eeprom_board_type(1)==BSP_OK)
            {
                while((g_fan_get_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_get_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM BoardType读取成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    if(BSP_OK == stringisnull(g_fan_eeprom_get_par.board_type, 32))
                        memcpy(g_fan_eeprom_get_par.board_type, null, 5);
                    send_msg(cmd, "风扇板BoardType=%s\r\n",g_fan_eeprom_get_par.board_type);
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM BoardType读取失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM BoardType读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_board_type(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_board_type_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_mac_addr1_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    char str[128] = {0};
    uint8_t u8Slot = 0;
     //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if( u8Slot != bsp_get_slot_id())
    {
        send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
        bsp_mutex_unlock(cmd_list[cmd.index].plock);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;

        if(board_type == BOARD_TYPE_MCT)
        {
            if(bsp_get_bbu_macaddr1()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM MACADDR1读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                ucharxtostr(g_mca_eeprom_par.mac_addr1, str, 6);
                send_msg(cmd,"主控板MACADDR1=%s\r\n", str);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM MACADDR1读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_mac_addr1(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_mac_addr1_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_mac_addr2_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    char str[128] = {0};
    uint8_t u8Slot = 0;
     //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if( u8Slot != bsp_get_slot_id())
    {
        send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
        bsp_mutex_unlock(cmd_list[cmd.index].plock);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;

        if(board_type == BOARD_TYPE_MCT)
        {
            if(bsp_get_bbu_macaddr2()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM MACADDR2读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                ucharxtostr(g_mca_eeprom_par.mac_addr2, str, 6);
                send_msg(cmd,"主控板MACADDR2=%s\r\n", str);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM MACADDR2读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_mac_addr2(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_mac_addr2_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_product_sn_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    char str[128] = {0};
    char *null = "null";
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u8 i=0;

        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_get_bbu_productsn()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM ProductSN读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.product_sn, 32))
                    memcpy(g_mca_eeprom_par.product_sn, null, 5);
                send_msg(cmd,"主控板ProductSN=%s\r\n",g_mca_eeprom_par.product_sn);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM ProductSN读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_productsn(u8Slot)==BSP_OK)
            {
                if(BSP_OK == stringisnull(g_bbp_eeprom_par.product_sn, 32))
                    memcpy(g_bbp_eeprom_par.product_sn, null, 5);
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " 基带板EEPROM ProductSN读取成功!\r\n");
                    send_msg(cmd, " 基带板ProductSN=%s\r\n",g_bbp_eeprom_par.product_sn);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " 交换板EEPROM ProductSN读取成功!\r\n");
                    send_msg(cmd, " 交换板ProductSN=%s\r\n",g_bbp_eeprom_par.product_sn);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " 增强板EEPROM ProductSN读取成功!\r\n");
                    send_msg(cmd, " 增强板ProductSN=%s\r\n",g_bbp_eeprom_par.product_sn);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;              
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM ProductSN读取失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM ProductSN读取失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM ProductSN读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_get_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_hmi_fan_eeprom_product_sn(1)==BSP_OK)
            {
                while((g_fan_get_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_get_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM ProductSN读取成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;                    
                    if(BSP_OK == stringisnull(g_fan_eeprom_get_par.product_sn, 32))
                        memcpy(g_fan_eeprom_get_par.product_sn, null, 5);
                    send_msg(cmd,"风扇板ProductSN=%s\r\n",g_fan_eeprom_get_par.product_sn);
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM ProductSN读取失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM ProductSN读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_product_sn(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_product_sn_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_manufacturer_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    char *null = "null";
    int i = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;

        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_get_bbu_manufacturer()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM Manufacture读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.manufacturer, 12))
                    memcpy(g_mca_eeprom_par.manufacturer, null, 5);
                send_msg(cmd, "主控板Manufacture=%s\r\n",g_mca_eeprom_par.manufacturer);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM Manufacture读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_manufacturer(u8Slot)==BSP_OK)
            {
                if(BSP_OK == stringisnull(g_bbp_eeprom_par.manufacturer, 12))
                    memcpy(g_bbp_eeprom_par.manufacturer, null, 5);
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " 基带板EEPROM Manufacture读取成功!\r\n");
                    send_msg(cmd, " 基带板Manufacture=%s\r\n",g_bbp_eeprom_par.manufacturer);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " 交换板EEPROM Manufacture读取成功!\r\n");
                    send_msg(cmd, " 交换板Manufacture=%s\r\n",g_bbp_eeprom_par.manufacturer);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " 增强板EEPROM Manufacture读取成功!\r\n");
                    send_msg(cmd, " 增强板Manufacture=%s\r\n",g_bbp_eeprom_par.manufacturer);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM Manufacture读取失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM Manufacture读取失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM Manufacture读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_get_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_hmi_fan_eeprom_manufacture(1)==BSP_OK)
            {
                while((g_fan_get_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_get_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM Manufacture读取成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    if(BSP_OK == stringisnull(g_fan_eeprom_get_par.manufacturer, 12))
                        memcpy(g_fan_eeprom_get_par.manufacturer, null, 5);
                    send_msg(cmd, "风扇板Manufacture=%s\r\n",g_fan_eeprom_get_par.manufacturer);
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM Manufacture读取失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM Manufacture读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_manufacturer(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_manufacturer_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_product_date_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    char str[128] = {0};
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        u8 i=0;

        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_get_bbu_productdate()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM ProductDate读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                ucharxtostr(g_mca_eeprom_par.product_date, str, 4);
                send_msg(cmd,"主控板ProductDate=%s\r\n", str);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM ProductDate读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_productdate(u8Slot)==BSP_OK)
            {
                ucharxtostr(g_bbp_eeprom_par.product_date, str, 4);
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " 基带板EEPROM ProductDate读取成功!\r\n");
                    send_msg(cmd, " 基带板ProductDate=%s\r\n", str);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " 交换板EEPROM ProductDate读取成功!\r\n");
                    send_msg(cmd, " 交换板ProductDate=%s\r\n", str);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " 增强板EEPROM ProductDate读取成功!\r\n");
                    send_msg(cmd, " 增强板ProductDate=%s\r\n", str);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;                                
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM ProductDate读取失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM ProductDate读取失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM ProductDate读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_get_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_hmi_fan_eeprom_product_date(1)==BSP_OK)
            {
                while((g_fan_get_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_get_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM ProductDate读取成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;                   
                    ucharxtostr(g_fan_eeprom_get_par.product_date, str, 4);
                    send_msg(cmd,"风扇板ProductDate=%s\r\n", str);
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM ProductDate读取失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM ProductDate读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_product_date(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_product_date_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_satellite_receiver_thread(void *arg)
{

    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    char *null = "null";
    uint8_t u8Slot = 0;

     //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if( u8Slot != bsp_get_slot_id())
    {
        send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
        bsp_mutex_unlock(cmd_list[cmd.index].plock);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;

        if(board_type == BOARD_TYPE_MCT)
        {
            if(bsp_get_bbu_satellitereceiver()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM SatelliteReceiver读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.satellite_receiver, 12))
                    memcpy(g_mca_eeprom_par.satellite_receiver, null, 5);
                send_msg(cmd, "主控板SatelliteReceiver=%s\r\n",g_mca_eeprom_par.satellite_receiver);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM SatelliteReceiver读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_satellite_receiver(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_satellite_receiver_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

void *test_bbu_get_fan_init_speed_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot != IPMB_SLOT10)
    {
        send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;
        if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_get_eeprom = 0;
            if(bsp_hmi_fan_eeprom_initial_speed(1)==BSP_OK)
            {
                while((g_fan_get_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_get_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM FanInitialSpeed获取成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    send_msg(cmd, "风扇板Fan_InitialSpeed0=0x%x, Fan_InitialSpeed1=0x%x, Fan_InitialSpeed2=0x%x\r\n",
                        g_fan_eeprom_get_par.fan_initialspeed[0],g_fan_eeprom_get_par.fan_initialspeed[1], g_fan_eeprom_get_par.fan_initialspeed[2]);
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM FanInitialSpeed获取失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM FanInitialSpeed获取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_fan_init_speed(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_fan_init_speed_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}
void *test_bbu_get_temp_threshold_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    uint8_t u8Slot = 0;
    uint8_t u8BoardType = 0;
    
    //获取槽位号
    u8Slot = cmd.pkg_data[0];
    if(u8Slot > 11)
    {
        send_msg(cmd, "error slot id(%d).\r\n", u8Slot);
        return NULL;
    }
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        u8 board_type = cmd.pkg_board;

        if(board_type == BOARD_TYPE_MCT)
        {
            if( u8Slot != bsp_get_slot_id())
            {
                send_msg(cmd, "error mct slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_get_bbu_temperaturethreshold()==BSP_OK)
            {
                send_msg(cmd, " 主控板EEPROM TemperatureThreshold读取成功!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                send_msg(cmd,"主控板TemperatureThreshold=%d %d\r\n",g_mca_eeprom_par.temperature_threshold[0],
                         g_mca_eeprom_par.temperature_threshold[1]);
            }
            else
            {
                send_msg(cmd, " 主控板EEPROM TemperatureThreshold读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if((board_type == BOARD_TYPE_BBP)||(board_type == BOARD_TYPE_FSA)||(board_type == BOARD_TYPE_ES))
        {
            if((u8Slot < 2) || (u8Slot > 7))
            {
                send_msg(cmd, "error bbp slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            //获取板类型
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_tempthreshold(u8Slot)==BSP_OK)
            {                
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " 基带板EEPROM TemperatureThreshold读取成功!\r\n");
                    send_msg(cmd, " 基带板TemperatureThreshold=%d %d\r\n",g_bbp_eeprom_par.temperature_threshold[0],
                         g_bbp_eeprom_par.temperature_threshold[1]);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " 交换板EEPROM TemperatureThreshold读取成功!\r\n");
                    send_msg(cmd, " 交换板TemperatureThreshold=%d %d\r\n",g_bbp_eeprom_par.temperature_threshold[0],
                         g_bbp_eeprom_par.temperature_threshold[1]);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " 增强板EEPROM TemperatureThreshold读取成功!\r\n");
                    send_msg(cmd, " 增强板TemperatureThreshold=%d %d\r\n",g_bbp_eeprom_par.temperature_threshold[0],
                         g_bbp_eeprom_par.temperature_threshold[1]);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " 基带板EEPROM TemperatureThreshold读取失败!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " 交换板EEPROM TemperatureThreshold读取失败!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " 增强板EEPROM TemperatureThreshold读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
        else if(board_type == BOARD_TYPE_FAN)
        {
            int wait_time = 2000;
            g_fan_get_eeprom = 0;
            if(u8Slot != IPMB_SLOT10)
            {
                send_msg(cmd, "error fan slot id(%d).\r\n", u8Slot);
                bsp_mutex_unlock(cmd_list[cmd.index].plock);
                return NULL;
            }
            if(bsp_hmi_fan_eeprom_temp_threshold(1)==BSP_OK)
            {
                while((g_fan_get_eeprom == 0) && (wait_time-->0))
                {
                    usleep(1000);
                }
                if(g_fan_get_eeprom)
                {
                    send_msg(cmd, " 风扇板EEPROM TemperatureThreshold读取成功!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    send_msg(cmd,"风扇板TemperatureThreshold=%d %d\r\n",g_fan_eeprom_get_par.temperature_threshold[0],
                             g_fan_eeprom_get_par.temperature_threshold[1]);
                }
                else
                {
                    send_msg(cmd, " 风扇板EEPROM TemperatureThreshold读取失败!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " 风扇板EEPROM TemperatureThreshold读取失败!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            cmd.pkg_datalen = htonl(0);
            send_result(cmd);
        }
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return NULL;
}
cmd_t test_bbu_get_temp_threshold(cmd_t cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_bbu_get_temp_threshold_thread, (void*)&cmd);
    pthread_detach(tid);
    sleep(1);
    return cmd;
}

testcmd_t cmd_list[]=
{
    /*****************************************主控板测试项***************************************/
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_TEMPERATURE, "主控温度", test_mct_temprature, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_PPCDDR, "主控DDR", test_ppc_ddr, &ppcddr_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_NORFLASH, "主控NorFlash", test_ppc_norflash, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_NANDFLASH, "主控NandFlash", test_nandflash, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_CPLD_READ, "主控CPLD版本", test_cpld_version, &mtd_lock},
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_CPLD_LOAD, "主控CPLD加载", test_cpld_load, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_GESSWITCH, "主控GESwitch", test_ppc_geswitch, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_GPS, "GPS", test_ppc_gps, &mtd_lock},
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_USB, "主控USB", test_ppc_usb, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_SFP, "主控光模块", test_ppc_sfp, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_POWER, "主控功耗", test_ppc_power, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_EEPROM, "主控eeprom", test_ppc_eeprom, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_PHY, "主控phy", test_ppc_phy, &mtd_lock},
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_VER, "主控版本", test_ppc_ver, &mtd_lock},    
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_EXTSYNC_PP1S, "主控外同步pp1s接口", test_ppc_extsync_pp1s, &mtd_lock},    
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_EXTSYNC_TOD, "主控外同步tod接口", test_ppc_extsync_tod, &mtd_lock},
    /*****************************************基带板测试项***************************************/
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_BOOT, "基带板复位", test_bbp_reset, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_FPGA_LOAD, "基带板fpga加载", test_bbp_fpgaload, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_DSP_LOAD, "基带板dsp加载", test_bbp_dsp_load, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_MCU_UPDATE, "基带板mcu更新", test_bbp_mcuupdate, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_CPLD_UPDATE, "基带板cpld更新", test_bbp_cpldupdate, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_ETHSW, "基带板GESwitch", test_bbp_ethsw, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_SDRAM, "基带板SDRAM", test_bbp_sdram, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_TEMPERATURE, "基带板温度", test_bbp_temprature, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_POWER, "基带板功耗", test_bbp_power_info, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_EEPROM, "基带板eeprom", test_bbp_eeprom, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_SRIOSWITCH, "基带板sriosw", test_bbp_srioswt, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_CPLD, "基带板cpld版本", test_bbp_cpld_version, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_VER, "基带板版本", test_bbp_ver, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_SFP, "基带板光模块", test_bbp_sfp, &bbp_lock},    
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_IR_SYNC, "基带板光口同步", test_bbp_ir_sync, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_DSP_CPU, "CPU-DSP 链路", test_bbp_dsp_cpu, NULL},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_DSP_DDR, "DSP-DDR 测试", test_bbp_dsp_ddr, NULL},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_SRIO_DATA, "DSP-DSP SRIO 数据", test_bbp_srio_data, NULL},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_AIF, "DSP AIF", test_bbp_dsp_aif, NULL},
    /*****************************************交换板测试项***************************************/
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_BOOT, "交换板复位", test_bbp_reset, &bbp_lock},
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_FPGA_160T, "交换板fpga_160t加载", test_fsa_fpga160t_fpgaload, &bbp_lock},
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_FPGA_325T, "交换板fpga_325t加载", test_bbp_fpgaload, &bbp_lock},
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_MCU_UPDATE, "交换板mcu更新", test_bbp_mcuupdate, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_ETHSW, "交换板GESwitch", test_bbp_ethsw, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_SDRAM, "交换板SDRAM", test_bbp_sdram, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_TEMPERATURE, "交换板温度", test_bbp_temprature, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_POWER, "交换板功耗", test_bbp_power_info, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_EEPROM, "交换板eeprom", test_bbp_eeprom, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_SRIOSWITCH, "交换板sriosw", test_bbp_srioswt, &bbp_lock},
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_VER, "交换板版本", test_bbp_ver, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_SFP, "交换板光模块", test_bbp_sfp, &bbp_lock},    
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_IR_SYNC, "交换板光口同步", test_bbp_ir_sync, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_PHY, "交换板PHY", test_fsa_phy, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_PLL_CFG, "交换板PLL配置", test_fsa_pll_cfg, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_FPGA_325T_DDR, "交换板FPGA_325T DDR 测试", test_fsa_fpga_325t_ddr, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_FPGA_160T_SRIO, "交换板FPGA_160T SRIO数据", test_fsa_fpga_160t_srio, &bbp_lock},
    /*****************************************同步板测试项***************************************/
    {0, BOARD_TYPE_ES, 0, TEST_ES_BOOT, "同步板复位", test_bbp_reset, &bbp_lock},
    {0, BOARD_TYPE_ES, 0, TEST_ES_FPGA, "同步板fpga加载", test_bbp_fpgaload, &bbp_lock},
    {0, BOARD_TYPE_ES, 0, TEST_ES_MCU_UPDATE, "同步板mcu更新", test_bbp_mcuupdate, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_ETHSW, "同步板GESwitch", test_bbp_ethsw, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_SDRAM, "同步板SDRAM", test_bbp_sdram, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_TEMPERATURE, "同步板温度", test_bbp_temprature, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_EEPROM, "同步板eeprom", test_bbp_eeprom, &bbp_lock},
    {0, BOARD_TYPE_ES, 0, TEST_ES_VER, "同步板版本", test_bbp_ver, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_SFP, "同步板光模块", test_bbp_sfp, &bbp_lock},    
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_PLL_CFG, "同步板PLL配置", test_fsa_pll_cfg, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_COPPER_LINK, "同步板电口链路", test_es_copper_link, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_FIBBER_LINK, "同步板光口链路", test_es_fibber_link, &bbp_lock},
    /*****************************************风扇板测试项***************************************/
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_BOOT, "风扇板复位", test_fan_reset, &fan_lock},
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_SPEED_SET, "风扇转速设置", test_fan_speed_set, &fan_lock},
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_SPEED_GET, "风扇转速获取", test_fan_speed_get, &fan_lock},
    {MASK_TESTALL, BOARD_TYPE_FAN, 0, TEST_FAN_EEPROM, "风扇板eeprom", test_fan_eeprom, &fan_lock},
    {MASK_TESTALL, BOARD_TYPE_FAN, 0, TEST_FAN_TEST, "风扇测试", test_fan_test, &fan_lock},
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_VERSION, "风扇板版本", test_fan_version, &fan_lock},
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_UPDATE, "风扇板mcu升级", test_fan_mcuupdate, &fan_lock},
    /*****************************************监控板测试项***************************************/
    {0, BOARD_TYPE_PEU, 0, TEST_PEU_BOOT, "监控板复位", test_peu_reset, &peu_lock},
    {0, BOARD_TYPE_PEU, 0, TEST_PEU_POWER_DOWN, "监控板掉电", test_peu_power_down, &peu_lock},
    {MASK_TESTALL, BOARD_TYPE_PEU, 0, TEST_PEU_DRYIN, "监控板干接点状态", test_peu_dryin, &peu_lock},
    {MASK_TESTALL, BOARD_TYPE_PEU, 0, TEST_PEU_TEMPERATURE, "监控板温度", test_peu_temperature, &peu_lock},
    {0, BOARD_TYPE_PEU, 0, TEST_PEU_VERSION, "监控板版本", test_peu_version, &peu_lock},
    {0, BOARD_TYPE_PEU, 0, TEST_PEU_UPDATE, "监控板mcu升级", test_peu_mcuupdate, &peu_lock},    
    {MASK_TESTALL, BOARD_TYPE_PEU, 0, TEST_PEU_RS485, "监控板RS485测试", test_peu_rs485, &peu_lock},
    /******************************************板间测试项***************************************/
    {0, 0, 0, TEST_MCT_BBP_HMI, "HMI(主控<->基带)", test_mct_bbp_hmi, &hmi_lock},
    {0, 0, 0, TEST_MCT_PEU_HMI, "HMI(主控<->监控)", test_mct_peu_hmi, &hmi_lock},
    {0, 0, 0, TEST_MCT_FAN_HMI, "HMI(主控<->风扇)", test_mct_fan_hmi, &hmi_lock},
    {0, 0, 0, TEST_AFC, "AFC测试", test_afc, &hmi_lock},
    {0, 0, 0, TEST_BOARD_READY, "基站就绪", test_board_ready, &hmi_lock},
    {0, 0, 0, TEST_AFC_SYNC, "帧号同步", test_sync, &hmi_lock},
    //{0, TEST_REBOOT_BOARDS, "test_reboot_boards", test_reboot_boards, &bbp_lock},
    
    /******************************************RRU测试项***************************************/
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_VERQ, "RRU版本查询", rrutest_version_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_VERD, "RRU版本下载", rrutest_verdownload_req, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_VERA, "RRU版本激活", rrutest_veractivate_req, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_ANT, "RRU天线掩码配置", rrutest_antenna_config, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_RF, "RRU射频状态查询", rrutest_rfstatus_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_RUN, "RRU运行状态查询", rrutest_rrustatus_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_SFP, "RRU光口状态查询", rrutest_fiberstatus_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_TIME, "RRU系统时间配置", rrutest_systime_config, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_ALARM, "RRU告警查询", rrutest_almquery_req, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_PARA, "RRU参数查询", rrutest_para_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_CAL, "RRU功率校准", rrutest_power_cal, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_CELL, "RRU小区配置", rrutest_cell_config, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_DELAY, "RRU光延时配置", rrutest_fiberdelay_config, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_REBOOT, "RRU复位", rrutest_reset, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_SERIAL, "RRU远程串口关闭", rrutest_serialclose_req, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_HARDWARE, "RRU硬件参数查询", rrutest_hwparam_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_HEARTBEAT_PC, "RRU基本参数获取", rrutest_heartbeat_to_pc, &rru_lock},
	
    /***************************************EEPROM参数配置/读取测试项*************************/
    {0, 0, 0, TEST_SET_CRC, "设置EEPROM CRC", test_bbu_set_crc, &eeprom_lock},
    {0, 0, 0, TEST_SET_DEVICE_ID, "设置DeviceID", test_bbu_set_device_id, &eeprom_lock},
    {0, 0, 0, TEST_SET_BOARD_TYPE, "设置板卡类型", test_bbu_set_board_type, &eeprom_lock},
    {0, 0, 0, TEST_SET_MAC_ADDR1, "设置MAC1", test_bbu_set_mac_addr1, &eeprom_lock},
    {0, 0, 0, TEST_SET_MAC_ADDR2, "设置MAC2", test_bbu_set_mac_addr2, &eeprom_lock},
    {0, 0, 0, TEST_SET_ETH3_ADDR, "设置eth3 IP", test_bbu_set_eth3_addr, NULL},
    {0, 0, 0, TEST_SET_PRODUCT_SN, "设置产品序号", test_bbu_set_product_sn, &eeprom_lock},
    {0, 0, 0, TEST_SET_MANUFACTURE, "设置生产序号", test_bbu_set_manufacturer, &eeprom_lock},
    {0, 0, 0, TEST_SET_PRODUCT_DATE, "设置生产日期", test_bbu_set_product_date, &eeprom_lock},
    {0, 0, 0, TEST_SET_SATELLITE_RECEIVER, "设置GPS模块类型", test_bbu_set_satellite_receiver, &eeprom_lock},
    {0, 0, 0, TEST_SET_FAN_INIT_SPEED, "设置初始化转速", test_bbu_set_fan_init_speed, &eeprom_lock},
    {0, 0, 0, TEST_SET_TEMP_THRESHOLD, "设置温度阈值", test_bbu_set_temp_threshold, &eeprom_lock},

    {0, 0, 0, TEST_GET_CRC, "获取EEPROM CRC", test_bbu_get_crc, &eeprom_lock},
    {0, 0, 0, TEST_GET_DEVICE_ID, "获取DeviceID", test_bbu_get_device_id, &eeprom_lock},
    {0, 0, 0, TEST_GET_BOARD_TYPE, "获取板卡类型", test_bbu_get_board_type, &eeprom_lock},
    {0, 0, 0, TEST_GET_MAC_ADDR1, "获取MAC1", test_bbu_get_mac_addr1, &eeprom_lock},
    {0, 0, 0, TEST_GET_MAC_ADDR2, "获取MAC2", test_bbu_get_mac_addr2, &eeprom_lock},
    {0, 0, 0, TEST_GET_PRODUCT_SN, "获取产品序号", test_bbu_get_product_sn, &eeprom_lock},
    {0, 0, 0, TEST_GET_MANUFACTURE, "获取生产序号", test_bbu_get_manufacturer, &eeprom_lock},
    {0, 0, 0, TEST_GET_PRODUCT_DATE, "获取生产日期", test_bbu_get_product_date, &eeprom_lock},
    {0, 0, 0, TEST_GET_SATELLITE_RECEIVER, "获取GPS模块类型", test_bbu_get_satellite_receiver, &eeprom_lock},
    {0, 0, 0, TEST_GET_FAN_INIT_SPEED, "设置初始化转速", test_bbu_get_fan_init_speed, &eeprom_lock},
    {0, 0, 0, TEST_GET_TEMP_THRESHOLD, "获取温度阈值", test_bbu_get_temp_threshold, &eeprom_lock},

    {0, 0, 0, CMD_TESTALL, "test_all", NULL, NULL},
    {0, 0, 0, CMD_TESTDANBAN, "test_bbu", NULL, NULL},
};
#define CMD_LIST_SIZE	sizeof(cmd_list)/sizeof(testcmd_t)

void test_func(uint16_t cmdid, uint32_t times)
{
    int i = 0;
    cmd_t cmd;
    cmd.addr.sin_addr.s_addr = inet_addr("192.168.1.17");
    cmd.pkg_header = htonl(BOARD_TEST_HEADER);
    cmd.pkg_product = PRODUCT_TYPE;
    cmd.pkg_board = BOARD_TYPE_BBP;
    cmd.pkg_cmd = cmdid;
    cmd.pkg_totaltimes = htonl(times);
    cmd.pkg_failtimes = htonl(0);
    cmd.pkg_successtimes = htonl(0);
    cmd.pkg_datalen = htonl(0);
    for(i=0; i<CMD_LIST_SIZE; i++)
    {
        if(cmd.pkg_cmd==cmd_list[i].cmd)
        {
            if(cmd_list[i].func!=NULL)
            {
                cmd.index = i;
                cmd_list[i].func(cmd);
            }
            break;
        }
    }
}
int send_result(cmd_t cmd)
{
    int i = 0;
    u8 slot = cmd.pkg_data[0]; /*读取槽位号*/
    uint32_t *p = cmd.pkg_data + htonl(cmd.pkg_datalen);
    *p = htonl(BOARD_TEST_END);
    
    //cmd_list[cmd.index].error_times += cmd.pkg_failtimes;
    if(cmd_list[cmd.index].mask !=0)
    {
        /*判断槽位号*/
        if(slot < MAX_BOARDS_NUMBER)
        {
            if((test_slot_mask & (1<<slot)) !=0)
                test_error_times[cmd.index][slot] += cmd.pkg_failtimes;
        }
        else
        {
            printf("[%s]:error slot id(%d).\r\n", __func__, slot);
        }
    }
    if(cmd.pkg_cmd==CMD_TESTALL ||  cmd.pkg_cmd==CMD_TESTDANBAN)
        bsp_mutex_unlock(&alltest_lock);

    if(cmd.addr.sin_addr.s_addr == 0)
    {
        return 0;
    }
    cmd.addr.sin_port = htons(BOARD_TEST_PORT);
    if(sendto(boardtest_fd, &cmd.pkg, PACKET_HEADER_LEN(cmd.pkg)+htonl(cmd.pkg_datalen)+4, 0, (struct sockaddr*)&cmd.addr, sizeof(cmd.addr))<0)
    {
        perror("send_result");
        printf("addr:%s\n", inet_ntoa(cmd.addr.sin_addr));
    }
    return 0;
}
int test_all_times = 0;
cmd_t cmdall = {0};

void test_slot_config(int slotmask)
{
    test_slot_mask = slotmask;
}
void stop_test_all(void)
{
    test_all_times = 0;
    if(is_alltesting)
    {
        send_msg(cmdall, "停止%s测试中...\r\n", (cmdall.pkg_cmd==CMD_TESTDANBAN)?"单板":"老化");
    }
    else
    {
        send_msg(cmdall, "%s测试已停止!\r\n", (cmdall.pkg_cmd==CMD_TESTDANBAN)?"单板":"老化");
        send_result(cmdall);
    }
}
void test_all_stop(void)
{
    stop_test_all();
}
void test_stop(void)
{
    stop_test_all();
}
void *test_stop_thread(void *arg)
{
    int starttime = 0, runtime = 0;
    int howlong = (int)arg;
    printf("******%s测试时长: %d小时%d分钟(%ds)....\r\n", (cmdall.pkg_cmd==CMD_TESTDANBAN)?"单板":"老化",howlong/3600, (howlong%3600)/60, howlong);
    starttime = bsp_runing_time();
    runtime = bsp_runing_time();
    while(test_all_times>0 && runtime-starttime <= howlong)
    {
        runtime = bsp_runing_time();
        sleep(5);
    }
    if(test_all_times>0)
    {
        stop_test_all();
    }
}
void *test_all_thread(void *arg)
{
    cmd_t cmd = *(cmd_t*)arg;
    unsigned int i = 0;
    unsigned int slot = 0;
    unsigned int count = 0;

    is_alltesting = 1;
    test_all_times = cmd.pkg_totaltimes+1;
    for(slot=0; slot<MAX_BOARDS_NUMBER; slot++)
    {
        for(i=0; i<CMD_LIST_SIZE; i++)
        {
            test_error_times[i][slot] = 0;
        }
    }
    cmd.pkg_totaltimes = 1;
    while(test_all_times>1)
    {
        count++;
        send_msg(cmd, "\r\n");
        send_msg(cmd, "*******************第%d/%d次%s测试********\r\n", count, test_all_times+count-2,(cmd.pkg_cmd==CMD_TESTALL)?"老化":"单板");
        for(slot=0; slot<MAX_BOARDS_NUMBER; slot++)
        {
            if((test_slot_mask & (1<<slot)) != 0)
            { 
                send_msg(cmd, "------------------SLOT(%d)测试----------------\r\n", slot);
                for(i=0; i<CMD_LIST_SIZE; i++)
                {
                    if(cmd_list[i].mask!=0)
                    {                          
                        if((cmd.pkg_cmd==CMD_TESTALL)&&(cmd_list[i].mask & MASK_TESTALL)==0)
                        {
                            continue;
                        }
                        else if((cmd.pkg_cmd==CMD_TESTDANBAN)&&(cmd_list[i].mask & MASK_TEST_DANBAN)==0)
                        {
                            continue;
                        }
                        if((((slot == IPMB_SLOT0) || (slot == IPMB_SLOT1)) && (cmd_list[i].boardtype == BOARD_TYPE_MCT))
                            || ((slot>IPMB_SLOT1) && (slot<IPMB_SLOT8) && (cmd_list[i].boardtype == BOARD_TYPE_BBP) && (boards[slot].type == BOARD_TYPE_BBP))
                            || ((slot>IPMB_SLOT5) && (slot<IPMB_SLOT8) && (cmd_list[i].boardtype == BOARD_TYPE_FSA) && (boards[slot].type == BOARD_TYPE_FSA))
                            || ((slot>IPMB_SLOT5) && (slot<IPMB_SLOT8) && (cmd_list[i].boardtype == BOARD_TYPE_ES) && (boards[slot].type == BOARD_TYPE_ES))
                            || (((slot == IPMB_SLOT8) || (slot == IPMB_SLOT9)) && (cmd_list[i].boardtype == BOARD_TYPE_PEU))
                            || ((slot == IPMB_SLOT10) && (cmd_list[i].boardtype == BOARD_TYPE_FAN)))
                        {
                            if(cmd_list[i].cmd==TEST_MCT_PPCDDR)
                            {
                                cmd.pkg_mem.addr = 0x0;
                                cmd.pkg_mem.length = htonl(40*1024*1024);
                            }
                            bsp_mutex_lock(&alltest_lock);
                            if(cmd_list[i].func!=NULL)
                            {
                                cmd.index = i;
                                cmd.pkg_datalen = 1;
                                cmd.pkg_data[0] = slot;
                                cmd_list[i].func(cmd);
                            }
                            else
                            {
                                bsp_mutex_unlock(&alltest_lock);
                            }
                            bsp_mutex_lock(&alltest_lock);
                            bsp_mutex_unlock(&alltest_lock);
                            if(test_all_times==0)
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }
        test_all_times--;
    }
    usleep(500*1000);
    bsp_mutex_lock(&alltest_lock);
    bsp_mutex_unlock(&alltest_lock);
    send_msg(cmd, "****************%s测试%d次******************\r\n",(cmd.pkg_cmd==CMD_TESTDANBAN)?"单板":"老化",count);
    for(slot =0; slot<MAX_BOARDS_NUMBER; slot++)
    {
        if((test_slot_mask & (1<<slot)) !=0)
        {
            for(i=0; i<CMD_LIST_SIZE; i++)
            {
                //if(cmd_list[i].error_times>0)
                if(test_error_times[i][slot] > 0)
                {
                    send_msg(cmd, "*错误信息slot[%d]: %20s : %2d *\r\n",slot, cmd_list[i].cmdname, test_error_times[i][slot]);
                }
            }
        }
    }
    send_msg(cmd, "****************%s测试完成*****************\r\n",(cmd.pkg_cmd==CMD_TESTDANBAN)?"单板":"老化");
    cmd.pkg_datalen = 0;
    send_result(cmd);
    is_alltesting = 0;
    return NULL;
}
void test_all_func(cmd_t *cmd)
{
    pthread_t tid;
    pthread_create(&tid, NULL, test_all_thread, (void*)cmd);
    pthread_detach(tid);
}
void test_all(int times, int hour, int min)
{
    int hl = 0;
    if(is_alltesting)
    {
        send_msg(cmdall, "%s测试进行中,请不要重复启动%s测试...\r\n",(cmdall.pkg_cmd==CMD_TESTDANBAN)?"单板":"老化",(cmdall.pkg_cmd==CMD_TESTDANBAN)?"单板":"老化");
        return;
    }
    memset(&cmdall, 0, sizeof(cmdall));
    cmdall.pkg_cmd = CMD_TESTALL;
    if(times==0)
    {
        cmdall.pkg_totaltimes = 1000000;
    }
    else if(times>0)
    {
        cmdall.pkg_totaltimes = times;
    }
    if(hour==0 && min==0)
    {
        hour = 365*24;
    }
    hl = hour*3600+min*60;
    printf("hour=%d, min=%d, hl=%d\n", hour,min, hl);
    if(times>=0)
    {
        test_all_func(&cmdall);
    }
    if(times==0)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, test_stop_thread, (void*)(hl));
        pthread_detach(tid);
    }
}
void test_bbu(int times, int hour, int min)
{
    int hl = hour*3600+min*60;
    if(is_alltesting)
    {
        send_msg(cmdall, "单板测试进行中,请不要重复启动测试...\r\n");
        return;
    }
    printf("hour=%d, min=%d, hl=%d\n", hour,min, hl);
    cmdall.pkg_cmd = CMD_TESTDANBAN;
    if(times==0)
    {
        cmdall.pkg_totaltimes = 1000000;
    }
    else if(times>0)
    {
        cmdall.pkg_totaltimes = times;
    }
    if(times>=0)
    {
        //test_all_func(&cmdall);
        pthread_t tid;
        pthread_create(&tid, NULL, test_all_thread, (void*)&cmdall);
        pthread_detach(tid);
    }
    if(times==0)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, test_stop_thread, (void*)(hl));
        pthread_detach(tid);
    }
}
int test_item(int cmdid, uint32_t times)
{
    int i = 0;
    cmd_t cmd = {0};
    for(i=0; i<CMD_LIST_SIZE; i++)
    {
        if(cmdid == cmd_list[i].cmd)
        {
            if(cmd_list[i].cmd==TEST_MCT_PPCDDR)
            {
                cmd.pkg_mem.addr = 0x0;
                cmd.pkg_mem.length = htonl(0x1f400000);
            }
            cmd.pkg_cmd = cmdid;
            cmd.pkg_totaltimes = htonl(times);
            if(cmd_list[i].func!=NULL)
            {
                cmd.index = i;
                cmd_list[i].func(cmd);
            }
        }
    }
}

void *boardtest_thread(void *arg)
{
    struct sockaddr_in addr = {AF_INET};
    int addrlen = sizeof(addr);
    char buf[PACKET_DATA_SIZE] = "";
    cmd_t cmd;

    printf("start boardtest_thread...\r\n");
    boardtest_fd = socket(AF_INET, SOCK_DGRAM,0);
    if(boardtest_fd<0)
    {
        perror("[msg_proc_thread]:socket udp");
        return 0;
    }
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(BOARD_TEST_PORT);
    if(bind(boardtest_fd, (struct sockaddr*)&addr, sizeof(addr))!=0)
    {
        perror("[msg_proc_thread]:bind boardtest_fd");
        close(boardtest_fd);
        boardtest_fd = -1;
        return 0;
    }

    while(1)
    {
        int len = 0;
        int i = 0;
        memset(&cmd, 0, sizeof(cmd));
        len = recvfrom(boardtest_fd, &cmd.pkg, sizeof(cmd.pkg), 0, (struct sockaddr *)&cmd.addr, &addrlen);
        if( len <= 0 )
        {
            usleep(100);
            continue;
        }
        printf("recv from:%s\r\n", inet_ntoa(cmd.addr.sin_addr));
        //printf("[%s] 0x%x 0x%x\r\n", inet_ntoa(cmd.addr.sin_addr), htonl(cmd.pkg_header), htons(cmd.pkg_cmd));
        if( (cmd.pkg_header != htonl(BOARD_TEST_HEADER)) && (cmd.pkg.product != PRODUCT_TYPE))
        {
            printf("[boardtest]:error! header=0x%x, product=0x%x\r\n", cmd.pkg_header, cmd.pkg_product);
            continue;
        }

        if((htons(cmd.pkg_cmd)&0xf000)==0xd000)
        {
            test_bbp_cpudsp(cmd);
            continue;
        }
        for(i=0; i<CMD_LIST_SIZE; i++)
        {
            if(htons(cmd.pkg_cmd)==cmd_list[i].cmd)
            {
                if(cmd_list[i].func!=NULL)
                {
                    cmd.index = i;
                    cmd_list[i].func(cmd);
                }
                //printf("find cmd\r\n");
                break;
            }
        }
    }
    return NULL;
}
void init_boardtest_thread(void)
{
    pthread_t server_tid;
    pthread_create(&server_tid, NULL, boardtest_thread, NULL);
    pthread_detach(server_tid);
}
void init_boardtest(void)
{
    int fd = 0;
    int i = 0;
    printf("init_boardtest...\r\n");

    bsp_runing_time();
    for(i=0; i<CMD_LIST_SIZE; i++)
    {
        if(cmd_list[i].plock!=NULL)
        {
            pthread_mutex_init(cmd_list[i].plock,NULL);
        }
    }

    fd = open("/dev/mem",O_RDWR|O_SYNC);
    if (fd < 0)
    {
        printf("can not open mem!\r\n");
        return ;
    }
    g_u8ddrbase = (unsigned long)mmap((void *)0,RAM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,CONFIG_SYS_RAM_BASE);
    printf("g_u8ddrbase=0x%x\r\n", g_u8ddrbase);
    init_boardtest_thread();
    init_cpu_dsp_test();
    tod_uart_set();
    create_telnet_server();
}


