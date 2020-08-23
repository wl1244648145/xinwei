/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    voiceFSM.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   2006-11-18 fengbing  ��͸��״̬���յ�VACSetupNotifyʱ��Ӧ�ɹ���VACSetupRsp��Ϊ�˸��õ�֧�ֿտڵ��ؽ�
 *   2006-10-18 fengbing  �޸��������̣������������յ�����������Ӧ��ʱ��Ӱ���´������ķ���ʱ�� 
 *   2006-10-13 fengbing  ��֤����û�н���������²������쳣������л�
 *   2006-09-14 fengbing  ֧���쳣������л�
 *   2006-09-07 yanghw    ����׮����BroadCastSM��ģ��SAG���͹㲥��Ϣ
 *   2006-09-06 fengbing  ����moveaway���ӳ�disableӰ��moveaway�����̵�enable������
 *   2006-09-04 fengbing  fix moveaway bugs.
 *   2006-08-21 yanghw    �޸Ĺ㲥���ŵ���Ϣ��Ӧ�ṹ
 *   2006-08-19 fengbing  �ٴ��޸�Assign transport resource Req��Ϣ������ServiceOption,ServiceAppType 
 *   2006-08-11 yanghw    �޸�Assign transport resource Req��Ϣ������ServiceOption,ServiceAppType 
 *   2006-08-10 yanghw    ���ӶԹ㲥���ŵĴ���
 *   2006-08-07 yanghw    �޸�Assign transport resource Req���ͷ��򣬷���Assign transport resource Req��Ϣ
 *   2006-08-07 yanghw    �޸�����ƽ����DMUX�Ķ���
 *   2006-07-27 fengbing  ��Ϊ���ҽ����а���ʱ�Ÿ�SAG����ʧ�ܵ�LAPaging Rsp
                          Ϊ�˷�����ԣ����ӹرպͿ�����SAG֮��������ĵ�������
                          �����л�ʱ��tCM��Ϊ��MoveAway��Ϣ�Ĵ����յ�����Ϣ���Ӻ���Լ�����Disable��Ϣ
 *   2006-04-19 fengbing  ��������ӿ�������������ݵļ���ͳ��
 *   2006-04-18 fengbing  �޸�handleVoiceDataFromVDR������memcpy�Ĵ���
 *   2006-04-16 fengbing  �޸Ĳ��Ժ���clearCCBState��pagingCPEVoice
 *   2006-04-10 fengbing  �޸Ĳ��Ժ���clearCCBState�����Ӳ��Ժ���showBTree
 *   2006-04-10 fengbing  �޸Ĳ��Ժ���regCPEVoice,pagingCPEvoice,showCCBState,clearCCBState
 *   2006-04-09 fengbing  �޸����̣���CPE��BTS����VACʱ��ʧ�ܵ����̶���Ӧʧ�ܵ�VACSetupRsp(�����ԭ��),��������ǰ��VACRelease
 *   2006-04-09 fengbing  delete VOICE_IDLE_DELAPAGING from VOICE_TRANS
 *   2006-04-09 fengbing  �޸Ĳ��Ժ���regCPEVoice,pagingCPEvoice,showCCBState,clearCCBState
 *   2006-04-09 fengbing  ����SAbis1�ӿ��ĵ��޸ı��뷽ʽ�Ķ��壬�Ự���ȼ��Ķ��壬
 *                        DMUX�����������ȼ��Ķ���,APP_TYPE�ֶεĶ��壬Request transport
 *                        data rate�ı仯
 *   2006-3-29	fengbing  �޸�DMUX�ӿ�
 *   2006-3-27  fengbing  ���ӱ���ʱ��ӦPagingRsp���յ�Error Notification Req�Ĵ���
 *   2006-3-26  fengbing  ���ӹ��ܣ���SAG��������ӵ��ʱֱ���ͷſտڣ��ܾ��µĺ���
 *   2006-3-22  fengbing  modify DMUX control word field;
 *                        delete congestion timer;
 *                        add more serious checking for abnormal routine;
 *                        modify FSM, add Trans_Testab_DeLAPaging routine;
 *   2005-9-13  fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/
#include "localSag.h"
#include "localSagCfg.h"
#include "localSagMsgID.h"
#include "localSagStruct.h"
#include "DBroadCastSrv.h"

#include "string.h"
#include "time.h"
#include "VoiceFSM.h"
#include "BtsVMsgId.h"
#include "tVoice.h"
#include "voiceData.h"
#include "commessage.h"
#include "callsignalmsg.h"
#include "othermsg.h"
#include "timeoutVoiceMsg.h"
#include "VAC_session_interface.h"
#include "vac_voice_data.h"
#include "VDR_Voice_Data.h"
#include "NatpiApp.h"
#include "L3OamCfgCommon.h"
#include "tVCR.h"
#ifdef DSP_BIOS
#include "LogArea.h"
#endif
#include "sysBtsConfigData.h"
#include "Voicecfg.h"
#include "VoiceToolFunc.h"
#include "VoiceTone.h"
#include "l3oamsystem.h"
#include "ExternalIF.h"

//SAbis1Flag
bool blSAbisIFOK=false;
bool blSAbisIFOK_old=false;

bool blSAbisIFOK_Master=false;
bool blSAbisIFOK_Master_old=false;
bool blSAbisIFOK_Backup=false;
bool blSAbisIFOK_Backup_old=false;
//SAbis1Flag

VoicePerfCounterT Counters;
bool blUseBeatHeart = true;
bool g_blSMSSelfTestFlag = false;
extern "C" void setSmsSelfTestFlag(bool flag)
{
	g_blSMSSelfTestFlag=flag;
}

//20090531 fengbing broadcast SMS test begin
#define BROADCAST_SMS_TEST
#ifdef BROADCAST_SMS_TEST
#define MAX_SMS_BCHSMSTEST_LENGTH	(140)
UINT32 g_nBchSMSTestSN = 0x1;
char g_BchSMSStr[MAX_SMS_BCHSMSTEST_LENGTH+1]={0,};
char g_BchSmsTestDisplayNumber[50]="01012345678";
extern "C" void bchSmsTest(char *smsStr, char *displayNumber);
#ifdef DSP_BIOS
extern "C"
{
   T_TimeDate bspGetDateTime() ;
   int bspGetBtsID();
}
#endif
#endif//BROADCAST_SMS_TEST
//20090531 fengbing broadcast SMS test end

#define Mod2Power(x,y)	(x & (y-1))		//x/y��������y��2����
//=============================================================

UINT32 g_FrameNum=0;	//global 10ms counters for L3Voice

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


typedef struct _rsvVoiceCfgT
{
	UINT8 blUse10msULSrtp:1;
	UINT8 rsv1:7;
	UINT8 rsv2[63];
}rsvVoiceCfgT;

typedef struct _writeNvRamInfo
{
	char* dstAddr;
	char* srcAddr;
	int len;
}writeNvRamInfoT;
extern T_NvRamData *NvRamDataAddr;
rsvVoiceCfgT tmpCfg;
void readRsvVoiceCfg()
{
	memcpy((void*)&tmpCfg, 
		(void*)&NvRamDataAddr->Reserved4Voice, 
		sizeof(rsvVoiceCfgT));
}
void writeData2NvRam(char* dstAddr, char* srcAddr, UINT32 len)
{
	//����Ϣ��tcfg����дNVRAM
	CComMessage *pMsg = new (CTVoice::GetInstance(), sizeof(writeNvRamInfoT)) CComMessage;
	if(pMsg==NULL)
	{
		VPRINT("\n writeData2NvRam(), new commessage error!!!\n");
		return ;
	}
	pMsg->SetDstTid(M_TID_CM);
	pMsg->SetSrcTid(M_TID_VOICE);
	pMsg->SetMessageId(MSGID_OAM_WRITE_NVRAM);
	writeNvRamInfoT* pDataMsg = (writeNvRamInfoT*)pMsg->GetDataPtr();
	pDataMsg->dstAddr= dstAddr;
	pDataMsg->srcAddr = srcAddr;
	pDataMsg->len = len;
	if(!postComMsg(pMsg))
	{
		VPRINT("\n writeData2NvRam(), post message to tCM failed!!!! \n");
	}

}
void writeRsvVoiceCfg()
{
	writeData2NvRam((char*)&NvRamDataAddr->Reserved4Voice, 
		(char*)(&tmpCfg), sizeof(rsvVoiceCfgT));
}

bool g_blUse10msULSrtp = false;	//20091007 fengbing
extern "C" void set10msSrtpFlag(bool flag)
{
	readRsvVoiceCfg();
	g_blUse10msULSrtp=flag;
	tmpCfg.blUse10msULSrtp = g_blUse10msULSrtp?1:0;
	writeRsvVoiceCfg();
}
extern "C" void show10msSrtpFlag()
{
	VPRINT("\n Using 10msSrtp g_blUse10msULSrtp[%s] valInNvRam[0x%02X]\n", 
		g_blUse10msULSrtp? "true":"false", tmpCfg.blUse10msULSrtp);
}
void initRsvVoiceCfgFromNvRam()
{
	readRsvVoiceCfg();
	//srtp pkt type
	g_blUse10msULSrtp = tmpCfg.blUse10msULSrtp;
	//���õĺϷ��Լ������������������Ϸ������������
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////



bool g_blUseDownLinkJitterBuf = false;
bool g_blForceUseJbuf = false;
extern "C" void showJBufFlag()
{
	VPRINT("\nUseDownLinkJitterBuf(cpeZ only)[%d] ForceUseJbuf(all cpes)[%d] \n",
		g_blUseDownLinkJitterBuf, g_blForceUseJbuf);
}
extern "C" void enableJitterBuf(){g_blUseDownLinkJitterBuf=true;}
extern "C" void disableJitterBuf(){g_blUseDownLinkJitterBuf=false;g_blForceUseJbuf=false;}

UINT32 JITTER_FRAMES=32;
UINT32 g_Frames2Start = 4;
extern "C" void setJBufMaxFrames(UINT32 maxFrames)
{
	switch(maxFrames)
	{
	case 16:
	case 32:
	case 64:
	case 128:
	case 256:
		JITTER_FRAMES = maxFrames;
		break;
	default:
		VPRINT("\nmaxFrames[%d] should be in range [16,32,64,128,256].\n", (int)M_MAX_JITTER_FRAMES);
	}
}
extern "C" void setJBufStartFrameNum(UINT32 startFrames)
{
	if(startFrames<JITTER_FRAMES/2)
	{
		if(startFrames>=4)
		{
			g_Frames2Start = startFrames;
		}
		else
		{
			VPRINT("\nstartFrames should not be smaller than 4.\n");
		}
	}
	else
	{
		VPRINT("\nwhen JITTER_FRAMES is %d, startFrames should be smaller than %d/2 and can not be 0.\n", 
				(int)JITTER_FRAMES, (int)JITTER_FRAMES);
	}
}
extern "C" void forceUseJBuf()
{
	g_blForceUseJbuf = true;
	g_blUseDownLinkJitterBuf = true;
}

UINT16 g_nBufferFaxDataFrames = 20;
bool g_blBufFaxData = true;
extern "C" void setBufFaxDataFlag(bool flag)
{
	g_blBufFaxData = flag;
}
extern "C" void showBufFaxFlag()
{
	VPRINT("\ng_blBufFaxData=%d\n", g_blBufFaxData);
}
extern "C" void setBufFaxDataFrames(UINT16 n)
{
	if(n>M_MIN_BUFFER_FAXDATA_FRAMES && n<M_MAX_BUFFER_FAXDATA_FRAMES)
	{
		g_nBufferFaxDataFrames = n;
	}
}
extern "C" void showBufFaxDataFrames()
{
	VPRINT("\ng_nBufferFaxDataFrames=%d\n", g_nBufferFaxDataFrames);
}

voiceDataBufT*	 vDatabuf=NULL;
list<UINT16> FreeVDataBufList;
void initAllVDataBuf();
void initOneVDataBuf(UINT16 bufID);
UINT16 allocVDataBuf();
void deallocVDataBuf(UINT16 bufID);


void initOneVDataBuf(UINT16 bufID)
{
	int j;
	if(bufID<M_MAX_CALLS)
	{
		vDatabuf[bufID].uid = INVALID_UID;
		vDatabuf[bufID].eid = NO_EID;
		vDatabuf[bufID].cid = NO_CID;
		vDatabuf[bufID].peerEid = NO_EID;
		vDatabuf[bufID].peerCid = NO_CID;
		
		vDatabuf[bufID].upLinkBuf.blStarted = false;
		vDatabuf[bufID].upLinkBuf.timeStampLen = 0;
		vDatabuf[bufID].upLinkBuf.curBufId = 0;
		vDatabuf[bufID].upLinkBuf.len = 0;
		vDatabuf[bufID].upLinkBuf.srtpItmeHead.SN = 0;
		vDatabuf[bufID].upLinkBuf.srtpItmeHead.timeStamp = 0;
		vDatabuf[bufID].upLinkBuf.srtpItmeHead.Codec = CODEC_G729A;
		VSetU16BitVal(vDatabuf[bufID].upLinkBuf.srtpItmeHead.CallID, 0xffff);
		
		vDatabuf[bufID].downLinkBuf.nTimeStampNew = 0xffffffff;
		vDatabuf[bufID].downLinkBuf.blStarted = false;
		//vDatabuf[bufID].downLinkBuf.curRcvSN = 0;
		vDatabuf[bufID].downLinkBuf.curSndSN = 0;
		//vDatabuf[bufID].downLinkBuf.curRcvIdx = 0;
		vDatabuf[bufID].downLinkBuf.curSndIdx = 0;
		vDatabuf[bufID].downLinkBuf.nRcvFrames = 0;
		//vDatabuf[bufID].downLinkBuf.nTmp = 0;
		vDatabuf[bufID].downLinkBuf.nInvalidSNCounter = 0;
		
		for(j=0;j<(int)JITTER_FRAMES;j++)
		{
			vDatabuf[bufID].downLinkBuf.jitterBuffer[j].len = M_INVALID_10MS_DATALEN;
		}

		memset(&vDatabuf[bufID].faxDataBuf, 0, sizeof(FaxDataBufferT));
	}
}

void initAllVDataBuf()
{
	int i;
	//VPRINT("\n########################sizeof(voiceDataBufT)[%d]\n",sizeof(voiceDataBufT));
	vDatabuf = (voiceDataBufT*)malloc(M_MAX_CALLS * sizeof(voiceDataBufT));
	if(vDatabuf==NULL)
	{
		VPRINT("\n########################malloc failed!!!!!!!!!!\n");
		disableJitterBuf();
	}
	else
	{
		//VPRINT("\n########################malloc success\n");
	}
	FreeVDataBufList.clear();
	for(i=0;i<M_MAX_CALLS;i++)
	{
		initOneVDataBuf(i);
		FreeVDataBufList.push_back(i);
	}
}

UINT16 allocVDataBuf()
{
	UINT16 ret = M_INVALID_VDATABUF_IDX;
	if(!FreeVDataBufList.empty())
	{
		ret = *(FreeVDataBufList.begin());
		FreeVDataBufList.pop_front();
		initOneVDataBuf(ret);

		vDatabuf[ret].upLinkBuf.blStarted = true;
	}	
	return ret;
}

void deallocVDataBuf(UINT16 bufID)
{
	if(bufID<M_MAX_CALLS)
	{
		initOneVDataBuf(bufID);
		FreeVDataBufList.push_back(bufID);
		vDatabuf[bufID].upLinkBuf.blStarted = false;
	}
}

void initVDataBufCCBInfo(VoiceCCB& ccb)
{
	int i;
	VoiceTuple tuple;
	if(ccb.m_vDataIdx<M_MAX_CALLS)
	{
		vDatabuf[ccb.m_vDataIdx].uid = ccb.getUID();//fengbing 20090603 for spyVData with jitterbuffer

		VSetU16BitVal(vDatabuf[ccb.m_vDataIdx].upLinkBuf.srtpItmeHead.CallID, (ccb.getL3addr() & 0xffff));
		tuple = ccb.getVoiceTuple();
		for(i=0;i<(int)JITTER_FRAMES;i++)
		{
			VSetU32BitVal(vDatabuf[ccb.m_vDataIdx].downLinkBuf.jitterBuffer[i].VACItemHead.Eid, tuple.Eid);
			vDatabuf[ccb.m_vDataIdx].downLinkBuf.jitterBuffer[i].VACItemHead.Cid = tuple.Cid;
		}
		vDatabuf[ccb.m_vDataIdx].eid = tuple.Eid;
		vDatabuf[ccb.m_vDataIdx].cid = tuple.Cid;
	}	
}

char	timerName[][32]=
{
	"",//TIMER_MIN_TYPE=0,
	"TIMER_ASSIGN",
	"TIMER_VAC",
	"TIMER_ERRRSP",
	"TIMER_RELRES",
	"TIMER_PROBERSP",
	"TIMER_DELAY_RELVAC",

	"TIMER_BEATHEART",
	"TIMER_CONGEST",

	"TIMER_GrpPagingRsp",
	"TIMER_LePagingRsp",
	"TIMER_StautsReport",
	"TIMER_LePagingLoop",
	"TIMER_ResClear",
	"TIMER_LePagingStart",	
	"TIMER_GrpDataDetect",
	"TIMER_GrpRls",
	//TIMER_MAX
};


bool startTimer(
	UINT8 timerType, UINT32 lenTimer, UINT8 timerID,CTimer**ppTimer,
	TID dstTID,TID srcTID,UINT32 msgID,
	UINT32 eid=NO_EID,UINT8 cid=NO_CID,UINT16 gid=M_INVALID_GID)
{
	if(NULL==ppTimer)
		return false;
	if(*ppTimer!=NULL)
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "startTimer, pTimer!=NULL WARNING!!!!!");	
	}
	CMsg_VoiceTimeout timeoutMsg;
	if ( !timeoutMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		timeoutMsg.SetTimerType(timerID);
		timeoutMsg.SetMessageId(msgID);
		timeoutMsg.SetSrcTid(srcTID);
		timeoutMsg.SetDstTid(dstTID);
		timeoutMsg.SetEID(eid);
		timeoutMsg.SetCid(cid);
		timeoutMsg.SetGid(gid);
	}    
	*ppTimer = new CTimer(0!=timerType, lenTimer, timeoutMsg);
	if((*ppTimer)==NULL)
	{
		LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "startTimer, new CTimer failed!!!!");
		timeoutMsg.DeleteMessage();
		return false;
	}
	else
	{
		(*ppTimer)->Start();		
	}
#if 0
	if(NO_EID!=eid && NO_CID!=cid)
	{
		VoiceTuple tuple;
		tuple.Eid = eid;
		tuple.Cid = cid;
		VoiceCCB* pCCB=(VoiceCCB*)CTVoice::GetInstance()->getVoiceFSM()->CCBTable->FindCCBByEID_CID( tuple);
		if(pCCB!=NULL)
			LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", pCCB->getUID(), (int)timerName[timerID]);
	}
	else if(M_INVALID_GID!=gid)
	{
		GrpCCB* pCCB=(GrpCCB*)CTVoice::GetInstance()->getVoiceFSM()->pGrpCCBTbl->FindCCBByGID(gid);
		if(pCCB!=NULL)
			LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, GID[0x%08X], TimerType[%s]", pCCB->getGID(), (int)timerName[timerID]);
	}
	else
	{
		LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, TimerType[%s]", (int)timerName[timerID]);
	}
#endif	
	return true;
}

bool stopTimer(CTimer **ppTimer)
{
	if(ppTimer!=NULL)
	{
		if((*ppTimer)!=NULL)
		{
			(*ppTimer)->Stop();
			return true;
		}
	}
	return false;
}

bool deleteTimer(CTimer **ppTimer)
{
	if(ppTimer!=NULL)
	{
		if((*ppTimer)!=NULL)
		{
			(*ppTimer)->Stop();
			delete (*ppTimer);
			*ppTimer = NULL;
			return true;
		}
	}
	return false;
}
//=============================================================




#ifdef __UNITTEST__

void OutPutMessage(CMessage& msg)
{
	UINT8*	pPayload = (UINT8*)msg.GetDataPtr();
	int payloadLen = msg.GetDataLength();
	VPRINT("\r\n==========Message Content:==========");
	VPRINT("\r\nMessageID:[%x]	EID:[%x]", msg.GetMessageId(), msg.GetEID());
	VPRINT("\r\nPayload:");
	for(int i=0;i<payloadLen;i++)
	{
		if((i%8)==0)
			VPRINT("\r\n");
		VPRINT("%02x ", pPayload[i]);
	}
	VPRINT("\r\n===================================\n");
}

#endif

/*===========================================================================*/
//ccb
void VoiceCCB::startTimer(UINT8 timerType, UINT32 lenTimer, const CMessage &rMsg)
{
	if(pTimer!=NULL)
	{
		LOG(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceCCB::startTimer, pTimer!=NULL WARNING!!!!!");
	
	}
	CMsg_VoiceTimeout timeoutMsg(rMsg);
	timeoutMsg.SetEID(getVoiceTuple().Eid);
	timeoutMsg.SetCid(getVoiceTuple().Cid);
	timeoutMsg.SetDstTid(M_TID_VOICE);
	pTimer = new CTimer(0!=timerType, lenTimer, rMsg);
	if(pTimer==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "VoiceCCB::startTimer, new CTimer failed!!!!");
	}
	else
		pTimer->Start();
}
void VoiceCCB::stopTimer() 
{ 
	if(pTimer!=NULL) 
	{ 
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StopTimer, UID[0x%08X]", getUID());
		pTimer->Stop(); 
	} 
}
void VoiceCCB::deleteTimer() 
{ 
	if(pTimer!=NULL) 
	{ 
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StopTimer, UID[0x%08X]", getUID());
		pTimer->Stop(); 
		delete pTimer; 
		pTimer = NULL; 
	} 
}

void VoiceCCB::startTimerWaitSYNC()
{
	if(pTimerWaitSYNC!=NULL)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceCCB::startTimerWaitSYNC, pTimerWaitSYNC!=NULL WARNING!!!!!");
		deleteTimerWaitSYNC();
	}
	
	CMsg_VoiceTimeout waitSYNCTimeoutMsg;
	waitSYNCTimeoutMsg.CreateMessage(*CTVoice::GetInstance());
	waitSYNCTimeoutMsg.SetMessageId(MSGID_TIMER_WAITSYNC);
	waitSYNCTimeoutMsg.SetEID(getVoiceTuple().Eid);
	waitSYNCTimeoutMsg.SetCid(getVoiceTuple().Cid);
	waitSYNCTimeoutMsg.SetDstTid(M_TID_VOICE);
	waitSYNCTimeoutMsg.SetSrcTid(M_TID_VOICE);
	waitSYNCTimeoutMsg.SetDataLength(sizeof(TimerStructT));
	
	pTimerWaitSYNC = new CTimer(0, M_TIMERLEN_MOVEAWAY_DISABLE, waitSYNCTimeoutMsg);
	if(pTimerWaitSYNC==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "VoiceCCB::startTimerWaitSYNC, new CTimer failed!!!!");
	}
	else
	{
		pTimerWaitSYNC->Start();
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimerWaitSYNC, UID[0x%08X]", getUID());
	}
}

void VoiceCCB::deleteTimerWaitSYNC()
{
	if(pTimerWaitSYNC!=NULL) 
	{ 
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StopTimerWaitSYNC, UID[0x%08X]", getUID());
		pTimerWaitSYNC->Stop(); 
		delete pTimerWaitSYNC; 
		pTimerWaitSYNC = NULL; 
	} 
}

void VoiceCCB::startTimerMoveAwayDisable()
{
	if(pTimerMoveAwayDisable!=NULL)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceCCB::startTimerMoveAwayDisable, pTimerMoveAwayDisable!=NULL WARNING!!!!!");
		deleteTimerMoveAwayDisable();
	}
	
	CMsg_VoicePortUnReg portDisableMsg;
	portDisableMsg.CreateMessage(*CTVoice::GetInstance());
	portDisableMsg.SetMessageId(MSGID_VOICE_UT_UNREG);
	portDisableMsg.SetEID(getVoiceTuple().Eid);
	portDisableMsg.SetCid(getVoiceTuple().Cid);
	portDisableMsg.SetUid(getUID());//20091113 add by fengbing
	portDisableMsg.SetDstTid(M_TID_VOICE);
	portDisableMsg.SetSrcTid(M_TID_VOICE);
	portDisableMsg.SetDataLength(sizeof(CMVoiceRegMsgT));
	
	pTimerMoveAwayDisable = new CTimer(0, M_TIMERLEN_MOVEAWAY_DISABLE, portDisableMsg);
	if(pTimerMoveAwayDisable==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "VoiceCCB::startTimerMoveAwayDisable, new CTimer failed!!!!");
	}
	else
	{
		pTimerMoveAwayDisable->Start();
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimerMoveAwayDisable, UID[0x%08X]", getUID());
	}
}

void VoiceCCB::deleteTimerMoveAwayDisable()
{
	if(pTimerMoveAwayDisable!=NULL) 
	{ 
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StopTimerMoveAwayDisable, UID[0x%08X]", getUID());
		pTimerMoveAwayDisable->Stop(); 
		delete pTimerMoveAwayDisable; 
		pTimerMoveAwayDisable = NULL; 
	} 
}
void VoiceCCB::CCBClean()
{
	deallocVDataBuf(m_vDataIdx);
	m_vDataIdx = M_INVALID_VDATABUF_IDX;
	
	m_tuple.Cid = 0;
	m_tuple.Eid = 0;
	m_UID = 0;
	m_L3Addr = NO_L3ADDR;
	
	m_AppType = APPTYPE_VOICE_G729;
	m_CodecInfo = CODEC_G729A;
	m_AppPrio = M_DEFAULT_VOICE_PRIORITY;		//Ŀǰ�汾�������ȼ��̶�Ϊ
	
	deleteTimer();
	deleteTimerMoveAwayDisable();
	deleteTimerWaitSYNC();

	clearConnectedFlag();

//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
	disableInnerSwitch();
#endif
//20090531 fengbing bts inner switch for Voice Data end
}

//����Release transport resource Rsp��tVCR
void VoiceCCB::sendRlsResRsp()
{
	CMsg_Signal_VCR RlsResRsp;
	VoiceVCRCtrlMsgT *pData;
	if ( !RlsResRsp.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(RlsResRspT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		pData = (VoiceVCRCtrlMsgT*)RlsResRsp.GetDataPtr();
		RlsResRsp.SetDstTid(M_TID_VCR);
		RlsResRsp.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		RlsResRsp.SetBTSSAGID();
		RlsResRsp.SetSigIDS(RlsResRsp_MSG);

		VSetU32BitVal(pData->sigPayload.RlsResRsp.L3addr, getL3addr());

		RlsResRsp.SetSigHeaderLengthField(sizeof(RlsResRspT));
		RlsResRsp.SetPayloadLength(sizeof(SigHeaderT)+sizeof(RlsResRspT));
		RlsResRsp.Post();
	}    
}

//��SAG����Error Notification Req
void VoiceCCB::sendErrNotifyReqtoSAG(UINT8 errCause)
{
	CMsg_Signal_VCR errNotify;
	if ( !errNotify.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(ErrNotifyReqT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)errNotify.GetDataPtr();
		errNotify.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		errNotify.SetDstTid(M_TID_VCR);
		errNotify.SetBTSSAGID();
		errNotify.SetSigIDS(ErrNotifyReq_MSG);
		VSetU32BitVal(pData->sigPayload.ErrNotifyReq.Uid, getUID());		//UID
		pData->sigPayload.ErrNotifyReq.ErrCause = errCause;				//error cause
		VSetU32BitVal(pData->sigPayload.ErrNotifyReq.L3Addr, getL3addr());	//L3Addr
			
		errNotify.SetSigHeaderLengthField(sizeof(ErrNotifyReqT));
		errNotify.SetPayloadLength(sizeof(SigHeaderT)+sizeof(ErrNotifyReqT));
		errNotify.Post();
	}
}
//VAC session Release
void VoiceCCB::VACRelease()
{
	CMsg_VACRelease VACRelease;
	if ( !VACRelease.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		VACRelease.SetDstTid(M_TID_VAC);
		VACRelease.SetMessageId(MSGID_VAC_RLS_CMD);
		VACRelease.SetEID(getVoiceTuple().Eid);
		VACRelease.SetCid(getVoiceTuple().Cid);
		VACRelease.SetPayloadLen(sizeof(VACRlsCmdReqT));

		VACRelease.Post();
	}
}
void VoiceCCB::sendVACSetupReq(UINT8 reason, UINT8 rate)
{
	//����VAC setup cmd(init)��tVAC;
	CMsg_VACSetupReq VACSetupReq;
	if ( !VACSetupReq.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		VACSetupReq.SetDstTid(M_TID_VAC);
		VACSetupReq.SetMessageId(MSGID_VAC_SETUP_CMD);
		VACSetupReq.SetEID(getVoiceTuple().Eid);
		VACSetupReq.SetCid(getVoiceTuple().Cid);
		VACSetupReq.SetReason(reason);
		VACSetupReq.SetRate(rate);					//��ʱ8k
#ifdef M__SUPPORT__ENC_RYP_TION		
		VACSetupReq.SetRsv();
		VACSetupReq.SetSagStatus(sagStatusFlag ? 1:0);//fengbing 20100208
#endif		
		VACSetupReq.SetPayloadLen(sizeof(VACSetupCmdReqT));
		VACSetupReq.Post();
	}        
}
void VoiceCCB::sendVACSetupRsp(ENUM_VACSetupRspResultT result)
{
	CMsg_VACSetupRsp VACSetupRsp;
	if ( !VACSetupRsp.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		VACSetupRsp.SetDstTid(M_TID_VAC);
		VACSetupRsp.SetMessageId(MSGID_L3_VAC_SETUPRSP);
		VACSetupRsp.SetEID(getVoiceTuple().Eid);
		VACSetupRsp.SetCid(getVoiceTuple().Cid);
		VACSetupRsp.SetResult(result);
#ifdef M__SUPPORT__ENC_RYP_TION		
		VACSetupRsp.SetRsv();
		VACSetupRsp.SetSagStatus(sagStatusFlag ? 1:0);//fengbing 20100208
#endif															
		VACSetupRsp.SetPayloadLen(sizeof(VACSetupCmdRspT));
		VACSetupRsp.Post();
	}
}

void VoiceCCB::releaseActiveCall()
{
	CComMessage *pReleaseSignal = new (CTVoice::GetInstance(), 3) CComMessage;
	if(pReleaseSignal!=NULL)
	{
		VoiceVACCtrlMsgT* pDataRelease = (VoiceVACCtrlMsgT*)pReleaseSignal->GetDataPtr();
		pReleaseSignal->SetMessageId(MSGID_VOICE_VAC_SIGNAL);
		pReleaseSignal->SetDstTid(M_TID_VAC);
		pReleaseSignal->SetSrcTid(M_TID_VOICE);
		pReleaseSignal->SetDataLength(3);
		pReleaseSignal->SetEID(getVoiceTuple().Eid);
		pDataRelease->Cid = this->getVoiceTuple().Cid;
		pDataRelease->sigPayload[0] = M_MSGTYPE_RELEASE;
		pDataRelease->sigPayload[1] = REL_CAUSE_SAG;
		postComMsg(pReleaseSignal);
	}
	else
	{
		LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
						"New ComMessage failed!!!");
	}
	deleteTimerWaitSYNC();
	this->deleteTimer();
	//������ʱ��TdelayRelVac;
	CMsg_VoiceTimeout TdelayRelVac;
	if ( !TdelayRelVac.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TdelayRelVac.SetMessageId(MSGID_TIMER_DELAY_RELVAC);
		TdelayRelVac.SetTimerType(TIMER_DELAY_RELVAC);
		startTimer(0, M_TIMERLEN_DELAY_RELVAC, TdelayRelVac);
	
		LOG2(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", getUID(),(int) "TDelayRelVac");
	}
	SetCurrentState(VOICE_RELEASE_STATE);
	LOG1(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"UID[0x%08X], release Call!!!###########", 
		getUID());
}

UINT32 VoiceCCB::getInStateTime(UINT32 curFN)
{
	if( curFN>getEntryStateFN() )
	{
		return (curFN-getEntryStateFN()); 
	}
	else
	{
		return (0xffffffff-getEntryStateFN()+curFN);
	}
}

//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
void VoiceCCB::initInnerSwitch()
{
	setBtsInnerSwitchVDataFlag(false); 
	setPeerCCB(NULL);
	setPeerL3Addr(NO_L3ADDR);
}
void VoiceCCB::disableInnerSwitch()
{
	if(isBtsInnerSwitchVData())
	{
		LOG2(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"disableInnerSwitch(), UID[0x%08X] L3Addr[0x%08X]",
			getUID(), getL3addr());
	}
#if 0	
	if(isBtsInnerSwitchVData())
	{
		LOG(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"disableInnerSwitch(), UID[0x%08X] L3Addr[0x%08X] peerUID[0x%08X] peerL3Addr[0x%08X]",
			getUID(), getL3addr(), getPeerCCB()->getUID(), getPeerCCB()->getL3addr());
	}	
#endif
	if(isBtsInnerSwitchVData() && getPeerCCB()!=NULL)
	{
		if(getPeerCCB()!=this || getPeerCCB()->getL3addr()!=getPeerL3Addr())
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "disableInnerSwitch(), peerCCB or peerL3Addr error!!!");
		}
		getPeerCCB()->setBtsInnerSwitchVDataFlag(false);
		getPeerCCB()->setPeerCCB(NULL);
		getPeerCCB()->setPagingL3Addr(NO_L3ADDR);
	}
	setBtsInnerSwitchVDataFlag(false); 
	setPeerCCB(NULL);
	setPeerL3Addr(NO_L3ADDR);
	//��֤��Ҫʱ��sag���ͱ���timestamp�Ŀ�������,����Զ�Eid/Cid��faxdata��������ʱʹ��
	if(m_vDataIdx<M_MAX_CALLS)
	{
		vDatabuf[m_vDataIdx].upLinkBuf.blStarted = true;
		vDatabuf[m_vDataIdx].peerEid = NO_EID;
		vDatabuf[m_vDataIdx].peerCid = NO_CID;
	}	
}
void VoiceCCB::enableInnerSwitch(VoiceCCB *ppeerCCB)
{
	if(NULL==ppeerCCB)
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "enableInnerSwitch(), peerCCB is NULL!!!");		
	}
	else
	{
		if(!isBtsInnerSwitchVData())
		{
			LOG4(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"enableInnerSwitch(), UID[0x%08X] L3Addr[0x%08X] peerUID[0x%08X] peerL3Addr[0x%08X]",
				getUID(), getL3addr(), ppeerCCB->getUID(), ppeerCCB->getL3addr());
		}
		setBtsInnerSwitchVDataFlag(true);
		setPeerCCB(ppeerCCB);
		setPeerL3Addr(ppeerCCB->getL3addr());
		//�ݲ���sag���ͱ���timestamp�Ŀ�������,��¼�Զ�Eid/Cid��faxdata��������ʱʹ��
		if(m_vDataIdx<M_MAX_CALLS)
		{
			vDatabuf[m_vDataIdx].upLinkBuf.blStarted = false;
			VoiceTuple tuple = ppeerCCB->getVoiceTuple();
			vDatabuf[m_vDataIdx].peerEid = tuple.Eid;
			vDatabuf[m_vDataIdx].peerCid = tuple.Cid;
		}
	}
}
#endif
//20090531 fengbing bts inner switch for Voice Data end


/*===========================================================================*/
//ccbtable
/*
 *	������Ϣ�ҵ�CCB,ֻ��ע��״̬������Ϣ�ŵ��ô˺���
 */
CCBBase* VoiceCCBTable::FindCCB(CMessage &msg) 
{
	UINT16 msgID = msg.GetMessageId();

	//�����SAG���͵����ע�������ֽ�˳��ת��
	if( msgID==MSGID_VCR_VOICE_SIGNAL)
	{
		CMsg_Signal_VCR VCRsignal(msg);
		UINT8 how;
		UINT32 Uid_L3addr=NO_L3ADDR;
		if(VCRsignal.SAGSignalHowToFindCCB(how, Uid_L3addr))
		{
			if(how == 0)//use uid
				return FindCCBByUID(Uid_L3addr);
			if(how == 1)//use l3addr
				return FindCCBByL3addr(Uid_L3addr);
		}
		return NULL;
	}

	VoiceTuple tuple;

	//����Ƕ�ʱ����Ϣ
	if(msgID<=MSGID_CCBTIMER_END && msgID>=MSGID_CCBTIMER_BEGIN)
	{
		CMsg_VoiceTimeout CCBTimeoutMsg(msg);
		tuple.Eid = CCBTimeoutMsg.GetEID();
		tuple.Cid = CCBTimeoutMsg.GetCid();
		return FindCCBByEID_CID(tuple);
	}
	
	//�����VAC session��Ϣ������(EID,CID)��CCB
	if(msgID>=MSGID_VAC_SESSION_BEGIN && msgID<=MSGID_VAC_SESSION_END)
	{
		CMsg_VACSessionIFMsg VACSessionMsg(msg);
		tuple.Eid = VACSessionMsg.GetEID();
		tuple.Cid = VACSessionMsg.GetCid();
		return FindCCBByEID_CID(tuple);
	}

	//�����Probe��Ϣ
	if(msgID==MSGID_MAC_VOICE_PROBERSP)
	{
		CMsg_ProbeRsp ProbeRsp(msg);
		tuple.Eid = ProbeRsp.GetEID();
		tuple.Cid = ProbeRsp.GetCid();
		return FindCCBByEID_CID(tuple);
	}
/*
	͸������Ϣ��ע��״̬�������Ա�����������͸������Ϣ
	//�����VAC voice control
	if(msgID==MSGID_VAC_VOICE_SIGNAL)
	{
		CMsg_UTSAGSignal_VAC VACSignal(msg);
		tuple.Eid = VACSignal.GetEID();
		tuple.Cid = VACSignal.GetCID();
		return FindCCBByEID_CID(tuple);
	}
	
	//�����DAC voice control
	if(msgID==MSGID_DAC_VOICE_SIGNAL)
	{
		LOG(LOG_DEBUG3, 0, "should not run here!!!");
		CMsg_UTSAGSignal_DAC DACSignal(msg);
		tuple.Eid = DACSignal.GetEID();
		tuple.Cid = DACSignal.GetCID();
		return FindCCBByEID_CID(tuple);
	}	
*/
	//������Ϣ����״̬��
	return NULL;
}
/*===========================================================================*/
//all states: idle,O-Establish,T-Establish,Transparent,Release,Paging,Probe
char VoiceStateBase::m_stateName[MAX_VOICE_STATE+1][20] = 
{
	"Idle State", "O-Establish state", "T-Establish state", 
	"Transparent state", "Release state", "Paging state", 
	"Probe state", "WaitSYNC state", "MAX state"
};

void VoiceStateBase::Entry(CCBBase& ccb)
{
	VoiceCCB *pCCB = (VoiceCCB *)&ccb;
	pCCB->setEntryStateFN(g_FrameNum);
	LOG2(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"CCB[UID=0x%08X] Enter [%s] state", 
		pCCB->getUID(),(int) GetStateName()); 
}
void VoiceStateBase::Exit(CCBBase& ccb)
{
	VoiceCCB *pCCB = (VoiceCCB *)&ccb;
	LOG2(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"CCB[UID=0x%08X] Exit [%s] state", 
		pCCB->getUID(), (int)GetStateName()); 
}

FSMTransIndex  VoiceIdleState::DoParseEvent(CMessage& msg)
{
	FSMTransIndex ret = INVALID_EVENT;
	UINT16 msgID = msg.GetMessageId();

	//if VAC setup(init) received, return VOICE_IDLE_VACSETUP
	if(msgID==MSGID_VAC_SETUP_NOTIFY)
		return VOICE_IDLE_VACSETUP;
	//if LA Paging received, retrun VOICE_IDLE_LAPAGING
	else if(msgID==MSGID_VCR_VOICE_SIGNAL)
	{
		CMsg_Signal_VCR signal(msg);
		SignalType sigType = signal.ParseMessageFromSAG();
		if(LAPaging_MSG==sigType)
			return VOICE_IDLE_LAPAGING;
	}			   
	
	return ret;
}
FSMTransIndex  VoiceOEstablishState::DoParseEvent(CMessage& msg)
{
	FSMTransIndex ret = INVALID_EVENT;
	UINT16 msgID = msg.GetMessageId();
	//if Assign transport resource Rsp received, return VOICE_OESTAB_ASSGN_RES_RSP
	if(msgID==MSGID_VCR_VOICE_SIGNAL)
	{
		CMsg_Signal_VCR signal(msg);
		SignalType sigType = signal.ParseMessageFromSAG();
		if(AssignResRsp_MSG==sigType)
			return VOICE_OESTAB_ASSGN_RES_RSP;
	}
	//if VAC release notify received, retrun VOICE_OESTAB_VAC_REL
	else if(msgID==MSGID_VAC_RLS_NOTIFY)
		return VOICE_OESTAB_VAC_REL;
	//if Tassign��ʱ received, return VOICE_OESTAB_TMOUT_ASSGN_RES_RSP
	else if(msgID==MSGID_TIMER_ASSIGN)
		return VOICE_OESTAB_TMOUT_ASSGN_RES_RSP;
	
	return ret;
}
FSMTransIndex  VoiceTEstablishState::DoParseEvent(CMessage& msg)
{
	FSMTransIndex ret = INVALID_EVENT;
	UINT16 msgID = msg.GetMessageId();

	//if Assign transport resource Req received, return VOICE_TESTAB_ASSGN_RES_REQ
	if(msgID==MSGID_VCR_VOICE_SIGNAL)
	{
		CMsg_Signal_VCR signal(msg);
		SignalType sigType = signal.ParseMessageFromSAG();
		if(sigType==AssignResRsp_MSG)
			return VOICE_TESTAB_ASSGN_RES_RSP;
        if(sigType==DELAPagingReq_MSG)
            return VOICE_TESTAB_DELAPAGING;
		if(sigType==ErrNotifyReq_MSG)
			return VOICE_TESTAB_ERR_REQ;
	}
	//if Tassign timeout msg received, return VOICE_TESTAB_TMOUT_ASSGN_RES_REQ	
	else if(msgID==MSGID_TIMER_ASSIGN)
		return VOICE_TESTAB_TMOUT_ASSGN_RES_RSP;
	//if VAC session release notify received, return VOICE_TESTAB_VAC_REL
	else if(msgID==MSGID_VAC_RLS_NOTIFY)
		return VOICE_TESTAB_VAC_REL;
	return ret;
}
FSMTransIndex  VoiceTransparentState::DoParseEvent(CMessage& msg)
{
	FSMTransIndex ret = INVALID_EVENT;
	UINT16 msgID = msg.GetMessageId();
	//if Release transport resource Req received, return VOICE_TRANSPARENT_REL_RES_REQ
	if(msgID==MSGID_VCR_VOICE_SIGNAL)
	{
		CMsg_Signal_VCR signal(msg);
		SignalType sigType = signal.ParseMessageFromSAG();
		if(sigType==RlsResReq_MSG)
			return VOICE_TRANSPARENT_REL_RES_REQ;		
	}
	//if VAC session release notify received, retrun VOICE_TRANSPARENT_VAC_REL
	else if(msgID==MSGID_VAC_RLS_NOTIFY)
		return VOICE_TRANSPARENT_VAC_REL;
	//if Modify VAC session notify received, return VOICE_TRANSPARENT_MODI_VAC_NOTIFY	
	else if(msgID==MSGID_VAC_MODIFY_NOTIFY)
		return VOICE_TRANSPARENT_MODI_VAC_NOTIFY;
	
	//�������������Ϊ�˸��õı�֤�տ��ؽ��ɹ�
	if(MSGID_VAC_SETUP_NOTIFY==msgID)
	{
		VoiceCCB* pCCB = (VoiceCCB*)CTVoice::GetInstance()->getVoiceFSM()->FindCCB(msg);
		if(pCCB!=NULL)
		{
			pCCB->sendVACSetupRsp(VACSETUP_RESULT_OK);
		}
	}



	return ret;
}
FSMTransIndex  VoiceReleaseState::DoParseEvent(CMessage& msg)
{
	FSMTransIndex ret = INVALID_EVENT;
	UINT16 msgID = msg.GetMessageId();
	//if Error notification Rsp received, return VOICE_RELEASE_ERR_RSP
	//if Release transport resource Req received, return VOICE_RELEASE_REL_RES_REQ
	if(msgID==MSGID_VCR_VOICE_SIGNAL)
	{
		CMsg_Signal_VCR signal(msg);
		SignalType sigType = signal.ParseMessageFromSAG();
		if(sigType==ErrNotifyRsp_MSG)
			return VOICE_RELEASE_ERR_RSP;
		if(sigType==RlsResReq_MSG)
			return VOICE_RELEASE_REL_RES_REQ;
	}
	//if Terrrsp��ʱ received, retrun VOICE_RELEASE_TMOUT_ERR_RSP
	else if(msgID==MSGID_TIMER_ERRRSP)
		return VOICE_RELEASE_TMOUT_ERR_RSP;
	//if Trelres��ʱ received, return VOICE_RELEASE_TMOUT_REL_RES	
	else if(msgID==MSGID_TIMER_RELRES)
		return VOICE_RELEASE_TMOUT_REL_RES;
	else if(msgID==MSGID_TIMER_DELAY_RELVAC)
		return VOICE_RELEASE_TMOUT_DELAY_RELVAC;

	return ret;
}
FSMTransIndex  VoicePagingState::DoParseEvent(CMessage& msg)
{
	FSMTransIndex ret = INVALID_EVENT;
	UINT16 msgID = msg.GetMessageId();
	//if VAC setup Rsp received, return VOICE_PAGING_VAC_SETUP_RSP
	if(msgID==MSGID_VAC_SETUP_RSP)
		return VOICE_PAGING_VAC_SETUP_RSP;
	//if Tvac��ʱ received, return VOICE_PAGING_TMOUT_VACSETUP_RSP
	if(msgID==MSGID_TIMER_VAC)
		return VOICE_PAGING_TMOUT_VACSETUP_RSP;
	//if VAC session release notify received, return VOICE_PAGING_VAC_REL	
	if(msgID==MSGID_VAC_RLS_NOTIFY)
		return VOICE_PAGING_VAC_REL;
	if(msgID==MSGID_VCR_VOICE_SIGNAL)
	{
		CMsg_Signal_VCR signal(msg);
		SignalType sigType = signal.ParseMessageFromSAG();
		if(sigType==DELAPagingReq_MSG)
			return VOICE_PAGING_DELAPAGING;
	}

	return ret;
}
FSMTransIndex  VoiceProbeState::DoParseEvent(CMessage& msg)
{
	FSMTransIndex ret = INVALID_EVENT;
	UINT16 msgID = msg.GetMessageId();
	//if ProbeRsp received, return VOICE_PROBE_PROBERSP
	if(msgID==MSGID_MAC_VOICE_PROBERSP)
		return VOICE_PROBE_PROBERSP;
	//if Tprobersp��ʱ,return VOICE_PROBE_TMOUT_PROBERSP	
	if(msgID==MSGID_TIMER_PROBERSP)
		return VOICE_PROBE_TMOUT_PROBERSP;

	return ret;
}

FSMTransIndex  VoiceWaitSYNCState::DoParseEvent(CMessage& msg)
{
	FSMTransIndex ret = INVALID_EVENT;
	UINT16 msgID = msg.GetMessageId();

	//if Twaitsync
	if(msgID==MSGID_TIMER_WAITSYNC)
		return VOICE_WAITSYNC_TMOUT_WAITSYNC;
	//if VAC Release notify 
	if(msgID==MSGID_VAC_RLS_NOTIFY)
		return VOICE_WAITSYNC_VAC_REL;
	//if VAC setup Req
	if(msgID==MSGID_VAC_SETUP_NOTIFY)
		return VOICE_WAITSYNC_VACSETUP;
	//if release transport resource req
	if(msgID==MSGID_VCR_VOICE_SIGNAL)
	{
		CMsg_Signal_VCR signal(msg);
		SignalType sigType = signal.ParseMessageFromSAG();
		if(sigType==RlsResReq_MSG)
			return VOICE_WAITSYNC_REL_RES_REQ;
	}
	return ret;
}
void VoiceIdleState::Entry(CCBBase &ccb)
{
	VoiceStateBase::Entry(ccb);
	VoiceCCB *pCCB = (VoiceCCB *)&ccb;
	//release voice data buffer 
	deallocVDataBuf(pCCB->m_vDataIdx);
	pCCB->m_vDataIdx = M_INVALID_VDATABUF_IDX;
	//L3 Addr ����
	VoiceCCBTable* pCCBTab = pCCB->getCCBTable();
	pCCBTab->DelBTreeL3addr(pCCB->getL3addr());
	pCCB->setL3addr(NO_L3ADDR);	
//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
	pCCB->disableInnerSwitch();
#endif
//20090531 fengbing bts inner switch for Voice Data end

	pCCB->setAppType(APPTYPE_VOICE_G729);
	pCCB->setCodecInfo(CODEC_G729A);
	pCCB->setAppPrio(M_DEFAULT_VOICE_PRIORITY);	//Ŀǰ�汾�������ȼ��̶�Ϊ
	
	//pCCB->ClearTimer();	//20090723 fengbing del
	pCCB->deleteTimer();		//20090723 fengbing add
	pCCB->deleteTimerWaitSYNC();//20090723 fengbing add

	pCCB->clearConnectedFlag();
}

/*===========================================================================*/
//all transitions

FSMStateIndex Trans_Idle_VACSetupCmd::Action(VoiceCCB &ccb, CMessage &msg)
{
	//���SAG����������ӵ������ֱ���ͷſտڣ��ܾ�����
	if(CONGEST_LEVEL2<=CTVoice::GetInstance()->getSAGCongestLevel())
	{
		//ccb.VACRelease();
		ccb.sendVACSetupRsp(VACSETUP_RESULT_L3_REJECT);
		return VOICE_IDLE_STATE;
	}
	//�������·�����࣬��ֱ���ͷſտڣ��ܾ�����
	UINT16 vBufID = allocVDataBuf();
	if(vBufID>=M_MAX_CALLS)
	{
		ccb.sendVACSetupRsp(VACSETUP_RESULT_L3_REJECT);
		return VOICE_IDLE_STATE;
	}
	else
	{
		ccb.m_vDataIdx = vBufID;
	}
	
	CMsg_VACSetupNotify VACSetupNoti(msg);
	//��tVCR����Assign transport resource Req��Ϣ
	CMsg_Signal_VCR AssgnResReq;
	VoiceVCRCtrlMsgT *pDataAssgnResReq;
	if ( !AssgnResReq.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(AssignResReqT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		pDataAssgnResReq = (VoiceVCRCtrlMsgT*)AssgnResReq.GetDataPtr();

		AssgnResReq.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		AssgnResReq.SetDstTid(M_TID_VCR);
		AssgnResReq.SetBTSSAGID();
		AssgnResReq.SetSigIDS(AssignResReq_MSG);
		VSetU32BitVal(pDataAssgnResReq->sigPayload.AssignResReq.L3addr , (ccb.getL3addr()));	//��ʱ��д0xffffffff
		pDataAssgnResReq->sigPayload.AssignResReq.ReqRate = 0;//(VACSetupNoti.GetRate()==8 ? 1:2);
		if(M_VACSETUPREASON_HANDOVER==VACSetupNoti.GetReason() || 
			M_VACSETUPREASON_ENC_HANDOVER==VACSetupNoti.GetReason())
		{
			VSetU16BitVal(pDataAssgnResReq->sigPayload.AssignResReq.ServerOption , (APPTYPE_VOICE_SWITCH));
		}
		else
		{
			VSetU16BitVal(pDataAssgnResReq->sigPayload.AssignResReq.ServerOption , (APPTYPE_VOICE_G729));
		}
//20121115Modify by fengbing begin
//��¼VAC����ԭ��localSag��ʹ��
//����޸���Ϊ�˱����վ�ڲ���չ���ֶ�ֵ���͸�sag�����Ϣ��������
		AssgnResReq.SetEID(VACSetupNoti.GetReason());
#if 0
//20100112fengbingΪ�˼򻯼�Ⱥ��������begin
		if(!sagStatusFlag)
		{
			if(M_VACSETUPREASON_GRP_SETUP==VACSetupNoti.GetReason() ||
				M_VACSETUPREASON_ENC_GRP_SETUP==VACSetupNoti.GetReason())
			{
				VSetU16BitVal(pDataAssgnResReq->sigPayload.AssignResReq.ServerOption , 
					(M_LOCALSAG_ASSIGNRES_REASON_GRP_SETUP));
			}
			if(M_VACSETUPREASON_GRP_TALKING==VACSetupNoti.GetReason() ||
				M_VACSETUPREASON_ENC_GRP_TALKING==VACSetupNoti.GetReason())
			{
				VSetU16BitVal(pDataAssgnResReq->sigPayload.AssignResReq.ServerOption , 
					(M_LOCALSAG_ASSIGNRES_REASON_GRP_TALKING));
			}
		}
//20100112fengbingΪ�˼򻯼�Ⱥ��������end
#endif
//20121115Modify by fengbing end

		pDataAssgnResReq->sigPayload.AssignResReq.ServeAppType = SERV_APPTYPE_DTE;
		VSetU32BitVal(pDataAssgnResReq->sigPayload.AssignResReq.UID , (ccb.getUID()));
		AssgnResReq.SetSigHeaderLengthField(sizeof(AssignResReqT));
		AssgnResReq.SetPayloadLength(sizeof(SigHeaderT)+sizeof(AssignResReqT));
		AssgnResReq.Post();
	}    
	//������ʱ��Tassign;
	CMsg_VoiceTimeout TassignMsg;
	if ( !TassignMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TassignMsg.SetTimerType(TIMER_ASSIGN);
		TassignMsg.SetMessageId(MSGID_TIMER_ASSIGN);
		ccb.startTimer(0, M_TIMERLEN_ASSIGN, TassignMsg);

		LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(), (int)"Tassign");
	}    

	//����Ŀ��״̬O-Establish
	return VOICE_O_ESTABLISH_STATE;
}
FSMStateIndex Trans_Idle_LAPagingReq::Action(VoiceCCB &ccb, CMessage &msg)
{
	//��������������������ʾѰ������������(���Ը���UID�ҵ�CCB)
	VoiceVCRCtrlMsgT *pDataLAPagingReq = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT32 L3addr = VGetU32BitVal(pDataLAPagingReq->sigPayload.LAPaging.L3addr);
	//����AppType,����LAPagingRspʱʹ��
	UINT16 AppType = VGetU16BitVal(pDataLAPagingReq->sigPayload.LAPaging.App_Type);
	ccb.setAppType(AppType);

	//SMS or �I��Ѱ��
	if(APPTYPE_GRP_MANAGE==AppType || 
		APPTYPE_SMS==AppType || 
		APPTYPE_ENC_SIGNAL_SEND==AppType ||
		APPTYPE_LOWSPEED_DATA==AppType ||
		APPTYPE_DUPCPE_PAGING==AppType)	
	{
		ccb.setPagingL3Addr(L3addr);
		//����Probe Req,Page only
		CMsg_ProbeReq ProbeReq;
		if ( !ProbeReq.CreateMessage(*CTVoice::GetInstance()) )
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		}
		else
		{
			ProbeReq.SetDstTid(M_TID_VAC);	//Ŀ������M_TID_L2OAM
			ProbeReq.SetMessageId(MSGID_VOICE_MAC_PROBEREQ);
			ProbeReq.SetEID(ccb.getVoiceTuple().Eid);
			ProbeReq.SetCid(ccb.getVoiceTuple().Cid);
			if(!ProbeReq.Post())
			{
				ProbeReq.DeleteMessage();
			}
		}        

		//������ʱ��Tprobersp
		CMsg_VoiceTimeout TproberspMsg;
		if ( !TproberspMsg.CreateMessage(*CTVoice::GetInstance()) )
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		}
		else
		{
			TproberspMsg.SetTimerType(TIMER_PROBERSP);
			TproberspMsg.SetMessageId(MSGID_TIMER_PROBERSP);
			ccb.startTimer(0, M_TIMERLEN_PROBERSP, TproberspMsg);
    		
			LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(), (int)"Tprobersp");
		}        
		//����Ŀ��״̬Probe
		return VOICE_PROBE_STATE;
	}
	if(APPTYPE_VOICE_QCELP==AppType || APPTYPE_ENCRYPT_VOICE==AppType)
	{
		//�������·�����࣬��ֱ���ͷſտ�
		UINT16 vBufID = allocVDataBuf();
		if(vBufID>=M_MAX_CALLS)
		{
			return VOICE_IDLE_STATE;
		}
		else
		{
			ccb.m_vDataIdx = vBufID;
		}		
		
		//����VAC setup cmd(init)��tVAC;
		UINT8 reason = (APPTYPE_ENCRYPT_VOICE==AppType) ? M_VACSETUPREASON_ENC_CALL : M_VACSETUPREASON_CALL;
		ccb.sendVACSetupReq(reason, M_RATE_8K);
		
		//������ʱ��Tvac;
		CMsg_VoiceTimeout TvacMsg;
		if ( !TvacMsg.CreateMessage(*CTVoice::GetInstance()) )
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		}
		else
		{
			TvacMsg.SetMessageId(MSGID_TIMER_VAC);
			TvacMsg.SetTimerType(TIMER_VAC);
			ccb.startTimer(0, M_TIMERLEN_VAC, TvacMsg);

			LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(),(int) "Tvac");
		}        
		//ccb����L3addr;
		ccb.setL3addr(L3addr);
		ccb.setAppType(VGetU16BitVal(pDataLAPagingReq->sigPayload.LAPaging.App_Type));//����AppType
		//����BTree L3addr
		VoiceCCBTable* pCCBTab = ccb.getCCBTable();
		pCCBTab->AddBTreeL3addr(L3addr, ccb.getTabIndex());
		initVDataBufCCBInfo(ccb);
		//����Ŀ��״̬Paging
		return VOICE_PAGING_STATE;
	}

	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_OEstab_AssignTransResRsp::Action(VoiceCCB &ccb, CMessage &msg)
{
	VoiceVCRCtrlMsgT *pDataAssgnResRsp = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	//ͣTassign;
	ccb.deleteTimer();
	if(pDataAssgnResRsp->sigPayload.AssignResRsp.AssignResult==0)	//Ack==0
	{	
		//��¼L3Addr�͸���BTree
		ccb.setL3addr(VGetU32BitVal(pDataAssgnResRsp->sigPayload.AssignResRsp.L3addr));
		VoiceCCBTable *pCCBTab = ccb.getCCBTable();
		pCCBTab->AddBTreeL3addr(ccb.getL3addr(), ccb.getTabIndex());
		initVDataBufCCBInfo(ccb);
		//����VAC setup Rsp(Ack);
		ccb.sendVACSetupRsp(VACSETUP_RESULT_OK);
		return VOICE_TRANSPARENT_STATE;
	}
	else	//Nak
	{
		ccb.sendVACSetupRsp(VACSETUP_RESULT_SAG_REJECT);
		return VOICE_IDLE_STATE;
	}
}
FSMStateIndex Trans_OEstab_VACRelNotify::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣTassign;
	ccb.deleteTimer();
	//����Error notification Req��tVCR
	ccb.sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_AIRFAIL);		
	return VOICE_IDLE_STATE;
}
FSMStateIndex Trans_OEstab_TassignTimeout::Action(VoiceCCB &ccb, CMessage &msg)
{
	ccb.deleteTimer();	//ɾ����ʱ��
	//����ʧ�ܵ�VACSetupRsp��CPE
	ccb.sendVACSetupRsp(VACSETUP_RESULT_L3_TMO);
	//����Error notification Req��tVCR
	ccb.sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_CPERELEASE);		
	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_TEstab_AssignTransResRsp::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣTassign;
	ccb.deleteTimer();	
	VoiceVCRCtrlMsgT *pDataAssgnResRsp = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	if(pDataAssgnResRsp->sigPayload.AssignResRsp.AssignResult==0)	//Ack==0
	{	
		//��¼L3Addr�͸���BTree
		ccb.setL3addr(VGetU32BitVal(pDataAssgnResRsp->sigPayload.AssignResRsp.L3addr));
		VoiceCCBTable *pCCBTab = ccb.getCCBTable();
		pCCBTab->AddBTreeL3addr(ccb.getL3addr(), ccb.getTabIndex());
		initVDataBufCCBInfo(ccb);
		//����VAC setup Rsp(Ack);
		//ccb.sendVACSetupRsp(VACSETUP_RESULT_OK);del by fengbing 20061103
		return VOICE_TRANSPARENT_STATE;
	}
	else	//Nak
	{
		//����VAC release cmd;
		ccb.VACRelease();
		return VOICE_IDLE_STATE;
	}
}
FSMStateIndex Trans_TEstab_TassignTimeout::Action(VoiceCCB &ccb, CMessage &msg)
{
	ccb.deleteTimer();	//ɾ����ʱ��
	//����VAC release cmd��VAC
	ccb.VACRelease();
	//����Error notification Req��tVCR
	ccb.sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_CPERELEASE);
	//����Ŀ��״̬Idle
	return VOICE_IDLE_STATE;
}
FSMStateIndex Trans_TEstab_VACRel::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣTassign;
	ccb.deleteTimer();
	//����Error notification Req��tVCR
	ccb.sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_AIRFAIL);
	//����Ŀ��״̬Idle
	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_TEstab_DeLAPaging::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣ��ʱ��
	ccb.deleteTimer();
	//����VAC release cmd��VAC
	ccb.VACRelease();
	//����Ŀ��״̬Idle
	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_TEstab_ErrNotifyReq::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣ��ʱ��
	ccb.deleteTimer();
	//��ӦError Notification Rsp��SAG
	CMsg_Signal_VCR ErrNotiRsp;
	VoiceVCRCtrlMsgT *pDataRsp, *pDataReq;
	if(!ErrNotiRsp.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(ErrNotifyRspT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		ErrNotiRsp.SetDstTid(M_TID_VCR);
		ErrNotiRsp.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		ErrNotiRsp.SetBTSSAGID();
		ErrNotiRsp.SetSigIDS(ErrNotifyRsp_MSG);

		pDataRsp = (VoiceVCRCtrlMsgT*)ErrNotiRsp.GetDataPtr();
		pDataReq = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
		memcpy(pDataRsp->sigPayload.ErrNotifyRsp.L3Addr , pDataReq->sigPayload.ErrNotifyReq.L3Addr, 4);
		memcpy(pDataRsp->sigPayload.ErrNotifyRsp.Uid , pDataReq->sigPayload.ErrNotifyReq.Uid, 4);

		ErrNotiRsp.SetSigHeaderLengthField(sizeof(ErrNotifyRspT));
		ErrNotiRsp.SetPayloadLength(sizeof(SigHeaderT)+sizeof(ErrNotifyRspT));
		ErrNotiRsp.Post();
	}
	//����VAC release cmd��VAC
	ccb.VACRelease();
	//����Ŀ��״̬Idle
	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_Transparent_RelTransResReq::Action(VoiceCCB &ccb, CMessage &msg)
{

	//������ʱ��TdelayRelVac;
	CMsg_VoiceTimeout TdelayRelVac;
	if ( !TdelayRelVac.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TdelayRelVac.SetMessageId(MSGID_TIMER_DELAY_RELVAC);
		TdelayRelVac.SetTimerType(TIMER_DELAY_RELVAC);
		ccb.startTimer(0, M_TIMERLEN_DELAY_RELVAC, TdelayRelVac);
	
		LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(),(int) "TDelayRelVac");
	}

	//����Release transport resource Rsp��tVCR
	ccb.sendRlsResRsp();

	return VOICE_RELEASE_STATE;
}
FSMStateIndex Trans_Transparent_VACRelNotify::Action(VoiceCCB &ccb, CMessage &msg)
{
	VACRlsCmdNotifyT* pDataRlsNoti = (VACRlsCmdNotifyT*)msg.GetDataPtr();
	//���ҽ����տ�ʧ������ͨ���Ѿ�����ʱ��֧���쳣�л�������������
	if( VAC_REL_REASON_MAC==pDataRlsNoti->reason &&	
		ccb.isConnected())
	{
		ccb.startTimerWaitSYNC();
		return VOICE_WAITSYNC_STATE;
	}
	
	//����Error notification Req��tVCR
	ccb.sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_AIRFAIL);

   //20120615 ͬ�������ͷ�begin
	ccb.deleteTimer();	//ɾ����ʱ��
	return VOICE_IDLE_STATE;
#if 0
	//������ʱ��Terrrsp
	CMsg_VoiceTimeout TerrrspMsg;
	if ( !TerrrspMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TerrrspMsg.SetMessageId(MSGID_TIMER_ERRRSP);
		TerrrspMsg.SetTimerType(TIMER_ERRRSP);
		ccb.startTimer(0, M_TIMERLEN_ERRRSP,TerrrspMsg);

		LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(), (int)"Terrrsp");
	}    
	return VOICE_RELEASE_STATE;
#endif
//20120615 ͬ�������ͷ�end
}
FSMStateIndex Trans_Transparent_ModifyVACNotify::Action(VoiceCCB &ccb, CMessage &msg)
{
	//����Modify VAC session Rsp��VAC;
	CMsg_VACModifyReq VACModiReq(msg);
	CMsg_VACModifyRsp VACModiRsp;
	//VACModiCmdRspT *pDataVACModiRsp;
	if ( !VACModiRsp.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		VACModiRsp.SetMessageId(MSGID_VAC_MODIFY_RSP);
		VACModiRsp.SetDstTid(M_TID_VAC);
		VACModiRsp.SetEID(ccb.getVoiceTuple().Eid);
		VACModiRsp.SetCid(ccb.getVoiceTuple().Cid);
		VACModiRsp.SetResult(0);
		VACModiRsp.SetPayloadLen(sizeof(VACModiCmdRspT));
		VACModiRsp.Post();
	}    
/*
	//VAC session start;
	CMsg_VACStart vacStart;
	vacStart.CreateMessage(*CTVoice::GetInstance());
	vacStart.SetDstTid(M_TID_VAC);
	vacStart.SetMessageId(MSGID_VAC_START);
	vacStart.SetEID(ccb.getVoiceTuple().Eid);
	vacStart.SetCid(ccb.getVoiceTuple().Cid);
	vacStart.SetPayloadLen(sizeof(VACStartT));
	vacStart.Post();*/

	//���뷽ʽ����
	if(VACModiReq.GetRate()==64)
	{
		ccb.setCodecInfo(CODEC_G711A);
		LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "UID[0x%08X], Codec Changed to G711", ccb.getUID());
	}
	if(VACModiReq.GetRate()==8)
	{
		ccb.setCodecInfo(CODEC_G729A);
		LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "UID[0x%08X], Codec Changed to G729", ccb.getUID());
	}
	return VOICE_TRANSPARENT_STATE;
}

FSMStateIndex Trans_Release_TrelvacTimeout::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣ��ʱ��TdelayRelVac
	ccb.deleteTimer();
	//�ͷ�VAC session
	ccb.VACRelease();
	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_Release_ErrNotifyRsp::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣ��ʱ��Terrrsp
	ccb.deleteTimer();
	//������ʱ��Trelres
	CMsg_VoiceTimeout TrelresMsg;
	if ( !TrelresMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TrelresMsg.SetMessageId(MSGID_TIMER_RELRES);
		TrelresMsg.SetTimerType(TIMER_RELRES);
		ccb.startTimer(0, M_TIMERLEN_RELRES,TrelresMsg);

		LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(), (int)"Trelres");
	}    
	return VOICE_RELEASE_STATE;
}
FSMStateIndex Trans_Release_TerrrspTimeout::Action(VoiceCCB &ccb, CMessage &msg)
{
	ccb.deleteTimer();	//ɾ����ʱ��
	return VOICE_IDLE_STATE;
}
FSMStateIndex Trans_Release_RlsTransResReq::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣ��ʱ��Trelres
	ccb.deleteTimer();
	//����Release transport resource Rsp��tVCR
	ccb.sendRlsResRsp();    
	return VOICE_IDLE_STATE;
}
FSMStateIndex Trans_Release_TrelresTimeout::Action(VoiceCCB &ccb, CMessage &msg)
{
	ccb.deleteTimer();	//ɾ����ʱ��
	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_Paging_VACSetupRsp::Action(VoiceCCB &ccb, CMessage &msg)
{
	UINT8 VACSetupResult;
	//ͣTvac;
	ccb.deleteTimer();
	CMsg_VACSetupRsp VACSetupRsp(msg);
	VACSetupResult = VACSetupRsp.GetResult();

	//����Ѱ��ʧ�ܣ��а��ղŻ�Ѱ��ʧ��
	if(VACSETUP_RESULT_OK==VACSetupResult || VACSETUP_RESULT_L3_REJECT==VACSetupResult)
	{
		//����Paging Rsp(success or fail)��tVCR
		CMsg_Signal_VCR PagingRsp;
		VoiceVCRCtrlMsgT *pData;
		if ( !PagingRsp.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(LAPagingRspT)) )
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		}
		else
		{
			PagingRsp.SetDstTid(M_TID_VCR);
			PagingRsp.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
			PagingRsp.SetBTSSAGID();
			PagingRsp.SetSigIDS(LAPagingRsp_MSG);

			pData = (VoiceVCRCtrlMsgT*)PagingRsp.GetDataPtr();
			pData->sigPayload.LAPagingRsp.Cause = (VACSetupResult==VACSETUP_RESULT_OK) ? M_LAPAGING_CAUSE_SUCCESS : M_LAPAGING_CAUSE_HOOKOFF;	//0�ɹ�,����fail
			VSetU32BitVal(pData->sigPayload.LAPagingRsp.UID , ccb.getUID());
			VSetU32BitVal(pData->sigPayload.LAPagingRsp.L3addr , ccb.getL3addr());
			VSetU16BitVal(pData->sigPayload.LAPagingRsp.App_Type , APPTYPE_VOICE_G729);//htons(ccb.getAppType());

			PagingRsp.SetSigHeaderLengthField(sizeof(LAPagingRspT));
			PagingRsp.SetPayloadLength(sizeof(SigHeaderT)+sizeof(LAPagingRspT));
			PagingRsp.Post();
		}    
	}

    
	if(VACSetupResult!=0)	//Paging Fail
	{
		//����Ŀ��״̬Idle
		return VOICE_IDLE_STATE;
	}
	
	CMsg_VACSetupNotify VACSetupNoti(msg);
	//��tVCR����Assign transport resource Req��Ϣ
	CMsg_Signal_VCR AssgnResReq;
	VoiceVCRCtrlMsgT *pDataAssgnResReq;
	if ( !AssgnResReq.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(AssignResReqT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		pDataAssgnResReq = (VoiceVCRCtrlMsgT*)AssgnResReq.GetDataPtr();

		AssgnResReq.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		AssgnResReq.SetDstTid(M_TID_VCR);
		AssgnResReq.SetBTSSAGID();
		AssgnResReq.SetSigIDS(AssignResReq_MSG);
		VSetU32BitVal(pDataAssgnResReq->sigPayload.AssignResReq.L3addr , ccb.getL3addr());	
		pDataAssgnResReq->sigPayload.AssignResReq.ReqRate = 0;//(VACSetupNoti.GetRate()==8 ? 1:2);
		//pDataAssgnResReq->sigPayload.AssignResReq.AssignReason = htons(VACSetupNoti.GetReason());	//voice call or handover
		VSetU16BitVal(pDataAssgnResReq->sigPayload.AssignResReq.ServerOption , APPTYPE_VOICE_G729);
		pDataAssgnResReq->sigPayload.AssignResReq.ServeAppType = SERV_APPTYPE_DTE;
		VSetU32BitVal(pDataAssgnResReq->sigPayload.AssignResReq.UID , ccb.getUID());
		AssgnResReq.SetSigHeaderLengthField(sizeof(AssignResReqT));
		AssgnResReq.SetPayloadLength(sizeof(SigHeaderT)+sizeof(AssignResReqT));
		AssgnResReq.Post();
	}    
	
	//������ʱ��Tassign
	CMsg_VoiceTimeout TassignMsg;
	if ( !TassignMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TassignMsg.SetTimerType(TIMER_ASSIGN);
		TassignMsg.SetMessageId(MSGID_TIMER_ASSIGN);
		ccb.startTimer(0, M_TIMERLEN_ASSIGN, TassignMsg);
		LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(), (int)"Tassign");
	}    
	//����Ŀ��״̬T_Establish
	return VOICE_T_ESTABLISH_STATE;
}
FSMStateIndex Trans_Paging_TvacTimeout::Action(VoiceCCB &ccb, CMessage &msg)
{
	ccb.deleteTimer();	//ɾ����ʱ��
	//����Ѱ��ʧ�ܣ��а��ղŻ�Ѱ��ʧ��
	//����VAC release cmd��VAC
	ccb.VACRelease();
	//����Ŀ��״̬Idle
	return VOICE_IDLE_STATE;
}
FSMStateIndex Trans_Paging_VACRelNotify::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣTvac
	ccb.deleteTimer();
	//����Ѱ��ʧ�ܣ��а��ղŻ�Ѱ��ʧ��	
	//����VAC release cmd��VAC
	ccb.VACRelease();
	//����Ŀ��״̬Idle
	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_Paging_DeLaPaging::Action(VoiceCCB &ccb, CMessage &msg)
{
	//ͣTvac
	ccb.deleteTimer();
	//����VAC release cmd��VAC
	ccb.VACRelease();
	return VOICE_IDLE_STATE;
}

FSMStateIndex Trans_Probe_ProbeRsp::Action(VoiceCCB &ccb, CMessage &msg)
{
	CMsg_ProbeRsp ProbeRsp(msg);
	//ͣTprobersp
	ccb.deleteTimer();
	//����Ѱ��ʧ�ܣ��а��ղŻ�Ѱ��ʧ��
	if(0==ProbeRsp.GetProbeResult())
	{
		//����Paging Rsp(success/fail)��tVCR
		CMsg_Signal_VCR PagingRsp;
		VoiceVCRCtrlMsgT *pData;
		if ( !PagingRsp.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(LAPagingRspT)) )
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		}
		else
		{
			PagingRsp.SetDstTid(M_TID_VCR);
			PagingRsp.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
			PagingRsp.SetBTSSAGID();
			PagingRsp.SetSigIDS(LAPagingRsp_MSG);

			pData = (VoiceVCRCtrlMsgT*)PagingRsp.GetDataPtr();
			pData->sigPayload.LAPagingRsp.Cause = ProbeRsp.GetProbeResult();	//�ɹ�/ʧ��
			VSetU32BitVal(pData->sigPayload.LAPagingRsp.UID , ccb.getUID());
			VSetU32BitVal(pData->sigPayload.LAPagingRsp.L3addr , ccb.getPagingL3Addr());	//Paging L3addr
			VSetU16BitVal(pData->sigPayload.LAPagingRsp.App_Type , ccb.getAppType());

			PagingRsp.SetSigHeaderLengthField(sizeof(LAPagingRspT));
			PagingRsp.SetPayloadLength(sizeof(SigHeaderT)+sizeof(LAPagingRspT));
			PagingRsp.Post();
		}    
	}
	//����Ŀ��״̬Idle
	return VOICE_IDLE_STATE;
}
FSMStateIndex Trans_Probe_TproberspTimeout::Action(VoiceCCB &ccb, CMessage &msg)
{
	ccb.deleteTimer();	//ɾ����ʱ��
	//����Ѱ��ʧ�ܣ��а��ղŻ�Ѱ��ʧ��
	//����Ŀ��״̬Idle
	return VOICE_IDLE_STATE;
}


FSMStateIndex Trans_WaitSYNC_TwaisyncTimeout::Action(VoiceCCB & ccb, CMessage & msg)
{
	//ͣTwaitsync;
	ccb.deleteTimerWaitSYNC();
	//����Error notification Req��tVCR
	ccb.sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_AIRFAIL);
	//������ʱ��Terrrsp
	CMsg_VoiceTimeout TerrrspMsg;
	if ( !TerrrspMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TerrrspMsg.SetMessageId(MSGID_TIMER_ERRRSP);
		TerrrspMsg.SetTimerType(TIMER_ERRRSP);
		ccb.startTimer(0, M_TIMERLEN_ERRRSP,TerrrspMsg);

		LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(), (int)"Terrrsp");
	}    
	return VOICE_RELEASE_STATE;

}
FSMStateIndex Trans_WaitSYNC_VACRelNotify::Action(VoiceCCB & ccb, CMessage & msg)
{
	//ͣTwaitsync;
	ccb.deleteTimerWaitSYNC();
	//����Error notification Req��tVCR
	ccb.sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_AIRFAIL);
	//������ʱ��Terrrsp
	CMsg_VoiceTimeout TerrrspMsg;
	if ( !TerrrspMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TerrrspMsg.SetMessageId(MSGID_TIMER_ERRRSP);
		TerrrspMsg.SetTimerType(TIMER_ERRRSP);
		ccb.startTimer(0, M_TIMERLEN_ERRRSP,TerrrspMsg);

		LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "StartTimer, UID[0x%08X], TimerType[%s]", ccb.getUID(), (int)"Terrrsp");
	}    
	return VOICE_RELEASE_STATE;

}
FSMStateIndex Trans_WaitSYNC_VACSetupCmd::Action(VoiceCCB & ccb, CMessage & msg)
{
	//ͣTwaitsync;
	ccb.deleteTimerWaitSYNC();
	//ͣTmoveaway_disable;
	ccb.deleteTimerMoveAwayDisable();
	//VAC Setup Rsp(Ack)
	ccb.sendVACSetupRsp(VACSETUP_RESULT_OK);
	return VOICE_TRANSPARENT_STATE;
}
FSMStateIndex Trans_WaitSYNC_RelTransResReq::Action(VoiceCCB & ccb, CMessage & msg)
{
	//ͣTwaitsync
	ccb.deleteTimerWaitSYNC();
	//VAC Release
	ccb.VACRelease();
	//Release transport resource Rsp
	ccb.sendRlsResRsp();
	return VOICE_IDLE_STATE;
}
char VoiceTrans::m_transName[MAX_VOICE_TRANS][40]=
{
	//all transitions
	"Trans_Idle_VACSetupCmd",
	"Trans_Idle_LAPagingReq",

	"Trans_OEstab_AssignTransResRsp",
	"Trans_OEstab_VACRelNotify",
	"Trans_OEstab_TassignTimeout",
	
	"Trans_TEstab_AssignTransResRsp",
	"Trans_TEstab_TassignTimeout",
	"Trans_TEstab_VACRel",
	"Trans_TEstab_DeLAPaging",
	"Trans_TEstab_ErrNotifyReq",
	
	"Trans_Transparent_RelTransResReq",
	"Trans_Transparent_VACRelNotify",
	"Trans_Transparent_ModifyVACNotify",

	"Trans_Release_TrelvacTimeout",
	"Trans_Release_ErrNotifyRsp",
	"Trans_Release_TerrrspTimeout",
	"Trans_Release_RlsTransResReq",
	"Trans_Release_TrelresTimeout",

	"Trans_Paging_VACSetupRsp",
	"Trans_Paging_TvacTimeout",
	"Trans_Paging_VACRelNotify",
	"Trans_Paging_DeLaPaging",

	"Trans_Probe_ProbeRsp",
	"Trans_Probe_TproberspTimeout",

	"Trans_WaitSYNC_TwaisyncTimeout",
	"Trans_WaitSYNC_VACRelNotify",
	"Trans_WaitSYNC_VACSetupCmd",
	"Trans_WaitSYNC_RelTransResReq"
};
/*===========================================================================*/
//VoiceFSM
VoiceFSM::VoiceFSM(): FSM(MAX_VOICE_STATE,MAX_VOICE_TRANS)
{
	m_LocalCongestLevel = 0;
	m_RemoteCongestLevel = 0;

	//CCBTable
	CCBTable = new VoiceCCBTable;
	pGrpCCBTbl = new GrpCCBTable;
	//GrpCCBTbl.init();

	//all States
	m_pStateTable[VOICE_IDLE_STATE] = new VoiceIdleState;
	m_pStateTable[VOICE_O_ESTABLISH_STATE] = new VoiceOEstablishState;
	m_pStateTable[VOICE_T_ESTABLISH_STATE] = new VoiceTEstablishState;
	m_pStateTable[VOICE_TRANSPARENT_STATE] = new VoiceTransparentState;
	m_pStateTable[VOICE_RELEASE_STATE] = new VoiceReleaseState;
	m_pStateTable[VOICE_PAGING_STATE] = new VoicePagingState;
	m_pStateTable[VOICE_PROBE_STATE] = new VoiceProbeState;
	m_pStateTable[VOICE_WAITSYNC_STATE] = new VoiceWaitSYNCState;

	//all transitions
	m_pTransTable[VOICE_IDLE_VACSETUP] = new Trans_Idle_VACSetupCmd(VOICE_O_ESTABLISH_STATE, VOICE_IDLE_VACSETUP);
	m_pTransTable[VOICE_IDLE_LAPAGING] = new Trans_Idle_LAPagingReq(VOICE_PAGING_STATE, VOICE_IDLE_LAPAGING);

	m_pTransTable[VOICE_OESTAB_ASSGN_RES_RSP] = new Trans_OEstab_AssignTransResRsp(VOICE_TRANSPARENT_STATE, VOICE_OESTAB_ASSGN_RES_RSP);
	m_pTransTable[VOICE_OESTAB_VAC_REL] = new Trans_OEstab_VACRelNotify(VOICE_IDLE_STATE, VOICE_OESTAB_VAC_REL);
	m_pTransTable[VOICE_OESTAB_TMOUT_ASSGN_RES_RSP] = new Trans_OEstab_TassignTimeout(VOICE_IDLE_STATE, VOICE_OESTAB_TMOUT_ASSGN_RES_RSP);
	
	m_pTransTable[VOICE_TESTAB_ASSGN_RES_RSP] = new Trans_TEstab_AssignTransResRsp(VOICE_TRANSPARENT_STATE, VOICE_TESTAB_ASSGN_RES_RSP);
	m_pTransTable[VOICE_TESTAB_TMOUT_ASSGN_RES_RSP] = new Trans_TEstab_TassignTimeout(VOICE_IDLE_STATE, VOICE_TESTAB_TMOUT_ASSGN_RES_RSP);
	m_pTransTable[VOICE_TESTAB_VAC_REL] = new Trans_TEstab_VACRel(VOICE_IDLE_STATE, VOICE_TESTAB_VAC_REL);
	m_pTransTable[VOICE_TESTAB_DELAPAGING] = new Trans_TEstab_DeLAPaging(VOICE_IDLE_STATE, VOICE_TESTAB_DELAPAGING);
	m_pTransTable[VOICE_TESTAB_ERR_REQ] = new Trans_TEstab_ErrNotifyReq(VOICE_IDLE_STATE, VOICE_TESTAB_ERR_REQ);
	
	m_pTransTable[VOICE_TRANSPARENT_REL_RES_REQ] = new Trans_Transparent_RelTransResReq(VOICE_TRANSPARENT_STATE, VOICE_TRANSPARENT_REL_RES_REQ);
	m_pTransTable[VOICE_TRANSPARENT_VAC_REL] = new Trans_Transparent_VACRelNotify(VOICE_RELEASE_STATE, VOICE_TRANSPARENT_VAC_REL);
	m_pTransTable[VOICE_TRANSPARENT_MODI_VAC_NOTIFY] = new Trans_Transparent_ModifyVACNotify(VOICE_TRANSPARENT_STATE, VOICE_TRANSPARENT_MODI_VAC_NOTIFY);

	m_pTransTable[VOICE_RELEASE_TMOUT_DELAY_RELVAC] = new Trans_Release_TrelvacTimeout(VOICE_IDLE_STATE, VOICE_RELEASE_TMOUT_DELAY_RELVAC);
	m_pTransTable[VOICE_RELEASE_ERR_RSP] = new Trans_Release_ErrNotifyRsp(VOICE_RELEASE_STATE, VOICE_RELEASE_ERR_RSP);
	m_pTransTable[VOICE_RELEASE_TMOUT_ERR_RSP] = new Trans_Release_TerrrspTimeout(VOICE_IDLE_STATE, VOICE_RELEASE_TMOUT_ERR_RSP);
	m_pTransTable[VOICE_RELEASE_REL_RES_REQ] = new Trans_Release_RlsTransResReq(VOICE_IDLE_STATE, VOICE_RELEASE_REL_RES_REQ);
	m_pTransTable[VOICE_RELEASE_TMOUT_REL_RES] = new Trans_Release_TrelresTimeout(VOICE_IDLE_STATE, VOICE_RELEASE_TMOUT_REL_RES);

	m_pTransTable[VOICE_PAGING_VAC_SETUP_RSP] = new Trans_Paging_VACSetupRsp(VOICE_T_ESTABLISH_STATE, VOICE_PAGING_VAC_SETUP_RSP);
	m_pTransTable[VOICE_PAGING_TMOUT_VACSETUP_RSP] = new Trans_Paging_TvacTimeout(VOICE_IDLE_STATE, VOICE_PAGING_TMOUT_VACSETUP_RSP);
	m_pTransTable[VOICE_PAGING_VAC_REL] = new Trans_Paging_VACRelNotify(VOICE_IDLE_STATE, VOICE_PAGING_VAC_REL);
	m_pTransTable[VOICE_PAGING_DELAPAGING] = new Trans_Paging_DeLaPaging(VOICE_IDLE_STATE, VOICE_PAGING_DELAPAGING);

	m_pTransTable[VOICE_PROBE_PROBERSP] = new Trans_Probe_ProbeRsp(VOICE_IDLE_STATE, VOICE_PROBE_PROBERSP);
	m_pTransTable[VOICE_PROBE_TMOUT_PROBERSP] = new Trans_Probe_TproberspTimeout(VOICE_IDLE_STATE, VOICE_PROBE_TMOUT_PROBERSP);

	m_pTransTable[VOICE_WAITSYNC_TMOUT_WAITSYNC] = new Trans_WaitSYNC_TwaisyncTimeout(VOICE_RELEASE_STATE, VOICE_WAITSYNC_TMOUT_WAITSYNC);
	m_pTransTable[VOICE_WAITSYNC_VAC_REL] = new Trans_WaitSYNC_VACRelNotify(VOICE_RELEASE_STATE, VOICE_WAITSYNC_VAC_REL);
	m_pTransTable[VOICE_WAITSYNC_VACSETUP] = new Trans_WaitSYNC_VACSetupCmd(VOICE_TRANSPARENT_STATE, VOICE_WAITSYNC_VACSETUP);
	m_pTransTable[VOICE_WAITSYNC_REL_RES_REQ] = new Trans_WaitSYNC_RelTransResReq(VOICE_IDLE_STATE, VOICE_WAITSYNC_REL_RES_REQ);

	
}

void VoiceFSM::reloadVoiceCfg()
{
	updateVoiceCfgs();
	//tosֵ��������vdr��������Ч
	//jitter buffer �������begin--------------
	if(g_vSrvCfg.blForceUseJBuf)
	{
		forceUseJBuf();
	}
	else
	{
		if(g_vSrvCfg.blCpeZUseJBuf)
		{
			disableJitterBuf();
			enableJitterBuf();
		}
		else
		{
			disableJitterBuf();
		}
	}
	setJBufMaxFrames(g_vSrvCfg.JBufSize);
	setJBufStartFrameNum(g_vSrvCfg.nFramesToStartTx);
	//jitter buffer �������end------------------
	
	//�õ�BTSID��SAGID
	m_BTSID = (bspGetBtsID() & 0x0fff);				//BTSIDȡ��16λ
	m_SAGID = g_vSagBtsLinkCfg1.SAGID;				//������master SAG
	CMsg_Signal_VCR::m_BTSID = (m_BTSID);		//BTSIDȡ��16λ
	CMsg_Signal_VCR::m_SAGID = (m_SAGID);	
}

bool VoiceFSM::init()
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::init");
	//��ʼ������ͳ�Ƽ�����
	CMsg_Signal_VCR::ClearSignalCounters();
	memset(&Counters, 0, sizeof(Counters));
	clearDcsCounters();

	reloadVoiceCfg();
    
	initAllVDataBuf();

	CTimer *pTimerBeatHeart;
	pTimerBeatHeart = NULL;
	
	//����BeatHeart��ʱ��
	CMsg_VoiceTimeout TbeatheartMsg;
	if ( !TbeatheartMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TbeatheartMsg.SetTimerType(TIMER_BEATHEART);
		TbeatheartMsg.SetMessageId(MSGID_TIMER_BEATHEART);
		TbeatheartMsg.SetDstTid(M_TID_VOICE);

		pTimerBeatHeart = new CTimer(1, M_TIMERLEN_BEATHEART, TbeatheartMsg);
		if(NULL==pTimerBeatHeart)
		{
			LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "failed to start Beatheart Timer!!! ");
		}
		else
			pTimerBeatHeart->Start();
	}    
#if 0	
	//����CongestReport��ʱ��
	CMsg_VoiceTimeout TcongestMsg;
	if( !TcongestMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		TcongestMsg.SetTimerType(TIMER_CONGEST);
		TcongestMsg.SetMessageId(MSGID_TIMER_CONGEST);
		TcongestMsg.SetDstTid(M_TID_VOICE);
		
		pTimerCongest = new CTimer(1, M_TIMERLEN_CONGEST, TcongestMsg);
		if(NULL==pTimerCongest)
		{
			LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "failed to start Congest Report Timer!!! ");
		}
		else
			pTimerCongest->Start();	
	}
#endif
	return true;
}

#ifdef G729TONE_TEST

UINT8 g_SN = 0;
unsigned char *pTone = pG729TestTone;

UINT32 voiceTestEID = 0xffffffff;
UINT16 voiceTestGID = 0xffff;
extern "C" void showVoiceTestEid()
{
    VPRINT("\nVoiceTestEid[%08x]\n", voiceTestEID);
}
extern "C" void setVoiceTestEid(UINT32 Eid)
{
    voiceTestEID = Eid;
}
extern "C" void showVoiceTestGid()
{
    VPRINT("\nVoiceTestGid[%08x]\n", voiceTestGID);
}
extern "C" void setVoiceTestGid(UINT16 Gid)
{
    voiceTestGID = Gid;
}

#define MSGID_10ms (0x8888)
void send10msTone2L2()
{
	if(0xffffffff==voiceTestEID && 0xffff==voiceTestGID)
		return;

	if(pTone>pG729TestTone+nLenG729TestTone)
		pTone = pG729TestTone;

	UINT8 nG72910msVACDataSize = sizeof(VACVoiceDataHeaderT)	+ M_G729_10MS_DATALEN;
	CComMessage* pComMsg = new (CTVoice::GetInstance(), 1+nG72910msVACDataSize*2) CComMessage;
	if ( pComMsg==NULL )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
    
	UINT8 *VACBuf = (UINT8*)pComMsg->GetDataPtr();
	//ָ��VAC������Ϣ�е�һ����������
	VACVoiceDataIE_T* pVACDataIE = (VACVoiceDataIE_T*)(&VACBuf[1]); 

	VACBuf[0] = 0;
	if(voiceTestEID!=0xffffffff)
	{
		VSetU32BitVal(pVACDataIE->header.Eid , voiceTestEID);
		pVACDataIE->header.Cid = 0;
		pVACDataIE->header.SN = g_SN;
		pVACDataIE->header.Type = CODEC_G729A;
		memcpy( (void*)pVACDataIE->VoiceData, (void*)pTone, M_G729_10MS_DATALEN);
		VACBuf[0]++;
		pVACDataIE = (VACVoiceDataIE_T*)(&VACBuf[1]+nG72910msVACDataSize);
	}
	if(voiceTestGID!=0xffff)
	{
		VSetU32BitVal(pVACDataIE->header.Eid , voiceTestGID);
		pVACDataIE->header.Cid = CODEC_GRPTONE;
		pVACDataIE->header.SN = g_SN;
		pVACDataIE->header.Type = CODEC_G729A;
		memcpy( (void*)pVACDataIE->VoiceData, (void*)pTone, M_G729_10MS_DATALEN);
		VACBuf[0]++;		
	}
	
	pComMsg->SetDstTid(M_TID_VAC);
	pComMsg->SetMessageId(MSGID_VOICE_VAC_DATA);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDataLength(1+VACBuf[0]*nG72910msVACDataSize);
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
	}

	g_SN++;
	pTone+=M_G729_10MS_DATALEN;
}
#endif	//#ifdef G729TONE_TEST

BeartHeartCtrl::BeartHeartCtrl()
{
	init();
}
void BeartHeartCtrl::init()
{
	m_MasterSAG.m_LinkTID = M_TID_VCR;
	m_MasterSAG.m_BeartHeartSeq = 0;
	m_MasterSAG.m_BeartHeartUnReply = 0;
	m_MasterSAG.m_blShouldSendBeartHeart = false;

	m_BackupSAG.m_LinkTID = M_TID_VCR1;
	m_BackupSAG.m_BeartHeartSeq = 0;
	m_BackupSAG.m_BeartHeartUnReply = 0;
	m_BackupSAG.m_blShouldSendBeartHeart = false;	
}
bool BeartHeartCtrl::isFromMasterSag(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	return g_vSagBtsLinkCfg1.SAGID == VGetU32BitVal(pData->sigHeader.SAG_ID);
}
bool BeartHeartCtrl::isFromBackupSag(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	return g_vSagBtsLinkCfg2.SAGID == VGetU32BitVal(pData->sigHeader.SAG_ID);
}
void BeartHeartCtrl::whenRcvOtherSignalMsg(CMessage& msg)
{
	//�յ�����������Ϣ��Ϊ��·�ǻ�ģ�����Ҫ��������
	if(isFromMasterSag(msg))
	{
		m_MasterSAG.m_blShouldSendBeartHeart = false;
		m_MasterSAG.m_BeartHeartUnReply = 0;
	}
	else if(isFromBackupSag(msg))
	{
		m_BackupSAG.m_blShouldSendBeartHeart = false;
		m_BackupSAG.m_BeartHeartUnReply = 0;
	}
	else
	{
		//do nothing
	}
}
void BeartHeartCtrl::handleBeatHeartAckMsg(CMessage& msg)
{
    VoiceVCRCtrlMsgT *pDataBeatHeartAck;
    
    pDataBeatHeartAck = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	if(isFromMasterSag(msg))
	{
		m_MasterSAG.m_BeartHeartUnReply = 0;
        //new stat
        if(pDataBeatHeartAck->sigPayload.BeatHeartAck.sequence==m_MSAGBeatHeartNo)
        {
            if((m_MSAG10msTickSNum!=0)&&(m_MSAG10msTickSNum<g_FrameNum))//Ϊ0��ʾ����һ��ͳ�����ڷ���������������ͳ�������յ�Ӧ�𣬶���
            {
                m_MSAGBeatHeartDelayTotal += g_FrameNum - m_MSAG10msTickSNum;
                m_MSAG10msTickSNum = 0;
            }
            else
                m_MSAG10msTickSNum = 0;
        }
        LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"handleBeatHeartAckMsg(m_MasterSAG)!");
	}
	else if(isFromBackupSag(msg))
	{
		m_BackupSAG.m_BeartHeartUnReply = 0;
        //new stat
        if(pDataBeatHeartAck->sigPayload.BeatHeartAck.sequence==m_SSAGBeatHeartNo)
        {
            if((m_SSAG10msTickSNum!=0)&&(m_SSAG10msTickSNum<g_FrameNum))
            {
                m_SSAGBeatHeartDelayTotal += g_FrameNum - m_SSAG10msTickSNum;
                m_SSAG10msTickSNum = 0;
            }
            else
                m_SSAG10msTickSNum = 0;
        }
        LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"handleBeatHeartAckMsg(m_BackupSAG)!");
	}
	else
	{
		//do nothing
	}
}
void BeartHeartCtrl::handleBeatHeartTmoutMsg(CMessage& msg)
{    
    //new stat
    if(m_MSAG10msTickSNum!=0)
    {
        LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"Master SAG BeatHeartTimeout!");
        m_MSAGBeatHeartTimeoutNum++;
        m_MSAGBeatHeartDelayTotal += M_TIMERLEN_BEATHEART/10;//10ms
    }
    if(m_SSAG10msTickSNum!=0)
    {
        LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"Slave SAG BeatHeartTimeout!");
        m_SSAGBeatHeartTimeoutNum++;
        m_SSAGBeatHeartDelayTotal += M_TIMERLEN_BEATHEART/10;//10ms
    }
	//master sag
	if(m_MasterSAG.m_blShouldSendBeartHeart)
	{
		sendBeartHeart(m_MasterSAG);
        //new stat
        m_MSAG10msTickSNum = g_FrameNum;
        m_MSAGBeatHeartNum++;
        LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"sendBeartHeart(m_MasterSAG)!");
	}
	else
	{
		m_MasterSAG.m_blShouldSendBeartHeart = true;
	}
	if(CVCR::GetInstance()->IsSAGConnected())
	{
		m_MasterSAG.m_BeartHeartUnReply++;
	}
	else
	{
		m_MasterSAG.m_BeartHeartUnReply=0;
	}
	if(m_MasterSAG.m_BeartHeartUnReply>3)
	{
		if(CVCR::GetInstance()->IsSAGConnected())
		{
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"Reset master SAG-BTS link, for more than 3 BeatHeart Requests not answered!!!");
			resetBtsSagLink(m_MasterSAG);
		}
	}

	//backup sag
	if(m_BackupSAG.m_blShouldSendBeartHeart)
	{
		sendBeartHeart(m_BackupSAG);
        //new stat
        m_SSAG10msTickSNum = g_FrameNum;
        m_SSAGBeatHeartNum++;
        LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"sendBeartHeart(m_BackupSAG)!");
	}
	else
	{
		m_BackupSAG.m_blShouldSendBeartHeart = true;
	}
	if(CVCR::GetBakInstance()->IsSAGConnected())
	{
		m_BackupSAG.m_BeartHeartUnReply++;
	}
	else
	{
		m_BackupSAG.m_BeartHeartUnReply=0;
	}
	if(m_BackupSAG.m_BeartHeartUnReply>3)
	{
		if(CVCR::GetBakInstance()->IsSAGConnected())
		{
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"Reset backup SAG-BTS link, for more than 3 BeatHeart Requests not answered!!!");
			resetBtsSagLink(m_BackupSAG);
		}
	}
}
bool BeartHeartCtrl::sendBeartHeart(BeartHeartOptionsT& opt)
{
	VoiceVCRCtrlMsgT* pData;
	CMsg_Signal_VCR BeatHeart;
	if ( !BeatHeart.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(BeatHeartT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		BeatHeart.SetDstTid(opt.m_LinkTID);
		BeatHeart.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		BeatHeart.SetBTSSAGID();
		BeatHeart.SetSigIDS(BeatHeart_MSG);	
    	
		pData = (VoiceVCRCtrlMsgT*)BeatHeart.GetDataPtr();
		pData->sigPayload.BeatHeart.sequence = opt.m_BeartHeartSeq++;
		if(opt.m_LinkTID==M_TID_VCR1)	//����Ǳ���sag����Ҫ�޸�sagid
		{
			VSetU32BitVal(pData->sigHeader.SAG_ID , g_vSagBtsLinkCfg2.SAGID);
            //new stat
            m_SSAGBeatHeartNo = pData->sigPayload.BeatHeart.sequence;
		}
        else
        {
            //new stat
            m_MSAGBeatHeartNo = pData->sigPayload.BeatHeart.sequence;
        }
		BeatHeart.SetSigHeaderLengthField(sizeof(BeatHeartT));
		BeatHeart.SetPayloadLength(sizeof(SigHeaderT)+sizeof(BeatHeartT));
		LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"send beatheart message to %s SAG",
			opt.m_LinkTID==M_TID_VCR ? (int)"Master" : (int)"Backup" );
		return BeatHeart.Post();
	}
}
bool BeartHeartCtrl::sendBeartHeartAck(CMessage& msg)
{
	CMsg_Signal_VCR BeatHeartAck;
	VoiceVCRCtrlMsgT *pDataBeatHeart, *pDataBeatHeartAck;
	
	if ( !BeatHeartAck.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(BeatHeartAckT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		BeatHeartAck.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		BeatHeartAck.SetDstTid(msg.GetSrcTid());
		BeatHeartAck.SetBTSSAGID();
		BeatHeartAck.SetSigIDS(BeatHeartAck_MSG);
		pDataBeatHeartAck = (VoiceVCRCtrlMsgT*)BeatHeartAck.GetDataPtr();
		if(isFromBackupSag(msg))	//if from backup SAG
		{
			VSetU32BitVal(pDataBeatHeartAck->sigHeader.SAG_ID , g_vSagBtsLinkCfg2.SAGID);
		}
		pDataBeatHeart = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();

		pDataBeatHeartAck->sigPayload.BeatHeartAck.sequence = pDataBeatHeart->sigPayload.BeatHeart.sequence;

		BeatHeartAck.SetSigHeaderLengthField(sizeof(BeatHeartAckT));
		BeatHeartAck.SetPayloadLength(sizeof(SigHeaderT)+sizeof(BeatHeartAckT));
		LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"send beatheartAck message to %s SAG",
			msg.GetSrcTid()==M_TID_VCR ? (int)"Master" : (int)"Backup" );
		
		return BeatHeartAck.Post();
	}	
}
bool BeartHeartCtrl::resetBtsSagLink(BeartHeartOptionsT& opt)
{
	return sendNopayloadMsg(CTVoice::GetInstance(), 
		opt.m_LinkTID, M_TID_VOICE, MSGID_VOICE_VCR_RESETLINK);
}

void VoiceFSM::Voice10msProc()
{
	bool blSpyUidVData = g_SpyVData.isSpyUidVoiceData();
	
	bool blFreeVACVoiceData = false;
	CMsg_VACVoiceData VACVoiceData;
	UINT8	*VACBuf ;
	int VACMsgLen;
	UINT8	*cur_VACData_ptr;
	int i;
	if(g_blUseLocalSag)
	{
		sagPlayTone();
	}
	if(g_blUseDownLinkJitterBuf)
	{
		//VPRINT("\nsend10msVoiceData2L2\n");
		if ( !VACVoiceData.CreateMessage(*CTVoice::GetInstance()) )
		{
		        LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		        return;
		}
		blFreeVACVoiceData = true;
		VACVoiceData.SetDstTid(M_TID_VAC);
		VACVoiceData.SetMessageId(MSGID_VOICE_VAC_DATA);
		VACBuf = (UINT8*)VACVoiceData.GetDataPtr();
		VACMsgLen = 1;
		VACBuf[0] = 0;
		//ָ��VAC������Ϣ�е�ǰҪ������������
		cur_VACData_ptr = &VACBuf[VACMsgLen];
	}
#if 0 //20110613 del by fb 001 begin
	//������Ϣ
	DMUXHeadT* pDMUXHead;
	DMUXVoiDataCommonT *pVoiDataCommon;
	CComMessage *pComMsg = NULL;
	UINT8	*pUdp;						//UDP��������ָ��
	UINT16	UdpPktlen;					//UDP������
	int nVDataLen, nVFrameLen;
	UINT16 n10msPktNum;
	//�γ�UDP������
	pComMsg = new (CTVoice::GetInstance(), M_MAX_SRTP_PKT_LEN+1) CComMessage;
	if(pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "Cannot new CComMessage!!!!!!");
		if(blFreeVACVoiceData)
		{
			VACVoiceData.DeleteMessage();
		}
		return;
	}
	else
	{
		n10msPktNum = 0;
		
		pUdp = (UINT8*)pComMsg->GetDataPtr();
		pDMUXHead = (DMUXHeadT*)pUdp;
		pDMUXHead->FormID = 0;
		pDMUXHead->Prio = M_DMUX_VOICEDATA_PRIORITY;
		pDMUXHead->nFrameNum = 0;
		pDMUXHead->Reserved1 = 0;
		//pDMUXHead->srtpTimeStamp = htonl(g_VFrameNum);
		VSetU32BitVal(pDMUXHead->Reserved2 , 0);
		//UDP packet length
		UdpPktlen = sizeof(DMUXHeadT);
		pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen);	//point to the first frame
	}
#endif //20110613 del by fb 001 end
	for(i=0;i<M_MAX_CALLS;i++)
	{
		if(vDatabuf[i].faxDataBuf.blUlTxDataStarted || vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf>0)
		{
			TxUlFaxData(i);
			//���治ʹ��jitterbuffer�����в���Ҫά��sn����Ϊ������buffer����������
			continue;
		}
		
		if(g_blUseDownLinkJitterBuf)
		{
			//process downlink
			if(vDatabuf[i].downLinkBuf.blStarted)
			{
				if(vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].len!=M_INVALID_10MS_DATALEN)
				{
					memcpy((void*)cur_VACData_ptr,
							(void*)&vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].VACItemHead,
							sizeof(VACVoiceDataHeaderT) + vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].len);
					
					++VACBuf[0];
					UINT16 nLenAdd = ( sizeof(VACVoiceDataHeaderT) + vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].len );
					VACMsgLen += nLenAdd;
					cur_VACData_ptr += nLenAdd;
					vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].len = M_INVALID_10MS_DATALEN;

					//fengbing 20090603 for spyVData with jitterbuffer-----begin
					if(blSpyUidVData)
					{			
						if(vDatabuf[i].uid == g_SpyVData.getDiagUID())
						{
							if(CODEC_G729B_SID==vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].VACItemHead.Type)
							{
								++g_SpyVData.uCntUID10ms729BToVAC;
								if(g_SpyVData.isSpyDLUidVDataTS())
								{
									VPRINT("\n[%08X] send G.729B TS[%d]", 
										g_SpyVData.	getFrameID(),
										vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].VACItemHead.SN);
								}
							}
							else
							{
								++g_SpyVData.uCntUID10msVDataToVAC;
								if(g_SpyVData.isSpyDLUidVDataTS())
								{
									VPRINT("\n[%08X] send G.729A TS[%d]", 
										g_SpyVData.	getFrameID(),
										vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].VACItemHead.SN);
								}
							}
						}
					}				
					//fengbing 20090603 for spyVData with jitterbuffer -----end
					
				}
				else
				{
					if(blSpyUidVData)
					{			
						if(vDatabuf[i].uid == g_SpyVData.getDiagUID())
						{
							if(g_SpyVData.isSpyDLUidVDataTS())
							{
							    VPRINT("\nNo voice Data in JitterBuffer!!!(uid:0x%08x cid:0x%02x)\n", 
							        vDatabuf[i].uid ,
							        vDatabuf[i].downLinkBuf.jitterBuffer[vDatabuf[i].downLinkBuf.curSndIdx].VACItemHead.Cid );
							}
						}
					}
				}
				if(vDatabuf[i].downLinkBuf.nRcvFrames>0)
				{
					vDatabuf[i].downLinkBuf.nRcvFrames--;
				}
				vDatabuf[i].downLinkBuf.curSndIdx = Mod2Power( vDatabuf[i].downLinkBuf.curSndIdx+1 , JITTER_FRAMES);
				vDatabuf[i].downLinkBuf.curSndSN++;
			}
		}
#if 0 //20110613 del by fb 002 begin
		//process uplink
		if(vDatabuf[i].upLinkBuf.blStarted)
		{
			vDatabuf[i].upLinkBuf.timeStampLen++;
			
			if(vDatabuf[i].upLinkBuf.srtpItmeHead.Codec==CODEC_G729A)
			{
				nVDataLen = 2;
				nVFrameLen = 2+sizeof(DMUXVoiDataCommonT);
			}
			else
			{
				nVDataLen = M_G711_10MS_DATALEN;
				nVFrameLen = M_G711_10MS_DATALEN+sizeof(DMUXVoiDataCommonT);
			}

			//���srtp��������㹻
			if((UdpPktlen+nVFrameLen)<M_MAX_SRTP_PKT_LEN)
			{
				if(vDatabuf[i].upLinkBuf.timeStampLen>=100)
				{
					//srtp head
					memcpy(pVoiDataCommon->CallID , vDatabuf[i].upLinkBuf.srtpItmeHead.CallID, 2);
					pVoiDataCommon->Codec = vDatabuf[i].upLinkBuf.srtpItmeHead.Codec;
					pVoiDataCommon->blG729B = (pVoiDataCommon->Codec==CODEC_G729A);
					pVoiDataCommon->frameNum = (pVoiDataCommon->blG729B) ? 0 : 1;
					pVoiDataCommon->SN = vDatabuf[i].upLinkBuf.srtpItmeHead.SN++;
					//˵������100֡û����������
					//20090909 fengbing --- begin
					#if 0 
					pVoiDataCommon->timeStamp = pVoiDataCommon->timeStamp
						+vDatabuf[i].upLinkBuf.timeStampLen;
					#endif 
					pVoiDataCommon->timeStamp = vDatabuf[i].upLinkBuf.srtpItmeHead.timeStamp
						+vDatabuf[i].upLinkBuf.timeStampLen;
					//���µ�ǰ��timeStamp
					vDatabuf[i].upLinkBuf.srtpItmeHead.timeStamp = pVoiDataCommon->timeStamp;
					//20090909 fengbing --- end
					//srtp voice data
					memset((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
						0,nVDataLen);
					
					UdpPktlen += nVFrameLen;
					pDMUXHead->nFrameNum++;
					++n10msPktNum;

					++Counters.nNullPkt;
					//����NULL�������¿�ʼ����������ʱ
					vDatabuf[i].upLinkBuf.timeStampLen = 0;
				}
			}
			
		}
		//������Ϣ���ͷ���Ϣ
#endif //20110613 del by fb 002 end
		
	}	

	if(g_blUseDownLinkJitterBuf)
	{
		if(blFreeVACVoiceData)
		{
			if(VACBuf[0]>0)		//�����������
			{
				VACVoiceData.SetPayloadLen(VACMsgLen);	//��Ϣ����
				if(VACVoiceData.Post())
				{
					Counters.nVoiDataToVAC++;
					Counters.n10msPktToVAC += VACBuf[0];
				}
			}
			else				//���������Ϣ
			{
				VACVoiceData.DeleteMessage();
			}
		}
	}
#if 0 //20110613 del by fb 003 begin	
	//��д��ǰ���ݰ�����
	pComMsg->SetDataLength(UdpPktlen);
	if(pDMUXHead->nFrameNum>0)
	{
		if(SendVoiceDataToSAG(pComMsg))
		{
			Counters.nVoiDataToVDR++;
			Counters.n10msPktToVDR += n10msPktNum;
		}
	}
	else
	{
		pComMsg->Destroy();
		return;
	}	
#endif //20110613 del by fb 003 end
}
bool VoiceFSM::sendFaxDataToSag(UINT16 i)
{
	bool ret = false;
	CComMessage *pMsg;

	bool blSpyUidVData = (g_SpyVData.isSpyUidVoiceData() && vDatabuf[i].uid==g_SpyVData.getDiagUID()); 
	bool blLogULUidFaxData = (blSpyUidVData && g_SpyVData.isSpyULUidFaxData());
	
	UINT8 pos1 = vDatabuf[i].faxDataBuf.curUlTxSN & (M_MAX_BUFFER_FAXDATA_FRAMES-1);
	UINT8 pos2 = (vDatabuf[i].faxDataBuf.curUlTxSN+1) & (M_MAX_BUFFER_FAXDATA_FRAMES-1);
	if(vDatabuf[i].faxDataBuf.faxDataUlBuf[pos1].Codec!=CODEC_G729A ||
		vDatabuf[i].faxDataBuf.faxDataUlBuf[pos2].Codec!=CODEC_G729A)
	{
		UINT16 nMsgLen = sizeof(DMUXHeadT)+sizeof(DMUXVoiDataCommonT)+2*M_G711_10MS_DATALEN;
		pMsg = new (CTVoice::GetInstance(), nMsgLen) CComMessage;
		if(pMsg==NULL)
		{
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"new commessage failed!!!");
		}
		else
		{
			UINT8* pData = (UINT8*)pMsg->GetDataPtr();
			DMUXHeadT* pDMUXHead = (DMUXHeadT*)pData;
			DMUXVoiDataCommonT* pDMUXVoiDataCommon = (DMUXVoiDataCommonT*)(pData+sizeof(DMUXHeadT));
			UINT8* pFaxPayload = (UINT8*)pData+sizeof(DMUXHeadT)+sizeof(DMUXVoiDataCommonT);

			pDMUXHead->FormID = 0;
			pDMUXHead->Prio = M_DMUX_VOICEDATA_PRIORITY;
			pDMUXHead->nFrameNum = 1;
			pDMUXHead->Reserved1 = 0;
			VSetU32BitVal(pDMUXHead->Reserved2 , 0);

			memcpy(pDMUXVoiDataCommon->CallID, 
				vDatabuf[i].upLinkBuf.srtpItmeHead.CallID,
				sizeof(pDMUXVoiDataCommon->CallID));
			pDMUXVoiDataCommon->Codec = CODEC_G711A;
			pDMUXVoiDataCommon->blG729B = false;
			pDMUXVoiDataCommon->SN = vDatabuf[i].upLinkBuf.srtpItmeHead.SN++;
			
			pDMUXVoiDataCommon->frameNum=2;
			pDMUXVoiDataCommon->timeStamp=vDatabuf[i].faxDataBuf.curUlTxSN;
			//��һ֡
			if(vDatabuf[i].faxDataBuf.faxDataUlBuf[pos1].Codec==CODEC_G711A)
			{
				memcpy(pFaxPayload, 
					vDatabuf[i].faxDataBuf.faxDataUlBuf[pos1].Payload, 
					M_G711_10MS_DATALEN);
				if(blSpyUidVData)
				{
					g_SpyVData.uCntUID10ms711AToSAG++;
					if(blLogULUidFaxData)
					{
						VPRINT("\n ULTX--FN[0x%08X] curUlTxSN[%d] nUlDataFrmsInBuf[%d] blUlTxDataStarted[%d] ",
							g_SpyVData.getFrameID(), 
							vDatabuf[i].faxDataBuf.curUlTxSN,
							vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf,
							vDatabuf[i].faxDataBuf.blUlTxDataStarted);
					}
				}
				vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf--;
			}
			else
			{
				memcpy(pFaxPayload, 
					vDatabuf[i].faxDataBuf.faxDataUlBuf[pos2].Payload, 
					M_G711_10MS_DATALEN);
				if(blSpyUidVData)
				{
					g_SpyVData.uCntUID10ms711AFromL2Lost++;
					if(blLogULUidFaxData)
					{
						VPRINT("\n ULLost--FN[0x%08X] curUlTxSNLost[%d] nUlDataFrmsInBuf[%d] blUlTxDataStarted[%d] ???????????????",
							g_SpyVData.getFrameID(), 
							vDatabuf[i].faxDataBuf.curUlTxSN,
							vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf,
							vDatabuf[i].faxDataBuf.blUlTxDataStarted);
					}
				}				
			}
			//�ڶ�֡
			if(vDatabuf[i].faxDataBuf.faxDataUlBuf[pos2].Codec==CODEC_G711A)
			{
				memcpy(pFaxPayload+M_G711_10MS_DATALEN, 
					vDatabuf[i].faxDataBuf.faxDataUlBuf[pos2].Payload, 
					M_G711_10MS_DATALEN);
				if(blSpyUidVData)
				{
					g_SpyVData.uCntUID10ms711AToSAG++;
					if(blLogULUidFaxData)
					{
						VPRINT("\n ULTX--FN[0x%08X] curUlTxSN[%d] nUlDataFrmsInBuf[%d] blUlTxDataStarted[%d] ",
							g_SpyVData.getFrameID(), 
							0xff & (vDatabuf[i].faxDataBuf.curUlTxSN+1),
							vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf,
							vDatabuf[i].faxDataBuf.blUlTxDataStarted);
					}
				}
				vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf--;
			}
			else
			{
				memcpy(pFaxPayload+M_G711_10MS_DATALEN, 
					vDatabuf[i].faxDataBuf.faxDataUlBuf[pos1].Payload, 
					M_G711_10MS_DATALEN);
				if(blSpyUidVData)
				{
					g_SpyVData.uCntUID10ms711AFromL2Lost++;
					if(blLogULUidFaxData)
					{
						VPRINT("\n ULLost--FN[0x%08X] curUlTxSNLost[%d] nUlDataFrmsInBuf[%d] blUlTxDataStarted[%d] ???????????????",
							g_SpyVData.getFrameID(), 
							0xff & (vDatabuf[i].faxDataBuf.curUlTxSN+1),
							vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf,
							vDatabuf[i].faxDataBuf.blUlTxDataStarted);
					}
				}
			}
			vDatabuf[i].faxDataBuf.faxDataUlBuf[pos1].Codec = CODEC_G729A;
			vDatabuf[i].faxDataBuf.faxDataUlBuf[pos2].Codec = CODEC_G729A;

			pMsg->SetDataLength(nMsgLen);
			ret = SendVoiceDataToSAG(pMsg);
		}
	}
	else
	{
		//����2֡����ʧ�ݶ��޷�����
		if(blSpyUidVData)
		{
			g_SpyVData.uCntUID10ms711AFromL2Lost+=2;
			if(blLogULUidFaxData)
			{
				VPRINT("\n ULLost--FN[0x%08X] curUlTxSNLost[%d,%d] nUlDataFrmsInBuf[%d] blUlTxDataStarted[%d] ???????????????",
					g_SpyVData.getFrameID(), 
					vDatabuf[i].faxDataBuf.curUlTxSN,
					0xff & (vDatabuf[i].faxDataBuf.curUlTxSN+1),
					vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf,
					vDatabuf[i].faxDataBuf.blUlTxDataStarted);
			}
		}
	}
	return ret;
}
void VoiceFSM::sendFaxDataToL2(UINT16 i)
{
	//���͸�btsL2
	UINT8 nSndFrm;
	CComMessage *pMsg;
	bool blSpyUidVData = (g_SpyVData.isSpyUidVoiceData() && vDatabuf[i].uid==g_SpyVData.getDiagUID()); 
	bool blLogULUidFaxData = (blSpyUidVData && g_SpyVData.isSpyULUidFaxData());
	bool blLogDLUidFaxData = (blSpyUidVData && g_SpyVData.isSpyDLUidFaxData());
	
	for(nSndFrm=0;nSndFrm<2;nSndFrm++)
	{
		UINT8 pos = (vDatabuf[i].faxDataBuf.curUlTxSN+nSndFrm) & (M_MAX_BUFFER_FAXDATA_FRAMES-1);
		if(CODEC_G711A==vDatabuf[i].faxDataBuf.faxDataUlBuf[pos].Codec)
		{
			pMsg = new (CTVoice::GetInstance(), sizeof(FaxData10msFrameT)) CComMessage;
			if(pMsg!=NULL)
			{
				FaxData10msFrameT *pData = (FaxData10msFrameT*)pMsg->GetDataPtr();
				memcpy(pData, 
					&vDatabuf[i].faxDataBuf.faxDataUlBuf[pos], 
					sizeof(FaxData10msFrameT));
				pData->Cid = vDatabuf[i].peerCid;

				pMsg->SetDataLength(sizeof(FaxData10msFrameT));
				pMsg->SetEID(vDatabuf[i].peerEid);
				pMsg->SetMessageId(MSGID_FAX_DATA_DL);
				pMsg->SetSrcTid(M_TID_VOICE);
				pMsg->SetDstTid(M_TID_VAC);
				if(postComMsg(pMsg))
				{
					if(blSpyUidVData)
					{
						g_SpyVData.uCntUID10ms711AToL2++;
						if(blLogULUidFaxData || blLogDLUidFaxData)
						{
							VPRINT("\n UDTX--FN[0x%08X] curUlTxSN[%d] nUlDataFrmsInBuf[%d] blUlTxDataStarted[%d] ",
								g_SpyVData.getFrameID(), 
								0xff & (vDatabuf[i].faxDataBuf.curUlTxSN+nSndFrm),
								vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf,
								vDatabuf[i].faxDataBuf.blUlTxDataStarted);
						}
					}
				}
				else
				{
				}
			}
			else
			{
				LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"new commessage failed!!!");
			}

			vDatabuf[i].faxDataBuf.faxDataUlBuf[pos].Codec = CODEC_G729A;
			vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf--;
		}
		else
		{
			g_SpyVData.uCntUID10ms711AFromL2Lost++;

			if(blSpyUidVData)
			{
				if(blLogULUidFaxData || blLogDLUidFaxData)
				{
					VPRINT("\n ULLost--FN[0x%08X] curUlTxSNLost[%d] nUlDataFrmsInBuf[%d] blUlTxDataStarted[%d] ???????????????",
						g_SpyVData.getFrameID(), 
						0xff & (vDatabuf[i].faxDataBuf.curUlTxSN+nSndFrm),
						vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf,
						vDatabuf[i].faxDataBuf.blUlTxDataStarted);
				}
			}
		}
	}
}
//������������д������ݷ��͸�btsL2,���򷢸�sag
void VoiceFSM::TxUlFaxData(UINT16 i)
{
	if(vDatabuf[i].faxDataBuf.blUlTxDataStarted)
	{
		//ÿ2֡��һ�Σ�����2��10ms֡fax data
		bool blSend = ((vDatabuf[i].uid ^ g_SpyVData.getFrameID()) & 1);
		if(blSend)
		{
			if(NO_EID==vDatabuf[i].peerEid)
			{
				//���͸�sag
				sendFaxDataToSag(i);
			}
			else
			{
				//���͸�btsL2
				sendFaxDataToL2(i);
			}
		}
		//�ƶ�����λ��
		vDatabuf[i].faxDataBuf.curUlTxSN += 1;
	}
	else
	{
		if(vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf!=0)
		{
#if 0			
			//�յ���һ���󾭹�g_nBufferFaxDataFrames֡��ʼ����
			UINT32 curFN = g_SpyVData.getFrameID();
			UINT32 firstFN = vDatabuf[i].faxDataBuf.firstDataArriveFN;
			UINT32 nFnLen = (curFN>firstFN) ? (curFN-firstFN) : (0xffffffff-firstFN+curFN);
			if(g_nBufferFaxDataFrames<nFnLen)
#else				
			//�չ�g_nBufferFaxDataFrames֡��ʼ����
			if(g_nBufferFaxDataFrames<vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf)
#endif				
			{
				vDatabuf[i].faxDataBuf.blUlTxDataStarted = true;
				vDatabuf[i].faxDataBuf.curUlTxSN = vDatabuf[i].faxDataBuf.firstDataSN;

				bool blSpyUidVData = (g_SpyVData.isSpyUidVoiceData() && vDatabuf[i].uid==g_SpyVData.getDiagUID()); 
				bool blLogULUidFaxData = (blSpyUidVData && g_SpyVData.isSpyULUidFaxData());
				if(blLogULUidFaxData)
				{
					VPRINT("\n FN[0x%08X] firstDataArriveFN[0x%08X] firstDataSN[%d] curUlTxSN[%d] nUlDataFrmsInBuf[%d]++++++++++set UL faxdata TX start flag",
						g_SpyVData.getFrameID(), 
						vDatabuf[i].faxDataBuf.firstDataArriveFN,
						vDatabuf[i].faxDataBuf.firstDataSN,
						vDatabuf[i].faxDataBuf.curUlTxSN,
						vDatabuf[i].faxDataBuf.nUlDataFrmsInBuf);				
				}
			}
		}
		else
		{
			//do nothing
		}
	}
}

void VoiceFSM::handleUplinkFaxData(CMessage &msg)
{
	FaxData10msFrameT *pDataFaxFrame = (FaxData10msFrameT*)msg.GetDataPtr();
	if(pDataFaxFrame!=NULL)
	{
		if(pDataFaxFrame->Codec==CODEC_G711A)
		{
			if(g_blBufFaxData)
			{
				VoiceTuple tuple;
				tuple.Eid = msg.GetEID();
				tuple.Cid = pDataFaxFrame->Cid;
				VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
				if(pCCB!=NULL)
				{
					UINT16 idx = pCCB->m_vDataIdx;
					if(idx>=M_MAX_CALLS)
					{
						//�����ͷź�faxData��DAC��ʱ����ʱ��������̣���ʱidx�Ѿ�ΪM_INVALID_VDATABUF_IDX
						return;
					}
					bool blSpyUidVData = (g_SpyVData.isSpyUidVoiceData() && pCCB->getUID()==g_SpyVData.getDiagUID()); 
					bool blLogULUidFaxData = (blSpyUidVData && g_SpyVData.isSpyULUidFaxData());

					//log FN&SN of the first fax data may be used when starting TX
					if(0==vDatabuf[idx].faxDataBuf.firstDataArriveFN)					
					{
						if((0==vDatabuf[idx].faxDataBuf.firstDataSN) && 
							!vDatabuf[idx].faxDataBuf.blUlTxDataStarted &&
							0==vDatabuf[idx].faxDataBuf.nUlDataFrmsInBuf)
						{
							vDatabuf[idx].faxDataBuf.firstDataArriveFN = g_SpyVData.getFrameID();
							vDatabuf[idx].faxDataBuf.firstDataSN = pDataFaxFrame->Sn;

							if(blLogULUidFaxData)
							{
								VPRINT("\n FN[0x%08X] firstDataArriveFN[0x%08X] firstDataSN[%d] curUlTxSN[%d]++++++first UL faxdata arrive",
									g_SpyVData.getFrameID(), 
									vDatabuf[idx].faxDataBuf.firstDataArriveFN,
									pDataFaxFrame->Sn,
									vDatabuf[idx].faxDataBuf.curUlTxSN);
							}
						}
					}
					//buffer fax data
					UINT16 savePos = pDataFaxFrame->Sn & (M_MAX_BUFFER_FAXDATA_FRAMES-1);
					memcpy(&vDatabuf[idx].faxDataBuf.faxDataUlBuf[savePos], 
						pDataFaxFrame, 
						sizeof(FaxData10msFrameT));
					vDatabuf[idx].faxDataBuf.nUlDataFrmsInBuf++;

					if(blSpyUidVData)
					{			
						g_SpyVData.uCntUID10ms711AFromL2++;
						if(blLogULUidFaxData)
						{
							VPRINT("\n ULRX--FN[0x%08X] curUlRxSN[%d] curUlTxSN[%d] nUlDataFrmsInBuf[%d] blUlTxDataStarted[%d] ",
								g_SpyVData.getFrameID(), 
								pDataFaxFrame->Sn,
								vDatabuf[idx].faxDataBuf.curUlTxSN,
								vDatabuf[idx].faxDataBuf.nUlDataFrmsInBuf,
								vDatabuf[idx].faxDataBuf.blUlTxDataStarted);
						}
					}
//��ֹRX��TX̫���ӽ����´洢ת��ʱ����begin
					if(vDatabuf[idx].faxDataBuf.blUlTxDataStarted)
					{
						int DeltaRxTxVal = pDataFaxFrame->Sn - vDatabuf[idx].faxDataBuf.curUlTxSN;
						if(-M_FAX_ULDATA_RXTX_MIN_DISTANCE<DeltaRxTxVal && DeltaRxTxVal<M_FAX_ULDATA_RXTX_MIN_DISTANCE)
						{
							vDatabuf[idx].faxDataBuf.nUlDataTxRxTooNear++;
							if(vDatabuf[idx].faxDataBuf.nUlDataTxRxTooNear>M_FAX_ULDATA_RXTX_MAX_TOONEAR_TIMES)
							{
								if(vDatabuf[idx].faxDataBuf.curUlTxSN < M_FAX_ULDATA_TX_MOVEBACK_DISTANCE)
								{
									vDatabuf[idx].faxDataBuf.curUlTxSN += (0xff - M_FAX_ULDATA_TX_MOVEBACK_DISTANCE);
								}
								else
								{
									vDatabuf[idx].faxDataBuf.curUlTxSN -= M_FAX_ULDATA_TX_MOVEBACK_DISTANCE;
								}
								
								if(blLogULUidFaxData)
								{
									VPRINT("\n MoveBackTx--FN[0x%08X] curUlRxSN[%d] curUlTxSN[%d] *********",
										g_SpyVData.getFrameID(), 
										pDataFaxFrame->Sn,
										vDatabuf[idx].faxDataBuf.curUlTxSN);
								}
							}
						}
						else
						{
							vDatabuf[idx].faxDataBuf.nUlDataTxRxTooNear = 0;
						}
					}
//��ֹRX��TX̫���ӽ����´洢ת��ʱ����end

				}
				else
				{
					LOG2(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
						"handleUplinkFaxData(), Cann't find CCB!!!Eid[0x%08X] Cid[0x%02X]",
						tuple.Eid, tuple.Cid);
				}			
			}
			else
			{
	 			UINT8 *pVACData = (pDataFaxFrame->Payload - sizeof(VACVoiceDataHeaderT) - 2);
				VACVoiceDataHeaderT *pVACDataIE = (VACVoiceDataHeaderT*)(pVACData+2);
				VSetU32BitVal(pVACDataIE->Eid, msg.GetEID());
				pVACDataIE->Cid = pDataFaxFrame->Cid;
				pVACDataIE->SN = pDataFaxFrame->Sn;
				pVACDataIE->Type = CODEC_G711A;
				pVACData[0] = 0;
				pVACData[1] = 1;
				msg.SetDataPtr(pVACData);
				msg.SetDataLength(2+sizeof(VACVoiceDataHeaderT)+M_G711_10MS_DATALEN);
				handleVoiceDataFromVAC(msg);
			}

		}
		else
		{
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"handleUplinkFaxData(), Codec is not G.711A!!!");
		}
	}
	else
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"handleUplinkFaxData(), DataPtr NULL!!!");
	}
}

void VoiceFSM::handleFaxDataFromSAG(UINT8 *pData, VoiceCCB *pCCB)
{
	bool blSpyUidVData = (g_SpyVData.isSpyUidVoiceData() && pCCB->getUID()==g_SpyVData.getDiagUID()); 
	bool blLogDLUidFaxData = (blSpyUidVData && g_SpyVData.isSpyDLUidFaxData());
	
	int i;
	CComMessage *pMsg;
	FaxData10msFrameT *pDacFaxData;
	VoiceTuple tuple = pCCB->getVoiceTuple();
	DMUXVoiDataPkt711T *pSrtp711A = (DMUXVoiDataPkt711T*)pData;
	for(i=0;i<pSrtp711A->head.frameNum;i++)
	{
		pMsg = new (CTVoice::GetInstance(), sizeof(FaxData10msFrameT)) CComMessage;
		if(pMsg!=NULL)
		{
			pDacFaxData = (FaxData10msFrameT*)pMsg->GetDataPtr();
			pDacFaxData->Cid = tuple.Cid;
			pDacFaxData->Codec = pSrtp711A->head.Codec;
			pDacFaxData->Sn = pSrtp711A->head.timeStamp+i;
			memcpy(pDacFaxData->Payload,
				pSrtp711A->Data+i*M_G711_10MS_DATALEN,
				M_G711_10MS_DATALEN);

			pMsg->SetDataLength(sizeof(FaxData10msFrameT));
			pMsg->SetEID(tuple.Eid);
			pMsg->SetMessageId(MSGID_FAX_DATA_DL);
			pMsg->SetSrcTid(M_TID_VOICE);
			pMsg->SetDstTid(M_TID_VAC);
			if(postComMsg(pMsg))
			{
				if(blSpyUidVData)
				{
					g_SpyVData.uCntUID10ms711AFromSAG++;
					g_SpyVData.uCntUID10ms711AToL2++;
					if(blLogDLUidFaxData)
					{
						VPRINT("\n DLRXTX--FN[0x%08X] curUlTxSN[%d] ",
							g_SpyVData.getFrameID(), 
							0xff & (vDatabuf[i].faxDataBuf.curUlTxSN+i));
					}
				}
			}
			else
			{
			}
		}
		else
		{
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"new commessage failed!!!");
		}
	}
}

void VoiceFSM::Parse_Handle_Event(CMessage &msg)
{
	UINT32 *pFN;
#ifdef __UNITTEST__
	OutPutMessage(msg);
#endif
	//�����յ�����Ϣ
	UINT16 msgID = msg.GetMessageId();
	switch(msgID) 
	{

	case MSGID_10ms:
		pFN = (UINT32*)msg.GetDataPtr();
		g_FrameNum = *pFN;
		g_SpyVData.setFrameID(*pFN);
		Voice10msProc();
#ifdef G729TONE_TEST			
		send10msTone2L2();
#endif
		break;

    
	//Voice data messages from tVDR
	case MSGID_VDR_VOICE_DATA:
		handleVoiceDataFromVDR(msg);
		break;

	//Voice data messages from tVAC
	case MSGID_VAC_VOICE_DATA:
	//case MSGID_VOICE_VAC_DATA:		//�������յ�
		handleVoiceDataFromVAC(msg);
		break;

	case MSGID_FAX_DATA_UL:
		handleUplinkFaxData(msg);
		break;
	
	//VAC session Messages
	//case MSGID_VAC_SETUP_CMD:			//�������յ�
	case MSGID_VAC_SETUP_RSP:
	case MSGID_VAC_SETUP_NOTIFY:
	//case MSGID_VAC_RLS_CMD:			//�������յ�
	case MSGID_VAC_RLS_NOTIFY:
	//case MSGID_VAC_START:				//�������յ�
	//case MSGID_VAC_STOP:				//�������յ�
	//case MSGID_VAC_MODIFY_CMD:		//�������յ�
	case MSGID_VAC_MODIFY_RSP:
	case MSGID_VAC_MODIFY_NOTIFY:
		checkVacCtrlMsg(msg);
		InjectMsg(msg);
		break;
	//probe message
	case MSGID_MAC_VOICE_PROBERSP:

	//FSM timeout Messages
	case MSGID_TIMER_ASSIGN:
	case MSGID_TIMER_VAC:
	case MSGID_TIMER_ERRRSP:
	case MSGID_TIMER_RELRES:
	case MSGID_TIMER_PROBERSP:
	case MSGID_TIMER_WAITSYNC:
	case MSGID_TIMER_DELAY_RELVAC:
		//��Ϣע��״̬��
		InjectMsg(msg);
		break;
	//beatheart timeout Message
	case MSGID_TIMER_BEATHEART:
		handleBeatHeartTimeout(msg);
		break;
	//congestion timeout Message
	case MSGID_TIMER_CONGEST:
		handleCongestionTimeout(msg);
		break;
	
	//call signal from tVAC
	case MSGID_VAC_VOICE_SIGNAL:
	//case MSGID_VOICE_VAC_SIGNAL:	//�������յ�
		handleVACCallSignal(msg);
		break;

	//call signal from tVCR
	case MSGID_VCR_VOICE_SIGNAL:
	//case MSGID_VOICE_VCR_SIGNAL:	//�������յ�
		handleVCRCallSignal(msg);
		break;
	//signal from DAC, ע���Ȩ�Ͷ���Ϣ��
	case MSGID_DAC_VOICE_SIGNAL:
		handleDACSignal(msg);
		break;
	case MSGID_DAC_UL_CPEOAM_MAG:	//����͸����oam��Ϣ
		handleULDACCpeOamMsg(msg);
		break;
	case MSGID_CPE_UL_RELAY_MSG:
		handleULCpeRelayMsg(msg);
		break;
	//tVoice��tCM֮����Ϣ
	case MSGID_VOICE_UT_REG:						//Voice Port Register
		handleVoicePortReg(msg);
		break;
	case MSGID_VOICE_UT_UNREG:						//Voice Port UnRegister
		handleVoicePortUnReg(msg);
		break;
	case MSGID_VOICE_UT_MOVEAWAY:
		handleVoicePortMoveAway(msg);				//CPE move away
		break;
	
	case MSGID_CONGESTION_REPORT:
		handleL2CongestionReportMsg(msg);
		break;
	case MSGID_VOICE_SET_CFG:		//Voice����
		handleSetCfgMsg(msg);
		break;

	case MSGID_TMOUT_REG_LOOP:
	case MSGID_TMOUT_REG_RSP:
	case MSGID_TMOUT_HANDSHAKE:
	case MSGID_TMOUT_HANDSHAKERSP:
		handleNatApTimerMsg(msg);
		break;
	case MSGID_VCR_VOICE_SABIS1_RESET:
		handleSAbis1IFResetMsg(msg);
		break;
	case MSGID_VCR_VOICE_FORCE_UT_REGISTER:
		handleMakeAllVCpesRegVSrvMsg(msg);
		break;
	case MSGID_VCR_VOICE_RELEASE_VOICE_SRV:
		handleRelVoiceSrvMsg(msg);
		break;

//=================��Ⱥ���=====================begin
//from L2
	case MSGID_GRP_L2L3_RES_RSP:
		handle_ResCfm_frmL2(msg);
		break;
	case MSGID_GRP_L2L3_PAGING_RSP:
		handle_GrpPagingRsp_frmL2(msg);
		break;
	case MSGID_GRP_L2L3_CPE_STATUS_REPORT_IND:
		handle_StatusReportInd_frmL2(msg);
		break;
	case MSGID_GRP_L2L3_BTS_PTT_PRESS_REQ:
		handle_L2L3BtsPttPressReq(msg);
		break;
	case MSGID_GRP_L2L3_BTS_PTT_PRESS_CANCEL:
		handle_L2L3BtsPttPressCancel(msg);
		break;
	case MSGID_GRP_L2L3_BTS_PTT_PRESS_RELEASE:
		handle_L2L3BtsPttPressRelease(msg);
		break;
#ifdef M_SYNC_BROADCAST
	case MSGID_L2L3_BTSL2SXC_UL_MSG:
		handle_BtsL2SxcULMsg_frmL2(msg);
		break;
	case MSGID_GRP_L2L3_MBMS_MEDIA_DELAY_RSP:
		handle_L2L3MBMSMediaDelayRsp(msg);
		break;
#endif//M_SYNC_BROADCAST			
//from SXC
	//��handleVcrCallSignal()�д���
//from CPE
	case MSGID_GRP_DAC_UL_L3_SIGNAL:
		handle_GrpL3Addr_signal_frmCPE(msg);
		break;
	case MSGID_GRP_DAC_UL_UID_SIGNAL:
		handle_GrpUID_signal_frmCPE(msg);
		break;
	case MSGID_GRP_DAC_HO_RES_REQ:
		handle_HoResReq_frmCPE(msg);
		break;
	case MSGID_GRP_DAC_RES_REQ:
		handle_GrpResReq_frmCPE(msg);
		break;
	case MSGID_GRP_DAC_PTT_RES_REQ:
		handle_GrpDacPttPressReq(msg);
		break;
//��ʱ����Ϣ		
	case MSGID_TIMER_GRP_PAGING_RSP: 		//(0x2370)//Tm_GrpPagingRsp	���Ѱ����Ӧ��ʱ��	10��	0x2370
		handle_Timeout_GrpPagingRsp(msg);
		break;
	case MSGID_TIMER_GRP_LEPAGING_RSP: 	//(0x2371)//Tm_LePagingRsp	����ٺ����Ѱ����Ӧ��ʱ��	������Ĭ��0.5��	0x2371
		handle_Timeout_LePagingRsp(msg);
		break;
	case MSGID_TIMER_GRP_STATUSREPORT: 	//(0x2372)//Tm_StatusReport	���״̬�������ڶ�ʱ��	������Ĭ��5��	0x2372
		handle_Timeout_StatusReport(msg);
		break;
	case MSGID_TIMER_GRP_LEPAGING_LOOP: 	//(0x2373)//Tm_LePagingLoop	����ٺ�������ڶ�ʱ��	��SABis1�ĵ�	0x2373
		handle_Timeout_LePagingLoop(msg);
		break;
	case MSGID_TIMER_GRP_RES_CLEAR: 		//(0x2374)//Tm_ResClear	����տڹ㲥��Դ�ӳ��ͷŶ�ʱ��	������Ĭ��10��	0x2374
		handle_Timeout_ResClear(msg);
		break;
	case MSGID_TIMER_GRP_LEPAGING_START: 	//(0x2375)//Tm_LePagingStart	�ٺ���뿪ʼ��ʱ��	������Ĭ��20��	0x2375
		handle_Timeout_LePagingStart(msg);
		break;
	case MSGID_TIMER_GRP_DATA_DETECT:		//(0x2376)//Tm_GrpToneDetect	�����������ⶨʱ��	������Ĭ��60��	0x2376
		handle_Timeout_GrpDataDetect(msg);
		break;
	case MSGID_TIMER_GRP_RLS: 				//(0x2377)//Tm_GrpRls	�յ�Group release���ӳ�50ms�ͷſտ���Դ��ʱ��	50ms	0x2377
		handle_Timeout_GrpRls(msg);
		break;
//from other bts
	case MSGID_GRP_HO_RES_REQ:
		handle_HoResReq_frmOtherBTS(msg);
		break;
	case MSGID_GRP_HO_RES_RSP:
		handle_HoResRsp_frmOtherBTS(msg);
		break;
//=================��Ⱥ���=====================end
	case MSGID_DAC_UL_CPE_TELNO_MSG:
		handleTelNORegMsg(msg);
		break;
	case MSGID_SAG_BTS_PLAYTONE:
		addPlayToneItem(msg);
		break;
	case MSGID_SAG_BTS_STOPTONE:
		delPlayToneItem(msg);
		break;
//�����鲥������begin		
	case MSGID_CPE_DCS_DATA_UPLINK:
		handle_DB_Data_frmCPE(msg);
		break;
	case MSGID_CPE_DCS_UPLINK_SIGNAL:
		handle_DB_Signal_frmCPE(msg);
		break;
	case MSGID_DCS_VOICE_MSG:
		handle_Msg_frmDCS(msg);
		break;
//�����鲥������end
	default:
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_UNEXPECTED_MSGID), "VoiceFSM::Parse_Handle_Event, received Message with invalid MessageID!!!");
		break;
	}

}

//�����EMS�·�����������������Ϣ
void VoiceFSM::handleSetCfgMsg(CMessage& msg)
{
	//reload cfg for tVoice task
	reloadVoiceCfg();   
	//send msg to tVCRs&tVDRs to tell them cfg changed
	sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VDR, M_TID_VOICE, MSGID_VOICE_SET_CFG);
	sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VCR, M_TID_VOICE, MSGID_VOICE_SET_CFG);
	sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VDR1, M_TID_VOICE, MSGID_VOICE_SET_CFG);
	sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VCR1, M_TID_VOICE, MSGID_VOICE_SET_CFG);
}

//resend this msg to natap session task
void VoiceFSM::handleNatApTimerMsg(CMessage& msg)
{
	TID srcTid = msg.GetSrcTid();
 	switch(srcTid)
 	{
		case M_TID_VCR:
		case M_TID_VDR:
		case M_TID_VCR1:
		case M_TID_VDR1:
		case M_TID_DGRV_LINK:
			msg.SetDstTid(msg.GetSrcTid());
			msg.SetSrcTid(M_TID_VOICE);
			if(!msg.Post())
			{
				LOG(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
					"handleNatApTimerMsg() send msg error!!!");
				msg.DeleteMessage();
			}
			break;
		default:
			LOG3(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
				"NatApTimerMsg srcTid error!!! msgid[0x%04X], srcTid[%d] dstTid[%d]",
				msg.GetMessageId(), srcTid, M_TID_VOICE);
			break;
 	}
}

void VoiceFSM::checkVacCtrlMsg(CMessage& msg)
{
	VoiceTuple tuple;
	CMsg_VACSessionIFMsg VACSessionMsg(msg);
	tuple.Eid = VACSessionMsg.GetEID();
	tuple.Cid = VACSessionMsg.GetCid();
	VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(NULL==pCCB)
	{
		LOG1(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"VAC session Ctrl msg, cannot find CCB, force CPE-EID[0x%08X] to register...", tuple.Eid);
		notifyOneCpeToRegister(tuple.Eid, false);
	}
}

void VoiceFSM::checkCCBSrvStatus()
{
	map<UINT32, UINT16>::iterator itL3Addr;	
	for(itL3Addr=CCBTable->BTreeByL3addr.begin();itL3Addr!=CCBTable->BTreeByL3addr.end();itL3Addr++)
	{
		if((*itL3Addr).second < VOICE_CCB_NUM)
		{
			VoiceCCB *pCCB = &CCBTable->CCBTable[(*itL3Addr).second];
			if(VOICE_RELEASE_STATE==pCCB->GetCurrentState())
			{
				//�ݶ�20��
				 if( pCCB->getInStateTime(g_FrameNum)>2000 )
			 	{
					LOG1(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
						 "CCB UID[0x%08X] in RELEASE state too long, goto IDLE state...", 
						 pCCB->getUID());
					pCCB->releaseActiveCall();
			 	}
			}
		}		
	}
}

void VoiceFSM::sagPlayTone()
{
	if(g_blUseLocalSag)
	{
		bool blSpyUidVData = g_SpyVData.isSpyUidVoiceData();
		CMsg_VACVoiceData VACVoiceData;
		if ( !VACVoiceData.CreateMessage(*CTVoice::GetInstance()) )
		{
		        LOG(LOG_SEVERE, LOGNO(SAG, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		        return;
		}
		VACVoiceData.SetDstTid(M_TID_VAC);
		VACVoiceData.SetMessageId(MSGID_VOICE_VAC_DATA);
		UINT8	*VACBuf ;
		int VACMsgLen;
		UINT8	*cur_VACData_ptr;		
		UINT16 nLenG729Frame = ( sizeof(VACVoiceDataHeaderT) + M_G729_10MS_DATALEN);
		VACBuf = (UINT8*)VACVoiceData.GetDataPtr();
		VACMsgLen = 1;
		VACBuf[0] = 0;
		UINT16 i;
		map<VoiceTuple, UINT16>::iterator itPlayTone;
		for(itPlayTone=CCBTable->BTreePlayToneTbl.begin();itPlayTone!=CCBTable->BTreePlayToneTbl.end();itPlayTone++)
		{
			UINT16 toneID = (*itPlayTone).second;
			VoiceTuple tuple = (*itPlayTone).first;
			//ָ��VAC������Ϣ�е�ǰҪ������������
			cur_VACData_ptr = &VACBuf[VACMsgLen];
			VACVoiceDataIE_T* pVACDataIE = (VACVoiceDataIE_T*)cur_VACData_ptr;
			pVACDataIE->header.Cid = tuple.Cid;
			VSetU32BitVal(pVACDataIE->header.Eid , tuple.Eid);
			pVACDataIE->header.SN = g_ToneTbl[toneID].sn;
			pVACDataIE->header.Type = CODEC_G729A;
			memcpy(pVACDataIE->VoiceData, 
				g_ToneTbl[toneID].pG729Tone + g_ToneTbl[toneID].nCurOffset, 
				M_G729_10MS_DATALEN);
			//ready for next
			++VACBuf[0];
			VACMsgLen += nLenG729Frame;
			cur_VACData_ptr += nLenG729Frame;
			
			//fengbing 20091113 for spyVData with jitterbuffer-----begin
			if(blSpyUidVData)
			{			
				VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
				if(pCCB!=NULL)
				{
					if(pCCB->getUID() == g_SpyVData.getDiagUID())
					{
						++g_SpyVData.uCntUID10msVDataToVAC;
					}
				}
			}				
			//fengbing 20091113 for spyVData with jitterbuffer -----end	
			
		}
		

		//������������btsL2
		if(VACBuf[0]>0)		//�����������
		{
			VACVoiceData.SetPayloadLen(VACMsgLen);	//��Ϣ����
			if(VACVoiceData.Post())
			{
				Counters.nVoiDataToVAC++;
				Counters.n10msPktToVAC += VACBuf[0];
			}
		}
		else				//���������Ϣ
		{
			VACVoiceData.DeleteMessage();
		}
		for(i=0;i<TONE_COUNT;i++)
		{
			g_ToneTbl[i].sn++;
			g_ToneTbl[i].nCurOffset+=M_G729_10MS_DATALEN;
			if(g_ToneTbl[i].nCurOffset>=g_ToneTbl[i].nToneSize)
			{
				g_ToneTbl[i].nCurOffset = 0;
			}
		}
	}

}

void VoiceFSM::addPlayToneItem(CMessage& msg)
{
	if(g_blUseLocalSag)
	{
		PlayToneInfoT* pData = (PlayToneInfoT*)msg.GetDataPtr();
		UINT32 uid = VGetU32BitVal(pData->UID);
		UINT16 toneID = VGetU16BitVal(pData->toneID);
		VoiceTuple tuple;
		tuple.Eid = msg.GetEID();
		tuple.Cid = pData->CID;
		CCBTable->AddBTreePlayToneRecord(tuple, toneID);
		LOG2(SAG_LOG_PLAYTONE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"PlayTone UID[0x%08X] tone[%s]", uid, (int)g_ToneTbl[toneID].toneName);	
	}
}
void VoiceFSM::delPlayToneItem(CMessage& msg)
{
	if(g_blUseLocalSag)
	{
		StopToneInfoT* pData = (StopToneInfoT*)msg.GetDataPtr();
		UINT32 uid = VGetU32BitVal(pData->UID);
		UINT16 toneID = VGetU16BitVal(pData->toneID);
		VoiceTuple tuple;
		tuple.Eid = msg.GetEID();
		tuple.Cid = pData->CID;		
		CCBTable->DelBTreePlayToneRecord(tuple);
		LOG2(SAG_LOG_PLAYTONE, LOGNO(SAG, EC_L3VOICE_NORMAL), 
			"StopTone UID[0x%08X] tone[%s]", uid, (int)g_ToneTbl[toneID].toneName);	
	}
}

void VoiceFSM::handleTelNORegMsg(CMessage& msg)
{
	if(g_blUseLocalSag)
	{
		msg.SetDstTid(M_TID_SAG);
		msg.SetSrcTid(M_TID_VOICE);
		if(!msg.Post())
		{
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"Failed sending msg to localSAG!!!");	
		}
	}
}

#ifdef M__SUPPORT__ENC_RYP_TION
bool tellBtsL2SrvStatus()
{
	typedef struct __SrvStatus
	{
		UINT8 sagStatus[4];
	}SrvStatusT;
	CComMessage *pComMsg = new (CTVoice::GetInstance(), sizeof(SrvStatusT)) CComMessage;
	if(pComMsg!=NULL)
	{
		SrvStatusT* pData = (SrvStatusT*)pComMsg->GetDataPtr();
		VSetU32BitVal(pData->sagStatus, sagStatusFlag);

		pComMsg->SetMessageId(MSGID_SAGSTATUS_REPORT);
		pComMsg->SetDstTid(M_TID_VAC);
		pComMsg->SetSrcTid(M_TID_VOICE);
		return postComMsg(pComMsg);
	}
	return false;
}
#endif
/*
ֹ֮ͣǰ����ҵ��ԭ��:
1)������������״̬(localSag���)ʱ
2)����sag�л�ʱ
�ն�����ע��ԭ��:
1)�����sag(������localSag)�����仯ʱ
2)localSag�����ն�����ע��
*/
void SAbis1IfProc(CMessage& msg)
{
	//������·״̬��־
	if(M_TID_VCR==msg.GetSrcTid())
	{
		blSAbisIFOK_Master_old = blSAbisIFOK_Master;
		blSAbisIFOK_Master = true;
	}
	if(M_TID_VCR1==msg.GetSrcTid())
	{
		blSAbisIFOK_Backup_old = blSAbisIFOK_Backup;
		blSAbisIFOK_Backup = true;
	}
	blSAbisIFOK_old = blSAbisIFOK;
	blSAbisIFOK = blSAbisIFOK_Master || blSAbisIFOK_Backup;
	sagStatusFlag = blSAbisIFOK;


	if(blSAbisIFOK_old!=blSAbisIFOK)
	{
		//��tSag�����˳�������Ϣ
		sendNopayloadMsg(CTVoice::GetInstance(), M_TID_SAG, M_TID_VOICE, MSGID_BTS_SAG_STOP_SRV);
		//������������״̬��ֹ֮ͣǰ������ҵ��
		sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, MSGID_VCR_VOICE_RELEASE_VOICE_SRV);
#ifdef M__SUPPORT__ENC_RYP_TION	
		tellBtsL2SrvStatus();
#endif
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			" sagStatusFlag[%d] ", sagStatusFlag);
	}


	if(M_TID_VCR==msg.GetSrcTid())
	{
		//��sag�ָ�
		if(!blSAbisIFOK_Master_old)
		{
			//�ɱ���sag�л�����sag
			if(blSAbisIFOK_Backup)
			{
				//�ɱ���sag�л�����sag��ֹ֮ͣǰ������ҵ��
				sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, MSGID_VCR_VOICE_RELEASE_VOICE_SRV);
			}
			//��sag�ָ����ն�����ע��
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, 
				MSGID_VCR_VOICE_FORCE_UT_REGISTER);
		}
	}
	if(M_TID_VCR1==msg.GetSrcTid())
	{
		//��sag����ʱ��sag�ָ�
		if(!blSAbisIFOK_Master && !blSAbisIFOK_Backup_old)
		{
			//��sag����ʱ��sag�ָ����ն�����ע��
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, 
				MSGID_VCR_VOICE_FORCE_UT_REGISTER);
		}
	}

}
void VoiceFSM::handleSAbis1IFResetMsg(CMessage& msg)
{
	//������·״̬��־
	if(msg.GetSrcTid()==M_TID_VCR)
	{
		blSAbisIFOK_Master_old = blSAbisIFOK_Master;
		blSAbisIFOK_Master = false;
	}
	if(msg.GetSrcTid()==M_TID_VCR1)
	{
		blSAbisIFOK_Backup_old = blSAbisIFOK_Backup;
		blSAbisIFOK_Backup = false;
	}
	blSAbisIFOK_old = blSAbisIFOK;
	blSAbisIFOK = blSAbisIFOK_Master || blSAbisIFOK_Backup;
	sagStatusFlag = blSAbisIFOK;

	
	if(blSAbisIFOK_old!=blSAbisIFOK)
	{
		//������������״̬��ֹ֮ͣǰ������ҵ��
		sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, MSGID_VCR_VOICE_RELEASE_VOICE_SRV);
#ifdef M__SUPPORT__ENC_RYP_TION	
		tellBtsL2SrvStatus();
#endif
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			" sagStatusFlag[%d] ", sagStatusFlag);
	}


	if(msg.GetSrcTid()==M_TID_VCR)
	{
		//��sag�л�����sag
		if(blSAbisIFOK_Master_old && blSAbisIFOK_Backup)
		{
			//��sag�л�����sag��ֹ֮ͣǰ������ҵ��
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, MSGID_VCR_VOICE_RELEASE_VOICE_SRV);
			//��sag�л�����sag���ն�����ע��
			sendNopayloadMsg(CTVoice::GetInstance(), M_TID_VOICE, M_TID_VOICE, 
				MSGID_VCR_VOICE_FORCE_UT_REGISTER);
		}
	}
	if(msg.GetSrcTid()==M_TID_VCR1)
	{
	}

}

void VoiceFSM::handleMakeAllVCpesRegVSrvMsg(CMessage& msg)
{
	notifyAllCPEtoRegister();
	LOG(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"make all Voice CPEs to Register Voice Srv...");
}
void VoiceFSM::handleRelVoiceSrvMsg(CMessage& msg)
{
	LOG(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"release all Voice Service...");

	//�����ͷ�
	VoiceCCB *pCCB = NULL;
	map<UINT32, UINT16>::iterator itL3Addr;
	for(itL3Addr=CCBTable->BTreeByL3addr.begin();itL3Addr!=CCBTable->BTreeByL3addr.end();itL3Addr++)
	{
		pCCB = &CCBTable->CCBTable[(*itL3Addr).second];
		if(pCCB!=NULL)
		{
			pCCB->releaseActiveCall();
#if 0		
			pCCB->VACRelease();
			m_pStateTable[pCCB->GetCurrentState()]->Exit(*pCCB);
			m_pStateTable[VOICE_IDLE_STATE]->Entry(*pCCB);
			pCCB->SetCurrentState(VOICE_IDLE_STATE);
#endif			
		}
	}
	//����ͷ�
	GrpCCB *pGrpCCB = NULL;
	map<UINT16, UINT16>::iterator itGID;
	for(itGID=pGrpCCBTbl->BTreeByGID.begin();itGID!=pGrpCCBTbl->BTreeByGID.end();itGID++)
	{
		pGrpCCB = &pGrpCCBTbl->grpCCBTbl[(*itGID).second];
		if(pGrpCCB!=NULL)
		{
			pGrpCCB->sendGrpRelease2CPE(1);
			LOG1(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GID[0x%04X] BTS-SXC signal link down, release Group Call!!!###########", 
				pGrpCCB->getGID());
			pGrpCCB->clearAllTimers();
			pGrpCCB->startGrpTimer(TIMER_GrpRls);
			pGrpCCB->SetCurrentState(GRP_RELEASE_STATE);
			LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GID[0x%04X] Enter state[GRP_RELEASE_STATE]", pGrpCCB->getGID());
		}
	}
	
}

/*
 *	when Voice Port Register received, call this function to alloc CCB, and update
 *	CCB and BTree with UID and (EID,CID)
 */
void VoiceFSM::handleVoicePortReg(CMessage& msg)
{
	CMsg_VoicePortReg regMsg(msg);				
	UINT32 uidNew = regMsg.GetUid();

	LOG3(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVoicePortReg [ (0x%08x, 0x%02x), 0x%08x ]",
	    regMsg.GetEID(), regMsg.GetCid(), uidNew);

	VoiceTuple tupleMsg;
	tupleMsg.Eid = msg.GetEID();
	tupleMsg.Cid = regMsg.GetCid();

	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByUID(uidNew);
	VoiceCCB* pCCB_Tuple = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tupleMsg);
	
	if(pCCB==pCCB_Tuple && pCCB!=NULL)
	{
		//û��Zģ������̣���Zģ�鿪�ص������������������·���CCB��Դ
		//��ֹͣmoveaway�������ӳ�disabel��ʱ��,�����Ժ�ĳʱ��disable
		pCCB->setCPEZFlag(regMsg.isCPEZ());	//CPEZ flag
		pCCB->deleteTimerMoveAwayDisable();
		LOG3(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Voice Port Register Message received, CCB already exist,[ (0x%08x, 0x%02x), 0x%08x ]",
		            tupleMsg.Eid, tupleMsg.Cid, uidNew);
		return;
	}
	else	
	{	
		//���Zģ�����򿪹ص�����,���ͷ�ʧЧ����Դ���������µ���Դ
		if(pCCB_Tuple!=NULL)
		{
			pCCB_Tuple->deleteTimerMoveAwayDisable();
			pCCB_Tuple->VACRelease();		//VAC release,let l2 release resource
			//pCCB->sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_CPERELEASE);	//let SAG release resource
			CCBTable->DeAllocCCB(pCCB_Tuple);
			LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "DeAllocCCB success by TUPLE (0x%08x, 0x%02x)",
				            tupleMsg.Eid, tupleMsg.Cid);
		}
		if(pCCB!=NULL)
		{
			pCCB->deleteTimerMoveAwayDisable();
			pCCB->VACRelease();		//VAC release,let l2 release resource
			//pCCB->sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_CPERELEASE);	//let SAG release resource
			CCBTable->DeAllocCCB(pCCB);
			LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "DeAllocCCB success by UID (0x%08x)",
			        uidNew);
		}
	}
	
	//Alloc CCB
	pCCB = CCBTable->AllocCCB();
	if(pCCB==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "AllocCCB() returned NULL, allocCCB failed");
	}
	else
	{
		//��ֹͣmoveaway�������ӳ�disabel��ʱ��,�����Ժ�ĳʱ��disable
		pCCB->deleteTimerMoveAwayDisable();
		
		pCCB->setUID(uidNew);		//update UID for CCB
		pCCB->setVoiceTuple(tupleMsg);		//update (EID,CID) for CCB
		pCCB->setCPEZFlag(regMsg.isCPEZ());	//CPEZ flag
		//update BTree 
		CCBTable->AddBTreeTuple(tupleMsg, pCCB->getTabIndex());
		CCBTable->AddBTreeUID(pCCB->getUID(), pCCB->getTabIndex());
		
		LOG3(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Voice Port Register Message received, AllocCCB success, [ (0x%08x, 0x%02x), 0x%08x ]", 
		        tupleMsg.Eid, tupleMsg.Cid, uidNew);
	}

	//֪ͨlocalSag�ն˲��ڱ���վ
	if(g_blUseLocalSag)
	{
		//localSAGʹ�ñ����û���Ϣ�����ļ�ʱ��֪ͨ�û����룬�����������û��ն˴洢�ĵ绰���������Ϣ
		if(g_blUseLocalUserInfoFile)
		{
			msg.SetDstTid(M_TID_SAG);
			msg.SetSrcTid(M_TID_VOICE);
			if(!msg.Post())
			{
				LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"Failed sending msg to localSAG!!!");	
			}
		}
	}	
}
/*
 *	when Voice Port UnRegister received, call this function to DeAlloc CCB, and update
 *	CCB and clear BTree for this CCB
 */
void VoiceFSM::handleVoicePortUnReg(CMessage& msg)
{
	CMsg_VoicePortUnReg unRegMsg(msg);	
	VoiceTuple tuple;
	tuple.Eid = unRegMsg.GetEID();
	tuple.Cid = unRegMsg.GetCid();

	LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVoicePortUnReg, (0x%08x, 0x%02x)", 
	    tuple.Eid, tuple.Cid);
	
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(pCCB==NULL)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Voice Port UnRegister Message received, but cannot find CCB");
	}
	else
	{
		pCCB->deleteTimerMoveAwayDisable();
		pCCB->VACRelease();		//VAC release,let l2 release resource
		//pCCB->sendErrNotifyReqtoSAG(M_ERRNOTIFY_ERR_CAUSE_CPERELEASE);	//let SAG release resource
		CCBTable->DeAllocCCB(pCCB);
		LOG2(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Voice Port UnRegister Message received, DeAllocCCB success by TUPLE (0x%08x, 0x%02x)",
		    tuple.Eid, tuple.Cid);
		
		//֪ͨlocalSag�ն˲��ڱ���վ
		if(g_blUseLocalSag)
		{
			msg.SetDstTid(M_TID_SAG);
			msg.SetSrcTid(M_TID_VOICE);
			if(!msg.Post())
			{
				LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"Failed sending msg to localSAG!!!");	
			}
		}
	}
	
}

//rcv this msg when cpe move to another BTS
void VoiceFSM::handleVoicePortMoveAway(CMessage& msg)
{
	//������ʱ��Tmoveawaydisable����ʱ����Disable voice port��Ϣ
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVoicePortMoveAway, start timer to delay Disable port");
	CMsg_VoicePortUnReg unRegMsg(msg);
	VoiceTuple tuple;
	tuple.Eid = unRegMsg.GetEID();
	tuple.Cid = unRegMsg.GetCid();
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(pCCB==NULL)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Voice Port MoveAway Message received, but cannot find CCB");
	}
	else
	{
		pCCB->startTimerMoveAwayDisable();
	}	
}

extern "C" void checkVDRNatApSession()
{
#ifdef NATAP_FUNCTION
	if(CVCR::GetInstance()->IsSAGConnected())
	{
		if(!getVdrNatpiSession()->isSessionAlive())
		{
			sendNopayloadMsg(CTVoice::GetInstance(), 
				M_TID_VDR, M_TID_VOICE, MSGID_NATAP_RESTART);
		}
	}
	if(CVCR::GetBakInstance()->IsSAGConnected())
	{
		if(!getVdr1NatpiSession()->isSessionAlive())
		{
			sendNopayloadMsg(CTVoice::GetInstance(), 
				M_TID_VDR1, M_TID_VOICE, MSGID_NATAP_RESTART);
		}
	}
#endif	
}

/*
 *	beatheart timer timeout, send beatheart message to SAG
 */
void VoiceFSM::handleBeatHeartTimeout(CMessage& msg)
{
	static UINT32 cnt=0;
	if(cnt++ & 0x1)
	{
		if(g_SpyVData.isShowPeriodic())
		{
			g_SpyVData.showDiagResult();
		}
	}
	else
	{
#ifdef M__SUPPORT__ENC_RYP_TION
		tellBtsL2SrvStatus(M_TID_VOICE);
#endif		
#ifdef BROADCAST_SMS_TEST	
		bchSmsTest(g_BchSMSStr, g_BchSmsTestDisplayNumber);
#endif
		//����Ƿ��к��й���
		checkCCBSrvStatus();
	}
	
	//���VDR�����natap session�Ƿ�����
	checkVDRNatApSession();

	if(!blUseBeatHeart)
		return;
	
	//LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleBeatHeartTimeout");

	m_BeatHeart_Manager.handleBeatHeartTmoutMsg(msg);
	
}
/*
 *	congestion timer timeout, send congestion request to L2
 */
void VoiceFSM::handleCongestionTimeout(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleCongestionTimeout");

	CMsg_CongestReqtoL2 CongestReqtoL2;
	if ( !CongestReqtoL2.CreateMessage(*CTVoice::GetInstance(), 1) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CongestReqtoL2.SetDstTid(M_TID_VAC);	//Ŀ������M_TID_L2OAM
		CongestReqtoL2.SetMessageId(MSGID_CONGESTION_REQUEST);
		if(!CongestReqtoL2.Post())
		{
			CongestReqtoL2.DeleteMessage();
		}
    	
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Congest Timer timeout message received, send Congestion Req message to L2");
	}    
}

//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
void VoiceFSM::handle_DVoiceConfigReq(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT8 DVoice = pData->sigPayload.DVoiceCfgReq.DVoice;
	UINT32 L3Addr1 = VGetU32BitVal(pData->sigPayload.DVoiceCfgReq.L3Addr1);
	UINT32 L3Addr2 = VGetU32BitVal(pData->sigPayload.DVoiceCfgReq.L3Addr2);
	
	LOG3(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"DVoiceConfigReq<----SAG; DVoice[0x%02X] L3Addr1[0x%08X] L3Addr2[0x%08X] >>>>>>>>",
		DVoice, L3Addr1, L3Addr2);
	
	VoiceCCB *pCCB1 = (VoiceCCB*)CCBTable->FindCCBByL3addr(L3Addr1);
	VoiceCCB *pCCB2 = (VoiceCCB*)CCBTable->FindCCBByL3addr(L3Addr2);
	if(DISABLE_BTS_VDATA_INNER_SWITCH==DVoice)
	{
		//ȡ���ڲ�����
		if(pCCB1!=NULL)
		{
			pCCB1->disableInnerSwitch();
		}
		if(pCCB2!=NULL)
		{
			pCCB2->disableInnerSwitch();
		}
	}
	else if(ENABLE_BTS_VDATA_INNER_SWITCH==pData->sigPayload.DVoiceCfgReq.DVoice)
	{
		//��ʼ�ڲ�����
		if(pCCB1!=NULL && pCCB2!=NULL)
		{
			pCCB1->enableInnerSwitch(pCCB2);
			pCCB2->enableInnerSwitch(pCCB1);
		}
		else
		{
			//do nothing
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "handle_DVoiceConfigReq(), at least one pCCB is NULL!!!");
		}
	}
	else
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "invalid DVoice field value!!!");
		//unknow value,do nothing
	}
}
void VoiceFSM::handle_RelTransResReq(CMessage& msg)
{
	//ȡ���ڲ�����
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT32 L3Addr = VGetU32BitVal(pData->sigPayload.RlsResReq.L3addr);
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByL3addr(L3Addr);
	if(pCCB!=NULL)
	{
		pCCB->disableInnerSwitch();
	}
}
#endif

//20090531 fengbing bts inner switch for Voice Data end


/*
 *	handle call signal messages received from tVCR
 */
void VoiceFSM::handleVCRCallSignal(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVCRCallSignal");
	Counters.nSigFromVCR++;
	CMsg_Signal_VCR signalMsgVCR(msg);
	SignalType sigType = signalMsgVCR.ParseMessageFromSAG();
	switch(sigType) 
	{
	//͸������Ϣ
	case UTSAG_L3Addr_MSG:
		handleVCR_UTSAG_Msg_toVAC(msg);
		break;
	case UTSAG_UID_MSG:
		handleVCR_UTSAG_Msg_toDAC(msg);
		break;
	case UTSAG_UID_OAM_MSG: 	
		handleVCR_UTSAG_OAM_Msg_toDAC(msg);
		break;
	case UTSAG_RELAY_MSG:
		handleVCR_UT_DL_Relay_Msg(msg);
		break;
//=================��Ⱥ���=====================begin
//͸������Ϣ
	case UTSXC_GRPUID_MSG:
		handle_GrpUID_Signal_frmSXC(msg);
		break;
	case UTSXC_GRPL3Addr_MSG:
		handle_GrpL3Addr_Signal_frmSXC(msg);
		break;
//�����������Ϣ
	//case LAPaging_MSG:
	case LAGrpPagingReq_MSG:
		handle_LAGrpPaging_frmSXC(msg);
		break;
	case LEGrpPaging_MSG:
		handle_LEGrpPaging_frmSXC(msg);
		break;
	case DeLEGrpPaging_MSG:
		handle_DeLEGrpPaging_frmSXC(msg);
		break;
	case GrpHandoverRsp_MSG:
		handle_GrpHandoverRsp_frmSXC(msg);
		break;
	case GrpResRsp_MSG:
		handle_GrpResRsp_frmSXC(msg);
		break;
	case PTT_PressApplyRsp_MSG:
		handle_GrpPttPressApplyRsp_frmSXC(msg);
		break;
#ifdef M_SYNC_BROADCAST
	case BTSL2_SAG_MSG:
		handle_BtsL2SxcDLMsg_frmSXC(msg);
		break;
#endif//M_SYNC_BROADCAST			
	//case ErrNotifyReq_MSG:
		
//=================��Ⱥ���=====================end

	//===================================================================
	case LAPaging_MSG:
		handleLAPagingReq(msg);
		break;
	case DELAPagingReq_MSG:
		handleDELAPagingReq(msg);
		InjectMsg(msg);
		break;
	case AssignResReq_MSG:
	case AssignResRsp_MSG:
	case RlsResReq_MSG:
	//case RlsResRsp_MSG:�������յ�
//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
		if(RlsResReq_MSG==sigType)
		{
			handle_RelTransResReq(msg);
		}
#endif
//20090531 fengbing bts inner switch for Voice Data end	
		InjectMsg(msg);
		break;
//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
	case DVoiceConfigReg_MSG:
		handle_DVoiceConfigReq(msg);
		break;
#endif
//20090531 fengbing bts inner switch for Voice Data end	

	//====================================================================
	//�����������Ϣ
	case Reset_MSG:
		handleReset(msg);
		break;
	case ResetAck_MSG://������
		break;
	case BeatHeart_MSG:
		handleBeatHeart(msg);
		break;
	case BeatHeartAck_MSG://������
		m_BeatHeart_Manager.handleBeatHeartAckMsg(msg);
		break;
	case CongestionCtrlReq_MSG:
		handleCongestReq(msg);
		break;
	case CongestionCtrlRsp_MSG:
		handleCongestRsp(msg);
		break;
    //yhw
	case BROADCAST_SM_MSG:
		//add code
		handleBroadcast_SM(msg);
		break;
	case RESET_REQ_MSG:   
		//add code
		break;
	//edit by yhw	

	//====================================================================
	case ErrNotifyReq_MSG:
		handle_ErrorNotify_frmSXC(msg);
	case ErrNotifyRsp_MSG:
		InjectMsg(msg);
		break;
	//====================================================================
	case StatusQry_MSG:		//�ݲ�֧��
	case Status_MSG:		//�ݲ�֧��
		break;
	//====================================================================
	case InvalidSignal_MSG:
	default:
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_UNEXPECTED_MSGID), "VoiceFSM::handleVCRCallSignal, Unknown message received");
	}

	//�յ�����������Ϣ��Ϊ��·�ǻ�ģ�����Ҫ��������
	if(BeatHeart_MSG!=sigType && BeatHeartAck_MSG!=sigType)
	{
		m_BeatHeart_Manager.whenRcvOtherSignalMsg(msg);
	}
	else
	{
		//localSag�����������������֤��Ϣ��Դ��������sag
		SAbis1IfProc(msg);
	}
}
/*
 *	handle call signal messages received from tVAC, 
 *	all messages should be sent to SAG
 */
void VoiceFSM::handleVACCallSignal(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVACCallSignal");
	Counters.nSigFromVAC++;
	VoiceCCB *pCCB;
	VoiceTuple tuple;
	
	CMsg_Signal_VCR signalMsgVCR;
	VoiceVCRCtrlMsgT* pDataVCR;
	CMsg_UTSAGSignal_VAC signalMsgVAC(msg);
	
	tuple.Eid = msg.GetEID();
	tuple.Cid = signalMsgVAC.GetCID();
	pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(pCCB==NULL)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "cannot find CCB");
		notifyOneCpeToRegister(tuple.Eid, false);
		return;
	}
	
	UINT8* pVACSigPayload=NULL;
	UINT16 nUTSAGSigPayloadLen=0;
	signalMsgVAC.GetSignalPayload(pVACSigPayload, nUTSAGSigPayloadLen);

	if ( !signalMsgVCR.CreateMessage(*CTVoice::GetInstance(), nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
    
	signalMsgVCR.SetDstTid(M_TID_VCR);
	signalMsgVCR.SetMessageId(MSGID_VOICE_VCR_SIGNAL);

	pDataVCR = (VoiceVCRCtrlMsgT*)signalMsgVCR.GetDataPtr();
	signalMsgVCR.SetBTSSAGID();		//SAGID, BTSID
	//EVENT GROUP ID, EVENT ID(net order)
	pDataVCR->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_UTSAG;
	VSetU16BitVal(pDataVCR->sigHeader.Event_ID , M_MSG_EVENT_ID_UTSAG_L3ADDR);
	//ͨ��VAC͸�����������L3��ַ
	VSetU32BitVal(pDataVCR->sigPayload.UTSAG_Payload_L3Addr.L3Addr , pCCB->getL3addr());

	//payload, 
	memcpy((void*)&(pDataVCR->sigPayload.UTSAG_Payload_L3Addr.msgType),
	       (void*)(pVACSigPayload),
		   nUTSAGSigPayloadLen);

	//length, ���㳤�������͸����L3��ַ�ĳ���
	signalMsgVCR.SetSigHeaderLengthField(nUTSAGSigPayloadLen+sizeof(UINT32));
	signalMsgVCR.SetPayloadLength(nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT));

	//���ͨ���Ƿ����ӳɹ�
	if(ConnectAck_MSG==signalMsgVCR.ParseUTSAGMsgToSAG())
	{
		pCCB->setConnectedFlag();
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Connected ******************************,UID[0x%08x] ", pCCB->getUID());
	}

	//transfer message to SAG
	signalMsgVCR.Post();

	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVACCallSignal, UT-SAG message received, transfer to SAG");


}
/*
 *	handle call signal messages received from tDAC, 
 *	all messages should be sent to SAG
 */
void VoiceFSM::handleDACSignal(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleDACSignal");
	Counters.nSigFromDAC++;
	VoiceCCB *pCCB;
	VoiceTuple tuple;

	CMsg_Signal_VCR signalMsgVCR;
	VoiceVCRCtrlMsgT* pDataVCR;
	CMsg_UTSAGSignal_DAC signalMsgDAC(msg);

	tuple.Eid = msg.GetEID();
	tuple.Cid = signalMsgDAC.GetCID();
	pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(pCCB==NULL)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "cannot find CCB");
		notifyOneCpeToRegister(tuple.Eid, false);
		return;
	}

	UINT8* pDACSigPayload=NULL;
	UINT16 nUTSAGSigPayloadLen=0;
	signalMsgDAC.GetSignalPayload(pDACSigPayload, nUTSAGSigPayloadLen);
	
	if ( !signalMsgVCR.CreateMessage(*CTVoice::GetInstance(), nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
    
	signalMsgVCR.SetDstTid(M_TID_VCR);
	signalMsgVCR.SetMessageId(MSGID_VOICE_VCR_SIGNAL);

	pDataVCR = (VoiceVCRCtrlMsgT*)signalMsgVCR.GetDataPtr();
	signalMsgVCR.SetBTSSAGID();		//SAGID, BTSID
	//EVENT GROUP ID, EVENT ID(net order)
	pDataVCR->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_UTSAG;
	VSetU16BitVal(pDataVCR->sigHeader.Event_ID , M_MSG_EVENT_ID_UTSAG_UID);

	//ͨ��DAC͸�����������UID
	VSetU32BitVal(pDataVCR->sigPayload.UTSAG_Payload_Uid.Uid , pCCB->getUID());

	//payload, 
	memcpy((void*)&(pDataVCR->sigPayload.UTSAG_Payload_Uid.msgType),
	       (void*)(pDACSigPayload),
		   nUTSAGSigPayloadLen);

	//length, ���㳤�������͸����UID�ĳ���
	signalMsgVCR.SetSigHeaderLengthField(nUTSAGSigPayloadLen+sizeof(UINT32));
	signalMsgVCR.SetPayloadLength(nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT));
	//transfer message to SAG
	signalMsgVCR.Post();

	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleDACSignal, UT-SAG message received, transfer to SAG");

	if(g_blSMSSelfTestFlag)
	{
	//����Ƕ���Ϣ��ص�������͸�cpe������Ϣ�ڴ�֪ͨ���Ӧ��cpeӦ����Ϣ������֧��cpe�Լ����͸��Լ��Ķ���Ϣ���ܲ���
		UINT8* pDataPayload;
		UINT16 nLenPayload;
		int k;
		signalMsgDAC.GetSignalPayload(pDataPayload, nLenPayload);
		UINT8 msgType = pDataPayload[0];
		if(M_MSGTYPE_MOSMSREQ<=msgType && msgType<=M_MSGTYPE_SMSMEMAVAILRSP)
		{
			if(M_MSGTYPE_SMSMEMAVAILREQ==msgType)
			{
				//change req msg into rsp msg
				pDataPayload[0] = M_MSGTYPE_SMSMEMAVAILRSP;
			}
			if(M_MSGTYPE_MOSMSREQ==msgType)
			{
				pDataPayload[0] = M_MSGTYPE_MTSMSREQ;
			}
			if(M_MSGTYPE_MTSMSRSP==msgType)
			{
				pDataPayload[0] = M_MSGTYPE_MOSMSRSP;
			}
			
			//send msg back to cpe
			signalMsgDAC.SetSrcTid(M_TID_VOICE);
			signalMsgDAC.SetMessageId(MSGID_VOICE_DAC_SIGNAL);
			signalMsgDAC.SetDstTid(M_TID_UTV);
		
			if(signalMsgDAC.Post())
			{
				Counters.nSigToDAC++;
			}
			
			//log msg
			VPRINT("\n");
			for(k=0;k<nLenPayload;k++)
				VPRINT(" 0x%02x", pDataPayload[k]);
			VPRINT("\n");
		}
	}
}

/*
 *	handle UTSAG-OAM messages received from tDAC, 
 *	all messages should be sent to SAG
 */
void VoiceFSM::handleULDACCpeOamMsg(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleULDACCpeOamMsg");
	Counters.nSigFromDAC++;

	VoiceTuple tuple;

	CMsg_Signal_VCR signalMsgVCR;
	VoiceVCRCtrlMsgT* pDataVCR;
	CMsg_UTSAGSignal_DAC signalMsgDAC(msg);

	tuple.Eid = msg.GetEID();
	tuple.Cid = signalMsgDAC.GetCID();
	
	UINT32 tmpUid = l3oamGetUIDByEID(tuple.Eid);
	if(0xffffffff==tmpUid)
	{
		LOG1(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "cannot find UID with EID[0x%08X]", tuple.Eid);
		notifyOneCpeToRegister(tuple.Eid, false);
		return;
	}

	UINT8* pDACSigPayload=NULL;
	UINT16 nUTSAGSigPayloadLen=0;
	signalMsgDAC.GetSignalPayload(pDACSigPayload, nUTSAGSigPayloadLen);
	
	if ( !signalMsgVCR.CreateMessage(*CTVoice::GetInstance(), nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
    
	signalMsgVCR.SetDstTid(M_TID_VCR);
	signalMsgVCR.SetMessageId(MSGID_VOICE_VCR_SIGNAL);

	pDataVCR = (VoiceVCRCtrlMsgT*)signalMsgVCR.GetDataPtr();
	signalMsgVCR.SetBTSSAGID();		//SAGID, BTSID
	//EVENT GROUP ID, EVENT ID(net order)
	pDataVCR->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_CPEOAM;
	VSetU16BitVal(pDataVCR->sigHeader.Event_ID , M_MSG_EVENT_ID_CPEOAM);

	//ͨ��DAC͸�����������UID
	VSetU32BitVal(pDataVCR->sigPayload.UTSAG_Payload_Uid.Uid , tmpUid);

	//payload, 
	memcpy((void*)&(pDataVCR->sigPayload.UTSAG_Payload_Uid.msgType),
	       (void*)(pDACSigPayload),
		   nUTSAGSigPayloadLen);

	//length, ���㳤�������͸����UID�ĳ���
	signalMsgVCR.SetSigHeaderLengthField(nUTSAGSigPayloadLen+sizeof(UINT32));
	signalMsgVCR.SetPayloadLength(nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT));
	//transfer message to SAG
	signalMsgVCR.Post();

	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleULDACCpeOamMsg, UT-SAG message received, transfer to SAG");
}

void VoiceFSM::handleULCpeRelayMsg(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleULCpeRelayMsg");
	Counters.nSigFromDAC++;
	UINT16 len = msg.GetDataLength();
	CMsg_Signal_VCR signalMsgVCR;
	VoiceVCRCtrlMsgT* pDataVCR;
	if ( !signalMsgVCR.CreateMessage(*CTVoice::GetInstance(), 
		len+sizeof(SigHeaderT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
	
	signalMsgVCR.SetDstTid(M_TID_VCR);
	signalMsgVCR.SetMessageId(MSGID_VOICE_VCR_SIGNAL);

	pDataVCR = (VoiceVCRCtrlMsgT*)signalMsgVCR.GetDataPtr();
	signalMsgVCR.SetBTSSAGID(); 	//SAGID, BTSID
	signalMsgVCR.SetSigIDS(UTSAG_RELAY_MSG);

	//payload, 
	memcpy((void*)&(pDataVCR->sigPayload.UTSAG_Signal_XXX.XXX),
		   (void*)(msg.GetDataPtr()),
		   len);

	//length, ���㳤�������͸����UID�ĳ���
	signalMsgVCR.SetSigHeaderLengthField(len);
	signalMsgVCR.SetPayloadLength(len+sizeof(SigHeaderT));
	//transfer message to SAG
	signalMsgVCR.Post();

	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleULCpeRelayMsg, transfer to SAG");
}


/*
 *	UDP-->VAC,�����UDP�յ����������ݰ�������VACDataIE��ʽ������jitterbuffer��
 */

void VoiceFSM::handleVoiceDataFromVDR(CMessage& msg)
{
#ifdef M_SYNC_BROADCAST
	UINT8 formID = ((DMUXHeadT*)(msg.GetDataPtr()))->FormID;
	if(formID==SyncBroadcastVData || formID==SyncBroadcastDData)
	{
		if(gMBMSParaFlag&&gMBMSGPSFlag)
		{
			handle_SyncBroadcastData(msg);
		}
		return;
	}
#endif//M_SYNC_BROADCAST
	//LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVoiceDataFromVDR");
	Counters.nVoiDataFromVDR++;
	UINT8 curCodec;
	UINT32 VAC_EidGid;
	UINT8 VAC_cid;
	UINT32 L3addr;
	GrpCCB* pGrpCCB = NULL;
	VoiceCCB* pCCB = NULL;
	UINT8	i,j,nDataPkt,nFrameNum,nDataLen;
	UINT8	*pUdp = (UINT8*)msg.GetDataPtr();
	UINT16 nTmpVDataLen;

	bool blSpyUidVData = g_SpyVData.isSpyUidVoiceData();
	bool blSpyGidVData = g_SpyVData.isSpyGidVoiceData();
/*
	UINT16	UdpPktlen = msg.GetDataLength();
	VPRINT("\nUDP pkt:");
	for(i=0;i<UdpPktlen;i++)
	{
		VPRINT(" %02X", pUdp[i]);
	}	
	VPRINT("\n");
*/
	CMsg_VACVoiceData VACVoiceData;
	if ( !VACVoiceData.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
    
	VACVoiceData.SetDstTid(M_TID_VAC);
	VACVoiceData.SetMessageId(MSGID_VOICE_VAC_DATA);

	UINT8	*VACBuf = (UINT8*)VACVoiceData.GetDataPtr();
	UINT8	*cur_VACData_ptr;
	cur_VACData_ptr = VACBuf+1;					//ָ��VAC������Ϣ�е�һ����������
	VACVoiceDataIE_T* pVACDataIE = (VACVoiceDataIE_T*)cur_VACData_ptr;

	VACBuf[0] = 0;
	int VACMsgLen = 1;

	nDataPkt = ((DMUXHeadT*)pUdp)->nFrameNum;	//UDP���������ݸ���
    
	DMUXVoiDataCommonT* pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+sizeof(DMUXHeadT));	//ָ���һ����������֡
	//�յ��İ������jitterbuffer��
	for(i=0;i<nDataPkt;i++)
	{
		//����
		//729B֡����pVoiDataCommon->frameNum�м���
		curCodec = pVoiDataCommon->Codec;
		nFrameNum = pVoiDataCommon->frameNum;
		if(isEncSrtpVData(curCodec))
		{
			//��������40ms���
			Counters.n10msPktFromVDR += 4;
		}
		else
		{
			Counters.n10msPktFromVDR += nFrameNum;
		}
		if(pVoiDataCommon->blG729B)
		{
			++Counters.n10msPktFromVDR;
			++Counters.nG729BPktFromVDR;
		}
		switch(curCodec)
		{
			case CODEC_GRPTONE:
			case CODEC_G729A:
				nDataLen = M_G729_10MS_DATALEN;
				break;
			case CODEC_ENCRYPT_GRPTONE:
			case CODEC_ENCRYPT_G729A:
				nDataLen = M_ENC_VDATA_PKT_LEN;
				break;
			case CODEC_G711A:
				nDataLen = M_G711_10MS_DATALEN;
				break;
	//now only 729A&711A codec used, when 729B rcv, the codec field filled with
	//729A and 729B flag bit is set
	#if 0			
			case CODEC_G729B_SID:
				nDataLen = 2;
				break;
			case CODEC_G729B_UNT:
				nDataLen = 0;
				break;
			case CODEC_G729D:
				nDataLen = 8;
				break;
	#endif 			
			default:
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "codec not support!!!!");
				VACVoiceData.DeleteMessage();
				return;
		}
		//�õ�L3Addr;
		L3addr = VGetU16BitVal( pVoiDataCommon->CallID );
		//�ҵ�CCB,׼����EIDGID��CID�ֶ�����
		pGrpCCB = NULL;
		pCCB = NULL;
		if(isGrpSrtpVData(curCodec))
		{
			pGrpCCB = pGrpCCBTbl->FindCCBByGrpL3addr(L3addr);
			if(NULL!=pGrpCCB)
			{
				VAC_EidGid = pGrpCCB->getGID();
				VAC_cid = GRP_TONE_FLAG_CID;
				pGrpCCB->airRes.setGrpDataDetectFlag(true);
			}
		}
		else
		{
			pCCB = (VoiceCCB*)CCBTable->FindCCBByL3addr(L3addr);
			if(NULL!=pCCB)
			{
				VAC_EidGid = pCCB->getVoiceTuple().Eid;
				VAC_cid = pCCB->getVoiceTuple().Cid;
			}
		}

		if(pCCB!=NULL && blSpyUidVData)
		{			
			if(pCCB->getUID() == g_SpyVData.getDiagUID())
			{
				if(isEncSrtpVData(curCodec))
				{
					//���������̶�40ms���
					g_SpyVData.uCntUID10msVDataFromSAG += 4;
				}
				else
				{
					g_SpyVData.uCntUID10msVDataFromSAG += nFrameNum;
				}
				if(pVoiDataCommon->blG729B)
				{
					++g_SpyVData.uCntUID10ms729BFromSAG;
				}
			}
		}

		if(pGrpCCB!=NULL && blSpyGidVData)
		{			
			if(pGrpCCB->getGID()== g_SpyVData.getDiagGID())
			{
				if(isEncSrtpVData(curCodec))
				{
					//���������̶�40ms���
					g_SpyVData.uCntGID10msVDataFromSAG += 4;
				}
				else
				{
					g_SpyVData.uCntGID10msVDataFromSAG += nFrameNum;
				}
				if(pVoiDataCommon->blG729B)
				{
					++g_SpyVData.uCntGID10ms729BFromSAG;
				}
			}
		}		

		bool blNoCCBFound = false;
		if(pCCB==NULL || pGrpCCB==NULL)
		{
			blNoCCBFound = true;
		}
		if(CODEC_G711A==pVoiDataCommon->Codec && pCCB!=NULL)
		{
			handleFaxDataFromSAG((UINT8*)pVoiDataCommon, pCCB);
			pCCB=NULL;
		}
		
#ifdef G729TONE_TEST
		if(pCCB!=NULL)
		{
			if(pCCB->getVoiceTuple().Eid == voiceTestEID)
			{
				//���������͸���������cpe��������
				pCCB = NULL;
			}
		}
		//20090914 fengbing add begin
		if(pGrpCCB!=NULL)
		{
			if(pGrpCCB->getGID()== voiceTestGID)
			{
				//���������͸������������������
				pGrpCCB = NULL;
			}
		}
		//20090914 fengbing add end
#endif
        
		if(pCCB==NULL && pGrpCCB==NULL)
		{
			if(blNoCCBFound)
			{
				LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "cannot find VoiceCCB/GrpCCB with L3Addr[0x%x]!!!!", L3addr);
				Counters.nVDRCCBNULL++;//wangwenhua add 20081222
			}
		}
		else
		{
#if 1
			bool blCpeZ = (NULL==pCCB) ? false : pCCB->isCPEZ();
			//��Ⱥ������ʹ��jitterbuffer,ֱ��͸��
			//����������ʹ��jitterbuffer,ֱ��͸��
			if( ( (g_blUseDownLinkJitterBuf && blCpeZ) || (g_blForceUseJbuf && pCCB!=NULL) ) && curCodec!=CODEC_ENCRYPT_G729A)
			{
				if(pCCB->isCPEZ() || g_blForceUseJbuf)
				{
					UINT16 idx = pCCB->m_vDataIdx;
					for(j=0;j<nFrameNum;j++)
					{
						//UINT8 nSN_in = pVoiDataCommon->SN+j;
						UINT8 nSN_in = pVoiDataCommon->timeStamp+j;		

						
						if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
						{
							if(g_SpyVData.isSpyDLUidVDataTS())
							{
								VPRINT("\n[%08X] nTS_in[%d] nTS_snd[%d] frmsInJBuf[%d] invalidTSCounter[%d] started[%d]", 
									g_SpyVData.	getFrameID(),
									nSN_in, vDatabuf[idx].downLinkBuf.curSndSN, 
									vDatabuf[idx].downLinkBuf.nRcvFrames,
									vDatabuf[idx].downLinkBuf.nInvalidSNCounter,
									vDatabuf[idx].downLinkBuf.blStarted);
							}
						}
						
						if(vDatabuf[idx].downLinkBuf.blStarted)
						{
							UINT8 nSN_out = vDatabuf[idx].downLinkBuf.curSndSN;

							bool blSNConvert = abs(nSN_in-nSN_out)>128;//SN�Ƿ�ת��
							//if vdata is efficient, store vdata in jitterbuffer
							if( (!blSNConvert && nSN_in>=nSN_out) ||
								(blSNConvert && nSN_in<nSN_out) )
							{
								vDatabuf[idx].downLinkBuf.nInvalidSNCounter=0;
								
								UINT8 jbidx = Mod2Power(nSN_in , JITTER_FRAMES);
								vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].len = nDataLen;
								vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].VACItemHead.Type = pVoiDataCommon->Codec;
								vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].VACItemHead.SN = nSN_in;
								memcpy((void*)vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].voiceData,
										(void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)+j*nDataLen),
										nDataLen);
								vDatabuf[idx].downLinkBuf.nRcvFrames++;

								if(vDatabuf[idx].downLinkBuf.nRcvFrames>=JITTER_FRAMES/2)
								{
									vDatabuf[idx].downLinkBuf.curSndSN++;
									//��ǰ����֡��������
									vDatabuf[idx].downLinkBuf.jitterBuffer[vDatabuf[idx].downLinkBuf.curSndIdx].len = M_INVALID_10MS_DATALEN;
									vDatabuf[idx].downLinkBuf.curSndIdx = Mod2Power( (vDatabuf[idx].downLinkBuf.curSndIdx+1) , JITTER_FRAMES);
									vDatabuf[idx].downLinkBuf.nRcvFrames--;
									if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
									{
										if(g_SpyVData.isSpyDLUidVDataTS())
										{
											VPRINT("\n move snd pointer forward, UID[0x%08x]\n", pCCB->getUID());
										}
									}
								}
								else 
								{
									//fengbing20100517��Ϊ10msʱ����׼ȷ�ģ��������ӳ٣��������������뵱�տڶ���ʱ��������ʱ
									#if 0
									if(vDatabuf[idx].downLinkBuf.nRcvFrames<2)
									{
										vDatabuf[idx].downLinkBuf.curSndSN--;
										//��ǰ����֡����������
										//vDatabuf[idx].downLinkBuf.jitterBuffer[vDatabuf[idx].downLinkBuf.curSndIdx].len = M_INVALID_10MS_DATALEN;
										vDatabuf[idx].downLinkBuf.curSndIdx = Mod2Power( (vDatabuf[idx].downLinkBuf.curSndIdx-1) , JITTER_FRAMES);
										vDatabuf[idx].downLinkBuf.nRcvFrames++;
										if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
										{
											if(g_SpyVData.isSpyDLUidVDataTS())
											{
												VPRINT("\n move snd pointer backward, UID[0x%08x]\n", pCCB->getUID());
											}
										}
									}
									#endif
								}
								
							}
							//else discard
							else
							{
								if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
								{
									if(g_SpyVData.isSpyDLUidVDataTS())
									{
										VPRINT("\nvdata is not efficient. nSN_in[%d] , nSN_out[%d], UID[0x%08x]\n", nSN_in, nSN_out, pCCB->getUID());
									}
								}								

								//�������10֡�����⣬������ѡ���ʹ���
								++vDatabuf[idx].downLinkBuf.nInvalidSNCounter;
								if(vDatabuf[idx].downLinkBuf.nInvalidSNCounter>10)
								{
									vDatabuf[idx].downLinkBuf.nInvalidSNCounter=0;
									vDatabuf[idx].downLinkBuf.blStarted = false;
									vDatabuf[idx].downLinkBuf.nRcvFrames = 0;

									if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
									{
										if(g_SpyVData.isSpyDLUidVDataTS())
										{
											VPRINT("\nmore than 10 contineous SN unwanted, Reset sndSN, UID[0x%08x]\n", pCCB->getUID());
										}
									}								
								}
	                            
							}						
						}
						else
						{
							vDatabuf[idx].downLinkBuf.nInvalidSNCounter=0;
							
							//store vdata in jitterbuffer
							UINT8 jbidx = Mod2Power( nSN_in , JITTER_FRAMES );
							vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].len = nDataLen;
							vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].VACItemHead.Type = pVoiDataCommon->Codec;
							vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].VACItemHead.SN = nSN_in;
							memcpy((void*)vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].voiceData,
									(void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)+j*nDataLen),
									nDataLen);

							//if nRcvFrames>4,start transfer
							vDatabuf[idx].downLinkBuf.nRcvFrames++;
							//if(vDatabuf[idx].downLinkBuf.nRcvFrames>4)
							if(vDatabuf[idx].downLinkBuf.nRcvFrames>g_Frames2Start)									
							{
								vDatabuf[idx].downLinkBuf.blStarted = true;
								//vDatabuf[idx].downLinkBuf.curSndSN = (nSN_in<g_Frames2Start) ? 252+nSN_in : nSN_in-g_Frames2Start;
								vDatabuf[idx].downLinkBuf.curSndSN = (nSN_in<g_Frames2Start) ? (256-g_Frames2Start)+nSN_in : nSN_in-g_Frames2Start;
								vDatabuf[idx].downLinkBuf.curSndIdx = Mod2Power( vDatabuf[idx].downLinkBuf.curSndSN , JITTER_FRAMES);

								if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
								{
									if(g_SpyVData.isSpyDLUidVDataTS())
									{
										#if 1
										VPRINT("\nvDatabuf[idx].downLinkBuf.nRcvFrames>%d, nSN_in[%d] , nSN_out[%d] UID[0x%08x]\n", g_Frames2Start, nSN_in , 
										    vDatabuf[idx].downLinkBuf.curSndSN, pCCB->getUID());
										#endif
									}
								}
							}
							//else
							else
							{
								if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
								{
									if(g_SpyVData.isSpyDLUidVDataTS())
									{
										#if 1
										VPRINT("\nvDatabuf[idx].downLinkBuf.nRcvFrames<=%d, nSN_in[%d] UID[0x%08x]\n", g_Frames2Start, nSN_in, pCCB->getUID());
										#endif
									}
								}
							}						
						}
					}

					//��G729B frame
					if(pVoiDataCommon->blG729B)
					{
						//UINT8 nSN_in = pVoiDataCommon->SN+j;
						UINT8 nSN_in = pVoiDataCommon->timeStamp+nFrameNum;		

						if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
						{
							if(g_SpyVData.isSpyDLUidVDataTS())
							{
								VPRINT("\n[%08X] nTS_in[%d] nTS_snd[%d] frmsInJBuf[%d] invalidTSCounter[%d] started[%d]", 
									g_SpyVData.	getFrameID(),
									nSN_in, vDatabuf[idx].downLinkBuf.curSndSN, 
									vDatabuf[idx].downLinkBuf.nRcvFrames,
									vDatabuf[idx].downLinkBuf.nInvalidSNCounter,
									vDatabuf[idx].downLinkBuf.blStarted);
							}
						}						
						
						if(vDatabuf[idx].downLinkBuf.blStarted)
						{
							UINT8 nSN_out = vDatabuf[idx].downLinkBuf.curSndSN;

							bool blSNConvert = abs(nSN_in-nSN_out)>128;//SN�Ƿ�ת��
							//if vdata is efficient, store vdata in jitterbuffer
							if( (!blSNConvert && nSN_in>=nSN_out) ||
								(blSNConvert && nSN_in<nSN_out) )
							{
								vDatabuf[idx].downLinkBuf.nInvalidSNCounter=0;
								
								UINT8 jbidx = Mod2Power(nSN_in , JITTER_FRAMES);
								vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].len = M_G729B_SRTP_DATALEN;//nDataLen;
								vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].VACItemHead.Type = CODEC_G729B_SID;//pVoiDataCommon->Codec;
								vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].VACItemHead.SN = nSN_in;
								memcpy((void*)vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].voiceData,
										(void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)+nFrameNum*nDataLen),
										M_G729B_SRTP_DATALEN);
								vDatabuf[idx].downLinkBuf.nRcvFrames++;

								if(vDatabuf[idx].downLinkBuf.nRcvFrames>=JITTER_FRAMES/2)
								{
									vDatabuf[idx].downLinkBuf.curSndSN++;
									//��ǰ����֡��������
									vDatabuf[idx].downLinkBuf.jitterBuffer[vDatabuf[idx].downLinkBuf.curSndIdx].len = M_INVALID_10MS_DATALEN;
									vDatabuf[idx].downLinkBuf.curSndIdx = Mod2Power( (vDatabuf[idx].downLinkBuf.curSndIdx+1) , JITTER_FRAMES);
									vDatabuf[idx].downLinkBuf.nRcvFrames--;

									if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
									{
										if(g_SpyVData.isSpyDLUidVDataTS())
										{
											VPRINT("\nG729B move snd pointer forward, UID[0x%08x]\n", pCCB->getUID());
										}
									}									
								}
								else 
								{
									//fengbing20100517��Ϊ10msʱ����׼ȷ�ģ��������ӳ٣��������������뵱�տڶ���ʱ��������ʱ
									#if 0
									if(vDatabuf[idx].downLinkBuf.nRcvFrames<2)
									{
										vDatabuf[idx].downLinkBuf.curSndSN--;
										//��ǰ����֡����������
										//vDatabuf[idx].downLinkBuf.jitterBuffer[vDatabuf[idx].downLinkBuf.curSndIdx].len = M_INVALID_10MS_DATALEN;
										vDatabuf[idx].downLinkBuf.curSndIdx = Mod2Power( (vDatabuf[idx].downLinkBuf.curSndIdx-1) , JITTER_FRAMES);
										vDatabuf[idx].downLinkBuf.nRcvFrames++;
										
										if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
										{
											if(g_SpyVData.isSpyDLUidVDataTS())
											{
												VPRINT("\nG729B move snd pointer backward, UID[0x%08x]\n", pCCB->getUID());
											}
										}
									}
									#endif
								}
								
							}
							//else discard
							else
							{
								if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
								{
									if(g_SpyVData.isSpyDLUidVDataTS())
									{
										VPRINT("\nG729B vdata is not efficient. nSN_in[%d] , nSN_out[%d], UID[0x%08x]\n", nSN_in, nSN_out, pCCB->getUID());
									}
								}

								//�������10֡�����⣬������ѡ���ʹ���
								++vDatabuf[idx].downLinkBuf.nInvalidSNCounter;
								if(vDatabuf[idx].downLinkBuf.nInvalidSNCounter>10)
								{
									vDatabuf[idx].downLinkBuf.nInvalidSNCounter=0;
									vDatabuf[idx].downLinkBuf.blStarted = false;
									vDatabuf[idx].downLinkBuf.nRcvFrames = 0;
									
									if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
									{
										if(g_SpyVData.isSpyDLUidVDataTS())
										{
											VPRINT("\nG729B more than 10 contineous SN unwanted, Reset sndSN, UID[0x%08x]\n", pCCB->getUID());
										}
									}
								}
	                            
							}						
						}
						else
						{
							vDatabuf[idx].downLinkBuf.nInvalidSNCounter=0;
							
							//store vdata in jitterbuffer
							UINT8 jbidx = Mod2Power( nSN_in , JITTER_FRAMES );
							vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].len = M_G729B_SRTP_DATALEN;//nDataLen;
							vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].VACItemHead.Type = CODEC_G729B_SID;//pVoiDataCommon->Codec;
							vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].VACItemHead.SN = nSN_in;
							memcpy((void*)vDatabuf[idx].downLinkBuf.jitterBuffer[jbidx].voiceData,
									(void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)+nFrameNum*nDataLen),
									M_G729B_SRTP_DATALEN);

							//if nRcvFrames>4,start transfer
							vDatabuf[idx].downLinkBuf.nRcvFrames++;
							//if(vDatabuf[idx].downLinkBuf.nRcvFrames>4)
							if(vDatabuf[idx].downLinkBuf.nRcvFrames>g_Frames2Start)								
							{
								vDatabuf[idx].downLinkBuf.blStarted = true;
								//vDatabuf[idx].downLinkBuf.curSndSN = (nSN_in<g_Frames2Start) ? 252+nSN_in : nSN_in-g_Frames2Start;
								vDatabuf[idx].downLinkBuf.curSndSN = (nSN_in<g_Frames2Start) ? (256-g_Frames2Start)+nSN_in : nSN_in-g_Frames2Start;
								vDatabuf[idx].downLinkBuf.curSndIdx = Mod2Power( vDatabuf[idx].downLinkBuf.curSndSN , JITTER_FRAMES);

								if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
								{
									if(g_SpyVData.isSpyDLUidVDataTS())
									{
										#if 1
										VPRINT("\nG729B vDatabuf[idx].downLinkBuf.nRcvFrames>%d, nSN_in[%d] , nSN_out[%d] UID[0x%08x]\n", g_Frames2Start, nSN_in , 
										    vDatabuf[idx].downLinkBuf.curSndSN, pCCB->getUID());
										#endif
									}
								}
							}
							//else
							else
							{
								if(blSpyUidVData && pCCB->getUID() == g_SpyVData.getDiagUID())
								{
									if(g_SpyVData.isSpyDLUidVDataTS())
									{
										#if 1
										VPRINT("\nG729B vDatabuf[idx].downLinkBuf.nRcvFrames<=%d, nSN_in[%d] UID[0x%08x]\n", g_Frames2Start, nSN_in, pCCB->getUID());
										#endif
									}
								}
							}						
						}
					}			
				}	
			}
			//ֱ��ת����L2VAC
			else
#endif			    
			{
				#if 0
				VPRINT("\n [SN : 0x%02x][ts : 0x%02x] [10msFrames : %d] [0x%08x,0x%02x] \n",
					pVoiDataCommon->SN, pVoiDataCommon->timeStamp,nFrameNum, 
					pCCB->getVoiceTuple().Eid, pCCB->getVoiceTuple().Cid);
				#endif

//#define INS_BAD_FRAME
#ifdef INS_BAD_FRAME
				if(pCCB!=NULL)
				{
					UINT16 idx = pCCB->m_vDataIdx;

					int nBadFrames = pVoiDataCommon->timeStamp - 
						vDatabuf[idx].downLinkBuf.nTimeStampNew;
					if(nBadFrames<0)
					{
						nBadFrames = 0-nBadFrames;
					}
					nBadFrames = (nBadFrames>4)? 4 : nBadFrames;
				#if 0	
					VPRINT("\nnBadFrames[%d]", nBadFrames);
				#endif
					//�ж��Ƿ���뻵֡
					if(vDatabuf[idx].downLinkBuf.nTimeStampNew!=0xffffffff &&
						nBadFrames )
					{
						//���뻵֡
						UINT8 nBadDataLen = (CODEC_G711A==curCodec) ? M_G711_10MS_DATALEN : M_G729_10MS_DATALEN;
						if(isEncSrtpVData(curCodec)
						{
							nBadDataLen = M_ENC_VDATA_PKT_LEN;
						}
						int k;
						for(k=0;k<nBadFrames;k++)
						{
							//����L3addr�õ�Eid,Cid
							pVACDataIE->header.Eid = htonl(pCCB->getVoiceTuple().Eid);
							pVACDataIE->header.Cid = pCCB->getVoiceTuple().Cid;
							//type
							pVACDataIE->header.Type = pVoiDataCommon->Codec;
							//�õ�SN;
							//pVACDataIE->header.SN = pVoiDataCommon->SN+j;
							pVACDataIE->header.SN = pVoiDataCommon->timeStamp-nBadFrames+k;
							
							//������������
							memset((void*)pVACDataIE->VoiceData, 0, 
									nBadDataLen);
							//�������
							cur_VACData_ptr += ( sizeof(VACVoiceDataHeaderT) + nBadDataLen );
							pVACDataIE = (VACVoiceDataIE_T*)cur_VACData_ptr;
							
							//������������1
							++VACBuf[0];
							VACMsgLen += (sizeof(VACVoiceDataHeaderT)+nBadDataLen);
							
						}
						Counters.nDownLinkVdataBadSent +=nBadFrames;
					}
					//������֡��ʱ��
					if(!isEncSrtpVData(curCodec))
					{
						vDatabuf[idx].downLinkBuf.nTimeStampNew = 
							pVoiDataCommon->timeStamp + nFrameNum;
					}
					else
					{
						//���������̶�40ms���ʱ��
						vDatabuf[idx].downLinkBuf.nTimeStampNew = 
							pVoiDataCommon->timeStamp + 4;
					}
					if(pVoiDataCommon->blG729B)
						++vDatabuf[idx].downLinkBuf.nTimeStampNew;
					vDatabuf[idx].downLinkBuf.nTimeStampNew &= 0xFF;
					#if 0
					VPRINT("\nNewTimeStame Wanted[0x%02x]", vDatabuf[idx].downLinkBuf.nTimeStampNew);
					#endif
				}
#endif	//INS_BAD_FRAME

				for(j=0;j<nFrameNum;j++)
				{
					//����L3addr�õ�Eid,Cid
					VSetU32BitVal(pVACDataIE->header.Eid , VAC_EidGid);//htonl(pCCB->getVoiceTuple().Eid);
					pVACDataIE->header.Cid = VAC_cid;//pCCB->getVoiceTuple().Cid;
					//type
					pVACDataIE->header.Type = pVoiDataCommon->Codec;
					if(CODEC_GRPTONE==pVoiDataCommon->Codec)	//��Ⱥ�������뷽ʽΪG.729A
					{
						pVACDataIE->header.Type = CODEC_G729A;
					}
					else
					{
						if(CODEC_ENCRYPT_GRPTONE==pVoiDataCommon->Codec)
						{
							pVACDataIE->header.Type = CODEC_ENCRYPT_G729A;
						}
					}
					//�õ�SN;
					//pVACDataIE->header.SN = pVoiDataCommon->SN+j;
					pVACDataIE->header.SN = pVoiDataCommon->timeStamp+j;
					
					//������������
					memcpy((void*)pVACDataIE->VoiceData, 
							(void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)+j*nDataLen), 
							nDataLen);
					//�������
					cur_VACData_ptr += ( sizeof(VACVoiceDataHeaderT) + nDataLen );
					pVACDataIE = (VACVoiceDataIE_T*)cur_VACData_ptr;
					
					//������������1
					++VACBuf[0];
					VACMsgLen += (sizeof(VACVoiceDataHeaderT)+nDataLen);
					if(isEncSrtpVData(curCodec))
					{
						Counters.n10msPktToVAC += 3;//��������,201103191238 by fb
						//��������Ϊ10bytes������Ϣ��40msG.729A����֡�����һ�δ������
						break;
					}
				}
				//��G729B frame
				if(pVoiDataCommon->blG729B)
				{
					//����L3addr�õ�Eid,Cid
					VSetU32BitVal(pVACDataIE->header.Eid , VAC_EidGid);//htonl(pCCB->getVoiceTuple().Eid);
					pVACDataIE->header.Cid = VAC_cid;//pCCB->getVoiceTuple().Cid;
					//type
					pVACDataIE->header.Type = CODEC_G729B_SID;
					//�õ�SN;
					//pVACDataIE->header.SN = pVoiDataCommon->SN+j;
					pVACDataIE->header.SN = pVoiDataCommon->timeStamp+nFrameNum;
					
					//������������
					memcpy((void*)pVACDataIE->VoiceData, 
							(void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)+nFrameNum*nDataLen), 
							M_G729B_SRTP_DATALEN);
					//�������
					cur_VACData_ptr += ( sizeof(VACVoiceDataHeaderT) + M_G729B_SRTP_DATALEN );
					pVACDataIE = (VACVoiceDataIE_T*)cur_VACData_ptr;
					
					//������������1
					++VACBuf[0];
					VACMsgLen += (sizeof(VACVoiceDataHeaderT)+M_G729B_SRTP_DATALEN);

					++Counters.nG729BPktToVAC;
					
				}

				if(pCCB!=NULL && blSpyUidVData)
				{			
					if(pCCB->getUID() == g_SpyVData.getDiagUID())
					{
						if(isEncSrtpVData(curCodec))
						{
							//��������40ms���
							g_SpyVData.uCntUID10msVDataToVAC += 4;
						}
						else
						{
							g_SpyVData.uCntUID10msVDataToVAC += nFrameNum;
						}
						if(g_SpyVData.isSpyDLUidVDataTS() && nFrameNum>0)
						{
							VPRINT("\n[%08X] G.729A nTS_in[%d---%d] ", 
								g_SpyVData.	getFrameID(),
								pVoiDataCommon->timeStamp,
								pVoiDataCommon->timeStamp+nFrameNum-1);
						}
						if(pVoiDataCommon->blG729B)
						{
							++g_SpyVData.uCntUID10ms729BToVAC;
							if(g_SpyVData.isSpyDLUidVDataTS())
							{
								VPRINT("\n[%08X] G.729B nTS_in[%d] ", 
									g_SpyVData.	getFrameID(),
									pVoiDataCommon->timeStamp+nFrameNum);								
							}
						}
					}
				}
				
				if(pGrpCCB!=NULL && blSpyGidVData)
				{			
					if(pGrpCCB->getGID() == g_SpyVData.getDiagGID())
					{
						if(isEncSrtpVData(curCodec))
						{
							//��������40ms���
							g_SpyVData.uCntGID10msVDataToVAC += 4;
						}
						else
						{
							g_SpyVData.uCntGID10msVDataToVAC += nFrameNum;
						}
						if(g_SpyVData.isSpyGidVDataTS() && nFrameNum>0)
						{
							VPRINT("\n[%08X] G.729A GrpTone nTS[%d---%d] ", 
								g_SpyVData.	getFrameID(),
								pVoiDataCommon->timeStamp,
								pVoiDataCommon->timeStamp+nFrameNum-1);
						}
						if(pVoiDataCommon->blG729B)
						{
							++g_SpyVData.uCntGID10ms729BToVAC;
							if(g_SpyVData.isSpyGidVDataTS())
							{
								VPRINT("\n[%08X] G.729B GrpTone nTS[%d] ", 
									g_SpyVData.	getFrameID(),
									pVoiDataCommon->timeStamp+nFrameNum);								
							}						
						}
					}
				}


			}
			
		}

		if(isEncSrtpVData(curCodec))
		{
			nTmpVDataLen = nDataLen;
		}
		else
		{
			nTmpVDataLen = nFrameNum*nDataLen;
		}
		//ָ����һ����������
		pVoiDataCommon = (DMUXVoiDataCommonT*)((UINT8*)pVoiDataCommon + 
												sizeof(DMUXVoiDataCommonT) + 
												nTmpVDataLen +
												M_G729B_SRTP_DATALEN*pVoiDataCommon->blG729B);	
		
	}

	if(VACBuf[0]>0)		//�����������
	{
		VACVoiceData.SetPayloadLen(VACMsgLen);	//��Ϣ����
		if(VACVoiceData.Post())
		{
			Counters.nVoiDataToVAC++;
			Counters.n10msPktToVAC += VACBuf[0];
		}
/*		
		VPRINT("\nVAC pkt:");
		for(i=0;i<VACMsgLen;i++)
		{
			VPRINT(" %02X", VACBuf[i]);
		}
		VPRINT("\n");
*/
	}
	else				//���������Ϣ
	{
		VACVoiceData.DeleteMessage();
	}	


}

/*
 *	VAC-->UDP,�����VAC�յ����������ݰ�����VAC�ӿڸ�ʽת��Ϊsrtp��ʽ����UDP���͸�SAG
 *	���ʱ��Ϊ20ms,ÿ10ms��VAC�յ�һ�ΰ�
 */
void VoiceFSM::handleVoiceDataFromVAC(CMessage& msg)
{
	//LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVoiceDataFromVAC");
	bool blSpyUidVData = g_SpyVData.isSpyUidVoiceData();

//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
	//bts�ڲ�����������ʹ�ñ��������������msg��������Խ����ڴ���ʳ�ͻ
	UINT8 *VACBuf_IS = (UINT8*)msg.GetDataPtr();
	UINT16 CN_IS = 0;
	UINT16 cnt10ms_IS = 0;
	UINT8 *cur_VACData_ptr_IS = VACBuf_IS+1;
	UINT16 VACMsgLen_IS = 1;
	VACVoiceDataIE_T* pVACDataIE_IS;
#endif
//20090531 fengbing bts inner switch for Voice Data end
		
	Counters.nVoiDataFromVAC++;
	CMsg_VACVoiceData VACVoiceData(msg);
	if(1300<msg.GetDataLength())
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "Message Length>1300!!!");
		//return;
	}

	UINT8	*VACBuf = (UINT8*)VACVoiceData.GetDataPtr();	//�����VAC�������ݰ��Ļ�����
	UINT8	*cur_VACData_ptr;
	cur_VACData_ptr = VACBuf+1+1;					//ָ��VAC������Ϣ�е�һ����������
	DMUXHeadT* pDMUXHead;
	DMUXVoiDataCommonT *pVoiDataCommon;
	CComMessage *pComMsg = NULL;
	UINT8	*pUdp;						//UDP��������ָ��
	UINT16	UdpPktlen;					//UDP������
	int	i, CN;
	UINT16 n10msPktNum;
/*
	VPRINT("\nfrom VAC:");
	for(i=0;i<msg.GetDataLength();i++)
	{
		VPRINT(" %02X", VACBuf[i]);
	}
	VPRINT("\n");
*/

	//�γ�UDP������
	pComMsg = new (CTVoice::GetInstance(), M_MAX_SRTP_PKT_LEN+1) CComMessage;
	if(pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "Cannot new CComMessage!!!!!!");
		return;
	}
	else
	{
		n10msPktNum = 0;
		
		pUdp = (UINT8*)pComMsg->GetDataPtr();
		pDMUXHead = (DMUXHeadT*)pUdp;
		pDMUXHead->FormID = 0;
		pDMUXHead->Prio = M_DMUX_VOICEDATA_PRIORITY;
		pDMUXHead->nFrameNum = 0;
		pDMUXHead->Reserved1 = 0;
		//pDMUXHead->srtpTimeStamp = htonl(g_VFrameNum);
		VSetU32BitVal(pDMUXHead->Reserved2 , 0);
		//UDP packet length
		UdpPktlen = sizeof(DMUXHeadT);
		pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen);	//point to the first frame
	}

	//����ÿ��VAC���ݰ�����Ϣ
	CN = VACBuf[1];
	Counters.n10msPktFromVAC += CN;
	for(i=0;i<CN;i++)
	{
		VACVoiceDataIE_T* pVACDataIE = (VACVoiceDataIE_T*)cur_VACData_ptr;
		UINT8 dataLen = 0;
		UINT16 ntmp = 30;
		switch(pVACDataIE->header.Type)
		{
			case CODEC_G729A:
				dataLen = M_G729_10MS_DATALEN;
				break;
			case CODEC_ENCRYPT_G729A:
				//��������40ms�������������
				Counters.n10msPktFromVAC += 3;
				dataLen = M_ENC_VDATA_PKT_LEN;
				ntmp = 64;
				break;
			case CODEC_G711A:
				dataLen = M_G711_10MS_DATALEN;
				ntmp = 188;
				break;
			case CODEC_G729B_SID:
				dataLen = 2;
				break;
			case CODEC_G729B_UNT:
				dataLen = 0;
				break;
			case CODEC_G729D:
				dataLen = 8;
				break;
			default:
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "Codec not support!!!!");
				pComMsg->Destroy();
				return;
		}


		//���srtp������ȿ쳬���ˣ��򷢳��˰������¹����¸�srtp��
		if((UdpPktlen+ntmp)>M_MAX_SRTP_PKT_LEN)
		{
			//��д��ǰ���ݰ�����
			pComMsg->SetDataLength(UdpPktlen);
			if(pDMUXHead->nFrameNum>0)
			{
				if(SendVoiceDataToSAG(pComMsg))
				{
					Counters.nVoiDataToVDR++;
					Counters.n10msPktToVDR += n10msPktNum;
				}
			}
			else
			{
				pComMsg->Destroy();
				return;
			}

			//�γ�UDP������
			pComMsg = new (CTVoice::GetInstance(), M_MAX_SRTP_PKT_LEN+1) CComMessage;
			if(pComMsg==NULL)
			{
				LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "Cannot new CComMessage!!!!!!");
				return;
			}
			else
			{
				n10msPktNum = 0;
				
				pUdp = (UINT8*)pComMsg->GetDataPtr();
				pDMUXHead = (DMUXHeadT*)pUdp;
				pDMUXHead->FormID = 0;
				pDMUXHead->Prio = M_DMUX_VOICEDATA_PRIORITY;
				pDMUXHead->nFrameNum = 0;
				pDMUXHead->Reserved1 = 0;
				//pDMUXHead->srtpTimeStamp = htonl(g_VFrameNum);
				VSetU32BitVal(pDMUXHead->Reserved2 , 0);
				//UDP packet length
				UdpPktlen = sizeof(DMUXHeadT);
				pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen);	//point to the first frame
			}
			
		}


		//����Eid,Cid�ҵ�CCB;
		VoiceTuple tup;
		tup.Eid = VGetU32BitVal(pVACDataIE->header.Eid);
		tup.Cid = pVACDataIE->header.Cid;
		VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tup);
		if(pCCB!=NULL)
		{
			if(blSpyUidVData)
			{
				if(g_SpyVData.getDiagUID()==pCCB->getUID())
				{
					if(pVACDataIE->header.Type==CODEC_G729B_SID)
					{
						++g_SpyVData.uCntUID10ms729BFromVAC;
					}
					else
					{
						if(CODEC_ENCRYPT_G729A==pVACDataIE->header.Type)
						{
							//��������40ms���������������������Ϣ֡
							g_SpyVData.uCntUID10msVDataFromVAC+=4;
						}
						else
						{
							++g_SpyVData.uCntUID10msVDataFromVAC;
						}
					}
				}
			}
//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
			VoiceCCB *pPeerCCB = pCCB->getPeerCCB();
			if(pCCB->isBtsInnerSwitchVData() && pPeerCCB!=NULL)
			{
				//�ڲ�����������VAC
				memcpy(cur_VACData_ptr_IS, cur_VACData_ptr, sizeof(VACVoiceDataHeaderT)+dataLen);
				pVACDataIE_IS = (VACVoiceDataIE_T*)(cur_VACData_ptr_IS);
				VSetU32BitVal(pVACDataIE_IS->header.Eid , pPeerCCB->getVoiceTuple().Eid);
				pVACDataIE_IS->header.Cid = pPeerCCB->getVoiceTuple().Cid;

				if(blSpyUidVData)
				{
					if(g_SpyVData.getDiagUID()==pPeerCCB->getUID())
					{
						if(pVACDataIE_IS->header.Type==CODEC_G729B_SID)
						{
							++g_SpyVData.uCntUID10ms729BToVAC;
						}
						else
						{
							if(CODEC_ENCRYPT_G729A==pVACDataIE_IS->header.Type)
							{
								//��������40ms���������������������Ϣ֡
								g_SpyVData.uCntUID10msVDataToVAC+=4;
							}
							else
							{
								++g_SpyVData.uCntUID10msVDataToVAC;
							}
						}
					}
				}

				++CN_IS;
				if(CODEC_ENCRYPT_G729A==pVACDataIE_IS->header.Type)
				{
					cnt10ms_IS+=4;
				}
				else
				{
					++cnt10ms_IS;
				}
				cur_VACData_ptr_IS += (sizeof(VACVoiceDataHeaderT)+dataLen);
				VACMsgLen_IS += (sizeof(VACVoiceDataHeaderT)+dataLen);
			}
			else
#endif
//20090531 fengbing bts inner switch for Voice Data end
			{
				//���ڲ�����������tVDR����
				UINT16 idx = pCCB->m_vDataIdx;
				if(idx<M_MAX_CALLS)
				{
					if(CODEC_ENCRYPT_G729A==pVACDataIE->header.Type)
					{
						//����������40ms�����ʽ
						//CallID,Codec,frameNum,blG729b,SN
						memcpy(pVoiDataCommon->CallID , vDatabuf[idx].upLinkBuf.srtpItmeHead.CallID, 2);
						pVoiDataCommon->Codec = pVACDataIE->header.Type;//codec
						pVoiDataCommon->frameNum = 1;//10byte������Ϣ֡+40msG.729A����֡
						pVoiDataCommon->blG729B = false;
						
						pVoiDataCommon->SN = vDatabuf[idx].upLinkBuf.srtpItmeHead.SN++; //SN 0-255
						if(pCCB->isCPEZ())	//cpeZ
						{
							//40ms frame
							vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp += 4;
							pVoiDataCommon->timeStamp = vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;
						}
						else	//cpe
						{
							//40ms frame
							pVoiDataCommon->timeStamp = 
							vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp = 
							pVACDataIE->header.SN;
						}
						
						//copy voice data
						memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
								(void*)pVACDataIE->VoiceData,
								dataLen);
						
						n10msPktNum +=4;
						++pDMUXHead->nFrameNum; //�û�֡��
						UdpPktlen += (sizeof(DMUXVoiDataCommonT) + dataLen);//Udp Packet length
						pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen); //point to next frame
					
						if(blSpyUidVData)
						{
							if(g_SpyVData.getDiagUID()==pCCB->getUID())
							{
								g_SpyVData.uCntUID10msVDataToSAG+=4;
							}
						}						
					}
					else//if(CODEC_ENCRYPT_G729A==pVACDataIE->header.Type)
					{
						if(g_blUse10msULSrtp)
						{
							
							//����10ms������͸�sag,������ͨcpe��sn==timeStamp��
							//����cpeZ��ʱ����sn��bts���ɣ����޷���֤����
							
							//if is g729B,send it
							if(pVACDataIE->header.Type==CODEC_G729B_SID)
							{
								//CallID,Codec,frameNum,blG729b,SN
								memcpy(pVoiDataCommon->CallID , vDatabuf[idx].upLinkBuf.srtpItmeHead.CallID, 2);
								//pVoiDataCommon->Codec = pVACDataIE->header.Type;//codec
								pVoiDataCommon->Codec = CODEC_G729A;//codec
								pVoiDataCommon->frameNum = 0;
								pVoiDataCommon->blG729B = true;
							
								//pVoiDataCommon->SN = vDatabuf[idx].upLinkBuf.srtpItmeHead.SN++;	//SN 0-255
								if(pCCB->isCPEZ())	//cpeZ
								{
									pVoiDataCommon->timeStamp = ++vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;//10ms frame
								}
								else	//cpe
								{
									//10ms frame
									pVoiDataCommon->timeStamp = 
									vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp = 
									pVACDataIE->header.SN;
								}
								pVoiDataCommon->SN = pVoiDataCommon->timeStamp;
							
								//copy voice data
								memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
										(void*)pVACDataIE->VoiceData,
										dataLen);
							
								n10msPktNum +=1;
								++pDMUXHead->nFrameNum; //�û�֡��
								UdpPktlen += (sizeof(DMUXVoiDataCommonT) + dataLen);//Udp Packet length
								pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen); //point to next frame
							
								++Counters.nG729BPkt;
							
								if(blSpyUidVData)
								{
									if(g_SpyVData.getDiagUID()==pCCB->getUID())
									{
										++g_SpyVData.uCntUID10ms729BToSAG;
									}
								}
								
							}
							else
							{
								//CallID,Codec,frameNum,blG729b,SN
								memcpy(pVoiDataCommon->CallID , vDatabuf[idx].upLinkBuf.srtpItmeHead.CallID, 2);
								pVoiDataCommon->Codec = pVACDataIE->header.Type;//codec
								pVoiDataCommon->frameNum = 1;
								pVoiDataCommon->blG729B = false;
								
								//pVoiDataCommon->SN = vDatabuf[idx].upLinkBuf.srtpItmeHead.SN++; //SN 0-255
								if(pCCB->isCPEZ())	//cpeZ
								{
									pVoiDataCommon->timeStamp = ++vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;//10ms frame
								}
								else	//cpe
								{
									//10ms frame
									pVoiDataCommon->timeStamp = 
									vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp = 
									pVACDataIE->header.SN;
								}
								pVoiDataCommon->SN = pVoiDataCommon->timeStamp;
								
								//copy voice data
								memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
										(void*)pVACDataIE->VoiceData,
										dataLen);
								
								n10msPktNum +=1;
								++pDMUXHead->nFrameNum; //�û�֡��
								UdpPktlen += (sizeof(DMUXVoiDataCommonT) + dataLen);//Udp Packet length
								pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen); //point to next frame
							
								if(blSpyUidVData)
								{
									if(g_SpyVData.getDiagUID()==pCCB->getUID())
									{
										++g_SpyVData.uCntUID10msVDataToSAG;
									}
								}
							
							}
						}
						else	//if(g_blUse10msULSrtp)
						{
		//------------------
#if 1				
							//if uplink buffer is empty
							if(0==vDatabuf[idx].upLinkBuf.curBufId)
							{
								//if is g729B,send it
								if(pVACDataIE->header.Type==CODEC_G729B_SID)
								{
									//CallID,Codec,frameNum,blG729b,SN
									memcpy(pVoiDataCommon->CallID , vDatabuf[idx].upLinkBuf.srtpItmeHead.CallID, 2);
									//pVoiDataCommon->Codec = pVACDataIE->header.Type;//codec
									pVoiDataCommon->Codec = CODEC_G729A;//codec
									pVoiDataCommon->frameNum = 0;
									pVoiDataCommon->blG729B = true;

									pVoiDataCommon->SN = vDatabuf[idx].upLinkBuf.srtpItmeHead.SN++;	//SN 0-255
									if(pCCB->isCPEZ())	//cpeZ
									{
										pVoiDataCommon->timeStamp = ++vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;//10ms frame
									}
									else	//cpe
									{
										//10ms frame
										pVoiDataCommon->timeStamp = 
										vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp = 
										pVACDataIE->header.SN;
									}

									//copy voice data
									memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
											(void*)pVACDataIE->VoiceData,
											dataLen);

									n10msPktNum +=1;
									++pDMUXHead->nFrameNum;	//�û�֡��
									UdpPktlen += (sizeof(DMUXVoiDataCommonT) + dataLen);//Udp Packet length
									pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen);	//point to next frame

									++Counters.nG729BPkt;

									if(blSpyUidVData)
									{
										if(g_SpyVData.getDiagUID()==pCCB->getUID())
										{
											++g_SpyVData.uCntUID10ms729BToSAG;
										}
									}
														
								}
								//else save it, only save when timestame is even number ,else send it
								else
								{
									if(!(pVACDataIE->header.SN & 1))	//even number, save it
									{
										vDatabuf[idx].upLinkBuf.len = dataLen;
										vDatabuf[idx].upLinkBuf.srtpItmeHead.Codec = pVACDataIE->header.Type;
										vDatabuf[idx].upLinkBuf.srtpItmeHead.blG729B = 0;
										if(!pCCB->isCPEZ())//cpe
										{
											vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp = 
												pVACDataIE->header.SN;
										}
										//copy voice data
										memcpy((void*)vDatabuf[idx].upLinkBuf.voiceData,
												(void*)pVACDataIE->VoiceData,
												dataLen);
										vDatabuf[idx].upLinkBuf.curBufId = 1;//mark uplinkbuffer not empty
									}
									else	//send it
									{
										//CallID,Codec,frameNum,blG729b,SN
										memcpy(pVoiDataCommon->CallID , vDatabuf[idx].upLinkBuf.srtpItmeHead.CallID, 2);
										pVoiDataCommon->Codec = pVACDataIE->header.Type;//codec
										pVoiDataCommon->frameNum = 1;
										pVoiDataCommon->blG729B = false;
										
										pVoiDataCommon->SN = vDatabuf[idx].upLinkBuf.srtpItmeHead.SN++; //SN 0-255
										if(pCCB->isCPEZ())	//cpeZ
										{
											pVoiDataCommon->timeStamp = ++vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;//10ms frame
										}
										else	//cpe
										{
											//10ms frame
											pVoiDataCommon->timeStamp = 
											vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp = 
											pVACDataIE->header.SN;
										}
										
										//copy voice data
										memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
												(void*)pVACDataIE->VoiceData,
												dataLen);
										
										n10msPktNum +=1;
										++pDMUXHead->nFrameNum; //�û�֡��
										UdpPktlen += (sizeof(DMUXVoiDataCommonT) + dataLen);//Udp Packet length
										pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen); //point to next frame

										if(blSpyUidVData)
										{
											if(g_SpyVData.getDiagUID()==pCCB->getUID())
											{
												++g_SpyVData.uCntUID10msVDataToSAG;
											}
										}

									}
								}
							}
							//if uplink buffer is not empty,
							else		
							{
								int len = pVACDataIE->header.SN - vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;
								bool blConsecutive = (len==1 || len==-255);

								if(!pCCB->isCPEZ())//cpe
								{
									if(!blConsecutive)
										++Counters.nUpLinkVdataLost;
								}
								else		//cpeZ,�޷��ж϶�������Ϊ����������
									
									blConsecutive = true;
									
								
								//���뷽ʽδ�����仯����ʱ���������ϳ�һ���ͳ�
								if((vDatabuf[idx].upLinkBuf.srtpItmeHead.Codec==pVACDataIE->header.Type)
									&& blConsecutive)
								{
									//CallID,Codec,frameNum,blG729b,SN
									memcpy(pVoiDataCommon->CallID , vDatabuf[idx].upLinkBuf.srtpItmeHead.CallID, 2);
									pVoiDataCommon->Codec = pVACDataIE->header.Type;//codec
									pVoiDataCommon->frameNum = 2;
									pVoiDataCommon->blG729B = 0;
									pVoiDataCommon->SN = vDatabuf[idx].upLinkBuf.srtpItmeHead.SN++;	//SN 0-255
									if(pCCB->isCPEZ())
									{
										vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp += 2;
										pVoiDataCommon->timeStamp = vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;//20ms frame
									}
									else
									{
										pVoiDataCommon->timeStamp = pVACDataIE->header.SN-1;//20ms frame
										vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp = pVACDataIE->header.SN;//10ms frame
									}
									//copy voice data
									memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
											(void*)vDatabuf[idx].upLinkBuf.voiceData,
											dataLen);
									memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)+dataLen),
											(void*)pVACDataIE->VoiceData,
											dataLen);

									++pDMUXHead->nFrameNum;	//�û�֡��
									UdpPktlen += (sizeof(DMUXVoiDataCommonT) + dataLen+dataLen);//Udp Packet length
									pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen);	//point to next frame
									
								}
								//���뷽ʽ�����仯����ʱ�����������������ͳ�
								else
								{
								//��һ��,�϶�����729B
									//CallID,Codec,frameNum,blG729b,SN
									memcpy(pVoiDataCommon->CallID , vDatabuf[idx].upLinkBuf.srtpItmeHead.CallID, 2);
									pVoiDataCommon->Codec = vDatabuf[idx].upLinkBuf.srtpItmeHead.Codec;//codec
									pVoiDataCommon->frameNum = 1;
									pVoiDataCommon->blG729B = 0;
									pVoiDataCommon->SN = vDatabuf[idx].upLinkBuf.srtpItmeHead.SN++;	//SN 0-255
									if(pCCB->isCPEZ())
										pVoiDataCommon->timeStamp = ++vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;//10ms frame
									else
										pVoiDataCommon->timeStamp = vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;//10ms frame
									//copy voice data
									memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
											(void*)vDatabuf[idx].upLinkBuf.voiceData,
											vDatabuf[idx].upLinkBuf.len);

									++pDMUXHead->nFrameNum;	//�û�֡��
									UdpPktlen += (sizeof(DMUXVoiDataCommonT) + dataLen);//Udp Packet length
									pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen);	//point to next frame

								//�ڶ���
									//CallID,Codec,frameNum,blG729b,SN
									memcpy(pVoiDataCommon->CallID , vDatabuf[idx].upLinkBuf.srtpItmeHead.CallID, 2);
									if(pVACDataIE->header.Type==CODEC_G729B_SID)
									{
										pVoiDataCommon->Codec = CODEC_G729A;
										pVoiDataCommon->frameNum = 0;
										pVoiDataCommon->blG729B = 1;

										++Counters.nG729BPkt;
									}
									else
									{
										pVoiDataCommon->Codec = pVACDataIE->header.Type;//codec
										pVoiDataCommon->frameNum = 1;
										pVoiDataCommon->blG729B = 0;
									}
									
									pVoiDataCommon->SN = vDatabuf[idx].upLinkBuf.srtpItmeHead.SN++;	//SN 0-255
									if(pCCB->isCPEZ())
										pVoiDataCommon->timeStamp = ++vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp;//10ms frame
									else
									{
										pVoiDataCommon->timeStamp = 
										vDatabuf[idx].upLinkBuf.srtpItmeHead.timeStamp = 
										pVACDataIE->header.SN;//10ms frame
									}
									//copy voice data
									memcpy((void*)((UINT8*)pVoiDataCommon+sizeof(DMUXVoiDataCommonT)),
											(void*)pVACDataIE->VoiceData,
											dataLen);

									++pDMUXHead->nFrameNum;	//�û�֡��
									UdpPktlen += (sizeof(DMUXVoiDataCommonT) + dataLen);//Udp Packet length
									pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+UdpPktlen);	//point to next frame		

								}

								n10msPktNum +=2;
								vDatabuf[idx].upLinkBuf.curBufId = 0;	//mark uplinkbuffer empty

								if(blSpyUidVData)
								{
									if(g_SpyVData.getDiagUID()==pCCB->getUID())
									{
										g_SpyVData.uCntUID10msVDataToSAG+=2;
									}
								}
								
							}


#endif
		//------------------

						}//else	//if(g_blUse10msULSrtp)
					}//if(CODEC_ENCRYPT_G729A==pVACDataIE->header.Type)					
					vDatabuf[idx].upLinkBuf.timeStampLen = 0;
				}
				else
				{
					LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"rcv vdata from VAC when vBuf[0x%X] release......", idx);
				}
			}
		}
		else
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Cannot find CCB!");
		}
		//VAC������һ���������ݰ�
		cur_VACData_ptr = pVACDataIE->VoiceData + dataLen;

		

		
	}

//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
	if(CN_IS>0)			//�����������
	{
		VACBuf_IS[0] = CN_IS;	//����������		
		VACVoiceData.SetPayloadLen(VACMsgLen_IS);	//��Ϣ����
		VACVoiceData.SetMessageId(MSGID_VOICE_VAC_DATA);
		VACVoiceData.SetSrcTid(M_TID_VOICE);
		VACVoiceData.SetDstTid(M_TID_VAC);
		if(VACVoiceData.Post())
		{
			Counters.nVoiDataToVAC++;
			Counters.n10msPktToVAC += cnt10ms_IS;
		}
/*		
		VPRINT("\nVAC pkt:");
		for(i=0;i<VACMsgLen;i++)
		{
			VPRINT(" %02X", VACBuf[i]);
		}
		VPRINT("\n");
*/
	}
	else				//����do nothing
	{
	}	
#endif
//20090531 fengbing bts inner switch for Voice Data end

	//��д��ǰ���ݰ�����
	pComMsg->SetDataLength(UdpPktlen);
	if(pDMUXHead->nFrameNum>0)
	{
		if(SendVoiceDataToSAG(pComMsg))
		{
			Counters.nVoiDataToVDR++;
			Counters.n10msPktToVDR += n10msPktNum;
		}
	}
	else
	{
		pComMsg->Destroy();
		return;
	}

/*
	VPRINT("\nto SAG:");
	for(i=0;i<UdpPktlen;i++)
	{
		VPRINT(" %02X", pUdp[i]);
	}
	VPRINT("\n");
*/	
}


/*
 *	sending voice data to SAG
 */
bool VoiceFSM::SendVoiceDataToSAG(CComMessage* pComMsg)
{
	if(pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::SendVoiceDataToSAG(), pComMsg==NULL, error!!!!");
		return false;
	}
	pComMsg->SetDstTid(M_TID_VDR);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_VOICE_VDR_DATA);
	return postComMsg(pComMsg);
}
/*
 *	�����յ���LAPagingReq,�������UID�����ҵ�CCB,��Ϣע��״̬��
 */
void VoiceFSM::handleLAPagingReq(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleLAPagingReq");
	VoiceVCRCtrlMsgT* pDataPaging = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT32 uid = VGetU32BitVal(pDataPaging->sigPayload.LAPaging.UID);
	UINT16 appType = VGetU16BitVal(pDataPaging->sigPayload.LAPaging.App_Type);
	if(APPTYPE_OAM_FORCE_REGISTER==appType)
	{
		UINT32 Eid = l3oamGetEIDByUID(uid);
		if(0xffffffff!=Eid)
		{
			notifyOneCpeToRegister(Eid, false);
		}
		return;
	}	
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByUID(uid);
	if(pCCB!=NULL)
	{
		if(APPTYPE_VOICE_QCELP==appType || 
			APPTYPE_ENCRYPT_VOICE==appType ||
			APPTYPE_ENC_SIGNAL_SEND==appType ||
			APPTYPE_GRP_MANAGE==appType ||
			APPTYPE_SMS==appType ||
			APPTYPE_LOWSPEED_DATA==appType ||
			APPTYPE_DUPCPE_PAGING==appType)	//������������I�������Ϣ
		{
			InjectMsg(msg);
		}
		if(APPTYPE_CMD_REG==appType)	//ָ��Ǽ�ע��Ҫ��
		{
			CMsg_UTSAGSignal_DAC MsgToCpe_DAC;
			//������Ϣ
			if ( !MsgToCpe_DAC.CreateMessage(*CTVoice::GetInstance(), 2) )	
			{
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
				return;
			}
			else
			{
				//��Ϣ����CID+MSGTYPE
				MsgToCpe_DAC.SetMessageId(MSGID_VOICE_DAC_SIGNAL);
				MsgToCpe_DAC.SetDstTid(M_TID_UTV);
				MsgToCpe_DAC.SetEID(pCCB->getVoiceTuple().Eid);
				MsgToCpe_DAC.SetCID(pCCB->getVoiceTuple().Cid);

				UINT8* pData = (UINT8*)MsgToCpe_DAC.GetDataPtr();
				pData[0] = pCCB->getVoiceTuple().Cid;
				pData[1] = M_MSGTYPE_LOGINREQ;

				MsgToCpe_DAC.SetPayloadLength(2);
				MsgToCpe_DAC.Post();
			}			

			//����Paging Rsp(success or fail)��tVCR
			CMsg_Signal_VCR PagingRsp;
			VoiceVCRCtrlMsgT *pDataPagingRsp;
			if ( !PagingRsp.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(LAPagingRspT)) )
			{
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
			}
			else
			{
				PagingRsp.SetDstTid(M_TID_VCR);
				PagingRsp.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
				PagingRsp.SetBTSSAGID();
				PagingRsp.SetSigIDS(LAPagingRsp_MSG);
	    			
				pDataPagingRsp = (VoiceVCRCtrlMsgT*)PagingRsp.GetDataPtr();
				pDataPagingRsp->sigPayload.LAPagingRsp.Cause = 0;	//0�ɹ�,����fail
				memcpy(pDataPagingRsp->sigPayload.LAPagingRsp.UID , pDataPaging->sigPayload.LAPaging.UID, 4);
				memcpy(pDataPagingRsp->sigPayload.LAPagingRsp.L3addr , pDataPaging->sigPayload.LAPaging.L3addr, 4);
				memcpy(pDataPagingRsp->sigPayload.LAPagingRsp.App_Type , pDataPaging->sigPayload.LAPaging.App_Type, 2);
	    			
				PagingRsp.SetSigHeaderLengthField(sizeof(LAPagingRspT));
				PagingRsp.SetPayloadLength(sizeof(SigHeaderT)+sizeof(LAPagingRspT));
				PagingRsp.Post();	
			}
			
		}
	}
	else
	{
		LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleLAPagingReq, cannot find CCB by UID[0x%08x], BTS not Serving for this CPE", uid);
		return;
	}
}
/*
 *	�����յ���DELAPagingReq,����DELAPagingRsp
 */
void VoiceFSM::handleDELAPagingReq(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleDELAPagingReq");
	CMsg_Signal_VCR DELAPagingRsp;
	VoiceVCRCtrlMsgT *pDataReq, *pDataRsp;

	if ( !DELAPagingRsp.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(DELAPagingRspT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		DELAPagingRsp.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		DELAPagingRsp.SetDstTid(M_TID_VCR);
		DELAPagingRsp.SetBTSSAGID();
		DELAPagingRsp.SetSigIDS(DELAPagingRsp_MSG);

		pDataReq = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
		pDataRsp = (VoiceVCRCtrlMsgT*)DELAPagingRsp.GetDataPtr();

		memcpy(pDataRsp->sigPayload.DELAPagingRsp.UID , pDataReq->sigPayload.DELAPagingReq.UID, 4);
		pDataRsp->sigPayload.DELAPagingRsp.Cause = 0;

		DELAPagingRsp.SetSigHeaderLengthField(sizeof(DELAPagingRspT));
		DELAPagingRsp.SetPayloadLength(sizeof(SigHeaderT)+sizeof(DELAPagingRspT));

		DELAPagingRsp.Post();
	}    
}
/*
 *	�յ�SAG��Reset��Ϣ,��ӦResetAck
 */
void VoiceFSM::handleReset(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleReset");
	CMsg_Signal_VCR ResetAckMsg;

	if ( !ResetAckMsg.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		ResetAckMsg.SetDstTid(M_TID_VCR);
		ResetAckMsg.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		ResetAckMsg.SetBTSSAGID();
		ResetAckMsg.SetSigIDS(ResetAck_MSG);

		ResetAckMsg.SetSigHeaderLengthField(0);//û��Payload
		ResetAckMsg.SetPayloadLength(sizeof(SigHeaderT));

		ResetAckMsg.Post();
	}    
}
/*
 *	�յ�SAG��BeatHeart��Ϣ,��ӦBeatHeartAck
 */
void VoiceFSM::handleBeatHeart(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleBeatHeart");
	m_BeatHeart_Manager.handleBeatHeartMsg(msg);
}

/*
 *	�յ�SAG��ӵ������������Ϣ,��Ӧӵ������Ӧ�𲢱���Զ˵�ӵ���ȼ�
 */
void VoiceFSM::handleCongestReq(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleCongestReq");

	CMsg_Signal_VCR CongestRspMsg;
	VoiceVCRCtrlMsgT *pDataCongestReq, *pDataCongestRsp;

	pDataCongestReq = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	if ( !CongestRspMsg.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(CongestionCtrlRspT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		CongestRspMsg.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		CongestRspMsg.SetDstTid(M_TID_VCR);
		CongestRspMsg.SetBTSSAGID();
		CongestRspMsg.SetSigIDS(CongestionCtrlRsp_MSG);

		pDataCongestRsp = (VoiceVCRCtrlMsgT*)CongestRspMsg.GetDataPtr();

		pDataCongestRsp->sigPayload.CongestionCtrlRsp.level = m_LocalCongestLevel;

		CongestRspMsg.SetSigHeaderLengthField(sizeof(CongestionCtrlRspT));
		CongestRspMsg.SetPayloadLength(sizeof(SigHeaderT)+sizeof(CongestionCtrlRspT));
		CongestRspMsg.Post();
	}    
	//����SAGӵ���ȼ�
	m_RemoteCongestLevel = pDataCongestReq->sigPayload.CongestionCtrlReq.level;
}
/*
 *	�յ�SAG��ӵ������Ӧ����Ϣ,����Զ˵�ӵ���ȼ�
 */
void VoiceFSM::handleCongestRsp(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleCongestRsp");
	VoiceVCRCtrlMsgT *pDataCongestRsp = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	m_RemoteCongestLevel = pDataCongestRsp->sigPayload.CongestionCtrlRsp.level;
}
/*
 *	�յ�SAG��Ҫͨ��VAC͸����CPE����Ϣ,ͨ��VAC���͵�CPE,ͨ��VAC͸������Ϣͨ��L3Addr��ʶ
 */
void VoiceFSM::handleVCR_UTSAG_Msg_toVAC(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVCR_UTSAG_Msg_toVAC");

	CMsg_UTSAGSignal_VAC MsgToCpe_VAC;
	VoiceVCRCtrlMsgT *pDataVCR = (VoiceVCRCtrlMsgT*)msg.GetDataPtr(); 
	UINT16 nUTSAGPayloadLen = msg.GetDataLength() - sizeof(SigHeaderT) - sizeof(UINT32);

	UINT32 L3Addr = VGetU32BitVal(pDataVCR->sigPayload.UTSAG_Payload_L3Addr.L3Addr);
	VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByL3addr(L3Addr);
	if(NULL==pCCB)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVCR_UTSAG_Msg_toVAC, cannot find CCB");
		return;
	}

	//������Ϣ
	if ( !MsgToCpe_VAC.CreateMessage(*CTVoice::GetInstance(), nUTSAGPayloadLen+sizeof(UINT8)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}

	MsgToCpe_VAC.SetMessageId(MSGID_VOICE_VAC_SIGNAL);
	MsgToCpe_VAC.SetDstTid(M_TID_VAC);
	//�����,��Message Type��ʼ
	MsgToCpe_VAC.SetSignalPayload(&(pDataVCR->sigPayload.UTSAG_Payload_L3Addr.msgType), nUTSAGPayloadLen);
	//����ͷ
	MsgToCpe_VAC.SetEID(pCCB->getVoiceTuple().Eid);
	MsgToCpe_VAC.SetCID(pCCB->getVoiceTuple().Cid);
	MsgToCpe_VAC.SetPayloadLength(nUTSAGPayloadLen+sizeof(UINT8));

	if(MsgToCpe_VAC.Post())
	{
		Counters.nSigToVAC++;
	}

	//���ͨ���Ƿ����ӳɹ�
	CMsg_Signal_VCR signalMsgVCR(msg);
	if(ConnectAck_MSG==signalMsgVCR.ParseUTSAGMsgFromSAG())
	{
		pCCB->setConnectedFlag();
		LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Connected ******************************,UID[0x%08x] ", pCCB->getUID());
	}
	
}
/*
 *	�յ�SAG��Ҫͨ��DAC͸����CPE����Ϣ,ͨ��DAC���͵�CPE��ͨ��DAC͸������Ϣͨ��UID��ʶ
 */
void VoiceFSM::handleVCR_UTSAG_Msg_toDAC(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVCR_UTSAG_Msg_toDAC");
	
	CMsg_UTSAGSignal_DAC MsgToCpe_DAC;
	VoiceVCRCtrlMsgT *pDataVCR = (VoiceVCRCtrlMsgT*)msg.GetDataPtr(); 
	UINT16 nUTSAGPayloadLen = msg.GetDataLength() - sizeof(SigHeaderT) - sizeof(UINT32);
	
	UINT32 Uid = VGetU32BitVal(pDataVCR->sigPayload.UTSAG_Payload_Uid.Uid);
	VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByUID(Uid);
	if(NULL==pCCB)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVCR_UTSAG_Msg_toDAC, cannot find CCB");
		return;
	}
	
	//������Ϣ
	if ( !MsgToCpe_DAC.CreateMessage(*CTVoice::GetInstance(), nUTSAGPayloadLen+sizeof(UINT8)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}

	MsgToCpe_DAC.SetMessageId(MSGID_VOICE_DAC_SIGNAL);
	MsgToCpe_DAC.SetDstTid(M_TID_UTV);

	//�����,��Message Type��ʼ
	MsgToCpe_DAC.SetSignalPayload(&(pDataVCR->sigPayload.UTSAG_Payload_Uid.msgType), nUTSAGPayloadLen);
	//����ͷ
	MsgToCpe_DAC.SetEID(pCCB->getVoiceTuple().Eid);
	MsgToCpe_DAC.SetCID(pCCB->getVoiceTuple().Cid);
	MsgToCpe_DAC.SetPayloadLength(nUTSAGPayloadLen+sizeof(UINT8));
	
	if(MsgToCpe_DAC.Post())
	{
		Counters.nSigToDAC++;
	}
}

/*
 *	�յ�SAG��Ҫͨ��DAC͸����CPE��oam��Ϣ,ͨ��DAC���͵�CPE��ͨ��DAC͸������Ϣͨ��UID��ʶ
 */
void VoiceFSM::handleVCR_UTSAG_OAM_Msg_toDAC(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVCR_UTSAG_OAM_Msg_toDAC");
	
	CMsg_UTSAGSignal_DAC MsgToCpe_DAC;
	VoiceVCRCtrlMsgT *pDataVCR = (VoiceVCRCtrlMsgT*)msg.GetDataPtr(); 
	UINT16 nUTSAGPayloadLen = msg.GetDataLength() - sizeof(SigHeaderT) - sizeof(UINT32);
	
	UINT32 Uid = VGetU32BitVal(pDataVCR->sigPayload.UTSAG_Payload_Uid.Uid);
	UINT32 tmpEid = l3oamGetEIDByUID(Uid);
	if(0xffffffff==tmpEid)
	{
		LOG1(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "cannot get EID for UID[0x%08X]!!!", Uid);
		return;
	}
	
	//������Ϣ
	if ( !MsgToCpe_DAC.CreateMessage(*CTVoice::GetInstance(), nUTSAGPayloadLen+sizeof(UINT8)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}

	MsgToCpe_DAC.SetMessageId(MSGID_DAC_DL_CPEOAM_MSG);
	MsgToCpe_DAC.SetDstTid(M_TID_CPECM);

	//�����,��Message Type��ʼ
	MsgToCpe_DAC.SetSignalPayload(&(pDataVCR->sigPayload.UTSAG_Payload_Uid.msgType), nUTSAGPayloadLen);
	//����ͷ
	MsgToCpe_DAC.SetEID(tmpEid);
	MsgToCpe_DAC.SetCID(0);
	MsgToCpe_DAC.SetPayloadLength(nUTSAGPayloadLen+sizeof(UINT8));
	
	if(MsgToCpe_DAC.Post())
	{
		Counters.nSigToDAC++;
	}
}

void VoiceFSM::handleVCR_UT_DL_Relay_Msg(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleVCR_UT_DL_Relay_Msg");
	
	CComMessage *pMsg;
	VoiceVCRCtrlMsgT *pDataVCR = (VoiceVCRCtrlMsgT*)msg.GetDataPtr(); 
	UINT16 nUTSAGPayloadLen = VGetU16BitVal(pDataVCR->sigHeader.Length);
	
	//������Ϣ
	pMsg = new (CTVoice::GetInstance(), nUTSAGPayloadLen) CComMessage;
	if ( !pMsg )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}

	pMsg->SetMessageId(MSGID_CPE_DL_RELAY_MSG);
	pMsg->SetDstTid(M_TID_CPECM);
	pMsg->SetSrcTid(M_TID_VOICE);

	//�����
	memcpy((UINT8*)pMsg->GetDataPtr(), 
		pDataVCR->sigPayload.UTSAG_Signal_XXX.XXX, nUTSAGPayloadLen);
	//����ͷ
	UINT32 tmpEid = VGetU32BitVal(pDataVCR->sigPayload.UTSAG_Signal_XXX.UTSAGPayload);
	pMsg->SetEID(tmpEid);
	pMsg->SetDataLength(nUTSAGPayloadLen);
		
	if(postComMsg(pMsg))
	{
		Counters.nSigToDAC++;
	}
}

/*
 *	�յ�L2�����ϱ���ӵ���ȼ���Ϣ,����congestion Req��SAG
 */
void VoiceFSM::handleL2CongestionReportMsg(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleL2CongestionReportMsg");
	UINT8 level = *((UINT8*)msg.GetDataPtr());	//congestion level
	
	CMsg_Signal_VCR congestReq;
	if ( !congestReq.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(CongestionCtrlReqT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		congestReq.SetDstTid(M_TID_VCR);
		congestReq.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		congestReq.SetBTSSAGID();
		congestReq.SetSigIDS(CongestionCtrlReq_MSG);

		VoiceVCRCtrlMsgT *pData = (VoiceVCRCtrlMsgT*)congestReq.GetDataPtr();

		pData->sigPayload.CongestionCtrlReq.level = level;

		congestReq.SetSigHeaderLengthField(sizeof(CongestionCtrlReqT));
		congestReq.SetPayloadLength(sizeof(SigHeaderT)+sizeof(CongestionCtrlReqT));

		congestReq.Post();
	}    
	//���汾��ӵ���ȼ�
	m_LocalCongestLevel = level;
}
/*************************************
*����SAG->Bts�Ĺ㲥��������
*edit by yhw
*
**************************************/
#ifdef NUCLEAR_CODE
UINT8 g_BchSmsSndCnt=10;
#else
UINT8 g_BchSmsSndCnt=1;
#endif
extern "C" void showBchSmsCnt()
{
    VPRINT("g_BchSmsSndCnt = %d\n", g_BchSmsSndCnt);
}

extern "C" void setBchSmsCnt(UINT8 n)
{
    g_BchSmsSndCnt = n;
}

void VoiceFSM::handleBroadcast_SM(CMessage& msg)
{
//��utv���͹㲥���Ÿ�ʽ20091111by fengbing
	typedef struct __BchSmsMsgT
	{
		UINT8 msgType[2];
		UINT8 CID;
		UINT8 SequenceNo;//from sag
		UINT8 payload[200];//from sag
	}BchSmsMsgT;
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleBroadcast_SM");
	VoiceVCRCtrlMsgT *pDataVCR = (VoiceVCRCtrlMsgT*)msg.GetDataPtr(); 
	UINT16 LenNotUsed = ((UINT8*)&pDataVCR->sigPayload.Broadcast_SM.BroadcastData) - 
		((UINT8*)&pDataVCR->sigPayload.Broadcast_SM);
	//yhw
	/*perserved code ,broadcast SM to L2*/
	UINT8 blSuccess = BRDCSTSTAS_SUCCESS;
	for(int i=0;i<g_BchSmsSndCnt;i++)
	{
		UINT16 nPayloadLen = msg.GetDataLength() - sizeof(SigHeaderT) - LenNotUsed;
		CComMessage* pBroadcastSMS = new (CTVoice::GetInstance(),nPayloadLen+3) CComMessage;
		if(pBroadcastSMS!=NULL)
		{
			pBroadcastSMS->SetEID(0xFFFFFFFE);
			pBroadcastSMS->SetMessageId(MSGID_BROADCAST_OAM);
			pBroadcastSMS->SetDstTid(M_TID_UTV);
			pBroadcastSMS->SetSrcTid(M_TID_VOICE);
			pBroadcastSMS->SetDataLength(nPayloadLen+3);
			BchSmsMsgT* pDataSms = (BchSmsMsgT*)pBroadcastSMS->GetDataPtr();
			VSetU16BitVal(pDataSms->msgType, M_BROADCAST_MSG_SMS);
			pDataSms->CID = 0;
			memcpy((void*)(&pDataSms->SequenceNo), (void*)&pDataVCR->sigPayload.Broadcast_SM.BroadcastData, nPayloadLen);
			if(!CComEntity::PostEntityMessage(pBroadcastSMS))
			{
				pBroadcastSMS->Destroy();
				blSuccess = BRDCSTSTAS_FAIL;
			}
		}
		else
		{
			blSuccess = BRDCSTSTAS_FAIL;
		}
	}
	//send ack to SAG
	CMsg_Signal_VCR broadcast_SMACK;
	if ( !broadcast_SMACK.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(Broadcast_SM_ACKT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		broadcast_SMACK.SetDstTid(M_TID_VCR);
		broadcast_SMACK.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		broadcast_SMACK.SetBTSSAGID();
		broadcast_SMACK.SetSigIDS(BROADCAST_SM_ACK_MSG);

		VoiceVCRCtrlMsgT *pData = (VoiceVCRCtrlMsgT*)broadcast_SMACK.GetDataPtr();

		memcpy(pData->sigPayload.Broadcast_SM_ACK.GlobalMsgSeq , pDataVCR->sigPayload.Broadcast_SM.GlobalMsgSeq, 4);
		pData->sigPayload.Broadcast_SM_ACK.BroadcastStatus = blSuccess;


		broadcast_SMACK.SetSigHeaderLengthField(sizeof(Broadcast_SM_ACKT));
		broadcast_SMACK.SetPayloadLength(sizeof(SigHeaderT)+sizeof(Broadcast_SM_ACKT));

		broadcast_SMACK.Post();
	}    
	
}

/*
 *	��Ӧ reject
 */
void VoiceFSM::handleSendRejectToSAG(ENUM_REJECTRESN reason)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleSendRejectToSAG");

	CMsg_Signal_VCR sendRejectMsg;
	VoiceVCRCtrlMsgT  *pDataRejectRsp;

	if ( !sendRejectMsg.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(SendRejectToSAGT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		sendRejectMsg.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		sendRejectMsg.SetDstTid(M_TID_VCR);
		sendRejectMsg.SetBTSSAGID();
		sendRejectMsg.SetSigIDS(REJECT_MSG);

		pDataRejectRsp = (VoiceVCRCtrlMsgT*)sendRejectMsg.GetDataPtr();

		pDataRejectRsp->sigPayload.SendRejectToSAG.RejectReason =reason;

		sendRejectMsg.SetSigHeaderLengthField(sizeof(SendRejectToSAGT));
		sendRejectMsg.SetPayloadLength(sizeof(SigHeaderT)+sizeof(SendRejectToSAGT));
		sendRejectMsg.Post();
	}    
}
/*
 *	�յ�SAG��Reset Request��Ϣ,��ӦRestart Indication
 */
void VoiceFSM::handleResetReq(CMessage& msg)
{
	LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "VoiceFSM::handleReset");
	CMsg_Signal_VCR RestartMsg;
	if ( !RestartMsg.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(RestartIndiToSAGT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		RestartMsg.SetDstTid(M_TID_VCR);
		RestartMsg.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		RestartMsg.SetBTSSAGID();
		RestartMsg.SetSigIDS(ResetAck_MSG);

		VoiceVCRCtrlMsgT *pDataIndi = (VoiceVCRCtrlMsgT*)RestartMsg.GetDataPtr();

		pDataIndi->sigPayload.RestartIndiToSAG.Indication=0;

		RestartMsg.SetSigHeaderLengthField(sizeof(RestartIndiToSAGT));//û��Payload
		RestartMsg.SetPayloadLength(sizeof(SigHeaderT)+sizeof(RestartIndiToSAGT));

		RestartMsg.Post();
	}    
}

//��Ⱥ���----------------------------------------------------------
//from L2
void VoiceFSM::handle_ResCfm_frmL2(CMessage& msg)
{
	//Ŀǰ������ͷ���Դ����Ҫ��Ӧ
	L2L3_GrpResCfmT* pData = (L2L3_GrpResCfmT*)msg.GetDataPtr();
	UINT16 tmpGID = VGetU16BitVal(pData->GID);
	LOG4(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"ResConfirm<----btsL2; GID[0x%04X] result[0x%02X] reason[0x%02X] reportID[0x%04X]",
		tmpGID, pData->Result, pData->Reason, VGetU16BitVal(pData->ReportID));
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(tmpGID);
	if(NULL!=pGrpCCB)
	{
		if(GET_GRPRES==pData->Operation || GET_GRPRES_VIDEOUT==pData->Operation)
		{
			if(M_GRP_L2L3_SUCCESS==pData->Result)
			{
				pGrpCCB->airRes.setResReadyFlag(true);
				pGrpCCB->airRes.airRes = pData->airRes;
				switch(pData->Reason)
				{
				case ResReason_GrpPaging:		//���Ѱ��
					pGrpCCB->sendGrpPagingReq2L2(pGrpCCB->getCommType(), ISNOT_LEPAGING, pGrpCCB->getTransID());
					pGrpCCB->startGrpTimer(TIMER_GrpPagingRsp);
					pGrpCCB->sendGrpCallInd2CPE(pGrpCCB->getCommType(), ISNOT_LEPAGING, pGrpCCB->getTransID());
					break;
				case ResReason_LePaging:			//�ٺ����Ѱ��
					pGrpCCB->sendGrpPagingReq2L2(pGrpCCB->getCommType(), IS_LEPAGING, pGrpCCB->getTransID());
					pGrpCCB->startGrpTimer(TIMER_LePagingRsp);
					pGrpCCB->sendGrpCallInd2CPE(pGrpCCB->getCommType(), IS_LEPAGING, pGrpCCB->getTransID());
					break;
				case ResReason_GrpHandover:		//����л�
					pGrpCCB->sendHoResRsp2otherBTS(msg);
					break;
				case ResReason_GrpCpeRequest:	//cpe���룬���绰Ȩ�л�ʱ
					pGrpCCB->sendGrpShareResRsp2CPE(msg);
					break;
				default:
					LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_INVALID_SIGNAL), "invalid ResReason!!!");
				}
			}
			else
			{
				LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_INVALID_SIGNAL), "Alloc AirRes Failed !!!#########");
				switch(pData->Reason)
				{
				case ResReason_GrpPaging:		//���Ѱ��
					//��sxc��ʧ�ܵ�Ѱ����Ӧ
					pGrpCCB->sendGrpPagingRsp2SXC(1, 0xffffffff);
					LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
						"DeAllocGrpCCB; GID[0x%04X]", pGrpCCB->getGID());
					pGrpCCBTbl->DeAllocGrpCCB(pGrpCCB);
					break;
				case ResReason_LePaging:			//�ٺ����Ѱ��
					//do nothing
					break;
				case ResReason_GrpHandover:		//����л�
					pGrpCCB->sendHoResRsp2otherBTS(msg);
					break;
				case ResReason_GrpCpeRequest:	//cpe���룬���绰Ȩ�л�ʱ
					pGrpCCB->sendGrpShareResRsp2CPE(msg);
					break;
				default:
					LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_INVALID_SIGNAL), "invalid ResReason!!!");
				}				
				
				LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "GET GrpRes failed!!!");
			}		
		}
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find GrpCCB!!!");
	}
}

void VoiceFSM::handle_StatusReportInd_frmL2(CMessage& msg)
{
	L2L3_StatusReportIndiT* pData = (L2L3_StatusReportIndiT*)msg.GetDataPtr();
	VoiceTuple tuple;
	tuple.Eid = VGetU32BitVal(pData->EID);
	tuple.Cid = pData->cid;
	UINT16 tmpGID = VGetU16BitVal(pData->GID);
	LOG3(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"StatusReportInd<----btsL2; GID[0x%04X] EID[0x%08X] CID[0x%02X]",
		tmpGID, tuple.Eid, pData->cid);
	
	VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(NULL==pCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find VoiceCCB!!!");
		return;
	}
	GrpCCB *pGrpCCB = pGrpCCBTbl->FindCCBByGID(tmpGID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find GrpCCB!!!");
		return;
	}
	GrpCpeStatusT item;
	VSetU32BitVal(item.UID , pCCB->getUID());
	item.status = M_STATUSREPORT_HEARTBEAT;
	pGrpCCB->statusReport.addStatusReportItem(item);
	pGrpCCB->markGrpNotEmpty();//fengbing 20091026 �޸�״̬�����ж����Ƿ�Ϊ�յ�ȱ��
}

void VoiceFSM::handle_GrpPagingRsp_frmL2(CMessage& msg)
{
	L2L3_GrpPagingRspT*pData =  (L2L3_GrpPagingRspT*)msg.GetDataPtr();
	VoiceTuple tuple;
	tuple.Eid = VGetU32BitVal(pData->EID);
	tuple.Cid = pData->cid;
	UINT16 tmpGID = VGetU16BitVal(pData->GID);
	LOG4(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GrpPagingRsp<----btsL2; GID[0x%04X] Result[0x%02X] EID[0x%08X] CID[0x%02X]",
		tmpGID, pData->Result, tuple.Eid, pData->cid);
	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(tmpGID);
	if(NULL!=pGrpCCB)
	{
		if(M_GRP_L2L3_SUCCESS==pData->Result)
		{
			UINT32 tmpUID = INVALID_UID;
			if(tuple.Eid!=NO_EID)
			{
				VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
				if(NULL==pCCB)
				{
					LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find VoiceCCB!!!");
					return;
				}
				tmpUID = pCCB->getUID();
			}
			else
			{
				//������Ѱ����Ӧ
			}
			
			//�ٺ����Ѱ������׸�Ѱ����Ӧ
			if(GRP_WORKING_STATE==pGrpCCB->GetCurrentState())
			{
				pGrpCCB->deleteGrpTimer(TIMER_LePagingRsp);
				#if 0//GrpVersion002
				pGrpCCB->sendGrpResReq2SXC(pCCB->getUID(), pCCB->getVoiceTuple().Eid);
				#endif
				//GrpVersion003 begin				
				if(M_INVALID_GRPL3ADDR==pGrpCCB->getGrpL3Addr())
				{
					//grpL3Addr��Чʱ����SXC����GrpResReq������SXC�Ļ�Ӧ��Ϣ����grpL3Addr,�Ա��������������͸����Ϣ
					//20090508 by fengbing
					pGrpCCB->sendGrpResReq2SXC(tmpUID, 0);//pData->EID);
				}
				else
				{
					//���е�Ѱ����Ӧ������SXC,�����ٺ�Ѱ����Ӧ
					pGrpCCB->sendGrpPagingRsp2SXC(pData->Result, tmpUID);
				}

				//���������ͷ���Դ
				if(pGrpCCB->airRes.isResReady())
				{
					pGrpCCB->airRes.setResReason(ResReason_GrpCpeRequest);
					pGrpCCB->deleteGrpTimer(TIMER_ResClear);
					pGrpCCB->startGrpTimer(TIMER_ResClear);
					pGrpCCB->markGrpNotEmpty();//fengbing 20091026 �޸�״̬�����ж����Ƿ�Ϊ�յ�ȱ��
				}
				else
				{
					LOG3(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
					"GID[0x%04X] UID[0x%08X] PID[0x%08X] airRes not ready when pagingRsp received##########", 
					tmpGID, tmpUID, tuple.Eid);
				}
				//GrpVersion003 end
			}
			//�ǳٺ����Ѱ��
			else
			{
				pGrpCCB->deleteGrpTimer(TIMER_GrpPagingRsp);
				pGrpCCB->SetCurrentState(GRP_WORKING_STATE);
				LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
					"GID[0x%04X] Enter state[GRP_WORKING_STATE]", tmpGID);
				pGrpCCB->sendGrpPagingRsp2SXC(pData->Result, tmpUID);
				pGrpCCB->deleteGrpTimer(TIMER_ResClear);
				pGrpCCB->startGrpTimer(TIMER_ResClear);//fengbing 20091026 ���ϼ����Դ������
				pGrpCCB->markGrpNotEmpty();//fengbing 20091026 �޸�״̬�����ж����Ƿ�Ϊ�յ�ȱ��
			}
		}
		else
		{
			LOG(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "recv failed GrpPagingRsp Msg!!!");
		}
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find GrpCCB!!!");
	}
}

//from SXC
bool VoiceFSM::refuseGrpPagingRsp2SXC(CMessage& msg)
{
	bool ret;
	CMsg_Signal_VCR SAbisSignal;
	VoiceVCRCtrlMsgT *pData, *pDataSrc;
	if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpLAPagingRspT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		pDataSrc = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
		pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
		SAbisSignal.SetDstTid(M_TID_VCR);
		SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		SAbisSignal.SetBTSSAGID();
		SAbisSignal.SetSigIDS(LAGrpPagingRsp_MSG);

		pData->sigPayload.GrpLAPagingRsp.Cause = 1;
		pData->sigPayload.GrpLAPagingRsp.commType = pDataSrc->sigPayload.GrpLAPaging.CommType;
		memcpy(pData->sigPayload.GrpLAPagingRsp.GrpL3Addr , pDataSrc->sigPayload.GrpLAPaging.GrpL3Addr, 4);
		VSetU32BitVal(pData->sigPayload.GrpLAPagingRsp.UID , 0xffffffff);

		SAbisSignal.SetSigHeaderLengthField(sizeof(GrpLAPagingRspT));
		SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpLAPagingRspT));
		ret = SAbisSignal.Post();
		if(ret)
		{
			LOG3(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"LAGrpPagingRsp(refuse)---->SXC; GID[0x%04X] Result[0x%02X] UID[0x%08X]",
				VGetU16BitVal(pDataSrc->sigPayload.GrpLAPaging.GID), pData->sigPayload.GrpLAPagingRsp.Cause, 
				VGetU32BitVal(pData->sigPayload.GrpLAPagingRsp.UID));
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Send Msg to VCR fail!!!");			
		}
		return ret;
	}	
}

void VoiceFSM::handle_LAGrpPaging_frmSXC(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pData->sigPayload.GrpLAPaging.GID);
	UINT32 grpL3Addr = VGetU32BitVal(pData->sigPayload.GrpLAPaging.GrpL3Addr);
	UINT16 grpSize = VGetU16BitVal(pData->sigPayload.GrpLAPaging.GrpSize);
	UINT8 transID = pData->sigPayload.GrpLAPaging.transID;
	UINT8 encryptFlag = pData->sigPayload.GrpLAPaging.EncryptFlag;
	LOG5(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"LAGrpPaging<----SXC; GID[0x%04X] grpL3Addr[0x%08X] transID[0x%02X] grpSize[0x%04X] EncryptFlag[0x%02X]************",
		GID, grpL3Addr, transID, grpSize, encryptFlag);
	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(GID);
	if(NULL==pGrpCCB)
	{
		if( MAX_INSRV_GRPNUM<=pGrpCCBTbl->getActiveGrpNum())
		{
			refuseGrpPagingRsp2SXC(msg);
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Too many Active Groups !!!refuseGrpPagingRsp2SXC");
			return;
		}
		pGrpCCB = pGrpCCBTbl->AllocGrpCCB();
		if(NULL==pGrpCCB)
		{
			refuseGrpPagingRsp2SXC(msg);
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot Alloc GrpCCB!!!refuseGrpPagingRsp2SXC");
			return;
		}
		LOG2(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"ALLOC  GrpCCB;GID[0x%04X] grpL3Addr[0x%08X]  ",
			GID, grpL3Addr);
		//����transID
		pGrpCCB->setTransID(transID);
		//����������
		pGrpCCB->setGID(GID);
		pGrpCCBTbl->AddBTreeGID(pGrpCCB->getGID(), pGrpCCB->getTabIndex());		
	}
	else
	{
		LOG2(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GID[0x%04X] grpL3Addr[0x%08X]  GrpCCB already Exist!!!",
			GID, grpL3Addr);
		pGrpCCB->deleteGrpTimer(TIMER_GrpRls);//ʹ���ͷŹ����������齨��ʱ�����ٱ��ͷ�20120703 add by fb
		pGrpCCB->airRes.setResReadyFlag(false);//��֤���ͷŹ������ٴ��½���Ҫ�������루��ѯ����Դ20120703 add by fb
		//����transID
		pGrpCCB->setTransID(transID);
		//��GrpL3Addr����
		pGrpCCBTbl->DelBTreeGrpL3addr(pGrpCCB->getGrpL3Addr());
		
		if(pGrpCCB->airRes.isResReady())
		{
			//��ǰ��CCB������Դ�Ѿ���������
			//����Ѿ���������Դ��ֻ�����grpL3Addr��
			//������������Դ��������btsL2��GrpPaging(�ǳٺ�)
			LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GID[0x%04X] already has Resource!Sending GrpPaging(Not LEPaging)", pGrpCCB->getGID());
#if 0			
			pGrpCCB->sendGrpResReq2L2(FREE_GRPRES, NOT_NEED_RSP, 
				ResReason_GrpPaging, pGrpCCB->getTransID(), 
				NO_REPORTID, M_INVALID_EID, M_INVALID_BTSID);
#endif
		}
		else
		{
			//���û�з�����Դ��һ�д�ͷ��ʼ
			pGrpCCB->CCBClean();
			LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GID[0x%04X] has no Resource!Restart just like a new alloced GrpCCB!", pGrpCCB->getGID());
		}		
	}
	pGrpCCB->SetCurrentState(GRP_PAGING_STATE);
	LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GID[0x%04X] Enter state[GRP_PAGING_STATE]", pGrpCCB->getGID());
	//����ҵ������
	pGrpCCB->setGID(GID);						//UINT16 GID;			//GID	2	M		
	pGrpCCB->setGrpSize(grpSize);	//UINT16 GrpSize;		//Group size	2	M		���Ա�ڸû�վ����Ŀ
	pGrpCCB->setGrpL3Addr(grpL3Addr);//UINT32 GrpL3Addr;	//L3Addr	4	M		��L3
	pGrpCCB->setCommType(pData->sigPayload.GrpLAPaging.CommType);	//UINT32 CommType;	//Communication Type	1	M		ͨ������
	pGrpCCB->setPrioty(pData->sigPayload.GrpLAPaging.CallPrioty);			//UINT8 CallPrioty;		//Call priority	1	M		��������Ȩ
	pGrpCCB->setEncryptFlag(encryptFlag);	//UINT8 EncryptFlag;	//Encryption Flag	1	M		�˵��˼���
#ifdef M__SUPPORT__ENC_RYP_TION
	//���¼�������
	if(encryptFlag!=ENCRYPT_CTRL_NOTUSE)
	{
		pGrpCCB->setEncryptKey(pData->sigPayload.GrpLAPaging.EncryptKey);
	}
#endif
	//����������
	pGrpCCBTbl->AddBTreeGrpL3addr(pGrpCCB->getGrpL3Addr(), pGrpCCB->getTabIndex());
	pGrpCCB->airRes.setResReason(ResReason_GrpPaging);
#ifdef M_SYNC_BROADCAST
	if(pGrpCCB->getCommType()==COMM_TYPE_SYNC_BROADCAST_VOICE ||
		pGrpCCB->getCommType()==COMM_TYPE_SYNC_BROADCAST_DATA)
	{
		//����MBMS_Group_Resource.Indication��Ϣ��BTSL2֪ͨSXCΪͬ����ѡȡ����Դ
		pGrpCCB->sendGrpMBMSGrpResIndication2L2(msg);
		//��BTSL2���»�ȡ��Դ����֤����Դʹ��SXC���·��������Դ
		pGrpCCB->airRes.setResReadyFlag(false);
	}
#endif//M_SYNC_BROADCAST	
	if(pGrpCCB->airRes.isResReady())
	{
		//��ǰ��CCB������Դ�Ѿ���������
		//����Ѿ���������Դ��ֻ�����grpL3Addr��
		//������������Դ��������btsL2��GrpPaging(�ǳٺ�)
		pGrpCCB->sendGrpPagingReq2L2(pData->sigPayload.GrpLAPaging.CommType, 
				ISNOT_LEPAGING, pGrpCCB->getTransID());
		pGrpCCB->clearAllTimers();
		pGrpCCB->startGrpTimer(TIMER_GrpPagingRsp);
		pGrpCCB->sendGrpCallInd2CPE(pData->sigPayload.GrpLAPaging.CommType, 
			ISNOT_LEPAGING, pGrpCCB->getTransID());
	}
	else
	{
		//��L2������Դ
		pGrpCCB->sendGrpResReq2L2(GET_GRPRES, NEED_RSP, 
					ResReason_GrpPaging, pGrpCCB->getTransID(), 
					NO_REPORTID, M_INVALID_EID, M_INVALID_BTSID);
	}
#if 0//GrpVersion003����Ⱥ3�ڳٺ����ֻ������SXC
	pGrpCCB->startGrpTimer(TIMER_LePagingStart);
#endif	
	pGrpCCB->startGrpTimer(TIMER_GrpDataDetect);
	pGrpCCB->startGrpTimer(TIMER_StautsReport);
	
}

void VoiceFSM::handle_LEGrpPaging_frmSXC(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pData->sigPayload.LEGrpPaging.GID);
	UINT32 grpL3Addr = VGetU32BitVal(pData->sigPayload.LEGrpPaging.GrpL3Addr);
	UINT16 grpSize = VGetU16BitVal(pData->sigPayload.LEGrpPaging.GrpSize);
	UINT8 transID = pData->sigPayload.LEGrpPaging.transID;
	UINT8 encryptFlag = pData->sigPayload.LEGrpPaging.EncryptFlag;
	LOG5(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"LEGrpPaging<----SXC; GID[0x%04X] grpL3Addr[0x%08X] transID[0x%02X] grpSize[0x%04X] EncryptFlag[0x%02X]************",
		GID,  grpL3Addr, transID, grpSize, encryptFlag);
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(GID);
	if(NULL==pGrpCCB)
	{
#if 0	//GrpVersion002		
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find GrpCCB!!!");
		return;
#endif
		//GrpVersion003 begin
		//�ٺ����Ѱ������ĵ�һ����Ϣ����ʱ����bts�����飬����ҵ������
		LOG(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "GrpCCB not exist when LEGrpPaging received...");
		if( MAX_INSRV_GRPNUM<=pGrpCCBTbl->getActiveGrpNum())
		{
			refuseGrpPagingRsp2SXC(msg);
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Too many Active Groups !!!refuseGrpPagingRsp2SXC");
			return;
		}
		pGrpCCB = pGrpCCBTbl->AllocGrpCCB();
		if(NULL==pGrpCCB)
		{
			refuseGrpPagingRsp2SXC(msg);
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot Alloc GrpCCB!!!refuseGrpPagingRsp2SXC");
			return;
		}
		LOG2(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"ALLOC  GrpCCB;GID[0x%04X] grpL3Addr[0x%08X]  ",
			GID, grpL3Addr);
		//����transID
		pGrpCCB->setTransID(transID);
		//����������
		pGrpCCB->setGID(GID);
		pGrpCCBTbl->AddBTreeGID(pGrpCCB->getGID(), pGrpCCB->getTabIndex());

		pGrpCCB->SetCurrentState(GRP_WORKING_STATE);
		LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GID[0x%04X] Enter state[GRP_WORKING_STATE]", pGrpCCB->getGID());
		pGrpCCB->setGrpL3Addr(grpL3Addr);//UINT32 GrpL3Addr;	//L3Addr	4	M		��L3
		//����������
		pGrpCCBTbl->AddBTreeGrpL3addr(pGrpCCB->getGrpL3Addr(), pGrpCCB->getTabIndex());
		pGrpCCB->airRes.setResReason(ResReason_LePaging);
		pGrpCCB->airRes.setResReadyFlag(false);

		pGrpCCB->startGrpTimer(TIMER_GrpDataDetect);
		pGrpCCB->startGrpTimer(TIMER_StautsReport);
		
		//GrpVersion003 end
	}
	else
	{
		pGrpCCB->deleteGrpTimer(TIMER_GrpRls);//ʹ���ͷŹ����������齨��ʱ�����ٱ��ͷ�20120703 add by fb
		//pGrpCCB->airRes.setResReadyFlag(false);//��֤���ͷŹ������ٴ��½���Ҫ�������루��ѯ����Դ20120703 add by fb
		//����transID
		pGrpCCB->setTransID(transID);		
		//GrpVersion003 begin
		//�п���grpL3Addr�����仯
		if(pGrpCCB->getGrpL3Addr() != grpL3Addr)//20090401
		{
			pGrpCCBTbl->DelBTreeGrpL3addr(pGrpCCB->getGrpL3Addr());
			pGrpCCB->setGrpL3Addr(grpL3Addr);
			pGrpCCBTbl->AddBTreeGrpL3addr(pGrpCCB->getGrpL3Addr(), pGrpCCB->getTabIndex());
		}
		//����grpSize
		pGrpCCB->setGrpSize(grpSize);
		//GrpVersion003 end

		//fengbing,20090708����ٺ���뿪ʼ���״�Ѱ����ʱ�����L3,�������޷��ͷ�begin
		pGrpCCB->deleteGrpTimer(TIMER_GrpPagingRsp);
		//pGrpCCB->airRes.setResReadyFlag(false);
		pGrpCCB->SetCurrentState(GRP_WORKING_STATE);
		LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GID[0x%04X] Enter state[GRP_WORKING_STATE]", pGrpCCB->getGID());
		//fengbing,20090708����ٺ���뿪ʼ���״�Ѱ����ʱ�����L3,�������޷��ͷ�end		
	}
	//����ҵ������
	pGrpCCB->setGID(GID);						//UINT16 GID;			//GID	2	M		
	pGrpCCB->setGrpSize(grpSize);	//UINT16 GrpSize;		//Group size	2	M		���Ա�ڸû�վ����Ŀ
	pGrpCCB->setGrpL3Addr(grpL3Addr);//UINT32 GrpL3Addr;	//L3Addr	4	M		��L3
	pGrpCCB->setCommType(pData->sigPayload.LEGrpPaging.commType);	//UINT32 CommType;	//Communication Type	1	M		ͨ������
	pGrpCCB->setPrioty(pData->sigPayload.LEGrpPaging.CallPrioty);			//UINT8 CallPrioty;		//Call priority	1	M		��������Ȩ
	pGrpCCB->setEncryptFlag(encryptFlag);	//UINT8 EncryptFlag;	//Encryption Flag	1	M		�˵��˼���
#ifdef M__SUPPORT__ENC_RYP_TION
	//���¼�������
	if(encryptFlag!=ENCRYPT_CTRL_NOTUSE)
	{
		pGrpCCB->setEncryptKey(pData->sigPayload.LEGrpPaging.EncryptKey);
	}	
#endif
	pGrpCCB->utmLenLePagingLoop = VOICE_ONE_SECOND*pData->sigPayload.LEGrpPaging.LEPeriod;
	pGrpCCB->deleteGrpTimer(TIMER_LePagingStart);

#ifdef M_SYNC_BROADCAST
	if(pGrpCCB->getCommType()==COMM_TYPE_SYNC_BROADCAST_VOICE ||
		pGrpCCB->getCommType()==COMM_TYPE_SYNC_BROADCAST_DATA)
	{
		//����MBMS_Group_Resource.Indication��Ϣ��BTSL2֪ͨSXCΪͬ����ѡȡ����Դ
		pGrpCCB->sendGrpMBMSGrpResIndication2L2(msg);
		//��BTSL2���»�ȡ��Դ����֤����Դʹ��SXC���·��������Դ
		pGrpCCB->airRes.setResReadyFlag(false);
	}
#endif//M_SYNC_BROADCAST	

	//���ų�֮ǰ�Ѿ������˸ö�ʱ��Tm_LepagingLoop
	pGrpCCB->deleteGrpTimer(TIMER_LePagingLoop);
	pGrpCCB->startGrpTimer(TIMER_LePagingLoop);
	pGrpCCB->setLePagingStartFlag(true);
	//GrpVersion003 begin
	//���̿�ʼ�ٺ�Ѱ��,���Լ�����һ���ٺ����ڵ�����Ϣ����
	CMsg_VoiceTimeout timeoutMsg;
	if ( !timeoutMsg.CreateMessage(*CTVoice::GetInstance()) )
	{
		LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
	else
	{
		timeoutMsg.SetTimerType(TIMER_LePagingLoop);
		timeoutMsg.SetMessageId(MSGID_TIMER_GRP_LEPAGING_LOOP);
		timeoutMsg.SetSrcTid(M_TID_VOICE);
		timeoutMsg.SetDstTid(M_TID_VOICE);
		timeoutMsg.SetGid(GID);
		timeoutMsg.SetCid(DEFAULT_CID);
	} 
	if(!timeoutMsg.Post())
	{
		timeoutMsg.DeleteMessage();
	}
	//GrpVersion003 end
	
}

void VoiceFSM::handle_DeLEGrpPaging_frmSXC(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pData->sigPayload.DeLEGrpPaging.GID);
	LOG1(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"DeLEGrpPaging<----SXC; GID[0x%04X] ", GID);
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find GrpCCB!!!");
		return;
	}
	pGrpCCB->deleteGrpTimer(TIMER_LePagingRsp);	
	pGrpCCB->deleteGrpTimer(TIMER_LePagingLoop);
	pGrpCCB->setLePagingStartFlag(false);
}

void VoiceFSM::handle_GrpHandoverRsp_frmSXC(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pData->sigPayload.GrpHandoverRsp.GID);
	UINT32 grpL3Addr = VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.GrpL3Addr);
	UINT32 UID = VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.UID);
	UINT32 PID = VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.PID);
	UINT32 bsID = VGetU32BitVal(pData->sigPayload.GrpHandoverRsp.curBTSID);
	LOG4(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GroupHandoverRsp<----SXC; GID[0x%04X] grpL3Addr[0x%08X] Resultp[0x%02X] UID[0x%08X]",
		GID, grpL3Addr, pData->sigPayload.GrpHandoverRsp.Result, UID);	
	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(GID);
//GrpVersion003 begin
	if(M_SABIS_SUCCESS==pData->sigPayload.GrpHandoverRsp.Result)
	{
		if(NULL==pGrpCCB)
		{
			pGrpCCB = pGrpCCBTbl->AllocGrpCCB();
			if(NULL==pGrpCCB)
			{
				LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot Alloc GrpCCB!!!");
				sendGrpHandoverRsp2otherBTS(msg, false, 9);
				return;
			}
			LOG2(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"ALLOC  GrpCCB;GID[0x%04X] grpL3Addr[0x%08X]  ",
				GID, grpL3Addr);
			//����transID
			if(sizeof(GrpHandoverRspT)> VGetU16BitVal(pData->sigHeader.Length))
			{
				//���ݾɵ�sac�汾���п���û��transID
				pGrpCCB->setTransID(M_NOTUSE_TRANSID);
			}
			else
			{
				pGrpCCB->setTransID(pData->sigPayload.GrpHandoverRsp.transID);
			}
			//����������
			pGrpCCB->setGID(GID);
			pGrpCCBTbl->AddBTreeGID(pGrpCCB->getGID(), pGrpCCB->getTabIndex());

			pGrpCCB->SetCurrentState(GRP_WORKING_STATE);
			LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GID[0x%04X] Enter state[GRP_WORKING_STATE]", pGrpCCB->getGID());
			//����ҵ������
			pGrpCCB->setGID(GID);						//UINT16 GID;			//GID	2	M		
			pGrpCCB->setGrpL3Addr(grpL3Addr);//UINT32 GrpL3Addr;	//L3Addr	4	M		��L3
			#if 0
			pGrpCCB->setGrpSize(ntohs(pData->sigPayload.GrpHandoverRsp.GrpSize));	//UINT16 GrpSize;		//Group size	2	M		���Ա�ڸû�վ����Ŀ
			pGrpCCB->setCommType(pData->sigPayload.GrpHandoverRsp.commType);	//UINT32 CommType;	//Communication Type	1	M		ͨ������
			pGrpCCB->setPrioty(pData->sigPayload.GrpHandoverRsp.CallPrioty);			//UINT8 CallPrioty;		//Call priority	1	M		��������Ȩ
			pGrpCCB->setEncryptFlag(pData->sigPayload.GrpHandoverRsp.EncryptFlag);	//UINT8 EncryptFlag;	//Encryption Flag	1	M		�˵��˼���
			#endif
			//����������
			pGrpCCBTbl->AddBTreeGrpL3addr(pGrpCCB->getGrpL3Addr(), pGrpCCB->getTabIndex());
			pGrpCCB->airRes.setResReason(ResReason_LePaging);
			pGrpCCB->airRes.setResReadyFlag(false);

			pGrpCCB->startGrpTimer(TIMER_GrpDataDetect);
			pGrpCCB->startGrpTimer(TIMER_StautsReport);
		}
		else
		{
			//GrpVersion003 begin
			//�п���grpL3Addr�����仯
			if(pGrpCCB->getGrpL3Addr() != grpL3Addr)//20090401
			{
				pGrpCCBTbl->DelBTreeGrpL3addr(pGrpCCB->getGrpL3Addr());
				pGrpCCB->setGrpL3Addr(grpL3Addr);
				pGrpCCBTbl->AddBTreeGrpL3addr(pGrpCCB->getGrpL3Addr(), pGrpCCB->getTabIndex());
			}
			//GrpVersion003 end
		}
		
		//GrpVersion003 begin
		if(0==PID)
		{
			//����Ҫ������տ���Դ����ǰ����Դ����ǰ�����,20090401
			return;
		}
		//GrpVersion003 end
		
		//��L2������Դ
		pGrpCCB->airRes.setResReason(ResReason_GrpHandover);
		pGrpCCB->deleteGrpTimer(TIMER_GrpPagingRsp);
		pGrpCCB->deleteGrpTimer(TIMER_LePagingRsp);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
		ENUM_GrpResOptType opType = isVideoUT(UID) ? GET_GRPRES_VIDEOUT : GET_GRPRES;
#else
		ENUM_GrpResOptType opType = GET_GRPRES;
#endif
		pGrpCCB->sendGrpResReq2L2(opType, NEED_RSP, 
			ResReason_GrpHandover, pGrpCCB->getTransID(), 
			NEED_REPORTID, PID, bsID,DEFAULT_CID);
		pGrpCCB->deleteGrpTimer(TIMER_ResClear);
		pGrpCCB->startGrpTimer(TIMER_ResClear);
		pGrpCCB->markGrpNotEmpty();//fengbing 20091026 �޸�״̬�����ж����Ƿ�Ϊ�յ�ȱ��
	}
	else
	{
		//���͸�cpe֪ͨʧ��
		sendGrpHandoverRsp2otherBTS(msg);
	}
//GrpVersion003 end
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	unRegisterVideoUT(UID);
#endif
}

void VoiceFSM::handle_GrpResRsp_frmSXC(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pData->sigPayload.GrpResRsp.GID);
	UINT32 grpL3Addr = VGetU32BitVal(pData->sigPayload.GrpResRsp.GrpL3Addr);
	UINT32 PID = VGetU32BitVal(pData->sigPayload.GrpResRsp.PID);
	UINT32 UID = VGetU32BitVal(pData->sigPayload.GrpResRsp.UID);
	LOG5(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GroupResourceRsp<----SXC; GID[0x%04X] grpL3Addr[0x%08X] Result[0x%02X] UID[0x%08X] PID[0x%08X]",
		GID, grpL3Addr, pData->sigPayload.GrpResRsp.Cause, UID, PID);
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(GID);
//GrpVersion003 begin
	if(M_SABIS_SUCCESS==pData->sigPayload.GrpResRsp.Cause)
	{
		if(NULL==pGrpCCB)
		{
			pGrpCCB = pGrpCCBTbl->AllocGrpCCB();
			if(NULL==pGrpCCB)
			{
				LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot Alloc GrpCCB!!!");
				sendGrpResRsp2CPE(msg, false, 9);
				return;
			}
			LOG2(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"ALLOC  GrpCCB;GID[0x%04X] grpL3Addr[0x%08X]  ",
				GID, grpL3Addr);
			//����transID
			if(sizeof(GrpResRspT)> VGetU16BitVal(pData->sigHeader.Length))
			{
				//���ݾɵ�sac�汾���п���û��transID
				pGrpCCB->setTransID(M_NOTUSE_TRANSID);
			}
			else
			{
				pGrpCCB->setTransID(pData->sigPayload.GrpResRsp.transID);
			}
			//����������
			pGrpCCB->setGID(GID);
			pGrpCCBTbl->AddBTreeGID(pGrpCCB->getGID(), pGrpCCB->getTabIndex());

			pGrpCCB->SetCurrentState(GRP_WORKING_STATE);
			LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GID[0x%04X] Enter state[GRP_WORKING_STATE]", pGrpCCB->getGID());
			//����ҵ������
			pGrpCCB->setGID(GID);						//UINT16 GID;			//GID	2	M		
			pGrpCCB->setGrpL3Addr(grpL3Addr);//UINT32 GrpL3Addr;	//L3Addr	4	M		��L3
#ifdef M__SUPPORT__ENC_RYP_TION
			UINT8 encryptFlag = pData->sigPayload.GrpResRsp.EncryptFlag;
			pGrpCCB->setEncryptFlag(encryptFlag);
			//���¼�������
			if(encryptFlag!=ENCRYPT_CTRL_NOTUSE)
			{
				pGrpCCB->setEncryptKey(&pData->sigPayload.GrpResRsp.EncryptFlag+1);
			}				
#endif
			#if 0
			pGrpCCB->setGrpSize(ntohs(pData->sigPayload.GrpResRsp.GrpSize));	//UINT16 GrpSize;		//Group size	2	M		���Ա�ڸû�վ����Ŀ
			pGrpCCB->setCommType(pData->sigPayload.GrpResRsp.commType);	//UINT32 CommType;	//Communication Type	1	M		ͨ������
			pGrpCCB->setPrioty(pData->sigPayload.GrpResRsp.CallPrioty);			//UINT8 CallPrioty;		//Call priority	1	M		��������Ȩ
			pGrpCCB->setEncryptFlag(pData->sigPayload.GrpResRsp.EncryptFlag);	//UINT8 EncryptFlag;	//Encryption Flag	1	M		�˵��˼���
			#endif
			//����������
			pGrpCCBTbl->AddBTreeGrpL3addr(pGrpCCB->getGrpL3Addr(), pGrpCCB->getTabIndex());
			pGrpCCB->airRes.setResReason(ResReason_LePaging);
			pGrpCCB->airRes.setResReadyFlag(false);

			pGrpCCB->startGrpTimer(TIMER_GrpDataDetect);
			pGrpCCB->startGrpTimer(TIMER_StautsReport);
		}
		else
		{
			//GrpVersion003 begin
			//�п���grpL3Addr�����仯
			if(pGrpCCB->getGrpL3Addr() != grpL3Addr)//20090401
			{
				pGrpCCBTbl->DelBTreeGrpL3addr(pGrpCCB->getGrpL3Addr());
				pGrpCCB->setGrpL3Addr(grpL3Addr);
				pGrpCCBTbl->AddBTreeGrpL3addr(pGrpCCB->getGrpL3Addr(), pGrpCCB->getTabIndex());
			}
			//GrpVersion003 end
		}

		//GrpVersion003 begin
		if(0==PID)
		{
			//����Ҫ������տ���Դ����ǰ����Դ����ǰ�����,20090401
			return;
		}
		//GrpVersion003 end
		
		//��L2������Դ
		pGrpCCB->airRes.setResReason(ResReason_GrpCpeRequest);
		pGrpCCB->deleteGrpTimer(TIMER_GrpPagingRsp);
		pGrpCCB->deleteGrpTimer(TIMER_LePagingRsp);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
		ENUM_GrpResOptType opType = isVideoUT(UID) ? GET_GRPRES_VIDEOUT : GET_GRPRES;
#else
		ENUM_GrpResOptType opType = GET_GRPRES;
#endif
		pGrpCCB->sendGrpResReq2L2(opType, NEED_RSP, 
			ResReason_GrpCpeRequest, pGrpCCB->getTransID(), 
			NEED_REPORTID, PID, 
			M_INVALID_BTSID,DEFAULT_CID);
		pGrpCCB->deleteGrpTimer(TIMER_ResClear);
		pGrpCCB->startGrpTimer(TIMER_ResClear);
		pGrpCCB->markGrpNotEmpty();//fengbing 20091026 �޸�״̬�����ж����Ƿ�Ϊ�յ�ȱ��
	}
	else
	{
		//���͸�cpe֪ͨʧ��
		sendGrpResRsp2CPE(msg);
	}
//GrpVersion003 end
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	unRegisterVideoUT(UID);
#endif
}

void VoiceFSM::handle_ErrorNotify_frmSXC(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT32 UID = VGetU32BitVal(pData->sigPayload.ErrNotifyReq.Uid);
	LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"ErrorNotifyReq<----SXC; UID[0x%08X] ", UID);
	
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByUID(UID);
	if(NULL!=pCCB)
	{
		if(VOICE_T_ESTABLISH_STATE==pCCB->GetCurrentState())
		{
			//������ǰ�����̣���״̬��
			return;
		}
		if(VOICE_IDLE_STATE!=pCCB->GetCurrentState())
		{
			pCCB->VACRelease();
			pCCB->deleteTimer();
			pCCB->deleteTimerWaitSYNC();

			//release voice data buffer 
			deallocVDataBuf(pCCB->m_vDataIdx);
			pCCB->m_vDataIdx = M_INVALID_VDATABUF_IDX;
			//L3 Addr ����
			VoiceCCBTable* pCCBTab = pCCB->getCCBTable();
			pCCBTab->DelBTreeL3addr(pCCB->getL3addr());
			pCCB->setL3addr(NO_L3ADDR);	
//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
			pCCB->disableInnerSwitch();
#endif
//20090531 fengbing bts inner switch for Voice Data end

			pCCB->setAppType(APPTYPE_VOICE_G729);
			pCCB->setCodecInfo(CODEC_G729A);
			pCCB->setAppPrio(M_DEFAULT_VOICE_PRIORITY);	//Ŀǰ�汾�������ȼ��̶�Ϊ
			
			pCCB->clearConnectedFlag();

			LOG2(LOG_DEBUG, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"CCB[UID=0x%08X][%d] recv ErrorNotifyMsg, Enter [VOICE_IDLE_STATE] state", 
				pCCB->getUID(), pCCB->GetCurrentState()); 

			pCCB->SetCurrentState(VOICE_IDLE_STATE);
		}
	}
	else
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Cannot find VoiceCCB!!!");
	}
}

void VoiceFSM::handle_GrpL3Addr_Signal_frmSXC(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pDataVCR = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT32 grpL3Addr = VGetU32BitVal(pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.GrpL3Addr);
	LOG2(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"UTSXC_GrpL3Addr_Signal<----SXC; grpL3Addr[0x%08X] msgType[0x%02X]",
		grpL3Addr, pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.msgType);
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGrpL3addr(grpL3Addr);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "Cannot find GrpCCB with GrpL3Addr!!!");
		if(pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.msgType==M_MSGTYPE_GRP_RLS)
		{
			typedef struct 
			{
				UINT8 GrpL3Addr[4];
				UINT8 msgType;
				UINT8 relCause;
				UINT8 Detail[4];	
				UINT8 GID[2];
			}tmpGrpRlsT;

			UINT16 len = VGetU16BitVal(pDataVCR->sigHeader.Length);
			UINT16 gid;
			if(len>=sizeof(tmpGrpRlsT))
			{
				tmpGrpRlsT *pGrpRlsData = (tmpGrpRlsT*)pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.GrpL3Addr;
				gid = VGetU16BitVal(pGrpRlsData->GID);
				pGrpCCB = pGrpCCBTbl->FindCCBByGID(gid);
				LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"find GrpCCB with GID[0x%04X] for GrpRelease...", gid);
			}
			else
			{
				LOG(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"GrpRlease without GID!!!");
			}

			if(!pGrpCCB)
			{
				LOG1(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
					"cannot find GrpCCB with GID[0x%04X] for GrpRelease!!!", gid);
				return;
			}
			LOG1(LOG_DEBUG1, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"GrpCCB with GID[0x%04X] found for GrpRelease.", gid);
		}
		else
		{
			return;
		}
	}
	//sendto btsL2
	pGrpCCB->sendGrpSignal2L2(&pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.msgType, 
			msg.GetDataLength() - sizeof(SigHeaderT) - 
			sizeof(pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.GrpL3Addr));
	//is call release
	if(M_MSGTYPE_GRP_RLS==pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.msgType)
	{
		LOG2(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"Group Release<----SXC;GID[0x%04X] grpL3Addr[0x%08X]************", 
			pGrpCCB->getGID(), pGrpCCB->getGrpL3Addr());
		pGrpCCB->clearAllTimers();
		pGrpCCB->startGrpTimer(TIMER_GrpRls);
		pGrpCCB->SetCurrentState(GRP_RELEASE_STATE);
		LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GID[0x%04X] Enter state[GRP_RELEASE_STATE]", pGrpCCB->getGID());
	}	
}

void VoiceFSM::handle_GrpUID_Signal_frmSXC(CMessage& msg)
{
	CMsg_UTSAGSignal_DAC MsgToCpe_DAC;
	VoiceVCRCtrlMsgT *pDataVCR = (VoiceVCRCtrlMsgT*)msg.GetDataPtr(); 
	UINT16 nUTSAGPayloadLen = msg.GetDataLength() - sizeof(SigHeaderT) - sizeof(UINT32);
	UINT32 Uid = VGetU32BitVal(pDataVCR->sigPayload.UTSXC_Payload_GrpUid.GrpUid);
	LOG2(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"UTSXC_GrpUID_Signal<----SXC; UID[0x%08X] msgType[0x%02X]",
		Uid, 	pDataVCR->sigPayload.UTSXC_Payload_GrpUid.msgType);	
	VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByUID(Uid);
	if(NULL==pCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find VoiceCCB!!!");
		return;
	}
	
	//������Ϣ
	if ( !MsgToCpe_DAC.CreateMessage(*CTVoice::GetInstance(), nUTSAGPayloadLen+sizeof(UINT8)) )
   	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
    
	MsgToCpe_DAC.SetMessageId(MSGID_GRP_DAC_DL_UID_SIGNAL);
	MsgToCpe_DAC.SetDstTid(M_TID_UTV);
	//�����,��Message Type��ʼ
	MsgToCpe_DAC.SetSignalPayload(&(pDataVCR->sigPayload.UTSXC_Payload_GrpUid.msgType), nUTSAGPayloadLen);
	//����ͷ
	MsgToCpe_DAC.SetEID(pCCB->getVoiceTuple().Eid);
	MsgToCpe_DAC.SetCID(pCCB->getVoiceTuple().Cid);
	MsgToCpe_DAC.SetPayloadLength(nUTSAGPayloadLen+sizeof(UINT8));
	
	if(MsgToCpe_DAC.Post())
	{
		Counters.nSigToDAC++;
		LOG4(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"UTSXC_GrpUID_Signal---->CPE; UID[0x%08X] EID[0x%08X] CID[0x%02X] msgType[0x%02X]",
			Uid, 	pCCB->getVoiceTuple().Eid, pCCB->getVoiceTuple().Cid,
			pDataVCR->sigPayload.UTSXC_Payload_GrpUid.msgType);
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Send UTSXC_GrpUID_Signal to CPE fail!!!");
	}
}

//from CPE
void VoiceFSM::handle_GrpL3Addr_signal_frmCPE(CMessage& msg)
{
	Counters.nSigFromDAC++;
	GrpCCB *pGrpCCB;

	CMsg_Signal_VCR signalMsgVCR;
	VoiceVCRCtrlMsgT* pDataVCR;

	CPEGrpL3AddrSignalT* pDataCPE = (CPEGrpL3AddrSignalT*)msg.GetDataPtr();
	UINT16 nUTSAGSigPayloadLen=msg.GetDataLength()-sizeof(pDataCPE->cid) -sizeof(pDataCPE->GID);
	UINT16 tmpGID = VGetU16BitVal(pDataCPE->GID);

	LOG4(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"UTSXC_GrpL3Addr_Signal<----CPE; GID[0x%04X] EID[0x%08X] CID[0x%02X] msgType[0x%02X]",
		tmpGID, msg.GetEID(), pDataCPE->cid, pDataCPE->msgType);
	pGrpCCB = pGrpCCBTbl->FindCCBByGID(tmpGID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB!!!");
		return;
	}
	if ( !signalMsgVCR.CreateMessage(*CTVoice::GetInstance(), nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
    
	signalMsgVCR.SetDstTid(M_TID_VCR);
	signalMsgVCR.SetMessageId(MSGID_VOICE_VCR_SIGNAL);

	pDataVCR = (VoiceVCRCtrlMsgT*)signalMsgVCR.GetDataPtr();
	signalMsgVCR.SetBTSSAGID();		//SAGID, BTSID
	//EVENT GROUP ID, EVENT ID(net order)
	pDataVCR->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_UTSAG;
	VSetU16BitVal(pDataVCR->sigHeader.Event_ID , M_MSG_EVENT_ID_GRP_UTSAG_L3ADDR);
	//����ͨ��DAC͸�����������GrpL3Addr
	VSetU32BitVal(pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.GrpL3Addr , pGrpCCB->getGrpL3Addr());
	//msgtype,
	pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.msgType = pDataCPE->msgType;
	//payload, 
	memcpy((void*)&(pDataVCR->sigPayload.UTSXC_Payload_GrpL3Addr.UTSAGPayload),
	       (void*)&(pDataCPE->payload),
		   nUTSAGSigPayloadLen-1);

	//length, ���㳤�������͸����GrpL3Addr�ĳ���
	signalMsgVCR.SetSigHeaderLengthField(nUTSAGSigPayloadLen+sizeof(UINT32));
	signalMsgVCR.SetPayloadLength(nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT));
	//transfer message to SAG
	if(signalMsgVCR.Post())
	{
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"UTSXC_GrpL3Addr_Signal---->SXC; GID[0x%04X] grpL3Addr[0x%08X] EID[0x%08X] CID[0x%02X] msgType[0x%02X]",
			tmpGID, pGrpCCB->getGrpL3Addr(), 
			msg.GetEID(), pDataCPE->cid, pDataCPE->msgType);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Send UTSXC_GrpL3Addr_Signal to SXC fail!!!");
	}
}

void VoiceFSM::handle_GrpUID_signal_frmCPE(CMessage& msg)
{
	Counters.nSigFromDAC++;
	VoiceCCB *pCCB;
	VoiceTuple tuple;

	CMsg_Signal_VCR signalMsgVCR;
	VoiceVCRCtrlMsgT* pDataVCR;
	CMsg_UTSAGSignal_DAC signalMsgDAC(msg);

	tuple.Eid = msg.GetEID();
	tuple.Cid = signalMsgDAC.GetCID();
	CPEGrpUIDSignalT *pDataCPE = (CPEGrpUIDSignalT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pDataCPE->payload.GID);

	LOG4(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"UTSXC_GrpUID_Signal<----CPE; GID[0x%04X] EID[0x%08X] CID[0x%02X] msgType[0x%02X]",
		GID, msg.GetEID(), signalMsgDAC.GetCID(), pDataCPE->msgType);
	
	pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(pCCB==NULL)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find CCB");
		return;
	}

	UINT8* pDACSigPayload=NULL;
	UINT16 nUTSAGSigPayloadLen=0;
	signalMsgDAC.GetSignalPayload(pDACSigPayload, nUTSAGSigPayloadLen);
	
	if ( !signalMsgVCR.CreateMessage(*CTVoice::GetInstance(), 
		nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
    
	signalMsgVCR.SetDstTid(M_TID_VCR);
	signalMsgVCR.SetMessageId(MSGID_VOICE_VCR_SIGNAL);

	pDataVCR = (VoiceVCRCtrlMsgT*)signalMsgVCR.GetDataPtr();
	signalMsgVCR.SetBTSSAGID();		//SAGID, BTSID
	//EVENT GROUP ID, EVENT ID(net order)
	pDataVCR->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_UTSAG;
	VSetU16BitVal(pDataVCR->sigHeader.Event_ID , M_MSG_EVENT_ID_GRP_UTSAG_UID);
	//ͨ��DAC͸�����������UID
	VSetU32BitVal(pDataVCR->sigPayload.UTSXC_Payload_GrpUid.GrpUid, pCCB->getUID());

	//payload, 
	memcpy((void*)&(pDataVCR->sigPayload.UTSXC_Payload_GrpUid.msgType),
	       (void*)(pDACSigPayload),
		   nUTSAGSigPayloadLen);

	//length, ���㳤�������͸����UID�ĳ���
	signalMsgVCR.SetSigHeaderLengthField(nUTSAGSigPayloadLen+sizeof(UINT32));
	signalMsgVCR.SetPayloadLength(nUTSAGSigPayloadLen+sizeof(UINT32)+sizeof(SigHeaderT));
	//transfer message to SAG
	if(signalMsgVCR.Post())
	{
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"UTSXC_GrpUID_Signal---->SXC; GID[0x%04X] UID[0x%08X] EID[0x%08X] CID[0x%02X] msgType[0x%02X]",
			GID, pCCB->getUID(), msg.GetEID(), 
			signalMsgDAC.GetCID(), pDataCPE->msgType);	
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"Send UTSXC_GrpUID_Signal to SXC fail!!!");
	}
}

void VoiceFSM::handle_HoResReq_frmCPE(CMessage& msg)
{
	UINT32 selfBTSID = bspGetBtsID();
	CPE_HoResReqT* pData = (CPE_HoResReqT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pData->GID);
	UINT32 UID = VGetU32BitVal(pData->UID);
	UINT32 PID = msg.GetEID();
	UINT32 tgtBTSID = VGetU32BitVal(pData->btsID);
	LOG5(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"HoResReq<----CPE; GID[0x%04X] UID[0x%08X] EID[0x%08X] CID[0x%02X] targetBTSID[0x%08X]",
		GID, UID, PID, pData->cid, tgtBTSID);
	if(selfBTSID==tgtBTSID)
	{
		LOG(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"HoResReq tgtBTSID==curBTSID, discard!!!");
		return;
	}
	
	msg.SetMessageId(MSGID_GRP_HO_RES_REQ);
	msg.SetSrcTid(M_TID_VOICE);
	msg.SetDstTid(M_TID_EMSAGENTTX);
	msg.SetBTS(tgtBTSID);
	//��Ϊ�Լ���btsID������Ϣ��Ӧ������bts����
	VSetU32BitVal(pData->btsID , selfBTSID);	
	if(!msg.Post())
	{
		msg.DeleteMessage();
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"Send HoResReq to other BTS fail!!!");
	}
	else
	{
		Counters.nSigToOtherBTS++;
		LOG5(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"HoResReq---->BTS[0x%08X]; GID[0x%04X] UID[0x%08X] EID[0x%08X] CID[0x%02X] ",
			tgtBTSID, GID, UID, PID, pData->cid);
	}
}

bool VoiceFSM::sendGrpResReq2SXC(CMessage& cpeGrpResReq)
{
	CMsg_Signal_VCR SAbisSignal;
	VoiceVCRCtrlMsgT *pData;
	CPEGrpResReqT* pDataCPE; 
	if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpResReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		pDataCPE = (CPEGrpResReqT*)cpeGrpResReq.GetDataPtr();
		pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
		SAbisSignal.SetDstTid(M_TID_VCR);
		SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		SAbisSignal.SetBTSSAGID();
		SAbisSignal.SetSigIDS(GrpResReq_MSG);

		UINT16 GID = VGetU16BitVal(pDataCPE->GID);
		UINT32 UID = VGetU32BitVal(pDataCPE->UID);
		UINT32 PID = VGetU32BitVal(pDataCPE->EID);
		
		VSetU32BitVal(pData->sigPayload.GrpResReq.UID , UID);
		VSetU16BitVal(pData->sigPayload.GrpResReq.GID , GID);
		VSetU32BitVal(pData->sigPayload.GrpResReq.PID , PID);
		if(sizeof(CPEGrpResReqT) > cpeGrpResReq.GetDataLength())
		{
			//Grp002,cpeGrpResReq��Ϣδ����commType�ֶ�,����Ĭ��ֵ
			pData->sigPayload.GrpResReq.commType = COMM_TYPE_VOICE_GRPCALL;
		}
		else
		{
			//Grp003
			pData->sigPayload.GrpResReq.commType = pDataCPE->CommType;			
		}
		SAbisSignal.SetSigHeaderLengthField(sizeof(GrpResReqT));
		SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpResReqT));
		if(SAbisSignal.Post())
		{
			LOG4(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpResReq---->SXC, GID[0x%04X]  commType[0x%02X] UID[0x%08X] PID[0x%08X]", 
				GID, pData->sigPayload.GrpResReq.commType, UID, PID);
			return true;
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send msg to VCR failed!!!");
			return false;
		}
	}	
}

bool VoiceFSM::sendGrpHandoverReq2SXC(CMessage& cpeHandoverReq)
{
	CMsg_Signal_VCR SAbisSignal;
	VoiceVCRCtrlMsgT *pData;
	BTS_HoResReqT* pDataBTS; 
	if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpHandoverReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		pDataBTS = (BTS_HoResReqT*) cpeHandoverReq.GetDataPtr();
		pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
		SAbisSignal.SetDstTid(M_TID_VCR);
		SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		SAbisSignal.SetBTSSAGID();
		SAbisSignal.SetSigIDS(GrpHandoverReq_MSG);

		memcpy(pData->sigPayload.GrpHandoverReq.GID , pDataBTS->GID, 2);
		memcpy(pData->sigPayload.GrpHandoverReq.UID , pDataBTS->UID, 4);
		pData->sigPayload.GrpHandoverReq.HO_Type = HO_TYPE_CONVERSATION;
		pData->sigPayload.GrpHandoverReq.VersionInfo[0] = M_TERMINAL_TYPE_GRPCPE;
		pData->sigPayload.GrpHandoverReq.VersionInfo[1] = M_SWVERSION_V52PLUS;
		memcpy(pData->sigPayload.GrpHandoverReq.PID , pDataBTS->EID, 4);
		memcpy(pData->sigPayload.GrpHandoverReq.curBTSID , pDataBTS->btsID, 4);

		SAbisSignal.SetSigHeaderLengthField(sizeof(GrpHandoverReqT));
		SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpHandoverReqT));
		if(SAbisSignal.Post())
		{
			LOG2(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpHandoverReq---->SXC, GID[0x%04X]  UID[0x%08X]", 
				VGetU16BitVal(pDataBTS->GID), VGetU32BitVal(pDataBTS->UID));
			return true;
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send msg to VCR failed!!!");
			return false;
		}
	}	
}

//���SXC�ظ�ʧ�ܵ�GrpResRsp�����ն˻�Ӧ�л���ȡ��Դ����ʧ�ܵ���Ϣ
bool VoiceFSM::sendGrpHandoverRsp2otherBTS(CMessage& sxcGrpHandoverRsp, bool blUSeSXCResult, UINT8 result)
{
	CComMessage* pComMsg = new(CTVoice::GetInstance(),sizeof(BTSHoResRspT)) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	VoiceVCRCtrlMsgT* pDataSXC = (VoiceVCRCtrlMsgT*)sxcGrpHandoverRsp.GetDataPtr();
	BTSHoResRspT* pDataBTS = (BTSHoResRspT*)pComMsg->GetDataPtr();
	pDataBTS->cid = DEFAULT_CID;
	memcpy(pDataBTS->btsID , pDataSXC->sigPayload.GrpHandoverRsp.curBTSID, 4);
	memcpy(pDataBTS->GID , pDataSXC->sigPayload.GrpHandoverRsp.GID, 2);
	memcpy(pDataBTS->EID , pDataSXC->sigPayload.GrpHandoverRsp.PID, 4);
	pDataBTS->Result = (blUSeSXCResult ? pDataSXC->sigPayload.GrpHandoverRsp.Result : result);
	#if 0
	pDataBTS->reportID = pDataL2L3->ReportID;
	pDataBTS->transID = pDataL2L3->transID;
	pDataBTS->reportID_Ind = pDataL2L3->ReportIndexFlag;
	pDataBTS->airRes = pDataL2L3->airRes;	
	#endif
	pComMsg->SetDataLength(sizeof(BTSHoResRspT));
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_EMSAGENTTX);
	pComMsg->SetMessageId(MSGID_GRP_HO_RES_RSP);
	pComMsg->SetBTS(VGetU32BitVal(pDataSXC->sigPayload.GrpHandoverRsp.curBTSID));
	bool ret =  postComMsg(pComMsg);
	if(ret)
	{
		Counters.nSigToOtherBTS++;
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"HoResRsp---->ohterBTS, GID[0x%04X] Result[0x%02X] btsID[0x%08X] EID[0x%08X] cid[0x%02X]", 
			VGetU16BitVal(pDataBTS->GID), pDataBTS->Result, 
			VGetU32BitVal(pDataBTS->btsID), VGetU32BitVal(pDataBTS->EID), pDataBTS->cid);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send HoResRsp msg to other BTS failed!!!");
	}
	return ret;	
}

//���SXC�ظ�ʧ�ܵ�GrpResRsp�����ն˻�Ӧ��ȡ��Դ����ʧ�ܵ���Ϣ
bool VoiceFSM::sendGrpResRsp2CPE(CMessage& sxcGrpResRsp, bool blUSeSXCResult, UINT8 result)
{
	CComMessage* pComMsg = new(CTVoice::GetInstance(),sizeof(CPEGrpResRspT)) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	CPEGrpResRspT* pDataCPE = (CPEGrpResRspT*)pComMsg->GetDataPtr();
	VoiceVCRCtrlMsgT* pDataSXC = (VoiceVCRCtrlMsgT*)sxcGrpResRsp.GetDataPtr();

	memset((void*)pDataCPE, 0, sizeof(CPEGrpResRspT));
	pDataCPE->cid = DEFAULT_CID;
	memcpy(pDataCPE->GID , pDataSXC->sigPayload.GrpResRsp.GID, 2);
	pDataCPE->Result = ( blUSeSXCResult ? pDataSXC->sigPayload.GrpResRsp.Cause : result );
	
	pComMsg->SetEID(VGetU32BitVal(pDataSXC->sigPayload.GrpResRsp.PID));
	pComMsg->SetDataLength(sizeof(CPEGrpResRspT));
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_UTV);
	pComMsg->SetMessageId(MSGID_GRP_DAC_RES_RSP);
	bool ret = postComMsg(pComMsg);
	if(ret)
	{
		Counters.nSigToDAC++;
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GrpShareResRsp---->CPE, GID[0x%04X] Result[0x%02X] reportID[0x%04X] EID[0x%08X] cid[0x%02X]", 
			VGetU16BitVal(pDataCPE->GID), pDataCPE->Result, 
			VGetU16BitVal(pDataCPE->reportID), 
			VGetU32BitVal(pDataSXC->sigPayload.GrpResRsp.PID), pDataCPE->cid);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send GrpShareResRsp msg to CPE failed!!!");
	}
	return ret;	
}

void VoiceFSM::handle_GrpResReq_frmCPE(CMessage& msg)
{
	CPEGrpResReqT* pData = (CPEGrpResReqT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pData->GID);
	UINT32 UID = VGetU32BitVal(pData->UID);
	UINT32 PID = VGetU32BitVal(pData->EID);
	LOG4(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GrpResReq<----CPE; GID[0x%04X] UID[0x%08X] EID[0x%08X] CID[0x%02X] ",
		GID, UID, PID, pData->cid);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	bool blIsVideoUT = false;
	if((&pData->ReqType-&pData->cid)<msg.GetDataLength())
	{
		blIsVideoUT = pData->ReqType==VIDEO_UT ? true : false;
	}
#endif	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(GID);
//GrpVersion003 begin
	if(NULL==pGrpCCB || 
		M_INVALID_GRPL3ADDR==pGrpCCB->getGrpL3Addr())
	{
		//�鲻���ڻ�grpL3Addr��Чʱ����SXC����GrpResReq����SXC�ж��Ƿ���Ҫ����Դ
		if(NULL==pGrpCCB)
		{
			LOG(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpResReq received from CPE when Grp Not Exist.");
		}
		else
		{
			LOG(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpResReq received from CPE when GrpL3Addr Not Exist.");
		}
		sendGrpResReq2SXC(msg);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
		if(blIsVideoUT)//fengbing 20091229
		{
			registerVideoUT(UID);
		}
		else
		{
			unRegisterVideoUT(UID);
		}
#endif
		return;
	}
//GrpVersion003 end

	pGrpCCB->airRes.setResReason(ResReason_GrpCpeRequest);
	pGrpCCB->deleteGrpTimer(TIMER_GrpPagingRsp);
	pGrpCCB->deleteGrpTimer(TIMER_LePagingRsp);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	ENUM_GrpResOptType opType = blIsVideoUT ? GET_GRPRES_VIDEOUT : GET_GRPRES;
#else
	ENUM_GrpResOptType opType = GET_GRPRES;
#endif
	pGrpCCB->sendGrpResReq2L2(opType, NEED_RSP, 
		ResReason_GrpCpeRequest, pGrpCCB->getTransID(), 
		NEED_REPORTID, PID, M_INVALID_BTSID,pData->cid);
	if(GRP_PAGING_STATE==pGrpCCB->GetCurrentState())
	{
		pGrpCCB->sendGrpPagingRsp2SXC(M_SABIS_SUCCESS, UID);
		pGrpCCB->SetCurrentState(GRP_WORKING_STATE);
		LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GID[0x%04X] Enter state[GRP_WORKING_STATE]", pGrpCCB->getGID());		
	}
	else
	{
		//GrpVersion003 begin
		//��ʱ��SXC����PID==0��GrpResReq��ֻ�Ǳ�֤SAG��bts������,20090401
		pGrpCCB->sendGrpResReq2SXC(UID, 0);//pData->EID);
		//GrpVersion003 end
	}
	pGrpCCB->deleteGrpTimer(TIMER_ResClear);
	pGrpCCB->startGrpTimer(TIMER_ResClear);
	pGrpCCB->markGrpNotEmpty();//fengbing 20091026 �޸�״̬�����ж����Ƿ�Ϊ�յ�ȱ��
}

//from other BTS
void VoiceFSM::handle_HoResReq_frmOtherBTS(CMessage& msg)
{
	Counters.nSigFromOtherBTS++;
	BTS_HoResReqT* pDataBTS = (BTS_HoResReqT*) msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pDataBTS->GID);
	UINT32 UID = VGetU32BitVal(pDataBTS->UID);
	UINT32 PID = VGetU32BitVal(pDataBTS->EID);
	UINT32 tmpBTSID = VGetU32BitVal(pDataBTS->btsID);
	LOG5(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"HoResReq<----BTS[0x%08X]; GID[0x%04X] UID[0x%08X] EID[0x%08X] CID[0x%02X] ",
		tmpBTSID, GID, UID, PID, pDataBTS->cid);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	bool blIsVideoUT = false;
	if((&pDataBTS->ReqType-&pDataBTS->cid)<msg.GetDataLength())
	{
		blIsVideoUT = pDataBTS->ReqType==VIDEO_UT ? true : false;
	}
#endif
#if 0	
	if(pDataBTS->btsID!=bspGetBtsID())
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "handle_HoResReq_frmOtherBTS, invalid BTSID!!!");
		return;
	}
#endif    
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(GID);
//GrpVersion003 begin
	if(NULL==pGrpCCB || 
		M_INVALID_GRPL3ADDR==pGrpCCB->getGrpL3Addr())
	{
		//�鲻���ڻ�grpL3Addr��Чʱ����SXC����GrpHandoverReq����SXC�ж��Ƿ���Ҫ����Դ
		if(NULL==pGrpCCB)
		{
			LOG(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpHandoverReq received from other BTS when Grp Not Exist.");
		}
		else
		{
			LOG(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpHandoverReq received from other BTS when GrpL3Addr Not Exist.");
		}
		sendGrpHandoverReq2SXC(msg);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
		if(blIsVideoUT)//fengbing 20091229
		{
			registerVideoUT(UID);
		}
		else
		{
			unRegisterVideoUT(UID);
		}
#endif
		return;
	}
//GrpVersion003 end
	
	//�����״�Ѱ����ʱ�ͷ���Դ
	pGrpCCB->deleteGrpTimer(TIMER_GrpPagingRsp);
	pGrpCCB->airRes.setResReason(ResReason_GrpHandover);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	ENUM_GrpResOptType opType = blIsVideoUT ? GET_GRPRES_VIDEOUT : GET_GRPRES;
#else
	ENUM_GrpResOptType opType = GET_GRPRES;
#endif
	pGrpCCB->sendGrpResReq2L2(opType, NEED_RSP, 
		ResReason_GrpHandover, pGrpCCB->getTransID(), 
		NEED_REPORTID, PID, tmpBTSID, pDataBTS->cid);		

	//GrpVersion003 begin
	//��ʱ��SXC����PID==0��GrpHandoverReq,ֻ�Ǳ�֤SAG��bts������,20090401
	pGrpCCB->sendGrpHandoverReq2SXC(UID, 0, tmpBTSID);
	//GrpVersion003 end
	
	pGrpCCB->deleteGrpTimer(TIMER_ResClear);
	pGrpCCB->startGrpTimer(TIMER_ResClear);
	pGrpCCB->markGrpNotEmpty();//fengbing 20091026 �޸�״̬�����ж����Ƿ�Ϊ�յ�ȱ��
}

void VoiceFSM::handle_HoResRsp_frmOtherBTS(CMessage& msg)
{
	Counters.nSigFromOtherBTS++;
	BTSHoResRspT* pData = (BTSHoResRspT*)msg.GetDataPtr();
	UINT16 GID = VGetU16BitVal(pData->GID);
	LOG5(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"HoResRsp<----BTS[0x%08X]; GID[0x%04X] Result[0x%02X] EID[0x%08X] CID[0x%02X] ",
		VGetU32BitVal(pData->btsID), GID, pData->Result, VGetU32BitVal(pData->EID), pData->cid);
	
	GrpCCB* pGrpCCB = (GrpCCB*)pGrpCCBTbl->FindCCBByGID(GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
	}
	else
	{
		pGrpCCB->sendHoResRsp2CPE(msg);
	}		
}

//timers
void VoiceFSM::handle_Timeout_GrpPagingRsp(CMessage& msg)
{
	TimerStructT* pData = (TimerStructT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"Timeout_GrpPagingRsp  GID[0x%04X] ", pData->GID);
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(pData->GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
		return;
	}
	pGrpCCB->deleteGrpTimer(TIMER_GrpPagingRsp);
	pGrpCCB->airRes.setResReadyFlag(false);
	pGrpCCB->SetCurrentState(GRP_WORKING_STATE);
	LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GID[0x%04X] Enter state[GRP_WORKING_STATE]", pGrpCCB->getGID());	
	pGrpCCB->sendGrpResReq2L2(FREE_GRPRES, NOT_NEED_RSP, 
		ResReason_GrpPaging, pGrpCCB->getTransID(), 
		NO_REPORTID, NO_EID, M_INVALID_BTSID);
//GrpVersion003 begin
	//��bts��û������նˣ����grpL3Addr
	pGrpCCBTbl->DelBTreeGrpL3addr(pGrpCCB->getGrpL3Addr());
	pGrpCCB->setGrpL3Addr(M_INVALID_GRPL3ADDR);
	LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GID[0x%04X] clear GrpL3Addr...", pGrpCCB->getGID());
//GrpVersion003 end
	
}

void VoiceFSM::handle_Timeout_LePagingRsp(CMessage& msg)
{
	TimerStructT* pData = (TimerStructT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"Timeout_LePagingRsp  GID[0x%04X] ", pData->GID);	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(pData->GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
		return;
	}
	pGrpCCB->deleteGrpTimer(TIMER_LePagingRsp);
	if(pGrpCCB->airRes.isResReady() && 
		ResReason_LePaging==pGrpCCB->airRes.getResReason() &&
		pGrpCCB->isGrpEmpty())
	{
		pGrpCCB->sendGrpResReq2L2(FREE_GRPRES, NOT_NEED_RSP, 
			ResReason_GrpPaging, pGrpCCB->getTransID(), 
			NO_REPORTID, NO_EID, M_INVALID_BTSID);
		pGrpCCB->airRes.setResReadyFlag(false);
	}
}

void VoiceFSM::handle_Timeout_StatusReport(CMessage& msg)
{
	//ԭ���Ͽտ��ϵ�StatusReportҪ������
	//�����޷��ж��ٺ������Ӧ��ʱ���Ƿ�Ҫ�Ƿ���Դ
	//g_blUseStatusReport�����Ƿ���SAbis�ӿ�����StatusReport
	TimerStructT* pData = (TimerStructT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"Timeout_StatusReport  GID[0x%04X] ", pData->GID);	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(pData->GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
		return;
	}	

	//StatusReport--->SXC
	pGrpCCB->sendGrpStatusReport2SXC();
	
	if(pGrpCCB->statusReport.curCounter>=M_StatusReport_CountPeriod)
	{
		pGrpCCB->statusReport.curCounter = 0;
	}
	pGrpCCB->statusReport.GrpCpesCounters[pGrpCCB->statusReport.curCounter++] = 
		pGrpCCB->statusReport.cpesCnt;
	pGrpCCB->statusReport.clearLst();
//GrpVersion003 begin
	if(!pGrpCCB->isLePagingStarted())
	{
		//û�гٺ��������Ѱ��ʱ����Դ�ͷ�
		if(pGrpCCB->airRes.isResReady() && pGrpCCB->isGrpEmpty())
		{
			LOG1(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GID[0x%04X] without LePaging has no StatusReports for a long time, now free airRes......", 
			pGrpCCB->getGID());
			pGrpCCB->sendGrpResReq2L2(FREE_GRPRES, NOT_NEED_RSP, 
				ResReason_GrpPaging, pGrpCCB->getTransID(), 
				NO_REPORTID, NO_EID, M_INVALID_BTSID);
			pGrpCCB->airRes.setResReadyFlag(false);
		}
	}	
//GrpVersion003 end
}

void VoiceFSM::handle_Timeout_LePagingLoop(CMessage& msg)
{
	TimerStructT* pData = (TimerStructT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"Timeout_LePagingLoop  GID[0x%04X] ", pData->GID);	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(pData->GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
		return;
	}
	if(pGrpCCB->airRes.isResReady())
	{
#if 1	
		if(pGrpCCB->isGrpEmpty())
		{
			UINT8 resReason = pGrpCCB->airRes.getResReason();
			if(resReason!=ResReason_GrpHandover && resReason!=ResReason_GrpCpeRequest)
			{
				pGrpCCB->airRes.setResReason(ResReason_LePaging);
			}			
			else
			{
				//do nothing, resReason�ȴ�timer_ResClear��ʱ���ΪResReason_LePaging
			}
		}
#endif		
		pGrpCCB->sendGrpPagingReq2L2(pGrpCCB->getCommType(), IS_LEPAGING, pGrpCCB->getTransID());
		pGrpCCB->startGrpTimer(TIMER_LePagingRsp);
		pGrpCCB->sendGrpCallInd2CPE(pGrpCCB->getCommType(), IS_LEPAGING, pGrpCCB->getTransID());	
	}
	else
	{
		pGrpCCB->airRes.setResReason(ResReason_LePaging);
		pGrpCCB->sendGrpResReq2L2(GET_GRPRES, NEED_RSP, 
				ResReason_LePaging, pGrpCCB->getTransID(), 
				NO_REPORTID, M_INVALID_EID, M_INVALID_BTSID);
	}
}

void VoiceFSM::handle_Timeout_ResClear(CMessage& msg)
{
	TimerStructT* pData = (TimerStructT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"Timeout_ResClear  GID[0x%04X] ", pData->GID);	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(pData->GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
		return;
	}
	pGrpCCB->deleteGrpTimer(TIMER_ResClear);
	pGrpCCB->airRes.setResReason(ResReason_LePaging);
}

void VoiceFSM::handle_Timeout_LePagingStart(CMessage& msg)
{
	TimerStructT* pData = (TimerStructT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"Timeout_LePagingStart  GID[0x%04X] ", pData->GID);	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(pData->GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
		return;
	}
	pGrpCCB->deleteGrpTimer(TIMER_LePagingStart);
	pGrpCCB->startGrpTimer(TIMER_LePagingLoop);
}

void VoiceFSM::handle_Timeout_GrpDataDetect(CMessage& msg)
{
	TimerStructT* pData = (TimerStructT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"Timeout_GrpDataDetect  GID[0x%04X] ", pData->GID);	
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(pData->GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
		return;
	}
	//����ϵͳ��涨ȥ���㲥�ŵ��������ļ��
#if 0	
	if(!pGrpCCB->airRes.getGrpDataDetectFlag())
	{
#if 0	
		pGrpCCB->sendGrpResReq2L2(FREE_GRPRES, NOT_NEED_RSP, 
			ResReason_GrpPaging, pGrpCCB->getTransID(), 
			NO_REPORTID, NO_EID, M_INVALID_BTSID);		
		pGrpCCB->airRes.setResReadyFlag(false);
#endif	
		LOG1(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GID[0x%04X] have no GrpData for a long time, release Group Call!!!###########", 
			pGrpCCB->getGID());
		pGrpCCB->clearAllTimers();
		pGrpCCB->startGrpTimer(TIMER_GrpRls);
		pGrpCCB->SetCurrentState(GRP_RELEASE_STATE);
		LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GID[0x%04X] Enter state[GRP_RELEASE_STATE]", pGrpCCB->getGID());
	}
#endif	
	pGrpCCB->airRes.setGrpDataDetectFlag(false);
}

void VoiceFSM::handle_Timeout_GrpRls(CMessage& msg)
{
	TimerStructT* pData = (TimerStructT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"Timeout_GrpRls  GID[0x%04X] ", pData->GID);		
	GrpCCB* pGrpCCB = pGrpCCBTbl->FindCCBByGID(pData->GID);
	if(NULL==pGrpCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find GrpCCB");
		return;
	}
	pGrpCCB->deleteGrpTimer(TIMER_GrpRls);
	pGrpCCB->sendGrpResReq2L2(FREE_GRPRES, NOT_NEED_RSP, 
		ResReason_GrpPaging, pGrpCCB->getTransID(), 
		NO_REPORTID, NO_EID, M_INVALID_BTSID);	
	LOG1(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"DeAllocGrpCCB; GID[0x%04X]", pData->GID);
	pGrpCCBTbl->DeAllocGrpCCB(pGrpCCB);
}

//���뻰Ȩ��ʡ��Դ---begin
//from btsL2
void VoiceFSM::handle_L2L3BtsPttPressReq(CMessage& msg)
{
	//��L3����PTT PRESS APPLY REQ��Ϣ��SAG,not using DAC
	L2L3_BtsPttPressReqT*pData =  (L2L3_BtsPttPressReqT*)msg.GetDataPtr();
	UINT16 tmpGID = VGetU16BitVal(pData->GID);
	UINT32 tmpEID = msg.GetEID();
	LOG5(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"BtsPttPressReq<----btsL2; GID[0x%04X] EID[0x%08X] CID[0x%02X] Prio[0x%02X] EncryptCtrl[0x%02X]",
		 tmpGID, tmpEID, pData->cid, pData->prio, pData->EncryptCtrl);
	VoiceTuple tuple;
	tuple.Eid = tmpEID;
	tuple.Cid = pData->cid;
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(NULL==pCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find VoiceCCB!!!");
		return;
	}
	else
	{
		send_grpPttPressApplyReq2SXC(pCCB->getUID(), 
			tmpGID, pData->prio, pData->EncryptCtrl, USE_PAGING_CHANNEL);
	}
}
void VoiceFSM::handle_L2L3BtsPttPressCancel(CMessage& msg)
{
	//����PTT PRESS CANCEL��Ϣ��SAG
	L2L3_BtsPttPressCancelT *pDataL2L3 = (L2L3_BtsPttPressCancelT*)msg.GetDataPtr();
	UINT16 tmpGID = VGetU16BitVal(pDataL2L3->GID);
	UINT32 tmpEID = msg.GetEID();
	LOG3(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"BtsPttPressCancel<----btsL2; GID[0x%04X] EID[0x%08X] CID[0x%02X]",
		 tmpGID, tmpEID, pDataL2L3->cid);
	VoiceTuple tuple;
	tuple.Eid = tmpEID;
	tuple.Cid = pDataL2L3->cid;
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(NULL==pCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find VoiceCCB!!!");
		return;
	}
	else
	{
		CMsg_Signal_VCR SAbisSignal;
		VoiceVCRCtrlMsgT *pData;
		typedef struct 
		{
			UINT8 uid[4];
			UINT8 msgType;	
			UINT8 GID[2];
		}PttPressCancelT;
		if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(PttPressCancelT)) )
		{
			LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
			return;
		}
		else
		{
			pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
			SAbisSignal.SetDstTid(M_TID_VCR);
			SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
			SAbisSignal.SetBTSSAGID();
			SAbisSignal.SetSigIDS(PTT_PressCancel_MSG);

			PttPressCancelT* pInnerData = (PttPressCancelT*)&pData->sigPayload;
			
			VSetU32BitVal(pInnerData->uid, pCCB->getUID());
			pInnerData->msgType = M_MSGTYPE_PTT_PRESS_CANCEL;
			VSetU16BitVal(pInnerData->GID, tmpGID);
			
			SAbisSignal.SetSigHeaderLengthField(sizeof(PttPressCancelT));
			SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(PttPressCancelT));
			if(SAbisSignal.Post())
			{
				LOG2(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
					"PttPressCancel---->SXC, GID[0x%04X]  UID[0x%08X]", 
					tmpGID, pCCB->getUID());	
			}
			else
			{
				LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send msg to VCR failed!!!");
			}
		}
	}
	
}
void VoiceFSM::handle_L2L3BtsPttPressRelease(CMessage& msg)
{
	//����PTT RELEASE��Ϣ��SAG
	L2L3_BtsPttPressReleaseT *pDataL2L3 = (L2L3_BtsPttPressReleaseT*)msg.GetDataPtr();
	UINT16 tmpGID = VGetU16BitVal(pDataL2L3->GID);
	UINT32 tmpEID = msg.GetEID();
	LOG3(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"BtsPttPressRelease<----btsL2; GID[0x%04X] EID[0x%08X] CID[0x%02X]",
		 tmpGID, tmpEID, pDataL2L3->cid);
	VoiceTuple tuple;
	tuple.Eid = tmpEID;
	tuple.Cid = pDataL2L3->cid;
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(NULL==pCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find VoiceCCB!!!");
		return;
	}
	else
	{
		//20090903��ʱ��sag����ptt press cancel
		CMsg_Signal_VCR SAbisSignal;
		VoiceVCRCtrlMsgT *pData;
		typedef struct 
		{
			UINT8 uid[4];
			UINT8 msgType;	
			UINT8 GID[2];
		}PttPressCancelT;
		if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(PttPressCancelT)) )
		{
			LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
			return;
		}
		else
		{
			pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
			SAbisSignal.SetDstTid(M_TID_VCR);
			SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
			SAbisSignal.SetBTSSAGID();
			SAbisSignal.SetSigIDS(PTT_PressCancel_MSG);

			PttPressCancelT* pInnerData = (PttPressCancelT*)&pData->sigPayload;
			
			VSetU32BitVal(pInnerData->uid, pCCB->getUID());
			pInnerData->msgType = M_MSGTYPE_PTT_PRESS_CANCEL;
			VSetU16BitVal(pInnerData->GID, tmpGID);
			
			SAbisSignal.SetSigHeaderLengthField(sizeof(PttPressCancelT));
			SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(PttPressCancelT));
			if(SAbisSignal.Post())
			{
				LOG2(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
					"PttPressCancel---->SXC, GID[0x%04X]  UID[0x%08X]", 
					tmpGID, pCCB->getUID());	
			}
			else
			{
				LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send msg to VCR failed!!!");
			}
		}		
	}
}
//from SXC
void VoiceFSM::handle_GrpPttPressApplyRsp_frmSXC(CMessage& msg)
{
	//��վL3֪ͨ��վL2��Ȩ����Ľ����
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	UINT32 tmpUID = VGetU32BitVal(pData->sigPayload.PttPressApplyRsp.UID);
	UINT16 tmpGID = VGetU16BitVal(pData->sigPayload.PttPressApplyRsp.GID);

	LOG4(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"PttPressApplyRsp<----SXC; GID[0x%04X] UID[0x%08X] Grant[0x%02X], EncryptCtrl[%d]",
		tmpGID, tmpUID, pData->sigPayload.PttPressApplyRsp.TransmissionGrant,
		pData->sigPayload.PttPressApplyRsp.EncryptCtrl);

	VoiceCCB *pCCB = (VoiceCCB*)CCBTable->FindCCBByUID(tmpUID);
	if(NULL==pCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "cannot find VoiceCCB!!!");
		return;
	}
	//BS�鿴transmission grant�ֶΣ����ŶӾܾ�������
	if(TRANSGRANT_FORBID_TALK==pData->sigPayload.PttPressApplyRsp.TransmissionGrant)
	{
		return;
	}
	//�����Ŷӳɹ���ȡ���Ŷӡ���Ȩ������鲻���ڣ�
	//�ȷ���0x3c18��Ϣ��20120507 fengbing
	CComMessage *pL2L3Msg = new (CTVoice::GetInstance(), sizeof(L2L3_BtsPttPressRspT)) CComMessage;
	if(NULL==pL2L3Msg)
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		L2L3_BtsPttPressRspT* pL2L3Data = (L2L3_BtsPttPressRspT*)pL2L3Msg->GetDataPtr();
		pL2L3Data->cid= pCCB->getVoiceTuple().Cid;
		VSetU16BitVal(pL2L3Data->GID, tmpGID);
		//20090903����Ϣ�ṹ��Ϊ��DAC����Ӧһ����grant�ֶζ���Ҳһ����������encryptCtrl�ֶ�
		pL2L3Data->Grant = pData->sigPayload.PttPressApplyRsp.TransmissionGrant;
		pL2L3Data->EncryptCtrl = pData->sigPayload.PttPressApplyRsp.EncryptCtrl;
		
		pL2L3Msg->SetEID(pCCB->getVoiceTuple().Eid);
		pL2L3Msg->SetDstTid(M_TID_VAC);
		pL2L3Msg->SetSrcTid(M_TID_VOICE);
		pL2L3Msg->SetMessageId(MSGID_GRP_L2L3_BTS_PTT_PRESS_RSP);
		pL2L3Msg->SetDataLength(sizeof(PTTPressRspT));
		if(postComMsg(pL2L3Msg))
		{
			LOG6(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"BtsPttPressRsp---->btsL2, GID[0x%04X] UID[0x%08X] EID[0x%08X] cid[0x%02X] Grant[%d] EncryptCtrl[%d]", 
				tmpGID, tmpUID, pCCB->getVoiceTuple().Eid, 
				pCCB->getVoiceTuple().Cid, pL2L3Data->Grant, pL2L3Data->EncryptCtrl);
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send PTTPressRsp(DAC) msg to UTV failed!!!");
		}
	}
	//����0x680D��DAC�Ļ�Ȩ��Ӧ��Ϣ��20120507 fengbing
	CComMessage *pDacMsg = new (CTVoice::GetInstance(), sizeof(PTTPressRspT)) CComMessage;
	if(NULL==pDacMsg)
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		PTTPressRspT* pDacData = (PTTPressRspT*)pDacMsg->GetDataPtr();
		pDacData->Cid = pCCB->getVoiceTuple().Cid;
		pDacData->msgType = M_MSGTYPE_PTT_PRESS_RSP;
		VSetU16BitVal(pDacData->GID, tmpGID);
		pDacData->Grant = pData->sigPayload.PttPressApplyRsp.TransmissionGrant;
		pDacData->EncryptCtrl = pData->sigPayload.PttPressApplyRsp.EncryptCtrl;
		pDacMsg->SetEID(pCCB->getVoiceTuple().Eid);
		pDacMsg->SetDstTid(M_TID_UTV);
		pDacMsg->SetSrcTid(M_TID_VOICE);
		pDacMsg->SetMessageId(MSGID_GRP_DAC_PTT_RES_RSP);
		pDacMsg->SetDataLength(sizeof(PTTPressRspT));
		if(postComMsg(pDacMsg))
		{
			LOG6(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"PTTPressRsp(DAC)---->CPE, GID[0x%04X] UID[0x%08X] EID[0x%08X] cid[0x%02X] Grant[%d] EncryptCtrl[%d]", 
				tmpGID, tmpUID, pCCB->getVoiceTuple().Eid, 
				pCCB->getVoiceTuple().Cid, pDacData->Grant, pDacData->EncryptCtrl);
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send PTTPressRsp(DAC) msg to UTV failed!!!");
		}
	}	
	
}
//from CPEL3 
void VoiceFSM::handle_GrpDacPttPressReq(CMessage& msg)
{
	//��L3����PTT PRESS APPLY REQ��Ϣ��SAG,using DAC
	PTTPressReqT* pData =  (PTTPressReqT*)msg.GetDataPtr();
	UINT16 tmpGID = VGetU16BitVal(pData->Gid);
	UINT32 tmpEID = msg.GetEID();
	LOG5(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"PTTPressReq(DAC)<----cpeL3; GID[0x%04X] EID[0x%08X] CID[0x%02X] Prio[0x%02X] EncryptCtrl[0x%02X]",
		 tmpGID, tmpEID, pData->Cid, pData->CallPrioity, pData->EncryptControl);
	VoiceTuple tuple;
	tuple.Eid = tmpEID;
	tuple.Cid = pData->Cid;
	VoiceCCB* pCCB = (VoiceCCB*)CCBTable->FindCCBByEID_CID(tuple);
	if(NULL==pCCB)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Cannot find VoiceCCB!!!");
		return;
	}
	else
	{
		send_grpPttPressApplyReq2SXC(pCCB->getUID(), 
			tmpGID, pData->CallPrioity, pData->EncryptControl, USE_DAC_CHANNEL);
	}
	
}

bool VoiceFSM::send_grpPttPressApplyReq2SXC(UINT32 uid, 
	UINT16 gid, UINT8 prio, UINT8 encryptCtrl, UINT16 sessionType)
{
	CMsg_Signal_VCR SAbisSignal;
	VoiceVCRCtrlMsgT *pData;
	if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(PttPressApplyReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
		SAbisSignal.SetDstTid(M_TID_VCR);
		SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		SAbisSignal.SetBTSSAGID();
		SAbisSignal.SetSigIDS(PTT_PressApplyReq_MSG);

		VSetU32BitVal(pData->sigPayload.PttPressApplyReq.UID, uid);
		VSetU16BitVal(pData->sigPayload.PttPressApplyReq.GID, gid);
		pData->sigPayload.PttPressApplyReq.prio = prio;
		pData->sigPayload.PttPressApplyReq.EncryptCtrl = encryptCtrl;
		VSetU16BitVal(pData->sigPayload.PttPressApplyReq.sessionType, sessionType);

		SAbisSignal.SetSigHeaderLengthField(sizeof(PttPressApplyReqT));
		SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(PttPressApplyReqT));
		if(SAbisSignal.Post())
		{
			LOG5(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"PttPressApplyReq---->SXC, GID[0x%04X]  UID[0x%08X] Prio[0x%02X] encryptCtrl[%d] sessionType[%d]", 
				gid, uid, prio, encryptCtrl, sessionType);	
			return true;
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send msg to VCR failed!!!");
			return false;
		}
	}	
}
//���뻰Ȩ��ʡ��Դ---end

#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
void VoiceFSM::registerVideoUT(UINT32 uid)
{
	if(!isVideoUT(uid))
	{
		m_VideoUTList.push_back(uid);
		LOG1(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"registerVideoUT UID[0x%08X] ", uid);
	}
}
void VoiceFSM::unRegisterVideoUT(UINT32 uid)
{
	if(isVideoUT(uid))
	{
		LOG1(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
					"unRegisterVideoUT UID[0x%08X] ", uid);
		m_VideoUTList.remove(uid);
	}	
}
bool VoiceFSM::isVideoUT(UINT32 uid)
{
	list<UINT32>::iterator itUID;
	for(itUID=m_VideoUTList.begin();itUID!=m_VideoUTList.end();itUID++)
	{
		if(*itUID==uid)
		{
			return true;
		}
	}
	return false;
}
void VoiceFSM::showVideoUTList()
{
	int i=0;
	list<UINT32>::iterator itUID;
	VPRINT("\n-------VideoCpeList(%d)--------", m_VideoUTList.size());
	for(itUID=m_VideoUTList.begin();itUID!=m_VideoUTList.end();itUID++)
	{
		if(i++ % 8 == 0)
			VPRINT("\n");
		VPRINT(" 0x%08X", *itUID);
	}
	VPRINT("\n------------------------------\n");
}
extern "C" void showVideoCpeList()
{
	CTVoice::GetInstance()->getVoiceFSM()->showVideoUTList();
}
#endif//#ifdef M_VIDEOUT_GRPRES_OPTIMIZE

////////////////////////////////////////////////////////////////////////////////
#ifdef M_SYNC_BROADCAST

void VoiceFSM::handle_BtsL2SxcDLMsg_frmSXC(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"BTSL2<----BtsL2SxcMsg<----SXC, msgType1[0x%02X]", 
		pData->sigPayload.BtsL2SxcMsg.msgType1);
		
	msg.SetDstTid(M_TID_VAC);
	msg.SetSrcTid(M_TID_VOICE);
	msg.SetMessageId(MSGID_L2L3_BTSL2SXC_DL_MSG);
	msg.SetDataLength(VGetU16BitVal(pData->sigHeader.Length));
	msg.SetDataPtr(&pData->sigPayload.BtsL2SxcMsg.msgType1);

	if(!msg.Post())
	{
		LOG1(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"PostMsgError!!! BTSL2<----BtsL2SxcMsg<----SXC, msgType1[0x%02X]", 
			pData->sigPayload.BtsL2SxcMsg.msgType1);
		msg.DeleteMessage();
	}
}

void VoiceFSM::handle_BtsL2SxcULMsg_frmL2(CMessage& msg)
{
	UINT8* pData = (UINT8*)msg.GetDataPtr();
	LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
		"SXC<----BtsL2SxcMsg<----BTSL2, msgType1[0x%02X]", pData[0]);
		
	UINT16 len = msg.GetDataLength();
	CMsg_Signal_VCR signalMsgVCR;
	VoiceVCRCtrlMsgT* pDataVCR;
	if ( !signalMsgVCR.CreateMessage(*CTVoice::GetInstance(), 
		len+sizeof(SigHeaderT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}

	signalMsgVCR.SetDstTid(M_TID_VCR);
	signalMsgVCR.SetMessageId(MSGID_VOICE_VCR_SIGNAL);

	pDataVCR = (VoiceVCRCtrlMsgT*)signalMsgVCR.GetDataPtr();
	signalMsgVCR.SetBTSSAGID(); 	//SAGID, BTSID
	signalMsgVCR.SetSigIDS(BTSL2_SAG_MSG);

	//payload, 
	memcpy((void*)&(pDataVCR->sigPayload.BtsL2SxcMsg.msgType1),
		   (void*)(msg.GetDataPtr()),
		   len);

	//length
	signalMsgVCR.SetSigHeaderLengthField(len);
	signalMsgVCR.SetPayloadLength(len+sizeof(SigHeaderT));
	//transfer message to SAG
	signalMsgVCR.Post();
}

void VoiceFSM::handle_L2L3MBMSMediaDelayRsp(CMessage& msg)
{
	msg.SetDstTid(M_TID_VDR);
	if(!msg.Post())
	{
		msg.DeleteMessage();
	}
}

typedef struct _SyncHeadT
{
	UINT8 Rev:6;//Rev 6bit	1byte	ͬ��Ԥ���ֶ�
	UINT8 STATU:2;//STATU	2bit		��״̬��//00��ҵ���У�01������ͬ����10��ͬ����Ӧ
	UINT8 TimeStamp[4];//TimeStamp 4byte	ʱ�������λ10ms
	UINT8 ElapsedByteCnt[4];//ElapsedByteCnt	4byte	SAG�ѷ���MBMS���ֽ���
}SyncHeadT;

void VoiceFSM::handle_SyncBroadcastData(CMessage & msg)
{
	//�û�ֻ����һ�����ͬ�����ݰ���������������ݰ�(ͬ�����ͬ��)��Ҳ�޸����������ݰ�
	Counters.nVoiDataFromVDR++;
	
	UINT8 curCodec;
	UINT32 L3addr;
	GrpCCB* pGrpCCB = NULL;
	UINT8	i,nDataPkt,nFrameNum,nDataLen;
	UINT8	*pUdp = (UINT8*)msg.GetDataPtr();
	UINT16 nTmpVDataLen;
	
	nDataPkt = ((DMUXHeadT*)pUdp)->nFrameNum;	//UDP���������ݸ���
	DMUXVoiDataCommonT* pVoiDataCommon = (DMUXVoiDataCommonT*)(pUdp+sizeof(DMUXHeadT)); //ָ���һ����������֡	
	for(i=0;i<nDataPkt;i++)
	{
		//����
		//729B֡����pVoiDataCommon->frameNum�м���
		curCodec = pVoiDataCommon->Codec;
		nFrameNum = pVoiDataCommon->frameNum;
		switch(curCodec)
		{
			case CODEC_GRPTONE:
				nDataLen = M_G729_10MS_DATALEN;
				break;
			case CODEC_ENCRYPT_GRPTONE:
				nDataLen = M_ENC_VDATA_PKT_LEN;
				break;

			default:
				LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "codec not support!!!!");
				return;
		}
		//�õ�L3Addr;
		L3addr = VGetU16BitVal( pVoiDataCommon->CallID );
		if(L3addr==(M_INVALID_GRPL3ADDR&0xffff))//SXC��BTS��ȡ�����ӳ�
		{
			nTmpVDataLen = 0;
		}
		else
		{			
			//�ҵ�CCB
			pGrpCCB = pGrpCCBTbl->FindCCBByGrpL3addr(L3addr);
			if(NULL!=pGrpCCB)
			{
				VSetU16BitVal(pVoiDataCommon->CallID, pGrpCCB->getGID());
				pGrpCCB->airRes.setGrpDataDetectFlag(true);
			}
			else
			{
				VSetU16BitVal(pVoiDataCommon->CallID, M_INVALID_GID);
				LOG1(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), "cannot find GrpCCB with L3Addr[0x%x]!!!!", L3addr);
				Counters.nVDRCCBNULL++;//wangwenhua add 20081222			
			}
			
			nTmpVDataLen = isEncSrtpVData(curCodec) ? nDataLen : nFrameNum*nDataLen;
		}
		//ָ����һ����������
		pVoiDataCommon = (DMUXVoiDataCommonT*)((UINT8*)pVoiDataCommon + 
												sizeof(DMUXVoiDataCommonT) + 
												nTmpVDataLen +
												M_G729B_SRTP_DATALEN*pVoiDataCommon->blG729B +
												sizeof(SyncHeadT)); 		
	}

	msg.SetDstTid(M_TID_VAC);
	msg.SetMessageId(MSGID_SYNCBROADCAST_DATA);
	if(msg.Post())
	{
		Counters.nVoiDataToVAC++;
	}
	else				//���������Ϣ
	{
		msg.DeleteMessage();
	}
}

#endif//M_SYNC_BROADCAST			
////////////////////////////////////////////////////////////////////////////////

//��Ⱥ���----------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

UINT32	g_tmLenStatusReport=M_TIMERLEN_STATUSREPORT;	//״̬��������ms
UINT32	g_tmGrpPagingRsp = M_TIMERLEN_GRPPAGINGRSP;
UINT32 	g_tmLenLePagingRsp = M_TIMERLEN_LEPAGINGRSP;
UINT32 	g_tmLenGrpRls = M_TIMERLEN_GRPRLS;
UINT32 	g_tmLenLePagingLoop =  M_TIMERLEN_LEPAGINGLOOP;
UINT32 	g_tmLenResClear = M_TIMERLEN_RESCLEAR;
UINT32 	g_tmLenLePagingStart = M_TIMERLEN_LEPAGINGSTART;
UINT32 	g_tmLenGrpDataDetect =  M_TIMERLEN_GRPDATADETECT;

bool g_blUseStatusReport = false;
UINT32 g_cntFreeStatusReportNode = STATUSREPORT_POOLSIZE;
statusReportNodeT  g_StatusReportPool[STATUSREPORT_POOLSIZE];	
statusReportNodeT* g_freeStatusReportLst;

//״̬���������ʼ��
//��ʼ��״̬����洢�غͿ��������
void StatusReport_Init()
{
	UINT32 i;
	for(i=0;i<STATUSREPORT_POOLSIZE-1;i++)
	{
		g_StatusReportPool[i].pNext = &g_StatusReportPool[i+1];
	}
	g_StatusReportPool[i].pNext = NULL;
	g_freeStatusReportLst = &g_StatusReportPool[0];
	g_cntFreeStatusReportNode = STATUSREPORT_POOLSIZE;
}

//clear StatusReport
void StatusReport::clear()
{
	UINT16 i;
	clearLst();
	curCounter = 0;
	for(i=0;i<M_StatusReport_CountPeriod;i++)
	{
		//������ι����ж�����û���û�
		GrpCpesCounters[i]=0xffff;
	}
}

void StatusReport::showInfo(char* txtMark)
{
	UINT16 i;
	statusReportNodeT* pNode = grpStatusList;
	VPRINT("\n\n==========begin=============");
	if(txtMark!=NULL)
	{
		VPRINT("\n%s\n", txtMark);
	}
	VPRINT("\t g_freeStatusReportLst[0x%08X] g_cntFreeStatusReportNode[0x%08X]",
			(UINT32)g_freeStatusReportLst, g_cntFreeStatusReportNode);
	VPRINT("\n\t g_StatusReportPool Begin[0x%08X]---End[0x%08X] sizeof(statusReportNodeT)[%d]",
			(UINT32)g_StatusReportPool, (UINT32)&g_StatusReportPool[STATUSREPORT_POOLSIZE-1], sizeof(statusReportNodeT));	
	VPRINT("\n\t grpStatusList[0x%08X] cpesCnt[0x%08X]",
			(UINT32)grpStatusList, cpesCnt);
	for(i=0;i<cpesCnt;i++)
	{
		if(0==(i&0x03))
			VPRINT("\n");
		if(pNode!=NULL)
		{
			VPRINT("\t i[%d] pNode[0x%08X] pNode->pNext[0x%08X]", i, (UINT32)pNode, (UINT32)pNode->pNext);
			if(pNode<g_StatusReportPool || pNode>(&g_StatusReportPool[STATUSREPORT_POOLSIZE-1]))
			{
				VPRINT("\t i[%d] pNode pointer Invalid!!!!!!!",i);
				break;
			}
			if(pNode->pNext<g_StatusReportPool || pNode->pNext>(&g_StatusReportPool[STATUSREPORT_POOLSIZE-1]))
			{
				if(pNode->pNext!=NULL)
				{
					VPRINT("\t i[%d] pNode->pNext pointer Invalid!!!!!!!",i);
					break;
				}
			}
			pNode=pNode->pNext;
		}
		else
		{
			VPRINT("\t i[%d] pNode==NULL!!!!!!!",i);
			break;
		}
	}
	VPRINT("\n\n==========end===============");
	//cout.flush();
}

void StatusReport::clearLst()
{
	//showInfo("before clearLst");
	statusReportNodeT* pNode;
	//��Ϊ����״̬�������쳣���̿��ܵ���grpStatusList�����cpesCnt��һ�£�
	//�����ͷ�����ʱ��grpStatusListΪ׼!!!!!!!!
	//while(grpStatusList!=NULL && cpesCnt>0)	
	while(grpStatusList!=NULL)
	{
#if 1	
		if(grpStatusList<g_StatusReportPool || grpStatusList>(&g_StatusReportPool[STATUSREPORT_POOLSIZE-1]))
		{
			showInfo("during clearLst!Pointer Exception!!!!!!!!!!!!");
			//taskDelay(1);
		}
#endif		
		pNode = grpStatusList->pNext;					//remember new head
		grpStatusList->pNext = g_freeStatusReportLst;	//head return to freelist
		g_freeStatusReportLst = grpStatusList;			//update head of freelist
		grpStatusList = pNode;							//update head of list
		//cpesCnt--;
		g_cntFreeStatusReportNode++;
	}
	grpStatusList = NULL;
	cpesCnt = 0;	
	//showInfo("after clearLst");
}

statusReportNodeT* StatusReport::getFreeNode()
{
	statusReportNodeT* ret = NULL;
	if(g_freeStatusReportLst!=NULL && g_cntFreeStatusReportNode>0)
	{
		ret = g_freeStatusReportLst;
#if 1	
		if(ret<g_StatusReportPool || ret>(&g_StatusReportPool[STATUSREPORT_POOLSIZE-1]))
		{
			showInfo("during getFreeNode!Pointer Exception!!!!!!!!!!!!");
			//taskDelay(1);
		}
#endif			
		g_freeStatusReportLst = ret->pNext;
		g_cntFreeStatusReportNode--;
	}
	else
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "g_cntFreeStatusReportNode<=0!!!");
	}
	return ret;
}

bool StatusReport::addStatusReportItem(GrpCpeStatusT item)
{
	//showInfo("before addStatusReportItem");
	statusReportNodeT* pNode = getFreeNode();	//��ȡ�½ڵ�
	if(NULL!=pNode)
	{
		pNode->grpCpeStatus = item;			//��д����
		pNode->pNext = grpStatusList;			//ͷ��������
		grpStatusList = pNode;					//��������ͷ��
		cpesCnt++;
	}
	else
	{
		//�˴�����cpe����������Ϊ�˱�����SXC����״̬����ʱ�쳣
		//����cpe������״̬���������Ȳ�һ�£�
		//Ӧ��֤״̬��������pool��С�㹻�������������
		//cpesCnt++
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "StatusReport POOL used up!!!");
	}
	//showInfo("after addStatusReportItem");
	return true;
}

void StatusReport::sendStatusReport2SXC()
{
	//showInfo("before sendStatusReport2SXC");
	UINT8 num,i;
	UINT16 numLeft = cpesCnt;
	statusReportNodeT* pNodeNow = grpStatusList;
	LOG2(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GID[0x%04X] has [0x%04X] StatusReportItems ; ",
		pGrpCCB->getGID(), cpesCnt);
	while(numLeft!=0)
	{
		num = (numLeft>M_MAX_CPENUM_IN_ONE_STATUSREPORT) ? M_MAX_CPENUM_IN_ONE_STATUSREPORT : numLeft;

		CMsg_Signal_VCR SAbisSignal;
		VoiceVCRCtrlMsgT *pData;
		//statusReportNodeT* pNode;
		
		if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), 
			sizeof(SigHeaderT)+sizeof(UINT32)+sizeof(UINT8)+num*sizeof(GrpCpeStatusT) ) )
		{
			LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
			return;
		}
		else
		{
			pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
			SAbisSignal.SetDstTid(M_TID_VCR);
			SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
			SAbisSignal.SetBTSSAGID();
			SAbisSignal.SetSigIDS(StatusReport_MSG);

			VSetU32BitVal(pData->sigPayload.StatusReport.GrpL3Addr , pGrpCCB->getGrpL3Addr());
			pData->sigPayload.StatusReport.num = num;
			//��д״̬������
			for(i=0;i<num;i++)
			{
#if 0			
				if(grpStatusList!=NULL)
				{
					pData->sigPayload.StatusReport.UserStatusArr[i] = grpStatusList->grpCpeStatus;
					pNode = grpStatusList;				//ȡ��ͷ�ڵ�
					grpStatusList = pNode->pNext;		//��������ͷ��
					pNode->pNext = g_freeStatusReportLst;	//ͷ�����������
					g_freeStatusReportLst = pNode;			//���¿�������ͷ
					g_cntFreeStatusReportNode++;			//���м�������
				}
#endif				
				if(pNodeNow!=NULL)
				{
					pData->sigPayload.StatusReport.UserStatusArr[i] = pNodeNow->grpCpeStatus;
					pNodeNow = pNodeNow->pNext;
				}				
				else
				{
					LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "grpStatusList abnormal!!!");
					SAbisSignal.DeleteMessage();
					showInfo("during sendStatusReport2SXC! Pointer Exception!!!!!!!!");
					return;
				}
			}

			SAbisSignal.SetSigHeaderLengthField(sizeof(UINT32)+sizeof(UINT8)+num*sizeof(GrpCpeStatusT));
			SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(UINT32)+sizeof(UINT8)+num*sizeof(GrpCpeStatusT));
			if(SAbisSignal.Post())
			{
				LOG2(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
					"StatusReport---->SXC; GID[0x%04X] num[0x%02X]",
					pGrpCCB->getGID(), num);
			}
			else
			{
				LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Send Msg to VCR fail!!!");
			}
		}

		numLeft -=num;
	}	
	//showInfo("after sendStatusReport2SXC");
}

void AirResource::setResReadyFlag(bool flag)
{
	LOG2(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"setResReadyFlag; GID[0x%04X]  isResReady[0x%02X]", 
		pGrpCCB->getGID(), flag);
	blResReady=flag;
	if(!flag){setGrpDataDetectFlag(false);}
}
void AirResource::setResReason(UINT8 reason)
{
	ResReason=reason;
	LOG2(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"setResReason; GID[0x%04X] reason[0x%02X]",
		pGrpCCB->getGID(), reason);
}


//��ʼ��
//GrpCCBTable��ʼ��
void GrpCCBTable::init()
{ 
	UINT16	i;
	//FreeGrpCCBList��ʼ��,ȫ��grpCCB������������;
	for(i=0;i<GRP_CCB_NUM;i++)
	{
		//��grpCCB���г�ʼ��;
		grpCCBTbl[i].initCCB();
		grpCCBTbl[i].setTabIndex(i);
		grpCCBTbl[i].setCCBTable(this);
		//��������
		FreeGrpCCBList.push_back(i);
	}
	//GID���������;
	BTreeByGID.clear();
	//grpL3���������;
	BTreeByGrpL3addr.clear();

	//״̬������س�ʼ��
	StatusReport_Init();
}

UINT32 GrpCCBTable::getActiveGrpNum()
{
	return(BTreeByGrpL3addr.size());
}


GrpCCB* GrpCCBTable::AllocGrpCCB()
{
	GrpCCB* ret = NULL;
	if (FreeGrpCCBList.empty()) 
	{
		return ret;
	}
	ret = &grpCCBTbl[*(FreeGrpCCBList.begin())];
	if(ret!=NULL)
	{
		ret->CCBClean();//200100104 fengbing
		//����GRPCCB��Ĭ��ΪGRP_WORKING_STATE
		ret->SetCurrentState(GRP_WORKING_STATE);	//200100104 fengbing
	}
	FreeGrpCCBList.pop_front();
	return ret;	
}

void GrpCCBTable::DeAllocGrpCCB(GrpCCB *pCCB)
{
	UINT16 index = pCCB->getTabIndex();
	DelBTreeGID(pCCB->getGID());
	DelBTreeGrpL3addr(pCCB->getGrpL3Addr());
	pCCB->CCBClean();
	FreeGrpCCBList.push_back(index);	
}

// ���������
void GrpCCBTable::AddBTreeGID(UINT16 gid, UINT16 index)
{
	BTreeByGID.insert(map<UINT16, UINT16>::value_type(gid,index)); 	
}

void GrpCCBTable::DelBTreeGID(UINT16 gid)
{
	map<UINT16, UINT16>::iterator it = BTreeByGID.find(gid);
	if(it!=BTreeByGID.end())
		BTreeByGID.erase(it);	
}

void GrpCCBTable::AddBTreeGrpL3addr(UINT32 grpL3addr, UINT16 index)
{
	BTreeByGrpL3addr.insert(map<UINT32, UINT16>::value_type(grpL3addr,index)); 	
}

void GrpCCBTable::DelBTreeGrpL3addr(UINT32 grpL3addr)
{
	map<UINT32, UINT16>::iterator it = BTreeByGrpL3addr.find(grpL3addr);
	if(it!=BTreeByGrpL3addr.end())
		BTreeByGrpL3addr.erase(it);
}

GrpCCB* GrpCCBTable::FindCCBByGID(UINT16 GID)
{
	map<UINT16, UINT16>::iterator it = BTreeByGID.find(GID);
	return (it==BTreeByGID.end()) ? NULL: &grpCCBTbl[(*it).second];	
}

GrpCCB* GrpCCBTable::FindCCBByGrpL3addr(unsigned int grpL3addr)
{
	map<UINT32, UINT16>::iterator it = BTreeByGrpL3addr.find(grpL3addr);
	return (it==BTreeByGrpL3addr.end()) ? NULL: &grpCCBTbl[(*it).second];	
}

void GrpCCBTable::showGrpInfo(bool blDetail)
{
	map<UINT32, UINT16>::iterator itGrpL3Addr;
	map<UINT16, UINT16>::iterator itGID;

	VPRINT("\n====================================================================\n");
	VPRINT("\nGRPCCB Pool Size[%d], Free[%d]\n", GRP_CCB_NUM, FreeGrpCCBList.size());
	VPRINT("\nStatusReport Node Pool Size[%d], Free[%d]\n", STATUSREPORT_POOLSIZE, g_cntFreeStatusReportNode);
	VPRINT("\n----------BTreeByGID Size[%d]    Format[ UID , &CCB ]-------------\n", BTreeByGID.size());
	if(blDetail)
		for(itGID=BTreeByGID.begin();itGID!=BTreeByGID.end();itGID++)
		{
			VPRINT(" [ 0x%08X , 0x%08X ]", 
				(*itGID).first,  
				(UINT32)&grpCCBTbl[(*itGID).second]);
		}
	VPRINT("\n----------BTreeByGrpL3addr Size[%d] Format[ L3Addr , &CCB ]----------\n", BTreeByGrpL3addr.size());
	if(blDetail)
		for(itGrpL3Addr=BTreeByGrpL3addr.begin();itGrpL3Addr!=BTreeByGrpL3addr.end();itGrpL3Addr++)
		{
			VPRINT(" [ 0x%08X , 0x%08X ]", 
				(*itGrpL3Addr).first, 
				(UINT32)&grpCCBTbl[(*itGrpL3Addr).second]);
		}
	VPRINT("\n====================================================================\n");
	
}

void GrpCCB::markGrpNotEmpty()
{
	UINT16 i;
	if(ifUseStatusReport())
	{
		for(i=0;i<M_StatusReport_CountPeriod;i++)
		{
			if(statusReport.GrpCpesCounters[i]==0)
			{
				statusReport.GrpCpesCounters[i]=0xffff;
			}
		}	
	}
}

//�����Ƿ��޽����û�,
//�ݸ������4��״̬�������ڵ�cpe�����ж�
bool GrpCCB::isGrpEmpty()
{
	UINT8 i;
	if(ifUseStatusReport())
	{
		for(i=0;i<M_StatusReport_CountPeriod;i++)
		{
			if(statusReport.GrpCpesCounters[i]>0)
				return false;
		}
		//fengbing 20091026 �������û�����ն˻�����жϵ�ǰcpe�
		return 0==statusReport.cpesCnt;
	}
	else
	{
		//fengbing 20091026
		//���û��״̬���棬����Ϊ����Զ�ǿգ���Դ�ͷ�������SXC
		return false;
	}
}

void GrpCCB::initMembers()
{
	blLEPagingStarted = false;
	blStatusReportFlag=g_blUseStatusReport;
	utmLenLePagingLoop=g_tmLenLePagingLoop;
	utmLenStatusReport=g_tmLenStatusReport;
	GID=M_INVALID_GID;
	grpSize=0;
	grpL3Addr=M_INVALID_GRPL3ADDR;
	pTimerStatus=NULL;
	pTimerLePaging=NULL;
	pTimerGrpPagingRsp=NULL;
	pTimerGrpLePagingRsp=NULL;
	pTimerRes=NULL;
	pTimerToneDetect=NULL;
	pTimerLePagingStart=NULL;
	pTimerGrpRls=NULL;
	airRes.pGrpCCB = this;
	statusReport.pGrpCCB = this;
	m_transID = 0;
	setEncryptFlag(ENCRYPT_CTRL_NOTUSE);
}

//GrpCCB�ĳ�ʼ��
void GrpCCB::initCCB()
{ 
	airRes.setGrpDataDetectFlag(false);
	statusReport.clear();
	airRes.setResReadyFlag(false);
	initAllTimers();
	setLePagingStartFlag(false);
	setEncryptFlag(ENCRYPT_CTRL_NOTUSE);	
}

//�������
void GrpCCB::CCBClean()
{
	setUseStatusReportFlag(g_blUseStatusReport);//20100104fengbing
	setLePagingStartFlag(false);
	setGrpL3Addr(M_INVALID_GRPL3ADDR);
	setGID(M_INVALID_GID);
	statusReport.clear();
	airRes.setResReadyFlag(false);
	clearAllTimers();
	initAllTimers();
	setEncryptFlag(ENCRYPT_CTRL_NOTUSE);
	setCommType(PAGINGTYPE_GRP_VOICE);
}

//��ʱ��--------------------------------------------------------------------

//��ʼ�����ж�ʱ��
void GrpCCB::initAllTimers()
{
	//update timeout length
	utmLenLePagingLoop=g_tmLenLePagingLoop;
	utmLenStatusReport=g_tmLenStatusReport;
	
}
//������ж�ʱ��
void GrpCCB::clearAllTimers()
{
	LOG1(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"clearAllTimers; GID[0x%04X]  ",	getGID());	
	deleteTimer(&pTimerStatus);
	deleteTimer(&pTimerLePaging);
	deleteTimer(&pTimerGrpPagingRsp);
	deleteTimer(&pTimerGrpLePagingRsp);
	deleteTimer(&pTimerRes);
	deleteTimer(&pTimerToneDetect);
	deleteTimer(&pTimerLePagingStart);	
	deleteTimer(&pTimerGrpRls);
}

void GrpCCB::startGrpTimer(UINT8 timerID)
{
	UINT8 timerType;
	UINT32 lenTimer;
	UINT32 msgID;
	CTimer **ppTimer=NULL;
	switch(timerID)
	{
	case TIMER_GrpPagingRsp:
		timerType = M_TIMER_TYPE_ONCE;
		lenTimer = g_tmGrpPagingRsp;
		msgID = MSGID_TIMER_GRP_PAGING_RSP;
		ppTimer = &pTimerGrpPagingRsp;
		break;
	case TIMER_LePagingRsp:
		timerType = M_TIMER_TYPE_ONCE;
		lenTimer = g_tmLenLePagingRsp;
		msgID = MSGID_TIMER_GRP_LEPAGING_RSP;
		ppTimer = &pTimerGrpLePagingRsp;
		break;
	case TIMER_StautsReport:
		timerType = M_TIMER_TYPE_PERIOD;
		lenTimer = g_tmLenStatusReport;
		msgID = MSGID_TIMER_GRP_STATUSREPORT;
		ppTimer = &pTimerStatus;		
		break;
	case TIMER_LePagingLoop:
		timerType = M_TIMER_TYPE_PERIOD;
		lenTimer = utmLenLePagingLoop;
		msgID = MSGID_TIMER_GRP_LEPAGING_LOOP;
		ppTimer = &pTimerLePaging;
		break;
	case TIMER_ResClear:
		timerType = M_TIMER_TYPE_ONCE;
		lenTimer = g_tmLenResClear;
		msgID = MSGID_TIMER_GRP_RES_CLEAR;
		ppTimer = &pTimerRes;		
		break;
	case TIMER_LePagingStart:
		timerType = M_TIMER_TYPE_ONCE;
		lenTimer = g_tmLenLePagingStart;
		msgID = MSGID_TIMER_GRP_LEPAGING_START;
		ppTimer = &pTimerLePagingStart;
		break;
	case TIMER_GrpDataDetect:
		timerType = M_TIMER_TYPE_PERIOD;
		lenTimer = g_tmLenGrpDataDetect;
		msgID = MSGID_TIMER_GRP_DATA_DETECT;
		ppTimer = &pTimerToneDetect;	
		break;
	case TIMER_GrpRls:
		timerType = M_TIMER_TYPE_ONCE;
		lenTimer = g_tmLenGrpRls;
		msgID = MSGID_TIMER_GRP_RLS;
		ppTimer = &pTimerGrpRls;
		break;
	default:
		return;
	}
	if(startTimer(timerType, lenTimer, timerID, ppTimer, 
		M_TID_VOICE, M_TID_VOICE, msgID, 
		NO_EID, NO_CID, getGID()))
	{
		LOG2(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"StartGrpTimer, GID[0x%04X], TimerType[%s]", 
			getGID(), (int)timerName[timerID]);
	}
	else
	{
		LOG2(LOG_MAJOR, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"!!!!StartGrpTimer error, GID[0x%04X], TimerType[%s]", 
			getGID(), (int)timerName[timerID]);
	}
	
}

void GrpCCB::stopGrpTimer(UINT8 timerID)
{
	CTimer **ppTimer=NULL;
	switch(timerID)
	{
	case TIMER_GrpPagingRsp:
		ppTimer = &pTimerGrpPagingRsp;
		break;
	case TIMER_LePagingRsp:
		ppTimer = &pTimerGrpLePagingRsp;
		break;
	case TIMER_StautsReport:
		ppTimer = &pTimerStatus;		
		break;
	case TIMER_LePagingLoop:
		ppTimer = &pTimerLePaging;
		break;
	case TIMER_ResClear:
		ppTimer = &pTimerRes;		
		break;
	case TIMER_LePagingStart:
		ppTimer = &pTimerLePagingStart;
		break;
	case TIMER_GrpDataDetect:
		ppTimer = &pTimerToneDetect;	
		break;
	case TIMER_GrpRls:
		ppTimer = &pTimerGrpRls;
		break;
	default:
		return;
	}	
	if(stopTimer(ppTimer))
	{
		LOG2(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"StopGrpTimer, GID[0x%04X], TimerType[%s]", 
			getGID(), (int)timerName[timerID]);
	}
	else
	{
		LOG2(LOG_MAJOR, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"!!!!StopGrpTimer error, GID[0x%04X], TimerType[%s]", 
			getGID(), (int)timerName[timerID]);
	}
}

void GrpCCB::deleteGrpTimer(UINT8 timerID)
{
	CTimer **ppTimer=NULL;
	switch(timerID)
	{
	case TIMER_GrpPagingRsp:
		ppTimer = &pTimerGrpPagingRsp;
		break;
	case TIMER_LePagingRsp:
		ppTimer = &pTimerGrpLePagingRsp;
		break;
	case TIMER_StautsReport:
		ppTimer = &pTimerStatus;		
		break;
	case TIMER_LePagingLoop:
		ppTimer = &pTimerLePaging;
		break;
	case TIMER_ResClear:
		ppTimer = &pTimerRes;		
		break;
	case TIMER_LePagingStart:
		ppTimer = &pTimerLePagingStart;
		break;
	case TIMER_GrpDataDetect:
		ppTimer = &pTimerToneDetect;	
		break;
	case TIMER_GrpRls:
		ppTimer = &pTimerGrpRls;
		break;
	default:
		return;
	}	
	if(deleteTimer(ppTimer))
	{
		LOG2(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"DeleteGrpTimer, GID[0x%04X], TimerType[%s]", 
			getGID(), (int)timerName[timerID]);
	}
	else
	{
#if 0	
		LOG2(LOG_MAJOR, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"!!!!DeleteGrpTimer error, GID[0x%04X], TimerType[%s]", 
			getGID(), (int)timerName[timerID]);
#endif
	}
}


//��ʱ��--------------------------------------------------------------------
#ifdef M_SYNC_BROADCAST
bool GrpCCB::sendGrpMBMSGrpResIndication2L2(CMessage& msg)
{
	VoiceVCRCtrlMsgT* pDataInMsg = (VoiceVCRCtrlMsgT*)msg.GetDataPtr();
	if(pDataInMsg->sigHeader.EVENT_GROUP_ID!=M_MSG_EVENT_GROUP_ID_CALLCTRL)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"sendGrpMBMSGrpResIndication2L2 fail, input msg EventGroupID error!!!");
		return false;
	}
	UINT16 eventID = VGetU16BitVal(pDataInMsg->sigHeader.Event_ID);
	if(eventID!=M_MSG_EVENT_ID_LAGrpPaging && eventID!=M_MSG_EVENT_ID_LEGrpPaging)
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"sendGrpMBMSGrpResIndication2L2 fail, input msg EventID error!!!");
		return false;
	}
	
	UINT8 transID, encryptFlag, *pResPtrU8;
	SXCGrpResT *pSXCRes;
	if(M_MSG_EVENT_ID_LAGrpPaging==eventID)
	{
		transID = pDataInMsg->sigPayload.GrpLAPaging.transID;
		encryptFlag = pDataInMsg->sigPayload.GrpLAPaging.EncryptFlag;
		pResPtrU8 = &pDataInMsg->sigPayload.GrpLAPaging.transID + 1;
	}
	else
	{
		transID = pDataInMsg->sigPayload.LEGrpPaging.transID;
		encryptFlag = pDataInMsg->sigPayload.LEGrpPaging.EncryptFlag;
		pResPtrU8 = &pDataInMsg->sigPayload.LEGrpPaging.transID + 1;
	}
#ifdef M__SUPPORT__ENC_RYP_TION
	//���¼�������
	if(encryptFlag!=ENCRYPT_CTRL_NOTUSE)
	{
		pResPtrU8 += M_ENCRYPT_KEY_LENGTH;
	}
#endif
	pSXCRes = (SXCGrpResT*)pResPtrU8;

	CComMessage* pComMsg = new(CTVoice::GetInstance(), sizeof(L2L3_GrpResIndicationT)) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	L2L3_GrpResIndicationT* pDataL2L3 = (L2L3_GrpResIndicationT*)pComMsg->GetDataPtr();
	memset(pComMsg->GetDataPtr(), 0, sizeof(L2L3_GrpResIndicationT));
	pDataL2L3->CID= DEFAULT_CID;	
	VSetU16BitVal(pDataL2L3->GID , getGID());
	pDataL2L3->Transaction_Id = transID;
	memcpy((void*)&pDataL2L3->sxcGrpRes, (void*)pSXCRes, sizeof(SXCGrpResT));
	pComMsg->SetDataLength(sizeof(L2L3_GrpResIndicationT));
	//send message
	pComMsg->SetDstTid(M_TID_VAC);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_GRP_L2L3_MBMSGrpRes_Indication);
	if(postComMsg(pComMsg))
	{
		LOG1(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"MBMSGroupResource.Indication---->btsL2, GID[0x%04X]", 
			VGetU16BitVal(pDataL2L3->GID));	
		return true;
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"Send MBMSGroupResource.Indication Msg to btsL2 fail!!!");
		return false;
	}	
}
#endif//M_SYNC_BROADCAST	

bool GrpCCB::sendGrpResReq2L2(ENUM_GrpResOptType optFlag,UINT8 needRspFlag,UINT8 reason,UINT8 transID,
		ENUM_ReportIDFlag statusReportIndexFlag,UINT32 eid,UINT32 btsID,UINT8 cid)
{
	//�������Ƶ�ն���ȡ��״̬������	
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	if(GET_GRPRES_VIDEOUT==optFlag)
	{
		//��Ƶ�ն�û��״̬����20100104fengbing
		//setUseStatusReportFlag(false);
	}
#endif
	CComMessage* pComMsg = new(CTVoice::GetInstance(),sizeof(L2L3_GrpResReqT)) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	L2L3_GrpResReqT* pData =(L2L3_GrpResReqT*)pComMsg->GetDataPtr();
	pData->cid = cid;
	VSetU16BitVal(pData->GID , getGID());
	pData->Operation = optFlag;
	pData->ReportIndexFlag = statusReportIndexFlag;
	pData->Reason = reason;
	pData->transID = transID;
	pData->needRspFlag = needRspFlag;
	VSetU32BitVal(pData->btsID , btsID);
	VSetU32BitVal(pData->EID , eid);
	VSetU16BitVal(pData->grpSize , getGrpSize());
#ifdef M__SUPPORT__ENC_RYP_TION	
	pData->EncryptFlag = getEncryptFlag();
#endif	
	//send message
	pComMsg->SetDstTid(M_TID_VAC);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_GRP_L2L3_RES_REQ);
	pComMsg->SetDataLength(sizeof(L2L3_GrpResReqT));
	if(postComMsg(pComMsg))
	{
		LOG6(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GrpResReq---->btsL2, GID[0x%04X], OptFlag[0x%02X] transID[0x%02X] reason[0x%02X] btsID[0x%02X] EID[0x%08X]", 
			VGetU16BitVal(pData->GID), pData->Operation, pData->transID, pData->Reason, 
			VGetU32BitVal(pData->btsID), VGetU32BitVal(pData->EID));
		return true;
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Send GrpResReq Msg to btsL2 fail!!!");
		return false;
	}
	
}

bool GrpCCB::sendGrpSignal2L2(UINT8 *pPayload,UINT16 len)
{
	CComMessage* pComMsg = new(CTVoice::GetInstance(),sizeof(L2L3_GrpSignalReqT)) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	L2L3_GrpSignalReqT* pData = (L2L3_GrpSignalReqT*)pComMsg->GetDataPtr();
	pData->cid = DEFAULT_CID;	
	VSetU16BitVal(pData->GID , getGID());
	memcpy((void*)pData->payload, (void*)pPayload, len);
	pComMsg->SetDataLength(len+sizeof(pData->cid)+sizeof(pData->GID));
	//send message
	pComMsg->SetDstTid(M_TID_VAC);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_GRP_L2L3_SIGNAL_REQ);
	if(postComMsg(pComMsg))
	{
		LOG3(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GrpSignalReq---->btsL2, GID[0x%04X], CID[0x%02X] msgType[0x%02X]", 
			VGetU16BitVal(pData->GID), pData->cid, pData->payload[0]);	
		return true;
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Send GrpSignalReq Msg to btsL2 fail!!!");
		return false;
	}
}

bool GrpCCB::sendGrpPagingReq2L2(UINT8 pagingType, UINT8 LEFlag, UINT8 transID)
{
	CComMessage* pComMsg = new(CTVoice::GetInstance(),sizeof(L2L3_GrpPagingReqT)) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	L2L3_GrpPagingReqT* pData = (L2L3_GrpPagingReqT*)pComMsg->GetDataPtr();
	pData->cid = DEFAULT_CID;	
	VSetU16BitVal(pData->GID , getGID());
	pData->PagingType = pagingType;
	pData->needRspFlag = 1;
	pData->isLEPaging = LEFlag;
	VSetU16BitVal(pData->grpSize , getGrpSize());
	pData->pagingTimes = 3;
	pData->transID = transID;
	pData->callPrioty = getPrioty();
	pData->EncryptFlag = getEncryptFlag();
#ifdef M__SUPPORT__ENC_RYP_TION	
	if(getEncryptFlag()!=ENCRYPT_CTRL_NOTUSE)
	{
		memcpy(pData->EncryptKey, getEncryptKey(), sizeof(pData->EncryptKey));
	}
#endif	
	pComMsg->SetDataLength(sizeof(L2L3_GrpPagingReqT));
	//send message
	pComMsg->SetDstTid(M_TID_VAC);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_GRP_L2L3_PAGING_REQ);
	if(postComMsg(pComMsg))
	{
		LOG6(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GrpPagingReq---->btsL2, GID[0x%04X], transID[0x%02X], isLEPaging[0x%02X] grpSize[0x%04X] pagingTimes[0x%02X] PagingType[0x%02X]", 
			VGetU16BitVal(pData->GID), pData->transID, pData->isLEPaging, 
			VGetU16BitVal(pData->grpSize), pData->pagingTimes, pData->PagingType);
		return true;
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "Send GrpPagingReq Msg to btsL2 fail!!!");
		return false;
	}
	
}

//to SXC
bool GrpCCB::sendGrpPagingRsp2SXC(UINT8 Result, UINT32 UID)
{
	CMsg_Signal_VCR SAbisSignal;
	VoiceVCRCtrlMsgT *pData;
	if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpLAPagingRspT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
		SAbisSignal.SetDstTid(M_TID_VCR);
		SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		SAbisSignal.SetBTSSAGID();
		SAbisSignal.SetSigIDS(LAGrpPagingRsp_MSG);

		pData->sigPayload.GrpLAPagingRsp.Cause = Result;
		pData->sigPayload.GrpLAPagingRsp.commType = getCommType();
		VSetU32BitVal(pData->sigPayload.GrpLAPagingRsp.GrpL3Addr , getGrpL3Addr());
		VSetU32BitVal(pData->sigPayload.GrpLAPagingRsp.UID , UID);

		SAbisSignal.SetSigHeaderLengthField(sizeof(GrpLAPagingRspT));
		SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpLAPagingRspT));
		if(SAbisSignal.Post())
		{
			LOG5(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpLAPagingRsp---->SXC, GID[0x%04X] Result[0x%02X] commType[0x%02X] GrpL3Addr[0x%08X] UID[0x%08X]", 
				getGID(), Result, getCommType(), getGrpL3Addr(), UID);
			return true;
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send msg to VCR failed!!!");
			return false;
		}
	}	
}

bool GrpCCB::sendGrpResReq2SXC(UINT32 UID, UINT32 PID)
{
	CMsg_Signal_VCR SAbisSignal;
	VoiceVCRCtrlMsgT *pData;
	if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpResReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
		SAbisSignal.SetDstTid(M_TID_VCR);
		SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		SAbisSignal.SetBTSSAGID();
		SAbisSignal.SetSigIDS(GrpResReq_MSG);

		VSetU32BitVal(pData->sigPayload.GrpResReq.UID , UID);
		pData->sigPayload.GrpResReq.commType = getCommType();
		VSetU16BitVal(pData->sigPayload.GrpResReq.GID , getGID());
		VSetU32BitVal(pData->sigPayload.GrpResReq.PID , PID);

		SAbisSignal.SetSigHeaderLengthField(sizeof(GrpResReqT));
		SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpResReqT));
		if(SAbisSignal.Post())
		{
			LOG3(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpResReq---->SXC, GID[0x%04X]  commType[0x%02X] UID[0x%08X]", 
				getGID(), getCommType(), UID);	
			return true;
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send msg to VCR failed!!!");
			return false;
		}
	}	
}

bool GrpCCB::sendGrpHandoverReq2SXC(UINT32 UID, UINT32 PID, UINT32 curBTSID)
{
	CMsg_Signal_VCR SAbisSignal;
	VoiceVCRCtrlMsgT *pData;
	if ( !SAbisSignal.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(GrpHandoverReqT)) )
	{
		LOG(LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return false;
	}
	else
	{
		pData = (VoiceVCRCtrlMsgT*)SAbisSignal.GetDataPtr();
		SAbisSignal.SetDstTid(M_TID_VCR);
		SAbisSignal.SetMessageId(MSGID_VOICE_VCR_SIGNAL);
		SAbisSignal.SetBTSSAGID();
		SAbisSignal.SetSigIDS(GrpHandoverReq_MSG);

		VSetU16BitVal(pData->sigPayload.GrpHandoverReq.GID , getGID());
		VSetU32BitVal(pData->sigPayload.GrpHandoverReq.UID , UID);
		pData->sigPayload.GrpHandoverReq.HO_Type = HO_TYPE_CONVERSATION;
		pData->sigPayload.GrpHandoverReq.VersionInfo[0] = M_TERMINAL_TYPE_GRPCPE;
		pData->sigPayload.GrpHandoverReq.VersionInfo[1] = M_SWVERSION_V52PLUS;
		VSetU32BitVal(pData->sigPayload.GrpHandoverReq.PID , PID);
		VSetU32BitVal(pData->sigPayload.GrpHandoverReq.curBTSID , curBTSID);

		SAbisSignal.SetSigHeaderLengthField(sizeof(GrpHandoverReqT));
		SAbisSignal.SetPayloadLength(sizeof(SigHeaderT)+sizeof(GrpHandoverReqT));
		if(SAbisSignal.Post())
		{
			LOG2(LOG_DEBUG1, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
				"GrpHandoverReq---->SXC, GID[0x%04X]  UID[0x%08X]", 
				getGID(), UID);
			return true;
		}
		else
		{
			LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send msg to VCR failed!!!");
			return false;
		}
	}	
}

bool GrpCCB::sendGrpStatusReport2SXC()
{
	LOG(LOG_DEBUG3, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "sending Status to SXC..... ");
	statusReport.sendStatusReport2SXC();
	return true;
}

//to other BTS
#if 0
bool GrpCCB::sendHoResReq2otherBTS(CMessage& HoResReq)
{
	
}
#endif
bool GrpCCB::sendHoResRsp2otherBTS(CMessage& GrpResCfm)
#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
{
	UINT32 nNewDataLen = sizeof(BTSHoResRspT);
	UINT32 nPostDataLen = sizeof(BTSHoResRspT);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	if(GrpResCfm.GetDataLength()>sizeof(L2L3_GrpResCfmT))
	{
		nNewDataLen += sizeof(VideoCpeGrpResT);
	}
#endif
	CComMessage* pComMsg = new(CTVoice::GetInstance(),nNewDataLen) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	L2L3_GrpResCfmT* pDataL2L3 = (L2L3_GrpResCfmT*)GrpResCfm.GetDataPtr();
	BTSHoResRspT* pDataBTS = (BTSHoResRspT*)pComMsg->GetDataPtr();
	pDataBTS->cid = pDataL2L3->cid;
	memcpy(pDataBTS->btsID , pDataL2L3->btsID, 4);
	VSetU16BitVal(pDataBTS->GID , getGID());
	memcpy(pDataBTS->EID , pDataL2L3->EID, 4);
	pDataBTS->Result = pDataL2L3->Result;
	memcpy(pDataBTS->reportID , pDataL2L3->ReportID, 2);
	pDataBTS->transID = pDataL2L3->transID;
	pDataBTS->reportID_Ind = pDataL2L3->ReportIndexFlag;
	pDataBTS->airRes = pDataL2L3->airRes;	
	pComMsg->SetDataLength(nPostDataLen);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_EMSAGENTTX);
	pComMsg->SetMessageId(MSGID_GRP_HO_RES_RSP);
	pComMsg->SetBTS(VGetU32BitVal(pDataL2L3->btsID));
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	if(nNewDataLen>nPostDataLen)
	{
		if(pDataL2L3->Operation==GET_GRPRES_VIDEOUT)
		{
			memcpy( 	((UINT8*)pDataBTS)+sizeof(BTSHoResRspT), 
						((UINT8*)pDataL2L3)+sizeof(L2L3_GrpResCfmT), 
						sizeof(VideoCpeGrpResT) );
			nPostDataLen += sizeof(VideoCpeGrpResT);
			pComMsg->SetDataLength(nPostDataLen);
		}
	}
#endif	
	bool ret =  postComMsg(pComMsg);
	if(ret)
	{
		Counters.nSigToOtherBTS++;
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"HoResRsp---->ohterBTS, GID[0x%04X] Result[0x%02X] btsID[0x%08X] EID[0x%08X] cid[0x%02X]", 
			getGID(), pDataL2L3->Result, VGetU32BitVal(pDataL2L3->btsID), VGetU32BitVal(pDataL2L3->EID), pDataL2L3->cid);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send HoResRsp msg to other BTS failed!!!");
	}
	return ret;
}
#else //#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
{
	L2L3_GrpResCfmT* pDataL2L3 = (L2L3_GrpResCfmT*)GrpResCfm.GetDataPtr();
	AirResPart2T *pResPart2Src=NULL;
	AirResPart2T *pResPart2Dst=NULL;
	if(GrpResCfm.GetDataLength()>sizeof(L2L3_GrpResCfmT))
	{
		pResPart2Src = (AirResPart2T*)( ((UINT8*)pDataL2L3) + sizeof(L2L3_GrpResCfmT) );
	}
	UINT32 nNewDataLen = sizeof(BTSHoResRspT);
	if(pResPart2Src!=NULL)
	{
		//���ڵڶ�������Դ
		nNewDataLen += pResPart2Src->GetStructLen();//LV����
	}
	else
	{
		//û�еڶ�����
		nNewDataLen += 1;//LV����ֻ��L=0,û��V
	}
	//����������ʱû��
	
	CComMessage* pComMsg = new(CTVoice::GetInstance(),nNewDataLen) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	BTSHoResRspT* pDataBTS = (BTSHoResRspT*)pComMsg->GetDataPtr();
	pDataBTS->cid = pDataL2L3->cid;
	memcpy(pDataBTS->btsID , pDataL2L3->btsID, 4);
	VSetU16BitVal(pDataBTS->GID , getGID());
	memcpy(pDataBTS->EID , pDataL2L3->EID, 4);
	pDataBTS->Result = pDataL2L3->Result;
	memcpy(pDataBTS->reportID , pDataL2L3->ReportID, 2);
	pDataBTS->transID = pDataL2L3->transID;
	pDataBTS->reportID_Ind = pDataL2L3->ReportIndexFlag;
	pDataBTS->airRes = pDataL2L3->airRes;	
	//��Դ�ڶ�����begin
	pResPart2Dst = (AirResPart2T*)( ((UINT8*)pDataBTS) + sizeof(BTSHoResRspT) );
	if(pResPart2Src!=NULL)
	{
		memcpy(&pResPart2Dst->len, &pResPart2Src->len, pResPart2Src->GetStructLen());
	}
	else
	{
		pResPart2Dst->len = 0;
	}
	//��Դ�ڶ�����end	
	//����������ʱû��
	
	pComMsg->SetDataLength(nNewDataLen);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_EMSAGENTTX);
	pComMsg->SetMessageId(MSGID_GRP_HO_RES_RSP);
	pComMsg->SetBTS(VGetU32BitVal(pDataL2L3->btsID));
	bool ret =  postComMsg(pComMsg);
	if(ret)
	{
		Counters.nSigToOtherBTS++;
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"HoResRsp---->ohterBTS, GID[0x%04X] Result[0x%02X] btsID[0x%08X] EID[0x%08X] cid[0x%02X]", 
			getGID(), pDataL2L3->Result, VGetU32BitVal(pDataL2L3->btsID), VGetU32BitVal(pDataL2L3->EID), pDataL2L3->cid);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send HoResRsp msg to other BTS failed!!!");
	}
	return ret;
}
#endif //#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
//to cpe
bool GrpCCB::sendHoResRsp2CPE(CMessage& HoResRsp)
#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
{
	UINT32 nNewDataLen = sizeof(CPEHoResRspT);
	UINT32 nPostDataLen = sizeof(CPEHoResRspT);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	if(HoResRsp.GetDataLength()>sizeof(BTSHoResRspT))
	{
		nNewDataLen += sizeof(VideoCpeGrpResT);
	}
#endif
	
	CComMessage* pComMsg = new(CTVoice::GetInstance(), nNewDataLen) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	BTSHoResRspT* pDataBTS = (BTSHoResRspT*)HoResRsp.GetDataPtr();
	CPEHoResRspT* pDataCPE = (CPEHoResRspT*)pComMsg->GetDataPtr();

	pDataCPE->cid= pDataBTS->cid;
	memcpy(pDataCPE->btsID , pDataBTS->btsID, 4);
	memcpy(pDataCPE->GID , pDataBTS->GID, 2);
	pDataCPE->Result= pDataBTS->Result;
	pDataCPE->reportID_Ind= pDataBTS->reportID_Ind;
	pDataCPE->transID= pDataBTS->transID;
	pDataCPE->airRes= pDataBTS->airRes;
	memcpy(pDataCPE->reportID, pDataBTS->reportID, 2);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	if(nNewDataLen>nPostDataLen)
	{
		memcpy( 	((UINT8*)pDataCPE)+sizeof(CPEHoResRspT), 
					((UINT8*)pDataBTS)+sizeof(BTSHoResRspT), 
					sizeof(VideoCpeGrpResT) );
		nPostDataLen += sizeof(VideoCpeGrpResT);
	}
#endif
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_CPECM);
	pComMsg->SetMessageId(MSGID_GRP_DAC_HO_RES_RSP);
	pComMsg->SetDataLength(nPostDataLen);
	pComMsg->SetEID(VGetU32BitVal(pDataBTS->EID));

	bool ret =  postComMsg(pComMsg);
	if(ret)
	{
		Counters.nSigToDAC++;
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"HoResRsp---->CPE, GID[0x%04X] Result[0x%02X] btsID[0x%08X] EID[0x%08X] cid[0x%02X]", 
			getGID(), pDataBTS->Result, VGetU32BitVal(pDataBTS->btsID), VGetU32BitVal(pDataBTS->EID), pDataBTS->cid);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send HoResRsp msg to CPE failed!!!");
	}
	return ret;
}
#else //#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
{
	BTSHoResRspT* pDataBTS = (BTSHoResRspT*)HoResRsp.GetDataPtr();
	AirResPart2T *pResPart2Src=NULL;
	AirResPart2T *pResPart2Dst=NULL;
	if(HoResRsp.GetDataLength()>sizeof(BTSHoResRspT))
	{
		pResPart2Src = (AirResPart2T*)( ((UINT8*)pDataBTS) + sizeof(BTSHoResRspT) );
	}
	UINT32 nNewDataLen = sizeof(CPEHoResRspT);
	if(pResPart2Src!=NULL)
	{
		//���ڵڶ�������Դ
		nNewDataLen += pResPart2Src->GetStructLen();//LV����
	}
	else
	{
		//û�еڶ�����
		nNewDataLen += 1;//LV����ֻ��L=0,û��V
	}
	//����������ʱû��
	
	CComMessage* pComMsg = new(CTVoice::GetInstance(), nNewDataLen) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	CPEHoResRspT* pDataCPE = (CPEHoResRspT*)pComMsg->GetDataPtr();

	pDataCPE->cid= pDataBTS->cid;
	memcpy(pDataCPE->btsID , pDataBTS->btsID, 4);
	memcpy(pDataCPE->GID , pDataBTS->GID, 2);
	pDataCPE->Result= pDataBTS->Result;
	pDataCPE->reportID_Ind= pDataBTS->reportID_Ind;
	pDataCPE->transID= pDataBTS->transID;
	pDataCPE->airRes= pDataBTS->airRes;
	memcpy(pDataCPE->reportID, pDataBTS->reportID, 2);
	//��Դ�ڶ�����begin
	pResPart2Dst = (AirResPart2T*)( ((UINT8*)pDataCPE) + sizeof(CPEHoResRspT) );
	if(pResPart2Src!=NULL)
	{
		memcpy(&pResPart2Dst->len, &pResPart2Src->len, pResPart2Src->GetStructLen());
	}
	else
	{
		pResPart2Dst->len = 0;
	}
	//��Դ�ڶ�����end	
	//����������ʱû��
	
	pComMsg->SetDataLength(nNewDataLen);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_CPECM);
	pComMsg->SetMessageId(MSGID_GRP_DAC_HO_RES_RSP);
	pComMsg->SetEID(VGetU32BitVal(pDataBTS->EID));
	bool ret =  postComMsg(pComMsg);
	if(ret)
	{
		Counters.nSigToDAC++;
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"HoResRsp---->CPE, GID[0x%04X] Result[0x%02X] btsID[0x%08X] EID[0x%08X] cid[0x%02X]", 
			getGID(), pDataBTS->Result, VGetU32BitVal(pDataBTS->btsID), VGetU32BitVal(pDataBTS->EID), pDataBTS->cid);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send HoResRsp msg to CPE failed!!!");
	}
	return ret;
}
#endif //#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
bool GrpCCB::sendGrpShareResRsp2CPE(CMessage& GrpResCfm)
#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
{
	UINT32 nNewDataLen = sizeof(CPEGrpResRspT);
	UINT32 nPostDataLen = sizeof(CPEGrpResRspT);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	if(GrpResCfm.GetDataLength()>sizeof(L2L3_GrpResCfmT))
	{
		nNewDataLen += sizeof(VideoCpeGrpResT);
	}
#endif	
	CComMessage* pComMsg = new(CTVoice::GetInstance(), nNewDataLen) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	L2L3_GrpResCfmT* pDataL2L3 = (L2L3_GrpResCfmT*)GrpResCfm.GetDataPtr();
	CPEGrpResRspT* pDataCPE = (CPEGrpResRspT*)pComMsg->GetDataPtr();

	pDataCPE->cid = pDataL2L3->cid;
	VSetU16BitVal(pDataCPE->GID , getGID());
	pDataCPE->Result = pDataL2L3->Result;
	memcpy(pDataCPE->reportID , pDataL2L3->ReportID, 2);
	pDataCPE->transID = pDataL2L3->transID;
	pDataCPE->airRes = pDataL2L3->airRes;
	pDataCPE->reportID_Ind = pDataL2L3->ReportIndexFlag;

	pComMsg->SetEID(VGetU32BitVal(pDataL2L3->EID));
	pComMsg->SetDataLength(nPostDataLen);
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_UTV);
	pComMsg->SetMessageId(MSGID_GRP_DAC_RES_RSP);
#ifdef M_VIDEOUT_GRPRES_OPTIMIZE
	if(nNewDataLen>nPostDataLen)
	{
		if(pDataL2L3->Operation==GET_GRPRES_VIDEOUT)
		{
			memcpy( 	((UINT8*)pDataCPE)+sizeof(CPEGrpResRspT), 
						((UINT8*)pDataL2L3)+sizeof(L2L3_GrpResCfmT), 
						sizeof(VideoCpeGrpResT) );
			nPostDataLen += sizeof(VideoCpeGrpResT);
			pComMsg->SetDataLength(nPostDataLen);
		}
	}
#endif	
	bool ret =  postComMsg(pComMsg);
	if(ret)
	{
		Counters.nSigToDAC++;
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GrpShareResRsp---->CPE, GID[0x%04X] Result[0x%02X] reportID[0x%04X] EID[0x%08X] cid[0x%02X]", 
			getGID(), pDataL2L3->Result, VGetU16BitVal(pDataL2L3->ReportID), VGetU32BitVal(pDataL2L3->EID), pDataL2L3->cid);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send GrpShareResRsp msg to CPE failed!!!");
	}
	return ret;
	
}
#else //#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
{
	L2L3_GrpResCfmT* pDataL2L3 = (L2L3_GrpResCfmT*)GrpResCfm.GetDataPtr();
	AirResPart2T *pResPart2Src=NULL;
	AirResPart2T *pResPart2Dst=NULL;
	if(GrpResCfm.GetDataLength()>sizeof(L2L3_GrpResCfmT))
	{
		pResPart2Src = (AirResPart2T*)( ((UINT8*)pDataL2L3) + sizeof(L2L3_GrpResCfmT) );
	}
	UINT32 nNewDataLen = sizeof(CPEGrpResRspT);
	if(pResPart2Src!=NULL)
	{
		//���ڵڶ�������Դ
		nNewDataLen += pResPart2Src->GetStructLen();//LV����
	}
	else
	{
		//û�еڶ�����
		nNewDataLen += 1;//LV����ֻ��L=0,û��V
	}
	//����������ʱû��
#ifdef M__SUPPORT__ENC_RYP_TION	
		nNewDataLen += 1;
		if(ENCRYPT_CTRL_NO_ENCRYPTION!=getEncryptFlag())
		{
			nNewDataLen += M_ENCRYPT_KEY_LENGTH;
		}
#endif	
	CComMessage* pComMsg = new(CTVoice::GetInstance(), nNewDataLen) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	CPEGrpResRspT* pDataCPE = (CPEGrpResRspT*)pComMsg->GetDataPtr();

	pDataCPE->cid = pDataL2L3->cid;
	VSetU16BitVal(pDataCPE->GID , getGID());
	pDataCPE->Result = pDataL2L3->Result;
	memcpy(pDataCPE->reportID , pDataL2L3->ReportID, 2);
	pDataCPE->transID = pDataL2L3->transID;
	pDataCPE->airRes = pDataL2L3->airRes;
	pDataCPE->reportID_Ind = pDataL2L3->ReportIndexFlag;
	//��Դ�ڶ�����begin
	pResPart2Dst = (AirResPart2T*)( ((UINT8*)pDataCPE) + sizeof(CPEGrpResRspT) );
	if(pResPart2Src!=NULL)
	{
		memcpy(&pResPart2Dst->len, &pResPart2Src->len, pResPart2Src->GetStructLen());
	}
	else
	{
		pResPart2Dst->len = 0;
	}
	//��Դ�ڶ�����end	
	//����������ʱû��
#ifdef M__SUPPORT__ENC_RYP_TION	
	UINT8 *pDataEnc = (UINT8*)pResPart2Dst + pResPart2Src->len + 1;
	pDataEnc[0] = getEncryptFlag();
	if(ENCRYPT_CTRL_NO_ENCRYPTION!=getEncryptFlag())
	{
		memcpy((void*)&pDataEnc[1], getEncryptKey(), M_ENCRYPT_KEY_LENGTH);
	}
#endif
	pComMsg->SetDataLength(nNewDataLen);
	pComMsg->SetEID(VGetU32BitVal(pDataL2L3->EID));
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_UTV);
	pComMsg->SetMessageId(MSGID_GRP_DAC_RES_RSP);
	bool ret =  postComMsg(pComMsg);
	if(ret)
	{
		Counters.nSigToDAC++;
		LOG5(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GrpShareResRsp---->CPE, GID[0x%04X] Result[0x%02X] reportID[0x%04X] EID[0x%08X] cid[0x%02X]", 
			getGID(), pDataL2L3->Result, VGetU16BitVal(pDataL2L3->ReportID), VGetU32BitVal(pDataL2L3->EID), pDataL2L3->cid);		
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send GrpShareResRsp msg to CPE failed!!!");
	}
	return ret;
	
}
#endif //#ifndef M_L2L3_GRPRES_USE_LV_STRUCT
bool GrpCCB::sendGrpRelease2CPE(UINT8 reason)
{
	UINT8 buf[2];
	buf[0] = M_MSGTYPE_GRP_RLS;
	buf[1] = reason;
	LOG2(LOG_DEBUG, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
		"GrpRelease(not from SXC)---->CPEs, GID[0x%04X] Reason[0x%02X] ##########", 
		getGID(), reason);
	return sendGrpSignal2L2(buf,sizeof(buf));
}

bool GrpCCB::sendGrpCallInd2CPE(UINT8 type, UINT8 LEFlag, UINT8 transID)
{
	CComMessage* pComMsg = new(CTVoice::GetInstance(),sizeof(GrpCallIndT)) CComMessage;
	V__AssertRtnV(NULL!=pComMsg, LOG_SEVERE, LOGNO(GRPSRV, EC_L3VOICE_SYS_FAIL), 
		"CreateMessage failed!!!", ;, false);	
	GrpCallIndT* pDataCPE = (GrpCallIndT*)pComMsg->GetDataPtr();

	pDataCPE->cid = DEFAULT_CID;
	VSetU16BitVal(pDataCPE->GID , getGID());
	pDataCPE->callPrioty = getPrioty();
	pDataCPE->pagingType = type;
	pDataCPE->LEFlag = LEFlag;
	pDataCPE->transID = transID;
#ifdef M__SUPPORT__ENC_RYP_TION
	pDataCPE->EncryptFlag = getEncryptFlag();
	if(getEncryptFlag()!=ENCRYPT_CTRL_NOTUSE)
	{
		memcpy(pDataCPE->EncryptKey, getEncryptKey(), sizeof(pDataCPE->EncryptKey));
	}
#endif	
	pComMsg->SetDataLength(sizeof(GrpCallIndT));
	pComMsg->SetSrcTid(M_TID_VOICE);
	pComMsg->SetDstTid(M_TID_VAC);
	pComMsg->SetMessageId(MSGID_GRP_DAC_CALL_IND);
	bool ret =  postComMsg(pComMsg);
	if(ret)
	{
		LOG4(LOG_DEBUG2, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), 
			"GrpCallInd---->btsL2, GID[0x%04X] pagingType[0x%02X] LEFlag[0x%02X] cid[0x%02X]", 
			getGID(), type, LEFlag, pDataCPE->cid);
	}
	else
	{
		LOG(LOG_WARN, LOGNO(GRPSRV, EC_L3VOICE_NORMAL), "send GrpCallInd msg to btsL2 failed!!!");
	}
	return ret;
}



void GrpCCB::showCCBInfo()
{
	VPRINT("\nGRPCCB Info: ==============\n");
	VPRINT(" GID[0x%08X] State[0x%02x] GRPL3Addr[0x%08X]",
		getGID(), 
		GetCurrentState(), 
		getGrpL3Addr());
	VPRINT("\n SrvType[0x%02X] prioty[0x%02x] EncryptFlag[0x%02X] transID[0x%02X]",
		getCommType(), 
		getPrioty(), 
		getEncryptFlag(),
		getTransID());
	VPRINT("\n isResReady[0x%02X] ResReason[0x%02x] ",
		airRes.isResReady(),
		airRes.getResReason());
	VPRINT("\n SCG_INDEX[0x%02X] DL_DSCH_SLOT_INDEX[0x%02x] DL_DSCH_SCH_INDEX[0x%02X] UL_SLOT_INDEX[0x%02X] UL_SCH_INDEX[0x%02X] ReportPeriod[0x%04X] BackoffWin[0x%04X] EncryptFlag[0x%02X]",	
		airRes.airRes.SCG_INDEX,
		airRes.airRes.DL_DSCH_SLOT_INDEX,
		airRes.airRes.DL_DSCH_SCH_INDEX,
		airRes.airRes.UL_SLOT_INDEX,
		airRes.airRes.UL_SCH_INDEX,
		VGetU16BitVal(airRes.airRes.ReportPeriod),
		VGetU16BitVal(airRes.airRes.BackoffWin),
		airRes.airRes.EncryptFlag);
	VPRINT("\n cpesCnt[%d] GrpCpesCounters0[%d] GrpCpesCounters1[%d] GrpCpesCounters2[%d] GrpCpesCounters3[%d]",
		statusReport.cpesCnt, 
		statusReport.GrpCpesCounters[0], 
		statusReport.GrpCpesCounters[1],
		statusReport.GrpCpesCounters[2],
		statusReport.GrpCpesCounters[3]);	
	VPRINT("\n===========================\n");
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////


//for test begin
VoiceCCB* findCCBByUid(UINT32 uid)
{
	CMsg_Signal_VCR LAPagingReq;
	VoiceVCRCtrlMsgT *pData;
	if ( !LAPagingReq.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(LAPagingT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return NULL;
	}
	else
	{
		LAPagingReq.SetSrcTid(M_TID_VCR);
		LAPagingReq.SetDstTid(M_TID_VOICE);
		LAPagingReq.SetMessageId(MSGID_VCR_VOICE_SIGNAL);
		LAPagingReq.SetBTSSAGID();
		LAPagingReq.SetSigIDS(LAPaging_MSG);

		pData = (VoiceVCRCtrlMsgT*)LAPagingReq.GetDataPtr();
		VSetU16BitVal(pData->sigPayload.LAPaging.App_Type , APPTYPE_SMS);
		VSetU32BitVal(pData->sigPayload.LAPaging.L3addr , 1);
		VSetU32BitVal(pData->sigPayload.LAPaging.UID , uid);
			
		LAPagingReq.SetSigHeaderLengthField(sizeof(LAPagingT));
		LAPagingReq.SetPayloadLength(sizeof(SigHeaderT)+sizeof(LAPagingT));
		VoiceCCB* pCCB = (VoiceCCB*)CTVoice::GetInstance()->getVoiceFSM()->FindCCB(LAPagingReq);
		LAPagingReq.DeleteMessage();
		return pCCB;
    }
}

void VoiceCCBTable::showBTree(bool blDetail)
{
	map<VoiceTuple, UINT16>::iterator itTuple, itPlayTone;
	map<UINT32, UINT16>::iterator itUID,itL3Addr;

	VPRINT("\n====================================================================\n");
	VPRINT("\nCCB Pool Size[%d], Free[%d]\n", VOICE_CCB_NUM, FreeCCBList.size());
	VPRINT("\n----------BTreeByTuple Size[%d]  Format[ (EID,CID) , &CCB ]-------\n", BTreeByTuple.size());
	if(blDetail)
		for(itTuple=BTreeByTuple.begin();itTuple!=BTreeByTuple.end();itTuple++)
		{
			VPRINT(" [ (0x%08X,0x%02X) , 0x%08X ]", 
				(*itTuple).first.Eid, (*itTuple).first.Cid, 
				(UINT32)&CCBTable[(*itTuple).second]);
		}
	VPRINT("\n----------BTreeByUID Size[%d]    Format[ UID , &CCB ]-------------\n", BTreeByUID.size());
	if(blDetail)
		for(itUID=BTreeByUID.begin();itUID!=BTreeByUID.end();itUID++)
		{
			VPRINT(" [ 0x%08X , 0x%08X ]", 
				(*itUID).first,  
				(UINT32)&CCBTable[(*itUID).second]);
		}
	VPRINT("\n----------BTreeByL3Addr Size[%d] Format[ L3Addr , &CCB ]----------\n", BTreeByL3addr.size());
	if(blDetail)
		for(itL3Addr=BTreeByL3addr.begin();itL3Addr!=BTreeByL3addr.end();itL3Addr++)
		{
			VPRINT(" [ 0x%08X , 0x%08X ]", 
				(*itL3Addr).first, 
				(UINT32)&CCBTable[(*itL3Addr).second]);
		}
	VPRINT("\n----------BTreePlayToneTbl Size[%d] Format[ UID , toneID ]----------\n", BTreePlayToneTbl.size());
	if(blDetail)
		for(itPlayTone=BTreePlayToneTbl.begin();itPlayTone!=BTreePlayToneTbl.end();itPlayTone++)
		{
			VPRINT(" [ (0x%08X,0x%02X) , 0x%04X ]", 
				(*itPlayTone).first.Eid, (*itPlayTone).first.Cid, 
				(*itPlayTone).second);
		}	VPRINT("\n====================================================================\n");
}


void VoiceFSM::showBTree(bool blDetail)
{
	CCBTable->showBTree(blDetail);
}

extern "C" {

void queryIPListTest(UINT32 eid)
{
	typedef struct _testPayload
	{
		UINT8 cid;
		UINT8 msgType;
		UINT8 PID[4];
		UINT8 funcCode;
		UINT8 len;
	}testPayloadT;
	//������Ϣ
	CComMessage *pMsg = new (CTVoice::GetInstance(), sizeof(testPayloadT)) CComMessage;
	if ( pMsg!=NULL )
   	{
		pMsg->SetEID(eid);
		pMsg->SetMessageId(MSGID_DAC_DL_CPEOAM_MSG);
		pMsg->SetDstTid(M_TID_CPECM);
		pMsg->SetSrcTid(M_TID_VOICE);
		pMsg->SetDataLength(sizeof(testPayloadT));
		testPayloadT *pData = (testPayloadT*)pMsg->GetDataPtr();
		pData->cid = 0;
		pData->msgType = 0x53;
		VSetU32BitVal(pData->PID, eid);
		pData->funcCode = 1;
		pData->len = 0;
		postComMsg(pMsg);
	}
	else
	{
		LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
		return;
	}
}

void closeHeartBeat()
{
	blUseBeatHeart = false;
}
void openHeartBeat()
{
	blUseBeatHeart = true;
}

void showBTree(bool blDetail)
{
	CTVoice::GetInstance()->getVoiceFSM()->showBTree(blDetail);
}

void regCPEVoice(UINT32 uid, UINT32 eid, UINT8 cid)
{
	CComMessage *pRegisterMsg = new (CTVoice::GetInstance(), 20) CComMessage;
	CMVoiceRegMsgT *pRegisterData = (CMVoiceRegMsgT*)pRegisterMsg->GetDataPtr();

	pRegisterMsg->SetDstTid(M_TID_VOICE);
	pRegisterMsg->SetSrcTid(M_TID_CM);
	pRegisterMsg->SetMessageId(MSGID_VOICE_UT_REG);
	
	pRegisterMsg->SetEID(eid);
	
	pRegisterData->Cid = cid;
	VSetU32BitVal(pRegisterData->Uid , uid);

	pRegisterMsg->SetDataLength(sizeof(CMVoiceRegMsgT));
	if(!CComEntity::PostEntityMessage(pRegisterMsg))
	{
		pRegisterMsg->Destroy();
	};

}
void pagingCPEVoice(UINT32 uid)
{
	CMsg_Signal_VCR LAPagingReq;
	VoiceVCRCtrlMsgT *pData;
	if ( !LAPagingReq.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(LAPagingT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		LAPagingReq.SetSrcTid(M_TID_VCR);
		LAPagingReq.SetDstTid(M_TID_VOICE);
		LAPagingReq.SetMessageId(MSGID_VCR_VOICE_SIGNAL);
		LAPagingReq.SetBTSSAGID();
		LAPagingReq.SetSigIDS(LAPaging_MSG);

		pData = (VoiceVCRCtrlMsgT*)LAPagingReq.GetDataPtr();
		VSetU16BitVal(pData->sigPayload.LAPaging.App_Type , APPTYPE_SMS);
		VSetU32BitVal(pData->sigPayload.LAPaging.L3addr , 1);
		VSetU32BitVal(pData->sigPayload.LAPaging.UID , uid);
//		pData->sigPayload.LAPaging.VersionInfo[]
			
		LAPagingReq.SetSigHeaderLengthField(sizeof(LAPagingT));
		LAPagingReq.SetPayloadLength(sizeof(SigHeaderT)+sizeof(LAPagingT));
		LAPagingReq.Post();
    }    
}	

//20090531 fengbing broadcast SMS test begin
#ifdef BROADCAST_SMS_TEST
#ifndef DSP_BIOS
#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif
#endif
//part1
typedef struct _SMS_Part1
{
	UINT8 	UID[4];
	UINT8	SMS_DataLen;
}SMS_Part1;

//SMS_BearerData


//BEARER_DATA

//part2
typedef struct _SMS_Part2
{
	UINT8	SMSType_label;	//01H	
	UINT8	SMSType_len;	//01H	
	UINT8	SMSType;	//01H	
	UINT8	MESSAGEID_label;	//02H	
	UINT8	MESSAGEID_len;	//04H	
	UINT8	MESSAGEID[4];	//0000H-ffffH	����MESSAGEID��Ϊ���ŵ�Ψһ��ʶ���㲥�ط�ʱ����MESSAGEID���䡣�û����·����������ʱ��������һ��MESSAGEID
	UINT8	SMSContent_label;	//03H	
	UINT8	SMSContent_len;	//Variable	1�ֽڳ���
}SMS_Part2;
//part3

typedef struct _SMS_Part3
{
	UINT8	SMSContent[MAX_SMS_BCHSMSTEST_LENGTH+1];
	UINT32 getLength()
		{ 
			UINT32 nLen = strlen((char*)SMSContent); 
			if( nLen>MAX_SMS_BCHSMSTEST_LENGTH )
				return MAX_SMS_BCHSMSTEST_LENGTH;
			else
				return nLen;
		}
}SMS_Part3;

//part4
typedef struct _SMS_Part4
{
	UINT8	Time_label;	//09H	
	UINT8	Time_len;		
	UINT8	Time[14];		//12�ֽڳ���
}SMS_Part4;

//part5
typedef struct _SMS_Part5
{
	UINT8	SMS_CallNumLen;	//�绰���볤��,1byte
	UINT8	SMS_CalledNo[30];	//����Ϣ����(BCD��),�̶�Ϊ010-12345678
	UINT32 getLength(){return (SMS_CallNumLen+1);};
}SMS_Part5;
#ifndef DSP_BIOS
#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif
#endif
extern "C" void stopBchSmsTest()
{
	strcpy(g_BchSMSStr,"");
}
extern "C" void bchSmsTest(char *smsStr, char *displayNumber)
{
	//��Ϣ��ֻ����һ��MT����Ϣ,������Broadcast_SM��ʽ

	if(smsStr==NULL || smsStr==(char*)0xffffffff)
	{
		VPRINT("\nbchSmsTest(TEST BCH SMS TEXT, display Number)\n");
		return;		
	}
	if(strlen(smsStr)==0)
	{
		return;
	}
	if(strlen(smsStr)>MAX_SMS_BCHSMSTEST_LENGTH)
	{
		VPRINT("\nsms text is too long !!!\n");
		return;
	}

	if(displayNumber!=NULL && displayNumber!=(char*)0xffffffff)
	{
		if(strlen(displayNumber)<50)
		{
			strcpy(g_BchSmsTestDisplayNumber, displayNumber);
		}
	}
	
	UINT8 i;
	UINT8 nNumLen = strlen(g_BchSmsTestDisplayNumber);
	UINT8 nSmsStrLen = strlen(smsStr);
	if(strncmp(smsStr, g_BchSMSStr, MAX_SMS_BCHSMSTEST_LENGTH)!=0)
	{
		g_nBchSMSTestSN++;
		strncpy(g_BchSMSStr, smsStr, MAX_SMS_BCHSMSTEST_LENGTH);
	}
	else
	{
	}
	
	SMS_Part1 part1;
	SMS_Part2 part2;
	SMS_Part3 part3;
	SMS_Part4 part4;
	SMS_Part5 part5;
//part1
	VSetU32BitVal(part1.UID , 0xffffffff);
	part1.SMS_DataLen = sizeof(SMS_Part2) + nSmsStrLen + sizeof(SMS_Part4);//len =  p2+p3+p4;
//part2
	part2.SMSType_label = 0x1;
	part2.SMSType_len = 0x1;
	part2.SMSType = 0x1;//MT SMS

	part2.MESSAGEID_label = 0x2;
	part2.MESSAGEID_len = 0x4;
	VSetU32BitVal(part2.MESSAGEID , g_nBchSMSTestSN);
	
	part2.SMSContent_label = 0x3;
	part2.SMSContent_len = nSmsStrLen;
//part3
	strcpy((char*)part3.SMSContent, smsStr);
//part4
	part4.Time_label = 0x9;
	part4.Time_len = 14;
	T_TimeDate TimeData = bspGetDateTime();
	sprintf((char*)part4.Time, "%04d%02d%02d%02d%02d%02d", TimeData.year, TimeData.month, TimeData.day, TimeData.hour, TimeData.minute, TimeData.second);
//part5
	part5.SMS_CallNumLen = (nNumLen & 0x1) ? nNumLen/2+1 : nNumLen/2;
	for(i=0;i<part5.SMS_CallNumLen;i++)
	{
		part5.SMS_CalledNo[i] = (g_BchSmsTestDisplayNumber[i*2]-'0') | ((g_BchSmsTestDisplayNumber[i*2+1]-'0')<<4);
	}
	if((nNumLen & 0x1))
	{
		part5.SMS_CalledNo[part5.SMS_CallNumLen-1] |= 0xF0;
	}

	UINT8 nPayloadLen = sizeof(part1) + sizeof(part2) + part3.getLength() + sizeof(part4) + part5.getLength();
	UINT8 nSigPayloadLen = sizeof(Broadcast_SMT) + nPayloadLen;

	CMsg_Signal_VCR BroadCastSMReq;
	VoiceVCRCtrlMsgT *pData;
	
	if ( !BroadCastSMReq.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+nSigPayloadLen))
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		BroadCastSMReq.SetSrcTid(M_TID_VCR);
		BroadCastSMReq.SetDstTid(M_TID_VOICE);
		BroadCastSMReq.SetMessageId(MSGID_VCR_VOICE_SIGNAL);
		BroadCastSMReq.SetBTSSAGID();
		BroadCastSMReq.SetSigIDS(BROADCAST_SM_MSG);

		pData = (VoiceVCRCtrlMsgT*)BroadCastSMReq.GetDataPtr();
#if 1		
		VSetU16BitVal(pData->sigPayload.Broadcast_SM.SequenceNumber,0xff);
		VSetU32BitVal(pData->sigPayload.Broadcast_SM.GlobalMsgSeq,g_nBchSMSTestSN);
		VSetU16BitVal(pData->sigPayload.Broadcast_SM.ServiceCategory,0);
		VSetU16BitVal(pData->sigPayload.Broadcast_SM.ServiceCategoryIndicator,0);
		pData->sigPayload.Broadcast_SM.BroadcastData=0xff;
#endif
		UINT8* pPayload = &pData->sigPayload.Broadcast_SM.BroadcastData+1;
		//part1
		memcpy(pPayload, &part1, sizeof(part1));
		pPayload += sizeof(part1);
		//part2
		memcpy(pPayload, &part2, sizeof(part2));
		pPayload += sizeof(part2);
		//part3
		memcpy(pPayload, &part3, part3.getLength());
		pPayload += part3.getLength();
		//part4
		memcpy(pPayload, &part4, sizeof(part4));
		pPayload += sizeof(part4);
		//part5
		memcpy(pPayload, &part5, part5.getLength());
		pPayload += part5.getLength();

		BroadCastSMReq.SetSigHeaderLengthField(nSigPayloadLen);
		BroadCastSMReq.SetPayloadLength(sizeof(SigHeaderT)+nSigPayloadLen);
		BroadCastSMReq.Post();
	}	 
	
}

#endif//BROADCAST_SMS_TEST
//20090531 fengbing broadcast SMS test end

/*edit by yhw,add tmp-message to support Broatcast-SM*/
void BroadCastSM()
{
	CMsg_Signal_VCR BroadCastSMReq;
	VoiceVCRCtrlMsgT *pData;
	if ( !BroadCastSMReq.CreateMessage(*CTVoice::GetInstance(), sizeof(SigHeaderT)+sizeof(Broadcast_SMT)) )
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "CreateMessage failed!!!");
	}
	else
	{
		BroadCastSMReq.SetSrcTid(M_TID_VCR);
		BroadCastSMReq.SetDstTid(M_TID_VOICE);
		BroadCastSMReq.SetMessageId(MSGID_VCR_VOICE_SIGNAL);
		BroadCastSMReq.SetBTSSAGID();
		BroadCastSMReq.SetSigIDS(BROADCAST_SM_MSG);

		pData = (VoiceVCRCtrlMsgT*)BroadCastSMReq.GetDataPtr();
		VSetU16BitVal(pData->sigPayload.Broadcast_SM.SequenceNumber,0);
		VSetU32BitVal(pData->sigPayload.Broadcast_SM.GlobalMsgSeq,0);
		VSetU16BitVal(pData->sigPayload.Broadcast_SM.ServiceCategory,0);
		VSetU16BitVal(pData->sigPayload.Broadcast_SM.ServiceCategoryIndicator,0);
		pData->sigPayload.Broadcast_SM.BroadcastData=0xff;

		BroadCastSMReq.SetSigHeaderLengthField(sizeof(Broadcast_SMT));
		BroadCastSMReq.SetPayloadLength(sizeof(SigHeaderT)+sizeof(Broadcast_SMT));
		BroadCastSMReq.Post();
	}    
}	
//end


void showCCBState(UINT32 uid)
{
	VoiceCCB* pCCB = findCCBByUid(uid);

	if(pCCB!=NULL)
	{
		VPRINT("\n======================\n");
		VPRINT("CCB Info: UID[0x%08X] State[%s] L3Addr[0x%08X]",
			pCCB->getUID(), 
			&(VoiceStateBase::m_stateName[pCCB->GetCurrentState()][0]), 
			pCCB->getL3addr());
		VPRINT("\n======================\n");
	}
}

void clearCCBState(UINT32 uid)
{
	VoiceCCB* pCCB = findCCBByUid(uid);
	if(pCCB==NULL)
		return;
	
	VoiceTuple tuple = pCCB->getVoiceTuple();
	UINT32 eid = tuple.Eid;
	UINT8 cid = tuple.Cid;

	CComMessage* pUnRegMsg = new (CTVoice::GetInstance(), 20) CComMessage;
	CMVoiceRegMsgT* pDataUnReg = (CMVoiceRegMsgT*)pUnRegMsg->GetDataPtr();
	pUnRegMsg->SetSrcTid(M_TID_CM);
	pUnRegMsg->SetDstTid(M_TID_VOICE);
	pUnRegMsg->SetMessageId(MSGID_VOICE_UT_UNREG);
	pUnRegMsg->SetEID(eid);
	VSetU32BitVal(pDataUnReg->Uid , uid);
	pDataUnReg->Cid = cid;
	pUnRegMsg->SetDataLength(sizeof(CMVoiceRegMsgT));
	if(!CComEntity::PostEntityMessage(pUnRegMsg))
	{
		pUnRegMsg->Destroy();
	}
	
	regCPEVoice(uid, eid, cid);

	pCCB = findCCBByUid(uid);

	if(pCCB!=NULL)
	{
		pCCB->SetCurrentState(VOICE_IDLE_STATE);
	}
	
}

void showCounters()
{
	
 	VPRINT("\n========================================================================================================\n");
	VPRINT(" tVoice Signals:   \n\tnSigFromVAC\t[%10d]  \n\tnSigFromDAC\t[%10d]  \n\tnSigToVAC\t[%10d]  \n\tnSigToDAC\t[%10d]",
		Counters.nSigFromVAC, Counters.nSigFromDAC, Counters.nSigToVAC, Counters.nSigToDAC);
	VPRINT("\n\tnSigToOtherBTS\t[%10d]  \n\tnSigFromOtherBTS\t[%10d]",
		Counters.nSigToOtherBTS, Counters.nSigFromOtherBTS);
	VPRINT("\n\tnSigToVCR\t[%10d]  \n\tnSigFromVCR\t[%10d]",
		Counters.nSigToVCR, Counters.nSigFromVCR);
	VPRINT("\n tVCR Signals:   \n\tnSigToSAG\t[%10d]  \n\tnSigFromSAG\t[%10d]  \n\tnSigToTvoice\t[%10d]  \n\tnSigFromTvoice\t[%10d]  \n\tnSigToUm\t[%10d]  \n\tnSigFromUm\t[%10d]",
		Counters.nSigToSAG, Counters.nSigFromSAG, Counters.nSigToTvoice, Counters.nSigFromTvoice, Counters.nSigToUm, Counters.nSigFromUm);
	VPRINT("\n");
	VPRINT(" tVoice VoiceData: \n\tnVoiDataToVAC\t[%10d]  \n\tnVoiDataFromVAC\t[%10d]  \n\tnVoiDataToVDR\t[%10d]  \n\tnVoiDataFromVDR\t[%10d]",
		Counters.nVoiDataToVAC, Counters.nVoiDataFromVAC, Counters.nVoiDataToVDR, Counters.nVoiDataFromVDR);

	VPRINT("\n\tn10msPktToVAC\t[%10d]  \n\tn10msPktFromVAC\t[%10d]  \n\tn10msPktToVDR\t[%10d]  \n\tn10msPktFromVDR\t[%10d]",
		Counters.n10msPktToVAC, Counters.n10msPktFromVAC, Counters.n10msPktToVDR, Counters.n10msPktFromVDR);
	VPRINT("\n\nVDRCCBNULL\t[%10d]\n",Counters.nVDRCCBNULL);//wangwenhua add 20081222

	VPRINT("\n uplink null pkt Sent\t[%10d]", Counters.nNullPkt); 
	VPRINT("\n uplink G729B pkt Sent\t[%10d]", Counters.nG729BPkt); 
	VPRINT("\n UpLinkVdataLost\t[%10d]", Counters.nUpLinkVdataLost);
	VPRINT("\n DownLinkVdataBadSent\t[%10d]", Counters.nDownLinkVdataBadSent); 

	VPRINT("\n G729BPktFromVDR\t[%10d]", Counters.nG729BPktFromVDR);
	VPRINT("\n G729BPktToVAC(without jitterbuffer)\t[%10d]", Counters.nG729BPktToVAC);

	VPRINT("\n tVDR VoiceData: \n\tnVoiDataToMG\t[%10d]  \n\tnVoiDataFromMG\t[%10d]  \n\tnVoiDataToTvoice\t[%10d]  \n\tnVoiDataFromTvoice\t[%10d]", 
		Counters.nVoiDataToMG, Counters.nVoiDataFromMG, Counters.nVoiDataToTvoice, Counters.nVoiDataFromTvoice);
	VPRINT("\n========================================================================================================\n");
	
 
}

void clearAllCounters()
{
	memset(&Counters, 0, sizeof(Counters));
	CMsg_Signal_VCR::ClearSignalCounters();
}

void showSignalCounters()
{
	int i;
	VPRINT("\nSignal Counters:===========================================\n");
	for(i=0;i<InvalidSignal_MSG;i++)
	{
		VPRINT("  %40s :     RX[%10d]   TX[%10d] \n", CMsg_Signal_VCR::m_sigName[i],
					CMsg_Signal_VCR::RxSignalCounter[i],	
					CMsg_Signal_VCR::TxSignalCounter[i]);
	}
}

void showGrpCCBInfo(UINT16 gid)
{
	GrpCCB *pCCB = (GrpCCB*)CTVoice::GetInstance()->getVoiceFSM()->pGrpCCBTbl->FindCCBByGID(gid);
	if(NULL!=pCCB)
	{
		pCCB->showCCBInfo();
	}
}

void showGrpSrvInfo(bool blDetail)
{
	CTVoice::GetInstance()->getVoiceFSM()->pGrpCCBTbl->showGrpInfo(blDetail);
}

//void grpPaging(UINT16 gid, UINT32 grpL3Addr=0xffffffff, UINT16 grpSize=5, UINT8 prioty=PRIO_2);
//void grpPaging(UINT16 gid, UINT32 grpL3Addr, UINT16 grpSize, UINT8 prioty)
void grpPaging(UINT16 gid)	
{
	UINT32 grpL3Addr=0xffffffff;
	UINT16 grpSize=5;
	UINT8 prioty=PRIO_2;
	//ʵ��ʹ������Ϊ��srtp�ӿڵ�Լ����grpL3Addrֻ��16λ��
	//����������Ա���ʹ�ô���0xffff��grpL3Addr������Ӱ��ʵ�ʹ���
	if(grpL3Addr<=0xffff)
	{
		VPRINT("\ngrpPaging(UINT16 gid=0x%04X, UINT32 grpL3Addr=0x%08X, UINT16 grpSize=0x%04X, UINT8 prioty=0x%04X  grpL3Addr should >0xffff \n",
			gid, grpL3Addr, grpSize, prioty);
		return;
	}
	//GrpCCB* pGrpCCB = CTVoice::GetInstance()->m_fsm.pGrpCCBTbl->FindCCBByGID(gid);
	CComMessage* pComMsg = new (CTVoice::GetInstance(), 100) CComMessage;
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)pComMsg->GetDataPtr();

	pData->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_CALLCTRL;
	VSetU16BitVal(pData->sigHeader.Event_ID , M_MSG_EVENT_ID_LAGrpPaging);

	VSetU16BitVal(pData->sigPayload.GrpLAPaging.GID , gid);
	VSetU32BitVal(pData->sigPayload.GrpLAPaging.GrpL3Addr , grpL3Addr);
	pData->sigPayload.GrpLAPaging.EncryptFlag = 0;
	pData->sigPayload.GrpLAPaging.CommType = 1;
	pData->sigPayload.GrpLAPaging.CallPrioty = prioty;
	VSetU16BitVal(pData->sigPayload.GrpLAPaging.GrpSize , grpSize);

	pComMsg->SetSrcTid(M_TID_VCR);
	pComMsg->SetDstTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_VCR_VOICE_SIGNAL);
	pComMsg->SetDataLength(sizeof(SigHeaderT)+sizeof(GrpLAPagingT));
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
	} 
}

//void grpRelease(UINT16 gid, UINT32 grpL3Addr=0xffffffff);
//void grpRelease(UINT16 gid, UINT32 grpL3Addr)
void grpRelease(UINT16 gid)
{
	UINT32 grpL3Addr=0xffffffff;
	GrpCCB* pGrpCCB1 = CTVoice::GetInstance()->getVoiceFSM()->pGrpCCBTbl->FindCCBByGrpL3addr(grpL3Addr);
	GrpCCB* pGrpCCB2 = CTVoice::GetInstance()->getVoiceFSM()->pGrpCCBTbl->FindCCBByGID(gid);
	if(pGrpCCB1==pGrpCCB2 && pGrpCCB1!=NULL)
	{
		CComMessage* pComMsg = new (CTVoice::GetInstance(), 100) CComMessage;
		VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)pComMsg->GetDataPtr();

		pData->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_UTSAG;
		VSetU16BitVal(pData->sigHeader.Event_ID , M_MSG_EVENT_ID_GRP_UTSAG_L3ADDR);

		VSetU32BitVal(pData->sigPayload.UTSXC_Payload_GrpL3Addr.GrpL3Addr , grpL3Addr);
		pData->sigPayload.UTSXC_Payload_GrpL3Addr.msgType = M_MSGTYPE_GRP_RLS;
		pData->sigPayload.UTSXC_Payload_GrpL3Addr.UTSAGPayload[0] = REL_CAUSE_NORMAL;
		
		pComMsg->SetSrcTid(M_TID_VCR);
		pComMsg->SetDstTid(M_TID_VOICE);
		pComMsg->SetMessageId(MSGID_VCR_VOICE_SIGNAL);
		pComMsg->SetDataLength(sizeof(SigHeaderT)+6);
		if(!CComEntity::PostEntityMessage(pComMsg))
		{
			pComMsg->Destroy();
			VPRINT("\n send commessage fail!!!!!! \n");
		} 		
	}
	else
	{
		VPRINT("cannot find GrpCCB with grpL3Addr[0x%08X]!!!", grpL3Addr);
	}
}

void grpSndStatusRptMsg(UINT32 EID, UINT8 cid, UINT16 GID)
{
	CComMessage* pComMsg = new (CTVoice::GetInstance(), 100) CComMessage;
	L2L3_StatusReportIndiT* pData = (L2L3_StatusReportIndiT*)pComMsg->GetDataPtr();
	
	pData->cid = cid;
	VSetU16BitVal(pData->GID , GID);
	VSetU32BitVal(pData->EID , EID);
		
	pComMsg->SetSrcTid(M_TID_VAC);
	pComMsg->SetDstTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_GRP_L2L3_CPE_STATUS_REPORT_IND);
	pComMsg->SetDataLength(sizeof(L2L3_StatusReportIndiT));
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
	}	
}

void grpStatusRptMsgTest()
{
	grpSndStatusRptMsg(0x1, 0x0, 0x1);
}

void grpStatusRptTest()
	
{
	regCPEVoice(0x1, 0x1, 0x0);	//ģ��ע��һ��cpe
	grpPaging(0x1);
	
}

void grpSndCpeHoResReqMsg(UINT32 tgtBTSID, UINT32 EID, UINT8 cid, UINT16 GID, UINT32 UID)
{
	CComMessage* pComMsg = new (CTVoice::GetInstance(), 100) CComMessage;
	CPE_HoResReqT* pData = (CPE_HoResReqT*)pComMsg->GetDataPtr();
	
	pData->cid = cid;
	VSetU16BitVal(pData->GID , GID);
	VSetU32BitVal(pData->EID , EID);
	VSetU32BitVal(pData->UID , UID);
	pData->rsv1 = 0xAA;
	VSetU32BitVal(pData->btsID , tgtBTSID);
		
	pComMsg->SetSrcTid(M_TID_CPECM);
	pComMsg->SetDstTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_GRP_DAC_HO_RES_REQ);
	pComMsg->SetDataLength(sizeof(CPE_HoResReqT));
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
	}	
}

void grpHoTest(UINT32 tgtBTSID, UINT32 EID, UINT8 cid, UINT16 GID, UINT32 UID)
{
	grpSndCpeHoResReqMsg(tgtBTSID, EID, cid, GID, UID);

}

//20090531 fengbing bts inner switch for Voice Data begin
#ifdef M_VDATA_BTS_INNER_SWITCH
void sndDVoiceCfgReq(UINT32 uid1, UINT32 uid2, UINT8 DVoice)
{
	UINT32 L3Addr1,L3Addr2;
	VoiceCCB* pCCB1; 
	VoiceCCB* pCCB2; 
	if(uid1!=0xFFFFFFFF && uid2!=0xFFFFFFFF)
	{
		pCCB1 = findCCBByUid(uid1);
		if(pCCB1==NULL)
		{
			VPRINT("\ncannot find CCB with UID[0x%08X]!!!\n", uid1);
			return;
		}
		pCCB2 = findCCBByUid(uid2);
		if(pCCB2==NULL)
		{
			VPRINT("\ncannot find CCB with UID[0x%08X]!!!\n", uid2);
			return;	
		}
		if(NO_L3ADDR==pCCB1->getL3addr() || NO_L3ADDR==pCCB2->getL3addr())
		{
			VPRINT("\nBoth CPEs should be in a Conversation!!!\n");
			return;
		}
		L3Addr1 = pCCB1->getL3addr();
		L3Addr2 = pCCB2->getL3addr();
	}
	else
	{
		L3Addr1 = 0xAAAAAAAA;
		L3Addr2 = 0xBBBBBBBB;
	}

	CComMessage* pComMsg = new (CTVoice::GetInstance(), 100) CComMessage;
	VoiceVCRCtrlMsgT* pData = (VoiceVCRCtrlMsgT*)pComMsg->GetDataPtr();

	pData->sigHeader.EVENT_GROUP_ID = M_MSG_EVENT_GROUP_ID_CALLCTRL;
	VSetU16BitVal(pData->sigHeader.Event_ID , M_MSG_EVENT_ID_DVoiceConfigReq);

	pData->sigPayload.DVoiceCfgReq.DVoice = DVoice;
	VSetU32BitVal(pData->sigPayload.DVoiceCfgReq.L3Addr1 , L3Addr1);
	VSetU32BitVal(pData->sigPayload.DVoiceCfgReq.L3Addr2 , L3Addr2);
	
	pComMsg->SetSrcTid(M_TID_VCR);
	pComMsg->SetDstTid(M_TID_VOICE);
	pComMsg->SetMessageId(MSGID_VCR_VOICE_SIGNAL);
	pComMsg->SetDataLength(sizeof(SigHeaderT)+sizeof(DVoiceCfgReqT));
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		VPRINT("\n sndDVoiceCfgReq(),send commessage fail!!!!!! \n");
	}	
}
void enableInnerSwitch(UINT32 uid1, UINT32 uid2)
{
	sndDVoiceCfgReq(uid1, uid2, ENABLE_BTS_VDATA_INNER_SWITCH);
}

void disableInnerSwitch(UINT32 uid1, UINT32 uid2)
{
	sndDVoiceCfgReq(uid1, uid2, DISABLE_BTS_VDATA_INNER_SWITCH);
}

#endif

//20090531 fengbing bts inner switch for Voice Data end
}//extern "C"

////////////////////////////////////////////////////////////////////////////////
//spyVData---begin
////////////////////////////////////////////////////////////////////////////////

void VoiceDataSpy::clearUidResult()
{
	//UID
	uCntUID10msVDataFromSAG=0;
	uCntUID10msVDataToVAC=0;
	
	uCntUID10msVDataFromVAC=0;
	uCntUID10msVDataToSAG=0;		
	
	//729B, now only downlink
	uCntUID10ms729BFromSAG=0;
	uCntUID10ms729BToVAC=0;	
	uCntUID10ms729BFromVAC=0;
	uCntUID10ms729BToSAG=0;	
	//711A, fax data
	uCntUID10ms711AFromSAG=0;
	uCntUID10ms711AToL2=0;
	uCntUID10ms711AFromL2=0;
	uCntUID10ms711AToSAG=0;
	uCntUID10ms711AFromL2Lost=0;
}

void VoiceDataSpy::clearGidResult()
{
	//GID
	uCntGID10msVDataFromSAG=0;
	uCntGID10msVDataToVAC=0;
	//729B, now only downlink
	uCntGID10ms729BFromSAG=0;
	uCntGID10ms729BToVAC=0;


}

VoiceDataSpy::VoiceDataSpy()
{
	setDiagUID(0xffffffff);
	setDiagGID(0xffff);
	setShowPeriodicFlag(false);
	setFrameID(0);

	clearUidResult();
	clearGidResult();

	blLogULUidVDataTS = false;
	blLogDLUidVDataTS = false;
	blLogGidVDataTS = false;
}
void VoiceDataSpy::setDiagUID(UINT32 uid)
{
	if(diagUID!=uid)
	{
		diagUID=uid;
		clearUidResult();
	}
}
void VoiceDataSpy::setDiagGID(UINT16 gid)
{ 
	if(diagGID!=gid)
	{
		diagGID=gid; 
		clearGidResult();
	}
}

void VoiceDataSpy::showDiagResult()
{	
#if 0
	char txtOut[2000];
	UINT16 len = 0;
	len += sVPRINT(txtOut+len,"\nSpyVData===================================================");
	len += sVPRINT(txtOut+len,"\n FrameNum[%d][0x%08X] UID[0x%08X] GID[0x%04X] ", 
		getFrameID(), getFrameID(), getDiagUID(), getDiagGID());
//UID	
	len += sVPRINT(txtOut+len,"\n uCntUID10msVDataFromSAG[%d] uCntUID10msVDataToVAC[%d] ",
		uCntUID10msVDataFromSAG, uCntUID10msVDataToVAC);
	len += sVPRINT(txtOut+len,"\n uCntUID10msVDataFromVAC[%d] uCntUID10msVDataToSAG[%d] ",
		uCntUID10msVDataFromVAC, uCntUID10msVDataToSAG);
//GID	
	len += sVPRINT(txtOut+len,"\n uCntGID10msVDataFromSAG[%d] uCntGID10msVDataToVAC[%d] ",
		uCntGID10msVDataFromSAG, uCntGID10msVDataToVAC);
//729B
	len += sVPRINT(txtOut+len,"\n uCntUID10ms729BFromSAG[%d] uCntUID10ms729BToVAC[%d] ",
		uCntUID10ms729BFromSAG, uCntUID10ms729BToVAC);

	len += sVPRINT(txtOut+len,"\n uCntUID10ms729BFromVAC[%d] uCntUID10ms729BToSAG[%d] ",
		uCntUID10ms729BFromVAC, uCntUID10ms729BToSAG);

	len += sVPRINT(txtOut+len,"\n uCntGID10ms729BFromSAG[%d] uCntGID10ms729BToVAC[%d] ",
		uCntGID10ms729BFromSAG, uCntGID10ms729BToVAC);
	
	len += sVPRINT(txtOut+len,"\n===========================================================\n");	
	VPRINT("%s", txtOut);
#endif
	VPRINT("\nSpyVData===================================================");
	VPRINT("\n FrameNum[%d][0x%08X] UID[0x%08X] GID[0x%04X] ", 
		getFrameID(), getFrameID(), getDiagUID(), getDiagGID());
//UID	
	VPRINT("\n uCntUID10msVDataFromSAG[%d] uCntUID10msVDataToVAC[%d] ",
		uCntUID10msVDataFromSAG, uCntUID10msVDataToVAC);
	VPRINT("\n uCntUID10msVDataFromVAC[%d] uCntUID10msVDataToSAG[%d] ",
		uCntUID10msVDataFromVAC, uCntUID10msVDataToSAG);
//GID	
	VPRINT("\n uCntGID10msVDataFromSAG[%d] uCntGID10msVDataToVAC[%d] ",
		uCntGID10msVDataFromSAG, uCntGID10msVDataToVAC);
//729B
	VPRINT("\n uCntUID10ms729BFromSAG[%d] uCntUID10ms729BToVAC[%d] ",
		uCntUID10ms729BFromSAG, uCntUID10ms729BToVAC);

	VPRINT("\n uCntUID10ms729BFromVAC[%d] uCntUID10ms729BToSAG[%d] ",
		uCntUID10ms729BFromVAC, uCntUID10ms729BToSAG);

	VPRINT("\n uCntGID10ms729BFromSAG[%d] uCntGID10ms729BToVAC[%d] ",
		uCntGID10ms729BFromSAG, uCntGID10ms729BToVAC);
//711A, fax data
	VPRINT("\n uCntUID10ms711AFromSAG[%d] uCntUID10ms711AToL2[%d] ",
		uCntUID10ms711AFromSAG, uCntUID10ms711AToL2);
	VPRINT("\n uCntUID10ms711AFromL2[%d] uCntUID10ms711AToSAG[%d] uCntUID10ms711AFromL2Lost[%d]",
		uCntUID10ms711AFromL2, uCntUID10ms711AToSAG, uCntUID10ms711AFromL2Lost);
	
	VPRINT("\n===========================================================\n");	
}

VoiceDataSpy g_SpyVData;

void spyVDataSetUid(UINT32 uid)
{
	g_SpyVData.setDiagUID(uid);
	if(uid!=0 && uid!=0xffffffff)
	{
		VPRINT("use spyVDataSetUid(0) or spyVDataSetUid(0xffffffff) to clear the UID when you need not monitor voice data");
	}
}
void spyVDataSetGid(UINT16 gid)
{
	g_SpyVData.setDiagGID(gid);
	if(gid!=0 && gid!=0xffff)
	{
		VPRINT("use spyVDataSetGid(0) or spyVDataSetGid(0xffff) to clear the GID when you need not monitor voice data");
	}	
}
void spyVDataShow()
{
	g_SpyVData.showDiagResult();
}
void spyVDataStart()
{
	if(!g_SpyVData.isSpyUidVoiceData() && !g_SpyVData.isSpyGidVoiceData())
	{
		VPRINT("\n Set UID or GID to monitor first...\n");
		VPRINT("\n use spyVDataSetUid(UID) or spyVDataSetGid(GID) command. \n");
	}
	else
	{
		g_SpyVData.setShowPeriodicFlag(true);
		VPRINT("\nuse spyVDataSop command to stop...\n");
	}
}
void spyVDataStop()
{
	g_SpyVData.setShowPeriodicFlag(false);
}

void logULUidVDataTS(bool flag)
{
	g_SpyVData.logULUidVDataTS(flag);
}
void logDLUidVDataTS(bool flag)
{
	g_SpyVData.logDLUidVDataTS(flag);
}
void logGidVDataTS(bool flag)
{
	g_SpyVData.logGidVDataTS(flag);
}
void logULUidFaxData(bool flag)
{
	g_SpyVData.logULUidFaxData(flag);
}
void logDLUidFaxData(bool flag)
{
	g_SpyVData.logDLUidFaxData(flag);
}

////////////////////////////////////////////////////////////////////////////////
//spyVData---end
////////////////////////////////////////////////////////////////////////////////


//for test end

//new stat
/*get perf data*/
void GetVoicePerfDataNew(UINT32 *pdata)
{
    CTVoice::GetInstance()->getVoiceFSM()->GetVoicePerfDataNew(pdata);
}
/*clear perf data*/
void ClearVoicePerfDataNew()
{
    CTVoice::GetInstance()->getVoiceFSM()->ClearVoicePerfDataNew();
}


