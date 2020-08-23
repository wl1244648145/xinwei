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
 *   10/22/2007   肖卫方       Initial file creation.
 *---------------------------------------------------------------------------*/
#include <string.h>

#include "L3OamAuth.h"
#include "L3OamCfgCommon.h"

extern T_NvRamData *NvRamDataAddr;
extern "C" int bspGetBtsID();
/*
========================================
        message         evtID       grpID
Authentication_Info Req 0x01        0x05
AuthenticationCommand   0x02        0x05
Authentication_Rsp      0x03        0x05
Authentication_Result   0x04        0x05
BWInfo_Req              0x05        0x05
BWInfo_Rsp              0x06        0x05
Delete BWInfo_Req       0x07        0x05
Modify_BWInfo_Req       0x08        0x05
Modify_BWInfo_Rsp       0x09        0x05
=========================================
*/

#define M_BTS_MASK (0x0FFF)
void CsabisAuthenticationInfoReq::setInfo(UINT32 ulUID, UINT32 ulPID, UINT32 ulMZPID )
{
    UINT8 *ptr = (UINT8*)GetDataPtr();
    ptr -= 2;
    SetDataPtr(ptr);
    T_sabisAuthenticationInfoReq *pAuthReq = (T_sabisAuthenticationInfoReq*)ptr;
    pAuthReq->ulSagID       = NvRamDataAddr->BtsGDataCfgEle.SAGID;
    pAuthReq->usBtsID       = M_BTS_MASK & bspGetBtsID();
    pAuthReq->ucEvtGrpID    = 0x05;
    pAuthReq->usEvtID       = 0x01;
    if( 0 == ulMZPID )
        pAuthReq->usLength      = sizeof(T_sabisAuthenticationInfoReq) - ((UINT32)&(pAuthReq->ulUID) - (UINT32)pAuthReq) - 4;
    else
        pAuthReq->usLength      = sizeof(T_sabisAuthenticationInfoReq) - ((UINT32)&(pAuthReq->ulUID) - (UINT32)pAuthReq);
    pAuthReq->ulUID         = ulUID;
    pAuthReq->ulPID         = ulPID;
#ifdef MZ_2ND
    pAuthReq->ulMZPid       = ulMZPID;
#endif
////pAuthReq->ulCID         = ulCID;
////memcpy(pAuthReq->ucLAI, &NvRamDataAddr->BtsGDataCfgEle.LocAreaID, M_LAI_LENGTH);
}


void CUTAuthenticationCMD::setInfo(UINT32 ulUID, UINT32 ulPID, const UINT8 *pRand)
{
    if(NULL == pRand)
        return;
    T_UTAuthenticationCMD *pCMD = (T_UTAuthenticationCMD*)GetDataPtr();
    pCMD->ulUID = ulUID;
    pCMD->ulPID = ulPID;
#ifdef MZ_2ND
    memcpy(pCMD->rand, pRand, sizeof(pCMD->rand)+4/*MZPid*/);
#else
    memcpy(pCMD->rand, pRand, sizeof(pCMD->rand));
#endif
}

void CsabisAuthenticationRsp::setInfo(UINT32 ulUID, UINT32 ulPID, UINT32 ulResult)
{
    UINT8 *ptr = (UINT8*)GetDataPtr();
    ptr -= 2;
    SetDataPtr(ptr);
    T_sabisAuthenticationRsp *pCMD = (T_sabisAuthenticationRsp*)ptr;
    pCMD->ulSagID       = NvRamDataAddr->BtsGDataCfgEle.SAGID;
    pCMD->usBtsID       = M_BTS_MASK & bspGetBtsID();
    pCMD->ucEvtGrpID    = 0x05;
    pCMD->usEvtID       = 0x03;
    pCMD->usLength      = sizeof(T_sabisAuthenticationRsp) - ((UINT32)&(pCMD->ulUID) - (UINT32)pCMD);
    pCMD->ulUID         = ulUID;
    pCMD->ulPID         = ulPID;
    pCMD->ulResult      = ulResult;
}

