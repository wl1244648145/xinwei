#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

struct fileinfo
{
	int fileno;//文件号
	int type;//客户端想说什么（前面那两句话，用1,2表示）
	long len;//文件长度
	int seek;//文件开始位置，用于多线程 

	char name[100];//文件名
}; 