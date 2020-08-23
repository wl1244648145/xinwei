/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    L3DataSocketMsgs.h
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
#ifndef _INC_TSOCKET_MSGS
#define _INC_TSOCKET_MSGS
#pragma pack(1)
typedef struct _tag_SocketMsgArea
{
	UINT16 MsgArea;
}SocketMsgArea;

typedef struct _tag_SocketEmsMsgHeader
{
	UINT32 btsid;
	UINT16 ma;
	UINT16 moc;
	UINT16 action;
}EmsMsgHeader;

typedef struct _tag_SockethlrMsgHeader
{
	UINT32 btsid;
	UINT16 ma;
	UINT16 moc;
	UINT16 action;
	UINT16 transId;
	UINT32 period;
}HlrMsgHeader;
typedef EmsMsgHeader SocketEmsMsgHeader ;
//////////////////////////////////////////////////////////////////////////
//btsip notification to ems
typedef struct _tag_SocketBtsIpNotification
{
	UINT16 TransId;
	UINT32 BtsId;
}SocketBtsIpNotification;

typedef struct _tag_SocketBtsIpRequest
{
	UINT16 TransId;
	UINT32 BtsId;
	UINT32 IP;
	UINT16 Port;
}SocketBtsIpRequest;

typedef struct _tag_SocketBtsIpResponse
{
	UINT16 TransId;
	UINT16 Result;
}SocketBtsIpResponse;


//for Jamming forwarding
typedef struct
{
#if defined NUCLEUS_PORT || defined WIN32
	UINT16 RSV:8;
	UINT16 Type:5;
	UINT16 Entity:3;
#else
	UINT16 Entity:3;    
	UINT16 Type:5;  
	UINT16 RSV:8;
#endif    
}stMacContrlInfo;

typedef struct
{
#if defined NUCLEUS_PORT || defined WIN32
	UINT16 		Rsv1:1;
	UINT16 		OBS1:5;
	UINT16 		OBS0:5;
	UINT16 		ScgMask:5;
#else
	UINT16 		ScgMask:5;
	UINT16 		OBS0:5;
	UINT16 		OBS1:5;
	UINT16 		Rsv1:1;
#endif
}stOBS;

//mq lrc 100121
typedef struct
{
#if defined NUCLEUS_PORT || defined WIN32 || defined BF_NU_L2
    UINT16      Rsv1:1;//new OBS Flag ,0-old mode , 1-new mode
    UINT16      Rsv0:2;
    UINT16      SeqIdMaskLow:8;
    UINT16      ScgMask:5;
#else
    UINT16      ScgMask:5;
    UINT16      SeqIdMaskLow:8;
    UINT16      Rsv0:2;
    UINT16      Rsv1:1;//new OBS Flag ,0-old mode , 1-new mode
#endif
}stOBSNew;


typedef struct
{
	stMacContrlInfo macCtrlInfo;
	UINT16 		Fn;       
	UINT16 		TransId; 
	stOBS 	    OBSInfo;   	
}stCpeJammingRptHdr;

typedef struct
{
	stMacContrlInfo macCtrlInfo;
	UINT16 		Fn;       
	UINT16 		TransId; 
	stOBS 	    OBSInfo;   
	UINT16 		DlCiMask[5][8];	
	UINT16 		CRC;
}stCpeJammingRpt;

typedef struct
{
	UINT16 		transId;
	UINT32 		srcBtsId;       
	UINT32 		cpeId;    
	stCpeJammingRpt cpeJammingRptInfo;	
	UINT16      schCurMask[5][8];
}stBtsJammingRptL3;

typedef struct
{
	UINT16 		transId; 
	UINT32 		dstBtsId;       
	UINT32 		srcBtsId;	  
	UINT16 		seqId;
	UINT32      cpeId;
	UINT32 	    pairedCpeId;
	UINT16      pairedCpeFbdMaskFlag;
}stBtsJammingRptRspL3;

typedef struct
{
	UINT16 		transId;
	UINT32 		dstBtsId;       
	UINT32 		srcBtsId;        	   
	UINT16 		seqId;
	UINT32      cpeId;
	UINT16 		scgMask;
	UINT16 		schMask[5][8];
	UINT16      bwReqDl;
    	UINT16      bwReqUl;
	UINT16      cpeNum;
	UINT32 		pairedCpeIdArr[5];
}stBtsPairedCpeProfMsgL3;


#if 1//def M_CLUSTER_SAME_F
typedef struct 
{
    UINT16  L2MsgId;
    UINT16  freqIndex; //源基站中心频点
    UINT16  seqId;//源基站seqId
    UINT16  allocatedSchMask[5][8]; //已占用组呼资源
    UINT16  rsvSchMask[5][8]; //预留组呼资源
    
    //相邻基站信息，用于三层转发消息
    UINT16 OBSNum;
    UINT16 OBSSeqId[15];
    
}stBtsGroupResourceRptRsp;

#endif
#pragma pack()

#endif/*_INC_TSOCKET_MSGS*/
