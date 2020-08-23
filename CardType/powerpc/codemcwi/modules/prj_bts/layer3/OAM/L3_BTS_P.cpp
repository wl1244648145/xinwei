/***********************************************************************************
** 周期性数据发送当前值
** 实时性数据发送差值
** 
***********************************************************************************/
#ifndef _INC_L3_BTS_PM
#include "L3_BTS_PM.h"
#endif

#ifndef _INC_L3OAMMESSAGEID
#include "L3OAMMessageId.h"
#endif   
                                                  
#ifndef __DATA_API_H__
#include "l3dataapi.h"
#endif

#include "L3OamFile.h"
#include "ErrorCodeDef.h" 
#include "L3OamCfgCommon.h"
#include "L3OamCfg.h"


//new stat
extern "C" void statBegin();
extern "C" void statEnd(UINT8 *pdata, UINT8 *ptimelong);
extern T_NvRamData *NvRamDataAddr;
//new stat
extern void GetDataPerfDataNew( UINT8 Type, UINT32 *pData );
extern void ClearDataPerfDataNew(UINT8 ucType);
extern void GetVoicePerfDataNew(UINT32 *pdata);
extern void ClearVoicePerfDataNew();



T_BTSPM_FTPINFO g_tFTPInfo;
 CTaskPM*   CTaskPM::m_Instance = NULL;          

 #ifdef WBBU_CODE

 UINT16 g_last_active_user =0;
 UINT16 g_active_user_num_same_time = 0;
 UINT32  g_minute_time = 0;
  UINT32  g_last_minute_time = 0;

  extern "C" void    ResetDSP1_NOUSER();
  extern unsigned int L2_L3_Ingress;
  UINT32     g_last_2_L3_Ingress = 0;
 #endif
CTaskPM :: CTaskPM ()
{    

    memcpy(m_szName, "tPM", 3);
    m_szName[3] = '\0';
    m_uPriority = M_TP_L3PM;
    m_uOptions  = 0;
    m_uStackSize= 1000 * 60 ;
    m_iMsgQMax = 100;

    m_iMsgQOption  = 0;
}
CTaskPM :: ~CTaskPM ()
{ 
        int i;
        if(m_tRsrvPerfData.pUTPerfData)
            delete m_tRsrvPerfData.pUTPerfData;
        if(m_tRsrvPerfData.pL2PerfData)
            delete m_tRsrvPerfData.pL2PerfData;
        if(m_tRsrvPerfData.pL3PerfData)
            delete m_tRsrvPerfData.pL3PerfData;
        if(m_pReportTm!=NULL)
        {
            m_pReportTm->Stop();
            delete m_pReportTm;
        }
        if(m_pCollectTm!=NULL)
        {
            m_pCollectTm->Stop();
            delete m_pCollectTm;
        }
        if(m_pWaitTM!=NULL)
        {
            m_pWaitTM->Stop();
            delete m_pWaitTM;
        }
        if(m_pWaitColTm!=NULL)
        {
            m_pWaitColTm->Stop();
            delete m_pWaitColTm;        
        }
        for( i=0; i<MAX_TIMER;i++)
        {
            if(m_TimerInfo[i].pTimer!=NULL)
            {
                m_TimerInfo[i].pTimer->Stop();
                delete m_TimerInfo[i].pTimer;
            }
        }
	if( m_pucDelayData[0] )
       {
           delete [] m_pucDelayData[0];
           m_pucDelayData[0] = NULL;
       }
       if( m_pucDelayData[1] )
       {
           delete [] m_pucDelayData[1];
           m_pucDelayData[1] = NULL;
       }
	for(i=0; i<RTMntMaxTm; i++)
	{
	    if(m_ptmArrRTMnt[i]!=NULL)
	    {
	        m_ptmArrRTMnt[i]->Stop();
		 delete m_ptmArrRTMnt[i];
	    }
	}
	m_Instance = NULL;
}

TID CTaskPM :: GetEntityId() const
{
    return M_TID_PM;
}
CTaskPM* CTaskPM ::GetInstance()
{
    if ( NULL == m_Instance )
    {
        m_Instance = new CTaskPM;
    }

    return m_Instance;
}
bool CTaskPM :: Initialize()
{
    if( ! CBizTask :: Initialize() )
		return false;

    if( ! InitPMApp() )
        return false;

    return true;

}
bool CTaskPM :: InitPMApp()
{   
    m_TransationID = 0;
    m_pReportTm  = NULL;
    m_pCollectTm = NULL;
    m_pWaitTM = NULL;/*jy 090823*/
    m_pWaitColTm = NULL;
    m_bPeriodCollect = false;
    m_uBTSId = bspGetBtsID();

    memset( (UINT8*)&m_tRcdHead, 0, sizeof(m_tRcdHead) );
    m_tRcdHead.BTSId = bspGetBtsID();
    m_tRcdHead.Data_Type = 0;

    //memset( (UINT8*)&g_tFTPInfo, 0, sizeof(g_tFTPInfo) );不清0，基站刚开始工作时会被赋值,任务被删除重启后该变量不变jiaying20100906
    memset( (UINT8*)&m_TimerInfo, 0, sizeof(m_TimerInfo) );

    m_tRsrvPerfData.pL2PerfData = NULL;
    m_tRsrvPerfData.pL3PerfData = NULL;
    m_tRsrvPerfData.pUTPerfData = NULL;

    StartCollectTm();
    StartReportTm();
    if( false!=StartWaitColTm())
    {
        //上报延迟时间为(btsid%300+1)*2秒
        m_pWaitColTm->SetInterval(((m_uBTSId%300)+1)*2*1000);
        m_pWaitColTm->Stop();        
    }
    
    if( ! InitTimer( L2_CUR_TIMER, M_L2_CUR_PERF_REQ_TIMEOUT_NOTIFY ) )
        return false;
    if( ! InitTimer( L2_PERIOD_TIMER, M_L2_PERIOD_PERF_REQ_TIMEOUT_NOTIFY ) )
        return false;
    if( ! InitTimer( CPE_CUR_TIMER, M_CPE_CUR_PERF_REQ_TIMEOUT_NOTIFY ) )
        return false;
    for ( int i=0; i<MAX_TIMER; i++ )
        m_TimerInfo[i].uEID = 0x12345613;
#ifdef WBBU_CODE
   mkdir(SM_CPE_PERF_DIR);
   mkdir(SM_CPE_SIG_DIR);
#endif
    m_bRTMonitorAct = true;
    m_ptmArrRTMnt[RTMntActTm] = NULL;
    m_ptmArrRTMnt[RTMntStartTm] = NULL;
    m_ptmArrRTMnt[RTMntDelayTm] = NULL;
    m_usRTMonitorTmLen = 0;
    m_pucDelayData[0] = NULL;
    m_pucDelayData[1] = NULL;
    m_pucDelayDataLen[0] = 0;
    m_pucDelayDataLen[1] = 0;
    m_usGlobalIdx = 0;
    m_curren_Index_put = 0;
    m_curren_Index_get = 0;
    memset( &m_CpeSignalFileInfo, 0, sizeof(m_CpeSignalFileInfo));
    memset( &m_tFileInfo, 0, sizeof(m_tFileInfo) );
    //new stat
    memset(&m_tFileInfo_new, 0, sizeof(m_tFileInfo_new));
    memset( (UINT8*)&m_tRcdHead_new, 0, sizeof(m_tRcdHead_new) );
    m_tRcdHead_new.BTSId = bspGetBtsID();
    m_tRcdHead_new.version = 0;
    m_tRcdHead_new.rev = 0;
    return true;
}


bool CTaskPM :: InitTimer( UINT8 const nIdx, UINT16 const usMsgId )
{

    CComMessage *pMsgTimer = new ( this, 0 ) CComMessage;
    CMessage *pMsgConTainer = new CMessage( pMsgTimer );
    if (pMsgTimer!=NULL)
    {
        pMsgTimer->SetDstTid( M_TID_PM );
        pMsgTimer->SetSrcTid( M_TID_PM );
        pMsgTimer->SetMessageId( usMsgId );
    }
    else
        return false;
    m_TimerInfo[nIdx].pTimer = new  CTimer( false, L1L2CPE_REQUEST_TIMEOUT, *pMsgConTainer );    
    if(NULL==m_TimerInfo[nIdx].pTimer)
    {
        pMsgConTainer->DeleteMessage();
        return false;
    }
    return true;
}

