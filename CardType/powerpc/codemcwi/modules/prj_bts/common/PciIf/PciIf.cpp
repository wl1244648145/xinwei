/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:  implementation of PCI Interface class
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   11/17/2005   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#include <intLib.h>
#include <msgQEvLib.h>
#include <logLib.h>
#include <taskLib.h>
#include <cacheLib.h>
#include <wvLib.h>
#include <string.h>
#include <stdio.h>
#include <end.h>
#include <endLib.h>
#include <netBufLib.h>
#include <netLib.h>
#include <tickLib.h>

#include "mv64360.h"
#include "PciIf.h"
#include "log.h"
#include "sysMv64360IntrCtl.h"
#include "mcWill_bts.h"

#ifdef M_TGT_L3
#include "sysBtsConfigData.h"
#include "L2L3MsgDef.h"
#else
#include "sysL2BspCsi.h"
#endif
#include "L3L2MessageId.h"

UINT32 pciTimeOut = 2;

#ifdef M_TGT_L3
extern void sendL1TddMsgToL2Tx(UINT16 msgId, UINT8 ts);
#endif
UINT32 L2TxTimingCountIsr[9];

/*----------------------------------------------------------------------------
 *   Forward Declaration of global funcitons and type
 *---------------------------------------------------------------------------*/
void  mvI2oCircularQueueEnable(I2O_CIRCULAR_QUEUE_SIZE cirQueSize, UINT32 queueBaseAddr);
STATUS PciIfSendIpFunc(M_BLK_ID pMblk);   

typedef STATUS (*PCISENDFUNC) (M_BLK_ID);		/* pfunction returning int */


/*----------------------------------------------------------------------------
 *  external funciton declaration
 *---------------------------------------------------------------------------*/
extern "C" void netPoolShow(NET_POOL_ID);
extern "C"  STATUS StartPciEnd(NET_POOL_ID);
extern "C" STATUS RegisterPciEndSend(PCISENDFUNC funcPtr);


/*----------------------------------------------------------------------------
 *  Global data definition
 *---------------------------------------------------------------------------*/
extern END_OBJ*     PciEndDrv;
CTaskPciIf * CTaskPciIf::Instance=NULL;
CTaskPciIf *PciIfObj = NULL;
#ifdef M_TGT_WANIF
extern UINT32  WorkingWcpeEid;
extern UINT16 Wanif_Switch;
#endif

#ifdef M_TGT_L3
const TID CTaskPciIf::ProxyTIDs[PROXY_TID_NUM] = { 
                           M_TID_L2MAIN,      // 101  
                           M_TID_L2BOOT,      // 102
                           M_TID_DIAG,    // 103
                           M_TID_VAC,	      // 106 CPE voice task
                           M_TID_DAC,    
                           M_TID_L2OAM,
                           M_TID_PCISIO,
                           M_TID_L1TDDIF,
                           //M_TID_RSV2
                        };
extern "C" void rebootL2();
#else
const TID CTaskPciIf::ProxyTIDs[PROXY_TID_NUM] = { 
                           M_TID_CM,
                           M_TID_FM,
                           M_TID_PM,
                           M_TID_UM,
                           M_TID_DIAGM,
                           M_TID_BM,
                           M_TID_EB,
                           M_TID_SYS,
                           M_TID_UTAGENT,
                           M_TID_VOICE,
                           M_TID_GM,
                           M_TID_L2SHELL,
                           M_TID_L2_TXINL3,
                           M_TID_EMSAGENTTX
                           //M_TID_RSV3
                         };
extern "C" void RequestL3ToResetL2(int);
UINT32 ForceL2TimingTokenCount = 0;
#endif

void SendDoorbellToPeer(UINT32 setBit)
{
    // trigger a doorbell interrupt to the peer
    *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_DBL_CPU0) = LONGSWAP (setBit); 
}


/*****************************************************************************
 *
 *   Method:     CTaskPciIf::GetInstance()
 *
 *   Description: The only interface to create a singleton object
 *
 *   Parameters:  None
 *
 *   Returns:  object of the class
 *
 *****************************************************************************
 */
CTaskPciIf* CTaskPciIf::GetInstance()
{
    if (NULL == Instance)
    {
        Instance = new CTaskPciIf;
    }
    return Instance;
}




/*****************************************************************************
 *
 *   Method:     CTaskPciIf::CTaskPciIf()
 *
 *   Description: Constructor, 
 *
 *   Parameters:  None
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
CTaskPciIf::CTaskPciIf()
{
    #ifdef M_TGT_L2
    strcpy(m_szName, "tL3If");
    m_uPriority   = M_TP_L3IF;
    #else
    strcpy(m_szName, "tL2If");
    m_uPriority   = M_TP_L2IF;
    #endif
    
    m_uOptions    = 0;
    m_uStackSize  = 1024 * 10;

    #ifdef M_TGT_L2
    CurrentTid = M_TID_L3IF;
    #else
    CurrentTid = M_TID_L2IF;
    #endif

    MsgQPciIfLow = NULL;
    MsgQPciIfHigh = NULL;
    MsgQEvent = NULL;
    MsgQL1TddEvent = NULL;

    OnFlightToFreeMsgList = &ToFreeMsgListArray[0];
    PendingToFreeMsgList = &ToFreeMsgListArray[1];
    lstInit(OnFlightToFreeMsgList);
    lstInit(PendingToFreeMsgList);

    lstInit(&ToFreeMsgNodePoolList);

    memset(PeerTxBufNode, 0, sizeof(PeerTxBufNode));

    memset(RxBufStatusTable, 0, sizeof(RxBufStatusTable));

    TxDmaChannel = NULL;
    OnFlightDmaDesc = NULL;
    PendingDmaDescHead = NULL;
    PendingDmaDescTail = NULL;

    CircularQMem = 0;
    InBoundQMemBase = 0;     
    OutBoundFreeQMemBase = 0;
    InBoundQMemTop = 0;    
    OutBoundFreeQMemTop = 0;
    
    CircularQMem = PCI_CIRCULAR_BUF_ADDR;
    InBoundQMemBase =  CircularQMem + CircularQSize*_16K;
    OutBoundFreeQMemBase = CircularQMem + 3*_16K*CircularQSize;
    InBoundQMemTop = InBoundQMemBase + CircularQSize*_16K;
    OutBoundFreeQMemTop = OutBoundFreeQMemBase + CircularQSize*_16K;

    NetBufPool.pNetPool = NULL;

    ContinuousRunOutOfTxBufCount = 0;
    MaxContRunOutOfTxBufCount = 0;


    #ifdef M_TGT_L3
    L2RunningState = L2_SYSTEM_STATE_REBOOT;
    L2BootUpCount = 0;
    ToL2ComMsgDiscardedCount = 0;
    ToL2IpPacketDiscardedCount = 0;
    L2RequestForResetCount[0] = 0;
    L2RequestForResetCount[1] = 0;
    L2RequestForResetCount[2] = 0;
    #else
    L2TimingEventCCB.HasPostToken = true;
    L2TimingEventCCB.UnsentHeadIndex = 0;
    L2TimingEventCCB.UnsentTailIndex = 0;
    memset((void*)&L2TimingEventCCB.EventBuf[0], 0xFFFFFFFF, sizeof(L2TimingEventCCB.EventBuf));
    #endif


    MaxPendingDescCount = 0; 
    CurrentPendingDescCount = 0;
    TooLongComMsgCount = 0;
    FreeNodeRunOutCount = 0;
    TxFreeMsgCount = 0;
    DmaDescriptorRunOutCount = 0;
    TotalSentComMsgCount = 0;
    TotalTxComMsgCount  = 0;
    InboundMsgPostFailureCount = 0;
    TotalTxIpPktCount = 0;
    TotalSentIpPktCount = 0;
    TxIpPktFormatErrorCount = 0;
    TooLongIPPacketCount = 0;

	MaxInboundMsgInOneRound = 0;
	MaxOutboundFreeMsgInOneRound = 0;

    OutboundMsgPostFailCount_LOW = 0;

    for (int i=0; i<BUF_POOL_NUM; i++)
    {
        lstInit( &FreePeerTxBufList[i]);
        TxBufRunOutCount[i] = 0;
        TxBufAllocateCount[i] = 0;
        TxBufFreeCount[i] = 0;
        CurrentTxBufCount[i] = 0;
        MaxUsedTxBufCount[i] = 0;
        IpPacketSendCount[i] = 0;
        IpPacketReceiveCount[i] = 0;
        ComMsgSendCount[i] = 0;
        ComMsgReceiveCount[i] = 0;
        AppFreeCount[i] = 0;
        NetBufPoolExhaustedCount[i] = 0;
    }
    MainLoopErrorCount = 0;
    PeerFreeInvalidBufCount = 0;
    PeerPostInvalidBufCount = 0;
    RequestL2RebootCount = 0;

    MaxOutboundFreeSendCount = 0;
    MaxInboundSendCount = 0;
    ForceTxPostMsgTokenCount = 0;

    NotifyPeerInboundCount = 0;
    TxTokenReturnCount = 0;
    NotifyPeerOutboundFreeCount = 0;
    TotalPostMsgCount=0; 

    HighQueueMsgPostCount = 0;
    HighQueueMsgProcessCount = 0; 

}

/*****************************************************************************
 *
 *   Method:     CTaskPciIf::Initialize()
 *
 *   Description: function to initialize the data structure used by the entity.
 *                Called by framework before mainloop
 *
 *   Parameters:  None
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
bool CTaskPciIf::Initialize()
{

    MV64360_REG_WR(MV64360_SRAM_CFG, 0x00160080);  // disable parity checking on SRAM
       // disable sram parity checking due to some debug experience
       
    // set up pool index map table, to expedite the pool index search from packet length
    UINT32 size = 0;
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
    {
        for (; size<PciTxBufSize[poolIndex]; size++)
        {
            PoolIndexTable[size] = poolIndex;
        }
    }
    // create message queues to accept com message and IP packets to peer 
    MsgQPciIfLow = msgQCreate(OUT_BOUND_Q_LOW_SIZE, sizeof(PCI_IF_Q_MSG),MSG_Q_FIFO);
    MsgQPciIfHigh = msgQCreate(OUT_BOUND_Q_HIGH_SIZE, sizeof(PCI_IF_Q_MSG),MSG_Q_FIFO);
    
    MsgQEvent = msgQCreate(EVENT_Q_SIZE, sizeof(T_PciIfEvent),MSG_Q_FIFO);
    MsgQL1TddEvent = msgQCreate(EVENT_Q_SIZE, sizeof(UINT32),MSG_Q_FIFO);

    // register to ComEntity for proxy TIDs
    for (int i=0; i<SIZEOF(ProxyTIDs); i++)
    {
        CurrentTid = ProxyTIDs[i];
        RegisterEntity(false);
        LOG1(LOG_DEBUG, LOG_ERR_PCI_DEBUG_INFO, "Register proxy for task %d finished\n", CurrentTid);
    }

    UINT32 bufCount=0;
    UINT32 bufSize = 0;
    for (int i=0; i<BUF_POOL_NUM; i++)
    {
        #ifdef M_TGT_L3
        bufCount += PciL3ToL2BufNum[i];
        bufSize  += PciL3ToL2BufNum[i] * PciTxBufSize[i];
        #else
        bufCount += PciL2ToL3BufNum[i];
        bufSize  += PciL2ToL3BufNum[i] * PciTxBufSize[i];
        #endif
    }

    TotalTxBufferCount = bufCount;
    
    #ifdef M_TGT_L3
    if (bufSize > PCIIF_L3TOL2_BUF_SIZE)
    #else
    if (bufSize > PCIIF_L2TOL3_BUF_SIZE)
    #endif    
    {
        LOG(LOG_CRITICAL, LOG_ERR_PCI_DEBUG_INFO, "Reserved Pci Buffer space is not enough\n");
        return false;
    }

    ////////////////
    // build up TX buffer pools
    PEER_TX_BUF_NODE *nodePtr;

    #ifdef M_TGT_L3
    UINT32 bufDmaAddr = PCIIF_L3TOL2_BUF_L3_DMA_BASE_ADDR; 
    UINT32 bufCpuAddr = PCIIF_L3TOL2_BUF_L3_CPU_BASE_ADDR;
    UINT32 rxBufCpuAddr = PCIIF_L2TOL3_BUF_L3_CPU_BASE_ADDR;
    UINT16 *PciTxBufNum =  (UINT16 *)PciL3ToL2BufNum;
    UINT16 *PciRxBufNum =  (UINT16 *)PciL2ToL3BufNum;
    #else
    UINT32 bufDmaAddr = PCIIF_L2TOL3_BUF_L2_DMA_BASE_ADDR; 
    UINT32 bufCpuAddr = PCIIF_L2TOL3_BUF_L2_CPU_BASE_ADDR;
    UINT32 rxBufCpuAddr = PCIIF_L3TOL2_BUF_L2_CPU_BASE_ADDR;
    UINT16 *PciTxBufNum =  (UINT16 *)PciL2ToL3BufNum;
    UINT16 *PciRxBufNum =  (UINT16 *)PciL3ToL2BufNum;
    #endif
    
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
    {
        PeerTxBufNode[poolIndex] = (PEER_TX_BUF_NODE*)malloc( PciTxBufNum[poolIndex]
                                                           * sizeof(PEER_TX_BUF_NODE));

        nodePtr = &PeerTxBufNode[poolIndex][0];
        if (NULL == PeerTxBufNode[poolIndex])
        {
            LOG(LOG_CRITICAL, LOG_ERR_PCI_INIT_ERROR, "ERROR Creating buffers for PCI interface");
        }

        for (int bufIndex=0; bufIndex<PciTxBufNum[poolIndex]; bufIndex++)
        {
            nodePtr->PoolIndex = poolIndex;
            nodePtr->BufferIndex = bufIndex;
            nodePtr->BufferDmaAddress = bufDmaAddr;
            nodePtr->BufferCpuAddress = bufCpuAddr;
            nodePtr->BufferStatus = FREE;


            bufDmaAddr += PciTxBufSize[poolIndex];
            bufCpuAddr += PciTxBufSize[poolIndex];
            lstAdd(&FreePeerTxBufList[poolIndex], &nodePtr->lstHdr);
            nodePtr++;
        }

        // allocate memory for Rx buffer status table
        RxBufStatusTable[poolIndex] = (T_RxBufStatus*)malloc(PciRxBufNum[poolIndex] 
                                                             * sizeof(T_RxBufStatus));

        for (int bufIndex = 0; bufIndex<PciRxBufNum[poolIndex]; bufIndex++)
        {
            RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress = rxBufCpuAddr;
            RxBufStatusTable[poolIndex][bufIndex].BufferStatus = PCI_RX_BUF_EMPTY;
            RxBufStatusTable[poolIndex][bufIndex].pComMsg = NULL;
            RxBufStatusTable[poolIndex][bufIndex].PostRound = 0;

            #ifdef CHECK_BUFFER_FREE_FLAG
            *(UINT32*)rxBufCpuAddr = PCI_BUF_FREE_FLAG;
            #endif
            rxBufCpuAddr += PciTxBufSize[poolIndex];

        }
    }

    //////////////////////////////////////
    // build up to-be-free node pool
    TO_FREE_MSG_LIST_ENTRY* freeNode = (TO_FREE_MSG_LIST_ENTRY*)malloc(bufCount * sizeof(TO_FREE_MSG_LIST_ENTRY));
    if ( NULL == freeNode )
    { 
        LOG(LOG_CRITICAL, LOG_ERR_PCI_INIT_ERROR, "fail in create buffer on PCI interface\n");
    }
    memset(freeNode, 0, bufCount*sizeof(TO_FREE_MSG_LIST_ENTRY));
    for (int i=0; i<bufCount; i++)
    {
        lstAdd(&ToFreeMsgNodePoolList, &freeNode->lstHdr);
        freeNode++;
    }

    //////////////////
    // allocate DMA channel
    TxDmaChannel = sysDmaChannelAlloc(DMA_WINDOW_PCI, DMA_CHANNEL_DESCRIPTOR_COUNT, INT_MODE_LAST_DESCRIPTOR);
    if ( NULL == TxDmaChannel )
    {
        LOG(LOG_CRITICAL, LOG_ERR_PCI_INIT_ERROR, "fail in allocate DMA channel in PCI interface");
    }

    // initialze PCI interrupt registers and enable interrupt
    mvI2oCircularQueueEnable(CircularQSize, CircularQMem);
    OnflightTxMsgPostPendingBuf = (PCI_MSG*)malloc(TotalTxBufferCount * sizeof(PCI_MSG));
    PendingTxMsgPostPendingBuf = (PCI_MSG*)malloc(TotalTxBufferCount * sizeof(PCI_MSG));
    OnflightTxMsgPostPendingCount = 0;
    PendingTxMsgPostPendingCount = 0;

    RxMsgFreePendingBuf = (PCI_MSG*)malloc(TotalTxBufferCount * sizeof(PCI_MSG));
    RxMsgFreePendingCount = 0;

    InboundMsgCirQue.QueueHead = 0;
    InboundMsgCirQue.QueueTail = 0;

    HasTxPostMsgToken = true;   

    // enable inbound message interrupt
    UINT32 regVal;
    #ifdef M_TGT_L3
    regVal = ~(BIT0 | BIT1 | BIT4 | BIT5 |BIT16);    
                  // enable inbound message, inbound circular queue,inbound doorbell low bits 
                  // and outbound free queue overflow
                  // BIT16 is for inbound msg 1
    #else
    regVal = ~(BIT0 | BIT1 | BIT4 | BIT5);   
    //regVal = ~(BIT1 | BIT4 | BIT5);   
                   // enable inbound circular queue and inbound doorbell low bits 
                    // and outbound free queue overflow, inbound message for timing event token returning
    #endif
    
    // clear all the previous pending interrupts
    MV64360_REG_WR(MV64360_I2O_IB_INT_CAUSE_CPU0, 0xffffffff);
    MV64360_REG_WR(MV64360_I2O_IB_INT_MASK_CPU0,regVal);

    // allocate memory for netbuf pool
    NetBufPool.pNetPool = (NET_POOL_ID)malloc (sizeof(NET_POOL));

    NetBufPool.mclBlkConfig.clBlkNum = IP_NET_BUF_NUM;
    NetBufPool.mclBlkConfig.mBlkNum  = IP_NET_BUF_NUM; // mblk number and cluster block number are the same
    /* Calculate the total memory for all the M-Blks and CL-Blks. */
    NetBufPool.mclBlkConfig.memSize = (NetBufPool.mclBlkConfig.mBlkNum  * (MSIZE + sizeof (long))) +
                                   (NetBufPool.mclBlkConfig.clBlkNum * (CL_BLK_SZ + sizeof(long)));

    // allocate memory for pool
    NetBufPool.mclBlkConfig.memArea = (char*)memalign(sizeof(long), NetBufPool.mclBlkConfig.memSize);

    NetBufPool.clDescTbl.clSize = IP_NET_BUF_SIZE;
    NetBufPool.clDescTbl.clNum = IP_NET_BUF_NUM;
    NetBufPool.clDescTbl.memSize = IP_NET_BUF_NUM * (IP_NET_BUF_SIZE + 8) + sizeof(int);
    NetBufPool.clDescTbl.memArea = (char*)memalign(sizeof(long), NetBufPool.clDescTbl.memSize);
    

    /* Initialise the memory pool. */
    STATUS rc = netPoolInit(NetBufPool.pNetPool, &NetBufPool.mclBlkConfig, &NetBufPool.clDescTbl, 1, NULL);
    if ( OK != rc)
    {
        printf(" PCI interface netbuf Pool initialization failure\n");
        return false;
    }
    //enable MUX device for IP connectivity
    StartPciEnd(NetBufPool.pNetPool);
    RegisterPciEndSend(PciIfSendIpFunc);


    //debug: use mv64360 timer 0 to probe pci inbound message latency
    unsigned int tmrimask;
    unsigned int tmrctrl;
