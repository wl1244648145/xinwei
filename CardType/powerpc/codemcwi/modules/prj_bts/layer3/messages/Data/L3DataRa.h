/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataRaid.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/02/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_RAID_H__
#define __SNOOP_RAID_H__

#include "Message.h"

/*****************************************
 *CRAIDConfig��
 *OAM��Snoop����RAID��
 *****************************************/
class CRAIDConfig:public CMessage
{
public:
    CRAIDConfig(){}
    CRAIDConfig(const CMessage &msg):CMessage( msg ){}
    ~CRAIDConfig(){}

    UINT16 GetTransactionId() const;
    UINT8  GetAgentCircuitIDSubOption() const;
    UINT8  GetAgentRemoteIDSubOption() const;
    UINT8  GetPPPoERemoteIDSubOption() const;
    UINT32 GetRAID() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CCRAIDConfigResp��
 *Snoop��OAM����RAID��Ļ�Ӧ
 *****************************************/
class CRAIDConfigResp:public CMessage
{
public:
    CRAIDConfigResp(){}
    CRAIDConfigResp(const CMessage &msg):CMessage( msg ){}
    ~CRAIDConfigResp(){}

    UINT16 SetTransactionId(UINT16);
    UINT16 SetResult(UINT16);

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif /*__SNOOP_RAID_H__*/
