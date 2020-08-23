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

#ifndef _INC_L3L3L2OAMCPEREGNOTIFY
#define _INC_L3L3L2OAMCPEREGNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif

//UT Registration notification£¨from cpe to BTS£©
class CL3L2CpeRegistNotify : public CMessage
{
public: 
    CL3L2CpeRegistNotify(CMessage &rMsg):CMessage(rMsg){}
    CL3L2CpeRegistNotify(){}
    ~CL3L2CpeRegistNotify(){}

    bool CreateMessage(CComEntity&);
    bool CreateMessage(CComEntity &Entity, UINT32 uDataSize);
    UINT32 GetDefaultDataLen() const;  
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT8  GetDataValid()const; 
    const T_CpeBaseInfo& GetCpeBaseInfo() const;
    const T_UTProfile& getUTProfile() const;
    void   getUTHold(T_UT_HOLD_BW& bw) const;
    void   getSpecialFlag(T_L2SpecialFlag& specialFlag)const;
    const T_MEM_CFG & getMemCfg()const;
    bool isWCPEorRCPE() const;
private:
#pragma pack(1)
    struct T_Notify
    {
        UINT16  TransId;
        UINT16  Version;
        UINT8   DataValid;
        UINT8   Reserve;
        T_CpeBaseInfo CpeBaseInfo;
        T_UTProfile   UTProfile;
        UINT16  magic[2];
        T_L2SpecialFlag   specialFlag;
	T_UT_HOLD_BW   bw;//wangwenhua add 20080917
	T_MEM_CFG memCfg;//mem info, if cpe, is 0 jy081217
    };
#pragma pack()
};


class CL3ZmoduleRegister:public CMessage
{
public: 
    CL3ZmoduleRegister(){}
    CL3ZmoduleRegister(const CMessage &msg):CMessage( msg ){};
    ~CL3ZmoduleRegister(){}

    T_ZmoduleBlock* getZmoduleBlock() const
        {
        return (T_ZmoduleBlock *)GetDataPtr();
        }
};
#endif 
