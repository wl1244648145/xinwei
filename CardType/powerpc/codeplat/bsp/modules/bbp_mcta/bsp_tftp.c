#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <arpa/inet.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "bsp_tftps.h"
#include "bsp_types.h"

#define MCT_SLAVE 0


#define BSP_OK		 0
#define BSP_ERROR 	-1

//单包超时时间，单位（秒）
#define TIMEOUT_VAL_MS         500*1000

static int tftp_dbg_flag = 0;

typedef struct cmdMsg
{
    unsigned int head;  //报文头0xFFBBFFBB
    unsigned char cmd;	//命令字
} CMDMSG;

typedef struct Trans_File
{
    int sockfd;
    struct sockaddr_in addr;    //发送目标地址
    char filename[50];         //文件名
    unsigned int filesize;
    unsigned int block_size;
} TransFile;

static char pathname[100] = "./";
static char tftp_filename[128] = "";
static int tftp_get_file_ok = 0;

#define CMD_ID_CMD_GET	0x00
#define CMD_ID_CMDACK	0x01
#define CMD_ID_DATA		0x02
#define CMD_ID_ACK		0x03
#define CMD_ID_CMD_PUT	0x04

pthread_mutex_t local_lock = PTHREAD_MUTEX_INITIALIZER;

int save_file(char *filename, unsigned char *buf, int len)
{
    FILE *fp = NULL;
    if(filename==NULL)
    {
        printf("[%s]:filename==NULL! ERROR!\r\n");
        return -1;
    }
    fp = fopen(filename, "w");
    if(fp==NULL)
    {
        printf("[%s]:*%s*\r\n",__func__, filename);
        perror("save file");
        return BSP_ERROR;
    }
    fwrite(buf, len, 1, fp);
    fclose(fp);
}

/***********************************************************************
*函数名称：unsigned int calCRC(unsigned char *buf, unsigned int len)
*函数功能：计算32位累加和
*函数参数：  参数名                    描述
*        unsigned char *buf     存放要计算的数据地址
*        unsigned int len        数据长度
*函数返回：32位累加和
***********************************************************************/
unsigned int calCRC(unsigned char *buf, unsigned int len)
{
    int i = 0;
    unsigned int crc = 0;
    for(i=0; i<len; i++)
    {
        crc += buf[i];
    }
    return crc;
}

