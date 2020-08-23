#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#include "bsp_types.h"
#include "rrutest.h"
#include "bsp_conkers_ext.h"


#define RRU_TEST_PORT	4445
#define RRU_TEST_LOCAL_PORT	4444

#define FPGA_STATUS (((boards[2].fpga_status== DSP_FPGA_STATE_LOADED)||\
					    (boards[3].fpga_status== DSP_FPGA_STATE_LOADED)||\
					    (boards[4].fpga_status== DSP_FPGA_STATE_LOADED)||\
					    (boards[5].fpga_status== DSP_FPGA_STATE_LOADED)||\
					    (boards[6].fpga_status== DSP_FPGA_STATE_LOADED)||\
					    (boards[7].fpga_status== DSP_FPGA_STATE_LOADED))? 1 : 0)
					    
static UINT32 len_rrutest_header = sizeof(rru_test_msg) - 1024;

UINT16 g_rrutest_heartbeat_count = 1;
u8 g_heartbeat_to_pc = 0;
int g_alive_pc = -1;
cmd_t cmd_for_rrumsg;
static int rrutest_fd_recv = -1;
static int rrutest_fd_send = -1;
struct sockaddr_in rru_test_addr;
struct _RRUinfoT rru_info;
static u8 g_rruID;
pthread_mutex_t msg_send_lock = PTHREAD_MUTEX_INITIALIZER;

static sem_t sem_rru_cell_config;
static sem_t sem_rru_sw_query;
static sem_t sem_rru_hw_query;
static sem_t sem_rru_sw_download;
static sem_t sem_rru_sw_active;
static sem_t sem_rru_rf_query;
static sem_t sem_rru_rru_query;
static sem_t sem_rru_fiber_query;
static sem_t sem_rru_power_cal;
static sem_t sem_rru_alm_query;
static sem_t sem_rru_hw_para;
static sem_t sem_rru_reset;
static sem_t sem_rru_fiberDelay_cfg;
static sem_t sem_rru_parameter_query;
static sem_t sem_rru_time_cfg;
static sem_t sem_rru_mask_cfg;

extern int boardtest_fd;
extern pthread_mutex_t bbp_lock;
extern pthread_mutex_t rru_lock;

extern char bsp_bbp_fpga_write(u32 boardid, u8 reg, u16 val);
extern char bsp_bbp_fpga_read(u32 boardid, u8 reg, u16 *val);
extern void bsp_mutex_lock(pthread_mutex_t * lock);
extern void bsp_mutex_unlock(pthread_mutex_t * lock);

static char RRU_PKG_PATH[] = "/mnt/btsa/RRU";
void print_array(int len, u8 *array){
	int i = 0;
	while (i<len){
		printf("%02X ", array[i]);
			i++;
			}
	printf("\r\n");
	}

void word32tobyte(UINT32 long_var,  u8 *byte_array){
	memset(byte_array, 0, 4);
	byte_array[3] = long_var & 0xff;
	byte_array[2] = (long_var >> 8) & 0xff;
	byte_array[1] = (long_var >> 16) & 0xff;
	byte_array[0] = (long_var >> 24) & 0xff;
	}

void word16tobyte(UINT16 long_var,  u8 *byte_array){
	memset(byte_array, 0, 2);
	byte_array[1] = long_var & 0xff;
	byte_array[0] = (long_var >> 8) & 0xff;
	}

UINT32 bytetoword32(u8 *byte_array){
	UINT32 var = 0;
	var = byte_array[3]|byte_array[2]<<8|byte_array[1]<<16|byte_array[0]<<24;
	return var;
	
}

u16 bytetoword16(u8 *byte_array){
	u16 var = 0;
	var = byte_array[1]|byte_array[0]<<8;
	return var;
}

s16 sbytetoword16(s8 *byte_array){
	s16 var = 0;
	var = byte_array[1]|byte_array[0]<<8;
	return var;
}

int array_cmp(u32 *src, u8 *dst, u8 len){
	int i;
	for(i=0;i<len;i++){
	if((src[i] & 0xff)!= dst[i]){
	printf("wrong compare data index is %d, src is %02x, dst is %02x\r\n", i, src[i]&0xff, dst[i]);
	break;}
	}
	//printf("compare len is %d\r\n", i);
      return i;
}

struct timespec timeout_calc(int timeout){
	struct timespec ts;
	if(clock_gettime(CLOCK_REALTIME, &ts) == -1)
		perror("timeout_calc wrong!");
	ts.tv_sec += timeout;
	return ts;
	}


int rru_version_get(u32 *pkg_ver){
	char raw_ver[11] = {0};
	int ret;
	FILE *fp = fopen(RRU_PKG_PATH, "r");
	if(fp == NULL){
	printf("/mnt/btsa/RRU is not exist!\r\n");
	return -1;
	}
	fread(raw_ver, 1, 11, fp);
	printf("RRU version is %c%c.%c%c.%c%c.%c%c\r\n", raw_ver[0], raw_ver[1], raw_ver[3], raw_ver[4], raw_ver[6], raw_ver[7], raw_ver[9], raw_ver[10]);
	ret = sscanf(raw_ver, "%x.%x.%x.%x", &pkg_ver[0], &pkg_ver[1], &pkg_ver[2], &pkg_ver[3]);
	if(ret!=4){
	printf("RRU version read is wrong\r\n");
	return -1;
	}
	else{
	return 0;}
	fclose(fp);
}


