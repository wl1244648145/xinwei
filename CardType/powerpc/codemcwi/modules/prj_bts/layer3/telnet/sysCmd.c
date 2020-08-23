/*-----------------------------------------------------------
    sysCmd.c -  ϵͳģ��������c�ļ�

    ��Ȩ���� 2004 -2006 ������˾������BSC��Ŀ��. 


    �޸���ʷ��¼
    --------------------
    20.00.01,       12-28-2004,      ���༽      ����.   
-----------------------------------------------------------*/
#ifdef  __cplusplus
extern  "C"
{
#endif


/*
*ͷ�ļ�����
*/
#include "sysos.h"
#include "cliConfig.h"
#include "cliShell.h"

#include "sysCmd.h"
//#include "sysPrint.h"

/*
*�궨��
*/
#define THIS_FILE_ID FILE_SYN_C
#define MAX_TRACE_DIRE_NUM      10          //������Ϣ���ٷ�����Ŀ
#define MAX_FID 4
#define MAX_MOD_ID  8
/*
*�ⲿ��������
*/
//extern FID_PRINT_RECORD_S  g_stFidPrintCB[ MAX_FID ];                   //ģ���ӡ���ƿ�
//extern MSG_TRACE_DIRE_CFG  g_stMsgTraceDireCfg[MAX_TRACE_DIRE_NUM];     //[��Ϣ���ٿ���]���ƿ�
extern _UCHAR              g_ucModMsgRedireCtrl[MAX_MOD_ID];            //ģ��֮�����Ϣ�ض������([SWITCH_OFF]Ϊ�رգ�[SWITCH_ON]Ϊ��)
extern _UCHAR              g_ucFidMsgRedireCtrl[MAX_FID];               //���ܿ�֮�����Ϣ�ض������([SWITCH_OFF]Ϊ�رգ�[SWITCH_ON]Ϊ��)
extern _UCHAR              g_ucMsgTraceSwtich;                          //[��Ϣ���ٿ���]�ܿ���
extern _UCHAR              g_ucDataSynClass;                            //����ͬ������Ĭ��ֻ������̬����

/*----------------------------------------------------------------
  ˵��:[sysSymTbl]ԭ�������� SYMTAB_ID, ����һ��ָ��;
       ���Ҫ��������SYMTAB_ID,������ܶ��ͬ�ļ���������,
       ���,�����Ϊ _ULONG �͵�ָ�룬��Ȼָ�����Ͳ�ͬ�����������
----------------------------------------------------------------*/
//extern SYMTAB_ID  sysSymTbl;  /* system symbol table */
//extern _ULONG   *sysSymTbl;


/*
*�ⲿ��������
*/
//extern _UINT SYS_NowDataTimeGet(SYS_DATE_TIME_STRU *pstNowTime);
//extern _UINT SYS_SystemStartDataTimeGet(SYS_DATE_TIME_STRU *pstTime);

/*
*ȫ�ֱ�������
*/


/*
*���غ�������
*/
/*---------------------------------------------------------------------
    printopenall    - �����д�ӡ��������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID printopenall(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
 //   SYS_PrintOpenAll(pCliEnv);
	printf("\n\r printfopenall called!");
}

/*---------------------------------------------------------------------
    printcloseall    - �ر����д�ӡ��������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID printcloseall(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
 //   SYS_PrintCloseAll(pCliEnv);
 	printf("\n\r printcloseall called!");
}

/*---------------------------------------------------------------------
    printopenone    - ��һ�����ܿ�Ĵ�ӡ��������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID printopenone(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _ULONG      ulFid; 
//    PRINT_LEVEL ePrintLevel;
//
//    if(3 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n���������������");
//        return;
//    }
//
//    ulFid       = SYS_ATOL(ppArgv[1]);
//    ePrintLevel = SYS_ATOL(ppArgv[2]);
//
//    if((ulFid >= MAX_FID) || (ePrintLevel >= MAX_PL))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n�������ֵ����");
//        return;
//    }
//    
//    SYS_PrintOpenOne(pCliEnv, ulFid, ePrintLevel);

    return;
}

/*---------------------------------------------------------------------
    printcloseone    - �ر�һ�����ܿ�Ĵ�ӡ��������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID printcloseone(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _ULONG      ulFid; 
//
//    if(2 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n���������������");
//        return;
//    }
//
//    ulFid       = SYS_ATOL(ppArgv[1]);
//
//    if(ulFid >= MAX_FID)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n�������ֵ����");
//        return;
//    }
//    
//    SYS_PrintCloseOne(pCliEnv, ulFid);

    return;
}

/*---------------------------------------------------------------------
    printopenfileline    - �򿪴�ӡ�ļ������кŵĴ�ӡ��������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID printopenfileline(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_PrintOpenFileLine(pCliEnv);
}

/*---------------------------------------------------------------------
    printclosefileline    - �رմ�ӡ�ļ������кŵĴ�ӡ��������
    

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID printclosefileline(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_PrintCloseFileLine(pCliEnv);
}

/*---------------------------------------------------------------------
    printshow    - ��ʾ���й��ܿ�Ĵ�ӡ��������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID printshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_PrintShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    taskinfoshow    - ��ʾ����������Ϣ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID taskinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_AllRegTaskInfoShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    fidinfoshow    - ��ʾ���й��ܿ���Ϣ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID fidinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
 //   SYS_AllRegFidInfoShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    ipcfginfoshow    - ��ʾIP�����������Ϣ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID ipcfginfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_AllIPCfgInfoShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    msgstatinfoshow    - ��ʾ��Ϣͳ����Ϣ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID msgstatinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_MsgStatShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    msgtraceopen    - ��Ϣ���ٴ�����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID msgtraceopen(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    if(SUCC == SYS_MsgTraceSwitchOpen())
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ�����Ѵ�");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ���ٴ�ʧ��");
//    }

    return;
}

/*---------------------------------------------------------------------
    msgtraceclose    - ��Ϣ���ٹر�����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID msgtraceclose(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//   if(SUCC == SYS_MsgTraceSwitchClose())
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ�����ѹر�");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ���ٹر�ʧ��");
//    }

    return;
}

/*---------------------------------------------------------------------
    msgtracedireadd    - �����Ϣ���ٷ�������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID msgtracedireadd(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR ucMod1, ucFid1, ucMod2, ucFid2;
//    
//
//    if(5 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n���������������");
//        return;
//    }
//
//    ucMod1  = (_UCHAR)SYS_ATOL(ppArgv[1]);
//    ucFid1  = (_UCHAR)SYS_ATOL(ppArgv[2]);
//    ucMod2  = (_UCHAR)SYS_ATOL(ppArgv[3]);
//    ucFid2  = (_UCHAR)SYS_ATOL(ppArgv[4]);
//
//    if(SUCC == SYS_MsgTraceDireAdd(ucMod1, ucFid1, ucMod2, ucFid2))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ���ٷ�����ӳɹ�.");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ���ٷ������ʧ��");
//    }

    return;
}

/*---------------------------------------------------------------------
    msgtracedireadd    - ɾ����Ϣ���ٷ�������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID msgtracediredel(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR ucIndex;
//    
//
//    if(2 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n���������������");
//        return;
//    }
//
//    ucIndex  = (_UCHAR)SYS_ATOL(ppArgv[1]);
//
//    if(SUCC == SYS_MsgTraceDireDel(ucIndex))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ���ٷ���ɾ���ɹ�.");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ���ٷ���ɾ��ʧ��");
//    }

    return;
}

/*---------------------------------------------------------------------
    msgtraceshow    - ��Ϣ���ٿ��غͷ�����ʾ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID msgtraceshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR  i;
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n-------------------------------------------------------------");
//
//    CLI_EXT_Printf(pCliEnv, "\r\n��Ϣ���ٿ���״̬: ");
//
//    if(SWITCH_ON == g_ucMsgTraceSwtich)
//    {
//        CLI_EXT_Printf(pCliEnv, "ON");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "OFF");
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n%-8s%-8s%-10s%-8s%-10s", "INDEX", "MOD1", "FID1", "MOD2", "FID2");
//
//    for(i = 0; i < MAX_TRACE_DIRE_NUM; i++)
//    {
//        if(MAX_MOD_ID != g_stMsgTraceDireCfg[i].ucMod1)
//        {
//            if((g_stMsgTraceDireCfg[i].ucFid1 >= MAX_FID) 
//                || (g_stMsgTraceDireCfg[i].ucFid2 >= MAX_FID))
//            {
//                CLI_EXT_Printf(pCliEnv, "\r\n%-8d%-8d%-10d%-8d%-10d",
//                    i,
//                    g_stMsgTraceDireCfg[i].ucMod1,
//                    g_stMsgTraceDireCfg[i].ucFid1,
//                    g_stMsgTraceDireCfg[i].ucMod2,
//                    g_stMsgTraceDireCfg[i].ucFid2);
//            }
//            else if((0 == g_stFidPrintCB[g_stMsgTraceDireCfg[i].ucFid1].ucFidName[0])
//                && (0 == g_stFidPrintCB[g_stMsgTraceDireCfg[i].ucFid2].ucFidName[0]))
//            {
//                CLI_EXT_Printf(pCliEnv, "\r\n%-8d%-8d%-10d%-8d%-10d",
//                    i,
//                    g_stMsgTraceDireCfg[i].ucMod1,
//                    g_stMsgTraceDireCfg[i].ucFid1,
//                    g_stMsgTraceDireCfg[i].ucMod2,
//                    g_stMsgTraceDireCfg[i].ucFid2);
//            }
//            else if(0 == g_stFidPrintCB[g_stMsgTraceDireCfg[i].ucFid1].ucFidName[0])
//            {
//                CLI_EXT_Printf(pCliEnv, "\r\n%-8d%-8d%-10d%-8d%-10s",
//                    i,
//                    g_stMsgTraceDireCfg[i].ucMod1,
//                    g_stMsgTraceDireCfg[i].ucFid1,
//                    g_stMsgTraceDireCfg[i].ucMod2,
//                    g_stFidPrintCB[g_stMsgTraceDireCfg[i].ucFid2].ucFidName);
//            }
//            else if(0 == g_stFidPrintCB[g_stMsgTraceDireCfg[i].ucFid2].ucFidName[0])
//            {
//                CLI_EXT_Printf(pCliEnv, "\r\n%-8d%-8d%-10s%-8d%-10d",
//                    i,
//                    g_stMsgTraceDireCfg[i].ucMod1,
//                    g_stFidPrintCB[g_stMsgTraceDireCfg[i].ucFid1].ucFidName,
//                    g_stMsgTraceDireCfg[i].ucMod2,
//                    g_stMsgTraceDireCfg[i].ucFid2);
//            }
//            else
//            {
//                CLI_EXT_Printf(pCliEnv, "\r\n%-8d%-8d%-10s%-8d%-10s",
//                    i,
//                    g_stMsgTraceDireCfg[i].ucMod1,
//                    g_stFidPrintCB[g_stMsgTraceDireCfg[i].ucFid1].ucFidName,
//                    g_stMsgTraceDireCfg[i].ucMod2,
//                    g_stFidPrintCB[g_stMsgTraceDireCfg[i].ucFid2].ucFidName);
//            }
//        }
//    }    
//
//    CLI_EXT_Printf(pCliEnv, "\r\n-------------------------------------------------------------");
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n");

    return;
}

/*---------------------------------------------------------------------
    modmsgredire    - ģ����Ϣ�ض�������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID modmsgredirecfg(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR  ucModId, ucCtrl;
//    _CHAR   *cStr[2] = {"�ر�", "��"};
//
//    if(3 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n���������������");
//        return;
//    }
//
//    ucModId = (_UCHAR)SYS_ATOI(ppArgv[1]);
//    ucCtrl  = (_UCHAR)SYS_ATOI(ppArgv[2]);
//
//    if(ucModId >= MAX_MOD_ID)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n����ģ��Ų��ԣ���������ģ�����: %d", MAX_MOD_ID);
//        return;
//    }
//
//    if((SWITCH_ON != ucCtrl) && (SWITCH_OFF != ucCtrl))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��������ֲ��ԣ���: %d, �ر�: %d", SWITCH_ON, SWITCH_OFF);
//        return;
//    }
//
//    if(SUCC == SYS_ModMsgRedireCfg(ucModId, ucCtrl))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n%sģ�� [%d] ��Ϣת�ӳɹ�", cStr[ucCtrl], ucModId);
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n%sģ�� [%d] ��Ϣת��ʧ��", cStr[ucCtrl], ucModId);
//    }

    return;
}

/*---------------------------------------------------------------------
    fidmsgredire    - ���ܿ���Ϣ�ض�������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID fidmsgredirecfg(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR  ucFid, ucCtrl;
//    _CHAR   *cStr[2] = {"�ر�", "��"};
//
//    if(3 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n���������������");
//        return;
//    }
//
//    ucFid = (_UCHAR)SYS_ATOI(ppArgv[1]);
//    ucCtrl  = (_UCHAR)SYS_ATOI(ppArgv[2]);
//
//    if(ucFid >= MAX_FID)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n���빦�ܿ�Ų��ԣ��������Ĺ��ܿ����: %d", MAX_FID);
//        return;
//    }
//
//    if((SWITCH_ON != ucCtrl) && (SWITCH_OFF != ucCtrl))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��������ֲ��ԣ���: %d, �ر�: %d", SWITCH_ON, SWITCH_OFF);
//        return;
//    }
//
//    if(SUCC == SYS_FidMsgRedireCfg(ucFid, ucCtrl))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n%s���ܿ� [%s] ��Ϣת�ӳɹ�", cStr[ucCtrl], g_stFidPrintCB[ucFid].ucFidName);
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n%s���ܿ� [%s] ��Ϣת��ʧ��", cStr[ucCtrl], g_stFidPrintCB[ucFid].ucFidName);
//    }
//
    return;
}


/*---------------------------------------------------------------------
    msgredireshow    - ��Ϣ�ض���������ʾ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID msgredireshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR  i;
//    _CHAR   *cStr[2] = {"OFF", "ON"};
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n-------------------------------------------------------------");
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\t%-20s%-20s", "MODID", "SWITCH STATE");
//
//    for(i = 0; i < MAX_MOD_ID; i++)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n\t%-20d%-20s", i, cStr[g_ucModMsgRedireCtrl[i]]);
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n                      - - - - - -");
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n\t%-20s%-20s%-20s", "FID", "FID NAME", "SWITCH STATE");
//
//    for(i = 0; i < MAX_FID; i++)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n\t%-20d%-20s%-20s", i, g_stFidPrintCB[i].ucFidName, cStr[g_ucFidMsgRedireCtrl[i]]);
//    }
//    
//    CLI_EXT_Printf(pCliEnv, "\r\n-------------------------------------------------------------");
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n");

    return;
}

/*---------------------------------------------------------------------
    msgqnumshow    - �����������Ϣ���е���Ϣ������ʾ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID msgqnumshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _ULONG  i;
//
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n-------------------------------------------------------------");
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\t%-10s%-10s%-10s%-13s", "TaskNo.", "TaskName", "MsgNum", "MaxMsgAlloc");
//
//    for(i = 0; i < MAX_TASK_NUM; i++)
//    {
//        if(FLAG_YES == stSysData.stTaskInfo[i].ucHaveMsgQFlg)
//        {
//#ifdef WINDOWS
//            CLI_EXT_Printf(pCliEnv, "\r\n\t%-10d%-10s%-10d%-13d", 
//                i,
//                stSysData.stTaskInfo[i].cTaskName, 
//                0, 
//                stSysData.stTaskInfo[i].maxMsgs);
//#endif
//
//#ifdef VXWORKS
//            CLI_EXT_Printf(pCliEnv, "\r\n\t%-10d%-10s%-10d%-13d", 
//                i,
//                stSysData.stTaskInfo[i].cTaskName, 
//                msgQNumMsgs(stSysData.stTaskInfo[i].qMsgQId), 
//                stSysData.stTaskInfo[i].maxMsgs);
//#endif                
//        }
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\n-------------------------------------------------------------");
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n");

    return;
}


    /*==================================================*            
     *  ��VxWorks�����£�ע�� i, ti, d ����
     *==================================================*/
#ifdef VXWORKS

#define MAX_SYS_SYM_LEN 256 /* system symbols will not exceed this limit */
#define MAX_DSP_TASKS   500 /* max tasks that can be displayed */
/*---------------------------------------------------------------------
    debugprintTaskInfo    - VxWorks����ʾĳһ������Ϣ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID debugprintTaskInfo(CLI_ENV *pCliEnv, TASK_DESC *pTd )
{
//    char        tmp[256];
//    FUNCPTR     entry;                      /* task's initial entry point */
//    FUNCPTR     symboladdr;                 /* address associated with 'name' */
//    SYM_TYPE    type;                       /* symbol type */
//    char        name[MAX_SYS_SYM_LEN + 1];  /* main routine names go here */
//    char        demangled[MAX_SYS_SYM_LEN + 1];
//    char        *nameToPrint;
//    char        task_status[100];
//    REG_SET     Regs;
//
//    /*find task's initial entry point, and name (if any) in symbol table*/
//
//    entry = pTd->td_entry;
//
//    symFindByValueAndType (sysSymTbl, (int)entry, name, (int *)&symboladdr,
//               &type, N_EXT | N_TEXT, N_EXT | N_TEXT);
//
//    /* print the name of the task */
//    sprintf (tmp,"%-12.12s", pTd->td_name); 
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    /* entry address (symbolic if poss.) */
//    if(entry == symboladdr)     
//    {
//        nameToPrint = (char *)cplusDemangle (name, demangled, MAX_SYS_SYM_LEN + 1);
//        sprintf (tmp," %-12.12s", nameToPrint);
//        CLI_EXT_Printf(pCliEnv, tmp );
//    }
//    else
//    {
//        sprintf (tmp," %#-12.12x", (int) entry);
//        CLI_EXT_Printf(pCliEnv, tmp );
//    }
//
//    sprintf ( tmp , " %8x " ,pTd->td_id );
//    CLI_EXT_Printf(pCliEnv, tmp );
//    
//    sprintf ( tmp , " %3d " ,pTd->td_priority );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    taskStatusString ( pTd->td_id , task_status );
//    sprintf ( tmp , " %-8.8s" ,task_status );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    taskRegsGet (pTd->td_id , &Regs );
//    sprintf ( tmp , " %8x" ,Regs.pc);
//    CLI_EXT_Printf(pCliEnv, tmp );
//    
//    sprintf ( tmp , " %8x " ,pTd->td_errorStatus );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , " %8d " ,pTd->td_delay );
//    CLI_EXT_Printf(pCliEnv, tmp );

    return;
}


