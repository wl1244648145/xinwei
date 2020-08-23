/*******************************************************************************
* Copyright (c) 2010 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : tcpE.h
* Create Date    : 22-Mar-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__TCPE_H
#define	__TCPE_H

//TCP接口定义
#define M_TCP_PKT_BEGIN_FLAG				(0x7ea5)	//HEAD FALG取值为0x7ea5
#define M_TCP_PKT_END_FLAG				(0x7e0d)	//END FLAG取值为0x7e0d
#define M_TCP_PKT_SABIS1_USERTYPE			(0x1106)	//用户类型，SAbis1接口固定为0x1106
#define M_TCP_PKT_TEST_USERTYPE			(0x1110)	//TCP封装功能模块测试消息：0x1110
#define M_TCP_PKT_DGRP_CTRL_USERTYPE 	(0x1109) //McBTS与DCS接口应用层控制协议：0x1109
#define M_TCP_PKT_DGRP_DATA_USERTYPE 	(0x110A) //McBTS与DCS接口应用层数据：0x110A


#define M_TCP_PKT_TEST_AUTH_REQ			(0x02)		//TCP连接认证请求
#define M_TCP_PKT_TEST_AUTH_RSP			(0x03)		//TCP连接认证应答

#define M_TCP_PKT_TEST_HEARTBEAT_REQ	(0x00)		//TCP连接握手请求
#define M_TCP_PKT_TEST_HEARTBEAT_RSP	(0x01)		//TCP连接握手应答

#define M_TCP_PKT_DEFAULT_TTL			(32)		//路由计数器：为防止TCP封装层用户路由错误出现循环路由，此计数器每经过一个节点减一，若为0则此包将被丢弃。此计数器的默认值为32。
#define M_TCP_DEFAULT_SPC_CODE (0xffff)
#define M_TCP_DEFAULT_DPC_CODE (0xffff)

typedef UINT16 HeadFlagT;
typedef UINT16 EndFlagT;

typedef struct tagTcpPktHeader
{
//	HeadFlagT	HeadFlag;	//HeadFlag
	UINT8		PktLen[2];		//TCP报文长度，包括PktLen本身，不包含HeadFlag和EndFlag
	UINT8		DPC_Code[2];	//目的信令点编码
	UINT8		SPC_Code[2];	//源信令点编码
	UINT8		UserType[2];	//用户类型，SAbis1接口固定为M_TCP_PKT_SABIS1_USERTYPE
	UINT8		TTL[2];			//路由计数器：为防止TCP封装层用户路由错误出现循环路由，此计数器每经过一个节点减一，若为0则此包将被丢弃。此计数器的默认值为32。
}TcpPktHeaderT;

typedef struct tagTcpPktTestAuthReq
{
	UINT8	msgType;
	UINT8	RAND[4];
}TcpPktTestAuthReqT;

typedef struct tagTcpPktTestAuthRsp
{
	UINT8	msgType;
	UINT8	Result[4];
}TcpPktTestAuthRspT;

typedef struct tagTcpPktTestBearHeartReq
{
	UINT8	msgType;

}TcpPktTestBearHeartReqT;

typedef struct tagTcpPktTestBearHeartRsp
{
	UINT8	msgType;

}TcpPktTestBearHeartRspT;


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __TCPE_H */


