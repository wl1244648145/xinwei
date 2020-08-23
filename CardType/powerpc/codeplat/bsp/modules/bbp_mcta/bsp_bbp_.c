#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "bsp_bbp_command.h"
#include "bsp_i2c_ext.h"
#include "bsp_msg_proc.h"
#include "bsp_conkers_ext.h"
#include "../ms/inc/bsp_ms.h"

EEPROM_PAR_STRU g_bbp_eeprom_par;

int8_t bsp_mcta_start(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    uint8_t ret = 0;
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_MCTA_START);
    sbuf->send.data[0] = bsp_get_slot_id();
    sbuf->send.datalen = htonl(1);
    sbuf->timeout_ms = 500;
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}

int bsp_mcta_start_msg(void)
{
	u16 boardid;

	for(boardid=2; boardid<8; boardid++)
	{
		bsp_mcta_start(boardid);
	}

	return BSP_OK;
}

int8_t bsp_inform_bbp_master_mcta_slotid(u16 boardid,u8 slotid)
{
    msg_sendbuf_t *sbuf = NULL;
    uint8_t ret = 0;
	sbuf = bsp_get_sendbuf();
	if(sbuf==NULL)
	{
		printf("[%s]:get sendbuf error!\r\n", __func__);
		return BSP_ERROR;
	}
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_MCTA_START);
	sbuf->send.data[0] = slotid;
    sbuf->send.datalen = htonl(1);
	sbuf->timeout_ms = 0;
    ret = msg_send(boardid, sbuf);
	bsp_release_sendbuf(sbuf);
    return ret;
}

extern u8 g_u8bbpAckMSSwitchMsg[6];
int bsp_inform_bbp_MS_switch(u8 master_slotid)
{
	u16 boardid;
	int i;

	memset(&g_u8bbpAckMSSwitchMsg,0,sizeof(g_u8bbpAckMSSwitchMsg));
	
	for(boardid = IPMB_SLOT2; boardid <= IPMB_SLOT7; boardid++)
	{
		if((boards[boardid].mcu_status&0xF0)==MCU_STATUS_RECV_FIRSTMSG)
		{
			g_u8bbpAckMSSwitchMsg[boardid-2] = MCT_MS_SWITCH_INFORMED;
			bsp_inform_bbp_master_mcta_slotid(boardid,master_slotid);
		}
	}
		
}


int8_t bsp_bbp_start(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    uint8_t ret = 0;
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_BOARD_START);
    sbuf->send.data[0] = bsp_get_slot_id(0);
    sbuf->send.datalen = htonl(1);
    sbuf->timeout_ms = 0;
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_get_boardtype(uint32_t boardid, uint8_t *boardtype)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL || boardtype==NULL)
    {
        printf("[%s]:error! sendbuf=%p, pboardtype=%p!\r\n", __func__, sbuf,boardtype);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_BOARD_TYPE);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        *boardtype = sbuf->ack.data[0];
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_get_pcb_version(uint32_t boardid, uint8_t * pcb_version)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL || (pcb_version==NULL))
    {
        printf("[%s]:error! sendbuf=%p, ppcb_version=%p!\r\n", __func__, sbuf,pcb_version);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_PCB_VERSION);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        *pcb_version = sbuf->ack.data[0];
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_get_slot(uint32_t boardid, uint8_t *slot)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p, pslot=%p!\r\n", __func__, sbuf,slot);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_SLOT);
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    if(ret == BSP_OK)
    {
        *slot = sbuf->ack.data[0];
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_cpld_read(uint32_t boardid, uint8_t reg, uint8_t *val)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL ||val==NULL)
    {
        printf("[%s]:error! sendbuf=%p, pval=%p!\r\n", __func__, sbuf,val);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_CPLD_REG_READ);
    sbuf->send.datalen = htonl(1);
    sbuf->timeout_ms = 1000;
    sbuf->send.data[0] = reg;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        *val = sbuf->ack.data[1];
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_cpld_write(uint32_t boardid, uint8_t reg, uint8_t val)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_CPLD_REG_WRITE);
    sbuf->send.datalen = htonl(2);
    sbuf->send.data[0] = reg;
    sbuf->send.data[1] = val;
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_fpga_read(uint32_t boardid, uint8_t reg, uint16_t *val)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_FPGA_REG_READ);
    sbuf->send.datalen = htonl(2);
    sbuf->timeout_ms = 1000;
    *(uint16_t*)(sbuf->send.data) = htons(reg);
    ret = msg_send(boardid, sbuf);
    if(ret == BSP_OK)
    {
        *val = *(uint16_t*)(sbuf->ack.data+2);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
unsigned int fpga_debug = 0;
int8_t bsp_bbp_fpga_write(uint32_t boardid, uint8_t reg, uint16_t val)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_FPGA_REG_WRITE);
    sbuf->send.datalen = htonl(4);
    sbuf->timeout_ms = 1000;
    *(uint16_t*)(sbuf->send.data) = htons(reg);
    *(uint16_t*)(sbuf->send.data+2) = htons(val);
    if(1 == fpga_debug)
    {
        printf("write fpga reg:%d[0x%x]-->%d[0x%x]\r\n", reg,reg,val,val);
    }
    ret = msg_send(boardid, sbuf);
    if(1 == fpga_debug)
    {
        printf("write fpga reg:%d[0x%x] Over!\r\n",reg,reg);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_fpga_load(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_FPGA_LOAD);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 0;
    ret = msg_send(boardid, sbuf);
#if 0
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        printf("bsp_bbp_fpga_load error,sbuf->ack.data[0]=0x%lx\r\n",sbuf->ack.data[0]);
        ret = BSP_ERROR;
    }
