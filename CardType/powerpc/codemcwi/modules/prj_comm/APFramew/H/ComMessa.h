/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                 Arrowping Confidential Proprietary
 *
 * FILENAME: ComMessage.h
 *
 * DESCRIPTION:
 *     Declaration of the FWKLIB's internal message object.
 *
 * HISTORY:
 * Date        Author      Description
 * ----------  ----------  ----------------------------------------------------
 * 11/01/2005  Liu Qun     Removed m_uEID for M_TGT_CPE.
 * 07/12/2005  Liu Qun     Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_COMMESSAGE
#define _INC_COMMESSAGE

#ifndef _INC_OBJECT
#include "Object.h"
#include "datatype.h"
#include "new"//add by huangjl
#endif

#ifndef _INC_TASKDEF
#include "taskdef.h"
#endif

//#ifndef _INC_SFIDDEF
//#include "sfiddef.h"
//#endif

class CMsgQueue;
class CMessage;
class CComEntity;

#ifdef __WIN32_SIM__
class CPersonator;
#endif

#ifndef M_INVALID_MESSAGE_ID
#define M_INVALID_MESSAGE_ID 0xFFFF
#endif

#ifdef ENABLE_UT_CSI_SERVICE
#define CPE_CPE_CSI_DETECT_ID             0xFEFE
#endif

#ifndef M_COMMSG_FLAG_INVALID
#define M_COMMSG_FLAG_INVALID            0xFFFFFFFF
#define M_COMMSG_FLAG_HEADER_ALL         0x00FF0000
#define M_COMMSG_FLAG_EMS_HEADER_MADE    0x00010000
#define M_COMMSG_FLAG_L2_HEADER_MADE     0x00020000
#define M_COMMSG_FLAG_UT_HEADER_MADE
#endif

#ifndef M_INVALID_EID
#define M_INVALID_EID        0x00000000
#endif

#ifndef M_INVALID_UID
#define M_INVALID_UID        0x0000
#endif

