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
 *   ----------  ----------  ----------------------------------------------------
 *   10/11/2006   肖卫方       修改
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/
#pragma warning (disable : 4786)
#ifdef __WIN32_SIM__
#include <windows.h>
#else
#ifndef __INCtaskLibh
#include <vxWorks.h>
#endif
#include <taskLib.h>
#include "mcWill_bts.h"
#include <fioLib.h>
#include <stdarg.h>
#endif

#include <stdio.h>
#include <string.h>

#include "L3OamAlm.h"
#include "MsgQueue.h"
#include "ErrorCodeDef.h"
#include "Log.h"
//#include "L3OamTest.h"
#include "L3EmsMessageId.h"
#include "L3OamAlarmHandleNotify.h"
#include "L3L2MessageId.h"
#include "L3OamCfgCommon.h"
#include "L3L3L2BtsConfigSynPower.h"


CTaskAlm* CTaskAlm:: m_Instance = NULL;
extern T_NvRamData *NvRamDataAddr;
typedef map<UINT16, UINT32>::value_type ValType;

static bool s_L2CommFail = false;
extern void SaveRebootAlarmInfo(UINT16 almCode, UINT16 almEntity, UINT16 almInst);
extern "C"
{
int rebootMCP(int);
int rebootAUX();
int rebootFEP(int);
}
#ifdef WBBU_CODE
extern unsigned char rru_report_flag;
extern "C"  void WrruRFC(unsigned char antennamask,unsigned char flag,unsigned char flag1);
extern "C"  void ResetWrru();

extern  void sendCdrForReboot();
extern "C"  void sendAnntenaMsk(unsigned short  antennamask,unsigned char flag,unsigned char flag1);
#endif
/*************************************************
 *初始化告警信息表
 *只列出需要进行告警异常处理的告警ID
 */
 #define M_ALM_ID_DEF   (0xFFFF)
CTaskAlm::T_AlmBaseTab CTaskAlm::m_AlmBaseTab[] = {
////L2通信
    {ALM_ID_L2PPC_COMMFAIL,                     M_ALM_MAX_ENDURE_SEC_L2PPC},
////AUX
    {ALM_ID_AUX_TO_L2_CONTROL_FAIL,             M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_TO_L2_RANGING_BUF_NOT_EMPTY,    M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_L2_TO_AUX_CONTROL_FAIL,         M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_L2_TO_AUX_WEIGHT_NOT_EMPTY,     M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_L2_TO_AUX_CONFIG_NOT_EMPTY,     M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_L2_FROM_AUX_RANGING_NOT_FULL,   M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_L2_FROM_AUX_RANGING_CHKSUM,     M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_AUX_FROM_L2_BUF_NOT_FULL,       M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_AUX_FROM_L2_CHKSUM,             M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_AUX_TO_FEP_BUF_NOT_EMPTY,       M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_FEP_FROM_AUX_CHKSUM,            M_ALM_MAX_ENDURE_SEC_AUX},
    {ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM,    M_ALM_MAX_ENDURE_SEC_AUX},
////MCP
    {ALM_ID_MCP_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY, M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_L2_TO_MCP_UPLINKPROF_NOT_EMPTY, M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_L2_TO_MCP_CONFIG_NOT_EMPTY,     M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_NOT_FULL,M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_CHKSUM,  M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_MCP_TO_L2_UPLINK_NOT_EMPTY,     M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_NOT_FULL,  M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_MCP_FROM_L2_UPPROF_NOT_FULL,    M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_MCP_FROM_L2_CONFIG_NOT_FULL,    M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_CHKSUM,    M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_MCP_FROM_L2_UPPROF_CHKSUM,      M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_MCP_FROM_L2_CONFIG_CHKSUM,      M_ALM_MAX_ENDURE_SEC_MCP},
    {ALM_ID_MCP_MCP_FEP_ALARM,                  M_ALM_MAX_ENDURE_SEC_MCP},
////ENV
    {ALM_ID_ENV_SYNC_SHUTDOWN,          M_ALM_MAX_ENDURE_SEC_ENV},
    {ALM_ID_ENV_FAN_STOP,               M_ALM_MAX_ENDURE_SEC_ENV},
///TCXO
    {ALM_ID_PLL_TCXO_FREQOFF,           M_ALM_MAX_ENDURE_SEC_TCXO},
////RF
    {ALM_ID_RF_BOARD_VOLTAGE_MINOR,     M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_BOARD_VOLTAGE_SERIOUS,   M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_BOARD_CURRENT_MINOR,     M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_BOARD_CURRENT_SERIOUS,   M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_TTA_VOLTAGE_MINOR,       M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_TTA_VOLTAGE_SERIOUS,     M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_TTA_CURRENT_MINOR,       M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_TTA_CURRENT_SERIOUS,     M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_TX_POWER_MINOR,          M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_TX_POWER_SERIOUS,        M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_RF_DISABLED,             M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_BOARD_SSP_CHKSUM_ERROR,  M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_BOARD_RF_CHKSUM_ERROR,   M_ALM_MAX_ENDURE_SEC_RF},
    {ALM_ID_RF_BOARD_RF_NORESPONSE,     M_ALM_MAX_ENDURE_SEC_RF},
////GPS
    {ALM_ID_GPS_SIGNAL,                 M_ALM_MAX_ENDURE_SEC_GPS},
    {ALM_ID_GPS_LOC_CLOCK,              M_ALM_MAX_ENDURE_SEC_GPS},
    {ALM_ID_GPS_LOST,                   M_ALM_MAX_ENDURE_SEC_GPS},
////PLL
    {ALM_ID_PLL_PLLLOSELOCK_SERIOUS,    M_ALM_MAX_ENDURE_SEC_PLL},
#ifdef WBBU_CODE
    { ALM_ID_gps_warning,     M_ALM_MAX_ENDURE_SEC_GPS*2     },
   {ALM_ID_tdd_fep_rx_warning ,   M_ALM_MAX_ENDURE_SEC_GPS*2         },
  {ALM_ID_frame_sync_warning ,  M_ALM_MAX_ENDURE_SEC_GPS *2       },
  {ALM_ID_sfp_los_warning     ,        M_ALM_MAX_ENDURE_SEC_GPS*2      },
{Alarm_ID_WRRU_recv_nopoll,M_ALM_MAX_ENDURE_SEC_TCXO},
//{Alarm_ID_WRRU_Reset,M_ALM_MAX_ENDURE_SEC_GPS*2},
#endif
////END.没有找到的告警
    {M_ALM_ID_DEF,              M_ALM_MAX_ENDURE_SEC_DEF}
};

CTaskAlm::CTaskAlm()
{
    strcpy(m_szName, "tAlm");
    m_uPriority   = M_TP_L3FM;
    m_uOptions    = 0;
    m_uStackSize  = SIZE_KBYTE * 50;
    m_iMsgQMax    = 1000; 
    m_iMsgQOption = 0;

    m_AlmSeqID = 0;

    m_bHandleException             = false;
    m_pAlarmKeepClearTimer         = NULL;
    m_pAlarmKeepNotClearTimer      = NULL;
    m_ucRFDisabledMask             = 0xFF;
}

bool CTaskAlm::Initialize()
{
#define SYSFLG_HANDLE_ALARM (0x10000)

    m_pAlarmKeepClearTimer = InitTimer(E_TIMER_ALARM_KEEP_CLEAR);
    if (NULL == m_pAlarmKeepClearTimer)
        {
        return false;
        }
    m_pAlarmKeepNotClearTimer = InitTimer(E_TIMER_ALARM_KEEP_NOT_CLEAR);
    if (NULL == m_pAlarmKeepNotClearTimer)
        {
        return false;
        }

    if (false == CBizTask :: Initialize())
        {
        return false;
        }
   
    BOOT_PARAMS bootParams;
    char *pS;
#ifdef WBBU_CODE
       m_bHandleException = true;
#else
    pS = bootStringToStruct ((char*)BOOT_LINE_ADRS_L3, &bootParams);
    if (*pS == EOS)
        {
        m_bHandleException = true;
        }
    else
        {
        m_bHandleException = (bootParams.flags  & SYSFLG_HANDLE_ALARM)?false:true;
        }
#endif
    UINT16 usIdx    = 0;
    UINT16 usAlmID  = 0;
    while(true)
        {
        usAlmID = m_AlmBaseTab[usIdx].usAlmID;
        if(M_ALM_ID_DEF == usAlmID)
            {
            break;
            }
        m_mapAlmID_KeepTime.insert(ValType(usAlmID, m_AlmBaseTab[usIdx].slSecondsKeepNotClear));
        ++usIdx;
        }

    return true;
}

CTaskAlm* CTaskAlm::GetInstance()
{
    if ( NULL == m_Instance )
        {
        m_Instance = new CTaskAlm;
        }

    return m_Instance;
}

const UINT16 CTaskAlm::getCurrentRFMask()
{
    //当前的天线淹码 = (配置值) & (因告警关闭的)
    UINT16 mask = NvRamDataAddr->L1GenCfgEle.AntennaMask;
    if (true == m_bHandleException)
        {
        mask &= m_RFAlmState.getRFMask();   //因为告警关闭的
        //mask &= m_ucRFDisabledMask; //因为L1关闭的
        }
    return mask;
}


const UINT16 CTaskAlm::getDisplayRFMask()
{
    UINT16 mask = getCurrentRFMask();
    if (true == m_bHandleException)
        {
        mask &= m_ucRFDisabledMask;   //因为告警关闭的
        }
    return mask;
}


void CTaskAlm::MainLoop()
{
////
    m_pAlarmKeepClearTimer->Start();
    m_pAlarmKeepNotClearTimer->Start();
    CBizTask::MainLoop();
}

