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
 * -------  ----------  ------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifdef __WIN32_SIM__
#include <windows.h>
#else
#ifndef __INCtaskLibh
#include <vxWorks.h>
#endif
#endif

//socket:
#ifdef __WIN32_SIM__
    #include <Winsock2.h>
    #include <ws2tcpip.h>
#else   //VxWorks:
    #include "vxWorks.h" 
    #include "sockLib.h" 
    #include "inetLib.h" 
    #include "stdioLib.h" 
    #include "strLib.h" 
    #include "hostLib.h" 
    #include "ioLib.h" 
    #include "selectlib.h"
#endif
#include <tasklib.h>
#include <string.h>
#include <stdio.h>

#include "MsgQueue.h"

#ifndef _INC_OAML3CPEM
#include "L3OamCpeM.h"
#endif

#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif

#ifndef _INC_L3OAMCOMMONRSP
#include "L3OamCommonRsp.h"
#endif

#include "L3OamCommonReq.h"

#ifndef _INC_L3OAMMESSAGEID
#include "L3OAMMessageId.h"
#endif

#ifndef _INC_L3OAMCOMMONRSPFROMCPE
#include "L3OamCommonRspFromCpe.h"
#endif

#ifndef _INC_L3L3L2CPEPROFUPDATENOTIFY
#include "L3L3L2CpeProfUpdateNotify.h"
#endif

#ifndef _INC_L3OAMCFGBTSNEIBLISTREQ
#include "L3OamCfgBtsNeibListReq.h" 
#endif
#include "L3OamCpeTrans.h"

#ifndef _INC_L3L3CPEETHCFGREQ
#include "L3L3CpeEthCfgReq.h"
#endif

#ifndef _INC_L3L3CPEETHCFGRSP
#include "L3L3CpeEthCfgRsp.h"
#endif
#ifdef WBBU_CODE
#include "L3DataArp.h"
#endif
//loadinginfo
#define M_SOCKET_LOADINGINFO_TX          0xABCD

CTaskCpeM* CTaskCpeM :: m_Instance = NULL;
#ifndef WBBU_CODE
list<T_ToBeDeleteCpe> CTaskCpeM::m_ToBeDeleteCpeList;
#endif
extern T_NvRamData *NvRamDataAddr;
extern "C" int bspGetBtsID();
#define L3OAM_DEFAULT_CPEID 0XFFFFFFFE//wangwenhua modify 20080715
T_BTSLoadInfo gL3OamBTSLoadInfo;
T_BTSLoadInfo gTestBTSLoadInfo;
bool          bDebugLoadInfoMode = false;
extern bool sagStatusFlag;
CPE_NETIF_ADA_STATUS CTaskCpeM::NetifAdaStatus = CPE_NETIF_ADA_INIT;
CPE_NETIF_ADA_STATUS CTaskCpeM::TempNetifAdaStatus = CPE_NETIF_ADA_INIT;
extern UINT32  RelayWanifCpeEid[20];
CTaskCpeM :: CTaskCpeM()
{
    strcpy(m_szName, "tCpeM");
    m_uPriority     = M_TP_L3UM;
    m_uOptions      = VX_FP_TASK;
    m_uStackSize    = SIZE_KBYTE * 50;
    m_iMsgQMax      = 100;
    m_iMsgQOption   = ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY );

    m_pCheckCPETimer= 0;
#ifdef M_TGT_WANIF
   m_pProbeWanCPETimer = 0;
#endif
    m_TxSocket      = 0;
    memset(&m_NeighborBTSLoadInfoEle, 0, sizeof(m_NeighborBTSLoadInfoEle));
}

bool CTaskCpeM :: Initialize()
{
#ifdef __WIN32_SIM__
    ::WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
#endif

    //create socket
    m_TxSocket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_IP );
    if ( M_DATA_INVALID_SOCKET == m_TxSocket )
        {
#ifdef __WIN32_SIM__
        ::WSACleanup();
#endif
        return false;
        }

    if (false == CBizTask :: Initialize())
        {
#ifdef __WIN32_SIM__
        ::closesocket( m_TxSocket );
        ::WSACleanup();
#else
        ::close( m_TxSocket );
#endif
        return false;
        }

    m_pCheckCPETimer = CPE_Createtimer(M_OAM_CPE_CHECK_TO_DELETE_TIMER, M_TIMER_PERIOD_YES, CHECK_CPE_TO_DELETE_PERIOD);
    if (NULL == m_pCheckCPETimer)
        {
#ifdef __WIN32_SIM__
        ::closesocket( m_TxSocket );
        ::WSACleanup();
#else
        ::close( m_TxSocket );
#endif
        return false;
        }
#ifdef WBBU_CODE
    m_ToBeDeleteCpeList.clear();
#endif
	    //启动定时器检查是否有需要删除CPE记录
    m_pCheckCPETimer->Start();
#ifdef M_TGT_WANIF
m_pProbeWanCPETimer = CPE_Createtimer(M_OAM_CPE_Probe_TO_WanIF_TIMER, M_TIMER_PERIOD_YES, Probe_WanCPE_TO_DELETE_PERIOD);
m_pProbeWanCPETimer->Start();
#endif
 //   m_SagStatus = 0;

#if 1//def PAYLOAD_BALABCE
	for( UINT8 uc=0; uc<NEIGHBOR_BTS_NUM; uc++ )
		m_ucNBtsRcdForPld[uc] = PAYLOAD_UNKNOW;
	m_ucPayloadSupportNotifyCnt = 0;
#endif
#ifdef LOCATION_2ND
    m_ulBTSID = bspGetBtsID();
#endif
    //启动周期性查询ccb表定时器，找到故障弱化时上来的终端，如果当前sag链路好，则让其重新注册，周期10s
    m_pCheckSAGDefaultTimer = CPE_Createtimer(M_OAM_CPE_CHECK_SAG_DEFAULT_TIMER, M_TIMER_PERIOD_YES, 10000);
#if 0
    if(m_pCheckSAGDefaultTimer!=NULL)
        m_pCheckSAGDefaultTimer->Start();
    m_check_SAG_Default_saved = m_ToBeDeleteCpeList.begin();
#endif
    return true;
}
extern "C" bool notifyOneCpeToRegister(UINT32 EID,bool blStopDataSrv);

#ifdef M_TGT_WANIF
extern UINT32  WanIfCpeEid ;
extern UINT32  BakWanIfCpeEid ;
extern UINT32  WorkingWcpeEid;
unsigned int   WCPE_Probe_No_Rsp_Times = 0;
extern unsigned char begin_2_probeWCPE ;
extern UINT16   Wanif_Switch;
//extern UINT32  RelayWanifCpeEid;
extern "C" void SetLocalEms(unsigned char  flag);
#include "L3DataArp.h"
#endif
//#ifdef LJF_RPT_ALTER_2_NVRAMLIST
#include "L3OamSystem.h"
bool CTaskCpeM::isRptInNvram( UINT32 ulpid )
{
	for( UINT8 uc=0; uc<M_RPT_LIST_MAX; uc++ )
	{
		if( NvRamDataAddr->ulRptList[uc] == ulpid )
			return true;
	}
	return false;
}
bool CTaskCpeM::rptIsNotInNvramList( UINT32 ulpid )
{
struct 
    {
		T_L3CpeProfUpdateReqBaseNoIE hdr; 
		T_UTProfile UTProfile;
		UINT16  QosEntryNum;
		T_CpeToSCfgEle *TosInfo;  
		UINT16  BtsNum;            // 0 -- 20 Neighbor BTS number
		UINT32 *BTSID;
		UINT16  RepeaterNum;
		UINT16 *repeaterFreq;
		UINT8   RFprofile[12];//RF profile;
		T_UT_HOLD_BW  UTHoldBW;//wangwenhua add 20080916;
		T_MEM_CFG memCfg;//mem info, if cpe, is 0 jy081217
    }req;
	memset( (UINT8*)&req, 0, sizeof(req) );
	req.hdr.ProfileDataFlag = 1;
	req.UTProfile.UTProfileIEBase.AdminStatus = 0xFE;
	RPT_SendComMsg( ulpid, M_TID_CPECM, M_L3_CPE_UPDATE_BWINFO, (UINT8*)&req, sizeof(req) );
	return true;
}
bool CTaskCpeM :: ProcessMessage(CMessage &rMsg)
{ 
    UINT16 MsgId = rMsg.GetMessageId();
    //OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] receive msg[0x%04x] from task[%d], EID[0x%x]", MsgId, rMsg.GetSrcTid(), rMsg.GetEID());
   UINT32  eid ;
	CPECCB    *ccb ;
	  FSMStateIndex originState;
    switch(MsgId)
    {
	#ifdef RCPE_SWITCH
		case M_CPE_L3_TRUNK_MAC_MOVEAWAY_GET:
			//RPT_SendComMsg( rMsg.GetEID(), rMsg.GetDstTid(), M_CPE_L3_TRUNK_MAC_MOVEAWAY_GET, (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength() );
			//RPT_SendComMsg( rMsg.GetEID(), M_TID_UTDM, M_CPE_L3_TRUNK_MAC_MOVEAWAY_GET, (UINT8*)&rMsg.GetEID(), 4 );
			break;
	#endif
//#ifdef LJF_RPT_ALTER_2_NVRAMLIST
		case M_EMS_BTS_RPT_LIST_REQ:
			CPE_RptListReq( rMsg );
			break;
#ifdef LOCATION_2ND
    	case M_EMS_BTS_LOCATION_REQ://ems定位请求
	        OAM_LOGSTR(LOG_DEBUG2, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] recive msg M_EMS_BTS_LOCATION_REQ");//wangwenhua modify 20110429 to decrease the print level
			CPE_Location_EmsReq(rMsg);
			break;
		case M_CPE_BTS_LOCATION_RSP:
	        OAM_LOGSTR(LOG_DEBUG2, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] recive msg M_CPE_BTS_LOCATION_RSP");
			CPE_Location_GpsRsp(rMsg);
			break;
        case M_L2_L3_LOCATION_DOA_RSP:
            CPE_Location_DoaRsp(rMsg);
            break;
        case MSGID_LOCATION_TIMER_OUT:
            CPE_Location_TmOut(rMsg);
            break;
#endif
	case M_CPE_BTS_LOADINFO_REQ:
		CPE_LoadinfoReq(rMsg);
		break;
    case M_EMS_BTS_NEIGHBOUR_PAYLOAD_WEIGHT_USER_REQ:
		UINT16 usRsp[2];
		usRsp[0] = *(UINT16*)rMsg.GetDataPtr();
		usRsp[1] = 0;
		//SetWeightedUserNoByBtsID();
		RPT_SendComMsg(0, M_TID_EMSAGENTTX, M_BTS_EMS_NEIGHBOUR_PAYLOAD_WEIGHT_USER_RSP, (UINT8*)usRsp, 4 );
        break;
        case M_CPE_L3_ACCESS_REQ:
            CPE_AccessReq(rMsg);
            break;

        case MSGID_AUTH_CMD:
            CPE_sabisAuthCMD(rMsg);
            break;

        case M_CPE_L3_AUTH_RSP:
            CPE_UTAuthRsp(rMsg);
            break;

        case MSGID_AUTHDEV_RESULT:
            CPE_sabisAuthResult(rMsg);
            break;
        case M_CPE_L3_AccountLogin_req:
	     CPE_AccountLogin_req(rMsg);
	     break;
        case M_CPE_L3_PROFILE_UPDATE_RSP:
            CPE_ProfileUpateRsp(rMsg);
            break;

        case M_OAM_CPE_PROF_UPDATE_FAIL:
            CPE_ProfileUpateFail(rMsg);
            break;
	case M_L3_CPE_BTS_BEAT_REQ://   = 0x6510;
	      eid= rMsg.GetEID();
		
	      if(m_CpeFsm.FindCCB(eid)==NULL)
	      	{
	      	   //注册
	      	   notifyOneCpeToRegister(eid,true);
	      	}
		 else
		 {
		      ccb = (CPECCB*)m_CpeFsm.FindCCB(eid);
		        originState =  ccb->GetCurrentState();
			if((originState==CPE_NORECORD)||(originState==CPE_MOVEDAWAY)||(originState==CPE_SWITCHOFF))
			{
			     //注册
			      notifyOneCpeToRegister(eid,true);
			     
			}
		 }
	      CPE_HeartBeatMsg(rMsg);
	        break;

        case M_EMS_BTS_PROBE_UT_REQ:
            CPE_ProbeCPE(rMsg, M_BTS_EMS_PROBE_UT_RSP);
            break;

        case M_CPE_L3_PROBE_UT_RSP:
            CPE_ProbeUTResponse(rMsg);
            break;
#if 0
        case M_OAM_CPE_PROBE_FAIL:
            OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!!Receive Probe UT fail response.");
            CPE_ProbeCpeFail(rMsg);
            break;
#endif
        case M_EMS_BTS_RESET_UT_REQ:
            CPE_ResetCPE(rMsg, M_BTS_EMS_RESET_UT_RSP);
            break;

        case M_CPE_L3_RESET_UT_RSP:
            OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!!Receive Reset UT response.");
            //CPE_NoParaRspHandler(rMsg);
            break;
		//liujianfeng 20080515 CPE clear HIST
		case M_L3_CPE_CLEAR_HIST_REQ:
            OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]Get Clear CPE HIST" );
			CPE_ClearHist(rMsg);
			break;
		case M_CPE_L3_CLEAR_HIST_RSP:
			if( 0 == *((UINT16*)rMsg.GetDataPtr()+2) ) 
            	OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]Clear CPE[0x%02x] HIST successed!", rMsg.GetEID(), *((UINT16*)rMsg.GetDataPtr()+2) );
			else 
            	OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]Clear CPE[0x%02x] HIST failed!", rMsg.GetEID(), *((UINT16*)rMsg.GetDataPtr()+2) );
			break;
    		
        case M_OAM_CPE_CHECK_TO_DELETE_TIMER:
            CPE_CheckToDelete(rMsg);
			break;
 #ifdef M_TGT_WANIF
	  case M_OAM_CPE_Probe_TO_WanIF_TIMER:/*20s**/
	   if(Wanif_Switch == 0x5a5a)
	   {
		  if( begin_2_probeWCPE == 1)
		  {
	            
		     if(((WCPE_Probe_No_Rsp_Times+1)%4)==0)//if contine 1 min no rsp
		     	{
		     	      OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Wanif CPE[0x%.8X] Work not noraml ", WorkingWcpeEid); 
			      if(WorkingWcpeEid== WanIfCpeEid)
			      	{
			      	  if(BakWanIfCpeEid!=0)
			      	  	{
			      	           WorkingWcpeEid = BakWanIfCpeEid;
			      	  	}
			      	}
				 else if(WorkingWcpeEid== BakWanIfCpeEid)
				 {
				     if(WanIfCpeEid!=0)
				     	{
				                WorkingWcpeEid = WanIfCpeEid;
				     	}
				 }
				   CTaskARP::GetInstance()->sendAllGARP();
				 
				 OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Now change to Wanif CPE[0x%.8X]  ", WorkingWcpeEid); 
		     	}
			if(WCPE_Probe_No_Rsp_Times>7)
			{
			    // SetLocalEms(1);//设置不正常
			}
			if(WorkingWcpeEid!=0)
			{
			ProbeWanifCpe(WorkingWcpeEid);
			}
		     WCPE_Probe_No_Rsp_Times++;
			 OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Probe Wanif CPE[0x%.8X]  ", WorkingWcpeEid);
		  }
	  }
 #endif
            break;

        case M_OAM_CPE_REG_TO_EMS_FAIL:
            OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%.8X] Register to SAG/HLR FAIL", rMsg.GetEID());
	     CPE_SagProfileReqFail(rMsg);
            break;

        case M_EMS_BTS_NEIGHBOUR_BTS_LOADINFO_CFG_REQ:
            CPE_BtsNeighborBtsLoadInfoSyncReq(rMsg);
            break;
        case M_L2_L3_BTS_LOADINFO_NOTIFY:
            CPE_L2BtsLoadInfoNotify(rMsg);
            break;
        case M_CPE_L3_GET_NEIGHBOR_LIST_REQ:
            CPE_CpeGetNeighborListReq(rMsg);
            break;
	case M_EMS_BTS_UT_STATUS_DATA_REQ:
	      CPE_BtsUtStatusDataReq(rMsg);
		break;
	case M_L2_L3_BTS_UT_STATUS_DATA_RSP:
		CPE_BtsUtStatusDataRsp(rMsg);
		break;
	case M_EMS_BTS_UT_LAYER3_DATA_REQ:
	      CPE_BtsLayer3DataReq(rMsg);
		break;
	case M_CPE_BTS_UT_LAYER3_DATA_RSP:
		CPE_BtsLayer3DataRsp(rMsg);
		break;
//zengjihan 20120503 for MEM
	case M_EMS_BTS_MEMINFO_REPORT_REQ:
	      CPE_EmsBtsUtMemInfoReportReq(rMsg);
		break;
//zengjihan 20120503 for MEM
	case M_UT_BTS_MEMINFO_REPORT_RSP:
		CPE_UtBtsEmsMemInfoReportRsp(rMsg);
		break;
        case M_EMS_BTS_HWDESC_REQ:
            CpeHWtypeRequest(rMsg);
            break;

        case M_CPE_BTS_HWDESC_RSP:
            CpeHWtypeResponse(rMsg);
	     break;
	case M_L3_CPE_BTS_ETHERNET_CFG_RSP://sunshanggu -- 080708
		CPE_Netif_Rsp(rMsg);
		break;
	//wangwenhua add 20081119
    case M_EMS_BTS_CFG_NETWORK_UT_REQ ://= 0x031d;//配置终端网口双工模式请求
              CPE_CfgNetWorkCPE(rMsg); 
            break;

    case M_EMS_BTS_CFG_NETWORK_UT_QUERY_REQ:// = 0x031f;//查询终端网口双工模式请求
         CPE_ReqNetWorkCPE(rMsg);
		   break;
		   
	case M_EMS_BTS_UT_IPLIST_REQ://查询iplist信息,发给dm处理
              CPE_GetIplistReq(rMsg);
		break;
       case M_EMS_BTS_MEM_SIGNAL_RPT_CFG:
	  	CPE_SendCommonMsg(M_L3_CPE_MEM_SIGNAL_RPT_CFG, rMsg);
		break;
       case M_BTS_CPE_MEM_SIGNAL_RPT_RSP:
	  	CPE_RcvCommonMsg(M_BTS_EMS_MEM_SIGNAL_RPT_RSP, rMsg);
		break;	  	
       case M_EMS_BTS_MEM_RUNTIME_REQ:
	  	CPE_SendCommonMsg(M_L3_CPE_MEM_RUNTIME_REQ, rMsg);
		break;
       case M_BTS_CPE_MEM_RUNTIME_RSP:
	  	CPE_RcvCommonMsg(M_BTS_EMS_MEM_RUNTIME_RSP, rMsg);
		break;	
	case M_L3_CPE_BTS_DEBUG_COMM_MSG:
             CPE_Debug_MSg(rMsg);
		break;
    case M_OAM_CPE_CHECK_SAG_DEFAULT_TIMER:
        CPE_Check_Sag_Default_Proc();
        break;    
        default:
//			#ifdef LJF_RPT_ALTER_2_NVRAMLIST
		    if( (MsgId == M_CPE_L3_REG_NOTIFY) || (MsgId == M_CPE_L3_REGISTER_REQ) )
	        {
				T_RegisterReq* pRegisterReq1 = (T_RegisterReq*)rMsg.GetDataPtr();
			    if( RPTHW_TYPE_3B == pRegisterReq1->UTBaseInfo.usHWtype || RPTHW_TYPE_3C == pRegisterReq1->UTBaseInfo.usHWtype )
		    	{
		    //		char ch[20];
//					if( true == (CTaskSystem::GetInstance())->SYS_IsEmsConnectOK() )
//	           	    	OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08X] SYS_IsEmsConnectOK()[TRUE]", pRegisterReq1->ulPID );
//					else
//	           	    	OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08X] SYS_IsEmsConnectOK()[FALSE]", pRegisterReq1->ulPID );
			        if( false == ((CTaskSystem::GetInstance())->SYS_IsEmsConnectOK()) )
		        	{
						if( !isRptInNvram( pRegisterReq1->ulPID ) )
						{
							rptIsNotInNvramList( pRegisterReq1->ulPID );
							break;
						}
			    	}
		    	}
			}

           if( m_CpeFsm.InjectMsg(rMsg)==false)//wangwenhua modify 20080717
           {
           	   OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%.8X] Msg [%x]cope FAIL", rMsg.GetEID(),MsgId);
           }
            break;
    }

    return true;
}

