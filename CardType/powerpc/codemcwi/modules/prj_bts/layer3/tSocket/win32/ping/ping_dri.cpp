#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

//#include "iostream.h"
#include <iostream>
#include "ping.h"

using std::cout;
using std::endl;

#define DEFAULT_PACKET_SIZE 32   // Ĭ��ICMP���ֽ���
#define DEFAULT_TTL 30           // Ĭ��TTLֵ
#define MAX_PING_DATA_SIZE 1024  // ������ݿ� 
#define MAX_PING_PACKET_SIZE (MAX_PING_DATA_SIZE + sizeof(IPHeader)) //���ICMP������

/* Ϊ send_buf �� recv_buf �����ڴ�
* send_buf��СΪ packet_size
* recv_buf��СΪ MAX_PING_PACKET_SIZE, ��֤����send_buf
*/
int allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf,
					 int packet_size); 


///////////////////////////////////////////////////////////////////////
// Program entry point

int main(int argc, char* argv[])
{

	int seq_no = 0;   //���ڷ��ͺͽ��ܵ�ICMP��ͷ��
	ICMPHeader* send_buf = 0; 
	IPHeader* recv_buf = 0;

	// �ж��������Ƿ�Ϸ�
	if (argc < 2) {
		std::cout << "usage: " << argv[0] << " <host> [data_size] [ttl]" <<
			endl;
		std::cout << "\tdata_size can be up to " << MAX_PING_DATA_SIZE <<
			" bytes.  Default is " << DEFAULT_PACKET_SIZE << "." << 
			endl; 
		std::cout << "\tttl should be 255 or lower.  Default is " <<
			DEFAULT_TTL << "." << endl;
		return 1;
	}

	// ���������в���
	int packet_size = DEFAULT_PACKET_SIZE;
	int ttl = DEFAULT_TTL;
	if (argc > 2) {
		int temp = atoi(argv[2]);
		if (temp != 0) {
			packet_size = temp;
		}
		if (argc > 3) {
			temp = atoi(argv[3]);
			if ((temp >= 0) && (temp <= 255)) {
				ttl = temp;
			}
		}
	}
	packet_size = max(sizeof(ICMPHeader), 
		min(MAX_PING_DATA_SIZE, (unsigned int)packet_size));

	// ���� Winsock
	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0) {
		std::cout << "Failed to find Winsock 2.1 or better." << endl;
		return 1;
	}

	SOCKET sd; // RAW Socket���
	sockaddr_in dest, source; 

	// ��������(����sd, ����ttl, ����dest��ֵ)
	if (setup_for_ping(argv[1], ttl, sd, dest) < 0) {
		goto cleanup; //�ͷ���Դ���˳�
	}
	// Ϊsend_buf��recv_buf�����ڴ� 
	if (allocate_buffers(send_buf, recv_buf, packet_size) < 0) {
		goto cleanup;
	}
	// ���Ի�ICMP���ݰ�(type=8,code=0)
	init_ping_packet(send_buf, packet_size, seq_no);

	// ����ICMP���ݰ�
	if (send_ping(sd, dest, send_buf, packet_size) >= 0) {
		while (1) {
			// ���ܻ�Ӧ��
			if (recv_ping(sd, source, recv_buf, MAX_PING_PACKET_SIZE) <
				0) {
					// Pull the sequence number out of the ICMP header.  If 
					// it's bad, we just complain, but otherwise we take 
					// off, because the read failed for some reason.
					unsigned short header_len = recv_buf->h_len * 4;
					ICMPHeader* icmphdr = (ICMPHeader*)
						((char*)recv_buf + header_len);
					if (icmphdr->seq != seq_no) {
						std::cout << "bad sequence number!" << endl;
						continue;
					}
					else {
						break;
					}
				}
				if (decode_reply(recv_buf, packet_size, &source) != -2) {
					// Success or fatal error (as opposed to a minor error) 
					// so take off.
					break;
				}
		}
	}

cleanup:
	delete[]send_buf;  //�ͷŷ�����ڴ�
	delete[]recv_buf;
	WSACleanup(); // ����winsock
	return 0;
}

// Ϊsend_buf �� recv_buf���ڴ����. ̫��, ���Թ�
int allocate_buffers(ICMPHeader*& send_buf, IPHeader*& recv_buf,
					 int packet_size)
{
	// First the send buffer
	send_buf = (ICMPHeader*)new char[packet_size];  
	if (send_buf == 0) {
		std::cout << "Failed to allocate output buffer." << endl;
		return -1;
	}

	// And then the receive buffer
	recv_buf = (IPHeader*)new char[MAX_PING_PACKET_SIZE];
	if (recv_buf == 0) {
		std::cout << "Failed to allocate output buffer." << endl;
		return -1;
	}

	return 0;
}

