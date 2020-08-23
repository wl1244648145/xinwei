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
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMBTSSWDLOADREQ
#include "L3OamBtsSWDLoadReq.h"
#endif

#include <string.h>
CBtsSWDLoadReq :: CBtsSWDLoadReq(CMessage &rMsg)
:CMessage(rMsg)
{
     
}

CBtsSWDLoadReq:: CBtsSWDLoadReq()
{

}

bool CBtsSWDLoadReq:: CreateMessage(CComEntity& ComEntity)
{
    CMessage :: CreateMessage(ComEntity);
    SetMessageId(M_EMS_BTS_DL_BTS_SW_REQ);

    return true;
}

UINT32 CBtsSWDLoadReq :: GetDefaultDataLen() const
{
    return sizeof(T_DLBtsSWReq);
}

UINT16 CBtsSWDLoadReq :: GetTransactionId() const
{
    return ((T_DLBtsSWReq*)GetDataPtr())->m_TransId;
}

UINT16 CBtsSWDLoadReq :: SetTransactionId(UINT16 TransId)
{
    ((T_DLBtsSWReq*)GetDataPtr())->m_TransId = TransId;
	return 0;
}
    
SINT32 CBtsSWDLoadReq :: GetFtpServerIp() const
{
    return ((T_DLBtsSWReq *)GetDataPtr())->m_FTPserverIP;
}

void CBtsSWDLoadReq :: SetFtpServerIp(SINT32 FTPserverIP)
{
   ((T_DLBtsSWReq*)GetDataPtr())->m_FTPserverIP = FTPserverIP;
}

SINT16 CBtsSWDLoadReq :: GetFtpServerPort() const
{
    return ((T_DLBtsSWReq *)GetDataPtr())->m_FTPserverPort;
}

void CBtsSWDLoadReq :: SetFtpServerPort(SINT16 FTPserverPort)
{
   ((T_DLBtsSWReq*)GetDataPtr())->m_FTPserverPort = FTPserverPort;
}

UINT8 CBtsSWDLoadReq :: GetUserNameLen() const
{
    return ((T_DLBtsSWReq*)GetDataPtr())->m_UserNameLen;
}

void CBtsSWDLoadReq :: SetUserNameLen(UINT8 UserNameLen)
{
    ((T_DLBtsSWReq*)GetDataPtr())->m_UserNameLen = UserNameLen;
}

bool CBtsSWDLoadReq :: GetUserName(SINT8* UserName, UINT8 Len) const
{
    if(NULL != UserName)
    {
        memcpy(UserName, ((T_DLBtsSWReq*)GetDataPtr())->m_UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool CBtsSWDLoadReq :: SetUserName(SINT8 *UserName, UINT8 Len)
{
    if(NULL != UserName)
    {
        memcpy(((T_DLBtsSWReq*)GetDataPtr())->m_UserName, UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

//一下计算全以 OFFSET_USERNAME 为基准，加的1字节是长度字段占的空间
UINT8 CBtsSWDLoadReq :: GetFtpPassLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void CBtsSWDLoadReq :: SetFtpPassLen(UINT8 FTPPasswordLen)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = FTPPasswordLen;
}

bool CBtsSWDLoadReq :: GetFtpPass(SINT8 *FTPPassword, UINT8 Len) const
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


bool CBtsSWDLoadReq :: SetFtpPass(SINT8 *FTPPassword, UINT8 Len)
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

UINT8 CBtsSWDLoadReq :: GetFtpDirLen() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void CBtsSWDLoadReq :: SetFtpDirLen(UINT8 FileDirLen)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = FileDirLen;
}

bool  CBtsSWDLoadReq :: GetFtpDir(SINT8 *FileDirectory, UINT8 Len) const
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

bool   CBtsSWDLoadReq :: SetFtpDir(SINT8 *FileDirectory, UINT8 Len)
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

UINT8  CBtsSWDLoadReq :: GetSWType() const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen();
    return *(UINT8*)((SINT8*)GetDataPtr() + OffSet);
}

void  CBtsSWDLoadReq :: SetSWType(UINT8 T)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen();
    *(UINT8*)((SINT8*)GetDataPtr() + OffSet) = T;
}

bool  CBtsSWDLoadReq :: GetSWVer(SINT8* V, UINT8 Len) const
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen()
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

bool  CBtsSWDLoadReq :: SetSWVer(SINT8* V, UINT8 Len)
{
    UINT32 OffSet = OFFSET_USERNAME + GetUserNameLen()
                   + 1
                   + GetFtpPassLen()
                   + 1
                   + GetFtpDirLen()
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

bool CBtsSWDLoadReq :: GetFileName(SINT8* Name) const
{
    if(!Name)
    {
        return false;
    }

    SINT8 NameBuff[FILE_NAME_LEN];
    memset(NameBuff, 0, sizeof(NameBuff));
 //   UINT8 Type = GetSWType();

#if 0		
    if(0 == Type)
    {
        strcpy(NameBuff, "BTSA.");
    }
    else if(1 == Type)
    {
        strcpy(NameBuff, "BTSB.");
    }
    else if(2 == Type)
    {
        strcpy(NameBuff, "BTSC.");
    }
    else
    {
        false;
    }
#else
#ifndef WBBU_CODE
    strcpy(NameBuff, "BTSA.");

#else
    strcpy(NameBuff, "BBU_RRU.");
#endif
#endif
    SINT8 VerBuff[FILE_NAME_LEN];
    memset(VerBuff, 0, sizeof(VerBuff));
    GetSWVer(VerBuff);
    strcat(NameBuff, VerBuff);
    strcat(NameBuff, ".BIN");
    
    memcpy(Name, NameBuff, strlen(NameBuff));
    return true;
}

CBtsSWDLoadReq :: ~CBtsSWDLoadReq()
{

}


