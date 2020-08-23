#ifndef __BSP_BOARD_TEST_EXT_H__
#define __BSP_BOARD_TEST_EXT_H__

#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BOARD_TEST_PORT		9000
#define BOARD_TEST_HEADER	0xAABBBEEF
#define BOARD_TEST_END		0xAA5555AA

#define CMD_TESTALL			0xAAAA
#define CMD_TESTDANBAN	0xBBBB
#define CMD_STOP_TESTALL	0xCCCC

#ifdef CS
#define PRODUCT_TYPE		0x1  //²úÆ·ÀàÐÍ
#endif
//#ifdef EBBU
#define PRODUCT_TYPE		0x2  //À©Õ¹Ê½²úÆ·ÀàÐÍ
//#endif

#define BOARD_TYPE_MCT    (0x4)
#define BOARD_TYPE_BBP    (0x2)
#define BOARD_TYPE_GES    (0x5)
#define BOARD_TYPE_FSA    (0x5)
#define BOARD_TYPE_FAN    (0x1)
#define BOARD_TYPE_PEU    (0x3)

#define BOARD_TYPE_RRU   (0xa)  //rrutest add

#define TEST_CMD_MSG				0xFEFE

/*******************½»»»°å²âÊÔÃüÁî×Ö**************************/
#define TEST_GES_RESET				0x0000
#define TEST_GES_GESWITCH			0x0001
#define TEST_GES_WORKMODE			0x0002
#define TEST_GES_TEMPERATURE		0x0003
#define TEST_GES_EEPROM		       0x0004
#define TEST_GES_MCU_UPDATE		0x0005

/*******************·çÉÈ°å²âÊÔÃüÁî×Ö*************************/
#define TEST_FAN_BOOT				0x0100
#define TEST_FAN_SPEED_SET			0x0101
#define TEST_FAN_SPEED_GET              0x0102
#define TEST_FAN_UPDATE			0x0103
#define TEST_FAN_EEPROM			0x0104
#define TEST_FAN_VERSION                  0x0105
#define TEST_FAN_TEST				0x0106
/*******************¼à¿Ø°å²âÊÔÃüÁî×Ö*************************/
#define TEST_PEU_BOOT				0x500
#define TEST_PEU_POWER_DOWN		0x501
#define TEST_PEU_DRYIN                      0x502
#define TEST_PEU_TEMPERATURE		0x503
#define TEST_PEU_VERSION			0x504
#define TEST_PEU_UPDATE                    0x505
#define TEST_PEU_RS485                      0x506
/*******************»ù´ø°å²âÊÔÃüÁî×Ö**************************/
#define TEST_BBP_BOOT				0x0200
#define TEST_BBP_FPGA_LOAD			0x0201
#define TEST_BBP_DSP_LOAD			0x0202
#define TEST_BBP_MCU_UPDATE		0x0203
#define TEST_BBP_CPLD_UPDATE		0x0204
#define TEST_BBP_ETHSW				0x0205
#define TEST_BBP_SRIOSWITCH		0x0206
#define TEST_BBP_VER				0x0207
#define TEST_BBP_CPLD				0x0208
#define TEST_BBP_SDRAM				0x0209
#define TEST_BBP_TEMPERATURE		0x020A
#define TEST_BBP_POWER				0x020B
#define TEST_BBP_EEPROM			0x020C
#define TEST_BBP_IR_SYNC                   0x020D
#define TEST_BBP_SFP                           0x020E

#define TEST_BBP_DSP_CPU                  0xD303
#define TEST_BBP_DSP_DDR                 0xD601
#define TEST_BBP_DSP_VERION            0xD602
#define TEST_BBP_SRIO_DATA			0xD401
#define TEST_BBP_SRIO_EFFI			0xD402
#define TEST_BBP_AIF				0xD501
#define TEST_BBP_UP				       0xD502
#define TEST_BBP_DOWN				0xD503
/*******************½»»»°å²âÊÔÃüÁî×Ö**************************/
#define TEST_FSA_ETHSW                      0x0600
#define TEST_FSA_PLL_CFG                    0x0601
#define TEST_FSA_SRIOSWITCH             0x0602
#define TEST_FSA_FPGA_160T                0x0603
#define TEST_FSA_FPGA_325T                0x0604
#define TEST_FSA_SDRAM                       0x0605
#define TEST_FSA_MCU_UPDATE             0x0606
#define TEST_FSA_TEMPERATURE		  0x0607
#define TEST_FSA_POWER				  0x0608
#define TEST_FSA_EEPROM			  0x0609
#define TEST_FSA_PHY                            0x060A
#define TEST_FSA_SFP                             0x060B
#define TEST_FSA_BOOT				  0x060C
#define TEST_FSA_IR_SYNC                     0x060D
#define TEST_FSA_FPGA_325T_DDR        0x060E
#define TEST_FSA_10G_IR_SYNC             0x060F

