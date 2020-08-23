#ifndef _INC_COMMESSAGE
#include "ComMessage.h"
#endif

#ifndef _INC_COMENTITY
#include "ComEntity.h"
#endif

#ifndef _FWK_ERROR_H
#include "FrameworkErrorCode.h"
#endif

#ifndef _INC_APASSERT
#include "APASSERT.h"
#endif

#ifdef __WIN32_SIM__ //Visual C++

#ifndef _INC_TYPEINFO
#include <typeinfo.h>
#endif

#elif __NUCLEUS__

#ifndef NUCLEUS
#include "Nucleus.h"
#endif

#include <typeinfo>	//OMAP

#else //Tornado

//#include <intLib.h>//delete by huangjl
//#include <taskLib.h>//delete by huangjl

#ifndef __INCstringh
#include <string.h>
#endif

#ifndef __TYPEINFO__
#include <typeinfo>
using namespace std;
#endif

#endif

CComMessage::CComMessage()
:m_uRefCount(0),m_ulflags(0),
m_tidDst(M_TID_MAX),m_tidSrc(M_TID_MAX),
m_uMsgId(M_INVALID_MESSAGE_ID),
#ifdef WBBU_CODE
m_module(0),m_pBlk(NULL),
#endif
//m_ulTimeStamp(0xffffffff),
m_pNextComMsg(NULL),
#if !(M_TARGET==M_TGT_CPE)
m_uEID(0),m_ucDirection(0xff),m_ucIpType(0xff),m_ulBtsAddr(0),
m_pUdpHdrOffset(0),m_pDhcpHdrOffset(0),m_pKeyMacOffset(0)
#else
m_uBTSID(0xFFFFFFFF)
#endif
{
   m_usVlanid = 1;//M_NO_VLAN_TAG
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::CComMessage");

    if (!ASSERT_VALID(this))
        return;
        
    if (!Construct(M_OID_COMMESSAGE))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif
}

CComMessage::CComMessage(void* pBuf, UINT32 uBufLen, void* pData, UINT32 uDataLen)
:m_uRefCount(0),m_ulflags(0),
m_tidDst(M_TID_MAX),m_tidSrc(M_TID_MAX),
m_uMsgId(M_INVALID_MESSAGE_ID),
//m_ulTimeStamp(0xffffffff),
#if !(M_TARGET==M_TGT_CPE)
m_uEID(0),m_ucDirection(0xff),m_ucIpType(0xff),m_ulBtsAddr(0),
m_pUdpHdrOffset(0),m_pDhcpHdrOffset(0),m_pKeyMacOffset(0)
#else
m_uBTSID(0xFFFFFFFF)
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::CComMessage(...)");

    if (!ASSERT_VALID(this))
        return;
    if (m_pBuf!=NULL || m_uBufLen!=0 || m_pData!=NULL || m_uDataLen!=0)
        return;

    if (!Construct(M_OID_COMMESSAGE))
        LOG(LOG_SEVERE,0,"Construct failed.");
#endif

    m_pBuf = pBuf;
    m_uBufLen = uBufLen;
    m_pData = pData;
    m_uDataLen = uDataLen;
	m_pNextComMsg = NULL;
	m_usVlanid = 1;//M_NO_VLAN_TAG;
#ifdef WBBU_CODE
	m_module = 0;
#endif
}

CComMessage::~CComMessage()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::~CComMessage");

    if (!ASSERT_VALID(this))
        return;

    if (!Destruct(CObject::M_OID_COMMESSAGE))
        LOG(LOG_SEVERE,0,"Destruct failed.");
#endif
#ifdef WBBU_CODE
  m_module = 0;
#endif
}

#ifndef __NUCLEUS__
bool CComMessage::AddRef() const
#else
bool CComMessage::AddRef()
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::AddRef");
    if (!ASSERT_VALID(this))
        return false;
#endif

#ifdef __WIN32_SIM__
    if (!m_mutexRef.Wait())
    {
        LOG(LOG_SEVERE,0,"mutex Wait failed.");
        return false;
    }
#elif __NUCLEUS__
    UINT32 oldlevel;
    oldlevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);//disable global interrupt
#else //VxWorks
    ::taskLock();
//    UINT32 oldlevel = ::intLock();//delete by huangjl
#endif

    ++m_uRefCount;

#ifdef __WIN32_SIM__
    if (!m_mutexRef.Release())
    {
        LOG(LOG_SEVERE,0,"mutex Release failed.");
        return false;
    }
