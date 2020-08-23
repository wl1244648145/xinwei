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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

//已根据最新接口文档更新
#ifndef _INC_L3OAMPROFUPDATEREQ
#define _INC_L3OAMPROFUPDATEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif


//UT profile update req（ems to bts）
class CCpeProfUpdateReq: public CMessage
{
public: 
    CCpeProfUpdateReq(CMessage &rMsg):CMessage(rMsg){}
    CCpeProfUpdateReq(){}
    bool CreateMessage(CComEntity&);
    ~CCpeProfUpdateReq(){}

    UINT32 GetDefaultDataLen() const;  
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT32 GetCPEID() const;
    void   SetCPEID(UINT32);
    
#if 0
    UINT8  GetAdminStatus()const{return getProfile()->UTProfileIEBase.AdminStatus;}
    void   SetAdminStatus(UINT8); 

    UINT8  GetLogStatus()const; 
    void   SetLogStatus(UINT8); 

    UINT16 GetDataCInter()const; 
    void   SetDataCInter(UINT16); 

    UINT8  GetMobility() const;
    void   SetMobility(UINT8);
    
    UINT8  GetDHCPRenew() const;
    void   SetDHCPRenew(UINT8);

    bool   GetUTSerDisIE(SINT8*, UINT16)const; 
    bool   SetUTSerDisIE(SINT8*, UINT16); 
	UINT8 GetMaxIpNum() const;
	void  SetMaxIpNum(UINT8 MaxIpNum);

    UINT8  GetFixIpNum() const;
    void   SetFixIpNum(UINT8);
    
    bool   GetCpeFixIpInfo(SINT8*, UINT16 Len) const;
    bool   SetCpeFixIpInfo(SINT8*, UINT16 Len);

////bool   GetUTProfIE(SINT8*, UINT16 Len)const; 
////bool   SetUTProfIE(SINT8*, UINT16); 
#endif
    const T_UTProfile* getProfile()const;
    const T_UT_HOLD_BW* getHoldBW()const;
////bool   GetRegData(SINT8*, UINT16 Len)const; 
////bool   SetRegData(SINT8*, UINT16); 

////UINT16 GetUTProfIESize();
private:
#pragma pack(1)
    struct T_ProfUpReq
    {        
        UINT16  TransId;
        UINT32  CPEID;
		T_UTProfile UTProfile;
              T_UT_HOLD_BW  UTHoldBW;
    };
#pragma pack()
};
#endif


