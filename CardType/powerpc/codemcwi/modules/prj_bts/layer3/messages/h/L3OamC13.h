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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

//5.4.4.1	Calibration Result Notification（BTS）
#ifndef _INC_L3OAMCFGCALRSTGENNOTIFY
#define _INC_L3OAMCFGCALRSTGENNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgCalRstGenNotify : public CMessage
{
public: 
    CCfgCalRstGenNotify(CMessage &rMsg);
    CCfgCalRstGenNotify();
    bool CreateMessage(CComEntity&);
    ~CCfgCalRstGenNotify();
    UINT32 GetDefaultDataLen() const;  
public:
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    SINT8* GetEle() const;
    void   SetEle(SINT8 *);
#if 0
    bool   IsAntennaErr();
    bool   IsSynErr();
    bool   IsPreDistortErr();
    bool   IsPreCalibrationErr();
#endif
#ifndef WBBU_CODE
    bool   IsCalibrationErr();
#else
    unsigned short   IsCalibrationErr();
#endif
    UINT16 GetErrorAntenna();     //返回值是位域类型，每一位标识一个天线的状态 
    bool   GetEle(SINT8* , UINT16) const;
    bool   SetEle(SINT8* , UINT16);

    
private:
#pragma pack(1)
    struct T_CalRstGenNotify
    {
        UINT16          TransId;
        T_CaliGenCfgEle Ele; 
        T_CalibrationError Error;
    };
#pragma pack()
};
#endif
