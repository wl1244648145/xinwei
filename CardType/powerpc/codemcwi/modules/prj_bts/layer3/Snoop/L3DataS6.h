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
 *CIdleDiscoveryTrans��: 
 ****************************/
class CIdleDiscoveryTrans: public CSnoopTrans
{
public:
    CIdleDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

/****************************
 *CIdleRequestTrans��: 
 ****************************/
class CIdleRequestTrans: public CSnoopTrans
{
public:
    CIdleRequestTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};
/****************************
 *CIdleTunnelSyncReqTrans��: 
 *IDLE״̬��Tunnel Sync Request.
 ****************************/
class CIdleTunnelSyncReqTrans: public CSnoopTrans
{
public:
    CIdleTunnelSyncReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CIdleTunnelChgAnchorReqTrans��: 
 *IDLE״̬��Tunnel Change Anchor����
 ****************************/
class CIdleTunnelChgAnchorReqTrans: public CSnoopTrans
{
public:
    CIdleTunnelChgAnchorReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CIdleTunnelTerminateReqTrans��: 
 *IDLE״̬��Tunnel Terminate����
 ****************************/
class CIdleTunnelTerminateReqTrans: public CSnoopTrans
{
public:
    CIdleTunnelTerminateReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CIdleFixIpTunnelEstReqTrans��: 
 *IDLE״̬��Fixed IP�Ĵ����������
 ****************************/
class CIdleFixIpTunnelEstReqTrans: public CSnoopTrans
{
public:
    CIdleFixIpTunnelEstReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CIdleAddFixIPTrans��: 
 *IDLE״̬��ADD Fixed IP��Ϣ
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
 *CSelectingDiscoveryTrans��: 
 ****************************/
class CSelectingDiscoveryTrans: public CSnoopTrans
{
public:
    CSelectingDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSelectingOfferTrans��: 
 ****************************/
class CSelectingOfferTrans: public CSnoopTrans
{
public:
    CSelectingOfferTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSelectingRequestTrans��: 
 ****************************/
class CSelectingRequestTrans: public CSnoopTrans
{
public:
    CSelectingRequestTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingDiscoveryTrans��: 
 ****************************/
class CRequestingDiscoveryTrans: public CSnoopTrans
{
public:
    CRequestingDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingRequestTrans��: 
 ****************************/
class CRequestingRequestTrans: public CSnoopTrans
{
public:
    CRequestingRequestTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingNAKTrans��: 
 ****************************/
class CRequestingNAKTrans: public CSnoopTrans
{
public:
    CRequestingNAKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingACKTrans��: 
 ****************************/
class CRequestingACKTrans: public CSnoopTrans
{
public:
    CRequestingACKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};



/****************************
 *CSyncingDiscoveryTrans��: 
 ****************************/
class CSyncingDiscoveryTrans: public CSnoopTrans
{
public:
    CSyncingDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSyncingDHCPSyncOKTrans��: 
 ****************************/
class CSyncingDHCPSyncOKTrans: public CSnoopTrans
{
public:
    CSyncingDHCPSyncOKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBoundRelDecTrans��: 
 *Bound״̬��Release��Decline
 ****************************/
class CBoundRelDecTrans: public CSnoopTrans
{
public:
    CBoundRelDecTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

#if 0
/****************************
 *CBoundACKTrans��: 
 ****************************/
class CBoundACKTrans: public CSnoopTrans
{
public:
    CBoundACKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};
#endif

/****************************
 *CBoundRequestInitTrans��: 
 *Bound״̬�յ�Initial Request����
 ****************************/
class CBoundRequestInitTrans: public CSnoopTrans
{
public:
    CBoundRequestInitTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBoundRequestTrans��: 
 *Bound״̬�յ�Renew Request����
 ****************************/
class CBoundRequestRenewTrans: public CSnoopTrans
{
public:
    CBoundRequestRenewTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRenewingRequestTrans��: 
 ****************************/
class CRenewingRequestTrans: public CSnoopTrans
{
public:
    CRenewingRequestTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRenBndDelEntryTrans��: 
 ****************************/
class CRenBndDelEntryTrans: public CSnoopTrans
{
public:
    CRenBndDelEntryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRenewingNAKTrans��: 
 ****************************/
class CRenewingNAKTrans: public CSnoopTrans
{
public:
    CRenewingNAKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRenewingACKTrans��: 
 ****************************/
class CRenewingACKTrans: public CSnoopTrans
{
public:
    CRenewingACKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBRRDiscoveryTrans��: 
 ****************************/
class CBRRDiscoveryTrans: public CSnoopTrans
{
public:
    CBRRDiscoveryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

#endif/*__SNOOP_TRANS_DHCP_H__*/
