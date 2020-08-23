/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMCFGACLREQ
#define _INC_L3OAMCFGACLREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgACLReq : public CMessage
{
public: 
    CCfgACLReq(CMessage &rMsg):CMessage(rMsg){}
    CCfgACLReq(){}
    bool CreateMessage(CComEntity&);
    ~CCfgACLReq(){}
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_ACLCfgReq
    {
        UINT16 TransId;
        T_ACLCfgEle Ele;
    };
#pragma pack()
};

class CCfgVlanGroupReq : public CMessage
{
public: 
    CCfgVlanGroupReq(CMessage &rMsg):CMessage(rMsg){}
    CCfgVlanGroupReq(){}
    ~CCfgVlanGroupReq(){}

    bool CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    UINT8* GetEle() const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_VlanGroupCfgReq
    {
        UINT16 TransId;
        T_VlanGroupCfgEle Ele;
    };
#pragma pack()
};
class CCfgClusterParaReq : public CMessage
{
public: 
    CCfgClusterParaReq(CMessage &rMsg):CMessage(rMsg){}
    CCfgClusterParaReq(){}
    bool CreateMessage(CComEntity&);
    ~CCfgClusterParaReq(){}
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_ClusterPara
    {
        UINT16 TransId;
        T_CLUSTER_PARA Ele;
    };
#pragma pack()
};
#endif
