/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* ?′???t??:           bbu_config.h
* 1|?ü:
* °?±?:
* ±à??è??ú:
* ×÷??:
*******************************************************************************/
/************************** °üo????téù?÷ **********************************/
/**************************** 12ó?í·???t* **********************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <linux/if_vlan.h>
#include <linux/sockios.h>

/**************************** ??ó?í·???t* **********************************/
#include "../../../com_inc/bsp_types.h"
#include "../inc/bsp_usdpaa.h"
#include "../../spinlock/inc/spinlockapi.h"
#include "../inc/fsl_shmem.h"
#include "../../../modules/usdpaa/inc/compat.h"
#include "../../../modules/hmi/inc/hmi.h"
#include "../../../com_inc/bsp_boardtest_ext.h"
#include "../../ms/inc/bsp_ms.h"
#include "../../../com_inc/bsp_conkers_ext.h"

int bsp_send_host_standby_switch_msg(u8 switchto,u32 cause);

static volatile T_SpinLock  ethspinlock;

extern unsigned char g_eth0auMacAddr[6];
extern unsigned char g_eth1auMacAddr[6];
extern unsigned char g_eth2auMacAddr[6];
extern unsigned char g_eth3auMacAddr[6];
extern u32 g_subboard_reset_over;
#define ETH_TYPE_DEBUG_INFO_TRACE  (0)
#define ETH_TYPE_LTE_SIGNAL_TRACE  (1)
#define ETH_TYPE_LTE_MTS_TRACE     (2)
#define  ETH_TYPE_LTE_NRL2_TRACE    (3)

#define MAX_PRTCL_NAME_LEN                  20
#define MAX_FEATURE_LEN                     6
#define MAX_FEATURE_PER_PRTCL              10
#define MAX_IF_NAME_LEN                     20
#define MAX_IF_NAME_LEN                     20
#define MAX_PRTCL_NAME_LEN                  20
#define MAX_FEATURE_LEN                     6
#define MAX_PRTCL_NUM                       100
#define MAX_IFBIND_NUM                      10
#define MAX_FEATURE_PER_PRTCL              10
#define MAX_PRTCL_PER_IF                    10
#define PACKET_NROP     9 /* Not Receive the Outgoing Packets */

#define GMAC_COMM_ERR_BASE                  0x1000000
#define GMAC_COMM_ERR_IF_NOEXIST            GMAC_COMM_ERR_BASE+1
#define GMAC_COMM_ERR_IFTBL_FULL            GMAC_COMM_ERR_BASE+2
#define GMAC_COMM_ERR_IF_BINDERR            GMAC_COMM_ERR_BASE+3
#define GMAC_COMM_ERR_IF_NOTFIND            GMAC_COMM_ERR_BASE+4
#define GMAC_COMM_ERR_PRTCL_NOTFIND         GMAC_COMM_ERR_BASE+5
#define GMAC_COMM_ERR_IF_NOTBIND            GMAC_COMM_ERR_BASE+6
#define GMAC_COMM_ERR_PARAMERR              GMAC_COMM_ERR_BASE+7
#define GMAC_COMM_ERR_IF_PRTCLFULL          GMAC_COMM_ERR_BASE+8
#define GMAC_COMM_ERR_IF_SEND_NOBUF         GMAC_COMM_ERR_BASE+9
#define GMAC_COMM_ERR_IF_SEND_ERR           GMAC_COMM_ERR_BASE+10
#define GMAC_COMM_ERR_PRTCL_NOTSUPPORT      GMAC_COMM_ERR_BASE+11
#define GMAC_COMM_ERR_SKERROR      GMAC_COMM_ERR_BASE+12

#define IPF_NOTF          0
#define IPF_NEW           1
#define IPF_ISF           2
extern unsigned char g_iprebuild[1024*500];
extern int g_ippacklen;



//{0x30,0x31,0x32,0x33,0x34,0x30} --ctl
//{0x30,0x31,0x32,0x33,0x34,0x31} ---data

unsigned char EI_DSP_Monitor_Header[]=
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x30, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0xaa, 0xbb,0xaa,0xaa
};

#define OAM_MONITER_DSP_DATA_MSG	3630
#define OAM_MONITER_DSP_STOP_MSG	3632

#define OAM_MONITOR_EI_BASIC_START_MSG 	3633
#define OAM_MONITOR_EI_SPECIFIC_START_MSG 3634
#define OAM_MONITOR_EI_ALL_STOP_MSG 	3635
#define OAM_MONITOR_EI_SPECIFIC_STOP_MSG 3636

unsigned char gaucHeardBeatPacketHead[]=
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x30, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0xaa, 0xbb,0xaa,0xaa
};

unsigned char gaucCpuDspPacketHead[]=
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x30, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0xab, 0xcd,0x00,0x00
};

unsigned char gaucCtlPacketHead[]=
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x3c, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0xaa, 0xaa,0xaa,0xaa
};

unsigned char gaucDataPacketHead[]=
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x3d, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0xaa, 0xaa,0xaa,0xaa
};

/*o?D?í?μ?macμ??·?**/
unsigned char gaucCoreNetPacketHead[]=
{
    0xfc, 0x4d, 0xd4, 0xf0, 0xeb, 0x3f, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0x08, 0x00,
};

unsigned char gaucoremac[][6] =
{
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x30},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x31},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x32},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x33},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x34},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x35},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x36},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x37},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x38},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x39},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x3a},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x3b},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x3c},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x3d},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x3e},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x3f},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x60},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x61},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x62},
    {0x30, 0x31, 0x32, 0x33, 0x34, 0x63}
};

struct stSlotDspMac
{
    int slot;
    const unsigned char * mac;
};
unsigned char gauslot2coremac[][6] =
{
    {0x00,0x01,0x02,0x00,0x02,0x0a},
    {0x00,0x01,0x02,0x00,0x02,0x0b},
    {0x00,0x01,0x02,0x00,0x02,0x0c},
    {0x00,0x01,0x02,0x00,0x02,0x0d},
    {0x00,0x01,0x02,0x00,0x02,0x14},
    {0x00,0x01,0x02,0x00,0x02,0x15},
    {0x00,0x01,0x02,0x00,0x02,0x16},
    {0x00,0x01,0x02,0x00,0x02,0x17},
    {0x00,0x01,0x02,0x00,0x02,0x1e},
    {0x00,0x01,0x02,0x00,0x02,0x1f},
    {0x00,0x01,0x02,0x00,0x02,0x20},
    {0x00,0x01,0x02,0x00,0x02,0x21},
    {0x00,0x01,0x02,0x00,0x02,0x28},
    {0x00,0x01,0x02,0x00,0x02,0x29},
    {0x00,0x01,0x02,0x00,0x02,0x2a},
    {0x00,0x01,0x02,0x00,0x02,0x2b},
    {0x00,0x01,0x02,0x00,0x02,0x2c},
    {0x00,0x01,0x02,0x00,0x02,0x2d},
    {0x00,0x01,0x02,0x00,0x02,0x2e},
    {0x00,0x01,0x02,0x00,0x02,0x2f}
};
unsigned char gauslot3coremac[][6] =
{
    {0x00,0x01,0x02,0x00,0x03,0x0a},
    {0x00,0x01,0x02,0x00,0x03,0x0b},
    {0x00,0x01,0x02,0x00,0x03,0x0c},
    {0x00,0x01,0x02,0x00,0x03,0x0d},
    {0x00,0x01,0x02,0x00,0x03,0x14},
    {0x00,0x01,0x02,0x00,0x03,0x15},
    {0x00,0x01,0x02,0x00,0x03,0x16},
    {0x00,0x01,0x02,0x00,0x03,0x17},
    {0x00,0x01,0x02,0x00,0x03,0x1e},
    {0x00,0x01,0x02,0x00,0x03,0x1f},
    {0x00,0x01,0x02,0x00,0x03,0x20},
    {0x00,0x01,0x02,0x00,0x03,0x21},
    {0x00,0x01,0x02,0x00,0x03,0x28},
    {0x00,0x01,0x02,0x00,0x03,0x29},
    {0x00,0x01,0x02,0x00,0x03,0x2a},
    {0x00,0x01,0x02,0x00,0x03,0x2b},
    {0x00,0x01,0x02,0x00,0x03,0x2c},
    {0x00,0x01,0x02,0x00,0x03,0x2d},
    {0x00,0x01,0x02,0x00,0x03,0x2e},
    {0x00,0x01,0x02,0x00,0x03,0x2f}
};
unsigned char gauslot4coremac[][6] =
{
    {0x00,0x01,0x02,0x00,0x04,0x0a},
    {0x00,0x01,0x02,0x00,0x04,0x0b},
    {0x00,0x01,0x02,0x00,0x04,0x0c},
    {0x00,0x01,0x02,0x00,0x04,0x0d},
    {0x00,0x01,0x02,0x00,0x04,0x14},
    {0x00,0x01,0x02,0x00,0x04,0x15},
    {0x00,0x01,0x02,0x00,0x04,0x16},
    {0x00,0x01,0x02,0x00,0x04,0x17},
    {0x00,0x01,0x02,0x00,0x04,0x1e},
    {0x00,0x01,0x02,0x00,0x04,0x1f},
    {0x00,0x01,0x02,0x00,0x04,0x20},
    {0x00,0x01,0x02,0x00,0x04,0x21},
    {0x00,0x01,0x02,0x00,0x04,0x28},
    {0x00,0x01,0x02,0x00,0x04,0x29},
    {0x00,0x01,0x02,0x00,0x04,0x2a},
    {0x00,0x01,0x02,0x00,0x04,0x2b},
    {0x00,0x01,0x02,0x00,0x04,0x2c},
    {0x00,0x01,0x02,0x00,0x04,0x2d},
    {0x00,0x01,0x02,0x00,0x04,0x2e},
    {0x00,0x01,0x02,0x00,0x04,0x2f}
};
unsigned char gauslot5coremac[][6] =
{
    {0x00,0x01,0x02,0x00,0x05,0x0a},
    {0x00,0x01,0x02,0x00,0x05,0x0b},
    {0x00,0x01,0x02,0x00,0x05,0x0c},
    {0x00,0x01,0x02,0x00,0x05,0x0d},
    {0x00,0x01,0x02,0x00,0x05,0x14},
    {0x00,0x01,0x02,0x00,0x05,0x15},
    {0x00,0x01,0x02,0x00,0x05,0x16},
    {0x00,0x01,0x02,0x00,0x05,0x17},
    {0x00,0x01,0x02,0x00,0x05,0x1e},
    {0x00,0x01,0x02,0x00,0x05,0x1f},
    {0x00,0x01,0x02,0x00,0x05,0x20},
    {0x00,0x01,0x02,0x00,0x05,0x21},
    {0x00,0x01,0x02,0x00,0x05,0x28},
    {0x00,0x01,0x02,0x00,0x05,0x29},
    {0x00,0x01,0x02,0x00,0x05,0x2a},
    {0x00,0x01,0x02,0x00,0x05,0x2b},
    {0x00,0x01,0x02,0x00,0x05,0x2c},
    {0x00,0x01,0x02,0x00,0x05,0x2d},
    {0x00,0x01,0x02,0x00,0x05,0x2e},
    {0x00,0x01,0x02,0x00,0x05,0x2f}
};
unsigned char gauslot6coremac[][6] =
{
    {0x00,0x01,0x02,0x00,0x06,0x0a},
    {0x00,0x01,0x02,0x00,0x06,0x0b},
    {0x00,0x01,0x02,0x00,0x06,0x0c},
    {0x00,0x01,0x02,0x00,0x06,0x0d},
    {0x00,0x01,0x02,0x00,0x06,0x14},
    {0x00,0x01,0x02,0x00,0x06,0x15},
    {0x00,0x01,0x02,0x00,0x06,0x16},
    {0x00,0x01,0x02,0x00,0x06,0x17},
    {0x00,0x01,0x02,0x00,0x06,0x1e},
    {0x00,0x01,0x02,0x00,0x06,0x1f},
    {0x00,0x01,0x02,0x00,0x06,0x20},
    {0x00,0x01,0x02,0x00,0x06,0x21},
    {0x00,0x01,0x02,0x00,0x06,0x28},
    {0x00,0x01,0x02,0x00,0x06,0x29},
    {0x00,0x01,0x02,0x00,0x06,0x2a},
    {0x00,0x01,0x02,0x00,0x06,0x2b},
    {0x00,0x01,0x02,0x00,0x06,0x2c},
    {0x00,0x01,0x02,0x00,0x06,0x2d},
    {0x00,0x01,0x02,0x00,0x06,0x2e},
    {0x00,0x01,0x02,0x00,0x06,0x2f}
};
unsigned char gauslot7coremac[][6] =
{
    {0x00,0x01,0x02,0x00,0x07,0x0a},
    {0x00,0x01,0x02,0x00,0x07,0x0b},
    {0x00,0x01,0x02,0x00,0x07,0x0c},
    {0x00,0x01,0x02,0x00,0x07,0x0d},
    {0x00,0x01,0x02,0x00,0x07,0x14},
    {0x00,0x01,0x02,0x00,0x07,0x15},
    {0x00,0x01,0x02,0x00,0x07,0x16},
    {0x00,0x01,0x02,0x00,0x07,0x17},
    {0x00,0x01,0x02,0x00,0x07,0x1e},
    {0x00,0x01,0x02,0x00,0x07,0x1f},
    {0x00,0x01,0x02,0x00,0x07,0x20},
    {0x00,0x01,0x02,0x00,0x07,0x21},
    {0x00,0x01,0x02,0x00,0x07,0x28},
    {0x00,0x01,0x02,0x00,0x07,0x29},
    {0x00,0x01,0x02,0x00,0x07,0x2a},
    {0x00,0x01,0x02,0x00,0x07,0x2b},
    {0x00,0x01,0x02,0x00,0x07,0x2c},
    {0x00,0x01,0x02,0x00,0x07,0x2d},
    {0x00,0x01,0x02,0x00,0x07,0x2e},
    {0x00,0x01,0x02,0x00,0x07,0x2f}
};

struct stSlotDspMac gstcoremac[] =
{
    [2] = {.slot = 2, .mac = gauslot2coremac,},
    [3] = {.slot = 3, .mac = gauslot3coremac,},
    [4] = {.slot = 4, .mac = gauslot4coremac,},
    [5] = {.slot = 5, .mac = gauslot5coremac,},
    [6] = {.slot = 6, .mac = gauslot6coremac,},
    [7] = {.slot = 7, .mac = gauslot7coremac,},
};

