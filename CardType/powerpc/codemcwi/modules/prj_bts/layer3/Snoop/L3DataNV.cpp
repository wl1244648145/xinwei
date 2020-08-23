/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataNVRAM.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   03/30/06   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

 #ifndef _NO_NVRAM_RECOVER_

//����Winsock 1
#define _WINSOCKAPI_
#include <time.h>
#include <stdio.h>
#include <taskLib.h>
#ifdef __WIN32_SIM__
#include <winsock2.h>
#else
#include <inetLib.h>
#endif

#include "L3DataMacAddress.h"
#include "L3DataSnoopFsm.h"
#include "L3DataNVRAM.h"
#include "sysBtsConfigData.h"

//externs:
extern UINT32 GetRouterAreaId();
#ifdef WBBU_CODE
#define NVRAM_BASE_ADDR_DATA           (0xf0001000 + 60*1024 + 4096)	/*+4096:��Ϊ�˱�֤��OAM����ͬһNVRAM����ҳ��*/
#define NVRAM_DATA_SERVICE_SIZE        (204*1024)
#endif
typedef map<CMac, UINT16>::value_type ValType;

/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::showStatus

DESCRIPTION:
    ��ӡ״̬��������

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CNVRamCCBTable::showStatus(UINT32 ulEid)
{
    //CCB table Attributes.
    printf( "\r\n" );
    printf( "\r\n****************************" );
    printf( "\r\n*NVRam CCB table Attributes*" );
    printf( "\r\n****************************" );

    if ( 0 == ulEid )
        {
        printf( "\r\nAll CCB information" );
        }
    else
        {
        printf( "\r\nEID[%d] CCB information*", ulEid );
        }

    UINT8 ucFlowCtrl = 0;
    map<CMac, UINT16>::iterator it = m_CCBtree.begin();
    while ( m_CCBtree.end() != it )
        {
        //show..
        if ( 0 != ulEid ) 
            {
            stNVRamCCB *pCCB = GetCCBByIdx( it->second );
            if ( ( NULL == pCCB ) || ( ulEid != pCCB->ulEid ) )
                {
                ++it;
                continue;
                }
            }

        showCCB( it->second );
        ++it;

        //����
        if ( 200 == ++ucFlowCtrl )
            {
#ifdef __WIN32_SIM__
//Win32:
            ::Sleep( 100 );//�ͷ�CPU
#else
//VxWorks:
            ::taskDelay( 1 );
#endif
            ucFlowCtrl = 0;
            }
        }
    //Free CCB Entries
    printf( "\r\n%-20s:: %-5d entries", "Free    CCB Entries", m_listFreeCCB.size() );

    //BPtree
    if ( true == m_CCBtree.empty() )
        {
        printf( "\r\n%-20s:: %s", "Indexed CCB Entries", "0     entries" );
        }
    else
        {
        printf( "\r\n%-20s:: %-5d entries", "Indexed CCB Entries", m_CCBtree.size() );
        }
    printf( "\r\n" );
}

#if 0
/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::InitFreeCCB

DESCRIPTION:
    ��ʼ������CCB����m_listFreeCCB

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CNVRamCCBTable::InitFreeCCB()
{
    m_listFreeCCB.clear();
    for ( UINT16 usIdx = 0; usIdx < SNOOP_NVRAM_CCB_NUM; ++usIdx )
        {
        m_listFreeCCB.push_back( usIdx );
        }
}
#endif

/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::GetFreeCCBIdxFromList

DESCRIPTION:
    �ӿ�������m_listFreeCCBȡ����ת��������±�;(������ͷ��ȡ)

ARGUMENTS:
    NULL

RETURN VALUE:
    usIdx:�����±�

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CNVRamCCBTable::GetFreeCCBIdxFromList()
{
    if ( true == m_listFreeCCB.empty() )
        {
        return M_DATA_INDEX_ERR;
        }

    UINT16 usIdx = *m_listFreeCCB.begin();
    m_listFreeCCB.pop_front();

    if ( M_MAX_USER_PER_BTS <= usIdx )
        {
        //�±����
        return M_DATA_INDEX_ERR;
        }

    return usIdx;
}


/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::InsertFreeCCB

DESCRIPTION:
    �������Ip List�����±굽����m_listFreeCCBβ��

ARGUMENTS:
    usIdx:�����±�

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CNVRamCCBTable::InsertFreeCCB(UINT16 usIdx )
{
    if( usIdx < SNOOP_NVRAM_CCB_NUM )
        {
        m_listFreeCCB.push_back( usIdx );
        }
    else
        {
        DATA_assert( 0 );
        }
}



/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::GetCCBByIdx

DESCRIPTION:
    ����ָ���±��CCB

ARGUMENTS:
    usIdx: CCB�±�

RETURN VALUE:
    CCBBase*: CCBָ��

SIDE EFFECTS:
    none
