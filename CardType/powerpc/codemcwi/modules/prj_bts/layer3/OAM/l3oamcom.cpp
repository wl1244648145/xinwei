#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#include <stdio.h>

#ifdef __WIN32_SIM__
#include <l3OamTest.h>
#include <time.h>
#else
#include <string.h>
#include "mcWill_bts.h"
#include "DebugLevel.h"
#endif


extern CDebugLevel g_debugLevel[LOG_AI_MAX];
extern UINT8 EntityMsgLogEnableTable[M_TID_MAX][M_TID_MAX];
extern char *EntityNameStr[M_TID_MAX];

extern FILE* pgL3oamLogFile;
bool bgRedirectLog = false;
void l3oamredirectlog()
{
    bgRedirectLog = !bgRedirectLog;
}
#ifndef __WIN32_SIM__
void OAM_LOGSTR(LOGLEVEL level, UINT32 errcode, const char* text)
{
#if 1


    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

     char temp[250];

	memset(temp, 0, sizeof(temp));
    T_TimeDate TimeData = bspGetDateTime();
	sprintf(temp, "[%04d/%02d/%02d %02d:%02d:%02d] ", TimeData.year, TimeData.month, TimeData.day, TimeData.hour, TimeData.minute, TimeData.second);
    strcat(temp, text);	

	///////////////////
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR(level, errcode, temp); 
#else
    if(bgRedirectLog)
	{
	    fwrite(text, 1, strlen(text), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR(level, errcode, text); 
#endif
}

void OAM_LOGSTR1(LOGLEVEL level, UINT32 errcode, const char* text, int arg1)
{
#if 1


    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    char temp[250];
	memset(temp, 0, sizeof(temp));
    T_TimeDate TimeData = bspGetDateTime();
	sprintf(temp, "[%04d/%02d/%02d %02d:%02d:%02d] ", TimeData.year, TimeData.month, TimeData.day, TimeData.hour, TimeData.minute, TimeData.second);
    strcat(temp, text);	
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR1(level, errcode, temp, arg1); 
#else
    if(bgRedirectLog)
	{
	    fwrite(text, 1, strlen(text), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR1(level, errcode, text, arg1); 
#endif
}

void OAM_LOGSTR2(LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2)
{
#if 1


    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    char temp[250];
	memset(temp, 0, sizeof(temp));
    T_TimeDate TimeData = bspGetDateTime();
	sprintf(temp, "[%04d/%02d/%02d %02d:%02d:%02d] ", TimeData.year, TimeData.month, TimeData.day, TimeData.hour, TimeData.minute, TimeData.second);
    strcat(temp, text);	

    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR2(level, errcode, temp, arg1, arg2); 
#else
    if(bgRedirectLog)
	{
	    fwrite(text, 1, strlen(text), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR2(level, errcode, text, arg1, arg2); 
#endif
}

void OAM_LOGSTR3(LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2, int arg3)
{
#if 1


    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    char temp[250];
	memset(temp, 0, sizeof(temp));
    T_TimeDate TimeData = bspGetDateTime();
	sprintf(temp, "[%04d/%02d/%02d %02d:%02d:%02d] ", TimeData.year, TimeData.month, TimeData.day, TimeData.hour, TimeData.minute, TimeData.second);
    strcat(temp, text);	
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR3(level, errcode, temp, arg1, arg2, arg3); 
#else
    if(bgRedirectLog)
	{
	    fwrite(text, 1, strlen(text), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR3(level, errcode, text, arg1, arg2, arg3); 
#endif
}

void OAM_LOGSTR4(LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2, int arg3, int arg4)
{
#if 1


    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }


    char temp[250];
	memset(temp, 0, sizeof(temp));
    T_TimeDate TimeData = bspGetDateTime();
	sprintf(temp, "[%04d/%02d/%02d %02d:%02d:%02d] ", TimeData.year, TimeData.month, TimeData.day, TimeData.hour, TimeData.minute, TimeData.second);
    strcat(temp, text);	
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR4(level, errcode, temp, arg1, arg2, arg3, arg4); 
#else
    if(bgRedirectLog)
	{
	    fwrite(text, 1, strlen(text), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else LOG_STR4(level, errcode, text, arg1, arg2, arg3, arg4); 
#endif
}
#else
void OAM_LOGSTR(char *filename, int lineno, LOGLEVEL level, UINT32 errcode, const char* text)
{

    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    char temp[250];
	memset(temp, 0, sizeof(temp));
	time_t now = time(NULL);
	sprintf(temp, "\r\n[%s] ", ctime(&now));
    temp[27] = ' ';
    strcat(temp, text);	

	///////////////////
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else printf(temp);

}

void OAM_LOGSTR1(char *filename, int lineno, LOGLEVEL level, UINT32 errcode, const char* text, int arg1)
{


    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    char temp[250];
	memset(temp, 0, sizeof(temp));
	time_t now = time(NULL);
	sprintf(temp, "\r\n[%s] ", ctime(&now));
    temp[27] = ' ';
    strcat(temp, text);	
 	
	///////////////////
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else printf(temp, arg1);

}

void OAM_LOGSTR2(char *filename, int lineno, LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2)
{


    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    char temp[250];
	memset(temp, 0, sizeof(temp));
	time_t now = time(NULL);
	sprintf(temp, "\r\n[%s] ", ctime(&now));
    temp[27] = ' ';
    strcat(temp, text);	

	///////////////////
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else printf(temp, arg1, arg2);

}

void OAM_LOGSTR3(char *filename, int lineno, LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2, int arg3)
{



    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    char temp[250];
	memset(temp, 0, sizeof(temp));
	time_t now = time(NULL);
	sprintf(temp, "\r\n[%s] ", ctime(&now));
    temp[27] = ' ';
    strcat(temp, text);	
    
	///////////////////
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else printf(temp, arg1, arg2, arg3);

}

void OAM_LOGSTR4(char *filename, int lineno, LOGLEVEL level , UINT32 errcode, const char* text, int arg1, int arg2, int arg3, int arg4)
{



    if (level > g_debugLevel[errcode>>16].GetDebugLevel())
    {   // no need to display the message if area debug level is higher than error level
        return;
    }

    char temp[250];
	memset(temp, 0, sizeof(temp));
	time_t now = time(NULL);
	sprintf(temp, "\r\n[%s] ", ctime(&now));
    temp[27] = ' ';
    strcat(temp, text);	

	///////////////////
    if(bgRedirectLog)
	{
	    fwrite(temp, 1, strlen(temp), pgL3oamLogFile);
		fflush(pgL3oamLogFile);
	}else printf(temp, arg1, arg2, arg3, arg4);
}
#endif

