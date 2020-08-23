/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: DebugLevel.cpp
 *
 * DESCRIPTION:
 *
 *     This file defines the debugLevel arrays and implements the shell 
 *     methods to modify the debug level of each debug area
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/010/2005   Yushu Shi  Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_DEBUGLEVEL
#include "DebugLevel.h"
#endif

#ifdef __WIN32_SIM__ //Windows
#ifndef _INC_STRING
#include "string.h"
#endif
#else //VxWorks
#ifndef __INCstringh
#include <string.h>
#endif
#endif

#ifdef __WIN32_SIM__
#ifndef _INC_STDIO
#include <stdio.h>
#endif
#else
#ifndef __INCstdioh
#include <stdio.h>
#endif
#endif

// constructor of the DebugLevel class
CDebugLevel::CDebugLevel(LOGLEVEL level, const char *lpszAreaName, const char *shellName)
:m_lvlLevel(M_LL_SEVERE)
{
    SetDebugLevel(level);
    ::memcpy(m_szName, lpszAreaName, AREA_NAME_LEN);
    m_szName[AREA_NAME_LEN] = '\0';   // end of a string
    ::memcpy(m_shellName, shellName, SHELL_NAME_LEN);
    m_shellName[SHELL_NAME_LEN] = '\0';  
}

void CDebugLevel::SetDebugLevel(LOGLEVEL lvlNew)
{
    if (lvlNew < M_LL_LEVELS)
        m_lvlLevel = lvlNew;
}

char DebugLevelName[M_LL_LEVELS][16] = 
{
    "M_LL_CRITICAL",
    "M_LL_SEVERE  ",
    "M_LL_MAJOR   ",
    "M_LL_MINOR   ",
    "M_LL_WARN    ",
    "M_LL_DEBUG0  ",
    "M_LL_DEBUG1  ",
    "M_LL_DEBUG2  ",
    "M_LL_DEBUG3  "
};

void CDebugLevel::Show()
{
    ::fprintf(stdout,"%s level: %s (shell name: %s)\n",m_szName, DebugLevelName[m_lvlLevel], m_shellName);
}

