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
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_CPE_LOG
#define _INC_CPE_LOG

#ifndef NUCLEUS
#include "NUCLEUS.h"
#endif

#define LOG_STR   LOG
#define LOG_STR1  LOG1
#define LOG_STR2  LOG2
#define LOG_STR3  LOG3
#define LOG_STR4  LOG4

#if !defined NDEBUG && !defined BF_NU_L2
#include "ComMessage.h"

void LogDebugMsg(LOGLEVEL level, UINT32 errcode, const char *text, int arg1, int arg2, int arg3, int arg4);
void LogComMessage(LOGLEVEL level, UINT32 errcode, const CComMessage* pComMsg, const char *text);

#define _LOG(logLevel, errorCode, text, arg1, arg2, arg3, arg4) \
              LogDebugMsg(logLevel, errorCode, text, arg1, arg2, arg3, arg4)
#define LOG(logLevel, errorCode, text)                          _LOG(logLevel, errorCode, text, 0,0,0,0)
#define LOG1(logLevel, errorCode, text, arg1)                   _LOG(logLevel, errorCode, text, arg1, 0,    0,    0)
#define LOG2(logLevel, errorCode, text, arg1,arg2)              _LOG(logLevel, errorCode, text, arg1, arg2, 0,    0)
#define LOG3(logLevel, errorCode, text, arg1, arg2, arg3)       _LOG(logLevel, errorCode, text, arg1, arg2, arg3, 0)
#define LOG4(logLevel, errorCode, text, arg1, arg2, arg3, arg4) _LOG(logLevel, errorCode, text, arg1, arg2, arg3, arg4)


#define LOGMSG(logLevel, errorCode, pComMsg, text)\
             LogComMessage(logLevel, errorCode, pComMsg, text)

#define 	LOG_PRINT(text)  printf(text)
#define 	LOG_PRINT2(text,d1,d2) printf(text, d1,d2)


#else
extern "C" void Trace(int lev,char *fmt,...);
#define LOG(logLevel, errorCode, text)                          ;//Trace(10,text)
#define LOG1(logLevel, errorCode, text, arg1)                   ;//Trace(10,text,arg1)
#define LOG2(logLevel, errorCode, text, arg1,arg2)              ;//Trace(10,text, arg1,arg2)
#define LOG3(logLevel, errorCode, text, arg1, arg2, arg3)       ;//Trace(10,text, arg1,arg2, arg3)
#define LOG4(logLevel, errorCode, text, arg1, arg2, arg3, arg4) ;//Trace(10,text, arg1,arg2, arg3,arg4)

#define LOGMSG(logLevel, errorCode, pComMsg, text)  ;

#define 	LOG_PRINT(x) ;
#define 	LOG_PRINT2(x,d1,d2) ;

#endif

#endif 



