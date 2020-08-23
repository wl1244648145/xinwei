/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMessages.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   03/28/06   xiao weifang  FixIp用户做成非永久性用户
 *   08/01/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __L3_DATAMESSAGE_H__
#define __L3_DATAMESSAGE_H__

#include "L3DataTypes.h"
#include "L3DataCommon.h"

#pragma pack (1)

/*******************************
 *stDataConfig:数据配置
 *消息格式,由OAM发往Bridge任务
 *******************************/
typedef struct _tag_stDataConfig
{
    UINT16      usTransId;
    UINT8       ucEgressBCFltr;
    UINT16      usLearnedBridgingAgingTime;
    UINT16      usPPPoESessionKeepAliveTime;
    UINT8       ucWorkMode; 
} stDataConfig;


/*******************************
 *stDataConfigResp:
 *数据配置回应消息
 ******************************/
typedef struct _tag_msgDataConfigResp
{
    UINT16      usTransId;
    UINT16      usResult;       //result.
} stDataConfigResp;


/**********************************
 *stTosSFIDConfig:配置TOS & SFID
 *消息结构
 *保持和CPE上的定义一致
 **********************************/
typedef struct _tag_stTosSFIDConfig
{
    UINT16      usTransId;
    UINT8       aSFID[M_TOS_SFID_NO];
} stTosSFIDConfig;


/*************************************
 *stTosSFIDConfigResp:配置TOS & SFID
 *回应消息结构
 *保持和CPE上的定义一致
 *************************************/
typedef struct _tag_stTosSFIDConfigResp
{
    UINT16      usTransId;
    UINT16      usResult;       //result.
} stTosSFIDConfigResp;


/*******************************************
 *stFTEntryExpire:转发表表项超时消息格式
 *由CleanUp任务发往Bridge任务
 *******************************************/
typedef struct _tag_stFTEntryExpire
{
    UINT8       aucMac[M_MAC_ADDRLEN];
	UINT8       Dm_Sync_Flag;
} stFTEntryExpire;


/*******************************************
 *stFTAddEntry:增加转发表表项消息格式
 *由Snoop任务发往Bridge任务
 *******************************************/
typedef struct _tag_stFTAddEntry
{
    UINT32      ulEid;
    UINT8       aucMac[M_MAC_ADDRLEN];
    bool        bIsServing;
    bool        bIsTunnel;
    UINT32      ulPeerBtsID;
    UINT8       ucIpType;
    UINT16      usGroupID;
    bool        bIsAuthed;
} stFTAddEntry;


/*******************************************
 *stFTDelEntry:删除转发表表项消息格式
 *由Snoop任务发往Bridge任务
 *******************************************/
typedef struct _tag_stFTDelEntry
{
    UINT8   aucMac[M_MAC_ADDRLEN];
    bool    bCreateTempTunnel;
    UINT32  ulPeerIP;
    UINT16  usPeerPort;
} stFTDelEntry;

/*******************************************
*stMFTEntryExpire:MAC Filtering Table超时消息
*由CleanUp任务发往Bridge任务
*******************************************/
typedef struct _tag_stMFTEntryExpire
{
	UINT8       TYPE;
	UINT8       MAC[M_MAC_ADDRLEN];
} stMFTEntryExpire;

/*******************************************
*stMFTAddEntry:增加Mac Filter表表项消息格式
*******************************************/
typedef struct _tag_stMFTAddEntry
{
	UINT8       TYPE;
	UINT8       MAC[M_MAC_ADDRLEN];
} stMFTAddEntry;

/*******************************************
*stMFTDelEntry:删除Mac Filter表表项消息格式
*******************************************/
typedef struct _tag_stMFTDelEntry
{
	UINT8   TYPE;
	UINT8   MAC[M_MAC_ADDRLEN];
} stMFTDelEntry;

/*******************************************
 *stRAIDConfig:RAID配置消息格式
 *由OAM发往Snoop任务
 *******************************************/