// define the debugLevel 
CDebugLevel g_debugLevel[LOG_AI_MAX] =
{
    CDebugLevel(M_LL_SEVERE, "OS      ", "aios"),   //    LOG_AI_OS  =  0x00,
#if defined M_TGT_L3 || defined L2_TX
    CDebugLevel(M_LL_MAJOR, "SYS     ", "aisys"),   //    LOG_AI_SYS,  //0x01
    CDebugLevel(M_LL_DEBUG2, "CM      ", "aicm"),    //    LOG_AI_CM,      
    CDebugLevel(M_LL_MAJOR, "SM      ", "aism"),    //    LOG_AI_SM,      
    CDebugLevel(M_LL_MAJOR, "FM      ", "aialm"),    //    LOG_AI_FM,
    CDebugLevel(M_LL_MAJOR, "UM      ", "aium"),    //    LOG_AI_UM,                
    CDebugLevel(M_LL_MAJOR, "PM      ", "aipm"),    //    LOG_AI_PM,                
    CDebugLevel(M_LL_SEVERE, "EMSAGENT", "aiemsa"),  //    LOG_AI_EMSA,            
    CDebugLevel(M_LL_SEVERE, "BM      ", "aibm"),    //    LOG_AI_BM                 
    CDebugLevel(M_LL_WARN, "EB      ", "aieb"),    //    LOG_AI_EB,                
    CDebugLevel(M_LL_WARN, "SNOOP   ", "aisnoop"), //    LOG_AI_SNOOP,          
    CDebugLevel(M_LL_WARN, "ARP     ", "aiarp"),   //    LOG_AI_ARP,              
    CDebugLevel(M_LL_WARN, "DM      ", "aidm"),    //    LOG_AI_DM,            
////CDebugLevel(M_LL_WARN, "TDR     ", "aitdr"),   //    LOG_AI_TDR,              
////CDebugLevel(M_LL_WARN, "TCR     ", "aitcr"),   //    LOG_AI_TCR,              
    CDebugLevel(M_LL_WARN, "TUNNEL  ", "aitunnel"),//    LOG_AI_TUNNEL,        

   #ifdef WBBU_CODE

   CDebugLevel(M_LL_WARN, "RRU  ", "airru"),//    LOG_AI_RRU,        
   #endif
#ifdef __WIN32_SIM__
	CDebugLevel(M_LL_MAJOR, "DM      ", "aidm"),    //    LOG_AI_UTDM,
#endif
    CDebugLevel(M_LL_SEVERE, "GPS     ", "aigps"),   //    LOG_AI_GPS,     
    CDebugLevel(M_LL_DEBUG0, "DIAG    ", "aidiag"),  //    LOG_AI_DIAG,
    CDebugLevel(M_LL_SEVERE, "VOICE   ", "aivoice"), //    LOG_AI_DIAG,  
    CDebugLevel(M_LL_SEVERE, "GRPSRV  ", "aigrp"), //LOG_AI_GRPSRV,	//集群业务btsL3专用
    CDebugLevel(M_LL_SEVERE, "SAG  ", "aisag"), //20091101 add by fengbing
    CDebugLevel(M_LL_SEVERE, "DATAGRP  ", "aidgrp"), //20091101 add by fengbing
#endif    

#if defined M_TGT_L3 || defined L2_TX
    CDebugLevel(M_LL_SEVERE, "PCIIF   ", "ail2if"),    //     LOG_AI_FRMWK
#else
    CDebugLevel(M_LL_SEVERE, "PCIIF   ", "ail3if"),    //     LOG_AI_FRMWK
#endif

#if defined M_TGT_L2 || defined M_TGT_L3
    CDebugLevel(M_LL_WARN, "L2MAC   ", "aimac"),   //    LOG_AI_L2MAC,             
    CDebugLevel(M_LL_WARN, "L2DAC   ", "aidac"),   //    LOG_AI_L2DAC              
    CDebugLevel(M_LL_WARN, "L2VAC   ", "aivac"),   //    LOG_AI_L2VAC,             
    CDebugLevel(M_LL_SEVERE, "L2DIAG  ", "ail2diag"),//    LOG_AI_L2DIAG,         
    CDebugLevel(M_LL_SEVERE, "RRM     ", "airrm"),//    LOG_AI_L2DIAG,         
    CDebugLevel(M_LL_SEVERE, "L1IF    ", "ail1if"),  //    LOG_AI_L1IF,        
    CDebugLevel(M_LL_SEVERE, "L2GRP   ", "ail2grp"),//LOG_AI_L2GRP,   //集群业务btsL2专用
#endif

#ifdef M_TGT_CPE
    CDebugLevel(M_LL_SEVERE, "SYS     ", "aisys"),   //     LOG_AI_SYS,
    CDebugLevel(M_LL_SEVERE, "CM      ", "aicm"),    //     LOG_AI_CM,                     
    CDebugLevel(M_LL_SEVERE, "SM      ", "aism"),    //     LOG_AI_SM,                     
    CDebugLevel(M_LL_SEVERE, "DIAG    ", "aidiag"),  //     LOG_AI_DIAG,                 
    CDebugLevel(M_LL_SEVERE, "VOICE   ", "aivoice"), //     LOG_AI_VOICE,               
    CDebugLevel(M_LL_SEVERE, "UTDM    ", "aiutdm"),  //     LOG_AI_UTDM,                 
    CDebugLevel(M_LL_SEVERE, "EMBEDDED", "aiemb"),   //     LOG_AI_EMBEDDED
#endif 
    CDebugLevel(M_LL_SEVERE, "FRMWORK ", "aifmk"),    //     LOG_AI_FRMWK
    CDebugLevel(M_LL_SEVERE, "CSI     ", "aicsi")
};

