#include <msgQLib.h>
#include <string.h>
#include <stdio.h>
#include "PcitestStub.h"
#include "log.h"

PciTestStub* PciTestStub::instance = NULL;

PciTestStub::PciTestStub()
{
    strcpy(m_szName, "tPciTest");
    m_uPriority   = 115;
    m_uOptions    = 0;
    m_uStackSize  = 1024 * 10;
    m_iMsgQMax       = 1000;
    m_iMsgQOption    = MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY ;
}

TID PciTestStub::GetEntityId() const 
{
    #ifdef M_TGT_L2
    return M_TID_L2MAIN;
    #else
    return M_TID_CM;
    #endif
}


 
PciTestStub* PciTestStub::GetInstance()
{
    if ( instance == NULL)
    {
        instance = new PciTestStub;
    }
    return instance;
}



UINT32 RxMsgCount = 0;
bool PciTestStub::ProcessMessage(CMessage &msg)
{
    RxMsgCount ++;
    if ( !(RxMsgCount & 0xFFF))
    {
        printf("pci test task got 4K message\n");
    }
}

void PciTestStub::SendPciTestMsg(UINT32 len)
{
    CComMessage *comMsg = new (this, len)CComMessage;
    for (int i=0; i<len; i++)
    {
       *((UINT8*)(comMsg->GetDataPtr())+i) = i;
    }
    #ifdef M_TGT_L2
    comMsg->SetDstTid(M_TID_CM);
    comMsg->SetSrcTid(M_TID_L2MAIN);
    #else
    comMsg->SetDstTid(M_TID_L2MAIN);
    comMsg->SetSrcTid(M_TID_CM);
    #endif
    comMsg->SetMessageId(0x1234);
    
    ::CComEntity::PostEntityMessage(comMsg);
}


extern "C"
STATUS pcisend(UINT32 len=16)
{
    PciTestStub::GetInstance()->SendPciTestMsg(len);
    return OK;
}


extern "C"
STATUS pcisendm(UINT32 len, UINT32 num)
{
    for (int i=0; i<num; i++)
    {
        PciTestStub::GetInstance()->SendPciTestMsg(len);
    }
    return OK;
}
#include "taskLib.h"
STATUS pcisendk(int len, int delay, int burst)
{
    for (;;)
    {
        pcisendm(len, burst);
        taskDelay(delay);
    }
    return OK;
}

extern "C" 
STATUS pcis(int len, int delay, int burst)
{
    if (delay == 0)
    {
        delay = 10;
    }
    if (len == 0)
    {
        len = 16;
    }
    if (burst == 0)
    {
        burst = 2;
    }
    taskSpawn ("pciTest", 50, 0, 2000, (FUNCPTR)pcisendk, len,delay,burst,0,0,0,0,0,0,0);
}

#include <taskLib.h>

#include "sysDma.h"
#include "AfwInit.h"
#include "PciIf.h"
#include "utAgent.h"

void pcimain()
{
    AfwInit();
    CTaskPciIf *pciIf = CTaskPciIf::GetInstance();
    CUtAgent  *agent = CUtAgent::GetInstance();

    pciIf->Begin();
}

extern "C"
void pcitest()
{
    taskSpawn ("tPciApp", 50, 0, 5120, (FUNCPTR)pcimain, 0,0,0,0,0,0,0,0,0,0);
}


bool InitL3DataSvc()
{    
    return true;
}

bool InitL3VoiceSvc()
{   
    return true;
}


bool InitL3Oam(void)
{
    return true;
}



#define BufLen (1024)
UINT8 srcBuf[BufLen];
UINT16 bufLen = BufLen;
DmaChannel *channel=NULL;
SYS_DMA_DESC  *dmaDesc0;
SEM_ID dmaTestSem= NULL;

#include <logLib.h>
#include <cacheLib.h>
extern "C"
void dmaCallBk(UINT32 arg1, UINT32 arg2)
{
    logMsg("Dst=%x, src=%x DMA finished\n",arg2, arg1,0,0,0,0);
    semGive(dmaTestSem);
}


extern "C"
STATUS dmaInit()
{
    // allocate a dma channel for DRAM<->DRAM dma
//    channel = sysDmaChannelAlloc(DMA_WINDOW_SRAM, 10);
    channel = sysDmaChannelAlloc(DMA_WINDOW_PCI, 10);
    dmaTestSem = semCCreate(SEM_Q_FIFO, SEM_EMPTY);
}

UINT8* destAddr = (UINT8*)0x0F000000;
UINT8 *srcAddr = (UINT8*)0x0D000000;

STATUS dmaWTest()
{
    static UINT8 seq=1;
    if (channel==NULL)
    {
        dmaInit();
    }
    for (int i=0; i<bufLen;i++)
    {
        srcAddr[i] = seq+ i;
    }

    STATUS rc = cacheInvalidate(DATA_CACHE, srcAddr, bufLen);
    if (rc!=OK )//|| rc1!=OK)
    {
        printf("cacheinvalidate failed\n");
    }

    
    dmaDesc0 = channel->CreateDesc(destAddr, (UINT8 *)srcAddr,bufLen, dmaCallBk, (UINT32)srcAddr,(UINT32)destAddr);
    printf("seq = %d, srcBuf=%x, destAddr= %x dmaDesc=%x\n", seq, (UINT32)srcAddr, destAddr, (UINT32)dmaDesc0);
    seq++;
    seq&=0xf;
    if ( NULL != dmaDesc0 )
    {
        channel->StartWithDesc(dmaDesc0);
    }
    semTake(dmaTestSem, WAIT_FOREVER);
    printf("DMA finished \n", seq, (UINT32)srcAddr, destAddr, (UINT32)dmaDesc0);
}


