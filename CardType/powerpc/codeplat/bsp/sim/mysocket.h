/* 
File Name:	mysocket.h
Description:	This is a common head file that can be used in socket program.
Date:2005/3/25
Author:
*/

#ifndef _MYSOCKET_H
#define _MYSOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#define LISTENQ 10

#ifndef MAXLINE
#define MAXLINE 4096*2
#endif

#define BUFFERSIZE 8192*2

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

ssize_t readn(int fd,void *buf,size_t nbytes);
ssize_t writen(int fd,const void *buf,size_t nbytes);
ssize_t read_line(int fd,void *vptr,size_t maxlen);

#endif
