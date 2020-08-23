/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataDelEidTable.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/14/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_DEL_EID_TABLE_H__
#define __SNOOP_DEL_EID_TABLE_H__

#include "Message.h"
#include "L3DataMessages.h"

/*****************************************
 *CDelEidTable¿‡
 *****************************************/
class CDelEidTable:public CMessage
{
public:
    CDelEidTable(){}
    CDelEidTable(const CMessage &msg):CMessage( msg ){}
    ~CDelEidTable(){}

    UINT32 SetEidInPayload(UINT32);
    UINT32 GetEidInPayload() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif /*__SNOOP_DEL_EID_TABLE_H__*/