#if 0    
    MV64360_REG_RD(MV64360_TIMER_CTR_INT_MASK, &tmrimask);
    tmrimask &= 0xFFFFFFFE;
    MV64360_REG_WR(MV64360_TIMER_CTR_INT_MASK, tmrimask);

    MV64360_REG_WR(MV64360_TIMER_CTR0, 0xFFFFFFFF);

    MV64360_REG_RD(MV64360_TIMER_CTR_CTRL, &tmrctrl);
    tmrctrl &= 0xFFFFFFF0;
    tmrctrl |= 0x00000003;
    MV64360_REG_WR(MV64360_TIMER_CTR_CTRL, tmrctrl);
 #endif   
    // enable the CPU interrupt
    ::intConnect(INUM_TO_IVEC(INT_VEC_PCI0_IN), (VOIDFUNCPTR)PciInBoundISR, (int)this);
    ::intEnable(INT_VEC_PCI0_IN);

    // set this variable to allow IP packets transfer
    PciIfObj = this;

    return true;
}

/*****************************************************************************
 *
 *   Method:     CTaskPciIf::PostMessage()
 *
 *   Description: Interface function provided to other ComEntity objects to 
 *                pass ComMessage to peer CPU. 
 *
 *   Parameters:  None
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
bool CTaskPciIf::PostMessage(CComMessage* msg, SINT32 timeOut, bool isUrgent)
{
    #ifdef M_TGT_L3
    if ( L2_SYSTEM_STATE_REBOOT == L2RunningState )
    {
        ToL2ComMsgDiscardedCount ++;
        return false;
    }
    #endif
    

    PCI_IF_Q_MSG postMsg;
    postMsg.type = TYPE_COMMSG;
    postMsg.comMsg = msg;

    if (!msg->AddRef())
    {
        return false;
    }
    STATUS rc;
    if (isUrgent)
    {
        rc = msgQSend(MsgQPciIfHigh, (char*)(&postMsg), sizeof(PCI_IF_Q_MSG),NO_WAIT, MSG_PRI_NORMAL);
        if ( OK != rc)
        {
            msg->Release();


            LOG1(LOG_CRITICAL, LOG_ERR_PCI_QUEUE_OVERFLOW, "Post ComMessage to PCI interface message q 0x%x high failed",
                               (UINT32)MsgQPciIfHigh);
            OutboundMsgPostFailCount_HIGH ++;
            return false;
        }
        else
        {
            HighQueueMsgPostCount ++;
        }
    }
    else
    {
        rc = msgQSend(MsgQPciIfLow, (char*)(&postMsg), sizeof(PCI_IF_Q_MSG),NO_WAIT, MSG_PRI_NORMAL);
        if ( OK != rc)
        {
            msg->Release();
            LOG(LOG_CRITICAL, LOG_ERR_PCI_QUEUE_OVERFLOW, "Post ComMessage to PCI interface message q failed");
            OutboundMsgPostFailCount_LOW++;
            return false;
        }
    }
    return true;
}



void CTaskPciIf::CheckTxTokenSanity()
{
    if (!HasTxPostMsgToken)
    {
        if (tickGet() - TxPostMsgTokenLostTick > pciTimeOut)
        {
            LOG(LOG_MAJOR, LOG_ERR_PCI_POST_BUSY_RX_BUF,"Tx Post msg Token Reclaimed by timeout");
            ReclaimTxPostMsgToken();
            ForceTxPostMsgTokenCount++;
        }
    }

    #ifndef M_TGT_L3
    if (false == L2TimingEventCCB.HasPostToken)
    {
        if ( tickGet() - L2TimingEventCCB.TokenLostTick > pciTimeOut)
        {
            LOG(LOG_MAJOR, LOG_ERR_PCI_POST_BUSY_RX_BUF,"L2 Timing Post Token Reclaimed by timeout");
            L2TimingTokenReclaim();
            ForceL2TimingTokenCount++;
        }
    }
    #endif
    
}

//#define DEBUG_INBOUND_ERROR
#ifdef DEBUG_INBOUND_ERROR
UINT32 IntMaskHistory[0x100][5];
UINT8  IntMaskHistoryIndex=0;
bool intMaskFail = false;
#endif
void CTaskPciIf::CheckInboundIntMaskSanity()
{

    taskLock();
    int oldLvl = intLock();
    if ( tickGet() - LastInboundQueueIsrTick >= 50)  // no msg for peer for the last half second
    {  // try to recover from an error condition periodically
        UINT32 inBoundPostQueHead, inBoundPostQueTail;
        //read the head and tail pointer of the inbound circular queue
        MV64360_REG_RD(MV64360_I2O_IB_POST_TAIL_PTR_CPU0,&inBoundPostQueTail);
        MV64360_REG_RD(MV64360_I2O_IB_POST_HEAD_PTR_CPU0,&inBoundPostQueHead);
        if (inBoundPostQueHead != inBoundPostQueTail)
        {
            UINT32 intMask;
            MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &intMask);
            if ( BIT4 == (intMask & BIT4) ) // inbound message queue is disabled, not a valid state
            {
                intMask &= ~BIT4;  // reenable the inbound message queue interrupt
                MV64360_REG_WR(MV64360_I2O_IB_INT_MASK_CPU0, intMask);
                MainLoopErrorCount ++;
                LOG(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF,"Int Mask invalid status in mainloop");

                #ifdef DEBUG_INBOUND_ERROR
                if ( !intMaskFail)
                {
                    IntMaskHistory[IntMaskHistoryIndex][0] = 3;
                    IntMaskHistory[IntMaskHistoryIndex][2] = tickGet();
                    MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &IntMaskHistory[IntMaskHistoryIndex][1]);
                    IntMaskHistory[IntMaskHistoryIndex][3] = inBoundPostQueHead;
                    IntMaskHistory[IntMaskHistoryIndex][4] = inBoundPostQueTail;
                    IntMaskHistoryIndex++;
                    intMaskFail = true;
                }
                #endif
            }
        }
    }
    intUnlock(oldLvl);
    taskUnlock();
}

#ifdef DEBUG_INBOUND_ERROR
extern "C" void printIntFail()
{
    if (intMaskFail)
    {   
        UINT8 bufIndex = IntMaskHistoryIndex; 
        for (int i=0; i<0x100; i++)
        {
            printf("(%d, 0x%08x, tick %d, head=0x%x, tail=0x%x)  ", IntMaskHistory[bufIndex][0],IntMaskHistory[bufIndex][1],IntMaskHistory[bufIndex][2],
                   IntMaskHistory[bufIndex][3],IntMaskHistory[bufIndex][4]);
            bufIndex++;
            if (bufIndex % 2 == 0)
            {
                printf("\n");
            }
        }
        printf("=========================");
        IntMaskHistoryIndex = 0;
        memset(IntMaskHistory, 0, sizeof(IntMaskHistory));
        intMaskFail = false;
    }
}
#endif




/*****************************************************************************
 *
 *   Method:     CTaskPciIf::MainLoop()
 *
 *   Description: Mainloop of the task
 *
 *   Parameters:  None
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::MainLoop()
{
    UINT32 rxEvents;

    #ifndef M_TGT_L3
     //notify L3 that L2 is ready to accept messages  by sending a inbound message
    *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_MSG0_CPU0) = 
                        LONGSWAP (L2_PCI_IF_READY_NOTIFY);   /**clear interruptr**/			
    #endif

    const UINT32 eventMaskBothQ =   EV_MSG_Q_LOW | EV_MSG_Q_HIGH | EV_EVENT_Q | EVT_L1TDD_MSG_Q;
    const UINT32 eventMaskEventQ = EV_EVENT_Q | EVT_L1TDD_MSG_Q;
    T_PciIfEvent internalEvent;
    UINT32 l1TddMsg;
    bool  isRxNotify, isPeerReturnToken;
    UINT32 eventMask;

    for(;;)
    {
        STATUS rc = msgQEvStart(MsgQEvent, EV_EVENT_Q, EVENTS_SEND_IF_FREE );
        rc = msgQEvStart(MsgQL1TddEvent, EVT_L1TDD_MSG_Q, EVENTS_SEND_IF_FREE );

        if (  (CurrentPendingDescCount < SEND_MESSAGE_BATCH_SIZE) 
        #ifdef M_TGT_L3
              || ( L2_SYSTEM_STATE_REBOOT == L2RunningState )
                // discard all the messages in the queue if L2 is rebooting
        #endif
           )
        {
            STATUS rc = msgQEvStart(MsgQPciIfLow, EV_MSG_Q_LOW, EVENTS_SEND_IF_FREE );
            rc = msgQEvStart(MsgQPciIfHigh, EV_MSG_Q_HIGH, EVENTS_SEND_IF_FREE );
            eventMask = eventMaskBothQ;
        }
        else
        {
            eventMask =eventMaskEventQ;    
        }

        isRxNotify = false;
        isPeerReturnToken = false;
        rc = eventReceive(eventMask, EVENTS_WAIT_ANY|EVENTS_KEEP_UNWANTED, 5, &rxEvents);

        OutBoundFreeQueueMsgProc();

        if ( OK == rc )
        {   
#ifdef M_TGT_L3
            //liruichao
            if(EVT_L1TDD_MSG_Q & rxEvents)
            {                
                if ( sizeof(int) == msgQReceive(MsgQL1TddEvent, (char *)&l1TddMsg, sizeof(int), NO_WAIT))
                {
                    UINT16 msgId= l1TddMsg >> 16;
                    UINT8 ts = l1TddMsg & 0xff;
                    
                    if (ts<9)
                    {
                        L2TxTimingCountIsr[ts]++;
                        
                        sendL1TddMsgToL2Tx(msgId, ts);
                    }
                }
            }
#endif            
            if ( EV_EVENT_Q & rxEvents)
            {
                if ( sizeof(T_PciIfEvent) == msgQReceive(MsgQEvent, (char *)&internalEvent, sizeof(T_PciIfEvent), NO_WAIT))
                {  // process all the events in this iteration
                    switch (internalEvent)
                    {
                        #ifdef M_TGT_L3
                        case PCI_EVENT_PCI_IN_MSG:
                            InBoundMsgProc();
                            break;
                        #endif

                        case PCI_EVENT_RX_IN_NOTIFY:
                            RxNotifyProc();
                            break;
                        case PCI_EVENT_PCI_IN_Q:
                            InBoundQueueMsgProc();
                            break;
                        case PCI_EVENT_TX_POST_MSG_TOKEN:
                            ReclaimTxPostMsgToken();
                            TxTokenReturnCount ++;
                            isPeerReturnToken = true;
                            break;

                        case PCI_EVENT_DMA_CHAIN_END:
                            TxDmaFinishProc();
                            break;
                    }
                }
            }

            if ( (EV_MSG_Q_LOW & rxEvents) | (EV_MSG_Q_HIGH & rxEvents) )
            {   // queue is not empty, process one at each iteration
                OutBoundMsgProc();
            }
        }

        CheckInboundIntMaskSanity();
        if (!isPeerReturnToken)
        {
            CheckTxTokenSanity();
        }

        TxDmaChannel->PollDmaEnd();
    }

}



