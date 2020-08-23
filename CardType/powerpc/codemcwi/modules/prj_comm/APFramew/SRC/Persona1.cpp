/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: PersonatorConfig.cpp
 *
 * DESCRIPTION:
 *     Definition of the CPersonator::s_TaskList[].
 *     Should be modified when taskdef.h changed.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 08/29/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifdef WIN32
#ifdef __WIN32_SIM__

#ifndef _INC_PERSONATOR
#include "Personator.h"
#endif

CPersonator::PersonatorTaskListEntry CPersonator::s_TaskList[] =
{
    {M_TID_CM, "tCM"},
    {M_TID_FM, "tFM"},
    {M_TID_PM, "tPM"},
    {M_TID_UM, "tUM"},
    {M_TID_SM, "tSM"},
    {M_TID_DIAGM, "tDiagM"},
    {M_TID_BM, "tBM"},
    {M_TID_GM, "tGM"},
    {M_TID_SYS, "tSys"},
    {M_TID_FTPCLIENT,"tFtpClient"},
    {M_TID_EMSAGENTTX, "tEmsAgentTx"},
    {M_TID_L2IF, "tL2AgentTx"},
    {M_TID_LOG, "tLog"},
    {M_TID_EB, "tEB"},
    {M_TID_SNOOP, "tSnoop"},
    {M_TID_TUNNEL, "tTunnel"},
    {M_TID_ARP, "tArp"},
    {M_TID_CLEANUP, "tCleanup"},
    {M_TID_DM, "tDM"},
    {M_TID_TCR, "tTCR"},
    {M_TID_TDR, "tTDR"},
    {M_TID_VOICE, "tVoice"},
    {M_TID_VCR, "tVCR"},
    {M_TID_VDR, "tVDR"},
////{M_TID_EMSAGENTRX, "tEmsAgentRx"},
    {M_TID_L2IF, "tL2IF"},
    {M_TID_DAC, "tDAC"},
    {M_TID_VAC, "tVAC"},
    {M_TID_DIAG, "tDiag"},
    {M_TID_L2MAIN, "tL2OAM"},
//    {M_TID_CPESYS, "tCPESYS"},
    {M_TID_CPECM, "tCPECM"},
    {M_TID_CPESM, "tCPESM"},
    {M_TID_CPEDIAG, "tCPEDIAG"},
    {M_TID_CPEPCIF, "tCPEPCIF"},
    {M_TID_CPEL1L2IF, "tCPEL1L2IF"},
    {M_TID_UTV, "tUTV"},
//    {M_TID_VSCAN, "tVScan"},
    {M_TID_UTDM, "tUTDM"},
    {M_TID_TT, "tTT"},
    {M_TID_MAX,""}
};

#else
#error "This module can only be used on Windows platform."
#endif //__WIN32_SIM__

#endif //WIN32
