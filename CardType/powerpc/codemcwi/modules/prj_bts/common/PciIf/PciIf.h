/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   define the class for PciIf entity
 *                deliver packets between L2 and L3 through PCI interface
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   12/11/2005   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef __INC_PCIIF
#define __INC_PCIIF

#include <netBufLib.h>
#include <msgQLib.h>
#include <lstLib.h>
#include <eventLib.h>

#include "Task.h"
#include "ComMessage.h"
#include "taskDef.h"
#include "mv64360.h"
#include "sysDma.h"
#include "logArea.h"

#ifdef M_TGT_L2
#define PROXY_TID_NUM 14
#else
#define PROXY_TID_NUM 8
#endif

#define BUF_POOL_NUM  5
#define MAX_TX_BUF_SIZE   10240   
const UINT16 PciTxBufSize[BUF_POOL_NUM]= { 256,   512, 1024, 2048, MAX_TX_BUF_SIZE};

const UINT16 PciL2ToL3BufNum[BUF_POOL_NUM] = {1000,1000,500, 1000, 200 };  // must smaller than 16K
const UINT16 PciL3ToL2BufNum[BUF_POOL_NUM] = {500,500,1000, 100, 200 };  // must smaller than 16K

#define DMA_CHANNEL_DESCRIPTOR_COUNT  2000

#define L2_L3_PKT_LEADING_SPACE  M_DEFAULT_RESERVED  // reserve 64 bytes before the payload

#define SEND_MESSAGE_BATCH_SIZE 100
#define FREE_MESSAGE_BATCH_SIZE 100

#define L2_PCI_IF_READY_NOTIFY         0x55555555
#define L2_PCI_IF_READY_FOR_BOOT       0xAAAAAAAA   //#define TRAN_READY   0xAAAAAAAA  in L2 Preloader
#define L2_PCI_IF_L2_REQUEST_REBOOT_IF   0x99999999  /* should be the same as defined in sysL2BspCsi.h  */
#define L2_PCI_IF_L2_REQUEST_REBOOT_CSI  0x9999999A
#define L2_PCI_IF_L2_REQUEST_REBOOT_CMD  0x9999999B

#define L3_PCI_IF_L2_TIMING_TOKEN     0x12345678

//#define CHECK_BUFFER_FREE_FLAG
#ifdef  CHECK_BUFFER_FREE_FLAG
#define PCI_BUF_FREE_FLAG 0x55555555
#define PCI_BUF_BUSY_FLAG 0xaaaaaaaa
#endif

#define MV64360_REG_PEER_INB_POST_QUEUE   ((volatile UINT32*)(PCI_PEER_INB_POST_QUEUE))
#define MV64360_REG_PEER_OUTB_FREE_QUEUE  ((volatile UINT32 *)(PCI_PEER_OUTB_FREE_QUEUE))

typedef enum _cirQueSize{I20_16K = 0x1,I20_32K = 0x2,I20_64K = 0x4,
                         I20_128K = 0x8,I20_256K = 0x10
                        } I2O_CIRCULAR_QUEUE_SIZE;

const  I2O_CIRCULAR_QUEUE_SIZE   CircularQSize=  I20_256K ; 

#define DOORBELL_INBOUND_TOKEN_BIT        BIT1

#define OUT_BOUND_Q_LOW_SIZE 60000
#define OUT_BOUND_Q_HIGH_SIZE 100
#define EVENT_Q_SIZE     16

typedef enum
{
    TYPE_COMMSG = 0,
    TYPE_IPPACKET = 1
}PCI_IF_PKT_TYPE;

typedef struct 
{
    PCI_IF_PKT_TYPE type;
    union
    {
        M_BLK_ID mBlk;
        CComMessage* comMsg;
    };
}PCI_IF_Q_MSG;   // data type in outbound  message queue

typedef struct 
{
    unsigned int MUST_BE_ZERO :1;     // because when got from outbound free queue, 0xffffffff means empty
    unsigned int MessageFormat:1;   // 0 -- commsg, 1 -- IP 
    unsigned int BufferIndex  :14;
    unsigned int PoolIndex    :3;
    unsigned int MessageLength:13;
}T_PCI_MSG;   // message posted to peer

typedef union
{
    UINT32 Val;
    T_PCI_MSG  Msg;
}PCI_MSG;

typedef struct 
{
    NODE lstHdr;
    PCI_IF_Q_MSG  SentMsg;
    PCI_MSG       PciMsg;
}TO_FREE_MSG_LIST_ENTRY;   // DMA finished messages, need to be freed