typedef struct _T_MSG_H
{
    unsigned short u16SrcSPID;
    unsigned short u16DstSPID;
    unsigned short u16MsgType;
    unsigned short u16MsgLength;
    unsigned short s16DevID;
    unsigned short s16CoreNO;
} T_MSG_H;
#define L1_MSG_ID_START                       (20001)
#define DSP2CPU_START_OK                      (unsigned short)L1_MSG_ID_START + 100
#define CPU2DSP_WORK                          (unsigned short)L1_MSG_ID_START + 101
#define DSP2CPU_HEARTBEAT                     (unsigned short)L1_MSG_ID_START + 102
#define MAX_PACK_LEN  (20*1024)
unsigned char g_eth0buff[MAX_PACK_LEN]= {0};
unsigned char g_eth1buff[MAX_PACK_LEN]= {0};
unsigned char g_eth3buff[MAX_PACK_LEN]= {0};
unsigned char g_au8UserPlaneBuf[MAX_PACK_LEN] =
{
    0x30, 0x31, 0x32, 0x33, 0x34, 0x3d, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0xaa, 0xaa,0xaa,0xaa
};

CHAR *BSP_S_strcpy( CHAR *pcDst, WORD32 dwMaxSize, const CHAR *pcSrc );
unsigned int BspSendIpData(int dwport,unsigned char *pbuf,int len);
//int BspSendIpData(char wan_port, char *pData, UINT16 usDataLength);
extern void(*EmacFromEth0ToCpu)(unsigned char *pbuf,int len);
extern void(*EmacFromEth1ToCpu)(unsigned char *pbuf,int len);
extern void(*EmacFromEth3ToCpu)(unsigned char *pbuf,int len);
extern void(*EmacFromEth0ToDsp)(unsigned char *pbuf,int len);
extern void(*EmacFromEth0ToDspTest)(unsigned char *pbuf,int len);

static void bsp_netif_throughput_init(void);
typedef int (*FUNCPTR)();
typedef struct t_Feature
{
    unsigned short          ftOffset;
    unsigned char           ftLen;
    unsigned char           ftValue[MAX_FEATURE_LEN];
} T_FEATURE;

typedef struct t_Protocol
{
    UINT8           name[MAX_PRTCL_NAME_LEN];
    INT32           dwSndPcktOkCnt;
    INT32           dwSndPcktErrCnt;
    INT32           dwRcvPcktCnt;
    T_FEATURE       atFeature[MAX_FEATURE_PER_PRTCL];
    FUNCPTR         pReceiveReturn;
} T_PROTOCOL;
typedef struct t_If
{
    unsigned char   name[MAX_IF_NAME_LEN];
    unsigned char   unit;
} T_IF;

typedef struct t_IfPrtcl
{
    unsigned char   name[MAX_PRTCL_NAME_LEN];
    T_IF            tIf;
} T_IFPRTCL;

typedef struct t_IfBind
{
    T_IF            tIf;
    unsigned char   isBind;
    unsigned long   dwSndPcktCnt;
    unsigned long   dwRcvPcktCnt;
    unsigned long   dwRcvPcktNoFndPrtclCnt;
    unsigned long   sockRcv;
    unsigned long   sockSend;
    unsigned char   byRegPrtclNum;
    T_PROTOCOL      atProtocol[MAX_PRTCL_PER_IF];
} T_IFBIND;

typedef struct t_GmacStat
{
    int             dwRcvPcktCnt;
    int             dwPcktToCallBack;
    int             dwPcktCallBackFree;
} T_GMACSTAT;

T_GMACSTAT      gtGmacStat;
T_IFPRTCL       gatIfPrtclTbl[MAX_PRTCL_NUM];
T_IFBIND        gatIfBindTbl[MAX_IFBIND_NUM];
T_IFPRTCL       gatIfPrtclTbl[MAX_PRTCL_NUM] =
{
    {"ETHSW1",  {"eth", 0}},
    {"ETHSW2",  {"eth", 1}},
    {"DEBUG",   {"eth", 2}},
    {"CORENET", {"eth", 3}},
    {""},
};

static unsigned int usdpaa_ip_defrag = 0;
static unsigned int usdpaa_ip_new = 0;
static unsigned int usdpaa_ip_notf = 0;
static unsigned int usdpaa_ip_frag = 0;

#ifdef BSP_DEBUG
unsigned char g_u8EpcSendSwitch = 0;
#else
extern unsigned char g_u8EpcSendSwitch;
#endif
unsigned char MacStackRcvRtn(unsigned long ifBindTblIndex, unsigned char *buf, int len)
{
    int			 dwDataLen = 0;
    unsigned char			*byData;
    int			 i, j, k;
    char			 byFeatureInPrtcl = 0;
    char          ifName[20] = { 0 };
    unsigned char SrcMac[6] = {0};
    dwDataLen = len;
    byData = buf;
    i = ifBindTblIndex;
#if 0
    sprintf(ifName, "%s%d", gatIfBindTbl[i].tIf.name, gatIfBindTbl[i].tIf.unit);
    if (BSP_OK != bsp_get_netif_mac_addr((CHAR*)ifName,SrcMac))
    {
        printf("MAC Address Get Error\n");
        return BSP_ERROR;
    }
    if(0 == memcmp(byData + 6, SrcMac, 6))
    {
        return;
    }
#endif
    if (ifBindTblIndex == 3)
    {
        usdpaa_ip_defrag = ip_defrag_stub((unsigned char *)buf);
        if(IPF_NEW == usdpaa_ip_defrag)
        {
            byData =  g_iprebuild;
            dwDataLen = g_ippacklen;
            usdpaa_ip_new++;
        }
        else if(IPF_NOTF == usdpaa_ip_defrag)
        {
            byData = buf;
            dwDataLen = len;
            usdpaa_ip_notf++;
        }
        else
        {
            usdpaa_ip_frag++;
            return;
        }
    }
    gtGmacStat.dwRcvPcktCnt++;

    if (gatIfBindTbl[i].isBind)
    {
        gatIfBindTbl[i].dwRcvPcktCnt++;
        for (j = 0; j < MAX_PRTCL_PER_IF; j++)
        {
            if (0 != gatIfBindTbl[i].atProtocol[j].name[0])
            {
                byFeatureInPrtcl = 0;
                for (k = 0; k < MAX_FEATURE_PER_PRTCL; k++)
                {
                    if (0 != gatIfBindTbl[i].atProtocol[j].atFeature[k].ftLen)
                    {
                        if ((gatIfBindTbl[i].atProtocol[j].atFeature[k].ftOffset +
                                gatIfBindTbl[i].atProtocol[j].atFeature[k].ftLen) <= dwDataLen)
                        {
                            if (0 ==
                                    memcmp(byData + gatIfBindTbl[i].atProtocol[j].atFeature[k].ftOffset,
                                           gatIfBindTbl[i].atProtocol[j].atFeature[k].ftValue,
                                           gatIfBindTbl[i].atProtocol[j].atFeature[k].ftLen))
                            {
                                byFeatureInPrtcl++;
                            }
                            else
                                break;
                        }
                        else
                            break;
                    }
                }

                if ((MAX_FEATURE_PER_PRTCL == k) && byFeatureInPrtcl)
                {
                    gatIfBindTbl[i].atProtocol[j].dwRcvPcktCnt++;
                    if (NULL != gatIfBindTbl[i].atProtocol[j].pReceiveReturn)
                    {
                        gtGmacStat.dwPcktToCallBack++;
                        gatIfBindTbl[i].atProtocol[j].pReceiveReturn((unsigned char *) byData,dwDataLen);
                    }
                    return;
                }
            }
        }

        gatIfBindTbl[i].dwRcvPcktNoFndPrtclCnt++;
    }
    return;
}


int g_caphandletask=0;
unsigned int g_eth0recvpackcnt=0;
unsigned int g_eth1recvpackcnt=0;
unsigned int g_eth2recvpackcnt=0;
unsigned int g_eth3recvpackcnt=0;



typedef struct mac_data_count
{
    unsigned char *mac;
    unsigned int count;
} MAC_COUNT;

MAC_COUNT mac_count[] =
{
    {gaucoremac[0], 0},
    {gaucoremac[1], 0},
    {gaucoremac[2], 0},
    {gaucoremac[3], 0},
    {gaucoremac[4], 0},
    {gaucoremac[5], 0},
    {gaucoremac[6], 0},
    {gaucoremac[7], 0},
    {gaucoremac[8], 0},
    {gaucoremac[9], 0},
    {gaucoremac[10], 0},
    {gaucoremac[11], 0},
    {gaucoremac[12], 0},
    {gaucoremac[13], 0},
    {gaucoremac[14], 0},
    {gaucoremac[15], 0}
};

#define SIZE_OF_MAC_COUNT  sizeof(mac_count)/sizeof(MAC_COUNT)

void print_mac_count(void)
{
    unsigned int i = 0;
    for(i=0; i<SIZE_OF_MAC_COUNT; i++)
    {
        printf("[0x%x]->[%d]\n", mac_count[i].mac[5], mac_count[i].count);
    }
}
pthread_mutex_t cpudsp_lock = PTHREAD_MUTEX_INITIALIZER;

int print_buff_en = 0;
#if 1
/* 定义任务ID */
#define TASK_EMAC_MSG_PROC    0x0001
#define TASK_EMAC_RECV_TEST   0x0002
#define TASK_EMAC_TRAN_TEST   0x0003
#define TASK_SRIO_TEST        0x0004
#define TASK_AIF_TEST         0x0005
#define TASK_DDR_TEST         0x0006
#define TASK_EDMA_TEST        0x0007
#define TASK_CP_TEST          0x0008
#define TASK_EMAC_REPORT_PPC  0x0009
#define TASK_DUMMY_TEST       0x0020

/* 定义测试用例编号 */
typedef enum
{
    //DSP_TEST_NUM0,
    DSP_TEST1  = 1,
    DSP_TEST2,
    DSP_TEST3,
    DSP_TEST4,
    DSP_TEST5,
    DSP_TEST6,
    DSP_TEST7,

    //DSP_TEST_MAX,
} BSP_TESTNUM;


#define DSP_EMAC_TRAN_TEST1     ((TASK_EMAC_TRAN_TEST << 8) + DSP_TEST1)
#define DSP_EMAC_TRAN_TEST2     ((TASK_EMAC_TRAN_TEST << 8) + DSP_TEST2)
#define DSP_EMAC_TRAN_TEST3     ((TASK_EMAC_TRAN_TEST << 8) + DSP_TEST3)

#define DSP_SRIO_TEST1          ((TASK_SRIO_TEST << 8) + DSP_TEST1)
#define DSP_SRIO_TEST2          ((TASK_SRIO_TEST << 8) + DSP_TEST2)

#define DSP_AIF_TEST1           ((TASK_AIF_TEST << 8)  + DSP_TEST1)

#define DSP_DDR3_TEST1          ((TASK_DDR_TEST << 8)  + DSP_TEST1)

#define DSP_EDMA_TEST1          ((TASK_EDMA_TEST << 8)  + DSP_TEST1)

#define DSP_CP_TEST1            ((TASK_CP_TEST << 8)  + DSP_TEST1)

/* 测试命令消息 */
#define	DSP_TEST_COMMAND	0x1111
/* eth数据报文 */
#define	DSP_DATA_PACKET		0x2222
/* DSP启动成功消息 */
#define	DSP_BOOT_SUCCES		0x3333
/* DSP心跳 */

unsigned char g_eth0bufffortest[MAX_PACK_LEN]= {0};
u32 u32CpuToDspCount=1;
int testtimeflag=0;
unsigned int testtimes=0;
typedef struct
{
    UINT8  DstMacAddr[6];    //目的Mac地址
    UINT8  SrcMacAddr[6];    //源Mac地址
    UINT16 MacType;	     //无用
    UINT16 Rsv;
    UINT16 MsgType;          //消息类型，用来指示出报文是测试命令消息，eth数据报文或DSP启动成功消息
    UINT16 TestID;           //测试用例编号，指示出DSP具体的测试用例
    UINT32 udEmacMsgIdx;     //CPU下发下来的报文中携带字段，DSP上报时原样返回
    UINT32 Result;           //测试结果，指示出该测试成功或失败
    UINT32 SoftInfoLen;      //DSP平台输出软信息长度
} T_EmacTest_MsgHead;

typedef struct
{
    UINT16 MsgType;          //消息类型，用来指示出报文是测试命令消息，eth数据报文或DSP启动成功消息
    UINT16 TestID;           //测试用例编号，指示出DSP具体的测试用例
    UINT32 udEmacMsgIdx;     //CPU下发下来的报文中携带字段，DSP上报时原样返回
    UINT32 Result;           //测试结果，指示出该测试成功或失败
    UINT32 SoftInfoLen;      //DSP平台输出软信息长度
} t_Msgfortest;

typedef struct
{
    int slotid;
    int dspid;
    int coreid;
} t_DspInfo;
t_DspInfo stDspInfo;
void bsp_set_dsp_info(int slotid,  int dspid, int coreid)
{
    stDspInfo.slotid = slotid;
    stDspInfo.dspid = dspid;
    stDspInfo.coreid = coreid;
}

