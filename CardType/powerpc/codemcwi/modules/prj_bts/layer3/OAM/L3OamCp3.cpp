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
 * ----------  ----------  ------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3CPEMESSAGEID
#include "L3CPEMessageId.h"
#endif

#ifndef _INC_BIZTASK
#include "BizTask.h"
#endif

#ifndef _INC_OAML3CPEM
#include "L3OamCpeM.h"
#endif

#ifndef _INC_TRANSACTION
#include "Transaction.h"
#endif

#ifndef _INC_L3OAMCPESTRANS
#include "L3OamCpeTrans.h"
#endif

#ifndef _INC_L3OAMCPEFSM
#include "L3OamCpeFSM.h"
#endif

#ifndef _INC_L3L3L2OAMCPEREGNOTIFY
#include "L3L3L2CpeRegNotify.h"
#endif

#ifndef _INC_L3L3CPEPROFUPDATEREQ
#include "L3L3CpeProfUpdateReq.h"     
#endif

#ifndef _INC_L3L3L2CPEPROFUPDATENOTIFY
#include "L3L3L2CpeProfUpdateNotify.h"
#endif

#ifndef _INC_L3L3L2OAMCPEREGNOTIFY
#include "L3L3L2CpeRegNotify.h"
#endif

#ifndef _INC_L3OAMCOMMONRSP
#include "L3OamCommonRsp.h"
#endif

#ifndef _INC_L3L3CFGVOICEPORTNOTIFY
#include "L3L3CfgVoicePortNotify.h"     
#endif

#ifndef _INC_L3L3CFGDMDATANOTIFY
#include "L3L3CfgDMDataNotify.h"     
#endif

#ifndef _INC_L3OAMCPEREQ
#include "L3OamCpeReq.h"
#endif

#ifndef _INC_L3OAMTEST
//#include "L3OamTest.h"
#endif

#ifndef _INC_L3OAMCOMMONREQ
#include "L3OamCommonReq.h"
#endif

#ifndef _INC_L3OAMCFGBTSREPEATERREQ
#include "L3OamCfgBtsRepeaterReq.h"
#endif
#ifndef _INC_L3OAMCFGBTSNEIBLISTREQ
#include "L3OamCfgBtsNeibListReq.h" 
#endif
#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

const UINT16  HIGH_TOS_CLASS        = 0;
const UINT16  DEFAULT_TOS_CLASS     = 1;

const UINT8   DATA_INVALID          = 0; 
const UINT8   DATA_VALID            = 1; 

const UINT8   MSGTYPE_CPEREGNOTIFY  = 0;
const UINT8   MSGTYPE_CPEPROFUPDATE = 1;


const UINT32 CPE_REGISTER_TIMER_PERIOD = 5000;//jy modified,080911
extern T_NvRamData *NvRamDataAddr;
#ifdef WBBU_CODE
extern void CPE_AddDeleteCpeInfo(UINT32 CpeID, UINT32 period);
#endif
#ifdef M_TGT_WANIF
extern UINT32  WorkingWcpeEid;
extern UINT32  WanIfCpeEid ;
extern UINT32  BakWanIfCpeEid ;
unsigned char begin_2_probeWCPE = 0;
#endif

#ifdef PID
#undef PID
#endif


extern UINT32  Enable_Wcpe_Flag_Judge;
extern UINT16   Wanif_Switch;
extern bool sagStatusFlag;
extern UINT32  RelayWanifCpeEid[20];
UINT32 bspGetLocAreaID()
{
    return NvRamDataAddr->BtsGDataCfgEle.LocAreaID;
}
 extern "C" int bspGetBtsID();
extern "C" BOOL bspGetIsPermitUseWhenSagDown();
FSMStateIndex CPEanyStateRegisterNotifyTrans :: Action(CPECCB &ccb, CMessage &msg)
{
    UINT32 eid = msg.GetEID();
    CL3L2CpeRegistNotify L3L2CpeRegNotify(msg);  
    OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[0x%08x] Register_Req, [%s state],transID:0x%x", eid, (int)strCpeSTATE[ccb.GetCurrentState()], L3L2CpeRegNotify.GetTransactionId());
      #ifdef M_TGT_WANIF
 //  if((WorkingWcpeEid == eid)||(WanIfCpeEid==eid)||(BakWanIfCpeEid==eid))
     if(begin_2_probeWCPE==0)
   	{
   	    begin_2_probeWCPE = 1;
	OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx WorkingWcpeEid[0x%08x] Register_Req, [%s state],begin_2_probeWCPE:0x%x", eid, (int)strCpeSTATE[ccb.GetCurrentState()],begin_2_probeWCPE);
   	}

   #endif
    return RegisterProcedure(ccb, L3L2CpeRegNotify);
}


//新注册流程
FSMStateIndex CPEanyStateRegisterReqTrans :: Action(CPECCB &ccb, CMessage &msg)
{
    UINT32 eid = msg.GetEID();
    CUTRegisterReq UTRegisterReq(msg);  
    OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[0x%08x] Register_Req, [%s state],transID:0x%x", eid, (int)strCpeSTATE[ccb.GetCurrentState()], UTRegisterReq.GetTransactionId());
  #ifdef M_TGT_WANIF
 // if ((WorkingWcpeEid == eid)||(WanIfCpeEid==eid)||(BakWanIfCpeEid==eid))
      if(begin_2_probeWCPE==0)
   	{
   	    begin_2_probeWCPE = 1;
	   OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx WorkingWcpeEid[0x%08x] Register_Req, [%s state],begin_2_probeWCPE:0x%x", eid, (int)strCpeSTATE[ccb.GetCurrentState()],begin_2_probeWCPE);
   	}

   #endif
    return RegisterReq(ccb, UTRegisterReq);
}


FSMStateIndex CPENorecordProfileUpdateReqTrans::Action(CPECCB &ccb, CMessage &msg)
{
if(( M_EMS_BTS_UT_PROFILE_UPDATE_REQ == msg.GetMessageId())&&(ccb.getRptFlag()==1))
//for rpt #if 0
{
    CCpeProfUpdateReq EmstoBtsReq(msg);  
    UINT32 CpeId = EmstoBtsReq.GetCPEID();
    //OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive EMS update CPE[0x%08x] profile. [IDLE state]", CpeId);

#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(CpeId, MAX_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(CpeId, MAX_CPE_REMANENT_CYCLE);
#endif
    //( 1  获取消息获得消息中的的 ProfileIE
    //UINT16  ProfSize = EmstoBtsReq.GetUTProfIESize();
    //SINT8  ProfBuff[MAX_CPE_PROFILE_SIZE];
    //memset(ProfBuff, 0, sizeof(ProfBuff));
    //EmstoBtsReq.GetUTProfIE(ProfBuff, ProfSize);

    // 修改rCCB的内容
    const T_UTProfile* profileFromEMS = EmstoBtsReq.getProfile();
    const T_UT_HOLD_BW *bw = NULL;
    //if((profileFromEMS->UTProfileIEBase.UTSDCfgInfo.Reserved&0x01) == 1)//保持带宽无条件取，桂林机场问题,jy20110802
    {
	bw= EmstoBtsReq.getHoldBW();
    }
    ccb.setUTProfile(*profileFromEMS);
    //if(bw!=NULL)
    	{
    ccb.setUTHoldBW(*bw);
    	}

    //( 2  向ems返回应答消息
    ProfUpdateRspToEms(EmstoBtsReq.GetTransactionId());

    //( 3  创建transaction向cpe发送profile更新消息  
    CL3CpeProfUpdateReq UpdateReq;
    CreateProfUpdateReqToCpe(UpdateReq, ccb);
    BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);

    //( 4  向相关模块发送相应消息
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [IDLE]", CpeId, profileFromEMS->UTProfileIEBase.AdminStatus);
    if(CPE_ADM_STATUS_INV == profileFromEMS->UTProfileIEBase.AdminStatus) //无效
    {
        
        SendDisableToDM_Voice(ccb, msg.GetMessageId(), CpeId);
    }
    else
    {        
        SendProfDatatoOther( ccb, 1);

	//开通或者欠费lijinan 20090331
	 if((profileFromEMS->UTProfileIEBase.AdminStatus==0)||(profileFromEMS->UTProfileIEBase.AdminStatus==0x10))
	 	SendEidMoneyStatustoEbCdr(CpeId,profileFromEMS->UTProfileIEBase.AdminStatus);
	 	
    }

    return CPE_SERVING;
}
    else //for rpt#else
    {
        CsabisBWInfoRsp msgBWInfoRsp(msg);
        T_sabisBWInfoRsp* pRsp = msgBWInfoRsp.getInfo();
        
        UINT32 eid = pRsp->ulPID;
        OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%08x] BWInfo_Rsp, Admin_status:%x. [IDLE state]", eid, pRsp->UTProfile.ucAdminStatus);
        
        ////CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
        
        // 修改rCCB的内容
        const T_UTProfileNew* profileFromEMS = &pRsp->UTProfile;	
	#ifdef M_TGT_WANIF	
    //如果是wcpe/rcpe,并且结果不为0,则不进行任何处理
    if((profileFromEMS->ucAdminStatus != 0)&&(ccb.getWcpeORRcpeFlag()==1))
    {
       OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WCPE/RCPE[0x%08x] BWInfo_Rsp ucAdminStatus is not 0, return. [SERVING state]", eid);
       stopBWInfoTransaction(ccb);
	return  CPE_NORECORD;
    }
	#endif
	if(pRsp->usLength == 9)//只携带状态
	{
	       ccb.setAdminStatus(pRsp->UTProfile.ucAdminStatus);
	       //创建transaction向cpe发送profile更新消息  
            CL3CpeProfUpdateReq UpdateReq;
            //CUpdateUTBWInfo UpdateReq;
            CreateProfUpdateReqToCpe(UpdateReq, ccb);
            //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
            if(M_CCB_TYPE_OLD == ccb.getCCBType())
                UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
            else
                UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
	}
	else
	{
            //构造和空口交互一样的结构比较mem新加部分jy081218
            //BWInfo.reserved1/reserved2字段被用作mem ip type/softPhone，和空口含义不同，需要恢复为0
            int mem_len;   
            T_MEM_CFG memCfgTemp;
            mem_len = msg.GetDataLength() - (19 + profileFromEMS->length() + sizeof(T_UT_HOLD_BW)); 
            memset(&memCfgTemp, 0, sizeof(T_MEM_CFG));	
            if((ccb.getCCBType()==M_CCB_TYPE_NEW)&&(mem_len>0))   	
            {
                T_sabisMemCfg *pmemTemp=NULL;      
                memCfgTemp.MemIpType = pRsp->UTProfile.BWInfo.reserved1;
                memCfgTemp.rev = 0;  
                pRsp->UTProfile.BWInfo.reserved1 = 0; 
                pmemTemp = msgBWInfoRsp.getMemCfg();
                memcpy((UINT8*)&memCfgTemp.DNSServer, (UINT8*)pmemTemp, sizeof(T_sabisMemCfg));   
            }
            if(ccb.getCCBType()==M_CCB_TYPE_NEW)
                ccb.setMemCfg(memCfgTemp);
            ccb.setUTProfile(*profileFromEMS);
            const T_UT_HOLD_BW *bw = NULL;
            //if((profileFromEMS->BWInfo.UTSDCfgInfo.Reserved &0x01)  == 1)//保持带宽无条件取，桂林机场问题,jy20110802
            {
                bw = msgBWInfoRsp.getHoldBW();
            }
            //if(bw!=NULL)
            {    
                ccb.setUTHoldBW(*bw);    
            }
            //创建transaction向cpe发送profile更新消息  
            CL3CpeProfUpdateReq UpdateReq;
            //CUpdateUTBWInfo UpdateReq;
            CreateProfUpdateReqToCpe(UpdateReq, ccb);
            //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
            if(M_CCB_TYPE_OLD == ccb.getCCBType())
                UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
            else
                UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
	}
        //向相关模块发送相应消息
        OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [IDLE]", eid, profileFromEMS->ucAdminStatus);
        if(CPE_ADM_STATUS_INV!= profileFromEMS->ucAdminStatus) //无效
        {
        
        SendProfDatatoOther(ccb, 1);
	 //开通或者欠费lijinan 20090331
	 if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
	 	SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
        }
        else
        {
        
            SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid);
        }
        stopBWInfoTransaction(ccb);
        //for rpt #endif
    }
    return CPE_NORECORD;
}


FSMStateIndex CPEServingProfileUpdateReqTrans:: Action(CPECCB &ccb, CMessage &msg)
{
if(( M_EMS_BTS_UT_PROFILE_UPDATE_REQ == msg.GetMessageId())&&(ccb.getRptFlag()==1))
{
    CCpeProfUpdateReq EmstoBtsReq(msg);  
    UINT32 CpeId = EmstoBtsReq.GetCPEID();
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive EMS update CPE[0x%08x] profile. [SERVING state]", CpeId);

    const T_UTProfile *profileFromEMS = EmstoBtsReq.getProfile();
    const T_UT_HOLD_BW *bw = NULL;
   //if((profileFromEMS->UTProfileIEBase.UTSDCfgInfo.Reserved&0x01) ==1)//保持带宽无条件取，桂林机场问题,jy20110802
   {
            bw = EmstoBtsReq.getHoldBW();	
	     OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM1] CPE[0x%08x] register Status[%d]. [SERVING]", CpeId, profileFromEMS->UTProfileIEBase.AdminStatus);
   }    
    if(bw!=NULL)
    	{
    OAM_LOGSTR4(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [SERVING],BW:%x,%x", CpeId, profileFromEMS->UTProfileIEBase.AdminStatus,bw->UL_Hold_BW,bw->DL_Hold_BW);
    	}
	else
	{
	  //OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [SERVING]", CpeId, profileFromEMS->UTProfileIEBase.AdminStatus);
	}
	if(CPE_ADM_STATUS_INV == profileFromEMS->UTProfileIEBase.AdminStatus) //无效
    {        
        SendDisableToDM_Voice(ccb, msg.GetMessageId(), CpeId);
        //设置最短保留时间, 删除CPE
#ifndef WBBU_CODE
        CTaskCpeM::CPE_AddDeleteCpeInfo(CpeId, MIN_CPE_REMANENT_CYCLE);
#else
        CPE_AddDeleteCpeInfo(CpeId, MIN_CPE_REMANENT_CYCLE);
#endif
    }
    else
    {        
        
    }
    
   bool bwsame = true;
   if(bw!=NULL)
   {
      bwsame= ccb.HoldBWCompare(*bw);
   }
    bool same = ccb.ProfileCompare(*profileFromEMS);
    if((false == same)||(false == bwsame))
    {

        // 修改rCCB的内容
        ccb.setUTProfile(*profileFromEMS);
	if(bw!=NULL)
	{
		ccb.setUTHoldBW(*bw);
	}

        //( 2  创建transaction向cpe发送profile更新消息     
        CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
        CreateProfUpdateReqToCpe(UpdateReq, ccb);
        BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);

        //( 3  向相关模块发送相应消息
        SendProfDatatoOther(ccb, 1);
		 //开通或者欠费lijinan 20090331
	 if((profileFromEMS->UTProfileIEBase.AdminStatus==0)||(profileFromEMS->UTProfileIEBase.AdminStatus==0x10))
	 	SendEidMoneyStatustoEbCdr(CpeId,profileFromEMS->UTProfileIEBase.AdminStatus);
    }

    // 向ems返回应答消息
    ProfUpdateRspToEms(EmstoBtsReq.GetTransactionId());

    if(NULL != ccb.m_pCpeRegTimer) 
        {
        ccb.m_pCpeRegTimer->Stop();
        delete ccb.m_pCpeRegTimer;
        ccb.m_pCpeRegTimer = NULL;
        }

    return  CPE_SERVING;
}

    CsabisBWInfoRsp msgBWInfoRsp(msg);
    T_sabisBWInfoRsp* pBWInfoRsp = msgBWInfoRsp.getInfo();
    UINT32 eid = pBWInfoRsp->ulPID;
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%08x] BWInfo_Rsp, Admin_status: %x. [SERVING state]", eid, pBWInfoRsp->UTProfile.ucAdminStatus);
    ccb.setSagDefaultFlag(0);//收到sag注册应答，将故障弱化标志清0
    const T_UTProfileNew* profileFromEMS = &(pBWInfoRsp->UTProfile);
    const T_UT_HOLD_BW *bw = NULL;
	#ifdef M_TGT_WANIF	
    //如果是wcpe/rcpe,并且结果不为0,则不进行任何处理
    if((profileFromEMS->ucAdminStatus != 0)&&(ccb.getWcpeORRcpeFlag()==1))
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WCPE/RCPE[0x%08x] BWInfo_Rsp ucAdminStatus is not 0, return. [SERVING state]", eid);
        stopBWInfoTransaction(ccb);	
	    if(NULL != ccb.m_pCpeRegTimer) 
        {
            ccb.m_pCpeRegTimer->Stop();
            delete ccb.m_pCpeRegTimer;
            ccb.m_pCpeRegTimer = NULL;
        }    
        CL3CpeProfUpdateReq UpdateReq;
        //因为需要将rcpe/wcpe标志带下去,所以带profile
        CreateProfUpdateReqToCpe(UpdateReq, ccb);         
        UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
        BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
        //向cpe下发切换参数
        SendHandoverParaToCpe(eid);
        SendValidFreqsParaToCpe(eid);
        SendProfDatatoOther(ccb, 1);//profile is valid
            
        //wcpe/rcpe通知计费模块状态为profileFromEMS->ucAdminStatus   
        if(profileFromEMS->ucAdminStatus==0x10)
            SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
	    return  CPE_SERVING;
    }
	#endif
	if(pBWInfoRsp->usLength == 9)//只携带状态
	{	    
       //如果终端欠费，则发送原有值给终端，发送后再记录到ccb中
       if((profileFromEMS->ucAdminStatus ==0x10)||(profileFromEMS->ucAdminStatus ==0x11))
       {
           //ccb.setAdminStatus(0);
       }
       else
       {
	       ccb.setAdminStatus(pBWInfoRsp->UTProfile.ucAdminStatus);
       }
	    //创建transaction向cpe发送profile更新消息  
            CL3CpeProfUpdateReq UpdateReq;            
            CreateProfUpdateReqToCpe(UpdateReq, ccb);            
            if(M_CCB_TYPE_OLD == ccb.getCCBType())
                UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
            else
                UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            if((profileFromEMS->ucAdminStatus ==0x10)||(profileFromEMS->ucAdminStatus ==0x11))
                ccb.setAdminStatus(profileFromEMS->ucAdminStatus);
	      //( 3  向相关模块发送相应消息
            SendProfDatatoOther(ccb, 1);
          //向cpe下发切换参数
          SendHandoverParaToCpe(eid);
          SendValidFreqsParaToCpe(eid);
            //开通或者欠费lijinan 20090331
            if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
                SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
	     //wangwenhua add 20080714
	     if(M_CCB_TYPE_NEW== ccb.getCCBType())
{
#ifndef WBBU_CODE
                CTaskCpeM::CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
#else
                CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
#endif
}
	}
	 else
        {
            //if((profileFromEMS->BWInfo.UTSDCfgInfo.Reserved&0x01)== 1)//保持带宽无条件取，桂林机场问题,jy20110802
            {
                bw =  msgBWInfoRsp.getHoldBW();
               // OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM1] CPE[0x%08x] register Status[%d]. [SERVING]", eid, profileFromEMS->ucAdminStatus);
            }
            
            if(bw!=NULL)
            {
                OAM_LOGSTR4(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [SERVING],BW:%x,%x", eid, profileFromEMS->ucAdminStatus,bw->UL_Hold_BW,bw->DL_Hold_BW);
            }
            else
            {
               // OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [SERVING]", eid, profileFromEMS->ucAdminStatus);
            }
            
            bool bwsame = true;
            if(bw!=NULL)
            {
                bwsame= ccb.HoldBWCompare(*bw);
            }
            if(ccb.getCCBType()==M_CCB_TYPE_NEW)
            {
                int mem_len;
                bool memInfoSame=true;
                T_MEM_CFG memCfgTemp;
                mem_len = msg.GetDataLength() - (19 + pBWInfoRsp->UTProfile.length() + sizeof(T_UT_HOLD_BW));
                if(mem_len>0)//携带dns等数据
                {
                    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] carry dns cfg", eid);
                    //构造和空口交互一样的结构比较mem新加部分jy081218
                    //BWInfo.reserved1/reserved2字段被用作mem ip type/softPhone，和空口含义不同，需要恢复为0
                    T_sabisMemCfg *pmemTemp=NULL; 
                    memCfgTemp.MemIpType = pBWInfoRsp->UTProfile.BWInfo.reserved1;
                    memCfgTemp.rev = 0;  
                    pBWInfoRsp->UTProfile.BWInfo.reserved1 = 0; 
                    pmemTemp = msgBWInfoRsp.getMemCfg();
                    memcpy((UINT8*)&memCfgTemp.DNSServer, (UINT8*)pmemTemp, sizeof(T_sabisMemCfg));
                    memInfoSame = ccb.MemCfgCompare(memCfgTemp);   
                }
                bool same = ccb.ProfileCompare(*profileFromEMS);
                OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x], proflie same=%d , ccb.getProfileInvalidFlag=%d", eid, same, ccb.getProfileInvalidFlag());
                if((false == same)||(false==bwsame )||(false == memInfoSame)||(ccb.getProfileInvalidFlag()==1))
                {
                    // 修改rCCB的内容
                    //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
                    ccb.setUTProfile(*profileFromEMS);
                    if(bw!=NULL)
                    {
                    ccb.setUTHoldBW(*bw);
                    }
                    //记录mem新加部分
                    if(memInfoSame==false)                
                        ccb.setMemCfg(memCfgTemp);
                    //( 2  创建transaction向cpe发送profile更新消息     
                    CL3CpeProfUpdateReq UpdateReq;
                    //CUpdateUTBWInfo UpdateReq;
                    CreateProfUpdateReqToCpe(UpdateReq, ccb);
                    //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
                    UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
                    BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                    if(ccb.getProfileInvalidFlag()==1)
                        ccb.setProfileInvalidFlag(0);
                    
                    //( 3  向相关模块发送相应消息
                    SendProfDatatoOther(ccb, 1);
                    //向cpe下发切换参数
                    SendHandoverParaToCpe(eid);
                    SendValidFreqsParaToCpe(eid);
                    //开通或者欠费lijinan 20090331
                    if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
                       SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
                }
                //如果profile相同，则只发送基本数据给cpe
                else
                {
                    //( 2  创建transaction向cpe发送profile更新消息
                    CL3CpeProfUpdateReq UpdateReq;
                    CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 1,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
                                        
                    UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
                    BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                    //向cpe下发切换参数
                    SendHandoverParaToCpe(eid);
                    SendValidFreqsParaToCpe(eid);
                    SendProfDatatoOther(ccb, 1);//profile is valid
                    //UpdateReqTemp.DeleteMessage();                    
                   
                    if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
                        SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
                }
                //wangwenhua add 20080714
#ifndef WBBU_CODE
                CTaskCpeM::CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
#else
        CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
#endif
            }
            else//v51 ut
            {
                bool same = ccb.ProfileCompare(*profileFromEMS);
                if((false == same)||(false==bwsame )||(ccb.getProfileInvalidFlag()==1))
                {
                    // 修改rCCB的内容
                    //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
                    ccb.setUTProfile(*profileFromEMS);
                    if(bw!=NULL)
                    {
                        ccb.setUTHoldBW(*bw);
                    }                 
                    //( 2  创建transaction向cpe发送profile更新消息     
                    CL3CpeProfUpdateReq UpdateReq;
                    //CUpdateUTBWInfo UpdateReq;
                    CreateProfUpdateReqToCpe(UpdateReq, ccb);
                    //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());     
                    UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);      
                    BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                    if(ccb.getProfileInvalidFlag()==1)
                    ccb.setProfileInvalidFlag(0);
                    //( 3  向相关模块发送相应消息
                    SendProfDatatoOther(ccb, 1);  
                    //向cpe下发切换参数
                    SendHandoverParaToCpe(eid);
                    SendValidFreqsParaToCpe(eid);
                    //开通或者欠费lijinan 20090331
                    if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
                        SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);			
                }
                else
                {
                    //( 2  创建transaction向cpe发送profile更新消息
                    CL3CpeProfUpdateReq UpdateReq;
                    CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 0,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
                                        
                    UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
                    BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                    //向cpe下发切换参数
                    SendHandoverParaToCpe(eid);
                    SendValidFreqsParaToCpe(eid);
                    SendProfDatatoOther(ccb, 1);//profile is valid
                    //UpdateReqTemp.DeleteMessage();                    
                   
                    if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
                        SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
                }
            }
    }
    //向相关模块发送相应消息
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [SERVING]", eid, profileFromEMS->ucAdminStatus);
    if(CPE_ADM_STATUS_INV!= profileFromEMS->ucAdminStatus)
    {
    }
    else
    {    
        SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid);
        //设置最短保留时间, 删除CPE
