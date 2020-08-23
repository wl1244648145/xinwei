/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataFTCheckVLAN.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   06/01/06   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_FTCHECKVLAN_H__
#define __DATA_FTCHECKVLAN_H__

#include "Message.h"

/*****************************************
 *CFTCheckVLAN¿‡
 *****************************************/
class CFTCheckVLAN:public CMessage
{
public:
    CFTCheckVLAN(){}
    CFTCheckVLAN(const CMessage &msg):CMessage( msg ){}
    ~CFTCheckVLAN(){}

    void   SetVlanID(UINT16);
    UINT16 GetVlanID() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif /*__DATA_FTCHECKVLAN_H__*/

