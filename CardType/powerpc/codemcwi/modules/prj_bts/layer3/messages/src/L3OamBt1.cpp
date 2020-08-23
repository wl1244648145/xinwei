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

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMBTSLOADINFONOTIFYTOEMS
#include "L3OamBtsLoadInfoNotifyToEms.h"
#endif


CL3OamBtsLoadInfoToEms :: CL3OamBtsLoadInfoToEms(CMessage &rMsg)    
:CMessage(rMsg)
{
}


CL3OamBtsLoadInfoToEms :: CL3OamBtsLoadInfoToEms()
{
}

bool CL3OamBtsLoadInfoToEms :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_BTS_EMS_BTSLOADINFOR_NOTIFY);
    return true;
}

UINT32 CL3OamBtsLoadInfoToEms :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}


UINT16 CL3OamBtsLoadInfoToEms :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CL3OamBtsLoadInfoToEms :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
		return 0;
}

CL3OamBtsLoadInfoToEms :: ~CL3OamBtsLoadInfoToEms()
{

}


