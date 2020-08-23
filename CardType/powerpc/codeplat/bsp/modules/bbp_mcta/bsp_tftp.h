#ifndef __BPS_TFTPS_H__
#define __BPS_TFTPS_H__


//文件传输命令端口
#define PORT_TFTPS     8069

#define SOCKET_TYPE_TCP  	0x0
#define SOCKET_TYPE_UDP		0x1

//#define TFTPS_SOCKET_TYPE SOCKET_TYPE_TCP
#define TFTPS_SOCKET_TYPE SOCKET_TYPE_UDP

/***********************************************************************
*函数名称：int init_tftp_server_thread(char *path)
*函数功能：初始化文件下载服务器
*函数参数：  参数名             描述
*           char *path       文件存放路径
*函数返回：无
***********************************************************************/
void init_tftp_server_thread(char *path);

#endif //__BPS_TFTPS_H__
