#include <msgQLib.h>
#include <string.h>
#include <stdio.h>
#include "CsiTest.h"

CsiTest1* CsiTest1::instance = NULL;
CsiTest2* CsiTest2::instance = NULL;

SEM_ID  CsiTestSemId= NULL;

CsiTest1::CsiTest1()
{
    strcpy(m_szName, "tCsiTest1");
    m_uPriority   = 115;
    m_uOptions    = 0;
    m_uStackSize  = 4096;
    m_iMsgQMax       = 10;
    m_iMsgQOption    = MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY ;
}

TID CsiTest1::GetEntityId() const 
{
    return M_TID_RSV1;
}
 
CsiTest1* CsiTest1::GetInstance()
{
    if ( instance == NULL)
    {
        instance = new CsiTest1;
    }
    return instance;
}

void DataException()
{
    *((unsigned int *)0x1c803000) = 0xffffffff;
}


void StackOverflow()
{
    UINT8 tmp[3000];
    memset(tmp, 0x55, 2000);
}

void Deadloop()
{
    for (;;);
}

void Deadlock()
{
    semTake(CsiTestSemId, WAIT_FOREVER);
}

bool CsiTest1::ProcessComMessage(CComMessage* pComMsg)
{

    printf("CsiTest1 Received test case %d\n", pComMsg->GetMessageId());

    switch (pComMsg->GetMessageId())
    {
        case 1:
            DataException();
            break;
        case 2:
            StackOverflow();
            break;
        case 3:
            Deadloop();
            break;
        case 4:
            Deadlock();
            break;
        default:
            printf("Unknown CSI test case\n");
            break;
    }
}

void CsiTest1::SendCsiTestMsg(UINT32 value)
{
    CComMessage *comMsg = new (this, 0)CComMessage;
    comMsg->SetDstTid(M_TID_RSV1);
    comMsg->SetSrcTid(M_TID_RSV1);
    comMsg->SetMessageId(value);
    
    ::CComEntity::PostEntityMessage(comMsg);
}

extern "C" 
int csitest1(int testCase)
{
    CsiTest1::GetInstance()->SendCsiTestMsg(testCase);
    return OK;
}

CsiTest2::CsiTest2()
{
    strcpy(m_szName, "tCsiTest2");
    m_uPriority   = 115;
    m_uOptions    = 0;
    m_uStackSize  = 4096;
    m_iMsgQMax       = 10;
    m_iMsgQOption    = MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY ;
}

TID CsiTest2::GetEntityId() const 
{
    return M_TID_RSV2;
}
 
CsiTest2* CsiTest2::GetInstance()
{
    if ( instance == NULL)
    {
        instance = new CsiTest2;
    }
    return instance;
}

bool CsiTest2::ProcessComMessage(CComMessage* pComMsg)
{

    printf("CsiTest2 Received test case %d\n", pComMsg->GetMessageId());
    switch (pComMsg->GetMessageId())
    {
        case 1:
            DataException();
            break;
        case 2:
            StackOverflow();
            break;
        case 3:
            Deadloop();
            break;
        case 4:
            Deadlock();
            break;
        default:
            printf("Unknown CSI test case\n");
            break;
    }
}

void CsiTest2::SendCsiTestMsg(UINT32 value)
{
    CComMessage *comMsg = new (this, 0)CComMessage;
    comMsg->SetDstTid(M_TID_RSV2);
    comMsg->SetSrcTid(M_TID_RSV2);
    comMsg->SetMessageId(value);
    
    ::CComEntity::PostEntityMessage(comMsg);
}

extern "C" 
int csitest2(int testCase)
{
    CsiTest2::GetInstance()->SendCsiTestMsg(testCase);
    return OK;
}

extern STATUS csiInit(void);

int startcsitest()
{
    csiInit();

    CsiTest1 *testObj1 = CsiTest1::GetInstance();
    testObj1->Begin();

    CsiTest2 *testObj2 = CsiTest2::GetInstance();
    testObj2->Begin();

    CsiTestSemId = semCCreate(SEM_Q_FIFO, 1);
    printf("Csi Test Init Finished\n");
    return OK;
}

