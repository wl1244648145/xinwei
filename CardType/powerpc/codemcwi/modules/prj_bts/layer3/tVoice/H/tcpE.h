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

//TCP�ӿڶ���
#define M_TCP_PKT_BEGIN_FLAG				(0x7ea5)	//HEAD FALGȡֵΪ0x7ea5
#define M_TCP_PKT_END_FLAG				(0x7e0d)	//END FLAGȡֵΪ0x7e0d
#define M_TCP_PKT_SABIS1_USERTYPE			(0x1106)	//�û����ͣ�SAbis1�ӿڹ̶�Ϊ0x1106
#define M_TCP_PKT_TEST_USERTYPE			(0x1110)	//TCP��װ����ģ�������Ϣ��0x1110
#define M_TCP_PKT_DGRP_CTRL_USERTYPE 	(0x1109) //McBTS��DCS�ӿ�Ӧ�ò����Э�飺0x1109
#define M_TCP_PKT_DGRP_DATA_USERTYPE 	(0x110A) //McBTS��DCS�ӿ�Ӧ�ò����ݣ�0x110A


#define M_TCP_PKT_TEST_AUTH_REQ			(0x02)		//TCP������֤����
#define M_TCP_PKT_TEST_AUTH_RSP			(0x03)		//TCP������֤Ӧ��

#define M_TCP_PKT_TEST_HEARTBEAT_REQ	(0x00)		//TCP������������
#define M_TCP_PKT_TEST_HEARTBEAT_RSP	(0x01)		//TCP��������Ӧ��

#define M_TCP_PKT_DEFAULT_TTL			(32)		//·�ɼ�������Ϊ��ֹTCP��װ���û�·�ɴ������ѭ��·�ɣ��˼�����ÿ����һ���ڵ��һ����Ϊ0��˰������������˼�������Ĭ��ֵΪ32��
#define M_TCP_DEFAULT_SPC_CODE (0xffff)
#define M_TCP_DEFAULT_DPC_CODE (0xffff)

typedef UINT16 HeadFlagT;
typedef UINT16 EndFlagT;

typedef struct tagTcpPktHeader
{
//	HeadFlagT	HeadFlag;	//HeadFlag
	UINT8		PktLen[2];		//TCP���ĳ��ȣ�����PktLen����������HeadFlag��EndFlag
	UINT8		DPC_Code[2];	//Ŀ����������
	UINT8		SPC_Code[2];	//Դ��������
	UINT8		UserType[2];	//�û����ͣ�SAbis1�ӿڹ̶�ΪM_TCP_PKT_SABIS1_USERTYPE
	UINT8		TTL[2];			//·�ɼ�������Ϊ��ֹTCP��װ���û�·�ɴ������ѭ��·�ɣ��˼�����ÿ����һ���ڵ��һ����Ϊ0��˰������������˼�������Ĭ��ֵΪ32��
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


