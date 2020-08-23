/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localDialPlan.h
* Create Date    : 27-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__LOCALDIALPLAN_H
#define	__LOCALDIALPLAN_H

#define M_MAX_PREFIX_LEN (10)
#define M_MAX_PREFIX_SIZE (5)
#define M_MAX_DIALPLAN_COUNT (20)

typedef struct __DialPlanItemT
{
	unsigned char Len;
	char PrefixNumber[M_MAX_PREFIX_LEN+1];
}DialPlanItemT;

typedef struct __DialPlanT
{
	unsigned short Count;
	DialPlanItemT lst_DialPlanItems[M_MAX_DIALPLAN_COUNT];
}DialPlanT;

typedef struct __DialPlanRecordT
{
	unsigned char PrefixNumber[M_MAX_PREFIX_SIZE];
	unsigned char Len;
}DialPlanRecordT;

typedef struct __DialPlanTblT
{
	DialPlanRecordT DialPlanTbl[M_MAX_DIALPLAN_COUNT];
}DialPlanTblT;

enum
{
	DIAL_CONTINUE=0,
	DIAL_COMPLETE,
	DIAL_VALID,
	DIAL_ERROR
};

int buildDialPlanTree(DialPlanT* pDialPlan);
int parseNumberDialed(unsigned char bLength, 
					  char *pDgt, 
					  unsigned char &bFetId, 
					  unsigned char &bAddDgts);

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LOCALDIALPLAN_H */




