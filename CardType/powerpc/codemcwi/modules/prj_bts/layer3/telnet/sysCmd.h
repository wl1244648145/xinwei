/*-----------------------------------------------------------
    sysCmd.h -  系统模块命令行头文件

    版权所有 2004 -2006 信威公司深研所BSC项目组. 


    修改历史记录
    --------------------
    20.00.01,       12-28-2004,      李培冀      创建.   
-----------------------------------------------------------*/
#ifndef _SYSCMD_H_
#define _SYSCMD_H_

#ifdef  __cplusplus
extern  "C"
{
#endif



//-------------------------------宏定义------------------------------------



//-------------------------------枚举定义------------------------------------



//-------------------------------结构定义------------------------------------



//-------------------------------函数声明------------------------------------
_VOID printopenall(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID printcloseall(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID printopenone(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID printcloseone(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID printopenfileline(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID printclosefileline(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID printshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID taskinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID fidinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID ipcfginfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID msgstatinfoshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID msgtraceopen(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID msgtraceclose(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID msgtracedireadd(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID msgtracediredel(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID msgtraceshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID modmsgredirecfg(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID fidmsgredirecfg(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID msgredireshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID msgqnumshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);

#ifdef VXWORKS
_VOID debugprintTaskInfo(CLI_ENV *pCliEnv, TASK_DESC *pTd );
_VOID debugprintTaskRegs(CLI_ENV *pCliEnv, TASK_DESC *pTd );
_VOID si(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID sti(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
#endif

_VOID systimeshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID synclassset(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);
_VOID synclassshow(CLI_ENV *pCliEnv, _INT siArgc,_CHAR **ppArgv);




#ifdef  __cplusplus
}
#endif
#endif
