/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTypes.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   07/28/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __L3_DATATYPES_H__
#define __L3_DATATYPES_H__

#include "L3DataCommon.h"

#pragma pack (1)
/*********************
 *Ip and Lease
 *********************/
typedef struct _tag_IpLease
{
    UINT32      ulIp;       //主机序
    UINT32      ulLease;    //主机序
}IpLease;


/*****************************************
 *Session Id and PPPoE Server Mac Address
 *****************************************/
typedef struct _tag_SessionMac
{
    UINT16      usSessionId;
    UINT8       aucServerMac[ M_MAC_ADDRLEN ];
}SessionMac;


/*****************************************
 *union DATA:
 *****************************************/
typedef union _tag_DATA                 /*DHCP/PPPoE用户共用的内存*/
{
    IpLease     stIpLease;
    SessionMac  stSessionMac;
} DATA;


/*********************
 *UT IpList Entry
 *********************/
typedef struct _tag_UTILEntry
{
    UINT8       aucMAC[M_MAC_ADDRLEN];
    UINT8       ucIpType;       /*0: DHCP; 1: PPPoE; 02: FIXIP*/
    DATA        Data;
    UINT32      ulRouterAreaId;
    UINT32      ulAnchorBts;
} UTILEntry;



/*********************
 *Ethernet Header
 *********************/
typedef struct _tag_EthHdr
{
    UINT8       aucDstMAC[ M_MAC_ADDRLEN ];
    UINT8       aucSrcMAC[ M_MAC_ADDRLEN ]; 
    UINT16      usProto;                    /*network byte order*/
} EtherHdr;

typedef struct _tag_EthHdr_EX
{
    UINT8       aucDstMAC[ M_MAC_ADDRLEN ];
    UINT8       aucSrcMAC[ M_MAC_ADDRLEN ]; 
    UINT16      VLANTAG;                    /*network byte order*/
    UINT16      VLANNO;
    UINT16      usProto;
} EtherHdrEX;
/*********************/
/*802.3 LLC & SNAP   */
/*********************/
typedef struct _tag_LLCSNAP
{
	UINT8  DSAP;
	UINT8  SSAP;
	UINT8  cntl;
	UINT8  orgcode[3];
	UINT16 type;
}LLCSNAP;


/*********************
 *VLAN Header
 *********************/
typedef struct _tag_VLAN
{
    UINT16 usProto_vlan;    //NBO
    UINT16 usVlanID;        //NBO
}VLAN_hdr;



/*********************
 *Ip Header
 *********************/
typedef struct _tag_IpHdr
{
    UINT8       ucLenVer;       /* 4位首部长度+4位IP版本号 */
    UINT8       ucTOS;          /* 8位服务类型TOS */
    UINT16      usTotalLen;     /* 16位总长度（字节） network byte order */ 
    UINT16      usId;           /* 16位标识 network byte order*/
    UINT16      usFragAndFlags; /* 3位标志位 network byte order*/
    UINT8       ucTTL;          /* 8位生存时间 TTL */
    UINT8       ucProto;        /* 8位协议 (TCP, UDP 或其他) */
    UINT16      usCheckSum;     /* 16位IP首部校验和 network byte order*/
    UINT32      ulSrcIp;        /* 32位源IP地址 network byte order*/
    UINT32      ulDstIp;        /* 32位目的IP地址 network byte order*/
} IpHdr;


/*********************
 *TCP Header
 *********************/
typedef struct _tag_TcpHdr
{
    UINT16      usSrcPort;      /*16位源端口*/
    UINT16      usDstPort;      /*16位目的端口*/
    UINT32      ulSeq;          /*32位序列号*/
    UINT32      ulAck;          /*32位确认号*/
    UINT8       ucLenRes;       /*4位首部长度/6位保留字*/
    UINT8       ucFlags;        /*6位标志位*/
    UINT16      usWinSize;      /*16位窗口大小*/
    UINT16      usCheckSum;     /*16位校验和 network byte order*/
    UINT16      usUrp;          /*16位紧急数据偏移量*/
} TcpHdr;


/*********************
 *UDP Header
 *********************/
typedef struct _tag_UdpHdr
{
    UINT16      usSrcPort;      /* 源端口 NBO*/
    UINT16      usDstPort;      /* 目标端口 NBO*/
    UINT16      usLen;          /* 长度 NBO*/
    UINT16      usCheckSum;     /* 校验和  NBO*/
} UdpHdr;