#elif __NUCLEUS__
    ::NU_Control_Interrupts(oldlevel);//restore CPSR(restore global interrput)
#else //VxWorks
//    ::intUnlock(oldlevel);//delete by huangjl
		::taskUnlock();
#endif

    return true;
}

#if 0
UINT32 CComMessage::RefCount() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::RefCount");

    if (!ASSERT_VALID(this))
        return 0xFFFFFFFF;
#endif
    return m_uRefCount;
}
#endif

#ifndef __NUCLEUS__
bool CComMessage::Release() const
#else
bool CComMessage::Release()
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::Release");

    if (!ASSERT_VALID(this))
        return false;
#endif

    if (m_uRefCount==0)
        return true;

#ifdef __WIN32_SIM__
    if (!m_mutexRef.Wait())
    {
        LOG(LOG_SEVERE,0,"mutex Wait failed.");
        return false;
    }
#elif __NUCLEUS__
    UINT32 oldlevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);//disable global interrupt
#else //VxWorks
    ::taskLock();
//    UINT32 oldlevel = ::intLock();//delete by huangjl
#endif

    --m_uRefCount;

#ifdef __WIN32_SIM__
    if (!m_mutexRef.Release())
    {
        LOG(LOG_SEVERE,0,"mutex Release failed.");
        return false;
    }
#elif __NUCLEUS__
    ::NU_Control_Interrupts(oldlevel);//restore global interrput
#else //VxWorks
//    ::intUnlock(oldlevel);//delete by huangjl
	::taskUnlock();
#endif

    return true;
}

#ifndef __NUCLEUS__
void* CComMessage::operator new(size_t size, CComEntity* pEntity, size_t DataSize)
#else
void* CComMessage::operator new(size_t size, CComEntity* pEntity, size_t DataSize, bool canBeDiscarded)
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::operator new(CComEntity*,size_t)");

    void* p = NULL;
    if (!ASSERT_VALID(pEntity))
        return p;
#endif


    #ifndef __NUCLEUS__
    return pEntity->AllocateComMessage(size, DataSize);
    #else
    return pEntity->AllocateComMessage(size, DataSize, canBeDiscarded);
    #endif
}

void CComMessage::operator delete(void* p, CComEntity* pEntity, size_t)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::operator delete(CComEntity,size_t)");

    if (!ASSERT_VALID(pEntity))
        return;
#endif

    CComMessage* pComMsg = (CComMessage*)p;
    if (!pEntity->DeallocateComMessage(pComMsg))
        LOG(LOG_SEVERE,0,"Deallocate failed.");
}

bool CComMessage::Destroy()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::Destroy");

#endif
//delete by huangjl
//    ApAssertRtnV( this, LOG_SEVERE,FMK_ERROR_DESTROY_NULL_MSG, "Destroy Null ComMessage", ; ,false);
    if( !(this->m_pCreator))
    {
      LOG3( LOG_SEVERE, FMK_ERROR_DESTROY_NULL_MSG, "Destory Null creator:%x,%x,%x\n",m_tidDst,m_tidSrc,m_uMsgId);
     
    }
 //delete by huangjl   
//    ApAssertRtnV( this->m_pCreator, LOG_SEVERE, FMK_ERROR_DESTROY_NULL_MSG, "Destroy ComMessage with NULL Creator", 
//                  {LOGMSG(LOG_SEVERE,FMK_ERROR_DESTROY_NULL_MSG, this, "");}, false);

#ifdef __WIN32_SIM__
    if (!m_mutexRef.Wait())
    {
        LOG(LOG_CRITICAL,0,"Wait m_mutexRef failed.");
        return NULL;
    }
#elif __NUCLEUS__
    UINT32 oldlevel = ::NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);
#else //VxWorks
    ::taskLock();
//    UINT32 oldlevel = ::intLock();//delete by huangjl
#endif

    if ( m_uRefCount > 0 )
    {
        m_uRefCount --;
    }

    if ( m_uRefCount > 0)
	{
#ifdef __WIN32_SIM__
		if (!m_mutexRef.Release())
		{
			LOG(LOG_CRITICAL,0,"Release m_mutexRef failed.");
		}
#elif __NUCLEUS__ 
        ::NU_Control_Interrupts(oldlevel);	//restore CPSR(restore global interrput)
#else //VxWorks
//        ::intUnlock(oldlevel);//delete by huangjl
		::taskUnlock();
#endif
        return true;
	}

