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

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3L3CFGDMDATANOTIFY
#include "L3L3CfgDMDataNotify.h"     
#endif

#include <string.h>

CCfgDMDataNotify :: CCfgDMDataNotify(CMessage &rMsg)
:CMessage(rMsg)
{  
}

CCfgDMDataNotify :: CCfgDMDataNotify()
{

}

bool CCfgDMDataNotify :: CreateMessage(CComEntity &Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;

    SetMessageId(M_CPEM_DM_CPE_DATA_CFG_NOTIFY);

	return true;
}

UINT32 CCfgDMDataNotify :: GetDefaultDataLen() const
{
    return (sizeof(T_CfgNotify));
}


UINT16 CCfgDMDataNotify :: GetTransactionId()const
{
    return ((T_CfgNotify*)GetDataPtr())->TransId;
}

UINT16 CCfgDMDataNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_CfgNotify*)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8  CCfgDMDataNotify :: GetMobility() const
{
    return ((T_CfgNotify*)GetDataPtr())->Mobility;
}

void   CCfgDMDataNotify :: SetMobility(UINT8 Mobility)
{
    ((T_CfgNotify*)GetDataPtr())->Mobility = Mobility;
}

UINT8 CCfgDMDataNotify :: GetDHCPRenew() const
{
    return ((T_CfgNotify*)GetDataPtr())->DHCPRenew;
}

void  CCfgDMDataNotify :: SetDHCPRenew(UINT8 DHCPRenew)
{
    ((T_CfgNotify*)GetDataPtr())->DHCPRenew = DHCPRenew;
}


UINT16 CCfgDMDataNotify :: GetVLanID() const
{
    return ((T_CfgNotify*)GetDataPtr())->VLanID;
}

void  CCfgDMDataNotify ::  SetVLanID(UINT16 VLanID)
{
    ((T_CfgNotify*)GetDataPtr())->VLanID = VLanID;
}

UINT8 CCfgDMDataNotify :: GetMaxIpNum() const
{
    return ((T_CfgNotify*)GetDataPtr())->MaxIpNum;
}

void  CCfgDMDataNotify ::  SetMaxIpNum(UINT8 MaxIpNum)
{
    ((T_CfgNotify*)GetDataPtr())->MaxIpNum = MaxIpNum;
}

UINT8 CCfgDMDataNotify :: GetFixIpNum() const
{
    return ((T_CfgNotify*)GetDataPtr())->FixIpNum;
}

void  CCfgDMDataNotify ::  SetFixIpNum(UINT8 FixIpNum)
{
    ((T_CfgNotify*)GetDataPtr())->FixIpNum = FixIpNum;
}

bool   CCfgDMDataNotify :: GetCpeFixIpInfo(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (SINT8*)&(((T_CfgNotify*)GetDataPtr())->CpeFixIpInfo),
               Len);
        return true;
    }
}

bool   CCfgDMDataNotify :: SetCpeFixIpInfo(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false; 
    }
    else
    {
        memcpy((SINT8*)&(((T_CfgNotify*)GetDataPtr())->CpeFixIpInfo), 
                SrcBuff, 
                Len);
        return true;
    }
}


CCfgDMDataNotify ::  ~CCfgDMDataNotify() 
{

}
