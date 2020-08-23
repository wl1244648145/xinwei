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
 *   07/08/2008   sunshanggu
 *---------------------------------------------------------------------------*/


#ifndef _INC_L3L3CPEETHCFGRSP
#define _INC_L3L3CPEETHCFGRSP

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#define MAX_L3_CPE_ETH_CFG_RSP_MSG_LEN 12

class CL3L3CpeEthCfgRsp : public CMessage
{
public: 
    CL3L3CpeEthCfgRsp(CMessage &rMsg);
    CL3L3CpeEthCfgRsp();
    bool CreateMessage(CComEntity& Entity);
    ~CL3L3CpeEthCfgRsp();
    UINT32  GetDefaultDataLen() const;
    
    //UINT16 GetTransactionId() const;
    //UINT16 SetTransactionId(UINT16);
    
    UINT16 GetCmdType() const;
	UINT8 * GetCmdContent() const;
private:
#pragma pack(1)
    struct T_Rsp
    {
        UINT16 CmdType;
        UINT8 Content[MAX_L3_CPE_ETH_CFG_RSP_MSG_LEN];        
    };
#pragma pack()
};
#endif

