#ifndef _INC_L3DTSYNCMSG
#define _INC_L3DTSYNCMSG

//socket:
#ifdef __WIN32_SIM__
    #include <winsock2.h>
#else   //VxWorks:
    #include "vxWorks.h" 
    #include "sockLib.h" 
    #include "inetLib.h" 
    #include "stdioLib.h" 
    #include "strLib.h" 
    #include "hostLib.h" 
    #include "ioLib.h" 
#endif

#include "Message.h"
#include "L3dataMsgId.h"
#include "L3dataCommon.h"
#include "L3dataTypes.h"

#include "L3DataDmMessage.h"
#include "L3DataDmComm.h"


class CUTBtsResponse:public CMessage
    {
public: 
    CUTBtsResponse(CMessage &rMsg):CMessage( rMsg ){}
    CUTBtsResponse(){}
    ~CUTBtsResponse(){}

public:
    UINT16   SetXid(UINT16 id ){ return((UtBtsResponse*)GetDataPtr())->Xid = htons(id) ;}
    UINT16    GetXid() const {return  ntohs( ((UtBtsResponse*)GetDataPtr())->Xid );}
    UINT16   SetVersion(UINT16 ver){ return((UtBtsResponse*)GetDataPtr())->Version =htons( ver );}
    UINT16    GetVersion() const {return ntohs( ((UtBtsResponse*)GetDataPtr())->Version );}

    UINT16    SetResult(UINT16 rst ){ return((UtBtsResponse*)GetDataPtr())->Result = htons(rst) ;}
    UINT16    GetResult() const {return  ntohs( ((UtBtsResponse*)GetDataPtr())->Result );}
protected:
    UINT32 GetDefaultDataLen() const{return sizeof(UtBtsResponse);}
    };

class CUTACLUpdtRsp:public CUTBtsResponse
    {
public: 
    CUTACLUpdtRsp(CMessage &rMsg):CUTBtsResponse( rMsg ){}
    CUTACLUpdtRsp(){}
    ~CUTACLUpdtRsp(){}
    };

// Dm Update ACL Request
class CDtACLUpdtReq: public CMessage
    {
public: 
    CDtACLUpdtReq(CMessage &rMsg):CMessage( rMsg ){}
    CDtACLUpdtReq(){}
    ~CDtACLUpdtReq(){}

public:
    UINT16   SetACLXid(UINT16);
    UINT16   SetACLVersion(UINT16);
    UINT32   SetACLNum(UINT32);
    void   SetValue(const PrtclFltrEntry*,const UINT32);
    void   SetPayLoadLenth(UINT32 len) {SetDataLength(len);}
protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
    };



// Dm DataService Response 
class CDtDataServsRsp : public CMessage
    {
public: 
    CDtDataServsRsp(CMessage &rMsg):CMessage( rMsg ){}
    CDtDataServsRsp(){}
    ~CDtDataServsRsp(){}

public:
    UINT8   SetResult(bool);  
    UINT8   SetWorkMode(UINT8);  
    UINT16  SetVlanID(UINT16);  //UT profile中携带的group id对应于注册bts上的vlan id;

protected:
    UINT32  GetDefaultDataLen() const;
    UINT16  GetDefaultMsgId() const;

    };

/*****************************************
 *CDataServiceReq类,只用作接收消息处理，故构造消息部分不需实现
 *****************************************/
class CDataServiceReq:public CMessage
    {
public:
    CDataServiceReq(){}
    CDataServiceReq(const CMessage& Msg):CMessage(Msg){}
    ~CDataServiceReq(){}

    UINT32 GetTimeStamp() const{return  ntohl(((DataServsReq*)GetDataPtr())->TimeStamp) ;}
    UINT16 GetVersion() const{return  ntohs(((DataServsReq*)GetDataPtr())->Version) ;}
    UINT32  GetIpCount() const{return  ntohl(((DataServsReq*)GetDataPtr())->IpCount) ;}

    const UINT8* GetDAIB() const
    { 
        return(UINT8*)( ((DataServsReq*)GetDataPtr())->IplstTLV );
    }

    };

//只做接收回应消息使用
class CDtSyncIpListRsp:public CUTBtsResponse
    {
public:
    CDtSyncIpListRsp(CMessage &rMsg):CUTBtsResponse( rMsg ){}
    CDtSyncIpListRsp(){}
    ~CDtSyncIpListRsp(){}

protected:
    UINT16 GetDefaultMsgId() const{return MSGID_UT_BTS_DAIBUPDATE_RESP;}
    };

// BTS Synchronzise Iplist Request 
class CDtSyncIplistReq: public CMessage
    {
public: 
    CDtSyncIplistReq(CMessage &rMsg):CMessage( rMsg ){}
    CDtSyncIplistReq(){}
    ~CDtSyncIplistReq(){}

public:
    void    SetXid(UINT16);  
    void    SetTimeStamp(UINT32);
    void    SetVersion(UINT16);
    void    SetIpCount(UINT32);
    void    Set_DAIBTLV(const UTILEntry*,UINT32);
    void    SetPayLoadLenth(UINT32 len){SetDataLength(len);}

protected:
    UINT32  GetDefaultDataLen() const;
    UINT16  GetDefaultMsgId() const;
    };

// BTS SynchronizeAddrTb Request
class CDtSyncAddrFBReq: public CMessage
    {
public: 
    CDtSyncAddrFBReq(CMessage &rMsg):CMessage( rMsg ){}
    CDtSyncAddrFBReq(){}
    ~CDtSyncAddrFBReq(){}

public:
    UINT16   SetAddrXid(UINT16);  
    UINT16   SetAddrVersion(UINT16);  

    UINT32   SetAddrCount(UINT32);  
    void   SetAddrFB(const AddressFltrTb*,const UINT32);
    void   SetPayLoadLenth(UINT32 len){SetDataLength(len);}

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
    };
#endif

