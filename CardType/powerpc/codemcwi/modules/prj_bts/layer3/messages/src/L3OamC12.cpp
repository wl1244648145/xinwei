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

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3OAMCFGCALIBRATIONTIMER
#include "L3OamCfgCalibrationTimer.h"
#endif


CL3CalibrationTimer :: CL3CalibrationTimer()
{
}

CL3CalibrationTimer :: CL3CalibrationTimer(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3CalibrationTimer :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_OAM_CALIBRATION_TIMER);
    return true;
}

CL3CalibrationTimer :: ~CL3CalibrationTimer()
{

}