void handle_channel_setup(rru_test_msg *msg){
	memset(&rru_info, 0, sizeof(RruInformationT));
	u8 bbuip[4] = {0xa, 0x0, 0x0, 0x1};
	chnSetupT *chnSetupMsg;
	chnSetupCfgReqT *chnSetupCfgReqMsg;
	u32 pkg_ver[4] = {0};
	u8 cmp_value;
	u8 rruver_cmp_result;
	
	chnSetupMsg = (chnSetupT *)msg->data;
	g_rruID = msg->rruID;
	if(rru_version_get(pkg_ver)<0){
		rruver_cmp_result = 0;
	}
	else{
		cmp_value = array_cmp(pkg_ver, chnSetupMsg->au8pkg_ver, 4);
		if(cmp_value == 4)
			rruver_cmp_result = 0;
		else
			rruver_cmp_result = 1;
	}
	printf("RRU[%d]channel setup reason is %d\r\n", msg->rruID, chnSetupMsg->u8ChnSetupReason);
	printf("RRU[%d]reset warn code is %d\r\n", msg->rruID, chnSetupMsg->u32Warncode);
	printf("RRU[%d]software package is %02X\r\n", msg->rruID, bytetoword32(chnSetupMsg->au8pkg_ver));
	printf("RRU[%d]FPGA version is %02X\r\n", msg->rruID, bytetoword32(chnSetupMsg->au8fpga_ver));
	printf("RRU[%d]antenna state is %02X\r\n",msg->rruID, (u16)(chnSetupMsg->u16AntNum));
	printf("RRU[%d]product type is %02X\r\n",  msg->rruID, (u16)(chnSetupMsg->u16ProductType));
	printf("RRU[%d]hardware master board version is %02X\r\n",  msg->rruID, chnSetupMsg->u32masterboard_ver);
	printf("RRU[%d]hardware slave board version is %02X\r\n",  msg->rruID, chnSetupMsg->u32slaveboard_ver);
	printf("RRU[%d]hardware master board pa version is %02X\r\n",  msg->rruID, chnSetupMsg->u32pa_masterboard_ver);
	printf("RRU[%d]hardware slave board pa version is %02X\r\n",  msg->rruID, chnSetupMsg->u32pa_slaveboard_ver);
	printf("RRU[%d]channel max tx power is %.1f(dBm)\r\n",  msg->rruID, (chnSetupMsg->u16tx_max_pow)*0.1);
	printf("RRU[%d]channel max rx power is %.1f(dBm)\r\n",  msg->rruID, (chnSetupMsg->u16rx_max_pow)*0.1);
	printf("RRU[%d]aiq_tx_nom is %d\r\n",  msg->rruID, chnSetupMsg->s16aiq_tx_nom);
	printf("RRU[%d]aiq_rx_nom is %d\r\n",  msg->rruID, chnSetupMsg->s16aiq_rx_nom);
	printf("RRU[%d]freq band is %d\r\n",  msg->rruID, chnSetupMsg->u16FreqBand);
	printf("RRU[%d]hardware master board sn is %s\r\n",  msg->rruID, chnSetupMsg->au8masterboard_sn);
	printf("RRU[%d]hardware slave board sn is %s\r\n",  msg->rruID, chnSetupMsg->au8slaveboard_sn);
	printf("RRU[%d]hardware master board pa sn is %s\r\n",  msg->rruID, chnSetupMsg->au8pa_masterboard_sn);
	printf("RRU[%d]hardware slave board pa sn is %s\r\n",  msg->rruID, chnSetupMsg->au8pa_slaveboard_sn);

	/*add rru_info struct*/
	rru_info.rruID = msg->rruID;
	rru_info.u8ChnSetupReason = chnSetupMsg->u8ChnSetupReason;
	rru_info.u32Warncode = chnSetupMsg->u32Warncode;
	memcpy(rru_info.au8pkg_ver, chnSetupMsg->au8pkg_ver, 4);
	memcpy(rru_info.au8app_ver, chnSetupMsg->au8app_ver, 4);
	memcpy(rru_info.au8fpga_ver, chnSetupMsg->au8fpga_ver, 4);
	rru_info.u32masterboard_ver = chnSetupMsg->u32masterboard_ver;
	rru_info.u32slaveboard_ver = chnSetupMsg->u32slaveboard_ver;
	rru_info.u32pa_masterboard_ver = chnSetupMsg->u32pa_masterboard_ver;
	rru_info.u32pa_slaveboard_ver= chnSetupMsg->u32pa_slaveboard_ver;
	memcpy(rru_info.au8masterboard_sn, chnSetupMsg->au8masterboard_sn, 32);
	memcpy(rru_info.au8slaveboard_sn, chnSetupMsg->au8slaveboard_sn, 32);
	memcpy(rru_info.au8pa_slaveboard_sn, chnSetupMsg->au8pa_slaveboard_sn, 32);
	memcpy(rru_info.au8pa_masterboard_sn, chnSetupMsg->au8pa_masterboard_sn, 32);
	rru_info.u16tx_max_pow= chnSetupMsg->u16tx_max_pow;
	rru_info.s16aiq_tx_nom= chnSetupMsg->s16aiq_tx_nom;
	rru_info.s16aiq_rx_nom= chnSetupMsg->s16aiq_rx_nom;
	rru_info.u16rx_max_pow= chnSetupMsg->u16rx_max_pow;
	rru_info.u16FreqBand= chnSetupMsg->u16FreqBand;
	rru_info.u16AntNum= chnSetupMsg->u16AntNum;
	rru_info.u16ProductType= chnSetupMsg->u16ProductType;

	word32tobyte(2, msg->type);
	word32tobyte(len_rrutest_header+sizeof(chnSetupCfgReqT), msg->len);	
	
	chnSetupCfgReqMsg = (chnSetupCfgReqT *)msg->data;
	memset(chnSetupCfgReqMsg, 0, sizeof(chnSetupCfgReqT));
	
	time_t t;
	struct tm *p;
	time(&t);
	p=localtime(&t);
	chnSetupCfgReqMsg->second = (u8)(p->tm_sec);
	chnSetupCfgReqMsg->minute = (u8)(p->tm_min);
	chnSetupCfgReqMsg->hour = (u8)(p->tm_hour);
	chnSetupCfgReqMsg->day = (u8)(p->tm_mday);
	chnSetupCfgReqMsg->month = (u8)(p->tm_mon+1);
	word16tobyte((u16)(p->tm_year+1900), chnSetupCfgReqMsg->year);

	chnSetupCfgReqMsg->workmode = 0;
	chnSetupCfgReqMsg->rrumode = 0;
	chnSetupCfgReqMsg->irmode = 1;
	
	memcpy(chnSetupCfgReqMsg->bbuip, bbuip, 4);
	strcpy(chnSetupCfgReqMsg->ftpusername, "root");
	strcpy(chnSetupCfgReqMsg->ftppassword, "12345678");
	strcpy(chnSetupCfgReqMsg->filename, "RRU");
	
	chnSetupCfgReqMsg->ver_comp_res= rruver_cmp_result;

	chnSetupCfgReqMsg->board_temp_threshold= 85;
	chnSetupCfgReqMsg->rfchn_temp_threshold= 100;
	chnSetupCfgReqMsg->vswr[0]= 0x0;
	chnSetupCfgReqMsg->vswr[1]= 0x3;
	int k = -1;
	pthread_mutex_lock(&msg_send_lock);
	k=sendto(rrutest_fd_send, msg, len_rrutest_header+sizeof(chnSetupCfgReqT), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	pthread_mutex_unlock(&msg_send_lock);
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
	else{
	printf("send to rru messtype: %d\r\n", bytetoword32(msg->type));
	}
	
}

void handle_chnSetupCfg_response(rru_test_msg *msg){
	u32 channel_setup_result;
	chnSetupCfgRspT *chnSetupCfgRspMsg;
	chnSetupCfgRspMsg= (chnSetupCfgRspT *)msg ->data;
	channel_setup_result = bytetoword32(chnSetupCfgRspMsg->allresults);
	printf("RRU[%d]通道建立请求结果[%02X]\r\n", msg->rruID, channel_setup_result);
}

void handle_alarm_report(rru_test_msg *msg){
	AlarmRptT *alarmReportMsg;
	alarmReportMsg = (AlarmRptT *)msg ->data;
	if(g_heartbeat_to_pc == 1)
		send_msg(cmd_for_rrumsg, "RRU[%d] ALARM: %s\r\n", msg->rruID, alarmReportMsg->info)
	else
		printf("RRU[%d] ALARM: %s\r\n", msg->rruID, alarmReportMsg->info);
}

void handle_SWVer_result(rru_test_msg *msg){
	SWQryRspT *SWQryRspMsg;
	SWQryRspMsg = (SWQryRspT *)msg->data;
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] SWVer info: RRU[%02X]、rruapp[%02X]、fpga[%02X]\r\n", \
			msg->rruID, bytetoword32(SWQryRspMsg->bigPkgVer), bytetoword32(SWQryRspMsg->appver), bytetoword32(SWQryRspMsg->fpgaver));
		send_msg(cmd_for_rrumsg, "RRU[%d] software version query result is %d\r\n", msg->rruID, bytetoword16(SWQryRspMsg->result));
	}
	else{
		printf("RRU[%d] SWVer info: RRU[%02X]、", msg->rruID, bytetoword32(SWQryRspMsg->bigPkgVer));
		printf("rruapp[%02X]、fpga[%02X]\r\n", bytetoword32(SWQryRspMsg->appver), bytetoword32(SWQryRspMsg->fpgaver));
		}
	memcpy(rru_info.au8pkg_ver, SWQryRspMsg->bigPkgVer, 4);
	memcpy(rru_info.au8app_ver, SWQryRspMsg->appver, 4);
	memcpy(rru_info.au8fpga_ver, SWQryRspMsg->fpgaver, 4);
	sem_post(&sem_rru_sw_query);
}

void handle_HWVer_result(rru_test_msg *msg){
	HWQryRspT *HWQryRspMsg;
	HWQryRspMsg = (HWQryRspT *)msg->data;

	UINT32 u32mainboardver = bytetoword32(HWQryRspMsg->mainboardver);
	UINT32 u32slaveboardver = bytetoword32(HWQryRspMsg->slaveboardver);
	UINT32 u32pamainboardver = bytetoword32(HWQryRspMsg->pamainboardver);
	UINT32 u32paslaveboardver = bytetoword32(HWQryRspMsg->paslaveboardver);
	
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] HWVer info: mboardver[%02X]、sboardver[%02X]、pamboardver[%02X]、pasboardver[%02X]\r\n", msg->rruID, u32mainboardver, u32slaveboardver, u32pamainboardver, u32paslaveboardver);
		send_msg(cmd_for_rrumsg, "RRU[%d] HWVer info: mboardsn[%s]、sboardsn[%s]、pamboardsn[%s]、pasboardsn[%s]\r\n", msg->rruID, HWQryRspMsg->mainboardsn, HWQryRspMsg->slaveboardsn, HWQryRspMsg->pamainboardsn, HWQryRspMsg->paslaveboardsn);
		send_msg(cmd_for_rrumsg, "RRU[%d] hardware version query result is %d\r\n", msg->rruID, bytetoword16(HWQryRspMsg->result));
		}
	else{
	printf("RRU[%d] HWVer info: mboardver[%02X]、", msg->rruID, bytetoword32(HWQryRspMsg->mainboardver));
	printf("sboardver[%02X]、pamboardver[%02X]、pasboardver[%02X]\r\n", bytetoword32(HWQryRspMsg->slaveboardver), bytetoword32(HWQryRspMsg->pamainboardver),bytetoword32(HWQryRspMsg->paslaveboardver));
	printf("RRU[%d] HWVer info: mboardsn[%s]、sboardsn[%s]、pamboardsn[%s]、pasboardsn[%s]\r\n", msg->rruID, HWQryRspMsg->mainboardsn, HWQryRspMsg->slaveboardsn, HWQryRspMsg->pamainboardsn, HWQryRspMsg->paslaveboardsn);
	}
	rru_info.u32masterboard_ver = u32mainboardver;
	rru_info.u32slaveboard_ver = u32slaveboardver;
	rru_info.u32pa_masterboard_ver = u32pamainboardver;
	rru_info.u32pa_slaveboard_ver = u32paslaveboardver;
	memcpy(rru_info.au8masterboard_sn, HWQryRspMsg->mainboardsn, 32);
	memcpy(rru_info.au8slaveboard_sn, HWQryRspMsg->slaveboardsn, 32);
	memcpy(rru_info.au8pa_masterboard_sn, HWQryRspMsg->pamainboardsn, 32);
	memcpy(rru_info.au8pa_slaveboard_sn, HWQryRspMsg->paslaveboardsn, 32);
	
	sem_post(&sem_rru_hw_query);
}

