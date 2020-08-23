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

#ifndef _INC_L3L3L2CPEPROFUPDATENOTIFY
#include "L3L3L2CpeProfUpdateNotify.h"
#endif
#include <string.h>
CL3L2CpeProfUpdateNotify :: CL3L2CpeProfUpdateNotify()
{
}

CL3L2CpeProfUpdateNotify :: CL3L2CpeProfUpdateNotify(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3L2CpeProfUpdateNotify :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_L3_L2_CPE_PROFILE_UPDATE_NOTIFY);
    return true;
}

UINT32 CL3L2CpeProfUpdateNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CL3L2CpeProfUpdateNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CL3L2CpeProfUpdateNotify :: SetTransactionId(UINT16 T)
{
    ((T_Notify*)GetDataPtr())->TransId = T;
	return 0;
}

UINT32 CL3L2CpeProfUpdateNotify :: GetCpeId() const
{
    return ((T_Notify *)GetDataPtr())->CpeId;
}

void CL3L2CpeProfUpdateNotify :: SetCpeId(UINT32 T)
{
    ((T_Notify*)GetDataPtr())->CpeId = T;
}


bool   CL3L2CpeProfUpdateNotify::SetUTSDCfgInfo(const T_UTSDCfgInfo &data)
{
    memcpy((SINT8*)&(((T_Notify*)GetDataPtr())->UTSDCfgInfo), &data, sizeof(data));
}


void CL3L2CpeProfUpdateNotify::SetZmoduleEnabled(bool flag)
{
    ((T_Notify*)GetDataPtr())->Z_Module_enabled = (UINT8)flag;
}


CL3L2CpeProfUpdateNotify :: ~CL3L2CpeProfUpdateNotify(){}