//#define TEST_FSA_DSP_SRIO_DATA         0xD403
#define TEST_FSA_FPGA_160T_SRIO        0x0611
#define TEST_FSA_VER				   0x0612
/*******************Í¬²½°å²âÊÔÃüÁî×Ö**************************/
#define TEST_ES_ETHSW                          0x0700
#define TEST_ES_PLL_CFG                        0x0701
#define TEST_ES_FPGA                             0x0702
#define TEST_ES_SDRAM                          0x0703
#define TEST_ES_MCU_UPDATE                0x0704
#define TEST_ES_TEMPERATURE		   0x0705
#define TEST_ES_VER				          0x0706
#define TEST_ES_EEPROM			          0x0707
#define TEST_ES_SFP                                0x0708
#define TEST_ES_BOOT                             0x0709
#define TEST_ES_COPPER_LINK                0x070A
#define TEST_ES_FIBBER_LINK                 0x070B
/*******************Ö÷¿Ø°å²âÊÔÃüÁî×Ö*************************/
#define TEST_MCT_CPLD_LOAD		0x0300
#define TEST_MCT_CPLD_READ		0x0301
#define TEST_MCT_GESSWITCH		0x0302
#define TEST_MCT_PPCDDR			0x0303
#define TEST_MCT_NANDFLASH		0x0304
#define TEST_MCT_NORFLASH			0x0305
#define TEST_MCT_USB			       0x0306
#define TEST_MCT_SFP			       0x0307
#define TEST_MCT_TEMPERATURE		0x0308
#define TEST_MCT_POWER			       0x0309
#define TEST_MCT_GPS			       0x030A
#define TEST_MCT_EEPROM			0x030B
#define TEST_MCT_PHY			       0x030C
#define TEST_MCT_VER			       0x030D
#define TEST_MCT_EXTSYNC_PP1S        0x030E
#define TEST_MCT_EXTSYNC_TOD          0x030F
/******************RRU ²âÊÔÃüÁî×Ö************************/
#define TEST_RRU_VERQ				0xA001
#define TEST_RRU_VERD				0xA002
#define TEST_RRU_VERA				0xA003
#define TEST_RRU_ANT				0xA004
#define TEST_RRU_RF					0xA005
#define TEST_RRU_RUN				0xA006
#define TEST_RRU_SFP			       0xA007
#define TEST_RRU_TIME			       0xA008
#define TEST_RRU_ALARM				0xA009
#define TEST_RRU_PARA				0xA00A
#define TEST_RRU_CAL			       0xA00B
#define TEST_RRU_CELL			       0xA00C
#define TEST_RRU_DELAY				0xA00D
#define TEST_RRU_REBOOT		       0xA00E
#define TEST_RRU_SERIAL		       	0xA00F
#define TEST_RRU_HARDWARE		       0xA010


#define TEST_HEARTBEAT_PC			0xAAAA

/*******************°å¼ä²âÊÔÃüÁî×Ö**************************/
#define TEST_LTE_LINK			        0x0400
#define TEST_MCT_BBP_HMI		        0x0401
#define TEST_MCT_FAN_HMI			 0x0402
#define TEST_MCT_PEU_HMI			 0x0403
#define TEST_AFC			                      0x0404
#define TEST_BOARD_READY	               0x040B
#define TEST_AFC_SYNC		               0x040A
//#define TEST_REBOOT_BOARDS		 0x040B
#define TEST_IR 			                      0xD405
#define TEST_SET_ETH3_ADDR			 0x0405
#define TEST_BOARDS_SRIO		        0xD403
#define TEST_SYNC_START		        0xD40E
#define TEST_SYNC_STOP		               0xD40F
/*********************************************************/

