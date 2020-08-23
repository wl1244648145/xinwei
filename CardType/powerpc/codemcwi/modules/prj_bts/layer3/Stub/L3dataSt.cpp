/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataStub.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   08/03/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <memory>
#include <stdio.h>

#include "L3DataStub.h"
#include "Flash.h"

extern "C" {
typedef void (*pfEBFreeCallBack) (UINT32 param);
BOOL mv643xxRecvMsgFromEB(char * pData, UINT16 ulLength, pfEBFreeCallBack ComMsgFreeFunc, UINT32 param)
{
    char str[ 4096 ] = {0};
    sprintf(str,"%s",      "\r\n*******************************");
    sprintf(str,"%s%s",str,"\r\n*****STUB::Send To WAN....*****");
    sprintf(str,"%s%s",str,"\r\n*******************************\r\n");
    UINT8 *pBuf = (UINT8*)pData;
    for (UINT32 i = 1; i <= ulLength; ++i)
        {
        sprintf(str,"%s%.2X ", str, pBuf[ i - 1 ] );
        if ( 0 == i )
            {
            continue;
            }
        if ( 0 == i % 16 )
            {
            sprintf(str,"%s\r\n",str);
            continue;
            }
        if ( 0 == i % 8 )
            {
            sprintf(str,"%s  ",str);
            }
        }

    printf(str);

    ComMsgFreeFunc(param);
    return TRUE;
}


UINT32 GetBtsIpAddr()
{
    printf("\r\n*****STUB::GetBtsIpAddr....return 192.168.2.219*****\r\n");
    //返回主机序
    return 0xc0a802DB;
}

void Drv_Reclaim(void *)
{
    printf("\r\n*****STUB::Drv_Reclaim....*****\r\n");
}

typedef void (*pfEBRxCallBack)(char *, UINT16, char *);
void Drv_RegisterEB(pfEBRxCallBack EBFunc)
{
    printf("\r\n*****STUB::Drv_RegisterEB(pfEBRxCallBack EBFunc)....*****\r\n");
}
}/*extern "C"*/

#if 0
void ARP_DelILEntry(UINT32 ulIp)
{
    printf("\r\n*****STUB::ARP_DelILEntry( IP: 0X%X )....*****\r\n", ulIp);
}


void ARP_AddILEntry(UINT32 ulEid, const UINT8 *pMac, UINT32 ulIp)
{
    //ARP模块保证不重复增加
    printf("\r\n*****STUB::ARP_AddILEntry(Eid:0x%X; IP: 0X%X; Mac:%.2X-%.2X-%.2X-%.2X-%.2X-%.2X )....*****\r\n", ulEid, ulIp, pMac[0],pMac[1],pMac[2],pMac[3],pMac[4],pMac[5]);
}

bool DM_QueryRenewFlag(UINT32 ulEid)
{
    bool flag = true;
    printf("\r\n*****STUB::DM_QueryRenewFlag(Eid:0x%X),,return %s....*****\r\n", ulEid, ( flag == true )?"true":"false" );
    return flag;
}

bool DM_QueryMobilityFlag(UINT32 ulEid)
{
    bool flag = true;
    printf("\r\n*****STUB::DM_QueryMobilityFlag(Eid:0x%X)..return %s..*****\r\n", ulEid, ( flag == true )?"true":"false" );
    return flag;
}


/*
 *对EID是否已经达到最大配置的用户?
 *true: 允许上线
 *false:禁止上线
 */
bool DM_AllowAccess(UINT32 ulEid)
{
    bool flag = true;
    printf("\r\n*****STUB::DM_QueryAccess(Eid:0x%X)..return %s..*****\r\n", ulEid, ( flag == true )?"true":"false" );
    return flag;
}
#endif
#ifdef UNITEST

/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardingTable_Print

DESCRIPTION:
    桩函数:打印Mac对应的转发表内容