#endif
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_fsa_fpga_read(uint32_t boardid, uint8_t fpgaid, uint16_t reg, uint16_t *val)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_FPGA_REG_READ);
    sbuf->send.datalen = htonl(3);
    sbuf->timeout_ms = 1000;
    sbuf->send.data[0] = fpgaid;
    *(uint16_t*)(sbuf->send.data + 1) = htons(reg);
    ret = msg_send(boardid, sbuf);
    if(ret == BSP_OK)
    {
        *val = *(uint16_t*)(sbuf->ack.data+3);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_fsa_fpga_write(uint32_t boardid, uint8_t fpgaid, uint8_t reg, uint8_t val)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_FPGA_REG_WRITE);
    sbuf->send.datalen = htonl(5);
    sbuf->timeout_ms = 1000;
    sbuf->send.data[0] = fpgaid;
    *(uint16_t*)(sbuf->send.data+1) = htons(reg);
    *(uint16_t*)(sbuf->send.data+3) = htons(val);
    if(1 == fpga_debug)
    {
        printf("write fpga reg:%d[0x%x]-->%d[0x%x]\r\n", reg,reg,val,val);
    }
    ret = msg_send(boardid, sbuf);
    if(1 == fpga_debug)
    {
        printf("write fpga reg:%d[0x%x] Over!\r\n",reg,reg);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_fsa_fpga_load(uint8_t fpgaid, uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_FPGA_LOAD);
    sbuf->send.data[0] = htonl(fpgaid);
    sbuf->send.datalen = htonl(1);
    sbuf->timeout_ms = 0;
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_bbp_reset(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    uint8_t ret = 0;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_BOARD_RESET);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 0;
    //如果复位成功，则会超时，否则会收到回复
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_bbp_mcu_update(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = 0;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_TEST_MCU_UPLOAD);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 2000;
    ret = msg_send(boardid, sbuf); //返回失败说明板卡正在重启，准备更新MCU
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_bbp_dsp_load_status(uint16_t boardid, uint8_t dsp_load_status){
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL){
    	printf("[%s]:get sendbuf error!\r\n", __func__);
    	return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_SET_DSP_LOAD_STATUS);
    sbuf->send.datalen = htonl(1);
    sbuf->send.data[0] = dsp_load_status;
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_bbp_dsp_reset(uint32_t boardid, uint8_t dsp_set)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_DSP_RESET);
    sbuf->send.datalen = htonl(1);
    sbuf->send.data[0] = dsp_set;
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_cpld_update(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_CPLD_UPLOAD);
    sbuf->send.datalen = htonl(0);
    //sbuf->timeout_ms = 20*1000;
    sbuf->timeout_ms = 0;
    ret = msg_send(boardid, sbuf);
    #if 0
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    #endif
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_dsp_close(uint32_t boardid, uint8_t dsp_set)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:get sendbuf error!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_DSP_CLOSE);
    sbuf->send.datalen = htonl(1);
    sbuf->send.data[0] = dsp_set;
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_read_temp(uint32_t boardid, uint8_t *temp)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL || temp==NULL)
    {
        printf("[%s]:error! sendbuf=%p, temp=%p!\r\n", __func__, sbuf,temp);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_TEMPERATURE);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        if(BOARD_TYPE_BBP == boards[boardid].type)
            memcpy(temp, sbuf->ack.data, 8);
        if(BOARD_TYPE_FSA == boards[boardid].type)
            memcpy(temp, sbuf->ack.data, 4);
        if(BOARD_TYPE_ES == boards[boardid].type)
            memcpy(temp, sbuf->ack.data, 2);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_bbp_powerinfo(uint32_t boardid, float *power)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL || power==NULL)
    {
        printf("[%s]:error! sendbuf=%p, power=%p!\r\n", __func__, sbuf,power);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_POWER_INFO);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        memcpy(power, sbuf->ack.data, 12);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_get_srio_info(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    uint8_t ret = 0;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_SRIO_PORT_STATUS);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 3000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        u8 *pu8Buf = (u8 *)sbuf->ack.data;
        u8 u8icnt = 0;
        for(u8icnt = 0; u8icnt < 9; u8icnt++)
        {
    		boards[boardid].srio_status[u8icnt] = (pu8Buf[0 + u8icnt*4]<<24) | (pu8Buf[1 + u8icnt*4]<<16) | (pu8Buf[2 + u8icnt*4]<<8) | (pu8Buf[3 + u8icnt*4]);
        }
        if(BOARD_TYPE_BBP == boards[boardid].type)
        {
            printf("[%d]srio port status:0x%lx,0x%lx,0x%lx,0x%lx,0x%lx\r\n", boardid,
    		    boards[boardid].srio_status[0],boards[boardid].srio_status[1],boards[boardid].srio_status[2],
    		    boards[boardid].srio_status[3],boards[boardid].srio_status[4]);
        }
        if(BOARD_TYPE_FSA == boards[boardid].type)
        {
            printf("[%d]srio port status:0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx\r\n", boardid,
    		    boards[boardid].srio_status[0],boards[boardid].srio_status[1],boards[boardid].srio_status[2],
    		    boards[boardid].srio_status[3],boards[boardid].srio_status[4],boards[boardid].srio_status[5],
    		    boards[boardid].srio_status[6],boards[boardid].srio_status[7],boards[boardid].srio_status[8]);
        }
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

