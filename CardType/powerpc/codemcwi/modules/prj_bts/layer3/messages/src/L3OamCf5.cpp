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

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMCFGBTSGENDATAREQ
#include "L3OamCfgBtsGenDataReq.h"  
#endif
#include <string.h>

CCfgBtsGenDataReq :: CCfgBtsGenDataReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgBtsGenDataReq :: CCfgBtsGenDataReq()
{
}

bool CCfgBtsGenDataReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_GEN_DATA_CFG_REQ);
	return true;
}

UINT32 CCfgBtsGenDataReq :: GetDefaultDataLen() const
{
    return sizeof(T_BtsGDataCfgReq);
}

UINT16 CCfgBtsGenDataReq :: GetTransactionId() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgBtsGenDataReq :: SetTransactionId(UINT16 TransId)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}


UINT32 CCfgBtsGenDataReq :: GetBtsIPAddr() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.BtsIPAddr;
}
    
void   CCfgBtsGenDataReq :: SetBtsIPAddr(UINT32 BtsIPAddr)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.BtsIPAddr = BtsIPAddr;
}

UINT32  CCfgBtsGenDataReq :: GetDefGateway() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.DefGateway;
}
void   CCfgBtsGenDataReq :: SetDefGateway(UINT32 DefGateway)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.DefGateway = DefGateway;
}


UINT32  CCfgBtsGenDataReq :: GetSubnetMask() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.SubnetMask;
}

void   CCfgBtsGenDataReq :: SetSubnetMask(UINT32 SubnetMask)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.SubnetMask = SubnetMask;
}


UINT16  CCfgBtsGenDataReq :: GetSAGID() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.SAGID;
}

void   CCfgBtsGenDataReq :: SetSAGID(UINT16 SAGID)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.SAGID = SAGID;
}


UINT32  CCfgBtsGenDataReq :: GetSAGVoiceIP() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.SAGVoiceIP;
}

void   CCfgBtsGenDataReq :: SetSAGVoiceIP(UINT32 SAGIPaddr)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.SAGVoiceIP = SAGIPaddr;
}

UINT32  CCfgBtsGenDataReq :: GetSAGSignalIP() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.SAGSignalIP;
}

void   CCfgBtsGenDataReq :: SetSAGSignalIP(UINT32 SAGIPaddr)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.SAGSignalIP = SAGIPaddr;
}

UINT32  CCfgBtsGenDataReq :: GetLocAreaID() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.LocAreaID;
}

void   CCfgBtsGenDataReq :: SetLocAreaID(UINT32 LocAreaID)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.LocAreaID = LocAreaID;
}


UINT32 CCfgBtsGenDataReq ::  GetEmsIPAddr() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.EmsIPAddr;
}

void   CCfgBtsGenDataReq :: SetEmsIPAddr(UINT32 EmsIPAddr)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.EmsIPAddr = EmsIPAddr;
}


UINT16  CCfgBtsGenDataReq :: GetNetworkID() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.NetworkID;
}

void   CCfgBtsGenDataReq :: SetNetworkID(UINT16 NetworkID)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.NetworkID = NetworkID;
}

UINT16 CCfgBtsGenDataReq ::  GetBtsBootSource() const
{
    return ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.BtsBootSource;
}

void  CCfgBtsGenDataReq ::  SetBtsBootSource(UINT16 Source)
{
    ((T_BtsGDataCfgReq *)GetDataPtr())->Ele.BtsBootSource = Source;
}

SINT8* CCfgBtsGenDataReq :: GetEle() const
{
    return (SINT8*) &(((T_BtsGDataCfgReq *)GetDataPtr())->Ele);
}

void   CCfgBtsGenDataReq :: SetEle(SINT8 * E)
{
    memcpy(&(((T_BtsGDataCfgReq *)GetDataPtr())->Ele), 
            E, 
            sizeof(T_BtsGDataCfgEle));
}

bool   CCfgBtsGenDataReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_BtsGDataCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgBtsGenDataReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_BtsGDataCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}



CCfgBtsGenDataReq :: ~CCfgBtsGenDataReq()
{

}


