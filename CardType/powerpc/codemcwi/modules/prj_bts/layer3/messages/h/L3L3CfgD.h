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

#ifndef _INC_L3L3CFGDMDATANOTIFY
#define _INC_L3L3CFGDMDATANOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif

class CCfgDMDataNotify : public CMessage
{
public: 
    CCfgDMDataNotify(CMessage &rMsg);
    CCfgDMDataNotify();
    bool CreateMessage(CComEntity&);
    ~CCfgDMDataNotify();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT8  GetMobility() const;
    void   SetMobility(UINT8);
    
    UINT8  GetDHCPRenew() const;
    void   SetDHCPRenew(UINT8);

    UINT16  GetVLanID() const;
    void   SetVLanID(UINT16);

    UINT8  GetMaxIpNum() const;
    void   SetMaxIpNum(UINT8);

    UINT8  GetFixIpNum() const;
    void   SetFixIpNum(UINT8);

    bool   GetCpeFixIpInfo(SINT8*, UINT16 Len) const;
    bool   SetCpeFixIpInfo(SINT8*, UINT16 Len);
private:
#pragma pack(1)
    struct T_CfgNotify
    {
        UINT16 TransId;
        UINT8  Mobility;   //0 -- disabled     1 -- enabled
        UINT8  DHCPRenew;  //Allow DHCP Renew in serving BTS 0-- disabled  1-- enabled
        UINT16 VLanID;
        UINT8  MaxIpNum;   //Max IP ddress number	1~20	
        UINT8  FixIpNum;   //Fixed Ip number		0~20
        T_CpeFixIpInfoForData CpeFixIpInfo[MAX_FIX_IP_NUM];
    };
#pragma pack()
};
#endif
