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
/* ���ò���slot */
#define MAX_CMD_NUMBER 200
unsigned int test_slot_mask = 0;
unsigned int test_error_times[MAX_CMD_NUMBER][MAX_BOARDS_NUMBER] = {0};

#define  MASK_TESTALL			0x0001
#define  MASK_TEST_DANBAN		0x0002

#define CONFIG_SYS_RAM_BASE		1500*1024*1024
#define RAM_SIZE (2*1024*1024*1024-1500*1024*1024)
unsigned long g_u8ddrbase = 0;

/* ��ģ�鹦������ */
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

#define CHECK_RET(x)	 ((x<0)?"ʧ��":"ͨ��")

extern unsigned char g_BBU_Fiber_INfo[128];
/*****************���ذ������***************************/
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
        //��ȡ�¶�
        for(i=0; i<4; i++)
        {
            if(bsp_read_temp(0, i+1,&temp[i])!=BSP_OK)
            {
                success = 0;
                send_msg(cmd, "��ȡ�¶ȵ�%d(1~4)ʧ��!\r\n", (i+1));
            }
            else if(temp[i]<-10)
            {
                success = 0;
                send_msg(cmd, "�¶�%d(1~4)����(ʧ��)!\r\n", (i+1));
            }
            else if(temp[i]>85)
            {
                success = 0;
                send_msg(cmd, "�¶�%d(1~4)����(ʧ��)!\r\n", (i+1));
            }
        }
        //�õ�����¶Ⱥ���С�¶�ֵ
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
        //����¶Ⱥ���С�¶Ȳ�ֵ
        if((temp[max_temp_id]-temp[min_temp_id])>25)
        {
            success = 0;
            send_msg(cmd, "�²����(ʧ��)!\r\n");
        }

        send_msg(cmd, "�¶�(1~4)=%d,%d,%d,%d\r\n",temp[0],temp[1],temp[2],temp[3]);

        if(success == 1)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, " �¶�����!\r\n");
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
                send_msg(cmd, "PPC DDRд0����,data[0x%x]<=>0x%x[0x%x]\r\n", base+i, data, (0x01010101<<loop));
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
                send_msg(cmd, "PPC DDRд1����,data[0x%x]<=>0x%x[0x%x]\r\n", base+i, data, (0x01010101<<loop));
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
            send_msg(cmd, "PPC DDRд5A5A����,data[0x%x]<=>0x%x[0x%x]\r\n", base+i, data, (0x01010101<<loop));
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
            send_msg(cmd, "PPC DDRдbase����,data[0x%x]<=>0x%x[0x%x]\r\n", base+i, data, (0x01010101<<loop));
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
        send_msg(cmd, "PPC DDR����, ����ĵ�ַ�򳤶�addr=0x%x, len=0x%x.\r\n"+1, base, length);
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
        send_msg(cmd, " PPC DDR��ʼ����..\r\n");

        if((base%4) != 0)
        {
            send_msg(cmd, " PPC DDR��ʼ��ַ����4�ֽڶ���!\r\n");
        }
        else
        {
            if(test_ram(cmd, base, length)!=BSP_OK)
            {
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                send_msg(cmd, " PPC DDR����ʧ��!\r\n");
            }
            else
            {
                send_msg(cmd, " PPC DDR���Գɹ�!\r\n");
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
    send_msg(cmd, "    ����nor...%s\r\n", CHECK_RET(ret));;
    if(ret<0)
        return BSP_ERROR;
    ret += erase_nor_flash(devname, 0, size);
    send_msg(cmd, "    ����nor...%s\r\n", CHECK_RET(ret));
    if(ret<0)
        return BSP_ERROR;
    sleep(1);
    ret = file_fill_and_check(devname, size);
    send_msg(cmd, "    д��and��֤...%s\r\n", CHECK_RET(ret));
    if(ret<0)
        return BSP_ERROR;
    ret = erase_nor_flash(devname, 0, size);
    send_msg(cmd, "    ����...%s\r\n", CHECK_RET(ret));
    if(ret<0)
        return BSP_ERROR;
    sleep(1);
    ret = copy_file(backfile, devname, size, 1);
    send_msg(cmd, "    �ָ�...%s\r\n", CHECK_RET(ret));
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

    send_msg(cmd, "��ʼ����nor flash...\r\n");

    if(bsp_start_time<0x70)
    {
        sleep(0x70-bsp_start_time);
    }

    cmd.pkg_failtimes = 0;
    cmd.pkg_successtimes = 0;
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        send_msg(cmd, "NorFlash����...\r\n");
        ret = test_norflash(cmd);
        if(ret==BSP_OK)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, "NorFlash���Գɹ�\r\n");
        }
        else
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            send_msg(cmd, "NorFlash����ʧ��\r\n");
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
        send_msg(cmd, " ����nand flash...\r\n");
        ret = file_fill_and_check(filename, 20*1024*1024);
        if(ret<0)
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            send_msg(cmd, "    Nand����ʧ��!\r\n");
        }
        else
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, "    Nand���Գɹ�!\r\n");
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
        send_msg(cmd, " CPLD����ʧ��!\r\n");
        send_result(cmd);
    }
    else
    {
        cmd.pkg_successtimes = htonl(1);
        send_msg(cmd, " CPLD���سɹ�!\r\n");
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
            send_msg(cmd, "geswitch ��дʧ��!\r\n");
        }

        if(success == 1)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, " geswitch ��д�ɹ�!\r\n");
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
            send_msg(cmd, "   ��CPLD�汾ʧ��!(��%x)\r\n", version[0]);
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
    int data_len;      /* dsp���Խ������*/     
    char result[500]; /* dsp���Խ����Ϣ */
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
    {TEST_BBP_DSP_CPU, "CPU-DSP ��·", &g_cpu_dsp_sem, 30},
    {TEST_BBP_DSP_DDR, "DSP-DDR ����", &g_dsp_ddr_sem, 300},
    {TEST_BBP_SRIO_DATA, "DSP-DSP SRIO ����", &g_srio_data_sem, 30},
    {TEST_BBP_SRIO_EFFI, "DSP-DSP SRIO Ч��", &g_srio_effi_sem, 30},
    {TEST_BBP_AIF, "DSP AIF", &g_dsp_aif_sem, 30},
    {TEST_BBP_UP, "������·", &g_dsp_up_sem, 30},
    {TEST_BBP_DOWN, "������·", &g_dsp_down_sem, 30},
    {TEST_SYNC_START, "ͬ����", &g_sync_start_sem, 30},
    {TEST_SYNC_STOP, "ͬ��ͣ", &g_sync_stop_sem, 30},
    {TEST_IR, "����ʱ����", &g_ir_delay_sem, 30},
    {TEST_BOARDS_SRIO, "���SRIO ", &g_boards_srio_sem, 30},
    {TEST_BBP_DSP_VERION, "DSP�汾", &g_dsp_ver_sem, 30}
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
        //�ж��Ƿ��ǰ��SRIO���ݲ���
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
            //��ȡ������
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
    //��ʼ���ź���
    sem_init(strCpuDspTestCont[index].sem,0,0);
    send_msg(cmd, "��ʼ%s(TESTID:0x%04x)����!\r\n", strCpuDspTestCont[index].name, testid);

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
                send_msg(cmd, "SLOT%d CPU DSP--dsp %d ����ʧ��!\r\n",u8BbpSlot, dspid);
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
        /* ����ϵͳ�ĵ�ǰʱ�丳ֵ�ź����ĵȴ�ʱ�� */
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
                send_msg(cmd, "SLOT%d DSP1 %s���Բ�ͨ��!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
            }          
            send_msg(cmd, "SLOT%d DSP1 %s���Խ��%s!\r\n", u8BbpSlot,strCpuDspTestCont[index].name, cpudspresult[0].result);
        }
        else
        {
            send_msg(cmd, "SLOT%d DSP1 %s���Գ�ʱ!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
        }
        if(cpudspresult[1].dspid == 2)
        {            
            if(cpudspresult[1].res == 1)
                dspres2 = 1;
            else
            {
                send_msg(cmd, "SLOT%d DSP2 %s���Բ�ͨ��!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
            } 
            send_msg(cmd, "SLOT%d DSP2 %s���Խ��%s!\r\n", u8BbpSlot,strCpuDspTestCont[index].name, cpudspresult[1].result);
        }
        else
        {
            send_msg(cmd, "SLOT%d DSP2 %s���Գ�ʱ!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
        }
        if(cpudspresult[2].dspid == 3)
        {
            if(cpudspresult[2].res == 1)
                dspres3 = 1;
            else
            {
                send_msg(cmd, "SLOT%d DSP3 %s���Բ�ͨ��!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
            }
            send_msg(cmd, "SLOT%d DSP3 %s���Խ��%s!\r\n", u8BbpSlot,strCpuDspTestCont[index].name, cpudspresult[2].result);
        }
        else
        {
            send_msg(cmd, "SLOT%d DSP3 %s���Գ�ʱ!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
        }
        if((TEST_BBP_AIF != testid) && (TEST_BBP_UP != testid) && (TEST_BBP_DOWN != testid))
        {
            if(cpudspresult[3].dspid == 4)
            {
                if(cpudspresult[3].res == 1)
                    dspres4 = 1;
                else
                {
                    send_msg(cmd, "SLOT%d DSP4 %s���Բ�ͨ��!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
                }
                send_msg(cmd, "SLOT%d DSP4 %s���Խ��%s!\r\n", u8BbpSlot,strCpuDspTestCont[index].name, cpudspresult[3].result);
            }
            else
            {
                send_msg(cmd, "SLOT%d DSP4 %s���Գ�ʱ!\r\n", u8BbpSlot,strCpuDspTestCont[index].name);
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
        send_msg(cmd, " %s����--slot%d dsp1 ����%s!\r\n", strCpuDspTestCont[index].name, u8BbpSlot, ((dspres1==1)?"�ɹ�":"ʧ��"));
        send_msg(cmd, " %s����--slot%d dsp2 ����%s!\r\n", strCpuDspTestCont[index].name, u8BbpSlot, ((dspres2==1)?"�ɹ�":"ʧ��"));
        send_msg(cmd, " %s����--slot%d dsp3 ����%s!\r\n", strCpuDspTestCont[index].name, u8BbpSlot, ((dspres3==1)?"�ɹ�":"ʧ��"));
        if(TEST_BBP_AIF != testid && TEST_BBP_UP != testid && TEST_BBP_DOWN != testid)
        {
            send_msg(cmd, " %s����--slot%d dsp4 ����%s!\r\n", strCpuDspTestCont[index].name, u8BbpSlot, ((dspres4==1)?"�ɹ�":"ʧ��"));
        }
        if(success == 1)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, " %s���Գɹ�!\r\n", strCpuDspTestCont[index].name);
        }
        else
        {
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            send_msg(cmd, " %s����ʧ��!\r\n", strCpuDspTestCont[index].name);
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
            send_msg(cmd, " gps ���Գɹ�!\r\n");
            send_msg(cmd, "��������%d����׷������%d!\r\n", bsp_gps_VisibleSatellites(),bsp_gps_TrackedSatellites());
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " gps ����ʧ��!\r\n");
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
            send_msg(cmd, " usb��д ���Գɹ�!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " usb��д ����ʧ��!\r\n");
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
            send_msg(cmd, " sfp0�� ���Գɹ�!\r\n");
            send_msg(cmd, "sfp0 ��Ϣ:\r\n");
            send_msg(cmd, "��ѹ%6.2f(uV), ����%6.2f(uA), ���͹���%6.2f(uW), ���չ���%6.2f(uW), ������Ϣ%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
            if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
            {
                send_msg(cmd, " sfp0��ģ����Գɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp0���͹��ʲ���ʧ��!\r\n");
                }
                if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp0���չ��ʲ���ʧ��!\r\n");
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " sfp0�� ����ʧ��!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if(bsp_get_fiber_Info_mutex(0, 1, &fiberdata)==BSP_OK)
        {
            send_msg(cmd, " sfp1�� ���Գɹ�!\r\n");
            send_msg(cmd, "sfp1 ��Ϣ:\r\n");
            send_msg(cmd, "��ѹ%6.2f(uV), ����%6.2f(uA), ���͹���%6.2f(uW), ���չ���%6.2f(uW), ������Ϣ%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
            if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
            {
                send_msg(cmd, " sfp1��ģ����Գɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp1���͹��ʲ���ʧ��!\r\n");
                }
                if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp1���չ��ʲ���ʧ��!\r\n");
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " sfp1�� ����ʧ��!\r\n");
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
            send_msg(cmd, " ������ ���Գɹ�!\r\n");
            send_msg(cmd, "��ѹ%f, ����%f, ����%f \r\n", power[0], power[1], power[2]);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " �����ʲ���ʧ��!\r\n");
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
            send_msg(cmd, " ��eeprom ���Գɹ�!\r\n");
            //ucharxtostr(u8Buff, as8Str, 128);
            //send_msg(cmd, "eeprom ��Ϣ:\r\n");
            //send_msg(cmd, "%s \r\n", as8Str);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " ��eeprom ����ʧ��!\r\n");
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
            send_msg(cmd, " IPRANPHY1���Գɹ�!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " IPRANPHY1����ʧ��!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if(bsp_phy_bcm54210s_test(0x09)==BSP_OK)
        {
            send_msg(cmd, " IPRANPHY2���Գɹ�!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " IPRANPHY2����ʧ��!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if(bsp_phy_bcm54210s_test(0x19)==BSP_OK)
        {
            send_msg(cmd, " TRACEPHY���Գɹ�!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " TRACEPHY����ʧ��!\r\n");
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
        /* ����RJ45 ͬ��ԴPP1S*/
        printf("change to pp1s...\r\n");
        bsp_cpld_write_reg(102, 1);
        printf("read data from uart2...\r\n");
        if(bsp_get_tod_status()==BSP_OK)
        {
            send_msg(cmd, " ��ͬ���ӿ�pp1s���Գɹ�!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " ��ͬ���ӿ�pp1s����ʧ��!\r\n");
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
        /* ����RJ45 ͬ��ԴTOD*/
        printf("change to tod...\r\n");
        bsp_cpld_write_reg(102, 0);
        printf("read data from uart2...\r\n");
        if(bsp_get_tod_status()==BSP_OK)
        {
            send_msg(cmd, " ��ͬ���ӿ�tod���Գɹ�!\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " ��ͬ���ӿ�tod����ʧ��!\r\n");
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


/*****************�����������***************************/
cmd_t test_bbp_temprature(cmd_t cmd)
{
    int times = 0;
    u8 u8BbpSlot = 0;
    u8 u8BoardType = 0;

    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //��ȡ������
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times<htonl(cmd.pkg_totaltimes)+1; times++)
    {
        int i;
        int8_t temp[8]={0};
        int max_temp_id = 0,min_temp_id = 0;
        uint32_t success = 1;
        //��ȡ�¶�
        if(bsp_bbp_read_temp(u8BbpSlot,temp)!=BSP_OK)
        {
            success = 0;
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "��ȡ������(slot %d)(1~8)���¶Ȼ�ȡʧ��!\r\n",u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "��ȡ������(slot %d)(1~4)���¶Ȼ�ȡʧ��!\r\n",u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "��ȡͬ����(slot %d)�¶Ȼ�ȡʧ��!\r\n",u8BbpSlot);
        }
        if(BOARD_TYPE_BBP == u8BoardType)
        {
            //�õ�����¶Ⱥ���С�¶�ֵ
            for(i = 0; i < 8; i++)
            {
                //�ҳ����ͻ���ߵ��¶�ֵ
                if( (temp[i] < -25) || (temp[i] > 90))
                {
                    success = 0;
                    send_msg(cmd, "������(slot %d)�¶�%d(1~8) %d����(ʧ��)!\r\n", u8BbpSlot, (i+1), temp[i]);
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
            //�õ�����¶Ⱥ���С�¶�ֵ
            for(i = 0; i < 4; i++)
            {
                //�ҳ����ͻ���ߵ��¶�ֵ
                if( (temp[i] < -25) || (temp[i] > 90))
                {
                    success = 0;
                    send_msg(cmd, "������(slot %d)�¶�%d(1~8) %d����(ʧ��)!\r\n", u8BbpSlot, (i+1), temp[i]);
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
            //����¶Ⱥ���С�¶Ȳ�ֵ
            if((temp[max_temp_id]-temp[min_temp_id])>25)
            {
                success = 0;
                send_msg(cmd, "(slot %d)�²����(ʧ��)!\r\n",u8BbpSlot);
            }
        }
        if(BOARD_TYPE_BBP == u8BoardType)
            send_msg(cmd, "������(slot %d)�¶�(1~8)=%d,%d,%d,%d,%d,%d,%d,%d\r\n",u8BbpSlot,temp[0],temp[1],temp[2],temp[3], temp[4],temp[5],temp[6],temp[7]);
        if(BOARD_TYPE_FSA == u8BoardType)
            send_msg(cmd, "������(slot %d)�¶�(1~4)=%d,%d,%d,%d\r\n",u8BbpSlot,temp[0],temp[1],temp[2],temp[3]);
        if(BOARD_TYPE_ES == u8BoardType)
            send_msg(cmd, "ͬ����(slot %d)�¶�=%d\r\n",u8BbpSlot, *(int16_t *)(temp));
        if(success == 1)
        {
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            send_msg(cmd, "(slot %d)�¶�����!\r\n", u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //��ȡ������
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
                send_msg(cmd, " ������(slot %d)Power����������ѹ=%f, ����=%f, ����=%f\r\n",u8BbpSlot, power[0],power[1],power[2]);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " ������(slot %d)Power����������ѹ=%f, ����=%f, ����=%f\r\n",u8BbpSlot, power[0],power[1],power[2]);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " ��ȡ������(slot %d)Power������ʧ��!\r\n",u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " ��ȡ������(slot %d)Power������ʧ��!\r\n",u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //��ȡ������
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_bbp_arm_version(u8BbpSlot, boards[u8BbpSlot].arm_version)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "������(slot %d)arm version = %s\r\n", u8BbpSlot,boards[u8BbpSlot].arm_version);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "������(slot %d)arm version = %s\r\n", u8BbpSlot,boards[u8BbpSlot].arm_version);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "ͬ����(slot %d)arm version = %s\r\n", u8BbpSlot,boards[u8BbpSlot].arm_version);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "������(slot %d)arm version��ȡʧ��!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "������(slot %d)arm version��ȡʧ��!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "ͬ����(slot %d)arm version��ȡʧ��!\r\n", u8BbpSlot);
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

    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //��ȡ������
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
            send_msg(cmd, "������(slot %d)���ͬ�����Կ�ʼ!\r\n", u8BbpSlot);
        if(BOARD_TYPE_FSA == u8BoardType)
            send_msg(cmd, "������(slot %d)���ͬ�����Կ�ʼ!\r\n", u8BbpSlot);        
        //���ù�ڻػ�
        //bsp_bbp_fpga_write(u8BbpSlot, 205, 1);
        if(BOARD_TYPE_BBP == u8BoardType)
        {
            //���ͬ��״̬
            bsp_bbp_fpga_write(u8BbpSlot, 11, 0xffff);
            //��ȡͬ��״̬
            bsp_bbp_fpga_read(u8BbpSlot, 11, &u16IrSyncStat);
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            //���ͬ��״̬
            bsp_fsa_fpga_write(u8BbpSlot, FSA_FPGA_325T, 11, 0xffff);
            //��ȡͬ��״̬
            bsp_fsa_fpga_read(u8BbpSlot, FSA_FPGA_325T, 11, &u16IrSyncStat);
        }
        if(BOARD_TYPE_BBP == u8BoardType)
        {
            if((u16IrSyncStat & 0x07) == 0x07)
            {
                send_msg(cmd, "������(slot %d)���ͬ�����Գɹ�!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {            
                send_msg(cmd, "������(slot %d)���ͬ��״̬u16IrSyncStat = 0x%x.\r\n", u8BbpSlot, u16IrSyncStat);
                if((u16IrSyncStat & 0x01) != 0x01)
                {                
                    send_msg(cmd, "������(slot %d)���0ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x02) != 0x02)
                {                
                    send_msg(cmd, "������(slot %d)���1ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x04) != 0x04)
                {                
                    send_msg(cmd, "������(slot %d)���2ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            if((u16IrSyncStat & 0x3F) == 0x3F)
            {
                send_msg(cmd, "������(slot %d)���ͬ�����Գɹ�!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {            
                send_msg(cmd, "������(slot %d)���ͬ��״̬u16IrSyncStat = 0x%x.\r\n", u8BbpSlot, u16IrSyncStat);
                if((u16IrSyncStat & 0x01) != 0x01)
                {                
                    send_msg(cmd, "������(slot %d)���0ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x02) != 0x02)
                {                
                    send_msg(cmd, "������(slot %d)���1ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x04) != 0x04)
                {                
                    send_msg(cmd, "������(slot %d)���2ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x08) != 0x08)
                {                
                    send_msg(cmd, "������(slot %d)���3ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x10) != 0x10)
                {                
                    send_msg(cmd, "������(slot %d)���4ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                if((u16IrSyncStat & 0x20) != 0x20)
                {                
                    send_msg(cmd, "������(slot %d)���5ͬ������ʧ��.\r\n", u8BbpSlot);
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        //ȡ����ڻػ�
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

    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //��ȡ������
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_bbp_get_sfpinfo(u8BbpSlot, 0, &fiberdata)==BSP_OK)
        {
            send_msg(cmd, " sfp0�� ���Գɹ�!\r\n");
            send_msg(cmd, "sfp0 ��Ϣ:\r\n");
            send_msg(cmd, "��ѹ%6.2f(uV), ����%6.2f(uA), ���͹���%6.2f(uW), ���չ���%6.2f(uW), ������Ϣ%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
            if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
            {
                send_msg(cmd, " sfp0��ģ����Գɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp0���͹��ʲ���ʧ��!\r\n");
                }
                if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp0���չ��ʲ���ʧ��!\r\n");
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " sfp0�� ����ʧ��!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if(bsp_bbp_get_sfpinfo(u8BbpSlot, 1, &fiberdata)==BSP_OK)
        {
            send_msg(cmd, " sfp1�� ���Գɹ�!\r\n");
            send_msg(cmd, "sfp1 ��Ϣ:\r\n");
            send_msg(cmd, "��ѹ%6.2f(uV), ����%6.2f(uA), ���͹���%6.2f(uW), ���չ���%6.2f(uW), ������Ϣ%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
            if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
            {
                send_msg(cmd, " sfp1��ģ����Գɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp1���͹��ʲ���ʧ��!\r\n");
                }
                if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                {
                    send_msg(cmd, " sfp1���չ��ʲ���ʧ��!\r\n");
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " sfp1�� ����ʧ��!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        if((BOARD_TYPE_BBP == u8BoardType) || (BOARD_TYPE_FSA == u8BoardType))
        {
            if(bsp_bbp_get_sfpinfo(u8BbpSlot, 2, &fiberdata)==BSP_OK)
            {
                send_msg(cmd, " sfp2�� ���Գɹ�!\r\n");
                send_msg(cmd, "sfp2 ��Ϣ:\r\n");
                send_msg(cmd, "��ѹ%6.2f(uV), ����%6.2f(uA), ���͹���%6.2f(uW), ���չ���%6.2f(uW), ������Ϣ%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
                if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                    && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
                {
                    send_msg(cmd, " sfp2��ģ����Գɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp2���͹��ʲ���ʧ��!\r\n");
                    }
                    if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp2���չ��ʲ���ʧ��!\r\n");
                    }
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " sfp2�� ����ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        if(BOARD_TYPE_FSA == u8BoardType)
        {
            if(bsp_bbp_get_sfpinfo(u8BbpSlot, 3, &fiberdata)==BSP_OK)
            {
                send_msg(cmd, " sfp3�� ���Գɹ�!\r\n");
                send_msg(cmd, "sfp3 ��Ϣ:\r\n");
                send_msg(cmd, "��ѹ%6.2f(uV), ����%6.2f(uA), ���͹���%6.2f(uW), ���չ���%6.2f(uW), ������Ϣ%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
                if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                    && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
                {
                    send_msg(cmd, " sfp3��ģ����Գɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp3���͹��ʲ���ʧ��!\r\n");
                    }
                    if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp3���չ��ʲ���ʧ��!\r\n");
                    }
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " sfp3�� ����ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            if(bsp_bbp_get_sfpinfo(u8BbpSlot, 4, &fiberdata)==BSP_OK)
            {
                send_msg(cmd, " sfp4�� ���Գɹ�!\r\n");
                send_msg(cmd, "sfp4 ��Ϣ:\r\n");
                send_msg(cmd, "��ѹ%6.2f(uV), ����%6.2f(uA), ���͹���%6.2f(uW), ���չ���%6.2f(uW), ������Ϣ%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
                if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                    && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
                {
                    send_msg(cmd, " sfp4��ģ����Գɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp4���͹��ʲ���ʧ��!\r\n");
                    }
                    if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp4���չ��ʲ���ʧ��!\r\n");
                    }
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " sfp4�� ����ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            if(bsp_bbp_get_sfpinfo(u8BbpSlot, 5, &fiberdata)==BSP_OK)
            {
                send_msg(cmd, " sfp5�� ���Գɹ�!\r\n");
                send_msg(cmd, "sfp5 ��Ϣ:\r\n");
                send_msg(cmd, "��ѹ%6.2f(uV), ����%6.2f(uA), ���͹���%6.2f(uW), ���չ���%6.2f(uW), ������Ϣ%s\r\n",fiberdata.vol,fiberdata.current,fiberdata.tx_power,fiberdata.rx_power,fiberdata.vendor_name);
                if(((fiberdata.tx_power < SFP_MAX) && (fiberdata.tx_power > SFP_MIN))
                    && ((fiberdata.rx_power < SFP_MAX) && (fiberdata.rx_power > SFP_MIN)))
                {
                    send_msg(cmd, " sfp5��ģ����Գɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((fiberdata.tx_power > SFP_MAX) || (fiberdata.tx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp5���͹��ʲ���ʧ��!\r\n");
                    }
                    if((fiberdata.rx_power > SFP_MAX) || (fiberdata.rx_power < SFP_MIN))
                    {
                        send_msg(cmd, " sfp5���չ��ʲ���ʧ��!\r\n");
                    }
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " sfp5�� ����ʧ��!\r\n");
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    } 
    //��ȡ������
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(BOARD_TYPE_BBP == u8BoardType)
            send_msg(cmd, "������(slot %d)EEPROM����...\r\n", u8BbpSlot);
        if(BOARD_TYPE_FSA == u8BoardType)
            send_msg(cmd, "������(slot %d)EEPROM����...\r\n", u8BbpSlot);
        if(BOARD_TYPE_ES == u8BoardType)
            send_msg(cmd, "ͬ����(slot %d)EEPROM����...\r\n", u8BbpSlot);
        if(bsp_bbp_test_eeprom(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "������(slot %d)EEPROM���Գɹ�!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "������(slot %d)EEPROM���Գɹ�!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "ͬ����(slot %d)EEPROM���Գɹ�!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "������(slot %d)EEPROM����ʧ��!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "������(slot %d)EEPROM����ʧ��!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "ͬ����(slot %d)EEPROM����ʧ��!\r\n", u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    } 
    //��ȡ������
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(BOARD_TYPE_BBP == u8BoardType)
            send_msg(cmd, "������(slot %d)SDRAM����...!\r\n", u8BbpSlot);
        if(BOARD_TYPE_FSA == u8BoardType)
            send_msg(cmd, "������(slot %d)SDRAM����...!\r\n", u8BbpSlot);
        if(BOARD_TYPE_ES == u8BoardType)
            send_msg(cmd, "ͬ����(slot %d)SDRAM����...!\r\n", u8BbpSlot);
        if(bsp_bbp_test_sdram(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "������(slot %d)SDRAM���Գɹ�!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "������(slot %d)SDRAM���Գɹ�!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "ͬ����(slot %d)SDRAM���Գɹ�!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "������(slot %d)SDRAM����ʧ��!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "������(slot %d)SDRAM����ʧ��!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "ͬ����(slot %d)SDRAM����ʧ��!\r\n", u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    } 
    //��ȡ������
    u8BoardType = boards[u8BbpSlot].type;
    bsp_mutex_lock(cmd_list[cmd.index].plock);
    for(times=1; times< htonl(cmd.pkg_totaltimes)+1; times++)
    {
        if(bsp_bbp_test_ethsw(u8BbpSlot)==BSP_OK)
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " ������(slot %d)Ethsw���Գɹ�!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " ������(slot %d)Ethsw���Գɹ�!\r\n", u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, " ͬ����(slot %d)Ethsw���Գɹ�!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
             if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " ������(slot %d)Ethsw����ʧ��!\r\n", u8BbpSlot);
             if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " ������(slot %d)Ethsw����ʧ��!\r\n", u8BbpSlot);
             if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, " ͬ����(slot %d)Ethsw����ʧ��!\r\n", u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //��ȡ������
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
                send_msg(cmd, " ������(slot %d)srioswt·�ɲ��Գɹ�!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " ������(slot %d)srioswt·�ɲ��Գɹ�!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, " ������(slot %d)srioswt·�ɲ���ʧ��!\r\n", u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, " ������(slot %d)srioswt·�ɲ���ʧ��!\r\n", u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_fsa_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //��ȡ������
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
            send_msg(cmd, " ������(slot %d)phy���Գɹ�!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " ������(slot %d)phy����ʧ��!\r\n", u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa or es slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //��ȡ������
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
                send_msg(cmd, " ������(slot %d)pll1���ò��Գɹ�!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ������(slot %d)pll1���ò���ʧ��!\r\n", u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            #if 0
            if(bsp_test_pll2_config(u8BbpSlot)==BSP_OK)
            {
                send_msg(cmd, " ������(slot %d)pll2���ò��Գɹ�!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ������(slot %d)pll2���ò���ʧ��!\r\n", u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
            #endif
        }
        if(BOARD_TYPE_ES == u8BoardType)
        {
            if(bsp_test_pll1_config(u8BbpSlot)==BSP_OK)
            {
                send_msg(cmd, " ͬ����(slot %d)pll1���ò��Գɹ�!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ͬ����(slot %d)pll1���ò���ʧ��!\r\n", u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_fsa_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //��ȡ������
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
        //��ʼ����DDR,д1M���ݺ�����Ƚ�
        bsp_fsa_fpga_write(u8BbpSlot, FSA_FPGA_325T, 13, 1);
        //�ȴ�2��
        sleep(2);
        //�����Խ��(1��ȷ,0�쳣)
        bsp_fsa_fpga_read(u8BbpSlot, FSA_FPGA_325T, 222, &u16ret);
        if(u16ret == 1)
        {
            send_msg(cmd, " ������(slot %d)fpga_325t DDR���Գɹ�!\r\n", u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " ������(slot %d)fpga_325t DDR����ʧ��!\r\n", u8BbpSlot);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        //ֹͣ���Բ����DDR���Խ��
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_fsa_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }
    //��ȡ������
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
                send_msg(cmd, " ������(slot %d)fpga_160t���ٽӿڲ��Գɹ�!\r\n", u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ������(slot %d)fpga_160t���ٽӿ�ͬ��״̬0x%x!\r\n", u8BbpSlot, u16status);
                if((u16status&0x01) != 0x01)                    
                    send_msg(cmd, " ������(slot %d)fpga_160t���ٽӿ�10Gʧ��!\r\n", u8BbpSlot);
                if((u16status&0x02) != 0x02)                    
                    send_msg(cmd, " ������(slot %d)fpga_160t���ٽӿ�SRIO1ʧ��!\r\n", u8BbpSlot);
                if((u16status&0x04) != 0x04)                    
                    send_msg(cmd, " ������(slot %d)fpga_160t���ٽӿ�SRIO0ʧ��!\r\n", u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ������(slot %d)fpga_160t���ٽӿڲ���ʧ��!\r\n", u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error es slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }     
    //��ȡ������
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
        send_msg(cmd, "ͬ����(slot %d)��ڲ��Կ�ʼ\r\n",u8BbpSlot);
        //���õ��2���1PPS��TOD������
        bsp_bbp_fpga_write(u8BbpSlot, 8, 1);
        //���õ��1Ϊ�����2Ϊ����
        bsp_bbp_fpga_write(u8BbpSlot, 6, 5);
        sleep(1);
        for(u8cnt=0; u8cnt<u8timeout;u8cnt++)
        {
            if(0x11 == bsp_cpld_read_reg(124))
                break;
            sleep(1);
        }
        //�ж����ذ��Ƿ��յ�1PPS��TOD
        u8mcts_tsync = bsp_cpld_read_reg(124);
        send_msg(cmd, "ͬ����(slot %d)���ͬ��״̬:0x%x\r\n", u8BbpSlot, u8mcts_tsync);
        if((u8mcts_tsync&0x11) == 0x11)
        {
            send_msg(cmd, "ͬ����(slot %d)��ڷ���1PPS��TOD�ɹ�\r\n",u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            if((u8mcts_tsync&0x01) != 0x01)
                send_msg(cmd, "ͬ����(slot %d)��ڷ���TODʧ��\r\n",u8BbpSlot);
            if((u8mcts_tsync&0x10) != 0x10)
                send_msg(cmd, "ͬ����(slot %d)��ڷ���1PPSʧ��\r\n",u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error es slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }     
    //��ȡ������
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
        send_msg(cmd, "ͬ����(slot %d)��ڲ��Կ�ʼ\r\n",u8BbpSlot);
        //���ù��2���1PPS��TOD������
        bsp_bbp_fpga_write(u8BbpSlot, 8, 3);
        //���������ʹ�ܿ�
        bsp_bbp_fpga_write(u8BbpSlot, 6, 4);
        sleep(1);
        for(u8cnt=0; u8cnt<u8timeout;u8cnt++)
        {
            if(0x11 == bsp_cpld_read_reg(124))
                break;
            sleep(1);
        }        
        //�ж����ذ��Ƿ��յ�1PPS��TOD
        u8mcts_tsync = bsp_cpld_read_reg(124);
        send_msg(cmd, "ͬ����(slot %d)���ͬ��״̬:0x%x\r\n", u8BbpSlot, u8mcts_tsync);
        if((u8mcts_tsync&0x11) == 0x11)
        {
            send_msg(cmd, "ͬ����(slot %d)��ڷ���1PPS��TOD�ɹ�\r\n",u8BbpSlot);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {            
            if((u8mcts_tsync&0x01) != 0x01)
                send_msg(cmd, "ͬ����(slot %d)��ڷ���TODʧ��\r\n",u8BbpSlot);
            if((u8mcts_tsync&0x10) != 0x10)
                send_msg(cmd, "ͬ����(slot %d)��ڷ���1PPSʧ��\r\n",u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return cmd;
    }     
    //��ȡ������
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

        send_msg(cmd, "������(slot %d)CPLD Version:%02x%02x%02x%02x\r\n",u8BbpSlot, version[0], version[1], version[2], version[3]);
        if(version[0]<0x10 || version[0]>0x24)
        {
            send_msg(cmd, "��������(slot %d)CPLD�汾ʧ��!(��%x)\r\n",u8BbpSlot, version[0]);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }
    //��ȡ������
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
                send_msg(cmd, "������(slot %d)CPLD���³ɹ�!\r\n",u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "������(slot %d)CPLD����ʧ��!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "������(slot %d)CPLD����ʧ��!\r\n",u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }
    //��ȡ������
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
                        send_msg(cmd, "������(slot %d)FPGA���سɹ�!\r\n",u8BbpSlot);
                    if(BOARD_TYPE_ES == u8BoardType)
                        send_msg(cmd, "ͬ����(slot %d)FPGA���سɹ�!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if(BOARD_TYPE_BBP == u8BoardType)
                        send_msg(cmd, "������(slot %d)FPGA����ʧ��!\r\n",u8BbpSlot);
                    if(BOARD_TYPE_ES == u8BoardType)
                        send_msg(cmd, "ͬ����(slot %d)FPGA����ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, "������(slot %d)FPGA����ʧ��!\r\n",u8BbpSlot);
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, "ͬ����(slot %d)FPGA����ʧ��!\r\n",u8BbpSlot);
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
                    send_msg(cmd, "������(slot %d)FPGA_325T���سɹ�!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, "������(slot %d)FPGA_325T����ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, "������(slot %d)FPGA_325T����ʧ��!\r\n",u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_fsa_slot(cmd)!=0)
    {
        send_msg(cmd, "error fsa slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }
    //��ȡ������
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
                send_msg(cmd, "������(slot %d)FPGA_160T���سɹ�!\r\n",u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "������(slot %d)FPGA_160T����ʧ��!\r\n",u8BbpSlot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "������(slot %d)FPGA_160T����ʧ��!\r\n",u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    }
    //��ȡ������
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
                send_msg(cmd, "������(slot %d)MCU����...\r\n",u8BbpSlot);
                while((bbp_boot_over!=1) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(1);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
                    &&(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED)
                    &&(boards[u8BbpSlot].dsp_isready == 0x0F))
                {
                    send_msg(cmd, "������(slot %d)MCU���³ɹ�!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, "������(slot %d)MCU����ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            if(BOARD_TYPE_FSA == u8BoardType)
            {
                send_msg(cmd, "������(slot %d)MCU����...\r\n",u8BbpSlot);
                while((boards[u8BbpSlot].fpga_status != 0x30) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(1);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)&&(boards[u8BbpSlot].fpga_status == 0x30))
                {
                    send_msg(cmd, "������(slot %d)MCU���³ɹ�!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, "������(slot %d)MCU����ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            if(BOARD_TYPE_ES == u8BoardType)
            {
                send_msg(cmd, "ͬ����(slot %d)MCU����...\r\n",u8BbpSlot);
                while((boards[u8BbpSlot].fpga_status != DSP_FPGA_STATE_LOADED) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(1);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)&&(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED))
                {
                    send_msg(cmd, "ͬ����(slot %d)MCU���³ɹ�\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, "ͬ����(slot %d)MCU����ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "������(slot %d)MCU����ʧ��!\r\n",u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "������(slot %d)MCU����ʧ��!\r\n",u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "ͬ����(slot %d)MCU����ʧ��!\r\n",u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    } 
    //��ȡ������
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
                send_msg(cmd, "������(slot %d)��λ��...\r\n",u8BbpSlot);
                while((bbp_boot_over != 1) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(2);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
                    &&(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED)
                    &&(boards[u8BbpSlot].dsp_isready == 0x0F))
                {
                    send_msg(cmd, "������(slot %d)��������!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((boards[u8BbpSlot].mcu_status&0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                    {
                        send_msg(cmd, " ������(slot %d)bbp����ʧ��!\r\n",u8BbpSlot);
                    }
                    if(boards[u8BbpSlot].fpga_status != DSP_FPGA_STATE_LOADED)
                    {
                        send_msg(cmd, " ������(slot %d)fpga����ʧ��!\r\n",u8BbpSlot);
                    }
                    if(boards[u8BbpSlot].dsp_isready != 0x0F)
                    {
                        if(boards[u8BbpSlot].dsp_isready != 0x1)
                        {
                            send_msg(cmd, " ������(slot %d)DSP1����ʧ��!\r\n",u8BbpSlot);
                        }
                        if(boards[u8BbpSlot].dsp_isready != 0x2)
                        {
                            send_msg(cmd, " ������(slot %d)DSP2����ʧ��!\r\n",u8BbpSlot);
                        }
                        if(boards[u8BbpSlot].dsp_isready != 0x3)
                        {
                            send_msg(cmd, " ������(slot %d)DSP3����ʧ��!\r\n",u8BbpSlot);
                        }
                        if(boards[u8BbpSlot].dsp_isready != 0x4)
                        {
                            send_msg(cmd, " ������(slot %d)DSP4����ʧ��!\r\n",u8BbpSlot);
                        }
                        send_msg(cmd, " ������(slot %d)DSP����ʧ��!\r\n",u8BbpSlot);
                    }
                    send_msg(cmd, "������(slot %d)����ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            if(BOARD_TYPE_FSA == u8BoardType)
            {
                send_msg(cmd, "������(slot %d)��λ��...\r\n",u8BbpSlot);
                while((boards[u8BbpSlot].fpga_status != 0x30) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(2);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)&&(boards[u8BbpSlot].fpga_status == 0x30))
                {
                    send_msg(cmd, "������(slot %d)��������!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((boards[u8BbpSlot].mcu_status&0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                    {
                        send_msg(cmd, " ������(slot %d)����ʧ��!\r\n",u8BbpSlot);
                    }
                    if((boards[u8BbpSlot].fpga_status&FSA_FPGA_325T_LOADED) != FSA_FPGA_325T_LOADED)
                    {
                        send_msg(cmd, " ������(slot %d)fpga_325t����ʧ��!\r\n",u8BbpSlot);
                    }
                    if((boards[u8BbpSlot].fpga_status&FSA_FPGA_160T_LOADED) != FSA_FPGA_160T_LOADED)
                    {
                        send_msg(cmd, " ������(slot %d)fpga_160t����ʧ��!\r\n",u8BbpSlot);
                    }
                    send_msg(cmd, "������(slot %d)����ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            if(BOARD_TYPE_ES == u8BoardType)
            {
                send_msg(cmd, "ͬ����(slot %d)��λ��...\r\n",u8BbpSlot);
                while((boards[u8BbpSlot].fpga_status != DSP_FPGA_STATE_LOADED) && (wait_time-- > 0))
                {
                    sleep(1);
                }
                sleep(2);
                if(((boards[u8BbpSlot].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)&&(boards[u8BbpSlot].fpga_status == DSP_FPGA_STATE_LOADED))
                {
                    send_msg(cmd, "ͬ����(slot %d)��������!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((boards[u8BbpSlot].mcu_status&0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                    {
                        send_msg(cmd, " ͬ����(slot %d)����ʧ��!\r\n",u8BbpSlot);
                    }
                    if(boards[u8BbpSlot].fpga_status != DSP_FPGA_STATE_LOADED)
                    {
                        send_msg(cmd, " ͬ����(slot %d)fpga����ʧ��!\r\n",u8BbpSlot);
                    }
                    send_msg(cmd, "ͬ����(slot %d)����ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
        }
        else
        {
            if(BOARD_TYPE_BBP == u8BoardType)
                send_msg(cmd, "������(slot %d)��λʧ��!\r\n",u8BbpSlot);
            if(BOARD_TYPE_FSA == u8BoardType)
                send_msg(cmd, "������(slot %d)��λʧ��!\r\n",u8BbpSlot);
            if(BOARD_TYPE_ES == u8BoardType)
                send_msg(cmd, "ͬ����(slot %d)��λʧ��!\r\n",u8BbpSlot);
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
    
    //��ȡ��λ��
    u8BbpSlot = bsp_get_bbp_slot(cmd);
    if(check_bbp_slot(cmd)!=0)
    {
        send_msg(cmd, "error bbp slot id(%d).\r\n", u8BbpSlot);
        return NULL;
    } 
    //��ȡ������
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
                send_msg(cmd, " ������(slot %d)DSP���سɹ�!\r\n",u8BbpSlot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(boards[u8BbpSlot].dsp_isready != 0x1)
                {
                    send_msg(cmd, " ������(slot %d)DSP1����ʧ��-no ready!\r\n",u8BbpSlot);
                }
                 if(boards[u8BbpSlot].dsp_isready != 0x2)
                {
                    send_msg(cmd, " ������(slot %d)DSP2����ʧ��-no ready!\r\n",u8BbpSlot);
                }
                 if(boards[u8BbpSlot].dsp_isready != 0x3)
                {
                    send_msg(cmd, " ������(slot %d)DSP3����ʧ��-no ready!\r\n",u8BbpSlot);
                }
                 if(boards[u8BbpSlot].dsp_isready != 0x4)
                {
                    send_msg(cmd, " ������(slot %d)DSP4����ʧ��-no ready!\r\n",u8BbpSlot);
                }
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ������(slot %d)DSP����ʧ��!\r\n",u8BbpSlot);
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

/*****************���Ȱ������***************************/
void *fan_reset_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    uint8_t u8Slot = 0;
    
    //��ȡ��λ��
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
            send_msg(cmd, " ���Ȱ帴λ��...\r\n");
            while(((boards[u8Slot].mcu_status & 0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                &&(wait_time-- > 0))
            {
                sleep(1);
            }
            sleep(1);
            if((boards[u8Slot].mcu_status & 0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, " ���Ȱ���������!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���Ȱ�����ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ���Ȱ帴λʧ��!\r\n");
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

    //��ȡ��λ��
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
            send_msg(cmd, "���Ȱ�(slot %d)MCU����...\r\n",u8Slot);
            while(((boards[u8Slot].mcu_status & 0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                &&(wait_time-- > 0))
            {
                sleep(1);
            }
            sleep(1);
            if((boards[u8Slot].mcu_status & 0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "���Ȱ�(slot %d)MCU���³ɹ�!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "���Ȱ�(slot %d)MCU����ʧ��!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "���Ȱ�(slot %d)MCU����ʧ��!\r\n",u8Slot);
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

    //��ȡ��λ��
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
                send_msg(cmd, " ���Ȱ����ת�ٿ��Ƴɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���Ȱ����ת�ٿ���ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ���Ȱ����ת������ʧ��!\r\n");
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

    //��ȡ��λ��
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
                    send_msg(cmd, "ת��(channel 0) = %d.\r\n",g_fan1_speed);
                if(fanchannel == 1)
                    send_msg(cmd, "ת��(channel 1) = %d.\r\n",g_fan2_speed);
                if(fanchannel == 2)
                    send_msg(cmd, "ת��(channel 2) = %d.\r\n",g_fan3_speed);
                if(fanchannel == 3)
                    send_msg(cmd, "ת��(channel 0~2) = %d %d %d.\r\n",
                             g_fan1_speed, g_fan2_speed, g_fan3_speed);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���Ȱ����ת�ٻ�ȡʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ���Ȱ����ת�ٻ�ȡʧ��!\r\n");
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

    //��ȡ��λ��
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
                send_msg(cmd, "���Ȱ�EEPROM���Գɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "���Ȱ�EEPROM����ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "���Ȱ�EEPROM����ʧ��!\r\n");
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

    //��ȡ��λ��
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
                send_msg(cmd, "���Ȱ�arm version = %s\r\n", boards[IPMB_SLOT10].arm_version);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "���Ȱ�arm version��ȡʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "���Ȱ�arm version��ȡʧ��!\r\n");
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
        //�������ת��
        if(bsp_hmi_fan_speed(0, 3, 99)==BSP_OK)
        {
            while((g_fan_speed_set == 0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_fan_speed_set)
            {
                send_msg(cmd, "����ת������Ϊ���ת��!\r\n");
            }
            else
            {
                send_msg(cmd, "�������ת������ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                goto END;
            }
            /* �ȴ�20s�ض�ת�٣��ж�����ת���Ƿ�ɹ� */
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
                    send_msg(cmd, "��ȡ����ת�ٳɹ�!\r\n");
                    send_msg(cmd, "��ȡͨ��1������ת��=%d!\r\n",g_fan1_speed);
                    send_msg(cmd, "��ȡͨ��2������ת��=%d!\r\n",g_fan2_speed);
                    send_msg(cmd, "��ȡͨ��3������ת��=%d!\r\n",g_fan3_speed);
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
                    send_msg(cmd, "��ȡ����ת��ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                    goto END;
                }
            }
            else
            {
                send_msg(cmd, "����ת�ٻ�ȡʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                goto END;
            }
        }
        else
        {
            send_msg(cmd, "���Ȱ����ת������ʧ��!\r\n");
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            goto END;
        }
        cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
END:
        if(cmd.pkg_failtimes != 0)
        {
            send_msg(cmd, "���Ȱ����ʧ��!\r\n");
        }
        /* ���÷���ת��Ϊ60% */
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

/*****************��ذ������***************************/
void *peu_reset_thread(void *arg)
{
    int times = 0;
    cmd_t cmd = *(cmd_t*)arg;
    u8 u8Slot = 0;

     //��ȡ��λ��
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
            send_msg(cmd, " ��ذ�(slot %d)��λ��...\r\n",u8Slot);
            while(((boards[u8Slot].mcu_status & 0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                &&(wait_time-- > 0))
            {
                usleep(1000);
            }
            sleep(1);
            if((boards[u8Slot].mcu_status & 0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, " ��ذ�(slot %d)��������!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ��ذ�(slot %d)����ʧ��!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ��ذ�(slot %d)��λʧ��!\r\n",u8Slot);
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

    //��ȡ��λ��
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
            send_msg(cmd, "��ذ�(slot %d)MCU����...\r\n",u8Slot);
            while(((boards[u8Slot].mcu_status & 0xF0) != MCU_STATUS_RECV_FIRSTMSG)
                &&(wait_time-- > 0))
            {
                sleep(1);
            }
            sleep(1);
            if((boards[u8Slot].mcu_status & 0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "��ذ�(slot %d)MCU���³ɹ�!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "��ذ�(slot %d)MCU����ʧ��!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "��ذ�(slot %d)MCU����ʧ��!\r\n",u8Slot);
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
    
    //��ȡ��λ��
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
            send_msg(cmd, "��ذ�(slot %d)�����µ���...\r\n",u8Slot);
            while((g_peu_power_down_flag==0)&&(wait_time-- > 0))
            {
                usleep(1000);
            }
            if(g_peu_power_down_flag==1)
            {
                send_msg(cmd, "��ذ�(slot %d)�����µ�ɹ�!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "��ذ�(slot %d)�����µ�ʧ��!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "��ذ�(slot %d)�����µ�ʧ��!\r\n",u8Slot);
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

    //��ȡ��λ��
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
                send_msg(cmd, " ��ذ�(slot %d)�ɽ��״̬dryin0123=0x%x, dryin4567=0x%x!\r\n",
                         u8Slot, g_peu_dryin0123, g_peu_dryin4567);
                if((g_peu_dryin0123 == 0) && (g_peu_dryin4567 == 0))
                {
                    send_msg(cmd, " ��ذ�(slot %d)�ɽӵ���Գɹ�!\r\n", u8Slot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    if((g_peu_dryin0123 & 0x01) == 0x01)                        
                        send_msg(cmd, " ��ذ�(slot %d)�ɽӵ�0����ʧ��!\r\n", u8Slot);
                    if((g_peu_dryin0123 & 0x02) == 0x02)                        
                        send_msg(cmd, " ��ذ�(slot %d)�ɽӵ�1����ʧ��!\r\n", u8Slot);
                    if((g_peu_dryin0123 & 0x04) == 0x04)                        
                        send_msg(cmd, " ��ذ�(slot %d)�ɽӵ�2����ʧ��!\r\n", u8Slot);
                    if((g_peu_dryin0123 & 0x08) == 0x08)                        
                        send_msg(cmd, " ��ذ�(slot %d)�ɽӵ�3����ʧ��!\r\n", u8Slot);
                    if((g_peu_dryin4567 & 0x01) == 0x01)                        
                        send_msg(cmd, " ��ذ�(slot %d)�ɽӵ�4����ʧ��!\r\n", u8Slot);
                    if((g_peu_dryin4567 & 0x02) == 0x02)                        
                        send_msg(cmd, " ��ذ�(slot %d)�ɽӵ�5����ʧ��!\r\n", u8Slot);
                    if((g_peu_dryin4567 & 0x04) == 0x04)                        
                        send_msg(cmd, " ��ذ�(slot %d)�ɽӵ�6����ʧ��!\r\n", u8Slot);
                    if((g_peu_dryin4567 & 0x08) == 0x08)                        
                        send_msg(cmd, " ��ذ�(slot %d)�ɽӵ�7����ʧ��!\r\n", u8Slot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ��ذ�(slot %d)�ɽ��״̬��ȡ��ʱ!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ��ذ�(slot %d)�ɽ��״̬��ȡʧ��!!\r\n",u8Slot);
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

    //��ȡ��λ��
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
                send_msg(cmd, " ��ذ�(slot %d)�¶�=%d.\r\n",u8Slot, g_s16PEUTemp);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ��ذ�(slot %d)�¶Ȼ�ȡʧ��!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ��ذ�(slot %d)�¶Ȼ�ȡʧ��!\r\n",u8Slot);
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

    //��ȡ��λ��
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
                send_msg(cmd, " ��ذ�(slot %d) RS485���Գɹ�!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ��ذ�(slot %d) RS485����ʧ��!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ��ذ�(slot %d) RS485����ʧ��!!\r\n",u8Slot);
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

    //��ȡ��λ��
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
                send_msg(cmd, "��ذ�(slot %d)arm version = %s\r\n", u8Slot,boards[u8Slot].arm_version);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, "��ذ�(slot %d)arm version��ȡʧ��!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, "��ذ�(slot %d)arm version��ȡʧ��!\r\n",u8Slot);
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


/*****************��������***************************/
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
    //��ȡ��λ��
    u8BbpSlot = bsp_get_mctbbp_bbp_slot(cmd);
    //��ȡ������
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
                    send_msg(cmd, " ������(slot %d)HMI���Խ��ճɹ�!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ������(slot %d)HMI���Խ���ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ������(slot %d)HMI���Է���ʧ��!\r\n",u8BbpSlot);
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
                    send_msg(cmd, " FSA��(slot %d)HMI���Խ��ճɹ�!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " FSA��(slot %d)HMI���Խ���ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " FSA��(slot %d)HMI���Է���ʧ��!\r\n",u8BbpSlot);
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
                    send_msg(cmd, " ES��(slot %d)HMI���Խ��ճɹ�!\r\n",u8BbpSlot);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ES��(slot %d)HMI���Խ���ʧ��!\r\n",u8BbpSlot);
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ES��(slot %d)HMI���Է���ʧ��!\r\n",u8BbpSlot);
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
                send_msg(cmd, " ������HMI���Խ��ճɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ������HMI���Խ���ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ������HMI���Է���ʧ��!\r\n");
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
                send_msg(cmd, " ���Ȱ�HMI���Խ��ճɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���Ȱ�HMI���Խ���ʧ��!\r\n");
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ���Ȱ�HMI���Է���ʧ��!\r\n");
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
                send_msg(cmd, " ��ذ�(slot %d)HMI���Խ��ճɹ�!\r\n",u8Slot);
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ��ذ�(slot %d)HMI���Խ���ʧ��!\r\n",u8Slot);
                cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
            }
        }
        else
        {
            send_msg(cmd, " ��ذ�HMI(slot %d)���Է���ʧ��!\r\n",u8Slot);
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
    "������",
    "������",
    "������",
    "����",
    "����",
    "���ֳ�ʱ",
    "�쳣"
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
            send_msg(cmd, " AFC״̬��ȡ���Գɹ�!\r\n");
            send_msg(cmd, " AFC״̬:%s \r\n",  afc_status[afclock]);
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " AFC״̬��ȡ����ʧ��!\r\n");
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
                send_msg(cmd, "SLOT(%d)bbp�忨�����ɹ�!\r\n",bid);
                if(boards[bid].fpga_status== DSP_FPGA_STATE_LOADED)
                {
                    send_msg(cmd, "SLOT(%d) bbp�忨fpga���سɹ�!\r\n",bid);
                }
                else
                {
                    send_msg(cmd, "SLOT(%d) bbp�忨fpga���� ʧ��!\r\n",bid);
                }
                if(boards[bid].dsp_isload == DSP_FPGA_STATE_LOADED)
                {
                    send_msg(cmd, "SLOT(%d) bbp�忨dsp���سɹ�!\r\n",bid);
                }
                else
                {
                    send_msg(cmd, "SLOT(%d) bbp�忨dsp���� ʧ��!\r\n",bid);
                }
                if(boards[bid].dsp_isready == 0xf)
                {
                    send_msg(cmd, "SLOT(%d) bbp�忨dsp all ready!\r\n",bid);
                }
                else
                {
                    if((boards[bid].dsp_isready & 0x1) == 0x1)
                    {
                        send_msg(cmd, "SLOT(%d) bbp�忨dsp1 ready!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) bbp�忨dsp1 not ready!\r\n",bid);
                    }
                    if((boards[bid].dsp_isready & 0x2) == 0x2)
                    {
                        send_msg(cmd, "SLOT(%d) bbp�忨dsp2 ready!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) bbp�忨dsp2 not ready!\r\n",bid);
                    }
                    if((boards[bid].dsp_isready & 0x4) == 0x4)
                    {
                        send_msg(cmd, "SLOT(%d) bbp�忨dsp3 ready!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) bbp�忨dsp3 not ready!\r\n",bid);
                    }
                    if((boards[bid].dsp_isready & 0x8) == 0x8)
                    {
                        send_msg(cmd, "SLOT(%d) bbp�忨dsp4 ready!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) bbp�忨dsp4 not ready!\r\n",bid);
                    }
                }
            }
            else
            {
                send_msg(cmd, "SLOT(%d)bbp�忨����ʧ��!\r\n",bid);
            }
        }
        else if(boards[bid].type == BOARD_TYPE_FSA)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)fsa�忨�����ɹ�!\r\n",bid);
                if(boards[bid].fpga_status == 0x30)
                {
                    send_msg(cmd, "SLOT(%d) fsa�忨fpga���سɹ�!\r\n",bid);
                }
                else
                {
                    if((boards[bid].fpga_status&FSA_FPGA_325T_LOADED) != FSA_FPGA_325T_LOADED)
                    {
                        send_msg(cmd, "SLOT(%d) fsa�忨fpga_325t����ʧ��!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) fsa�忨fpga_325t���سɹ�!\r\n",bid);
                    }
                    if((boards[bid].fpga_status&FSA_FPGA_160T_LOADED) != FSA_FPGA_160T_LOADED)
                    {
                        send_msg(cmd, "SLOT(%d) fsa�忨fpga_160t����ʧ��!\r\n",bid);
                    }
                    else
                    {
                        send_msg(cmd, "SLOT(%d) fsa�忨fpga_160t���سɹ�!\r\n",bid);
                    }
                }
            }
            else
            {
                send_msg(cmd, "SLOT(%d)fsa�忨����ʧ��!\r\n",bid);
            }
        }
        else if(boards[bid].type == BOARD_TYPE_ES)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)es�忨�����ɹ�!\r\n",bid);
                if(boards[bid].fpga_status == DSP_FPGA_STATE_LOADED)
                {
                    send_msg(cmd, "SLOT(%d) es�忨fpga���سɹ�!\r\n",bid);
                }
                else
                {
                    send_msg(cmd, "SLOT(%d) es�忨fpga���� ʧ��!\r\n",bid);
                }
            }
            else
            {
                send_msg(cmd, "SLOT(%d)fsa�忨����ʧ��!\r\n",bid);
            }
        }
        else if(boards[bid].type == BOARD_TYPE_FAN)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)fan�忨�����ɹ�!\r\n",bid); 
            }
            else
            {
                send_msg(cmd, "SLOT(%d)fan�忨����ʧ��!\r\n",bid);
            }
        }
        else if(boards[bid].type == BOARD_TYPE_PEU)
        {
            if((boards[bid].mcu_status&0xF0) == MCU_STATUS_RECV_FIRSTMSG)
            {
                send_msg(cmd, "SLOT(%d)peu�忨�����ɹ�!\r\n",bid);
            }
            else
            {
                send_msg(cmd, "SLOT(%d)peu�忨����ʧ��!\r\n",bid);
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

        send_msg(cmd, "�忨������ѯ�ɹ�!\r\n");
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
        //		send_msg(cmd, " ͬ������ʧ�ܣ�AFC״̬0x%x!\r\n", afc_lock_status_get());
        //       cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        //	}else
        bsp_bbp_fpga_write(2,192, 0x1);
        long_frame_number = (long_frame_number+0x1111111) & 0xFFFFFFF;
        bsp_write_long_frame(long_frame_number);
        usleep(frame_read_delay);
        read_frame = read_long_frame_from_fpga();
        if(long_frame_number==read_frame)
        {
            send_msg(cmd, " ͬ�����Գɹ���\r\n");
            cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
        }
        else
        {
            send_msg(cmd, " ͬ������ʧ�ܣ�0x%x<->0x%x\r\n", long_frame_number,read_frame);
            cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
        }
        cmd.pkg_datalen = htonl(0);
        send_result(cmd);
    }
    bsp_mutex_unlock(cmd_list[cmd.index].plock);
    return cmd;
}

/*****************EEPROM����/��ȡ������***************************/
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
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM CRC���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM CRC����ʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_crc(u8Slot, crc)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM CRC���óɹ�!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM CRC���óɹ�!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM CRC���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM CRC����ʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM CRC����ʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM CRC����ʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM CRC���óɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM CRC����ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM CRC����ʧ��!\r\n");
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

    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM DeviceID���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM DeviceID����ʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_deviceid(u8Slot, deviceid)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM DeviceID���óɹ�!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM DeviceID���óɹ�!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM DeviceID���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {                
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM DeviceID����ʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM DeviceID����ʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM DeviceID����ʧ��!\r\n");                
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
                    send_msg(cmd, " ���Ȱ�EEPROM DeviceID���óɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM DeviceID����ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM DeviceID����ʧ��!\r\n");
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
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM BoardType���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM BoardType����ʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_boardtype(u8Slot, boardtype)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)                    
                    send_msg(cmd, " ������EEPROM BoardType���óɹ�!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)                    
                    send_msg(cmd, " ������EEPROM BoardType���óɹ�!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)                    
                    send_msg(cmd, " ��ǿ��EEPROM BoardType���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType) 
                    send_msg(cmd, " ������EEPROM BoardType����ʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType) 
                    send_msg(cmd, " ������EEPROM BoardType����ʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType) 
                    send_msg(cmd, " ��ǿ��EEPROM BoardType����ʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM BoardType���óɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM BoardType����ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM BoardType����ʧ��!\r\n");
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

    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM MACADDR1���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM MACADDR1����ʧ��!\r\n");
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
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM MACADDR2���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM MACADDR2����ʧ��!\r\n");
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

        send_msg(cmd, " ���ذ�ETH3 IP���óɹ�!\r\n");
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
    
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM ProductSN���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM ProductSN����ʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_productsn(u8Slot, productsn)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductSN���óɹ�!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductSN���óɹ�!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM ProductSN���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductSN����ʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductSN����ʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM ProductSN����ʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM ProductSN���óɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM ProductSN����ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM ProductSN����ʧ��!\r\n");
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

    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM Manufacture���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM Manufacture����ʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_manufacturer(u8Slot, manufacturer)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM Manufacture���óɹ�!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM Manufacture���óɹ�!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM Manufacture���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM Manufacture����ʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM Manufacture����ʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM Manufacture����ʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM Manufacture���óɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM Manufacture����ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM Manufacture����ʧ��!\r\n");
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

    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM ProductDate���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM ProductDate����ʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_productdate(u8Slot, productdate)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductDate���óɹ�!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductDate���óɹ�!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM ProductDate���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductDate����ʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductDate����ʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM ProductDate����ʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM ProductDate���óɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM ProductDate����ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM ProductDate����ʧ��!\r\n");
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

    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM SatelliteReceiver���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM SatelliteReceiver����ʧ��!\r\n");
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

    //��ȡ��λ��
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
                    send_msg(cmd, " ���Ȱ�EEPROM FanInitialSpeed���óɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM FanInitialSpeed����ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM FanInitialSpeed����ʧ��!\r\n");
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

    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM TemperatureThreshold���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM TemperatureThreshold����ʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_set_tempthreshold(u8Slot, tempthreshold)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM TemperatureThreshold���óɹ�!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM TemperatureThreshold���óɹ�!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM TemperatureThreshold���óɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM TemperatureThreshold����ʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM TemperatureThreshold����ʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM TemperatureThreshold����ʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM TemperatureThreshold���óɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM TemperatureThreshold����ʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM TemperatureThreshold����ʧ��!\r\n");
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
    
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM CRC��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                send_msg(cmd, "���ذ�CRC=0x%x\r\n",g_mca_eeprom_par.checkSum);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM CRC��ȡʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_crc(u8Slot)==BSP_OK)
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM CRC��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������CRC=0x%x\r\n",g_bbp_eeprom_par.checkSum);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM CRC��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������CRC=0x%x\r\n",g_bbp_eeprom_par.checkSum);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " ��ǿ��EEPROM CRC��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ��ǿ��CRC=0x%x\r\n",g_bbp_eeprom_par.checkSum);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM CRC��ȡʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM CRC��ȡʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM CRC��ȡʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM CRC��ȡ�ɹ�!\r\n");                    
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    send_msg(cmd, "���Ȱ�CRC=0x%x\r\n",g_fan_eeprom_get_par.checkSum);
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM CRC��ȡʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM CRC��ȡʧ��!\r\n");
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
    
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM DeviceID��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.device_id, 16))
                {
                    memcpy(g_mca_eeprom_par.device_id, null, 5);
                }
                send_msg(cmd, "���ذ�DeviceID=%s\r\n",g_mca_eeprom_par.device_id);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM DeviceID��ȡʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_deviceid(u8Slot)==BSP_OK)
            {
                if(BSP_OK == stringisnull(g_bbp_eeprom_par.device_id, 16))
                {
                    memcpy(g_bbp_eeprom_par.device_id, null, 5);
                }
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM DeviceID��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������DeviceID=%s\r\n",g_bbp_eeprom_par.device_id);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM DeviceID��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������DeviceID=%s\r\n",g_bbp_eeprom_par.device_id);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " ��ǿ��EEPROM DeviceID��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ��ǿ��DeviceID=%s\r\n",g_bbp_eeprom_par.device_id);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM DeviceID��ȡʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM DeviceID��ȡʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM DeviceID��ȡʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM DeviceID��ȡ�ɹ�!\r\n");
                    if(BSP_OK == stringisnull(g_fan_eeprom_get_par.device_id, 16))
                    {
                        memcpy(g_fan_eeprom_get_par.device_id, null, 5);
                    }
                    send_msg(cmd, "���Ȱ�DeviceID=%s\r\n",g_fan_eeprom_get_par.device_id);
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM DeviceID��ȡʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM DeviceID��ȡʧ��!\r\n");
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
    
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM BoardType��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.board_type, 32))
                    memcpy(g_mca_eeprom_par.board_type, null, 5);
                send_msg(cmd, "���ذ�BoardType=%s\r\n",g_mca_eeprom_par.board_type);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM BoardType��ȡʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_boardtype(u8Slot)==BSP_OK)
            {
                if(BSP_OK == stringisnull(g_bbp_eeprom_par.board_type, 32))
                    memcpy(g_bbp_eeprom_par.board_type, null, 5);
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM BoardType��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������BoardType=%s\r\n",g_bbp_eeprom_par.board_type);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM BoardType��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������BoardType=%s\r\n",g_bbp_eeprom_par.board_type);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " ��ǿ��EEPROM BoardType��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ��ǿ��BoardType=%s\r\n",g_bbp_eeprom_par.board_type);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM BoardType��ȡʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM BoardType��ȡʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM BoardType��ȡʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM BoardType��ȡ�ɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    if(BSP_OK == stringisnull(g_fan_eeprom_get_par.board_type, 32))
                        memcpy(g_fan_eeprom_get_par.board_type, null, 5);
                    send_msg(cmd, "���Ȱ�BoardType=%s\r\n",g_fan_eeprom_get_par.board_type);
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM BoardType��ȡʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM BoardType��ȡʧ��!\r\n");
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
     //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM MACADDR1��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                ucharxtostr(g_mca_eeprom_par.mac_addr1, str, 6);
                send_msg(cmd,"���ذ�MACADDR1=%s\r\n", str);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM MACADDR1��ȡʧ��!\r\n");
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
     //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM MACADDR2��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                ucharxtostr(g_mca_eeprom_par.mac_addr2, str, 6);
                send_msg(cmd,"���ذ�MACADDR2=%s\r\n", str);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM MACADDR2��ȡʧ��!\r\n");
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
    
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM ProductSN��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.product_sn, 32))
                    memcpy(g_mca_eeprom_par.product_sn, null, 5);
                send_msg(cmd,"���ذ�ProductSN=%s\r\n",g_mca_eeprom_par.product_sn);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM ProductSN��ȡʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_productsn(u8Slot)==BSP_OK)
            {
                if(BSP_OK == stringisnull(g_bbp_eeprom_par.product_sn, 32))
                    memcpy(g_bbp_eeprom_par.product_sn, null, 5);
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM ProductSN��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������ProductSN=%s\r\n",g_bbp_eeprom_par.product_sn);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM ProductSN��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������ProductSN=%s\r\n",g_bbp_eeprom_par.product_sn);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " ��ǿ��EEPROM ProductSN��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ��ǿ��ProductSN=%s\r\n",g_bbp_eeprom_par.product_sn);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;              
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductSN��ȡʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductSN��ȡʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM ProductSN��ȡʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM ProductSN��ȡ�ɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;                    
                    if(BSP_OK == stringisnull(g_fan_eeprom_get_par.product_sn, 32))
                        memcpy(g_fan_eeprom_get_par.product_sn, null, 5);
                    send_msg(cmd,"���Ȱ�ProductSN=%s\r\n",g_fan_eeprom_get_par.product_sn);
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM ProductSN��ȡʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM ProductSN��ȡʧ��!\r\n");
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
    
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM Manufacture��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.manufacturer, 12))
                    memcpy(g_mca_eeprom_par.manufacturer, null, 5);
                send_msg(cmd, "���ذ�Manufacture=%s\r\n",g_mca_eeprom_par.manufacturer);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM Manufacture��ȡʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_manufacturer(u8Slot)==BSP_OK)
            {
                if(BSP_OK == stringisnull(g_bbp_eeprom_par.manufacturer, 12))
                    memcpy(g_bbp_eeprom_par.manufacturer, null, 5);
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM Manufacture��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������Manufacture=%s\r\n",g_bbp_eeprom_par.manufacturer);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM Manufacture��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������Manufacture=%s\r\n",g_bbp_eeprom_par.manufacturer);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " ��ǿ��EEPROM Manufacture��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ��ǿ��Manufacture=%s\r\n",g_bbp_eeprom_par.manufacturer);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM Manufacture��ȡʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM Manufacture��ȡʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM Manufacture��ȡʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM Manufacture��ȡ�ɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    if(BSP_OK == stringisnull(g_fan_eeprom_get_par.manufacturer, 12))
                        memcpy(g_fan_eeprom_get_par.manufacturer, null, 5);
                    send_msg(cmd, "���Ȱ�Manufacture=%s\r\n",g_fan_eeprom_get_par.manufacturer);
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM Manufacture��ȡʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM Manufacture��ȡʧ��!\r\n");
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
    
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM ProductDate��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                ucharxtostr(g_mca_eeprom_par.product_date, str, 4);
                send_msg(cmd,"���ذ�ProductDate=%s\r\n", str);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM ProductDate��ȡʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_productdate(u8Slot)==BSP_OK)
            {
                ucharxtostr(g_bbp_eeprom_par.product_date, str, 4);
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM ProductDate��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������ProductDate=%s\r\n", str);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM ProductDate��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������ProductDate=%s\r\n", str);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " ��ǿ��EEPROM ProductDate��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ��ǿ��ProductDate=%s\r\n", str);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;                                
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductDate��ȡʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM ProductDate��ȡʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM ProductDate��ȡʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM ProductDate��ȡ�ɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;                   
                    ucharxtostr(g_fan_eeprom_get_par.product_date, str, 4);
                    send_msg(cmd,"���Ȱ�ProductDate=%s\r\n", str);
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM ProductDate��ȡʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM ProductDate��ȡʧ��!\r\n");
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

     //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM SatelliteReceiver��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                if(BSP_OK == stringisnull(g_mca_eeprom_par.satellite_receiver, 12))
                    memcpy(g_mca_eeprom_par.satellite_receiver, null, 5);
                send_msg(cmd, "���ذ�SatelliteReceiver=%s\r\n",g_mca_eeprom_par.satellite_receiver);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM SatelliteReceiver��ȡʧ��!\r\n");
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
    //��ȡ��λ��
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
                    send_msg(cmd, " ���Ȱ�EEPROM FanInitialSpeed��ȡ�ɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    send_msg(cmd, "���Ȱ�Fan_InitialSpeed0=0x%x, Fan_InitialSpeed1=0x%x, Fan_InitialSpeed2=0x%x\r\n",
                        g_fan_eeprom_get_par.fan_initialspeed[0],g_fan_eeprom_get_par.fan_initialspeed[1], g_fan_eeprom_get_par.fan_initialspeed[2]);
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM FanInitialSpeed��ȡʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM FanInitialSpeed��ȡʧ��!\r\n");
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
    
    //��ȡ��λ��
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
                send_msg(cmd, " ���ذ�EEPROM TemperatureThreshold��ȡ�ɹ�!\r\n");
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                send_msg(cmd,"���ذ�TemperatureThreshold=%d %d\r\n",g_mca_eeprom_par.temperature_threshold[0],
                         g_mca_eeprom_par.temperature_threshold[1]);
            }
            else
            {
                send_msg(cmd, " ���ذ�EEPROM TemperatureThreshold��ȡʧ��!\r\n");
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
            //��ȡ������
            u8BoardType = boards[u8Slot].type;
            if(board_eeprom_get_tempthreshold(u8Slot)==BSP_OK)
            {                
                if(BOARD_TYPE_BBP == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM TemperatureThreshold��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������TemperatureThreshold=%d %d\r\n",g_bbp_eeprom_par.temperature_threshold[0],
                         g_bbp_eeprom_par.temperature_threshold[1]);
                }
                if(BOARD_TYPE_FSA == u8BoardType)
                {
                    send_msg(cmd, " ������EEPROM TemperatureThreshold��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ������TemperatureThreshold=%d %d\r\n",g_bbp_eeprom_par.temperature_threshold[0],
                         g_bbp_eeprom_par.temperature_threshold[1]);
                }
                if(BOARD_TYPE_ES == u8BoardType)
                {
                    send_msg(cmd, " ��ǿ��EEPROM TemperatureThreshold��ȡ�ɹ�!\r\n");
                    send_msg(cmd, " ��ǿ��TemperatureThreshold=%d %d\r\n",g_bbp_eeprom_par.temperature_threshold[0],
                         g_bbp_eeprom_par.temperature_threshold[1]);
                }
                cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
            }
            else
            {
                if(BOARD_TYPE_BBP == u8BoardType)
                    send_msg(cmd, " ������EEPROM TemperatureThreshold��ȡʧ��!\r\n");
                if(BOARD_TYPE_FSA == u8BoardType)
                    send_msg(cmd, " ������EEPROM TemperatureThreshold��ȡʧ��!\r\n");
                if(BOARD_TYPE_ES == u8BoardType)
                    send_msg(cmd, " ��ǿ��EEPROM TemperatureThreshold��ȡʧ��!\r\n");
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
                    send_msg(cmd, " ���Ȱ�EEPROM TemperatureThreshold��ȡ�ɹ�!\r\n");
                    cmd.pkg_successtimes = htonl(cmd.pkg_successtimes)+1;
                    send_msg(cmd,"���Ȱ�TemperatureThreshold=%d %d\r\n",g_fan_eeprom_get_par.temperature_threshold[0],
                             g_fan_eeprom_get_par.temperature_threshold[1]);
                }
                else
                {
                    send_msg(cmd, " ���Ȱ�EEPROM TemperatureThreshold��ȡʧ��!\r\n");
                    cmd.pkg_failtimes = htonl(cmd.pkg_failtimes)+1;
                }
            }
            else
            {
                send_msg(cmd, " ���Ȱ�EEPROM TemperatureThreshold��ȡʧ��!\r\n");
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
    /*****************************************���ذ������***************************************/
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_TEMPERATURE, "�����¶�", test_mct_temprature, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_PPCDDR, "����DDR", test_ppc_ddr, &ppcddr_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_NORFLASH, "����NorFlash", test_ppc_norflash, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_NANDFLASH, "����NandFlash", test_nandflash, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_CPLD_READ, "����CPLD�汾", test_cpld_version, &mtd_lock},
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_CPLD_LOAD, "����CPLD����", test_cpld_load, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_GESSWITCH, "����GESwitch", test_ppc_geswitch, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_GPS, "GPS", test_ppc_gps, &mtd_lock},
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_USB, "����USB", test_ppc_usb, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_SFP, "���ع�ģ��", test_ppc_sfp, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_POWER, "���ع���", test_ppc_power, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_EEPROM, "����eeprom", test_ppc_eeprom, &mtd_lock},
    {MASK_TESTALL, BOARD_TYPE_MCT, 0, TEST_MCT_PHY, "����phy", test_ppc_phy, &mtd_lock},
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_VER, "���ذ汾", test_ppc_ver, &mtd_lock},    
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_EXTSYNC_PP1S, "������ͬ��pp1s�ӿ�", test_ppc_extsync_pp1s, &mtd_lock},    
    {0, BOARD_TYPE_MCT, 0, TEST_MCT_EXTSYNC_TOD, "������ͬ��tod�ӿ�", test_ppc_extsync_tod, &mtd_lock},
    /*****************************************�����������***************************************/
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_BOOT, "�����帴λ", test_bbp_reset, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_FPGA_LOAD, "������fpga����", test_bbp_fpgaload, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_DSP_LOAD, "������dsp����", test_bbp_dsp_load, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_MCU_UPDATE, "������mcu����", test_bbp_mcuupdate, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_CPLD_UPDATE, "������cpld����", test_bbp_cpldupdate, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_ETHSW, "������GESwitch", test_bbp_ethsw, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_SDRAM, "������SDRAM", test_bbp_sdram, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_TEMPERATURE, "�������¶�", test_bbp_temprature, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_POWER, "�����幦��", test_bbp_power_info, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_EEPROM, "������eeprom", test_bbp_eeprom, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_SRIOSWITCH, "������sriosw", test_bbp_srioswt, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_CPLD, "������cpld�汾", test_bbp_cpld_version, &bbp_lock},
    {0, BOARD_TYPE_BBP, 0, TEST_BBP_VER, "������汾", test_bbp_ver, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_SFP, "�������ģ��", test_bbp_sfp, &bbp_lock},    
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_IR_SYNC, "��������ͬ��", test_bbp_ir_sync, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_DSP_CPU, "CPU-DSP ��·", test_bbp_dsp_cpu, NULL},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_DSP_DDR, "DSP-DDR ����", test_bbp_dsp_ddr, NULL},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_SRIO_DATA, "DSP-DSP SRIO ����", test_bbp_srio_data, NULL},
    {MASK_TESTALL, BOARD_TYPE_BBP, 0, TEST_BBP_AIF, "DSP AIF", test_bbp_dsp_aif, NULL},
    /*****************************************�����������***************************************/
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_BOOT, "�����帴λ", test_bbp_reset, &bbp_lock},
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_FPGA_160T, "������fpga_160t����", test_fsa_fpga160t_fpgaload, &bbp_lock},
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_FPGA_325T, "������fpga_325t����", test_bbp_fpgaload, &bbp_lock},
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_MCU_UPDATE, "������mcu����", test_bbp_mcuupdate, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_ETHSW, "������GESwitch", test_bbp_ethsw, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_SDRAM, "������SDRAM", test_bbp_sdram, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_TEMPERATURE, "�������¶�", test_bbp_temprature, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_POWER, "�����幦��", test_bbp_power_info, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_EEPROM, "������eeprom", test_bbp_eeprom, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_SRIOSWITCH, "������sriosw", test_bbp_srioswt, &bbp_lock},
    {0, BOARD_TYPE_FSA, 0, TEST_FSA_VER, "������汾", test_bbp_ver, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_SFP, "�������ģ��", test_bbp_sfp, &bbp_lock},    
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_IR_SYNC, "��������ͬ��", test_bbp_ir_sync, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_PHY, "������PHY", test_fsa_phy, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_PLL_CFG, "������PLL����", test_fsa_pll_cfg, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_FPGA_325T_DDR, "������FPGA_325T DDR ����", test_fsa_fpga_325t_ddr, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_FSA, 0, TEST_FSA_FPGA_160T_SRIO, "������FPGA_160T SRIO����", test_fsa_fpga_160t_srio, &bbp_lock},
    /*****************************************ͬ���������***************************************/
    {0, BOARD_TYPE_ES, 0, TEST_ES_BOOT, "ͬ���帴λ", test_bbp_reset, &bbp_lock},
    {0, BOARD_TYPE_ES, 0, TEST_ES_FPGA, "ͬ����fpga����", test_bbp_fpgaload, &bbp_lock},
    {0, BOARD_TYPE_ES, 0, TEST_ES_MCU_UPDATE, "ͬ����mcu����", test_bbp_mcuupdate, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_ETHSW, "ͬ����GESwitch", test_bbp_ethsw, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_SDRAM, "ͬ����SDRAM", test_bbp_sdram, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_TEMPERATURE, "ͬ�����¶�", test_bbp_temprature, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_EEPROM, "ͬ����eeprom", test_bbp_eeprom, &bbp_lock},
    {0, BOARD_TYPE_ES, 0, TEST_ES_VER, "ͬ����汾", test_bbp_ver, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_SFP, "ͬ�����ģ��", test_bbp_sfp, &bbp_lock},    
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_PLL_CFG, "ͬ����PLL����", test_fsa_pll_cfg, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_COPPER_LINK, "ͬ��������·", test_es_copper_link, &bbp_lock},
    {MASK_TESTALL, BOARD_TYPE_ES, 0, TEST_ES_FIBBER_LINK, "ͬ��������·", test_es_fibber_link, &bbp_lock},
    /*****************************************���Ȱ������***************************************/
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_BOOT, "���Ȱ帴λ", test_fan_reset, &fan_lock},
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_SPEED_SET, "����ת������", test_fan_speed_set, &fan_lock},
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_SPEED_GET, "����ת�ٻ�ȡ", test_fan_speed_get, &fan_lock},
    {MASK_TESTALL, BOARD_TYPE_FAN, 0, TEST_FAN_EEPROM, "���Ȱ�eeprom", test_fan_eeprom, &fan_lock},
    {MASK_TESTALL, BOARD_TYPE_FAN, 0, TEST_FAN_TEST, "���Ȳ���", test_fan_test, &fan_lock},
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_VERSION, "���Ȱ�汾", test_fan_version, &fan_lock},
    {0, BOARD_TYPE_FAN, 0, TEST_FAN_UPDATE, "���Ȱ�mcu����", test_fan_mcuupdate, &fan_lock},
    /*****************************************��ذ������***************************************/
    {0, BOARD_TYPE_PEU, 0, TEST_PEU_BOOT, "��ذ帴λ", test_peu_reset, &peu_lock},
    {0, BOARD_TYPE_PEU, 0, TEST_PEU_POWER_DOWN, "��ذ����", test_peu_power_down, &peu_lock},
    {MASK_TESTALL, BOARD_TYPE_PEU, 0, TEST_PEU_DRYIN, "��ذ�ɽӵ�״̬", test_peu_dryin, &peu_lock},
    {MASK_TESTALL, BOARD_TYPE_PEU, 0, TEST_PEU_TEMPERATURE, "��ذ��¶�", test_peu_temperature, &peu_lock},
    {0, BOARD_TYPE_PEU, 0, TEST_PEU_VERSION, "��ذ�汾", test_peu_version, &peu_lock},
    {0, BOARD_TYPE_PEU, 0, TEST_PEU_UPDATE, "��ذ�mcu����", test_peu_mcuupdate, &peu_lock},    
    {MASK_TESTALL, BOARD_TYPE_PEU, 0, TEST_PEU_RS485, "��ذ�RS485����", test_peu_rs485, &peu_lock},
    /******************************************��������***************************************/
    {0, 0, 0, TEST_MCT_BBP_HMI, "HMI(����<->����)", test_mct_bbp_hmi, &hmi_lock},
    {0, 0, 0, TEST_MCT_PEU_HMI, "HMI(����<->���)", test_mct_peu_hmi, &hmi_lock},
    {0, 0, 0, TEST_MCT_FAN_HMI, "HMI(����<->����)", test_mct_fan_hmi, &hmi_lock},
    {0, 0, 0, TEST_AFC, "AFC����", test_afc, &hmi_lock},
    {0, 0, 0, TEST_BOARD_READY, "��վ����", test_board_ready, &hmi_lock},
    {0, 0, 0, TEST_AFC_SYNC, "֡��ͬ��", test_sync, &hmi_lock},
    //{0, TEST_REBOOT_BOARDS, "test_reboot_boards", test_reboot_boards, &bbp_lock},
    
    /******************************************RRU������***************************************/
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_VERQ, "RRU�汾��ѯ", rrutest_version_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_VERD, "RRU�汾����", rrutest_verdownload_req, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_VERA, "RRU�汾����", rrutest_veractivate_req, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_ANT, "RRU������������", rrutest_antenna_config, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_RF, "RRU��Ƶ״̬��ѯ", rrutest_rfstatus_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_RUN, "RRU����״̬��ѯ", rrutest_rrustatus_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_SFP, "RRU���״̬��ѯ", rrutest_fiberstatus_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_TIME, "RRUϵͳʱ������", rrutest_systime_config, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_ALARM, "RRU�澯��ѯ", rrutest_almquery_req, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_PARA, "RRU������ѯ", rrutest_para_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_CAL, "RRU����У׼", rrutest_power_cal, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_CELL, "RRUС������", rrutest_cell_config, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_DELAY, "RRU����ʱ����", rrutest_fiberdelay_config, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_REBOOT, "RRU��λ", rrutest_reset, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_SERIAL, "RRUԶ�̴��ڹر�", rrutest_serialclose_req, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_RRU_HARDWARE, "RRUӲ��������ѯ", rrutest_hwparam_query, &rru_lock},
    {0, BOARD_TYPE_RRU, 0, TEST_HEARTBEAT_PC, "RRU����������ȡ", rrutest_heartbeat_to_pc, &rru_lock},
	
    /***************************************EEPROM��������/��ȡ������*************************/
    {0, 0, 0, TEST_SET_CRC, "����EEPROM CRC", test_bbu_set_crc, &eeprom_lock},
    {0, 0, 0, TEST_SET_DEVICE_ID, "����DeviceID", test_bbu_set_device_id, &eeprom_lock},
    {0, 0, 0, TEST_SET_BOARD_TYPE, "���ð忨����", test_bbu_set_board_type, &eeprom_lock},
    {0, 0, 0, TEST_SET_MAC_ADDR1, "����MAC1", test_bbu_set_mac_addr1, &eeprom_lock},
    {0, 0, 0, TEST_SET_MAC_ADDR2, "����MAC2", test_bbu_set_mac_addr2, &eeprom_lock},
    {0, 0, 0, TEST_SET_ETH3_ADDR, "����eth3 IP", test_bbu_set_eth3_addr, NULL},
    {0, 0, 0, TEST_SET_PRODUCT_SN, "���ò�Ʒ���", test_bbu_set_product_sn, &eeprom_lock},
    {0, 0, 0, TEST_SET_MANUFACTURE, "�����������", test_bbu_set_manufacturer, &eeprom_lock},
    {0, 0, 0, TEST_SET_PRODUCT_DATE, "������������", test_bbu_set_product_date, &eeprom_lock},
    {0, 0, 0, TEST_SET_SATELLITE_RECEIVER, "����GPSģ������", test_bbu_set_satellite_receiver, &eeprom_lock},
    {0, 0, 0, TEST_SET_FAN_INIT_SPEED, "���ó�ʼ��ת��", test_bbu_set_fan_init_speed, &eeprom_lock},
    {0, 0, 0, TEST_SET_TEMP_THRESHOLD, "�����¶���ֵ", test_bbu_set_temp_threshold, &eeprom_lock},

    {0, 0, 0, TEST_GET_CRC, "��ȡEEPROM CRC", test_bbu_get_crc, &eeprom_lock},
    {0, 0, 0, TEST_GET_DEVICE_ID, "��ȡDeviceID", test_bbu_get_device_id, &eeprom_lock},
    {0, 0, 0, TEST_GET_BOARD_TYPE, "��ȡ�忨����", test_bbu_get_board_type, &eeprom_lock},
    {0, 0, 0, TEST_GET_MAC_ADDR1, "��ȡMAC1", test_bbu_get_mac_addr1, &eeprom_lock},
    {0, 0, 0, TEST_GET_MAC_ADDR2, "��ȡMAC2", test_bbu_get_mac_addr2, &eeprom_lock},
    {0, 0, 0, TEST_GET_PRODUCT_SN, "��ȡ��Ʒ���", test_bbu_get_product_sn, &eeprom_lock},
    {0, 0, 0, TEST_GET_MANUFACTURE, "��ȡ�������", test_bbu_get_manufacturer, &eeprom_lock},
    {0, 0, 0, TEST_GET_PRODUCT_DATE, "��ȡ��������", test_bbu_get_product_date, &eeprom_lock},
    {0, 0, 0, TEST_GET_SATELLITE_RECEIVER, "��ȡGPSģ������", test_bbu_get_satellite_receiver, &eeprom_lock},
    {0, 0, 0, TEST_GET_FAN_INIT_SPEED, "���ó�ʼ��ת��", test_bbu_get_fan_init_speed, &eeprom_lock},
    {0, 0, 0, TEST_GET_TEMP_THRESHOLD, "��ȡ�¶���ֵ", test_bbu_get_temp_threshold, &eeprom_lock},

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
    u8 slot = cmd.pkg_data[0]; /*��ȡ��λ��*/
    uint32_t *p = cmd.pkg_data + htonl(cmd.pkg_datalen);
    *p = htonl(BOARD_TEST_END);
    
    //cmd_list[cmd.index].error_times += cmd.pkg_failtimes;
    if(cmd_list[cmd.index].mask !=0)
    {
        /*�жϲ�λ��*/
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
        send_msg(cmdall, "ֹͣ%s������...\r\n", (cmdall.pkg_cmd==CMD_TESTDANBAN)?"����":"�ϻ�");
    }
    else
    {
        send_msg(cmdall, "%s������ֹͣ!\r\n", (cmdall.pkg_cmd==CMD_TESTDANBAN)?"����":"�ϻ�");
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
    printf("******%s����ʱ��: %dСʱ%d����(%ds)....\r\n", (cmdall.pkg_cmd==CMD_TESTDANBAN)?"����":"�ϻ�",howlong/3600, (howlong%3600)/60, howlong);
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
        send_msg(cmd, "*******************��%d/%d��%s����********\r\n", count, test_all_times+count-2,(cmd.pkg_cmd==CMD_TESTALL)?"�ϻ�":"����");
        for(slot=0; slot<MAX_BOARDS_NUMBER; slot++)
        {
            if((test_slot_mask & (1<<slot)) != 0)
            { 
                send_msg(cmd, "------------------SLOT(%d)����----------------\r\n", slot);
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
    send_msg(cmd, "****************%s����%d��******************\r\n",(cmd.pkg_cmd==CMD_TESTDANBAN)?"����":"�ϻ�",count);
    for(slot =0; slot<MAX_BOARDS_NUMBER; slot++)
    {
        if((test_slot_mask & (1<<slot)) !=0)
        {
            for(i=0; i<CMD_LIST_SIZE; i++)
            {
                //if(cmd_list[i].error_times>0)
                if(test_error_times[i][slot] > 0)
                {
                    send_msg(cmd, "*������Ϣslot[%d]: %20s : %2d *\r\n",slot, cmd_list[i].cmdname, test_error_times[i][slot]);
                }
            }
        }
    }
    send_msg(cmd, "****************%s�������*****************\r\n",(cmd.pkg_cmd==CMD_TESTDANBAN)?"����":"�ϻ�");
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
        send_msg(cmdall, "%s���Խ�����,�벻Ҫ�ظ�����%s����...\r\n",(cmdall.pkg_cmd==CMD_TESTDANBAN)?"����":"�ϻ�",(cmdall.pkg_cmd==CMD_TESTDANBAN)?"����":"�ϻ�");
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
        send_msg(cmdall, "������Խ�����,�벻Ҫ�ظ���������...\r\n");
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