bool CTaskAlm::ProcessMessage(CMessage &msg)
{
    UINT16 MsgId = msg.GetMessageId();
#ifdef WBBU_CODE
    unsigned char *ptr = (unsigned char *)msg.GetDataPtr();
    unsigned char  j= 0;
    UINT16 usRFMask;
    unsigned char antennamask =0;
    static UINT16 usRFMask_last =0xff00;
    UINT8 *puc0;
#endif
    switch(MsgId)
    {
        case M_BTS_EMS_ALM_NOTIFY: //收到告警检测模块发送的告警上报消息后对消息进行处理
        {
            CAlarmNotifyOam msgNotify(msg);
            AlarmAnalysis(msgNotify);
            break;
        }

        case M_OAM_ALARM_SEND_TO_EMS_TIMER:
        {
            AlarmTimeOut(msg);
            break;
        }

        case M_EMS_BTS_GET_ACTIVE_ALM_NOTIFY: //收到ems查询当前告警消息后对消息进行处理
        {
            getActiveAlmNotify();
            break;
        }
        
        case M_OAM_DELETE_ALM_RECORD_NOTIFY:
        {
            CDeleteAlarmNotify msgNotify(msg);
            deleteAlarmNotify(msgNotify);
            break;
        }
#ifndef WBBU_CODE
	    case M_GPS_ALM_CHG_RF_MASK_REQ:            
            if (BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
            {                
                m_RFAlmState.setAllRFAlarm(E_ALM_GPS_LOST);                
            }
            //Check if RF enable or disable.
            if ((true == m_bHandleException) && (true == m_RFAlmState.isRFMaskModified()))
            {
                ////////UINT16 usRFMask = NvRamDataAddr->L1GenCfgEle.AntennaMask & m_RFAlmState.getRFMask();
                UINT16 usRFMask = getCurrentRFMask();
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm Modify RF Mask by M_GPS_ALM_CHG_RF_MASK_REQ [0x%.4X].", usRFMask);
                if (true == sendAlarmHandleMsg(M_L3_L2_CFG_ANTENNA_NOTIFY, M_TID_L2MAIN, (SINT8*)&usRFMask, sizeof(usRFMask)))
                {
                    m_RFAlmState.RFMaskRefresh();
                }
            }
            break;
#else
	 case M_GPS_ALM_CHG_RF_MASK_REQ:
        {
            puc0 = (UINT8*)msg.GetDataPtr();
            if (BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
            {
                if(*puc0==0)// 设置标志位
                    m_RFAlmState.setAllRFAlarm(E_ALM_GPS_LOST);
                else//清楚标志位
                    m_RFAlmState.clearAllRFAlarm(E_ALM_GPS_LOST);
            }
            //Check if RF enable or disable.
            if (1/*(true == m_bHandleException) && (true == m_RFAlmState.isRFMaskModified())*/)
            {
                ////////UINT16 usRFMask = NvRamDataAddr->L1GenCfgEle.AntennaMask & m_RFAlmState.getRFMask();
                usRFMask = getCurrentRFMask();
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm Modify RF Mask[0x%.4X].", usRFMask);
                if(MsgId!=M_WRRU_ALM_CHG_RF_MASK_REQ)//如果是SYN失所的情况，不关闭RF,由wwrruFPGA进行相关操作
                {
                    sendAnntenaMsk(usRFMask,1,0);
                }
                //  if (true == sendAlarmHandleMsg(M_L3_L2_CFG_ANTENNA_NOTIFY, M_TID_L2MAIN, (SINT8*)&usRFMask, sizeof(usRFMask)))
                {
                    m_RFAlmState.RFMaskRefresh();
                }
            }
            break;
	 }
	 case M_FPGA_ALM_CHG_RF_MASK_REQ:
	       antennamask = ptr[1];
		 if(antennamask==0)
		{
		 	if (BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
	              {
	                  m_RFAlmState.setAllRFAlarm(E_ALM_GPS_LOST);
	              }
			//Check if RF enable or disable.
			OAM_LOGSTR2(LOG_DEBUG1, L3FM_ERROR_PRINT_SW_INFO, "[tAlm1] wwh Alarm Modify RF Mask[0x%.4X,%x].", m_bHandleException,m_RFAlmState.isRFMaskModified());
	             if (1/*(true == m_bHandleException) && (true == m_RFAlmState.isRFMaskModified())*/)
	                 {
	         ////////UINT16 usRFMask = NvRamDataAddr->L1GenCfgEle.AntennaMask & m_RFAlmState.getRFMask();
	                  usRFMask = getCurrentRFMask();
	                 OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm Modify RF Mask[0x%.4X].", usRFMask);
			  if(MsgId!=M_WRRU_ALM_CHG_RF_MASK_REQ)//如果是SYN失所的情况，不关闭RF,由wwrruFPGA进行相关操作
			  	{
	                 		sendAnntenaMsk(usRFMask,1,1);
			  	}
	               //  if (true == sendAlarmHandleMsg(M_L3_L2_CFG_ANTENNA_NOTIFY, M_TID_L2MAIN, (SINT8*)&usRFMask, sizeof(usRFMask)))
	                     {
	                     m_RFAlmState.RFMaskRefresh();
	                     }
	                 }
    		}
		 else
		 	{
		 	#if 0
		 	     if (BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
	              {
	                  m_RFAlmState.clearAllRFAlarm(E_ALM_GPS_LOST);
	              }
		 	 #endif
		 	 	for(j =0 ;j < 8 ;j++)
				{
				        if((antennamask&(1<<j))!=0)
				       {
		     				m_RFAlmState.clearRFAlarm(j, E_ALM_GPS_LOST);
				       }
				       else
				       {
				              m_RFAlmState.setRFAlarm(j, E_ALM_GPS_LOST);
				        }
				}
			//Check if RF enable or disable.
	             if (1/*(true == m_bHandleException) && (true == m_RFAlmState.isRFMaskModified())*/)
	                 {
	                 // usRFMask = NvRamDataAddr->L1GenCfgEle.AntennaMask & m_RFAlmState.getRFMask();
	                  usRFMask = getCurrentRFMask();
	                 OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm Modify RF Mask[0x%.4X].", usRFMask);
			 // if(MsgId!=M_WRRU_ALM_CHG_RF_MASK_REQ)//如果是SYN失所的情况，不关闭RF,由wwrruFPGA进行相关操作
			  //	{
	                 //		sendAnntenaMsk(usRFMask);
			  	//}
	               //  if (true == sendAlarmHandleMsg(M_L3_L2_CFG_ANTENNA_NOTIFY, M_TID_L2MAIN, (SINT8*)&usRFMask, sizeof(usRFMask)))
	                     {
	                     m_RFAlmState.RFMaskRefresh();
	                     }
                     }
                 }
	 	break;
	//case M_WRRU_ALM_CHG_RF_MASK_REQ:
	//#ifdef   RRU_ALM_CLOSE_RF
	
	case M_WRRU_ALM_CHG_RF_MASK_REQ:
	
		for(j =0 ;j < 8 ;j++)
		{
		        if((ptr[0]&(1<<j))!=0)
		       {
     				m_RFAlmState.setRFAlarm(j, E_ALM_PLL_LOSE_LOCK);
		       }
		       else
		       {
		              m_RFAlmState.clearRFAlarm(j, E_ALM_PLL_LOSE_LOCK);
		        }
		}
			
		 if (1/*(true == m_bHandleException) && (true == m_RFAlmState.isRFMaskModified())*/)
                 {
         ////////UINT16 usRFMask = NvRamDataAddr->L1GenCfgEle.AntennaMask & m_RFAlmState.getRFMask();
                  usRFMask = getCurrentRFMask();
                  OAM_LOGSTR3(LOG_DEBUG1, L3FM_ERROR_PRINT_SW_INFO, "[tAlm2] Alarm Modify RF Mask[0x%.4X，0x%.4X, 0x%.4X].", usRFMask,usRFMask_last,ptr[0]);
                   if(usRFMask==usRFMask_last)
                   	{
                         //  break;
                   	}
                   else
                   	{
                   	
                  usRFMask_last = usRFMask;
                   	}
              //   OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm Modify RF Mask[0x%.4X].", usRFMask);
		  if(ptr[1]==0)
		  	{
                  		sendAnntenaMsk(usRFMask,1,2);
		  	}
                // if (true == sendAlarmHandleMsg(M_L3_L2_CFG_ANTENNA_NOTIFY, M_TID_L2MAIN, (SINT8*)&usRFMask, sizeof(usRFMask)))
                     {
                      m_RFAlmState.RFMaskRefresh();
                     }
             //       WrruRFC((unsigned char )usRFMask,1);
		//  OAM_LOGSTR2(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] M_WRRU_ALM_CHG_RF_MASK_REQ Modify RF Mask[idex:%d,][0x%.4X].",ptr[0], usRFMask);
                 }		 
		  else
		  {
		  #if 0
		  	       usRFMask = getCurrentRFMask();
                 	OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm1] Alarm Modify RF Mask[0x%.4X].", usRFMask);
                 	 sendAnntenaMsk(usRFMask,1,3);
                // if (true == sendAlarmHandleMsg(M_L3_L2_CFG_ANTENNA_NOTIFY, M_TID_L2MAIN, (SINT8*)&usRFMask, sizeof(usRFMask)))
                     {
                     m_RFAlmState.RFMaskRefresh();
                     }
               //     WrruRFC((unsigned char )usRFMask,1);
		  OAM_LOGSTR2(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm1] M_WRRU_ALM_CHG_RF_MASK_REQ Modify RF Mask[idex:%d,][0x%.4X].",ptr[0], usRFMask);
               #endif
		  }
	
	 	break;


#endif
        case M_OAM_CFG_ALM_CLR_RF_ALM_REQ:
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_REV_ERR_MSG, "[tAlm] receive M_OAM_CFG_ALM_CLR_RF_ALM_REQ");
            m_RFAlmState.clearAllRFAlarm(E_ALM_GPS_LOST);
            break;
        default:
        {
            OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_REV_ERR_MSG, "[tAlm] receive error msg[0x%04x]", MsgId);
            break;
        }
    }
  
    return true;
}


TID CTaskAlm::GetEntityId() const
{
   return M_TID_FM;
}

#if 0
//
//接受到的告警，首先缓存到链表中，同时把立刻恢复的告警从
//缓存链表中删除.
//告警真正的处理是一个1秒的定时器完成的
//
bool CTaskAlm::BufferThisAlarm(CAlarmNotifyOam &Alarm) 
{
    bool found = false;          //首先判断是否有告警记录
    list<T_AlarmHandle*>::iterator it;
    T_AlarmHandle *pCurAlmHandler = NULL;

    UINT16 Code  = Alarm.GetAlarmCode();
    UINT16 Type  = Alarm.GetEntityType();
    UINT16 Index = Alarm.GetEntityIndex();
    for (it = m_listBufferedAlarm.begin(); it != m_listBufferedAlarm.end(); ++it)
        {
        pCurAlmHandler   = (*it);
        UINT16 TempCode  = pCurAlmHandler->pAlarmNotify->GetAlarmCode();
        UINT16 TempType  = pCurAlmHandler->pAlarmNotify->GetEntityType();
        UINT16 TempIndex = pCurAlmHandler->pAlarmNotify->GetEntityIndex();
        if((TempCode == Code) && (TempType == Type) && (TempIndex == Index))
            {
            found = true;
            break;
            }
        }

    UINT8 AlmFlg = Alarm.GetFlag();

    if(false == found)
        {
        CAlarmNotifyOam *pCloneAlmNotify = (CAlarmNotifyOam *)Alarm.Clone();
        if (NULL == pCloneAlmNotify)
            {
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Encounter system exception. Message.clone() failed.");
            return false;
            }
        T_AlarmHandle *pAlarmHandler = new T_AlarmHandle(pCloneAlmNotify);
        if (NULL == pAlarmHandler)
            {
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Encounter system exception. new(...) failed.");
            pCloneAlmNotify->DeleteMessage();
            delete pCloneAlmNotify; //why?
            return false;
            }

        pAlarmHandler->pAlarmNotify->SetFlag(AlmFlg);

        //将新告警缓冲起来
        m_listBufferedAlarm.push_back(pAlarmHandler);
        }
    else        
        {
        if (ALM_FLAG_SET == AlmFlg)    
            {
            //有些重复告警，同样的alarm code但是告警内容有区别，以最后一次为准
            pCurAlmHandler->pAlarmNotify->SetFlag(AlmFlg);
            pCurAlmHandler->pAlarmNotify->SetAlarmInfo(Alarm.GetAlarmInfo());
            }
        else
            {
            if (ALM_FLAG_SET == pCurAlmHandler->pAlarmNotify->GetFlag())
                {
                m_listBufferedAlarm.erase(it);      //将当前告警清除
                delete pCurAlmHandler;
                }
            else
                {
                ////....保留
                }
            }
        }

    return true;
}
#endif


/*
 *在Almlist寻找inAlarm,如果找到，返回true和对应的迭代子it
 */
bool CTaskAlm::searchInAlarmList(list<T_AlarmHandle*> &Almlist, CAlarmNotifyOam &inAlarm, list<T_AlarmHandle*>::iterator &it)
{
//    if ((NULL == ppAlmHandler) || (NULL == *ppAlmHandler))
//        return false;

    bool found = false;
    T_AlarmHandle *pCurAlmHandler = NULL;

    UINT16 code  = inAlarm.GetAlarmCode();
    UINT16 type  = inAlarm.GetEntityType();
    UINT16 index = inAlarm.GetEntityIndex();
    for (it = Almlist.begin(); it != Almlist.end(); ++it)
        {
        pCurAlmHandler   = (*it);
        UINT16 TempCode  = pCurAlmHandler->pAlarmNotify->GetAlarmCode();
        UINT16 TempType  = pCurAlmHandler->pAlarmNotify->GetEntityType();
        UINT16 TempIndex = pCurAlmHandler->pAlarmNotify->GetEntityIndex();
        if ((TempCode == code) && (TempType == type) && (TempIndex == index))
            {
            found = true;
            break;
            }
        }
//    *ppAlmHandler = pCurAlmHandler;

    return found;
}


bool CTaskAlm::AlarmAnalysis(CAlarmNotifyOam &inAlarm) 
{
    bool bFindInCurr = false;          //首先判断是否有告警记录
    list<T_AlarmHandle*>::iterator curr_it;

    //在当前告警列表中搜是否存在这个告警
    bFindInCurr = searchInAlarmList(m_CurAlmList, inAlarm, curr_it);
    UINT8 AlmFlg = inAlarm.GetFlag();
    if ((false == bFindInCurr)&&(ALM_FLAG_SET == AlmFlg))
        {
        bool bFindInCleared = false;
        list<T_AlarmHandle*>::iterator clear_it;

        //在当前告警列表中搜是否存在这个告警
        bFindInCleared = searchInAlarmList(m_listClearedAlarm, inAlarm, clear_it);
        if (false == bFindInCleared)
            {
            //重新申请控制块
            //传入的控制块在函数外释放
            CAlarmNotifyOam *pCloneAlmNotify = (CAlarmNotifyOam *)inAlarm.Clone();
            if (NULL == pCloneAlmNotify)
                {
                OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Encounter system exception. Message.clone() failed.");
                return false;
                }
            T_AlarmHandle *pAlarmHandler = new T_AlarmHandle(pCloneAlmNotify);
            if (NULL == pAlarmHandler)
                {
                OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Encounter system exception. new(...) failed.");
                pCloneAlmNotify->DeleteMessage();
                delete pCloneAlmNotify;
                return false;
                }

            pAlarmHandler->pAlarmNotify->SetFlag(AlmFlg);
            pAlarmHandler->slKeepAlarmSeconds = getAlarmTimeByID(inAlarm.GetAlarmCode());
            inAlarm.SetSequenceNum(++m_AlmSeqID);
            m_CurAlmList.push_back(pAlarmHandler);        //将新告警添加到告警列表
            sendActiveAlmToEMS(inAlarm);                  //向EMS上报告警产生
            }
        else
            {
            T_AlarmHandle *pClearedAlarmHandler = *clear_it;
            //从cleared alarm list删除
            m_listClearedAlarm.erase(clear_it);
            //插入回curr alarm list.
            pClearedAlarmHandler->pAlarmNotify->SetFlag(AlmFlg);
            pClearedAlarmHandler->slKeepAlarmSeconds = getAlarmTimeByID(inAlarm.GetAlarmCode());
            m_CurAlmList.push_back(pClearedAlarmHandler);        //将新告警添加到告警列表
            }
        return true;
        }

    if((false == bFindInCurr)&&(ALM_FLAG_CLEAR == AlmFlg))
        {
        return true;                 //忽略此种消息         
        }

    if((true == bFindInCurr)&&(ALM_FLAG_SET == AlmFlg))
        {
        T_AlarmHandle *pCurrAlarmHandler = *curr_it;
        pCurrAlarmHandler->pAlarmNotify->SetAlarmInfo(inAlarm.GetAlarmInfo());
        inAlarm.SetSequenceNum(++m_AlmSeqID);
        sendActiveAlmToEMS(inAlarm);                //向EMS上报告警产生
        if (true == pCurrAlarmHandler->bPostProcessFlag)
            {
            //告警处理完后,如果仍然出现告警的话,需要进一步处理
            pCurrAlarmHandler->slKeepAlarmSeconds = getAlarmTimeByID(inAlarm.GetAlarmCode());
            pCurrAlarmHandler->bPostProcessFlag = false;
            }
        return true;
        }

    if((true == bFindInCurr)&&(ALM_FLAG_CLEAR == AlmFlg))
        {
        //从当前告警列表转移到已清除告警队列
        //sendActiveAlmToEMS(inAlarm);      //向EMS上报告警清除
        T_AlarmHandle *pCurrAlarmHandler = *curr_it;
        m_CurAlmList.erase(curr_it);      //将当前告警清除
        clearAlarmHandler(inAlarm);
        //delete pCurAlmHandler;
        pCurrAlarmHandler->pAlarmNotify->SetFlag(AlmFlg);
        pCurrAlarmHandler->slKeepAlarmSeconds = M_CLEAR_ALM_KEEP_TIME;
        m_listClearedAlarm.push_back(pCurrAlarmHandler);        //将新告警添加到告警列表
        return true;
        }

    return true;
}

extern "C" void rebootL2();//lijinan 2010304
//
//告警在一定时间内如果还没有恢复，进行异常处理
//
extern  void sendCdrForReboot();/*lijinan 20100916 for bill*/
bool CTaskAlm::setAlarmHandler(T_AlarmHandle *pHandler)
{
    if (NULL == pHandler)
        return false;

    SINT8  DataBuff[2];
    CAlarmNotifyOam Alarm = *(pHandler->pAlarmNotify);
    UINT16 usAlarmID = Alarm.GetAlarmCode();
    switch(usAlarmID)
    {
        /////////////////////////////////////////
        //连续5秒告警则复位该CPU；连续5秒无告警产生则清除告警；
        case ALM_ID_L2PPC_COMMFAIL:
        {
            if (true == m_bHandleException)
                {
                pHandler->bPostProcessFlag = true;//需要进一步处理，L2启动后如果仍然告警,则reboot BTS.
                if (false == s_L2CommFail)
                    {
                    #ifndef WBBU_CODE
                    DataBuff[0] = BTS_CPU_TYPE_L2PPC;   // cpe type
                    DataBuff[1] = 0;                    // cpe index
                    OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[L2PPC Comm] Reset L2");
                    sendAlarmHandleMsg(M_OAM_CPU_RESET_NOTIFY, M_TID_SYS, DataBuff, 2);
                    #else
                    OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[L2PPC Comm] is not cleared after L2 reset. Now reset whole BTS");
#ifndef __WIN32_SIM__
                    SaveRebootAlarmInfo(usAlarmID,BTS_CPU_TYPE_L2PPC,0);
                    bspSetBtsResetReason(RESET_REASON_SW_ALARM);
		      sendCdrForReboot();
                    taskDelay(1000);
                    rebootBTS(BOOT_CLEAR);
#endif
                    #endif
                    s_L2CommFail = true;
                    }
                else
                    {
                    OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[L2PPC Comm] is not cleared after L2 reset. Now reset whole BTS");
#ifndef __WIN32_SIM__
                    SaveRebootAlarmInfo(usAlarmID,BTS_CPU_TYPE_L2PPC,0);
                    bspSetBtsResetReason(RESET_REASON_SW_ALARM);
		      sendCdrForReboot();
                    taskDelay(1000);
                    rebootBTS(BOOT_CLEAR);
#endif
                    }
                }
            break;
        }

        case ALM_ID_AUX_AUX_TO_FEP_BUF_NOT_EMPTY:   //reboot AUX and FEP?
        case ALM_ID_AUX_FEP_FROM_AUX_CHKSUM:        //reboot AUX and FEP?
        case ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM://reboot AUX and FEP?
        {
            if(true == m_bHandleException)
                {
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[0x%X] reboot FEPs", usAlarmID);
                rebootFEP(2);
                }
            //break;    //need to reboot AUX.
        }
        case ALM_ID_AUX_TO_L2_CONTROL_FAIL:
        case ALM_ID_AUX_TO_L2_RANGING_BUF_NOT_EMPTY:
        case ALM_ID_AUX_L2_TO_AUX_CONTROL_FAIL:
        case ALM_ID_AUX_L2_TO_AUX_WEIGHT_NOT_EMPTY:
        case ALM_ID_AUX_L2_TO_AUX_CONFIG_NOT_EMPTY:
        case ALM_ID_AUX_L2_FROM_AUX_RANGING_NOT_FULL:
        case ALM_ID_AUX_L2_FROM_AUX_RANGING_CHKSUM:
        case ALM_ID_AUX_AUX_FROM_L2_BUF_NOT_FULL:
        case ALM_ID_AUX_AUX_FROM_L2_CHKSUM:
        {
            //DataBuff[0] = BTS_CPU_TYPE_AUX;       // cpe type
            //DataBuff[1] = 0;                      // cpe index            
            if(true == m_bHandleException)
                {
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] AUX Alarm[0x%X], reboot AUX", usAlarmID);
                //sendAlarmHandleMsg(M_OAM_CPU_RESET_NOTIFY, M_TID_SYS, DataBuff, 2);
                rebootAUX();
                }
            break;
        }
        //case ALM_CODE_MCP_COMMFAIL:
        case ALM_ID_MCP_MCP_FEP_ALARM:  //reboot FEP? and MCP ?
        {
            if(true == m_bHandleException)
                {
                OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[0x%X] reboot FEPs", usAlarmID);
                rebootFEP(2);
                }
            //break;    //need to reboot MCP.
        }
        case ALM_ID_MCP_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY:
        case ALM_ID_MCP_L2_TO_MCP_UPLINKPROF_NOT_EMPTY:
        case ALM_ID_MCP_L2_TO_MCP_CONFIG_NOT_EMPTY:
        case ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_NOT_FULL:
        //case ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_CHKSUM: //lijinan 20100304
        case ALM_ID_MCP_MCP_TO_L2_UPLINK_NOT_EMPTY:
        case ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_NOT_FULL:
        case ALM_ID_MCP_MCP_FROM_L2_UPPROF_NOT_FULL:
        case ALM_ID_MCP_MCP_FROM_L2_CONFIG_NOT_FULL:
        case ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_CHKSUM:
        case ALM_ID_MCP_MCP_FROM_L2_UPPROF_CHKSUM:
        case ALM_ID_MCP_MCP_FROM_L2_CONFIG_CHKSUM:
        {
            //DataBuff[0] = BTS_CPU_TYPE_MCP;       // cpe type
            DataBuff[1] = (UINT8)Alarm.GetEntityIndex();
            if(true == m_bHandleException)
                {
                OAM_LOGSTR2(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] MCP Alarm[0x%X], Reset MCP[%d]", usAlarmID, DataBuff[1]);
                //sendAlarmHandleMsg(M_OAM_CPU_RESET_NOTIFY, M_TID_SYS, DataBuff, 2);
                rebootMCP(DataBuff[1]);
                }
            break;
        }
	 case ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_CHKSUM://lijinan 2010304
	{
		
            //DataBuff[0] = BTS_CPU_TYPE_MCP;       // cpe type
            DataBuff[1] = (UINT8)Alarm.GetEntityIndex();
            if(true == m_bHandleException)
                {
                OAM_LOGSTR2(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] MCP[%d] Alarm[0x%X],uplinkdata checksum error Reset L2", DataBuff[1],usAlarmID);
                //sendAlarmHandleMsg(M_OAM_CPU_RESET_NOTIFY, M_TID_SYS, DataBuff, 2);
                //rebootL2();//by liwei for test 20100511
                }
            break;
        }
        case ALM_ID_ENV_SYNC_SHUTDOWN:
        {
            //shutdown SYNC
            if(true == m_bHandleException)
                {
                OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] SYNC temperature alarm, SYNC power off.");
                configSYNCPower(M_SYNC_POWER_OFF);
                }
            break;
        }
        case ALM_ID_ENV_FAN_STOP:
            //关闭BTS.
            break;

        case ALM_ID_PLL_TCXO_FREQOFF:       //disable所有天线, MASK为2字节
        {
            m_RFAlmState.setAllRFAlarm(E_ALM_TCXO_FREQOFF);
            if(true == m_bHandleException)
                {
                DataBuff[0] = BTS_CPU_TYPE_FEP;
                DataBuff[1] = BTS_CPU_INDEX_FEP1;
                OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[TCXO offset], reset FEP1");
                sendAlarmHandleMsg(M_OAM_CPU_RESET_NOTIFY, M_TID_SYS, DataBuff, 2);
                }

            break;
        } 

        //case ALM_CODE_ANTENNA:                    //disable故障天线
        case ALM_ID_RF_BOARD_VOLTAGE_MINOR:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_VOLTAGE_MINOR);
            break;
        case ALM_ID_RF_BOARD_VOLTAGE_SERIOUS:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_VOLTAGE_SERIOUS);
            break;
        case ALM_ID_RF_BOARD_CURRENT_MINOR:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_CURRENT_MINOR);
            break;
        case ALM_ID_RF_BOARD_CURRENT_SERIOUS:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_CURRENT_SERIOUS);
            break;
        case ALM_ID_RF_TTA_VOLTAGE_MINOR:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TTA_VOLTAGE_MINOR);
            break;
        case ALM_ID_RF_TTA_VOLTAGE_SERIOUS:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TTA_VOLTAGE_SERIOUS);
            break;
        case ALM_ID_RF_TTA_CURRENT_MINOR:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TTA_CURRENT_MINOR);
            break;
        case ALM_ID_RF_TTA_CURRENT_SERIOUS:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TTA_CURRENT_SERIOUS);
            break;
        case ALM_ID_RF_TX_POWER_MINOR:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TX_POWER_MINOR);
            break;
        case ALM_ID_RF_TX_POWER_SERIOUS:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TX_POWER_SERIOUS);
            break;
        case ALM_ID_RF_RF_DISABLED:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_RF_DISABLED);
            m_ucRFDisabledMask &= ~(1 << (UINT8)Alarm.GetEntityIndex());
            break;
        case ALM_ID_RF_BOARD_SSP_CHKSUM_ERROR:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_SSP_CHKSUM_ERROR);
            break;
        case ALM_ID_RF_BOARD_RF_CHKSUM_ERROR:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_RF_CHKSUM_ERROR);
            break;
        case ALM_ID_RF_BOARD_RF_NORESPONSE:
            //m_RFAlmState.setRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_RF_NORESPONSE);
            break;

        case ALM_ID_GPS_SIGNAL:
            if(BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                {
                //m_RFAlmState.setAllRFAlarm(E_ALM_GPS_SIGNAL);liuweidong
                }
            break;

        case ALM_ID_GPS_LOC_CLOCK:
            if(BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                {
                //m_RFAlmState.setAllRFAlarm(E_ALM_GPS_LOC_CLOCK); liuweidong
                }
            break;
        #if 0
        case ALM_ID_GPS_LOST:         
            if (BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                {
                m_RFAlmState.setAllRFAlarm(E_ALM_GPS_LOST);
                }
            break;
        #endif
        case ALM_ID_PLL_PLLLOSELOCK_SERIOUS:
            m_RFAlmState.setAllRFAlarm(E_ALM_PLL_LOSE_LOCK);
#if 0
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[PLL lose lock] shutdown entire system.");
//            printf("shutdown entire system.");
            if (true == m_bHandleException)
                {
                //delay();
                //shutdown;
                }
#endif
            break;
#ifdef WBBU_CODE
       case Alarm_ID_WRRU_recv_nopoll:
       	
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[Not Recv RRU HeartBeat] Reset WRRU.");
//            printf("shutdown entire system.");
            if (true == m_bHandleException)
                {
                //delay();
                //shutdown;
                   ResetWrru();
                
                }
#endif
        default:
            break;
    }

    return true;
}


bool CTaskAlm::clearAlarmHandler(CAlarmNotifyOam &Alarm)
{
   // SINT8  DataBuff[2];
    UINT16 usAlarmID = Alarm.GetAlarmCode();
    switch(usAlarmID)
    {
        case ALM_ID_L2PPC_COMMFAIL:
        {
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm[L2PPC Comm] is cleared after L2 reset.");
            s_L2CommFail = false;
            break;
        }

        //case ALM_CODE_AUX_COMMFAIL:
        case ALM_ID_AUX_TO_L2_CONTROL_FAIL:
        case ALM_ID_AUX_TO_L2_RANGING_BUF_NOT_EMPTY:
        case ALM_ID_AUX_L2_TO_AUX_CONTROL_FAIL:
        case ALM_ID_AUX_L2_TO_AUX_WEIGHT_NOT_EMPTY:
        case ALM_ID_AUX_L2_TO_AUX_CONFIG_NOT_EMPTY:
        case ALM_ID_AUX_L2_FROM_AUX_RANGING_NOT_FULL:
        case ALM_ID_AUX_L2_FROM_AUX_RANGING_CHKSUM:
        case ALM_ID_AUX_AUX_FROM_L2_BUF_NOT_FULL:
        case ALM_ID_AUX_AUX_FROM_L2_CHKSUM:
        case ALM_ID_AUX_AUX_TO_FEP_BUF_NOT_EMPTY:
        case ALM_ID_AUX_FEP_FROM_AUX_CHKSUM:
        case ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM:
        {
            break;
        }

        case ALM_ID_MCP_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY:
        case ALM_ID_MCP_L2_TO_MCP_UPLINKPROF_NOT_EMPTY:
        case ALM_ID_MCP_L2_TO_MCP_CONFIG_NOT_EMPTY:
        case ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_NOT_FULL:
        case ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_CHKSUM:
        case ALM_ID_MCP_MCP_TO_L2_UPLINK_NOT_EMPTY:
        case ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_NOT_FULL:
        case ALM_ID_MCP_MCP_FROM_L2_UPPROF_NOT_FULL:
        case ALM_ID_MCP_MCP_FROM_L2_CONFIG_NOT_FULL:
        case ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_CHKSUM:
        case ALM_ID_MCP_MCP_FROM_L2_UPPROF_CHKSUM:
        case ALM_ID_MCP_MCP_FROM_L2_CONFIG_CHKSUM:
        case ALM_ID_MCP_MCP_FEP_ALARM:
        {
            break;
        }

        case ALM_ID_ENV_SYNC_SHUTDOWN:
        {
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] SYNC temperature alarm clear, SYNC power on.");
            configSYNCPower(M_SYNC_POWER_ON);
            break;
        }
        case ALM_ID_ENV_FAN_STOP:
            break;

        case ALM_ID_PLL_TCXO_FREQOFF:       //disable所有天线, MASK为2字节
            m_RFAlmState.clearAllRFAlarm(E_ALM_TCXO_FREQOFF);
            break;

        case ALM_ID_RF_BOARD_VOLTAGE_MINOR:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_VOLTAGE_MINOR);
            break;
        case ALM_ID_RF_BOARD_VOLTAGE_SERIOUS:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_VOLTAGE_SERIOUS);
            break;
        case ALM_ID_RF_BOARD_CURRENT_MINOR:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_CURRENT_MINOR);
            break;
        case ALM_ID_RF_BOARD_CURRENT_SERIOUS:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_CURRENT_SERIOUS);
            break;
        case ALM_ID_RF_TTA_VOLTAGE_MINOR:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TTA_VOLTAGE_MINOR);
            break;
        case ALM_ID_RF_TTA_VOLTAGE_SERIOUS:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TTA_VOLTAGE_SERIOUS);
            break;
        case ALM_ID_RF_TTA_CURRENT_MINOR:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TTA_CURRENT_MINOR);
            break;
        case ALM_ID_RF_TTA_CURRENT_SERIOUS:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TTA_CURRENT_SERIOUS);
            break;
        case ALM_ID_RF_TX_POWER_MINOR:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TX_POWER_MINOR);
            break;
        case ALM_ID_RF_TX_POWER_SERIOUS:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_TX_POWER_SERIOUS);
            break;
        case ALM_ID_RF_RF_DISABLED:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_RF_DISABLED);
            m_ucRFDisabledMask |= (1 << (UINT8)Alarm.GetEntityIndex());
            break;
        case ALM_ID_RF_BOARD_SSP_CHKSUM_ERROR:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_SSP_CHKSUM_ERROR);
            break;
        case ALM_ID_RF_BOARD_RF_CHKSUM_ERROR:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_RF_CHKSUM_ERROR);
            break;
        case ALM_ID_RF_BOARD_RF_NORESPONSE:
            //m_RFAlmState.clearRFAlarm((UINT8)Alarm.GetEntityIndex(), E_ALM_RF_BOARD_RF_NORESPONSE);
            break;

        case ALM_ID_GPS_SIGNAL:
            //if(BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                {
                //m_RFAlmState.clearAllRFAlarm(E_ALM_GPS_SIGNAL);
                }
            break;

        case ALM_ID_GPS_LOC_CLOCK:
            //if(BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                {
               // m_RFAlmState.clearAllRFAlarm(E_ALM_GPS_LOC_CLOCK);
                }
            break;

        case ALM_ID_GPS_LOST:         
            //if (BTS_SYNC_SRC_GPS  == NvRamDataAddr->L1GenCfgEle.SyncSrc)
                {
                    m_RFAlmState.clearAllRFAlarm(E_ALM_GPS_LOST);
                }
            break;

        case ALM_ID_PLL_PLLLOSELOCK_SERIOUS:
	   #ifndef WBBU_CODE
            m_RFAlmState.clearAllRFAlarm(E_ALM_PLL_LOSE_LOCK);
