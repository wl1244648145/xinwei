#ifndef __BSP_EXTERNAL_H__
#define __BSP_EXTERNAL_H__


/*+++++++++++++++++++++++++++++++++++++++
+                     BSP  函数声明                                  +
+                                                                                    +
+++++++++++++++++++++++++++++++++++++++++*/

#ifdef __CPU_PPC__
#ifndef BSP_GET_CPU_CYCLE
#define BSP_GET_CPU_CYCLE
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
/* Alternate Time Base */
#define SPR_ATBL	526
#define SPR_ATBU	527
#define mfspr(reg) \
({ \
    register_t ret; \
    asm volatile("mfspr %0, %1" : "=r" (ret) : "i" (reg) : "memory");\
    ret; \
})

#define mtspr(rn, v)   asm volatile("mtspr " __stringify(rn) ",%0" : : "r" (v))

#define BspGetCpuCycle mfatb
static inline uint64_t mfatb(void)
{
    uint32_t hi, lo, chk;
    do {
        hi = mfspr(SPR_ATBU);
        lo = mfspr(SPR_ATBL);
        chk = mfspr(SPR_ATBU);
    } while (unlikely(hi != chk));
    return (uint64_t) hi << 32 | (uint64_t) lo;
}
#endif

typedef unsigned int UINT32;

extern u16 bsp_fpga_read_reg(u16 u16Reg_offset);

extern void bsp_fpga_write_reg(u16 u16Reg_offset, u16 u16Dat);

extern s16 bbp_fpga_read_reg(u16 u16BordId, u16 u16Reg_offset, u16 *u16Dat);

extern s32 bsp_get_write_bbp_registered_reg_results(u8 slotid, u8 RegType,u16 u16offset);

extern s16 bbp_cpld_read_reg(u16 u16BordId, u8 u8Reg_offset, u8 *u8Dat);

extern s16 bbp_cpld_write_reg(u16 u16BordId, u8 u8Reg_offset, u8 u8Dat);

extern s32 power_write(u8 addr, u8 *data, u8 count);

extern s32 power_read(u8 addr);

extern int bsp_subboard_heart_bit(u16 boardid);

u32 bsp_get_longframe_num(void);

extern u8	read_CONF_DONE(void);

extern void *bsp_get_ub_addr(int iLen);

extern void BspTimerCallBack(VOIDFUNCPTR pCallBack);

extern u8 bsp_get_gps_level(void);