#ifdef  WBBU_CODE
//CDebugLevel g_debugLevel[LOG_AI_MAX] ;
extern "C"  void initdbglvl()
{

#if 0
// define the debugLevel 
g_debugLevel[1] =CDebugLevel(M_LL_MAJOR/*M_LL_MAJOR*/, "SYS     ", "aisys");



g_debugLevel[0]=CDebugLevel(M_LL_SEVERE, "OS      ", "aios");   //    LOG_AI_OS  =  0x00,
#ifdef M_TGT_L3

g_debugLevel[2]=CDebugLevel(M_LL_MAJOR, "CM      ", "aicm");   //    LOG_AI_CM,      
g_debugLevel[3]=CDebugLevel(M_LL_MAJOR, "SM      ", "aism");    //    LOG_AI_SM,      
g_debugLevel[4]=CDebugLevel(M_LL_MAJOR, "FM      ", "aialm");    //    LOG_AI_FM,
g_debugLevel[5]=CDebugLevel(M_LL_MAJOR, "UM      ", "aium");    //    LOG_AI_UM,                
g_debugLevel[6]=CDebugLevel(M_LL_MAJOR, "PM      ", "aipm");    //    LOG_AI_PM,                
g_debugLevel[7]=CDebugLevel(M_LL_MAJOR/*M_LL_SEVERE*/, "EMSAGENT", "aiemsa");  //    LOG_AI_EMSA,            
g_debugLevel[8]=CDebugLevel(M_LL_SEVERE, "BM      ", "aibm");    //    LOG_AI_BM                 
g_debugLevel[9]=CDebugLevel(M_LL_WARN, "EB      ", "aieb");    //    LOG_AI_EB,                
g_debugLevel[10]=CDebugLevel(M_LL_WARN, "SNOOP   ", "aisnoop"); //    LOG_AI_SNOOP,          
g_debugLevel[11]=CDebugLevel(M_LL_WARN, "ARP     ", "aiarp");   //    LOG_AI_ARP,              
g_debugLevel[12]=CDebugLevel(M_LL_WARN, "DM      ", "aidm");   //    LOG_AI_DM,            
////CDebugLevel(M_LL_WARN, "TDR     ", "aitdr"),   //    LOG_AI_TDR,              
////CDebugLevel(M_LL_WARN, "TCR     ", "aitcr"),   //    LOG_AI_TCR,              
g_debugLevel[13]=CDebugLevel(M_LL_WARN, "TUNNEL  ", "aitunnel");//    LOG_AI_TUNNEL,        

g_debugLevel[14]=CDebugLevel(M_LL_SEVERE, "GPS     ", "aigps");   //    LOG_AI_GPS,     
g_debugLevel[15]=CDebugLevel(M_LL_DEBUG0, "DIAG    ", "aidiag");  //    LOG_AI_DIAG,
g_debugLevel[16]=CDebugLevel(M_LL_SEVERE, "VOICE   ", "aivoice"); //    LOG_AI_DIAG,
g_debugLevel[17]=CDebugLevel(M_LL_SEVERE, "PCIIF   ", "ail2if");    //     LOG_AI_FRMWK
#endif    


g_debugLevel[18]=CDebugLevel(M_LL_SEVERE, "FRMWORK ", "aifmk");    //     LOG_AI_FRMWK
g_debugLevel[19]=CDebugLevel(M_LL_SEVERE, "CSI     ", "aicsi");
#endif
   g_debugLevel[0]=CDebugLevel(M_LL_SEVERE, "OS      ", "aios");  //    LOG_AI_OS  =  0x00,
#ifdef M_TGT_L3
    g_debugLevel[1]=CDebugLevel(M_LL_MAJOR, "SYS     ", "aisys");   //    LOG_AI_SYS,  //0x01
    g_debugLevel[2]=CDebugLevel(M_LL_WARN, "CM      ", "aicm");  //    LOG_AI_CM,      
    g_debugLevel[3]=CDebugLevel(M_LL_MAJOR, "SM      ", "aism");   //    LOG_AI_SM,      
    g_debugLevel[4]=CDebugLevel(M_LL_MAJOR, "FM      ", "aialm");    //    LOG_AI_FM,
    g_debugLevel[5]=CDebugLevel(M_LL_MAJOR, "UM      ", "aium");  //    LOG_AI_UM,                
    g_debugLevel[6]=CDebugLevel(M_LL_MAJOR, "PM      ", "aipm");  //    LOG_AI_PM,                
    g_debugLevel[7]=CDebugLevel(M_LL_SEVERE, "EMSAGENT", "aiemsa"); //    LOG_AI_EMSA,            
    g_debugLevel[8]=CDebugLevel(M_LL_SEVERE, "BM      ", "aibm");    //    LOG_AI_BM                 
    g_debugLevel[9]=CDebugLevel(M_LL_SEVERE, "EB      ", "aieb");    //    LOG_AI_EB,                
    g_debugLevel[10]=CDebugLevel(M_LL_WARN, "SNOOP   ", "aisnoop");//    LOG_AI_SNOOP,          
    g_debugLevel[11]=CDebugLevel(M_LL_WARN, "ARP     ", "aiarp");  //    LOG_AI_ARP,              
    g_debugLevel[12]=CDebugLevel(M_LL_WARN, "DM      ", "aidm");    //    LOG_AI_DM,            
////CDebugLevel(M_LL_WARN, "TDR     ", "aitdr"),   //    LOG_AI_TDR,              
////CDebugLevel(M_LL_WARN, "TCR     ", "aitcr"),   //    LOG_AI_TCR,              
    g_debugLevel[13]=CDebugLevel(M_LL_WARN, "TUNNEL  ", "aitunnel");//    LOG_AI_TUNNEL,        
#if 0
	CDebugLevel(M_LL_MAJOR, "DM      ", "aidm"),    //    LOG_AI_UTDM,
#endif
    g_debugLevel[14]=CDebugLevel(M_LL_SEVERE, "GPS     ", "aigps");   //    LOG_AI_GPS,     
    g_debugLevel[15]=CDebugLevel(M_LL_WARN, "DIAG    ", "aidiag");  //    LOG_AI_DIAG,
    g_debugLevel[16]=CDebugLevel(M_LL_SEVERE, "VOICE   ", "aivoice"); //    LOG_AI_DIAG,
    g_debugLevel[17]=CDebugLevel(M_LL_SEVERE, "GRPSRV  ", "aigrp");//LOG_AI_GRPSRV,	//集群业务btsL3专用
    g_debugLevel[18]=CDebugLevel(M_LL_SEVERE, "PCIIF   ", "ail2if");   //     LOG_AI_FRMWK
#endif    

#if 0                                 
    CDebugLevel(M_LL_SEVERE, "L2MAC   ", "aimac"),   //    LOG_AI_L2MAC,             
    CDebugLevel(M_LL_SEVERE, "L2DAC   ", "aidac"),   //    LOG_AI_L2DAC              
    CDebugLevel(M_LL_SEVERE, "L2VAC   ", "aivac"),   //    LOG_AI_L2VAC,             
    CDebugLevel(M_LL_SEVERE, "L2DIAG  ", "ail2diag"),//    LOG_AI_L2DIAG,         
    CDebugLevel(M_LL_SEVERE, "RRM     ", "airrm"),//    LOG_AI_L2DIAG,         
    CDebugLevel(M_LL_SEVERE, "L1IF    ", "ail1if"),  //    LOG_AI_L1IF,             
    CDebugLevel(M_LL_SEVERE, "PCIIF   ", "ail3if"),    //     LOG_AI_FRMWK
    CDebugLevel(M_LL_SEVERE, "L2GRP   ", "aigrp"),//LOG_AI_L2GRP,	//集群业务btsL2专用
#endif

#if 0
    CDebugLevel(M_LL_SEVERE, "SYS     ", "aisys"),   //     LOG_AI_SYS,
    CDebugLevel(M_LL_SEVERE, "CM      ", "aicm"),    //     LOG_AI_CM,                     
    CDebugLevel(M_LL_SEVERE, "SM      ", "aism"),    //     LOG_AI_SM,                     
    CDebugLevel(M_LL_SEVERE, "DIAG    ", "aidiag"),  //     LOG_AI_DIAG,                 
    CDebugLevel(M_LL_SEVERE, "VOICE   ", "aivoice"), //     LOG_AI_VOICE,               
    CDebugLevel(M_LL_SEVERE, "UTDM    ", "aiutdm"),  //     LOG_AI_UTDM,                 
    CDebugLevel(M_LL_SEVERE, "EMBEDDED", "aiemb"),   //     LOG_AI_EMBEDDED
#endif 
    g_debugLevel[19]=CDebugLevel(M_LL_SEVERE, "FRMWORK ", "aifmk");  //     LOG_AI_FRMWK
    g_debugLevel[20]=CDebugLevel(M_LL_SEVERE, "CSI     ", "aicsi");
     g_debugLevel[21] = CDebugLevel(M_LL_SEVERE, "SAG  ", "aisag"); //20091101 add by fengbing
     g_debugLevel[22] = CDebugLevel(M_LL_SEVERE, "DATAGRP  ", "aidgrp"); //20091101 add by fengbing
#ifdef WBBU_CODE
      g_debugLevel[23] = CDebugLevel(M_LL_SEVERE, "RRU  ", "airru"); //201110228 add by wangwenhua
#endif

}
#endif
// define the ai_** to stand for the log area ID in shell commands
UINT32 aios            = LOG_AI_OS;
UINT32 aifmk           = LOG_AI_FRMWK ;
UINT32 aicsi           = LOG_AI_CSI;
#if defined M_TGT_L3 || defined L2_TX
UINT32 aisys           = LOG_AI_SYS;
UINT32 aicm            = LOG_AI_CM;
UINT32 aism            = LOG_AI_SM;
UINT32 aialm           = LOG_AI_FM;
UINT32 aium            = LOG_AI_UM;
UINT32 aipm            = LOG_AI_PM;
UINT32 aiemsa          = LOG_AI_EMSA;
UINT32 aibm            = LOG_AI_BM;
UINT32 aieb            = LOG_AI_EB;
UINT32 aisnoop         = LOG_AI_SNOOP;
UINT32 aiarp           = LOG_AI_ARP;
UINT32 aidm            = LOG_AI_DM;
//UINT32 aitdr           = LOG_AI_TDR;
//UINT32 aitcr           = LOG_AI_TCR;
UINT32 aitunnel        = LOG_AI_TUNNEL;