/*******************EEPROM²ÎÊýÅäÖÃ/¶ÁÈ¡ÃüÁî×Ö**************************/
#define TEST_SET_CRC			        0xE001
#define TEST_SET_DEVICE_ID		        0xE002
#define TEST_SET_BOARD_TYPE		 0xE003
#define TEST_SET_MAC_ADDR1		 0xE004
#define TEST_SET_MAC_ADDR2		 0xE005
#define TEST_SET_PRODUCT_SN		 0xE006
#define TEST_SET_MANUFACTURE		 0xE007
#define TEST_SET_PRODUCT_DATE		 0xE008
#define TEST_SET_PRODUCT_SN		 0xE006
#define TEST_SET_MANUFACTURE		 0xE007
#define TEST_SET_PRODUCT_DATE		 0xE008
#define TEST_SET_SATELLITE_RECEIVER	0xE009
#define TEST_SET_FAN_INIT_SPEED		0xE00A
#define TEST_SET_TEMP_THRESHOLD		0xE00B

#define TEST_GET_CRC			              0xE101
#define TEST_GET_DEVICE_ID		              0xE102
#define TEST_GET_BOARD_TYPE			0xE103
#define TEST_GET_MAC_ADDR1			0xE104
#define TEST_GET_MAC_ADDR2			0xE105
#define TEST_GET_PRODUCT_SN			0xE106
#define TEST_GET_MANUFACTURE		       0xE107
#define TEST_GET_PRODUCT_DATE		       0xE108
#define TEST_GET_PRODUCT_SN			0xE106
#define TEST_GET_MANUFACTURE		       0xE107
#define TEST_GET_PRODUCT_DATE		       0xE108
#define TEST_GET_SATELLITE_RECEIVER	0xE109
#define TEST_GET_FAN_INIT_SPEED		0xE10A
#define TEST_GET_TEMP_THRESHOLD		0xE10B
/*********************************************************/
//ppc ddr info
struct meminfo_t{
	unsigned int addr;
	unsigned int length;
};

#define PACKET_DATA_SIZE 	1024
typedef struct
{
    uint32_t header;
    uint8_t  product;
    uint8_t  board;
    uint16_t cmd;
    uint32_t totaltimes;
    uint32_t failtimes;
    uint32_t successtimes;
    uint32_t datalen;
    union{
    	uint8_t st_data[PACKET_DATA_SIZE];
    	struct meminfo_t st_mem;
    }u_data;
}test_pkg_type_t;

typedef struct{	
    int index;
    struct sockaddr_in addr;
    test_pkg_type_t pkg;
}cmd_t;

#define pkg_header			pkg.header
#define pkg_product			pkg.product
#define pkg_board			pkg.board
#define pkg_cmd				pkg.cmd
#define pkg_totaltimes		pkg.totaltimes
#define pkg_failtimes 		       pkg.failtimes
#define pkg_successtimes 	       pkg.successtimes
#define pkg_datalen 		       pkg.datalen
#define pkg_data 			       pkg.u_data.st_data
#define pkg_mem 			pkg.u_data.st_mem

#define PACKET_HEADER_LEN(pkg) 	(sizeof(pkg)-sizeof(pkg.u_data))

int send_result(cmd_t cmd);
void init_boardtest(void);

#if 0
#define send_msg(cmd, fmt...) printf("[BoardTest]:"fmt)
#else
#define send_msg(cmd, fmt...) \
{\
	cmd_t pcmd = cmd;\
	uint32_t *pend = NULL;\
	time_t t;\
	struct tm *p;\
	pcmd.pkg_header = htonl(BOARD_TEST_HEADER);\
	pcmd.pkg_cmd = htons(TEST_CMD_MSG);\
	pcmd.pkg_totaltimes = 1;\
	pcmd.pkg_failtimes = 0;\
	pcmd.pkg_successtimes = 1;\
	time(&t);\
	p = localtime(&t);\
	printf("\r[%d/%02d/%02d %02d:%02d:%02d]: ", p->tm_year+1900,p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);\
	sprintf((char *)pcmd.pkg_data, "[boardtest]:"fmt);\
	printf((char *)pcmd.pkg_data);\
	pcmd.pkg_datalen = htonl(strlen((char *)pcmd.pkg_data)+1);\
	pend = pcmd.pkg_data + htonl(pcmd.pkg_datalen);\
	*pend = htonl(BOARD_TEST_END);\
	pcmd.addr.sin_port = htons(BOARD_TEST_PORT);\
	if((boardtest_fd>0) && (pcmd.addr.sin_addr.s_addr != 0))\
	sendto(boardtest_fd, &pcmd.pkg, PACKET_HEADER_LEN(pcmd.pkg)+htonl(pcmd.pkg_datalen)+4, 0, (struct sockaddr*)&(pcmd.addr), sizeof(pcmd.addr));\
}
#endif

#endif //__BSP_BOARD_TEST_EXT_H__