void CUTAuthenticationResult::setInfo(UINT32 ulUID, UINT32 ulPID, UINT8 result)
{
    T_UTAuthenticationResult *pCMD = (T_UTAuthenticationResult*)GetDataPtr();
#ifdef MZ_2ND
    memset( (UINT8*)pCMD, 0, sizeof(T_UTAuthenticationResult) );
#endif
    pCMD->ulUID         = ulUID;
    pCMD->ulPID         = ulPID;
    pCMD->Auth_result   = result;
    pCMD->Ind       = 0;
}
void CUTAuthenticationResult::setInfo(UINT32 ulUID, UINT32 ulPID, UINT8 auth_result, UINT8 ind , UINT8 result)
{
    T_UTAuthenticationResult *pCMD = (T_UTAuthenticationResult*)GetDataPtr();
#ifdef MZ_2ND
    memset( (UINT8*)pCMD, 0, sizeof(T_UTAuthenticationResult) );
#endif
    pCMD->ulUID         = ulUID;
    pCMD->ulPID         = ulPID;
    pCMD->Auth_result   = auth_result;
    pCMD->Ind       = ind;
    pCMD->result = result;
}
#ifdef MZ_2ND
void CUTAuthenticationResult:: setInfo(UINT32 ulUID, UINT32 ulPID ,UINT8 auth_result ,UINT8 ind , UINT8 result ,const UINT8* SID,const UINT8* CID)
{
	T_UTAuthenticationResult *pCMD = (T_UTAuthenticationResult*)GetDataPtr();
	pCMD->ulUID         = ulUID;
	pCMD->ulPID         = ulPID;
	pCMD->Auth_result   = auth_result;
	pCMD->Ind     = ind;
	if((ind == 1)||(ind == 2))
	{
		pCMD->result = result;
		if(result ==0)
		{
			if(SID!=NULL)
			{
				memcpy(pCMD->SID,SID,16);
			}
			else
			{
				memset(pCMD->SID,0,16);
			}
			if(CID!=NULL)
			{
				memcpy(pCMD->CID,CID,3);
			}
			else
			{
				memset(pCMD->CID,0,3);
			}
            memcpy(pCMD->ulMZPid,CID+3,4);/*MZPid*/
		}
		else
		{
			memset(pCMD->SID,0,16);
			memset(pCMD->CID,0,3);
            memcpy(pCMD->ulMZPid,CID+3,4);/*MZPid*/
		}
	}
	else
	{
//    	printf( "\r\nauth->MZPID[%02X][%02X][%02X][%02X]\r\n", \
//    		*(SID), *(SID+1), *(SID+2), *(SID+3) );
		pCMD->Ind   = 0;
		pCMD->result = 0xff;
    	memset(pCMD->SID,0,16);
		memset(pCMD->CID,0,3);
        memcpy(pCMD->ulMZPid,SID,4);/*MZPid ind==0时，sid送入的是mz的pid*/

	}
}
#else
void CUTAuthenticationResult:: setInfo(UINT32 ulUID, UINT32 ulPID ,UINT8 auth_result ,UINT8 ind , UINT8 result ,const UINT8* SID,const UINT8* CID)
{
	T_UTAuthenticationResult *pCMD = (T_UTAuthenticationResult*)GetDataPtr();
	pCMD->ulUID         = ulUID;
	pCMD->ulPID         = ulPID;
	pCMD->Auth_result   = auth_result;
	pCMD->Ind     = ind;
	if((ind == 1)||(ind == 2))
	{
		pCMD->result = result;
		if(result ==0)
		{
			if(SID!=NULL)
			{
				memcpy(pCMD->SID,SID,16);
			}
			else
			{
				memset(pCMD->SID,0,16);
			}
			if(CID!=NULL)
			{
				memcpy(pCMD->CID,CID,3);
			}
			else
			{
				memset(pCMD->CID,0,3);
			}
		}
		else
		{

			memset(pCMD->SID,0,16);

			memset(pCMD->CID,0,3);
		}
	}
	else
	{
		pCMD->Ind   = 0;
		pCMD->result = 0xff;

		memset(pCMD->SID,0,16);

		memset(pCMD->CID,0,3);


	}
}
#endif
//该消息是为了机卡分离用户作的,需要携带NickName在cid前，18个字节
/*
UINT32 ulUID;
        UINT32 ulPID;
        UINT8  Auth_result;
	 UINT8  Ind;
	 UINT8 result;   
	 UINT8 nickName[18];
	 UINT8 CID[3];
	 UINT8 SID[16];
*/
void CUTAuthenticationResult:: setInfo(UINT32 ulUID, UINT32 ulPID, UINT8 auth_result, UINT8 ind , UINT8 *pData, UINT16 len)
{
    T_UTAuthenticationResult *pCMD = (T_UTAuthenticationResult*)GetDataPtr();
    pCMD->ulUID         = ulUID;
    pCMD->ulPID         = ulPID;
    pCMD->Auth_result   = auth_result;
    pCMD->Ind     = ind;
    memcpy((UINT8*)((UINT8*)&pCMD->Ind +1), pData, len);	 
}
void CsabisBWInfoReq::setInfo(UINT32 ulUID, UINT32 ulPID, const T_UTBaseInfo &UTBaseInfo)
{
    UINT8 *ptr = (UINT8*)GetDataPtr();
    ptr -= 2;
    SetDataPtr(ptr);
    T_sabisBWInfoReq *pBWInfoReq = (T_sabisBWInfoReq*)ptr;
    pBWInfoReq->ulSagID     = NvRamDataAddr->BtsGDataCfgEle.SAGID;
    pBWInfoReq->usBtsID     = M_BTS_MASK & bspGetBtsID();
    pBWInfoReq->ucEvtGrpID  = 0x05;
    pBWInfoReq->usEvtID     = 0x05;
    pBWInfoReq->usLength    = sizeof(T_sabisBWInfoReq) - ((UINT32)&(pBWInfoReq->ulUID) - (UINT32)pBWInfoReq);
    pBWInfoReq->ulUID       = ulUID;
    pBWInfoReq->ulPID       = ulPID;
    memcpy(&pBWInfoReq->UTBaseInfo, &UTBaseInfo, sizeof(UTBaseInfo));
////memcpy(pBWInfoReq->ucLAI, &NvRamDataAddr->BtsGDataCfgEle.LocAreaID, M_LAI_LENGTH);
}

