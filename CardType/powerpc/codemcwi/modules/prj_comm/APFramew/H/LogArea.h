/*****************************************************************************
               (C) Copyright 2005: Arrowping Telecom
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: LogArea.h
 *
 * DESCRIPTION:
 *
 *    Log Error Codes. These are qualified to be recorded in <errno> for each task
 *    Error Numbers are grouped by debug areas (DA) (8 bits). With 16 bits for
 *    error codes within each area.
 *    Refer to each functional areas' Log<fa>No.h file for error codes within that
 *    group.
 *    errno in Vxworks is a 32bit value, the first 16bit is used for module number
 *    and 1-500 is reserved for Vxworks. When we set the errno of each task, we need 
 *    to set the errno to be (log code | 0x8000) to indicate it's a user defines
 *    code.
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   Yushu Shi  Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_LOGAREA
#define _INC_LOGAREA

#ifndef _INC_STDHDR
#include "stdhdr.h"
#endif

typedef enum
{
    LOG_AI_OS  = 0,
#if (defined M_TGT_L3 || defined L2_TX) && (!defined M_TGT_CPE)
    LOG_AI_SYS,     //0x01
    LOG_AI_CM,      //2
    LOG_AI_SM,      //3   
    LOG_AI_FM,      //4 
    LOG_AI_UM,      //5
    LOG_AI_PM,      
    LOG_AI_EMSA,    
    LOG_AI_BM,
    LOG_AI_EB,
    LOG_AI_SNOOP,
    LOG_AI_ARP,
    LOG_AI_DM,
////LOG_AI_TDR,
////LOG_AI_TCR,
    LOG_AI_TUNNEL,

   #ifdef WBBU_CODE

    LOG_AI_RRU, //wangwenhua add 20110228
    #endif
   
    //for test//xiaoweifang
#ifdef __WIN32_SIM__
    LOG_AI_UTDM,
#endif
    LOG_AI_GPS,
    LOG_AI_DIAG,
    LOG_AI_VOICE,
    LOG_AI_GRPSRV,	//集群业务btsL3专用
    LOG_AI_SAG,	//20091101 add by fengbing
    LOG_AI_DGRPSRV,//20091101 add by fengbing
#endif

#if defined M_TGT_L2 || defined M_TGT_L3
    LOG_AI_L2MAC,
    LOG_AI_L2DAC,
    LOG_AI_L2VAC,
    LOG_AI_L2DIAG,
    LOG_AI_RRM,
    LOG_AI_L1IF,    
    LOG_AI_L2GRP,   //集群业务btsL2专用
#endif

#ifdef M_TGT_CPE  // BUILD_CPE
    LOG_AI_SYS,
    LOG_AI_CM,
    LOG_AI_SM,
    LOG_AI_DIAG,
    LOG_AI_VOICE,
    LOG_AI_UTDM,
    LOG_AI_EMBEDDED,
#endif
    LOG_AI_PCIIF,
    LOG_AI_FRMWK,
    LOG_AI_CSI,
    LOG_AI_MAX
}LogAreaID;


#define LOGNO(ai, num)    (UINT32)(((LOG_AI_##ai) << 16) | (num))

#define ERRNO_USR         0x80000000

#endif
