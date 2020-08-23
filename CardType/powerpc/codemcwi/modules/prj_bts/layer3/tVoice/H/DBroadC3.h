/*******************************************************************************
* Copyright (c) 2010 by Beijing AP Electrical Co.Ltd.All Rights Reserved   
* File Name      : DBroadCastStruct.h
* Create Date    : 21-Jun-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__DBROADCASTSTRUCT_H
#define	__DBROADCASTSTRUCT_H
#include "datatype.h"

//SABIS1接口在sabis信令结构处定义

//BTS-DCS接口
typedef struct _DB_BtsDcsCtrlMsgHeadT
{
	UINT8 DBMessagetype;
}DB_BtsDcsCtrlMsgHeadT;

//DB Ready
typedef struct _DBReadyT
{
	UINT8 DBMessagetype;
	UINT8 GID[2];
	UINT8 DB_SN;
}DBReadyT;

//DB data req
typedef struct _DBDataReqT
{
	UINT8 DBMessagetype;
	UINT8 GID[2];
	UINT8 DB_SN;
	UINT8 MemoryCap[2];
}DBDataReqT;

//DB CReady
typedef struct _DBCReadyT
{
	UINT8 DBMessagetype;
	UINT8 GID[2];
	UINT8 DB_SN;
	UINT8 Servicetype:4;
	UINT8 Rev1:4;
	UINT8 Erasure:1;
	UINT8 ErasureCodeType:2;
	UINT8 Rev2:5;
}DBCReadyT;
		
//UT-DCS Control message
typedef struct _DB_UtDcs_CtrlMsgHeadT
{
	UINT8 DBMessagetype;
	UINT8 Subtype;
	UINT8 UID[4];	
}DB_UtDcs_CtrlMsgHeadT;

//UT-DCS User Data
typedef struct _DB_BtsDcsDataMsgHeadT
{
	UINT8 DBDatatype;
}DB_BtsDcsDataMsgHeadT;

typedef struct _DB_UnicastDataT
{
	UINT8 DBDatatype:2;
	UINT8 Rev1:6;
	UINT8 UID[4];
	UINT8 DB_SN;
	UINT8 payload[1];
}DB_UnicastDataT;

typedef struct _DB_BroadcastDataT
{
	UINT8 DBDatatype:2;
	UINT8 Rev1:6;
	UINT8 GID[2];
	UINT8 DB_SN;
	UINT8 payload[1];
}DB_BroadcastDataT;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __DBROADCASTSTRUCT_H */


