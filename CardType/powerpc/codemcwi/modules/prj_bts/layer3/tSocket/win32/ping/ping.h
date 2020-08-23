#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

// ICMP 包类型, 具体参见本文的第一节
#define ICMP_ECHO_REPLY 0   
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

// 最小的ICMP包大小
#define ICMP_MIN 8


// IP 包头
struct IPHeader {
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
};

// ICMP 包头(实际的包不包括timestamp字段, 
// 作者用来计算包的回应时间,其实完全没有必要这样做)
struct ICMPHeader {
	BYTE type;          // ICMP packet type
	BYTE code;          // Type sub code
	USHORT checksum;
	USHORT id;
	USHORT seq;
	ULONG timestamp;    // not part of ICMP, but we need it
};


extern USHORT ip_checksum(USHORT* buffer, int size);
extern int setup_for_ping(char* host, int ttl, SOCKET& sd,  sockaddr_in& dest);
extern int send_ping(SOCKET sd, const sockaddr_in& dest, ICMPHeader* send_buf, int packet_size);
extern int recv_ping(SOCKET sd, sockaddr_in& source, IPHeader* recv_buf,
					 int packet_size);
extern int decode_reply(IPHeader* reply, int bytes, sockaddr_in* from);
extern void init_ping_packet(ICMPHeader* icmp_hdr, int packet_size, int seq_no);