UINT32 bsp_show_cps1616_escsrInfo(UINT32 regVal)
{
    if (regVal & (1<<0))
    {
        printf("    Port Un-initialized \n");
    }
    if (regVal & (1<<1))
    {
        printf("    Port OK \n");
    }
    if (regVal & (1<<2))
    {
        printf("    Port ERROR \n");
    }
    if (regVal & (1<<3))
    {
        printf("    Port UNAVAILABLE \n");
    }
    if (regVal & (1<<4))
    {
        printf("    Port WRITE PENDING \n");
    }
    if (regVal & (1<<8))
    {
        printf("    Port INPUT ERROR STOPPED \n");
    }
    if (regVal & (1<<9))
    {
        printf("    Port INPUT ERROR ENCOUNTERED \n");
    }
    if (regVal & (1<<10))
    {
        printf("    Port INPUT RETRY STOPPED \n");
    }
    if (regVal & (1<<16))
    {
        printf("    Port OUTPUT ERROR STOPPED \n");
    }
    if (regVal & (1<<17))
    {
        printf("    Port OUTPUT ERROR ENCOUNTERED \n");
    }
    if (regVal & (1<<18))
    {
        printf("    Port OUTPUT RETRY STOPPED \n");
    }
    if (regVal & (1<<19))
    {
        printf("    Port OUTPUT RETRYIED \n");
    }
    if (regVal & (1<<20))
    {
        printf("    Port OUTPUT RETRY ENCOUNTERED \n");
    }
    if (regVal & (1<<24))
    {
        printf("    Port OUTPUT DEGRADED ENCOUNTERED \n");
    }
    if (regVal & (1<<25))
    {
        printf("    Port OUTPUT FAILED ENCOUNTERED \n");
    }
    if (regVal & (1<<26))
    {
        printf("    Port OUTPUT PACKET DROPPED \n");
    }
    if (regVal & (1<<29))
    {
        printf("    Port IDLE_SEQUENCE ACTIVE \n");
    }
    if (regVal & (1<<30))
    {
        printf("    Port IDLE2_SEQUENCE_ENABLE \n");
    }
    if (regVal & (1<<31))
    {
        printf("    Port IDLE2_SEQUENCE_SUPPORT \n");
    }
}

