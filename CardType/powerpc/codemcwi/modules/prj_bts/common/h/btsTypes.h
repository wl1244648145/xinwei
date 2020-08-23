/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    btsTypes.h
 *
 * DESCRIPTION: This module defines types common for any processor and
 *              specifies the specifics for that processor's compiler.
 *
 * HISTORY:
 *
 *   Date       Author    Description
 *   ---------  ------    ----------------------------------------------------
 *   06/14/05   Hui Zhou  Initial file creation. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __BTSTYPES_H__
#define __BTSTYPES_H__

#if 0
#if !defined __INCvxTypesOldh && !defined NUCLEUS_PORT
typedef unsigned long    ULONG;
typedef unsigned int    UINT32;
typedef unsigned short  UINT16;
typedef unsigned char   UINT8;
#endif//__INCvxTypesOldh


#ifndef _INC_TYPES
typedef int             SINT32;
typedef short           SINT16;
typedef signed char     SINT8;
#endif

#define MinutesToTicks(expr)  (sysClkRateGet() * (expr) * 60)
#define SecondsToTicks(expr)  (sysClkRateGet() * (expr))
#define MSToTicks(expr)       (sysClkRateGet() * (expr) / 1000)

#endif

#include "datatype.h"


#define SIZEOFWORDS(element)  (sizeof(element)/sizeof(UINT32))
#define SIZEOF(slice)         (sizeof(slice)/sizeof((slice)[0]))
#define BIT(x)                (1 << (x))

typedef enum 
{
    _8_BYTE_ALIGN = 8,
    _16_BYTE_ALIGN = 16,
    _32_BYTE_ALIGN = 32,
    _64_BYTE_ALIGH = 64
}ALIGN_SIZE;

typedef void(*BI_FUNCPTR) (UINT32, UINT32);		/* pfunction returning int */

#endif


