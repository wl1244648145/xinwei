/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelRequestBase.h
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

#ifndef __TUNNEL_REQUEST_BASE_H__
#define __TUNNEL_REQUEST_BASE_H__

#include "Message.h"
#include "L3DataMessages.h"


/*****************************************
 *CTunnelRequestBase类
 *目前所有Tunnel相关的Request消息结构都
 *类似，所以做成基类
 *****************************************/
class CTunnelRequestBase:public CMessage
{
public:
    CTunnelRequestBase(){}
    CTunnelRequestBase(const CMessage &msg):CMessage( msg ){}
    virtual ~CTunnelRequestBase(){}

    UINT16 GetMsgCode() const;

    UINT32 SetEidInPayload(UINT32);
    UINT32 GetEidInPayload() const;

    void SetMac(const UINT8*);
    UINT8* GetMac() const;

    UINT32 SetDstBtsID(UINT32);
    UINT32 GetDstBtsID() const;
	/*
    UINT32 SetDstBtsIP(UINT32);
    UINT32 GetDstBtsIP() const;
    UINT16 SetDstPort(UINT16);
    UINT16 GetDstPort() const;
*/
    UINT32 SetSenderBtsID(UINT32);
    UINT32 GetSenderBtsID() const;
    UINT32 SetSenderBtsIP(UINT32);
    UINT32 GetSenderBtsIP() const;
    UINT16 SetSenderPort(UINT16);
    UINT16 GetSenderPort() const;


    UINT8 SetIpType(UINT8);
    UINT8 GetIpType() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 SetMsgCode(UINT16);
};

#endif /*__TUNNEL_REQUEST_BASE_H__*/