int tftps_send(int fd, char *buf, int len, struct sockaddr *addr, int addrlen)
{
    if(buf==NULL || len < 0)
    {
        return BSP_ERROR;
    }
#if TFTPS_SOCKET_TYPE == SOCKET_TYPE_TCP
    return send(fd, buf, len, 0);
#else
    return sendto(fd, buf, len, 0, addr, addrlen);
#endif
}
/***********************************************************************
*函数名称：void *get_file_thread(void *arg)
*函数功能：获取文件任务（ARM发送给PPC）
*函数参数：  参数名                                  描述
*        TransFile fileinfo = *(TransFile*)arg;  arg指向保存文件信息的结构地址
*函数返回：0成功  1失败
***********************************************************************/
void *get_file_thread(void *arg)
{
    TransFile fileinfo = *(TransFile*)arg;
    unsigned char temp_buf[1500] = "";
    unsigned int recv_size = 0;
    unsigned short pkg_id = 1;
    struct timeval timeout = {1, 0};
    fd_set readset;
    FD_ZERO(&readset);
    int sockfd = 0;
    unsigned int error_count = 0;
    unsigned char *file_data_buf = NULL;

    file_data_buf = malloc(fileinfo.filesize);
    if(file_data_buf == NULL)
    {
        printf("[%s]: malloc file error!\r\n", __func__);
        return NULL;
    }
    if(tftp_dbg_flag)
    {
        printf("start receive file:%s, size:%d\r\n", fileinfo.filename, fileinfo.filesize);
    }

#if TFTPS_SOCKET_TYPE == SOCKET_TYPE_TCP
    sockfd = fileinfo.sockfd;
#else
    sockfd = socket(AF_INET, SOCK_DGRAM,0);
#endif
    if(sockfd<0)
    {
        perror("socket transfile");
        return (void*)BSP_ERROR;
    }
    *(unsigned int *)temp_buf = htonl(0xFFBBFFBB);
    temp_buf[4] = CMD_ID_CMDACK;
    temp_buf[5] = 1;
    tftps_send(sockfd, temp_buf, 6, (struct sockaddr *)&fileinfo.addr, sizeof(struct sockaddr));

    while(recv_size < fileinfo.filesize && error_count<3)
    {
        //等待确认报文
        FD_ZERO(&readset);
        FD_SET(sockfd, &readset);
        timeout.tv_sec = 0;
        timeout.tv_usec = TIMEOUT_VAL_MS;
        if(select(sockfd+1, &readset, NULL, NULL, &timeout)>0)
        {
            if(FD_ISSET(sockfd, &readset)>0)
            {
                struct sockaddr_in addr = {AF_INET};
                unsigned int addrlen = sizeof(addr);
                int recvlen = recvfrom(sockfd, temp_buf, sizeof(temp_buf), 0, (struct sockaddr*)&addr, &addrlen);
                if(*((unsigned int *)temp_buf) == htonl(0xFFBBFFBB) && temp_buf[4] == CMD_ID_DATA)
                {
                    unsigned short ackid = *(unsigned short*)(temp_buf+5);
                    if(pkg_id == htons(ackid))
                    {
                        unsigned int crc = *(unsigned int *)(temp_buf+7);
                        unsigned int crc_temp = calCRC(temp_buf+11, recvlen-11);
                        if(htonl(crc_temp) == crc)
                        {
                            error_count = 0;
                            memcpy(file_data_buf+recv_size, temp_buf+11, recvlen-11);
                            recv_size += recvlen-11;
                            //send ack to client
                            *(unsigned int *)temp_buf = htonl(0xFFBBFFBB);
                            temp_buf[4] = CMD_ID_ACK;
                            *(unsigned short *)(temp_buf+5) = htons(pkg_id);
                            tftps_send(sockfd, temp_buf, 9, (struct sockaddr *)&fileinfo.addr, sizeof(struct sockaddr));
                            pkg_id++;
                            if(recv_size==fileinfo.filesize)
                                break;
                        }
                        else
                        {
                            printf("error crc:0x%x[0x%x],pkgid=0x%x!\r\n", crc, crc_temp, pkg_id);
                        }
                    }
                    else
                    {
                        printf("error pkgid:0x%x[0x%x]!\r\n", pkg_id, htons(ackid));
                    }
                }
            }
            else
            {
                printf("not fd...\r\n");
            }
        }
        else
        {
            error_count++;
            printf("timeout: %d[0x%x]\r\n", error_count, pkg_id);
        }
    }

    if(recv_size==fileinfo.filesize)
    {
        //save_file(fileinfo.filename, file_data_buf, fileinfo.filesize);
        save_file(tftp_filename, file_data_buf, fileinfo.filesize);
        tftp_get_file_ok = 1;
    }
    if(file_data_buf!=NULL)
        free(file_data_buf);

    if(tftp_dbg_flag)
    {
        printf("receive fie over!recv_size=%d,fileinfo.size=%d\r\n", recv_size, fileinfo.filesize);
        printf("*************************************************\r\n");
    }
    close(sockfd);
    return NULL;
}
/***********************************************************************
*函数名称：void *transmission_thread(void *arg)
*函数功能：文件传输任务（PPC发送给ARM）
*函数参数：  参数名                                  描述
*        TransFile fileinfo = *(TransFile*)arg;  arg指向保存文件信息的结构地址
*函数返回：0成功  1失败
***********************************************************************/
void *transmission_thread(void *arg)
{
    unsigned int times = 3;      //同一个数据包丢包次数
    unsigned int is_lost = 0;
    int filefd = 0;
    int sockfd = 0;
    unsigned char sendbuf[1500] = "";
    unsigned char recvbuf[1500] = "";
    char filename[200] = "";
    unsigned int len = 0;
    struct stat st;
    unsigned short sendid = 0;
    unsigned short reacked  = 0;
    fd_set readset;

    TransFile fileinfo = *(TransFile*)arg;
    pthread_mutex_unlock(&local_lock);
    if(fileinfo.block_size==0 || fileinfo.block_size>1200)
    {
        fileinfo.block_size = 1024-11;
    }
    //if(tftp_dbg_flag){
    printf("send file *%s* to %s, block_size=%d\r\n", fileinfo.filename, inet_ntoa(fileinfo.addr.sin_addr),fileinfo.block_size);
    //}
    FD_ZERO(&readset);
#if TFTPS_SOCKET_TYPE == SOCKET_TYPE_TCP
    sockfd = fileinfo.sockfd;
#else
    sockfd = socket(AF_INET, SOCK_DGRAM,0);
#endif
    if(sockfd<0)
    {
        perror("socket transfile");
        return (void*)BSP_ERROR;
    }

    *(unsigned int *)sendbuf = htonl(0xFFBBFFBB);
    sendbuf[4] = CMD_ID_CMDACK;

    //sprintf(filename, "%s/%s", pathname, fileinfo.filename);
    filefd = open(fileinfo.filename, O_RDONLY);
    if(filefd < 0)
    {
        perror("open transfile");
        sendbuf[5] = 1; //open file error
        memcpy(sendbuf+6, "open file error", 16);
        tftps_send(sockfd, sendbuf, 5+16, (struct sockaddr *)&fileinfo.addr, sizeof(struct sockaddr));
        close(sockfd);
        return (void*)BSP_ERROR;
    }
    fstat(filefd, &st);
    if(tftp_dbg_flag)
    {
        printf("filesize=%d\r\n", st.st_size);
    }
    sendbuf[5] = 0; //open file ok
    *(unsigned int *)(sendbuf+6) = htonl(st.st_size);
    tftps_send(sockfd, sendbuf, 10, (struct sockaddr *)&fileinfo.addr, sizeof(struct sockaddr));
    while(times>0 && st.st_size>0)
    {
        struct sockaddr_in addr = {AF_INET};
        unsigned int addrlen = sizeof(addr);
        struct timeval timeout = {1, 0};
        if(!is_lost && !reacked)
        {
            //如果上包数据未丢，则从文件中读取新的一包数据发送，如果上包丢失，则不读新数据，重新发送上包数据
            memset(sendbuf, 0, sizeof(sendbuf));
            len = read(filefd, sendbuf+11, fileinfo.block_size);
            if(len==0)
            {
                break;
            }
            *(unsigned int *)sendbuf = htonl(0xFFBBFFBB);
            sendbuf[4] = CMD_ID_DATA;
            *(unsigned short *)(sendbuf+5) = htons(sendid);
            *(unsigned int *)(sendbuf+7) = htonl(calCRC(sendbuf+11, len));
        }
        if(reacked == 0)
        {
            //发送数据
            tftps_send(sockfd, sendbuf, len+11, (struct sockaddr *)&fileinfo.addr, sizeof(struct sockaddr));
        }
        //等待确认报文
        FD_SET(sockfd, &readset);
        timeout.tv_sec = 0;
        timeout.tv_usec = TIMEOUT_VAL_MS;
        if(select(sockfd+1, &readset, NULL, NULL, &timeout)>0)
        {
            if(FD_ISSET(sockfd, &readset)>0)
            {
                //接收确认报文
                int recvlen = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*)&addr, &addrlen);
                if(*((unsigned int *)recvbuf)==htonl(0xFFBBFFBB) && recvbuf[4] == CMD_ID_ACK)
                {
                    unsigned short ackid = *(unsigned short*)(recvbuf+5);
                    if(sendid == htons(ackid))
                    {
                        sendid++;
                        is_lost = 0;
                        times = 3;
                        st.st_size -= len;
                        reacked = 0;
                    }
                    else if(sendid > ackid)
                    {
                        reacked = 1;
                    }
                }
            }
        }
        else
        {
            //未收到确认报文
            is_lost = 1;
            printf("lost package...0x%x, last_size=%d\r\n", sendid, st.st_size);
            times--;
        }
    }
    close(filefd);
    close(sockfd);
    if(st.st_size>0)
    {
        printf("send file error!\r\n");
        return (void*)BSP_ERROR;
    }
    else
    {
        if(tftp_dbg_flag)
        {
            printf("send file successed!\r\n");
        }
        return (void*)BSP_OK;
    }
}

