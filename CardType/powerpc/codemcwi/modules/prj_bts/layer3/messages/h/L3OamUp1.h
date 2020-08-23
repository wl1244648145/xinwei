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

#ifndef _INC_L3OAMUPDATECPESWREQ
#define _INC_L3OAMUPDATECPESWREQ


#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

class CUpdateCpeSWReq : public CMessage
{
public: 
    CUpdateCpeSWReq(CMessage &rMsg);
    CUpdateCpeSWReq();
    bool CreateMessage(CComEntity&);
    ~CUpdateCpeSWReq();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT32 GetCPEID();
    void   SetCPEID(UINT32);

    UINT8  GetModelType() const;
    void   SetModelType(UINT8);
        
    UINT8  GetSWType() const;
    void   SetSWType(UINT8);
    
    bool   GetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN) const;
    bool   SetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN);
	
    bool   GetFileName(SINT8*) const;     //构造文件名

#pragma pack(1)
    struct T_UpdateUTSWReq
    {
        UINT16  TransId;
        UINT32  m_CPEID;
        UINT8   m_ModelType; // 0:CPE    1:HS    2:PCCARD
        UINT8   m_SWType;    //软件类型; 0;  1 -- b;  2 -- c;
        SINT8   m_SWVer[FILEVER_STR_LEN];     //软件版本; 同软件类型的组合构造出bts软件文件名
        UINT32  m_uid;
        SINT8   m_ZProductType[FILEVER_STR_LEN];
                             //文件名如 FILENAME = CPE.1.12.1.1.BIN
    };                       //         FILENAME = HS.1.12.1.1.BIN           
#pragma pack()
};  



//////////////////////////////////////////////////////////////////////////////////////////////
class CUpdateCpeSWReqNew : public CMessage
{
public: 
    CUpdateCpeSWReqNew(CMessage &rMsg);
    CUpdateCpeSWReqNew();
    bool CreateMessage(CComEntity&);
    ~CUpdateCpeSWReqNew();
    UINT32 GetDefaultDataLen() const;  
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT32 GetCPEID();
    void   SetCPEID(UINT32);
	
    UINT8  GetFileNameLen();
	void   SetFileNameLen(UINT8);

    bool   GetFileName(SINT8*, UINT8 Len = FILE_NAME_LEN) const;
    bool   SetFileName(SINT8*, UINT8 Len = FILE_NAME_LEN);

//    UINT8  GetModelType() const;
        
//    UINT8  GetSWType() const;
#ifdef MZ_2ND
    UINT8  GetHWtype();
	UINT32 getPid();
#endif
#pragma pack(1)
    struct T_UpdateUTSWReqNew
    {
        UINT16  TransId;
        UINT32  m_CPEID;
        UINT8   m_FileNameLen;
        SINT8   m_FileName[FILE_NAME_LEN];
#ifdef MZ_2ND
        UINT8   m_ucHWType;
        UINT32  m_ulPid;
#endif
     };
#pragma pack()
};  

#if 0
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////             For Z Software Update           //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
class CUpdateZSWReq : public CMessage
{
public: 
	CUpdateZSWReq(CMessage &rMsg);
	CUpdateZSWReq();
	bool CreateMessage(CComEntity&);
	~CUpdateZSWReq();
	UINT32 GetDefaultDataLen() const;  
	UINT16 GetTransactionId() const;
	UINT16  GetUpdateType() const;
	UINT32 GetCPEID();
	UINT32 GetUid();                                      
	bool   GetSWVer(SINT8*, UINT8 Len = FILEVER_STR_LEN) const;
	bool   GetZProductType(SINT8* ZProductType, UINT8 Len = FILEVER_STR_LEN) const;
	bool   GetFileName(SINT8* Name) const;     //构造文件名                                

#pragma pack(1)
	struct T_UpdateZSWReq
	{
		UINT16  TransId;
		UINT16  m_UpdateType;  //0：强制  1：非强制
		UINT32  m_CPEID;
		UINT32  m_uid;
		SINT8   m_SWVer[FILEVER_STR_LEN]; 
		SINT8   m_ZProductType[FILEVER_STR_LEN];
	};          
#pragma pack()

public:

};  
#endif //0
#endif
