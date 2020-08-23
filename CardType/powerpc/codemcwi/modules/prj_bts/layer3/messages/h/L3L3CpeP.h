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

#ifndef _INC_L3L3CPEPROFUPDATEREQ
#define _INC_L3L3CPEPROFUPDATEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif
#include <fioLib.h>

class CL3CpeProfUpdateReq : public CMessage
{
public: 
    CL3CpeProfUpdateReq(CMessage &rMsg):CMessage(rMsg){}
    CL3CpeProfUpdateReq(){}
    ~CL3CpeProfUpdateReq(){}

    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    void   SetVersion(UINT16);
    void   SetLocAreaId(UINT32);
    void   SetBWReqInter(UINT16);
    void   SetSessRelThres(UINT16);
    void   SetProfileDataFlag(UINT16);
    UINT16 getProfileDataFlag();
	void   validation(UINT8);
    void   SetQosEntryNum(UINT16);
    bool   SetTosInfo(const SINT8*, UINT16);
    bool   SetFreqInfo(const SINT8*, UINT16);//include neighborBTS and Repeater;
////const  T_UTProfile& getUTProfile()const; 
    bool   setUTProfile(const T_UTProfile &, UINT8 = 0);     
#ifdef RCPE_SWITCH
	bool   setUTSDCfgRsv(UINT8);
#endif
    UINT32 length()const;
protected:
    UINT32 getQosEntryNumOffset()const; 
    UINT16 getQosEntryNum()const; 
    UINT32 getNeighborBtsNumOffset()const; 
    UINT16 getNeighborBtsNum()const; 
    UINT8  GetFixIpNum() const;

    UINT32 GetDefaultDataLen() const    {return MAX_MESSAGE_SIZE;}
    UINT16 GetDefaultMsgId() const      {return M_L3_CPE_PROFILE_UPDATE_REQ;}
private:
#pragma pack(1)
    struct T_L3CpeProfUpdateReq
    {
	    T_L3CpeProfUpdateReqBaseNoIE hdr; 
        T_UTProfile UTProfile;
        UINT16  QosEntryNum;
        T_CpeToSCfgEle *TosInfo;  
        UINT16  BtsNum;            // 0 -- 20 Neighbor BTS number
        UINT32 *BTSID;
		UINT16  RepeaterNum;
		UINT16 *repeaterFreq;
		UINT8   RFprofile[12];//RF profile;
	 T_UT_HOLD_BW  UTHoldBW;//wangwenhua add 20080916;
	 T_MEM_CFG memCfg;//mem info, if cpe, is 0 jy081217
    };
#pragma pack()
};


class CZmoduleRegsterResponse:public CMessage
{
public: 
    CZmoduleRegsterResponse(const CMessage &rMsg):CMessage(rMsg){}
    CZmoduleRegsterResponse(){}
    ~CZmoduleRegsterResponse(){}

    bool   CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;  

public:
    void setLoginFlag  (UINT8 flag);
    void setCidNum(UINT8 ucCidNum);
    void setZNum  (UINT8 uc);
    void setMsgInd(UINT8 uc);

private:
#pragma pack(1)
    struct T_ZloginResponse
    {
        UINT8  cidNum;
        UINT8  LoginFlag;
#ifdef MZ_2ND
        UINT8 ucZNum;
        UINT8 ucMsgInd;
#endif
    };
#pragma pack()
};
#if 0
class CUpdateUTBWInfo:public CL3CpeProfUpdateReq
{
public:
    CUpdateUTBWInfo(){}
    ~CUpdateUTBWInfo(){}
#if 0
    void setPrdRegTime(UINT16);
    UINT32 length()const    
        {
        //printf("->CUpdateUTBWInfo::length()");
        return CL3CpeProfUpdateReq::length()+2;
        }
#endif
protected:
    UINT16 GetDefaultMsgId() const {return M_L3_CPE_UPDATE_BWINFO;}
#if 0
private:
    UINT16 m_usPeriodicalRegisterTimerValue;    //hours.
#endif
};
#endif
#endif