/*****************************************************************************
 *
 *   Method:     CTaskPciIf::FindFreeTxBuffer()
 *
 *   Description: Find a free Tx buffer based on the data amount to be transfered
 *                to the peer. If the exact match pool is exhausted, try to 
 *                find a buffer in bigger size pools
 *
 *   Parameters:  isOamMsg -- last pool is for OAM packets only
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
PEER_TX_BUF_NODE * CTaskPciIf::FindFreeTxBuffer(UINT16 bufLen, bool isOamMsg)
{
    PEER_TX_BUF_NODE *node = NULL;

    int maxPoolIndex = (isOamMsg)? BUF_POOL_NUM: (BUF_POOL_NUM -1 );

    for (int poolIndex=PoolIndexTable[bufLen]; poolIndex<maxPoolIndex; poolIndex++)
    {
        node = (PEER_TX_BUF_NODE*)lstGet(&FreePeerTxBufList[poolIndex]);

        if ( NULL == node)
        {
            TxBufRunOutCount[poolIndex]++;
            continue;
        }
        else
        {
            TxBufAllocateCount[poolIndex]++;
            CurrentTxBufCount[poolIndex]++;
            if ( CurrentTxBufCount[poolIndex]>MaxUsedTxBufCount[poolIndex])
            {
                MaxUsedTxBufCount[poolIndex] =  CurrentTxBufCount[poolIndex];
            }
            break;
        }
    }

    if (! isOamMsg)
    {
        if ( NULL == node )
        {
            if (0 == ContinuousRunOutOfTxBufCount)
            {
                FirstRunOutOfTxBufTick = tickGet();
            }
            ContinuousRunOutOfTxBufCount ++;
            if (ContinuousRunOutOfTxBufCount> MaxContRunOutOfTxBufCount )
            {
                MaxContRunOutOfTxBufCount = ContinuousRunOutOfTxBufCount;
            }
            if ( ContinuousRunOutOfTxBufCount > 10000 )
            {
                if (tickGet() - FirstRunOutOfTxBufTick > SecondsToTicks(5))
                {
                    LOG(LOG_CRITICAL, 0, "Need to reboot L2 because keeps running out of Tx buffer");
                    #ifdef M_TGT_L3
                    rebootL2();   // rebootL2
                    #else
                    RequestL3ToResetL2(L2_PCI_IF_L2_REQUEST_REBOOT_IF);   // reboot L2
                    #endif
                    ContinuousRunOutOfTxBufCount = 0;
                    RequestL2RebootCount ++;
                }
            }
        }
        else
        {
            ContinuousRunOutOfTxBufCount = 0;
        }
    }

    return node;
}



void CTaskPciIf::StartPendingDma()
{
    OnFlightDmaDesc = PendingDmaDescHead;
    OnflightTxMsgPostPendingCount = PendingTxMsgPostPendingCount;
    PCI_MSG *tmpBuf = OnflightTxMsgPostPendingBuf;
    OnflightTxMsgPostPendingBuf = PendingTxMsgPostPendingBuf;
    PendingTxMsgPostPendingBuf = tmpBuf;

    LIST* tmpList = OnFlightToFreeMsgList;
    OnFlightToFreeMsgList = PendingToFreeMsgList;
    PendingToFreeMsgList = tmpList;

    TxDmaChannel->StartWithDesc(OnFlightDmaDesc, 
                                (SYS_DMA_FUNCPTR)LastDmaDescCallBack, 
                                (int)this, 
                                0);
    
    PendingDmaDescHead = NULL;
    PendingDmaDescTail = NULL;
    PendingTxMsgPostPendingCount = 0;

    CurrentPendingDescCount = 0;
}

/*****************************************************************************
 *
 *   Method:     CTaskPciIf::OutBoundMsgProc()
 *
 *   Description: transfer ComMessages and IP packets to the peer
 *                create DMA descriptors for each packets, and start DMA if the 
 *                engine is free now
 *
 *   Parameters:  None
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::OutBoundMsgProc()
{
    PCI_IF_Q_MSG msg;
    UINT32 highPriCount=0;

    while (sizeof(PCI_IF_Q_MSG) == msgQReceive(MsgQPciIfHigh, (char*)&msg, sizeof(PCI_IF_Q_MSG), NO_WAIT))
    {
        PrepareDmaForOneMsg(msg);
        HighQueueMsgProcessCount ++;
        highPriCount ++;
        if (CurrentPendingDescCount > MaxPendingDescCount)
        {
            MaxPendingDescCount = CurrentPendingDescCount;
            break;
        }
    }

    if (highPriCount == 0)
    {
        while ( sizeof(PCI_IF_Q_MSG) == msgQReceive(MsgQPciIfLow, (char*)&msg, sizeof(PCI_IF_Q_MSG), NO_WAIT) )
        {
            PrepareDmaForOneMsg(msg);
            if (CurrentPendingDescCount >= SEND_MESSAGE_BATCH_SIZE)
            {
                break;
            }

        }
    }

    if (CurrentPendingDescCount > MaxPendingDescCount)
    {
        MaxPendingDescCount = CurrentPendingDescCount;
    }

    // start DMA if the engine is idle. even if there is no DMA descriptor created
    // this time, there is no harm to start the DMA here
    #ifdef M_TGT_L3
    if ( L2_SYSTEM_STATE_REBOOT != L2RunningState )
    #endif
    {
        if ((!(TxDmaChannel->IsActive())) && NULL== OnFlightDmaDesc && PendingDmaDescHead && HasTxPostMsgToken)
        {
            StartPendingDma();
        }
    }
}


void CTaskPciIf::PrepareDmaForOneMsg(PCI_IF_Q_MSG &msg)
{
    UINT32 msgLen;
    SYS_DMA_DESC *firstDmaDesc,*lastDmaDesc;


    firstDmaDesc = NULL;
    lastDmaDesc = NULL;

    if ( msg.type == TYPE_COMMSG )
    {  // a ComMessage

        TotalTxComMsgCount ++;
        CComMessage *comMsg = msg.comMsg;

        #ifdef M_TGT_L3
        if ( L2_SYSTEM_STATE_REBOOT == L2RunningState )
        {
            ToL2ComMsgDiscardedCount ++;
            comMsg->Destroy();
            return;
        }
        #endif

        msgLen = comMsg->GetDataLength();
        msgLen += sizeof(L2L3_MSG_HEADER);
        UINT32 bufLen = msgLen + L2_L3_PKT_LEADING_SPACE;

        // validate message length
        if (bufLen > PciTxBufSize[BUF_POOL_NUM-1])
        {
            TooLongComMsgCount++;
            comMsg->Destroy();
            return; 
        }

        // validate there is enough leading space in the buffer
        if ((UINT32)comMsg->GetDataPtr() - (UINT32)comMsg->GetBufferPtr() < sizeof(L2L3_MSG_HEADER))
        {
            LOGMSG(LOG_MAJOR, LOG_ERR_PCI_NOT_ENOUGH_LEADING, comMsg, "No Enough leading space in comMsg");
            comMsg->Destroy();
            return;
        }

        ComMsgSendCount[PoolIndexTable[bufLen]]++;

        // find a free Tx buffer in the peer
        PEER_TX_BUF_NODE *node;
        #ifdef M_TGT_L3
        if (comMsg->GetSrcTid() == M_TID_EB )
        #else
        if (comMsg->GetDstTid() == M_TID_EB )
        #endif
        {   // traffic messages
            node = FindFreeTxBuffer(bufLen, false);
        }
        else
        {   // OAM messages
            node = FindFreeTxBuffer(bufLen, true );
        }
        if ( NULL == node )
        {
            comMsg->Destroy();
            return;
        }

        // find a free node to save notification message to peer and comMessage address
        TO_FREE_MSG_LIST_ENTRY *freeNode = (TO_FREE_MSG_LIST_ENTRY*)lstGet(&ToFreeMsgNodePoolList);
        if ( NULL == freeNode)
        {   // discard the message and return the tx buffer node to list
            FreeNodeRunOutCount++;
            comMsg->Destroy();
            lstAdd(&FreePeerTxBufList[node->PoolIndex], &node->lstHdr);
            CurrentTxBufCount[node->PoolIndex]--;
            return;
        }

        // fill in the msg header
        L2L3_MSG_HEADER *header = (L2L3_MSG_HEADER*)(((UINT32) comMsg->GetDataPtr())- sizeof(L2L3_MSG_HEADER));
        header->DestTid = comMsg->GetDstTid();
        header->SrcTid = comMsg->GetSrcTid();
        header->UID = comMsg->GetUID();
        header->EID = comMsg->GetEID();
        header->MsgId = comMsg->GetMessageId();
        header->MsgLen = comMsg->GetDataLength();

        // fill in notification message to peer
        freeNode->SentMsg.type = TYPE_COMMSG;
        freeNode->SentMsg.comMsg = comMsg;
        freeNode->PciMsg.Msg.BufferIndex = node->BufferIndex;
        freeNode->PciMsg.Msg.PoolIndex = node->PoolIndex;
        freeNode->PciMsg.Msg.MUST_BE_ZERO = 0;
        freeNode->PciMsg.Msg.MessageFormat = TYPE_COMMSG;
        freeNode->PciMsg.Msg.MessageLength = msgLen;

        // cache Invlidate is necessary for DMA to the peer Tx buffer
        // even for CPU write, need to invalidate first for the peer
        // to get the data correctly
        cacheInvalidate(DATA_CACHE, (UINT8*)header, msgLen);
        // create one DMA descriptor
        firstDmaDesc=TxDmaChannel->CreateDesc((UINT8*)(node->BufferDmaAddress + L2_L3_PKT_LEADING_SPACE), 
                                                      (UINT8*)header,
                                                      msgLen
                                                      );
        if ( NULL == firstDmaDesc)
        {  // DMA descriptor creation failed, need to destroy the message
           // and free allocated resources
            comMsg->Destroy();
            DmaDescriptorRunOutCount ++;
            lstAdd(&FreePeerTxBufList[node->PoolIndex], &node->lstHdr);
            lstAdd(&ToFreeMsgNodePoolList, &freeNode->lstHdr);
            CurrentTxBufCount[node->PoolIndex]--;
        }
        else
        {
            lastDmaDesc = firstDmaDesc;
            TotalSentComMsgCount ++;
            CurrentPendingDescCount ++;
            node->BufferStatus = OCCUPIED;

            // save to pending buffers
            lstAdd(PendingToFreeMsgList, &freeNode->lstHdr);
            PendingTxMsgPostPendingBuf[PendingTxMsgPostPendingCount++] = freeNode->PciMsg;
        }
    }
    else
    {   // IP packets
        TotalTxIpPktCount ++;

        M_BLK_ID pMblk = msg.mBlk;

        #ifdef M_TGT_L3
        if ( L2_SYSTEM_STATE_REBOOT == L2RunningState )
        {
            netMblkClChainFree(pMblk);
            return; 
        }
        #endif

        // get IP packet length
        UINT16 ipPktLen = pMblk->mBlkPktHdr.len;

        // validate the packet length
        if (ipPktLen > PciTxBufSize[BUF_POOL_NUM-1])
        {   
            TooLongIPPacketCount++;
            netMblkClChainFree(pMblk);
            return; 
        }

        IpPacketSendCount[PoolIndexTable[ipPktLen]]++;

        // try to find a free buffer in the peer
        PEER_TX_BUF_NODE *node=FindFreeTxBuffer(ipPktLen, false);
        if ( NULL == node )
        {
            netMblkClChainFree(pMblk);
            return;
        }

        // try to find a free node to save notification to peer and IP packet to be free
        TO_FREE_MSG_LIST_ENTRY *freeNode = (TO_FREE_MSG_LIST_ENTRY*)lstGet(&ToFreeMsgNodePoolList);
        if ( NULL == freeNode)
        {   // discard the message and return the tx buffer node to list
            FreeNodeRunOutCount++;
            netMblkClChainFree(pMblk);
            lstAdd(&FreePeerTxBufList[node->PoolIndex], &node->lstHdr);
            CurrentTxBufCount[node->PoolIndex]--;
            return;
        }

        SYS_DMA_DESC *tmpDmaDesc;
        UINT32 destAddr = node->BufferDmaAddress;

        // create a DMA descriptor list for the IP packet becasue it may have
        // more than one fragment
        do
        {
            msgLen = pMblk->mBlkHdr.mLen;

            SYS_DMA_DESC *tmpDmaDesc=TxDmaChannel->CreateDesc((UINT8*)destAddr,
                                                          (UINT8*)pMblk->mBlkHdr.mData,
                                                          msgLen
                                                          );
            if ( NULL == tmpDmaDesc)
            {
                DmaDescriptorRunOutCount ++;
                break;
            }
            else
            {
                // cache Invlidate is necessary for DMA to the peer Tx buffer
                // even for CPU write, need to invalidate first for the peer
                // to get the data correctly
                cacheInvalidate(DATA_CACHE, pMblk->mBlkHdr.mData, msgLen);

                destAddr += msgLen;  // move the destination address forward

                if ( NULL == firstDmaDesc )  // first dma descritor 
                {
                    firstDmaDesc = tmpDmaDesc;
                    lastDmaDesc = tmpDmaDesc;
                }
                else
                {
                    lastDmaDesc->AppendNextDesc(tmpDmaDesc);
                    lastDmaDesc = tmpDmaDesc;
                }
            }

            // go prepare for next 
            pMblk = pMblk->mBlkHdr.mNext;

        }while ( NULL != pMblk );

        // successfully prepared the DMA chain, may have only one node in it
        if ( NULL != lastDmaDesc )  
        {
            CurrentPendingDescCount ++;

            // fill in the peer notification message and IP packet information
            freeNode->SentMsg.type = TYPE_IPPACKET;
            freeNode->SentMsg.mBlk = msg.mBlk;
            freeNode->PciMsg.Msg.BufferIndex = node->BufferIndex;
            freeNode->PciMsg.Msg.PoolIndex = node->PoolIndex;
            freeNode->PciMsg.Msg.MUST_BE_ZERO = 0;
            freeNode->PciMsg.Msg.MessageFormat = TYPE_IPPACKET;
            freeNode->PciMsg.Msg.MessageLength = ipPktLen;
            node->BufferStatus = OCCUPIED;
            TotalSentIpPktCount ++;

            // save to pending buffers
            lstAdd(PendingToFreeMsgList, &freeNode->lstHdr);
            PendingTxMsgPostPendingBuf[PendingTxMsgPostPendingCount++] = freeNode->PciMsg;

        }
        else
        {
            // need to return the peer Tx buffer, free node and allocated DMA descriptors
            lstAdd(&FreePeerTxBufList[node->PoolIndex], &node->lstHdr);
            lstAdd(&ToFreeMsgNodePoolList, &freeNode->lstHdr);
            CurrentTxBufCount[node->PoolIndex]--;
            netMblkClChainFree(pMblk);
            if ( NULL != firstDmaDesc)
            {
                // free all the DMA descriptor created for this message
                TxDmaChannel->FreeDesc(firstDmaDesc);
            }
            return;  // forward to prepare next packet, no need to operate on DMA list
        }
    }


    if ( NULL != firstDmaDesc )
    {  // successfully allocated DMA descriptors for the packet,
       // enqueue it to the pending DMA list 
        if ( NULL == PendingDmaDescHead )
        {
            PendingDmaDescHead = firstDmaDesc;
            PendingDmaDescTail = lastDmaDesc;
        }
        else
        {
            PendingDmaDescTail->AppendNextDesc(firstDmaDesc);
            PendingDmaDescTail = lastDmaDesc;
        }
    }
}

/*****************************************************************************
 *
 *   Method:     CTaskPciIf::PostToFreeMsg()
 *
 *   Description: this function is called by DMA call back routine to add 
 *               the free node to the to-be-free list
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::PostToFreeMsg(TO_FREE_MSG_LIST_ENTRY *msg)
{
}
    
/*****************************************************************************
 *
 *   Method:     CTaskPciIf::TxDmaFinishProc()
 *
 *   Description: routine called by the mainloop to process DMA end event
 *                need to start the DMA engine if any descriptor is pending
 *                also need to free all the ComMessage and IP packets
 *                transfered in the last DMA descriptor chain
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::TxDmaFinishProc()
{
    #ifdef M_TGT_L3
    if ( L2_SYSTEM_STATE_RUNNING == L2RunningState)
    #endif
    {
        NotifyPeerInboundMsg();
        NotifyPeerOutboundFreeMsg();
    }

    OnFlightDmaDesc = NULL;
    OnflightTxMsgPostPendingCount = 0;

    ////////////////////////////////
    // free all the ComMessage and IP packets just transfered
    TO_FREE_MSG_LIST_ENTRY *freeNode;

    freeNode = (TO_FREE_MSG_LIST_ENTRY*)lstGet(OnFlightToFreeMsgList);

    // get commessage pointer or IP packet M_BLK from the to-be-free list
    while (freeNode)
    {
        if (freeNode->SentMsg.type == TYPE_COMMSG )
        {
            // free the Com Message
            freeNode->SentMsg.comMsg->Destroy();
        }
        else
        {
            // free the MBlk
            netMblkClChainFree(freeNode->SentMsg.mBlk);
        }

        // return the free node to the pool
        lstAdd(&ToFreeMsgNodePoolList, &freeNode->lstHdr);
        freeNode = (TO_FREE_MSG_LIST_ENTRY*)lstGet(OnFlightToFreeMsgList);
        TxFreeMsgCount ++;
    }
}


/*****************************************************************************
 *
 *   Method:     CTaskPciIf::RxNotifyProc()
 *
 *   Description: routine called by the mainloop when the Inbound Rx Doorbell event is posted 
 *                by the ISR
 *                need to get PCI messages from the circular queue to Rx buffer, send the token
 *                back to the peer, and call Rx buffer process routine.
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
UINT32 InboundProcessRound=0;
UINT32 MaxPoolQueueCount=0;
UINT32 EmptyInboundQueueCount=0;
UINT32 LastPostRound;
void CTaskPciIf::RxNotifyProc()
{
    UINT32 inBoundPostQueHead, inBoundPostQueTail;
    UINT16 inBoundMsgCount=0;

    int readCount=0;
    bool finishedThisRound = false;
    MV64360_REG_RD(MV64360_I2O_IB_POST_TAIL_PTR_CPU0,&inBoundPostQueTail);
    //read the head and tail pointer of the inbound circular queue
    MV64360_REG_RD(MV64360_I2O_IB_POST_HEAD_PTR_CPU0,&inBoundPostQueHead);

    InboundProcessRound ++;
    LastInboundQueueIsrTick = tickGet();
    do
    {
        readCount++;
        if ( inBoundPostQueHead==inBoundPostQueTail)
        {
           if (finishedThisRound && ((inBoundPostQueHead & 0x4)==0))
            {   // may need to process messages posted in two rounds, because
               // we may read faster than the peer write and break out in the middle of last round
               // but always try to read out the coupled buffers, to avoid
               // a break before the last dummy write, which will cause another interrupt
                break;
            }
            if (readCount > 200)
            {
                EmptyInboundQueueCount ++;
                LOG(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF,"Read Inbound Queue failed when got token");
                #ifdef DEBUG_INBOUND_ERROR
                if (!intMaskFail)
                {
                    IntMaskHistory[IntMaskHistoryIndex][0] = 0x2000|finishedThisRound;
                    IntMaskHistory[IntMaskHistoryIndex][2] = tickGet();
                    IntMaskHistory[IntMaskHistoryIndex][3] = inBoundPostQueHead;
                    IntMaskHistory[IntMaskHistoryIndex][4] = inBoundPostQueTail;
                    IntMaskHistoryIndex++;
                    intMaskFail = true;
                }
                #endif
                break; // give up
            }
            //read the head and tail pointer of the inbound circular queue
            MV64360_REG_RD(MV64360_I2O_IB_POST_HEAD_PTR_CPU0,&inBoundPostQueHead);
            continue;
        }

        if (MaxPoolQueueCount < readCount)
        {
            MaxPoolQueueCount = readCount;
        }
        readCount = 0;


        while ( inBoundPostQueHead!=inBoundPostQueTail)
        {
            // process all the inbound messages in the queue
            // need to skip the 2N+1 entry of the queue due to error in Marvell chip
            if ( !(inBoundPostQueTail & 0x4))
            {
                /* Gets the Data From the pointer Address */
                PCI_MSG pciMsg = *((PCI_MSG*)inBoundPostQueTail);

                if (pciMsg.Msg.MUST_BE_ZERO == 1)
                {
                    pciMsg.Msg.MUST_BE_ZERO = 0;
                    LastPostRound = pciMsg.Val;
                    finishedThisRound = true;   // got the last msg
                }
                else
                {
                    finishedThisRound = false;
                    InboundMsgCirQue.QueueEntry[InboundMsgCirQue.QueueHead].pciMsg = *((PCI_MSG*)inBoundPostQueTail);
                    InboundMsgCirQue.QueueEntry[InboundMsgCirQue.QueueHead].postRound = InboundProcessRound;
                    InboundMsgCirQue.QueueHead++;
                    // no need to consider wrap around case, because the buffer size and variable type match
                    inBoundMsgCount ++;
                }
            }

            *(UINT32*)inBoundPostQueTail = 0xFFFFFFFF;   // reset the data in the circular queue

            inBoundPostQueTail += 4;
            // need to consider wrap around senario
            if (inBoundPostQueTail == InBoundQMemTop)
            {
                inBoundPostQueTail = InBoundQMemBase;
            }
        } // while loop, process all the known msg before read the head again
    }while( 1);

    if ( inBoundMsgCount > MaxInboundMsgInOneRound )
    {
        MaxInboundMsgInOneRound = inBoundMsgCount;
    }

    /* incrementing head process: */
    /* updating the pointer back to INBOUND_POST_TAIL_POINTER_REG */
    MV64360_REG_WR(MV64360_I2O_IB_POST_TAIL_PTR_CPU0,inBoundPostQueTail);

    MV64360_REG_WR(MV64360_I2O_IB_INT_CAUSE_CPU0, BIT4);  //clear the interrupt cause register
          // have to do it here, otherwise,there will be some fake interrupt

    taskLock();
    int oldLvl = intLock();
    UINT32 intMask;
    MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &intMask);
    intMask &= ~BIT4;   // enable the inbound message queue interrupt
    MV64360_REG_WR(MV64360_I2O_IB_INT_MASK_CPU0, intMask);
    #ifdef DEBUG_INBOUND_ERROR
    if (!intMaskFail)
    {
        IntMaskHistory[IntMaskHistoryIndex][0] = 0x2000|finishedThisRound;
        IntMaskHistory[IntMaskHistoryIndex][2] = tickGet();
        IntMaskHistory[IntMaskHistoryIndex][3] = inBoundPostQueHead;
        IntMaskHistory[IntMaskHistoryIndex][4] = inBoundPostQueTail;
        MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &IntMaskHistory[IntMaskHistoryIndex][1]);
        IntMaskHistoryIndex++;
    }
    #endif
    intUnlock(oldLvl);
    taskUnlock();	

    // send back the Tx token to the peer
    // trigger a doorbell interrupt to the peer
    SendDoorbellToPeer(DOORBELL_INBOUND_TOKEN_BIT);

    // start the process of queued messages
    InBoundQueueMsgProc();

}
/*****************************************************************************
 *
 *   Method:     CTaskPciIf::InBoundQueueMsgProc()
 *
 *   Description: routine called by the mainloop or RxNotifyProc when the InBound Queue 
 *                is not empty
 *                need to get PCI messages from the circular queue, compose ComMessage
 *                or IP packet based on the PCI message and data transfered from peer to 
 *                local buffer, then forward the received packet to its destination
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
#ifdef DEBUG_PCI_IF
bool InboundFailBufFilled=false;
T_LocalQueueBuffer inboundFailSnapshot;
#endif
void CTaskPciIf::InBoundQueueMsgProc()
{

	UINT32 inBoundMsgCount = 0;

#ifdef DEBUG_PCI_IF
bool failInThisRound=false;
UINT16 quehead = InboundMsgCirQue.QueueHead;
UINT16 quetail = InboundMsgCirQue.QueueTail;
#endif

    // process all the inbound messages in the queue
    while (InboundMsgCirQue.QueueTail != InboundMsgCirQue.QueueHead)
    {
        inBoundMsgCount ++;

        PCI_MSG pciMsg = InboundMsgCirQue.QueueEntry[InboundMsgCirQue.QueueTail].pciMsg;

        UINT16 poolIndex = pciMsg.Msg.PoolIndex;
        UINT16 bufIndex = pciMsg.Msg.BufferIndex;

        if (pciMsg.Msg.MUST_BE_ZERO )
        {
             LOG1(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF,
                 "Got PCI Inbound msg with non MUST_BE_ZERO field 0x%08x", pciMsg.Val);
             PeerPostInvalidBufCount ++;
        }
        else if (pciMsg.Msg.MessageLength >= 8192)
        {
            LOG2(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF,
                "Got PCI Inbound msg with invalid msg length field %d, val=0x%08x", 
                 pciMsg.Msg.MessageLength, pciMsg.Val);
            PeerPostInvalidBufCount ++;
        }
        #ifdef M_TGT_L3
        else if (poolIndex >= BUF_POOL_NUM || bufIndex>=PciL2ToL3BufNum[poolIndex])
        #else
        else if (poolIndex >= BUF_POOL_NUM || bufIndex>=PciL3ToL2BufNum[poolIndex])
        #endif
        {
            LOG2(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF,
                "Got PCI Inbound msg with invalid buffer index[%d][%d]", poolIndex, bufIndex);
            PeerPostInvalidBufCount ++;
        }
        else if ( PCI_RX_BUF_OCCUPIED == RxBufStatusTable[poolIndex][bufIndex].BufferStatus )
        {
            #ifdef DEBUG_PCI_IF
            failInThisRound=true;
            #endif
            
            LOG3(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF, 
                 "Busy Rx buffer pool %d buf %d addr 0x%x is posted again by Peer, discarded",
                 poolIndex, bufIndex, RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress);
            LOG4(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF,
                 "        pComMsg= 0x%x, PostRound = %d, InboundProcessRound = %d,queTail=%d",
                 (int)RxBufStatusTable[poolIndex][bufIndex].pComMsg, 
                 RxBufStatusTable[poolIndex][bufIndex].PostRound,
                 InboundMsgCirQue.QueueEntry[InboundMsgCirQue.QueueTail].postRound,
                 InboundMsgCirQue.QueueTail);

            PeerPostInvalidBufCount++;
        }
        else if (pciMsg.Msg.MessageFormat == TYPE_COMMSG)
        {
            // the inbound message is a ComMessage

            // no need to cacheInvalidate the TX buffer, because
            // write from the peer has snoop, CPU read is always right

            UINT8* bufPtr = (UINT8*)RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress;
            L2L3_MSG_HEADER *dataPtr = (L2L3_MSG_HEADER*) (bufPtr + L2_L3_PKT_LEADING_SPACE);
            UINT32 *payloadPtr = (UINT32*)((UINT32)dataPtr + sizeof(L2L3_MSG_HEADER));
            UINT32 dataLen = dataPtr->MsgLen;

            #ifdef CHECK_BUFFER_FREE_FLAG 
            if ( PCI_BUF_FREE_FLAG != *(UINT32*)bufPtr)
            {
                LOG2(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF, 
                     "Peer uses busy buffer, flag = %x, addr = %x",
                     (int)(*(UINT32*)bufPtr), (int)bufPtr);
            }
            #endif
            if (dataLen >= MAX_TX_BUF_SIZE)
            {
                LOG4(LOG_CRITICAL, LOG_ERR_PCI_POST_BUSY_RX_BUF, 
                     "Peer Send an invalid messge, dst = %d, src=%d, msgid=%d, len = %d",
                     dataPtr->DestTid, dataPtr->SrcTid, dataPtr->MsgId, dataPtr->MsgLen);
                            // post to peer outbound free queue
                SendOutboundFreeMsg(pciMsg);
            }
            else
            {
                // compose a ComMessage
                CComMessage *comMsg = new(this, 0) CComMessage(bufPtr, 
                                                               PciTxBufSize[poolIndex], 
                                                               payloadPtr, 
                                                               dataLen
                                                               );
                comMsg->SetDstTid((TID)dataPtr->DestTid);
                comMsg->SetSrcTid((TID)dataPtr->SrcTid);
                comMsg->SetMessageId(dataPtr->MsgId);
                comMsg->SetUID(dataPtr->UID);
                comMsg->SetEID(dataPtr->EID);
		#ifdef M_TGT_WANIF
                if(Wanif_Switch == 0x5a5a)//加上判断功能开关是否打开判断jiaying20100720
                {
                    if((WorkingWcpeEid==dataPtr->EID)&&((dataPtr->MsgId==MSGID_HIGH_PRIORITY_TRAFFIC)||(dataPtr->MsgId==MSGID_LOW_PRIORITY_TRAFFIC)
    					||(dataPtr->MsgId==MSGID_REALTIME_TRAFFIC)))
                    {
                         comMsg->SetDstTid(M_TID_WANIF);
                    }
                }
		#endif
                // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                //  the flag field is used to save buffer information
                comMsg->SetFlag ( pciMsg.Val);  

                #ifdef CHECK_BUFFER_FREE_FLAG
                * (UINT32*)bufPtr = PCI_BUF_BUSY_FLAG;
                #endif

                RxBufStatusTable[poolIndex][bufIndex].BufferStatus = PCI_RX_BUF_OCCUPIED;
                RxBufStatusTable[poolIndex][bufIndex].pComMsg = comMsg;
                RxBufStatusTable[poolIndex][bufIndex].PostRound = 
                             InboundMsgCirQue.QueueEntry[InboundMsgCirQue.QueueTail].postRound;

                // post the message to its destination as specified in the header field
                if ( !CComEntity::PostEntityMessage(comMsg) )
                {  // message post failed
                    comMsg->Destroy();
                    InboundMsgPostFailureCount++;
                }
                ComMsgReceiveCount[poolIndex]++; 
            }
        }
        else
        {   // IP packets

            IpPacketReceiveCount[poolIndex]++;

            UINT8* bufPtr = (UINT8*)RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress;
            if (PciEndDrv == NULL)
            {
                printf("Got IP packet from PCI interface before END device is ready\n");
            }
            else
            {
                UINT32 msgLen = pciMsg.Msg.MessageLength;

                // allocate a net buffer to hold the IP packet
                M_BLK_ID mBlk = netTupleGet(NetBufPool.pNetPool,
                                  msgLen,
                                  M_DONTWAIT,
                                  MT_DATA,  
                                  true
                                 );

                if ( NULL == mBlk )
                {   // run out of pool buffer
                    NetBufPoolExhaustedCount[PoolIndexTable[msgLen]]++;
                }
                else
                {
                    // need to copy the data to a cluster, then return the Tx buffer to peer right away
                    mBlk->mBlkHdr.mLen = msgLen;
                    mBlk->mBlkPktHdr.len = msgLen;
                    mBlk->mBlkHdr.mFlags |= M_PKTHDR;

                    memcpy(mBlk->mBlkHdr.mData, bufPtr, msgLen);
                    endRcvRtnCall(PciEndDrv, mBlk);

                }
            }

            #ifdef CHECK_BUFFER_FREE_FLAG
            *(UINT32*)bufPtr = PCI_BUF_FREE_FLAG;  
            cacheInvalidate(DATA_CACHE, bufPtr, 4); 
                    // have to do this,because the first 4 bytes are used for IP packet
            #endif

            // post to peer outbound free queue
            SendOutboundFreeMsg(pciMsg);
        }

#ifndef DEBUG_PCI_IF
        InboundMsgCirQue.QueueEntry[InboundMsgCirQue.QueueTail].pciMsg.Val = 0xFFFFFFFF;
#endif
        InboundMsgCirQue.QueueTail = InboundMsgCirQue.QueueTail + 1;  // no need to consider wrap around 

        if (inBoundMsgCount >= 100)
        {  // only process 500 msg in one round, to give other events the chance to run
            break;
        }
    }

#ifdef DEBUG_PCI_IF
if (failInThisRound)
{
InboundFailBufFilled = true;
memcpy(&inboundFailSnapshot, &InboundMsgCirQue, sizeof(InboundMsgCirQue));
inboundFailSnapshot.QueueHead = quehead;
inboundFailSnapshot.QueueTail = quetail;
}
#endif

    if (InboundMsgCirQue.QueueHead != InboundMsgCirQue.QueueTail)
    {   // post event let mainloop keep going
        PostEvent(PCI_EVENT_PCI_IN_Q);  
    }
}
    
/*****************************************************************************
 *
 *   Method:     CTaskPciIf::NotifyPeerOutboundFreeMsg()
 *
 *   Description: routine called by DeallocateComMessage to free the Com Message
 *                created by PciIf, or by InboundMsgQueueProc() to free the 
 *                buffer used for an IP packet.
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::SendOutboundFreeMsg(PCI_MSG pciMsg)
{
    taskLock();
    UINT32 oldLvl = intLock();
    RxMsgFreePendingBuf[RxMsgFreePendingCount++] = pciMsg;
    intUnlock(oldLvl);	
    taskUnlock();//liuweidong

    if ( NULL == OnFlightDmaDesc )
    {    // only notify peer the free of Rx messages when the DMA is not ongoing.
        NotifyPeerOutboundFreeMsg();
    }

}

void CTaskPciIf::NotifyPeerOutboundFreeMsg()
{
    taskLock();
    UINT32 oldLvl = intLock();
    // this routine must be called with taskLock() executed
    if (RxMsgFreePendingCount)
    {
        for (int i=0; i<RxMsgFreePendingCount; i++)
        {
            // From ERRATA: PCI access to outbound free queue:
            // 2N entry in the queue is written to the 2N+1 entry
            // 2N+1 entry is written to the 2N+1 entry,
            // need to post twice to correct the problem
            *MV64360_REG_PEER_OUTB_FREE_QUEUE = RxMsgFreePendingBuf[i].Val;
            *MV64360_REG_PEER_OUTB_FREE_QUEUE = RxMsgFreePendingBuf[i].Val;
        }
        
        if (MaxOutboundFreeSendCount < RxMsgFreePendingCount)
        {
            MaxOutboundFreeSendCount = RxMsgFreePendingCount;
        }
        RxMsgFreePendingCount = 0;

        NotifyPeerOutboundFreeCount++;
    }
    intUnlock(oldLvl);
    taskUnlock();
}


/*****************************************************************************
 *
 *   Method:     CTaskPciIf::DeallocateComMessage()
 *
 *   Description: routine called by other ComEntity to free the Com Message
 *                created by PciIf. All the ComMessages from peer has the 
 *                flag field != 0xffffffff
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
bool CTaskPciIf::DeallocateComMessage(CComMessage* pComMsg)
{
    if (pComMsg->GetFlag ()!= 0xffffffff)
    {  // to free a message using PCI Tx buffers

        // post an outbound free message to the peer
        PCI_MSG pciMsg;
        pciMsg.Val = pComMsg->GetFlag();

        UINT16 poolIndex = pciMsg.Msg.PoolIndex;
        UINT16 bufIndex = pciMsg.Msg.BufferIndex;
        #ifdef M_TGT_L3
        if (poolIndex >= BUF_POOL_NUM || bufIndex>=PciL2ToL3BufNum[poolIndex])
        #else
        if (poolIndex >= BUF_POOL_NUM || bufIndex>=PciL3ToL2BufNum[poolIndex])
        #endif
        {
            LOG2(LOG_CRITICAL, LOG_ERR_PCI_APP_FREE_INVALID_BUF, 
                 "App free invalid PCI buffer, pool %d, buf %d", poolIndex, bufIndex);
        }
        else
        {
            UINT32 *bufAddr = (UINT32*)RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress;
            if (pComMsg->GetBufferPtr() != (void *)RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress)
            {
                LOG2(LOG_CRITICAL, LOG_ERR_PCI_APP_FREE_INVALID_BUF,
                     "App free with buffer pointer changed -- should be %x, is %x",
                     (int)RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress,
                     (int)(pComMsg->GetBufferPtr())
                );
            }

            #ifdef M_TGT_L3
            if (L2_SYSTEM_STATE_RUNNING == L2RunningState)
            // return the Tx buffer to L2 only when it is in running state
            #endif
            {
                #ifdef CHECK_BUFFER_FREE_FLAG
                if ( PCI_BUF_BUSY_FLAG != *bufAddr )
                {
                    LOG1(LOG_CRITICAL, LOG_ERR_PCI_APP_FREE_INVALID_BUF,
                         " App Free a comMsg with a buffer with status flag = %x, (should be 0xaaaaaaaa)",
                         *bufAddr);
                }
                #endif

                if (PCI_RX_BUF_EMPTY == RxBufStatusTable[poolIndex][bufIndex].BufferStatus)
                {
                    LOG(LOG_CRITICAL, LOG_ERR_PCI_APP_FREE_INVALID_BUF, "App free empty PCI buffer");
                }
                else
                {
                    #ifdef CHECK_BUFFER_FREE_FLAG
                    // FIXME, need to get rid of the following checking, 
                    // it is allowed to change comMsg for the buffer in application processing
                    if (RxBufStatusTable[poolIndex][bufIndex].pComMsg != pComMsg)
                    {
                        LOG3(LOG_CRITICAL, LOG_ERR_PCI_APP_FREE_INVALID_BUF,
                             "App free ComMsg with changed Buffer, \n           orig ComMsg = %x, InboundProcessRound = %d, bufAddr=%x",
                             (int)RxBufStatusTable[poolIndex][bufIndex].pComMsg,
                             (int)RxBufStatusTable[poolIndex][bufIndex].PostRound, 
                             (int)RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress);
                    }
                    #endif

                    // change the flag in the beginning of the buffer to free pattern
                    RxBufStatusTable[poolIndex][bufIndex].BufferStatus = PCI_RX_BUF_EMPTY;
                    RxBufStatusTable[poolIndex][bufIndex].pComMsg = NULL;
                    RxBufStatusTable[poolIndex][bufIndex].PostRound = 0;

                    #ifdef CHECK_BUFFER_FREE_FLAG
                    *bufAddr = PCI_BUF_FREE_FLAG;  
                    cacheInvalidate(DATA_CACHE, bufAddr, 128); 
                    // need to flush the cache because when the buffer is used for the next round
                    // the CPU may flush the cache, which will cause the memory be updated with
                    // value in cache, causing problems. This is a must because the recipient
                    // may change the buffer memory which is cacheable (such as VLAN tagging)
                    #endif

                    // inform peer the free of the tx buffer
                    SendOutboundFreeMsg(pciMsg);

                    AppFreeCount[pciMsg.Msg.PoolIndex]++;
                }
            }
            #ifdef M_TGT_L3
            #ifdef CHECK_BUFFER_FREE_FLAG
            else //if (L2RunningState = L2_SYSTEM_STATE_RUNNING)
            {
                *bufAddr = PCI_BUF_FREE_FLAG;  
                cacheInvalidate(DATA_CACHE, bufAddr, 128); 
            }
            // return the Tx buffer to L2 only when it is in running state
            #endif
            #endif
        }
        // delete the ComMessage object
        pComMsg->SetBuffer(NULL, 0);
    }

    return CComEntity::DeallocateComMessage(pComMsg);
}


/*****************************************************************************
 *
 *   Method:     CTaskPciIf::OutBoundFreeQueueMsgProc()
 *
 *   Description: routine called by mainloop periodially to get the messages
 *                in the outbound free circular queue posted by peer when
 *                the peer has finished using any Tx buffer
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::OutBoundFreeQueueMsgProc()
{

    UINT32 outBoundFreeQueHead, outBoundFreeQueTail;

    #ifdef M_TGT_L3
    if ( L2_SYSTEM_STATE_REBOOT == L2RunningState )
    {   // wait for the pointers to settle down 
        return;
    }
    #endif

    // read the outbound free queue head and tail
    MV64360_REG_RD(MV64360_I2O_OB_FREE_HEAD_PTR_CPU0, &outBoundFreeQueHead);
    MV64360_REG_RD(MV64360_I2O_OB_FREE_TAIL_PTR_CPU0, &outBoundFreeQueTail);

	UINT32 outBoundFreeMsgCount=0;
    while (outBoundFreeQueHead != outBoundFreeQueTail)
    {
        // need to skip 2N entry due to bug of Marvell chip 
        if (outBoundFreeQueTail & 0x4)
        {
            PCI_MSG pciMsg;
            /* Gets the Data From the pointer Address */
            pciMsg = *(PCI_MSG*)outBoundFreeQueTail;
            UINT32 maxBufIndex;
            #ifdef M_TGT_L3
            maxBufIndex = PciL3ToL2BufNum[pciMsg.Msg.PoolIndex];
            #else
            maxBufIndex = PciL2ToL3BufNum[pciMsg.Msg.PoolIndex];
            #endif

            if (   pciMsg.Msg.MUST_BE_ZERO 
                || pciMsg.Msg.PoolIndex >= BUF_POOL_NUM 
                || pciMsg.Msg.BufferIndex>= maxBufIndex )
            {
                LOG2(LOG_CRITICAL, LOG_ERR_PCI_FREE_BUFFER_INVALID, 
                     "Peer try to free invalid buffer pool = %d, index=%d\n",
                     pciMsg.Msg.PoolIndex, pciMsg.Msg.BufferIndex);
                PeerFreeInvalidBufCount ++;
            }
            else
            {
                // return the tx buffer to the list
                PEER_TX_BUF_NODE *node = &PeerTxBufNode[pciMsg.Msg.PoolIndex][pciMsg.Msg.BufferIndex];
                UINT32 *peerBufAddr = (UINT32*)node->BufferCpuAddress;
                
                if (node->BufferStatus == FREE)
                {
                    LOG4(LOG_CRITICAL, LOG_ERR_PCI_FREE_BUF_FREE, 
                         "Peer try to free buffer not allocated, pool = %d, index=%d, bufferAddr = 0x%x, queTail=0x%x\n",
                         pciMsg.Msg.PoolIndex, pciMsg.Msg.BufferIndex, 
                         (UINT32)peerBufAddr,
                         outBoundFreeQueTail);
                    PeerFreeInvalidBufCount ++;
                }
                #ifdef CHECK_BUFFER_FREE_FLAG
                else if ( PCI_BUF_BUSY_FLAG == *peerBufAddr )
                {
                    LOG4(LOG_CRITICAL, LOG_ERR_PCI_FREE_BUF_FREE, 
                         "Peer try to free buffer still in use, pool = %d, index=%d, bufferAddr = 0x%x, queTail=0x%x\n",
                         pciMsg.Msg.PoolIndex, pciMsg.Msg.BufferIndex, 
                         node->BufferCpuAddress,
                         outBoundFreeQueTail);
                    LOG4(LOG_CRITICAL, LOG_ERR_PCI_FREE_BUF_FREE,
                         "Node pool = %d, index=%d, bufferAddr = 0x%x, value=0x%x\n",
                         node->PoolIndex, node->BufferIndex, 
                         (UINT32)peerBufAddr, *peerBufAddr);
                    PeerFreeInvalidBufCount++;
                }
                #endif
                else
                {
                    node->BufferStatus = FREE;
                    lstAdd(&FreePeerTxBufList[pciMsg.Msg.PoolIndex], &node->lstHdr);

                    TxBufFreeCount[pciMsg.Msg.PoolIndex]++;
                    CurrentTxBufCount[pciMsg.Msg.PoolIndex]--;
                }
            }
			outBoundFreeMsgCount ++;
        }

        *(UINT32*) outBoundFreeQueTail = 0xFFFFFFFF;

        outBoundFreeQueTail += 4;
        if (outBoundFreeQueTail == OutBoundFreeQMemTop)
        {
            outBoundFreeQueTail = OutBoundFreeQMemBase;
        }
        if ( 0 == (outBoundFreeMsgCount %1024 ))
        {
            MV64360_REG_WR(MV64360_I2O_OB_FREE_TAIL_PTR_CPU0,outBoundFreeQueTail);
        }
    }

    /* incrementing head process: */
    /* updating the pointer back to OUTBOUND_FREE_TAIL_POINTER_REG */
    MV64360_REG_WR(MV64360_I2O_OB_FREE_TAIL_PTR_CPU0,outBoundFreeQueTail);

	if (outBoundFreeMsgCount > MaxOutboundFreeMsgInOneRound)
	{
		MaxOutboundFreeMsgInOneRound = outBoundFreeMsgCount;
	}

    // send back the Post Free Msg token to the peer
    // trigger a doorbell interrupt to the peer