int bsp_cpudsp_starttest_emac_pkt(T_EmacTest_MsgHead *pmsghead, int testid)
{
    pmsghead->MsgType = DSP_TEST_COMMAND;
    pmsghead->TestID = testid;
    pmsghead->MacType = 0xabcd;
}
unsigned int send_bytes=0;
int BspSendIfDataForTest(unsigned long ifBindTblIndex, char *pbyData, int dwLen)
{
    struct ifreq    ifstruct;
    char           *pBuf = NULL;
    unsigned char pucAddr[6]= {0};
    unsigned char pbyTmpData[16]= {0};
    int icnttmp=0;
    unsigned char *sendbufto;
    t_Msgfortest msg;
    int sendlen;
    if ((ifBindTblIndex >= MAX_IFBIND_NUM) || (NULL == pbyData) || (0 == dwLen))
    {
        return GMAC_COMM_ERR_PARAMERR;
    }

    struct sockaddr_ll dest;
    CHAR  ifName[20] = {0};

    snprintf(ifName, 20, "%s%d", gatIfBindTbl[ifBindTblIndex].tIf.name,gatIfBindTbl[ifBindTblIndex].tIf.unit);

    //pbytmpData
    if ( 1 == ifBindTblIndex )
    {
    }
    if ( 0 == ifBindTblIndex )
    {
        memset(g_eth0bufffortest,0,MAX_PACK_LEN);
        memcpy(g_eth0bufffortest, pbyData, dwLen);

        switch (((T_EmacTest_MsgHead*)(pbyData))->MsgType)
        {
        case DSP_TEST_COMMAND:
            if(send_bytes!=0)
            {
#if 0
                sendbufto = (unsigned char*)malloc(send_bytes);
                memset(sendbufto, 0xa5, send_bytes);
                memcpy(g_eth0bufffortest+dwLen, sendbufto, send_bytes);
                free(sendbufto);
                sendbufto=NULL;
#else
                if(send_bytes>4)
                    memcpy(g_eth0bufffortest+dwLen, &testtimes, 4);
#endif
            }
            if (stDspInfo.dspid ==  0x01)
            {
                memcpy(gaucCpuDspPacketHead+6,g_eth0auMacAddr,6);
                memcpy(gaucCpuDspPacketHead,&gstcoremac[stDspInfo.slotid].mac[(0+stDspInfo.coreid)*6],6);
                memcpy(g_eth0bufffortest,gaucCpuDspPacketHead,12);
            }
            else if(stDspInfo.dspid == 0x2)
            {
                memcpy(gaucCpuDspPacketHead+6,g_eth0auMacAddr,6);
                memcpy(gaucCpuDspPacketHead,&gstcoremac[stDspInfo.slotid].mac[(4+stDspInfo.coreid)*6],6);
                memcpy(g_eth0bufffortest,gaucCpuDspPacketHead,12);
            }
            else if(stDspInfo.dspid == 0x3)
            {
                memcpy(gaucCpuDspPacketHead+6,g_eth0auMacAddr,6);
                memcpy(gaucCpuDspPacketHead,&gstcoremac[stDspInfo.slotid].mac[(8+stDspInfo.coreid)*6],6);
                memcpy(g_eth0bufffortest,gaucCpuDspPacketHead,12);
            }
            else if(stDspInfo.dspid == 0x4)
            {
                memcpy(gaucCpuDspPacketHead+6,g_eth0auMacAddr,6);
                memcpy(gaucCpuDspPacketHead,&gstcoremac[stDspInfo.slotid].mac[(12+stDspInfo.coreid)*6],6); //review
                memcpy(g_eth0bufffortest,gaucCpuDspPacketHead,12);
            }
            break;
        case DSP_DATA_PACKET:

            break;
        case DSP_BOOT_SUCCES:
            break;
        default:
            printf("unknown cpu to dsp msg type.\n");
            break;
        }

    }
    if ( 3 == ifBindTblIndex )
    {
    }

    BSP_S_strcpy(ifstruct.ifr_name, 20, ifName);
    ioctl(gatIfBindTbl[ifBindTblIndex].sockRcv, SIOCGIFINDEX, &ifstruct);
    memset(&dest, 0, sizeof(dest));
    dest.sll_family = AF_PACKET;
    dest.sll_protocol = htons(ETH_P_ALL);
    dest.sll_ifindex = ifstruct.ifr_ifindex;
    dest.sll_halen = 6;
    //printf("send ifName->%s,pBuf->%s\r\n",ifName,pBuf);
    if ( 0 == ifBindTblIndex )
    {

        if( (((T_EmacTest_MsgHead*)(pbyData))->MsgType)==DSP_TEST_COMMAND)
        {
            sendlen = dwLen+send_bytes;
#if 1
            if(testtimeflag == 0)
            {
                printf("send data to dsp%d:\n",stDspInfo.dspid);
                {
                    int ii=0;
                    for(ii=0; ii<sendlen; ii++)
                        printf("%02x ",g_eth0bufffortest[ii]);
                    printf("\n");
                }
            }
#endif
        }
        else if( (((T_EmacTest_MsgHead*)(pbyData))->MsgType)==DSP_DATA_PACKET)
        {
            sendlen = dwLen;
        }
        if (sendto(gatIfBindTbl[ifBindTblIndex].sockSend, g_eth0bufffortest, sendlen, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
        {
            gatIfBindTbl[ifBindTblIndex].dwSndPcktCnt++;
            //free(pBuf);
            return BSP_SUCCESS;
        }
        else
        {
            perror("send error!\n");
            //free(pBuf);
            return GMAC_COMM_ERR_IF_SEND_ERR;
        }
    }
    else if(1 == ifBindTblIndex)
    {
        if (sendto(gatIfBindTbl[ifBindTblIndex].sockSend, g_eth1buff, dwLen+16, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
        {
            gatIfBindTbl[ifBindTblIndex].dwSndPcktCnt++;
            //free(pBuf);
            return BSP_SUCCESS;
        }
        else
        {
            perror("send error!\n");
            //free(pBuf);
            return GMAC_COMM_ERR_IF_SEND_ERR;
        }
    }
    else if(3 == ifBindTblIndex)
    {
        if (sendto(gatIfBindTbl[ifBindTblIndex].sockSend, pbyData, dwLen, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
        {
            gatIfBindTbl[ifBindTblIndex].dwSndPcktCnt++;
            //free(pBuf);
            return BSP_SUCCESS;
        }
        else
        {
            perror("send error!\n");
            //free(pBuf);
            return GMAC_COMM_ERR_IF_SEND_ERR;
        }
    }
    else
    {
        return GMAC_COMM_ERR_IF_SEND_ERR;
    }
}
struct cpudspres
{
    int slotid;
    int dspid;
    int res;
    int data_len;      /* dsp测试结果长度*/   
    char result[500]; /* dsp返回测试结果 */
};
struct cpudsptestcont
{
    int testid;
    char *name;
    sem_t *sem;
    unsigned int timeout;
};

#define MAX_DSP_TEST_NUMBER 12
extern struct cpudsptestcont strCpuDspTestCont[MAX_DSP_TEST_NUMBER];
extern int find_cpudsp_test_item(int id);

struct cpudspres cpudspresult[4];
unsigned int cpudsp1pktcnt=0;
unsigned int cpudsp2pktcnt=0;
unsigned int cpudsp3pktcnt=0;
unsigned int cpudsp4pktcnt=0;
pthread_mutex_t send_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t recv_lock = PTHREAD_MUTEX_INITIALIZER;
unsigned int cpudspcycle=0;
unsigned int testpktcntok=0;
unsigned int testpktcnterr=0;
unsigned int testpktcnttimeout=0;

void dsp_boot_check(int slot, unsigned char *mac)
{
    if(memcmp(mac, &gstcoremac[slot].mac[0*6], 6) == 0)
    {
        printf("slot%d dsp1 ready.\n", slot);
        boards[slot].dsp_isready |= 0x1;
        return;
    }
    if(memcmp(mac, &gstcoremac[slot].mac[4*6], 6) == 0)
    {
        printf("slot%d dsp2 ready.\n", slot);
        boards[slot].dsp_isready |= 0x2;
        return;
    }
    if(memcmp(mac, &gstcoremac[slot].mac[8*6], 6) == 0)
    {
        printf("slot%d dsp3 ready.\n", slot);
        boards[slot].dsp_isready |= 0x4;
        return;
    }
    if(memcmp(mac, &gstcoremac[slot].mac[12*6], 6) == 0)
    {
        printf("slot%d dsp4 ready.\n", slot);
        boards[slot].dsp_isready |= 0x8;
        return;
    }

    return;
}

void bsp_cputodsp_callback(unsigned char *pbuf,int len)
{
    int res = -1;
    short type;
    unsigned short testid;
    int i;
    unsigned char mac[8];
    unsigned char slot;
    int index = 0;

    if(pbuf == NULL)
    {
        printf("recv dsp to cpu buff is null,error.\n");
        return;
    }
    pthread_mutex_lock(&recv_lock);
#if 0
    if( print_buff_en==1)
    {
        printf("receive data from dsp:\n");
        int i;
        for(i=0; i<len; i++)
            printf("%02x ",pbuf[i]);
        printf("\n");
    }
#endif
    if(0 == memcmp(pbuf + 6, g_eth0auMacAddr, 6))
    {
        pthread_mutex_unlock(&recv_lock);
        return;
    }
    memcpy(&type, pbuf+16, 2);
    memcpy(&testid, pbuf+18, 2);
    testid |= 0xd000;
    if(type == DSP_TEST_COMMAND)
    {
        memcpy(mac, pbuf+6, 6);
        slot = mac[4];
        if(memcmp(pbuf+6, &gstcoremac[slot].mac[0*6], 6) == 0)
        {
            cpudspresult[0].dspid = 1;            
            memcpy(&(cpudspresult[0].res), pbuf+24, 4);
            strcpy(cpudspresult[0].result, pbuf+32);
            printf("receive data from slot(%d) dsp(%d) command 0x%x:\n",slot,cpudspresult[0].dspid,type);
            for(i=0; i<len; i++)
                printf("%02x ",pbuf[i]);
            printf("\n");
        }
        if(memcmp(pbuf+6, &gstcoremac[slot].mac[4*6], 6) == 0)
        {
            cpudspresult[1].dspid = 2;
            memcpy(&(cpudspresult[1].res), pbuf+24, 4);
            strcpy(cpudspresult[1].result, pbuf+32);
            printf("receive data from slot(%d) dsp(%d) command 0x%x:\n",slot,cpudspresult[1].dspid,type);
            for(i=0; i<len; i++)
                printf("%02x ",pbuf[i]);
            printf("\n");
        }
        if(memcmp(pbuf+6, &gstcoremac[slot].mac[8*6], 6) == 0)
        {
            cpudspresult[2].dspid = 3;
            memcpy(&(cpudspresult[2].res), pbuf+24, 4);
            strcpy(cpudspresult[2].result, pbuf+32);
            printf("receive data from slot(%d) dsp(%d) command 0x%x:\n",slot,cpudspresult[2].dspid,type);
            for(i=0; i<len; i++)
                printf("%02x ",pbuf[i]);
            printf("\n");
        }
        if(memcmp(pbuf+6, &gstcoremac[slot].mac[12*6], 6) == 0)
        {
            cpudspresult[3].dspid = 4;
            memcpy(&(cpudspresult[3].res), pbuf+24, 4);
            strcpy(cpudspresult[3].result, pbuf+32);
            printf("receive data from slot(%d) dsp(%d) command 0x%x:\n",slot,cpudspresult[3].dspid,type);
            for(i=0; i<len; i++)
                printf("%02x ",pbuf[i]);
            printf("\n");
        }
#if 1
        index = find_cpudsp_test_item(testid);
        if(index < 0)
        {
            pthread_mutex_unlock(&recv_lock);
            return;
        }
        if((testid == TEST_BBP_AIF) || (testid == TEST_BBP_UP) || (testid == TEST_BBP_DOWN))
        {
            if((cpudspresult[0].dspid == 1)
                    &&(cpudspresult[1].dspid == 2)
                    &&(cpudspresult[2].dspid == 3)
              )
            {
                cpudsp1pktcnt=cpudsp2pktcnt=cpudsp3pktcnt=cpudsp4pktcnt=0;
                sem_post(strCpuDspTestCont[index].sem);
            }
        }
        else
        {
            if((cpudspresult[0].dspid == 1)
                    &&(cpudspresult[1].dspid == 2)
                    &&(cpudspresult[2].dspid == 3)
                    &&(cpudspresult[3].dspid == 4))
            {
                cpudsp1pktcnt=cpudsp2pktcnt=cpudsp3pktcnt=cpudsp4pktcnt=0;
                sem_post(strCpuDspTestCont[index].sem);
            }
        }
#endif
    }
    else if(type == DSP_DATA_PACKET)
    {
        unsigned char mac1[8];
        unsigned char mac2[8];
        memcpy(mac, pbuf+6, 6);
        slot = mac[4];
        if(!memcmp(pbuf+6, &gstcoremac[slot].mac[0*6], 6))
            cpudsp1pktcnt++;
        if(!memcmp(pbuf+6, &gstcoremac[slot].mac[4*6], 6))
            cpudsp2pktcnt++;
        if(!memcmp(pbuf+6, &gstcoremac[slot].mac[8*6], 6))
            cpudsp3pktcnt++;
        if(!memcmp(pbuf+6, &gstcoremac[slot].mac[12*6], 6))
            cpudsp4pktcnt++;
        memcpy(mac1, pbuf, 6);
        memcpy(mac2, pbuf+6, 6);
        memcpy(pbuf, mac2, 6);
        memcpy(pbuf+6, mac1, 6);

        pthread_mutex_lock(&send_lock);
        res = BspSendIfDataForTest(0, pbuf, len);
        pthread_mutex_unlock(&send_lock);
    }
    else if(type == DSP_BOOT_SUCCES)
    {
        memcpy(mac, pbuf+6, 6);
        slot = mac[4];
        dsp_boot_check(slot, mac);
    }
    else
    {
        //printf("unknown cmd.\n");
    }
    pthread_mutex_unlock(&recv_lock);
}

int cpu_dsp_flag=0;
sem_t  g_cpu_dsp_sem1;
void bsp_cputodsptest_callback(unsigned char *pbuf,int len)
{
    int res = -1;
    short type;
    if(pbuf == NULL)
    {
        printf("recv dsp to cpu buff is null,error.\n");
        return;
    }
    if( print_buff_en==1)
    {
        printf("receive data from dsp:\n");
        int i;
        for(i=0; i<len; i++)
            printf("%02x ",pbuf[i]);
        printf("\n");
    }
    if(!memcmp(pbuf+32, &testtimes,4))
        sem_post(&g_cpu_dsp_sem1);
    else
    {
        testpktcnterr++;
    }
}

#endif
void   cap_handle_task(int arg)
{
    int             ifBindTblIndex;
    UCHAR   rbuff[8*1024] = { 0 };
    unsigned long   len = 0;
    unsigned int retaddr=0;
    unsigned int retlen=0;
    ifBindTblIndex = arg;
    int icnttmp=0;
    unsigned long cpumask;
#if 1
    if (arg == 3)
    {
        cpumask = (1 << 3);
        if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
        {
            printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
            return ;
        }
    }
    bsp_print_reg_info(__func__, __FILE__, __LINE__);
#endif
    while (1)
    {
        if (gatIfBindTbl[ifBindTblIndex].isBind == 0)
            return;
        len = recvfrom(gatIfBindTbl[ifBindTblIndex].sockRcv,rbuff, sizeof(rbuff), 0, NULL, NULL);
        if (len < 0)
        {
            perror("Failed to receive packet");
            continue;
        }
#if 1
        if( print_buff_en==1)
        {
            if((ifBindTblIndex == 0) && (rbuff[12] ==0xab)&& (rbuff[13] ==0xcd))
            {
                printf("receive data from dsp----:\n");
                int i;
                for(i=0; i<len; i++)
                    printf("%02x ",rbuff[i]);
                printf("\n");
            }
            if((ifBindTblIndex == 0) && (rbuff[12] ==0xc1)&& (rbuff[13] ==0xc2))
            {
                printf("receive data from dsp----:\n");
                int i;
                for(i=0; i<len; i++)
                    printf("%02x ",rbuff[i]);
                printf("\n");
            }
        }
#endif

        {
#if 0
            if(!EnQueue(pQueue,rbuff,len))
            {
                printf("EnQueue FULL!\n");
                //return;
            }
#else
            if (len <=8192)
            {
                if (ifBindTblIndex == 0)
                {
                    if(0 != memcmp(rbuff + 6, g_eth0auMacAddr, 6))
                    {
                        if ((ifBindTblIndex == 0) && (rbuff[12] ==0xaa) && (rbuff[13] ==0xbb))
                        {
                            g_eth0recvpackcnt++;
                            if((rbuff[14]==0xaa) && (rbuff[15]==0xaa) && (rbuff[11]>=0x30)&&(rbuff[11]<=0x3f) \
                                    && (rbuff[6]==0x30)&&(rbuff[7]==0x31)&&(rbuff[8]==0x32)&&(rbuff[9]==0x33)&&(rbuff[10]==0x34) )
                            {
                                int index = 0;
                                index = (rbuff[11]-0x30);
                                mac_count[index].count++;
                            }
                        }
                        if((ifBindTblIndex == 0) && (rbuff[12] ==0xab)&& (rbuff[13] ==0xcd))
                        {
                            u32CpuToDspCount++;
                        }
                        MacStackRcvRtn(ifBindTblIndex, (unsigned char *)rbuff, len);
                    }
                }
                if (ifBindTblIndex == 1)
                {
                    if(0 != memcmp(rbuff + 6, g_eth1auMacAddr, 6))
                    {
                        if ((ifBindTblIndex == 1) && (rbuff[12] ==0xaa) && (rbuff[13] ==0xaa))
                        {
                            g_eth1recvpackcnt++;

                        }
                        MacStackRcvRtn(ifBindTblIndex, (unsigned char *)rbuff, len);
                    }
                }
                if (ifBindTblIndex == 3)
                {
                    if(0 != memcmp(rbuff + 6, g_eth3auMacAddr, 6))
                    {
                        if ((ifBindTblIndex == 3) && (rbuff[12] ==0x08) && (rbuff[13] ==0x00))
                        {
                            g_eth3recvpackcnt++;

                        }
                        MacStackRcvRtn(ifBindTblIndex, (unsigned char *)rbuff, len);
                    }
                }
            }
#endif
        }
    }
}

int IsIfExists(unsigned char *name, unsigned char unit)
{
    return 1;
}

CHAR *BSP_S_strcpy( CHAR *pcDst, WORD32 dwMaxSize, const CHAR *pcSrc )
{
    WORD32  i = 0;
    CHAR   *pcResult = pcDst;
    if ( ( pcDst == NULL ) || ( pcSrc == NULL ) || ( dwMaxSize == 0 ) )
    {
        return pcResult;
    }
    while ( ( i++ < dwMaxSize ) && ( '\0' != ( *pcDst++ = *pcSrc++ ) ) )
    {
    }
    if ( i >= dwMaxSize )
    {
        *( pcResult + dwMaxSize - 1 ) = '\0';
    }
    return pcResult;
}

int BspBindIfMuxProInit(T_IF * ptIf)
{
    pthread_t       a_thread;
    pthread_attr_t  attr;
    struct sched_param parm;

    void           *thread_result;
    int             res;
    int             i;
    int             ret = 0;
    void           *arg;
    struct sockaddr_ll sll;
    struct ifreq    ifstruct;
    CHAR          ifName[20] = { 0 };
    int optval = 0;
    pthread_t ptid;

    if (NULL == ptIf)
    {
        return GMAC_COMM_ERR_PARAMERR;
    }
    for (i = 0; i < MAX_IFBIND_NUM; i++)
    {
        if (gatIfBindTbl[i].isBind)
        {
            if ((0 == strcmp(gatIfBindTbl[i].tIf.name, ptIf->name)) &&
                    (gatIfBindTbl[i].tIf.unit == ptIf->unit))
                break;
        }
    }
    if (i == MAX_IFBIND_NUM)
    {
        if (0 == IsIfExists(ptIf->name, ptIf->unit))
        {
            return GMAC_COMM_ERR_IF_NOEXIST;
        }
        else
        {
            for (i = 0; i < MAX_IFBIND_NUM; i++)
            {
                if (!gatIfBindTbl[i].isBind)
                    break;
            }
            if (i == MAX_IFBIND_NUM)
            {
                return GMAC_COMM_ERR_IFTBL_FULL;
            }
            else
            {
                int sock_rcv = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
                if (sock_rcv == -1)
                {
                    perror("sock_rcv");
                    return GMAC_COMM_ERR_SKERROR;
                }
                int sock_snd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
                if (sock_snd == -1)
                {
                    perror("sock_snd");
                    return GMAC_COMM_ERR_SKERROR;
                }

                memset(&sll, 0, sizeof(sll));
                sll.sll_family = AF_PACKET;
                sprintf(ifName, "%s%d", ptIf->name, ptIf->unit);
                BSP_S_strcpy(ifstruct.ifr_name, 20, ifName);
                ioctl(sock_rcv, SIOCGIFINDEX, &ifstruct);
                sll.sll_ifindex = ifstruct.ifr_ifindex;
                sll.sll_protocol = htons(ETH_P_ALL);
                if (bind(sock_rcv, (struct sockaddr *) &sll, sizeof(sll)) == -1)
                {
                    perror("bind");
                    return GMAC_COMM_ERR_SKERROR;
                }
                optval = 1;
                setsockopt(sock_rcv, SOL_PACKET, PACKET_NROP, &optval, sizeof(int));
                setsockopt(sock_snd, SOL_PACKET, PACKET_NROP, &optval, sizeof(int));

                gatIfBindTbl[i].tIf = *ptIf;
                gatIfBindTbl[i].sockRcv = sock_rcv;
                gatIfBindTbl[i].sockSend = sock_snd;
                gatIfBindTbl[i].isBind = 1;
                //printf("gatIfBindTbl[%d]\n",i);
                /*???ˉ?óê???3ì */
                pthread_attr_init(&attr);
#if 0
                pthread_attr_setschedparam(&attr, PTHREAD_CREATE_DETACHED);
                res = pthread_create(&ptid, &attr, (FUNCPTR)cap_handle_task, i);
                // res = pthread_create(&ptid, NULL, (FUNCPTR)cap_handle_task, i);
                //pthread_attr_destory(&attr);
#endif
                pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
                pthread_attr_setstacksize(&attr, 1024*1024);
                pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
                parm.sched_priority = 49;
                pthread_attr_setschedparam(&attr, &parm);

                res = pthread_create(&ptid, &attr, (FUNCPTR)cap_handle_task, i);
                pthread_attr_destroy(&attr);
                if (ERROR == res)
                {
                    perror("tCapHandleTask");
                    return GMAC_COMM_ERR_SKERROR;
                }
                int     status = ioctl(sock_rcv, SIOCSIFMEM, gatIfBindTbl);
                if (-1 == status)
                {
                    memset(&gatIfBindTbl[i], 0, sizeof(T_IFBIND));
                    perror("setsockopt(...,SO_REUSEADDR,...)");
                    return GMAC_COMM_ERR_SKERROR;
                }
                return BSP_SUCCESS;
            }
        }
    }
    else
    {
        return BSP_SUCCESS;
    }
}

int BspIfBindInit(T_IF * ptIf)
{
    /*2?êy?ì2é */
    if (NULL == ptIf)
    {
        return GMAC_COMM_ERR_PARAMERR;
    }
    return BspBindIfMuxProInit(ptIf);
}

int BspRegisterProtocol(T_PROTOCOL * ptProtocol)
{
#if 1
    int             i, j, k;
    int             ret = BSP_SUCCESS;
    if ((0 == strcmp(ptProtocol->name, "")) || (0 == ptProtocol->atFeature[0].ftLen))//|| (0 == ptProtocol->pReceiveReturn))
    {
        return GMAC_COMM_ERR_PARAMERR;
    }

    for (i = 0; i < MAX_PRTCL_NUM; i++)
    {
        if (0 != strcmp(gatIfPrtclTbl[i].name, ""))
        {
            if (0 == strcmp(gatIfPrtclTbl[i].name, ptProtocol->name))
            {

                ret = BspIfBindInit(&(gatIfPrtclTbl[i].tIf));
                if (BSP_SUCCESS != ret)
                {

                    return ret;
                }
                else
                {

                    for (j = 0; j < MAX_IFBIND_NUM; j++)
                    {
                        if (gatIfBindTbl[j].isBind)
                        {
                            if ((0 == strcmp(gatIfBindTbl[j].tIf.name, gatIfPrtclTbl[i].tIf.name)) && (gatIfBindTbl[j].tIf.unit == gatIfPrtclTbl[i].tIf.unit))
                                break;
                        }
                    }
                    if (MAX_IFBIND_NUM == j)
                    {
                        return GMAC_COMM_ERR_IF_NOTBIND;
                    }
                    else
                    {
                        for (k = 0; k < MAX_PRTCL_PER_IF; k++)
                        {
                            if (0 == gatIfBindTbl[j].atProtocol[k].name[0])
                            {
                                break;
                            }
                        }
                        if (MAX_PRTCL_PER_IF == k)
                        {
                            return GMAC_COMM_ERR_IF_PRTCLFULL;
                        }
                        else
                        {
                            gatIfBindTbl[j].atProtocol[k] = *ptProtocol;
                            gatIfBindTbl[j].byRegPrtclNum++;

                            int status = ioctl(gatIfBindTbl[j].sockRcv, SIOCSIFMEM, gatIfBindTbl);
                            if (-1 == status)
                            {
                                memset(&gatIfBindTbl[j].atProtocol[k], 0, sizeof(T_PROTOCOL));
                                gatIfBindTbl[j].byRegPrtclNum--;
                                perror("setsockopt(...,SO_REUSEADDR,...)");
                                return GMAC_COMM_ERR_SKERROR;
                            }
                            return BSP_SUCCESS;
                        }
                    }
                }
            }
        }
        else
            break;
    }
    return GMAC_COMM_ERR_PRTCL_NOTSUPPORT;
#endif
}

void BspEthSwPort1CallBack(unsigned char *pbyReceiveData, unsigned long dwDataLen)
{
    int             i;

    UCHAR ucaTempMac[6] = {0};
    printf("loading fGmaccTestCallBack0!\n");
    for (i = 0; i < dwDataLen; i++)
    {
        if (0 == (i % 0x10))
            printf("\n");
        printf("%02x ", pbyReceiveData[i]);
    }
    printf("\n");

}

void BspEthSwPort2CallBack(unsigned char *pbyReceiveData, unsigned long dwDataLen)
{
    int             i;
    UCHAR ucaTempMac[6] = {0};
    printf("loading fGmaccTestCallBack1!\n");
    for (i = 0; i < dwDataLen; i++)
    {
        if (0 == (i % 0x10))
            printf("\n");
        printf("%02x ", pbyReceiveData[i]);
    }
    printf("\n");
}

void BspEthDebugCallBack(unsigned char *pbyReceiveData, unsigned long dwDataLen)
{
    int             i;

    UCHAR ucaTempMac[6] = {0};
    printf("loading fGmaccTestCallBack2!\n");
    for (i = 0; i < dwDataLen; i++)
    {
        if (0 == (i % 0x10))
            printf("\n");
        printf("%02x ", pbyReceiveData[i]);
    }
    printf("\n");

}

int icorecallback=0;

void BspEthCoreNetCallBack( unsigned char *pbyReceiveData, unsigned long dwDataLen)
{
    unsigned int retaddr=0;
    unsigned int retlen=0;
    int 			i;
    UCHAR ucaTempMac[6] = {0};
    icorecallback++;
}

/***********************************************************************
*函数名称：BspRecvHostStandbySwitchSmg
*函数功能：接收主板切换消息
*函数参数：  参数名                    描述
*       
*函数返回：发送成功或失败
***********************************************************************/
extern u32 g_u32MasterSlaveSwitchCause;
int g_print_MS_switch_flag = 0;
void BspRecvHostStandbySwitchSmg(unsigned char *pbyReceiveData, unsigned long dwDataLen)
{
	u32 tmp_u32_MS_switch_cause;
	/*接收消息打印*/
	int i;
	if(1 == g_print_MS_switch_flag)
	{
		printf("host-- standby switch data :\n");
		for (i = 0; i < dwDataLen; i++)
		{
			if (0 == (i % 0x10))
			  printf("\n");
			  	
			printf("%02x ", pbyReceiveData[i]);
		}
		printf("\n");
	}
	memcpy(&tmp_u32_MS_switch_cause,pbyReceiveData+15,4);
	
	if(MCT_SLAVE == *(pbyReceiveData+14))	//接收降备消息
	{
		if(MCT_AVIABLE == bsp_get_opp_available() && MCT_MASTER == bsp_get_self_MS_state())
		{
			bsp_send_boards_status_info();
			usleep(100);
			g_u32MasterSlaveSwitchCause = tmp_u32_MS_switch_cause;
			bsp_set_self_slave();
		}
	}
	
	if(MCT_MASTER == *(pbyReceiveData+14))	/*接收升主消息*/
	{
		if(MCT_AVIABLE == bsp_get_self_available())
		{
			g_u32MasterSlaveSwitchCause = tmp_u32_MS_switch_cause;
			for(i = 0;i < 3;i++)
			{
				if(BSP_OK == bsp_send_host_standby_switch_msg(MCT_SLAVE,tmp_u32_MS_switch_cause))
					break;
			}
		}
	}
			
	if(1 == *(pbyReceiveData+14) && 0 == bsp_get_self_MS_state())
	{
		bsp_set_self_master();
	}
	memcpy(&g_u32MasterSlaveSwitchCause,pbyReceiveData+15,4);
}

/***********************************************************************
*函数名称：BspRecvDspAckMasterSlaveSwitchMsg
*函数功能：接收DSP主备切换回应消息
*函数参数：  参数名                    描述
*       
*函数返回：发送成功或失败
***********************************************************************/
#if 0
int g_s32DspAckMSSwitchMsg[6][4] = {0};
int BspRecvDspAckMasterSlaveSwitchMsg(unsigned char *pbyReceiveData, unsigned long dwDataLen)
{
	int i;
	int slotid;
	int dspid;
	if(1 == g_print_MS_switch_flag)
	{
		printf("MASTER--SLAVE switch ACK data :\n");
		for (i = 0; i < dwDataLen; i++)
		{
			if (0 == (i % 0x10))
			  printf("\n");
			  	
			printf("%02x ", pbyReceiveData[i]);
		}
		printf("\n");
	}
	for(slotid = IPMB_SLOT2;slotid <= IPMB_SLOT7;slotid++)
	{
		for(dspid = 0; dspid < 4;dspid ++)
		{
			if(0 == memcmp(pbyReceiveData+6,get_dsp_mac(slotid,dspid,0),6))
			{
				g_s32DspAckMSSwitchMsg[slotid-2][dspid] = 1;
			}
		}
	}
}
#endif

/***********************************************************************
*函数名称：BspRecvBoardsStatusInfo
*函数功能：备板接收从板状态信息
*函数参数：  参数名                    描述
*       
*函数返回：发送成功或失败
***********************************************************************/
u32 g_u32print_boards_status_info = 0;
int BspRecvBoardsStatusInfo(unsigned char *pbyReceiveData, unsigned long dwDataLen)
{
	int i;
	
	if(1 == g_u32print_boards_status_info)
	{
		printf("boards status data,boards len=%d \n",sizeof(boards));
		for (i = 0; i < dwDataLen; i++)
		{
			if (0 == (i % 0x10))
			  printf("\n");
			  	
			printf("%02x ", pbyReceiveData[i]);
		}
		printf("\n");
	}
	
	if( MCT_MASTER == bsp_get_self_MS_state())
	{
		return BSP_ERROR;
	}
	memcpy(&boards,pbyReceiveData+14 ,sizeof(boards));
	memcpy(&g_subboard_reset_over,pbyReceiveData+14+sizeof(boards),4);
	return BSP_OK;
	
}

void BspRegisterRecvData(void)
{
    T_PROTOCOL		tProtocol;
    T_PROTOCOL	 *ptProtocol = &tProtocol;

    memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "ETHSW1");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 2;
    ptProtocol->atFeature[0].ftValue[0] = 0xc1;
    ptProtocol->atFeature[0].ftValue[1] = 0xc2;
    ptProtocol->pReceiveReturn = (FUNCPTR)EmacFromEth0ToDspTest;
    BspRegisterProtocol(ptProtocol);

    memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "ETHSW1");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 2;
    ptProtocol->atFeature[0].ftValue[0] = 0xab;
    ptProtocol->atFeature[0].ftValue[1] = 0xcd;
    ptProtocol->pReceiveReturn = (FUNCPTR)EmacFromEth0ToDsp;
    BspRegisterProtocol(ptProtocol);

    memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "ETHSW1");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 1;
    ptProtocol->atFeature[0].ftValue[0] = 0xaa;
    //ptProtocol->atFeature[0].ftValue[1] = 0xaa;
    ptProtocol->pReceiveReturn = (FUNCPTR)EmacFromEth0ToCpu;
    BspRegisterProtocol(ptProtocol);

    memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "ETHSW1");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 2;
    ptProtocol->atFeature[0].ftValue[0] = 0xbc;
    ptProtocol->atFeature[0].ftValue[1] = 0xbc;
    ptProtocol->pReceiveReturn = (FUNCPTR)EmacFromEth0ToCpu;
    BspRegisterProtocol(ptProtocol);
	#if 0
	memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "ETHSW1");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 2;
    ptProtocol->atFeature[0].ftValue[0] = 0xcd;
    ptProtocol->atFeature[0].ftValue[1] = 0xef;
    ptProtocol->pReceiveReturn = (FUNCPTR)BspRecvDspAckMasterSlaveSwitchMsg;
    BspRegisterProtocol(ptProtocol);
	#endif
	
    memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "ETHSW2");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 2;
    ptProtocol->atFeature[0].ftValue[0] = 0xaa;
    ptProtocol->atFeature[0].ftValue[1] = 0xaa;
    ptProtocol->pReceiveReturn = (FUNCPTR)EmacFromEth1ToCpu;
    BspRegisterProtocol(ptProtocol);
	
	memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "ETHSW2");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 2;
    ptProtocol->atFeature[0].ftValue[0] = 0xab;
    ptProtocol->atFeature[0].ftValue[1] = 0xab;
    ptProtocol->pReceiveReturn = (FUNCPTR)BspRecvBoardsStatusInfo;
    BspRegisterProtocol(ptProtocol);
	
	memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "ETHSW2");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 2;
    ptProtocol->atFeature[0].ftValue[0] = 0xab;
    ptProtocol->atFeature[0].ftValue[1] = 0xba;
    ptProtocol->pReceiveReturn = (FUNCPTR)BspRecvHostStandbySwitchSmg;
    BspRegisterProtocol(ptProtocol);

    memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "DEBUG");
    ptProtocol->atFeature[0].ftOffset = 12;
    ptProtocol->atFeature[0].ftLen = 2;
    ptProtocol->atFeature[0].ftValue[0] = 0xaa;
    ptProtocol->atFeature[0].ftValue[1] = 0xaa;
    ptProtocol->pReceiveReturn = (FUNCPTR)BspEthDebugCallBack;
    BspRegisterProtocol(ptProtocol);

    memset(ptProtocol, 0, sizeof(T_PROTOCOL));
    strcpy(ptProtocol->name, "CORENET");
    ptProtocol->atFeature[0].ftOffset = 34;
    ptProtocol->atFeature[0].ftLen = 4;
    ptProtocol->atFeature[0].ftValue[0] = 0x08;
    ptProtocol->atFeature[0].ftValue[1] = 0x68;
    ptProtocol->atFeature[0].ftValue[2] = 0x08;
    ptProtocol->atFeature[0].ftValue[3] = 0x68;
    ptProtocol->pReceiveReturn = (FUNCPTR)EmacFromEth3ToCpu;//BspEthCoreNetCallBack;
    BspRegisterProtocol(ptProtocol);
    return BSP_SUCCESS;
}