#ifdef WBBU_CODE
UINT32 airru = LOG_AI_RRU; //wangwenhua add 20110228 for RRU debug
#endif
//UINT32 aicleanup       = LOG_AI_CLEANUP;
UINT32 aigps           = LOG_AI_GPS;
UINT32 aidiag          = LOG_AI_DIAG;
UINT32 aivoice         = LOG_AI_VOICE;
UINT32 aigrp            = LOG_AI_GRPSRV;	//集群业务btsL3使用
UINT32 aisag			= LOG_AI_SAG;//20091101 add by fengbing
UINT32 aidgrp		=LOG_AI_DGRPSRV;//20091101 add by fengbing
#endif

#if defined M_TGT_L3 || defined L2_TX
UINT32 ail2if         = LOG_AI_PCIIF;
#else
UINT32 ail3if          = LOG_AI_PCIIF;
#endif

#if defined M_TGT_L2 || defined M_TGT_L3
UINT32 aimac           = LOG_AI_L2MAC;
UINT32 aidac           = LOG_AI_L2DAC;
UINT32 aivac           = LOG_AI_L2VAC;
UINT32 ail2diag        = LOG_AI_L2DIAG;
UINT32 airrm           = LOG_AI_RRM;
UINT32 ail1if          = LOG_AI_L1IF;
UINT32 ail2grp         = LOG_AI_L2GRP;	//集群业务btsL2使用
#endif

