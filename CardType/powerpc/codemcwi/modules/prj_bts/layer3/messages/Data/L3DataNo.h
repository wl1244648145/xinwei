/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataNotifyBTSPubIP.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   01/04/07   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __L3_DATANOTIFY_H__
#define __L3_DATANOTIFY_H__

#include "Message.h"
#include "L3DataMessages.h"
#include "L3DataTunnelRequestBase.h"
#include "L3DataTunnelResponseBase.h"

/*****************************************
 *CDataNotifyBtsIP类
 *OAM发给EtherBridge的通知消息
 *****************************************/
class CDataNotifyBtsIP:public CMessage
{
public:
	CDataNotifyBtsIP(){}
    CDataNotifyBtsIP(const CMessage &msg):CMessage( msg ){}
    virtual ~CDataNotifyBtsIP(){}

    UINT32 SetBTSPubIP(UINT32);
    UINT32 GetBTSPubIP() const;

    UINT16 SetBTSPubPort(UINT16);
    UINT16 GetBTSPubPort() const;


protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

/*****************************************
 *CTunnelEstablish类
 *****************************************/
class CTunnelNotifyBTSIP:public CTunnelRequestBase
{
public:
    CTunnelNotifyBTSIP(){}
    CTunnelNotifyBTSIP(const CMessage &msg):CTunnelRequestBase( msg ){}
    ~CTunnelNotifyBTSIP(){}
    bool   CreateMessage(CComEntity&);

protected:
    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CUpdateBTSPubIp类
 *Tunnel发给tSocket的通知消息
 *****************************************/
class CUpdateBTSPubIp:public CDataNotifyBtsIP
{
public:
	CUpdateBTSPubIp(){}
    CUpdateBTSPubIp(const CMessage &msg):CDataNotifyBtsIP( msg ){}
    ~CUpdateBTSPubIp(){}

    UINT32 SetBTSID(UINT32);
    UINT32 GetBTSID() const;

protected:
    UINT16 GetDefaultMsgId() const;
};

#endif  /*__L3_DATACONFIG_H__*/

