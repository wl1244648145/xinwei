#ifndef _WINSOCK2API_
#include <winsock2.h>
#include <WS2tcpip.h>
#include <MSTcpIP.h>
#endif
#include "stdio.h"

#define BACKLOG 10 
#define MAXBUFLEN 100

int main(int argc, char* argv[])
{
	WSADATA wsadata;
	if (::WSAStartup(MAKEWORD(2,2), &wsadata))
		return false;

	SOCKET sockfd, new_fd; 
	struct sockaddr_in my_addr; 
	struct sockaddr_in their_addr; 
	int sin_size;
	int numbytes;
	char buf[MAXBUFLEN];
	int ret = 0;
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); 

	int dwValue = 1;
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)

	ret = ioctlsocket(sockfd, SIO_RCVALL, (u_long*)&dwValue);//让 socket 接受所有的数据
	//WSANOTINITIALISED
	int a = WSAGetLastError();

	int flag = 1;
	int optlen=sizeof(flag);

	ret = ::setsockopt(sockfd,IPPROTO_IP,IP_HDRINCL,(const char*)&flag,optlen);//设置 IP 头操作选项

	my_addr.sin_family = AF_INET; 
	my_addr.sin_port = htons(atoi(argv[2])); 
	my_addr.sin_addr.s_addr = inet_addr(argv[1]);
	memset(&(my_addr.sin_zero), '\0', 8); 

	ret = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));



	sin_size = sizeof(struct sockaddr_in);

	ret = listen(sockfd, BACKLOG);
	do
	{
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);		
		while(1)
		{
			if ((numbytes=recv(new_fd, buf, MAXBUFLEN-1, 0)) == -1) 
			{
				perror("recv");
				break;
			}
			printf("\ngot packet from %s\n",inet_ntoa(their_addr.sin_addr));
			printf("packet is %d bytes long\n",numbytes);
			buf[numbytes] = '\0';
			printf("packet contains \"%s\"\n",buf);
		}	
	}while(1);

	closesocket(sockfd);
	return 0;
}