void handle_RFStatusQry_response(rru_test_msg *msg){
	int i;
	RFStateQryRspT *RFStateQryRspMsg;
	RFStateQryRspMsg = (RFStateQryRspT *)msg->data;
	u16 RF_status_result;
	u8 channel;
	channel = RFStateQryRspMsg->rfchn;
	RF_status_result = bytetoword16(RFStateQryRspMsg->result);

	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] RF status query result is %d\r\n", msg->rruID, RF_status_result);}
	else
		printf("RRU[%d] RF status query result is %d\r\n", msg->rruID, RF_status_result);
		
	if(RF_status_result == 0){
		if(g_alive_pc == 1){
			send_msg(cmd_for_rrumsg, "RRU[%d]rf query channel number is %d\r\n", msg->rruID, channel);}
		else
			printf("RRU[%d]rf query channel number is %d\r\n", msg->rruID, channel);

		if(channel == 8){
			u16 antmask;
			u8 ulantmask = 0;
			u8 dlantmask = 0;
			
			if(g_alive_pc == 1){
			for(i=0; i<8; i++){
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] dl antenna is %d\r\n", msg->rruID, i, RFStateQryRspMsg->rfchnstate[i].u8dl_antenna_state);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] ul antenna is %d\r\n", msg->rruID, i, RFStateQryRspMsg->rfchnstate[i].u8ul_antenna_state);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] channel temp is %d(℃)\r\n", msg->rruID, i, RFStateQryRspMsg->rfchnstate[i].s8chn_temp);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] tx power is %.1f(dBm)\r\n", msg->rruID, i, (RFStateQryRspMsg->rfchnstate[i].s16txpw)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] rx power is %.1f(dBm)\r\n", msg->rruID, i, (RFStateQryRspMsg->rfchnstate[i].s16rxpw)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] tx gain is %.1f(dB)\r\n", msg->rruID, i, (RFStateQryRspMsg->rfchnstate[i].u8txgain)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] rx gain is %.1f(dB)\r\n", msg->rruID, i, (RFStateQryRspMsg->rfchnstate[i].u8rxgain)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] vswr is %.1f\r\n", msg->rruID,  i, (RFStateQryRspMsg->rfchnstate[i].s16vswr)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] vswr calc result is %d\r\n", msg->rruID,  i, RFStateQryRspMsg->rfchnstate[i].u8ResultGetVswr);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] get rx power result is %d\r\n", msg->rruID,  i, RFStateQryRspMsg->rfchnstate[i].u8ResultGetRxpw);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] get tx power result is %d\r\n", msg->rruID,  i, RFStateQryRspMsg->rfchnstate[i].u8ResultGetTxpw);
			
			dlantmask |= (RFStateQryRspMsg->rfchnstate[i].u8dl_antenna_state<<i);
			ulantmask |= (RFStateQryRspMsg->rfchnstate[i].u8ul_antenna_state<<i);
			
			}
				}
			else{
				for(i=0; i<8; i++){
			printf("RRU[%d] CHN[%d] dl antenna is %d\r\n", msg->rruID, i, RFStateQryRspMsg->rfchnstate[i].u8dl_antenna_state);
			printf("RRU[%d] CHN[%d] ul antenna is %d\r\n", msg->rruID, i, RFStateQryRspMsg->rfchnstate[i].u8ul_antenna_state);
			printf("RRU[%d] CHN[%d] channel temp is %d(℃)\r\n", msg->rruID, i, RFStateQryRspMsg->rfchnstate[i].s8chn_temp);
			printf("RRU[%d] CHN[%d] tx power is %.1f(dBm)\r\n", msg->rruID, i, (RFStateQryRspMsg->rfchnstate[i].s16txpw)*0.1);
			printf("RRU[%d] CHN[%d] rx power is %.1f(dBm)\r\n", msg->rruID, i, (RFStateQryRspMsg->rfchnstate[i].s16rxpw)*0.1);
			printf("RRU[%d] CHN[%d] tx gain is %.1f(dB)\r\n", msg->rruID, i, (RFStateQryRspMsg->rfchnstate[i].u8txgain)*0.1);
			printf("RRU[%d] CHN[%d] rx gain is %.1f(dB)\r\n", msg->rruID, i, (RFStateQryRspMsg->rfchnstate[i].u8rxgain)*0.1);
			printf("RRU[%d] CHN[%d] vswr is %.1f\r\n", msg->rruID,  i, (RFStateQryRspMsg->rfchnstate[i].s16vswr)*0.1);
			printf("RRU[%d] CHN[%d] vswr calc result is %d\r\n", msg->rruID,  i, RFStateQryRspMsg->rfchnstate[i].u8ResultGetVswr);
			printf("RRU[%d] CHN[%d] get rx power result is %d\r\n", msg->rruID,  i, RFStateQryRspMsg->rfchnstate[i].u8ResultGetRxpw);
			printf("RRU[%d] CHN[%d] get tx power result is %d\r\n", msg->rruID,  i, RFStateQryRspMsg->rfchnstate[i].u8ResultGetTxpw);
			dlantmask |= (RFStateQryRspMsg->rfchnstate[i].u8dl_antenna_state<<i);
			ulantmask |= (RFStateQryRspMsg->rfchnstate[i].u8ul_antenna_state<<i);
			}
			} 
			antmask = ((dlantmask&0xf)<<12) | ((ulantmask&0xf)<<8) | ((dlantmask >> 4) & 0xf) | ((ulantmask>>4) &0xf);
			rru_info.u16AntNum = antmask;
			}
		else{
			if(g_alive_pc == 1){
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] dl antenna is %d\r\n", msg->rruID, channel, RFStateQryRspMsg->rfchnstate[channel].u8dl_antenna_state);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] ul antenna is %d\r\n", msg->rruID, channel, RFStateQryRspMsg->rfchnstate[channel].u8ul_antenna_state);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] channel temp is %d(℃)\r\n", msg->rruID, channel, RFStateQryRspMsg->rfchnstate[channel].s8chn_temp);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] tx power is %.1f(dBm)\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].s16txpw)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] rx power is %.1f(dBm)\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].s16rxpw)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] tx gain is %.1f(dB)\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].u8txgain)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] rx gain is %.1f(dB)\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].u8rxgain)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] vswr is %.1f\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].s16vswr)*0.1);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] vswr calc result is %d\r\n", msg->rruID,  channel, RFStateQryRspMsg->rfchnstate[channel].u8ResultGetVswr);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] get rx power result is %d\r\n", msg->rruID,  channel, RFStateQryRspMsg->rfchnstate[channel].u8ResultGetRxpw);
			send_msg(cmd_for_rrumsg, "RRU[%d] RF CHN[%d] get tx power result is %d\r\n", msg->rruID,  channel, RFStateQryRspMsg->rfchnstate[channel].u8ResultGetTxpw);
			}
			else{
			printf("RRU[%d] CHN[%d] dl antenna is %d\r\n", msg->rruID, channel, RFStateQryRspMsg->rfchnstate[channel].u8dl_antenna_state);
			printf("RRU[%d] CHN[%d] ul antenna is %d\r\n", msg->rruID, channel, RFStateQryRspMsg->rfchnstate[channel].u8ul_antenna_state);
			printf("RRU[%d] CHN[%d] channel temp is %d(℃)\r\n", msg->rruID, channel, RFStateQryRspMsg->rfchnstate[channel].s8chn_temp);
			printf("RRU[%d] CHN[%d] tx power is %.1f(dBm)\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].s16txpw)*0.1);
			printf("RRU[%d] CHN[%d] rx power is %.1f(dBm)\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].s16rxpw)*0.1);
			printf("RRU[%d] CHN[%d] tx gain is %.1f(dB)\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].u8txgain)*0.1);
			printf("RRU[%d] CHN[%d] rx gain is %.1f(dB)\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].u8rxgain)*0.1);
			printf("RRU[%d] CHN[%d] vswr is %.1f\r\n", msg->rruID, channel, (RFStateQryRspMsg->rfchnstate[channel].s16vswr)*0.1);
			printf("RRU[%d] CHN[%d] vswr calc result is %d\r\n", msg->rruID,  channel, RFStateQryRspMsg->rfchnstate[channel].u8ResultGetVswr);
			printf("RRU[%d] CHN[%d] get rx power result is %d\r\n", msg->rruID,  channel, RFStateQryRspMsg->rfchnstate[channel].u8ResultGetRxpw);
			printf("RRU[%d] CHN[%d] get tx power result is %d\r\n", msg->rruID,  channel, RFStateQryRspMsg->rfchnstate[channel].u8ResultGetTxpw);
			}
			}
			}
		sem_post(&sem_rru_rf_query);
	}


void handle_RRUStatusQry_response(rru_test_msg *msg){
	RruStateQryRspT *recvMsgFormRRU;
	recvMsgFormRRU = (RruStateQryRspT *)msg->data;
	u16 query_result;
	query_result = bytetoword16(recvMsgFormRRU->result);
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] RRU status query result is %d\r\n", msg->rruID, query_result);}
	else
		printf("RRU[%d] RRU status query result is %d\r\n", msg->rruID, query_result);
	if(query_result == 0){
		if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] rf lo freq is %.1f(MHz)\r\n", msg->rruID, (bytetoword32(recvMsgFormRRU->rflofreq))*0.1);
		send_msg(cmd_for_rrumsg, "RRU[%d] rf lo is %d\r\n", msg->rruID, recvMsgFormRRU->rflostate);
		send_msg(cmd_for_rrumsg, "RRU[%d] clock is %d\r\n", msg->rruID, recvMsgFormRRU->clkstate);
		send_msg(cmd_for_rrumsg, "RRU[%d] ir interface is %d\r\n", msg->rruID, recvMsgFormRRU->irifmode);
		//printf("RRU[%d] total time-slot is %d!, dl time-slot rate is %d\r\n", msg->rruID, recvMsgFormRRU->totalts, recvMsgFormRRU->dlts);
		send_msg(cmd_for_rrumsg, "RRU[%d] work mode is %d\r\n", msg->rruID, recvMsgFormRRU->workmode);
		send_msg(cmd_for_rrumsg, "RRU[%d] main board temp is %d(℃)\r\n", msg->rruID, recvMsgFormRRU->mainboard_temp);
		send_msg(cmd_for_rrumsg, "RRU[%d] slave board temp is %d(℃)\r\n", msg->rruID, recvMsgFormRRU->slaveboard_temp);
		send_msg(cmd_for_rrumsg, "RRU[%d] system time is %d-%d-%d_%d:%d:%d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->year), recvMsgFormRRU->mon, 
			recvMsgFormRRU->day, recvMsgFormRRU->hour, recvMsgFormRRU->min, recvMsgFormRRU->sec);
		send_msg(cmd_for_rrumsg, "RRU[%d] running time is %d(s)\r\n", msg->rruID, bytetoword32(recvMsgFormRRU->workingtime));
			}
		
		else{
		printf("RRU[%d] rf lo freq is %.1f(MHz)\r\n", msg->rruID, (bytetoword32(recvMsgFormRRU->rflofreq))*0.1);
		printf("RRU[%d] rf lo is %d\r\n", msg->rruID, recvMsgFormRRU->rflostate);
		printf("RRU[%d] clock is %d\r\n", msg->rruID, recvMsgFormRRU->clkstate);
		printf("RRU[%d] ir interface is %d\r\n", msg->rruID, recvMsgFormRRU->irifmode);
		//printf("RRU[%d] total time-slot is %d!, dl time-slot rate is %d\r\n", msg->rruID, recvMsgFormRRU->totalts, recvMsgFormRRU->dlts);
		printf("RRU[%d] work mode is %d\r\n", msg->rruID, recvMsgFormRRU->workmode);
		printf("RRU[%d] M/S temp is %d/%d(℃)\r\n", msg->rruID, recvMsgFormRRU->mainboard_temp, recvMsgFormRRU->slaveboard_temp);
		printf("RRU[%d] system time is %d-%d-%d %d:%d:%d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->year), recvMsgFormRRU->mon, 
			recvMsgFormRRU->day, recvMsgFormRRU->hour, recvMsgFormRRU->min, recvMsgFormRRU->sec);
		printf("RRU[%d] running time is %d(s)\r\n", msg->rruID, bytetoword32(recvMsgFormRRU->workingtime));
		}
		}
	sem_post(&sem_rru_rru_query);
}