/*---------------------------------------------------------------------
    debugprintTaskRegs    - VxWorks����ʾ������Ϣ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID debugprintTaskRegs(CLI_ENV *pCliEnv, TASK_DESC *pTd )
{
//    char    tmp[256];
//    REG_SET Regs;
//
//    sprintf( tmp, "STACK  :\r\n       base   : 0x%x \r\n       end    : 0x%x \r\n       size   : %d \r\n       high   : %d \r\n       margin : %d\r\n" ,
//        pTd->td_pStackBase, 
//        pTd->td_pStackEnd, 
//        pTd->td_stackSize,
//        pTd->td_stackHigh, 
//        pTd->td_stackMargin );
//        
//    CLI_EXT_Printf(pCliEnv, tmp );
//    CLI_EXT_Printf(pCliEnv, "\r\n");
//
//    sprintf( tmp, "OPTIONS:  0x%x\r\n" , pTd->td_options );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    memset( tmp , 0 , sizeof ( tmp ) );
//    
//    if( pTd->td_options & 0x0001 )
//    {
//        strcat( tmp, "VX_SUPERVISOR_MODE " );
//    }
//    
//    if ( pTd->td_options & 0x0002 )
//        strcat( tmp, "VX_UNBREAKABLE " );
//        
//    if ( pTd->td_options & 0x0004 )
//        strcat( tmp, "VX_DEALLOC_STACK " );
//        
//    if ( pTd->td_options & 0x0008 )
//        strcat( tmp, "VX_FP_TASK " );
//        
//    if ( pTd->td_options & 0x0010 )
//        strcat( tmp, "VX_STDIO " );
//        
//    if ( pTd->td_options & 0x0020 )
//        strcat( tmp, "VX_ADA_DEBUG " );
//        
//    if ( pTd->td_options & 0x0040 )
//        strcat( tmp, "VX_FORTRAN " );
//        
//    if ( pTd->td_options & 0x0080 )
//        strcat( tmp, "VX_PRIVATE_ENV " );
//        
//    if ( pTd->td_options & 0x0100 )
//        strcat( tmp, "VX_NO_STACK_FILL " );
//        
//    CLI_EXT_Printf(pCliEnv, tmp );
//    CLI_EXT_Printf(pCliEnv, "\r\n");
//    CLI_EXT_Printf(pCliEnv, "\r\n");
//    
//    taskRegsGet(pTd->td_id , &Regs );
//    sprintf( tmp , "r0    = %8x    r1/sp = %8x    r2    = %8x    r3    = %8x\r\n",
//                   Regs.gpr[0] , Regs.gpr[1] , Regs.gpr[2] , Regs.gpr[3] );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "r4    = %8x    r5    = %8x    r6    = %8x    r7    = %8x\r\n",
//                   Regs.gpr[4] , Regs.gpr[5] , Regs.gpr[6] , Regs.gpr[7] );
//
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "r8    = %8x    r9    = %8x    r10   = %8x    r11   = %8x\r\n",
//                   Regs.gpr[8] , Regs.gpr[9] , Regs.gpr[10] , Regs.gpr[11] );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "r12   = %8x    r13   = %8x    r14   = %8x    r15   = %8x\r\n",
//                   Regs.gpr[12] , Regs.gpr[13] , Regs.gpr[14] , Regs.gpr[15] );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "r16   = %8x    r17   = %8x    r18   = %8x    r19   = %8x\r\n",
//                   Regs.gpr[16] , Regs.gpr[17] , Regs.gpr[18] , Regs.gpr[19] );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "r20   = %8x    r21   = %8x    r22   = %8x    r23   = %8x\r\n",
//                   Regs.gpr[20] , Regs.gpr[21] , Regs.gpr[22] , Regs.gpr[23] );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "r24   = %8x    r25   = %8x    r26   = %8x    r27   = %8x\r\n",
//                   Regs.gpr[24] , Regs.gpr[25] , Regs.gpr[26] , Regs.gpr[27] );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "r28   = %8x    r29   = %8x    r30   = %8x    r31   = %8x\r\n",
//                   Regs.gpr[28] , Regs.gpr[29] , Regs.gpr[30] , Regs.gpr[31] );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "msr   = %8x    lr    = %8x    ctr   = %8x    pc    = %8x\r\n",
//                   Regs.msr , Regs.lr , Regs.ctr , Regs.pc );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    sprintf ( tmp , "cr    = %8x    xer   = %8x    mq    = %8x\r\n",
//                   Regs.cr , Regs.xer , 0 );
//    CLI_EXT_Printf(pCliEnv, tmp );
//
//    CLI_EXT_Printf(pCliEnv, "\r\n" );
//
    return;
}

/*---------------------------------------------------------------------
    i    - VxWorks����ʾ������Ϣ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID si(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    FAST int        nTasks;                 /* number of task */
