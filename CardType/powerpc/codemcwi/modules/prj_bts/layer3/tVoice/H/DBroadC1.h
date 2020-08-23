/*******************************************************************************
* Copyright (c) 2010 by AP Co.Ltd.All Rights Reserved   
* File Name      : DBroadCastMsg.h
* Create Date    : 5-Aug-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__DBROADCASTMSG_H
#define	__DBROADCASTMSG_H

#include "DBroadCastStruct.h"
#include "DBroadCastCommon.h"
#include "message.h"

typedef struct _DcsSignalDictionaryItemT
{
	UINT8 msgType;
	UINT8 subType;
	char name[40];
}DcsSignalDictionaryItemT;
extern DcsSignalDictionaryItemT DcsSignalDictionary[DB_signalMax];


int parseSignal(CMessage& msg);
int parseSignal(CComMessage *pMsg);
int parseSignal(unsigned char *pBuf, unsigned short len);


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif /* __DBROADCASTMSG_H */