CTaskCpeM* CTaskCpeM :: GetInstance()
{
    if ( NULL == m_Instance )
    {
        m_Instance = new CTaskCpeM;
    }

    return m_Instance;
}
void CTaskCpeM::ProbeWanifCpe(UINT32 eid)
{
         CComMessage* pComMsg = new (this, 8) CComMessage;
	if (pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_SendCommonMsg.");
		return ;
	}
        unsigned char *p =(unsigned char*) pComMsg->GetDataPtr();
	
	pComMsg->SetDstTid(M_TID_CPECM);
	pComMsg->SetSrcTid(M_TID_UM);    
	pComMsg->SetEID(eid);
	pComMsg->SetMessageId(M_L3_CPE_PROBE_UT_REQ);    	
	unsigned short  transid = 0xffff;
	unsigned short version = 0xffff;
	memcpy((p),(UINT8*)&transid,2);
	memcpy((p+2),(UINT8*)&version,2);
	memcpy((p+4),(UINT8*)&eid,4);
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
}
void CTaskCpeM::ResetWanifCpe(UINT32 eid)
{
         CComMessage* pComMsg = new (this, 8) CComMessage;
	if (pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_SendCommonMsg.");
		return ;
	}
        unsigned char *p =(unsigned char*) pComMsg->GetDataPtr();
	
	pComMsg->SetDstTid(M_TID_CPECM);
	pComMsg->SetSrcTid(M_TID_UM);    
	pComMsg->SetEID(eid);
	pComMsg->SetMessageId(M_L3_CPE_RESET_UT_REQ);    	
	unsigned short  transid = 0xffff;
	unsigned short version = 0xffff;
	memcpy((p),(UINT8*)&transid,2);
	memcpy((p+2),(UINT8*)&version,2);
	memcpy((p+4),(UINT8*)&eid,4);
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
}
TID CTaskCpeM:: GetEntityId() const
{
   return M_TID_UM;
}

#if 0
bool CTaskCpeM :: CPE_L2ProfileUpateNotify(CMessage &rmsg)
{
    UINT32 cpeid = *(UINT32*)(((char*)(rmsg.GetDataPtr()))  + 2 );

    CL3L2CpeProfUpdateNotify Notify;
    if (false == Notify.CreateMessage(*this))
        return false;

    Notify.SetDstTid(M_TID_L2MAIN);
    Notify.SetSrcTid(M_TID_UM);
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetCpeId(cpeid);
    Notify.SetUTSDCfgInfo((SINT8 *)&(NvRamDataAddr->UTSDCfgEle.Info), sizeof(NvRamDataAddr->UTSDCfgEle.Info));
    Notify.setZmoduleEnabled(bool flag);

    if(true != Notify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post CPE profile update Notify to L2 fail.");
        Notify.DeleteMessage();
        return false;
    }

    return true;
}
#endif

bool CTaskCpeM::CpeHWtypeRequest(CMessage &rMsg)
{
#pragma pack(1)
typedef struct
    {
        UINT16  transId;
        UINT32  eid;
        UINT16  HWtype;
    } EMSHWtypeReq;
#pragma pack()
    EMSHWtypeReq *pEMSreq = (EMSHWtypeReq*)(rMsg.GetDataPtr());
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "Request CPE[0x%.8X] Hardware type descriptor", pEMSreq->eid);
    CCpeHWDescriptorReq msgReq;
    if (false == msgReq.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, new Message fail.");
        return false;
        }

    msgReq.SetEID(pEMSreq->eid);
    msgReq.setCPEID(pEMSreq->eid);
////msgReq.SetTransactionId(pEMSreq->transId);
    msgReq.setHWtype(pEMSreq->HWtype);
    msgReq.SetDstTid(M_TID_CPECM);

    CCpeHWDescriptorRsp FailMsg;
    if (false == FailMsg.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, new Message fail.");
        msgReq.DeleteMessage();
        return false;
        }

    FailMsg.SetDstTid(M_TID_EMSAGENTTX);
    FailMsg.SetResult(OAM_TIMEOUT_ERR);
////FailMsg.setRspEID(pEMSreq->eid);

    CTransaction *pTrans = CreateTransact(msgReq, FailMsg, 2, 1000);
    if (NULL == pTrans)
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, create transaction fail.");
        msgReq.DeleteMessage();
        FailMsg.DeleteMessage();
        return false;
    }

    msgReq.SetTransactionId(pTrans->GetId());
    FailMsg.SetTransactionId(pEMSreq->transId);
    if (false == pTrans->BeginTransact())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, Begin transaction fail.");
        pTrans->EndTransact();//如果失败释放处理jiaying20100811
        delete pTrans;
	    return false;
    }

    return  true;
}


bool CTaskCpeM::CpeHWtypeResponse(CMessage &rMsg)
{
    CCpeHWDescriptorRsp RspMsg(rMsg);

    CTransaction *pTransaction = FindTransact(RspMsg.GetTransactionId());
    if (NULL != pTransaction)
        {
        pTransaction->EndTransact();
        //向ems返回应答消息
#if 0
        CCpeHWDescriptorRsp msgCpeHWDescriptorRsp;
        if (false == msgCpeHWDescriptorRsp.CreateMessage(*this))
            {
            OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, new Message fail.");
            return false;
            }
#endif
        rMsg.SetDstTid(M_TID_EMSAGENTTX);
        rMsg.SetTransactionId(pTransaction->GetRequestTransId());
        rMsg.SetMessageId(M_BTS_EMS_HWDESC_RESP);
//rMsg.SetResult(RspMsg.GetResult());
//rMsg.setRspEID(RspMsg.GetEID());
        delete pTransaction;
        if (false == rMsg.Post())
            {
            OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post msg[0x%04x]  to task[%d] fail", rMsg.GetMessageId(), rMsg.GetSrcTid());
//rMsg.DeleteMessage();
            return false;
            }
        }
    else
        {
        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCpeM] get CPE HW type descriptor response can not find transid!, transid[0x%04x]", RspMsg.GetTransactionId());
        return false;
        }

    return true;
}


bool CTaskCpeM::CPE_ProbeCPE(CMessage &rMsg, UINT16 FailRspMsgID)
{
    CCpeCommonReq ReqMsg;
    if (false == ReqMsg.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, new Message fail.");
        return false;
        }

    ReqMsg.SetMessageId(M_L3_CPE_PROBE_UT_REQ);
    UINT32 EID = *(UINT32*)((SINT8*)rMsg.GetDataPtr() + 2);
    ReqMsg.SetEID(EID);
    ReqMsg.SetCPEID(EID);
    ReqMsg.SetDstTid(M_TID_CPECM);
    UINT16 ReqTranid = *(UINT16*)(rMsg.GetDataPtr());
    ReqMsg.SetTransactionId(ReqTranid);

    CL3OamProbeCPERsp FailMsg;
    if (false == FailMsg.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, new Message fail.");
        ReqMsg.DeleteMessage();
        return false;
        }

    FailMsg.SetDstTid(M_TID_EMSAGENTTX);
    FailMsg.SetMessageId(FailRspMsgID);
    FailMsg.SetResult(OAM_TIMEOUT_ERR);
    FailMsg.setRspEID(EID);

    CTransaction *pTrans = CreateTransact(ReqMsg, FailMsg, 2, 5000);
    if (NULL == pTrans)
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, create transaction fail.");
        ReqMsg.DeleteMessage();
        FailMsg.DeleteMessage();
        return false;
    }

    ReqMsg.SetTransactionId(pTrans->GetId());
    FailMsg.SetTransactionId(ReqTranid);
    if (false == pTrans->BeginTransact())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, Begin transaction fail.");
        pTrans->EndTransact();//如果失败释放处理jiaying20100811
        delete pTrans;
	    return false;
    }

    return  true;
}
bool  CTaskCpeM::CPE_HeartBeatMsg(CMessage &rMsg)
{

   UINT32 eid = rMsg.GetEID();
   UINT32 btsid = *(UINT32*)((SINT8*)rMsg.GetDataPtr());
   UINT16 result;
   UINT32 local_btsid = bspGetBtsID();
   if(btsid!=local_btsid)
   {
   	   result = 0x01;
	   OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive BTSID[0x%08x]LOCALBTSID[0x%08x]", btsid,local_btsid);
   }
   else
   {
   	result = 0x00;
   }
   CComMessage* pComMsg = new (this, 2) CComMessage;
	if (pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_SendCommonMsg.");
		return false;
	}

	
	pComMsg->SetDstTid(M_TID_CPECM);
	pComMsg->SetSrcTid(M_TID_UM);    
	pComMsg->SetEID(eid);
	pComMsg->SetMessageId(M_L3_BTS_CPE_BEAT_RSP);    	
	memcpy(((UINT8*)pComMsg->GetDataPtr()), (UINT8*)&result,2);
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
	return true;
   
}
//wangwenhua add 20081119
bool   CTaskCpeM::CPE_CfgNetWorkCPE(CMessage &rMsg)
{
    UINT32 eid = *(UINT32*)((SINT8*)rMsg.GetDataPtr() + 2);
    UINT16 type = *(UINT16*)((SINT8*)rMsg.GetDataPtr() + 6);
    UINT16 transid = *(UINT16*)(rMsg.GetDataPtr());
     UINT16 content = 0;
	CPECCB *CCB;
     CCB = (CPECCB *)m_CpeFsm.FindCCB(eid);
    if(CCB==NULL)
    	{
    	  return false;
    	}
	CCB->setCfgTranid(transid);
	OAM_LOGSTR3(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]CPE_CfgNetWorkCPE:eid:%x,type:%x,tranid:%x\n",eid,type,transid);
    if(type == 0)//自适应只发送一条消息
    {
	content = 0x1000;
    	CPE_NetIf_Cfg(eid, CFG_CPE_NETIF_ADA, (UINT8 *)(&content));  	
    }
    else//非自适应需要发送2条消息
   {
       content = 0x1001;
    	CPE_NetIf_Cfg(eid, CFG_CPE_NETIF_ADA, (UINT8 *)(&content));

	UINT16 content1[2] = {0};

	if(type == 2)//10M ,1/2
	{
	  content1[0] = 0x1000;
	  content1[1] = 0x1000;
	}
	else if(type ==1)//10M, 1
	{
	  content1[0] = 0x1000;
	  content1[1] = 0x1001;
	}
	else if(type == 4)//100M,1/2
	{
	  content1[0] = 0x1001;
	  content1[1] = 0x1000;
	}
	else if(type == 3)//100M,1
	{
	  content1[0] = 0x1001;
	  content1[1] = 0x1001;
	}
	else 
		return false;
      CPE_NetIf_Cfg(eid, CFG_CPE_NETIF_SPEED_DUPLEX, (UINT8 *)content1);
	return true;
   }

     
    return  true;
}
bool  CTaskCpeM::CPE_ReqNetWorkCPE(CMessage &rMsg)
{
    UINT32 eid = *(UINT32*)((SINT8*)rMsg.GetDataPtr() + 2);
 
    UINT16 transid = *(UINT16*)(rMsg.GetDataPtr());
//     UINT16 content = 0;
	CPECCB *CCB;
     CCB = (CPECCB *)m_CpeFsm.FindCCB(eid);
    if(CCB==NULL)
    	{
    	  return false;
    	}
	CCB->setQueryTransid(transid);
      CPE_NetIf_Cfg(eid, READ_CPE_NETIF_SPEED_DUPLEX, NULL);   
    return  true;
}
//wangwenhua add end 20081119
bool CTaskCpeM::CPE_ProbeUTResponse(CMessage &rMsg)
{
    CL3OamCommonRspFromCpe RspMsg(rMsg);
    #ifdef M_TGT_WANIF
if(Wanif_Switch== 0x5a5a)
{
	   if( WorkingWcpeEid == RspMsg.GetEID())
	   {
	          WCPE_Probe_No_Rsp_Times  = 0;
		//只有在打开开关的情况下面才能进行设置
		 {
		        SetLocalEms(0);
		 }
		   if(RspMsg.GetTransactionId()==0xffff)
		   	return true;
	   }
}
   #endif

    CTransaction *pTransaction = FindTransact(RspMsg.GetTransactionId());
    if (pTransaction)
    {
        pTransaction->EndTransact();
        //向ems返回应答消息
        //UM_PostCommonRsp(M_TID_EMSAGENTTX, pTransaction->GetRequestTransId(), M_BTS_EMS_PROBE_UT_RSP, RspMsg.GetResult());
        CL3OamProbeCPERsp msgProbeCPERsp;
        if (false == msgProbeCPERsp.CreateMessage(*this))
            {
            OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, new Message fail.");
            return false;
            }
        msgProbeCPERsp.SetDstTid(M_TID_EMSAGENTTX);
        msgProbeCPERsp.SetTransactionId(pTransaction->GetRequestTransId());
        msgProbeCPERsp.SetMessageId(M_BTS_EMS_PROBE_UT_RSP);
        msgProbeCPERsp.SetResult(RspMsg.GetResult());
        msgProbeCPERsp.setRspEID(RspMsg.GetEID());
        delete pTransaction;
        if (true != msgProbeCPERsp.Post())
        {
            OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post msg[0x%04x]  to task[%d] fail", msgProbeCPERsp.GetMessageId(), msgProbeCPERsp.GetSrcTid());
            msgProbeCPERsp.DeleteMessage();
            return false;
        }
    }
    else
    {
        OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tCpeM] Probe CPE response can not find transid!, transid[0x%04x]", RspMsg.GetTransactionId());
        return false;
    }
    
    return true;
}

bool CTaskCpeM::CPE_ResetCPE(CMessage &rMsg, UINT16 FailRspMsgID)
{
    UINT16 ReqTranid = *(UINT16*)(rMsg.GetDataPtr());
    CL3OamCommonRsp L3OamCommonReq(rMsg); 
    UM_PostCommonRsp(M_TID_EMSAGENTTX, ReqTranid, M_BTS_EMS_RESET_UT_RSP, OAM_SUCCESS);

    CCpeCommonReq ReqMsg;
    if (false == ReqMsg.CreateMessage(*this))
        return false;

    ReqMsg.SetMessageId(M_L3_CPE_RESET_UT_REQ);
    UINT32 eid = *(UINT32*)((SINT8*)rMsg.GetDataPtr() + 2);
    ReqMsg.SetEID(eid);
    ReqMsg.SetCPEID(eid);
    ReqMsg.SetDstTid(M_TID_CPECM);
    ReqMsg.SetTransactionId(ReqTranid);
#if 1
    if (false == ReqMsg.Post())
        {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post Reset CPE[0x%x] message fail", eid);
        ReqMsg.DeleteMessage();
        return false;
        }
    return  true;
#else
    CL3OamCommonRsp FailMsg;
    if (false == FailMsg.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, new Message fail.");
        ReqMsg.DeleteMessage();
        return false;
        }

    FailMsg.SetDstTid(M_TID_EMSAGENTTX);
    FailMsg.SetMessageId(FailRspMsgID);
    FailMsg.SetResult(OAM_TIMEOUT_ERR);

    CTransaction *pTrans = CreateTransact(ReqMsg, FailMsg, OAM_REQ_RESEND_CNT3, 1000);
    if (NULL == pTrans)
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, create transaction fail.");
        FailMsg.DeleteMessage();
        ReqMsg.DeleteMessage();
        return false;
        }

    ReqMsg.SetTransactionId(pTrans->GetId());
    FailMsg.SetTransactionId(ReqTranid);
    pTrans->BeginTransact();
    return  true;
#endif
}


bool CTaskCpeM :: CPE_ProfileUpateRsp(CMessage &rMsg)
{
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%08x] Profile Update Response", rMsg.GetEID());
    CL3OamCommonRspFromCpe RspMsg(rMsg);
    UINT16 TransId = RspMsg.GetTransactionId();
    CTransaction * pTransaction = FindTransact(TransId);
    if(!pTransaction)
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] can not find transid[0x%x] in msg[M_CPE_L3_PROFILE_UPDATE_RSP]", TransId);
        return false;
    }
    else
    {
        pTransaction->EndTransact();
        delete pTransaction;

        CPECCB* pCPECCB = (CPECCB*)m_CpeFsm.FindCCB(RspMsg);
        if (NULL != pCPECCB)
            {
            pCPECCB->setUpdUTProfileTransId(0X0000);
            //T_CpeRegNotifyToEms& RegInfo = pCPECCB->GetCpeRegInfo();
            UINT32 ulEid  = pCPECCB->getEid();
            if (0 != RspMsg.GetResult())
                {
                OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile update response FAIL, maybe profile data error.", ulEid);
                //CPE response FAIL.
                return true;
                }
            else
                {
                UINT8  status = pCPECCB->getAdminStatus();
                if (CPE_ADM_STATUS_INV == status)
                    {
                    OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register FAIL.", ulEid);
                    }
                if (CPE_ADM_STATUS_SUS == status)
                    {
                    OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] SUSPEND.", ulEid);
                    }
                if (CPE_ADM_STATUS_ADM == status)
                    {
                    OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register SUCCESS.", ulEid);
                    }
                }
            }
        
        return true;
    }
}
#if 0
bool CTaskCpeM :: CPE_ProfileUpateFail(CMessage& msg)
{
    CL3OamCommonRsp failMsg(msg);
    UINT16 TransId = failMsg.GetTransactionId();

    CPECCB* pCPECCB = (CPECCB*)m_CpeFsm.FindCCB(failMsg);
    if (NULL != pCPECCB)
    {
        pCPECCB->setUpdUTProfileTransId(0X0000);
    }

    UINT32 Eid = msg.GetEID();
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Update CPE[0x%04x] profile fail,transID = 0x%x.", Eid,TransId);
    return true;
}
#endif
bool CTaskCpeM :: CPE_ProfileUpateFail(CMessage& msg)
{
    CL3OamCommonRsp failMsg(msg);
    UINT16 TransId = failMsg.GetTransactionId();

    CPECCB* pCPECCB = (CPECCB*)m_CpeFsm.FindCCB(failMsg);
    if (NULL != pCPECCB)
    {
        pCPECCB->setUpdUTProfileTransId(0X0000);
    }

    UINT32 Eid = msg.GetEID();
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Update CPE[0x%04x] profile fail,transID = 0x%x.", Eid,TransId);
  CTransaction * pTransaction = FindTransact(TransId);
    if(!pTransaction)
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] can not find transid[0x%x] in msg[CPE_ProfileUpateFail]", TransId);
        return false;
    }
    else
    {
        pTransaction->EndTransact();
        delete pTransaction;
     }

    return true;
}
#if 1
void CTaskCpeM :: CPE_DeleteCpeData(UINT32 eid)
{

    m_CpeFsm.DeleteCCB(eid);
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Delete CPE[0x%08x]'s info", eid);
}
#endif

bool CTaskCpeM::CPE_CheckToDelete(CMessage &msg)
{
    list<T_ToBeDeleteCpe>::iterator it = m_ToBeDeleteCpeList.begin();
    list<T_ToBeDeleteCpe>::iterator it_bak;
    while(it != m_ToBeDeleteCpeList.end())
        {
        UINT32 eid = it->CPEID;
	 //欠费状态校验
	 {

            CPECCB *pCCB1 = (CPECCB*)m_CpeFsm.FindCCB(eid);
            if (NULL != pCCB1)
            {
			      T_UTProfile profile ;
			  
			      profile = pCCB1->getUTProfile();
			      if(profile.UTProfileIEBase.AdminStatus==0x10)
			      {
					CComMessage      *pComMsg     = new ( CTaskCpeM::GetInstance(), 1 )CComMessage;
				        if ( NULL != pComMsg )
				        {
				    
				    

					    pComMsg->SetDstTid( M_TID_EB );
					    pComMsg->SetSrcTid( M_TID_UM );
					    pComMsg->SetEID( eid );
					    pComMsg->SetMessageId( M_EMS_BTS_UT_PROFILE_UPDATE_REQ ); 
					    UINT8*pData = (UINT8*)pComMsg->GetDataPtr();
					    pData[0] = 1;

					    CComEntity::PostEntityMessage( pComMsg );	
				        }
			      }

            }

	 }
        it->RemanentTime -= 1;
	 if(it->RemanentTime == 5)//提前5分钟发送
	 {	   
	     //注册
	     notifyOneCpeToRegister(eid,true);
	 }
        if(it->RemanentTime == 0)
        {
            
            //将其从列表m_ToBeDeleteCpeList中删除
            OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] time out, delete from TOBE delete list.", eid);
            
            //释放相关资源
            //msg.SetEID(eid);
            CPECCB *pCCB = (CPECCB*)m_CpeFsm.FindCCB(eid);
            if (NULL != pCCB)
            {
                OAM_LOGSTR1(LOG_CRITICAL, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Notify tDM/tVoice to release CPE[0x%08x] related resource.", eid);
                CPETrans::CPE_StopTransaction(*pCCB);
                CPETrans::SendDisableToDM_Voice(*pCCB, msg.GetMessageId(), eid);
                delete pCCB;
            }

            //将对应的CCB从ccb table删除
            m_CpeFsm.DeleteCCB(eid);
            it_bak = it;
            ++it;
            m_ToBeDeleteCpeList.erase(it_bak);
        }
        else
            ++it;
    }

    return true;
}   