extern int bsp_get_if_flag(int index);
extern int bsp_gps_sync_time_alarm(void);
extern u8 bsp_cpld_read_reg(u8 u8Reg_offset);
#if defined(__CPU_LTE_CARDTYPE__) || defined(__CPU_LTE_CENTERSTATION__)
extern s32 dsp_download_status_get(u16 boardid);
extern s32 bsp_get_gps_status(u8 *u8GpsStatus);
#else
extern s32 dsp_download_status_get();
#endif
extern s32 bsp_get_dsp_isload(u16 slotid);
extern s32 bsp_get_tc_type();
extern u16 afc_lock_status_get();
extern u8 bsp_gps_TrackedSatellites(void);
extern u8 bsp_gps_VisibleSatellites(void);
#if defined(__CPU_LTE_MACRO__) || defined(__CPU_LTE_CENTERSTATION__)
extern s32 bsp_get_bbu_pcbversion(char *buffer_version, int len);
extern int bsp_get_bbu_production_date(int *year, int *month, int *day);
extern s32 bsp_get_bbu_devicdid(char *buffer_devicdid, int len);
#ifndef __CPU_LTE_CENTERSTATION__
extern s32 bsp_read_power(f32 *f32p_power);
#endif 
extern float bsp_get_temperature();
#endif 
#ifdef __CPU_LTE_CARDTYPE__
extern s32 bsp_get_bbu_pcbversion(unsigned char boardid, char *buffer_version, int len);
extern int bsp_get_bbu_production_date(unsigned char boardid, int *year, int *month, int *day);
//extern s32 bsp_get_bbu_devicdid(unsigned char boardid, char *buffer_devicdid, int len);
extern s32 bsp_get_bbu_productionsn(unsigned char boardid, char *buffer_productionsn, int len);
extern s32 bsp_read_power(unsigned char boardid, f32 *f32p_power, f32 *f32p_voltage, f32 *f32p_current);
extern float bsp_get_temperature(unsigned char boardid);
extern s32 bsp_peu_power_off(u16 slotid);
extern void bsp_disable_mswd(void);
#endif
#if defined(__CPU_LTE_SMALLCELL__)||defined(__CPU_LTE_SMALLCELL_PACK__)
extern float bsp_get_temperature();
#endif
#ifdef __CPU_LTE_CENTERSTATION__
extern s32 bsp_read_power(unsigned char boardid, f32 *f32p_power, f32 *f32p_voltage, f32 *f32p_current);
#endif
extern s32 bsp_get_fan_speed(u8 u8fan_id,u16 *u16pfan_speed);
extern s32 fan_control_algorithm_1(s8 temp);
extern int bsp_sfp_online(int sfp_id);
extern u32 get_afc_lock_times();
extern void bsp_power_off(u16 power_off_time);
extern s32 bsp_set_tc_type(u8 Type, u8 ClockSynWay, u8 CascadeCfg);
extern u8 bsp_init_get_gps_tc_type(u8 Value);
extern u8 bsp_gps_support_type(void);
extern s32 bsp_gps_get_type(void);
extern s32 bspGetBtsHWVersion(s8 *phwversion);
extern s32 bspGetPauHWVersion(s8 *phwversion);
extern s32 bspGetDeviceID(char *pdevice);
extern s32 bspGetPauDeviceID(char *pdevice);
extern unsigned char bsp_get_resetcause(void);
extern void reboot_aif_dsp_fpga();
extern void bsp_led_run_1s(void);
extern s32 bsp_get_mbd_manufacture_date(char *date);
extern s32 bsp_get_pau_product_date(char *product_date);
extern UINT16 mbd_eeprom_get_pau_manu_id();
extern void single_bat_alert(u8 * single_bat_status, int bat_num);
extern u8 battery_power_status();
extern UINT16 mbd_eeprom_get_freq_band_index();
extern UINT16 mbd_eeprom_get_product_type();
extern unsigned int bsp_ntpd_serve_client_config(unsigned int dwvalue);
/*ms interface*/
extern unsigned char bsp_get_slot_id(void);
/*SRIO_Switch_Status*/
extern unsigned int bsp_subboard_srio_status(unsigned short boardid, u32 **u32sriostatus);
#endif


/* Led extern interface */
void bsp_led_run_on(void);
void bsp_led_run_off(void);
void bsp_led_run_blink(int period_ms);	/* period_ms is unit of ms */

void bsp_led_alarm_on(void);
void bsp_led_alarm_off(void);
void bsp_led_alarm_blink(int period_ms);

void bsp_led_fan_on(void);
void bsp_led_fan_off(void);
void bsp_led_fan_blink(int period_ms);

void bsp_led_act_on(void);
void bsp_led_act_off(void);

/* Fun interface */
int bsp_get_fan_speed(unsigned char u8fan_id, unsigned short *u16pfan_speed);	/* u8fan_id is rang 1 to 4. */

/* sfp infterface */
#ifdef __CPU_LTE_MACRO__
int bsp_get_fiber_rx_power(int sfp_id, float *rx_power);
int bsp_get_fiber_tx_power(int sfp_id, float *tx_power);
int bsp_get_sfp_voltuage(int sfp_id, float *volatuage);
int bsp_get_sfp_temperature(int sfp_id, float *temperature);
int bsp_get_sfp_current(int sfp_id, float *current);
int bsp_get_sfp_los_state(int sfp_id);
int bsp_get_sfp_vendor(int sfp_id, char *vendor);
int bsp_get_sfp_speed(int sfp_id);
#endif
#ifdef __CPU_PPC__
/* ms infterface */
unsigned char bsp_get_self_MS_state();
unsigned char bsp_get_MS_state_for_OAM(void);
s8 bsp_get_host_ip(u8 *ip);
s8 bsp_get_host_mac(u8 * mac);
s8 bsp_get_standby_ip(u8 *ip);
s8 bsp_get_standby_mac(u8 *mac);
s8 bsp_get_standby_online(void);
signed char bsp_get_another_board_ip(unsigned char *ip);
s8 bsp_get_self_ETH1_ip(u8 *ip);
signed char bsp_ready_MCT_state_switch();
signed char bsp_master_slave_switch(unsigned char switchto, unsigned int cause);
int bsp_send_boards_status_info(void);
#endif