typedef enum   
{ FREE=0, 
  OCCUPIED
} PEER_TX_BUF_STATUS;

typedef enum
{
    PCI_RX_BUF_EMPTY = 0x55,
    PCI_RX_BUF_OCCUPIED
}RX_BUF_STATUS;

typedef struct
{
    NODE           lstHdr;
    UINT16         PoolIndex;
    UINT16         BufferIndex;
    UINT32         BufferDmaAddress;
    UINT32         BufferCpuAddress;
    PEER_TX_BUF_STATUS  BufferStatus;
}PEER_TX_BUF_NODE;   // list node of free Tx buffer in peer memory

#ifdef M_TGT_L3
typedef enum
{
    L2_SYSTEM_STATE_REBOOT = 0x55,
    L2_SYSTEM_STATE_RUNNING = 0xaa
}L2_SYSTEM_STATE;
#endif

#ifdef M_TGT_L2
typedef struct
{
    volatile bool    HasPostToken;
    volatile UINT32  TokenLostTick;
    volatile UINT8   UnsentHeadIndex;
    volatile UINT8   UnsentTailIndex;
    volatile UINT32  EventBuf[256];
}L2_TIMING_CCB;
#endif

typedef struct
{
    UINT16 DestTid;
    UINT16 SrcTid;
    UINT32 EID;
    UINT16 MsgId;
    UINT16 MsgLen;
    UINT16 Reserved;
    UINT16 UID;
}L2L3_MSG_HEADER;

#define IP_NET_BUF_NUM  200
#define IP_NET_BUF_SIZE   2048   //1600, 1600 is not enough for big packets
typedef struct 
{
    NET_POOL_ID pNetPool;
    M_CL_CONFIG mclBlkConfig;
    CL_DESC     clDescTbl;
} PCIIF_MEMPOOL;

typedef struct
{
    UINT32            BufferCpuAddress;
    RX_BUF_STATUS     BufferStatus;
    CComMessage       *pComMsg;
    UINT32            PostRound;
}T_RxBufStatus;

typedef struct
{
    PCI_MSG  pciMsg;
    UINT32   postRound;
}T_LocalQueueBufferEntry;

typedef struct 
{
    UINT16 QueueHead;   // save the latest message
    UINT16 QueueTail;   // save the oldest message, need to be processed first
    T_LocalQueueBufferEntry  QueueEntry[0x10000];  // should be more than total Tx buffer count
}T_LocalQueueBuffer;


typedef enum
{
    PCI_EVENT_PCI_IN_MSG   = 0x12345678,
    PCI_EVENT_PCI_IN_Q,
    PCI_EVENT_DMA_CHAIN_END ,    
    PCI_EVENT_TX_POST_MSG_TOKEN, 
    PCI_EVENT_RX_IN_NOTIFY,
    PCI_EVENT_PCI_OUT_F_Q,
#ifdef M_TGT_L3
    PCI_EVENT_L1_TDD_MSG
#endif
}T_PciIfEvent;

class CTaskPciIf: public CTask
{
public:
    static CTaskPciIf* GetInstance();
    bool PostMessage(CComMessage*, SINT32, bool isUrgent = false);
    TID GetEntityId() const { return CurrentTid;} ;
    void ShowStatus(int);
    bool DeallocateComMessage(CComMessage* pComMsg);
    STATUS PciEndSend(M_BLK_ID pMblk);
    UINT32  GetRxBufAddr(UINT16 poolIndex, UINT16 bufIndex);
    UINT32  GetTxBufAddr(UINT16 poolIndex, UINT16 bufIndex);
    void   SearchForBusyTxBuf();
    #ifndef M_TGT_L3
    void SendL2TimingEventToL3(UINT32 msg);
    static CTaskPciIf* GetPciIfObj();
    #endif


friend class CComMessage;

private:
    
    CTaskPciIf();
    bool Initialize();
    void MainLoop();