typedef struct _tag_stRAIDConfig
{
    UINT16      usTransId;
    UINT8       ucAgentCircuitID;
    UINT8       ucAgentRemoteID;
    UINT8       ucPPPoERemoteID;	
    UINT32      ulRouterAreaId;
} stRAIDConfig;


/*******************************************
 *stRAIDConfigResp:RAID配置消息格式
 *由OAM发往Snoop任务
 *******************************************/
typedef struct _tag_stRAIDConfigResp
{
    UINT16      usTransId;
    UINT16      usResult;       //result.
} stRAIDConfigResp;


/*******************************************
 *stSyncIL:同步Ip List的消息结构
 *******************************************/
typedef struct _tag_stSyncIL
{
    UINT32      ulEid;
    UINT8       ucOp;
    UINT8       ucIpType;
    bool        bNeedResp;  /*true,when need response*/
    UTILEntry   Entry;
} stSyncIL;


/*******************************************
 *stSyncILResp:同步Ip List的回应消息结构
 *******************************************/
typedef struct _tag_stSyncILResp
{
    UINT32      ulEid;
    UINT8       ucIpType;
    UINT8       aucMac[ M_MAC_ADDRLEN ];
    bool        bSuccess;       //result.
} stSyncILResp;


/*******************************************
 *stTunnelReq: Tunnel Request消息结构
 *******************************************/
typedef struct _tag_stTunnelReq
{
    UINT16      usMsgCode;      //NBO
    UINT32      ulEid;          //NBO
    UINT8       aucMac[ M_MAC_ADDRLEN ];  
	UINT32      ulDstBtsID;
/*
	UINT32      ulDstBtsIP;       //NBO
    UINT16      usDstPort;
    */
    UINT32      ulSenderBtsID;    //NBO
    UINT32      ulSenderBtsIP;    //NBO
    UINT16      usSenderPort;
    UINT8       ucIpType;
} stTunnelReq;


/*******************************************
 *stTunnelSync: Tunnel Sync消息结构
 *******************************************/
struct stTunnelSync:stTunnelReq
{
    DATA        Data;
};


/*******************************************
 *stTunnelEstablish: Tunnel Establish消息结构
 *******************************************/
struct stTunnelEstablish:stTunnelReq
{
    UINT32      ulFixIp;        //NBO
    UINT16      usGroupId;      //NBO
};


/*******************************************
 *stTunnelResp:Tunnel Response消息结构
 *******************************************/
typedef struct _tag_stTunnelResp
{
    UINT16      usMsgCode;      //NBO
    UINT32      ulEid;          //NBO
    UINT8       aucMac[ M_MAC_ADDRLEN ];
    UINT32      ulDstBtsID;       //NBO
    UINT32      ulDstBtsIP;       //NBO
    UINT16      ulDstPort;       //NBO
    #ifndef WBBU_CODE
    bool        bSuccess;       //result.
    #else
	int        bSuccess;
	#endif
} stTunnelResp;


/*******************************************
 *stDelEidTable:删除EID表消息结构
 *******************************************/
typedef struct _tag_stDelEidTable
{
    UINT32      ulEid;
} stDelEidTable;


/*******************************************
 *stFTCheckVLAN: 检查VLAN ID的消息结构
 *******************************************/
typedef struct _tag_stFTCheckVLAN
{
    UINT16      usVlanID;
} stFTCheckVLAN;

/*******************************************
 *stNotifyBTSPubIP: 更新动态的BSPUB-IP&Port消息结构
 *******************************************/
typedef struct _tag_stNotifyBTSPubIP
{
	UINT32      ulBtsID;
	UINT32      ulBtsIp;
	UINT16      usBtsPort;
} stNotifyBTSPubIP;

typedef struct _tag_stNotifyRefreshJammingNeighbor
{
	UINT32      flag;//0:更新BtsFreq, 1:更新NeighborList
} stNotifyRefreshJammingNeighbor;

#pragma pack ()
#endif /*__L3_DATAMESSAGE_H__*/