/* dsp black box fpga dump */
int bsp_dsp_black_box(unsigned char, unsigned int, int);

/* bsp reset */
int bsp_reset_subboard(unsigned int boardid);






#define NET_WORK_TYPE_SELF_ADAPTIVE     0
#define NET_WORK_TYPE_ELECTRICAL        1
#define NET_WORK_TYPE_OPTICAL           2

#define NET_WORK_MODE_AUTONEG                (0)
#define NET_WORK_MODE_FULL_DUPLEX_10M        (1)
#define NET_WORK_MODE_FULL_DUPLEX_100M       (2)
#define NET_WORK_MODE_FULL_DUPLEX_1000M      (3)

typedef struct {
	int type;	//0表示电 1表示光
	int speed;	//1000, 100, 10
	int duplex;	//1 full 0 half
} NetPortInfo_t;

#define ERR_MODE                1
#define ERR_NO_LINK             2
#define ERR_UNSUPPORT           3

#ifdef __CPU_PPC__
UINT32 BspGetNetPortState(UINT32  dwPort, NetPortInfo_t *info);
UINT32 BspSetNetWorkInfo(UINT32 dwPort, UINT32 dwNetWorkType, UINT32 dwElecWorkMode, UINT32 dwOptWorkMode); 
#endif

#define RESET_CAUSE_CPLD_UPDATE 0xF0 /* update cpld */
#define RESET_CAUSE_POWER_ON    0x80 /* power on reset */
#define RESET_CAUSE_WATCHDOG    0x40 /* watch dog reset */
#define RESET_CAUSE_BUTTON      0x20 /* reset key */
#define RESET_CAUSE_BOARD       0x10 /* global reset */
#ifdef __CPU_LTE_CARDTYPE__
#define RESET_CAUSE_PPC         0x08  /*PPC reset*/
#define RESET_CAUSE_MSWD        0x04  /* cardtype master(to slave) watchdog */
#else
#define RESET_CAUSE_PPC         0x00  /*PPC reset*/
#endif

