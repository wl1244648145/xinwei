#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

bool main()
{
	WSADATA wsadata;
	if (::WSAStartup(MAKEWORD(2,2), &wsadata))
		return false;

	SOCKET m_sfd = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sfd==INVALID_SOCKET)
		return false;

    sockaddr_in target;
    int nChars;    
	target.sin_family = AF_INET;
	target.sin_addr.s_addr = inet_addr("192.168.2.69");/*htonl(INADDR_ANY);*/
	target.sin_port = htons(10000);

	UINT32 count = 0;
	char sendbuf[1000] = {"I love you !\n"};
  
	while(1)
	{
		itoa(count,sendbuf,10);
		strcat(sendbuf, " > I love you !\n");
		nChars = ::sendto(m_sfd, sendbuf, sizeof(sendbuf), 0, (sockaddr*)&target, sizeof(target));
		++count;
		if (nChars==SOCKET_ERROR)
		{
			printf("%d > send fail!\n", count);
		}
		else
		{
			printf("%d > %s", count,sendbuf);
		}
		Sleep(1000);
	}

	return true;
}