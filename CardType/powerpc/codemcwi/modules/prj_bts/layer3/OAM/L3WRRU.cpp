/*****************
*file: boot_task.c
*Author: dingfojin
*date: 2005.12.2
****************************/

#include "L3WRRU.h"
#include "ErrorCodeDef.h"
#include "L3OamCommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "sysBtsConfigData.h"
#include "L3OamCfgCommon.h"
#include "L3EmsMessageId.h"
//#include "PciIf.h"
#include "Log.h"
#define LONGSWAP_D(value)  (value)

#if !defined(__WIN32_SIM__)&&!defined(__NUCLEUS__)
    #include <ioLib.h>
    #include <stat.h>
    #include <netDrv.h>
    #include <fiolib.h>
    #include <ramDrv.h>
    #include <dosFsLib.h>
    #include <remLib.h>
    #include <taskLib.h>
    #include <inetLib.h>
    #include <vmLib.h>
#endif

#include "mcWill_bts.h"

#include "Message.h"
#include "ComMessage.h"
#include "Timer.h"
#include "log.h"
#include <sysLib.h>
#include "L3OamCommonReq.h"
#include "L3OamAlm.h"
//#define NVRAM_BASE_ADDR_OAM 0x1000000;
//extern T_NvRamData *NvRamDataAddr;//= (T_NvRamData*)NVRAM_BASE_ADDR_OAM;


unsigned char Loading_Wrru_Code = 0;
extern T_NvRamData *NvRamData; //= (T_NvRamData*)NVRAM_BASE_ADDR_OAM;
SEM_ID CWRRU::s_semWriteNvram    = NULL;

CWRRU* CWRRU::instance=NULL;
 extern "C" void fpga_wrru_code(unsigned char *pcode);
 extern bool AlarmReport(UINT8   Flag, UINT16  EntityType,
                 UINT16  EntityIndex, UINT16  AlarmCode,      
                 UINT8   Severity, const char format[],...);
 extern "C" void WrruRFC(unsigned char antennamask,unsigned char flag,unsigned char flag1);
 extern "C" void ResetWrru();
 extern "C"  unsigned short  Read_Fpga_Alarm();
 extern "C" unsigned short Read_Fiber_Delay(unsigned char index);
extern bool    bGpsStatus;//wangwenhua add 20110311

extern unsigned char   Calibration_Antenna ;
extern  UINT8     bGpsStatus_RF;
 int  WRRU_Temperature = 0; //此温度将报给AUX进行校准用
 unsigned char rru_status = 0;
 unsigned char  g_BBU_Fiber_INfo[256];

 unsigned short  g_RRU_Eprom = 0;
 unsigned short g_RRU_Freq_Offset = 0;
 unsigned short g_BBU_Fiber_Transid = 0;
 unsigned short g_RRU_Fiber_Transid = 0;
extern"C"  void ReadBBUFiberInfo(unsigned char fiber_no);
extern "C"  void ShowBBUFiberInfo(unsigned char flag);
 unsigned char g_print_rru_fiber_info = 0;
  unsigned char g_print_bbu_fiber_info = 0;
  unsigned char g_antenna_record_lase = 0xff;

  extern "C" void SetLocalFepRxInt(unsigned char flag);
  extern "C" void  Reset_ALL();
  extern "C" unsigned char  ReadFiberno();

  extern "C" unsigned char  RestFPGA();
  extern "C" void SetFpga_Para();
  extern "C" unsigned char   R_FPGA();

  extern "C" unsigned int  bspGetRRUChannelNum();
CWRRU::CWRRU()
{
    ::strcpy(m_szName, "tWRRU");
    m_uPriority = M_TP_L3RM;//M_TP_L3BM
    m_uOptions = 0;
    m_uStackSize = 52960;//20000;
    m_iMsgQMax = 1000;

      m_uOptions      = VX_FP_TASK;
 
    m_iMsgQOption   = ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY );

    pWRRULinkTimer=NULL;
    pWRRUCfgTimer =NULL;
    pWrruLoadCodeTimer = NULL;
    pWRRUCfgTimer_other = NULL;
    send_comm_alm_flag = 0;

}

void CWRRU::change_bytes(unsigned char *p)
{
        unsigned char change_array[112];
        int i;
        memcpy(change_array,p,112);
        for(i = 0; i < 56; i++)
        {
             p[2*i] = change_array[2*i+1];
             p[2*i+1] = change_array[2*i];
        }
}

CTimer* CWRRU :: SYS_Createtimer(UINT16 MsgId, UINT8 IsPeriod, UINT32 TimerPeriod)
{
    CL3OamCommonReq TimerMsg;
    if (false == TimerMsg.CreateMessage(*this))
        return NULL;
 //  printf("MsgId:%x\n",MsgId);
    TimerMsg.SetDstTid(M_TID_WRRU);
    TimerMsg.SetSrcTid(M_TID_WRRU);
    TimerMsg.SetMessageId(MsgId);
    return new CTimer(IsPeriod, TimerPeriod, TimerMsg);
}

TID CWRRU::GetEntityId() const
{
    return M_TID_WRRU;//CWRRU id;
}

CWRRU::~CWRRU ()
{
}


CWRRU* CWRRU::GetInstance()
{
    if ( instance == NULL )
    {
        instance = new CWRRU;
    }
    return instance;
}

bool CWRRU::Initialize()
{

    /***init******/
    if ( !CBizTask::Initialize() )
    {
        LOG(LOG_CRITICAL,0,"CWRRU Initialize failed.");
        return false;
    }

    m_uFlags |= M_TF_RUNNING;
    m_uFlags |= M_TF_INITED;
//    RegisterEntity(false);
    data_WRRU.WRRU_Num=0xff;
    data_WRRU.WRRU_State=IDLE;
    flag_rf_ver_get=0;
   memset(Calibration_HW_Data,0,1200);//初始化为0
    s_semWriteNvram = semBCreate(SEM_Q_FIFO, SEM_FULL);   // initial value of the semaphore
   pWRRULinkTimer_Report= SYS_Createtimer(M_BTS_WRRU_LINK_TIMER_Report, M_TIMER_PERIOD_YES, 3*60*1000);
   pWRRULinkTimer= SYS_Createtimer(M_BTS_WRRU_LINK_TIMER, M_TIMER_PERIOD_YES, BTS_WRRU_LINK_PERIOD);
   pWRRUCfgTimer = SYS_Createtimer(M_BTS_WRRU_CFG_TIMER,M_TIMER_PERIOD_NOT,BTS_WRRU_CFG_PERIOD);
   pWRRUCfgTimer_other = SYS_Createtimer(M_BTS_WRRU_CFG_TIMER_OTHER,M_TIMER_PERIOD_NOT,BTS_WRRU_CFG_PERIOD_OTHER);
   pWrruLoadCodeTimer = SYS_Createtimer(M_BTS_WRRU_LoadCode_TIMER, M_TIMER_PERIOD_NOT, BTS_WRRU_LOAD_CODE_PERIOD);
   pWBBURRU_Run_Timer= SYS_Createtimer(M_BBU_RRU_RUNNING, M_TIMER_PERIOD_YES/*M_TIMER_PERIOD_NOT*/, /*3*60*/30*1000);//wangwenhua add 20110809 
  pWBBURRU_REST_Timer = SYS_Createtimer(M_BTS_WRRU_RESET_TIMER, M_TIMER_PERIOD_NOT/*M_TIMER_PERIOD_NOT*/, /*3*60*/20*1000);
    if(NULL == pWRRULinkTimer) 
    {
        return false;
    }
   if(NULL==pWRRUCfgTimer)
   	{
   	    return false;
   	}
   if(pWRRULinkTimer_Report==NULL)
   	{
   	    return false;
   	}
   if(pWBBURRU_Run_Timer==NULL)
   	{
   	   return false;
   	}
   if(pWBBURRU_REST_Timer==NULL)
   	{
   	   return false;
   	}
      pWRRULinkTimer_Report->Start();
	pWBBURRU_Run_Timer->Start();
    link_packet_flag=0;
    link_flag=0;
    Link_Frame_No=0;
    	 m_cfg_times = 0;
	 m_antennaMask = 0x0;//默认全部打开
	 m_send_anntennaMask = 0;//0-没有送，1-发送
	 m_rru_state = 0;
	  send_temp_alm_flag = 0;
		 send_current_alm_flag = 0;
		 send_syn_alm_flag = 0;
		 send_Ad401_alm_flag = 0;
		 m_bitmap = 0;
		 m_cfg_times_other = 0;
       memset(g_BBU_Fiber_INfo,0,sizeof(g_BBU_Fiber_INfo));

     if(NvRamData->WRRUCfgEle.Max_Temp==0)//如果没有配治露让的话，将默认值写入NVRAM中
     	{
	     	char Max_temp,min_temp;
	     	Max_temp = 100;
	     	min_temp = 0;
	      l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.Max_Temp) ,&Max_temp,1);
	     l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.Min_Temp) ,&min_temp,1);
     	}
     if(NvRamData->WRRUCfgEle.Max_current==0)//如果没有配置电流的话，将默认值写入NVRAM中
     	{
	     	char Max_Curent,min_Current;
	     	Max_Curent = 10;
	     	min_Current = 0;
	     l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.Max_current) ,&Max_Curent,1);
	     l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.Min_current) ,&min_Current,1);
     	}
     if(NvRamData->fiber_para.initialized!=0x5555aaaa)
     	{
     	    /************************************
字段名称	长度（Byte）	类型	定义	描述
光模块电压告警门限	4	M	2000000uV	
光模块电流告警门限	4	M	8000uA	
光模块发射功率告警门限	4	M	300uW	
光模块接收功率告警门限	4	M	200uW	

************************************************************/
  	FiberInfoLimint temp_struct;
    	 temp_struct.initialized = 0x5555aaaa;
	  temp_struct.Voltage = 2000000;
	 temp_struct.Current = 8000;
	  temp_struct.Tx_Power = 150;
	  temp_struct.Rx_Power = 50;
	  	
	  l3rrubspNvRamWrite(( char *)&NvRamData->fiber_para,( char*)&temp_struct,sizeof(temp_struct));
       
     	}
    return true;
}

bool CWRRU::ProcessComMessage(CComMessage* pComMsg)
{
    OAM_LOGSTR1(LOG_DEBUG3, L3RRU_ERROR_REV_MSG, "[tWRRU] receive msg = 0x%04x", pComMsg->GetMessageId());
   UINT16 msgid = pComMsg->GetMessageId();
   unsigned char *ptr;
   unsigned short Fpga_alarm;
   int k = 0;
   static unsigned char m_30s_count =0;
    switch (pComMsg->GetMessageId())
    {
	case Type_L2_WRRU:		
		Process_WRRU_msg(pComMsg);
		m_rru_state = 1;
		break;
	case M_BTS_WRRU_LINK_TIMER://发送心跳定时器
		Process_Link_msg();
		pComMsg->Destroy();
		break;
	case M_BTS_WRRU_CFG_TIMER:
		  if(Loading_Wrru_Code==0)
             	{
             m_cfg_times++;
         
		RRU_CFG_AntennaMask(m_antennaMask);
		if(m_cfg_times==10)//连续三次没有收到的话，则不继续发送
		{
		     if(NULL==pWRRUCfgTimer)
		    	{
		    	}
			else
			{
			    pWRRUCfgTimer->Stop();
			}
			m_cfg_times = 0;
		}
		else
		{
			  RRU_Start_CFG_Timer();
		}
		}
		pComMsg->Destroy();
		break;
	case M_BTS_WRRU_CFG_TIMER_OTHER:
		    if(Loading_Wrru_Code==0)
             	{
		m_cfg_times_other++;
              RRU_ProceedRsvRRU(NULL ,Type_Query_WRRU_Calibration_HW,0x02);
              
              if(m_cfg_times_other==4)//连续三次没有收到的话，则不继续发送
		{
		     if(NULL==pWRRUCfgTimer)
		    	{
		    	}
			else
			{
			    pWRRUCfgTimer_other->Stop();
			}
			m_cfg_times_other = 0;
		}
		else
		{
			  RRU_Start_CFG_Timer_other();
		}
		    	}
		pComMsg->Destroy();

		break;
	 case  M_BTS_WRRU_LoadCode_TIMER:
	 	OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] M_BTS_WRRU_LoadCode_TIMER\n");

           if(Loading_Wrru_Code==1)//MCU
           	{
           	    for( k = 0; k < 2; k ++)
			{
			   fpga_wrru_code(&mcu_code[k*4][0]);
			    	    
		      	}  
           	}
             else if(Loading_Wrru_Code==2)//FPGA
             	{
             	        for( k = 0; k < 2; k ++)
			{
			   fpga_wrru_code(&fpga_code[k*4][0]);
			    	    
		      	}  
             	}
	 	
              pComMsg->Destroy();
	 	break;
	case M_OTHER2_WRRU:
	        if(Loading_Wrru_Code==0)
	       {
		ptr= (unsigned char *)pComMsg->GetDataPtr();
		// pWRRUCfgTimer->Stop();
		// m_antennaMask = ptr[0];
		
		m_cfg_times = 0;
		
		Fpga_alarm =  Read_Fpga_Alarm();
		if(ptr[0]==0)//如果是关闭的，就允许关闭
		{
		    RRU_CFG_AntennaMask(ptr[0]);
			RRU_Start_CFG_Timer();
			m_send_anntennaMask = 1;
			m_antennaMask = ptr[0]; 
			
		}
		else
		{
		if(ptr[1]==0)//如果手动配置的话，必须10ms是正常的
		{
		        if((NvRamData->L1GenCfgEle.SyncSrc)==1)
		        {
				if(bGpsStatus_RF==0/*((Fpga_alarm>>10)&0x1)||(ptr[0]==0)*/)//只有同步的情况才进行发送
				{
				   RRU_CFG_AntennaMask(ptr[0]);
				   RRU_Start_CFG_Timer();
				   m_send_anntennaMask = 1;
				    m_antennaMask = ptr[0];
				}
				else
				{
				  OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] Gps Not SYN,Can not Cfg To WRRU:m_antennaMask:%x\n",m_antennaMask);

				}
		      }
			else
			{
			     	  RRU_CFG_AntennaMask(ptr[0]);
				   RRU_Start_CFG_Timer();
				   m_send_anntennaMask = 1; 
				    m_antennaMask = ptr[0];
			}
		}
		else
		{
		         if((NvRamData->L1GenCfgEle.SyncSrc)==1)
		        {
	 				if(bGpsStatus_RF==0/*((Fpga_alarm>>10)&0x1)||(ptr[0]==0)*/)//只有同步的情况才进行发送
	 				{
	 				   RRU_CFG_AntennaMask(ptr[0]);
	 				   RRU_Start_CFG_Timer();
	 				   m_send_anntennaMask = 1;
	 				    m_antennaMask = ptr[0];
	 				}
	 				else
	 				{
	 				  OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU1] GPS Not Sync,Can not Cfg To WRRU:m_antennaMask:%x\n",m_antennaMask);
	 
	 				}
		      }
		       else
		       {
				   RRU_CFG_AntennaMask(ptr[0]);
				   RRU_Start_CFG_Timer();
				   m_send_anntennaMask = 1;
				    m_antennaMask = ptr[0];
			     }
		     }
		}
	     }
		pComMsg->Destroy();
		break;
	case M_BTS_WRRU_LINK_TIMER_Report:
		if(rru_status==1)
		{
	  AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_WRRU_recv_nopoll,
                       ALM_CLASS_CRITICAL,
                       STR_WRRU_L3_COMM_FAIL); 
		}
		else
		{
			    	  AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_WRRU_recv_nopoll,
                       ALM_CLASS_CRITICAL,
                       STR_WRRU_L3_COMM_FAIL); 
		}
		pComMsg->Destroy();
		break;
	case M_BBU_RRU_RUNNING://如果3分钟之内RRU还没有起来，则复位BBU
	        m_30s_count++;
			if(m_30s_count==3)//1.5 min 内复位wrru
			{
				    RRU_ProceedResetRRU(Type_Reset_WRRU, 0x02);
			}
	        if(m_30s_count==5)//2 min or 2.5 min 
	        {
	              OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] ReLoad BBU FPGA Due to BBU can not connect with RRU\n ");   
	             Reset_ALL();
	            
	        }
	        if(m_30s_count>=8)
	        {
	        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] Reboot BBU Due to BBU can not connect with RRU\n ");
               taskDelay(500);
    
    		bspSetBtsResetReason(RESET_REASON_RRU_NOWORKING);
    		rebootBTS(BOOT_CLEAR );
	        }
			pComMsg->Destroy();
         //wangwenhua add end 20110809
		break;
	case M_BTS_WRRU_RESET_TIMER://从收到RRU第一条消息起，如果20s内没有收到RRU的复位消息，则复位RRU
		RRU_ProceedResetRRU(Type_Reset_WRRU, 0x02);
		pComMsg->Destroy();
		break;
	default:
		Process_EMS_msg(pComMsg);
		break;
    }
    return true;
}
void set_rru(unsigned char flag)
{
   rru_status = flag;
}
unsigned char  g_rru_ad4001_status = 0;
unsigned int     g_rru_heartbeat_count =0;
void CWRRU::RRU_ProcessLinkMsg(CComMessage* pComMsg)
{
#if 0
    link_packet_flag=0x0;
    if (result_msg->transID==0x00)
		link_flag=0x00;//right
    else
		link_flag=0x01;//wrong,for alarm purpose
#endif
#if 0
DES    1
SRC  1
Type 1
Length 1
Result 2
FramNo_Expect 2
Temperature 2 //需要产生告警
Current 4  //需要产生告警
SynPllLoseCount 2  //需要产生告警
VSWRCount0 2
VSWRCount1 2
VSWRCount2 2
VSWRCount3 2
VSWRCount4 2
VSWRCount5 2
VSWRCount6 2
VSWRCount7  2
WrruState 2 
CRC
p[6] = b;//a=0xe0 b=0x24
	p[7] =a;//
	temp = (result_msg->DSB_HW_Temp&0x7fff)>>6; //先将高位置0，只取9bit
#endif
g_rru_heartbeat_count++;
    unsigned char *pdata = (unsigned char *)pComMsg->GetDataPtr();
    short temperature;
    int current;
    char temp1,temp2;
    unsigned short SynPllLoseCount;
   unsigned char flag_send = 0;
   unsigned short fpga_alarm = 0;
   unsigned short rru_state_flag = 0;
//温度处理
   g_RRU_Eprom = pdata[12]*0x100+pdata[13];
   g_RRU_Freq_Offset =  pdata[35]*0x100+pdata[34];
   	 if((g_RRU_Freq_Offset<0xa2E0)||(g_RRU_Freq_Offset>0xa320)) 
	{ 
     		 OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] g_RRU_Freq_Offset:%x\n ",g_RRU_Freq_Offset);   
	} 
   fpga_alarm =  pdata[81]*0x100+pdata[80];
    temp1 = pdata[8] ; //high byte
    temp2 = pdata[9] ;//low byte
    temperature = temp1*0x100+temp2;
    temperature = (temperature&0x7fff)>>6;
	if(fpga_alarm&0x4000)
	{
	      if(send_Ad401_alm_flag==0)
	      	{
	      	     send_Ad401_alm_flag= 1;
		    AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_RRU_AFC4001_ERR,
                                   ALM_CLASS_MINOR,
                                   STR_RRU_AFC4001_WARN);//暂时不关RF
	      	}
	      g_rru_ad4001_status = 1;
	      if((g_rru_heartbeat_count++)%12==0)
	      	{
	      	     OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] AFC 4001 Alarm\n ");   
	      	}
	}
	else
	{
	      if(send_Ad401_alm_flag==1)
	      	{
	      	     send_Ad401_alm_flag= 0;
		    AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_RRU_AFC4001_ERR,
                                   ALM_CLASS_MINOR,
                                   STR_RRU_AFC4001_WARN);//暂时不关RF
	      	}
	      g_rru_ad4001_status = 0;
	}
  rru_status =  1;
    if(temp1&0x80)//温度值为负
    	{
    	   temperature = (temperature-512);
    	}
	temperature = temperature/4;
	WRRU_Temperature = temperature;
	if((temperature> NvRamData->WRRUCfgEle.Max_Temp)||(temperature<NvRamData->WRRUCfgEle.Min_Temp))
	{
	       if(send_temp_alm_flag==0)
	       {
	            send_temp_alm_flag = 1;
			AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Temp,
                                   ALM_CLASS_MINOR,
                                   STR_WRRU_TEMP,temperature);//暂时不关RF
	       }
	}
	else
	{
		  if(send_temp_alm_flag==1)
	       {
	            send_temp_alm_flag = 0;
			AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Temp,
                                   ALM_CLASS_MINOR,
                                   STR_WRRU_TEMP,temperature);
	       }
	}
    
    //电流处理
   current = pdata[11]*0x100+pdata[10];
    current = (11*current*2)/1088; 
	//according to cuijianli suggest the current should multi 2;
	
   // printf("temp:%d curent:%d\n",temperature,current);
  
 

   if((current> NvRamData->WRRUCfgEle.Max_current)||(current<NvRamData->WRRUCfgEle.Min_current))
  {
	       if(send_current_alm_flag==0)
	       {
	            send_current_alm_flag = 1;
	  AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Current,
                                   ALM_CLASS_MINOR,
                                   STR_WRRU_CURRENT,current);//暂时不关RF
	       }
	}
	else
	{
		  if(send_current_alm_flag==1)
	       {
	            send_current_alm_flag = 0;
	  AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Current,
                                   ALM_CLASS_MINOR,
                                   STR_WRRU_CURRENT,current);
	       }
	}
//syn 
   SynPllLoseCount = pdata[15]*0x100+pdata[14];
  if(SynPllLoseCount>300)//产生syn lose 告警
  	{
  	    if(send_syn_alm_flag==0)
  	    	{
  	    	     send_syn_alm_flag = 1;
		     	AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_PLL_PLLLOSELOCK_SERIOUS,
                       ALM_CLASS_MAJOR,
                       STR_PLLLOSELOCK_SERIOUS); 
			   //应该关RF;
			   flag_send = 1;
			   RRU_CFG_RFMask(0xff,/*0xff*/0); //from 0xff to 0
  	    	}
  	}
      else //恢复告警
      	{
      	    if(send_syn_alm_flag)
      	    	{
      	    	     send_syn_alm_flag = 0;
			   AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_PLL_PLLLOSELOCK_SERIOUS,
                       ALM_CLASS_MAJOR,
                       STR_PLLLOSELOCK_SERIOUS); 
			   //应该开RF
			  if( bGpsStatus_RF==0)
			  	{
			   		RRU_CFG_RFMask(Calibration_Antenna/*0*/,0);//wangwenhua add 20110228
			  	}
      	    	}
      	}
   

   if(send_comm_alm_flag)
   	{
   	   OAM_LOGSTR2(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] RRU Comm Recover:%d,%d", link_packet_flag,send_comm_alm_flag);
   	  AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_WRRU_recv_nopoll,
                       ALM_CLASS_MAJOR,
                       STR_WRRU_L3_COMM_FAIL); 
		         send_comm_alm_flag = 0;
		       if(NULL != pWRRULinkTimer)
    			{
       		 pWRRULinkTimer->Start();
   		 	}
		       if(flag_send==0)
		       {    
		                if( bGpsStatus_RF==0)
		                	{
				       	RRU_CFG_RFMask(Calibration_Antenna/*0*/,0);//wangwenhua add 20110311
				       	flag_send = 1;
		                	}
		       }
		        // link_packet_flag = 0;
   	}
         if(flag_send==0)
	{
	                      if( bGpsStatus_RF==0)
	                      {
		       			RRU_CFG_RFMask(Calibration_Antenna/*0*/,1);//wangwenhua add 20110311
	                      	
		       			flag_send = 1;
	                      }
   	}
     link_packet_flag =0;
   data_WRRU.WRRU_State =RUNNING;
   rru_state_flag = pdata[47]*0x100+pdata[46];
   if((g_rru_heartbeat_count%36)==0)//每分钟增加打印状态
   {
   	 OAM_LOGSTR4(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] RRU Freq Offset:[%x],Antenna Mask:[%x] ,Temp:[%d],Current:[%d] ", g_RRU_Freq_Offset,m_antennaMask, temperature,current);
     OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] rru_state_flag:[%x]", rru_state_flag);
   }
    OAM_LOGSTR4(LOG_DEBUG, L3RRU_ERROR_REV_MSG, "[tWRRU] RRU Freq Offset:[%x],Antenna Mask:[%x] ,Temp:[%d],Current:[%d] ", g_RRU_Freq_Offset,m_antennaMask, temperature,current);
    OAM_LOGSTR1(LOG_DEBUG, L3RRU_ERROR_REV_MSG, "[tWRRU] rru_state_flag:[%x]", rru_state_flag);
}