void bbp_sriosw_get_port_status(uint32_t boardid)
{
    u8 u8cnt = 0;

    if(BSP_OK==bsp_get_srio_info(boardid))
    {
        if(BOARD_TYPE_BBP == boards[boardid].type)
        {
            for(u8cnt=0;u8cnt<g_SrioBbpDevParamNum;u8cnt++)
            {
                printf("%s status is:\r\n", strBbpSrioDevParam[u8cnt].Name);
                bsp_show_cps1616_escsrInfo(boards[boardid].srio_status[u8cnt]);
            }
        }
        else if(BOARD_TYPE_FSA == boards[boardid].type)
        {
            for(u8cnt=0;u8cnt<g_SrioFsaDevParamNum;u8cnt++)
            {
                printf("%s status is:\r\n", strFsaSrioDevParam[u8cnt].Name);
                bsp_show_cps1616_escsrInfo(boards[boardid].srio_status[u8cnt]);
            }
        }
        else
        {
            printf("[%s]:error boardtype 0x%x\r\n", __func__, boards[boardid].type);
        }
    }
}

int8_t bsp_get_geswitch_port_status(uint8_t boardid, uint16_t *u16status)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    int8_t u8icnt = 0;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_ETHSW_LINK_STATUS);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        *u16status = *(uint16_t*)(sbuf->ack.data);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

void bbp_ethsw_get_port_status(uint8_t boardid)
{
    uint16_t u16status = 0;
    uint8_t u8boardtype = 0;

    /* 获取板类型 */
    u8boardtype = boards[boardid].type;
    if((u8boardtype != BOARD_TYPE_BBP) && (u8boardtype != BOARD_TYPE_FSA) && (u8boardtype != BOARD_TYPE_ES))
    {
        printf("[%s]:error boardtype(0x%x)\r\n", u8boardtype);
    }
    /* 读取芯片端口状态寄存器的值 */
    if(BSP_OK==bsp_get_geswitch_port_status(boardid, &u16status))
    {
        if(BOARD_TYPE_BBP == boards[boardid].type)
        {
            if(u16status&BBP_DSP0_PORT)
                printf("BBP_DSP0_PORT is link up!\r\n");
            else
                printf("BBP_DSP0_PORT is link down!\r\n");
            if(u16status&BBP_DSP1_PORT)
                printf("BBP_DSP1_PORT is link up!\r\n");
            else
                printf("BBP_DSP1_PORT is link down!\r\n");
            if(u16status&BBP_DSP2_PORT)
                printf("BBP_DSP2_PORT is link up!\r\n");
            else
                printf("BBP_DSP2_PORT is link down!\r\n");
            if(u16status&BBP_DSP3_PORT)
                printf("BBP_DSP3_PORT is link up!\r\n");
            else
                printf("BBP_DSP3_PORT is link down!\r\n");
        }
        else if(BOARD_TYPE_FSA == boards[boardid].type)
        {
            if(u16status&FSA_PHY_PORT)
                printf("FSA_PHY_PORT is link up!\r\n");
            else
                printf("FSA_PHY_PORT is link down!\r\n");
        }
        if(u16status&FPGA_PORT)
                printf("FPGA_PORT is link up!\r\n");
        else
            printf("FPGA_PORT is link down!\r\n");
        if(u16status&PORT_PPC_SLOT0)
                printf("PORT_PPC_SLOT0 is link up!\r\n");
        else
            printf("PORT_PPC_SLOT0 is link down!\r\n");
         if(u16status&PORT_PPC_SLOT1)
                printf("PORT_PPC_SLOT1 is link up!\r\n");
        else
            printf("PORT_PPC_SLOT1 is link down!\r\n");
    }
}

