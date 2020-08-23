/* wrSbc74x7.h - wrSbc7447/57 board header file */

/* Copyright 2004 Wind River Systems, Inc. */

/*
modification history
--------------------
01c,17sep04,mdo  Adding wrSbc7447A support
01b,18mar03,kp   teamF1, defined macros which are under the macro
                 TORNADO_AE_653, to make romInit.s and sysAlib.s common for
                 the both AE653 and Tornado-221 BSPs.
01a,19nov03,kp   created by teamF1.
*/

/*
DESCRIPTION

This file contains I/O addresses and related constants for the wrSbc7447/57 board.
*/

#ifndef __INCwrSbc74x7h
#define __INCwrSbc74x7h
#ifdef __cplusplus
extern "C" {
#endif

#include "wrSbc8548.h"
#include "config.h"

#ifdef TORNADO_AE_653 /* defined in AE Makefile as
                       * EXTRA_DEFINE = -DTORNADO_AE_653
                       */

/* the following macros are not available in AE */
    
#ifndef _WRS_ASM
#define _WRS_ASM(x)     __asm__ volatile (x)
#endif /*  _WRS_ASM */

#ifndef WRS_ASM
#define WRS_ASM(x)      _WRS_ASM(x)
#endif /* WRS_ASM */

#ifndef FUNC_EXPORT
#define FUNC_EXPORT(x)      .globl x
#endif /* FUNC_EXPORT */

#ifndef FUNC_IMPORT
#define FUNC_IMPORT(x)      .extern x
#endif /* FUNC_IMPORT */

#ifndef FUNC_BEGIN
#define FUNC_BEGIN(x)       x:
#endif /* FUNC_BEGIN */

#ifndef FUNC_END
#define FUNC_END(x)
#endif /* FUNC_END */
    
#endif /* TORNADO_AE_653 */
    

/***ADD BY DD**/
#define L3_SYSTEM


/* C PPC syncronization macro */
#define PPC_EIEIO_SYNC  WRS_ASM (" eieio; sync")    

#ifndef _ASMLANGUAGE
/*#include "sysBootLoad.h" */
#include "sysBtsConfigData.h"
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __INCwrSbc745xh */                     


    
