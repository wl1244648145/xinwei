/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopFSM.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/05/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

//禁用Winsock 1
#define _WINSOCKAPI_
#include <time.h>

#ifdef __WIN32_SIM__
#include <winsock2.h>
#else
#include <inetLib.h>
#endif
#include <taskLib.h>
#include "Timer.h"
#include "LogArea.h"

#include "L3DataSnoopFSM.h"
#include "L3DataSnoopState.h"
#include "L3DataSnoopTransPPPoE.h"
#include "L3DataSnoopTransDHCP.h"
#include "L3DataDhcp.h"
#include "L3DataSnoopErrCode.h"
#include "L3DataSnoopTimer.h"
#include "L3DataSnoop.h"
#include "L3DataFTAddEntry.h"
#include "L3DataDelIL.h"


//externs:
extern "C" int bspGetBtsID();
extern void ARP_AddILEntry(UINT32, const UINT8 *, UINT32, UINT8);

typedef map<CMac, UINT16>::value_type ValType;
extern unsigned char CPE_IS_Rcpe(unsigned int eid);
unsigned int g_snoop_no_freelist = 0;

/*============================================================
MEMBER FUNCTION:
    CSnoopCCB::showCCB

DESCRIPTION:
    打印CCB各属性值

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopCCB::showCCB()
{
   // struct in_addr AnchorBts;
   // struct in_addr ServingBts;

    UINT32 strAnchorBtsIp = htonl( m_ulAnchorBts );
    UINT32 strServingBtsIp = htonl( m_ulServingBts );

    printf( "\r\n" );
    printf( "\r\n*************************" );
    printf( "\r\n*CCB Attributes         *" );
    printf( "\r\n*************************" );

    printf( "\r\nCCB State : %s", strSTATE[ GetCurrentState() ] );
    printf( "\r\nAttributes:\
        \r\n\t%-10s: %s\
        \r\n\t%-10s: 0x%.8X\
        \r\n\t%-10s: %d\
        \r\n\t%-10s: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\
        \r\n\t%-10s: 0x%.8X\
        \r\n\t%-10s: %s\
        \r\n\t%-10s: %s\
        \r\n\t%-10s: %s\
        \r\n\t%-10s: %8x\
        \r\n\t%-10s: %8x\
        \r\n\t%-10s: 0x%X\
        \r\n\t%-10s: 0x%X\
        \r\n\t%-10s: 0x%X",
        "IsAuth", ( true == (bool)m_ucIsAuthed )?"Yes":"No",
        "Eid", m_ulEid,
        "Group ID", m_usGroupID,
        "Mac",
        m_aucMac[0], m_aucMac[1], m_aucMac[2], m_aucMac[3], m_aucMac[4], m_aucMac[5],
        "Raid", m_ulRouterAreaId,
        "IsAnchor", ( true  == (bool)m_ucIsAnchor )?"Yes":"No",
        "IsServing", ( true == (bool)m_ucIsServing)?"Yes":"No",
         "IsRealAnchor", ( true  == (bool)m_IsRealAnchor )?"Yes":"No",//wangwenhua add 20081023
        "ABTS", strAnchorBtsIp,
        "SBTS", strServingBtsIp,
        "MSG", m_pMsg,
        "Timer", m_pTimer,
        "HeartBeat", m_pHeartBeatTimer );

    if ( IPTYPE_PPPoE == m_ucIpType )
        {
        printf( "\r\n\t%-10s: %s\
            \r\n\t%-10s: %d\
            \r\n\t%-10s: %.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
            "IpType", "PPPoE",
            "SID", m_Data.stSessionMac.usSessionId,
            "SMAC", 
            m_Data.stSessionMac.aucServerMac[0],
            m_Data.stSessionMac.aucServerMac[1],
            m_Data.stSessionMac.aucServerMac[2],
            m_Data.stSessionMac.aucServerMac[3],
            m_Data.stSessionMac.aucServerMac[4],
            m_Data.stSessionMac.aucServerMac[5] );
        }

    else if ( IPTYPE_DHCP == m_ucIpType )
        {
        struct in_addr IpAddr;
#ifdef __WIN32_SIM__
        IpAddr.S_un.S_addr = htonl( m_Data.stIpLease.ulIp );
#else
        IpAddr.s_addr = htonl( m_Data.stIpLease.ulIp );
#endif
        SINT8 strIp[ INET_ADDR_LEN ] = {0};
        inet_ntoa_b( IpAddr, strIp );

        printf( "\r\n\t%-10s: %s\
            \r\n\t%-10s: %s\
            \r\n\t%-10s: 0x%.8X  %s",
            "IpType", "DHCP",
            "IP", strIp,
            "Lease", m_Data.stIpLease.ulLease, ctime( (time_t*)&m_Data.stIpLease.ulLease ) );
        }

    else if ( IPTYPE_FIXIP == m_ucIpType )
        {
        struct in_addr IpAddr;
#ifdef __WIN32_SIM__
        IpAddr.S_un.S_addr = htonl( m_Data.stIpLease.ulIp );
#else
        IpAddr.s_addr = htonl( m_Data.stIpLease.ulIp );
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
            "IpType", m_ucIpType,
            "DATA", 
            ( (UINT8*)&m_Data )[0],
            ( (UINT8*)&m_Data )[1],
            ( (UINT8*)&m_Data )[2],
            ( (UINT8*)&m_Data )[3],
            ( (UINT8*)&m_Data )[4],
            ( (UINT8*)&m_Data )[5],
            ( (UINT8*)&m_Data )[6],
            ( (UINT8*)&m_Data )[7] );
        }
    return ;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopCCB::ResetCCB

DESCRIPTION:
    重置CCB.删除CCB暂存的消息,删除定时器

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopCCB::ResetCCB()
{
    //.
    DeleteMsg();
    DeleteTimer();
    stopHeartBeat();
    InitCCB();
}


/*============================================================
MEMBER FUNCTION:
    CSnoopCCB::InitCCB

DESCRIPTION:
    初始化CCB.

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopCCB::InitCCB()
{
//    m_bIsOccupied = false;
    m_ucIsAuthed = (UINT8)false;          /*has been authenticated? */
    m_ulEid = 0;
    memset( m_aucMac, 0, M_MAC_ADDRLEN );
    m_ucIpType = 0xFF;           /*0: DHCP; 1: PPPoE; 0xFF:无效值*/
    memset( (void*)&m_Data, 0, sizeof( DATA ) );
    m_ulRouterAreaId = M_RAID_INVALID;
    m_ucIsAnchor     = (UINT8)false;
    m_ucIsServing    = (UINT8)false;
    m_IsRealAnchor = (UINT8)true;//wangwenhua add 20081023
    m_ulAnchorBts    = 0;
    m_ulServingBts   = 0;
    m_pMsg           = NULL;
    m_pTimer         = NULL;
    m_pHeartBeatTimer= NULL;
    m_IsRcpeFlag = false;
}


