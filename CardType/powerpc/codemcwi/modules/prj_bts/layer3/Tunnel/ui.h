#ifndef UI_H 
#define UI_H


#ifdef __cplusplus
extern "C" {
#endif


/* type */
typedef unsigned char  	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;


/* system */
#include "msgQLib.h"
#include "ioLib.h"
#include "tasklib.h"
#include "sysLib.h"
#include "string.h"
#include "stdio.h"
#include "semLib.h"
#include "wdLib.h"
#include "sockLib.h"
#include "in.h"
#include "pipedrv.h"
#include "kernelLib.h"
#include "rebootLib.h"
#include "wdLib.h"
#include "net/inet.h"
#include "ctype.h"
#include "routeLib.h"
#include "intLib.h"
#include "usrlib.h"
#include "net\Mbuf.h"
#include "ipProto.h"
#include "net\inet.h"
#include "muxlib.h"
#include "netbuflib.h"
#include "rnglib.h"
#include "netinet\in_var.h"
//#include "netinet\in.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"

/********************************* copy from li.h */
/*#include "li.h"*/


#define insque(q,p)     _insque((caddr_t)q,(caddr_t)p)
#define remque(q)       _remque((caddr_t)q)


typedef enum
{
    UDP,
    TCP,
    XTP
}   Protocol_t;

typedef void(*UDP_CALLBACK_HANDLER)(int socketId,int error,void* context);

/********************************** end li.h  */


#define RVERROR	-1
#define TRUE	1
#define FALSE	0

#define UDP_MTU_LEN         1400

#define ETH_PAYLOAD_LEN 1600
#define IP_PAYLOAD_LEN 1500
#define UDP_PAYLOAD_LEN     3000        //1024
#define M_TSOCKET_BUFFER_LENGTH     (2400) // lijinan (2000)
#define RING_BUF_LEN	2000

#define ARP_TABLE_LEN	200
#define ARP_LIVE_TIME	600			/* 600*100ms */
#define	ARP_RESP_TIME	5			/* Arp Response waiting time 50*10ms */
#define	ARP_REQUEST		1			/* ARP request			*/
#define	ARP_REPLY		2			/* ARP reply			*/


#define ETH_ALEN	6			/* Octets in one ethernet addr	 */
#define ETH_P_ARP	0x0806		/* Address Resolution packet	*/
#define ETH_P_802_3	0x0100		/* Dummy type for 802.3 frames  */
#define ETH_P_IP	0x0800		/* Internet Protocol packet	*/


#define UI_MIN_PORT		7000	
#define UI_MAX_PORT		8999
#define UI_MAX_HANDLE	1999		/* 8999 - 7000 */

#define LOCAL_MEM_LOCAL_ADRS    0x00000000
#define BOOT_LINE_OFFSET 0x4200

#define BOOT_LINE_ADRS	((char *) (LOCAL_MEM_LOCAL_ADRS+BOOT_LINE_OFFSET))

/* ip->mac resolve result*/
typedef enum
{
	res_ok=1,
	res_wait=2,
	res_fail=3
}resResult;



/* begin byte alignment */
#pragma pack(1)

/* ethernet head */
typedef struct _ethhdr 
{
	unsigned char	h_dest[ETH_ALEN];	/* destination eth addr	*/
	unsigned char	h_source[ETH_ALEN];	/* source ether addr	*/
	unsigned short	h_proto;		/* packet type ID field	*/
}ethHeadT;

/* ethernet packet */
typedef struct _etherpkt
{
	ethHeadT	ethHead;
	BYTE		payload[ETH_PAYLOAD_LEN];
}ethPktT;

/* arp head */
typedef struct _arphdr
{
	unsigned short	ar_hrd;		/* format of hardware address	*/
	unsigned short	ar_pro;		/* format of protocol address	*/
	unsigned char	ar_hln;		/* length of hardware address	*/
	unsigned char	ar_pln;		/* length of protocol address	*/
	unsigned short	ar_op;		/* ARP opcode (command)		*/

	unsigned char	ar_sha[ETH_ALEN];	/* sender hardware address	*/
	unsigned char	ar_sip[4];			/* sender IP address		*/
	unsigned char	ar_tha[ETH_ALEN];	/* target hardware address	*/
	unsigned char	ar_tip[4];			/* target IP address		*/
}arpHeadT;


/* arp packet */
typedef struct _arpPacket
{
	ethHeadT	ethHead;
	arpHeadT	arpHead;
}arpPktT;


/* ipv4 address */
typedef union _ip4Addr
{
	DWORD dwIP;
	struct
	{
		BYTE b1;
		BYTE b2;
		BYTE b3;
		BYTE b4;
	}bIP;
}ipAddrT;

/* ipv4 packet head */ 
typedef struct  
{ 
	unsigned char h_lenver; 
	unsigned char tos;  
	unsigned short total_len; 
	unsigned short ident;  
	unsigned short frag_and_flags;  
	unsigned char ttl;  
	unsigned char proto;  
	unsigned short checksum;  
	ipAddrT	srcIP;
	ipAddrT	dstIP;
}ipHeadT;


/* ipv4 packet */ 
typedef struct
{
	ethHeadT ethHead;
	ipHeadT	ipHead;
	BYTE	payload[IP_PAYLOAD_LEN];
}ipPktT;

/* udp head */
typedef struct
{
	unsigned short srcPort;
	unsigned short dstPort;
	unsigned short len;
	unsigned short checksum;
}udpHeadT;

/* udp packet */
typedef struct
{
	ethHeadT ethHead;
	ipHeadT	ipHead;
	udpHeadT udpHead;
	BYTE	payload[UDP_PAYLOAD_LEN];
}udpPktT;

typedef struct
{
	ipHeadT	ipHead;
	udpHeadT udpHead;
	BYTE	payload[UDP_PAYLOAD_LEN];
}ipUdpPktT;


/* ip pseudo head */
typedef struct /*ipovly*/
{
	ipAddrT	srcIP;
	ipAddrT	dstIP;
	unsigned short proto;  
	unsigned short len;
}pseudoHeadT;

/* pseudo udp packet */
typedef struct
{
	/*ethHeadT ethHead;*/
	char	fill[8];
	pseudoHeadT phead;
	udpHeadT udpHead;
	BYTE	payload[UDP_PAYLOAD_LEN];
}pseudoPktT;



#pragma	/* end pack(1) */


extern NET_POOL_ID _pNetDpool; 

/*  arp table */
typedef struct _arptable
{
	struct _arptable	*prev;
	struct _arptable	*next;
	DWORD	dwIP;	
	BYTE	bMac[ETH_ALEN];
	WORD	wTimer;			/* response/keepAlive timer */
	M_BLK_ID hold;			/* packets that waiting for arp response */
/*	M_BLK_ID tail;			 holding tail */
}arpTableT;
/*  arp table */
typedef struct _arptable_new
{

	DWORD	dwIP;	
	BYTE	bMac[ETH_ALEN];
	WORD	wTimer;			/* response/keepAlive timer */
	M_BLK_ID hold;			/* packets that waiting for arp response */
/*	M_BLK_ID tail;			 holding tail */
}newarpTableT;

typedef struct
{
	DWORD		localIP;
	WORD		localPort;
	M_BLK_ID	pMblk;	
	UDP_CALLBACK_HANDLER	callback;
	void	*context;
}socketT;

 DWORD dwIP;
int uiEnd(void);
int uiOpen (UINT32 ipAddr, UINT16 myPort, Protocol_t protocol);
int uiClose(int handle);
int uiCallOn(int handle, UDP_CALLBACK_HANDLER callback, void* context);
int uiUdpSend (int handle,UINT8 *buff,int len,UINT8 *ucDstBtsMac,UINT32 ipAddr,UINT16 port);
int uiUdpRecv (int handle, UINT8 *buff,int len, UINT32 *ipAddr, UINT16 *port);
int uiInit(void);

UINT16  uiGetSockPort (int handle);
int  uiBlock (int handle);
int  uiUnblock (int handle);
int  uiJoinMulticastGroup(int handle,UINT32 mcastip,UINT32 ipaddr);
int  uiSetSocketBuffers(int handle, int sendSize, int recvSize);
int  uiBytesAvailable (int handle,int *bytesAvailable);

/*void netShow2(void);*/
void UDPReceive(int socketId,int error, void* context);

/* overide li */
/*
#define	liInit			uiInit
#define	liEnd			uiEnd
#define liOpen			uiOpen
#define	liClose			uiClose
#define liCallOn		uiCallOn
#define	liUdpSend		uiUdpSend
#define	liUdpRecv		uiUdpRecv

#define liGetSockPort	uiGetSockPort
#define liBlock			uiBlock
#define liUnblock		uiUnblock
#define liJoinMulticastGroup	uiJoinMulticastGroup
#define liSetSocketBuffers		uiSetSocketBuffers
#define liBytesAvailable		uiBytesAvailable
*/

// ICMP 包类型
#define ICMP_ECHO_REPLY 0   
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

// 最小的ICMP包大小
#define ICMP_MIN 8

// IP 包头
typedef struct  {
       ethHeadT ethHead;
	BYTE h_len:4;           // Length of the header in dwords
	BYTE version:4;         // Version of IP
	BYTE tos;               // Type of service
	USHORT total_len;       // Length of the packet in dwords
	USHORT ident;           // unique identifier
	USHORT flags;           // Flags
	BYTE ttl;               // Time to live, 这个字段我在下一节中用来实现Tracert功能
	BYTE proto;             // Protocol number (TCP, UDP etc)
	USHORT checksum;        // IP checksum
	ULONG source_ip;
	ULONG dest_ip;
}IPHeader;


// ICMP 包头
typedef struct  {
	BYTE type;          // ICMP packet type
	BYTE code;          // Type sub code
	USHORT checksum;
	USHORT id;
	USHORT seq;
}ICMPHeader;

typedef  struct _TCTA_ABNORMAL_IP
{
        ULONG   ulSourceIp;  
        USHORT  usCntOfDestUnreach;
	 USHORT  usCntOfTtlExpire;
	 USHORT usOtherIcmp;
	
} TCTA_ABNORMAL_IP;

typedef  struct _TCTA_STAT_REGISTER
{
     BYTE ucStatAbnmlIpCnt;/*每10s清零，最大增加到10*/
     TCTA_ABNORMAL_IP  stIp[10];/* 每10s最多统计10个用户，用于区分不同的用户*/
     /*------------------------------------------------*/
} TCTA_STAT_UIIP;

typedef struct   _UDP_STATS
{
	unsigned int err_times_no_blk ;/**发送没有mblk**/
	unsigned int send_success_times ;/***发送成功次数**/
	unsigned int send_fail_times ;/***发送失败次数***/
	unsigned int send_buf_times ;/***发送buffer数***/
	unsigned int called_by_sockets;/***上层应用调用发包数****/
	unsigned int Rec_Ring_Buf_OverFlow;/**uitask接收ringbuffer over flow数***/
	unsigned int rec_mux_Ring_Buf_OverFlow;/**mux层接收ring buffer over flow数**/
	unsigned int Rec_buf;/**UItask 处理的消息数**/
	unsigned int Rec_by_socket;/*****socket处理的消息数******/
	unsigned int rec_ip;/**muxrec的IP包的个数***/
	unsigned int rec_arp;/**muxrec的arp包的个数**/
	unsigned int rec_icmp;/*IP中的ICMP包的个数**/
	unsigned int arp_fail;
	unsigned int arp_wait;
       unsigned int arp_fail1;
       unsigned int arp_fail2;
       unsigned int arp_fail3;
       unsigned int arp_fail4;
}UDP_STATS;

#ifdef __cplusplus
}
#endif


#endif
