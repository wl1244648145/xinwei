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

#ifndef _INC_L3OAMFILELOADNOTIFY
#include "L3OamFileLoadNotify.h"
#endif
#include <string.h>

CFileLoadNotify :: CFileLoadNotify(CMessage &rMsg)
:CMessage(rMsg)
{}

CFileLoadNotify :: CFileLoadNotify()
{}

bool CFileLoadNotify :: CreateMessage(CComEntity &ComEntity)
{
    CMessage :: CreateMessage(ComEntity);
//    SetMessageId(M_EMS_BTS_DL_BTS_SW_REQ);
    return true;
}

UINT32 CFileLoadNotify :: GetDefaultDataLen() const
{
    return sizeof(T_FileLoadInfo);
}

UINT16 CFileLoadNotify :: GetTransactionId() const
{
    return ((T_FileLoadInfo*)GetDataPtr())->m_TransId;
}

UINT16 CFileLoadNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_FileLoadInfo*)GetDataPtr())->m_TransId = TransId;
	return 0;
}

SINT32 CFileLoadNotify :: GetFtpServerIp() const
{
    return ((T_FileLoadInfo *)GetDataPtr())->m_FTPserverIP;
}

void CFileLoadNotify :: SetFtpServerIp(SINT32 FTPserverIP)
{
    ((T_FileLoadInfo*)GetDataPtr())->m_FTPserverIP = FTPserverIP;
}

UINT16 CFileLoadNotify :: GetFtpServerPort() const
{
    return ((T_FileLoadInfo *)GetDataPtr())->m_FTPserverPort;
}

void CFileLoadNotify :: SetFtpServerPort(UINT16 Port)
{
    ((T_FileLoadInfo*)GetDataPtr())->m_FTPserverPort = Port;
}

bool CFileLoadNotify :: GetUserName(SINT8* UserName, UINT8 Len) const
{
    if(NULL != UserName)
    {
        memcpy(UserName, ((T_FileLoadInfo*)GetDataPtr())->m_UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool CFileLoadNotify :: SetUserName(SINT8 *UserName, UINT8 Len)
{
    if(NULL != UserName)
    {
        memcpy(((T_FileLoadInfo*)GetDataPtr())->m_UserName, UserName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

//一下计算全以 OFFSET_USERNAME 为基准，加的1字节是长度字段占的空间
bool CFileLoadNotify :: GetFtpPass(SINT8 *FTPPassword, UINT8 Len) const
{
    if(NULL != FTPPassword)
    {
        memcpy(FTPPassword, ((T_FileLoadInfo*)GetDataPtr())->m_FTPPassword, Len);
        return true;
    }
    else
    {
        return false;
    }
}


bool CFileLoadNotify :: SetFtpPass(SINT8 *FTPPassword, UINT8 Len)
{
    if(NULL != FTPPassword)
    {
        memcpy(((T_FileLoadInfo*)GetDataPtr())->m_FTPPassword, FTPPassword, Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool  CFileLoadNotify :: GetFtpDir(SINT8 *FileDirectory, UINT8 Len) const
{
    if(NULL != FileDirectory)
    {
        memcpy(FileDirectory, ((T_FileLoadInfo*)GetDataPtr())->m_FtpDir, Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool   CFileLoadNotify :: SetFtpDir(SINT8 *FileDirectory, UINT8 Len)
{
    if(NULL != FileDirectory)
    {
        memcpy(((T_FileLoadInfo*)GetDataPtr())->m_FtpDir, FileDirectory, Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool  CFileLoadNotify :: GetFileName(SINT8* FileName, UINT8 Len)const
{
    if(NULL != FileName)
    {
        memcpy(FileName, ((T_FileLoadInfo*)GetDataPtr())->m_FileName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

bool  CFileLoadNotify :: SetFileName(SINT8* FileName, UINT8 Len)
{
    if(NULL != FileName)
    {
        memcpy(((T_FileLoadInfo*)GetDataPtr())->m_FileName, FileName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

CFileLoadNotify :: ~CFileLoadNotify()
{}


