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

//5.4.4.1	Calibration Result Notification£¨BTS£©
#ifndef _INC_L3OAMCFGCALRSTNOTIFY
#define _INC_L3OAMCFGCALRSTNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgCalRstNotify : public CMessage
{
public: 
    CCfgCalRstNotify(CMessage &rMsg);
    CCfgCalRstNotify();
    bool CreateMessage(CComEntity&);
    ~CCfgCalRstNotify();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    UINT16 GetType() const;  // 1:TXCAL_I  2:TXCAL_Q  3:RXCAL_I  4:RXCAL_Q
    UINT16 GetAntennalIndex() const;  //	0~7
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_CalRstNotify
    {
        UINT16  TransId;
        T_CaliResultNotifyEle Ele;
    };
#pragma pack()
};
#endif
