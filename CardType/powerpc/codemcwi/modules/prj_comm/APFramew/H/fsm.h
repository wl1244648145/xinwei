/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: Fsm.h
 *
 * DESCRIPTION:
 *
 *    Base class definitions used to implement a generic FSM, which includes FSMState, 
 *     FSMTransition, CCBBase, CCBTableBase and FSM.
 *    
 *    Normally, an FSM is comprised of a state table, a transition table, a CCB table.
 *    
 *    When a input message comes in, the corresponding CCB should be found first to get the
 *    current state of the CCB. The state should translate the incoming message to the
 *    transition to process the message. The transition will provide the target
 *    state upon finish.
 *
 *    Since the structure of CCBs can be quite different, the derived FSM classes should
 *    define the CCBTable of the right class.
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   09/09/2005   Liu Qun    Added parent-state support
 *   08/03/2005   Yushu Shi  Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_FSM
#define _INC_FSM

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

#ifndef _INC_OBJECT
#include "Object.h"
#endif

class CMessage;

//------------------------------------------------------------------
//------------------------------------------------------------------
typedef SINT32 FSMStateIndex;
typedef SINT32 FSMTransIndex;

const FSMTransIndex  INVALID_EVENT = -1;

//------------------------------------------------------------------
//------------------------------------------------------------------
class CCBBase
#ifndef NDEBUG
: public CObject
#endif
{
public:
    CCBBase() {}
    CCBBase(FSMStateIndex state):currentState(state) {}
    FSMStateIndex GetCurrentState() { return currentState; }
    void SetCurrentState(FSMStateIndex state) { currentState = state;}
private:
    FSMStateIndex currentState;
};

class CCBTableBase
#ifndef NDEBUG
: public CObject
#endif
{
public:
    CCBTableBase(){};
    virtual CCBBase *FindCCB(CMessage &)=0;
};

//------------------------------------------------------------------
//------------------------------------------------------------------
class FSMState
#ifndef NDEBUG
: public CObject
#endif
{
public:
    FSMState():m_pParent(NULL) {}
    FSMState(FSMState& rParent):m_pParent(&rParent) {}
    FSMTransIndex ParseEvent(CMessage& msg);

    virtual void              Entry(CCBBase &){}
    virtual void              Exit(CCBBase &){}
    
    virtual FSMTransIndex     DoParseEvent(CMessage&)=0;

    virtual const UINT8 *const GetStateName (void) const = 0;
    virtual ~FSMState(void){}

private:
    FSMState* m_pParent;
};
            


//------------------------------------------------------------------
//------------------------------------------------------------------
class FSMTransition
#ifndef NDEBUG
: public CObject
#endif
{
public:
    FSMTransition(FSMStateIndex theTargetState) :TargetState(theTargetState){}
    virtual FSMStateIndex Action(CCBBase &, CMessage &) =0;

protected:
    FSMStateIndex TargetState;
};


//------------------------------------------------------------------
//------------------------------------------------------------------
class FSM
#ifndef NDEBUG
: public CObject
#endif
{
public:
    FSM(UINT32 nStateNum, UINT32 nTransNum);

    virtual bool InjectMsg(CMessage &);
	virtual UINT8 GetStateNumber()=0;
	virtual UINT8 GetTransNumber()=0;
    virtual CCBBase *FindCCB(CMessage &)=0;
    virtual void Reclaim(CCBBase*){}

protected:
    FSMState **m_pStateTable;
    FSMTransition **m_pTransTable;
};

#endif // _INC_FSM
