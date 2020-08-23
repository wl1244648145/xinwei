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

#include <string.h>

#ifndef _INC_L3OAMCPEFSM
#include "L3OamCpeFSM.h"
#endif

#ifndef _INC_L3OAMCPESTRANS
#include "L3OamCpeTrans.h"
#endif

#ifndef  _INC_L3OAMCPESTATE
#include "L3OamCpeState.h"
#endif

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3EMSMESSAGEID
#include "L3MessageId.h"
#endif

#ifndef _INC_TIMEOUTNOTIFY
#include "TimeOutNotify.h"
#endif

#ifndef _INC_L3OAMCPELOCATIONNOTIFY
#include "L3OamCpeLocationNotify.h"     
#endif

#ifndef _INC_L3OAMPROFUPDATEREQ
#include "L3OamCpeProfUpdateReq.h"
#endif

#ifndef _INC_L3OAMCPEREGNOTIFY
#include "L3oamCpeRegNotify.h"
#endif

#ifndef _INC_L3OAMCPESWITCHOFFNOTIFY
#include "L3OamCpeSwitchOffNotify.h"    
#endif

#ifndef _INC_L3OAMCOMMONRSP
#include "L3OamCommonRsp.h"
#endif

#ifndef _INC_OAML3CPEM
#include "L3OamCpeM.h"
#endif

UINT8  CPECCB::getAdminStatus()const
{
    return m_UTProfile.UTProfileIEBase.AdminStatus;
}
void   CPECCB::setAdminStatus(const UINT8 &status)
{
    m_UTProfile.UTProfileIEBase.AdminStatus = status;
}
UINT8  CPECCB::getMobility()const
{
    return m_UTProfile.UTProfileIEBase.Mobility;
}
UINT8  CPECCB::getDHCPrenew()const
{
    return m_UTProfile.UTProfileIEBase.DHCPRenew;
}
UINT16 CPECCB::getVlanID()const
{
    return m_UTProfile.UTProfileIEBase.WLANID;
}
UINT16 CPECCB::getMaxIPnum()const
{
    return m_UTProfile.UTProfileIEBase.MaxIpNum;
}
UINT16 CPECCB::getFixIpnum()const
{
    return m_UTProfile.UTProfileIEBase.FixIpNum;
}
UINT8  CPECCB::getPerfLogStatus()const
{
    return m_UTProfile.UTProfileIEBase.LogStatus;
}
UINT16 CPECCB::getPerfDataCollectInterval()const
{
    return m_UTProfile.UTProfileIEBase.DataCInter;
}
UINT32 CPECCB::getVoicePortMask()const
{
    return m_UTProfile.UTProfileIEBase.VoicePortMask;
}
const T_UTSDCfgInfo& CPECCB::getUTSDCfg()const
{
    return m_UTProfile.UTProfileIEBase.UTSDCfgInfo;
}
const T_CpeFixIpInfo* CPECCB::getFixIPinfo()const
{
    return m_UTProfile.CpeFixIpInfo;
}

void CPECCB::setUTProfile(const T_UTProfile &profile)
{
    UINT32 len = profile.length();
    memcpy(&m_UTProfile, &profile, len);
#if 0
    if (m_UTProfile.UTProfileIEBase.FixIpNum > MAX_FIX_IP_NUM)
        m_UTProfile.UTProfileIEBase.FixIpNum = MAX_FIX_IP_NUM;
#define profile_PrdRegTime (m_UTProfile.UTProfileIEBase.ucPeriodicalRegisterTimerValue)
    if ((0 == profile_PrdRegTime)||(profile_PrdRegTime > gMAXPrdRegTime))
        {
        profile_PrdRegTime = gDefaultPrdRegTime;
        }
#endif
}