int8_t bsp_bbp_arm_version(uint32_t boardid, char *version)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    int8_t u8len = 0;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL || version==NULL)
    {
        printf("[%s]:error! sendbuf=%p, version=%p!\r\n", __func__, sbuf,version);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_SW_VERSION);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 1000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        u8len = sbuf->ack.datalen;
        memcpy(version, sbuf->ack.data, u8len);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_bbp_get_sfpinfo(u8 slotid, uint8_t sfp_id, fiber_info *sfpinfo)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(slotid);	
    if(sfpinfo==NULL)
    {
        printf("[%s]:error! sfpinfo==NULL!\r\n", __func__);
        return BSP_ERROR;	
    }
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
    	printf("[%s]:get sendbuf error!\r\n", __func__);
    	return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_SFP_INFO);
    sbuf->timeout_ms = 1000;
    sbuf->send.data[0] = sfp_id;
    sbuf->send.datalen = 1;
    ret = msg_send(slotid, sbuf);
    if(ret == BSP_OK)
    {
    	//memcpy(sfpinfo,&(sbuf->ack.data[0]),sizeof(sfpinfo)); 
    	sfpinfo->los = sbuf->ack.data[0];
    	sfpinfo->temper = *(uint32_t*)(sbuf->ack.data+1);
    	sfpinfo->vol = *(uint32_t*)(sbuf->ack.data+5);
    	sfpinfo->current = *(uint32_t*)(sbuf->ack.data+9);
    	sfpinfo->tx_power = *(uint32_t*)(sbuf->ack.data+13);
    	sfpinfo->rx_power = *(uint32_t*)(sbuf->ack.data+17);
    	sfpinfo->speed = *(uint32_t*)(sbuf->ack.data+21);
    	strncpy(sfpinfo->vendor_name,&(sbuf->ack.data[25]),16);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int bsp_bbp_get_sfpinfo_test(u8 slotid, uint8_t sfp_id)
{
    int i;
    fiber_info sfpinfo ={0};

    if(BSP_OK == bsp_bbp_get_sfpinfo(slotid,sfp_id,&sfpinfo))
    {
        printf("los_state = %d\n", sfpinfo.los);
        printf("temperature = %6.2f(℃)\n", sfpinfo.temper);
        printf("vol = %8.2f(uV)\n", sfpinfo.vol);
        printf("current = %6.2f(uA)\n", sfpinfo.current);
        printf("tx_power = %6.2f(uW)\n", sfpinfo.tx_power);
        printf("rx_power = %6.2f(uW)\n", sfpinfo.rx_power);
        printf("speed = %d\n", sfpinfo.speed);
        printf("vendorname = %s\n", sfpinfo.vendor_name);
    }
}

int bsp_bbp_dsp_dump(uint32_t boardid, uint8_t devid, uint8_t coreid)
{
    msg_sendbuf_t *sbuf = NULL;
    int ret = BSP_ERROR;
    int i = 0;
    CHECK_BBP_BOARDID(boardid);
    //	printf("[%s]:error! devid=%d, coreid=%d!\r\n", __func__, devid, coreid);
    //	return BSP_ERROR;
    //}

    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=NULL!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_DSP_DUMP);
    sbuf->send.data[0] = devid;
    sbuf->send.data[1] = coreid;
    sbuf->send.datalen = htonl(2);
    sbuf->timeout_ms = 5000;
    ret = msg_send(boardid, sbuf);

    if(ret == BSP_OK)
    {
        ret = htonl(*(int*)sbuf->ack.data);
    }
    else
    {
        ret = 0;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_get_file(uint32_t boardid, int size)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=NULL!\r\n", __func__);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_TFTP_GET_FILE);
    *(unsigned int *)sbuf->send.data = htonl(size);
    sbuf->send.datalen = htonl(4);
    sbuf->timeout_ms = 10000;
    ret = msg_send(boardid, sbuf);
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_test_eeprom(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_EEPROM_TEST);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 10*1000;
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_bbp_test_sdram(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_TEST_SDRAM);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 30*1000;
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t bsp_bbp_test_ethsw(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_TEST_ETHSW);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_bbp_test_srioswt(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_TEST_SRIOSWT);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_fsa_test_phy54210s(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_TEST_PHY54210S);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_test_pll1_config(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_TEST_PLL1_CFG);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 10*1000;
    ret = msg_send(boardid, sbuf);
    if((ret!=BSP_OK) || (sbuf->ack.data[0]!=1))
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_get_pll1_lock_status(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_PLL1_STATUS);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 2*1000;
    ret = msg_send(boardid, sbuf);
    if((ret!=BSP_OK) || (sbuf->ack.data[0]!=0))
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_test_pll2_config(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_TEST_PLL2_CFG);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 10*1000;
    ret = msg_send(boardid, sbuf);
    if((ret!=BSP_OK) || (sbuf->ack.data[0]!=1))
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_get_pll2_lock_status(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_PLL2_STATUS);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 2*1000;
    ret = msg_send(boardid, sbuf);
    if((ret!=BSP_OK) || (sbuf->ack.data[0]!=1))
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t bsp_get_fsa_fpga160t_sync_status(uint32_t boardid, uint16_t *u16syncstatus)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_GET_FPGA160T_SYNC_STATUS);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 10*1000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        *u16syncstatus = *(uint16_t*)(sbuf->ack.data);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

void bsp_test_board_type(uint32_t boardid)
{
    uint8_t boardtype = 0;
    CHECK_BBP_BOARDID(boardid);
    if(bsp_get_boardtype(boardid, &boardtype)==BSP_OK)
    {
        printf("boardtype=0x%x\r\n", boardtype);
    }
    else
    {
        printf("read boardtype timeout!\r\n");
    }
}
void bsp_test_slot(uint32_t boardid)
{
    uint8_t slot = 0;
    CHECK_BBP_BOARDID(boardid);
    if(bsp_get_slot(boardid, &slot)==BSP_OK)
    {
        printf("slot=0x%x\r\n", slot);
    }
    else
    {
        printf("read slot timeout!\r\n");
    }
}
void bsp_test_pcb_version(uint32_t boardid)
{
    uint8_t pcb_version = 0;
    CHECK_BBP_BOARDID(boardid);
    if(bsp_get_pcb_version(boardid, &pcb_version)==BSP_OK)
    {
        printf("pcb_version=0x%x\r\n", pcb_version);
    }
    else
    {
        printf("read pcb_version timeout!\r\n");
    }
}
void bsp_test_bbp_temp(uint32_t boardid)
{
    uint8_t temp[8] = {0};
    CHECK_BBP_BOARDID(boardid);  
    if(bsp_bbp_read_temp(boardid, temp)==BSP_OK)
    {
        if(BOARD_TYPE_BBP == boards[boardid].type)
        {
            printf("bbp_temp:%d %d %d %d %d %d %d %d\r\n", temp[0],temp[1],temp[2],temp[3],temp[4],temp[5],temp[6],temp[7]);
        }
        else if(BOARD_TYPE_FSA == boards[boardid].type)
        {
            printf("fsa_temp:%d %d %d %d\r\n", temp[0],temp[1],temp[2],temp[3]);
        }
        else if(BOARD_TYPE_ES == boards[boardid].type)
        {
            printf("es_temp:%d\r\n", *(int16_t *)(temp));
        }
    }
    else
    {
        printf("read bbp_temp error!\r\n");
    }
}
void bsp_test_bbp_power(uint32_t boardid)
{
    float power[3] = {0.0};
    CHECK_BBP_BOARDID(boardid);
    if(bsp_bbp_powerinfo(boardid, power)==BSP_OK)
    {
        printf("bbp: vol=%f, current=%f, power=%f\r\n", power[0],power[1],power[2]);
    }
    else
    {
        printf("read bbp_temp error!\r\n");
    }
}

void bsp_test_fpga_read_addr(uint32_t boardid, uint8_t fpgaid, uint16_t reg)
{
    u16 u16regval = 0;
    if(BOARD_TYPE_FSA == boards[boardid].type)
    {
        if(BSP_OK == bsp_fsa_fpga_read(boardid, fpgaid, reg, &u16regval))
        {
            printf("slot(%d) fpga reg(%d) : val(0x%x)\r\n", boardid, reg, u16regval);
        }
    }
    else
    {
         if(BSP_OK == bsp_bbp_fpga_read(boardid, reg, &u16regval))
        {
            printf("slot(%d) fpga reg(%d) : val(0x%x)\r\n", boardid, reg, u16regval);
        }
    }
}

void bsp_test_get_fsa_fpga160t_syncstatus(uint32_t boardid)
{
    u16 u16status = 0;
    if(BSP_OK == bsp_get_fsa_fpga160t_sync_status(boardid, &u16status))
    {
        printf("fsa fpga_160t syncstatus:0x%x\r\n", u16status);
    }
}

int8_t board_eeprom_set_crc(uint32_t boardid, u16 crc)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_CRC);
    sbuf->send.datalen = htonl(2);
    sbuf->timeout_ms = 5*1000;
    *(uint16_t*)(sbuf->send.data) = htons(crc);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_get_crc(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_GET_CRC);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;

    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        g_bbp_eeprom_par.checkSum = *(uint16_t*)(sbuf->ack.data);
    }

    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_deviceid(uint32_t boardid, char *deviceid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    int8_t len = strlen(deviceid);

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_DEVICE_ID);
    sbuf->send.datalen = htonl(len);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, deviceid, len);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_get_deviceid(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_GET_DEVICE_ID);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;

    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        memcpy(g_bbp_eeprom_par.device_id, sbuf->ack.data, 16);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_boardtype(uint32_t boardid, char *boardtype)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    int8_t len = strlen(boardtype);

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_BOARD_TYPE);
    sbuf->send.datalen = htonl(len);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, boardtype, len);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_get_boardtype(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_GET_BOARD_TYPE);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;

    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        memcpy(g_bbp_eeprom_par.board_type, sbuf->ack.data, 32);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