/*
 *如果是第一次注册把CPE加入到m_ToBeDeleteCpeList,
 *或者如果已经在系统中存在，则修改存在时间
 */
#ifndef WBBU_CODE

bool CTaskCpeM :: CPE_AddDeleteCpeInfo(UINT32 CpeID, UINT32 period)
{
    bool bfound = false;
    list<T_ToBeDeleteCpe>::iterator it;
    for(it = m_ToBeDeleteCpeList.begin(); it != m_ToBeDeleteCpeList.end(); ++it )
        {
        if (it->CPEID == CpeID)
            {
            bfound = true;
            break;
            }
        }
    if (false == bfound)
        {
        //链表中没有找到CPE,创建一个新的插入到表里
        T_ToBeDeleteCpe ToBeDeleteCpe;
        ToBeDeleteCpe.CPEID        = CpeID;
        ToBeDeleteCpe.RemanentTime = period;
        m_ToBeDeleteCpeList.push_back(ToBeDeleteCpe);
        }
    else
        {
        if(0 != period)
            {
            //找到，只修改时间
            it->RemanentTime = period;
            }
        else
            {
            //cpe节点已经在状态机删除
            //从链表删除该节点
            m_ToBeDeleteCpeList.erase(it);
            }
        }
    return true;
}
#else
bool CTaskCpeM :: CPE_AddDeleteCpeInfo_function(UINT32 CpeID, UINT32 period)
{
    bool bfound = false;
    list<T_ToBeDeleteCpe>::iterator it;
    for(it = m_ToBeDeleteCpeList.begin(); it != m_ToBeDeleteCpeList.end(); ++it )
        {
        if (it->CPEID == CpeID)
            {
            bfound = true;
            break;
            }
        }
    if (false == bfound)
        {
        //链表中没有找到CPE,创建一个新的插入到表里
        T_ToBeDeleteCpe ToBeDeleteCpe;
        ToBeDeleteCpe.CPEID        = CpeID;
        ToBeDeleteCpe.RemanentTime = period;
        m_ToBeDeleteCpeList.push_back(ToBeDeleteCpe);
        }
    else
        {
        if(0 != period)
            {
            //找到，只修改时间
            it->RemanentTime = period;
            }
        else
            {
            //cpe节点已经在状态机删除
            //从链表删除该节点
            m_ToBeDeleteCpeList.erase(it);
            }
        }
    return true;
}
#endif

bool GetCPEConfig(UINT32 uleid, UINT8* pDataBuff) 
{   
#pragma pack(1)
    struct T_CfgNotify
    {
        UINT16 TransId;
        UINT8  Mobility;   //0 -- disabled     1 -- enabled
        UINT8  DHCPRenew;  //Allow DHCP Renew in serving BTS 0-- disabled  1-- enabled
        UINT16 VLanID;
        UINT8  MaxIpNum;   //Max IP ddress number   1~20    
        UINT8  FixIpNum;   //Fixed Ip number        0~20
        T_CpeFixIpInfoForData CpeFixIpInfo[MAX_FIX_IP_NUM];
    };
#pragma pack()
    if (NULL == pDataBuff)
        return false;

    CCpeCommonReq ReqMsg;
    CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();

    if (false == ReqMsg.CreateMessage(*pTInstance))
        return false;

    ReqMsg.SetEID(uleid);
    ReqMsg.SetCPEID(uleid);
    CPECCB *pCPECCB = (CPECCB*)pTInstance->m_CpeFsm.FindCCB(ReqMsg);
    ReqMsg.DeleteMessage();

    if(NULL == pCPECCB)
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Can't find CPE[0x%x] configurations", uleid);
        return false;  
    }
     UINT8 admin = pCPECCB->getAdminStatus();
    if ((CPE_ADM_STATUS_ADM != admin)&&(admin!=CPE_ADM_STATUS_FLOW_LIMITED)&&(admin!=CPE_ADM_STATUS_NOPAY))
        {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%x] profile invalid, get CPE configure data FAIL.", uleid);
        return false;
        }

    T_CfgNotify CPEDataConfigReq;
    memset(&CPEDataConfigReq, 0, sizeof(T_CfgNotify));
    CPEDataConfigReq.TransId   = 0XFFFF;
    CPEDataConfigReq.Mobility  = pCPECCB->getMobility();
    CPEDataConfigReq.DHCPRenew = pCPECCB->getDHCPrenew();
    CPEDataConfigReq.VLanID    = pCPECCB->getVlanID();
    CPEDataConfigReq.MaxIpNum  = pCPECCB->getMaxIPnum();
    CPEDataConfigReq.FixIpNum  = pCPECCB->getFixIpnum();

    UINT8 FixIpNum = CPEDataConfigReq.FixIpNum;
    if(FixIpNum > MAX_FIX_IP_NUM) 
    {
        OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "\r\nFixed IP number ERROR[%d], and is corrected to %d", FixIpNum, MAX_FIX_IP_NUM);
        CPEDataConfigReq.FixIpNum = FixIpNum = MAX_FIX_IP_NUM;
    }

    const T_CpeFixIpInfo *pFixIPinfo = pCPECCB->getFixIPinfo();
    for(UINT8 i = 0; i < FixIpNum; i++)
    {
        CPEDataConfigReq.CpeFixIpInfo[i].CPEID = uleid;
        memcpy(CPEDataConfigReq.CpeFixIpInfo[i].MAC, &pFixIPinfo[i], sizeof(T_CpeFixIpInfo));
    }

    memcpy(pDataBuff, &CPEDataConfigReq, sizeof(CPEDataConfigReq));

    return true;
}

void l3oamprintcpedata(UINT32 uleid) 
{   
    bool Find = false;

    CCpeCommonReq ReqMsg;
    CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();

    if (false == ReqMsg.CreateMessage(*pTInstance))
        return;
    ReqMsg.SetMessageId(0xFFFF);
    ReqMsg.SetEID(uleid);
    ReqMsg.SetCPEID(uleid);
    ReqMsg.SetDstTid(M_TID_CPECM);

    CPECCB* pCCB = (CPECCB*)pTInstance->m_CpeFsm.FindCCB(ReqMsg);
    ReqMsg.DeleteMessage();
    if(pCCB)
    {
        Find = true;
    }
    
    if(!Find)
    {
         printf("\r\nCPE[0x%08X] not found", uleid);
         return ;
    }
    printf("\r\nEID  : 0x%X", pCCB->getEid());
    printf("\r\nUID  : 0x%X", pCCB->getUid());
    printf("\r\ntype : %s", (pCCB->getCCBType() == M_CCB_TYPE_OLD)?"old":"new");
    printf("\r\nstate: %s", (char*)(strCpeSTATE[pCCB->GetCurrentState()]));

    const T_UTBaseInfo &UTBaseInfo = pCCB->getCpeBaseInfo();

    printf("\r\nBase Info:::::");   
    printf("\r\nHardwareType = 0x%X"
           "\r\nSoftwareType = 0x%X"
           "\r\nActiveSWVer  = 0x%X"
           "\r\nStandbySWVer = 0x%X"
           "\r\nHardwareVer  = %s",
           //UTBaseInfo.ulCID,
           UTBaseInfo.usHWtype,
           UTBaseInfo.ucSWtype,
           UTBaseInfo.ulActiveSWversion,
           UTBaseInfo.ulStandbySWversion,  
           UTBaseInfo.ucHWversion); 
    T_UT_HOLD_BW *bw = (pCCB->getUTHoldBW());
    T_MEM_CFG memcfg = (pCCB->getMemCfg());
    printf("\r\nProfile::::::"); 
    printf("\r\nAdmin Status            = %d (0-Active,1-Sus,0xFF-Invalid)"
           "\r\nPerf Log Status         = %d (0-disable,1-enable)"
           "\r\nPerf Collect Interval   = %d"
           "\r\nMobility    = %d (0-disable,1-enable)"
           "\r\nDHCP Renew  = %d (0-disable,1-enable)"
           "\r\nPrd Reg Time= %d (hours)"
           "\r\nVlanID      = %d"
           "\r\nMAX IP num  = %d"
           "\r\nPort Mask   = 0x%08X"
           "\r\nClass       = %d"
           "\r\nUT flag       = %d"
           "\r\nULMaxBW     = %d"
           "\r\nULMinBW     = %d"
           "\r\nDLMaxBW     = %d"
           "\r\nDLMinBW     = %d"
           "\r\nFixedIP num = %d"
            "\r\nULHoldBW     = %04X"
           "\r\nDLHoldBW     = %04X",
           pCCB->getAdminStatus(),
           pCCB->getPerfLogStatus(),
           pCCB->getPerfDataCollectInterval(),
           pCCB->getMobility(),
           pCCB->getDHCPrenew(),
           pCCB->getPrdRegTimeValue(),
           pCCB->getVlanID(),
           pCCB->getMaxIPnum(),
           pCCB->getVoicePortMask(),
           pCCB->getUTSDCfg().Class,
           pCCB->getUTSDCfg().Reserved,
           pCCB->getUTSDCfg().ULMaxBW,
           pCCB->getUTSDCfg().ULMinBW,
           pCCB->getUTSDCfg().DLMaxBW,
           pCCB->getUTSDCfg().DLMinBW,
           pCCB->getFixIpnum(),
           bw->UL_Hold_BW,
           bw->DL_Hold_BW           
          );
    printf("\r\nMemIpType = %d"
           "\r\nDNSServer = %08x"
           "\r\nMemIp = %08x"
           "\r\nSubMask = %08x"
           "\r\nGateWay = %08x",
           memcfg.MemIpType,
           memcfg.DNSServer,
           memcfg.MemIp,
           memcfg.SubMask,
           memcfg.GateWay
           );
    printf("\r\nFixed IP:::::");
    if(0 == pCCB->getFixIpnum())
    {
        printf("\r\n--NULL--");
    }
    else
    {   
        const T_CpeFixIpInfo* pFixIPInfo = pCCB->getFixIPinfo();
        for(int i = 0; i< pCCB->getFixIpnum(); i++)
        {
             printf("\r\n    MAC = %.2X-%.2X-%.2X-%.2X-%.2X-%.2X  FixIP = 0x%.8X  ABtsId = %d  RAID   = %d",
                    pFixIPInfo[i].MAC[0],
                    pFixIPInfo[i].MAC[1],
                    pFixIPInfo[i].MAC[2],
                    pFixIPInfo[i].MAC[3],
                    pFixIPInfo[i].MAC[4],
                    pFixIPInfo[i].MAC[5],
                    pFixIPInfo[i].FixIP,
                    pFixIPInfo[i].AnchorBTSID,
                    pFixIPInfo[i].RouterAreaID
                    );
        }
    }
    printf("\r\nL2 flag:::::");
    T_L2SpecialFlag flag;
    pCCB->getSpecialFlag(flag);
    printf("\r\nL2 special flag1 = 0x%04X"
           "\r\nL2 special flag2 = 0x%04X"
           "\r\nL2 special flag3 = 0x%04X"
           "\r\nL2 special flag4 = 0x%04X",
           "\r\nL2 RF profile(L) = 0x%04X"
           "\r\nL2 RF profile(H) = 0x%04X"
           "\r\nrsv              = 0x%04X",
           flag.L2_Special_Flag1,
           flag.L2_Special_Flag2,
           flag.L2_Special_Flag3,
           flag.L2_Special_Flag4,
           *((UINT32*)flag.RFprofile),
           *((UINT32*)(flag.RFprofile+4)),
           *((UINT32*)(flag.RFprofile+8))
           ); 
    printf("\r\nWcpe/Rcpe flag: %d\n", pCCB->getWcpeORRcpeFlag());
    printf("\r\n cpe download flag: %d\n", pCCB->getUtDlFlag());
    printf("\r\n SAG default flag: %d\n", pCCB->getSagDefaultFlag());
    printf("\r\n---END---");
    return ;
}

void  CTaskCpeM :: UM_PostCommonRsp(TID tid, UINT16 transid, UINT16 msgid, UINT16 result)
{
    CL3OamCommonRsp CommonRsp;
    if (false == CommonRsp.CreateMessage(*this))
        return;
    CommonRsp.SetDstTid(tid);
    CommonRsp.SetTransactionId(transid);
    CommonRsp.SetMessageId(msgid);
    CommonRsp.SetResult(result);
    if(true != CommonRsp.Post())
    {
        OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post msg[0x%04x]  to task[%d] fail", CommonRsp.GetMessageId(), CommonRsp.GetSrcTid());
        CommonRsp.DeleteMessage();
    }
}

#if 0
void CTaskCpeM :: CPE_SendCpeRegNotifySimMsg()
{
    CL3L2CpeRegistNotify L3L2CpeRegNotify;  
    L3L2CpeRegNotify.CreateMessage(*this, 200);
    L3L2CpeRegNotify.SetEID(0xabcdef);
    L3L2CpeRegNotify.SetDstTid(M_TID_UM);
    L3L2CpeRegNotify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    char data[1000];
    memset(data, 0, sizeof(data));
    L3L2CpeRegNotify.SetUTProfIE(data, sizeof(data));   
    L3L2CpeRegNotify.Post();

    
    OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send CPE register sim msg [0x%04x]", M_CPE_L3_REG_NOTIFY);
}
#endif

extern UINT16 getServingEIDnum();
void CTaskCpeM :: CPE_L2BtsLoadInfoNotify(CMessage& rmsg)
{
    UINT8 *psrcdata = (UINT8*)rmsg.GetDataPtr() + 2;
    if (false == bDebugLoadInfoMode)
        {
        //发送2层上报的Load Info.
        gL3OamBTSLoadInfo.CurrentUserNumber = getServingEIDnum();
        memcpy(&(gL3OamBTSLoadInfo.AverageFreeDLchannel), psrcdata, sizeof(T_BTSLoadInfo) - 2);
        }
    else
        {
        //发送供调试用的Load Info.
        memcpy(&gL3OamBTSLoadInfo, &gTestBTSLoadInfo, sizeof(gTestBTSLoadInfo));
        }

    //开始往neighbor BTS同步
    CPE_SyncBTSLoadInfo();
	#if 1//def PAYLOAD_BALABCE
	if( 10 == ++m_ucPayloadSupportNotifyCnt )
	{
		m_ucPayloadSupportNotifyCnt = 0;
		CPE_SyncBTSLoadInfo( true );
	}
	#endif
    //广播Load Info给CPE.
    //CPE_BroadcastBtsLoadInfoToCpe(rmsg);
}


void CTaskCpeM :: CPE_SyncBTSLoadInfo( bool bNotify )
{
    //synchronize loadinfo to all neighbor BTS.
    T_NeighbotBTSLoadInfo stSyncLoadInfo;
    stSyncLoadInfo.BTSID = bspGetBtsID();
    memcpy(&(stSyncLoadInfo.BTSLoadInfo), &gL3OamBTSLoadInfo, sizeof(gL3OamBTSLoadInfo));
    UINT32 ulLoadInfoLen = sizeof(T_NeighbotBTSLoadInfo);

    UINT16 NeigBtsNum = NvRamDataAddr->BtsNeighborCfgEle.NeighborBtsNum;
    if(NeigBtsNum > NEIGHBOR_BTS_NUM)
        {
        return;
        }

//    UINT16 RepeaterNum = 0, repeaterLen = 0;
    char *pdata = (char*)(NvRamDataAddr->BtsNeighborCfgEle.BtsNeighborCfgData);  //从第一个BtsNeighborCfgData开始计算
    for(int i = 0; i < NeigBtsNum; ++i)
        {
        T_BtsNeighborCfgData *pNeighborDataElement = (T_BtsNeighborCfgData*)pdata;
        //UINT32 ulNeighborBTS = pNeighborDataElement->BtsIP;
        UINT32 neighborBtsId = pNeighborDataElement->BtsInfoIE.BTSID;
        ////
        //SendBySocket(ulNeighborBTS, (void*)&stSyncLoadInfo, ulLoadInfoLen);
        #if 1//def PAYLOAD_BALABCE
		if( ! bNotify )
		{
			if( PAYLOAD_NA == GetNBtsPayloadSupport(neighborBtsId) )
			{
				*(UINT32*)&stSyncLoadInfo.BTSLoadInfo.WeightedUserNo = stSyncLoadInfo.BTSLoadInfo.Reserve;
		        SendMsgTotSocket(neighborBtsId, (UINT8*)&stSyncLoadInfo, ulLoadInfoLen-2);
			    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_SyncBTSLoadInfo send old loadinfo to [%08X]", neighborBtsId );
			}
			else
			{
		        SendMsgTotSocket(neighborBtsId, (UINT8*)&stSyncLoadInfo, ulLoadInfoLen);
			    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_SyncBTSLoadInfo send new loadinfo to [%08X]", neighborBtsId );
			}
		}
		else
		{
	        SendMsgTotSocket(neighborBtsId, (UINT8*)&stSyncLoadInfo, ulLoadInfoLen);
		    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_SyncBTSLoadInfo send new notify to [%08X]", neighborBtsId );
		}
		#else
        SendMsgTotSocket(neighborBtsId, (UINT8*)&stSyncLoadInfo, ulLoadInfoLen);
		#endif
        ////跳到下一个neighbor BTS
        UINT16 usElementLen = pNeighborDataElement->length();
        if (0xFFFF == usElementLen)
            {
            return;
            }
        pdata += usElementLen;
        }
}

void CTaskCpeM :: CPE_CpeGetNeighborListReq(CMessage& rmsg)
{
    CL3OamCommonRsp ReqMsg;
    CCfgBtsNeibListReq  CfgBtsNeibListReq;
    UINT32 ulNvDataLen = CfgBtsNeibListReq.GetDataLenFromNvram();
    UINT32 ulBuffLen = 4 + ulNvDataLen;    /*transid(2) + version(2)*/
    if (false == ReqMsg.CreateMessage(*this, ulBuffLen))
        {
        return;
        }
    ReqMsg.SetMessageId(M_L3_CPE_GET_NEIGHBOR_LIST_RSP);
    ReqMsg.SetEID(rmsg.GetEID());
    ReqMsg.SetDstTid(M_TID_CPECM);
    UINT16 Transid = *(UINT16*)(rmsg.GetDataPtr());
    ReqMsg.SetTransactionId(Transid);
    UINT16 version = 0xffff;
    UINT8 * pdata = (UINT8*)(ReqMsg.GetDataPtr());
    memcpy(pdata + 2, &version, 2);
    UINT32 ulDataLen = 0;
    CfgBtsNeibListReq.GetDataFromNvramForCpe(pdata + 4, ulDataLen);
    ReqMsg.SetDataLength(ulDataLen+4);

    if(true != ReqMsg.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post Get neighbor BTS list response to CPE fail");
        ReqMsg.DeleteMessage();
    }
}

void CTaskCpeM :: CPE_BroadcastBtsLoadInfoToCpe(CMessage&)
{
#pragma pack(1)
    struct T_NeighbotBTSLoadInfoEle
    {
        UINT16 NeighborBTSNum;  
        T_NeighbotBTSLoadInfo NeighbotBTSLoadInfo[NEIGHBOR_BTS_NUM + 1];
    };
#pragma pack()

    T_NeighbotBTSLoadInfoEle AllNeighborBTSLoadInfo;
    UINT16 NeighborBTSNum = m_NeighborBTSLoadInfoEle.NeighborBTSNum;
    if(NEIGHBOR_BTS_NUM < NeighborBTSNum )
    {
         NeighborBTSNum = NEIGHBOR_BTS_NUM;
    }

    AllNeighborBTSLoadInfo.NeighborBTSNum = NeighborBTSNum + 1;   // 包括bts自己的信息
    AllNeighborBTSLoadInfo.NeighbotBTSLoadInfo[0].BTSID = bspGetBtsID();
    memcpy(&(AllNeighborBTSLoadInfo.NeighbotBTSLoadInfo[0].BTSLoadInfo),&gL3OamBTSLoadInfo, sizeof(T_BTSLoadInfo));
    memcpy(&(AllNeighborBTSLoadInfo.NeighbotBTSLoadInfo[1]),
           &(m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo),
           NeighborBTSNum * sizeof(T_NeighbotBTSLoadInfo));
    NeighborBTSNum = NeighborBTSNum + 1;

    CL3OamCommonRsp ReqMsg;
    UINT32 DataLen = 2 //NeighborBtsNum
                  + NeighborBTSNum * sizeof(T_NeighbotBTSLoadInfo);
    if (false == ReqMsg.CreateMessage(*this, DataLen + 4))
        return;

    ReqMsg.SetMessageId(M_L3_CPE_BROADCAST_BTS_LOADINFO);
    ReqMsg.SetEID(L3OAM_DEFAULT_CPEID);
    ReqMsg.SetDstTid(M_TID_CPECM);
    ReqMsg.SetTransactionId(OAM_DEFAUIT_TRANSID);
    UINT16 version = 0xffff;
    char * pdata = (char*)(ReqMsg.GetDataPtr());
    memcpy(pdata + 2, &version, 2);
    memcpy(pdata + 4, &AllNeighborBTSLoadInfo, DataLen);

    if(true != ReqMsg.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Broadcast BTS Loadinfo to CPE fail");
        ReqMsg.DeleteMessage();
    }
}

