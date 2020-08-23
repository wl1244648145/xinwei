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
 *   10/11/2007   肖卫方       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef __h_L3OamAuth
#define __h_L3OamAuth

#include "Message.h"
#include "L3CPEMessageId.h"
#include "L3VoiceMsgId.h"
#include "L3OAMcommon.h"
#include "L3OAMcpeCommon.h"

#pragma pack(1)
struct T_UTAccessReq
{
    UINT32 ulUID;
    UINT32 ulPID;
   UINT32 BTSID;//wangwenhua add 20080716
    UINT8  ucRegType;
    UINT8  ucVersion;
#ifdef MZ_2ND
    UINT32 ulMZPid;
#endif
};
#pragma pack()

class CUTAccessReq:public CMessage
{
public: 
    CUTAccessReq(CMessage &msg):CMessage(msg){}
    //CUTAccessReq(){}
    ~CUTAccessReq(){}

protected:
    //UINT32 GetDefaultDataLen() const    { return sizeof( T_Req ); }
    //UINT16 GetDefaultMsgId() const      { return M_CPE_L3_ACCESS_REQ;}
public: 
    //UINT16 getTransID() const                { return ((T_Req*)GetDataPtr())->transID; }
    const T_UTAccessReq* getAccessReq() const{ return (T_UTAccessReq*)GetDataPtr(); }
    //void convert();/*转化成AuthenticationInfo_Req*/
};


/********************************
AuthenticationInfo_Req:McBTS->SAG
*********************************/
class CsabisAuthenticationInfoReq:public CMessage
{
public: 
    //CsabisAuthenticationInfoReq(CMessage &msg):CMessage(msg){}
    CsabisAuthenticationInfoReq(){}
    ~CsabisAuthenticationInfoReq(){}

protected:
    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisAuthenticationInfoReq );}
    UINT16 GetDefaultMsgId()   const {return MSGID_UM_VCR;}

public:
    void setInfo(UINT32, UINT32, UINT32 ulMZPID=0 );

private:
#pragma pack(1)
    struct T_sabisAuthenticationInfoReq
    {
        UINT32 ulSagID;
        UINT16 usBtsID;
        UINT8  ucEvtGrpID;
        UINT16 usEvtID;
        UINT16 usLength;
        UINT32 ulUID;
        UINT32 ulPID;
#ifdef MZ_2ND
        UINT32 ulMZPid;
#endif
////////UINT32 ulCID;
////////UINT8  ucLAI[M_LAI_LENGTH];
    };
#pragma pack()
};


/*
================================
Authentication CMD:   SAG->McBTS
================================
*/
#pragma pack(1)
struct T_sabisAuthenticationCMD
{
    UINT32 ulSagID;
    UINT16 usBtsID;
    UINT8  ucEvtGrpID;
    UINT16 usEvtID;
    UINT16 usLength;
    UINT32 ulUID;
    UINT32 ulPID;
    UINT8  rand[16];
#ifdef MZ_2ND
    UINT32 ulMZPid;
#endif
};
#pragma pack()
class CsabisAuthenticationCMD:public CMessage
{
public: 
    CsabisAuthenticationCMD(CMessage &msg):CMessage(msg){}
    //CSabisAuthenticationCMD(){}
    ~CsabisAuthenticationCMD(){}

//protected:
//    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisAuthenticationCMD );}
//    UINT16 GetDefaultMsgId()   const {return MSGID_OAM_VCR_AUTHINFO_REQ;}

public:
    const T_sabisAuthenticationCMD * const getInfo() const{return (const T_sabisAuthenticationCMD * const)GetDataPtr();}
};


/*
********************************
Authentication CMD:   McBTS->UT
********************************
*/
class CUTAuthenticationCMD:public CMessage
{
public: 
    //CUTAuthenticationCMD(CMessage &msg):CMessage(msg){}
    CUTAuthenticationCMD(){}
    ~CUTAuthenticationCMD(){}

protected:
    UINT32 GetDefaultDataLen() const {return sizeof( T_UTAuthenticationCMD );}
    UINT16 GetDefaultMsgId()   const {return M_L3_CPE_AUTH_CMD;}

public:
    void setInfo(UINT32, UINT32, const UINT8*);

private:
#pragma pack(1)
    struct T_UTAuthenticationCMD
    {
        UINT32 ulUID;
        UINT32 ulPID;
        UINT8  rand[16];
#ifdef MZ_2ND
        UINT32 ulMZPid;
#endif
    };
#pragma pack()
};