#ifdef __WIN32_SIM__
	if (!m_mutexRef.Release())
	{
		LOG(LOG_CRITICAL,0,"Release m_mutexRef failed.");
	}
#endif
    delete this;

#ifdef __WIN32_SIM__
#elif __NUCLEUS__ 
    ::NU_Control_Interrupts(oldlevel);	//restore CPSR(restore global interrput)
#else //VxWorks
  //  ::intUnlock(oldlevel);//delete by huangjl
		::taskUnlock();
#endif

    return true;
}

void CComMessage::operator delete(void* p)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::operator delete");
#endif
     if(p==NULL)
     	return;
    CComMessage* pComMsg = (CComMessage*)p;

#ifndef NDEBUG
    if (!ASSERT_VALID(pComMsg->m_pCreator))
    {
        LOG(LOG_CRITICAL,0,"Invalid Creator pointer. delete failed.");
        return;
    }
#endif

    if (!pComMsg->m_pCreator->DeallocateComMessage(pComMsg))
        LOG(LOG_SEVERE,0,"Deallocate CComMessage failed.");
}

UINT32 CComMessage::GetFlag() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetFlag");

    if ( !ASSERT_VALID(this) )
        return M_COMMSG_FLAG_INVALID;
#endif
    return m_ulflags;
}

UINT32 CComMessage::SetFlag(UINT32 ulFlag)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetFlag");
    if (!ASSERT_VALID(this))
        return M_COMMSG_FLAG_INVALID;
#endif

    return m_ulflags = ulFlag;
}

TID CComMessage::GetDstTid() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetDstTid");

    if ( !ASSERT_VALID(this) || m_tidDst >= M_TID_MAX)
        return M_TID_MAX;
#endif

    return (TID)m_tidDst;
}

TID CComMessage::SetDstTid(TID tid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetDstTid");

    if (!ASSERT_VALID(this))
        return M_TID_MAX;

    if (tid >= M_TID_MAX)
    {
        LOG2(LOG_SEVERE,0,"Invalid parameter.:%x,%x",tid,m_uMsgId);
        return (TID)m_tidDst;
    }
#endif

    m_tidDst = tid;
    return tid;
}

TID CComMessage::GetSrcTid() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetSrcTid");

    if (!ASSERT_VALID(this))
        return M_TID_MAX;
    if (m_tidSrc>=M_TID_MAX)
    {
        LOG(LOG_SEVERE,0,"m_tidSrc>=M_TID_MAX.");
        return M_TID_MAX;
    }
#endif
    return (TID)m_tidSrc;
}

TID CComMessage::SetSrcTid(TID tid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetSrcTid");

    if (!ASSERT_VALID(this))
        return M_TID_MAX;

    if (tid >= M_TID_MAX)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter >= M_TID_MAX.");
        return (TID)m_tidSrc;
    }
#endif

    m_tidSrc = tid;
    return tid;
    
}

UINT16 CComMessage::GetMessageId() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetMessageId");

    if (!ASSERT_VALID(this))
        return M_INVALID_MESSAGE_ID;
#endif

    return m_uMsgId;
}

UINT16 CComMessage::SetMessageId(UINT16 mid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetMessageId");

    if (!ASSERT_VALID(this))
        return M_INVALID_MESSAGE_ID;

    if (mid == M_INVALID_MESSAGE_ID)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter.");
        return m_uMsgId;
    }
#endif

    return m_uMsgId = mid;
}

#if !(M_TARGET==M_TGT_CPE)
UINT32 CComMessage::GetEID() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetEID");

    if (!ASSERT_VALID(this))
        return M_INVALID_EID;
#endif

    return m_uEID;
}

UINT32 CComMessage::SetEID(UINT32 eid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetEID");

    if (!ASSERT_VALID(this))
        return M_INVALID_EID;

    if (eid == M_INVALID_EID)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter.");
        return m_uEID;
    }
#endif

    return m_uEID = eid;
}

UINT16 CComMessage::GetUID() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetUID");

    if (!ASSERT_VALID(this))
        return M_INVALID_UID;
#endif

    return m_uUID;
}

UINT16 CComMessage::SetUID(UINT16 uid)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetUID");

    if (!ASSERT_VALID(this))
        return M_INVALID_UID;

    if (uid == M_INVALID_UID)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter.");
        return m_uUID;
    }
#endif

    return m_uUID = uid;
}

#endif //endof !(M_TARGET==M_TGT_CPE)

