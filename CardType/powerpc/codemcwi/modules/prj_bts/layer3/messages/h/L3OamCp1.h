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

#ifndef _INC_L3OAMCPECOMMONREQ
#define _INC_L3OAMCPECOMMONREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif
#include "L3CpeMessageId.h"


class CCpeCommonReq : public CMessage
{
public: 
    CCpeCommonReq(CMessage &rMsg);
    CCpeCommonReq();
    bool CreateMessage(CComEntity&);
    virtual ~CCpeCommonReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT32 GetCPEID() const;
    void   SetCPEID(UINT32);
    
private:
#pragma pack(1)
    struct T_Req
    {
        UINT16  TransId;
        UINT16  Version; 
        UINT32  CPEID;
    };
#pragma pack()
};


class CCpeHWDescriptorReq : public CMessage
{
public: 
    CCpeHWDescriptorReq(CMessage &rMsg):CMessage(rMsg){}
    CCpeHWDescriptorReq(){}
    ~CCpeHWDescriptorReq(){}
    UINT16 SetTransactionId(UINT16 trans)	{return ((_tag_HWReq*)GetDataPtr())->transId = trans;}
    UINT32 setCPEID(UINT32 eid)				{return ((_tag_HWReq*)GetDataPtr())->eid = eid;}
    UINT16 setHWtype(UINT16 type)			{return ((_tag_HWReq*)GetDataPtr())->HWtype = type;}

protected:
    UINT32 GetDefaultDataLen() const		{return sizeof(_tag_HWReq);}
    UINT16 GetDefaultMsgId() const			{return M_BTS_CPE_HWDESC_REQ;}
private:
#pragma pack(1)
    struct _tag_HWReq
    {
        UINT16  transId;
        UINT16  version; 
        UINT32  eid;
		UINT16  HWtype;
    };
#pragma pack()
};
#endif
