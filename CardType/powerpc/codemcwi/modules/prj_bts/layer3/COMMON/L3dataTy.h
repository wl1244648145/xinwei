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
    UINT32      ulIp;       //������
    UINT32      ulLease;    //������
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
typedef union _tag_DATA                 /*DHCP/PPPoE�û����õ��ڴ�*/
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
    UINT8       ucLenVer;       /* 4λ�ײ�����+4λIP�汾�� */
    UINT8       ucTOS;          /* 8λ��������TOS */
    UINT16      usTotalLen;     /* 16λ�ܳ��ȣ��ֽڣ� network byte order */ 
    UINT16      usId;           /* 16λ��ʶ network byte order*/
    UINT16      usFragAndFlags; /* 3λ��־λ network byte order*/
    UINT8       ucTTL;          /* 8λ����ʱ�� TTL */
    UINT8       ucProto;        /* 8λЭ�� (TCP, UDP ������) */
    UINT16      usCheckSum;     /* 16λIP�ײ�У��� network byte order*/
    UINT32      ulSrcIp;        /* 32λԴIP��ַ network byte order*/
    UINT32      ulDstIp;        /* 32λĿ��IP��ַ network byte order*/
} IpHdr;


/*********************
 *TCP Header
 *********************/
typedef struct _tag_TcpHdr
{
    UINT16      usSrcPort;      /*16λԴ�˿�*/
    UINT16      usDstPort;      /*16λĿ�Ķ˿�*/
    UINT32      ulSeq;          /*32λ���к�*/
    UINT32      ulAck;          /*32λȷ�Ϻ�*/
    UINT8       ucLenRes;       /*4λ�ײ�����/6λ������*/
    UINT8       ucFlags;        /*6λ��־λ*/
    UINT16      usWinSize;      /*16λ���ڴ�С*/
    UINT16      usCheckSum;     /*16λУ��� network byte order*/
    UINT16      usUrp;          /*16λ��������ƫ����*/
} TcpHdr;


/*********************
 *UDP Header
 *********************/
typedef struct _tag_UdpHdr
{
    UINT16      usSrcPort;      /* Դ�˿� NBO*/
    UINT16      usDstPort;      /* Ŀ��˿� NBO*/
    UINT16      usLen;          /* ���� NBO*/
    UINT16      usCheckSum;     /* У���  NBO*/
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
    UINT16      usHtype;        /* Ӳ������ NBO*/
    UINT16      usPtype;        /* Э������ NBO*/
    UINT8       ucHlen;         /* Ӳ����ַ���� =0x06*/
    UINT8       ucPlen;         /* Э���ַ���� =0x04*/
    UINT16      usOp;           /* ARP�������� NBO*/
    UINT8       aucSenderHaddr[M_MAC_ADDRLEN];  /* �����ߵ�Ӳ����ַ */
    UINT32      ulSenderPaddr;  /* �����ߵ�Э���ַ NBO*/
    UINT8       aucDestHaddr[M_MAC_ADDRLEN];    /* Ŀ���Ӳ����ַ */
    UINT32      ulDestPaddr;    /* Ŀ���Э���ַ NBO*/
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
 *ֻ�������ĵ�Option
 *********************/
typedef struct _tag_DhcpOption
{
    UINT32      ulIpLeaseTime;  /*NBO*/
    UINT8       ucMessageType;  /*Discovery,Offer,Request...?*/
    UINT32      ulServerId;     /*NBO*/
    UINT32      ulRequestIp;    /*NBO*/
    /*����ѡ����֮�����*/

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