//---------------------------
//---------------------------
void CSnoopCCB::SetMac(const UINT8 *pMac )
{
    if ( NULL == pMac )
        {
        DATA_assert( 0 );
        return;
        }
    memcpy( m_aucMac, pMac, M_MAC_ADDRLEN );
}


//---------------------------
//---------------------------
CMessage*  CSnoopCCB::SaveMsg(CMessage *pMsg ) 
{
    return m_pMsg = pMsg;
}


//---------------------------
//---------------------------
void CSnoopCCB::DeleteMsg()
{
    if ( NULL != m_pMsg )
        {
        m_pMsg->DeleteMessage();
        delete m_pMsg;
        m_pMsg = NULL;
        }
}


//---------------------------
//---------------------------
CTimer*  CSnoopCCB::SetTimer( CTimer *pTimer ) 
{
    return m_pTimer = pTimer;
}


//---------------------------
//---------------------------
void CSnoopCCB::DeleteTimer()
{
    if ( NULL != m_pTimer )
        {
        m_pTimer->Stop();
        delete m_pTimer;
        m_pTimer = NULL;
        }
}


//---------------------------
//心跳由anchorBTS往servingBTS方向发送
//servingBTS接收心跳并作心跳回应
//---------------------------
bool CSnoopCCB::startHeartBeat()
{
    if (IPTYPE_FIXIP != m_ucIpType)
        {
        return true;
        }
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL), "start Tunnel Heart Beat timer." );
    if ( (true == (bool)m_ucIsAnchor) && (false == (bool)m_ucIsServing) && (true == (bool)m_ucIsAuthed) )
        {
        stopHeartBeat();

        CSnoopTimerExpire msgTimerExpire;
        if ( false == msgTimerExpire.CreateMessage( *( CTSnoop::GetInstance() ) ) )
            {
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create snoop timer message failed." );
            return false;
            }
        msgTimerExpire.SetMessageId(MSGID_TIMER_HEARTBEAT);
        msgTimerExpire.SetMac( m_aucMac );
        msgTimerExpire.SetDstTid( M_TID_SNOOP );
        m_pHeartBeatTimer = new CTimer( true, 1000 * 60/*1 min*/, msgTimerExpire );
        if ( NULL == m_pHeartBeatTimer )
            {
            ////System Exception!
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create Timer failed" );
            //删除消息
            msgTimerExpire.DeleteMessage();
            return false;
            }

        if ( false == m_pHeartBeatTimer->Start() )
            {
            ////System Exception!
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Timer start err." );

            delete m_pHeartBeatTimer;
            m_pHeartBeatTimer = NULL;

            //删除消息；
            msgTimerExpire.DeleteMessage();
            return false;
            }        
        }

    return true;
}


//---------------------------
//---------------------------
void CSnoopCCB::stopHeartBeat()
{
    if ( NULL != m_pHeartBeatTimer)
        {
        m_pHeartBeatTimer->Stop();
        delete m_pHeartBeatTimer;
        m_pHeartBeatTimer = NULL;
        }
}


/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::showStatus

