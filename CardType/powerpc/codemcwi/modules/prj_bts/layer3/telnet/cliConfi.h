/*-------------------------------------------------------------------
        .h - 命令行模块配置管理部分

版权所有 2004 -2006 信威公司深研所BSC项目组. 

修改历史记录
--------------------

04.00.01A,  08-05-2004,     L.Y     创建
---------------------------------------------------------------------*/



#ifndef _CLICONFIG_H_
#define _CLICONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */


/*
* 本模块负责用户配置的定制内容
* 
*/


/* 
*   头文件引用 
*/



/* 
*   宏定义
*/


/* 字节序定义*/
#undef  __LITTLE_ENDIAN_SYSTEM__
#define __BIG_ENDIAN_SYSTEM__


/* 系统时钟设置 */
#define K_HWTIC_PER_SEC 1

#define K_MAKEUP_BUF_SIZE   1000

/* telnet等待超时 */
#define K_SOCK_TO   30

/* 平台选择 */
#undef  __SINGLE_THREADED_SERVER_ENABLED__
#define __MULTI_THREADED_SERVER_ENABLED__

/*HTTP 请求队列长度 */
#define kHTTPD_QUEUE_SIZE           10

//任务驱动关闭
#undef __BSC_TASK_CLI__

/* 
* Telnet 相关
*/


/* Telnet Port */
#define kCLI_FIXED_PORT 32

/* CLI Task Priority */
#define kCLI_SERVER_PRIO    100

/* Session Timeout Length */
#define kCLI_TIMEOUT    1800


/* Login Specifications */
#define kCLI_MAX_LOGIN_ATTEMPTS 5
#define kCLI_MAX_LOGIN_LEN  16
#define kCLI_MAX_PASSWORD_LEN   16
#define kCLI_LOGIN_PROMPT   "Login:"
#define kCLI_PASSWORD_PROMPT    "Passwd:"


/* Shell 相关 */

/* Prompt Displayed by the Shell */
#define kCLI_DEFAULT_PROMPT "SYS"

/* 支持shell */
#define __CLI_CONSOLE_ENABLED__ 


/* 最大连接数 */
#define kCLI_MAX_CLI_TASK                   2//7

#define kCLI_HISTORY_BUFFER_SIZE    30

#endif /* _CLICONFIG_H_ */


