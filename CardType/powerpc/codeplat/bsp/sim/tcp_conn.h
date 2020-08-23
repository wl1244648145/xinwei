#ifndef TCP_CONNECT_H
#define TCP_CONNECT_H

#include "mysocket.h"
int tcp_connect(const char *host,const char *service)
{
	struct hostent *hstent;
	struct servent *svrent;
	struct sockaddr_in sin;
	int s;
	memset(&sin,0,sizeof(sin));
	sin.sin_family = AF_INET;
	svrent = getservbyname(service,NULL);
	if(svrent)
	{
		sin.sin_port = svrent->s_port;	
	}
	else if((sin.sin_port = htons((unsigned short)atoi(service))) == 0 )
	{
		return -4;	
	}
	hstent = gethostbyname(host);
	if(svrent)
	{
		memcpy(&sin.sin_addr,hstent->h_addr,hstent->h_length);
	}
	else if((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE)
	{
		return -3;
	}
	s = socket(AF_INET,SOCK_STREAM,0); 
	if(s < 0)
	{
		return -2;	
	}
	if(connect(s,(struct sockaddr *)&sin,sizeof(sin)) < 0)
	{
		return -1;
	}
	return s;
}

#endif
