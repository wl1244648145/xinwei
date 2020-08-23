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
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMCPESWDLOADREQ
#include "L3OamCPESWDLoadReq.h"
#endif
#include <string.h>

CCpeSWDLoadReq :: CCpeSWDLoadReq(CMessage &rMsg)
:CMessage(rMsg)
{
     
}

CCpeSWDLoadReq:: CCpeSWDLoadReq()
{

}

bool CCpeSWDLoadReq:: CreateMessage(CComEntity& ComEntity)
{
    CMessage :: CreateMessage(ComEntity);
    SetMessageId(M_EMS_BTS_DL_UT_SW_REQ_NEW);

    return true;
}

UINT32 CCpeSWDLoadReq :: GetDefaultDataLen() const
{
    return sizeof(T_DLCpeSWReq);
}

UINT16 CCpeSWDLoadReq :: GetTransactionId() const
{
    return ((T_DLCpeSWReq*)GetDataPtr())->m_TransId;
}

UINT16 CCpeSWDLoadReq :: SetTransactionId(UINT16 TransId)
{
    ((T_DLCpeSWReq*)GetDataPtr())->m_TransId = TransId;
	return 0;
}

SINT32 CCpeSWDLoadReq :: GetFtpServerIp()const
{
    return ((T_DLCpeSWReq *)GetDataPtr())->m_FTPserverIP;
}

void CCpeSWDLoadReq :: SetFtpServerIp(SINT32 FTPserverIP)
{
    ((T_DLCpeSWReq*)GetDataPtr())->m_FTPserverIP = FTPserverIP;
}

UINT16 CCpeSWDLoadReq :: GetFtpServerPort()const
{
    return ((T_DLCpeSWReq *)GetDataPtr())->m_FTPserverPort;
}

void CCpeSWDLoadReq :: SetFtpServerPort(UINT16 FTPserverPort)
{
    ((T_DLCpeSWReq*)GetDataPtr())->m_FTPserverPort = FTPserverPort;
}

UINT8 CCpeSWDLoadReq :: GetUserNameLen()const
{
    return ((T_DLCpeSWReq*)GetDataPtr())->m_UserNameLen;
}

void CCpeSWDLoadReq :: SetUserNameLen(UINT8 UserNameLen)
{
    ((T_DLCpeSWReq*)GetDataPtr())->m_UserNameLen = UserNameLen;
}

