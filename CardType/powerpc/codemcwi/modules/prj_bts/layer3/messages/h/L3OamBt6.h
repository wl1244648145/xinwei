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

#ifndef _INC_L3OAMBTSSWDLOADREQ
#define _INC_L3OAMBTSSWDLOADREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMFILECOMMON
#include "L3OamFileCommon.h"
#endif
// Download BTS Software Request（EMS）
class CBtsSWDLoadReq : public CMessage
{
public: 
    CBtsSWDLoadReq(CMessage &rMsg);
    CBtsSWDLoadReq();
    bool CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;
    ~CBtsSWDLoadReq();

    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    SINT32 GetFtpServerIp() const;
    void   SetFtpServerIp(SINT32);

    SINT16 GetFtpServerPort() const;
    void   SetFtpServerPort(SINT16);
    
    UINT8  GetUserNameLen() const;
    void   SetUserNameLen(UINT8);
    
    bool   GetUserName(SINT8*, UINT8) const;
    bool   SetUserName(SINT8*, UINT8);

    UINT8  GetFtpPassLen() const;
    void   SetFtpPassLen(UINT8);
    
    bool   GetFtpPass(SINT8*, UINT8) const;
    bool   SetFtpPass(SINT8*, UINT8);
    
    UINT8  GetFtpDirLen() const;
    void   SetFtpDirLen(UINT8);
    
    bool   GetFtpDir(SINT8*, UINT8) const;
    bool   SetFtpDir(SINT8*, UINT8);
    
    UINT8  GetSWType() const;
    void   SetSWType(UINT8);
    
    bool   GetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN) const;
    bool   SetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN);

    bool   GetFileName(SINT8* Name) const;     //构造文件名
private:
    enum
    {
        OFFSET_USERNAME = 9  
    }; 
#pragma pack(1)
    struct T_DLBtsSWReq
    {
        UINT16  m_TransId;
        SINT32  m_FTPserverIP;
        UINT16  m_FTPserverPort;  
        UINT8   m_UserNameLen;
        SINT8   m_UserName[USER_NAME_LEN];
        UINT8   m_FTPPasswordLen;
        SINT8   m_FTPPassword[USER_PASSWORD_LEN];
        UINT8   m_FtpDirLen;
        SINT8   m_FtpDir[FILE_DIRECTORY_LEN];
        UINT8   m_SWType;   //软件类型; 0;  1 -- b;  2 -- c;
        SINT8   m_SWVer[FILEVER_STR_LEN];    //软件版本; 同软件类型的组合构造出bts软件文件名
                                             //文件名如 FILENAME = BTS.1.12.1.1.BIN
    };
#pragma pack()
};
#endif

