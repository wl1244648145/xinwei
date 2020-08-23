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
 *OP: ACL����ʱ�˿ڵıȽϲ�����
 ******************************/
typedef enum 
{
    eq = 0,
    neq,
    lt,
    gt
}OP;


/*************************************************
 *IPTYPE: IP ���ͣ�
 *************************************************/
typedef enum 
{
    IPTYPE_DHCP = 0,
    IPTYPE_PPPoE,
    IPTYPE_FIXIP,       //�̶�IP
    IPTYPE_TCP,
    IPTYPE_ICMP,
    IPTYPE_ARP,
    IPTYPE_ERR
}IPTYPE;


/*************************************************
 *WORKMODE: ����ģʽ
 *************************************************/
typedef enum 
{
    WM_LEARNED_BRIDGING = 0,
    WM_NETWORK_AWARE,

    WM_ERR_MODE
}WORKMODE;


/*************************************************
 *DIRECTION:����ת������
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
*MACFILTERTYPE:MACת������������
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
 *M_HARDWARE_TYPE_ETHERNET:��̫����Ӳ������
 *************************************************/
#define M_HARDWARE_TYPE_ETHERNET        (1)


/***************************************************************
 *M_ETHER_TYPE_IP: IP��EtherType
 *M_ETHER_TYPE_ARP: ARP��EtherType
 *M_ETHER_TYPE_PPPoE_DISCOVERY: PPPoE Dicovery stage��EtherType
 *M_ETHER_TYPE_PPPoE_SESSION: PPPoE Session stage��EtherType
 *M_ETHER_TYPE_VLAN: VLAN.
****************************************************************/
#define M_ETHER_TYPE_IP                 (0x0800)
#define M_ETHER_TYPE_IP_v6              (0x86DD)
#define M_ETHER_TYPE_ARP                (0x0806)
#define M_ETHER_TYPE_PPPoE_DISCOVERY    (0x8863)
#define M_ETHER_TYPE_PPPoE_SESSION      (0x8864)
#define M_ETHER_TYPE_VLAN               (0x8100)


/****************************************************
 *M_PROTOCOL_TYPE_TCP: TCP��Э������
 *M_PROTOCOL_TYPE_UDP: UDP��Э������
 *M_PROTOCOL_TYPE_ETHERIP: EtherIp��Э������
****************************************************/
#define M_PROTOCOL_TYPE_ICMP            (1)
#define M_PROTOCOL_TYPE_TCP             (6)
#define M_PROTOCOL_TYPE_UDP             (17)
#define M_PROTOCOL_TYPE_ETHERIP         (97)


/****************************************************
 *M_PPP_PROTOCOL_IP: PPPЭ����δѹ��ʱ��IPЭ��
 *M_PPP_PROTOCOL_IP_SUPPRESSED: PPPЭ����ѹ�����IPЭ��
****************************************************/
#define M_PPP_PROTOCOL_IP               (0x0021)
#define M_PPP_PROTOCOL_IP_SUPPRESSED    (0x21)


/*************************************************
 *M_PPPOE_SESSION_ALIVETIME: Network Awareģʽ��
 *ת�������Ĭ�ϳ�ʱʱ��
 *M_LEARNED_BRIDGE_AGINGTIME: Learned Bridgingģʽ��
 *ת�������Ĭ�ϳ�ʱʱ��
 *************************************************/
#define M_PPPOE_SESSION_ALIVETIME       (30)
#define M_LEARNED_BRIDGE_AGINGTIME      (1800)



/******************************************************
 *M_DHCP_CLIENT_HADDR_LEN: DHCP�ͻ���Ӳ����ַ����
 *M_DHCP_SERVER_HOSTNAME_LEN:DHCP����������������
 *M_DHCP_BOOT_FILENAME_LEN:DHCP�����ļ�������
 ******************************************************/
#define M_DHCP_CLIENT_HADDR_LEN         (16)
#define M_DHCP_SERVER_HOSTNAME_LEN      (64)
#define M_DHCP_BOOT_FILENAME_LEN        (128)



/*************************************************
 *M_DATA_INDEX_ERR:�±����
 *������ת����IpList�±�, CCB�±�
 *************************************************/
#define M_DATA_INDEX_ERR                (0xffff)


/*************************************************
 *M_TTL_FFFF:DHCP/PPPoE�û���ת����TTL.
 *************************************************/
#define M_TTL_FFFF                      (0xffff)


/*************************************************
 *M_DATA_PERFMEASURE_ERR:���ܼ���ֵ����
 *************************************************/