/*
********************************
Authentication Rsp:   UT->McBTS
********************************
*/
#pragma pack(1)
struct T_UTAuthenticationRsp
{
    UINT32 ulUID;
    UINT32 ulPID;
    UINT32 ulResult;
};
#pragma pack()
class CUTAuthenticationRsp:public CMessage
{
public: 
    CUTAuthenticationRsp(CMessage &msg):CMessage(msg){}
    //CUTAuthenticationRsp(){}
    ~CUTAuthenticationRsp(){}

//protected:
//    UINT32 GetDefaultDataLen() const {return sizeof( T_UTAuthenticationCMD );}
//    UINT16 GetDefaultMsgId()   const {return M_L3_CPE_AUTH_CMD;}

public:
    const T_UTAuthenticationRsp* getInfo() const {return (T_UTAuthenticationRsp*)GetDataPtr();}

};


/********************************
Authentication Rsp:   McBTS->SAG
*********************************/
class CsabisAuthenticationRsp:public CMessage
{
public: 
    //CsabisAuthenticationRsp(CMessage &msg):CMessage(msg){}
    CsabisAuthenticationRsp(){}
    ~CsabisAuthenticationRsp(){}

protected:
    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisAuthenticationRsp );}
    UINT16 GetDefaultMsgId()   const {return MSGID_UM_VCR;}

public:
    void setInfo(UINT32, UINT32, UINT32);

private:
#pragma pack(1)
struct T_sabisAuthenticationRsp
{
    UINT32 ulSagID;
    UINT16 usBtsID;
    UINT8  ucEvtGrpID;
    UINT16 usEvtID;
    UINT16 usLength;
    UINT32 ulUID;
    UINT32 ulPID;
    UINT32 ulResult;
};
#pragma pack()
};


/*
================================
Authentication Result:SAG->McBTS
================================
*/
#pragma pack(1)
struct T_sabisAuthenticationResult
{
    UINT32 ulSagID;
    UINT16 usBtsID;
    UINT8  ucEvtGrpID;
    UINT16 usEvtID;
    UINT16 usLength;
    UINT32 ulUID;
    UINT32 ulPID;
    UINT8  auth_result;
   UINT8   Ind;
   UINT8 result;//当Ind为1或者2时携带
   UINT8 SID[16];//当result为0x00时携带
   UINT8 NickName[18];    //当Ind为3时，携带
   UINT8 CID[3];//无论Ind值如何，必填项
#ifdef MZ_2ND
    UINT8* ulMZPid[4];//无论Ind值如何，必填项
#endif
};
#pragma pack()
class CsabisAuthenticationResult:public CMessage
{
public: 
    CsabisAuthenticationResult(CMessage &msg):CMessage(msg){}
    //CsabisAuthenticationResult(){}
    ~CsabisAuthenticationResult(){}

//protected:
//    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisAuthenticationRst );}
//    UINT16 GetDefaultMsgId()   const {return MSGID_VCR_OAM_AUTH_RST;}

public:
    const T_sabisAuthenticationResult * const getInfo() const{return (const T_sabisAuthenticationResult * const)GetDataPtr();}
};


/*
********************************
Authentication Result: McBTS->UT
********************************
*/
class CUTAuthenticationResult:public CMessage
{
public: 
    //CUTAuthenticationResult(CMessage &msg):CMessage(msg){}
    CUTAuthenticationResult(){}
    ~CUTAuthenticationResult(){}

protected:
    UINT32 GetDefaultDataLen() const {return sizeof( T_UTAuthenticationResult );}
    UINT16 GetDefaultMsgId()   const {return M_L3_CPE_AUTH_RESULT;}

public:
    void setInfo(UINT32, UINT32, UINT8);
     void setInfo(UINT32,UINT32,UINT8,UINT8,UINT8,const UINT8* SID,const UINT8* CID);
    void setInfo(UINT32 ulUID, UINT32 ulPID, UINT8 auth_result, UINT8 ind , UINT8 result);
    void setInfo(UINT32 ulUID, UINT32 ulPID, UINT8 auth_result, UINT8 ind , UINT8 *pData, UINT16 len);
    void setMsgLen(UINT16 len){SetDataLength(len);}
private:
#pragma pack(1)
    struct T_UTAuthenticationResult
    {
        UINT32 ulUID;
        UINT32 ulPID;
        UINT8  Auth_result;
	 UINT8  Ind;
	 UINT8 result;        
	 UINT8 CID[3];
	 UINT8 SID[16];
 #ifdef MZ_2ND
    UINT8 ulMZPid[4];
 #endif
    };
#pragma pack()
};


