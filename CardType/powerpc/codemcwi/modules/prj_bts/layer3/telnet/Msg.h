/*-----------------------------------------------------------
    MPUMsg.h -  MPU应用程序公共部分消息机制部分头文件

    版权所有 北京信威公司

    修改历史记录
    --------------------
    2005-11-08,      金伟民      创建
-----------------------------------------------------------*/

#ifndef __MSG_H__
#define __MSG_H__



#ifdef  __cplusplus
extern  "C"
{
#endif

/*****************************************************************************/

#define MAX_MSG_LEN		0x200
#define SYS_MSG_HEAD_LEN   6

/*****************************************************************************/

#pragma pack(1)

/*公共消息头结构*/
typedef struct tagCommonHeader
{
	_UCHAR       ucDstTaskNo;    /*接收者*/
	_UCHAR	     ucSrcTaskNo;    /*发送者*/
	_USHORT      usMsgId;        /*消息号*/
	_USHORT      usMsgLen;       /*消息长度*/
}PKD COMMON_HEADER;
	
/*长消息结构*/
typedef struct tagLongMessage
{
	COMMON_HEADER   stHeader;
	_UCHAR          ucBuffer[MAX_MSG_LEN-sizeof(COMMON_HEADER)];
}PKD LONG_MESSAGE;




#define    IP_FSM_CLI_FIRST_CLIENT    12      /*telnet 第一个客户端*/
#define    IP_FSM_CLI_SERVER   IP_FSM_CLI_FIRST_CLIENT + 7 //命令行的侦听服务端
#define    MAX_IP_FSM     IP_FSM_CLI_SERVER+1    

/*所有任务的名称的宏定义*/



#define IP_TASK_ID      0x06
#define CLI_TASK_ID     0x0F
#define TASK_NO_CLCK    0x10

#define    MSG_IP_CONFIG                  1        /*配置          OAM->IP*/
#define    MSG_IP_ADDRCHANGE        2          /*IP地址改变   OAM->IP*/
#define    MSG_IP_CONNECTIND	   3	/*连接指示      IP->USER*/
#define    MSG_IP_CLOSEREQ              4 /*关闭请求      USER->IP*/
#define    MSG_IP_CLOSEIND              5 /*关闭指示      IP->USER*/
#define    MSG_IP_DATAREQ               6 /*数据请求      USER->IP*/
#define    MSG_IP_DATAIND               7 /*数据指示      IP->USER*/





#ifdef  __cplusplus
}
#endif

#endif
