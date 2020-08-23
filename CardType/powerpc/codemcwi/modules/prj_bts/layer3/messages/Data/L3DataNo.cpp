/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataNotifyBTSPubIP.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   01/04/07   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include "L3DataNotifyBTSPubIP.h"
#include "L3DataMsgId.h"
#include "L3OAMMessageID.h"
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif

UINT16 CDataNotifyBtsIP::GetDefaultMsgId() const
{
    return MSGID_OAM_NOTIFYBTSPUBIP;
}

UINT32 CDataNotifyBtsIP::GetDefaultDataLen() const
{
    return sizeof( stNotifyBTSPubIP );
}

UINT32 CDataNotifyBtsIP::SetBTSPubIP(UINT32 ulIp)
{
    return ( (stNotifyBTSPubIP*)GetDataPtr() )->ulBtsIp = htonl(ulIp);

}
UINT32 CDataNotifyBtsIP::GetBTSPubIP() const
{
    return ntohl(( (stNotifyBTSPubIP*)GetDataPtr() )->ulBtsIp );

}

UINT16 CDataNotifyBtsIP::SetBTSPubPort(UINT16 usPort)
{
    return ( (stNotifyBTSPubIP*)GetDataPtr() )->usBtsPort = htons(usPort);

}
UINT16 CDataNotifyBtsIP::GetBTSPubPort() const
{
    return ntohs(( (stNotifyBTSPubIP*)GetDataPtr() )->usBtsPort );

}


//CTunnelNotifyBTSIP类定义
//----------------------------------------------------

bool CTunnelNotifyBTSIP::CreateMessage(CComEntity& Entity)
{
    if ( false == CMessage::CreateMessage( Entity ) )
        return false;
    SetMsgCode( MSGID_FT_MODIFY_BTSPUBIP );
    return true;
}


UINT16 CTunnelNotifyBTSIP::GetDefaultMsgId() const
{
    return MSGID_FT_MODIFY_BTSPUBIP;
}



//CUpdateBTSPubIp类定义
//----------------------------------------------------

UINT16 CUpdateBTSPubIp::GetDefaultMsgId() const
{
    return MSGID_IPLIST_PUBIP_MODIFY;
}


UINT32 CUpdateBTSPubIp::SetBTSID(UINT32 ulId)
{
    return ( (stNotifyBTSPubIP*)GetDataPtr() )->ulBtsID = htonl(ulId);

}
UINT32 CUpdateBTSPubIp::GetBTSID() const
{
    return ntohl(( (stNotifyBTSPubIP*)GetDataPtr() )->ulBtsID );

}