#pragma pack(1)
typedef struct tag_UTBaseInfo
{
////UINT32 ulCID;
    UINT16 usHWtype;    //0-CPE; 1-HS; 2-PC card;
    UINT8  ucSWtype;
	UINT8  rsv;
    UINT32 ulActiveSWversion;
    UINT32 ulStandbySWversion;
    UINT8  ucHWversion[HW_VERSION_LEN];
}T_UTBaseInfo;

typedef struct tag_BWInfo
{
    UINT8  ucMobility;   //0-禁止; 1-允许
    UINT8  ucIF_DHCP_Renew;  //0-禁止; 1-允许
    UINT16 usPeriodicalRegisterTimerValue;  //最大不超过7天，超过此值则按照7天处理
    UINT16 usVlanID;    //1~4095;
    UINT8  reserved1;
    UINT8  ucMaxIPnum;  //<=20;
    UINT32 ulVoicePortMask;
    T_UTSDCfgInfo UTSDCfgInfo;
    UINT8  ucFixIPnum;  //<=20;
    UINT8  reserved2;
    T_CpeFixIpInfo CpeFixIpInfo[MAX_FIX_IP_NUM];
}T_BWInfo;

typedef struct tag_UTProfileIE
{
    UINT8  ucAdminStatus;
    UINT8  ucPerfLogStatus;             //0-disable, 1-enable.
    UINT16 usPerfDataCollectInterval;   //second.
    T_BWInfo BWInfo;
    UINT32 length()const
        {
        UINT8 fixIPnum = (BWInfo.ucFixIPnum > MAX_FIX_IP_NUM)?MAX_FIX_IP_NUM:BWInfo.ucFixIPnum;
        return sizeof(tag_UTProfileIE) -  (MAX_FIX_IP_NUM - fixIPnum) * sizeof(T_CpeFixIpInfo);
        }
    //新老profile结构的转换
    void convert2(T_UTProfile &profile)const
        {
        profile.UTProfileIEBase.AdminStatus = ucAdminStatus;
        profile.UTProfileIEBase.LogStatus   = ucPerfLogStatus;
        profile.UTProfileIEBase.DataCInter  = usPerfDataCollectInterval;
        profile.UTProfileIEBase.Mobility    = BWInfo.ucMobility;
        profile.UTProfileIEBase.DHCPRenew   = BWInfo.ucIF_DHCP_Renew;
        profile.UTProfileIEBase.WLANID      = BWInfo.usVlanID;
        profile.UTProfileIEBase.isDefaultProfile = 0;
        profile.UTProfileIEBase.MaxIpNum    = BWInfo.ucMaxIPnum;
        profile.UTProfileIEBase.VoicePortMask = BWInfo.ulVoicePortMask;
        memcpy(&(profile.UTProfileIEBase.UTSDCfgInfo), &(BWInfo.UTSDCfgInfo), sizeof(T_UTSDCfgInfo));
        profile.UTProfileIEBase.FixIpNum    = (BWInfo.ucFixIPnum > MAX_FIX_IP_NUM)?MAX_FIX_IP_NUM:BWInfo.ucFixIPnum;
        memcpy(profile.CpeFixIpInfo, BWInfo.CpeFixIpInfo, profile.UTProfileIEBase.FixIpNum*sizeof(T_CpeFixIpInfo));
		UINT16 usValueFromBWInfo = BWInfo.usPeriodicalRegisterTimerValue;
	    UINT8 ucPrd   = (usValueFromBWInfo&0xFF00)>>8;
	    UINT8 ucValue = usValueFromBWInfo&0x00FF;
	    UINT16 hours = 0;
	    if (0x2 == ucPrd)//hour
	        hours = ucValue;//seconds
	    else if (0x3 == ucPrd)//Day
	        hours = ucValue * 24;//seconds
	    else if (0x4 == ucPrd)//week
	        hours = ucValue * 24 * 7;//seconds
	    else
	        hours = gDefaultPrdRegTime;
	    if (0 == hours)
	        hours = gDefaultPrdRegTime;
	    if (hours > gMAXPrdRegTime)
	        hours = gMAXPrdRegTime;
	    profile.UTProfileIEBase.ucPeriodicalRegisterTimerValue = hours;
        }
}T_UTProfileNew;