//    SendDoorbellToPeer(DOORBELL_OUTBOUND_FREE_TOKEN_BIT);
}

/*****************************************************************************
 *
 *   Method:     CTaskPciIf::InBoundMsgProc()
 *
 *   Description: routine called by mainloop when any In bound message is posted
 *                 by L2. Only L3 interface has this function. Need to notify
 *               L3 Boot task.
 *                If the L2 CPU just reboot and ready to receive packets, the pool nees to be rebuilt
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
UINT32 PciIsrCount=0;
UINT32 ISRBreakOutCount=0;
UINT32 InboundPostIsrCount=0;
UINT32 FakeInboundPostIsrCount = 0;
UINT32 InboundFreeTokenCount=0;

#ifdef M_TGT_L3
UINT32 StopDmaDescCount = 0;
extern void OAM_LOGSTR(LOGLEVEL level, UINT32 errcode, const char* text);
extern void OAM_LOGSTR1(LOGLEVEL level, UINT32 errcode, const char* text, int arg1);
T_TimeDate LastL2BootUpTime;

void CTaskPciIf::InBoundMsgProc()
{
    UINT32 msg;
    MV64360_REG_RD(MV64360_I2O_IB_MSG0_CPU0, &msg);

    switch (msg )
    {
        case L2_PCI_IF_READY_NOTIFY:
            OAM_LOGSTR(LOG_CRITICAL, LOG_ERR_PCI_L2_REBOOT, "L2 PCI interface is ready for message exchange");

            LastL2BootUpTime = bspGetDateTime();
            L2RunningState = L2_SYSTEM_STATE_RUNNING;
            ContinuousRunOutOfTxBufCount = 0;
            MaxContRunOutOfTxBufCount = 0;

            L2BootUpCount ++;
            break;

        case L2_PCI_IF_L2_REQUEST_REBOOT_IF:
        case L2_PCI_IF_L2_REQUEST_REBOOT_CSI:
        case L2_PCI_IF_L2_REQUEST_REBOOT_CMD:
            {
                static char *reason[3] = {"L3 interface Lockup",
                                          "CSI - SW abnormal",
                                          "reboot"
                                         };
                OAM_LOGSTR1(LOG_CRITICAL, LOG_ERR_PCI_L2_REBOOT, "L2 PPC Request L3 for reboot %s",
                          (int) reason[msg-L2_PCI_IF_L2_REQUEST_REBOOT_IF] );

                L2RequestForResetCount[msg-L2_PCI_IF_L2_REQUEST_REBOOT_IF] ++;
                CComMessage *comMsg = new (this, 0) CComMessage;

                comMsg->SetDstTid(M_TID_BM);
                comMsg->SetSrcTid(M_TID_L2IF);
                comMsg->SetMessageId(L2_PCI_IF_L2_REQUEST_REBOOT_IF);
                comMsg->SetDataLength(0);
                comMsg->SetFlag(0xffffffff);
                if ( false == CComEntity::PostEntityMessage(comMsg))
                {
                    comMsg->Destroy();
                }
            }

            break;
        case L2_PCI_IF_READY_FOR_BOOT:
            {
                taskLock();
		int oldLvl = intLock();
                L2RunningState = L2_SYSTEM_STATE_REBOOT;

                UINT32 inBoundPostQueHead, outBoundFreeQueHead;
                //empty the circular queue
                MV64360_REG_RD(MV64360_I2O_IB_POST_HEAD_PTR_CPU0,&inBoundPostQueHead);
                MV64360_REG_WR(MV64360_I2O_IB_POST_TAIL_PTR_CPU0,inBoundPostQueHead);

                MV64360_REG_RD(MV64360_I2O_OB_FREE_HEAD_PTR_CPU0, &outBoundFreeQueHead);
                MV64360_REG_WR(MV64360_I2O_OB_FREE_TAIL_PTR_CPU0,outBoundFreeQueHead);

                UINT32 regVal;
                regVal = ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT16);    
                              // enable inbound message, inbound circular queue,inbound doorbell low bits 
                              // and outbound free queue overflow
                MV64360_REG_WR(MV64360_I2O_IB_INT_MASK_CPU0,regVal);

                memset((char *)PCI_CIRCULAR_BUF_ADDR, 0xff, PCI_CIRCULAR_BUF_SIZE);

                InboundMsgCirQue.QueueHead = 0;
                InboundMsgCirQue.QueueTail = 0;

                // clear the pending DMAs, the pending To free list does not need to be 
                // cleared, the next DMA finish event will free the ComMsg in the pending list
                CurrentPendingDescCount = 0;   
                if (PendingDmaDescHead)
                {
                    // free all the DMA descriptor created for this message
                    TxDmaChannel->FreeDesc(PendingDmaDescHead);
                    PendingDmaDescHead = NULL;
                    PendingDmaDescTail = NULL;
                }

                // reclaim the tokens
                HasTxPostMsgToken = true; 
                TotalPostMsgCount=0;
                intUnlock(oldLvl);

                PciIsrCount=0;
                ISRBreakOutCount=0;
                InboundPostIsrCount=0;
                InboundFreeTokenCount=0;

                InboundProcessRound = 0;
                NotifyPeerInboundCount = 0;
                TxTokenReturnCount = 0;
                NotifyPeerOutboundFreeCount = 0;
                ForceTxPostMsgTokenCount = 0;

                // rebuild the Tx buffer pool to retrieve any occupied tx buffer 
                UINT32 *bufAddr;
                for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
                {
                    lstInit( &FreePeerTxBufList[poolIndex]);
                    for (int bufIndex=0; bufIndex<PciL3ToL2BufNum[poolIndex]; bufIndex++)
                    {
                        PeerTxBufNode[poolIndex][bufIndex].BufferStatus = FREE;
                        lstAdd(&FreePeerTxBufList[poolIndex], &PeerTxBufNode[poolIndex][bufIndex].lstHdr);
                    }

                    for (int bufIndex = 0; bufIndex<PciL2ToL3BufNum[poolIndex];bufIndex++)
                    {
                        RxBufStatusTable[poolIndex][bufIndex].BufferStatus = PCI_RX_BUF_EMPTY;
                        RxBufStatusTable[poolIndex][bufIndex].pComMsg = NULL;
                        RxBufStatusTable[poolIndex][bufIndex].PostRound = 0;


                        bufAddr = (UINT32*)RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress;

                        #ifdef CHECK_BUFFER_FREE_FLAG
                        *bufAddr = PCI_BUF_FREE_FLAG;
                        #endif
                    }

                    TxBufAllocateCount[poolIndex] = 0;
                    TxBufFreeCount[poolIndex] = 0;
                    CurrentTxBufCount[poolIndex] = 0;
                    IpPacketSendCount[poolIndex] = 0;
                    IpPacketReceiveCount[poolIndex] = 0;
                    ComMsgSendCount[poolIndex] = 0;
                    ComMsgReceiveCount[poolIndex] = 0;
                    AppFreeCount[poolIndex] = 0;
                    MaxUsedTxBufCount[poolIndex] = 0;
                }
                taskUnlock();

                CComMessage *comMsg = new (this, 0) CComMessage;

                ////////////////////////////////////
                // notify BOOT task that the L2 is ready to be booted
                if (comMsg)
                {
                    comMsg->SetDstTid(M_TID_BM);
                    comMsg->SetSrcTid(M_TID_L2IF);
                    comMsg->SetMessageId(msg);
                    comMsg->SetDataLength(0);
                    comMsg->SetFlag(0xffffffff);

                    if ( false == CComEntity::PostEntityMessage(comMsg))
                    {
                        comMsg->Destroy();
                    }
                }

                //////////////////////////////////////////
                // notify L2TxInL3 task that the L2 has reset
                comMsg = new (this, sizeof(stL2RxResetNotify)) CComMessage;
                if (comMsg)
                {
                    comMsg->SetDstTid(M_TID_L2_TXINL3);
                    comMsg->SetSrcTid(M_TID_L2IF);
                    comMsg->SetMessageId(L3_ToL2Tx_L2Reset_Notify);
                    comMsg->SetDataLength(sizeof(stL2RxResetNotify));
                    comMsg->SetFlag(0xffffffff);
                    memset(comMsg->GetDataPtr(), 0, sizeof(stL2RxResetNotify));

                    if ( false == CComEntity::PostEntityMessage(comMsg))
                    {
                        comMsg->Destroy();
                    }
                }

            }
            break;
    }

}


#endif



/*****************************************************************************
 *
 *   Method:     CTaskPciIf::PciInBoundISR()
 *
 *   Description: Inbound Interrupt Service Routine.
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
//#define DEBUG_PCI_ISR
#ifdef DEBUG_PCI_ISR
UINT32 MaxLoopInISR=0;
UINT32 PciIsrCauseRecord[10];
UINT32 PciIsrMaskRecord[10];
#endif

void CTaskPciIf::PciInBoundISR(CTaskPciIf *taskObj)
{
    UINT32 cause, intMask;
    #ifdef DEBUG_PCI_ISR
    UINT32 causeRecord[10];
    UINT32 maskRecord[10];
    #endif
    static UINT32 lastInboundQueueHead = 0;
    UINT32 queHead;

    PciIsrCount++;

    int loopCount =0;

    MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &intMask);

    // read the cause register
    MV64360_REG_RD(MV64360_I2O_IB_INT_CAUSE_CPU0, &cause);

    while ( 0 != ((~intMask) &cause) )
    {
        #ifdef DEBUG_PCI_ISR
        causeRecord[loopCount] = cause;
        MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &maskRecord[loopCount]);
        #endif
        
        cause &= (~intMask);   // more than one interrupt may come, but some maybe disabled,
                               // should not clear those interrupt
        // clear the interrupt
        MV64360_REG_WR(MV64360_I2O_IB_INT_CAUSE_CPU0, cause);

        #ifdef M_TGT_L3
        if (cause & BIT16)
        {   // inbound message 1 interrupt
#if 1
            register UINT32 msg;
            register int lvl;

            lvl = ::intLock();
            //MV64360_REG_RD(MV64360_I2O_IB_MSG1_CPU0, &msg);
            msg = *(volatile UINT32*)(MV64360_BASE_ADRS + MV64360_I2O_IB_MSG1_CPU0);
            asm volatile ("eieio ");

            UINT16 msgId= msg >> 16;
            UINT8 ts = msg & 0xff;

            
            if (ts<9)
            {
                L2TxTimingCountIsr[ts]++;
                //notify L2 that L3 is ready to accept another L2 timing event  by sending a inbound message
                //*(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_MSG0_CPU0) = 
                //             LONGSWAP (L3_PCI_IF_L2_TIMING_TOKEN);

                *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_MSG0_CPU0) = msg;
                asm volatile ("eieio ");
                *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_PCI0_SYNC_BARR_TRGR_REG) = 1;
                asm volatile ("eieio ");
                
                sendL1TddMsgToL2Tx(msgId, ts);
            }
            ::intUnlock(lvl);
#else
            register UINT32 msg;

            msg = *(volatile UINT32*)(MV64360_BASE_ADRS + MV64360_I2O_IB_MSG1_CPU0);

            //notify L2 that L3 is ready to accept another L2 timing event  by sending a inbound message
            *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_MSG0_CPU0) = msg;
	     asm volatile ("eieio ");
            *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_PCI0_SYNC_BARR_TRGR_REG) = 1;
	     asm volatile ("eieio ");
            
            taskObj->PostL1TddEvent(msg);  
#endif
        }
        
        
        if (cause & BIT0)
        {  
            // inbound message 0 interrupt
            taskObj->PostEvent(PCI_EVENT_PCI_IN_MSG);
        }

        #else
        if (cause & BIT0)
        {
            register UINT32 msg;
            register int ts;
            unsigned int time;

            //MV64360_REG_RD(MV64360_I2O_IB_MSG0_CPU0, &msg);
            msg = *(volatile UINT32*)(MV64360_BASE_ADRS + MV64360_I2O_IB_MSG0_CPU0);
            ts = msg & 0xff;
            if (ts<9)
            {
                taskObj->L2TimingTokenReclaim();
            }
        }
        
        #endif

        if (cause & BIT4)
        {
            MV64360_REG_RD(MV64360_I2O_IB_POST_HEAD_PTR_CPU0,&queHead);
            if (queHead != lastInboundQueueHead)
            {
                taskObj->PostEvent(PCI_EVENT_RX_IN_NOTIFY);
                // inbound post queue interrupt
                InboundPostIsrCount ++;
                taskObj->LastInboundQueueIsrTick = tickGet();
                lastInboundQueueHead = queHead;

                MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &intMask);
                intMask |= BIT4;   // disable the inbound interrupt before the mainloop prcess it
                MV64360_REG_WR(MV64360_I2O_IB_INT_MASK_CPU0, intMask);

                #ifdef DEBUG_INBOUND_ERROR
                if (!intMaskFail)
                {
                    IntMaskHistory[IntMaskHistoryIndex][0] = 1;
                    IntMaskHistory[IntMaskHistoryIndex][2] = tickGet();
                    MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &IntMaskHistory[IntMaskHistoryIndex][1]);
                    IntMaskHistory[IntMaskHistoryIndex][3] = queHead;
                    MV64360_REG_RD(MV64360_I2O_IB_POST_TAIL_PTR_CPU0,&IntMaskHistory[IntMaskHistoryIndex][4]);
                    IntMaskHistoryIndex++;
                }
                #endif
            }
            else
            {
                FakeInboundPostIsrCount++;
            }
        }

        if (cause & BIT5)
        {   // outbound free queue overflow interrupt
            taskObj->PostEvent(PCI_EVENT_PCI_OUT_F_Q);
        }

        if (cause & BIT1 )
        {
            // inbound doorbell interrupt
            UINT32 dblVal;  // clear the doorbell register to de-assert the interrupt
            MV64360_REG_RD(MV64360_I2O_IB_DBL_CPU0, &dblVal);
            MV64360_REG_WR(MV64360_I2O_IB_DBL_CPU0, dblVal);

            if (dblVal & DOORBELL_INBOUND_TOKEN_BIT)
            {
                InboundFreeTokenCount++;
                taskObj->PostEvent(PCI_EVENT_TX_POST_MSG_TOKEN);
                taskObj->TxPostMsgTokenLostTick = tickGet();
            }
        }

        // read in the cause register again 
        MV64360_REG_RD(MV64360_I2O_IB_INT_CAUSE_CPU0, &cause);

        loopCount++;
        if(loopCount>=10)
        {
            ISRBreakOutCount++;
            break;
        }
    };
    #ifdef DEBUG_PCI_ISR
    if (loopCount > MaxLoopInISR) 
    {
        MaxLoopInISR = loopCount;
        memcpy(PciIsrCauseRecord, causeRecord, loopCount*4);
        memcpy(PciIsrMaskRecord, maskRecord, loopCount*4);
    }
    #endif
}

/*****************************************************************************
 *
 *   Method:     CTaskPciIf::NotifyPeerInboundMsg()
 *
 *   Description: Send all the buffered Post PCI messages to the peer.
 *                 called by the last DMA finish ISR, or when the peer returns the token
 *                If called from non-ISR, need to disable the interrupts to prevent
 *                any last DMA finish ISR from sending the buffer again.
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::NotifyPeerInboundMsg()
{
    // this routine can only be called when there is token and buffered msg to send
     HasTxPostMsgToken = false;
     TxPostMsgTokenLostTick = tickGet();

     // post an invalid msg as the indicator of end of this round
     NotifyPeerInboundCount ++;
     PCI_MSG  lastMsg;
     lastMsg.Val = NotifyPeerInboundCount;
     lastMsg.Msg.MUST_BE_ZERO = 1;
     taskLock();
     UINT32 oldLvl = intLock();

     for (int i= 0; i<OnflightTxMsgPostPendingCount; i++)
     {
         // From ERRATA: PCI access to inbound post queue:
         // 2N entry in the queue is written to the 2N entry
         // 2N+1 entry is written to the 2N entry,
         // need to post twice to correct the problem
         *MV64360_REG_PEER_INB_POST_QUEUE = OnflightTxMsgPostPendingBuf[i].Val;
         *MV64360_REG_PEER_INB_POST_QUEUE = OnflightTxMsgPostPendingBuf[i].Val;
     }

     *MV64360_REG_PEER_INB_POST_QUEUE = lastMsg.Val;
     *MV64360_REG_PEER_INB_POST_QUEUE = lastMsg.Val;

     intUnlock(oldLvl);
    taskUnlock();

     if (MaxInboundSendCount < OnflightTxMsgPostPendingCount)
     {
         MaxInboundSendCount = OnflightTxMsgPostPendingCount;
     }
TotalPostMsgCount += OnflightTxMsgPostPendingCount;
     OnflightTxMsgPostPendingCount = 0;
}




/*****************************************************************************
 *
 *   Method:     CTaskPciIf::LastDmaDescCallBack()
 *
 *   Description: DMA call back routine hooked to last DMA descriptor
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::LastDmaDescCallBack(CTaskPciIf *obj, int dummyArg)
{
    // post an event to let the mainloop process the DMA finish event
    obj->PostEvent(PCI_EVENT_DMA_CHAIN_END);
}


/*****************************************************************************
 *
 *   Method:     CTaskPciIf::PostEvent()
 *
 *   Description: routine called by ISRs to notify any event to the mainloop
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
void CTaskPciIf::PostEvent(T_PciIfEvent event)
{
    if ( OK != msgQSend(MsgQEvent, (char*)(&event), sizeof(T_PciIfEvent),NO_WAIT, MSG_PRI_NORMAL))
    {
        logMsg("Pci If Post Event Fail \n", 0,0,0,0,0,0);
    }
}

void CTaskPciIf::PostL1TddEvent(UINT32 event)
{
    if ( OK != msgQSend(MsgQL1TddEvent, (char*)(&event), sizeof(UINT32),NO_WAIT, MSG_PRI_NORMAL))
    {
        logMsg("Pci If Post L1Tdd Event Fail \n", 0,0,0,0,0,0);
    }
}

    
void CTaskPciIf::ReclaimTxPostMsgToken()
{
    HasTxPostMsgToken = true;

    // start the pending DMAs
    if ((!(TxDmaChannel->IsActive())) && PendingDmaDescHead && (NULL==OnFlightDmaDesc) )
    {
        StartPendingDma();
    }
}


/*****************************************************************************
 *
 *   Method:     CTaskPciIf::PciEndSend()
 *
 *   Description: routine provided to PCI END driver to transfer an IP packet to 
 *                the other end
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
STATUS CTaskPciIf::PciEndSend( M_BLK_ID pMblk)
{
    #ifdef M_TGT_L3
    if ( L2_SYSTEM_STATE_REBOOT == L2RunningState )
    {
        ToL2IpPacketDiscardedCount ++;
        netMblkClChainFree(pMblk);
        return ERROR;
    }
    #endif
    
    PCI_IF_Q_MSG postMsg;
    postMsg.type = TYPE_IPPACKET;
    postMsg.mBlk = pMblk;
    STATUS rc = msgQSend(MsgQPciIfLow, (char*)(&postMsg), sizeof(PCI_IF_Q_MSG),NO_WAIT, MSG_PRI_NORMAL);
    if ( OK != rc)
    {
        LOG(LOG_CRITICAL,LOG_ERR_PCI_QUEUE_OVERFLOW, "Post IP packet to PCI interface message q failed");
        netMblkClChainFree(pMblk);
        return ERROR;
    }
    else
    {
        return OK;
    }
}

/*****************************************************************************
 *
 *   Method:     PciIfSendIpFunc()
 *
 *   Description: routine provided to PCI END driver to transfer an IP packet to 
 *                the other end, used as MUX interface function
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
STATUS PciIfSendIpFunc(M_BLK_ID pMblk)
{
    if ( NULL == PciIfObj )
    {
        printf("Got an IP packet to PCI interface before the driver is ready\n");
        return ERROR;
    }
    return PciIfObj->PciEndSend(pMblk);
}


void CTaskPciIf::ShowStatus(int verbose)
{
    T_TimeDate TimeData = bspGetDateTime();
	printf("At %04d/%02d/%02d %02d:%02d:%02d \n ", TimeData.year, TimeData.month, TimeData.day, TimeData.hour, TimeData.minute, TimeData.second);    
    printf("============PCI Interface Counters: ==========================\n");
    printf("                         | pool 0   | pool 1   | pool 2   | pool 3   | pool 4   |\n");
    printf("Buffer start Addr        |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("0x%08x|", PeerTxBufNode[poolIndex][0].BufferDmaAddress);
    printf("\n");

    printf("Buffer Count             |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
    #ifdef M_TGT_L3
        printf("%10d|", PciL3ToL2BufNum[poolIndex]);
    #else
        printf("%10d|", PciL2ToL3BufNum[poolIndex]);
    #endif
        
    
    printf("\n");

    printf("Buffer Size              |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10d|", PciTxBufSize[poolIndex]);
    printf("\n");


    printf("IpPacketSendCount        |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|", IpPacketSendCount[poolIndex]);
    printf("\n");

    printf("IpPacketReceiveCount     |"); 
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|", IpPacketReceiveCount[poolIndex]);
    printf("\n");

    printf("ComMsgSendCount          |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|", ComMsgSendCount[poolIndex]);
    printf("\n");

    printf("ComMsgReceiveCount       |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|",  ComMsgReceiveCount[poolIndex]);
    printf("\n");

    printf("TxBufRunOutCount         |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|",  TxBufRunOutCount[poolIndex]);
    printf("\n");

    printf("TxBufAllocateCount       |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|",  TxBufAllocateCount[poolIndex]);
    printf("\n");

    printf("TxBufFreeCount           |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|",  TxBufFreeCount[poolIndex]);
    printf("\n");

    printf("CurrentTxBufCount        |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|",  CurrentTxBufCount[poolIndex]);
    printf("\n");

    printf("MaxUsedTxBufCount        |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|",  MaxUsedTxBufCount[poolIndex]);
    printf("\n");

    printf("AppFreeCount             |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|",  AppFreeCount[poolIndex]);
    printf("\n");

    printf("NetBufPoolExhaustedCount |");
    for (int poolIndex=0; poolIndex<BUF_POOL_NUM; poolIndex++)
        printf("%10u|",  NetBufPoolExhaustedCount[poolIndex]);
    printf("\n");

    printf("---------------------------------\n");
    printf("MaxPendingDescCount =  %u\n", MaxPendingDescCount);
    printf("CurrentPendingDescCount = %u\n",CurrentPendingDescCount);
    printf("TooLongComMsgCount = %u\n",  TooLongComMsgCount);
    printf("FreeNodeRunOutCount = %u\n", FreeNodeRunOutCount);   
    printf("TxFreeMsgCount = %u\n", TxFreeMsgCount);
    printf("DmaDescriptorRunOutCount = %u \n", DmaDescriptorRunOutCount);
    printf("TotalSentComMsgCount = %u\n", TotalSentComMsgCount);
    printf("TotalTxComMsgCount = %u\n", TotalTxComMsgCount);
    printf("InboundMsgPostFailureCount = %u\n", InboundMsgPostFailureCount);
    printf("OutboundMsgPostFailCount_LOW = %u\n", OutboundMsgPostFailCount_LOW);
    printf("OutboundMsgPostFailCount_HIGH = %u\n", OutboundMsgPostFailCount_HIGH);
    printf("TotalTxIpPktCount = %u\n",  TotalTxIpPktCount);
    printf("TotalSentIpPktCount = %u\n",TotalSentIpPktCount);
    printf("TxIpPktFormatErrorCount = %u\n", TxIpPktFormatErrorCount);
    printf("TooLongIPPacketCount = %u\n", TooLongIPPacketCount);
	printf("MaxInboundMsgInOneRound = %u\n",MaxInboundMsgInOneRound);
	printf("MaxOutboundFreeMsgInOneRound = %u \n",MaxOutboundFreeMsgInOneRound);
    printf("MaxContRunOutOfTxBufCount = %u \n", MaxContRunOutOfTxBufCount);
    int oldLvl = intLock();
    UINT32 postCount = HighQueueMsgPostCount;
    UINT32 procCount = HighQueueMsgProcessCount;
    intUnlock(oldLvl);
    printf("HighQueueMsgPostCount = %u\n",postCount);
    printf("HighQueueMsgProcessCount = %u\n", procCount);
    #ifdef M_TGT_L3
    printf("L2RunningState = %s\n", (L2RunningState == L2_SYSTEM_STATE_REBOOT)?"Rebooting":"Running");
    printf("L2BootUpCount = %u\n", L2BootUpCount);
    printf("ToL2ComMsgDiscardedCount = %u\n", ToL2ComMsgDiscardedCount);
    printf("ToL2IpPacketDiscardedCount = %u\n", ToL2IpPacketDiscardedCount);
    printf("L2 request reboot(IF)= %u\n", L2RequestForResetCount[0]);
    printf("L2 request reboot(CSI)= %u\n", L2RequestForResetCount[1]);
    printf("L2 request reboot(CMD)= %u\n", L2RequestForResetCount[2]);
    printf("LastL2BootUpTime [%04d/%02d/%02d %02d:%02d:%02d] \n", LastL2BootUpTime.year, LastL2BootUpTime.month, 
           LastL2BootUpTime.day, LastL2BootUpTime.hour, LastL2BootUpTime.minute, LastL2BootUpTime.second);
    #endif

    printf("RequestL2RebootCount = %u\n", RequestL2RebootCount);
    printf("\nMaxOutboundFreeSendCount = %u\n", MaxOutboundFreeSendCount);
    printf("MaxInboundSendCount = %u\n", MaxInboundSendCount);
    printf("MaxPoolQueueCount = %u\n\n",MaxPoolQueueCount);

    printf("ForceTxPostMsgTokenCount = %u\n",ForceTxPostMsgTokenCount) ;
    printf("EmptyInboundQueueCount= %u\n", EmptyInboundQueueCount);
    printf("MainLoopErrorCount = %d\n", MainLoopErrorCount);
    printf("\nPeerFreeInvalidBufCount = %d\n", PeerFreeInvalidBufCount);
    printf("PeerPostInvalidBufCount = %d\n", PeerPostInvalidBufCount);
    printf("FakeInboundPostIsrCount = %u\n", FakeInboundPostIsrCount);


    #ifdef DEBUG_PCI_ISR
    printf("MaxLoopInISR = %u\n",MaxLoopInISR);
    printf(" cause = " );
    for (int i=0; i<MaxLoopInISR; i++)
    {
        printf(" 0x%08x ",PciIsrCauseRecord[i]);
    }
    printf("\n mask  = ");
    for (int i=0; i<MaxLoopInISR; i++)
    {
        printf(" 0x%08x ",PciIsrMaskRecord[i]);
    }
    #endif
    
    oldLvl = intLock();
    UINT32 tmpPciIsrCount = PciIsrCount;
    UINT32 tmpInboundPostIsrCount = InboundPostIsrCount;
    UINT32 tmpInboundFreeTokenCount = InboundFreeTokenCount;
    UINT32 tmpISRBreakOutCount = ISRBreakOutCount;
    UINT32 tmpLastPostRound = LastPostRound;
    UINT32 tmpInboundProcessRound = InboundProcessRound;
    UINT32 tmpNotifyPeerInboundCount = NotifyPeerInboundCount;
    UINT32 tmpTxTokenReturnCount = TxTokenReturnCount;
    UINT32 tmpNotifyPeerOutboundFreeCount = NotifyPeerOutboundFreeCount;
    UINT32 tmpQueHead = InboundMsgCirQue.QueueHead;
    UINT32 tmpQueTail = InboundMsgCirQue.QueueTail;
    UINT32 tmpTotalPostMsgCount = TotalPostMsgCount;
    intUnlock(oldLvl);

    printf("\n");
    printf("PciIsrCount = %u,  InboundPostIsrCount = %u, InboundFreeTokenCount = %u, ISRBreakOutCount = %u\n", 
              tmpPciIsrCount, tmpInboundPostIsrCount, tmpInboundFreeTokenCount, tmpISRBreakOutCount);
    printf("LastPostRound = %u, InboundProcessRound = %u\n",tmpLastPostRound,tmpInboundProcessRound);
    printf("NotifyPeerInboundCount = %u\n", tmpNotifyPeerInboundCount);
    printf("TxTokenReturnCount = %u\n", tmpTxTokenReturnCount);
    printf("NotifyPeerOutboundFreeCount = %u\n", tmpNotifyPeerOutboundFreeCount);

    printf("HasTxPostMsgToken = %d\n",   HasTxPostMsgToken );
    printf("InboundMsgCirQue.QueueHead = %d, InboundMsgCirQue.QueueTail = %d\n",
           tmpQueHead, tmpQueTail);
    printf("tmpTotalPostMsgCount= %u, %u\n",tmpTotalPostMsgCount, (tmpTotalPostMsgCount&0xFFFF) );

#ifdef M_TGT_L3
    printf("StopDmaDescCount= %d\n", StopDmaDescCount);
#endif

    

    if ( verbose)
    {
        printf("----   DMA Channel Status ---------------------------\n");
        TxDmaChannel->ShowStats();

        printf("---------- Net Pool Show -----------------------\n");
        netPoolShow(NetBufPool.pNetPool);
        printf("netPoolId = 0x%x\n", NetBufPool.pNetPool);
    }
}



extern "C"
STATUS pcishow(int verbose)
{
#ifdef VXWORKS
    taskPrioritySet(0, 220);
#endif

    CTaskPciIf::GetInstance()->ShowStatus(verbose);
    return OK;
}


/*******************************************************************************
* mvI2oCircularQueueEnable - Initialize the I2O queue messaging mechanism.
*
* DESCRIPTION:
*       The queues must be initialized with there base address and size prior to
*       the use of this mechanism. This function sets the base address for 
*       queues (QueueBaseAddr?parameter) as shown in the table, sets the size
*       of each queue which is equal for all four queues (CirQueSize?
*       parameter) and set the head and tail location for all four queues, it 
*       also enables the queue mechanism by setting bit0 in the Queue Control 
*       Register. Note that accessing the queue's data is handled through the 
*       head and tail registers.
*       The starting address of each queue is based on the QBAR address and the 
*       size of the queues as shown in the following table:
*
*        The Circular Queue Starting Addresses as written in the spec:
*        ----------------------------------------
*        |    Queue       |  Starting Address   |
*        |----------------|---------------------|
*        | Inbound Free   |       QBAR          |
*        | Inbound Post   | QBAR + Queue Size   |
*        | Outbound Post  | QBAR + 2*Queue Size |
*        | Outbound Free  | QBAR + 3*Queue Size |
*        ----------------------------------------
*
* INPUT:
*       cirQueSize - Desired size for the queue .Can be one of the following 
*                    values (defined in mvI2o.h).
*       queueBaseAddr - Desired base address for the queues in the local SDRAM 
*                       or DEVICE.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
void  mvI2oCircularQueueEnable(I2O_CIRCULAR_QUEUE_SIZE cirQueSize,
                             UINT32 queueBaseAddr)
{
    UINT32 regData;

    regData = BIT0 | (cirQueSize << 1);
    /* Enable Queue Operation */
    MV64360_REG_WR(MV64360_I2O_Q_CTRL_CPU0,regData);
    /* Writing The base Address for the 4 Queues*/
    MV64360_REG_WR(MV64360_I2O_Q_BASE_ADRS_CPU0,queueBaseAddr);
    /* Update The Inbound Free Queue Base Address, offset=0 */
    MV64360_REG_WR(MV64360_I2O_IB_FREE_HEAD_PTR_CPU0,0);
    MV64360_REG_WR(MV64360_I2O_IB_FREE_TAIL_PTR_CPU0,0);
    /* Update The Inbound Post Queue Base Address, offset=_16K*cirQueSize */
    MV64360_REG_WR(MV64360_I2O_IB_POST_HEAD_PTR_CPU0, _16K * cirQueSize);
    MV64360_REG_WR(MV64360_I2O_IB_POST_TAIL_PTR_CPU0, _16K * cirQueSize);
    /* Update The Outbound Post Queue Base Address, offset=2*_16K*cirQueSize */
    MV64360_REG_WR(MV64360_I2O_OB_POST_HEAD_PTR_CPU0,2 * _16K * cirQueSize);
    MV64360_REG_WR(MV64360_I2O_OB_POST_TAIL_PTR_CPU0,2 * _16K * cirQueSize);
    /* Update The Outbound Free Queue Base Address, offset=3*_16K*cirQueSize */
    MV64360_REG_WR(MV64360_I2O_OB_FREE_HEAD_PTR_CPU0,3 * _16K * cirQueSize);
    MV64360_REG_WR(MV64360_I2O_OB_FREE_TAIL_PTR_CPU0,3 * _16K * cirQueSize);

    MV64360_REG_RD(MV64360_PCI0_PCI_ADRS_DECODE_CTRL, &regData);
    regData &= ~BIT3;
    MV64360_REG_WR(MV64360_PCI0_PCI_ADRS_DECODE_CTRL, regData);
    // clear bit 3, CS0 

}