#ifndef WBBU_CODE
        CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#else
        CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#endif
    }
    
    if(NULL != ccb.m_pCpeRegTimer) 
    {
        ccb.m_pCpeRegTimer->Stop();
        delete ccb.m_pCpeRegTimer;
        ccb.m_pCpeRegTimer = NULL;
    }
    stopBWInfoTransaction(ccb);
    
    return  CPE_SERVING;
}

//Delete_BWInfo_Req;
FSMStateIndex CPEServingMovedawayNotifyTrans:: Action(CPECCB &ccb, CMessage &msg)
{
    CsabisDeleteBWInfoReq msgDelBWInfoReq(msg);
    const T_sabisDelBWInfoReq * const pReq = msgDelBWInfoReq.getInfo();
    UINT32 eid = pReq->ulPID;

    UINT16 flag = 0;
   
    UINT16 len = msg.GetDataLength();
    if(len == 19)
    {
        flag =0;
    }
	else
	{

        char *p =(char*) msg.GetDataPtr();
        flag = *(UINT16*)(p+19);
        switch (flag)
        {
            case 0:
            case 1:
                break;
            case 2:
                sendDeleteMsgtoUT(eid);
                break;
            case 3:
                sendDeleteMsgtoZ( (char*) msg.GetDataPtr() );
                break;
            default:
                flag = 0;
                break;                
        }
    }
    if( 3 == flag )
    {
        OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] delete MZ Port flag:%d.\n", eid,flag);//将打印级别提高wangwenhua modify 20110429
        return CPE_SERVING;
    }
    else
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] move away[SERVING] flag:%d.\n", eid,flag);//将打印级别提高wangwenhua modify 20110429

   if(flag!=2)
{
#ifndef WBBU_CODE
       CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#endif
}
   else
{
#ifndef WBBU_CODE
   	CTaskCpeM::CPE_AddDeleteCpeInfo(eid, 0);
#else
    CPE_AddDeleteCpeInfo(eid, 0);
#endif
}
    SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid,flag);
    if((ccb.getUtDlFlag()==1)&&(msg.GetDataLength()>6))
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] move away[SERVING], cpe is updating now.\n", eid);
        if(*(UINT16*)((SINT8*)msg.GetDataPtr() + 6) == 0x5a5a)//带了新基站号 v51
        {
            CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
            CComMessage* pComMsg ;
            pComMsg = new (pTaskCpeM/*this*/, msg.GetDataLength()) CComMessage;
            if (pComMsg==NULL)
            {
            	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in move away msg.");
            	return CPE_MOVEDAWAY;
            }
            pComMsg->SetDstTid(M_TID_SM);
            pComMsg->SetSrcTid(M_TID_UM);    
            pComMsg->SetMessageId(M_UM_SM_UT_MOVE_AWAY_NOTIFY); 
	        memcpy((UINT8*)((UINT8*)pComMsg->GetDataPtr()), (SINT8*)msg.GetDataPtr(), msg.GetDataLength());
	     
            if(!CComEntity::PostEntityMessage(pComMsg))
            {
            	pComMsg->Destroy();
            	pComMsg = NULL;
            }
        }
        else//v53 什么都不带，
        {
            CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
            CComMessage* pComMsg ;
            pComMsg = new (pTaskCpeM/*this*/, 12) CComMessage;
            if (pComMsg==NULL)
            {
            	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in move away msg.");
            	return CPE_MOVEDAWAY;
            }
            pComMsg->SetDstTid(M_TID_SM);
            pComMsg->SetSrcTid(M_TID_UM);    
            pComMsg->SetMessageId(M_UM_SM_UT_MOVE_AWAY_NOTIFY); 
            UINT8 uctemp[12];
            memset(uctemp, 0, 12);//只是eid有效，其他都为0
            uctemp[0] = 0xff;
            uctemp[1] = 0xff;
            memcpy(uctemp, &eid, 4);            
            if(!CComEntity::PostEntityMessage(pComMsg))
            {
            	pComMsg->Destroy();
            	pComMsg = NULL;
            }
        }
    }
    SendMoveAwaytoEbCdr(msg.GetMessageId(),eid);
    return CPE_MOVEDAWAY;
}


FSMStateIndex CPEServingCpeProfileUpdateNotifyTrans::Action(CPECCB &ccb, CMessage &msg)
{
    UINT32 eid = *(UINT32*)(((char*)(msg.GetDataPtr())) + 2 );
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx CPE[0x%08x] profile update notify.", eid);

    CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    CL3L2CpeProfUpdateNotify Notify;
    if (false == Notify.CreateMessage(*pTaskCpeM))
        return ccb.GetCurrentState();

   T_UTProfile profile ;
  
     profile = ccb.getUTProfile();
  
   unsigned int temp = 0;


    temp = (profile.UTProfileIEBase.UTSDCfgInfo.ULMaxBW )+ (profile.UTProfileIEBase.UTSDCfgInfo.DLMaxBW ) + 
		(profile.UTProfileIEBase.UTSDCfgInfo.ULMinBW ) + (profile.UTProfileIEBase.UTSDCfgInfo.DLMinBW );
  
    Notify.SetDstTid(M_TID_L2MAIN);
    Notify.SetSrcTid(M_TID_UM);
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetCpeId(eid);
    T_UT_HOLD_BW *bw = ccb.getUTHoldBW();
    Notify.setUTHoldBW(*bw);
   if(temp!=0)
   {
       Notify.SetUTSDCfgInfo(profile.UTProfileIEBase.UTSDCfgInfo);
   }
   else
   {
           Notify.SetUTSDCfgInfo(NvRamDataAddr->UTSDCfgEle.Info);
	    #ifdef M_TGT_WANIF
          T_UTSDCfgInfo data;
           if(ccb.getWcpeORRcpeFlag()==1)//如果是wcpe,且profile 无效,重新写默认的带宽和保持带宽
           {
               data = ccb.getUTSDCfg();
       	 data.Reserved |= 0x05;
       	 data.DLMaxBW = 1024;
       	 data.DLMinBW = 512;
       	 data.ULMaxBW = 1024;
       	 data.ULMinBW = 512;
       	 Notify.SetUTSDCfgInfo(data);
		 T_UT_HOLD_BW BWdefault;
               BWdefault.DL_Hold_BW = 512;
	        BWdefault.UL_Hold_BW = 512;
		 Notify.setUTHoldBW(BWdefault);			 
           }
          #endif
   }
   
    Notify.SetZmoduleEnabled( HW_TYPE_CPEZ_77 == ccb.getCpeHWType() );
    T_L2SpecialFlag flag;
    ccb.getSpecialFlag(flag);
    Notify.setSpecialFlag(flag);
    

    if(false == Notify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post CPE profile update Notify to L2 fail.");
        Notify.DeleteMessage();
    }

    return ccb.GetCurrentState();
}

//#ifdef RPT_FOR_V6
FSMStateIndex CPEServingRptBWInfoRspTrans::Action(CPECCB &ccb, CMessage &msg)
{ 
    //CsabisModifyBWInfoReq msgModifyBWInfoReq(msg);
    //T_sabisModBWInfoReq* pReqSab = msgModifyBWInfoReq.getInfo();
	//T_EmsModBWInfoRsp* pReq = (T_EmsModBWInfoRsp*)&pReqSab->ulUID;
    //UINT32 eid = pReq->ulPID;

	T_EmsModBWInfoRsp* pRsp = (T_EmsModBWInfoRsp*)((UINT8*)msg.GetDataPtr()+4);
    UINT32 eid = pRsp->ulPID;
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%08x] Modify_BWInfo_Rsp. [%s]", eid, (int)strCpeSTATE[ccb.GetCurrentState()]);
    
    const T_UTProfileNew* profileFromEMS = &(pRsp->UTProfile);
    const T_UT_HOLD_BW *bw = NULL;
    //if((profileFromEMS->BWInfo.UTSDCfgInfo.Reserved&0x01 )== 1)//保持带宽无条件取，桂林机场问题,jy20110802
    {
        //bw = msgModifyBWInfoReq.getHoldBW();
        bw = (T_UT_HOLD_BW*)( (UINT8*)&pRsp->UTProfile + pRsp->UTProfile.length() );
    }
    bool bwsame = true;
    //if(bw!= NULL)
    {
        bwsame = ccb.HoldBWCompare(*bw);
    }
    //构造和空口交互一样的结构比较mem新加部分jy081218
    //if(M_CCB_TYPE_NEW == ccb.getCCBType())
    //{
    int mem_len;
    bool memInfoSame=true;        	
    T_MEM_CFG memCfgTemp;
    T_sabisMemCfg *pmemTemp=NULL;  
    mem_len = msg.GetDataLength() - (8 + profileFromEMS->length() + sizeof(T_UT_HOLD_BW));	 
    if(mem_len>0)	
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] carry dns cfg", eid);
        //BWInfo.reserved1/reserved2字段被用作mem ip type/softPhone，和空口含义不同，需要恢复为0
        memCfgTemp.MemIpType = pRsp->UTProfile.BWInfo.reserved1;
        memCfgTemp.rev = 0;  
        pRsp->UTProfile.BWInfo.reserved1 = 0; 
        //pmemTemp = msgModifyBWInfoReq.getMemCfg();
		pmemTemp = (T_sabisMemCfg * )((UINT8*)&pRsp->UTProfile + pRsp->UTProfile.length() + sizeof(T_UT_HOLD_BW));
        memcpy((UINT8*)&memCfgTemp.DNSServer, (UINT8*)pmemTemp, sizeof(T_sabisMemCfg));
        memInfoSame = ccb.MemCfgCompare(memCfgTemp); 	     
    }
    bool same = ccb.ProfileCompare(*profileFromEMS);
    if((false == same)||(bwsame == false)||(false == memInfoSame)||(ccb.getProfileInvalidFlag()==1))
    {
        //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
        // 修改rCCB的内容
        ccb.setUTProfile(*profileFromEMS);
        if(bw!=NULL)
        {
            ccb.setUTHoldBW(*bw);	     
        }
        //记录mem新加部分
        if(memInfoSame==false)
            ccb.setMemCfg(memCfgTemp);
        
        //( 2  创建transaction向cpe发送profile更新消息     
        CL3CpeProfUpdateReq UpdateReq;
        //CUpdateUTBWInfo UpdateReq;
        CreateProfUpdateReqToCpe(UpdateReq, ccb);
        //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
        UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
        BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
        if( ccb.getProfileInvalidFlag()==1)
	        ccb.setProfileInvalidFlag(0);
    }      
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
#else
    CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
#endif
    return ccb.GetCurrentState();
} 
FSMStateIndex CPEServingRptRfOnOffReqTrans::Action(CPECCB &ccb, CMessage &msg)
{ 
	tRptRfReq* pd = (tRptRfReq*)msg.GetDataPtr();
	CTaskCpeM::GetInstance()->RPT_SendComMsg(pd->ulPID, M_TID_CPECM,M_BTS_RPT_RPTONOFF_REQ,(UINT8*)msg.GetDataPtr(),sizeof(tRptRfReq));
//	OAM_LOGSTR1(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPEServingRptRfOnOffReqTrans[%08x]", pd->ulPID);
    return ccb.GetCurrentState();
}    
FSMStateIndex CPEServingRptRfOnOffRspTrans::Action(CPECCB &ccb, CMessage &msg)
{ 
//	OAM_LOGSTR1(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] msg.GetDefaultDataLen[%02d]", sizeof(tRptRfCfgRsp));
	tRptRfCfgRsp* pd = (tRptRfCfgRsp*)msg.GetDataPtr();
	CTaskCpeM::GetInstance()->RPT_SendComMsg(pd->ulPID, M_TID_EMSAGENTTX, M_BTS_EMS_RPTONOFF_RSP,(UINT8*)msg.GetDataPtr(),sizeof(tRptRfCfgRsp));
	
    return ccb.GetCurrentState();
}    
FSMStateIndex CPEServingRptCfgReqTrans::Action(CPECCB &ccb, CMessage &msg)
{
//	OAM_LOGSTR1(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] msg.GetDefaultDataLen[%02d]", sizeof(tRptCfgReq));
	tRptCfgReq* pd = (tRptCfgReq*)msg.GetDataPtr();
	CTaskCpeM::GetInstance()->RPT_SendComMsg(pd->ulPID, M_TID_CPECM,M_BTS_RPT_RPTCFG_REQ,(UINT8*)msg.GetDataPtr(),sizeof(tRptCfgReq));
	return ccb.GetCurrentState();
}
FSMStateIndex CPEServingRptCfgRspTrans::Action(CPECCB &ccb, CMessage &msg)
{   
//	OAM_LOGSTR1(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] msg.GetDefaultDataLen[%02d]", sizeof(tRptRfCfgRsp));
	tRptRfCfgRsp* pd = (tRptRfCfgRsp*)msg.GetDataPtr();
	CTaskCpeM::GetInstance()->RPT_SendComMsg(pd->ulPID, M_TID_EMSAGENTTX, M_BTS_EMS_RPTCFG_RSP,(UINT8*)msg.GetDataPtr(),sizeof(tRptRfCfgRsp));
	
    return ccb.GetCurrentState();
}
FSMStateIndex CPEServingRptGetReqTrans::Action(CPECCB &ccb, CMessage &msg)
{
//	OAM_LOGSTR1(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] msg.GetDefaultDataLen[%02d]", sizeof(tRptGetReq));
	tRptGetReq* pd = (tRptGetReq*)msg.GetDataPtr();
	CTaskCpeM::GetInstance()->RPT_SendComMsg(pd->ulPID, M_TID_CPECM,M_BTS_RPT_RPTGET_REQ,(UINT8*)msg.GetDataPtr(),sizeof(tRptGetReq));
    return ccb.GetCurrentState();
}
FSMStateIndex CPEServingRptGetRspTrans::Action(CPECCB &ccb, CMessage &msg)
{
//	OAM_LOGSTR1(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPEServingRptGetRspTrans::msg.GetDefaultDataLen[%02d]", sizeof(tRptGetRsp));
	tRptGetRsp* pd = (tRptGetRsp*)msg.GetDataPtr();
		if( 0 == pd->usTransID )
		{
			tRptAlrmNtfy tNtfy;
			tNtfy.usTransID      = pd->usTransID     ;  
			tNtfy.ulPID          = pd->ulPID         ; 
			tNtfy.usPwr          = pd->usPwr         ; 
			tNtfy.usRcvFreq      = pd->usRcvFreq     ; 
			tNtfy.usTransFreq    = pd->usTransFreq   ; 
			tNtfy.usDownGain     = pd->usDownGain    ; 
			tNtfy.usUpGain       = pd->usUpGain      ; 
			tNtfy.usHemperature  = pd->usHemperature ; 
			tNtfy.usHumidity     = pd->usHumidity    ; 
			tNtfy.usHeartBeat    = pd->usHeartBeat   ; 
			tNtfy.usVerData      = pd->usVerData     ; 
			tNtfy.usRFAlrm       = pd->usRFAlrm      ; 
			tNtfy.usEnvAlarm     = pd->usEnvAlarm    ; 
			tNtfy.usCtrlAlrm     = pd->usCtrlAlrm    ; 
			tNtfy.usRsvAlrm      = pd->usRsvAlrm     ; 
			tNtfy.usBBStatus     = pd->usBBStatus    ; 
			CTaskCpeM::GetInstance()->RPT_SendComMsg(pd->ulPID, M_TID_EMSAGENTTX, M_BTS_EMS_RPTALRM_NOTIFY,(UINT8*)&tNtfy,sizeof(tRptAlrmNtfy));
		}
		else
			CTaskCpeM::GetInstance()->RPT_SendComMsg(pd->ulPID, M_TID_EMSAGENTTX, M_BTS_EMS_RPTGET_RSP,(UINT8*)msg.GetDataPtr(),sizeof(tRptGetRsp));
    return ccb.GetCurrentState();
}

//Add for Z_Module Register.
FSMStateIndex CPE_Z_RegisterNotifyTrans::Action(CPECCB &ccb, CMessage &msg)
{
    FSMStateIndex state = ccb.GetCurrentState();
    UINT32 eid = msg.GetEID();
    if ( HW_TYPE_CPEZ_77 != ccb.getCpeHWType() )
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WARNING!!! Z_module Register from CPE[0x%08x], which can't support Z module.", eid);
        return state;
    }

    //Z模块注册
    CL3ZmoduleRegister msgZRegister(msg);  
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Z_module Register from CPE[0x%08x]", eid);
    T_ZmoduleBlock *pZmodBlock = msgZRegister.getZmoduleBlock();
    if ((pZmodBlock->cidNum > MAX_CID_NUM) || (pZmodBlock->Znum > MAX_Z_MOD_NUM))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Z_module Register from CPE[0x%08x], error CID number or Z module number.", eid);
        return state;
        }
    //回复response.
    CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    CZmoduleRegsterResponse msgZmodRegisterResponse;
    if (false == msgZmodRegisterResponse.CreateMessage(*pTaskCpeM))
        return state;
    msgZmodRegisterResponse.SetEID(eid);
    msgZmodRegisterResponse.SetDstTid(msgZRegister.GetSrcTid());
    msgZmodRegisterResponse.setLoginFlag(pZmodBlock->LoginFlag);
    msgZmodRegisterResponse.setCidNum(pZmodBlock->cidNum);
    msgZmodRegisterResponse.setZNum(pZmodBlock->Znum);
    msgZmodRegisterResponse.setMsgInd(pZmodBlock->reserve);
    if (false == msgZmodRegisterResponse.Post())
        {
        msgZmodRegisterResponse.DeleteMessage();
        return state;
        }

    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Z_module Register from CPE[0x%08x], notify VOICE.", eid);
////记录Z模块注册信息
    ccb.setZmoduleBlock(pZmodBlock);

////给Voice发enable.
    UINT32 ulMask  = 0;
    UINT8  idx     = 0;
    UINT32 UIDs[MAX_CID_NUM]  = {0};
    for (idx = 0; idx < pZmodBlock->cidNum; ++idx)
        {
        //有效的端口先enable.
        UINT8  cid = pZmodBlock->cid_uid[idx].cid;
        UIDs[idx]  = pZmodBlock->cid_uid[idx].UID;
        CCfgVoicePortNotify NotifyVoiceMsg; 
        if (true == NotifyVoiceMsg.CreateMessage(*pTaskCpeM))
            {
            NotifyVoiceMsg.SetEID(eid);
            NotifyVoiceMsg.SetDstTid(M_TID_VOICE);
            NotifyVoiceMsg.SetMessageId(M_CPEM_VOICE_VPORT_ENABLE_NOTIFY);
            NotifyVoiceMsg.SetVoicePort(cid);
            NotifyVoiceMsg.SetVoicePortID(UIDs[idx]);
            NotifyVoiceMsg.setIsCpeZ(true);
            if (false == NotifyVoiceMsg.Post())
                {
                OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post M_CPEM_VOICE_VPORT_ENABLE_NOTIFY fail when Z_module register.");
                NotifyVoiceMsg.DeleteMessage();
                }
            else
                {
                OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%X] Z_module register::Enable  CID:%d, UID:%X", eid, cid, UIDs[idx]);
                }
            }
        ulMask |= (1<<cid); //把相应的位置1
        }

    //ulMask显示的无效端口disable.
    for (idx = 0; idx < MAX_CID_NUM; ++idx)
        {
        if (0 == (ulMask & 0x1))
            {
            //发disable.
            CCfgVoicePortNotify NotifyVoiceMsg; 
            if (true == NotifyVoiceMsg.CreateMessage(*pTaskCpeM))
                {
                NotifyVoiceMsg.SetEID(eid);
                NotifyVoiceMsg.SetDstTid(M_TID_VOICE);
                NotifyVoiceMsg.SetMessageId(M_CPEM_VOICE_VPORT_DISABLE_NOTIFY);
                NotifyVoiceMsg.SetVoicePort(idx);
                NotifyVoiceMsg.SetVoicePortID(idx); //UID.
                NotifyVoiceMsg.setIsCpeZ(true);
                if (false == NotifyVoiceMsg.Post())
                    {
                    OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post M_CPEM_VOICE_VPORT_DISABLE_NOTIFY fail when Z_module register.");
                    NotifyVoiceMsg.DeleteMessage();
                    }
                else
                    {
                    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%X] Z_module register::Disable CID:%d", eid, idx);
                    }
                }
            }
        ulMask >>= 1;
        }