#pragma pack()

/*
 *=========================
 *UT Register_Req:UT=>McBTS
 *=========================
*/
#pragma pack(1)
    struct T_RegisterReq
    {
        UINT16 usTransId;
        UINT16 usVersion;
        UINT32 ulUID;
        UINT32 ulPID;
        T_UTBaseInfo UTBaseInfo;
        T_UTProfile  UTProfile;
        UINT16  magic[2];
        T_L2SpecialFlag   specialFlag;
        UINT32 btsid;
        T_UT_HOLD_BW bwInfo;
        T_MEM_CFG memCfg;
    };
#pragma pack()
class CUTRegisterReq:public CMessage
{
public: 
    CUTRegisterReq(const CMessage &msg):CMessage(msg){}
    //CUTRegisterReq(){}
    ~CUTRegisterReq(){}

protected:
    //UINT32 GetDefaultDataLen() const;
    //UINT16 GetDefaultMsgId() const;
public: 
    UINT16 GetTransactionId() const {return getInfo()->usTransId;}
////UINT16 SetTransactionId(UINT16);

    bool   isValid() const;
    const  T_RegisterReq* const getInfo()const {return (const T_RegisterReq * const)GetDataPtr();};

    void   getSpecialFlag(T_L2SpecialFlag&)const;
    UINT32 getBTSID()const;
     void   getUTHold(T_UT_HOLD_BW& bw) const;
     UINT16 getLen()const {return GetDataLength();};
     void   getMemCfg(T_MEM_CFG& mem_cfg) const;
     bool isWCPEorRCPE() const;
//#ifdef RPT_FOR_V6
	 UINT16 getHWtype()const 
	{ 
		T_RegisterReq* pst = (T_RegisterReq *)GetDataPtr();
		return pst->UTBaseInfo.usHWtype;
	 };
private:
};

#if 0
/*
 **************************
 *UT Register_Rsp:McBTS=>UT
 **************************
*/
class CUTRegisterRsp:public CMessage
{
public: 
    CUTRegisterRsp(const CMessage &msg):CMessage(msg){}
    CUTRegisterRsp(){}
    ~CUTRegisterRsp(){}

protected:
////bool   CreateMessage(CComEntity&);
////bool   CreateMessage(CComEntity &Entity, UINT32 uDataSize);
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId()   const;
public: 
////UINT16 getTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    void   setBWInfoFlag(UINT8);
    void   setUID(UINT32);
    void   setPID(UINT32);
    void   setProfile(const T_UTProfileNew&);
private:
#pragma pack(1)
    struct T_RegisterRsp
    {
        UINT16 usTransId;
////////UINT16 Version;
        UINT8  ucCarryBWInfo;
        UINT32 ulUID;
        UINT32 ulPID;
        T_UTProfileNew UTProfile;
    };
#pragma pack()
};

/*
 **************************
 *update_BWInfo: McBTS=>UT
 **************************
*/
class CUpdateBWInfo:public CMessage
{
public: 
    CUpdateBWInfo(const CMessage &msg):CMessage(msg){}
    CUpdateBWInfo(){}
    ~CUpdateBWInfo(){}

protected:
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId()   const;
public: 
    UINT16 setTransactionId(UINT16);

private:
#pragma pack(1)
    struct T_RegisterRsp
    {
        UINT16 usTransId;
        UINT32 ulUID;
        T_UTProfileNew UTProfile;
    };
#pragma pack()
};
#endif


/*
 ==============================================================
 ==============================================================
 ==============================================================
 */
/***********************
 *BWInfo_Req:McBTS->SAG
************************/
class CsabisBWInfoReq:public CMessage
{
public: 
    //CsabisBWInfoReq(CMessage &msg):CMessage(msg){}
    CsabisBWInfoReq(){}
    ~CsabisBWInfoReq(){}

protected:
    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisBWInfoReq );}
    UINT16 GetDefaultMsgId()   const {return MSGID_UM_VCR;}