UINT16 CTaskPM :: GetTRMonitorTimerDelay()
{
	T_TimeDate stGpsTime = bspGetDateTime();
	UINT16 usCrrtTime = (stGpsTime.minute*60)+stGpsTime.second;
	if( 0 == usCrrtTime % m_usRTMonitorTmLen )
		return 0;
	else
		return m_usRTMonitorTmLen - (usCrrtTime%m_usRTMonitorTmLen);
}
void CTaskPM :: InitRTMonitor()
{
//	m_usTransId = 0;
    OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_FTP_INFO_MSG, "[tPM] PM_RTMonitorReq::InitRTMonitor Stop RTMonitor" );
	m_bRTMonitorAct = false;
	m_usRTMonitorTmLen = 0;					
	m_pucDelayDataLen[0] = 0;
	m_pucDelayDataLen[1] = 0;
	m_usGlobalIdx = 0;
	for( UINT8 uc=0; uc<RTMntMaxTm; uc++ )
	{
		if( m_ptmArrRTMnt[uc])
		{
			m_ptmArrRTMnt[uc]->Stop();
			delete m_ptmArrRTMnt[uc];
			m_ptmArrRTMnt[uc] = NULL;
		}
	}
	if( m_pucDelayData[0] )
   {
		delete [] m_pucDelayData[0];
        m_pucDelayData[0] = NULL;
    }
	if( m_pucDelayData[1] )
    {
		delete [] m_pucDelayData[1];
        m_pucDelayData[1] = NULL;
    }
	m_curren_Index_put = 0;
	m_curren_Index_get = 0;
} 
CTimer* CTaskPM :: InitRTMonitorTimer( bool bPeriod, UINT16 usMsgID, UINT16 usPeriod )
{
    CComMessage *pMsgTimer = new ( this, 0 ) CComMessage;
    CMessage *pMsgConTainer = new CMessage( pMsgTimer );
    if (pMsgTimer!=NULL)
    {
        pMsgTimer->SetDstTid( M_TID_PM );
        pMsgTimer->SetSrcTid( M_TID_PM );
        pMsgTimer->SetMessageId( usMsgID );
        CTimer* pTmTmp = new  CTimer( bPeriod, usPeriod, *pMsgConTainer );
        if(NULL==pTmTmp)
        {
            pMsgConTainer->DeleteMessage();
            return NULL;
        }
        return pTmTmp;
    }
    return NULL;
} 
UINT32 CTaskPM::GetRTMonitorTimeStamp( UINT8* uc )
{
	T_TimeDate tTimeData;
    //struct tm time_s;
    //UINT32 secCount;

    tTimeData = bspGetDateTime();
	*(UINT16*)uc = tTimeData.year;
	*(uc+2) = tTimeData.month;
	*(uc+3) = tTimeData.day;
	*(uc+4) = tTimeData.hour;
	*(uc+5) = tTimeData.minute;
	*(uc+6) = tTimeData.second;
	return 1;
	#if 0
    //去掉终端延迟时间
    time_s.tm_min = tTimeData.minute;
    time_s.tm_hour = tTimeData.hour;
    time_s.tm_mday = tTimeData.day;
    time_s.tm_mon = tTimeData.month - 1;
    time_s.tm_year = tTimeData.year - 1900;
    time_s.tm_sec = tTimeData.second;
    time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

    secCount = mktime(&time_s);
    if( (secCount%m_usRTMonitorTmLen)==0 )
		return secCount;
    if( (secCount%m_usRTMonitorTmLen)>=(m_usRTMonitorTmLen/2))
		return secCount+(m_usRTMonitorTmLen-secCount%m_usRTMonitorTmLen);
    if( (secCount%m_usRTMonitorTmLen)<(m_usRTMonitorTmLen/2))
		return secCount-secCount%m_usRTMonitorTmLen;  
	#endif
}
void CTaskPM::PM_StartRTMonitor(CMessage& rMsg )
{
	UINT16 usStartTimeLen, usRecvTmLen, us[2];
	m_usGlobalIdx = *((UINT16*)rMsg.GetDataPtr()+1);
	usRecvTmLen = *((UINT16*)rMsg.GetDataPtr()+2);
	m_usRTMonitorTmLen = usRecvTmLen;
	m_bRTMonitorAct = true;
	usStartTimeLen = GetTRMonitorTimerDelay();//m_usRTMonitorTmLen-( (stGpsTime.minute*60)+stGpsTime.second ) % m_usRTMonitorTmLen;
    OAM_LOGSTR3(LOG_DEBUG3, L3PM_ERROR_FTP_INFO_MSG, "[tPM] PM_RTMonitorReq::PM_StartRTMonitor currentSec[%08d],after[%02d], period[%02d]", testGetSec(),usStartTimeLen, usRecvTmLen );
	m_ptmArrRTMnt[RTMntActTm]= InitRTMonitorTimer(false,M_OAM_PERIOD_RTMONITOR_ACTIVE_TIMER,m_usRTMonitorTmLen*1000);
	if( 0 == usStartTimeLen )
	{
             if(NULL!=m_ptmArrRTMnt[RTMntActTm])
		    m_ptmArrRTMnt[RTMntActTm]->Start();												
		us[0] = RTMONITOR_TRANSID_SEND;
		us[1] = m_usGlobalIdx;
		SendMessage( M_L3TOL2_BTS_RTMONITOR_REQ, M_TID_L2OAM, 4, (UINT8*)us );
	}
	else
	{
		m_ptmArrRTMnt[RTMntStartTm]= InitRTMonitorTimer(false,M_OAM_PERIOD_RTMONITOR_START_TIMER, usStartTimeLen*1000);
		if(NULL!=m_ptmArrRTMnt[RTMntStartTm])
		    m_ptmArrRTMnt[RTMntStartTm]->Start();
	}
	m_ptmArrRTMnt[RTMntDelayTm]= InitRTMonitorTimer(false,M_OAM_PERIOD_RTMONITOR_DELAY_TIMER,(m_uBTSId%7)*500);

}
void CTaskPM::PM_StartRTMonitor( )
{
	UINT16 usStartTimeLen, usRecvTmLen, us[2];
	 if( NvRamDataAddr->RTMonitorCfg.cfg_flag != 0x5f5f )	
	 	return;
	 if(  NvRamDataAddr->RTMonitorCfg.usRTMonitorTmLen ==0 )
	 	return;
	m_usGlobalIdx = NvRamDataAddr->RTMonitorCfg.usGlobalIdx;;
	usRecvTmLen = NvRamDataAddr->RTMonitorCfg.usRTMonitorTmLen;
	m_usRTMonitorTmLen = usRecvTmLen;
	m_bRTMonitorAct = true;
	usStartTimeLen = GetTRMonitorTimerDelay();//m_usRTMonitorTmLen-( (stGpsTime.minute*60)+stGpsTime.second ) % m_usRTMonitorTmLen;
    OAM_LOGSTR3(RPT_LOG, L3UM_ERROR_CPE_INFO_PRINT, "[tPM] PM_RTMonitorReq::PM_StartRTMonitor currentSec[%08d],after[%02d], period[%02d]", testGetSec(),usStartTimeLen, usRecvTmLen );
	m_ptmArrRTMnt[RTMntActTm]= InitRTMonitorTimer(false,M_OAM_PERIOD_RTMONITOR_ACTIVE_TIMER,m_usRTMonitorTmLen*1000);
	if( 0 == usStartTimeLen )
	{
	       if(NULL!=m_ptmArrRTMnt[RTMntActTm])
		    m_ptmArrRTMnt[RTMntActTm]->Start();												
		us[0] = RTMONITOR_TRANSID_SEND;
		us[1] = m_usGlobalIdx;
		SendMessage( M_L3TOL2_BTS_RTMONITOR_REQ, M_TID_L2OAM, 4, (UINT8*)us );
	}
	else
	{
		m_ptmArrRTMnt[RTMntStartTm]= InitRTMonitorTimer(false,M_OAM_PERIOD_RTMONITOR_START_TIMER, usStartTimeLen*1000);
		if(NULL!=m_ptmArrRTMnt[RTMntStartTm])
		    m_ptmArrRTMnt[RTMntStartTm]->Start();
	}
	m_ptmArrRTMnt[RTMntDelayTm]= InitRTMonitorTimer(false,M_OAM_PERIOD_RTMONITOR_DELAY_TIMER,(m_uBTSId%7)*500);

}
void CTaskPM::PM_RTMonitorReq(CMessage& rMsg )
{
	if( *((UINT16*)rMsg.GetDataPtr()) > RTMONITOR_TRANSID_RECV)
	{
		UINT16  usRsp[134];

		memset( (UINT8*)usRsp, 0 , 268 );
		usRsp[0] = *((UINT16*)rMsg.GetDataPtr());
		usRsp[2] = 0x7;
		SendMessage( M_BTS_EMS_BTS_RTMONITOR_RSP, M_TID_EMSAGENTTX, 268, (UINT8*)usRsp );
        OAM_LOGSTR3(LOG_DEBUG3, L3PM_ERROR_FTP_INFO_MSG, "[tPM] PM_RTMonitorReq::PM_RTMonitorReq AAA data0[%04x], data1[%04x], data2[%04x]", *((UINT16*)rMsg.GetDataPtr()),*((UINT16*)rMsg.GetDataPtr()+1), *((UINT16*)rMsg.GetDataPtr()+2) );
         //save the rtmonitor config to nvram 
        T_RTMONITORCFG RTMonitorCfg;
        RTMonitorCfg.cfg_flag = 0x5f5f;			   
	    RTMonitorCfg.usGlobalIdx = *((UINT16*)rMsg.GetDataPtr()+1);;
	    RTMonitorCfg.usRTMonitorTmLen = *((UINT16*)rMsg.GetDataPtr()+2);			
	    CTaskCfg::l3oambspNvRamWrite( (char*)&NvRamDataAddr->RTMonitorCfg.usGlobalIdx, (char*)&RTMonitorCfg, sizeof(RTMonitorCfg) );

		if( m_bRTMonitorAct )
		{
			if ( 0 == *((UINT16*)rMsg.GetDataPtr()+2) )
			{
                OAM_LOGSTR3(LOG_DEBUG3, L3PM_ERROR_FTP_INFO_MSG, "[tPM] PM_RTMonitorReq::PM_RTMonitorReq BBB data0[%04x], data1[%04x], data2[%04x]", *((UINT16*)rMsg.GetDataPtr()),*((UINT16*)rMsg.GetDataPtr()+1), *((UINT16*)rMsg.GetDataPtr()+2) );
				InitRTMonitor();
			}
			if( m_usRTMonitorTmLen != *((UINT16*)rMsg.GetDataPtr()+2) )
			{
				InitRTMonitor();
				PM_StartRTMonitor( rMsg );
			}
		}
		else 
		{
			if( 0 != *((UINT16*)rMsg.GetDataPtr()+2) )
			{
				PM_StartRTMonitor( rMsg );
			}
		}
	}
	else
	{
		SendMessage( M_L3TOL2_BTS_RTMONITOR_REQ, M_TID_L2OAM, rMsg.GetDataLength()-2, (UINT8*)rMsg.GetDataPtr() );
	}
}
bool CTaskPM :: ProcessMessage(CMessage &rMsg)
{
    UINT16 ust[2], MsgId;
    MsgId = rMsg.GetMessageId();
    T_TimeDate tTimeData;
    int offset;
	#ifdef WBBU_CODE
      UINT8  *TPtr;
	  UINT16  Active_User_NUM = 0;
      
	  #endif
    switch(MsgId)
    {
	case M_EMS_BTS_LINK_TIMER:
		InitRTMonitor();
		break;
    case M_OAM_PERIOD_RTMONITOR_ACTIVE_TIMER:
		ust[0] = RTMONITOR_TRANSID_SEND;
		ust[1] = m_usGlobalIdx;
		SendMessage( M_L3TOL2_BTS_RTMONITOR_REQ, M_TID_L2OAM, 4, (UINT8*)ust );
		break;
	case M_OAM_PERIOD_RTMONITOR_DELAY_TIMER:
		SendMessage( M_BTS_EMS_BTS_RTMONITOR_RSP, M_TID_EMSAGENTTX, m_pucDelayDataLen[m_curren_Index_get], m_pucDelayData[m_curren_Index_get] );
		delete [] m_pucDelayData[m_curren_Index_get];
		m_pucDelayData[m_curren_Index_get] = NULL;
		m_pucDelayDataLen[m_curren_Index_get] = 0;
		if(m_curren_Index_get==0)
		{
		     m_curren_Index_get = 1;
		}
		else if(m_curren_Index_get==1)
		{
		     m_curren_Index_get = 0;
		}
		break;
	case M_OAM_PERIOD_RTMONITOR_START_TIMER:
		if(NULL!=m_ptmArrRTMnt[RTMntActTm])
		    m_ptmArrRTMnt[RTMntActTm]->Start();
		ust[0] = RTMONITOR_TRANSID_SEND;
		ust[1] = m_usGlobalIdx;
		SendMessage( M_L3TOL2_BTS_RTMONITOR_REQ, M_TID_L2OAM, 4, (UINT8*)ust );
		break;
    case M_EMS_BTS_BTS_RTMONITOR_REQ:
		OAM_LOGSTR1(RPT_LOG, L3PM_ERROR_FTP_INFO_MSG, "[tPM] M_EMS_BTS_BTS_RTMONITOR_REQ, period[%04d]", *((UINT16*)rMsg.GetDataPtr()+2)  );
		PM_RTMonitorReq(rMsg);
		break;
    case M_L2TOL3_BTS_RTMONITOR_RSP:
    	if( *(UINT16*)rMsg.GetDataPtr() == PERIOD_TRANSACTIONID )
    	{
    		WriteToFile( true, BTSPM_ENT_RTMONITOR, (UINT8*)rMsg.GetDataPtr()+2+2, rMsg.GetDataLength()-4 );
    	}
    	else if( RTMONITOR_TRANSID_SEND == *(UINT16*)rMsg.GetDataPtr() )
    	{
    		//UINT32 ul = GetRTMonitorTimeStamp();
    		m_pucDelayDataLen[m_curren_Index_put] = rMsg.GetDataLength() + sizeof(UINT32) + sizeof(UINT32) + sizeof(m_usRTMonitorTmLen);
    	   
    		m_pucDelayData[m_curren_Index_put] = new UINT8[m_pucDelayDataLen[m_curren_Index_put]];
		if(NULL!=m_pucDelayData[m_curren_Index_put])
		{
    		    memset( m_pucDelayData[m_curren_Index_put], 0, m_pucDelayDataLen[m_curren_Index_put] );
    		    memcpy( m_pucDelayData[m_curren_Index_put], (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength() );
    		    GetRTMonitorTimeStamp( m_pucDelayData[m_curren_Index_put]+rMsg.GetDataLength() );
    		    //memcpy( m_pucDelayData+rMsg.GetDataLength()+sizeof(UINT32), (UINT8*)&ul, sizeof(UINT32) );
    		    memcpy( m_pucDelayData[m_curren_Index_put]+rMsg.GetDataLength()+sizeof(UINT32)+sizeof(UINT32), (UINT8*)&m_usRTMonitorTmLen, sizeof(m_usRTMonitorTmLen) );
                  if(m_curren_Index_put==1)
                  {
                      m_curren_Index_put = 0;
                  }
                  else if(m_curren_Index_put==0)
                  {
                      m_curren_Index_put = 1;
                  }
        	OAM_LOGSTR2(RPT_LOG, L3PM_ERROR_FTP_INFO_MSG, "[tPM] PM_RTMonitorReq::ProcessMessage GetRsponse currentSec[%08d] currentDelay[%04d]", testGetSec(), (0==GetTRMonitorTimerDelay())?m_usRTMonitorTmLen:GetTRMonitorTimerDelay() );
                  if(NULL!=m_ptmArrRTMnt[RTMntDelayTm])
    		        m_ptmArrRTMnt[RTMntDelayTm]->Start();
		    if(NULL!=m_ptmArrRTMnt[RTMntActTm])	  
                  {
    		        m_ptmArrRTMnt[RTMntActTm]->SetInterval( 1000*((0==GetTRMonitorTimerDelay())?m_usRTMonitorTmLen:GetTRMonitorTimerDelay()) );
    		        m_ptmArrRTMnt[RTMntActTm]->Start();
                  }
		}
    	}
    	else
    	{
    		UINT16 usDataLen = rMsg.GetDataLength() + sizeof(UINT32) + sizeof(UINT32) + sizeof(UINT16);
    		UINT8* pucData = new UINT8[usDataLen];
              if(NULL!=pucData)
              {
    		    memset( pucData, 0, usDataLen );
    		    memcpy( pucData, (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength() );
    		    SendMessage( M_BTS_EMS_BTS_RTMONITOR_RSP, M_TID_EMSAGENTTX, usDataLen, pucData );
    		    delete [] pucData;
              }
    	}


		break;
    case M_EMS_BTS_ACTIVE_USER_LIST_REQ:
        OAM_LOGSTR(RPT_LOG, L3PM_ERROR_FTP_INFO_MSG, "[tPM] ProcessMessage::M_EMS_BTS_ACTIVE_USER_LIST_REQ");
		SendMessage( M_L3TOL2_ACTIVE_USER_LIST_REQ, M_TID_L2OAM, rMsg.GetDataLength(), (UINT8*)rMsg.GetDataPtr() );
		break;
    case M_L2TOL3_ACTIVE_USER_LIST_RSP:
       	OAM_LOGSTR(RPT_LOG, L3PM_ERROR_FTP_INFO_MSG, "[tPM] ProcessMessage::M_BTS_EMS_ACTIVE_USER_LIST_RSP");
		SendMessage( M_BTS_EMS_ACTIVE_USER_LIST_RSP, M_TID_EMSAGENTTX, rMsg.GetDataLength(), (UINT8*)rMsg.GetDataPtr() );
		break;
    case M_EMS_BTS_ACTIVE_USER_INFO_LIST_REQ:
        OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_FTP_INFO_MSG, "[tPM] ProcessMessage::M_EMS_BTS_ACTIVE_USER_INFO_LIST_REQ");
          SendMessage( M_L3TOL2_ACTIVE_USER_INFO_LIST_REQ, M_TID_L2OAM, rMsg.GetDataLength(), (UINT8*)rMsg.GetDataPtr() );
          break;
    case M_L2TOL3_ACTIVE_USER_INFO_LIST_RSP:
            OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_FTP_INFO_MSG, "[tPM] ProcessMessage::M_EMS_BTS_ACTIVE_USER_INFO_LIST_RSP");

	  /******************************************

     增加处理，如果Transid为0xffff;则不往ems发送这是由L3主动查询的

      ********************************************/
     #ifdef WBBU_CODE
	 TPtr = (UINT8*)rMsg.GetDataPtr();
	Active_User_NUM = TPtr[4]*0x100+TPtr[5];
	//frageMent = TPtr[8]*0x100+TPtr[9];
	  if((g_last_active_user  ==Active_User_NUM)&&(g_last_2_L3_Ingress==L2_L3_Ingress)/*&&(g_last_fragment==frageMent)*/)
	  {
	        if( 0xffff == *((UINT16*)rMsg.GetDataPtr()) )
	        {
	        OAM_LOGSTR2(LOG_DEBUG2, L3PM_ERROR_FTP_INFO_MSG,"[tPM] g_active_user_num_same_time:%d,%d\n",g_last_active_user,g_active_user_num_same_time);
	  	 	g_active_user_num_same_time++;
	        }
	  }
	  else
	  {
	         g_active_user_num_same_time = 0;
	  }
	   g_last_active_user =Active_User_NUM;
          g_last_2_L3_Ingress = L2_L3_Ingress;//记录数据包的个数
          //g_last_fragment = frageMent;
	   if(g_active_user_num_same_time==15)
	   {
	        
	          if(g_last_minute_time==0)
	          {
	          	       //复位DSP1  ;
	          	        OAM_LOGSTR4(LOG_DEBUG2, L3PM_ERROR_FTP_INFO_MSG,"[tPM] g_last_minute_time:%d,%d,%d,%d\n",g_last_active_user,g_active_user_num_same_time,g_last_minute_time,g_minute_time);
	          	       ResetDSP1_NOUSER();
	          	       g_last_minute_time =  g_minute_time;
	          }
		  else
		   {

                       if((g_minute_time-g_last_minute_time )>24*60)
                       {
			 		 //复位DSP1  ;
			 		     ResetDSP1_NOUSER();
			            g_last_minute_time =  g_minute_time;
                       }
			 }

		

		g_active_user_num_same_time = 0;
	   }

	 if( 0xffff == *((UINT16*)rMsg.GetDataPtr()) )//如果是L3主动查询响应
	 {
	     //不主动送给ems
	     //  g_minute_time++;
	 }
	 else
	 {
	     SendMessage( M_EMS_BTS_ACTIVE_USER_INFO_LIST_RSP, M_TID_EMSAGENTTX, rMsg.GetDataLength(), (UINT8*)rMsg.GetDataPtr() );
	 }
	  
	#else
	SendMessage( M_EMS_BTS_ACTIVE_USER_INFO_LIST_RSP, M_TID_EMSAGENTTX, rMsg.GetDataLength(), (UINT8*)rMsg.GetDataPtr() );
   #endif

		break;
    case M_EMS_BTS_PERF_LOGGING_CFG_REQ:
        PM_PerfLogCfgReq(rMsg);
        break;

    case M_EMS_BTS_L2_PERF_DATA_REQ:
        PM_L2PerfDataQueryReq(rMsg);
        break;              
    case M_EMS_BTS_L3_PERF_DATA_REQ:
        PM_L3PerfDataQueryReq(rMsg);
        break;              
    case M_EMS_BTS_UT_PERF_DATA_REQ:
        PM_CPEPerfDataQueryReq(rMsg);
        break;

    case M_OAM_PERIOD_PERFDATAQUERY_TIMER:          //周期性能数据查询
    {
        OAM_LOGSTR(LOG_DEBUG2, L3PM_ERROR_WRITE_FILE, "begin to get stat data now...");
        PM_PerfDataQueryTimer();
        //new stat
        PM_PerfDataQueryTimer_new();
        m_pCollectTm->Stop();
        
        tTimeData = bspGetDateTime(); 
        if((tTimeData.minute%g_tFTPInfo.uCollectInterval)>10)//提前了
        {            
            offset =60 - tTimeData.second + (g_tFTPInfo.uCollectInterval - tTimeData.minute%g_tFTPInfo.uCollectInterval -1)*60;         
            #ifdef WBBU_CODE
            m_pCollectTm->SetInterval((g_tFTPInfo.uCollectInterval * 60 + offset +12)*1000);//通过测试，bbu每15分钟最多提前12秒，故加上
            #else
            m_pCollectTm->SetInterval((g_tFTPInfo.uCollectInterval * 60 + offset)*1000);
            #endif
        }
        else 
        {
            offset = tTimeData.second + (tTimeData.minute%g_tFTPInfo.uCollectInterval)*60;
            #ifdef WBBU_CODE
            m_pCollectTm->SetInterval((g_tFTPInfo.uCollectInterval * 60 -offset +12)*1000);//通过测试，bbu每15分钟最多提前12秒，故加上
            #else
            m_pCollectTm->SetInterval((g_tFTPInfo.uCollectInterval * 60 -offset)*1000);
            #endif
        }
        m_pCollectTm->Start();
        break;
    }
    case M_OAM_PERIOD_PERFDATA_LOGFILE_UPLOAD_TIMER://周期性能日志上传
        if(NULL!=m_pWaitColTm)
        {
            m_pWaitColTm->Stop();
            m_pWaitColTm->SetInterval(((m_uBTSId%300)+1)*2*1000);
            m_pWaitColTm->Start();
        }
        
        tTimeData = bspGetDateTime();
        m_pReportTm->Stop();       
        if((tTimeData.minute%g_tFTPInfo.uRqtInterval)>10)//提前了
        {
            offset =60 - tTimeData.second + (g_tFTPInfo.uRqtInterval - tTimeData.minute%g_tFTPInfo.uRqtInterval -1)*60;     
            #ifdef WBBU_CODE
            m_pReportTm->SetInterval((g_tFTPInfo.uRqtInterval * 60 + offset +12) *1000);//通过测试，bbu每15分钟最多提前12秒，故加上
            #else
            m_pReportTm->SetInterval((g_tFTPInfo.uRqtInterval * 60 + offset) *1000);
            #endif
        }
        else
        {
            offset = tTimeData.second + (tTimeData.minute%g_tFTPInfo.uRqtInterval)*60;
            #ifdef WBBU_CODE
            m_pReportTm->SetInterval((g_tFTPInfo.uRqtInterval * 60 - offset+12)*1000);  //通过测试，bbu每15分钟最多提前12秒，故加上
            #else
            m_pReportTm->SetInterval((g_tFTPInfo.uRqtInterval * 60 - offset)*1000);        
            #endif
        }
        m_pReportTm->Start();
	    break;
    case M_OAM_PERIOD_PERFDATA_WAIT_UPLOAD_TIMER:
        OAM_LOGSTR(LOG_DEBUG2, L3PM_ERROR_WRITE_FILE, "begin to upload stat data now...");
        if(NULL!=m_pWaitColTm)	
            m_pWaitColTm->Stop();
        PM_PerfDataFileUploadTimer();
        if(m_CpeSignalFileInfo.isCpeSigFileExist==TRUE)
            PM_SignalDataFileUploadTimer();
        //new stat
        PM_PerfDataFileUploadTimer_new();
        break;
    case M_OAM_PERIOD_PERFDATA_WAIT_TIMER://等待统计开始时间到,启动相关定时器     
        OAM_LOGSTR(LOG_DEBUG2, L3PM_ERROR_WRITE_FILE, "begin to stat data now...");
        if(NULL!=m_pWaitTM)
            m_pWaitTM->Stop();
        
        tTimeData = bspGetDateTime();        
        if(NULL!=m_pCollectTm)
        {
            if((tTimeData.minute%g_tFTPInfo.uCollectInterval)>10)//提前了
            {
                offset =60 - tTimeData.second + (g_tFTPInfo.uCollectInterval - tTimeData.minute%g_tFTPInfo.uCollectInterval -1)*60;             
                #ifdef WBBU_CODE
                m_pCollectTm->SetInterval((g_tFTPInfo.uCollectInterval * 60 + offset +12)*1000);
                #else
                m_pCollectTm->SetInterval((g_tFTPInfo.uCollectInterval * 60 + offset)*1000);
                #endif
            }
            else
            {
                offset = tTimeData.second + (tTimeData.minute%g_tFTPInfo.uCollectInterval)*60;
                #ifdef WBBU_CODE
                m_pCollectTm->SetInterval((g_tFTPInfo.uCollectInterval * 60 - offset+12)*1000); 
                #else                
                m_pCollectTm->SetInterval((g_tFTPInfo.uCollectInterval * 60 - offset)*1000);            
                #endif
            }            
            m_pCollectTm->Start();
        }
        if(NULL!=m_pReportTm)
        {
            if((tTimeData.minute%g_tFTPInfo.uRqtInterval)>10)//提前了
            {
                offset =60 - tTimeData.second + (g_tFTPInfo.uRqtInterval - tTimeData.minute%g_tFTPInfo.uRqtInterval -1)*60; 
                #ifdef WBBU_CODE
                m_pReportTm->SetInterval((g_tFTPInfo.uRqtInterval * 60 + offset+ 12)*1000);
                #else
                m_pReportTm->SetInterval((g_tFTPInfo.uRqtInterval * 60 + offset)*1000);
                #endif
            }
            else
            {
                offset = tTimeData.second + (tTimeData.minute%g_tFTPInfo.uRqtInterval)*60;
                #ifdef WBBU_CODE
                m_pReportTm->SetInterval((g_tFTPInfo.uRqtInterval * 60 - offset+12)*1000); 
                #else
                m_pReportTm->SetInterval((g_tFTPInfo.uRqtInterval * 60 - offset)*1000);  
                #endif
            }
            m_pReportTm->Start();
        }
        m_tRcdHead.uEndTime = GetSystemTime();
        m_bPeriodCollect = true;
        MakePerfFileInfo();
        PM_MakeCpeSignalFile();
        //new stat
        m_tRcdHead_new.uEndTime = GetSystemTime();
        MakePerfFileInfo_new();
        PM_ClearDataNew();        
	    break;
    case M_L3TOL2_DOWN_PERFDATA_RSP:
        PM_L2PerfDataRsp(rMsg);
        break;
    case M_BTS_EMS_UT_PERF_DATA_RSP:
        PM_CPEPerfDataRsp(rMsg);
        break;

        //请求数据超时消息
    case M_L2_PERIOD_PERF_REQ_TIMEOUT_NOTIFY:        //0x0291        //L2数据请求超时
        SetTimerInfo( true, *(UINT16*)rMsg.GetDataPtr(), L2_PERIOD_TIMER, false );
        break;
    case M_L2_CUR_PERF_REQ_TIMEOUT_NOTIFY:        //0x0293        //L2数据请求超时
        SetTimerInfo( true, *(UINT16*)rMsg.GetDataPtr(), L2_CUR_TIMER, false );
        break;
    case M_CPE_CUR_PERF_REQ_TIMEOUT_NOTIFY:        //0x0294        //CPE数据请求超时
        SetTimerInfo( true, *(UINT16*)rMsg.GetDataPtr(), CPE_CUR_TIMER, false );
        break;
    case M_EMS_BTS_BTS_USER_REQ://EMS查询基站用户数
	PM_BtsUserReq(rMsg);
       break;
    case M_L2TOL3_BTS_USER_RSP:
	PM_BtsUserRsp(rMsg);
	break;	
    case M_CPE_REPORT_SIGNAL:
	PM_CPEReportSignal(rMsg);
	break;
    //new stat
    case M_L3TOL2_PERFDATA_RSP_new:
        PM_L2PerfDataRsp_new(rMsg);
        break;
#ifdef WBBU_CODE
		/*************************************
1)L3层每min向L2发送M_L3TOL2_ACTIVE_USER_INFO_LIST_REQ的消息；


		**************************************/
    case M_BOOT_ONE_Minter_Timer:
        SendMessage( M_L3TOL2_ACTIVE_USER_INFO_LIST_REQ, M_TID_L2OAM, 2, (UINT8*)rMsg.GetDataPtr() );
         g_minute_time++;
	break;
#endif
    default:
        break;
    }
    return true;
}
void CTaskPM :: GetFTPInfo( T_BTSPM_FTPINFO& rFtpInfo, CMessage& rMsg )
{
    memset( (UINT8*)&rFtpInfo, 0, sizeof(T_BTSPM_FTPINFO) );
    m_TransationID = *(UINT16*)rMsg.GetDataPtr();
    rFtpInfo.uIPAddr = *((UINT16*)rMsg.GetDataPtr()+1)*65536 + *((UINT16*)rMsg.GetDataPtr()+2);
    rFtpInfo.uPortNum = *((UINT16*)rMsg.GetDataPtr()+3);
    rFtpInfo.UserLen = *((UINT8*)rMsg.GetDataPtr()+8);
    if( 0 != rFtpInfo.UserLen )
        memcpy( rFtpInfo.chUserName, (UINT8*)rMsg.GetDataPtr()+9 , rFtpInfo.UserLen );

    rFtpInfo.PassLen = *((UINT8*)rMsg.GetDataPtr() + 8 + 1 + rFtpInfo.UserLen);
    if( 0 != rFtpInfo.PassLen )
        memcpy( rFtpInfo.chPassWord, (UINT8*)rMsg.GetDataPtr()+8+1+rFtpInfo.UserLen+1, rFtpInfo.PassLen );

    rFtpInfo.uRqtInterval = *((UINT8*)rMsg.GetDataPtr()+8+1+rFtpInfo.UserLen+1+rFtpInfo.PassLen)*256 + 
        *((UINT8*)rMsg.GetDataPtr()+8+1+rFtpInfo.UserLen+1+rFtpInfo.PassLen+1);
    rFtpInfo.uCollectInterval = *((UINT8*)rMsg.GetDataPtr()+8+1+rFtpInfo.UserLen+1+rFtpInfo.PassLen+2)*256 + 
        *((UINT8*)rMsg.GetDataPtr()+8+1+rFtpInfo.UserLen+1+rFtpInfo.PassLen+3);
}
bool CTaskPM :: StartCollectTm()
{
    CComMessage *pMsgCol = new ( this, 0 ) CComMessage;
    CMessage *pMsgConTainer = new CMessage( pMsgCol );
    if (pMsgCol!=NULL)
    {
        pMsgCol->SetDstTid   ( M_TID_PM );
        pMsgCol->SetSrcTid( M_TID_PM );
        pMsgCol->SetMessageId( M_OAM_PERIOD_PERFDATAQUERY_TIMER );
        m_pCollectTm = new  CTimer( true, /*g_tFTPInfo.uCollectInterval **/ 10000, *pMsgConTainer );    //设置一分启动
        if(NULL==m_pCollectTm)
        {
            pMsgConTainer->DeleteMessage();
            return false;
        }
        return true;
    }
    return false;
}
bool CTaskPM :: StartReportTm()
{
    CComMessage* pMsgRpt = new ( this, 0 ) CComMessage;
    CMessage *pMsgConTainer = new CMessage( pMsgRpt );
    if (pMsgRpt!=NULL)
    {
        pMsgRpt->SetDstTid   ( M_TID_PM );
        pMsgRpt->SetSrcTid( M_TID_PM );
        pMsgRpt->SetMessageId( M_OAM_PERIOD_PERFDATA_LOGFILE_UPLOAD_TIMER );
        m_pReportTm = new  CTimer( true, /*g_tFTPInfo.uRqtInterval **/ 90000, *pMsgConTainer );    //
        if(NULL==m_pReportTm)
        {
            pMsgConTainer->DeleteMessage();
            return false;
        }
        return true;
    }
    return false;
}
/*统计延时,为了在统计周期的倍数开始统计jy080923*/
bool CTaskPM :: StartWaitTm()
{
    CComMessage* pMsgWait = new ( this, 0 ) CComMessage;
    CMessage *pMsgConTainer = new CMessage( pMsgWait);
    if (pMsgWait!=NULL)
    {
        pMsgWait->SetDstTid   ( M_TID_PM );
        pMsgWait->SetSrcTid( M_TID_PM );
        pMsgWait->SetMessageId( M_OAM_PERIOD_PERFDATA_WAIT_TIMER );
        m_pWaitTM= new  CTimer( true, /*g_tFTPInfo.uRqtInterval **/ 90000, *pMsgConTainer );    //
        if(NULL==m_pWaitTM)
        {
            pMsgConTainer->DeleteMessage();
            return false;
        }
        return true;
    }
    return false;
}
/*上报延时,延迟一段时间上报,防止数据拥塞jy080923*/
bool CTaskPM :: StartWaitColTm()
{
    CComMessage* pMsgRpt = new ( this, 0 ) CComMessage;
    CMessage *pMsgConTainer = new CMessage( pMsgRpt );
    if (pMsgRpt!=NULL)
    {
        pMsgRpt->SetDstTid   ( M_TID_PM );
        pMsgRpt->SetSrcTid( M_TID_PM );
        pMsgRpt->SetMessageId( M_OAM_PERIOD_PERFDATA_WAIT_UPLOAD_TIMER );
        m_pWaitColTm= new  CTimer( true, /*g_tFTPInfo.uRqtInterval **/ 90000, *pMsgConTainer );    //
        if(NULL==m_pWaitColTm)
        {
            pMsgConTainer->DeleteMessage();
            return false;
        }
        return true;
    }
    return false;
}
bool CTaskPM :: DelPerfFile()
{
    if( chdir(SM_CPE_PERF_DIR) )
        return false;
    if( 0 != strlen(m_tFileInfo.BTSRsvPerfFile) && m_tFileInfo.isBTSFileExist )
        RemoveFile( m_tFileInfo.BTSRsvPerfFile );
    if( 0 != strlen(m_tFileInfo.BTSPerfFile) && m_tFileInfo.BTSPerfFile )
        RemoveFile( m_tFileInfo.BTSPerfFile );

    if( 0 != strlen(m_tFileInfo.CPERsvPerfFile) && m_tFileInfo.isCPEFileExist )
        RemoveFile( m_tFileInfo.CPERsvPerfFile );
    if( 0 != strlen(m_tFileInfo.CPEPerfFile) && m_tFileInfo.CPEPerfFile )
        RemoveFile( m_tFileInfo.CPEPerfFile );

    memset( &m_tFileInfo, 0, sizeof(m_tFileInfo) );
    return true;
}
bool CTaskPM :: MakePerfFileInfo()
{
    char chTmp[10], chTm[20], chTail[30];
    memset( chTmp, 0, 10 );
    memset( chTm, 0, 20 );
    memset( chTail, 0, 30 );
    GetFileTime( chTm );

    sprintf( chTmp, "%08d", bspGetBtsID() );

    memset( chTail, 0, 30 );
    memcpy( chTail, chTmp, strlen(chTmp) );

    strcat( chTail, "." );
    strcat( chTail, chTm );
    strcat( chTail, ".perf" );

    memset( (UINT8*)&m_tFileInfo.BTSPerfFile, 0, sizeof(m_tFileInfo.BTSPerfFile) );
    memcpy( m_tFileInfo.BTSPerfFile, "BTS.", 4 );
    strcat( m_tFileInfo.BTSPerfFile, chTail );

    memset( (UINT8*)&m_tFileInfo.CPEPerfFile, 0, sizeof(m_tFileInfo.CPEPerfFile) );
    memcpy( m_tFileInfo.CPEPerfFile, "CPE.", 4 );
    strcat( m_tFileInfo.CPEPerfFile, chTail );

    m_tFileInfo.isBTSFileExist = false;
    m_tFileInfo.isCPEFileExist = false;
    return true;
}
bool CTaskPM :: WriteToFile( bool bBTS,                //是否写入BTS文件
                               UINT16 uEntType,        //实体类型
                               unsigned char* chData, //写入的数据
                               unsigned int uDataLen )//写入的数据长度

