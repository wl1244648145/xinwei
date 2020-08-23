/*******************************************************************************
* Copyright (c) 2009 by Beijing Beijing Arrowping Co.Ltd.All Rights Reserved   
* File Name      : voiceToolFunc.cpp
* Create Date    : 26-Jun-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#include "ComMessage.h"
#ifdef DSP_BIOS
#include "BtsLog.h"
#else
#include "log.h"
#endif
#include "VoiceCommon.h"
#include "string.h"
#include "localSagCommon.h"
#include "tVoice.h"
#include "voiceToolFunc.h"


////////////////////////////////////////////////////////////////////////////////
//RcvBuf/TcpRcvBuf/TcpEWithNatApRcvBuf---------begin
////////////////////////////////////////////////////////////////////////////////

#include "natpi.h"
#include "tcpE.h"

RcvBuf::RcvBuf()
{
	m_blShowInfo = false;
	m_blShowInfoDetail = false;
	
	pBuf = NULL;
	pBuf = new char[M_MAX_DATA_LEN*2];
	if(pBuf==NULL)
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
			"new buffer failed!!!");
	}
	resetRcvBuf();
}
RcvBuf::~RcvBuf()
{
	if(pBuf!=NULL)
	{
		delete [] pBuf;
	}
}
void RcvBuf::resetRcvBuf()
{
	pDataHead = pBuf;
	nDataLen = 0;
	seekFromHead(0);
}
void RcvBuf::moveRcvDataToHead()
{
	if(pDataHead!=pBuf)
	{
		if(pDataHead>=(pBuf+M_MAX_DATA_LEN) || getDataLen()<10)
		{
			memcpy(pBuf, pDataHead, nDataLen);
			pDataHead = pBuf;
			seekFromHead(0);
		}
	}
}
int RcvBuf::readFromBuf(char *dstBuf, UINT16 nLen)
{
	UINT32 nReadLen;
	UINT32 nDataLeft;
	if(dstBuf!=NULL)
	{
		nDataLeft = pDataHead+nDataLen-pCurPos;
		nReadLen = nLen<nDataLeft ? nLen : nDataLeft;
		memcpy(dstBuf, pCurPos, nReadLen);
		pCurPos+=nReadLen;
		return nReadLen;
	}
	else
	{
		return -1;
	}
}

int RcvBuf::getOneMsgOutofBuf(char ** ppBuf, UINT32 *pLen)
{
	return M_CONTINUE_RECV;
}

void RcvBuf::show()
{
	UINT32 i;
	if(m_blShowInfo)
	{
		VPRINT("\n pBuf[0x%08X] bufferLen[%d*2] pDataHead[0x%08X] \
nDataLen[%d] pCurPos[0x%08X]",
			(UINT32)pBuf, (int)M_MAX_DATA_LEN, (UINT32)pDataHead, 
			(int)nDataLen, (UINT32)pCurPos);
		if(pDataHead>(pBuf+M_MAX_DATA_LEN))
		{
			VPRINT("\nWarning!!! pDataHead>(pBuf+M_MAX_DATA_LEN  \n");
			return;
		}
		if(nDataLen>=M_MAX_DATA_LEN)
		{
			VPRINT("\nWarning!!! nDataLen>=M_MAX_DATA_LEN  \n");
			return;
		}
		if(pCurPos<pDataHead || pCurPos>(pDataHead+nDataLen))
		{
			VPRINT("\nWarning!!! pCurPos<pDataHead || \
pCurPos>(pDataHead+nDataLen)  \n");
			return;
		}
		if(m_blShowInfoDetail)
		{
			for(i=0;i<nDataLen;i++)
			{
				if(0==(i & 0x0F))
				{
					VPRINT("\n");
				}
				VPRINT(" %02X", (UINT8)pDataHead[i]);
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////
int TcpRcvBuf::recvData2Buf()
{
	if(NULL==pDataHead)
	{
		LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), "RcvBuffer is NULL.");
		return -1;
	}
	else
	{
		//how many bytes available
		int bytesAvailable = 0;
		int ret = Ioctl_Socket(m_fd, FIONREAD, &bytesAvailable);
		if(ret<0)	//error
		{
			LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_SOCKET_ERR), 
				"ioctl(m_fd, FIONREAD, &bytesAvailable) error!!!");
			OutputSocketErrCode("ioctl(m_fd, FIONREAD, &bytesAvailable)");
			return ret;
		}
		if(0==bytesAvailable)
		{
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
				"Peer closed tcp link.");
			return -1;
		}
		ret = recv(m_fd, pDataHead+nDataLen, M_MAX_DATA_LEN, 0);
		if(ret>=0)
		{
			nDataLen+=ret;
		}
		show();
		return ret;
	}
}

////////////////////////////////////////////////////////////////////////////////
int TcpEWithNatApRcvBuf::getOneMsgOutofBuf(char * * ppBuf, UINT32 * pLen)
{
	if(0==getDataLen())
	{
		return M_CONTINUE_RECV;
	}

	char *bufHead = pCurPos;
	char *pDataEnd = pDataHead + nDataLen -1;
	bool blHeadFound = false;
	const UINT8 HeadByte1 = (M_TCP_PKT_BEGIN_FLAG>>8) & 0xff;//0x7E;
	//找到头部
	while(bufHead<pDataEnd)
	{
		if( M_TCP_PKT_BEGIN_FLAG == VGetU16BitVal((UINT8*)bufHead) )
		{
			blHeadFound = true;
			break;
		}
		else
		{
			bufHead++;
		}
	}
	if(!blHeadFound)
	{
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), 
			"Cannot Find packet Header");
		//只有一个字节
		if(bufHead==pCurPos)
		{
			if(bufHead[0]==HeadByte1)
			{
				moveRcvDataToHead();
			}
			else
			{
				resetRcvBuf();
			}
		}
		//多于一个字节
		else
		{
			if(pDataEnd[0]==HeadByte1)
			{
				setDataHeadPtr(pDataEnd);
				setDataLen(1);
				seekFromHead(0);	
				moveRcvDataToHead();
			}
			else
			{
				resetRcvBuf();
			}		
		}		
		return M_CONTINUE_RECV;
	}

	//NATAP包，读取并验证
	char *bufNatPkt = bufHead+2;
	AH_T *pAHhead;
	UINT16 nNatApLen;

	//读出AHhead
	if(bufNatPkt + sizeof(AH_T)>=pDataEnd)
	{
		//消息不全
		setDataHeadPtr(bufHead);
		setDataLen(pDataEnd-bufHead+1);
		seekToPos(bufHead);
		moveRcvDataToHead();
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"NATAP msg not full.");
		return M_CONTINUE_RECV;
	}
	else
	{
		pAHhead = (AH_T*)bufNatPkt;
	}
	
	if(M_PROTOCOL_ID_NATAP!=pAHhead->protocolID)//protocol id error
	{
		setDataHeadPtr(bufNatPkt);
		setDataLen(pDataEnd-bufNatPkt+1);
		seekToPos(bufNatPkt);
		moveRcvDataToHead();
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), 
			"NATAP protocol ID error, invalid packet!!!");
		return M_CONTINUE_PARSE;
	}
	nNatApLen = VGetU16BitVal(pAHhead->nLen);
	if(nNatApLen<=sizeof(AH_T))	//len error
	{
		setDataHeadPtr(bufNatPkt);
		setDataLen(pDataEnd-bufNatPkt+1);
		seekToPos(bufNatPkt);
		moveRcvDataToHead();
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), 
			"NATAP AHhead length error, invalid packet!!!");
		return M_CONTINUE_PARSE;
	}
	if(pDataEnd-bufNatPkt+1<(int)(nNatApLen+sizeof(EndFlagT)))
	{
		//消息不全
		setDataHeadPtr(bufHead);
		setDataLen(pDataEnd-bufHead+1);
		seekToPos(bufHead);
		moveRcvDataToHead();
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_NORMAL), 
			"NATAP msg not full.");
		return M_CONTINUE_RECV;
	}
	//tcpEnd flag
	UINT8* pTcpEndFlag = (UINT8*)(bufNatPkt+nNatApLen);
	if(VGetU16BitVal(pTcpEndFlag)!=M_TCP_PKT_END_FLAG)
	{
		setDataHeadPtr(bufNatPkt);
		setDataLen(pDataEnd-bufNatPkt+1);
		seekToPos(bufNatPkt);
		moveRcvDataToHead();
		LOG(LOG_DEBUG3, LOGNO(VOICE, EC_L3VOICE_INVALID_SIGNAL), 
			"recv NATAP Data TCP ENDFLAG error!!!");
		return M_CONTINUE_PARSE;
	}		

	//合法信令
	*ppBuf = bufNatPkt;
	*pLen = nNatApLen;

	char *pDataHeadNext = (char*)(pTcpEndFlag+sizeof(EndFlagT));
	setDataHeadPtr(pDataHeadNext);
	setDataLen(pDataEnd-pDataHeadNext+1);
	seekToPos(pDataHeadNext);
	
	return M_GET_ONE_MSG;
}

int TcpEWithNatApRcvBuf::getOneMsgOutofBuf(CComMessage **ppMsg)
{
	int ret;
	char *bufPtr=NULL;
	UINT32 bufLen=0;
	CComMessage *pMsg = NULL;
	
	ret =  getOneMsgOutofBuf(&bufPtr, &bufLen);
	if(bufPtr!=NULL && bufLen>0)
	{
		pMsg = new (CTVoice::GetInstance(), bufLen) CComMessage;
		if(pMsg)
		{
			memcpy( (void*)pMsg->GetDataPtr(),
					(void*)bufPtr,
					bufLen );
			pMsg->SetDataLength(bufLen);			
		}
		else
		{
			LOG(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
				"new ComMessage error!!!");
		}
	}
	moveRcvDataToHead();

	*ppMsg = pMsg;
	show();
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
//RcvBuf/TcpRcvBuf/TcpEWithNatApRcvBuf---------end
////////////////////////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static UINT8 tmpTestMsg[M_DEFAULT_RESERVED+2048];
static UINT8 *pDataPtrTmpTestMsg = &tmpTestMsg[M_DEFAULT_RESERVED];

void setTestMsgU8(int offset, UINT8 data)
{
	V__ApAssertRtn(offset>=-64 && offset<2048, LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
		"-64<=offset<2048!!!", ;);
	pDataPtrTmpTestMsg[offset] = data;
}
void setTestMsgU16(int offset, UINT16 data)
{
	V__ApAssertRtn(offset>=-64 && offset<2046, LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
		"-64<=offset<2046!!!", ;);
	VSetU16BitVal(&pDataPtrTmpTestMsg[offset], data);
}
void setTestMsgU32(int offset, UINT32 data)
{
	V__ApAssertRtn(offset>=-64 && offset<2044, LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
		"-64<=offset<2044!!!", ;);
	VSetU32BitVal(&pDataPtrTmpTestMsg[offset], data);
}
void setTestMsgStr(int offset, const char *data)
{
	V__ApAssertRtn(offset>=-64 && offset<2047, LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
		"-64<=offset<2047!!!", ;);
	V__ApAssertRtn(data!=NULL, LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
		"str is NULL!!!", ;);
	V__ApAssertRtn(offset+strlen(data)<2047, LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), 
		"str too long!!!", ;);
	strcpy((char*)&pDataPtrTmpTestMsg[offset], data);
}
bool sendTestMsg(TID srcTID, TID dstTID, UINT16 msgID, UINT16 dataLength, UINT32 eid)
{
	bool ret=false;
	CComMessage *pComMsg = new (CTVoice::GetInstance(), dataLength) CComMessage;
	if(pComMsg!=NULL)
	{
		pComMsg->SetDstTid(dstTID);
		pComMsg->SetSrcTid(srcTID);
		pComMsg->SetDataLength(dataLength);
		pComMsg->SetMessageId(msgID);
		pComMsg->SetEID(eid);
		memcpy((void*)pComMsg->GetBufferPtr(), 
				(void*)tmpTestMsg, 
				M_DEFAULT_RESERVED+dataLength);
		ret = CComEntity::PostEntityMessage(pComMsg);
		if(!ret)
		{
			VPRINT("\n sendTestMsg failed!MessageID[0x%04X] srcTID[%d] dstTID[%d] \n",
				pComMsg->GetMessageId(), pComMsg->GetSrcTid(), pComMsg->GetDstTid());
			pComMsg->Destroy();
		}        
	}
	else
	{
		VPRINT("\n Cannot new CComMessage!!!!!! \n");
	}
	return ret;
}

bool sendNopayloadMsg(CComEntity* pEntity, TID dstTID, TID srcTID, UINT16 messageID, UINT32 eid)
{
	bool ret=false;
	CComMessage *pComMsg = new (pEntity, 0) CComMessage;
	if(pComMsg!=NULL)
	{
		pComMsg->SetDstTid(dstTID);
		pComMsg->SetSrcTid(srcTID);
		pComMsg->SetDataLength(0);
		pComMsg->SetMessageId(messageID);
		pComMsg->SetEID(eid);
		ret = CComEntity::PostEntityMessage(pComMsg);
		if(!ret)
		{
			LOG3(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_MSG_SND_FAIL), 
				"sendNopayloadMsg failed!MessageID[0x%04X] srcTID[%d] dstTID[%d]",
				pComMsg->GetMessageId(), 
				pComMsg->GetSrcTid(), pComMsg->GetDstTid());
			pComMsg->Destroy();
		}        
	}
	else
	{
		LOG(LOG_CRITICAL, LOGNO(VOICE, EC_L3VOICE_SYS_FAIL), "Cannot new CComMessage!!!!!!");
	}
	return ret;
}

#define M_TMP_MAX_NUMBERLEN (M_MAX_PHONE_NUMBER_LEN)
typedef struct _tmpDigitNoT
{
	UINT8 type;
	UINT8 len;
	UINT8 number[M_TMP_MAX_NUMBERLEN/2];
}tmpDigitNoT;
UINT8 convertStr2BCDCode(void* pDigitNo, char *str)
{
	UINT8 i, nStrLen;
	UINT8 tmpDtmfCode=0x0F;
	char tmpStr[M_TMP_MAX_NUMBERLEN+1];
	for(i=0;i<M_TMP_MAX_NUMBERLEN;i++)
	{
		if(str[i])
			tmpStr[i]=str[i];
		else
			break;
	}
	tmpStr[i]=0;
	nStrLen = strlen(tmpStr);
	UINT8* pNO = (UINT8*)pDigitNo;
	UINT8 ret = (nStrLen+1)/2;
	for(i=0;i<nStrLen;i++)
	{
		switch(tmpStr[i])
		{
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				tmpDtmfCode = tmpStr[i]-'0';
				break;
			case '0':
				tmpDtmfCode = 0x0A;
				break;
			case '*':
				tmpDtmfCode = 0x0B;
				break;
			case '#':
				tmpDtmfCode = 0x0C;
				break;
			default:
				tmpDtmfCode = 0x0F;
				break;
		}
		if(i&0x1)
		{
			//high 4 bits
			pNO[i/2] |= (tmpDtmfCode<<4);
		}
		else
		{
			//low 4 bits
			pNO[i/2] = tmpDtmfCode;
		}
	}
	if(nStrLen&0x1)
	{
		//high 4 bits should be 0xf when the length of telNo is odd
		pNO[i/2] |= 0xF0;
	}	
	return(ret);
}
UINT8 convertStr2DigitNO(void* pDigitNo, char *str)
{
	UINT8 i, nStrLen;
	UINT8 tmpDtmfCode=0x0F;
	char tmpStr[M_TMP_MAX_NUMBERLEN+1];
	for(i=0;i<M_TMP_MAX_NUMBERLEN;i++)
	{
		if(str[i])
			tmpStr[i]=str[i];
		else
			break;
	}
	tmpStr[i]=0;
	nStrLen = strlen(tmpStr);
	tmpDigitNoT* pNO = (tmpDigitNoT*)pDigitNo;
	pNO->type = 0x01;
	pNO->len = (nStrLen+1)/2;
	for(i=0;i<nStrLen;i++)
	{
		switch(tmpStr[i])
		{
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				tmpDtmfCode = tmpStr[i]-'0';
				break;
			case '0':
				tmpDtmfCode = 0x0A;
				break;
			case '*':
				tmpDtmfCode = 0x0B;
				break;
			case '#':
				tmpDtmfCode = 0x0C;
				break;
			default:
				tmpDtmfCode = 0x0F;
				break;
		}
		if(i&0x1)
		{
			//high 4 bits
			pNO->number[i/2] |= (tmpDtmfCode<<4);
		}
		else
		{
			//low 4 bits
			pNO->number[i/2] = tmpDtmfCode;
		}
	}
	if(nStrLen&0x1)
	{
		//high 4 bits should be 0xf when the length of telNo is odd
		pNO->number[i/2] |= 0xF0;
	}
	return(pNO->len+2);
}
UINT8 convertDigitNO2Str(void* pDigitNo, char *str)
{
	UINT8 i, j, tmpDtmfCode;
	char dictionary[]={'\0','1','2','3','4','5','6','7','8','9','0','*','#','\0','\0','\0'};
	tmpDigitNoT* pNO = (tmpDigitNoT*)pDigitNo;
	UINT8 nLen = pNO->len>M_TMP_MAX_NUMBERLEN/2 ? M_TMP_MAX_NUMBERLEN/2 : pNO->len;
	for(i=0,j=0;i<nLen;i++)
	{
		tmpDtmfCode = pNO->number[i] & 0x0F;
		str[j++] = tmpDtmfCode==0x0F ? 0x00 : dictionary[tmpDtmfCode];
		tmpDtmfCode = (pNO->number[i] & 0xF0)>>4;	
		str[j++] = tmpDtmfCode==0x0F ? 0x00 : dictionary[tmpDtmfCode];
	}
	str[j]=0;
	return j;
}

bool isValidTelNO(char *szNumber, UINT8 maxNumberLen)
{
	bool ret = false;
	UINT32 i, tmpLen;
	if(szNumber!=NULL)
	{
		tmpLen = strlen(szNumber);
		if(tmpLen<=maxNumberLen)
		{
			for(i=0;i<tmpLen;i++)
			{
				if(szNumber[i]<'0' || szNumber[i]>'9')
				{
					return false;
				}
			}
			return true;
		}
	}
	return ret;
}

bool isValidDialedNO(char *szNumber, UINT8 maxNumberLen)
{
	bool ret = false;
	UINT32 i, tmpLen;
	if(szNumber!=NULL)
	{
		tmpLen = strlen(szNumber);
		if(tmpLen<=maxNumberLen)
		{
			for(i=0;i<tmpLen;i++)
			{
				if( (szNumber[i]<'0' || szNumber[i]>'9') &&
					szNumber[i]!='*' && szNumber[i]!='#' )
				{
					return false;
				}
			}
			return true;
		}
	}
	return ret;	
}

bool postComMsg(CComMessage* pComMsg)
{
	bool ret;
	if(pComMsg!=NULL)
	{
		ret = CComEntity::PostEntityMessage(pComMsg);
		if(!ret)
		{
			LOG3(LOG_WARN, LOGNO(VOICE, EC_L3VOICE_MSG_SND_FAIL), 
				"postComMsg failed!MessageID[0x%04X] srcTID[%d] dstTID[%d]",
				pComMsg->GetMessageId(), 
				pComMsg->GetSrcTid(), pComMsg->GetDstTid());
			pComMsg->Destroy();
		}
		return ret;		
	}
	else
	{
		LOG(LOG_SEVERE, LOGNO(VOICE, EC_L3VOICE_MSG_SND_FAIL), "postComMsg failed!NULL ComMsg!!!");
		return false;
	}
}

//返回pBuf中第一个不是空格回车换行制表字符的指针
//pSpaceCharCount指向的整数存储跳过的空格类字符的个数
char* jumpSpaces(char *pBuf, const long bufSize, long *pSpaceCharCount)
{
	char *pTmp = pBuf;
	char *pMargin = pBuf+bufSize;
	while( (*pTmp=='\n' || *pTmp=='\r' || *pTmp==' ' || *pTmp=='\t') && 
		(pTmp<pMargin && 0!=*pTmp) )
	{
		pTmp++;
	}
	*pSpaceCharCount = (pTmp-pBuf);
	return pTmp;
}

char* own_strtok_r(char* str, char* sep, char**ppNext)
{
	if ((str == NULL) && ((str = *ppNext) == NULL))
		return (NULL);

	if (*(str += strspn (str, sep)) == '\0')
		return (*ppNext = NULL);

	if ((*ppNext = strpbrk (str, sep)) != NULL)
		*(*ppNext)++ =  '\0';

	return (str);
}
bool ValGuard::checkIfValChanged()
{
	bool blChanged = (m_lastVal!=m_val);
	m_lastVal = m_val;
	m_unchangedTimes = blChanged ? 0 : (m_unchangedTimes+1);
	return blChanged;
}
UINT32 ValGuard::getTimeUnchanged()
{
	return(m_unchangedTimes*m_checkTimeInterval);
}