/*********************
 *DNS Header
 *********************/
typedef struct _tag_DnsHdr
{
    UINT16      usID;      
    UINT16      usDnsFlag;      
    UINT16      usQusNum;          
    UINT16      usAnswerNum;   
    UINT16      usAuth;
    UINT16      usAdd;
    
} DnsHdr;

/* DNS packet Header*/
typedef struct
{
	EtherHdr ethHead;
	IpHdr	ipHead;
	UdpHdr  udpHead;
	DnsHdr	dnsHead;
}DnsPktHead;


/* DNS packet answer*/
typedef struct
{
	UINT8     CmpName[2];
	UINT16	ClassType;
	UINT16    Class;
	UINT32  TTL;
	UINT16	len;
	UINT32   ip;
}DnsPktAns;

/* ip pseudo head */
typedef struct /*ipovly*/
{
	UINT32	srcIP;
	UINT32	dstIP;
	unsigned short proto;  
	unsigned short len;
}pseudoHeadTEb;

/* pseudo udp packet */
typedef struct
{
	/*ethHeadT ethHead;*/
	char	fill[8];
	pseudoHeadTEb phead;
	UdpHdr udpHead;
}pseudoPktTEb;

/*********************
 *ARP Header
 *********************/
typedef struct _tag_ArpHdr
{
    UINT16      usHtype;        /* 硬件类型 NBO*/
    UINT16      usPtype;        /* 协议类型 NBO*/
    UINT8       ucHlen;         /* 硬件地址长度 =0x06*/
    UINT8       ucPlen;         /* 协议地址长度 =0x04*/
    UINT16      usOp;           /* ARP操作类型 NBO*/
    UINT8       aucSenderHaddr[M_MAC_ADDRLEN];  /* 发送者的硬件地址 */
    UINT32      ulSenderPaddr;  /* 发送者的协议地址 NBO*/
    UINT8       aucDestHaddr[M_MAC_ADDRLEN];    /* 目标的硬件地址 */
    UINT32      ulDestPaddr;    /* 目标的协议地址 NBO*/
} ArpHdr;


/*********************
 *Ether Ip Header
 *********************/
typedef struct _tag_EtherIpHdr
{
    UINT16      usVer;          /*ver = 3 << 12 NBO*/
} EtherIpHdr;


/*********************
 *DHCP Header
 *********************/
typedef struct _tag_DhcpHdr
{
    UINT8       ucOpCode;
    UINT8       ucHtype;
    UINT8       ucHlen;
    UINT8       ucHop;
    UINT32      ulXid;
    UINT16      usSec;
    UINT16      usFlag;
    UINT32      ulCiaddr;       /*NBO*/
    UINT32      ulYiaddr;       /*NBO*/
    UINT32      ulSiaddr;       /*NBO*/
    UINT32      ulGiaddr;       /*NBO*/
    UINT8       aucChaddr[M_DHCP_CLIENT_HADDR_LEN];
    UINT8       aucSname[M_DHCP_SERVER_HOSTNAME_LEN];
    UINT8       aucFile[M_DHCP_BOOT_FILENAME_LEN];
} DhcpHdr;



/*********************
 *DHCP Option
 *只包含关心的Option
 *********************/
typedef struct _tag_DhcpOption
{
    UINT32      ulIpLeaseTime;  /*NBO*/
    UINT8       ucMessageType;  /*Discovery,Offer,Request...?*/
    UINT32      ulServerId;     /*NBO*/
    UINT32      ulRequestIp;    /*NBO*/
    /*其他选项在之后添加*/

}DhcpOption;


/*********************
 *PPPoE Header
 *********************/
typedef struct _tag_PPPoEHdr
{
    UINT8       ucVerType;
    UINT8       ucCode;
    UINT16      usSessionId;    /*network byte order*/
    UINT16      usLength;       /*network byte order*/
} PPPoEHdr;



//define struct of interface in DM task and Snoop Task
typedef struct T_CPEProfile
{
    bool        bMobility;
    bool        bDHCPRenew;
    bool        bIsFull;   //true:IPlist have no space,disable to add new;otherwise enable 
}CPEProfile;


#pragma pack ()
#endif /*__L3_DATATYPES_H__*/
