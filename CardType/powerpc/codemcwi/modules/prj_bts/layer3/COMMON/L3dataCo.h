/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    dataCommon.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   07/29/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __L3_DATACOMMON_H__
#define __L3_DATACOMMON_H__


/******************************
 *OP: ACL过滤时端口的比较操作符
 ******************************/
typedef enum 
{
    eq = 0,
    neq,
    lt,
    gt
}OP;


/*************************************************
 *IPTYPE: IP 类型；
 *************************************************/
typedef enum 
{
    IPTYPE_DHCP = 0,
    IPTYPE_PPPoE,
    IPTYPE_FIXIP,       //固定IP
    IPTYPE_TCP,
    IPTYPE_ICMP,
    IPTYPE_ARP,
    IPTYPE_ERR
}IPTYPE;


/*************************************************
 *WORKMODE: 工作模式
 *************************************************/
typedef enum 
{
    WM_LEARNED_BRIDGING = 0,
    WM_NETWORK_AWARE,

    WM_ERR_MODE
}WORKMODE;


/*************************************************
 *DIRECTION:数据转发方向
 *************************************************/
typedef enum 
{
    DIR_FROM_AI = 0,
    DIR_FROM_WAN,   /*1*/
    DIR_FROM_TDR,   /*2*/
    DIR_TO_AI,      /*3*/
    DIR_TO_WAN,     /*4*/
    DIR_TO_TDR,     /*5*/

    DIR_MAX
}DIRECTION;

/*************************************************
*MACFILTERTYPE:MAC转发过滤器类型
*************************************************/
typedef enum 
{
    EB_MACFILTER_UNKNOWN = 0,
#if 0		
    EB_MACFILTER_ROUTER,
    EB_MACFILTER_PPPOESERVER,
    EB_MACFILTER_DHCPSERVER,
#else
    EB_MACFILTER_ROUTER
#endif
}MACFILTERTYPE;

/*************************************************
 *define Software Version of Data  
 *************************************************/
#define M_DATA_SOFT_VERSION             (0x0002)//0x0001:handover  0x0002:nating

/*************************************************
 *M_HARDWARE_TYPE_ETHERNET:以太网的硬件类型
 *************************************************/
#define M_HARDWARE_TYPE_ETHERNET        (1)


/***************************************************************
 *M_ETHER_TYPE_IP: IP的EtherType
 *M_ETHER_TYPE_ARP: ARP的EtherType
 *M_ETHER_TYPE_PPPoE_DISCOVERY: PPPoE Dicovery stage的EtherType
 *M_ETHER_TYPE_PPPoE_SESSION: PPPoE Session stage的EtherType
 *M_ETHER_TYPE_VLAN: VLAN.
****************************************************************/
#define M_ETHER_TYPE_IP                 (0x0800)
#define M_ETHER_TYPE_IP_v6              (0x86DD)
#define M_ETHER_TYPE_ARP                (0x0806)
#define M_ETHER_TYPE_PPPoE_DISCOVERY    (0x8863)
#define M_ETHER_TYPE_PPPoE_SESSION      (0x8864)
#define M_ETHER_TYPE_VLAN               (0x8100)


/****************************************************
 *M_PROTOCOL_TYPE_TCP: TCP的协议类型
 *M_PROTOCOL_TYPE_UDP: UDP的协议类型
 *M_PROTOCOL_TYPE_ETHERIP: EtherIp的协议类型
****************************************************/
#define M_PROTOCOL_TYPE_ICMP            (1)
#define M_PROTOCOL_TYPE_TCP             (6)
#define M_PROTOCOL_TYPE_UDP             (17)
#define M_PROTOCOL_TYPE_ETHERIP         (97)


/****************************************************
 *M_PPP_PROTOCOL_IP: PPP协议域未压缩时的IP协议
 *M_PPP_PROTOCOL_IP_SUPPRESSED: PPP协议域压缩后的IP协议
****************************************************/
#define M_PPP_PROTOCOL_IP               (0x0021)
#define M_PPP_PROTOCOL_IP_SUPPRESSED    (0x21)


/*************************************************
 *M_PPPOE_SESSION_ALIVETIME: Network Aware模式下
 *转发表项的默认超时时间
 *M_LEARNED_BRIDGE_AGINGTIME: Learned Bridging模式下
 *转发表项的默认超时时间
 *************************************************/
#define M_PPPOE_SESSION_ALIVETIME       (30)
#define M_LEARNED_BRIDGE_AGINGTIME      (1800)



/******************************************************
 *M_DHCP_CLIENT_HADDR_LEN: DHCP客户端硬件地址长度
 *M_DHCP_SERVER_HOSTNAME_LEN:DHCP服务器主机名长度
 *M_DHCP_BOOT_FILENAME_LEN:DHCP启动文件名长度
 ******************************************************/
#define M_DHCP_CLIENT_HADDR_LEN         (16)
#define M_DHCP_SERVER_HOSTNAME_LEN      (64)
#define M_DHCP_BOOT_FILENAME_LEN        (128)



/*************************************************
 *M_DATA_INDEX_ERR:下标错误
 *可用于转发表，IpList下标, CCB下标
 *************************************************/
#define M_DATA_INDEX_ERR                (0xffff)


/*************************************************
 *M_TTL_FFFF:DHCP/PPPoE用户的转发表TTL.
 *************************************************/
#define M_TTL_FFFF                      (0xffff)


/*************************************************
 *M_DATA_PERFMEASURE_ERR:性能计数值错误
 *************************************************/
#define M_DATA_PERFMEASURE_ERR          (0xffffffff)


