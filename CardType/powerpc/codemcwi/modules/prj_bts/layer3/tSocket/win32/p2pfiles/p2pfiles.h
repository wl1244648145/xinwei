#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif
#include "stdio.h"

struct fileinfo
{
	int fileno;//�ļ���
	int type;//�ͻ�����˵ʲô��ǰ�������仰����1,2��ʾ��
	long len;//�ļ�����
	int seek;//�ļ���ʼλ�ã����ڶ��߳� 

	char name[100];//�ļ���
}; 