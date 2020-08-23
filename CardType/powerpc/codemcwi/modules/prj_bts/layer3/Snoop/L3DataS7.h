/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopTransPPPoE.h
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

#ifndef __SNOOP_TRANS_PPPoE_H__
#define __SNOOP_TRANS_PPPoE_H__

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
 *CIdlePADITrans类: 
 ****************************/
class CIdlePADITrans: public CSnoopTrans
{
public:
    CIdlePADITrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};



/****************************
 *CSelectingPADITrans类: 
 ****************************/
class CSelectingPADITrans: public CSnoopTrans
{
public:
    CSelectingPADITrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};



/****************************
 *CSelectingPADOTrans类: 
 ****************************/
class CSelectingPADOTrans: public CSnoopTrans
{
public:
    CSelectingPADOTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};



/****************************
 *CSelReqTimeOutTrans类: 
 ****************************/
class CSelReqTimeOutTrans: public CSnoopTrans
{
public:
    CSelReqTimeOutTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSelReqDelEntryTrans类: 
 ****************************/
class CSelReqDelEntryTrans: public CSnoopTrans
{
public:
    CSelReqDelEntryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSelectingPADRTrans类: 
 ****************************/
class CSelectingPADRTrans: public CSnoopTrans
{
public:
    CSelectingPADRTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingPADITrans类: 
 ****************************/
class CRequestingPADITrans: public CSnoopTrans
{
public:
    CRequestingPADITrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingPADRTrans类: 
 ****************************/
class CRequestingPADRTrans: public CSnoopTrans
{
public:
    CRequestingPADRTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CRequestingPADSTrans类: 
 ****************************/
class CRequestingPADSTrans: public CSnoopTrans
{
public:
    CRequestingPADSTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSyncingPADITrans类: 
 ****************************/
class CSyncingPADITrans: public CSnoopTrans
{
public:
    CSyncingPADITrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSyncingPPPoESyncOKTrans类: 
 ****************************/
class CSyncingPPPoESyncOKTrans: public CSnoopTrans
{
public:
    CSyncingPPPoESyncOKTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSyncingSyncFAILTrans类: 
 ****************************/
class CSyncingSyncFAILTrans: public CSnoopTrans
{
public:
    CSyncingSyncFAILTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CSyncingDelEntryTrans类: 
 ****************************/
class CSyncingDelEntryTrans: public CSnoopTrans
{
public:
    CSyncingDelEntryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBoundPADRTrans类: 
 ****************************/
class CBoundPADRTrans: public CSnoopTrans
{
public:
    CBoundPADRTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBoundPADSTrans类: 
 ****************************/
class CBoundPADSTrans: public CSnoopTrans
{
public:
    CBoundPADSTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBRPADITrans类: 
 ****************************/
class CBRPADITrans: public CSnoopTrans
{
public:
    CBRPADITrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBoundPADTTrans类: 
 ****************************/
class CBoundPADTTrans: public CSnoopTrans
{
public:
    CBoundPADTTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/****************************
 *CBoundTimeOutTrans类: 
 ****************************/
class CBoundTimeOutTrans: public CSnoopTrans
{
public:
    CBoundTimeOutTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

/*********************************
 *CTunnelEstablishReqTrans类: 
 ********************************/
class CTunnelEstablishReqTrans: public CSnoopTrans
{
public:
    CTunnelEstablishReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/*********************************
 *CTunnelTerminateReqTrans类: 
 ********************************/
class CTunnelTerminateReqTrans: public CSnoopTrans
{
public:
    CTunnelTerminateReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/************************************
 *CTunnelChangeAnchorReqTrans类: 
 ***********************************/
class CTunnelChangeAnchorReqTrans: public CSnoopTrans
{
public:
    CTunnelChangeAnchorReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/*************************************
 *CRoamingTunnelEstablishRespTrans类: 
 *************************************/
class CRoamingTunnelEstablishRespTrans: public CSnoopTrans
{
public:
    CRoamingTunnelEstablishRespTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/*************************************
 *CRoamingTimeOutTrans类: 
 *************************************/
class CRoamingTimeOutTrans: public CSnoopTrans
{
public:
    CRoamingTimeOutTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/*************************************
 *CRoamingDelEntryTrans类: 
 *************************************/
class CRoamingDelEntryTrans: public CSnoopTrans
{
public:
    CRoamingDelEntryTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

/****************************
 *CPPPoEEntryExpireTrans类: 
 ****************************/
class CEntryExpireTrans: public CSnoopTrans
{
public:
    CEntryExpireTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


#endif/*__SNOOP_TRANS_PPPoE_H__*/