//同步源相关告警事件内部ID 
/** 1PPS+TOD **/
#define BSP_ALM_1PPS_TOD_PPS_ABNORMAL						(0X1001) //秒脉冲不可用
#define BSP_ALM_1PPS_TOD_PPS_NORMAL							(0X1002) //秒脉冲正常
#define BSP_ALM_1PPS_TOD_SRC_STATE_UNRELIABLE				(0X1003) //时钟源工作状态不可靠(不是3D-fix或Time only fix)
#define BSP_ALM_1PPS_TOD_SRC_STATE_RELIABLE					(0X1004) //时钟源工作状态可靠(是3D-fix或Time only fix)
//以下为TOD时间状态消息中监控告警字段定义的时钟源状态告警
#define BSP_ALM_1PPS_TOD_MON_ALM_ANTENNA_OPEN				(0X1011) 
#define BSP_ALM_1PPS_TOD_MON_ALM_ANTENNA_OPEN_CLEAR 		(0X1012)
#define BSP_ALM_1PPS_TOD_MON_ALM_ANTENNA_SHORT				(0X1013)
#define BSP_ALM_1PPS_TOD_MON_ALM_ANTENNA_SHORT_CLEAR		(0X1014)
#define BSP_ALM_1PPS_TOD_MON_ALM_NOT_TRACKING_SATELLITES 	(0X1015)
#define BSP_ALM_1PPS_TOD_MON_ALM_NOT_TRACKING_SATELLITES_CLEAR (0X1016)
#define BSP_ALM_1PPS_TOD_MON_ALM_SURVEY_IN_PROGRESS 		(0X1017)
#define BSP_ALM_1PPS_TOD_MON_ALM_SURVEY_IN_PROGRESS_CLEAR 	(0X1018)
#define BSP_ALM_1PPS_TOD_MON_ALM_NO_STORED_POSITION 		(0X1019)
#define BSP_ALM_1PPS_TOD_MON_ALM_NO_STORED_POSITION_CLEAR 	(0X101A)
#define BSP_ALM_1PPS_TOD_MON_ALM_LEAP_SEC_PENDING			(0X101B)
#define BSP_ALM_1PPS_TOD_MON_ALM_LEAP_SEC_PENDING_CLEAR		(0X101C)
#define BSP_ALM_1PPS_TOD_MON_ALM_IN_TEST_MODE				(0X101D)
#define BSP_ALM_1PPS_TOD_MON_ALM_IN_TEST_MODE_CLEAR			(0X101E)
#define BSP_ALM_1PPS_TOD_MON_ALM_POS_IS_QUESTIONABLE		(0X101F)
#define BSP_ALM_1PPS_TOD_MON_ALM_POS_IS_QUESTIONABLE_CLEAR	(0X1020)
#define BSP_ALM_1PPS_TOD_MON_ALM_ALMANAC_NOT_COMPLETE		(0X1021)
#define BSP_ALM_1PPS_TOD_MON_ALM_ALMANAC_NOT_COMPLETE_CLEAR	(0X1022)
#define BSP_ALM_1PPS_TOD_MON_ALM_PPS_WAS_GENERATED			(0X1023)
#define BSP_ALM_1PPS_TOD_MON_ALM_PPS_WAS_GENERATED_CLEAR	(0X1024)
/** 1588 **/
#define BSP_ALM_1588_LINK_DOWN					(0X2001) //1588通讯异常
#define BSP_ALM_1588_LINK_UP					(0X2002) //1588通讯正常
#define BSP_ALM_1588_NOT_SAME_DOMAIN			(0X2003) //1588主从设备不在同一域内
#define BSP_ALM_1588_SAME_DOMAIM				(0X2004) //1588主从设备在同一域内
#define BSP_ALM_1588_NOT_SAME_VERSION			(0X2005)
#define BSP_ALM_1588_SAME_VERSION				(0X2006)
#define BSP_ALM_1588_SYNCSOURCE_ERROR			(0X2007)
#define BSP_ALM_1588_SYNCSOURCE_OK				(0X2008)
#define BSP_ALM_1588_CONNECT_ERROR			       (0X2009)
#define BSP_ALM_1588_CONNECT_OK				       (0X200a)

/** Sync Source configuration and switch **/
#define BSP_ALM_SYN_SRC_INVALID_CONFIGURATION	(0X0001) //配置了不支持的同步源
#define BSP_ALM_SYN_SRC_REMAIN_UNCHANGED		(0X0002) //将要切换到的目标同步源和当前同步源一致，不进行操作
#define BSP_ALM_SYN_SRC_ERROR					(0X0003) //同步源不可用
#define BSP_ALM_SYN_SRC_SWITCH					(0X0004) //同步源切换
#define BSP_ALM_SYN_SRC_OK                      (0X0005) //同步源可用
#define BSM_ALM_SYN_SRC_FAULT                   (0X0006)
#define BSM_ALM_SYN_SRC_FAULT_CLEAR             (0X0007)

typedef struct
{
	unsigned char oldSynSrc;
	unsigned char newSynSrc;
}T_SyncSrcSwitch;

typedef union
{
	T_SyncSrcSwitch t_switchInfo;
	unsigned char currentSynSrc;
}T_SynSrcAlmInfo;

