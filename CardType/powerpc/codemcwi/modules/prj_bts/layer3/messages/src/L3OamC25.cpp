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

#ifndef _INC_L3OAMCFGNEIGHBOURBTSLOADINFOREQ
#include "L3OamCfgNeighbourBtsLoadInfo.h"
#endif

#include <string.h>

bool CL3OamSyncNeighbourBtsLoadInfoReq :: CreateMessage(CComEntity &Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        {
        return false;
        }
    SetMessageId(M_EMS_BTS_NEIGHBOUR_BTS_LOADINFO_CFG_REQ);
    return true;
}

UINT32 CL3OamSyncNeighbourBtsLoadInfoReq :: GetDefaultDataLen() const
{
    return sizeof(T_NeighbotBTSLoadInfo);
}

UINT32 CL3OamSyncNeighbourBtsLoadInfoReq :: GetRealDataLen() const
{
#if 0
    UINT16 NeighborBTSNum = ((T_Req *)GetDataPtr())->NeighborBTSNum;
    if(NeighborBTSNum > NEIGHBOR_BTS_NUM)
        {
        return 2;
        }

    return NeighborBTSNum * sizeof(T_NeighbotBTSLoadInfo) + 4;
#else
    return sizeof(T_NeighbotBTSLoadInfo);
#endif
}

#if 0
UINT16 CL3OamSyncNeighbourBtsLoadInfoReq :: GetTransactionId() const
{
    return ((T_Req *)GetDataPtr())->TransId;
}

UINT16 CL3OamSyncNeighbourBtsLoadInfoReq :: SetTransactionId(UINT16 TransId)
{
    ((T_Req *)GetDataPtr())->TransId = TransId;
    return 0;
}
#endif

void CL3OamSyncNeighbourBtsLoadInfoReq::setLoadInfo(const SINT8 *pLoadInfo)
{
    memcpy((UINT8*)GetDataPtr(), pLoadInfo, sizeof(T_NeighbotBTSLoadInfo));
}


T_NeighbotBTSLoadInfo* CL3OamSyncNeighbourBtsLoadInfoReq::getLoadInfo()const
{
    return (T_NeighbotBTSLoadInfo*)GetDataPtr();
}