////通知EMS.
    CCpeZRegisterNotify msgCpeZRegister;
    if (false == msgCpeZRegister.CreateMessage(*pTaskCpeM))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] System encounter exception!!! CreateMessage fail.");
        return state;
        }
    msgCpeZRegister.SetTransactionId(OAM_DEFAUIT_TRANSID);
    msgCpeZRegister.setCpeZId(eid);
    UINT8 Znum = pZmodBlock->Znum;
    msgCpeZRegister.setZnum(Znum);
    UINT8 *p = (UINT8*)pZmodBlock;
    UINT8 *pUid_Ver = p + pZmodBlock->UidVer_Offset();
    msgCpeZRegister.setUID_Ver(pUid_Ver, Znum*sizeof(T_UID_VER_PAIR));
    msgCpeZRegister.SetDstTid(M_TID_EMSAGENTTX);

    if(false == msgCpeZRegister.Post())
        {
        msgCpeZRegister.DeleteMessage();
        return state;
        }

    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Z_module Register success from CPE[0x%08x], notify EMS...", eid);
    return state;
}


FSMStateIndex CPEWaitforemsProfileUpdateReqTrans:: Action(CPECCB &ccb, CMessage &msg)
{
if(( M_EMS_BTS_UT_PROFILE_UPDATE_REQ == msg.GetMessageId())&&(ccb.getRptFlag()==1))
//for rpt #if 0
{ //#if 0
    CCpeProfUpdateReq EmstoBtsReq(msg);  
    UINT32 CpeId = EmstoBtsReq.GetCPEID();
    //OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive EMS update CPE[0x%08x] profile. [WAITING state]", CpeId);

    //( 1  获取消息中的 ProfileIE
    //UINT16  ProfSize = EmstoBtsReq.GetUTProfIESize();
    //SINT8  ProfBuff[MAX_CPE_PROFILE_SIZE];
    //memset(ProfBuff, 0, sizeof(ProfBuff));
    //EmstoBtsReq.GetUTProfIE(ProfBuff, ProfSize);
    const T_UTProfile *profileFromEMS = EmstoBtsReq.getProfile();
	const  T_UT_HOLD_BW *bw = NULL;
   //if((profileFromEMS->UTProfileIEBase.UTSDCfgInfo.Reserved&0x01) ==1)//保持带宽无条件取，桂林机场问题,jy20110802
   	{
              bw= EmstoBtsReq.getHoldBW();
   	}

    // 修改rCCB的内容
    ccb.setUTProfile(*profileFromEMS);
  //if(bw!=NULL)
  {
   	ccb.setUTHoldBW(*bw);
  }

    //( 2  创建transaction向cpe发送profile更新消息     
    CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
    CreateProfUpdateReqToCpe(UpdateReq, ccb);
    BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);

    //( 3  向相关模块发送相应消息
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [WAITING]", CpeId, profileFromEMS->UTProfileIEBase.AdminStatus);
    if(CPE_ADM_STATUS_INV == profileFromEMS->UTProfileIEBase.AdminStatus) //无效
    {
        
        SendDisableToDM_Voice(ccb, msg.GetMessageId(), CpeId);
        //设置最短保留时间, 删除CPE
#ifndef WBBU_CODE
        CTaskCpeM::CPE_AddDeleteCpeInfo(CpeId, MIN_CPE_REMANENT_CYCLE);
#else
        CPE_AddDeleteCpeInfo(CpeId, MIN_CPE_REMANENT_CYCLE);
#endif
    }
    else
    {        
        SendProfDatatoOther( ccb, 1);
	 if((profileFromEMS->UTProfileIEBase.AdminStatus==0)||(profileFromEMS->UTProfileIEBase.AdminStatus==0x10))
		SendEidMoneyStatustoEbCdr(CpeId,profileFromEMS->UTProfileIEBase.AdminStatus);
     }

    //( 4  向ems返回应答消息
    ProfUpdateRspToEms(EmstoBtsReq.GetTransactionId());

    if(NULL != ccb.m_pCpeRegTimer) 
    {
        ccb.m_pCpeRegTimer->Stop();
        ccb.m_pCpeRegTimer = NULL;
    }

    return  CPE_SERVING;
}
else //for rpt #else
{
    CsabisBWInfoRsp msgBWInfoRsp(msg);
    T_sabisBWInfoRsp* pBWInfoRsp = msgBWInfoRsp.getInfo();
    UINT32 eid = pBWInfoRsp->ulPID;
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] ReceReceive CPE[0x%08x] BWInfo_Rsp, Admin_status:%x. [WAITING state]", eid, pBWInfoRsp->UTProfile.ucAdminStatus);
    ccb.setSagDefaultFlag(0);//收到sag注册应答，将故障弱化标志清0
    const T_UTProfileNew* profileFromEMS = &(pBWInfoRsp->UTProfile);
    const T_UT_HOLD_BW *bw = NULL;
	#ifdef M_TGT_WANIF	
    //如果是wcpe/rcpe,并且结果不为0,则不进行任何处理
    if((profileFromEMS->ucAdminStatus != 0)&&(ccb.getWcpeORRcpeFlag()==1))
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WCPE/RCPE[0x%08x] BWInfo_Rsp ucAdminStatus is not 0, return. [WAITING state]", eid);
        stopBWInfoTransaction(ccb);	
	    if(NULL != ccb.m_pCpeRegTimer) 
        {
           ccb.m_pCpeRegTimer->Stop();
           delete ccb.m_pCpeRegTimer;
           ccb.m_pCpeRegTimer = NULL;
        }
        CL3CpeProfUpdateReq UpdateReq;
        //因为需要将rcpe/wcpe标志带下去,所以待profile
        CreateProfUpdateReqToCpe(UpdateReq, ccb); 
        UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
        BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
        //向cpe下发切换参数
        SendHandoverParaToCpe(eid);
        SendValidFreqsParaToCpe(eid);
        SendProfDatatoOther(ccb, 1);//profile is valid             
        //wcpe/rcpe通知计费模块 
        if(profileFromEMS->ucAdminStatus==0x10)
            SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
        
    
	    return  CPE_SERVING;
    }
	#endif
     if(pBWInfoRsp->usLength == 9)//只携带状态
     {	    
         //如果终端欠费，则发送结果0给终端，发送后再记录到ccb中
         if((profileFromEMS->ucAdminStatus ==0x10)||(profileFromEMS->ucAdminStatus ==0x11))
         {
             //ccb.setAdminStatus(0);
         }
         else
             ccb.setAdminStatus(pBWInfoRsp->UTProfile.ucAdminStatus);
         //创建transaction向cpe发送profile更新消息  
         CL3CpeProfUpdateReq UpdateReq;
         //CUpdateUTBWInfo UpdateReq;
         CreateProfUpdateReqToCpe(UpdateReq, ccb);
         //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
         if(M_CCB_TYPE_OLD == ccb.getCCBType())
             UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
         else
             UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
         BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
         if((profileFromEMS->ucAdminStatus ==0x10)||(profileFromEMS->ucAdminStatus ==0x11))
                ccb.setAdminStatus(profileFromEMS->ucAdminStatus);
         //( 3  向相关模块发送相应消息
         SendProfDatatoOther(ccb, 1);
         //向cpe下发切换参数
         SendHandoverParaToCpe(eid);
         SendValidFreqsParaToCpe(eid);
         //开通或者欠费lijinan 20090331
         if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
             SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);        
         //wangwenhua add 20080714
         if(M_CCB_TYPE_NEW== ccb.getCCBType())
         {
             #ifndef WBBU_CODE
             CTaskCpeM::CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
             #else
             CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
             #endif
         
         }
     }
   else
   {
    //if((profileFromEMS->BWInfo.UTSDCfgInfo.Reserved&0x01 )== 1)//保持带宽无条件取，桂林机场问题,jy20110802
    {
        bw = msgBWInfoRsp.getHoldBW();
    }
    if(bw!=NULL)
    {
        OAM_LOGSTR4(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [SERVING],BW:%x,%x", eid, profileFromEMS->ucAdminStatus,bw->UL_Hold_BW,bw->DL_Hold_BW);
    }
    else
    {
       // OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [SERVING]", eid, profileFromEMS->ucAdminStatus);
    }
    
    bool bwsame = true;
    if(bw!=NULL)
    {
        bwsame= ccb.HoldBWCompare(*bw);
    }
    if(ccb.getCCBType()==M_CCB_TYPE_NEW)
    {
        int mem_len;
        bool memInfoSame=true;
        T_MEM_CFG memCfgTemp;
        mem_len = msg.GetDataLength() - (19 + pBWInfoRsp->UTProfile.length() + sizeof(T_UT_HOLD_BW));
        if(mem_len>0)//携带dns等数据
        {
            OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] carry dns cfg", eid);
            //构造和空口交互一样的结构比较mem新加部分jy081218
            //BWInfo.reserved1/reserved2字段被用作mem ip type/softPhone，和空口含义不同，需要恢复为0
            T_sabisMemCfg *pmemTemp=NULL; 
            memCfgTemp.MemIpType = pBWInfoRsp->UTProfile.BWInfo.reserved1;
            memCfgTemp.rev = 0;  
            pBWInfoRsp->UTProfile.BWInfo.reserved1 = 0; 
            pmemTemp = msgBWInfoRsp.getMemCfg();
            memcpy((UINT8*)&memCfgTemp.DNSServer, (UINT8*)pmemTemp, sizeof(T_sabisMemCfg));
            memInfoSame = ccb.MemCfgCompare(memCfgTemp);   
        }
        bool same = ccb.ProfileCompare(*profileFromEMS);
        if((false == same)||(false==bwsame )||(false == memInfoSame)||(ccb.getProfileInvalidFlag()==1))
        {
            // 修改rCCB的内容
            //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
            ccb.setUTProfile(*profileFromEMS);
            if(bw!=NULL)
            {
            ccb.setUTHoldBW(*bw);
            }
            //记录mem新加部分
            if(memInfoSame==false)                
            ccb.setMemCfg(memCfgTemp);
            //( 2  创建transaction向cpe发送profile更新消息     
            CL3CpeProfUpdateReq UpdateReq;
            //CUpdateUTBWInfo UpdateReq;
            CreateProfUpdateReqToCpe(UpdateReq, ccb);
            //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
            UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            if(ccb.getProfileInvalidFlag()==1)
		  ccb.setProfileInvalidFlag(0);
			
            //( 3  向相关模块发送相应消息
            SendProfDatatoOther(ccb, 1);
            //向cpe下发切换参数
            SendHandoverParaToCpe(eid);
            SendValidFreqsParaToCpe(eid);
	 		 //开通或者欠费lijinan 20090331
 	 if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
 	 	SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
        }
        //如果profile相同，则只发送基本数据给cpe
        else
        {
            //( 2  创建transaction向cpe发送profile更新消息
            CL3CpeProfUpdateReq UpdateReq;
            CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 1,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
                                
            UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            //向cpe下发切换参数
            SendHandoverParaToCpe(eid);
            SendValidFreqsParaToCpe(eid);
            SendProfDatatoOther(ccb, 1);//profile is valid
           
            if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
            SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
        }
        //wangwenhua add 20080714
#ifndef WBBU_CODE
        CTaskCpeM::CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
#else
    CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2

#endif
    }
    else//v51 ut
    {
        bool same = ccb.ProfileCompare(*profileFromEMS);
        if((false == same)||(false==bwsame )||(ccb.getProfileInvalidFlag()==1))
        {
             // 修改rCCB的内容
             //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
             ccb.setUTProfile(*profileFromEMS);
             if(bw!=NULL)
             {
                 ccb.setUTHoldBW(*bw);
             }                 
             //( 2  创建transaction向cpe发送profile更新消息     
             CL3CpeProfUpdateReq UpdateReq;
             //CUpdateUTBWInfo UpdateReq;
             CreateProfUpdateReqToCpe(UpdateReq, ccb);
             //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());     
             UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);      
             BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
	      if(ccb.getProfileInvalidFlag()==1)
		  ccb.setProfileInvalidFlag(0);
             //( 3  向相关模块发送相应消息
             SendProfDatatoOther(ccb, 1);   
             //向cpe下发切换参数
             SendHandoverParaToCpe(eid);
             SendValidFreqsParaToCpe(eid);
			 		 //开通或者欠费lijinan 20090331
	 if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
	 	SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
         }
        //如果profile相同，则只发送基本数据给cpe
        else
        {
            //( 2  创建transaction向cpe发送profile更新消息
            CL3CpeProfUpdateReq UpdateReq;
            CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 0,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
                                
            UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            //向cpe下发切换参数
            SendHandoverParaToCpe(eid);
            SendValidFreqsParaToCpe(eid);
            SendProfDatatoOther(ccb, 1);//profile is valid
           
            if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
            SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
        }
       }
    }
    //向相关模块发送相应消息
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register Status[%d]. [SERVING]", eid, profileFromEMS->ucAdminStatus);
    if(CPE_ADM_STATUS_INV!= profileFromEMS->ucAdminStatus)
    {
    }
    else
    {    
        SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid);
        //设置最短保留时间, 删除CPE
#ifndef WBBU_CODE
        CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//分钟*2
#endif
    }
     if(NULL != ccb.m_pCpeRegTimer) 
    {
        ccb.m_pCpeRegTimer->Stop();
        delete ccb.m_pCpeRegTimer;
        ccb.m_pCpeRegTimer = NULL;
    }
    stopBWInfoTransaction(ccb);
    return  CPE_SERVING;
}
}


/*
 *NoRecod/WaitEMS状态下收到Register Notify时启动的定时器超时
 */
FSMStateIndex CPERegisterTimeOutTrans:: Action(CPECCB &ccb, CMessage &msg)
{
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%.8X] Register to EMS time out.", ccb.getEid());
    //故障弱化时不启动定时器jy20100310
    if((sagStatusFlag == false)&&(TRUE== bspGetIsPermitUseWhenSagDown()))   
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID and SAG is down, msg timeout. ", ccb.getEid() );
        delete ccb.m_pCpeRegTimer;//wangwenhua add 20090112
        ccb.m_pCpeRegTimer = NULL;
        return CPE_SERVING; 
    }
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
   
    ccb.setAdminStatus(CPE_ADM_STATUS_SAG_DOWN);//sag is down jiaying20100720
   
#if 0
    if (M_CCB_TYPE_OLD == ccb.getCCBType())
        {
        CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
        CreateDefaultProfUpdateReqToCpe(UpdateReq, ccb.getEid(), CPE_ADM_STATUS_INV);
        BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
        }
    else
        {
        CUpdateUTBWInfo UpdateReq;
        CreateDefaultProfUpdateReqToCpe(UpdateReq, ccb.getEid(), CPE_ADM_STATUS_INV);
        UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
        BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
        }
#else
    CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
    CreateDefaultProfUpdateReqToCpe(UpdateReq, ccb.getCCBType(),ccb.getEid(), CPE_ADM_STATUS_SAG_DOWN);
    if (M_CCB_TYPE_OLD == ccb.getCCBType())
        UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
    else
        UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
    BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
#endif
    SendDisableToDM_Voice(ccb, msg.GetMessageId(), ccb.getEid());

#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(ccb.getEid(), 0);
#else
    CPE_AddDeleteCpeInfo(ccb.getEid(), 0);
#endif
    delete ccb.m_pCpeRegTimer;//wangwenhua add 20090112
    ccb.m_pCpeRegTimer = NULL;

    return  CPE_NORECORD;
}


FSMStateIndex CPESwitchoffProfileUpdateReqTrans:: Action(CPECCB &ccb, CMessage &msg)
{
if(( M_EMS_BTS_UT_PROFILE_UPDATE_REQ == msg.GetMessageId())&&(ccb.getRptFlag()==1))
//for rpt #if 0
{ //#if 0
    CCpeProfUpdateReq EmstoBtsReq(msg);  
    UINT32 CpeId = EmstoBtsReq.GetCPEID();
    OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive EMS update CPE[0x%08x] profile. [SWITCHOFF state]", CpeId);

    //UINT16 ProfSize = EmstoBtsReq.GetUTProfIESize();
    //SINT8  ProfBuff[MAX_CPE_PROFILE_SIZE];
    //memset(ProfBuff, 0, sizeof(ProfBuff));
    //EmstoBtsReq.GetUTProfIE(ProfBuff, ProfSize);
    const T_UTProfile *profileFromEMS = EmstoBtsReq.getProfile();
    const T_UT_HOLD_BW *bw = NULL ;
   //if((profileFromEMS->UTProfileIEBase.UTSDCfgInfo.Reserved&0x01) == 1)//保持带宽无条件取，桂林机场问题,jy20110802
   {
	bw = EmstoBtsReq.getHoldBW();
   }

    //修改rCCB的内容
    ccb.setUTProfile(*profileFromEMS);
    //if(bw!=NULL)
    	{
	ccb.setUTHoldBW(*bw);
    	}

    //////////////////////////////////////////////
    CPE_StopTransaction(ccb);
    
    //向ems返回应答消息
    ProfUpdateRspToEms(EmstoBtsReq.GetTransactionId());

    return  CPE_SERVING;
}
else //for rpt #else
{
    CsabisBWInfoRsp msgBWInfoRsp(msg);
    T_sabisBWInfoRsp * pRsp = msgBWInfoRsp.getInfo();
    UINT32 eid = pRsp->ulPID;
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x.8X] BWInfo_Rsp, Admin_status:%x. [SWITCHOFF state]", eid, pRsp->UTProfile.ucAdminStatus);
    ccb.setSagDefaultFlag(0);//收到sag注册应答，将故障弱化标志清0
     const T_UT_HOLD_BW *bw = NULL ;
    const T_UTProfileNew *profileFromEMS = &(pRsp->UTProfile);
    #ifdef M_TGT_WANIF	
    //如果是wcpe/rcpe,并且结果不为0,则不进行任何处理
    if((profileFromEMS->ucAdminStatus != 0)&&(ccb.getWcpeORRcpeFlag()==1))
    {
       OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WCPE/RCPE[0x%08x] BWInfo_Rsp ucAdminStatus is not 0, return. [SERVING state]", eid);
       stopBWInfoTransaction(ccb);	
	if(NULL != ccb.m_pCpeRegTimer) 
       {
           ccb.m_pCpeRegTimer->Stop();
           delete ccb.m_pCpeRegTimer;
           ccb.m_pCpeRegTimer = NULL;
       }
	return  ccb.GetCurrentState();;
    }
	#endif
  if(pRsp->usLength == 9)//只携带状态
    {	    
	    ccb.setAdminStatus(pRsp->UTProfile.ucAdminStatus);	
   }
   else
   {
      //if((profileFromEMS->BWInfo.UTSDCfgInfo.Reserved&0x01 )== 1)//保持带宽无条件取，桂林机场问题,jy20110802
      {
          bw = msgBWInfoRsp.getHoldBW();
      }
    
      //if(bw!=NULL)
      {
         ccb.setUTHoldBW(*bw);
      }
        ccb.setUTProfile(*profileFromEMS);
   }
    stopBWInfoTransaction(ccb);
    if(NULL != ccb.m_pCpeRegTimer) 
    {
        ccb.m_pCpeRegTimer->Stop();
        delete ccb.m_pCpeRegTimer;
        ccb.m_pCpeRegTimer = NULL;
    }
    return  ccb.GetCurrentState();
}
//for rpt #endif
}


FSMStateIndex CPESwitchoffMovedawayNotifyTrans:: Action(CPECCB &ccb, CMessage &msg)
{
    UINT32 eid = ccb.getEid();
    UINT16 flag = 0;
   
    UINT16 len = msg.GetDataLength();
    if(len == 19)
    	{
    	    flag =0;
    	}
	else
	{
           if(len==21)
           {
               char *p =(char*) msg.GetDataPtr();
		 flag = *(UINT16*)(p+19);
           	if(flag>2)
           	{
			flag = 0;
           	}
           }
	
	}
    OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] move away. [SWITCHOFF] flag:%d.", eid,flag);//将打印级别提高wangwenhua modify 20110429
   
  
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#endif

   // if(pReq->usLength==8)
       SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid, flag);

    return CPE_MOVEDAWAY;
}


FSMStateIndex CPEAnyStateSwitchOffTrans::Action(CPECCB &ccb, CMessage &msg)
{
    UINT32 eid = msg.GetEID();
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] Switch off.", eid);
    //////////////////////////////////////////////
    CPE_StopTransaction(ccb);
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#endif
 
    SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid);
    // wang wenhua add 20080805    to send cpe switch off msg to sag ,in the msg body uid exist 
    CsabisSwitchOFFNotifyReq    msgCPESwitchOff;
    if(false == msgCPESwitchOff.CreateMessage(*CTaskCpeM::GetInstance()))
    	{
    	    return CPE_SWITCHOFF;
    	}
	 UINT32 ulUID = ccb.getUid();//*(UINT32*)(((char*)(msg.GetDataPtr())) + 2 );//这个地方可能有问题的
	 OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] Switch off.uid %x", eid,ulUID);
	//UINT32 ulUID = (UINT32*)(msg.GetDataPtr());
	msgCPESwitchOff.SetDstTid(M_TID_VCR);
	msgCPESwitchOff.setInfo(ulUID);

    if(false == msgCPESwitchOff.Post())
        {
        msgCPESwitchOff.DeleteMessage();
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Tx SWICH_OFF_Req to tVCR fail.");
        return CPE_SWITCHOFF;
        }
	
#if 0
////Notify EMS
    CCpeSwitchOffNotify msgCPESwithcOff;
    if (false == msgCPESwithcOff.CreateMessage(*CTaskCpeM::GetInstance()))
        return CPE_SWITCHOFF;

    msgCPESwithcOff.SetDstTid(M_TID_EMSAGENTTX);
    msgCPESwithcOff.SetTransactionId(OAM_DEFAUIT_TRANSID);
    msgCPESwithcOff.SetCPEID(eid);
    if (true != msgCPESwithcOff.Post())
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post CPE switchoff message fail.");
        msgCPESwithcOff.DeleteMessage();
        }
    else
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Notify EMS CPE[0x%x] switch off.", eid);
#endif
    return CPE_SWITCHOFF;
}


