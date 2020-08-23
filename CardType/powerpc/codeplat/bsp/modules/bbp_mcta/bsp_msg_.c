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

#include "bsp_msg_proc.h"
#include "bsp_conkers_ext.h"
#include "bsp_stack.h"
#include "bsp_types.h"
#include "../ms/inc/bsp_ms.h"

static int msg_proc_fd = -1;
extern u8 g_u8bbpAckMSSwitchMsg[6];


//extern UINT8 g_fpga_load_succ ;
//extern UINT8 g_fpga_load_fail ;
unsigned msg_proc_debug= 0;
//MSG_SENDBUF_COUNT
static msg_sendbuf_t *sendbuf = NULL;
sem_t g_fpgadsp_sema;
sem_t g_pack_sem;

pthread_mutex_t g_dsp_load_mutex = PTHREAD_MUTEX_INITIALIZER;
static uint16_t pkgid = 0;
static bsp_stack_t stack_sendbufid = {NULL,0,0,0};
extern unsigned char bbp_boot_over;

void print_stack(int count)
{
    bsp_stack_print(&stack_sendbufid, count);
}
int addr_to_slot(struct sockaddr_in addr)
{
    int slot = 0;
    struct sockaddr_in addr_temp = {AF_INET};
    addr_temp.sin_addr.s_addr = inet_addr("192.168.0.100");
    if((addr.sin_addr.s_addr&0xFFFF00FF)!=addr_temp.sin_addr.s_addr)
    {
        printf("[%s]:error subBoard IP addr:%s!\r\n", __func__, inet_ntoa(addr.sin_addr));
        return -1;
    }
    slot = (addr.sin_addr.s_addr>>8) & 0x0000FF;
    return (slot>=2 && slot <= 7)?slot:-1;
}

void bsp_init_sendbuf(void)
{
    uint32_t index = 0;
    printf("[%s]...\r\n", __func__);
    sendbuf = malloc(sizeof(msg_sendbuf_t)*MSG_SENDBUF_COUNT);
    if(sendbuf==NULL)
    {
        printf("[%s]:init pmsg_ack_data error!\r\n", __func__);
        return;
    }
    bsp_stack_init(&stack_sendbufid, MSG_SENDBUF_COUNT);
    for(index=0; index<MSG_SENDBUF_COUNT; index++)
    {
        pthread_mutex_init(&sendbuf[index].wait,NULL);
        bsp_stack_push(&stack_sendbufid, index);
    }
}

msg_sendbuf_t *bsp_get_sendbuf(void)
{
    uint32_t index = 0;
    if(bsp_stack_pop(&stack_sendbufid, &index)==BSP_OK)
    {
        memset(&sendbuf[index], 0, sizeof(msg_sendbuf_t));
        pkgid = (pkgid==0)?1:(pkgid+1);
        sendbuf[index].pkgid = (index<<16) | pkgid;
        sendbuf[index].send.pkgid = htonl(sendbuf[index].pkgid);
        return &sendbuf[index];
    }
    return NULL;
}
msg_sendbuf_t *bsp_get_sendbuf_by_pkgid(int pkg_id)
{
    uint32_t index = 0;
    index = pkg_id>>16;
    if(sendbuf[index].pkgid == pkg_id)
    {
        return &sendbuf[index];
    }
    return NULL;
}
void bsp_release_sendbuf(msg_sendbuf_t *psend)
{
    uint32_t index = (psend - sendbuf);
    if(index < MSG_SENDBUF_COUNT)
    {
        sendbuf[index].pkgid = 0;
        sendbuf[index].send.pkgid = 0;
        bsp_stack_push(&stack_sendbufid, index);
    }
}

//unsigned lock_wait_flag = 0;
int msg_send(uint16_t boardid, msg_sendbuf_t *psend)
{
    struct sockaddr_in addr = {AF_INET};
    CHECK_BOARDID(boardid);
    if(msg_proc_fd >= 0 && psend!=NULL)
    {
        int ret = BSP_OK;
        uint32_t ack_id = 0;
        unsigned char target_ip[16] = "";
        addr.sin_port = htons(MSG_PROC_PORT);
        sprintf(target_ip, "192.168.%d.100", boardid);
        addr.sin_addr.s_addr = inet_addr(target_ip);
        if(msg_proc_debug > 1)
        {
            printf("target_ip=*%s*0x%x\r\n", target_ip, addr.sin_addr.s_addr);
        }

        if(psend->timeout_ms>0)
        {
            while((pthread_mutex_trylock(&psend->wait)==0));
        }
        // printf("send...psend\r\n");
        sendto(msg_proc_fd, &psend->send, PACKET_HEADER_SIZE+htonl(psend->send.datalen), 0, (struct sockaddr*)&addr, sizeof(addr));
        //每1ms查询一次有无收到数据
        //printf("psend->timeout_ms=%d\r\n", psend->timeout_ms);
        while(psend->timeout_ms > 0)
        {
            usleep(10*1000);
            //printf("wait...psend\r\n");
            ret = pthread_mutex_trylock(&psend->wait);
            if(ret == BSP_OK)
            {
                if(2 < msg_proc_debug)
                    printf("recv ack..222.\r\n");
                break;
            }
            psend->timeout_ms -= 10;
        }
        if(2 < msg_proc_debug)
            printf("33333333333!\r\n");
        if(ret==BSP_OK)
        {
            return BSP_OK; //收到回复
        }
        else
        {
            printf("[%s]Error bbp_cmd [0x%x] time out!\r\n", __func__, psend->send.cmd);
            return BSP_ERROR; //未收到回复
        }
    }
    else
    {
        printf("Error: msg_proc_fd=%d, psend=%x\r\n", msg_proc_fd, psend);
        pthread_mutex_unlock(&psend->wait);
        return BSP_ERROR;
    }
}