class CComMessage
#ifndef NDEBUG
:public CObject
#endif
{
//private:
protected:
#if defined __WIN32_SIM__
    CMutex m_mutexRef;
#endif

    //TID m_tidDst;
    //TID m_tidSrc;
    UINT16 m_tidDst;
    UINT16 m_tidSrc;
    UINT16 m_uMsgId;
#ifdef WBBU_CODE
    UINT16 m_module;/*OSH表示发送给哪个核进行处理*/
#endif
#ifndef __NUCLEUS__    
    mutable UINT16 m_uRefCount;
#else
    UINT8 m_uRefCount;
    UINT8 m_uBlockSize;   // number of commesages newed in one block of memory
#endif
    UINT32 m_ulflags;

#if !(M_TARGET==M_TGT_CPE)
    UINT32 m_uEID;
    UINT16 m_uUID;
#endif
    UINT32 m_uBTSID;

    void* m_pBuf;
    void* m_pData;
    UINT16 m_uBufLen;
    UINT16 m_uDataLen;
#ifdef WBBU_CODE
        void*  m_pBlk;/*wangwenhua add 20090324***/
#endif
    CComEntity* m_pCreator;

#ifndef __NUCLEUS__
    mutable UINT32 m_ulTimeStamp;
#else
    UINT32 m_ulTimeStamp;
#endif


    //additional info for DataService
#if !(M_TARGET==M_TGT_CPE)
    UINT8 m_ucDirection;
    UINT8 m_ucIpType;
	UINT32 m_ulBtsAddr;
	UINT16 m_usBtsPort;
    UINT32 m_pUdpHdrOffset;     //UDP头到DataPtr的偏移
    UINT32 m_pDhcpHdrOffset;    //IP头到DataPtr的偏移
    UINT32 m_pKeyMacOffset;     //KeyMac到DataPtr的偏移
    //add by yhw, to support to add vlanid while broadcasting
	UINT16 m_usVlanid;
#endif

    CComMessage *m_pNextComMsg;

public:
    CComMessage();
    CComMessage(void* pBuf, UINT32 uBufLen, void* pData, UINT32 uDataLen);

#ifndef __NUCLEUS__ //added by maqiang 081022   
    UINT16 getRefCnt() { return  m_uRefCount;}
#else
    UINT8 getRefCnt() { return m_uRefCount; }
#endif

    #ifndef __NUCLEUS__
    void* operator new(size_t size, CComEntity* pEntity, size_t DataSize = 0);
	#else
    void* operator new(size_t size, CComEntity* pEntity, size_t DataSize = 0, bool canBeDiscarded=false);
	#endif
	
    void operator delete(void* p, CComEntity* pEntity, size_t DataSize);
    bool Destroy();

//    UINT32 RefCount() const;
#ifndef __NUCLEUS__    
    bool AddRef() const;
    bool Release() const;
#else
    bool AddRef();
    bool Release();
#endif
	CComEntity* getCreator() const{return m_pCreator;};
    UINT32 GetFlag() const;
    UINT32 SetFlag(UINT32);
    TID GetDstTid() const;
    TID SetDstTid(TID);
    TID GetSrcTid() const;
    TID SetSrcTid(TID);
    UINT16 GetMessageId() const;
    UINT16 SetMessageId(UINT16);
#ifdef WBBU_CODE
  inline  void SetMoudlue(UINT16 module){m_module = module;};
   inline UINT16 GetModule() {return m_module;};
#endif
#if !(M_TARGET==M_TGT_CPE)
    UINT32 GetEID() const;
    UINT32 SetEID(UINT32);
    UINT16 GetUID() const;
    UINT16 SetUID(UINT16);
#endif
    UINT32 GetBTS() const;
    UINT32 SetBTS(UINT32 usBtsId);

    //SFID GetSFID() const;
    //SFID SetSFID(SFID sfidNew);

    inline void* GetDataPtr() const { return m_pData;};
    void* SetDataPtr(void*);

    inline UINT16 GetDataLength() const {  return m_uDataLen;};
    inline UINT16 SetDataLength(UINT16 len) {  return m_uDataLen = len;};

//    inline bool IsUrgent() const {    return m_bUrgent;};
//    inline bool SetUrgent(bool bUrgent) { return m_bUrgent = bUrgent;};

    // xiao weifang add {
#if !(M_TARGET==M_TGT_CPE)
    UINT8 SetDirection(UINT8);
    UINT8 GetDirection() const;

    UINT8 SetIpType(UINT8);
    UINT8 GetIpType() const;

    UINT32 SetBtsAddr(UINT32);
    UINT32 GetBtsAddr() const;

	UINT16 SetBtsPort(UINT16);
	UINT16 GetBtsPort() const;

    void  SetDhcpPtr(void*);
    void* GetDhcpPtr() const;

    void  SetUdpPtr(void*);
    void* GetUdpPtr() const;

    void   SetKeyMac(UINT8*);
    UINT8* GetKeyMac() const;

	inline UINT16 SetVlanID(UINT16 ulTS=1) { return m_usVlanid = ulTS;};
	inline UINT16 GetVlanID()const  { return m_usVlanid ;};

#endif

#ifdef __NUCLEUS__    
	inline UINT32 SetTimeStamp(UINT32 ulTS) { return m_ulTimeStamp = ulTS;};
#else
	inline UINT32 SetTimeStamp(UINT32 ulTS) const { return m_ulTimeStamp = ulTS;};
#endif
	inline UINT32 GetTimeStamp() const {  return m_ulTimeStamp;};
#ifdef WBBU_CODE
       inline void SetMblk(void *pmblk){m_pBlk = pmblk;};/**wangwenhua add 20090324**/
	inline void*GetMblk(){return m_pBlk;};/**wangwenhua add 20090324***/
#endif
#ifndef NDEBUG
    bool AssertValid(const char* lpszFileName, UINT32 nLine) const;
#endif

    UINT16 GetBufferLength() const { return m_uBufLen;};

    bool SetBuffer(void*, UINT32);
    bool DeleteBuffer();

    inline void* GetBufferPtr() const{  return m_pBuf;};

    CComMessage* getNext() const{ return m_pNextComMsg; }
    void setNext(CComMessage *pComMessage){ m_pNextComMsg = pComMessage; }

private:
    ~CComMessage();
    void* operator new(size_t);
    void operator delete(void* p);

    friend class CComEntity;
    friend class CBtsAgent;
#ifdef __WIN32_SIM__
    friend class CPersonator;
#endif
};

#endif
