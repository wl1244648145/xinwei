/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTunnelTerminate.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/06/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __TUNNEL_TERMINATE_H__
#define __TUNNEL_TERMINATE_H__

#include "Message.h"
#include "L3DataMessages.h"
#include "L3DataTunnelRequestBase.h"
#include "L3DataTunnelResponseBase.h"

/*****************************************
 *CTunnelTerminate¿‡
 *****************************************/
class CTunnelTerminate:public CTunnelRequestBase
{
public:
    CTunnelTerminate(){}
    CTunnelTerminate(const CMessage &msg):CTunnelRequestBase( msg ){}
    ~CTunnelTerminate(){}

    bool   CreateMessage(CComEntity&);
//    UINT16 SetMsgCode();

protected:
    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CTunnelTerminateResp¿‡
 *****************************************/
class CTunnelTerminateResp:public CTunnelResponseBase
{
public:
    CTunnelTerminateResp(){}
    CTunnelTerminateResp(const CMessage &msg):CTunnelResponseBase( msg ){}
    ~CTunnelTerminateResp(){}

    bool   CreateMessage(CComEntity&);
//    UINT16 SetMsgCode();

protected:
    UINT16 GetDefaultMsgId() const;
};

/*****************************************
 *CTUNNELDelete¿‡
 *****************************************/
class CTUNNELDelete:public CTunnelRequestBase
{
public:
    CTUNNELDelete(){}
    CTUNNELDelete(const CMessage &msg):CTunnelRequestBase( msg ){}
    ~CTUNNELDelete(){}

    bool   CreateMessage(CComEntity&);


protected:
    UINT16 GetDefaultMsgId() const;
};
#endif /*__TUNNEL_TERMINATE_H__*/
