#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

#include <ws2tcpip.h>
#include <iostream>
#include "ping.h"
using std::cout;
using std::endl;

// ����ICMP����У��͵ļ��㷨, �ܶ�ط�����˵��, ����û�б�Ҫ��ϸ��
// ֻ��һ��Ҫ��, ��У��֮ǰ, ��ؽ�ICMP��ͷ��checksum�ֶ���Ϊ0
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

//���Ի�RAW Socket, ����ttl, ���Ի�dest
// ����ֵ <0 ��ʧ��

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



//���Ի�ICMP�İ�ͷ, ��data�����������, ��������������У���

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

// �������ɵ�ICMP��
// ����ֵ <0 ��ʧ��

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


// ����ICMP��
// ����ֵ <0 ��ʧ��
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


// ���յ���ICMP����
// ����ֵ -2�����, -1 ��ʧ��, 0 �ɹ�
int decode_reply(IPHeader* reply, int bytes, sockaddr_in* from) 
{
	// ����IP��ͷ, �ҵ�ICMP�İ�ͷ
	unsigned short header_len = reply->h_len * 4;
	ICMPHeader* icmphdr = (ICMPHeader*)((char*)reply + header_len);

	// ���ĳ��ȺϷ�, header_len + ICMP_MINΪ��СICMP���ĳ���
	if (bytes < header_len + ICMP_MIN) {
		std::cout << "too few bytes from " << inet_ntoa(from->sin_addr) << 
			endl;
		return -1;
	}
	// ����İ�������ϸ�μ��ҵĵ�һ���� "͸��ICMPЭ��(һ): Э��ԭ��"
	else if (icmphdr->type != ICMP_ECHO_REPLY) {  //�������ظ�
		if (icmphdr->type != ICMP_TTL_EXPIRE) {   //ttl��Ϊ��
			if (icmphdr->type == ICMP_DEST_UNREACH) { //�������ɴ�
				std::cout << "Destination unreachable" << endl;
			}
			else {   //�Ƿ���ICMP������
				std::cout << "Unknown ICMP packet type " << int(icmphdr->type) <<
					" received" << endl;
			}
			return -1;
		}
	}
	else if (icmphdr->id != (USHORT)GetCurrentProcessId()) { 
		//���Ǳ����̷��İ�, ������ͬ��������ping���̷���
		return -2;
	}

	// ָ���������˶�Զ
	// [bugfree]����Ϊ��������������, ��Ϊ��Щϵͳ��ttl��ֵΪ128��winXP, 
	// ��ЩΪ256���ҵ�DNS������211.97.168.129, ���߼���Ϊ256�е����, 
	// ����һ��̽���������, ��email:zhangliangsd@hotmail.com
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

	// ���й�������,��ӡ��Ϣ
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

