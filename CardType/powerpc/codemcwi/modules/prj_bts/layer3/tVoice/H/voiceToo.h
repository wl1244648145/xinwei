/*******************************************************************************
* Copyright (c) 2009 by Beijing Arrowping Communication Co.Ltd.All Rights Reserved   
* File Name      : voiceToolFunc.h
* Create Date    : 26-Jun-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/

#ifndef	__VOICETOOLFUNC_H
#define	__VOICETOOLFUNC_H

#include "ComEntity.h"
#include "taskdef.h"
#include "Log.h"


#define V__Assert(condition, level, code, str, block)(condition, level, code, str, block)  \
    { \
    if (!(condition)) { \
        LOG(level, code, str); \
        {block;} \
        } \
    }

#define V__AssertRtnV(condition, level, code, str, block, rv)  \
    { \
    if (!(condition)) { \
        LOG(level, code, str); \
        { block ;} \
        return ( rv ); \
        } \
    }

#define V__ApAssertRtn(condition, level, code, str, block)  \
    { \
    if (!(condition)) { \
        LOG(level, code, str); \
        {block;} \
        return; \
        } \
    }

////////////////////////////////////////////////////////////////////////////////
//for socket --------------begin
////////////////////////////////////////////////////////////////////////////////


#ifdef __VXWORKS__

#include "vxWorks.h" 
#include "ioLib.h"
#include "selectLib.h"
#include "sockLib.h"
#include "inetLib.h" 
#include "errnoLib.h"
#include "netinet/tcp.h"

#elif defined __USE_LWIP__

#include <math.h>
#include "datatype.h"
#include "util.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip_test.h"
#include "sck_test.h"


#elif defined __WIN32_SIM__

#include "winsock2.h"

#else

//#error "socket include file not set"

#endif


//=========================

#ifdef __WIN32_SIM__

#define errnoGet WSAGetLastError
#define Ioctl_Socket(a,b,c) ioctlsocket(a,b,(u_long*)c)
#define IOCTL_SOCKET_ERROR (SOCKET_ERROR)

#else

typedef struct fd_set FD_SET;
#ifndef closesocket
#define closesocket close
#endif
#define Ioctl_Socket(a,b,c) ioctl(a,b,c)
#define IOCTL_SOCKET_ERROR (ERROR)

#endif

#ifdef __USE_LWIP__
#undef Ioctl_Socket
#define Ioctl_Socket(a,b,c) ioctlsocket(a,b,(void*)c)
#define get_sock_name(a,b,c) getsockname(a,b,(u32_t*)c)
#else
#define get_sock_name getsockname
#endif//#ifdef __USE_LWIP

#ifdef __VXWORKS__
#undef Ioctl_Socket
#define Ioctl_Socket(a,b,c) ioctl(a,b,(int)c)
#endif


//===================================

extern void OutputSocketErrCode(char *p);

////////////////////////////////////////////////////////////////////////////////
//for socket --------------end
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//RcvBuf/TcpRcvBuf/TcpEWithNatApRcvBuf---------begin
////////////////////////////////////////////////////////////////////////////////

#define M_MAX_DATA_LEN		(0xffff)
#define M_CONTINUE_RECV (0)
#define M_GET_ONE_MSG (1)
#define M_CONTINUE_PARSE (2)
class RcvBuf
{
public:
	RcvBuf();
	virtual ~RcvBuf();
	void resetRcvBuf();
	void moveRcvDataToHead();
	int readFromBuf(char *dstBuf, UINT16 nLen);
	virtual int getOneMsgOutofBuf(char ** ppBuf, UINT32 *pLen);
	void setShowInfoFlag(bool flag){m_blShowInfo = flag;};
	bool getShowInfoFlag(){return m_blShowInfo;};
	void setShowDetailFlag(bool flag){m_blShowInfoDetail= flag;};
	bool getShowDetailFlag(){return m_blShowInfoDetail;};
	void show();

	char* getBufPtr(){return pBuf;};
	char* getDataHeadPtr(){return pDataHead;};
	void setDataHeadPtr(char* pHead){pDataHead=pHead;};
	void seekToPos(char* pPos){pCurPos=pPos;};
	void seekFromHead(UINT32 offset){pCurPos=pDataHead+offset;};
	unsigned long getDataLen(){return nDataLen;};
	void setDataLen(unsigned long len){nDataLen=len;};
	
	char *pBuf;
	char *pDataHead;
	char *pCurPos;
	unsigned long nDataLen;

protected:

private:
	bool m_blShowInfo;
	bool m_blShowInfoDetail;

};

class TcpRcvBuf:public RcvBuf
{
public:
	int getFd(){return m_fd;};
	void setFd(int fd){m_fd=fd;};
	int recvData2Buf();
private:
	int m_fd;
};

class TcpEWithNatApRcvBuf:public TcpRcvBuf
{
public:
	int getOneMsgOutofBuf(char ** ppBuf, UINT32 *pLen);
	int getOneMsgOutofBuf(CComMessage **ppMsg);
};


////////////////////////////////////////////////////////////////////////////////
//RcvBuf/TcpRcvBuf/TcpEWithNatApRcvBuf---------end
////////////////////////////////////////////////////////////////////////////////

class ValGuard
{
public:
	explicit ValGuard(){};
	virtual ~ValGuard(){};
	void init(UINT32 val, UINT32 checkInterval)
	{
		m_val = m_lastVal = val;
		m_checkTimeInterval = checkInterval;
		m_unchangedTimes = 0;
	};
	void init(){m_unchangedTimes = 0;};
	UINT32 valAdd(int delta){return m_val+=delta;};
	bool checkIfValChanged();
	UINT32 getTimeUnchanged();

	UINT32 getCheckInterval(){return m_checkTimeInterval;};
	void setCheckInterval(UINT32 checkInterval){m_checkTimeInterval=checkInterval;};
private:
	UINT32 m_val;
	UINT32 m_lastVal;
	UINT32 m_checkTimeInterval;
	UINT32 m_unchangedTimes;
};

#ifdef __cplusplus
extern "C" {
#endif


inline UINT32 VGetU32BitVal(UINT8 *pU8)
{
	return ( *pU8<<24 | *(pU8+1)<<16 | *(pU8+2)<<8 | *(pU8+3) );
}
inline UINT32 VSetU32BitVal(UINT8 *pU8, UINT32 val)
{
	*(pU8+3) = val & 0xFF;	
	*(pU8+2) = ((val>>8) & 0xFF);
	*(pU8+1) = ((val>>16) & 0xFF);
	*pU8 = ((val>>24) & 0xFF);
	return val;
}
inline UINT16 VGetU16BitVal(UINT8 *pU8)
{
	return ( *pU8<<8 | *(pU8+1) );
}
inline UINT16 VSetU16BitVal(UINT8 *pU8, UINT16 val)
{
	*pU8 = ((val & 0xFF00)>>8);
	*(pU8+1) = val & 0x00FF;
	return val;
}
inline int VCmpU32BitVal(UINT8 *pU8_1, UINT8 *pU8_2)
{
	UINT8 i=0;
	for(i=0;i<4;i++)
	{
		if(pU8_1[i]!=pU8_2[i])
		{
			return(pU8_1[i]-pU8_2[i]);
		}
	}
	return 0;
}
inline int VCmpU16BitVal(UINT8 *pU8_1, UINT8 *pU8_2)
{
	return( (pU8_1[0]==pU8_2[0]) ? (pU8_1[1] - pU8_2[1]) : (pU8_1[0] - pU8_2[0]) );
}


UINT8 convertStr2BCDCode(void* pDigitNo, char *str);
UINT8 convertStr2DigitNO(void* pDigitNo, char *str);
UINT8 convertDigitNO2Str(void* pDigitNo, char *str);
bool isValidTelNO(char *szNumber, UINT8 maxNumberLen);
bool isValidDialedNO(char *szNumber, UINT8 maxNumberLen);

bool postComMsg(CComMessage* pComMsg);
bool sendNopayloadMsg(CComEntity* pEntity, TID dstTID, TID srcTID, UINT16 messageID, UINT32 eid=0xffffffff);

void setTestMsgU8(int offset, UINT8 data);
void setTestMsgU16(int offset, UINT16 data);
void setTestMsgU32(int offset, UINT32 data);
void setTestMsgStr(int offset, const char *data);
bool sendTestMsg(TID srcTID, TID dstTID, UINT16 msgID, UINT16 dataLength, UINT32 eid=0xFFFFFFFF);

void writeData2NvRam(char* dstAddr, char* srcAddr, UINT32 len);
char* jumpSpaces(char *pBuf, const long bufSize, long *pSpaceCharCount);
char* own_strtok_r(char* str, char* sep, char**ppNext);

#ifdef __cplusplus
}
#endif

#endif /* __VOICETOOLFUNC_H */


