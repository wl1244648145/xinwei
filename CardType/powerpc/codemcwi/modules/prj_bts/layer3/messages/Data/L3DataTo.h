/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataTosSFID.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   08/12/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_TOSSFID_H__
#define __DATA_TOSSFID_H__

#include "Message.h"
#include "L3DataMessages.h"
#include "L3DataMsgId.h"

/*****************************************
 *CTosSFIDConfig类
 *OAM往EB配置SFID表
 *****************************************/
class CTosSFIDConfig:public CMessage
{
public:
    CTosSFIDConfig(){}
    CTosSFIDConfig(const CMessage &msg):CMessage( msg ){}
    ~CTosSFIDConfig(){}

    UINT16 SetTransactionId(UINT16 usTransId)
        {
        return ( (stTosSFIDConfig*)GetDataPtr() )->usTransId = usTransId;
        }
    UINT16 GetTransactionId() const
        {
            return ( (stTosSFIDConfig*)GetDataPtr() )->usTransId;
        }

    void SetSFID(UINT8 *pSFID)
        {
            memcpy( ( (stTosSFIDConfig*)GetDataPtr() )->aSFID, pSFID,\
                sizeof( ( (stTosSFIDConfig*)GetDataPtr() )->aSFID ) );
        }
    UINT8* GetSFID() const
        {
            return ( (stTosSFIDConfig*)GetDataPtr() )->aSFID;
        }

protected:
    UINT32 GetDefaultDataLen() const    { return sizeof( stTosSFIDConfig ); }
    UINT16 GetDefaultMsgId() const      { return M_EMS_BTS_CFG_SFID_REQ; };
};

/*****************************************
 *CTosSFIDConfigResp类
 *OAM往EB配置SFID表的回应
 *****************************************/
class CTosSFIDConfigResp:public CMessage
{
public:
    CTosSFIDConfigResp(){}
    CTosSFIDConfigResp(const CMessage &msg):CMessage( msg ){}
    ~CTosSFIDConfigResp(){}

    UINT16 SetTransactionId(UINT16 usTransId)
        {
            return ( (stTosSFIDConfigResp*)GetDataPtr() )->usTransId = usTransId;
        }
    UINT16 GetTransactionId()
        {
            return ( (stTosSFIDConfigResp*)GetDataPtr() )->usTransId;
        }

    UINT16 SetResult(UINT16 usResult)
        {
            return ( (stTosSFIDConfigResp*)GetDataPtr() )->usResult = htons( usResult );
        }

    UINT16 GetResult() const
        {
            return ntohs( ( (stTosSFIDConfigResp*)GetDataPtr() )->usResult );
        }

protected:
    UINT32 GetDefaultDataLen() const    { return sizeof( stTosSFIDConfigResp ); }
    UINT16 GetDefaultMsgId() const      { return M_BTS_EMS_QOS_CFG_RSP; }
};

#endif /*__DATA_TOSSFID_H__*/