#if 0
            if(true == m_bHandleException)
                {
                //delay();
                //shutdown;
                }
#endif
#else
      m_RFAlmState.clearAllRFAlarm(E_ALM_GPS_LOST);
#endif
            break;
#ifdef WBBU_CODE
      case ALM_ID_10ms_warning:
	  	m_RFAlmState.clearAllRFAlarm(E_ALM_GPS_LOST);
		break;

#endif
        default:
            break;
    }

    return true;
}


bool CTaskAlm::sendAlarmHandleMsg(UINT16 MsgId, TID DstTID,SINT8* pData, UINT16 Len)
{
    CL3OamAlarmHandleNotify NotifyMessage;
    if (false == NotifyMessage.CreateMessage(*this))
        {
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Encounter system exceptions, create message failed.");
        return false;
        }
    NotifyMessage.SetMessageId(MsgId);
    NotifyMessage.SetSrcTid(M_TID_FM);
    NotifyMessage.SetDstTid(DstTID);
    if (Len > NotifyMessage.GetDataLength() - 2)
        {
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Message is too long, cant send this message.");
        NotifyMessage.DeleteMessage();
        return false;
        }
    NotifyMessage.SetTransactionId(OAM_DEFAUIT_TRANSID);
    NotifyMessage.SetMsgData(pData, Len);
    
    if (true != NotifyMessage.Post())
        {
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] post msg[0x%04x] fail", MsgId);
        NotifyMessage.DeleteMessage();
        return false;
        }

    return true; 
}