UINT32 CTaskPciIf::GetRxBufAddr(UINT16 poolIndex, UINT16 bufIndex)
{
    #ifdef M_TGT_L3
    if (poolIndex < BUF_POOL_NUM && bufIndex<PciL2ToL3BufNum[poolIndex])
    #else
    if (poolIndex < BUF_POOL_NUM && bufIndex<PciL3ToL2BufNum[poolIndex])
    #endif
    {
        return RxBufStatusTable[poolIndex][bufIndex].BufferCpuAddress;
    }
    else
    {
        printf("Invalid buf index\n");
        return 0;
    }
}


UINT32 CTaskPciIf::GetTxBufAddr(UINT16 poolIndex, UINT16 bufIndex)
{
    #ifdef M_TGT_L3
    if (poolIndex < BUF_POOL_NUM && bufIndex<PciL3ToL2BufNum[poolIndex])
    #else
    if (poolIndex < BUF_POOL_NUM && bufIndex<PciL2ToL3BufNum[poolIndex])
    #endif
    {
        printf("Pci Buffer address[%d,%d] DMA address is 0x%x\n", poolIndex,
               bufIndex,PeerTxBufNode[poolIndex][bufIndex].BufferDmaAddress);
        printf("Pci Buffer address[%d,%d] CPU address is 0x%x\n", poolIndex,
               bufIndex,PeerTxBufNode[poolIndex][bufIndex].BufferCpuAddress);
        return PeerTxBufNode[poolIndex][bufIndex].BufferDmaAddress;
    }
    else
    {
        printf("Invalid buf index\n");
        return 0;
    }
}