void CPECCB::setUTProfile(const T_UTProfileNew &newProfile)
{
    if ((CPE_ADM_STATUS_ADM != newProfile.ucAdminStatus)&&(newProfile.ucAdminStatus!=CPE_ADM_STATUS_FLOW_LIMITED)&&(newProfile.ucAdminStatus!=CPE_ADM_STATUS_NOPAY))
        {
        m_UTProfile.UTProfileIEBase.AdminStatus = newProfile.ucAdminStatus;
        }
    else
        {
        T_UTProfile Profile;
        newProfile.convert2(Profile);
        memcpy(&m_UTProfile, &Profile, Profile.length());
        }
}
void  CPECCB::setUTHoldBW(const T_UT_HOLD_BW &HoldBW)//wangwenhua 20080916
{
    memcpy(&UTHoldBW,&HoldBW,4);
}
void CPECCB::setMemCfg(const T_MEM_CFG&memCfg)
{
	memcpy(&m_MemCfg, &memCfg, sizeof(T_MEM_CFG));	 
}
const T_MEM_CFG & CPECCB::getMemCfg()const
{
    return m_MemCfg;
}
const T_UTProfile & CPECCB::getUTProfile()const
{
    return m_UTProfile;
}
 bool CPECCB::MemCfgCompare(const T_MEM_CFG &memCfg)
  {
     SINT8 *src = (SINT8*)&m_MemCfg;
     SINT8 *dst = (SINT8*)&memCfg;
     if(memcmp(src,dst,sizeof(T_MEM_CFG)))
     	{
	     return false;
     	}
	 return true;

	 
  }
bool CPECCB::ProfileCompare(const T_UTProfileNew &profileFromEMS)
{
    T_UTProfile tempProfile;
    profileFromEMS.convert2(tempProfile);
    UINT32 len = m_UTProfile.length();
    if (len != tempProfile.length())
        return false;
    SINT8 *src = (SINT8*)&m_UTProfile;
    SINT8 *dst = (SINT8*)&tempProfile;
    for(UINT32 idx = 0; idx < len; ++idx)
        {
        if (src[idx] != dst[idx])
            return false;
        }
    return true;
}

  bool CPECCB::HoldBWCompare(const T_UT_HOLD_BW &bw)
  {
     SINT8 *src = (SINT8*)&UTHoldBW;
     SINT8 *dst = (SINT8*)&bw;
     if(memcmp(src,dst,4))
     	{
	 return false;
     	}
	 return true;

	 
  }
bool CPECCB::ProfileCompare(const T_UTProfile &profileFromCPE)
{
    UINT32 len = m_UTProfile.length();
    if (len != profileFromCPE.length())
        return false;
    SINT8 *src = (SINT8*)&m_UTProfile;
    SINT8 *dst = (SINT8*)&profileFromCPE;
    for(UINT32 idx = 0; idx < len; ++idx)
        {
        if (src[idx] != dst[idx])
            return false;
        }
    return true;
}

bool CPECCB::setUTBaseInfo(const T_CpeBaseInfo &BaseInfo)
{
////m_UTBaseInfo.ulCID   = BaseInfo.NetworkID;
    m_UTBaseInfo.usHWtype = BaseInfo.HardwareType;
    m_UTBaseInfo.ucSWtype = BaseInfo.SoftwareType;
    m_UTBaseInfo.rsv      = 0;
    m_UTBaseInfo.ulActiveSWversion = BaseInfo.ActiveSWVer;
    m_UTBaseInfo.ulStandbySWversion = BaseInfo.StandbySWVer;
    memcpy(m_UTBaseInfo.ucHWversion, BaseInfo.HWVer, sizeof(BaseInfo.HWVer));
    return true;
}

bool CPECCB::setUTBaseInfo(const T_UTBaseInfo &BaseInfo)
{
    memcpy(&m_UTBaseInfo, &BaseInfo, sizeof(T_CpeBaseInfo));
    return true;
}
#if 0
void CPECCB::getCpeBaseInfo(T_CpeBaseInfo& BaseInfo)const  
{
    BaseInfo.NetworkID = 0;
    memcpy(&BaseInfo.HardwareType, &m_UTBaseInfo.ucHWtype, sizeof(BaseInfo)-sizeof(BaseInfo.NetworkID));
}
#endif

const T_UTBaseInfo& CPECCB::getCpeBaseInfo()const  
{
    return m_UTBaseInfo;
}

#if 0
void CPECCB :: SetIsToDelete(bool flag)
{
    IsToDelete = flag;
}
#endif

