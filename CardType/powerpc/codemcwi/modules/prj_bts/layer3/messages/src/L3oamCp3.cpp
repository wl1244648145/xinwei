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
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMCPEREGNOTIFY
#include "L3OamCpeRegNotify.h"
#endif
#include <string.h>

bool CCpeRegistNotify :: CreateMessage(CComEntity &ComEntity, UINT32 Len)
{
    if (false == CMessage :: CreateMessage(ComEntity, Len))
        return false;
    SetMessageId(M_BTS_EMS_UT_REG_NOTIFY);
    return true;
}

UINT32 CCpeRegistNotify :: GetDefaultDataLen() const
{
    return sizeof(T_RegNotify);
}

UINT16 CCpeRegistNotify :: GetTransactionId() const
{
    return ((T_RegNotify *)GetDataPtr())->TransId;
}

UINT16 CCpeRegistNotify :: SetTransactionId(UINT16 T)
{
    ((T_RegNotify *)GetDataPtr())->TransId = T;
	return 0;
}

void   CCpeRegistNotify ::SetCPEID(UINT32 T)
{
    ((T_RegNotify *)GetDataPtr())->CPEID = T;
}

#if 0
bool   CCpeRegistNotify :: GetCpeBaseInfo(SINT8* DstBuff, UINT16 Len) const
{
    if(NULL ==  DstBuff)
    {
        return false;
    }
    else
    {
        memcpy((UINT8*)&(((T_RegNotify*)GetDataPtr())->CpeBaseInfo),
               DstBuff, 
               Len);
        return true;
    }
}


bool   CCpeRegistNotify :: SetCpeBaseInfo(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL ==  SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy((UINT8*)&(((T_RegNotify*)GetDataPtr())->CpeBaseInfo),
               SrcBuff, 
               Len);
        return true;
    }
}

UINT8  CCpeRegistNotify :: GetAdminStatus()const
{
    return (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).AdminStatus;
}

void  CCpeRegistNotify ::  SetAdminStatus(UINT8 E) 
{
    (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).AdminStatus = E;
}

UINT8  CCpeRegistNotify :: GetLogStatus()const 
{
    return (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).LogStatus;
}

void   CCpeRegistNotify :: SetLogStatus(UINT8 E) 
{
    (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).LogStatus = E;
}

UINT16 CCpeRegistNotify :: GetDataCInter()const 
{
    return (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).DataCInter;
}

void  CCpeRegistNotify ::  SetDataCInter(UINT16 E) 
{
    (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).DataCInter = E;
}

UINT8  CCpeRegistNotify :: GetMobility() const
{
    return (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).Mobility;
}

void   CCpeRegistNotify :: SetMobility(UINT8 Mobility)
{
    (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).Mobility = Mobility;
}

UINT8  CCpeRegistNotify :: GetDHCPRenew() const
{
    return (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).DHCPRenew;
}

void  CCpeRegistNotify :: SetDHCPRenew(UINT8 DHCPRenew)
{
    (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).DHCPRenew = DHCPRenew;
}


UINT8 CCpeRegistNotify :: GetMaxIpNum() const
{
    return (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).MaxIpNum;
}

void  CCpeRegistNotify ::  SetMaxIpNum(UINT8 MaxIpNum)
{
    (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).MaxIpNum = MaxIpNum;
}


UINT16  CCpeRegistNotify :: GetWLANID() const
{    
   return (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).WLANID;
}

void   CCpeRegistNotify :: SetWLANID(UINT16 WLANID)
{
    (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).WLANID = WLANID;
}


bool   CCpeRegistNotify :: GetUTSerDisIE(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL ==  DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (UINT8*)&((((T_RegNotify*)GetDataPtr())->UTProfileIEBase).UTSDCfgInfo),//((UINT8*)GetDataPtr() + (SINT32)OFFSET_UTSERVDISIE)
               Len);
        return true;
    }
}

bool   CCpeRegistNotify :: SetUTSerDisIE(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL ==  SrcBuff)
    {
        return false; 
    }
    else
    {
        memcpy((UINT8*)&((((T_RegNotify*)GetDataPtr())->UTProfileIEBase).UTSDCfgInfo),//((UINT8*)GetDataPtr() + (SINT32)OFFSET_UTSERVDISIE)
               SrcBuff, 
               Len);
        return true;
    }
}

UINT8  CCpeRegistNotify :: GetFixIpNum() const
{
    return (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).FixIpNum;
}

void  CCpeRegistNotify ::  SetFixIpNum(UINT8 FixIpNum)
{
    (((T_RegNotify*)GetDataPtr())->UTProfileIEBase).FixIpNum = FixIpNum;
}

bool   CCpeRegistNotify :: GetCpeFixIpInfo(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL ==  DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (SINT8*)&(((T_RegNotify*)GetDataPtr())->CpeFixIpInfo),
               Len);
        return true;
    }
}
#endif


void CCpeRegistNotify::setCpeBaseInfo(const T_CpeBaseInfo &cpeBaseInfo)
{
    memcpy((SINT8*)&(((T_RegNotify*)GetDataPtr())->CpeBaseInfo), (SINT8*)&cpeBaseInfo, sizeof(cpeBaseInfo));
}


void CCpeRegistNotify::setUTProfile(const T_UTProfile &profile)
{
    memcpy((SINT8*)&(((T_RegNotify*)GetDataPtr())->UTProfile), (SINT8*)&profile, profile.length());
}


#if 0
UINT16 CCpeZRegisterNotify::GetTransactionId() const
{
    return ((T_CpeZRegister2EMS *)GetDataPtr())->TransId;
}
#endif

UINT16 CCpeZRegisterNotify::SetTransactionId(UINT16 transid)
{
    ((T_CpeZRegister2EMS *)GetDataPtr())->TransId = transid;
	return 0;
}

bool CCpeZRegisterNotify::CreateMessage(CComEntity &ComEntity)
{
    if (false == CMessage::CreateMessage(ComEntity))
        return false;
    SetMessageId(M_BTS_EMS_CPEZ_REG_NOTIFY);
    return true;
}

UINT32 CCpeZRegisterNotify::GetDefaultDataLen() const
{
    return sizeof(T_CpeZRegister2EMS);
}

#if 0
UINT32 CCpeZRegisterNotify::getCpeZId() const
{
    return ((T_CpeZRegister2EMS *)GetDataPtr())->eid;
}
#endif
void CCpeZRegisterNotify::setCpeZId(UINT32 eid)
{
    ((T_CpeZRegister2EMS *)GetDataPtr())->eid = eid;
}

#if 0
UINT8 CCpeZRegisterNotify::getZnum() const
{
    return ((T_CpeZRegister2EMS *)GetDataPtr())->UIDnum;
}
#endif

void CCpeZRegisterNotify::setZnum(UINT8 num)
{
    ((T_CpeZRegister2EMS *)GetDataPtr())->Znum = num;
}

#if 0
UINT32* CCpeZRegisterNotify::getUIDs() const
{
    return ((T_CpeZRegister2EMS *)GetDataPtr())->UIDs;
}
#endif

void CCpeZRegisterNotify::setUID_Ver(UINT8 *pUid_ver, UINT32 length)
{
    if ((0 == length) || (length > MAX_Z_MOD_NUM*sizeof(T_UID_VER_PAIR)))
        return;
    memcpy(((T_CpeZRegister2EMS *)GetDataPtr())->uid_ver, pUid_ver, length);
}
