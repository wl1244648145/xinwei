/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3L3L2BtsRstCntChangeNotify.cpp
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

#ifndef _INC_L3L2BTSRSTCNTCHANGENOTIFY
#include "L3L3L2BtsRstCntChangeNotify.h"
#endif


CL3L2BtsRstCntChangeNotify :: CL3L2BtsRstCntChangeNotify()
{
}

CL3L2BtsRstCntChangeNotify :: CL3L2BtsRstCntChangeNotify(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3L2BtsRstCntChangeNotify :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_L3_L2_BTSRSTCNT_CHANGE_NOTIFY);
    return true;
}

UINT32 CL3L2BtsRstCntChangeNotify :: GetDefaultDataLen() const
{
    return sizeof(T_Notify);
}

UINT16 CL3L2BtsRstCntChangeNotify :: GetTransactionId() const
{
    return ((T_Notify *)GetDataPtr())->TransId;
}

UINT16 CL3L2BtsRstCntChangeNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CL3L2BtsRstCntChangeNotify :: GetRstCnt() const
{
    return ((T_Notify *)GetDataPtr())->RstCnt;
}

void CL3L2BtsRstCntChangeNotify :: SetRstCnt(UINT16 R)
{
    ((T_Notify *)GetDataPtr())->RstCnt = R;
}


CL3L2BtsRstCntChangeNotify :: ~CL3L2BtsRstCntChangeNotify()
{

}


