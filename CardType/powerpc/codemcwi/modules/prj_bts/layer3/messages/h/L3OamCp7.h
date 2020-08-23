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

#ifndef _INC_L3OAMCPESWDLOADREQ
#define _INC_L3OAMCPESWDLOADREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMFILECOMMON
#include "L3OamFileCommon.h"
#endif
// Download BTS Software Request（EMS）
class CCpeSWDLoadReq : public CMessage
{
public: 
    CCpeSWDLoadReq(CMessage &rMsg);
    CCpeSWDLoadReq();
    bool CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;
    ~CCpeSWDLoadReq();
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16 TransId);

    SINT32 GetFtpServerIp()const;
    void   SetFtpServerIp(SINT32);
    
    UINT16 GetFtpServerPort()const;
    void   SetFtpServerPort(UINT16);

    UINT8  GetUserNameLen()const;
    void   SetUserNameLen(UINT8);
    
    bool   GetUserName(SINT8*, UINT8)const;
    bool   SetUserName(SINT8*, UINT8);
    
    UINT8  GetFtpPassLen()const;
    void   SetFtpPassLen(UINT8);
    
    bool   GetFtpPass(SINT8*, UINT8)const;
    bool   SetFtpPass(SINT8*, UINT8);
    
    UINT8  GetFtpDirLen()const;
    void   SetFtpDirLen(UINT8);
    
    bool   GetFtpDir(SINT8*, UINT8)const;
    bool   SetFtpDir(SINT8*, UINT8);
    
    UINT8  GetModelType() const;
    void   SetModelType(UINT8);
        
    UINT8  GetSWType() const;
    void   SetSWType(UINT8);
    
    bool   GetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN) const;
    bool   SetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN);

    bool   GetFileName(SINT8*) const;     //构造文件名
private:
    enum
    {
        OFFSET_USERNAME = 9
    };
#pragma pack(1)
    struct T_DLCpeSWReq
    {
        UINT16  m_TransId;
        SINT32  m_FTPserverIP;
        UINT16  m_FTPserverPort;  
        UINT8   m_UserNameLen;
        SINT8   m_UserName[USER_NAME_LEN];
        UINT8   m_FTPPasswordLen;
        SINT8   m_FTPPassword[USER_PASSWORD_LEN];
        UINT8   m_FileDirLen;
        SINT8   m_FileDirectory[FILE_DIRECTORY_LEN];
        UINT8   m_ModelType; // 0:CPE    1:HS    2:PCCARD
        UINT8   m_SWType;    //软件类型; 0;  1 -- b;  2 -- c;
        SINT8   m_SWVer[FILEVER_STR_LEN];     //软件版本; 同软件类型的组合构造出bts软件文件名
                             //文件名如 FILENAME = CPE.1.12.1.1.BIN
    };                       //         FILENAME = HS.1.12.1.1.BIN           
#pragma pack()
};                          //         FILENAME = PCCARD.1.12.1.1.BIN


///////////////////////////////////////////////////////////////////////////////////////////////////////
// Download BTS Software Request New（EMS）
class CCpeSWDLoadReqNew : public CMessage
{
public: 
    CCpeSWDLoadReqNew(CMessage &rMsg);
    CCpeSWDLoadReqNew();
    bool CreateMessage(CComEntity&);
    UINT32 GetDefaultDataLen() const;
    ~CCpeSWDLoadReqNew();
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16 TransId);

    SINT32 GetFtpServerIp()const;
    void   SetFtpServerIp(SINT32);
    
    UINT16 GetFtpServerPort()const;
    void   SetFtpServerPort(UINT16);

    UINT8  GetUserNameLen()const;
    void   SetUserNameLen(UINT8);
    
    bool   GetUserName(SINT8*, UINT8)const;
    bool   SetUserName(SINT8*, UINT8);
    
    UINT8  GetFtpPassLen()const;
    void   SetFtpPassLen(UINT8);
    
    bool   GetFtpPass(SINT8*, UINT8)const;
    bool   SetFtpPass(SINT8*, UINT8);
    
    UINT8  GetFtpDirLen()const;
    void   SetFtpDirLen(UINT8);
    
    bool   GetFtpDir(SINT8*, UINT8)const;
    bool   SetFtpDir(SINT8*, UINT8);

    UINT8  GetFileNameLen()const;
    void   SetFileNameLen(UINT8);
	
    bool   GetFileName(SINT8*, UINT8)const;
    bool   SetFileName(SINT8*, UINT8);
 private:
    enum
    {
        OFFSET_USERNAME = 9
    };
#pragma pack(1)
    struct T_DLCpeSWReqNew
    {
        UINT16  m_TransId;
        SINT32  m_FTPserverIP;
        UINT16  m_FTPserverPort;  
        UINT8   m_UserNameLen;
        SINT8   m_UserName[USER_NAME_LEN];
        UINT8   m_FTPPasswordLen;
        SINT8   m_FTPPassword[USER_PASSWORD_LEN];
        UINT8   m_FileDirLen;
        SINT8   m_FileDirectory[FILE_DIRECTORY_LEN];
		UINT8   m_FileNameLen;
		SINT8   m_FIleName[FILE_NAME_LEN];
    };   
#pragma pack()
};
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////             For Z Software Update           //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
class CZSWDLoadReq : public CMessage
{
public:
    CZSWDLoadReq(CMessage &rMsg);
    CZSWDLoadReq();
    ~CZSWDLoadReq();

    bool CreateMessage(CComEntity& ComEntity);
    UINT32 GetDefaultDataLen() const;
    UINT16 GetTransactionId() const;
    SINT32 GetFtpServerIp()const;
    UINT16 GetFtpServerPort()const;

    UINT8  GetUserNameLen()const;
    bool GetUserName(SINT8* UserName, UINT8 Len) const;
    UINT8 GetFtpPassLen() const;
    bool GetFtpPass(SINT8 *FTPPassword, UINT8 Len) const;
    UINT8 GetFtpDirLen() const;
    bool  GetFtpDir(SINT8 *FileDirectory, UINT8 Len) const;
    bool  GetSWVer(SINT8* V, UINT8 Len = FILEVER_STR_LEN) const;
    bool  GetSWProductType(SINT8* V, UINT8 Len = FILEVER_STR_LEN) const;
    bool GetFileName(SINT8* Name) const;
private:
#pragma pack(1)
    enum
    {
        OFFSET_USERNAME = 9
    };
    struct T_DLZSWReq
    {
        UINT16  m_TransId;
        SINT32  m_FTPserverIP;
        UINT16  m_FTPserverPort;  
        UINT8   m_UserNameLen;
        SINT8   m_UserName[USER_NAME_LEN];
        UINT8   m_FTPPasswordLen;
        SINT8   m_FTPPassword[USER_PASSWORD_LEN];
        UINT8   m_FileDirLen;
        SINT8   m_FileDirectory[FILE_DIRECTORY_LEN];
        //UINT8   m_ModelType; // 0:CPE    1:HS    2:PCCARD
        //UINT8   m_SWType;    //软件类型; 0;  1 -- b;  2 -- c;
        SINT8   m_SWVer[FILEVER_STR_LEN];     //软件版本; 同软件类型的组合构造出bts软件文件名
        UINT8   m_ZProductType[FILEVER_STR_LEN];
    };         
#pragma pack()
};
#endif //MZ_2ND

#endif
