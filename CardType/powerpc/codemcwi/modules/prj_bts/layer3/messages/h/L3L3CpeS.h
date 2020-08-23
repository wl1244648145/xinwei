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
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/


#ifndef _INC_L3L3CPESWDLPACKREQ
#define _INC_L3L3CPESWDLPACKREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMFILECOMMON
#include "L3OamFileCommon.h"
#endif
#if 0
class CL3CpeSWBCDLPackReq : public CMessage
{
public: 
    CL3CpeSWBCDLPackReq(CMessage &rMsg);
    CL3CpeSWBCDLPackReq();
    bool CreateMessage(CComEntity& Entity);
    ~CL3CpeSWBCDLPackReq();
    UINT32  GetDefaultDataLen() const;
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetDLReqSeqNum()const;
    void   SetDLReqSeqNum(UINT16);

    UINT16 GetSWPackSeqNum()const;
    void   SetSWPackSeqNum(UINT16);

    SINT8*  GetSWPackData()const;
    void   SetSWPackData(SINT8*, UINT16);
private:
#pragma pack(1)
    struct T_ReqBase
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 DLReqSeqNum;
        UINT16 SWPackSeqNum;
    };

    struct T_Req
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 DLReqSeqNum;
        UINT16 SWPackSeqNum;
        SINT8  SWPackData[MAX_SWPACK_BC_SIZE];
    };
#pragma pack()
};
#endif //0
class CL3CpeSWUCDLPackReq : public CMessage
{
public: 
    CL3CpeSWUCDLPackReq(CMessage &rMsg);
    CL3CpeSWUCDLPackReq();
    bool CreateMessage(CComEntity& Entity);
    ~CL3CpeSWUCDLPackReq();
    UINT32  GetDefaultDataLen() const;
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT16 GetDLReqSeqNum()const;
    void   SetDLReqSeqNum(UINT16);

    UINT16 GetSWPackSeqNum()const;
    void   SetSWPackSeqNum(UINT16);

    SINT8*  GetSWPackData()const;
    void   SetSWPackData(SINT8*, UINT16);
private:
#pragma pack(1)
    struct T_ReqBase
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 DLReqSeqNum;
        UINT16 SWPackSeqNum;
    };

    struct T_Req
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 DLReqSeqNum;
        UINT16 SWPackSeqNum;
        SINT8  SWPackData[MAX_SWPACK_UC_SIZE];
    };
#pragma pack()
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////             For Z Software Update           //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


class CL3ZSWUCDLPackReq : public CMessage
{
public: 
    CL3ZSWUCDLPackReq(CMessage &rMsg);
    CL3ZSWUCDLPackReq();
    bool CreateMessage(CComEntity& Entity);
    ~CL3ZSWUCDLPackReq();
    UINT32  GetDefaultDataLen() const;
    UINT16  SetTransactionId(UINT16);
    void   SetDLReqSeqNum(UINT16);
    void   SetSWPackSeqNum(UINT16);
    void   SetSWPackData(SINT8*, UINT16);
    void  SetPID(UINT32 E);

private:
#pragma pack(1)
    struct T_ZPACK_Base
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 DLReqSeqNum;
        UINT16 SWPackSeqNum;
        UINT32 ulPID;
    };
    struct T_ZPACK_Req
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 DLReqSeqNum;
        UINT16 SWPackSeqNum;
        UINT32 ulPID;
        SINT8  SWPackData[MAX_SWPACK_UC_SIZE];
    };
#pragma pack()
};

#endif
