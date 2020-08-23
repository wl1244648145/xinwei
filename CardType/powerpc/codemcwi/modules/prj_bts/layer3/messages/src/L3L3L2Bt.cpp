/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3L3L2BtsConfigSYNPower.cpp
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   09/18/2006   Ð¤ÎÀ·½       Initial file creation.
 *---------------------------------------------------------------------------*/

#include "L3L2MessageId.h"
#include "L3L3L2BtsConfigSYNPower.h"


bool CL3L2BtsConfigSYNPower :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        {
        return false;
        }
    SetMessageId(M_L3_L2_CFG_SYN_POWER_REQ);
    return true;
}

UINT32 CL3L2BtsConfigSYNPower :: GetDefaultDataLen() const
{
    return sizeof(stConfigSYNPower);
}

UINT16 CL3L2BtsConfigSYNPower :: SetTransactionId(UINT16 usTransId)
{
    return ((stConfigSYNPower *)GetDataPtr())->usTransId = usTransId;
}

UINT16 CL3L2BtsConfigSYNPower :: SetOp(UINT16 usOp)
{
    return ((stConfigSYNPower *)GetDataPtr())->usConfigBit = usOp;
}
