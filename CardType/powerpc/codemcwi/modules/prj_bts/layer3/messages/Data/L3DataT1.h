/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelEstablish.h
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

#ifndef __TUNNEL_ESTABLISH_H__
#define __TUNNEL_ESTABLISH_H__

#include "Message.h"
#include "L3DataMessages.h"
#include "L3DataTunnelRequestBase.h"
#include "L3DataTunnelResponseBase.h"

/*****************************************
 *CTunnelEstablish��
 *****************************************/
class CTunnelEstablish:public CTunnelRequestBase
{
public:
    CTunnelEstablish(){}
    CTunnelEstablish(const CMessage &msg):CTunnelRequestBase( msg ){}
    ~CTunnelEstablish(){}

    bool   CreateMessage(CComEntity&);
//    UINT16 SetMsgCode();
    UINT32 SetFixIp(UINT32);
    UINT32 GetFixIp();

    UINT16 SetGroupId(UINT16);
    UINT16 GetGroupId();

protected:
    //��Ϣ�ṹ�б䣬�����б�
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CTunnelEstablishResp��
 *****************************************/
class CTunnelEstablishResp:public CTunnelResponseBase
{
public:
    CTunnelEstablishResp(){}
    CTunnelEstablishResp(const CMessage &msg):CTunnelResponseBase( msg ){}
    ~CTunnelEstablishResp(){}

    bool   CreateMessage(CComEntity&);
//    UINT16 SetMsgCode();

protected:
    UINT16 GetDefaultMsgId() const;
};

#endif /*__TUNNEL_ESTABLISH_H__*/
