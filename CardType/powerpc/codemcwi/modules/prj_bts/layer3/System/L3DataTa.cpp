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

#include "L3DataEB.h"
#include "L3DataDM.h"
#include "L3DataSnoop.h"
#include "L3DataTunnel.h"
//#include "L3DataTDR.h"
//#include "L3DataTCR.h"
#include "L3DataARP.h"

#include <string.h>


#ifdef __WIN32_SIM__
#else
#include <rebootLib.h> 
#endif



bool InitL3DataSvc()
{

	CTBridge *taskEB = CTBridge::GetInstance();
	taskEB->Begin();
#if 1
	CTSnoop *taskSnoop = CTSnoop::GetInstance();
	taskSnoop->Begin();

	CTunnel *taskTunnel = CTunnel::GetInstance();
	taskTunnel->Begin();
/*
	CTaskTDR *taskTDR = CTaskTDR::GetInstance();
	taskTDR->Begin();

	CTaskTCR *taskTCR = CTaskTCR::GetInstance();
	taskTCR->Begin();
*/
	CTaskDm *taskDM = CTaskDm::GetInstance();
	taskDM->Begin();

	CTaskARP *taskARP = CTaskARP::GetInstance();
	taskARP->Begin(); 
#endif
    return true;
}


bool InitL3Oam()
{
	return true;
}
bool InitL3VoiceSvc()
{
        return true;
}