#include "L3OamCfgNeighbourBtsLoadInfo.h"
bool CTaskCpeM :: CPE_BtsNeighborBtsLoadInfoSyncReq(CMessage &msg)
{
    CL3OamSyncNeighbourBtsLoadInfoReq  msgSyncLoadInfo((const CMessage&)msg);
	UINT32 ulbtsid = msgSyncLoadInfo.getLoadInfo()->BTSID;
	UINT16 usWeigh = msgSyncLoadInfo.getLoadInfo()->BTSLoadInfo.WeightedUserNo;
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_BtsNeighborBtsLoadInfoSyncReq btsid[%08x] WeightedUserNo[%04x]", ulbtsid, usWeigh );

    UINT16 MsgLen = msgSyncLoadInfo.GetDataLength();
#if 1//def PAYLOAD_BALABCE
	NBtsPayload bPayloadSupport = PAYLOAD_UNKNOW;
    if (MsgLen >= sizeof(T_NeighbotBTSLoadInfo))//rcpe自动补0导致长度不正确，wwhua
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_BtsNeighborBtsLoadInfoSyncReq Get LoadInfo from bts[%08X], Payload[True]", ulbtsid );
        bPayloadSupport = PAYLOAD_A;
    }
    if (MsgLen == sizeof(T_NeighbotBTSLoadInfo)-2)
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_BtsNeighborBtsLoadInfoSyncReq Get LoadInfo from bts[%08X], Payload[false]", ulbtsid );
        bPayloadSupport = PAYLOAD_NA;
    }
    if (PAYLOAD_UNKNOW == bPayloadSupport)
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] BTS loading info length error.");
        return false;
    }
#else
    if (MsgLen != sizeof(T_NeighbotBTSLoadInfo))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] BTS loading info length error.");
        return false;
        }
#endif
    UINT16 usNeighborBtsNum = m_NeighborBTSLoadInfoEle.NeighborBTSNum;

    if (usNeighborBtsNum > NEIGHBOR_BTS_NUM)
        {
        //已经饱和
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Neighbor BTS number in loading info error.");
        m_NeighborBTSLoadInfoEle.NeighborBTSNum = 0;
        return false;
        }
    T_NeighbotBTSLoadInfo *pNvLoadInfo = NULL;
    T_NeighbotBTSLoadInfo *pRxLoadInfo = msgSyncLoadInfo.getLoadInfo();

    UINT16 i = 0;
    pNvLoadInfo = &(m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[0]);
    for (i = 0; i < usNeighborBtsNum; ++i)
        {
        //查是否已经有这个BTS的LoadInfo?
        pNvLoadInfo = &(m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[i]);
        if (pRxLoadInfo->BTSID == pNvLoadInfo->BTSID)
            {
            break;
            }
        }
    if (i == usNeighborBtsNum)
        {
        //查完了，Nvram没有该BTS的LoadInfo信息
        if (NEIGHBOR_BTS_NUM == i)
            {
            //已经饱和,不能再增加新的
            return false;
            }

        //增加NeighborBTS个数.
        m_NeighborBTSLoadInfoEle.NeighborBTSNum++;

        //pNvLoadInfo指向最后的LoadInfo,在最后追加
        pNvLoadInfo = &(m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[i]);
        }
    else
        {
        //Nvram存在该BTS的LoadInfo信息,仅仅覆盖
        //此时pNvLoadInfo指向目的LoadInfo.
        }
#if 1//def PAYLOAD_BALABCE
	if( PAYLOAD_A == bPayloadSupport )
	{
		m_ucNBtsRcdForPld[i] = PAYLOAD_A;
	}
	else
	{
		pRxLoadInfo->BTSLoadInfo.Reserve = *(UINT32*)&pRxLoadInfo->BTSLoadInfo.WeightedUserNo;
		pRxLoadInfo->BTSLoadInfo.WeightedUserNo = 10000;
		m_ucNBtsRcdForPld[i] = PAYLOAD_NA;
	}
#endif
    memcpy((char*)pNvLoadInfo, (char*)pRxLoadInfo, sizeof(T_NeighbotBTSLoadInfo));

    return true;
}

void CTaskCpeM ::  CPE_ProbeCpeFail(CMessage&)
{    
}

CTimer* CTaskCpeM :: CPE_Createtimer(UINT16 MsgId, UINT8 IsPeriod, UINT32 TimerPeriod)
{
    CL3OamCommonReq CheckCPETimer;
    if (false == CheckCPETimer.CreateMessage(*this))
        return NULL;
    CheckCPETimer.SetDstTid(M_TID_UM);
    CheckCPETimer.SetSrcTid(M_TID_UM);
    CheckCPETimer.SetTransactionId(OAM_DEFAUIT_TRANSID);
    CheckCPETimer.SetMessageId(MsgId);
    CTimer* timer = new CTimer(IsPeriod, TimerPeriod, CheckCPETimer);
    if (NULL == timer)
        {
        CheckCPETimer.DeleteMessage();
        }
    return timer;
}


bool CTaskCpeM::SendMsgTotSocket(UINT32 btsid, UINT8* pData, UINT32 len)
{
    //将发往的ip地址放在commessage最后
    CComMessage* pComMsg = new (this, sizeof(UINT8)*len) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed.");
        return false;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(this->GetEntityId());
    pComMsg->SetBTS(btsid);
    pComMsg->SetMessageId(M_SOCKET_LOADINGINFO_TX);
    memcpy((void*)pComMsg->GetDataPtr(), pData, len);
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }
}
//20100117,add by fengbing begin
extern "C" bool ifCpeCanAccess(UINT32 pid, UINT32 uid);
//20100117,add by fengbing end
extern "C" BOOL bspGetIsPermitUseWhenSagDown();
bool CTaskCpeM::CPE_AccessReq(const CUTAccessReq &msgAccessReq)
{
#define M_REGTYPE_STARTUP   (0)
    const T_UTAccessReq *pAccessReq = msgAccessReq.getAccessReq();
    bool wcpe_rcpe_flag =0;
    UINT32 eid = pAccessReq->ulPID;//v53 ut
    UINT32 btsid = pAccessReq->BTSID;
    if(btsid!=bspGetBtsID())//wangwenhua add 20080716
    {
        OAM_LOGSTR3(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] Access_Req, btsid not match:%x,%x.", msgAccessReq.GetEID(),btsid,bspGetBtsID());
	 return false;
    }
	//20100117,add by fengbing begin
	if(!ifCpeCanAccess(pAccessReq->ulPID, pAccessReq->ulUID))
	{
		return sendAuthResult(pAccessReq->ulUID, pAccessReq->ulPID, 0x0d);
	}
	//20100117,add by fengbing end
    if((Wanif_Switch== 0x5a5a)&&((NvRamDataAddr->WanIfCpeEid==eid)||(NvRamDataAddr->BakWanIfCpeEid==eid)))
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT,"[tCpeM]CPE[0x%.8X] this is wcpe.  ", eid);
        wcpe_rcpe_flag = 1;	    
    }
    else
    {
        //如果在rcpe中找到该eid则设置为1
        if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
        {
            for(int mm=0; mm<NvRamDataAddr->Relay_num; mm++)
            {
                if(RelayWanifCpeEid[mm]==eid)
                {
                    OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT,"[tCpeM]CPE[0x%.8X] this is rcpe.  ", eid);
                    wcpe_rcpe_flag = 1;
                    break;
                }
            }
        }
    }
    //如果sag坏了,并且故障弱化功能打开则直接回应答jiaying20100720
    if(sagStatusFlag == false)
    {
        if((TRUE== bspGetIsPermitUseWhenSagDown())||(wcpe_rcpe_flag == 1))//rcpe,wcpe直接回应答
        {
            OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] Access_Req, Sag is down, so Tx AuthenticationInfo_Result to UT.", msgAccessReq.GetEID());
            if(pAccessReq->ulUID==0xffffffff)
            {
                return sendAuthResult(pAccessReq->ulUID, pAccessReq->ulPID, 0, 1, 2);
            }
            else
                return sendAuthResult(pAccessReq->ulUID, pAccessReq->ulPID, OAM_SUCCESS);
        }
	 else//故障弱化开关关闭则丢弃消息
	 {
	     OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] Access_Req, Sag is down, not permit access.", msgAccessReq.GetEID());
	     return false;
	 }
    }
    if (M_REGTYPE_STARTUP != pAccessReq->ucRegType)
		//如果sag故障,也直接回成功
        {
        //BTS判断Reg_Type为切换，直接返回Authentication_Result，不进行认证
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] Access_Req, Tx AuthenticationInfo_Result to UT.", msgAccessReq.GetEID());
        return sendAuthResult(pAccessReq->ulUID, pAccessReq->ulPID, OAM_SUCCESS);
        }

    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[0x%.8x][UID:0x%.8X] Access_Req, Tx AuthenticationInfo_Req to SAG.", pAccessReq->ulPID, pAccessReq->ulUID);
    CsabisAuthenticationInfoReq msgAuthInfoReq;
    if(false == msgAuthInfoReq.CreateMessage(*this))
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] system failure, createMessage(AuthenticationInfo_Req) fail.");
        return false;
    }
#ifdef MZ_2ND
    if( 1 == (pAccessReq->ucVersion&0x01) )//判断为Z模块请求，带上MZPID
        msgAuthInfoReq.setInfo(pAccessReq->ulUID, pAccessReq->ulPID, pAccessReq->ulMZPid);
    else
        msgAuthInfoReq.setInfo(pAccessReq->ulUID, pAccessReq->ulPID);
#else
    msgAuthInfoReq.setInfo(pAccessReq->ulUID, pAccessReq->ulPID);
#endif
    msgAuthInfoReq.SetDstTid(M_TID_VCR);
    if(false == msgAuthInfoReq.Post())
    {
        msgAuthInfoReq.DeleteMessage();
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx AuthenticationInfo_Req to tVCR fail.");
        return false;
    }
    return true;
}


bool CTaskCpeM::CPE_sabisAuthCMD(const CsabisAuthenticationCMD &msgSabisAuthCMD)
{
    OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx Authentication_CMD from SAG, Tx Authentication_CMD to UT.");
    CUTAuthenticationCMD msgUTAuthCMD;
    if(false == msgUTAuthCMD.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] system failure, createMessage(Authentication_CMD to UT) fail.");
        return false;
        }
    const T_sabisAuthenticationCMD * pSabisCMD = msgSabisAuthCMD.getInfo();
    msgUTAuthCMD.setInfo(pSabisCMD->ulUID, pSabisCMD->ulPID, pSabisCMD->rand);
    msgUTAuthCMD.SetEID(pSabisCMD->ulPID);
    msgUTAuthCMD.SetDstTid(M_TID_CPECM);
    if(false == msgUTAuthCMD.Post())
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx Authentication_CMD to tUTCM fail.");
        msgUTAuthCMD.DeleteMessage();
        return false;
        }
    return true;
}


bool CTaskCpeM::CPE_UTAuthRsp(const CUTAuthenticationRsp &msgAuthRsp)
{    
    const T_UTAuthenticationRsp *pUTRsp = (const T_UTAuthenticationRsp*)msgAuthRsp.getInfo();
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] Authentication_Rsp, result : %d, Tx AuthenticationInfo_Rsp to SAG.", msgAuthRsp.GetEID(), pUTRsp->ulResult);
    CsabisAuthenticationRsp msgSabisRsp;
    if(false == msgSabisRsp.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] system failure, createMessage(Authentication_Rsp to SAG) fail.");
        return false;
        }
    msgSabisRsp.setInfo(pUTRsp->ulUID, pUTRsp->ulPID, pUTRsp->ulResult);
    msgSabisRsp.SetDstTid(M_TID_VCR);
    if(false == msgSabisRsp.Post())
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx Authentication_Rsp to tVCR fail.");
        msgSabisRsp.DeleteMessage();
        return false;
        }
    return true;
}


bool CTaskCpeM::CPE_sabisAuthResult(const CsabisAuthenticationResult &msgAuthResult)
{
	const T_sabisAuthenticationResult * const pResult = msgAuthResult.getInfo();
	if(pResult->Ind!=3)
	{
		if(pResult->Ind==0)
		{
			OAM_LOGSTR3(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] Authentication_Result, auth_result: %d, ind: %d, Tx AuthenticationInfo_Result to UT.", \
				pResult->ulPID, pResult->auth_result, pResult->Ind);
		}
		else
		{
			OAM_LOGSTR4(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] Authentication_Result, auth_result: %d, ind: %d, result: %d, Tx AuthenticationInfo_Result to UT.", \
				pResult->ulPID, pResult->auth_result, pResult->Ind, pResult->result);
		}

		//return sendAuthResult(pResult->ulUID, pResult->ulPID, pResult->result);
#ifdef MZ_2ND
        if( pResult->Ind==0 )
        {//ind==0时，ind后面只有mzpid，用result指针代替mzpid的指针
			OAM_LOGSTR4(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "CPE_sabisAuthResult->MZPID[%02X][%02X][%02X][%02X]", \
				*((UINT8*)&pResult->result), *((UINT8*)&pResult->result+1), *((UINT8*)&pResult->result+2), *((UINT8*)&pResult->result+3) );
    		return sendAuthResult(pResult->ulUID,pResult->ulPID,pResult->auth_result,pResult->Ind,
    			pResult->result,(UINT8*)&pResult->result,(UINT8*)(pResult->SID+16));
        }
        else
        {
    		return sendAuthResult(pResult->ulUID,pResult->ulPID,pResult->auth_result,pResult->Ind,
    			pResult->result,pResult->SID,(UINT8*)(pResult->SID+16));
        }
#else
		return sendAuthResult(pResult->ulUID,pResult->ulPID,pResult->auth_result,pResult->Ind,
			pResult->result,pResult->SID,(UINT8*)(pResult->SID+16));
#endif
	}
	else// if(pResult->Ind==3)
	{
		OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] Authentication_Result, ind is 3, auth_result: %d, Tx AuthenticationInfo_Result to UT.", \
			pResult->ulPID, pResult->auth_result);
		CUTAuthenticationResult msgUTAuthResult;
		if(false == msgUTAuthResult.CreateMessage(*this))
		{
			OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] system failure, createMessage(Authentication_Result to UT) fail.");
			return false;
		}
#ifdef MZ_2ND
		UINT8 ctemp1[30+4/*mzpid*/];
		memcpy(ctemp1, (UINT8*)(&pResult->Ind+1), 21+4/*mzpid*/);
		msgUTAuthResult.setInfo(pResult->ulUID,pResult->ulPID,pResult->auth_result,pResult->Ind,ctemp1, 21+4/*mzpid*/);
		msgUTAuthResult.setMsgLen(31+4/*mzpid*/);
#else
		UINT8 ctemp1[30];
		memcpy(ctemp1, (UINT8*)(&pResult->Ind+1), 21);
		msgUTAuthResult.setInfo(pResult->ulUID,pResult->ulPID,pResult->auth_result,pResult->Ind,ctemp1, 21);
		msgUTAuthResult.setMsgLen(31);
#endif
		msgUTAuthResult.SetEID(pResult->ulPID);
		msgUTAuthResult.SetDstTid(M_TID_CPECM);	
		if(false == msgUTAuthResult.Post())
		{
			msgUTAuthResult.DeleteMessage();
			OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx Authentication_Result to tUTCM fail.");
			return false;
		}
		return true;
	}   
}


bool CTaskCpeM::sendAuthResult(UINT32 ulUID, UINT32 ulPID, UINT8 result)
{
	OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx AuthenticationInfo_Result to CPE[0x%08X].", ulPID);
	CUTAuthenticationResult msgUTAuthResult;
	if(false == msgUTAuthResult.CreateMessage(*this))
	{
		OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] system failure, createMessage(Authentication_Result to UT) fail.");
		return false;
	}
	msgUTAuthResult.setInfo(ulUID, ulPID, result);
	msgUTAuthResult.SetEID(ulPID);
	msgUTAuthResult.SetDstTid(M_TID_CPECM);
	if(false == msgUTAuthResult.Post())
	{
		msgUTAuthResult.DeleteMessage();
		OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx Authentication_Result to tUTCM fail.");
		return false;
	}
	return true;
}
bool CTaskCpeM::sendAuthResult(UINT32 ulUID, UINT32 ulPID, UINT8 auth_result, UINT8 ind, UINT8 result)
{
	OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx AuthenticationInfo_Result to CPE[0x%08X].", ulPID);
	CUTAuthenticationResult msgUTAuthResult;
	if(false == msgUTAuthResult.CreateMessage(*this))
	{
		OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] system failure, createMessage(Authentication_Result to UT) fail.");
		return false;
	}
	msgUTAuthResult.setInfo(ulUID, ulPID, auth_result, ind, result);
	msgUTAuthResult.SetEID(ulPID);
	msgUTAuthResult.SetDstTid(M_TID_CPECM);
	if(false == msgUTAuthResult.Post())
	{
		msgUTAuthResult.DeleteMessage();
		OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx Authentication_Result to tUTCM fail.");
		return false;
	}
	return true;
}

bool CTaskCpeM::sendAuthResult(UINT32 ulUID,UINT32 ulPID,UINT8 auth_result,UINT8 ind ,UINT8 result ,const UINT8* SID,const UINT8* CID)
{
	OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx AuthenticationInfo_Result to CPE[0x%08X].", ulPID);
	CUTAuthenticationResult msgUTAuthResult;
	if(false == msgUTAuthResult.CreateMessage(*this))
	{
		OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] system failure, createMessage(Authentication_Result to UT) fail.");
		return false;
	}
	OAM_LOGSTR4(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "sendAuthResult->MZPID[%02X][%02X][%02X][%02X]", \
		*(SID), *(SID+1), *(SID+2), *(SID+3) );
	msgUTAuthResult.setInfo(ulUID, ulPID, auth_result,ind,result,SID,CID);
	msgUTAuthResult.SetEID(ulPID);
	msgUTAuthResult.SetDstTid(M_TID_CPECM);
	if(false == msgUTAuthResult.Post())
	{
		msgUTAuthResult.DeleteMessage();
		OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx Authentication_Result to tUTCM fail.");
		return false;
	}
	return true;

}
bool CTaskCpeM::CPE_AccountLogin_req(const CUTAccountLoginReq &msgAccountLoginReq)
{
    const T_AccountLoginReq *pAccountLoginReq = msgAccountLoginReq.getAccountLoginReq();
    UINT32 btsid = pAccountLoginReq->BTSID;
    if(btsid!=bspGetBtsID())//wangwenhua add 20080716
    {
        OAM_LOGSTR3(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[%.8x] AccountLogin_Req, btsid not match:%x,%x.", msgAccountLoginReq.GetEID(),btsid,bspGetBtsID());
	 return false;
    }    
    OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[0x%.8x]AccountLogin_Req, Tx AccountLogin_Req to SAG.", msgAccountLoginReq.GetEID());
    CsabisAccountLoginReq sabisAccountLoginReq;
    if(false == sabisAccountLoginReq.CreateMessage(*this))
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] system failure, createMessage(AuthenticationInfo_Req) fail.");
        return false;
    }
    UINT16 tLen;
#if 0//liujianfeng alter for mz,mz's pAccountLoginReq->AccountType=[2|3|4|5]
    if(pAccountLoginReq->AccountType ==0)//carry uid
		tLen = 30;
    else if(pAccountLoginReq->AccountType ==1)
		tLen = 44;
    else
    {
        OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[0x%.8x]AccountLogin_Req, AccountType is :%d, error!", msgAccountLoginReq.GetEID(), pAccountLoginReq->AccountType);
        return false;
    }
#else
    switch (pAccountLoginReq->AccountType)
    {
        case 0:
        case 2:
        case 3:
        case 4:
        case 5:
    		tLen = 30;
            break;
        case 1:
        	tLen = 44;
            break;
        default:
            OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[0x%.8x]AccountLogin_Req, AccountType is :%d, error!", msgAccountLoginReq.GetEID(), pAccountLoginReq->AccountType);
            return false;
    }