void CPECCB::setZmoduleBlock(T_ZmoduleBlock *pZB)
{
    memcpy(&m_ZmoduleBlock, pZB, sizeof(m_ZmoduleBlock));
}


T_ZmoduleBlock* CPECCB::getZmoduleBlock()
{
    return &m_ZmoduleBlock;
}
//wangwenhua add 20081119
  bool CPECCB::setCfgTranid(const UINT16 transid)
  {
      m_CFG_TransID = transid;
       return true;
  }
   bool CPECCB::setQueryTransid(const UINT16 transid)
  {
       m_Query_TransID = transid;
  	 return true;
   }
   UINT16 CPECCB::getCfgTranid()
   {
     return  m_CFG_TransID;
   }
   UINT16 CPECCB::getQueryTransid()
   {
      return  m_Query_TransID;
   }

   //wangwenhua add end 20081119
/*
 *����ͻ���Ե�CPEע������,ϵͳ��ʱ��1s�ڽ��ܵ���ͬһCPE�����ĺܶ�ע������
 *return
 * false:���������ע��
 * true :����ע��
 */
extern UINT32 gVx_System_Clock;    /*ϵͳ�������е�ʱ��(��)*/
bool CPECCB::getRegisterStatus(const UINT16 transId)
{
    if ((gVx_System_Clock - m_ulRegTime < 2) && (transId == m_RegisterReqTransId))
        {
        //����ͻ���Ե�CPEע������,ϵͳ��ʱ��1s�ڽ��ܵ���ͬһCPE�����ĺܶ�ע������
        return false;
        }
    m_ulRegTime          = gVx_System_Clock;
    m_RegisterReqTransId = transId;
    return true;
}


CCBBase* CPECCBTable :: FindCCB(CMessage &rMsg)
{
    //����msg�õ�cpe eid
    UINT16 MsgID = rMsg.GetMessageId();
    UINT32 CpeId;    

     // �� BTS ������Ϣ����ֱ�ӻ�ȡ CPEID,����ת�ɶ�Ӧ����Ϣ��ʽ    
    switch(MsgID)
    {
        case M_EMS_BTS_RPTONOFF_REQ:
        case M_EMS_BTS_RPTCFG_REQ:
        case M_EMS_BTS_RPTGET_REQ:
//        case M_CPEM_RPT_CFG_TIMEOUT:
//        case M_CPEM_RPT_GET_TIMEOUT:
        {   
            CL3_EMS2L3_GetReq Msg(rMsg);   
            CpeId = Msg.GetRptPID();
		    OAM_LOGSTR2(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] FindCCB:[%04X]:[%08x]", MsgID, CpeId );
            break;
        }        
        case M_EMS_BTS_BWINFO_RSP:
		{
            T_EmsModBWInfoRsp* pRsp = (T_EmsModBWInfoRsp*)((UINT8*)rMsg.GetDataPtr()+4);   
            CpeId = pRsp->ulUID;
		    OAM_LOGSTR2(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] FindCCB:[%04X]:[%08x]", MsgID, CpeId );
            break;
		}
        case M_EMS_BTS_PROBE_UT_REQ:
        {   
            CCpeReq Msg(rMsg);   
            CpeId = Msg.GetCPEID();
            break;
        }        
        case M_EMS_BTS_RESET_UT_REQ:
        {   
            CCpeReq Msg(rMsg);   
            CpeId = Msg.GetCPEID();
            break;
        }
#if 0
        case M_EMS_BTS_UT_MOVEAWAY_NOTIFY:
        {
            CCpeLocationNotify Msg(rMsg); 
            CpeId = Msg.GetCPEID();
            break;
        }
 #else
        case M_EMS_BTS_UT_PROFILE_UPDATE_REQ:
        {   
            CCpeProfUpdateReq Msg(rMsg);
            CpeId = Msg.GetCPEID();
            break;
        }        
        case MSGID_BWINFO_RSP:
        {
            CsabisBWInfoRsp msgBWInfoRsp(rMsg);
            CpeId = msgBWInfoRsp.getInfo()->ulPID;
            break;
        }
        case MSGID_BWINFO_DEL_REQ:
        {
            CsabisDeleteBWInfoReq msgDelBWInfoReq(rMsg);
            CpeId = msgDelBWInfoReq.getInfo()->ulPID;
            break;
        }
        case MSGID_BWINFO_UPDATE_REQ:
        {
            CsabisModifyBWInfoReq msgModifyBWInfoReq(rMsg);
            CpeId = msgModifyBWInfoReq.getInfo()->ulPID;
            break;
        }
#endif
       default:     // �� CPE ������Ϣ��ֱ�ӻ�ȡ CPEID
        {
            CpeId = rMsg.GetEID();
            break;
        }
    }

    //�������Ӧ����Ϣ,ͨ��EID����CCB
    CpeIter Iter = CpeCCBTable.find(CpeId);
    if(Iter != CpeCCBTable.end())
    {
        return (*Iter).second;
    }
 
    //���û���ҵ�ccb,��eid������,�������´���:����CCB ����ems����ʧ��
#if 0
    if((MsgID == M_CPE_L3_REG_NOTIFY)||
       (MsgID == M_EMS_BTS_UT_PROFILE_UPDATE_REQ))
#endif
    if((MsgID == M_CPE_L3_REG_NOTIFY)||
       (MsgID == M_EMS_BTS_UT_PROFILE_UPDATE_REQ)
        ||(MsgID == M_CPE_L3_REGISTER_REQ)
        ||(MsgID == MSGID_BWINFO_UPDATE_REQ)
        )
    {
        CPECCB *pEle = new CPECCB;  
        if (NULL == pEle)
            return NULL;
        CpeCCBTable.insert(CpeValType(CpeId, pEle));  //��ӵ� ccb table
        pEle->setEid(CpeId);
        if (MsgID == M_CPE_L3_REG_NOTIFY)
            pEle->setCCBType(M_CCB_TYPE_OLD);
        else
            pEle->setCCBType(M_CCB_TYPE_NEW);
        return pEle;
    }

    return NULL;
}

