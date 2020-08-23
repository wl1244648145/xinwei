/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3OamCfgGetRFData.h
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   09/21/2006   肖卫方       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMCFGGETRFDATA_H_
#define _INC_L3OAMCFGGETRFDATA_H_

#include "Message.h"
#include "L3OamCfgCommon.h"


class CCfgCalGetRFDataReq:public CMessage
{
public: 
    CCfgCalGetRFDataReq(const CMessage &rMsg):CMessage(rMsg){}
    ~CCfgCalGetRFDataReq(){}

public:
    UINT16 GetTransactionId() const
        {
        return ((_tag_GetRFDataReq *)GetDataPtr())->usTransId;
        }
    UINT16 SetTransactionId(UINT16 usTransID)
        {
        return ((_tag_GetRFDataReq *)GetDataPtr())->usTransId = usTransID;
        }

private:
#pragma pack(1)
    struct _tag_GetRFDataReq
    {
        UINT16  usTransId;
        UINT16  IdontCare;  //我不关心的数据结构
    };
#pragma pack()
};
#endif

