/*-----------------------------------------------------------
    sysCmd.c -  系统模块命令行c文件

    版权所有 2004 -2006 信威公司深研所BSC项目组. 


    修改历史记录
    --------------------
    20.00.01,       12-28-2004,      李培冀      创建.   
-----------------------------------------------------------*/
#ifdef  __cplusplus
extern  "C"
{
#endif


/*
*头文件包含
*/
#include "sysos.h"
#include "cliConfig.h"
#include "cliShell.h"

#include "sysCmd.h"
//#include "sysPrint.h"

/*
*宏定义
*/
#define THIS_FILE_ID FILE_SYN_C
#define MAX_TRACE_DIRE_NUM      10          //最大的消息跟踪方向数目
#define MAX_FID 4
#define MAX_MOD_ID  8
/*
*外部变量声明
*/
//extern FID_PRINT_RECORD_S  g_stFidPrintCB[ MAX_FID ];                   //模块打印控制块
//extern MSG_TRACE_DIRE_CFG  g_stMsgTraceDireCfg[MAX_TRACE_DIRE_NUM];     //[消息跟踪控制]控制块
extern _UCHAR              g_ucModMsgRedireCtrl[MAX_MOD_ID];            //模块之间的消息重定向控制([SWITCH_OFF]为关闭，[SWITCH_ON]为打开)
extern _UCHAR              g_ucFidMsgRedireCtrl[MAX_FID];               //功能块之间的消息重定向控制([SWITCH_OFF]为关闭，[SWITCH_ON]为打开)
extern _UCHAR              g_ucMsgTraceSwtich;                          //[消息跟踪控制]总开关
extern _UCHAR              g_ucDataSynClass;                            //数据同步级别，默认只备份稳态数据

/*----------------------------------------------------------------
  说明:[sysSymTbl]原来类型是 SYMTAB_ID, 这是一个指针;
       如果要定义类型SYMTAB_ID,会引起很多的同文件定义问题,
       因此,将其改为 _ULONG 型的指针，虽然指针类型不同，但不会出错
----------------------------------------------------------------*/
//extern SYMTAB_ID  sysSymTbl;  /* system symbol table */
//extern _ULONG   *sysSymTbl;


/*
*外部函数声明
*/
//extern _UINT SYS_NowDataTimeGet(SYS_DATE_TIME_STRU *pstNowTime);
//extern _UINT SYS_SystemStartDataTimeGet(SYS_DATE_TIME_STRU *pstTime);

/*
*全局变量定义
*/


/*
*本地函数定义
*/
/*---------------------------------------------------------------------
    printopenall    - 打开所有打印开关命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID printopenall(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
 //   SYS_PrintOpenAll(pCliEnv);
	printf("\n\r printfopenall called!");
}

/*---------------------------------------------------------------------
    printcloseall    - 关闭所有打印开关命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID printcloseall(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
 //   SYS_PrintCloseAll(pCliEnv);
 	printf("\n\r printcloseall called!");
}

/*---------------------------------------------------------------------
    printopenone    - 打开一个功能块的打印开关命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID printopenone(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _ULONG      ulFid; 
//    PRINT_LEVEL ePrintLevel;
//
//    if(3 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数个数不对");
//        return;
//    }
//
//    ulFid       = SYS_ATOL(ppArgv[1]);
//    ePrintLevel = SYS_ATOL(ppArgv[2]);
//
//    if((ulFid >= MAX_FID) || (ePrintLevel >= MAX_PL))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数值不对");
//        return;
//    }
//    
//    SYS_PrintOpenOne(pCliEnv, ulFid, ePrintLevel);

    return;
}

/*---------------------------------------------------------------------
    printcloseone    - 关闭一个功能块的打印开关命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID printcloseone(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _ULONG      ulFid; 
//
//    if(2 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数个数不对");
//        return;
//    }
//
//    ulFid       = SYS_ATOL(ppArgv[1]);
//
//    if(ulFid >= MAX_FID)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数值不对");
//        return;
//    }
//    
//    SYS_PrintCloseOne(pCliEnv, ulFid);

    return;
}

/*---------------------------------------------------------------------
    printopenfileline    - 打开打印文件名和行号的打印开关命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID printopenfileline(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_PrintOpenFileLine(pCliEnv);
}

/*---------------------------------------------------------------------
    printclosefileline    - 关闭打印文件名和行号的打印开关命令
    

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID printclosefileline(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_PrintCloseFileLine(pCliEnv);
}

/*---------------------------------------------------------------------
    printshow    - 显示所有功能块的打印开关命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID printshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_PrintShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    taskinfoshow    - 显示所有任务信息命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID taskinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_AllRegTaskInfoShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    fidinfoshow    - 显示所有功能块信息命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID fidinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
 //   SYS_AllRegFidInfoShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    ipcfginfoshow    - 显示IP的相关配置信息命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID ipcfginfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_AllIPCfgInfoShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    msgstatinfoshow    - 显示消息统计信息命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID msgstatinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_MsgStatShow(pCliEnv);

    return;
}

/*---------------------------------------------------------------------
    msgtraceopen    - 消息跟踪打开命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID msgtraceopen(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    if(SUCC == SYS_MsgTraceSwitchOpen())
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪已打开");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪打开失败");
//    }

    return;
}

/*---------------------------------------------------------------------
    msgtraceclose    - 消息跟踪关闭命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID msgtraceclose(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//   if(SUCC == SYS_MsgTraceSwitchClose())
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪已关闭");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪关闭失败");
//    }

    return;
}

/*---------------------------------------------------------------------
    msgtracedireadd    - 添加消息跟踪方向命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID msgtracedireadd(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR ucMod1, ucFid1, ucMod2, ucFid2;
//    
//
//    if(5 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数个数不对");
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
//        CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪方向添加成功.");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪方向添加失败");
//    }

    return;
}

/*---------------------------------------------------------------------
    msgtracedireadd    - 删除消息跟踪方向命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID msgtracediredel(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR ucIndex;
//    
//
//    if(2 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数个数不对");
//        return;
//    }
//
//    ucIndex  = (_UCHAR)SYS_ATOL(ppArgv[1]);
//
//    if(SUCC == SYS_MsgTraceDireDel(ucIndex))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪方向删除成功.");
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪方向删除失败");
//    }

    return;
}

/*---------------------------------------------------------------------
    msgtraceshow    - 消息跟踪开关和方向显示命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID msgtraceshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR  i;
//
//    CLI_EXT_Printf(pCliEnv, "\r\n\r\n-------------------------------------------------------------");
//
//    CLI_EXT_Printf(pCliEnv, "\r\n消息跟踪开关状态: ");
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
    modmsgredire    - 模块消息重定向命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID modmsgredirecfg(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR  ucModId, ucCtrl;
//    _CHAR   *cStr[2] = {"关闭", "打开"};
//
//    if(3 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数个数不对");
//        return;
//    }
//
//    ucModId = (_UCHAR)SYS_ATOI(ppArgv[1]);
//    ucCtrl  = (_UCHAR)SYS_ATOI(ppArgv[2]);
//
//    if(ucModId >= MAX_MOD_ID)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入模块号不对，大于最大的模块号数: %d", MAX_MOD_ID);
//        return;
//    }
//
//    if((SWITCH_ON != ucCtrl) && (SWITCH_OFF != ucCtrl))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入控制字不对，打开: %d, 关闭: %d", SWITCH_ON, SWITCH_OFF);
//        return;
//    }
//
//    if(SUCC == SYS_ModMsgRedireCfg(ucModId, ucCtrl))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n%s模块 [%d] 消息转接成功", cStr[ucCtrl], ucModId);
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n%s模块 [%d] 消息转接失败", cStr[ucCtrl], ucModId);
//    }

    return;
}

/*---------------------------------------------------------------------
    fidmsgredire    - 功能块消息重定向命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID fidmsgredirecfg(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR  ucFid, ucCtrl;
//    _CHAR   *cStr[2] = {"关闭", "打开"};
//
//    if(3 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数个数不对");
//        return;
//    }
//
//    ucFid = (_UCHAR)SYS_ATOI(ppArgv[1]);
//    ucCtrl  = (_UCHAR)SYS_ATOI(ppArgv[2]);
//
//    if(ucFid >= MAX_FID)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入功能块号不对，大于最大的功能块号数: %d", MAX_FID);
//        return;
//    }
//
//    if((SWITCH_ON != ucCtrl) && (SWITCH_OFF != ucCtrl))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入控制字不对，打开: %d, 关闭: %d", SWITCH_ON, SWITCH_OFF);
//        return;
//    }
//
//    if(SUCC == SYS_FidMsgRedireCfg(ucFid, ucCtrl))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n%s功能块 [%s] 消息转接成功", cStr[ucCtrl], g_stFidPrintCB[ucFid].ucFidName);
//    }
//    else
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n%s功能块 [%s] 消息转接失败", cStr[ucCtrl], g_stFidPrintCB[ucFid].ucFidName);
//    }
//
    return;
}


/*---------------------------------------------------------------------
    msgredireshow    - 消息重定向配置显示命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
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
    msgqnumshow    - 各个任务的消息队列的消息个数显示命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
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
     *  在VxWorks环境下，注册 i, ti, d 命令
     *==================================================*/
#ifdef VXWORKS

#define MAX_SYS_SYM_LEN 256 /* system symbols will not exceed this limit */
#define MAX_DSP_TASKS   500 /* max tasks that can be displayed */
/*---------------------------------------------------------------------
    debugprintTaskInfo    - VxWorks下显示某一任务信息函数

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
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
    debugprintTaskRegs    - VxWorks下显示任务信息命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
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
    i    - VxWorks下显示任务信息命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
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
//    /*显示所有任务信息*/
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
    ti    - VxWorks下显示任务控制块的信息命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
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
//        CLI_EXT_Printf(pCliEnv,"\r\n没有输入参数");
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
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数不正确" );
//    }
//     
    return;
}