FSMStateIndex CPENorecordModifyBWInfoReqTrans::Action(CPECCB &ccb, CMessage &msg)
{
    CsabisModifyBWInfoReq msgModifyBWInfoReq(msg);
    T_sabisModBWInfoReq* pReq = msgModifyBWInfoReq.getInfo();
    UINT32 eid = pReq->ulPID;
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%08x] Modify_BWInfo_Req. [%s]", eid, (int)strCpeSTATE[ccb.GetCurrentState()]);
    ccb.setSagDefaultFlag(0);//收到sag注册应答，将故障弱化标志清0
    //CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MAX_CPE_REMANENT_CYCLE);

    // 修改rCCB的内容
    const T_UTProfileNew* profileFromEMS = &pReq->UTProfile; 
    #ifdef M_TGT_WANIF	
    //如果是wcpe/rcpe,并且结果不为0,则不进行任何处理
    if((profileFromEMS->ucAdminStatus != 0)&&(ccb.getWcpeORRcpeFlag()==1))
    {
       OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WCPE/RCPE[0x%08x] BWInfo_Rsp ucAdminStatus is not 0, return. [SERVING state]", eid);
      sendModifyBWInfoRsp(ccb);
	return  ccb.GetCurrentState();;
    }
	#endif
    if(pReq->usLength == 9)//只携带状态
    {	    
	    ccb.setAdminStatus(pReq->UTProfile.ucAdminStatus);	    
	}
   else
   {
        ccb.setUTProfile(*profileFromEMS);
    
       const T_UT_HOLD_BW  *bw = NULL;
       //if((pReq->UTProfile.BWInfo.UTSDCfgInfo.Reserved&0x01) == 1)//保持带宽无条件取，桂林机场问题,jy20110802
       {
           bw = msgModifyBWInfoReq.getHoldBW();
       }
       //if(bw !=NULL)
       { 
          ccb.setUTHoldBW(*bw);
       }
   }
    //创建transaction向cpe发送profile更新消息  
    //CL3CpeProfUpdateReq UpdateReq;
    //CreateProfUpdateReqToCpe(UpdateReq, ccb);
    //BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);

    //向相关模块发送相应消息
#if 0
    if(CPE_ADM_STATUS_INV == profileFromEMS->ucAdminStatus) //无效
    {
        SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid);
    }
    else
    {
        SendProfDatatoOther(ccb);
    }
#endif

    //response to HLR
    sendModifyBWInfoRsp(ccb);

    return ccb.GetCurrentState();;
}

FSMStateIndex CPEServingModifyBWInfoReqTrans:: Action(CPECCB &ccb, CMessage &msg)
{
    CsabisModifyBWInfoReq msgModifyBWInfoReq(msg);
    T_sabisModBWInfoReq* pReq = msgModifyBWInfoReq.getInfo();
    UINT32 eid = pReq->ulPID;
    OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%08x] Modify_BWInfo_Req, Admin_status:%x. [%s]", eid, pReq->UTProfile.ucAdminStatus, (int)strCpeSTATE[ccb.GetCurrentState()]);
    ccb.setSagDefaultFlag(0);//收到sag注册应答，将故障弱化标志清0
    const T_UTProfileNew* profileFromEMS = &(pReq->UTProfile);
    #ifdef M_TGT_WANIF	
    //如果是wcpe/rcpe,并且结果不为0,则不进行任何处理
    if((profileFromEMS->ucAdminStatus != 0)&&(ccb.getWcpeORRcpeFlag()==1))
    {
       OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WCPE/RCPE[0x%08x] BWInfo_Rsp ucAdminStatus is not 0, return. [SERVING state]", eid);
       sendModifyBWInfoRsp(ccb);
	return  ccb.GetCurrentState();;
    }
	#endif
    if(pReq->usLength == 9)//只携带状态
    {	   
        //如果终端欠费，则发送原有值给终端，发送后再记录到ccb中
        if((profileFromEMS->ucAdminStatus ==0x10)||(profileFromEMS->ucAdminStatus ==0x11))
        {
            //ccb.setAdminStatus(0);
        }
        else
	        ccb.setAdminStatus(pReq->UTProfile.ucAdminStatus);
	    //创建transaction向cpe发送profile更新消息  
            CL3CpeProfUpdateReq UpdateReq;
            //CUpdateUTBWInfo UpdateReq;
            CreateProfUpdateReqToCpe(UpdateReq, ccb);
            //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
            if(M_CCB_TYPE_OLD == ccb.getCCBType())
                UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
            else
                UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            if((profileFromEMS->ucAdminStatus ==0x10)||(profileFromEMS->ucAdminStatus ==0x11))
                ccb.setAdminStatus(profileFromEMS->ucAdminStatus);
	    //( 3  向相关模块发送相应消息
            SendProfDatatoOther(ccb, 1);
            //开通或者欠费lijinan 20090331
            if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
                SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
	     //wangwenhua add 20080714
	     if(M_CCB_TYPE_NEW== ccb.getCCBType())
{
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);
#else
    CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);
#endif
               
}
	}
   else
   {
        const T_UT_HOLD_BW *bw = NULL;
        //if((profileFromEMS->BWInfo.UTSDCfgInfo.Reserved&0x01 )== 1)//保持带宽无条件取，桂林机场问题,jy20110802
        {
            bw = msgModifyBWInfoReq.getHoldBW();
        }
        bool bwsame = true;
        //if(bw!= NULL)
        {
            bwsame = ccb.HoldBWCompare(*bw);
        }
        //构造和空口交互一样的结构比较mem新加部分jy081218
        if(M_CCB_TYPE_NEW == ccb.getCCBType())
        {
            int mem_len;
            bool memInfoSame=true;        	
    	 T_MEM_CFG memCfgTemp;
            T_sabisMemCfg *pmemTemp=NULL;  
            mem_len = msg.GetDataLength() - (19 + profileFromEMS->length() + sizeof(T_UT_HOLD_BW));	 
            if(mem_len>0)	
            {
                OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] carry dns cfg", eid);
                //BWInfo.reserved1/reserved2字段被用作mem ip type/softPhone，和空口含义不同，需要恢复为0
                memCfgTemp.MemIpType = pReq->UTProfile.BWInfo.reserved1;
                memCfgTemp.rev = 0;  
                pReq->UTProfile.BWInfo.reserved1 = 0; 
                pmemTemp = msgModifyBWInfoReq.getMemCfg();
    	     memcpy((UINT8*)&memCfgTemp.DNSServer, (UINT8*)pmemTemp, sizeof(T_sabisMemCfg));
                memInfoSame = ccb.MemCfgCompare(memCfgTemp); 	     
            }
            bool same = ccb.ProfileCompare(*profileFromEMS);
            if((false == same)||(bwsame == false)||(false == memInfoSame)||(ccb.getProfileInvalidFlag()==1))
            {
                //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
                // 修改rCCB的内容
                ccb.setUTProfile(*profileFromEMS);
                if(bw!=NULL)
                {
                    ccb.setUTHoldBW(*bw);	     
                }
                //记录mem新加部分
                if(memInfoSame==false)
                    ccb.setMemCfg(memCfgTemp);
                
                //( 2  创建transaction向cpe发送profile更新消息     
                CL3CpeProfUpdateReq UpdateReq;
                //CUpdateUTBWInfo UpdateReq;
                CreateProfUpdateReqToCpe(UpdateReq, ccb);
                //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
                UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
                BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                if(ccb.getProfileInvalidFlag()==1)
    		   ccb.setProfileInvalidFlag(0);
                //( 3  向相关模块发送相应消息
                SendProfDatatoOther(ccb, 1);
    	    	 		 //开通或者欠费lijinan 20090331
    	 if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
    	 	SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
                //重起定时器
                //if (usPeriodRegTimer != ccb.getPrdRegTimeValue())
            }      
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);
#else
    CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);
#endif
            
            
        }
        else
        {
            bool same = ccb.ProfileCompare(*profileFromEMS);
            if((false == same)||(bwsame == false)||(ccb.getProfileInvalidFlag()==1))
            {
                //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
                // 修改rCCB的内容
                ccb.setUTProfile(*profileFromEMS);
                if(bw!=NULL)
                {
                    ccb.setUTHoldBW(*bw);
                }
                
                //( 2  创建transaction向cpe发送profile更新消息     
                CL3CpeProfUpdateReq UpdateReq;
                //CUpdateUTBWInfo UpdateReq;
                CreateProfUpdateReqToCpe(UpdateReq, ccb);            
                UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);          
                BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                if(ccb.getProfileInvalidFlag()==1)
    		   ccb.setProfileInvalidFlag(0);
                //( 3  向相关模块发送相应消息
                SendProfDatatoOther(ccb, 1);
    	 	 if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
    		SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
                //重起定时器
                //if (usPeriodRegTimer != ccb.getPrdRegTimeValue())
            }
        }
   }
       
    if(/*CPE_ADM_STATUS_ADM */CPE_ADM_STATUS_INV!= profileFromEMS->ucAdminStatus)//wangwenhua modify 20080815
    {
    //OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register SUCCESS. [SERVING]", eid);
    }
    else
    {
        SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid);
        //设置最短保留时间, 删除CPE
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid,MIN_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#endif
      
    }
    
    //response to HLR
    sendModifyBWInfoRsp(ccb);
    return  ccb.GetCurrentState();
}

FSMStateIndex CPEWaitforemsModifyBWInfoReqTrans:: Action(CPECCB &ccb, CMessage &msg)
{
    CsabisModifyBWInfoReq msgModifyBWInfoReq(msg);
    T_sabisModBWInfoReq* pReq = msgModifyBWInfoReq.getInfo();
    UINT32 eid = pReq->ulPID;
    OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%08x] Modify_BWInfo_Req, Admin_status:%x. [%s]", eid, pReq->UTProfile.ucAdminStatus, (int)strCpeSTATE[ccb.GetCurrentState()]);
    ccb.setSagDefaultFlag(0);//收到sag注册应答，将故障弱化标志清0
    const T_UTProfileNew* profileFromEMS = &(pReq->UTProfile);
    #ifdef M_TGT_WANIF	
    //如果是wcpe/rcpe,并且结果不为0,则不进行任何处理
    if((profileFromEMS->ucAdminStatus != 0)&&(ccb.getWcpeORRcpeFlag()==1))
    {
       OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WCPE/RCPE[0x%08x] BWInfo_Rsp ucAdminStatus is not 0, return. [SERVING state]", eid);
       sendModifyBWInfoRsp(ccb);
	return  ccb.GetCurrentState();;
    }
	#endif
    if(pReq->usLength == 9)//只携带状态
    {	    
        //如果终端欠费，则发送原有值给终端，发送后再记录到ccb中
        if((profileFromEMS->ucAdminStatus ==0x10)||(profileFromEMS->ucAdminStatus ==0x11))
        {
            //ccb.setAdminStatus(0);
        }
        else
	        ccb.setAdminStatus(pReq->UTProfile.ucAdminStatus);
	    //创建transaction向cpe发送profile更新消息  
            CL3CpeProfUpdateReq UpdateReq;
            //CUpdateUTBWInfo UpdateReq;
            CreateProfUpdateReqToCpe(UpdateReq, ccb);
            //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
            if(M_CCB_TYPE_OLD == ccb.getCCBType())
                UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
            else
                UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            if((profileFromEMS->ucAdminStatus ==0x10)||(profileFromEMS->ucAdminStatus ==0x11))
                ccb.setAdminStatus(profileFromEMS->ucAdminStatus);
	     //( 3  向相关模块发送相应消息
            SendProfDatatoOther(ccb, 1);
            //开通或者欠费lijinan 20090331
            if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
                SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
	     //wangwenhua add 20080714
	     if(M_CCB_TYPE_NEW== ccb.getCCBType())
{
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid,ccb.getPrdRegTimeValue() * 60 * 2);
#else
    CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);
#endif
                
}
	}
   else
   {
        const T_UT_HOLD_BW *bw = NULL;
        //if((profileFromEMS->BWInfo.UTSDCfgInfo.Reserved&0x01 )== 1)//保持带宽无条件取，桂林机场问题,jy20110802
        {
            bw = msgModifyBWInfoReq.getHoldBW();
        }
        bool bwsame = true;
        //if(bw!= NULL)
        {
            bwsame = ccb.HoldBWCompare(*bw);
        }
        //构造和空口交互一样的结构比较mem新加部分jy081218
        if(M_CCB_TYPE_NEW == ccb.getCCBType())
        {
            int mem_len;
            bool memInfoSame=true;        	
    	 T_MEM_CFG memCfgTemp;
            T_sabisMemCfg *pmemTemp=NULL;  
            mem_len = msg.GetDataLength() - (19 + profileFromEMS->length() + sizeof(T_UT_HOLD_BW));	 
            if(mem_len>0)	
            {
                OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] carry dns cfg", eid);
                //BWInfo.reserved1/reserved2字段被用作mem ip type/softPhone，和空口含义不同，需要恢复为0
                memCfgTemp.MemIpType = pReq->UTProfile.BWInfo.reserved1;
                memCfgTemp.rev = 0;  
                pReq->UTProfile.BWInfo.reserved1 = 0; 
                pmemTemp = msgModifyBWInfoReq.getMemCfg();
    	     memcpy((UINT8*)&memCfgTemp.DNSServer, (UINT8*)pmemTemp, sizeof(T_sabisMemCfg));
                memInfoSame = ccb.MemCfgCompare(memCfgTemp); 	     
            }
            bool same = ccb.ProfileCompare(*profileFromEMS);
            if((false == same)||(bwsame == false)||(false == memInfoSame)||(ccb.getProfileInvalidFlag()==1))
            {
                //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
                // 修改rCCB的内容
                ccb.setUTProfile(*profileFromEMS);
                if(bw!=NULL)
                {
                    ccb.setUTHoldBW(*bw);	     
                }
                //记录mem新加部分
                if(memInfoSame==false)
                    ccb.setMemCfg(memCfgTemp);
                
                //( 2  创建transaction向cpe发送profile更新消息     
                CL3CpeProfUpdateReq UpdateReq;
                //CUpdateUTBWInfo UpdateReq;
                CreateProfUpdateReqToCpe(UpdateReq, ccb);
                //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
                UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
                BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                if(ccb.getProfileInvalidFlag()==1)
    		   ccb.setProfileInvalidFlag(0);
                //( 3  向相关模块发送相应消息
                SendProfDatatoOther(ccb, 1);
    	    	 		 //开通或者欠费lijinan 20090331
    	 if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
    	 	SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
                //重起定时器
                //if (usPeriodRegTimer != ccb.getPrdRegTimeValue())
            }      
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid,ccb.getPrdRegTimeValue() * 60 * 2);
#else
    CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);
#endif
          
            
        }
        else
        {
            bool same = ccb.ProfileCompare(*profileFromEMS);
            if((false == same)||(bwsame == false)||(ccb.getProfileInvalidFlag()==1))
            {
                //UINT16 usPeriodRegTimer = ccb.getPrdRegTimeValue();
                // 修改rCCB的内容
                ccb.setUTProfile(*profileFromEMS);
                if(bw!=NULL)
                {
                    ccb.setUTHoldBW(*bw);
                }
                
                //( 2  创建transaction向cpe发送profile更新消息     
                CL3CpeProfUpdateReq UpdateReq;
                //CUpdateUTBWInfo UpdateReq;
                CreateProfUpdateReqToCpe(UpdateReq, ccb);            
                UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);          
                BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                if(ccb.getProfileInvalidFlag()==1)
    		      ccb.setProfileInvalidFlag(0);
                //( 3  向相关模块发送相应消息
                SendProfDatatoOther(ccb, 1);
    	 	 if((profileFromEMS->ucAdminStatus==0)||(profileFromEMS->ucAdminStatus==0x10))
    		    SendEidMoneyStatustoEbCdr(eid,profileFromEMS->ucAdminStatus);
                //重起定时器
                //if (usPeriodRegTimer != ccb.getPrdRegTimeValue())
            }
        }
    }    
    
    if(/*CPE_ADM_STATUS_ADM */CPE_ADM_STATUS_INV!= profileFromEMS->ucAdminStatus)//wangwenhua modify 20080815
    {
    //OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register SUCCESS. [SERVING]", eid);
    }
    else
    {
        SendDisableToDM_Voice(ccb, msg.GetMessageId(), eid);
        //设置最短保留时间, 删除CPE
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid,MIN_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE);
#endif
    
    }
    
    //response to HLR
    sendModifyBWInfoRsp(ccb);
    return  ccb.GetCurrentState();
    }


FSMStateIndex CPESwitchoffModifyBWInfoReqTrans:: Action(CPECCB &ccb, CMessage &msg)
{
    CsabisModifyBWInfoReq msgModifyBWInfoReq(msg);
    T_sabisModBWInfoReq* pReq = msgModifyBWInfoReq.getInfo();
    UINT32 eid = pReq->ulPID;
    OAM_LOGSTR3(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Receive CPE[0x%08x] Modify_BWInfo_Req, Admin_status. [%s]", eid, pReq->UTProfile.ucAdminStatus, (int)strCpeSTATE[ccb.GetCurrentState()]);
    ccb.setSagDefaultFlag(0);//收到sag注册应答，将故障弱化标志清0
    const T_UTProfileNew *profileFromEMS = &(pReq->UTProfile);
   const T_UT_HOLD_BW *bw = NULL;
   #ifdef M_TGT_WANIF	
    //如果是wcpe/rcpe,并且结果不为0,则不进行任何处理
    if((profileFromEMS->ucAdminStatus != 0)&&(ccb.getWcpeORRcpeFlag()==1))
    {
       OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] WCPE/RCPE[0x%08x] BWInfo_Rsp ucAdminStatus is not 0, return. [SERVING state]", eid);
       sendModifyBWInfoRsp(ccb);
	return  ccb.GetCurrentState();;
    }
	#endif
   if(pReq->usLength == 9)//只携带状态
    {	    
	    ccb.setAdminStatus(pReq->UTProfile.ucAdminStatus);	    
	}
   else
   {
       //if((profileFromEMS->BWInfo.UTSDCfgInfo.Reserved&0x01 )== 1)//保持带宽无条件取，桂林机场问题,jy20110802
       {
          bw = msgModifyBWInfoReq.getHoldBW();
       }
        ccb.setUTProfile(*profileFromEMS);
        //if(bw!=NULL)
        {
           ccb.setUTHoldBW(*bw);
        }
   }
    //response to HLR
    sendModifyBWInfoRsp(ccb);

    return  ccb.GetCurrentState();
}


void CPETrans :: CPE_StopTransaction(CPECCB &ccb)
{
#if 1
    CTaskCpeM *taskCpeM = CTaskCpeM::GetInstance();
    UINT16 usTransId = ccb.getUpdUTProfileTransId();
    if(0 != usTransId)
    {
        CTransaction * pTransaction = taskCpeM->FindTransact(usTransId);
        if(!pTransaction)
        {
            OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] can not find CPE[0x%.8X] Profile Update transaction; transid[0x%x]", ccb.getEid(), usTransId);
            //2007-4-27如果transaction被停止后立刻创建了新的transaction.
            //这个时候不能改称0
            //ccb.SetUpdateTransId(0X0000);
        }
        else
        {
            pTransaction->EndTransact();
            delete pTransaction;
            OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] stop CPE[0x%.8X] Profile Update transaction[0x%x].", ccb.getEid(), usTransId);
            ccb.setUpdUTProfileTransId(0);
        }
    }
    usTransId = ccb.getBWInfoTransId();
    if(0 != usTransId)
    {
        CTransaction * pTransaction = taskCpeM->FindTransact(usTransId);
        if(!pTransaction)
        {
            OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] can not find CPE[0x%.8X] BWInfo_Req transaction; transid[0x%x]", ccb.getEid(), usTransId);
        }
        else
        {
            pTransaction->EndTransact();
            delete pTransaction;
            OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] stop CPE[0x%.8X] BWInfo_Req transaction[0x%x].", ccb.getEid(), usTransId);
            ccb.setBWInfoTransId(0);
        }
    }
#else
    stopUpdUTProfileTransaction(ccb);
    stopBWInfoTransaction(ccb);
#endif
}    

/****flag
0------表示正常切换
1-------表示取消
2------表示删除


****/
bool CPETrans :: SendDisableToDM_Voice(CPECCB &ccb, UINT16 usMsgId, UINT32 eid,UINT16 flag)
{
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
    UINT16 MsgId;
    if((MSGID_BWINFO_DEL_REQ == usMsgId)&&(flag==0))
    { 
        MsgId = M_EMS_BTS_UT_MOVEAWAY_NOTIFY;
    }
    else  //Profile Update Req Msg 指示CPE无效
    {
        MsgId = M_CM_DM_DELETE_UT_NOTIFY;
    }

    CCpeReq NotifyDMMsg;
    if (false == NotifyDMMsg.CreateMessage(*pTCpeInst))
        return false;

    NotifyDMMsg.SetTransactionId(OAM_DEFAUIT_TRANSID);
    NotifyDMMsg.SetMessageId(MsgId);
    NotifyDMMsg.SetDstTid(M_TID_DM);
    NotifyDMMsg.SetEID(eid);
    if(true != NotifyDMMsg.Post())
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Send CPE[0x%.8x] move away to tDM fail.", eid);
        NotifyDMMsg.DeleteMessage();
    }
    else
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Send CPE[0x%.8x] move away to tDM", eid);
    }

    if(MSGID_BWINFO_DEL_REQ == usMsgId)
    { 
        MsgId = M_CPEM_VOICE_CPE_MOVEAWAY_NOTIFY;
    }
    else  //Profile Update Req Msg 指示CPE无效
    {
        MsgId = M_CPEM_VOICE_VPORT_DISABLE_NOTIFY;
    }

////向Voice发送VoicePortDisable指示消息
////UINT32 OldVPInfo = ccb.GetVoicePortMask();
    for (UINT8 Index = 0; Index < MAX_VOICE_PORT_NUM; Index++)
    {
        CCfgVoicePortNotify NotifyVoiceMsg; 
        if (false == NotifyVoiceMsg.CreateMessage(*pTCpeInst))
            continue;
        NotifyVoiceMsg.SetEID(eid);
        NotifyVoiceMsg.SetDstTid(M_TID_VOICE);
        NotifyVoiceMsg.SetMessageId(MsgId);
        NotifyVoiceMsg.SetVoicePort(Index);
        NotifyVoiceMsg.SetVoicePortID(ccb.getUid());
        NotifyVoiceMsg.setIsCpeZ( HW_TYPE_CPEZ_77 == ccb.getCpeHWType() );
        if(true != NotifyVoiceMsg.Post())
        {
            OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Send CPE[0x%.8x] Disable to VOICE port[%d] fail", eid, Index);
            NotifyVoiceMsg.DeleteMessage();
        }
    }

    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Send CPE[0x%.8x] Disable to VOICE", eid);

    CCpeReq NotifyL2;
    if (false == NotifyL2.CreateMessage(*pTCpeInst))
        return false;

    NotifyL2.SetTransactionId(OAM_DEFAUIT_TRANSID);
    NotifyL2.SetMessageId(M_L3_L2_CPE_PROFILE_DELETE_NOTIFY);
    NotifyL2.SetDstTid(M_TID_L2MAIN);
    NotifyL2.SetCPEID(eid);
    if(true != NotifyL2.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post CPE profile delete Notify to L2 fail");
        NotifyL2.DeleteMessage();
    }
    else
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post CPE[0x%.8x] profile delete Notify to L2", eid);
    }

    return true;
}

