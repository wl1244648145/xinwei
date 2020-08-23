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


#ifndef _INC_L3OAMALMHANDLENOTIFY
#include "L3OamAlarmHandleNotify.h"
#endif

#include <string.h>

CL3OamAlarmHandleNotify :: CL3OamAlarmHandleNotify()
{

}

CL3OamAlarmHandleNotify :: CL3OamAlarmHandleNotify(CMessage& rMsg)
:CMessage(rMsg)
{

}

bool CL3OamAlarmHandleNotify :: CreateMessage(CComEntity& Entity)
{
    return CMessage :: CreateMessage(Entity);
}

UINT32 CL3OamAlarmHandleNotify :: GetDefaultDataLen() const
{
    return (sizeof(T_Notify));
}

UINT16 CL3OamAlarmHandleNotify :: GetTransactionId()const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CL3OamAlarmHandleNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

bool CL3OamAlarmHandleNotify:: SetMsgData(SINT8* SrcData, UINT16 Len)
{
    if((NULL == SrcData) ||(Len > 2))
    {
        return false;
    }
    else
    {
        memcpy(&(((T_Notify*)GetDataPtr())->Rsv),
               SrcData, 
               Len);
        return true;
    }
}


CL3OamAlarmHandleNotify :: ~CL3OamAlarmHandleNotify()
{
}
