/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bsp_types.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_TYPES_H
#define BSP_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "errno.h"
#include "stdarg.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/types.h"
#include "asm/types.h"
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include "sys/times.h"
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <signal.h>
#include "sys/param.h"
#include "sys/statfs.h"
#include "stdint.h"
#include <sys/types.h>
#include "sys/select.h"
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/* access control define */
#define	Private	 static
#define	Public
#define Import   extern
#define Export   extern

/* general type declare */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long ul32;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long sl32;

typedef volatile unsigned int vu32;
typedef volatile signed int vs32;
typedef volatile unsigned char vu8;

typedef unsigned int    UINT32;
typedef unsigned short  UINT16;
typedef unsigned char   UINT8;
typedef signed int   SINT32;
typedef signed short  SINT16;
typedef signed char   SINT8; 



typedef float f32;
typedef double f64;
typedef double u64;

typedef unsigned int ubool;

#if 1
typedef char                CHAR;
//typedef unsigned char       UINT8;
typedef unsigned char       BYTE;
typedef unsigned char       UCHAR;
typedef unsigned char       BOOL;
typedef unsigned char       INT8;
typedef unsigned char       BOOLEAN;
typedef unsigned short      WORD16;
typedef signed short        SWORD16;
typedef signed short        INT16;
//typedef unsigned short      UINT16;
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
//typedef unsigned int        UINT32;
typedef unsigned long int   WORDPTR; 
typedef double              DOUBLE;
typedef void			          VOID;
typedef void*               LPVOID;

#endif
typedef int (*FUNCPTR)();
typedef void (*VOIDFUNCPTR)();

#define NULLPTR (0)

#define P3_MAX_CORE_NUM                  4
#define BSP_OK              (0)
#define BSP_ERROR           ((-1))

#define BSP_SUCCESS         (BSP_OK)
#define BSP_FAIL            (-1)

#define BSP_ON              (1)
#define BSP_OFF             (0)

#define FALSE               (0)
#define TRUE                (1)

#define ERROR               (-1)

#define OK                  (0)
#define YES                (1)
#define NO                  (0)
#if defined   CPU_FSL_SYS_BIT32_WIDTH
#undef        CPU_FSL_SYS_BIT32_WIDTH  
#endif

#define   CPU_FSL_SYS_BIT64_WIDTH

/* socket define */
typedef u32 SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)


#undef BSP_DBG_ON
#if defined(BSP_DBG_ON)
	#define bsp_dbg(format, arg...) printf("[%s:%s():%u]- " format,	__FILE__,__func__, __LINE__,  ##arg)
#else
#define bsp_dbg(arg...)
#endif

#ifdef __cplusplus
}
#endif

#endif
/******************************* 头文件结束 ********************************/

