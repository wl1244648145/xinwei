/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataRaid.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   08/12/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

//socket:
#ifdef __WIN32_SIM__
#include <winsock2.h>
#else	//VxWorks:
#include "vxWorks.h" 
#include "sockLib.h" 
#include "inetLib.h" 
#include "stdioLib.h" 
#include "strLib.h" 
#include "hostLib.h" 
#include "ioLib.h" 
#endif

#include "L3DataRaid.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"
#include "L3OAMMessageId.h"

//----------------------------------------------------
//CRAIDConfig类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CRAIDConfig::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CRAIDConfig::GetDefaultDataLen() const
{
    return sizeof( stRAIDConfig );
}

UINT16 CRAIDConfig::GetDefaultMsgId() const
{
    return M_CFG_SNOOP_DATA_SERVICE_CFG_REQ;
}

UINT16 CRAIDConfig::GetTransactionId() const
{
    return ( (stRAIDConfig*)GetDataPtr() )->usTransId;
}

UINT8 CRAIDConfig::GetAgentCircuitIDSubOption() const
{
    return ( (stRAIDConfig*)GetDataPtr() )->ucAgentCircuitID;
}

UINT8 CRAIDConfig::GetAgentRemoteIDSubOption() const
{
    return ( (stRAIDConfig*)GetDataPtr() )->ucAgentRemoteID;
}

UINT8  CRAIDConfig::GetPPPoERemoteIDSubOption() const
{
    return ( (stRAIDConfig*)GetDataPtr() )->ucPPPoERemoteID;
}
UINT32 CRAIDConfig::GetRAID() const
{
    return ntohl( ( (stRAIDConfig*)GetDataPtr() )->ulRouterAreaId );
}


//----------------------------------------------------
//CRAIDConfigResp类定义
//----------------------------------------------------


UINT16 CRAIDConfigResp::SetTransactionId(UINT16 usTransId)
{
    return ( (stRAIDConfigResp*)GetDataPtr() )->usTransId = usTransId;
}

UINT16 CRAIDConfigResp::SetResult(UINT16 usResult)
{
    return ( (stRAIDConfigResp*)GetDataPtr() )->usResult = htons( usResult );
}


/*============================================================
MEMBER FUNCTION:
    CRAIDConfigResp::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CRAIDConfigResp::GetDefaultDataLen() const
{
    return sizeof( stRAIDConfigResp );
}


UINT16 CRAIDConfigResp::GetDefaultMsgId() const
{
    return M_SNOOP_CFG_DATA_SERVICE_CFG_RSP;
}
