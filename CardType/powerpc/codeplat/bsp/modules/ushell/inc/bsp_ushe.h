/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bbu_config.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_USHELL_H
#define BSP_USHELL_H

#define IPC_MSG_LENTH 512
#define ARG_NUM 10
#define ARG_LENGTH 100

typedef char                CHAR;
typedef unsigned char       BYTE;
typedef unsigned char       UCHAR;
typedef unsigned char       BOOL;
typedef unsigned char       INT8;
typedef unsigned char       BOOLEAN;
typedef unsigned char       UINT8;
typedef unsigned short      WORD16;
typedef signed short        SWORD16;
typedef signed short        INT16;
typedef unsigned short      UINT16;
typedef unsigned short      WORD;
typedef unsigned short      USHORT;
typedef float               FLOAT;
typedef unsigned long       WORD32;
typedef signed long         SWORD32;
typedef unsigned long       OSS_STATUS; 
typedef unsigned long       ULONG; 
typedef signed long long    SWORD64;
typedef unsigned long long  WORD64;
typedef unsigned long       DWORD;
typedef unsigned long long  UINT64;
typedef long                LONG;
typedef int                 INT;
typedef int                 INT32;
typedef unsigned int        UINT32;
typedef unsigned int        UINT;
typedef unsigned long int   WORDPTR; 
typedef double              DOUBLE;
typedef void			          VOID;
typedef void*               LPVOID;


#define BSP_OK              (0)
#define BSP_ERROR           ((unsigned long)(-1))
#define BSP_SUCCESS         (BSP_OK)
#define BSP_FAIL            ((long)(-1))
#define BSP_ON              (1)
#define BSP_OFF             (0)
#define FALSE               (0)
#define TRUE                (1)
#define ERROR               (-1)
#define OK                  (0)
#define YES                 (BYTE)1
typedef int (*FUNCPTR)();
typedef void (*VOIDFUNCPTR)();

#endif
/******************************* 头文件结束 ********************************/