/***********************************************************************
*函数名称：void *server_thread(void *arg)
*函数功能：处理文件请求任务，接收文件下载请求，并创建文件传输任务
*函数参数：无
*函数返回：NULL
***********************************************************************/
void *server_thread(void *arg)
{
    struct sockaddr_in addr = {AF_INET};
    int addrlen = sizeof(addr);
    int len = 0;
    unsigned char buf[200] = "";
    CMDMSG *pCmd = (CMDMSG *)buf;
    char filename[200] = "";
    int server_fd = 0;
    char ifr[] = "eth1";

    //文件请求套接口
#if TFTPS_SOCKET_TYPE == SOCKET_TYPE_TCP
    server_fd = socket(AF_INET, SOCK_STREAM,0);
#else
    server_fd = socket(AF_INET, SOCK_DGRAM,0);
#endif
    if(server_fd<0)
    {
        perror("socket create");
        return 0;
    }

    if(setsockopt(server_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)ifr, sizeof(ifr))<0)
    {
        perror("setsockopt SO_BINDTODEVICE");
    }

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT_TFTPS);
    if(bind(server_fd, (struct sockaddr*)&addr, sizeof(addr))!=0)
    {
        perror("bind server_fd");
        close(server_fd);
        return 0;
    }

#if TFTPS_SOCKET_TYPE == SOCKET_TYPE_TCP
    listen(server_fd, 1);
