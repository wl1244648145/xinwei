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
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

// 5.3.2	Resource Management Policy Request£¨EMS£©
#ifndef _INC_L3OAMCFGRMPOLICYREQ
#include "L3OamCfgRMPolicy.h"       
#endif
#include <string.h>
CCfgRMPolicyReq :: CCfgRMPolicyReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgRMPolicyReq :: CCfgRMPolicyReq()
{
}

bool CCfgRMPolicyReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_RF_CFG_REQ);
	return true;
}

UINT32 CCfgRMPolicyReq :: GetDefaultDataLen() const
{
    return sizeof(T_RMPoliceReq);
}

UINT16 CCfgRMPolicyReq :: GetTransactionId() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->TransId;
}

UINT16 CCfgRMPolicyReq :: SetTransactionId(UINT16 TransId)
{
    ((T_RMPoliceReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CCfgRMPolicyReq :: GetBWReqInterval() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.BWReqInterval;
}

void   CCfgRMPolicyReq :: SetBWReqInterval(UINT16 BWReqInterval)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.BWReqInterval = BWReqInterval;
}

UINT16 CCfgRMPolicyReq :: GetSRelThreshold() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.SRelThreshold;
}

void   CCfgRMPolicyReq :: SetSRelThreshold(UINT16 SRelThreshold)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.SRelThreshold = SRelThreshold;
}

SINT16 CCfgRMPolicyReq :: GetMinULSS() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.MinULSS;
}

void   CCfgRMPolicyReq :: SetMinULSS(SINT16 MinULSS)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.MinULSS = MinULSS;
}

UINT16  CCfgRMPolicyReq :: GetMaxDLPPUser() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.MaxDLPPUser;
}

void   CCfgRMPolicyReq :: SetMaxDLPPUser(UINT16 MaxDLPPUser)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.MaxDLPPUser = MaxDLPPUser;
}

UINT16 CCfgRMPolicyReq :: GetDLBWPerUser() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.DLBWPerUser;
}

void   CCfgRMPolicyReq :: SetDLBWPerUser(UINT16 DLBWPerUser)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.DLBWPerUser = DLBWPerUser;
}

UINT16 CCfgRMPolicyReq :: GetULBWPerUser() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.ULBWPerUser;
}

void   CCfgRMPolicyReq :: SetULBWPerUser(UINT16 ULBWPerUser)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.ULBWPerUser = ULBWPerUser;
}

UINT16 CCfgRMPolicyReq :: GetRsvTCH() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.RsvTCH;
}

void   CCfgRMPolicyReq :: SetRsvTCH(UINT16 RsvTCH)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.RsvTCH = RsvTCH;
}

UINT16 CCfgRMPolicyReq :: GetRsvPower() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.RsvPower;
}

void   CCfgRMPolicyReq :: SetRsvPower(UINT16 RsvPower)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.RsvPower = RsvPower;
}

UINT16 CCfgRMPolicyReq :: GetPCRange() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.PCRange;
}

void   CCfgRMPolicyReq :: SetPCRange(UINT16 PCRange)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.PCRange = PCRange;
}

UINT16 CCfgRMPolicyReq :: GetStepSize() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.StepSize;
}

void   CCfgRMPolicyReq :: SetStepSize(UINT16 StepSize)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.StepSize = StepSize;
}

UINT16 CCfgRMPolicyReq :: GetMaxUser() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.MaxUser;
}

void   CCfgRMPolicyReq :: SetMaxUser(UINT16 MaxUser)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.MaxUser = MaxUser;
}

UINT8* CCfgRMPolicyReq :: GetBWDistClass() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.BWDistClass;
}

void   CCfgRMPolicyReq :: SetBWDistClass(UINT8* )
{
//    ((T_RMPoliceReq *)GetDataPtr())->Ele.BWDistClass;
}

UINT8 CCfgRMPolicyReq :: GetULModMask() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.ULModMask;
}

void  CCfgRMPolicyReq ::  SetULModMask(UINT8 ULModMask)
{
    ((T_RMPoliceReq *)GetDataPtr())->Ele.ULModMask = ULModMask;
}

UINT8 CCfgRMPolicyReq :: GetDLModMask() const
{
    return ((T_RMPoliceReq *)GetDataPtr())->Ele.DLModMask;
}

void  CCfgRMPolicyReq ::  SetDLModMask(UINT8 DLModMask)
{
    ((T_RMPoliceReq*)GetDataPtr())->Ele.DLModMask = DLModMask;
}

SINT8* CCfgRMPolicyReq :: GetEle() const
{
    return (SINT8*) (&(((T_RMPoliceReq *)GetDataPtr())->Ele));
}

void CCfgRMPolicyReq :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_RMPoliceReq *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_RMPoliceEle));
}

bool   CCfgRMPolicyReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_RMPoliceReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgRMPolicyReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_RMPoliceReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgRMPolicyReq :: ~CCfgRMPolicyReq()
{

}