void CTaskPciIf::SearchForBusyTxBuf()
{
    for (int poolIndex = 0; poolIndex< BUF_POOL_NUM; poolIndex++)
    {
        #ifdef M_TGT_L3
        for ( int bufIndex=0; bufIndex<PciL3ToL2BufNum[poolIndex]; bufIndex++)
        #else
        for ( int bufIndex=0; bufIndex<PciL2ToL3BufNum[poolIndex]; bufIndex++)
        #endif
        {
            if (OCCUPIED == PeerTxBufNode[poolIndex][bufIndex].BufferStatus)
            {
                printf("Pool %d Buffer %d, address 0x%x is busy\n", 
                        poolIndex, bufIndex, PeerTxBufNode[poolIndex][bufIndex].BufferDmaAddress);
            }
        }
    }
}


extern "C" void StartL3If(void)
{
   CTaskPciIf *pciIf = CTaskPciIf::GetInstance();
   pciIf->Begin();
}


extern "C" int displayPciMsg(UINT32 flag)
{
    PCI_MSG msg;
#ifdef VXWORKS
    taskPrioritySet(0, 220);
#endif

    msg.Val = flag;
    UINT32 addr = CTaskPciIf::GetInstance()->GetRxBufAddr(msg.Msg.PoolIndex, msg.Msg.BufferIndex);
    printf("Pci Buffer address for pool %d, buf %d is 0x%x\n", msg.Msg.PoolIndex, msg.Msg.BufferIndex,addr);
    return 0;
}


