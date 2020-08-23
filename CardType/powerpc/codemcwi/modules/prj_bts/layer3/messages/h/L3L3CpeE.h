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
 *   08/07/2008   Sunshangu For CPE net interface configuration
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3L3CPEETHCFGREQ
#define _INC_L3L3CPEETHCFGREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif

#define MAX_L3_CPE_ETH_CFG_REQ_MSG_LEN 4

class CL3CpeEthCfgReq : public CMessage
{
public: 
    CL3CpeEthCfgReq(CMessage &rMsg):CMessage(rMsg){}
    CL3CpeEthCfgReq(){}
    ~CL3CpeEthCfgReq(){}
    bool CreateMessage(CComEntity&, UINT32 uDataSize);
    UINT32 GetDefaultDataLen() const;  
public: 
    //UINT16 GetTransactionId() const;
   // UINT16 SetTransactionId(UINT16);
    UINT16   SetCmdType(UINT16);
	UINT16   GetCmdType() const;
	UINT8 * GetCmdContent() const;

private:
#pragma pack(1)
  typedef  struct T_L3CpeEthCfgReq
    {
        UINT16 CmdType;         
        UINT8 Content[MAX_L3_CPE_ETH_CFG_REQ_MSG_LEN];
    }L3CPEETHCFGREQ;
#pragma pack()
};
#define MAX_REQ_MSG_LEN   100
class CL3CpeCommMsgReq : public CMessage
{
public: 
    CL3CpeCommMsgReq(CMessage &rMsg):CMessage(rMsg){}
    CL3CpeCommMsgReq(){}
    ~CL3CpeCommMsgReq(){}
    bool CreateMessage(CComEntity&, UINT32 uDataSize);
    UINT32 GetDefaultDataLen() const;  
public: 

	UINT16   GetCommMsgType() const;
	UINT16   GetCommMsgLen() const;
	UINT8 * GetCommContent() const;

private:
#pragma pack(1)
  typedef  struct T_L3CpeCommMsgReq
    {
        UINT16 Len;  
	 UINT16 type;/*0-str,1-binary***/
        UINT8 Content[MAX_REQ_MSG_LEN];
    }L3CpeCommMsgReq;
#pragma pack()
};

class CL3CpeCommCfgReq : public CMessage
{
public: 
    CL3CpeCommCfgReq(CMessage &rMsg):CMessage(rMsg){}
    CL3CpeCommCfgReq(){}
    ~CL3CpeCommCfgReq(){}
    bool CreateMessage(CComEntity&, UINT32 uDataSize);
    UINT32 GetDefaultDataLen() const;  
public: 

	void    SetCommMsgType(UINT16 type) ;


private:
#pragma pack(1)
 typedef  struct T_L3CpeCommCfgReq
{
     
	 UINT16 type;/**0-close,1-open**/
       
}L3CpeCommCfgReq;
#pragma pack()
};
#endif

