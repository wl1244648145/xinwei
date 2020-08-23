/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef  _INC_L3OAMCPESTATE
#define  _INC_L3OAMCPESTATE

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#include "fsm.h"

class CPENorecordState: public FSMState
{
public:
    CPENorecordState () {};
private:    
    FSMTransIndex     DoParseEvent(CMessage& msg);  
    const UINT8 *const GetStateName (void) const
    {
        return (const UINT8 *)"CPE-IDLE"; 
    }
};


class CPEServingState: public FSMState
{
public:
    CPEServingState () {};
private:    
    FSMTransIndex     DoParseEvent(CMessage& msg);
    const UINT8 *const GetStateName (void) const
    {
        return (const UINT8 *)"CPE-SERVING"; 
    }
};


class CPEWaitforemsState: public FSMState
{
public:
    CPEWaitforemsState () {};
private:    
    FSMTransIndex     DoParseEvent(CMessage& msg);  
    const UINT8 *const GetStateName (void) const
    {
        return (const UINT8 *)"CPE-WAITFOREMS"; 
    }
};



class CPESwitchoffState: public FSMState
{
public:
    CPESwitchoffState () {};
private:    
    FSMTransIndex     DoParseEvent(CMessage& msg);
    const UINT8 *const GetStateName (void) const
    { 
        return (const UINT8 *)"CPE-SWITCHOFF"; 
    }
};


class CPEMovedawayState: public FSMState
{
public:
    CPEMovedawayState () {};
private:    
    FSMTransIndex     DoParseEvent(CMessage& msg);
    const UINT8 *const GetStateName (void) const
    {
        return (const UINT8 *)"CPE-MOVEDAWAY"; 
    }
};
#endif
