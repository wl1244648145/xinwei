/*****************************************************************************
(C) Copyright 2005: Arrowping Networks, Inc.
Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
* FILENAME:    L3DataSocket.h
*
* DESCRIPTION: 
*		BTS上的Socket任务类
* HISTORY:
*
*   Date       Author         Description
*   ---------  ---------     ----------------------------------------------------
*   10/17/06   yanghuawei    initialization. 
*   11/01/06   wangxin       modification.
*
*---------------------------------------------------------------------------*/
#ifndef	_INC_TSOCKET
#define	_INC_TSOCKET

#include "BizTask.h"
#include "ComMessage.h"
#include "taskdef.h"

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

#ifndef _INC_LOGAREA
#include "LogArea.h"
#endif

#include <stdio.h>

#ifdef __WIN32_SIM__
#ifndef _VECTOR_
#include <vector>
#endif
#else/*__WIN32_SIM__*/
#include "taskLib.h"
#include "msgQLib.h"
#include "selectLib.h"
#include "sockLib.h"
#include "ioLib.h"
#include "pipeDrv.h"
#include "inetLib.h" 
#include "errnoLib.h"
#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#define SOCKET int
typedef struct fd_set FD_SET;
#endif/*__WIN32_SIM__*/

#include <list>
#include <map>
using namespace std;

#ifndef _INC_TSOCKET_MSGS
#include "L3DataSocketTable.h"
#endif

#ifndef _INC_TSOCKET_MSGS
#include "L3DataSocketMsgs.h"
#endif

#ifndef _INC_TSOCKET_MSGIDS
#include "L3DataSocketMsgIds.h"
#endif

#ifndef _INC_TSOCKET_ERR_CODE
#include "L3DataSocketErrCode.h"
#endif

#ifndef __L3_DATAMSGID_H__
#include "L3DataMsgId.h"
#endif
#include <map>

#ifdef __WIN32_SIM__
#define INET_ADDR_LEN  (17) 
#endif

//任务参数定义
#define M_TSOCKET_NAME_LEN			  (20)
#define M_TSOCKET_TASKNAME            "tSOCKET"
#define M_TSOCKET_PRIORITY            (95)
#ifdef __WIN32_SIM__
#define M_TSOCKET_OPTION              (0x0008)
#define M_TSOCKET_MSGOPTION           (0x02)
#elif __NUCLEUS__
#define M_TSOCKET_OPTION              (NULL)
#define M_TSOCKET_MSGOPTION           (NU_FIFO)
#else
#define M_TSOCKET_OPTION              (VX_FP_TASK)
#define M_TSOCKET_MSGOPTION           ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#ifndef WBBU_CODE
#define M_TSOCKET_STACKSIZE           (20480)
#else
#define M_TSOCKET_STACKSIZE           (40960)
#endif
#define M_TSOCKET_MAXMSG              (2048)

#define M_TSOCKET_PIPENAME		     "/dev/SOCKETPipe"
#define M_MSG_AREA_LENGTH             (2)

#define M_TSOCKET_BUFFER_LENGTH       (2400)// lijinan (2000)
#define M_TSOCKET_BUFFER_NUM          (5000)
#define M_TSOCKET_MAX_MSGLEN          (M_TSOCKET_BUFFER_LENGTH- M_DEFAULT_RESERVED + M_MSG_AREA_LENGTH)

#define M_TSOCKET_DEFAULT_PORT        (10000)    
#define M_TSOCKET_PIPE_PORT           (10001)   

#define M_TSOCKET_FTENTRYNUM_MAX      (200)   
#define M_TSOCKET_FT_INDEX_ERR        (0xffff)
#define M_TSOCKET_FTENTRY_EXPIRE      (20)

#define M_TSOCKET_BTSADDRNOTIFY_CB_NUM (500)      
#define M_TSOCKET_CB_INDEX_ERR        (0xffff)
#define M_TSOCKET_MAX_RETRY_COUNT     (3)   

#define M_TSOCKET_FCB_INDEX_ERR       (0xffff)

#define DEFAULT_BUFFER_RSV            (64)
#define DEFAULT_TRANS_ID              (0xffff)

