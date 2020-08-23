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

#ifndef _INC_L3EMSMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L3CPEPROFUPDATEREQ
#include "L3L3CpeProfUpdateReq.h"     
#endif

#ifndef _INC_L3CPEMESSAGEID
#include "L3CPEMessageId.h"
#endif

#include <string.h>
////////////////
//对于此消息要进行特殊处理,因为中间的数据是可变的,计算后面数据
//的偏移需要前面的数据,因此对数据的操作要按定义的顺序进行
////////////////
UINT16 CL3CpeProfUpdateReq :: GetTransactionId()const
{
    return ((T_L3CpeProfUpdateReq*)GetDataPtr())->hdr.TransId;
}

UINT16 CL3CpeProfUpdateReq :: SetTransactionId(UINT16 TransId)
{
    ((T_L3CpeProfUpdateReq*)GetDataPtr())->hdr.TransId = TransId;
	return 0;
}

void  CL3CpeProfUpdateReq ::  SetVersion(UINT16 V)
{
    ((T_L3CpeProfUpdateReq*)GetDataPtr())->hdr.Version = V;
}

void CL3CpeProfUpdateReq :: SetLocAreaId(UINT32 E )
{
    ((T_L3CpeProfUpdateReq*)GetDataPtr())->hdr.LocAreaId = E;
}


void CL3CpeProfUpdateReq :: SetBWReqInter(UINT16 E )
{
    ((T_L3CpeProfUpdateReq*)GetDataPtr())->hdr.BWReqInter = E;
}

void CL3CpeProfUpdateReq :: SetSessRelThres(UINT16 E )
{
    ((T_L3CpeProfUpdateReq*)GetDataPtr())->hdr.SessRelThres = E;
}


void CL3CpeProfUpdateReq :: SetProfileDataFlag(UINT16 E )
{
    ((T_L3CpeProfUpdateReq*)GetDataPtr())->hdr.ProfileDataFlag = E;
}

UINT16 CL3CpeProfUpdateReq::getProfileDataFlag()
{
    return ((T_L3CpeProfUpdateReq*)GetDataPtr())->hdr.ProfileDataFlag;
}


