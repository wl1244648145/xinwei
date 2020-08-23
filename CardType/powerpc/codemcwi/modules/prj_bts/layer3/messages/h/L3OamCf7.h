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

#ifndef _INC_L3OAMCFGBTSREPEATERREQ
#define _INC_L3OAMCFGBTSREPEATERREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CL3OamCfgBtsRepeaterReq : public CMessage
{
public: 
    CL3OamCfgBtsRepeaterReq(CMessage &rMsg);
    CL3OamCfgBtsRepeaterReq();
    bool CreateMessage(CComEntity&, UINT32 uDataSize);
    ~CL3OamCfgBtsRepeaterReq();
    UINT32 GetDefaultDataLen() const;  
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    bool   SetLoadInfo(SINT8*, UINT16 Len); 
    bool   GetLoadInfo(SINT8*, UINT16 Len)const; 
    UINT32 GetRealDataLen() const;  
	UINT32 GetDataLenFromNvram() const;  
private:
    
#pragma pack(1)
    struct T_Req
    {
        UINT16 TransId;         // 0
		UINT16 RepeaterFreqNum;	
		UINT16 RepeaterFreqInd[NEIGHBOR_BTS_NUM];
    };
#pragma pack()
};
#endif



