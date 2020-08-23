/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataACLConfig.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   10/12/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __ACL_CONFIG_H__
#define __ACL_CONFIG_H__

#include "L3EmsMessageId.h"

#include "Message.h"
#include "L3DataDmMessage.h"


class SetACLConfigReq:public CMessage
    {
public: 
    SetACLConfigReq(CMessage &rMsg):CMessage( rMsg ){}
    SetACLConfigReq(){}
    ~SetACLConfigReq(){}

public:

    UINT16   GetXid() const ;
    UINT8    GetIndex() const ;
    UINT8    GetACLCount() const ;
    const ACLConfig* GetACLConfigEntry();

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
    };

// ACL Config Response 
class SetACLConfigRsp: public CMessage
    {
public: 
    SetACLConfigRsp(CMessage &rMsg):CMessage( rMsg ){}
    SetACLConfigRsp(){}
    ~SetACLConfigRsp(){}
public:
    UINT16   SetXid(UINT16 id ){ return((DataResponse*)GetDataPtr())->Xid = id ;}
    UINT16   GetXid() const {return((DataResponse*)GetDataPtr())->Xid;}
    UINT16    SetResult(UINT16 rst ){ return((DataResponse*)GetDataPtr())->Result = rst ;}
    UINT16    GetResult() const {return((DataResponse*)GetDataPtr())->Result;}

protected:
    UINT32 GetDefaultDataLen() const{return sizeof(DataResponse);}
    UINT16 GetDefaultMsgId() const{return M_BTS_EMS_CFG_ACL_RSP;}
    };

#endif
