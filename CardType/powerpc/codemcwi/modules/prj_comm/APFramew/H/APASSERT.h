#ifndef _INC_APASSERT
#define _INC_APASSERT

#ifndef _INC_LOG
#include "Log.h"
#endif

#define ApAssert(condition, level, code, str, block)  \
    { \
    if (!(condition)) { \
        LOG(level, code, str); \
        {##block##;} \
        } \
    }

#define ApAssertRtnV(condition, level, code, str, block, rv)  \
    { \
    if (!(condition)) { \
        LOG(level, code, str); \
        {##block##; return(##rv##); } \
        } \
    }

#define ApAssertRtn(condition, level, code, str, block)  \
    { \
    if (!(condition)) { \
        LOG(level, code, str); \
        {##block##; return; } \
        } \
    }

#endif
