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
 *   07/08/2008   sunshanggu
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3L3CPEETHCFGRSP
#include "L3L3CpeEthCfgRsp.h"
#endif


CL3L3CpeEthCfgRsp :: CL3L3CpeEthCfgRsp()
{
}

CL3L3CpeEthCfgRsp :: CL3L3CpeEthCfgRsp(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3L3CpeEthCfgRsp :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    return true;
}

UINT32 CL3L3CpeEthCfgRsp :: GetDefaultDataLen() const
{
    return sizeof(T_Rsp);
}


UINT16 CL3L3CpeEthCfgRsp :: GetCmdType() const
{
    return ((T_Rsp*)GetDataPtr())->CmdType;
}

UINT8 * CL3L3CpeEthCfgRsp :: GetCmdContent() const
{
    return ((T_Rsp*)GetDataPtr())->Content;
}

CL3L3CpeEthCfgRsp :: ~CL3L3CpeEthCfgRsp()
{

}


