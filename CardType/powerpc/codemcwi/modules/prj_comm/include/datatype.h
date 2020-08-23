
#ifndef DATATYPE_H
#define DATATYPE_H

#if (defined __NUCLEUS__) && (!defined NUCLEUS)
#include "NUCLEUS.h"
#endif 

//self def types
typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef char int8;
typedef short int16;
typedef int int32;
typedef int INT32;
//typedef unsigned long u_long;
//typedef unsigned int u_int;
#ifndef VXWORKS
typedef unsigned short u_short;
typedef unsigned char u_char;
#endif//VXWORKS
typedef unsigned int u32_t;
typedef unsigned short u16_t;
typedef unsigned char u8_t;
typedef short s16_t;
typedef char s8_t;
typedef int s32_t;



#if defined DSP_BIOS
//data type def
typedef unsigned int u_long; //32bit unsigned int
//typedef unsigned short u_short;
typedef unsigned char 		UWord8;
typedef char 				Word8;
typedef unsigned short		UWord16;
typedef short 				Word16;
//typedef unsigned int		Uint32;
//typedef signed int			int32;
typedef unsigned int 		UWord32;
typedef u32_t 				Word32;
/*typedef UWord8 UINT8;
typedef UWord16 UINT16;
typedef UWord32 UINT32;*/
#endif

#ifdef WIN32
typedef uint8 UINT8;
typedef uint16 UINT16;
typedef uint32 UINT32;
#endif

#ifdef NUCLEUS_PORT
typedef unsigned char u_char;
typedef unsigned int u_int;
typedef unsigned long ULONG;
#endif

#ifdef NO_OS
typedef unsigned long u_long;
typedef unsigned short u_short;
typedef uint8 UINT8;
typedef uint16 UINT16;
typedef uint32 UINT32;
#endif//NO_OS

#if !defined __INCvxTypesOldh && !defined __NUCLEUS__
//typedef unsigned long    ULONG;
typedef unsigned int    UINT32;
typedef unsigned short  UINT16;
typedef unsigned char   UINT8;
#endif//__INCvxTypesOldh

#ifndef PACKED
#define PACKED __attribute__ ((packed))
#endif

#ifdef COMIP
typedef signed int             SINT32;
typedef signed short           SINT16;
typedef signed char            SINT8; /*in vxWorks, char is unsigned, only signed char can be used for signed value */
#else
typedef int             SINT32;
typedef short           SINT16;
typedef char            SINT8; /*in vxWorks, char is unsigned, only signed char can be used for signed value */
#endif

/*
typedef UINT8 u8_t;
typedef UINT16 u16_t;
typedef UINT32 u32_t;
*/

typedef volatile unsigned char VUINT8;

#if defined __NUCLEUS__
#ifdef BF_NU_L2
#include "stddef.h"
#else
typedef unsigned int size_t;
#endif

typedef int            BOOL;
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

#endif

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#if defined __NUCLEUS__
#ifdef PCMCIA_ARM
#define NO_VOICE
#define NO_SDRAM
#endif
#endif

#ifndef CPE_5M
#define QUICK_SYNC
#endif

#define RF_PROFILE

#ifdef MEM113
#define  L3DATA_REPORT
#endif

//added by maqiang 080813
#ifdef BTS_SIDE
#define M_CLUSTER_EN
#endif

#ifdef BTS_SIDE
#define NUCLEAR
#endif


#ifdef M_CLUSTER_EN
    #define M_CLUSTER_DEBUG
    //#define M_CLUSTER_NOSLEEP
    #define M_CLUSTER_SAME_F
    #define M_CLUSTER_SLEEP
    #define M_CLUSTER_DIAG
    #define PTT_PRESS_OPTIMIZE

    #define M_CLUSTER_CLONE
    #define M_CLUSTER_MULTI_RSC //added for guilin airport. lrc 110704
#endif
//#define QUICK_BW_RECONFIG

#define LOAD_BALANCE

#define QUICK_DECREASE_BW
#define MULTI_RECONFIG_PERFRAME
#define PREEMPT_TEST
//#ifdef LARGE_BTS
#define FAX_EN
#define FAX_QAM4
//#endif

#define RPT_LOG LOG_DEBUG3
#define RCPE_SWITCH
//#ifdef CPE_RPT
//#undef NO_VOICE
//#endif
#define NUCLEAR_CODE
#define LOCATION_2ND
#define MZ_2ND
//#define REMOTE_PRJ  //‘∂æ‡¿Îranging
#define PCSS_MINIACK
#define PAYLOAD_BALANCE_2ND
#endif//DATATYPE_H