extern "C" int pciBufAddr(UINT16 pool, UINT16 bufIndex)
{
    UINT32 addr = CTaskPciIf::GetInstance()->GetTxBufAddr(pool, bufIndex);
//    printf("Pci Buffer address for pool %d, buf %d is 0x%x\n", pool,bufIndex,addr);
    return 0;
}

extern "C" STATUS findBusyPciBuf()
{
    CTaskPciIf::GetInstance()->SearchForBusyTxBuf();
}


#ifdef DEBUG_PCI_IF
extern "C" STATUS showpcifail()
{
    if ( false == InboundFailBufFilled)
    {
        printf(" no error yet\n");
        return OK;
    }
    printf("inboundFailSnapshot.QueHead = %d, inboundFailSnapshot.QueTail = %d\n",
           inboundFailSnapshot.QueueHead, inboundFailSnapshot.QueueTail);
    for (int i=0; i<0x10000; i++)
    {
        printf("(%d,%d,%d) ", inboundFailSnapshot.QueueEntry[i].pciMsg.Msg.PoolIndex,
            inboundFailSnapshot.QueueEntry[i].pciMsg.Msg.BufferIndex,
               inboundFailSnapshot.QueueEntry[i].postRound);
        if (0 == ((i+1) % 5))
        {
            printf("\n");
        }
    }
    InboundFailBufFilled = false;
    return OK;
}
#endif