bool CTaskAlm::deleteAlarmNotify(CDeleteAlarmNotify &Notify)
{
    list<T_AlarmHandle*>::iterator it;
    for(it = m_CurAlmList.begin(); it != m_CurAlmList.end(); ++it)
        {
        T_AlarmHandle *pCurAlmHandler = (*it);
        if ((pCurAlmHandler->pAlarmNotify->GetAlarmCode()    == Notify.GetAlarmCode())&&
            (pCurAlmHandler->pAlarmNotify->GetEntityType()   == Notify.GetEntityType())&&
            ((pCurAlmHandler->pAlarmNotify->GetEntityIndex() == Notify.GetEntityIndex())))
            {
            pCurAlmHandler->pAlarmNotify->SetFlag(ALM_FLAG_CLEAR);
            pCurAlmHandler->pAlarmNotify->SetSequenceNum(++m_AlmSeqID);
            sendActiveAlmToEMS(*(pCurAlmHandler->pAlarmNotify)); //向EMS上报告警清除
            m_CurAlmList.erase(it);      //将当前告警清除
            delete pCurAlmHandler;
            return true;
            }
        }

    return true;
}


bool CTaskAlm::getActiveAlmNotify()
{
    for (list<T_AlarmHandle*>::iterator it = m_CurAlmList.begin(); it != m_CurAlmList.end(); ++it )
        {
        sendActiveAlmToEMS(*(*it)->pAlarmNotify);    //組消息上報EMS
        }

    return true;
}

