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
 *M_MSGTYPE_ERR: 包错误
 *****************************/
#define M_MSGTYPE_ERR   (0xFF)

class CParentState;

/****************************
 *CSnoopStateBase类: 
 *Snoop状态机基类
 ****************************/
class CSnoopStateBase: public FSMState
{
public:
    CSnoopStateBase(){}
    CSnoopStateBase(FSMState& rParent):FSMState( rParent ){}
    bool ParseDhcpPacket(const CMessage &msg, DhcpOption &);
    UINT8 ParsePPPoEPacket(const CMessage &msg);
    /*
     *ParseDhcpOpt在FSMState类和FSMTransition类都会使用
     *所以做成静态函数
     */
    static bool ParseDhcpOpt(UINT8*, const SINT16, DhcpOption*);
};


/****************************
 *CIdleState类: 
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
 *CSelectingState类: 
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
 *CRequestingState类: 
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
 *CSyncingState类: 
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
 *CBoundState类: 
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
 *CRenewingState类: 
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
 *CRoamingState类: 
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
 *CParentState类: 
 *所有状态都需要处理的事件归结
 *为Parent状态的事件
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