//    FAST int        ix;                     /* index */
//    int             tid;                    /* task ID */
//    TASK_DESC       td;                     /* task info structure */
//    int             idList [MAX_DSP_TASKS]; /* list of active IDs */
//    _ULONG          taskId  = 0;
//
//    /*��ʾ����������Ϣ*/
//
//    CLI_EXT_Printf(pCliEnv, "\r\n" );
//    CLI_EXT_Printf(pCliEnv, "  NAME        ENTRY        TID     PRI  STATUS       PC       ERRNO      DELAY\r\n");
//    CLI_EXT_Printf(pCliEnv, "------------ ------------ -------- -------   ----- ----- ----------------------\r\n");
//
//    nTasks = taskIdListGet(idList, NELEMENTS(idList));
//    
//    taskIdListSort(idList, nTasks);
//
//    for (ix = 0; ix < nTasks; ++ix)
//    {
//        if(OK == taskInfoGet(idList[ix], &td))
//        {
//            debugprintTaskInfo(pCliEnv, &td);
//            
//            CLI_EXT_Printf(pCliEnv, "\r\n");
//        }
//    }
//
    return;
}


/*---------------------------------------------------------------------
    ti    - VxWorks����ʾ������ƿ����Ϣ����

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID sti(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//     FAST int       nTasks;                     /* number of task */
//     FAST int       ix;                         /* index */
//     int            tid;                        /* task ID */
//     TASK_DESC      td;                         /* task info structure */
//     int            idList[MAX_DSP_TASKS];      /* list of active IDs */
//     ulong_t        taskId  = 0;
//     
//    if(2 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv,"\r\nû���������");
//        return;
//    }
//    
//    taskId = SYS_ATOL(ppArgv[1]);
//    
//    if(taskId != 0)
//    {
//        /* do specified task */
//
//        tid = taskIdFigure (taskId);
//
//        if((ERROR == tid) || (OK != taskInfoGet (tid, &td)))
//        {
//            CLI_EXT_Printf(pCliEnv, "Task not found.\r\n");
//        }
//        else
//        {
//            CLI_EXT_Printf(pCliEnv, "\r\n" );
//            CLI_EXT_Printf(pCliEnv, "  NAME        ENTRY        TID     PRI  STATUS       PC       ERRNO      DELAY\r\n");
//            CLI_EXT_Printf(pCliEnv, "------------ ------------ -------- -------   ----- ----- ------------------\r\n");
//
//            debugprintTaskInfo(pCliEnv, &td);
//
//            CLI_EXT_Printf(pCliEnv, "\r\n" );
//            CLI_EXT_Printf(pCliEnv, "\r\n" );
//
//            debugprintTaskRegs(pCliEnv, &td);
//        }
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n�����������ȷ" );
//    }
//     
    return;
}