#endif
    sabisAccountLoginReq.setInfo(&pAccountLoginReq->ActiveType, tLen);
    sabisAccountLoginReq.SetDstTid(M_TID_VCR);
    if(false == sabisAccountLoginReq.Post())
    {
        sabisAccountLoginReq.DeleteMessage();
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx AccountLogin_Req to tVCR fail.");
        return false;
    }
    return true;
}
#if 0
extern "C" int getSTDNum(int*);
void ModifyLoadInfo() 
{
    printf("\r\nModify BTS Load Info(for test only).");
    UINT32 usValue = 0;
    printf("\r\n%30s: ", "Current User Number");
    getSTDNum((int*)&usValue);
    gTestBTSLoadInfo.CurrentUserNumber = usValue;

    printf("\r\n%30s: ", "Average Free DL channel");
    getSTDNum((int*)&usValue);
    gTestBTSLoadInfo.AverageFreeDLchannel = usValue;

    printf("\r\n%30s: ", "Average Free UL channel");
    getSTDNum((int*)&usValue);
    gTestBTSLoadInfo.AverageFreeULchannel = usValue;

    printf("\r\n%30s: ", "Average Free Power Usage");
    getSTDNum((int*)&usValue);
    gTestBTSLoadInfo.AverageFreePowerUsage = usValue;

    printf("\r\nDetail");
    printf("\r\n%30s: %d", "BTS ID", bspGetBtsID());
    printf("\r\n%30s: %d", "Current User Number", gTestBTSLoadInfo.CurrentUserNumber);
    printf("\r\n%30s: %d", "Average Free DL channel", gTestBTSLoadInfo.AverageFreeDLchannel);
    printf("\r\n%30s: %d", "Average Free UL channel", gTestBTSLoadInfo.AverageFreeULchannel);
    printf("\r\n%30s: %d", "Average Free Power Usage", gTestBTSLoadInfo.AverageFreePowerUsage);

    return;
}

void l3oamDebugLoadInfo()
{
    bDebugLoadInfoMode = !bDebugLoadInfoMode;
    if (true == bDebugLoadInfoMode)
        {
        printf("\r\n==>Enter into Loadinfo Debug mode...");
        ModifyLoadInfo();
        }
    else
        {
        printf("\r\nEXIT from Loadinfo Debug mode==>!");
        }
}
#endif


void l3oamshowcpe()
{

    printf("[tCpeM] show all CPEs, including movedaway and switchoff CPEs");
#ifndef WBBU_CODE
    list<T_ToBeDeleteCpe>::iterator it;
    printf("\r\n%-10s:%-8s:%-15s","Eid", "TTL(min)", "state");
    printf("\r\n-----------------------------------");
    for(it = CTaskCpeM::m_ToBeDeleteCpeList.begin(); it != CTaskCpeM::m_ToBeDeleteCpeList.end(); ++it)
        {
        const UINT32 eid = it->CPEID;
        const CPECCB *pCCB = (CPECCB*)CTaskCpeM::GetInstance()->m_CpeFsm.FindCCB(eid);
        if (NULL == pCCB)
            printf("\r\n0x%-8X:%-8d:%-15s", eid, it->RemanentTime, "CCB not found");
        else
            printf("\r\n0x%-8X:%-8d:%-15s", eid, it->RemanentTime, (const UINT8*)strCpeSTATE[((CPECCB*)pCCB)->GetCurrentState()]);
        }
    printf("\r\ntotal:%d", CTaskCpeM::m_ToBeDeleteCpeList.size());
#else
	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	    list<T_ToBeDeleteCpe>::iterator it;
	    /*
	    for(it = CTaskCpeM::m_ToBeDeleteCpeList.begin(); it != CTaskCpeM::m_ToBeDeleteCpeList.end(); ++it)
	        {
	        const UINT32 eid = it->CPEID;
	        const CPECCB *pCCB = (CPECCB*)CTaskCpeM::GetInstance()->m_CpeFsm.FindCCB(eid);
	        if (NULL == pCCB)
	            {
	            printf("\r\nEid: 0x%8x, period: %d(min), state: CCB not found!!", eid, it->RemanentTime);
	            }
	        else
	            printf("\r\nEid: 0x%8x, period: %d(min), state: %s", eid, it->RemanentTime, (const UINT8*)strCpeSTATE[((CPECCB*)pCCB)->GetCurrentState()]);
	        }
	    printf("\r\ntotal:%d", CTaskCpeM::m_ToBeDeleteCpeList.size());
	    */
	    for(it = pTInstance->m_ToBeDeleteCpeList.begin(); it != pTInstance->m_ToBeDeleteCpeList.end(); ++it)
	        {
	        const UINT32 eid = it->CPEID;
	        const CPECCB *pCCB = (CPECCB*)CTaskCpeM::GetInstance()->m_CpeFsm.FindCCB(eid);
	        if (NULL == pCCB)
	            {
	            printf("\r\nEid: 0x%8x, period: %d(min), state: CCB not found!!", eid, it->RemanentTime);
	            }
	        else
	            printf("\r\nEid: 0x%8x, period: %d(min), state: %s", eid, it->RemanentTime, (const UINT8*)strCpeSTATE[((CPECCB*)pCCB)->GetCurrentState()]);
	        }
	    printf("\r\ntotal:%d", pTInstance->m_ToBeDeleteCpeList.size());

#endif
}


void CTaskCpeM::CPE_ClearHist(CMessage& rMsg)//liujianfeng 20080515 CPE clear HIST
{
//    UINT16 ReqTranid = *(UINT16*)(rMsg.GetDataPtr());
//    CL3OamCommonRsp L3OamCommonReq(rMsg); 
//    UM_PostCommonRsp(M_TID_EMSAGENTTX, ReqTranid, M_BTS_EMS_RESET_UT_RSP, OAM_SUCCESS);

    CCpeCommonReq ReqMsg;
    if (false == ReqMsg.CreateMessage(*this))
        return;

    ReqMsg.SetMessageId( rMsg.GetMessageId());
	ReqMsg.SetTransactionId( 0xAAAA );
    UINT32 eid = *(UINT32*)((SINT8*)rMsg.GetDataPtr());
    ReqMsg.SetEID(eid);
    ReqMsg.SetCPEID(eid);
    ReqMsg.SetDstTid(M_TID_CPECM);
    if (false == ReqMsg.Post())
        {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post Reset CPE[0x%x] message fail", eid);
        ReqMsg.DeleteMessage();
        return;
        }
    return;
	
}
/*
查询终端性能数据消息处理,直接转发给l2查询,收到应答后返回给ems
*/
void CTaskCpeM::CPE_BtsUtStatusDataReq(CMessage& rMsg)
{    
    CComMessage* pComMsg = new (this, 6) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_BtsUtStatusDataReq.");
        return ;
    }
    pComMsg->SetDstTid(M_TID_L2MAIN);
    pComMsg->SetSrcTid(M_TID_UM); 
    pComMsg->SetMessageId(M_L3_L2_BTS_UT_STATUS_DATA_REQ);
    memcpy((pComMsg->GetDataPtr()), ((UINT8*)(rMsg.GetDataPtr())), 6);
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }
}

void CTaskCpeM::CPE_BtsUtStatusDataRsp(CMessage& rMsg)
{
    //将发往的ip地址放在commessage最后
    CComMessage* pComMsg = new (this, rMsg.GetDataLength()) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_BtsUtStatusDataRsp.");
        return ;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_UM);    
    pComMsg->SetMessageId(M_BTS_EMS_UT_STATUS_DATA_RSP);    
    memcpy(((UINT8*)(pComMsg->GetDataPtr())), (rMsg.GetDataPtr()), rMsg.GetDataLength());
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }
}
//zengjihan 20120503 for MEM
void CTaskCpeM::CPE_EmsBtsUtMemInfoReportReq(CMessage& rMsg)
{    
    UINT32 eid=0;
    CComMessage* pComMsg = new (this, rMsg.GetDataLength()) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_EmsBtsUtMemInfoReportReq.");
        return ;
    }
    pComMsg->SetDstTid(M_TID_CPECM);
    pComMsg->SetSrcTid(M_TID_UM); 
    memcpy((UINT8*)&eid,((UINT8*)(rMsg.GetDataPtr())+6), 4);
    pComMsg->SetEID(eid);
    pComMsg->SetMessageId(M_BTS_UT_MEMINFO_REPORT_REQ);
    memcpy((pComMsg->GetDataPtr()), ((UINT8*)(rMsg.GetDataPtr())), rMsg.GetDataLength());
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }
}
//zengjihan 20120503 for MEM
void CTaskCpeM::CPE_UtBtsEmsMemInfoReportRsp(CMessage& rMsg)
{
    //将发往的ip地址放在commessage最后
    CComMessage* pComMsg = new (this, rMsg.GetDataLength()) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_UtBtsEmsMemInfoReportRsp.");
        return ;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_UM);
    pComMsg->SetMessageId(M_BTS_EMS_MEMINFO_REPORT_RSP);
    memcpy(((UINT8*)(pComMsg->GetDataPtr())), (rMsg.GetDataPtr()), rMsg.GetDataLength());
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }
}
/*查询CPE携带的iplist信息
*/
void CTaskCpeM::CPE_GetIplistReq(CMessage& rMsg)
{
	UINT16 ReqTranid = *(UINT16*)(rMsg.GetDataPtr());
	CCpeCommonReq ReqMsg;
	if (false == ReqMsg.CreateMessage(*this))
		return;

	ReqMsg.SetMessageId(M_CPEM_DM_CPE_PROBE_REQ);
	UINT32 eid = *(UINT32*)((SINT8*)rMsg.GetDataPtr()+2);
	ReqMsg.SetEID(eid);
	ReqMsg.SetCPEID(eid);
	ReqMsg.SetDstTid(M_TID_DM);
       ReqMsg.SetTransactionId(ReqTranid);
	if (false == ReqMsg.Post())
	{
		OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post Reset CPE[0x%x] message fail", eid);
		ReqMsg.DeleteMessage();		
	}
	
}
/*
查询终端三层参数消息处理,直接转发给cpe查询,收到应答后返回给ems
*/
void CTaskCpeM::CPE_BtsLayer3DataReq(CMessage& rMsg)
{
	UINT32 eid;
	
	CComMessage* pComMsg = new (this, rMsg.GetDataLength()) CComMessage;
	if (pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_BtsLayer3DataReq.");
		return;
	}

	eid = *(UINT32*)((UINT8*)rMsg.GetDataPtr()+2);
	pComMsg->SetDstTid(M_TID_CPECM);
	pComMsg->SetSrcTid(M_TID_UM);    
	pComMsg->SetEID(eid);
	pComMsg->SetMessageId(M_BTS_CPE_UT_LAYER3_DATA_REQ);    	
	memcpy(((UINT8*)pComMsg->GetDataPtr()), rMsg.GetDataPtr(), rMsg.GetDataLength());
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
	return;
}

void CTaskCpeM::CPE_BtsLayer3DataRsp(CMessage& rMsg)
{
	//将发往的ip地址放在commessage最后
	CComMessage* pComMsg = new (this, rMsg.GetDataLength()) CComMessage;
	if (pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_BtsLayer3DataRsp.");
		return;
	}
	pComMsg->SetDstTid(M_TID_EMSAGENTTX);
	pComMsg->SetSrcTid(M_TID_UM);    
	pComMsg->SetMessageId(M_BTS_EMS_UT_LAYER3_DATA_RSP);    	
	memcpy(((UINT8*)pComMsg->GetDataPtr()), rMsg.GetDataPtr(), rMsg.GetDataLength());
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
	return;
}
bool CTaskCpeM :: CPE_SagProfileReqFail(CMessage& rMsg)
{
     CL3OamCommonRsp failMsg(rMsg);
    UINT16 TransId = failMsg.GetTransactionId();

    CPECCB* pCPECCB = (CPECCB*)m_CpeFsm.FindCCB(failMsg);
 

    UINT32 Eid = rMsg.GetEID();
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] sag CPE[0x%04x] profile req fail,transID = 0x%x.", Eid,TransId);
    if(pCPECCB!=NULL)
    pCPECCB->setBWInfoTransId(0);//set 0 jiaying20100816
    CTransaction * pTransaction = FindTransact(TransId);
    if(!pTransaction)
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] can not find transid[0x%x] in msg[M_CPE_L3_PROFILE_UPDATE_RSP]", TransId);
        return false;
    }
    else
    {
        pTransaction->EndTransact();
        delete pTransaction;
     }
     return true;
}
void CTaskCpeM::CPE_NetIf_Cfg(UINT32 eid, CPE_NETIF_CFG_TYPE cmdType, UINT8 * content)
{
	CL3CpeEthCfgReq ReqMsg;
	UINT32 uMsgLen = 0;

	if(cmdType >= CPE_NETIF_CFG_MAX_TYPE)
	{
		OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Cpe net interface cfg, wrong cmd type:%d!", cmdType);
		return;
	}

	if((cmdType==WRITE_CPE_RTL8150_REG)||(cmdType == CFG_CPE_NETIF_SPEED_DUPLEX ))
	{
		uMsgLen = 6;
	}
	else
	{
		uMsgLen = 4;
	}

	if (false == ReqMsg.CreateMessage(*this, uMsgLen))
        return;

    	ReqMsg.SetMessageId(M_L3_BTS_CPE_ETHERNET_CFG_REQ);
    	ReqMsg.SetDstTid(M_TID_CPECM);
	ReqMsg.SetEID(eid);
    	ReqMsg.SetCmdType(cmdType);

	if(cmdType>READ_CPE_NETIF_STATUS)
	{
	    if((content!=NULL)&&(uMsgLen>2))//20110416
	    {
		::memcpy(ReqMsg.GetCmdContent(), content, uMsgLen-2);
	    }
	}	
	
    	if(true != ReqMsg.Post())
    	{
        	OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post Reset Network interface cmd to CPE fail");
        	ReqMsg.DeleteMessage();
    	}
	
}

void CPE_Reset_Rsp(CL3L3CpeEthCfgRsp & RspCmd)
{

	if(!(*(UINT16 *)RspCmd.GetCmdContent()))
	{
		printf("Reset CPE network interface failed");
	}
	else if(*(UINT16 *)RspCmd.GetCmdContent()==1)
	{
		printf("Reset CPE network interface succeed");
	}
	else
	{
		printf("Reset CPE network interface N/A");
	}
}


void CPE_Netif_Reg_Rsp(CL3L3CpeEthCfgRsp & RspCmd)
{
	UINT8 * pContent = NULL;
	UINT8 value = 0;
	UINT16 value1=0;

	pContent = RspCmd.GetCmdContent();

	printf("--CPE Network interface status reg--\n");

	/////////////////////////////////////////////////////////////////////////
	printf("1.Transmit wrong number before network interface reset:0x%x\n", *pContent);
	pContent++;
	
	/////////////////////////////////////////////////////////////////////////
	printf("2.Status of last transmit:0x%x--", *pContent);
	if(!(*pContent ))
	{
		printf("normal\n");
	}
	else
	{
		printf("abnormal\n");
	}
	pContent++;

	//////////////////////////////////////////////////////////////////////////
	printf("3.RTL8150 Trasmit Status Reg:0x%x\n", *pContent);
	printf("value:\n");
	value = *pContent;
	printf("Tx buffer full indication:%d\n", value&1);
	value=value>>1;
	printf("Tx buffer empty indication:%d\n", value&1);
	value=value>>1;
	printf("Jaber time out indication:%d\n", value&1);
	value=value>>1;
	printf("Loss of carrier indication:%d\n", value&1);
	value = value >>1;
	printf("Late collision indication:%d\n", value&1);
	value = value >>1;
	printf("Excessive collision indication:%d\n", value&1);
	pContent++;

	//////////////////////////////////////////////////////////////////////////
	printf("4.RTL8150 Receive Status Reg:0x%x\n", *pContent);
	value = *pContent;
	printf("Receive OK indication:%d(1:indicate that a packet is received without error.)", value&1);
	value = value >>1;
	printf("Frame Alignment Error:%d(1:indicates that a frame alignment error occurred on this received packet.)", value&1);
	value = value >>1;
	printf("CRC Error indication:%d(1:The received packet is checked with CRC error)\n", value&1);
	value = value >>1;
	printf("Long Packet indication:%d(1:The size of the received packer exceeds 4K bytes.)\n", value&1);
	value = value >>1;
	printf("Runt Packet indication:%d(1:The received packet length is smaller than 64bytes)\n", value&1);
	value = value>>1;
	printf("Link change indication:%d\n", value&1);
	value = value >>1;
	printf("Rx Buffer Full indication:%d\n", value&1);
	value=value>>1;
	printf("Wake Up Event indication:%d(1:Wakeup event occurs)\n", value&1);
	pContent++;

	//////////////////////////////////////////////////////////////////////////
	printf("5.Media Status Reg:0x%x\n", *pContent);
	value = *pContent;
	printf("RXPF:%d\n", value&1);
	printf("    1:Indicate that RTL8150L(M) is in Backoff state because a pause packet from remote station has veen receipt.\n");
	printf("    2:Indicate that RTL8150L(M) is not in pause state.\n");
	value = value >>1;
	printf("TXPF:%d\n", value&1);
	printf("    1:Indicate that RTL8150L(M) sends pause packet.\n");
	printf("    2:Indicate that RTL8150L(M) has sent timer done packet to release remote sation from pause Tx state.\n");
	value = value >>1;
	printf("LINK:%d(1:Link OK; 0:Link Fail)", value&1);
	value = value >>1;
	printf("SPEED_100:%d\n", value&1);
	printf("    1:Indicate that the current link is in 100Mbps mode\n");
	printf("    0:Indicate that the current link is in 10Mbps mode\n");
	value = value >>1;
	printf("Duplex:%d\n", value&1);
	printf("    1:Indicate that the current link is full-duplex\n");
	printf("    0:Indicate that the current link is half-duplex\n");
	pContent=pContent+2;

	//////////////////////////////////////////////////////////////////////////
	printf("6.Basic Mode StatusReg:0x%x\n",*(UINT16*)pContent);
	value1=*(UINT16*)pContent;
	printf("Extended Capability:%d\n", value1&1);
	printf("    1:Extended register capabilities\n");
	printf("    0:Basic register set capabilities\n");
	value1 = value1 >>3;
	printf("Auto Negotiation ability:%d\n", value1 &1);
	printf("    1:Device is able to perform Auto-Negotiation\n");
	printf("    0:Device not able to perform Auto-Negotiation\n");
	value1= value1 >>1;
	printf("Remote fault:%d\n", value1&1);
	printf("    1:Remote fault condition detected(clear or read)\n");
	printf("    0:No remote fault condition detected\n");
	value1 = value1 >>1;
	printf("Auto Negotiation Complete:%d\n", value1&1);
	printf("    1:Auto negotiation process completed\n");
	printf("    0:Auto negotiation process not completed\n");
	value1 = value1 >>6;
	printf("10Base-T Half Duplex Capable:%d\n", value1&1);
	printf("    1:Device able to perform 10Base-T in half duplex mode\n");
	value1 = value1>>1;
	printf("10Base-T Full Duplex Capable:%d\n", value1&1);
	printf("    1:Device able to perform 10Base-T in full duplex mode\n");
	value1 = value1 >>1;
	printf("100Base-TX half Duplex Capable:%d\n", value1&1);
	printf("    1:Device able to perform 100Base-Tx in half duplex mode\n");
	value1 = value1 >>1;
	printf("100 Base-TX Full Duplex Capable:%d\n", value1&1);
	printf("    1:Device able to perform 100Base-TX in full duplex mode\n");
	value1 = value1 >>1;
	printf("100Base-T4 Capable:%d\n", value1&1);
	printf("    0:Device not able to perform 100Base-T4 mode\n");
	pContent=pContent+2;

	//////////////////////////////////////////////////////////////////////////
	printf("7.Auto Negotiation Link partner Reg:0x%x\n", *(UINT16*)pContent);
	value1 = *(UINT16*)pContent;
	printf("Protocol Selection Bits:%d\n", value1&0x11111);
	printf("    Link Partner's binary encoded protocol selector\n");
	value1 = value1>>5;
	printf("10BASE-T Support:%d\n", value1&1);
	printf("    1:10Base-T is supported by the link partner\n");
	printf("    0:10Base-T not supported by the link partner\n");
	value1 = value1>>1;
	printf("10BASE-T Full Duplex Support:%d\n", value1&1);
	printf("    1:10Base-T full duplex is supported by the link partner\n");
	printf("    0:10Base-T full duplex not supported by teh link partner\n");
	value1 = value1 >>1;
	printf("100BASE-TX support:%d\n", value1&1);
	printf("    1:100Base-TX is supported by the link partner\n");
	printf("    0:100Base-TX not supported by the link partner\n");
	value1 = value1 >>1;
	printf("100BASE-TX Full Duplex Support:%d\n", value1&1);
	printf("    1:100Base-TX full duplex is supported by the link partner\n");
	printf("    0:100Base-TX full duplex not supported by the link partner\n");
	value1 = value1 >>1;
	printf("100BASE-T4 Support:%d\n", value1&1);
	printf("    1:100Base-T4 is supported by the link partner\n");
	printf("    0:100Base-T4 not supported by the link partner\n");
	value1 = value1>>1;
	printf("Pause:%d\n", value1&1);
	printf("    1:Flow control is supported by the link partner\n");
	printf("    0:Flow control is not supported by the link partner\n");
	value1 = value1>>3;
	printf("Remote Fault:%d\n", value1&1);
	printf("    1:Remote fault indicated by the link partner\n");
	printf("    0:No remote fault indicated by the link partner\n");
	value1=value1>>1;
	printf("ACK:%d\n", value1&1);
	printf("    1:Link partner acknowledges reception of the capability data word\n");
	printf("    0:No acknowledged\n");
	value1=value1>>1;
	printf("Next Page Indication:%d\n", value1&1);
	printf("    1:Link partner does not desire Next Page Transfer\n");
	printf("    0:Link partner desires Next Page Transfer\n");	
	
}