int BspSendEmacData(unsigned char *pProtocolName, char *pbyData, int dwLen)
{
    int             i, j;
    int             ret = BSP_SUCCESS;

    if ((NULL == pProtocolName) || (NULL == pbyData) || (0 == dwLen))
    {
        return GMAC_COMM_ERR_PARAMERR;
    }
    for (i = 0; i < MAX_IFBIND_NUM; i++)
    {
        if (gatIfBindTbl[i].isBind)
        {
            for (j = 0; j < MAX_PRTCL_PER_IF; j++)
            {
                if (0 != gatIfBindTbl[i].atProtocol[j].name[0])
                {
                    if (0 == strcmp(pProtocolName, gatIfBindTbl[i].atProtocol[j].name))
                    {
#if 0
                        ret = BspSendIfData(i, pbyData, dwLen);
#else
                        pthread_mutex_lock(&send_lock);
                        ret = BspSendIfDataForTest(i, pbyData, dwLen);
                        pthread_mutex_unlock(&send_lock);
#endif

                        if (BSP_SUCCESS == ret)
                            gatIfBindTbl[i].atProtocol[j].dwSndPcktOkCnt++;
                        else
                            gatIfBindTbl[i].atProtocol[j].dwSndPcktErrCnt++;
                        return ret;
                    }
                }
            }
        }
    }
    return GMAC_COMM_ERR_PRTCL_NOTFIND;
}

