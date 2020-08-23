#ifndef _INC_L3OAMCPERPOFILEUPDATEREQ
#define _INC_L3OAMCPERPOFILEUPDATEREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

//This message can be sent either as the response to the registration 
//request or when the configuration of the CPE is changed
class CCpeProfUpdateReq : public CMessage
{
public: 
    CCpeProfUpdateReq(CMessage &rMsg);
    CCpeProfUpdateReq();
    bool CreateMessage(CComEntity&);
    ~CCpeProfUpdateReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransId();
    void   SetTransId(UINT16);

    UINT32 GetEID();
    void   SetEID(UINT32);
    
    UINT8  GetAdminStatus()const; 
    void   SetAdminStatus(UINT8); 

    UINT8  GetLogStatus()const; 
    void   SetLogStatus(UINT8); 

    UINT16 GetDataCInter()const; 
    void   SetDataCInter(UINT16); 

    UINT8  GetMobility() const;
    void   SetMobility(UINT8);
    
    UINT32 GetDHCPRenew() const;
    void   SetDHCPRenew(UINT32);

    UINT32 GetSecurity() const;
    void   SetSecurity(UINT32);

    UINT32 GetBCFilter() const;
    void   SetBCFilter(UINT32);

    UINT32 GetMaxIpNum() const;
    void   SetMaxIpNum(UINT32);

    UINT8  GetVoiceSrv() const;
    void   SetVoiceSrv(UINT8);

    bool   GeTVPortInfo(SINT8*, UINT16)const;
    bool   SetVPortInfo(SINT8*, UINT16);

    bool   GetUTSerDisIE(SINT8*, UINT16)const; 
    bool   SetUTSerDisIE(SINT8*, UINT16); 

    UINT32 GetFixIpNum() const;
    void   SetFixIpNum(UINT32);
    
    bool   GetCpeFixIpInfo(SINT8*, UINT16 Len) const;
    bool   SetCpeFixIpInfo(SINT8*, UINT16 Len);

    UINT16 GetUTProfIESize();
private:
    enum 
    {
        SIZE_BASEMESSAGE    =  186
        // 基本消息大小指的是 T_CpeFixIpInfo *CpeFixIpInfo 以前的域大小
    };
#pragma pack(1)
    struct T_L3CpeProfUpdateReq
    {
        UINT16  TransId;         // 0
        UINT32  CPEID;
        /////////// PROFILE IE BEGIN /////////sizeof(T_UTProfileIEBase) = 179
        UINT8   AdminStatus;    //  0 - active  1 - suspended  2 -- invalid	
        UINT8   LogStatus;      //  Perf Log Status	1	0 - disabled  1 - enabled	
        UINT16  DataCInter;     //  Perf data collection Interval	2	Seconds	
        UINT8   Mobility;       //  0 - disabled  1 - enabled	
        UINT8   DHCPRenew;      //  Allow DHCP renew in serving BTS	1	0 - disabled  1 - enabled	
        UINT8   Security;	    //  0 - disabled  1 - enabled	
        UINT8   BCFilter;       //  Broadcast Filtering		0 - disabled  1 - enabled	
        UINT8   MaxIpNum;       //  Max IP address number		1~20	
        UINT8   VoiceSrv;       //  Voice service		0 - disabled  1 - enabled	
        T_VoicePortInfo VoicePort[MAX_VOICE_PORT_NUM];
        T_UTSDCfgInfo UTSDCfgInfo;
        UINT8  FixIpNum;        //Fixed Ip number		0~20
        T_CpeFixIpInfo *CpeFixIpInfo;
        /////////// PROFILE IE END ////////////////
    };
#pragma pack()
};
#endif
