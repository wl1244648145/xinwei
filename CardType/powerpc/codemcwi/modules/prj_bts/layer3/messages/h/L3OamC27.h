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

#ifndef _INC_L3OAMCFGNEIGHBOURBTSLOADINFOREQ
#define _INC_L3OAMCFGNEIGHBOURBTSLOADINFOREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif


class CL3OamSyncNeighbourBtsLoadInfoReq : public CMessage
{
public: 
    CL3OamSyncNeighbourBtsLoadInfoReq(const CMessage &rMsg):CMessage( rMsg ){}
    CL3OamSyncNeighbourBtsLoadInfoReq(){}
    ~CL3OamSyncNeighbourBtsLoadInfoReq(){}

    bool   CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;  
public: 
//    UINT16 GetTransactionId() const;
//    UINT16 SetTransactionId(UINT16);

    UINT32 GetRealDataLen() const;  
    void                    setLoadInfo(const SINT8 *);
    T_NeighbotBTSLoadInfo*  getLoadInfo()const;

private:
#if 0
#pragma pack(1)
    struct T_Req
    {
        UINT16 TransId;         // 0
		UINT16 NeighborBTSNum;	
		T_NeighbotBTSLoadInfo NeighbotBTSLoadInfo[NEIGHBOR_BTS_NUM];
    };
#pragma pack()
#endif
};
#endif



