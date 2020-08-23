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

#ifndef _INC_L3OAMCFGBTSREPEATERREQ
#include "L3OamCfgBtsRepeaterReq.h"
#endif




#include <string.h>
extern T_NvRamData *NvRamDataAddr;
CL3OamCfgBtsRepeaterReq :: CL3OamCfgBtsRepeaterReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CL3OamCfgBtsRepeaterReq :: CL3OamCfgBtsRepeaterReq()
{
}

bool CL3OamCfgBtsRepeaterReq :: CreateMessage(CComEntity &Entity, UINT32 uDataSize)
{
    CMessage :: CreateMessage(Entity, uDataSize);
    SetMessageId(M_EMS_BTS_BTS_REPEATER_CFG_REQ);
	return true;
}

UINT32 CL3OamCfgBtsRepeaterReq :: GetDefaultDataLen() const
{
    return sizeof(T_Req);
}

UINT32 CL3OamCfgBtsRepeaterReq :: GetRealDataLen() const   
{
    UINT16 Num = ((T_Req *)GetDataPtr())->RepeaterFreqNum;
	if(Num > NEIGHBOR_BTS_NUM)
	{
        return sizeof(UINT16);
	}

	return (Num+1) * sizeof(UINT16);
}

UINT32 CL3OamCfgBtsRepeaterReq :: GetDataLenFromNvram() const  
{
    UINT16 Num = NvRamDataAddr->BTSRepeaterEle.RepeaterFreqNum;
	if(Num > NEIGHBOR_BTS_NUM)
	{
        return sizeof(UINT16);  //size for the repeater number field 
	}

	return (Num+1) * sizeof(UINT16);
}


UINT16 CL3OamCfgBtsRepeaterReq :: GetTransactionId() const
{
    return ((T_Req *)GetDataPtr())->TransId;
}

UINT16 CL3OamCfgBtsRepeaterReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
	return 0;
}


CL3OamCfgBtsRepeaterReq :: ~CL3OamCfgBtsRepeaterReq()
{

}

