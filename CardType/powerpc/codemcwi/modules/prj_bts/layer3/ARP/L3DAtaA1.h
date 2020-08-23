/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataARPConfig.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   11/23/05    yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __ARP_CONFIG_H__
#define __ARP_CONFIG_H__

#include "Message.h"
#include "L3oamMessageid.h"


class ARPConfigReq:public CMessage {
public: 
    ARPConfigReq(CMessage &rMsg):CMessage( rMsg ){}
    ARPConfigReq(){}
    ~ARPConfigReq(){}

public:

    UINT16   GetXid() const {return((stARPconfigReq*)GetDataPtr())->Xid;}
    UINT8    GetEgressProxEn() const {return((stARPconfigReq*)GetDataPtr())->EgressProxEn;}
    UINT8    GetIngressProxEn() const {return((stARPconfigReq*)GetDataPtr())->IngressProxEn;}

private:
#pragma pack(1)
    struct stARPconfigReq {
        UINT16 Xid;
        UINT8  IngressProxEn;
        UINT8  EgressProxEn;
    };
#pragma pack()

};
//ARP  Configuration 
class ARPConfigRsp: public CMessage {
public: 
    ARPConfigRsp(CMessage &rMsg):CMessage( rMsg ){}
    ARPConfigRsp(){}
    ~ARPConfigRsp(){}

public:
    UINT16   SetXid(UINT16 id ){ return((ArpConfigResp*)GetDataPtr())->Xid = id ;}
    UINT16    GetXid() const {return((ArpConfigResp*)GetDataPtr())->Xid;}
    UINT16    SetResult(bool rst ){ return((ArpConfigResp*)GetDataPtr())->Result = (UINT16)(rst) ;}
    UINT16    GetResult() const {return((ArpConfigResp*)GetDataPtr())->Result;}
private:
#pragma pack(1)
    struct ArpConfigResp {
        UINT16   Xid;
        UINT16   Result;
    };
#pragma pack()
protected:
    UINT32 GetDefaultDataLen() const{return sizeof(ArpConfigResp);}
    UINT16 GetDefaultMsgId() const{return M_ARP_CFG_DATA_SERVICE_CFG_RSP;}
};


#endif
