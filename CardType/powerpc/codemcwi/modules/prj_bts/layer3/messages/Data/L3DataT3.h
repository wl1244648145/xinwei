/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelResponseBase.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/05/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __TUNNEL_RESPONSE_BASE_H__
#define __TUNNEL_RESPONSE_BASE_H__

#include "Message.h"
#include "L3DataMessages.h"


/*****************************************
 *CTunnelResponseBase类
 *目前所有Tunnel相关的response消息结构都
 *类似，所以做成基类
 *****************************************/
class CTunnelResponseBase:public CMessage
{
public:
    CTunnelResponseBase(){}
    CTunnelResponseBase(const CMessage &msg):CMessage( msg ){}
    virtual ~CTunnelResponseBase(){}

    UINT16 GetMsgCode() const;

    UINT32 SetEidInPayload(UINT32);
    UINT32 GetEidInPayload() const;

    void SetMac(const UINT8*);
    UINT8* GetMac() const;

    UINT32 SetDstBtsID(UINT32);
    UINT32 GetDstBtsID() const;
    UINT32 SetDstBtsIP(UINT32);
    UINT32 GetDstBtsIP() const;
    UINT16 SetDstPort(UINT32);
    UINT16 GetDstPort() const;

    bool SetResult(bool);
    bool GetResult() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 SetMsgCode(UINT16);
};

#endif /*__TUNNEL_RESPONSE_BASE_H__*/
