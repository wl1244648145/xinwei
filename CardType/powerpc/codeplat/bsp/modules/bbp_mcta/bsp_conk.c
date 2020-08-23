#include "bsp_types.h"
#include "bsp_conkers_ext.h"

//#define USE_HMI_INTERFACE
board_info boards[MAX_BOARDS_NUMBER] = {{0}};
static const unsigned char *mcu_firmware_name[10] = {
	[BOARD_TYPE_FAN] = "/mnt/btsa/fan_work.bin",
	[BOARD_TYPE_BBP] = "/mnt/btsa/bbp_work.bin",
	[BOARD_TYPE_PEU] = "/mnt/btsa/peu_work.bin",
	[BOARD_TYPE_FSA] = "/mnt/btsa/fsa_work.bin",
	[BOARD_TYPE_ES] = "/mnt/btsa/es_work.bin"
};

typedef UINT32 (*RECVMESSAGE_FUNCPTR)(UINT32 u32BoardId);
UINT32(*RecvFristMessageFromBbp)(UINT32 u32BoardId);
UINT32(*RecvFristMessageFromGes)(UINT32 u32BoardId);
UINT32(*RecvFristMessageFromFan)(UINT32 u32BoardId);

#define BOARD_BBP_FIRST_MESSAGE (0)
#define BOARD_GES_FIRST_MESSAGE (1)
#define BOARD_FAN_FIRST_MESSAGE (2)
void BspBoardFirstMessageCallBack(UINT32 dwIndex,RECVMESSAGE_FUNCPTR pCallBack)
{	
    if ((dwIndex <0) || (pCallBack == NULL))
		return;
    switch(dwIndex)
    {
        case BOARD_BBP_FIRST_MESSAGE:
			RecvFristMessageFromBbp = pCallBack;
			break;
		case BOARD_GES_FIRST_MESSAGE:
			RecvFristMessageFromGes = pCallBack;
			break;
        case BOARD_FAN_FIRST_MESSAGE:
			RecvFristMessageFromFan = pCallBack;
			break;
		default:
			break;
	}
}

int bsp_subboard_heart_bit(u16 boardid)
{
	CHECK_BOARDID(boardid);
	return boards[boardid].mcu_heart;
}
int8_t bsp_mcu_update(uint16_t boardid)
{
	CHECK_BOARDID(boardid);
	bsp_hmi_mcu_update(boards[boardid].type, boardid);
}
int bsp_mcu_need_update(unsigned int boardtype, char *ver){
	const char *filename = NULL;	
	char buf[30] = "";
	FILE *fp = NULL;
	if(ver==NULL){
		printf("[%s]:ver==NULL\r\n", __func__);
		return BSP_OK;
	}
	if(boardtype > 7){
		printf("[%s]:unknow boardtype=%d\r\n", __func__, boardtype);
		return BSP_OK;
	}
	switch(boardtype)
	{
		case BOARD_TYPE_BBP:
			sprintf(buf, "Bbp_Work_");
			break;
		case BOARD_TYPE_FSA:
			sprintf(buf, "Fsa_Work_");	
			break;
              case BOARD_TYPE_ES:
                     sprintf(buf, "Es_Work_");
                     break;
		case BOARD_TYPE_FAN:
			sprintf(buf, "Fan_Work_");
			break;
		case BOARD_TYPE_PEU:
			sprintf(buf, "Peu_Work_");			
			break;
		default:
			break;
	}
	filename = mcu_firmware_name[boardtype];
	fp = fopen(filename, "rb");
	if(fp == NULL){		
		printf("[%s]:openfile Failed!(%s)\r\n", __func__,filename);
		return BSP_ERROR;
	}
	fseek(fp, 4, SEEK_SET);
       if(BOARD_TYPE_ES == boardtype)
       {
            fread(buf+8, 15, 1, fp);
       }
       else
       {
	     fread(buf+9, 15, 1, fp);
       }
	fclose(fp);
	printf("[%s]:board_ver:*%s*, file_ver:*%s*\r\n", __func__, ver, buf);
	if(strncmp(ver, buf, 9+15)==0){
		return BSP_ERROR;
	}
	return BSP_OK;
}
float bsp_show_boards_temperature(u16 boardid)
{
    CHECK_BOARDID(boardid);
    printf("slot[%d] temperature = %f \r\n", boardid, boards[boardid].temperature);
    return boards[boardid].temperature;   
}

