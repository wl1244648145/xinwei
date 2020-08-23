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
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3L3L2AUXSTATENOTIFY
#include "L3L3L2AuxStateNotify.h"
#endif


CL3L2AuxStateNoitfy :: CL3L2AuxStateNoitfy()
{
}

CL3L2AuxStateNoitfy :: CL3L2AuxStateNoitfy(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3L2AuxStateNoitfy :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L2_L3_AUXSTATE_NOTIFY);
    return true;
}

UINT32 CL3L2AuxStateNoitfy :: GetDefaultDataLen() const
{
    return (sizeof(T_Notify));
}

UINT16 CL3L2AuxStateNoitfy :: GetTransactionId()const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CL3L2AuxStateNoitfy :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

const T_AUXStateInfo* CL3L2AuxStateNoitfy ::  GetAUXStateInfo()const
{
    return (T_AUXStateInfo*)&(((T_Notify*)GetDataPtr())->AUXStateInfo);
}

CL3L2AuxStateNoitfy :: ~CL3L2AuxStateNoitfy()
{

}
