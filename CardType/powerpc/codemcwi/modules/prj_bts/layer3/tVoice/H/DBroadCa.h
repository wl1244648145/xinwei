/*******************************************************************************
* Copyright (c) 2010 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : DBroadCastCommon.h
* Create Date    : 21-Jun-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__DBROADCASTCOMMON_H
#define	__DBROADCASTCOMMON_H
#include "datatype.h"

typedef enum
{
	DB_InvalidSignal=0,
	DB_READY,
	DB_DataReq,
	DB_CReady,
	MO_ReceiveReady,
	MO_ReceiveComplete,
	DB_SingleReady,
	DB_SingleDataReq,
	DB_SingleDataComplete,

	DB_signalMax
}ENUM_DB_SIGNAL_T;

typedef enum
{
	MSGTYPE_DB_Ready=0,
	MSGTYPE_DB_DataReq,
	MSGTYPE_DB_CReady,
	MSGTYPE_DB_UIDMsg,

	MSGTYPE_DB_MAX,
	DB_UtDcs_UID_MSG=255
}ENUM_DB_MSGTYPE_T;

typedef enum
{
	SUBTYPE_MO_ReceiveReady=0,
	SUBTYPE_MO_ReceiveComplete,
	SUBTYPE_DB_SingleReady=4,
	SUBTYPE_DB_SingleDataReq,
	SUBTYPE_DB_SingleDataComplete,

	SUBTYPE_DB_MAX
}ENUM_DB_SUBTYPE_T;

typedef enum
{
	DataType_GrpBroadcastData=0,
	DataType_UnicastData
}ENUM_DB_DATATYPE_T;

typedef enum
{
	DBSrvType_SmallData=0,
	DBSrvType_SMS,
	DBSrvType_File,
	DBSrvType_StreamMedia
}ENUM_DB_SRVTYPE_T;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __DBROADCASTCOMMON_H */


