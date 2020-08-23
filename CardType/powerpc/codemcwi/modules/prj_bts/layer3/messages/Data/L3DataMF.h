/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMFTAddEntry.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   05/08/07   xin wang      initialization.
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_MFTADDENTRY_H__
#define __DATA_MFTADDENTRY_H__

#include "Message.h"

/*****************************************
 *CMFTAddEntry类
 *****************************************/
class CMFTAddEntry:public CMessage
{
public:
    CMFTAddEntry(){}
    CMFTAddEntry(const CMessage &msg):CMessage( msg ){}
    virtual ~CMFTAddEntry(){}

    void SetMac(const UINT8*);
    UINT8* GetMac() const;
#if 0
    void SetType(UINT8);
    UINT8 GetType() const;
#endif
protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};


/*************************************************************
 *CMFTUpdateEntry类
 *MFTUpdateEntry具有和MFTAddEntry相同的属性
 *只是在Snoop任务发送消息时，MessageId不同，
 *所以CMFTUpdateEntry继承自CMFTAddEntry，但需重载CreateMessage
 *************************************************************/
class CMFTUpdateEntry:public CMFTAddEntry
{
public:
    CMFTUpdateEntry(){}
    CMFTUpdateEntry(const CMessage &msg):CMFTAddEntry( msg ){}
    ~CMFTUpdateEntry(){}

protected:
    UINT16 GetDefaultMsgId() const;
};

#endif /*__DATA_MFTADDENTRY_H__*/

