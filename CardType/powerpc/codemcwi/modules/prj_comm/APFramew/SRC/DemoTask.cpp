#ifndef _INC_DEMOTASK1
#include "DemoTask1.h"
#endif

#include "ComMessage.h"
#include "MsgQueue.h"

CDemoTask1::CDemoTask1()
{
    ::strcpy(m_szName, "tDemo1");
    m_uPriority = 100;
    m_uOptions = 0;
    m_uStackSize = 2000;
    m_iMsgQMax = 100;
    m_tid = M_TID_CM;
}

TID CDemoTask1::GetEntityId() const
{
    return m_tid;
}

bool CDemoTask1::PostMessage(CComMessage* pComMsg, SINT32 timeout)
{
    if (!ASSERT_VALID(this))
        return false;
    if (!ASSERT_VALID(pComMsg))
        return false;
    return m_pMsgQueue->PostMessage(pComMsg, timeout);
}

bool CDemoTask1::DeallocateComMessage(CComMessage* pComMsg)
{
    if (!ASSERT_VALID(pComMsg))
        return false;
    if (pComMsg->GetFlag() & 0x00000001)
    {
        char* pBuf =(char*) (pComMsg->GetBufferPtr());
        pComMsg->DeleteBuffer();
        delete [] pBuf;
    }
    return CComEntity::DeallocateComMessage(pComMsg);
}

bool CDemoTask1::Initialize()
{
    if (!CBizTask::Initialize())
        return false;
    m_tid = M_TID_CM;
    RegisterEntity(false);
    m_tid = M_TID_PM;
    RegisterEntity(false);
    m_tid = M_TID_FM;
    RegisterEntity(false);
    m_tid = M_TID_DAC;
    RegisterEntity(false);
    m_tid = M_TID_VAC;
    RegisterEntity(false);
    return true;
}

bool CDemoTask1::ProcessComMessage(CComMessage* pComMsg)
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
            pComMsg->SetFlag(pComMsg->GetFlag() | 0x00000001);
        }
        char* pData = (char*)pComMsg->GetDataPtr();
        ::memset(pData, '1', 100);
        pComMsg->SetDstTid(M_TID_L2OAM);
        pComMsg->SetSrcTid(M_TID_DAC);
        if (!CComEntity::PostEntityMessage(pComMsg))
            pComMsg->Destroy();
    }
    return true;
}


