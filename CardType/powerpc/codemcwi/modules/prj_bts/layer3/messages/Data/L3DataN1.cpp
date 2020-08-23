/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataNotifyRefreshJammingNeighbor.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   05/14/07   xinwang		  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include "L3DataNotifyRefreshJammingNeighbor.h"
#include "L3DataMsgId.h"
#include "L3OAMMessageID.h"
#define M_SOCKET_JNT_REFRESH 0xDCBA
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif

UINT16 CDataRefreshJammingNeighbor::GetDefaultMsgId() const
{
    return M_SOCKET_JNT_REFRESH;
}

bool CDataRefreshJammingNeighbor :: CreateMessage(CComEntity& Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_SOCKET_JNT_REFRESH);
    return true;
}
UINT32 CDataRefreshJammingNeighbor::GetDefaultDataLen() const
{
    return sizeof( stNotifyRefreshJammingNeighbor );
}

UINT32 CDataRefreshJammingNeighbor::SetFlag(UINT32 flag)
{
    return ( (stNotifyRefreshJammingNeighbor*)GetDataPtr() )->flag = flag;

}
UINT32 CDataRefreshJammingNeighbor::GetFlag() const
{
    return ( (stNotifyRefreshJammingNeighbor*)GetDataPtr() )->flag;

}

