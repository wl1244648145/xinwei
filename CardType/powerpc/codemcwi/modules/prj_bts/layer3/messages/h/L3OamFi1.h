/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3OamBtsSWDLoadNotify.h
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

#ifndef _INC_L3OAMFILELOADNOTIFY
#define _INC_L3OAMFILELOADNOTIFY

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMFILECOMMON
#include "L3OamFileCommon.h"
#endif

//发送到ftp client 任务，完成文件上/下载
class CFileLoadNotify : public CMessage
{
public: 
    CFileLoadNotify(CMessage &rMsg);
    CFileLoadNotify();
    bool CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;    
    ~CFileLoadNotify();
    
    SINT32 GetFtpServerIp() const;
    void   SetFtpServerIp(SINT32);
   
	UINT16 GetFtpServerPort() const;
    void   SetFtpServerPort(UINT16);
    
    bool   GetUserName(SINT8*, UINT8 Len = USER_NAME_LEN) const;
    bool   SetUserName(SINT8*, UINT8 Len = USER_NAME_LEN);
    
    bool   GetFtpPass(SINT8*, UINT8 Len = USER_PASSWORD_LEN) const;
    bool   SetFtpPass(SINT8*, UINT8 Len = USER_PASSWORD_LEN);
    
    bool   GetFtpDir(SINT8*, UINT8 Len = FILE_DIRECTORY_LEN) const;
    bool   SetFtpDir(SINT8*, UINT8 Len = FILE_DIRECTORY_LEN);
    
    bool   GetFileName(SINT8*, UINT8 Len = FILE_NAME_LEN) const;
    bool   SetFileName(SINT8*, UINT8 Len = FILE_NAME_LEN);
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
private:
#pragma pack(1)
    struct T_FileLoadInfo
    {
        UINT16  m_TransId;
        SINT32  m_FTPserverIP;
        UINT16  m_FTPserverPort;  
        SINT8   m_UserName[USER_NAME_LEN];
        SINT8   m_FTPPassword[USER_PASSWORD_LEN];
        SINT8   m_FtpDir[FILE_DIRECTORY_LEN];
        SINT8   m_FileName[FILE_NAME_LEN];
    };
#pragma pack()
};
#endif