==============================================================*/
stNVRamCCB* CNVRamCCBTable::GetCCBByIdx(UINT16 usIdx)
{
    if ( !( usIdx < SNOOP_NVRAM_CCB_NUM ) )
        {
        return NULL;
        }
    return m_pCCBTable + usIdx;
}


/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::BPtreeAdd

DESCRIPTION:
    CCB����������ڵ㣻CCB��������Mac��ַ����ֵ����CCB
    �±���뵽������

ARGUMENTS:
    Mac:Mac��ַ
    usIdx:�����±�

RETURN VALUE:
    bool:����ɹ�/ʧ��

SIDE EFFECTS:
    ��������ظ�����᷵��ʧ�ܣ����Ա�����BPtreeAdd֮ǰ��֤
    û���ظ��������� BPtreeFind һ��
==============================================================*/
bool CNVRamCCBTable::BPtreeAdd(CMac& Mac, UINT16 usIdx)
{
    pair<map<CMac, UINT16>::iterator, bool> stPair;

    stPair = m_CCBtree.insert( ValType( Mac, usIdx ) );
    /*
     *���뱣֤�������ظ�����򽫻᷵��ʧ��
     */
    return stPair.second;
}


/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::BPtreeDel

DESCRIPTION:
    CCB������ɾ��Mac��Ӧ�Ľڵ㣻

ARGUMENTS:
    Mac:Mac��ַ

RETURN VALUE:
    bool:ɾ���ɹ�/ʧ��

SIDE EFFECTS:
    none