/*************************************************
 *M_DATA_BROADCAST_EID: 广播给所有EID的标志
 *************************************************/
#define M_DATA_BROADCAST_EID            (0xffffffff)


/*************************************************
 *M_CLEANUP_MONITOR_FULLCIRCLE: CleanUp任务完整
 *检查完转发表需要的时间(秒)
 *************************************************/
#define M_CLEANUP_MONITOR_FULLCIRCLE    (10)


/*************************************************
 *M_ARP_REQUEST: ARP请求
 *M_ARP_REPLY: ARP响应
 *************************************************/
#define M_ARP_REQUEST       (1)
#define M_ARP_REPLY         (2)


/*************************************************
 *M_DHCP_PORT_SERVER: DHCP Server端口
 *M_DHCP_PORT_CLIENT: DHCP Client端口
 *************************************************/
#define M_DHCP_PORT_SERVER  (67)
#define M_DHCP_PORT_CLIENT  (68)

#define M_DNS_PORT  (53)

/*************************************************
 *M_PPPoE_VERTYPE: PPPoE头Ver Type的值
 *Ver = 1; Type = 1;
 *************************************************/
#define M_PPPoE_VERTYPE     (0x11)



/*************************************************
 *M_PPPoE_PADI: PADI代码
 *M_PPPoE_PADO: PADO代码
 *************************************************/
#define M_PPPoE_PADI        (0x09)
#define M_PPPoE_PADO        (0x07)
#define M_PPPoE_PADR        (0x19)
#define M_PPPoE_PADS        (0x65)
#define M_PPPoE_PADT        (0xa7)
#define M_PPPoE_SESSION     (0x0)


/*************************************************
 *M_DHCP_TYPE_ERR:不是DHCP包
 *M_DHCP_FROM_SERVER: Server发往Client的DHCP包
 *M_DHCP_TO_SERVER:Client发往Server的DHCP包
 *************************************************/
#define M_DHCP_TYPE_ERR     (0)
#define M_DHCP_FROM_SERVER  (1)
#define M_DHCP_TO_SERVER    (2)


/*************************************************
 *M_MAC_ADDRLEN:  Mac地址长度
 *M_MACADDR_STRLEN: XX-XX-XX-XX-XX-XX形式的Mac地址
 *的字符串长度
 *M_IP_ADDRLEN:  Mac地址长度
 *M_IPADDR_STRLEN: XXX.XXX.XXX.XXX形式的IP地址长度
 *************************************************/
#define M_MAC_ADDRLEN       (6)
#define M_MACADDR_STRLEN    (18)
#define M_IP_ADDRLEN        (4)
#define M_IPADDR_STRLEN     (16)



/*************************************************
 *M_TOS_SFID_NO:  TOS & SFID最多配置数量
 *************************************************/
#define M_TOS_SFID_NO       (256)


/*************************************************
 *M_SFID_DEFAULT:  SFID默认值为低
 *必须和系统相关的定义保持一致
 *M_NO_VLAN_TAG: 是否添加VLAN Tag.
 *************************************************/
#define M_SFID_DEFAULT      (3)
#define M_NO_VLAN_TAG       (1)


/*************************************************
 *M_MAX_USER_PER_UT:  UT最大支持的用户数
 *M_MAX_USER_PER_BTS: BTS支持的最大UT数
 *M_MAX_USER_PER_BTS: BTS支持的最大用户数
 *************************************************/
#define M_MAX_USER_PER_UT   (20)
#define M_MAX_UT_PER_BTS    (6400)//(320)
#define M_MAX_USER_PER_BTS  ( 320 * M_MAX_USER_PER_UT )


/***************************************************
 *M_SYNC_ADD   :  同步增加IP List
 *M_SYNC_DEL   :  同步删除IP List
 *M_SYNC_UPDATE:  同步修改IP List
 ***************************************************/
#define M_SYNC_ADD          (0)
#define M_SYNC_DELETE       (1)
#define M_SYNC_UPDATE       (2)


/*************************************************
 *Socket Err no.
 *************************************************/
#ifdef __WIN32_SIM__
#define M_DATA_INVALID_SOCKET   (INVALID_SOCKET)
#define M_DATA_SOCKET_ERR       (SOCKET_ERROR)
#else
#define M_DATA_INVALID_SOCKET   (ERROR)
#define M_DATA_SOCKET_ERR       (ERROR)
#endif


/****************************
 *M_RAID_INVALID: 
 *Invalid Router Area Id;
 ****************************/
#define M_RAID_INVALID          (0xFFFFFFFF)


/************************************************************************/
/* 兼容802和EthernetType2协议                                           */
/************************************************************************/
#define M_DATA_PKT_LEN_MAX      (1518)//802.3协议数据报最大值
#define IS_8023_PACKET(lenth)        (lenth<=M_DATA_PKT_LEN_MAX)  

/************************************************************************/
/*MAC Filter表Entry最大值                                               */
/*M_CLEANUP_MACFILTER_ROUTER大约30秒左右做一次arp resolve.如果5分钟内都得*/
/*不到回应，则认为异常，rebootBTS.*/
/************************************************************************/
#if 0
#define M_MAX_MACFILTER_TABLE_NUM  		(10)
#define M_CLEANUP_MACFILTER_SERVER    	(600)
#define M_CLEANUP_MACFILTER_ROUTER    	(M_MAX_MACFILTER_TABLE_NUM*3)
#else
#define M_MAX_MACFILTER_TABLE_NUM		(2)
#define M_CLEANUP_MACFILTER_ROUTER    	(M_MAX_MACFILTER_TABLE_NUM*15)
#endif

#endif /*__L3_DATACOMMON_H__*/