//从告警管理列表中获取告警信息上报EMS
bool CTaskAlm::sendActiveAlmToEMS(CAlarmNotifyOam &AlarmNotify)
{
    OAM_LOGSTR4(LOG_DEBUG3, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] send Alarm to EMS, flag[%d] code[%04x] type[%d] index[%d]", AlarmNotify.GetFlag(), AlarmNotify.GetAlarmCode(), AlarmNotify.GetEntityType(), AlarmNotify.GetEntityIndex());

    CAlarmNotifyEms Notify(AlarmNotify);
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetSrcTid(M_TID_FM);    
    Notify.SetDstTid(M_TID_EMSAGENTTX);
	#ifdef WBBU_CODE
	if(Notify.GetAlarmCode()!=Alarm_ID_WRRU_Reset)
	{
    	if(true != Notify.Post())
        {
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] FAIL to report alarm to EMS!");
        return false;
        }
	}
	else
		{
		   if(rru_report_flag==1)
		   	{
		   	    if(true != Notify.Post())
		        {
		        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] FAIL to report alarm to EMS!");
		        return false;
		        }
				rru_report_flag = 0;
		   	}
		}
	#else

        if(true != Notify.Post())
        {
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] FAIL to report alarm to EMS!");
        return false;
        }


	#endif

    return true;
}

CTimer *CTaskAlm::InitTimer(ALARMTimer type)
{
    if (type >= E_TIMER_ALARM_MAX)
        {
        return NULL;
        }
    CComMessage *pComMsg = new(this, sizeof(stAlarmTimerExpire))CComMessage;
    if (NULL == pComMsg)
        {
        return NULL;
        }
    pComMsg->SetMessageId(M_OAM_ALARM_SEND_TO_EMS_TIMER);
    pComMsg->SetDstTid(M_TID_FM);
    pComMsg->SetSrcTid(M_TID_FM);
    ((stAlarmTimerExpire*)pComMsg->GetDataPtr())->type = type;

    UINT32 ulInterval = 0;
    if (E_TIMER_ALARM_KEEP_CLEAR == type)
        {
        ulInterval = M_ALARM_INTERVAL_KEEP_CLEAR;
        }
    else
        {
        ulInterval = M_ALARM_INTERVAL_KEEP_NOT_CLEAR;   //milli-seconds
        }
    CTimer *tmpTimer = new CTimer(true, ulInterval, pComMsg);
    if (NULL == tmpTimer)
        {
        //删除消息
        pComMsg->Destroy();
        }

    return tmpTimer;
}


void CTaskAlm::AlarmTimeOut(CMessage &msg)
{
    ALARMTimer type = ((stAlarmTimerExpire*)msg.GetDataPtr())->type;
    if (E_TIMER_ALARM_KEEP_CLEAR == type)
        {
        ProcessClearedAlarm();
        }
    else if (E_TIMER_ALARM_KEEP_NOT_CLEAR == type)
        {
        CheckAlarmKeepNotClear();
        }
    else
        {
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Unexpected timer type[%d]", type);
        }
}