#define M_DATA_PERFMEASURE_ERR          (0xffffffff)


/*************************************************
 *M_DATA_BROADCAST_EID: �㲥������EID�ı�־
 *************************************************/
#define M_DATA_BROADCAST_EID            (0xffffffff)


/*************************************************
 *M_CLEANUP_MONITOR_FULLCIRCLE: CleanUp��������
 *�����ת������Ҫ��ʱ��(��)
 *************************************************/
#define M_CLEANUP_MONITOR_FULLCIRCLE    (10)


/*************************************************
 *M_ARP_REQUEST: ARP����
 *M_ARP_REPLY: ARP��Ӧ
 *************************************************/
#define M_ARP_REQUEST       (1)
#define M_ARP_REPLY         (2)


/*************************************************
 *M_DHCP_PORT_SERVER: DHCP Server�˿�
 *M_DHCP_PORT_CLIENT: DHCP Client�˿�
 *************************************************/
#define M_DHCP_PORT_SERVER  (67)
#define M_DHCP_PORT_CLIENT  (68)

#define M_DNS_PORT  (53)

/*************************************************
 *M_PPPoE_VERTYPE: PPPoEͷVer Type��ֵ
 *Ver = 1; Type = 1;
 *************************************************/
#define M_PPPoE_VERTYPE     (0x11)



/*************************************************
 *M_PPPoE_PADI: PADI����
 *M_PPPoE_PADO: PADO����
 *************************************************/
#define M_PPPoE_PADI        (0x09)
#define M_PPPoE_PADO        (0x07)
#define M_PPPoE_PADR        (0x19)
#define M_PPPoE_PADS        (0x65)
#define M_PPPoE_PADT        (0xa7)
#define M_PPPoE_SESSION     (0x0)


/*************************************************
 *M_DHCP_TYPE_ERR:����DHCP��
 *M_DHCP_FROM_SERVER: Server����Client��DHCP��
 *M_DHCP_TO_SERVER:Client����Server��DHCP��
 *************************************************/
#define M_DHCP_TYPE_ERR     (0)
#define M_DHCP_FROM_SERVER  (1)
#define M_DHCP_TO_SERVER    (2)


/*************************************************
 *M_MAC_ADDRLEN:  Mac��ַ����
 *M_MACADDR_STRLEN: XX-XX-XX-XX-XX-XX��ʽ��Mac��ַ
 *���ַ�������
 *M_IP_ADDRLEN:  Mac��ַ����
 *M_IPADDR_STRLEN: XXX.XXX.XXX.XXX��ʽ��IP��ַ����
 *************************************************/
#define M_MAC_ADDRLEN       (6)
#define M_MACADDR_STRLEN    (18)
#define M_IP_ADDRLEN        (4)
#define M_IPADDR_STRLEN     (16)



/*************************************************
 *M_TOS_SFID_NO:  TOS & SFID�����������
 *************************************************/
#define M_TOS_SFID_NO       (256)


/*************************************************
 *M_SFID_DEFAULT:  SFIDĬ��ֵΪ��
 *�����ϵͳ��صĶ��屣��һ��
 *M_NO_VLAN_TAG: �Ƿ����VLAN Tag.
 *************************************************/
#define M_SFID_DEFAULT      (3)
#define M_NO_VLAN_TAG       (1)


/*************************************************
 *M_MAX_USER_PER_UT:  UT���֧�ֵ��û���
 *M_MAX_USER_PER_BTS: BTS֧�ֵ����UT��
 *M_MAX_USER_PER_BTS: BTS֧�ֵ�����û���
 *************************************************/
#define M_MAX_USER_PER_UT   (20)
#define M_MAX_UT_PER_BTS    (6400)//(320)
#define M_MAX_USER_PER_BTS  ( 320 * M_MAX_USER_PER_UT )


/***************************************************
 *M_SYNC_ADD   :  ͬ������IP List
 *M_SYNC_DEL   :  ͬ��ɾ��IP List
 *M_SYNC_UPDATE:  ͬ���޸�IP List
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
/* ����802��EthernetType2Э��                                           */
/************************************************************************/
#define M_DATA_PKT_LEN_MAX      (1518)//802.3Э�����ݱ����ֵ
#define IS_8023_PACKET(lenth)        (lenth<=M_DATA_PKT_LEN_MAX)  

/************************************************************************/
/*MAC Filter��Entry���ֵ                                               */
/*M_CLEANUP_MACFILTER_ROUTER��Լ30��������һ��arp resolve.���5�����ڶ���*/
/*������Ӧ������Ϊ�쳣��rebootBTS.*/
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