void CWRRU::Process_Link_msg(void)
{
       	CComMessage *pMsg = NULL;
		pMsg = new (this, 112) CComMessage;
		if(pMsg==NULL)
		{
			 return;
		}
              pMsg->SetDstTid(M_TID_L2MAIN);
              pMsg->SetSrcTid(M_TID_WRRU);
   	       pMsg->SetMessageId(Type_L3_WRRU);
   	       pMsg->SetMoudlue(0);
	 //      memcpy(Byte * dest, Byte * src, uInt nBytes)
               T_TimeDate timeDate = bspGetDateTime();
		WRRU_message* p=(WRRU_message*)pMsg->GetDataPtr();
		p->src=BBU_No;
		p->des=data_WRRU.WRRU_Num;
		p->type=Type_Poll_Mcu;
		p->length=0x06;
         	//unsigned short *pFrame=(unsigned short*)((UINT8*)(pMsg->GetDataPtr())+sizeof(WRRU_message));
	      // *pFrame=Link_Frame_No;
	      unsigned char *pFrame = ((UINT8*)(pMsg->GetDataPtr())+sizeof(WRRU_message));
         	pFrame[0] = (unsigned char)Link_Frame_No;
         	pFrame[1] = Link_Frame_No/0x100;
             pFrame[2] = timeDate.month;//month
             pFrame[3] = timeDate.day;//day
             pFrame[4] = timeDate.hour;//hour
             pFrame[5] = timeDate.minute;//min
		
 		UINT16 *pCRC=(UINT16 *)((char *)(pMsg->GetDataPtr())+sizeof(WRRU_message)+6/*sizeof(unsigned short)*/);
        
 		unsigned short length=p->length/2+2;
 		*pCRC=CheckSum((unsigned short *) p,length);
               change_bytes((unsigned char*)pMsg->GetDataPtr());
		CComEntity::PostEntityMessage(pMsg);     
		Link_Frame_No++;
		link_packet_flag++;
		if(link_packet_flag==6)
		{
		      // RestFPGA();
			 R_FPGA();
			OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] BBU Not Receive RRU Heartbeat Over 30s ,RestFPGA");
		}
		if (link_packet_flag==12)
		{
		     
		  	if(send_comm_alm_flag==0)
		  	{
		   AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_WRRU_recv_nopoll,
                       ALM_CLASS_MAJOR,
                       STR_WRRU_L3_COMM_FAIL); 
		         send_comm_alm_flag = 1;
			  #if 0
		            if(NULL != pWRRULinkTimer)
		    		{
		       		 pWRRULinkTimer->Stop();
		   		 }
				#endif
				RRU_CFG_RFMask(0xff,0);//wangwenhua add  20110228  from 0xff to 0
				m_rru_state = 0;
				rru_status = 0;
		  	}
			Reset_ALL();
		  	data_WRRU.WRRU_State  = ALARMING;
			OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] BBU with RRU comm Error Over 1 mins Reset All Cpu Except L3");
		        // link_packet_flag = 0;
              }
		if(link_packet_flag==36)
		{
			 OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] Reboot L3Due to BBU With RRU Comm Error Over 3 mins \n ");
	               taskDelay(500);
	    
	    		bspSetBtsResetReason(RESET_REASON_RRU_NOWORKING);
	    		rebootBTS(BOOT_CLEAR );
		}
		
			//lost 3 packet then ,alarm

}


 void CWRRU:: RRU_CFG_AntennaMask(unsigned char anntenamask)
 {
//   	 int i = 0;
  //    unsigned char change_array[112];
 	// unsigned char *ptr;
    UINT16 *pCRC;
    unsigned short length;
   
   CComMessage *pComMsg = NULL;
   pComMsg = new (this, 112) CComMessage;
   if(pComMsg==NULL)
   	{
   	    return;
   	}
   pComMsg->SetDstTid(M_TID_L2MAIN);
  pComMsg->SetSrcTid(M_TID_WRRU);
   pComMsg->SetMoudlue(0);
   pComMsg->SetMessageId(Type_L3_WRRU);
   pComMsg->SetEID(0x12345678);
   WRRU_message*p=(WRRU_message *)pComMsg->GetDataPtr();
   p->des=0;
   p->src=0x10;
   p->length=0x06;
   p->type=0x14;
   unsigned char *pFrame = ((UINT8*)(pComMsg->GetDataPtr())+sizeof(WRRU_message));   	
	pFrame[0] = 0;//transid
	pFrame[1] = 0;
        pFrame[2] = 1;//BBU同步；
	pFrame[3] = 0;
	pFrame[4] = anntenamask;//
	 pFrame[5] = 1;// syn power open constly
	pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+6);
		        
	length=p->length/2+2;
	#if 0
	*pCRC=checksum((unsigned short *) p,length);
   	
       memcpy(change_array,(unsigned char*)pComMsg->GetDataPtr(),112);
     	   ptr= (unsigned char*)pComMsg->GetDataPtr();
        for(i = 0; i < 56; i++)
        {
             ptr[2*i] = change_array[2*i+1];
             ptr[2*i+1] = change_array[2*i];
        }
#endif
       *pCRC=CheckSum((unsigned short *) p,length);
       change_bytes((unsigned char*)pComMsg->GetDataPtr());
   	CComEntity::PostEntityMessage(pComMsg);
		OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] CFg antennaMask:%x\n", anntenamask);
      //根据天线开关进行告警
        UINT8 uctemp1,uctemp2;
	  UINT32 anntena_num= bspGetRRUChannelNum();
        if(g_antenna_record_lase!= anntenamask)
        {
            for(int antcount=0; antcount<anntena_num; antcount++)
            {
                uctemp1 = (g_antenna_record_lase&(1<<antcount))>>antcount;
                uctemp2 = (anntenamask&(1<<antcount))>>antcount;
                if(uctemp1!=uctemp2)
                {
                    if(uctemp2 == 0)//set alarm
                    {
                        AlarmReport(ALM_FLAG_SET,
                                       ALM_ENT_RF,
                                       antcount,
                                       ALM_ID_RF_RF_DISABLED,
                                       ALM_CLASS_CRITICAL,
                                       STR_RF_DISABLED, antcount + 1);
                    }
                    else//clear alarm
                    {
                        AlarmReport(ALM_FLAG_CLEAR,
                                       ALM_ENT_RF,
                                       antcount,
                                       ALM_ID_RF_RF_DISABLED,
                                       ALM_CLASS_CRITICAL,
                                       STR_CLEAR_ALARM);
                    }
                }
                
            }
            g_antenna_record_lase = anntenamask;
        }
        
 }
unsigned int a1 = 0;
unsigned int a2 =0;
unsigned int a3 =0;
unsigned int a4 =0;
unsigned int a5 =0;
void printwrru()
{
    printf("wr:%d,%d,%d,%d\n",a1,a2,a3,a4);
}
unsigned char rru_report_flag = 0;
void CWRRU::Process_WRRU_msg(CComMessage* pComMsg)
{
       change_bytes((unsigned char*)pComMsg->GetDataPtr());
      WRRU_message* message_WRRU=(WRRU_message*)((UINT8*)pComMsg->GetDataPtr());
      UINT16* msg=(UINT16*)((UINT8*)pComMsg->GetDataPtr()+sizeof(WRRU_message));
       int i = 0;
	   int j = 0;
	   char temp1;
	unsigned short count_num;
	unsigned short  k;
	 unsigned char *ptr;
	  UINT16 *pCRC;  	  
	  static unsigned char Crc_Alam_Flag =0;
      unsigned short length=message_WRRU->length/2+2;
      unsigned short check=CheckSum((unsigned short*)message_WRRU,length);
      UINT16* rru_check=(UINT16*)((UINT8*)pComMsg->GetDataPtr()+sizeof(WRRU_message)+message_WRRU->length);   
	  unsigned short  Fpga_alarm;
      if (check!=(*rru_check))   
      	{
         OAM_LOGSTR3(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] RRU CheckSum error:%x,%x,Fpga:%x\n", message_WRRU->type,check,Read_Fpga_Alarm()&0xfff);
		 pComMsg->Destroy();
	       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_RRU_CheckSum_ERR,
                       ALM_CLASS_INFO,
                       STR_BBU_RRU_MSG_WARN); 
	       Crc_Alam_Flag = 1;
                  return;
      	}
	else
	{
	            if(Crc_Alam_Flag==1)
	            	{
		       AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_RRU_CheckSum_ERR,
                       ALM_CLASS_MAJOR,
                       STR_BBU_RRU_MSG_WARN); 
	            	}
	            Crc_Alam_Flag = 0;
     }
      a4++;
	  link_packet_flag = 0;/****只要收到WRRU的消息包即可认为正常*****/
      switch (message_WRRU->type)
      {
	case Type_WRRU_No:
		SetLocalFepRxInt(1);//wangwenhua add 20111026
		m_bitmap = 0;
		data_WRRU.WRRU_Num=message_WRRU->src;//*msg;
		data_WRRU.WRRU_State=BOOTING;
		RRU_ProceedConfigRRUToRRU(pComMsg, Type_Config_WRRU, 0x2a);
		Loading_Wrru_Code = 0;
		m_bitmap =1;
		//pWRRULinkTimer->Stop();
		   if(NULL != pWRRULinkTimer)
    		{
    		     pWRRULinkTimer->Stop();
       		pWRRULinkTimer->Start();
   		 }
		  if(NULL!=pWrruLoadCodeTimer)
		 {
		      pWrruLoadCodeTimer->Stop();
		 }
		  if(pWBBURRU_REST_Timer)
		  {
		  	   pWBBURRU_REST_Timer->Stop();
		  	    pWBBURRU_REST_Timer->Start();
		  }
		  m_send_anntennaMask  = 0;
		 send_temp_alm_flag = 0;
		 send_current_alm_flag = 0;
		 send_syn_alm_flag = 0;
		 m_cfg_times_other = 0;
		 m_cfg_times = 0;
		   a1++;
		   a3 = 0;
		//pWRRULinkTimer->Start();
		pComMsg->Destroy();
	 	break;
		
	case Type_Config_WRRU_Ack:
		m_bitmap = 0;
		if (*msg==fail)
		{
			RRU_ProceedConfigRRUToRRU(pComMsg, Type_Config_WRRU, 0x2a);
			m_bitmap =1;
		}
		else
		{
		       RRU_ProceedRsvRRU(pComMsg,Type_Query_WRRU_Calibration_HW,0x02);
		       RRU_Start_CFG_Timer_other();//启动定时器
		       m_bitmap = 2;
		       
		}
              pComMsg->Destroy();
		a2++;
		break;
		
	case Type_Query_WRRU_Calibration_HW_Ack:
		  if(NULL==pWRRUCfgTimer_other)
		 {
		 }
	       else
		{
			    pWRRUCfgTimer_other->Stop();
		}
		m_bitmap = 0;
              RRU_ProcessCaliHW(pComMsg);
              m_bitmap = 4;
               pComMsg->Destroy();
              a3++;
		break;

	case Alarm_WRRU_Reset:
		m_bitmap = 0;
		Loading_Wrru_Code = 0;
		 UINT16 mask ;
		data_WRRU.WRRU_State=RUNNING;
              if(pWBBURRU_REST_Timer)
		  {
		  	    pWBBURRU_REST_Timer->Stop();
		  }
	
		if(a3>0)//表示得到了硬件参数
		{
		    if(pWBBURRU_Run_Timer)
			{
		     pWBBURRU_Run_Timer->Stop();
			}
			RRU_SendCaliHWToAUX(0);
		}
		OAM_LOGSTR(LOG_SEVERE, L3RRU_ERROR_REV_MSG, "[tWRRU] WRRU_Reset Msg\n");
		rru_status = 1;
		      AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Reset,
                                   ALM_CLASS_MINOR,
                                   STR_WRRU_RESET);
			  rru_report_flag = 1;
		//此时如果同步的话，则应该立即发送打开RF的开关?
		 if((NvRamData->L1GenCfgEle.SyncSrc)==1)//只有在GPS的情况下才进行检测
		 {
		 Fpga_alarm =  Read_Fpga_Alarm();
		
			if(bGpsStatus_RF==0/*((Fpga_alarm>>10)&0x1)&&(bGpsStatus==true)*/)//只有同步的情况才进行发送并且GPS同步的情况下才进行配置
			{
					 mask = NvRamData->L1GenCfgEle.AntennaMask;
					mask = (mask)&(~Calibration_Antenna);
				   RRU_CFG_AntennaMask((unsigned char )mask);
				   RRU_Start_CFG_Timer();
				   m_send_anntennaMask = 1;
				   m_antennaMask = mask;
			}
			else
			{
			      	  RRU_CFG_AntennaMask(0);
				   RRU_Start_CFG_Timer();
				   m_send_anntennaMask = 1;
				   m_antennaMask = 0;
			}
		}
		 else
		 {
		 	          mask = NvRamData->L1GenCfgEle.AntennaMask;
				mask = (mask)&(~Calibration_Antenna);
			   RRU_CFG_AntennaMask((unsigned char )mask);
			   RRU_Start_CFG_Timer();
			   m_send_anntennaMask = 1;
			    m_antennaMask = mask;
		 }
		 pComMsg->Destroy();
		break;
  


	case Type_Query_WRRU_Ver_Ack://ok查询版本
		RRU_RRUVERToEMS((T_WRRU_Ver*)(msg),M_BTS_EMS_RRU_VER_GET_RSP);
		pComMsg->Destroy();
		break;
	case Type_Query_WRRU_SPF_Ver_Ack://光模块的版本号已经调试完毕
		RRU_RRUSPFToEMS((T_SPF_Ver_Msg *)(msg),M_BTS_EMS_RRU_SPF_GET_RSP);
		pComMsg->Destroy();
		break;
	case Type_Query_WRRU_RF1to4_Ver_Ack:
		if (flag_rf_ver_get==0)
		{
			flag_rf_ver_get=1;
			#if 1
			memcpy(rf_ver,(unsigned short *)((UINT8*)msg+sizeof(unsigned short)), sizeof(rf_ver));
			for(i = 0;i<44;i++)
		      {
				if((i!=0)&&(i!=11)&&(i!=22)&&(i!=33))
				{
					rf_ver[i] = (rf_ver[i]>>8)|(rf_ver[i]<<8);
				}
		       }//added by chenxi 100927
		       #endif

		}
		
		else 
		{
			//RRU_RRURFVerToEMS(1,msg,M_BTS_EMS_RRU_RF_VER_GET_RSP);
			memcpy(rf_ver,(unsigned short *)((UINT8*)msg+sizeof(unsigned short)), sizeof(rf_ver));
			for(i = 0;i<44;i++)
		      {
				if((i!=0)&&(i!=11)&&(i!=22)&&(i!=33))
				{
					rf_ver[i] = (rf_ver[i]>>8)|(rf_ver[i]<<8);
				}
		       }//added by chenxi 100927
		       RRU_RRURFVerToEMS(1,msg,M_BTS_EMS_RRU_RF_VER_GET_RSP);
		
		}
	
	//	RRU_RRURFVerToEMS(1,msg,M_BTS_EMS_RRU_RF_VER_GET_RSP);
		pComMsg->Destroy();

		break;

	case Type_Query_WRRU_RF5to8_Ver_Ack:
		if (flag_rf_ver_get==0)
		{
			flag_rf_ver_get=1;
			memcpy(rf_ver2,(unsigned short *)((UINT8*)msg+sizeof(unsigned short)), sizeof(rf_ver2));
			for(i = 0;i<44;i++)
		      {
				if((i!=0)&&(i!=11)&&(i!=22)&&(i!=33))
				{
					rf_ver2[i] = (rf_ver2[i]>>8)|(rf_ver2[i]<<8);
				}
		       }//added by chenxi 100927
		       //在重新发一次
		       RRU_ProceedQueryVer(NULL,Type_Query_WRRU_RF1to4_Version,2);
                 
		}
		else 
		{
		     	memcpy(rf_ver2,(unsigned short *)((UINT8*)msg+sizeof(unsigned short)), sizeof(rf_ver2));
			for(i = 0;i<44;i++)
		      {
				if((i!=0)&&(i!=11)&&(i!=22)&&(i!=33))
				{
					rf_ver2[i] = (rf_ver2[i]>>8)|(rf_ver2[i]<<8);
				}
		       }//added by chenxi 100927
			RRU_RRURFVerToEMS(5,msg,M_BTS_EMS_RRU_RF_VER_GET_RSP);
		
		}
		//RRU_RRURFVerToEMS(5,msg,M_BTS_EMS_RRU_RF_VER_GET_RSP);
		pComMsg->Destroy();
		break;
    	case Type_WRRU_TEmp_Current://送给ems
    	      RRU_HwStatusToEMS(pComMsg/*(T_WRRU_HW_STATUS*)(msg)*/,M_BTS_EMS_RRU_HW_STATUS_GET_RSP);
		pComMsg->Destroy();
		break;
      case  Type_WRRU_SYN_DSB_Ver:
	  	memcpy(DSB_SYN_ver,(unsigned char *)((UINT8*)msg+sizeof(unsigned short)), sizeof(DSB_SYN_ver));
		//memcpy(SYN_Ver,(unsigned short *)((UINT8*)msg+sizeof(unsigned short)), sizeof(rf_ver));
            for(j = 0; j< 22;j++)// to avoid the oxff  
            	{
            	    if(DSB_SYN_ver[j]== 0xff)
            	    	{
            	    	    DSB_SYN_ver[j] = 0;
            	    	}
            	}
		temp1 = DSB_SYN_ver[22];
		DSB_SYN_ver[22] = DSB_SYN_ver[23];
		DSB_SYN_ver[23] = temp1;//added by chenxi 100927
	  	pComMsg->Destroy();
	  	break;
      case Type_Fpga_Register: //no cpoe 
               ptr = (unsigned char *)pComMsg->GetDataPtr();
               if(ptr[4]==1)
               {
               		OAM_LOGSTR3(LOG_SEVERE, L3RRU_ERROR_REV_MSG, "[tWRRU] WRRU_FPGA Read addr:0x%x,value:0x%x(%d)\n",ptr[5],  ptr[7]*0x100+ptr[6],ptr[7]*0x100+ptr[6]);
               }
               else
               {
               	     OAM_LOGSTR3(LOG_SEVERE, L3RRU_ERROR_REV_MSG, "[tWRRU] WRRU_FPGA Write addr:0x%x,value:0x%x(%d)\n",ptr[5],  ptr[7]*0x100+ptr[6],ptr[7]*0x100+ptr[6]);
               }
	  	pComMsg->Destroy();
	  	break;
		