extern unsigned char g_nexthopaddr[128];

extern unsigned int BspGetRemoteMacAddr(char *pdestip);

extern int iremotemacflag;
static time_t lasttime;
#include <signal.h>
#include <sys/time.h>


void bsp_set_remote_mac_addr(void)
{
    //printf("sync mac addr!\n");
    if (1 == iremotemacflag)
    {
        BspGetRemoteMacAddr(g_nexthopaddr);
        //printf("get eth3 mac addr!\n");
        bsp_get_netif_mac_addr((CHAR*)"eth3",g_eth3auMacAddr);
    }
}

void timer_handle_task(int signo)
{
    //struct sigaction act;
    //int len;
    //union sigval tsval;
    //act.sa_handler = bsp_set_remote_mac_addr;
    //act.sa_flags = 0;
    //sigemptyset(&act.sa_mask);
    //sigaction(50,&act,NULL);
    //len = strlen(msg);
    time(&lasttime);

    bsp_print_reg_info(__func__, __FILE__, __LINE__);
    // printf("sync mac addr!\n");
    while(1)
    {
        time_t nowtime;
        time(&nowtime);
        if (nowtime - lasttime >=10)
        {
            //sigqueue(getpid(),50,tsval);
            bsp_set_remote_mac_addr();
            lasttime = nowtime;
        }
        sleep(1);
    }
}

void init_time(void)
{
    // struct itimerval val;
    // val.it_value.tv_sec=1;
    //val.it_value.tv_usec=0;
    //val.it_interval = val.it_value;
    //setitimer(ITIMER_PROF,&val,NULL);
    int res=0;
    pthread_t ptid;
    pthread_t       a_thread;
    pthread_attr_t  attr;
    struct sched_param parm;
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setstacksize(&attr, 1024*1024);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    parm.sched_priority = 30;
    pthread_attr_setschedparam(&attr, &parm);
    res = pthread_create(&ptid, &attr, (FUNCPTR)timer_handle_task,NULL);
    pthread_attr_destroy(&attr);
    if (-1 == res)
    {
        perror("create gps thread error!\n");
    }
}



int BspEth1SendData(unsigned char *pu8Buf,unsigned char *pu8Data,unsigned short u16Len, unsigned short u16UtOffset)
{
    static unsigned char u8InitialFlag = 0;
    static struct sockaddr_ll tUserPlaneSockAddr;
    static struct ifreq	ifstruct;
    int s32Ret = -1;
    int s32Offset = 0;
    unsigned char *pu8SendBuf = NULL;


    if( NULL == pu8Buf || NULL == pu8Data || 0 == u16Len || (pu8Buf > pu8Data))
    {
        s32Ret = BSP_FAIL;
        return BSP_FAIL;
    }

    //初始化发包参数
    if(0 == u8InitialFlag)
    {
        u8InitialFlag = 1;
        //Copy SrcMacAddr
        memcpy(g_au8UserPlaneBuf+6,g_eth1auMacAddr,6);
        memset(&ifstruct , 0 , sizeof(ifstruct));

        //Config Kernal Eth1 Para
        snprintf(ifstruct.ifr_name, 20, "%s%d", gatIfBindTbl[1].tIf.name,gatIfBindTbl[1].tIf.unit);
        ioctl(gatIfBindTbl[1].sockRcv, SIOCGIFINDEX, &ifstruct);

        //Fill in Sendto Struct
        memset(&tUserPlaneSockAddr, 0, sizeof(tUserPlaneSockAddr));
        tUserPlaneSockAddr.sll_family = AF_PACKET;
        tUserPlaneSockAddr.sll_protocol = htons(ETH_P_ALL);
        tUserPlaneSockAddr.sll_ifindex = ifstruct.ifr_ifindex;
        tUserPlaneSockAddr.sll_halen = 6;
    }
    else
    {
        ioctl(gatIfBindTbl[1].sockRcv, SIOCGIFINDEX, &ifstruct);
    }

    s32Offset = (pu8Data - pu8Buf);

    if(s32Offset >= (u16UtOffset + 16))
    {
        pu8SendBuf = (pu8Data - u16UtOffset - 16);
        memcpy( pu8SendBuf, g_au8UserPlaneBuf, 16);
        gatIfBindTbl[1].dwSndPcktCnt +=1;
    }
    else
    {
        pu8SendBuf = g_au8UserPlaneBuf;
        memcpy(g_au8UserPlaneBuf+16+u16UtOffset, pu8Data, u16Len);
    }

    s32Ret = sendto(gatIfBindTbl[1].sockSend, pu8SendBuf, u16Len+16+u16UtOffset, 0,
                    (struct sockaddr *) (&tUserPlaneSockAddr),sizeof(tUserPlaneSockAddr));

    return s32Ret;
}