#ifdef M_TGT_CPE
UINT32 aisys           = LOG_AI_SYS;
UINT32 aicm            = LOG_AI_CM;
UINT32 aism            = LOG_AI_SM;
UINT32 aidiag          = LOG_AI_DIAG;
UINT32 aivoice         = LOG_AI_VOICE;
UINT32 aiutdm          = LOG_AI_UTDM;
//UINT32 aimac           = LOG_AI_L2MAC;
//UINT32 aidac           = LOG_AI_L2DAC;
//UINT32 aivac           = LOG_AI_L2VAC;
UINT32 aiemb           = LOG_AI_EMBEDDED;
#endif

#define LOG_LEVEL_CONSOLE_BASE 100

extern "C"
{
UINT32 llcri  = LOG_LEVEL_CONSOLE_BASE + M_LL_CRITICAL;
UINT32 llsev  = LOG_LEVEL_CONSOLE_BASE + M_LL_SEVERE;
UINT32 llmaj  = LOG_LEVEL_CONSOLE_BASE + M_LL_MAJOR;
UINT32 llmin  = LOG_LEVEL_CONSOLE_BASE + M_LL_MINOR;
UINT32 llwarn = LOG_LEVEL_CONSOLE_BASE + M_LL_WARN;
UINT32 lldbg0 = LOG_LEVEL_CONSOLE_BASE + M_LL_DEBUG0;
UINT32 lldbg1 = LOG_LEVEL_CONSOLE_BASE + M_LL_DEBUG1;
UINT32 lldbg2 = LOG_LEVEL_CONSOLE_BASE + M_LL_DEBUG2;
UINT32 lldbg3 = LOG_LEVEL_CONSOLE_BASE + M_LL_DEBUG3;
}


