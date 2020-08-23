/*******************************************************************************
* Copyright (c) 2009 by AP Co.Ltd.All Rights Reserved   
* File Name      : localSagCfg.h
* Create Date    : 5-Nov-2009
* programmer     :
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__LOCALSAGCFG_H
#define	__LOCALSAGCFG_H

#include "localSagDialPlan.h"

#define M_MAX_AREACODE_LENGTH	(9)

#ifndef M_LOCALSAG_CFG_VALID_FLAG
#define M_LOCALSAG_CFG_VALID_FLAG 	(0xA7B92E18)
#endif
typedef struct __LocalSagCfgT
{
	char blUseLocalSag;
	char blUseUserListFile;//故障弱化时是否使用用户列表限制用户接入使用，非故障弱化时依赖核心网
	char AreaCode[M_MAX_AREACODE_LENGTH+1];
	DialPlanTblT DialPlanTbl;
	char maxGrpIdleTime[2];//组呼空闲时长	2	M		默认值5s，0表示不使用
	char maxGrpTalkTime[2];//讲话方最大时长	2	M		默认值5s，0表示不使用
	char maxGrpAliveTime[2];//组呼最大时长	2	M		默认值0，0表示不使用
	char GrpLePagingLoopTime[2];//迟后进入周期	2	M		默认值5s，0表示不使用
	char blUseLocalUserInfoFile;//是否使用内部用户信息文件定义的用户信息
	char blUseLocalGrpInfoFile;//是否使用内部组信息文件定义的组信息
	char validFlag[4];//muset be M_LOCALSAG_CFG_VALID_FLAG after configuation
}LocalSagCfgT;

extern bool g_blUseLocalSag;
extern bool g_blUseUserListFile;
extern DialPlanT dialPlan_Para;
extern char g_AreaCode[M_MAX_AREACODE_LENGTH+1];
extern UINT16 g_maxGrpIdleTime;//组呼空闲时长	2	M		默认值5s，0表示不使用
extern UINT16 g_maxGrpTalkTime;//讲话方最大时长	2	M		默认值5s，0表示不使用
extern UINT16 g_maxGrpAliveTime;//组呼最大时长	2	M		默认值0，0表示不使用
extern UINT16 g_GrpLePagingLoopTime;//迟后进入周期	2	M		默认值5s，0表示不使用
extern bool g_blUseLocalGrpInfoFile;//是否使用内部组信息文件定义的组信息
extern bool g_blUseLocalUserInfoFile;//是否使用内部用户信息文件定义的用户信息


#ifdef __cplusplus
extern "C" {
#endif

void updateLocalSagCfg(bool blUpdateVSrvInfoFromCfgFile);
char* getAreaCode();
void setUseUserInfoFileFlag(char flag);
void setUseGrpInfoFileFlag(char flag);
void setLocalSagGrpLePagingLoopTime(UINT16 seconds);
void setLocalSagGrpMaxIdleTime(UINT16 seconds);
void setLocalSagGrpMaxTalkTime(UINT16 seconds);
void setLocalSagGrpMaxAliveTime(UINT16 seconds);
void showDialPlan();
void showLocalSagCfg();

#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAGCFG_H */


