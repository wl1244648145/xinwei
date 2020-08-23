/*******************************************************************************
* Copyright (c) 2010 by AP Co.Ltd.All Rights Reserved   
* File Name      : DBroadCastSrv.h
* Create Date    : 5-Aug-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__DBROADCASTSRV_H
#define	__DBROADCASTSRV_H

#include "DBroadCastCommon.h"

typedef struct _DcsCountersT
{
	UINT32 cntTxSignal[DB_signalMax];
	UINT32 cntRxSignal[DB_signalMax];
	UINT32 cntUidDataFromDCS;
	UINT32 cntUidDataToCPE;
	UINT32 cntUidDataFromCPE;
	UINT32 cntUidDataToDCS;
	UINT32 cntGidDataFromDCS;
	UINT32 cntGidDataToL2;
}DcsCountersT;
extern DcsCountersT dcsCounters;

#ifdef __cplusplus
extern "C" {
#endif

void showDcsCounters();
void clearDcsCounters();

#ifdef __cplusplus
}
#endif

#endif /* __DBROADCASTSRV_H */