void handle_FiberStatusQry_response(rru_test_msg *msg){
	FiberPortStateQryRspT *recvMsgFormRRU;
	recvMsgFormRRU = (FiberPortStateQryRspT *)msg->data;
	u16 query_result;
	u8 channel;
	u8 i;
	query_result = bytetoword16(recvMsgFormRRU->result);
	channel = recvMsgFormRRU->fiberport;
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber status query result is %d\r\n", msg->rruID, query_result);}
	else
		printf("RRU[%d] Fiber status query result is %d\r\n", msg->rruID, query_result);
	if(query_result == 0){
		if(g_alive_pc==1){
			send_msg(cmd_for_rrumsg, "RRU[%d]fiber query channel number is %d\r\n", msg->rruID, channel);}
		else
			printf("RRU[%d]fiber query channel number is %d\r\n", msg->rruID, channel);
		if(channel ==2){
			if(g_alive_pc == 1){
			for(i=0;i<channel;i++){
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] rx power is %.1f(uW)\r\n", msg->rruID, i, (bytetoword16(recvMsgFormRRU->fpstat[i].rxpw))*0.1);
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] tx power is %.1f(uW)\r\n", msg->rruID, i, (bytetoword16(recvMsgFormRRU->fpstat[i].txpw))*0.1);
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module ineffect is %d\r\n", msg->rruID, i, recvMsgFormRRU->fpstat[i].ineffect);
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module vendor is %s\r\n", msg->rruID, i, recvMsgFormRRU->fpstat[i].vendor);
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module transfer rate is %d(Mbit/s)\r\n", msg->rruID, i, bytetoword16((u8 *)(recvMsgFormRRU->fpstat[i].rate)));
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module temp is %d(℃)\r\n", msg->rruID, i, (s8)(recvMsgFormRRU->fpstat[i].temperature));
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module voltage is %d(mV)\r\n", msg->rruID, i, (bytetoword16(recvMsgFormRRU->fpstat[i].voltage)));
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module current is %d(mA)\r\n", msg->rruID, i, (bytetoword16(recvMsgFormRRU->fpstat[i].current)));	
		}
				}
			else{
		for(i=0;i<channel;i++){
		printf("RRU[%d] Fiber[%d] rx power is %.1f(uW)\r\n", msg->rruID, i, (bytetoword16(recvMsgFormRRU->fpstat[i].rxpw))*0.1);
		printf("RRU[%d] Fiber[%d] tx power is %.1f(uW)\r\n", msg->rruID, i, (bytetoword16(recvMsgFormRRU->fpstat[i].txpw))*0.1);
		printf("RRU[%d] Fiber[%d] optical module ineffect is %d\r\n", msg->rruID, i, recvMsgFormRRU->fpstat[i].ineffect);
		printf("RRU[%d] Fiber[%d] optical module vendor is %s\r\n", msg->rruID, i, recvMsgFormRRU->fpstat[i].vendor);
		printf("RRU[%d] Fiber[%d] optical module transfer rate is %d(Mbit/s)\r\n", msg->rruID, i, bytetoword16((u8 *)(recvMsgFormRRU->fpstat[i].rate)));
		printf("RRU[%d] Fiber[%d] optical module temp is %d(℃)\r\n", msg->rruID, i, (s8)(recvMsgFormRRU->fpstat[i].temperature));
		printf("RRU[%d] Fiber[%d] optical module voltage is %d(mV)\r\n", msg->rruID, i, (bytetoword16(recvMsgFormRRU->fpstat[i].voltage)));
		printf("RRU[%d] Fiber[%d] optical module current is %d(mA)\r\n", msg->rruID, i, (bytetoword16(recvMsgFormRRU->fpstat[i].current)));	
		}}
		}
			
		else{
			if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] rx power is %.1f(uw)\r\n", msg->rruID, channel, (bytetoword16(recvMsgFormRRU->fpstat[channel].rxpw))*0.1);
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] tx power is %.1f(uW)\r\n", msg->rruID, channel, (bytetoword16(recvMsgFormRRU->fpstat[channel].txpw))*0.1);
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module ineffect is %d\r\n", msg->rruID, channel, recvMsgFormRRU->fpstat[channel].ineffect);
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module vendor is %s\r\n", msg->rruID, channel, recvMsgFormRRU->fpstat[channel].vendor);
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module transfer rate is %d(Mbit/s)\r\n", msg->rruID, channel, bytetoword16((u8 *)(recvMsgFormRRU->fpstat[channel].rate)));
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module temp is %d(℃)\r\n", msg->rruID, channel, (s8)(recvMsgFormRRU->fpstat[channel].temperature));
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module voltage is %d(mV)\r\n", msg->rruID, channel, (bytetoword16(recvMsgFormRRU->fpstat[channel].voltage)));
		send_msg(cmd_for_rrumsg, "RRU[%d] Fiber CHN[%d] optical module current is %d(mA)\r\n", msg->rruID, channel, (bytetoword16(recvMsgFormRRU->fpstat[channel].current)));	
			}
			else{
		printf("RRU[%d] Fiber[%d] rx power is %.1f(uw)\r\n", msg->rruID, channel, (bytetoword16(recvMsgFormRRU->fpstat[channel].rxpw))*0.1);
		printf("RRU[%d] Fiber[%d] tx power is %.1f(uW)\r\n", msg->rruID, channel, (bytetoword16(recvMsgFormRRU->fpstat[channel].txpw))*0.1);
		printf("RRU[%d] Fiber[%d] optical module ineffect is %d\r\n", msg->rruID, channel, recvMsgFormRRU->fpstat[channel].ineffect);
		printf("RRU[%d] Fiber[%d] optical module vendor is %s\r\n", msg->rruID, channel, recvMsgFormRRU->fpstat[channel].vendor);
		printf("RRU[%d] Fiber[%d] optical module transfer rate is %d(Mbit/s)\r\n", msg->rruID, channel, bytetoword16((u8 *)(recvMsgFormRRU->fpstat[channel].rate)));
		printf("RRU[%d] Fiber[%d] optical module temp is %d(℃)\r\n", msg->rruID, channel, (s8)(recvMsgFormRRU->fpstat[channel].temperature));
		printf("RRU[%d] Fiber[%d] optical module voltage is %d(mV)\r\n", msg->rruID, channel, (bytetoword16(recvMsgFormRRU->fpstat[channel].voltage)));
		printf("RRU[%d] Fiber[%d] optical module current is %d(mA)\r\n", msg->rruID, channel, (bytetoword16(recvMsgFormRRU->fpstat[channel].current)));	
		}
			}
		}
	sem_post(&sem_rru_fiber_query);
}


void handle_paraQry_response(rru_test_msg *msg){
	ParameterQryRspT *recvMsgFormRRU;
	recvMsgFormRRU = (ParameterQryRspT *)msg->data;
	u16 query_result;
	query_result = recvMsgFormRRU->u16result;
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] parameter query result is %d\r\n", msg->rruID, query_result);}
	else
		printf("RRU[%d] parameter query result is %d\r\n", msg->rruID, query_result);
	if(query_result == 0){
		if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] vswr threshold is %.1f\r\n", msg->rruID, (recvMsgFormRRU->u16vswrThreshold)*0.1);
		send_msg(cmd_for_rrumsg, "RRU[%d] board temp threshold is %d(℃)\r\n", msg->rruID, recvMsgFormRRU->s8board_temp_threshold);
		send_msg(cmd_for_rrumsg, "RRU[%d] rf channel temp threshold is %d(℃)\r\n", msg->rruID, recvMsgFormRRU->s8rfchn_temp_threshold);
		send_msg(cmd_for_rrumsg, "RRU[%d] total time-slot is %d\r\n", msg->rruID, recvMsgFormRRU->u8TsTotalNum);
		send_msg(cmd_for_rrumsg, "RRU[%d] dl time-slot is %d\r\n", msg->rruID, recvMsgFormRRU->u8DLTsNum);
		}
		else{
		printf("RRU[%d] vswr threshold is %.1f\r\n", msg->rruID, (recvMsgFormRRU->u16vswrThreshold)*0.1);
		printf("RRU[%d] board temp threshold is %d(℃)\r\n", msg->rruID, recvMsgFormRRU->s8board_temp_threshold);
		printf("RRU[%d] rf channel  temp threshold is %d(℃)\r\n", msg->rruID, recvMsgFormRRU->s8rfchn_temp_threshold);
		printf("RRU[%d] total time-slot is %d, dl is %d\r\n", msg->rruID, recvMsgFormRRU->u8TsTotalNum, recvMsgFormRRU->u8DLTsNum);
		}
		}
	sem_post(&sem_rru_parameter_query);
}

