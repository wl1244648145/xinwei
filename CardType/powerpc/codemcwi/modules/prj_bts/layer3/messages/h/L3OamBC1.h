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

#ifndef _INC_L3OAMBCCPESWREQ
#define _INC_L3OAMBCCPESWREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif
#if 0
class CBCUTSWReq : public CMessage
{
public: 
    CBCUTSWReq(CMessage &rMsg);
    CBCUTSWReq();
    bool CreateMessage(CComEntity&);
    ~CBCUTSWReq();
    UINT32 GetDefaultDataLen() const;  
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT8  GetRetryTimes();
    void   SetRetryTimes(UINT8);
    
    UINT8  GetModelType() const;
    void   SetModelType(UINT8);
        
    UINT8  GetSWType() const;
    void   SetSWType(UINT8);
    
    bool   GetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN) const;
    bool   SetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN);

    bool   GetFileName(SINT8*) const;     //构造文件名
private:
#pragma pack(1)
    struct T_BCUTSWReq
    {
        UINT16  TransId;
    	UINT8   m_RetryTimes;
        UINT8   m_ModelType; // 0:CPE    1:HS    2:PCCARD
        UINT8   m_SWType;    //软件类型; 0;  1 -- b;  2 -- c;
        SINT8   m_SWVer[FILEVER_STR_LEN];     //软件版本; 同软件类型的组合构造出bts软件文件名
                             //文件名如 FILENAME = CPE.1.12.1.1.BIN
    };                       //         FILENAME = HS.1.12.1.1.BIN           
#pragma pack()
};           

/////////////////////////////////////////////////////////////////////////////////////////////
class CBCUTSWReqNew : public CMessage
{
public: 
    CBCUTSWReqNew(CMessage &rMsg);
    CBCUTSWReqNew();
    bool CreateMessage(CComEntity&);
    ~CBCUTSWReqNew();
    UINT32 GetDefaultDataLen() const;  
public: 
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);
    
    UINT8  GetRetryTimes();
    void   SetRetryTimes(UINT8);

    UINT8  GetFileNameLen();
	void   SetFileNameLen(UINT8);

    bool   GetFileName(SINT8*, UINT8 Len = FILE_NAME_LEN) const;
    bool   SetFileName(SINT8*, UINT8 Len = FILE_NAME_LEN);
    
private:
#pragma pack(1)
    struct T_BCUTSWReqNew
    {
        UINT16  TransId;
    	UINT8   m_RetryTimes;
        UINT8   m_FileNameLen;
        SINT8   m_FileName[FILE_NAME_LEN];		
    };   
#pragma pack()
};         
#endif //0
#endif