#endif

    //接收文件下载请求
    while(1)
    {
        TransFile fileinfo;
        int fd_client = server_fd;

        memset(buf, 0, sizeof(buf));
#if TFTPS_SOCKET_TYPE == SOCKET_TYPE_TCP
        fd_client = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
        if(fd_client<0)
        {
            break;
        }
        len = recv(fd_client, buf, sizeof(buf), 0);
#else
        len = recvfrom(fd_client, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addrlen);
#endif
        if(pCmd->head != htonl(0xFFBBFFBB))
        {
            printf("error head\r\n");
            continue;
        }
		if(MCT_SLAVE == bsp_get_self_MS_state())
		{
			continue;
		}
        switch(pCmd->cmd)
        {
        case CMD_ID_CMD_GET:
            memset(&fileinfo, 0, sizeof(fileinfo));
            strcpy(fileinfo.filename, buf+5);
            fileinfo.block_size = htons(*(unsigned short*)(buf+5+strlen(fileinfo.filename)+1));
            sprintf(filename, "%s/%s", pathname, fileinfo.filename);
            if(tftp_dbg_flag)
            {
                printf("[%s] request file: *%s*,block_size=%d\r\n", inet_ntoa(addr.sin_addr), fileinfo.filename,fileinfo.block_size);
            }
            if(access(filename, F_OK)==0)
            {
                pthread_t transmission_tid;
                fileinfo.addr = addr;
                fileinfo.sockfd = fd_client;
                pthread_mutex_lock(&local_lock);
                if(0!=pthread_create(&transmission_tid, NULL, transmission_thread, (void*)&fileinfo))
                {
                    printf("[%s]:create thread error!\r\n",__func__);
                    perror("pthread_create");
					pthread_mutex_unlock(&local_lock);
					break;
                }
                if(0!=pthread_detach(transmission_tid))
                {
                    printf("[%s]:pthread detach!\r\n",__func__);
                    perror("pthread_detach");
                }
                pthread_mutex_lock(&local_lock);
                pthread_mutex_unlock(&local_lock);
            }
            else
            {
                printf("error! *%s* is not exist!\r\n", filename);
                *(unsigned int *)buf = htonl(0xFFBBFFBB);
                buf[4] = CMD_ID_CMDACK;
                buf[5] = 1;
                memcpy(buf+6, "file not exist", 16);
                tftps_send(fd_client, buf, 6+strlen(buf+6), (struct sockaddr *)&addr, sizeof(struct sockaddr));
            }
            break;
        case CMD_ID_CMD_PUT:
        {
            pthread_t transmission_tid;
            memset(&fileinfo, 0, sizeof(fileinfo));
            fileinfo.filesize = htonl(*(unsigned int *)(buf+5));
            strcpy(fileinfo.filename, buf+9);
            fileinfo.block_size = htons(*(unsigned short*)(buf+9+strlen(fileinfo.filename)+1));
            //sprintf(filename, "%s/%s", pathname, fileinfo.filename);
            if(tftp_dbg_flag)
            {
                printf("[%s] put file:*%s*, size=%d, block_size=%d\r\n", inet_ntoa(addr.sin_addr), fileinfo.filename,fileinfo.filesize,fileinfo.block_size);
            }

            fileinfo.addr = addr;
            fileinfo.sockfd = fd_client;
            pthread_create(&transmission_tid, NULL, get_file_thread, (void*)&fileinfo);
            pthread_detach(transmission_tid);
            break;
        }
        default:
            printf("unknow cmd:0x%x\r\n", pCmd->cmd);
            break;
        }
    }
    return NULL;
}

