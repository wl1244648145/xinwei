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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMPROFUPDATEREQ
#include "L3OamCpeProfUpdateReq.h"
#endif
#include <string.h>


bool CCpeProfUpdateReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_UT_PROFILE_UPDATE_REQ);
    return true;
}

UINT32 CCpeProfUpdateReq :: GetDefaultDataLen() const
{
    return MAX_MESSAGE_SIZE;
}

UINT16 CCpeProfUpdateReq :: GetTransactionId()const
{
    return ((T_ProfUpReq *)GetDataPtr())->TransId;
}

UINT16 CCpeProfUpdateReq :: SetTransactionId(UINT16 T)
{
    ((T_ProfUpReq*)GetDataPtr())->TransId = T;
	return 0;
}

UINT32 CCpeProfUpdateReq ::GetCPEID()const
{
    return ((T_ProfUpReq *)GetDataPtr())->CPEID;
}


void  CCpeProfUpdateReq :: SetCPEID(UINT32 T)
{
    ((T_ProfUpReq*)GetDataPtr())->CPEID = T;
}
#if 0
UINT8  CCpeProfUpdateReq :: GetAdminStatus()const
{
    return ((T_ProfUpReq*)GetDataPtr())->UTProfile.UTProfileIEBase.AdminStatus;
}

void  CCpeProfUpdateReq ::  SetAdminStatus(UINT8 E) 
{
    ((T_ProfUpReq*)GetDataPtr())->UTProfile.UTProfileIEBase.AdminStatus = E;
}

UINT8  CCpeProfUpdateReq :: GetLogStatus()const 
{
    return ((T_ProfUpReq*)GetDataPtr())->UTProfile.UTProfileIEBase.LogStatus;
}

void   CCpeProfUpdateReq :: SetLogStatus(UINT8 E) 
{
    ((T_ProfUpReq*)GetDataPtr())->UTProfile.UTProfileIEBase.LogStatus = E;
}

UINT16 CCpeProfUpdateReq :: GetDataCInter()const 
{
    return ((T_ProfUpReq*)GetDataPtr())->UTProfile.UTProfileIEBase.DataCInter;
}

void  CCpeProfUpdateReq ::  SetDataCInter(UINT16 E) 
{
    ((T_ProfUpReq*)GetDataPtr())->UTProfile.UTProfileIEBase.DataCInter = E;
}

UINT8  CCpeProfUpdateReq :: GetMobility() const
{
    return ((T_ProfUpReq*)GetDataPtr())->UTProfile.UTProfileIEBase.Mobility;
}

void   CCpeProfUpdateReq :: SetMobility(UINT8 Mobility)
{
    (((T_ProfUpReq*)GetDataPtr())->UTProfile.UTProfileIEBase).Mobility = Mobility;
}

