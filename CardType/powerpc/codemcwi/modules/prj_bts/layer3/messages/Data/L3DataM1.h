/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMFTDelEntry.h
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

#ifndef __DATA_MFTDELENTRY_H__
#define __DATA_MFTDELENTRY_H__

#include "Message.h"

/*****************************************
 *CMFTAddEntry¿‡
 *****************************************/
class CMFTDelEntry:public CMessage
{
public:
    CMFTDelEntry(){}
    CMFTDelEntry(const CMessage &msg):CMessage( msg ){}
    ~CMFTDelEntry(){}

    void   SetMac(const UINT8*);
    UINT8* GetMac() const;
#if 0	
    void SetType(UINT8);
    UINT8 GetType() const;
#endif
protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif /*__DATA_MFTDELENTRY_H__*/