bool sendEidtoEb(UINT32 eid,UINT8 AdminStatus)
{
	CComMessage      *pComMsg     = new ( CTaskCpeM::GetInstance(), 1 )CComMessage;
        if ( NULL == pComMsg )
        {
        //delete pComMsgNode;
        return false;
        }

	    pComMsg->SetDstTid( M_TID_EB );
	    pComMsg->SetSrcTid( M_TID_UM );
	    pComMsg->SetEID( eid );
	    pComMsg->SetMessageId( M_EMS_BTS_UT_PROFILE_UPDATE_REQ ); 
	    UINT8*pData = (UINT8*)pComMsg->GetDataPtr();
	    pData[0] = AdminStatus;

	   return CComEntity::PostEntityMessage( pComMsg );
}

bool CPETrans ::SendEidMoneyStatustoEbCdr(UINT32 eid,UINT8 AdminStatus)
{
	CComMessage      *pComMsg     = new ( CTaskCpeM::GetInstance(), 1 )CComMessage;
        if ( NULL == pComMsg )
        {
        //delete pComMsgNode;
        return false;
        }

	    pComMsg->SetDstTid( M_TID_EB );
	    pComMsg->SetSrcTid( M_TID_UM );
	    pComMsg->SetEID( eid );
	    pComMsg->SetMessageId( M_EMS_BTS_UT_PROFILE_UPDATE_REQ ); 
	    UINT8*pData = (UINT8*)pComMsg->GetDataPtr();
	    pData[0] = AdminStatus;

	   if( CComEntity::PostEntityMessage( pComMsg ) ==false)
	   	pComMsg->Destroy();
	   return true;
		
	

}

bool CPETrans ::SendMoveAwaytoEbCdr(UINT16 msg_id,UINT32 eid)
{
	CComMessage      *pComMsg     = new ( CTaskCpeM::GetInstance(), 0 )CComMessage;
        if ( NULL == pComMsg )
        {
        //delete pComMsgNode;
        return false;
        }

	    pComMsg->SetDstTid( M_TID_EB );
	    pComMsg->SetSrcTid( M_TID_UM );
	    pComMsg->SetEID( eid );
	    pComMsg->SetMessageId( msg_id ); 

	   if( CComEntity::PostEntityMessage( pComMsg ) ==false)
	   	pComMsg->Destroy();
	   return true;
}


/*添加参数表示该消息是否从hlr收到应当后的反应*/
bool CPETrans :: SendProfDatatoOther(const CPECCB &ccb, UINT8 forDefault)
{
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
    UINT32 eid = ccb.getEid();

    //  向L2发送cpe profile update notifycation.
    CL3L2CpeProfUpdateNotify L3L2ProfUpdateNotify;
    if (false == L3L2ProfUpdateNotify.CreateMessage(*pTCpeInst))
        return false;

    L3L2ProfUpdateNotify.SetEID(eid);
    L3L2ProfUpdateNotify.SetCpeId(eid);    
    L3L2ProfUpdateNotify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    L3L2ProfUpdateNotify.SetDstTid(M_TID_L2MAIN);        
    L3L2ProfUpdateNotify.SetZmoduleEnabled( HW_TYPE_CPEZ_77 == ccb.getCpeHWType() );
    T_L2SpecialFlag flag;
    ccb.getSpecialFlag(flag);
    L3L2ProfUpdateNotify.setSpecialFlag(flag);
    T_UT_HOLD_BW* bw = ((CPECCB&)ccb).getUTHoldBW();
    #ifdef M_TGT_WANIF
    T_UTSDCfgInfo data;
	/*如果收到的profile无效,则下发默认配置*/
    if((ccb.getWcpeORRcpeFlag()==1)&&(forDefault==0))
    {
        data = ccb.getUTSDCfg();
	 data.Reserved |= 0x05;
	 data.DLMaxBW = 1024;
	 data.DLMinBW = 512;
	 data.ULMaxBW = 1024;
	 data.ULMinBW = 512;
	 L3L2ProfUpdateNotify.SetUTSDCfgInfo(data);
	 T_UT_HOLD_BW bwDefault;
	 bwDefault.DL_Hold_BW = 512;
	 bwDefault.UL_Hold_BW = 512;
	 L3L2ProfUpdateNotify.setUTHoldBW(bwDefault);
    }
    else
    {
	L3L2ProfUpdateNotify.SetUTSDCfgInfo(ccb.getUTSDCfg());	
	L3L2ProfUpdateNotify.setUTHoldBW(*bw);
    }
    #else
    L3L2ProfUpdateNotify.SetUTSDCfgInfo(ccb.getUTSDCfg());
    L3L2ProfUpdateNotify.setUTHoldBW(*bw);
    #endif     
     
    if(true != L3L2ProfUpdateNotify.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Post profile update notify to L2 fail");
        L3L2ProfUpdateNotify.DeleteMessage();
    }

    if ( HW_TYPE_CPEZ_77 != ccb.getCpeHWType() )
        {
        //不带Z模块的CPE
        UINT32 portNum = ccb.getVoicePortMask();
        UINT16 MsgId;
        if (0 == portNum)
            { 
            MsgId = M_CPEM_VOICE_VPORT_DISABLE_NOTIFY;
            }
        else
            {
            MsgId = M_CPEM_VOICE_VPORT_ENABLE_NOTIFY;
            }
        CCfgVoicePortNotify NotifyVoiceMsg; 
        if (true == NotifyVoiceMsg.CreateMessage(*pTCpeInst))
            {
            NotifyVoiceMsg.SetEID(eid);
            NotifyVoiceMsg.SetDstTid(M_TID_VOICE);
            NotifyVoiceMsg.SetMessageId(MsgId);
            NotifyVoiceMsg.SetVoicePort(0);
            NotifyVoiceMsg.SetVoicePortID(ccb.getUid());
            NotifyVoiceMsg.setIsCpeZ(false);
            if (false == NotifyVoiceMsg.Post())
                {
                OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post msg[0x%04x] fail", NotifyVoiceMsg.GetMessageId());
                NotifyVoiceMsg.DeleteMessage();
                }
            else
                {
                OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Notify tVoice to enable or disable port.");
                }
            }
        }

    //  向L3的数据管理任务发送cpe profile相关消息
    CCfgDMDataNotify    NotifyDMMsg; 
    if (false == NotifyDMMsg.CreateMessage(*pTCpeInst))
        return false;

    NotifyDMMsg.SetEID(eid);
    NotifyDMMsg.SetTransactionId(OAM_DEFAUIT_TRANSID);
    NotifyDMMsg.SetDstTid(M_TID_DM);
    NotifyDMMsg.SetMobility(ccb.getMobility());
    NotifyDMMsg.SetDHCPRenew(ccb.getDHCPrenew());
    NotifyDMMsg.SetVLanID(ccb.getVlanID());
    UINT8 MaxIpNum = ccb.getMaxIPnum();
    if(MaxIpNum > 20)
    {
        MaxIpNum = 20;
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "Configuration to DM error. Max IP mum > 20 and is corrected to 20");
    }
    NotifyDMMsg.SetMaxIpNum(MaxIpNum);
    
    T_CpeFixIpInfoForData CpeFixIpInfoForData[MAX_FIX_IP_NUM];
    memset(CpeFixIpInfoForData, 0, sizeof(CpeFixIpInfoForData));

    UINT8 FixIpNum = ccb.getFixIpnum();
    if(FixIpNum > MAX_FIX_IP_NUM)
    {
        FixIpNum = MAX_FIX_IP_NUM;
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "Configuration to DM error. Fixed-IP mum > 20 and is corrected to 20");
    }
    NotifyDMMsg.SetFixIpNum(FixIpNum);
    const T_CpeFixIpInfo *pFixIpFromEMS = ccb.getFixIPinfo();
    for(UINT8 i = 0; i < FixIpNum; i++)
    {
        CpeFixIpInfoForData[i].CPEID = eid;
        memcpy(CpeFixIpInfoForData[i].MAC, &pFixIpFromEMS[i], sizeof(T_CpeFixIpInfo));
    }
    NotifyDMMsg.SetCpeFixIpInfo((SINT8*)CpeFixIpInfoForData, FixIpNum * sizeof(T_CpeFixIpInfoForData));
    if(true != NotifyDMMsg.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post tDM CPE-config fail");
        NotifyDMMsg.DeleteMessage();
    }

    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post tDM CPE[0x%08x]-config", eid);

    return true;
}


bool CPETrans :: GetTosInfo(T_CpeToSCfgEle *TosInfo, UINT16 &TosNum, UINT8 Type)
{
    T_ToSCfgEle *pCfgTos = (T_ToSCfgEle*)&(NvRamDataAddr->ToSCfgEle);        
    UINT16 Num = 0;
    for(UINT16 Index = 0; Index < MAX_TOS_ELE_NUM; Index++)
    {    
        if(pCfgTos[Index].SFMapping == Type) //将符合条件的配置下去
        {
            TosInfo[Num].TOSValue = (UINT8)Index;
            TosInfo[Num].Priority = pCfgTos[Index].SFMapping;
            Num++;       
        }
    }
    TosNum = Num;

    return true;
}

bool CPETrans :: CreateNoPrifileIEProfUpdateReqToCpe(CL3CpeProfUpdateReq &UpdateReq,UINT8 Ut_type ,UINT32 eid)
{
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();

    //计算消息的实际大小    
    UINT16 TosNum = 0;
    T_CpeToSCfgEle TosCfgA[MAX_TOS_ELE_NUM]; 
    memset(TosCfgA, 0, sizeof(TosCfgA));
    GetTosInfo(TosCfgA, TosNum, HIGH_TOS_CLASS);
    if(TosNum > MAX_TOS_ELE_NUM)
    { 
        TosNum = MAX_TOS_ELE_NUM;
    }

    UINT16 TosDataLen = TosNum * sizeof(T_CpeToSCfgEle);

    CL3OamCfgBtsRepeaterReq L3OamCfgBtsRepeaterReq;
    UINT32 RepeaterInfoLen = L3OamCfgBtsRepeaterReq.GetDataLenFromNvram();

    UINT32 BTSIDs[NEIGHBOR_BTS_NUM] = {0};
    UINT16 BtsNum = 0;
    UINT16 BtsinfoLen = getBtsIDs(BTSIDs, BtsNum);
    
    if (false == UpdateReq.CreateMessage(*pTCpeInst/*, MsgLen -2*/))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CreateMessage error, this may cause unexpected results.");
        return false;
        }
    UpdateReq.SetEID(eid);
    UpdateReq.SetDstTid(M_TID_CPECM);
    UpdateReq.SetTransactionId(OAM_DEFAUIT_TRANSID);
    UpdateReq.SetVersion(OAM_DEFAUIT_TRANSID);
    //UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);

    //设置资源管理相关消息内容
    T_RMPoliceEle *pRMPCfgEle = (T_RMPoliceEle*)&(NvRamDataAddr->RMPoliceEle);
    UpdateReq.SetLocAreaId(bspGetLocAreaID()); 
    UpdateReq.SetBWReqInter(pRMPCfgEle->BWReqInterval);
    UpdateReq.SetSessRelThres(pRMPCfgEle->SRelThreshold);
    UpdateReq.SetProfileDataFlag(WITHOUT_PROFILEDATA);  //profile is invalide
    
    //设置tos相关消息内容
    UINT8 *pTosData = (UINT8 *)(UpdateReq.GetDataPtr()) + sizeof(T_L3CpeProfUpdateReqBaseNoIE);
    memcpy(pTosData, &TosNum, sizeof(UINT16));
    pTosData = pTosData + sizeof(UINT16);
    memcpy(pTosData, (SINT8 *)&TosCfgA[0] , TosDataLen);
    UINT8 *pNeiData = pTosData + TosDataLen;
    UINT8 temp_len = 0;
    //设置BTS Repeater 相关消息内容  
    if(BtsNum <= NEIGHBOR_BTS_NUM)
    {
        memcpy(pNeiData, &BtsNum, 2);
        pNeiData += 2;
        memcpy(pNeiData, BTSIDs, BtsNum * sizeof(UINT32));
        pNeiData += BtsNum * sizeof(UINT32);
        if ( sizeof(UINT16) == RepeaterInfoLen )
        {
            memset(pNeiData, 0, sizeof(UINT16));
	    temp_len = 2;
        }
        else
        {
            memcpy(pNeiData, (SINT8 *)&(NvRamDataAddr->BTSRepeaterEle), RepeaterInfoLen );
	    temp_len = RepeaterInfoLen;
        }
    }
    else
    {
        OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Profile Update Request[No IE] to CPE with invalid Repeater configuration.");  
        memset(pNeiData, 0, 4);
	temp_len = 4;
    }
	UINT16 *p;
   if(Ut_type == M_CCB_TYPE_NEW)//if v5.2 UT  wangwenhua add 20080716
   {
          UINT8 *pBtsDta = pNeiData + temp_len;
              UINT32 btsid = (bspGetBtsID());
           memcpy(pBtsDta,(SINT8*)&btsid,4);
	   //magic
	    p = (UINT16*)(pBtsDta+4);
	    *(p) = 0x55aa;
	    p += 1;

	    //N_parameter;
	    *p = ( (NvRamDataAddr->N_parameter.UT_PowerLock_Timer_Th & 0xF)<<7 )
	       | ( (NvRamDataAddr->N_parameter.Ci_Jump_detection& 0x1F)<<2 )
	       | ( (NvRamDataAddr->N_parameter.N_Algorithm_Switch&0x1)<<1 )
	       | 0x1;
          UpdateReq.SetDataLength(UpdateReq.length() + 4 +4);
		  
   }
   else
   {
   	  //magic
	    p = (UINT16*)(pNeiData + temp_len);
	    *(p) = 0x55aa;
	    p += 1;

	    //N_parameter;
	    *p = ( (NvRamDataAddr->N_parameter.UT_PowerLock_Timer_Th & 0xF)<<7 )
	       | ( (NvRamDataAddr->N_parameter.Ci_Jump_detection& 0x1F)<<2 )
	       | ( (NvRamDataAddr->N_parameter.N_Algorithm_Switch&0x1)<<1 )
	       | 0x1;
   	  UpdateReq.SetDataLength(UpdateReq.length()+4);
      OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send register rsp to CPE[0x%08x], no profile.", eid );
   }
   	

    return true;
}

bool CPETrans :: CreateProfUpdateReqToCpe(CL3CpeProfUpdateReq &UpdateReq, const CPECCB& ccb)
{
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
    const T_UTProfile &profile = ccb.getUTProfile();
    UINT32 eid = ccb.getEid();
     const T_UT_HOLD_BW *bw =  ((CPECCB&)ccb).getUTHoldBW();//wangwenhua add 20080916
    UINT16 TosNum = 0;
    T_CpeToSCfgEle TosCfgA[MAX_TOS_ELE_NUM]; 
    memset(TosCfgA, 0, sizeof(TosCfgA));
    GetTosInfo(TosCfgA, TosNum, HIGH_TOS_CLASS);

    CL3OamCfgBtsRepeaterReq L3OamCfgBtsRepeaterReq;
    UINT32 RepeaterInfoLen = L3OamCfgBtsRepeaterReq.GetDataLenFromNvram();

    UINT32 BTSIDs[NEIGHBOR_BTS_NUM] = {0};
    UINT16 BtsNum = 0;
    UINT16 BtsinfoLen = getBtsIDs(BTSIDs, BtsNum);

    if (false == UpdateReq.CreateMessage(*pTCpeInst/*, MsgLen -2*/))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CreateMessage error, this may cause unexpected results.");
        return false;
        }
    UpdateReq.SetEID(eid);
    UpdateReq.SetDstTid(M_TID_CPECM);
    UpdateReq.SetTransactionId(OAM_DEFAUIT_TRANSID);
    UpdateReq.SetVersion(OAM_DEFAUIT_TRANSID);
    //UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);

    //设置资源管理相关消息内容
    T_RMPoliceEle *pRMPCfgEle = (T_RMPoliceEle*)&(NvRamDataAddr->RMPoliceEle);
    UpdateReq.SetLocAreaId(bspGetLocAreaID()); 
    UpdateReq.SetBWReqInter(pRMPCfgEle->BWReqInterval);
    UpdateReq.SetSessRelThres(pRMPCfgEle->SRelThreshold);
#if 0
    if (CPE_ADM_STATUS_ADM == profile.UTProfileIEBase.AdminStatus)
        {
        UpdateReq.SetProfileDataFlag(WITH_PROFILEDATA);  //profile is valide
        UpdateReq.setUTProfile(profile);
        }
    else
        {
        UpdateReq.SetProfileDataFlag(WITHOUT_PROFILEDATA);  //profile is valide
        }
#else
    //必须给CPE把Admin_Status带下去，BTS带已存的profile。CPE会保存
    UpdateReq.SetProfileDataFlag(WITH_PROFILEDATA);  //profile is valide
    #ifdef M_TGT_WANIF	
    if(ccb.getWcpeORRcpeFlag()==1)
    {    	
        UpdateReq.setUTProfile(profile, 1);
#ifdef RCPE_SWITCH
		if( M_TRUNK_MRCPE_FLAG == NvRamDataAddr->stTrunkMRCpe.usflag )
		{
			for( UINT8 uc=0; uc<NvRamDataAddr->stTrunkMRCpe.usNum; uc++ )
			{
				if( NvRamDataAddr->stTrunkMRCpe.aulMRCpe[uc] == eid )
				{
					OAM_LOGSTR1(LOG_CRITICAL, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] set MOVE-RCPE falg to CPE[0x%08x]. ", eid );
					UpdateReq.setUTSDCfgRsv(0x06);
					//profile.UTProfileIEBase.UTSDCfgInfo.Reserved |= 0x06; 
					break;
				}
			}
		}
#endif
    }
    else
	UpdateReq.setUTProfile(profile, 0);
    #else
	UpdateReq.setUTProfile(profile, 0);
    #endif
#endif
    //设置tos相关消息内容
    UpdateReq.SetQosEntryNum(TosNum);
    UpdateReq.SetTosInfo((SINT8 *)&TosCfgA[0] , sizeof(T_CpeToSCfgEle) * TosNum);
       
    //设置BTS Repeater 相关消息内容   
    SINT8 RepeaterInfo[200];
    memset(RepeaterInfo, 0, sizeof(RepeaterInfo));
    SINT8* pdata = RepeaterInfo;
    UINT8 temp_len = 0;
    if(BtsNum <= NEIGHBOR_BTS_NUM)
    {
        memcpy(pdata, &BtsNum, 2);
        pdata += 2;
        memcpy(pdata, BTSIDs, BtsNum * sizeof(UINT32));
        pdata += BtsNum * sizeof(UINT32);
        if ( sizeof(UINT16) == RepeaterInfoLen )
        {
            memset(pdata, 0, sizeof(UINT16));
	     temp_len = 2;
        }
        else
        {
            memcpy(pdata, (SINT8 *)&(NvRamDataAddr->BTSRepeaterEle), RepeaterInfoLen );
	     temp_len =  RepeaterInfoLen;
        }

        UpdateReq.SetFreqInfo(RepeaterInfo , RepeaterInfoLen + BtsinfoLen);
    }
    else
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Profile Update Request to CPE with invalid Repeater configuration.");  
        UpdateReq.SetFreqInfo(RepeaterInfo ,4);
	temp_len = 4;
    }
    
UINT16 *p;
   if(ccb.getCCBType() == M_CCB_TYPE_NEW)//if v5.2 UT  wangwenhua add 20080716
   {
          UINT8 *pBtsDta = (UINT8 *)(UpdateReq.GetDataPtr()) ;
	    pBtsDta +=UpdateReq.length();
          UINT32 btsid = (bspGetBtsID());
           memcpy(pBtsDta,(SINT8*)&btsid,4);
	   //magic
	    p = (UINT16*)(pBtsDta+4);
	    *(p) = 0x55aa;
	    p += 1;

	    //N_parameter;
	    *p = ( (NvRamDataAddr->N_parameter.UT_PowerLock_Timer_Th & 0xF)<<7 )
	       | ( (NvRamDataAddr->N_parameter.Ci_Jump_detection& 0x1F)<<2 )
	       | ( (NvRamDataAddr->N_parameter.N_Algorithm_Switch&0x1)<<1 )
	       | 0x1;
		  p +=1;
             *p = bw->UL_Hold_BW;
               p +=1;
               *p = bw->DL_Hold_BW;
	   //加入mem信息
          const T_MEM_CFG &memCfg = ccb.getMemCfg();   
          UINT8 *ptemp;
	   p+=1;
          ptemp = (UINT8*)p;
          memcpy(ptemp, (UINT8*)&memCfg, sizeof(T_MEM_CFG));  
	   UINT16 msglen =  UpdateReq.length() + 4 +4 +4 +sizeof(T_MEM_CFG);
          UpdateReq.SetDataLength(msglen);
	   OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post msg to ut ,len:%d", msglen);	   
   }
   else
   {
   	  //magic
	    p = (UINT16*)((UINT8 *)(UpdateReq.GetDataPtr()) + UpdateReq.length());
	    *(p) = 0x55aa;
	    p += 1;

	    //N_parameter;
	    *p = ( (NvRamDataAddr->N_parameter.UT_PowerLock_Timer_Th & 0xF)<<7 )
	       | ( (NvRamDataAddr->N_parameter.Ci_Jump_detection& 0x1F)<<2 )
	       | ( (NvRamDataAddr->N_parameter.N_Algorithm_Switch&0x1)<<1 )
	       | 0x1;
		p +=1;
             *p = bw->UL_Hold_BW;
               p +=1;
               *p = bw->DL_Hold_BW;
	   
	   UINT16 msglen =  UpdateReq.length() + 4 +4;
          UpdateReq.SetDataLength(msglen);
	   OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post msg to ut ,len:%d", msglen);   	  
   }
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send register rsp to CPE[0x%08x], with profile.", eid );
    return true;
}