void *bsp_bbp_boot_dsp(void* arg)
{
    uint16_t boardid = (uint16_t)arg;
    CHECK_BBP_BOARDID(boardid);
    printf("config aif loopback...\n");
    bsp_open_aifloopback(boardid);
    bsp_boot_all_dsp(boardid);
    pthread_mutex_unlock(&g_dsp_load_mutex);
    sleep(3);
    bsp_get_srio_info(boardid);
    bsp_bbp_dsp_load_status(boardid, DSP_FPGA_STATE_LOADED);
    /* fpga开工 */
    bbp_fpga_write_reg(boardid, 48, 0x10);
    bbp_boot_over = 1;
    return NULL;
}

int bsp_bbp_load_dsp(uint16_t boardid)
{
    CHECK_BBP_BOARDID(boardid);
    if(pthread_mutex_trylock(&g_dsp_load_mutex)==BSP_OK)
    {
        pthread_t tid;
        pthread_create(&tid, NULL, bsp_bbp_boot_dsp, (void*)boardid);
        pthread_detach(tid);
        return BSP_OK;
    }
    return BSP_ERROR;
}

void *msg_proc_thread(void *arg)
{
    struct sockaddr_in addr = {AF_INET};
    int addrlen = sizeof(addr);
    int i = 0;
    msg_sendbuf_t sbuf;
    char ifr[] = "eth1";

    bsp_init_sendbuf();

    msg_proc_fd = socket(AF_INET, SOCK_DGRAM,0);
    if(msg_proc_fd<0)
    {
        perror("[msg_proc_thread]:socket udp");
        return NULL;
    }

    if(setsockopt(msg_proc_fd, SOL_SOCKET, SO_BINDTODEVICE, (void *)ifr, sizeof(ifr))<0)
    {
        perror("setsockopt SO_BINDTODEVICE");
    }

    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(MSG_PROC_PORT);
    if(bind(msg_proc_fd, (struct sockaddr*)&addr, sizeof(addr))!=0)
    {
        perror("[msg_proc_thread]:bind msg_proc_fd");
        close(msg_proc_fd);
        msg_proc_fd = -1;
        return  NULL;
    }

    while(1)
    {
        int len = 0;
        int slot = 0;
        msg_sendbuf_t *psendbuf = NULL;
        memset(&sbuf.ack, 0, sizeof(sbuf.ack));
        len = recvfrom(msg_proc_fd, (char*)&sbuf.ack, sizeof(sbuf.ack), 0, (struct sockaddr *)&addr, &addrlen);
        if (len < 0)
        {
            perror("Failed to receive packet");
            continue;
        }
        //printf("pCmd->header=0x%x, pCmd->cmd=0x%x\r\n", pCmd->header, pCmd->cmd);
	if(MCT_SLAVE == bsp_get_self_MS_state())
	{
		continue;
	}
        if(sbuf.ack.header != htonl(NET_MSG_HEADER))
        {
            printf("[msg_proc_thread]:error head! pCmd->header=0x%x\r\n", htonl(sbuf.ack.header));
            continue;
        }

        psendbuf = bsp_get_sendbuf_by_pkgid(htonl(sbuf.ack.pkgid));
        if(psendbuf != NULL)
        {
            psendbuf->ack = sbuf.ack;
            if(5 < msg_proc_debug)
            {
                printf("psendbuf->ack.cmd:0x%lx\r\n",psendbuf->ack.cmd);
            }
#if 1
            if(psendbuf->timeout_ms > 0)
            {
                if(2 < msg_proc_debug)
                    printf("recv msg...1111\r\n");
                if (0 !=pthread_mutex_unlock(&psendbuf->wait))
                {
                    printf("pthread_mutex_unlock fail!\r\n");
                }
                continue;
            }
#endif
        }
        slot = addr_to_slot(addr);
        if(1 < msg_proc_debug)
        {
            printf("[%s]: slot=%d, addr:0x%x, mcu_status=0x%x\r\n", inet_ntoa(addr.sin_addr), slot, addr.sin_addr.s_addr,boards[slot].mcu_status);
        }
        if(slot < 0)
        {
            continue;
        }
        switch(sbuf.ack.cmd)
        {
        case CMD_PRINT_MSG:
            printf("[%s]:%s\r\n", inet_ntoa(addr.sin_addr), sbuf.ack.data);
            break;
        case CMD_FPGA_LOAD:
        {
            uint8_t fpgaid = sbuf.ack.data[0];
            if((fpgaid == BBP_FPGA) || (fpgaid == ES_FPGA))
            {
                boards[slot].fpga_status = DSP_FPGA_STATE_NOLOAD;                
                printf("[%s:%d]:load fpga ack!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
            else if(fpgaid == FSA_FPGA_160T)
            {
                boards[slot].fpga_status &= ~FSA_FPGA_160T_LOADED;
                printf("[%s:%d]:load fsa_fpga_160t ack!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
            else if(fpgaid == FSA_FPGA_325T)
            {
                boards[slot].fpga_status &= ~FSA_FPGA_325T_LOADED;
                printf("[%s:%d]:load fsa_fpga_325t ack!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
        }
        break;
        case CMD_BOARD_RESET:
            printf("[%s:%d]:reset bbp ack!\r\n", inet_ntoa(addr.sin_addr), slot);
            boards[slot].mcu_status = MCU_STATUS_RESET_ACKED;
            break;
        case CMD_FPGA_SUCCESS_MSG:
        {
            uint8_t fpgaid = sbuf.ack.data[0];
            if((fpgaid == BBP_FPGA) || (fpgaid == ES_FPGA))
            {
                boards[slot].fpga_status = DSP_FPGA_STATE_LOADED;
                printf("[%s:%d]fpga load success!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
            else if(fpgaid == FSA_FPGA_160T)
            {
                boards[slot].fpga_status |= FSA_FPGA_160T_LOADED;
                printf("[%s:%d]fsa_fpga_160t load success!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
            else if(fpgaid == FSA_FPGA_325T)
            {
                boards[slot].fpga_status |= FSA_FPGA_325T_LOADED;
                printf("[%s:%d]fsa_fpga_325t load success!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
        }
        break;
        case CMD_FPGA_FAIL_MSG:
        {
            uint8_t fpgaid = sbuf.ack.data[0];
            if((fpgaid == BBP_FPGA) || (fpgaid == ES_FPGA))
            {
                boards[slot].fpga_status = DSP_FPGA_STATE_NOLOAD;
                printf("[%s:%d]fpga load failed!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
            else if(fpgaid == FSA_FPGA_160T)
            {
                boards[slot].fpga_status &= ~FSA_FPGA_160T_LOADED;
                printf("[%s:%d] fsa_fpga_160t load failed!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
            else if(fpgaid == FSA_FPGA_325T)
            {
                boards[slot].fpga_status &= ~FSA_FPGA_325T_LOADED;
                printf("[%s:%d] fsa_fpga_325t load failed!\r\n", inet_ntoa(addr.sin_addr), slot);
            }
        }
        break;
        case CMD_BOARD_START:
        {
            unsigned char board_type = 0;
            board_type = sbuf.ack.data[0];
            //if(0 < msg_proc_debug)
            printf("recv first msg: [%s] slot=%d, board_type=0x%x\r\n", inet_ntoa(addr.sin_addr), slot,board_type);
            //bsp_recv_firstmsg(slot, board_type);
            bsp_bbp_start(slot);
        }
        break;
        case CMD_MCTA_START:
            boards[slot].mcu_alive = 1;
            g_u8bbpAckMSSwitchMsg[slot-2] = MCT_MS_SWITCH_EVENT_CLEAR;
            printf("recv mct start ack: [%s] slot=%d.\r\n", inet_ntoa(addr.sin_addr), slot);
            break;
        case CMD_HEART_BEAT:
        {
            unsigned char board_type = 0;
            board_type = sbuf.ack.data[0];
            //boards[slot].type = board_type;
            //bbp_board_ready |= sbuf.ack.data[1];
            if(0 < msg_proc_debug)
                printf("recv heart: board_type=0x%x, data[1]=%d, mcu_status=0x%x\r\n", board_type, sbuf.ack.data[1], boards[slot].mcu_status);
            //bsp_subboard_recv_heartBt(slot, board_type);
        }
        break;
        default:
        {
            int tt = 0;
            unsigned char *p = (unsigned char*)&sbuf.ack;
            printf("bsp_msg_proc: error cmd:\r\n");
            for(tt=0; tt<24; tt++)
            {
                printf("0x%x ", p[tt]);
            }
            printf("\r\n");
        }
        break;
        }
    }
}

void init_msg_proc_thread(void)
{
    pthread_t server_tid;
    pthread_attr_t  attr;
    struct sched_param parm;
    pthread_attr_init(&attr);
    pthread_mutex_init(&g_dsp_load_mutex, NULL);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 92;
    pthread_attr_setschedparam(&attr, &parm);
    pthread_create(&server_tid, &attr, msg_proc_thread, NULL);
    pthread_attr_destroy(&attr);
    sleep(1);

    //pthread_detach(server_tid);
}

int test_msg_proc_main(int argc, char *argv[])
{
    init_msg_proc_thread();
    init_tftp_server_thread("./");
    printf("WelCome Server...\r\n");
    while(1)
    {
        sleep(1000);
    }
    return 0;
}

