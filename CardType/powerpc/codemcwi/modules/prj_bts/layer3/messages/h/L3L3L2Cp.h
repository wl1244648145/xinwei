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
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3L3L2CPEPROFUPDATENOTIFY
#define _INC_L3L3L2CPEPROFUPDATENOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif
#include "string.h"

//UT profile update Notify£¨BTS l3 to l2£©
class CL3L2CpeProfUpdateNotify: public CMessage
{
public: 
    CL3L2CpeProfUpdateNotify(CMessage &rMsg);
    CL3L2CpeProfUpdateNotify();
    bool CreateMessage(CComEntity&);
    ~CL3L2CpeProfUpdateNotify();
    UINT32 GetDefaultDataLen() const;  
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT32 GetCpeId() const;
    void   SetCpeId(UINT32);
    
    bool   SetUTSDCfgInfo(const T_UTSDCfgInfo &);

    void   SetZmoduleEnabled(bool);

    void   setSpecialFlag(T_L2SpecialFlag &flag) {memcpy(&(((T_Notify*)GetDataPtr())->flag), &flag, sizeof(flag));}
    void setUTHoldBW(T_UT_HOLD_BW &bw){memcpy(&(((T_Notify*)GetDataPtr())->bw), &bw, sizeof(bw));}

private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        UINT32  CpeId; 
        T_UTSDCfgInfo UTSDCfgInfo;
        UINT8   Z_Module_enabled;
        UINT8   reserve;
        T_L2SpecialFlag flag;
	T_UT_HOLD_BW bw;//wangwenhua add 20080916
    };
#pragma pack()
};
#endif