////////EMS msg response
	case Type_Config_WRRU_CircumsPara_Ack:
		RRU_RRUResToEMS((T_RSV_Msg *)(msg),M_BTS_EMS_RRU_TEMP_CFG_RSP);
		pComMsg->Destroy();
		break;
	case Type_Config_WRRU_General_Ack:
	//	RRU_RRUResToEMS((T_RSV_Msg *)(msg),M_BTS_EMS_RRU_SYN_SWITCH_CFG_RSP);
	      	if(NULL==pWRRUCfgTimer)
		 {
		 }
	       else
		{
			    pWRRUCfgTimer->Stop();
		}
		pComMsg->Destroy();
		break;

	//response in other task
	case Type_Config_WRRU_PLL_Ack:
              pComMsg->Destroy();		
		break;
	case Type_Config_WRRU_Timeslot_Ack:
		pComMsg->Destroy();
		break;
	case Type_Config_WRRU_CalibrationData_Ack:
		pComMsg->Destroy();
		break;

	case Type_MCUCode_End_Ack:
	  if(!file_mcu)
	    {
	    	    fclose(file_mcu);
	    }
	  pComMsg->Destroy();
 	 if(NULL!=pWrruLoadCodeTimer)
      	{
            	 pWrruLoadCodeTimer->Stop();
        }
	  OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU]Type_MCUCode_End_Ack\n");
	  //发送EMS 代码加载成功的消息
				break;
	case Type_MCUCode_Download_Ack:  //下载代码8包
	//pWRRULinkTimer->Stop();
		   if(NULL != pWRRULinkTimer)
    		{
       		pWRRULinkTimer->Stop();
   		 }
		 if(NULL!=pWrruLoadCodeTimer)
     		{
           	 pWrruLoadCodeTimer->Stop();
   		 }
	    if(!file_mcu)
	    {
	    	    fclose(file_mcu);
	    }
	    file_mcu = fopen("/RAMD:0/load/wrru_mcu","rb");
	    if(file_mcu==NULL)
	    	{
	    	        pComMsg->Destroy();
	    	        OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] open wrru_mcu file fail\n");
	    	    	    break;
	    	 }
	  Loading_Wrru_Code = 1;
	          OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] Loading_Wrru_Code :MCU:%x\n",Loading_Wrru_Code);
	    	m_last_mcu = 0;
	    	mcu_end_flag = 0;
	    	memset(&mcu_code[0][0],0xff,8*112);
      for( k = 0; k < 2; k ++)
      	{
			    	for( i = 0; i< 4; i++)
			    		{
		   
		       
							 count_num = fread(&mcu_code[k*4+i][6], 1, 102, file_mcu);  
				
							  mcu_code[k*4+i][0] = data_WRRU.WRRU_Num;  /**dest*/
							  mcu_code[k*4+i][1] = BBU_No;/*src*/
							  mcu_code[k*4+i][2] = Type_MCUCode;/**type*/
							  mcu_code[k*4+i][3] = 104;/*len*/
							  mcu_code[k*4+i][4] = (unsigned char)(m_last_mcu+k*4+i);/**seq*/
							  mcu_code[k*4+i][5] = (m_last_mcu+k*4+i)/0x100;/**seq*/
							 // mcu_code[k*4+i][110] = 0;
							//  mcu_code[k*4+i][111] =0;/**crc**/
							   pCRC=(UINT16 *)&(mcu_code[k*4+i][108]);//((char *)(pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
        
 							//unsigned short length=55;//p->length/2+2;
 							*pCRC=CheckSum((unsigned short *)mcu_code[k*4+i] ,54);
							change_bytes( &mcu_code[k*4+i][0])	  ;   
					/*		printf("wrru code:%d\n",k*4+i);
							for(int l =0; l <112;l++)
								{
								    printf("%02x,",mcu_code[k*4+i][l]);
								}
							printf("/n");
						*/	
							  
			    		}
	      fpga_wrru_code((unsigned char *)&mcu_code[k*4][0]);
	      
	    	    
      	}
      m_last_mcu+= 8;
       if(NULL!=pWrruLoadCodeTimer)
       {
            pWrruLoadCodeTimer->Start();
       }
      pComMsg->Destroy();
		break;
	case Type_MCUCode_Ack:
     ptr = (unsigned char *)pComMsg->GetDataPtr();
        if(NULL!=pWrruLoadCodeTimer)
       {
            pWrruLoadCodeTimer->Stop();
       }
      if(ptr[4] ==0)/**成功***/
      	{
      	      if(mcu_end_flag==1)//发送END标志给WRRU
      	      	{
      	      	    	pComMsg->Destroy();
      	      	    	//  OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] the last code end\n");
      	      	    	
		      	       CComMessage *pMsg = NULL;
				pMsg = new (this, 112) CComMessage;
				if(pMsg==NULL)
				{
				     break;
				}
		                pMsg->SetDstTid(M_TID_L2MAIN);
		                pMsg->SetSrcTid(M_TID_WRRU);
		   	           pMsg->SetMessageId(Type_L3_WRRU);
		   	            pMsg->SetMoudlue(0);
		   	            char *pmucend =(char*)pMsg->GetDataPtr();


					WRRU_message* p=(WRRU_message*)((unsigned char*)pMsg->GetDataPtr());
					p->src=BBU_No;
					p->des=data_WRRU.WRRU_Num;
					p->type=Type_MCUCode_End;
					p->length=2;
					pmucend[4]  = 0;
					pmucend[5] = 0;
				        pCRC=(UINT16 *)(&(pmucend[6]));
 					*pCRC=CheckSum((unsigned short *)pmucend ,3);
		                  change_bytes((unsigned char*)pMsg->GetDataPtr());
				   CComEntity::PostEntityMessage(pMsg);     
			 OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] mcu the last code end \n");		
		   	 //printf("mcu the last code end \n");
		      	       
		    
			//printf("the last code end \n");
			return;
      	      	}
      	  			memset(&mcu_code[0][0],0xff,8*112);
		      			for( k = 0; k < 2; k ++)
				      	{
							    	for( i = 0; i< 4; i++)
							    		{
						   
						            if(file_mcu==NULL)
						            	{
						            	      pComMsg->Destroy();
						            	    //  printf("open wrru_mcu file fail file_mcu is null\n");
						            	      OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] open wrru_mcu file fail file_mcu is null\n");
						                  	return;
						            	}
													count_num = fread(&mcu_code[k*4+i][6], 1, 102, file_mcu);  
													if(count_num ==0)
														{
														OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] count_num:%d\n",count_num);
														    mcu_end_flag = 1;
														}
								   
								      	
														  mcu_code[k*4+i][0] = data_WRRU.WRRU_Num;  /**dest*/
														  mcu_code[k*4+i][1] = BBU_No;/*src*/
														  mcu_code[k*4+i][2] = Type_MCUCode;/**type*/
														  mcu_code[k*4+i][3] = 104;/*len*/
														  mcu_code[k*4+i][4] = (unsigned char)(m_last_mcu+k*4+i);/**seq*/
														  mcu_code[k*4+i][5] = (m_last_mcu+k*4+i)/0x100;/**seq*/
														  //mcu_code[k*4+i][110] = 0;
														  //mcu_code[k*4+i][111] =0;/**crc**/
														   pCRC=(UINT16 *)&(mcu_code[k*4+i][108]);//((char *)(pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
        
 							//unsigned short length=55;//p->length/2+2;
 														*pCRC=CheckSum((unsigned short *)mcu_code[k*4+i] ,54);
													  change_bytes( &mcu_code[k*4+i][0])	  ; 
								      		     
							    		}
			    	  fpga_wrru_code(&mcu_code[k*4][0]);
			    	    
		      	}   
		      	    if(NULL!=pWrruLoadCodeTimer)
     			  {
           			 pWrruLoadCodeTimer->Start();
       		  }
		      	m_last_mcu+=8;
      	}
      else
      	{

      	   OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] mcu_code_ack error:%d\n",m_last_mcu);
      	   		  for( k = 0; k < 2; k ++)
				      	{
				      	  #if 0
							    	for( i = 0; i< 4; i++)
							    		{
	
														  mcu_code[k][0] = data_WRRU.WRRU_Num;  /**dest*/
														  mcu_code[k][1] = BBU_No;/*src*/
														  mcu_code[k][2] = Type_MCUCode;/**type*/
														  mcu_code[k][3] = 106;/*len*/
														  mcu_code[k][4] = 0;/**seq*/
														  
														  mcu_code[k][5] = (unsigned char)(k*4+i);/**seq*/
														  mcu_code[k][110] = 0;
														  mcu_code[k][111] =0;/**crc**/
								    	     
							    		}
							    	#endif
			    	        fpga_wrru_code(&mcu_code[k*4][0]);
			    	    
		      	}   
      	   		   if(NULL!=pWrruLoadCodeTimer)
     			  {
           			 pWrruLoadCodeTimer->Start();
       		  }
      	    
      	}
      #if 0
      if(mcu_end_flag==1)//发送END标志给WRRU
      	{
      	       CComMessage *pMsg = NULL;
		        pMsg = new (this, 112) CComMessage;
                pMsg->SetDstTid(M_TID_L2MAIN);
                pMsg->SetSrcTid(M_TID_WRRU);
   	           pMsg->SetMessageId(Type_L3_WRRU);
   	            pMsg->SetMoudlue(0);


				WRRU_message* p=(WRRU_message*)((unsigned char*)pMsg->GetDataPtr());
				p->src=BBU_No;
				p->des=data_WRRU.WRRU_Num;
				p->type=Type_MCUCode_End;
				p->length=4;
		
         change_bytes((unsigned char*)pMsg->GetDataPtr());
		   CComEntity::PostEntityMessage(pMsg);     
	 OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] mcu the last code end \n");		
   	 //printf("mcu the last code end \n");
      	       
    }
      #endif
		  pComMsg->Destroy();
				break;


  case Type_FPGACode_Download_Ack:
	    if(!file_fpga)
	    {
	    	    fclose(file_fpga);
	    }
	       if(NULL != pWRRULinkTimer)
    		{
       		pWRRULinkTimer->Stop();
   		 }
    	    if(NULL!=pWrruLoadCodeTimer)
	      {
   			 pWrruLoadCodeTimer->Stop();
		  }
	    file_fpga = fopen("/RAMD:0/load/wrru_fpga","rb");
	    if(file_fpga==NULL)
	    	{
	    	         OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] open wrru_fpga file fail\n");
	    	        pComMsg->Destroy();
	    	    	    break;
	    	 }
	   
	  Loading_Wrru_Code = 2;
	   OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] Loading_Wrru_Code :FPGA :%x\n",Loading_Wrru_Code);
	    	m_last_fpga= 0;
	    	fpga_end_flag = 0;
	    	memset(&fpga_code[0][0],0xff,8*112);
      for( k = 0; k < 2; k ++)
      	{
			    	for( i = 0; i< 4; i++)
			    		{
		   
		       
							 count_num = fread(&fpga_code[k*4+i][6], 1, 102, file_fpga);  
				
							  fpga_code[k*4+i][0] = data_WRRU.WRRU_Num;  /**dest*/
							  fpga_code[k*4+i][1] = BBU_No;/*src*/
							  fpga_code[k*4+i][2] = Type_FPGACode;/**type*/
							  fpga_code[k*4+i][3] = 104;/*len*/
							  fpga_code[k*4+i][4] = (unsigned char)(m_last_fpga+k*4+i);/**seq*/
							  fpga_code[k*4+i][5] = (m_last_fpga+k*4+i)/0x100;/**seq*/
							  //fpga_code[k*4+i][110] = 0;
							  //fpga_code[k*4+i][111] =0;/**crc**/
							pCRC=(UINT16 *)&(fpga_code[k*4+i][108]);//((char *)(pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
        
 							//unsigned short length=55;//p->length/2+2;
 							*pCRC=CheckSum((unsigned short *)fpga_code[k*4+i] ,54);
							  change_bytes( &fpga_code[k*4+i][0])	  ;   
			    		}
	    	  fpga_wrru_code(&fpga_code[k*4][0]);
	    	    
      	}
      m_last_fpga+= 8;
	    if(NULL!=pWrruLoadCodeTimer)
	{
   			 pWrruLoadCodeTimer->Start();
	  }
      pComMsg->Destroy();
	     break;
  case  Type_FPGACode_Ack:

  	ptr = (unsigned char *)pComMsg->GetDataPtr();
        if(NULL!=pWrruLoadCodeTimer)
     	{
           	pWrruLoadCodeTimer->Stop();
       }
      if(ptr[4] ==0)/**成功***/
      	{

      	         if(fpga_end_flag==1)//发送END标志给WRRU
      	      	{
      	      	    		pComMsg->Destroy();
      	      	    		CComMessage *pMsg = NULL;
		              pMsg = new (this, 112) CComMessage;
				if(pMsg==NULL)
					{
					     break;
					}
               		 pMsg->SetDstTid(M_TID_L2MAIN);
                		pMsg->SetSrcTid(M_TID_WRRU);
   	           		pMsg->SetMessageId(Type_L3_WRRU);
   	            		pMsg->SetMoudlue(0);
				char *pfpgaend =(char*)pMsg->GetDataPtr();

				WRRU_message* p=(WRRU_message*)((unsigned char*)pMsg->GetDataPtr());
				p->src=BBU_No;
				p->des=data_WRRU.WRRU_Num;
				p->type=Type_FPGACode_End;
				p->length=2;
				pfpgaend[4]  = 0;//transid = 0;
				pfpgaend[5] = 0;
				pCRC=(UINT16 *)(&(pfpgaend[6]));
 				*pCRC=CheckSum((unsigned short *)pfpgaend ,3);
		
        		      change_bytes((unsigned char*)pMsg->GetDataPtr());
		  		 CComEntity::PostEntityMessage(pMsg);     
				// printf("fpga the last code end \n");
				//printf("fpga the last code end \n");
				OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] fpga the last code end\n");
				 break;
      	      	}
      	                //   OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] Type_FPGACode_Ack\n");
      	  			memset(&fpga_code[0][0],0xff,8*112);
		      			for( k = 0; k < 2; k ++)
				      	{
							    	for( i = 0; i< 4; i++)
							    		{
						   
						            if(file_fpga==NULL)
						            	{
						            	    OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] open wrru_fpga file fail file_fpga is null\n");
						            	      pComMsg->Destroy();
						                  	return;
						            	}
													count_num = fread(&fpga_code[k*4+i][6], 1, 102, file_fpga);  
													if(count_num ==0)
														{
														   OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] FPGA count_num:%d\n",count_num);
														    fpga_end_flag = 1;
														}
								   
								      	
														  fpga_code[k*4+i][0] = data_WRRU.WRRU_Num;  /**dest*/
														  fpga_code[k*4+i][1] = BBU_No;/*src*/
														  fpga_code[k*4+i][2] = Type_FPGACode;/**type*/
														  fpga_code[k*4+i][3] = 104;/*len*/
														  fpga_code[k*4+i][4] = (unsigned char)(m_last_fpga+k*4+i);/**seq*/
														  fpga_code[k*4+i][5] = (m_last_fpga+k*4+i)/0x100;/**seq*/
														 // fpga_code[k*4+i][110] = 0;
														 // fpga_code[k*4+i][111] =0;/**crc**/
														 pCRC=(UINT16 *)&(fpga_code[k*4+i][108]);//((char *)(pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
        
 							//unsigned short length=55;//p->length/2+2;
 														*pCRC=CheckSum((unsigned short *)fpga_code[k*4+i] ,54);
														  change_bytes( &fpga_code[k*4+i][0])	  ; 
								      		     
							    		}
			    	  fpga_wrru_code(&fpga_code[k*4][0]);
			    	    
		      	}   
		      	m_last_fpga+=8;
		      	 if(NULL!=pWrruLoadCodeTimer)
     			  {
           			 pWrruLoadCodeTimer->Start();
       		  }
      	}
      else
      	{
      	     OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU] fpga_code_ack error:%d\n",m_last_fpga);
      	   		  for( k = 0; k < 2; k ++)
				      	{
				      	#if 0
							    	for( i = 0; i< 4; i++)
							    		{
	
														  fpga_code[k][0] = data_WRRU.WRRU_Num;  /**dest*/
														  fpga_code[k][1] = BBU_No;/*src*/
														  fpga_code[k][2] = Type_FPGACode;/**type*/
														  fpga_code[k][3] = 106;/*len*/
														  fpga_code[k][4] = 0;/**seq*/
														  
														  fpga_code[k][5] = (unsigned char)(k*4+i);/**seq*/
														  fpga_code[k][110] = 0;
														  fpga_code[k][111] =0;/**crc**/
								    	     
							    		}
							    	#endif
			    	        fpga_wrru_code(&fpga_code[k*4][0]);
			    	    
		      	}   
      	   		 if(NULL!=pWrruLoadCodeTimer)
     			  {
           			 pWrruLoadCodeTimer->Start();
       		  }
      	    
      	}
      #if 0
      if(fpga_end_flag==1)//发送END标志给WRRU
      	{
      	       CComMessage *pMsg = NULL;
		        pMsg = new (this, 112) CComMessage;
                pMsg->SetDstTid(M_TID_L2MAIN);
                pMsg->SetSrcTid(M_TID_WRRU);
   	           pMsg->SetMessageId(Type_L3_WRRU);
   	            pMsg->SetMoudlue(0);


				WRRU_message* p=(WRRU_message*)((unsigned char*)pMsg->GetDataPtr());
				p->src=BBU_No;
				p->des=data_WRRU.WRRU_Num;
				p->type=Type_FPGACode_End;
				p->length=4;
		
        change_bytes((unsigned char*)pMsg->GetDataPtr());
		   CComEntity::PostEntityMessage(pMsg);     
				 printf("fpga the last code end \n");
  
      	       
    }
      #endif
		  pComMsg->Destroy();
	    break;

   case Type_FPGACode_End_Ack:
   		if(!file_fpga)
	    {
	    	    fclose(file_fpga);
	    }
   	 if(NULL!=pWrruLoadCodeTimer)
     	{
           	 pWrruLoadCodeTimer->Stop();
       }
   		pComMsg->Destroy();
   		OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU]Type_FPGACode_End_Ack\n");
				break;
   case Type_Poll_Mcu_Ack:
	RRU_ProcessLinkMsg(pComMsg);
	pComMsg->Destroy();
	break;
  case  Alarm_WRRU_SYNPwOff:
  	#if 0
  		ptr = (unsigned char *)pComMsg->GetDataPtr();
		if(ptr[4]==1)//
		{
		AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_PLL_FACTORY_DATA,
                                   ALM_CLASS_CRITICAL,
                                   STR_WRRU_SYN_ERROR,ptr[5]);
		}
		else if(ptr[4]==0)
		{
		
		}
	#endif
  	pComMsg->Destroy();
  	break;
case  Alarm_WRRU_Temp :
	#if 0 //这个告警由L3层根据心跳消息产生
	ptr = (unsigned char *)pComMsg->GetDataPtr();
	if(ptr[4]==0)//recover
	{
	     AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Temp,
                                   ALM_CLASS_CRITICAL,
                                   STR_WRRU_TEMP,ptr[5]);
	}
	else
	{
		       AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Temp,
                                   ALM_CLASS_CRITICAL,
                                   STR_WRRU_TEMP,ptr[5]);
	}
	#endif
	pComMsg->Destroy();
	break;
case  Alarm_WRRU_Current :
	ptr = (unsigned char *)pComMsg->GetDataPtr();
	#if 0 //这个告警由L3层根据心跳消息产生
	if(ptr[4] == 0)//recover
	{
	  AlarmReport(ALM_FLAG_CLEAR,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Current,
                                   ALM_CLASS_CRITICAL,
                                   STR_WRRU_CURRENT,ptr[6]);
	}
	else
	{
		  	  AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   Alarm_ID_WRRU_Current,
                                   ALM_CLASS_CRITICAL,
                                   STR_WRRU_CURRENT,ptr[6]);
	}
	#endif
	pComMsg->Destroy();
	break;
case  Alarm_WRRU_TRB_VSWR :
	ptr = (unsigned char *)pComMsg->GetDataPtr();
	#if 0//do not report to ems,only for R&D
	 AlarmReport(ALM_FLAG_SET,
                                   ALM_ENT_WRRU,
                                   ALM_ENT_INDEX0,
                                   ALM_ID_RF_BOARD_ZBB_SERIOUS,
                                   ALM_CLASS_CRITICAL,
                                   STR_WRRU_ZBB,ptr[5]);
	 #endif
	//  OAM_LOGSTR1(LOG_SEVERE, L3CM_ERROR_REV_MSG, "[tWRRU]Alarm_WRRU_TRB_VSWR:%x\n",ptr[5]);
	/*  WrruRFC(ptr[5]);*/
	#ifdef NO_CLOSE_RF
	RRU_CFG_RFMask(ptr[5]);//close RF channel 
	#endif  
	//RRU_CFG_RFMask(ptr[5]);//close RF channel
	pComMsg->Destroy();
	break;
case  Alarm_WRRU_10ms :
	pComMsg->Destroy();
	break;
//case Alarm_WRRU_LightLink 0x47/*no use**/
//case  Alarm_WRRU_FNWrong 0x48/*no use**/
case  Alarm_WRRU_recv_nopoll:
	ptr = (unsigned char *)pComMsg->GetDataPtr();
	if(ptr[4]==0)//recover
	{
	   AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_WRRU_recv_nopoll,
                       ALM_CLASS_MAJOR,
                       STR_WRRU_L3_COMM_FAIL); 
	}
	else
	{
		       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_WRRU_recv_nopoll,
                       ALM_CLASS_MAJOR,
                       STR_WRRU_L3_COMM_FAIL); 
	}
	pComMsg->Destroy();
	break;
case  Alarm_Sync_PLL_Lost:
	#if 0 ////这个告警由L3层根据心跳消息产生
	ptr = (unsigned char *)pComMsg->GetDataPtr();
	static unsigned char pll_times = 0;
	if(ptr[4] ==0)//recover
	{
	pll_times = 0;
	   AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_PLL_PLLLOSELOCK_SERIOUS,
                       ALM_CLASS_MAJOR,
                       STR_PLLLOSELOCK_SERIOUS); 
	}
	else
	{
		     AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_PLL_PLLLOSELOCK_SERIOUS,
                       ALM_CLASS_MAJOR,
                       STR_PLLLOSELOCK_SERIOUS); 
			 pll_times++;
			 if(pll_times==3)
			 {
	//		    ResetWrru();//reset wrru
			 }
	}
	#endif
	pComMsg->Destroy();
	break;
case  Alarm_WRRU_Download_MCU:
	   AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_WRRU_Download_MCU,
                       ALM_CLASS_MAJOR,
                       STR_MCU_DOWNLOAD_WARN); 
	   Loading_Wrru_Code = 0;
	 if(NULL!=pWrruLoadCodeTimer)
     	{
           	pWrruLoadCodeTimer->Stop();
       }
	pComMsg->Destroy();
	break;
case  Alarm_WRRU_Download_FPGA:
	   AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       Alarm_ID_WRRU_Download_FPGA,
                       ALM_CLASS_MAJOR,
                       STR_FPGA_DOWNLOAD_WARN); 
	   Loading_Wrru_Code = 0;
	  if(NULL!=pWrruLoadCodeTimer)
     	{
           	pWrruLoadCodeTimer->Stop();
       }
	pComMsg->Destroy();
	break;
case  Alarm_WRRU_RF_LOS:
	#if 0
	ptr = (unsigned char *)pComMsg->GetDataPtr();
	if(ptr[4]==1)
		{
		if(ptr[5]!=0)
			{
	   AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RF_BOARD_RF_NORESPONSE,
                       ALM_CLASS_MAJOR,
                       STR_BOARD_RF_NORESPONSE,ptr[5]); 
			}
		}
	else if (ptr[4]==0)
		{
		      if(ptr[5]!=0)
		      	{
			   AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RF_BOARD_RF_NORESPONSE,
                       ALM_CLASS_MAJOR,
                       STR_BOARD_RF_NORESPONSE,ptr[5]); 
		      	}
		}
	 #ifdef NO_CLOSE_RF
	RRU_CFG_RFMask(ptr[5]);//close RF channel 
	#endif  
	#endif
	pComMsg->Destroy();
	break;
  case Type_CSI_INFO_Read:
        RRU_Read_CSI_Info(pComMsg);
	 pComMsg->Destroy();
 	break;
 case Type_Read_RRU_Fiber_INfo:
    RRU_Read_Fiber_Info(pComMsg);
     pComMsg->Destroy();
      break;
      
  default:
   	pComMsg->Destroy();
   	break;
      }

}
/****************************************************************************
#define STR_RRU_Fiber_Voltage_Warn    "\r\n RRU Fiber optic module Voltuage Low:%6.2f(uV) "
#define STR_RRU_Fiber_Current_Warn      "\r\n RRU Fiber optic module Current Low:%6.2f(uV)"
#define STR_RRU_Fiber_TX_PWR_Warn      "\r\n RRU Fiber optic module TX_Power Low:%6.2f(uV)"
#define STR_RRU_Fiber_RX_PWR_Warn      "\r\n RRU Fiber optic module Rx_Power Low :%6.2f(uV)"

#define STR_BBU_Fiber_Voltage_Warn          "\r\n BBU Fiber optic module Voltuage Low:%6.2f(uV) "
#define STR_BBU_Fiber_Current_Warn            "\r\n BBU Fiber optic module Current Low:%6.2f(uV)"
#define STR_BBU_Fiber_TX_PWR_Warn            "\r\n BBU Fiber optic module TX_Power Low:%6.2f(uV)"
#define STR_BBU_Fiber_RX_PWR_Warn             "\r\n BBU Fiber optic module Rx_Power Low :%6.2f(uV)"


******************************************************************************/
void CWRRU:: RRU_Read_Fiber_Info(CComMessage *pComMsg)
{
  //需要将信息给显示出来
   float  *Rx_PWR4,*Rx_PWR3,*Rx_PWR2,*Rx_PWR1,*Rx_PWR0;
   short *Tx_I_O,*TX_PWR_O,*T_O,*V_O;//offset;
   float TX_I_S,TX_PWR_S,T_S,V_S;//scope
    short *TX_I_AD,*TX_PWR_AD,*T_AD,*V_AD,*RX_PWR_AD;//AD

    float Temper,Current ,Vol,Tx_pwer,Rx_pwer;
    unsigned char *ptr = (unsigned char *)pComMsg->GetDataPtr();
    if((ptr[6]==0x55)&&(ptr[7]==0x55))
    	{
    	       OAM_LOGSTR(LOG_SEVERE, L3RRU_ERROR_REV_MSG, "[tWRRU]Read RRU Fiber Info ERR\n");
    	       return;
    	}
    unsigned char rru_fiber_info[50];
    memcpy(rru_fiber_info,(ptr+6),50);
   static  unsigned char Vol_warn_flag = 0;
    static  unsigned char Current_warn_flag = 0;
     static  unsigned char Tx_pwr_warn_flag = 0;
      static  unsigned char Rx_pwr_warn_flag = 0;
     #if 0
    for(int j =0 ; j < 50; j++)
    {
       // g_BBU_Fiber_INfo[j] = tt_info[j+1];
    	  printf("0x%x, ",rru_fiber_info[j])   ;
    	  if((j>0)&&(j%9==0))
    	  {
    	     printf("\n");
    	  }
    }
     printf("\n");
    #endif
   /*56-59 4 Rx_PWR(4) Single precision floating point calibration data - Rx optical power. Bit7 of byte 56 is MSB. Bit 0 of byte 59 is LSB. Rx_PWR(4) should be set to zero for “internally calibrated” devices.
60-63 4 Rx_PWR(3) Single precision floating point calibration data - Rx optical power.Bit 7 of byte 60 is MSB. Bit 0 of byte 63 is LSB. Rx_PWR(3) should be set to zero for “internally calibrated” devices.
64-67 4 Rx_PWR(2) Single precision floating point calibration data, Rx optical power.Bit 7 of byte 64 is MSB, bit 0 of byte 67 is LSB. Rx_PWR(2) should be set to zero for “internally calibrated” devices.
68-71 4 Rx_PWR(1) 
*/
   Rx_PWR4 = (float*)(&rru_fiber_info[0]);
   Rx_PWR3 = (float*)(&rru_fiber_info[4]);
   Rx_PWR2 = (float*)(&rru_fiber_info[8]);
   Rx_PWR1 = (float*)(&rru_fiber_info[12]);
   Rx_PWR0 = (float*)(&rru_fiber_info[16]);
   /*********************

76-77 2 Tx_I(Slope) Fixed decimal (unsigned) calibration data, laser bias current. Bit 7 of byte 76 is MSB, bit 0 of byte 77 is LSB. Tx_I(Slope) should be set to 1 for “internally calibrated” devices.
78-79 2 Tx_I(Offset) Fixed decima
   ************************/
   TX_I_S = rru_fiber_info[20] +rru_fiber_info[21]*0.004;
   Tx_I_O = (short*)(&rru_fiber_info[22]);//电流


      /*******
80-81 2 Tx_PWR(Slope) Fixed decimal (unsigned) calibration data, transmitter coupled output power. Bit 7 of byte 80 is MSB, bit 0 of byte 81 is LSB. Tx_PWR(Slope) should be set to 1 for “internally calibrated” devices.
82-83 2 Tx_PWR(Offset) Fixed


   *************/
   TX_PWR_S =  rru_fiber_info[24] +rru_fiber_info[25]*0.004;
   TX_PWR_O =(short*)(&rru_fiber_info[26]);//功率

     /*****
84-85 2 T (Slope) Fixed decimal (unsigned) calibration data, internal module temperature. Bit 7 of byte 84 is MSB, bit 0 of byte 85 is LSB.T(Slope) should be set to 1 for “internally calibrated” devices.
86-87 2 T (Offset) Fixed decimal

  *****/
    T_S = rru_fiber_info[28] +rru_fiber_info[29]*0.004;
   T_O =(short*)(&rru_fiber_info[30]);//温度
   
   /*******************
88-89 2 V (Slope) Fixed decimal (unsigned) calibration data, internal module supply voltage. Bit 7 of byte 88 is MSB, bit 0 of byte 89 is LSB. V(Slope)should be set to 1 for “internally calibrated” devices.
90-91 2 V (Offset) Fixed decimal
   **********************/
   V_S =rru_fiber_info[32] +rru_fiber_info[33]*0.004;
   V_O =(short*)(&rru_fiber_info[34]);//电压

 /*  96 All Temperature MSB Internally measured module temperature.
97 All Temperature LSB*/
T_AD = ( short*)(&rru_fiber_info[40]);

/*
98 All Vcc MSB Internally measured supply voltage in transceiver.
99 All Vcc LSB****/

V_AD = ( short*)(&rru_fiber_info[42]);
/******
100 All TX Bias MSB Internally measured TX Bias Current.
101 All TX Bias LSB******************/
TX_I_AD = (short*)(&rru_fiber_info[44]);
/*******************
102 All TX Power MSB Measured TX output power.
103 All TX Power LSB*****************/
TX_PWR_AD = ( short*)(&rru_fiber_info[46]);


/*********************
104 All RX Power MSB Measured RX input power.
105 All RX Power LSB
*/
    RX_PWR_AD = ( short*)(&rru_fiber_info[48]);
/*
1) Internally measured transceiver temperature. Module temperature, T, is given by the
following equation: T(C) = Tslope * TAD (16 bit signed twos complement value) + Toffset. The result is
in units of 1/256C,*/


Temper = T_S*(*T_AD) + (*T_O);
if(g_print_rru_fiber_info)
{
printf("RRU Fiber optic module Temperature:%6.2f(℃)\n",Temper*0.004);
}

/***************************************
2) Internally measured supply voltage. Module internal supply voltage, V, is given in microvolts
by the following equation: V(uV) = VSLOPE * VAD (16 bit unsigned integer) + VOFFSET. The result is in
units of 100uV,

*******************************************/
Vol = V_S*(*V_AD) +(*V_O);
if(Vol<0)
{
   Vol = Vol*(-1);
}
if(g_print_rru_fiber_info)
{
printf("RRU Fiber optic module Voltuage:%8.2f(uV)\n",Vol*100);
}
if((Vol*100)<NvRamData->fiber_para.Voltage)
{
       if(Vol_warn_flag==0)
       {
    		   AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RRU_FIBER_VOL,
                       ALM_CLASS_INFO,
                       STR_RRU_Fiber_Voltage_Warn,Vol*100); 
    		   Vol_warn_flag = 1;
       }
}
else
{
     if(Vol_warn_flag)
     	{
     	    	   AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RRU_FIBER_VOL,
                       ALM_CLASS_INFO,
                       STR_RRU_Fiber_Voltage_Warn,Vol*100); 
     	}
     Vol_warn_flag = 0;
}
/***************************************
3) Measured transmitter laser bias current. Module laser bias current, I, is given in microamps
by the following equation: I (uA) = ISLOPE * IAD (16 bit unsigned integer) + IOFFSET. This result is in
units of 2 uA, 

***************************************/
Current = (TX_I_S)*(*TX_I_AD)+(*Tx_I_O);

