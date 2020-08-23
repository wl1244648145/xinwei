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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

//5.4.4.1	Calibration Result Notification（BTS）
#ifndef _INC_L3OAMCFGCALRSTGENNOTIFY
#include "L3OamCfgCalRstGenNotify.h"   
#endif
#include <string.h>

CCfgCalRstGenNotify :: CCfgCalRstGenNotify(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgCalRstGenNotify :: CCfgCalRstGenNotify()
{
}

bool CCfgCalRstGenNotify :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_BTS_EMS_CALIBRAT_CFG_GENDATA_RSP);
	return true;
}

UINT32 CCfgCalRstGenNotify :: GetDefaultDataLen() const
{
    return sizeof(T_CalRstGenNotify);
}

UINT16 CCfgCalRstGenNotify :: GetTransactionId() const
{
    return ((T_CalRstGenNotify *)GetDataPtr())->TransId;
}

UINT16 CCfgCalRstGenNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_CalRstGenNotify *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT8* CCfgCalRstGenNotify :: GetEle() const
{
    return (SINT8*) (&(((T_CalRstGenNotify *)GetDataPtr())->Ele));
}

void   CCfgCalRstGenNotify :: SetEle(SINT8 * E)
{
    memcpy((SINT8*) (&(((T_CalRstGenNotify *)GetDataPtr())->Ele)), 
            E, 
            sizeof(T_CaliGenCfgEle));
}

bool   CCfgCalRstGenNotify :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_CalRstGenNotify *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgCalRstGenNotify :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_CalRstGenNotify*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}
#if 0
bool   CCfgCalRstGenNotify :: IsAntennaErr()
{
    for(UINT8 i = 0; i < ANTENNA_NUM; i++)
    {
        if(((T_CalRstGenNotify*)GetDataPtr())->Error.CaliAntErr[i] != 0)
        return true;    
    }

    return false;
}


bool   CCfgCalRstGenNotify :: IsSynErr()
{
    return (((T_CalRstGenNotify*)GetDataPtr())->Error.SYNError != 0);
}

bool   CCfgCalRstGenNotify :: IsPreDistortErr()
{
    return (((T_CalRstGenNotify*)GetDataPtr())->Error.PreDistortErr != 0);
}

bool   CCfgCalRstGenNotify :: IsPreCalibrationErr()
{
    return (((T_CalRstGenNotify*)GetDataPtr())->Error.PreCalibrationErr != 0);
}
#endif
#ifndef WBBU_CODE
bool   CCfgCalRstGenNotify::IsCalibrationErr()
{
    return (bool)(((T_CalRstGenNotify*)GetDataPtr())->Error.calCorrFlag);
}
#else
unsigned short   CCfgCalRstGenNotify::IsCalibrationErr()
{
    return (unsigned short )(((T_CalRstGenNotify*)GetDataPtr())->Error.calCorrFlag);
}

#endif    
UINT16  CCfgCalRstGenNotify :: GetErrorAntenna()   
{
    UINT16 State = 0;//每一位标识一个天线的状态 0 - disable     1 - enable
  
    for(UINT8 i = 0; i < ANTENNA_NUM; i++) 
    {
        if(((T_CalRstGenNotify*)GetDataPtr())->Error.CaliAntErr[i] == 0)
        {
            State = State | 1;
        }
        State = State << 1;
    }

    return State;
}



CCfgCalRstGenNotify :: ~CCfgCalRstGenNotify()
{

}

