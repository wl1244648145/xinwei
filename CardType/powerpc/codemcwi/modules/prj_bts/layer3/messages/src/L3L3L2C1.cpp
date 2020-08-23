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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L3L2OAMCPEREGNOTIFY
#include "L3L3L2CpeRegNotify.h"
#endif

#include <string.h>
#include <stdio.h>

bool CL3L2CpeRegistNotify :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_CPE_L3_REG_NOTIFY);
    return true;
}
bool CL3L2CpeRegistNotify :: CreateMessage(CComEntity &Entity, UINT32 uDataSize)
{
    CMessage :: CreateMessage(Entity, uDataSize);
    SetMessageId(M_CPE_L3_REG_NOTIFY);

	return true;
}

UINT32 CL3L2CpeRegistNotify :: GetDefaultDataLen() const
{
    return MAX_MESSAGE_SIZE;
}

UINT16 CL3L2CpeRegistNotify :: GetTransactionId()const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CL3L2CpeRegistNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8  CL3L2CpeRegistNotify :: GetDataValid()const
{
    return ((T_Notify*)GetDataPtr())->DataValid;
}


const T_CpeBaseInfo& CL3L2CpeRegistNotify::GetCpeBaseInfo() const
{
    return ((T_Notify*)GetDataPtr())->CpeBaseInfo;
}

#if 0
bool   CL3L2CpeRegistNotify :: SetCpeBaseInfo(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL ==  SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy((UINT8*)&(((T_Notify*)GetDataPtr())->CpeBaseInfo),//((UINT8*)GetDataPtr() + (SINT32)OFFSET_VPORTINFO)
               SrcBuff, 
               Len);
        return true;
    }
}
#endif



const T_UTProfile& CL3L2CpeRegistNotify::getUTProfile() const
{
    return ((T_Notify*)GetDataPtr())->UTProfile;
}


void CL3L2CpeRegistNotify::getSpecialFlag(T_L2SpecialFlag& specialFlag)const 
{
    T_UTProfile *profile = &(((T_Notify*)GetDataPtr())->UTProfile);
    UINT16 *pMagic = (UINT16 *)((UINT8*)profile + profile->length());
    if ((pMagic[0] == 0x55aa) && (pMagic[1] == 0x55aa))
        {
        memcpy((SINT8*)&specialFlag, (SINT8*)(pMagic+2), sizeof(T_L2SpecialFlag));
        }
    else
        {
        memset((SINT8*)&specialFlag, 0, sizeof(T_L2SpecialFlag));
        }
    return;
}

 void CL3L2CpeRegistNotify::getUTHold(T_UT_HOLD_BW& bw) const
 {
       T_UTProfile *profile = &(((T_Notify*)GetDataPtr())->UTProfile);
	 //if(1 ==(profile->UTProfileIEBase.UTSDCfgInfo.Reserved&0x01))//有效的情况下//保持带宽无条件取，桂林机场问题,jy20110802
	 {
	   UINT8 *p = (UINT8 *)((UINT8*)profile + profile->length() + 4 + sizeof(T_L2SpecialFlag) );
	     memcpy((SINT8*)&bw,(SINT8*)p,sizeof(bw));
	 }
	 /*else
	 {
	     memset((SINT8*)&bw, 0, sizeof(bw));
	 }*/
	 return;
  }
const T_MEM_CFG & CL3L2CpeRegistNotify::getMemCfg()const
{
    return ((T_Notify*)GetDataPtr())->memCfg;
}
bool CL3L2CpeRegistNotify::isWCPEorRCPE() const
{
    UINT8 flag;
    T_UTProfile *profile = &(((T_Notify*)GetDataPtr())->UTProfile);
    flag = profile->UTProfileIEBase.UTSDCfgInfo.Reserved;
    if((flag&0x06)==4)
	return true;
    else
	return false;
}