if(g_print_rru_fiber_info)
{
printf("RRU Fiber optic module Current:%6.2f(uA)\n",Current*2);
}
if((Current*2)<NvRamData->fiber_para.Current)
{
       if(Current_warn_flag==0)
       {
       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RRU_FIBER_Current,
                       ALM_CLASS_INFO,
                       STR_RRU_Fiber_Current_Warn,Current*2); 
       Current_warn_flag = 1;
       }
}
else
{
       if(Current_warn_flag)
     	{
     	    	   AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RRU_FIBER_Current,
                       ALM_CLASS_INFO,
                       STR_RRU_Fiber_Current_Warn,Current*2); 
     	}
     Current_warn_flag = 0;
}
/*
4) Measured coupled TX output power. Module transmitter coupled output power, TX_PWR,
is given in uW by the following equation: TX_PWR (uW) = TX_PWRSLOPE * TX_PWRAD (16 bit
unsigned integer) + TX_PWROFFSET. This result is in units of 0.1uW */

Tx_pwer = TX_PWR_S*(*TX_PWR_AD) +(*TX_PWR_O);
if(g_print_rru_fiber_info)
{
printf("RRU Fiber optic module Tx_Powert:%6.2f(uW)\n",Tx_pwer*0.1);
}

if((Tx_pwer*0.1)<NvRamData->fiber_para.Tx_Power)
{
    if(Tx_pwr_warn_flag==0)
    	{
       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RRU_FIBER_TX_PWR,
                       ALM_CLASS_INFO,
                       STR_RRU_Fiber_TX_PWR_Warn,(Tx_pwer*0.1)); 
       Tx_pwr_warn_flag = 1;
    	}
}
else
{
    if(Tx_pwr_warn_flag)
    	{
    	    
       AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RRU_FIBER_TX_PWR,
                       ALM_CLASS_INFO,
                       STR_RRU_Fiber_TX_PWR_Warn,Tx_pwer*0.1); 
    	}
    Tx_pwr_warn_flag = 0;
}
/*
5) Measured received optical power. Received power, RX_PWR, is given in uW by the
following equation:
Rx_PWR (uW) = Rx_PWR(4) * Rx_PWRAD
4 (16 bit unsigned integer) +
Rx_PWR(3) * Rx_PWRAD
3 (16 bit unsigned integer) +
Rx_PWR(2) * Rx_PWRAD
2 (16 bit unsigned integer) +
Rx_PWR(1) * Rx_PWRAD (16 bit unsigned integer) +
Rx_PWR(0)
The result is in units of 0.1uW yielding a total range of 0 – 6.5mW.*/


Rx_pwer = (*Rx_PWR4)*(*RX_PWR_AD^4) +  (*Rx_PWR3)*(*RX_PWR_AD^3)  +( *Rx_PWR2)*(*RX_PWR_AD^2) + (*Rx_PWR1)*(*RX_PWR_AD) + (*Rx_PWR0);
if(g_print_rru_fiber_info)
{
printf("RRU Fiber optic module Rx_Powert:%6.2f(uW)\n",Rx_pwer*0.1);
}
g_print_rru_fiber_info = 0;
if((Rx_pwer*0.1)<NvRamData->fiber_para.Rx_Power)
{
   if(Rx_pwr_warn_flag==0)
   	{
            AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RRU_FIBER_RX_PWR,
                       ALM_CLASS_INFO,
                       STR_RRU_Fiber_RX_PWR_Warn,Rx_pwer*0.1); 
            Rx_pwr_warn_flag = 1;
   	}
}
else
{
    if(Rx_pwr_warn_flag)
    	{
    	     AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_WRRU, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_RRU_FIBER_RX_PWR,
                       ALM_CLASS_INFO,
                       STR_RRU_Fiber_RX_PWR_Warn,Rx_pwer*0.1); 
    	}
     Rx_pwr_warn_flag= 0;
}
unsigned int  Temper1,Current1 ,Vol1,Tx_pwer1,Rx_pwer1;
/******************************************************

消息方向：BTS ' EMS
字段名称	长度（Byte）	类型	定义	描述
RRU光模块温度	4	M		单位：摄氏度
RRU光模块电压	4	M		单位：uV
RRU光模块电流	4	M		单位：uA
RRU光模块发射功率	4	M		单位：uW
RRU光模块接收功率	4	M		单位：uW
				
**********************************************************/
if(g_RRU_Fiber_Transid!=0)
{
 CComMessage *pMsg = NULL;
            
 unsigned char *p;
 pMsg = new (this, 2+2+20) CComMessage;
 if(pMsg==NULL)
 {
     return;
 }
p =(unsigned char *)pMsg->GetDataPtr();
pMsg->SetDstTid(M_TID_EMSAGENTTX);
pMsg->SetSrcTid(M_TID_WRRU);
pMsg->SetMessageId(M_EMS_BTS_RRU_Fiber_Info_RSP);
p[0] =g_RRU_Fiber_Transid>>8;//ptr[0];
p[1] =g_RRU_Fiber_Transid;//ptr[1];//result;
p[2] =0;
p[3] = 0;
Temper1 =(unsigned int) (Temper*0.004*100);
memcpy(p+4,(unsigned char *)&Temper1,4);
Vol1 = (unsigned int)(Vol*10000);
memcpy(p+8,(unsigned char *)&Vol1,4);
Current1 = (unsigned int)(Current*2*100);
memcpy(p+12,(unsigned char *)&Current1,4);
Tx_pwer1 = (unsigned int)(Tx_pwer*0.1*100);
memcpy(p+16,(unsigned char *)&Tx_pwer1,4);
Rx_pwer1 = (unsigned int)(Rx_pwer*0.1*100);
memcpy(p+20,(unsigned char *)&Rx_pwer1,4);
if(true !=  CComEntity::PostEntityMessage(pMsg))     
{
	OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	pMsg->Destroy();
}
}
g_RRU_Fiber_Transid = 0;

}
/**********************************************************************************
对RRU csi信息的返回处理
如果type为 1，则表示读fiber module info ,不需要继续处理
如果type为0，则表示继续需要发送消息给RRU读CSI的下一条消息，一直读完为止

*********************************************************************************/
 void CWRRU::RRU_Read_CSI_Info(CComMessage *pComMsg)
 {
      /*****************************消息格式如下*************************************
       	unsigned char des;
		unsigned char src;
		unsigned char type;
		unsigned char length;
		unsigned char type  0----csi,1----fiber Module info
		unsigned char seq: 0
		content:


      *********************************************************************/
	 char *pMsgWRRU=(char*)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message));
	 unsigned char seq= 0;
	 if(pMsgWRRU[0] == 1)
	 {
	   //返回的是fiber module info
	   //OAM_LOGSTR(LOG_SEVERE, L3RRU_ERROR_REV_MSG, "[tWRRU] RRU_Read_CSI_Info fiber Module Info\n");
	   //需要将信息给显示出来

	   //需要将信息给显示出来
		   float  *Rx_PWR4,*Rx_PWR3,*Rx_PWR2,*Rx_PWR1,*Rx_PWR0;
		   short *Tx_I_O,*TX_PWR_O,*T_O,*V_O;//offset;
		   float TX_I_S,TX_PWR_S,T_S,V_S;//scope
		    short *TX_I_AD,*TX_PWR_AD,*T_AD,*V_AD,*RX_PWR_AD;//AD

		    float Temper,Current ,Vol,Tx_pwer,Rx_pwer;

		      unsigned char *ptr = (unsigned char *)pComMsg->GetDataPtr();
		    unsigned char rru_fiber_info[50];
		    unsigned char time_info[6];
		    memcpy(time_info,ptr+6,6);
		    printf("Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",time_info[0],time_info[1],time_info[2],time_info[3],time_info[5]*0x100+time_info[4]);
		    memcpy(rru_fiber_info,(ptr+12),50);
		     #if 0
		    for(int j =0 ; j < 50; j++)
		    {
		       // g_BBU_Fiber_INfo[j] = tt_info[j+1];
		    	  printf("0x%x, ",rru_fiber_info[j])   ;
		    	  if((j>0)&&(j%9==0))
		    	  {
		    	     printf("\n");
		    	  }
		    }
		    printf("\n");
		    #endif
		   /*56-59 4 Rx_PWR(4) Single precision floating point calibration data - Rx optical power. Bit7 of byte 56 is MSB. Bit 0 of byte 59 is LSB. Rx_PWR(4) should be set to zero for “internally calibrated” devices.
		60-63 4 Rx_PWR(3) Single precision floating point calibration data - Rx optical power.Bit 7 of byte 60 is MSB. Bit 0 of byte 63 is LSB. Rx_PWR(3) should be set to zero for “internally calibrated” devices.
		64-67 4 Rx_PWR(2) Single precision floating point calibration data, Rx optical power.Bit 7 of byte 64 is MSB, bit 0 of byte 67 is LSB. Rx_PWR(2) should be set to zero for “internally calibrated” devices.
		68-71 4 Rx_PWR(1) 
		*/
		   Rx_PWR4 = (float*)(&rru_fiber_info[0]);
		   Rx_PWR3 = (float*)(&rru_fiber_info[4]);
		   Rx_PWR2 = (float*)(&rru_fiber_info[8]);
		   Rx_PWR1 = (float*)(&rru_fiber_info[12]);
		   Rx_PWR0 = (float*)(&rru_fiber_info[16]);
		   /*********************

		76-77 2 Tx_I(Slope) Fixed decimal (unsigned) calibration data, laser bias current. Bit 7 of byte 76 is MSB, bit 0 of byte 77 is LSB. Tx_I(Slope) should be set to 1 for “internally calibrated” devices.
		78-79 2 Tx_I(Offset) Fixed decima
		   ************************/
		   TX_I_S = rru_fiber_info[20] +rru_fiber_info[21]*0.004;
		   Tx_I_O = (short*)(&rru_fiber_info[22]);//电流


		      /*******
		80-81 2 Tx_PWR(Slope) Fixed decimal (unsigned) calibration data, transmitter coupled output power. Bit 7 of byte 80 is MSB, bit 0 of byte 81 is LSB. Tx_PWR(Slope) should be set to 1 for “internally calibrated” devices.
		82-83 2 Tx_PWR(Offset) Fixed


		   *************/
		   TX_PWR_S =  rru_fiber_info[24] +rru_fiber_info[25]*0.004;
		   TX_PWR_O =(short*)(&rru_fiber_info[26]);//功率

		     /*****
		84-85 2 T (Slope) Fixed decimal (unsigned) calibration data, internal module temperature. Bit 7 of byte 84 is MSB, bit 0 of byte 85 is LSB.T(Slope) should be set to 1 for “internally calibrated” devices.
		86-87 2 T (Offset) Fixed decimal

		  *****/
		    T_S = rru_fiber_info[28] +rru_fiber_info[29]*0.004;
		   T_O =(short*)(&rru_fiber_info[30]);//温度
		   
		   /*******************
		88-89 2 V (Slope) Fixed decimal (unsigned) calibration data, internal module supply voltage. Bit 7 of byte 88 is MSB, bit 0 of byte 89 is LSB. V(Slope)should be set to 1 for “internally calibrated” devices.
		90-91 2 V (Offset) Fixed decimal
		   **********************/
		   V_S =rru_fiber_info[32] +rru_fiber_info[33]*0.004;
		   V_O =(short*)(&rru_fiber_info[34]);//电压

		 /*  96 All Temperature MSB Internally measured module temperature.
		97 All Temperature LSB*/
		T_AD = ( short*)(&rru_fiber_info[40]);

		/*
		98 All Vcc MSB Internally measured supply voltage in transceiver.
		99 All Vcc LSB****/

		V_AD = ( short*)(&rru_fiber_info[42]);
		/******
		100 All TX Bias MSB Internally measured TX Bias Current.
		101 All TX Bias LSB******************/
		TX_I_AD = (short*)(&rru_fiber_info[44]);
		/*******************
		102 All TX Power MSB Measured TX output power.
		103 All TX Power LSB*****************/
		TX_PWR_AD = ( short*)(&rru_fiber_info[46]);


		/*********************
		104 All RX Power MSB Measured RX input power.
		105 All RX Power LSB
		*/
		    RX_PWR_AD = ( short*)(&rru_fiber_info[48]);
		/*
		1) Internally measured transceiver temperature. Module temperature, T, is given by the
		following equation: T(C) = Tslope * TAD (16 bit signed twos complement value) + Toffset. The result is
		in units of 1/256C,*/


		Temper = T_S*(*T_AD) + (*T_O);

		printf("RRU CSI Fiber optic module Temperature:%6.2f(℃)\n",Temper*0.004);

		/***************************************
		2) Internally measured supply voltage. Module internal supply voltage, V, is given in microvolts
		by the following equation: V(uV) = VSLOPE * VAD (16 bit unsigned integer) + VOFFSET. The result is in
		units of 100uV,

		*******************************************/
		Vol = V_S*(*V_AD) +(*V_O);

		printf("RRU CSI Fiber optic module Voltuage:%8.2f(uV)\n",Vol*100);
		/***************************************
		3) Measured transmitter laser bias current. Module laser bias current, I, is given in microamps
		by the following equation: I (uA) = ISLOPE * IAD (16 bit unsigned integer) + IOFFSET. This result is in
		units of 2 uA, 

		***************************************/
		Current = (TX_I_S)*(*TX_I_AD)+(*Tx_I_O);


		printf("RRU CSI Fiber optic module Current:%6.2f(uA)\n",Current*2);
		/*
		4) Measured coupled TX output power. Module transmitter coupled output power, TX_PWR,
		is given in uW by the following equation: TX_PWR (uW) = TX_PWRSLOPE * TX_PWRAD (16 bit
		unsigned integer) + TX_PWROFFSET. This result is in units of 0.1uW */

		Tx_pwer = TX_PWR_S*(*TX_PWR_AD) +(*TX_PWR_O);
		printf("RRU CSI Fiber optic module Tx_Powert:%6.2f(uW)\n",Tx_pwer*0.1);
		/*
		5) Measured received optical power. Received power, RX_PWR, is given in uW by the
		following equation:
		Rx_PWR (uW) = Rx_PWR(4) * Rx_PWRAD
		4 (16 bit unsigned integer) +
		Rx_PWR(3) * Rx_PWRAD
		3 (16 bit unsigned integer) +
		Rx_PWR(2) * Rx_PWRAD
		2 (16 bit unsigned integer) +
		Rx_PWR(1) * Rx_PWRAD (16 bit unsigned integer) +
		Rx_PWR(0)
		The result is in units of 0.1uW yielding a total range of 0 – 6.5mW.*/


		Rx_pwer = (*Rx_PWR4)*(*RX_PWR_AD^4) +  (*Rx_PWR3)*(*RX_PWR_AD^3)  +( *Rx_PWR2)*(*RX_PWR_AD^2) + (*Rx_PWR1)*(*RX_PWR_AD) + (*Rx_PWR0);

		printf("RRU CSI Fiber optic module Rx_Powert:%6.2f(uW)\n",Rx_pwer*0.1);
	 }
	 else
	 {
	       unsigned char *ptr = (unsigned char *)pComMsg->GetDataPtr();
             unsigned char pdata[96];//12+80+4
             memcpy(pdata,(ptr+6),96);
             int k =0;
             unsigned char flag = 0;
            #if 0
		    for(int j =0 ; j < 92; j++)
		    {
		       // g_BBU_Fiber_INfo[j] = tt_info[j+1];
		    	  printf("%x, ",pdata[j])   ;
		    	  if((j>0)&&(j%9==0))
		    	  {
		    	     printf("\n");
		    	  }
		    }
		    printf("\n");
	   #endif
             /**************************0-10记录的是各个复位原因复位的次数
			 typedef enum
				{
				    CSI_ERROR_SPURIOUS_ABORT =0,//Spurious
				    CSI_ERROR_UNDEF_IRQ,//不存在的IRQ中断
				    CSI_ERROR_UNDEF_FIQ,//不存在的FIQ中断
				    CSI_ERROR_UNDEFINE_INT,//未定义异常中断
				    CSI_ERROR_UNDEF_SWI,//未定义软中断
				    CSI_ERROR_PREFETCH,//指令预取错误
				    CSI_ERROR_DATA_ABORT,//数据异常
				    CSI_ERROR_FPGA_LOAD,//fpga加载失败
				    CSI_ERROR_MSG_REV_FAIL,//接收polling消息失败
				    CSI_ERROR_REV_RESET_MSG,//接收到重启动消息
				    //CSI_ERROR_FPGA_DL,//fpga下载完成\
				    CSI_POWERON_RUNNING,//接收到重启动消息

				    CSI_RESET_REASON_MAX//11
				}T_CSIRESETREASON;

             ********************************************************************************/
                for(int i = 0; i< 11; i++)
             	{
             	    if(pdata[i]>0)
             	    	{
             	    	    flag = 1;
             	    	    break;
             	    	}
             	}
             if(flag==0)
             	{
             	printf("there are no useful rru csi Record\n");
             	   return;
             	}
             printf("RRU Reset Counter for all Reason\n");
             printf("CSI_ERROR_SPURIOUS_ABORT:%d\n",pdata[0]);
             printf("CSI_ERROR_UNDEF_IRQ:%d\n",pdata[1]);
             printf("CSI_ERROR_UNDEF_FIQ:%d\n",pdata[2]);
             printf("CSI_ERROR_UNDEFINE_INT:%d\n",pdata[3]);
             printf("CSI_ERROR_UNDEF_SWI:%d\n",pdata[4]);
             printf("CSI_ERROR_PREFETCH:%d\n",pdata[5]);
             printf("CSI_ERROR_DATA_ABORT:%d\n",pdata[6]);
             printf("CSI_ERROR_FPGA_LOAD:%d\n",pdata[7]);
             printf("CSI_ERROR_MSG_REV_FAIL:%d\n",pdata[8]);
             printf("CSI_ERROR_REV_RESET_MSG:%d\n",pdata[9]);
             printf("CSI_POWERON_RUNNING:%d\n",pdata[10]);
              printf("CSI_POWERDown_Reset:%d\n",pdata[11]);
            
             /********************11-90 80个字节记录复位原因，具体结构如下

		             typedef struct _csiinfo
				{
				    UINT8   s_reason_reset;//重启动原因
				    UINT8   s_rev;
				    UINT8   s_month;
				    UINT8   s_day;
				    UINT8   s_hour;		
				    UINT8   s_minute;		
				    UINT16  s_runTime;//开机运行的时间
				}CsiInfo;
		***********************************************************************************/
              for(k =0; k< 10;k++)
              {
                    printf("RRU CSI Record :%d\n",k);
                    if(pdata[12+k*8]==0)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_SPURIOUS_ABORT :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                    else if(pdata[12+k*8]==1)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_UNDEF_IRQ :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                    else if(pdata[12+k*8]==2)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_UNDEF_FIQ :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                       else if(pdata[12+k*8]==3)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_UNDEFINE_INT :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                       else if(pdata[12+k*8]==4)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_UNDEF_SWI :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                       else if(pdata[12+k*8]==5)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_PREFETCH :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                        else if(pdata[12+k*8]==6)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_DATA_ABORT :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                       else if(pdata[12+k*8]==7)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_FPGA_LOAD :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                       else if(pdata[12+k*8]==8)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_MSG_REV_FAIL :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                       else if(pdata[12+k*8]==9)
                    {
                    	printf("Reset Reseaon:CSI_ERROR_REV_RESET_MSG :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                       else if(pdata[12+k*8]==10)
                    {
                    	printf("Reset Reseaon:CSI_POWERON_RUNNING :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                     else if(pdata[12+k*8]==11)
                    {
                    	printf("Reset Reseaon:CSI_POWERDown_RESET :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
                    else
                    {
                    	printf("Reset Reseaon:Unknown :Reset Time:%d/%d/%d/%d:RunningTime:%d(min)\n",pdata[14+k*8],pdata[15+k*8],pdata[16+k*8],pdata[17+k*8],pdata[19+k*8]*0x100+pdata[18+k*8]);
                    }
              }
		printf("RRU  Total Reset times:%d\n",pdata[92]*0x1000000+pdata[93]*0x10000+pdata[94]*0x100+pdata[95]);
	  //返回的是CSI信息；
	  //将csi格式信息给打印出来；
	  // seq = pMsgWRRU[1];
	 //  OAM_LOGSTR1(LOG_SEVERE, L3RRU_ERROR_REV_MSG, "[tWRRU] RRU_Read_CSI_Info Record:%d\n",seq);
	    
	  
	 }
 }
void CWRRU::RRU_CFG_RFMask(unsigned char anntenaidx,unsigned char flag)
{
     CComMessage* pComMsg = new (this, 2) CComMessage;
	if (pComMsg==NULL)
	{
	LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in gpsConfigRFMask.");
	return ;
	}
	 unsigned char *ptr;
	 ptr =(unsigned char *) pComMsg->GetDataPtr();

	pComMsg->SetDstTid(M_TID_FM);
	pComMsg->SetSrcTid(M_TID_WRRU);    
	pComMsg->SetMessageId(M_WRRU_ALM_CHG_RF_MASK_REQ);
	ptr[0] = anntenaidx;
	ptr[1] = flag;
	pComMsg->SetDataLength(2);
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
	}
}

void CWRRU::RRU_Start_CFG_Timer()
{
    if(NULL==pWRRUCfgTimer)
    	{
    	}
	else
	{
	    pWRRUCfgTimer->Start();
	}
}
void CWRRU::RRU_Start_CFG_Timer_other()
{
        if(NULL==pWRRUCfgTimer_other)
    	{
    	}
	else
	{
	    pWRRUCfgTimer_other->Start();
	}
}

void CWRRU::RRU_SendCaliHWToAUX(unsigned char flag)
{
/*
Field	Length(bit)	Definition	
version	16	Define factory data version number	
Head size	16	Default 128bytes	
Band ID	16	Integer of 100Mhz，1800	
Dtmax	16	Dt Power Max,( lowest sensitivity level)	
Dtmin	16	Dt Power Min( highest sensitivity level)	
Dtstep	16	Dt power step(db)	
Drmax	16	Dr Power Max,( lowest sensitivity level)	
Drmin	16	Dr Power Min( highest sensitivity level)	
Drstep	16	Dr power step(db)	
Tmax	16	Maximum temperature	
Tmin	16	Minimum temperature	
Tstep	16	Temperature step size	
Fmin	       16	Minimum calibration frequency Default 1790	
Fmax	16	Maximum calibration frequency，Default 1800	
Fstep	16	Calibration frequency step size Default 10	
PLLVersion	16	Rsv, no use	
PLLFormat	16	Rsv, no use	
PLLCnt	16	Rsv, no use	
PLLmin	16	Rsv, no use	
*/
       short  i=0,j=0,k=0;
	short Fre_Max=0,Fre_Min=0,Fre_Step=0,Dt_Max=0,Dt_Min=0,Dt_Step=0;
	short Dr_Max=0,Dr_Min=0,Dr_Step=0;
    if(flag==1)
    	{
    	     memcpy(Calibration_HW_Data,NvRamData->Cal_Para.Para,104*11);
			  memcpy(&Calibration_HW_Data[11*104],NvRamData->Cal_Para_Seccond,1200);
    	}
	Dt_Max=Calibration_HW_Data[7]|(Calibration_HW_Data[6]<<8);
	Dt_Min=Calibration_HW_Data[9]|(Calibration_HW_Data[8]<<8);
	Dt_Step=Calibration_HW_Data[11]|(Calibration_HW_Data[10]<<8);
	Dr_Max=Calibration_HW_Data[13]|(Calibration_HW_Data[12]<<8);
	Dr_Min=Calibration_HW_Data[15]|(Calibration_HW_Data[14]<<8);
	Dr_Step=Calibration_HW_Data[17]|(Calibration_HW_Data[16]<<8);
	Fre_Min=Calibration_HW_Data[25]|(Calibration_HW_Data[24]<<8);
	Fre_Max=Calibration_HW_Data[27]|(Calibration_HW_Data[26]<<8);
	Fre_Step=Calibration_HW_Data[29]|(Calibration_HW_Data[28]<<8);

      printf("HW_28=%d,HW_29 =%d\n",Calibration_HW_Data[28],Calibration_HW_Data[29]);

      printf("Dt_Max=%d,Dt_Min =%d,Dt_Step=%d\n",Dt_Max,Dt_Min,Dt_Step);
      printf("Dr_Max=%d,Dr_Min =%d,Dr_Step=%d\n",Dr_Max,Dr_Min,Dr_Step);
      printf("Fre_Max=%d,Fre_Min =%d,Fre_Step=%d\n",Fre_Max,Fre_Min,Fre_Step);

	i=(Fre_Max-Fre_Min)/Fre_Step+1;
	j=((Dt_Max-Dt_Min)/Dt_Step+1)*i*2;
	k=((Dr_Max-Dr_Min)/Dr_Step+1)*i*2;
      printf("i=%d,j =%d,k=%d\n",i,j,k);

	i=128+j+k;//now i means the total byte of HW should be sent

      printf("i=%d,j =%d,k=%d\n",i,j,k);
	CComMessage* pMsgHWToL2=new(this, i+2) CComMessage;
	if(pMsgHWToL2==NULL)
	{
	    return;
	}
	pMsgHWToL2->SetDstTid(M_TID_L2MAIN);
	pMsgHWToL2->SetSrcTid(M_TID_WRRU);
	pMsgHWToL2->SetMessageId(M_L1_Calibration_HW);
	pMsgHWToL2->SetMoudlue(0);
	pMsgHWToL2->SetDataLength(i+2);
	memcpy(((char *)pMsgHWToL2->GetDataPtr() +2), &Calibration_HW_Data, i);
	//NvRamData->Cal_Para.IsValid = 0x55aa55aa;
	//l3rrubspNvRamWrite(&(NvRamData->Cal_Para.Para[0]),&Calibration_HW_Data[0],1200);
	//change_bytes((unsigned char*)pMsgHWToL2->GetDataPtr());
	CComEntity::PostEntityMessage(pMsgHWToL2);    
}


void CWRRU::Process_EMS_msg(CComMessage* pComMsg)
{
    UINT16 MsgId;
   unsigned char fiber_index =0;
    MsgId = pComMsg->GetMessageId();
    T_RSV_Msg  temp;
    unsigned char *p,*ptr;
    static unsigned int temp_flag = 0;
    	FiberInfoLimint temp_struct;
 //   OAM_LOGSTR1(LOG_SEVERE, L3RRU_ERROR_REV_MSG, "[tCWRRU] receive msg = 0x%04x", MsgId);

   /* if (data_WRRU.WRRU_State!=RUNNING)
    {
     //    OAM_LOGSTR1(LOG_SEVERE, L3RRU_ERROR_RRU_UNREADY, "[tCWRRU] receive msg when RRU is not working msg = 0x%04x", MsgId);
         return;
    }*/
    
    switch (MsgId)
    {
        case M_EMS_BTS_RRU_TEMP_CFG_REQ:

        	RRU_ProceedCirRRU(pComMsg, Type_Config_WRRU_CircumsPara, 0x0c);
        	
        	p = (unsigned char*)pComMsg->GetDataPtr();
        	temp.Rsv = 0;
        	temp.transID = p[0]*0x100+p[1];
        	RRU_RRUResToEMS(&temp,M_BTS_EMS_RRU_TEMP_CFG_RSP);
              pComMsg->Destroy();
        	break;
			
         case M_EMS_BTS_RRU_TEMP_GET_REQ:
        	RRU_NvramCirBackEms((UINT16*)(pComMsg->GetDataPtr()), M_BTS_EMS_RRU_TEMP_GET_RSP);
              pComMsg->Destroy();
        	break;
			
         case M_EMS_BTS_RRU_WORKING_STATUS_GET_REQ:
        	RRU_StatusEms((UINT16*)(pComMsg->GetDataPtr()),M_BTS_EMS_RRU_WORKING_STATUS_GET_RSP);
              pComMsg->Destroy();
        	break;

         case M_EMS_BTS_RRU_RESET_REQ:
        	 RRU_ProceedResetRRU(Type_Reset_WRRU, 0x02);
              pComMsg->Destroy();
        	break;

        case M_EMS_BTS_RRU_SYN_SWITCH_CFG_REQ:
        	//T_RSV_Msg  temp;
        	p = (unsigned char*)pComMsg->GetDataPtr();
        	temp.Rsv = 0;
        	temp.transID = p[0]*0x100+p[1];
        	RRU_RRUResToEMS(&temp,M_BTS_EMS_RRU_SYN_SWITCH_CFG_RSP);
         	RRU_ProceedSYNSwithRRU(pComMsg, Type_Config_WRRU_General, 0x06);
              pComMsg->Destroy();
       	break;

	case M_EMS_BTS_RRU_SYN_SWITCH_GET_REQ:
        	RRU_NvramSYNBackEms((UINT16*)(pComMsg->GetDataPtr()), M_BTS_EMS_RRU_SYN_SWITCH_GET_RSP);
              pComMsg->Destroy();
		break;

	case M_EMS_BTS_RF_CFG_REQ:
		RRU_ProcessRFCFGEmsMsg(pComMsg,Type_Config_WRRU_PLL,0x06);
		break;

	case M_EMS_BTS_CARRIER_DATA_CFG_REQ: 
	 	RRU_ProcessCarrierEmsMsg(pComMsg,Type_Config_WRRU_Timeslot,0x04);
	 	break;

	case M_EMS_BTS_CALIBRAT_CFG_GENDATA_REQ:
	 	RRU_ProcessCalibratEmsMsg(pComMsg,Type_Config_WRRU_CalibrationData,0x16);
		break;
		
///structure has not defined yet
        case M_EMS_BTS_RRU_SPF_CFG_REQ:
        	//T_RSV_Msg  temp;
        	p = (unsigned char*)pComMsg->GetDataPtr();
        	temp.Rsv = 0;
        	temp.transID = p[0]*0x100+p[1];
        	RRU_RRUResToEMS(&temp,M_BTS_EMS_RRU_SPF_CFG_RSP);
              pComMsg->Destroy();
        	break;
        case M_EMS_BTS_RRU_SPF_GET_REQ:
		RRU_ProceedQueryVer(pComMsg,Type_Query_WRRU_SPF_Version, 0x02);		
              pComMsg->Destroy();
        	break;
	 case M_EMS_BTS_RRU_VER_GET_REQ:
		RRU_ProceedQueryVer(pComMsg,Type_Query_WRRU_Version, 0x02);		
              pComMsg->Destroy();
	 	break;
	 case M_EMS_BTS_RRU_RF_VER_GET_REQ:
            	memset((char*)rf_ver,0,88);
            	memset((char*)rf_ver2,0,88);
		memset((char*)DSB_SYN_ver,0,44);
            
		RRU_ProceedQueryVer(pComMsg,Type_WRRU_SYN_DSB_Ver, 0x02);	//wangwenhua add 20100505
		taskDelay(20);
		if(1)
		{
	   		RRU_ProceedQueryVer(pComMsg,Type_Query_WRRU_RF5to8_Version/*Type_Query_WRRU_RF1to4_Version*/, 0x02);	

	   		taskDelay(27);
	   	//	RRU_ProceedQueryVer(pComMsg,Type_Query_WRRU_RF1to4_Version/*Type_Query_WRRU_RF5to8_Version*/, 0x02);
		}
	
		temp_flag++;
		flag_rf_ver_get=0;
              pComMsg->Destroy();
	     	break;
	case M_EMS_BTS_RRU_HW_STATUS_GET_REQ://2010-5-19 add 
             RRU_ProcessQueryHwStatus(pComMsg,Type_WRRU_TEmp_Current, 0x02);
			pComMsg->Destroy();
		break;
	  case M_EMS_BTS_Fiber_Info_CFG_REQ:
	  
	  	temp_struct.initialized = 0x5555aaaa;
	  	ptr = (unsigned char *)pComMsg->GetDataPtr();
	  	memcpy((unsigned char *)&temp_struct.Voltage,ptr+2,4);
	  	memcpy((unsigned char *)&temp_struct.Current,ptr+6,4);
	  	memcpy((unsigned char *)&temp_struct.Tx_Power,ptr+10,4);
	  	memcpy((unsigned char *)&temp_struct.Rx_Power,ptr+14,4);
	  	l3rrubspNvRamWrite(( char *)&NvRamData->fiber_para,( char*)&temp_struct,sizeof(temp_struct));
	  	temp.Rsv = 0;
        	temp.transID = ptr[0]*0x100+ptr[1];
        	RRU_RRUResToEMS(&temp,M_EMS_BTS_Fiber_Info_CFG_RSP);
	  	pComMsg->Destroy();
	  	break;
	  case M_EMS_BTS_Fiber_Info_GET_REQ:
	  	RRU_FiberInfoToEMS((UINT16*)(pComMsg->GetDataPtr()),M_EMS_BTS_Fiber_Info_GET_RSP);
	  	pComMsg->Destroy();
	  	break;
	case M_EMS_BTS_RRU_Fiber_Info_REQ:
		g_RRU_Fiber_Transid = *(UINT16*)(pComMsg->GetDataPtr());
		RRU_ReadFiberInfo((UINT16*)(pComMsg->GetDataPtr()));
		pComMsg->Destroy();
		break;
	case M_EMS_BTS_BBU_Fiber_Info_REQ	:
		g_BBU_Fiber_Transid = *(UINT16*)(pComMsg->GetDataPtr());
		fiber_index = ReadFiberno();
		ReadBBUFiberInfo(fiber_index);
		taskDelay(50);
		ShowBBUFiberInfo(0);
		pComMsg->Destroy();
		break;
	 default:
	 	pComMsg->Destroy();
		break;

        
    }
 
}
void  CWRRU::RRU_ReadFiberInfo(unsigned short *transid)
{

     	        OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] RRU_ReadFiberInfo...");
     //  unsigned short *pData=(unsigned short *)((unsigned char*)InMsg->GetDataPtr());

	CComMessage *pComMsg = NULL;
	pComMsg = new (this, 112) CComMessage;
	if(pComMsg==NULL)
	{
	    return;
	}
	pComMsg->SetDstTid(M_TID_L2MAIN);
	pComMsg->SetSrcTid(M_TID_WRRU);
	pComMsg->SetMessageId(Type_L3_WRRU);
	pComMsg->SetMoudlue(0);

	WRRU_message* p=(WRRU_message*)pComMsg->GetDataPtr();
	p->src=BBU_No;
	p->des=data_WRRU.WRRU_Num;
	p->type=Type_Read_RRU_Fiber_INfo;
	p->length=2;

	
	UINT16 *pRes=(UINT16*)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message));
	*pRes=0;
	UINT16 *pResCRC=(UINT16 *)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(UINT16));

       unsigned short length=p->length/2+2;
	*pResCRC=CheckSum((unsigned short *) p,length);
       change_bytes((unsigned char*)pComMsg->GetDataPtr());
         CComEntity::PostEntityMessage(pComMsg);    
}
/************************************************************8
6.2.1.2.2.125　光模块告警门限查询应答
消息名称：光模块告警门限查询应答
消息方向： BTS ' EMS
消息功能：查询光模块的告警门限应答
表86　光模块告警门限查询请求
字段名称	长度（Byte）	类型	定义	描述
光模块电压告警门限	4	M	2000000uV	
光模块电流告警门限	4	M	8000uA	
光模块发射功率告警门限	4	M	300uW	
光模块接收功率告警门限	4	M	200uW	

*****************************************************************/
void CWRRU::RRU_FiberInfoToEMS(UINT16 *tranID,UINT16 MsgId)
{
              CComMessage *pMsg = NULL;
     	     pMsg = new (this, 4*4+2) CComMessage;
	  	if(pMsg==NULL)
	{
	    return;
	}
            unsigned char *ptr = (unsigned char*)pMsg->GetDataPtr();
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgId);
	     
            T_ConfigCirCumsPara_Msg* pData=(T_ConfigCirCumsPara_Msg*)((unsigned char*)pMsg->GetDataPtr());

            memcpy(ptr,(unsigned char*)tranID,2);
            memcpy(ptr+2,(unsigned char*)&(NvRamData->fiber_para.Voltage),16);
	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }
}
void CWRRU::RRU_RRURFVerToEMS(unsigned char flag,UINT16 *result_msg,UINT16 MsgID)
{
            CComMessage *pMsg = NULL;
//	     UINT16    temp_buffer[44];
	//	 int i;
     	     pMsg = new (this, 180/*176+2+2*/+44) CComMessage;
        if(pMsg==NULL)
	{
	    return;
	}
          unsigned  char * p;
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgID);
#if 0
	// 2(transid)+2(result)+88+88
	    for(i = 0; i < 44; i++)
	    {
              if((i!=0)&&(i!=11)&&(i!=22)&&(i!=33))
		{
			temp_buffer[i] = (result_msg[i+1]>>8)|(result_msg[i+1]<<8);
		}
		 
	    }