bool CPETrans :: CreateDefaultProfUpdateReqToCpe(CL3CpeProfUpdateReq &UpdateReq,UINT8 ut_type,
                                                  UINT32 eid, UINT8 State, UINT8 isWcpeORRcpe)
{
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();

    UINT16 TosNum = 0;
    T_CpeToSCfgEle TosCfgA[MAX_TOS_ELE_NUM]; 
    memset(TosCfgA, 0, sizeof(TosCfgA));
    GetTosInfo(TosCfgA, TosNum, HIGH_TOS_CLASS);
    
    CL3OamCfgBtsRepeaterReq L3OamCfgBtsRepeaterReq;
    UINT32 RepeaterInfoLen = L3OamCfgBtsRepeaterReq.GetDataLenFromNvram();

    UINT32 BTSIDs[NEIGHBOR_BTS_NUM] = {0};
    UINT16 BtsNum = 0;
    UINT16 BtsinfoLen = getBtsIDs(BTSIDs, BtsNum);

    if (false == UpdateReq.CreateMessage(*pTCpeInst/*, MsgLen -2*/))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CreateMessage error, this may cause unexpected results.");
        return false;
        }
    UpdateReq.SetEID(eid);
    //UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
    UpdateReq.SetDstTid(M_TID_CPECM);
    UpdateReq.SetVersion(OAM_DEFAUIT_TRANSID);    
    
    //设置资源管理相关消息内容
    T_RMPoliceEle *pRMPCfgEle = (T_RMPoliceEle*)&(NvRamDataAddr->RMPoliceEle);
    UpdateReq.SetLocAreaId(bspGetLocAreaID()); 
    UpdateReq.SetBWReqInter(pRMPCfgEle->BWReqInterval);
    UpdateReq.SetSessRelThres(pRMPCfgEle->SRelThreshold);
    UpdateReq.SetProfileDataFlag(WITH_PROFILEDATA);  //profile is valide

    //设置缺省profile ie
    T_UTProfile UTProfile;
    memset(&(UTProfile.UTProfileIEBase), 0, sizeof(T_UTProfileIEBase));
    if(isWcpeORRcpe == 1)//如果是wcpe or rcpe,则写入flash
        UTProfile.UTProfileIEBase.isDefaultProfile = 0;//CPE根据这个标志决定是否把profile写入flash.
    else
	UTProfile.UTProfileIEBase.isDefaultProfile = 1;//CPE根据这个标志决定是否把profile写入flash.
    UTProfile.UTProfileIEBase.ucPeriodicalRegisterTimerValue = 1;//修改为1小时jiaying20100920//gDefaultPrdRegTime;
    UTProfile.UTProfileIEBase.AdminStatus = State;
    UTProfile.UTProfileIEBase.VoicePortMask = 1;
    if(isWcpeORRcpe == 1)//如果是wcpe or rcpe,则下发标识,默认带宽
    {
	UTProfile.UTProfileIEBase.UTSDCfgInfo.Reserved |= 0x05;//wcpe且携带保持带宽
	UTProfile.UTProfileIEBase.UTSDCfgInfo.ULMaxBW = 1024;
	UTProfile.UTProfileIEBase.UTSDCfgInfo.ULMinBW = 512;
	UTProfile.UTProfileIEBase.UTSDCfgInfo.DLMaxBW = 1024;
	UTProfile.UTProfileIEBase.UTSDCfgInfo.DLMinBW = 512;
    }
    UpdateReq.setUTProfile(UTProfile);

    UpdateReq.SetQosEntryNum(TosNum); //设置tos相关消息内容
    UpdateReq.SetTosInfo((SINT8 *)&TosCfgA[0] , sizeof(T_ToSCfgEle) * TosNum);
    //设置BTS Repeater 相关消息内容  
    SINT8 RepeaterInfo[200];
    memset(RepeaterInfo, 0, sizeof(RepeaterInfo));
    SINT8* pdata = RepeaterInfo;
    UINT8 temp_len = 0;
    if(BtsNum <= NEIGHBOR_BTS_NUM )
    {
        OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send valid config data to CPE");    
        memcpy(pdata, &BtsNum, 2);
        pdata += 2;
        memcpy(pdata, BTSIDs, BtsNum * sizeof(UINT32));
        pdata += BtsNum * sizeof(UINT32);
        if ( sizeof(UINT16) == RepeaterInfoLen )
        {
            memset(pdata, 0, sizeof(UINT16));
	     temp_len = 2;
        }
        else
        {
            memcpy(pdata, (SINT8 *)&(NvRamDataAddr->BTSRepeaterEle), RepeaterInfoLen );
	    temp_len = RepeaterInfoLen;
        }
        UpdateReq.SetFreqInfo(RepeaterInfo , RepeaterInfoLen + BtsinfoLen);
    }
    else
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send invalide config data to CPE");  
        UpdateReq.SetFreqInfo(RepeaterInfo ,4);
	temp_len = 4;
    }
     
   UINT16 *p;
   if(ut_type == M_CCB_TYPE_NEW)//if v5.2 UT  wangwenhua add 20080716
   {
          UINT8 *pBtsDta = (UINT8 *)(UpdateReq.GetDataPtr()) ;
	    pBtsDta +=UpdateReq.length();
          UINT32 btsid = (bspGetBtsID());
           memcpy(pBtsDta,(SINT8*)&btsid,4);
	   //magic
	    p = (UINT16*)(pBtsDta+4);
	    *(p) = 0x55aa;
	    p += 1;

	    //N_parameter;
	    *p = ( (NvRamDataAddr->N_parameter.UT_PowerLock_Timer_Th & 0xF)<<7 )
	       | ( (NvRamDataAddr->N_parameter.Ci_Jump_detection& 0x1F)<<2 )
	       | ( (NvRamDataAddr->N_parameter.N_Algorithm_Switch&0x1)<<1 )
	       | 0x1;
	    if(isWcpeORRcpe == 1)
	    {
	         p +=1;
	         *p = 512;
                p +=1;
                *p = 512;
                UpdateReq.SetDataLength(UpdateReq.length() + 4 +4+4);
	    }
	    else
		UpdateReq.SetDataLength(UpdateReq.length() + 4 +4);
   }
   else
   {
   	  //magic
	    p = (UINT16*)((UINT8 *)(UpdateReq.GetDataPtr()) + UpdateReq.length());
	    *(p) = 0x55aa;
	    p += 1;

	    //N_parameter;
	    *p = ( (NvRamDataAddr->N_parameter.UT_PowerLock_Timer_Th & 0xF)<<7 )
	       | ( (NvRamDataAddr->N_parameter.Ci_Jump_detection& 0x1F)<<2 )
	       | ( (NvRamDataAddr->N_parameter.N_Algorithm_Switch&0x1)<<1 )
	       | 0x1;
	     if(isWcpeORRcpe == 1)
	     {
                p +=1;
                *p = 512;
                p +=1;
                *p = 512;
                UpdateReq.SetDataLength(UpdateReq.length()+4+4);
	     }
	     else
		 UpdateReq.SetDataLength(UpdateReq.length()+4);
   }
//这里和终端协商，如果是缺省配置，新加的项就不下发了
    return true;
}


UINT16 CPETrans::getBtsIDs(UINT32 *pBTSIDs, UINT16 &BtsNum)
{
    CCfgBtsNeibListReq  CfgBtsNeibListReq;
    CfgBtsNeibListReq.GetNeiberBtsInfo(&BtsNum, pBTSIDs);
    UINT16 BtsinfoLen;
    if(BtsNum > NEIGHBOR_BTS_NUM)
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] NVRAM data error, too many neighbor BTS number[%d]", BtsNum);   
        BtsinfoLen = sizeof(UINT16);
    }
    else
    {
        BtsinfoLen = sizeof(UINT16) + BtsNum * sizeof(UINT32);
    }
    
    return BtsinfoLen;
}

bool CPETrans :: CreateCpeNotifyToEms(CCpeRegistNotify &CpeRegNotify,
                                      CL3L2CpeRegistNotify &L3L2CpeRegNotify
                                      )
{
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
    //计算消息的实际大小
    const T_UTProfile &UTProfile = L3L2CpeRegNotify.getUTProfile();
    UINT16 DataLen = UTProfile.length() + sizeof(T_CpeBaseInfo) + 2/*TransID*/ + 4 /*EID8*/;

    if (false == CpeRegNotify.CreateMessage(*pTCpeInst, DataLen))
        {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CreateMessage error, this may cause unexpected results.");
        return false;
        }

    CpeRegNotify.SetMessageId(M_BTS_EMS_UT_REG_NOTIFY);
    UINT32 eid = L3L2CpeRegNotify.GetEID();
    CpeRegNotify.SetCPEID(eid);
    CpeRegNotify.SetDstTid(M_TID_EMSAGENTTX);
    OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] RPT[0x%08x] send reg to ems", CpeRegNotify.GetEID() );
    //SINT8 RegDataBff[MAX_MESSAGE_SIZE];
    //memset(RegDataBff, 0, sizeof(RegDataBff));
    //UINT16 RegDataLen = L3L2CpeRegNotify.GetUTProfIESize() 
    //                + sizeof(T_CpeBaseInfo);
    //L3L2CpeRegNotify.GetRegData(RegDataBff, RegDataLen);
    //CpeRegNotify.SetRegData(RegDataBff, RegDataLen);
    CpeRegNotify.setCpeBaseInfo(L3L2CpeRegNotify.GetCpeBaseInfo());
    CpeRegNotify.setUTProfile(UTProfile);

    return true;
}

bool CPETrans:: ProfUpdateRspToEms(UINT16 TransId)
{
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
    CL3OamCommonRsp CommonRsp;
    if (false == CommonRsp.CreateMessage(*pTCpeInst))
        return false;

    OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post CPE profile update response to EMS");
    CommonRsp.SetTransactionId(TransId);
    CommonRsp.SetMessageId(M_BTS_EMS_UT_PROFILE_UPDATE_RSP);
    CommonRsp.SetResult(OAM_SUCCESS);
    CommonRsp.SetDstTid(M_TID_EMSAGENTTX);
    if(true != CommonRsp.Post())
    {
        OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] post CPE profile update response to EMS fail");
        CommonRsp.DeleteMessage();
    }

    CTransaction * pTransaction = pTCpeInst->FindTransact(TransId);
    if(pTransaction)
    {
        pTransaction->EndTransact();
        delete pTransaction;
    }

    return true;
}

bool CPETrans :: BeginProfUpdateReqToCpeTrasaction(CL3CpeProfUpdateReq &UpdateReq, CPECCB &ccb, UINT8 RetransCounter)
{
    //为了兼容旧CPE。对Admin_Status做处理
    UpdateReq.validation(ccb.getCCBType());
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
    stopUpdUTProfileTransaction(ccb,0);
    CL3OamCommonRsp FailMsg;
    if (false == FailMsg.CreateMessage(*pTCpeInst))
    {
        UpdateReq.DeleteMessage();
        return false;
    }
    FailMsg.SetEID(UpdateReq.GetEID()); 
    FailMsg.SetDstTid(M_TID_UM);
    FailMsg.SetMessageId(M_OAM_CPE_PROF_UPDATE_FAIL);
    CTransaction *pTrans = pTCpeInst->CreateTransact(UpdateReq, 
                                       FailMsg, 
                                       RetransCounter, 
                                       OAM_REQ_RSP_PERIOD);
    if (NULL == pTrans)
    {        
        UpdateReq.DeleteMessage();
        FailMsg.DeleteMessage();	 
        return false;
    }
    UINT16 TransId = pTrans->GetId();
    UpdateReq.SetTransactionId(TransId);
    FailMsg.SetTransactionId(TransId);
    if(!pTrans->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pTrans->EndTransact();
	 delete pTrans;
	 return false;
    }
    ccb.setUpdUTProfileTransId(TransId);

    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE profile update to CPE[0x%08x], transid:0x%x", UpdateReq.GetEID(), TransId);

    return true;
}

bool CPETrans :: BeginCpeRegNotifyToEmsTrasaction(CCpeRegistNotify &CpeRegNotify)
{
    CTaskCpeM    *pTCpeInst = CTaskCpeM::GetInstance();
    CTransaction *pTrans;  

    CL3OamCommonRsp RegtoEmsFailMsg;
    if (false == RegtoEmsFailMsg.CreateMessage(*pTCpeInst))
    {
        CpeRegNotify.DeleteMessage();
        return false;
    }
    RegtoEmsFailMsg.SetDstTid(M_TID_UM);
    RegtoEmsFailMsg.SetMessageId(M_OAM_CPE_REG_TO_EMS_FAIL);
    pTrans = pTCpeInst->CreateTransact(CpeRegNotify, 
                                       RegtoEmsFailMsg, 
                                       OAM_REQ_RESEND_CNT3, 
                                       OAM_REQ_RSP_PERIOD);
    if ( NULL == pTrans )//如果失败释放处理jiaying20100811
    {
        OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT,  "->BeginCpeRegNotifyToEmsTrasaction:CreatTransact Fail" );
        CpeRegNotify.DeleteMessage();
        RegtoEmsFailMsg.DeleteMessage();
        return false;
    } 
    CpeRegNotify.SetTransactionId(pTrans->GetId());
    RegtoEmsFailMsg.SetTransactionId(pTrans->GetId());
    if(!pTrans->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pTrans->EndTransact();
        delete pTrans;
	 return false;
    }

    OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Send CPE register Notify to EMS.");

    return true;
}


bool CPETrans::sendBWInfoReq(CPECCB &ccb)
{
    CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    CsabisBWInfoReq msgBWInfoReq;
    if (false == msgBWInfoReq.CreateMessage(*pTaskCpeM))
        return false;
	//wangwenhua modify 20080703 in order to send pid 0xffffffff to sag for old CPE ,thus HLR to judge new Ut or not 
	if((ccb.getCCBType()) == M_CCB_TYPE_OLD )
	{
       	 msgBWInfoReq.setInfo(ccb.getUid(), 0xffffffff, ccb.getCpeBaseInfo());
	}
	else
	{
	 	msgBWInfoReq.setInfo(ccb.getUid(), ccb.getEid(), ccb.getCpeBaseInfo());	
	}
    msgBWInfoReq.SetDstTid(M_TID_VCR);
#if 1
    stopUpdUTProfileTransaction(ccb,1);
    CL3OamCommonRsp msgFail;
    if (false == msgFail.CreateMessage(*pTaskCpeM))
        {
        msgBWInfoReq.DeleteMessage();
        return false;
        }
    msgFail.SetDstTid(M_TID_UM);
    msgFail.SetMessageId(M_OAM_CPE_REG_TO_EMS_FAIL);
    msgFail.SetEID(ccb.getEid());
    CTransaction *pTrans = pTaskCpeM->CreateTransact(msgBWInfoReq,
                                       msgFail, 
                                       OAM_REQ_RESEND_CNT3, 
                                       OAM_REQ_RSP_PERIOD);
    if(NULL == pTrans)
    {
        msgBWInfoReq.DeleteMessage();
        msgFail.DeleteMessage();
        return false;
    }
////msgBWInfoReq.SetTransactionId(pTrans->GetId());
    msgFail.SetTransactionId(pTrans->GetId());
    if(false == pTrans->BeginTransact())
    {
        pTrans->EndTransact();       
        delete pTrans;
        return false;
    }
    ccb.setBWInfoTransId(pTrans->GetId());
#else
    if(false == msgBWInfoReq.Post())
        {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send CPE[0x%08x] BWInfo_Req FAIL", ccb.getEid());
        //tVCR postMessage will destroy the message.
        //msgRsp.DeleteMessage();
        return false;
        }
#endif
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Send CPE[0x%.8X] BWInfo_Req to HLR[trans:0x%x].", ccb.getEid(), pTrans->GetId());

    return true;
}


bool CPETrans::stopBWInfoTransaction(CPECCB &ccb)
{
    UINT16 usTransID = ccb.getBWInfoTransId();
    if(0 == usTransID)
        return true;
    CTransaction *pTransaction = CTaskCpeM::GetInstance()->FindTransact(usTransID);
    if(NULL != pTransaction)
        {
        pTransaction->EndTransact();
        delete pTransaction;
        }
    ccb.setBWInfoTransId(0);
    OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] stop CPE[0x%.8X] BWInfo_Req transaction[0x%X].", ccb.getEid(), usTransID);
    return true;
}


bool CPETrans::stopUpdUTProfileTransaction(CPECCB &ccb,UINT8 flag)
{
    UINT16 usTransID ;
if(flag==0)
{
   usTransID= ccb.getUpdUTProfileTransId();
}
else
{
     usTransID= ccb.getBWInfoTransId();
}
   if(0 == usTransID)
      return true;
    CTransaction * pTransaction = CTaskCpeM::GetInstance()->FindTransact(usTransID);
    if(!pTransaction)
    {
        OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] can not find CPE[0x%.8X] Profile Update transaction; transid[0x%x]", ccb.getEid(), usTransID);
        //2007-4-27如果transaction被停止后立刻创建了新的transaction.
        //这个时候不能改称0
        //ccb.SetUpdateTransId(0X0000);
    }
    else
    {
        pTransaction->EndTransact();
        delete pTransaction;
        OAM_LOGSTR2(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] stop CPE[0x%.8X] Profile Update transaction[0x%x].", ccb.getEid(), usTransID);
	if(flag==0)
	{
        ccb.setUpdUTProfileTransId(0);
	}
	else
	{
	   ccb.setBWInfoTransId(0);
	}
    }
    return true;
}

bool CPETrans::sendModifyBWInfoRsp(const CPECCB &ccb)
{
    CsabisModifyBWInfoRsp msgRsp;
    if(false == msgRsp.CreateMessage(*CTaskCpeM::GetInstance()))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] create CPE[0x%08x] Modify_BWInfo_Rsp FAIL. [IDLE state]", ccb.getEid());
        return false;
        }
    msgRsp.setInfo(ccb.getUid());
    msgRsp.SetDstTid(M_TID_VCR);
    if(false == msgRsp.Post())
        {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send CPE[0x%08x] Modify_BWInfo_Rsp FAIL", ccb.getEid());
        msgRsp.DeleteMessage();
        return false;
        }
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send CPE[0x%08x] Modify_BWInfo_Rsp to HLR SUCCESS", ccb.getEid());
    return true;
}