UINT32 CComMessage::GetBTS() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetBTS");

    if (!ASSERT_VALID(this))
        return 0xFFFF;
#endif

    return m_uBTSID;
}

UINT32 CComMessage::SetBTS(UINT32 ulBtsID)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetBTS");

    if (!ASSERT_VALID(this))
        return 0xFFFF;

    if (ulBtsID == 0xFFFFFFFF)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter.");
        return ulBtsID;
    }
#endif

    return m_uBTSID = ulBtsID;
}

/*
SFID CComMessage::GetSFID() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetSFID");
#endif
    if (!ASSERT_VALID(this))
        return M_SFID_INVALID;
    return m_uSFID;
}

SFID CComMessage::SetSFID(SFID sfidNew)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetSFID");
#endif
    if (!ASSERT_VALID(this) || sfidNew==M_SFID_INVALID)
        return M_SFID_INVALID;

    return m_uSFID = sfidNew;
}
*/

/*
void* CComMessage::GetBufferPtr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetBufferPtr");

    if (!ASSERT_VALID(this))
        return NULL;
#endif

    return m_pBuf;
}
*/


/*
UINT32 CComMessage::GetBufferLength() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetBufferLength");

    if (!ASSERT_VALID(this))
        return 0;
#endif

    return m_uBufLen;
}
*/

bool CComMessage::SetBuffer(void* ptr, UINT32 ulLen)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetBuffer");

    if (!ASSERT_VALID(this))
        return false;

    if ( ( NULL != m_pBuf )
        &&( ( m_pBuf != ptr ) || ( m_uBufLen != ulLen ) )  )
    {
        return false;
    }
#endif

    m_pBuf = ptr;
    m_uBufLen = ulLen;

    return true;
}

bool CComMessage::DeleteBuffer()
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::DeleteBuffer");

    if (!ASSERT_VALID(this))
        return false;
#endif

    m_uBufLen = 0;
    m_pBuf = NULL;
    return true;
}

/*
void* CComMessage::GetDataPtr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetDataPtr");

    if (!ASSERT_VALID(this))
        return NULL;
#endif

    return m_pData;
}
*/


void* CComMessage::SetDataPtr(void* ptr)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetDataPtr");

    if (!ASSERT_VALID(this))
        return NULL;

    if (m_pBuf!=NULL && ptr<m_pBuf)
    {
        LOG(LOG_CRITICAL,0,"Invalid parameter.");
        return m_pData;
    }
#endif
    return m_pData = ptr;
}


/*
UINT32 CComMessage::GetDataLength() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetDataLength");

    if (!ASSERT_VALID(this))
        return 0;
#endif

    return m_uDataLen;
}
*/
/*
UINT32 CComMessage::SetDataLength(UINT32 len)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetDataLength");

    if (!ASSERT_VALID(this))
        return ~len;
#endif

    return m_uDataLen = len;
}
*/

/*
CComEntity* CComMessage::GetCreator() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetCreator");

    if (!ASSERT_VALID(this))
        return NULL;
#endif

    return m_pCreator;
}
*/

/*
CComEntity* CComMessage::SetCreator(CComEntity* pCreator)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetCreator");

    if (!ASSERT_VALID(this))
        return NULL;

    if (!ASSERT_VALID(pCreator))
        return m_pCreator;
#endif

    return m_pCreator = pCreator;
}
*/

/*
bool CComMessage::IsUrgent() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::IsUrgent");

    if (!ASSERT_VALID(this))
        return false;
#endif

    return m_bUrgent;
}
*/

/*
bool CComMessage::SetUrgent(bool bUrgent)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetUrgent");

    if (!ASSERT_VALID(this))
        return false;
#endif

    return m_bUrgent = bUrgent;
}
*/


// xiao weifang add {
#if !(M_TARGET==M_TGT_CPE)
UINT8 CComMessage::SetDirection(UINT8 ucDir)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetDirection");

    if (!ASSERT_VALID(this))
        return (~ucDir);
#endif
    
    return m_ucDirection = ucDir;
}

UINT8 CComMessage::GetDirection() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetDirection");

    if (!ASSERT_VALID(this))
        return (~0);
#endif

    return m_ucDirection;
}

UINT8 CComMessage::SetIpType(UINT8 ucIpType)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetIpType");

    if (!ASSERT_VALID(this))
        return (~ucIpType);
#endif

    return m_ucIpType = ucIpType;
}

UINT8 CComMessage::GetIpType() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetIpType");

    if (!ASSERT_VALID(this))
        return (~0);