public:
    void setInfo(UINT32, UINT32, const T_UTBaseInfo &);

private:
#pragma pack(1)
    struct T_sabisBWInfoReq
    {
        UINT32 ulSagID;
        UINT16 usBtsID;
        UINT8  ucEvtGrpID;
        UINT16 usEvtID;
        UINT16 usLength;
        UINT32 ulUID;
        UINT32 ulPID;
        T_UTBaseInfo UTBaseInfo;
        //UINT8  ucLAI[M_LAI_LENGTH];
    };
#pragma pack()
};

/*
================================
BWInfo_Rsp:   SAG->McBTS
================================
*/
#pragma pack(1)
struct T_sabisMemCfg
    {    
        UINT32 DNSServer;
        UINT32 MemIp;
        UINT32 SubMask;
        UINT32 GateWay;
        UINT32 Rev;
    };
struct T_sabisBWInfoRsp
{
    UINT32   ulSagID;
    UINT16   usBtsID;
    UINT8    ucEvtGrpID;
    UINT16   usEvtID;
    UINT16   usLength;
    UINT32   ulUID;
    UINT32   ulPID;
    T_UTProfileNew UTProfile;
    T_UT_HOLD_BW bwInfo;
    T_sabisMemCfg memCfg;
};
#pragma pack()
class CsabisBWInfoRsp:public CMessage
{
public: 
    CsabisBWInfoRsp(CMessage &msg):CMessage(msg){}
    //CSabisAuthenticationCMD(){}
    ~CsabisBWInfoRsp(){}

//protected:
//    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisAuthenticationCMD );}
//    UINT16 GetDefaultMsgId()   const {return MSGID_OAM_VCR_AUTHINFO_REQ;}

public:
     T_sabisBWInfoRsp*  getInfo() {return ( T_sabisBWInfoRsp * )GetDataPtr();}
   const T_UT_HOLD_BW*   getHoldBW()const ;
   T_sabisMemCfg* getMemCfg();
   UINT16 getLen()const {return GetDataLength();};
};


/*
================================
Delete BWInfo Req:   SAG->McBTS
================================
*/
#pragma pack(1)
struct T_sabisDelBWInfoReq
{
    UINT32   ulSagID;
    UINT16   usBtsID;
    UINT8    ucEvtGrpID;
    UINT16   usEvtID;
    UINT16   usLength;
    UINT32   ulUID;
    UINT32   ulPID;
   // UINT16   flag;/**********/
};
#pragma pack()
class CsabisDeleteBWInfoReq:public CMessage
{
public: 
    CsabisDeleteBWInfoReq(CMessage &msg):CMessage(msg){}
    //CsabisDeleteBWInfoReq(){}
    ~CsabisDeleteBWInfoReq(){}

//protected:
//    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisAuthenticationCMD );}
//    UINT16 GetDefaultMsgId()   const {return MSGID_OAM_VCR_AUTHINFO_REQ;}

public:
    const T_sabisDelBWInfoReq * const getInfo() const{return (const T_sabisDelBWInfoReq * const)GetDataPtr();}
};

/*
================================
Modify BWInfo Req:   SAG->McBTS
================================
*/
#pragma pack(1)
struct T_sabisModBWInfoReq
{
    UINT32   ulSagID;
    UINT16   usBtsID;
    UINT8    ucEvtGrpID;
    UINT16   usEvtID;
    UINT16   usLength;
    UINT32   ulUID;
    UINT32   ulPID;
    T_UTProfileNew UTProfile;
    T_UT_HOLD_BW bwInfo;
    T_sabisMemCfg memCfg;
};
#pragma pack()
//#ifdef RPT_FOR_V6
#pragma pack(1)
struct T_EmsModBWInfoRsp
{
    UINT32   ulUID;
    UINT32   ulPID;
    T_UTProfileNew UTProfile;
    T_UT_HOLD_BW bwInfo;
    T_sabisMemCfg memCfg;
};
#pragma pack()
class CsabisModifyBWInfoReq:public CMessage
{
public: 
    CsabisModifyBWInfoReq(CMessage &msg):CMessage(msg){}
    //CsabisModifyBWInfoReq(){}
    ~CsabisModifyBWInfoReq(){}

//protected:
//    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisAuthenticationCMD );}
//    UINT16 GetDefaultMsgId()   const {return MSGID_OAM_VCR_AUTHINFO_REQ;}

public:
    T_sabisModBWInfoReq * getInfo() const{return ( T_sabisModBWInfoReq * )GetDataPtr();}
     const T_UT_HOLD_BW*   getHoldBW()const ;
     T_sabisMemCfg* getMemCfg();
};