void handle_sysTimeCfg_response(rru_test_msg *msg){
	TimeCfgRspT *recvMsgFormRRU;
	recvMsgFormRRU = (TimeCfgRspT *)msg->data;
	u16 query_result;
	query_result = bytetoword16(recvMsgFormRRU->result);
	if(g_alive_pc == 1){
	send_msg(cmd_for_rrumsg, "RRU[%d] system time config response is %d\r\n", msg->rruID, query_result);
	send_msg(cmd_for_rrumsg, "RRU[%d] system time config result is %d\r\n", msg->rruID, recvMsgFormRRU->ret);
	}
	else{
		printf("RRU[%d] system time config is %d\r\n", msg->rruID, query_result);
		printf("RRU[%d] system time config result is %d\r\n", msg->rruID, recvMsgFormRRU->ret);
		}
	sem_post(&sem_rru_time_cfg);
	}

void handle_almThreshodCfg_response(rru_test_msg *msg){
	AlarmThresholdCfgRspT *recvMsgFormRRU;
	recvMsgFormRRU = (AlarmThresholdCfgRspT *)msg->data;
	u16 query_result;
	query_result = bytetoword16(recvMsgFormRRU->result);
	if(query_result == 0){
		printf("RRU[%d] alarm threshold config is SUCC\r\n", msg->rruID);
		printf("RRU[%d] alarm threshold config result is %02X\r\n", msg->rruID, recvMsgFormRRU->ret);
		}
	else if(query_result == 1)
		printf("RRU[%d] alarm threshold config is FAIL\r\n", msg->rruID);
}

void handle_antennaStateCfg_response(rru_test_msg *msg){
	AntennaStateCfgRspT *recvMsgFormRRU;
	recvMsgFormRRU = (AntennaStateCfgRspT *)msg->data;
	u16 query_result;
	query_result = bytetoword16(recvMsgFormRRU->result);
	if(g_alive_pc==1){
		send_msg(cmd_for_rrumsg, "RRU[%d] antenna state config result is %d\r\n", msg->rruID, query_result);
		send_msg(cmd_for_rrumsg, "RRU[%d] antenna state config response is %d\r\n", msg->rruID, recvMsgFormRRU->ret);
		}
	else{
		printf("RRU[%d] antenna state config result is %d\r\n", msg->rruID, query_result);
		printf("RRU[%d] antenna state config response is %d\r\n", msg->rruID, recvMsgFormRRU->ret);
		}

	sem_post(&sem_rru_mask_cfg);
}

void handle_powerCal_response(rru_test_msg *msg){
	PowCalRsp *recvMsgFormRRU;
	recvMsgFormRRU = (PowCalRsp *)msg->data;
	u16 query_result;
	query_result = bytetoword16(recvMsgFormRRU->result);
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] power calibration response is %d\r\n", msg->rruID, query_result);}
	else
		printf("RRU[%d] power calibration is %d\r\n", msg->rruID, query_result);
	sem_post(&sem_rru_power_cal);
}