struct st1588Attr
{
    unsigned char u8NodeType;
    unsigned char u8delay_mechanism;
    unsigned char u8ClockMode;
    unsigned char u8Domain;
    unsigned char u8PtpProtocol;
    signed char s8AnnounceInterval;
    signed char s8SyncInterval;
    signed char s8DelayReqInterval;
    signed char s8PdelayReqInterval;
    unsigned char u8ClockPriority1;
    unsigned char u8ClockPriority2;
    unsigned char u8Res;
    unsigned char u8VlanTag;
    unsigned char u8VlanPri;
    unsigned short u16VlanId;
    unsigned int u32DelayInAsymmetry;
    unsigned int u32DelayOutAsymmetry;
};
#ifdef __CPU_LTE_CARDTYPE__
typedef struct
{
    unsigned char   los;
    unsigned int  temper;
    unsigned int  vol;
    unsigned int  current;
    unsigned int  tx_power;
    unsigned int  rx_power;
    unsigned int  speed;
    char   vendor_name[16];
}fiber_info;
#ifdef __CPU_PPC__
extern s32 bsp_get_bbp_sfpinfo(u8 slotid, uint8_t sfp_id, fiber_info *sfpinfo);
#endif
#endif
typedef void (*SynSrcAlarmFuncPtr)(unsigned short u16EventId, T_SynSrcAlmInfo *pt_info);
typedef unsigned int (*FMALARM_FUNCPTR)(unsigned short u16EventId);
typedef unsigned int (*LOADALARM_FUNCPTR)(unsigned short u16EventId, unsigned short u16SlotId);
#ifdef __CPU_PPC__
extern s32 bsp_get_ptp_gm_clockIdentity (u8 *pu8GMclockIdentity);
extern s32 bsp_get_ptp_gm_clockClass (u8 *pu8gmClockClass);
extern s32 bsp_get_ptp_gm_clockAccuracy(u8 *pu8gmClockAccuracy);
extern u32 bsp_get_synsrc_configuration(u8 Value);
extern s32 bsp_set_clocksource(u32 u32SynSrc,  u8 ClockSynWay, u8 CascadeCfg);
extern s32 bsp_get_clocksource(void);
extern void BspFanFaultAlarmCallBack(FMALARM_FUNCPTR pCallBack);
extern void BspLoadFaultAlarmCallBack(LOADALARM_FUNCPTR pCallBack);
extern u32 bsp_register_synsrc_alm(SynSrcAlarmFuncPtr pfSynSrcAlm);
extern s32 bsp_set_1588_attr(struct st1588Attr *pAttr);
extern int bsp_subboard_check_alive(unsigned short boardid);
#endif

/*主备切换原因及不能切换原因,注意增删改需要通知OAM*/
#define MCT_MS_SWITCH_CASE_CLEAR 	0x00	//清除主备切换原因
#define MCT_SWITCH_CASE_OPP_OUT		0x01	//对板拔出
#define	MCT_SWITCH_CASE_OPP_RESET 	0x02	//对板复位
#define MCT_SWITCH_CASE_OPP_DISABLE	0x03	//对板不可用
#define MCT_SWITCH_CASE_COMPETITION	0x04	//主备竞争
#define MCT_SWITCH_CASE_GE0_UNLINK  0x05	//核心网网线没有连接
#define MCT_SWITCH_CASE_GPS_UNLOCK	0x06	//GPS锁定星数不足
#define MCT_SWITCH_CASE_CPLD_PLL	0x07	//CPLD PLL失锁
#define MCT_SWITCH_CASE_SWITCH		0x08	//SWITCH配置失败
#define MCT_SWITCH_CASE_CLK_DISABLE	0x09	//本板时钟不可用
#define MCT_SWITCH_CASE_NO_OAM_MS_FUNC	0x0a	//OAM未注册主备切换函数

#ifdef __CPU_LTE_CARDTYPE__
//告警命令字
#define BSP_FAN_LOS_CONTROL_ERROR		      (3)
#define BSP_FAN_LOS_CONTROL_OK			      (4)
#define BSP_FAN_OLD_TEST_ERROR			      (5)
#define BSP_FAN_OLD_TEST_OK			          (6)
#define BSP_DSP_LOAD_ERROR			          (7)
#define BSP_DSP_LOAD_OK			              (8)
#define BSP_FPGA_LOAD_ERROR			          (9)
#define BSP_FPGA_LOAD_OK			          (10)
#define BSP_ARM_VERSION_UPDATE_ERROR	      (11)
#define BSP_ARM_VERSION_UPDATE_OK			  (12)

//typedef UINT32 (*FMALARM_FUNCPTR)(UINT16 u16EventId);
//typedef UINT32 (*LOADALARM_FUNCPTR)(UINT16 u16EventId, UINT16 u16SlotId);
UINT32 (*FanAlarmProc)(UINT16 u16EventId);
UINT32 (*LoadFaultAlarmProc)(UINT16 u16EventId, UINT16 u16SlotId);
#endif
#endif /* __BSP_EXTERNAL_H__ */
