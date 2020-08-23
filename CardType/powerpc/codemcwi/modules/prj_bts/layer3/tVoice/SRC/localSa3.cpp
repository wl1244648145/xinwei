/*******************************************************************************
* Copyright (c) 2009 by AP Co.Ltd.All Rights Reserved   
* File Name      : localDialPlan.cpp
* Create Date    : 27-Oct-2009
* programmer     :
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "stdio.h"
#include "string.h"
#include "voiceCommon.h"
#include "localSagDialPlan.h"

//#define M_SELFTEST

#ifndef UINT8
#define UINT8 unsigned char
#endif
#ifndef UINT16
#define UINT16 unsigned short
#endif

#define MAX_DIAL_PLAN_NODE		200

//���������
typedef	struct
{
	UINT16	wNodeId[12];// ��һ���������(1-MAX_DIAL_PLAN_NODE) 0=�Ƿ�
	UINT8	bFetId;		// ��������
	UINT8	bAddDgts;	// ʣ��ĺ�����
}translateNodeT;

// ��żƻ��ڵ����� 
translateNodeT transTbl[MAX_DIAL_PLAN_NODE];

int buildDialPlanTree(DialPlanT* pDialPlan)
{
	UINT8 bCode;
	int nItem;	        // ���żƻ�/�ط��ż���
	int nChar;	        // ���żƻ��ֹ�/�ط�������ַ�����
	int nFreeNode=1;	// ���б�ڵ�����
	int nCurNode=0;

	// ��ʼ����ṹ
	memset(&transTbl, 0, sizeof(transTbl));

	// �����żƻ� 
	for(nItem = 0; nItem < pDialPlan->Count; nItem++)
	{
		nCurNode = 0;
		for(nChar = 0; nChar < M_MAX_PREFIX_LEN; nChar++)
		{
			//bCode = boardDB.systemCfg.aNumPlan[nItem].bCode[nChar];
			bCode = pDialPlan->lst_DialPlanItems[nItem].PrefixNumber[nChar];
			if(bCode >= '0' && bCode <= '9')	// '0'->'9'
			{
				// ����wNodeId 
				bCode -= '0';
				// �Ƶ���һ��ڵ㣬������һ���½ڵ� 
				if(transTbl[nCurNode].wNodeId[bCode] > 0 && 
					transTbl[nCurNode].wNodeId[bCode] < MAX_DIAL_PLAN_NODE)	// �ѷ���
				{
					nCurNode = transTbl[nCurNode].wNodeId[bCode];
				}
				else 
				{
					if(nFreeNode < MAX_DIAL_PLAN_NODE)    // ����һ���½ڵ�
					{
						transTbl[nCurNode].wNodeId[bCode] = nFreeNode;
						nCurNode = nFreeNode;
						nFreeNode++;
					}
					else	// �޿��нڵ�
					{
						VPRINT("\nDialPlan tree node overflow!!!\n");
						return nFreeNode;
					}
				}
				
				//if(nChar >= 10)	// ��10�㣬���ù�����
				if(nChar == (M_MAX_PREFIX_LEN-1))	// ��10�㣬���ù�����
				{
					transTbl[nCurNode].bFetId = DIAL_VALID;
					transTbl[nCurNode].bAddDgts = pDialPlan->lst_DialPlanItems[nItem].Len  - 10;
					break;
				}
			}
			else 
			{
				if(nChar != 0) // �ַ������������ù�����
				{
					transTbl[nCurNode].bFetId = DIAL_VALID;
					//transTbl[nCurNode].bAddDgts = boardDB.systemCfg.aNumPlan[nItem].bLength - nChar;	// exclude prefix
					transTbl[nCurNode].bAddDgts = pDialPlan->lst_DialPlanItems[nItem].Len - nChar;	// exclude prefix
					break;
				}
				else 	// ��һ���ַ��ǿգ�����
				{
					break;
				}
			}
		}
	}
	return nFreeNode;
}
UINT8 getDigitCode(char digit)
{
	UINT8 bCode = digit;
	if(bCode >= '0' && bCode <= '9')	// '0'->'9'
	{
		bCode -= '0';
	}
	else
	{
		if(bCode=='*')
		{
			bCode=10;
		}
		else
		{
			if(bCode=='#')
			{
				bCode=11;
			}
			else
			{
				bCode=0xff;
			}
		}
	}
	return bCode;
}
int parseNumberDialed(UINT8 bLength, 
					  char *pDgt, 
					  UINT8 &bFetId, 
					  UINT8 &bAddDgts)
{
	UINT8 bCode, bCodeNext;
	UINT8 i, h;
	UINT16 wNextNode, wCurNode=0;
	UINT8 cmpLen;
	bAddDgts = 0;
	bFetId = DIAL_CONTINUE;

	cmpLen = (bLength > M_MAX_PREFIX_LEN) ? M_MAX_PREFIX_LEN : bLength;

	for(i=0; i< cmpLen; i++)
	{
		// ����һλ���� 
		bCode = getDigitCode(pDgt[i]);
		if(bCode > 11)	              // �������
		{
			bFetId = DIAL_ERROR;      // ������Ч������
			break;
		}
		wCurNode = transTbl[wCurNode].wNodeId[bCode];
		if(wCurNode == 0)
		{
			bFetId = DIAL_ERROR;//0;
			break;
		}
		if(wCurNode >= MAX_DIAL_PLAN_NODE)
		{
			bFetId = DIAL_ERROR;//0;
			break;
		}
		if(transTbl[wCurNode].bFetId)	// ���������ϣ���ȡ������
		{
			//�жϺ��������Ƿ��и���ȷ�Ĳ��żƻ�
			bCodeNext = getDigitCode(pDgt[i+1]);
			if(bCodeNext<=11)
			{
				wNextNode = transTbl[wCurNode].wNodeId[bCodeNext];
				if(wNextNode>0 && wNextNode<MAX_DIAL_PLAN_NODE)
				{
					//�и���ȷ�Ĳ��żƻ�
					continue;
				}
			}
			
			bFetId = transTbl[wCurNode].bFetId;
			h = bLength - (i+1);//û���õĸ���
			if( h >= transTbl[wCurNode].bAddDgts )
				 bAddDgts = 0;
			else
				 bAddDgts = transTbl[wCurNode].bAddDgts - h;

			break;
		}
	}
	
	if(DIAL_VALID==bFetId && 0==bAddDgts)
	{
		bFetId = DIAL_COMPLETE;
	}
#ifdef M_SELFTEST
	char szResult[20];
	strcpy(szResult, "Invalid value!");
	switch(bFetId)
	{
		case DIAL_CONTINUE:
			strcpy(szResult, "go on dialing");
			break;
		case DIAL_COMPLETE:
			strcpy(szResult, "dial complete");
			break;
		case DIAL_VALID:
			strcpy(szResult, "dial valid");
			break;
		case DIAL_ERROR:
			strcpy(szResult, "wrong number!");
			break;
		default:
			strcpy(szResult, "Invalid value!");
	}
	VPRINT("\n NumberDialed[%s] Result[%s] DigitsWanted[%d] \n",
		pDgt, szResult, bAddDgts);
#endif //M_SELFTEST
	return bFetId;
}
#ifdef M_SELFTEST
int main()
{
	int i;
	VPRINT("\nsizeof(translateNodeT)=%d  \n", 
		sizeof(translateNodeT));
//#define M_USE_NEWMEM
#ifdef M_USE_NEWMEM	
	UINT16 nParaSize = (sizeof(DialPlanItemT)*20 + sizeof(UINT16));
	void *pPara = new char[nParaSize];
#else
	DialPlanT dialPlan_Para;
	void *pPara = (void*)&dialPlan_Para;
	UINT16 nParaSize = sizeof(DialPlanT);
#endif	
	memset(pPara, 0, nParaSize);
	DialPlanT* pDialPlan = (DialPlanT*)pPara;
	i=0;
	pDialPlan->lst_DialPlanItems[i].Len=8;
	strncpy(pDialPlan->lst_DialPlanItems[i++].PrefixNumber,"01",M_MAX_PREFIX_LEN);
	pDialPlan->lst_DialPlanItems[i].Len=7;
	strncpy(pDialPlan->lst_DialPlanItems[i++].PrefixNumber,"02",M_MAX_PREFIX_LEN);
	pDialPlan->lst_DialPlanItems[i].Len=8;
	strncpy(pDialPlan->lst_DialPlanItems[i++].PrefixNumber,"0",M_MAX_PREFIX_LEN);
	pDialPlan->lst_DialPlanItems[i].Len=11;
	strncpy(pDialPlan->lst_DialPlanItems[i++].PrefixNumber,"136",M_MAX_PREFIX_LEN);
	pDialPlan->lst_DialPlanItems[i].Len=11;
	strncpy(pDialPlan->lst_DialPlanItems[i++].PrefixNumber,"1391039868",M_MAX_PREFIX_LEN);
	pDialPlan->Count=i;

	buildDialPlanTree(pDialPlan);

	UINT8 bFetId;
	UINT8 bAddDgts;	
	parseNumberDialed(strlen("13910398687"), "13910398687", bFetId, bAddDgts);
	parseNumberDialed(strlen("13691004296"), "13691004296", bFetId, bAddDgts);
	parseNumberDialed(strlen("13910398"), "13910398", bFetId, bAddDgts);
	parseNumberDialed(strlen("1369"), "1369", bFetId, bAddDgts);
	parseNumberDialed(strlen("01666666"), "01666666", bFetId, bAddDgts);
	parseNumberDialed(strlen("0255555"), "0255555", bFetId, bAddDgts);
	parseNumberDialed(strlen("02555"), "02555", bFetId, bAddDgts);
	parseNumberDialed(strlen("04355555"), "04355555", bFetId, bAddDgts);
	parseNumberDialed(strlen("0434444"), "0434444", bFetId, bAddDgts);
	parseNumberDialed(strlen("7"), "7", bFetId, bAddDgts);
	parseNumberDialed(strlen("1391039867"), "1391039867", bFetId, bAddDgts);
	parseNumberDialed(strlen("0188888888"), "0188888888", bFetId, bAddDgts);
#ifdef M_USE_NEWMEM
	delete [] pPara;
#endif
	return 0;
}
#endif //M_SELFTEST



