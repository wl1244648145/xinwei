/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    voiceData.h
*
* DESCRIPTION: 
*		BTS上的Voice语音数据功能相关
* HISTORY:
*
*   Date       Author         Description
*   ---------  ------        ----------------------------------------------------
*   2005-01-25 fengbing  initialization. 
*
*---------------------------------------------------------------------------*/

#ifndef	__VOICE_DATA_H
#define	__VOICE_DATA_H


#include "voiceCommon.h"
#include "VAC_Voice_Data.h"
#include "VDR_Voice_Data.h"

#define M_G729B_SRTP_DATALEN	(2)			//G729B srtp data length
#define  M_G729_10MS_DATALEN	10			//G729 10ms DATA length
#define  M_G711_10MS_DATALEN	80			//G711 10ms DATA length
#define M_MAX_10MS_DATALEN	(M_G711_10MS_DATALEN)
#define UPLINK_CODEC_FRAMES		(2)

#define M_ENC_VDATA_PKT_FRAME_NUM (4)
#define M_ENC_VDATA_KEY_LEN	(10)
#define M_ENC_VDATA_PKT_LEN	(M_ENC_VDATA_KEY_LEN+M_ENC_VDATA_PKT_FRAME_NUM*M_G729_10MS_DATALEN)

//#define JITTER_FRAMES			(32)
#define M_MAX_JITTER_FRAMES		(256)
#define M_MAX_CALLS			(300)
#define M_MAX_SRTP_PKT_LEN	(1452)		//最大srtp的udp包大小
#define M_INVALID_VDATABUF_IDX	(0xffff)
#define M_INVALID_10MS_DATALEN (0xff)
#define M_MAX_BUFFER_FAXDATA_FRAMES (64)
#define M_MIN_BUFFER_FAXDATA_FRAMES (5)

typedef struct _FaxData10msFrame
{
	UINT8 Cid:5;
	UINT8 Codec:3;//fax data is invalid when this field is CODEC_G729A
	UINT8 Sn:8;
	UINT8 Payload[M_G711_10MS_DATALEN];
}FaxData10msFrameT;

#define M_FAX_ULDATA_RXTX_MIN_DISTANCE (5)
#define M_FAX_ULDATA_RXTX_MAX_TOONEAR_TIMES (15)
#define M_FAX_ULDATA_TX_MOVEBACK_DISTANCE (20)

typedef struct _FaxDataBufferT
{
	FaxData10msFrameT faxDataUlBuf[M_MAX_BUFFER_FAXDATA_FRAMES];
	UINT32 firstDataArriveFN;
	UINT8 firstDataSN;
	
	UINT16 nUlDataFrmsInBuf;
	UINT16 nUlDataTxRxTooNear;
	
	UINT8 blUlTxDataStarted;
	UINT8 curUlTxSN;
}FaxDataBufferT;

typedef struct _JitterItemT
{
	UINT8	len;
	VACVoiceDataHeaderT	VACItemHead;
	UINT8	voiceData[M_MAX_10MS_DATALEN];
}JitterItmeT;

//uplink buffer 20ms buffer, srtp format
typedef struct _upLinkBufferT
{
	UINT8	blStarted;
	UINT8	timeStampLen;
	UINT8	len;
	UINT8	curBufId;
	DMUXVoiDataCommonT srtpItmeHead;
	UINT8	voiceData[M_MAX_10MS_DATALEN];
}upLinkBufT;

//downlink buffer 80ms buffer, VACDataIE format
typedef struct _downLinkBufferT
{
	UINT32	nTimeStampNew;
	UINT8	blStarted;
	//UINT8	curRcvSN;
	UINT8	curSndSN;
	//UINT8	curRcvIdx;	//jitter buffer idx
	UINT8	curSndIdx;	//jitter buffer idx
	UINT16	nRcvFrames;
	UINT8	nInvalidSNCounter;	//连续的下行不合要求的SN计数，不能存在jitterbuffer中，需要丢弃的包
	//UINT16	nTmp;
//	JitterItmeT	jitterBuffer[JITTER_FRAMES];
	JitterItmeT jitterBuffer[M_MAX_JITTER_FRAMES];

}downLinkBufT;

typedef struct _voiceDataBufT
{
	UINT32 uid;
	UINT32 eid;
	UINT8 cid;
	UINT32 peerEid;
	UINT8 peerCid;
	upLinkBufT upLinkBuf;
	downLinkBufT downLinkBuf;
	FaxDataBufferT faxDataBuf;
}voiceDataBufT;



#endif /* __VOICE_DATA_H */



