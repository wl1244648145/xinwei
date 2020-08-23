/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopState.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   09/08/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_STATE_H__
#define __SNOOP_STATE_H__

#include "L3DataSnoopFSM.h"


/*****************************
 *M_MSGTYPE_ERR: ������
 *****************************/
#define M_MSGTYPE_ERR   (0xFF)

class CParentState;

/****************************
 *CSnoopStateBase��: 
 *Snoop״̬������
 ****************************/
class CSnoopStateBase: public FSMState
{
public:
    CSnoopStateBase(){}
    CSnoopStateBase(FSMState& rParent):FSMState( rParent ){}
    bool ParseDhcpPacket(const CMessage &msg, DhcpOption &);
    UINT8 ParsePPPoEPacket(const CMessage &msg);
    /*
     *ParseDhcpOpt��FSMState���FSMTransition�඼��ʹ��
     *�������ɾ�̬����
     */
    static bool ParseDhcpOpt(UINT8*, const SINT16, DhcpOption*);
};


/****************************
 *CIdleState��: 
 ****************************/
class CIdleState: public CSnoopStateBase
{
public:
    CIdleState(){}
    CIdleState(FSMState& rParent):CSnoopStateBase( rParent ){}
    virtual FSMTransIndex  DoParseEvent(CMessage& msg);
    virtual const UINT8 *const GetStateName(void) const
        {
        return (const UINT8 *)"SNOOP-IDLE-STATE";
        };
};


/****************************
 *CSelectingState��: 
 ****************************/
class CSelectingState:public CSnoopStateBase
{
public:
    CSelectingState() {};
    CSelectingState(FSMState& rParent):CSnoopStateBase( rParent ){}
    virtual FSMTransIndex  DoParseEvent(CMessage& msg);
    virtual const UINT8 *const GetStateName(void) const
        {
        return (const UINT8 *)("SNOOP-SELECTING-STATE");
        };
};


/****************************
 *CRequestingState��: 
 ****************************/
class CRequestingState:public CSnoopStateBase
{
public:
    CRequestingState() {};
    CRequestingState(FSMState& rParent):CSnoopStateBase( rParent ){}
    virtual FSMTransIndex  DoParseEvent(CMessage& msg);
    virtual const UINT8 *const GetStateName(void) const
        {
        return (const UINT8 *)("SNOOP-REQUESTING-STATE");
        };
};


/****************************
 *CSyncingState��: 
 ****************************/
class CSyncingState:public CSnoopStateBase
{
public:
    CSyncingState() {};
    CSyncingState(FSMState& rParent):CSnoopStateBase( rParent ){}
    virtual FSMTransIndex  DoParseEvent(CMessage& msg);
    virtual const UINT8 *const GetStateName(void) const
        {
        return (const UINT8 *)("SNOOP-SYNCING-STATE");
        };
};


/****************************
 *CBoundState��: 
 ****************************/
class CBoundState:public CSnoopStateBase
{
public:
    CBoundState() {};
    CBoundState(FSMState& rParent):CSnoopStateBase( rParent ){}
    virtual FSMTransIndex  DoParseEvent(CMessage& msg);
    virtual const UINT8 *const GetStateName(void) const
        {
        return (const UINT8 *)("SNOOP-BOUND-STATE");
        };
};


/****************************
 *CRenewingState��: 
 ****************************/
class CRenewingState:public CSnoopStateBase
{
public:
    CRenewingState() {};
    CRenewingState(FSMState& rParent):CSnoopStateBase( rParent ){}
    virtual FSMTransIndex  DoParseEvent(CMessage& msg);
    virtual const UINT8 *const GetStateName(void) const
        {
        return (const UINT8 *)("SNOOP-RENEWING-STATE");
        };
};


/****************************
 *CRoamingState��: 
 ****************************/
class CRoamingState:public CSnoopStateBase
{
public:
    CRoamingState() {};
    CRoamingState(FSMState& rParent):CSnoopStateBase( rParent ){}
    virtual FSMTransIndex  DoParseEvent(CMessage& msg);
    virtual const UINT8 *const GetStateName(void) const
        {
        return (const UINT8 *)("SNOOP-ROAMING-STATE");
        };
};


/****************************
 *CParentState��: 
 *����״̬����Ҫ������¼����
 *ΪParent״̬���¼�
 ****************************/
class CParentState:public CSnoopStateBase
{
public:
    CParentState() {};
    CParentState(FSMState& rParent):CSnoopStateBase( rParent ){}
    virtual FSMTransIndex  DoParseEvent(CMessage& msg);
    virtual const UINT8 *const GetStateName(void) const
        {
        return (const UINT8 *)("SNOOP-ANY-STATE");
        };
};

#endif/*__SNOOP_STATE_H__*/