    #define PCIIF_MAX_BLOCKED_TIME_IN_10ms_TICK 1000 //100 liuweidong fox l2if deadlock
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return PCIIF_MAX_BLOCKED_TIME_IN_10ms_TICK ;};
    
    static void PciInBoundISR(CTaskPciIf *obj);

    static void LastDmaDescCallBack(CTaskPciIf *obj, int arg);
    void PostEvent(T_PciIfEvent event);    
    void PostL1TddEvent(UINT32 event);
    
    void TxDmaFinishProc();
    void InBoundQueueMsgProc();
    void OutBoundFreeQueueMsgProc();
    void RxNotifyProc();
    void TxFreeNotifyProc();

    #ifndef M_TGT_L3
    void L2TimingTokenReclaim();
    #endif

    void StartPendingDma();

    #ifdef M_TGT_L3
    void InBoundMsgProc();
    #endif
    void OutBoundMsgProc();  // send packet to peer
    void ReclaimTxPostMsgToken();

    void PostToFreeMsg(TO_FREE_MSG_LIST_ENTRY *msg);

    void CheckTxTokenSanity();
    void CheckInboundIntMaskSanity();

    PEER_TX_BUF_NODE *FindFreeTxBuffer(UINT16 bufLen, bool isOamMsg);


    void NotifyPeerOutboundFreeMsg();
    void SendOutboundFreeMsg(PCI_MSG);
    void NotifyPeerInboundMsg();

    void PrepareDmaForOneMsg(PCI_IF_Q_MSG &);
    

    static CTaskPciIf *Instance;

    TID CurrentTid;
    static const TID ProxyTIDs[PROXY_TID_NUM]; 

    static const UINT32 EV_MSG_Q_LOW       = VXEV01;    
    static const UINT32 EV_EVENT_Q         = VXEV02;
    static const UINT32 EV_MSG_Q_HIGH      = VXEV03;
    static const UINT32 EVT_L1TDD_MSG_Q    = VXEV04; //liruichao
                    

    MSG_Q_ID     MsgQPciIfLow;
    MSG_Q_ID     MsgQPciIfHigh;
    MSG_Q_ID     MsgQEvent;    
    MSG_Q_ID     MsgQL1TddEvent;//liruichao
    UINT32       HighQueueMsgPostCount;
    UINT32       HighQueueMsgProcessCount; 


    LIST         ToFreeMsgListArray[2];
    LIST*        OnFlightToFreeMsgList;
    LIST*        PendingToFreeMsgList;
    LIST         ToFreeMsgNodePoolList;

    PEER_TX_BUF_NODE   *PeerTxBufNode[BUF_POOL_NUM];
    LIST               FreePeerTxBufList[BUF_POOL_NUM];

    T_RxBufStatus      *RxBufStatusTable[BUF_POOL_NUM];
            
    DmaChannel   *TxDmaChannel;
    SYS_DMA_DESC *OnFlightDmaDesc;
    SYS_DMA_DESC *PendingDmaDescHead;
    SYS_DMA_DESC *PendingDmaDescTail;

    PCI_MSG *OnflightTxMsgPostPendingBuf;
    UINT32  OnflightTxMsgPostPendingCount;
    PCI_MSG *PendingTxMsgPostPendingBuf;
    UINT32  PendingTxMsgPostPendingCount;

    PCI_MSG *RxMsgFreePendingBuf;
    UINT32  RxMsgFreePendingCount;

    T_LocalQueueBuffer InboundMsgCirQue;
    UINT32  LastInboundQueueIsrTick;

    bool    HasTxPostMsgToken;
    UINT32  TxPostMsgTokenLostTick;

    UINT32   CircularQMem; // must be 1MB aligned
    UINT32   InBoundQMemBase;
    UINT32   InBoundQMemTop;
    UINT32   OutBoundFreeQMemBase;
    UINT32   OutBoundFreeQMemTop;
    UINT32   TotalTxBufferCount;

    UINT8   PoolIndexTable[MAX_TX_BUF_SIZE];

    PCIIF_MEMPOOL NetBufPool;

    UINT32   ContinuousRunOutOfTxBufCount;
    UINT32   MaxContRunOutOfTxBufCount;
    UINT32   FirstRunOutOfTxBufTick;

    #ifdef M_TGT_L3
    L2_SYSTEM_STATE L2RunningState;
    UINT32          L2BootUpCount;
    UINT32          ToL2ComMsgDiscardedCount;
    UINT32          ToL2IpPacketDiscardedCount;
    UINT32          L2RequestForResetCount[3];
    #else
    L2_TIMING_CCB   L2TimingEventCCB;
    #endif
    

    // performance counters
    UINT32 IpPacketSendCount[BUF_POOL_NUM];
    UINT32 IpPacketReceiveCount[BUF_POOL_NUM];
    UINT32 ComMsgSendCount[BUF_POOL_NUM];
    UINT32 ComMsgReceiveCount[BUF_POOL_NUM];
    UINT32 TxBufRunOutCount[BUF_POOL_NUM];
    UINT32 TxBufAllocateCount[BUF_POOL_NUM];
    UINT32 TxBufFreeCount[BUF_POOL_NUM];
    UINT32 CurrentTxBufCount[BUF_POOL_NUM];
    UINT32 MaxUsedTxBufCount[BUF_POOL_NUM];
    UINT32 AppFreeCount[BUF_POOL_NUM];
    UINT32 NetBufPoolExhaustedCount[BUF_POOL_NUM];
    UINT32 MaxPendingDescCount;
    UINT32 CurrentPendingDescCount;
    UINT32 TooLongComMsgCount;
    UINT32 FreeNodeRunOutCount;
    UINT32 TxFreeMsgCount;
    UINT32 DmaDescriptorRunOutCount;
    UINT32 TotalSentComMsgCount;
    UINT32 TotalTxComMsgCount;
    UINT32 InboundMsgPostFailureCount;
    UINT32 OutboundMsgPostFailCount_LOW;
    UINT32 OutboundMsgPostFailCount_HIGH;
    UINT32 TotalTxIpPktCount;
    UINT32 TotalSentIpPktCount;
    UINT32 TxIpPktFormatErrorCount;
    UINT32 TooLongIPPacketCount;
	UINT32 MaxInboundMsgInOneRound;
	UINT32 MaxOutboundFreeMsgInOneRound;
    UINT32 MainLoopErrorCount;
    UINT32 PeerFreeInvalidBufCount;
    UINT32 PeerPostInvalidBufCount;
    UINT32 RequestL2RebootCount;
    UINT32 MaxOutboundFreeSendCount;
    UINT32 MaxInboundSendCount;
    UINT32 ForceTxPostMsgTokenCount;
    UINT32 NotifyPeerInboundCount;
    UINT32 TxTokenReturnCount;
    UINT32 NotifyPeerOutboundFreeCount;
    UINT32 TotalPostMsgCount;

};


