/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopTransDHCP.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/12/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_TRANS_DHCP_H__
#define __SNOOP_TRANS_DHCP_H__

//Sockets.
#ifdef __WIN32_SIM__
#include <winsock2.h>
#else	//VxWorks:
#include "vxWorks.h" 
#include "sockLib.h" 
#include "inetLib.h" 
#include "stdioLib.h" 
#include "strLib.h" 
#include "hostLib.h" 
#include "ioLib.h" 
#endif

#include "L3DataSnoopTrans.h"

/****************************
 *CIdleDiscoveryTrans类: 
 ****************************/
class CIdleDiscoveryTrans: public CSnoopTrans
{
public:
    CIdleDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

/****************************
 *CIdleRequestTrans类: 
 ****************************/
class CIdleRequestTrans: public CSnoopTrans
{
public:
    CIdleRequestTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};
/****************************
 *CIdleTunnelSyncReqTrans类: 
 *IDLE状态收Tunnel Sync Request.
 ****************************/
class CIdleTunnelSyncReqTrans: public CSnoopTrans
{
public:
    CIdleTunnelSyncReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CIdleTunnelChgAnchorReqTrans类: 
 *IDLE状态收Tunnel Change Anchor请求
 ****************************/
class CIdleTunnelChgAnchorReqTrans: public CSnoopTrans
{
public:
    CIdleTunnelChgAnchorReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CIdleTunnelTerminateReqTrans类: 
 *IDLE状态收Tunnel Terminate请求
 ****************************/
class CIdleTunnelTerminateReqTrans: public CSnoopTrans
{
public:
    CIdleTunnelTerminateReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CIdleFixIpTunnelEstReqTrans类: 
 *IDLE状态收Fixed IP的创建隧道请求
 ****************************/
class CIdleFixIpTunnelEstReqTrans: public CSnoopTrans
{
public:
    CIdleFixIpTunnelEstReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CIdleAddFixIPTrans类: 
 *IDLE状态收ADD Fixed IP消息
 ****************************/
class CIdleAddFixIPTrans: public CSnoopTrans
{
public:
    CIdleAddFixIPTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
private:
    bool OutRaid(UINT32, const UTILEntry &, CSnoopCCB &);
    bool AddFixIpEntry(CSnoopCCB &, UINT32, bool, UINT32, UINT32, const UTILEntry &);

};


/****************************
 *CSelectingDiscoveryTrans类: 
 ****************************/
class CSelectingDiscoveryTrans: public CSnoopTrans
{
public:
    CSelectingDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSelectingOfferTrans类: 
 ****************************/
class CSelectingOfferTrans: public CSnoopTrans
{
public:
    CSelectingOfferTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSelectingRequestTrans类: 
 ****************************/
class CSelectingRequestTrans: public CSnoopTrans
{
public:
    CSelectingRequestTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingDiscoveryTrans类: 
 ****************************/
class CRequestingDiscoveryTrans: public CSnoopTrans
{
public:
    CRequestingDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingRequestTrans类: 
 ****************************/
class CRequestingRequestTrans: public CSnoopTrans
{
public:
    CRequestingRequestTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingNAKTrans类: 
 ****************************/
class CRequestingNAKTrans: public CSnoopTrans
{
public:
    CRequestingNAKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingACKTrans类: 
 ****************************/
class CRequestingACKTrans: public CSnoopTrans
{
public:
    CRequestingACKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};



/****************************
 *CSyncingDiscoveryTrans类: 
 ****************************/
class CSyncingDiscoveryTrans: public CSnoopTrans
{
public:
    CSyncingDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSyncingDHCPSyncOKTrans类: 
 ****************************/
class CSyncingDHCPSyncOKTrans: public CSnoopTrans
{
public:
    CSyncingDHCPSyncOKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBoundRelDecTrans类: 
 *Bound状态收Release和Decline
 ****************************/
class CBoundRelDecTrans: public CSnoopTrans
{
public:
    CBoundRelDecTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

#if 0
/****************************
 *CBoundACKTrans类: 
 ****************************/
class CBoundACKTrans: public CSnoopTrans
{
public:
    CBoundACKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};
#endif

/****************************
 *CBoundRequestInitTrans类: 
 *Bound状态收到Initial Request报文
 ****************************/
class CBoundRequestInitTrans: public CSnoopTrans
{
public:
    CBoundRequestInitTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBoundRequestTrans类: 
 *Bound状态收到Renew Request报文
 ****************************/
class CBoundRequestRenewTrans: public CSnoopTrans
{
public:
    CBoundRequestRenewTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRenewingRequestTrans类: 
 ****************************/
class CRenewingRequestTrans: public CSnoopTrans
{
public:
    CRenewingRequestTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRenBndDelEntryTrans类: 
 ****************************/
class CRenBndDelEntryTrans: public CSnoopTrans
{
public:
    CRenBndDelEntryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRenewingNAKTrans类: 
 ****************************/
class CRenewingNAKTrans: public CSnoopTrans
{
public:
    CRenewingNAKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRenewingACKTrans类: 
 ****************************/
class CRenewingACKTrans: public CSnoopTrans
{
public:
    CRenewingACKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBRRDiscoveryTrans类: 
 ****************************/
class CBRRDiscoveryTrans: public CSnoopTrans
{
public:
    CBRRDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

#endif/*__SNOOP_TRANS_DHCP_H__*/
