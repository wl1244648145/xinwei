/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: Fsm..cpp
 *
 * DESCRIPTION:
 *    This file implements the FSM classes.
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   10/31/2005   Liu Qun    Corrected data types of return code
 *   10/24/2005   Liu Qun    Added Injecting msg processing error protection
 *   09/27/2005   Liu Qun    Added Parent-state support
 *   08/03/2005   Yushu Shi  Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_FSM
#include "fsm.h"
#endif

FSMTransIndex FSMState::ParseEvent(CMessage& msg)
{
    FSMTransIndex trans = DoParseEvent(msg);
    if ( trans== INVALID_EVENT && m_pParent!=NULL)
        trans = m_pParent->ParseEvent(msg);
    return trans;
}

FSM::FSM(UINT32 nStateNum, UINT32 nTransNum)
{
    UINT32 i;
    m_pStateTable = new FSMState*[nStateNum];
    for (i=0; i < nStateNum; i++)
    {
        m_pStateTable[i] = NULL;
    }

    m_pTransTable = new FSMTransition*[nTransNum];
    for (i=0; i<nTransNum; i++)
    {
        m_pTransTable[i] = NULL;
    }
}

bool FSM::InjectMsg(CMessage &msg)
{
    CCBBase *CCB = FindCCB(msg);
    if (CCB != NULL)
    {
        FSMStateIndex originState = CCB->GetCurrentState();

		if (originState>=GetStateNumber())
            {
            Reclaim( CCB );
            return false;
            }      

        FSMTransIndex trans = m_pStateTable[originState]->ParseEvent(msg);
        if (trans == INVALID_EVENT || trans >= GetTransNumber())
            {
            Reclaim( CCB );
            return false;
            }      

        FSMStateIndex targetState = m_pTransTable[trans]->Action(*CCB, msg);
        if (targetState >= GetStateNumber())
            {
            Reclaim( CCB );
            return false;
            }      

        if (targetState != originState)
        {
            m_pStateTable[originState]->Exit(*CCB);
            m_pStateTable[targetState]->Entry(*CCB);
            CCB->SetCurrentState(targetState);
        }
        Reclaim( CCB );
        return true;
    }
    return false;
}