void CsabisModifyBWInfoRsp::setInfo(UINT32 ulUID, UINT8 ucFlag)
{
    UINT8 *ptr = (UINT8*)GetDataPtr();
    ptr -= 2;
    SetDataPtr(ptr);
    T_sabisModBWInfoRsp *pModifyBWInfoRsp = (T_sabisModBWInfoRsp*)ptr;
    pModifyBWInfoRsp->ulSagID     = NvRamDataAddr->BtsGDataCfgEle.SAGID;
    pModifyBWInfoRsp->usBtsID     = M_BTS_MASK & bspGetBtsID();
    pModifyBWInfoRsp->ucEvtGrpID  = 0x05;
    pModifyBWInfoRsp->usEvtID     = 0x09;
    pModifyBWInfoRsp->usLength    = sizeof(T_sabisModBWInfoRsp) - ((UINT32)&(pModifyBWInfoRsp->ulUID) - (UINT32)pModifyBWInfoRsp);
    pModifyBWInfoRsp->ulUID       = ulUID;
    pModifyBWInfoRsp->ucFlag      = ucFlag;
}

bool CUTRegisterReq::isValid() const
{
    const T_RegisterReq* const pRegReq = (const T_RegisterReq* const)GetDataPtr();
    const UINT8 *ptr = (const UINT8*)&(pRegReq->UTProfile);
    UINT32 length    = pRegReq->UTProfile.length();
    for (UINT32 idx  = 0; idx < length; ++idx)
        {
        if (0x0 != *(ptr++))
            return true;
        }
    //全00
    return false;
}