/*
 *目的:旧CPE对应的Admin_Status不能兼容新CPE, 所有下发的Admin_Status如果是
 *旧CPE,则修改，否则CPE对profile的校准通不过
*/
void CL3CpeProfUpdateReq::validation(UINT8 ucCCBtype)
{
#define M_CCB_TYPE_OLD  (0)
#define M_CCB_TYPE_NEW  (1)
  if(WITH_PROFILEDATA != getProfileDataFlag())
	return;
  UINT8 ucAdmin_Status = ((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.UTProfileIEBase.AdminStatus;
    if ((M_CCB_TYPE_OLD == ucCCBtype)&&(WITH_PROFILEDATA == getProfileDataFlag()))
        {
   //     UINT8 ucAdmin_Status = ((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.UTProfileIEBase.AdminStatus;
        if ((CPE_ADM_STATUS_NOROAM_CID == ucAdmin_Status)||
            (CPE_ADM_STATUS_PID_UID_ERR == ucAdmin_Status)||
            (CPE_ADM_STATUS_NOROAM_LAI == ucAdmin_Status)||
            (CPE_ADM_STATUS_NOROAM_BID == ucAdmin_Status))
            ((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.UTProfileIEBase.AdminStatus = CPE_ADM_STATUS_INV;
        }
	//lijinan 20090420 如果是终端欠费停机0x10或者欠费降带宽0x11
	if((ucAdmin_Status==CPE_ADM_STATUS_FLOW_LIMITED)||(ucAdmin_Status==CPE_ADM_STATUS_NOPAY))
		((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.UTProfileIEBase.AdminStatus = 0;
}


UINT8 CL3CpeProfUpdateReq :: GetFixIpNum() const
{    
   return (((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.UTProfileIEBase).FixIpNum;
}


////////////////////////////////////////////////////////////
//FIX IP Info 以前的数据都是固定的,因此在下面的计算中,都以成员 FIX IP INFO 的地址进行偏移计算
////////////////////////////////////////////////////////////
UINT32 CL3CpeProfUpdateReq::getQosEntryNumOffset()const
{
    T_L3CpeProfUpdateReq *ptr = (T_L3CpeProfUpdateReq*)GetDataPtr();
    UINT32 Offset = sizeof(ptr->hdr);
    if (WITH_PROFILEDATA == ptr->hdr.ProfileDataFlag)
        Offset += ((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.length();
    return Offset;
}

UINT32 CL3CpeProfUpdateReq::getNeighborBtsNumOffset()const
{
    return getQosEntryNumOffset() + 2 + getQosEntryNum() * sizeof(T_CpeToSCfgEle);
}

UINT16 CL3CpeProfUpdateReq::getNeighborBtsNum()const
{
    return *(UINT16*)((UINT8*)GetDataPtr() + getNeighborBtsNumOffset());
}

void CL3CpeProfUpdateReq :: SetQosEntryNum(UINT16 E)
{
    E = (E>MAX_TOS_ELE_NUM)?MAX_TOS_ELE_NUM:E;
    *(UINT16*)((UINT8*)GetDataPtr() + getQosEntryNumOffset()) = E;
}

UINT16 CL3CpeProfUpdateReq::getQosEntryNum()const
{
    return *(UINT16*)((UINT8*)GetDataPtr() + getQosEntryNumOffset());
}

bool CL3CpeProfUpdateReq::SetTosInfo(const SINT8* SrcBuff, UINT16 Len)
{
    if(NULL ==  SrcBuff)
    {
        return false; 
    }
    else
    {
        UINT32 Offset = getQosEntryNumOffset() + 2;
        memcpy(((SINT8*)GetDataPtr() + Offset), SrcBuff, Len);
        return true;
    }
}



bool CL3CpeProfUpdateReq::SetFreqInfo(const SINT8* SrcBuff, UINT16 Len)
{
    if(NULL ==  SrcBuff)
    {
        return false; 
    }
    else
    {
        memcpy(((SINT8*)GetDataPtr() + getNeighborBtsNumOffset()), SrcBuff, Len);
        return true;
    }
}
#if 0
const T_UTProfile& CL3CpeProfUpdateReq::getUTProfile()const
{
    return ((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile;
}
#endif
#ifdef RCPE_SWITCH
bool CL3CpeProfUpdateReq::setUTSDCfgRsv(UINT8 ucRsv)
{
   ((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.UTProfileIEBase.UTSDCfgInfo.Reserved |= ucRsv;
}
#endif

bool CL3CpeProfUpdateReq::setUTProfile(const T_UTProfile &profile, UINT8 isWcpeORRcpe)
{
    UINT32 len = profile.length();
    SINT8 * pDst = (SINT8*)&((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile;   
    T_UTProfile tprofile;
    memcpy(&tprofile, &profile, len);    
    if(isWcpeORRcpe==1)
    {
        tprofile.UTProfileIEBase.UTSDCfgInfo.Reserved &= 0xf9;
        tprofile.UTProfileIEBase.UTSDCfgInfo.Reserved |= 0x04;
    }
    else
	 tprofile.UTProfileIEBase.UTSDCfgInfo.Reserved &= 0xf9;
    memcpy(pDst, &tprofile, len);
    SetProfileDataFlag(WITH_PROFILEDATA);
    return true;
}

/////////////////////////////////////////
//消息创建完后，可使用该函数计算
//消息体的长度
/////////////////////////////////////////
UINT32 CL3CpeProfUpdateReq::length()const
{
#if 0
    //printf("->CL3CpeProfUpdateReq::length()");
    T_L3CpeProfUpdateReq *ptr = (T_L3CpeProfUpdateReq*)GetDataPtr();
    //不包含指针的长度
    UINT32 len = sizeof(T_L3CpeProfUpdateReq)-sizeof(UINT32*)/*UINT32 *BTSID*/-sizeof(T_CpeToSCfgEle *);
    //profile实际长度
    len -= (MAX_FIX_IP_NUM - ptr->UTProfile.UTProfileIEBase.FixIpNum)*sizeof(T_CpeFixIpInfo);
    //指针指向的数据长度
    len += getQosEntryNum() * sizeof(T_CpeToSCfgEle) + getNeighborBtsNum() * sizeof(UINT32);
#endif
    T_L3CpeProfUpdateReq *ptr = (T_L3CpeProfUpdateReq*)GetDataPtr();
    UINT32 len = sizeof(ptr->hdr)
        + 2 + getQosEntryNum()    * sizeof(T_CpeToSCfgEle)
        + 2 + getNeighborBtsNum() * sizeof(UINT32);
    if (WITH_PROFILEDATA == ptr->hdr.ProfileDataFlag)
        len += ptr->UTProfile.length();

    //没有计算repeater的长度
    UINT16 repeaterNum = *(UINT16*)(((UINT8*)ptr) + len);
    UINT32 repeaterLen = 2 + repeaterNum*sizeof(UINT16);
    len += repeaterLen;
    return len;
}


////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////
bool CZmoduleRegsterResponse::CreateMessage(CComEntity &Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;

    SetMessageId(M_BTS_CPE_Z_REGISTER_RESPONSE);

    return true;
}

UINT32 CZmoduleRegsterResponse::GetDefaultDataLen() const
{
    return sizeof(T_ZloginResponse);
}

void CZmoduleRegsterResponse::setLoginFlag  (UINT8 flag)
{
    ((T_ZloginResponse *)GetDataPtr())->LoginFlag = flag;
}

void CZmoduleRegsterResponse::setCidNum(UINT8 ucCidNum)
{
    ((T_ZloginResponse *)GetDataPtr())->cidNum = ucCidNum;
}
void CZmoduleRegsterResponse::setZNum  (UINT8 uc)
{
    ((T_ZloginResponse *)GetDataPtr())->ucZNum = uc;
}
void CZmoduleRegsterResponse::setMsgInd(UINT8 uc)
{
    ((T_ZloginResponse *)GetDataPtr())->ucMsgInd = uc;
}
#if 0
////////////////////////////////////////
//
//
////////////////////////////////////////
void CUpdateUTBWInfo::setPrdRegTime(UINT16 value)
{
    *(UINT16*)((SINT8*)GetDataPtr() + CL3CpeProfUpdateReq::length()) = value;
}
#endif
