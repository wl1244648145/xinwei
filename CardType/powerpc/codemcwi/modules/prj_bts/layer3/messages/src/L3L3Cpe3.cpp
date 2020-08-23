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
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef  _INC_L3L3CPESWDLREQ
#include "L3L3CpeSWDLReq.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif
#include <string.h>
CL3CpeSWDLReq :: CL3CpeSWDLReq()
{
}

CL3CpeSWDLReq :: CL3CpeSWDLReq(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3CpeSWDLReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L3_CPE_UPGRADE_SW_REQ);
    return true;
}

UINT32 CL3CpeSWDLReq :: GetDefaultDataLen() const
{
    return sizeof(T_BaseMsg);
}

UINT16 CL3CpeSWDLReq :: GetTransactionId() const
{
    return ((T_BaseMsg *)GetDataPtr())->TransId;
}

UINT16 CL3CpeSWDLReq :: SetTransactionId(UINT16 TransId)
{
    ((T_BaseMsg *)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT16 CL3CpeSWDLReq :: GetDLType()const      // 0 - unicast; 1 - broadcast
{
    return ((T_BaseMsg *)GetDataPtr())->DLType;
}

void   CL3CpeSWDLReq :: SetDLType(UINT16 P)      // 0 - unicast; 1 - broadcast
{
    ((T_BaseMsg *)GetDataPtr())->DLType = P;
}

UINT16 CL3CpeSWDLReq :: GetDLReqSeqNum()const	// 1~65535
{
    return ((T_BaseMsg *)GetDataPtr())->DLReqSeqNum;
}

void   CL3CpeSWDLReq :: SetDLReqSeqNum(UINT16 P)	// 1~65535
{
    ((T_BaseMsg *)GetDataPtr())->DLReqSeqNum = P;
}

UINT32 CL3CpeSWDLReq :: GetFileVersion()const	// Major.minor.build
{
    return ((T_BaseMsg *)GetDataPtr())->FileVersion;
}

void   CL3CpeSWDLReq :: SetFileVersion(UINT32 P)	// Major.minor.build
{
    ((T_BaseMsg *)GetDataPtr())->FileVersion = P;
}

UINT16 CL3CpeSWDLReq :: GetInterfaceType()const
{
    return ((T_BaseMsg *)GetDataPtr())->InterfaceType;
}

void   CL3CpeSWDLReq :: SetInterfaceType(UINT16 P)
{
    ((T_BaseMsg *)GetDataPtr())->InterfaceType = P;
}

UINT32 CL3CpeSWDLReq :: GetFileSize()const		
{
    return ((T_BaseMsg *)GetDataPtr())->FileSize;
}

void   CL3CpeSWDLReq :: SetFileSize(UINT32 P)		
{
    ((T_BaseMsg *)GetDataPtr())->FileSize = P;
}

UINT16 CL3CpeSWDLReq :: GetDLPackSize()const		
{
    return ((T_BaseMsg *)GetDataPtr())->DLPackSize;
}

void   CL3CpeSWDLReq :: SetDLPackSize(UINT16 P)		
{
    ((T_BaseMsg *)GetDataPtr())->DLPackSize = P;
}

UINT16 CL3CpeSWDLReq :: GetIsComipApp()const		
{
    return ((T_BaseMsg *)GetDataPtr())->IsComipApp;
}

void   CL3CpeSWDLReq :: SetIsComipApp(UINT16 P)		
{
    ((T_BaseMsg *)GetDataPtr())->IsComipApp = P;
}

UINT16 CL3CpeSWDLReq :: GetHWverListNum()const		
{
    return ((T_BaseMsg *)GetDataPtr())->HWverListNum;
}

void   CL3CpeSWDLReq :: SetHWverListNum(UINT16 P)		
{
    ((T_BaseMsg *)GetDataPtr())->HWverListNum = P;
}

bool CL3CpeSWDLReq :: GetHWverList(SINT8* DstBuff, UINT16 Len)const	// DEVICE_HW_VER_SIZE V*8B List of compatible HW version 
{
    if(NULL == DstBuff)
    {
        return false;
    }
    else
    {
        memcpy(DstBuff, ((T_BaseMsg *)GetDataPtr())->HWverList, Len);
        return true;
    }
}

bool CL3CpeSWDLReq :: SetHWverList(SINT8* SrcBuff, UINT16 Len)	
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(((T_BaseMsg *)GetDataPtr())->HWverList, SrcBuff, Len);
        return true;
    }
}

CL3CpeSWDLReq :: ~CL3CpeSWDLReq()
{

}



///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////             For Z Software Update           //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////


CL3ZSWDLReq :: CL3ZSWDLReq(){}
CL3ZSWDLReq :: CL3ZSWDLReq(CMessage& rMsg):CMessage(rMsg){}
CL3ZSWDLReq :: ~CL3ZSWDLReq(){}
bool CL3ZSWDLReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L3_CPE_UPGRADE_Z_SW_REQ);
    return true;
}
UINT16 CL3ZSWDLReq :: SetTransactionId(UINT16 TransId  )
{ 
	((T_ZSW_UPDATE *)GetDataPtr())->TransId     = TransId    ;
}

UINT32 CL3ZSWDLReq :: GetDefaultDataLen() const{	return sizeof( T_ZSW_UPDATE );}

void CL3ZSWDLReq :: SetDLReqSeqNum(UINT16 P){ ((T_ZSW_UPDATE *)GetDataPtr())->DLReqSeqNum = P; }
void CL3ZSWDLReq :: SetFileVersion(UINT32 P){ ((T_ZSW_UPDATE *)GetDataPtr())->SWVersion = P;   }
void CL3ZSWDLReq :: SetFileSize   (UINT32 P){ ((T_ZSW_UPDATE *)GetDataPtr())->FileSize = P;    } 
void CL3ZSWDLReq :: SetDLPackSize (UINT16 P){ ((T_ZSW_UPDATE *)GetDataPtr())->PacketSize = P;  }
void CL3ZSWDLReq :: SetVersion    (UINT16 Version    ){ ((T_ZSW_UPDATE *)GetDataPtr())->Version     = Version    ; }
void CL3ZSWDLReq :: SetProductType(UINT32 ProductType){ ((T_ZSW_UPDATE *)GetDataPtr())->ProductType = ProductType; }
void CL3ZSWDLReq :: SetPid        (UINT32 ulpid        ){ ((T_ZSW_UPDATE *)GetDataPtr())->ulPid       = ulpid        ; }
void CL3ZSWDLReq :: SetFileType   (UINT16 FileType   ){ ((T_ZSW_UPDATE *)GetDataPtr())->FileType    = FileType   ; }
void CL3ZSWDLReq :: SetUpgradeFlag(UINT16 UpgradeFlag){ ((T_ZSW_UPDATE *)GetDataPtr())->UpgradeFlag = UpgradeFlag; }


