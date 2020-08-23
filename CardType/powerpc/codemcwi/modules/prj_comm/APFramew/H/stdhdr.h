#ifndef _INC_STDHDR
#define _INC_STDHDR

#ifndef DATATYPE_H
#include "datatype.h"
#endif
#include "Vxw_hdrs.h"//add by huangjl

#ifdef __WIN32_SIM__
#undef WAIT_FOREVER
#define WAIT_FOREVER INFINITE

#undef NO_WAIT
#define NO_WAIT 0
#endif

#ifdef __NUCLEUS__
#ifndef NUCLEUS
#include "NUCLEUS.h"
#endif

#undef WAIT_FOREVER
#define WAIT_FOREVER NU_SUSPEND

#undef NO_WAIT
#define NO_WAIT NU_NO_SUSPEND
#endif

#ifdef M_TGT_L3
#define M_TARGET M_TGT_L3
#elif M_TGT_CPE
#define M_TARGET M_TGT_CPE
#elif M_TGT_L2
#define M_TARGET M_TGT_L2
#else
#ifndef COMIP
//#error "M_TGT_xxx undefined" //modify by huangjl
#endif
#endif

#if (M_TARGET==M_TGT_L3 || M_TARGET==M_TGT_L2)
#define M_DEFAULT_RESERVED 64
#elif (M_TARGET==M_TGT_CPE)
#define M_DEFAULT_RESERVED 64
#endif

#ifndef SIZEOFWORDS
#define SIZEOFWORDS(element)  (sizeof(element)/sizeof(UINT32))
#endif

#ifndef SIZEOF
#define SIZEOF(slice)         (sizeof(slice)/sizeof((slice)[0]))
#endif

#ifndef BIT
#define BIT(x)                (1 << (x))
#endif

#ifdef __WIN32_SIM__

#ifndef _WINDOWS_
#include <windows.h>
#endif

#undef MinutesToTicks
#define MinutesToTicks(expr)  (6000 * (expr))

#undef SecondsToTicks
#define SecondsToTicks(expr)  (1000 * (expr))

#undef MSToTicks
#define MSToTicks(expr)       ((expr))

#elif __NUCLEUS__

#undef MinutesToTicks
#define MinutesToTicks(expr)  (NU_PLUS_Ticks_Per_Second * 60 * (expr))

#undef SecondsToTicks
#define SecondsToTicks(expr)  (NU_PLUS_Ticks_Per_Second * (expr))

#undef MSToTicks
#define MSToTicks(expr)       ((NU_PLUS_Ticks_Per_Second)/1000)

#else

#ifndef __INCvxWorksh
#ifndef COMIP
//#include <vxWorks.h>//modify by huangjl
#endif
#endif

#ifndef __INCsysLibh
#ifndef COMIP
//#include <sysLib.h>//modify by huangjl
#endif
#endif

//***************add by huangjl******************************
#undef sysClkRateGet
#define sysClkRateGet() 100000
//*****************************************************/
#undef MinutesToTicks
#define MinutesToTicks(expr)  (sysClkRateGet() * (expr) * 60)

#undef SecondsToTicks
#define SecondsToTicks(expr)  (sysClkRateGet() * (expr))

#undef MSToTicks
#define MSToTicks(expr)       (sysClkRateGet() * (expr) / 1000)

#endif


#endif