DESCRIPTION:
    打印状态机的属性

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopCCBTable::showStatus(UINT32 ulEid) 
{
    //CCB table Attributes.
    printf( "\r\n" );
    printf( "\r\n*************************" );
    printf( "\r\n*CCB table Attributes   *" );
    printf( "\r\n*************************" );

    if ( 0 == ulEid )
        {
        printf( "\r\nAll CCB information" );
        }
    else
        {
        printf( "\r\nEID[%d] CCB information*", ulEid );
        }

////#ifdef __WIN32_SIM__
    UINT8 ucFlowCtrl = 0;
    map<CMac, UINT16>::iterator it = m_CCBtree.begin();
    while ( m_CCBtree.end() != it )
        {
        //show..
        if ( 0 != ulEid ) 
            {
            CSnoopCCB *pSnoopCCB = GetCCBByIdx( it->second );
            if ( ( NULL == pSnoopCCB ) || ( ulEid != pSnoopCCB->GetEid() ) )
                {
                ++it;
                continue;
                }
            }

        showCCB( it->second );
        ++it;

        //流控
        if ( 200 == ++ucFlowCtrl )
            {
#ifdef __WIN32_SIM__
//Win32:
            ::Sleep( 100 );//释放CPU
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
////#endif
}

/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::showStatus

DESCRIPTION:
    根据mac,打印状态机的属性

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopCCBTable::showStatus(CMac &Mac) 
{	
	showCCB(Mac);
}

UINT32 CSnoopCCBTable::getIpByMAC(CMac &Mac)
{

	CSnoopCCB* ccb = GetCCBByIdx( BPtreeFind( Mac ) );
	DATA Data = ccb->GetData();
	if(ccb!=NULL)
		return Data.stIpLease.ulIp;
	else
		return 0xffffffff;
}
/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::InitFreeCCB

DESCRIPTION:
    初始化空闲CCB链表m_listFreeCCB

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopCCBTable::InitFreeCCB()
{
    m_listFreeCCB.clear();
    for ( UINT16 usIdx = 0; usIdx < SNOOP_CCB_NUM; ++usIdx )
        {
        m_listFreeCCB.push_back( usIdx );
        }
}


/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::GetFreeCCBIdxFromList

DESCRIPTION:
    从空闲链表m_listFreeCCB取空闲转发表表项下标;(从链表头部取)

ARGUMENTS:
    NULL

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CSnoopCCBTable::GetFreeCCBIdxFromList()
{
    if ( true == m_listFreeCCB.empty() )
        {
        g_snoop_no_freelist++;
        return M_DATA_INDEX_ERR;
        }

    UINT16 usIdx = *m_listFreeCCB.begin();
    m_listFreeCCB.pop_front();

    if ( M_MAX_USER_PER_BTS <= usIdx )
        {
        //下标错误
        g_snoop_no_freelist++;
        return M_DATA_INDEX_ERR;
        }
    g_snoop_no_freelist =0;
    return usIdx;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::InsertFreeCCB

DESCRIPTION:
    插入空闲Ip List表项下标到链表m_listFreeCCB尾部

ARGUMENTS:
    usIdx:表项下标

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopCCBTable::InsertFreeCCB(UINT16 usIdx )
{
    if( usIdx < SNOOP_CCB_NUM )
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
    CSnoopCCBTable::FindCCB

DESCRIPTION:
    查找CCB

ARGUMENTS:
    *pComMsg: 输入消息

RETURN VALUE:
    CCBBase*: CCB指针

SIDE EFFECTS:
    none
==============================================================*/
CCBBase* CSnoopCCBTable::FindCCB(CMessage &msg)
{
    CMac Mac( msg.GetKeyMac() );
    return GetCCBByIdx( BPtreeFind( Mac ) );
}


/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::GetCCBByIdx

DESCRIPTION:
    返回指定下标的CCB

ARGUMENTS:
    usIdx: CCB下标

RETURN VALUE:
    CCBBase*: CCB指针

SIDE EFFECTS:
    none
==============================================================*/
CSnoopCCB* CSnoopCCBTable::GetCCBByIdx(UINT16 usIdx)
{
    if ( !( usIdx < SNOOP_CCB_NUM ) )
        {
        return NULL;
        }
    return &( m_CCBTable[ usIdx ] );
}


/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::BPtreeAdd

DESCRIPTION:
    CCB索引树插入节点；CCB索引树以Mac地址作键值，把CCB
    下标插入到索引树

ARGUMENTS:
    Mac:Mac地址
    usIdx:表项下标

RETURN VALUE:
    bool:插入成功/失败

SIDE EFFECTS:
    如果存在重复项，将会返回失败，所以必须在BPtreeAdd之前保证
    没有重复。即首先 BPtreeFind 一下
==============================================================*/
bool CSnoopCCBTable::BPtreeAdd(CMac& Mac, UINT16 usIdx)
{
    pair<map<CMac, UINT16>::iterator, bool> stPair;

    stPair = m_CCBtree.insert( ValType( Mac, usIdx ) );
    /*
     *必须保证不存在重复项，否则将会返回失败
     */
    return stPair.second;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::BPtreeDel

DESCRIPTION:
    CCB索引树删除Mac对应的节点；

ARGUMENTS:
    Mac:Mac地址

RETURN VALUE:
    bool:删除成功/失败

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopCCBTable::BPtreeDel(CMac& Mac)
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
    CSnoopCCBTable::BPtreeFind

DESCRIPTION:
    从BPtree搜索Mac地址对应的CCB下标

ARGUMENTS:
    Mac:Mac地址

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CSnoopCCBTable::BPtreeFind(CMac& Mac)
{
    map<CMac, UINT16>::iterator it = m_CCBtree.find( Mac );
    
    if ( it != m_CCBtree.end() )
        {
        //返回Index.
        return it->second;
        }
    return M_DATA_INDEX_ERR;
}



/*============================================================
MEMBER FUNCTION:
    CSnoopCCBTable::showCCB

DESCRIPTION:
    打印指定下标的CCB

ARGUMENTS:
    usIdx: CCB下标

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopCCBTable::showCCB(UINT16 usIdx)
{
    CSnoopCCB *pSnoopCCB = GetCCBByIdx( usIdx );
    if ( NULL != pSnoopCCB )
        {
        pSnoopCCB->showCCB();
        }
}


void CSnoopCCBTable::groupModify(UINT32 ulEid, UINT16 usGid)
{
    map<CMac, UINT16>::iterator it = m_CCBtree.begin();
    UINT8 ucFlowCtrl = 0;
    CSnoopCCB *pCCB = NULL;
    while ( m_CCBtree.end() != it )
        {
        pCCB = GetCCBByIdx( it->second );
        if ( (NULL == pCCB) || (pCCB->GetEid() != ulEid) )
            {
            ++it;
            continue;
            }

        pCCB->setGroupId( usGid );

        //next.
        ++it;
        if ( 200 == ++ucFlowCtrl )
            {
            ::taskDelay( 1 );
            ucFlowCtrl = 0;
            }
        }
}

void CSnoopCCBTable::clearCPEData(UINT32 ulEid)
{
    map<CMac, UINT16>::iterator it = m_CCBtree.begin();
    UINT8 ucFlowCtrl = 0;
    CSnoopCCB *pCCB = NULL;
    while ( m_CCBtree.end() != it )
        {
        pCCB = GetCCBByIdx( it->second );
        if ( (NULL == pCCB) || (pCCB->GetEid() != ulEid) || (IPTYPE_FIXIP == pCCB->GetIpType()))
            {
            ++it;
            continue;
            }

        //给snoop任务发消息删除
        CDelILEntry msgDelIP;
        if (false == msgDelIP.CreateMessage(*CTSnoop::GetInstance()))
            return;
        msgDelIP.SetMac(pCCB->GetMac());
        msgDelIP.SetOp(M_UT_REMOVE);
        msgDelIP.SetDstTid(M_TID_SNOOP);
        if (false == msgDelIP.Post())
            {
            msgDelIP.DeleteMessage();
            }

        //next.
        ++it;
        if ( 200 == ++ucFlowCtrl )
            {
            ::taskDelay( 1 );
            ucFlowCtrl = 0;
            }
        }
}


bool CSnoopCCBTable::isLeaseUpdated(const UINT32 eid, const UINT32 leasetime, const CMac &macAddr)
{
	unsigned result = 0;
 	result =   CPE_IS_Rcpe(eid);
	if(result==1)
	{
	    LOG1(LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "CPE[0x%.8X] this is rcpe isLeaseUpdated1.  ", eid);
		return false;
	}
    CSnoopCCB *pCCB = GetCCBByIdx(BPtreeFind(const_cast<CMac&>(macAddr)));
     if(NULL==pCCB)
     	{
     	     return false;
     	}
	if (  (eid == pCCB->GetEid()) 
        || (IPTYPE_DHCP != pCCB->GetIpType())
        || (leasetime >= pCCB->GetData().stIpLease.ulLease))
        {
        //dhcp lease肯定没有更新
        LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "DHCP lease time is not updated when CPE[0x%.8X] register, it's normal", eid);
        return false;
        }

	
	 result =   CPE_IS_Rcpe(pCCB->GetEid());
	if(result==1)
	{
	    LOG2(LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "CPE[0x%.8X] this is rcpe isLeaseUpdated2. %08x\n ", eid,pCCB->GetEid());
		return false;
	}
    LOG1(LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "WARNING.DHCP lease time is renew from another CPE[0x%.8X], it's abnormal", pCCB->GetEid());
    return true;
}


#ifndef _NO_NVRAM_RECOVER_
CNVRamCCBTable *CSnoopFSM::s_pNVRamCCBTable = new CNVRamCCBTable;
#endif
/*============================================================
MEMBER FUNCTION:
    CSnoopFSM::CSnoopFSM

DESCRIPTION:
    CSnoopFSM构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CSnoopFSM::CSnoopFSM() : FSM( STATE_SNOOP_MAX, TRANS_SNOOP_MAX )
#ifndef _NO_NVRAM_RECOVER_
, m_bRecoverFinished( false )
#endif
{
    //CCB table.
#ifdef WBBU_CODE
    s_pNVRamCCBTable = new CNVRamCCBTable;
#endif
    m_pCCBTable             = new CSnoopCCBTable;

    //Parent State.
    CParentState *pParent   = new CParentState();

    //State table.
    m_pStateTable[ STATE_SNOOP_IDLE ]       = new CIdleState( *pParent );
    m_pStateTable[ STATE_SNOOP_SELECTING ]  = new CSelectingState( *pParent );
    m_pStateTable[ STATE_SNOOP_REQUESTING ] = new CRequestingState( *pParent );
    m_pStateTable[ STATE_SNOOP_SYNCING ]    = new CSyncingState( *pParent );
    m_pStateTable[ STATE_SNOOP_BOUND ]      = new CBoundState( *pParent );
    m_pStateTable[ STATE_SNOOP_RENEWING ]   = new CRenewingState( *pParent );
    m_pStateTable[ STATE_SNOOP_ROAMING ]    = new CRoamingState( *pParent );

    //Transition table.
    m_pTransTable[ TRANS_SNOOP_IDLE_PADI ]                      = new CIdlePADITrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_IDLE_DISC ]                      = new CIdleDiscoveryTrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_IDLE_TUNNEL_SYNC_REQ ]           = new CIdleTunnelSyncReqTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_IDLE_TUNNEL_TERMINATE_REQ ]      = new CIdleTunnelTerminateReqTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_IDLE_TUNNEL_CHANGE_ANCHOR_REQ ]  = new CIdleTunnelChgAnchorReqTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_IDLE_FIXIP_TUNNEL_ESTABLISH_REQ ]= new CIdleFixIpTunnelEstReqTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_IDLE_ADD_FIXIP ]                 = new CIdleAddFixIPTrans( STATE_SNOOP_BOUND );
    m_pTransTable[TRANS_SNOOP_IDLE_REQUEST]                      = new CIdleRequestTrans(STATE_SNOOP_REQUESTING);//wangwenhua add 20081016 for new state machine under idle state to cope with the DHCP msg request

    m_pTransTable[ TRANS_SNOOP_SELECTING_REQ ]                  = new CSelectingRequestTrans( STATE_SNOOP_REQUESTING );
    m_pTransTable[ TRANS_SNOOP_SELECTING_DISC ]                 = new CSelectingDiscoveryTrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_SELECTING_OFFER ]                = new CSelectingOfferTrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_SELECTING_PADR ]                 = new CSelectingPADRTrans( STATE_SNOOP_REQUESTING );
    m_pTransTable[ TRANS_SNOOP_SELECTING_PADI ]                 = new CSelectingPADITrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_SELECTING_PADO ]                 = new CSelectingPADOTrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_SELREQ_TIMEOUT ]                 = new CSelReqTimeOutTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_SELREQ_DELENTRY ]                = new CSelReqDelEntryTrans( STATE_SNOOP_IDLE );

    m_pTransTable[ TRANS_SNOOP_REQUESTING_ACK ]                 = new CRequestingACKTrans( STATE_SNOOP_SYNCING );
    m_pTransTable[ TRANS_SNOOP_REQUESTING_NAK ]                 = new CRequestingNAKTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_REQUESTING_REQ ]                 = new CRequestingRequestTrans( STATE_SNOOP_REQUESTING );
    m_pTransTable[ TRANS_SNOOP_REQUESTING_DISC ]                = new CRequestingDiscoveryTrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_REQUESTING_PADS ]                = new CRequestingPADSTrans( STATE_SNOOP_SYNCING );
    m_pTransTable[ TRANS_SNOOP_REQUESTING_PADR ]                = new CRequestingPADRTrans( STATE_SNOOP_REQUESTING );
    m_pTransTable[ TRANS_SNOOP_REQUESTING_PADI ]                = new CRequestingPADITrans( STATE_SNOOP_SELECTING );

    m_pTransTable[ TRANS_SNOOP_SYNCING_DHCP_SYNC_SUCCESS ]      = new CSyncingDHCPSyncOKTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_SYNCING_PPPoE_SYNC_SUCCESS ]     = new CSyncingPPPoESyncOKTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_SYNCING_FAIL ]                   = new CSyncingSyncFAILTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_SYNCING_PADI ]                   = new CSyncingPADITrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_SYNCING_DISC ]                   = new CSyncingDiscoveryTrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_SYNCING_DELENTRY ]               = new CSyncingDelEntryTrans( STATE_SNOOP_IDLE );

    m_pTransTable[ TRANS_SNOOP_BOUND_REQ_SELECTING ]            = new CBoundRequestInitTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_BOUND_REQ_RENEWING ]             = new CBoundRequestRenewTrans( STATE_SNOOP_RENEWING );
////m_pTransTable[ TRANS_SNOOP_BOUND_ACK ]                      = new CBoundACKTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_BOUND_PADR ]                     = new CBoundPADRTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_BOUND_PADS ]                     = new CBoundPADSTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_BOUND_PADT ]                     = new CBoundPADTTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_BOUND_TIMEOUT ]                  = new CBoundTimeOutTrans( STATE_SNOOP_IDLE );

    m_pTransTable[ TRANS_SNOOP_RENEWING_ACK ]                   = new CRenewingACKTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_RENEWING_NAK ]                   = new CRenewingNAKTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_RENEWING_REQ ]                   = new CRenewingRequestTrans( STATE_SNOOP_RENEWING );
    m_pTransTable[ TRANS_SNOOP_RENBND_DELENTRY ]                = new CRenBndDelEntryTrans( STATE_SNOOP_IDLE );

    m_pTransTable[ TRANS_SNOOP_ROAMING_TUNNEL_ESTABLISH_RESP ]  = new CRoamingTunnelEstablishRespTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_ROAMING_TIMEOUT ]                = new CRoamingTimeOutTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_ROAMING_DELENTRY ]               = new CRoamingDelEntryTrans( STATE_SNOOP_IDLE );

    m_pTransTable[ TRANS_SNOOP_PARENT_TUNNEL_SYNC_REQ ]         = new CParentTunnelSyncReqTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_PARENT_ROAM_REQ ]                = new CParentRoamReqTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_PARENT_TUNNEL_ESTABLISH_REQ ]    = new CParentTunnelEstablishReqTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_PARENT_TUNNEL_TERMINATE_REQ ]    = new CParentTunnelTerminateReqTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_PARENT_ADD_FIXIP ]               = new CParentAddFixIPTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_PARENT_TUNNEL_CHGANCHOR_RESP ]   = new CParentTunnelChgAnchorRespTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_PARENT_HEARTBEAT_TIMER]          = new CParentHeartBeatTimerTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_PARENT_HEARTBEAT]                = new CParentHeartBeatTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_PARENT_HEARTBEAT_RESP]           = new CParentHeartBeatRespTrans( STATE_SNOOP_BOUND );

    //有多个状态共用的transition.
    m_pTransTable[ TRANS_SNOOP_TUNNEL_ESTABLISH_REQ ]           = new CTunnelEstablishReqTrans( STATE_SNOOP_BOUND );
    m_pTransTable[ TRANS_SNOOP_TUNNEL_TERMINATE_REQ ]           = new CTunnelTerminateReqTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_TUNNEL_CHANGE_ANCHOR_REQ ]       = new CTunnelChangeAnchorReqTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_DISCOVERY ]                      = new CBRRDiscoveryTrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_PADI ]                           = new CBRPADITrans( STATE_SNOOP_SELECTING );
    m_pTransTable[ TRANS_SNOOP_RELEASE_DECLINE ]                = new CBoundRelDecTrans( STATE_SNOOP_IDLE );
    m_pTransTable[ TRANS_SNOOP_ENTRY_EXPIRE ]                   = new CEntryExpireTrans( STATE_SNOOP_IDLE );
}



/*============================================================
MEMBER FUNCTION:
    CSnoopFSM::FindCCB

DESCRIPTION:
    根据msg，找相应的CCB

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CCBBase* CSnoopFSM::FindCCB(CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Snoop FindCCB()" );
    /*
     *进状态机之前，所有用于查找CCB的Mac地址都放在消息头
     *的KeyMac处。
     */
    CMac Mac( msg.GetKeyMac() );
    if ( ( true == Mac.IsZero() ) || ( true == Mac.IsBroadCast() ) )
        {
        UINT8 strMac[ M_MACADDR_STRLEN ];
        LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Illegal Key Mac Address[%s].", (int)Mac.str( strMac ) );
        return NULL;
        }

    CSnoopCCB* pSnoopCCB = (CSnoopCCB*)m_pCCBTable->FindCCB( msg );
    if ( NULL != pSnoopCCB )
        {
        //find.
        return pSnoopCCB;
        }
    else
        {
        //NULL,Create CCB??
        UINT16 usFreeIdx = m_pCCBTable->GetFreeCCBIdxFromList();
        pSnoopCCB = m_pCCBTable->GetCCBByIdx( usFreeIdx );
        if ( NULL == pSnoopCCB )
            {
            //用户满
            UINT8 strMac[ M_MACADDR_STRLEN ];
            LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NO_CCB ), "Licence used up, refuse user:%s", (int)Mac.str( strMac ) );
            return NULL;
            }

        //Add to BPtree.
        pSnoopCCB->SetMac( Mac.GetMac() );
        pSnoopCCB->SetCurrentState( STATE_SNOOP_IDLE );
        m_pCCBTable->BPtreeAdd( Mac, usFreeIdx );
        return pSnoopCCB;
        }
}


/*============================================================
MEMBER FUNCTION:
    CSnoopFSM::Reclaim

DESCRIPTION:
    回收CCB，状态机在CCB的状态回到IDLE状态时，需要把CCB回收。
    从CCB BPtree删除，重置CCB，插入空闲CCB表

ARGUMENTS:
    *pCCB:  CCB

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopFSM::Reclaim(CCBBase *pCCB)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Snoop Reclaim()" );
    CSnoopCCB *pSnoopCCB = (CSnoopCCB*)pCCB;
    if ( STATE_SNOOP_IDLE != pSnoopCCB->GetCurrentState() )
        {
        return;
        }

    CMac Mac( pSnoopCCB->GetMac() );
    UINT16 usIdx = m_pCCBTable->BPtreeFind( Mac );
    DATA_assert( pSnoopCCB == m_pCCBTable->GetCCBByIdx( usIdx ) );
    m_pCCBTable->BPtreeDel( Mac );

    //Reset.
    //Delete Message and timer.
    pSnoopCCB->ResetCCB();

    //Reclaim.
    m_pCCBTable->InsertFreeCCB( usIdx );
    return;
}


#ifndef _NO_NVRAM_RECOVER_
/*============================================================
MEMBER FUNCTION:
    CSnoopFSM::RecoverUserInfoFromNVRAM

DESCRIPTION:
    从NVRAM恢复CCB.

ARGUMENTS:
    void

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopFSM::RecoverUserInfoFromNVRAM()
{
    if ( true == m_bRecoverFinished )
        {
        return;
        }
    
    m_bRecoverFinished = true;

    //重启前后，IP地址是否有改变
    //如果改变，不恢复NVRAM的用户
    UINT32 ulAnchorBts = ::bspGetBtsID();//::GetBtsIpAddr();//edit by yhw 
    if ( s_pNVRamCCBTable->GetLocalBts() != ulAnchorBts )
        {
        //第一次启动或重启前后Ip地址改变
        s_pNVRamCCBTable->SetLocalBts( ulAnchorBts );
        s_pNVRamCCBTable->clearCCB();
        return;
        }
   LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Snoop RecoverUserInfoFromNVRAM..................." );
    //逐项查询NVRAM
    stNVRamCCB *pNVRamCCB = s_pNVRamCCBTable->GetCCBTableBasePtr();
    for ( UINT16 idx = 0; idx < SNOOP_NVRAM_CCB_NUM; ++idx, ++pNVRamCCB )
        {
        if ( 0 == pNVRamCCB->ucIsOccupied )
            {
            //该表项空闲，插入空闲表项
            s_pNVRamCCBTable->InsertFreeCCB( idx );
            }
        else
            {
            UINT32 now = time( NULL );
            UINT8 ucIpType = pNVRamCCB->ucIpType;
            if((ucIpType>2)||(pNVRamCCB->usGroupID >4096))//对于type大于2不是DHCP\fixip,pppoe情况
            {
            	 pNVRamCCB->ucIsOccupied = 0;
                s_pNVRamCCBTable->InsertFreeCCB( idx );
                continue;
            }
            //如果rcpe标志不对，则不恢复
            if((pNVRamCCB->ucIsRcpeFlag!=0)&&(pNVRamCCB->ucIsRcpeFlag!=1))
            {
            	 pNVRamCCB->ucIsOccupied = 0;
                s_pNVRamCCBTable->InsertFreeCCB( idx );
                continue;
            }
            if ( ( IPTYPE_DHCP == ucIpType )
                &&( pNVRamCCB->Data.stIpLease.ulLease < now ) )
                {
                //,,过期用户,,不恢复；空闲之
                pNVRamCCB->ucIsOccupied = 0;
//UINT8 ucOccupied = 0;
//bspNvRamWrite( (char*)&( pNVRamCCB->ucIsOccupied ), (char*)&ucOccupied, sizeof( ucOccupied ) );
                s_pNVRamCCBTable->InsertFreeCCB( idx );
                continue;
                }
            if ((IPTYPE_FIXIP == ucIpType)
                &&(true == pNVRamCCB->ucIsServing))
                {
                //CPE配置了FixedIP, 如果BTS reboot
                //对于没有切换走的fixedip，不予恢复
                //否则如果该cpe不注册，将会导致这部分用户永远无法删除的情况。
                pNVRamCCB->ucIsOccupied = 0;
                s_pNVRamCCBTable->InsertFreeCCB( idx );
                continue;
                }

            //表项需要恢复到内存
            CMac Mac( pNVRamCCB->aucMac );
            UINT16 usFreeIdx     = m_pCCBTable->GetFreeCCBIdxFromList();
            CSnoopCCB* pSnoopCCB = m_pCCBTable->GetCCBByIdx( usFreeIdx );
            if ( NULL == pSnoopCCB )
                {
                //空闲之
                pNVRamCCB->ucIsOccupied = 0;
//UINT8 ucOccupied = 0;
//bspNvRamWrite( (char*)&( pNVRamCCB->ucIsOccupied ), (char*)&ucOccupied, sizeof( ucOccupied ) );
                s_pNVRamCCBTable->InsertFreeCCB( idx );
                continue;
                }
            m_pCCBTable->BPtreeAdd( Mac, usFreeIdx );
            s_pNVRamCCBTable->BPtreeAdd( Mac, idx );

            //恢复SNOOP;
            pSnoopCCB->SetCurrentState( STATE_SNOOP_BOUND );
            pSnoopCCB->SetIsAuthed( true );
            pSnoopCCB->SetEid( pNVRamCCB->ulEid );
            pSnoopCCB->SetMac( pNVRamCCB->aucMac );
            pSnoopCCB->SetIpType( ucIpType );
            pSnoopCCB->SetDATA( pNVRamCCB->Data );
            pSnoopCCB->SetRAID( s_pNVRamCCBTable->GetLastRAID() );
            pSnoopCCB->SetIsAnchor( pNVRamCCB->ucIsAnchor );
            pSnoopCCB->SetIsServing( pNVRamCCB->ucIsServing );
            //just for debug.{{
            DATA_assert(!((false == pNVRamCCB->ucIsServing)&&(false == pNVRamCCB->ucIsAnchor)));
            //just for debug.}}
            pSnoopCCB->SetAnchorBts( ulAnchorBts );
            pSnoopCCB->SetServingBts( pNVRamCCB->ulServingBts );
            pSnoopCCB->setGroupId( pNVRamCCB->usGroupID );
            if ( IPTYPE_DHCP == ucIpType )
                {
                CTimer *pTimer = StartTimer( pNVRamCCB->aucMac, 1000 * ( pNVRamCCB->Data.stIpLease.ulLease - now ) );
                if ( NULL == pTimer )
                    {
                    m_pCCBTable->BPtreeDel( Mac );
                    pSnoopCCB->ResetCCB();
                    m_pCCBTable->InsertFreeCCB( usFreeIdx );
                    //空闲之
                    s_pNVRamCCBTable->BPtreeDel( Mac );
                    pNVRamCCB->ucIsOccupied = 0;
//UINT8 ucOccupied = 0;
//bspNvRamWrite( (char*)&( pNVRamCCB->ucIsOccupied ), (char*)&ucOccupied, sizeof( ucOccupied ) );
                    s_pNVRamCCBTable->InsertFreeCCB( idx );
                    continue;
                    }
                pSnoopCCB->SetTimer( pTimer );
                }
             if((pSnoopCCB->GetIsServing()==false )&&(pSnoopCCB->IsExistTunnel() == false))//对于又不是服务基站又不存在隧道的情况进行删除
             {
                         m_pCCBTable->BPtreeDel( Mac );
                //删定时器，初始化CCB.
	                pSnoopCCB->ResetCCB();
	                m_pCCBTable->InsertFreeCCB( usFreeIdx );
	                //空闲之
	                s_pNVRamCCBTable->BPtreeDel( Mac );
	                pNVRamCCB->ucIsOccupied = 0;
	//UINT8 ucOccupied = 0;
	//bspNvRamWrite( (char*)&( pNVRamCCB->ucIsOccupied ), (char*)&ucOccupied, sizeof( ucOccupied ) );
	                s_pNVRamCCBTable->InsertFreeCCB( idx );
	                continue;
             }

            //恢复EB;
            if ( false == AddFTEntry( pSnoopCCB ) )
                {
                m_pCCBTable->BPtreeDel( Mac );
                //删定时器，初始化CCB.
                pSnoopCCB->ResetCCB();
                m_pCCBTable->InsertFreeCCB( usFreeIdx );
                //空闲之
                s_pNVRamCCBTable->BPtreeDel( Mac );
                pNVRamCCB->ucIsOccupied = 0;
//UINT8 ucOccupied = 0;
//bspNvRamWrite( (char*)&( pNVRamCCB->ucIsOccupied ), (char*)&ucOccupied, sizeof( ucOccupied ) );
                s_pNVRamCCBTable->InsertFreeCCB( idx );
                continue;
                }

            //恢复ARP;
            if ( IPTYPE_PPPoE != ucIpType )
                {
                //增加ARP的Ip List.
                ::ARP_AddILEntry( pNVRamCCB->ulEid, pNVRamCCB->aucMac, pNVRamCCB->Data.stIpLease.ulIp, pNVRamCCB->ucIsRcpeFlag);
                }
            if ((true == pSnoopCCB->IsExistTunnel()) && (true == pSnoopCCB->GetIsAnchor()))
                {
                //启动心跳定时器
                pSnoopCCB->startHeartBeat();
                }
            }
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopFSM::StartTimer

DESCRIPTION:
    通知DM,同步Ip List

ARGUMENTS:
    pMac:   定时器超时消息中携带的信息；
    ulMillSecs: 定时器时长(毫秒):

RETURN VALUE:
    CTimer*:返回定时器指针

SIDE EFFECTS:
    none
==============================================================*/
CTimer* CSnoopFSM::StartTimer(const UINT8 *pMac, UINT32 ulMillSecs )
{
    if ( NULL == pMac )
        {
        return NULL;
        }

    CSnoopTimerExpire msgTimerExpire;
    if ( false == msgTimerExpire.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        return NULL;
        }

    msgTimerExpire.SetMac( pMac );
    msgTimerExpire.SetDstTid( M_TID_SNOOP );
    CTimer *pTimer = new CTimer( false, ulMillSecs, msgTimerExpire );
    if ( NULL == pTimer )
        {
        //删除消息
        msgTimerExpire.DeleteMessage();
        return NULL;
        }

    if ( false == pTimer->Start() )
        {
        //删除消息
        msgTimerExpire.DeleteMessage();
        delete pTimer;
        return NULL;
        }
    return pTimer;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopFSM::AddFTEntry

DESCRIPTION:
    通知EB,增加FT表项

ARGUMENTS:
    *pMac:  Mac地址

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopFSM::AddFTEntry(CSnoopCCB *ccb)
{
    //Add FT Entry.
    if ( NULL == ccb )
        {
        return false;
        }

    CFTAddEntry msgFTAddEntry;
    if ( false == msgFTAddEntry.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        return false;
        }

    msgFTAddEntry.SetEidInPayload( ccb->GetEid() );
    msgFTAddEntry.SetMac( ccb->GetMac() );
    msgFTAddEntry.SetServing( ccb->GetIsServing() );
    msgFTAddEntry.SetTunnel( ccb->IsExistTunnel() );
    msgFTAddEntry.SetPeerBtsID( ccb->GetServingBts() );
    msgFTAddEntry.SetIpType( ccb->GetIpType() );
    msgFTAddEntry.setGroupId( ccb->getGroupId() );
    msgFTAddEntry.SetAuth( ccb->GetIsAuthed() );

    msgFTAddEntry.SetDstTid( M_TID_EB );

    if ( false == msgFTAddEntry.Post() )
        {
        msgFTAddEntry.DeleteMessage();
        return false;
        }

    return true;
}
#endif
