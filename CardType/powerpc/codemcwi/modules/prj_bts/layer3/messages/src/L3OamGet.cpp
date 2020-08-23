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
----
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMGETBOARDSSTATERSP
#include "L3OamGetBoardsStateRsp.h"
#endif

#include "string.h"

CL3GetBoardsStatesRsp :: CL3GetBoardsStatesRsp(CMessage &rMsg)    
    :CMessage(rMsg)
{
}

CL3GetBoardsStatesRsp :: CL3GetBoardsStatesRsp()
{
}

bool CL3GetBoardsStatesRsp :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_BTS_EMS_GET_BOARDS_STATE_RSP);
    return true;
}

UINT32 CL3GetBoardsStatesRsp :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CL3GetBoardsStatesRsp :: GetTransactionId() const
{
    return ((T_Req*)GetDataPtr())->TransId;
}

UINT16 CL3GetBoardsStatesRsp :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
	return 0;
}



UINT16 CL3GetBoardsStatesRsp :: GetResult() const
{
    return ((T_Req*)GetDataPtr())->Result;
}

UINT16 CL3GetBoardsStatesRsp :: SetResult(UINT16 r)
{
    ((T_Req *)GetDataPtr())->Result = r;
	return 0;
}



void  CL3GetBoardsStatesRsp :: SetEle(SINT8* pData, UINT32 Len)
{
    if((pData != NULL)&&(Len<= GetDefaultDataLen()))
    {
         memcpy(&(((T_Req *)GetDataPtr())->Ele) , pData, Len); 
	}
}

CL3GetBoardsStatesRsp :: ~CL3GetBoardsStatesRsp()
{
}

