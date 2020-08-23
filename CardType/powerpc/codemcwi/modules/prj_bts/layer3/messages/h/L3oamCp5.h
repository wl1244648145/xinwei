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

//数据结构已根据最新文档更新
#ifndef _INC_L3OAMCPEREGNOTIFY
#define _INC_L3OAMCPEREGNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif



#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif


//UT Registration Notify（BTS）
class CCpeRegistNotify: public CMessage
{
public: 
    CCpeRegistNotify(CMessage &rMsg):CMessage(rMsg){}
    CCpeRegistNotify(){}
    ~CCpeRegistNotify(){}

    bool CreateMessage(CComEntity&, UINT32);
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
////UINT32 GetCPEID() const;
    void   SetCPEID(UINT32);
    
#if 0
    bool   GetCpeBaseInfo(SINT8*, UINT16 Len)const; 
    bool   SetCpeBaseInfo(SINT8*, UINT16 Len); 

    UINT8  GetAdminStatus()const; 
    void   SetAdminStatus(UINT8); 

    UINT8  GetLogStatus()const; 
    void   SetLogStatus(UINT8); 

    UINT16 GetDataCInter()const; 
    void   SetDataCInter(UINT16); 

    UINT8  GetMobility() const;
    void   SetMobility(UINT8);
    
    UINT8  GetDHCPRenew() const;
    void   SetDHCPRenew(UINT8);

    UINT8  GetMaxIpNum() const;
    void   SetMaxIpNum(UINT8 );

    bool   GetUTSerDisIE(SINT8*, UINT16)const; 
    bool   SetUTSerDisIE(SINT8*, UINT16); 

    UINT8  GetFixIpNum() const;
    void   SetFixIpNum(UINT8);

    UINT16  GetWLANID() const;
    void    SetWLANID(UINT16 WLANID);
#endif
    
////bool   GetUTProfIE(SINT8*, UINT16 Len)const; 
////bool   SetUTProfIE(SINT8*, UINT16); 

////bool   GetRegData(SINT8*, UINT16 Len)const; 
////bool   SetRegData(SINT8*, UINT16 Len); 
    void   setCpeBaseInfo(const T_CpeBaseInfo&);
    void   setUTProfile(const T_UTProfile&);
private:
#pragma pack(1)
    struct T_RegNotify
    {
        UINT16  TransId;
        UINT32  CPEID;
        T_CpeBaseInfo  CpeBaseInfo;   // SIZE IS 20 BYTES
        T_UTProfile    UTProfile;
    };
#pragma pack()
};


//Z module register notification.
class CCpeZRegisterNotify:public CMessage
{
public: 
    CCpeZRegisterNotify (CMessage &rMsg):CMessage(rMsg){}
    CCpeZRegisterNotify (){}
    ~CCpeZRegisterNotify(){}

    bool CreateMessage(CComEntity&);

////UINT16  GetTransactionId() const;
    UINT16  SetTransactionId(UINT16);
////UINT32  getCpeZId() const;
    void    setCpeZId(UINT32);
////UINT8   getZnum() const;
    void    setZnum(UINT8);
////UINT32* getUIDs() const;
    void    setUID_Ver(UINT8*, UINT32);

protected:
    UINT32 GetDefaultDataLen() const;

private:
#pragma pack(1)
    struct T_CpeZRegister2EMS
    {        
        UINT16  TransId;
        UINT32  eid;
////////UINT8   UIDnum;
////////UINT32  UIDs[MAX_CID_NUM];
        UINT8   Znum;
        T_UID_VER_PAIR uid_ver[MAX_Z_MOD_NUM];
    };
#pragma pack()
};
#endif