void PciEndMuxSend(M_BLK_ID mblk);


#define _1K             0x00000400
#define _2K             0x00000800
#define _4K             0x00001000
#define _8K             0x00002000
#define _16K            0x00004000
#define _32K            0x00008000
#define _64K            0x00010000
#define _128K           0x00020000
#define _256K           0x00040000
#define _512K           0x00080000
#define _1M             0x00100000
#define _2M             0x00200000
#define _3M             0x00300000
#define _4M             0x00400000
#define _5M             0x00500000
#define _6M             0x00600000
#define _7M             0x00700000
#define _8M             0x00800000
#define _9M             0x00900000
#define _10M            0x00a00000
#define _11M            0x00b00000
#define _12M            0x00c00000
#define _13M            0x00d00000
#define _14M            0x00e00000
#define _15M            0x00f00000
#define _16M            0x01000000
#define _32M            0x02000000

#define LOG_ERR_PCI_DEBUG_INFO          LOGNO(PCIIF, 0)
#define LOG_ERR_PCI_L2_REBOOT           LOGNO(PCIIF, 1)
#define LOG_ERR_PCI_INIT_ERROR          LOGNO(PCIIF, 2)
#define LOG_ERR_PCI_QUEUE_OVERFLOW      LOGNO(PCIIF, 3)
#define LOG_ERR_PCI_NOT_ENOUGH_LEADING  LOGNO(PCIIF, 4)
#define LOG_ERR_PCI_FREE_BUFFER_INVALID LOGNO(PCIIF, 5)
#define LOG_ERR_PCI_FREE_BUF_FREE       LOGNO(PCIIF, 6)
#define LOG_ERR_PCI_POST_BUSY_RX_BUF    LOGNO(PCIIF, 7)
#define LOG_ERR_PCI_FREE_EMPTY_RX_BUF   LOGNO(PCIIF, 8)
#define LOG_ERR_PCI_APP_FREE_INVALID_BUF   LOGNO(PCIIF, 9)

#if 0

#include "BizTask.h"
class PciTestStub:public CBizTask
{
public:
    virtual TID GetEntityId() const ; 
    virtual bool IsNeedTransaction() const { return false;};
    ~PciTestStub(){};
    static PciTestStub *GetInstance();
    bool ProcessComMessage(CComMessage* pComMsg);
    bool Initialize();
    #ifdef M_TGT_L3
    void SendPciTestMsg();
    #endif
    bool IsMonitoredForDeadlock()  { return false; };
    int  GetMaxBlockedTime() { return 0xffffffff ;};

private:
    PciTestStub();
    static PciTestStub *instance;
    CComMessage *testMsg;
};
#endif //__INC_PCIIF

#endif