//Timer
#define M_TSOCKET_FT_EXPIRE_TIMEOUT   (3600000)
#define M_TSOCKET_CB_EXPIRE_TIMEOUT   (2000)

#define MSG_BUFFER_FROM_POOL          (0x55555555)
#define MSG_BUFFER_FROM_HEAP          (0xaaaaaaaa)


#define M_TSOCKET_NVRAM_INITIALIZED (0x2007528)


//For Jamming forwarding
#define M_TSOCKET_JAMMINGNEIGHBOR_NUM (16)  
#define M_TSOCKET_JNT_INDEX_ERR       (0xffff)

//lijinan 090105 for jamming rpt flow control
typedef struct _BtsJammingRptFlowCtrl
{
	CTimer *pFromL2Timer;
	CTimer *pFromBtsTimer;
	UINT32 L2TimerIsStart;
	UINT32 BtsTimerIsStart;
	UINT32 jammingFromL2Cnt;
	UINT32 jammingFromBtsCnt;
} BtsJammingRptFlowCtrl;
#define FLAG_YES                     1
#define FLAG_NO                       0

/****************************************************
*TSOCKET_PERF_TYPE: TSOCKET任务性能统计类型
****************************************************/
typedef enum 
{
	PERF_TSOCKET_UDP_TOTAL_TX = 0,
	PERF_TSOCKET_SOCK_ERR,
    PERF_TSOCKET_INVALID_RX,

    PERF_TSOCKET_EMSA_PACKETS_RX,
    PERF_TSOCKET_EMSA_PACKETS_TX,
	PERF_TSOCKET_EMSA_ERR,
    PERF_TSOCKET_EMSA_TX_FAIL,

	PERF_TSOCKET_TCR_ERR,
    PERF_TSOCKET_TCR_PACKETS_RX,
    PERF_TSOCKET_TCR_PACKETS_TX,


	PERF_TSOCKET_TDR_ERR,
	PERF_TSOCKET_TDR_PACKETS,

	PERF_TSOCKET_LOADING_ERR,
	PERF_TSOCKET_LOADING_PACKETS_RX,
    PERF_TSOCKET_LOADING_PACKETS_TX,

	PERF_TSOCKET_DIAG_ERR,
	PERF_TSOCKET_DIAG_PACKETS,

	PERF_TSOCKET_JAMMING_BTS_ERR,
	PERF_TSOCKET_JAMMING_BTS_PACKETS,

	PERF_TSOCKET_JAMMING_L2_ERR,
	PERF_TSOCKET_JAMMING_L2_PACKETS,

	PERF_TSOCKET_NO_BUFFER,
	PERF_TSOCKET_FT_NO_BUFFER,
	PERF_TSOCKET_CB_NO_BUFFER,
    PERF_TSOCKET_JNT_NO_BUFFER,

	PERF_TSOCKET_GRPSRV_HO_ERR,
	PERF_TSOCKET_GRPSRV_HANDOVER,

	PERF_TSOCKET_MAX
}TSOCKET_PERF_TYPE;

#define M_TYPE_STRINGLEN    (50)
const UINT8 strPerfType[ PERF_TSOCKET_MAX ][ M_TYPE_STRINGLEN ] = 
{
	"TOtal Tx ",
    "Socket Error",
    "Invalid Rx",

    "EmsAgent Rx",
    "EmsAgnet Tx",
	"EmsAgent Rx Err",
    "EmsAgent Tx Fail",

	"TCR Err",
    "TCR Packets Rx",
    "TCR Packets Tx",

	"TDR Err",
	"TDR Packets Rx",

	"Loading Err",
	"Loading Packets Rx",
    "Loading Packets Tx",

	"Diag Err",
	"Diag Packets Rx",

	"Jamming From Bts Err",
	"Jamming From Bts Packets",

	"Jamming From L2 Err",
	"Jamming From L2 Packets",

	"No Buffer Err",
	"No FT Entries Err",
	"No CB Entries Err",
	"No JNT Entries Err",

	"GrpSrv HORes Err",
	"GrpSrv HORes Packets"
};


