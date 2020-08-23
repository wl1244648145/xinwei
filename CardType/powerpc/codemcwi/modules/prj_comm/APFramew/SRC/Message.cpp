#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef _INC_LOG
#include "Log.h"
#endif

CMessage::CMessage(CComMessage* pMsg)
:m_pMsg(pMsg)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::CMessage(CComMessage*)");

    if (!Construct(M_OID_MESSAGE))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif
}

CMessage::CMessage()
:m_pMsg(NULL)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::CMessage()");

    if (!Construct(M_OID_MESSAGE))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif
}

CMessage::~CMessage()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::~CMessage");

    if (!Destruct(M_OID_MESSAGE))
        LOG(LOG_SEVERE,0,"Destruct failed.");
#endif
}

#if 0
UINT16 CMessage::GetMessageId() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetMessageId");

    if (!ASSERT_VALID(this))
        return M_INVALID_MESSAGE_ID;
#endif

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return M_INVALID_MESSAGE_ID;
    }
    return m_pMsg->GetMessageId();
}

UINT16 CMessage::SetMessageId(UINT16 mid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetMessageId");

    if (!ASSERT_VALID(this))
        return M_INVALID_MESSAGE_ID;
#endif
    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return M_INVALID_MESSAGE_ID;
    }

    return m_pMsg->SetMessageId(mid);
}

TID CMessage::GetDstTid() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetDstTid");

    if (!ASSERT_VALID(this))
        return M_TID_MAX;
#endif

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return M_TID_MAX;
    }

    return m_pMsg->GetDstTid();
}

TID CMessage::SetDstTid(TID tid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetDstTid");

    if (!ASSERT_VALID(this))
        return M_TID_MAX;
#endif

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return M_TID_MAX;
    }

    return m_pMsg->SetDstTid(tid);
}

TID CMessage::GetSrcTid() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetSrcTid");

    if (!ASSERT_VALID(this))
        return M_TID_MAX;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return M_TID_MAX;
    }
#endif

    return m_pMsg->GetSrcTid();
}

TID CMessage::SetSrcTid(TID tid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetSrcTid");

    if (!ASSERT_VALID(this))
        return M_TID_MAX;
#endif

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return M_TID_MAX;
    }

    return m_pMsg->SetSrcTid(tid);
}

#if !(M_TARGET==M_TGT_CPE)
UINT32 CMessage::GetEID() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetEID");

    if (!ASSERT_VALID(this))
        return 0;
#endif

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return 0;
    }

    return m_pMsg->GetEID();
}

UINT32 CMessage::SetEID(UINT32 uEID)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetEID");

    if (!ASSERT_VALID(this))
        return ~uEID;
#endif

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return ~uEID;
    }

    return m_pMsg->SetEID(uEID);
}
#endif


UINT16 CMessage::GetUID() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetUID");

    if (!ASSERT_VALID(this))
        return 0;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return 0;
    }
#endif

    return m_pMsg->GetUID();
}

UINT16 CMessage::SetUID(UINT16 uUID)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetUID");

    if (!ASSERT_VALID(this))
        return ~uUID;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return ~uUID;
    }
#endif

    return m_pMsg->SetUID(uUID);
}
#endif

/*
SFID CMessage::GetSFID() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetSFID");
#endif
    if (!ASSERT_VALID(this))
        return M_SFID_INVALID;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return M_SFID_INVALID;
    }

    return m_pMsg->GetSFID();
}

SFID CMessage::SetSFID(SFID sfidNew)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetSFID");
#endif
    if (!ASSERT_VALID(this) || sfidNew==M_SFID_INVALID)
        return M_SFID_INVALID;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return M_SFID_INVALID;
    }

    return m_pMsg->SetSFID(sfidNew);
}
*/

// xiao weifang add {
#if !(M_TARGET==M_TGT_CPE)
UINT8 CMessage::SetDirection(UINT8 ucDir)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetDirection");

    if (!ASSERT_VALID(this))
        return (~ucDir);
    
    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return (~ucDir);
    }
#endif

    return m_pMsg->SetDirection( ucDir );
}

UINT8 CMessage::GetDirection() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetDirection");
    if (!ASSERT_VALID(this))
        return (~0);

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return (~0);
    }
#endif

    return m_pMsg->GetDirection();
}

#if 0
UINT8 CMessage::SetIpType(UINT8 ucIpType)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetIpType");

    if (!ASSERT_VALID(this))
        return (~ucIpType);
    
    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return (~ucIpType);
    }
#endif

    return m_pMsg->SetIpType( ucIpType );
}
#endif

UINT8 CMessage::GetIpType() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetIpType");

    if (!ASSERT_VALID(this))
        return (~0);

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return (~0);
    }
