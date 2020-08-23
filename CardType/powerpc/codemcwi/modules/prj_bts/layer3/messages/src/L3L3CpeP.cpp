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
//���ڴ���ϢҪ�������⴦��,��Ϊ�м�������ǿɱ��,�����������
//��ƫ����Ҫǰ�������,��˶����ݵĲ���Ҫ�������˳�����
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
 *Ŀ��:��CPE��Ӧ��Admin_Status���ܼ�����CPE, �����·���Admin_Status�����
 *��CPE,���޸ģ�����CPE��profile��У׼ͨ����
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
	//lijinan 20090420 ������ն�Ƿ��ͣ��0x10����Ƿ�ѽ�����0x11
	if((ucAdmin_Status==CPE_ADM_STATUS_FLOW_LIMITED)||(ucAdmin_Status==CPE_ADM_STATUS_NOPAY))
		((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.UTProfileIEBase.AdminStatus = 0;
}


UINT8 CL3CpeProfUpdateReq :: GetFixIpNum() const
{    
   return (((T_L3CpeProfUpdateReq*)GetDataPtr())->UTProfile.UTProfileIEBase).FixIpNum;
}


////////////////////////////////////////////////////////////
//FIX IP Info ��ǰ�����ݶ��ǹ̶���,���������ļ�����,���Գ�Ա FIX IP INFO �ĵ�ַ����ƫ�Ƽ���
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
//��Ϣ������󣬿�ʹ�øú�������
//��Ϣ��ĳ���
/////////////////////////////////////////
UINT32 CL3CpeProfUpdateReq::length()const
{
#if 0
    //printf("->CL3CpeProfUpdateReq::length()");
    T_L3CpeProfUpdateReq *ptr = (T_L3CpeProfUpdateReq*)GetDataPtr();
    //������ָ��ĳ���
    UINT32 len = sizeof(T_L3CpeProfUpdateReq)-sizeof(UINT32*)/*UINT32 *BTSID*/-sizeof(T_CpeToSCfgEle *);
    //profileʵ�ʳ���
    len -= (MAX_FIX_IP_NUM - ptr->UTProfile.UTProfileIEBase.FixIpNum)*sizeof(T_CpeFixIpInfo);
    //ָ��ָ������ݳ���
    len += getQosEntryNum() * sizeof(T_CpeToSCfgEle) + getNeighborBtsNum() * sizeof(UINT32);
#endif
    T_L3CpeProfUpdateReq *ptr = (T_L3CpeProfUpdateReq*)GetDataPtr();
    UINT32 len = sizeof(ptr->hdr)
        + 2 + getQosEntryNum()    * sizeof(T_CpeToSCfgEle)
        + 2 + getNeighborBtsNum() * sizeof(UINT32);
    if (WITH_PROFILEDATA == ptr->hdr.ProfileDataFlag)
        len += ptr->UTProfile.length();

    //û�м���repeater�ĳ���
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