#endif
	     if (flag==1)
	     {
	            p =(unsigned char*) pMsg->GetDataPtr() ;
	            p[2] = 0;
	            p[3] = 0;//result;
	           memcpy((pMsg->GetDataPtr() ),result_msg, sizeof(unsigned short));//transid
	            memcpy(((unsigned char*)pMsg->GetDataPtr() + 4),(((unsigned char*)rf_ver/*result_msg*/) /*+2*/), 88);
		     memcpy(((UINT8 *)pMsg->GetDataPtr()+92),(unsigned char*)rf_ver2,sizeof(rf_ver2));
		     memcpy(((UINT8 *)pMsg->GetDataPtr()+180),&DSB_SYN_ver[22],22);//syn
		     memcpy(((UINT8 *)pMsg->GetDataPtr()+180+22),&DSB_SYN_ver[0],22);//DSB
	     }
	     if (flag==5)
	     {
	            memcpy((pMsg->GetDataPtr() ),result_msg, sizeof(unsigned short));//transid
	            p =(unsigned char*) pMsg->GetDataPtr() ;
	            p[2] =0 ;
	            p[3] =0;//result
		     memcpy(((UINT8 *)pMsg->GetDataPtr()+sizeof(unsigned short)+2),(unsigned char*)rf_ver,sizeof(rf_ver));
		     memcpy(((UINT8 *)pMsg->GetDataPtr()+92),((UINT8 *)rf_ver2/*temp_buffer*//*result_msg*//*+sizeof(unsigned short)*/),sizeof(rf_ver2));	
		     memcpy(((UINT8 *)pMsg->GetDataPtr()+180),&DSB_SYN_ver[22],22);//syn
		     memcpy(((UINT8 *)pMsg->GetDataPtr()+180+22),&DSB_SYN_ver[0],22);//DSB
	     }
	     
	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }

}

void CWRRU::RRU_RRUVERToEMS(T_WRRU_Ver *result_msg,UINT16 MsgID)
{
            CComMessage *pMsg = NULL;
            unsigned char a,b,c,d;
            unsigned char *p;
     	     pMsg = new (this, sizeof(T_WRRU_Ver)+2) CComMessage;
	if(pMsg==NULL)
	{
	    return;
	}
            p =(unsigned char *)pMsg->GetDataPtr();
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgID);
	     p[0] =(result_msg->transID)>>8;
	     p[1] =result_msg->transID;//result;
	     p[2] =0;
	     p[3] = 0;
         //   memcpy(p+2,result_msg, sizeof(T_WRRU_Ver));
            p[2] =0;
	     p[3] = 0;
	
        a = result_msg->DSB_HW_version;
        b= (unsigned char )(result_msg->DSB_HW_version>>8);
	p[4] = a;
	p[5] =b;
	// printf("DSB_HW_version:.%02x.%02x\n",a,b);
	 a= result_msg->MCU_SW_version;
	 b =(unsigned char)(result_msg->MCU_SW_version>>8);
	 c =(unsigned char) (result_msg->MCU_SW_version>>16);
	 d =(unsigned char ) (result_msg->MCU_SW_version>>24);
	 p[6] = a;
	 p[7] = b;
	 p[8] =c;
	 p[9] = d;
	 printf("MCU_SW_version:%02x.%02x.%02x.%02x\n",a,b,c,d);
	 a=result_msg->FPGA_SW_version;
	 b =(unsigned char) (result_msg->FPGA_SW_version>>8);
	 c =(unsigned char) (result_msg->FPGA_SW_version>>16);
	 d =(unsigned char ) (result_msg->FPGA_SW_version>>24);
	 p[10] = a;
	 p[11] =b;
	 p[12] = c;
	 p[13] =d ;
	 printf("FPGA_SW_version:%02x.%02x.%02x.%02x\n",a,b,c,d);
	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }

}

//2 //-trandid
//2//-current
//temp
//fiberdelay
void CWRRU::RRU_HwStatusToEMS(CComMessage *InMsg/*T_WRRU_HW_STATUS *result_msg*/,UINT16 MsgID)
{
#if 1
           CComMessage *pMsg = NULL;
//            unsigned char a,b,c,d;
            unsigned char *p,*pdata;
	     unsigned short temp;
	  //   unsigned short _temp;
		 unsigned short fiber_delay;
     	     pMsg = new (this, sizeof(T_WRRU_HW_STATUS)+2) CComMessage;
	if(pMsg==NULL)
	{
	    return;
	}
            p =(unsigned char *)pMsg->GetDataPtr();
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgID);
		 pdata = (unsigned char*)InMsg->GetDataPtr();
		 p[0] = pdata[4];
		 p[1] = pdata[5];//transid
		 p[2] =0;
	       p[3] = 0;
		   #if 0
		 p[4]  = pdata[8];
		 p[5]  = pdata[9];//current
		 p[6]  = pdata[7];
		 p[7]  = pdata[6];//temp
		 #endif
		 temp =  pdata[9]*0x100+pdata[8];
		 temp = temp*2;
		 p[4]  = pdata[9];
		 p[5]  = pdata[8];//current
              p[4] = temp>>8;
		  p[5] = temp;
		 p[6]  = pdata[6];
		 p[7]  = pdata[7];//temp
	      fiber_delay = Read_Fiber_Delay(0);
	 	p[8] = fiber_delay>>8;
	 	p[9] = fiber_delay;//fiber delay
	 	p[10] = g_RRU_Freq_Offset>>8;
	 	p[11] = g_RRU_Freq_Offset;//fiber delay
#if 0
	     p[0] =(result_msg->transID)>>8;
	     p[1] =result_msg->transID;//result;
	     p[2] =0;
	     p[3] = 0;
  
	 a= (unsigned char )result_msg->DSB_HW_Current>>8;
	 b =(unsigned char)(result_msg->DSB_HW_Current);

	 p[4] = a;
	 p[5] = b;
       
	 printf("DSB_HW_Current:%02x.%02x\n",a,b);
	  a = result_msg->DSB_HW_Temp;
        b= (unsigned char )(result_msg->DSB_HW_Temp>>8);
	p[6] = b;//a=0xe0 b=0x24
	p[7] =a;//
	temp = (result_msg->DSB_HW_Temp&0x7fff)>>6; //先将高位置0，只取9bit
	if((result_msg->DSB_HW_Temp)&0x8000)
	{
	    _temp = (temp -512)/4;
	}
	else
	{
	    _temp = temp/4;
	}
	 printf("DSB_HW_Temp:.%02x.%02x,%d\n",a,b,_temp);

	 p[8] =0;
	 p[9] = 0;
	 p[10] = 0;
	 p[11] = 0;
	
#endif
	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }
#endif
}

void CWRRU::RRU_RRUSPFToEMS(T_SPF_Ver_Msg *result_msg,UINT16 MsgID)
{
            CComMessage *pMsg = NULL;
     	     pMsg = new (this, 98+2) CComMessage;
	   if(pMsg==NULL)
	{
	    return;
	}
            unsigned char *p;
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgID);
	     p =(unsigned char*)pMsg->GetDataPtr();
	     p[2] =0;
	     p[3] =0;//result;
	     p[0] = ((result_msg->transID)>>8);
	     p[1] = (result_msg->transID);
            memcpy(p+4,(unsigned char*)(&(result_msg->Value)), 96);
	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }

}

void CWRRU::RRU_ProceedQueryVer(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth)
{
	OAM_LOGSTR1(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] Query Ver:%x...",MsgId);
	if(InMsg!=NULL)
		{
      			 unsigned short *pData=(unsigned short *)((unsigned char*)InMsg->GetDataPtr());
      			 m_transid = *pData;
		}

	CComMessage *pComMsg = NULL;
	pComMsg = new (this, 112) CComMessage;
	  	if(pComMsg==NULL)
	{
	    return;
	}
	pComMsg->SetDstTid(M_TID_L2MAIN);
	pComMsg->SetSrcTid(M_TID_WRRU);
	pComMsg->SetMessageId(Type_L3_WRRU);
	pComMsg->SetMoudlue(0);

	WRRU_message* p=(WRRU_message*)pComMsg->GetDataPtr();
	p->src=BBU_No;
	p->des=data_WRRU.WRRU_Num;
	p->type=MsgId;
	p->length=Lenth;

	
	UINT16 *pRes=(UINT16*)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message));
	
	*pRes=m_transid/**pData*/;
	
	UINT16 *pResCRC=(UINT16 *)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(UINT16));

       unsigned short length=p->length/2+2;
	*pResCRC=CheckSum((unsigned short *) p,length);
       change_bytes((unsigned char*)pComMsg->GetDataPtr());
         CComEntity::PostEntityMessage(pComMsg);     

}
 void CWRRU::RRU_ProcessQueryHwStatus(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth)
 {
       OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] Query Hw status...");
       unsigned short *pData=(unsigned short *)((unsigned char*)InMsg->GetDataPtr());

	CComMessage *pComMsg = NULL;
	pComMsg = new (this, 112) CComMessage;
	if(pComMsg==NULL)
	{
	    return;
	}
	pComMsg->SetDstTid(M_TID_L2MAIN);
	pComMsg->SetSrcTid(M_TID_WRRU);
	pComMsg->SetMessageId(Type_L3_WRRU);
	pComMsg->SetMoudlue(0);

	WRRU_message* p=(WRRU_message*)pComMsg->GetDataPtr();
	p->src=BBU_No;
	p->des=data_WRRU.WRRU_Num;
	p->type=MsgId;
	p->length=Lenth;

	
	UINT16 *pRes=(UINT16*)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message));
	*pRes=*pData;
	UINT16 *pResCRC=(UINT16 *)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(UINT16));

       unsigned short length=p->length/2+2;
	*pResCRC=CheckSum((unsigned short *) p,length);
       change_bytes((unsigned char*)pComMsg->GetDataPtr());
         CComEntity::PostEntityMessage(pComMsg);     
 
 }
void CWRRU::RRU_ProcessCaliHW(CComMessage *pComMsg)
{
       unsigned char change_array[104],pp[104];
        int i;
        CComMessage *pComMsg1 = NULL;
	pComMsg1 = new (this, 112) CComMessage;
	if(pComMsg1==NULL)
	{
	    return;
	}
 	WRRU_message* p=(WRRU_message*)((unsigned char*)pComMsg1->GetDataPtr());
	char *pMsgWRRU=(char*)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message));

	unsigned short Bag_no=*pMsgWRRU;//低位
	pMsgWRRU++;//高位
	Bag_no=(*pMsgWRRU<<8)|Bag_no;
	pMsgWRRU++;
	
        memcpy(change_array,pMsgWRRU,104);
        for(i = 0; i < 52; i++)
        {
             pp[2*i] = change_array[2*i+1];
             pp[2*i+1] = change_array[2*i];
        }
	memcpy(&(Calibration_HW_Data[104*(Bag_no)]), pp, 104);
	unsigned int isvalid = 0x55aa55aa;
	if(Bag_no<11)
	{
	 l3rrubspNvRamWrite((char*)&(NvRamData->Cal_Para.IsValid) ,(char *)&isvalid,4);//保存硬件参数
        l3rrubspNvRamWrite(&(NvRamData->Cal_Para.Para[104*(Bag_no)]) ,(char*)pp,104);
	}
	else
	{
		    l3rrubspNvRamWrite(&(NvRamData->Cal_Para_Seccond[104*(Bag_no-11)]) ,(char*)pp,104);
	}
	pComMsg1->SetDstTid(M_TID_L2MAIN);
	pComMsg1->SetSrcTid(M_TID_WRRU);
	pComMsg1->SetMessageId(Type_L3_WRRU);
	pComMsg1->SetMoudlue(0);

	p->des=data_WRRU.WRRU_Num;
	p->src=BBU_No;
	p->type=Type_Get_WRRU_Calibration_HW;
	p->length=0x02;

	pMsgWRRU=(char*)(((unsigned char*)pComMsg1->GetDataPtr())+sizeof(WRRU_message));
	*pMsgWRRU=0x01;//succ
	pMsgWRRU++;
	*pMsgWRRU=(unsigned char)Bag_no;
	pMsgWRRU++;

	unsigned short length=p->length/2+2;
	UINT16 *pResCRC=(UINT16 *)(((unsigned char*)pComMsg1->GetDataPtr())+sizeof(WRRU_message)+sizeof(UINT16));

	*pResCRC=CheckSum((unsigned short *) p,length);
	   change_bytes((unsigned char*)pComMsg1->GetDataPtr());
	CComEntity::PostEntityMessage(pComMsg1);