pthread_mutex_t dsp_dump_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tftp_get_file_mutex = PTHREAD_MUTEX_INITIALIZER;
/***********************************************************************
*函数名称：int init_tftp_server_thread(char *path)
*函数功能：初始化文件下载服务器
*函数参数：  参数名             描述
*           char *path       文件存放路径
*函数返回：无
***********************************************************************/
void init_tftp_server_thread(char *path)
{
    pthread_t server_tid;
    pthread_attr_t  attr;
    struct sched_param parm;

    pthread_mutex_init(&dsp_dump_mutex, NULL);
    pthread_mutex_init(&tftp_get_file_mutex, NULL);
    pthread_mutex_init(&local_lock, NULL);

    if( (path!=NULL) && (strlen(path)<(sizeof(pathname)-1)))
    {
        strcpy(pathname, path);
    }
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 91;
    pthread_attr_setschedparam(&attr, &parm);
    pthread_create(&server_tid, &attr, server_thread, NULL);
    //pthread_detach(server_tid);
    pthread_attr_destroy(&attr);
}

int tftp_get_file(u16 slotid, char *filename, int size)
{
    pthread_mutex_lock(&tftp_get_file_mutex);
    if(filename!=NULL && strlen(filename)>0)
    {
        strcpy(tftp_filename, filename);
    }
    else
    {
        strcpy(tftp_filename, "tftp_up.dat");
    }
    tftp_get_file_ok = 0;
    bsp_bbp_get_file(slotid, size);
    pthread_mutex_unlock(&tftp_get_file_mutex);
    return tftp_get_file_ok;
}

int bsp_get_dsp_dump(u16 slotid, int devid, int coreid, char *filename)
{
    int size = 0;
    pthread_mutex_lock(&dsp_dump_mutex);
    size = htonl(bsp_bbp_dsp_dump(slotid, devid, coreid));
    if(size == 0)
    {
        printf("[%s]:create dsp_dump error!\r\n", __func__);
        pthread_mutex_unlock(&dsp_dump_mutex);
        return BSP_ERROR;
    }
    if(tftp_get_file(slotid, filename, size)==0)
    {
        printf("[%s]:get dsp_dump file error!\r\n", __func__);
        pthread_mutex_unlock(&dsp_dump_mutex);
        return BSP_ERROR;
    }
    pthread_mutex_unlock(&dsp_dump_mutex);
    return BSP_OK;
}

#if 0
int main(int argc, char *argv[])
{
    init_tftp_server_thread("./");
    printf("WelCome Server...\r\n");
    while(1)
    {
        sleep(1000);
    }
    return 0;
}
#endif
