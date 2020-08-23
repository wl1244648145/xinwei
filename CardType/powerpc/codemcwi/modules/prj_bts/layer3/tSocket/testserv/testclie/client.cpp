#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

int main(int argc, char *argv[])
{
	if (argc != 4) 
	{
		fprintf(stderr,"usage: tcp_client dest_ip dest_port messages\n");
		exit(1);
	}

	WSADATA wsadata;
	if (::WSAStartup(MAKEWORD(2,2), &wsadata))
		return false;

	SOCKET sockfd;
	struct sockaddr_in dest_addr; 
	int numbytes;
	char *message = argv[3];
	int len = (int)strlen(message);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}

	int sndbuf;
	int optlen=sizeof(sndbuf);
	::getsockopt(sockfd, SOL_SOCKET,SO_SNDBUF, (char*)&sndbuf, &optlen);
	sndbuf = sndbuf<<4;
	::setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,(const char*)&sndbuf,optlen);

	dest_addr.sin_family = AF_INET; 
	dest_addr.sin_port = htons(atoi(argv[2])); 
	dest_addr.sin_addr.s_addr = inet_addr((argv[1]));
	memset(&(dest_addr.sin_zero), '\0', 8); 
	connect(sockfd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr));
	int count = 0;
	while(1)
	{
		if ((numbytes = send(sockfd, message, len, 0)) == -1) 
		{
			perror("send");
			exit(1);
		}
		Sleep(1);
		printf("%d > sent OK\n",++count);
		printf("sent %d bytes to %s\n", numbytes, inet_ntoa(dest_addr.sin_addr));
	}
	closesocket(sockfd);

	return 0;
}