void CTaskAlm::ProcessClearedAlarm()
{
#if 0
#define M_ALARM_MAX_SEND_CNT    (12)
    T_AlarmHandle *pCurAlmHandler = NULL;
    SINT8 counter = (m_listBufferedAlarm.size() > M_ALARM_MAX_SEND_CNT)?M_ALARM_MAX_SEND_CNT:m_listBufferedAlarm.size();
    for(; counter > 0; --counter)
        {
        pCurAlmHandler = m_listBufferedAlarm.front();
        m_listBufferedAlarm.pop_front();
        HandleAlarm(pCurAlmHandler);
        //为逻辑简单，T_AlarmHandle在HandleAlarm()重新申请空间
        delete pCurAlmHandler;
        }
#endif

//#define M_ALARM_MAX_SEND_CNT    (12)
    T_AlarmHandle *pCurAlmHandler = NULL;
//    SINT8 counter = 0;
    list<T_AlarmHandle*>::iterator it;
    list<T_AlarmHandle*>::iterator tmp_it;
    for (it = m_listClearedAlarm.begin(); it != m_listClearedAlarm.end(); )
        {
        pCurAlmHandler = *it;
        tmp_it = it;
        ++it;
        pCurAlmHandler->slKeepAlarmSeconds -= M_ALARM_INTERVAL_KEEP_CLEAR;
        if (pCurAlmHandler->slKeepAlarmSeconds < 0)
            {
            pCurAlmHandler->slKeepAlarmSeconds = 0;
            }
        if (0 == pCurAlmHandler->slKeepAlarmSeconds)
            {
            //告警处理
            pCurAlmHandler->pAlarmNotify->SetSequenceNum(++m_AlmSeqID);
            sendActiveAlmToEMS(*(pCurAlmHandler->pAlarmNotify));                //向EMS上报告警产生
            m_listClearedAlarm.erase(tmp_it);
            delete pCurAlmHandler;
            }
#if 0
        ++counter;
        if (counter > M_ALARM_MAX_SEND_CNT)
            break;
#endif
        }

    return;
}

#if 0
bool CTaskAlm::HandleAlarm(T_AlarmHandle *pInAlarmHandler) 
{
    if ((NULL == pInAlarmHandler)||(NULL == pInAlarmHandler->pAlarmNotify))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Err alarm handler[%x].", (UINT32)pInAlarmHandler);
        return false;
        }

    bool found = false;          //首先判断是否有告警记录
    list<T_AlarmHandle*>::iterator it;
    CAlarmNotifyOam *pInAlarm = pInAlarmHandler->pAlarmNotify;
    T_AlarmHandle   *pCurAlmHandler = NULL;

    UINT16 Code  = pInAlarm->GetAlarmCode();
    UINT16 Type  = pInAlarm->GetEntityType();
    UINT16 Index = pInAlarm->GetEntityIndex();
    for(it = m_CurAlmList.begin(); it != m_CurAlmList.end(); ++it)
        {
        pCurAlmHandler   = (*it);
        UINT16 TempCode  = pCurAlmHandler->pAlarmNotify->GetAlarmCode();
        UINT16 TempType  = pCurAlmHandler->pAlarmNotify->GetEntityType();
        UINT16 TempIndex = pCurAlmHandler->pAlarmNotify->GetEntityIndex();
        if((TempCode == Code) && (TempType == Type) && (TempIndex == Index))
            {
            found = true;
            break;
            }
        }

    UINT8 AlmFlg = pInAlarm->GetFlag();
    if((false == found)&&(ALM_FLAG_SET == AlmFlg))
        {
        //重新申请控制块
        //传入的控制块在函数外释放
        CAlarmNotifyOam *pCloneAlmNotify = (CAlarmNotifyOam *)pInAlarm->Clone();
        if (NULL == pCloneAlmNotify)
            {
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Encounter system exception. Message.clone() failed.");
            return false;
            }
        T_AlarmHandle *pAlarmHandler = new T_AlarmHandle(pCloneAlmNotify);
        if (NULL == pAlarmHandler)
            {
            OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Encounter system exception. new(...) failed.");
            pCloneAlmNotify->DeleteMessage();
            delete pCloneAlmNotify;
            return false;
            }

        pAlarmHandler->pAlarmNotify->SetFlag(AlmFlg);
        pAlarmHandler->slKeepAlarmSeconds = getAlarmTimeByID(Code);
        pInAlarm->SetSequenceNum(++m_AlmSeqID);
        m_CurAlmList.push_back(pAlarmHandler);        //将新告警添加到告警列表
        sendActiveAlmToEMS(*pInAlarm);             //向EMS上报告警产生
        return true;
        }

    if((false == found)&&(ALM_FLAG_CLEAR == AlmFlg))
        {
        return true;                 //忽略此种消息         
        }

    if((true == found)&&(ALM_FLAG_SET == AlmFlg))
        {
        //pCurAlmHandler->slKeepAlarmSeconds = getAlarmTimeByID(Code);
        pCurAlmHandler->pAlarmNotify->SetAlarmInfo(pInAlarm->GetAlarmInfo());
        pCurAlmHandler->pAlarmNotify->SetSequenceNum(++m_AlmSeqID);
        sendActiveAlmToEMS(*pInAlarm);                //向EMS上报告警产生
        return true;
        }

    if((true == found)&&(ALM_FLAG_CLEAR == AlmFlg))
        {
        pInAlarm->SetSequenceNum(++m_AlmSeqID);
        sendActiveAlmToEMS(*pInAlarm);  //向EMS上报告警清除
        m_CurAlmList.erase(it);         //将当前告警清除
        clearAlarmHandler(*pInAlarm);
        delete pCurAlmHandler;
        return true;
        }

    return true;
}
#endif

void CTaskAlm::CheckAlarmKeepNotClear()
{
    list<T_AlarmHandle*>::iterator it;
    T_AlarmHandle *pCurAlmHandler = NULL;

    for(it = m_CurAlmList.begin(); it != m_CurAlmList.end(); ++it)
        {
        pCurAlmHandler = (*it);
        if (M_ALM_MAX_ENDURE_SEC_DEF == pCurAlmHandler->slKeepAlarmSeconds)
            {
            continue;
            }
        //维护时间值
        pCurAlmHandler->slKeepAlarmSeconds -= M_ALARM_INTERVAL_KEEP_NOT_CLEAR;
        if (pCurAlmHandler->slKeepAlarmSeconds < 0)
            {
            pCurAlmHandler->slKeepAlarmSeconds = 0;
            }
        if (0 == pCurAlmHandler->slKeepAlarmSeconds)
            {
            //告警处理
            setAlarmHandler(pCurAlmHandler);
            //已经处理过标志
            pCurAlmHandler->slKeepAlarmSeconds = M_ALM_MAX_ENDURE_SEC_DEF;
            }
        }

    //Check if RF enable or disable.
    if ((true == m_bHandleException) && (true == m_RFAlmState.isRFMaskModified()))
        {
////////UINT16 usRFMask = NvRamDataAddr->L1GenCfgEle.AntennaMask & m_RFAlmState.getRFMask();
        UINT16 usRFMask = getCurrentRFMask();
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Alarm Modify RF Mask by CheckAlarmKeepNotClear [0x%.4X].", usRFMask);
#ifdef WBBU_CODE
         sendAnntenaMsk(usRFMask,1,4);
            m_RFAlmState.RFMaskRefresh();
#else
        if (true == sendAlarmHandleMsg(M_L3_L2_CFG_ANTENNA_NOTIFY, M_TID_L2MAIN, (SINT8*)&usRFMask, sizeof(usRFMask)))
            {
            m_RFAlmState.RFMaskRefresh();
            }
#endif
        }
}


SINT32 CTaskAlm::getAlarmTimeByID(UINT16 usAlarmID)
{
    map<UINT16, UINT32>::iterator it = m_mapAlmID_KeepTime.find(usAlarmID);

    if ( it != m_mapAlmID_KeepTime.end() )
        {
        //返回Index.
        return it->second;
        }
    return M_ALM_MAX_ENDURE_SEC_DEF;
}


bool CTaskAlm::configSYNCPower(UINT16 usOp)
{
    if(usOp > M_SYNC_POWER_RESET)
        {
        ////invalid Operation BITS
        OAM_LOGSTR1(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "[tAlm] Invalid SYNC Power bits[0x%x], rather than 0,1,2.", usOp);
        }
    CL3L2BtsConfigSYNPower msgCfgSYNPower;
    if (false == msgCfgSYNPower.CreateMessage(*this))
        return false;
    msgCfgSYNPower.SetTransactionId(OAM_DEFAUIT_TRANSID);
    msgCfgSYNPower.SetDstTid(M_TID_L2MAIN);
    msgCfgSYNPower.SetSrcTid(M_TID_FM);
    msgCfgSYNPower.SetOp(usOp);
    if(false == msgCfgSYNPower.Post())
        {
        OAM_LOGSTR1(LOG_DEBUG3, L3SYS_ERROR_PRINT_SYS_INFO, "[tAlm] post Config SYNC Power[0x%x] message to L2 fail.", usOp);
        msgCfgSYNPower.DeleteMessage();
        return false;
        }

    return true;
}

#ifndef WBBU_CODE

/*
 *RF告警,只需要关闭对应的天线即可
 */
bool CRFAlmState::setRFAlarm(UINT8 ucAntennaIdx, E_AlmType type)
{
    if ((ucAntennaIdx >= ANTENNA_NUM) || (type >= E_ALM_RF_MAX))
        {
        OAM_LOGSTR2(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Unexpected RF index[%d] or Alarm type[%d]", ucAntennaIdx, type);
        return false;
        }

    if (0 == m_arrCard[ucAntennaIdx])
        {
        //原来该RF不存在告警.
        m_bRFMaskModified = true;
        }

    UINT32 ulTypeMask       =  (1 << type);
    m_arrCard[ucAntennaIdx] |= ulTypeMask;

    return true;
}

bool CRFAlmState::setAllRFAlarm(E_AlmType type)
{
    if ((type < E_ALM_RF_MAX) || (type > E_ALM_MAX))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Unexpected Alarm type[%d]", type);
        return false;
        }

    UINT32 ulTypeMask       = (1 << type);
    for (UINT8 ucAntennaIdx = 0; ucAntennaIdx < ANTENNA_NUM; ++ucAntennaIdx)
        {
        if (0 == m_arrCard[ucAntennaIdx])
            {
            //原来该RF不存在告警.
            m_bRFMaskModified = true;
            }
        m_arrCard[ucAntennaIdx] |= ulTypeMask;
        }

    return true;
}


/*
 *有些告警，需要关闭所有RF的告警,如TCXO,GPS告警
 */
