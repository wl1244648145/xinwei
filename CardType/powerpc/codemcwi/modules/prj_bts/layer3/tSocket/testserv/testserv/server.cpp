#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

#define BACKLOG 10 
#define MAXBUFLEN 100

int main(int argc, char *argv[])
{
	if (argc != 2) 
	{
		fprintf(stderr,"usage: tcp_server port\n");
		exit(1);
	}

	WSADATA wsadata;
	if (::WSAStartup(MAKEWORD(2,2), &wsadata))
		return false;

	SOCKET sockfd, new_fd; 
	struct sockaddr_in my_addr; 
	struct sockaddr_in their_addr; 
	int sin_size;
	int numbytes;
	char buf[MAXBUFLEN];

	sockfd = socket(PF_INET, SOCK_STREAM, 0); 

	int rcvbuf;
	int optlen=sizeof(rcvbuf);
	::getsockopt(sockfd, SOL_SOCKET,SO_RCVBUF, (char*)&rcvbuf, &optlen);
	rcvbuf = rcvbuf<<4;
	::setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(const char*)&rcvbuf,optlen);

	my_addr.sin_family = AF_INET; 
	my_addr.sin_port = htons(atoi(argv[1])); 
	my_addr.sin_addr.s_addr = INADDR_ANY; 
	memset(&(my_addr.sin_zero), '\0', 8); 

	bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
	sin_size = sizeof(struct sockaddr_in);

	listen(sockfd, BACKLOG);
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
