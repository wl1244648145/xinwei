#ifndef __BPS_TFTPS_H__
#define __BPS_TFTPS_H__


//�ļ���������˿�
#define PORT_TFTPS     8069

#define SOCKET_TYPE_TCP  	0x0
#define SOCKET_TYPE_UDP		0x1

//#define TFTPS_SOCKET_TYPE SOCKET_TYPE_TCP
#define TFTPS_SOCKET_TYPE SOCKET_TYPE_UDP

/***********************************************************************
*�������ƣ�int init_tftp_server_thread(char *path)
*�������ܣ���ʼ���ļ����ط�����
*����������  ������             ����
*           char *path       �ļ����·��
*�������أ���
***********************************************************************/
void init_tftp_server_thread(char *path);

#endif //__BPS_TFTPS_H__
