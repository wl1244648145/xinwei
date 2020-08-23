/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataDhcp.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   09/05/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __SNOOP_DHCP_H__
#define __SNOOP_DHCP_H__


/******************************************************
 *按照RFC2132定义所有DHCP选项值
 *在Snoop任务事实上只关注了其中几个选项
 ******************************************************/
#define M_DHCP_OPTION_PAD                (0)
#define M_DHCP_OPTION_SUBNET_MASK        (1)
#define M_DHCP_OPTION_TIME_OFFSET        (2)
#define M_DHCP_OPTION_ROUTER             (3)
#define M_DHCP_OPTION_TIME_SERVER        (4)
#define M_DHCP_OPTION_NAME_SERVER        (5)
#define M_DHCP_OPTION_DNS_SERVER         (6)
#define M_DHCP_OPTION_LOG_SERVER         (7)
#define M_DHCP_OPTION_COOKIE_SERVER      (8)
#define M_DHCP_OPTION_LPR_SERVER         (9)
#define M_DHCP_OPTION_IMPRESS_SERVER     (10)
#define M_DHCP_OPTION_RLS_SERVER         (11)
#define M_DHCP_OPTION_HOSTNAME           (12)
#define M_DHCP_OPTION_BOOTSIZE           (13)
#define M_DHCP_OPTION_MERIT_DUMP         (14)
#define M_DHCP_OPTION_DNS_DOMAIN         (15)
#define M_DHCP_OPTION_SWAP_SERVER        (16)
#define M_DHCP_OPTION_ROOT_PATH          (17)
#define M_DHCP_OPTION_EXTENSIONS_PATH    (18)
#define M_DHCP_OPTION_IP_FORWARD         (19)
#define M_DHCP_OPTION_NONLOCAL_SRCROUTE  (20)
#define M_DHCP_OPTION_POLICY_FILTER      (21)
#define M_DHCP_OPTION_MAX_DGRAM_SIZE     (22)
#define M_DHCP_OPTION_DEFAULT_IP_TTL     (23)
#define M_DHCP_OPTION_MTU_AGING_TIMEOUT  (24)
#define M_DHCP_OPTION_MTU_PLATEAU_TABLE  (25)
#define M_DHCP_OPTION_IF_MTU             (26)
#define M_DHCP_OPTION_ALL_SUBNET_LOCAL   (27)
#define M_DHCP_OPTION_BRDCAST_ADDR       (28)
#define M_DHCP_OPTION_MASK_DISCOVER      (29)
#define M_DHCP_OPTION_MASK_SUPPLIER      (30)
#define M_DHCP_OPTION_ROUTER_DISCOVER    (31)
#define M_DHCP_OPTION_ROUTER_SOLICIT     (32)
#define M_DHCP_OPTION_STATIC_ROUTE       (33)
#define M_DHCP_OPTION_TRAILER            (34)
#define M_DHCP_OPTION_ARP_CACHE_TIMEOUT  (35)
#define M_DHCP_OPTION_ETHER_ENCAP        (36)
#define M_DHCP_OPTION_DEFAULT_TCP_TTL    (37)
#define M_DHCP_OPTION_KEEPALIVE_INTERVAL (38)
#define M_DHCP_OPTION_KEEPALIVE_GARBAGE  (39)
#define M_DHCP_OPTION_NIS_DOMAIN         (40)
#define M_DHCP_OPTION_NIS_SERVER         (41)
#define M_DHCP_OPTION_NTP_SERVER         (42)
#define M_DHCP_OPTION_VENDOR_SPEC        (43)
#define M_DHCP_OPTION_NBN_SERVER         (44)
#define M_DHCP_OPTION_NBDD_SERVER        (45)
#define M_DHCP_OPTION_NB_NODETYPE        (46)
#define M_DHCP_OPTION_NB_SCOPE           (47)
#define M_DHCP_OPTION_XFONT_SERVER       (48)
#define M_DHCP_OPTION_XDISPLAY_MANAGER   (49)
#define M_DHCP_OPTION_REQUEST_IPADDR     (50)
#define M_DHCP_OPTION_LEASE_TIME         (51)
#define M_DHCP_OPTION_OPT_OVERLOAD       (52)
#define M_DHCP_OPTION_MSGTYPE            (53)
#define M_DHCP_OPTION_SERVER_ID          (54)
#define M_DHCP_OPTION_REQ_LIST           (55)
#define M_DHCP_OPTION_ERRMSG             (56)
#define M_DHCP_OPTION_MAXMSGSIZE         (57)
#define M_DHCP_OPTION_T1                 (58) 
#define M_DHCP_OPTION_T2                 (59)
#define M_DHCP_OPTION_CLASS_ID           (60)
#define M_DHCP_OPTION_CLIENT_ID          (61)
#define M_DHCP_OPTION_NISP_DOMAIN        (64)
#define M_DHCP_OPTION_NISP_SERVER        (65)
#define M_DHCP_OPTION_TFTP_SERVERNAME    (66)
#define M_DHCP_OPTION_BOOTFILE           (67)
#define M_DHCP_OPTION_MOBILEIP_HA        (68)
#define M_DHCP_OPTION_SMTP_SERVER        (69)
#define M_DHCP_OPTION_POP3_SERVER        (70)
#define M_DHCP_OPTION_NNTP_SERVER        (71)
#define M_DHCP_OPTION_DFLT_WWW_SERVER    (72)
#define M_DHCP_OPTION_DFLT_FINGER_SERVER (73)
#define M_DHCP_OPTION_DFLT_IRC_SERVER    (74)
#define M_DHCP_OPTION_STREETTALK_SERVER  (75)
#define M_DHCP_OPTION_STDA_SERVER        (76)
#define M_DHCP_OPTION_RELAY_AGENT_INFO   (82)
#define M_DHCP_OPTION_END                (255)
//选项82的子选项
#define M_DHCP_OPTION_AGENT_CIRCUIT_ID   (1)
#define M_DHCP_OPTION_REMOTE_CIRCUIT_ID  (2)