int8_t board_eeprom_set_productsn(uint32_t boardid, u8 *productsn)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_PRODUCT_SN);
    sbuf->send.datalen = htonl(32);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, productsn, 32);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}


int8_t board_eeprom_get_productsn(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_GET_PRODUCT_SN);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;

    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        memcpy(g_bbp_eeprom_par.product_sn, sbuf->ack.data, 32);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_manufacturer(uint32_t boardid, char *manufacturer)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_MANUFACTURER);
    sbuf->send.datalen = htonl(12);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, manufacturer, 12);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_get_manufacturer(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_GET_MANUFACTURER);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;
    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        memcpy(g_bbp_eeprom_par.manufacturer, sbuf->ack.data, 12);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_productdate(uint32_t boardid, u8 *productdate)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_PRODUCT_DATE);
    sbuf->send.datalen = htonl(4);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, productdate, 4);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_get_productdate(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_GET_PRODUCT_DATE);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;

    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        memcpy(g_bbp_eeprom_par.product_date, sbuf->ack.data, 4);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_tempthreshold(uint32_t boardid, s8 *tempthreshold)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_TEMP_THRESHOLD);
    sbuf->send.datalen = htonl(2);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, tempthreshold, 2);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_get_tempthreshold(uint32_t boardid)
{
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    CHECK_BBP_BOARDID(boardid);
    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_GET_TEMP_THRESHOLD);
    sbuf->send.datalen = htonl(0);
    sbuf->timeout_ms = 5*1000;

    ret = msg_send(boardid, sbuf);
    if(ret==BSP_OK)
    {
        memcpy(g_bbp_eeprom_par.temperature_threshold, sbuf->ack.data, 2);
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

/*********************************************************************************/
/*                                      for test                                 */
/*********************************************************************************/
#if 1
int8_t board_eeprom_set_crc_test(uint32_t boardid)
{
    u16 crc = 0x1234;
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_CRC);
    sbuf->send.datalen = htonl(2);
    sbuf->timeout_ms = 5*1000;
    *(uint16_t*)(sbuf->send.data) = htons(crc);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_deviceid_test(uint32_t boardid)
{
    char *deviceid = "C6482";
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    int8_t len = strlen(deviceid);

    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_DEVICE_ID);
    sbuf->send.datalen = htonl(len);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, deviceid, len);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_boardtype_test(uint32_t boardid)
{
    char *boardtype = NULL;
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    int8_t len = 0;

    boardtype = "C6482_BBp.00.01";
    len = strlen(boardtype);

    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_BOARD_TYPE);
    sbuf->send.datalen = htonl(len);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, boardtype, len);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_productsn_test(uint32_t boardid)
{
    u8 productsn[32] = {0x01,0x02,0x03,0x04,0x05,0x06};
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_PRODUCT_SN);
    sbuf->send.datalen = htonl(32);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, productsn, 32);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_manufacturer_test(uint32_t boardid)
{
    char *manufacturer = "XINWEI";
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;
    int8_t len = strlen(manufacturer);

    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_MANUFACTURER);
    sbuf->send.datalen = htonl(len);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, manufacturer, len);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_productdate_test(uint32_t boardid)
{
    u8 productdate[4] = {0x03,0x22,0x20,0x16};
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_PRODUCT_DATE);
    sbuf->send.datalen = htonl(4);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, productdate, 4);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}