#endif

/*---------------------------------------------------------------------
    systimeshow    - ��ʾ��ǰ���ڡ�ʱ���ϵͳ�������ڡ�ʱ������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID systimeshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_DATE_TIME_STRU stTmpTime;
//
//    if(SUCC != SYS_NowDataTimeGet(&stTmpTime))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��ȡ��ǰʱ��ʧ��" );
//        return;
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\n��ǰʱ��:     Year: %d Month: %d Data: %d Week: %d  Time(H:M:S): %d:%d:%d",
//        stTmpTime.SpiDate.ucYear + 2000,
//        stTmpTime.SpiDate.ucMonth,
//        stTmpTime.SpiDate.ucDate,
//        stTmpTime.SpiDate.ucWeekDay,
//        stTmpTime.SpiTime.ucHour,
//        stTmpTime.SpiTime.ucMinute,
//        stTmpTime.SpiTime.ucSecond);
//
//    if(SUCC != SYS_SystemStartDataTimeGet(&stTmpTime))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n��ȡϵͳ����ʱ��ʧ��" );
//        return;
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\nϵͳ����ʱ��: Year: %d Month: %d Data: %d Week: %d  Time(H:M:S): %d:%d:%d",
//        stTmpTime.SpiDate.ucYear + 2000,
//        stTmpTime.SpiDate.ucMonth,
//        stTmpTime.SpiDate.ucDate,
//        stTmpTime.SpiDate.ucWeekDay,
//        stTmpTime.SpiTime.ucHour,
//        stTmpTime.SpiTime.ucMinute,
//        stTmpTime.SpiTime.ucSecond);

    return;
}


#ifdef DEF_SYN
/*---------------------------------------------------------------------
    synclassset    - �趨���ݼ�������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID synclassset(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR ucTmp;
//    _CHAR  *cMsg[] = {"��̬������", "��̬�����ݺ���̬������", "������"};
//
//    if(2 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n���������������");
//        return;
//    }
//
//    ucTmp = (_UCHAR)SYS_ATOL(ppArgv[1]);
//
//    if(ucTmp > MAX_DATA_CLASS)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n�����������,ֵ����%d", MAX_DATA_CLASS);
//        return;
//    }
//    
//    g_ucDataSynClass = ucTmp;
//
//    CLI_EXT_Printf(pCliEnv, "\r\n�����õı��ݼ���Ϊ: [%s]", cMsg[g_ucDataSynClass]);

    return;
}

/*---------------------------------------------------------------------
    synclassshow    - ��ʾ���ݼ�������

    ����˵����
        pCliEnv         - ���룬�ն˺�
        siArgc          - ���룬����������Ĳ�������
        ppArgv          - ���룬����������Ĳ����б�
    
    ����ֵ: ��
------------------------------------------------------------------------*/
_VOID synclassshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _CHAR  *cMsg[] = {"��̬������", "��̬�����ݺ���̬������", "������"};
//
//    if(g_ucDataSynClass > MAX_DATA_CLASS)
//    {
//        g_ucDataSynClass = MAX_DATA_CLASS;
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\n���ݼ���Ϊ: [%s]", cMsg[g_ucDataSynClass]);

    return;
}
#endif



#ifdef  __cplusplus
}
#endif