unsigned int bsp_recv_firstmsg(u16 boardid, u8 board_type)
{
    CHECK_BOARDID(boardid);
    memset(&boards[boardid], 0, sizeof(board_info));
    boards[boardid].type = board_type;
    boards[boardid].mcu_status = MCU_STATUS_RECV_FIRSTMSG;
    if(BOARD_TYPE_BBP == board_type)
    {
        boards[boardid].cpld_updated = 1;
    }
    if(RecvFristMessageFromBbp !=NULL)
    {
        RecvFristMessageFromBbp(boardid);
    }
    return BSP_OK;
}
void bsp_subboard_recv_heartBt(u16 boardid, u8 board_type)
{
	CHECK_BOARDID(boardid);	
	boards[boardid].type = board_type;
       #if 0
	if((boards[boardid].mcu_status & MCU_STATUS_RECV_FIRSTMSG)==0)
	{
		bsp_reset_subboard(boardid);
		return;
	}
       #endif
	boards[boardid].mcu_heart++;
	if((board_type==BOARD_TYPE_BBP) || (board_type==BOARD_TYPE_ES))
	{
		if(boards[boardid].fpga_status==DSP_FPGA_STATE_NOLOAD)
		{
			boards[boardid].dsp_isload = DSP_FPGA_STATE_NOLOAD;
			bsp_bbp_fpga_load(boardid);		
		}
		else
		{
                     if(board_type==BOARD_TYPE_BBP)
                     {
                         if(boards[boardid].dsp_isload == DSP_FPGA_STATE_NOLOAD)
    			    {
                            if(bsp_bbp_load_dsp(boardid)==BSP_OK)
                            {
                                boards[boardid].dsp_isload = DSP_FPGA_STATE_LOADED;
                            }
    			    }
                     }
		}
	}
	if(board_type==BOARD_TYPE_FSA)
	{
		if(boards[boardid].fpga_status == DSP_FPGA_STATE_NOLOAD)
		{
			bsp_fsa_fpga_load(FSA_FPGA_325T, boardid);
		}
		else
		{
			if((boards[boardid].fpga_status&0X10) != FSA_FPGA_160T_LOADED)
			{
				bsp_fsa_fpga_load(FSA_FPGA_160T, boardid);
			}
		}
	}

}
void bsp_show_boards_info(u16 boardid)
{
	int i = boardid;
	unsigned int temp = 0;
	CHECK_BOARDID(boardid);
	printf("board[%d]:\r\n", i);
	printf("   type:%d(0x%x)\r\n", boards[i].type,boards[i].type);
	temp = boards[i].mcu_status;
	printf("   mcu_status:%d(0x%x)[%s %s %s]\r\n", temp, temp,
		(temp&MCU_STATUS_RESET_ACKED)?"":"No reset_ack", (temp&MCU_STATUS_RECV_FIRSTMSG)?"":"No first_msg", (temp&0x4)?"bbp_work":"bbp_boot");
       printf("   arm_version:%s\r\n", boards[boardid].arm_version);
       if(boards[boardid].type == BOARD_TYPE_BBP)
       {
              printf("   fpga_status:%d(0x%x)[0:NoLoad 1:Loading  2:Loaded]\r\n", boards[i].fpga_status, boards[i].fpga_status);
        	printf("   dsp_isload:%d(0x%x)[0:NoLoad 1:Loading  2:Loaded]\r\n", boards[i].dsp_isload, boards[i].dsp_isload);
              printf("   dsp_isready:%d(0x%x)[1:dsp1 2:dsp2 4:dsp3 8:dsp4]\r\n", boards[i].dsp_isready, boards[i].dsp_isready);
              printf("   srio port :0x%lx,0x%lx,0x%lx,0x%lx,0x%lx\r\n",
        		boards[boardid].srio_status[0],boards[boardid].srio_status[1],boards[boardid].srio_status[2],
        		boards[boardid].srio_status[3],boards[boardid].srio_status[4]);
       }
       if(boards[boardid].type == BOARD_TYPE_FSA)
       {            
            printf("   fpga_status:%d(0x%x)[0:NoLoad 0x10:160T Loaded  0x20:325T Loaded]\r\n", boards[i].fpga_status, boards[i].fpga_status);            
            printf("   srio port :0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx,0x%lx\r\n",
              boards[boardid].srio_status[0],boards[boardid].srio_status[1],boards[boardid].srio_status[2],
              boards[boardid].srio_status[3],boards[boardid].srio_status[4],boards[boardid].srio_status[5],
              boards[boardid].srio_status[6],boards[boardid].srio_status[7],boards[boardid].srio_status[8]);
       }
       if(boards[boardid].type == BOARD_TYPE_ES)
       {            
            printf("   fpga_status:%d(0x%x)[0:NoLoad 1:Loading  2:Loaded]\r\n", boards[i].fpga_status, boards[i].fpga_status);            
       }
}