/*
	if (Bag_no==Max_Calibration_HW_Bag)//测试代码，只是测流程使用，正常流程收到Reset消息，才发送给aux
	{
              CComMessage* pMsgHWToL2=new(this, sizeof(Calibration_HW_Data)) CComMessage;
		pMsgHWToL2->SetDstTid(M_TID_L2MAIN);
		pMsgHWToL2->SetSrcTid(M_TID_WRRU);
		pMsgHWToL2->SetMessageId(M_L1_Calibration_HW);
		pMsgHWToL2->SetMoudlue(0);
		pMsgHWToL2->SetDataLength(sizeof(Calibration_HW_Data));
              memcpy((pMsgHWToL2->GetDataPtr()), &Calibration_HW_Data, sizeof(Calibration_HW_Data));
		CComEntity::PostEntityMessage(pMsgHWToL2);

	}
*/
}

void CWRRU::RRU_ProceedRsvRRU(CComMessage *pComMsg1,unsigned char MsgId,unsigned char Lenth)
{
          		CComMessage *pComMsg = NULL;
		pComMsg = new (this, 112) CComMessage;
		if(pComMsg==NULL)
			return;
	pComMsg->SetDstTid(M_TID_L2MAIN);
	pComMsg->SetSrcTid(M_TID_WRRU);
	pComMsg->SetMessageId(Type_L3_WRRU);
	pComMsg->SetMoudlue(0);

	WRRU_message* p=(WRRU_message*)pComMsg->GetDataPtr();
	p->src=BBU_No;
	p->des=data_WRRU.WRRU_Num;
	p->type=MsgId;
	p->length=Lenth;

	
	UINT16 *pRes=(UINT16*)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message));
	*pRes=0x00;
	UINT16 *pResCRC=(UINT16 *)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(UINT16));
       unsigned short length=p->length/2+2;
	*pResCRC=CheckSum((unsigned short *) p,length);
       change_bytes((unsigned char*)pComMsg->GetDataPtr());
         CComEntity::PostEntityMessage(pComMsg);     
}

void CWRRU:: RRU_ProceedConfigRRUToRRU(CComMessage *pComMsg1,unsigned char MsgId,unsigned char Lenth)
{
     		CComMessage *pComMsg = NULL;
		pComMsg = new (this, 112) CComMessage;
		if(pComMsg==NULL)
			return;
      WRRU_message* RRU_Head=(WRRU_message*)((unsigned char*)pComMsg->GetDataPtr());
      T_ConfigWRRU_msg* msg=(T_ConfigWRRU_msg*)(((unsigned char*)pComMsg->GetDataPtr())+sizeof(WRRU_message));
	  
	pComMsg->SetDstTid(M_TID_L2MAIN);
	pComMsg->SetSrcTid(M_TID_WRRU);
	pComMsg->SetMessageId(Type_L3_WRRU);
	pComMsg->SetMoudlue(0);
	
	RRU_Head->src=BBU_No;
	RRU_Head->des=data_WRRU.WRRU_Num;
	RRU_Head->type=Type_Config_WRRU;
	RRU_Head->length=0x2a;

	msg->StartFreqIndex=NvRamData->WRRUCfgEle.StartFreqindex;  //根据此确定WRRU的工作频点
	msg->TimeSlotNum=NvRamData->WRRUCfgEle.TDD_timeslot;
	msg->DLTSNum=NvRamData->WRRUCfgEle.DLSTNum;
	msg->Syn_TxGain=NvRamData->WRRUCfgEle.TXGAIN_SYN;
	msg->Syn_RxGain=NvRamData->WRRUCfgEle.RXGAIN_SYN;
	memcpy(&(msg->RF_TxGain), &(NvRamData->WRRUCfgEle.TXGAIN_RFB), 8);
	memcpy(&(msg->RF_RxGain), &(NvRamData->WRRUCfgEle.RXGAIN_RFB), 8);
	msg->RFPOWERONOFF=NvRamData->WRRUCfgEle.TRSYN_Power_SW;
	msg->SYNPOWERONOFF=(NvRamData->WRRUCfgEle.TRSYN_Power_SW)>>8;
	msg->Max_Temp=NvRamData->WRRUCfgEle.Max_Temp;//环境参数
	msg->Min_Temp=NvRamData->WRRUCfgEle.Min_Temp;
	msg->Max_Current=NvRamData->WRRUCfgEle.Max_current;
	msg->Min_Current=NvRamData->WRRUCfgEle.Min_current;
	msg->TDDSRC=NvRamData->WRRUCfgEle.TDD10ms_Sel;  
	msg->DbgCtrl=NvRamData->WRRUCfgEle.DbgCtrl;

       unsigned short length=RRU_Head->length/2+2;
	msg->CRC=CheckSum((unsigned short *) RRU_Head,length);
	
	   change_bytes((unsigned char*)pComMsg->GetDataPtr());
	CComEntity::PostEntityMessage(pComMsg);

}

void CWRRU::RRU_ProcessCalibratEmsMsg(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth)
{
              T_Calibrat_Ems_Msg *pData=(T_Calibrat_Ems_Msg *)((unsigned char*)InMsg->GetDataPtr());
			  
		CComMessage *pMsg = NULL;
		pMsg = new (this, 112) CComMessage;
		if(pMsg==NULL)
	{
	    return;
	}
              pMsg->SetDstTid(M_TID_L2MAIN);
              pMsg->SetSrcTid(M_TID_WRRU);
   	       pMsg->SetMessageId(Type_L3_WRRU);
   	       pMsg->SetMoudlue(0);
	 //      memcpy(Byte * dest, Byte * src, uInt nBytes)

		WRRU_message* p=(WRRU_message*)pMsg->GetDataPtr();
		p->src=BBU_No;
		p->des=data_WRRU.WRRU_Num;
		p->type=MsgId;
		p->length=Lenth;
		
         	T_ConfigCalibrationData_Msg *pCalibrat=(T_ConfigCalibrationData_Msg*)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message));
		pCalibrat->transID=pData->transID;
		pCalibrat->RF_RxGain[0]=pData->TR_Cali[0].RxGain;
		pCalibrat->RF_RxGain[1]=pData->TR_Cali[1].RxGain;
		pCalibrat->RF_RxGain[2]=pData->TR_Cali[2].RxGain;
		pCalibrat->RF_RxGain[3]=pData->TR_Cali[3].RxGain;
		pCalibrat->RF_RxGain[4]=pData->TR_Cali[4].RxGain;
		pCalibrat->RF_RxGain[5]=pData->TR_Cali[5].RxGain;
		pCalibrat->RF_RxGain[6]=pData->TR_Cali[6].RxGain;
		pCalibrat->RF_RxGain[7]=pData->TR_Cali[7].RxGain;
		pCalibrat->RF_TxGain[0]=pData->TR_Cali[0].TxGain;
		pCalibrat->RF_TxGain[1]=pData->TR_Cali[1].TxGain;
		pCalibrat->RF_TxGain[2]=pData->TR_Cali[2].TxGain;
		pCalibrat->RF_TxGain[3]=pData->TR_Cali[3].TxGain;
		pCalibrat->RF_TxGain[4]=pData->TR_Cali[4].TxGain;
		pCalibrat->RF_TxGain[5]=pData->TR_Cali[5].TxGain;
		pCalibrat->RF_TxGain[6]=pData->TR_Cali[6].TxGain;
		pCalibrat->RF_TxGain[7]=pData->TR_Cali[7].TxGain;
		pCalibrat->SYN_RxGain=pData->SYN_RxGain;
		pCalibrat->SYN_TxGain=pData->SYN_TxGain;

//temp
//		memcpy(&(NvRamData->WRRUCfgEle.RXGAIN_RFB),&(pCalibrat->RF_RxGain),8);
//		memcpy(&(NvRamData->WRRUCfgEle.TXGAIN_RFB),&(pCalibrat->RF_TxGain),8);
//		NvRamData->WRRUCfgEle.RXGAIN_SYN=pCalibrat->SYN_RxGain;
//		NvRamData->WRRUCfgEle.TXGAIN_SYN=pCalibrat->SYN_TxGain;
		l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.RXGAIN_RFB) ,(char *)(pCalibrat->RF_RxGain),8);
             l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.TXGAIN_RFB) ,(char *)(pCalibrat->RF_TxGain),8);
            l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.RXGAIN_SYN) ,(char *)(&(pCalibrat->SYN_RxGain)),2);
             l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.TXGAIN_SYN) ,(char *)(&(pCalibrat->SYN_TxGain)),2);

             unsigned short length=p->length/2+2;
 		UINT16 *pSwitchCRC=(UINT16 *)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(T_ConfigCalibrationData_Msg));
		*pSwitchCRC=CheckSum((unsigned short *) p,length);
                 change_bytes((unsigned char*)pMsg->GetDataPtr());
		CComEntity::PostEntityMessage(pMsg);     
			
              InMsg->SetDstTid(M_TID_CM);
		CComEntity::PostEntityMessage(InMsg);     

}


void CWRRU::RRU_ProcessCarrierEmsMsg(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth)
{
#if 1
              T_Carrier_Ems_Msg*pData=(T_Carrier_Ems_Msg *)((unsigned char*)InMsg->GetDataPtr());
			  
		CComMessage *pMsg = NULL;
		pMsg = new (this, 112) CComMessage;
			if(pMsg==NULL)
	{
	    return;
	}
              pMsg->SetDstTid(M_TID_L2MAIN);
              pMsg->SetSrcTid(M_TID_WRRU);
   	       pMsg->SetMessageId(Type_L3_WRRU);
   	       pMsg->SetMoudlue(0);
	 //      memcpy(Byte * dest, Byte * src, uInt nBytes)

		WRRU_message* p=(WRRU_message*)((unsigned char*)pMsg->GetDataPtr());
		p->src=BBU_No;
		p->des=data_WRRU.WRRU_Num;
		p->type=MsgId;
		p->length=Lenth;
	 //   l3rrubspNvRamWrite();		
         	T_ConfigTimeSlot_Msg *pSlot=(T_ConfigTimeSlot_Msg*)((unsigned char*)(pMsg->GetDataPtr())+sizeof(WRRU_message));
		pSlot->transID=pData->transID;
		pSlot->TimeSlotNum=pData->TotalSlotN;
		pSlot->DLTSNum=pData->DownSlotN;
              l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.TDD_timeslot) ,(char *)(&pData->TotalSlotN),1);
             l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.DLSTNum) ,(char *)(&pData->DownSlotN),1);

//		bspNvRamWrite((char *)NvRamData->WRRUCfgEle.TDD_timeslot,(char *)pData->TotalSlotN,sizeof(pData->TotalSlotN));
//		NvRamData->WRRUCfgEle.TDD_timeslot=pData->TotalSlotN;
//		NvRamData->WRRUCfgEle.DLSTNum=pData->DownSlotN;

 		UINT16 *pSwitchCRC=(UINT16 *)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(T_ConfigTimeSlot_Msg));

              unsigned short length=p->length/2+2;
     		*pSwitchCRC=CheckSum((unsigned short *) p,length);
               change_bytes((unsigned char*)pMsg->GetDataPtr());
		CComEntity::PostEntityMessage(pMsg);     
		#endif	
              InMsg->SetDstTid(M_TID_CM);
		CComEntity::PostEntityMessage(InMsg);     

}

void CWRRU::RRU_ProcessRFCFGEmsMsg(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth)
{

#if 0
 unsigned int  IsValid;
     unsigned int FPGA_Version;
     unsigned int StartFreqindex;//ok
     unsigned char TDD_timeslot;//ok
     unsigned char DLSTNum;//ok
     unsigned short TDD10ms_Sel; //10ms source select
     ////////Set SYN & RF
     unsigned char RXGAIN_RFB[8];//射频增益
     unsigned char TXGAIN_RFB[8];
     unsigned short RXGAIN_SYN;//SYN增益
     unsigned short TXGAIN_SYN;
     unsigned short count_30M_10s;//频偏，计算TcoOffset
     unsigned short TRSYN_Power_SW;//SYN&RF Power ON/OFF	
     ///////////////threshold
     unsigned char Max_Temp; 
     unsigned char Min_Temp; 
     unsigned int Max_current;
     unsigned int Min_current;
     ////////////////not in use currently	
     unsigned short DbgCtrl;
     #endif
#if 1
              T_PLL_Msg *pData=(T_PLL_Msg *)((unsigned char*)InMsg->GetDataPtr());
			  
		CComMessage *pMsg = NULL;
		pMsg = new (this, 112) CComMessage;
			if(pMsg==NULL)
	{
	    return;
	}
              pMsg->SetDstTid(M_TID_L2MAIN);
              pMsg->SetSrcTid(M_TID_WRRU);
   	       pMsg->SetMessageId(Type_L3_WRRU);
   	       pMsg->SetMoudlue(0);
	 //      memcpy(Byte * dest, Byte * src, uInt nBytes)

		WRRU_message* p=(WRRU_message*)((unsigned char*)pMsg->GetDataPtr());
		p->src=BBU_No;
		p->des=data_WRRU.WRRU_Num;
		p->type=MsgId;
		p->length=Lenth;
		
         	T_PLL_Msg *pPLL=(T_PLL_Msg*)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message));
              pPLL->PLL=pData->PLL;
		pPLL->transID=pData->transID;

//temp
		//NvRamData->WRRUCfgEle.StartFreqindex=pData->PLL;
           //  NvRamData->WRRUCfgEle.IsValid = 0x55aa55aa;
             unsigned int  isValid = 0x55aa55aa;
            l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.IsValid) ,(char *)(&isValid),4);
             l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.StartFreqindex) ,(char *)(&pData->PLL),4);

 		UINT16 *pSwitchCRC=(UINT16 *)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(T_PLL_Msg));

              unsigned short length=p->length/2+2;
		*pSwitchCRC=CheckSum((unsigned short *) p,length);
           //   FPGA_download
               change_bytes((unsigned char*)pMsg->GetDataPtr());
		CComEntity::PostEntityMessage(pMsg);     
		#endif	
     InMsg->SetDstTid(M_TID_CM);
		CComEntity::PostEntityMessage(InMsg);     

}
void CWRRU::RRU_ProceedSYNSwithRRU(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth)
{

	CComMessage *pMsg = NULL;
	pMsg = new (this, 112) CComMessage;
		if(pMsg==NULL)
	{
	    return;
	}
	pMsg->SetDstTid(M_TID_L2MAIN);
	pMsg->SetSrcTid(M_TID_WRRU);
	pMsg->SetMessageId(Type_L3_WRRU);
	pMsg->SetMoudlue(0);

	WRRU_message* p=(WRRU_message*)((unsigned char*)pMsg->GetDataPtr());
	p->src=BBU_No;
	p->des=data_WRRU.WRRU_Num;
	p->type=MsgId;
	p->length=Lenth;


	T_ConfigSYNSwitchEms_Msg* pData=(T_ConfigSYNSwitchEms_Msg*)((unsigned char*)InMsg->GetDataPtr());
	
	T_ConfigGeneral_Msg *pSwitch=(T_ConfigGeneral_Msg*)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message));

/*
	if ((pData->status)==0x00)
		NvRamData->WRRUCfgEle.TRSYN_Power_SW=(NvRamData->WRRUCfgEle.TRSYN_Power_SW)|0x0100;
	else
		NvRamData->WRRUCfgEle.TRSYN_Power_SW=(NvRamData->WRRUCfgEle.TRSYN_Power_SW)&0x00FF;
*/

	pSwitch->transID=pData->transID;
       pSwitch->RFPOWERONOFF=0xff-pData->RFPOWERONOFF;
	pSwitch->SYNPOWERONOFF=0xff-pData->SYNPOWERONOFF;
	pSwitch->SYNSRC=pData->SYNSRC;
#if 0
	pSwitch->SYNPOWERONOFF=(NvRamData->WRRUCfgEle.TRSYN_Power_SW)>>8;
	pSwitch->RFPOWERONOFF=NvRamData->WRRUCfgEle.TRSYN_Power_SW;	
	pSwitch->SYNSRC=NvRamData->WRRUCfgEle.TDD10ms_Sel;
#endif
	UINT16 *pSwitchCRC=(UINT16 *)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(T_ConfigGeneral_Msg));

       unsigned short length=p->length/2+2;
	*pSwitchCRC=CheckSum((unsigned short *) p,length);
        change_bytes((unsigned char*)pMsg->GetDataPtr());

      CComEntity::PostEntityMessage(pMsg);     


}

void CWRRU::RRU_RRUResToEMS(T_RSV_Msg *result_msg,UINT16 MsgID)
{
            CComMessage *pMsg = NULL;
     	     pMsg = new (this, sizeof(T_RSV_Msg)) CComMessage;
			if(pMsg==NULL)
	{
	    return;
	}
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgID);
	     
            memcpy((pMsg->GetDataPtr()),result_msg, sizeof(T_RSV_Msg));
	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }

}

void CWRRU::RRU_ProceedResetRRU(unsigned char MsgId,unsigned char Lenth)
{
	OAM_LOGSTR(LOG_SEVERE, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] Reset WRRU...");

	/*CComMessage *pMsg = NULL;
	pMsg = new (this, 112) CComMessage;
	*/
	RRU_ProceedRsvRRU(NULL, MsgId, Lenth);
}
void CWRRU::RRU_ProceedCirRRU(CComMessage *InMsg,unsigned char MsgId,unsigned char Lenth)
{
	OAM_LOGSTR(LOG_DEBUG3, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] Config Circums Para...");

	CComMessage *pMsg = NULL;
	pMsg = new (this, 112) CComMessage;
		if(pMsg==NULL)
	{
	    return;
	}
	pMsg->SetDstTid(M_TID_L2MAIN);
	pMsg->SetSrcTid(M_TID_WRRU);
	pMsg->SetMessageId(Type_L3_WRRU);
	pMsg->SetMoudlue(0);

	WRRU_message* p=(WRRU_message*)pMsg->GetDataPtr();
	p->src=BBU_No;
	p->des=data_WRRU.WRRU_Num;
	p->type=MsgId;
	p->length=Lenth;

	
	T_ConfigCirCumsPara_Msg *pCirData=(T_ConfigCirCumsPara_Msg*)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message));

	memcpy(pCirData, (InMsg->GetDataPtr()), sizeof(T_ConfigCirCumsPara_Msg));
//temp
//	NvRamData->WRRUCfgEle.Max_Temp=pCirData->Max_Temp;
//	NvRamData->WRRUCfgEle.Min_Temp=pCirData->Min_Temp;
//	NvRamData->WRRUCfgEle.Max_current=pCirData->Max_Current;
//	NvRamData->WRRUCfgEle.Min_current=pCirData->Min_Current;
      l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.Max_Temp) ,(char *)(&(pCirData->Max_Temp)),1);
     l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.Min_Temp) ,(char *)(&(pCirData->Min_Temp)),1);
     l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.Max_current) ,(char *)(&(pCirData->Max_Current)),1);
     l3rrubspNvRamWrite((char*)&(NvRamData->WRRUCfgEle.Min_current) ,(char *)(&(pCirData->Min_Current)),1);
    UINT16 *pCirCRC=(UINT16 *)(((unsigned char*)pMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(T_ConfigCirCumsPara_Msg));

     unsigned short length=p->length/2+2;
    *pCirCRC=CheckSum((unsigned short *) p,length);
        change_bytes((unsigned char*)pMsg->GetDataPtr());

     CComEntity::PostEntityMessage(pMsg);     


}
void CWRRU::RRU_StatusEms(UINT16* tranID,UINT16 MsgId)
{
            CComMessage *pMsg = NULL;
     	     pMsg = new (this, sizeof(T_Status_Ems_Msg)) CComMessage;
			if(pMsg==NULL)
	{
	    return;
	}
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgId);
#if 1
            T_Status_Ems_Msg* pData=(T_Status_Ems_Msg*)((unsigned char*)pMsg->GetDataPtr());
	     pData->transID=*tranID;
	     pData->RSV1 = 0;
	     pData->RSV2 = 0;
	     if (data_WRRU.WRRU_State!=RUNNING)
		 	pData->status=0x01;
	     else
		 	pData->status=0x00;
		 
#endif
	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }

}
void CWRRU::RRU_NvramSYNBackEms(UINT16* tranID,UINT16 MsgId)
{
            CComMessage *pMsg = NULL;
     	     pMsg = new (this, sizeof(T_ConfigSYNSwitchEms_Msg)) CComMessage;
			if(pMsg==NULL)
	{
	    return;
	}
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgId);
	     
            T_ConfigSYNSwitchEms_Msg* pData=(T_ConfigSYNSwitchEms_Msg*)((unsigned char*)pMsg->GetDataPtr());
	     pData->transID=*tranID;
	     pData->RFPOWERONOFF=0xffff-NvRamData->WRRUCfgEle.TRSYN_Power_SW;
	     pData->SYNPOWERONOFF=0xff-(NvRamData->WRRUCfgEle.TRSYN_Power_SW>>8);
	     pData->SYNSRC=NvRamData->WRRUCfgEle.TDD10ms_Sel;
		 
//	     pData->status=(NvRamData->WRRUCfgEle.TRSYN_Power_SW)>>8;

	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }
}

void CWRRU::RRU_NvramCirBackEms(UINT16* tranID,UINT16 MsgId)
{
            CComMessage *pMsg = NULL;
     	     pMsg = new (this, sizeof(T_ConfigCirCumsPara_Msg)) CComMessage;
			if(pMsg==NULL)
	{
	    return;
	}
	     pMsg->SetDstTid(M_TID_EMSAGENTTX);
	     pMsg->SetSrcTid(M_TID_WRRU);
	     pMsg->SetMessageId(MsgId);
	     
            T_ConfigCirCumsPara_Msg* pData=(T_ConfigCirCumsPara_Msg*)((unsigned char*)pMsg->GetDataPtr());
	     pData->transID=*tranID;	     
	     pData->Max_Temp=NvRamData->WRRUCfgEle.Max_Temp;
	     pData->Min_Temp=NvRamData->WRRUCfgEle.Min_Temp;
	     pData->Max_Current=NvRamData->WRRUCfgEle.Max_current;
	     pData->Min_Current=NvRamData->WRRUCfgEle.Min_current;

	     if(true !=  CComEntity::PostEntityMessage(pMsg))     
            {
	        OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
	        pMsg->Destroy();
            }
}

unsigned short CWRRU::CheckSum (unsigned short * msg, unsigned short length)
{
     unsigned short i=0;
     unsigned short c=*msg;
     for (i=0;i<(length -1 );i++)
     {
        //OAM_LOGSTR2(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] calc checksum [0x%04x] [0x%04x]", *msg, c);

     	msg++;
     	c=c^(*msg);
     	
     }
     return c;

}

void CWRRU::print_wrru_state()
{
        	//unsigned char  m_antennaMask;
		//unsigned char m_send_anntennaMask;//表示是否已经发送了RF开关.
		//unsigned char m_rru_state ;
		if(m_rru_state==1)
		{
				printf("m_rru_state is ok\n");
		}
		else
		{
			   	printf("m_rru_state is not ok\n");
		}
		printf("rru anntennaMask:%x,%x\n",m_antennaMask,m_send_anntennaMask);
		
		
}
unsigned short checksum(unsigned short * msg, unsigned short length)
{
     unsigned short i=0;
     unsigned short c=* msg;
     for (i=0;i<(length -1);i++)
     {
     	msg++;
     	c=c^(*msg);
     }
     return c;

}
void testWrru(unsigned short  msgid,UINT8  core,UINT32 times,UINT16 len)
{
     int i = 0;
 
     for (i = 0; i< times; i++)
     	{
     	     	CComMessage *pComMsg = NULL;
     	     		pComMsg = new (CWRRU::GetInstance(), 112) CComMessage;
				if(pComMsg==NULL)
	{
	    return;
	}
     	     		pComMsg->SetDstTid(M_TID_WRRU);
     	     		pComMsg->SetSrcTid(M_TID_L2MAIN);
     	     		pComMsg->SetMoudlue(core);
     	     		pComMsg->SetMessageId(Type_L2_WRRU);
     	     		pComMsg->SetEID(0x12345678);
     	     		WRRU_message*p=(WRRU_message *)pComMsg->GetDataPtr();
     	     		memset((unsigned char*)pComMsg->GetDataPtr(),0,112);
     	     		p->des=BBU_No;
     	     		p->src=0x00;
     	     		p->length=0x02;
     	     		p->type=msgid;
     	     	/*	for(int j = 0;j< 108; j++, p++)
     	     		{
     	     			      *p =0x00;
     	     		}*/
     	     		unsigned char *pFrame = ((UINT8*)(pComMsg->GetDataPtr())+sizeof(WRRU_message));
     	     			pFrame[0] = 0;//transid
		         	pFrame[1] = 0;
                        UINT16 *pCRC;
		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
		           unsigned short length;
		 		 length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
     	     		if(false==CComEntity::PostEntityMessage(pComMsg))
     	     			{
     	     			         printf("hello2\n",0,1,2,3,4,5);
     	     			}
     	     		
     	     	
     	}
}

