#ifndef _INC_L3DMCONFIG
#define _INC_L3DMCONFIG

#include "Message.h"
#include "L3DataDmMessage.h"
#include "L3OAMMessageId.h"


//CPE Data Service Configuration 
class CCPEDataConfigReq: public CMessage
    {
public: 
    CCPEDataConfigReq(CMessage &rMsg):CMessage( rMsg ){}
    CCPEDataConfigReq(){}
    ~CCPEDataConfigReq(){}

public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT8  GetMobility() const;
    void   SetMobility(UINT8);

    UINT8  GetDHCPRenew() const;
    void   SetDHCPRenew(UINT8);

    UINT16 GetVlanId() const;

    UINT8  GetMaxIpNum() const;
    void   SetMaxIpNum(UINT8);

    UINT8  GetFixIpNum() const;
    void   SetFixIpNum(UINT8);

    bool   GetCpeFixIpInfo(SINT8*, UINT16 Len) const;
    bool   SetCpeFixIpInfo(SINT8*, UINT16 Len);

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
    };

// DM Configuration Request 
class CDMConfigReq: public CMessage
    {
public: 
    CDMConfigReq(CMessage &rMsg):CMessage( rMsg ){}
    CDMConfigReq(){}
    ~CDMConfigReq(){}

public:
    UINT16  GetXid() const;
    UINT16  SetXid(UINT16) ;
    UINT8   GetMobility()const;
    UINT8    SetMobility(UINT8);  

    UINT8   GetAccessCtrl()const;
    UINT8    SetAccessCtrl(UINT8);  
protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
    };

//CPE Data Service Configuration 
class CDMDataConfigRsp: public CMessage
    {
public: 
    CDMDataConfigRsp(CMessage &rMsg):CMessage( rMsg ){}
    CDMDataConfigRsp(){}
    ~CDMDataConfigRsp(){}

public:
    UINT16   SetXid(UINT16 id ){ return((DataResponse*)GetDataPtr())->Xid = id ;}
    UINT16    GetXid() const {return((DataResponse*)GetDataPtr())->Xid;}
    UINT16    SetResult(UINT16 rst ){ return((DataResponse*)GetDataPtr())->Result = rst ;}
    UINT16    GetResult() const {return((DataResponse*)GetDataPtr())->Result;}

protected:
    UINT32 GetDefaultDataLen() const{return sizeof(DataResponse);}
    UINT16 GetDefaultMsgId() const{return M_DM_CFG_DATA_SERVICE_CFG_RSP;}
    };

#endif
