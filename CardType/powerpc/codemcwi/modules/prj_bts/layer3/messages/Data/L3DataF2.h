/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataFTDelEntry.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   08/10/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_FTDELENTRY_H__
#define __DATA_FTDELENTRY_H__

#include "Message.h"

/*****************************************
 *CFTAddEntry¿‡
 *****************************************/
class CFTDelEntry:public CMessage
{
public:
    CFTDelEntry(){}
    CFTDelEntry(const CMessage &msg):CMessage( msg ){}
    ~CFTDelEntry(){}

    void   SetMac(const UINT8*);
    UINT8* GetMac() const;
    void   SetTunnel(bool);
    bool   isCreateTempTunnel() const;
    void   setTunnelPeerBtsIP(UINT32);
    UINT32 getTunnelPeerBtsIP() const;
    void   setTunnelPeerBtsPort(UINT16);
    UINT16 getTunnelPeerBtsPort() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif /*__DATA_FTDELENTRY_H__*/
