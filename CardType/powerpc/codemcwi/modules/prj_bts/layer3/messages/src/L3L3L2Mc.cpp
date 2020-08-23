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

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3L3L2MCPSTATENOTIFY
#include "L3L3L2McpStateNotify.h"
#endif


CL3L2McpStateNoitfy :: CL3L2McpStateNoitfy()
{
}

CL3L2McpStateNoitfy :: CL3L2McpStateNoitfy(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3L2McpStateNoitfy :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L2_L3_MCPSTATE_NOTIFY);
    return true;
}

UINT32 CL3L2McpStateNoitfy :: GetDefaultDataLen() const
{
    return (sizeof(T_Notify));
}

UINT16 CL3L2McpStateNoitfy :: GetTransactionId()const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CL3L2McpStateNoitfy :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

const T_MCPStateInfo* CL3L2McpStateNoitfy ::  GetMCPStateInfo()const
{
    return (T_MCPStateInfo*)&(((T_Notify*)GetDataPtr())->MCPStateInfo);
}

CL3L2McpStateNoitfy :: ~CL3L2McpStateNoitfy()
{

}
#ifdef WBBU_CODE

CL3L2Core9StateNoitfy :: CL3L2Core9StateNoitfy()
{
}

CL3L2Core9StateNoitfy :: CL3L2Core9StateNoitfy(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3L2Core9StateNoitfy :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L2_CORE9_STATUS_NOTIFY);
    return true;
}

UINT32 CL3L2Core9StateNoitfy :: GetDefaultDataLen() const
{
    return (sizeof(T_Notify));
}

UINT16 CL3L2Core9StateNoitfy :: GetTransactionId()const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CL3L2Core9StateNoitfy :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

const T_Core9StateInfo* CL3L2Core9StateNoitfy ::  GetCORE9StateInfo()const
{
    return (T_Core9StateInfo*)&(((T_Notify*)GetDataPtr())->CORE9StateInfo);
}

CL3L2Core9StateNoitfy :: ~CL3L2Core9StateNoitfy()
{

}

//aif info

CL3L2AifStateNoitfy :: CL3L2AifStateNoitfy()
{
}

CL3L2AifStateNoitfy :: CL3L2AifStateNoitfy(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3L2AifStateNoitfy :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L2_AIF_STATAS_NOTIFY);
    return true;
}

UINT32 CL3L2AifStateNoitfy :: GetDefaultDataLen() const
{
    return (sizeof(T_Notify));
}

UINT16 CL3L2AifStateNoitfy :: GetTransactionId()const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CL3L2AifStateNoitfy :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

const T_AifStateInfo* CL3L2AifStateNoitfy ::  GetAifStateInfo()const
{
    return (T_AifStateInfo*)&(((T_Notify*)GetDataPtr())->Aifinfo);
}

CL3L2AifStateNoitfy :: ~CL3L2AifStateNoitfy()
{

}
#endif