/*
================================
Modify BWInfo Rsp:   McBTS->SAG
================================
*/
#pragma pack(1)
struct T_sabisModBWInfoRsp
{
    UINT32   ulSagID;
    UINT16   usBtsID;
    UINT8    ucEvtGrpID;
    UINT16   usEvtID;
    UINT16   usLength;
    UINT32   ulUID;
    UINT8    ucFlag;    //0-SUCCESS, 1-FAIL.
};
#pragma pack()
class CsabisModifyBWInfoRsp:public CMessage
{
public: 
    //CsabisModifyBWInfoRsp(CMessage &msg):CMessage(msg){}
    CsabisModifyBWInfoRsp(){}
    ~CsabisModifyBWInfoRsp(){}

//protected:
    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisModBWInfoRsp );}
    UINT16 GetDefaultMsgId()   const {return MSGID_UM_VCR;}

public:
    void setInfo(UINT32, UINT8 = OAM_SUCCESS);
};

/********************************
SWICH_OFF_Notify:McBTS->SAG
*********************************/
class CsabisSwitchOFFNotifyReq:public CMessage
{
public: 
    //CsabisAuthenticationInfoReq(CMessage &msg):CMessage(msg){}
    CsabisSwitchOFFNotifyReq(){}
    ~CsabisSwitchOFFNotifyReq(){}

protected:
    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisSwitchOFFNotifyReq );}
    UINT16 GetDefaultMsgId()   const {return MSGID_UM_VCR;}

public:
    void setInfo(UINT32);

private:
#pragma pack(1)
    struct T_sabisSwitchOFFNotifyReq
    {
        UINT32 ulSagID;
        UINT16 usBtsID;
        UINT8  ucEvtGrpID;
        UINT16 usEvtID;
        UINT16 usLength;
        UINT32 ulUID;
 
    };
#pragma pack()
};
/********************************
AccountLogin_Req:UT->McBTS
*********************************/
#pragma pack(1)
struct T_AccountLoginReq
{
    UINT32 BTSID;
    UINT8 ActiveType;
    UINT8 AccountType;
    UINT32 UID;
    UINT8 NickName[18];    
    UINT8 Password[16];
   UINT32 ulPID;
    //UINT32 PID;
    UINT32 Rev; 
};
#pragma pack()
class CUTAccountLoginReq:public CMessage
{
public: 
    CUTAccountLoginReq(CMessage &msg):CMessage(msg){}
    CUTAccountLoginReq(){}
    ~CUTAccountLoginReq(){}

protected:
    //UINT32 GetDefaultDataLen() const {return sizeof( T_AccountLoginReq );}
    //UINT16 GetDefaultMsgId()   const {return MSGID_UM_VCR;}

public:
    const T_AccountLoginReq* getAccountLoginReq() const{ return (T_AccountLoginReq*)GetDataPtr(); }

};
/********************************
AccountLogin_Req:McBTS->SAG
*********************************/
class CsabisAccountLoginReq:public CMessage
{
public: 
    //CsabisAuthenticationInfoReq(CMessage &msg):CMessage(msg){}
    CsabisAccountLoginReq(){}
    ~CsabisAccountLoginReq(){}

protected:
    UINT32 GetDefaultDataLen() const {return sizeof( T_sabisAccountLoginReq );}
    UINT16 GetDefaultMsgId()   const {return MSGID_UM_VCR;}

public:
    void setInfo(const UINT8* pdata, UINT16 len);

private:
#pragma pack(1)
    struct T_sabisAccountLoginReq
    {
        UINT32 ulSagID;
        UINT16 usBtsID;
        UINT8  ucEvtGrpID;
        UINT16 usEvtID;
        UINT16 usLength;
        UINT8 ActiveType;
        UINT8 AccountType;
        UINT32 UID;
        UINT8 NickName[18];    
        UINT8 Password[16];
        UINT32 ulPID;
     //   UINT32 PID;
        UINT32 Rev; 
    };
#pragma pack()
};
#endif
