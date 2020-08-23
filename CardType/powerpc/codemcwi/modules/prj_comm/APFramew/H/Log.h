/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: Log.h
 *
 * DESCRIPTION:
 *     Declaration of FWKLIB's Log Utility.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 10/30/2005  Liu Qun     Added CPE log support interface.
 * 09/04/2005  Liu Qun     Initial file creation.
 * 02/14/2006  Yushu Shi   split files for cpe, bts and windows, 
 *                         change to fixed number of parameter list for CPE and BTS
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_LOG
#define _INC_LOG

typedef enum
{
    M_LL_CRITICAL = 0,
    M_LL_SEVERE,
    M_LL_MAJOR,
    M_LL_MINOR,
    M_LL_WARN,
    M_LL_DEBUG0,
    M_LL_DEBUG1,
    M_LL_DEBUG2,
    M_LL_DEBUG3,
    M_LL_LEVELS
}LOGLEVEL;
#ifndef WBBU_CODE
#define MAX_STRING_LEN 200
#else

#define MAX_STRING_LEN 1300
#endif
bool InitLogTask();


#ifndef __WIN32_SIM__

#define LOG_CRITICAL M_LL_CRITICAL   // no file name for CPEs
#define LOG_SEVERE   M_LL_SEVERE
#define LOG_MAJOR    M_LL_MAJOR
#define LOG_MINOR    M_LL_MINOR
#define LOG_WARN     M_LL_WARN
#define LOG_DEBUG    M_LL_DEBUG0
#define LOG_DEBUG1   M_LL_DEBUG1
#define LOG_DEBUG2   M_LL_DEBUG2
#define LOG_DEBUG3   M_LL_DEBUG3

#if defined __NUCLEUS__ ||defined  BF_NU_L2
#include "CpeLog.h"
#else
#include "BtsLog.h"
#endif

#else
#define LOG_CRITICAL __FILE__,__LINE__,M_LL_CRITICAL
#define LOG_SEVERE   __FILE__,__LINE__,M_LL_SEVERE
#define LOG_MAJOR    __FILE__,__LINE__,M_LL_MAJOR
#define LOG_MINOR    __FILE__,__LINE__,M_LL_MINOR
#define LOG_WARN     __FILE__,__LINE__,M_LL_WARN
#define LOG_DEBUG    __FILE__,__LINE__,M_LL_DEBUG0
#define LOG_DEBUG1   __FILE__,__LINE__,M_LL_DEBUG1
#define LOG_DEBUG2   __FILE__,__LINE__,M_LL_DEBUG2
#define LOG_DEBUG3   __FILE__,__LINE__,M_LL_DEBUG3

#ifndef M_TGT_L2
#include "ComMessage.h"
#endif

#include "datatype.h"

class CLog
{
private:

public:
    static bool LogAdd(const char* lpszFileName,int nLine, LOGLEVEL level, UINT32 errcode, const char format[],...);
    #ifndef M_TGT_L2
    static bool LogAddMessage(const char* lpszFileName, int nLine, LOGLEVEL level, UINT32 errcode, const CComMessage* pComMsg, const char format[],...);
    static bool LogAddBulk(const char* lpszFileName, int nLine, LOGLEVEL level, UINT32 errcode, UINT8* pBlock, UINT32 uLen, const char format[],...);
    #endif
    static bool InitLogTask();

private:
    CLog(CLog&);
};

#define LOG       CLog::LogAdd
#define LOG1      CLog::LogAdd
#define LOG2      CLog::LogAdd
#define LOG3      CLog::LogAdd
#define LOG4      CLog::LogAdd

#ifndef M_TGT_L2
#define LOGMSG    CLog::LogAddMessage
#define LOGBULK   CLog::LogAddBulk
#endif


#define LOG_STR   LOG
#define LOG_STR1  LOG1
#define LOG_STR2  LOG2
#define LOG_STR3  LOG3
#define LOG_STR4  LOG4



#endif  // __WIN32_SIM__

#endif //_INC_LOG
