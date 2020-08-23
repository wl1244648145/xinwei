/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMUPDATECPESWREQ
#include "L3OamUpDateCpeSWReq.h"
#endif
#include <string.h>
CUpdateCpeSWReq :: CUpdateCpeSWReq(CMessage &rMsg)
    :CMessage(rMsg)
{  }

CUpdateCpeSWReq :: CUpdateCpeSWReq()
{
}
bool CUpdateCpeSWReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_UPGRADE_UT_SW_REQ_NEW);
	return true;
}



UINT32 CUpdateCpeSWReq :: GetDefaultDataLen() const
{
    return sizeof(T_UpdateUTSWReq);
}

UINT16 CUpdateCpeSWReq :: GetTransactionId() const
{
    return ((T_UpdateUTSWReq *)GetDataPtr())->TransId;
}

UINT16 CUpdateCpeSWReq :: SetTransactionId(UINT16 TransId)
{
    ((T_UpdateUTSWReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CUpdateCpeSWReq :: GetCPEID()
{
    return ((T_UpdateUTSWReq *)GetDataPtr())->m_CPEID;
}

void CUpdateCpeSWReq :: SetCPEID(UINT32 T)
{
    ((T_UpdateUTSWReq *)GetDataPtr())->m_CPEID = T;
}

UINT8  CUpdateCpeSWReq :: GetModelType() const
{
    return ((T_UpdateUTSWReq *)GetDataPtr())->m_ModelType;
}

void   CUpdateCpeSWReq :: SetModelType(UINT8 T)
{
    ((T_UpdateUTSWReq *)GetDataPtr())->m_ModelType = T;
}

UINT8  CUpdateCpeSWReq :: GetSWType() const
{
    return ((T_UpdateUTSWReq *)GetDataPtr())->m_SWType;
}

void   CUpdateCpeSWReq :: SetSWType(UINT8 T)
{
    ((T_UpdateUTSWReq *)GetDataPtr())->m_SWType = T;
}

bool CUpdateCpeSWReq :: GetSWVer(SINT8* SWVer, UINT8 Len) const
{
    if(NULL == SWVer)
    {
        return false;
    }
    else
    {
        memcpy(SWVer, ((T_UpdateUTSWReq*)GetDataPtr())->m_SWVer, Len);
        return true;
    }    
}

bool CUpdateCpeSWReq :: SetSWVer(SINT8* SWVer, UINT8 Len)
{
    if(NULL == SWVer)
    {
        return false;
    }
    else
    {
        memcpy(((T_UpdateUTSWReq*)GetDataPtr())->m_SWVer, SWVer, Len);
        return true;
    }    
}
//test start
//#include "stdio.h"
//test stop
bool CUpdateCpeSWReq :: GetFileName(SINT8* Name) const
{
    if(!Name)
    {
        return false;
    }

    SINT8 NameBuff[FILE_NAME_LEN];
    memset(NameBuff, 0, sizeof(NameBuff));
    UINT8 ModType = GetModelType();
    //UINT8 SWType  = GetSWType();
    UINT8 hwtype1 = (ModType & 0xf0)>>4;
    UINT8 hwtype2 = (ModType & 0x0f);
    //test start
    //printf("\r\nModType:0x%x,   hwtype1:%d,   hwtype2:%d   \r\n",ModType,hwtype1,hwtype2);
    //test stop
    if(0 == hwtype1)
    {
        if(1 == hwtype2)
        {
            strcpy(NameBuff, "hs.om.");
        }
        else if(2 == hwtype2)
        {
            strcpy(NameBuff, "pcmcia.om.");
        }
        else if(3 == hwtype2)
        {
            strcpy(NameBuff, "cpe.om.");
        }
        else if(4 == hwtype2)
        {
            strcpy(NameBuff, "cpe.bm.");
        }        
        else
        {
            return false;
        }
    }

    if(1 == hwtype1)
    {
        if(1 == hwtype2)
        {
            strcpy(NameBuff, "hs.or.");
        }
        else if(2 == hwtype2)
        {
            strcpy(NameBuff, "pcmcia.or.");
        }
        else if(3 == hwtype2)
        {
            strcpy(NameBuff, "cpe.or.");
        }
        else if(4 == hwtype2)
        {
            strcpy(NameBuff, "cpe.br.");
        }        
        else
        {
            return false;
        }
    }
    
    if(8 == hwtype1)
    {
        if(3 == hwtype2)
        {
            strcpy(NameBuff, "cpez.om.");
        }
        else if(4 == hwtype2)
        {
            strcpy(NameBuff, "cpez.bm.");
        }
        else
        {
            return false;
        }
    }

    if(9 == hwtype1)
    {
        if(3 == hwtype2)
        {
            strcpy(NameBuff, "cpez.or.");
        }
        else if(4 == hwtype2)
        {
            strcpy(NameBuff, "cpez.br.");
        }
        else
        {
            return false;
        }
    }

    SINT8 VerBuff[FILE_NAME_LEN];
    memset(VerBuff, 0, sizeof(VerBuff));
    GetSWVer(VerBuff);
    strcat(NameBuff, VerBuff);
    strcat(NameBuff, ".bin");
    
    memcpy(Name, NameBuff, strlen(NameBuff));
//test start
//printf("\r\nName:%s\r\n",Name);
//test stop
    return true;
}


CUpdateCpeSWReq :: ~CUpdateCpeSWReq()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
CUpdateCpeSWReqNew :: CUpdateCpeSWReqNew(CMessage &rMsg)
    :CMessage(rMsg)
{  }

CUpdateCpeSWReqNew :: CUpdateCpeSWReqNew()
{
}
bool CUpdateCpeSWReqNew :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_UPGRADE_UT_SW_REQ_NEW);
	return true;
}

UINT32 CUpdateCpeSWReqNew :: GetDefaultDataLen() const
{
    return sizeof(T_UpdateUTSWReqNew);
}

UINT16 CUpdateCpeSWReqNew :: GetTransactionId() const
{
    return ((T_UpdateUTSWReqNew *)GetDataPtr())->TransId;
}

UINT16 CUpdateCpeSWReqNew :: SetTransactionId(UINT16 TransId)
{
    ((T_UpdateUTSWReqNew *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT32 CUpdateCpeSWReqNew :: GetCPEID()
{
    return ((T_UpdateUTSWReqNew *)GetDataPtr())->m_CPEID;
}

void CUpdateCpeSWReqNew :: SetCPEID(UINT32 T)
{
    ((T_UpdateUTSWReqNew *)GetDataPtr())->m_CPEID = T;
}

UINT8 CUpdateCpeSWReqNew :: GetFileNameLen()
{
    return ((T_UpdateUTSWReqNew *)GetDataPtr())->m_FileNameLen;
}

void CUpdateCpeSWReqNew :: SetFileNameLen(UINT8 len)
{
    ((T_UpdateUTSWReqNew*)GetDataPtr())->m_FileNameLen = len;
}

bool  CUpdateCpeSWReqNew :: GetFileName(SINT8 *FileName, UINT8 Len) const
{
    if(NULL == FileName)
    {
        return false;
    }
    else
    {
        memcpy(FileName, ((T_UpdateUTSWReqNew*)GetDataPtr())->m_FileName, Len);
        return true;
    }  
}
UINT8  CUpdateCpeSWReqNew :: GetHWtype()
{
    UINT8* puc;
    puc = (UINT8*)&(((T_UpdateUTSWReqNew*)GetDataPtr())->m_FileNameLen);
    UINT8 uc = *puc;
    puc = puc + 1 + uc;
    return *puc;
}
UINT32  CUpdateCpeSWReqNew :: getPid()
{
    UINT8* puc;
    puc = (UINT8*)&(((T_UpdateUTSWReqNew*)GetDataPtr())->m_FileNameLen);
    UINT8 uc = *puc;
    puc = puc + 1 + uc +1;
    return *(UINT32*)puc;
}
CUpdateCpeSWReqNew :: ~CUpdateCpeSWReqNew()
{
}
#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////             For Z Software Update           //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CUpdateZSWReq :: CUpdateZSWReq(CMessage &rMsg)    :CMessage(rMsg){  }
CUpdateZSWReq :: CUpdateZSWReq(){}
CUpdateZSWReq :: ~CUpdateZSWReq(){}

bool CUpdateZSWReq :: CreateMessage(CComEntity& Entity)
{
	CMessage :: CreateMessage(Entity);
	SetMessageId(M_EMS_BTS_UPGRADE_UT_SW_REQ);
	return true;
}

UINT32 CUpdateZSWReq :: GetDefaultDataLen() const{ return sizeof(T_UpdateZSWReq);}
UINT16 CUpdateZSWReq :: GetTransactionId() const { return ((T_UpdateZSWReq*)GetDataPtr())->TransId;}
UINT16 CUpdateZSWReq :: GetUpdateType()	const		 { return ((T_UpdateZSWReq*)GetDataPtr())->m_UpdateType; }   
UINT32 CUpdateZSWReq :: GetCPEID()				 { return ((T_UpdateZSWReq*)GetDataPtr())->m_CPEID;}
UINT32 CUpdateZSWReq :: GetUid()				 { return ((T_UpdateZSWReq*)GetDataPtr())->m_uid;};       
bool   CUpdateZSWReq :: GetSWVer(SINT8* SWVer, UINT8 Len) const
{
	if(NULL == SWVer)
	{
		return false;
	}
	else
	{
		memcpy(SWVer, ((T_UpdateZSWReq*)GetDataPtr())->m_SWVer, Len);
		return true;
	}    
}
bool CUpdateZSWReq :: GetZProductType(SINT8* ZProductType, UINT8 Len) const                                                           
{                                                                     
	if(NULL == ZProductType)                                                 
	{                                                                 
		return false;                                                 
	}                                                                 
	else                                                              
	{                                                                 
		memcpy(ZProductType, ((T_UpdateZSWReq*)GetDataPtr())->m_ZProductType, Len);
		return true;                                                  
	}
}
bool CUpdateZSWReq :: GetFileName(SINT8* Name) const
{
	if( NULL == Name )
	{
		return false;
	}

	memset(Name, 0, sizeof(FILE_NAME_LEN));
	strcpy(Name, "MZ.");
	strcat(Name, (char*)((T_UpdateZSWReq*)GetDataPtr())->m_ZProductType );
	strcat(Name, "."); 

	SINT8 VerBuff[FILE_NAME_LEN];
	memset(VerBuff, 0, sizeof(VerBuff));
	GetSWVer(VerBuff);
	strcat(Name, VerBuff);
	strcat(Name, ".BIN");
	return true;                                          
} 
#endif