bool CRFAlmState::clearRFAlarm(UINT8 ucAntennaIdx, E_AlmType type)
{
    if ((ucAntennaIdx >= ANTENNA_NUM) || (type >= E_ALM_RF_MAX))
        {
        OAM_LOGSTR2(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Unexpected RF index[%d] or Alarm type[%d]", ucAntennaIdx, type);
        return false;
        }

    UINT32 ulTypeMask       =  (1 << type);
    m_arrCard[ucAntennaIdx] &= ~ulTypeMask;
    if (0 == m_arrCard[ucAntennaIdx])
        {
        //原来该RF不存在告警.
        m_bRFMaskModified = true;
        }

    return true;
}


bool CRFAlmState::clearAllRFAlarm(E_AlmType type)
{
    if ((type < E_ALM_RF_MAX) || (type >= E_ALM_MAX))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Unexpected Alarm type[%d]", type);
        return false;
        }

    UINT32 ulTypeMask       = (1 << type);
    for (UINT8 ucAntennaIdx = 0; ucAntennaIdx < ANTENNA_NUM; ++ucAntennaIdx)
        {
        m_arrCard[ucAntennaIdx] &= ~ulTypeMask;
        if (0 == m_arrCard[ucAntennaIdx])
            {
            //原来该RF不存在告警.
            m_bRFMaskModified = true;
            }
        }

    return true;
}
#else
/*
 *RF告警,只需要关闭对应的天线即可
 */
bool CRFAlmState::setRFAlarm(UINT8 ucAntennaIdx, E_AlmType type)
{
    if ((ucAntennaIdx >= ANTENNA_NUM) /*|| (type >= E_ALM_RF_MAX)*/)
        {
        OAM_LOGSTR2(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Unexpected RF index[%d] or Alarm type[%d]", ucAntennaIdx, type);
        return false;
        }

    if (0 == m_arrCard[ucAntennaIdx])
        {
        //原来该RF不存在告警.
         OAM_LOGSTR1(LOG_DEBUG1, L3FM_ERROR_PRINT_SW_INFO, "[tAlm1]  Alarm type[%d]", ucAntennaIdx);
        m_bRFMaskModified = true;
        }

    UINT32 ulTypeMask       =  (1 << type);
    m_arrCard[ucAntennaIdx] |= ulTypeMask;

    return true;
}

bool CRFAlmState::setAllRFAlarm(E_AlmType type)
{
    if ((type < E_ALM_RF_MAX) || (type > E_ALM_MAX))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm2] Unexpected Alarm type[%d]", type);
        return false;
        }

    UINT32 ulTypeMask       = (1 << type);
    for (UINT8 ucAntennaIdx = 0; ucAntennaIdx < ANTENNA_NUM; ++ucAntennaIdx)
        {
        if (0 == m_arrCard[ucAntennaIdx])
            {
            //原来该RF不存在告警.
             OAM_LOGSTR1(LOG_DEBUG1, L3FM_ERROR_PRINT_SW_INFO, "[tAlm2]  Alarm type[%d]", ucAntennaIdx);
            m_bRFMaskModified = true;
            }
        m_arrCard[ucAntennaIdx] |= ulTypeMask;
        }

    return true;
}


/*
 *有些告警，需要关闭所有RF的告警,如TCXO,GPS告警
 */
