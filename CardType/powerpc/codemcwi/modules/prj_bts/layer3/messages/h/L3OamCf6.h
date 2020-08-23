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

// BTS Neighbor List Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGBTSNEIBLISTREQ
#define _INC_L3OAMCFGBTSNEIBLISTREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

class CCfgBtsNeibListReq : public CMessage
{
public: 
    CCfgBtsNeibListReq(CMessage &rMsg);
    CCfgBtsNeibListReq();
    bool CreateMessage(CComEntity&, UINT32 datasize);
    ~CCfgBtsNeibListReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    UINT32 GetRealDataLen() const;  
	UINT16 GetNeighborBtsNum() const;
    UINT32 GetDataLenFromNvram();
    void   GetDataFromNvramForCpe(UINT8 *DataBuff, UINT32&);
	void   GetNeiberBtsInfo(UINT16 *BtsNum, UINT32 *BtsId);
private:
#pragma pack(1)
    struct T_BtsNeighborCfgReq
    {
    UINT16 TransId;
    UINT16 NeighborBtsNum;       // 0 -- 20  Neighbor BTS number
    T_BtsNeighborCfgData BtsNeighborCfgData[NEIGHBOR_BTS_NUM];
    };
#pragma pack()
};
class CCfgBtsNeibListCommReq : public CMessage
{
public: 
    CCfgBtsNeibListCommReq(CMessage &rMsg);
    CCfgBtsNeibListCommReq();
    bool CreateMessage(CComEntity&, UINT32 datasize);
    ~CCfgBtsNeibListCommReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    UINT32 GetRealDataLen() const;  
	UINT16 GetNeighborBtsNum() const;
    UINT32 GetDataLenFromNvram();   
	void   GetNeiberBtsInfo(UINT16 *BtsNum, UINT32 *BtsId);
private:
#pragma pack(1)
    struct T_BtsNeighborCommCfgReq
    {
    UINT16 TransId;
    UINT16 NeighborBtsNum;       // 0 -- 20  Neighbor BTS number
    T_BtsNeighborCfgData BtsNeighborCfgData[NEIGHBOR_BTS_NUM];
    };
#pragma pack()
};

#endif