int8_t board_eeprom_set_tempthreshold_test(uint32_t boardid)
{
    s8 tempthreshold[2] = {85,-10};
    msg_sendbuf_t *sbuf = NULL;
    int8_t ret = BSP_ERROR;

    sbuf = bsp_get_sendbuf();
    if(sbuf==NULL)
    {
        printf("[%s]:error! sendbuf=%p!\r\n", __func__, sbuf);
        return BSP_ERROR;
    }
    sbuf->send.header = htonl(NET_MSG_HEADER);
    sbuf->send.cmd = htonl(CMD_EEPROM_SET_TEMP_THRESHOLD);
    sbuf->send.datalen = htonl(2);
    sbuf->timeout_ms = 5*1000;
    memcpy(sbuf->send.data, tempthreshold, 2);
    ret = msg_send(boardid, sbuf);
    if(ret!=BSP_OK || sbuf->ack.data[0]!=1)
    {
        ret = BSP_ERROR;
    }
    bsp_release_sendbuf(sbuf);
    return ret;
}
#endif

void bsp_open_aifloopback(u8 u8BbpSlot)
{
    sleep(1);
    //设置aif回环
    bsp_bbp_fpga_write(u8BbpSlot, 139, 0x4);
    bsp_delay_1ms(100);
    bsp_bbp_fpga_write(u8BbpSlot, 141, 0x4);
    bsp_delay_1ms(100);
    bsp_bbp_fpga_write(u8BbpSlot, 143, 0x4);
    //设置213寄存器
    bsp_delay_1ms(100);
    bsp_bbp_fpga_write(u8BbpSlot, 213, 1);
    bsp_delay_1ms(100);
    bsp_bbp_fpga_write(u8BbpSlot, 213, 0);
    sleep(3);
}
void bsp_close_aifloopback(u8 u8BbpSlot)
{
    bsp_bbp_fpga_write(u8BbpSlot, 139, 0x0);
    bsp_delay_1ms(100);
    bsp_bbp_fpga_write(u8BbpSlot, 141, 0x0);
    bsp_delay_1ms(100);
    bsp_bbp_fpga_write(u8BbpSlot, 143, 0x0);
    sleep(3);
}

#if 0
cmd_t cmd_set[]=
{
    {"bsp_mcta_start", bsp_mcta_start},
    {"bsp_bbp_dsp_close", bsp_bbp_dsp_close},
    {"bsp_bbp_dsp_reset", bsp_bbp_dsp_reset},
    {"bsp_bbp_cpld_read", bsp_bbp_cpld_read},
    {"bsp_bbp_cpld_write", bsp_bbp_cpld_write},
    {"bsp_bbp_fpga_read", bsp_bbp_fpga_read},
    {"bsp_bbp_fpga_write", bsp_bbp_fpga_write},
    {"bsp_bbp_fpga_load", bsp_bbp_fpga_load},
    {"bsp_bbp_mcu_update", bsp_bbp_mcu_update},
    {"bsp_bbp_cpld_update", bsp_bbp_cpld_update},
    {"bsp_get_boardtype", bsp_get_boardtype},
    {"bsp_get_pcb_version", bsp_get_pcb_version},
    {"bsp_get_slot", bsp_get_slot}
};

unsigned int CMD_SET_SIZE = sizeof(cmd_set)/sizeof(cmd_t);
#endif
