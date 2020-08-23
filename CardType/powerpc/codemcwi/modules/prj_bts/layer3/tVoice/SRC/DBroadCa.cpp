/*******************************************************************************
* Copyright (c) 2010 by AP Co.Ltd.All Rights Reserved   
* File Name      : DBroadCastMsg.cpp
* Create Date    : 5-Aug-2010
* programmer     :
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "DBroadCastMsg.h"
#include "tcpE.h"

int btsDcsSigTbl[MSGTYPE_DB_MAX]=
{
	DB_READY,
	DB_DataReq,
	DB_CReady,
	DB_UtDcs_UID_MSG
};

int utDcsSigTbl[SUBTYPE_DB_MAX]=
{
	MO_ReceiveReady,
	MO_ReceiveComplete,
	DB_InvalidSignal, // 2
	DB_InvalidSignal, // 3
	DB_SingleReady,
	DB_SingleDataReq,
	DB_SingleDataComplete
};

DcsSignalDictionaryItemT DcsSignalDictionary[DB_signalMax]=
{
	{ MSGTYPE_DB_MAX, SUBTYPE_DB_MAX, "DB_InvalidSignal" },
	{ MSGTYPE_DB_Ready, SUBTYPE_DB_MAX, "DB_READY" },
	{ MSGTYPE_DB_DataReq, SUBTYPE_DB_MAX, "DB_DataReq" },
	{ MSGTYPE_DB_CReady, SUBTYPE_DB_MAX, "DB_CReady" },
	{ MSGTYPE_DB_UIDMsg, SUBTYPE_MO_ReceiveReady, "MO_ReceiveReady" },
	{ MSGTYPE_DB_UIDMsg, SUBTYPE_MO_ReceiveComplete, "MO_ReceiveComplete" },
	{ MSGTYPE_DB_UIDMsg, SUBTYPE_DB_SingleReady, "DB_SingleReady" },
	{ MSGTYPE_DB_UIDMsg, SUBTYPE_DB_SingleDataReq, "DB_SingleDataReq" },
	{ MSGTYPE_DB_UIDMsg, SUBTYPE_DB_SingleDataComplete, "DB_SingleDataComplete" }
};

/*******************************************************************************
* Prototype      : 
* Parameters     :signal buffer ptr and signal data length
* Return Value   :signal type
* Global Variable: 
* Description    : parse dcs signal type
*******************************************************************************/
int parseSignal(unsigned char *pBuf, unsigned short len)
{
	int ret = DB_InvalidSignal;
	if(len<sizeof(DB_BtsDcsCtrlMsgHeadT))
	{
		//invalid
	}
	else
	{
		DB_BtsDcsCtrlMsgHeadT *pBtsDcsCtrlMsgHead = (DB_BtsDcsCtrlMsgHeadT*)pBuf;
		if(MSGTYPE_DB_UIDMsg==pBtsDcsCtrlMsgHead->DBMessagetype)
		{
			if(len<sizeof(DB_UtDcs_CtrlMsgHeadT))
			{
				//invalid
			}
			else
			{
				DB_UtDcs_CtrlMsgHeadT *pUtDcsCtrlMsgHead = (DB_UtDcs_CtrlMsgHeadT*)pBuf;
				if(pUtDcsCtrlMsgHead->Subtype<SUBTYPE_DB_MAX)
				{
					ret = utDcsSigTbl[pUtDcsCtrlMsgHead->Subtype];
				}
				else
				{
					//invalid
				}
			}
		}
		else
		{
			if(pBtsDcsCtrlMsgHead->DBMessagetype<MSGTYPE_DB_MAX)
			{
				ret = btsDcsSigTbl[pBtsDcsCtrlMsgHead->DBMessagetype];
			}
			else
			{
				//invalid
			}
		}
	}

	return ret;
}

/*******************************************************************************
* Prototype      : 
* Parameters     : signal msg, begin with tcpEHead
* Return Value   :
* Global Variable: signal type
* Description    : 
*******************************************************************************/
int parseSignal(CMessage& msg)
{
	UINT8 *pBuf = (UINT8*)msg.GetDataPtr();
	pBuf += sizeof(TcpPktHeaderT);
	UINT16 len = msg.GetDataLength() - sizeof(TcpPktHeaderT);
	return parseSignal( pBuf, len );
}
/*******************************************************************************
* Prototype      : 
* Parameters     : signal msg, begin with tcpEHead
* Return Value   :
* Global Variable: signal type
* Description    : 
*******************************************************************************/
int parseSignal(CComMessage *pMsg)
{
	UINT8 *pBuf = (UINT8*)pMsg->GetDataPtr();
	pBuf += sizeof(TcpPktHeaderT);
	UINT16 len = pMsg->GetDataLength() - sizeof(TcpPktHeaderT);
	return parseSignal( pBuf, len );
}


