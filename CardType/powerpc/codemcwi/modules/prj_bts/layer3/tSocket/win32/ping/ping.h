#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

// ICMP ������, ����μ����ĵĵ�һ��
#define ICMP_ECHO_REPLY 0   
#define ICMP_DEST_UNREACH 3
#define ICMP_TTL_EXPIRE 11
#define ICMP_ECHO_REQUEST 8

// ��С��ICMP����С
#define ICMP_MIN 8


// IP ��ͷ
struct IPHeader {
	BYTE h_len:4;           // Length of the header in dwords
	BYTE version:4;         // Version of IP
	BYTE tos;               // Type of service
	USHORT total_len;       // Length of the packet in dwords
	USHORT ident;           // unique identifier
	USHORT flags;           // Flags
	BYTE ttl;               // Time to live, ����ֶ�������һ��������ʵ��Tracert����
	BYTE proto;             // Protocol number (TCP, UDP etc)
	USHORT checksum;        // IP checksum
	ULONG source_ip;
	ULONG dest_ip;
};

// ICMP ��ͷ(ʵ�ʵİ�������timestamp�ֶ�, 
// ��������������Ļ�Ӧʱ��,��ʵ��ȫû�б�Ҫ������)
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

