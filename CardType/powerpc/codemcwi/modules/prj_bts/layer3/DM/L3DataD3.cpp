/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    SnoopTimer.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   10/12/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include "L3DataDmTimerExpire.h"
#include "L3DataAssert.h"


//----------------------------------------------
//CTimerExpire�ඨ��
//----------------------------------------------

UINT32 CDmTimerExpire::GetDefaultDataLen() const
{
    return sizeof(DmTimerExpire);
}

UINT16 CDmTimerExpire::GetDefaultMsgId() const
{
    return MSGID_TIMER_DM;
}
void CDmTimerExpire::SetInEid(UINT32 Eid) 
{
    ( (DmTimerExpire*)GetDataPtr() )->Eid = Eid;
}
void CDmTimerExpire::SetTimerType(UINT8 Type) 
{
    ( (DmTimerExpire*)GetDataPtr() )->TimerType = Type;
}

//------------------------------------------
//------------------------------------------
UINT8 CDmTimerExpire::GetTimerType() const
{
    return( (DmTimerExpire*)GetDataPtr() )->TimerType;
}


//------------------------------------------
//------------------------------------------
UINT32 CDmTimerExpire::GetInEid() const
{
    return( (DmTimerExpire*)GetDataPtr() )->Eid;
}
/**********************************************************************
*
*  NAME:          SetTransactionId
*  FUNTION:       ��¼tranid
*  INPUT:          tranid
*  OUTPUT:        ��
*  OTHERS:        jiaying20100813
**********************************************************************/

UINT16 CDmTimerExpire::SetTransactionId(UINT16 tranid)
{
    ( (DmTimerExpire*)GetDataPtr() )->tranid = tranid;
}
/**********************************************************************
*
*  NAME:          GetTransactionId
*  FUNTION:       ���tranid
*  INPUT:          ��
*  OUTPUT:        tranid
*  OTHERS:        jiaying20100813
**********************************************************************/
UINT16 CDmTimerExpire::GetTransactionId()const
{
    return( (DmTimerExpire*)GetDataPtr() )->tranid;
}

