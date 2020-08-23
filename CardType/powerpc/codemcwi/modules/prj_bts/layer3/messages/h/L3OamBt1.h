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

#ifndef _INC_L3OAMBTSLOADINFONOTIFYTOEMS
#define _INC_L3OAMBTSLOADINFONOTIFYTOEMS

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CL3OamBtsLoadInfoToEms : public CMessage
{
public: 
    CL3OamBtsLoadInfoToEms(CMessage &rMsg);
    CL3OamBtsLoadInfoToEms();
    bool CreateMessage(CComEntity&);
    ~CL3OamBtsLoadInfoToEms();
    UINT32 GetDefaultDataLen() const;  
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    bool   SetLoadInfo(SINT8*, UINT16 Len); 
    bool   GetLoadInfo(SINT8*, UINT16 Len)const; 
private:
    
#pragma pack(1)
    struct T_Notify
    {
        UINT16 TransId;         // 0
        T_BTSLoadInfo BTSLoadInfo;    
	};
#pragma pack()
};
#endif