void CPE_Read_8150_Reg_Rsp(CL3L3CpeEthCfgRsp & RspCmd)
{
	printf("--RTL8150 reg--\n");
	printf("Value:0x%x\n", *(UINT16*)RspCmd.GetCmdContent());
}

void CPE_Write_8150_Reg_Rsp(CL3L3CpeEthCfgRsp & RspCmd)
{
	printf("--Write RTL8150 reg--");
	if(!(*(UINT16 *)RspCmd.GetCmdContent()))
		printf("Failed\n");
	else if(*(UINT16 *)RspCmd.GetCmdContent()==1)
		printf("Succeed\n");
	else
		printf("N/A\n");

}

void CPE_Netif_Statistic(CL3L3CpeEthCfgRsp & RspCmd)
{
	UINT32 * pContent = NULL;

	pContent = (UINT32 *)RspCmd.GetCmdContent();

	printf("--CPE Network interface statistic value--\n");
	printf("1.UL packet lost counter:0x%x\n", *pContent);
	pContent++;
	printf("2.UL packet filter counter:0x%x\n", *pContent);
	pContent++;
	printf("3.UL packet counter(include diag packet):0x%x\n", *pContent);
	pContent++;
	printf("4.DL packet counter:0x%x\n", *pContent);
	pContent++;
	printf("5.DL packet inqueue failed counter:0x%x\n",*pContent);

}


void CPE_Cfg_Netif_Ada_Rsp(CL3L3CpeEthCfgRsp & RspCmd)
{
	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	

	printf("--Configure CPE network interface adapation mode:--");
	if(*(UINT16*)RspCmd.GetCmdContent()==0)
	{
		printf("FAIL!\n");
	}
	else if(*(UINT16*)RspCmd.GetCmdContent()==1)
	{
	
		if(pTInstance->Get_TempNetIfAdaStatus() == CPE_NETIF_ADA_NONEAUTO)
		{
			pTInstance->Set_TempNetIfAdaStatus(CPE_NETIF_ADA_INIT);
			pTInstance->Set_NetIfAdaStatus(CPE_NETIF_ADA_NONEAUTO);
		}
		else if(pTInstance->Get_TempNetIfAdaStatus() == CPE_NETIF_ADA_AUTO)
		{
			pTInstance->Set_TempNetIfAdaStatus(CPE_NETIF_ADA_INIT);
			pTInstance->Set_NetIfAdaStatus(CPE_NETIF_ADA_AUTO);
		}

		printf("SUCC!\n");
	}
	else
	{
		printf("N/A\n");
	}
}

void CPE_Cfg_Netif_Speed_Deplux_Rsp(CL3L3CpeEthCfgRsp & RspCmd)
{
	printf("--Configure CPE network interface Speed and Deplux:--");
	if(*(UINT16*)RspCmd.GetCmdContent()==0)
	{
		printf("FAIL!\n");
	}
	else if(*(UINT16*)RspCmd.GetCmdContent()==1)
	{
	
		printf("SUCC!\n");
	}
	else
	{
		printf("N/A\n");
	}
}

void CPE_Read_Netif_Speed_Deplux_Rsp(CL3L3CpeEthCfgRsp & RspCmd)
{
	UINT16 *Result = NULL;
	
	printf("--Read CPE network interface Speed and Deplux--");

	Result = (UINT16*)RspCmd.GetCmdContent();

	printf("1. Auto adapation mode:");
	if(*Result == 0x1000)
	{
		printf("AUTO\n");
	}
	else if(*Result == 0x1001)
	{
		printf("NO AUTO\n");
	}
	else
	{
		printf("N/A\n");
	}

	Result++;

	printf("2. Speed selection:");
	if(*Result == 0x1000)
	{
		printf("10M\n");
	}
	else if(*Result == 0x1001)
	{
		printf("100M\n");
	}
	else
	{
		printf("N/A\n");
	}

	Result++;

	printf("3. Duplex mode:");
	if(*Result == 0x1000)
	{
		printf("Half duplex\n");
	}
	else if(*Result == 0x1001)
	{
		printf("Full duplex\n");
	}
	else
	{
		printf("N/A\n");
	}

	return;
	
}



 bool CTaskCpeM::CPE_Netif_Rsp(CMessage& rMsg)
{
	
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%8x] Network interface cfg Response", rMsg.GetEID());
    CL3L3CpeEthCfgRsp RspMsg(rMsg);
    UINT32 eid = rMsg.GetEID();
    UINT16 cmdType = RspMsg.GetCmdType();
     CPECCB *CCB = NULL;
	UINT16 ReqTranid;
	  UINT16 *Result = NULL;
	  UINT16 type;
	   CL3OamQueryCPENetWorkRsp msgQueryNetWorkCPERsp;
    if(cmdType >= CPE_NETIF_CFG_MAX_TYPE)
   {
	OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Recv Cpe net interface Rsp, wrong cmd type:%d!", cmdType);
	return false;
    }
       OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%8x] Network interface cfg Response:cmd:%x!", rMsg.GetEID(),cmdType);
   switch(cmdType)
   {
	case RST_CPE_NETIF:
			::CPE_Reset_Rsp(RspMsg);
		break;
	case READ_CPE_NETIF_REG:
		       ::CPE_Netif_Reg_Rsp(RspMsg);
		break;
	case READ_CPE_RTL8150_REG:
			::CPE_Read_8150_Reg_Rsp(RspMsg);	
		break;
	case READ_CPE_NETIF_STATUS:
			::CPE_Netif_Statistic(RspMsg);
		break;
	case WRITE_CPE_RTL8150_REG:
			::CPE_Write_8150_Reg_Rsp(RspMsg);	
		break;
	case CFG_CPE_NETIF_ADA:
		::CPE_Cfg_Netif_Ada_Rsp(RspMsg);
	      //这里要给网管发送一个回应消息，表示配置是否成功?
		//CPECCB *CCB;
               CCB = (CPECCB *)m_CpeFsm.FindCCB(eid);
               if(CCB==NULL)
    	        {
    	                return false ;
    	         }
	         ReqTranid = CCB->getCfgTranid();
		if(*(UINT16*)RspMsg.GetCmdContent()==0)
	       {
		     UM_PostCommonRsp(M_TID_EMSAGENTTX, ReqTranid, M_BTS_EMS_CFG_NETWORK_UT_RSP, OAM_FAILURE);

	        }
	        else if(*(UINT16*)RspMsg.GetCmdContent()==1)
	        {
		     UM_PostCommonRsp(M_TID_EMSAGENTTX, ReqTranid, M_BTS_EMS_CFG_NETWORK_UT_RSP, OAM_SUCCESS);

	        }
	
		break;
	case CFG_CPE_NETIF_SPEED_DUPLEX:
			::CPE_Cfg_Netif_Speed_Deplux_Rsp(RspMsg);
	        
             CCB = (CPECCB *)m_CpeFsm.FindCCB(eid);
               if(CCB==NULL)
    	        {
    	                return false;
    	         }
	         ReqTranid = CCB->getCfgTranid();
		   if(*(UINT16*)RspMsg.GetCmdContent()==0)
	           {
	                UM_PostCommonRsp(M_TID_EMSAGENTTX, ReqTranid, M_BTS_EMS_CFG_NETWORK_UT_RSP, OAM_FAILURE);
	            }
	        else if(*(UINT16*)RspMsg.GetCmdContent()==1)
	           {
	
		        UM_PostCommonRsp(M_TID_EMSAGENTTX, ReqTranid, M_BTS_EMS_CFG_NETWORK_UT_RSP, OAM_SUCCESS);
	           }
		break;
	case READ_CPE_NETIF_SPEED_DUPLEX:
			::CPE_Read_Netif_Speed_Deplux_Rsp(RspMsg);
	   //   CPECCB *CCB;
             CCB = (CPECCB *)m_CpeFsm.FindCCB(eid);
             if(CCB==NULL)
    	      {
    	                return false;
    	       }
	   ReqTranid = CCB->getQueryTransid();
	
	
        if (false == msgQueryNetWorkCPERsp.CreateMessage(*this))
            {
            OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]ERROR!!! System encounter exceptions, new Message fail.");
            return false;
            }
        msgQueryNetWorkCPERsp.SetDstTid(M_TID_EMSAGENTTX);
        msgQueryNetWorkCPERsp.SetTransactionId(ReqTranid);
        msgQueryNetWorkCPERsp.SetMessageId(M_EMS_BTS_CFG_NETWORK_UT_QUERY_RSP);
        msgQueryNetWorkCPERsp.SetResult(OAM_SUCCESS);
        msgQueryNetWorkCPERsp.setRspEID(eid);
       
     

	Result = (UINT16*)RspMsg.GetCmdContent();

	
	if(*Result == 0x1000) //	printf("AUTO\n");//直接回送消息
	{
	
		type = 0;
	  msgQueryNetWorkCPERsp.setRspType( type);
	    if (true != msgQueryNetWorkCPERsp.Post())
           {
            OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post msg[0x%04x]  to task[%d] fail", msgQueryNetWorkCPERsp.GetMessageId(), msgQueryNetWorkCPERsp.GetSrcTid());
            msgQueryNetWorkCPERsp.DeleteMessage();
            
           }
	    return true;
	}
	else if(*Result == 0x1001) //printf("NO AUTO\n");
	{
		
	}


	Result++;


	if(*Result == 0x1000)//10M
	{
		
		 Result++;

		if(*Result == 0x1000) //	printf("Half duplex\n");
		{
		
			type = 2;
		}
		else if(*Result == 0x1001) //	printf("Full duplex\n");
		{
		
			type = 1;
		}
	}
	else if(*Result == 0x1001) //100M
	{
		
		Result++;

		
		if(*Result == 0x1000) //	printf("Half duplex\n");
		{

			type = 4;
		}
		else if(*Result == 0x1001)//	printf("Full duplex\n");
		{
		
			type = 3;
		}
	}
        msgQueryNetWorkCPERsp.setRspType( type);
	  if (true != msgQueryNetWorkCPERsp.Post())
        {
            OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post msg[0x%04x]  to task[%d] fail", msgQueryNetWorkCPERsp.GetMessageId(), msgQueryNetWorkCPERsp.GetSrcTid());
            msgQueryNetWorkCPERsp.DeleteMessage();
            return false;
         }      
	break;
	default:
		break;
	return true;
   };
}
 //wangwenhua add 20081211
 void CTaskCpeM:: CPE_Debug_Cfg_OK(UINT32 eid,UINT16 type)
 {
      CL3CpeCommCfgReq ReqMsg;
     UINT32 uMsgLen = 2;

    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_Debug_Cfg_OK:%08x!", eid);
	

	if (false == ReqMsg.CreateMessage(*this, uMsgLen))
        return;


    	ReqMsg.SetDstTid(M_TID_CPECM);
	ReqMsg.SetSrcTid(M_TID_UM);
	ReqMsg.SetEID(eid);
    	ReqMsg.SetCommMsgType( type);


    	if(true != ReqMsg.Post())
    	{
        	OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_Debug_Cfg fail");
        	ReqMsg.DeleteMessage();
    	}
 }
bool CTaskCpeM:: CPE_Debug_MSg(CMessage& rMsg)
{
   #if 0
    CL3CpeCommMsgReq RspMsg(rMsg);
    UINT16  type = RspMsg.GetCommMsgType();
    UINT16  len =RspMsg.GetCommMsgLen();
     char  *ptr = (char*)RspMsg.GetCommContent();
  OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%8x] CPE_Debug_MSg,len:%d,type:%d", rMsg.GetEID(),len,type);
    if(type == 0)//按str打印
    {
     
       OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%8x] CPE_Debug_MSg:%s\n", rMsg.GetEID(),(int)ptr);
      for(int i =0; i< len; i++)
        {
        	
		  OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "%c,",ptr[i]);
   
        }
    }
    else if(type == 1)//按16进制打印
    {
        for(int i =0; i< len; i++)
        {
        
		  OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "%x,",ptr[i]);
   
        }
    }
  #endif
   return true;
}
void  CPE_Debug_Cfg(UINT32 eid, UINT16 type)
{

     	if(eid==0)
    	{
		printf("CPE_Debug_Cfg Please input right  eid number\n");
		return;
    	}

    	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	
    	pTInstance->CPE_Debug_Cfg_OK(eid, type);
}
 //wangwenhua add end 20081211

 UINT32 CTaskCpeM::CPE_GetUIDByEID(UINT32 eid)
 {
        UINT32 uid = 0xffffffff;
	 CPECCB   *CCB = NULL;
	CCB =(CPECCB*)m_CpeFsm.FindCCB(eid);
	if(CCB!=NULL)
	{
	     uid = CCB->getUid();
		    
	}	
	return uid;
 }
 void CTaskCpeM::CPE_SendCommonMsg(UINT16 msgId, CMessage &rMsg)
{
	UINT32 eid;
	
	CComMessage* pComMsg = new (this, rMsg.GetDataLength()) CComMessage;
	if (pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_SendCommonMsg.");
		return;
	}

	eid = *(UINT32*)((UINT8*)rMsg.GetDataPtr()+2);
	pComMsg->SetDstTid(M_TID_CPECM);
	pComMsg->SetSrcTid(M_TID_UM);    
	pComMsg->SetEID(eid);
	pComMsg->SetMessageId(msgId);    	
	memcpy(((UINT8*)pComMsg->GetDataPtr()), rMsg.GetDataPtr(), rMsg.GetDataLength());
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
	return;
}
void CTaskCpeM::CPE_RcvCommonMsg(UINT16 msgId, CMessage &rMsg)
{
       if(*(UINT16*)((UINT8*)rMsg.GetDataPtr())==0xffff)//tranid, 是基站发送的
   	{
   	    if(msgId == M_BTS_EMS_MEM_SIGNAL_RPT_RSP)
   	    {
   	        printf("config Mem report signal is : %d\n", *(UINT16*)((UINT8*)rMsg.GetDataPtr()+2));
   	    }
	    else if(msgId == M_BTS_EMS_MEM_RUNTIME_RSP)
	    {
	        UINT32 runtime;
		 runtime = *(UINT32*)((UINT8*)rMsg.GetDataPtr()+4)/100;
	        printf("get mem runtime rsp, result:%d, time:%d\n", *(UINT16*)((UINT8*)rMsg.GetDataPtr()+2), runtime);
    	    }
	    return;
   	}
	//将发往的ip地址放在commessage最后
	CComMessage* pComMsg = new (this, rMsg.GetDataLength()) CComMessage;
	if (pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in CPE_RcvCommonMsg.");
		return;
	}
	pComMsg->SetDstTid(M_TID_EMSAGENTTX);
	pComMsg->SetSrcTid(M_TID_UM);    
	pComMsg->SetMessageId(msgId);    	
	memcpy(((UINT8*)pComMsg->GetDataPtr()), rMsg.GetDataPtr(), rMsg.GetDataLength());
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
	return;
}
void CPE_Reset_NetIf(UINT32 eid)
{
    	if(eid==0)
    	{
		printf("Please input eid number\n");
		return;
    	}

    	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	
    	pTInstance->CPE_NetIf_Cfg(eid, RST_CPE_NETIF, NULL);
}

void CPE_Read_NetIf_Reg(UINT32 eid)
{
    	if(eid==0)
    	{
		printf("Please input eid number\n");
		return;
    	}

    	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	
    	pTInstance->CPE_NetIf_Cfg(eid, READ_CPE_NETIF_REG, NULL);
}

void CPE_Read_8150_Reg(UINT32 eid, UINT16 addr)
{

    	if(eid==0)
    	{
		printf("Please input eid number!\n");
		return;
    	}


	if((addr<0x120)||(addr>0x14d))
	{
		printf("Wrong address!\n");
		return;
	}

	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	
    	pTInstance->CPE_NetIf_Cfg(eid, READ_CPE_RTL8150_REG, (UINT8 *)&addr);
	
}

void CPE_Read_NetIf_Status(UINT32 eid)
{
    	if(eid==0)
    	{
		printf("Please input eid number!\n");
		return;
    	}

    	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	
    	pTInstance->CPE_NetIf_Cfg(eid, READ_CPE_NETIF_STATUS, NULL);
}

void CPE_Write_8150_Reg(UINT32 eid, UINT16 addr, UINT16 value)
{
	UINT16 content[2] = {0};

    	if(eid==0)
    	{
		printf("Please input eid number!\n");
		return;
    	}


    	if((addr<0x120)||(addr>0x14d))
	{
		printf("Wrong address!\n");
		return;
	}

	content[0] = addr;
	content[1] = value;

    	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	
    	pTInstance->CPE_NetIf_Cfg(eid, WRITE_CPE_RTL8150_REG, (UINT8 *)content);
}

void CPE_Cfg_NetIf_Ada(UINT32 eid)
{
	UINT16 content = 0;

	if(eid==0)
    	{
		printf("Please input eid number!\n");
		return;
    	}

    	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	content = 0x1000;
    	pTInstance->CPE_NetIf_Cfg(eid, CFG_CPE_NETIF_ADA, (UINT8 *)(&content));
	pTInstance->Set_TempNetIfAdaStatus(CPE_NETIF_ADA_AUTO);
}

void CPE_Cfg_NetIf_NoneAda(UINT32 eid)
{
	UINT16 content = 0;

	if(eid==0)
    	{
		printf("Please input eid number!\n");
		return;
    	}

    	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	content = 0x1001;
    	pTInstance->CPE_NetIf_Cfg(eid, CFG_CPE_NETIF_ADA, (UINT8 *)(&content));
	pTInstance->Set_TempNetIfAdaStatus(CPE_NETIF_ADA_NONEAUTO);
}
/***********************************************
0-10M ,half duplex
1-10M ,full duplex
2-100M ,half duplex;
3-100M, full duplex
***********************************************/
void CPE_Cfg_NetIf_Speed_Duplex(UINT32 eid,UINT8 flag)
{
	UINT16 content[2] = {0};
	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();

	if(eid==0)
    	{
		printf("Please input eid number!\n");
		return;
    	}
#if 0
	if(pTInstance->Get_NetIfAdaStatus()!=CPE_NETIF_ADA_NONEAUTO)
	{
		printf("Can not configure the network interface, please make sure it is not in auto adapation status!");
		return;
	}
#endif
	if(flag == 0)
	{
	  content[0] = 0x1000;
	  content[1] = 0x1000;
	}
	else if(flag == 1)
	{
	  content[0] = 0x1000;
	  content[1] = 0x1001;
	}
	else if(flag == 2)
	{
	  content[0] = 0x1001;
	  content[1] = 0x1000;
	}
	else if(flag == 3)
	{
	  content[0] = 0x1001;
	  content[1] = 0x1001;
	}
	else
	{
		printf("flag err!");
		return;
	}
#if 0	
	while((content[0]!=10)||(content[0]!=100))
	{
		printf("Please input the speed of network interface(10 or 100):\n");
		cin >> content[0];
	}

	while((content[1]!=1)||(content[1]!=2))
	{
		printf("Please input the duplex mode of network interface(1:half duplex, 2:full duplex):\n");
		cin >> content[1];
	}
#endif
    	pTInstance->CPE_NetIf_Cfg(eid, CFG_CPE_NETIF_SPEED_DUPLEX, (UINT8 *)content);
	
}

void CPE_Read_NetIf_Speed_Duplex(UINT32 eid)
{
	if(eid==0)
    	{
		printf("Please input eid number!\n");
		return;
    	}

	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	pTInstance->CPE_NetIf_Cfg(eid, READ_CPE_NETIF_SPEED_DUPLEX, NULL);
}
void ResetCPE(UINT32 eid)
{
   	if(eid==0)
    	{
		printf("Please input eid number!\n");
		return;
    	}

	CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	pTInstance->ResetWanifCpe(eid);
}
UINT32   l3oamGetEIDByUID(UINT32 UID)//通过UID得到EID
{
   if((UID==0)||(UID==0xffffffff))
   {
         printf("invalid UID:%08x\n",UID);
   	    return 0xffffffff;
   }
   UINT32 eid32 = 0xffffffff;
     list<T_ToBeDeleteCpe>::iterator it;
#ifndef WBBU_CODE
    for(it = CTaskCpeM::m_ToBeDeleteCpeList.begin(); it != CTaskCpeM::m_ToBeDeleteCpeList.end(); ++it)
        {
        	const UINT32 eid = it->CPEID;
       	 const CPECCB *pCCB = (CPECCB*)CTaskCpeM::GetInstance()->m_CpeFsm.FindCCB(eid);
		if(pCCB!=NULL)
		{
	     		if((pCCB->getUid()) == UID)
	     		{
	     		   eid32 = eid;
	    		   break;
	     		}
		}
    	}
#else
	      CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
	    for(it = pTInstance->m_ToBeDeleteCpeList.begin(); it != pTInstance->m_ToBeDeleteCpeList.end(); ++it)
	        {
	        	const UINT32 eid = it->CPEID;
	       	 const CPECCB *pCCB = (CPECCB*)CTaskCpeM::GetInstance()->m_CpeFsm.FindCCB(eid);
			if(pCCB!=NULL)
			{
		     		if((pCCB->getUid()) == UID)
		     		{
		     		   eid32 = eid;
		    		   break;
		     		}
			}
	    	}

#endif
       return eid32;

}