#endif

    return m_pMsg->GetIpType();
}

//Add by yanghuawei 
UINT32 CMessage::SetBTSPubIP(UINT32 ulBtsIP)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetBtsAddr");

    if (!ASSERT_VALID(this))
        return ~ulBtsIP;
    
    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return (~ulBtsIP);
    }
#endif

    return m_pMsg->SetBtsAddr( ulBtsIP );
}

UINT32 CMessage::GetBTSPubIP() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetBTS");

    if (!ASSERT_VALID(this))
        return 0;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return 0;
    }
#endif

    return m_pMsg->GetBtsAddr();
}

UINT16 CMessage::SetBTSPubPort(UINT16 usBtsPort)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetBtsAddr");

    if (!ASSERT_VALID(this))
        return ~usBtsPort;
    
    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return (~usBtsPort);
    }
#endif

    return m_pMsg->SetBtsPort( usBtsPort );
}

UINT16 CMessage::GetBTSPubPort() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetBTS");

    if (!ASSERT_VALID(this))
        return 0;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return 0;
    }
#endif

    return m_pMsg->GetBtsPort();
}

#if 0
void* CMessage::SetUdpPtr(void *pUdp)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetUdpPtr");

    if (!ASSERT_VALID(this))
        return NULL;

    if (pUdp==NULL)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter DhcpPtr.");
        return NULL;
    }

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return NULL;
    }
#endif

    return m_pMsg->SetUdpPtr( pUdp );
}
#endif

void* CMessage::GetUdpPtr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetUdpPtr");

    if (!ASSERT_VALID(this))
        return NULL;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return NULL;
    }
#endif

    return m_pMsg->GetUdpPtr();
}

#if 0
void* CMessage::SetDhcpPtr(void* pDhcpPtr)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetDhcpPtr");

    if (!ASSERT_VALID(this))
        return NULL;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return NULL;
    }

    if (pDhcpPtr==NULL)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter DhcpPtr.");
        return NULL;
    }
#endif

    return m_pMsg->SetDhcpPtr( pDhcpPtr );
}
#endif
void* CMessage::GetDhcpPtr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetDhcpPtr");

    if (!ASSERT_VALID(this))
        return NULL;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return NULL;
    }
#endif

    return m_pMsg->GetDhcpPtr();
}

void CMessage::SetKeyMac(UINT8 *pMac)
{
#ifndef NDEBUG
    LOG( LOG_DEBUG3, 0, "CMessage::SetKeyMac" );

    if ( NULL == pMac )
    {
        LOG(LOG_SEVERE,0,"Invalid parameter pMac.");
        return ;
    }

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return ;
    }
#endif

    m_pMsg->SetKeyMac( pMac );
}

UINT8* CMessage::GetKeyMac() const
{
#ifndef NDEBUG
    LOG( LOG_DEBUG3, 0, "CMessage::GetKeyMac" );

    if ( !ASSERT_VALID( this ) )
        return NULL;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return NULL;
    }
#endif

    return m_pMsg->GetKeyMac();
}

UINT32 CMessage::SetBtsAddr(UINT32 ulBtsIp)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetBtsAddr");

    if (!ASSERT_VALID(this))
        return ~ulBtsIp;
#endif

    return m_pMsg->SetBtsAddr(ulBtsIp);
}

UINT32 CMessage::GetBtsAddr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetBtsAddr");
    if (!ASSERT_VALID(this))
        return 0;
#endif
    return m_pMsg->GetBtsAddr();
}

UINT16 CMessage::SetBtsPort(UINT16 ulBtsPort)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetBtsPort");

    if (!ASSERT_VALID(this))
        return ~ulBtsPort;
#endif

    return m_pMsg->SetBtsPort(ulBtsPort);
}

UINT16 CMessage::GetBtsPort() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetBtsPort");
    if (!ASSERT_VALID(this))
        return 0;
#endif
    return m_pMsg->GetBtsPort();
}

#endif


UINT32 CMessage::GetBTS() const
{
#ifndef NDEBUG
    LOG( LOG_DEBUG3, 0, "CMessage::GetBTS" );

    if ( !ASSERT_VALID( this ) )
        return 0xFFFFFFFF;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return 0xFFFFFFFF;
    }
#endif

    return m_pMsg->GetBTS();
}

UINT32 CMessage::SetBTS(UINT32 ulBtsId)
{
#ifndef NDEBUG
    LOG( LOG_DEBUG3, 0, "CMessage::SetBTS" );

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return NULL;
    }
#endif

    return m_pMsg->SetBTS( ulBtsId );
}