#endif

    return m_ucIpType;
}

UINT32 CComMessage::SetBtsAddr(UINT32 ulBtsIp)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetBtsAddr");

    if (!ASSERT_VALID(this))
        return ~ulBtsIp;
#endif

    return m_ulBtsAddr = ulBtsIp;
}

UINT32 CComMessage::GetBtsAddr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetBtsAddr");
    if (!ASSERT_VALID(this))
        return 0;
#endif
    return m_ulBtsAddr;
}

UINT16 CComMessage::SetBtsPort(UINT16 ulBtsPort)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetBtsPort");

	if (!ASSERT_VALID(this))
		return ~ulBtsPort;
#endif

    return m_usBtsPort = ulBtsPort;
}

UINT16 CComMessage::GetBtsPort() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetBtsPort");
	if (!ASSERT_VALID(this))
		return 0;
#endif
    return m_usBtsPort;
}

void CComMessage::SetDhcpPtr(void* pDhcpPtr)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetDhcpPtr");

    if (!ASSERT_VALID(this))
        return ;

    if (pDhcpPtr==NULL)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter DhcpPtr.");
        return ;
    }
#endif

    m_pDhcpHdrOffset = (UINT32)pDhcpPtr - (UINT32)m_pData;
}

void* CComMessage::GetDhcpPtr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetDhcpPtr");

    if (!ASSERT_VALID(this))
        return NULL;
#endif

    return (void*)( m_pDhcpHdrOffset + (UINT32)m_pData );
}


void CComMessage::SetUdpPtr(void *pUdp)
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetUdpPtr");

    if (!ASSERT_VALID(this))
        return ;

    if (pUdp==NULL)
    {
        LOG(LOG_SEVERE,0,"Invalid parameter DhcpPtr.");
        return ;
    }
#endif

    m_pUdpHdrOffset = (UINT32)pUdp - (UINT32)m_pData;
}


void* CComMessage::GetUdpPtr() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetUdpPtr");

    if (!ASSERT_VALID(this))
        return NULL;
#endif

    return (void*)( m_pUdpHdrOffset + (UINT32)m_pData );
}


void CComMessage::SetKeyMac(UINT8 *pMac)
{
#ifndef NDEBUG
    LOG( LOG_DEBUG3, 0, "CComMessage::SetKeyMac" );

    if ( NULL == pMac )
    {
        LOG(LOG_SEVERE,0,"Invalid parameter pMac.");
        return ;
    }
#endif

    m_pKeyMacOffset = (UINT32)pMac - (UINT32)m_pData;
}

UINT8* CComMessage::GetKeyMac() const
{
#ifndef NDEBUG
    LOG( LOG_DEBUG3, 0, "CComMessage::GetKeyMac" );

    if ( !ASSERT_VALID( this ) )
        return NULL;
#endif

    return (UINT8*)( m_pKeyMacOffset + (UINT32)m_pData );
}

#endif

/*
#ifdef __NUCLEUS__    
UINT32 CComMessage::SetTimeStamp(UINT32 ulTS)
#else
UINT32 CComMessage::SetTimeStamp(UINT32 ulTS) const
#endif
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::SetTimeStamp");
#endif

    if (!ASSERT_VALID(this))
        return ~ulTS;

    return m_ulTimeStamp = ulTS;
}
*/
/*UINT32 CComMessage::GetTimeStamp() const
{
#ifndef NDEBUG
    LOG(LOG_DEBUG3,0,"CComMessage::GetTimeStamp");
#endif

    if (!ASSERT_VALID(this))
        return 0;

    return m_ulTimeStamp;
}
*/
// xiao weifang add }

#ifndef NDEBUG
bool CComMessage::AssertValid(const char* lpszFileName, UINT32 nLine) const
{
#ifdef __WIN32_SIM__
    if (::IsBadReadPtr(this,sizeof(CComMessage)) || ::IsBadWritePtr((void*)this,sizeof(CComMessage)) )
    {
        CLog::LogAdd(lpszFileName,nLine,M_LL_CRITICAL,0,"Invalid CComMessage pointer.");
        return false;
    }
#endif
#ifndef __NUCLEUS__
    if (typeid(CComMessage) != typeid(*this))
    {
        LOG(LOG_CRITICAL,0,"Invalid typeid.");
        return false;
    }
#endif
    return true;
}
#endif

void* CComMessage::operator new(size_t)
{
    void* p=NULL;
    return p;
}