/*************************************************
 *RFC1048_MAGIC:DHCP Option的Magic Cookie
 *************************************************/
#define RFC1048_MAGIC   { 99, 130, 83, 99 }


/******************************************************
 *M_DHCP_OPTION_LEN_REQUESTIP: Request Ip选项长度
 *M_DHCP_OPTION_LEN_LEASETIME: Ip Lease Time选项长度
 *M_DHCP_OPTION_LEN_MESSAGETYPE: MessageType选项长度
 *M_DHCP_OPTION_LEN_SERVERID: Server Identifier选项长度
 *M_DHCP_OPTION_LEN_MAGIC:Magic Cookie选项长度
 ******************************************************/
#define M_DHCP_OPTION_LEN_REQUESTIP     (4)
#define M_DHCP_OPTION_LEN_LEASETIME     (4)
#define M_DHCP_OPTION_LEN_MESSAGETYPE   (1)
#define M_DHCP_OPTION_LEN_SERVERID      (4)
#define M_DHCP_OPTION_LEN_MAGIC         (4)


/*************************************************
 *M_DHCP_OP_REQUEST: DHCP请求
 *M_DHCP_OP_REPLY: DHCP响应
 *************************************************/
#define M_DHCP_OP_REQUEST   (1)
#define M_DHCP_OP_REPLY     (2)



/******************************
 *DHCP包类型
 ******************************/
#define M_DHCP_DISCOVERY    (1)
#define M_DHCP_OFFER        (2)
#define M_DHCP_REQUEST      (3)
#define M_DHCP_DECLINE      (4)
#define M_DHCP_ACK          (5)
#define M_DHCP_NAK          (6)
#define M_DHCP_RELEASE      (7)
#define M_DHCP_INFORM       (8)

#endif/*__SNOOP_DHCP_H__*/
