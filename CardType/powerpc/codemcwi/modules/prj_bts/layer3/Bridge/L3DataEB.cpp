/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataEB.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   06/11/07   xin wang      增加转发表的MAC Filter
 *   05/14/07   xiaoweifang   增加支持用户组(group-vlan)
 *   10/09/06   yanghuawei    增加下行广播报时，将vlanid插入到payload中
 *   04/18/06   xiao weifang  Fix some VLAN problems.
 *   04/10/06   xiao weifang  学习模式支持切换漫游 _LearningBridgeNotSupportMobility
 *   03/23/06   xiao weifang  support vlan, FixIp改成非永久性的用户
 *   03/23/06   xiao weifang  修改Egress broadcast filter. enable为不转发
 *   07/29/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

//禁用Winsock 1
#define _WINSOCKAPI_

#include <string.h>
#ifndef __WIN32_SIM__
//VxWorks.
#include <intLib.h >
#include <logLib.h>
#endif

#include "Object.h"
#include "biztask.h"
#include "MsgQueue.h"
#include "Message.h"
#include "SFIDdef.h"

#include "taskDef.h"
#include "ErrorCodeDef.h"
#include "L3DataCommon.h"
#include "L3DataFTCheckVLAN.h"
#include "L3DataMsgId.h"
#include "L3OAMMessageId.h"
#include "L3OAMCommonRsp.h"
#include "l3OamSystem.h"
#include "L3DataSocketTable.h"
#include "L3DataSocket.h"
#include "L3DataSocketMsgIds.h"
#include "L3DataNotifyBTSPubIP.h"

#include "L3DataEB.h"
#include "L3DataDm.h"
#ifdef WBBU_CODE
#include "bootLib.h" //wangwenhua add 20100712
#include <pinglib.h>
char cdrBigBuf[40000];
char vcdrBigBuf[40000];
#endif
#ifdef RCPE_SWITCH
#include "l3OamCfg.h"
UINT8 getIPLastD(UINT32 ip)
{ 
	UINT8* puc = (UINT8*)&ip;
	return *(puc+3);
};
#endif
#ifdef __WIN32_SIM__
//Win32:
#include <ws2tcpip.h>
#else
//VxWorks:
#endif

#ifdef UNITEST
//包含桩函数
#include "L3DataStub.h"
#endif
#include "L3DataAssert.h"

#define TRUNK_CODE_VLAN
extern "C" {
typedef void (*pfEBFreeCallBack) (UINT32 param);
#ifndef WBBU_CODE
typedef void (*pfEBRxCallBack)(char *, UINT16, char *);
#else
typedef void (*pfEBRxCallBack)(void*,char *, UINT16, char *);
#endif
typedef void (*pIsCPEVisitBTSMacCallBack)(char *,char *);
bool   mv643xxRecvMsgFromEB(char *,UINT16,pfEBFreeCallBack,UINT32);
#ifdef WBBU_CODE
BOOL   vxbEtsecRecvMsgFromEB(char *buf, UINT16 len, pfEBFreeCallBack func, UINT32 param);
#endif
void   Drv_Reclaim(void *);
void   Drv_RegisterEB(pfEBRxCallBack);

void Drv_RegisterMacCompare(pIsCPEVisitBTSMacCallBack);
int bspGetBtsID();
int bspGetBtsPubPort(); 
int bspGetBtsPubIp(); 
void CsiMonitorDeadlock(UINT32 tid, UINT32 maxBlockedTime); 
void CsiEnableStackTrace(UINT32 tid);
BOOL bspGetFilterSTP();
#ifdef WBBU_CODE
void L3_L2_Ether_Packet(unsigned char* ptr,unsigned short len);
u_short ui_checksum_BBU(u_short *pAddr, int len);
#endif
#ifndef WBBU_CODE
u_short ui_checksum(u_short *pAddr, int len);
#endif
STATUS  GetBtsMac(unsigned char *pMac);
BOOL   mv643xxRecvMsgFromWANIF(char *,UINT16,pfEBFreeCallBack,UINT32);
BOOL   mv643xxRcvMsgToBTSIPStk(char *,UINT16,pfEBFreeCallBack,UINT32);
UINT32 changeTimetoSec(int y,int mon,int d,int h,int m,int s);
}
#ifdef WBBU_CODE
extern bool notifyAllCPEtoRegister();
extern unsigned char  g_Mac[6];//wangwenhua add 20110516
#endif
extern UINT32 getDhcpIpByMac(UINT8 *mac);
UINT8 compwithLocalMac(UINT8 * macaddr);
    char m_cpe_to_myBTS_Mac[6]={0,0,0,0,0,0};  
    UINT32 m_cpe_to_myBTS_Eid=0xffffffff;
    UINT32 m_cpe_to_myBTS_IP=0xffffffff;

//lijinan 20081208 计费系统增加
extern UINT32  findUidFromEid(UINT32 eid,UINT8 *adminStatus,UINT8 *ut_type,char*bw);
extern bool getWifiFlagByEid(UINT32 eid);
UINT32 uiCdrSwitch = 0;
UINT32 real_time_switch = 1;
stCdrNameNVRAM* const gpNVRamCdrName  = (stCdrNameNVRAM*)( (UINT8*)( NVRAM_CDR_BASE ) + sizeof( stCdrNVRAMhdr ) );
#if 1// LJF_BILLING_VOICE
stCdrNameNVRAM* const gpNVRamVcdrName  = (stCdrNameNVRAM*)( (UINT8*)( NVRAM_CDR_BASE ) + sizeof( stCdrNVRAMhdr ) + sizeof(stCdrNameNVRAM)*MAX_SAVE_CDR_NUM );
const UINT32 NVRamCdrNameLength = (sizeof(stCdrNameNVRAM) * MAX_SAVE_CDR_NUM) * 2;
#else
const UINT32 NVRamCdrNameLength = (sizeof(stCdrNameNVRAM) * MAX_SAVE_CDR_NUM);
#endif
int gNeedSendCFFileToFtp = FLAG_NO;
int rt_bill_need_ftp = FLAG_NO;
int gFlagInFtp = FLAG_NO;
UINT32 ftpSecond = 0;
UINT32 IP_CB3000 = 0;
UINT32 time_beart_cb3000 = 0;
UINT32 cdr_para_err_flag = 0;
extern T_NvRamData *NvRamDataAddr;
///////////统计eb任务收发流量jiaying20110801
UINT32 statFromAir=0;//统计从二层来的数据长度
UINT32 statFromWan=0;//统计从网络侧来的数据长度
UINT32 statFromTDR=0;//统计从TDR来的数据长度
UINT32 statToAir=0;//统计发给二层的数据长度
UINT32 statToWan=0;//统计发给网络侧的数据长度
UINT32 statToTDR=0;//统计发给隧道的数据长度
UINT16 statLen = 10;//统计时长
UINT16 statPrintFlag = 0;//是否打印
UINT32 statPktFromAir=0;//统计从二层来的数据包个数
UINT32 statPktFromWan=0;//统计从网络侧来的数据包个数
UINT32 statPktFromTDR=0;//统计从TDR来的数据包个数
UINT32 statPktToAir=0;//统计发给二层的数据包个数
UINT32 statPktToWan=0;//统计发给网络侧的数据包个数
UINT32 statPktToTDR=0;//统计发给隧道的数据包个数

extern UINT32 statFromWanDrv;//从网口驱动收到的数据长度
extern UINT32 statToWanDrv1;//发给网口驱动的数据长度
extern UINT32 statToWanDrv2;//发给网口驱动的数据长度
extern UINT32 statPktFromWanDrv;//从网口驱动收到的数据包个数
extern UINT32 statPktToWanDrv1;//发给网口驱动的数据包个数
extern UINT32 statPktToWanDrv2;//发给网口驱动的数据包个数
unsigned int g_eb_no_ft_freelist =0;
unsigned int g_eb_no_cdr_freelist =0;
unsigned int g_cdr_btree_err =0;
///////////统计eb任务收发流量jiaying20110801
typedef map<CMac, UINT16>::value_type ValType;

//任务实例指针的初始化
CTBridge*   CTBridge::  s_ptaskBridge = NULL;

stEBDebugInfo gEBDbgInfo;
//FIXIP默认的TTL总长(秒)
UINT16 g_usFixIpTTL        = 3600;
UINT32 g_ulOccupiedCounter = 0;
UINT8 Ping_Check_Flag = 0;//wangwenhua add 20080804
UINT8 Ping_Check_End = 0;
#ifdef WBBU_CODE
unsigned int Procount = 0;
unsigned char  L2_BOOT_FLAG = 0;
unsigned int    Commsg_Send_err =0;
extern "C" u_short ui_checksum_BBU
(
unsigned short *           pAddr,                  /* pointer to buffer  */
int                 len                     /* length of buffer   */
)
{
register int nLeft=len;
register int sum= 0;
register unsigned short *w=pAddr;
/*    u_short     answer;*/

while (nLeft > 1)
    {
    sum     += *w++;
    nLeft   -= 2;
    }

if (nLeft == 1)
#if _BYTE_ORDER == _BIG_ENDIAN
    sum += 0 | ((*(u_char *) w) << 8);
#else
    sum += *(u_char *) w;
#endif

sum = (sum >> 16) + (sum & 0xffff);
sum += (sum >> 16);

/*    answer = sum;
return (~answer & 0xffff);
*/
return (~sum & 0xffff);
}
unsigned int L2_L3_Ingress = 0;
unsigned int L2_L3_Wan = 0;
unsigned int L2_L3_Rels = 0;
unsigned int L3_L2_Send_err = 0;
unsigned int L2_L3_Long = 0;
unsigned int Wanif_L3_IP = 0;
#endif
UINT8 g_BC_test_flag = 0xf;//default is open wangwenhua 20100621
UINT32  g_send_2_self = 0;
#ifdef M_TGT_WANIF
extern UINT32  WorkingWcpeEid;
extern UINT32  RelayWanifCpeEid[20];
//extern unsigned char WCPE_falg;
extern UINT16   Wanif_Switch;
extern UINT32 WanIfCpeEid;
extern UINT32 BakWanIfCpeEid;
#endif

extern int video_debug;//lijinan 20101020 for video

static UINT32 dwMask;
static UINT32 dwGateway;
static char bMac[6];
static UINT32 dwIP;
RegisterInfo g_last_register_eid[5];
UINT32 Register_eid_index = 0;
extern RF_Operation_Para    g_rf_openation;//wangwenhua add 2012-2-26
DuplicateMac   g_duplicate_mac[20];
UINT8    Duplicate_eid_index = 0;
/**********************************************************************
*
*  NAME:    set_bc_flag      
*  FUNTION:  测试函数
*  INPUT:        测试值
*  OUTPUT:        无
*  OTHERS:   wangwenhua20100721     
*********************************************************************/
void set_bc_flag(unsigned char flag)
{
     g_BC_test_flag = flag;
}

bool eidIsWifiEid(UINT32 eid)
{
	return getWifiFlagByEid(eid);
}
/*============================================================
MEMBER FUNCTION:
    CTBridge::ProcessComMessage

DESCRIPTION:
    Bridge任务消息处理函数

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    bool:true or false,FrameWork根据返回值决定是否做PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
#ifdef DIAG_TOOL_TEST
extern "C"  void send_2_diag_tools(unsigned char *pBuf,unsigned int Datalen);
#endif
//bool CTBridge::ProcessComMessage(CComMessage *pComMsg)
bool CTBridge::ProcessMessage(CMessage &cMsg)
{
    CComMessage *pComMsg = cMsg.GetpComMsg();
       if(pComMsg==NULL )
      {
          return false;
        }
    UINT16 usMsgId = pComMsg->GetMessageId();
#ifdef WBBU_CODE
    unsigned char *ptr;
    unsigned int data_len;
    CMac DstMac1( GetDstMac( pComMsg ) );
#endif
    switch ( usMsgId )
        {
#if 0//def RCPE_SWITCH
        case M_CPE_L3_TRUNK_MAC_MOVEAWAY_RSP:
			usNum = pComMsg->GetDataLength()/10;
			pe = (T_MACIP_LIST_E*)pComMsg->GetDataPtr();
		    LOG1( LOG_SEVERE, LOGNO( EB, EC_EB_NORMAL ), "Get MRCPE MAC-IP NUM[%d]", pComMsg->GetDataLength()/10 );
			for( us=0; us<*(UINT16*)pComMsg->GetDataPtr(); us++ )
			{
			    LOG4( LOG_SEVERE, LOGNO( EB, EC_EB_NORMAL ), "ip[%-3d], mac[%02x-%02x-%02x]", 
					getIPLastD(pe->ulIp), *(pe->cMac.GetMac()+3), *(pe->cMac.GetMac()+4), *(pe->cMac.GetMac()+5));
				pe++;
			}
			break;
#endif
//#ifdef LJF_BILLING_VOICE
        case M_VOICE_EB_BILLING_NOTIFY:
		    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "CTBridge::M_VOICE_EB_BILLING_NOTIFY" );
			cCdrObj.sendVoiceMsgToWriteFileTask( FromVoiceInfo, (UINT8*)pComMsg->GetDataPtr(), pComMsg->GetDataLength() );
			break;
        case MSGID_HIGH_PRIORITY_TRAFFIC:
        case MSGID_LOW_PRIORITY_TRAFFIC:
        case MSGID_REALTIME_TRAFFIC:
	 case MSGID_VIDEO_DATA_TRAFFIC://lijinan 20101020
                // Ingress traffic process.
                Ingress( pComMsg );
            break;

        case MSGID_TRAFFIC_EGRESS:
                // Egress traffic process.
#ifndef WBBU_CODE
                Egress( pComMsg );
#else
		 #ifdef DIAG_TOOL_TEST
		    if((DstMac1.IsL2Addr()== false))//将消息发送给L2
		    	{
		    	      Egress( pComMsg );
		    	}
			else
			{
		    	   ptr = (unsigned char*)pComMsg->GetDataPtr();
			   data_len =( ptr[38]*0x100+ptr[39]) - 8;
		    	    send_2_diag_tools(ptr+42,data_len);
			   pComMsg->SetMblk(NULL);//不释放，完全由IP协议栈释放
		    	   
			}
		    

		#endif
#endif
            break;
	#ifdef  M_TGT_WANIF
      case  MSGID_TRAFFIC_IPSTACK:
	  	IPEngress(pComMsg);
	  	break;
	#endif
        case MSGID_TRAFFIC_FORWARD:
                // Forward Traffic.
                ForwardTraffic( pComMsg );
            break;

        case MSGID_TRAFFIC_ETHERIP:
                // EtherIp traffic process.
                {
                TDR( pComMsg );
                }
            break;

        case MSGID_FT_ADD_ENTRY:
                // Add FT Entry.
                {
                CMessage msg( pComMsg );
                FTAddEntry( msg );
                }
            break;

        case MSGID_FT_UPDATE_ENTRY:
                // Update FT Entry.
                {
                CMessage msg( pComMsg );
                FTUpdateEntry( msg );
                }
            break;

        case MSGID_FT_DEL_ENTRY:
                // Delete FT Entry.
                {
                CMessage msg( pComMsg );
                FTDelEntry( msg );
                }
            break;

        case MSGID_FT_ENTRY_EXPIRE:
                // FT Entry Expire.
                {
                CMessage msg( pComMsg );
                FTEntryExpire( msg );
                }
            break;

        case MSGID_MFT_UPDATE_ENTRY:
        case MSGID_MFT_ADD_ENTRY:
            // MFT Entry Update.
            {
                CMessage msg( pComMsg );
                MFTUpdateEntry( msg );
            }
            break;
        
        case MSGID_MFT_ENTRY_EXPIRE:
        case MSGID_MFT_DEL_ENTRY:
            // MFT Entry Expire.
            {
                CMessage msg( pComMsg );
                MFTEntryExpire( msg );
            }
            break;

        case MSGID_FT_CHECK_VLAN:
                //check if VLAN changes.
                {
                CMessage msg( pComMsg );
                CheckVLAN( msg );
                }
            break;

        case M_CFG_EB_DATA_SERVICE_CFG_REQ:
                // Data Config.
                {
                CMessage msg( pComMsg );
                EBConfig( msg );
                }
            break;
        case MSGID_OAM_NOTIFYBTSPUBIP:
                {  
                CMessage msg( pComMsg );
                EBNotifyBTSIPChange(msg);   
                }
            break;
        case MSGID_FT_MODIFY_BTSPUBIP:
                {  
                CMessage msg( pComMsg );
                FTModifyBTSPubIP(msg);
                }
            break;
        case M_EMS_BTS_QOS_CFG_REQ:
                // SFID Config.
                {
                CMessage msg( pComMsg );
                TosSFIDConfig( msg );
                }
            break;
        case M_EMS_BTS_VLAN_GROUP_CFG_REQ:
                {
                CMessage msg( pComMsg );
                VlanGroupConfig(msg);
                }
            break;

	  case M_EMS_BTS_UT_PROFILE_UPDATE_REQ:
	  	EidCdrAdminStatus(pComMsg);
	  	break;
	case MSGID_VEDIO_IPADDRESS_REPORT://lijinan 20101020 for video
		updateVideoIp( pComMsg);
		break;
       case MSG_BTS_TO_CB3000:
	   	proMsgFromCb3000(pComMsg);
	   	break;
	case MSG_BTS_TO_CB3000_FAIL:
		func_msg_fail(pComMsg);
		break;
	case MSGID_BWINFO_DEL_REQ:
		if(uiCdrSwitch==1)
		{
		if(real_time_switch)
			func_moveAway_RT_Charge(pComMsg);
		}
		break;
        default:
                LOG1( LOG_WARN, LOGNO( EB, EC_EB_UNEXPECTED_MSGID ), 
                 "Bridge receive unexpected message[Id: 0x%x].", usMsgId );
            break;
        }

    //destroy message.
   // pComMsg->Destroy();

#ifdef __WIN32_SIM__
    showStatus();
#endif
    return true;
}

UINT16 CTBridge::m_ping_seq = 0;//wangwenhua add 20080617
UINT16 CTBridge::m_ping_ack_seq = 0;//wangwenhua add 20080918
/*============================================================
MEMBER FUNCTION:
    CTBridge::CTBridge

DESCRIPTION:
    CTBridge构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTBridge::CTBridge()
{
#ifndef WBBU_CODE
    BOOT_PARAMS params;
    struct ifnet 		*ifp;
    char	buf[100]; 	
#else
   BOOT_PARAMS       bootParams;
    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
#endif
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "CTBridge::CTBridge()" );

#ifndef NDEBUG
    if ( !Construct( CObject::M_OID_EB ) )
        {
        LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "ERROR!!!CTBridge::CTBridge()% Construct failed." );
        }
#endif

    memset( m_szName, 0, sizeof( m_szName ) );
    memcpy( m_szName, M_TASK_BRIDGE_TASKNAME, strlen( M_TASK_BRIDGE_TASKNAME ) );
    m_uPriority     = M_TP_L3EB;
    m_uOptions      = M_TASK_BRIDGE_OPTION;
    m_uStackSize    = M_TASK_BRIDGE_STACKSIZE;

    m_iMsgQMax      = M_TASK_BRIDGE_MAXMSG;
    m_iMsgQOption   = M_TASK_BRIDGE_MSGOPTION;

    m_EtherIpSocket     = 0;
    m_bEgressBCFltrEn   = false;
    m_ucWorkingMode     = WM_NETWORK_AWARE;
    m_usPPPoESessionAliveTime   = M_PPPOE_SESSION_ALIVETIME;
    m_usLearnedBridgeAgingTime  = M_LEARNED_BRIDGE_AGINGTIME;
    m_usExpireTimeInSeconds     = M_PPPOE_SESSION_ALIVETIME;

    m_blNotifyBTSPubIP=false;

    ////不同类型用户数统计值
    m_usNotServing       = 0;
    m_usNowServing       = 0;
    m_usAnchorAndServing = 0;
    m_usServingNotAnchor = 0;

    memset( m_FT, 0, sizeof( m_FT ) );
    memset( m_MFT, 0, sizeof( m_MFT ) );
    memset( m_aulDirTrafficMeasure, 0, sizeof( m_aulDirTrafficMeasure ) );

    //SFID默认值 = M_SFID_LOW
    for ( UINT32 idx = 0; idx < M_TOS_SFID_NO; ++idx )
        {
        m_aSFID[ idx ] = M_SFID_LOW;
        }

    InitFreeFTList();
    InitFreeMFTList();
    //初始化ComMessage链表
#ifndef __WIN32_SIM__
////VxWorks.
////lstInit( &m_listFreeNode );
////lstInit( &m_listComMessage );
    m_plistComMessage = NULL;
#endif

    for (UINT16 idx = 0; idx < M_MAX_GROUP_NUM; ++idx)
        {
        m_group[idx].usVlanID = idx;
        }

    //调试控制块信息初始化
    memset(&gEBDbgInfo, 0, sizeof(gEBDbgInfo));
#ifndef WBBU_CODE

	/* get  ip and subnet mask */
	if(ifAddrGet("mv0", buf) != OK)
	{
		printf("\nget ip addr failure!");		
		return ;
	}
	dwIP = (UINT32)inet_addr(buf);
	if(ifMaskGet("mv0", (int*)&dwMask) != OK)
	{
		printf("\nget subnetmask failure!");		
		return ;
	}    
#else
   dwIP =     inet_addr(bootParams.ead);
   
   bootNetmaskExtract( bootParams.ead,(int*)&dwMask);
   logMsg("dwip:%08x,dwMask:%08x\n",dwIP,dwMask,0,1,2,3);
#endif
    if(GetBtsMac(m_btsMac)!=OK)
      	{
      	     logMsg("GetBtsMac error\n",1,2,3,4,5,6);
	      
      	}
    else
	 {
	 #ifdef WBBU_CODE
	    memcpy(g_Mac,m_btsMac,6);
	 #endif
	     logMsg("bts mac :%x,%x,%x,%x,%x,%x\n",m_btsMac[0],m_btsMac[1],m_btsMac[2],m_btsMac[3],m_btsMac[4],m_btsMac[5]);
	 }
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::~CTBridge

DESCRIPTION:
    CTBridge析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTBridge::~CTBridge()
{
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "CTBridge::~CTBridge" );

#ifdef __WIN32_SIM__
    ::WSACleanup();
#endif
#ifndef NDEBUG
    if ( !Destruct( CObject::M_OID_EB ) )
        {
        LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "ERROR!!!CTBridge::~CTBridge failed." );
        }
#endif
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::GetInstance

DESCRIPTION:
    Get CTBridge Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTBridge* 

SIDE EFFECTS:
    none
==============================================================*/
CTBridge* CTBridge::GetInstance()
{
    if ( NULL == s_ptaskBridge )
        {
        s_ptaskBridge = new CTBridge;
        }
    return s_ptaskBridge;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::showStatus

DESCRIPTION:
    打印Bridge任务转发表索引数和空闲转发表信息

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::showStatus()
{
#ifdef __WIN32_SIM__
    //等showStatus信号量
    WAIT();
#endif
    printf( "\r\n*************************" );
    printf( "\r\n*Bridge Task Attributes *" );
    printf( "\r\n*************************" );
    printf( "\r\n%-35s: %d", "Task   stack   size", M_TASK_BRIDGE_STACKSIZE );
    printf( "\r\n%-35s: %d", "Task   Max  messages", M_TASK_BRIDGE_MAXMSG );
    printf( "\r\n" );
    printf( "\r\n%-35s: %s", "Access Control", \
        ( WM_LEARNED_BRIDGING == m_ucWorkingMode )?"Learned Bridging Mode":"Network Layer Aware Mode" );
    printf( "\r\n%-35s: %-5d (sec)", "PPPoE Session Keep Alive Time", m_usPPPoESessionAliveTime );
    printf( "\r\n%-35s: %-5d (sec)", "Learned Bridging Aging Time", m_usLearnedBridgeAgingTime );
    printf( "\r\n%-35s: %s", "Egress Broadcast filter", 
        ( true == m_bEgressBCFltrEn )?"Enable":"Disable" );
    printf( "\r\n%-35s: %-5d (sec)", "Forwarding Entry Expire Time", m_usExpireTimeInSeconds );

#ifndef __WIN32_SIM__
//VxWorks.
#endif
#ifdef BUFFER_EN
    printf( "\r\n%-35s: %s",   "Downlink Data Buffer", "Enable" );
#else
    printf( "\r\n%-35s: %s",   "Downlink Data Buffer", "Disable" );
#endif

    printf( "\r\n%-35s: %d/%d",   "Occupied message number/total", g_ulOccupiedCounter, M_MAX_LIST_SIZE);

    printf( "\r\n" );
    //Free FT Entries
    printf( "\r\n%-35s: %-5d entries", "Free FT Entries", m_listFreeFT.size() );

    //BPtree
    if ( true == m_FTBptree.empty() )
        {
        printf( "\r\n%-35s: %s", "Forwarding Table", "0     entries" );
        }
    else
        {
        printf( "\r\n%-35s: %-5d entries", "Forwarding Table", m_FTBptree.size() );
        }

    printf( "\r\n" );
    printf( "\r\n%-35s: %-5d", "Not serving users", m_usNotServing );
    printf( "\r\n%-35s: %-5d", "Serving users", m_usNowServing );
    printf( "\r\n  %-33s: %-5d", "Anchor and serving users", m_usAnchorAndServing );
    printf( "\r\n  %-33s: %-5d", "Serving but not anchor users", m_usServingNotAnchor );

    printf( "\r\n" );
    printf( "\r\n***************************************" );
    printf( "\r\n*Direction and Type Measurement Status*" );
    printf( "\r\n***************************************" );

    printf( "\r\n%-20s", "From Direction" );
    for ( UINT8 from = EB_FROM_AI; from < EB_FROM_MAX; ++from )
        {
        printf( "%-10s", strFromDir[ from ] );
        }
    printf( "\r\n-----------------------------------------------------------" );
    for ( UINT8 type = TYPE_TRAFFIC_DHCP; type < TYPE_TRAFFIC_MAX; ++type )
        {
        printf( "\r\n%-20s", strTrafficType[ type ] );
        for ( UINT8 from = EB_FROM_AI; from < EB_FROM_MAX; ++from )
            {
            printf( "%-10d", m_aulDirTrafficMeasure[from][type] );
            }
        }

    printf( "\r\n" );

#ifdef __WIN32_SIM__
    //释放showStatus信号量
    RELEASE();
#endif
    return ;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::showFT

DESCRIPTION:
    打印Bridge任务转发表详细信息，可按 EID 分类打印

ARGUMENTS:
    EID

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::showFT(UINT32 ulEID)
{
    if ( 0 == ulEID )
        {
        printf( "\r\nAll forwarding table information" );
        }
    else
        {
        printf( "\r\nEID[0x%.8X] forwarding table information*", ulEID );
        }

    FTEntry *pFT;
    map<CMac, UINT16>::iterator it = m_FTBptree.begin();
    UINT8 ucFlowCtrl = 0;
    printf( "\r\n%-18s:%-10s:%-5s:%-3s:%-6s:%-10s:% -10s:%-12s:%-4s:%-6s:%-8s",
            "Mac-Address", "Eid(Hex)", "Group", "Srv",
            "Tunnel", "Peer-BtsID","Peer-BtsPort", "Elapse(/TTL)", "Auth" ,"Buffed","DM_SYNC" );
    printf( "\r\n----------------------------------------------------------------------------------------" );
    while ( m_FTBptree.end() != it )
        {
        pFT = GetFTEntryByIdx( it->second );
        if ( ( 0 != ulEID ) && ( pFT->ulEid != ulEID ) )
            {
            ++it;
            continue;
            }
#if 0
        struct in_addr IpAddr;
#ifdef __WIN32_SIM__
        IpAddr.S_un.S_addr = htonl( pFT->ulPeerBtsAddr );
#else
        IpAddr.s_addr = htonl( pFT->ulPeerBtsAddr );
#endif
#endif
#ifdef BUFFER_EN
        UINT16 usBuffCount = ( NULL == pFT->pBufList )?0:pFT->pBufList->count();
#else
        UINT16 usBuffCount = 0;
#endif
       // SINT8 strIpAddr[ INET_ADDR_LEN ] = {0};
        //inet_ntoa_b( IpAddr, strIpAddr );
        printf( "\r\n%.2X-%.2X-%.2X-%.2X-%.2X-%.2X  %.8X   %-5d %-3s %-6s %-8x %-10d %-5d/%5d %-4s %-5d,%-2d", 
            pFT->aucMAC[0],pFT->aucMAC[1],pFT->aucMAC[2],pFT->aucMAC[3],pFT->aucMAC[4],pFT->aucMAC[5],
            pFT->ulEid,
            pFT->usGroupId,
            ( true == pFT->bIsServing )?"Yes":"No",
            ( true == pFT->bIsTunnel  )?"Yes":"No",
            pFT->ulPeerBtsID,//inet_ntoa( IpAddr ),
            pFT->usPeerBtsPort,
            pFT->usElapsed,pFT->usTTL,
            ( true == pFT->bIsAuthed )?"Yes":"No" ,
            usBuffCount, pFT->DM_Sync_Flag
            );
        //next.
        ++it;
        if ( EB_FLOWCTRL_CNT == ++ucFlowCtrl )
            {
#ifdef __WIN32_SIM__
//Win32:
            ::Sleep( 100 );//释放CPU
#else
//VxWorks:
            ::taskDelay( 1 );
#endif
            ucFlowCtrl = 0;
            }
        }

    printf( "\r\n" );

    return ;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::showPerf

DESCRIPTION:
    按Mac地址打印Bridge任务性能统计信息

ARGUMENTS:
    MAC-Address

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::showPerf(const UINT8 *pMac)
{
    if ( NULL == pMac )
        {
        printf("\r\nPlease input a MAC Address.\r\n");
        return;
        }

    UINT8   ucFlowCtrl = 0;
    FTEntry *pFT;
    CMac    QueryMac( pMac );
    map<CMac, UINT16>::iterator it = m_FTBptree.begin();

    printf( "\r\nUser[%.2X-%.2X-%.2X-%.2X-%.2X-%.2X] performance information",
        pMac[0],pMac[1],pMac[2],pMac[3],pMac[4],pMac[5] );
    printf( "\r\n%-20s", "      from\\to       " );
    for ( UINT8 to = EB_TO_AI; to < EB_TO_MAX; ++to )
        {
        printf( "%-20s", strToDir[ to ] );
        }

    printf( "\r\n--------------------------------------------------------------------" );
    while ( m_FTBptree.end() != it )
        {
        pFT = GetFTEntryByIdx( it->second );
        if ( !( it->first == QueryMac ) )
            {
            ++it;
            continue;
            }

        for ( UINT8 from = EB_FROM_AI; from < EB_FROM_MAX; ++from )
            {
            printf( "\r\n%-20s", strFromDir[ from ] );
            for ( UINT8 to = EB_TO_AI; to < EB_TO_MAX; ++to )
                {
                printf( "%-20d", pFT->aulTrafficMeasure[ from ][ to ] );
                }
            }
        ++it;

        if ( EB_FLOWCTRL_CNT == ++ucFlowCtrl )
            {
#ifdef __WIN32_SIM__
//Win32:
            ::Sleep( 100 );//释放CPU
#else
//VxWorks:
            ::taskDelay( 1 );
#endif
            ucFlowCtrl = 0;
            }
        }

    printf( "\r\n" );

    return ;
}

/*============================================================
MEMBER FUNCTION:
CTBridge::showFT

DESCRIPTION:
打印Bridge任务转发表详细信息，可按 EID 分类打印

ARGUMENTS:
EID

RETURN VALUE:
void 

SIDE EFFECTS:
none
==============================================================*/
void CTBridge::showMFT()
{
    printf( "\r\nAll MAC filtering table information" );

#if 0
    MACFilterEntry *pMFT;
    map<CMac, UINT16>::iterator it = m_MFTBptree.begin();
    UINT8 ucFlowCtrl = 0;
    printf( "\r\n%-20s:%-13s:%10s%-10s",
        "Mac-Address", "Type", "Elapse","/TTL");
    printf( "\r\n------------------------------------------------------" );

    char type[20];
    
    while ( m_MFTBptree.end() != it )
    {
        pMFT = GetMFTEntryByIdx( it->second );
        if(pMFT->TYPE == EB_MACFILTER_ROUTER)
            strcpy(type,"ROUTER");
        else if(pMFT->TYPE == EB_MACFILTER_PPPOESERVER)
            strcpy(type,"PPPOE SERVER");
        else if(pMFT->TYPE == EB_MACFILTER_DHCPSERVER)
            strcpy(type,"DHCP SERVER");
        else
            strcpy(type,"UNKNOWN");
        
        printf( "\r\n%.2X-%.2X-%.2X-%.2X-%.2X-%.2X    %-14s%10d/%-10d", 
            pMFT->MAC[0],pMFT->MAC[1],pMFT->MAC[2],pMFT->MAC[3],pMFT->MAC[4],pMFT->MAC[5],
            type,
            pMFT->Elapsed,
            pMFT->TTL);
        //next.
#else
    MACFilterEntry *pMFT;
    map<CMac, UINT16>::iterator it = m_MFTBptree.begin();
    UINT8 ucFlowCtrl = 0;
    printf( "\r\n%-20s:%10s%-10s", "Mac-Address", "Elapse","/TTL");
    printf( "\r\n------------------------------------------------------" );
    while ( m_MFTBptree.end() != it )
    {
        pMFT = GetMFTEntryByIdx( it->second );
        printf( "\r\n%.2X-%.2X-%.2X-%.2X-%.2X-%.2X%15d/%d", 
            pMFT->MAC[0],
            pMFT->MAC[1],
            pMFT->MAC[2],
            pMFT->MAC[3],
            pMFT->MAC[4],
            pMFT->MAC[5],
            pMFT->Elapsed,
            pMFT->TTL);
        //next.
#endif      

        ++it;
        if ( EB_FLOWCTRL_CNT == ++ucFlowCtrl )
        {
#ifdef __WIN32_SIM__
            //Win32:
            ::Sleep( 100 );//释放CPU
#else
            //VxWorks:
            ::taskDelay( 1 );
#endif
            ucFlowCtrl = 0;
        }
    }

    printf( "\r\n" );

    return ;
}


void CTBridge::showQoS()
{
    printf("\r\nConfigured high priority ToS value:");
    printf("\r\n----------------------------------");
    int count = 0;
    for(int idx = 0; idx < M_TOS_SFID_NO; ++idx)
        {
        if (0 == count % 5)
            printf("\r\n");
        if (M_SFID_HIGH == m_aSFID[idx])
            {
            printf("%d, ",idx);
            ++count;
            }
        }
    printf("\r\n%d ToS configured.", count);
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::Initialize

DESCRIPTION:
    创建Message Queue;创建Socket

ARGUMENTS:
    NULL

RETURN VALUE:
    true of false 

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::Initialize()
{
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "CTBridge::Initialize" );
#ifdef __WIN32_SIM__
    ::WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
#endif

    //create socket
    m_EtherIpSocket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);   /*IPPROTO_RAW只能用于send*/
    if ( M_DATA_INVALID_SOCKET == m_EtherIpSocket )
        {
#ifdef __WIN32_SIM__
        ::WSACleanup();
#endif
        LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SOCKET_ERR ), "socket create err" );
        return false;
        }
#if 0
    //设置选项，不让协议栈修改Ip头
    UINT32 ulOption = 1;
    UINT32 ulRet = ::setsockopt( m_EtherIpSocket, IPPROTO_IP, IP_HDRINCL, (char*)&ulOption, sizeof( ulOption ) );
    if ( M_DATA_SOCKET_ERR == ulRet )
        {
        LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SOCKET_ERR ), "Set EtherIp Socket Option IP_HDRINCL Fails!" );
#ifdef __WIN32_SIM__
        ::closesocket( m_EtherIpSocket );
        ::WSACleanup();
#else
        ::close( m_EtherIpSocket );
#endif
        return false;
        }
#endif

#ifndef __WIN32_SIM__
    //VxWorks.
    if( false == InitComMessageList() )
        {
        LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "Fail to initialize EB message pool!" );
        ::close( m_EtherIpSocket );
        return false;
        }
#endif

    UINT8 ucInit = CBizTask::Initialize();
    if ( false == ucInit )
        {
        //close socket.
#ifdef __WIN32_SIM__
        ::closesocket( m_EtherIpSocket );
        ::WSACleanup();
#else
        ::close( m_EtherIpSocket );
#endif
        return false;
        }

#ifdef __WIN32_SIM__
    DWORD Tid;
    if ( ( ::CreateThread( NULL, M_TASK_CLEANUP_STACKSIZE, RunCleanUp,(void*)this, NULL, &Tid ) ) == NULL )
    {
        LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "task CleanUp Create failed." );
        ::closesocket( m_EtherIpSocket );
        ::WSACleanup();
        return false;
    }
#else
    UINT32 tid;
    if ( ( tid = ::taskSpawn( M_TASK_CLEANUP_TASKNAME, 
                M_TASK_CLEANUP_PRIORITY, 
                M_TASK_CLEANUP_OPTION,
                M_TASK_CLEANUP_STACKSIZE,
                (FUNCPTR)RunCleanUp, (int)this, 0, 0, 0, 0, 0, 0, 0, 0, 0) ) == ERROR )
    {
        LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "task CleanUp spawn failed." );
        ::close( m_EtherIpSocket );
        return false;
    }
    CsiMonitorDeadlock(tid, EB_CLEANUP_MAX_BLOCKED_TIME_IN_10ms_ICK); 
    CsiEnableStackTrace(tid);

#endif
    //初始化就将Router MAC加入MFT
    InitRouterMAC();

    InitFilterMAC();

#if 0
    //配置完成以后才注册
    //向网卡注册报文接收函数
    Drv_RegisterEB( CTBridge::RxDriverPacketCallBack );
#endif
#ifdef WBBU_CODE
 Drv_RegisterEB( CTBridge::RxDriverPacketCallBack );
#endif
    //lijinan 20081127
    ucInit = cCdrObj.init();
    if(ucInit==false)
    {
		return false;
    }
     if ( ( tid = ::taskSpawn( "tWriteCdrFile", 
            189, 
            0,
            10*1024,
            (FUNCPTR)RunWriteCdrFile, (int)this, 0, 0, 0, 0, 0, 0, 0, 0, 0) ) == ERROR )
    {
	 printf("\nwrite cdr file task create err\n");
        return false;
    }
   cCdrObj.setWriteFileTaskId(tid);
	
#ifdef RCPE_SWITCH
	m_pstTrnukMRCpe = &CTaskCfg::GetInstance()->m_stTrnukMRCpe;
#endif
     for(int j = 0; j < 5; j++)
        {
           g_last_register_eid[j].DebugEID = 0;
           g_last_register_eid[j].TTL = 0;
        }

    for(int k = 0; k<10; k++)
    {
	  g_duplicate_mac[k].DebugEID[0] =0;
	  g_duplicate_mac[k].DebugEID[1] =0;
	  
    }
  
    return true;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::DeallocateComMessage

DESCRIPTION:
    释放ComMessage内存

ARGUMENTS:
    *pComMsg

RETURN VALUE:
    bool: true,成功；false,失败

SIDE EFFECTS:
    none
==============================================================*/
#ifdef WBBU_CODE
unsigned int decount =0;
unsigned int decount1 =0;
#endif
bool CTBridge::DeallocateComMessage(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge DeallocateComMessage()" );
		if(pComMsg==m_plistComMessage)//lijinan 20100121
			return true;
    UINT32 ulFlag = pComMsg->GetFlag();
    if (M_CREATOR_DRV == ulFlag) 
        {
        //驱动提供的缓存，调用驱动的释放函数
#ifdef WBBU_CODE
         if( pComMsg->GetMblk()!=NULL)
         {
        ::Drv_Reclaim( pComMsg->GetMblk());
        decount++;
         }
          decount1 ++;
#else
        ::Drv_Reclaim( pComMsg->GetBufferPtr() );
#endif
        pComMsg->DeleteBuffer();

#ifdef __WIN32_SIM__
        //Windows.
        CComEntity::DeallocateComMessage( pComMsg );
#else
        //VxWorks.
        pComMsg->SetDstTid( M_TID_EB ); 
        pComMsg->SetSrcTid( M_TID_EB ); 
        pComMsg->SetMessageId( MSGID_TRAFFIC_EGRESS );
        pComMsg->SetFlag( 0 );
        pComMsg->SetEID( 0 );
        pComMsg->SetDataLength( 0 );
        pComMsg->SetDataPtr( NULL );
        pComMsg->SetTimeStamp( 0xffffffff );
        pComMsg->SetDirection( 0xff );
        pComMsg->SetIpType( 0xff );
        pComMsg->SetBTS( 0 );
        pComMsg->SetUdpPtr( NULL );
        pComMsg->SetDhcpPtr( NULL );
        pComMsg->SetKeyMac( NULL );
        pComMsg->SetVlanID(M_NO_VLAN_TAG);
#ifdef WBBU_CODE
        pComMsg->SetMblk(NULL);/**wangwenhua add 20090325**/
#endif
        //空闲的ComMessage插入到链表头
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_plistComMessage);
        m_plistComMessage = pComMsg;
        --g_ulOccupiedCounter;
        ::intUnlock( intKey );
        ::taskUnlock();
#endif
        }
    else
        {
        CComEntity::DeallocateComMessage( pComMsg );
        }

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::RunCleanUp

DESCRIPTION:
    启动CleanUp任务

ARGUMENTS:
    CTBridge*

RETURN VALUE:
    0

SIDE EFFECTS:
    none
==============================================================*/
#ifdef __WIN32_SIM__
DWORD WINAPI CTBridge::RunCleanUp(void *pTask)
#else
STATUS CTBridge::RunCleanUp(CTBridge *pTask)
#endif
{
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "run task CleanUp." );

    ( (CTBridge*)pTask )->doMonitor();
    return 0;
}

STATUS CTBridge::RunWriteCdrFile(CTBridge *pTask)
{
	 ( (CTBridge*)pTask )->cCdrObj.writeFileMainloop();
	 return 0;

}

int CdrTestFlag = 0;
UINT32 gcdrSendPeriod = 900;
/*============================================================
MEMBER FUNCTION:
    CTBridge::doMonitor

DESCRIPTION:
    CleanUp任务进行的转发表表项检查
    CleanUp任务分10次遍历完一遍整个转发表，每次delay 1秒

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::doMonitor()
{
    //do Monitor
    //CleanUp不是FrameWork任务，日志级别必须提高，暂定最高级别Critial.
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "CleanUp start monitoring." );

    FTEntry *pFT;
    MACFilterEntry *pMFT;
    CFTEntryExpire msgFTEntryExp;
    CMFTEntryExpire msgMFTEntryExp;
    UINT16 usEntryIdx = 0;   //表项下标
    UINT16 mftIndex = 0; //MAC Filtering Table 下标
    //每次遍历的表项个数
    UINT16 usMonNum = M_MAX_USER_PER_BTS / M_CLEANUP_MONITOR_FULLCIRCLE + 1;
    UINT16 usRecentEntryIdx = 0xffff;
  //  UINT16 mftRecentIndex   = 0xffff;

    //lijinan 20081204 计费系统增加
    UINT32 secondCnt = 0;
    //UINT32 ftpSecond = 0;
    UINT32 btsId = bspGetBtsID();
    UINT32 ftpInterval = btsId%10;
    UINT32 statTimerCount = 0;   
    UINT32 tcount;

    //循环不退出
    while ( true )
    {
        //统计速率jiaying20110801        
        if(statPrintFlag==1)
        {   
            statTimerCount++;
            if(statTimerCount%statLen == 0)
            {
                tcount = statFromAir*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "EB::stat from air is: %dBps->%dbps -> %d.%.3dMbps, packets: %d\n", tcount/8, tcount, tcount/0x100000, \
                        (tcount%0x100000)/0x400, statPktFromAir/statLen);                 
                tcount = statToAir*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "EB::stat to air is: %dBps->%dbps -> %d.%.3dMbps, packets: %dpps\n", tcount/8, tcount, tcount/0x100000, \
                        (tcount%0x100000)/0x400, statPktToAir/statLen);  
                tcount = statFromWan*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "EB::stat from wan is: %dBps->%dbps --> %d.%.3dMbps, packets: %dpps\n", tcount/8, tcount, tcount/0x100000, \
                    (tcount%0x100000)/0x400, statPktFromWan/statLen); 
                tcount = statToWan*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "EB::stat to wan is: %dBps->%dbps-> %d.%.3dMbps, packets: %dpps\n", tcount/8, tcount, tcount/0x100000, \
                    (tcount%0x100000)/0x400, statPktToWan/statLen); 
                tcount = statFromTDR*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "EB::stat from tunnel is: %dBps->%dbps -> %d.%.3dMbps, packets: %dpps\n", tcount/8, tcount, tcount/0x100000, \
                    (tcount%0x100000)/0x400, statPktFromTDR/statLen);     
                tcount = statToTDR*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "EB::stat to tunnel is: %dBps->%dbps -> %d.%.3dMbps, packets: %dpps\n", tcount/8, tcount, tcount/0x100000, \
                    (tcount%0x100000)/0x400, statPktToTDR/statLen);                    
              
                tcount = statFromWanDrv*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "Drv::stat from driver is: %dBps->%dbps -> %d.%.3dMbps, packets: %dpps\n", tcount/8, tcount, tcount/0x100000,\
                    (tcount%0x100000)/0x400, statPktFromWanDrv/statLen); 
                tcount = statToWanDrv1*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "Drv::stat stack to driver is: %dBps->%dbps -> %d.%.3dMbps, packets: %dpps\n", tcount/8, tcount, tcount/0x100000,\
                    (tcount%0x100000)/0x400, statPktToWanDrv1/statLen); 
                tcount = statToWanDrv2*8/statLen;
                LOG5( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "Drv::stat eb to driver is: %dBps->%dbps -> %d.%.3dMbps, packets: %dpps\n", tcount/8, tcount, tcount/0x100000, \
                    (tcount%0x100000)/0x400, statPktToWanDrv2/statLen); 
		  LOG( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "**************stat end****************************\n"); 	
                statFromAir = 0;
                statFromWan = 0;
                statFromTDR = 0;
                statToAir = 0;
                statToWan = 0;
                statToTDR = 0;
                statPktFromAir = 0;
                statPktFromWan = 0;
                statPktFromTDR = 0;
                statPktToAir = 0;
                statPktToWan = 0;
                statPktToTDR = 0;
               
                statFromWanDrv = 0;
                statToWanDrv1 = 0;
                statToWanDrv2 = 0;
                statPktFromWanDrv = 0;
                statPktToWanDrv1 = 0;
                statPktToWanDrv2 = 0;
               
                //statTimerCount = 0;
            }
            
        }
        //统计速率jiaying20110801 end
        pMFT = GetMFTEntryByIdx( mftIndex );
        if ( true == pMFT->IsOccupied ) 
        {
            if ( pMFT->TTL <= pMFT->Elapsed )
            {
                //发现表项过期，发消息通知Bridge任务
                if( true == msgMFTEntryExp.CreateMessage( *( CTBridge::GetInstance() ) ) )
                {
                    msgMFTEntryExp.SetMac(pMFT->MAC);
#if 0                   
                    msgMFTEntryExp.SetType(pMFT->TYPE);
#endif
                    msgMFTEntryExp.SetDstTid( M_TID_EB );
                    
                    //发送
                    if ( false == msgMFTEntryExp.Post() )
                    {
                        LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "task CleanUp Send Message failed." );
                        msgMFTEntryExp.DeleteMessage();
                    }
                }
#ifndef M_TGT_WANIF
                ResolveRouterMAC();
#else
                if(Wanif_Switch!=0x5a5a)
   	        {
   	           #ifndef WBBU_CODE
   	             ResolveRouterMAC();
			#endif
   	        }
#endif
            }
            else
            {
                //表项目前没有过期，Elapsed time + 遍历周期
                pMFT->Elapsed += M_MAX_MACFILTER_TABLE_NUM;//seconds
            }
                
            
        }
        if(++mftIndex >=M_MAX_MACFILTER_TABLE_NUM) 
            mftIndex = 0;

        for ( UINT16 usIdx = 0; usIdx < usMonNum; ++usIdx, ++usEntryIdx )
            {
            pFT = GetFTEntryByIdx( usEntryIdx );
            if ( NULL == pFT )
                {
                //到转发表尾，跳出从头开始
                usEntryIdx = 0;
                if(usRecentEntryIdx == 0)
                    usRecentEntryIdx=0xffff;
                break;
                }

            //
            if ( false == pFT->bIsOccupied ) 
                {
                continue;
                }

            if((usRecentEntryIdx !=usEntryIdx) && (usRecentEntryIdx!=0xffff))
                {
                EBSendBTSIPChangeReq(*pFT);
                }
            else if(usRecentEntryIdx ==usEntryIdx)
                {
                usRecentEntryIdx = 0xffff;
                }

            //处理更新PubIP&Port的请求
            if(GetNotifyBTSPubIP() )
                {
                usRecentEntryIdx=usEntryIdx;
                SetNotifyBTSPubIP(false);
                EBSendBTSIPChangeReq(*pFT);
                }

            if ( M_TTL_FFFF == pFT->usTTL )
                {
                //DHCP.
#ifdef BUFFER_EN
                //处理缓存的数据链表
                if ( NULL != pFT->pBufList )
                    {
                    pFT->pBufList->Reclaim();
                    }
#endif
                continue;
                }

            //PPPoE/FixIP
            if ( pFT->usTTL <= pFT->usElapsed )
                /*<改成<=,原因:删除转发表时，TTL,Elapse都=0,希望10秒钟后由cleanUp删除
                 *但如果此时不断有上行或下行数据，Elapse会不断置0，导致表项长期不能被删除
                 *所以改成<=时都删除表项
                 */
                {
                //发现表项过期，发消息通知Bridge任务
                if( false == msgFTEntryExp.CreateMessage( *( CTBridge::GetInstance() ) ) )
                    {
                    break;
                    }
                msgFTEntryExp.SetMac( pFT->aucMAC );
                msgFTEntryExp.SetDstTid( M_TID_EB );
                //发送
                if ( false == msgFTEntryExp.Post() )
                    {
                    LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "task CleanUp Send Message failed." );
                    msgFTEntryExp.DeleteMessage();
                    }
                }
            else
                {
                //表项目前没有过期，Elapsed time + 遍历周期
                pFT->usElapsed += M_CLEANUP_MONITOR_FULLCIRCLE;
#ifdef BUFFER_EN
                //处理缓存的数据链表
                if ( NULL != pFT->pBufList )
                    {
                    pFT->pBufList->Reclaim();
                    }
#endif
                }
            }

#ifdef __WIN32_SIM__
        ::Sleep( 1000 ); //delay 1 seconds
#else
        ::taskDelay( 100 ); //delay 1 seconds
#endif

	//lijinan 20081204 计费系统增加------------------------------
	if(uiCdrSwitch == 1)
	{
		if(secondCnt%gcdrSendPeriod==ftpInterval) //15分钟
		{
			
			//cCdrObj.CdrDataFileUploadTimer();
			//改成发送消息
			//if(uiCdrSwitch == 1)
			cCdrObj.sendMsgToWriteFileTask(From15MinTimer,(UINT16)0);
			
		}
		if(gNeedSendCFFileToFtp==FLAG_YES)
		{
			if(secondCnt%120==0)
			{
				//if(uiCdrSwitch == 1)
					cCdrObj.sendMsgToWriteFileTask(FromSendCfToFtp,(UINT16)0);
			}
		}
		if(rt_bill_need_ftp)
		{
			if(secondCnt%60==0)
			{
				cCdrObj.sendMsgToWriteFileTask(From15MinTimer,(UINT16)0);
				rt_bill_need_ftp = FLAG_NO;
				if(CdrTestFlag)
				{
					printf("\nrt_bill_need_ftp send to cdr tsk\n");
				}
			}
		}

		//lijinan  计费系统ftp 故障的处理
		if(gFlagInFtp==FLAG_YES)
		{
			ftpSecond++;
			if(ftpSecond==300)
			{
				LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "task cdr fail,restart cdr task." );
				restartcdrTsk();
				gFlagInFtp = FLAG_NO;
			}

		}
		else
			ftpSecond = 0;
		cCdrObj.spyCdrMacTbl();
		time_beart_cb3000++;
		if(time_beart_cb3000==900)
		{
			time_beart_cb3000 = 0;
			cCdrObj.delWifiUser();
		}
	}
	//-----------------------------------------------------------------
	 secondCnt++;
	 if(secondCnt%300==0&&cdr_para_err_flag)
	 {
		LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "cdr para have no config,pls config" );
	 }
	//-----------------------------------------------------------------

        }

    return;
}




/*============================================================
MEMBER FUNCTION:
    CTBridge::InitFreeFTList

DESCRIPTION:
    初始化空闲转发表表项链表m_listFreeFT

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::InitFreeFTList()
{
    m_listFreeFT.clear();
    for ( UINT16 usIdx = 0; usIdx < M_MAX_USER_PER_BTS; ++usIdx )
        {
        m_listFreeFT.push_back( usIdx );
        }
}


#ifndef __WIN32_SIM__
/*============================================================
MEMBER FUNCTION:
    CTBridge::InitComMessageList

DESCRIPTION:
    初始化ComMessage链表，初始化=2000个。
    这个链表只允许给网卡驱动使用

ARGUMENTS:
    NULL

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::InitComMessageList()
{
    for( UINT16 usIdx = 0; usIdx < M_MAX_LIST_SIZE; ++usIdx )
        {
#if 0
        stComMessageNode *pComMsgNode = (stComMessageNode *)new stComMessageNode;
        if ( NULL == pComMsgNode )
            {
            return false;
            }
#endif
        CComMessage      *pComMsg     = new ( this, 0 )CComMessage;
        if ( NULL == pComMsg )
            {
            //delete pComMsgNode;
            return false;
            }

        //pComMsgNode->pstComMsg = pComMsg;
        //只是网卡驱动给EB任务发送消息
        pComMsg->SetDstTid( M_TID_EB ); 
        pComMsg->SetSrcTid( M_TID_EB ); 
        pComMsg->SetMessageId( MSGID_TRAFFIC_EGRESS );
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_plistComMessage);
        m_plistComMessage = pComMsg;
        ::intUnlock( intKey );
        ::taskUnlock();
        }

    return true;
}
#endif


/*============================================================
MEMBER FUNCTION:
    CTBridge::InsertFreeFTList

DESCRIPTION:
    插入空闲转发表表项下标到链表m_listFreeFT尾部

ARGUMENTS:
    usIdx:表项下标

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::InsertFreeFTList(UINT16 usIdx )
{
    if( usIdx < M_MAX_USER_PER_BTS )
        {
        m_listFreeFT.push_back( usIdx );
        }
    else
        {
        DATA_assert( 0 );
        }
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::GetFreeFTEntryIdxFromList

DESCRIPTION:
    从空闲链表m_listFreeFT取空闲转发表表项下标;(从链表头部取)

ARGUMENTS:
    NULL

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTBridge::GetFreeFTEntryIdxFromList()
{
    if ( true == m_listFreeFT.empty() )
        {
        g_eb_no_ft_freelist++;
        return M_DATA_INDEX_ERR;
        }

    UINT16 usIdx = *m_listFreeFT.begin();
    m_listFreeFT.pop_front();

    if ( M_MAX_USER_PER_BTS <= usIdx )
        {
        //下标错误
        g_eb_no_ft_freelist++;
        return M_DATA_INDEX_ERR;
        }
   g_eb_no_ft_freelist = 0;
    return usIdx;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::BPtreeAdd

DESCRIPTION:
    转发表索引树插入节点；转发表索引树以Mac地址作键值，把转发表
    表项下标插入到索引树

ARGUMENTS:
    Mac:Mac地址
    usIdx:表项下标

RETURN VALUE:
    bool:插入成功/失败

SIDE EFFECTS:
    如果存在重复项，将会返回失败，所以必须在BPtreeAdd之前保证
    没有重复。即首先 BPtreeFind 一下
==============================================================*/
bool CTBridge::BPtreeAdd(CMac &Mac, UINT16 usIdx)
{
    if ( ( true == Mac.IsZero() ) || ( true == Mac.IsBroadCast() ) )
        {
        UINT8 strMac[ M_MACADDR_STRLEN ];
        LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), 
               "CTBridge::BPtreeAdd() try to add an entry with Mac:%s ", (int)Mac.str( strMac ) );
        return false;
        }
    pair<map<CMac, UINT16>::iterator, bool> stPair;

    stPair = m_FTBptree.insert( ValType( Mac, usIdx ) );
    /*
     *必须保证不存在重复项，否则将会返回失败
     */
    return stPair.second;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::BPtreeDel

DESCRIPTION:
    转发表索引树删除Mac对应的节点；

ARGUMENTS:
    Mac:Mac地址

RETURN VALUE:
    bool:删除成功/失败

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::BPtreeDel(CMac &Mac)
{
    map<CMac, UINT16>::iterator it;
    
    if ( ( it = m_FTBptree.find( Mac ) ) != m_FTBptree.end() )
        {
        //find, and erase;
        m_FTBptree.erase( it );
        }
    //not find
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::BPtreeFind

DESCRIPTION:
    从BPtree搜索Mac地址对应的转发表表项下标

ARGUMENTS:
    Mac:Mac地址

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTBridge::BPtreeFind(CMac &Mac)
{
    map<CMac, UINT16>::iterator it = m_FTBptree.find( Mac );
    
    if ( it != m_FTBptree.end() )
        {
        //返回Index.
        return it->second;
        }
    return M_DATA_INDEX_ERR;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::SetWorkingMode

DESCRIPTION:
    设置工作模式

ARGUMENTS:
    ucWorkingMode:工作模式

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::SetWorkingMode(UINT8 ucWorkingMode)
{
    if ( (UINT8)WM_ERR_MODE > ucWorkingMode )
        {
        m_ucWorkingMode = ucWorkingMode;
        }
    else
        {
        LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "CTBridge::SetWorkingMode( %d ): mode out of range", ucWorkingMode );
        }
}


void CTBridge:: EidCdrAdminStatus(CComMessage *pComMsg)
{
	UINT32 uid;
	UINT8 utType;
	char bw[8];
       UINT32  eid = pComMsg->GetEID();
	if(eidIsWifiEid(eid))//wifi eid
	{
		cCdrObj.DelEid(eid,1);
		return;
	}
	UINT16 cdrIndex = cCdrObj.BPtreeFind( eid);
	if(cdrIndex>=M_MAX_UT_PER_BTS)
	{
	     return;
	}
	UINT8 *p = (UINT8 *)pComMsg->GetDataPtr();
	UINT8 adminstatus =  p[0];
	if(M_DATA_INDEX_ERR==cdrIndex)
	{
		if(CdrTestFlag)
 		     logMsg("\nEidCdrAdminStatus cannot find :%x,%x......\n",eid ,p[0],0,0,0,0);
		return;
	}
	if(adminstatus==0)
		cCdrObj.SetNoPayFlag(cdrIndex, adminstatus);
	else
		cCdrObj.SetNoPayFlag(cdrIndex, 1);
	cCdrObj.func_clear_Rt_data(cdrIndex);
	uid = findUidFromEid(eid,&adminstatus,&utType,bw);
	if(uid!=cCdrObj.getCdrUid(cdrIndex))
	{
		cCdrObj.uidChange(uid,cdrIndex);
	}
	cCdrObj.setCdrUid(cdrIndex,uid);
	cCdrObj.setUtType(cdrIndex,utType);
	cCdrObj.setUtUpDownBW(cdrIndex,bw);

}

void CTBridge::func_moveAway_RT_Charge(CComMessage*pComMsg)
{
	UINT32 eid = pComMsg->GetEID();
	UINT16 cdrIndex = cCdrObj.BPtreeFind( eid);
	//if(M_DATA_INDEX_ERR==cdrIndex)
	if((M_MAX_UT_PER_BTS -1)<cdrIndex)
	{
		if(CdrTestFlag)
 		     printf("\nfunc_moveAway_RT_Charge cannot find eid:0x%x....,..%d\n",eid,cdrIndex);
		return;
	}
	cCdrObj.func_moveAway_Rt_cdr(cdrIndex);
}
void CTBridge:: proMsgFromCb3000(CComMessage *pComMsg)
{
	UINT8 *p = (UINT8 *)pComMsg->GetDataPtr();
	UINT16 msg_type,result,len,cdrIndex;
	UINT32 eid,wifi_uid,trans_id,wifi_ip;
	char buf[32];
	memcpy((char*)&msg_type,(char*)(p+6),2);
	p+=6;
	CMac wif_mac0( p+10);
	CMac wif_mac( p+16);
	memcpy((char*)&trans_id,(p+2),4);
	 CTransaction *pTransaction = FindTransact(trans_id);
    	if (NULL != pTransaction)
        {
	        pTransaction->EndTransact();
	        delete pTransaction;
		 if(CdrTestFlag)
		 	LOG1( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "msg from cb3000 have get trans_id:%d.",trans_id );
    	}
	if(CdrTestFlag)
		LOG1( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "msg from cb3000 type:%d.",msg_type );
	switch(msg_type)
	{
		case WIFI_USER_ACC_RSP:
			/*字段名称	长度（Byte）	类型	描述
			MESSAGE TYPE	2	    					M	0x02，接入响应
			Session ID		4						M	
			Result			2						M	00：成功01：失败
			Hot spot CPE UID	4						M	Result为成功时携带
			User IP			4						M	Result为成功时携带
			User MAC		6						M	Result为成功时携带
			User Wifi UID		4						M 	用户的虚拟UID
			*/
			memcpy((char*)&result,(char*)(p+6),2);
			p+=8;
			memcpy((char*)&wifi_uid,(p+14),4);
			memcpy((char*)&eid,p,4);
			 cdrIndex = cCdrObj.BPtreeFind(wif_mac);
			if(cdrIndex>=M_MAX_UT_PER_BTS)
			{
				LOG2( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "wifi_uid:0x%x,eid:0x%x get cdr index err.",wifi_uid,eid );
				return;
			}
			if(result==0)
			{
					cCdrObj.SetWifiUid(cdrIndex, wifi_uid);
			}
			else
			{
				wifi_uid = 0xffffffff;
				cCdrObj.SetWifiUid(cdrIndex, wifi_uid);
				if(CdrTestFlag)
					LOG2( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "eid:0x%x,wifi_uid acc req fail,result:%d.",eid,result);
			}
			break;
		case CB3000_BTS_HEART_REQ:
			/*直接回心跳响应*/
			/*
			字段名称	长度（Byte）	类型	描述
				MESSAGE TYPE	2	M	0x03，心跳请求
				User1 session ID	4	M	
				User1 MAC	6	M	
				User1 Wifi UID	4	M	
				……			
				User session ID	4	M	
				User MAC	6	M	
				User Wifi UID	4	M	

			*/
			time_beart_cb3000 = 0;
			cCdrObj.func_heart_pro((char*)pComMsg->GetDataPtr());
		       len = 2;
			buf[0] = 0;
			buf[1] = BTS_CB3000_HEART_RSP;
			sendMsg2CB3000(buf,len);
			break;
		case WIFI_USER_LOG_OUT_REQ:
			/*
			      字段名称	长度（Byte）	类型	描述
				MESSAGE TYPE	2				M		0x04，注销请求
				Session ID		4				M	
				User Wifi UID		4				Ｍ		注销用户虚拟UID
				User MAC		6				M	
				User IP			4				M	
			*/
			memcpy((char*)&trans_id,(p+2),4);
			memcpy((char*)&wifi_uid,(p+6),4);
			memcpy((char*)&wifi_ip,(p+16),4);
			//CMac wif_mac( p+8);
			cdrIndex = cCdrObj.BPtreeFind(wif_mac0);
			if(cdrIndex>=M_MAX_UT_PER_BTS)
			{
				LOG1( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "wifi_uid:0x%x,%x get cdr index err.",wifi_uid );
				return;
			}
			else
			{
				cCdrObj.DelWifiMac(wif_mac0);	
			}
			len = 10;
			buf[0] = 0;
			buf[1] = WIFI_USER_LOG_OUT_RSP;
			memcpy(&buf[2],(char*)&trans_id,4);
			memcpy(&buf[6],(char*)&wifi_ip,4);
			sendMsg2CB3000(buf,len);
			break;
		case RT_CHARGE_RSP:
		case RT_CHARGE_INFORM:
			/*字段名称	长度（Byte）	类型	描述
			MESSAGE TYPE	2						M	0x08
			TransId			4						M	
			UID				4						M	
			MAC				6						M	0表示不需要解析
			FLAG_Final		1						M	0：非最后业务量 1：最后的业务量
			FLAG_Traffic		1						M	0：不启用流量 1：启用流量
			FLAG_Time		1						M	0：不启用时长 1：启用时长
			TimeLen			2						M	若时长超过65535填写65535,单位s
			Traffic			4						M	若流量超过65535填写65535,单位kb
			*/
			char mac_buf[6];
			memcpy(mac_buf,(p+10),6);
			if((mac_buf[0]==0)&&(mac_buf[1]==0)&&(mac_buf[2]==0)&&(mac_buf[3]==0)&&(mac_buf[4]==0)&&(mac_buf[5]==0))
			{
				cCdrObj.func_rtCharge_rsp((p+6),0);
			}
			else
			{
				cCdrObj.func_rtCharge_rsp((p+6),1);
			}
			if(msg_type==RT_CHARGE_INFORM)
			{
				memcpy((char*)&trans_id,(p+2),4);
				len = 10;
				buf[0] = 0;
				buf[1] = RT_CHARGE_INFORM_RSP;
				memcpy(&buf[2],(char*)&trans_id,4);
				memcpy(&buf[6],(p+6),4);
				sendMsg2CB3000(buf,len);
			}
			break;
		case RT_CHARGE_REPT_RSP:
		case RT_CHARGE_RPT_PERIOD_ACK:
			break;
		default:
			LOG1( LOG_DEBUG1, LOGNO( EB, EC_EB_NORMAL ), "msg from cb3000 type:%d err.",msg_type );
			break;
			
	}
}


void CTBridge::func_msg_fail(CComMessage *pComMsg)
{
	char *p = (char *)pComMsg->GetDataPtr();
	UINT16 msg_len = pComMsg->GetDataLength();
	int cdr_num = msg_len/sizeof(_CDR);
	for(int i =0;i<cdr_num;i++)
	{
		cCdrObj.WriteToFile(p);
		p+= sizeof(_CDR);
	}
	rt_bill_need_ftp = FLAG_YES;
	if(CdrTestFlag)
	{
		printf("\nrec rt bill fail msg len:%d\n",msg_len);
	}
}

//lijinan 20101020 for video
void CTBridge:: updateVideoIp(CComMessage *pComMsg)
{
       UINT32  ulEID = pComMsg->GetEID();
	UINT8 *p = (UINT8 *)pComMsg->GetDataPtr();
	FTEntry *pFT;
       map<CMac, UINT16>::iterator it = m_FTBptree.begin();
        while ( m_FTBptree.end() != it )
        {
	        pFT = GetFTEntryByIdx( it->second );
	        if ( ( 0 != ulEID ) && ( pFT->ulEid != ulEID ) )
	            {
	            ++it;
	            continue;
	            }
         
	if(pFT==NULL)
	{
		logMsg("\nupdateVideoIp cannot find pFT:%x,%x......\n",ulEID ,p[0],0,0,0,0);
	}
	else
	{
		if(p[0]==0)
		{
			memset(pFT->video_ip,0,8);
		}
		else if(p[0]==1)
		{
			memcpy(pFT->video_ip,p+1,4);
			pFT->video_ip[1] = 0;
		}
		else if(p[0]==2)
		{
			memcpy(pFT->video_ip,p+1,8);
		}
		else
		{
			;//参数出错
		}
		if(video_debug)
		{
			logMsg("\nEB rec eid:0x%x,updateVideoIp,num:%d,ip1:0x%x,ip2:0x%x\n",ulEID,p[0],pFT->video_ip[0],pFT->video_ip[1],0,0);
		}
		
	}
	 ++it;
}
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::Ingress

DESCRIPTION:
    上行数据转发处理函数

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
   void

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::IsMyBTSARP(CComMessage *pComMsg,UINT32 *srcIP)
{
    EtherHdr *pEtherPkt = (EtherHdr*) ( pComMsg->GetDataPtr() );
    ArpHdr *pArphdr;
    if(IS_8023_PACKET(ntohs(pEtherPkt->usProto)))
    {
        pArphdr = (ArpHdr*)((UINT8*)pEtherPkt + sizeof(EtherHdr) + sizeof(LLCSNAP));
    }
    else
    {
        pArphdr = (ArpHdr*)((UINT8*)pEtherPkt + sizeof(EtherHdr));
    }
    if( pArphdr->ulDestPaddr == dwIP) //用户试图访问本基站
    {
    	*srcIP = pArphdr->ulSenderPaddr;
	return true;
    }
    return false;	   
	   
}

void CTBridge::Ingress(CComMessage *pComMsg)
{
    #ifdef M_TGT_WANIF
//    bool result;
    unsigned int  relayeid;
    char flag = 0;
    #endif
#ifdef WBBU_CODE
    L2_L3_Ingress++;
#endif
    bool res;
    
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge Ingress()" );
    //统计从二层收到的数据包总长度jiaying20110801    
    statFromAir += pComMsg->GetDataLength();    
    statPktFromAir++;
    //统计从二层收到的数据包总长度jiaying20110801 end
    if (pComMsg->GetDataLength() >= 1600)
    {
        DATA_log(pComMsg, LOG_DEBUG1, 0, "invalid uplink Data length %d", pComMsg->GetDataLength());
        return; 
    }
 //wangwenhua add 2012-2-26  统计上行发送给数据网关的包
    CMac   tempMac1(g_rf_openation.GateWay1_MAC);
    CMac   tempMac2(g_rf_openation.GateWay2_MAC);

    //增加MAC filter功能
    CMac SrcMac( GetSrcMac( pComMsg ) );
    CMac  DstMac(GetDstMac( pComMsg ) );

   if(g_rf_openation.type!=0)
  {
        if(g_rf_openation.GateWay1_valid)
        {
               if(tempMac1==DstMac)
               {
                   g_rf_openation.GateWay1_UP++;
               }
        }
	     if(g_rf_openation.GateWay2_valid)
        {
               if(tempMac2==DstMac)
               {
                   g_rf_openation.GateWay2_UP++;
               }
        }
  }
    if(true==SrcMac.IsZero())//wangwenhua add 20080926
    {    
        return;    
    }
    if ( true == SrcMac.IsBroadCast() )
    {
        //
        DATA_log(pComMsg, LOG_WARN, LOGNO( EB, EC_EB_MSG_EXCEPTION ), "Err! source MAC address is a broad/multicast address, discard packet.");
        return;
    }
#ifdef TRUNK_CODE_VLAN

	UINT8    *pData = (UINT8*)( pComMsg->GetDataPtr() );
	VLAN_hdr *pVlan = (VLAN_hdr*)( pData + 12 );
				
	if ( M_ETHER_TYPE_VLAN == ntohs( pVlan->usProto_vlan ) )
	{
		pComMsg->SetVlanID(pVlan->usVlanID);//not groupId
		//去掉VLAN tag.
		UINT8 *p = pData + 11;
		for ( UINT8 idx = 12; idx > 0; --idx, --p )
		{
		  *( p + 4 ) = *p;
		}
		pComMsg->SetDataPtr( pData + 4 );
		if (pComMsg->GetDataLength() >= 64)
		{
			pComMsg->SetDataLength( pComMsg->GetDataLength() - 4 );
		}
		else
		{   
			pComMsg->SetDataLength( 60 );   // to make sure the packet is longer than the minimum Ethernet packet 
			// requirement, to prevent the packets being discarded 
		}
	}
	else
	{
		pComMsg->SetVlanID(M_NO_VLAN_TAG);//not groupId
	}
  
#endif
    MACFilterEntry *pMFT = GetMFTEntryByIdx( MFTBPtreeFind( SrcMac ) );
    if(NULL != pMFT)
    {
        DATA_log(pComMsg, LOG_WARN, LOGNO(EB, EC_EB_MSG_EXCEPTION), "MAC[%x-%x-%x-%x-%x-%x] has been in MAC filtering table, will not add into forwarding table.", pMFT->MAC[0], pMFT->MAC[1], pMFT->MAC[2], pMFT->MAC[3], pMFT->MAC[4], pMFT->MAC[5]);
        return;
    }
    if(Wanif_Switch == 0x5a5a)
    {
        UINT32 teid =pComMsg->GetEID();
        if((teid!=0)&&((teid == WanIfCpeEid)||(teid == BakWanIfCpeEid))&&(teid!=WorkingWcpeEid))
        {
            DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_MSG_EXCEPTION ), "wcpe mode, not working wcpe send packet, return, eid:%x", teid);
            return;
        }
    }
    if ( WM_LEARNED_BRIDGING == m_ucWorkingMode )
    {
        //学习模式
        LearnedUplinkTraffic( pComMsg ,0);
    }
       
    //安全模式
    //UINT32 ulLen = pComMsg->GetDataLength();
    UINT16 usProto = GetProtoType( pComMsg );
    
    //设置 Client Mac Address (Source Mac) 指针
    EtherHdr *pEther = (EtherHdr*)( pComMsg->GetDataPtr() );
    pComMsg->SetKeyMac( pEther->aucSrcMAC );
    if(Ping_Check_Flag == 1)
    {
        GetPingPacket_Seq(pComMsg,0);
        GetPingACKPacket_Seq(pComMsg,0);
    }
    //增加Ingress方向的性能统计值
    FROMDIR from = EB_FROM_AI;
    
    if ( M_ETHER_TYPE_ARP == usProto )
    {
        UINT32 srcIP;	
        if(IsMyBTSARP(pComMsg,&srcIP))
        {
            recordCPEToBTSIPStack(GetSrcMac( pComMsg ),pComMsg->GetEID(),srcIP); 
            SendToBTSIPStack(pComMsg);   
            return;       	
        }  	
        
        FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
        if ( NULL == pSrcFT )
        {
            //Illegal User Packet++;
            IncreaseDirTrafficMeasureByOne( from, TYPE_ILLEGAL_USER );
            //是否是FixIp?是则需要重建FixIp用户信息
            res = CTaskDm::GetInstance()->BuildFixIpContext( SrcMac, pComMsg->GetEID() );
            if(res == false)
            {            
            
            }        
        }
#ifdef RCPE_SWITCH
		else
		{
			if( trunkIsMRcpe(pSrcFT->ulEid) )
			{
				if( pSrcFT->ulEid != pComMsg->GetEID() )
				{
					CComMessage *RspMsg = new ( CTBridge::GetInstance(), M_MAC_ADDRLEN ) CComMessage;
					if( RspMsg != NULL )
					{
						memcpy( (UINT8*)RspMsg->GetDataPtr(), pSrcFT->aucMAC, M_MAC_ADDRLEN );
						RspMsg->SetDstTid( M_TID_UTDM );
						RspMsg->SetMessageId( M_CPE_L3_TRUNK_MAC_MOVEAWAY_NOTIFY );
						RspMsg->SetEID( pSrcFT->ulEid );
						RspMsg->SetSrcTid( (CTBridge::GetInstance())->GetEntityId());
						RspMsg->SetDirection( DIR_TO_AI );
						if( ! CComEntity :: PostEntityMessage( RspMsg ) )
						{
						    RspMsg->Destroy();
							//return false;
						}
					}
                }
			}
		}
#endif
        #ifdef M_TGT_WANIF         
        relayeid = pComMsg->GetEID();
        if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)//rcpe修改，扩大到所有类型ip，wangwenhua20100721
        {
            for(int i =0;i<NvRamDataAddr->Relay_num;i++)
            {
                if(i>=20)
                	{
                	    break;
                	}
                if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                {
                    flag =1;
                    break;
                }            
            }
        }
        if (flag==1)
        {        
            LearnedUplinkTraffic( pComMsg, 1 );        
        }
        else
        {
            //notifyOneCpeToRegister(pComMsg->GetEID(),true);
            // return;
        }			
        #endif							  
        // ARP packet.
        pComMsg->SetDirection( DIR_FROM_AI );
        //            pComMsg->SetPara(pComMsg->GetEID());
        
        pComMsg->SetDstTid( M_TID_ARP );
        pComMsg->SetSrcTid( M_TID_EB );
         pComMsg->SetIpType( IPTYPE_ARP );
        pComMsg->SetMessageId( MSGID_ARP_PROXY_REQ );
        CComEntity::PostEntityMessage( pComMsg );
        
        //Ingress ARP 包统计值++
        IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_ARP );
        return;
    }
    else if ( M_ETHER_TYPE_PPPoE_DISCOVERY == usProto )
    {    
        FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
        if ( NULL == pSrcFT )
        {
            //Illegal User Packet++;
            IncreaseDirTrafficMeasureByOne( from, TYPE_ILLEGAL_USER );
            //是否是FixIp?是则需要重建FixIp用户信息
            res = CTaskDm::GetInstance()->BuildFixIpContext( SrcMac, pComMsg->GetEID() );
            if(res == false)
            {
            
            }
            //  return;
        }			
        #ifdef M_TGT_WANIF         
        relayeid = pComMsg->GetEID();
        if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)//rcpe修改，扩大到所有类型ip，wangwenhua20100721
        {
            for(int i =0;i<NvRamDataAddr->Relay_num;i++)
            {
               if(i>=20)
               	{
               	    break;
               	}
                if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                {
                    flag =1;
                    break;
                }            
            }
        }
        if (flag==1)
        {        
            LearnedUplinkTraffic( pComMsg, 1 );        
        }
        else
        {
        //notifyOneCpeToRegister(pComMsg->GetEID(),true);
        // return;
        }			
        #endif		
        //PPPoE Discovery Stage packet.
        pComMsg->SetDirection( DIR_FROM_AI );
        pComMsg->SetIpType( IPTYPE_PPPoE );
        
        pComMsg->SetDstTid( M_TID_SNOOP );
        pComMsg->SetSrcTid( M_TID_EB );
        
        pComMsg->SetMessageId( MSGID_TRAFFIC_SNOOP_REQ );
        CComEntity::PostEntityMessage( pComMsg );
        
        //Ingress PPPoE Discovery stage 包统计值++
        IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_PPPoEDISCOVERY );
        #if 0
        //
        FilterMac( pComMsg, EB_MACFILTER_PPPOESERVER );
        #endif
        return;
    }
    else if ( M_ETHER_TYPE_PPPoE_SESSION == usProto )
    {
        //PPPoE Session Stage packet.
        ForwardUplinkPacket( pComMsg );
        return;
    }
    else
    {
        //other packets.
        UINT8 ucType = IsDhcpPacket( pComMsg );
        if ( M_DHCP_TO_SERVER == ucType )
        {
            //DHCP Client packet.上行只转发DHCP Client报文
            
            FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
            if ( NULL == pSrcFT )
            {
                //Illegal User Packet++;
                IncreaseDirTrafficMeasureByOne( from, TYPE_ILLEGAL_USER );
                //是否是FixIp?是则需要重建FixIp用户信息
                res = CTaskDm::GetInstance()->BuildFixIpContext( SrcMac, pComMsg->GetEID() );
                if(res == false)
                {
                
                }
                //return;
            }
#ifdef RCPE_SWITCH
			else
			{
				if( trunkIsMRcpe(pSrcFT->ulEid) )
				{
					if( pSrcFT->ulEid != pComMsg->GetEID() )
					{
						CComMessage *RspMsg = new ( CTBridge::GetInstance(), M_MAC_ADDRLEN ) CComMessage;
						if( RspMsg != NULL )
						{
							memcpy( (UINT8*)RspMsg->GetDataPtr(), pSrcFT->aucMAC, M_MAC_ADDRLEN );
							RspMsg->SetDstTid( M_TID_UTDM );
							RspMsg->SetMessageId( M_CPE_L3_TRUNK_MAC_MOVEAWAY_NOTIFY );
							RspMsg->SetEID( pSrcFT->ulEid );
							RspMsg->SetSrcTid( (CTBridge::GetInstance())->GetEntityId());
							RspMsg->SetDirection( DIR_TO_AI );
							if( ! CComEntity :: PostEntityMessage( RspMsg ) )
							{
							    RspMsg->Destroy();
								//return false;
							}
						}
	                }
				}
			}
#endif

        
            #ifdef M_TGT_WANIF         
            relayeid = pComMsg->GetEID();
            if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)//rcpe修改，扩大到所有类型ip，wangwenhua20100721
            {
                for(int i =0;i<NvRamDataAddr->Relay_num;i++)
                {
                   if(i>=20)
                   	{
                   	    break;
                   	}
                    if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                    {
                        flag =1;
                        break;
                    }
                }
            }            
            if (flag==1)
            {		       
                LearnedUplinkTraffic( pComMsg ,1 );
                // ForwardUplinkPacket( pComMsg ); //pppoe from rcpe ,we donot do snoop, lwd
            }
            else
            {
                //notifyOneCpeToRegister(pComMsg->GetEID(),true);
            }
            #endif				
            pComMsg->SetDirection( DIR_FROM_AI );
            pComMsg->SetIpType( IPTYPE_DHCP );
            
            pComMsg->SetDstTid( M_TID_SNOOP );
            pComMsg->SetSrcTid( M_TID_EB );
            pComMsg->SetMessageId( MSGID_TRAFFIC_SNOOP_REQ );
            CComEntity::PostEntityMessage( pComMsg );
            
            //Ingress DHCP 包统计值++
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_DHCP );
            #if 0               
            //
            FilterMac( pComMsg, EB_MACFILTER_DHCPSERVER );
            #endif
        }
        else
        {
            //转发
            ForwardUplinkPacket( pComMsg );
        }
        return;
    }
    
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::FilterMac

DESCRIPTION:
    把指定消息报文的目的MAC加入到过滤地指表.广播地址除外

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::FilterMac(const CComMessage *pComMsg, const UINT8 type)
{
    CMac DstMac( GetDstMac( pComMsg ) );
    if(false == DstMac.IsBroadCast())
        {
        CMFTAddEntry msg;
        if(true == msg.CreateMessage(*this))
            {
            msg.SetMac(DstMac.GetMac());
#if 0           
            msg.SetType(type);
#endif
            MFTAddEntry(msg);
            msg.DeleteMessage();
            }
        }
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::IsDhcpPacket

DESCRIPTION:
    判断CommMessage里Payload是否是DHCP报文，
    如果是，设置ComMessage里的DHCP指针和ClientMacAddress指针
    和CPE上UTDM任务的同名函数保持一致

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    0  : 不是DHCP报文
    1  :DHCP Server发出的报文
    2  :DHCP Client发出的报文

SIDE EFFECTS:
    none
==============================================================*/
UINT8 CTBridge::IsDhcpPacket(CComMessage *pComMsg)
{
    if ( M_ETHER_TYPE_IP != GetProtoType( pComMsg ) )
        {
        //DHCP包是IP包
        return M_DHCP_TYPE_ERR;
        }
	IpHdr *pIp;
    UINT8    *pData = (UINT8*)( pComMsg->GetDataPtr() );
          VLAN_hdr *pVlan = (VLAN_hdr*)( pData + 12 );
 if ( M_ETHER_TYPE_VLAN == ntohs( pVlan->usProto_vlan ) )
       {
               EtherHdrEX *pEtherPkt1 = (EtherHdrEX*)(pComMsg->GetDataPtr());
    
	    if(IS_8023_PACKET(ntohs(pEtherPkt1->usProto)))
	    {
	        pIp = (IpHdr*)( (UINT8*)pEtherPkt1 + sizeof(EtherHdrEX) + sizeof(LLCSNAP));
	    }
	    else
	    {
	        pIp = (IpHdr*)( pEtherPkt1 + 1 );
	    }
 	}
 else
 	{
		    EtherHdr *pEtherPkt = (EtherHdr*)(pComMsg->GetDataPtr());
		    
		    if(IS_8023_PACKET(ntohs(pEtherPkt->usProto)))
		    {
		        pIp = (IpHdr*)( (UINT8*)pEtherPkt + sizeof(EtherHdr) + sizeof(LLCSNAP));
		    }
		    else
		    {
		        pIp = (IpHdr*)( pEtherPkt + 1 );
		    }
 	}
    if( M_PROTOCOL_TYPE_TCP == pIp->ucProto )//wangwenhua add 20101202 for gehua only cmall tcp packet priority; 
  	{
  	     pComMsg->SetIpType(IPTYPE_TCP);
  	}
    if( M_PROTOCOL_TYPE_ICMP==pIp->ucProto )
        {
   	     pComMsg->SetIpType(IPTYPE_ICMP);  	
        }
    if ( M_PROTOCOL_TYPE_UDP != pIp->ucProto)
        {
        //DHCP包是UDP包
        return M_DHCP_TYPE_ERR;
        }

    UINT16 usIpHdrLen = ( ( pIp->ucLenVer ) & 0x0f ) * 4;
    UdpHdr *pUdp = (UdpHdr*) ( (UINT8*)pIp + usIpHdrLen );
    UINT16 usSrcPort = ntohs( pUdp->usSrcPort );
    UINT16 usDstPort = ntohs( pUdp->usDstPort );

    if ( ( M_DHCP_PORT_SERVER == usSrcPort ) 
        && ( M_DHCP_PORT_CLIENT == usDstPort ) )
        {
        //DHCP Server发出的报文
        DhcpHdr *pDhcp = (DhcpHdr*)( (UINT8*)pUdp + sizeof( UdpHdr ) );
        pComMsg->SetUdpPtr( pUdp );
        pComMsg->SetDhcpPtr( (void*)pDhcp );
        //Set Client Mac Addr.
        pComMsg->SetKeyMac( pDhcp->aucChaddr );
        return M_DHCP_FROM_SERVER;
        }

    if ( (M_DHCP_PORT_CLIENT == usSrcPort ) 
        && (M_DHCP_PORT_SERVER == usDstPort ) )
        {
        //DHCP Client发出的报文
        DhcpHdr *pDhcp = (DhcpHdr*)( (UINT8*)pUdp + sizeof( UdpHdr ) );
        pComMsg->SetUdpPtr( pUdp );
        pComMsg->SetDhcpPtr( (void*)pDhcp );
        //Set Client Mac Addr.
        pComMsg->SetKeyMac( pDhcp->aucChaddr );
        return M_DHCP_TO_SERVER;
        }

    return M_DHCP_TYPE_ERR;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::LearnedUplinkTraffic

DESCRIPTION:
    Learned Bridge模式转发上行数据

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::LearnedUplinkTraffic(CComMessage *pComMsg,UINT8 rcpeflag)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge LearnedUplinkTraffic()" );

    //增加Ingress方向的性能统计值
    FROMDIR from = EB_FROM_AI;
//    TODIR   to;

    CMac srcMac( GetSrcMac( pComMsg ) );
    FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( srcMac ) );
    if ( NULL == pSrcFT )
        {
        if ( true == CTaskDm::GetInstance()->BuildFixIpContext( srcMac, pComMsg->GetEID() ) )
            {
            //是FixIP
            return;
            }

        UINT16 usIdx = GetFreeFTEntryIdxFromList();
        //新建转发表表项
        pSrcFT = GetFTEntryByIdx( usIdx );
        if ( NULL == pSrcFT )
            {
            //用户满
            IncreaseDirTrafficMeasureByOne( from, TYPE_LICENSE_USEDUP );
            return;
            }

        FTEntry tmpFT;
        tmpFT.bIsOccupied   = true;
        tmpFT.ulEid         = pComMsg->GetEID();
        tmpFT.usGroupId     = CTaskDm::GetInstance()->GetVlanIDbyEid( tmpFT.ulEid );
        memcpy( tmpFT.aucMAC, GetSrcMac(pComMsg), M_MAC_ADDRLEN );
        tmpFT.bIsServing    = true;
        tmpFT.bIsTunnel     = false;
        tmpFT.ulPeerBtsID   = 0;
        tmpFT.ulPeerBtsAddr = 0;
        tmpFT.usPeerBtsPort = 0;
        if(rcpeflag)
            tmpFT.usTTL = 0xffff;
        else
            tmpFT.usTTL = m_usLearnedBridgeAgingTime;
        tmpFT.usElapsed     = 0;
        tmpFT.bIsAuthed     = true;             //false;
        if(rcpeflag)
            tmpFT.bIsRcpe = true;
	    else 
       	    tmpFT.bIsRcpe = false;

#ifdef BUFFER_EN
        if ( NULL == pSrcFT->pBufList )
            {
            pSrcFT->pBufList = new CBufferList;
            }
#endif

        INIT_FT( pSrcFT, tmpFT );

        BPtreeAdd( srcMac,usIdx );

	 //lijinan 20081203 数据计费系统增加
	 cCdrObj.cdrMacAdd(pSrcFT->ulEid);

	 
        }
    else
        {
	    UINT32 t_uid = pComMsg->GetEID();
#ifdef RCPE_SWITCH
			if( trunkIsMRcpe(pSrcFT->ulEid) )
			{
				if( pSrcFT->ulEid != pComMsg->GetEID() )
				{
					CComMessage *RspMsg = new ( CTBridge::GetInstance(), M_MAC_ADDRLEN ) CComMessage;
					if( RspMsg != NULL )
					{
						memcpy( (UINT8*)RspMsg->GetDataPtr(), pSrcFT->aucMAC, M_MAC_ADDRLEN );
						RspMsg->SetDstTid( M_TID_UTDM );
						RspMsg->SetMessageId( M_CPE_L3_TRUNK_MAC_MOVEAWAY_NOTIFY );
						RspMsg->SetEID( pSrcFT->ulEid );
						RspMsg->SetSrcTid( (CTBridge::GetInstance())->GetEntityId());
						RspMsg->SetDirection( DIR_TO_AI );
						if( ! CComEntity :: PostEntityMessage( RspMsg ) )
						{
						    RspMsg->Destroy();
							//return false;
						}
					}
                }
			}
#endif
	     if(pSrcFT->ulEid!=t_uid)
	     {
		if(CdrTestFlag)
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "\nLearnedUplinkTraffic eid:0x%x,eid bf:0x%x\n",t_uid,pSrcFT->ulEid );
		//lijinan 20081203 数据计费系统增加
		 cCdrObj.cdrMacAdd(t_uid);
	 	 cCdrObj.DelEid(pSrcFT->ulEid,0);
		     pSrcFT->bIsTunnel = false;
		     pSrcFT->bIsServing = true;
		     pSrcFT->usPeerBtsPort = 0;
		     pSrcFT->ulPeerBtsID = 0;
		 
	     }
        //用户已存在，刷新Elapsed time
        pSrcFT->usElapsed = 0;
        pSrcFT->ulEid     = pComMsg->GetEID();
#ifdef TRUNK_CODE_VLAN
	if( rcpeflag )
		pSrcFT->usVlanID = pComMsg->GetVlanID();
       
	else
		pSrcFT->usVlanID = M_NO_VLAN_TAG;
#endif       
        if(rcpeflag)
            pSrcFT->usTTL = 0xffff;
        else
            pSrcFT->usTTL = m_usLearnedBridgeAgingTime; //DHCP->staticIP时, TTL没有正确改过来
        pSrcFT->bIsAuthed = true;
        ////
        CalcUsrCount( pSrcFT, M_COUNT_USERTYPE_DEL );
        }
    ////
    CalcUsrCount( pSrcFT, M_COUNT_USERTYPE_ADD );

    return;
}



/*消息名称：用户实时计费请求
				 字段名称		长度（Byte）	类型	描述
					MESSAGE TYPE	2						M	0x07 
					TransId			4						M	
					UID				4						M
					MAC				6						M
					TimeStamp		7						M	BCD，YYYYMMDDHHMMSS
					Class			1							0-7,0xff- invalid
					BAND_FLAG		1		
					UL Max Bandwidth	2		
					UL Min Bandwidth	2		
					DL Max Bandwidth	2		
					DL Min Bandwidth	2		
*/
void CTBridge::func_user_RTCharge_req(UINT16 index)
{
                if(index>=M_MAX_UT_PER_BTS)
                	{
                	    return;
                	}
		CComMessage      *pComMsgtoCb3000     = new ( this, 33 )CComMessage;
		CMessage msgReq(pComMsgtoCb3000);
		CMessage msgReqTimeOut(NULL);
	       if ( NULL == pComMsgtoCb3000 )
	        {
		        return ;
	        }
		 char* pData = (char*)pComMsgtoCb3000->GetDataPtr();
		 memset(pData,0,33);
		 if(cCdrObj.get_cdr_profile(&pData[6],index)==false)
		 {
		 	pComMsgtoCb3000->Destroy();
		 	return;
		 }
		 cCdrObj.SetReqRemainFlag(index);
		 pData[1] = RT_CHARGE_REQ;
	        pComMsgtoCb3000->SetDstTid( M_TID_EMSAGENTTX ); 
	        pComMsgtoCb3000->SetSrcTid( M_TID_EB ); 
	        pComMsgtoCb3000->SetMessageId( MSG_BTS_TO_CB3000 );
		 CTransaction *pTrans = CreateTransact(msgReq, msgReqTimeOut, 3, 5000);
		if (NULL == pTrans)
		{
		    LOG(LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "[tCdr]ERROR!!! System encounter exceptions, create transaction fail.");
		    msgReq.DeleteMessage();
		    return ;
		}
		UINT16 trans_id = pTrans->GetId();
	        memcpy(&pData[4],(char*)&trans_id,2);/*本来是4字节，高两个字节已经清零*/
		if (false == pTrans->BeginTransact())
		{
		    LOG(LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "[tCdr]ERROR!!! System encounter exceptions, Begin transaction fail.");
		    pTrans->EndTransact();
		    delete pTrans;
		}
}


void CTBridge::func_RTCharge_rpt(UINT16 index,RT_RPT_MSG_TYPE flag)
{
               if(index>=M_MAX_UT_PER_BTS)
                	{
                	    return;
                	}
		CComMessage      *pComMsgtoCb3000     = new ( this, 64 )CComMessage;
		CMessage msgReq(pComMsgtoCb3000);
	       if ( NULL == pComMsgtoCb3000 )
	        {
		        return ;
	        }
		 CComMessage      *MsgtoCb3000Fail     = new ( this, sizeof(_CDR) )CComMessage;
		 CMessage msgReqTimeOut(MsgtoCb3000Fail);
		  if ( NULL == MsgtoCb3000Fail )
	        {
	        	 pComMsgtoCb3000->Destroy();
		        return ;
	        }
		 char* pData = (char*)pComMsgtoCb3000->GetDataPtr();
		 _CDR* pCdr = cCdrObj.setRtChargeRptBuf(&pData[6],index,flag);
		 pData[0] = 0;
		 pData[1] = RT_CHARGE_REPT_REQ;
	        pComMsgtoCb3000->SetDstTid( M_TID_EMSAGENTTX ); 
	        pComMsgtoCb3000->SetSrcTid( M_TID_EB ); 
	        pComMsgtoCb3000->SetMessageId( MSG_BTS_TO_CB3000 );

		 //填写发送失败消息
		 MsgtoCb3000Fail->SetMessageId(MSG_BTS_TO_CB3000_FAIL);
		 MsgtoCb3000Fail->SetDstTid( M_TID_EB ); 
	        MsgtoCb3000Fail->SetSrcTid( M_TID_EB ); 
		 char* p = (char*)MsgtoCb3000Fail->GetDataPtr();
		 if(pCdr!=NULL)
		 {
		 memcpy(p,(char*)pCdr,sizeof(_CDR));
		 }
		 
		 CTransaction *pTrans = CreateTransact(msgReq, msgReqTimeOut, 3, 5000);
		if (NULL == pTrans)
		{
		    LOG(LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "[tCdr]ERROR!!! System encounter exceptions, create transaction fail.");
		    msgReq.DeleteMessage();
		    return ;
		}
		UINT16 trans_id = pTrans->GetId();
		pData[2] = 0;
		pData[3] = 0;
	        memcpy(&pData[4],(char*)&trans_id,2);/*本来是4字节，高两个字节已经清零*/
		if (false == pTrans->BeginTransact())
		{
		    LOG(LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "[tCdr]ERROR!!! System encounter exceptions, Begin transaction fail.");
		    pTrans->EndTransact();
		    delete pTrans;
		}
}


void CTBridge::func_RTCharge_rpt(char* buf,int len,BTS_CB3000_MSG type,char* buf_fail,int fail_buf_len)
{
		CComMessage      *pComMsgtoCb3000     = new ( this, (len+6) )CComMessage;
		CMessage msgReq(pComMsgtoCb3000);
	       if ( NULL == pComMsgtoCb3000 )
	        {
		        return ;
	        }
		 CComMessage      *pComMsgtoCb3000fail     = new ( this, fail_buf_len )CComMessage;
		 if(NULL==pComMsgtoCb3000fail)
		 {
			pComMsgtoCb3000->Destroy();
			return;
		 }
		 CMessage msgReqTimeOut(pComMsgtoCb3000fail);
		 char* pData = (char*)pComMsgtoCb3000->GetDataPtr();
		 memcpy(&pData[6],buf,len);
		 pData[0] = 0;
		 pData[1] = type;
	        pComMsgtoCb3000->SetDstTid( M_TID_EMSAGENTTX ); 
	        pComMsgtoCb3000->SetSrcTid( M_TID_EB ); 
	        pComMsgtoCb3000->SetMessageId( MSG_BTS_TO_CB3000 );

		 pComMsgtoCb3000fail->SetDstTid(M_TID_EB);
		 pComMsgtoCb3000fail->SetSrcTid(M_TID_EB);
		 pComMsgtoCb3000fail->SetMessageId( MSG_BTS_TO_CB3000_FAIL);
		 char*p = (char*)pComMsgtoCb3000fail->GetDataPtr();
		 memcpy(p,buf_fail,fail_buf_len);
		 
		 CTransaction *pTrans = CreateTransact(msgReq, msgReqTimeOut, 3, 5000);
		if (NULL == pTrans)
		{
		    LOG(LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "[tCdr]ERROR!!! System encounter exceptions, create transaction fail.");
		    msgReq.DeleteMessage();
		    return ;
		}
		UINT16 trans_id = pTrans->GetId();
		pData[2] = 0;
		pData[3] = 0;
	        memcpy(&pData[4],(char*)&trans_id,2);/*本来是4字节，高两个字节已经清零*/
		if (false == pTrans->BeginTransact())
		{
		    LOG(LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "[tCdr]ERROR!!! System encounter exceptions, Begin transaction fail.");
		    pTrans->EndTransact();
		    delete pTrans;
		}
}

bool CTBridge::NoMoneyEidIpPro(CComMessage *pComMsg,UINT32 ulEid)
{
		UINT16 usProto = GetProtoType( pComMsg );
		//printf("\nNoMoneyEidIpPro eid :0x%x,have no money\n",ulEid);
		if(usProto!=M_ETHER_TYPE_IP)
			return false;

		    EtherHdr *pEtherPkt = (EtherHdr*)(pComMsg->GetDataPtr());
 		    IpHdr *pIp;
 		    if(IS_8023_PACKET(ntohs(pEtherPkt->usProto)))
 		    {
 		        pIp = (IpHdr*)( (UINT8*)pEtherPkt + sizeof(EtherHdr) + sizeof(LLCSNAP));
 		    }
 		    else
 		    {
 		        pIp = (IpHdr*)( pEtherPkt + 1 );
 		    }
		   //printf("\nNoMoneyEidIpPro IP:%x,proto:%x,%x\n",ntohl(pIp->ulDstIp),pIp->ucProto,ulEid);
		    if(ntohl(pIp->ulDstIp)!=IP_CB3000)
		    {
			if(pIp->ucProto!=M_PROTOCOL_TYPE_UDP)
				return false;

			    UINT16 usIpHdrLen = ( ( pIp->ucLenVer ) & 0x0f ) * 4;
			    UdpHdr *pUdp = (UdpHdr*) ( (UINT8*)pIp + usIpHdrLen );
			    UINT16 usSrcPort = ntohs( pUdp->usSrcPort );
			    UINT16 usDstPort = ntohs( pUdp->usDstPort );
			   // printf("\nNoMoneyEidIpPro port:%x\n",usDstPort);
			    if(M_DNS_PORT!=usDstPort)
				return false;
			    //组DNS响应将cb3000的地址回给终端
			    UINT16 usUdpLen = ntohs( pUdp->usLen) ;
			    DnsHdr *pDns = (DnsHdr*)((UINT8*)pUdp + sizeof(UdpHdr));
			   /* printf("\nNoMoneyEidIpPro usQusNum:%x,udplen:%x\n",ntohs(pDns->usQusNum),usUdpLen);
			    UINT8 *p = (UINT8 *)pUdp;
			    for(int j = 0;j<4;j++)
			    	printf("\n j:%d,dns head data:%x,%x,%x,%x,%x\n",j,p[0+j*5], p[1+j*5],p[2+j*5],p[3+j*5],p[4+j*5]);*/
			    if(ntohs(pDns->usQusNum)!=1)
					return false;
			    //通常只有一个域名DNS请求
			    //在原包长的基础上增加16个字节的回答
			    	 UINT16 msglen = 14+20+usUdpLen +16;
       		        CComMessage      *pComMsg     = new ( this, msglen )CComMessage;
       		        if ( NULL == pComMsg )
       		            {
       		            //delete pComMsgNode;
       		            return false;
       		            }
				DnsPktHead *pDnsRspH = (DnsPktHead *)pComMsg->GetDataPtr();
				memcpy(pDnsRspH->ethHead.aucDstMAC,pEtherPkt->aucSrcMAC,6);
				memcpy(pDnsRspH->ethHead.aucSrcMAC,pEtherPkt->aucDstMAC,6);
				pDnsRspH->ethHead.usProto = pEtherPkt->usProto;

				pDnsRspH->dnsHead.usID = pDns->usID;
				pDnsRspH->dnsHead.usDnsFlag = htons(0x8580);
				pDnsRspH->dnsHead.usQusNum = htons(0x0001);
				pDnsRspH->dnsHead.usAnswerNum= htons(0x0001);
				pDnsRspH->dnsHead.usAuth = 0;
				pDnsRspH->dnsHead.usAdd = 0;



				

				UINT8 *pDnsData = (UINT8 *)(pDnsRspH+1);
				UINT16 usDnsDataLen = usUdpLen-sizeof(UdpHdr)-sizeof(DnsHdr);
				memcpy(pDnsData,(UINT8*)(pDns+1),usDnsDataLen);

				DnsPktAns *pAns =  (DnsPktAns *)(pDnsData+usDnsDataLen);
				pAns->CmpName[0] = 0xc0;
				pAns->CmpName[1] = 0x0c;
				pAns->ClassType = htons(0x0001);
				pAns->Class = pAns->ClassType;
				pAns->TTL = htonl(5);//原1200 秒 为使被重定向的原网页充值后能尽快访问，减少为5秒 liuweidong 2011.4.20
				pAns->len = htons(0x04);
				pAns->ip = htonl(IP_CB3000);


				
				IpHdr* pIpHead = (IpHdr*)((UINT8*)pDnsRspH+14);			

				UdpHdr*pUdpHead = (UdpHdr*)(pIpHead+1);
				pUdpHead->usDstPort = pUdp->usSrcPort;
				pUdpHead->usSrcPort = htons(M_DNS_PORT);
				pUdpHead->usLen = htons(usUdpLen +16);
				pUdpHead->usCheckSum = 0;

				/* set udp pseudo head */
				pseudoHeadTEb* phead = &((pseudoPktTEb*)pIpHead)->phead;
				phead->srcIP = pIp->ulDstIp;
				phead->dstIP= pIp->ulSrcIp;
				phead->proto = 0x11;
				phead->len = usUdpLen +16;

				/* count checksum (plus pseudo head) */
				#ifdef WBBU_CODE
			       UINT16 sum = ui_checksum_BBU((UINT16*)phead, usUdpLen +16+ sizeof(pseudoHeadTEb));
				#else
				UINT16 sum = ui_checksum((UINT16*)phead, usUdpLen +16+ sizeof(pseudoHeadTEb));
				#endif
				if(sum == 0)
					sum = 0xffff;
				pUdpHead->usCheckSum = sum;

				
				//IpHdr* pIpHead = (IpHdr*)((UINT8*)pDnsRspH+14);
				pIpHead->ucLenVer = 0x45;
				pIpHead->ucTOS = 0;
				pIpHead->usTotalLen = htons(msglen - 14);
				pIpHead->usId = 0xc2d2;
				pIpHead->usFragAndFlags = 0;
				pIpHead->ucTTL = 64;
				pIpHead->ucProto = M_PROTOCOL_TYPE_UDP;
				pIpHead->usCheckSum = 0;
				pIpHead->ulDstIp = pIp->ulSrcIp;
				pIpHead->ulSrcIp = pIp->ulDstIp;
				/* count checksum */
				#ifdef WBBU_CODE
				sum = ui_checksum_BBU((UINT16*)pIpHead, sizeof(IpHdr));
				#else
				sum = ui_checksum((UINT16*)pIpHead, sizeof(IpHdr));
				#endif
				if(sum==0)
					sum = 0xffff;
				pIpHead->usCheckSum = sum;
				

				     if ( true == SendToAI( pComMsg, ulEid,GetRelayMsgID( pComMsg ) ,false) )
			          {
			          	//printf("\n send dns ack to:%x\n",ulEid);
			           
				   }
#ifdef WBBU_CODE
				     else
				     	{
				     	   L3_L2_Send_err++;
				    // 	   pComMsg->Destroy();
				     	}
#endif
				   return false;
				
			    
			    
		    }

		   return true;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardUplinkPacket

DESCRIPTION:
    转发上行数据

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardUplinkPacket(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge ForwardUplinkPacket()" );
    
    //增加From Ingress方向的性能统计值
    FROMDIR from = EB_FROM_AI;
    TODIR   to;
    CComMessage *pComMsg_temp =NULL;
    #ifdef M_TGT_WANIF
    bool result;
    unsigned int  relayeid;
    char flag = 0;
    #endif
    bool res;
    CMac SrcMac( GetSrcMac( pComMsg ) );
    FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
	
    if ( NULL == pSrcFT )
    {
        //Illegal User Packet++;
        IncreaseDirTrafficMeasureByOne( from, TYPE_ILLEGAL_USER );
        //是否是FixIp?是则需要重建FixIp用户信息
        res = CTaskDm::GetInstance()->BuildFixIpContext( SrcMac, pComMsg->GetEID() );
        if(res == false)
        {
        
        }
        
        #ifdef M_TGT_WANIF         
        relayeid = pComMsg->GetEID();
        if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
        {
             for(int i =0;i<NvRamDataAddr->Relay_num;i++)
             {
                 if(i>=20)
                 	{
                 	   break;
                 	}
                  if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                  {
                      flag =1;
                      break;
                  }        
             }
        }
        if (flag==1)
        {
            LearnedUplinkTraffic( pComMsg,1 );
            pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
        }
        else
        {
            flag = 0;
            int j =0;
            for( j = 0; j< 5; j++)
            {
                if(g_last_register_eid[j].DebugEID == pComMsg->GetEID())
                 {
                      UINT32 tt = (UINT32)time( NULL );
                      if((tt-g_last_register_eid[j].TTL )>8)
                        {
                          flag = 1;
                          
                        }
                      break;
                 }
            }
            if((flag ==1)||(j==5))
             {
                notifyOneCpeToRegister(pComMsg->GetEID(),true);
                if(j==5)
                   {
                      g_last_register_eid[Register_eid_index%5].DebugEID =pComMsg->GetEID();
                      g_last_register_eid[Register_eid_index%5].TTL = (UINT32)time( NULL );
                      (Register_eid_index++);
                   }
             }
            return;
        }		
        #endif	 
        //return;
    }
    if(pSrcFT->ulEid!=pComMsg->GetEID())
    { //in case that pc change relay cpe
        #ifdef M_TGT_WANIF         
        relayeid = pComMsg->GetEID();
        if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
        {
            for(int i =0;i<NvRamDataAddr->Relay_num;i++)
            {
                if(i>=20)
                	{
                	   break;
                	}
                if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                {
                    flag =1;
                    break;
                }        
            }
        }
        if (flag==1)
        {
            LearnedUplinkTraffic( pComMsg,1 );
            pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
        }
        else
        {
            //notifyOneCpeToRegister(pComMsg->GetEID(),true);
            return;
        }		
        #endif	
    }
    
    if ( false == pSrcFT->bIsAuthed )
    {
        //还未认证成功
        IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_NOT_AUTHED );
        return;
    }
	#if 0
    if((pSrcFT->bIsServing == false)&&(pSrcFT->bIsTunnel == true))
     {
           pSrcFT->bIsServing = true;
           pSrcFT->bIsTunnel =false;
           pSrcFT->usTTL = M_TTL_FFFF;
           DATA_log(pComMsg, LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->pSrcFT->bIsServing err\n" );
           return;
     }
	#endif
    pSrcFT->usElapsed = 0;
    
    //lijinan 20081202 进行上行数据统计
    if(uiCdrSwitch == 1)
    {
        UINT16 cdrIndex = cCdrObj.BPtreeFind( pSrcFT->ulEid);
        //如果终端已经欠费，只对DNS req消息响应，将CB3000的IP给终端
        //如果是发向CB3000的包一律放行
        if(cdrIndex==M_DATA_INDEX_ERR)
        {
            if(eidIsWifiEid(pSrcFT->ulEid)==true)
            {
	     		cdrIndex = cCdrObj.BPtreeFind(SrcMac);
			if(cdrIndex==M_DATA_INDEX_ERR)
			{
					cdrIndex = cCdrObj.GetFreeCDRIdxFromList();
					if(cdrIndex<M_MAX_UT_PER_BTS)
	 					cCdrObj.BPtreeAdd(SrcMac, cdrIndex,pSrcFT->ulEid);	
					else
					{
						if(CdrTestFlag)
							DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "up wifi eid:%x,get cdr list err",pSrcFT->ulEid);
						return;
					}

						
			}
            }
	     else
	     {
	        if(CdrTestFlag)
	     	 	DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " up eid:%x,get cdr list err",pSrcFT->ulEid);
		 cCdrObj.cdrMacAdd(pSrcFT->ulEid);
		 cdrIndex = cCdrObj.BPtreeFind( pSrcFT->ulEid);
		 if(cdrIndex==M_DATA_INDEX_ERR)
		 {
			
			DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "up 3  eid:%x,get cdr list err",pSrcFT->ulEid);
			return;
		 }
            	 //return;
	     }
    	}
        if(cCdrObj.GetWifiEidFlag(cdrIndex)==FLAG_YES)
        {
		if(cCdrObj.getWifiUid(cdrIndex)==0)
		{
			//还没有认证，请求wifiUid
			/*字段名称	长度（Byte）	类型	描述
				MESSAGE TYPE	2				M	0x01，接入请求
				Session ID		4				M	
				Hot spot CPE UID	4				M	CPE真实的UID
				User IP			4				M	PC用户的IP地址
				User MAC		6				M	PC的MAC地址
			*/
			char buf[32];
			memset(buf,0,32);
			buf[0] = 0;
			buf[1] = WIFI_USER_ACC_REQ;
			memcpy(&buf[6],(char*)&pSrcFT->ulEid,4);
			UINT32 wifi_ip =  getDhcpIpByMac(GetSrcMac( pComMsg ));
			cCdrObj.SetWifiIp(cdrIndex,wifi_ip);
			memcpy(&buf[10],(char*)&wifi_ip,4);
			memcpy(&buf[14],(char*)GetSrcMac( pComMsg ),6);
			UINT16 msglen = 20;
			sendMsg2CB3000(buf,msglen);
			return;
		}

		//认证没有过，重定向默认充值网站
		/*if(cCdrObj.getWifiUid(cdrIndex)==0xffffffff)
		{
	            if(NoMoneyEidIpPro(pComMsg,pSrcFT->ulEid)==false)
	            	return;
		}*/
	 }
	if(cCdrObj.GetNoPayFlag(cdrIndex)==1)//wangwenhua add 20120116
	{
	 	UINT32 uid1=0;
		 UINT8 utType1=0;
		char bw1[8];
		UINT32  eid1 = pSrcFT->ulEid;
		UINT8 adminstatus1  = 0;
		UINT8 wififlag= 0;
		 if(eidIsWifiEid(eid1)==true)
		 {
		   	wififlag = 1;
			if(CdrTestFlag)
			DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " wifi eid:%x ",eid1);
		 }
	      uid1 = findUidFromEid(eid1,&adminstatus1,&utType1,bw1);
		if(adminstatus1==0)
		{
		     cCdrObj.SetNoPayFlag(cdrIndex,  adminstatus1);
		      if(CdrTestFlag)
		    	DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " eid:%x UID:%x no Pay Flag Err1,",eid1,uid1);
		     
		
		}
	}
        if(cCdrObj.GetNoPayFlag(cdrIndex)==1||(cCdrObj.getWifiUid(cdrIndex)==0xffffffff))
        {
            if(NoMoneyEidIpPro(pComMsg,pSrcFT->ulEid)==false)
            return;
        }
        else
        {
            if(real_time_switch)
            {
			if(cCdrObj.GetReqRemainFlag(cdrIndex)==0||cCdrObj.GetOverFlag(cdrIndex)==1)
			{
				func_user_RTCharge_req(cdrIndex);	
				if(CdrTestFlag)
				{
					DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " eid:%x,send RT charge req",pSrcFT->ulEid);
				}
			
			}
			if(cCdrObj.GetOverFlag(cdrIndex)==1)
			{
				if(CdrTestFlag)
				{
					DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " eid:%x,send RT charge req,have no charge",pSrcFT->ulEid);
				}
				return;
			}
	     }
            semTake (cCdrObj.GetCdrSemId(), WAIT_FOREVER);
            //UINT16 cdrIndex = cCdrObj.BPtreeFind( pSrcFT->ulEid);
            cCdrObj.StatDataFlow(CDR_UP, cdrIndex, pComMsg->GetDataLength());
            cCdrObj.StatTimeLen(FromPKT,cdrIndex);            
            semGive (cCdrObj.GetCdrSemId());
        }
    }
    
    CMac DstMac( GetDstMac( pComMsg ) );
    if(true==DstMac.IsZero())//wangwenhua add 20080926
    {
        return;
    }
    if( DstMac==SrcMac )//liuweidong for spain tree loop back
    {
        DATA_log(NULL, LOG_DEBUG1, LOGNO(EB, EC_EB_PARAMETER), "dstMac = srcMac! discard !MAC[%.2X-%.2X-%.2X-%.2X-%.2X-%.2X]  EID[%.8x].", pSrcFT->aucMAC[0], pSrcFT->aucMAC[1], pSrcFT->aucMAC[2], pSrcFT->aucMAC[3], pSrcFT->aucMAC[4], pSrcFT->aucMAC[5], pSrcFT->ulEid);	
    	return;
    }
    UINT32 srcIP;
    char type = getMacType(pComMsg,&srcIP);
    if(type == 0)// to bts ip stack;
    {
        DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "try to visit local bts [CPEs[0x%X,0x%X] ]", pSrcFT->ulEid, 0);
        recordCPEToBTSIPStack(GetSrcMac( pComMsg ),pComMsg->GetEID(),srcIP); 
        SendToBTSIPStack(pComMsg);
        return;
    }	
    
    if ( true == DstMac.IsBroadCast() )
    {
        //broad-cast or multicast.
        if (TRUE == bspGetFilterSTP())//是否允许过滤vlan bridge和snap hdlc报文?
        {
            if (true == m_filterMAC.find(DstMac))
            {
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                DATA_log(pComMsg, LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "Uplink packet is filtered.[destination MAC address is in the filter MAC table]" );
                return;
            }
        }
        if ( true == pSrcFT->bIsTunnel )
        {
            //Serving BTS的上行广播包，隧道转发给Anchor
            if (true == SendToTunnel(pSrcFT, pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
            {
                //Source Mac的AI to TDR方向的统计值 ++
                to = EB_TO_TDR;
                IncreaseMacTrafficMeasureByOne( pSrcFT, from, to );
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_BROADCAST_TDR );
            } 
            
            else
            {
                //丢包统计
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }
        }
        else
        {                  	               
            //wangwenhua add 20100618 for rcpe
            if(g_BC_test_flag&0x1)//rcpe修改，发给rcpe，wangwenhua20100721
            {
                if (1/*true == DstMac.IsBroadCast()*/)
                {
                    if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
                    {
                        for(int i =0;i<NvRamDataAddr->Relay_num;i++)
                        {
                           if(i>=20)
                           	{
                           	    break;
                           	}
                            if(RelayWanifCpeEid[i]!=0)
                            {
                                if(RelayWanifCpeEid[i] !=pSrcFT->ulEid)
                                {
                                    pComMsg_temp = new (this, pComMsg->GetDataLength())  CComMessage;
                                    if(pComMsg_temp!=NULL)
                                    {
                                        memcpy((unsigned char*)(pComMsg_temp->GetDataPtr()),(unsigned char*)(pComMsg->GetDataPtr()),pComMsg->GetDataLength());
                                        UINT16 usVlanId = pComMsg->GetVlanID();
                                        pComMsg_temp->SetVlanID(usVlanId);
                                        if( true == SendToAI( pComMsg_temp, 
                                          RelayWanifCpeEid[i], 
                                          GetRelayMsgID( pComMsg_temp ) ,true)
                                        )
                                        {
                                        }
                                    }
                                }
                                else
                                {
                                      g_send_2_self++;
                                }
                            }
                            
                            
                        }
                    }
                }
            }
            //Anchor BTS的上行广播包，转发到WAN
            if(Wanif_Switch==0x5a5a)//wcpe模式下广播包都转jy20100318
                SendToWAN( pComMsg, pSrcFT->usGroupId , 2);
            if( true == SendToWAN( pComMsg, pSrcFT->usGroupId  , 0) )   //mod by xiaoweifang to support vlan.
            {
                //Source Mac的AI to WAN方向的统计值 ++
                to = EB_TO_WAN;
                IncreaseMacTrafficMeasureByOne( pSrcFT, from, to );
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_BROADCAST_WAN );
            }
            else
            {
                //丢包统计
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }
            
        }
    }
    else
    {
        //uni-cast
        FTEntry *pDstFT = GetFTEntryByIdx( BPtreeFind( DstMac ) );
        if ( NULL == pDstFT )
        {
            //
            if ( true == pSrcFT->bIsTunnel )
            {
                //source mac是在Serving BTS
                if (true == SendToTunnel(pSrcFT, pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
                {
                    //Source Mac的AI to TDR方向的统计值 ++
                    to = EB_TO_TDR;
                    IncreaseMacTrafficMeasureByOne( pSrcFT, from, to );
                    IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_TDR );
                }
                else
                {
                    //丢包统计
                    IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                }
            }
            else
            {
                UINT8 flag = compwithLocalMac(GetDstMac( pComMsg )); 
                //source mac在Anchor BTS
                if( true == SendToWAN( pComMsg, pSrcFT->usGroupId, flag))  //mod by xiaoweifang to support vlan
                {
                    //Source Mac的AI to WAN方向的统计值 ++
                    to = EB_TO_WAN;
                    IncreaseMacTrafficMeasureByOne( pSrcFT, from, to );
                    IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_WAN );
                }
                else
                {
                    //丢包统计
                    IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                }
            }
        }
        else
        {
            if( pSrcFT->ulEid == pDstFT->ulEid )	
   	        { //both source and dest in one cpe,discard
   	      	return;
   	        } 	
            if ( true == pDstFT->bIsServing )
            {
                //dest mac在BTS服务区
                //转发给UT
                UINT16 srcVLAN = m_group[pSrcFT->usGroupId].usVlanID;
                UINT16 dstVLAN = m_group[pDstFT->usGroupId].usVlanID;
                if ((srcVLAN != dstVLAN) && 
                ((srcVLAN > M_NO_VLAN_TAG) || (dstVLAN > M_NO_VLAN_TAG)))
                {
                    DATA_log(pComMsg, LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "Packet is discarded, Reson:[CPEs[0x%X,0x%X] is not in the save vlan group]", pSrcFT->ulEid, pDstFT->ulEid);
                    IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                    return;
                }
			//lijinan  20121020 for video
	        UINT16 toAirMsgId = pComMsg->GetMessageId();
	        if(pDstFT->video_ip[0]!=0)
		{
		       UINT32 src_ip = 0;
			if(M_ETHER_TYPE_IP == GetProtoType(pComMsg))
			{
				src_ip = GetSrcIpAddr(pComMsg);
				if((src_ip==pDstFT->video_ip[0])||(src_ip==pDstFT->video_ip[1]))
				{
					toAirMsgId = MSGID_VIDEO_DATA_TRAFFIC;
				}	
			}
			if(video_debug)
			{
				logMsg("\neid to eid:0x%x,rec VSS park,src_ip:0x%x,MsgId:0x%x\n",pDstFT->ulEid,src_ip,toAirMsgId,0,0,0);
			}

		}  
			
                //MsgID使用原来的
                if ( true == SendToAI( pComMsg, 
                 pDstFT->ulEid, 
                 toAirMsgId,pDstFT->bIsRcpe ) )
                {
                    //Source Mac的AI to AI方向的统计值 ++
                    to = EB_TO_AI;
                    IncreaseMacTrafficMeasureByOne( pSrcFT, from, to );
                    IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_AI );
                    pDstFT->usElapsed = 0;
                    
                    //lijinan 20081202 进行下行数据统计
                    if(uiCdrSwitch == 1)
                    {
                        //semTake (cCdrObj.GetCdrSemId(), WAIT_FOREVER);
                        UINT16 cdrIndex = cCdrObj.BPtreeFind( pDstFT->ulEid);
				   if(cdrIndex==M_DATA_INDEX_ERR)
				   {
				            if(eidIsWifiEid(pDstFT->ulEid)==true)
				            {
					     		cdrIndex = cCdrObj.BPtreeFind(DstMac);
							if(cdrIndex>=M_MAX_UT_PER_BTS)
							{
							 	if(CdrTestFlag)
								DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " wifi eid:%x,get cdr list err",pDstFT->ulEid);
								 return;	
							}
				            }
					     else
					     {
					    	       if(CdrTestFlag)
					     		DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " eid:%x,get cdr list err",pDstFT->ulEid);
							cCdrObj.cdrMacAdd(pDstFT->ulEid);
							cdrIndex = cCdrObj.BPtreeFind( pDstFT->ulEid);
							if(cdrIndex==M_DATA_INDEX_ERR)
							{

									DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " eid:%x down 1,get cdr list err",pDstFT->ulEid);
									return;
							}
						 	//return;
					     }
				   }
			   semTake (cCdrObj.GetCdrSemId(), WAIT_FOREVER);
                        cCdrObj.StatDataFlow(CDR_DOWN, cdrIndex, pComMsg->GetDataLength());
                        cCdrObj.StatTimeLen(FromPKT,cdrIndex);			   
                        semGive (cCdrObj.GetCdrSemId());
                    }
                    }
                else
                	{
#ifdef WBBU_CODE
                	    	   L3_L2_Send_err++;
				  //   	   pComMsg->Destroy();
#endif
                	}
            }
            else
            {
                //dest mac不在BTS服务区
                if (true == SendToTunnel(pDstFT, pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
                {
                    //Source Mac的AI to TDR方向的统计值 ++
                    to = EB_TO_TDR;
                    IncreaseMacTrafficMeasureByOne( pSrcFT, from, to );
                    IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_TDR );
                    pDstFT->usElapsed = 0;
                }
                else
                {
                    //丢包统计
                    IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                }
            }
        }
    }    
    return;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::SendToTunnel

DESCRIPTION:
    EB只关心BtsID, 往隧道对端的bts发送数据时首先查询btsID对应的
    公网IP, port.隧道既然建立起来,tSocket应该已经维护好了这个对应
    关系,所以查询应该会很快而且成功

ARGUMENTS:
    ulDstBtsAddr:隧道对端BTS的地址
    ulLen:数据长度
    *pData:发送数据

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::SendToTunnel(FTEntry *pFT,UINT32 ulLen, void *pData)
{
    if(false == CheckPeerBtsAddr(pFT))
        {
        return false;
        }
    return SendToTunnel(pFT->ulPeerBtsAddr, pFT->usPeerBtsPort, ulLen, pData);
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::SendToTunnel

DESCRIPTION:
    通过EtherIp隧道转发数据,所有Traffic数据DataPtr()前
    都要预留至少22字节

ARGUMENTS:
    ulDstBtsAddr:隧道对端BTS的地址
    ulLen:数据长度
    *pData:发送数据

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
#ifdef WBBU_CODE
extern  UINT32 dwIP;
unsigned int long_packet= 0;
unsigned int discard_packet = 0;
extern  unsigned char  Etsec_Blocked;

#endif
bool CTBridge::SendToTunnel(UINT32 ulDstBtsAddr, UINT16 usDstBtsPort,UINT32 ulLen, void *pData)
{
    LOG2( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge SendToTunnel(IP:0x%X, PORT:%d)", ulDstBtsAddr, usDstBtsPort );
    ////
#ifndef WBBU_CODE
    if ( (0 == ulDstBtsAddr) || (0 == usDstBtsPort) )
        {
        DATA_assert(0);
        return false;
        }
#else
    if ( (0 == ulDstBtsAddr) || (0 == usDstBtsPort)||(ulDstBtsAddr==dwIP) )
        {
        DATA_assert(0);
        return false;
        }
#endif
    //统计发给隧道的数据包总长度jiaying20110801    
    statToTDR += ulLen;    
    statPktToTDR++;
    //统计发给隧道的数据包总长度jiaying20110801 end
    //new stat
    m_dataToTDR += ulLen;
    sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_addr.s_addr = htonl(ulDstBtsAddr);
    target.sin_port = htons(usDstBtsPort);
    
    UINT16 usBufLen = (UINT16)( ulLen + sizeof( SocketMsgArea )  );
    UINT8 *pBuf = (UINT8*)pData - sizeof( SocketMsgArea ); //使用预留空间
    SocketMsgArea *pMsgArea = (SocketMsgArea*)pBuf;
    pMsgArea->MsgArea = MSGAREA_TUNNEL_DATA;
 #ifdef WBBU_CODE 
    if(usBufLen>=1500)
    {
    	    long_packet++;
		//	printf("%x,%x,%x,%x,%x,%x,%x,%x\n",pData[15])
    }
	if(Etsec_Blocked==0)
	{
    int ulVal =::sendto(m_EtherIpSocket, (char*)pMsgArea, usBufLen, 0, (sockaddr*)&target, sizeof(target));
    if ( M_DATA_SOCKET_ERR == ulVal )
        {
        LOG3( LOG_DEBUG1, LOGNO( EB, EC_EB_SOCKET_ERR ), 
             "Send EtherIp Packet(length:%d) to BTS(IP:0x%X, port:%d) Fails!", ulLen, ulDstBtsAddr, usDstBtsPort );
        return false;
        }
	}
	else
	{
		  discard_packet++;
	}
 #else
    int ulVal =::sendto(m_EtherIpSocket, (char*)pMsgArea, usBufLen, 0, (sockaddr*)&target, sizeof(target));
    if ( M_DATA_SOCKET_ERR == ulVal )
        {
        LOG3( LOG_DEBUG1, LOGNO( EB, EC_EB_SOCKET_ERR ), 
             "Send EtherIp Packet(length:%d) to BTS(IP:0x%X, port:%d) Fails!", ulLen, ulDstBtsAddr, usDstBtsPort );
        return false;
        }
        
  
#endif

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::SendToAI

DESCRIPTION:
    通过AI转发数据

ARGUMENTS:
    *pComMsg:
    ulEid:
    usMsgId:

RETURN VALUE:
    bool: 发送成功/失败

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::SendToAI(CComMessage *pComMsg, UINT32 ulEid, UINT16 usMsgId,bool bIsRcpe)
{
#ifdef TRUNK_CODE_VLAN
   // UINT32 eid = pComMsg->GetEID();
     UINT16 usVlanId2 =0xffff;
    if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
    {

	if(  bIsRcpe == true ) 
	{	
		UINT16 usVlanId = pComMsg->GetVlanID();
		
		//DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge SendToWAN(group:%d, vlanId:%d)", grpID, usVlanId);
		if (   M_NO_VLAN_TAG != usVlanId ) 
		{

			CMac DstMac( GetDstMac( pComMsg ) );

                      FTEntry *pDstFT = GetFTEntryByIdx( BPtreeFind( DstMac ) );
		        if( pDstFT !=NULL)
 		                   usVlanId2 = m_group[pDstFT->usGroupId].usVlanID;
                     // 当消息内带的vlan和终端配置相等时，不带vlan，否则带，如广播包，带vlan tag
		       if ( usVlanId ==usVlanId2 ) 
		       {
		       }
			else   
		      	{
				UINT8    *pData1 = (UINT8*)( pComMsg->GetDataPtr() );
				VLAN_hdr *pVlan1 = (VLAN_hdr*)( pData1 + 12 );
				if ( M_ETHER_TYPE_VLAN == ntohs( pVlan1->usProto_vlan ) )//avoid rewriting vlan id two times wangwenhua 20100412
				{
				}
				else
				{
					UINT8 *pDataPtr = (UINT8*)( pComMsg->GetDataPtr() );
					UINT8 *p = pDataPtr - 4;
					memcpy( p, pDataPtr, 12 );
					VLAN_hdr *pVlan = (VLAN_hdr*)( p + 12 );
					pVlan->usProto_vlan = htons( M_ETHER_TYPE_VLAN );
					pVlan->usVlanID = htons( usVlanId );
					pComMsg->SetDataPtr( p );
					pComMsg->SetDataLength( pComMsg->GetDataLength() + 4 );
				}
		       }       
		 }
	} 


    }   

#endif
    //统计发给空口的数据包总长度jiaying20110801    
    statToAir+= pComMsg->GetDataLength();  
    statPktToAir++;
    
    //统计发给空口的数据包总长度jiaying20110801 end
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge SendToAI(EID:0x%X)", ulEid );
    //pComMsg有效性检查;
#ifndef WBBU_CODE
    pComMsg->SetDstTid( M_TID_L2_TXINL3 );
#else
    pComMsg->SetDstTid( M_TID_L2MAIN );
#endif
    pComMsg->SetSrcTid( M_TID_EB );
    pComMsg->SetEID( ulEid );
    pComMsg->SetMessageId( usMsgId ); 
#ifdef WBBU_CODE
    pComMsg->SetMoudlue(1);/***表示给1核进行处理***/
#endif
    return CComEntity::PostEntityMessage( pComMsg );
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::SendToWAN

DESCRIPTION:
    通过WAN转发数据

ARGUMENTS:
    *pComMsg:

RETURN VALUE:
    bool: 发送成功/失败

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::SendToWAN(CComMessage *pComMsg, UINT16 grpID, UINT8 flag)
{
#ifdef TRUNK_CODE_VLAN
    CMac SrcMac( GetSrcMac( pComMsg ) );
    FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
    if( NULL != pSrcFT )
    {
        if( pSrcFT->bIsRcpe==true ) 
        {	
            UINT16 usVlanId = pComMsg->GetVlanID();
            DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge SendToWAN1(group:%d, vlanId:%d)", grpID, usVlanId);
            if ( M_NO_VLAN_TAG != usVlanId )
            {
                 UINT8    *pData1 = (UINT8*)( pComMsg->GetDataPtr() );
                 VLAN_hdr *pVlan1 = (VLAN_hdr*)( pData1 + 12 );
                 if ( M_ETHER_TYPE_VLAN == ntohs( pVlan1->usProto_vlan ) )//avoid rewriting vlan id two times wangwenhua 20100412
                 {
                 }
                 else
                 {
                     UINT8 *pDataPtr = (UINT8*)( pComMsg->GetDataPtr() );
                     UINT8 *p = pDataPtr - 4;
                     memcpy( p, pDataPtr, 12 );
                     VLAN_hdr *pVlan = (VLAN_hdr*)( p + 12 );
                     pVlan->usProto_vlan = htons( M_ETHER_TYPE_VLAN );
                     pVlan->usVlanID = htons( usVlanId );
                     pComMsg->SetDataPtr( p );
                     pComMsg->SetDataLength( pComMsg->GetDataLength() + 4 );
                 }
            }
	   else
	    {
	            //add by xiaoweifang to support vlan.{{
	            UINT16 usVlanId2 = m_group[grpID].usVlanID;
	         //   CMac DstMac( GetDstMac( pComMsg ) );
	            DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge SendToWAN(group:%d, vlanId:%d)", grpID, usVlanId2);
	            if ( M_NO_VLAN_TAG < usVlanId2 )
	            {
	        
	                UINT8    *pData2 = (UINT8*)( pComMsg->GetDataPtr() );
	                VLAN_hdr *pVlan2 = (VLAN_hdr*)( pData2 + 12 );
	                if ( M_ETHER_TYPE_VLAN == ntohs( pVlan2->usProto_vlan ) )//avoid rewriting vlan id two times wangwenhua 20100412
	                {
		       	   
	                }
	                else
	                {
	                    //添加VLAN tag.
	                    UINT8 *pDataPtr2 = (UINT8*)( pComMsg->GetDataPtr() );
	                    UINT8 *p2 = pDataPtr2 - 4;
	                    memcpy( p2, pDataPtr2, 12 );
	                    VLAN_hdr *pVlan2 = (VLAN_hdr*)( p2 + 12 );
	                    pVlan2->usProto_vlan = htons( M_ETHER_TYPE_VLAN );
	                    pVlan2->usVlanID = htons( usVlanId2 );
	                    //
	                    pComMsg->SetDataPtr( p2 );
	                    pComMsg->SetDataLength( pComMsg->GetDataLength() + 4 );
	               }
	               //消息其他字段不用修改
	            }

	   	}
				
       }
       else
       {
            //add by xiaoweifang to support vlan.{{
            UINT16 usVlanId = m_group[grpID].usVlanID;
            CMac DstMac( GetDstMac( pComMsg ) );
            DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge SendToWAN(group:%d, vlanId:%d)", grpID, usVlanId);
            if ( M_NO_VLAN_TAG < usVlanId )
            {
        
                UINT8    *pData1 = (UINT8*)( pComMsg->GetDataPtr() );
                VLAN_hdr *pVlan1 = (VLAN_hdr*)( pData1 + 12 );
                if ( M_ETHER_TYPE_VLAN == ntohs( pVlan1->usProto_vlan ) )//avoid rewriting vlan id two times wangwenhua 20100412
                {
	       	   
                }
                else
                {
                    //添加VLAN tag.
                    UINT8 *pDataPtr = (UINT8*)( pComMsg->GetDataPtr() );
                    UINT8 *p = pDataPtr - 4;
                    memcpy( p, pDataPtr, 12 );
                    VLAN_hdr *pVlan = (VLAN_hdr*)( p + 12 );
                    pVlan->usProto_vlan = htons( M_ETHER_TYPE_VLAN );
                    pVlan->usVlanID = htons( usVlanId );
                    //
                    pComMsg->SetDataPtr( p );
                    pComMsg->SetDataLength( pComMsg->GetDataLength() + 4 );
               }
               //消息其他字段不用修改
            }
	    //}}add by xiaoweifang           	
        }
    }	
#else
    //add by xiaoweifang to support vlan.{{
    UINT16 usVlanId = m_group[grpID].usVlanID;
    CMac DstMac( GetDstMac( pComMsg ) );
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge SendToWAN(group:%d, vlanId:%d)", grpID, usVlanId);
    if ( M_NO_VLAN_TAG < usVlanId )
        {
          
            UINT8    *pData1 = (UINT8*)( pComMsg->GetDataPtr() );
    	    VLAN_hdr *pVlan1 = (VLAN_hdr*)( pData1 + 12 );
       if ( M_ETHER_TYPE_VLAN == ntohs( pVlan1->usProto_vlan ) )//avoid rewriting vlan id two times wangwenhua 20100412
       	{
       	   
       	}
	   else
	   	{
        //添加VLAN tag.
        UINT8 *pDataPtr = (UINT8*)( pComMsg->GetDataPtr() );
        UINT8 *p = pDataPtr - 4;
        memcpy( p, pDataPtr, 12 );
        VLAN_hdr *pVlan = (VLAN_hdr*)( p + 12 );
        pVlan->usProto_vlan = htons( M_ETHER_TYPE_VLAN );
        pVlan->usVlanID = htons( usVlanId );
        //
        pComMsg->SetDataPtr( p );
        pComMsg->SetDataLength( pComMsg->GetDataLength() + 4 );
	   	}
        //消息其他字段不用修改
        }
    //}}add by xiaoweifang
#endif
    //统计发给网络侧的数据包总长度jiaying20110801
    statToWan += pComMsg->GetDataLength();  
    statPktToWan++;
    //统计发给网络侧的数据包总长度jiaying20110801 end
    //newtat
    m_dataToWan += pComMsg->GetDataLength(); 
#ifndef WBBU_CODE
    //增加计数
    #ifndef  M_TGT_WANIF 
     pComMsg->AddRef();
    return mv643xxRecvMsgFromEB
        (
        (char*)pComMsg->GetDataPtr(),       //Data to send
        (UINT16)pComMsg->GetDataLength(),   //Data length
        CTBridge::EBFreeMsgCallBack,        //function.
        (UINT32)pComMsg                     //ComMessage ptr.
        );
   #else
       if(Wanif_Switch==0x5a5a)//只有在打开开关的情况下才进行处理
       {
           if(flag == 2)//send to network jy20100318
           {
               pComMsg->AddRef();
	       return mv643xxRecvMsgFromEB
	        (
	        (char*)pComMsg->GetDataPtr(),       //Data to send
	        (UINT16)pComMsg->GetDataLength(),   //Data length
	        CTBridge::EBFreeMsgCallBack,        //function.
	        (UINT32)pComMsg                     //ComMessage ptr.
	        );
           }
	    else if ( true == SendToAI( pComMsg, 
	                            WorkingWcpeEid, 
	                            GetRelayMsgID( pComMsg ) ,false))
	      	{
	 
	        }
       }
	else
	 {
		   	    pComMsg->AddRef();
	    return mv643xxRecvMsgFromEB
	        (
	        (char*)pComMsg->GetDataPtr(),       //Data to send
	        (UINT16)pComMsg->GetDataLength(),   //Data length
	        CTBridge::EBFreeMsgCallBack,        //function.
	        (UINT32)pComMsg                     //ComMessage ptr.
	        );
	 }
		

   #endif
#else
#ifndef M_TGT_WANIF
    pComMsg->AddRef();
    //发送WAN

    L2_L3_Wan++;
    if(pComMsg->GetDataLength()>1510)
    	{
    	   L2_L3_Long++;
    	}
    return vxbEtsecRecvMsgFromEB( (char*)pComMsg->GetDataPtr(),       //Data to send
        (UINT16)pComMsg->GetDataLength(),   //Data length
        CTBridge::EBFreeMsgCallBack,        //function.
        (UINT32)pComMsg                     //ComMessage ptr.
        );
#else
	if(Wanif_Switch==0x5a5a)//只有在打开开关的情况下才进行处理
       {
           if(flag == 2)//send to network jy20100318
           {
               pComMsg->AddRef();
	       return vxbEtsecRecvMsgFromEB( (char*)pComMsg->GetDataPtr(),       //Data to send
        (UINT16)pComMsg->GetDataLength(),   //Data length
        CTBridge::EBFreeMsgCallBack,        //function.
        (UINT32)pComMsg                     //ComMessage ptr.
        );
           }
	    else if ( true == SendToAI( pComMsg, 
	                            WorkingWcpeEid, 
	                            GetRelayMsgID( pComMsg ) ,false))
	      	{
	 
	        }
       }
	else
	 {
		   	    pComMsg->AddRef();
	    return vxbEtsecRecvMsgFromEB( (char*)pComMsg->GetDataPtr(),       //Data to send
        (UINT16)pComMsg->GetDataLength(),   //Data length
        CTBridge::EBFreeMsgCallBack,        //function.
        (UINT32)pComMsg                     //ComMessage ptr.
        );
	 }
	#endif
#endif
return true;
}


//add by xiaoweifang to support vlan.
/*============================================================
MEMBER FUNCTION:
    CTBridge::GetGroupID

DESCRIPTION:
    取ComMessage对应用户所属的Vlan

ARGUMENTS:
    *pComMsg:

RETURN VALUE:
    GroupID,   0: no group

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTBridge::GetGroupID(CComMessage *pComMsg)
{
    CMac SrcMac( GetSrcMac( pComMsg ) );
    FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
    return (NULL == pSrcFT)?0:pSrcFT->usGroupId;
}


//////////////////////////////////////////////////
//
//返回IpType对应的TTL.
//
//////////////////////////////////////////////////
UINT16 CTBridge::TTL_IPTYPE(const UINT8 &ucIpType, const bool &bIsServing)
{
#if 0
    return ( IPTYPE_DHCP == ucIpType )?M_TTL_FFFF:
            ( ( IPTYPE_PPPoE == ucIpType )?m_usPPPoESessionAliveTime:
                g_usFixIpTTL );
#else
    /*
     *FixedIP在servingBTS上,不过期.但是在切换走后,TTL更改为1小时
     *在servingBTS上,FixedIP资源的释放依靠OAM的CPE注册定时器或者CPE表的删除
     *在切换后,依靠心跳定时器释放资源
     */
    if ((false == bIsServing) && (IPTYPE_FIXIP == ucIpType))
        return g_usFixIpTTL;
    else
        return (IPTYPE_PPPoE == ucIpType)?m_usPPPoESessionAliveTime : M_TTL_FFFF;
#endif
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::EBConfig

DESCRIPTION:
    数据配置消息处理，处理完后发送回应消息
ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::EBConfig(const CDataConfig &msgDataCfg)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge EBConfig()" );

    SetPPPoESessionAliveTime( msgDataCfg.GetPPPoESessionKeepAliveTime() );
    SetLearnedBridgeAgingTime( msgDataCfg.GetLearnedBridgingAgingTime() );
    SetEgressBCFilter( msgDataCfg.GetEgressBCFltr() );

    //设置转发表表项过期时间
    if ( WM_LEARNED_BRIDGING == m_ucWorkingMode )
        {
        SetExpireTimeInSeconds( m_usLearnedBridgeAgingTime );
        }
    else
        {
        SetExpireTimeInSeconds( m_usPPPoESessionAliveTime );
        }
    if (m_ucWorkingMode != msgDataCfg.GetWorkMode())
        {
        SetWorkingMode( msgDataCfg.GetWorkMode() );
        //安全模式改成学习模式后，CPE侧acl过滤掉数据包的情况
        LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "BTS access control mode is modified, notify all serving CPE to RE-register" );
        notifyAllCPEtoRegister();
        }

    CDataConfigResp msgDataCfgResp;
    if ( false == msgDataCfgResp.CreateMessage( *this ) )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Create EB configure response message failed." );
        return false;
        }
    msgDataCfgResp.SetTransactionId( msgDataCfg.GetTransactionId() );
    msgDataCfgResp.SetResult( ERR_SUCCESS );

    msgDataCfgResp.SetDstTid( msgDataCfg.GetSrcTid() );
    if ( false == msgDataCfgResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Send Data Configure Message failed." );
        msgDataCfgResp.DeleteMessage();
        return false;
        }

    //向网卡注册报文接收函数
    Drv_RegisterEB( CTBridge::RxDriverPacketCallBack );
   Drv_RegisterMacCompare( CTBridge::IsCPEVisitBTSMacCallBack);

    return true;    
}

/*============================================================
MEMBER FUNCTION:
    CTBridge::TosSFIDConfig

DESCRIPTION:
    配置Tos SFID表，处理完后发送回应消息

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::TosSFIDConfig(const CTosSFIDConfig &msgTosSFIDCfg)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge TosSFIDConfig()" );
    UINT8 *pSFID = msgTosSFIDCfg.GetSFID();
    memcpy( m_aSFID, pSFID, sizeof( m_aSFID ) );

    CTosSFIDConfigResp msgTosSFIDConfResp;
    if ( false == msgTosSFIDConfResp.CreateMessage( *this ) )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Create TOS-SFID response message failed." );
        return false;
        }
    msgTosSFIDConfResp.SetTransactionId( msgTosSFIDCfg.GetTransactionId() );
    msgTosSFIDConfResp.SetResult( ERR_SUCCESS );

    msgTosSFIDConfResp.SetDstTid( msgTosSFIDCfg.GetSrcTid() );
    if ( false == msgTosSFIDConfResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Send SFID Configure Message failed." );
        msgTosSFIDConfResp.DeleteMessage();
        return false;
        }

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::VlanGroupConfig

DESCRIPTION:
    Vlan group配置表，处理完后发送回应消息

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
extern bool notifyAllCPEtoRegister();
extern bool notifyOneCpeToRegister(UINT32 EID,bool blStopDataSrv);
bool CTBridge::VlanGroupConfig(const CCfgVlanGroupReq &msgVlanGroupCfg)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge VlanGroupConfig()" );
    //更新Group-VLAN表
    UINT16 result = ERR_SUCCESS;
    ////记录到表内
    T_VlanGroupCfgEle *pGroup = (T_VlanGroupCfgEle*)msgVlanGroupCfg.GetEle();
    UINT16 cnt = pGroup->number;
    if (cnt > M_VLAN_GROUP_MAX)
        result = ERR_FAIL;
    else
        {
        result = ERR_SUCCESS;
        for (UINT16 idx = 0; idx < M_MAX_GROUP_NUM; ++idx)
            {
            m_group[idx].usVlanID = idx;
            }
        for (UINT8 idx = 0; idx < cnt; ++idx)
            {
            UINT16 gID = pGroup->group[idx].usGroupID;
            UINT16 vID = pGroup->group[idx].usVlanID;
            if ((gID < M_MAX_GROUP_NUM) && (vID < M_MAX_GROUP_NUM))
                m_group[gID].usVlanID = vID;
            }
        }

    //通知所有cpe重新注册，获取配置
    LOG(LOG_DEBUG3, LOGNO(EB, EC_EB_NORMAL), "notiry all CPE to register again.");
    if (false == notifyAllCPEtoRegister())
        {
        LOG(LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Fail to notify all CPE to register again.");
        return false;
        }

////回应
    CL3OamCommonRsp msgVlanGroupCfgResp;
    if ( false == msgVlanGroupCfgResp.CreateMessage( *this ) )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Create VLAN-GROUP response message failed." );
        return false;
        }
    msgVlanGroupCfgResp.SetMessageId(M_BTS_EMS_VLAN_GROUP_CFG_RSP);
    msgVlanGroupCfgResp.SetTransactionId( msgVlanGroupCfg.GetTransactionId() );
    msgVlanGroupCfgResp.SetResult( result );
    msgVlanGroupCfgResp.SetDstTid( msgVlanGroupCfg.GetSrcTid() );
    if ( false == msgVlanGroupCfgResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Send Vlan Group Configure Response Message failed." );
        msgVlanGroupCfgResp.DeleteMessage();
        return false;
        }
    return true;
}
#ifdef M_TGT_WANIF
/*============================================================
MEMBER FUNCTION:
    CTBridge::IPEngress

DESCRIPTION:
  IP协议栈包的处理，直接发送给空口，不进行MAC地址分析，相当于egress包的处理后，调用sendtoai函数

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
  void

SIDE EFFECTS:
    none
==============================================================*/
  void CTBridge::IPEngress(CComMessage* pComMsg)
{
//这个函数可以分析SRC MAC地址，如果SRC MAC地址是基站的话，则进行转发。
    //   printf("IPEngress\n");
      if ( true == SendToAI( pComMsg, 
                            pComMsg->GetEID(), 
                            GetRelayMsgID( pComMsg ),false ))
      	{
      	}
}
#endif

void CTBridge::ClearCPEVisitBTSMacFromWan(char *Mac)
{
     if ((m_cpe_to_myBTS_Mac[0] == Mac[0])&& (m_cpe_to_myBTS_Mac[1] == Mac[1] ) &&(m_cpe_to_myBTS_Mac[2] == Mac[2]) &&
	 ( m_cpe_to_myBTS_Mac[3] == Mac[3]) &&
	   (m_cpe_to_myBTS_Mac[4] == Mac[4]) &&(m_cpe_to_myBTS_Mac[5] == Mac[5]))
     	{
     	    UINT8 zMac[6]={0,0,0,0,0,0};   
     	    recordCPEToBTSIPStack( zMac,0,0);
     	}
}
#ifdef WBBU_CODE
unsigned int egresscount = 0;
unsigned int  L3_L2_Eth_Packet = 0;
unsigned int  L3_L2_Eth_Packet_BC = 0;
unsigned int L2_L3_Eth_Packet = 0;
#endif
/*============================================================
MEMBER FUNCTION:
    CTBridge::Egress

DESCRIPTION:
    下行数据转发处理函数
ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::Egress(CComMessage *pComMsg)
{
#ifdef WBBU_CODE
  egresscount++;
#endif
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge Egress()" );
    CComMessage   *pComMsg_temp = NULL;
    //统计从网络侧收到的数据包总长度jiaying20110801
    statFromWan += pComMsg->GetDataLength();  
    statPktFromWan++;
    //统计从网路侧收到的数据包总长度jiaying20110801 end
    //newtat
    m_dataFromWan += pComMsg->GetDataLength();
    //add by xiaoweifang to support vlan{{
    UINT8    *pData = (UINT8*)( pComMsg->GetDataPtr() );
    VLAN_hdr *pVlan = (VLAN_hdr*)( pData + 12 );
	
    if ( M_ETHER_TYPE_VLAN == ntohs( pVlan->usProto_vlan ) )
    {
        pComMsg->SetVlanID(pVlan->usVlanID);//not groupId
        //去掉VLAN tag.
        UINT8 *p = pData + 11;
        for ( UINT8 idx = 12; idx > 0; --idx, --p )
        {
            *( p + 4 ) = *p;
        }
        ///
        pComMsg->SetDataPtr( pData + 4 );
        if (pComMsg->GetDataLength() >= 64)
        {
            pComMsg->SetDataLength( pComMsg->GetDataLength() - 4 );
        }
        else
        {   
            pComMsg->SetDataLength( 60 );   // to make sure the packet is longer than the minimum Ethernet packet 
            // requirement, to prevent the packets being discarded 
        }
    }
    
    else
    {
        pComMsg->SetVlanID(M_NO_VLAN_TAG);//not groupId
    }
#ifdef WBBU_CODE
      CMac DstMac1( GetDstMac( pComMsg ) );
 //   #ifdef M_L3_L2_ETH_PACKET
 #ifdef DIAG_TOOL
    if((DstMac1.IsL2Addr()))//将消息发送给L2
    	{
    	     L3_L2_Ether_Packet((unsigned char*)pComMsg->GetDataPtr(),pComMsg->GetDataLength());
    	     L3_L2_Eth_Packet++;
    	     return;
    	}
 
    if(DstMac1.IsBroadCast())//广播包也发送一份
    	{
    	     L3_L2_Ether_Packet((unsigned char*)pComMsg->GetDataPtr(),pComMsg->GetDataLength());
    	     L3_L2_Eth_Packet_BC++;
    	}
    #endif
#endif
//wangwenhua 2012-2-26 统计下行网关包
    CMac   tempMac1(g_rf_openation.GateWay1_MAC);
    CMac   tempMac2(g_rf_openation.GateWay2_MAC);

    //增加MAC filter功能
    CMac SrcMac( GetSrcMac( pComMsg ) );
  

   if(g_rf_openation.type!=0)
  {
        if((g_rf_openation.GateWay1_valid)&&(g_rf_openation.GateWayIP1!=0))
        {
               if(tempMac1==SrcMac)
               {
                   g_rf_openation.GateWay1_down++;
               }
        }
	     if((g_rf_openation.GateWay2_valid)&&(g_rf_openation.GateWayIP2!=0))
        {
               if(tempMac2==SrcMac)
               {
                   g_rf_openation.GateWay2_down++;
               }
        }
  }
    //}}add by xiaoweifang
    if(Ping_Check_Flag == 1)
    {
        GetPingACKPacket_Seq(pComMsg,2);
        GetPingPacket_Seq(pComMsg,2);
    }
    
    //增加Egress方向的性能统计值
    FROMDIR from = EB_FROM_WAN;
    
    //UINT32 ulLen = pComMsg->GetDataLength();
    UINT16 usProto = GetProtoType( pComMsg );
    
    //设置 Client Mac Address (Dest Mac) 指针
    EtherHdr *pEther = (EtherHdr*)( pComMsg->GetDataPtr() );
    pComMsg->SetKeyMac( pEther->aucDstMAC );
    
    if ( M_ETHER_TYPE_ARP == usProto )
    {
        // ARP packet.
     //   CMac SrcMac( GetSrcMac( pComMsg ) );//wangwenhua add 20081013
        
        ClearCPEVisitBTSMacFromWan((char*)SrcMac.GetMac());//if the mac which visit bts come from wan,
        
        if(true==SrcMac.IsZero())
            return;
        FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
        if(pSrcFT!=NULL)
        {
            if((pSrcFT->bIsTunnel) == true)
            {
                //如果处于切换隧道模式,则不做处?由于可能收到ARP包?
            }
            else
            {
                pSrcFT->usTTL = 0;
                pSrcFT->usElapsed = 0;
                pSrcFT->DM_Sync_Flag = 0x55;
		        g_duplicate_mac[Duplicate_eid_index%20].DebugEID[0] = pSrcFT->ulEid;
		        g_duplicate_mac[Duplicate_eid_index%20].DebugEID[1] = 0;
		        memcpy( g_duplicate_mac[Duplicate_eid_index%20].MacAddress,pSrcFT->aucMAC,M_MAC_ADDRLEN);
                 Duplicate_eid_index++;
                
                LOG1( LOG_DEBUG, LOGNO(EB,EC_EB_SYS_FAIL), "->ARP MAC FROM Engress:eid:%x",pSrcFT->ulEid);
                LOG4( LOG_DEBUG, LOGNO(EB,EC_EB_SYS_FAIL), "->ARP MAC FROM Engress:[X-X-%.2X-%.2X-%.2X-%.2X]",
                pSrcFT->aucMAC[2],pSrcFT->aucMAC[3],pSrcFT->aucMAC[4],pSrcFT->aucMAC[5]);
#ifdef RCPE_SWITCH
				if( trunkIsMRcpe(pSrcFT->ulEid) )
				{
					//if( pSrcFT->ulEid != pComMsg->GetEID() )
					//{
						CComMessage *RspMsg = new ( CTBridge::GetInstance(), M_MAC_ADDRLEN ) CComMessage;
						if( RspMsg != NULL )
						{
							memcpy( (UINT8*)RspMsg->GetDataPtr(), pSrcFT->aucMAC, M_MAC_ADDRLEN );
							RspMsg->SetDstTid( M_TID_UTDM );
							RspMsg->SetMessageId( M_CPE_L3_TRUNK_MAC_MOVEAWAY_NOTIFY );
							RspMsg->SetEID( pSrcFT->ulEid );
							RspMsg->SetSrcTid( (CTBridge::GetInstance())->GetEntityId());
							RspMsg->SetDirection( DIR_TO_AI );
							if( ! CComEntity :: PostEntityMessage( RspMsg ) )
							{
							    RspMsg->Destroy();
								//return false;
							}
						}
	                //}
				}
#endif
				
            }
#ifndef RCPE_SWITCH
            return;
#endif
        }
        
        pComMsg->SetDirection( DIR_FROM_WAN );        
        pComMsg->SetDstTid( M_TID_ARP );
        pComMsg->SetSrcTid( M_TID_EB );
        pComMsg->SetIpType( IPTYPE_ARP );
        pComMsg->SetMessageId( MSGID_ARP_PROXY_REQ );
            if(CComEntity::PostEntityMessage( pComMsg )==false)
            	{
#ifdef WBBU_CODE
                   Commsg_Send_err++;

            	  //   pComMsg->Destroy();
#endif
            	}
        
        //Ingress ARP 包统计值++
        IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_ARP );
        
        return;
    }
    
    UINT8 ucType = IsDhcpPacket( pComMsg );
    CMac DstMac( GetDstMac( pComMsg ) );
    if(true==DstMac.IsZero())
    {
        return;
    }
    
    if (TRUE == bspGetFilterSTP())//是否允许过滤vlan bridge和snap hdlc报文?
    {
        if (true == m_filterMAC.find(DstMac))
        {
            UINT8 strMac[ M_MACADDR_STRLEN ];
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            DATA_log(pComMsg, LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "Downlink packet is filtered.[destination MAC address %s is in the filter MAC table]",DstMac.str( strMac ) );     
            return;
        }
    }
    
    if ( true == DstMac.IsBroadCast() )
    {
        //broad-cast   
        if(true == DstMac.IsMultiCast())//多播地址不管广播过滤开关是否势能，一律扔掉
        {
            //在这里将数据包发送给RCPE
            if(g_BC_test_flag&0x2)////rcpe修改，发给rcpe，wangwenhua20100721
            {
                if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
                {
                    for(int i =0;i<NvRamDataAddr->Relay_num;i++)
                    {
                        if(i>=20)
                        	{
                        	    break;
                        	}
                        if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]!=pComMsg->GetEID()))
                        {
                            pComMsg_temp = new(this,pComMsg->GetDataLength()) CComMessage;
                            if(pComMsg_temp!=NULL)
                            {
                                memcpy((unsigned char*)pComMsg_temp->GetDataPtr(),(unsigned char*)pComMsg->GetDataPtr(),pComMsg->GetDataLength());
                                UINT16 usVlanId = pComMsg->GetVlanID();
                                pComMsg_temp->SetVlanID(usVlanId);                                
                                if( true == SendToAI( pComMsg_temp, 
                                  RelayWanifCpeEid[i], 
                                  GetRelayMsgID( pComMsg_temp ),true )
                                )
                                {
                                }
                            }
                        }
                    
                    }
                }
            }
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_BROADCAST_FORBIDDEN );
            return;
        }
        if ( M_DHCP_FROM_SERVER == ucType ) //下行只转发DHCP Server包
        {
            DhcpHdr *pDhcp = (DhcpHdr*)pComMsg->GetDhcpPtr();
            CMac ChMac( pDhcp->aucChaddr );
            DownlinkPacketDispatcher( pComMsg, usProto, ChMac, ucType );
        }
        else if ( M_DHCP_TO_SERVER == ucType ) 
        {
            //下行的DHCP Client包，丢弃。
            IncreaseDirTrafficMeasureByOne( from, TYPE_EGRESS_DHCPC );
            return;
        }
        else
        {
            //不是DHCP包
            ForwardDownlinkPacket( pComMsg, NULL, true, from );
        }
    }
    else
    {
        //uni-cast
        DownlinkPacketDispatcher( pComMsg, usProto, DstMac, ucType );
    }  
    return;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::DownlinkPacketDispatcher

DESCRIPTION:
    下行数据包分发函数
ARGUMENTS:
    *pComMsg: 消息
    usProto: Ether Type
    DstMac: Mac地址
    ucType: DHCP Type

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::DownlinkPacketDispatcher(CComMessage *pComMsg, UINT16 usProto, CMac &DstMac, UINT8 ucType)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge DownlinkPacketDispatcher()" );

    //增加Egress方向的性能统计值
    FROMDIR from = EB_FROM_WAN;
    TODIR to;
    UINT8 flag;
    FTEntry *pDstFT = GetFTEntryByIdx( BPtreeFind( DstMac ) );
    if ( NULL == pDstFT )
        {
        if(Wanif_Switch==0x5a5a)//wcpe模式下广播包都转jy20100318
        {
             flag = compwithLocalMac(GetDstMac( pComMsg )); 
	     if(flag == 2)
	     	{
                SendToWAN( pComMsg, GetGroupID( pComMsg ) , 2);
	     	}
		 else
		 {
		     flag =  compwithLocalMac(GetSrcMac( pComMsg )); 
			 if(flag == 2)
			 {
			      SendToWAN( pComMsg, GetGroupID( pComMsg ) , 0);
			 }
		 }
	     return;
        }
        //unknown packets ++.
        IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_UNKNOWN );
        return;
        }

#ifdef BUFFER_EN
    if ( false == DstMac.IsBroadCast() )
        {
        ////缓存数据包
        if ( NULL != pDstFT->pBufList )
            pDstFT->pBufList->Buffer( pComMsg );
        }
#endif

    if ( true != pDstFT->bIsServing )
        {
        //用户已离开BTS，隧道转发
         if (true == SendToTunnel(pDstFT, pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
            {
            //Dest Mac的WAN to TDR方向的统计值 ++
            to = EB_TO_TDR;
            IncreaseMacTrafficMeasureByOne( pDstFT, from, to );
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_TDR );
            }
        else
            {
            //丢包统计
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }
        }

    else if ( M_DHCP_FROM_SERVER == ucType )
        {
        //只转发DHCP Server的下行数据
        pComMsg->SetDirection( DIR_FROM_WAN );
        pComMsg->SetIpType( IPTYPE_DHCP );

        pComMsg->SetDstTid( M_TID_SNOOP );
        pComMsg->SetSrcTid( M_TID_EB );
        pComMsg->SetMessageId( MSGID_TRAFFIC_SNOOP_REQ );
        if(CComEntity::PostEntityMessage( pComMsg )==false)
        	{
#ifdef WBBU_CODE
                  Commsg_Send_err++;
        	//   pComMsg->Destroy();
#endif
        	}

        //Dest Mac的WAN to TDR方向的统计值 ++
        IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_DHCP );
        }

    else if ( M_ETHER_TYPE_PPPoE_DISCOVERY == usProto )
        {
        //PPPoE Discovery Stage packet.
        pComMsg->SetDirection( DIR_FROM_WAN );
        pComMsg->SetIpType( IPTYPE_PPPoE );

        pComMsg->SetDstTid( M_TID_SNOOP );
        pComMsg->SetSrcTid( M_TID_EB );
        pComMsg->SetMessageId( MSGID_TRAFFIC_SNOOP_REQ );
        if(CComEntity::PostEntityMessage( pComMsg )==false)
        	{
#ifdef WBBU_CODE
                  Commsg_Send_err++;
        	 //   pComMsg->Destroy();
#endif
        	}

        //Dest Mac的WAN to TDR方向的统计值 ++
        IncreaseDirTrafficMeasureByOne (from, TYPE_TRAFFIC_PPPoEDISCOVERY );
        }

    else
        {
        ForwardDownlinkPacket( pComMsg, pDstFT, false, from);
        }

    //刷新Elapsed time
    pDstFT->usElapsed = 0;

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardDownlinkPacket

DESCRIPTION:
    转发下行数据
ARGUMENTS:
    *pComMsg: 消息
    *pFTEntry: 表项
    bBroadcast: 广播标志
    from: direction (Egress or TDR)

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardDownlinkPacket(CComMessage *pComMsg, FTEntry *pFTEntry, bool bBroadcast, FROMDIR from)
{
    CComMessage   *pComMsg_temp=NULL;
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge ForwardDownlinkPacket()" );
    
    if ( true == bBroadcast )
    {
        //broad-cast
        if(Wanif_Switch==0x5a5a)//wcpe模式下广播包都转jy20100318
            SendToWAN( pComMsg, GetGroupID( pComMsg ) , 2);
        if ((false == m_bEgressBCFltrEn) || (M_ETHER_TYPE_IP_v6 == GetProtoType(pComMsg)))
        {
            if(M_ETHER_TYPE_IP_v6 == GetProtoType(pComMsg))
            {
                DATA_log(pComMsg, LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "IPv6 broadcast packets, discard." );
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_BROADCAST_FORBIDDEN );
                return;
            
            }
            if(g_BC_test_flag&0x4)//rcpe修改，发给rcpe，wangwenhua20100721
            {
                if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
                {
                    for(int i =0;i<NvRamDataAddr->Relay_num;i++)
                    {
                        if(i>=20)
                        	{
                        	  break;
                        	}
                        if((RelayWanifCpeEid[i]!=0))
                        {
                            pComMsg_temp  = new(this,pComMsg->GetDataLength()) CComMessage;
                            if(pComMsg_temp!=NULL)
                            {
                                memcpy((unsigned char*)pComMsg_temp->GetDataPtr(),(unsigned char*)pComMsg->GetDataPtr(),pComMsg->GetDataLength());
                                UINT16 usVlanId = pComMsg->GetVlanID();
                                pComMsg_temp->SetVlanID(usVlanId);                                
                                if ( true == SendToAI( pComMsg_temp, 
                                 RelayWanifCpeEid[i], 
                                 GetRelayMsgID( pComMsg_temp ),true )
                                )
                                {
                                }
                            }
                        }
                        
                    }
                }
            }
            //modify databuffer for adding vlanid
            ModifyPacketWithVlanID(pComMsg);
            //允许转发下行广播,转发给UT
            if ( true == SendToAI( pComMsg, 
             0,
             MSGID_BROADCAST_TRAFFIC_TO_UT ,false)
            )
            {
                //广播包++
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_BROADCAST_AI );
            }
            else
            	{
#ifdef WBBU_CODE
            	    L3_L2_Send_err++;
            	//    pComMsg->Destroy();
#endif
            	}

            #if 0
            //将下行广播包转发给RCPE.
            if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
            {
            for(int i =0;i<NvRamDataAddr->Relay_num;i++)
            {
            if((RelayWanifCpeEid[i]!=0))
            {
            if ( true == SendToAI( pComMsg, 
            RelayWanifCpeEid[i], 
            GetRelayMsgID( pComMsg ),true )
            )
            {
            }
            }
            
            }
            }
            #endif
            
        }
        else
        {
            //不允许转发下行广播
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_BROADCAST_FORBIDDEN );
            //将下行广播包转发给RCPE.
            if(g_BC_test_flag&0x8)//rcpe修改，发给rcpe，wangwenhua20100721
            {
                if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
                {
                    for(int i =0;i<NvRamDataAddr->Relay_num;i++)
                    {
                         if(i>=20)
                         	{
                         	    break;
                         	}
                        if((RelayWanifCpeEid[i]!=0))
                        {
                            pComMsg_temp = new(this,pComMsg->GetDataLength()) CComMessage;
                            if(pComMsg_temp!=NULL)
                            {
                                memcpy((unsigned char*)pComMsg_temp->GetDataPtr(),(unsigned char*)pComMsg->GetDataPtr(),pComMsg->GetDataLength());
                                UINT16 usVlanId = pComMsg->GetVlanID();
                                pComMsg_temp->SetVlanID(usVlanId);                                
                                if ( true == SendToAI( pComMsg_temp, 
                                RelayWanifCpeEid[i], 
                                GetRelayMsgID( pComMsg_temp ),true )
                                )
                                {
                                }
                            }
                        }
                        
                    }
                }
            }
        }
        
        return;
    }
    else
    {
        //uni-cast
        DATA_assert( NULL != pFTEntry );
        if ( true != pFTEntry->bIsAuthed )
        {
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_NOT_AUTHED );
            return;
        }
        
        //lijinan 20081202 进行下行数据统计
        if(uiCdrSwitch == 1)
        {
            UINT16 cdrIndex = cCdrObj.BPtreeFind( pFTEntry->ulEid);
            if(cdrIndex==M_DATA_INDEX_ERR)
            {
	            if(eidIsWifiEid(pFTEntry->ulEid)==true)
	            {
	            	       CMac DstMac(pFTEntry->aucMAC);
		     		cdrIndex = cCdrObj.BPtreeFind(DstMac);
				if(cdrIndex>=M_MAX_UT_PER_BTS)
				{
					/*if(CdrTestFlag)
					{
						UINT8 temB[8];
						DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "downpro wifi eid:%x,mac:%s,have no cdr table",pFTEntry->ulEid,DstMac.str( temB));
					}*/	
					return;
				}
				if(cCdrObj.getWifiUid(cdrIndex)==0)
				{
					return;//还没有认证
				}
	            }
		     else
		     {
		     		if(CdrTestFlag)
					DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "  downpkt eid:%x,get cdr list err",pFTEntry->ulEid);
				//return;
				cCdrObj.cdrMacAdd(pFTEntry->ulEid);
				cdrIndex = cCdrObj.BPtreeFind( pFTEntry->ulEid);
				if(cdrIndex==M_DATA_INDEX_ERR)
				{

						DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " eid:%x down 2,get cdr list err",pFTEntry->ulEid);
						return;
				}
		     }
            }
            //如果终端已经欠费，只对cb3000下来的包放行

	     	if(cCdrObj.GetNoPayFlag(cdrIndex)==1)//wangwenhua add 20120116
		{
			      UINT32 uid1;
			     UINT8 utType1;
				char bw1[8];
			       UINT32  eid1 = pFTEntry->ulEid;
				UINT8 adminstatus1  = 0;
				UINT8 wififlag= 0;
				 if(eidIsWifiEid(eid1)==true)
				 {
				   	wififlag = 1;
					if(CdrTestFlag)
					DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " wifi eid:%x ",eid1);
				 }
			      uid1 = findUidFromEid(eid1,&adminstatus1,&utType1,bw1);
				if(adminstatus1==0)
				{
				     cCdrObj.SetNoPayFlag(cdrIndex,  adminstatus1);
				      if(CdrTestFlag)
				    	DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " eid:%x UID:%x no Pay Flag Err2,",eid1,uid1);
				     
				
				}
		}
            if(cCdrObj.GetNoPayFlag(cdrIndex)==1||(cCdrObj.getWifiUid(cdrIndex)==0xffffffff))
            {
                UINT16 usProto = GetProtoType( pComMsg );
                if(usProto!=M_ETHER_TYPE_IP)
                    return;
                
                EtherHdr *pEtherPkt = (EtherHdr*)(pComMsg->GetDataPtr());
                IpHdr *pIp;
                if(IS_8023_PACKET(ntohs(pEtherPkt->usProto)))
                {
                    pIp = (IpHdr*)( (UINT8*)pEtherPkt + sizeof(EtherHdr) + sizeof(LLCSNAP));
                }
                else
                {
                    pIp = (IpHdr*)( pEtherPkt + 1 );
                }
                
                if(ntohl(pIp->ulSrcIp)!=IP_CB3000)
                {
                    return;
                }                
            }
            else
            {
            		if(real_time_switch)
            		{
		            	if(cCdrObj.GetOverFlag(cdrIndex)==1)
				{
					if(CdrTestFlag)
					{
						DATA_log(NULL, LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), " down pkt eid:%x,have no charge",pFTEntry->ulEid);
					}
					return;
				}
            		}
                semTake (cCdrObj.GetCdrSemId(), WAIT_FOREVER);
                // UINT16 cdrIndex = cCdrObj.BPtreeFind( pFTEntry->ulEid);
                cCdrObj.StatDataFlow(CDR_DOWN, cdrIndex, pComMsg->GetDataLength());
                cCdrObj.StatTimeLen(FromPKT,cdrIndex);		  
                semGive (cCdrObj.GetCdrSemId());
            }
        }

		//lijinan  20101020 for video
	UINT16 toAirMsgId = GetRelayMsgID( pComMsg ) ;
	if(pFTEntry->video_ip[0]!=0)
	{
		UINT32 src_ip = 0; 
		if(M_ETHER_TYPE_IP == GetProtoType(pComMsg))
		{
			src_ip = GetSrcIpAddr(pComMsg);
			if((src_ip==pFTEntry->video_ip[0])||(src_ip==pFTEntry->video_ip[1]))
			{
				toAirMsgId = MSGID_VIDEO_DATA_TRAFFIC;
			}	
		}
		if(video_debug)
		{
			logMsg("\neid:0x%x,rec VSS park,src_ip:0x%x,MsgId:0x%x\n",pFTEntry->ulEid,src_ip,toAirMsgId,0,0,0);
		}

	}
        //单播转发给UT
        if ( true == SendToAI( pComMsg, 
         pFTEntry->ulEid, 
	         toAirMsgId,pFTEntry->bIsRcpe)//GetRelayMsgID( pComMsg ) )
        )
        {
            //Dest Mac的WAN to AI方向的统计值 ++
            IncreaseMacTrafficMeasureByOne( pFTEntry, from, EB_TO_AI );
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_AI );
            
            #if 0
            //lijinan 20081202 进行下行数据统计
            if(uiCdrSwitch == 1)
            {
            semTake (cCdrObj.GetCdrSemId(), WAIT_FOREVER);
            // UINT16 cdrIndex = cCdrObj.BPtreeFind( pFTEntry->ulEid);
            cCdrObj.StatDataFlow(CDR_DOWN, cdrIndex, pComMsg->GetDataLength());
            cCdrObj.StatTimeLen(FromPKT,cdrIndex);
            semGive (cCdrObj.GetCdrSemId());
            }
            #endif            
            }
        else
        	{
#ifdef WBBU_CODE
        	   L3_L2_Send_err++;
        	//    pComMsg->Destroy();
#endif
        }
        //TTL的刷新由本函数调用者执行
        #if 0
        if ( M_TTL_FFFF != pFTEntry->usTTL )
        {
        pFTEntry->usTTL = 0;
        }
        #endif
    }
    
    return;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::TDR

DESCRIPTION:
    隧道数据转发处理函数
ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::TDR(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge TDR()" );
    //统计从隧道收到的数据包总长度jiaying20110801
    statFromTDR += pComMsg->GetDataLength();  
    statPktFromTDR++;
    //统计从隧道收到的数据包总长度jiaying20110801 end
    //new stat
    m_dataFromTDR += pComMsg->GetDataLength();
    //UINT16 ulLen = pComMsg->GetDataLength();
    UINT16 usProto = GetProtoType( pComMsg );

    //增加TDR方向的性能统计值
    FROMDIR from = EB_FROM_TDR;
    TODIR to;
    if(Ping_Check_Flag == 1)
    	{
    	   GetPingPacket_Seq(pComMsg,1);
          GetPingACKPacket_Seq(pComMsg,1);
    	}
    if ( M_ETHER_TYPE_ARP == usProto )
        {
        // ARP packet.
        pComMsg->SetDirection( DIR_FROM_TDR );

        pComMsg->SetDstTid( M_TID_ARP );
        pComMsg->SetSrcTid( M_TID_EB );
        pComMsg->SetIpType( IPTYPE_ARP );
        pComMsg->SetMessageId( MSGID_ARP_PROXY_REQ );
        CComEntity::PostEntityMessage( pComMsg );

        //TDR ARP 包统计值++
        IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_ARP );

        return;
        }

    UINT8 ucType = IsDhcpPacket( pComMsg );
    CMac DstMac( GetDstMac( pComMsg ) );
     if(true==DstMac.IsZero())
     	{
     	    return;
     	}
    if ( true == DstMac.IsBroadCast() )
        {
        //broad-cast.
        if ( M_DHCP_FROM_SERVER == ucType )
            {
            //////////
            pComMsg->SetDirection( DIR_FROM_TDR );
            pComMsg->SetIpType( IPTYPE_DHCP );

            pComMsg->SetDstTid( M_TID_SNOOP );
            pComMsg->SetSrcTid( M_TID_EB );
            pComMsg->SetMessageId( MSGID_TRAFFIC_SNOOP_REQ );
            CComEntity::PostEntityMessage( pComMsg );

            //TDR DHCP 包统计值++
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_DHCP );
            }
        else
            {
            if(Wanif_Switch==0x5a5a)//wcpe模式下广播包都转jy20100318
                SendToWAN( pComMsg, GetGroupID( pComMsg ) , 2);
            //DHCP Client包或其他非DHCP包
            if( true == SendToWAN( pComMsg, GetGroupID( pComMsg ) , 0))    //mod by xiaoweifang
                {
                //TDR to WAN 包统计值++
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_BROADCAST_WAN );
                }
            else
                {
                //丢包统计
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                }
            }

        return;
        }
    else
        {
        //uni-cast.
        FTEntry *pDstFT = GetFTEntryByIdx( BPtreeFind( DstMac ) );
        if ( NULL == pDstFT )
            {
            CMac SrcMac( GetSrcMac( pComMsg ) );
            FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
	     UINT8 flag = compwithLocalMac(GetDstMac( pComMsg )); 
	     
            if((NULL != pSrcFT) && (true == SendToWAN(pComMsg, GetGroupID(pComMsg), flag)))    //by xiaoweifang.
                {
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_WAN );
                }
            else
                {
                //丢包统计
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                }

            return;
            }

        else if ( true != pDstFT->bIsServing )
            {
            //隧道转发
            if (true == SendToTunnel(pDstFT, pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
                {
                //Dest Mac的TDR to TDR方向的统计值 ++
                to = EB_TO_TDR;
                IncreaseMacTrafficMeasureByOne( pDstFT, from, to );
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_TDR );
                }
            else
                {
                //丢包统计
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                }
            }

        else if ( M_ETHER_TYPE_PPPoE_DISCOVERY == usProto )
            {
            //PPPoE Discovery Stage packet.
            pComMsg->SetDirection( DIR_FROM_TDR );
            pComMsg->SetIpType( IPTYPE_PPPoE );

            //确认Client Mac Addr Pointer已设置
            DATA_assert( NULL != pComMsg->GetKeyMac() );

            pComMsg->SetDstTid( M_TID_SNOOP );
            pComMsg->SetSrcTid( M_TID_EB );
            pComMsg->SetMessageId( MSGID_TRAFFIC_SNOOP_REQ );
            CComEntity::PostEntityMessage( pComMsg );

            //TDR PPPoE Discovery 包统计值++
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_PPPoEDISCOVERY );
            }

        else if ( M_DHCP_FROM_SERVER == ucType ) 
            {
            //DHCP packet from server.
            pComMsg->SetDirection( DIR_FROM_TDR );
            pComMsg->SetIpType( IPTYPE_DHCP );

            pComMsg->SetDstTid( M_TID_SNOOP );
            pComMsg->SetSrcTid( M_TID_EB );
            pComMsg->SetMessageId( MSGID_TRAFFIC_SNOOP_REQ );
            CComEntity::PostEntityMessage( pComMsg );

            //TDR DHCP 包统计值++
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_DHCP );
            }

        else if ( M_DHCP_TO_SERVER == ucType ) 
            {
            //从TDR过来的下行DHCP Client包，丢弃
            IncreaseDirTrafficMeasureByOne( from, TYPE_EGRESS_DHCPC );
            }

        else
            {
            ForwardDownlinkPacket( pComMsg, pDstFT, false, from );
            }

        //刷新Elapsed time
        pDstFT->usElapsed = 0;

        return;
        }
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardTraffic

DESCRIPTION:
    提供给其他任务的转发接口，转发经过侦听处理的数据;
    通过该函数转发的数据都是经过侦听的Snoop/ARP等数据，

    不刷新表项的TTL值

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardTraffic(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge ForwardTraffic()" );

    //增加Interal方向的性能统计值
    FROMDIR from = EB_FROM_INTERNAL;

    UINT8 ucDir = pComMsg->GetDirection();
    CMac DstMac( GetDstMac( pComMsg ) );
	if(Wanif_Switch==0x5a5a)
	{
          if ( true == DstMac.IsBroadCast() )
	   {
	     	SendToWAN( pComMsg, GetGroupID( pComMsg )  , 2);		
	   }
	}
	  
    switch ( ucDir )
        {
        case DIR_TO_AI:
            //支持VLAN时有点问题,不同VLAN的上行数据可能
            //能互通起来
            ForwardTrafficToAI( pComMsg );

            //Internal 包统计值++
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_AI );

            break;

        case DIR_TO_WAN:
        {     
	 
	     UINT8 flag = compwithLocalMac(GetDstMac( pComMsg )); 
            if( true == SendToWAN( pComMsg, GetGroupID( pComMsg )  , flag))
                {
                //Internal 包统计值++
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_WAN );
                }
            else
                {
                //丢包统计
                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
                }
            }

            break;

        case DIR_TO_TDR:
            {
		 CMac DstMac1( GetDstMac( pComMsg ) );
		   FTEntry *pDstFT = GetFTEntryByIdx( BPtreeFind( DstMac1 ) ); //wangwenhua modify 20100823
		   if(pDstFT==NULL)
		  {
#if 0
	            if (true == SendToTunnel(pComMsg->GetBtsAddr(), pComMsg->GetBtsPort(),
	                    pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
	                {
	                //Internal 包统计值++
	                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_TDR );
	                }
	            else
	                {
	                //丢包统计
	                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
	                }
#endif
			return ;
		 }
		   else
		   {
		   	       if (true == SendToTunnel(pDstFT,
			                    pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
			                {
			                //Internal 包统计值++
			                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_TDR );
			                }
			            else
			                {
			                //丢包统计
			                IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
			                }
		   	}
            }
            break;

        case DIR_FROM_WAN:
            //may be PADT
            ForwardDownlinkTraffic( pComMsg );
            break;

        case DIR_FROM_TDR:
            //may be PADT
            ForwardTunnelTraffic( pComMsg );
            break;

        case DIR_FROM_AI:
            //may be from ARP Proxy
            ForwardUplinkTraffic( pComMsg );
            break;

        default:
            LOG1( LOG_WARN, LOGNO( EB, EC_EB_MSG_EXCEPTION ), "Forward Traffic: Err direction: %d.", ucDir );
            break;
        }

    return;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardTrafficToAI

DESCRIPTION:
    转发侦听后的数据到UT

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardTrafficToAI(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge ForwardTrafficToAI()" );
    bool flag=false;
    UINT32 ulEid   = pComMsg->GetEID();
    UINT16 usMsgId = 0;

        #ifdef M_TGT_WANIF         
        UINT32 relayeid = pComMsg->GetEID();
        if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)//rcpe修改，扩大到所有类型ip，wangwenhua20100721
        {
            for(int i =0;i<NvRamDataAddr->Relay_num;i++)
            {
              if(i>=20)
              	{
              	   break;
              	}
                if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                {
                    flag =true;
                    break;
                }            
            }
        }
		
        #endif	

    if ( M_DATA_BROADCAST_EID == ulEid )
        {
        //modify databuffer for adding vlanid
        ModifyPacketWithVlanID(pComMsg);
        
        usMsgId = MSGID_BROADCAST_TRAFFIC_TO_UT; 
        }
    else
        {
        usMsgId = GetRelayMsgID( pComMsg );
        }

    //发送到AI.
    if(SendToAI( pComMsg, ulEid, usMsgId,flag )==false)
    	{
#ifdef WBBU_CODE
    	      	   L3_L2_Send_err++;
	//	 pComMsg->Destroy();
#endif
    	}

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardDownlinkTraffic

DESCRIPTION:
    转发侦听/处理后的下行数据

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardDownlinkTraffic(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge ForwardDownlinkTraffic()" );

    //增加Egress方向的性能统计值
    FROMDIR from = EB_FROM_WAN;
    TODIR to;

    CMac DstMac( GetDstMac(pComMsg) );
    FTEntry *pFT = GetFTEntryByIdx( BPtreeFind( DstMac ) );
    if ( NULL == pFT )
        {
            if(Wanif_Switch==0x5a5a)
            {
                UINT8 flag = compwithLocalMac(GetSrcMac( pComMsg )); 
                if(flag==2)
                {
                    SendToWAN( pComMsg, GetGroupID( pComMsg )  , 0);	
                }
            }
        //Egress Unknown 包统计值++
        IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_UNKNOWN );

        return;
        }

    if ( true == pFT->bIsServing )
        {
        //lijinan  20101020 for video
        UINT16 toAirMsgId = GetRelayMsgID( pComMsg );
        if(pFT->video_ip[0]!=0)
	{
	       UINT32 src_ip = 0;
		if(M_ETHER_TYPE_IP == GetProtoType(pComMsg))
		{
			src_ip = GetSrcIpAddr(pComMsg);
			if((src_ip==pFT->video_ip[0])||(src_ip==pFT->video_ip[1]))
			{
				toAirMsgId = MSGID_VIDEO_DATA_TRAFFIC;
			}	
		}
		if(video_debug)
		{
			logMsg("\neid:0x%x,rec VSS park,src_ip:0x%x,MsgId:0x%x\n",pFT->ulEid,src_ip,toAirMsgId,0,0,0);
		}

	}
        //serving,转发到UT
        if ( true == SendToAI( pComMsg, 
                            pFT->ulEid,     /*pComMsg->GetEID()*/
                            toAirMsgId,pFT->bIsRcpe)//GetRelayMsgID( pComMsg ) )
            )
            {
            //Dest Mac的WAN to AI方向的统计值 ++
            to = EB_TO_AI;
            IncreaseMacTrafficMeasureByOne( pFT, from, to );
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_AI );
            }
        else
        	{
#ifdef WBBU_CODE
        	     	   L3_L2_Send_err++;
			//	     	   pComMsg->Destroy();
#endif
            }
        }
    else
        {
        //tunnel
        if (true == SendToTunnel(pFT, pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
            {
            //Dest Mac的WAN to TDR方向的统计值 ++
            to = EB_TO_TDR;
            IncreaseMacTrafficMeasureByOne( pFT, from, to );
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_TDR );
            }
        else
            {
            //丢包统计
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardTunnelTraffic

DESCRIPTION:
    转发侦听/处理后的TDR数据

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardTunnelTraffic(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge ForwardTunnelTraffic()" );

    //增加TDR方向的性能统计值
    FROMDIR from = EB_FROM_TDR;
    TODIR to;

    CMac DstMac( GetDstMac( pComMsg ) );
    FTEntry *pFT = GetFTEntryByIdx( BPtreeFind( DstMac ) );
    if ( NULL == pFT )
        {
        ////
        UINT8 flag = compwithLocalMac(GetDstMac( pComMsg ));
          CMac SrcMac( GetSrcMac( pComMsg ) );
        	FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
        	if((NULL != pSrcFT) && (true == SendToWAN(pComMsg, GetGroupID(pComMsg), flag)))    //by xiaoweifang.
            {
            //TDR to WAN 包统计值++
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_WAN);
            }
        else
            {
            //丢包统计
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }

        return;
        }

    if ( true == pFT->bIsServing )
        {
        //lijinan  20101020 for video
        UINT16 toAirMsgId = GetRelayMsgID( pComMsg );
        if(pFT->video_ip[0]!=0)
	{
		UINT32 src_ip =0;
		if(M_ETHER_TYPE_IP == GetProtoType(pComMsg))
		{
			src_ip = GetSrcIpAddr(pComMsg);
			if((src_ip==pFT->video_ip[0])||(src_ip==pFT->video_ip[1]))
			{
				toAirMsgId = MSGID_VIDEO_DATA_TRAFFIC;
			}	
		}
		if(video_debug)
		{
			logMsg("\neid:0x%x,rec VSS park,src_ip:0x%x,MsgId:0x%x\n",pFT->ulEid,src_ip,toAirMsgId,0,0,0);
		}

	}
        //serving
        if ( true == SendToAI( pComMsg, 
                            pFT->ulEid , 
                            toAirMsgId,pFT->bIsRcpe)//GetRelayMsgID( pComMsg ) )
            )
            {
            //Dest Mac的TDR to AI方向的统计值 ++
            to = EB_TO_AI;
            IncreaseMacTrafficMeasureByOne( pFT, from, to );
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_AI );
            }
        else
        	{
#ifdef WBBU_CODE
        	     	   L3_L2_Send_err++;
				//     	   pComMsg->Destroy();
#endif
            }
        }
    else
        {
        ////
        if( true == SendToWAN( pComMsg, GetGroupID( pComMsg ) , 0) )
            {
            //Dest Mac的TDR to WAN方向的统计值 ++
            to = EB_TO_WAN;
            IncreaseMacTrafficMeasureByOne( pFT, from, to );
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_WAN );
            }
        else
            {
            //丢包统计
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardUplinkTraffic

DESCRIPTION:
    转发侦听/处理后的上行数据

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardUplinkTraffic(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge ForwardUplinkTraffic()" );

    //增加AI方向的性能统计值
    FROMDIR from = EB_FROM_AI;
    TODIR to;

    CMac    SrcMac( GetSrcMac( pComMsg ) );
    FTEntry *pSrcFT = GetFTEntryByIdx( BPtreeFind( SrcMac ) );
    if ( NULL == pSrcFT )
        {
        UINT8 flag = compwithLocalMac(GetDstMac( pComMsg )); 
        //discard or send to wan??
        if( true == SendToWAN( pComMsg, 0  , flag))
            {
            //Src Mac的AI to WAN方向的统计值 ++
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_WAN );
            }
        else
            {
            //丢包统计
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }

        return;
        }
    if ( true == pSrcFT->bIsTunnel )
        {
        //tunnel.
        if (true == SendToTunnel(pSrcFT, pComMsg->GetDataLength(), pComMsg->GetDataPtr()))
            {
            //Src Mac的AI to TDR方向的统计值 ++
            to = EB_TO_TDR;
            IncreaseMacTrafficMeasureByOne( pSrcFT, from, to );
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_TDR );
            }
        else
            {
            //丢包统计
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }
        }
    else
        {

         CMac DstMac( GetDstMac( pComMsg ) );

         FTEntry *pDstFT = GetFTEntryByIdx( BPtreeFind( DstMac ) );
	 if(NULL != pDstFT)
   	    {//the destination mac is in this bts
             if( pSrcFT->ulEid == pDstFT->ulEid )	
   	        { //both source and dest in one cpe,discard
   	      	return;
   	        }    	     	
   	     if(pDstFT->bIsServing)
   	     	{
		 if ( true == SendToAI( pComMsg, 
		                        pDstFT->ulEid, 
		                        GetRelayMsgID( pComMsg ) ,pDstFT->bIsRcpe)
		     )
		 	{
		            to = EB_TO_AI;
		            IncreaseMacTrafficMeasureByOne( pDstFT, from, to );
		            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_AI );
		 	}
               return; 
   	     	}
	     }
		
        if( true == SendToWAN( pComMsg, pSrcFT->usGroupId, 0))
            {
            //Src Mac的AI to WAN方向的统计值 ++
            to = EB_TO_WAN;
            IncreaseMacTrafficMeasureByOne( pSrcFT, from, to );
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_TO_WAN );
            }
        else
            {
            //丢包统计
            IncreaseDirTrafficMeasureByOne( from, TYPE_TRAFFIC_LOST );
            }
        }

    return;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::FTAddEntry

DESCRIPTION:
    增加转发表表项

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::FTAddEntry(const CFTAddEntry &msgFTAddEntry)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge FTAddEntry()" );

    CMac Mac( msgFTAddEntry.GetMac() );

    if ( true == Mac.IsBroadCast() )
        {
        //Err FT entry.
        LOG( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "Err! Trying to add a broadcast forwarding entry." );
        return;
        }

    MACFilterEntry *pMFT = GetMFTEntryByIdx( MFTBPtreeFind( Mac ) );
    if(NULL != pMFT)
        {
        DATA_log(NULL, LOG_WARN, LOGNO(EB, EC_EB_PARAMETER), "Err! MAC[%.2X-%.2X-%.2X-%.2X-%.2X-%.2X] is filtered, pls check EID[%.8x]'s FixedIP configuration.", pMFT->MAC[0], pMFT->MAC[1], pMFT->MAC[2], pMFT->MAC[3], pMFT->MAC[4], pMFT->MAC[5], msgFTAddEntry.GetEidInPayload());
        return;
        }

    FTEntry *pFT = GetFTEntryByIdx( BPtreeFind( Mac ) );
    if ( NULL == pFT )
        {
        //新建转发表表项
        UINT16 usIdx  = GetFreeFTEntryIdxFromList();
        pFT = GetFTEntryByIdx( usIdx );
        if ( NULL == pFT )
            {
            //用户满,删除消息；
            LOG( LOG_WARN, LOGNO( EB, EC_EB_NO_ENTRY ), "WARNING! no more free forwarding entry when trying to add a new entry." );
            return;
            }
        BPtreeAdd( Mac,usIdx );


        //lijinan 20081203 数据计费系统增加
        UINT32 cdrEid = msgFTAddEntry.GetEidInPayload();
	 cCdrObj.cdrMacAdd(cdrEid);
	 
        }
    else
        {
        ////存在表项，统计值--
        CalcUsrCount( pFT, M_COUNT_USERTYPE_DEL );
        }

    ////
    FTEntry tmpFT;
    tmpFT.bIsOccupied   = true;
    tmpFT.ulEid         = msgFTAddEntry.GetEidInPayload();
    tmpFT.usGroupId     = msgFTAddEntry.getGroupId();   //add by xiaoweifang to support vlan.
    memcpy( tmpFT.aucMAC, msgFTAddEntry.GetMac(), M_MAC_ADDRLEN );
    tmpFT.bIsServing    = msgFTAddEntry.GetServing();
    tmpFT.bIsTunnel     = msgFTAddEntry.GetTunnel();
    tmpFT.ulPeerBtsID   = msgFTAddEntry.GetPeerBtsID();
    tmpFT.ulPeerBtsAddr = 0;
    tmpFT.usPeerBtsPort = 0;
    tmpFT.usElapsed     = 0;
    tmpFT.usTTL         = TTL_IPTYPE( msgFTAddEntry.GetIpType(), tmpFT.bIsServing );
    tmpFT.bIsAuthed     = msgFTAddEntry.GetAuth();
    tmpFT.bIsRcpe     =false;
    
    UINT32 relayeid = tmpFT.ulEid ;
    if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)//rcpe修改，扩大到所有类型ip，wangwenhua20100721
        {
            for(int i =0;i<NvRamDataAddr->Relay_num;i++)
            {
                if(i>=20)
                	{
                	   break;
                	}
                if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                {
                    tmpFT.bIsRcpe     =true;
                    break;
                }        
            }      
    	 }

#ifdef BUFFER_EN
    HandleBuffer( pFT, tmpFT );
#endif
	     if(pFT->ulEid!=tmpFT.ulEid&&pFT->ulEid!=0)
	     {
		if(CdrTestFlag)
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "\nFTAddEntry eid:0x%x,eid bf:0x%x\n",tmpFT.ulEid,pFT->ulEid );
		//lijinan 20081203 数据计费系统增加
		 cCdrObj.cdrMacAdd(tmpFT.ulEid);
	 	 cCdrObj.DelEid(pFT->ulEid,0);
		  g_duplicate_mac[Duplicate_eid_index%20].DebugEID[0] = pFT->ulEid;
		   g_duplicate_mac[Duplicate_eid_index%20].DebugEID[1] = tmpFT.ulEid;
		   memcpy( g_duplicate_mac[Duplicate_eid_index%20].MacAddress,tmpFT.aucMAC,M_MAC_ADDRLEN);
                 Duplicate_eid_index++;
		 
	     }
    INIT_FT( pFT, tmpFT );

    //增加统计值
    CalcUsrCount( pFT, M_COUNT_USERTYPE_ADD );

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::FTUpdateEntry

DESCRIPTION:
    修改转发表表项

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::FTUpdateEntry(const CFTUpdateEntry &msgFTUpdateEntry)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge FTUpdateEntry()" );

    CMac Mac( msgFTUpdateEntry.GetMac() );
    UINT32 relayeid = 0;
    if ( true == Mac.IsBroadCast() )
        {
        //Err FT entry.
        return;
        }

    MACFilterEntry *pMFT = GetMFTEntryByIdx( MFTBPtreeFind( Mac ) );
    if(NULL != pMFT)
        {
        DATA_log(NULL, LOG_WARN, LOGNO(EB, EC_EB_PARAMETER), "Err! MAC[%.2X-%.2X-%.2X-%.2X-%.2X-%.2X] is filtered, pls check EID[%.8x]'s FixedIP configuration.", pMFT->MAC[0], pMFT->MAC[1], pMFT->MAC[2], pMFT->MAC[3], pMFT->MAC[4], pMFT->MAC[5], msgFTUpdateEntry.GetEidInPayload());
        return;
        }

    FTEntry *pFT = GetFTEntryByIdx( BPtreeFind( Mac ) );
    if ( NULL == pFT )
        {
        //新建转发表表项
        UINT16 usIdx  = GetFreeFTEntryIdxFromList();
        pFT = GetFTEntryByIdx( usIdx );
        if ( NULL == pFT )
            {
            //用户满,删除消息；
            LOG( LOG_WARN, LOGNO( EB, EC_EB_NO_ENTRY ), "WARNING! cant update a new entry." );
            return;
            }

        ////
        FTEntry tmpFT;
        tmpFT.bIsOccupied   = true;
        tmpFT.ulEid         = msgFTUpdateEntry.GetEidInPayload();
        tmpFT.usGroupId     = msgFTUpdateEntry.getGroupId();    //add by xiaoweifang to support vlan.
        memcpy( tmpFT.aucMAC, msgFTUpdateEntry.GetMac(), M_MAC_ADDRLEN );
        tmpFT.bIsServing    = msgFTUpdateEntry.GetServing();
        tmpFT.bIsTunnel     = msgFTUpdateEntry.GetTunnel();
        tmpFT.ulPeerBtsID   = msgFTUpdateEntry.GetPeerBtsID();
        tmpFT.ulPeerBtsAddr = 0;
        tmpFT.usPeerBtsPort = 0;
        tmpFT.usElapsed     = 0;
        tmpFT.bIsAuthed     = msgFTUpdateEntry.GetAuth();
        tmpFT.usTTL         = TTL_IPTYPE( msgFTUpdateEntry.GetIpType(), tmpFT.bIsServing );
	 tmpFT.bIsRcpe  =false;
	  relayeid = tmpFT.ulEid ;
	     if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)//rcpe修改，扩大到所有类型ip，wangwenhua20100721
            {
               for(int i =0;i<NvRamDataAddr->Relay_num;i++)
               {
               if(i>=20)
               	{
               	   break;
               	}
                if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                   {
                    tmpFT.bIsRcpe     = true;
                    break;
                   }        
               }      
    	     }

#ifdef BUFFER_EN
        HandleBuffer( pFT, tmpFT );
#endif

        INIT_FT( pFT, tmpFT );

        BPtreeAdd( Mac,usIdx );

	//lijinan 20081203 数据计费系统增加
	 cCdrObj.cdrMacAdd(pFT->ulEid);


        //增加统计值
        CalcUsrCount( pFT, M_COUNT_USERTYPE_ADD );
        }
    else
        {
        if (true == msgFTUpdateEntry.getRefreshTTLOnly())
            {
            //Tunnel Heart Beat回应,仅仅刷新TTL
            pFT->usElapsed = 0;
            }
        else
            {
            //表项重复，覆盖
            FTEntry tmpFT;
            tmpFT.bIsOccupied   = true;
            tmpFT.ulEid         = msgFTUpdateEntry.GetEidInPayload();
            tmpFT.usGroupId     = msgFTUpdateEntry.getGroupId();    //add by xiaoweifang to support vlan.//切换时anchor DM的表已经被删除
            memcpy( tmpFT.aucMAC, msgFTUpdateEntry.GetMac(), M_MAC_ADDRLEN );
            tmpFT.bIsServing    = msgFTUpdateEntry.GetServing();
            tmpFT.bIsTunnel     = msgFTUpdateEntry.GetTunnel();
            tmpFT.ulPeerBtsID   = msgFTUpdateEntry.GetPeerBtsID();
            tmpFT.ulPeerBtsAddr = 0;
            tmpFT.usPeerBtsPort = 0;
            tmpFT.usElapsed     = 0;
            tmpFT.usTTL         = TTL_IPTYPE( msgFTUpdateEntry.GetIpType(), tmpFT.bIsServing );
            tmpFT.bIsAuthed     = msgFTUpdateEntry.GetAuth();
	     tmpFT.bIsRcpe   = false;

             relayeid = tmpFT.ulEid ;
            if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)//rcpe修改，扩大到所有类型ip，wangwenhua20100721
            {
               for(int i =0;i<NvRamDataAddr->Relay_num;i++)
               {
                 if(i>=20)
                 	{
                 	    break;
                 	}
                if((RelayWanifCpeEid[i]!=0)&&(RelayWanifCpeEid[i]==relayeid)&&(relayeid!=0))
                   {
                    tmpFT.bIsRcpe     = true;
                    break;
                   }        
               }      
    	     }
#ifdef BUFFER_EN
            HandleBuffer( pFT, tmpFT );
#endif

            ////存在表项，统计值--
            CalcUsrCount( pFT, M_COUNT_USERTYPE_DEL );
	     if(pFT->ulEid!=tmpFT.ulEid)
	     {
		if(CdrTestFlag)
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "\nFTUpdateEntry eid:0x%x,eid bf:0x%x\n",tmpFT.ulEid,pFT->ulEid );
		//lijinan 20081203 数据计费系统增加
		 cCdrObj.cdrMacAdd(tmpFT.ulEid);
	 	 cCdrObj.DelEid(pFT->ulEid,0);
		   g_duplicate_mac[Duplicate_eid_index%20].DebugEID[0] = pFT->ulEid;
		   g_duplicate_mac[Duplicate_eid_index%20].DebugEID[1] = tmpFT.ulEid;
		   memcpy( g_duplicate_mac[Duplicate_eid_index%20].MacAddress,tmpFT.aucMAC,M_MAC_ADDRLEN);
                 Duplicate_eid_index++;
		 
	     }

            UPDATE_FT( pFT, tmpFT );

            //增加统计值
            CalcUsrCount( pFT, M_COUNT_USERTYPE_ADD );       
            }
        }

    return;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::FTDelEntry

DESCRIPTION:
    Mac地址对应的转发表表项TTL值=0;等待cleanup任务删除

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::FTDelEntry(const CFTDelEntry &msgFTDelEntry)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge FTDelEntry()" );

    CMac    Mac( msgFTDelEntry.GetMac() );
    UINT16  usIdx = BPtreeFind(Mac);
    FTEntry *pFTEntry = GetFTEntryByIdx( usIdx );
    if ( NULL == pFTEntry )
        {
        return;
        }

    //10s后由clean up任务通知删除
    pFTEntry->usTTL     = 0;
    pFTEntry->usElapsed = 0;
    LOG1( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge FTDelEntry(), eid:%x", pFTEntry->ulEid);
    if (true == msgFTDelEntry.isCreateTempTunnel())
        {
        pFTEntry->DM_Sync_Flag = 0x55;//wangwenhua add 20110918
        CalcUsrCount( pFTEntry, M_COUNT_USERTYPE_DEL );

        ////目前是ChangeAnchor的情况，需要创建临时隧道
        FTEntry stOldFT;
        memcpy(&stOldFT, pFTEntry, sizeof(stOldFT));
        pFTEntry->bIsServing    = false;
        pFTEntry->bIsTunnel     = true;
        pFTEntry->ulPeerBtsAddr = msgFTDelEntry.getTunnelPeerBtsIP();
        pFTEntry->usPeerBtsPort = msgFTDelEntry.getTunnelPeerBtsPort();

        CalcUsrCount( pFTEntry, M_COUNT_USERTYPE_ADD );
#ifdef BUFFER_EN
        HandleBuffer((FTEntry*)&stOldFT, *pFTEntry);
#endif
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::EntryAgeOut

DESCRIPTION:
    Mac地址对应的转发表表项到期，删除

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::EntryAgeOut(const CFTDelEntry &msgFTDelEnt,unsigned char *flag)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge EntryAgeOut()" );

    CMac    Mac( msgFTDelEnt.GetMac() );
    UINT16  usIdx = BPtreeFind(Mac);
    FTEntry *pFTEntry = GetFTEntryByIdx( usIdx );
    if ( NULL != pFTEntry )
        {
          if(pFTEntry->DM_Sync_Flag==0x55)
            {
            //  *flag = 1;//wangwenhua mark 2012-11-1
            }
        //切换过快问题,防止新注册的用户数据被删除
        if ( pFTEntry->usTTL > pFTEntry->usElapsed )
        {
           LOG1( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge EntryAgeOut(), FT is changed!!return!!!, eid:%x", pFTEntry->ulEid);
           return false;
        }
	if(*flag==1)//wangwenhua add 2012-6-20
	{
	   LOG1( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge EntryAgeOut(), Not Delete FT, eid:%x", pFTEntry->ulEid);
       //修复由于强制转换状态导致的统计错误
       CalcUsrCount( pFTEntry, M_COUNT_USERTYPE_DEL );
	   pFTEntry->usTTL = 2000;
	   pFTEntry->DM_Sync_Flag =0;
       pFTEntry->bIsServing = true;
       pFTEntry->bIsTunnel = false;
       CalcUsrCount( pFTEntry, M_COUNT_USERTYPE_ADD);
	     return true;
	}
         LOG2( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge EntryAgeOut(),  delete eid:%x,DM_Sync_Flag:%x", pFTEntry->ulEid,pFTEntry->DM_Sync_Flag);
         LOG2( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge EntryAgeOut(),  delete eid:%x,DM_Sync_Flag:%x", pFTEntry->ulEid,pFTEntry->DM_Sync_Flag);
	        if(eidIsWifiEid(pFTEntry->ulEid)==true)
				 cCdrObj.DelWifiMac(Mac);
		 else
		 {
		 	if(CdrTestFlag)
				LOG1( LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "\n->Bridge EntryAgeOut(),eid :0x%x\n",pFTEntry->ulEid );
		        //lijinan 20081203 计费系统新增
			 cCdrObj.DelEid(pFTEntry->ulEid,0);
		 }

        
        BPtreeDel( Mac );
#ifdef BUFFER_EN
        if ( NULL != pFTEntry->pBufList )
            {
            delete pFTEntry->pBufList;
            }
#endif

        ////统计值--
        CalcUsrCount( pFTEntry, M_COUNT_USERTYPE_DEL );

        memset( pFTEntry, 0, sizeof( FTEntry ) );
        InsertFreeFTList( usIdx );
        }

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::FTEntryExpire

DESCRIPTION:
    Mac对应的转发表表项超时处理

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTBridge::FTEntryExpire(CMessage &msg)
{
    CMac Mac( ( (CFTDelEntry&)msg ).GetMac() );
    UINT8 strMac[ M_MACADDR_STRLEN ];
    LOG1( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge FTEntryExpire(MAC:%s)", (int)Mac.str( strMac ) );
     unsigned char flag = 0;
    if(!EntryAgeOut( msg,&flag ))
        return false;

    CFTEntryExpire msgFTEntryExp( msg );
    msgFTEntryExp.SetDstTid( M_TID_SNOOP );
    msgFTEntryExp.SetSrcTid( M_TID_EB );
    if(flag==1)
    {
       msgFTEntryExp.SetFlag(1);
    }
    if ( false == msgFTEntryExp.Post() )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Send FT Entry Expire Message failed." );
        //msgFTEntryExp.DeleteMessage();
        return false;
        }
    return true;;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::FTModifyBTSPubIP & Port

DESCRIPTION:
    Mac对应的转发表表项PeerBTSIP 更新

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::FTModifyBTSPubIP(const CTunnelNotifyBTSIP &msgBTSIP)
{
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge FTModifyBTSPubIP()" );

    CMac Mac( msgBTSIP.GetMac() );

#ifndef NDEBUG
    if ( true == Mac.IsBroadCast() )
        {
        //Err FT entry.
        LOG( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "Err! Trying to add a broadcast forwarding entry." );
        return;
        }
#endif

    FTEntry *pFT = GetFTEntryByIdx( BPtreeFind( Mac ) );
    if ( NULL == pFT )
        {
        return;
        }

    if(pFT->ulPeerBtsID == msgBTSIP.GetSenderBtsID())
        {
        pFT->ulPeerBtsAddr = msgBTSIP.GetSenderBtsIP();
        pFT->usPeerBtsPort = msgBTSIP.GetSenderPort();
        }
    return;
}

/*============================================================
MEMBER FUNCTION:
    CTBridge::EBNotifyBTSIPChange

DESCRIPTION:
    OAM通知EB当前的BTS更新了动态的IP&Port,检索转发表项，漫游过来的用户，逐个通知更新IP&Port

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    none

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::EBNotifyBTSIPChange(const CDataNotifyBtsIP &msgNotifyBTSIp)
{
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge EBNotifyBTSIPChange()" );

    UINT32 ulPubIp=msgNotifyBTSIp.GetBTSPubIP();
    UINT16 usPubPort=msgNotifyBTSIp.GetBTSPubPort();

    SetNotifyBTSPubIP(true);

    return;
}

void CTBridge::EBSendBTSIPChangeReq(const FTEntry& entry)
{
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge EBSendBTSIPChangeReq()" );
    CTunnelNotifyBTSIP msgTunnelNotifyBTSIP;

    if (entry.bIsServing != entry.bIsTunnel)
        {
        //发现表项是漫游用户，发消息通知ServingBTS更新
        if (msgTunnelNotifyBTSIP.CreateMessage(*(CTBridge::GetInstance())))
            {
            msgTunnelNotifyBTSIP.SetBTS(entry.ulPeerBtsID);
            msgTunnelNotifyBTSIP.SetEidInPayload(entry.ulEid);
            msgTunnelNotifyBTSIP.SetMac(entry.aucMAC);
            //edit by yhw
            msgTunnelNotifyBTSIP.SetDstBtsID(entry.ulPeerBtsID);
            msgTunnelNotifyBTSIP.SetSenderBtsID(::bspGetBtsID());
            msgTunnelNotifyBTSIP.SetSenderBtsIP(::bspGetBtsPubIp());
            msgTunnelNotifyBTSIP.SetSenderPort(::bspGetBtsPubPort());

            msgTunnelNotifyBTSIP.SetDstTid(M_TID_TUNNEL);

            if (false == msgTunnelNotifyBTSIP.Post())
                {
                LOG( LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "Send Tunnel Establish Request failed." );
                msgTunnelNotifyBTSIP.DeleteMessage();
                }
            }
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::GetSFID

DESCRIPTION:
    取pComMessage中Ip头的Tos值，然后从SFID表找对应的SFID值
    如果pComMessage中不包含一个Ip头，则返回默认的SFID值(低优先级)
    Ether Bridge任务发往UT的消息都是Traffic消息，必须包含
    Ethernet Header
    CPE上的UTDM任务有一个相同的函数，请保持一致

ARGUMENTS:
    *pComMessge: 消息

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
UINT8 CTBridge::GetTosSFID(const CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge GetTosSFID()" );
    if ( M_ETHER_TYPE_IP != GetProtoType(pComMsg) )
    {
        //不是Ip包，返回低优先级
        return M_SFID_LOW;
    }

    EtherHdr *pEther = (EtherHdr*)( pComMsg->GetDataPtr() );
    IpHdr *pIp;
    if(IS_8023_PACKET(ntohs(pEther->usProto)))
    {
        pIp = (IpHdr*)((UINT8*)pEther + sizeof(EtherHdr) + sizeof(LLCSNAP));
    }
    else
    {
        pIp = (IpHdr*)((UINT8*)pEther + sizeof(EtherHdr));
    }
    
    return m_aSFID[ pIp->ucTOS ];
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::GetRelayMsgID

DESCRIPTION:
    根据SFID，返回相应的MsgID;

ARGUMENTS:
    ucSFID:

RETURN VALUE:
    UINT16: MsgID

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTBridge::GetRelayMsgID(const CComMessage *pComMsg)
{
   UINT8 iptype=pComMsg->GetIpType();

	
   if( ( pComMsg->GetDataLength() < 100 )&&((iptype==IPTYPE_TCP)||(iptype==IPTYPE_ICMP ) ) )
        {
        //数据比较小的tcp/icmp包，优先级提高
        return MSGID_HIGH_PRIORITY_TRAFFIC;
        }
    else if( ( iptype==IPTYPE_DHCP )|| ( iptype==IPTYPE_PPPoE ) ||( iptype==IPTYPE_ARP ) )
        {
        //优先级发送DHCP/PPPOE/ARP
        return MSGID_HIGH_PRIORITY_TRAFFIC;
        }   
 

    switch( GetTosSFID( pComMsg )  )
        {
        case M_SFID_HIGH:
            //高优先级
            return MSGID_HIGH_PRIORITY_TRAFFIC;                

        case M_SFID_LOW:
            //低优先级
            return MSGID_LOW_PRIORITY_TRAFFIC;                

        case M_SFID_REALTIME:
            //实时
            return MSGID_REALTIME_TRAFFIC;                

        default:
            //其他...返回低优先级
            return MSGID_LOW_PRIORITY_TRAFFIC;                
        }
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::GetMacTrafficMeasure

DESCRIPTION:
    获取指定MAC的不同方向上的转发包统计值

ARGUMENTS:
    Mac: Mac地址

RETURN VALUE:
    UINT32**: 统计值二维数组的首指针

SIDE EFFECTS:
    none
==============================================================*/
UINT32** CTBridge::GetMacTrafficMeasure(CMac &Mac)
{
    FTEntry *pFT = GetFTEntryByIdx( BPtreeFind( Mac ) );
    if ( NULL == pFT )
        {
        return NULL;
        }

    return (UINT32**)( pFT->aulTrafficMeasure ) ;
}


/**********************************
 *UPDATE_FT: 修改一条转发表表项pFT
 *保留性能计数值
 */
void CTBridge::UPDATE_FT(FTEntry *pFT, FTEntry &tmpFT)
{
    if ( NULL == pFT )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "[-UPDATE_FT-InputErr-]:Err para" );
        return;
        }

    pFT->bIsOccupied    = tmpFT.bIsOccupied;
    pFT->ulEid          = tmpFT.ulEid;
    pFT->usGroupId      = tmpFT.usGroupId;   //add by xiaoweifang to support vlan.
    memcpy( pFT->aucMAC, tmpFT.aucMAC, M_MAC_ADDRLEN );
    pFT->bIsServing     = tmpFT.bIsServing;
    pFT->bIsTunnel      = tmpFT.bIsTunnel;
    pFT->ulPeerBtsID    = tmpFT.ulPeerBtsID;
    pFT->ulPeerBtsAddr  = tmpFT.ulPeerBtsAddr;
    pFT->usPeerBtsPort  = tmpFT.usPeerBtsPort;
    pFT->usTTL          = tmpFT.usTTL;
    pFT->usElapsed      = tmpFT.usElapsed;
    pFT->bIsAuthed      = tmpFT.bIsAuthed;
    pFT->bIsRcpe         = tmpFT.bIsRcpe;
#ifdef TRUNK_CODE_VLAN
    pFT->usVlanID = tmpFT.usVlanID;
#endif
    pFT->DM_Sync_Flag = 0;//wangwenhua add 2012-6-20
    return;
}


/**********************************
 *INIT_FT: 初始化一条转发表表项pFT
 *性能计数值清0
 */
void CTBridge::INIT_FT(FTEntry *pFT, FTEntry &tmpFT )
{
    if ( NULL == pFT )
        {
        LOG( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "[-INIT_FT-InputErr-]:Err para" );
        return;
        }
    UPDATE_FT( pFT, tmpFT );
    memset( pFT->aulTrafficMeasure, 0, sizeof( pFT->aulTrafficMeasure ) ); 
}


#ifdef BUFFER_EN
/****************************************
 *HandleBuffer: 处理该转发表项的缓存数据
 */
void CTBridge::HandleBuffer(FTEntry *pFT, FTEntry &tmpFT)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge HandleBuffer" );

    DATA_assert( NULL != pFT );
    if ( NULL == pFT->pBufList )
        {
        pFT->pBufList = new CBufferList;
        }
    else if ( true == pFT->bIsAuthed )
        {
        /////把缓存的数据包重发
        if ( ( (false == pFT->bIsTunnel) && (true  == tmpFT.bIsTunnel) )/*漫游离开*/
          || ( (true  == pFT->bIsTunnel) && (false == tmpFT.bIsTunnel) )/*漫游回来*/
          || ( (true  == pFT->bIsTunnel) && (true  == tmpFT.bIsTunnel) && (pFT->ulPeerBtsID != tmpFT.ulPeerBtsID) )/*从ServingBTS漫游到其他ServingBTS*/
            )
            {
            /****************************
             *1)Anchor漫游到Serving
             *2)Serving漫游到Serving
             *3)Serving漫游回Anchor
             *需要把缓存的数据重发
             *ChangeAnchor目前不支持重发
             */
            pFT->pBufList->ReForward();
            }
        }

    return;
}
#endif


/****************************************
 *CalcUsrCount: 计算不同类型用户的数量
 */
void CTBridge::CalcUsrCount(FTEntry *pFT, bool bDelUser)
{
    DATA_assert( NULL != pFT );
    if ( true == bDelUser )
    {
        if ( false == pFT->bIsServing )
        {
            if(m_usNotServing>0)
                --m_usNotServing;
        }
        else
        {
            if(m_usNowServing>0)
                --m_usNowServing;
            ( false == pFT->bIsTunnel )?( --m_usAnchorAndServing ):( --m_usServingNotAnchor );
        }
    }
    else
    {
        if ( false == pFT->bIsServing )
        {
            ++m_usNotServing;
        }
        else
        {
            ++m_usNowServing;
            ( false == pFT->bIsTunnel )?( ++m_usAnchorAndServing ):( ++m_usServingNotAnchor );
        }
    }
}



#ifndef __WIN32_SIM__
/*============================================================
MEMBER FUNCTION:
    CTBridge::GetComMessage

DESCRIPTION:
    从ComMessage链表中获取ComMessage.

ARGUMENTS:
    void

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
CComMessage* CTBridge::GetComMessage()
{
    //Get first Node.
#if 0
    stComMessageNode *pNode = (stComMessageNode *)lstGet( &m_listComMessage );
    if ( NULL == pNode )
        {
        ::intUnlock( intKey );
        return NULL;
        }
#endif
    //空闲表头插入到空闲表头链表
    ::taskLock();
    UINT32 intKey = ::intLock();
    if (NULL == m_plistComMessage)
        {
        ::intUnlock( intKey );
        ::taskUnlock();
        return NULL;
        }
    //lstAdd( &m_listFreeNode, &( pNode->lstHdr ) );
    CComMessage *pComMsg = m_plistComMessage;
    m_plistComMessage    = m_plistComMessage->getNext();
    ++g_ulOccupiedCounter;
    ::intUnlock( intKey );
    ::taskUnlock();
    //CComMessage *pComMsg = pNode->pstComMsg;
    return pComMsg;
}
#endif
#ifdef WBBU_CODE
unsigned int  RxCount = 0;
unsigned int Rxcount1 = 0;
unsigned int RX_Rls = 0;


unsigned int EB_No_Buf = 0;
#endif
/*============================================================
MEMBER FUNCTION:
    CTBridge::RxDriverPacketCallBack

DESCRIPTION:
    接收网卡上送的报文

ARGUMENTS:
    char* : data to be sent.
    UINT16: data length
    char* : buffer ptr.

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
#ifndef WBBU_CODE
void CTBridge::RxDriverPacketCallBack(char *pRxData, UINT16 usDataLength, char *pRxBuf)
{
    usDataLength -= 6;/*a driver error.*/
    static CTBridge *pBridgeInstance = CTBridge::GetInstance();
#ifdef __WIN32_SIM__
    CComMessage *pComMsg = new( CTBridge::GetInstance(), 0 ) CComMessage;
#else
    CComMessage *pComMsg = pBridgeInstance->GetComMessage();
#endif
    if( NULL == pComMsg )
        {
        //释放RDR.
        //DATA_assert( 0 );
        logMsg("\r\nEB Message pool is used up", 0, 0, 0, 0, 0, 0);
        ::Drv_Reclaim( pRxBuf );
        return;
        }

    pComMsg->SetBuffer( pRxBuf, usDataLength + M_DEFAULT_RESERVED );
    pComMsg->SetDataPtr( pRxData );
    pComMsg->SetDataLength( usDataLength );
    pComMsg->SetTimeStamp( 0 );
    pComMsg->SetFlag( M_CREATOR_DRV );

    if( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        //DATA_assert( 0 );
        logMsg("\r\nDriver post packets to EB failed, EB pool usage[%d/%d].", g_ulOccupiedCounter, M_MAX_LIST_SIZE, 0, 0, 0, 0);
        //pComMsg->Destroy();
        pBridgeInstance->DeallocateComMessage( pComMsg );
        }

    return;
}
#else
void CTBridge::RxDriverPacketCallBack(void *pmblk,char *pRxData, UINT16 usDataLength, char *pRxBuf)
{
  //  usDataLength += 4;/*a driver error.*/
    static CTBridge *pBridgeInstance = NULL;
    pBridgeInstance = CTBridge::GetInstance();
    static unsigned int count22= 0;
	if(L2_BOOT_FLAG==0)
	{
	      ::Drv_Reclaim( pmblk );
		 RX_Rls++;
		  return;
	}
#ifdef __WIN32_SIM__
    CComMessage *pComMsg = new( CTBridge::GetInstance(), 0 ) CComMessage;
#else
    CComMessage *pComMsg = pBridgeInstance->GetComMessage();
#endif
    if( NULL == pComMsg )
        {
        //释放RDR.
        //DATA_assert( 0 );
        count22++;
        EB_No_Buf++;
        if(count22%1000 ==0)
        {
        i(0);
        logMsg("\r\nEB Message pool is used up", 0, 0, 0, 0, 0, 0);
        }
       
        ::Drv_Reclaim( pmblk );
        if( EB_No_Buf==2000)
        {
            //此处复位基站
        }
        return;
        }
    EB_No_Buf=0;
   pComMsg->SetDstTid( M_TID_EB ); 
   pComMsg->SetSrcTid( M_TID_EB ); 
  pComMsg->SetMessageId( MSGID_TRAFFIC_EGRESS );
   pComMsg->SetMblk( pmblk);
    pComMsg->SetBuffer( pRxBuf, usDataLength + M_DEFAULT_RESERVED );
    pComMsg->SetDataPtr( pRxData );
    pComMsg->SetDataLength( usDataLength );
    pComMsg->SetTimeStamp( 0 );
    pComMsg->SetFlag( M_CREATOR_DRV );
    Rxcount1++;
    if( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        //DATA_assert( 0 );
  //      logMsg("\r\nDriver post packets to EB failed, EB pool usage[%d/%d].", g_ulOccupiedCounter, M_MAX_LIST_SIZE, 0, 0, 0, 0);
        pComMsg->Destroy();
          ::Drv_Reclaim( pmblk );
      //  pBridgeInstance->DeallocateComMessage( pComMsg );
        }
        else
        	{
               RxCount++;
        	}
    return;
}

#endif
/*============================================================
MEMBER FUNCTION:
    CTBridge::EBFreeMsgCallBack

DESCRIPTION:
    Driver释放ComMessage的回调函数

ARGUMENTS:
    UINT32 :ComMessage ptr.

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::EBFreeMsgCallBack (UINT32 param)
{
    ((CComMessage*)param)->Destroy();
#ifdef WBBU_CODE
    L2_L3_Rels++;
#endif
    return;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::CheckVLAN

DESCRIPTION:
    VLAN是否改变，保持转发表和CPE配置的VLAN ID一致

ARGUMENTS:
    const UINT16 :CPE配置的VLAN ID

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::CheckVLAN(CMessage &msg)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "Bridge check if VLAN changed." );

    CFTCheckVLAN msgFTCheckVLAN( msg );
    UINT32 ulEID    = msgFTCheckVLAN.GetEID();
    UINT16 usGId    = msgFTCheckVLAN.GetVlanID();

////转发给SNOOP, snoop中用户CCB的group id也需要修改
    msgFTCheckVLAN.SetDstTid( M_TID_SNOOP );
    if (false == msgFTCheckVLAN.Post())
        {
        DATA_log( NULL, LOG_WARN, LOGNO( EB, EC_EB_SYS_FAIL ), "WARNING!!group configuration changes. task tSNOOP doesn't finish the group modification." );
////////return;
        }

    FTEntry *pFT;
    map<CMac, UINT16>::iterator it = m_FTBptree.begin();
    UINT8 ucFlowCtrl = 0;
    while ( m_FTBptree.end() != it )
        {
        pFT = GetFTEntryByIdx( it->second );
        if ( pFT->ulEid != ulEID )
            {
            ++it;
            continue;
            }

        pFT->usGroupId = usGId;

        //next.
        ++it;
        if ( EB_FLOWCTRL_CNT * 6 == ++ucFlowCtrl )
            {
#ifdef __WIN32_SIM__
//Win32:
            ::Sleep( 100 );//释放CPU
#else
//VxWorks:
            ::taskDelay( 1 );
#endif
            ucFlowCtrl = 0;
            }
        }

    return ;
}


bool CTBridge::CheckPeerBtsAddr(FTEntry *pFT )
{
    if (pFT->ulPeerBtsAddr==0 && pFT->usPeerBtsPort==0)
    {
        BtsAddr tmpBtsAddr;
        bool blResult=CSOCKET::GetInstance()->GetBtsPubAddrByData(pFT->ulPeerBtsID,&tmpBtsAddr);
        if(blResult)
        {
            pFT->ulPeerBtsAddr = tmpBtsAddr.IP;
            pFT->usPeerBtsPort = tmpBtsAddr.Port;
        }
        else //无法确定目的IP&Port,数据包丢失;
        {
            if (0 != pFT->ulPeerBtsID)
            LOG2( LOG_DEBUG, LOGNO( EB, EC_EB_SOCKET_ERR ), "\r\nWARNING!! Query public IP & Port for btsID[%x] eid[%08x]failed.\r\n", pFT->ulPeerBtsID ,pFT->ulEid);
            return false; 
        }
    }
return true;
}


/*============================================================
MEMBER FUNCTION:
    ModifyPacketWithVlanID

DESCRIPTION:
    对于广播报文,用于在Payload前增加2字节的空间，用于保存Vlanid
    CPE用于过滤掉其他vlan id的广播报

ARGUMENTS:
    vlanid

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ModifyPacketWithVlanID(CComMessage *pComMsg)
{
    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "Bridge modify payload with vlanid" );
    //添加VLAN tag.
    UINT8 *pDataPtr = (UINT8*)( pComMsg->GetDataPtr() );
    UINT8 *p = pDataPtr - 2;
    UINT16 *pVlan = (UINT16*)(p);
    *pVlan= htons( pComMsg->GetVlanID());
    pComMsg->SetDataPtr( p );
    pComMsg->SetDataLength( pComMsg->GetDataLength() + 2 );
    //消息其他字段不用修改
    return ;
}



/*============================================================
MEMBER FUNCTION:
    DATA_log

DESCRIPTION:
    EB任务的日志输出函数
    EB任务输出信息太多,所以封装日志函数,可以根据需要打印日志
    分为:
    1)根据EID打印日志
    2)根据MAC地址打印日志
    3)根据数据转发方向(上行/下行/隧道)打印日志

ARGUMENTS:

RETURN VALUE:
    void
    如果pComMsg＝NULL时，DATA_log　= LOG;

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::DATA_log(const CComMessage *pComMsg, LOGLEVEL level, UINT32 errcode, const char text[], ...)
{
    bool bDebugOut = false;
    if ((true == gEBDbgInfo.bEbDebugON) && (NULL != pComMsg))
        {
        if ((0 != gEBDbgInfo.DebugEID) && (gEBDbgInfo.DebugEID == pComMsg->GetEID()))
            {
            //是否根据EID打印调试信息
            bDebugOut |= true;
            }

        UINT16 usMessageID = pComMsg->GetMessageId();
        UINT8  ucDebugTrafficDir = 0;
        //是否根据转发方向打印调试信息:上行(1),下行(2),隧道(4)
        switch(usMessageID)
            {
            case MSGID_HIGH_PRIORITY_TRAFFIC:
            case MSGID_LOW_PRIORITY_TRAFFIC:
            case MSGID_REALTIME_TRAFFIC:
                    // Ingress traffic debug.
                    ucDebugTrafficDir = 0x1;
                break;

            case MSGID_TRAFFIC_EGRESS:
                    // Egress traffic debug.
                    ucDebugTrafficDir = 0x2;
                break;

            case MSGID_TRAFFIC_ETHERIP:
                    // Tunnel traffic debug
                    ucDebugTrafficDir = 0x4;
                break;
            }
        if (ucDebugTrafficDir & gEBDbgInfo.DebugTrafficDir)
            {
            bDebugOut |= true;
            }

        if (true == gEBDbgInfo.bDebugMAC)
            {
            CMac mac(gEBDbgInfo.MacAddress);
            EtherHdr *pEther = (EtherHdr*)(pComMsg->GetDataPtr());
            CMac srcMac(pEther->aucSrcMAC);
            CMac dstMac(pEther->aucDstMAC);
            if ((mac == srcMac) || (mac == dstMac))
                {
                bDebugOut |= true;
                }
            }
        }

    if ((NULL == pComMsg) || (true == bDebugOut) || (false == gEBDbgInfo.bEbDebugON))
        {
        va_list vaList;
        char fmt[MAX_STRING_LEN];
        va_start(vaList, text);
        ::vsprintf((char*)fmt, text, vaList);
        va_end(vaList);

        LOG(level, errcode, fmt);
        }
}


extern "C" int getMacFromSTD(char*);
extern "C" int promptParamNum(char *,int *,BOOL);
/*============================================================
MEMBER FUNCTION:
    ebDebug

DESCRIPTION:
    打开/关闭调试开关

ARGUMENTS:

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
extern UINT32 aieb;
extern UINT32 debug_eid;//wangwenhua add 20090210
extern "C" UINT32 llcri;
extern "C" UINT32 lldbg3;
extern "C" bool setDbgLvl(UINT32 ai, unsigned int  consoleLvl);
void ebDebug()
{
    int n = 0;
    int i = 0;
    int tmpValue;
    stEBDebugInfo stDbgInfo;
    memcpy(&stDbgInfo, &gEBDbgInfo, sizeof(stDbgInfo));
    printf ("\n'.' = clear field;  '-' = go to previous field;  ^D = quit\n\n");

    /* prompt the user for each item;
     *   i:  0 = same field, 1 = next field, -1 = previous field,
     *     -98 = error, -99 = quit
     */

    while(true)
    {
        switch ( n )
        {
            case 0:
                i = promptParamNum ("Debug Enable(0-disable/1-enable):", (int*)&(stDbgInfo.bEbDebugON), FALSE); 
                if (false == stDbgInfo.bEbDebugON)
                    {
                    memset(&stDbgInfo, 0, sizeof(stDbgInfo));
                    stDbgInfo.dbgLvl = llcri;
                    i = -99; break;
                    }
                break;
            case 1:
                printf("Debug by EID: all logs FROM or TO this EID may be printed out.");
                i = promptParamNum ("Enter an EID(0-disable):", (int*)&(stDbgInfo.DebugEID), FALSE); 
		  debug_eid = stDbgInfo.DebugEID;//wangwenhua add 20090210
                break;
            case 2:  
                printf("All logs FROM or TO this MAC address may be printed out.");
                tmpValue = (stDbgInfo.bDebugMAC)? 1:0;
                i = promptParamNum ("Debug by MAC address(0-disable/1-enable)", &tmpValue, FALSE); 
                if (tmpValue != 0 && tmpValue != 1)
                {
                    printf(" invalude input\n");
                    i = 0;
                }
                else if (tmpValue == 0)
                {
                    ++i;//跳过输入MAC address.
                    stDbgInfo.bDebugMAC = false;
                }
                else
                {
                    stDbgInfo.bDebugMAC = true;
                }
                break;
            case 3:
                printf("Please Enter a MAC Address[format:xx-xx-xx-xx-xx-xx]:");
                getMacFromSTD( (char*)(stDbgInfo.MacAddress) );
                i = 1;//继续下一个
                break;
            case 4:  
                tmpValue = (stDbgInfo.DebugTrafficDir & 0x1)? 1:0;
                i = promptParamNum ("Uplink traffic Debug(0-disable/1-enable)", &tmpValue, FALSE); 
                if (0 == tmpValue)
                    {
                    stDbgInfo.DebugTrafficDir &= ~(UINT8)1;
                    }
                else if (1 == tmpValue)
                    {
                    stDbgInfo.DebugTrafficDir |= tmpValue;
                    }
                else
                    {
                    printf(" invalude input\n");
                    i = 0;
                    }
                break;

            case 5: 
                tmpValue = (stDbgInfo.DebugTrafficDir & 0x2)? 1:0;
                i = promptParamNum ("Downlink traffic Debug(0-disable/1-enable)", &tmpValue, FALSE); 
                if (0 == tmpValue)
                    {
                    stDbgInfo.DebugTrafficDir &= ~(UINT8)(0x2);
                    }
                else if (1 == tmpValue)
                    {
                    stDbgInfo.DebugTrafficDir |= tmpValue<<1;
                    }
                else
                    {
                    printf(" invalude input\n");
                    i = 0;
                    }
                break;

            case 6:  
                tmpValue = (stDbgInfo.DebugTrafficDir & 0x4)? 1:0;
                i = promptParamNum ("Tunnel traffic Debug(0-disable/1-enable)", &tmpValue, FALSE); 
                if (0 == tmpValue)
                    {
                    stDbgInfo.DebugTrafficDir &= ~(UINT8)(0x4);
                    }
                else if (1 == tmpValue)
                    {
                    stDbgInfo.DebugTrafficDir |= tmpValue<<2;
                    }
                else
                    {
                    printf(" invalude input\n");
                    i = 0;
                    }
                break;
            case 7:
                tmpValue = stDbgInfo.dbgLvl;
                i = promptParamNum ("set EB debug level(eg. lldbg3)", &tmpValue, FALSE); 
                if ((llcri <= tmpValue) && (tmpValue <= lldbg3))
                    {
                    stDbgInfo.dbgLvl = tmpValue;
                    }
                else
                    {
                    printf(" invalude input, must between llcri[%d]..lldbg3[%d]\n",llcri,lldbg3);
                    i = 0;
                    }
                break;

            default: i = -99; break;
        }

        /* check for QUIT */

        if ( i == -99 )
        {
            printf ("\n");
            break;
        }

        /* move to new field */

        if ( i != -98 ) n += i;
        if ( i<0 )
        {
            i=0;
        }
    }
    memcpy(&gEBDbgInfo, &stDbgInfo, sizeof(stDbgInfo));
    setDbgLvl(aieb, stDbgInfo.dbgLvl);
}

void ebDebugShow()
{
    printf("\n");
    printf("Debug switch          (0-OFF, 1-ON) :%d\n", gEBDbgInfo.bEbDebugON); 
    printf("Debug MAC address     (0-OFF, 1-ON) :%d\n", gEBDbgInfo.bDebugMAC); 
    if (true == gEBDbgInfo.bDebugMAC)
        {
        printf("  MAC address:%.2x-%.2x-%.2x-%.2x-%.2x-%.2x\n", 
            gEBDbgInfo.MacAddress[0],gEBDbgInfo.MacAddress[1],gEBDbgInfo.MacAddress[2],
            gEBDbgInfo.MacAddress[3],gEBDbgInfo.MacAddress[4],gEBDbgInfo.MacAddress[5]); 
        }
    printf("Debug EID             (0-OFF)       :0x%X\n", gEBDbgInfo.DebugEID); 
    printf("Uplink traffic Debug  (0-OFF, 1-ON) :%d\n",   gEBDbgInfo.DebugTrafficDir&0x1); 
    printf("Downlink traffic Debug(0-OFF, 1-ON) :%d\n",  (gEBDbgInfo.DebugTrafficDir&0x2)>>1);
    printf("Tunnel traffic Debug  (0-OFF, 1-ON) :%d\n",  (gEBDbgInfo.DebugTrafficDir&0x4)>>2);
    printf("EB debug level(llcri:%d~~lldbg3:%d) :%d\n", llcri,lldbg3,gEBDbgInfo.dbgLvl); 
    return;
}



/*============================================================
MEMBER FUNCTION:
    EBShow

DESCRIPTION:
    用于Tornado Shell上调用执行

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
extern "C" int getSTDNum(int*);
void EBShow(UINT8 ucType)
{
    CTBridge *taskEB = CTBridge::GetInstance();
 //   int bytesRead = 0;
    switch ( ucType )
        {
        case 0:
            printf("\r\n EBShow(1): Show basic information");
            printf("\r\n EBShow(2): Show forwarding table");
            printf("\r\n EBShow(3): Show performance information");
            printf("\r\n EBShow(4): Show MAC filtering table");
            printf("\r\n EBShow(5): Show QoS configuration");
            printf("\r\n");
            break;
        case 1:
            printf("\r\nShow EB basic information");
            taskEB->showStatus();
            break;
        case 2:
            {
            printf("\r\nShow forwarding table information");
            printf("\r\nPlease Enter EID(0:all):");
            UINT32 ulEid = 0;
            getSTDNum((int*)&ulEid);
            printf( "EID = %d = 0x%x", ulEid, ulEid );
            taskEB->showFT( ulEid );
            }
            break;
        case 3:
            {
            printf("\r\nShow performance information");
            printf("\r\nPlease Enter a MAC Address[format:xx-xx-xx-xx-xx-xx]");
            UINT8 strMAC[ 7 ] = {0};
            getMacFromSTD( (char*)strMAC );
            taskEB->showPerf( strMAC );
            }
            break;
        case 4:
            {
                printf("\r\nShow MAC filtering table");
                taskEB->showMFT();
            }
        break;
        case 5:
            {
                printf("\r\nShow QoS Configuration");
                taskEB->showQoS();
            }
            break;
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    groupShow

DESCRIPTION:
    打印group对应的vlan

ARGUMENTS:
    group id

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void groupShow(UINT16 groupId)
{
    if ((groupId >= 4096)||(0 == groupId))
        {
        printf("\r\nPlease follow a valid group ID[1..4095]. 0 means belonging to no group.");
        printf("\r\neg. groupShow 25\r\n");
        return;
        }
    CTBridge *taskEB = CTBridge::GetInstance();
    printf("\r\nGroup Id:%d,\tVlan Id:%d.\r\n\r\n", groupId, taskEB->getVlanID(groupId));
}

/*============================================================
MEMBER FUNCTION:
CTBridge::InitFreeMFTList

DESCRIPTION:
初始化空闲转发表表项链表m_listFreeMFT

ARGUMENTS:
NULL

RETURN VALUE:
void

SIDE EFFECTS:
none
==============================================================*/
void CTBridge::InitFreeMFTList()
{
    m_listFreeMFT.clear();
    for ( UINT16 index = 0; index < M_MAX_MACFILTER_TABLE_NUM; ++index )
    {
        m_listFreeMFT.push_back( index );
    }
}

/*============================================================
MEMBER FUNCTION:
CTBridge::InsertFreeMFTList

DESCRIPTION:
插入空闲转发表表项下标到链表m_listFreeMFT尾部

ARGUMENTS:
index:表项下标

RETURN VALUE:
void

SIDE EFFECTS:
none
==============================================================*/
void CTBridge::InsertFreeMFTList(UINT16 index )
{
    if( index < M_MAX_MACFILTER_TABLE_NUM )
    {
        m_listFreeMFT.push_front( index );
    }
    else
    {
        DATA_assert( 0 );
    }
}

/*============================================================
MEMBER FUNCTION:
CTBridge::GetFreeMFTEntryIdxFromList

DESCRIPTION:
从空闲链表m_listFreeMFT取空闲转发表表项下标;(从链表头部取)

ARGUMENTS:
NULL

RETURN VALUE:
index:表项下标

SIDE EFFECTS:
none
==============================================================*/
UINT16 CTBridge::GetFreeMFTEntryIdxFromList()
{
    if ( true == m_listFreeMFT.empty() )
    {
        return M_DATA_INDEX_ERR;
    }

    UINT16 index = *m_listFreeMFT.begin();
    m_listFreeMFT.pop_front();

    if ( M_MAX_MACFILTER_TABLE_NUM <= index )
    {
        //下标错误
        return M_DATA_INDEX_ERR;
    }

    return index;
}

/*============================================================
MEMBER FUNCTION:
CTBridge::MFTBPtreeAdd

DESCRIPTION:
MACFilter表索引树插入节点；MACFilter表索引树以Mac地址作键值，把该表
表项下标插入到索引树

ARGUMENTS:
Mac:Mac地址
index:表项下标

RETURN VALUE:
bool:插入成功/失败

SIDE EFFECTS:
如果存在重复项，将会返回失败，所以必须在BPtreeAdd之前保证
没有重复。即首先 MFTBPtreeFind 一下
==============================================================*/
bool CTBridge::MFTBPtreeAdd(CMac &Mac, UINT16 index)
{
    if ( ( true == Mac.IsZero() ) || ( true == Mac.IsBroadCast() ) )
    {
        UINT8 strMac[ M_MACADDR_STRLEN ];
        LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), 
            "CTBridge::MFTBPtreeAdd() try to add an entry with Mac:%s ", (int)Mac.str( strMac ) );
        return false;
    }
    pair<map<CMac, UINT16>::iterator, bool> stPair;

    stPair = m_MFTBptree.insert( ValType( Mac, index ) );
    /*
    *必须保证不存在重复项，否则将会返回失败
    */
    return stPair.second;
}

/*============================================================
MEMBER FUNCTION:
CTBridge::MFTBPtreeDel

DESCRIPTION:
MACFilter表索引树删除Mac对应的节点；

ARGUMENTS:
Mac:Mac地址

RETURN VALUE:
bool:删除成功/失败

SIDE EFFECTS:
none
==============================================================*/
bool CTBridge::MFTBPtreeDel(CMac &Mac)
{
    map<CMac, UINT16>::iterator it;

    if ( ( it = m_MFTBptree.find( Mac ) ) != m_MFTBptree.end() )
    {
        //find, and erase;
        m_MFTBptree.erase( it );
    }
    //not find
    return true;
}


/*============================================================
MEMBER FUNCTION:
CTBridge::MFTBPtreeFind

DESCRIPTION:
从MFTBPtree搜索Mac地址对应的MACFilter表表项下标

ARGUMENTS:
Mac:Mac地址

RETURN VALUE:
index:表项下标

SIDE EFFECTS:
none
==============================================================*/
UINT16 CTBridge::MFTBPtreeFind(CMac &Mac)
{
    map<CMac, UINT16>::iterator it = m_MFTBptree.find( Mac );

    if ( it != m_MFTBptree.end() )
    {
        //返回Index.
        return it->second;
    }
    return M_DATA_INDEX_ERR;
}

/*============================================================
MEMBER FUNCTION:
CTBridge::MFTAddEntry

DESCRIPTION:
增加MAC Filter Table表项

ARGUMENTS:
CMessage: 消息

RETURN VALUE:
void

SIDE EFFECTS:
none
==============================================================*/
void CTBridge::MFTAddEntry(const CMFTAddEntry &msgMFTAddEntry)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge MFTAddEntry()" );
    CMac mac( msgMFTAddEntry.GetMac() );
    if ( true == mac.IsBroadCast() )
    {
        LOG( LOG_DEBUG1, LOGNO( EB, EC_EB_PARAMETER ), "Err! Trying to add a broadcast MAC filter forwarding entry." );
        return;
    }

    MACFilterEntry *pMFT = GetMFTEntryByIdx( MFTBPtreeFind( mac ) );
    if ( NULL == pMFT )
    {
        //新建转发表表项
        UINT16 index  = GetFreeMFTEntryIdxFromList();
        pMFT = GetMFTEntryByIdx( index );
        if ( NULL == pMFT )
        {
            //用户满,删除消息；
            LOG( LOG_WARN, LOGNO( EB, EC_EB_NO_ENTRY ), "WARNING! no more free MAC filter entry when trying to add a new entry." );
            return;
        }
        MFTBPtreeAdd( mac,index );
    }

    pMFT->IsOccupied  = true;
    memcpy( pMFT->MAC, msgMFTAddEntry.GetMac(), M_MAC_ADDRLEN );
#if 0
    pMFT->TYPE        = msgMFTAddEntry.GetType();
#endif

    pMFT->Elapsed     = 0;
#if 0   
    if (EB_MACFILTER_ROUTER == pMFT->TYPE)
    pMFT->TTL     = M_CLEANUP_MACFILTER_ROUTER;
    else
    pMFT->TTL     = M_CLEANUP_MACFILTER_SERVER;
#else
    pMFT->TTL     = M_CLEANUP_MACFILTER_ROUTER;
#endif
    return;
}


/*============================================================
MEMBER FUNCTION:
CTBridge::MFTUpdateEntry

DESCRIPTION:
修改MAC FILTER TABLE表项

ARGUMENTS:
CMessage: 消息

RETURN VALUE:
void

SIDE EFFECTS:
none
==============================================================*/
void CTBridge::MFTUpdateEntry(const CMFTUpdateEntry &msgMFTUpdateEntry)
{
    return MFTAddEntry(msgMFTUpdateEntry);
}


/*============================================================
MEMBER FUNCTION:
CTBridge::MFTDelEntry

DESCRIPTION:
Mac地址对应的Mac Filter表表项TTL值=0;等待cleanup任务删除

ARGUMENTS:
CMessage: 消息

RETURN VALUE:
void

SIDE EFFECTS:
none
==============================================================*/
void CTBridge::MFTDelEntry(const CMFTDelEntry &msgMFTDelEntry)
{
    LOG( LOG_DEBUG, LOGNO( EB, EC_EB_NORMAL ), "->Bridge MFTDelEntry()" );

    CMac    DstMac( msgMFTDelEntry.GetMac() );
    UINT16  index = MFTBPtreeFind(DstMac);
    MACFilterEntry *pMFTEntry = GetMFTEntryByIdx( index );
    if ( NULL == pMFTEntry )
    {
        return;
    }

    MFTBPtreeDel( DstMac );
    memset( pMFTEntry, 0, sizeof( MACFilterEntry ) );
    InsertFreeMFTList( index );

    return;
}



/*============================================================
MEMBER FUNCTION:
CTBridge::MFTEntryExpire

DESCRIPTION:
首先删除超时的MAC,如果是router，还要发送arp重新获取MAC

ARGUMENTS:
CMessage: 消息

RETURN VALUE:
bool

SIDE EFFECTS:
none
==============================================================*/
bool CTBridge::MFTEntryExpire(const CMFTDelEntry &msgMFTDelEntry)
{
    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge MFTEntryExpire()" );
#if 0
    UINT8  type = msgMFTDelEntry.GetType();
#endif
    CMac   DstMac(msgMFTDelEntry.GetMac());
    UINT16 index = MFTBPtreeFind(DstMac);
    MACFilterEntry *pMFTEntry = GetMFTEntryByIdx( index );
    if (NULL == pMFTEntry)
    {
        return false;
    }
#if 0   
    if (EB_MACFILTER_ROUTER == type) 
    {
        #ifndef M_TGT_WANIF
        ResolveRouterMAC();
        #endif
    }
    else
#endif		
    {
        memset(pMFTEntry, 0, sizeof(MACFilterEntry));
        MFTBPtreeDel(DstMac);
        InsertFreeMFTList(index);
    }
#if 0
#if 0 //moved to do_monitor() liuweidong 2011.5.13
    #ifndef M_TGT_WANIF
    ResolveRouterMAC();
    #else
   if(Wanif_Switch!=0x5a5a)
   	{
   	   ResolveRouterMAC();
   	}
   #endif
#endif
#endif  
    return true;
}

void opencdrdebug()
{
	CdrTestFlag = 1;
}
void closecdrdebug()
{
	CdrTestFlag = 0;
}
bool CTBridge::sendMsg2CB3000(char* pData,UINT16 len)
{
       CComMessage      *pComMsg     = new ( this, len )CComMessage;
        if ( NULL == pComMsg )
        {
        //delete pComMsgNode;
        return false;
        }
	 if(CdrTestFlag)
	 {
	 	if(len==18)
	 	{
	 		UINT32 eid =0;
			memcpy((char*)&eid,(pData+4),4);
	 		LOG3( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "eid :0x%x,sendMsg2CB3000 msgtype:%x,len:%d.",eid,pData[1],len );
	 	}
		else
			LOG2( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "sendMsg2CB3000 msgtype:%x,len:%d.",pData[1],len );
	 }
	 memcpy((char*)pComMsg->GetDataPtr(),pData,len);
        pComMsg->SetDstTid( M_TID_EMSAGENTTX ); 
        pComMsg->SetSrcTid( M_TID_EB ); 
        pComMsg->SetMessageId( MSG_BTS_TO_CB3000 );
	 if ( ! CComEntity::PostEntityMessage(pComMsg))
	 {
		pComMsg->Destroy();
	 }
	 
    return true;
}

/*============================================================
MEMBER FUNCTION:
CTBridge::ResolveRouterMAC

DESCRIPTION:
1:get router ip from bsp 
2:using ARPResolve to get router MAC
3:add/update router mac 

ARGUMENTS:
Mac:Mac地址

RETURN VALUE:
none

SIDE EFFECTS:
none
==============================================================*/

#include "config.h"
#ifndef WBBU_CODE
extern "C" int arpResolve(char*,char*,int,int);
#endif
extern "C" int arpDelete(char*);
const UINT8 badGWMac[M_MAC_ADDRLEN] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
void CTBridge::InitRouterMAC()
{
  #ifndef WBBU_CODE
    BOOT_PARAMS       bootParams;
    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
    if (bootParams.gad[0] != EOS)
    {
        DATA_log(NULL, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "Gateway inet address: %s", bootParams.gad );
    }

    struct in_addr router;      /* Internet Address */
    UINT8 tmpMac[M_MAC_ADDRLEN];
    STATUS status = arpResolve(/*gateway*/bootParams.gad,(char*)tmpMac,3,10);
    if(status == ERROR)
    {
        DATA_log(NULL, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "arpResolve gateway failed"); 
        memcpy(tmpMac, badGWMac, sizeof(badGWMac));
        //return;
    }
    DATA_log(NULL, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "Gateway MAC address: %.2x-%.2x-%.2x-%.2x-%.2x-%.2x", tmpMac[0],tmpMac[1],tmpMac[2],tmpMac[3],tmpMac[4],tmpMac[5]);

    CMFTAddEntry msg;
    if(true == msg.CreateMessage(*(CTBridge::GetInstance())))
    {
        msg.SetMac(tmpMac);
#if 0       
        msg.SetType(EB_MACFILTER_ROUTER);
#endif
        MFTAddEntry(msg);
        msg.DeleteMessage();
    }
#endif
    return;
}
 void sendCdrForReboot()
{
	if(uiCdrSwitch==1)
	{
	    CTBridge *taskEB = CTBridge::GetInstance();
	    taskEB->send_cdr_for_reboot();
	}

     return;
}
extern "C" bool bspGetIsRebootBtsWanIfDiscEnabled();
#ifndef WBBU_CODE
extern "C" STATUS exitTelnetSession();

#endif
void CTBridge::ResolveRouterMAC()
{
    DATA_log(NULL, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "try to arpResolve gateway's MAC address" );
    BOOT_PARAMS bootParams;
    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
    if (bootParams.gad[0] != EOS)
    {
        DATA_log(NULL, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "Gateway inet address: %s", bootParams.gad );
    }

   // arpDelete(bootParams.gad);//wangwenhua mark 20080911
   // struct in_addr router;
    UINT8 tmpMac[M_MAC_ADDRLEN];
    #ifndef WBBU_CODE
    STATUS status = arpResolve(bootParams.gad,(char*)tmpMac,3,10);
    static UINT8 errorCount = 0;
    if(status == ERROR)
    { 
        DATA_log(NULL, LOG_SEVERE, LOGNO( EB, EC_EB_NORMAL ), "arpResolve gateway[%s] failed, errCount=%d", bootParams.gad, errorCount); 
        if (bspGetIsRebootBtsWanIfDiscEnabled())
        {
            if (++errorCount > 200)  // when failed, the arpResolve is called every 2 seconds
            {
                DATA_log(NULL, LOG_SEVERE, LOGNO( EB, EC_EB_NORMAL ), "Link between BTS and gateway is down, rebootBTS."); 
                bspSetBtsResetReason(RESET_REASON_ARPNOT_GTAEWAY/*RESET_REASON_SW_ABNORMAL*/);
		  sendCdrForReboot();
                taskDelay(50);
                rebootBTS(BOOT_CLEAR);
            }
        }
        else
        {
            errorCount = 0;
        }
        //arp网关失败，telnet注销，否则网络恢复后，telnet无法登陆
     //   exitTelnetSession();//wangwenhua mark 20081028
        return;
    }
    errorCount = 0;
    DATA_log(NULL, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "Gateway MAC address: %.2x-%.2x-%.2x-%.2x-%.2x-%.2x", tmpMac[0],tmpMac[1],tmpMac[2],tmpMac[3],tmpMac[4],tmpMac[5]);
    #else
      
    STATUS status = ping(bootParams.gad,3,0);
    
    static UINT8 errorCount = 0;
    if(status == ERROR)
    { 
        DATA_log(NULL, LOG_SEVERE, LOGNO( EB, EC_EB_NORMAL ), "arpResolve gateway[%s] failed, errCount=%d", bootParams.gad, errorCount); 
        if (bspGetIsRebootBtsWanIfDiscEnabled())
        {
            if (++errorCount > 200)  // when failed, the arpResolve is called every 2 seconds
            {
                DATA_log(NULL, LOG_SEVERE, LOGNO( EB, EC_EB_NORMAL ), "Link between BTS and gateway is down, rebootBTS."); 
                bspSetBtsResetReason(RESET_REASON_ARPNOT_GTAEWAY/*RESET_REASON_SW_ABNORMAL*/);
		   sendCdrForReboot();
                taskDelay(50);
                rebootBTS(BOOT_CLEAR);
            }
        }
        else
        {
            errorCount = 0;
        }
        //arp网关失败，telnet注销，否则网络恢复后，telnet无法登陆
     //   exitTelnetSession();//wangwenhua mark 20081028
        return;
    }
    errorCount = 0;
    DATA_log(NULL, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "Gateway MAC address: %.2x-%.2x-%.2x-%.2x-%.2x-%.2x", tmpMac[0],tmpMac[1],tmpMac[2],tmpMac[3],tmpMac[4],tmpMac[5]);
    #endif
    CMFTAddEntry msg;
    if(true == msg.CreateMessage(*(CTBridge::GetInstance())))
    {
        msg.SetMac(tmpMac);
#if 0       
        msg.SetType(EB_MACFILTER_ROUTER);
#endif
        msg.SetDstTid( M_TID_EB );
        //MFTAddEntry(msg);
        //msg.DeleteMessage();
                    //发送
        if ( false == msg.Post() )
                    {
                        LOG( LOG_DEBUG1, LOGNO( EB, EC_EB_SYS_FAIL ), "task arp gateway update mft Send Message failed." );
                        msg.DeleteMessage();
                    }
    }


    CFTDelEntry msgFTDelEntry;
    if(true == msgFTDelEntry.CreateMessage(*(CTBridge::GetInstance())))
    {
        msgFTDelEntry.SetMac(tmpMac);
        msgFTDelEntry.SetDstTid( M_TID_EB );
        //MFTAddEntry(msg);
        //msg.DeleteMessage();
                    //发送
        if ( false == msgFTDelEntry.Post() )
                    {
                        LOG( LOG_DEBUG1, LOGNO( EB, EC_EB_SYS_FAIL ), "task arp gateway update mft Send Message failed." );
                        msgFTDelEntry.DeleteMessage();
                    }       
     }   
    CMFTDelEntry msgMFTDelEntry;
    if(true == msgMFTDelEntry.CreateMessage(*(CTBridge::GetInstance())))
    {
        //网关arpResolve成功，把假的GW MAC地址删除
        msgMFTDelEntry.SetMac(badGWMac);
        //MFTDelEntry(msgMFTDelEntry);
        //msgMFTDelEntry.DeleteMessage();
        msgMFTDelEntry.SetDstTid( M_TID_EB );
        //MFTAddEntry(msg);
        //msg.DeleteMessage();
                    //发送
        if ( false == msgMFTDelEntry.Post() )
                    {
                        LOG( LOG_DEBUG1, LOGNO( EB, EC_EB_SYS_FAIL ), "task arp gateway update mft Send Message failed." );
                        msgMFTDelEntry.DeleteMessage();
                    }          
    }

    return;
}

//////////////////////////////////////////////////////////////////////////
//以下是测试代码，用来模拟DHCP和PPPOE PADI数据包,还有一种错误数据包,暂且
//注释掉,使用时打开
//////////////////////////////////////////////////////////////////////////
#if 0
extern "C" void resolveRouterMAC()
{
    CTBridge *taskEB = CTBridge::GetInstance();
    taskEB->ResolveRouterMAC();
    return;
}

extern "C" void testPkt(UINT8 type)
{
    CTBridge *taskEB = CTBridge::GetInstance();
    switch ( type )
    {
    case 0:
        printf("\r\n testRecvPkt(1): test dhcp packet");
        printf("\r\n testRecvPkt(2): test pppoe padi packet");
        printf("\r\n testRecvPkt(3): test a packet with illigal mac");
        printf("\r\n");
        break;
    case 1:
        printf("\r\n send a dhcp packet to myself");
        taskEB->sendDHCPPkt();
        break;
    case 2:
        {
            printf("\r\n send a pppoe(padi) packet to myself");
            taskEB->sendPPPOEPkt();
        }
        break;
    case 3:
        {
            printf("\r\n testRecvPkt(3): test a packet with illigal mac");
            taskEB->sendIlligalMAC();
        }
        break;          
    }

    return;
}
typedef struct _tag_DHCP_IPREQ
{
    UINT8       DstMAC[M_MAC_ADDRLEN];
    UINT8       SrcMAC[M_MAC_ADDRLEN]; 
    UINT16      usProto;
    UINT8       ucLenVer;       /* 4位首部长度+4位IP版本号 */
    UINT8       ucTOS;          /* 8位服务类型TOS */
    UINT16      usTotalLen;     /* 16位总长度（字节） */
    UINT16      usId;           /* 16位标识 */
    UINT16      usFragAndFlags; /* 3位标志位 */
    UINT8       ucTTL;          /* 8位生存时间 TTL */
    UINT8       ucProto;        /* 8位协议 (TCP, UDP 或其他) */
    UINT16      usCheckSum;     /* 16位IP首部校验和 */
    UINT32      ulSrcIp;        /* 32位源IP地址 */
    UINT32      ulDstIp;        /* 32位目的IP地址 */

    UINT16      usSrcPort;      /* 源端口 */
    UINT16      usDstPort;      /* 目标端口*/
    UINT16      usLen;          /* 长度 */
    UINT16      ucCheckSum;     /* 校验和 */

    UINT8       ucOpCode;
    UINT8       ucHtype;
    UINT8       ucHlen;
    UINT8       ucHop;
    UINT32      ulXid;
    UINT16      usSec;
    UINT16      usFlag;
    UINT32      ulCiaddr;
    UINT32      ulYiaddr;
    UINT32      ulSiaddr;
    UINT32      ulGiaddr;
    UINT8       aucChaddr[M_DHCP_CLIENT_HADDR_LEN];
    UINT8       aucSname[M_DHCP_SERVER_HOSTNAME_LEN];
    UINT8       aucFile[M_DHCP_BOOT_FILENAME_LEN];
};

typedef struct _tag_PPPOE_PADI
{
    UINT8  DstMAC[M_MAC_ADDRLEN];
    UINT8  SrcMAC[M_MAC_ADDRLEN]; 
    UINT16 Proto;
    UINT8  VerType;
    UINT8  Code;
    UINT16 SessionId;    /*network byte order*/
    UINT16 Length;       /*network byte order*/
};
void CTBridge::sendDHCPPkt()
{
    CComMessage *pComMsg  = new ( this, 1000 )CComMessage;
    if ( NULL == pComMsg )
    {
        return;
    }

    pComMsg->SetDstTid( M_TID_EB ); 
    pComMsg->SetSrcTid( M_TID_EB ); 
    pComMsg->SetMessageId( MSGID_HIGH_PRIORITY_TRAFFIC );
    _tag_DHCP_IPREQ* pDHCP = (_tag_DHCP_IPREQ*)pComMsg->GetDataPtr();
    pDHCP->SrcMAC[0]= 0x20; 
    pDHCP->SrcMAC[1]= 0x0e; 
    pDHCP->SrcMAC[2]= 0xa6; 
    pDHCP->SrcMAC[3]= 0x79; 
    pDHCP->SrcMAC[4]= 0xA3; 
    pDHCP->SrcMAC[5]= 0x4E; 
    pDHCP->DstMAC[0]= 0x22; 
    pDHCP->DstMAC[1]= 0x22; 
    pDHCP->DstMAC[2]= 0x22; 
    pDHCP->DstMAC[3]= 0x22; 
    pDHCP->DstMAC[4]= 0x22; 
    pDHCP->DstMAC[5]= 0x22; 
    pDHCP->usProto = M_ETHER_TYPE_IP;

    pDHCP->ucLenVer = 0x45;       /* 4位首部长度+4位IP版本号 */
    pDHCP->ucTOS = 0;          /* 8位服务类型TOS */
    pDHCP->usTotalLen = 0x015e;     /* 16位总长度（字节） */
    pDHCP->usId = 0x0de0;           /* 16位标识 */
    pDHCP->usFragAndFlags = 0; /* 3位标志位 */
    pDHCP->ucTTL = 0x80;          /* 8位生存时间 TTL */
    pDHCP->ucProto = 0x11;        /* 8位协议 (TCP, UDP 或其他) */
    pDHCP->usCheckSum= 0x2bb0;     /* 16位IP首部校验和 */
    pDHCP->ulSrcIp =0;        /* 32位源IP地址 */
    pDHCP->ulDstIp=0xffffffff;        /* 32位目的IP地址 */

    pDHCP->usSrcPort=0x0044;      /* 源端口 */
    pDHCP->usDstPort=0x0043;      /* 目标端口*/
    pDHCP->usLen=0x014A;          /* 长度 */
    pDHCP->ucCheckSum=0x980b;     /* 校验和 */

    pDHCP->ucOpCode=1;
    pDHCP->ucHtype=1;
    pDHCP->ucHlen=6;

    
    m_ucWorkingMode = WM_NETWORK_AWARE;

    if( false == CComEntity::PostEntityMessage( pComMsg ) )
    {
//test start
printf("\r\npost failed\r\n");
//test stop        
        pComMsg->Destroy();
    }
//test start
printf("\r\npost successful\r\n");
//test stop
    return;
}
void CTBridge::sendPPPOEPkt()
{
//需要屏蔽部分认证原代码
    CComMessage *pComMsg  = new ( this, 1000 )CComMessage;
    if ( NULL == pComMsg )
    {
        return;
    }

    pComMsg->SetDstTid( M_TID_EB ); 
    pComMsg->SetSrcTid( M_TID_EB ); 
    pComMsg->SetMessageId( MSGID_HIGH_PRIORITY_TRAFFIC );
    _tag_PPPOE_PADI* pPPPOE = (_tag_PPPOE_PADI*)pComMsg->GetDataPtr();
    pPPPOE->SrcMAC[0]= 0x20; 
    pPPPOE->SrcMAC[1]= 0x0e; 
    pPPPOE->SrcMAC[2]= 0xa6; 
    pPPPOE->SrcMAC[3]= 0x79; 
    pPPPOE->SrcMAC[4]= 0xA3; 
    pPPPOE->SrcMAC[5]= 0x4E; 
    pPPPOE->DstMAC[0]= 0x11; 
    pPPPOE->DstMAC[1]= 0x11; 
    pPPPOE->DstMAC[2]= 0x11; 
    pPPPOE->DstMAC[3]= 0x11; 
    pPPPOE->DstMAC[4]= 0x11; 
    pPPPOE->DstMAC[5]= 0x11;    
    pPPPOE->Proto = M_ETHER_TYPE_PPPoE_SESSION;
    m_ucWorkingMode = WM_NETWORK_AWARE;
    if( false == CComEntity::PostEntityMessage( pComMsg ) )
    {
//test start
printf("\r\npost failed\r\n");
//test stop
        pComMsg->Destroy();
    }
//test start
printf("\r\npost successful\r\n");
//test stop
    return;
}


void CTBridge::sendIlligalMAC()
{
    CComMessage *pComMsg  = new ( this, 1000 )CComMessage;
    if ( NULL == pComMsg )
    {
        return;
    }

    pComMsg->SetDstTid( M_TID_EB ); 
    pComMsg->SetSrcTid( M_TID_EB ); 
    pComMsg->SetMessageId( MSGID_HIGH_PRIORITY_TRAFFIC );
    _tag_PPPOE_PADI* pPPPOE = (_tag_PPPOE_PADI*)pComMsg->GetDataPtr();
    pPPPOE->SrcMAC[0]= 0x11; 
    pPPPOE->SrcMAC[1]= 0x11; 
    pPPPOE->SrcMAC[2]= 0x11; 
    pPPPOE->SrcMAC[3]= 0x11; 
    pPPPOE->SrcMAC[4]= 0x11; 
    pPPPOE->SrcMAC[5]= 0x11;
    pPPPOE->DstMAC[0]= 0x20; 
    pPPPOE->DstMAC[1]= 0x0e; 
    pPPPOE->DstMAC[2]= 0xa6; 
    pPPPOE->DstMAC[3]= 0x79; 
    pPPPOE->DstMAC[4]= 0xA3; 
    pPPPOE->DstMAC[5]= 0x4E; 

    pPPPOE->Proto = M_ETHER_TYPE_PPPoE_SESSION;
    m_ucWorkingMode = WM_NETWORK_AWARE;
    if( false == CComEntity::PostEntityMessage( pComMsg ) )
    {
//test start
printf("\r\npost failed\r\n");
//test stop
        pComMsg->Destroy();
    }
//test start
printf("\r\npost successful\r\n");
//test stop
    return;
}
#endif

/*
 *过滤VLAN-Bridge 和 SNAP HDLC协议
 */
void CTBridge::InitFilterMAC()
{
	UINT8 strMAC1[M_MAC_ADDRLEN]={0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcd};
	m_filterMAC.Add2Filter(strMAC1);
	UINT8 strMAC2[M_MAC_ADDRLEN]={0x01, 0x00, 0x0c, 0xcd, 0xcd, 0xce};
	m_filterMAC.Add2Filter(strMAC2);
	/*根据喀麦隆问题添加过滤DTP和国际标准的STP*/
	UINT8 strMAC3[M_MAC_ADDRLEN]={0x01, 0x00, 0x0c, 0xcc, 0xcc, 0xcc};
	m_filterMAC.Add2Filter(strMAC3);
	UINT8 strMAC4[M_MAC_ADDRLEN]={0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
	m_filterMAC.Add2Filter(strMAC4);
}

/*
 *CfilterMAC类
 *过滤目的MAC地址，可以考虑放到CPE的ACL
 */

void CfilterMAC::Add2Filter(const UINT8* strMAC)
{
    if (NULL == strMAC)
        return;
    CMac mac(strMAC);
    m_listToBeFilteredMAC.push_back(mac);
}


bool CfilterMAC::find(const CMac &mac) const
{
    list<CMac>::const_iterator it;
    for (it = m_listToBeFilteredMAC.begin(); it != m_listToBeFilteredMAC.end(); ++it)
        {
        if (mac == *it)
            return true;
        }
    return false;
}

bool CfilterMAC::find(const UINT8 *strMAC) const
{
    if (NULL == strMAC)
        return false;
    CMac mac(strMAC);
    return find(mac);
}

void deleteuser()
{
    printf("\r\nDelete a user from BWA system.\r\n");
    printf("\r\nPlease Enter the MAC Address[format:xx-xx-xx-xx-xx-xx]");
    UINT8 strMAC[ 7 ] = {0};
    getMacFromSTD( (char*)strMAC );
    CFTDelEntry msgFTDelEntry;
    if ( false == msgFTDelEntry.CreateMessage(*(CTBridge::GetInstance())))
        {
        return;
        }
    msgFTDelEntry.SetDstTid( M_TID_EB );

    msgFTDelEntry.SetMac( strMAC );
    msgFTDelEntry.SetTunnel(false);
    msgFTDelEntry.setTunnelPeerBtsIP(0);
    msgFTDelEntry.setTunnelPeerBtsPort(0);
    if ( false == msgFTDelEntry.Post() )
        {
        msgFTDelEntry.DeleteMessage();
        }
    return;
}

//显示eid相关信息jy080626
#include "l3datasnoop.h"
#include "l3datasnoopfsm.h"
void eidShow()
{
	UINT32 ulEid = 0;
	
	printf("\r\nPlease Enter EID:");
	getSTDNum((int*)&ulEid);
	printf( "EID = %d = 0x%x", ulEid, ulEid );
	
	printf("\r\n--**--**--**--**--**--**--**--\n");
	printf("\r\nShow CPE Table information\n");
	CTaskDm *taskDM = CTaskDm::GetInstance();
	taskDM->showDMbyEid( ulEid );
	
	printf("\r\n--**--**--**--**--**--**--**--\n");
	printf("\r\nShow Snoop CCB information\n");
	CTSnoop *taskSN = CTSnoop::GetInstance();
	taskSN->showCCBinMem( ulEid );
	taskSN->showCCBinNVRAM( ulEid );
	
	printf("\r\n--**--**--**--**--**--**--**--\n");
	printf("\r\nShow Foward Table information\n");
	CTBridge *taskEB = CTBridge::GetInstance();
	taskEB->showFT( ulEid );	
}
//显示mac相关信息jy080626
void macShow()
{
	UINT8 strMAC[ 7 ] = {0};	
	UINT32 ulEid;
	//UINT16 usIdx;
	
	printf("\r\nPlease Enter a MAC Address[format:xx-xx-xx-xx-xx-xx]");	
	getMacFromSTD( (char*)strMAC );
//	FTEntry *pFT;
	CMac SrcMac( strMAC );
	CTBridge *taskEB = CTBridge::GetInstance();
	FTEntry *pSrcFT = taskEB->GetFTEntryByIdx( taskEB->BPtreeFind( SrcMac ) );
	if ( NULL == pSrcFT )
	{
		printf("\r\nCan not find this MAC in Foward Table\n");
		return;
	}
	else
	{		
		ulEid = pSrcFT->ulEid;		
		printf("\r\n--**--**--**--**--**--**--**--\n");
		printf("\r\nShow Foward Table information\n");
		taskEB->showFT( ulEid );
	}
	
	printf("\r\n--**--**--**--**--**--**--**--\n");
	printf("\r\nShow Snoop CCB information\n");	
	CTSnoop *taskSN = CTSnoop::GetInstance();
	taskSN->showCCBbyMac(SrcMac);	

}
 UINT16 CTBridge::GetPingPacket_Seq(const CComMessage *pComMessge,UINT8 flag)
   {
        EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
	if(pEtherHdr->usProto == M_ETHER_TYPE_IP)//如果是IP包
	{
	    UINT8 *p = (UINT8*) ( pComMessge->GetDataPtr() );
	    if(p[34]==8)//ICMP TYPE
	    {
	        if(p[35]==0)
	        {
	             if(Ping_Check_End==1)
	           {
	               if(flag == 0)
	               {
	                printf("ping from air\n");
	               }
			 else if(flag == 1)
			{
			    printf("ping from tunnel\n");
			}
			else if(flag == 2)
			{
			   printf("ping from wan\n");
			}
			   printf("ping SRC IP:%d-%d-%d-%d -> DST IP:%d-%d-%d-%d\n", p[26],p[27],p[28],p[29],p[30],p[31],p[32],p[33]);
	                 printf("ping SRC MAC:%x-%x-%x-%x-%x-%x -> DST MAC:%x-%x-%x-%x-%x-%x\n", p[6],p[7],p[8],p[9],p[10],p[11],p[0],p[1],p[2],p[3],p[4],p[5]);
			  
			
			    
	           }
	           UINT16 seg = p[41]*0x100+p[40];
		    UINT16 sub = seg - m_ping_seq;
		    
		    
		    if(sub==1)
		    {
		        m_ping_seq = seg;
		         return 1;
		    }
		   else
		   {
		    //  LOG3( LOG_WARN, LOGNO( EB, EC_EB_MSG_EXCEPTION ), "ping seq: %x,%x,%x.", seg, m_ping_seq ,flag);
		      OAM_LOGSTR3( LOG_WARN, LOGNO( EB, EC_EB_MSG_EXCEPTION ), "ping seq: %x,%x,%x.", seg, m_ping_seq ,flag);
		    m_ping_seq = seg;
		   }
		  
	        }
	    }
	}
	return 0;
	
   }
 UINT16 CTBridge::GetPingACKPacket_Seq(const CComMessage *pComMessge,UINT8 flag)
 {
        EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
	if(pEtherHdr->usProto == M_ETHER_TYPE_IP)//如果是IP包
	{
	    UINT8 *p = (UINT8*) ( pComMessge->GetDataPtr() );
	    if(p[34]==0)//Iping  ack TYPE
	    {
	        if(p[35]==0)
	        {
	           if(Ping_Check_End==1)
	           {
	                  if(flag == 0)
	               {
	                printf("ping ack  from air\n");
	               }
			 else if(flag == 1)
			{
			    printf("ping ack from tunnel\n");
			}
			else if(flag == 2)
			{
			   printf("ping ack from wan\n");
			}
			  printf("ping ack SRC IP:%d-%d-%d-%d -> DST IP:%d-%d-%d-%d\n",  p[26],p[27],p[28],p[29],p[30],p[31],p[32],p[33]);
	                 printf("ping ack SRC MAC:%x-%x-%x-%x-%x-%x -> DST MAC:%x-%x-%x-%x-%x-%x\n", p[6],p[7],p[8],p[9],p[10],p[11],p[0],p[1],p[2],p[3],p[4],p[5]);
	
			
			   
	           }
	           UINT16 seg = p[41]*0x100+p[40];
		    UINT16 sub = seg - m_ping_ack_seq;
		    
		    
		    if(sub==1)
		    {
		        m_ping_ack_seq = seg;
		         return 1;
		    }
		   else
		   {
		      //LOG3( LOG_WARN, LOGNO( EB, EC_EB_MSG_EXCEPTION ), "ping ack seq: %x,%x,%x.", seg, m_ping_ack_seq ,flag);
		      OAM_LOGSTR3( LOG_WARN, LOGNO( EB, EC_EB_MSG_EXCEPTION ), "ping ack seq: %x,%x,%x.", seg, m_ping_ack_seq ,flag);
		    m_ping_ack_seq = seg;
		   }
		  
	        }
	    }
	}
	return 0;
    
 }
//wangwenhua add 20080804 to check up lose packet
 void Enable_Ping_Check(UINT8  flag1,UINT8 flag2)
 {
     Ping_Check_Flag = flag1;
	 Ping_Check_End =flag2;
 }




//lijinan 20081205  计费系统增加
bool CCDR::init()
{ 
//	cdrId = 0;
	InitFreeCDRList();
	memset(cdrTbl, 0, sizeof(cdrTbl ) );    //  初始化cdrTbl
	if(OK!=mkdir(CDR_FILE_DIR))
	{
		printf("\n CDR init mkdir fail...\n");
		return false;
	}
		
	cdrSemId = semMCreate(SEM_Q_PRIORITY |SEM_INVERSION_SAFE | SEM_DELETE_SAFE );
       if (NULL == cdrSemId)
       {
           printf("\n CDR init semMCreate fail...\n");
           return false;
       }

   
     /* if ( (  ::taskSpawn( "tWriteCdrFile", 
                200, 
                0,
                10*1024,
                (FUNCPTR)writeFileMainloop, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) ) == ERROR )
    {
	 printf("\nwrite cdr file task create err\n");
        return false;
    }*/
	cdrMsgQ = msgQCreate(100, 100, MSG_Q_FIFO);
	if(cdrMsgQ==NULL)
	{
	    printf("\n CDR init msgQCreate fail...\n");
           return false;
	}
      
       m_tFileInfo.isCdrFileExist = false;
	checkNvramCdrFile();
	#ifdef WBBU_CODE
	memset(cdrBigBuf,0,40000);
	memset(vcdrBigBuf,0,40000);
	#endif
	//new stat
	memset(&billingNewStat, 0, sizeof(stBilling_newStat));
	return true;
	
}

/*============================================================
MEMBER FUNCTION:
    CCDR::InitFreeCDRList

DESCRIPTION:
    初始化空闲CDR 表项链表FreeCdrList

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CCDR::InitFreeCDRList()
{
    FreeCdrList.clear();
    for ( UINT16 usIdx = 0; usIdx <M_MAX_UT_PER_BTS; ++usIdx )
        {
        FreeCdrList.push_back( usIdx );
        }
}

/*============================================================
MEMBER FUNCTION:
    CCDR::InsertFreeCDRList

DESCRIPTION:
    插入空闲转发表表项下标到链表FreeCdrList尾部

ARGUMENTS:
    usIdx:表项下标

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CCDR::InsertFreeCDRList(UINT16 usIdx )
{
    if( usIdx < M_MAX_UT_PER_BTS )
    {
    FreeCdrList.push_back( usIdx );
    }
    else
    {
    DATA_assert( 0 );
    }
}



/*============================================================
MEMBER FUNCTION:
    CCDR::GetFreeCDRIdxFromList

DESCRIPTION:
    从空闲链表FreeCdrList取空闲转发表表项下标;(从链表头部取)

ARGUMENTS:
    NULL

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CCDR::GetFreeCDRIdxFromList()
{
    semTake (cdrSemId, WAIT_FOREVER);
    if ( true == FreeCdrList.empty() )
    {
    g_eb_no_cdr_freelist++;
    semGive (cdrSemId);
    return M_DATA_INDEX_ERR;
    }
   

    UINT16 usIdx = *FreeCdrList.begin();
    FreeCdrList.pop_front();
    semGive (cdrSemId);

    if ( M_MAX_UT_PER_BTS <= usIdx )
    {
    //下标错误
    g_eb_no_cdr_freelist++;
    return M_DATA_INDEX_ERR;
    }
   g_eb_no_cdr_freelist = 0;
    return usIdx;
}


/*============================================================
MEMBER FUNCTION:
    CCDR::BPtreeFind

DESCRIPTION:
    从BPtree搜索mac地址对应的CDR表项下标

ARGUMENTS:
   Eid

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CCDR::BPtreeFind(CMac &Mac)
{
    semTake (cdrSemId, WAIT_FOREVER);
    map<CMac, UINT16>::iterator it = BTreeByMAC.find( Mac );
    
    if ( it != BTreeByMAC.end() )
        {
         semGive (cdrSemId);
        //返回Index.
        return it->second;
        }
    semGive (cdrSemId);
    return M_DATA_INDEX_ERR;
}
/*============================================================
MEMBER FUNCTION:
    CCDR::BPtreeFind

DESCRIPTION:
    从BPtree搜索Eid地址对应的CDR表项下标

ARGUMENTS:
   Eid

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CCDR::BPtreeFind(UINT32 &Eid)
{
    semTake (cdrSemId, WAIT_FOREVER);
    map<UINT32, UINT16>::iterator it = BTreeByEID.find( Eid );
    
    if ( it != BTreeByEID.end() )
        {
        //返回Index.
           if((it->second)<M_MAX_UT_PER_BTS)
           	{
	           if(cdrTbl[it->second].cdr.ulEid!= Eid)
	           	{
	           	    g_cdr_btree_err++;
			  if(CdrTestFlag)
				LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "\nBPtreeFind  g_cdr_btree_err ,cdr eid :%x,eid:%x\n",cdrTbl[it->second].cdr.ulEid,Eid); 
	           	}
           	}
	 semGive (cdrSemId);
        return it->second;
        }
    semGive (cdrSemId);
    return M_DATA_INDEX_ERR;
}

/*============================================================
MEMBER FUNCTION:
    CCDR::BPtreeFindbyUid

DESCRIPTION:
    从BPtree搜索Eid地址对应的CDR表项下标

ARGUMENTS:
   Eid

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CCDR::BPtreeFindbyUid(UINT32 &Uid)
{
    semTake (cdrSemId, WAIT_FOREVER);
    map<UINT32, UINT16>::iterator it = BTreeByUID.find( Uid );
    
    if ( it != BTreeByUID.end() )
        {
         semGive (cdrSemId);
        //返回Index.
        return it->second;
        }
    semGive (cdrSemId);
    return M_DATA_INDEX_ERR;
}


/*============================================================
MEMBER FUNCTION:
    CCDR::BPtreeAdd

DESCRIPTION:
    CDR索引树插入节点；转发表索引树以Eid地址作键值，把CDR
    表项下标插入到索引树

ARGUMENTS:
    Eid
    usIdx:表项下标

RETURN VALUE:
    bool:插入成功/失败

SIDE EFFECTS:
    如果存在重复项，将会返回失败，所以必须在BPtreeAdd之前保证
    没有重复。即首先 BPtreeFind 一下
==============================================================*/
//将cdr的基本信息填写

bool CCDR::BPtreeAdd(UINT32 &Eid, UINT16 usIdx)
{

     UINT8 noPayflag=0;
     UINT8 User_type=0;
     char bw[8];
    if(!(usIdx<M_MAX_UT_PER_BTS))
		return false;
    pair<map<UINT32, UINT16>::iterator, bool> stPair;
    semTake (cdrSemId, WAIT_FOREVER);
    stPair = BTreeByEID.insert( map<UINT32, UINT16>::value_type( Eid, usIdx ) );
    semGive(cdrSemId);
    /*
     *必须保证不存在重复项，否则将会返回失败
     */

    cdrTbl[usIdx].cdr.ulEid = Eid ;

    cdrTbl[usIdx].cdr.ulUid = findUidFromEid(Eid,&noPayflag,&User_type,bw) ;
    cdrTbl[usIdx].noPayflag = noPayflag;
    cdrTbl[usIdx].cdr.User_type = User_type; 
    memcpy(cdrTbl[usIdx].cdr.bw,bw,8);
    if(cdrTbl[usIdx].cdr.ulUid!=0xffffffff)
    {
    	semTake (cdrSemId, WAIT_FOREVER);
    	BTreeByUID.insert( map<UINT32, UINT16>::value_type( cdrTbl[usIdx].cdr.ulUid, usIdx ) );
	semGive (cdrSemId);
    }

    if(CdrTestFlag)
    	 printf("\n CDR::BPtreeAdd:Eid:%04x,Uid:%04x,noPayflag:%02x,User_type:%02x\n",cdrTbl[usIdx].cdr.ulEid,cdrTbl[usIdx].cdr.ulUid,cdrTbl[usIdx].noPayflag,cdrTbl[usIdx].cdr.User_type);
    	 
    return stPair.second;
}


bool CCDR::BPtreeAdd(CMac &Mac, UINT16 usIdx,UINT32 Eid)
{
     UINT8 noPayflag=0;
     UINT8 User_type=0;
     char bw[8];
    if(!(usIdx<M_MAX_UT_PER_BTS))
		return false;
    pair<map<CMac, UINT16>::iterator, bool> stPair;
    semTake (cdrSemId, WAIT_FOREVER);
    stPair = BTreeByMAC.insert( map<CMac, UINT16>::value_type( Mac, usIdx ) );
    semGive (cdrSemId);
    /*
     *必须保证不存在重复项，否则将会返回失败
     */

    cdrTbl[usIdx].cdr.ulEid = Eid ;
    cdrTbl[usIdx].cdr.ulUid = findUidFromEid(Eid,&noPayflag,&User_type,bw) ;
    cdrTbl[usIdx].noPayflag = noPayflag;
    cdrTbl[usIdx].cdr.User_type = User_type; 
    cdrTbl[usIdx].wifi_eid_flag = FLAG_YES;
    memcpy(cdrTbl[usIdx].cdr.MAC,Mac.GetMac(),M_MAC_ADDRLEN);
    memcpy(cdrTbl[usIdx].cdr.bw,bw,8);

    if(CdrTestFlag)
    {
        UINT8 strMac[ M_MACADDR_STRLEN ];
    	 printf("\n wifi CDR::BPtreeAdd:Eid:%04x,Uid:%04x,noPayflag:%02x,User_type:%02x,mac:%s\n",cdrTbl[usIdx].cdr.ulEid,cdrTbl[usIdx].cdr.ulUid,cdrTbl[usIdx].noPayflag,cdrTbl[usIdx].cdr.User_type,Mac.str( strMac ));
    }
    return stPair.second;
}

/*
					UID				4						M
					MAC				6						M
					TimeStamp		7						M	BCD，YYYYMMDDHHMMSS
					Class			1							0-7,0xff- invalid
					BAND_FLAG		1		
					UL Max Bandwidth	2		
					UL Min Bandwidth	2		
					DL Max Bandwidth	2		
					DL Min Bandwidth	2		
*/
bool CCDR::get_cdr_profile(char* pData, UINT16 usIdx)
{
        if(!(usIdx<M_MAX_UT_PER_BTS))
		return false;
	if(cdrTbl[usIdx].wifi_eid_flag==FLAG_YES)
	{
		memcpy(pData,(char*)&cdrTbl[usIdx].cdr.wifi_UID,4);
		memcpy((char*)&pData[4],(char*)cdrTbl[usIdx].cdr.MAC,6);
	}
	else
	{
		if(cdrTbl[usIdx].cdr.ulUid==0xffffffff)
			return false;
		memcpy(pData,(char*)&cdrTbl[usIdx].cdr.ulUid,4);
	}
	memcpy((char*)&pData[19],(char*)cdrTbl[usIdx].cdr.bw,8);
	return true;

}

void CCDR::uidChange(UINT32 new_uid, UINT16 index)
{
       if(!(index<M_MAX_UT_PER_BTS))
		return ;
      semTake (cdrSemId, WAIT_FOREVER);
      if(cdrTbl[index].cdr.ulUid!=0xffffffff)
		BPtreeDelbyUid(cdrTbl[index].cdr.ulUid);
	//semTake (cdrSemId, WAIT_FOREVER);
	BTreeByUID.insert( map<UINT32, UINT16>::value_type( new_uid, index ) );
	semGive (cdrSemId);
}

/*============================================================
MEMBER FUNCTION:
    CCDR::BPtreeDel

DESCRIPTION:
    转发表索引树删除Eid对应的节点；

ARGUMENTS:
    Eid:
RETURN VALUE:
    bool:删除成功/失败

SIDE EFFECTS:
    none
==============================================================*/
bool CCDR::BPtreeDel(UINT32 &Eid)
{
    map<UINT32, UINT16>::iterator it;
    //semTake (cdrSemId, WAIT_FOREVER);
    if ( ( it = BTreeByEID.find( Eid ) ) != BTreeByEID.end() )
        {
        //find, and erase;
        BTreeByEID.erase( it );
        }
    //semGive(cdrSemId);
    //not find
    return true;
}
bool CCDR::BPtreeDelbyUid(UINT32 &Uid)
{
    map<UINT32, UINT16>::iterator it;
    //semTake (cdrSemId, WAIT_FOREVER);
    if ( ( it = BTreeByUID.find( Uid ) ) != BTreeByUID.end() )
        {
        //find, and erase;
        BTreeByUID.erase( it );
        }
   //semGive(cdrSemId);
    //not find
    return true;
}

bool CCDR::BPtreeDel(CMac &Mac)
{
    map<CMac, UINT16>::iterator it;
    //semTake (cdrSemId, WAIT_FOREVER);
    if ( ( it = BTreeByMAC.find( Mac) ) != BTreeByMAC.end() )
        {
        //find, and erase;
        
        BTreeByMAC.erase( it );
       
        }
   //semGive(cdrSemId);
    //not find
    return true;
}



bool CCDR :: RemoveFile( char* chFileName )
{
    DIR* pdir = opendir( CDR_FILE_DIR );
    if( NULL != pdir )
    {
        closedir( pdir );
        char chTmp[CDR_ABSTRACT_FILENAME_LEN];
        FILE * pFile = fopen( GetAbstractFile( chFileName, chTmp ), "rb");
        if( NULL != pFile )
        {
            fclose( pFile );
            remove( GetAbstractFile( chFileName, chTmp ) );
	     if(CdrTestFlag)
		printf("\nRemoveFile succ :%s\n",chTmp);
        }
	/*if(CdrTestFlag)
		printf("\nRemoveFile :%s\n",chTmp);*/
        return true;
    }
    if(CdrTestFlag)
		printf("\nRemoveFile,cannot open :%s\n",CDR_FILE_DIR);
}

bool CCDR::  DelEid(UINT32 &Eid,int flag)
{
	 UINT16 cdrIndex = BPtreeFind( Eid);
	 if(cdrIndex>=M_MAX_UT_PER_BTS)
	 {
		//printf("\ndelete cdr eid ,index err:%x\n",cdrIndex);
		if(CdrTestFlag)
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "\ndelete cdr eid ,index err:%x,eid:%x\n",cdrIndex,Eid );
		return false;
	 }
	 if(CdrTestFlag)
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "\ndelete cdr eid ,index:%x,eid:%x,flag:%d\n",cdrIndex,Eid );
	 if(flag==1)
	 {
	 		semTake (cdrSemId, WAIT_FOREVER);
			 BPtreeDel(Eid);
			 BPtreeDelbyUid(cdrTbl[cdrIndex].cdr.ulUid);
			  //semTake (cdrSemId, WAIT_FOREVER);
			 memset(&(cdrTbl[cdrIndex]),0,sizeof(CDR));
		        InsertFreeCDRList(cdrIndex);
			 semGive (cdrSemId);
			 return true;
	 }

	 if(cdrTbl[cdrIndex].ucMacNum>0)
	 {
	 	cdrTbl[cdrIndex].ucMacNum--;
		if(cdrTbl[cdrIndex].ucMacNum==0)
		{
			 //如果没有业务直接删除，否则发送消息到cdr写文件任务
			 if((cdrTbl[cdrIndex].cdr.Data_flowup[1]==0)&&(cdrTbl[cdrIndex].cdr.Data_flowdn[1]==0))
			 {
			 	 semTake (cdrSemId, WAIT_FOREVER);
				 BPtreeDel(Eid);
				 BPtreeDelbyUid(cdrTbl[cdrIndex].cdr.ulUid);
				 //semTake (cdrSemId, WAIT_FOREVER);
				 memset(&(cdrTbl[cdrIndex]),0,sizeof(CDR));
			        InsertFreeCDRList(cdrIndex);
				 semGive (cdrSemId);
			 }
			 //WriteToFile(&(cdrTbl[cdrIndex].cdr));
			 else
			 {
			       countDurationAndStoptime(&cdrTbl[cdrIndex].lastPtTime,&cdrTbl[cdrIndex].cdr.stStartTime,cdrIndex,0);
				sendMsgToWriteFileTask(FromDelEid,cdrIndex);
			 }
		}
	 }
	 else
	 {
		//如果没有业务直接删除，否则发送消息到cdr写文件任务
		 if((cdrTbl[cdrIndex].cdr.Data_flowup[1]==0)&&(cdrTbl[cdrIndex].cdr.Data_flowdn[1]==0))
		 {
		 	 semTake (cdrSemId, WAIT_FOREVER);
			 BPtreeDel(Eid);
			 BPtreeDelbyUid(cdrTbl[cdrIndex].cdr.ulUid);
			 //semTake (cdrSemId, WAIT_FOREVER);
			 memset(&(cdrTbl[cdrIndex]),0,sizeof(CDR));
		        InsertFreeCDRList(cdrIndex);
			 semGive (cdrSemId);
		 }
		 //WriteToFile(&(cdrTbl[cdrIndex].cdr));
		 else
		 {
		 	countDurationAndStoptime(&cdrTbl[cdrIndex].lastPtTime,&cdrTbl[cdrIndex].cdr.stStartTime,cdrIndex,0);
			sendMsgToWriteFileTask(FromDelEid,cdrIndex);
		 }
	 }
	 	

}
bool CCDR::  DelWifiMac(CMac &Mac)
{
	        UINT16 cdrIndex = BPtreeFind( Mac);
		 if(cdrIndex>=M_MAX_UT_PER_BTS)
		 {
		      UINT8 strMac[ M_MACADDR_STRLEN ];
	             LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "delete cdr wifi mac:%s,index err 0xffff", (int)Mac.str( strMac ) );
			//printf("\ndelete cdr eid ,index err:%x\n",cdrIndex);
			return false;
		 }
		 if(CdrTestFlag)
		 	LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "delete cdr wifi uid:0x%x", cdrTbl[cdrIndex].cdr.wifi_UID);
		
		//如果没有业务直接删除，否则发送消息到cdr写文件任务
		 if((cdrTbl[cdrIndex].cdr.Data_flowup[1]==0)&&(cdrTbl[cdrIndex].cdr.Data_flowdn[1]==0))
		 {
		  	semTake (cdrSemId, WAIT_FOREVER);
			 BPtreeDel(Mac);
			 //semTake (cdrSemId, WAIT_FOREVER);
			 memset(&(cdrTbl[cdrIndex]),0,sizeof(CDR));
		        InsertFreeCDRList(cdrIndex);
			semGive (cdrSemId);
		 }
		 //WriteToFile(&(cdrTbl[cdrIndex].cdr));
		 else
		 {
		 	countDurationAndStoptime(&cdrTbl[cdrIndex].lastPtTime,&cdrTbl[cdrIndex].cdr.stStartTime,cdrIndex,0);
			sendMsgToWriteFileTask(FromDelEid,cdrIndex);
		 }
	 	
		return true;
}



/*buf 数据内容
UID				4						M	
MAC				6						M	0表示不需要解析
FLAG_Limit		1						M	0：业务量无限   1：业务量有限
FLAG_Final		1						M	0：非最后业务量 1：最后的业务量
FLAG_Traffic		1						M	0：不启用流量 1：启用流量
FLAG_Time		1						M	0：不启用时长 1：启用时长
TimeLen			2						M	若时长超过65535填写65535,单位s
Traffic			4						M	若流量超过65535填写65535,单位b
*/
void  CCDR:: func_rtCharge_rsp(UINT8* buf,int flag_wifi_uid)
{
	UINT16 index = 0;
	UINT32 uid = 0;
	memcpy((char*)&uid,buf,4);
	if(flag_wifi_uid)
	{
		CMac wif_mac( buf+4);
		index = BPtreeFind(wif_mac);
	}
	else
	{
		index = BPtreeFindbyUid(uid);
	}
	   if(!(index<M_MAX_UT_PER_BTS))
		return ;
	if(index!=M_DATA_INDEX_ERR)
	{
		cdrTbl[index].flag_limit = buf[10];
		cdrTbl[index].flag_final = buf[11];
		cdrTbl[index].flag_traffic= buf[12];
		cdrTbl[index].flag_time = buf[13];
		memcpy(&cdrTbl[index].time_remain,&buf[14],2);
		memcpy(&cdrTbl[index].data_flow_remain,&buf[16],4);
		if(cdrTbl[index].flag_time)
		{
			if(cdrTbl[index].time_remain)
				cdrTbl[index].flag_over = 0;
		}
		if(cdrTbl[index].flag_traffic)
		{
			if(cdrTbl[index].data_flow_remain)
				cdrTbl[index].flag_over = 0;
		}
		if(CdrTestFlag)
		{
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "func_rtCharge_rsp uid:0x%x,flag:%d", uid,flag_wifi_uid );
			LOG6( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "limit:%d,final:%d,time:%d,traffic:%d,remain,%d,%d\n",cdrTbl[index].flag_limit,cdrTbl[index].flag_final,cdrTbl[index].flag_time,cdrTbl[index].flag_traffic,cdrTbl[index].time_remain,cdrTbl[index].data_flow_remain);
		}
	}
	else
	{
		if(CdrTestFlag)
		{
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "func_rtCharge_rsp uid:0x%x,flag:%d,index err 0xffff", uid,flag_wifi_uid );
		}
		
	}

}


_CDR* CCDR:: setRtChargeRptBuf(char* buf,UINT16 index,RT_RPT_MSG_TYPE flag)
{
	int uid = 0;
	buf[0] = 1;
	   if(!(index<M_MAX_UT_PER_BTS))
		return NULL;
	if(cdrTbl[index].wifi_eid_flag)
		memcpy(&buf[1],&cdrTbl[index].cdr.wifi_UID,4);
	else
		memcpy(&buf[1],&cdrTbl[index].cdr.ulUid,4);
	buf[5] = flag;
	cdrTbl[index].cdr.BS_ID = bspGetBtsID();
	cdrTbl[index].cdr.Len = sizeof(_CDR);
	memcpy(&buf[6],&cdrTbl[index].cdr.Id,4);
	memcpy(&buf[10],&cdrTbl[index].cdr_id_little,2);
	memcpy(&buf[12],&cdrTbl[index].cdr.stStartTime,38);
	memcpy(&buf[50],cdrTbl[index].cdr.bw,8);
	//cdrTbl[index].cdr_id_little++;
	if(CdrTestFlag)
	{ 
		memcpy(&uid,&buf[1],4);
		printf("\nsend RtCharge uid:0x%x,flag:%x,id:%d,%d\n",uid,flag,cdrTbl[index].cdr.Id,cdrTbl[index].cdr_id_little);
	}
	return &cdrTbl[index].cdr;
}

void CCDR::func_clear_Rt_data(UINT16 usIdx)
{
         if(!(usIdx<M_MAX_UT_PER_BTS))
		return ;
	cdrTbl[usIdx].flag_limit = 0;
	cdrTbl[usIdx].flag_final = 0;
	cdrTbl[usIdx].flag_time = 0;
	cdrTbl[usIdx].flag_traffic = 0;
	cdrTbl[usIdx].flag_over = 0;
	cdrTbl[usIdx].time_remain = 0;
	cdrTbl[usIdx].data_flow_remain = 0;
	cdrTbl[usIdx].req_remain_flag = 0;
}

int cdrTest = 0;
/*============================================================
MEMBER FUNCTION:
    CCDR::StatTimeLen

DESCRIPTION:
    统计数据业务时长

ARGUMENTS:
    usIdx:type: 1:15钟统计时间到时计算时长，0:其他统计时长
RETURN VALUE:
none

SIDE EFFECTS:
    none
==============================================================*/
extern "C" int tickGet ();
#ifdef WBBU_CODE
UINT32 changeTimetoSec(int y,int mon,int d,int h,int m,int s);
#endif
void CCDR::StatTimeLen(STAT_TIME_TYPE type,UINT16 usIdx)
{
	UINT32 nowTick = 0;
	UINT32 dltTckt = 0;
	if(usIdx>=M_MAX_UT_PER_BTS)//err
		return;
	 T_TimeDate tTimeData;
	 tTimeData = bspGetDateTime();

	 if(type==FromTimer)
	{		
		
		countDurationAndStoptime(&cdrTbl[usIdx].lastPtTime,&cdrTbl[usIdx].cdr.stStartTime,usIdx,0);

		//只出现过一个包，将统计点时间作为结束时间
		if(cdrTbl[usIdx].cdr.Duration ==0)
		{
			countDurationAndStoptime(&tTimeData,&cdrTbl[usIdx].cdr.stStartTime,usIdx,0);

			//超过一分钟算一分钟
			if(cdrTbl[usIdx].cdr.Duration >60)
			{
				countDurationAndStoptime(&cdrTbl[usIdx].lastPtTime,&cdrTbl[usIdx].cdr.stStartTime,usIdx,60);
			}
		}
		if(cdrTbl[usIdx].cdr.Duration>(gcdrSendPeriod+60))
		{
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ), "\ncdr Duration  err :%d,eid:0x%x\n",cdrTbl[usIdx].cdr.Duration,cdrTbl[usIdx].cdr.ulEid);	
			cdrTbl[usIdx].cdr.Duration = gcdrSendPeriod;
		}
			
		
		//写文件
		WriteToFile(&cdrTbl[usIdx].cdr,usIdx);
		//cdrTbl[usIdx].lastPktTick = 0;
	}

	else
	{
		if(cdrTest)
			printf("\nstat time index:%d,tick:%x,time:%d:%d:%d\n",usIdx,cdrTbl[usIdx].lastPktTick,\
			cdrTbl[usIdx].cdr.stStartTime.hour,cdrTbl[usIdx].cdr.stStartTime.minute,cdrTbl[usIdx].cdr.stStartTime.second);
		
		//以第一个上行包为触发
		if(cdrTbl[usIdx].cdr.Data_flowup[1]==0)
			return;
		
		//如果是到来的第一个包,取当前时间给开始时间
		if(cdrTbl[usIdx].lastPktTick == 0)
		{
			cdrTbl[usIdx].cdr.stStartTime.year[0] = (tTimeData.year/100);
			cdrTbl[usIdx].cdr.stStartTime.year[1] = (tTimeData.year%100);
			cdrTbl[usIdx].cdr.stStartTime.month = tTimeData.month;
			cdrTbl[usIdx].cdr.stStartTime.day = tTimeData.day;
			cdrTbl[usIdx].cdr.stStartTime.hour = tTimeData.hour;
			cdrTbl[usIdx].cdr.stStartTime.minute = tTimeData.minute;
			cdrTbl[usIdx].cdr.stStartTime.second = tTimeData.second;
			cdrTbl[usIdx].lastPktTick = tickGet();
			cdrTbl[usIdx].lastPtTime = tTimeData;

			//CB3000 need cdr id begin from 1
			if(cdrTbl[usIdx].cdr.Id ==0)
			        cdrTbl[usIdx].cdr.Id = changeTimetoSec(tTimeData.year,tTimeData.month,tTimeData.day,tTimeData.hour,tTimeData.minute,tTimeData.second);
				//cdrTbl[usIdx].cdr.Id = 1;
			//不是第一次话单
			if(cdrTbl[usIdx].laskCdrTick!=0)
			{
				//如果第一个包来的时间与上次cdr相差60秒,则认为是一个新的业务,另起话单id
				if((tickGet()-cdrTbl[usIdx].laskCdrTick)>=6000)
					cdrTbl[usIdx].cdr.Id++;

			}
		}
		//否则计算时长
		else
		{
			nowTick = tickGet() ;
			if(nowTick>=cdrTbl[usIdx].lastPktTick)
				dltTckt = nowTick - cdrTbl[usIdx].lastPktTick;
			//考虑到tick越界
			else
				dltTckt = nowTick;
			cdrTbl[usIdx].lastPktTick = nowTick;
			/*
			if(dltTckt>15*60*100)
			{
				//printf("\nStatTimeLen dltTckt err:%x,index:%d\n",dltTckt,usIdx);
				LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\nStatTimeLen dltTckt err:%x\n",dltTckt );
				return;
			}*/
			//大于15min,可能是,中间计费开关关闭过,但是之前的统计也要上报.
			
			//做判决，如果在一分钟之内则累加，否则写文件
			if(dltTckt<6000)
			{
				cdrTbl[usIdx].lastPtTime = tTimeData;
				//增加时长控制
				if(cdrTbl[usIdx].flag_time&&real_time_switch&&cdrTbl[usIdx].flag_limit)
				{
					   int  secLast,secStart,dlt_sec = 0;
					   secLast = changeTimetoSec(tTimeData.year,tTimeData.month,tTimeData.day,tTimeData.hour,tTimeData.minute,tTimeData.second);
					   CdrTime *pt = &cdrTbl[usIdx].cdr.stStartTime;
					   secStart =changeTimetoSec((pt->year[0]*100+pt->year[1]),pt->month,pt->day,pt->hour,pt->minute,pt->second);
					   if(secLast>=secStart)
						dlt_sec = secLast - secStart;
					if(dlt_sec>=cdrTbl[usIdx].time_remain)				/*	//超过时长*/
					{
						if((cdrTbl[usIdx].flag_final))/*断线*/
						{
							cdrTbl[usIdx].flag_over = 1;
						}
						countDurationAndStoptime(&cdrTbl[usIdx].lastPtTime,&cdrTbl[usIdx].cdr.stStartTime,usIdx,0);
						cdrTbl[usIdx].flag_time = 0;
						cdrTbl[usIdx].flag_limit = 0;
						CTBridge *taskEB = CTBridge::GetInstance();
						taskEB->func_RTCharge_rpt(usIdx,RPT_NO_FLOW);
						deleteCdrInform(usIdx);
					}
				}
			}
			
			//间隔超过一分钟了
			else//写文件
			{
				cdrTbl[usIdx].intvlOneMin = 1;
			     
				//将上一个包的时间+1min 算作结束时间，时长等于上一个包的接收时间减去开始时间再加60秒
			      	 countDurationAndStoptime(&cdrTbl[usIdx].lastPtTime,&cdrTbl[usIdx].cdr.stStartTime,usIdx,60);
				 sendMsgToWriteFileTask(From1MinInterval,usIdx);
			
				//cdrTbl[usIdx].lastPktTick = 0;

			}
		}
	}
}


/*============================================================
MEMBER FUNCTION:
    CCDR::StatDataFlow

DESCRIPTION:
    统计数据业务流量

ARGUMENTS:
    usIdx, type:上行或者下行
    dataLen:数据长度包括MAC头的长度
RETURN VALUE:
none

SIDE EFFECTS:
    none
==============================================================*/
void CCDR::StatDataFlow(WRITE_FILE_TYPE type, UINT16 usIdx,UINT16 dataLen)
{
	 if(cdrTest)
	 	printf("\neid:0x%x StatDataFlow type:0x%x,index:%d\n",cdrTbl[usIdx].cdr.ulEid,type,usIdx);

	if((usIdx>=M_MAX_UT_PER_BTS)||(type>CDR_DOWN))//err
		return;
	//算流量需要减去MAC头14字节
       if(dataLen<=14)
	   	return;
	//统计低4位就应该够用了
	//上行
	if(type==CDR_UP)
	{
		cdrTbl[usIdx].cdr.Data_flowup[1] += (dataLen - 14);
        //new stat
		stat_addDataFlow(dataLen - 14);
	}

	//下行
	else
	{
		//以上行第一个包触发
		if(cdrTbl[usIdx].cdr.Data_flowup[1] !=0)
		{
			cdrTbl[usIdx].cdr.Data_flowdn[1] += (dataLen - 14);
            //new stat
		    stat_addDataFlow(dataLen - 14);
		}
	}

	//增加数据流量控制
	if(cdrTbl[usIdx].flag_traffic&&real_time_switch&&cdrTbl[usIdx].flag_limit)
	{
		UINT32 data_flow = cdrTbl[usIdx].cdr.Data_flowup[1] +cdrTbl[usIdx].cdr.Data_flowdn[1];
		if(data_flow>=cdrTbl[usIdx].data_flow_remain)/*				//超过流量*/
		{
			if(cdrTbl[usIdx].flag_final)/*掐断*/
			{
					cdrTbl[usIdx].flag_over = 1;
			}
		       T_TimeDate tTimeData;
 			tTimeData = bspGetDateTime();
			countDurationAndStoptime(&tTimeData,&cdrTbl[usIdx].cdr.stStartTime,usIdx,0);
			cdrTbl[usIdx].flag_traffic = 0;
			cdrTbl[usIdx].flag_limit = 0;
			CTBridge *taskEB = CTBridge::GetInstance();
			taskEB->func_RTCharge_rpt(usIdx,RPT_NO_FLOW);
			deleteCdrInform(usIdx);
		}
	}
	
}


void CCDR::deleteCdrInform(UINT16 usIdx)
{
	if(usIdx>=M_MAX_UT_PER_BTS)//err
		return;
	
	_CDR * pstCdr = &cdrTbl[usIdx].cdr;
	semTake (cdrSemId, WAIT_FOREVER);
	 if(cdrTbl[usIdx].flag_limit)
	 {
		 pstCdr->Id_little = cdrTbl[usIdx].cdr_id_little;
		 cdrTbl[usIdx].cdr_id_little++;
		 if(cdrTbl[usIdx].flag_time)
		 {
		 	if(cdrTbl[usIdx].time_remain>=pstCdr->Duration)
				cdrTbl[usIdx].time_remain -=  pstCdr->Duration;
		 }
		 if(cdrTbl[usIdx].flag_traffic)
		 {
		 	int data_dlt = pstCdr->Data_flowup[1]+pstCdr->Data_flowdn[1];
			if(cdrTbl[usIdx].data_flow_remain>=data_dlt)
				cdrTbl[usIdx].data_flow_remain -=data_dlt;
		 }
	 }
	 memset(&pstCdr->stStartTime,0,sizeof(CdrTime));
	 memset(&pstCdr->stStopTime,0,sizeof(CdrTime));
	 pstCdr->Duration = 0;
	 pstCdr->Data_flowup[1] = 0;
	 pstCdr->Data_flowdn[1] = 0;
	 cdrTbl[usIdx].lastPktTick = 0;
	  semGive (cdrSemId);
	if(pstCdr->ulUid!= 0xffffffff)
	{
		cdrTbl[usIdx].laskCdrTick = tickGet();
	}
}



void CCDR :: GetFileTime( char * chVal )
{    
    T_TimeDate tTimeData;
    struct tm time;
    tTimeData = bspGetDateTime();
    time.tm_sec = tTimeData.second;
    time.tm_min = tTimeData.minute;
    time.tm_hour = tTimeData.hour;
    time.tm_mday = tTimeData.day;
    time.tm_mon = tTimeData.month - 1;
    time.tm_year = tTimeData.year - 1900 ;
    time.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */
   /* if(CdrTestFlag)
    	printf("\nGetFileTime,year:%d\n",tTimeData.year);*/
    ::strftime( chVal, 20, "%Y%m%d%H%M", &time );
	
}

bool CCDR :: MakeCdrFileInfo()
{
    char chTmp[10], chTm[20], chTail[30];
    memset( chTmp, 0, 10 );
    memset( chTm, 0, 20 );
    memset( chTail, 0, 30 );
    GetFileTime( chTm );

    sprintf( chTmp, "%08d", bspGetBtsID() );

    memset( chTail, 0, 30 );
    memcpy( chTail, chTmp, strlen(chTmp) );

    strcat( chTail, "_" );
    strcat( chTail, chTm );
    strcat( chTail, ".cdr" );

    memset( (UINT8*)&m_tFileInfo.CdrFile, 0, sizeof(m_tFileInfo.CdrFile) );
    memcpy( m_tFileInfo.CdrFile, "CDR_", 4 );
    strcat( m_tFileInfo.CdrFile, chTail );

    m_tFileInfo.isCdrFileExist = true;
    cdrId = 0; 

//#ifdef LJF_BILLING_VOICE
    //char chTmp[10], chTm[20], chTail[30];
    //memset( chTmp, 0, 10 );
    //memset( chTm, 0, 20 );
    memset( chTail, 0, 30 );
    //GetFileTime( chTm );

    //sprintf( chTmp, "%08d", bspGetBtsID() );

    memset( chTail, 0, 30 );
    memcpy( chTail, chTmp, strlen(chTmp) );

    strcat( chTail, "_" );
    strcat( chTail, chTm );
    strcat( chTail, ".vcdr" );

    memset( (UINT8*)&m_tFileInfo.VcdrFile, 0, sizeof(m_tFileInfo.VcdrFile) );
    memcpy( m_tFileInfo.VcdrFile, "VCDR_", 5 );
    strcat( m_tFileInfo.VcdrFile, chTail );

    vcdrId = 0; 
	#if 0
    if(CdrTestFlag)
    {
    	//printf("\ncdr file name:%s\n",m_tFileInfo.CdrFile);
	//cdrId = 1;
	T_TimeDate tTimeData = bspGetDateTime();
	#if 0
	cdrTbl[0].cdr.stStartTime.year[0] = (tTimeData.year/100);
	cdrTbl[0].cdr.stStartTime.year[1] = (tTimeData.year%100);
	cdrTbl[0].cdr.stStartTime.month = tTimeData.month;
	cdrTbl[0].cdr.stStartTime.day = tTimeData.day;
	cdrTbl[0].cdr.stStartTime.hour = tTimeData.hour;
	cdrTbl[0].cdr.stStartTime.minute = tTimeData.minute;
	cdrTbl[0].cdr.stStartTime.second = tTimeData.second;

	cdrTbl[0].cdr.Len = sizeof(_CDR);
       cdrTbl[0].cdr.BS_ID = bspGetBtsID();
	cdrTbl[0].cdr.Id = 0;
	cdrTbl[0].cdr.ulEid = 0x12345678;
	cdrTbl[0].cdr.ulUid = 0x87654321;
	cdrTbl[0].lastPktTick = tickGet();

	 cdrTbl[0].cdr.Duration = 900;
	 cdrTbl[0].cdr.Data_flowup[1] = 0x51515151;
	 cdrTbl[0].cdr.Data_flowdn[1] = 0x46464646;


	cdrTbl[2].cdr.stStartTime.year[0] = (tTimeData.year/100);
	cdrTbl[2].cdr.stStartTime.year[1] = (tTimeData.year%100);
	cdrTbl[2].cdr.stStartTime.month = tTimeData.month;
	cdrTbl[2].cdr.stStartTime.day = tTimeData.day;
	cdrTbl[2].cdr.stStartTime.hour = tTimeData.hour;
	cdrTbl[2].cdr.stStartTime.minute = tTimeData.minute;
	cdrTbl[2].cdr.stStartTime.second = tTimeData.second;

	cdrTbl[2].cdr.Len = sizeof(_CDR);
       cdrTbl[2].cdr.BS_ID = bspGetBtsID();
	cdrTbl[2].cdr.Id = 0;
	cdrTbl[2].cdr.ulEid = 0x12345678;
	cdrTbl[2].cdr.ulUid = 0x87654321;
	cdrTbl[2].lastPktTick = tickGet();

	 cdrTbl[2].cdr.Duration = 0x900;
	 cdrTbl[2].cdr.Data_flowup[1] = 0x22222222;
	 cdrTbl[2].cdr.Data_flowdn[1] = 0x33333333;
       // sendMsgToWriteFileTask(From1MinInterval,0);
        cdrTbl[2].lastPtTime = tTimeData;
 	 cdrTbl[0].lastPtTime = tTimeData;
	 #endif
	 
	 UINT16 index = GetFreeCDRIdxFromList();
	 UINT32 eid = 0x12345678;
	 BPtreeAdd(eid, index);
	 cdrTbl[index].cdr.stStartTime.year[0] = (tTimeData.year/100);
	cdrTbl[index].cdr.stStartTime.year[1] = (tTimeData.year%100);
	cdrTbl[index].cdr.stStartTime.month = tTimeData.month;
	cdrTbl[index].cdr.stStartTime.day = tTimeData.day;
	cdrTbl[index].cdr.stStartTime.hour = tTimeData.hour;
	cdrTbl[index].cdr.stStartTime.minute = tTimeData.minute;
	cdrTbl[index].cdr.stStartTime.second = tTimeData.second;

	cdrTbl[index].cdr.Len = sizeof(_CDR);
       cdrTbl[index].cdr.BS_ID = bspGetBtsID();
	cdrTbl[index].cdr.Id = 0;
	
	cdrTbl[index].lastPktTick = tickGet();

	 cdrTbl[index].cdr.Data_flowup[1] = 0x51515151;
	 cdrTbl[index].cdr.Data_flowdn[1] = 0x46464646;
	 cdrTbl[index].lastPtTime = tTimeData;
	 printf("\ncdr file name:%s,index:%d\n",m_tFileInfo.CdrFile,index);

    }
	#endif
    return true;
}

//#ifdef LJF_BILLING_VOICE
bool CCDR :: WriteVoiceToFile( UINT8*pd, UINT16 usLen )     
{
    if( NULL == pd )
        return false;

    FILE *stream;
    int numwritten;
//    _VCDR stTemCdr;

    DIR* pdir = opendir( CDR_FILE_DIR );
    if ( NULL == pdir )
    {
	printf("\nWriteVoiceToFile open CDR_FILE_DIR fail\n" );
        return false;
    }
    else
	{
		//printf("\nWriteVoiceToFile open CDR_FILE_DIR ok\n" );
		closedir( pdir );
	}
   // T_TimeDate tTimeData;
    char chTmp[CDR_ABSTRACT_FILENAME_LEN];

	stream = fopen( GetAbstractFile(m_tFileInfo.VcdrFile, chTmp), "ab" );

    if( stream != NULL )
    {
		//文件头部分等上报时再填，头共20 +12字节
		UINT16 offset = sizeof(m_tfileHead)+ vcdrId*sizeof(_VCDR);
		#ifndef WBBU_CODE
		fseek(stream,offset,SEEK_SET);
		numwritten = fwrite( ( char *)pd, sizeof( char ), sizeof(_VCDR), stream );
		if( sizeof(_VCDR) != numwritten )
			printf("\nWriteVoiceToFile write to file fail\n" );
		else
		#else
		if((offset+sizeof(_VCDR))>=40000)
		{
			fclose( stream );
			printf("\nvcdr file too long >40000\n");
			return false;
		}
		memcpy(&vcdrBigBuf[offset],( char *)pd,sizeof(_VCDR));
		#endif
			vcdrId++;
		fclose( stream );
	}
	else
	{
		//fclose( stream );
		printf("\nWriteVoiceToFile open file fail\n" );
	}
}

bool CCDR :: WriteToFile(      _CDR* pstCdr,UINT32 index)
                               		

{
    if( NULL == pstCdr )
        return false;
       if(!(index<M_MAX_UT_PER_BTS))
		return false;
    FILE *stream;
    int numwritten;
    _CDR stTemCdr;

    DIR* pdir = opendir( CDR_FILE_DIR );
    if ( NULL == pdir )
    {
        return false;
    }
    else
        closedir( pdir );
    if(m_tFileInfo.isCdrFileExist!=true)
    {
	  MakeCdrFileInfo();
    }
 //   T_TimeDate tTimeData;
    char chTmp[CDR_ABSTRACT_FILENAME_LEN];

     stream = fopen( GetAbstractFile(m_tFileInfo.CdrFile, chTmp), "ab" );

    if( stream != NULL )
    {

	//pstCdr->Id = cdrId+1;//id 从1开始


	pstCdr->Len = sizeof(_CDR);
       pstCdr->BS_ID = bspGetBtsID();
	//pstCdr->ulUid =  findUidFromEid(pstCdr->ulEid);
	semTake (cdrSemId, WAIT_FOREVER);
	 //将cdr相关统计数据清零
	// pstCdr->Id = 0;
	 if(cdrTbl[index].flag_limit)
	 {
		 pstCdr->Id_little = cdrTbl[index].cdr_id_little;
		 cdrTbl[index].cdr_id_little++;
		 if(cdrTbl[index].flag_time)
		 {
		 	if(cdrTbl[index].time_remain>=pstCdr->Duration)
				cdrTbl[index].time_remain -=  pstCdr->Duration;
		 }
		 if(cdrTbl[index].flag_traffic)
		 {
		 	int data_dlt = pstCdr->Data_flowup[1]+pstCdr->Data_flowdn[1];
			if(cdrTbl[index].data_flow_remain>=data_dlt)
				cdrTbl[index].data_flow_remain -=data_dlt;
		 }
	 }	 
	 
	 memcpy(( char *)&stTemCdr,( char *)pstCdr,sizeof(_CDR));
	 memset(&pstCdr->stStartTime,0,sizeof(CdrTime));
	 memset(&pstCdr->stStopTime,0,sizeof(CdrTime));
	 pstCdr->Duration = 0;
	 pstCdr->Data_flowup[1] = 0;
	 pstCdr->Data_flowdn[1] = 0;
	 cdrTbl[index].lastPktTick = 0;
	  if(CdrTestFlag)
	  	printf("\nwrite file :%s\n",chTmp);
	  semGive (cdrSemId);
	if(stTemCdr.ulUid!= 0xffffffff)
	{
		//memcpy(( char *)&stTemCdr,( char *)pstCdr,sizeof(_CDR));
		
	
	        //文件头部分等上报时再填，头共20 +12字节
	        UINT16 offset = sizeof(m_tfileHead)+ cdrId*sizeof(_CDR);
		#ifndef WBBU_CODE
		 fseek(stream,offset,SEEK_SET);
	        numwritten = fwrite( ( char *)&stTemCdr, sizeof( char ), sizeof(_CDR), stream );
	        #endif
	        fclose( stream );
		#ifdef WBBU_CODE
		if((offset+sizeof(_CDR))>=40000)
		{
			fclose( stream );
			printf("\ncdr file too long >40000\n");
			return false;
		}
		memcpy(&cdrBigBuf[offset],( char *)&stTemCdr,sizeof(_CDR));
		#endif
		 if(cdrTbl[index].intvlOneMin==1 )
		 {
		 	 cdrTbl[index].laskCdrTick = (tickGet() - 6000);
			 cdrTbl[index].intvlOneMin = 0;
		 }
		 else
		 	 cdrTbl[index].laskCdrTick = tickGet();
		 cdrId++;
	}
	else
	{
	
		        fclose( stream );
	}

	/*semTake (cdrSemId, WAIT_FOREVER);
	 //将cdr相关统计数据清零
	// pstCdr->Id = 0;
	
	 memset(&pstCdr->stStartTime,0,sizeof(CdrTime));
	 memset(&pstCdr->stStopTime,0,sizeof(CdrTime));
	 pstCdr->Duration = 0;
	 pstCdr->Data_flowup[1] = 0;
	 pstCdr->Data_flowdn[1] = 0;
	 cdrTbl[index].lastPktTick = 0;
	  if(CdrTestFlag)
	  	printf("\nwrite file :%s\n",chTmp);
	  semGive (cdrSemId);*/
        return true;
    }
    else
    {
       if(CdrTestFlag)
    	 printf("\ncannot open :%s\n",chTmp);
        return false;
    }
}

bool CCDR :: WriteToFile(char* data)

{
    if( NULL == data )
        return false;

    FILE *stream;
    int numwritten;

    DIR* pdir = opendir( CDR_FILE_DIR );
    if ( NULL == pdir )
    {
        return false;
    }
    else
        closedir( pdir );
    if(m_tFileInfo.isCdrFileExist!=true)
    {
	  MakeCdrFileInfo();
    }
    char chTmp[CDR_ABSTRACT_FILENAME_LEN];

     stream = fopen( GetAbstractFile(m_tFileInfo.CdrFile, chTmp), "ab" );

    if( stream != NULL )
    {

	  if(CdrTestFlag)
	  	printf("\nwrite file :%s\n",chTmp);

	        //文件头部分等上报时再填，头共20 +12字节
	        UINT16 offset = sizeof(m_tfileHead)+ cdrId*sizeof(_CDR);
		#ifndef WBBU_CODE
		 fseek(stream,offset,SEEK_SET);
	        numwritten = fwrite( data, sizeof( char ), sizeof(_CDR), stream );
	        #endif
	        fclose( stream );
		#ifdef WBBU_CODE
		if((offset+sizeof(_CDR))>=40000)
		{
			fclose( stream );
			printf("\ncdr file too long >40000\n");
			return false;
		}
		memcpy(&cdrBigBuf[offset],data,sizeof(_CDR));
		#endif
		 cdrId++;
		
        return true;
    }
    else
    {
       if(CdrTestFlag)
    	 	printf("\ncannot open :%s\n",chTmp);
        return false;
    }
}

//写文件头部

bool CCDR :: WriteFileHead( )

{

    FILE *stream;
    int numwritten;

    DIR* pdir = opendir( CDR_FILE_DIR );
    if ( NULL == pdir )
    {
        //printf("\nwrite file head,but cannot dir:%s\n",CDR_FILE_DIR);
	 LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\nwrite file head,but cannot dir:%s\n",(int)CDR_FILE_DIR);
        return false;
    }
    else
        closedir( pdir );
    char chTmp[CDR_ABSTRACT_FILENAME_LEN];

     stream = fopen( GetAbstractFile(m_tFileInfo.CdrFile, chTmp), "ab" );

    if( stream != NULL )
    {
        memset( (UINT8*)&m_tfileHead, 0, sizeof(m_tfileHead) );
        m_tfileHead.version = 0x00000001;
        m_tfileHead.cdrIndex = 0x0015;


	 m_tfileHead.cdrNum = cdrId;
	 m_tfileHead.fileLen = 32+cdrId*sizeof(_CDR) ;
	 #ifndef WBBU_CODE
	 fseek(stream,0,SEEK_SET);
        numwritten = fwrite( (char*)&m_tfileHead, sizeof( char ), 32, stream );
         #endif
        fclose( stream );
        
       #ifdef WBBU_CODE
       memcpy(cdrBigBuf,(char*)&m_tfileHead,32);
	#endif
       /* if(CdrTestFlag)
	{		
		printf("\nwrite file head cdrId:%x,%x,%x,%x,numwritten:%x \n",cdrId,sizeof(_CDR),sizeof(m_tfileHead),m_tfileHead.fileLen,numwritten);
		 stream = fopen( GetAbstractFile(m_tFileInfo.CdrFile, chTmp), "rb" );
		 fread(&chTmp, 1, 32, stream);
		 for(int i =0;i<8;i++)
		  printf("\n....................:%x,%x,%x,%x\n",chTmp[4*i],chTmp[4*i+1],chTmp[4*i+2],chTmp[4*i+3]);
		 fclose( stream );
		 char * p =  (char*)&m_tfileHead;
		 for(int i =0;i<8;i++)
		    printf("\n....................:%x,%x,%x,%x\n",p[4*i],p[4*i+1],p[4*i+2],p[4*i+3]);
        }*/
#if 0//ndef LJF_BILLING_VOICE
	return true;
#endif
    }
    else
    {
        //printf("\nwrite file head,but cannot open:%s\n",chTmp);
		LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\nwrite file head,but cannot open:%s\n",(int)chTmp);
#if 0//ndef LJF_BILLING_VOICE
		return false;
#endif
    }
//#ifdef LJF_BILLING_VOICE
	memset( chTmp, 0, CDR_ABSTRACT_FILENAME_LEN );
	stream = fopen( GetAbstractFile(m_tFileInfo.VcdrFile, chTmp), "ab" );

    if( stream != NULL )
    {
        memset( (UINT8*)&m_tfileHead, 0, sizeof(m_tfileHead) );
        m_tfileHead.version = 0x00000001;
        m_tfileHead.cdrIndex = 0x0015;
		m_tfileHead.cdrNum = vcdrId;
		m_tfileHead.fileLen = 32+vcdrId*sizeof(_VCDR) ;
		#ifndef WBBU_CODE
		fseek(stream,0,SEEK_SET);
		numwritten = fwrite( (char*)&m_tfileHead, sizeof( char ), 32, stream );
		#else
		memcpy(vcdrBigBuf,(char*)&m_tfileHead,32);
		#endif
		fclose( stream );
		return true;
    }
    else
    {
	 LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\nwrite file head,but cannot open:%s\n",(int)chTmp);
        return false;
    }
}
char buf_to_cb3000[1024];
char buf_to_cb3000_fail[1024];
void CCDR::CdrDataFileUploadTimer()
{
		int usr_cnt = 0;
		UINT32 data_flow = 0,nowTick,dltTckt;
		char temp_file[48] ;
		T_TimeDate tTimeData;
		if(real_time_switch)
		{
 			tTimeData = bspGetDateTime();
			nowTick = tickGet() ;
		}
              if(m_tFileInfo.isCdrFileExist == true)
		{
		      //计算时长，写文件，上传文件
		     //  semTake (cdrSemId, WAIT_FOREVER);
			 for ( UINT16 usIdx = 0; usIdx <M_MAX_UT_PER_BTS; ++usIdx )
			 {
			 	//判断是否存在数据
				//if(!(cdrTbl[index].cdr.Data_flowup[1]==0&&cdrTbl[index].cdr.Data_flowdn[1]==0))
				//以上行流量为触发
				if(!(cdrTbl[usIdx].cdr.Data_flowup[1]==0))
				{
					if(real_time_switch)
					{
						if(cdrTbl[usIdx].flag_limit&&cdrTbl[usIdx].flag_traffic)
						{
							char *p = &buf_to_cb3000[usr_cnt*65+1]; 

							if(cdrTbl[usIdx].wifi_eid_flag)
								memcpy(&p[0],&cdrTbl[usIdx].cdr.wifi_UID,4);
							else
								memcpy(&p[0],&cdrTbl[usIdx].cdr.ulUid,4);
							p[4] = RPT_PERIOD;
							countDurationAndStoptime(&tTimeData,&cdrTbl[usIdx].cdr.stStartTime,usIdx,0);
							cdrTbl[usIdx].cdr.BS_ID = bspGetBtsID();
							cdrTbl[usIdx].cdr.Len = sizeof(_CDR);
							memcpy(&p[5],&cdrTbl[usIdx].cdr.Id,4);
							memcpy(&p[9],&cdrTbl[usIdx].cdr_id_little,2);
							memcpy(&p[11],&cdrTbl[usIdx].cdr.stStartTime,38);
							memcpy(&p[49],cdrTbl[usIdx].cdr.bw,8);
						       data_flow = cdrTbl[usIdx].cdr.Data_flowup[1] +cdrTbl[usIdx].cdr.Data_flowdn[1];
							if(data_flow<cdrTbl[usIdx].data_flow_remain)	
								data_flow = cdrTbl[usIdx].data_flow_remain - data_flow;
							memset(&p[57],4,0);
							memcpy(&p[61],&data_flow,4);
							memcpy(&buf_to_cb3000_fail[sizeof(_CDR)*usr_cnt],(char*)&cdrTbl[usIdx].cdr,sizeof(_CDR));
							usr_cnt++;
							buf_to_cb3000[0] = usr_cnt;
							if(usr_cnt==10)
							{
								CTBridge *taskEB = CTBridge::GetInstance();
								taskEB->func_RTCharge_rpt(buf_to_cb3000,(usr_cnt*65+1),RT_CHARGE_RPT_PERIOD,buf_to_cb3000_fail,(usr_cnt*sizeof(_CDR)));/*一个大包发给cb3000*/
								usr_cnt = 0;
							}
							deleteCdrInform(usIdx);
							
						}
						if(cdrTbl[usIdx].flag_limit)
						{
							if(cdrTbl[usIdx].flag_time)
							{

								if(nowTick>=cdrTbl[usIdx].lastPktTick)
									dltTckt = nowTick - cdrTbl[usIdx].lastPktTick;
								//考虑到tick越界
								else
									dltTckt = nowTick;
								if(dltTckt>6000)
								{
									StatTimeLen(FromTimer,usIdx);
								}
								
							}
							continue;
						}
					}
					//计算时长,写文件
					StatTimeLen(FromTimer,usIdx);
				}
			 }
			if(real_time_switch)
			{
				if(usr_cnt)
				{
					CTBridge *taskEB = CTBridge::GetInstance();
					taskEB->func_RTCharge_rpt(buf_to_cb3000,(usr_cnt*65+1),RT_CHARGE_RPT_PERIOD,buf_to_cb3000_fail,(usr_cnt*sizeof(_CDR)));
				}
			}
			// semGive (cdrSemId);

			 //写文件头部分，发送文件
			 WriteFileHead();

			 
			 /*发送失败，将文件存到CF卡，文件名保存在NVRAM
				   并且开始一分钟ftp连接定时期,一小时4条cdr，一天96条，
				   只保存失败的96条cdr*/
		        if(cdrId!=0)
		        {
			        #ifdef WBBU_CODE
				 if(!(SendCdrFile(m_tFileInfo.CdrFile,0)))
				 #else
				 if(!(SendCdrFile(m_tFileInfo.CdrFile)))
				 #endif
				 {
					for(int i = 0;i<MAX_SAVE_CDR_NUM;i++)
					{
						if(strlen(gpNVRamCdrName[i].name)==0)
						{
							if( writeFileToCF(m_tFileInfo.CdrFile))
							{
								 if(bspEnableNvRamWrite( (char*)gpNVRamCdrName, NVRamCdrNameLength)==TRUE)
							 	{
								     memcpy(gpNVRamCdrName[i].name,m_tFileInfo.CdrFile,sizeof(m_tFileInfo.CdrFile));
							            //关闭
							            bspDisableNvRamWrite( (char*)gpNVRamCdrName, NVRamCdrNameLength );
							 	}
								 gNeedSendCFFileToFtp = FLAG_YES;
							}

							 break;
						}
					}
				 }
		        }
	

			 //将旧文件删除，建立新文件
			 RemoveFile(m_tFileInfo.CdrFile);
//#ifdef LJF_BILLING_VOICE//fix it
		 //printf("\nSendCdrFile VcdrFile......\n");
		 if(vcdrId!=0)
		  {
			 #ifdef WBBU_CODE
			 if(!(SendCdrFile(m_tFileInfo.VcdrFile,1)))   //ljf test alter, should open@@@@@@@@@
			 #else
			 if(!(SendCdrFile(m_tFileInfo.VcdrFile)))   //ljf test alter, should open@@@@@@@@@
			 #endif
			 {
				 printf("\nSendCdrFile VcdrFile......fail\n");
				for(int i = 0;i<MAX_SAVE_CDR_NUM;i++)
				{
					if(strlen(gpNVRamVcdrName[i].name)==0)
					{
						if( writeFileToCF(m_tFileInfo.VcdrFile))
						{
							 if(bspEnableNvRamWrite( (char*)gpNVRamVcdrName, NVRamCdrNameLength)==TRUE)
						 	{
							     memcpy(gpNVRamVcdrName[i].name,m_tFileInfo.VcdrFile,sizeof(m_tFileInfo.VcdrFile));
						            //关闭
						            bspDisableNvRamWrite( (char*)gpNVRamVcdrName, NVRamCdrNameLength );
						 	}
							 gNeedSendCFFileToFtp = FLAG_YES;
						}

						 break;
					}
				}
			}
		 }
		 RemoveFile(m_tFileInfo.VcdrFile);
	}		
	 memcpy(temp_file,m_tFileInfo.CdrFile,sizeof(m_tFileInfo.CdrFile));
	 MakeCdrFileInfo();
	 if(0==strcmp(temp_file,m_tFileInfo.CdrFile))
	 {
		for(int j=8;j<48;j++)
		{
			if(m_tFileInfo.CdrFile[j]=='.')
			{
				strcpy(&m_tFileInfo.CdrFile[j],"30.cdr");
				break;
			}
		}
	 }
}

void CCDR::bs_reboot_send_cdr()
{
              if(m_tFileInfo.isCdrFileExist == true)
		{
		      //计算时长，写文件，上传文件
		     //  semTake (cdrSemId, WAIT_FOREVER);
			 for ( UINT16 usIdx = 0; usIdx <M_MAX_UT_PER_BTS; ++usIdx )
			 {
			 	//判断是否存在数据
				//if(!(cdrTbl[index].cdr.Data_flowup[1]==0&&cdrTbl[index].cdr.Data_flowdn[1]==0))
				//以上行流量为触发
				if(!(cdrTbl[usIdx].cdr.Data_flowup[1]==0))
				{
					//计算时长,写文件
					StatTimeLen(FromTimer,usIdx);
				}
			 }
			// semGive (cdrSemId);

			 //写文件头部分，发送文件
			 WriteFileHead();

			 
			 /*发送失败，将文件存到CF卡，文件名保存在NVRAM
				   并且开始一分钟ftp连接定时期,一小时4条cdr，一天96条，
				   只保存失败的96条cdr*/
		        #ifdef WBBU_CODE
			 if(!(SendCdrFile(m_tFileInfo.CdrFile,0)))
			 #else
			 if(!(SendCdrFile(m_tFileInfo.CdrFile)))
			 #endif
			 {
				for(int i = 0;i<MAX_SAVE_CDR_NUM;i++)
				{
					if(strlen(gpNVRamCdrName[i].name)==0)
					{
						if( writeFileToCF(m_tFileInfo.CdrFile))
						{
							 if(bspEnableNvRamWrite( (char*)gpNVRamCdrName, NVRamCdrNameLength)==TRUE)
						 	{
							     memcpy(gpNVRamCdrName[i].name,m_tFileInfo.CdrFile,sizeof(m_tFileInfo.CdrFile));
						            //关闭
						            bspDisableNvRamWrite( (char*)gpNVRamCdrName, NVRamCdrNameLength );
						 	}
							 gNeedSendCFFileToFtp = FLAG_YES;
						}

						 break;
					}
				}
			 }
	

			 //将旧文件删除，建立新文件
			 RemoveFile(m_tFileInfo.CdrFile);
//#ifdef LJF_BILLING_VOICE//fix it
		 //printf("\nSendCdrFile VcdrFile......\n");
		 #ifdef WBBU_CODE
		 if(!(SendCdrFile(m_tFileInfo.VcdrFile,1)))   //ljf test alter, should open@@@@@@@@@
		 #else
		 if(!(SendCdrFile(m_tFileInfo.VcdrFile)))   //ljf test alter, should open@@@@@@@@@
		 #endif
		 {
			 printf("\nSendCdrFile VcdrFile......fail\n");
			for(int i = 0;i<MAX_SAVE_CDR_NUM;i++)
			{
				if(strlen(gpNVRamVcdrName[i].name)==0)
				{
					if( writeFileToCF(m_tFileInfo.VcdrFile))
					{
						 if(bspEnableNvRamWrite( (char*)gpNVRamVcdrName, NVRamCdrNameLength)==TRUE)
					 	{
						     memcpy(gpNVRamVcdrName[i].name,m_tFileInfo.VcdrFile,sizeof(m_tFileInfo.VcdrFile));
					            //关闭
					            bspDisableNvRamWrite( (char*)gpNVRamVcdrName, NVRamCdrNameLength );
					 	}
						 gNeedSendCFFileToFtp = FLAG_YES;
					}

					 break;
				}
			}
		 }
		 RemoveFile(m_tFileInfo.VcdrFile);
	}		
	 MakeCdrFileInfo();
}



 char    ftpbuf [512];
#ifdef WBBU_CODE
bool CCDR :: SendCdrFile( char* chFileName,int flag )
#else
bool CCDR :: SendCdrFile( char* chFileName)
#endif
{
    if( NULL == chFileName )
        return false;

    int        ctrlSock;
    int        dataSock;
    int        nBytes;
   // char    buf [512];
    STATUS    status;

    struct in_addr Svriaddr;
    Svriaddr.s_addr = g_tFTPInfo.uIPAddr;
    SINT8    IpAddr[16];
    memset( IpAddr, 0, sizeof(IpAddr) );
    inet_ntoa_b( Svriaddr, IpAddr );

    char chTmp[CDR_ABSTRACT_FILENAME_LEN];
    #ifdef WBBU_CODE
     FILE *stream;
      int fileLen = 0;
     if(flag==1)
	 	fileLen = sizeof(m_tfileHead)+vcdrId*sizeof(_VCDR) ;
     else
     		fileLen = sizeof(m_tfileHead)+cdrId*sizeof(_CDR) ;
    stream = fopen( GetAbstractFile(chFileName, chTmp), "ab" );
    if( stream != NULL )
    {
    		if(flag==1)
    		{
			fwrite( vcdrBigBuf, sizeof( char ), fileLen, stream );
    		}
		else
		{
    			fwrite( cdrBigBuf, sizeof( char ), fileLen, stream );
		}
    		 fclose( stream );
    }
    else
    {
    		if(CdrTestFlag)
			printf("\nwbbu send to ftp ,but cannot open  file:%s",chTmp);
		return false;
    }
    #endif
    FILE * pFile = fopen( GetAbstractFile( chFileName, chTmp ), "rb");
    if( NULL == pFile )
    {
    	if(CdrTestFlag)
			printf("\nsend to ftp ,but cannot open  file:%s",chTmp);
        return false;
    }
	if( ERROR == ftpXfer( IpAddr, 
                          g_tFTPInfo.chUserName, 
                          g_tFTPInfo.chPassWord, 
                          NULL, 
                          "STOR %s", 
                          CDR_UPLOAD_DIR, 
                          chFileName, 
                          &ctrlSock, 
                          &dataSock) )
    /*if( ERROR == ftpXfer( "172.16.8.219", 
                          "l3", 
                          "l3", 
                          NULL, 
                          "STOR %s", 
                          CDR_UPLOAD_DIR, 
                          chFileName, 
                          &ctrlSock, 
                          &dataSock) )*/
    {
        LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "FILE UPLOAD ERROR." );
        fclose(pFile);
		if(CdrTestFlag)
			printf("\nsend to ftp ,but cannot open  route file:%s",chTmp);
        return false;
    }

    //lijinan 090112	
  //  gFlagInFtp = FLAG_YES;
    memset(ftpbuf, 0, sizeof(ftpbuf));
    while( (nBytes = fread(ftpbuf, 1, 512, pFile)) > 0 )
    {
        ::write (dataSock, ftpbuf, nBytes );
        memset(ftpbuf, 0, sizeof(ftpbuf));
    }

    fclose(pFile);
    close (dataSock);

    if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
    {
        status = false;
    }
    if (ftpCommand (ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
    {
        status = false;
    }
    close (ctrlSock);
    		if(CdrTestFlag)
			printf("\nsend to ftp succ\n");
  //  gFlagInFtp = FLAG_NO;
    return true;
}

//#ifdef LJF_BILLING_VOICE
#include "L3OamCfg.h"
void bill( UINT8 uc )
{
	CTBridge* ptask = CTBridge::GetInstance();
	CCDR* pCdr = ptask->GetCdrInstance();
	pCdr->billing(uc);
	return;
}
UINT8 gCnt = 0;
void CCDR ::billing( UINT8 uc )
{
	switch (uc)
	{
		int i;
	case 0:
		printf( "gNeedSendCFFileToFtp = %s\n", (FLAG_YES==gNeedSendCFFileToFtp)?"YES":"NO" );
		printf( "gstrlen(gpNVRamVcdrName[0].name) = %d\n", strlen(gpNVRamVcdrName[0].name) );
		//printf( "%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ", gpNVRamVcdrName[0].name[0] )
		//if( 0 != strlen(gpNVRamVcdrName[0].name) )
		//	printf( "\n%s", gpNVRamVcdrName[0].name );
		//else
		//	printf( "\nstrlen(gpNVRamVcdrName[0].name)==0" );
		for( i=0; i<MAX_SAVE_CDR_NUM; i++ )
		{
			if( 0<strlen(gpNVRamVcdrName[i].name) && 40>strlen(gpNVRamVcdrName[i].name) )
				printf( "%s\n", gpNVRamVcdrName[i].name );
			if( 40<strlen(gpNVRamVcdrName[i].name) )
				printf( "40<strlen(gpNVRamVcdrName[%d].name\n", i );
		}
		break;
	case 1:
		printf( "\nclearup gpNVRamCdrName" );
		#if 1
		char ucc[3840];
		memset( ucc, 0, 3840 );
		CTaskCfg::GetInstance()->l3oambspNvRamWrite( (char*)gpNVRamCdrName, ucc, 3840 );
		#else
		char ucc[40];
		memset( ucc, 0, 40 );
		CTaskCfg::GetInstance()->l3oambspNvRamWrite( (char*)gpNVRamCdrName[gCounter].name, ucc, 40 );
		gCounter++;
		#endif
		break;
	default:
		printf( "\nbilling default" );
		break;
	}
	return;
}
void CCDR ::sendVoiceMsgToWriteFileTask( CDR_TYPE type, UINT8*pd, UINT16 usLen )
{

	UINT8* pcbuf = new UINT8[usLen];
	stVoiceBilling stSend;
	stSend.type = type;
	stSend.pucAdd = pcbuf;
	stSend.ulLen = usLen;
	if(pcbuf!=NULL)
	{
	memcpy( pcbuf, pd, usLen );
	LOG1(  LOG_SEVERE, LOGNO( EB, EC_EB_PARAMETER ),"sendVoiceMsgToWriteFileTask::usLen[%d]",usLen );
	LOG6(  LOG_SEVERE, LOGNO( EB, EC_EB_PARAMETER ),"[%02x][%02x][%02x][%02x][%02x][%02x]",*(pcbuf+0),*(pcbuf+1),*(pcbuf+2),*(pcbuf+3),*(pcbuf+4),*(pcbuf+5) );
	LOG6(  LOG_SEVERE, LOGNO( EB, EC_EB_PARAMETER ),"[%02x][%02x][%02x][%02x][%02x][%02x]",*(pcbuf+23),*(pcbuf+24),*(pcbuf+25),*(pcbuf+26),*(pcbuf+27),*(pcbuf+28) );
	LOG6(  LOG_SEVERE, LOGNO( EB, EC_EB_PARAMETER ),"[%02x][%02x][%02x][%02x][%02x][%02x]",*(pcbuf+73),*(pcbuf+74),*(pcbuf+75),*(pcbuf+76),*(pcbuf+77),*(pcbuf+78) );

	int state = ::msgQSend (cdrMsgQ, (char*)&stSend, sizeof(stVoiceBilling), NO_WAIT,  MSG_PRI_NORMAL);
	if(!(OK==state))
		{
		delete []pcbuf;
		//printf("write cdr file msg send err,errno:%x,msg type:%x\n ",state,type);
		LOG2( LOG_SEVERE, LOGNO( EB, EC_EB_PARAMETER ),"write cdr file voice msg send err,errno:%x,msg type:%x\n ",state,type);
		}
		else
		{
		     if(CdrTestFlag)
			LOG2( LOG_SEVERE, LOGNO( EB, EC_EB_PARAMETER ),"write cdr file voice msg send ok,no:%x,msg type:%x\n ",state,type);
		}
	}

}
void CCDR ::sendMsgToWriteFileTask(CDR_TYPE type,UINT16 index)
{
	char buf[16];
	if(CdrTestFlag)
		LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\nsend msg to write file task para :%x,index:0x%x\n",type,index);
	if(index>=M_MAX_UT_PER_BTS)
		//printf("\nsend msg to write file task para err:%x,%x\n",type,index);
		LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\nsend msg to write file task para err:%x\n",type);
	else
	{
		buf[0] = type;
		buf[1] = index>>8;
		buf[2] = index&0xff;
		int state = ::msgQSend (cdrMsgQ, buf, sizeof(buf), NO_WAIT,  MSG_PRI_NORMAL);
		if(!(OK==state))
		{
			//printf("write cdr file msg send err,errno:%x,msg type:%x\n ",state,type);
			LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"write cdr file msg send err,errno:%x,msg type:%x,restart cdr task\n ",state,type);
			restartTsk();
		}
			
	}
	

}

void CCDR ::restartTsk()
{
          semTake (cdrSemId, WAIT_FOREVER);
	   ::taskLock();
          taskRestart(getWriteFileTaskId());
	   ::msgQDelete(cdrMsgQ);
	   cdrMsgQ = msgQCreate(100, 100, MSG_Q_FIFO);
	    if(cdrMsgQ==NULL)
 	   {
	 	    printf("\n CDR init msgQCreate fail...\n");
 	   }
	   ::taskUnlock();
	   semGive (cdrSemId);
	   printf("\nrestart cdr task\n");
}

extern "C" void restartcdrtask()
{
    CTBridge *taskEB = CTBridge::GetInstance();
    taskEB->restartcdrTsk();		
}

//用于写cdr文件的任务
 STATUS CCDR::writeFileMainloop()
 {
#if 1//def LJF_BILLING_VOICE
	unsigned char buf[400];
//	T_TimeDate tTimeData;
#else
	unsigned char buf[128];
#endif
	int len = 0;
       UINT16 index  = 0;
 	while(1)
 	{
 		memset(buf,0,sizeof(buf));
 		len = msgQReceive(cdrMsgQ,(char*)buf,(UINT)sizeof(buf),WAIT_FOREVER);
//#ifdef LJF_BILLING_VOICE
		stVoiceBilling* stRcv = (stVoiceBilling*) buf;
		if((len==ERROR)||(len>100))
			continue;
		gFlagInFtp = FLAG_YES;
		if(CdrTestFlag)
			printf("\n(tick:%d)rec msg type:%d\n",tickGet(),buf[0]);
		switch(buf[0])
		{
			case From15MinTimer:
				CdrDataFileUploadTimer();
				/*if(CdrTestFlag)
				 printf("\nFrom15MinTimer......\n");*/
				break;
			case From1MinInterval:
			case FromMoveAway:
				index = buf[1]*256+buf[2];
				if(index>=M_MAX_UT_PER_BTS)
					//printf("\n write cdr file task rec unkown cdr index:%x\n",index);
					LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\n write cdr file task rec unkown cdr index:%x\n",index);
				else
				{
					//if(!(cdrTbl[index].cdr.Data_flowup[1]==0&&cdrTbl[index].cdr.Data_flowdn[1]==0))
					//以上行流量为触发
					if(!(cdrTbl[index].cdr.Data_flowup[1]==0))
					{
						//semTake (cdrSemId, WAIT_FOREVER);
						WriteToFile(&cdrTbl[index].cdr,index);
						//cdrTbl[index].lastPktTick = 0;
						//semGive (cdrSemId);
					}
				}
				 if(CdrTestFlag)
				 	printf("\nFrom1MinInterval,index:%x\n",index);
				break;
//#ifdef LJF_BILLING_VOICE
			case FromVoiceInfo:
				LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_PARAMETER ),"writeFileMainloop()::FromVoiceInfo\n ");
				WriteVoiceToFile( stRcv->pucAdd, stRcv->ulLen );
				delete [] stRcv->pucAdd;
				break;
			case FromDelEid:
				index = buf[1]*256+buf[2];
				if(index>=M_MAX_UT_PER_BTS)
					//printf("\n write cdr file task rec unkown cdr index:%x\n",index);
					LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\n write cdr file task rec unkown cdr index:%x\n",index);
				else
				{
					
					if(CdrTestFlag)
						LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\n write cdr file task rec FromDelEidcdr index:%x,eid:0x%x\n",index,cdrTbl[index].cdr.ulEid);
					//if(!(cdrTbl[index].cdr.Data_flowup[1]==0&&cdrTbl[index].cdr.Data_flowdn[1]==0))
					//以上行流量为触发
					if(cdrTbl[index].ucMacNum)
					{
						if(CdrTestFlag)
							LOG2( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\n write cdr file task rec FromDelEidcdr ucMacNum:%x,eid:0x%x\n",cdrTbl[index].ucMacNum,cdrTbl[index].cdr.ulEid);
						break;
					}
					if(!(cdrTbl[index].cdr.Data_flowup[1]==0))
					{

                                         if(cdrTbl[index].flag_time&&real_time_switch&&cdrTbl[index].flag_limit)
						{
								CTBridge *taskEB = CTBridge::GetInstance();
								taskEB->func_RTCharge_rpt(index,RPT_NO_FLOW);
						}
						else
						{
							//semTake (cdrSemId, WAIT_FOREVER);
							WriteToFile(&cdrTbl[index].cdr,index);
							//cdrTbl[index].lastPktTick = 0;
						}
						//semGive (cdrSemId);
					}
					
					 semTake (cdrSemId, WAIT_FOREVER);
					 if(cdrTbl[index].wifi_eid_flag==FLAG_YES)
					 {
					       CMac wifiMac(cdrTbl[index].cdr.MAC);
					 	BPtreeDel(wifiMac);
					 }
					 else
					 {
					 	BPtreeDel(cdrTbl[index].cdr.ulEid);
						BPtreeDelbyUid(cdrTbl[index].cdr.ulUid);
					 }
					 //semTake (cdrSemId, WAIT_FOREVER);
					 memset(&(cdrTbl[index]),0,sizeof(CDR));
				        InsertFreeCDRList(index);
					semGive (cdrSemId);

				}
				break;
			case FromSendCfToFtp:
				sendCfFileToFtp();
				break;
				
			default:
				//printf("\n write cdr file task rec unkown msg:%x\n",buf[0]);
				LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\n write cdr file task rec unkown msg:%x\n",buf[0]);
				break;
		}
		 gFlagInFtp = FLAG_NO;
		 if(CdrTestFlag)
			printf("\n(tick:%d)have handle msg type:%d,sec:%d\n",tickGet(),buf[0],ftpSecond);
		 ftpSecond = 0;

	}

 }


UINT32 changeTimetoSec(int y,int mon,int d,int h,int m,int s)
{


    struct tm time_s;
    UINT32 secCount;


    time_s.tm_sec = s;
    time_s.tm_min = m;
    time_s.tm_hour = h;
    time_s.tm_mday = d;
    time_s.tm_mon = mon -1;
    time_s.tm_year = y - 1900;
    time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

    secCount = mktime(&time_s);

    /*
    secCount+=splus;
    

	    time_t tm = secCount;
    struct tm *time = ::localtime( &tm );
	printf("\ntest year:%d,mon:%d,day:%d,h:%d.m:%d.s:%d\n",time->tm_year,\
		time->tm_mon,time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec);*/
     return secCount;

}

void CCDR::countDurationAndStoptime(T_TimeDate*pLastPktTime,
	CdrTime*pCdrStartTime,UINT16 &index,UINT16 dltSec)
	
{
          if(!(index<M_MAX_UT_PER_BTS))
		return ;
	   UINT32 secLast,secStart;
	   secLast = changeTimetoSec(pLastPktTime->year,pLastPktTime->month,pLastPktTime->day,pLastPktTime->hour,\
	   	pLastPktTime->minute,pLastPktTime->second);
	   secStart =changeTimetoSec((pCdrStartTime->year[0]*100+pCdrStartTime->year[1]),pCdrStartTime->month,\
	   	pCdrStartTime->day,pCdrStartTime->hour,pCdrStartTime->minute,pCdrStartTime->second);
	   if(secLast>=secStart)
	   {
	   	 cdrTbl[index].cdr.Duration = (secLast-secStart)+dltSec;
         //new stat
	     stat_addTimeLong(cdrTbl[index].cdr.Duration);
         
		 time_t tm = secLast +dltSec ;
	        struct tm *time = ::localtime( &tm );
		 cdrTbl[index].cdr.stStopTime.year[0] = (time->tm_year+1900)/100;
		 cdrTbl[index].cdr.stStopTime.year[1] = (time->tm_year+1900)%100;
		 cdrTbl[index].cdr.stStopTime.month =( time->tm_mon+1);
		 cdrTbl[index].cdr.stStopTime.day = time->tm_mday;
		 cdrTbl[index].cdr.stStopTime.hour = time->tm_hour;
		 cdrTbl[index].cdr.stStopTime.minute = time->tm_min;
		 cdrTbl[index].cdr.stStopTime.second = time->tm_sec;
	   }

}

void CCDR::func_moveAway_Rt_cdr(UINT16 index)
{
          if(!(index<M_MAX_UT_PER_BTS))
		return ;
	 countDurationAndStoptime(&cdrTbl[index].lastPtTime,&cdrTbl[index].cdr.stStartTime,index,0);
         if(cdrTbl[index].flag_limit==0)
         {
         	sendMsgToWriteFileTask(FromMoveAway,index);
        	return;
         }
	//countDurationAndStoptime(&cdrTbl[index].lastPtTime,&cdrTbl[index].cdr.stStartTime,index,0);
	CTBridge *taskEB = CTBridge::GetInstance();
	taskEB->func_RTCharge_rpt(index,RPT_MOVE_AWAY);
	deleteCdrInform(index);
}

void CCDR::spyCdrMacTbl()
{
	   semTake (cdrSemId, WAIT_FOREVER);
	    map<CMac, UINT16>::iterator it_mac = BTreeByMAC.begin();
	    while ( BTreeByMAC.end() != it_mac)
	    {
	        UINT16 index = ( it_mac->second );
		if(index<M_MAX_UT_PER_BTS)
		{
			if(cdrTbl[index].cdr.wifi_UID==0xffffffff)
				cdrTbl[index].wifi_eid_sec++;
			else if(cdrTbl[index].cdr.wifi_UID!=0)
				cdrTbl[index].wifi_eid_sec = 0;
			if(cdrTbl[index].wifi_eid_sec==180)//3//三分钟
			{
				if(CdrTestFlag)
					printf("\ncdr eid:0x%x,wifi uid 0x%x, ip:0x%x,time out\n",cdrTbl[index].cdr.ulEid,cdrTbl[index].cdr.wifi_UID,cdrTbl[index].cdr.wifi_IP);
				 map<CMac, UINT16>::iterator it_mac_t = it_mac;
				 ++it_mac_t;
				 BTreeByMAC.erase( it_mac );
				 memset(&(cdrTbl[index]),0,sizeof(CDR));
			        InsertFreeCDRList(index);
			        it_mac = it_mac_t;
			        continue;
			}
		}
		++it_mac;
	    }
	    semGive (cdrSemId);
}

void CCDR::delWifiUser()
{
	   if(CdrTestFlag)
		  printf("\ncb3000 link down del all wifi user\n");
	    semTake (cdrSemId, WAIT_FOREVER);
	    map<CMac, UINT16>::iterator it_mac = BTreeByMAC.begin();
	    while ( BTreeByMAC.end() != it_mac)
	    {
	        UINT16 index = ( it_mac->second );
		if(index<M_MAX_UT_PER_BTS)
		{
			        map<CMac, UINT16>::iterator it_mac_t = it_mac;
				 ++it_mac_t;
				 BTreeByMAC.erase( it_mac );
				 memset(&(cdrTbl[index]),0,sizeof(CDR));
			        InsertFreeCDRList(index);
			        it_mac = it_mac_t;
			        continue;
		}
		++it_mac;
	    }
	    semGive (cdrSemId);
}


/*
			字段名称	长度（Byte）	类型	描述
				MESSAGE TYPE	2	M	0x03，心跳请求
				User1 session ID	4	M	
				User1 MAC	6	M	
				User1 Wifi UID	4	M	
				……			
				User session ID	4	M	
				User MAC	6	M	
				User Wifi UID	4	M	

*/
void CCDR::func_heart_pro(char *p)
{
      UINT16 len= 0,num = 0;
	int i = 0;
	char *p1 =NULL; 
	UINT32 wifi_uid = 0;
      
			memcpy((char*)&len,(p+4),2);
			if(len<6)
			{
		       	if(CdrTestFlag)
					LOG1( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "rec heart len:%d.err",len);
				return;
			}
			num = (len - 2)/14;
			p+= 8;
		      semTake (cdrSemId, WAIT_FOREVER);
			map<CMac, UINT16>::iterator it_mac = BTreeByMAC.begin();
		    while ( BTreeByMAC.end() != it_mac)
		    {
		        UINT16 index = ( it_mac->second );
			if(index<M_MAX_UT_PER_BTS)
			{
				p1 = p;
			       for( i = 0;i<num;i++)
			       {
			       	memcpy(&wifi_uid,(p1+10),4);
					if(wifi_uid==getWifiUid(index))
						break;/*find*/
					p1+=14;
				}
				if(i==num)
				{
					
					//如果没有业务直接删除，否则发送消息到cdr写文件任务
					 if((cdrTbl[index].cdr.Data_flowup[1]==0)&&(cdrTbl[index].cdr.Data_flowdn[1]==0))
					 {
					       map<CMac, UINT16>::iterator it_mac_t = it_mac;
					        ++it_mac_t;
						 BTreeByMAC.erase( it_mac);
						 memset(&(cdrTbl[index]),0,sizeof(CDR));
					        InsertFreeCDRList(index);
					        it_mac = it_mac_t;
					        continue;
					 }
					 //WriteToFile(&(cdrTbl[cdrIndex].cdr));
					 else
					 {
					 	countDurationAndStoptime(&cdrTbl[index].lastPtTime,&cdrTbl[index].cdr.stStartTime,index,0);
						sendMsgToWriteFileTask(FromDelEid,index);
					 }
					 if(CdrTestFlag)
					 	LOG1( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "cb3000 have no uid:0x%x,delete cdr",cdrTbl[index].cdr.wifi_UID);
				}

			}
			++it_mac;
		    }
		    semGive(cdrSemId);
			if(CdrTestFlag)
					LOG1( LOG_CRITICAL, LOGNO( EB, EC_EB_NORMAL ), "rec heart len:%d.",len);
}

void CCDR::showCdr(UINT32 ulEID)
{
    if ( 0 == ulEID )
    {
    	printf( "\r\nAll cdr table information" );
    }
    else
    {
    	//printf( "\r\nEID[0x%.8X] cdr table information*", ulEID );
    }
    if(0==ulEID)
    {
    	    semTake (cdrSemId, WAIT_FOREVER);
	    map<UINT32, UINT16>::iterator it = BTreeByEID.begin();
	    while ( BTreeByEID.end() != it )
	    {
	        UINT16 index = ( it->second );
		if(index<M_MAX_UT_PER_BTS)
			printf("\ncdr eid 0x%x,have mac num:%d,no_charge_flag:%d,wifi uid 0x%x\n",cdrTbl[index].cdr.ulEid,cdrTbl[index].ucMacNum,cdrTbl[index].noPayflag,cdrTbl[index].cdr.wifi_UID);
		++it;
	    }
	    
	    map<CMac, UINT16>::iterator it_mac = BTreeByMAC.begin();
	    while ( BTreeByMAC.end() != it_mac)
	    {
	        UINT16 index = ( it_mac->second );
		if(index<M_MAX_UT_PER_BTS)
		{
			UINT8 *p = cdrTbl[index].cdr.MAC;
			printf("\ncdr eid:0x%x,wifi uid 0x%x, ip:0x%x,mac: %x-%x-%x-%x-%x-%x\n",cdrTbl[index].cdr.ulEid,cdrTbl[index].cdr.wifi_UID,cdrTbl[index].cdr.wifi_IP,p[0],p[1],p[2],p[3],p[4],p[5]);
		}
		++it_mac;
	    }
	    semGive (cdrSemId);
    }
    else if( 1 == ulEID )
    {
	 for ( UINT16 usIdx = 0; usIdx <M_MAX_UT_PER_BTS; ++usIdx )
	 {
	 	//判断是否存在数据
		if(!(cdrTbl[usIdx].cdr.Data_flowup[1]==0&&cdrTbl[usIdx].cdr.Data_flowdn[1]==0))
		{
			printf("\nEid:0x%x,flowup:0x%x,flowdn:0x%x,starttime:%d-%d-%d  %d:%d:%d,Uid:0x%x,Id:0x%x\n",cdrTbl[usIdx].cdr.ulEid,\
				cdrTbl[usIdx].cdr.Data_flowup[1],cdrTbl[usIdx].cdr.Data_flowdn[1],\
				(cdrTbl[usIdx].cdr.stStartTime.year[0]*100+cdrTbl[usIdx].cdr.stStartTime.year[1]),
				cdrTbl[usIdx].cdr.stStartTime.month,cdrTbl[usIdx].cdr.stStartTime.day,cdrTbl[usIdx].cdr.stStartTime.hour,\
				cdrTbl[usIdx].cdr.stStartTime.minute,cdrTbl[usIdx].cdr.stStartTime.second,\
				cdrTbl[usIdx].cdr.ulUid,cdrTbl[usIdx].cdr.Id
				);
			
			if(real_time_switch)
				printf("wifi:%d,limit:%d,final:%d,time:%d,traffic:%d,over:%d,usIdx:%d,id_L:%d,flow:%d,%d\n",cdrTbl[usIdx].wifi_eid_flag,cdrTbl[usIdx].flag_limit,cdrTbl[usIdx].flag_final,cdrTbl[usIdx].flag_time,cdrTbl[usIdx].flag_traffic,cdrTbl[usIdx].flag_over,usIdx,cdrTbl[usIdx].cdr_id_little,cdrTbl[usIdx].data_flow_remain,cdrTbl[usIdx].time_remain);
		}
	 }
	 printf("\ncdr search finish.......\n");

    }
    else
    {
		UINT16 index = BPtreeFind(ulEID);
		if(index>=M_MAX_UT_PER_BTS)
		{
			printf("\nEID 0x%x no cdr\n",ulEID);
		}
		else
		{
			printf("\nEid:0x%x,flowup:0x%x,flowdn:0x%x,starttime:%d-%d-%d  %d:%d:%d\n",cdrTbl[index].cdr.ulEid,\
				cdrTbl[index].cdr.Data_flowup[1],cdrTbl[index].cdr.Data_flowdn[1],\
				(cdrTbl[index].cdr.stStartTime.year[0]*100+cdrTbl[index].cdr.stStartTime.year[1]),
				cdrTbl[index].cdr.stStartTime.month,cdrTbl[index].cdr.stStartTime.day,cdrTbl[index].cdr.stStartTime.hour,\
				cdrTbl[index].cdr.stStartTime.minute,cdrTbl[index].cdr.stStartTime.second);
			printf("\ncdr1 eid 0x%x,have mac num:%d,no_charge_flag:%d,wifi uid 0x%x\n",cdrTbl[index].cdr.ulEid,cdrTbl[index].ucMacNum,cdrTbl[index].noPayflag,cdrTbl[index].cdr.wifi_UID);
			
		}
    }


    return ;
}

void CCDR::cdrMacAdd(UINT32 & uiEid)
{
//先判断是否是wifi_eid
	if(eidIsWifiEid(uiEid)==true)
	{
		return;
	}
	 UINT16 cdrIndex = BPtreeFind(uiEid);
	 if(cdrIndex==M_DATA_INDEX_ERR)
	 {
		cdrIndex = GetFreeCDRIdxFromList();
	 	BPtreeAdd(uiEid, cdrIndex);
	 }
	 if(cdrIndex<M_MAX_UT_PER_BTS)
	 	cdrTbl[cdrIndex].ucMacNum++;
	// if(cdrTest)
	 	//printf("\ncdrMacAdd eid:0x%x,index:%d\n",uiEid,cdrIndex);
	if(CdrTestFlag)
		LOG2( LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "\ncdrMacAdd eid:0x%x,index:%d\n",uiEid,cdrIndex );
	

}

bool CCDR::writeFileToCF( char* chFileName )
{
	    FILE *stream;
	   // char buf[512];
	    int        nBytes;
	    char chTmp[CDR_ABSTRACT_FILENAME_LEN];
 
	     DIR* pdir = opendir( CF_CDR_DIR);
	    if ( NULL == pdir )
	    {
	        	if(OK!=mkdir(CF_CDR_DIR))
			{
				//printf("\n mkdir:%s fail...\n",CF_CDR_DIR);
				LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\n mkdir:%s fail...\n",(int)CF_CDR_DIR);
				return false;
			}
			closedir( pdir );
	    }
	    else
	        closedir( pdir );

		
	       stream = fopen( GetCFAbstractFile(chFileName, chTmp), "ab" );
		if( stream != NULL )
		{

		
		    FILE * pFile = fopen( GetAbstractFile( chFileName, chTmp ), "rb");
		    if( NULL == pFile )
		    {
					printf("\nwrite file to cf ,but cannot open  file:%s",chTmp);
			 fclose( stream );
		        return false;
		    }
		
		    memset(ftpbuf, 0, sizeof(ftpbuf));
		    while( (nBytes = fread(ftpbuf, 1, 512, pFile)) > 0 )
		    {
		        fwrite( ftpbuf, sizeof( char ), nBytes, stream );
		        memset(ftpbuf, 0, sizeof(ftpbuf));
		    }

		    fclose(pFile);
		
		    fclose( stream );

		   return true;
		}
		else
		{
			//printf("\ncannot open cdf file:%s\n",chTmp);
			LOG1( LOG_DEBUG3, LOGNO( EB, EC_EB_PARAMETER ),"\ncannot open cdf file:%s\n",(int)chTmp);
			return false;
		}

				
}

//每次最多只发送10条cdr，避免同时数据量太大
//只要发送一次失败则不在发送
void CCDR::sendCfFileToFtp()
{
	//char chTmp[CDR_ABSTRACT_FILENAME_LEN];
    char file[128];
	int sendCdrNum = 0;
	int i ;
	for( i = 0;i<MAX_SAVE_CDR_NUM;i++)
	{
		if(strlen(gpNVRamCdrName[i].name)!=0)
		{
            memcpy(file,gpNVRamCdrName[i].name,sizeof(gpNVRamCdrName[i].name));
			if(!(SendCFCdrFile(file)))
			 {
				 return;
			 }
			 //将旧文件删除，
			 RemoveCfFile(file);
			 
			 if(bspEnableNvRamWrite( (char*)gpNVRamCdrName, NVRamCdrNameLength)==TRUE)
		 	{
			     memset(gpNVRamCdrName[i].name,0,sizeof(gpNVRamCdrName[i].name));
		            //关闭
		            bspDisableNvRamWrite( (char*)gpNVRamCdrName, NVRamCdrNameLength );
		 	}
//#ifdef LJF_BILLING_VOICE
			 sendCdrNum++;
		}
		if(strlen(gpNVRamVcdrName[i].name)!=0)
		{
			//LOG1( LOG_SEVERE, LOGNO( EB, EC_EB_PARAMETER ),);
			//printf( "sendCfFileToFtp()::VcdrName[%s]\r\n", gpNVRamVcdrName[i].name );
			#ifdef LJF_BILLING_VOICE
			memcpy(file,gpNVRamVcdrName[i].name,sizeof(gpNVRamVcdrName[i].name));
			#else
			memcpy(file,gpNVRamVcdrName[i].name,sizeof(gpNVRamVcdrName[i].name));
			#endif
			if(!(SendCFCdrFile(file)))
			 {
				 return;
			 }
			 //将旧文件删除，
			 RemoveCfFile(file);
			 
			 if(bspEnableNvRamWrite( (char*)gpNVRamCdrName, NVRamCdrNameLength)==TRUE)
		 	{
			     memset(gpNVRamVcdrName[i].name,0,sizeof(gpNVRamVcdrName[i].name));
		            //关闭
		            bspDisableNvRamWrite( (char*)gpNVRamCdrName, NVRamCdrNameLength );
		 	}
			 sendCdrNum++;
		}
			 if(sendCdrNum==10)
			 	break;
#if 0//ndef LJF_BILLING_VOICE
		}
#endif
		
	}

	if(i==MAX_SAVE_CDR_NUM)
		gNeedSendCFFileToFtp = FLAG_NO;
	
}

bool CCDR :: SendCFCdrFile( char* chFileName )
{
    if( NULL == chFileName )
        return false;

    int        ctrlSock;
    int        dataSock;
    int        nBytes;
    char    buf [512];
    STATUS    status;

    struct in_addr Svriaddr;
    Svriaddr.s_addr = g_tFTPInfo.uIPAddr;
    SINT8    IpAddr[16];
    memset( IpAddr, 0, sizeof(IpAddr) );
    inet_ntoa_b( Svriaddr, IpAddr );

    char chTmp[CDR_ABSTRACT_FILENAME_LEN];
    FILE * pFile = fopen( GetCFAbstractFile( chFileName, chTmp ), "rb");
    if( NULL == pFile )
    {
    	LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\nsend cf file to ftp ,but cannot open  file:%s",(int)chTmp);
	
	//cf卡内文件打不开，则删除nvram里面的文件名
        //return false;
        return true;
    }
	if( ERROR == ftpXfer( IpAddr, 
                          g_tFTPInfo.chUserName, 
                          g_tFTPInfo.chPassWord, 
                          NULL, 
                          "STOR %s", 
                          CDR_UPLOAD_DIR, 
                          chFileName, 
                          &ctrlSock, 
                          &dataSock) )
   /* if( ERROR == ftpXfer( "172.16.8.219", 
                          "l3", 
                          "l3", 
                          NULL, 
                          "STOR %s", 
                          CDR_UPLOAD_DIR, 
                          chFileName, 
                          &ctrlSock, 
                          &dataSock) )*/
    {
        LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "FILE UPLOAD ERROR." );
        fclose(pFile);
		if(CdrTestFlag)
			printf("\nsend cf file to ftp ,but cannot open  route file:%s",chTmp);
        return false;
    }

    //lijinan 090112
   //gFlagInFtp = FLAG_YES;
    memset(buf, 0, sizeof(buf));
    while( (nBytes = fread(buf, 1, 512, pFile)) > 0 )
    {
        ::write (dataSock, buf, nBytes );
        memset(buf, 0, sizeof(buf));
    }

    fclose(pFile);
    close (dataSock);

    if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
    {
        status = false;
    }
    if (ftpCommand (ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
    {
        status = false;
    }
    close (ctrlSock);
    		if(CdrTestFlag)
			printf("\nsend cf file to ftp succ\n");
   // gFlagInFtp = FLAG_NO;
    return true;
}


bool CCDR :: RemoveCfFile( char* chFileName )
{
    DIR* pdir = opendir( CF_CDR_DIR );
    if( NULL != pdir )
    {
        closedir( pdir );
        char chTmp[CDR_ABSTRACT_FILENAME_LEN];
        FILE * pFile = fopen( GetCFAbstractFile( chFileName, chTmp ), "rb");
        if( NULL != pFile )
        {
            fclose( pFile );
            remove( GetCFAbstractFile( chFileName, chTmp ) );
	     if(CdrTestFlag)
		printf("\nRemoveCfFile succ :%s\n",chTmp);
        }
	/*if(CdrTestFlag)
		printf("\nRemoveFile :%s\n",chTmp);*/
        return true;
    }
	
    if(CdrTestFlag)
		printf("\nRemoveCfFile,cannot open :%s\n",CDR_FILE_DIR);
    LOG1( LOG_WARN, LOGNO( EB, EC_EB_PARAMETER ),"\nRemoveCfFile,cannot open :%s\n",(int)CDR_FILE_DIR);
}


void CCDR::checkNvramCdrFile()
{
    stCdrNVRAMhdr *nvramHeader = (stCdrNVRAMhdr*)NVRAM_CDR_BASE;
    int * cdrNvRamEnd = (int *)((char *)gpNVRamCdrName+NVRamCdrNameLength);
    if (    M_CDR_NVRAM_INITIALIZED != nvramHeader->Initialized 
         || bspGetBtsID() != nvramHeader->LocalBtsId||(nvramHeader->cdrOnOff>1)||*cdrNvRamEnd!= M_CDR_NVRAM_INITIALIZED)
    {  

	 if(bspEnableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength))==TRUE)
	{
	       if(NvRamDataAddr->stCdr_para.Initialized!=M_CDR_NVRAM_INITIALIZED||bspGetBtsID() != NvRamDataAddr->stCdr_para.LocalBtsId||(NvRamDataAddr->stCdr_para.cdrOnOff>1))
	       {
	       
		     cdr_para_err_flag = 1;
		     bspDisableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength));
		     return;
		     //cdr para 告警
		}
		else
		{
			memcpy((char*)nvramHeader,(char*)&NvRamDataAddr->stCdr_para.Initialized,sizeof(stCdrNVRAMhdr));
			*cdrNvRamEnd = M_CDR_NVRAM_INITIALIZED;
	    	       memset((char *)gpNVRamCdrName,0,NVRamCdrNameLength);
			bspDisableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength));
		}

	 }
	 
	#if 0
        if(bspEnableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength))==TRUE)
	{
            nvramHeader->Initialized = M_CDR_NVRAM_INITIALIZED;
            nvramHeader->LocalBtsId = bspGetBtsID();
    	     nvramHeader->cdrOnOff = 0;
	     nvramHeader->cdrPeriod = 15;
	    *cdrNvRamEnd = M_CDR_NVRAM_INITIALIZED;
	     nvramHeader->IP_CB3000= 0xffffffff;
    	     memset((char *)gpNVRamCdrName,0,NVRamCdrNameLength);
            bspDisableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength));
    	}
        return;
	#endif
    }

     uiCdrSwitch = nvramHeader->cdrOnOff;
     IP_CB3000 = nvramHeader->IP_CB3000;
     if(nvramHeader->cdrPeriod>0)
     	gcdrSendPeriod =( nvramHeader->cdrPeriod*60);
     

   	for(int  i = 0;i<MAX_SAVE_CDR_NUM;i++)
	{
		if(strlen(gpNVRamCdrName[i].name)!=0)
		{
			//printf("\nnvram have cdr file :%s\n",gpNVRamCdrName[i].name);
			LOG1( LOG_DEBUG3, LOGNO( EB, EC_EB_PARAMETER ),"\nnvram have cdr file :%s\n",(int)gpNVRamCdrName[i].name);
			gNeedSendCFFileToFtp = FLAG_YES;
			break;
		}
		
	}

}

int cdrtest()
{
	T_TimeDate lstTm = bspGetDateTime();
	 printf("\ntest year:%d,mon:%d,day:%d,h:%d.m:%d.s:%d\n",lstTm.year,lstTm.month,\
	 	lstTm.day,lstTm.hour,lstTm.minute,lstTm.second);
	
	UINT32 sec = changeTimetoSec(lstTm.year,lstTm.month,lstTm.day,lstTm.hour,lstTm.minute,lstTm.second);
	
	 time_t tm = sec - 150 ;
        struct tm *time = ::localtime( &tm );
	 printf("\ntest year:%d,mon:%d,day:%d,h:%d.m:%d.s:%d\n",(time->tm_year+1900),\
		(time->tm_mon+1),time->tm_mday,time->tm_hour,time->tm_min,time->tm_sec);
}



void cdrShow(UINT32 eid)
{
    CTBridge *taskEB = CTBridge::GetInstance();
    taskEB->ShowCdr(eid);

     return;
}

//关于计费的开关
void closeCdr()
{
	uiCdrSwitch = 0;
	stCdrNVRAMhdr *nvramHeader = (stCdrNVRAMhdr*)NVRAM_CDR_BASE;


      if(bspEnableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength))==TRUE)
  	{

	 	nvramHeader->cdrOnOff = 0;
            bspDisableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength));
      	}

	

}
void openCdr()
{
	uiCdrSwitch = 1;
	stCdrNVRAMhdr *nvramHeader = (stCdrNVRAMhdr*)NVRAM_CDR_BASE;


      if(bspEnableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength))==TRUE)
      	{

	     nvramHeader->cdrOnOff = 1;
            bspDisableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)+NVRamCdrNameLength));
      	}


}

//计费上报周期控制
void setCdrSendPeriod(UINT32 period)
{
	if(period>60)
		gcdrSendPeriod  = period;

}

void setCdrPara(UINT8 onoff,UINT8 period,UINT32 Ip)
{
     if((period<4)||(onoff>1))
     	{
     		printf("\ncdr onoff must be 0 or 1,period must>3 mins\n");
	 	return;
     	}
	 stCdrNVRAMhdr *nvramHeader = (stCdrNVRAMhdr*)NVRAM_CDR_BASE;
     if(bspEnableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)))==TRUE)
  	{

	     nvramHeader->cdrOnOff = onoff;
	     nvramHeader->cdrPeriod = period;
	     nvramHeader->IP_CB3000 = Ip;
	     
	        bspDisableNvRamWrite( (char*)nvramHeader,  (sizeof(stCdrNVRAMhdr)));
  	}
	uiCdrSwitch = onoff;
	gcdrSendPeriod = period*60;
	IP_CB3000 = Ip;
}

void showCdrPara()
{
	printf("\n cdr para onoff:%x,period:%x sec,CB3000_IP:%x,flag:%d\n",uiCdrSwitch,gcdrSendPeriod,IP_CB3000,cdr_para_err_flag);
}
//lijinan 20081205 计费系统增加

//new stat
/*
* 话务统计开始，计算和计费的偏差
*/
void CCDR::newStatBegin()
{   
    memset(&billingNewStat, 0, sizeof(stBilling_newStat));    
    stat_countTimeLeft(STAT_PLUS);    
}

void CCDR::stat_countTimeLeft(STAT_DATA tt)
{   
  
    T_TimeDate tTimeData;
    UINT32 secLast,secStart;
    UINT32 ultemp;
    
    map<UINT32, UINT16>::iterator it = BTreeByEID.begin();
    semTake (cdrSemId, WAIT_FOREVER);
    tTimeData = bspGetDateTime(); 
    secLast = changeTimetoSec(tTimeData.year,tTimeData.month,tTimeData.day,tTimeData.hour,\
        tTimeData.minute,tTimeData.second);    
    while ( BTreeByEID.end() != it )
    {
        UINT16 index = ( it->second );
        if(cdrTbl[index].cdr.Data_flowup[1]==0)//没有流量，继续下一个
        {
            ++it;
            continue;
        }
        secStart =changeTimetoSec((cdrTbl[index].cdr.stStartTime.year[0]*100+cdrTbl[index].cdr.stStartTime.year[1]),\
			cdrTbl[index].cdr.stStartTime.month, cdrTbl[index].cdr.stStartTime.day,cdrTbl[index].cdr.stStartTime.hour,\
                     cdrTbl[index].cdr.stStartTime.minute,cdrTbl[index].cdr.stStartTime.second);
        if(secLast>=secStart)
        {
            ultemp = secLast-secStart;
            stat_addData(ultemp, tt);            
        }
	    ++it;
    }    
    semGive (cdrSemId); 
    
}
/*
*统计数据，注意溢出
*/
void  CCDR::stat_addData(UINT32 data, STAT_DATA tt)
{
    UINT32 *pHigh, *pLow;
    UINT32 temp;
	
    switch(tt)
    {
        case STAT_PLUS:	     
            pHigh = &billingNewStat.plusTimeHigh;
            pLow = & billingNewStat.plusTimeLow;
            break;
        case STAT_DATA_FLOW:
            pHigh = &billingNewStat.dataflowHigh;
            pLow = &billingNewStat.dataflowLow;
            break;
        case STAT_TIMELONG:
            pHigh = &billingNewStat.timelongHigh;
            pLow = &billingNewStat.timelongLow;	    
            break;
        default:
            return;    
    }    
    *pLow += data;    
    if(*pLow<data)//overflow
    {
        *pHigh++;        
    }
}
/*统计数据流量*/
void CCDR::stat_addDataFlow(UINT32 data)
{    
    stat_addData(data, STAT_DATA_FLOW);
}
/*统计时长
上报时长=上报结束时统计的数据+计费写文件时统计数据-上报开始时统计数据
*/
void CCDR::stat_addTimeLong(UINT32 data)
{
    stat_addData(data, STAT_TIMELONG);
}
/*
*统计周期结束，计算真正时长
*/
void CCDR::newStatEnd(UINT8 *pdata, UINT8 *ptimelong)
{    
    //统计数据流量
    memcpy(pdata, &billingNewStat.dataflowHigh, 4);
    memcpy(pdata+4, &billingNewStat.dataflowLow, 4);
    //计算时长
    stat_countTimeLeft(STAT_TIMELONG);
    if((billingNewStat.timelongHigh<billingNewStat.plusTimeHigh)\
    ||((billingNewStat.timelongLow<billingNewStat.plusTimeLow)\
    &&(billingNewStat.timelongHigh==billingNewStat.plusTimeHigh)))
    {
        printf("newStatEnd, time count error!!!!1\n");
    }
    else
    {
        if(billingNewStat.timelongLow>=billingNewStat.plusTimeLow)
        {
            billingNewStat.timelongLow -= billingNewStat.plusTimeLow;
            billingNewStat.timelongHigh -= billingNewStat.plusTimeHigh;
        }
        else
        {
          
            billingNewStat.timelongLow += 0xffffffff - billingNewStat.plusTimeLow + 1;
             billingNewStat.timelongHigh -= (billingNewStat.plusTimeHigh + 1);
        }
    }
    memcpy(ptimelong, &billingNewStat.timelongHigh, 4);
    memcpy(ptimelong+4, &billingNewStat.timelongLow, 4);
    memset(&billingNewStat, 0, sizeof(stBilling_newStat));
}


extern "C" void statBegin()
{
    CTBridge* ptask = CTBridge::GetInstance();
    CCDR* pCdr = ptask->GetCdrInstance();    
    pCdr->newStatBegin();  
}
extern "C" void statEnd(UINT8 *pdata, UINT8 *ptimelong)
{
    CTBridge* ptask = CTBridge::GetInstance();
    CCDR* pCdr = ptask->GetCdrInstance();    
    pCdr->newStatEnd(pdata, ptimelong);   
}

extern unsigned char  Local_Sac_Mac_Addr[5][6];
UINT8 compwithLocalMac(UINT8 * macaddr)
{        
    for(int i=0; i<5; i++)
    {
        if ((macaddr[0] == Local_Sac_Mac_Addr[i][0])&& (macaddr[1] == Local_Sac_Mac_Addr[i][1] ) \
   	 	&&(macaddr[2] == Local_Sac_Mac_Addr[i][2]) &&( macaddr[3] == Local_Sac_Mac_Addr[i][3]) \
                   &&(macaddr[4] == Local_Sac_Mac_Addr[i][4]) &&(macaddr[5] == Local_Sac_Mac_Addr[i][5]))
     	 {
     	     return  2;
     	 }
    }
    return 0;
}

 char  CTBridge::getMacType(CComMessage* pComMessge,UINT32* srcIP)
{
  // char type = 0xff;
   ArpHdr *pArphdr;
   EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );

     if ((pEtherHdr->aucDstMAC[0] == m_btsMac[0])&& (pEtherHdr->aucDstMAC[1] == m_btsMac[1] ) &&(pEtherHdr->aucDstMAC[2] == m_btsMac[2]) &&
	 ( pEtherHdr->aucDstMAC[3] == m_btsMac[3]) &&
	   (pEtherHdr->aucDstMAC[4] == m_btsMac[4]) &&(pEtherHdr->aucDstMAC[5] == m_btsMac[5]))
     	{
     	     return 0;
     	}
	 else
	 {
	 	   if(IS_8023_PACKET(ntohs(pEtherHdr->usProto)))
		    {
		        pArphdr = (ArpHdr*)((UINT8*)pEtherHdr + sizeof(EtherHdr) + sizeof(LLCSNAP));
		    }
		    else
		    {
		        pArphdr = (ArpHdr*)((UINT8*)pEtherHdr + sizeof(EtherHdr));
		    }
	 	    *srcIP=pArphdr->ulSenderPaddr;
	 	    return 1;
	 }
    // return type;
}

void CTBridge::recordCPEToBTSIPStack( UINT8 *srcMac,UINT32 eid,UINT32 srcIP)
{
     memcpy(m_cpe_to_myBTS_Mac,(char*)srcMac,6);
     m_cpe_to_myBTS_Eid =eid;
     m_cpe_to_myBTS_IP = srcIP;
     return;
}
UINT32 IsCPEVisitTrue=0;
UINT32 IsCPEVisit=0;
void CTBridge::IsCPEVisitBTSMacCallBack(char *Mac,char*result)
{
     IsCPEVisit ++;
     if ((m_cpe_to_myBTS_Mac[0] == Mac[0])&& (m_cpe_to_myBTS_Mac[1] == Mac[1] ) &&(m_cpe_to_myBTS_Mac[2] == Mac[2]) &&
	 ( m_cpe_to_myBTS_Mac[3] == Mac[3]) &&
	   (m_cpe_to_myBTS_Mac[4] == Mac[4]) &&(m_cpe_to_myBTS_Mac[5] == Mac[5]))
     	{
     	     *result =1 ;
     	     IsCPEVisitTrue ++;
     	}
     else if ((Mac[0]==(char)0xff))
        { //broadcast msg handle ,if is required ip 
            EtherHdr *pEtherPkt = (EtherHdr*)  Mac ;
	    ArpHdr *pArphdr;
	    if(IS_8023_PACKET(ntohs(pEtherPkt->usProto)))
	    {
	        pArphdr = (ArpHdr*)((UINT8*)pEtherPkt + sizeof(EtherHdr) + sizeof(LLCSNAP));
	    }
	    else
	    {
	        pArphdr = (ArpHdr*)((UINT8*)pEtherPkt + sizeof(EtherHdr));
	    }
	    
	    if (pArphdr->ulDestPaddr == m_cpe_to_myBTS_IP)
	    {
     	        *result =1 ;
     	        IsCPEVisitTrue ++;	    	
	    }
	    else
	        *result = 0;
        }	
    else
	 {
	 	
	     *result = 0;
	 }
    return;
}





UINT32 SendToBTSIPStackCount=0;
bool CTBridge::SendToBTSIPStack(CComMessage *pComMsg)
{

   SendToBTSIPStackCount ++;
    //增加计数
    pComMsg->AddRef();
    //发送WAN
    return mv643xxRcvMsgToBTSIPStk
        (
        (char*)pComMsg->GetDataPtr(),       //Data to send
        (UINT16)pComMsg->GetDataLength()+6,   //Data length
        CTBridge::EBFreeMsgCallBack,        //function.
        (UINT32)pComMsg                     //ComMessage ptr.
        );
}
#ifdef WBBU_CODE
extern "C" void Ebprint()
{
   printf("Rx:%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",egresscount,Procount,RxCount,Rxcount1,decount,decount1,L2_L3_Ingress,L2_L3_Wan,L2_L3_Rels,L3_L2_Send_err,L2_L3_Long,RX_Rls,Wanif_L3_IP,long_packet,discard_packet, Commsg_Send_err);
   //printf(")
}
extern "C" void CEbprint()
{
  egresscount = 0;
  Procount = 0;
  RxCount=0;
  Rxcount1 = 0;
  decount= 0;
  decount1=0;
  L2_L3_Ingress= 0;
  L2_L3_Wan=0;
  L2_L3_Rels = 0;
  Wanif_L3_IP = 0;
}

void CTBridge::send_packet_2_wan(unsigned char *pdata,unsigned short len)
{
  #if 0
    CComMessage* pMsg = new (this, len) CComMessage;
    if((pMsg!=NULL)&&(pdata!=NULL))
    	{
//	CMVoiceRegMsgT* pDataUnReg = (CMVoiceRegMsgT*)pUnRegMsg->GetDataPtr();
	pMsg->SetSrcTid(M_TID_EB);
	pMsg->SetDstTid(M_TID_EB);
	//pMsg->SetMessageId(MSGID_VOICE_UT_UNREG);
	memcpy((unsigned char*)pMsg->GetDataPtr(),pdata,len);
	    pMsg->AddRef();
    //发送WAN
      vxbEtsecRecvMsgFromEB( (char*)pMsg->GetDataPtr(),       //Data to send
        (UINT16)pMsg->GetDataLength(),   //Data length
        CTBridge::EBFreeMsgCallBack,        //function.
        (UINT32)pMsg                     //ComMessage ptr.
        );
       L2_L3_Eth_Packet++;
    	}
    #endif
	
}

extern "C" void send_2_wan(unsigned char *pdata,unsigned short len)
{
      CTBridge::GetInstance()->send_packet_2_wan(pdata,len);
}
void L3_L2_print()
{
   logMsg("L3_L2:%x,%x,%x\n",L3_L2_Eth_Packet,L3_L2_Eth_Packet_BC,L2_L3_Eth_Packet,1,2,3);
}
#endif
#ifdef RCPE_SWITCH
/*============================================================
MEMBER FUNCTION:
    CTBridge::trunkIsMRcpe

DESCRIPTION:
    判断一个pid是否为自中继mrcpe

ARGUMENTS:
	UINT32 ulPID
	
RETURN VALUE:
	true:是，false:否

SIDE EFFECTS:
    none
==============================================================*/
BOOL CTBridge::trunkIsMRcpe( UINT32 ulPID )
{
	if( 0 == ulPID )
		return false;
	for( UINT8 uc=0; uc<M_TRUNK_MRCPE_MAXNUM; uc++ )
	{
		if( ulPID == m_pstTrnukMRCpe->aulMRCpe[uc] )
			return true;
	}
	return false;
}
#endif

extern "C" void setStatPara(UINT16 flag, UINT16 len)
{
    if(flag!=0)
        statPrintFlag = 1;
    else
        statPrintFlag = 0;
    if(len!=0)
        statLen = len;
    else
        printf("len=0, error!!!!\n");
    statFromAir = 0;
    statFromWan = 0;
    statFromTDR = 0;
    statToAir = 0;
    statToWan = 0;
    statToTDR = 0;
    statPktFromAir = 0;
    statPktFromWan = 0;
    statPktFromTDR = 0;
    statPktToAir = 0;
    statPktToWan = 0;
    statPktToTDR = 0;
    
    statFromWanDrv = 0;
    statToWanDrv1 = 0;
    statToWanDrv2 = 0;
    statPktFromWanDrv = 0;
    statPktToWanDrv1 = 0;
    statPktToWanDrv2 = 0;
   
}
extern "C" void showStatPara()
{
    printf("stat time len is %d\n", statLen);
    printf("stat flag is %d\n", statPrintFlag);
}
extern "C" void clearStatData()
{
    statFromAir = 0;
    statFromWan = 0;
    statFromTDR = 0;
    statToAir = 0;
    statToWan = 0;
    statToTDR = 0;
    statPktFromAir = 0;
    statPktFromWan = 0;
    statPktFromTDR = 0;
    statPktToAir = 0;
    statPktToWan = 0;
    statPktToTDR = 0;
    #ifndef WBBU_CODE
    statFromWanDrv = 0;
    statToWanDrv1 = 0;
    statToWanDrv2 = 0;
    statPktFromWanDrv = 0;
    statPktToWanDrv1 = 0;
    statPktToWanDrv2 = 0;
    #endif
}
void printdmac()
{
     int i = 0;
     for(i = 0; i< 20; i++)
     {
        if((g_duplicate_mac[i].DebugEID[0]!=0)||(g_duplicate_mac[i].DebugEID[1]!=0))
        {
             printf("eid1:0x%x & eid2:0x%x may have same mac\n",g_duplicate_mac[i].DebugEID[0],g_duplicate_mac[i].DebugEID[1]);
	      printf("mac addr:%2x-%2x-%2x-%2x-%2x-%2x\n",g_duplicate_mac[i].MacAddress[0],g_duplicate_mac[i].MacAddress[1],g_duplicate_mac[i].MacAddress[2],g_duplicate_mac[i].MacAddress[3],g_duplicate_mac[i].MacAddress[4],g_duplicate_mac[i].MacAddress[5]);
        }
     }
}


