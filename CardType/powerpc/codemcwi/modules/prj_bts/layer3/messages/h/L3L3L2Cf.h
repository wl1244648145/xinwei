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

#ifndef _INC_L3L3L2CFGAIRLINKREQ
#define _INC_L3L3L2CFGAIRLINKREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

#include <string>
class CL3L2CfgAirLinkReq : public CMessage
{
public: 
    CL3L2CfgAirLinkReq(CMessage &rMsg);
    CL3L2CfgAirLinkReq();
    bool CreateMessage(CComEntity&);
    ~CL3L2CfgAirLinkReq();
    UINT32 GetDefaultDataLen() const;  
public: 
    UINT16 GetTransactionId()const;
    UINT16 SetTransactionId(UINT16);

    UINT32 GeBtsID()const;
    void   SetBtsID(UINT32);

    UINT16 GetNetworkID()const;
    void   SetNetworkID(UINT16);

    UINT16 GetResetCnt()const;
    void   SetResetCnt(UINT16);
    
    SINT8* GetEle()const;
    void   SetEle(SINT8 *);
private:
#pragma pack(1)
    struct T_AirLinkCfgReq
    {
        UINT16 TransId;
        UINT32 BtsID;
        UINT16 NetworkID;
        UINT16 ResetCnt;
        T_AirLinkCfgEle Ele;
    };
#pragma pack()
};

class CL3L2CfgN1ParameterReq : public CMessage
{
public: 
    CL3L2CfgN1ParameterReq(CMessage &rMsg):CMessage(rMsg){}
    CL3L2CfgN1ParameterReq(){}
    ~CL3L2CfgN1ParameterReq(){}

    UINT16 GetTransactionId()const {return ((T_N1ParameterReq *)GetDataPtr())->TransId;}
    UINT16 SetTransactionId(UINT16 transID) {((T_N1ParameterReq *)GetDataPtr())->TransId = transID;}

    void   SetEle(SINT8 *ele)
	{
	    memcpy(&(((T_N1ParameterReq *)GetDataPtr())->N1Parameter), ele, sizeof(T_N_Parameter));
	}

protected:
    UINT32 GetDefaultDataLen() const{return sizeof(T_N1ParameterReq);}
    UINT16 GetDefaultMsgId()   const{return M_L3_L2_N1_PARAMETER_REQ;}

private:
#pragma pack(1)
    struct T_N1ParameterReq
    {
        UINT16 TransId;
        T_N_Parameter N1Parameter;
    };
#pragma pack()
};
#endif