#define TSOCKET_MAX_BLOCKED_TIME_IN_10ms_TICK (2000) //20seconds
class CSOCKET:public CTask
{
friend void showMap(int arg);
public:
	CSOCKET();
	~CSOCKET();	
	static CSOCKET* GetInstance();
	void ShowStatus();
	bool GetBtsPubAddr(UINT32, BtsAddr*);
    bool GetBtsPubAddrByData(UINT32 btsid, BtsAddr* addr);
    bool PostBtsIpReqMsg(UINT32 btsid);
	void GetPerfData(UINT8 *,UINT8);
	void ClearMeasure()
    {
        memset( m_perf, 0, sizeof( m_perf ) );
    }
	void clearFT();
	#ifndef WBBU_CODE
	void ProcessRecvMsg(CComMessage* ,int ,sockaddr_in );
	CComMessage *GetFreeComMsg();
	#endif
	inline UINT32 IncreasePerf( TSOCKET_PERF_TYPE type ){return m_perf[ type ] += 1;};
       void testsocket(UINT32 ip,UINT32 len);
private:
	bool Initialize();
	TID GetEntityId() const { return M_TID_EMSAGENTTX;};
	virtual void MainLoop();
	bool DeallocateComMessage(CComMessage*);
	bool PostMessage(CComMessage* pMsg,	SINT32 timeout, bool isUrgent=false);

    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return TSOCKET_MAX_BLOCKED_TIME_IN_10ms_TICK ;};
#ifdef WBBU_CODE
	//inline UINT32 IncreasePerf( TSOCKET_PERF_TYPE type ){return m_perf[ type ] += 1;};
#endif
	bool CreateSocket(void);
	bool CloseSocket(void);
	bool ProcessEmsMsg(CComMessage*, UINT16);
	bool ProcessTunnelManagementMsg(CComMessage*, UINT16);
	bool ProcessTunnelDataMsg(CComMessage*, UINT16);
	bool ProcessLoadingInfoMsg(CComMessage*, UINT16);
	bool ProcessDiagMsg(CComMessage*, UINT16);
	#ifdef WBBU_CODE
	void ProcessRecvMsg(/*CComMessage* ,int*/ );
	#endif
	void ProcessSendMsg(CComMessage*);
	CComMessage* GetOneMsgFromPipe(void);
	void PostMsgToEms(CComMessage*);
	bool PostMsgToBts(CComMessage*, UINT16);
        //zengjihan 20120801 for GPSSYNC
        #ifdef WBBU_CODE
        bool PostMsgToGps(CComMessage* , UINT16 );
        bool ProcessGpsMsg(CComMessage* , UINT16 );
        #endif
	void PostBtsIpNotification(UINT32& btsid);
	void ProcessBtsIpRequest(CComMessage*);
	void PostBtsIpResponse();

	bool CreatePipe();
	bool DeletePipe();
	bool OpenPipe();
	bool ClosePipe();
	void OutputSocketErrCode(char*p);
	int SearchMap(TID tid, UINT16 msgid) const;
	int SearchMap(UINT16 ma, UINT16 moc, UINT16 action) const;
	void InitMapEntry();

    void InitFreeComMsgList();
  #ifdef WBBU_CODE
    CComMessage *GetFreeComMsg();
  #endif
    void ReclaimFreeComMsg(CComMessage *);

	//转发表操作
	bool   FTBPtreeAdd(UINT32&/*BtsId*/, UINT16);
	bool   FTBPtreeDel(UINT32&/*BtsId*/);
	UINT16 FTBPtreeFind(UINT32&/*BtsId*/);
	UINT16 GetFreeFTEntryIdxFromList();
	bool   FTAddEntry(UINT32& btsid, const BtsAddr&);
	bool   FTDelEntry(UINT32& btsid);
	BtsAddr* GetFTEntryByIdx(UINT16 index);
	void   FTBPtreeExpire();

	/***************************
	*tSocket BtsIp Notification
	*Control Block
	**************************/
	bool   InitFreeCBList();
	bool   InsertFreeCB(UINT16);
	UINT16 GetFreeCBIndexFromList();
	BtsAddrNotifyCB* GetCBbyIdx(UINT16 index);
	bool   CBBPtreeAdd(UINT32&/*BtsId*/, UINT16);
	bool   CBBPtreeDel(UINT32&/*BtsId*/);
	UINT16 CBBPtreeFind(UINT32&/*BtsId*/);
	CTimer* StartBtsIpNotifyTimer( UINT32 &btsid, UINT32 millsecs );
	void   BtsIpNotifyTimeout(const CComMessage*);
	void   ProcessUpdateBtsPubIp(CComMessage* );

	//通过udp socket发送数据
	bool SendBySocket(UINT32 ip, UINT16 port, void *pData, UINT32 len);

	CTimer* InitTimer(bool/*(in)periodic*/,UINT32/*(in)timeout*/,UINT16/*(in)msgId*/); 

    void RecoverFTfromFCB(); 

	bool ProcessGrpSrvHOMsgFromOtherBTS(CComMessage* pComMsg,UINT16 nChar);
	void ProcessGrpSrvHOMsg2OtherBTS(CComMessage* pComMsg);
	void ProcessSTime2HLR(CComMessage* pComMsg);
	void ProcessMsg2CB3000(CComMessage* pComMsg);
	//for Jamming forwarding
	void ProcessJNTRefreshNotify(CComMessage*);
    void ProcessJammingFromL2(CComMessage*);
	bool ProcessJammingFromBts(CComMessage*,UINT16);
	void ForwardingJammingRpt(UINT16,SocketMsgArea*,UINT32);
	bool GetJammingAddr(UINT16, JammingNeighborEntry*);
	bool   JNTBPtreeAdd(UINT16&/*seqid*/, UINT16);
	UINT16 JNTBPtreeFind(UINT16&/*seqid*/);
	bool   InitFreeJNTList();
	UINT16 GetFreeJNTEntryIdxFromList();
	bool   JNTAddEntry(UINT16& seqid, const JammingNeighborEntry&);
	JammingNeighborEntry* GetJNTEntryByIdx(UINT16 index);
	void InitialJNT();
	//void PrintMsgData(CComMessage*);
   
	void PostJammingBtsInfoIeToL2();	
	static CSOCKET* s_ptaskTSOCKET;