{
    if( NULL == chData )
        return false;

    FILE *stream;
    int numwritten;

    DIR* pdir = opendir( SM_CPE_PERF_DIR );
    if ( NULL == pdir )
    {
        return false;
    }
    else
        closedir( pdir );
    char chTmp[L3_BTS_PM_ABSTRACT_FILENAME_LEN];
    if( bBTS )
        stream = fopen( GetAbstractFile(m_tFileInfo.BTSPerfFile, chTmp), "ab" );
    else
        stream = fopen( GetAbstractFile(m_tFileInfo.CPEPerfFile, chTmp), "ab" );

    if( stream != NULL )
    {
        m_tRcdHead.EntType = uEntType;
        m_tRcdHead.DataLen = uDataLen;// + sizeof( m_tRcdHead );
        if( BTSPM_ENT_CPE == uEntType )
        {
            UINT32 secCount, ulEndTm, ulStartTm;
	     //获得时间信息,将时间修改为上报周期的整数倍jy080923
	     secCount = GetCpeReportTime();            
            ulEndTm = m_tRcdHead.uEndTime;
            ulStartTm = m_tRcdHead.uStartTime;
            m_tRcdHead.uEndTime = secCount;               
            m_tRcdHead.uStartTime = secCount - 60 * g_tFTPInfo.uCollectInterval;
            numwritten = fwrite( (char*)&m_tRcdHead, sizeof( char ), 20, stream );
            m_tRcdHead.uEndTime = ulEndTm;
            m_tRcdHead.uStartTime = ulStartTm;
        }
        else
            numwritten = fwrite( (char*)&m_tRcdHead, sizeof( char ), 20, stream );

        numwritten = fwrite( chData, sizeof( char ), uDataLen, stream );
        fclose( stream );

        if( bBTS )
            m_tFileInfo.isBTSFileExist = true;
        else
            m_tFileInfo.isCPEFileExist = true;

        return true;
    }
    else
    {
        return false;
    }
}

