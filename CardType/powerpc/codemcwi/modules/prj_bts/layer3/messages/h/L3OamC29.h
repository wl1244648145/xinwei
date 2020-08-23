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

// RF Reqeust（EMS）L2
#ifndef _INC_L3OAMCFGRFREQ
#define _INC_L3OAMCFGRFREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

#include <string.h>

class CCfgRfReq : public CMessage
{
public: 
    CCfgRfReq(CMessage &rMsg);
    CCfgRfReq();
    bool CreateMessage(CComEntity&);
    ~CCfgRfReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_RfCfgReq
    {
        UINT16 TransId;
        T_RfCfgEle Ele;
    };
#pragma pack()
};
class CCfgRRangingReq : public CMessage
{
public: 
    CCfgRRangingReq(CMessage &rMsg);
    CCfgRRangingReq();
    bool CreateMessage(CComEntity&);
    ~CCfgRRangingReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_RRangingReq
    {
        UINT16 TransId;
        T_RangingPara Ele;//传送时去掉最后的flag字节
    };
#pragma pack()
};


class CCfgVacPrefScgReq : public CMessage
{
public: 
    CCfgVacPrefScgReq(CMessage &rMsg):CMessage(rMsg){};
    CCfgVacPrefScgReq(){};
    bool CreateMessage(CComEntity& Entity)
	{
		CMessage :: CreateMessage(Entity);
		//SetMessageId(MSGID_L3_L2_VACPREFSCG_CFG);
		return true;
	};
    ~CCfgVacPrefScgReq(){};
    UINT32 GetDefaultDataLen() const{ return sizeof(UINT16) * 2; };  
    UINT16 SetTransactionId(UINT16 us){ ((T_VacPrefScgReq *)GetDataPtr())->TransId = us; return 0; };
    void SetValue(UINT16 us){ ((T_VacPrefScgReq *)GetDataPtr())->value = us; };
private:
    struct T_VacPrefScgReq
    {
        UINT16 TransId;
        UINT16 value;//传送时去掉最后的flag字节
    };
};
class CCfgSavePowerReq : public CMessage
{
private:
    struct T_SavePwrCfgReq
    {
        UINT16 TransId;
        UINT16 SavePwrFlag; 
        UINT16 TS1Channel; 
        UINT16 TS1User; 
        UINT16 TS2Channel; 
        UINT16 TS2User; 
    };
public: 
    CCfgSavePowerReq(CMessage &rMsg):CMessage(rMsg){};
    CCfgSavePowerReq(){};
    bool CreateMessage(CComEntity& Entity)
	{
		CMessage :: CreateMessage(Entity);
		//SetMessageId(MSGID_L3_L2_VACPREFSCG_CFG);
		return true;
	};
    ~CCfgSavePowerReq(){};
    UINT32 GetDefaultDataLen() const{ return sizeof(T_SavePwrCfgReq); };  
    UINT16 SetTransactionId(UINT16 us){ ((T_SavePwrCfgReq *)GetDataPtr())->TransId = us; return 0; };
    void SetValue(UINT8* uc)
	{ 
		memcpy( (UINT8*)GetDataPtr()+2, uc, sizeof(T_SavePwrCfgReq)-2 ); 
	};

};

class CCfgQam64Req : public CMessage
{
private:
    struct T_Qam64CfgReq
    {
        UINT16 TransId;
        UINT16 usValue; 

    };
public: 
    CCfgQam64Req(CMessage &rMsg):CMessage(rMsg){};
    CCfgQam64Req(){};
    bool CreateMessage(CComEntity& Entity)
	{
		CMessage :: CreateMessage(Entity);
		//SetMessageId(MSGID_L3_L2_VACPREFSCG_CFG);
		return true;
	};
    ~CCfgQam64Req(){};
    UINT32 GetDefaultDataLen() const{ return sizeof(T_Qam64CfgReq); };  
    UINT16 SetTransactionId(UINT16 us){ ((T_Qam64CfgReq *)GetDataPtr())->TransId = us; return 0; };
    void SetValue(UINT8* uc)
	{ 
		memcpy( (UINT8*)GetDataPtr()+2, uc, sizeof(T_Qam64CfgReq)-2 ); 
	};

};

class CCfgPayloadReq : public CMessage
{
private:
    struct T_PayloadCfgReq
    {
        UINT16 TransId;
        T_PayloadBalanceCfg tEle; 
    };
public: 
    CCfgPayloadReq(CMessage &rMsg):CMessage(rMsg){};
    CCfgPayloadReq(){};
    bool CreateMessage(CComEntity& Entity)
	{
		CMessage :: CreateMessage(Entity);
		//SetMessageId(MSGID_L3_L2_VACPREFSCG_CFG);
		return true;
	};
    ~CCfgPayloadReq(){};
#ifdef PAYLOAD_BALANCE_2ND
    UINT32 GetDefaultDataLen() const{ return sizeof(T_PayloadCfgReq)+2; };  
#else
    UINT32 GetDefaultDataLen() const{ return sizeof(T_PayloadCfgReq); };  
#endif
    UINT16 SetTransactionId(UINT16 us){ ((T_PayloadCfgReq *)GetDataPtr())->TransId = us; return 0; };
    void SetValue(UINT8* uc)
	{ 
		//memcpy( (UINT8*)GetDataPtr()+2, uc, sizeof(T_PayloadCfgReq)-2 ); 
     #ifdef PAYLOAD_BALANCE_2ND
		memcpy( (UINT8*)GetDataPtr()+2, uc, sizeof(T_PayloadCfgReq) ); 
     #else
		memcpy( (UINT8*)GetDataPtr()+2, uc, sizeof(T_PayloadCfgReq)-2 ); 
     #endif
	};

};

class CCfgClusterNumLmtReq : public CMessage
{
public: 
    CCfgClusterNumLmtReq(CMessage &rMsg);
    CCfgClusterNumLmtReq();
    bool CreateMessage(CComEntity&);
    ~CCfgClusterNumLmtReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_CLUSTERNUMLMTMSG
    {
        UINT16 TransId;
        T_CLUSTERNUMLMT Ele;//传送时去掉最后的flag字节
    };
#pragma pack()
};



#endif