#ifdef __WIN32_SIM__
	WSADATA m_wsaData;
#endif
	SOCKET m_fdtSocket;//send and receive message
	SOCKET m_fdPipePut;//message pipe in
	SOCKET m_fdPipe;//message pipe out
	sockaddr_in m_client;
	SINT32 m_lMaxMsgs;
	SINT32 m_lMsgQOption;
	UINT32 m_nChar;
	//UINT8* m_pRecvBuf;
	UINT8  m_aucBufferPool[ M_TSOCKET_BUFFER_NUM ][ M_TSOCKET_BUFFER_LENGTH ];

    CComMessage *m_plistFreeComMessage;
    UINT32 FreeComMsgBufferCount;

    UINT32 m_perf[PERF_TSOCKET_MAX];
	//////////////////////////////////////////////////////////////////////////
	//转发表
	BtsAddr m_FT[ M_TSOCKET_FTENTRYNUM_MAX ];// 转发表
	map<UINT32/*BtsId*/, UINT16> m_FTBptree;//转发表索引树
	list<UINT16> m_listFreeFT;//空闲转发表表项的链表
	CTimer* m_ptmFTExpire;

	/***************************
	*tSocket BtsIp Notification
	*Control Block
	**************************/
	BtsAddrNotifyCB m_CB[M_TSOCKET_BTSADDRNOTIFY_CB_NUM];
	list<UINT16> m_listFreeCB;      //空闲控制块的链表
	map<UINT32/*btsid*/, UINT16> m_CBptree;  //控制块索引树

	//////////////////////////////////////////////////////////////////////////
	//Jamming 转发表使用
	UINT32 m_curBtsId;
	UINT16 m_curBtsFreq;
	UINT16 m_curBtsSeq;
	map<UINT16, UINT32> m_mapRx, m_mapTx;
    //lijinan 090105 for jamming rpt flow ctl
    void initJmFlowCtl();
    BtsJammingRptFlowCtrl stJammingRptCtl;
	
};
#endif /* _INC_TSOCKET */