bool CRFAlmState::clearRFAlarm(UINT8 ucAntennaIdx, E_AlmType type)
{
    if ((ucAntennaIdx >= ANTENNA_NUM) /*|| (type >= E_ALM_RF_MAX)*/)
        {
        OAM_LOGSTR2(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Unexpected RF index[%d] or Alarm type[%d]", ucAntennaIdx, type);
        return false;
        }

    UINT32 ulTypeMask       =  (1 << type);
   
    if (1 == m_arrCard[ucAntennaIdx])
        {
        //原来该RF不存在告警.
           OAM_LOGSTR1(LOG_DEBUG1, L3FM_ERROR_PRINT_SW_INFO, "[tAlm3]  Alarm type[%d]", ucAntennaIdx);
        m_bRFMaskModified = true;
        }
       m_arrCard[ucAntennaIdx]  &= ~ulTypeMask;
    return true;
}


bool CRFAlmState::clearAllRFAlarm(E_AlmType type)
{
    if ((type < E_ALM_RF_MAX) || (type >= E_ALM_MAX))
        {
        OAM_LOGSTR1(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] Unexpected Alarm type[%d]", type);
        return false;
        }

    UINT32 ulTypeMask       = (1 << type);
    for (UINT8 ucAntennaIdx = 0; ucAntennaIdx < ANTENNA_NUM; ++ucAntennaIdx)
        {
       
        if (1 == m_arrCard[ucAntennaIdx])
            {
            //原来该RF不存在告警.
             OAM_LOGSTR1(LOG_DEBUG1, L3FM_ERROR_PRINT_SW_INFO, "[tAlm4]  Alarm type[%d]", ucAntennaIdx);
            m_bRFMaskModified = true;
            }
             m_arrCard[ucAntennaIdx] &= ~ulTypeMask;
        }

    return true;
}
#endif
/*
 *RF mask
 */
UINT16 CRFAlmState::getRFMask()
{
    UINT16 mask = 0;
    for(int i = 0; i < ANTENNA_NUM; ++i)
        {
        if (0 != m_arrCard[i])
        mask |= 1<<i;
        }
    return (~mask);
}



/*************************************************
 *全局接口函数，往Alarm任务发送告警
 ************************************************/
bool AlarmReport(UINT8   Flag, UINT16  EntityType,
                 UINT16  EntityIndex, UINT16  AlarmCode,      
                 UINT8   Severity, const char format[],...)
{
    CAlarmNotifyOam  Notify;
    if (false == Notify.CreateMessage(*CTaskAlm::GetInstance()))
        return false;
    Notify.SetTransactionId(OAM_DEFAUIT_TRANSID);
    Notify.SetDstTid(M_TID_FM);
    Notify.SetSrcTid(M_TID_FM);
    Notify.SetSequenceNum(0XFFFFFFFF);
    T_TimeDate TD = bspGetDateTime();
    Notify.SetFlag(Flag);
    Notify.SetYear(TD.year);
    Notify.SetMonth(TD.month);
    Notify.SetDay(TD.day);
    Notify.SetHour(TD.hour);
    Notify.SetMinute(TD.minute);
    Notify.SetSecond(TD.second);

    //对此告警   0-标识告警链路正常,  1-标识链路通信断
    Notify.SetEntityType(EntityType);
    Notify.SetEntityIndex(EntityIndex);
    Notify.SetAlarmCode(AlarmCode);

    //告警级别等信息可由告警信息表中查询得到
    Notify.SetSeverity(Severity);
    if (ALM_FLAG_SET == Flag)
        {
        UINT16  InfoLen = 0;
        va_list vaList;
        char fmt[ALM_INFO_LEN]={0};
     //   int  nChars;
        va_start(vaList, format);
        InfoLen = ::vsprintf((char*)fmt, format, vaList);
        va_end(vaList);

        //InfoLen:the number of characters copied into fmt, not including the NULL terminator.
        //so, the actual string length is (InfoLen + 1);
        Notify.SetInfoLen(InfoLen + 1);
        Notify.SetAlarmInfo((const SINT8*)fmt);
        }
    else
        {
        //恢复告警不发送内容
        Notify.SetInfoLen(0);
        }

    //设置告警的真实长度
    Notify.SetAlarmLength();

    if (true != Notify.Post())
        {
        OAM_LOGSTR(LOG_SEVERE, L3FM_ERROR_PRINT_SW_INFO, "[tAlm] post Alarm Notify msg to tAlm fail");
        Notify.DeleteMessage();
        return false;
        }

    return true;
}



void l3oamshowalarm()
{
    CTaskAlm *pInstance =  CTaskAlm::GetInstance();
    list<T_AlarmHandle*>::iterator it; 
    printf("\r\nHandle   exceptions : %s", (true == pInstance->isHandleException())?"true":"false");
////printf("\r\nBuffered Alarms     : %d", pInstance->m_listBufferedAlarm.size());
    for(it = pInstance->m_CurAlmList.begin(); it != pInstance->m_CurAlmList.end(); ++it )
        {
        (*it)->pAlarmNotify->show();
        }
    printf("\r\n\r\nAll current alarms  : %d", pInstance->m_CurAlmList.size());
    printf("\r\n\r\n----END----");
}

#if 1
//测试完可以删除代码, alarm任务的friend也可以删除
void l3oamresetsyn(UINT8 ucType)
{
    CTaskAlm *pInstance =  CTaskAlm::GetInstance();
    switch (ucType)
        {
        case 0:
            printf("\r\nSynthesizer cards power OFF");
            pInstance->configSYNCPower(ucType);
            break;
        case 1:
            printf("\r\nSynthesizer cards power ON");
            pInstance->configSYNCPower(ucType);
            break;
        case 2:
            printf("\r\nSynthesizer cards RESET");
            pInstance->configSYNCPower(ucType);
            break;
        default:
            printf("\r\n l3oamresetsyn(0): Synthesizer cards power OFF");
            printf("\r\n l3oamresetsyn(1): Synthesizer cards power ON");
            printf("\r\n l3oamresetsyn(2): Synthesizer cards RESET");
            printf("\r\n");
            break;
        }

    return;
}
#endif



static const struct
{
    UINT32 almID;
    UINT8  str[300];
} ALM_id_str_tbl[] = {
    {ALM_ID_BTS_UPGRADE_FAIL                       ,STR_BTS_BKVERFAIL},
    {ALM_ID_BTS_SAG_LINK                           ,STR_BTS_SAG_LINK},
    {ALM_ID_L2PPC_COMMFAIL                         ,STR_L3L2LINKFAIL}, 
    {ALM_ID_L2PPC_RESET                            ,STR_L2PPCRESET}, 

    {ALM_ID_MCP_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY   ,STR_L2_TO_MCP_DOWNLINKDATA_NOT_EMPTY}, 
    {ALM_ID_MCP_L2_TO_MCP_UPLINKPROF_NOT_EMPTY     ,STR_L2_TO_MCP_UPLINKPROF_NOT_EMPTY}, 
    {ALM_ID_MCP_L2_TO_MCP_CONFIG_NOT_EMPTY         ,STR_L2_TO_MCP_CONFIG_NOT_EMPTY}, 
    {ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_NOT_FULL    ,STR_L2_FROM_MCP_UPLINKDATA_NOT_FULL}, 
    {ALM_ID_MCP_L2_FROM_MCP_UPLINKDATA_CHKSUM      ,STR_L2_FROM_MCP_UPLINKDATA_CHKSUM}, 
    {ALM_ID_MCP_MCP_TO_L2_UPLINK_NOT_EMPTY         ,STR_MCP_TO_L2_UPLINK_NOT_EMPTY}, 
    {ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_NOT_FULL      ,STR_MCP_FROM_L2_DOWNLINK_NOT_FULL}, 
    {ALM_ID_MCP_MCP_FROM_L2_UPPROF_NOT_FULL        ,STR_MCP_FROM_L2_UPPROF_NOT_FULL}, 
    {ALM_ID_MCP_MCP_FROM_L2_CONFIG_NOT_FULL        ,STR_MCP_FROM_L2_CONFIG_NOT_FULL}, 
    {ALM_ID_MCP_MCP_FROM_L2_DOWNLINK_CHKSUM        ,STR_MCP_FROM_L2_DOWNLINK_CHKSUM}, 
    {ALM_ID_MCP_MCP_FROM_L2_UPPROF_CHKSUM          ,STR_MCP_FROM_L2_UPPROF_CHKSUM}, 
    {ALM_ID_MCP_MCP_FROM_L2_CONFIG_CHKSUM          ,STR_MCP_FROM_L2_CONFIG_CHKSUM}, 
    {ALM_ID_MCP_MCP_FEP_ALARM                      ,STR_MCP_TO_FEP_NOT_EMPTY}, 

    {ALM_ID_AUX_TO_L2_CONTROL_FAIL                 ,STR_AUX_TO_L2_CONTROL_FAIL}, 
    {ALM_ID_AUX_TO_L2_RANGING_BUF_NOT_EMPTY        ,STR_AUX_TO_L2_RANGING_BUF_NOT_EMPTY}, 
    {ALM_ID_AUX_L2_TO_AUX_CONTROL_FAIL             ,STR_L2_TO_AUX_CONTROL_FAIL}, 
    {ALM_ID_AUX_L2_TO_AUX_WEIGHT_NOT_EMPTY         ,STR_L2_TO_AUX_WEIGHT_NOT_EMPTY}, 
    {ALM_ID_AUX_L2_TO_AUX_CONFIG_NOT_EMPTY         ,STR_L2_TO_AUX_CONFIG_NOT_EMPTY}, 
    {ALM_ID_AUX_L2_FROM_AUX_RANGING_NOT_FULL       ,STR_L2_FROM_AUX_RANGING_NOT_FULL}, 
    {ALM_ID_AUX_L2_FROM_AUX_RANGING_CHKSUM         ,STR_L2_FROM_AUX_RANGING_CHKSUM}, 
    {ALM_ID_AUX_AUX_FROM_L2_BUF_NOT_FULL           ,STR_AUX_FROM_L2_BUF_NOT_FULL}, 
    {ALM_ID_AUX_AUX_FROM_L2_CHKSUM                 ,STR_AUX_FROM_L2_CHKSUM}, 
    {ALM_ID_AUX_AUX_TO_FEP_BUF_NOT_EMPTY           ,STR_AUX_TO_FEP_BUF_NOT_EMPTY}, 
    {ALM_ID_AUX_FEP_FROM_AUX_CHKSUM                ,STR_FEP_FROM_AUX_CHKSUM}, 
    {ALM_ID_AUX_AUX_FROM_FEP_CHKSUM_BUF_ALM        ,STR_AUX_FROM_FEP_CHKSUM}, 

    {ALM_ID_ENV_SYNC_TEMPERATURE                   ,STR_SYNC_TEMPERATURE}, 
    {ALM_ID_ENV_DIGITAL_BOARD_TEMPERATURE          ,STR_DIGITAL_BORD_TEMPERATURE},
    {ALM_ID_ENV_FAN_STOP                           ,STR_ENV_FAN_STOP},
    {ALM_ID_ENV_SYNC_SHUTDOWN                      ,STR_SYNC_SHUTDOWN}, 

    {ALM_ID_PLL_TCXO_FREQOFF                       ,STR_TCXOFRENOFFSET}, 
    {ALM_ID_PLL_PLLLOSELOCK_MINOR                  ,STR_PLLLOSELOCK_MINOR}, 
    {ALM_ID_PLL_PLLLOSELOCK_SERIOUS                ,STR_PLLLOSELOCK_SERIOUS}, 
    {ALM_ID_PLL_FACTORY_DATA                       ,STR_FSC_FACTORY_DATA}, 
    {ALM_ID_PLL_SSP_CHECKSUM_ERROR                 ,STR_FSC_SSP_CHECKSUM_ERROR}, 
    {ALM_ID_PLL_AUX2SYNC_CHECKSUM_ERROR            ,STR_FSC_AUX2SYNC_CHECKSUM_ERROR}, 
    {ALM_ID_PLL_SYNC_NORESP_ERROR                  ,STR_FSC_SYNC_NORESP_ERROR}, 

    {ALM_ID_RF_BOARD_VOLTAGE_MINOR                 ,STR_BOARD_VOLTAGE_OUTOFRANGE_MINOR}, 
    {ALM_ID_RF_BOARD_VOLTAGE_SERIOUS               ,STR_BOARD_VOLTAGE_OUTOFRANGE_SERIOUS}, 
    {ALM_ID_RF_BOARD_CURRENT_MINOR                 ,STR_BOARD_CURRENT_OUTOFRANGE_MINOR}, 
    {ALM_ID_RF_BOARD_CURRENT_SERIOUS               ,STR_BOARD_CURRENT_OUTOFRANGE_SERIOUS}, 
    {ALM_ID_RF_TTA_VOLTAGE_MINOR                   ,STR_TTA_VOLTAGE_OUTOFRANGE_MINOR}, 
    {ALM_ID_RF_TTA_VOLTAGE_SERIOUS                 ,STR_TTA_VOLTAGE_OUTOFRANGE_SERIOUS}, 
    {ALM_ID_RF_TTA_CURRENT_MINOR                   ,STR_TTA_CURRENT_OUTOFRANGE_MINOR}, 
    {ALM_ID_RF_TTA_CURRENT_SERIOUS                 ,STR_TTA_CURRENT_OUTOFRANGE_SERIOUS}, 
    {ALM_ID_RF_TX_POWER_MINOR                      ,STR_TX_POWER_OUTOFRANGE_MINOR}, 
    {ALM_ID_RF_TX_POWER_SERIOUS                    ,STR_TX_POWER_OUTOFRANGE_SERIOUS}, 
    {ALM_ID_RF_RF_DISABLED                         ,STR_RF_DISABLED}, 
    {ALM_ID_RF_BOARD_SSP_CHKSUM_ERROR              ,STR_BOARD_SSP_CHKSUM_ERROR}, 
    {ALM_ID_RF_BOARD_RF_CHKSUM_ERROR               ,STR_BOARD_RF_CHKSUM_ERROR}, 
    {ALM_ID_RF_BOARD_RF_NORESPONSE                 ,STR_BOARD_RF_NORESPONSE}, 

    {ALM_ID_GPS_SIGNAL                             ,STR_GPS_MNT_FAILURE_CNT}, 
    {ALM_ID_GPS_LOC_CLOCK                          ,STR_GPS_TRACKIN_FAILURE_CNT}, 
    {ALM_ID_GPS_LOST                               ,STR_GPS_LOST}, 
    {ALM_ID_GPS_CFG_ERROR                          ,STR_GPS_CFG_ERROR}, 
    {ALM_ID_GPS_COMMNICATION_FAIL                  ,STR_GPS_COMMUNICATE_FAIL}, 
    {ALM_ID_GPS_ANTENNA_NOT_CONNECTED              ,STR_GPS_ANTENNA_NOT_CONNECTED}, 
    {0xFFFF,""}
};

/*
 *通过AlarmID,查找显示字符串
 */
void l3oamqueryalarm(UINT16 usAlarmID)
{
    for(UINT16 idx = 0; ;++idx)
        {
        if (ALM_id_str_tbl[idx].almID == 0xFFFF)
            {
            printf("\r\nAlarm[0x%.4x] not found", usAlarmID);
            break;
            }
        if (ALM_id_str_tbl[idx].almID == usAlarmID)
            {
            printf("\r\nAlarm[0x%.4x]:\r\n%s", usAlarmID, ALM_id_str_tbl[idx].str);
            break;
            }
        }
    printf("\r\n");
    return;
}
#ifdef WBBU_CODE
void sendAnntenaMsk(unsigned short  antennamask,unsigned char flag,unsigned char flag1)
{
  //     CTaskAlm *pInstance =  CTaskAlm::GetInstance();
//       pInstance->sendAlarmHandleMsg(M_L3_L2_CFG_ANTENNA_NOTIFY, M_TID_L2MAIN, (SINT8*)&antennamask, sizeof(antennamask));
    T_L1GenCfgEle *pCfgEle = (T_L1GenCfgEle *)&(NvRamDataAddr->L1GenCfgEle);
   
   CComMessage *pComMsg = NULL;
   unsigned short *p;
   pComMsg = new (CTaskAlm::GetInstance(), 8) CComMessage;
   pComMsg->SetDstTid(M_TID_L2MAIN);
  pComMsg->SetSrcTid(M_TID_CM);
   pComMsg->SetMoudlue(0);
   pComMsg->SetMessageId(M_L3_L2_L1_GENERAL_SETTING_REQ);
   pComMsg->SetEID(0x12345678);
   p =(unsigned short*)pComMsg->GetDataPtr();
   p[0] = 0xffff;
   p[1] = pCfgEle->SyncSrc;
   p[2] = pCfgEle->GpsOffset;
   p[3] = antennamask;//pCfgEle->AntennaMask;
   if(false==CComEntity::PostEntityMessage(pComMsg))
     	 {
     	     	printf("hello2\n",0,1,2,3,4,5);
     	  }
        
       WrruRFC((unsigned char )antennamask,flag,flag1);
}
#endif
