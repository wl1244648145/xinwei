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

// GPS Data Configuration Request（EMS）
#ifndef _INC_L3OAMCFGGPSDATAREQ
#define _INC_L3OAMCFGGPSDATAREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif


#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CCfgGpsDataReq : public CMessage
{
public: 
    CCfgGpsDataReq(CMessage &rMsg);
    CCfgGpsDataReq();
    bool CreateMessage(CComEntity&);
    ~CCfgGpsDataReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    SINT32 GetLatitude() const;
    void   SetLatitude(SINT32);

    SINT32 GetLongitude() const;
    void   SetLongitude(SINT32);

    SINT32 GetHeight() const;
    void   SetHeight(SINT32);

    SINT32 GetGMTOffset() const;
    void   SetGMTOffset(SINT32);

    UINT8  GetSatelliteCnt() const;
    void   SetSatelliteCnt(UINT8);
private:
#pragma pack(1)
    struct T_GpsDataCfgReq
    {
        UINT16  TransId;
        T_GpsDataCfgEle Ele;
    };
#pragma pack()
};

class CGPSSynchReq : public CMessage
{
public: 
    CGPSSynchReq(CMessage &rMsg):CMessage(rMsg){}
    CGPSSynchReq(){}
    ~CGPSSynchReq(){}
public:
	UINT16 SetTransactionId(UINT16);
    
    void   SetHour(UINT8) ;
    void   SetMinute(UINT8) ;
	void   SetSecond(UINT8) ;
	void   SetMinSecond(int) ;
protected:
    //消息结构有变，长度有变
    UINT32 GetDefaultDataLen() const;
    UINT16 GetDefaultMsgId() const;
    
private:
#pragma pack(1)
    struct T_GPSSynchReq
    {
        UINT16  TransId;
	    UINT8  Hour;
        UINT8  Minute;
        UINT8  Second;
        int    MinSec;
    };
#pragma pack()
};
#endif
