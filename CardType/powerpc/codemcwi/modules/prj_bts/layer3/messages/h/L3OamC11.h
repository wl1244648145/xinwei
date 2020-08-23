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

//5.4.3	Calibration General (EMS)
#ifndef _INC_L3OAMCFGCALGENDATAREQ
#define _INC_L3OAMCFGCALGENDATAREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgCalGenDataReq : public CMessage
{
public: 
    CCfgCalGenDataReq(CMessage &rMsg);
    CCfgCalGenDataReq();
    bool CreateMessage(CComEntity&);
    ~CCfgCalGenDataReq();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
    
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);
private:
#pragma pack(1)
    struct T_CalCfgGenReq
    {
        UINT16  TransId;
        T_CaliGenCfgEle Ele;	
    };
#pragma pack()
};
#endif
