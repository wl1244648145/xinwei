#ifndef __BSP_CONKERS_EXT_H__
#define __BSP_CONKERS_EXT_H__

//板卡类型
#define BOARD_TYPE_FAN    			(0x1)
#define BOARD_TYPE_BBP    			(0x2)
#define BOARD_TYPE_PEU    			(0x3)
#define BOARD_TYPE_MCT    			(0x4)
#define BOARD_TYPE_GES    			(0x5)
#define BOARD_TYPE_FSA    			(0x5)
#define BOARD_TYPE_ES   			(0x7)

//槽位号
#define IPMB_SLOT0       			(0)
#define IPMB_SLOT1       			(1)
#define IPMB_SLOT2       			(2)
#define IPMB_SLOT3       			(3)
#define IPMB_SLOT4       			(4)
#define IPMB_SLOT5       			(5)
#define IPMB_SLOT6       			(6)
#define IPMB_SLOT7       			(7)
#define IPMB_SLOT8       			(8)
#define IPMB_SLOT9       			(9)
#define IPMB_SLOT10      			(10)

//bit7~4:DSP[3:0]  bit3:FPGA  bit2:MCU bit[1:0]:no use
#define BOARD_READY_MASK_DSP   	0xF0
#define BOARD_READY_MASK_FPGA 	0x08
#define BOARD_READY_MASK_MCU   	0x04
#define BOARD_READY_MASK_GES   	0x02
#define BOARD_READY_MASK_FAN   	0x01
#define MCU_STATUS_RESET_ACKED	0x10
#define MCU_STATUS_RECV_FIRSTMSG	0x20

#define DSP_FPGA_STATE_NOLOAD   	0
#define DSP_FPGA_STATE_LOADING	1
#define DSP_FPGA_STATE_LOADED	       2

#define BBP_FPGA             0
#define FSA_FPGA_160T   1
#define FSA_FPGA_325T   2
#define ES_FPGA               3

#define FSA_FPGA_160T_LOADED          0x10
#define FSA_FPGA_325T_LOADED          0x20

#define MAX_BOARDS_NUMBER            11
#define CHECK_BOARDID(boardid)		if((boardid) < 0 || (boardid) > (MAX_BOARDS_NUMBER-1))\
{\
	printf("[%s] erro boardid:%d\r\n", __func__, boardid);\
	return BSP_ERROR;\
}
#define CHECK_BBP_BOARDID(boardid)		if((boardid) < 2 || (boardid) > (7))\
{\
	printf("[%s] erro BBP boardid:%d\r\n", __func__, boardid);\
	return BSP_ERROR;\
}
typedef struct st_board_info{
    unsigned char type;				// 板卡类型
    unsigned char mcu_alive;		// 板卡在位状态
    unsigned char mcu_status;		// ARM状态： 0x10: reset ack 0x20: first msg 0x*H:boot状态
    unsigned char mcu_heart;		// MCU心跳
    float	temperature;			// 单板温度
    unsigned char fpga_status;   	// FPGA状态：0:未加载  1:加载中  2:已经加载
    unsigned char dsp_isload;   	// DSP加载状态：0:未加载  1:加载中  2:已经加载
    unsigned char dsp_isready;
    unsigned char cpld_updated;       //基带板cpld升级完成标志
    unsigned int srio_status[9];	      // SRIO Switch端口状态
    char arm_version[64];  
}board_info;

extern board_info boards[MAX_BOARDS_NUMBER];
int bsp_reset_subboard(unsigned int boardid);
unsigned int bsp_subboard_srio_status(u16 boardid, u8 portid);
s16 bbp_fpga_read_reg(u16 u16BordId, u16 u16Reg_offset, u16 *u16Dat);
s16 bbp_fpga_write_reg(u16 u16BordId, u16 u16Reg_offset,u16 u16Dat);
s16 bbp_cpld_read_reg(u16 u16BordId, u8 u8Reg_offset, u8 *u8Dat);
s16 bbp_cpld_write_reg(u16 u16BordId, u8 u8Reg_offset, u8 u8Dat);

#endif //__BSP_CONKERS_EXT_H__

