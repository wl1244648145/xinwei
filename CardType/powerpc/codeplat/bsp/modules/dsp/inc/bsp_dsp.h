/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_dsp.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_DSP_H
#define BSP_DSP_H

#define BOOTTBL_HEADER_LEN 4
#define MAX_PAYLOAD_LEN    1472     /* 发送镜像数据包长度*/
#define MAX_BOOTTBL_LEN (MAX_PAYLOAD_LEN - BOOTTBL_HEADER_LEN) 
#define SOCKET_ERROR (-1)

#define BBU_DSP_IS_DOWNLOADED          1          /*  dsp program is downloaded*/
#define BBU_DSP_NOT_DOWNLOADED         0          /*  DSP  program is not downloaded*/
#define BBU_DSP_DOWNLOAD_ENABLE        1          /* the preparation for dsp download had done*/
#define BBU_DSP_DOWNLOAD_DISABLE       0          /*the preparation for dsp download isnot done*/
#define BBU_DSP_DOWNLOAD_DO            1           /*the flag to show the state for dsp which want download each time*/
#define BBU_DSP_DOWNLOAD_DONE          2           /*to show the complete of download*/
#define BBU_DSP_DOWNLOAD_RAW           0           /*the raw state for flag*/

#define BBU_SOCKET_SEND_DELAY          6000
#define BBU_SOCKET_PORT_DSP_BOOTP      67
#define BBU_SOCKET_PORT_DSP_DOWNLOAD   9   //0  修改为9
#define BBU_DSP_MAX_BUF_LENGTH         500
#define BBU_SOCKET_PORT_DSP_SEND_UDP        2046
#define BBU_SOCKET_PORT_DSP_RECV_UDP        2000


#define  BBU_DSP_ID_1                   1
#define  BBU_DSP_ID_2                   2
#define  BBU_DSP_ID_3                   3
#define  BBU_DSP_ID_4                   4

#define BSP_RESET_DSP   1
#define BSP_CLOSE_DSP   2


#if 1
const s8 dsp_image_boot_init[] = "/mnt/btsa/dsp_image_boot_init";
const s8 dsp1_image_name[] = "/mnt/btsa/dsp_image_L1_1";
const s8 dsp2_image_name[] = "/mnt/btsa/dsp_image_L1_1";
const s8 dsp3_image_name[] = "/mnt/btsa/dsp_image_L1_1";
const s8 dsp4_image_name[] = "/mnt/btsa/dsp_image_L1_4";
#else
const s8 dsp_image_boot_init[] = "/tmp/download/dsp_image_boot_init";
const s8 dsp1_image_name[] = "/tmp/download/dsp_image_L1_1";
const s8 dsp2_image_name[] = "/tmp/download/dsp_image_L1_2";
const s8 dsp3_image_name[] = "/tmp/download/dsp_image_L1_3";
const s8 dsp4_image_name[] = "/tmp/download/dsp_image_L1_4";
#endif

const s8 dsp1_download_ip[20] = "10.0.0.54";
const s8 dsp2_download_ip[20] = "10.0.0.55";
const s8 dsp3_download_ip[20] = "10.0.0.56";
const s8 dsp4_download_ip[20] = "10.0.0.57";

s8   pDownloadIp[20] = {0};
s8  *pMPC_to_dsp_Ip = "10.0.0.1";

u8  g_u8Dsp1DlFlag = BBU_DSP_NOT_DOWNLOADED;         /* flag for dsp loaded or not */
u8  g_u8Dsp2DlFlag = BBU_DSP_NOT_DOWNLOADED;         /* flag for dsp loaded or not */
u8  g_u8Dsp3DlFlag = BBU_DSP_NOT_DOWNLOADED;         /* flag for dsp loaded or not */
u8  g_u8Dsp4DlFlag = BBU_DSP_NOT_DOWNLOADED;         /* flag for dsp loaded or not */

/*def for download record for each time */
u8  g_u8DspDownloadDo1 = BBU_DSP_DOWNLOAD_DO;          /* flag to indicate  download or not for each time of download */
u8  g_u8DspDownloadDo2 = BBU_DSP_DOWNLOAD_DO;          /* flag to indicate  download or not for each time of download */          
u8  g_u8DspDownloadDo3 = BBU_DSP_DOWNLOAD_DO;          /* flag to indicate  download or not for each time of download */
u8  g_u8DspDownloadDo4 = BBU_DSP_DOWNLOAD_DO;          /* flag to indicate  download or not for each time of download */

s32 g_s32SocketFd;                                      /* socket identification for download */
s32 g_s32SocketFdBootp;                                 /* socket identification for bootp wait */
u32 g_u32PacketSendNum;                                 /* for statistic of download packet*/
u8  g_u8DspDownloadEnFlag = BBU_DSP_DOWNLOAD_DISABLE;   /*flag for download preparation*/
u32 g_s32DspDownloadDebugSwitch = 1;                   /* hsi debug switch */

typedef struct bootp_msg{
    u8	bp_op;			/* packet opcode type	*/
    u8	bp_htype;      /* hardware addr type	*/
    u8	bp_hlen;		/* hardware addr length */
    u8	bp_hops;		/* gateway hops 	*/
    ul32 bp_xid;			/* transaction ID 	*/
    u16	bp_secs;		/* seconds since boot 	*/
    u16	bp_unused;
    struct in_addr	bp_ciaddr;		/* client IP address 	*/
    struct in_addr	bp_yiaddr;		/* 'your' IP address 	*/
    struct in_addr	bp_siaddr;		/* server IP address 	*/
    struct in_addr	bp_giaddr;		/* gateway IP address 	*/
    u8	bp_chaddr [16]; 	/* client hardware addr	*/
    u8	bp_sname [64]; 	/* server host name 	*/
    u8	bp_file [128];	/* boot file name 	*/
    u8	bp_vend [64];	/* vendor-specific area */
} BOOTP_MSG;

#endif
/******************************* 头文件结束 ********************************/

