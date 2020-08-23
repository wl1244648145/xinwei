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

#ifndef _INC_L3OAMCFGBTSGENDATAREQ
#define _INC_L3OAMCFGBTSGENDATAREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgBtsGenDataReq : public CMessage
{
public: 
    CCfgBtsGenDataReq(CMessage &rMsg);
    CCfgBtsGenDataReq();
    bool CreateMessage(CComEntity&);
    ~CCfgBtsGenDataReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT32 GetBtsIPAddr() const;
    void   SetBtsIPAddr(UINT32);
    
    UINT32 GetDefGateway() const;
    void   SetDefGateway(UINT32);
    
    UINT32 GetSubnetMask() const;
    void   SetSubnetMask(UINT32);
    
    UINT16 GetSAGID() const;
    void   SetSAGID(UINT16);
    
    UINT32 GetSAGVoiceIP() const;
    void   SetSAGVoiceIP(UINT32);

    UINT32 GetSAGSignalIP() const;
    void   SetSAGSignalIP(UINT32);

    UINT32 GetLocAreaID() const;
    void   SetLocAreaID(UINT32);
    
    UINT32 GetEmsIPAddr() const;
    void   SetEmsIPAddr(UINT32);
    
    UINT16 GetNetworkID() const;
    void   SetNetworkID(UINT16);
    
    UINT16  GetBtsBootSource() const;
    void   SetBtsBootSource(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
	#ifdef NUCLEAR_CODE
	UINT8 GetLimited()
	{
	    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.bLimited;
	};
	#endif
private:
#pragma pack(1)
    struct T_BtsGDataCfgReq
    {   //data ok 2005-09-07
        UINT16 TransId;
        T_BtsGDataCfgEle Ele;
    };
#pragma pack()
};

#endif