void CUTRegisterReq::getSpecialFlag(T_L2SpecialFlag& specialFlag)const 
{
    T_UTProfile*profile = &(((T_RegisterReq*)GetDataPtr())->UTProfile);
    UINT16 *pMagic = (UINT16 *)((UINT8*)profile + profile->length());
    if((pMagic[0] == 0x55aa)&&(pMagic[1] == 0x55aa))
        {
        T_L2SpecialFlag *pL2Flag = (T_L2SpecialFlag*)(pMagic+2);
        specialFlag.L2_Special_Flag1 = pL2Flag->L2_Special_Flag1;
        specialFlag.L2_Special_Flag2 = pL2Flag->L2_Special_Flag2;
        specialFlag.L2_Special_Flag3 = pL2Flag->L2_Special_Flag3;
        specialFlag.L2_Special_Flag4 = pL2Flag->L2_Special_Flag4;
	 memcpy(specialFlag.RFprofile, pL2Flag->RFprofile, 12);
        }
    else
        {

        specialFlag.L2_Special_Flag1 = 0;
        specialFlag.L2_Special_Flag2 = 0;
        specialFlag.L2_Special_Flag3 = 0;
        specialFlag.L2_Special_Flag4 = 0;
	 memset(specialFlag.RFprofile, 0, 12);
        }
    return;
}
//wangwenhua add 20080716
 UINT32 CUTRegisterReq::getBTSID()const
{
   T_UTProfile *profile = &(((T_RegisterReq*)GetDataPtr())->UTProfile);
   UINT32 *BTSID = (UINT32*)((UINT8*)profile + profile->length() + 4 + sizeof(T_L2SpecialFlag));
   return *BTSID;
}


