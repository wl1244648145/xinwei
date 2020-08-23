/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataConfig.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   08/09/05   xiao weifang  initialization. 
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

#include "L3DataConfig.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"
#include "L3OAMMessageId.h"

//----------------------------------------------------
//CDataConfig类定义
//----------------------------------------------------


UINT16 CDataConfig::GetTransactionId() const
{
    return ( (stDataConfig*)GetDataPtr() )->usTransId;
}

bool CDataConfig::GetEgressBCFltr() const
{
    return ( 0 == ( (stDataConfig*)GetDataPtr() )->ucEgressBCFltr )?false:true;
}

UINT16 CDataConfig::GetLearnedBridgingAgingTime() const
{
    return ntohs( ( (stDataConfig*)GetDataPtr() )->usLearnedBridgingAgingTime );
}

UINT16 CDataConfig::GetPPPoESessionKeepAliveTime() const
{
    return ntohs( ( (stDataConfig*)GetDataPtr() )->usPPPoESessionKeepAliveTime );
}

UINT8 CDataConfig::GetWorkMode() const
{
    return ( (stDataConfig*)GetDataPtr() )->ucWorkMode;
}



//----------------------------------------------------
//CDataConfigResp类定义
//----------------------------------------------------


/*============================================================
MEMBER FUNCTION:
    CDataConfigResp::GetDefaultMsgId

DESCRIPTION:
    获取消息ID

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT16: MsgID

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CDataConfigResp::GetDefaultMsgId() const
{
    return M_EB_CFG_DATA_SERVICE_CFG_RSP;
}


UINT16 CDataConfigResp::SetTransactionId(UINT16 usTransId)
{
    return ( (stDataConfigResp*)GetDataPtr() )->usTransId = usTransId;
}

UINT16 CDataConfigResp::GetTransactionId()
{
    return ( (stDataConfigResp*)GetDataPtr() )->usTransId;
}

UINT16 CDataConfigResp::SetResult(UINT16 usResult)
{
    return ( (stDataConfigResp*)GetDataPtr() )->usResult = htons( usResult );
}

UINT16 CDataConfigResp::GetResult() const
{
    return ntohs( ( (stDataConfigResp*)GetDataPtr() )->usResult );
}

/*============================================================
MEMBER FUNCTION:
    CDataConfigResp::GetDefaultDataLen

DESCRIPTION:
    获取CMessage Payload的长度

ARGUMENTS:
    NULL

RETURN VALUE:
    UINT32: 长度

SIDE EFFECTS:
    none
==============================================================*/
UINT32 CDataConfigResp::GetDefaultDataLen() const
{
    return sizeof( stDataConfigResp );
}