void CTaskPM :: GetFileTime( char * chVal )
{    
    time_t tm = m_tRcdHead.uEndTime;
    struct tm *time = ::localtime( &tm );

    ::strftime( chVal, 20, "%m%d%y%H%M", time );
}

void CTaskPM :: SetTimerInfo( bool bTimeoutNotify,  //是否为超时通知
                              UINT16& uTransID,     //uTransID
                              UINT8 uIndex,         //哪一个定时器
                              bool bPeriod )        //是否进入周期采集状态                            
{
    if( bTimeoutNotify )    //超时通知
    {
        if( PERIOD_TRANSACTIONID != uTransID )
        {
            UINT8 chTmp[12];
            memset( chTmp, 0, 12 );
            *((UINT16*)chTmp) = m_TimerInfo[uIndex].uTransID;// | 0x8000;
            *((UINT16*)chTmp+1) = ERROR_CODE;
            switch (uIndex)
            {
            case L2_CUR_TIMER:
                *((UINT16*)chTmp+2) = m_TimerInfo[uIndex].uType;
                memset( (UINT8*)chTmp+6, 255, 4 );
                *((UINT16*)chTmp+5) = ERROR_CODE;
                SendMessage( M_BTS_EMS_L2_PERF_DATA_RSP, M_TID_EMSAGENTTX, 12, chTmp );
                break;
            case CPE_CUR_TIMER:
                memset( (UINT8*)chTmp+4, 255, 4 );
                SendMessage( M_BTS_EMS_UT_PERF_DATA_RSP, M_TID_EMSAGENTTX, 8, chTmp );
                break;
            }
        }
	if(NULL!=m_TimerInfo[uIndex].pTimer)
            m_TimerInfo[uIndex].pTimer->Stop();	
        m_TimerInfo[uIndex].bTimerStart = false;
    }

    if( ! bTimeoutNotify )    //非超时通知情况下，启动定时消息
    {
        if(NULL!=m_TimerInfo[uIndex].pTimer)
            m_TimerInfo[uIndex].pTimer->Start();
        m_TimerInfo[uIndex].bTimerStart = true;
        m_TimerInfo[uIndex].uTransID = uTransID;
    }
}