void   CUTRegisterReq::getUTHold(T_UT_HOLD_BW& bw) const
{
       T_UTProfile *profile = &(((T_RegisterReq*)GetDataPtr())->UTProfile);
	 //if(1 ==(profile->UTProfileIEBase.UTSDCfgInfo.Reserved&0x01))//有效的情况下//保持带宽无条件取，桂林机场问题,jy20110802
	 {
	   UINT8 *p = (UINT8 *)((UINT8*)profile + profile->length() + 4 + sizeof(T_L2SpecialFlag) +4 );//44 -magic length,4,-btsid
	     memcpy((SINT8*)&bw,(SINT8*)p,sizeof(bw));
	 }
	 /*else
	 {
	     memset((SINT8*)&bw, 0, sizeof(bw));
	 }*/
	 return;
}
void   CUTRegisterReq::getMemCfg(T_MEM_CFG& mem_cfg) const
{ 
	 T_UTProfile *profile = &(((T_RegisterReq*)GetDataPtr())->UTProfile);
	 int length = sizeof(T_UTBaseInfo) + sizeof(T_L2SpecialFlag) + profile->length() \
	 	+sizeof(T_UT_HOLD_BW) + 20;
	 int lastlength = GetDataLength() - length;	 	 
	 if(lastlength>0)
   	 {
   	       UINT8 *p = (UINT8 *)((UINT8*)GetDataPtr() + length);//44 -magic length,4,-btsid ,bw
	 	memcpy((SINT8*)&mem_cfg,(SINT8*)p,sizeof(T_MEM_CFG));
 	 }
	 else
	 {
	     memset((SINT8*)&mem_cfg, 0, sizeof(T_MEM_CFG));
	 }
	 return;
}
bool CUTRegisterReq::isWCPEorRCPE() const
{
    UINT8 flag;
    T_UTProfile *profile = &(((T_RegisterReq*)GetDataPtr())->UTProfile);
    flag = profile->UTProfileIEBase.UTSDCfgInfo.Reserved;
#ifdef RCPE_SWITCH
    if( (flag&0x06)==4 || (flag&0x06)==6 )
#else
    if((flag&0x06)==4)
#endif
	return true;
    else
	return false;
}
void  CsabisSwitchOFFNotifyReq::setInfo(UINT32 UID)
{
     UINT8 *ptr = (UINT8*)GetDataPtr();
    ptr -= 2;//为什么偏移2个字节呢澹?
    SetDataPtr(ptr);
    T_sabisSwitchOFFNotifyReq *pSwitchOffReq = (T_sabisSwitchOFFNotifyReq*)ptr;
    pSwitchOffReq->ulSagID     = NvRamDataAddr->BtsGDataCfgEle.SAGID;
    pSwitchOffReq->usBtsID     = M_BTS_MASK & bspGetBtsID();
    pSwitchOffReq->ucEvtGrpID  = 0x05;
    pSwitchOffReq->usEvtID     = 0x0a;
    pSwitchOffReq->usLength    = 4;// sizeof(T_sabisSwitchOFFNotifyReq) - ((UINT32)&(pBWInfoReq->ulUID) - (UINT32)pBWInfoReq);
    pSwitchOffReq->ulUID       = UID;
   // pBWInfoReq->ulPID       = ulPID;
   // memcpy(&pBWInfoReq->UTBaseInfo, &UTBaseInfo, sizeof(UTBaseInfo));
}
   const T_UT_HOLD_BW* CsabisBWInfoRsp::getHoldBW()const 
   {
	const T_sabisBWInfoRsp* const pBWInfoRsp  = (const T_sabisBWInfoRsp * const)GetDataPtr();
     return  (const T_UT_HOLD_BW * const) ((char *)(&(pBWInfoRsp->UTProfile) )+ (pBWInfoRsp->UTProfile.length()));
   }


     const T_UT_HOLD_BW*   CsabisModifyBWInfoReq::getHoldBW()const 
     {
         const T_sabisModBWInfoReq* const pBWInfoRsp  = (const T_sabisModBWInfoReq * const)GetDataPtr();
     return  (const T_UT_HOLD_BW * const) ((char *)(&(pBWInfoRsp->UTProfile) )+ (pBWInfoRsp->UTProfile.length()));
     }
T_sabisMemCfg* CsabisBWInfoRsp::getMemCfg()
{
	const T_sabisBWInfoRsp* const pBWInfoRsp  = (const T_sabisBWInfoRsp * const)GetDataPtr();
     return  (T_sabisMemCfg * ) ((char *)(&(pBWInfoRsp->UTProfile) )+ (pBWInfoRsp->UTProfile.length()) + sizeof(T_UT_HOLD_BW));
}
T_sabisMemCfg* CsabisModifyBWInfoReq::getMemCfg()
{
	const T_sabisModBWInfoReq* const pBWInfoRsp  = (const T_sabisModBWInfoReq * const)GetDataPtr();
     return  (T_sabisMemCfg * ) ((char *)(&(pBWInfoRsp->UTProfile) )+ (pBWInfoRsp->UTProfile.length()) + sizeof(T_UT_HOLD_BW));
}

void CsabisAccountLoginReq::setInfo(const UINT8* pdata, UINT16 len)
{
    UINT8 *ptr = (UINT8*)GetDataPtr();
    ptr -= 2;
    SetDataPtr(ptr);
    T_sabisAccountLoginReq *pAuthReq = (T_sabisAccountLoginReq*)ptr;
    pAuthReq->ulSagID       = NvRamDataAddr->BtsGDataCfgEle.SAGID;
    pAuthReq->usBtsID       = M_BTS_MASK & bspGetBtsID();
    pAuthReq->ucEvtGrpID    = 0x05;
    pAuthReq->usEvtID       = 0x0b;    
    pAuthReq->usLength = len;    
    memcpy((UINT8*)&pAuthReq->ActiveType, pdata, len);
    
    return;	
}
