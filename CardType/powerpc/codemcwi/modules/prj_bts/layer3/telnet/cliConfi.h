/*-------------------------------------------------------------------
        .h - ������ģ�����ù�����

��Ȩ���� 2004 -2006 ������˾������BSC��Ŀ��. 

�޸���ʷ��¼
--------------------

04.00.01A,  08-05-2004,     L.Y     ����
---------------------------------------------------------------------*/



#ifndef _CLICONFIG_H_
#define _CLICONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* _ _cplusplus */


/*
* ��ģ�鸺���û����õĶ�������
* 
*/


/* 
*   ͷ�ļ����� 
*/



/* 
*   �궨��
*/


/* �ֽ�����*/
#undef  __LITTLE_ENDIAN_SYSTEM__
#define __BIG_ENDIAN_SYSTEM__


/* ϵͳʱ������ */
#define K_HWTIC_PER_SEC 1

#define K_MAKEUP_BUF_SIZE   1000

/* telnet�ȴ���ʱ */
#define K_SOCK_TO   30

/* ƽ̨ѡ�� */
#undef  __SINGLE_THREADED_SERVER_ENABLED__
#define __MULTI_THREADED_SERVER_ENABLED__

/*HTTP ������г��� */
#define kHTTPD_QUEUE_SIZE           10

//���������ر�
#undef __BSC_TASK_CLI__

/* 
* Telnet ���
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


/* Shell ��� */

/* Prompt Displayed by the Shell */
#define kCLI_DEFAULT_PROMPT "SYS"

/* ֧��shell */
#define __CLI_CONSOLE_ENABLED__ 


/* ��������� */
#define kCLI_MAX_CLI_TASK                   2//7

#define kCLI_HISTORY_BUFFER_SIZE    30

#endif /* _CLICONFIG_H_ */


