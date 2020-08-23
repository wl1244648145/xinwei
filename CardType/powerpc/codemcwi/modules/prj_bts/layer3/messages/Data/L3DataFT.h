/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataFTAddEntry.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   03/28/06   xiao weifang  FixIp用户做成非永久性用户
 *   08/09/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_FTADDENTRY_H__
#define __DATA_FTADDENTRY_H__

#include "Message.h"

/*****************************************
 *CFTAddEntry类
 *****************************************/
class CFTAddEntry:public CMessage
{
public:
    CFTAddEntry(){}
    CFTAddEntry(const CMessage &msg):CMessage( msg ){}
    virtual ~CFTAddEntry(){}

    UINT32 SetEidInPayload(UINT32);
    UINT32 GetEidInPayload() const;

    void SetMac(const UINT8*);
    UINT8* GetMac() const;

    bool SetServing(bool);
    bool GetServing() const;

    bool SetTunnel(bool);
    bool GetTunnel() const;

    UINT32 SetPeerBtsID(UINT32);
    UINT32 GetPeerBtsID() const;

    UINT8  SetIpType(UINT8);
    UINT8  GetIpType() const;

    UINT16 setGroupId(UINT16);
    UINT16 getGroupId() const;

    bool   SetAuth(bool);
    bool   GetAuth() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};


/*************************************************************
 *CFTUpdateEntry类
 *FTUpdateEntry具有和FTAddEntry相同的属性
 *只是在Snoop任务发送消息时，MessageId不同，
 *所以CFTUpdateEntry继承自CFTAddEntry，但需重载CreateMessage
 *************************************************************/
class CFTUpdateEntry:public CFTAddEntry
{
public:
    CFTUpdateEntry():m_bRefreshTTLOnly(false){}
    CFTUpdateEntry(const CMessage &msg):CFTAddEntry( msg ), m_bRefreshTTLOnly(false){}
    ~CFTUpdateEntry(){}

    void setRefreshTTLOnly(bool flag)   {m_bRefreshTTLOnly = flag;}
    bool getRefreshTTLOnly() const      {return m_bRefreshTTLOnly;}

protected:
    UINT16 GetDefaultMsgId() const;
    UINT32 GetDefaultDataLen() const;
private:
    bool m_bRefreshTTLOnly;
};

#endif /*__DATA_FTADDENTRY_H__*/

