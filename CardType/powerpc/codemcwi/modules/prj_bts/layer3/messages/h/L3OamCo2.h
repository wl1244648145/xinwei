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


#ifndef _INC_L3OAMCOMMONRSP
#define _INC_L3OAMCOMMONRSP

#include <string.h>
#ifndef _INC_MESSAGE
#include "Message.h"
#endif
#include "l3EmsMessageId.h"
class CL3OamCommonRsp : public CMessage
{
public: 
    CL3OamCommonRsp(CMessage &rMsg):CMessage(rMsg){}
    CL3OamCommonRsp(){}
    virtual ~CL3OamCommonRsp(){}

    bool CreateMessage(CComEntity& Entity);
    bool CreateMessage(CComEntity &ComEntity, UINT32 Len);
    UINT32  GetDefaultDataLen() const;

    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    UINT16 GetResult() const;
    void   SetResult (UINT16 );

//private:
#pragma pack(1)
    struct T_Rsp
    {
        UINT16 TransId;
        UINT16 Result;
    };
#pragma pack()
};

class CL3OamProbeCPERsp:public CL3OamCommonRsp
{
public:
    CL3OamProbeCPERsp(CMessage &rMsg):CL3OamCommonRsp(rMsg){}
    CL3OamProbeCPERsp(){}
    ~CL3OamProbeCPERsp(){}

    bool CreateMessage(CComEntity& Entity);
    UINT32  GetDefaultDataLen() const;
    void  setRspEID(UINT32 eid);
//private:
#pragma pack(1)
    struct T_ProbeCPE:T_Rsp
    {
        UINT32 eid;
    };
#pragma pack()
};


#define M_HW_TYPE_LEN (16)
class CCpeHWDescriptorRsp:public CL3OamCommonRsp
{
public:
    CCpeHWDescriptorRsp(CMessage &rMsg):CL3OamCommonRsp(rMsg){}
    CCpeHWDescriptorRsp(){}
    ~CCpeHWDescriptorRsp(){}
#if 0
////bool CreateMessage(CComEntity& Entity);
    void setRspEID(UINT32 eid) { ((T_CpeHWtypeRsp *)GetDataPtr())->eid = eid; }
    void setHWtype(UINT16 type){ ((T_CpeHWtypeRsp *)GetDataPtr())->HWtype = type; }
	void setDescriptor(SINT8 *ptr)
		{
		if (NULL != ptr)
			memcpy(((T_CpeHWtypeRsp *)GetDataPtr())->descriptor, ptr, M_HW_TYPE_LEN);
		else
			memset(((T_CpeHWtypeRsp *)GetDataPtr())->descriptor, 0, M_HW_TYPE_LEN);
		}
#endif
protected:
    UINT32  GetDefaultDataLen() const {return sizeof(T_CpeHWtypeRsp);}
    UINT16  GetDefaultMsgId() const   {return M_BTS_EMS_HWDESC_RESP;}

private:
#pragma pack(1)
    struct T_CpeHWtypeRsp:T_Rsp
    {
        UINT32 eid;
		UINT16 HWtype;
		UINT8  descriptor[M_HW_TYPE_LEN];
    };
#pragma pack()
};
//wangwenhua add 20081119

class CL3OamQueryCPENetWorkRsp:public CL3OamCommonRsp
{
public:
    CL3OamQueryCPENetWorkRsp(CMessage &rMsg):CL3OamCommonRsp(rMsg){}
    CL3OamQueryCPENetWorkRsp(){}
    ~CL3OamQueryCPENetWorkRsp(){}

    bool CreateMessage(CComEntity& Entity);
    UINT32  GetDefaultDataLen() const;
    void  setRspEID(UINT32 eid);
    void setRspType(UINT16 type);
private:
#pragma pack(1)
    struct T_QueryCPENetWork:T_Rsp
    {
        UINT32 eid;
	UINT16  type;
    };
#pragma pack()
};
//wangwenhua add end 20081119
#endif
