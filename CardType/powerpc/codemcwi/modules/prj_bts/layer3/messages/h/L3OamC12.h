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

////////////////////////////////////////////////////
//��L2�·�calibration������Ϣ
#ifndef _INC_L3OAMCFGCALIBRATIONTIMER
#define _INC_L3OAMCFGCALIBRATIONTIMER

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3CalibrationTimer : public CMessage
{
public: 
    CL3CalibrationTimer(CMessage &rMsg);
    CL3CalibrationTimer();
    bool CreateMessage(CComEntity& Entity);
    ~CL3CalibrationTimer();
private:
     
};

#endif
