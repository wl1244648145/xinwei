/*******************************************************************************
* Copyright (c) 2009 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagCfg.cpp
* Create Date    : 5-Nov-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include <stdio.h>
#include "string.h"
#include "localSag.h"
#include "voiceCommon.h"
#include "l3OamCfgCommon.h"
#include "localSagCfg.h"
#include "voiceToolFunc.h"
#include "localSagTimer.h"

extern T_NvRamData *NvRamDataAddr;
extern "C" void setLocalSagEnableFlag(char flag);
extern "C" void setUseUserListFileFlag(char flag);


bool g_blUseLocalSag=false;
bool g_blUseUserListFile=false;
DialPlanT dialPlan_Para;
char g_AreaCode[M_MAX_AREACODE_LENGTH+1]="";
UINT16 g_maxGrpIdleTime=5;//组呼空闲时长	2	M		默认值5s，0表示不使用
UINT16 g_maxGrpTalkTime=0;//讲话方最大时长	2	M		默认值5s，0表示不使用
UINT16 g_maxGrpAliveTime=0;//组呼最大时长	2	M		默认值0，0表示不使用
UINT16 g_GrpLePagingLoopTime=5;//迟后进入周期	2	M		默认值5s，0表示不使用
bool g_blUseLocalGrpInfoFile=false;//是否使用内部组信息文件定义的组信息
bool g_blUseLocalUserInfoFile=false;//是否使用内部用户信息文件定义的用户信息

void setUseUserInfoFileFlag(char flag)
{
	g_blUseLocalUserInfoFile = flag;
}
void setUseGrpInfoFileFlag(char flag)
{
	g_blUseLocalGrpInfoFile = flag;
}
void setLocalSagGrpLePagingLoopTime(UINT16 seconds)
{
	g_GrpLePagingLoopTime = seconds;
}
void setLocalSagGrpMaxIdleTime(UINT16 seconds)
{
	if(0==seconds)
	{
		seconds=5;//禁止配0
	}
	sagTimerCfgTbl[TIMERID_SAG_GRP_MAX_IDLE_TIME].timeoutVal = SAG_ONE_SECOND*seconds;
	g_maxGrpIdleTime = seconds;
}
void setLocalSagGrpMaxTalkTime(UINT16 seconds)
{
	sagTimerCfgTbl[TIMERID_SAG_GRP_MAX_TALKING_TIME].timeoutVal = SAG_ONE_SECOND*seconds;
	g_maxGrpTalkTime = seconds;
}
void setLocalSagGrpMaxAliveTime(UINT16 seconds)
{
	sagTimerCfgTbl[TIMERID_SAG_GRP_TTL].timeoutVal = SAG_ONE_SECOND*seconds;
	g_maxGrpAliveTime = seconds;
}

void convertDialPlan(DialPlanTblT *pDialPlanCfg, DialPlanT *pPara)
{
	char dictionary[]="D1234567890*#ABC";
	UINT32 i, j, len;
	char code;
	char buf[60];
	bool blIsValidRecord = false;
	DialPlanRecordT *pRecord;
	DialPlanItemT *pItem;
	if(pDialPlanCfg==NULL || pPara==NULL)
	{
		return;
	}
	pPara->Count = 0;
	for(j=0;j<M_MAX_DIALPLAN_COUNT;j++)
	{
		blIsValidRecord = false;
		pRecord = &pDialPlanCfg->DialPlanTbl[j];
		
		if(pRecord->Len>0)
		{
			len = 0;
			for(i=0;i<sizeof(pRecord->PrefixNumber);i++)
			{
				code = pRecord->PrefixNumber[i] & 0x0F;
				if(code>=0x01 && code<=0x0C)
				{
					buf[len++] = dictionary[(UINT8)code];
				}
				else
				{
					if(code==0x0F)
					{
						//end flag
						break;
					}
					else
					{
						//end flag
						break;
					}
				}
				code = (pRecord->PrefixNumber[i] & 0xF0)>>4;
				if(code>=0x01 && code<=0x0C)
				{
					buf[len++] = dictionary[(UINT8)code];
				}
				else
				{
					if(code==0x0F)
					{
						//end flag
						break;
					}
					else
					{
						//end flag
						break;
					}
				}			
			}
			buf[len] = 0;//end of string
			blIsValidRecord = (pRecord->Len>=len) ? true : false;
		}
		else
		{
			blIsValidRecord = false;
		}
		//合法的拨号计划
		if(blIsValidRecord)
		{
			pItem = &pPara->lst_DialPlanItems[pPara->Count];
			strcpy(pItem->PrefixNumber, buf);
			pItem->Len = pRecord->Len;
			++pPara->Count;
		}
	}

}
extern "C" void cleanLocalSagCfg();
void updateLocalSagCfg(bool blUpdateVSrvInfoFromCfgFile)
{
	int i;
	//从存储位置更新配置
	LocalSagCfgT	*pSagCfg = (LocalSagCfgT*)&NvRamDataAddr->localSagCfg;
	if(M_LOCALSAG_CFG_VALID_FLAG==VGetU32BitVal((UINT8*)pSagCfg->validFlag))
	{
		//是否使用SAG
		setLocalSagEnableFlag(pSagCfg->blUseLocalSag);
		//是否使用用户列表文件
		setUseUserListFileFlag(pSagCfg->blUseUserListFile);
		//本地区号
		memset(g_AreaCode, 0, sizeof(g_AreaCode));
		for(i=0;i<M_MAX_AREACODE_LENGTH;i++)
		{
			if(pSagCfg->AreaCode[i]>='0' && pSagCfg->AreaCode[i]<='9')
			{
				g_AreaCode[i] = pSagCfg->AreaCode[i];
			}
			else
			{
				g_AreaCode[i] = 0;
				break;
			}
		}
		//拨号计划
		memset((void*)&dialPlan_Para, 0, sizeof(DialPlanT));
		convertDialPlan(&pSagCfg->DialPlanTbl, &dialPlan_Para);
		buildDialPlanTree(&dialPlan_Para);
		//localSag集群相关配置
		setUseGrpInfoFileFlag(pSagCfg->blUseLocalGrpInfoFile);
		setUseUserInfoFileFlag(pSagCfg->blUseLocalUserInfoFile);
		setLocalSagGrpLePagingLoopTime(VGetU16BitVal((unsigned char*)pSagCfg->GrpLePagingLoopTime));
		setLocalSagGrpMaxIdleTime(VGetU16BitVal((unsigned char*)pSagCfg->maxGrpIdleTime));
		setLocalSagGrpMaxTalkTime(VGetU16BitVal((unsigned char*)pSagCfg->maxGrpTalkTime));
		setLocalSagGrpMaxAliveTime(VGetU16BitVal((unsigned char*)pSagCfg->maxGrpAliveTime));
	}
	else
	{
		cleanLocalSagCfg();
	}
	//根据用户和组业务信息配置文件生成用户和组业务信息
	if(blUpdateVSrvInfoFromCfgFile)
	{
		loadSrvCfgInfoFromFile();
	}
}

char* getAreaCode()
{
	return g_AreaCode;
}

void showDialPlan()
{
	int i;
	if(M_MAX_DIALPLAN_COUNT<dialPlan_Para.Count)
	{
		VPRINT("\nToo many DialPlan Items!!! The count is %d now, should be no more than %d. Error!!!\n",
			dialPlan_Para.Count, M_MAX_DIALPLAN_COUNT);
		return;
	}
	VPRINT("\n----------Local SAG Dial Plan----------");
	for(i=0;i<dialPlan_Para.Count;i++)
	{
		VPRINT("\n (%04d) Prefix[%10s] Length[%d]", 
			i+1, 
			dialPlan_Para.lst_DialPlanItems[i].PrefixNumber, 
			dialPlan_Para.lst_DialPlanItems[i].Len);
	}
	VPRINT("\n------------------------------------\n");
}

void showLocalSagCfg()
{

	VPRINT("\nUsing LocalSag:[%s]", g_blUseLocalSag ? "YES" : "NO");
	VPRINT("\nUsing UserListFile:[%s]", g_blUseUserListFile ? "YES" : "NO");
	VPRINT("\nArea Code:[%s]", g_AreaCode);
	showDialPlan();
	VPRINT("\nUsing UserInfoFile:[%s]", g_blUseLocalUserInfoFile ? "YES" : "NO");
	VPRINT("\nUsing GrpInfoFile:[%s]", g_blUseLocalGrpInfoFile ? "YES" : "NO");
	VPRINT("\nLeGrpPaging Period:[%d seconds]", g_GrpLePagingLoopTime);
	VPRINT("\nMaxGrpIdle time:[%d seconds]", g_maxGrpIdleTime);
	VPRINT("\nMaxGrpTalk time:[%d seconds]", g_maxGrpTalkTime);
	VPRINT("\nMaxGrpAlive time:[%d seconds]", g_maxGrpAliveTime);
}

