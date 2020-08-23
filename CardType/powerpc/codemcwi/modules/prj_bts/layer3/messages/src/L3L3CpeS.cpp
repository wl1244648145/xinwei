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

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef   _INC_L3L3CPESWDLPACKREQ
#include "L3L3CpeSWDLPackReq.h"
#endif

#include <string.h>
#if 0
CL3CpeSWBCDLPackReq :: CL3CpeSWBCDLPackReq()
{
}

CL3CpeSWBCDLPackReq :: CL3CpeSWBCDLPackReq(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3CpeSWBCDLPackReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L3_CPE_UPGRADE_SW_PACK_REQ);
    return true;
}

UINT32 CL3CpeSWBCDLPackReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CL3CpeSWBCDLPackReq :: GetTransactionId() const
{
    return ((T_Req *)GetDataPtr())->TransId;
}

UINT16 CL3CpeSWBCDLPackReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT16 CL3CpeSWBCDLPackReq ::GetDLReqSeqNum()const
{
    return ((T_Req *)GetDataPtr())->DLReqSeqNum;
}

void  CL3CpeSWBCDLPackReq :: SetDLReqSeqNum(UINT16 E)
{
    ((T_Req *)GetDataPtr())->DLReqSeqNum = E;
}

UINT16 CL3CpeSWBCDLPackReq ::GetSWPackSeqNum()const
{
    return ((T_Req *)GetDataPtr())->SWPackSeqNum;
}

void  CL3CpeSWBCDLPackReq :: SetSWPackSeqNum(UINT16 E)
{
    ((T_Req *)GetDataPtr())->SWPackSeqNum = E;
}


SINT8* CL3CpeSWBCDLPackReq :: GetSWPackData()const
{
    return ((T_Req *)GetDataPtr())->SWPackData;
}

void  CL3CpeSWBCDLPackReq :: SetSWPackData(SINT8* E, UINT16 Len)
{
    memcpy((SINT8*)((SINT8*)GetDataPtr() + sizeof(T_ReqBase)), E, Len);
}

CL3CpeSWBCDLPackReq :: ~CL3CpeSWBCDLPackReq()
{

}
#endif 0
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////    CL3CpeSWUCDLPackReq   ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

CL3CpeSWUCDLPackReq :: CL3CpeSWUCDLPackReq()
{
}

CL3CpeSWUCDLPackReq :: CL3CpeSWUCDLPackReq(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3CpeSWUCDLPackReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L3_CPE_UPGRADE_SW_PACK_REQ);
    return true;
}

UINT32 CL3CpeSWUCDLPackReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT16 CL3CpeSWUCDLPackReq :: GetTransactionId() const
{
    return ((T_Req *)GetDataPtr())->TransId;
}

UINT16 CL3CpeSWUCDLPackReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT16 CL3CpeSWUCDLPackReq ::GetDLReqSeqNum()const
{
    return ((T_Req *)GetDataPtr())->DLReqSeqNum;
}

void  CL3CpeSWUCDLPackReq :: SetDLReqSeqNum(UINT16 E)
{
    ((T_Req *)GetDataPtr())->DLReqSeqNum = E;
}

UINT16 CL3CpeSWUCDLPackReq ::GetSWPackSeqNum()const
{
    return ((T_Req *)GetDataPtr())->SWPackSeqNum;
}

void  CL3CpeSWUCDLPackReq :: SetSWPackSeqNum(UINT16 E)
{
    ((T_Req *)GetDataPtr())->SWPackSeqNum = E;
}


SINT8* CL3CpeSWUCDLPackReq :: GetSWPackData()const
{
    return ((T_Req *)GetDataPtr())->SWPackData;
}

void  CL3CpeSWUCDLPackReq :: SetSWPackData(SINT8* E, UINT16 Len)
{
    memcpy((SINT8*)((SINT8*)GetDataPtr() + sizeof(T_ReqBase)), E, Len);
}

CL3CpeSWUCDLPackReq :: ~CL3CpeSWUCDLPackReq()
{

}


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////             For Z Software Update           //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CL3ZSWUCDLPackReq :: CL3ZSWUCDLPackReq(){}
CL3ZSWUCDLPackReq :: CL3ZSWUCDLPackReq(CMessage& rMsg):CMessage(rMsg){}
CL3ZSWUCDLPackReq :: ~CL3ZSWUCDLPackReq(){}
UINT32 CL3ZSWUCDLPackReq :: GetDefaultDataLen() const{    return sizeof(T_ZPACK_Req);}

bool CL3ZSWUCDLPackReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L3_CPE_DL_Z_SW_PACK_REQ);
    return true;
}
UINT16  CL3ZSWUCDLPackReq :: SetTransactionId(UINT16 E)
{ 
	((T_ZPACK_Req *)GetDataPtr())->TransId = E; 
	return 0;
}

void  CL3ZSWUCDLPackReq :: SetDLReqSeqNum(UINT16 E)  { ((T_ZPACK_Req *)GetDataPtr())->DLReqSeqNum = E;  }
void  CL3ZSWUCDLPackReq :: SetSWPackSeqNum(UINT16 E) { ((T_ZPACK_Req *)GetDataPtr())->SWPackSeqNum = E; }
void  CL3ZSWUCDLPackReq :: SetPID(UINT32 E)                  { ((T_ZPACK_Req *)GetDataPtr())->ulPID = E;  }
void  CL3ZSWUCDLPackReq :: SetSWPackData(SINT8* E, UINT16 Len){ memcpy((SINT8*)((SINT8*)GetDataPtr() + sizeof(T_ZPACK_Base)), E, Len); }