UINT32  l3oamGetUIDByEID(UINT32 EID)//通过EID得到UID
{
      UINT32 UID = 0xffffffff;
      CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
     UID  =  pTInstance->CPE_GetUIDByEID(EID);
	return UID;
  
}


//lijinan 20081208  计费系统增加 
UINT32 CTaskCpeM::FindUidViaEid(UINT32 eid,UINT8 *adminStatus,UINT8 *ut_type,char*up_down_bw)
{
 
      CPECCB *pCCB = (CPECCB*)m_CpeFsm.FindCCB(eid);
      if(pCCB==NULL)
      {     *adminStatus = 0;
            *ut_type = 0;
            return 0xffffffff;
       }     

      T_UTProfile profile ;
  
      profile = pCCB->getUTProfile();
      if(profile.UTProfileIEBase.AdminStatus==0x10)
           *adminStatus = 1;
      else
           *adminStatus = 0;

      *ut_type =pCCB->getUTSDCfg().Class;
      memcpy(up_down_bw,(char*)&(pCCB->getUTSDCfg().ULMaxBW),8);
              
      return (pCCB->getUid());
}   

UINT32  findUidFromEid(UINT32 eid,UINT8 *adminStatus,UINT8 *ut_type,char* bw)
{
  CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
  return pTInstance->FindUidViaEid(eid,adminStatus,ut_type,bw);
}
bool CTaskCpeM::getWifiFlag(UINT32 eid)
{
 
      CPECCB *pCCB = (CPECCB*)m_CpeFsm.FindCCB(eid);
      if(pCCB==NULL)
      {    
            return false;
       }     

     if(pCCB->getUTSDCfg().Reserved&0x10)
        return true;
      return false;
}
bool  getWifiFlagByEid(UINT32 eid)
{
  CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
  return pTInstance->getWifiFlag(eid);
}
//lijinan 20081208  计费系统增加 
//flag:0close, 1open; length:second
void CPE_CfgSignalReport(UINT32 EID, UINT16 flag, UINT32 length)
{
    UINT8 data[12], *p;
    UINT16 tranid=0xffff;
    
    p = data;
    memcpy(p, (UINT8*)&tranid, 2);
    p+=2;
    memcpy(p, (UINT8*)&EID, 4);
    p+=4;
    memcpy(p, (UINT8*)&flag, 2);
    p++;
    memcpy(p, (UINT8*)&length, 4);
    CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
    pTInstance->RPT_SendComMsg(EID, M_TID_CPECM, M_L3_CPE_MEM_SIGNAL_RPT_CFG, data, 11);
     
}
void CPE_GetRuntime(UINT32 EID)
{
    UINT8 data[6], *p;
    UINT16 tranid=0xffff;

    p = data;
    memcpy(p, (UINT8*)&tranid, 2);
    p+=2;
    memcpy(p, (UINT8*)&EID, 4);
    CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
    pTInstance->RPT_SendComMsg(EID, M_TID_CPECM, M_L3_CPE_MEM_RUNTIME_REQ, data, 6);
}