#ifdef M_TGT_L3

extern "C"
STATUS pciL2TimeShow()
{
    printf("Time slot        0       1       2       3       4       5       6       7     >=8\n");
    printf("           ");
    UINT32 tmpSnapshot[9];
    UINT32 tmpSnapshotIsr[9];
    int oldLvl = intLock();
    memcpy(tmpSnapshotIsr, L2TxTimingCountIsr, sizeof(L2TxTimingCountIsr));
    intUnlock(oldLvl);

    for (int i=0; i<9; i++)
    {
        printf("%8d",tmpSnapshotIsr[i]);
    }
    printf("\n");
    return OK;
}


extern "C"
STATUS pciL2TimeClear()
{
    int oldLvl = intLock();
    memset(L2TxTimingCountIsr, 0, sizeof(L2TxTimingCountIsr));
    intUnlock(oldLvl);
    return OK;
}

#else

CTaskPciIf* CTaskPciIf::GetPciIfObj()
{
    return Instance;
}
// this routine is called in GPIO ISR in L1TddIf to post timing info to L3
void NotifyL3TxTiming(UINT32 msg)
{
    CTaskPciIf* taskObj = CTaskPciIf::GetPciIfObj();

    if ( taskObj )
    {
        taskObj->SendL2TimingEventToL3(msg);
    }
}

UINT32 L2TxTimingBufferCount[9]={0};
void CTaskPciIf::SendL2TimingEventToL3(UINT32 msg)
{
    if (L2TimingEventCCB.HasPostToken)
    {
        L2TimingEventCCB.HasPostToken = false;
        L2TimingEventCCB.TokenLostTick = tickGet();

        //notify L3 about L1 Tdd Tx timing by sending a inbound message
        *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_MSG1_CPU0) = msg;
        asm volatile ("eieio ");

        *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_PCI0_SYNC_BARR_TRGR_REG) = 1;
        asm volatile ("eieio ");

        //UINT16 msgId= msg>>16;
        UINT8 ts = msg&0xff;

        if (ts < 9)
        {
            L2TxTimingCountIsr[ts]++;
        }

    }
    else
    {   // 
        L2TimingEventCCB.EventBuf[L2TimingEventCCB.UnsentTailIndex++] = msg;
        //UINT16 msgId= msg>>16;
        UINT8 ts = msg&0xff;

        if (ts < 9)
        {
            L2TxTimingBufferCount[ts]++;
        }
    }
}

UINT32 L2TimingTokenReclaimCount=0;
void CTaskPciIf::L2TimingTokenReclaim()
{

    L2TimingTokenReclaimCount ++;
    if (L2TimingEventCCB.UnsentTailIndex != L2TimingEventCCB.UnsentHeadIndex)
    {
        UINT32 msg = L2TimingEventCCB.EventBuf[L2TimingEventCCB.UnsentHeadIndex];
        L2TimingEventCCB.UnsentHeadIndex ++;

        //notify L3 about L1 Tdd Tx timing by sending a inbound message
        //*(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_MSG1_CPU0) = LONGSWAP (msg);
        *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_I2O_IB_MSG1_CPU0) = msg;
        *(volatile UINT32*)(PCI0_MEM0_BASE | MV64360_PCI0_SYNC_BARR_TRGR_REG) = 1;
        asm volatile ("eieio ");

        L2TimingEventCCB.HasPostToken = false;
        L2TimingEventCCB.TokenLostTick = tickGet();
    }
    else
    {
        L2TimingEventCCB.HasPostToken = true;
    }

}


STATUS pciL2TimeShow()
{
    printf("Time slot        0       1       2       3       4       5       6       7     >=8\n");
    UINT32 tmpSnapshot[9];
    UINT32 tmpSnapshotIsr[9];
    int oldLvl = intLock();
    memcpy(tmpSnapshotIsr, L2TxTimingCountIsr, sizeof(L2TxTimingCountIsr));
    memcpy(tmpSnapshot, L2TxTimingBufferCount, sizeof(L2TxTimingBufferCount));
    intUnlock(oldLvl);

    printf(" ISR       ");
    for (int i=0; i<9; i++)
    {
        printf("%8d",tmpSnapshotIsr[i]);
    }
    printf("\n");
    printf(" Buffered ");
    for (int i=0; i<9; i++)
    {
        printf("%8d",tmpSnapshot[i]);
    }
    printf("\n");
    printf("L2TimingTokenReclaimCount= %u\n",L2TimingTokenReclaimCount);
    printf("ForceL2TimingTokenCount = %u\n", ForceL2TimingTokenCount);
    return OK;
}

extern "C"
STATUS pciL2TimeClear()
{
    int oldLvl = intLock();
    memset(L2TxTimingCountIsr, 0, sizeof(L2TxTimingCountIsr));
    memset(L2TxTimingBufferCount, 0, sizeof(L2TxTimingBufferCount));
    L2TimingTokenReclaimCount = 0;
    ForceL2TimingTokenCount =0;
    intUnlock(oldLvl);
    return OK;
}

#if 0
extern "C" 
STATUS disableInboundInt()
{
    UINT32 intMask;
    MV64360_REG_RD(MV64360_I2O_IB_INT_MASK_CPU0, &intMask);
    intMask |= BIT0;
    MV64360_REG_WR(MV64360_I2O_IB_INT_MASK_CPU0, intMask);
}
#endif
#endif




////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
#if 0
PciTestStub* PciTestStub::instance = NULL;

PciTestStub::PciTestStub()
{
    strcpy(m_szName, "tPciTest");
    m_uPriority   = 115;
    m_uOptions    = 0;
    m_uStackSize  = 1024 * 10;
    m_iMsgQMax       = 1000;
    m_iMsgQOption    = MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY ;
    testMsg = NULL;
}

TID PciTestStub::GetEntityId() const 
{
    #ifdef M_TGT_L2
    return M_TID_RSV2;
    #else
    return M_TID_RSV3;
    #endif
}

bool PciTestStub::Initialize ()
{
    testMsg = new (this, 10000)CComMessage;
    for (int i=0; i<10000; i++)
    {
       *((UINT8*)(testMsg->GetDataPtr())+i) = i;
    }
    #ifdef M_TGT_L3
    testMsg->SetDstTid(M_TID_RSV2);
    testMsg->SetSrcTid(M_TID_RSV3);
    #else
    testMsg->SetDstTid(M_TID_RSV3);
    testMsg->SetSrcTid(M_TID_RSV2);
    #endif
    testMsg->SetMessageId(0x1234);
    testMsg->AddRef();
    printf("PCI test Stub task started\n");

    if ( !CBizTask::Initialize() )
    {
        return false;
    }

    return true;
}

 
extern "C" UINT32 sysReadExTimer();
extern "C" UINT32 sysTimestampFreq();
extern "C" void sysExeTimerEnable();

PciTestStub* PciTestStub::GetInstance()
{
    if ( instance == NULL)
    {
        sysExeTimerEnable();
        instance = new PciTestStub;
        instance->Begin();
    }
    return instance;
}

UINT32 sysPpcGetUSCounter()
{
	return sysReadExTimer()/(sysTimestampFreq()/1000000);
}


UINT32 TxMsgCount=0;
UINT32 PciTestRound=0;
UINT32 TestBeginUsRead=0;
UINT32 TestEndUsRead=0;
bool PciTestStub::ProcessComMessage(CComMessage* pComMsg)
{
    UINT16 usMsgId = pComMsg->GetMessageId();

    #ifdef M_TGT_L3
    switch ( usMsgId )
    {
        case 0xFFFF:
            printf("A new round of test started\n");
            TestBeginUsRead = sysPpcGetUSCounter();

            CComEntity::PostEntityMessage(testMsg);
            TxMsgCount=1;
            PciTestRound ++;
            break;
        case 0x1234:  // test msg ack from L2
            if (TxMsgCount<10000)
            {
                //printf("Pci test got ack from L2  TxMsgCount=%d\n",TxMsgCount);
                CComEntity::PostEntityMessage(testMsg);
                TxMsgCount ++;
            }
            else
            {
                TestEndUsRead = sysPpcGetUSCounter();
                printf("PCI test finished, 10000 packet need %d us to transfer, \n",TestEndUsRead-TestBeginUsRead);
            }
            break;
    }
    #else
    //printf("Got a pcitest message from L3\n");
    CComEntity::PostEntityMessage(testMsg);
    #endif
    pComMsg->Destroy();
}

#ifdef M_TGT_L3
void PciTestStub::SendPciTestMsg()
{
    CComMessage *comMsg = new (this,0)CComMessage;
    comMsg->SetDstTid(M_TID_RSV3);
    comMsg->SetSrcTid(M_TID_RSV3);
    comMsg->SetMessageId(0xffff);
    
    ::CComEntity::PostEntityMessage(comMsg);
}

extern "C"
STATUS pcitest()
{
    PciTestStub::GetInstance()->SendPciTestMsg();
    return OK;
}
#else
extern "C"
STATUS pcitest()
{
    PciTestStub *testObj = PciTestStub::GetInstance();
    return OK;
}
#endif

#endif


extern "C" 
STATUS pcitest()
{
    CComMessage *pComMsg = new (CTaskPciIf::GetInstance(),10) CComMessage;
    pComMsg->SetDstTid(M_TID_L2MAIN);
    pComMsg->SetSrcTid(M_TID_L2IF);
    pComMsg->SetFlag(0xffffffff);
    pComMsg->SetMessageId(0);
    if ( false == CComEntity::PostEntityMessage(pComMsg,NO_WAIT,true))
    {
        pComMsg->Destroy();
    }
}