void CTaskPM :: PM_PerfLogCfgReq( CMessage& rMsg )
{
    UINT8 ucTmp[4];
    T_TimeDate tTimeData;
    UINT32 waitinterval;
    memset( ucTmp, 0, 4 );
    *(UINT16*)ucTmp = * (UINT16*)rMsg.GetDataPtr();// | 0x8000;

    T_BTSPM_FTPINFO tFtpInfo;
    GetFTPInfo( tFtpInfo, rMsg);
    for( UINT16 us=0; us<sizeof(T_BTSPM_FTPINFO); )
    {
        if( *((UINT8*)&tFtpInfo + us) != *((UINT8*)&g_tFTPInfo + us) )
            break;
        if( ++us == sizeof(T_BTSPM_FTPINFO) )
        {        
            *((UINT16*)ucTmp + 1) = 0;
            SendMessage( M_BTS_EMS_PERF_LOGGING_CFG_RSP, M_TID_CM, 4, ucTmp );
            return;
        }
    }
    memcpy( (UINT8*)&g_tFTPInfo, (UINT8*)&tFtpInfo, sizeof(T_BTSPM_FTPINFO) );


    if( 0 != g_tFTPInfo.uIPAddr )//IP地址不为零，合法数据
    {
        if( g_tFTPInfo.uRqtInterval==0 || g_tFTPInfo.uCollectInterval==0 )    //非周期采集指令
        {
            if( m_bPeriodCollect )    //停止周期性的采集
            {
                if(NULL!=m_pCollectTm)
                    m_pCollectTm->Stop();
                if(NULL!=m_pReportTm)
                    m_pReportTm->Stop();    
                if(NULL!=m_pWaitColTm)
                    m_pWaitColTm->Stop();
                if(NULL!=m_pWaitTM)
                    m_pWaitTM->Stop();
                if( 0 != strlen(m_tFileInfo.BTSRsvPerfFile) && m_tFileInfo.isBTSFileExist )        //处理保留的BTS文件
                {
                    SendPerfFile( m_tFileInfo.BTSRsvPerfFile );
                    
                    remove( m_tFileInfo.BTSRsvPerfFile );
                    memset( m_tFileInfo.BTSRsvPerfFile, 0, 30 );
                }
                if( 0 != strlen(m_tFileInfo.BTSPerfFile) && m_tFileInfo.isBTSFileExist )        //处理保留的BTS文件
                {
                    SendPerfFile(m_tFileInfo.BTSPerfFile);
                    remove( m_tFileInfo.BTSPerfFile );
                    memset( m_tFileInfo.BTSPerfFile, 0, 30 );
                }
                if( 0 != strlen(m_tFileInfo.CPERsvPerfFile) && m_tFileInfo.isCPEFileExist )        //处理保留的CPE文件
                {
                    SendPerfFile( m_tFileInfo.CPERsvPerfFile );
                    remove( m_tFileInfo.CPERsvPerfFile );
                    memset( m_tFileInfo.CPERsvPerfFile, 0, 30 );
                }
                if( 0 != strlen(m_tFileInfo.CPEPerfFile) && m_tFileInfo.isCPEFileExist )        //处理保留的CPE文件
                {
                    SendPerfFile( m_tFileInfo.CPEPerfFile );
                    remove( m_tFileInfo.CPEPerfFile );
                    memset( m_tFileInfo.CPEPerfFile, 0, 30 );
                }
                //new stat
                if( 0 != strlen(m_tFileInfo_new.BTSRsvPerfFile) && m_tFileInfo_new.isBTSFileExist )        //处理保留的BTS文件
                {
                    SendPerfFile_new( m_tFileInfo_new.BTSRsvPerfFile );
                    
                    remove( m_tFileInfo_new.BTSRsvPerfFile );
                    memset( m_tFileInfo_new.BTSRsvPerfFile, 0, 40 );
                }
                if( 0 != strlen(m_tFileInfo_new.BTSPerfFile) && m_tFileInfo_new.isBTSFileExist )        //处理保留的BTS文件
                {
                    SendPerfFile_new(m_tFileInfo_new.BTSPerfFile);
                    remove( m_tFileInfo_new.BTSPerfFile );
                    memset( m_tFileInfo_new.BTSPerfFile, 0, 40 );
                }
                m_bPeriodCollect = false;
            }
        }
        else
        {       
            DIR* pdir = opendir( SM_CPE_PERF_DIR );
            if( NULL == pdir )
            {
                OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "L3 OAM PM DIR --- NA", 0);
                return;
            }
            else
            {
                closedir( pdir );
            }
            if( m_bPeriodCollect )
            {
                DelPerfFile();
                DelPerfFile_new();
                if(NULL!=m_pCollectTm)
                    m_pCollectTm->Stop();
                if(NULL!=m_pReportTm)
                    m_pReportTm->Stop();    
                if(NULL!=m_pWaitColTm)
                    m_pWaitColTm->Stop();
                if(NULL!=m_pWaitTM)
                    m_pWaitTM->Stop();
            }
            StartWaitTm();
            if(NULL!=m_pWaitTM)
                m_pWaitTM->Stop();
            if(NULL!=m_pCollectTm)
                m_pCollectTm->SetInterval( g_tFTPInfo.uCollectInterval * 1000 * 60 );
            if(NULL!=m_pReportTm)
                m_pReportTm->SetInterval( g_tFTPInfo.uRqtInterval * 1000 * 60 );
            tTimeData = bspGetDateTime();
            //等待的秒数:(1小时-目前已过秒数)%设置间隔秒数jy080923
            waitinterval = (60*60-(tTimeData.minute*60+tTimeData.second))%(g_tFTPInfo.uRqtInterval *60);
            if(NULL!=m_pWaitTM)
            {
                m_pWaitTM->SetInterval(waitinterval*1000);
                m_pWaitTM->Start();//启动等待定时器jy080923      
            }
        }
    }
    else
    {
        LOG( LOG_SEVERE, L3PM_ERROR_FTP_INFO_MSG, "FTP IP is ZERO" );
        *((UINT16*)ucTmp + 1) = 0;
    }
    SendMessage( M_BTS_EMS_PERF_LOGGING_CFG_RSP, M_TID_CM, 4, ucTmp );
    
}

void CTaskPM :: PM_L2PerfDataQueryReq( CMessage& rMsg )
{    
    SetTimerInfo( false, *(UINT16*)rMsg.GetDataPtr(), L2_CUR_TIMER, false );
    SendMessage( M_L3TOL2_DOWN_PERFDATA_REQ, M_TID_L2OAM, rMsg.GetDataLength(), (UINT8*)rMsg.GetDataPtr() );    //send meg to L2
} 