extern "C" bool setDbgLvl(UINT32 ai, unsigned int  consoleLvl)
{
    bool rc = false;

    if (ai >= LOG_AI_MAX)
    {
        ::fprintf(stdout,"invalid log area ID\n");
        return rc;
    }

    if ((consoleLvl >= llcri) && (consoleLvl <= lldbg3))
    {
        g_debugLevel[ai].SetDebugLevel(LOGLEVEL(consoleLvl - LOG_LEVEL_CONSOLE_BASE));
        rc = ((UINT32) g_debugLevel[ai].GetDebugLevel()) >=0 ;
    }
    else
    {
        ::fprintf(stdout,"\nINPUT ERROR: Please use valid symbols\n");
        //cout.flush();
        rc = false;
    }
	
    return (rc);
}


extern "C" bool dbgLvlAll(unsigned int  consoleLvl)
{
   for (unsigned int ai=0; ai<LOG_AI_MAX; ai++)
   {
      if (setDbgLvl(ai, consoleLvl) == false)
         return (false);
   }

   return true;
}

extern "C" bool dbgLvlShow() 
{
    //LOGLEVEL  logLvl;
    //unsigned int  consoleLvl;
    //char          levelStr[20];
    bool          rc = true;

    ::fprintf(stdout,"\n");
    for (unsigned int ai=0; ai<LOG_AI_MAX; ai++)
    {
        g_debugLevel[ai].Show();
    }
    ::fprintf(stdout,"\n");
    //cout.flush();

    return (rc);
}
   

extern "C" bool dbgLvlList()
{
    char list[]=" valid debug level symbols are: \n" \
"       llcri  -- M_LL_CRITICAL\n" \
"       llsev  -- M_LL_SEVERE\n"
"       llmaj  -- M_LL_MAJOR\n"
"       llmin  -- M_LL_MINOR\n"
"       llwarn -- M_LL_WARN\n"
"       lldbg0 -- M_LL_DEBUG0\n"
"       lldbg1 -- M_LL_DEBUG1\n"
"       lldbg2 -- M_LL-DEBUG2\n"
"       lldbg3 -- M_LL_DEBUG3\n";
    ::fprintf(stdout,"%s",list);

    return true;
}