#endif

/*---------------------------------------------------------------------
    systimeshow    - 显示当前日期、时间和系统启动日期、时间命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID systimeshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    SYS_DATE_TIME_STRU stTmpTime;
//
//    if(SUCC != SYS_NowDataTimeGet(&stTmpTime))
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n获取当前时间失败" );
//        return;
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\n当前时间:     Year: %d Month: %d Data: %d Week: %d  Time(H:M:S): %d:%d:%d",
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
//        CLI_EXT_Printf(pCliEnv, "\r\n获取系统启动时间失败" );
//        return;
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\n系统启动时间: Year: %d Month: %d Data: %d Week: %d  Time(H:M:S): %d:%d:%d",
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
    synclassset    - 设定备份级别命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID synclassset(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _UCHAR ucTmp;
//    _CHAR  *cMsg[] = {"稳态类数据", "稳态类数据和暂态类数据", "不备份"};
//
//    if(2 != siArgc)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数个数不对");
//        return;
//    }
//
//    ucTmp = (_UCHAR)SYS_ATOL(ppArgv[1]);
//
//    if(ucTmp > MAX_DATA_CLASS)
//    {
//        CLI_EXT_Printf(pCliEnv, "\r\n输入参数不对,值大于%d", MAX_DATA_CLASS);
//        return;
//    }
//    
//    g_ucDataSynClass = ucTmp;
//
//    CLI_EXT_Printf(pCliEnv, "\r\n你设置的备份级别为: [%s]", cMsg[g_ucDataSynClass]);

    return;
}

/*---------------------------------------------------------------------
    synclassshow    - 显示备份级别命令

    参数说明：
        pCliEnv         - 输入，终端号
        siArgc          - 输入，命令行输入的参数个数
        ppArgv          - 输入，命令行输入的参数列表
    
    返回值: 无
------------------------------------------------------------------------*/
_VOID synclassshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv)
{
//    _CHAR  *cMsg[] = {"稳态类数据", "稳态类数据和暂态类数据", "不备份"};
//
//    if(g_ucDataSynClass > MAX_DATA_CLASS)
//    {
//        g_ucDataSynClass = MAX_DATA_CLASS;
//    }
//
//    CLI_EXT_Printf(pCliEnv, "\r\n备份级别为: [%s]", cMsg[g_ucDataSynClass]);

    return;
}
#endif



#ifdef  __cplusplus
}
#endif