unsigned int bsp_subboard_srio_status(u16 boardid, u8 portid)
{
	CHECK_BOARDID(boardid);
	if(portid > 8)
	{
		printf("[%s]:error portid:%d\r\n", __func__, portid);
		return 0;
	}
	if(bsp_get_srio_info(boardid)!=BSP_OK)
	{
		return 0;
	}
	return boards[boardid].srio_status[portid];
}
int bsp_reset_subboard(unsigned int boardid)
{
	u8 u8boardtype = 0;
	CHECK_BOARDID(boardid);
	u8boardtype = boards[boardid].type;
	memset(&boards[boardid], 0, sizeof(board_info));

	return bsp_hmi_reset_subboard(boardid);
}

s16 bbp_fpga_isload(u16 u16BordId)
{
	CHECK_BBP_BOARDID(u16BordId);
	return (boards[u16BordId].fpga_status==DSP_FPGA_STATE_LOADED)?BSP_OK:BSP_ERROR;
}
	
s16 bbp_fpga_read_reg(u16 u16BordId, u16 u16Reg_offset, u16 *u16Dat)
{
	CHECK_BBP_BOARDID(u16BordId);
	if(u16Dat==NULL)
	{
		printf("[%s]:error val=NULL!\r\n");
		return BSP_ERROR;
	}
#if defined(USE_HMI_INTERFACE)

#else
	return bsp_bbp_fpga_read(u16BordId, u16Reg_offset,u16Dat);
#endif
}
s16 bbp_fpga_write_reg(u16 u16BordId, u16 u16Reg_offset,u16 u16Dat)
{
	int ret = BSP_OK;
	CHECK_BBP_BOARDID(u16BordId);
#if defined(USE_HMI_INTERFACE)

#else
	ret = bsp_bbp_fpga_write(u16BordId, u16Reg_offset,u16Dat);
#endif
	return ret;
}
s16 bbp_cpld_read_reg(u16 u16BordId, u8 u8Reg_offset, u8 *u8Dat)
{
	CHECK_BBP_BOARDID(u16BordId);
	if(u8Dat==NULL)
	{
		printf("[%s]:error val=NULL!\r\n");
		return BSP_ERROR;
	}
#if defined(USE_HMI_INTERFACE)

#else
	return bsp_bbp_cpld_read(u16BordId, u8Reg_offset, u8Dat);
#endif
}
s16 bbp_cpld_write_reg(u16 u16BordId, u8 u8Reg_offset, u8 u8Dat)
{
	int ret = BSP_OK;
	CHECK_BBP_BOARDID(u16BordId);
#if defined(USE_HMI_INTERFACE)

#else
	ret = bsp_bbp_cpld_write(u16BordId, u8Reg_offset,u8Dat);
#endif
	return ret;
}

int board_is_started(u16 boardid)
{
	CHECK_BOARDID(boardid);
	if((boards[boardid].mcu_status & MCU_STATUS_RECV_FIRSTMSG)==0)
		return BSP_ERROR;
	return BSP_OK;
}