void handle_powerCal_indicate(rru_test_msg *msg){
	u8 i;
	calDataCfg *recvMsgFormRRU;
	recvMsgFormRRU = (calDataCfg*)msg->data;
	if(g_alive_pc == 1){
	send_msg(cmd_for_rrumsg, "RRU[%d] power calibration result is %02X\r\n", msg->rruID, recvMsgFormRRU->flag);
	send_msg(cmd_for_rrumsg, "RRU[%d]  tx power calibration RMS result is %02X\r\n", msg->rruID, recvMsgFormRRU->txrmsflg);
	send_msg(cmd_for_rrumsg, "RRU[%d]  tx DPD result is %02X\r\n", msg->rruID, recvMsgFormRRU->txdpdflg);
	for(i=0; i<recvMsgFormRRU->totalchn; i++){
	send_msg(cmd_for_rrumsg, "RRU[%d] calibration CHN[%d] tx power target is %d(dBm)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].u8antennapow);
	send_msg(cmd_for_rrumsg, "RRU[%d] calibration CHN[%d] tx power offset is %.1f(dB)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, (recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].s8txpowoffset)*0.1);
	send_msg(cmd_for_rrumsg, "RRU[%d] calibration CHN[%d] dl power gain is %.1f(dB)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, (recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].u16txgain)*0.5);
	send_msg(cmd_for_rrumsg, "RRU[%d] calibration CHN[%d] ul power gain is %.1f(dB)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, (recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].u16rxgain)*0.5);
	send_msg(cmd_for_rrumsg, "RRU[%d] calibration CHN[%d] rx power is %.1f(dBm)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, (recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].s16antRxPow)*0.1);
		}
		}
	else{
	printf("RRU[%d] power calibration result is %02X\r\n", msg->rruID, recvMsgFormRRU->flag);
	printf("RRU[%d]  tx power calibration RMS result is %02X\r\n", msg->rruID, recvMsgFormRRU->txrmsflg);
	printf("RRU[%d]  tx DPD result is %02X\r\n", msg->rruID, recvMsgFormRRU->txdpdflg);
	for(i=0; i<recvMsgFormRRU->totalchn; i++){
	printf("RRU[%d] CHN[%d]  tx power target is %d(dBm)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].u8antennapow);
	printf("RRU[%d] CHN[%d]  tx power offset is %.1f(dB)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, (recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].s8txpowoffset)*0.1);
	printf("RRU[%d] CHN[%d]  ul power gain is %.1f(dB)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, (recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].u16txgain)*0.5);
	printf("RRU[%d] CHN[%d]  dl power gain is %.1f(dB)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, (recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].u16rxgain)*0.5);
	printf("RRU[%d] CHN[%d]  rx power is %.1f(dBm)\r\n", msg->rruID, (recvMsgFormRRU->startchn)+i, (recvMsgFormRRU->chncell[(recvMsgFormRRU->startchn)+i].s16antRxPow)*0.1);
	}
	}
}


void handle_cellcfg_response(rru_test_msg *msg){
	CellCfgRspT *recvMsgFormRRU;
	recvMsgFormRRU = (CellCfgRspT*)msg->data;
	if(g_alive_pc ==1){
		send_msg(cmd_for_rrumsg, "RRU[%d] cell ID is %02X\r\n", msg->rruID, bytetoword32(recvMsgFormRRU->CellId));
		send_msg(cmd_for_rrumsg, "RRU[%d]  cell config result is %02X\r\n", msg->rruID, bytetoword32(recvMsgFormRRU->result));
		}
	else{
		printf ("RRU[%d] cell ID is %02X\r\n", msg->rruID, bytetoword32(recvMsgFormRRU->CellId));
		printf("RRU[%d]  cell config result is %02X\r\n", msg->rruID, bytetoword32(recvMsgFormRRU->result));
		}
	sem_post(&sem_rru_cell_config);
	 
}

void handle_rfOnOff_report(rru_test_msg *msg){
	int i;
	RFOnOffRptT *recvMsgFormRRU;
	recvMsgFormRRU = (RFOnOffRptT *)msg->data;
	if(g_alive_pc == 1){
	for(i=0; i<8; i++){
	send_msg(cmd_for_rrumsg, "RRU[%d] CHN[%d] DLPrevState is %d\r\n", msg->rruID, i, recvMsgFormRRU->AntennaOnOffStat[i].DLPrevState);
	send_msg(cmd_for_rrumsg, "RRU[%d] CHN[%d] DLNextState is %d\r\n", msg->rruID, i, recvMsgFormRRU->AntennaOnOffStat[i].DLNextState);
	send_msg(cmd_for_rrumsg, "RRU[%d] CHN[%d] ULPrevState is %d\r\n", msg->rruID, i, recvMsgFormRRU->AntennaOnOffStat[i].ULPrevState);
	send_msg(cmd_for_rrumsg, "RRU[%d] CHN[%d] DLNextState is %d\r\n", msg->rruID, i, recvMsgFormRRU->AntennaOnOffStat[i].ULNextState);
	}	
		}
	else{
	for(i=0; i<8; i++){
	printf("RRU[%d] CHN[%d] DLPrevState is %d\r\n", msg->rruID, i, recvMsgFormRRU->AntennaOnOffStat[i].DLPrevState);
	printf("RRU[%d] CHN[%d] DLNextState is %d\r\n", msg->rruID, i, recvMsgFormRRU->AntennaOnOffStat[i].DLNextState);
	printf("RRU[%d] CHN[%d] ULPrevState is %d\r\n", msg->rruID, i, recvMsgFormRRU->AntennaOnOffStat[i].ULPrevState);
	printf("RRU[%d] CHN[%d] DLNextState is %d\r\n", msg->rruID, i, recvMsgFormRRU->AntennaOnOffStat[i].ULNextState);
		}
	}
}

void handle_almQry_response(rru_test_msg *msg){
	AlarmQryRspT *recvMsgFormRRU;
	recvMsgFormRRU = (AlarmQryRspT *)msg->data;
	if(g_alive_pc== 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] alarm state is %d\r\n", msg->rruID, recvMsgFormRRU->result);}
	else
		printf("RRU[%d] alarm state is %d\r\n", msg->rruID, recvMsgFormRRU->result);
	sem_post(&sem_rru_alm_query);
}

void handle_reset_response(rru_test_msg *msg){
	ResetRspT *recvMsgFormRRU;
	recvMsgFormRRU = (ResetRspT *)msg->data;
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] reset result is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));}
	else
		printf("RRU[%d] reset result is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));
	sem_post(&sem_rru_reset);
}

void handle_powerReset_response(rru_test_msg *msg){
	ResetRspT *recvMsgFormRRU;
	recvMsgFormRRU = (ResetRspT *)msg->data;
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] power reset result is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));}
	else
		printf("RRU[%d] power reset result is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));
	sem_post(&sem_rru_reset);
}

void handle_hwParamQry_response(rru_test_msg *msg){
	HWParamRsp *recvMsgFormRRU;
	recvMsgFormRRU = (HWParamRsp*)msg->data;
	if(g_alive_pc == 1){
	send_msg(cmd_for_rrumsg, "RRU[%d] freq IO is %d\r\n", msg->rruID, recvMsgFormRRU->u8Freq_IO);
	send_msg(cmd_for_rrumsg, "RRU[%d] freq band value is %d(MHz)\r\n", msg->rruID, recvMsgFormRRU->u16Freq_Band_Val);
	send_msg(cmd_for_rrumsg, "RRU[%d] freq min is %d(KHz)\r\n", msg->rruID, recvMsgFormRRU->u32Freq_min);
	send_msg(cmd_for_rrumsg, "RRU[%d] freq max is %d(KHz)\r\n", msg->rruID, recvMsgFormRRU->u32Freq_max);
	send_msg(cmd_for_rrumsg, "RRU[%d] RF total channel is %d\r\n", msg->rruID, recvMsgFormRRU->u8RF_total_chn);	
	send_msg(cmd_for_rrumsg, "RRU[%d] RF Freq K is %d\r\n", msg->rruID, recvMsgFormRRU->u8RF_Freq_K);
	send_msg(cmd_for_rrumsg, "RRU[%d] RF Freq X is %d\r\n", msg->rruID, recvMsgFormRRU->u8RF_Freq_X);
	send_msg(cmd_for_rrumsg, "RRU[%d] hardware type is %s\r\n", msg->rruID, (char *)(recvMsgFormRRU->au8RRU_TYPE));
	send_msg(cmd_for_rrumsg, "RRU[%d] product type is %s\r\n", msg->rruID, (char *)(recvMsgFormRRU->au8RRU_ProductTYPE));
	}
	else{
	printf("RRU[%d] freq IO is %d\r\n", msg->rruID, recvMsgFormRRU->u8Freq_IO);
	printf("RRU[%d] freq band value is %d(MHz)\r\n", msg->rruID, recvMsgFormRRU->u16Freq_Band_Val);
	printf("RRU[%d] freq min is %d(KHz)\r\n", msg->rruID, recvMsgFormRRU->u32Freq_min);
	printf("RRU[%d] freq max is %d(KHz)\r\n", msg->rruID, recvMsgFormRRU->u32Freq_max);
	printf("RRU[%d] RF total channel is %d\r\n", msg->rruID, recvMsgFormRRU->u8RF_total_chn);	
	printf("RRU[%d] RF Freq K is %d\r\n", msg->rruID, recvMsgFormRRU->u8RF_Freq_K);
	printf("RRU[%d] RF Freq X is %d\r\n", msg->rruID, recvMsgFormRRU->u8RF_Freq_X);
	printf("RRU[%d] type is %s\r\n", msg->rruID, (char *)(recvMsgFormRRU->au8RRU_TYPE));
	printf("RRU[%d] product type is %s\r\n", msg->rruID, (char *)(recvMsgFormRRU->au8RRU_ProductTYPE));
	}
	sem_post(&sem_rru_hw_para);
}

void handle_verDownloadIndicate_response(rru_test_msg *msg){
	VersionDownloadRspT *recvMsgFormRRU;
	recvMsgFormRRU = (VersionDownloadRspT *)msg->data;
	if(g_alive_pc == 1)
		{send_msg(cmd_for_rrumsg, "RRU[%d] version download indicate is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));}
	else
		printf("RRU[%d] version download indicate is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));
}

void handle_verDownload_response(rru_test_msg *msg){
	VersionDownloadResIndT *recvMsgFormRRU;
	recvMsgFormRRU = (VersionDownloadResIndT *)msg->data;
	if(g_alive_pc == 1)
		{send_msg(cmd_for_rrumsg, "RRU[%d] version download result is %d\r\n", msg->rruID, recvMsgFormRRU->result);}
	else
		printf("RRU[%d] version download result is %d\r\n", msg->rruID, recvMsgFormRRU->result);
	sem_post(&sem_rru_sw_download);
}


void handle_verActivate_response(rru_test_msg *msg){
	VersionActivateRspT *recvMsgFormRRU;
	recvMsgFormRRU = (VersionActivateRspT *)msg->data;
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] version activate result is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));
		send_msg(cmd_for_rrumsg, "RRU[%d] version activate fail reason is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->reason));}
	else
		{
		printf("RRU[%d] version activate result is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));
		printf("RRU[%d] version activate fail reason is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->reason));
		}
	sem_post(&sem_rru_sw_active);	
}

void handle_fiberDelayCfg_response(rru_test_msg *msg){
	DelayMeasureRspT* recvMsgFormRRU;
	recvMsgFormRRU = (DelayMeasureRspT *)msg->data;
	if(g_alive_pc == 1){
		send_msg(cmd_for_rrumsg, "RRU[%d] fiber delay config is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));}
	else
		printf("RRU[%d] fiber delay config is %d\r\n", msg->rruID, bytetoword16(recvMsgFormRRU->result));
	sem_post(&sem_rru_fiberDelay_cfg);
}


cmd_t rrutest_version_query(cmd_t cmd){
	int k = -1;
	sem_init(&sem_rru_hw_query, 0, 0);
	sem_init(&sem_rru_sw_query, 0, 0);
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header, rrutest_msg_send.len);
	word32tobyte(type_SWVerQry, rrutest_msg_send.type);
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
	else{
	k=-1;}
	usleep(5000);
	word32tobyte(type_HWVerQry, rrutest_msg_send.type);
	
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
	else{
	k=-1;}
	
	
	struct timespec ts1 = timeout_calc(30);
	struct timespec ts2 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_hw_query, &ts1) == 0){
		cmd.pkg_successtimes = 1;
		cmd.pkg_failtimes = 0;
		}
	else{
		cmd.pkg_failtimes = 1;
		cmd.pkg_successtimes = 0;
		}

	if(sem_timedwait(&sem_rru_sw_query, &ts2) == 0)
		cmd.pkg_successtimes &= 1;
		
	else
		cmd.pkg_failtimes |= 1;
	
	send_result(cmd);
	
	pthread_mutex_unlock(&rru_lock);
	return cmd;	

}


cmd_t rrutest_rfstatus_query(cmd_t cmd){
	int k = -1;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	
	RFStateQryReqT *RFStateQryMsg;
	RFStateQryMsg = (RFStateQryReqT *)rrutest_msg_send.data;
	RFStateQryMsg->rfchn = cmd.pkg_data[1];
	
	word32tobyte(len_rrutest_header+1, rrutest_msg_send.len);
	word32tobyte(type_RFStateQryReq, rrutest_msg_send.type);
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+1, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
	sem_init(&sem_rru_rf_query, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_rf_query, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	cmd.pkg_datalen = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	
	return cmd;
	
}

cmd_t rrutest_rrustatus_query(cmd_t cmd){
	int k = -1;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	
	word32tobyte(len_rrutest_header, rrutest_msg_send.len);
	word32tobyte(type_RruStateQryReq, rrutest_msg_send.type);

	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	perror("error rrutest send: ");	
	}

	sem_init(&sem_rru_rru_query, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_rru_query, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
}


cmd_t rrutest_fiberstatus_query(cmd_t cmd){
	int k = -1;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	
	FiberPortStateQryReqT *FiberPortStateQryMsg;
	FiberPortStateQryMsg = (FiberPortStateQryReqT *)rrutest_msg_send.data;
	FiberPortStateQryMsg->fiberport = cmd.pkg_data[1];
	
	word32tobyte(len_rrutest_header+1, rrutest_msg_send.len);
	word32tobyte(type_FiberStateQryReq, rrutest_msg_send.type);

	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+1, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	
	if(k<0)
	{
	perror("error rrutest send: ");	
	}

	sem_init(&sem_rru_fiber_query, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_fiber_query, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	cmd.pkg_datalen = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
}

cmd_t rrutest_para_query(cmd_t cmd){
	int k = -1;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header, rrutest_msg_send.len);
	word32tobyte(type_ParameterQryReq, rrutest_msg_send.type);

	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
	sem_init(&sem_rru_parameter_query, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_parameter_query, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	cmd.pkg_datalen = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
}

cmd_t rrutest_systime_config(cmd_t cmd){
	int k = -1;
	TimeCfgReqT *timeCfgReqMsg;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header+sizeof(TimeCfgReqT), rrutest_msg_send.len);
	word32tobyte(type_SysTimeCfgReq, rrutest_msg_send.type);
	timeCfgReqMsg = (TimeCfgReqT *)rrutest_msg_send.data;

	time_t t;
	struct tm *p;
	time(&t);
	p=localtime(&t);
	timeCfgReqMsg->second = (u8)(p->tm_sec);
	timeCfgReqMsg->minute = (u8)(p->tm_min);
	timeCfgReqMsg->hour = (u8)(p->tm_hour);
	timeCfgReqMsg->day = (u8)(p->tm_mday);
	timeCfgReqMsg->month = (u8)(p->tm_mon+1);
	word16tobyte((u16)(p->tm_year+1900), timeCfgReqMsg->year);
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+sizeof(TimeCfgReqT), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
	sem_init(&sem_rru_time_cfg, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_time_cfg, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	cmd.pkg_datalen = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
}


void rrutest_threshold_config(u16 vswr, s8 board, s8 rfchn){
	int k = -1;
	AlarmThresholdCfgReqT *alarmThresholdCfgMsg;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header+sizeof(AlarmThresholdCfgReqT), rrutest_msg_send.len);
	word32tobyte(type_AlarmThresholdCfgReq, rrutest_msg_send.type);
	alarmThresholdCfgMsg = (AlarmThresholdCfgReqT *)rrutest_msg_send.data;

	alarmThresholdCfgMsg->swr[0] = (char)((vswr>>8) & 0xff);
	alarmThresholdCfgMsg->swr[1] = (char)(vswr & 0xff);

	alarmThresholdCfgMsg->board_temp_threshold= (char)(board);
	alarmThresholdCfgMsg->rfchn_temp_threshold = (char)(rfchn);
	
	pthread_mutex_lock(&msg_send_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+sizeof(AlarmThresholdCfgReqT), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	pthread_mutex_unlock(&msg_send_lock);
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
}

cmd_t rrutest_antenna_config(cmd_t cmd){
	int k = -1;
	AntennaStateCfgReqT *AntennaStateCfgMsg;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header+sizeof(AntennaStateCfgReqT), rrutest_msg_send.len);
	word32tobyte(type_AntennaStateCfgReq, rrutest_msg_send.type);
	AntennaStateCfgMsg = (AntennaStateCfgReqT *)rrutest_msg_send.data;
	AntennaStateCfgMsg->dlantennamask= cmd.pkg_data[1];
	AntennaStateCfgMsg->ulantennamask= cmd.pkg_data[2];
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+sizeof(AntennaStateCfgReqT), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	pthread_mutex_unlock(&msg_send_lock);
	if(k<0)
	{
	perror("error rrutest send: ");	
	}

	sem_init(&sem_rru_mask_cfg, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_mask_cfg, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	cmd.pkg_datalen = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
	
}

cmd_t rrutest_power_cal(cmd_t cmd){
	int k = -1;
	PowCal *PowCalMsg;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header+sizeof(PowCal), rrutest_msg_send.len);
	word32tobyte(type_PowerCaliCfgReq, rrutest_msg_send.type);
	PowCalMsg = (PowCal *)rrutest_msg_send.data;
	PowCalMsg->u16CalType= 0;

	PowCalMsg->u16CalOpType= bytetoword16(&(cmd.pkg_data[1]));
	PowCalMsg->u32CalCenterFreq= bytetoword32(&(cmd.pkg_data[3]));
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+sizeof(PowCal), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	pthread_mutex_unlock(&msg_send_lock);
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
	sem_init(&sem_rru_power_cal, 0, 0);
	struct timespec ts = timeout_calc(30);
	if(sem_timedwait(&sem_rru_power_cal, &ts) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	cmd.pkg_datalen = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
}

cmd_t rrutest_cell_config(cmd_t cmd){
	int k = -1;
	CellCfgReqT *cellCfgMsg;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header+sizeof(CellCfgReqT), rrutest_msg_send.len);
	word32tobyte(type_CellCfgReq, rrutest_msg_send.type);
	cellCfgMsg = (CellCfgReqT *)rrutest_msg_send.data;
	memcpy(cellCfgMsg, (CellCfgReqT *)&(cmd.pkg_data[1]), (cmd.pkg_datalen-1));
	cellCfgMsg->u16SCGMask[1] = cellCfgMsg->u16SCGMask[0];
	cellCfgMsg->u16SCGMask[2] = cellCfgMsg->u16SCGMask[0];
	cellCfgMsg->u16SCGMask[3] = cellCfgMsg->u16SCGMask[0];
	printf("====cell cfg msg =%d\r\n ", cellCfgMsg->u16SCGMask[3]);
	
	bsp_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+sizeof(CellCfgReqT), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	send_msg(cmd, "小区配置发送失败\r\n ");
	}
	sem_init(&sem_rru_cell_config, 0, 0);
	struct timespec ts = timeout_calc(30);
	if(sem_timedwait(&sem_rru_cell_config, &ts) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	cmd.pkg_datalen = 1;
	send_result(cmd);
	
	bsp_mutex_unlock(&rru_lock);
	return cmd;
	
}


cmd_t rrutest_reset(cmd_t cmd){
	int k = -1;
	ResetReqT *ResetReqMsg;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header+sizeof(ResetReqT), rrutest_msg_send.len);
	
	if(cmd.pkg_data[1] == 0)
		word32tobyte(type_ResetReq, rrutest_msg_send.type);
	else if(cmd.pkg_data[1] == 1)
		word32tobyte(type_PowerResetReq, rrutest_msg_send.type);

	ResetReqMsg = (ResetReqT *)rrutest_msg_send.data;
	ResetReqMsg->resettype=0;
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+sizeof(ResetReqT), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	perror("error rrutest send: ");	
	}
	sem_init(&sem_rru_reset, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_reset, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	cmd.pkg_datalen = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
	
}

cmd_t rrutest_hwparam_query(cmd_t cmd){
	int k = -1;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header, rrutest_msg_send.len);
	word32tobyte(type_HWParamReq, rrutest_msg_send.type);
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	perror("error rrutest send: ");
	}

	sem_init(&sem_rru_hw_para, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_hw_para, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
}

cmd_t rrutest_verdownload_req(cmd_t cmd){
	int k = -1;
	VersionDownloadReqT * verDownloadReqMsg;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header+sizeof(VersionDownloadReqT), rrutest_msg_send.len);
	word32tobyte(type_VerDownReq, rrutest_msg_send.type);

	verDownloadReqMsg = (VersionDownloadReqT *)rrutest_msg_send.data;
	strcpy(verDownloadReqMsg->filename, "RRU");
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+sizeof(VersionDownloadReqT), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	
	if(k<0)
	{
	perror("error rrutest send: ");
	}

	sem_init(&sem_rru_sw_download, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_sw_download, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);

	return cmd;
}

cmd_t rrutest_veractivate_req(cmd_t cmd){
	int k = -1;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header, rrutest_msg_send.len);
	word32tobyte(type_VerActivateReq, rrutest_msg_send.type);

	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	pthread_mutex_unlock(&msg_send_lock);
	if(k<0)
	{
	perror("error rrutest send: ");
	}

	sem_init(&sem_rru_sw_active, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_sw_active, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);

	return cmd;
}

cmd_t rrutest_almquery_req(cmd_t cmd){
	int k = -1;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header, rrutest_msg_send.len);
	word32tobyte(type_AlarmQryReq, rrutest_msg_send.type);
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	
	if(k<0)
	{
	perror("error rrutest send: ");
	}
	sem_init(&sem_rru_alm_query, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_alm_query, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	
	return cmd;
}

cmd_t rrutest_fiberdelay_config(cmd_t cmd){
	int k = -1;
	u16 y1, y2, FiberDelay;
	f32 usdelay;
	u8 slot, port; 
	DelayMeasureReqT * DelayMeasureReqMsg;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	slot = (g_rruID >>5) & 0x7;
	port = (g_rruID >>2) & 0x7;
	word32tobyte(len_rrutest_header+sizeof(DelayMeasureReqT), rrutest_msg_send.len);
	word32tobyte(type_DelayCfgReq, rrutest_msg_send.type);
	DelayMeasureReqMsg = (DelayMeasureReqT *)rrutest_msg_send.data;
	DelayMeasureReqMsg ->FiberId[1] = port;
	//printf("bbp slot is %d, port is %d\r\n", slot, port);
	pthread_mutex_lock(&bbp_lock);
	bsp_bbp_fpga_write(slot, 29, 7);
	bsp_bbp_fpga_write(slot, 29, 0);
	usleep(5000);
	bsp_bbp_fpga_read(slot, port+5, &FiberDelay);
	usdelay = FiberDelay/(153.6*2);
	send_msg(cmd, "FiberDelay measure is %.2f(us)\r\n", usdelay);
	y1 = (FiberDelay+20)/40;
    	y2 = (FiberDelay+1)/2;

	bsp_bbp_fpga_write(slot, port+2, y1);
	pthread_mutex_unlock(&bbp_lock);
	word16tobyte(y1, DelayMeasureReqMsg->Y1);
	word16tobyte(y2, DelayMeasureReqMsg->Y2);	
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header+sizeof(DelayMeasureReqT), 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	if(k<0)
	{
	perror("error rrutest send: ");
	}

	sem_init(&sem_rru_fiberDelay_cfg, 0, 0);
	struct timespec ts1 = timeout_calc(30);
	if(sem_timedwait(&sem_rru_fiberDelay_cfg, &ts1) == 0)
		cmd.pkg_successtimes = 1;
	else
		cmd.pkg_failtimes = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
}


cmd_t rrutest_serialclose_req(cmd_t cmd){
	int k = -1;
	rru_test_msg rrutest_msg_send;
	rrutest_msg_send.rruID=g_rruID;
	word32tobyte(len_rrutest_header, rrutest_msg_send.len);
	word32tobyte(type_SerialCloseReq, rrutest_msg_send.type);
	
	pthread_mutex_lock(&rru_lock);
	k=sendto(rrutest_fd_send, &rrutest_msg_send, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), sizeof(rru_test_addr));
	pthread_mutex_unlock(&msg_send_lock);
	if(k<0)
	{
	perror("error rrutest send: ");
	return cmd;
	}
	
	cmd.pkg_successtimes = 1;
	send_result(cmd);
	pthread_mutex_unlock(&rru_lock);
	return cmd;
}


cmd_t rrutest_heartbeat_to_pc(cmd_t cmd){
	cmd_for_rrumsg = cmd;
	g_heartbeat_to_pc = cmd.pkg_data[1];
	g_alive_pc = 1;
	
	memcpy(&(cmd.pkg_data[2]), &rru_info, sizeof(RruInformationT));	
	cmd.pkg_datalen = sizeof(RruInformationT)+2;
	bsp_mutex_lock(&rru_lock);
	send_result(cmd);
	bsp_mutex_unlock(&rru_lock);
	return cmd;
}

void *handle_rru_message(void *arg){

	int len,j = 0;
	u8 recv_bf[14]={0};
	u32 msgtype, msglen;
	memset(recv_bf,0,14);
	rru_test_msg heartbeat_msg;
	
	while (1){
		if (FPGA_STATUS != 1){
		//printf("board_status is %x \r\n", FPGA_STATUS);
		sleep(2);
		continue;
		}
		printf("board_status is %x \r\n", FPGA_STATUS);
		break;
	}

	rrutest_fd_recv = socket(AF_INET, SOCK_DGRAM, 0);
	if (rrutest_fd_recv < 0){
		printf("rru socket creat error!!\r\n");
			return NULL;
			}
	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = inet_addr("10.0.0.1");
	local_addr.sin_port = htons(RRU_TEST_LOCAL_PORT);
	if (bind(rrutest_fd_recv, (struct sockaddr*)&local_addr, sizeof(local_addr))!=0){
		printf("bind boardtest_fd fail!!\r\n");
		close(rrutest_fd_recv);
		rrutest_fd_recv = -1;
		return NULL;
			}

	int addrlen = sizeof(rru_test_addr);
	while(1){
	j=0;
	len = recvfrom(rrutest_fd_recv,  &heartbeat_msg, sizeof(heartbeat_msg), 0, (struct sockaddr*)&rru_test_addr, (socklen_t *)&addrlen);

	if(len<0){
	sleep(1);
	continue;
	}
	memcpy(recv_bf, &heartbeat_msg, 14);
	msgtype = bytetoword32(heartbeat_msg.type);
	msglen = bytetoword32(heartbeat_msg.len);
	
	if (len != msglen){
	printf("RRU[%d] message type[%d] message len[%d] is wrong\r\n", heartbeat_msg.rruID, msgtype, msglen);
	continue;
	}
	
	switch (msgtype)
	{
		case type_HeartBeatRsp:
			rru_info.rruID = heartbeat_msg.rruID;
			g_rrutest_heartbeat_count++;
			send_msg(cmd_for_rrumsg, "recv rru[%d] heartbeat count is: %d\r\n",heartbeat_msg.rruID, bytetoword16(heartbeat_msg.rsv));
			break;

		case type_chnSetup:
			handle_channel_setup(&heartbeat_msg);
			break;

		case type_chnSetupCfgRsp:
			handle_chnSetupCfg_response(&heartbeat_msg);
			break;

		case type_Alarm:
			handle_alarm_report(&heartbeat_msg);
			break;

		case type_SWVerQryRsp:
			handle_SWVer_result(&heartbeat_msg);
			break;

		case type_HWVerQryRsp:
			handle_HWVer_result(&heartbeat_msg);
			break;	

		case type_RFStateQryRsp:
			handle_RFStatusQry_response(&heartbeat_msg);
			break;

		case type_RruStateQryRsp:
			handle_RRUStatusQry_response(&heartbeat_msg);
			break;

		case type_FiberStateQryRsp:
			handle_FiberStatusQry_response(&heartbeat_msg);
			break;

		case type_ParameterQryRsp:
			handle_paraQry_response(&heartbeat_msg);
			break;

		case type_SysTimeCfgRsp:
			handle_sysTimeCfg_response(&heartbeat_msg);
			break;

		case type_AlarmThresholdCfgRsp:
			handle_almThreshodCfg_response(&heartbeat_msg);
			break;

		case type_AntennaStateCfgRsp:
			handle_antennaStateCfg_response(&heartbeat_msg);
			break;

		case type_PowerCaliCfgRsp:
			handle_powerCal_response(&heartbeat_msg);
			break;

		case type_PowerCaliCfgResInd:
			handle_powerCal_indicate(&heartbeat_msg);
			break;

		case type_CellCfgRsp:
			handle_cellcfg_response(&heartbeat_msg);
			break;

		case type_RFOnOffRpt:
			handle_rfOnOff_report(&heartbeat_msg);
			break;

		case type_ResetRsp:
			handle_reset_response(&heartbeat_msg);
			break;

		case type_PowerResetRsp:
			handle_powerReset_response(&heartbeat_msg);
			break;

		case type_HWParamRsp:
			handle_hwParamQry_response(&heartbeat_msg);
			break;

		case type_VerDownRsp:
			handle_verDownloadIndicate_response(&heartbeat_msg);
			break;

		case type_VerDownResInd:
			handle_verDownload_response(&heartbeat_msg);
			break;

		case type_VerActivateRsp:
			handle_verActivate_response(&heartbeat_msg);
			break;

		case type_AlarmQryRsp:
			handle_almQry_response(&heartbeat_msg);
			break;

		case type_DelayCfgRsp:
			handle_fiberDelayCfg_response(&heartbeat_msg);
			break;
			
		default:
			printf("recv msgtype: %d\r\n", msgtype);
			while(j<len_rrutest_header){
			printf("%02X ", recv_bf[j]);
			j++;}
			printf("\r\n");
			break;
		}
	}
	
}

void *send_heartbeat(void *arg){

	int heartbeat_count_self = 1;
	int i,k = 0;
	u8 send_bf[14]={0};
	u8 pc0, pc1;
	
	memset(send_bf,0,14);
	rrutest_fd_send = socket(AF_INET, SOCK_DGRAM, 0);
	if (rrutest_fd_send< 0){
		printf("socket creat error!!\r\n");
			return NULL;
			}

	int addrlen = sizeof(rru_test_addr);

	rru_test_msg heartbeat_msg;
	heartbeat_msg.bbuID = 0x01;
	
	word32tobyte(110, heartbeat_msg.type);
	word32tobyte(len_rrutest_header, heartbeat_msg.len);
	memcpy(send_bf, &heartbeat_msg, 14);
	
	while (1)
	{
	pc0 = g_heartbeat_to_pc;
	if (rru_test_addr.sin_port == RRU_TEST_PORT){		
		heartbeat_msg.rruID = rru_test_addr.sin_addr.s_addr & 0xff;
		for(i=0; i<3; i++)
		{
		if(heartbeat_count_self <= g_rrutest_heartbeat_count){
			word16tobyte(heartbeat_count_self, heartbeat_msg.rsv);
			pthread_mutex_lock(&msg_send_lock);
			k=sendto(rrutest_fd_send, &heartbeat_msg, len_rrutest_header, 0, (struct sockaddr*)&(rru_test_addr), addrlen);
			pthread_mutex_unlock(&msg_send_lock);
		if(k<0)
		{
		perror("error rrutest send");
		}
		else{
		send_msg(cmd_for_rrumsg, "heartbeat to rru[%d] send count: %d\r\n", heartbeat_msg.rruID, heartbeat_count_self);
		heartbeat_count_self ++;
		break;
		//printf("addrvalue1: date = %lX, len = %d\r\n", rru_test_addr1.sin_addr.s_addr, sizeof(rru_test_addr1.sin_addr.s_addr));
		}
		}
			
		else{
		send_msg(cmd_for_rrumsg, "heartbeat to rru count error!\r\n ");
		heartbeat_count_self--;
		memset(&rru_info, 0, sizeof(rru_info));
		}
		sleep(1);
		}
		
	}
	sleep(30);
	pc1 = g_heartbeat_to_pc;
	if(pc0 == pc1)
		g_alive_pc = -1;
	
	}
	
	}


void test_addr_st(void){

	ul32 recvaddr = 0;
	ul32 recvaddr2 = 0;
	
	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = inet_addr("10.0.0.1");
	local_addr.sin_port = htons(RRU_TEST_LOCAL_PORT);
      	recvaddr = local_addr.sin_addr.s_addr;

	struct sockaddr_in test_addr;
	test_addr.sin_family = AF_INET;
	test_addr.sin_addr.s_addr = inet_addr("10.0.0.160");
	test_addr.sin_port = htons(RRU_TEST_PORT);
      	recvaddr2 = test_addr.sin_addr.s_addr;
	
printf("recvaddr: date = %lX, len = %d\r\n", recvaddr, sizeof(local_addr.sin_addr.s_addr));
printf("recvaddr2: date = %lX, len = %d\r\n", recvaddr2, sizeof(test_addr.sin_addr.s_addr));

		}
	

void init_rrutest_thread(void){
	printf("rrutest thread init!\r\n");
	pthread_t heartbeat_send;
	pthread_t recv_rru_message;
	pthread_create(&heartbeat_send, NULL, send_heartbeat, NULL);
    pthread_detach(heartbeat_send);
	pthread_create(&recv_rru_message, NULL, handle_rru_message, NULL);
    pthread_detach(recv_rru_message);
	//test_addr_st();	
}
