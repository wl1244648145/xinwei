/* 
File Name:	mysocket.c
Description:	defination of the basic functions that were declared in mysocket.h
Date:2005/3/25
Author:
*/

#include "mysocket.h"

ssize_t 
readn(int fd,void *buf,size_t nbytes)
{
	size_t nleft = nbytes;
	char *ptr = buf;
	ssize_t nread;

	while(nleft > 0)
	{
		if((nread = read(fd,ptr,nleft)) < 0)	
		{
			if(errno == EINTR) nread = 0; //invoke read() again.
			else return (-1);
		}else if(nread == 0)
		{
			break;	//end of file
		}

		nleft -= nread;
		ptr += nread;
	}
	return (nbytes - nleft); 
}

ssize_t 
writen(int fd,const void *buf,size_t nbytes)
{
	size_t nleft = nbytes;
	const char *ptr = buf;
	ssize_t nwritten;
	
	while( nleft > 0)
	{
		if((nwritten = write(fd,ptr,nleft)) <= 0)	
		{
			if(nwritten < 0 && errno == EINTR)	
			{
				nwritten = 0;	//invoke write() again.
			}else
			{
				return (-1);	//error!
			}
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return (nbytes);
}


static int read_cnt;
static char *read_ptr = NULL;
static char read_buf[MAXLINE];

static ssize_t
n_read(int fd,char *ptr)
{
	if(read_cnt <= 0)
	{
	again:
		if((read_cnt = read(fd,read_buf,sizeof(read_buf))) < 0)
		{
			if(errno == EINTR)
			{
				goto again;	
			}
			return (-1);
		}else if(read_cnt == 0)
		{
			return (0);	
		}
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return (1);
}

ssize_t 
read_line(int fd,void *vptr,size_t maxlen)
{
	ssize_t n,rc;
	char c,*ptr;

	ptr = vptr;
	for(n = 1;n < maxlen;n++)
	{
		if((rc = n_read(fd,&c)) == 1)
		{
			*ptr++ = c;
			if(c == '\n')  break;	//new line is stored.
		}else if(rc == 0)
		{
			*ptr = 0;	
			return (n - 1); //end of file,n - 1 bytes were read.
		}else
		{
			return (-1); //error occured.	
		}
	}

	*ptr = 0;
	return (n);
}

