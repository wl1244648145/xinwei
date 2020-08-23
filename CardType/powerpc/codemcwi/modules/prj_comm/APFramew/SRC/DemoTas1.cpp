#ifndef _INC_DEMOTASK2
#include "DemoTask2.h"
#endif

#include "ComMessage.h"
#include "MsgQueue.h"

CDemoTask2::CDemoTask2()
{
    ::strcpy(m_szName, "tDemo2");
    m_uPriority = 100;
    m_uOptions = 0;
    m_uStackSize = 2000;
    m_iMsgQMax = 100;
}

TID CDemoTask2::GetEntityId() const
{
    return M_TID_L2OAM;
}

bool CDemoTask2::PostMessage(CComMessage* pComMsg, SINT32 timeout)
{
    if (!ASSERT_VALID(this))
        return false;
    if (!ASSERT_VALID(pComMsg))
        return false;
    return m_pMsgQueue->PostMessage(pComMsg, timeout);
}

bool CDemoTask2::DeallocateComMessage(CComMessage* pComMsg)
{
    if (!ASSERT_VALID(pComMsg))
        return false;
    if (pComMsg->GetFlag() & 0x00000010)
    {
        char* pBuf =(char*) (pComMsg->GetBufferPtr());
        pComMsg->DeleteBuffer();
        delete [] pBuf;
    }
    return CComEntity::DeallocateComMessage(pComMsg);
}

bool CDemoTask2::Initialize()
{
    if (!CBizTask::Initialize())
        return false;
    return true;
}

void CDemoTask2::MainLoop()
{
    CComMessage* pComMsg = new (this, 0) CComMessage;
    if (pComMsg!=NULL)
    {
        pComMsg->SetDstTid(M_TID_DAC);
        pComMsg->SetSrcTid(GetEntityId());
        CComEntity::PostEntityMessage(pComMsg);
    }
    CBizTask::MainLoop();
}

bool CDemoTask2::ProcessComMessage(CComMessage* pComMsg)
{
    pComMsg->Destroy();
    ::sleep(1);
    pComMsg = new (this, 0) CComMessage;
    if (pComMsg!=NULL)
    {
        char* pBuf = new char[M_DEFAULT_RESERVED+100];
        if (pBuf!=NULL)
        {
            ::memset(pBuf, 0, M_DEFAULT_RESERVED+100);
            pComMsg->SetBuffer(pBuf, M_DEFAULT_RESERVED+100);
            pComMsg->SetDataPtr(pBuf+M_DEFAULT_RESERVED);
            pComMsg->SetDataLength(100);
            pComMsg->SetFlag(pComMsg->GetFlag() | 0x00000010);
        }
        char* pData = (char*)pComMsg->GetDataPtr();
        ::memset(pData, '2', 100);
        pComMsg->SetDstTid(M_TID_DAC);
        pComMsg->SetSrcTid(M_TID_L2OAM);
        if (!CComEntity::PostEntityMessage(pComMsg))
            pComMsg->Destroy();
    }
    return true;
}