bool CTaskCpeM::RPT_SendComMsg(UINT32 ulEID, TID tid, UINT16 usMsgID, UINT8* pd, UINT16 usLen )
{
	if( (M_BTS_RPT_RESET_REQ>=usMsgID&&M_BTS_RPT_RPTONOFF_REQ<=usMsgID) ||
	     (M_BTS_EMS_RPTALRM_NOTIFY>=usMsgID&&M_EMS_BTS_RPTONOFF_REQ<=usMsgID)  )
		OAM_LOGSTR2(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CpeMTrans: RPT[%08Xd]   send MsgID[][%04Xd]", ulEID,usMsgID);
    CComMessage *RspMsg = new ( this, usLen ) CComMessage;
    if( RspMsg != NULL )
    {
        RspMsg->SetDstTid( tid );
        RspMsg->SetMessageId( usMsgID );
		RspMsg->SetEID(ulEID);
        RspMsg->SetSrcTid( this->GetEntityId());
		if( 0 != usLen )
        	memcpy( (UINT8*)RspMsg->GetDataPtr(), pd, usLen );
        if( ! CComEntity :: PostEntityMessage( RspMsg ) )
        {
            RspMsg->Destroy();
			return false;
        }
    }
    return true;
}


void CTaskCpeM::CPE_SetDlFlag(UINT32 eid, UINT8 flag)
{
    CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
 

    CPECCB* pCPECCB = (CPECCB*)pTInstance->m_CpeFsm.FindCCB(eid);       
    if(pCPECCB)
    {
        pCPECCB->setUtDlFlag(flag);
    }    
   
}
#ifdef WBBU_CODE
void CPE_AddDeleteCpeInfo(UINT32 CpeID, UINT32 period)
{
    CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
    pTInstance->CPE_AddDeleteCpeInfo_function(CpeID,period);
}   
#endif
void deletecpe(UINT32 eid)
{
  CTaskCpeM *pTInstance = CTaskCpeM::GetInstance();
  pTInstance->CPE_DeleteCpeData(eid);
}
#ifdef WBBU_CODE
void Reset_CPE(UINT32 EID)
{

    	CComMessage *pComMsg = NULL;
    	unsigned char *p;
     	 pComMsg = new (CTaskCpeM::GetInstance(), 8) CComMessage;
     //  CCpeCommonReq ReqMsg;
   // if (false == ReqMsg.CreateMessage(*this))
    //    return false;
        //    UINT16  TransId;
      //  UINT16  Version; 
     //   UINT32  CPEID;
       if(pComMsg==NULL)
       	return;
    p =(unsigned char*) pComMsg->GetDataPtr();
    pComMsg->SetMessageId(M_L3_CPE_RESET_UT_REQ);
   pComMsg->SetDstTid(M_TID_CPECM);
   pComMsg->SetSrcTid(M_TID_UM);
   pComMsg->SetEID(EID);
   p[0] = 0x12;
   p[1] = 0x34;
   p[2] = 0x00;
   p[3] =0x01;
   p[4] =(unsigned char) (EID>>24);
   p[5] =(unsigned char)( EID>>16);
   p[6] = (unsigned char)(EID>>8);
   p[7] =(unsigned char ) EID;
  if(false==CComEntity::PostEntityMessage(pComMsg))
  	{
  	    return;
  	}
  else
  	{
  	    printf("Reset CPE:%x\n",EID);
  	}
}
#endif
void CTaskCpeM::CPE_LoadinfoReq(CMessage& rMsg)
{
	UINT32 *usp = (UINT32*)rMsg.GetDataPtr();
	UINT8 ucBtsCnt = rMsg.GetDataLength()/sizeof(UINT32);
	UINT16 usRst[1+NEIGHBOR_BTS_NUM];
	usRst[0] = gL3OamBTSLoadInfo.WeightedUserNo;
	for( UINT8 uc=0; uc<ucBtsCnt; uc++ )
		usRst[uc+1] = GetWeightedUserNoByBtsID( *(usp+uc) );
	RPT_SendComMsg(rMsg.GetEID(), M_TID_CPECM, M_BTS_CPE_LOADINFO_RSP, (UINT8*)usRst, 2+ucBtsCnt*sizeof(UINT16) );
}
UINT16 CTaskCpeM::GetWeightedUserNoByBtsID(UINT32 ulBtsID)
{
	for( UINT16 us=0; us<m_NeighborBTSLoadInfoEle.NeighborBTSNum; us++ )
		if( m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[us].BTSID == ulBtsID )
			return m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[us].BTSLoadInfo.WeightedUserNo;
	return 0xFFFF;
}
void CTaskCpeM::SetWeightedUserNoByBtsID(UINT32 ulBtsID, UINT16  usdata)
{
	for( UINT16 us=0; us<m_NeighborBTSLoadInfoEle.NeighborBTSNum; us++ )
		if( m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[us].BTSID == ulBtsID )
		{
			m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[us].BTSLoadInfo.WeightedUserNo = usdata;
			return;
		}
}

NBtsPayload CTaskCpeM::GetNBtsPayloadSupport(UINT32 ulBtsID)
{
	for( UINT16 us=0; us<m_NeighborBTSLoadInfoEle.NeighborBTSNum; us++ )
		if( m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[us].BTSID == ulBtsID )
			return m_ucNBtsRcdForPld[us];
	return PAYLOAD_UNKNOW;	
}

#include "L3OamCfg.h"
void pbt( UINT8 uc, UINT8 uc1 )
{
    CTaskCpeM::GetInstance()->plbPrint(uc, uc1);
}
void CTaskCpeM::plbPrint(UINT8 uch, UINT8 uc1 )
{
    UINT8 uc;
    T_PayloadBalanceCfg2nd stPayloadReq;
    CTaskCfg *pTaskCfgM = CTaskCfg::GetInstance();
    struct
    {
        UINT16 transid;
        T_CLUSTER_PARA stele;
    }st;
    memset( (UINT8*)&st, 0xff, sizeof(st) );
	switch (uch)
	{
		case 1:
			for( UINT16 us=0; us<m_NeighborBTSLoadInfoEle.NeighborBTSNum; us++ )
			{
				if( PAYLOAD_UNKNOW == m_ucNBtsRcdForPld[us] )
					printf( "BtsID[%08X] PayloadSupport[PAYLOAD_UNKNOW]\r\n", m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[us].BTSID);
				if( PAYLOAD_A== m_ucNBtsRcdForPld[us] )
					printf( "BtsID[%08X] PayloadSupport[PAYLOAD_A]\r\n", m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[us].BTSID);
				if( PAYLOAD_NA== m_ucNBtsRcdForPld[us] )
					printf( "BtsID[%08X] PayloadSupport[PAYLOAD_NA]\r\n", m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[us].BTSID);
			}
			break;
		case 2:
			printf( "gL3OamBTSLoadInfo.CurrentUserNumber[%04X]\r\n", gL3OamBTSLoadInfo.CurrentUserNumber );
			printf( "gL3OamBTSLoadInfo.AverageFreeDLchannel[%04X]\r\n", gL3OamBTSLoadInfo.AverageFreeDLchannel);
			printf( "gL3OamBTSLoadInfo.AverageFreeULchannel[%04X]\r\n", gL3OamBTSLoadInfo.AverageFreeULchannel);
			printf( "gL3OamBTSLoadInfo.AverageFreePowerUsage[%04X]\r\n", gL3OamBTSLoadInfo.AverageFreePowerUsage);
			printf( "gL3OamBTSLoadInfo.WeightedUserNo[%04X]\r\n", gL3OamBTSLoadInfo.WeightedUserNo);
			printf( "gL3OamBTSLoadInfo.Reserve[%08X]\r\n", gL3OamBTSLoadInfo.Reserve);
			break;
        case 3:
			printf( "*********show m_NeighborBTSLoadInfoEle record Total[%04d]\r\n", m_NeighborBTSLoadInfoEle.NeighborBTSNum );
            for( uc=0; uc<m_NeighborBTSLoadInfoEle.NeighborBTSNum; uc++ )
            {
    			printf( "******%02d: BTSID[%08X]  WeightedUserNo[%02d]\r\n", uc, m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[uc].BTSID, 
                          m_NeighborBTSLoadInfoEle.NeighbotBTSLoadInfo[uc].BTSLoadInfo.WeightedUserNo );
            }
			break;
        case 7:
            if( uc1>4 || uc1<1 )
            {
    			printf( "the video para should be [1-4]\r\n");
                break;
            }
            st.stele.rsv1 = uc1;
			RPT_SendComMsg( 0, M_TID_CM, M_EMS_BTS_CLUSTER_PARA_CFG, (UINT8*)&st, sizeof(st) );
            break;
		case 8:
            stPayloadReq.usFlag        = pTaskCfgM->m_stPldBlnCfg.usFlag      ;
            stPayloadReq.usLi          = pTaskCfgM->m_stPldBlnCfg.usLi        ;
            stPayloadReq.usPeriod      = pTaskCfgM->m_stPldBlnCfg.usPeriod    ;
            stPayloadReq.usLd          = pTaskCfgM->m_stPldBlnCfg.usLd        ;
            stPayloadReq.usCount       = pTaskCfgM->m_stPldBlnCfg.usCount     ;
            stPayloadReq.usSignal      = pTaskCfgM->m_stPldBlnCfg.usSignal    ;
            stPayloadReq.usLdPeriod    = pTaskCfgM->m_stPldBlnCfg.usLdPeriod  ;
            stPayloadReq.usParam       = pTaskCfgM->m_stPldBlnCfg.usParam     ;
            stPayloadReq.usBandSwitch  = 0;
			RPT_SendComMsg( 0, M_TID_CM, M_EMS_BTS_PAYLOAD_BALANCE_CFG_REQ, (UINT8*)&stPayloadReq, sizeof(T_PayloadBalanceCfg2nd) );
			break;
		case 9:
            stPayloadReq.usFlag        = pTaskCfgM->m_stPldBlnCfg.usFlag      ;
            stPayloadReq.usLi          = pTaskCfgM->m_stPldBlnCfg.usLi        ;
            stPayloadReq.usPeriod      = pTaskCfgM->m_stPldBlnCfg.usPeriod    ;
            stPayloadReq.usLd          = pTaskCfgM->m_stPldBlnCfg.usLd        ;
            stPayloadReq.usCount       = pTaskCfgM->m_stPldBlnCfg.usCount     ;
            stPayloadReq.usSignal      = pTaskCfgM->m_stPldBlnCfg.usSignal    ;
            stPayloadReq.usLdPeriod    = pTaskCfgM->m_stPldBlnCfg.usLdPeriod  ;
            stPayloadReq.usParam       = pTaskCfgM->m_stPldBlnCfg.usParam     ;
            stPayloadReq.usBandSwitch  = 1;
			RPT_SendComMsg( 0, M_TID_CM, M_EMS_BTS_PAYLOAD_BALANCE_CFG_REQ, (UINT8*)&stPayloadReq, sizeof(T_PayloadBalanceCfg2nd) );
			break;
        default:
			printf( "1: show neighbour bts payload_balance N/A\r\n");
			printf( "2: show payload info of local bts \r\n");
			printf( "3: show neighbour bts WeightedUserNo\r\n");
			printf( "7: set video para [1-4]\r\n");
			printf( "8: set band-keeping true\r\n");
			printf( "9: set band_keeping false\r\n");
			printf( "gL3OamBTSLoadInfo.Reserve[%08X]\r\n");
			break;
	}
}
#ifdef LOCATION_2ND
///////////////////////////////////////////////////////////////////////
/***********************哈希表处理******************************/
void    HashInit(
				 hashtab_t *H,
				 unsigned int nbuckets,
				 unsigned long (*hash)(const void *),
				 int (*compare)(const void *, const void *),
				 const void *(*keyof)(const hash_node_t *),
				 hash_node_t **table
				 )
{
	unsigned int    i;

	H->count = 0;
	H->nbuckets = nbuckets;
	H->hash = hash;
	H->keyof = keyof;
	H->compare = compare;
	H->table = table;

	for( i=0; i<nbuckets; i++ )
	{
		H->table[i] = 0;
	}
}
void    HashInsert(hashtab_t *H, hash_node_t *N)
{
	unsigned int    index;

	index = (H->hash(H->keyof(N))) % (H->nbuckets);
	N->link = H->table[index];
	H->table[index] = N;
	H->count++;
}

void    HashDelete(hashtab_t *H, hash_node_t *N)
{
	unsigned int    index;
	hash_node_t *p;

	index = (H->hash(H->keyof(N))) % (H->nbuckets);
	p = H->table[index];

	if( p == N )
	{
		H->table[index] = N->link;
		N->link = 0;
		H->count--;
	}
	else
	{
		while( p && (p->link != N) )
		{
			p = p->link;
		}
		if( !p )
		{
			return;
		}
		p->link = N->link;
		N->link = 0;
		H->count--;
	}
}

unsigned int    HashCount(const hashtab_t *H)
{
	return H->count;
}

hash_node_t *HashFind(const hashtab_t *H, const void *item)
{
	unsigned int    index;
	hash_node_t *p;

	index = (H->hash(item)) % (H->nbuckets);
	p = H->table[index];

	while( p && H->compare( item, H->keyof(p) ) )
	{
		p = p->link;
	}

	return p;
}

hash_node_t *HashFirst( const hashtab_t *H )
{
	unsigned int    index;

	for( index=0; index<H->nbuckets; index++ )
	{
		if( H->table[index] )
		{
			return H->table[index];
		}
	}
	return 0;
}

hash_node_t *HashNext( const hashtab_t *H, const hash_node_t *N )
{
	unsigned int    index;

	if( N->link )
	{
		return N->link;
	}

	index = (H->hash(H->keyof(N))) % (H->nbuckets);
	index++;
	for( ; index<H->nbuckets; index++ )
	{
		if( H->table[index] )
		{
			return H->table[index];
		}
	}
	return 0;
}

static unsigned long location_record_t_hash(const void *v1)
{
	short i,j,*q=(short *)v1;
	j=0;
	for(i=0;i<2;i++)
		j+=q[i];
	return j%97;
}

static int location_record_t_compare(const void *v1, const void *v2)
{
	if( (*(UINT32*)v1) > (*(UINT32*)v2) ) 
		return 1;
	if( (*(UINT32*)v1) < (*(UINT32*)v2) ) 
		return -1;
	if( (*(UINT32*)v1) == (*(UINT32*)v2) ) 
		return 0;
}

static const void * location_record_t_keyof(const struct hash_node_t *v1)
{
	stLocationRcd *p=(stLocationRcd *)v1;
	return &p->ulEID;    
}

T_LOCATION_RECORD::T_LOCATION_RECORD()
{
    bGps = false;
    bDoa =false;
    bTmOutMsgFlag = false;
    pLctTimer = NULL;
    pComMsg = NULL;
//    bCreateFlag = true;
}
T_LOCATION_RECORD::~T_LOCATION_RECORD()
{
}

T_LocationRecordTable::T_LocationRecordTable()
{
	HashInit(&htab,LOCATION_RECORD_ENTRY_NUM,location_record_t_hash,location_record_t_compare,location_record_t_keyof,hentry);
}

bool T_LocationRecordTable::Insert(stLocationRcd *prec)
{
	stLocationRcd *p1;
	p1 = Find( (UINT8*)&prec->ulEID );
	if(p1)
	{
		return false;
	}

	p1 = new stLocationRcd;
	*p1 = *prec;

	HashInsert(&htab,(hash_node_t *)p1);
	return true;
}

stLocationRcd *T_LocationRecordTable::Find(const UINT8 *addr)
{
	stLocationRcd *p;
	p=(stLocationRcd *)HashFind(&htab,addr);
	return p;
}


bool T_LocationRecordTable::Delete(const UINT8 *addr)
{
	stLocationRcd *p;
	p = (stLocationRcd *)HashFind(&htab,addr);

	if(p)
	{
		HashDelete(&htab,(hash_node_t *)p);
		delete p;
		return true;

	}
	else
		return false;
}

stLocationRcd * T_LocationRecordTable::First()
{
	stLocationRcd *p;
	p = (stLocationRcd *)HashFirst(&htab);

	return p;
}


stLocationRcd * T_LocationRecordTable::Next(stLocationRcd *p)
{
	p = (stLocationRcd *)HashNext(&htab,(hash_node_t *)p);

	return p;
}
void CTaskCpeM::CPE_Location_EmsReq(CMessage& rMsg)
{
	T_EmsReq* tpReq = (T_EmsReq*)rMsg.GetDataPtr();
    BOOL bCreateFlag = true;

    stLocationRcd ele;
    memset( (UINT8*)&ele, 0, sizeof(ele) );
    ele.ulEID = tpReq->eid;
    ele.usTransID = tpReq->transId;
    ele.bTmOutMsgFlag = false;
    if( sizeof(T_EmsReq) == rMsg.GetDataLength() )
        ele.bGpsReq = true;
    else 
    {
        T_EmsReqNew* pst = (T_EmsReqNew*)rMsg.GetDataPtr();
        if( 0 == pst->doaLen )
            ele.bGpsReq = true;
        else
            ele.bGpsReq = false;
    }
    
    ele.pComMsg = new ( this, 200 ) CComMessage;
    if( ele.pComMsg != NULL )
    {
    	ele.pComMsg->SetDstTid( M_TID_EMSAGENTTX);
    	ele.pComMsg->SetMessageId( M_BTS_EMS_LOCATION_RSP );
    	ele.pComMsg->SetEID( tpReq->eid );
    	ele.pComMsg->SetSrcTid( this->GetEntityId() );
    }
    else
    {
        T_EmsRsp rsp;
        rsp.btsid = m_ulBTSID;
        rsp.transId = tpReq->transId;
        rsp.result = LCT_BTS_FAIL;
        rsp.gpsLen = 0;
        RPT_SendComMsg( tpReq->eid, M_TID_EMSAGENTTX, M_BTS_EMS_LOCATION_RSP, (UINT8*)&(rsp), sizeof(rsp) );
    	OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE new commessage failed!!!" );
        return;
   }

    CComMessage* pTimerMsg = new ( this, sizeof(T_EmsReq) ) CComMessage;    
    if (pTimerMsg!=NULL)
    {
    	pTimerMsg->SetDstTid( M_TID_UM  );
    	pTimerMsg->SetSrcTid( M_TID_UM  );
    	pTimerMsg->SetMessageId( MSGID_LOCATION_TIMER_OUT );
    	pTimerMsg->SetEID( tpReq->eid );
        memcpy( (UINT8*)pTimerMsg->GetDataPtr(), (UINT8*)tpReq, sizeof(T_EmsReq) );
    }
    else
    {
        T_EmsRsp rsp;
        rsp.btsid = m_ulBTSID;
        rsp.transId = tpReq->transId;
        rsp.result = LCT_BTS_FAIL;
        rsp.gpsLen = 0;
        RPT_SendComMsg( tpReq->eid, M_TID_EMSAGENTTX, M_BTS_EMS_LOCATION_RSP, (UINT8*)&(rsp), sizeof(rsp) );
    	OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE new timer commessage failed!!!" );
        return;
    }
    ele.pLctTimer = new CTimer( false, M_LOCATION_TIMER_LENGTH, pTimerMsg );
    if( NULL == ele.pLctTimer )
    {
        T_EmsRsp rsp;
        rsp.btsid = m_ulBTSID;
        rsp.transId = tpReq->transId;
        rsp.result = LCT_BTS_FAIL;
        rsp.gpsLen = 0;
        RPT_SendComMsg( tpReq->eid, M_TID_EMSAGENTTX, M_BTS_EMS_LOCATION_RSP, (UINT8*)&(rsp), sizeof(rsp) );
    	OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE new timer failed!!!" );
        pTimerMsg->Destroy();
        ele.pComMsg->Destroy();
        return;
    }    
    else
        ele.pLctTimer->Start();
  
    if( !m_stLocationRcdTbl.Insert( (stLocationRcd*)&ele ) )
    {
        T_EmsRsp rsp;
        rsp.btsid = m_ulBTSID;
        rsp.transId = tpReq->transId;
        rsp.result = LCT_BTS_FAIL;
        rsp.gpsLen = 0;
        RPT_SendComMsg( tpReq->eid, M_TID_EMSAGENTTX, M_BTS_EMS_LOCATION_RSP, (UINT8*)&(rsp), sizeof(rsp) );
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE eid[%08x] is in the hash table", ele.ulEID );
        ele.pComMsg->Destroy();
        ele.pLctTimer->Stop();
        delete ele.pLctTimer;
        return;
    }
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Get EMS location req, transid[%04X] eid[%08X]", tpReq->transId,tpReq->eid);
    if( false == ele.bGpsReq )
    {
        tpReq->rcv = 0;//通知2层是否需要gps数据
    }
    else
    {
        tpReq->rcv = 1;
        RPT_SendComMsg( tpReq->eid, M_TID_CPECM, M_BTS_CPE_LOCATION_REQ, (UINT8*)rMsg.GetDataPtr(), sizeof(UINT16) );
    }
	RPT_SendComMsg( tpReq->eid, M_TID_L2MAIN, M_L3_L2_LOCATION_DOA_REQ, (UINT8*)rMsg.GetDataPtr(), sizeof(T_EmsReq) );
    return;
}

void CTaskCpeM::CPE_Location_GpsRsp(CMessage& rMsg)
{
    UINT32 eid = rMsg.GetEID();
    //printf( "\neid[%08X]", eid );return;
    T_EmsRsp* pstEmsRsp;
    stLocationRcd* pRcd = m_stLocationRcdTbl.Find( (UINT8*)&eid );
	T_CpeRsp* pCpeRsp = (T_CpeRsp*)rMsg.GetDataPtr();
    if( pRcd )
    {
        if( pRcd->usTransID != pCpeRsp->transId )
        {
            OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE gps transid fail rcd[%04X] gps[%04X]", pRcd->usTransID, pCpeRsp->transId);
            return;
        }
        if( ! pRcd->bGpsReq )
        {
            OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE gps isnt request rcd[%04X] gps[%04X]", pRcd->usTransID, pCpeRsp->transId);
            return;
        }
        pRcd->bGps = true;
        pstEmsRsp = (T_EmsRsp*)(pRcd->pComMsg->GetDataPtr());
        pRcd->ucGpsLen = pCpeRsp->gpsLen;
        memcpy( (UINT8*)&pstEmsRsp->gpsLen, (UINT8*)&pCpeRsp->gpsLen, pCpeRsp->gpsLen + 2 );
    }
    else
    {
        OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE didnt find the gps-eid in hash" );
        return;
    }
	OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE get gps transid[%04X] length[%04X]", pCpeRsp->transId,pCpeRsp->gpsLen);

    if( pRcd->bDoa )
    {
        CPE_Location_CreateRsp( pRcd );
    }
    return;
}

void CTaskCpeM::CPE_Location_DoaRsp(CMessage& rMsg)
{
    UINT8* pstEmsRsp;
    UINT32 eid;
	T_L2Rsp* pL2Rsp = (T_L2Rsp*)rMsg.GetDataPtr();
    stLocationRcd* pRcd = m_stLocationRcdTbl.Find( (UINT8*)&pL2Rsp->eid );
    OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE get doa eid[%08X] transid[%04X] length[%02d]", pL2Rsp->eid, pL2Rsp->transId, pL2Rsp->doaLen);
    if( pRcd )
    {
        if( pRcd->usTransID != pL2Rsp->transId )
        {
            OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE doa transid fail eid[%08X] transid[%04X] gps[%04X]", pRcd->ulEID, pRcd->usTransID, pL2Rsp->transId);
            return;
        }
        
        pRcd->bDoa = true;
        pstEmsRsp = (UINT8*)(pRcd->pComMsg->GetDataPtr()) + LOCATION_GPS_MAX;
		if( pL2Rsp->doaLen > (LOCATION_GPS_MAX-2/*sizeof(pL2Rsp->doaLen)*/) )
		{
            OAM_LOGSTR(LOG_SEVERE/*LOG_DEBUG3*/, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE doa data > 100" );
			return;
		}
        pRcd->ucDoaLen = pL2Rsp->doaLen;
        memcpy( pstEmsRsp, (UINT8*)&pL2Rsp->doaLen, sizeof(pL2Rsp->doaLen) + pL2Rsp->doaLen );
    }
    else
    {
        OAM_LOGSTR(LOG_SEVERE/*LOG_DEBUG3*/, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE didnt find the doa-eid in hash" );
        return;
    }

    
    if( ! pRcd->bGpsReq )
    {
        CPE_Location_CreateRsp( pRcd );
    }
    else if( pRcd->bGps )
    {
        CPE_Location_CreateRsp( pRcd );
    }
    return;
}
void CTaskCpeM::CPE_Location_CreateRsp( stLocationRcd* pRcd, BOOL bTmCall  )
{
    UINT8* puc = (UINT8*)pRcd->pComMsg->GetDataPtr();
    UINT8* pucDst;
    UINT8 ucLength;
    T_EmsRsp* pHd = (T_EmsRsp*)puc;
    pHd->transId = pRcd->usTransID;
    pHd->result = LCT_OK;
    pHd->btsid = m_ulBTSID;
    pHd->uid = pRcd->ulEID;
    if( pRcd->bGpsReq )
    {
        pucDst = puc + sizeof(T_EmsRsp) + pHd->gpsLen;
        ucLength = sizeof(T_EmsRsp) + pHd->gpsLen;
    }
    else
    {
        pHd->gpsLen = 0;
        pucDst = puc + sizeof(T_EmsRsp);
        ucLength = sizeof(T_EmsRsp);
    }
    memcpy( m_ucLocationSwitch, puc+LOCATION_GPS_MAX, 2 + pRcd->ucDoaLen );
    memcpy( pucDst, m_ucLocationSwitch, 2 + pRcd->ucDoaLen );
    ucLength += 2;
    ucLength += pRcd->ucDoaLen;
    pRcd->pComMsg->SetDataLength( ucLength );
    if( ! CComEntity :: PostEntityMessage( pRcd->pComMsg ) )
    {
        OAM_LOGSTR(LOG_SEVERE/*LOG_DEBUG3*/, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE send msg failed!!!" );
        pRcd->pComMsg->Destroy();
    }
    else 
    {
        if( bTmCall )
        {
            pRcd->bTmOutMsgFlag = true;
        }
    }

    if( !bTmCall )
    {
        pRcd->pLctTimer->Stop();
        delete pRcd->pLctTimer;

        if( m_stLocationRcdTbl.Delete((UINT8*)&pRcd->ulEID) )
        {
            OAM_LOGSTR1(LOG_SEVERE/*LOG_DEBUG3*/, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE delete eid[%08x] OK!!!", pRcd->ulEID );
        }
        else
        {
            OAM_LOGSTR1(LOG_SEVERE/*LOG_DEBUG3*/, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE delete eid[%08x] failed!!!", pRcd->ulEID );
        }
    }
    return;
}

void CTaskCpeM::CPE_Location_CreateTmRsp( stLocationRcd* pRcd )
{
    UINT8* puc = (UINT8*)pRcd->pComMsg->GetDataPtr();
    UINT8* pucDst;
    UINT8 ucLength;
    T_EmsRsp* pHd = (T_EmsRsp*)puc;
    pHd->transId = pRcd->usTransID;
    pHd->result = LCT_OK;
    pHd->btsid = m_ulBTSID;
    pHd->uid = pRcd->ulEID;

    OAM_LOGSTR3( LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, 
        "[tCpeM]  CreateTmRsp bGpsReq[%d], ucGpsLen[%d], ucDoaLen[%d]", 
        (pRcd->bGpsReq)?1:0, pRcd->ucDoaLen, pRcd->ucGpsLen );
    
    if( pRcd->bGpsReq )
    {
        OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]  CPE_Location_CreateTmRsp 0" );
        if( 0==pRcd->ucDoaLen && 0==pRcd->ucGpsLen )
        {
            OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]  CPE_Location_CreateTmRsp 1" );
            return;
        }
        if( 0==pRcd->ucGpsLen )
        {
            pRcd->bGpsReq = false;
            CPE_Location_CreateRsp(pRcd, true );
             OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]  CPE_Location_CreateTmRsp 2" );
            return;
        }
        OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]  CPE_Location_CreateTmRsp 3" );
        pRcd->pComMsg->SetDataLength( sizeof(T_EmsRsp)+pRcd->ucGpsLen );
        if( ! CComEntity :: PostEntityMessage( pRcd->pComMsg ) )
        {
            OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE TmRsp send msg failed!!!" );
        }   
        else
        {
            pRcd->bTmOutMsgFlag = true;
        }
        return;
    }
}

void CTaskCpeM::CPE_Location_TmOut(CMessage& rMsg)
{
    T_EmsReq* pReq = (T_EmsReq*)rMsg.GetDataPtr();
    stLocationRcd* prcd = m_stLocationRcdTbl.Find( (UINT8*)&pReq->eid );
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE CPE_Location_TmOut prcd[%08x] eid[%08x] OK!!!", (UINT32)prcd, prcd->ulEID );
    if( prcd )
    {
        if( pReq->transId != prcd->usTransID )
            return;
        CPE_Location_CreateTmRsp(prcd);
        if( ! prcd->bTmOutMsgFlag )
            prcd->pComMsg->Destroy();
        delete prcd->pLctTimer;
        if( m_stLocationRcdTbl.Delete((UINT8*)&pReq->eid) )
        {
            OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE delete eid[%08x] OK!!!", prcd->ulEID );
        }
        else
        {
            OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] LOCATIONE delete eid[%08x] failed!!!", prcd->ulEID );
        }
    }
    return;
}
void lcttest( UINT8 uc1, UINT32 eid, UINT8 uc2 )
{
    CTaskCpeM::GetInstance()->CPE_Location_test( uc1, eid, uc2 );
}
void CTaskCpeM::CPE_Location_test( UINT8 uc1, UINT32 eid, UINT8 uc2 )
{
    T_EmsReq st;
    T_EmsReqNew stn;
    UINT16 aus[44];
    T_L2Rsp* pl2st = (T_L2Rsp*)aus;
    T_CpeRsp* pgpsst = (T_CpeRsp*)aus;
    switch (uc1)
    {
    case 1:
        CPE_Location_HashShow();
        break;
    case 6:
        if( 0 == eid )
            st.eid = 0xabcdef12;
        else
            st.eid = eid;
        st.transId = 0x5678;
        RPT_SendComMsg( st.eid, M_TID_UM, M_EMS_BTS_LOCATION_REQ, (UINT8*)&st, sizeof(st) );
        break;
    case 7:
        if( 0 == eid )
            stn.eid = 0xabcdef12;
        else
            stn.eid = eid;
        stn.transId = 0x5678;
        stn.doaLen = uc2;
        RPT_SendComMsg( st.eid, M_TID_UM, M_EMS_BTS_LOCATION_REQ, (UINT8*)&stn, sizeof(stn) );
        break;
    case 8:
        memset( (UINT8*)aus, 0xaa, 88 );
        pl2st->transId = 0x5678;
        pl2st->doaLen = 76;
        if( 0 == eid )
            pl2st->eid= 0xabcdef12;
        else
            pl2st->eid = eid;
        RPT_SendComMsg( pl2st->eid, M_TID_UM, M_L2_L3_LOCATION_DOA_RSP, (UINT8*)aus, 88 );
        break;
    case 9:
        memset( (UINT8*)aus, 0xff, 86 );
        pgpsst->transId = 0x5678;
        pgpsst->gpsLen = 82;
        RPT_SendComMsg( (0==eid)?0xabcdef12:eid, M_TID_UM, M_CPE_BTS_LOCATION_RSP, (UINT8*)aus, 86 );
        break;
    default:
        printf( "\n1: print the location hash table" );
        printf( "\n6: send old location request[6,eid] " );
        printf( "\n7: send new location request[7,eid,0|1] " );
        printf( "\n8: send doa response request[8,eid] " );
        printf( "\n9: send gps response request[9,eid] " );
    }
}
void CTaskCpeM::CPE_Location_HashShow()
{
    UINT8 uc;
    stLocationRcd* pst;
	printf( "\n**********LOCATION HASH TABLE**********" );
	printf( "\nnum  eid       transid  gpsreq  gpsok  gpslen  doaok  doalen  pComMsg" );
	for(pst = m_stLocationRcdTbl.First(), uc=0; pst; pst=m_stLocationRcdTbl.Next(pst),uc++ )
	{
		printf( "\n%-5d%-10X%-9X%-8d%-7d%-8d%-7d%-8d%-8X", 
          uc+1, pst->ulEID, pst->usTransID, pst->bGpsReq, pst->bGps, pst->ucGpsLen, pst->bDoa, pst->ucDoaLen, (UINT32)pst->pComMsg );
	}	
    return;
}

#endif//LOCATION_2ND
//#ifdef LJF_RPT_ALTER_2_NVRAMLIST
void CTaskCpeM::CPE_RptListReq(CMessage& rMsg)
{
	UINT32* pulNvList = NvRamDataAddr->ulRptList;

	UINT32 pulReqList[10];
	T_EmsRptListReq* pstReq = (T_EmsRptListReq*)rMsg.GetDataPtr();
	memset( (UINT8*)pulReqList, 0, sizeof(UINT32)*M_RPT_LIST_MAX );
	memcpy( (UINT8*)pulReqList, (UINT8*)pstReq->ulPID, sizeof(UINT32)*pstReq->usCnt );
	//UINT32* pulReqList = ( (T_EmsRptListReq*)rMsg.GetDataPtr() )->ulPID;
	INT32 lEqual = memcmp( (UINT8*)pulNvList, (UINT8*)pulReqList, M_RPT_LIST_MAX*sizeof(UINT32) );
	memset( ((UINT8*)rMsg.GetDataPtr())+2, 0, 2 );
	if( (0==lEqual) /*&& (M_RPT_LIST_FLAG==NvRamDataAddr->ulRptListFlag)*/ )
	{
		OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_RptListReq: nvram == req" );
		RPT_SendComMsg( 0, M_TID_EMSAGENTTX, M_BTS_EMS_RPT_LIST_RSP, (UINT8*)rMsg.GetDataPtr(), 4 );
		return;
	}

	for( UINT8 uc=0; uc<M_RPT_LIST_MAX; uc++ )
	{
		UINT8 ucarr[4];
		memset( ucarr, 0, 4 );
		if( 0 != pulNvList[uc] )
		{
			RPT_SendComMsg( pulNvList[uc], M_TID_CPECM, M_L3_BTS_CPE_FORCE_REGISTER, ucarr, 4 );
			OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] M_L3_BTS_CPE_FORCE_REGISTER[%08X]", pulNvList[uc] );
		}
	}
	CTaskCfg::GetInstance()->l3oambspNvRamWrite( (char*)NvRamDataAddr->ulRptList, (char*)pulReqList, M_RPT_LIST_MAX*sizeof(UINT32) );
	OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE_RptListReq write rpt list to nvram" );
	RPT_SendComMsg( 0, M_TID_EMSAGENTTX, M_BTS_EMS_RPT_LIST_RSP, (UINT8*)rMsg.GetDataPtr(), 4 );
	return;
}

void showRptList()
{
	CTaskCpeM::GetInstance()->CPE_ShowRprList();
	return;
}
void CTaskCpeM::CPE_ShowRprList()
{
	UINT32* pulNvList = NvRamDataAddr->ulRptList;
	printf( "\r\n***   ***   RPT NVRAM LIST   ***   ***");
	printf( "\r\n[%08x] [%08x] [%08x] [%08x] [%08x]", *pulNvList, *(pulNvList+1), *(pulNvList+2), *(pulNvList+3), *(pulNvList+4) );
	printf( "\r\n[%08x] [%08x] [%08x] [%08x] [%08x]", *(pulNvList+5), *(pulNvList+6), *(pulNvList+7), *(pulNvList+8), *(pulNvList+9) );
	printf( "\r\n***   ***   RPT NVRAM LIST   ***   ***");
	return;
}
/*
*如果当前sag链路正常，查询ccb表，如果有终端是在故障弱化时上来的，则要求其重新注册。每次最多查询100项，
*如果找到2个终端，也停止
*/
UINT32 g_check_sag_default_eid = 0xfffffffe;
UINT16 g_position = 0;
void CTaskCpeM::CPE_Check_Sag_Default_Proc()
{    
	list<T_ToBeDeleteCpe>::iterator it;
    
    UINT8 count = 0;
    UINT8 send_user_count = 0;
    
    if(sagStatusFlag == true)
    {        
        if(g_check_sag_default_eid == 0xfffffffe)//第一次超时
        {
            m_check_SAG_Default_saved = m_ToBeDeleteCpeList.begin();
        }
        else if(g_position>m_ToBeDeleteCpeList.size())//记录的位置已经不存在了
        {
            m_check_SAG_Default_saved = m_ToBeDeleteCpeList.begin();
        }   
        else if(m_check_SAG_Default_saved->CPEID!=g_check_sag_default_eid)//eid不对
        {
            m_check_SAG_Default_saved = m_ToBeDeleteCpeList.begin();
        }
        if(m_ToBeDeleteCpeList.size()>0)
        {
           for(it = m_check_SAG_Default_saved; it != m_ToBeDeleteCpeList.end(); ++it)
           {
               const UINT32 eid = it->CPEID;
               const CPECCB *pCCB = (CPECCB*)m_CpeFsm.FindCCB(eid);                
               
               if (NULL != pCCB)
               {
                    if(pCCB->getSagDefaultFlag()==1)
                    {
                        send_user_count++;
                        notifyOneCpeToRegister(eid,true);
                    }
               }
               count++;
               if((count>50)||(send_user_count>2))//每次只查50个,或者找到2个符合条件的终端
               {
                    it++;
                    if(it != m_ToBeDeleteCpeList.end())//记录下一个位置
                    {
                        g_position += count;
                        g_check_sag_default_eid = it->CPEID;
                        m_check_SAG_Default_saved = it;
                    }
                    else//再从头开始
                    {
                        g_position = 0;
                        m_check_SAG_Default_saved = m_ToBeDeleteCpeList.begin();
                        g_check_sag_default_eid = m_check_SAG_Default_saved->CPEID;
                    }
                    return;
                    
               }
           }
        }
    }
}

