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


#ifndef _INC_L3L3CPESWDLREQ
#define _INC_L3L3CPESWDLREQ

#ifndef _INC_MESSAGE
#include "Message.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

class CL3CpeSWDLReq : public CMessage
{
public: 
    CL3CpeSWDLReq(CMessage &rMsg);
    CL3CpeSWDLReq();
    bool CreateMessage(CComEntity& Entity);
    ~CL3CpeSWDLReq();
    UINT32  GetDefaultDataLen() const;
    
    UINT16 GetTransactionId() const;
    UINT16 SetTransactionId(UINT16);

    UINT16 GetDLType()const;      // 0 - unicast; 1 - broadcast
    void   SetDLType(UINT16);      // 0 - unicast; 1 - broadcast

    UINT16 GetDLReqSeqNum()const;	// 1~65535
    void   SetDLReqSeqNum(UINT16);	// 1~65535

    UINT32 GetFileVersion()const;	// 
    void   SetFileVersion(UINT32);	// 

    UINT16 GetInterfaceType()const;
    void   SetInterfaceType(UINT16);

    UINT32 GetFileSize()const;		
    void   SetFileSize(UINT32);		

    UINT16 GetDLPackSize()const;		
    void   SetDLPackSize(UINT16);		

    UINT16 GetHWverListNum()const;		
    void   SetHWverListNum(UINT16);		

    UINT16 GetIsComipApp()const;		
    void   SetIsComipApp(UINT16);		


    bool   GetHWverList(SINT8*, UINT16)const;	// DEVICE_HW_VER_SIZE V*8B List of compatible HW version 
    bool   SetHWverList(SINT8*, UINT16);	
    
private:
#pragma pack(1)
    struct T_BaseMsg
    {
        UINT16 TransId;
        UINT16 Version;        
        UINT16 DLType;      // 0 - unicast; 1 - broadcast
        UINT16 DLReqSeqNum;	// 1~65535
        UINT32 FileVersion;	// Major.minor.build
        UINT16 InterfaceType;
        UINT32 FileSize;		
        UINT16 DLPackSize;	
        UINT16 IsComipApp;   // 0 - not comip app; 1 - comipapp
        UINT16 HWverListNum;		
        SINT8  HWverList[MAX_HWVER_LIST_NUM * DEVICE_HW_VER_SIZE];	// DEVICE_HW_VER_SIZE V*8B List of compatible HW version 
    };
#pragma pack()
};


///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////             For Z Software Update           //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

class CL3ZSWDLReq : public CMessage
{
public: 
    CL3ZSWDLReq(CMessage &rMsg);
    CL3ZSWDLReq();
    bool CreateMessage(CComEntity& Entity);
    ~CL3ZSWDLReq();
    UINT32  GetDefaultDataLen() const;
    UINT16   SetTransactionId(UINT16);
    void   SetDLReqSeqNum(UINT16);	// 1~65535
    void   SetFileVersion(UINT32);	// 
    void   SetFileSize(UINT32);		
    void   SetDLPackSize(UINT16);		
	void   SetVersion    (UINT16 Version    );
	void   SetProductType(UINT32 ProductType);
	void   SetPid        (UINT32 ulPid        );
	void   SetFileType   (UINT16 FileType   );
	void   SetUpgradeFlag(UINT16 UpgradeFlag);
	
public:
#pragma pack(1)
    struct T_ZSW_UPDATE
    {
        UINT16 TransId;
        UINT16 Version;    
        UINT32 SWVersion;
        UINT32 FileSize;
        UINT32 ProductType;
        UINT32 ulPid;
        UINT16 DLReqSeqNum;
        UINT16 PacketSize;
        UINT16 FileType;//升级程序文件类型,固定为 0
        UINT16 UpgradeFlag;//0x0000：强制升级   :  0x0001：非强制升级
    };
#pragma pack()
};
#endif
