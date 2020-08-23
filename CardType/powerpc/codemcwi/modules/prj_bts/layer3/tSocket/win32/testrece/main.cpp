#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

bool main(int argc, char *argv[])
{
	if (argc != 2) 
	{
		fprintf(stderr,"usage: udp_server port\n");
		exit(1);
	}

	WSADATA wsadata;
	if (::WSAStartup(MAKEWORD(2,2), &wsadata))
		return false;

	SOCKET m_sfd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sfd==INVALID_SOCKET)
		return false;

	sockaddr_in server;
	int nChars;    
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY); /*inet_addr("127.0.0.1");*/
	server.sin_port = htons(atoi(argv[1]));

	UINT32 count = 0;
	char rcvbuf[1000] = {"\n"};
	int sockaddrlen = sizeof(server);
 
	if (::bind(m_sfd, (sockaddr*)&server, sizeof(server))==SOCKET_ERROR)
	{
		::closesocket(m_sfd);
		return false;
	}

	while(1)
	{
		++count;
		nChars = ::recvfrom(m_sfd, rcvbuf, sizeof(rcvbuf), 0, (sockaddr*)&server, &sockaddrlen);
		if (nChars==SOCKET_ERROR)
		{
			printf("%d > receive fail!\n", count);
		}
		else
		{
			rcvbuf[nChars] = '\0';
			printf("%d > packet contains \"%s\"\n",count,rcvbuf);
		}
	}


	return true;
}