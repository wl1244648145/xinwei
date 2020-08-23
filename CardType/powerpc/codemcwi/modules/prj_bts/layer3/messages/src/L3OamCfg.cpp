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

#ifndef _INC_L3OAMCFGACLREQ
#include "L3OamCfgACLReq.h"    
#endif
#include <string.h>

bool CCfgACLReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_CFG_ACL_REQ);
	return true;
}

UINT32 CCfgACLReq :: GetDefaultDataLen() const
{
    return sizeof(T_ACLCfgReq);
}

UINT16 CCfgACLReq :: GetTransactionId() const
{
    return ((T_ACLCfgReq*)GetDataPtr())->TransId;
}

UINT16 CCfgACLReq :: SetTransactionId(UINT16 TransId)
{
    ((T_ACLCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

bool   CCfgACLReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (UINT8*)&(((T_ACLCfgReq *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgACLReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy((UINT8*)&(((T_ACLCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}
/////////////////////////////////////////////
bool CCfgVlanGroupReq :: CreateMessage(CComEntity &Entity)
{
    if(false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_EMS_BTS_VLAN_GROUP_CFG_REQ);
    return true;
}

UINT32 CCfgVlanGroupReq :: GetDefaultDataLen() const
{
    return sizeof(T_VlanGroupCfgReq);
}

UINT16 CCfgVlanGroupReq :: GetTransactionId() const
{
    return ((T_VlanGroupCfgReq*)GetDataPtr())->TransId;
}

UINT16 CCfgVlanGroupReq :: SetTransactionId(UINT16 TransId)
{
    ((T_VlanGroupCfgReq *)GetDataPtr())->TransId = TransId;
    return 0;
}

UINT8* CCfgVlanGroupReq :: GetEle()const
{
    return (UINT8*)&(((T_VlanGroupCfgReq *)GetDataPtr())->Ele);
}

bool   CCfgVlanGroupReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy((UINT8*)&(((T_VlanGroupCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}
///////////////////////////////////////////////////
bool CCfgClusterParaReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_CLUSTER_PARA_CFG);
	return true;
}

UINT32 CCfgClusterParaReq :: GetDefaultDataLen() const
{
    return sizeof(T_ClusterPara);
}

UINT16 CCfgClusterParaReq :: GetTransactionId() const
{
    return ((T_ClusterPara*)GetDataPtr())->TransId;
}

UINT16 CCfgClusterParaReq :: SetTransactionId(UINT16 TransId)
{
    ((T_ClusterPara *)GetDataPtr())->TransId = TransId;
	return 0;
}

bool   CCfgClusterParaReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (UINT8*)&(((T_ClusterPara *)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgClusterParaReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy((UINT8*)&(((T_ClusterPara*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}
