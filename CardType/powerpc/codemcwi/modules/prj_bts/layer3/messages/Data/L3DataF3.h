/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataFTEntryExpire.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   08/09/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_FTENTRYEXPIRE_H__
#define __DATA_FTENTRYEXPIRE_H__

#include "Message.h"

/*****************************************
 *CFTEntryExpire类
 *EB处理完OAM的配置消息后往OAM回应的消息
 *****************************************/
class CFTEntryExpire:public CMessage
{
public:
    CFTEntryExpire(){}
    CFTEntryExpire(const CMessage &msg):CMessage( msg ){}
    ~CFTEntryExpire(){}

    void SetMac(const UINT8*);
    UINT8* GetMac() const;
	void SetFlag(const UINT8 flag);
     


   UINT8 GetFlag() const;
protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif /*__DATA_FTENTRYEXPIRE_H__*/