==============================================================*/
bool CNVRamCCBTable::BPtreeDel(CMac& Mac)
{
    map<CMac, UINT16>::iterator it = m_CCBtree.find( Mac );
    
    if ( it != m_CCBtree.end() )
        {
        //find, and erase;
        m_CCBtree.erase( it );
        }
    //not find
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::BPtreeFind

DESCRIPTION:
    ��BPtree����Mac��ַ��Ӧ��CCB�±�

ARGUMENTS:
    Mac:Mac��ַ

RETURN VALUE:
    usIdx:�����±�

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CNVRamCCBTable::BPtreeFind(CMac& Mac)
{
    map<CMac, UINT16>::iterator it = m_CCBtree.find( Mac );
    
    if ( it != m_CCBtree.end() )
        {
        //����Index.
        return it->second;
        }
    return M_DATA_INDEX_ERR;
}


/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::showCCB

DESCRIPTION:
    ��ӡָ���±��CCB

ARGUMENTS:
    usIdx: CCB�±�

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CNVRamCCBTable::showCCB(UINT16 usIdx)
{
    stNVRamCCB *pNVRamCCB = GetCCBByIdx( usIdx );

    UINT32 strServingBtsIp = htonl( pNVRamCCB->ulServingBts );
    if ( NULL != pNVRamCCB )
        {
        printf( "\r\n----------------");
        printf( "\r\n-CCBs in NVRAM:-" );
        printf( "\r\n----------------");
        printf( "\r\n\t%-10s: 0x%.8X\
            \r\n\t%-10s: %d\
            \r\n\t%-10s: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\
            \r\n\t%-10s: 0x%.8X\
            \r\n\t%-10s: %s\
            \r\n\t%-10s: %s\
            \r\n\t%-10s: %d",
            "Eid", pNVRamCCB->ulEid,
            "Group ID", pNVRamCCB->usGroupID,
            "Mac",
            pNVRamCCB->aucMac[0], pNVRamCCB->aucMac[1], pNVRamCCB->aucMac[2], pNVRamCCB->aucMac[3], pNVRamCCB->aucMac[4], pNVRamCCB->aucMac[5],
            "Raid", CSnoopFSM::s_pNVRamCCBTable->GetLastRAID(),
            "IsAnchor", ( 0 != pNVRamCCB->ucIsAnchor )?"Yes":"No",
            "IsServing",( 0 != pNVRamCCB->ucIsServing )?"Yes":"No",
            "Srv BTS", strServingBtsIp );

        UINT8 ucIpType = pNVRamCCB->ucIpType;
        if ( IPTYPE_PPPoE == ucIpType )
            {
            printf( "\r\n\t%-10s: %s\
                \r\n\t%-10s: %d\
                \r\n\t%-10s: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
                "IpType", "PPPoE",
                "SID", pNVRamCCB->Data.stSessionMac.usSessionId,
                "SMAC", 
                pNVRamCCB->Data.stSessionMac.aucServerMac[0],
                pNVRamCCB->Data.stSessionMac.aucServerMac[1],
                pNVRamCCB->Data.stSessionMac.aucServerMac[2],
                pNVRamCCB->Data.stSessionMac.aucServerMac[3],
                pNVRamCCB->Data.stSessionMac.aucServerMac[4],
                pNVRamCCB->Data.stSessionMac.aucServerMac[5] );
            }

        else if ( IPTYPE_DHCP == ucIpType )
            {
            struct in_addr IpAddr;
#ifdef __WIN32_SIM__
            IpAddr.S_un.S_addr = htonl( pNVRamCCB->Data.stIpLease.ulIp );
#else
            IpAddr.s_addr = htonl( pNVRamCCB->Data.stIpLease.ulIp );
#endif
            SINT8 strIp[ INET_ADDR_LEN ] = {0};
            inet_ntoa_b( IpAddr, strIp );

            printf( "\r\n\t%-10s: %s\
                \r\n\t%-10s: %s\
                \r\n\t%-10s: 0x%.8X  %s",
                "IpType", "DHCP",
                "IP", strIp,
                "Lease", pNVRamCCB->Data.stIpLease.ulLease, ctime( (time_t*)&pNVRamCCB->Data.stIpLease.ulLease ) );
            }

        else if ( IPTYPE_FIXIP == ucIpType )
            {
            struct in_addr IpAddr;
#ifdef __WIN32_SIM__
            IpAddr.S_un.S_addr = htonl( pNVRamCCB->Data.stIpLease.ulIp );
#else
            IpAddr.s_addr = htonl( pNVRamCCB->Data.stIpLease.ulIp );
#endif
            SINT8 strIp[ INET_ADDR_LEN ] = {0};
            inet_ntoa_b( IpAddr, strIp );

            printf( "\r\n\t%-10s: %s\
                \r\n\t%-10s: %s\
                \r\n\t%-10s: %s",
                "IpType", "FIXIP",
                "IP", strIp,
                "Lease", "--------" );
            }

        else
            {
            printf( "\r\n\t%-10s: %d\
                \r\n\t%-10s: %.2x %.2x %.2x %.2x   %.2x %.2x %.2x %.2x\
                \r\n",
                "IpType", ucIpType,
                "DATA", 
                ( (UINT8*)&pNVRamCCB->Data )[0],
                ( (UINT8*)&pNVRamCCB->Data )[1],
                ( (UINT8*)&pNVRamCCB->Data )[2],
                ( (UINT8*)&pNVRamCCB->Data )[3],
                ( (UINT8*)&pNVRamCCB->Data )[4],
                ( (UINT8*)&pNVRamCCB->Data )[5],
                ( (UINT8*)&pNVRamCCB->Data )[6],
                ( (UINT8*)&pNVRamCCB->Data )[7] );
            }
        }
}


/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::clearCCB

DESCRIPTION:
    NVRam������CCB��ʼ��Ϊ����

ARGUMENTS:
    void

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CNVRamCCBTable::clearCCB()
{
    stNVRamCCB *pNVRamCCB = m_pCCBTable;

    //����CCB������պ��ؽ�
    m_listFreeCCB.clear();

    //CCB�������
    m_CCBtree.clear();

//  bspEnableNvRamWrite( (char*)pNVRamCCB,  SNOOP_NVRAM_CCB_NUM * sizeof( stNVRamCCB ));
    memset( (char*)pNVRamCCB, 0, SNOOP_NVRAM_CCB_NUM * sizeof( stNVRamCCB ) );
////bspDisableNvRamWrite( (char*)pNVRamCCB,  SNOOP_NVRAM_CCB_NUM * sizeof( stNVRamCCB ) );

    for ( UINT16 idx = 0; idx < SNOOP_NVRAM_CCB_NUM; ++idx, ++pNVRamCCB )
        {
        m_listFreeCCB.push_back( idx );
        }
}


/*============================================================
MEMBER FUNCTION:
    CNVRamCCBTable::init

DESCRIPTION:
    NVRAM�״�ʹ��ǰ�ĳ�ʼ��

ARGUMENTS:
    void

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CNVRamCCBTable::init()
{
    bspEnableNvRamWrite( (char*)NVRAM_BASE_ADDR_DATA,  NVRAM_DATA_SERVICE_SIZE);
    if ( ( M_NVRAM_INITIALIZED == GetInitialized() ) && ( M_NVRAM_VERSION == getVersion() ) )
        {
        return;
        }

    clearCCB();

    SetInitialized();
    SetCurrRAID( M_RAID_INVALID );
    SetLocalBts( 0 );
    setVersion();
    return;
}


void CNVRamCCBTable::groupModify(UINT32 ulEid, UINT16 usGid)
{
    map<CMac, UINT16>::iterator it = m_CCBtree.begin();
    UINT8 ucFlowCtrl = 0;
    stNVRamCCB *pCCB = NULL;
    while ( m_CCBtree.end() != it )
        {
        pCCB = GetCCBByIdx( it->second );
        if ( (NULL == pCCB) || (pCCB->ulEid != ulEid) )
            {
            ++it;
            continue;
            }

        pCCB->usGroupID = usGid;

        //next.
        ++it;
        if ( 200 == ++ucFlowCtrl )
            {
            ::taskDelay( 1 );
            ucFlowCtrl = 0;
            }
        }
}
#endif