void CTaskPM :: PM_L3PerfDataQueryReq( CMessage& rMsg )
{
    T_L3PerfData *pPerfData;
    UINT8 chTmp[1200];
    memset( chTmp, 0, 1200 );
    pPerfData = (T_L3PerfData *)(chTmp+4+2);
    *(UINT16*)chTmp = *((UINT16*)rMsg.GetDataPtr());// | 0x8000;
    *((UINT16*)chTmp+1) = 0;// | 0x8000;
    *((UINT16*)chTmp+2) = 0;// | 0x8000;

    GetL3PerfData( pPerfData ); 
    SendMessage( M_BTS_EMS_L3_PERF_DATA_RSP, M_TID_EMSAGENTTX , 4 + 2 + sizeof(T_L3PerfData), chTmp );
}
void CTaskPM :: PM_CPEPerfDataQueryReq( CMessage& rMsg )
{
    SetTimerInfo( false, *(UINT16*)rMsg.GetDataPtr(), CPE_CUR_TIMER, false );
    CComMessage *RspMsg = new ( this, rMsg.GetDataLength() ) CComMessage;
    if( RspMsg != NULL )
    {
        RspMsg->SetDstTid( M_TID_CPECM );
        RspMsg->SetMessageId( M_EMS_BTS_UT_PERF_DATA_REQ );
        RspMsg->SetSrcTid( this->GetEntityId() );
        UINT32 iii = *((UINT32*)((UINT16*)rMsg.GetDataPtr()+1));
        OAM_LOGSTR1(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_CPEPerfDataQueryReq --- EID %08X", iii );

        RspMsg->SetEID( iii );
        memcpy( (UINT8*)RspMsg->GetDataPtr(), (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength() );
        if( ! CComEntity :: PostEntityMessage( RspMsg ) )
            RspMsg->Destroy();
    }
}


void CTaskPM :: PM_L2PerfDataRsp( CMessage& rMsg )
{
    if( PERIOD_TRANSACTIONID == *((UINT16*)rMsg.GetDataPtr()) )        //周期性数据
    {
        if( 0 == *((UINT16*)rMsg.GetDataPtr()+5) )                //返回有效
        {
            if ( NULL == m_tRsrvPerfData.pL2PerfData )            //未曾分配L2性能数据的内存
            {
                m_tRsrvPerfData.pL2PerfData = new T_L2PerfDataEle();
		  if(NULL!=m_tRsrvPerfData.pL2PerfData )
                    memcpy( m_tRsrvPerfData.pL2PerfData, (T_L2PerfDataEle*)((UINT8*)rMsg.GetDataPtr()+12), sizeof(T_L2PerfDataEle) );
            }
            else
            {
                GetCrntPerfData( (UINT32*)m_tRsrvPerfData.pL2PerfData, (UINT32*)((UINT8*)rMsg.GetDataPtr()+12), sizeof(T_L2PerfDataEle)/4 );
                WriteToFile( true, BTSPM_ENT_BTS_L2, (UINT8*)rMsg.GetDataPtr()+2, sizeof(T_L2PerfDataEle)+10 );
            }
        }
    }
    else 
    {
        UINT8 *ucTmp = new UINT8[ 12 + 2 + rMsg.GetDataLength() ]; //TransId后面加上result
        if(NULL!=ucTmp)
        {
            memcpy( ucTmp+2, (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength() );
            *( (UINT16*)ucTmp ) = *((UINT16*)rMsg.GetDataPtr());
            *( (UINT16*)ucTmp + 1 ) = 0;
            SendMessage( M_BTS_EMS_L2_PERF_DATA_RSP, M_TID_EMSAGENTTX, 12 + 2 + sizeof(T_L2PerfDataEle), ucTmp );
    	     delete [] ucTmp;
        }
    }
    if(NULL!=m_TimerInfo[L2_PERIOD_TIMER].pTimer)
        m_TimerInfo[L2_PERIOD_TIMER].pTimer->Stop();
    m_TimerInfo[L2_PERIOD_TIMER].bTimerStart = false;
}
void CTaskPM :: PM_CPEPerfDataRsp( CMessage& rMsg )
{
    if( PERIOD_TRANSACTIONID == *((UINT16*)rMsg.GetDataPtr()) )        //周期性数据
    {
        if( 0 == *((UINT16*)rMsg.GetDataPtr()+2) )                        //返回有效
        {
            //if应该没有使用
            if ( NULL == m_tRsrvPerfData.pUTPerfData ) 
            {
#ifndef WBBU_CODE
                m_tRsrvPerfData.pUTPerfData = new (T_UTPerfData)[rMsg.GetDataLength()-4];
#else
                m_tRsrvPerfData.pUTPerfData = new (T_UTPerfData)()/*[rMsg.GetDataLength()-4]*/;
#endif
		  //保存应该和写入文件的一致jy081014
		  if(NULL!=m_tRsrvPerfData.pUTPerfData )
                    memcpy( m_tRsrvPerfData.pUTPerfData, (T_UTPerfData*)((UINT8*)rMsg.GetDataPtr()+4), rMsg.GetDataLength()-4 );
            }
            else
            {
                //cpe上周期采集完后清零处理，不需要求差值
                WriteToFile( false, BTSPM_ENT_CPE, (UINT8*)rMsg.GetDataPtr()+4, rMsg.GetDataLength()-4 );//每条记录前放置EID
            }
        }
    }
    else                                                        //查询的数据
    {
        SendMessage(  M_BTS_EMS_UT_PERF_DATA_RSP, M_TID_EMSAGENTTX, rMsg.GetDataLength(), (UINT8*)rMsg.GetDataPtr() );
	 if(NULL!=m_TimerInfo[CPE_CUR_TIMER].pTimer)
            m_TimerInfo[CPE_CUR_TIMER].pTimer->Stop();
        m_TimerInfo[CPE_CUR_TIMER].bTimerStart = false;
    }
}

void CTaskPM :: PM_PerfDataQueryTimer()
{
    m_tRcdHead.uStartTime = m_tRcdHead.uEndTime;
    m_tRcdHead.uEndTime = GetSystemTime();
    
    UINT8* pucL3PerfData;
    pucL3PerfData = new UINT8[ 2 + sizeof(T_L3PerfData) ];
    if(pucL3PerfData==NULL)
		return;
    T_L3PerfData* L3PerfData = (T_L3PerfData*)( pucL3PerfData + 2 );
    GetL3PerfData( L3PerfData );

    if ( NULL == m_tRsrvPerfData.pL3PerfData )
    {
        m_tRsrvPerfData.pL3PerfData = new T_L3PerfData();
	 if(NULL != m_tRsrvPerfData.pL3PerfData )
            memcpy( m_tRsrvPerfData.pL3PerfData, L3PerfData, sizeof(T_L3PerfData) );
    }
    else
    {
        GetCrntPerfData( (UINT32*)m_tRsrvPerfData.pL3PerfData, (UINT32*)L3PerfData, sizeof(T_L3PerfData)/4 );
        WriteToFile( true, BTSPM_ENT_BTS_L3, pucL3PerfData, sizeof(T_L3PerfData)+2 );
    }
    delete [] pucL3PerfData;

    UINT16 usTmp[4];
    memset( (UINT8*)usTmp, 0, 8 );
    *(usTmp) = PERIOD_TRANSACTIONID;

    *(usTmp+1) = 0;
    *((UINT32*)usTmp+1) = 0x0;
    SetTimerInfo( false, *(UINT16*)usTmp, L2_PERIOD_TIMER, true );
    SendMessage( M_L3TOL2_DOWN_PERFDATA_REQ, M_TID_L2OAM, 8, (UINT8*)usTmp );    //send meg to L2
    SendMessage( M_L3TOL2_BTS_RTMONITOR_REQ, M_TID_L2OAM, 8, (UINT8*)usTmp );    //send meg to L2
    OAM_LOGSTR(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "[CTaskPM]    Send RealTime PM Data");
}
void CTaskPM :: PM_PerfDataFileUploadTimer()
{    
    DIR* pdir = opendir( SM_CPE_PERF_DIR ); 
    if( NULL == pdir )
    {
        OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_PerfDataFileUploadTimer --- OPEN DIR ERROR", 0);
        return;
    }
    else
    {
        closedir( pdir );
    }
    //判断条件：文件名长不为零 且 文件存在
    if( 0 != strlen(m_tFileInfo.BTSRsvPerfFile) && m_tFileInfo.isBTSFileExist )                        //保留的BTS性能文件
    {
        PM_UploadTimerDealFile( true, m_tFileInfo.BTSRsvPerfFile, strlen(m_tFileInfo.BTSRsvPerfFile) );
        RemoveFile( m_tFileInfo.BTSRsvPerfFile );
        memset( m_tFileInfo.BTSRsvPerfFile, 0, 30 );
    }
    if( 0 != strlen(m_tFileInfo.BTSPerfFile) && m_tFileInfo.isBTSFileExist )    
    {
        if( PM_UploadTimerDealFile( true, m_tFileInfo.BTSPerfFile, strlen(m_tFileInfo.BTSPerfFile) ) )    //当前BTS性能文件
        {
            RemoveFile( m_tFileInfo.BTSPerfFile );
        }
        else
        {
            OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "m_tFileInfo.PERFFILE ---> UPLOAD ERROR", 0);
            memcpy( m_tFileInfo.BTSRsvPerfFile, m_tFileInfo.BTSPerfFile, 30 );
        }
    }

    if( 0 != strlen(m_tFileInfo.CPERsvPerfFile) && m_tFileInfo.isCPEFileExist )                        //保留的CPE性能文件
    {
        PM_UploadTimerDealFile( false, m_tFileInfo.CPERsvPerfFile, strlen(m_tFileInfo.CPERsvPerfFile) );
        RemoveFile( m_tFileInfo.CPERsvPerfFile );
        memset( m_tFileInfo.CPERsvPerfFile, 0, 30 );
    }
    if( 0 != strlen(m_tFileInfo.CPEPerfFile) && m_tFileInfo.isCPEFileExist )
    {
        if( PM_UploadTimerDealFile( false, m_tFileInfo.CPEPerfFile, strlen(m_tFileInfo.CPEPerfFile) ) )    //当前BTS性能文件
            RemoveFile( m_tFileInfo.CPEPerfFile );
        else
            memcpy( m_tFileInfo.CPERsvPerfFile, m_tFileInfo.CPEPerfFile, 30 );
    }
    MakePerfFileInfo();
}
bool CTaskPM :: PM_UploadTimerDealFile( bool isBTSFile, char* scFileName, UINT8 ucFileNameLen )
{
    UINT8 uTmp[50];
    memset( uTmp, 0, 50 );
    * (UINT16*)uTmp = m_TransationID;//|0x8000;
    //启动定时器保护ftp任务jy081121
    CTaskFileMag *taskSM = CTaskFileMag::GetInstance();    
    taskSM->setFtpUsingFlag(M_TID_PM, TRUE);
    if( SendPerfFile( scFileName ) )
    {
        * (UINT8*)(uTmp + 2) = (isBTSFile) ? 0 : 1;
        * (UINT8*)(uTmp + 3) = ucFileNameLen;
        memcpy( uTmp+4, scFileName, ucFileNameLen );
        SendMessage( M_BTS_EMS_PERF_DATA_UL_OK_NOTIFY, M_TID_EMSAGENTTX, 4+ucFileNameLen, uTmp );
	 taskSM->setFtpUsingFlag(M_TID_PM, FALSE);
        return true;
    }
    else
    {
        taskSM->setFtpUsingFlag(M_TID_PM, FALSE);
        return false;
    }    
}
bool CTaskPM :: SendPerfFile( char* chFileName )
{
    if( NULL == chFileName )
        return false;

    int        ctrlSock;
    int        dataSock;
    int        nBytes;
    char    buf [512];
    STATUS    status;

    struct in_addr Svriaddr;
    Svriaddr.s_addr = g_tFTPInfo.uIPAddr;
    SINT8    IpAddr[16];
    memset( IpAddr, 0, sizeof(IpAddr) );
    inet_ntoa_b( Svriaddr, IpAddr );

    char chTmp[L3_BTS_PM_ABSTRACT_FILENAME_LEN];
    FILE * pFile = fopen( GetAbstractFile( chFileName, chTmp ), "rb");
    if( NULL == pFile )
        return false;
    if( ERROR == ftpXfer( IpAddr, 
                          g_tFTPInfo.chUserName, 
                          g_tFTPInfo.chPassWord, 
                          NULL, 
                          "STOR %s", 
                          PM_UPLOAD_DIR, 
                          chFileName, 
                          &ctrlSock, 
                          &dataSock) )
    {
        LOG( LOG_SEVERE, L3PM_ERROR_FTP_UPLOAD, "FILE UPLOAD ERROR." );
        fclose(pFile);
        return false;
    }

    memset(buf, 0, sizeof(buf));
    while( (nBytes = fread(buf, 1, 512, pFile)) > 0 )
    {
        ::write (dataSock, buf, nBytes );
        memset(buf, 0, sizeof(buf));
    }

    fclose(pFile);
    close (dataSock);

    if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
    {
        status = false;
    }
    if (ftpCommand (ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
    {
        status = false;
    }
    close (ctrlSock);
    return true;
}

void CTaskPM :: SendMessage( UINT16 uMsgID, TID uDstTid, unsigned int uDataLen, UINT8* pData )
{
    CComMessage *RspMsg = new ( this, uDataLen ) CComMessage;
    if( RspMsg != NULL )
    {
        RspMsg->SetDstTid( uDstTid );
        RspMsg->SetMessageId( uMsgID );
        RspMsg->SetSrcTid( this->GetEntityId());
        memcpy( (UINT8*)RspMsg->GetDataPtr(), pData, uDataLen );
        if( ! CComEntity :: PostEntityMessage( RspMsg ) )
            RspMsg->Destroy();
    }
}

void CTaskPM :: GetL3PerfData( T_L3PerfData* pPerfData )
{                       
    if( NULL == pPerfData )
        return;

    GetDataPerfData ( BTS_PERF_TYPE_EB    , (UINT8*)&pPerfData->DATAPerfData.EBGenPerfData  );
    GetDataPerfData ( BTS_PERF_TYPE_SNOOP , (UINT8*)&pPerfData->DATAPerfData.SnoopPerfData  );
    GetDataPerfData ( BTS_PERF_TYPE_DM    , (UINT8*)&pPerfData->DATAPerfData.DMPerfData     );
    GetDataPerfData ( BTS_PERF_TYPE_TUNNEL, (UINT8*)&pPerfData->DATAPerfData.TunnelPerfData );
    GetDataPerfData ( BTS_PERF_TYPE_ARP   , (UINT8*)&pPerfData->DATAPerfData.ARPPerfData    );
    GetDataPerfData ( BTS_PERF_TYPE_TCR   , (UINT8*)&pPerfData->DATAPerfData.TCRPerfData    );
    GetDataPerfData ( BTS_PERF_TYPE_TDR   , (UINT8*)&pPerfData->DATAPerfData.TDRPerfData    );
    GetOAMPerfData  ( (UINT8*)&pPerfData->OAMPerfData   );
    GetVoicePerfData( (UINT8*)&pPerfData->VOICEPerfData );
};
void CTaskPM :: GetCrntPerfData( UINT32* ulRsvPerfData,    //保存的基准值
                                 UINT32* ulStscPerfData,   //采集的值，完成后形成记录值
                                 UINT32 const usLen )      //数据长度
{
    if( NULL == ulRsvPerfData || NULL == ulStscPerfData )
        return;

    UINT32 ulTmp;
    for ( UINT32 i=0; i<usLen; i++ ) 
    {
        if ( (*( ulStscPerfData + i )) >= (*( ulRsvPerfData + i )) ) 
        {
            ulTmp = *( ulStscPerfData + i ) - *( ulRsvPerfData + i );
            *( ulRsvPerfData + i ) = *( ulStscPerfData + i );
            *( ulStscPerfData + i ) = ulTmp;
        }
        else
        {
            *( ulRsvPerfData + i ) = *( ulStscPerfData + i );
            *( ulStscPerfData + i ) = 0;
        }
    }
};

bool CTaskPM :: RemoveFile( char* chFileName )
{
    DIR* pdir = opendir( SM_CPE_PERF_DIR );
    if( NULL != pdir )
    {
        closedir( pdir );
        char chTmp[L3_BTS_PM_ABSTRACT_FILENAME_LEN];
        FILE * pFile = fopen( GetAbstractFile( chFileName, chTmp ), "rb");
        if( NULL != pFile )
        {
            fclose( pFile );
            remove( GetAbstractFile( chFileName, chTmp ) );
        }
        return true;
    }
    return false;
}

UINT32 GetSystemTime()
{
    T_TimeDate tTimeData;
    struct tm time_s;
    UINT32 secCount;

    tTimeData = bspGetDateTime();
    time_s.tm_sec = tTimeData.second;
    time_s.tm_min = tTimeData.minute;
    time_s.tm_hour = tTimeData.hour;
    time_s.tm_mday = tTimeData.day;
    time_s.tm_mon = tTimeData.month - 1;
    time_s.tm_year = tTimeData.year - 1900;
    time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

    secCount = mktime(&time_s);
    return secCount;
}
/*修改收到的时间,因为终端已经延迟了一段时间,所以进行修正jy080923
*/
UINT32 GetCpeReportTime()
{
    T_TimeDate tTimeData;
    struct tm time_s;
    UINT32 secCount;

    tTimeData = bspGetDateTime();
    time_s.tm_sec = 0;
    //去掉终端延迟时间
    if(g_tFTPInfo.uCollectInterval!=0)
        time_s.tm_min = tTimeData.minute - tTimeData.minute%g_tFTPInfo.uCollectInterval;
    time_s.tm_hour = tTimeData.hour;
    time_s.tm_mday = tTimeData.day;
    time_s.tm_mon = tTimeData.month - 1;
    time_s.tm_year = tTimeData.year - 1900;
    time_s.tm_isdst = 0;   /* +1 Daylight Savings Time, 0 No DST, * -1 don't know */

    secCount = mktime(&time_s);
    return secCount;
}
/*查询基站当前服务数据用户数和活动用户数
其中活动用户数由二层查询*/
void CTaskPM :: PM_BtsUserReq( CMessage& rMsg )
{        
    SendMessage( M_L3TOL2_BTS_USER_REQ, M_TID_L2MAIN, rMsg.GetDataLength(), (UINT8*)rMsg.GetDataPtr() );    //send meg to L2
} 
/*收到二层活动用户数应答后,查询当前服务的数据用户数,然后
汇总后发给ems
*/
void CTaskPM :: PM_BtsUserRsp( CMessage& rMsg )
{    
    T_L3DATAEBGenPerfData   EBGenPerfData;
    UINT32 serving_users;

    GetDataPerfData ( BTS_PERF_TYPE_EB    , (UINT8*)&EBGenPerfData );
    serving_users = EBGenPerfData.ServingUsers;
    //将发往的ip地址放在commessage最后
    CComMessage* pComMsg = new (this, rMsg.GetDataLength()) CComMessage;
    if (pComMsg==NULL)
    {
        LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in PM_BtsUserRsp.");
        return ;
    }
    pComMsg->SetDstTid(M_TID_EMSAGENTTX);
    pComMsg->SetSrcTid(M_TID_PM);    
    pComMsg->SetMessageId(M_BTS_EMS_BTS_USER_RSP);
    memcpy(((UINT8*)(pComMsg->GetDataPtr())), ((UINT8*)(rMsg.GetDataPtr())), rMsg.GetDataLength());    
    *(UINT32*)((SINT8*)pComMsg->GetDataPtr()+4) = serving_users;
    	
    if(!CComEntity::PostEntityMessage(pComMsg))
    {
        pComMsg->Destroy();
        pComMsg = NULL;
    }
} 

//
void CTaskPM ::PM_MakeCpeSignalFile()
{
    char chTmp[10], chTm[20], chTail[50];
    
    memset( chTmp, 0, 10 );
    memset( chTm, 0, 20 );
    memset( chTail, 0, 50 );
    GetFileTime( chTm );
    sprintf( chTmp, "%08d", bspGetBtsID() );
    memcpy( chTail, chTmp, strlen(chTmp) );
    
    strcat( chTail, "." );
    strcat( chTail, chTm );
    strcat( chTail, ".Sig" );

    memset( m_CpeSignalFileInfo.CPESignalFile, 0, 100 );
    memcpy( m_CpeSignalFileInfo.CPESignalFile, "Mem.", 4 );
    strcat( m_CpeSignalFileInfo.CPESignalFile, chTail);
    m_CpeSignalFileInfo.isCpeSigFileExist = false;
}
void CTaskPM ::PM_CPEReportSignal( CMessage& rMsg )
{    
    FILE *stream;
    int numwritten;   
    //测试目录是否有效
    DIR* pdir = opendir( SM_CPE_SIG_DIR );
    if ( NULL == pdir )
    {
        if(OK!=mkdir(SM_CPE_SIG_DIR))
  	 {
  		//printf("\n mkdir:%s fail...\n",CF_CDR_DIR);
  		OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_CPEReportSignal --- MAKE DIR ERROR", 0);
  		return;
  	 }
  	 closedir( pdir );      
    }
    else
        closedir( pdir );
    char chTmp[L3_BTS_PM_ABSTRACT_FILENAME_LEN];
    memset( chTmp, 0, L3_BTS_PM_ABSTRACT_FILENAME_LEN );
    strcpy( chTmp, SM_CPE_SIG_DIR );

    if(strlen(m_CpeSignalFileInfo.CPESignalFile)==0)//没有生成文件名
        PM_MakeCpeSignalFile();		
    strcat( chTmp, m_CpeSignalFileInfo.CPESignalFile);
    stream = fopen(chTmp, "ab" );

    if( stream != NULL )
    { 	
    	if(m_CpeSignalFileInfo.isCpeSigFileExist==false)
	{
		SIG_FILE_HEAD fileHead;
		fileHead.headLen = 6;
		fileHead.version_type = 2; 
		fileHead.Eid = 0xffffffff;
		numwritten = fwrite( (char*)&fileHead, sizeof( char), sizeof(SIG_FILE_HEAD), stream );
	}
	 m_signalHead.timeHead = 0;/*凑8字节,前面填0,因为时间不重要*/
	 m_signalHead.uEndTime = GetSystemTime();	 
        m_signalHead.DataLen = rMsg.GetDataLength()+16;
	 m_signalHead.flameNum = 0;
	 m_signalHead.seqNum = 0;
	 m_signalHead.version = 2;
	 m_signalHead.type = 4;
	 m_signalHead.len = rMsg.GetDataLength();
	 m_signalHead.rev = 0;
        numwritten = fwrite( (char*)&m_signalHead, sizeof( char), sizeof(SIGNAL_HEAD), stream );
        numwritten = fwrite( rMsg.GetDataPtr(), sizeof( char ), m_signalHead.len, stream );
        fclose( stream );
         m_CpeSignalFileInfo.isCpeSigFileExist = true;
        return;
    }
    else
    {
        OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_CPEReportSignal --- OPEN file ERROR", 0);
        return;
    }
}

void CTaskPM::PM_SignalDataFileUploadTimer()
{    
    DIR* pdir = opendir( SM_CPE_SIG_DIR ); 
    if( NULL == pdir )
    {
        OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_SignalDataFileUploadTimer --- OPEN DIR ERROR", 0);
        return;
    }
    else
    {
        closedir( pdir );
    }
    //判断条件：文件名长不为零 且 文件存在
    if( 0 != strlen(m_CpeSignalFileInfo.CPESignalFile) && m_CpeSignalFileInfo.isCpeSigFileExist)                        //保留的BTS性能文件
    {
        sendSignalDataFile();
	 char chTmp[L3_BTS_PM_ABSTRACT_FILENAME_LEN];
        remove( GetAbstractSigFile( m_CpeSignalFileInfo.CPESignalFile, chTmp ));
        memset( m_CpeSignalFileInfo.CPESignalFile, 0, 100 );
    }  
    
    PM_MakeCpeSignalFile();//信令数据上报间隔是采集周期+延迟上报时间
}

void CTaskPM::sendSignalDataFile()
{    
    int        ctrlSock;
    int        dataSock;
    int        nBytes;
    char    buf [512];
    STATUS    status;	
    
    struct in_addr Svriaddr;
    Svriaddr.s_addr = g_tFTPInfo.uIPAddr;
    SINT8    IpAddr[16];
    memset( IpAddr, 0, sizeof(IpAddr) );
    inet_ntoa_b( Svriaddr, IpAddr );

    char chTmp[L3_BTS_PM_ABSTRACT_FILENAME_LEN];
    FILE * pFile = fopen( GetAbstractSigFile( m_CpeSignalFileInfo.CPESignalFile, chTmp ), "rb");
    if( NULL == pFile )
    {
        printf("SIGNAL FILE UPLOAD,can't open %s", m_CpeSignalFileInfo.CPESignalFile);
        return;
     }
     //启动定时器保护ftp任务jy081121
    CTaskFileMag *taskSM = CTaskFileMag::GetInstance();   
     taskSM->setFtpUsingFlag(M_TID_PM, TRUE);
    if( ERROR == ftpXfer( IpAddr, 
                          g_tFTPInfo.chUserName, 
                          g_tFTPInfo.chPassWord, 
                          NULL, 
                          "STOR %s", 
                          SIG_UPLOAD_DIR, 
                          m_CpeSignalFileInfo.CPESignalFile, 
                          &ctrlSock, 
                          &dataSock) )
    {
        LOG( LOG_SEVERE, L3PM_ERROR_FTP_UPLOAD, "SIGNAL FILE UPLOAD ERROR." );
        fclose(pFile);
	 taskSM->setFtpUsingFlag(M_TID_PM, FALSE);
        return;
    }

    memset(buf, 0, sizeof(buf));
    while( (nBytes = fread(buf, 1, 512, pFile)) > 0 )
    {
        ::write (dataSock, buf, nBytes );
        memset(buf, 0, sizeof(buf));
    }

    fclose(pFile);
    close (dataSock);

    if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
    {
        status = false;
    }
    if (ftpCommand (ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
    {
        status = false;
    }
    close (ctrlSock);
    LOG( LOG_DEBUG3, L3PM_ERROR_FTP_UPLOAD, "send sig file." );
    taskSM->setFtpUsingFlag(M_TID_PM, FALSE);
}

void CTaskPM::test()
{
	UINT16 us[2];
	us[0] = M_EMS_BTS_BTS_RTMONITOR_REQ;
	us[1] = 10;
	SendMessage( M_EMS_BTS_BTS_RTMONITOR_REQ, this->GetEntityId(), 4, (UINT8*)us );
}
UINT32 CTaskPM::testGetSec()
{
	T_TimeDate stGpsTime = bspGetDateTime();
//	struct tm time_s;
 //   UINT32 secCount;
	//printf( "\nGPS TIME: %04d-%02d-%02d-%02d-%02d-%02d\n",stGpsTime.year, stGpsTime.month+1, stGpsTime.day, stGpsTime.hour, stGpsTime.minute, stGpsTime.second);
	return stGpsTime.minute*60 + stGpsTime.second;
}

/*
*PM任务删除后重启周期性能日志上报
*/
void CTaskPM::reStartPeriodRpt()
{
    T_TimeDate tTimeData;
    UINT32 waitinterval;
	
    if( g_tFTPInfo.uRqtInterval!=0 || g_tFTPInfo.uCollectInterval!=0 )
    {
        DIR* pdir = opendir( SM_CPE_PERF_DIR );
        if( NULL == pdir )
        {
            OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "L3 OAM PM DIR --- NA", 0);
            return;
        }
        else
        {
            closedir( pdir );
        }
        if( m_bPeriodCollect )
        {
            DelPerfFile();
	     DelPerfFile_new();
            if(NULL!=m_pCollectTm)
                m_pCollectTm->Stop();
            if(NULL!=m_pReportTm)
                m_pReportTm->Stop();    
            if(NULL!=m_pWaitColTm)
                m_pWaitColTm->Stop();
            if(NULL!=m_pWaitTM)
                m_pWaitTM->Stop();
        }
        StartWaitTm();
        if(NULL!=m_pWaitTM)
             m_pWaitTM->Stop();
        if(NULL!=m_pCollectTm)
            m_pCollectTm->SetInterval( g_tFTPInfo.uCollectInterval * 1000 * 60 );
        if(NULL!=m_pReportTm)
            m_pReportTm->SetInterval( g_tFTPInfo.uRqtInterval * 1000 * 60 );
        tTimeData = bspGetDateTime();
        //等待的秒数:(1小时-目前已过秒数)%设置间隔秒数jy080923
        waitinterval = (60*60-(tTimeData.minute*60+tTimeData.second))%(g_tFTPInfo.uRqtInterval *60);
        if(NULL!=m_pWaitTM)
        {
            m_pWaitTM->SetInterval(waitinterval*1000);
            m_pWaitTM->Start();//启动等待定时器jy080923      
        }
        
    }
}


int table[] = { 0xF078, 0xE1F1, 0xD36A, 0xC2E3, 0xB65C, 0xA7D5, 0x954E, 0x84C7, 0x7C30, 0x6DB9, 0x5F22, 0x4EAB, 0x3A14, 0x2B9D, 0x1906, 0x088F, 0xE0F9,
				0xF170, 0xC3EB, 0xD262, 0xA6DD, 0xB754, 0x85CF, 0x9446, 0x6CB1, 0x7D38, 0x4FA3, 0x5E2A, 0x2A95, 0x3B1C, 0x0987, 0x180E, 0xD17A, 0xC0F3, 0xF268,
				0xE3E1, 0x975E, 0x86D7, 0xB44C, 0xA5C5, 0x5D32, 0x4CBB, 0x7E20, 0x6FA9, 0x1B16, 0x0A9F, 0x3804, 0x298D, 0xC1FB, 0xD072, 0xE2E9, 0xF360, 0x87DF,
				0x9656, 0xA4CD, 0xB544, 0x4DB3, 0x5C3A, 0x6EA1, 0x7F28, 0x0B97, 0x1A1E, 0x2885, 0x390C, 0xB27C, 0xA3F5, 0x916E, 0x80E7, 0xF458, 0xE5D1, 0xD74A,
				0xC6C3, 0x3E34, 0x2FBD, 0x1D26, 0x0CAF, 0x7810, 0x6999, 0x5B02, 0x4A8B, 0xA2FD, 0xB374, 0x81EF, 0x9066, 0xE4D9, 0xF550, 0xC7CB, 0xD642, 0x2EB5,
				0x3F3C, 0x0DA7, 0x1C2E, 0x6891, 0x7918, 0x4B83, 0x5A0A, 0x937E, 0x82F7, 0xB06C, 0xA1E5, 0xD55A, 0xC4D3, 0xF648, 0xE7C1, 0x1F36, 0x0EBF, 0x3C24,
				0x2DAD, 0x5912, 0x489B, 0x7A00, 0x6B89, 0x83FF, 0x9276, 0xA0ED, 0xB164, 0xC5DB, 0xD452, 0xE6C9, 0xF740, 0x0FB7, 0x1E3E, 0x2CA5, 0x3D2C, 0x4993,
				0x581A, 0x6A81, 0x7B08, 0x7470, 0x65F9, 0x5762, 0x46EB, 0x3254, 0x23DD, 0x1146, 0x00CF, 0xF838, 0xE9B1, 0xDB2A, 0xCAA3, 0xBE1C, 0xAF95, 0x9D0E,
				0x8C87, 0x64F1, 0x7578, 0x47E3, 0x566A, 0x22D5, 0x335C, 0x01C7, 0x104E, 0xE8B9, 0xF930, 0xCBAB, 0xDA22, 0xAE9D, 0xBF14, 0x8D8F, 0x9C06, 0x5572,
				0x44FB, 0x7660, 0x67E9, 0x1356, 0x02DF, 0x3044, 0x21CD, 0xD93A, 0xC8B3, 0xFA28, 0xEBA1, 0x9F1E, 0x8E97, 0xBC0C, 0xAD85, 0x45F3, 0x547A, 0x66E1,
				0x7768, 0x03D7, 0x125E, 0x20C5, 0x314C, 0xC9BB, 0xD832, 0xEAA9, 0xFB20, 0x8F9F, 0x9E16, 0xAC8D, 0xBD04, 0x3674, 0x27FD, 0x1566, 0x04EF, 0x7050,
				0x61D9, 0x5342, 0x42CB, 0xBA3C, 0xABB5, 0x992E, 0x88A7, 0xFC18, 0xED91, 0xDF0A, 0xCE83, 0x26F5, 0x377C, 0x05E7, 0x146E, 0x60D1, 0x7158, 0x43C3,
				0x524A, 0xAABD, 0xBB34, 0x89AF, 0x9826, 0xEC99, 0xFD10, 0xCF8B, 0xDE02, 0x1776, 0x06FF, 0x3464, 0x25ED, 0x5152, 0x40DB, 0x7240, 0x63C9, 0x9B3E,
				0x8AB7, 0xB82C, 0xA9A5, 0xDD1A, 0xCC93, 0xFE08, 0xEF81, 0x07F7, 0x167E, 0x24E5, 0x356C, 0x41D3, 0x505A, 0x62C1, 0x7348, 0x8BBF, 0x9A36, 0xA8AD,
				0xB924, 0xCD9B, 0xDC12, 0xEE89, 0xFF00 };
UINT16 getCRC32(char *buf, int len)
{
    UINT16 crc=0;
	
    for(int i=0; i<len; i++)
    {
        crc = table[(crc ^ buf[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

/*
*新的话务统计功能，生成文件名， 文件名格式BTS.BTSid. YYMMDDHHMM.perf
*
*/
bool CTaskPM :: MakePerfFileInfo_new()
{
    char chTmp[15], chTm[25], chTail[40];
    memset( chTmp, 0, 15 );
    memset( chTm, 0, 25 );
    memset( chTail, 0, 40 );    
	
    time_t tm = m_tRcdHead_new.uEndTime;
    struct tm *time = ::localtime( &tm );
    ::strftime( chTm, 20, "%Y%m%d%H%M", time );
    sprintf( chTmp, "%08x", bspGetBtsID() );

    memset( chTail, 0, 40 );
    memcpy( chTail, chTmp, strlen(chTmp) );

    strcat( chTail, "." );
    strcat( chTail, chTm );
    strcat( chTail, ".perf" );

    memset( (UINT8*)&m_tFileInfo_new.BTSPerfFile, 0, sizeof(m_tFileInfo_new.BTSPerfFile) );
    memcpy( m_tFileInfo_new.BTSPerfFile, "BTS.", 4 );
    strcat( m_tFileInfo_new.BTSPerfFile, chTail );
    
    m_tFileInfo_new.isBTSFileExist = false; 
    return true;
}
/*
*删除内存中保留的性能日志文件
*/
bool CTaskPM :: DelPerfFile_new()
{
    if( chdir(SM_CPE_PERF_DIR) )
        return false;
    if( 0 != strlen(m_tFileInfo_new.BTSRsvPerfFile) && m_tFileInfo_new.isBTSFileExist )
        RemoveFile( m_tFileInfo_new.BTSRsvPerfFile );
    if( 0 != strlen(m_tFileInfo_new.BTSPerfFile) && m_tFileInfo_new.BTSPerfFile )
        RemoveFile( m_tFileInfo_new.BTSPerfFile );
    memset( &m_tFileInfo_new, 0, sizeof(m_tFileInfo_new) );
    return true;
}
/*
*上报性能日志文件，如果有保留文件就上报，然后删除该文件，如果本次上报文件上报不成功，则
*记录为保留文件，上报成功则删除该文件，重新生成文件头
*/
void CTaskPM :: PM_PerfDataFileUploadTimer_new()
{    
    OAM_LOGSTR1(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_PerfDataFileUploadTimer_new --- ", 0);
    DIR* pdir = opendir( SM_CPE_PERF_DIR ); 
    if( NULL == pdir )
    {
        OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_PerfDataFileUploadTimer_new --- OPEN DIR ERROR", 0);
        return;
    }
    else
    {
        closedir( pdir );
    }
    //判断条件：文件名长不为零 且 文件存在
    if( 0 != strlen(m_tFileInfo_new.BTSRsvPerfFile) && m_tFileInfo_new.isBTSFileExist )                        //保留的BTS性能文件
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_PerfDataFileUploadTimer_new --- upload BTSRsvPerfFile", 0);
        PM_UploadTimerDealFile_new( true, m_tFileInfo_new.BTSRsvPerfFile, strlen(m_tFileInfo_new.BTSRsvPerfFile) );
        RemoveFile( m_tFileInfo_new.BTSRsvPerfFile );
        memset( m_tFileInfo_new.BTSRsvPerfFile, 0, 40 );
    }
    if( 0 != strlen(m_tFileInfo_new.BTSPerfFile) && m_tFileInfo_new.isBTSFileExist )    
    {
        OAM_LOGSTR1(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "*********** L3 PM_PerfDataFileUploadTimer_new --- upload BTSPerfFile", 0);
        if( PM_UploadTimerDealFile_new( true, m_tFileInfo_new.BTSPerfFile, strlen(m_tFileInfo_new.BTSPerfFile) ) )    //当前BTS性能文件
        {
            RemoveFile( m_tFileInfo_new.BTSPerfFile );
        }
        else
        {
            OAM_LOGSTR1(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "m_tFileInfo_new.PERFFILE ---> UPLOAD ERROR", 0);
            memcpy( m_tFileInfo_new.BTSRsvPerfFile, m_tFileInfo_new.BTSPerfFile, 40 );  
        }
    }
    
    MakePerfFileInfo_new();
}
/*
*建立ftp连接，上报文件，与老的上报文件夹不同
*/
bool CTaskPM :: SendPerfFile_new( char* chFileName )
{
    if( NULL == chFileName )
        return false;

    int ctrlSock;
    int dataSock;
    int nBytes;
    char buf [512];
    STATUS status;

    struct in_addr Svriaddr;
    Svriaddr.s_addr = g_tFTPInfo.uIPAddr;
    SINT8    IpAddr[16];
    memset( IpAddr, 0, sizeof(IpAddr) );
    inet_ntoa_b( Svriaddr, IpAddr );

    char chTmp[L3_BTS_PM_ABSTRACT_FILENAME_LEN];
    FILE * pFile = fopen( GetAbstractFile( chFileName, chTmp ), "rb");
    if( NULL == pFile )
        return false;
    OAM_LOGSTR2(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "SendPerfFile_new, name:%s,  pwd:%s ---", \
		(int)g_tFTPInfo.chUserName, (int)g_tFTPInfo.chPassWord);
    if( ERROR == ftpXfer( IpAddr, 
                          g_tFTPInfo.chUserName, 
                          g_tFTPInfo.chPassWord, 
                          NULL, 
                          "STOR %s",                           
                          PM_UPLOAD_DIR_NEW,
                          chFileName, 
                          &ctrlSock, 
                          &dataSock) )
    {
        LOG( LOG_SEVERE, L3PM_ERROR_FTP_UPLOAD, "FILE UPLOAD ERROR new." );
        fclose(pFile);
        return false;
    }

    memset(buf, 0, sizeof(buf));
    while( (nBytes = fread(buf, 1, 512, pFile)) > 0 )
    {
        ::write (dataSock, buf, nBytes );
        memset(buf, 0, sizeof(buf));
    }

    fclose(pFile);
    close (dataSock);

    if (ftpReplyGet (ctrlSock, TRUE) != FTP_COMPLETE)
    {
        status = false;
    }
    if (ftpCommand (ctrlSock, "QUIT", 0, 0, 0, 0, 0, 0) != FTP_COMPLETE)
    {
        status = false;
    }
    close (ctrlSock);
    return true;
}
/*
*超时上报文件
*/
bool CTaskPM :: PM_UploadTimerDealFile_new( bool isBTSFile, char* scFileName, UINT8 ucFileNameLen )
{
    OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "PM_UploadTimerDealFile_new ---");
    UINT8 uTmp[50];
    memset( uTmp, 0, 50 );
    * (UINT16*)uTmp = m_TransationID;//|0x8000;
    //启动定时器保护ftp任务jy081121
    CTaskFileMag *taskSM = CTaskFileMag::GetInstance();    
    taskSM->setFtpUsingFlag(M_TID_PM, TRUE);
    if( SendPerfFile_new( scFileName ) )
    {
        OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "PM_UploadTimerDealFile_new ---> UPLOAD SUCC");
        taskSM->setFtpUsingFlag(M_TID_PM, FALSE);
        return true;
    }
    else
    {       
        OAM_LOGSTR(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "PM_UploadTimerDealFile_new---> UPLOAD ERROR");
        taskSM->setFtpUsingFlag(M_TID_PM, FALSE);
        return false;
    }    
}
/*
*生成上报文件，格式:文件头，crc，三层统计数据，crc，二层统计数据，crc
*/
bool CTaskPM :: WriteToFile_new(unsigned char* chData, //写入的数据
                               unsigned int uDataLen)//写入的数据长度

{
    OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "WriteToFile_new ---");
    if( NULL == chData )
        return false;

    FILE *stream;
    int numwritten;
    UINT32 crc;

    DIR* pdir = opendir( SM_CPE_PERF_DIR );
    if ( NULL == pdir )
    {
        return false;
    }
    else
        closedir( pdir );
    char chTmp[L3_BTS_PM_ABSTRACT_FILENAME_LEN];
    stream = fopen( GetAbstractFile(m_tFileInfo_new.BTSPerfFile, chTmp), "ab" );    

    if( stream != NULL )
    {  
        int headlen = sizeof(m_tRcdHead_new);
        m_tRcdHead_new.length = 	12 + uDataLen + sizeof(T_L3PerfData_new);	 
        crc = getCRC32((char*)&m_tRcdHead_new, headlen);
        numwritten = fwrite( (char*)&m_tRcdHead_new, sizeof( char ), headlen, stream );
        //head
        if(numwritten<headlen)
        {
            OAM_LOGSTR(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "WriteToFile_new--->write head error");
            fclose( stream );
            RemoveFile( m_tFileInfo_new.BTSPerfFile );
            return false;
        }
        //head crc
        numwritten = fwrite( (char*)&crc, sizeof( char ), 4, stream );
        if(numwritten<4)
        {
            OAM_LOGSTR(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "WriteToFile_new--->write head crc error");
            fclose( stream );
            RemoveFile( m_tFileInfo_new.BTSPerfFile );
            return false;
        }
        numwritten = fwrite( (char*)&l3PerfData_new, sizeof( char ), sizeof(T_L3PerfData_new), stream );
        //l3
        if(numwritten<sizeof(T_L3PerfData_new))
        {
            OAM_LOGSTR(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "WriteToFile_new--->write head error");
            fclose( stream );
            RemoveFile( m_tFileInfo_new.BTSPerfFile );
            return false;
        }
        //l3 crc
        crc = getCRC32((char*)&l3PerfData_new, sizeof(T_L3PerfData_new));
        numwritten = fwrite( (char*)&crc, sizeof( char ), 4, stream );
        if(numwritten<4)
        {
            OAM_LOGSTR(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "WriteToFile_new--->write head crc error");
            fclose( stream );
            RemoveFile( m_tFileInfo_new.BTSPerfFile );
            return false;
        }	         
        //l2
        crc = getCRC32((char*)chData, uDataLen);
        numwritten = fwrite( chData, sizeof( char ), uDataLen, stream );        
        if(numwritten<uDataLen)
        {
            OAM_LOGSTR(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "WriteToFile_new--->write file error");
            RemoveFile( m_tFileInfo_new.BTSPerfFile );
            fclose( stream );
            return false;
        }
        //l2 crc	
        numwritten = fwrite( (char*)&crc, sizeof( char ), 4, stream );
        fclose( stream );
        if(numwritten<4)
        {
            OAM_LOGSTR(LOG_SEVERE, L3PM_ERROR_WRITE_FILE, "WriteToFile_new--->write crc error");
            RemoveFile( m_tFileInfo_new.BTSPerfFile );
            return false;
        }
        m_tFileInfo_new.isBTSFileExist = true;        
        return true;
    }
    else
    {
        return false;
    }
}
/*
*统计三层数据
*/
void CTaskPM ::GetL3PerfData_new()
{
    OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "GetL3PerfData_new ---");
    T_L3DATAEBGenPerfData   EBGenPerfData;
    GetDataPerfData ( BTS_PERF_TYPE_EB, (UINT8*)&EBGenPerfData);
    l3PerfData_new.reguser = EBGenPerfData.ServingUsers;  
    statEnd((UINT8*)l3PerfData_new.dataflow, (UINT8*)l3PerfData_new.datatime);
    GetDataPerfDataNew(BTS_PERF_TYPE_EB, (UINT32*)&l3PerfData_new.dataToWan);
    l3PerfData_new.dataToWan = l3PerfData_new.dataToWan / (g_tFTPInfo.uCollectInterval*60) * 8 / 1024;//kbps
    l3PerfData_new.dataFromWan = l3PerfData_new.dataFromWan / (g_tFTPInfo.uCollectInterval*60) * 8 / 1024;//kbps
    l3PerfData_new.dataToTDR = l3PerfData_new.dataToTDR / (g_tFTPInfo.uCollectInterval*60) * 8 / 1024;//kbps
    l3PerfData_new.dataFromTDR = l3PerfData_new.dataFromTDR / (g_tFTPInfo.uCollectInterval*60) * 8 /1024;//kbps
    GetVoicePerfDataNew((UINT32*)&l3PerfData_new.M_SAGHeartTimeoutNum);    
    PM_ClearDataNew(); 
}
/*
*采集定时器超时，生成上报文件头，统计三层数据，给二层发送上报数据请求
*/
void CTaskPM :: PM_PerfDataQueryTimer_new()
{
    OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "PM_PerfDataQueryTimer_new ---");
    m_tRcdHead_new.uStartTime = m_tRcdHead_new.uEndTime;
    m_tRcdHead_new.uEndTime = GetSystemTime();  
    memset(&l3PerfData_new, 0, sizeof(l3PerfData_new));
    l3PerfData_new.tag = 1;
    l3PerfData_new.length = sizeof(l3PerfData_new)-4;
    GetL3PerfData_new();
    UINT16 usTmp[4];
    memset( (UINT8*)usTmp, 0, 8 );
    *(usTmp) = PERIOD_TRANSACTIONID;

    *(usTmp+1) = 0;
    *((UINT32*)usTmp+1) = 0x0;
    //SetTimerInfo( false, *(UINT16*)usTmp, L2_PERIOD_TIMER, true );
    SendMessage( M_L3TOL2_PERFDATA_REQ_new, M_TID_L2OAM, 8, (UINT8*)usTmp );    //send meg to L2    
}
/*
*二层上报统计数据，写入文件
*/
void CTaskPM :: PM_L2PerfDataRsp_new( CMessage& rMsg )
{
    OAM_LOGSTR(LOG_DEBUG3, L3PM_ERROR_WRITE_FILE, "PM_L2PerfDataRsp_new ---");
    //计算crc   
    WriteToFile_new( (UINT8*)rMsg.GetDataPtr(), rMsg.GetDataLength());
}

/*
*性能日志清除统计数据
*/
void CTaskPM :: PM_ClearDataNew()
{
    //开始计费流量和时长统计初始化
    statBegin();
    ClearDataPerfDataNew(BTS_PERF_TYPE_EB);
    ClearVoicePerfDataNew();    
}

