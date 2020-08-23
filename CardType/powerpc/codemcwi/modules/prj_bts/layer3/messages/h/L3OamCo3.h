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


#ifndef _INC_L3OAMCOMMONRSPFROMCPE
#define _INC_L3OAMCOMMONRSPFROMCPE

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3OamCommonRspFromCpe : public CMessage
{
public: 
    CL3OamCommonRspFromCpe(CMessage &rMsg);
    CL3OamCommonRspFromCpe();
    bool CreateMessage(CComEntity& Entity);
    virtual ~CL3OamCommonRspFromCpe();
    UINT32  GetDefaultDataLen() const;
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetResult() const;
    void   SetResult (UINT16 );
private:
#pragma pack(1)
    struct T_Rsp
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 Result;
    };
#pragma pack()
};
#if 0
class CL3OamCommonRspFromCpeZ : public CMessage
{
public: 
    CL3OamCommonRspFromCpeZ(CMessage &rMsg);
    CL3OamCommonRspFromCpeZ();
    bool CreateMessage(CComEntity& Entity);
    virtual ~CL3OamCommonRspFromCpeZ();
    UINT32  GetDefaultDataLen() const;
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetResult() const;
    UINT16 GetZPid() const;
    void   SetResult (UINT16 );
private:
#pragma pack(1)
    struct T_ZRsp
    {
        UINT16 TransId;
        UINT16 Version;   
        UINT32 ZPid;
        UINT16 Result;
    };
#pragma pack()
};
#endif
#if 0
class CCpeHWDescriptorRspFromCPE:public CL3OamCommonRspFromCpe
{
public:
    CCpeHWDescriptorRspFromCPE(CMessage &rMsg):CL3OamCommonRspFromCpe(rMsg){}
    CCpeHWDescriptorRspFromCPE(){}
    ~CCpeHWDescriptorRspFromCPE(){}

////bool CreateMessage(CComEntity& Entity);
    UINT32 getEID()       { return ((T_CpeHWtypeRspFromCPE *)GetDataPtr())->eid; }
    UINT16 getHWtype()    { return ((T_CpeHWtypeRspFromCPE *)GetDataPtr())->HWtype; }
	SINT8* getDescriptor(){ return ((T_CpeHWtypeRspFromCPE *)GetDataPtr())->descriptor; }

protected:
    UINT32  GetDefaultDataLen() const {return sizeof(T_CpeHWtypeRspFromCPE);}
    UINT16  GetDefaultMsgId() const   {return ;}

private:
#pragma pack(1)
    struct T_CpeHWtypeRspFromCPE:T_Rsp
    {
        UINT32 eid;
		UINT16 HWtype;
		UINT8  descriptor[M_HW_TYPE_LEN];
    };
#pragma pack()
};
#endif
#endif
