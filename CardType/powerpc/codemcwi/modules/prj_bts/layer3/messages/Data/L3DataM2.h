/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMFTEntryExpire.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   04/27/07   xin wang      initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_MFTENTRYEXPIRE_H__
#define __DATA_MFTENTRYEXPIRE_H__

#include "Message.h"

/*****************************************
 *CMFTEntryExpire类
 *MACFiltering Table 超时处理
 *****************************************/
class CMFTEntryExpire:public CMessage
{
public:
    CMFTEntryExpire(){}
    CMFTEntryExpire(const CMessage &msg):CMessage( msg ){}
    ~CMFTEntryExpire(){}

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

#endif /*__DATA_MFTENTRYEXPIRE_H__*/
