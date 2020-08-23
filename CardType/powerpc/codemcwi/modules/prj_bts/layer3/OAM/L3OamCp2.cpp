/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef  _INC_L3OAMCPESTATE
#include "L3OamCpeState.h"
#endif

#ifndef _INC_L3CPEMESSAGEID
#include "L3CPEMessageId.h"
#endif

#ifndef _INC_L3OAMCPEFSM
#include "L3OamCpeFSM.h"
#endif

#ifndef _INC_ERRORCODEDEF
#include "ErrorCodeDef.h"
#endif

FSMTransIndex  CPENorecordState:: DoParseEvent(CMessage& rMsg)
{
    FSMTransIndex Index;
    switch(rMsg.GetMessageId())
    {
        case M_CPE_L3_REG_NOTIFY:
            Index = CPE_ANYSTATE_REG_NOTIFY;
            break;
        case M_EMS_BTS_UT_PROFILE_UPDATE_REQ:
        case MSGID_BWINFO_RSP:
            Index = CPE_NORECORD_PROFILE_UPDATE_REQ; 
            break;
        case MSGID_BWINFO_UPDATE_REQ:
            Index = CPE_NORECORD_ModifyBWInfo_REQ;
            break;
        case M_CPE_L3_REGISTER_REQ:
            Index = CPE_ANYSTATE_REGISTER_REQ;
            break;
        default: 
            Index = INVALID_EVENT;
            break;
    }

    return Index;
}

FSMTransIndex  CPEServingState:: DoParseEvent(CMessage& rMsg)
{
    FSMTransIndex Index;
    switch(rMsg.GetMessageId())
    {
        case M_CPE_L3_REG_NOTIFY :
            Index = CPE_ANYSTATE_REG_NOTIFY;
            break;
        case M_CPE_L3_REGISTER_REQ:
            Index = CPE_ANYSTATE_REGISTER_REQ;
            break;
        case M_EMS_BTS_UT_PROFILE_UPDATE_REQ:
        case MSGID_BWINFO_RSP:
            Index = CPE_SERVING_PROFILE_UPDATE_REQ; 
            break;
        case MSGID_BWINFO_UPDATE_REQ:
            Index = CPE_SERVING_ModifyBWInfo_REQ;
            break;
        //case M_EMS_BTS_UT_MOVEAWAY_NOTIFY:
        case MSGID_BWINFO_DEL_REQ:
            Index = CPE_SERVING_MOVED_AWAY_NOTIFY; 
            break;
        case M_L2_L3_CPE_PROFILE_UPDATE_NOTIFY:
            Index = CPE_SERVING_CPE_PROFILE_UPDATE;
            break;
        case M_CPE_BTS_Z_REGISTER_REQ:
            Index = CPE_Z_REGISTER_REQ;
            break;
        case M_CPE_L3_SWITCH_OFF_NOTIFY:
            Index = CPE_ANYSTATE_SWITCHEDOFF;
            break;
//#ifdef RPT_FOR_V6
        case M_EMS_BTS_BWINFO_RSP:
            OAM_LOGSTR(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] DoParseEvent->M_EMS_BTS_BWINFO_RSP m_testTmCnt[%04d]");
            Index =  CPE_SERVING_RPT_BWINFO_RSP;     
            break;  
        case M_EMS_BTS_RPTONOFF_REQ:
            OAM_LOGSTR(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] DoParseEvent->M_EMS_BTS_RPTONOFF_REQ m_testTmCnt[%04d]");
            Index =  CPE_SERVING_RPT_RFONOFF_REQ;     
            break;  
        case M_RPT_BTS_RPTONOFF_RSP:
            OAM_LOGSTR(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] DoParseEvent->M_RPT_BTS_RPTONOFF_RSP m_testTmCnt[%04d]");
            Index =  CPE_SERVING_RPT_RFONOFF_RSP;     
            break;
        case M_EMS_BTS_RPTCFG_REQ:
            OAM_LOGSTR(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] DoParseEvent->M_EMS_BTS_RPTCFG_REQ m_testTmCnt[%04d]");
            Index =  CPE_SERVING_RPT_CFG_REQ;     
            break;
        case M_RPT_BTS_RPTCFG_RSP:
            OAM_LOGSTR(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] DoParseEvent->M_RPT_BTS_RPTCFG_RSP m_testTmCnt[%04d]");
            Index =  CPE_SERVING_RPT_CFG_RSP;     
            break;
        case M_EMS_BTS_RPTGET_REQ:   
            OAM_LOGSTR(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] DoParseEvent->M_EMS_BTS_RPTGET_REQ m_testTmCnt[%04d]");
            Index =  CPE_SERVING_RPT_Get_REQ;     
            break;
        case M_RPT_BTS_RPTGET_RSP:   
            OAM_LOGSTR(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] DoParseEvent->M_RPT_BTS_RPTGET_RSP m_testTmCnt[%04d]");
            Index =  CPE_SERVING_RPT_Get_RSP;     
            break;
        default:
            Index = INVALID_EVENT;
            break;
    }
    
    return Index;
}

