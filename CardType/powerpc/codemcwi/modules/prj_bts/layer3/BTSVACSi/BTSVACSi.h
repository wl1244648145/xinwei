/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   BTSVAC function
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   02/08/2006   Fengbing      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef __BTS_VAC_SIM_H__
#define __BTS_VAC_SIM_H__

#include "ComEntity.h"
//#include "BizTask.h"
#include "ComMessage.h"
#include "taskDef.h"

#include <list>
#include <map>
using namespace std;




typedef struct tagCPEVACVoiData_729
{
	UINT8 Cid;
	UINT8 Length;
	UINT8 VoiceData[10];
}CPEVACVoiData_729T;

typedef struct tagCPEVACVoiData_711
{
	UINT8 Cid;
	UINT8 Length;
	UINT8 VoiceData[80];
}CPEVACVoiData_711T;

//任务参数定义
#define M_TASK_NAME_LEN				(10)
#define M_TASK_VACSIM_TASKNAME      "tVACSim"
#define M_TASK_VACSIM_PRIORITY      (95)
#ifdef __WIN32_SIM__
#define M_TASK_VACSIM_OPTION        (0x0008)
#define M_TASK_VACSIM_MSGOPTION     (0x02)
#elif __NUCLEUS__
#define M_TASK_VACSIM_OPTION       (NULL)	//not use
#define M_TASK_VACSIM_MSGOPTION     (NU_FIFO)	//NUFIFO or NU_PRIORITY
#else
#define M_TASK_VACSIM_OPTION        (VX_FP_TASK)
#define M_TASK_VACSIM_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_VACSIM_STACKSIZE     (20480)
#define M_TASK_VACSIM_MAXMSG        (1024)



typedef struct tagVACVoiceDataT
{
	UINT32	EID;	
	UINT8	CID;	
	UINT8	SN;
	UINT8	Length;
	UINT8	voiceData[10];
}VACVoiceDataT;

typedef struct tagDownlinkVoiceData
{
	UINT8	SN;
	UINT8	voiceData[10];
}DownlinkVoiceDataT;

#define JITTER_BUF_SIZE		6


struct VoiceTupleStruct;
typedef struct VoiceTupleStruct VoiceTuple;
struct VoiceTupleStruct
{
	bool operator > (const VoiceTuple& b) const
	{
		return (Eid > b.Eid || (Eid==b.Eid && Cid > b.Cid) );
	}
	
	bool operator == (const VoiceTuple& b) const
	{
		return (Eid== b.Eid && Cid==b.Cid);
	}
	
	bool operator < (const VoiceTuple& b) const
	{
		return (Eid<b.Eid || (Eid==b.Eid && Cid<b.Cid));
	}
	UINT32 Eid;
	UINT16 Cid;
};


typedef struct tagDownlinkVACSession
{
	UINT32	EID;
	UINT8	CID;
	DownlinkVoiceDataT	DownlinkVoiceDataBuf[JITTER_BUF_SIZE];	//下行语音数据包缓存根据每个(EID,CID)做缓存60ms的语音数据包
	UINT8	curTx;
	UINT8	curFree;
}DownlinkVACSessionT;






class CBTSVACSim: public CComEntity
{
public:
    static CBTSVACSim* GetInstance();
    //bool PostMessage(CComMessage*, SINT32, bool isUrgent=false);
    TID GetEntityId() const; 

	//bool Initialize();
	//bool ProcessComMessage(CComMessage *msg);
	bool PostMessage(CComMessage*, SINT32, bool isUrgent=false);

private:
    CBTSVACSim();
    static CBTSVACSim *Instance;
/*
	//上行语音数据包缓存,上行语音数据包每10ms发送给tVoice任务
	VACVoiceDataT	UplinkVoiceDataBuf[500];
	UINT16	UplinkVoiceDataCount;
	//下行语音数据包缓存,下行语音数据包每10ms每个语音端口只发送一个语音数据包给CPE
	DownlinkVACSessionT	DownlinkVACSession[100];
	map<VoiceTuple, UINT32> BTreeByTuple;
	list <UINT32> lstFreeDownklinkVACSession;
	UINT8	m_SN;
*/
/*
	bool AllocDownVACSession(VoiceTuple tuple);
	bool DeAllocDownVACSession(VoiceTuple tuple);

	//处理从CPE收到的上行语音数据包
	void handleUpLinkVoiData(CComMessage* msg);
	//处理从tVoice收到的下行语音数据包
	void handleDownLinkVoiData(CComMessage* msg);
	//下行语音数据包10ms定时发送处理
	void CBTSVACSim::DownLinkVoiceDataTX();
	//上行语音数据包10ms定时发送处理
	void CBTSVACSim::UplinkVoiceDataTX();
*/

};



#endif //__BTS_VAC_SIM_H__