#if 0
bool CPETrans :: IsProfDataSame(SINT8 *ccbData,  SINT8 *MsgDate, UINT16 Len)//判断请求消息重的profile是否同ccb保存的相同    
{
    if((NULL == ccbData)||(NULL == MsgDate))
    {
        return false;
    }

    for(UINT16 Index = 0; Index < Len; Index++, ccbData++, MsgDate++)
    {
        if(*ccbData != *MsgDate)
        {
            return false;
        }
    }

    return true;
}
#endif
//20100117,add by fengbing begin
extern "C" bool ifCpeCanAccess(UINT32 pid, UINT32 uid);
//20100117,add by fengbing end
//老的注册流程
FSMStateIndex CPETrans::RegisterProcedure(CPECCB &ccb, CL3L2CpeRegistNotify &msgL3L2CpeRegNotify)
{
    //20100117,add by fengbing begin
    UINT32 pid_tmp   = msgL3L2CpeRegNotify.GetEID();
    UINT32 uid_tmp = pid_tmp;
    //20100117,add by fengbing end

	
	//#ifdef RPT_FOR_V6
	T_CpeBaseInfo baseInfo = msgL3L2CpeRegNotify.GetCpeBaseInfo();
    if( RPTHW_TYPE_3B == baseInfo.HardwareType || RPTHW_TYPE_3C == baseInfo.HardwareType )
    {
        UINT32 eid   = msgL3L2CpeRegNotify.GetEID();
        UINT8  Valid = msgL3L2CpeRegNotify.GetDataValid();
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid,MAX_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid, MAX_CPE_REMANENT_CYCLE);
#endif
     
        ccb.setRptFlag(1);
        if(DATA_VALID == Valid) //true
        {
            if(true == ccb.getRegisterStatus(msgL3L2CpeRegNotify.GetTransactionId()))
            {
                 OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is valid.", eid );
                 ccb.setEid(eid);
                 ccb.setUid(eid);
                 ccb.setUTBaseInfo(msgL3L2CpeRegNotify.GetCpeBaseInfo());
                 ccb.setUTProfile(msgL3L2CpeRegNotify.getUTProfile());
                 T_L2SpecialFlag flag;
                 msgL3L2CpeRegNotify.getSpecialFlag(flag);
                 ccb.setSpecialFlag(flag);		
                 T_UT_HOLD_BW bw;	
                 //wangwenhua add 20080916
                 bw.DL_Hold_BW = 0;
                 bw.UL_Hold_BW = 0;
                msgL3L2CpeRegNotify.getUTHold( bw);
                ccb.setUTHoldBW(bw);
                OAM_LOGSTR3(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register request BW:%x,%x.", eid, bw.UL_Hold_BW,bw.DL_Hold_BW );
                //( 2  创建transaction向cpe发送profile更新消息
                CL3CpeProfUpdateReq UpdateReq;
                CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 0,eid); 
                BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                //向cpe下发切换参数
                SendHandoverParaToCpe(eid);
                SendValidFreqsParaToCpe(eid);
                //( 3  创建trancaction向 EMS 发送cpe注册指示消息
                CCpeRegistNotify CpeRegNotify;
                CreateCpeNotifyToEms(CpeRegNotify, msgL3L2CpeRegNotify);
                BeginCpeRegNotifyToEmsTrasaction(CpeRegNotify);
                
                SendProfDatatoOther(ccb, 1);
			
                T_UTProfile UTProfile = msgL3L2CpeRegNotify.getUTProfile();
                if((UTProfile.UTProfileIEBase.AdminStatus==0)||(UTProfile.UTProfileIEBase.AdminStatus==0x10))
                SendEidMoneyStatustoEbCdr(eid,UTProfile.UTProfileIEBase.AdminStatus);

            }
            else
            {
                OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register request is discarded.", eid );
            }
            return CPE_SERVING;
        }
        else  //向cpe发送缺省配置消息     
        {
            OAM_LOGSTR1(LOG_CRITICAL, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID. ", eid );
            
            // 修改rCCB的baseInfo
            ccb.setUTBaseInfo(msgL3L2CpeRegNotify.GetCpeBaseInfo());
            ccb.setEid(eid);
            
            //( 1  创建transaction向cpe发送profile更新消息     
            CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
            CreateDefaultProfUpdateReqToCpe(UpdateReq,0, eid, CPE_ADM_STATUS_ADM);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            
            //( 2  创建trancaction向 EMS 发送cpe注册指示消息
            if(true == ccb.getRegisterStatus(msgL3L2CpeRegNotify.GetTransactionId()))
            {
                CCpeRegistNotify CpeRegNotify;
                CreateCpeNotifyToEms(CpeRegNotify, msgL3L2CpeRegNotify);
                BeginCpeRegNotifyToEmsTrasaction(CpeRegNotify);
            }
            //故障弱化时不启动定时器jy20100310
            if((sagStatusFlag == false)&&(TRUE== bspGetIsPermitUseWhenSagDown()))   
            {
                OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID and SAG is down. ", eid );
                return CPE_SERVING; 
            }
            if (NULL != ccb.m_pCpeRegTimer)
            {
                ccb.m_pCpeRegTimer->Stop();
                delete ccb.m_pCpeRegTimer;
                ccb.m_pCpeRegTimer = NULL;
            }
            CTimer *timer = createRegisterTimer(eid);
            ccb.m_pCpeRegTimer = timer;
            return CPE_WAITFOREMS; 
        }
    
    }
    else
    {     
		//#ifdef RPT_FOR_V6
		
        if(false == ccb.getRegisterStatus(msgL3L2CpeRegNotify.GetTransactionId()))
        {
            return ccb.GetCurrentState();
        }
        UINT32 eid   = msgL3L2CpeRegNotify.GetEID();
        ccb.setEid(eid);
        ccb.setUid(eid);
	 
        UINT8  Valid = msgL3L2CpeRegNotify.GetDataValid();
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid,MAX_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid, MAX_CPE_REMANENT_CYCLE);
#endif
    
        UINT8 wcpe_rcpe_flag = 0;
        #ifdef M_TGT_WANIF
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT,"[tCpeM]CPE[0x%.8X] judge wcpe or rcpe.  ", eid);
        //如果是wcpe设置为1
        if((Wanif_Switch== 0x5a5a)&&((NvRamDataAddr->WanIfCpeEid==eid)||(NvRamDataAddr->BakWanIfCpeEid==eid)))
        {
            OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT,"[tCpeM]CPE[0x%.8X] this is wcpe.  ", eid);
            wcpe_rcpe_flag = 1;	    
        }
        else
        {
            //如果在rcpe中找到该eid则设置为1
            if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
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
        if(wcpe_rcpe_flag==1)
            ccb.setWcpeORRcpeFlag(1);
        else
            ccb.setWcpeORRcpeFlag(0);//清0,防止上次是,这次不是,但标记没有清
        //如果终端上报为wcpe or rcpe, 但不是本基站的,发送注册失败消息
        if(Enable_Wcpe_Flag_Judge == (UINT32)0xeabdeabd)
        {
            OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]judge wcpe flag enabled [0x%.8X] .", Enable_Wcpe_Flag_Judge);
            if((msgL3L2CpeRegNotify.isWCPEorRCPE()==true)&&(wcpe_rcpe_flag!=1))
            {
                OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%.8X] this is wcpe or rcpe, but not in this bts.", eid);
//                CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
                ccb.setAdminStatus(0xff);       
                
                CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
                CreateDefaultProfUpdateReqToCpe(UpdateReq,0, eid, 0xff);
                BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                return  CPE_NORECORD;
            }
        }
        #endif
	 if((!ifCpeCanAccess(pid_tmp, uid_tmp))&&(wcpe_rcpe_flag!=1))
        {
            OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE eid[0x%.8X] uid[0x%.8X] is not allowed to access.", pid_tmp, uid_tmp);
       //     CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
            ccb.setAdminStatus(0xff);
            
            CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
            CreateDefaultProfUpdateReqToCpe(UpdateReq,0, pid_tmp, CPE_ADM_STATUS_INV);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            return	CPE_NORECORD;		
        }
	 //如果sag失连并且 故障弱化开关关闭，则不允许非rcpe/wcpe终端接入jiaying20100720
        if((sagStatusFlag == false)&&(FALSE== bspGetIsPermitUseWhenSagDown())&&(wcpe_rcpe_flag!=1)) 
        {
            OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE [0x%.8X] can't access, cause SAG is down, and not permit UT reg", eid);
      //      CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
            ccb.setAdminStatus(CPE_ADM_STATUS_SAG_DOWN);
            
            CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
            CreateDefaultProfUpdateReqToCpe(UpdateReq,0, eid, CPE_ADM_STATUS_SAG_DOWN);//sag失连
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
	     SendDisableToDM_Voice(ccb, M_CPE_L3_REG_NOTIFY, eid);//删除DM,voice, l2数据
            return  CPE_NORECORD;		
        }
        int sendRegisterRsp = 0;
        //非故障弱化情况下，给wcpe/rcpe发送注册应答
        if((sagStatusFlag == false)&&(FALSE== bspGetIsPermitUseWhenSagDown())&&(wcpe_rcpe_flag==1))
        {
            sendRegisterRsp = 1;
        }
        //故障弱化情况下给终端发送注册应答
        if((sagStatusFlag == false)&&(TRUE== bspGetIsPermitUseWhenSagDown()))
        {
            sendRegisterRsp = 1;
        }
        if(DATA_VALID == Valid) //true
        {
            OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is valid.", eid );
            if(sagStatusFlag == false)//当终端是故障弱化上来的，记录标志
                ccb.setSagDefaultFlag(1);
            ccb.setUTBaseInfo(msgL3L2CpeRegNotify.GetCpeBaseInfo());
            ccb.setUTProfile(msgL3L2CpeRegNotify.getUTProfile());
            
            T_L2SpecialFlag flag;
            msgL3L2CpeRegNotify.getSpecialFlag(flag);
            ccb.setSpecialFlag(flag);
            T_UT_HOLD_BW bw;
            
            //wangwenhua add 20080916
            
            bw.DL_Hold_BW = 0;
            bw.UL_Hold_BW = 0;
            msgL3L2CpeRegNotify.getUTHold( bw);
            ccb.setUTHoldBW(bw);	 
            OAM_LOGSTR3(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register request BW:%x,%x.", eid, bw.UL_Hold_BW,bw.DL_Hold_BW );
            if(sendRegisterRsp==1)
            {
                OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] SAG is down, and permit use, send register rsp to CPE[0x%08x].", eid );
                //( 2  创建transaction向cpe发送profile更新消息
                CL3CpeProfUpdateReq UpdateReq;
                #ifdef M_TGT_WANIF
                if(wcpe_rcpe_flag == 0)
                    CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 0,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
                else
                {
                    if((msgL3L2CpeRegNotify.isWCPEorRCPE()==false))//终端不知道是,下发原配置
                    {            	
                        CreateProfUpdateReqToCpe(UpdateReq, ccb);	     
                    }
                    //终端已经知道是wcpe则下发不带profle的
                    else
                        CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 0,eid);	
                }
                #else
                CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 0,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
                #endif
                //CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 0,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
                BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                //向cpe下发切换参数
                SendHandoverParaToCpe(eid);
                SendValidFreqsParaToCpe(eid);
                SendProfDatatoOther(ccb, 1);//profile is valid
                T_UTProfile UTProfile = msgL3L2CpeRegNotify.getUTProfile();
                if((UTProfile.UTProfileIEBase.AdminStatus==0)||(UTProfile.UTProfileIEBase.AdminStatus==0x10))
                    SendEidMoneyStatustoEbCdr(eid,UTProfile.UTProfileIEBase.AdminStatus);
                
            }
            //( 3  创建trancaction向 HLR 发送cpe注册指示消息            
            sendBWInfoReq(ccb);                        
            return CPE_SERVING;
        }
        else  //向cpe发送缺省配置消息     
        {
            OAM_LOGSTR1(LOG_CRITICAL, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID. ", eid );
            if(sagStatusFlag == false)//当终端是故障弱化上来的，记录标志
                ccb.setSagDefaultFlag(1);
            // 修改rCCB的baseInfo
            ccb.setUTBaseInfo(msgL3L2CpeRegNotify.GetCpeBaseInfo());
            ccb.setEid(eid);
            ccb.setProfileInvalidFlag(1);
            if(sendRegisterRsp==1)
            {
                OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] SAG is down, and permit use, send register rsp to CPE[0x%08x].", eid );
                T_UTProfile UTProfile;
                UTProfile = ccb.getUTProfile();         
                UTProfile.UTProfileIEBase.VoicePortMask = 1;
                UTProfile.UTProfileIEBase.AdminStatus = 0;
                UTProfile.UTProfileIEBase.LogStatus = 0;
                UTProfile.UTProfileIEBase.Mobility = 1;
                UTProfile.UTProfileIEBase.DHCPRenew = 1;
                UTProfile.UTProfileIEBase.WLANID = 0;
                UTProfile.UTProfileIEBase.MaxIpNum = 20;
                UTProfile.UTProfileIEBase.FixIpNum = 0;
                ccb.setUTProfile(UTProfile);
                
                //( 1  创建transaction向cpe发送profile更新消息     
                CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
                #ifdef M_TGT_WANIF
                if(wcpe_rcpe_flag == 0)
                {
                    CreateDefaultProfUpdateReqToCpe(UpdateReq, 0,eid, CPE_ADM_STATUS_ADM);
                    SendProfDatatoOther(ccb, 1);
                }
                else
                {
                    CreateDefaultProfUpdateReqToCpe(UpdateReq, 0, eid, CPE_ADM_STATUS_ADM, 1);
                    SendProfDatatoOther(ccb, 0);//profile is invalid
                }
                #else
                {
                    CreateDefaultProfUpdateReqToCpe(UpdateReq, 0,eid, CPE_ADM_STATUS_ADM);
                    SendProfDatatoOther(ccb, 1);
                }
                #endif
                //CreateDefaultProfUpdateReqToCpe(UpdateReq,0, eid, CPE_ADM_STATUS_ADM);
                BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
                //向cpe下发切换参数
                SendHandoverParaToCpe(eid); 
                SendValidFreqsParaToCpe(eid);
                
                SendEidMoneyStatustoEbCdr(eid,0);
            }
            //( 2  创建trancaction向 HLR 发送cpe注册指示消息           
            sendBWInfoReq(ccb);/*为了让用户在sag失连情况下也可以使用*/
            
            //故障弱化时不启动定时器jy20100310
            if((sagStatusFlag == false)&&(TRUE== bspGetIsPermitUseWhenSagDown()))   
            {
                OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID and SAG is down. ", eid );
                return CPE_SERVING; 
            }
            #ifdef M_TGT_WANIF
            if(wcpe_rcpe_flag == 0)//wcpe rcpe 不加定时器
            {
                if (NULL != ccb.m_pCpeRegTimer)
                {
                    ccb.m_pCpeRegTimer->Stop();
                    delete ccb.m_pCpeRegTimer;
                    ccb.m_pCpeRegTimer = NULL;
                }
                CTimer *timer = createRegisterTimer(eid);
                ccb.m_pCpeRegTimer = timer;
            }
            #else
            if (NULL != ccb.m_pCpeRegTimer)
            {
                ccb.m_pCpeRegTimer->Stop();
                delete ccb.m_pCpeRegTimer;
                ccb.m_pCpeRegTimer = NULL;
            }
            CTimer *timer = createRegisterTimer(eid);
            ccb.m_pCpeRegTimer = timer;
            #endif
            return CPE_WAITFOREMS; 
        }
    }
}


CTimer* CPETrans::createRegisterTimer(UINT32 eid)
{
    ///////////////////////////////////////////////////////////
    CL3OamCommonReq TimerMsg;
    CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    if (false == TimerMsg.CreateMessage(*pTaskCpeM))
        return NULL;

    TimerMsg.SetEID(eid);
    TimerMsg.SetDstTid(M_TID_UM);
    TimerMsg.SetSrcTid(M_TID_UM);
    TimerMsg.SetMessageId(M_OAM_CPE_REGISTER_TIMER);
    CTimer *timer = new CTimer(M_TIMER_PERIOD_NOT, CPE_REGISTER_TIMER_PERIOD, TimerMsg);
    if (NULL == timer) 
        {
        TimerMsg.DeleteMessage();
        return NULL;
        }

    timer->Start();
    return timer;
}


//新的注册流程
FSMStateIndex CPETrans::RegisterReq(CPECCB &ccb, const CUTRegisterReq &msgUTRegisterReq)
{
//#ifdef RPT_FOR_V6
	const T_RegisterReq *const pRegisterReq1 = msgUTRegisterReq.getInfo();
	//if( pRegisterReq1->UTBaseInfo.usHWtype )
   if( RPTHW_TYPE_3B == pRegisterReq1->UTBaseInfo.usHWtype || RPTHW_TYPE_3C == pRegisterReq1->UTBaseInfo.usHWtype )
   	{
		UINT32 eid1 = pRegisterReq1->ulPID;
		UINT32 uid1 = pRegisterReq1->ulUID;
		UINT32 btsid1 = msgUTRegisterReq.getBTSID();
		if(btsid1!=bspGetBtsID())//wangwenhua add 20080716
		{
			OAM_LOGSTR3(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register req carrys btsid error, discard.%x,%x", eid1 ,btsid1,bspGetBtsID());
			return ccb.GetCurrentState();
		}
		ccb.setEid(eid1);
		ccb.setUid(uid1);
		bool valid1 = msgUTRegisterReq.isValid();
		////CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MAX_CPE_REMANENT_CYCLE);
		if(true == valid1)
		{
			OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is valid.", eid1 );
	        //过滤突发性的CPE注册请求,系统有时候1s内接受到从同一CPE上来的很多注册请求
	        //( 1  获取消息获得消息中的的 ProfileIE
			ccb.setUTBaseInfo(pRegisterReq1->UTBaseInfo);
			ccb.setUTProfile(pRegisterReq1->UTProfile);

			T_L2SpecialFlag flag;
			msgUTRegisterReq.getSpecialFlag(flag);
			ccb.setSpecialFlag(flag);
			T_UT_HOLD_BW bw;

			//wangwenhua add 20080916
			bw.DL_Hold_BW = 0;
			bw.UL_Hold_BW = 0;
			msgUTRegisterReq.getUTHold( bw);
			ccb.setUTHoldBW(bw);  
			T_MEM_CFG mem_temp;
			msgUTRegisterReq.getMemCfg(mem_temp);
			ccb.setMemCfg(mem_temp);

	        //( 2  创建transaction向cpe发送profile更新消息
			CL3CpeProfUpdateReq UpdateReq;
			CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 1,eid1);    //?á?é2¨￠¨?|??e????é2????ProfileIE. ?à¨a¨o?2????à?CPE|ì?profileIE.
			//UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
			UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
			BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);

			sendRptReqToEms(pRegisterReq1);
	        //向cpe下发切换参数
	        //ljf alter ******************************************************
			//SendHandoverParaToCpe(eid);
		    //sendBWInfoReq(ccb);/*为了让用户在sag失连情况下也可以使用*/
			//SendProfDatatoOther(ccb, 1);//profile is valid
			//UpdateReqTemp.DeleteMessage();

			//T_UTProfile UTProfile = pRegisterReq->UTProfile;
			//if((UTProfile.UTProfileIEBase.AdminStatus==0)||(UTProfile.UTProfileIEBase.AdminStatus==0x10))
			//	SendEidMoneyStatustoEbCdr(eid,UTProfile.UTProfileIEBase.AdminStatus);

			//CTaskCpeM::CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);//?¤??¨?*2

			return CPE_SERVING;
		}
	    else  //向cpe发送缺省配置消息     
		{
			OAM_LOGSTR1(LOG_CRITICAL, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID. ", eid1 );

	        // DT??rCCBμ?baseInfo
			ccb.setUTBaseInfo(pRegisterReq1->UTBaseInfo);
			T_UTProfile UTProfile;
			UTProfile = ccb.getUTProfile();     
			UTProfile.UTProfileIEBase.VoicePortMask = 1;
			ccb.setUTProfile(UTProfile);
			ccb.setProfileInvalidFlag(1);
	        //( 1  ′′?¨transaction?òcpe·￠?íprofile?üD????￠     
			CL3CpeProfUpdateReq UpdateReq;
			//CUpdateUTBWInfo UpdateReq;
			CreateDefaultProfUpdateReqToCpe(UpdateReq, 1,eid1, CPE_ADM_STATUS_ADM);
	//*#ifdef M_TGT_WANIF
	//*		if(wcpe_rcpe_flag == 0)
	//*		{
	//*			CreateDefaultProfUpdateReqToCpe(UpdateReq, 1,eid, CPE_ADM_STATUS_ADM);	
	//*			SendProfDatatoOther(ccb, 1);//profile is invalid	
	//*		}
	//*		else
	//*		{
	//*			CreateDefaultProfUpdateReqToCpe(UpdateReq, 1, eid, CPE_ADM_STATUS_ADM, 1);
	//*			SendProfDatatoOther(ccb, 0);//profile is invalid	
	//*		}        
	//*#else
			//CreateDefaultProfUpdateReqToCpe(UpdateReq, 1,eid1, CPE_ADM_STATUS_ADM);
			SendProfDatatoOther(ccb, 1);//profile is invalid	
	//*#endif
			//UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
			UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
			BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);

	        //( 2  创建trancaction向 EMS 发送cpe注册指示消息
			//CCpeRegistNotify CpeRegNotify;
			//CreateCpeNotifyToEms(CpeRegNotify, msgUTRegisterReq);
			//BeginCpeRegNotifyToEmsTrasaction(CpeRegNotify);
	//*#if 0
	//*		if (false == sendBWInfoReq(ccb))
	//*		{
	//*			OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send CPE[0x%08x] BWInfo_Req FAIL.", eid );
	//*			return ccb.GetCurrentState();
	//*		}
	//*#endif
		/*为了让用户在sag失连情况下也可以使用*/
			sendBWInfoReq(ccb);	
		//故障弱化时不启动定时器jy20100310
			if((sagStatusFlag == false)&&(TRUE== bspGetIsPermitUseWhenSagDown()))   
			{
				OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID and SAG is down. ", eid1 );
				return CPE_SERVING; 
			}
	//*#ifdef M_TGT_WANIF
	//*       if(wcpe_rcpe_flag == 0)//wcpe rcpe 不加定时器
	//*		{
	//*			if (NULL != ccb.m_pCpeRegTimer)
	//*			{
	//*				ccb.m_pCpeRegTimer->Stop();
	//*				delete ccb.m_pCpeRegTimer;
	//*				ccb.m_pCpeRegTimer = NULL;
	//*			}
	//*			CTimer *timer = createRegisterTimer(eid);
	//*			ccb.m_pCpeRegTimer = timer;
	//*		}
	//*#else
			if (NULL != ccb.m_pCpeRegTimer)
			{
				ccb.m_pCpeRegTimer->Stop();
				delete ccb.m_pCpeRegTimer;
				ccb.m_pCpeRegTimer = NULL;
			}
			CTimer *timer = createRegisterTimer(eid1);
			ccb.m_pCpeRegTimer = timer;
	//*#endif
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid1,MIN_CPE_REMANENT_CYCLE);
#else
    CPE_AddDeleteCpeInfo(eid1, MIN_CPE_REMANENT_CYCLE);
#endif
			

			return CPE_WAITFOREMS; 
		}
   	}
    bool wcpe_rcpe_flag =0;
    if(false == ccb.getRegisterStatus(msgUTRegisterReq.GetTransactionId()))
    {
        return ccb.GetCurrentState();
    }
    const T_RegisterReq* const pRegisterReq = msgUTRegisterReq.getInfo();
    UINT32 eid = pRegisterReq->ulPID;
    UINT32 uid = pRegisterReq->ulUID;
    
    //20100117,add by fengbing end
    UINT32 btsid = msgUTRegisterReq.getBTSID();
    if(btsid!=bspGetBtsID())//wangwenhua add 20080716
    {
        OAM_LOGSTR3(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register req carrys btsid error, discard.%x,%x", eid ,btsid,bspGetBtsID());
        return ccb.GetCurrentState();
    }
    if (0 == uid)
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] register req carries no UID, discard.", eid );        
        return CPE_NORECORD;
    }
    ccb.setEid(eid);
    ccb.setUid(uid);
    #ifdef M_TGT_WANIF
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT,"[tCpeM]CPE[0x%.8X] judge wcpe or rcpe.  ", eid);
    //如果是wcpe设置为1
    if((Wanif_Switch== 0x5a5a)&&((NvRamDataAddr->WanIfCpeEid==eid)||(NvRamDataAddr->BakWanIfCpeEid==eid)))
    {
        OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT,"[tCpeM]CPE[0x%.8X] this is wcpe.  ", eid);
        wcpe_rcpe_flag = 1;	    
    }
    else
    {
        //如果在rcpe中找到该eid则设置为1
        if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
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
    if(wcpe_rcpe_flag==1)
        ccb.setWcpeORRcpeFlag(1);    
    else
        ccb.setWcpeORRcpeFlag(0);//清0,防止上次是,这次不是,但标记没有清
    //如果终端上报为wcpe or rcpe, 但不是本基站的,发送注册失败消息
    if(Enable_Wcpe_Flag_Judge == (UINT32)0xeabdeabd)
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]judge wcpe flag enabled [0x%.8X] .", Enable_Wcpe_Flag_Judge);
        
        if((msgUTRegisterReq.isWCPEorRCPE()==true)&&(wcpe_rcpe_flag!=1))
        {
            OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%.8X] this is wcpe or rcpe, but not in this bts.", ccb.getEid());
