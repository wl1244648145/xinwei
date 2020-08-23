/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSyncIL.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/05/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_SYNC_H__
#define __SNOOP_SYNC_H__

#include "Message.h"
#include "L3DataMessages.h"

/*****************************************
 *CSyncIL类
 *****************************************/
class CSyncIL:public CMessage
{
public:
    CSyncIL(){}
    CSyncIL(const CMessage &msg):CMessage( msg ){}
    ~CSyncIL(){}

    UINT32 SetEidInPayload(UINT32);
    UINT32 GetEidInPayload() const;

    UINT8 SetOp(UINT8);
    UINT8 GetOp() const;

    UINT8 SetIpType(UINT8);
    UINT8 GetIpType() const;

    bool SetNeedResp(bool);
    bool GetNeedResp() const;

    void SetEntry(const UTILEntry&);
    UTILEntry& GetEntry() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CSyncILResp类
 *DM往SNOOP的回应
 *****************************************/
class CSyncILResp:public CMessage
{
public:
    CSyncILResp(){}
    CSyncILResp(const CMessage &msg):CMessage( msg ){}
    ~CSyncILResp(){}

    UINT32 SetEidInPayload(UINT32);
    UINT32 GetEidInPayload() const;

    void SetMac(const UINT8*);
    UINT8* GetMac() const;

    UINT8 SetIpType(UINT8);
    UINT8 GetIpType() const;

    bool SetResult(bool);
    bool GetResult() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif /*__SNOOP_SYNC_H__*/

