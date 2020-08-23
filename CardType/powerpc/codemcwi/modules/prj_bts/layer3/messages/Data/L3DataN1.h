/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataNotifyRefreshJammingNeighbor.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   05/14/07   xinwang		  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __L3_DATAREFRESH_JAMMINGNEGHBOR_H__
#define __L3_DATAREFRESH_JAMMINGNEGHBOR_H__

#include "Message.h"
#include "L3DataMessages.h"

/*****************************************
 *CDataRefreshJammingNeighbor类
 *tConfig在改写NVRAM中的Neighbor信息时
 *发给tSocket的通知消息
 *****************************************/
class CDataRefreshJammingNeighbor:public CMessage
{
public:
	CDataRefreshJammingNeighbor(){}
    CDataRefreshJammingNeighbor(const CMessage &msg):CMessage( msg ){}
    ~CDataRefreshJammingNeighbor(){}
	bool CreateMessage(CComEntity&);
	UINT32 SetFlag(UINT32);
	UINT32 GetFlag() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
	
};

#endif  /*__L3_DATAREFRESH_JAMMINGNEGHBOR_H__*/