//            CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
            ccb.setAdminStatus(0x0d);
            
            CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
            CreateDefaultProfUpdateReqToCpe(UpdateReq, ccb.getCCBType(),ccb.getEid(), 0x0d);   
            UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            return  CPE_NORECORD;
        }
    }
    #endif
    //20100117,add by fengbing begin
    if((!ifCpeCanAccess(eid, uid))&&(wcpe_rcpe_flag!=1))
    {
        OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE pid[0x%.8X] uid[0x%.8X] is not allowed to access.", eid, uid);
    //    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
        ccb.setAdminStatus(0x0d);
        
        CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
        CreateDefaultProfUpdateReqToCpe(UpdateReq, ccb.getCCBType(),ccb.getEid(), CPE_ADM_STATUS_NOROAM_BID);   
        UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
        BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);	 
        return  CPE_NORECORD;		
    }
    //如果sag失连并且 故障弱化开关关闭，则不允许非rcpe/wcpe终端接入jiaying20100720
    if((sagStatusFlag == false)&&(FALSE== bspGetIsPermitUseWhenSagDown())&&(wcpe_rcpe_flag!=1))   
    {        
        OAM_LOGSTR2(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE pid[0x%.8X] uid[0x%.8X] can't access, cause SAG is down, and not permit UT reg", eid, uid);
    //    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();
        ccb.setAdminStatus(CPE_ADM_STATUS_SAG_DOWN);
        
        CL3CpeProfUpdateReq UpdateReq;//用请求消息的内容发送注册应答消息;
        CreateDefaultProfUpdateReqToCpe(UpdateReq, ccb.getCCBType(),ccb.getEid(), CPE_ADM_STATUS_SAG_DOWN); //sag失连 
        UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
        BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
	 SendDisableToDM_Voice(ccb, M_CPE_L3_REGISTER_REQ, eid);//删除DM,voice, l2数据
        return  CPE_NORECORD;	
    }
    bool valid = msgUTRegisterReq.isValid();
    ////CTaskCpeM::CPE_AddDeleteCpeInfo(eid, MAX_CPE_REMANENT_CYCLE);
    int sendRegisterRsp = 0;
    //非故障弱化情况下，给wcpe/rcpe发送注册应答
    if((sagStatusFlag == false)&&(FALSE== bspGetIsPermitUseWhenSagDown())&&(wcpe_rcpe_flag==1))
    {
        sendRegisterRsp = 1;
    }
    //故障弱化情况下给终端发送注册应答
    if((sagStatusFlag == false)&&(TRUE== bspGetIsPermitUseWhenSagDown()))
    {
        sendRegisterRsp = 1;
    }    
    if(true == valid)
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is valid.", eid );
        //过滤突发性的CPE注册请求,系统有时候1s内接受到从同一CPE上来的很多注册请求
        //( 1  获取消息获得消息中的的 ProfileIE
        ccb.setUTBaseInfo(pRegisterReq->UTBaseInfo);
        ccb.setUTProfile(pRegisterReq->UTProfile);
        if(sagStatusFlag == false)//当终端是故障弱化上来的，记录标志
            ccb.setSagDefaultFlag(1);
        T_L2SpecialFlag flag;
        msgUTRegisterReq.getSpecialFlag(flag);
        ccb.setSpecialFlag(flag);
        T_UT_HOLD_BW bw;
        
        //wangwenhua add 20080916
        
        bw.DL_Hold_BW = 0;
        bw.UL_Hold_BW = 0;
        msgUTRegisterReq.getUTHold( bw);
        ccb.setUTHoldBW(bw);  
        T_MEM_CFG mem_temp;
        msgUTRegisterReq.getMemCfg(mem_temp);
        ccb.setMemCfg(mem_temp);
       
        if(sendRegisterRsp==1)
        {
            OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] SAG is down, and permit use, send register rsp to CPE[0x%08x].", eid );
            //( 2  创建transaction向cpe发送profile更新消息
            CL3CpeProfUpdateReq UpdateReq;
            //CUpdateUTBWInfo UpdateReq;
            #ifdef M_TGT_WANIF
            if(wcpe_rcpe_flag == 0)
                CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 1,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
            else
            {  
                if(msgUTRegisterReq.isWCPEorRCPE()==false)//终端不知道是,下发原配置
                {            	
                    CreateProfUpdateReqToCpe(UpdateReq, ccb);	     
                }
                //终端已经知道是wcpe则下发不带profle的
                else
                    CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 1,eid);
            }
            #else
                CreateNoPrifileIEProfUpdateReqToCpe(UpdateReq, 1,eid);    //注册应答消息不带ProfileIE. 表示不改变CPE的profileIE.
            #endif
            //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
            UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            //向cpe下发切换参数
            SendHandoverParaToCpe(eid);
            SendValidFreqsParaToCpe(eid);
            SendProfDatatoOther(ccb, 1);//profile is valid
            //UpdateReqTemp.DeleteMessage();
            
            T_UTProfile UTProfile = pRegisterReq->UTProfile;
            if((UTProfile.UTProfileIEBase.AdminStatus==0)||(UTProfile.UTProfileIEBase.AdminStatus==0x10))
            SendEidMoneyStatustoEbCdr(eid,UTProfile.UTProfileIEBase.AdminStatus);
        }
        //( 3  创建trancaction向 HLR 发送cpe注册指示消息
              
        sendBWInfoReq(ccb);/*为了让用户在sag失连情况下也可以使用*/        
        //修改注册应答机制，改为由hlr应答 20120307        
        
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid,ccb.getPrdRegTimeValue() * 60 * 2);
#else
    CPE_AddDeleteCpeInfo(eid, ccb.getPrdRegTimeValue() * 60 * 2);
#endif

        
        return CPE_SERVING;
    }
    else  //向cpe发送缺省配置消息     
    {
        OAM_LOGSTR1(LOG_CRITICAL, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID. ", eid );
        if(sagStatusFlag == false)//当终端是故障弱化上来的，记录标志
            ccb.setSagDefaultFlag(1);
        ccb.setProfileInvalidFlag(1);
        //需要发送注册应答
        if(sendRegisterRsp==1)
        {
            OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] SAG is down, and permit use, send register rsp to CPE[0x%08x].", eid );
            // 修改rCCB的baseInfo
            ccb.setUTBaseInfo(pRegisterReq->UTBaseInfo);
            T_UTProfile UTProfile;
            UTProfile = ccb.getUTProfile();     
            UTProfile.UTProfileIEBase.VoicePortMask = 1;
            UTProfile.UTProfileIEBase.AdminStatus = 0;
            UTProfile.UTProfileIEBase.LogStatus = 0;
            UTProfile.UTProfileIEBase.Mobility = 1;
            UTProfile.UTProfileIEBase.DHCPRenew = 1;
            UTProfile.UTProfileIEBase.WLANID = 0;
            UTProfile.UTProfileIEBase.MaxIpNum = 20;
            UTProfile.UTProfileIEBase.FixIpNum = 0;
            ccb.setUTProfile(UTProfile);
            
            //( 1  创建transaction向cpe发送profile更新消息     
            CL3CpeProfUpdateReq UpdateReq;
            //CUpdateUTBWInfo UpdateReq;
            //CreateDefaultProfUpdateReqToCpe(UpdateReq, 1,eid, CPE_ADM_STATUS_ADM);
            #ifdef M_TGT_WANIF
            if(wcpe_rcpe_flag == 0)
            {
                CreateDefaultProfUpdateReqToCpe(UpdateReq, 1,eid, CPE_ADM_STATUS_ADM);	
                SendProfDatatoOther(ccb, 1);//profile is invalid	
            }
            else
            {
                CreateDefaultProfUpdateReqToCpe(UpdateReq, 1, eid, CPE_ADM_STATUS_ADM, 1);
                SendProfDatatoOther(ccb, 0);//profile is invalid
            }        
            #else
            CreateDefaultProfUpdateReqToCpe(UpdateReq, 1,eid, CPE_ADM_STATUS_ADM);
            SendProfDatatoOther(ccb, 1);//profile is invalid	
            #endif
            //UpdateReq.setPrdRegTime(ccb.getPrdRegTimeValue());
            UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
            BeginProfUpdateReqToCpeTrasaction(UpdateReq, ccb);
            
            //向cpe下发切换参数
            SendHandoverParaToCpe(eid);
            SendValidFreqsParaToCpe(eid);
        }
        
        /*为了让用户在sag失连情况下也可以使用*/
        sendBWInfoReq(ccb);	
        //故障弱化时不启动定时器jy20100310
        if((sagStatusFlag == false)&&(TRUE== bspGetIsPermitUseWhenSagDown()))   
        {
            OAM_LOGSTR1(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CPE[0x%08x] profile is INVALID and SAG is down. ", eid );
            return CPE_SERVING; 
        }
        #ifdef M_TGT_WANIF
        if(wcpe_rcpe_flag == 0)//wcpe rcp  e 不加定时器
        {
            if (NULL != ccb.m_pCpeRegTimer)
            {
                ccb.m_pCpeRegTimer->Stop();
                delete ccb.m_pCpeRegTimer;
                ccb.m_pCpeRegTimer = NULL;
            }
            CTimer *timer = createRegisterTimer(eid);
            ccb.m_pCpeRegTimer = timer;
        }
        #else
        if (NULL != ccb.m_pCpeRegTimer)
        {
            ccb.m_pCpeRegTimer->Stop();
            delete ccb.m_pCpeRegTimer;
            ccb.m_pCpeRegTimer = NULL;
        }
        CTimer *timer = createRegisterTimer(eid);
        ccb.m_pCpeRegTimer = timer;
        #endif
#ifndef WBBU_CODE
    CTaskCpeM::CPE_AddDeleteCpeInfo(eid,MIN_CPE_REMANENT_CYCLE*2);
#else
    CPE_AddDeleteCpeInfo(eid, MIN_CPE_REMANENT_CYCLE*2);
#endif
       //通知终端1小时,基站定时器修改为2小时jiaying20100921
        
        return CPE_WAITFOREMS; 
        }
}

#ifndef _INC_L3OAMCFG
#include "L3OamCfg.h"
//#include "L3OamCpem.h"
#endif
void CPETrans::SendHandoverParaToCpe(UINT32 eid)
{
#if 1//def NUCLEAR_CODE
#pragma pack(1)
	struct T_HandoverPara
	{
	    UINT8 M_HO_PWR_THD;
	    UINT8 M_HO_PWR_OFFSET1;
	    UINT8 M_HO_PWR_OFFSET2;    
	    UINT8 M_HO_PWR_FILTERCOEF_STAT;
	    UINT8 M_HO_PWR_FILTERCOEF_MOBILE;
	    UINT8 TIME_TO_TRIGGER;
	    UINT16 M_CPE_CM_HO_PERIOD;
        
		UINT16 usCount;
		UINT16 usSignal;
		UINT16 usLd;
		UINT16 usLdPeriod;

        UINT8 StrictArea_Pwr_THD;
        UINT8 StrictArea_TIME_TO_TRIGGER;
        UINT8 StrictArea_HO_PWR_OFFSET;
    	UINT8 reserve;

#ifdef PAYLOAD_BALANCE_2ND
        UINT16 usParam;
        UINT16 usBandSwitch;
#endif
	}st;
#pragma pack()
    if(NvRamDataAddr->UtHandoverPara.write_flag != 0x5a5a)
    {
       return;       
    }
       CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();

    memset( (UINT8*)&st, 0, sizeof(st) );
	memcpy( (UINT8*)&st, (UINT8*)&NvRamDataAddr->UtHandoverPara, sizeof(T_UtHandoverPara)-2);
    st.usCount = CTaskCfg::GetInstance()->m_stPldBlnCfg.usCount;
    st.usSignal = CTaskCfg::GetInstance()->m_stPldBlnCfg.usSignal;
    st.usLd = CTaskCfg::GetInstance()->m_stPldBlnCfg.usLd;
    st.usLdPeriod = CTaskCfg::GetInstance()->m_stPldBlnCfg.usLdPeriod;
	if(NvRamDataAddr->stUtHandoverPara2.write_flag == 0x5a5a)
    	memcpy( (UINT8*)&st.StrictArea_Pwr_THD, (UINT8*)&NvRamDataAddr->stUtHandoverPara2, sizeof(T_UtHandoverPara2)-2);
	st.usParam = CTaskCfg::GetInstance()->m_stPldBlnCfg.usParam;
	st.usBandSwitch = CTaskCfg::GetInstance()->m_stPldBlnCfg.usBandSwitch;
	CTaskCpeM::GetInstance()->RPT_SendComMsg( eid, M_TID_CPECM, M_CPE_L3_HANDOVER_PARA_CFG_REQ, (UINT8*)&st, sizeof(st) );
	CTaskCfg::GetInstance()->CM_NuclearSendLimitData( false, eid );
#else//NUCLEAR_CODE
    if(NvRamDataAddr->UtHandoverPara.write_flag != 0x5a5a)
    {
       return;       
    }
	struct T_HandoverPara
	{
	    UINT8 M_HO_PWR_THD;
	    UINT8 M_HO_PWR_OFFSET1;
	    UINT8 M_HO_PWR_OFFSET2;    
	    UINT8 M_HO_PWR_FILTERCOEF_STAT;
	    UINT8 M_HO_PWR_FILTERCOEF_MOBILE;
	    UINT8 TIME_TO_TRIGGER;
	    UINT16 M_CPE_CM_HO_PERIOD;
		UINT16 usCount;
		UINT16 usSignal;
		UINT16 usLd;
		UINT16 usLdPeriod;
#ifdef PAYLOAD_BALANCE_2ND
        UINT16 usBandSwitch;
#endif
	}*pstHandoverPara;
   CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    CComMessage* pComMsg ;
	pComMsg = new (pTaskCpeM/*this*/, sizeof(T_HandoverPara) ) CComMessage;
    if (pComMsg==NULL)
    {
    	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in SendHandoverParaToCpe.");
    	return;
    }
    pComMsg->SetDstTid(M_TID_CPECM);
    pComMsg->SetSrcTid(M_TID_UM);    
    pComMsg->SetMessageId(M_CPE_L3_HANDOVER_PARA_CFG_REQ);    
    pComMsg->SetEID(eid);
	pstHandoverPara = (T_HandoverPara*)pComMsg->GetDataPtr();
    memcpy((UINT8*)((UINT8*)pComMsg->GetDataPtr()), (UINT8*)&NvRamDataAddr->UtHandoverPara, sizeof(T_UtHandoverPara)-2);
    CTaskCfg *pTaskCfgM = CTaskCfg::GetInstance();
	pstHandoverPara->usCount = pTaskCfgM->m_stPldBlnCfg.usCount;
	pstHandoverPara->usSignal = pTaskCfgM->m_stPldBlnCfg.usSignal;
	pstHandoverPara->usLd = pTaskCfgM->m_stPldBlnCfg.usLd;
	pstHandoverPara->usLdPeriod = pTaskCfgM->m_stPldBlnCfg.usLdPeriod;
#ifdef PAYLOAD_BALANCE_2ND
	pstHandoverPara->usBandSwitch = pTaskCfgM->m_stPldBlnCfg.usBandSwitch;
#endif
	if(!CComEntity::PostEntityMessage(pComMsg))
    {
    	pComMsg->Destroy();
    	pComMsg = NULL;
    }
#endif
}
void CPETrans::sendDeleteMsgtoZ( char* p )
{
    memcpy( m_aucMZDelMsg, p+11, 8 );
    memcpy( m_aucMZDelMsg+8, p+21, 8 );
    memset( m_aucMZDelMsg+16, 0, 2 );
	CTaskCpeM::GetInstance()->RPT_SendComMsg( *(UINT32*)(m_aucMZDelMsg+4), M_TID_UTV, M_CPE_L3_MZ_DELETE_UID_REQ, m_aucMZDelMsg, 18 );
    return;
}
void CPETrans::sendDeleteMsgtoUT(UINT32 eid)
{ 
    CTaskCpeM *pTCpeInst = CTaskCpeM::GetInstance();    
     OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] Rx V53 bwinfo, CPE[0x%08X].", eid);
     UINT16 TosNum = 0;
     T_CpeToSCfgEle TosCfgA[MAX_TOS_ELE_NUM]; 
     memset(TosCfgA, 0, sizeof(TosCfgA));
     //GetTosInfo(TosCfgA, TosNum, HIGH_TOS_CLASS);
     
     //CL3OamCfgBtsRepeaterReq L3OamCfgBtsRepeaterReq;
     UINT32 RepeaterInfoLen = 2;
     
     UINT32 BTSIDs[NEIGHBOR_BTS_NUM] = {0};
     UINT16 BtsNum = 0;
     UINT16 BtsinfoLen = sizeof(UINT16);
     CL3CpeProfUpdateReq UpdateReq;
     if (false == UpdateReq.CreateMessage(*pTCpeInst,100))
     {
         OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] CreateMessage error, this may cause unexpected results.");
         return;
     }
     UpdateReq.SetEID(eid);
     //UpdateReq.SetMessageId(M_L3_CPE_PROFILE_UPDATE_REQ);
     UpdateReq.SetDstTid(M_TID_CPECM);
     UpdateReq.SetVersion(OAM_DEFAUIT_TRANSID);    
     
     //设置资源管理相关消息内容
     T_RMPoliceEle *pRMPCfgEle = (T_RMPoliceEle*)&(NvRamDataAddr->RMPoliceEle);
     UpdateReq.SetLocAreaId(bspGetLocAreaID()); 
     UpdateReq.SetBWReqInter(pRMPCfgEle->BWReqInterval);
     UpdateReq.SetSessRelThres(pRMPCfgEle->SRelThreshold);
     UpdateReq.SetProfileDataFlag(1);  //profile is valide
     UpdateReq.SetMessageId(M_L3_CPE_UPDATE_BWINFO);
     //设置缺省profile ie
     T_UTProfile UTProfile;
     memset(&(UTProfile.UTProfileIEBase), 0, sizeof(T_UTProfileIEBase));
     UTProfile.UTProfileIEBase.isDefaultProfile = 1;//CPE根据这个标志决定是否把profile写入flash.
     //UTProfile.UTProfileIEBase.ucPeriodicalRegisterTimerValue = 0;
     UTProfile.UTProfileIEBase.AdminStatus = 0x0b;
     UpdateReq.setUTProfile(UTProfile);
     
     UpdateReq.SetQosEntryNum(TosNum); //设置tos相关消息内容
     UpdateReq.SetTosInfo((SINT8 *)&TosCfgA[0] , sizeof(T_ToSCfgEle) * TosNum);
     //设置BTS Repeater 相关消息内容  
     SINT8 RepeaterInfo[200];
     memset(RepeaterInfo, 0, sizeof(RepeaterInfo));
     SINT8* pdata = RepeaterInfo;
     UINT8 temp_len = 0;
     if(BtsNum <= NEIGHBOR_BTS_NUM )
     {
         OAM_LOGSTR(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send valid config data to CPE");    
         memcpy(pdata, &BtsNum, 2);
         pdata += 2;
         memcpy(pdata, BTSIDs, BtsNum * sizeof(UINT32));
         pdata += BtsNum * sizeof(UINT32);
         if ( sizeof(UINT16) == RepeaterInfoLen )
         {
             memset(pdata, 0, sizeof(UINT16));
             temp_len = 2;
         }
         else
         {
             memcpy(pdata, (SINT8 *)&(NvRamDataAddr->BTSRepeaterEle), RepeaterInfoLen );
             temp_len = RepeaterInfoLen;
         }
         UpdateReq.SetFreqInfo(RepeaterInfo , RepeaterInfoLen + BtsinfoLen);
     }
     else
     {
         OAM_LOGSTR(LOG_SEVERE, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM] send invalide config data to CPE");  
         UpdateReq.SetFreqInfo(RepeaterInfo ,4);
         temp_len = 4;
     }
     
     UINT16 *p;
     
     
     UINT8 *pBtsDta = (UINT8 *)(UpdateReq.GetDataPtr()) ;
     pBtsDta +=46;
     UINT32 btsid = (bspGetBtsID());
     memcpy(pBtsDta,(SINT8*)&btsid,4);
     //magic
     p = (UINT16*)(pBtsDta+4);
     *(p) = 0x55aa;
     p += 1;
     
     //N_parameter;
     *p = ( (NvRamDataAddr->N_parameter.UT_PowerLock_Timer_Th & 0xF)<<7 )
     | ( (NvRamDataAddr->N_parameter.Ci_Jump_detection& 0x1F)<<2 )
     | ( (NvRamDataAddr->N_parameter.N_Algorithm_Switch&0x1)<<1 )
     | 0x1;
     UpdateReq.SetDataLength(46 + 4 +4+4);
     
     CL3OamCommonRsp FailMsg;
     if (false == FailMsg.CreateMessage(*pTCpeInst))
     return;
     FailMsg.SetEID(UpdateReq.GetEID()); 
     FailMsg.SetDstTid(M_TID_UM);
     FailMsg.SetMessageId(M_OAM_CPE_PROF_UPDATE_FAIL);
     
     CTransaction *pTrans = pTCpeInst->CreateTransact(UpdateReq, FailMsg, 1, OAM_REQ_RSP_PERIOD);
     if (NULL == pTrans)
     {
         UpdateReq.DeleteMessage();
         FailMsg.DeleteMessage();
         return;
     }
     UINT16 TransId = pTrans->GetId();
     UpdateReq.SetTransactionId(TransId);
     FailMsg.SetTransactionId(TransId);
    if(!pTrans->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pTrans->EndTransact();
        delete pTrans;
    }
     return;
}
//#ifdef RPT_FOR_V6
void CPETrans::sendRptReqToEms(const T_RegisterReq *const msgUTRegisterReq)
{
#pragma pack(1)
	struct
	{
		UINT16 usTranid;
		UINT32 UID;
		UINT32 ulPID;
		UINT16 HardwareType;
		UINT8 SoftwareType;
		UINT8 Rsv;
		UINT32 ActiveSWVersion;
		UINT32 StandbySWVersion;
		UINT8 HardwareVersion[16];
	}stReq;
#pragma pack()
	const T_RegisterReq *const pst = msgUTRegisterReq;
	stReq.UID = pst->ulUID;
	stReq.ulPID = pst->ulPID;
	stReq.HardwareType = pst->UTBaseInfo.usHWtype;
	stReq.SoftwareType = pst->UTBaseInfo.ucSWtype;
	stReq.Rsv = 0;
	stReq.ActiveSWVersion = pst->UTBaseInfo.ulActiveSWversion;
	stReq.StandbySWVersion = pst->UTBaseInfo.ulStandbySWversion;
	memcpy( stReq.HardwareVersion, pst->UTBaseInfo.ucHWversion, 16 );
	CTaskCpeM::GetInstance()->RPT_SendComMsg( pst->ulPID, M_TID_EMSAGENTTX, M_EMS_BTS_BWINFO_REQ, (UINT8*)&stReq, sizeof(stReq) );
}
unsigned char CPE_IS_Rcpe(unsigned int eid)
{
          if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
          	{
                for(int mm=0; mm<NvRamDataAddr->Relay_num; mm++)
                {
                    if(RelayWanifCpeEid[mm]==eid)
                    {
                       
                        return 1;
                       // break;
                    }
                }
          	}
		 return 0;
}

/*
* 终端注册成功后给它下发有效频点集
*/
void CPETrans::SendValidFreqsParaToCpe(UINT32 eid)
{
    if(NvRamDataAddr->volidFreqPara.validFlag!=0x5a5a)
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]volidFreqPara.validFlag!=0x5a5a, not send to cpe[0x%x]", eid);
        return;
    }
    CTaskCpeM *pTaskCpeM = CTaskCpeM::GetInstance();
    CComMessage* pComMsg ;
    pComMsg = new (pTaskCpeM/*this*/, NvRamDataAddr->volidFreqPara.validFreqsNum*2+2) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in SendValidFreqsParaToCpe.");
        return;
    }
    pComMsg->SetDstTid(M_TID_CPECM);
    pComMsg->SetSrcTid(M_TID_UM);    
    pComMsg->SetMessageId(M_BTS_CPE_VALIDFREQS_REQ);    
    pComMsg->SetEID(eid);    
    memcpy((UINT8*)((UINT8*)pComMsg->GetDataPtr()), (UINT8*)&NvRamDataAddr->volidFreqPara.validFreqsInd, \
        NvRamDataAddr->volidFreqPara.validFreqsNum*2+2);    
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }
    OAM_LOGSTR1(LOG_DEBUG3, L3UM_ERROR_CPE_INFO_PRINT, "[tCpeM]SendValidFreqsParaToCpe cpe[0x%x]", eid);
}
