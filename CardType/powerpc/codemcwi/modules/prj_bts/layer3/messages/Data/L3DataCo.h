/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataConfig.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   08/09/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __L3_DATACONFIG_H__
#define __L3_DATACONFIG_H__

#include "Message.h"
#include "L3DataCommon.h"

/*****************************************
 *CDataConfig类
 *OAM发给EtherBridge的配置消息
 *****************************************/
class CDataConfig:public CMessage
{
public:
//    CDataConfig(){}
    CDataConfig(const CMessage &msg):CMessage( msg ){}
    ~CDataConfig(){}

//    UINT16 SetTransactionId(UINT16);
    UINT16 GetTransactionId() const;

//    bool SetEgressBCFltr(bool);
    bool GetEgressBCFltr() const;

//    UINT16 SetLearnedBridgingAgingTime(UINT16);
    UINT16 GetLearnedBridgingAgingTime() const;

//    UINT16 SetPPPoESessionKeepAliveTime(UINT16);
    UINT16 GetPPPoESessionKeepAliveTime() const;

//    UINT8 SetWorkMode(UINT8);
    UINT8 GetWorkMode() const;

protected:
//    UINT32 GetDefaultDataLen() const;
//    UINT16 GetDefaultMsgId() const;
};


/*****************************************
 *CDataConfigResp类
 *EB处理完OAM的配置消息后往OAM回应的消息
 *****************************************/
class CDataConfigResp:public CMessage
{
public:
    CDataConfigResp(){}
    CDataConfigResp(CMessage &msg):CMessage( msg ){}
    ~CDataConfigResp(){}

    UINT16 SetTransactionId(UINT16);
    UINT16 GetTransactionId();
    UINT16 SetResult(UINT16);
    UINT16 GetResult() const;

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
};

#endif  /*__L3_DATACONFIG_H__*/
