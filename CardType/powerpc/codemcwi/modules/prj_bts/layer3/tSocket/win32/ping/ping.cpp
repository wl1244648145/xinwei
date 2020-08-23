#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

#include <ws2tcpip.h>
#include <iostream>
#include "ping.h"
using std::cout;
using std::endl;

// 计算ICMP包的校验和的简单算法, 很多地方都有说明, 这里没有必要详细将
// 只是一点要提, 做校验之前, 务必将ICMP包头的checksum字段置为0
USHORT ip_checksum(USHORT* buffer, int size) 
{
	unsigned long cksum = 0;

	// Sum all the words together, adding the final byte if size is odd
	while (size > 1) {
		cksum += *buffer++;
		size -= sizeof(USHORT);
	}
	if (size) {
		cksum += *(UCHAR*)buffer;
	}

	// Do a little shuffling
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);

	// Return the bitwise complement of the resulting mishmash
	return (USHORT)(~cksum);
}

//初试化RAW Socket, 设置ttl, 初试化dest
// 返回值 <0 表失败

int setup_for_ping(char* host, int ttl, SOCKET& sd, sockaddr_in& dest)
{
	// Create the socket
	sd = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, 0, 0, 0);
	if (sd == INVALID_SOCKET) {
		std::cout << "Failed to create raw socket: " << WSAGetLastError() <<
			endl;
		return -1;
	}

	if (setsockopt(sd, IPPROTO_IP, IP_TTL, (const char*)&ttl, 
		sizeof(ttl)) == SOCKET_ERROR) {
			std::cout << "TTL setsockopt failed: " << WSAGetLastError() << endl;
			return -1;
		}

		// Initialize the destination host info block
		memset(&dest, 0, sizeof(dest));

		// Turn first passed parameter into an IP address to ping
		unsigned int addr = inet_addr(host);
		if (addr != INADDR_NONE) {
			// It was a dotted quad number, so save result
			dest.sin_addr.s_addr = addr;
			dest.sin_family = AF_INET;
		}
		else {
			// Not in dotted quad form, so try and look it up
			hostent* hp = gethostbyname(host);
			if (hp != 0) {
				// Found an address for that host, so save it
				memcpy(&(dest.sin_addr), hp->h_addr, hp->h_length);
				dest.sin_family = hp->h_addrtype;
			}
			else {
				// Not a recognized hostname either!
				std::cout << "Failed to resolve " << host << endl;
				return -1;
			}
		}

		return 0;
}



//初试化ICMP的包头, 给data部分填充数据, 最后计算整个包的校验和

void init_ping_packet(ICMPHeader* icmp_hdr, int packet_size, int seq_no)
{
	// Set up the packet's fields
	icmp_hdr->type = ICMP_ECHO_REQUEST;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = 0;
	icmp_hdr->id = (USHORT)GetCurrentProcessId();
	icmp_hdr->seq = seq_no;
	icmp_hdr->timestamp = GetTickCount();

	// "You're dead meat now, packet!"
	const unsigned long int deadmeat = 0xDEADBEEF;
	char* datapart = (char*)icmp_hdr + sizeof(ICMPHeader);
	int bytes_left = packet_size - sizeof(ICMPHeader);
	while (bytes_left > 0) {
		memcpy(datapart, &deadmeat, min(int(sizeof(deadmeat)), 
			bytes_left));
		bytes_left -= sizeof(deadmeat);
		datapart += sizeof(deadmeat);
	}

	// Calculate a checksum on the result
	icmp_hdr->checksum = ip_checksum((USHORT*)icmp_hdr, packet_size);
}

// 发送生成的ICMP包
// 返回值 <0 表失败

int send_ping(SOCKET sd, const sockaddr_in& dest, ICMPHeader* send_buf,
			  int packet_size)
{
	// Send the ping packet in send_buf as-is
	std::cout << "Sending " << packet_size << " bytes to " << 
		inet_ntoa(dest.sin_addr) << "..." /*<< flush*/;
	int bwrote = sendto(sd, (char*)send_buf, packet_size, 0, 
		(sockaddr*)&dest, sizeof(dest));
	if (bwrote == SOCKET_ERROR) {
		std::cout << "send failed: " << WSAGetLastError() << endl;
		return -1;
	}
	else if (bwrote < packet_size) {
		std::cout << "sent " << bwrote << " bytes..." /*<< flush*/;
	}

	return 0;
}


// 接受ICMP包
// 返回值 <0 表失败
int recv_ping(SOCKET sd, sockaddr_in& source, IPHeader* recv_buf, 
			  int packet_size)
{
	// Wait for the ping reply
	int fromlen = sizeof(source);
	int bread = recvfrom(sd, (char*)recv_buf, 
		packet_size + sizeof(IPHeader), 0,
		(sockaddr*)&source, &fromlen);
	if (bread == SOCKET_ERROR) {
		std::cout << "read failed: ";
		if (WSAGetLastError() == WSAEMSGSIZE) {
			std::cout << "buffer too small" << endl;
		}
		else {
			std::cout << "error #" << WSAGetLastError() << endl;
		}
		return -1;
	}

	return 0;
}


// 对收到的ICMP解码
// 返回值 -2表忽略, -1 表失败, 0 成功
int decode_reply(IPHeader* reply, int bytes, sockaddr_in* from) 
{
	// 跳过IP包头, 找到ICMP的包头
	unsigned short header_len = reply->h_len * 4;
	ICMPHeader* icmphdr = (ICMPHeader*)((char*)reply + header_len);

	// 包的长度合法, header_len + ICMP_MIN为最小ICMP包的长度
	if (bytes < header_len + ICMP_MIN) {
		std::cout << "too few bytes from " << inet_ntoa(from->sin_addr) << 
			endl;
		return -1;
	}
	// 下面的包类型详细参见我的第一部分 "透析ICMP协议(一): 协议原理"
	else if (icmphdr->type != ICMP_ECHO_REPLY) {  //非正常回复
		if (icmphdr->type != ICMP_TTL_EXPIRE) {   //ttl减为零
			if (icmphdr->type == ICMP_DEST_UNREACH) { //主机不可达
				std::cout << "Destination unreachable" << endl;
			}
			else {   //非法的ICMP包类型
				std::cout << "Unknown ICMP packet type " << int(icmphdr->type) <<
					" received" << endl;
			}
			return -1;
		}
	}
	else if (icmphdr->id != (USHORT)GetCurrentProcessId()) { 
		//不是本进程发的包, 可能是同机的其它ping进程发的
		return -2;
	}

	// 指出包传递了多远
	// [bugfree]我认为作者这里有问题, 因为有些系统的ttl初值为128如winXP, 
	// 有些为256如我的DNS服务器211.97.168.129, 作者假设为256有点武断, 
	// 可以一起探讨这个问题, 回email:zhangliangsd@hotmail.com
	int nHops = int(256 - reply->ttl);
	if (nHops == 192) { 
		// TTL came back 64, so ping was probably to a host on the
		// LAN -- call it a single hop.
		nHops = 1;
	}
	else if (nHops == 128) {
		// Probably localhost
		nHops = 0;
	}

	// 所有工作结束,打印信息
	cout << endl << bytes << " bytes from " << 
		inet_ntoa(from->sin_addr) << ", icmp_seq " << 
		icmphdr->seq << ", ";
	if (icmphdr->type == ICMP_TTL_EXPIRE) {
		cout << "TTL expired." << endl;
	}
	else {
		cout << nHops << " hop" << (nHops == 1 ? "" : "s");
		cout << ", time: " << (GetTickCount() - icmphdr->timestamp) <<
			" ms." << endl;
	}

	return 0;
}