ARGUMENTS:
    Mac: Mac 地址

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardingTable_Print(CMac& Mac)
{
    UINT8 strMac[ M_MACADDR_STRLEN ];
    printf("\r\nFT Entry:: <Key(Mac) = %s >", Mac.str( strMac ) );
    FTEntry *pFT = GetFTEntryByIdx( BPtreeFind( Mac ) );
    if ( NULL == pFT )
        {
        printf("\r\nNULL");
        return ;
        }

    printf("\r\nEntry::\
        \r\n\t%-10s: %d\
        \r\n\t%-10s: 0x%x\
        \r\n\t%-10s: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\
        \r\n\t%-10s: %d\
        \r\n\t%-10s: %d\
        \r\n\t%-10s: 0x%x\
        \r\n\t%-10s: %d\
        \r\n\t%-10s: %d\r\n",
        "Occupied",pFT->bIsOccupied,
        "Eid",pFT->ulEid,
        "Mac",
        pFT->aucMAC[0],pFT->aucMAC[1],pFT->aucMAC[2],pFT->aucMAC[3],pFT->aucMAC[4],pFT->aucMAC[5],
        "Serving",pFT->bIsServing,
        "Tunnel",pFT->bIsTunnel,
        "PeerBTS",pFT->ulPeerBtsAddr,
        "TTL",pFT->usTTL,
        "Auth",pFT->bIsAuthed );

    return;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::ForwardingTable_AddEntry

DESCRIPTION:
    桩函数: 增加转发表表项，并修改BPtree

ARGUMENTS:
    FTEntry*: 转发表

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CTBridge::ForwardingTable_AddEntry( FTEntry *pFTin )
{
    UINT16 usIdx = GetFreeFTEntryIdxFromList();
    FTEntry *pFT = GetFTEntryByIdx( usIdx );
    if ( NULL == pFT )
        {
        printf("FT ADD Entry Failed!");
        }
    else
        {
        memcpy(pFT, pFTin, sizeof( FTEntry ) );

        CMac Mac(pFTin->aucMAC);
        BPtreeAdd( Mac, usIdx );
        }

    return;
}
#endif
#if 0
/*============================================================
MEMBER FUNCTION:
    CTSnoop::IpListTable_Print

DESCRIPTION:
    桩函数:打印Mac对应的Ip List内容

ARGUMENTS:
    Mac: Mac 地址

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::IpListTable_Print(CMac& Mac)
{
    printf("\r\nIL Entry:: <Key(Mac) = %.2X-%.2X-%.2X-%.2X-%.2X-%.2X >",
        ( Mac.GetMac() )[0],
        ( Mac.GetMac() )[1],
        ( Mac.GetMac() )[2],
        ( Mac.GetMac() )[3],
        ( Mac.GetMac() )[4],
        ( Mac.GetMac() )[5] );
    ILEntry *pILEntry = GetILEntryByIdx( BPtreeFind( Mac ) );

    PrintILEntry( pILEntry );

    return;
}



/*============================================================
MEMBER FUNCTION:
    CTSnoop::IpList_AddEntry

DESCRIPTION:
    桩函数: 增加Ip List表项，并修改BPtree

ARGUMENTS:
    ILEntry*: Ip List表项

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::IpList_AddEntry( ILEntry *pILin )
{
    UINT16 usIdx = GetFreeILEntryIdxFromList();
    ILEntry *pILEntry = GetILEntryByIdx( usIdx );
    if ( NULL == pILEntry )
        {
        printf("\r\nADD Ip List Entry Failed!");
        }
    else
        {
        memcpy(pILEntry, pILin, sizeof( ILEntry ) );

        CMac Mac(pILin->aucMAC);
        BPtreeAdd( Mac, usIdx );
        }

    return;
}
#endif

#ifdef __WIN32_SIM__

//DAIB存储的文件名
const char *lpszFileName = "C:/DAIB.txt";
extern "C"
BOOL Flash_Write_Sector(UINT8 sectorId, COMMON_SECTION_DATA *data_ptr, UINT16 wordCount)
{
    if ( NULL == data_ptr )
        {
        return false;
        }

    HANDLE hFile;
    UINT8  *pDAIB = (UINT8*)&( data_ptr->PlaceHolder );
    UINT32 ulLen  = wordCount * 2;

    hFile = ::CreateFile( lpszFileName,
                         GENERIC_READ|GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_ALWAYS, 
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
        {
		int err = GetLastError();
        return false;
        }

    DWORD dwPtr = ::SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
    if ( dwPtr == 0xFFFFFFFF )
        {
        ////
        ::CloseHandle( hFile );
        return false;
        }

    DWORD dwWrittenBytes = 0;
    if ( false == ::WriteFile( hFile, 
        pDAIB,
        ulLen,
        &dwWrittenBytes,
        NULL
        ) )
        {
        ////
        ::CloseHandle( hFile );
        return false;
        }
    ::SetEndOfFile( hFile );
    ::CloseHandle( hFile );

    if ( dwWrittenBytes != ulLen )
        {
        return false;
        }

    return true;
}

extern "C"
BOOL Flash_Read_Sector(UINT8 id, COMMON_SECTION_DATA *dataPtr)
{
    if ( NULL == dataPtr )
        {
        return FALSE;
        }

    HANDLE hFile;
    DWORD dwFileSize;

    hFile = ::CreateFile( lpszFileName,
                         GENERIC_READ,
                         0,
                         NULL,
                         OPEN_ALWAYS, 
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
        {
        return FALSE;
        }

    dwFileSize = ::GetFileSize(hFile, NULL);
    if ( INVALID_FILE_SIZE == dwFileSize || 0 == dwFileSize )
    {
		::CloseHandle(hFile);
        return FALSE;
    }

    ::ReadFile( hFile, (UINT8*)&(dataPtr->PlaceHolder), dwFileSize, &dwFileSize, NULL );
    dataPtr->Checksum = 0;
    dataPtr->WordCount = dwFileSize/2;

    ::CloseHandle(hFile);

    return TRUE;    
}


//DAIB存储的文件名
const char *lpszCfgFileName = "C:/OAMCfg.txt";

bool WriteConfig(const UINT8 *pConfig, UINT32 ulLen)
{
    HANDLE hFile;

    if ( NULL == pConfig )
        {
        return false;
        }

    hFile = ::CreateFile( lpszCfgFileName,
                         GENERIC_READ|GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_ALWAYS, 
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
        {
        return false;
        }

    DWORD dwPtr = ::SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
    if ( dwPtr == 0xFFFFFFFF )
        {
        ////
        ::CloseHandle( hFile );
        return false;
        }

    DWORD dwWrittenBytes = 0;
    if ( false == ::WriteFile( hFile, 
        pConfig,
        ulLen,
        &dwWrittenBytes,
        NULL
        ) )
        {
        ////
        ::CloseHandle( hFile );
        return false;
        }
    ::SetEndOfFile( hFile );
    ::CloseHandle( hFile );

    if ( dwWrittenBytes != ulLen )
        {
        return false;
        }

    return true;
}


UINT16 GetConfigLen()
{
    HANDLE hFile;
    DWORD dwFileSize;

    hFile = ::CreateFile( lpszCfgFileName,
                         GENERIC_READ,
                         0,
                         NULL,
                         OPEN_ALWAYS, 
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
        {
        return 0;
        }

    dwFileSize = ::GetFileSize(hFile, NULL);
    ::CloseHandle(hFile);

    if ( INVALID_FILE_SIZE == dwFileSize || 0 == dwFileSize )
    {
        return 0;
    }
    return dwFileSize;
}

bool ReadConfig(UINT8 *pConfig)
{
    if ( NULL == pConfig )
        {
        return false;
        }

    HANDLE hFile;
    DWORD dwFileSize;

    hFile = ::CreateFile( lpszCfgFileName,
                         GENERIC_READ,
                         0,
                         NULL,
                         OPEN_ALWAYS, 
                         FILE_ATTRIBUTE_NORMAL,
                         NULL );

    if ( INVALID_HANDLE_VALUE == hFile )
        {
        return false;
        }

    dwFileSize = ::GetFileSize(hFile, NULL);
    if ( INVALID_FILE_SIZE == dwFileSize || 0 == dwFileSize )
    {
        return false;
    }

    ::ReadFile( hFile, pConfig, dwFileSize, &dwFileSize, NULL );
    ::CloseHandle(hFile);

    return true;    
}
bool GetCPEConfig(UINT32 uleid,UINT8 *pConfig)
{
	return ReadConfig(pConfig);

}
#else
/*
typedef struct T_CpeFixIpInfo
{
    UINT32 Eid;
    UINT8  MAC[M_MAC_ADDRLEN];
    UINT32 FixIP;
    UINT32 AnchorBTSIP;
    UINT32 RouterAreaID;
}CpeFixIpInfo;

//define CPE DateConfig Resquest
typedef struct T_CPEDataConfigReq
{
    UINT16 TransId;
    UINT8  Mobility;   //0 -- disabled     1 -- enabled
    UINT8  DHCPRenew;  //Allow DHCP Renew in serving BTS 0-- disabled  1-- enabled
    UINT8  Security;   //0 - disabled   1 - enabled	
    UINT8  BCFilter;   //Broadcast Filtering	0 - disabled  1 - enabled	
    UINT8  MaxIpNum;   //Max IP ddress number	1~20	
    UINT8  FixIpNum;   //Fixed Ip number		0~20
    CpeFixIpInfo stCpeFixIpInfo[M_MAX_USER_PER_UT];
}CPEDataConfigReq;

*/
bool GetCPEConfig(UINT32 uleid,UINT8 *pConfig)
{
    UINT8 dataArray[30]=
		{01,00,01,01,01,01,14,01,
		00,00,00,00,
		00,00,00,00,00,00,
		00,00,00,00,
		00,00,00,00,
		00,00,00,00
		};
	memcpy(pConfig,dataArray,sizeof(dataArray));
	return true;
}

#endif
