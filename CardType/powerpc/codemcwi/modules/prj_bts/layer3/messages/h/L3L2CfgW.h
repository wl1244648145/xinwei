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
 *   10/19/07    Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef _INC_L3OAM_CFG_WANIF_CPE_REQ
#define _INC_L3OAM_CFG_WANIF_CPE_REQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

class CL3OamCfgWanIfCpeEidReq : public CMessage
{
public: 
    CL3OamCfgWanIfCpeEidReq(CMessage &);
    CL3OamCfgWanIfCpeEidReq(CComMessage *pMsg):CMessage(pMsg){};
    CL3OamCfgWanIfCpeEidReq();
    bool CreateMessage(CComEntity& Entity);
    ~CL3OamCfgWanIfCpeEidReq();
    UINT32  GetDefaultDataLen() const;
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    UINT32 GetWanCpeEid() const;
    UINT32 SetWanCpeEid(UINT32);
private:
#pragma pack(1)
    struct T_Req
    {
        UINT16 TransId;
        UINT32 WanIfCpeEID;
    };
#pragma pack()
};
#endif //_INC_L3OAM_CFG_WANIF_CPE_REQ
