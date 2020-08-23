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
	char blUseUserListFile;//��������ʱ�Ƿ�ʹ���û��б������û�����ʹ�ã��ǹ�������ʱ����������
	char AreaCode[M_MAX_AREACODE_LENGTH+1];
	DialPlanTblT DialPlanTbl;
	char maxGrpIdleTime[2];//�������ʱ��	2	M		Ĭ��ֵ5s��0��ʾ��ʹ��
	char maxGrpTalkTime[2];//���������ʱ��	2	M		Ĭ��ֵ5s��0��ʾ��ʹ��
	char maxGrpAliveTime[2];//������ʱ��	2	M		Ĭ��ֵ0��0��ʾ��ʹ��
	char GrpLePagingLoopTime[2];//�ٺ��������	2	M		Ĭ��ֵ5s��0��ʾ��ʹ��
	char blUseLocalUserInfoFile;//�Ƿ�ʹ���ڲ��û���Ϣ�ļ�������û���Ϣ
	char blUseLocalGrpInfoFile;//�Ƿ�ʹ���ڲ�����Ϣ�ļ����������Ϣ
	char validFlag[4];//muset be M_LOCALSAG_CFG_VALID_FLAG after configuation
}LocalSagCfgT;

extern bool g_blUseLocalSag;
extern bool g_blUseUserListFile;
extern DialPlanT dialPlan_Para;
extern char g_AreaCode[M_MAX_AREACODE_LENGTH+1];
extern UINT16 g_maxGrpIdleTime;//�������ʱ��	2	M		Ĭ��ֵ5s��0��ʾ��ʹ��
extern UINT16 g_maxGrpTalkTime;//���������ʱ��	2	M		Ĭ��ֵ5s��0��ʾ��ʹ��
extern UINT16 g_maxGrpAliveTime;//������ʱ��	2	M		Ĭ��ֵ0��0��ʾ��ʹ��
extern UINT16 g_GrpLePagingLoopTime;//�ٺ��������	2	M		Ĭ��ֵ5s��0��ʾ��ʹ��
extern bool g_blUseLocalGrpInfoFile;//�Ƿ�ʹ���ڲ�����Ϣ�ļ����������Ϣ
extern bool g_blUseLocalUserInfoFile;//�Ƿ�ʹ���ڲ��û���Ϣ�ļ�������û���Ϣ


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