static int BspEth3SendData( unsigned char *pu8Data,unsigned short u16Len )
{
    static unsigned char u8InitialFlag = 0;
    static struct sockaddr_ll tUserPlaneSockAddr;
    static struct ifreq	ifstruct;
    int s32Ret = -1;
    unsigned char *pu8SendBuf = NULL;

    if(NULL == pu8Data || 0 == u16Len)
    {
        s32Ret = BSP_FAIL;
        return BSP_FAIL;
    }

    if(0 == u8InitialFlag)
    {
        u8InitialFlag = 1;
        memset(&ifstruct , 0 , sizeof(ifstruct));
        snprintf(ifstruct.ifr_name, 20, "%s%d", gatIfBindTbl[3].tIf.name,gatIfBindTbl[3].tIf.unit);
        ioctl(gatIfBindTbl[3].sockRcv, SIOCGIFINDEX, &ifstruct);

        memset(&tUserPlaneSockAddr, 0, sizeof(tUserPlaneSockAddr));
        tUserPlaneSockAddr.sll_family = AF_PACKET;
        tUserPlaneSockAddr.sll_protocol = htons(ETH_P_ALL);
        tUserPlaneSockAddr.sll_ifindex = ifstruct.ifr_ifindex;
        tUserPlaneSockAddr.sll_halen = 6;
    }
    else
    {
        ioctl(gatIfBindTbl[3].sockRcv, SIOCGIFINDEX, &ifstruct);
    }

    memcpy(pu8Data + 6, g_eth3auMacAddr, 6);

    if (pu8Data[12] == 0x81 && pu8Data[13] == 0x00)
    {
        pu8Data[16] = 0x08;
        pu8Data[17] = 0x00;
    }
    else
    {
        pu8Data[12] = 0x08;
        pu8Data[13] = 0x00;
    }

    s32Ret = sendto( gatIfBindTbl[3].sockSend, pu8Data, u16Len, 0,
                     (struct sockaddr *)(&tUserPlaneSockAddr),sizeof(tUserPlaneSockAddr) );

    return s32Ret;
}
int BspSendIfData(unsigned long ifBindTblIndex, char *pbyData, int dwLen)
{
    struct ifreq    ifstruct;
    char           *pBuf = NULL;
    unsigned char pucAddr[6]= {0};
    unsigned char pbyTmpData[16]= {0};
    int icnttmp=0;
    T_MSG_H msg;
    if ((ifBindTblIndex >= MAX_IFBIND_NUM) || (NULL == pbyData) || (0 == dwLen))
    {
        return GMAC_COMM_ERR_PARAMERR;
    }
    if ( 0 == ifBindTblIndex )
    {
        memset(g_eth0buff,0,MAX_PACK_LEN);
        memcpy(g_eth0buff+16, pbyData, dwLen);
        //memcpy(pBuf+16, pbyData, dwLen);
    }
    else if( 1 == ifBindTblIndex )
    {
        memset(g_eth1buff,0,MAX_PACK_LEN);
        memcpy(g_eth1buff+16, pbyData, dwLen);
    }
    else if( 3 ==  ifBindTblIndex )
    {
        //memset(g_eth3buff,0,MAX_PACK_LEN);
        //memcpy(g_eth3buff+14, pbyData, dwLen);
        //memcpy(pBuf+14, pbyData, dwLen);
    }
    else
    {
        return BSP_ERROR;
    }
    struct sockaddr_ll dest;
    CHAR            ifName[20] = { 0 };

    snprintf(ifName, 20, "%s%d", gatIfBindTbl[ifBindTblIndex].tIf.name,gatIfBindTbl[ifBindTblIndex].tIf.unit);

    //pbytmpData
    if ( 1 == ifBindTblIndex )
    {
        memcpy(gaucDataPacketHead+6,g_eth1auMacAddr,6);
        memcpy(g_eth1buff,gaucDataPacketHead,16);
    }
    if ( 0 == ifBindTblIndex )
    {


        memcpy((void *)&msg,(void *)pbyData+8,sizeof(T_MSG_H));
        switch (msg.u16MsgType)
        {
        case DSP2CPU_START_OK:
            break;
        case CPU2DSP_WORK:
            if (msg.s16DevID ==  0x01)
            {
                printf("msg.s16DevId->0x%lx\n",msg.s16DevID);
                memcpy(gaucHeardBeatPacketHead+6,g_eth0auMacAddr,6);
                memcpy(gaucHeardBeatPacketHead,&gaucoremac[0][0],6);
                memcpy(g_eth0buff,gaucHeardBeatPacketHead,16);
            }
            else if(msg.s16DevID == 0x2)
            {
                printf("msg.s16DevId->0x%lx\n",msg.s16DevID);
                memcpy(gaucHeardBeatPacketHead+6,g_eth0auMacAddr,6);
                memcpy(gaucHeardBeatPacketHead,&gaucoremac[4][0],6);
                memcpy(g_eth0buff,gaucHeardBeatPacketHead,16);
            }
            else if(msg.s16DevID == 0x3)
            {
                printf("msg.s16DevId->0x%lx\n",msg.s16DevID);
                memcpy(gaucHeardBeatPacketHead+6,g_eth0auMacAddr,6);
                memcpy(gaucHeardBeatPacketHead,&gaucoremac[8][0],6);
                memcpy(g_eth0buff,gaucHeardBeatPacketHead,16);
            }
            else if(msg.s16DevID == 0x4)
            {
                printf("msg.s16DevId->0x%lx\n",msg.s16DevID);
                memcpy(gaucHeardBeatPacketHead+6,g_eth0auMacAddr,6);
                memcpy(gaucHeardBeatPacketHead,&gaucoremac[12][0],6); //review
                memcpy(g_eth0buff,gaucHeardBeatPacketHead,16);
            }

            break;
        case OAM_MONITER_DSP_DATA_MSG:
        case OAM_MONITER_DSP_STOP_MSG:
        case OAM_MONITOR_EI_BASIC_START_MSG:
        case OAM_MONITOR_EI_SPECIFIC_START_MSG:
        case OAM_MONITOR_EI_ALL_STOP_MSG:
        case OAM_MONITOR_EI_SPECIFIC_STOP_MSG:
            if (msg.s16DevID ==  0x01)
            {
                memcpy(EI_DSP_Monitor_Header + 6, g_eth0auMacAddr, 6);
                memcpy(EI_DSP_Monitor_Header, &gaucoremac[0 + msg.s16CoreNO][0], 6);
                memcpy(g_eth0buff, EI_DSP_Monitor_Header, 16);
            }
            else if (msg.s16DevID == 0x2)
            {
                memcpy(EI_DSP_Monitor_Header + 6, g_eth0auMacAddr, 6);
                memcpy(EI_DSP_Monitor_Header, &gaucoremac[4+ msg.s16CoreNO][0], 6);
                memcpy(g_eth0buff, EI_DSP_Monitor_Header, 16);
            }
            else if (msg.s16DevID == 0x3)
            {
                memcpy(EI_DSP_Monitor_Header + 6, g_eth0auMacAddr, 6);
                memcpy(EI_DSP_Monitor_Header, &gaucoremac[8 + msg.s16CoreNO][0], 6);
                memcpy(g_eth0buff, EI_DSP_Monitor_Header, 16);
            }
            else if (msg.s16DevID == 0x4)
            {
                memcpy(EI_DSP_Monitor_Header + 6, g_eth0auMacAddr, 6);
                memcpy(EI_DSP_Monitor_Header, &gaucoremac[12 + msg.s16CoreNO][0], 6);
                memcpy(g_eth0buff, EI_DSP_Monitor_Header, 16);
            }

            switch (msg.u16MsgType)
            {
            case OAM_MONITER_DSP_DATA_MSG:
            case OAM_MONITER_DSP_STOP_MSG:
                g_eth0buff[12] = 0xbc;
                g_eth0buff[13] = 0xbc;
                break;
            case OAM_MONITOR_EI_BASIC_START_MSG:
            case OAM_MONITOR_EI_SPECIFIC_START_MSG:
            case OAM_MONITOR_EI_ALL_STOP_MSG:
            case OAM_MONITOR_EI_SPECIFIC_STOP_MSG:
                g_eth0buff[12] = 0x9c;
                g_eth0buff[13] = 0x10;
                break;
            }
            break;
        case DSP2CPU_HEARTBEAT:
            break;
        default:
            memcpy(gaucCtlPacketHead+6,g_eth0auMacAddr,6);
            memcpy(gaucCtlPacketHead,&gaucoremac[12][0],6);
            memcpy(g_eth0buff,gaucCtlPacketHead,16);
            break;
        }
        //memcpy(g_eth1buff,gaucCtlPacketHead,16);
    }
    if ( 3 == ifBindTblIndex )
    {
#if 0
        if (BSP_OK != bsp_get_netif_mac_addr((CHAR*)ifName,pucAddr))
        {
            return BSP_ERROR;
        }
        memcpy(gaucCoreNetPacketHead+6,pucAddr,6);
#else
        memcpy(pbyData + 6, g_eth3auMacAddr, 6);
        if (pbyData[12] == 0x81 && pbyData[13] == 0x00)
        {
            pbyData[16] = 0x08;
            pbyData[17] = 0x00;
        }
        else
        {
            pbyData[12] = 0x08;
            pbyData[13] = 0x00;
        }
#endif
    }

    BSP_S_strcpy(ifstruct.ifr_name, 20, ifName);
    ioctl(gatIfBindTbl[ifBindTblIndex].sockRcv, SIOCGIFINDEX, &ifstruct);
    memset(&dest, 0, sizeof(dest));
    dest.sll_family = AF_PACKET;
    dest.sll_protocol = htons(ETH_P_ALL);
    dest.sll_ifindex = ifstruct.ifr_ifindex;
    dest.sll_halen = 6;
    //printf("send ifName->%s,pBuf->%s\r\n",ifName,pBuf);
    if ( 0 == ifBindTblIndex )
    {
#if 1
        printf("send data to dsp%d:\n",msg.s16DevID);
        {
            int ii=0;
            for(ii=0; ii<dwLen+16; ii++)
                printf("%02x ",g_eth0buff[ii]);
            printf("\n");
        }
#endif
        if (sendto(gatIfBindTbl[ifBindTblIndex].sockSend, g_eth0buff, dwLen+16, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
        {
            gatIfBindTbl[ifBindTblIndex].dwSndPcktCnt++;
            //free(pBuf);
            return BSP_SUCCESS;
        }
        else
        {
            perror("send error!\n");
            //free(pBuf);
            return GMAC_COMM_ERR_IF_SEND_ERR;
        }
    }
    else if(1 == ifBindTblIndex)
    {
        if (sendto(gatIfBindTbl[ifBindTblIndex].sockSend, g_eth1buff, dwLen+16, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
        {
            gatIfBindTbl[ifBindTblIndex].dwSndPcktCnt++;
            //free(pBuf);
            return BSP_SUCCESS;
        }
        else
        {
            perror("send error!\n");
            //free(pBuf);
            return GMAC_COMM_ERR_IF_SEND_ERR;
        }
    }
    else if(3 == ifBindTblIndex)
    {
        if (sendto(gatIfBindTbl[ifBindTblIndex].sockSend, pbyData, dwLen, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
        {
            gatIfBindTbl[ifBindTblIndex].dwSndPcktCnt++;
            //free(pBuf);
            return BSP_SUCCESS;
        }
        else
        {
            perror("send error!\n");
            //free(pBuf);
            return GMAC_COMM_ERR_IF_SEND_ERR;
        }
    }
    else
    {
        return GMAC_COMM_ERR_IF_SEND_ERR;
    }
}

unsigned char gaucDebugPacketHead[]=
{
    0x30, 0x31, 0x32, 0x33, 0x48, 0x48, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0x9c, 0x00
};

unsigned char gaucSignalPacketHead[]=
{
    0x30, 0x31, 0x32, 0x33, 0x49, 0x49, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0x9c, 0x01
};

unsigned char gaucMtsPacketHead[]=
{
    0x30, 0x31, 0x32, 0x33, 0x49, 0x49, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0x9c, 0x10
};

unsigned char gaucNrl2PacketHead[]=
{
    0x30, 0x31, 0x32, 0x33, 0x49, 0x49, /* DST MAC */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* SRC MAC */
    0x9c, 0x20
};
int BspSendNetLogData(unsigned long ifBindTblIndex, unsigned int dwtype,char *pbyData, int dwLen)
{
    struct ifreq    ifstruct;
    char           *pBuf = NULL;
    unsigned char pucAddr[6]= {0};
    unsigned char pbyTmpData[14]= {0};
    int icnttmp=0;
    if ((ifBindTblIndex >= MAX_IFBIND_NUM) || (NULL == pbyData) || (0 == dwLen))
    {
        return GMAC_COMM_ERR_PARAMERR;
    }
    /*上层传递的地址已经包含14个字节头，无需再申请了*/
#if 0
    if ((pBuf = malloc(dwLen+14)) == NULL)
    {
        printf("BspSendIfData no mem\n");
        return BSP_ERROR;
    }
    memcpy(pBuf+14, pbyData, dwLen);
#endif

    struct sockaddr_ll dest;
    CHAR            ifName[20] = { 0 };
    snprintf(ifName, 20, "%s%d", gatIfBindTbl[ifBindTblIndex].tIf.name,gatIfBindTbl[ifBindTblIndex].tIf.unit);
    if ( 0 == ifBindTblIndex )
    {
#if 0
        if (BSP_OK != bsp_get_netif_mac_addr((CHAR*)ifName,pucAddr))
        {
            return BSP_ERROR;
        }
#endif
        memcpy(pucAddr,g_eth0auMacAddr,6);
        switch(dwtype)
        {
        case ETH_TYPE_DEBUG_INFO_TRACE:
            memcpy(gaucDebugPacketHead+6,pucAddr,6);
            /*上层传递的地址已经包含14个字节头，无需再申请了
            memcpy(pBuf,gaucDebugPacketHead,14);
            */
            memcpy(pbyData-14,gaucDebugPacketHead,14);
            break;
        case ETH_TYPE_LTE_SIGNAL_TRACE:
            memcpy(gaucSignalPacketHead+6,pucAddr,6);
            /*
            memcpy(pBuf,gaucSignalPacketHead,14);
            */
            memcpy(pbyData-14,gaucSignalPacketHead,14);
            break;
        case ETH_TYPE_LTE_MTS_TRACE:
            memcpy(gaucMtsPacketHead+6,pucAddr,6);
            /*
            memcpy(pBuf,gaucSignalPacketHead,14);
            */
            memcpy(pbyData-14,gaucMtsPacketHead,14);
            break;
        case ETH_TYPE_LTE_NRL2_TRACE:

            memcpy(gaucNrl2PacketHead+6,pucAddr,6);

            memcpy(pbyData-14,gaucNrl2PacketHead,14);

            break;
        default:
            return BSP_ERROR;
            break;
        }

    }

    BSP_S_strcpy(ifstruct.ifr_name, 20, ifName);
    ioctl(gatIfBindTbl[ifBindTblIndex].sockRcv, SIOCGIFINDEX, &ifstruct);
    memset(&dest, 0, sizeof(dest));
    dest.sll_family = AF_PACKET;
    dest.sll_protocol = htons(ETH_P_ALL);
    dest.sll_ifindex = ifstruct.ifr_ifindex;
    dest.sll_halen = 6;
    //printf("send ifName->%s,pBuf->%s\r\n",ifName,pBuf);
#if 0
    printf("send ifName->%s ",ifName);

    for (icnttmp=0; icnttmp<dwLen+16; icnttmp++)
    {
        printf("%02x ",pBuf[icnttmp]);
    }
    printf("\n");
#endif
    if (sendto(gatIfBindTbl[ifBindTblIndex].sockSend, pbyData-14, dwLen, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
    {
        gatIfBindTbl[ifBindTblIndex].dwSndPcktCnt++;
        //free(pBuf);
        return BSP_SUCCESS;
    }
    else
    {
        perror("send error!\n");
        //free(pBuf);
        return GMAC_COMM_ERR_IF_SEND_ERR;;
    }
}
void BspMacInit(void)
{
    int             i;
    for (i = 0; i < MAX_IFBIND_NUM; i++)
        memset(&gatIfBindTbl[i], 0, sizeof(gatIfBindTbl[i]));
    memset(&gtGmacStat, 0, sizeof(gtGmacStat));
    return;
}

void BspRecvDataThread(int arg)
{
    int retaddr;
    int retlen;
    unsigned long cpumask;
    cpumask = (1 << 0);

    if (sched_setaffinity(0, sizeof(cpumask), &cpumask) < 0)
    {
        printf("pthread_setaffinity_np failed in %s\n", __FUNCTION__);
        return ;
    }
    while (1)
    {
        // retaddr = DeQueue(pQueue,&retlen);
        //if(retaddr)
        // {
        //     MacStackRcvRtn(arg, (unsigned char *)retaddr, retlen);
        // }
    }
}

extern pthread_mutex_t  g_recv_flag;

void BspRecvInit(void)
{
    pthread_t ptid;
    int 			res;
    pthread_attr_t  attr;
    struct sched_param parm;
    BspSpinLockInit((T_SpinLock *) &ethspinlock);
    BspMacInit();
    BspRegisterRecvData();
    bsp_netif_throughput_init();
}

//#include "../inc/Compat.h"

//static volatile spinlock_t  ethspinlock[0];

void BspEthPrintLog(unsigned char *pbuf,unsigned int dwlen)
{
#if 0
    static int flag = 0;
    if (0 == flag)
    {
        //spin_lock_init((spinlock_t *) &ethspinlock[0]);
        //ethsw_set_port_mirror(1,3,0,7);//′óeth0?ú3?
        //ethsw_set_port_mirror(1,3,1,7);//′óeth1?ú3?
        flag = 1;
    }
    if (dwlen>0 && pbuf != NULL)
    {

        //spin_lock((spinlock_t *) &ethspinlock[0]);
        BspSendEmacData("ETHSW1", pbuf, dwlen);
        //spin_unlock((spinlock_t *) &ethspinlock[0]);
    }
#endif
}


/*
    BspEthPrintLogInfo′òó?D??￠￡?ìá1?debug?úμ?′òó?oíD?á??ú×ùμ?′òó?1|?ü;
    í???ààDí?a0x9c00μ?ê?debug trace′òó?D??￠
    í???ààDí?a0x9c01μ?ê?lte2ú?·D?á??ú×ù￡¨D-òé×?￡?

*/
int g_sendemacflag =0;
int BspEthPrintLogInfo(unsigned int dwType,unsigned char *pBuf,unsigned int dwlen)
{
    int ret = BSP_ERROR;
    if (1 == g_sendemacflag )
    {
        switch( dwType )
        {
        case ETH_TYPE_DEBUG_INFO_TRACE:
            spin_lock((T_SpinLock *) &ethspinlock);
            ret = BspSendNetLogData(0,ETH_TYPE_DEBUG_INFO_TRACE,pBuf,dwlen);
            spin_unlock((T_SpinLock *) &ethspinlock);
            break;
        case ETH_TYPE_LTE_SIGNAL_TRACE:
            spin_lock((T_SpinLock *) &ethspinlock);
            ret = BspSendNetLogData(0,ETH_TYPE_LTE_SIGNAL_TRACE,pBuf,dwlen);
            spin_unlock((T_SpinLock *) &ethspinlock);
            break;
        case ETH_TYPE_LTE_MTS_TRACE:
            spin_lock((T_SpinLock *) &ethspinlock);
            ret = BspSendNetLogData(0,ETH_TYPE_LTE_MTS_TRACE,pBuf,dwlen);
            spin_unlock((T_SpinLock *) &ethspinlock);
            break;
        case ETH_TYPE_LTE_NRL2_TRACE:
            spin_lock((T_SpinLock *) &ethspinlock);
            ret = BspSendNetLogData(0,ETH_TYPE_LTE_NRL2_TRACE,pBuf,dwlen);
            spin_unlock((T_SpinLock *) &ethspinlock);
            break;
        default:
            break;
        }
    }
    return ret;
    //return BSP_OK;
}

unsigned int BspSendIpData(int dwport,unsigned char *pbuf,int len)
//int BspSendIpData(char dwport, char *pData, UINT16 usDataLength)
{
    int ret;
    switch(dwport)
    {
    case 0:

        spin_lock((T_SpinLock *) &ethspinlock);
        ret = BspSendEmacData("ETHSW1", pbuf, len);
        spin_unlock((T_SpinLock *) &ethspinlock);
        break;
    case 1:
        ret = BspSendEmacData("ETHSW2", pbuf, len);
        break;
    case 2:
        ret = BspSendEmacData("DEBUG", pbuf, len);
        break;
    case 3:
        //Bsp_Send_IpData(pData, usDataLength);
        ret = BspSendEmacData("CORENET", pbuf, len);
        break;
    default:
        printf("\n port error \n");
        break;
    }
    return ret;
}

void bsp_send_ipdata(s32 s32Port)
{
    int ret;
    u8 u8Buf1[] = {0xf1,0xf2,0xf3,0xf4,0xf1,0xf2,0xf3,0xf4,0x00,0x00,0x00,0x00,0x4e,0x86,0x00,0x00,0x00,0x1,0x00,0x00};
    u8 u8Buf2[] = {0xf1,0xf2,0xf3,0xf4,0xf1,0xf2,0xf3,0xf4,0x00,0x00,0x00,0x00,0x4e,0x86,0x00,0x00,0x00,0x2,0x00,0x00};
    u8 u8Buf3[] = {0xf1,0xf2,0xf3,0xf4,0xf1,0xf2,0xf3,0xf4,0x00,0x00,0x00,0x00,0x4e,0x86,0x00,0x00,0x00,0x3,0x00,0x00};
    u8 u8Buf4[] = {0xf1,0xf2,0xf3,0xf4,0xf1,0xf2,0xf3,0xf4,0x00,0x00,0x00,0x00,0x4e,0x86,0x00,0x00,0x00,0x4,0x00,0x00};
    if((ret = BspSendIpData(s32Port, u8Buf1, sizeof(u8Buf1))) != BSP_SUCCESS)
    {
        printf("send buf1 error %d.\n",ret);
    }
#if 0
    if((ret = BspSendIpData(s32Port, u8Buf2, sizeof(u8Buf2))) != BSP_SUCCESS)
    {
        printf("send buf2 error %d.\n",ret);
    }
    if((ret = BspSendIpData(s32Port, u8Buf3, sizeof(u8Buf3))) != BSP_SUCCESS)
    {
        printf("send buf3 error %d.\n",ret);
    }
    if((ret = BspSendIpData(s32Port, u8Buf4, sizeof(u8Buf4))) != BSP_SUCCESS)
    {
        printf("send buf4 error %d.\n",ret);
    }
#endif
}

int bsp_send_ipdata_to_dsp_fortest(int testid, int srcslot, int dstslot, int dspid, int coreid)
{
    T_EmacTest_MsgHead msgheadtest;
    int s32Port = 0;
    memset(&msgheadtest, 0, sizeof(msgheadtest));
    bsp_set_dsp_info(srcslot, dspid, coreid);
    if((TEST_BOARDS_SRIO&0x0fff) == testid)
    {
        msgheadtest.Rsv = dstslot;      //板间srio数据交换测试时目的slot
    }
    bsp_cpudsp_starttest_emac_pkt(&msgheadtest, testid);
    send_bytes = 128;
    return BspSendIpData(s32Port, &msgheadtest, sizeof(msgheadtest));
}
int bsp_cpudsp_testtest_emac_pkt(T_EmacTest_MsgHead *pmsghead)
{
    pmsghead->MsgType = DSP_TEST_COMMAND;
    pmsghead->TestID = DSP_EMAC_TRAN_TEST3;
    pmsghead->MacType = 0xc1c2;
}

int bsp_send_data_to_dsp_for_test(int slotid, int dspid, int coreid, int bytes)
{
    T_EmacTest_MsgHead msgheadtest;
    unsigned int cpucyclestart;
    unsigned int cpucycleend;
    int s32Port = 0;
    send_bytes = bytes;
    struct timespec ts;

    memset(&msgheadtest, 0, sizeof(msgheadtest));
    bsp_set_dsp_info(slotid, dspid, coreid);
    bsp_cpudsp_testtest_emac_pkt(&msgheadtest);
    cpucyclestart = BspGetCpuCycle();
    if( BspSendIpData(s32Port, &msgheadtest, sizeof(msgheadtest))!=BSP_OK)
    {
        printf("bsp send ip data error.\n");
        return BSP_ERROR;
    }
#if 1
    if(clock_gettime(CLOCK_REALTIME,&ts) == -1)
        perror("clock_gettime");
    ts.tv_sec += 1;
    if(0!=sem_timedwait(&g_cpu_dsp_sem1, &ts))
    {
        testpktcnttimeout++;
    }
    else
    {
        testpktcntok++;
        cpucycleend = BspGetCpuCycle();
        if(cpudspcycle<(cpucycleend-cpucyclestart)*1000/1200)
            cpudspcycle = (cpucycleend-cpucyclestart)*1000/1200;
    }
#endif
    return BSP_OK;
}

void bsp_send_data_to_dsp_for_test_by_times(int slotid, int dspid, int coreid, int bytes, int times)
{
    testtimes=times;
    cpudspcycle=0;
    testpktcntok=0;
    testpktcnterr=0;
    testpktcnttimeout=0;
    sem_init(&g_cpu_dsp_sem1,0,0);
    testtimeflag=1;
    while(testtimes--)
        bsp_send_data_to_dsp_for_test(slotid, dspid,coreid,bytes);
    printf("cpudspcycle is %d.\n",cpudspcycle);
    printf("testpktcntok is %d.\n",testpktcntok);
    printf("testpktcnterr is %d.\n",testpktcnterr);
    printf("testpktcnttimeout is %d.\n",testpktcnttimeout);
}

int BspSendToUser(u8 *pbuf, int len)
{
#ifndef __SIM_DSP__
    if(1 == g_u8EpcSendSwitch)
    {
        BspEth3SendData( pbuf, len );
        return BSP_SUCCESS;
    }
    else
    {
        return BspSendIpData(3, pbuf, len);
    }
#else
    return BspSendIpData(3, pbuf, len);
#endif
}

int BspAddVlan(u16 vlan_id, u8 priority)
{
    struct vlan_ioctl_args ifr;
    struct ifreq ifr1;
    int fd, ret;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    /* Add a vlan interface */
    memset(&ifr, 0, sizeof(ifr));
    sprintf(ifr.device1, "eth3");
    ifr.cmd = ADD_VLAN_CMD;
    ifr.u.VID = vlan_id;

    ret = ioctl(fd, SIOCSIFVLAN, &ifr);
    if (ret < 0)
    {
        perror("ioctl");
        goto out;
    }

    /* Set vlan priority */
    memset(&ifr, 0, sizeof(ifr));
    sprintf(ifr.device1, "eth3.%d", vlan_id);

    ifr.cmd = SET_VLAN_EGRESS_PRIORITY_CMD;
    ifr.vlan_qos = priority;

    ret = ioctl(fd, SIOCSIFVLAN, &ifr);
    if (ret < 0)
    {
        perror("ioctl");
        goto out;
    }

    /* Get interface flag */
    memset(&ifr1, 0, sizeof(ifr1));
    sprintf(ifr1.ifr_name, "eth3.%d", vlan_id);

    ret = ioctl(fd, SIOCGIFFLAGS, &ifr1);
    if (ret < 0)
    {
        perror("ioctl SIOCGIFFLAGS");
        goto out;
    }

    /* Let vlan interface up */
    sprintf(ifr1.ifr_name, "eth3.%d", vlan_id);
    ifr1.ifr_flags |= IFF_UP | IFF_RUNNING;
    ret = ioctl(fd, SIOCSIFFLAGS, &ifr1);
    if (ret < 0)
    {
        perror("ioctl SIOCSIFFLAGS");
        goto out;
    }
out:
    close(fd);

    return ret;
}

int BspRemVlan(u16 vlan_id)
{
    struct vlan_ioctl_args ifr;
    int fd, ret;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&ifr, 0, sizeof(ifr));
    sprintf(ifr.device1, "eth3.%d", vlan_id);
    ifr.cmd = DEL_VLAN_CMD;

    ret = ioctl(fd, SIOCSIFVLAN, &ifr);
    if (ret < 0)
    {
        perror("ioctl");
        goto out;
    }

out:
    close(fd);
    return ret;
}



UINT32 bsp_hmi_ioctl_init(UINT32 u32SlotId)
{
    int fd_iic;
    int ret;
    unsigned int dw_value =0;
    fd_iic = open("/dev/usdpaa", O_RDWR);
    if (fd_iic< 0)
    {
        return BSP_ERROR;
    }
    //dw_value = (u32SlotId<<16) |(IPMB_MCT_HMI1_I2C_ADDR(IPMB_SLOT1) <<8) | (IPMB_MCT_HMI1_I2C_ADDR(IPMB_SLOT0));
    dw_value = IPMB_MCT_HMI1_I2C_ADDR(u32SlotId);
    printf("dw_value:0x%lx\r\n",dw_value);
    ret  = ioctl(fd_iic, USDPAA_IOC_HMIINIT, &dw_value);
    if (ret <0 )
    {
        close(fd_iic);
        return BSP_ERROR;
    }
    close(fd_iic);
    return BSP_OK;
}


UINT32 bsp_get_iic_status(void)
{
    int fd_iic;
    int ret;
    unsigned int dw_value;
    fd_iic = open("/dev/usdpaa", O_RDWR);
    if (fd_iic< 0)
    {
        return BSP_ERROR;
    }
    ret = ioctl(fd_iic, USDPAA_IOC_GETIICSTATUS, &dw_value);
    if (ret < 0)
    {
        close(fd_iic);
        return BSP_ERROR;
    }
    close(fd_iic);
    //printf("dw_value->0x%lx\r\n",dw_value);
    return dw_value;
}
int bsp_set_iic_status(UINT32 dwvalue)
{
    volatile int ret;
    int fd_iic;
    fd_iic = open("/dev/usdpaa", O_RDWR);
    if (fd_iic< 0)
    {
        printf("can't open /dev/usdpaa device");
        return -ENODEV;
    }
    ret = ioctl(fd_iic, USDPAA_IOC_SETIICSTATUS, &dwvalue);
    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        return ret;
    }
    close(fd_iic);
}
extern int bsp_get_trace_port_mac(char *mac);

struct net_mirror_param
{
    int ibytecnt;
    char mac[6];
};

int BspOpenNetMirror(void)
{
    volatile int ret;
    int fd_usdpaa;
    struct net_mirror_param param = {0};
    char default_mac[6] = {0xbe, 0xbe, 0xbe, 0xbe, 0xbe, 0xbe};

    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return -ENODEV;
    }

    if (bsp_get_trace_port_mac(param.mac) < 0)
        memcpy(param.mac, default_mac, 6);


    ret = ioctl(fd_usdpaa, USDPAA_IOC_OPEN_MIRROR, &param);
    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        return ret;
    }
    printf("get BspOpenNetMirror->0x%x\n",param.ibytecnt);

    close(fd_usdpaa);
    return BSP_OK;
}


int BspCloseNetMirror(void)
{
    volatile int ret;
    int fd_usdpaa;
    struct net_mirror_param param = {0};
    fd_usdpaa = open("/dev/usdpaa", O_RDWR);
    if (fd_usdpaa< 0)
    {
        printf("can't open /dev/usdpaa device");
        return -ENODEV;
    }

    ret = ioctl(fd_usdpaa, USDPAA_IOC_CLOSE_MIRROR, &param);
    if (ret < 0)
    {
        printf("@%d, Unable to ioctl: %s\n", __LINE__, strerror(errno));
        return ret;
    }

    printf("get BspOpenNetMirror->0x%x\n",param.ibytecnt);

    close(fd_usdpaa);
    return BSP_OK;

}

/********************************rx throughput statistic***************************/
#define SIGUSR			(SIGRTMAX - 1)
#define RX_PACKETS		"/sys/class/net/eth3/statistics/rx_packets"
#define RX_BYTES		"/sys/class/net/eth3/statistics/rx_bytes"
#define LOG_LOCATION		"/mnt/btsa/s1_rx_throughput_log0"
#define LOG_LOCATION_BAK	"/mnt/btsa/s1_rx_throughput_log1"

static int g_rx_throughput_max_value = 30000;		/* 30000 packets/s */
static int g_rx_throughput_max_bits = 200000000;	/* 200Mb/s */
static int g_log_size_max = (1024 * 1024);		/* 1MB */
static int g_timer_period = 1;
static timer_t g_timerid;

/* The switch of recording throughput log, default value is on */
static int rx_throughput_on = 1;

extern uint32_t bsp_get_recv_pkts(int port);
extern uint32_t bsp_get_recv_bytes(int port);

static int store_throughput_log(char *log)
{
    FILE *fd;
    int len = strlen(log);
    int ret;
    struct stat file_stat;
    static uint32_t i = 0;	/* file index only can be 0 or 1 */

    ret = stat(LOG_LOCATION, &file_stat);
    if (ret < 0)
    {
        if (errno != ENOENT)
        {
            perror("store_throughput_log stat");
            return -1;
        }
    }
    else if (file_stat.st_size > g_log_size_max)
        rename(LOG_LOCATION, LOG_LOCATION_BAK);

create_file:
    fd = fopen(LOG_LOCATION, "a+");
    if (fd == NULL)
    {
        perror("store_throughput_log fopen");
        return -1;
    }

    ret = fwrite(log, len, 1, fd);
    if (ret < 0)
    {
        perror("store_throughput_log fwrite");
        return -1;
    }

    fclose(fd);

    return 0;
}

static void timer_handle(int signo)
{
    static uint32_t rx_bak_p = 0, rx_p = 0, rx_b = 0, rx_bak_b = 0;
    int i_p = 0, i_b = 0;
    uint32_t tmp;
    uint32_t rx_throughput_p, rx_throughput_b;
    char log[100];
    static int first_flag = 1;
    int period = g_timer_period;
    struct tm *tv;
    time_t new = time(NULL);
    static time_t old = 0;

    if (rx_throughput_on != 1)
        return;

    rx_p = bsp_get_recv_pkts(3);
    rx_b = bsp_get_recv_bytes(3);

    if (first_flag == 1)
    {
        rx_bak_p = rx_p;
        rx_bak_b = rx_b;
        first_flag = 0;
        old = new;
        return;
    }

    /* statistic of packets */
    if (rx_p >= rx_bak_p)
        rx_throughput_p = rx_p - rx_bak_p;
    else
        rx_throughput_p = 0x3fffff - rx_bak_p + rx_p;

    rx_bak_p = rx_p;

    /* statistic of bytes */
    if (rx_b >= rx_bak_b)
        rx_throughput_b = rx_b - rx_bak_b;
    else
        rx_throughput_b = 0xffffffff - rx_bak_b + rx_b;

    rx_bak_b = rx_b;

    period = gmtime(&new)->tm_sec - gmtime(&old)->tm_sec;
    old = new;

    if (period == 0)
        return;

    /* convent to bits */
    rx_throughput_b *= 8;

    /* calculate 1s */
    rx_throughput_b /= period;
    rx_throughput_p /= period;

    /* print the notice and store the log if the value is beyond the threshold */
    if (rx_throughput_b > g_rx_throughput_max_bits ||
            rx_throughput_p > g_rx_throughput_max_value)
    {
        char s[100];

        memset(s, 0, sizeof(s));

        strcpy(s, ctime(&new));

        /* remve '\n' in time string */
        s[strlen(s) - 1] = '\0';

        memset(log, 0, sizeof(log));

        sprintf(log, "[%s] within %ds:\t%u Mb/s %u pkts/s\n",
                s,
                period,
                rx_throughput_b / 1000000,
                rx_throughput_p);

        store_throughput_log(log);
        printf("%s", log);
    }
}

static timer_t init_timer(int period)
{
    clockid_t clockid = CLOCK_REALTIME;
    struct sigevent evp;
    timer_t timerid;
    struct itimerspec itp;
    struct sigaction sa;
    int ttid = syscall(207);

    memset(&sa, 0, sizeof(struct sigaction));
    memset(&evp, 0, sizeof(struct sigevent));
    memset(&itp, 0, sizeof(struct itimerspec));

    evp.sigev_notify = SIGEV_THREAD_ID;
    evp.sigev_signo = SIGUSR;
    evp._sigev_un._tid= ttid;

    if (sigemptyset(&sa.sa_mask) < 0)
    {
        perror("sigemptyset");
        return 0;
    }

    if (sigaddset(&sa.sa_mask, SIGUSR) < 0)
    {
        perror("sigaddset");
        return 0;
    }

    sa.sa_handler = timer_handle;

    if (sigaction(SIGUSR, &sa, NULL) < 0)
    {
        perror("sigaddset");
        return 0;
    }
    if (timer_create(clockid, &evp, &timerid) < 0)
    {
        perror("timer_create");
        return 0;
    }

    itp.it_interval.tv_sec = period;
    itp.it_value.tv_sec = period;

    if (timer_settime(timerid, TIMER_ABSTIME, &itp, NULL) < 0)
    {
        perror("timer_settime");
        timer_delete(timerid);
    }

    return timerid;
}

static void timer_thread(void *data)
{
    struct sigaction sa;
    int signo;


#if 0
    memset(&sa, 0, sizeof(struct sigaction));
    if (sigemptyset(&sa.sa_mask) < 0)
    {
        perror("sigemptyset");
        return 0;
    }

    if (sigaddset(&sa.sa_mask, SIGUSR) < 0)
    {
        perror("sigaddset");
        return 0;
    }
#endif

    g_timerid = init_timer(g_timer_period);

    while (1)
    {
        sleep(1);
    }

}

static int set_timer(int period)
{
    struct itimerspec itp;

    itp.it_interval.tv_sec = period;
    itp.it_value.tv_sec = period;

    if (timer_settime(g_timerid, TIMER_ABSTIME, &itp, NULL) < 0)
    {
        perror("timer_settime");
        return -1;
    }

    g_timer_period = period;

    return 0;
}

static void bsp_netif_throughput_init(void)
{
    pthread_t pid;

    pthread_create(&pid, NULL, timer_thread, NULL);
}

/***********************************************************************
*函数名称：bsp_inform_dsp_master_slotid
*函数功能：通知DSP本板为主
*函数参数：  参数名                    描述
*           
*        	
*函数返回：发送成功或失败
***********************************************************************/
#if 0
int bsp_inform_dsp_master_slotid(u8 slotid,u8 dspid,u8 master_slotid)
{
	u8 au8Data[50]={0};
	u8 *dstmac;

	dstmac=get_dsp_mac(slotid,dspid,0);	/*DSP MAC地址计算*/
	
	memcpy(au8Data, dstmac, 6);
	memcpy(au8Data+6,g_eth0auMacAddr,6);
	au8Data[12]=0xcd;
	au8Data[13]=0xef;
	au8Data[14]=master_slotid;
	struct ifreq ifstruct;
	struct sockaddr_ll dest;
		
	CHAR ifName[20] = { 0 };
	snprintf(ifName, 20, "%s%d", gatIfBindTbl[0].tIf.name,gatIfBindTbl[0].tIf.unit);
	BSP_S_strcpy(ifstruct.ifr_name, 20, ifName);
	ioctl(gatIfBindTbl[0].sockRcv, SIOCGIFINDEX, &ifstruct);
	memset(&dest, 0, sizeof(dest));
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_ALL);
	dest.sll_ifindex = ifstruct.ifr_ifindex;    
	dest.sll_halen = 6;

	if(sendto(gatIfBindTbl[0].sockSend, au8Data, 30, 0, (struct sockaddr *) (&dest),sizeof(dest)) == -1)
	{
		printf("Emac From MCT To Dsp error!\n");
	}

	return BSP_OK;
	/* sendto(gatIfBindTbl[1].sockSend, pu8SendBuf, u16Len+16+u16UtOffset, 0, 
		   (struct sockaddr *) (&tUserPlaneSockAddr),sizeof(tUserPlaneSockAddr)); */	
}
/***********************************************************************
*函数名称：bsp_inform_dsp_master_slave_switch
*函数功能：通知DSP本板为主
*函数参数：  参数名                    描述
*           
*        	
*函数返回：发送成功或失败
***********************************************************************/
void bsp_inform_dsp_MS_switch(u8 master_slotid)
{
	int i;
	u8 u8bbp_slotid;
	u8 dspid;

	memset(g_s32DspAckMSSwitchMsg,0,sizeof(g_s32DspAckMSSwitchMsg));
	for(i = 0;i < 3;i++)
	{
		for(u8bbp_slotid = IPMB_SLOT2;u8bbp_slotid <= IPMB_SLOT7;u8bbp_slotid++)
		{
			if((boards[u8bbp_slotid].mcu_status&0xF0)==MCU_STATUS_RECV_FIRSTMSG)
			{
				g_s32DspAckMSSwitchMsg[u8bbp_slotid-2]=MCT_MS_SWITCH_INFORMED;
				bsp_inform_dsp_master_slotid(u8bbp_slotid, 3, master_slotid);
			}
		}
	}
}
#endif

/***********************************************************************
*函数名称：bsp_send_host_standby_switch_msg
*函数功能：通知对板升主或降备,告知主备切换原因
*函数参数：  参数名                    描述
*           u8 switchto              1升主/0降备
*        	u8 cause        		主备切换原因
*函数返回：发送成功或失败
***********************************************************************/
unsigned int g_eth0hoststandbyswitchmsg = 0;
int bsp_send_host_standby_switch_msg(u8 switchto,u32 cause)
{
	if(MCT_SLAVE == switchto || MCT_MASTER == switchto)
	{
		u8 au8Data[50]={0};
		u8 dstmac[6]={0x00,0xA0,0x1E,0x01,0x01,0x02};
		//u8 srcmac[6]={0x00,0xA0,0x1E,0x01,0x01,0x02};
		if(IPMB_SLOT0 == bsp_get_slot_id())
		{
			dstmac[4]=0x02;		
		}
		if(IPMB_SLOT1 ==  bsp_get_slot_id())
		{
			dstmac[4]=0x01;
		}
		memcpy(au8Data, dstmac, 6);
		memcpy(au8Data+6,g_eth1auMacAddr,6);
		au8Data[12]=0xab;
		au8Data[13]=0xba;
		au8Data[14]=switchto;
		memcpy(au8Data+15,&cause,4);
		
		struct ifreq ifstruct;
		struct sockaddr_ll dest;
		
		CHAR ifName[20] = { 0 };

		snprintf(ifName, 20, "%s%d", gatIfBindTbl[1].tIf.name,gatIfBindTbl[1].tIf.unit);
		BSP_S_strcpy(ifstruct.ifr_name, 20, ifName);
		ioctl(gatIfBindTbl[1].sockRcv, SIOCGIFINDEX, &ifstruct);
		memset(&dest, 0, sizeof(dest));
		dest.sll_family = AF_PACKET;
		dest.sll_protocol = htons(ETH_P_ALL);
		dest.sll_ifindex = ifstruct.ifr_ifindex;    
		dest.sll_halen = 6;

		/* sendto(gatIfBindTbl[1].sockSend, pu8SendBuf, u16Len+16+u16UtOffset, 0, 
			   (struct sockaddr *) (&tUserPlaneSockAddr),sizeof(tUserPlaneSockAddr)); */
		if (sendto(gatIfBindTbl[1].sockSend, au8Data, 30, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
		{
			g_eth0hoststandbyswitchmsg++;
		}
		else
		{
			printf("Emac From MCT To MCT error!\n");
			return BSP_ERROR;
		}
	}
	return BSP_OK;
}

/***********************************************************************
*函数名称：bsp_send_boards_status_info
*函数功能：主板向备板发送从板状态信息
*函数参数：  参数名                    描述
*           u8 switchto              1升主/0降备
*        	u8 cause        		主备切换原因
*函数返回：发送成功或失败
***********************************************************************/
int bsp_send_boards_status_info(void)
{
	u8 au8Data[2048]={0};
	u8 dstmac[6]={0x00,0xA0,0x1E,0x01,0x01,0x02};
	//u8 srcmac[6]={0x00,0xA0,0x1E,0x01,0x01,0x02};
	if(MCT_MASTER != bsp_get_self_MS_state() || MCT_OPP_PD_ON != bsp_get_oppslot_pd())
	{
		return BSP_ERROR;
	}
	if(IPMB_SLOT0 == bsp_get_slot_id())
	{
		dstmac[4]=0x02;		
	}
	if(IPMB_SLOT1 ==  bsp_get_slot_id())
	{
		dstmac[4]=0x01;
	}
	memcpy(au8Data, dstmac, 6);
	memcpy(au8Data+6,g_eth1auMacAddr,6);
	au8Data[12]=0xab;
	au8Data[13]=0xab;
	
	memcpy(au8Data+14,&boards,sizeof(boards));
	memcpy(au8Data+14+sizeof(boards),&g_subboard_reset_over,4);
	
	struct ifreq ifstruct;
	struct sockaddr_ll dest;
		
	CHAR ifName[20] = { 0 };

	snprintf(ifName, 20, "%s%d", gatIfBindTbl[1].tIf.name,gatIfBindTbl[1].tIf.unit);
	BSP_S_strcpy(ifstruct.ifr_name, 20, ifName);
	ioctl(gatIfBindTbl[1].sockRcv, SIOCGIFINDEX, &ifstruct);
	memset(&dest, 0, sizeof(dest));
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_ALL);
	dest.sll_ifindex = ifstruct.ifr_ifindex;    
	dest.sll_halen = 6;

	/* sendto(gatIfBindTbl[1].sockSend, pu8SendBuf, u16Len+16+u16UtOffset, 0, 
		   (struct sockaddr *) (&tUserPlaneSockAddr),sizeof(tUserPlaneSockAddr)); */
	if (sendto(gatIfBindTbl[1].sockSend, au8Data,40+sizeof(boards), 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
	{
		g_eth0hoststandbyswitchmsg++;
		return BSP_OK;
	}else
	{
		perror("Emac syn boards status fail!\n");
		return BSP_ERROR;
	}
	
	return BSP_OK;
}

/***********************************************************************
*函数名称：bsp_send_gratuitous_arp
*函数功能：通知与基站相连的设备更改mac地址
*函数参数：  参数名                    描述
*           
*        	
*函数返回：发送成功或失败
***********************************************************************/
int bsp_send_gratuitous_arp(void)
{
	u8 au8Data[100]={0};
	u8 dstmac[6]={0xff,0xff,0xff,0xff,0xff,0xff};

	if(MCT_MASTER != bsp_get_self_MS_state())
	{
		return BSP_ERROR;
	}
	
	memcpy(au8Data, dstmac, 6);
	memcpy(au8Data+6,g_eth3auMacAddr,6);
	au8Data[12]=0x08;	//帧类型
	au8Data[13]=0x06;
	au8Data[14]=0x00;	//硬件类型
	au8Data[15]=0x01;
	au8Data[16]=0x08;
	au8Data[17]=0x00;	//协议类型
	au8Data[18]=0x06;	//硬件地址长度
	au8Data[19]=0x04;	//协议地址长度
	au8Data[20]=0x00;	//操作字段
	au8Data[21]=0x01;
	memcpy(au8Data+22,g_eth3auMacAddr,6);
	netif_get_ip("eth3",au8Data+28);
	memset(au8Data+32,0,6);
	netif_get_ip("eth3",au8Data+38);
	
	struct ifreq ifstruct;
	struct sockaddr_ll dest;
		
	CHAR ifName[20] = { 0 };

	snprintf(ifName, 20, "%s%d", gatIfBindTbl[3].tIf.name,gatIfBindTbl[3].tIf.unit);
	BSP_S_strcpy(ifstruct.ifr_name, 20, ifName);
	ioctl(gatIfBindTbl[3].sockRcv, SIOCGIFINDEX, &ifstruct);
	memset(&dest, 0, sizeof(dest));
	dest.sll_family = AF_PACKET;
	dest.sll_protocol = htons(ETH_P_ALL);
	dest.sll_ifindex = ifstruct.ifr_ifindex;    
	dest.sll_halen = 6;

	/* sendto(gatIfBindTbl[1].sockSend, pu8SendBuf, u16Len+16+u16UtOffset, 0, 
		   (struct sockaddr *) (&tUserPlaneSockAddr),sizeof(tUserPlaneSockAddr)); */
	if (sendto(gatIfBindTbl[3].sockSend, au8Data,50, 0, (struct sockaddr *) (&dest),sizeof(dest)) != -1)
	{
		g_eth0hoststandbyswitchmsg++;
		return BSP_OK;
	}else
	{
		printf("Emac syn boards status fail!\n");
		return BSP_ERROR;
	}
	
	return BSP_OK;
}