FSMTransIndex  CPEWaitforemsState :: DoParseEvent(CMessage& rMsg)
{
    FSMTransIndex Index;
    switch(rMsg.GetMessageId())
    {
        case M_CPE_L3_REG_NOTIFY:
            Index = CPE_ANYSTATE_REG_NOTIFY;
            break;
        case M_CPE_L3_REGISTER_REQ:
            Index = CPE_ANYSTATE_REGISTER_REQ;
            break;
        case M_EMS_BTS_UT_PROFILE_UPDATE_REQ:
        case MSGID_BWINFO_RSP:
            Index = CPE_WAITFOREMS_PROFILE_UPDATE_REQ; 
            break;
        case MSGID_BWINFO_UPDATE_REQ:
            Index = CPE_WAITFOREMS_ModifyBWInfo_REQ;
            break;
        case M_OAM_CPE_REGISTER_TIMER:
            Index = CPE_REGISTER_TIMER;
            break;
        case M_CPE_BTS_Z_REGISTER_REQ:
            Index = CPE_Z_REGISTER_REQ;
            break;
        case M_CPE_L3_SWITCH_OFF_NOTIFY:
            Index = CPE_ANYSTATE_SWITCHEDOFF;
            break;
        default:
            Index = INVALID_EVENT;
            break;
    }
    
    return Index;
}

FSMTransIndex  CPESwitchoffState:: DoParseEvent(CMessage& rMsg)
{
    FSMTransIndex Index;
    switch(rMsg.GetMessageId())
    {
        case M_CPE_L3_REG_NOTIFY:
            Index = CPE_ANYSTATE_REG_NOTIFY;
            break;
        case M_CPE_L3_REGISTER_REQ:
            Index = CPE_ANYSTATE_REGISTER_REQ;
            break;
        case M_EMS_BTS_UT_PROFILE_UPDATE_REQ:
        case MSGID_BWINFO_RSP:
            Index = CPE_SWITCHEDOFF_PROFILE_UPDATE_REQ; 
            break;
        case MSGID_BWINFO_UPDATE_REQ:
            Index = CPE_SWITCHEDOFF_ModifyBWInfo_REQ;
            break;
        //case M_EMS_BTS_UT_MOVEAWAY_NOTIFY:
        case MSGID_BWINFO_DEL_REQ:
            Index = CPE_SWITCHEDOFF_MOVEDAWAY_NOTIFY;
            break;
        default:
            Index = INVALID_EVENT;
            break;
    }

    return Index;
}

FSMTransIndex  CPEMovedawayState:: DoParseEvent(CMessage& rMsg)
{
    FSMTransIndex Index;
    switch(rMsg.GetMessageId())
    {
        case M_CPE_L3_REG_NOTIFY:
            Index = CPE_ANYSTATE_REG_NOTIFY;
            break;
        case M_CPE_L3_REGISTER_REQ:
            Index = CPE_ANYSTATE_REGISTER_REQ;
            break;
        case M_CPE_L3_SWITCH_OFF_NOTIFY:
            Index = CPE_ANYSTATE_SWITCHEDOFF;
            break;
        default:
            Index = INVALID_EVENT;
            break;
    }
    
    return Index;
}