UINT8  CCpeProfUpdateReq :: GetDHCPRenew() const
{
    return (((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).DHCPRenew;
}

void  CCpeProfUpdateReq :: SetDHCPRenew(UINT8 DHCPRenew)
{
    (((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).DHCPRenew = DHCPRenew;
}

UINT8 CCpeProfUpdateReq :: GetMaxIpNum() const
{
    return (((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).MaxIpNum;
}

void  CCpeProfUpdateReq ::  SetMaxIpNum(UINT8 MaxIpNum)
{
    (((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).MaxIpNum = MaxIpNum;
}

bool   CCpeProfUpdateReq :: GetUTSerDisIE(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL ==  DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (UINT8*)&((((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).UTSDCfgInfo),//((UINT8*)GetDataPtr() + (SINT32)OFFSET_UTSERVDISIE)
               Len);
        return true;
    }
}

bool   CCpeProfUpdateReq :: SetUTSerDisIE(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL ==  SrcBuff)
    {
        return false; 
    }
    else
    {
        memcpy((UINT8*)&((((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).UTSDCfgInfo),//((UINT8*)GetDataPtr() + (SINT32)OFFSET_UTSERVDISIE)
               SrcBuff, 
               Len);
        return true;
    }
}

UINT8 CCpeProfUpdateReq :: GetFixIpNum() const
{
    return (((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).FixIpNum;
}

void  CCpeProfUpdateReq ::  SetFixIpNum(UINT8 FixIpNum)
{
    (((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).FixIpNum = FixIpNum;
}

bool   CCpeProfUpdateReq :: GetCpeFixIpInfo(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL ==  DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (SINT8*)&(((T_ProfUpReq*)GetDataPtr())->CpeFixIpInfo),
               Len);
        return true;
    }
}

bool   CCpeProfUpdateReq :: SetCpeFixIpInfo(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL ==  SrcBuff)
    {
        return false; 
    }
    else
    {
        memcpy((SINT8*)&(((T_ProfUpReq*)GetDataPtr())->CpeFixIpInfo), 
                SrcBuff, 
                Len);
        return true;
    }
}

#if 0
bool  CCpeProfUpdateReq ::  GetUTProfIE(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL ==  DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (SINT8*)&((((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).AdminStatus),
               Len);
        return true;
    }
}

bool CCpeProfUpdateReq ::   SetUTProfIE(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL ==  SrcBuff)
    {
        return false; 
    }
    else
    {
        memcpy((SINT8*)&((((T_ProfUpReq*)GetDataPtr())->UTProfileIEBase).AdminStatus), 
                SrcBuff, 
                Len);
        return true;
    }
}
#endif

bool   CCpeProfUpdateReq :: GetRegData(SINT8* DstBuff, UINT16 Len)const 
{
    if(NULL ==  DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               (SINT8*)&(((T_ProfUpReq*)GetDataPtr())->CPEID),
               Len);
        return true;
    }
}

bool   CCpeProfUpdateReq :: SetRegData(SINT8* SrcBuff, UINT16 Len) 
{
    if(NULL ==  SrcBuff)
    {
        return false; 
    }
    else
    {
        memcpy((SINT8*)&(((T_ProfUpReq*)GetDataPtr())->CPEID), 
                SrcBuff, 
                Len);
        return true;
    }
}
UINT16 CCpeProfUpdateReq :: GetUTProfIESize()
{
     return DEF_PROFILE_IE_SIZE + GetFixIpNum() * sizeof(T_CpeFixIpInfo);
}
#endif
const T_UTProfile* CCpeProfUpdateReq::getProfile()const 
{
    return &(((T_ProfUpReq*)GetDataPtr())->UTProfile);
}
#if 0
  const T_UT_HOLD_BW* CCpeProfUpdateReq::getHoldBW()const //wangwenhua add 20080916
  {
  	 int i = 0;
	 int len = 0;
	 char *p = NULL;
        T_UTProfile  *profile = &(((T_ProfUpReq*)GetDataPtr())->UTProfile) ;
		if((profile->UTProfileIEBase.UTSDCfgInfo.Reserved )== 1)
			{
	  	#if 0
			     //lijinan 20080923 for test
			     p = (char *)profile;
			     len =  profile->length();
				 printf("\n");
			   //  for(i=0;i<len;i++)
			 //     printf("ems-->bts:%x ",*(p+i));
			     for(i=0;i<4;i++)
			    printf("ems-->bts:%x",*(p+len+i));
			      printf("bw p:%x\n",(profile + profile->length()));
	
		        p = (char *)(profile + profile->length());
		       printf("len:%d\n",profile->length());
		        for(i=0;i<4;i++)
		        {
			    printf("ems-->bts:%x",*(p+i));
		        }
		#endif
	                    return (T_UT_HOLD_BW*)((char *)profile + profile->length());
			}
		else
			return NULL;
  }
  #endif
   const T_UT_HOLD_BW* CCpeProfUpdateReq::getHoldBW()const //wangwenhua add 20080916
  {
//  	 int i = 0;
//	 int len = 0;
//	 char *p = NULL;
        T_UTProfile  *profile = &(((T_ProfUpReq*)GetDataPtr())->UTProfile) ;
		//if((profile->UTProfileIEBase.UTSDCfgInfo.Reserved &0x01)== 1)//保持带宽无条件取，桂林机场问题,jy20110802
			{
	  	#if 0
			     //lijinan 20080923 for test
			     p = (char *)profile;
			     len =  profile->length();
				 printf("\n");
			   //  for(i=0;i<len;i++)
			 //     printf("ems-->bts:%x ",*(p+i));
			     for(i=0;i<4;i++)
			    printf("ems-->bts:%x",*(p+len+i));
			      printf("bw p:%x\n",(profile + profile->length()));
	
		        p = (char *)(profile + profile->length());
		       printf("len:%d\n",profile->length());
		        for(i=0;i<4;i++)
		        {
			    printf("ems-->bts:%x",*(p+i));
		        }
		#endif
	                    return (T_UT_HOLD_BW*)((char *)profile + profile->length());
			}
		//else
			//return NULL;
  }