void testemstowrru(unsigned short msgid,UINT16 len)
{
     CComMessage *pComMsg = NULL;
     pComMsg = new (CWRRU::GetInstance(), len)CComMessage;
	if(pComMsg==NULL)
		{
		   return;
		}
     pComMsg->SetDstTid(M_TID_WRRU);
     pComMsg->SetSrcTid(M_TID_EMSAGENTRX);
     pComMsg->SetMessageId(msgid);
     	     		if(false==CComEntity::PostEntityMessage(pComMsg))
     	     			{
     	     			         printf("hellotestemstowrru\n",0,1,2,3,4,5);
     	     			}
}
/***********************************************************
4.8.1.6   Config_WRRU_PW_Switch
方向：WBBU-〉MCU
功能：WRRU电源开关控制消息
表 Config_WRRU_PW_Switch消息格式
字段名	长度Byte	参考章节	注释
DES	1		0x02
SRC	1		WRRU_No.
Type	1		0x62
Length	1		
TransID	2		
Switch	2		1-掉电 0-无效
CRC	2		

****************************************************************/

extern "C" void ControlRRUPower(unsigned short  flag)
{
         int i = 0;
      unsigned char change_array[112];
 	 unsigned char *ptr;
    UINT16 *pCRC;
    unsigned short length;

     	     	       CComMessage *pComMsg = NULL;
     	     		pComMsg = new (CWRRU::GetInstance(), 112) CComMessage;
			if(pComMsg==NULL)
		{
		   return;
		}
     	     		pComMsg->SetDstTid(M_TID_L2MAIN);
     	     		pComMsg->SetSrcTid(M_TID_WRRU);
     	     		pComMsg->SetMoudlue(0);
     	     		pComMsg->SetMessageId(Type_L3_WRRU);
     	     		pComMsg->SetEID(0x12345678);
     	     		WRRU_message*p=(WRRU_message *)pComMsg->GetDataPtr();
     	     		p->des=0;
     	     		p->src=0x10;
     	     		p->length=0x04;
     	     		p->type=Type_Control_RRU_Power;
     	     		 unsigned char *pFrame = ((UINT8*)(pComMsg->GetDataPtr())+sizeof(WRRU_message));
     	
     	     		      
     	     		
     	     		       pFrame[0] = 0;//transid
		         	pFrame[1] = 0;
		         	pFrame[2] = flag;
		         	pFrame[3] = flag>>8;

		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+4);
		               p->length=0x04;
		 		length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
     	     		
     	     
     	     	
       memcpy(change_array,(unsigned char*)pComMsg->GetDataPtr(),112);
     	   ptr= (unsigned char*)pComMsg->GetDataPtr();
        for(i = 0; i < 56; i++)
        {
             ptr[2*i] = change_array[2*i+1];
             ptr[2*i+1] = change_array[2*i];
        }
     	 if(false==CComEntity::PostEntityMessage(pComMsg))
     	 {
     	     			         printf("hello2\n",0,1,2,3,4,5);
     	 }
     	     		
}
/*****0x10读版本******/
extern "C" void test2Wrru(unsigned short  msgid,UINT8  core,UINT32 times,UINT16 len)
{
     int i = 0;
      unsigned char change_array[112];
 	 unsigned char *ptr;
    UINT16 *pCRC;
    unsigned short length;
     for (i = 0; i< times; i++)
     	{
     	     	       CComMessage *pComMsg = NULL;
     	     		pComMsg = new (CWRRU::GetInstance(), 112) CComMessage;
			if(pComMsg==NULL)
		{
		   return;
		}
     	     		pComMsg->SetDstTid(M_TID_L2MAIN);
     	     		pComMsg->SetSrcTid(M_TID_WRRU);
     	     		pComMsg->SetMoudlue(core);
     	     		pComMsg->SetMessageId(Type_L3_WRRU);
     	     		pComMsg->SetEID(0x12345678);
     	     		WRRU_message*p=(WRRU_message *)pComMsg->GetDataPtr();
     	     		p->des=0;
     	     		p->src=0x10;
     	     		p->length=0x02;
     	     		p->type=msgid;
     	     		 unsigned char *pFrame = ((UINT8*)(pComMsg->GetDataPtr())+sizeof(WRRU_message));
     	     		switch (msgid)
     	     		{
     	     		      case Type_MCUCode_Download:
     	     		      	
		         	pFrame[0] = 0;//transid
		         	pFrame[1] = 0;

		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
		        
		 		 length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
		           
     	     		      	break;
     	     		      	case Type_FPGACode_Download:
     	     		      pFrame[0] = 0;//transid
		         	pFrame[1] = 0;

		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
		        
		 		 length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
     	     		      	break;
     	     		      	case  Type_Reset_WRRU:
     	     		      	pFrame[0] = 0;//transid
		         	pFrame[1] = 0;

		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
		        
		 		length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
     	     		      		break;
     	     		      	case Type_Query_WRRU_Version:
     	     		      	pFrame[0] = 0;//transid
		         	pFrame[1] = 0;

		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
		        
		 		length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
     	     		      	   break;
     	     		      	case Type_Query_WRRU_RF1to4_Version:
     	     		      	case Type_Query_WRRU_RF5to8_Version:
     	     		      	case Type_Query_WRRU_SPF_Version:
     	     		       case Type_WRRU_TEmp_Current:
     	     		       case Type_WRRU_SYN_DSB_Ver:
     	     		      	pFrame[0] = 0;//transid
		         	pFrame[1] = 0;

		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+sizeof(unsigned short));
		        
		 		length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
     	     		      	break;
     	     		      	case Type_Fpga_Register:
     	     		      	break;
			      case  Type_Control_RRU_Power:



			      	break;
					
					
     	     		      	default:
     	     		      	    break;
     	     		      		
     	     		      	
     	     		}
     	     
     	     	
       memcpy(change_array,(unsigned char*)pComMsg->GetDataPtr(),112);
     	   ptr= (unsigned char*)pComMsg->GetDataPtr();
        for(i = 0; i < 56; i++)
        {
             ptr[2*i] = change_array[2*i+1];
             ptr[2*i+1] = change_array[2*i];
        }
     	     		if(false==CComEntity::PostEntityMessage(pComMsg))
     	     			{
     	     			         printf("hello2\n",0,1,2,3,4,5);
     	     			}
     	     		
     	     	
     	}
}
/*****************************

flag ----0 -write,1 -read
addr offset必须为2*

             

*******************/
extern "C" void test2WrruFpga(unsigned char flag,unsigned char addr,unsigned short value)
{
     int i = 0;
      unsigned char change_array[112];
 	 unsigned char *ptr;
    UINT16 *pCRC;
    unsigned short length;

     	     	       CComMessage *pComMsg = NULL;
     	     		pComMsg = new (CWRRU::GetInstance(), 112) CComMessage;
			if(pComMsg==NULL)
		{
		   return;
		}
     	     		pComMsg->SetDstTid(M_TID_L2MAIN);
     	     		pComMsg->SetSrcTid(M_TID_WRRU);
     	     		pComMsg->SetMoudlue(0);
     	     		pComMsg->SetMessageId(Type_L3_WRRU);
     	     		pComMsg->SetEID(0x12345678);
     	     		WRRU_message*p=(WRRU_message *)pComMsg->GetDataPtr();
     	     		p->des=0;
     	     		p->src=0x10;
     	     		p->length=0x04;
     	     		p->type=Type_Fpga_Register;
     	     		 unsigned char *pFrame = ((UINT8*)(pComMsg->GetDataPtr())+sizeof(WRRU_message));
     	
     	     		      
     	     		
     	     		       pFrame[0] = flag;//transid
		         	pFrame[1] = addr;
		         	pFrame[2] = value>>8;
		         	pFrame[3] = value;

		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+4);
		               p->length=0x04;
		 		length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
     	     		
     	     
     	     	
       memcpy(change_array,(unsigned char*)pComMsg->GetDataPtr(),112);
     	   ptr= (unsigned char*)pComMsg->GetDataPtr();
        for(i = 0; i < 56; i++)
        {
             ptr[2*i] = change_array[2*i+1];
             ptr[2*i+1] = change_array[2*i];
        }
     	 if(false==CComEntity::PostEntityMessage(pComMsg))
     	 {
     	     			         printf("hello2\n",0,1,2,3,4,5);
     	 }
     	     		
     	     	
     	
}



/*****************************

flag ----0 ,CSI ，1-fiber module info
seq---序列号

             

*******************/
extern "C" void ReadRRUCSI(unsigned char flag)
{
     int i = 0;
      unsigned char change_array[112];
 	 unsigned char *ptr;
    UINT16 *pCRC;
    unsigned short length;

     	     	       CComMessage *pComMsg = NULL;
     	     		pComMsg = new (CWRRU::GetInstance(), 112) CComMessage;
			if(pComMsg==NULL)
		{
		   return;
		}
     	     		pComMsg->SetDstTid(M_TID_L2MAIN);
     	     		pComMsg->SetSrcTid(M_TID_WRRU);
     	     		pComMsg->SetMoudlue(0);
     	     		pComMsg->SetMessageId(Type_L3_WRRU);
     	     		pComMsg->SetEID(0x12345678);
     	     		WRRU_message*p=(WRRU_message *)pComMsg->GetDataPtr();
     	     		p->des=0;
     	     		p->src=0x10;
     	     		p->length=0x02;
     	     		p->type=Type_CSI_INFO_Read;
     	     		 unsigned char *pFrame = ((UINT8*)(pComMsg->GetDataPtr())+sizeof(WRRU_message));
     	
     	     		      
     	     		
     	     		       pFrame[0] = flag;//transid
		         	pFrame[1] = 0;
		 		pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+2);
		               p->length=0x02;
		 		length=p->length/2+2;
		 		*pCRC=checksum((unsigned short *) p,length);
     	     		
     	     
     	     	
       memcpy(change_array,(unsigned char*)pComMsg->GetDataPtr(),112);
     	   ptr= (unsigned char*)pComMsg->GetDataPtr();
        for(i = 0; i < 56; i++)
        {
             ptr[2*i] = change_array[2*i+1];
             ptr[2*i+1] = change_array[2*i];
        }
     	 if(false==CComEntity::PostEntityMessage(pComMsg))
     	 {
     	     			         printf("hello2\n",0,1,2,3,4,5);
     	 }
     	     		
     	     	
     	
}
extern "C" void diagWrruVer()
{
      test2Wrru(0x10,0,1,112);
}
int ResetWrru_Enable = 1;
extern "C"  void  ResetWRRUDisable()
{
     ResetWrru_Enable = 0;
}
extern "C" void ResetWrru()
{
      if(ResetWrru_Enable ==1)
      	{
      	
        test2Wrru(0x17,0,1,112);
      	}
}

extern "C" void LoadMcu()
{
        test2Wrru(0x20,0,1,112);
}
extern "C" void LoadFpga()
{
        test2Wrru(0x23,0,1,112);
}

extern "C" void diagRFVer()
{
      
      test2Wrru(0x19,0,1,112);
      taskDelay(1);
      test2Wrru(0x1a,0,1,112);
}

extern "C" void diagSPFVer()
{
      test2Wrru(0x1b,0,1,112);
}
extern "C" void WrruRFC(unsigned char antennamask,unsigned char flag,unsigned char flag1)
{
#if 0
     int i = 0;
      unsigned char change_array[112];
 	 unsigned char *ptr;
    UINT16 *pCRC;
    unsigned short length;
   
   CComMessage *pComMsg = NULL;
   pComMsg = new (CWRRU::GetInstance(), 112) CComMessage;
   pComMsg->SetDstTid(M_TID_L2MAIN);
  pComMsg->SetSrcTid(M_TID_WRRU);
   pComMsg->SetMoudlue(0);
   pComMsg->SetMessageId(Type_L3_WRRU);
   pComMsg->SetEID(0x12345678);
   WRRU_message*p=(WRRU_message *)pComMsg->GetDataPtr();
   p->des=0;
   p->src=0x10;
   p->length=0x06;
   p->type=0x14;
   unsigned char *pFrame = ((UINT8*)(pComMsg->GetDataPtr())+sizeof(WRRU_message));   	
	pFrame[0] = 0;//transid
	pFrame[1] = 0;
        pFrame[2] = 1;//BBU同步；
	pFrame[3] = 0;
	pFrame[4] = antennamask;//
	 pFrame[5] = 1;// syn power open constly
	pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+6);
		        
	length=p->length/2+2;
	*pCRC=checksum((unsigned short *) p,length);
   	
       memcpy(change_array,(unsigned char*)pComMsg->GetDataPtr(),112);
     	   ptr= (unsigned char*)pComMsg->GetDataPtr();
        for(i = 0; i < 56; i++)
        {
             ptr[2*i] = change_array[2*i+1];
             ptr[2*i+1] = change_array[2*i];
        }
     	 if(false==CComEntity::PostEntityMessage(pComMsg))
     	 {
     	     	printf("hello2\n",0,1,2,3,4,5);
     	  }
     	     		
    #endif 	

//	 int i = 0;
     
 	 unsigned char *ptr;
   
    
   
   CComMessage *pComMsg = NULL;
   pComMsg = new (CWRRU::GetInstance(), 4) CComMessage;
   if(pComMsg==NULL)
		{
		   return;
		}
   pComMsg->SetDstTid(M_TID_WRRU);
  pComMsg->SetSrcTid(M_TID_WRRU);
   pComMsg->SetMoudlue(0);
   pComMsg->SetMessageId(M_OTHER2_WRRU);
   pComMsg->SetEID(0x12345678);
   ptr = (unsigned char *)pComMsg->GetDataPtr();
   ptr[0] = antennamask;
   ptr[1] = flag;
   ptr[2] = flag1;
   ptr[3] = flag1;
     	 if(false==CComEntity::PostEntityMessage(pComMsg))
     	 {
     	     	printf("hello2\n",0,1,2,3,4,5);
     	  }
     	
}
extern "C" void WRRU_Start_Cfg_Timer()
{
           static CWRRU *pCWRRUInstance = NULL;
           pCWRRUInstance = CWRRU::GetInstance();
           pCWRRUInstance->RRU_Start_CFG_Timer();
}
extern "C"  void Cfg_Annntena_mask(unsigned char anntenamask,unsigned char flag)
{
      static CWRRU *pCWRRUInstance = NULL;
           pCWRRUInstance = CWRRU::GetInstance();
           pCWRRUInstance->RRU_CFG_RFMask(anntenamask,flag);
}
extern "C" void SendHardWarePara()
{
           static CWRRU *pCWRRUInstance = NULL;
           pCWRRUInstance = CWRRU::GetInstance();
           if(NvRamData->Cal_Para.IsValid == 0x55aa55aa)
           	{
           	   pCWRRUInstance->RRU_SendCaliHWToAUX(1);
           	}
             else
             	{
             	   printf("there is no hardware par\n");
             	}
}
void CWRRU::l3rrubspNvRamWrite(char * TargAddr, char *ScrBuff, int size)
{


   if((ScrBuff==NULL)||(size==0)||(TargAddr==NULL))
   	{
   	       return ;
   	}
    semTake(s_semWriteNvram, WAIT_FOREVER);
    bspNvRamWrite(TargAddr, ScrBuff, size);
    semGive(s_semWriteNvram);
  return ;

    

}
extern "C" void l3oamprintWrrudata()
{
	int i;
          static CWRRU *pCWRRUInstance = NULL;
           pCWRRUInstance = CWRRU::GetInstance();
       printf("\r\nWrru GEN CFG:####################################");

    printf("\r\nStartFreqIndex     = %04X", NvRamData->WRRUCfgEle.StartFreqindex);
    printf("\r\nTimeSlotNum   = %04X", NvRamData->WRRUCfgEle.TDD_timeslot);//
    printf( "\r\nDLTSNum = %04X",NvRamData->WRRUCfgEle.DLSTNum);//
    printf( "\r\nSyn_TxGain = %04X",NvRamData->WRRUCfgEle.TXGAIN_SYN);
    printf("\r\nSyn_RxGain = %04X", NvRamData->WRRUCfgEle.RXGAIN_SYN);
    printf( "\r\nRFPOWERONOFF = %04X",  NvRamData->WRRUCfgEle.TRSYN_Power_SW);
    printf("\r\nMax_Temp = %04X",NvRamData->WRRUCfgEle.Max_Temp);
     printf("\r\nMin_Temp = %04X", NvRamData->WRRUCfgEle.Min_Temp);
     printf("\r\nMax_Current = %04X",   NvRamData->WRRUCfgEle.Max_current);
     printf( "\r\nMin_Current = %04X", NvRamData->WRRUCfgEle.Min_current);
     printf( "\r\nTDDSRC = %04X",NvRamData->WRRUCfgEle.TDD10ms_Sel);
     printf( "\r\nDbgCtrl = %04X",  NvRamData->WRRUCfgEle.DbgCtrl  );
     printf( "\r\nhardware para flag = %04x",NvRamData->Cal_Para.IsValid );
     printf("\r\nWrru temp:=%d",WRRU_Temperature);
     printf( "\r\nFiber Volt Range:= %0d",NvRamData->fiber_para.Voltage);
     printf( "\r\nFiber Current Range:= %0d",NvRamData->fiber_para.Current );
     printf( "\r\nFiber Tx_Power Range:= %0d",NvRamData->fiber_para.Tx_Power );
     printf( "\r\nFiber Rx_Power:= %0d",NvRamData->fiber_para.Rx_Power );
     printf("\r\nRX GAIN:");
    for(i = 0; i < 8; i++)
    	{
    	   printf("%x, ", NvRamData->WRRUCfgEle.RXGAIN_RFB[i]);
    	}
	  printf("\r\nTX GAIN:");
    for(i = 0; i < 8; i++)
    	{
    	   printf("%x, ", NvRamData->WRRUCfgEle.TXGAIN_RFB[i]);
    	}
     pCWRRUInstance->print_wrru_state();
     if(g_RRU_Eprom ==0)
     	{
     	     printf("there is not  eprom in WRRU\n");
     	}
     else 
     	{
     	   printf("there is an  eprom in WRRU\n");
     	}
        
}

unsigned int  get_freq_from_NVRAM()
{
      return NvRamData->RfCfgEle.StartFreqIndex;
}


unsigned char compare_freq()
{
     UINT32 a ,b;
     a = NvRamData->WRRUCfgEle.StartFreqindex;
     b = NvRamData->RfCfgEle.StartFreqIndex;
     if(a==b)
     	{
     	   return 1;
     	}
     else
     	{
     	    return 0;
     	}
}
void test_Nvram(void)
{
   /* char a=0x21;
	bspSetNvRamWrite(((char *)(&(NvRamData->WRRUCfgEle.TDD_timeslot))),&a,sizeof(char));

	    char * start;
	    int len;
	    UINT32 temp;
	    UINT32 pageSize = vmBasePageSizeGet();

	    if ( (UINT32)startAddr < NVRAM_BASE_ADDR || (UINT32)(startAddr+size)>= (NVRAM_BASE_ADDR + NVRAM_SIZE))
	    {
	        logMsg("bspSetNvRamWrite invalid address 0x%x, len=%d, flag=%d\n", (UINT32)startAddr, size,flag,0,0,0);
	        taskSuspend(0);
	        return FALSE;
	    }

	    start = (char *)((UINT32)startAddr & (~ (pageSize- 1)));
	    temp = (UINT32)startAddr & (pageSize-1);
	    len = (temp + size-1) - ((temp + size -1) & (pageSize -1)) + pageSize;
	    if(((unsigned int)start+len) >= (NVRAM_BASE_ADDR + NVRAM_SIZE))
	    {
	        len = len - pageSize;  
	    }

	    if(flag==1)
	    {
	        vmBaseStateSet(NULL, start , len , VM_STATE_MASK_WRITABLE, VM_STATE_WRITABLE);
		}
	    else
	    {
	        vmBaseStateSet(NULL, start , len , VM_STATE_MASK_WRITABLE, VM_STATE_WRITABLE_NOT);
		}


	
	
   semTake(s_semWriteNvram, WAIT_FOREVER);


   bspNvRamWrite(((char *)(&(NvRamData->WRRUCfgEle.TDD_timeslot))),&a,sizeof(char));

  semGive(s_semWriteNvram);
  */
}
void print_rf_state()
{
     printf("calibration close RF(1-close,0-open):%2x\n",Calibration_Antenna);
     printf("gps lose syn (have syned 0-no,1-yes):%d\n",bGpsStatus_RF);
}


extern "C" void ReadRRUFiberInfo(unsigned char flag)
{
      int i = 0;
      unsigned char change_array[112];
 	 unsigned char *ptr;
    UINT16 *pCRC;
    unsigned short length;

       CComMessage *pComMsg = NULL;
	pComMsg = new (CWRRU::GetInstance(), 112) CComMessage;
	if(pComMsg==NULL)
		{
		   return;
		}
	pComMsg->SetDstTid(M_TID_L2MAIN);
	pComMsg->SetSrcTid(M_TID_WRRU);
	pComMsg->SetMoudlue(0);
	pComMsg->SetMessageId(Type_L3_WRRU);
	pComMsg->SetEID(0x12345678);
	WRRU_message*p=(WRRU_message *)pComMsg->GetDataPtr();
	p->des=0;
	p->src=0x10;
	p->length=0x02;
	p->type=Type_Read_RRU_Fiber_INfo;
	 unsigned char *pFrame = ((UINT8*)(pComMsg->GetDataPtr())+sizeof(WRRU_message));

	      
	
	pFrame[0] = 0;//transid
	pFrame[1] = 0;
	pCRC=(UINT16 *)((char*)(pComMsg->GetDataPtr())+sizeof(WRRU_message)+2);
	p->length=0x02;
	length=p->length/2+2;
	*pCRC=checksum((unsigned short *) p,length);
     	     		
     	     
     	     	
       memcpy(change_array,(unsigned char*)pComMsg->GetDataPtr(),112);
     	   ptr= (unsigned char*)pComMsg->GetDataPtr();
        for(i = 0; i < 56; i++)
        {
             ptr[2*i] = change_array[2*i+1];
             ptr[2*i+1] = change_array[2*i];
        }
     	 if(false==CComEntity::PostEntityMessage(pComMsg))
     	 {
     	     	printf("hello2\n",0,1,2,3,4,5);
     	 }
     	 if(flag==1)
     	 	{
     			 g_print_rru_fiber_info = 0;
     	 	}
     	 else
     	 	{
     	 	  g_print_rru_fiber_info = 1;
     	 	}
}
/*************************************

36	reg_SFP_USED_NUM	0x0050	只读	指示在使用哪个光模块
				bit（1:0）： “01”  SFP1
				           “10”  SFP2
				           “11”  SFP3


返回fiber index 

********************************************/
extern "C" unsigned char  ReadFiberno()
{
       unsigned short  p = *(unsigned short*)0xd1000050 ;
       unsigned char  index = p&0x3;
	if(index==0)
	{
	    return 0;
	}
	else
	{
	    return index-1;
	}
	
}
/*******************************
给fpga寄存器设置值，为了防止等待，这里只进行设置，然后fpga执行I2C操作，
如果需要显示的话，则需要执行ShowBBUFiberInfo函数


***************/
/******************************************************************************************

34	reg_SFP_I2C	0x004C	读/写	reg_SFP_I2C(0)：write '0-1' start I2C read 
reg_SFP_I2C(1)：I2C read stuatus,'0' means read process,'1' means read complete
reg_SFP_I2C(3:2)："00"  SFP1
                 "01"  SFP2
                 "10"  SFP3
reg_SFP_I2C(15:4)：reserved
35	reg_SFP_Diag	0x004E	只读	光模块诊断数据
******************************************************************************************/
extern "C" void ReadBBUFiberInfo(unsigned char fiber_no)
{
        if(fiber_no>3)
        {
           printf("fiber_no Large Than 3 (input :%d)\n",fiber_no);
           return;
        }
        *(unsigned short*)0xd100004c = (fiber_no<<2) + 0;
        for(int i=0; i<100; i++)
        {
        }
         *(unsigned short*)0xd100004c = (fiber_no<<2 )+ 1;
    
}
/**********************************

8	REG_FEP_RX_INT	0x0010	读/写	FEP DSP接收中断选择：X"aaaa"=local fep rx int; X"5555"=5 DSPs no rx int; others=from rru rx link;




flag:0 设知为fpga提供rx int 为fep 程序启动时设置为本地；
       1是从rru   程序正常后再设置从rru 获得rx int 

*****************************************/
extern "C" void SetLocalFepRxInt(unsigned char flag)
{
         //  return;
            if(flag==0)
            	{
	            	 OAM_LOGSTR(LOG_DEBUG1, L3CM_ERROR_PRINT_CFG_INFO, "fpga provide rx int for fep\n");
	              *(unsigned short*)0xd1000010= 0xaaaa;
            	}
            else
            	{
            	      	 OAM_LOGSTR(LOG_DEBUG1, L3CM_ERROR_PRINT_CFG_INFO, "rru provide rx int for fep \n");
            
            	     *(unsigned short*)0xd1000010= 0x0000;
            	}
 
}