bool CCpeSWDLoadReq :: GetUserName(SINT8* UserName, UINT8 Len) const
{
    if(NULL != UserName)
    {
        memcpy(UserName, ((T_DLCpeSWReq*)GetDataPtr())->m_UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool CCpeSWDLoadReq :: SetUserName(SINT8 *UserName, UINT8 Len)
{
    if(NULL != UserName)
    {
        memcpy(((T_DLCpeSWReq*)GetDataPtr())->m_UserName, UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

//一下计算全以 OFFSET_USERNAME 为基准，加的1字节是长度字段占的空间
UINT8 CCpeSWDLoadReq :: GetFtpPassLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void CCpeSWDLoadReq :: SetFtpPassLen(UINT8 FTPPasswordLen)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = FTPPasswordLen;
}

bool CCpeSWDLoadReq :: GetFtpPass(SINT8 *FTPPassword, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1;
    if(NULL != FTPPassword)
    {
        memcpy(FTPPassword, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}


bool CCpeSWDLoadReq :: SetFtpPass(SINT8 *FTPPassword, UINT8 Len)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1;
    if(NULL != FTPPassword)
    {
        memcpy(((SINT8*)GetDataPtr() + OffSet), FTPPassword, Len);
        return true;
    }
    else
    {
        return false;
    }
}

UINT8 CCpeSWDLoadReq :: GetFtpDirLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void CCpeSWDLoadReq :: SetFtpDirLen(UINT8 FileDirLen)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = FileDirLen;
}

bool  CCpeSWDLoadReq :: GetFtpDir(SINT8 *FileDirectory, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1;
    if(NULL != FileDirectory)
    {
        memcpy(FileDirectory, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool   CCpeSWDLoadReq :: SetFtpDir(SINT8 *FileDirectory, UINT8 Len)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1;
    if(NULL != FileDirectory)
    {
        memcpy(((SINT8*)GetDataPtr() + OffSet), FileDirectory, Len);
        return true;
    }
    else
    {
        return false;
    }
}

UINT8  CCpeSWDLoadReq :: GetModelType() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void   CCpeSWDLoadReq :: SetModelType(UINT8 T)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = T;
}


UINT8  CCpeSWDLoadReq :: GetSWType() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen()
                   + 1;
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void  CCpeSWDLoadReq :: SetSWType(UINT8 T)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen()
                   + 1;
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = T;
}

bool  CCpeSWDLoadReq :: GetSWVer(SINT8* V, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen()
                   + 1
                   + 1;
    if(NULL != V)
    {
        memcpy(V, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool  CCpeSWDLoadReq :: SetSWVer(SINT8* V, UINT8 Len)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen()
                   + 1
                   + 1;
    if(NULL != V)
    {
        memcpy(((SINT8*)GetDataPtr() + OffSet), V, Len);
        return true;
    }
    else
    {
        return false;
    }
}

//test start
//#include "stdio.h"
//test stop
bool CCpeSWDLoadReq :: GetFileName(SINT8* Name) const
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
//printf("\r\n ModType:0x%x    NameBuff:%d    VerBuff:%d  \r\n",ModType,hwtype1,hwtype2);
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
//test start
//printf("\r\n NameBuff:%s    VerBuff:%s  \r\n",NameBuff,VerBuff);
//test stop
    strcat(NameBuff, VerBuff);
    strcat(NameBuff, ".bin");
    
    memcpy(Name, NameBuff, strlen(NameBuff));

    return true;
}

CCpeSWDLoadReq :: ~CCpeSWDLoadReq()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CCpeSWDLoadReqNew :: CCpeSWDLoadReqNew(CMessage &rMsg)
:CMessage(rMsg)
{
}

CCpeSWDLoadReqNew:: CCpeSWDLoadReqNew()
{
}

CCpeSWDLoadReqNew :: ~CCpeSWDLoadReqNew()
{
}

bool CCpeSWDLoadReqNew:: CreateMessage(CComEntity& ComEntity)
{
    CMessage :: CreateMessage(ComEntity);
    SetMessageId(M_EMS_BTS_DL_UT_SW_REQ_NEW);

    return true;
}

UINT32 CCpeSWDLoadReqNew :: GetDefaultDataLen() const
{
    return sizeof(T_DLCpeSWReqNew);
}

UINT16 CCpeSWDLoadReqNew :: GetTransactionId() const
{
    return ((T_DLCpeSWReqNew*)GetDataPtr())->m_TransId;
}

UINT16 CCpeSWDLoadReqNew :: SetTransactionId(UINT16 TransId)
{
    ((T_DLCpeSWReqNew*)GetDataPtr())->m_TransId = TransId;
    return 0;
}

SINT32 CCpeSWDLoadReqNew :: GetFtpServerIp()const
{
    return ((T_DLCpeSWReqNew *)GetDataPtr())->m_FTPserverIP;
}

void CCpeSWDLoadReqNew :: SetFtpServerIp(SINT32 FTPserverIP)
{
    ((T_DLCpeSWReqNew*)GetDataPtr())->m_FTPserverIP = FTPserverIP;
}

UINT16 CCpeSWDLoadReqNew :: GetFtpServerPort()const
{
    return ((T_DLCpeSWReqNew *)GetDataPtr())->m_FTPserverPort;
}

void CCpeSWDLoadReqNew :: SetFtpServerPort(UINT16 FTPserverPort)
{
    ((T_DLCpeSWReqNew*)GetDataPtr())->m_FTPserverPort = FTPserverPort;
}

UINT8 CCpeSWDLoadReqNew :: GetUserNameLen()const
{
    return ((T_DLCpeSWReqNew*)GetDataPtr())->m_UserNameLen;
}

void CCpeSWDLoadReqNew :: SetUserNameLen(UINT8 UserNameLen)
{
    ((T_DLCpeSWReqNew*)GetDataPtr())->m_UserNameLen = UserNameLen;
}

bool CCpeSWDLoadReqNew :: GetUserName(SINT8* UserName, UINT8 Len) const
{
    if(NULL != UserName)
    {
        memcpy(UserName, ((T_DLCpeSWReqNew*)GetDataPtr())->m_UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool CCpeSWDLoadReqNew :: SetUserName(SINT8 *UserName, UINT8 Len)
{
    if(NULL != UserName)
    {
        memcpy(((T_DLCpeSWReqNew*)GetDataPtr())->m_UserName, UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

//一下计算全以 OFFSET_USERNAME 为基准，加的1字节是长度字段占的空间
UINT8 CCpeSWDLoadReqNew :: GetFtpPassLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void CCpeSWDLoadReqNew :: SetFtpPassLen(UINT8 FTPPasswordLen)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = FTPPasswordLen;
}

bool CCpeSWDLoadReqNew :: GetFtpPass(SINT8 *FTPPassword, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1;
    if(NULL != FTPPassword)
    {
        memcpy(FTPPassword, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}


bool CCpeSWDLoadReqNew :: SetFtpPass(SINT8 *FTPPassword, UINT8 Len)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1;
    if(NULL != FTPPassword)
    {
        memcpy(((SINT8*)GetDataPtr() + OffSet), FTPPassword, Len);
        return true;
    }
    else
    {
        return false;
    }
}

UINT8 CCpeSWDLoadReqNew :: GetFtpDirLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void CCpeSWDLoadReqNew :: SetFtpDirLen(UINT8 FileDirLen)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = FileDirLen;
}

bool  CCpeSWDLoadReqNew :: GetFtpDir(SINT8 *FileDirectory, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1;
    if(NULL != FileDirectory)
    {
        memcpy(FileDirectory, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool   CCpeSWDLoadReqNew :: SetFtpDir(SINT8 *FileDirectory, UINT8 Len)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1;
    if(NULL != FileDirectory)
    {
        memcpy(((SINT8*)GetDataPtr() + OffSet), FileDirectory, Len);
        return true;
    }
    else
    {
        return false;
    }
}
////////////////////////////////

UINT8 CCpeSWDLoadReqNew :: GetFileNameLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
	               + 1
				   + GetFtpDirLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void CCpeSWDLoadReqNew :: SetFileNameLen(UINT8 FileNameLen)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = FileNameLen;
}

bool  CCpeSWDLoadReqNew :: GetFileName(SINT8 *FileName, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
	               + GetFtpDirLen()
	               + 1;
    if(NULL != FileName)
    {
        memcpy(FileName, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool   CCpeSWDLoadReqNew :: SetFileName(SINT8 *FileName, UINT8 Len)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
	               + GetFtpDirLen()
	               + 1;
    if(NULL != FileName)
    {
        memcpy(((SINT8*)GetDataPtr() + OffSet), FileName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////             For Z Software Update           //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CZSWDLoadReq :: CZSWDLoadReq(CMessage &rMsg):CMessage(rMsg){}
CZSWDLoadReq:: CZSWDLoadReq(){}
CZSWDLoadReq :: ~CZSWDLoadReq(){}

bool CZSWDLoadReq:: CreateMessage(CComEntity& ComEntity)
{
    CMessage :: CreateMessage(ComEntity);
    SetMessageId(M_EMS_BTS_DL_UT_SW_REQ);
    return true;
}

UINT32 CZSWDLoadReq :: GetDefaultDataLen() const{return sizeof(T_DLZSWReq);}
UINT16 CZSWDLoadReq :: GetTransactionId() const{return ((T_DLZSWReq*)GetDataPtr())->m_TransId;}
SINT32 CZSWDLoadReq :: GetFtpServerIp()const{return ((T_DLZSWReq *)GetDataPtr())->m_FTPserverIP;}
UINT16 CZSWDLoadReq :: GetFtpServerPort()const{return ((T_DLZSWReq *)GetDataPtr())->m_FTPserverPort;}
UINT8 CZSWDLoadReq :: GetUserNameLen()const{    return ((T_DLZSWReq*)GetDataPtr())->m_UserNameLen;}
bool CZSWDLoadReq :: GetUserName(SINT8* UserName, UINT8 Len) const
{
    if(NULL != UserName)
    {
        memcpy(UserName, ((T_DLZSWReq*)GetDataPtr())->m_UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

//一下计算全以 OFFSET_USERNAME 为基准，加的1字节是长度字段占的空间
UINT8 CZSWDLoadReq :: GetFtpPassLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

bool CZSWDLoadReq :: GetFtpPass(SINT8 *FTPPassword, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1;
    if(NULL != FTPPassword)
    {
        memcpy(FTPPassword, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}

UINT8 CZSWDLoadReq :: GetFtpDirLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}


bool  CZSWDLoadReq :: GetFtpDir(SINT8 *FileDirectory, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1;
    if(NULL != FileDirectory)
    {
        memcpy(FileDirectory, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}
bool  CZSWDLoadReq :: GetSWVer(SINT8* V, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen();
    if(NULL != V)
    {
        memcpy(V, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}
bool  CZSWDLoadReq :: GetSWProductType(SINT8* V, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen()
                   + FILEVER_STR_LEN;
    if(NULL != V)
    {
        memcpy(V, ((SINT8*)GetDataPtr() + OffSet), Len);
        return true;
    }
    else
    {
        return false;
    }
}
bool CZSWDLoadReq :: GetFileName(SINT8* Name) const
{
	if(!Name)
		return false;

	memset(Name, 0, FILE_NAME_LEN );
	strcpy(Name, "MZ.");
	
	SINT8 VerBuff[FILE_NAME_LEN];
	memset(VerBuff, 0, sizeof(VerBuff));
	GetSWProductType(VerBuff);
	strcat(Name, VerBuff);
	strcat(Name, ".");
	
	memset(VerBuff, 0, sizeof(VerBuff));
	GetSWVer(VerBuff);
	strcat(Name, VerBuff);
	strcat(Name, ".BIN");
}
#endif
