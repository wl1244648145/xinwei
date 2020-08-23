/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    L3DataSocketTable.h
*
* DESCRIPTION: 
*
* HISTORY:
*
*   Date       Author        Description
*   ---------  ------        ------------------------------------------------
*   11/01/06   xinwang		 initialization. 
*
*---------------------------------------------------------------------------*/
#ifndef _INC_TSOCKET_TABLE
#define _INC_TSOCKET_TABLE

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_TSOCKET_MSGS
#include "L3DataSocketMsgs.h"
#endif

//转发表项
#pragma pack (1)
typedef struct _tag_BtsAddr
{
	UINT32      IP;
	UINT16      Port;
	UINT16		TTL;
} BtsAddr;
#pragma pack ()


/****************************
*stDataNVRAMhdr:NVRAM的
*CCB前存储的上次启动前
*系统的有关配置
*/
typedef struct 
{
	UINT32 Initialized;
	UINT32 LocalBtsId;
}stDataSocketNVRAMhdr;


/****************************
*NVRamFCB结构: 
****************************/
typedef struct 
{
    bool   IsOccupied;
	UINT32 BtsId; 
	UINT32 IP;
	UINT16 Port;
}stNVRamFCB;


/***************************
*tSocket BtsIp Notification
*Control Block
**************************/
#define BtsAddrNodifyMsgBufLen /*100*/M_DEFAULT_RESERVED+sizeof(SocketEmsMsgHeader)+sizeof(SocketBtsIpNotification)
//#pragma pack (1)
typedef struct _tag_BtsAddrNotifyCB
{
	UINT32 Btsid;
	UINT8  Count;
	CTimer *pTimer;
	UINT32 DestIpAddr;
	UINT16 DestPort;
	UINT32 Length;
	//存储待发送的BtsIpNotification请求消息，
	//内容空间按最大的请求消息大小申请
	UINT8  Buf[BtsAddrNodifyMsgBufLen];
} BtsAddrNotifyCB;
//#pragma pack ()
//////////////////////////////////////////////////////////////////////////
//Ems message map
#pragma pack(1)
typedef struct _tag_EmsMessageMapEntry
{
	UINT8  bIsSend;
	UINT16 ma;
	UINT16 moc;
	UINT16 action;
	UINT16 msgid;
	TID tid;
}EmsMessageMapEntry;
#pragma pack()

//////////////////////////////////////////////////////////////////////////
//Jamming转发使用
//////////////////////////////////////////////////////////////////////////
//Ems message map
#pragma pack(1)
typedef struct _tag_JammingNeighborEntry
{
	UINT32 BtsId;
	UINT16 Seq;
	UINT32 IP;
	UINT16 Port;
}JammingNeighborEntry;
#pragma pack()

extern const EmsMessageMapEntry g_EmsMessageMap[];

#endif/*_INC_TSOCKET_TABLE*/