/*********************************************************
56-59 4 Rx_PWR(4) Single precision floating point calibration data - Rx optical power. Bit7 of byte 56 is MSB. Bit 0 of byte 59 is LSB. Rx_PWR(4) should be set to zero for “internally calibrated” devices.
60-63 4 Rx_PWR(3) Single precision floating point calibration data - Rx optical power.Bit 7 of byte 60 is MSB. Bit 0 of byte 63 is LSB. Rx_PWR(3) should be set to zero for “internally calibrated” devices.
64-67 4 Rx_PWR(2) Single precision floating point calibration data, Rx optical power.Bit 7 of byte 64 is MSB, bit 0 of byte 67 is LSB. Rx_PWR(2) should be set to zero for “internally calibrated” devices.
68-71 4 Rx_PWR(1) Single precision floating point calibration data, Rx optical power. Bit 7of byte 68 is MSB, bit 0 of byte 71 is LSB. Rx_PWR(1) should be set to 1 for “internally calibrated” devices.
72-75 4 Rx_PWR(0) Single precision floating point calibration data, Rx optical power. Bit 7of byte 72 is MSB, bit 0 of byte 75 is LSB. Rx_PWR(0) should be set to zero for “internally calibrated” devices.
76-77 2 Tx_I(Slope) Fixed decimal (unsigned) calibration data, laser bias current. Bit 7 of byte 76 is MSB, bit 0 of byte 77 is LSB. Tx_I(Slope) should be set to 1 for “internally calibrated” devices.
78-79 2 Tx_I(Offset) Fixed decimal (signed two’s complement) calibration data, laser bias current. Bit 7 of byte 78 is MSB, bit 0 of byte 79 is LSB. Tx_I(Offset) should be set to zero for “internally calibrated” devices.
80-81 2 Tx_PWR(Slope) Fixed decimal (unsigned) calibration data, transmitter coupled output power. Bit 7 of byte 80 is MSB, bit 0 of byte 81 is LSB. Tx_PWR(Slope) should be set to 1 for “internally calibrated” devices.
82-83 2 Tx_PWR(Offset) Fixed decimal (signed two’s complement) calibration data, transmitter coupled output power. Bit 7 of byte 82 is MSB, bit 0 of byte 83 is LSB. Tx_PWR(Offset) should be set to zero for “internally calibrated” devices.
84-85 2 T (Slope) Fixed decimal (unsigned) calibration data, internal module temperature. Bit 7 of byte 84 is MSB, bit 0 of byte 85 is LSB.T(Slope) should be set to 1 for “internally calibrated” devices.
86-87 2 T (Offset) Fixed decimal (signed two’s complement) calibration data, internal module temperature. Bit 7 of byte 86 is MSB, bit 0 of byte 87 is LSB.T(Offset) should be set to zero for “internally calibrated” devices.
88-89 2 V (Slope) Fixed decimal (unsigned) calibration data, internal module supply voltage. Bit 7 of byte 88 is MSB, bit 0 of byte 89 is LSB. V(Slope)should be set to 1 for “internally calibrated” devices.
90-91 2 V (Offset) Fixed decimal (signed two’s complement) calibration data, internal  module supply voltage. Bit 7 of byte 90 is MSB. Bit 0 of byte 91 is LSB. VLSB. V(Offset) should be set to zero for “internally calibrated”devices.

96 All Temperature MSB Internally measured module temperature.
97 All Temperature LSB
98 All Vcc MSB Internally measured supply voltage in transceiver.
99 All Vcc LSB
100 All TX Bias MSB Internally measured TX Bias Current.
101 All TX Bias LSB
102 All TX Power MSB Measured TX output power.
103 All TX Power LSB
104 All RX Power MSB Measured RX input power.
105 All RX Power LSB
106-109 All Unallocated Reserved for future diagnostic definitions


***********************************************************/


/********************************************************************
After calibration per the equations given below for each variable, the results are consistent with
the accuracy and resolution goals for internally calibrated devices.
1) Internally measured transceiver temperature. Module temperature, T, is given by the
following equation: T(C) = Tslope * TAD (16 bit signed twos complement value) + Toffset. The result is
in units of 1/256C, yielding a total range of –128C to +128C. See Table 3.16 for locations of
TSLOPE and TOFFSET. Temperature accuracy is vendor specific but must be better than ±3 degrees
Celsius over specified operating temperature and voltage. Please see vendor specification
sheet for details on location of temperature sensor. Tables 3.13 and 3.14 above give
examples of the 16 bit signed twos complement temperature format.
2) Internally measured supply voltage. Module internal supply voltage, V, is given in microvolts
by the following equation: V(uV) = VSLOPE * VAD (16 bit unsigned integer) + VOFFSET. The result is in
units of 100uV, yielding a total range of 0 – 6.55V. See Table 3.16 for locations of VSLOPE and
VOFFSET. Accuracy is vendor specific but must be better than ±3% of the manufacturer’s nominal
value over specified operating temperature and voltage. Note that in some transceivers,
transmitter supply voltage and receiver supply voltage are isolated. In that case, only one
supply is monitored. Refer to the manufacturer’s specification for more detail.
3) Measured transmitter laser bias current. Module laser bias current, I, is given in microamps
by the following equation: I (uA) = ISLOPE * IAD (16 bit unsigned integer) + IOFFSET. This result is in
units of 2 uA, yielding a total range of 0 to 131 mA. See Table 3.16 for locations of ISLOPE and
IOFFSET. Accuracy is vendor specific but must be better than ±10% of the manufacturer’s nominal
value over specified operating temperature and voltage.
4) Measured coupled TX output power. Module transmitter coupled output power, TX_PWR,
is given in uW by the following equation: TX_PWR (uW) = TX_PWRSLOPE * TX_PWRAD (16 bit
unsigned integer) + TX_PWROFFSET. This result is in units of 0.1uW yielding a total range of 0 –
6.5mW. See Table 3.16 for locations of TX_PWRSLOPE and TX_PWROFFSET. Accuracy is vendor
specific but must be better than ±3dB over specified operating temperature and voltage. Data
is assumed to be based on measurement of a laser monitor photodiode current. It is factory
calibrated to absolute units using the most representative fiber output type. Data is not valid
when the transmitter is disabled.
Published SFF-8472 Rev 11.0
Diagnostic Monitoring Interface for Optical Transceivers Page 30
5) Measured received optical power. Received power, RX_PWR, is given in uW by the
following equation:
Rx_PWR (uW) = Rx_PWR(4) * Rx_PWRAD
4 (16 bit unsigned integer) +
Rx_PWR(3) * Rx_PWRAD
3 (16 bit unsigned integer) +
Rx_PWR(2) * Rx_PWRAD
2 (16 bit unsigned integer) +
Rx_PWR(1) * Rx_PWRAD (16 bit unsigned integer) +
Rx_PWR(0)
The result is in units of 0.1uW yielding a total range of 0 – 6.5mW. See Table 3.16 for
locations of Rx_PWR(4-0). Absolute accuracy is dependent upon the exact optical
wavelength. For the vendor specified wavelength, accuracy shall be better than ±3dB over
specified temperature and voltage. This accuracy shall be maintained for input power levels up
to the lesser of maximum transmitted or maximum received optical power per the appropriate
standard. It shall be maintained down to the minimum transmitted power minus cable plant
loss (insertion loss or passive loss) per the appropriate standard. Absolute accuracy beyond
this minimum required received input optical power range is vendor specific.

const UINT16 ALM_ID_BTS_FIBER_VOL = 0x0105;
const UINT16 ALM_ID_BTS_FIBER_Current = 0x0106;
const UINT16 ALM_ID_BTS_FIBER_TX_PWR = 0x0107;
const UINT16 ALM_ID_BTS_FIBER_RX_PWR = 0x0108;

const UINT16 ALM_ID_RRU_FIBER_VOL = 0x0d09;
const UINT16 ALM_ID_RRU_FIBER_Current = 0x0d0A;
const UINT16 ALM_ID_RRU_FIBER_TX_PWR = 0x0d0B;
const UINT16 ALM_ID_RRU_FIBER_RX_PWR = 0x0d0C;

**********************************************************************/
extern "C" void ShowBBUFiberInfo(unsigned char flag)
{
   float  *Rx_PWR4,*Rx_PWR3,*Rx_PWR2,*Rx_PWR1,*Rx_PWR0;
   short *Tx_I_O,*TX_PWR_O,*T_O,*V_O;//offset;
   float TX_I_S,TX_PWR_S,T_S,V_S;//scope
    short *TX_I_AD,*TX_PWR_AD,*T_AD,*V_AD,*RX_PWR_AD;//AD

    float Temper,Current ,Vol,Tx_pwer,Rx_pwer;
     static  unsigned char BVol_warn_flag = 0;
    static  unsigned char BCurrent_warn_flag = 0;
     static  unsigned char BTx_pwr_warn_flag = 0;
      static  unsigned char BRx_pwr_warn_flag = 0;
//  unsigned char tt_info[256];
    unsigned short  p = *(unsigned short*)0xd100004c ;
    unsigned short  *ptr =(unsigned short*)& g_BBU_Fiber_INfo[0];
//   unsigned short  *ptr =(unsigned short*)& tt_info[0];
    if(p&0x2)
    {
    	for(int k= 0; k < 64;k++)
    	{
    		*ptr = *(unsigned short*)0xd100004e;
    		 ptr++;
    	}
    	*(unsigned short*)0xd100004c = 0;
    }
    else
    {
    	    printf("BBU FPGA Is Not Ready To Get Fiber Module Info\n");
    	    return;
    }
    #if 0
    for(int j =0 ; j < 128; j++)
    {
       // g_BBU_Fiber_INfo[j] = tt_info[j+1];
    	  printf("0x%x, ",g_BBU_Fiber_INfo[j])   ;
    	  if((j>0)&&(j%15==0))
    	  {
    	     printf("\n");
    	  }
    }
    #endif
        if(flag==1)
         {
         	   g_print_bbu_fiber_info = 1;
         }
         else
         	{
     			g_print_bbu_fiber_info = 0;
         	}
    
   /*56-59 4 Rx_PWR(4) Single precision floating point calibration data - Rx optical power. Bit7 of byte 56 is MSB. Bit 0 of byte 59 is LSB. Rx_PWR(4) should be set to zero for “internally calibrated” devices.
60-63 4 Rx_PWR(3) Single precision floating point calibration data - Rx optical power.Bit 7 of byte 60 is MSB. Bit 0 of byte 63 is LSB. Rx_PWR(3) should be set to zero for “internally calibrated” devices.
64-67 4 Rx_PWR(2) Single precision floating point calibration data, Rx optical power.Bit 7 of byte 64 is MSB, bit 0 of byte 67 is LSB. Rx_PWR(2) should be set to zero for “internally calibrated” devices.
68-71 4 Rx_PWR(1) 
*/
   Rx_PWR4 = (float*)(&g_BBU_Fiber_INfo[56]);
   Rx_PWR3 = (float*)(&g_BBU_Fiber_INfo[60]);
   Rx_PWR2 = (float*)(&g_BBU_Fiber_INfo[64]);
   Rx_PWR1 = (float*)(&g_BBU_Fiber_INfo[68]);
   Rx_PWR0 = (float*)(&g_BBU_Fiber_INfo[72]);
   /*********************

76-77 2 Tx_I(Slope) Fixed decimal (unsigned) calibration data, laser bias current. Bit 7 of byte 76 is MSB, bit 0 of byte 77 is LSB. Tx_I(Slope) should be set to 1 for “internally calibrated” devices.
78-79 2 Tx_I(Offset) Fixed decima
   ************************/
   TX_I_S = g_BBU_Fiber_INfo[76] +g_BBU_Fiber_INfo[77]*0.004;
   Tx_I_O = (short*)(&g_BBU_Fiber_INfo[78]);//电流


      /*******
80-81 2 Tx_PWR(Slope) Fixed decimal (unsigned) calibration data, transmitter coupled output power. Bit 7 of byte 80 is MSB, bit 0 of byte 81 is LSB. Tx_PWR(Slope) should be set to 1 for “internally calibrated” devices.
82-83 2 Tx_PWR(Offset) Fixed


   *************/
   TX_PWR_S =  g_BBU_Fiber_INfo[80] +g_BBU_Fiber_INfo[81]*0.004;
   TX_PWR_O =(short*)(&g_BBU_Fiber_INfo[82]);//功率

  // printf("TX_PWR:%f,%f\n",TX_PWR_S,TX_PWR_O);

     /*****
84-85 2 T (Slope) Fixed decimal (unsigned) calibration data, internal module temperature. Bit 7 of byte 84 is MSB, bit 0 of byte 85 is LSB.T(Slope) should be set to 1 for “internally calibrated” devices.
86-87 2 T (Offset) Fixed decimal

  *****/
    T_S = g_BBU_Fiber_INfo[84] +g_BBU_Fiber_INfo[85]*0.004;
   T_O =(short*)(&g_BBU_Fiber_INfo[86]);;//温度
   
   /*******************
88-89 2 V (Slope) Fixed decimal (unsigned) calibration data, internal module supply voltage. Bit 7 of byte 88 is MSB, bit 0 of byte 89 is LSB. V(Slope)should be set to 1 for “internally calibrated” devices.
90-91 2 V (Offset) Fixed decimal
   **********************/
   V_S =g_BBU_Fiber_INfo[88] +g_BBU_Fiber_INfo[89]*0.004;
   V_O =(short*)(&g_BBU_Fiber_INfo[90]);//电压

 /*  96 All Temperature MSB Internally measured module temperature.
97 All Temperature LSB*/
T_AD = ( short*)(&g_BBU_Fiber_INfo[96]);

/*
98 All Vcc MSB Internally measured supply voltage in transceiver.
99 All Vcc LSB****/

V_AD = ( short*)(&g_BBU_Fiber_INfo[98]);
/******
100 All TX Bias MSB Internally measured TX Bias Current.
101 All TX Bias LSB******************/
TX_I_AD = (short*)(&g_BBU_Fiber_INfo[100]);
/*******************
102 All TX Power MSB Measured TX output power.
103 All TX Power LSB*****************/
TX_PWR_AD = ( short*)(&g_BBU_Fiber_INfo[102]);
// printf("TX_PWR:%f,%f,%f\n",TX_PWR_S,TX_PWR_O,TX_PWR_AD);

/*********************
104 All RX Power MSB Measured RX input power.
105 All RX Power LSB
*/
    RX_PWR_AD = ( short*)(&g_BBU_Fiber_INfo[104]);
/*
1) Internally measured transceiver temperature. Module temperature, T, is given by the
following equation: T(C) = Tslope * TAD (16 bit signed twos complement value) + Toffset. The result is
in units of 1/256C,*/


Temper = T_S*(*T_AD) + (*T_O);
if(g_print_bbu_fiber_info)
{
printf("BBU Fiber optic module Temperature:%6.2f(℃)\n",Temper*0.004);
}

/***************************************
2) Internally measured supply voltage. Module internal supply voltage, V, is given in microvolts
by the following equation: V(uV) = VSLOPE * VAD (16 bit unsigned integer) + VOFFSET. The result is in
units of 100uV,

*******************************************/
Vol = V_S*(*V_AD) +(*V_O);
if(Vol<0)
{
   Vol = Vol*(-1);
}
if(g_print_bbu_fiber_info)
{
printf("BBU Fiber optic module Voltuage:%8.2f(uV)\n",Vol*100);
}

if((Vol*100)<NvRamData->fiber_para.Voltage)
{
          if(BVol_warn_flag==0)
          	{
    		   AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_BTS_FIBER_VOL,
                       ALM_CLASS_INFO,
                       STR_BBU_Fiber_Voltage_Warn,Vol*100); 
    		   BVol_warn_flag = 1;
          	}
}
else
{
     if(BVol_warn_flag)
     	{
     	    	   AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_BTS_FIBER_VOL,
                       ALM_CLASS_INFO,
                       STR_BBU_Fiber_Voltage_Warn,Vol*100); 
     	}
     BVol_warn_flag = 0;
}
/***************************************
3) Measured transmitter laser bias current. Module laser bias current, I, is given in microamps
by the following equation: I (uA) = ISLOPE * IAD (16 bit unsigned integer) + IOFFSET. This result is in
units of 2 uA, 

***************************************/
Current = (TX_I_S)*(*TX_I_AD)+(*Tx_I_O);

if(g_print_bbu_fiber_info)
{
printf("BBU Fiber optic module Current:%6.2f(uA)\n",Current*2);
}

if((Current*2)<NvRamData->fiber_para.Current)
{
    if(BCurrent_warn_flag==0)
    	{
       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_BTS_FIBER_Current,
                       ALM_CLASS_INFO,
                       STR_BBU_Fiber_Current_Warn,Current*2); 
          BCurrent_warn_flag = 1;
    	}
}
else
{
       if(BCurrent_warn_flag)
     	{
     	    	   AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_BTS_FIBER_Current,
                       ALM_CLASS_INFO,
                       STR_BBU_Fiber_Current_Warn,Current*2); 
     	}
     BCurrent_warn_flag = 0;
}


/*
4) Measured coupled TX output power. Module transmitter coupled output power, TX_PWR,
is given in uW by the following equation: TX_PWR (uW) = TX_PWRSLOPE * TX_PWRAD (16 bit
unsigned integer) + TX_PWROFFSET. This result is in units of 0.1uW */

Tx_pwer = TX_PWR_S*(*TX_PWR_AD) +(*TX_PWR_O);
if(g_print_bbu_fiber_info)
{
printf("BBU Fiber optic module Tx_Powert:%6.2f(uW)\n",Tx_pwer*0.1);
}

if((Tx_pwer*0.1)<NvRamData->fiber_para.Tx_Power)
{
        if(BTx_pwr_warn_flag==0)
        {
       AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_BTS_FIBER_TX_PWR,
                       ALM_CLASS_INFO,
                       STR_BBU_Fiber_TX_PWR_Warn,Tx_pwer*0.1); 
      			 BTx_pwr_warn_flag = 1;
        }
}
else
{
    if(BTx_pwr_warn_flag)
    	{
    	    
       AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_BTS_FIBER_TX_PWR,
                       ALM_CLASS_INFO,
                       STR_BBU_Fiber_TX_PWR_Warn,Tx_pwer*0.1); 
    	}
    BTx_pwr_warn_flag = 0;
}
/*
5) Measured received optical power. Received power, RX_PWR, is given in uW by the
following equation:
Rx_PWR (uW) = Rx_PWR(4) * Rx_PWRAD
4 (16 bit unsigned integer) +
Rx_PWR(3) * Rx_PWRAD
3 (16 bit unsigned integer) +
Rx_PWR(2) * Rx_PWRAD
2 (16 bit unsigned integer) +
Rx_PWR(1) * Rx_PWRAD (16 bit unsigned integer) +
Rx_PWR(0)
The result is in units of 0.1uW yielding a total range of 0 – 6.5mW.*/


Rx_pwer = (*Rx_PWR4)*(*RX_PWR_AD^4) +  (*Rx_PWR3)*(*RX_PWR_AD^3)  +( *Rx_PWR2)*(*RX_PWR_AD^2) + (*Rx_PWR1)*(*RX_PWR_AD) + (*Rx_PWR0);

if(g_print_bbu_fiber_info)
{
printf("BBU Fiber optic module Rx_Powert:%6.2f(uW)\n",Rx_pwer*0.1);

}


g_print_bbu_fiber_info = 0;
if((Rx_pwer*0.1)<NvRamData->fiber_para.Rx_Power)
{
      if(BRx_pwr_warn_flag==0)
      	{
            AlarmReport(ALM_FLAG_SET,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_BTS_FIBER_RX_PWR,
                       ALM_CLASS_INFO,
                       STR_BBU_Fiber_RX_PWR_Warn,Rx_pwer*0.1); 
            BRx_pwr_warn_flag = 1;
      	}
}
else
{
    if(BRx_pwr_warn_flag)
    	{
    	     AlarmReport(ALM_FLAG_CLEAR,
                       ALM_ENT_L3PPC, 
                       ALM_ENT_INDEX0, 
                       ALM_ID_BTS_FIBER_RX_PWR,
                       ALM_CLASS_INFO,
                       STR_BBU_Fiber_RX_PWR_Warn,Rx_pwer*0.1); 
    	}
     BRx_pwr_warn_flag= 0;
}

/******************************************************

消息方向：BTS ' EMS
字段名称	长度（Byte）	类型	定义	描述
RRU光模块温度	4	M		单位：摄氏度
RRU光模块电压	4	M		单位：uV
RRU光模块电流	4	M		单位：uA
RRU光模块发射功率	4	M		单位：uW
RRU光模块接收功率	4	M		单位：uW
				
**********************************************************/
   unsigned int  Temper1,Current1 ,Vol1,Tx_pwer1,Rx_pwer1;
if(g_BBU_Fiber_Transid!=0)
{
	 CComMessage *pMsg = NULL;
	            
	 unsigned char *ptemp;
	 pMsg = new (CWRRU::GetInstance(),  2+2+20) CComMessage;
	 if(pMsg==NULL)
		{
		   return;
		}
	ptemp =(unsigned char *)pMsg->GetDataPtr();
	pMsg->SetDstTid(M_TID_EMSAGENTTX);
	pMsg->SetSrcTid(M_TID_WRRU);
	pMsg->SetMessageId(M_EMS_BTS_BBU_Fiber_Info_RSP);
	ptemp[0] =g_BBU_Fiber_Transid>>8;
	ptemp[1] =g_BBU_Fiber_Transid;//result;
	ptemp[2] =0;
	ptemp[3] = 0;
	Temper1 =(unsigned int) ((Temper*0.004)*100);
	memcpy(ptemp+4,(unsigned char *)&(Temper1),4);
	Vol1 =(unsigned int) (Vol*10000);
	memcpy(ptemp+8,(unsigned char *)&(Vol1),4);
	Current1 = (unsigned int) (Current*200);
	memcpy(ptemp+12,(unsigned char *)&(Current1),4);
	Tx_pwer1 = (unsigned int) (Tx_pwer*0.1*100);
	memcpy(ptemp+16,(unsigned char *)&(Tx_pwer1),4);
	Rx_pwer1 = (unsigned int) (Rx_pwer*0.1*100);
	memcpy(ptemp+20,(unsigned char *)&(Rx_pwer1),4);
	if(true !=  CComEntity::PostEntityMessage(pMsg))     
	{
		OAM_LOGSTR1(LOG_DEBUG, L3CM_ERROR_PRINT_CFG_INFO, "[tWRRU] post msg[0x%04x] fail", pMsg->GetMessageId());
		pMsg->Destroy();
	}
}
g_BBU_Fiber_Transid = 0;
}


extern unsigned char g_bbu_afc4001_status ;
void print_afc4001_status()
{
   unsigned short Fpga_alarm;
    if(g_rru_ad4001_status==1)
    {
           printf("RRU afc4001 status is not right\n");
    }
    else
    {
    	 printf("RRU afc4001 status is  right\n");
    }
    if(g_bbu_afc4001_status==1)
    {
        Fpga_alarm =  Read_Fpga_Alarm();
        if(Fpga_alarm&0x800)
        {
        	    g_bbu_afc4001_status = 0;
        	    printf("BBU afc4001 status is  right\n");
        }
        else
        {
    		  printf("BBU afc4001 status is not right\n");
        }
    }
    else
    {
    	      printf("BBU afc4001 status is  right\n");
    	    
    }
}

/**************************************************

该函数提供给当系统正常，但没有用户时，通过复位DSP是否问题能解决

主要是现场反馈，系统正常，但用户注册不上，复位DSP1后用户可以注册上。

经过讨论，打算做如下工作:
1)1min查询L2层的在线终端列表，如果15min内一直没有变化，则复位DSP1
2)为了防止误操作，以后一天以后此功能才继续启动.
3)此函数返回1才允许复位DSP1

***************************************************/
extern "C" unsigned char  IfResetDSP()
{
      UINT16 mask;
 
	  if(rru_status==1)
	  {
	       mask = NvRamData->L1GenCfgEle.AntennaMask;
		mask = (mask)&(~Calibration_Antenna);
		if(mask>0)
		{
			return 1;
		}
	  }


	  return 0;
}
