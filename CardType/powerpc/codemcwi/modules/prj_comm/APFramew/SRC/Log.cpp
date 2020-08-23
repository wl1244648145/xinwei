/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: Log.cpp
 *
 * DESCRIPTION:
 *     Implementation of the FWKLIB's Log Utility.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 10/31/2005  Liu Qun     Added CPE log support interface.
 * 09/04/2005  Liu Qun     Initial file creation.
 * 02/14/2006  Yushu Shi   Change this file for windows implementation only
 *---------------------------------------------------------------------------*/

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifdef __NUCLEUS__
#include "CpeLog.cpp"
#else
#ifndef __WIN32_SIM__
#include "BtsLog.cpp"
#else

////////////////////////////////////////////////////////////////////

#ifndef _INC_STDARG
#include <stdarg.h>
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

#include "logArea.h"
#include "DebugLevel.h"
extern CDebugLevel g_debugLevel[LOG_AI_MAX] ;

///////////////////////////////////////////////////////////////////////////
bool CLog::LogAdd(const char* lpszFileName,int nLine, LOGLEVEL level, UINT32 errcode, const char format[],...)
{
    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return false;
    }

    va_list vaList;
    char fmt[260];
    int nChars;

    lpszFileName = ::strrchr(lpszFileName,'\\');
    lpszFileName++;

    va_start(vaList, format);
    if (::strlen(lpszFileName)>=8)
    {
        ::sprintf(fmt, "t%.15s\t%4d %.8X %s\n",lpszFileName,nLine,errcode,format);
    }
    else
    {
        ::sprintf(fmt, "t%.15s\t\t%4d %.8X %s\n",lpszFileName,nLine,errcode,format);
    }
        
    nChars = ::vprintf(fmt, vaList);
    va_end(vaList);
    return true;
}

#ifndef M_TGT_L2
#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

bool CLog::LogAddMessage(const char* lpszFileName,int nLine,LOGLEVEL level,UINT32 errcode,const CComMessage* pComMsg,const char format[],...)
{
    bool bRet = false;
    bool bNew = false;

    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return false;
    }

    va_list vaList;
    int nChars;

    char szTitle[260];

    lpszFileName = ::strrchr(lpszFileName,'\\');
    lpszFileName++;


    va_start(vaList, format);
    if (::strlen(lpszFileName)>=8)
    {
        ::sprintf(szTitle,"t%.15s\t%4d %.8X %s",lpszFileName,nLine,errcode,format);
    }
    else
    {
        ::sprintf(szTitle,"t%.15s\t\t%4d %.8X %s",lpszFileName,nLine,errcode,format);
    }
        
    nChars = vsprintf(szTitle,szTitle,vaList);
    va_end(vaList);

    char MsgHead[260]={0};
    char* pBuf;

    if (!ASSERT_VALID(pComMsg))
    {
        //LOG(LOG_CRITICAL,0,"Invalid CComMessage pointer.");
        pBuf = "Invalid CComMessage pointer.\n";
    }
    else
    {
#if !(M_TARGET==M_TGT_CPE)
		::sprintf(MsgHead,"Src:%d, Dst:%d, ID:0x%.4X, EID:%.8X, UID:%.4X, FLAG:%.8X\
			\n\tDirect:%.2X, IpType:%.2X, BtsAddr:0x%.8X, DhcpPtr:0x%0.8X,\
			\n\tUdpPtr:0x%.8X, KeyMac:0x%0.8X\n",
			pComMsg->GetSrcTid(), pComMsg->GetDstTid(), pComMsg->GetMessageId(), \
			pComMsg->GetEID(),pComMsg->GetUID(), pComMsg->GetFlag(), pComMsg->GetDirection(),\
			pComMsg->GetIpType(), pComMsg->GetBtsAddr(),
			pComMsg->GetDhcpPtr(), pComMsg->GetUdpPtr(), pComMsg->GetKeyMac()
        );
#else
		::sprintf(MsgHead,"Src:%d, Dst:%d, ID:0x%.4X, FLAG:%.8X\n",
			pComMsg->GetSrcTid(), pComMsg->GetDstTid(), pComMsg->GetMessageId(), \
			pComMsg->GetFlag());
#endif
        pBuf = new char[pComMsg->GetDataLength()*3+1];
        if (pBuf==NULL)
        {
            //LOG(LOG_CRITICAL,0,"");
            pBuf = "Allocate Buffer failed.\n";
        }
        else
        {
            bNew = true;
            UINT32 uDataLen = pComMsg->GetDataLength();
            UINT8* pData = (UINT8*)pComMsg->GetDataPtr();
            if (uDataLen==0 || pData==NULL)
            {
                delete [] pBuf;
                bNew = false;
                pBuf = "Empty CComMessage.\n";
            }
            else
            {
                int j=0;
                for (UINT32 i=0;i<uDataLen;i++,j+=3)
                {
                    ::sprintf(pBuf+j,"%.2X ",pData[i]);
                }
                pBuf[j-1]='\n';
            }
            bRet = true;
        }
    }

    nChars = ::printf("%s\n\t%s\t%s",szTitle,MsgHead,pBuf);
    if (bNew)
        delete [] pBuf;

    return bRet;
}

bool CLog::LogAddBulk(const char* lpszFileName, int nLine, LOGLEVEL level, UINT32 errcode, UINT8* pBlock, UINT32 uLen, const char format[],...)
{
    bool bRet = false;
    #if 0
    bool bNew = false;

    TID tid = GetCurrentTaskId();
    if (level > s_LogRegistry[tid].level)
        return true;

    va_list vaList;
    int nChars;

    char szTitle[260];

    lpszFileName = ::strrchr(lpszFileName,'\\');
    lpszFileName++;

    va_start(vaList, format);
    if (::strlen(lpszFileName)>=8)
    {
        ::sprintf(szTitle,"%3d\t%.15s\t%4d %.8X %s",tid,lpszFileName,nLine,errcode,format);
    }
    else
    {
        ::sprintf(szTitle,"%3d\t%.15s\t\t%4d %.8X %s",tid,lpszFileName,nLine,errcode,format);
    }

    nChars = vsprintf(szTitle,szTitle,vaList);
    va_end(vaList);

    char* pBuf;
    if (pBlock==NULL)
    {
        pBuf = "Tried to use a NULL pointer.\n";
    }
    else
    {
        if (uLen==0)
        {
            pBuf = "Zero length block.\n";
        }
        else
        {
            pBuf = new char[uLen*3 + 1];
            if (pBuf==NULL)
            {
                pBuf = "Allocate buffer failed.\n";
            }
            else
            {
                bNew = true;
                int j=0;
                for (UINT32 i=0;i<uLen;i++,j+=3)
                {
                    ::sprintf(pBuf+j, "%.2X ",pBlock[i]);
                }
                pBuf[j-1]='\n';
                bRet = true;
            }
        }
    }

    nChars = ::printf("%s\n\t%s",szTitle, pBuf);

    if (bNew)
        delete [] pBuf;
    #endif
    
    return bRet;
}

#endif

bool CLog::InitLogTask()
{
    return true;
}



bool InitLogTask()
{
    return CLog::InitLogTask();
}
#endif   // WIN_32
#endif  // NUCLEUS
