/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: Message.h
 *
 * DESCRIPTION:
 *     Declaration of the FWKLIB's internal message wrapper.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 11/01/2005  Liu Qun     Removed Get/Set EID for M_TGT_CPE.
 * 07/18/2005  Liu Qun     Initial file creation.
 *
 *---------------------------------------------------------------------------*/
#ifndef _INC_MESSAGE
#define _INC_MESSAGE

#ifndef _INC_OBJECT
#include "Object.h"
#endif

#ifndef _INC_COMENTITY
#include "ComEntity.h"
#endif

#include "ComMessage.h"
#include "Log.h"
//#ifndef _INC_SFIDDEF
//#include "sfiddef.h"
//#endif

class CComEntity;
class CComMessage;
class CTransaction;
class CTimer;

#ifndef M_INVALID_MESSAGE_ID
#define M_INVALID_MESSAGE_ID 0xFFFF
#endif

#ifndef M_COMMSG_FLAG_INVALID
#define M_COMMSG_FLAG_INVALID            0xFFFFFFFF
#define M_COMMSG_FLAG_HEADER_ALL         0x00FF0000
#define M_COMMSG_FLAG_EMS_HEADER_MADE    0x00010000
#define M_COMMSG_FLAG_L2_HEADER_MADE     0x00020000
#endif

#ifndef M_INVALID_EID
#define M_INVALID_EID        0x00000000
#endif

#ifndef M_INVALID_UID
#define M_INVALID_UID        0x0000
#endif


class CMessage 
#ifndef NDEBUG
:public CObject
#endif
{
private:
    CComMessage* m_pMsg;

public:
    CMessage(CComMessage* pMsg);

    inline UINT16 GetMessageId() const
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return M_INVALID_MESSAGE_ID;
        }
#endif
        return m_pMsg->GetMessageId();
    }

    inline UINT16 SetMessageId(UINT16 mid)
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return M_INVALID_MESSAGE_ID;
        }
#endif
        return m_pMsg->SetMessageId(mid);
    }
#ifdef WBBU_CODE
    inline void   SetModuleId(UINT16 mod)
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return; //modify by huangjl M_INVALID_MESSAGE_ID;
        }
#endif
        return m_pMsg->SetMoudlue(mod);
    }
#endif
    bool Post(SINT32 timeout = NO_WAIT);

    inline TID GetDstTid() const
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return M_TID_MAX;
        }
#endif
        return m_pMsg->GetDstTid();
    }

    inline TID GetSrcTid() const
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return M_TID_MAX;
        }
#endif
        return m_pMsg->GetSrcTid();
    }

    inline TID SetDstTid(TID tid)
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return M_TID_MAX;
        }
#endif
        return m_pMsg->SetDstTid(tid);
    }

    inline TID SetSrcTid(TID tid)
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return M_TID_MAX;
        }
#endif
        return m_pMsg->SetSrcTid(tid);
    }

#if !(M_TARGET==M_TGT_CPE)
    inline UINT32 GetEID() const
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return 0;
        }
#endif
        return m_pMsg->GetEID();
    }

    inline UINT32 SetEID(UINT32 uEID)
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return ~uEID;
        }
#endif
        return m_pMsg->SetEID(uEID);
    }

    void* SetDataPtr(void* ptr)
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
#ifndef WBBU_CODE
            return ~uEID;
#else
            return NULL/*~uEID*/;
#endif
        }
#endif
        return m_pMsg->SetDataPtr(ptr);
    }

    UINT16 GetUID() const;
    UINT16 SetUID(UINT16);
#endif

    // xiao weifang add {
#if !(M_TARGET==M_TGT_CPE)
    UINT8 SetDirection(UINT8);
    UINT8 GetDirection() const;

//    UINT8 SetIpType(UINT8);
    UINT8 GetIpType() const;
	//and by yanghuawei ,to support to saving IP@Port in tSOCKET
    UINT32 SetBTSPubIP(UINT32 );
    UINT32 GetBTSPubIP() const;
    UINT16 SetBTSPubPort(UINT16 );
    UINT16 GetBTSPubPort() const;
    //end 
	
//    void* SetUdpPtr(void*);
    void* GetUdpPtr() const;

//    void* SetDhcpPtr(void*);
    void* GetDhcpPtr() const;

    void   SetKeyMac(UINT8*);
    UINT8* GetKeyMac() const;

    UINT32 SetBtsAddr(UINT32);  //bts IP address.
    UINT32 GetBtsAddr() const;

    UINT16 SetBtsPort(UINT16);
    UINT16 GetBtsPort() const;
#endif
    UINT32 GetBTS() const;      //bts ID
    UINT32 SetBTS(UINT32 ulBtsId);

    // xiao weifang add }

    inline UINT16 GetDataLength() const
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return 0;
        }
#endif
        return m_pMsg->GetDataLength();
    }

    bool CreateMessage(CComEntity&);//Use GetDefaultDataLen()
    bool CreateMessage(CComEntity&, UINT32 uDataSize);
    bool DeleteMessage();

    CMessage* Clone();

    virtual UINT16 GetTransactionId() const;
    virtual UINT16 SetTransactionId(UINT16);

#ifndef NDEBUG
    virtual bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
#endif

    virtual ~CMessage();
#ifdef WBBU_CODE
     inline void SetModule(unsigned char flag)
     	{  
     		m_pMsg->SetMoudlue(flag);
     	}
#endif
    inline void* GetDataPtr() const
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return NULL;
        }
#endif
        return m_pMsg->GetDataPtr();
    }

    inline UINT16 SetDataLength(UINT16 len)
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return ~len;
        }
#endif
        return m_pMsg->SetDataLength(len);
    }
    inline CComMessage* GetpComMsg()
    {
#ifndef NDEBUG
        if (m_pMsg==NULL)
        {
            LOG(LOG_SEVERE,0,"m_pMsg==NULL.");
            return NULL;
        }
#endif
        return m_pMsg;
    }
protected:
    CMessage();
    CMessage(const CMessage&);

    inline virtual UINT32 GetDefaultDataLen() const 
    {
        return 0;
    }
    inline virtual UINT16 GetDefaultMsgId() const
    {
        return M_INVALID_MESSAGE_ID;
    }

    friend class CTransaction;
    friend class CTimer;

};

#endif