// xiao weifang add }
#if 0
void* CMessage::GetDataPtr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetDataPtr");

    if (!ASSERT_VALID(this))
        return NULL;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return NULL;
    }
#endif

    return m_pMsg->GetDataPtr();
}

UINT32 CMessage::GetDataLength() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetDataLength");

    if (!ASSERT_VALID(this))
        return 0;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return 0;
    }
#endif

    return m_pMsg->GetDataLength();
}

UINT32 CMessage::SetDataLength(UINT32 len)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetDataLength");

    if (!ASSERT_VALID(this))
        return ~len;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return ~len;
    }
#endif

    return m_pMsg->SetDataLength(len);
}
#endif

bool CMessage::DeleteMessage()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::DeleteMessage");
    if (!ASSERT_VALID(this))
        return false;
    if (!ASSERT_VALID(m_pMsg))
        return false;
#endif
    m_pMsg->Destroy();
    return true;
}

CMessage* CMessage::Clone()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3, 0, "CMessage::Clone");
    if (!ASSERT_VALID(this))
        return NULL;
    if (!ASSERT_VALID(m_pMsg))
        return NULL;
#endif
    m_pMsg->AddRef();
    return new CMessage(m_pMsg);
}

UINT16 CMessage::GetTransactionId() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetTransactionId");
    ASSERT_VALID(this);
    LOG(LOG_CRITICAL,0,"Should not call this function.");
#endif
    return 0;
}

UINT16 CMessage::SetTransactionId(UINT16)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::SetTransactionId");
    ASSERT_VALID(this);
    LOG(LOG_CRITICAL,0,"Should nto call this function.");
#endif
    return 0;
}

bool CMessage::CreateMessage(CComEntity& Entity)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::CreateMessage(CComEntity&)");

    if (!ASSERT_VALID(this))
        return false;
#endif

    UINT32 uDataLen = GetDefaultDataLen();

    return CreateMessage(Entity, uDataLen);
}

bool CMessage::CreateMessage(CComEntity& Entity, UINT32 uDataSize)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::CreateMessage(CComEntity&, UINT32)");

    if (!ASSERT_VALID(this))
        return false;
#endif

    m_pMsg = new (&Entity, uDataSize) CComMessage;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"Allocate CComMessage failed.");
        return false;
    }

    SetMessageId(GetDefaultMsgId());
    m_pMsg->SetSrcTid(Entity.GetEntityId());
#ifdef WBBU_CODE
    m_pMsg->SetMoudlue(0);
#endif

#if 0
    if (GetDefaultMsgId() != SetMessageId(GetDefaultMsgId()))
    {
        LOG(LOG_CRITICAL,0,"Set Message ID failed.");
        m_pMsg->Destroy();
        m_pMsg = NULL;
        return false;
    }
    if ( Entity.GetEntityId() != m_pMsg->SetSrcTid(Entity.GetEntityId()) )
    {
        LOG(LOG_CRITICAL,0,"Set CComMessage member failed.");
        m_pMsg->Destroy();
        m_pMsg = NULL;
        return false;
    }
#endif
    return true;
}

#if 0
bool CMessage::Post()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::Post");

    if (!ASSERT_VALID(this))
        return false;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return false;
    }
#endif

    return CComEntity::PostEntityMessage(m_pMsg);
}
#endif

bool CMessage::Post(SINT32 timeout)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::Post(timeout)");

    if (!ASSERT_VALID(this))
        return false;

    if (m_pMsg==NULL)
    {
        LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
        return false;
    }
#endif

    return CComEntity::PostEntityMessage(m_pMsg,timeout);
}

#ifndef NDEBUG
bool CMessage::AssertValid(const char* lpszFileName, UINT32 nLine) const
{
#ifdef __WIN32_SIM__
    if (::IsBadReadPtr(this,sizeof(CMessage)) || ::IsBadWritePtr((void*)this,sizeof(CMessage)) )
    {
        CLog::LogAdd(lpszFileName,nLine,M_LL_CRITICAL,0,"Invalid CMessage pointer.");
        return false;
    }
#endif
    return true;
}
#endif

CMessage::CMessage(const CMessage& msg)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::CMessage(const CMessage&)");

    if (!ASSERT_VALID(this))
        return;
#endif

    m_pMsg = msg.m_pMsg;

#ifndef NDEBUG
    if (!Construct(M_OID_MESSAGE))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif
}

#if 0
UINT32 CMessage::GetDefaultDataLen() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CMessage::GetDefaultDataLen");
#endif
    return 0;
}

UINT16 CMessage::GetDefaultMsgId() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"");
#endif
    return M_INVALID_MESSAGE_ID;
}
#endif