CCBBase* CPECCBTable :: FindCCB(const UINT32 eid)
{
    CpeIter it = CpeCCBTable.find(eid);
    if(it != CpeCCBTable.end())
        {
        return (*it).second;
        }
    return NULL;
}

void CPECCBTable :: DeleteCCB(UINT32 CpeID)
{
    CpeIter Iter = CpeCCBTable.find(CpeID);
    if(Iter != CpeCCBTable.end())
    {
        CpeCCBTable.erase(Iter);
    }
}


void CPEFSM::Reclaim(CCBBase *pCCB)
{
    OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] ->CPE fsm reclaim()");
    CPECCB *pCpeCCB = (CPECCB*)pCCB;
    if (CPE_NORECORD != pCpeCCB->GetCurrentState())
        {
        return;
        }

    UINT32 eid = pCpeCCB->getEid();
    //����Ӧ��CCB��ccb tableɾ��
    m_pCCBTable->DeleteCCB(eid);
	delete pCCB;
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Reclaim CPE[0x%08x]'s info", eid);
    return;
}


CPEFSM :: CPEFSM() : FSM(MAX_CPE_STATE, MAX_CPE_TRANS)
{
    //CCB table.
    m_pCCBTable = new CPECCBTable;

    //State table.
    m_pStateTable[CPE_NORECORD]   = new CPENorecordState;
    m_pStateTable[CPE_SERVING]    = new CPEServingState;
    m_pStateTable[CPE_WAITFOREMS] = new CPEWaitforemsState;
    m_pStateTable[CPE_SWITCHOFF]  = new CPESwitchoffState;
    m_pStateTable[CPE_MOVEDAWAY]  = new CPEMovedawayState;

    m_pTransTable[CPE_ANYSTATE_REG_NOTIFY]            = new CPEanyStateRegisterNotifyTrans(CPE_SERVING );
    m_pTransTable[CPE_ANYSTATE_REGISTER_REQ]          = new CPEanyStateRegisterReqTrans(CPE_SERVING );
    m_pTransTable[CPE_NORECORD_PROFILE_UPDATE_REQ]    = new CPENorecordProfileUpdateReqTrans( CPE_SERVING );
    m_pTransTable[CPE_NORECORD_ModifyBWInfo_REQ]      = new CPENorecordModifyBWInfoReqTrans(CPE_NORECORD);

////m_pTransTable[CPE_SERVING_REGISTER_REQ]           = new CPEServingRegisterNotifyTrans(CPE_SERVING);
    m_pTransTable[CPE_SERVING_PROFILE_UPDATE_REQ]     = new CPEServingProfileUpdateReqTrans(CPE_SERVING);
    m_pTransTable[CPE_SERVING_MOVED_AWAY_NOTIFY]      = new CPEServingMovedawayNotifyTrans(CPE_MOVEDAWAY);
    m_pTransTable[CPE_SERVING_CPE_PROFILE_UPDATE]     = new CPEServingCpeProfileUpdateNotifyTrans(CPE_SERVING);
    m_pTransTable[CPE_SERVING_ModifyBWInfo_REQ]       = new CPEServingModifyBWInfoReqTrans(CPE_SERVING);

//#ifdef RPT_FOR_V6
	m_pTransTable[CPE_SERVING_RPT_BWINFO_RSP] = new CPEServingRptBWInfoRspTrans(CPE_SERVING); 
    m_pTransTable[CPE_SERVING_RPT_RFONOFF_REQ] = new CPEServingRptRfOnOffReqTrans(CPE_SERVING);  
    m_pTransTable[CPE_SERVING_RPT_RFONOFF_RSP] = new CPEServingRptRfOnOffRspTrans(CPE_SERVING);  
    m_pTransTable[CPE_SERVING_RPT_CFG_REQ] = new CPEServingRptCfgReqTrans(CPE_SERVING);  
    m_pTransTable[CPE_SERVING_RPT_CFG_RSP] = new CPEServingRptCfgRspTrans(CPE_SERVING);  
    m_pTransTable[CPE_SERVING_RPT_Get_REQ] = new CPEServingRptGetReqTrans(CPE_SERVING);  
    m_pTransTable[CPE_SERVING_RPT_Get_RSP] = new CPEServingRptGetRspTrans(CPE_SERVING);  
    //m_pTransTable[CPE_SERVING_RPT_TIMEOUT] = new CPEServingRptTimeoutTrans(CPE_SERVING);  

////m_pTransTable[CPE_WAITFOREMS_REGISTER_REQ]        = new CPEWaitforemsRegisterNotifyTrans(CPE_WAITFOREMS);
    m_pTransTable[CPE_WAITFOREMS_PROFILE_UPDATE_REQ]  = new CPEWaitforemsProfileUpdateReqTrans(CPE_SERVING);
    m_pTransTable[CPE_REGISTER_TIMER]                 = new CPERegisterTimeOutTrans(CPE_NORECORD);
    m_pTransTable[CPE_WAITFOREMS_ModifyBWInfo_REQ]    = new CPEWaitforemsModifyBWInfoReqTrans(CPE_WAITFOREMS);

////m_pTransTable[CPE_MOVEDAWAY_REGISTER_REQ]         = new CPEMovedawayRegisterNotifyTrans(CPE_SERVING);

////m_pTransTable[CPE_SWITCHEDOFF_REGISTER_REQ]       = new CPESwitchoffRegisterNotifyTrans( CPE_SERVING);
    m_pTransTable[CPE_SWITCHEDOFF_PROFILE_UPDATE_REQ] = new CPESwitchoffProfileUpdateReqTrans( CPE_SWITCHOFF);
    m_pTransTable[CPE_SWITCHEDOFF_MOVEDAWAY_NOTIFY]   = new CPESwitchoffMovedawayNotifyTrans(CPE_MOVEDAWAY);
    m_pTransTable[CPE_SWITCHEDOFF_ModifyBWInfo_REQ]   = new CPESwitchoffModifyBWInfoReqTrans( CPE_SWITCHOFF);

    m_pTransTable[CPE_ANYSTATE_SWITCHEDOFF]           = new CPEAnyStateSwitchOffTrans(CPE_SWITCHOFF);
////Add for Z-Module Register
    m_pTransTable[CPE_Z_REGISTER_REQ]                 = new CPE_Z_RegisterNotifyTrans(CPE_SERVING);
}
