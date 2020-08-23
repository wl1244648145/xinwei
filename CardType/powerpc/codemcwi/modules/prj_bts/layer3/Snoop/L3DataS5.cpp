/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopTransPPPoE.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   04/10/06   xiao weifang  BTS不支持切换漫游需要的处理
 *   03/30/06   xiao weifang  NVRAM恢复用户信息. 
 *   09/12/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

//禁用Winsock 1
#define _WINSOCKAPI_
#include <time.h>

#include "Timer.h"
#include "LogArea.h"

#include "L3DataDM.h"
#include "L3DataSnoop.h"
#include "L3DataSnoopTransPPPoE.h"
#include "L3DataSnoopTimer.h"
#include "L3DataTunnelEstablish.h"
#include "L3DataTunnelTerminate.h"
#include "L3DataDelIL.h"
#include "L3DataTunnelChangeAnchor.h"
#include "L3DataSnoopErrCode.h"
#include "L3DataFTEntryExpire.h"
#ifdef UNITEST
//包含桩函数
#include "L3DataStub.h"
#endif

//externs.
extern "C" int bspGetBtsID();
extern "C" BOOL bspGetPPPoEpermit();
extern     UINT32 GetRouterAreaId();
extern     void   ARP_DelILEntry(UINT32 ulIp, UINT8 *pMac,UINT8 flag);

/*============================================================
MEMBER FUNCTION:
    CIdlePADITrans::Action

DESCRIPTION:
    Idle状态收PADI报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CIdlePADITrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADI Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], msg.GetEID() );
    DATA Data;
    memset( (void*)&Data, 0, sizeof( DATA ) );

    UINT8 *pMac  = ccb.GetMac();
    UINT32 ulEid = msg.GetEID();
    if (( false == IsUTAccessEnable( ulEid ) )
        || (false == bspGetPPPoEpermit()))
        {
        //用户已达到UT配置的容量，不允许该用户上线
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "User is not permit to access in EID:%X, check if user num reach MAX or PPPoE enabled.", ulEid );
        return STATE_SNOOP_IDLE;
        }

    UINT32 ulRaid = ( CTSnoop::GetInstance() )->GetRouterAreaId();
    if ( M_RAID_INVALID == ulRaid )
        {
        LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        return STATE_SNOOP_IDLE;
        }

    //Start timer.
    CTimer *pTimer = StartSnoopTimer( pMac, M_SNOOP_FTIMER_INTERVAL_8Sec );
    if ( NULL == pTimer )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start Snoop Timer failed." );
        return STATE_SNOOP_IDLE;
        }

    //ccb.
    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_PPPoE );
    ccb.SetDATA( Data );
    ccb.SetRAID( ulRaid );
    ccb.SetIsAnchor( true );
    ccb.SetIsServing( true );
    UINT32 ulLocalAddr = ::bspGetBtsID();
    ccb.SetAnchorBts( ulLocalAddr );
    ccb.SetServingBts( ulLocalAddr );
    //ccb.SaveMsg( NULL );
    ccb.SetTimer( pTimer );
    ccb.setGroupId( getGroupIDbyEid(ulEid) );

    //Send EB, Add FT Entry.
    AddFTEntry( ulEid,      /*UINT32 ulEid, */
        pMac,               /*const UINT8 * pMac, */
        true,               /*bool bIsServing, */
        false,              /*bool bIsTunnel, */
        0,                  /*UINT32 ulPeerBtsAddr, */
        IPTYPE_PPPoE,       /*UINT8 ucIpType, */
        false,              /*bool bIsAuthed*/
        ccb);            

    //Forward PADI to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}



/*============================================================
MEMBER FUNCTION:
    CSelectingPADITrans::Action

DESCRIPTION:
    Selecting状态收PADI报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelectingPADITrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADI Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    if (false == bspGetPPPoEpermit())
        {
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "User is not permit to access in EID:%X, check if PPPoE is enabled.", ccb.GetEid() );
        return ccb.GetCurrentState();
        }
    ForwardTrafficToWan( msg );
    if (IPTYPE_PPPoE != ccb.GetIpType())
        {
        //原来是DHCP用户,需要改变ccb的iptype;以及转发表的ttl
        UINT32 ulEid = msg.GetEID();
        UINT8 *pMac  = ccb.GetMac();
        CTimer *pTimer = ccb.GetTimer();
        if ( NULL != pTimer )
            {
            pTimer->Stop();
            pTimer->Start();
            }

        //Set CCB
        ccb.SetIsAuthed( false );
        ccb.SetEid( ulEid );
////////ccb.SetMac( pMac );
        ccb.SetIpType( IPTYPE_PPPoE );
////////ccb.SetDATA( Data );
////////ccb.SetRAID( ulRaid );
        ccb.SetIsAnchor( true );
        ccb.SetIsServing( true );
        UINT32 ulLocalAddr = ::bspGetBtsID();
        ccb.SetAnchorBts( ulLocalAddr );
        ccb.SetServingBts( ulLocalAddr );
        ccb.SetTimer( pTimer );
        ccb.setGroupId( getGroupIDbyEid(ulEid) );

        //Notify EB, Add FT Entry.
        AddFTEntry( ulEid,      /*UINT32 ulEid, */
            pMac,               /*const UINT8 * pMac, */
            true,               /*bool bIsServing, */
            false,              /*bool bIsTunnel, */
            0,                  /*UINT32 ulPeerBtsAddr, */
            IPTYPE_PPPoE,       /*UINT8 ucIpType, */
            false,              /*bool bIsAuthed*/
            ccb);            
        }
    return TargetState;
}



/*============================================================
MEMBER FUNCTION:
    CSelectingPADOTrans::Action

DESCRIPTION:
    Selecting状态收PADO报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelectingPADOTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADO Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    ForwardTrafficToAI( msg, ccb.GetEid() );
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSelReqTimeOutTrans::Action

DESCRIPTION:
    Selecting/Request状态定时器超时的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelReqTimeOutTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Timer Expire Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    //Delete timer.
    ccb.DeleteTimer();

    //Reclaim CCB.
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSelReqDelEntryTrans::Action

DESCRIPTION:
    Selecting/Request状态删除CCB表项的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelReqDelEntryTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Del Entry Action(eid:0x%x)",(int) strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    //Delete timer.
    ccb.DeleteTimer();

    //Reclaim CCB.
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSelectingPADRTrans::Action

DESCRIPTION:
    Selecting状态收PADR报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelectingPADRTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADR Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    ForwardTrafficToWan( msg );
    //状态-->Requesting
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingPADITrans::Action

DESCRIPTION:
    Requesting状态收PADI报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRequestingPADITrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADI Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    DATA Data;
    memset( (void*)&Data, 0, sizeof( DATA ) );
    UINT32 ulEid = msg.GetEID();
    UINT8 *pMac  = ccb.GetMac();

    if (false == bspGetPPPoEpermit())
        {
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "User is not permit to access in EID:%X, check if PPPoE is enabled.", ulEid );
        return ccb.GetCurrentState();
        }

    UINT32 ulRaid = ( CTSnoop::GetInstance() )->GetRouterAreaId();
    if ( M_RAID_INVALID == ulRaid )
        {
        LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        return STATE_SNOOP_IDLE;
        }

    CTimer *pTimer = ccb.GetTimer();
    if ( NULL != pTimer )
        {
        pTimer->Stop();
        pTimer->Start();
        }

    //Set CCB
    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_PPPoE );
    ccb.SetDATA( Data );
    ccb.SetRAID( ulRaid );
    ccb.SetIsAnchor( true );
    ccb.SetIsServing( true );
    UINT32 ulLocalAddr = ::bspGetBtsID();
    ccb.SetAnchorBts( ulLocalAddr );
    ccb.SetServingBts( ulLocalAddr );
    ccb.SetTimer( pTimer );
    ccb.setGroupId( getGroupIDbyEid(ulEid) );

    //Notify EB, Add FT Entry.
    AddFTEntry( ulEid,      /*UINT32 ulEid, */
        pMac,               /*const UINT8 * pMac, */
        true,               /*bool bIsServing, */
        false,              /*bool bIsTunnel, */
        0,                  /*UINT32 ulPeerBtsAddr, */
        IPTYPE_PPPoE,       /*UINT8 ucIpType, */
        false,              /*bool bIsAuthed*/
        ccb);            

    //Forward Discovery to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingPADRTrans::Action

DESCRIPTION:
    Requesting状态收PADR报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRequestingPADRTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADR Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    ForwardTrafficToWan( msg );
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingPADSTrans::Action

DESCRIPTION:
    Requesting状态收PADS报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRequestingPADSTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADS Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    EtherHdr *pEther = (EtherHdr*)msg.GetDataPtr();
	PPPoEHdr *pPPPoE;
	if(IS_8023_PACKET(ntohs(pEther->usProto)))
	{
		pPPPoE = (PPPoEHdr*)( (UINT8*)pEther + sizeof( EtherHdr ) + sizeof(LLCSNAP) );
	}
	else
	{
		pPPPoE = (PPPoEHdr*)( (UINT8*)pEther + sizeof( EtherHdr ) );
	}

    UINT16 usSessionId = ntohs( pPPPoE->usSessionId );
    if ( 0 == usSessionId )
        {
        //NAKed.
        //Forward packet to AI.
        ForwardTrafficToAI( msg, ccb.GetEid() );
        //Notify EB, Deletet FT Entry.
        DelFTEntry( ccb.GetMac() );

        //Reclaim CCB.
        return STATE_SNOOP_IDLE;
        }
    else
        {
        //ACKed.
        DATA Data;
        Data.stSessionMac.usSessionId = usSessionId;
        memcpy( (void*)&( Data.stSessionMac.aucServerMac ), pEther->aucSrcMAC, M_MAC_ADDRLEN );
        //Set CCB.
        ccb.SetDATA( Data );

        //修改定时器
        CTimer *pTimer = ccb.GetTimer();
        if ( NULL != pTimer )
            {
            pTimer->Stop();
            pTimer->SetInterval( M_SNOOP_SYNC_INTERVAL_2Sec );
            pTimer->Start();
            }
        else
            {
            //???异常
            pTimer = StartSnoopTimer( ccb.GetMac(), M_SNOOP_SYNC_INTERVAL_2Sec );
            if ( NULL == pTimer )
                {
                LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start Snoop Timer failed." );
                return STATE_SNOOP_IDLE;
                }
            //Set timer.
            ccb.SetTimer( pTimer );
            }

        //Start Synchronization.
        Synchronize( M_SYNC_ADD, true, ccb );

        //make copy.
        CMessage *pMsg = msg.Clone();
        //Save PADS Packet.
        ccb.SaveMsg( pMsg );

        return TargetState;
        }
}


/*============================================================
MEMBER FUNCTION:
    CSyncingPADITrans::Action

DESCRIPTION:
    Syncing状态收PADI报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSyncingPADITrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADI Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if (false == bspGetPPPoEpermit())
        {
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "User is not permit to access in EID:%X, check if PPPoE is enabled.", ccb.GetEid() );
        return ccb.GetCurrentState();
        }

    //Delete. Synchronize.
    Synchronize( M_SYNC_DELETE, false, ccb );

    //释放Syncing状态保存的消息
    ccb.DeleteMsg();

    DATA Data;
    memset( (void*)&Data, 0, sizeof( DATA ) );
    UINT32 ulEid = msg.GetEID();
    UINT8 *pMac  = ccb.GetMac();

    //Router Area Id
    UINT32 ulRaid = ( CTSnoop::GetInstance() )->GetRouterAreaId();
    if ( M_RAID_INVALID == ulRaid )
        {
        LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        return STATE_SNOOP_IDLE;
        }

    //修改定时器
    CTimer *pTimer = ccb.GetTimer();
    if ( NULL != pTimer )
        {
        pTimer->Stop();
        pTimer->SetInterval( M_SNOOP_FTIMER_INTERVAL_8Sec );
        pTimer->Start();
        }
    else
        {
        //???异常，重新创建定时器
        pTimer = StartSnoopTimer( ccb.GetMac(), M_SNOOP_FTIMER_INTERVAL_8Sec );
        if ( NULL == pTimer )
            {
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start Snoop Timer failed." );
            return STATE_SNOOP_IDLE;
            }
        ccb.SetTimer( pTimer );
        }

    //Set CCB.
    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_PPPoE );
    ccb.SetDATA( Data );
    ccb.SetRAID( ulRaid );
    ccb.SetIsAnchor( true );
    ccb.SetIsServing( true );
    UINT32 ulLocalAddr = ::bspGetBtsID();
    ccb.SetAnchorBts( ulLocalAddr );
    ccb.SetServingBts( ulLocalAddr );
    ccb.setGroupId( getGroupIDbyEid(ulEid) );

    //Notify EB, Add FT Entry.
    AddFTEntry( ulEid,      /*UINT32 ulEid, */
        pMac,               /*const UINT8 * pMac, */
        true,               /*bool bIsServing, */
        false,              /*bool bIsTunnel, */
        0,                  /*UINT32 ulPeerBtsAddr, */
        IPTYPE_PPPoE,       /*UINT8 ucIpType, */
        false,              /*bool bIsAuthed*/
        ccb);            

    //Forward PADI to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSyncingPPPoESyncOKTrans::Action

DESCRIPTION:
    Syncing状态PPPoE用户收Synchronize OK消息的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSyncingPPPoESyncOKTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PPPoE SYNCOK Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    CMessage *pMsg = (CMessage*)ccb.GetMsg();
    if ( NULL == pMsg )
        {
        //异常
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), "Find no PADS packet to forward when synchronize success." );
        return STATE_SNOOP_IDLE;
        }

    //forward traffic.
    ForwardTrafficToAI( *pMsg, ccb.GetEid() );

    //Delete timer.
    ccb.DeleteTimer();

    //Delete packet.
    ccb.DeleteMsg();

    //CCB认证成功
    ccb.SetIsAuthed( true );

    //更新PPPoE用户转发表
    UpdateFTEntry( ccb, IPTYPE_PPPoE );

#ifndef _NO_NVRAM_RECOVER_
    //更新到NVRam.
    AddToNVRAM( ccb );
#endif

    //状态-->BOUND
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSyncingSyncFAILTrans::Action

DESCRIPTION:
    Syncing状态收Synchronize FAIL消息的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSyncingSyncFAILTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s SYNC Fail Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    if ( IPTYPE_DHCP == ccb.GetIpType() )
        {
        //DHCP
        if ( true == ccb.GetIsAnchor() )
            {
            //Anchor BTS
            //调用ARP提供的接口删除IP List
            if ( true == ccb.GetIsAuthed() )
                {
                ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );

#ifndef _NO_NVRAM_RECOVER_
                //更新到NVRam.
                DelFromNVRAM( ccb );
#endif
                }
            }
        else
            {
            //Serving BTS
            //往Anchor发Sync(0)
            SendTunnelSync( ccb, true );
            }
        }

    //PPPoE 或其他情况
    ccb.DeleteMsg();

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSyncingDelEntryTrans::Action

DESCRIPTION:
    Syncing状态收Delete Entry消息的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSyncingDelEntryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Del Entry Action(eid:0x%x)",(int) strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    CDelILEntry msgDelILEntry( msg );
    UINT8 ucDelType = msgDelILEntry.GetOp();
    if ( ( true == ccb.GetIsAuthed() )
        && ( true == ccb.GetIsAnchor() )
        && ( M_UT_MOVE_AWAY == ucDelType ) )
        {
        //UT漫游离开,Anchor BTS已经认证的,,不删除
        return ccb.GetCurrentState();
        }

    if ( IPTYPE_DHCP == ccb.GetIpType() )
        {
        //DHCP,SYNCING状态不考虑Fix IP的情况
        if ( true == ccb.GetIsAnchor() )
            {
            //Anchor BTS
            //调用ARP提供的接口删除IP List
            if ( true == ccb.GetIsAuthed() )
                {
                ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );

#ifndef _NO_NVRAM_RECOVER_
                //更新到NVRam.
                DelFromNVRAM( ccb );
#endif
                }
            }
        else if ( M_UT_REMOVE == ucDelType )
            {
            //往Anchor发Sync(0)
            SendTunnelSync( ccb, true );
            }
        }

    //PPPoE 或其他情况
    ccb.DeleteMsg();

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CBoundPADRTrans::Action

DESCRIPTION:
    Bound状态收PADR报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundPADRTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADR Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    ForwardTrafficToWan( msg );
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CBoundPADSTrans::Action

DESCRIPTION:
    Bound状态收PADS报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundPADSTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADS Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    ForwardTrafficToAI( msg, ccb.GetEid() );
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CBRPADITrans::Action

DESCRIPTION:
    Bound/ROAMING状态收PADI报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBRPADITrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADI Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    if (false == bspGetPPPoEpermit())
        {
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "User is not permit to access in EID:%X, check if PPPoE is enabled.", ccb.GetEid() );
        return ccb.GetCurrentState();
        }

    if ( IPTYPE_FIXIP == ccb.GetIpType() )
        {
        //FIXIP用户不处理该消息
        return ccb.GetCurrentState();
        }

////如果原来的DHCP用户改成PPPoE拨号，PPPoE用户优先使用。
    if ( false == ccb.GetIsAnchor() )
        {
        //Serving BTS
        //往Anchor发Sync(0)
        SendTunnelSync( ccb, true );
        }
#ifndef _NO_NVRAM_RECOVER_
    else
        {
        //更新到NVRam.
        DelFromNVRAM( ccb );
        }
#endif
    if ( IPTYPE_DHCP== ccb.GetIpType() )
    {
            //delete arp proxy lijinan 20110222
            LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "pppoe padi ,have dhcp delete arp proxy" );
            ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);
    }

    //Delete. Synchronize.
    Synchronize( M_SYNC_DELETE, false, ccb );

    DATA Data;
    memset( (void*)&Data, 0, sizeof( DATA ) );
    UINT32 ulEid = msg.GetEID();
    UINT8 *pMac  = ccb.GetMac();

    //Router Area Id
    UINT32 ulRaid = ( CTSnoop::GetInstance() )->GetRouterAreaId();
    if ( M_RAID_INVALID == ulRaid )
        {
        LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        return STATE_SNOOP_IDLE;
        }

    //修改定时器
    CTimer *pTimer = ccb.GetTimer();
    if ( NULL != pTimer )
        {
        pTimer->Stop();
        pTimer->SetInterval( M_SNOOP_FTIMER_INTERVAL_8Sec );
        pTimer->Start();
        }
    else
        {
        //重新创建定时器
        pTimer = StartSnoopTimer( ccb.GetMac(), M_SNOOP_FTIMER_INTERVAL_8Sec );
        if ( NULL == pTimer )
            {
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start Snoop Timer failed." );
            return STATE_SNOOP_IDLE;
            }
        ccb.SetTimer( pTimer );
        }

    //Set CCB.
    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_PPPoE );
    ccb.SetDATA( Data );
    ccb.SetRAID( ulRaid );
    ccb.SetIsAnchor( true );
    ccb.SetIsServing( true );
    UINT32 ulLocalAddr = ::bspGetBtsID();
    ccb.SetAnchorBts( ulLocalAddr );
    ccb.SetServingBts( ulLocalAddr );
    ccb.setGroupId( getGroupIDbyEid(ulEid) );

    //Notify EB, Add FT Entry.
    AddFTEntry( ulEid,       /*UINT32 ulEid, */
        pMac,               /*const UINT8 * pMac, */
        true,               /*bool bIsServing, */
        false,              /*bool bIsTunnel, */
        0,                  /*UINT32 ulPeerBtsAddr, */
        IPTYPE_PPPoE,       /*UINT8 ucIpType, */
        false,              /*bool bIsAuthed*/
        ccb);            

    //Forward PADI to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}

/*============================================================
MEMBER FUNCTION:
    CBoundPADTTrans::Action

DESCRIPTION:
    Bound状态收PADT报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundPADTTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    if ( IPTYPE_PPPoE != ccb.GetIpType() )
        {
        //FIXIP用户不处理该消息
        return ccb.GetCurrentState();
        }

    UINT8 ucDir = msg.GetDirection();
    bool  bIsAnchor = ccb.GetIsAnchor();
    switch ( ucDir )
        {
        case DIR_FROM_AI:
                //Ingress.
                LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADT(Ingress) Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
                if ( true == bIsAnchor )
                    {
                    //AnchorBTS Ingress.
                    msg.SetDirection( DIR_TO_WAN );
                    }
                else
                    {
                    //ServingBTS Ingress PADT.
                    //Forward to AnchorBTS.
                    msg.SetDirection( DIR_TO_TDR );
                    msg.SetBTS( ccb.GetAnchorBts() );
                    }
            break;

        case DIR_FROM_WAN:
                //Egress.
                LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADT(Egress) Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
                if ( true == ccb.GetIsServing() )
                    {
                    //Anchor BTS.
                    //转发给UT
                    msg.SetDirection( DIR_TO_AI );
                    //Eid?
                    msg.SetEID( ccb.GetEid() );
                    }
                else
                    {
                    //UT已经漫游离开Anchor BTS.
                    msg.SetDirection( DIR_TO_TDR );
                    //转发给Serving BTS.
                    msg.SetBTS( ccb.GetServingBts() );
                    }
            break;

        case DIR_FROM_TDR:
                //PADT From Tunnel.
                LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADT(from Tunnel) Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
                if ( true == ccb.GetIsServing() )
                    {
                    //应该是PPPoE Server给Client发的PADT
                    //转发给UT
                    msg.SetDirection( DIR_TO_AI );
                    //Eid?
                    msg.SetEID( ccb.GetEid() );
                    }
                else
                    {
                    //应该是PPPoE Client给Server发的PADT
                    //转发给WAN
                    msg.SetDirection( DIR_TO_WAN );
                    }
            break;

        default:
            //其他不处理
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), "Err. Ambiguous direction found in PADT snoop request." );
            return STATE_SNOOP_BOUND;
        }

    //通知EB,转发PADT.
    ForwardTraffic( msg );

    if ( false == bIsAnchor )
        {
        //Not Anchor BTS.
        //往AnchorBTS发SYNC(0)
        SendTunnelSync( ccb, true );
        }
#ifndef _NO_NVRAM_RECOVER_
    else
        {
        //更新到NVRam.
        DelFromNVRAM( ccb );
        }
#endif

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    //Delete and Synchronize
    Synchronize( M_SYNC_DELETE, false, ccb );

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    //Reclaim CCB.
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CBoundTimeOutTrans::Action

DESCRIPTION:
    Bound状态定时器超时的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundTimeOutTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Timer Expire Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    DATA_assert( IPTYPE_DHCP == ccb.GetIpType() );
    if ( true == ccb.GetIsAnchor() )
        {
        //调用ARP提供的接口删除IP List
        ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );

#ifndef _NO_NVRAM_RECOVER_
        //更新到NVRam.
        DelFromNVRAM( ccb );
#endif
        }
    else
        {
        //往Anchor发Sync(0)
        SendTunnelSync( ccb, true );
        }

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    //Start Synchronization.
    Synchronize( M_SYNC_DELETE, false, ccb );

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    //Reclaim CCB.
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CTunnelEstablishReqTrans::Action

DESCRIPTION:
    Tunnel Establish Req消息的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CTunnelEstablishReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Tunnel Establish Request Action(eid:0x%x)", 
          (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    UINT32 tt = (UINT32)time( NULL );
	#if 0
    if((tt -ccb.getCreateTime())<=2)//如果和注册消息差2秒则认为是延迟消息，丢弃
    {
        LOG2( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->->%s Tunnel Establish Request Action(eid:0x%x), time <=2, discard msg", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid());
        return ccb.GetCurrentState();
    }
	#endif
    CTunnelEstablish msgTunnelEstablish (msg);
    UINT32 ulFromBtsID = msgTunnelEstablish.GetSenderBtsID();

    if ( false == IsBTSsupportMobility( GetRouterAreaId(), ccb.GetRAID(), ccb.GetAnchorBts() ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "[FIXIP] BTS mobility disabled" );
        //回 失败响应
        SendTunnelEstablishResponse( ccb, ulFromBtsID,msgTunnelEstablish.GetSenderBtsIP(),msgTunnelEstablish.GetSenderPort(), false );
        return ccb.GetCurrentState();
        }

    if ( false == ccb.GetIsAnchor() )
        {
        //如果不是AnchorBTS, 不处理，返回原状态
        //回 失败响应
        SendTunnelEstablishResponse( ccb, ulFromBtsID,msgTunnelEstablish.GetSenderBtsIP(),msgTunnelEstablish.GetSenderPort(), false );
        return ccb.GetCurrentState();
        }

    //是否存在隧道?
    UINT8 ucIpType = ccb.GetIpType();
    if ( ( true == ccb.IsExistTunnel() )
        && ( IPTYPE_FIXIP != ucIpType ) &&(ulFromBtsID!=ccb.GetServingBts()))/***wangwenhua add condition ,***/
        {
        //AnchorBTS还和其他BTS有隧道，需要先拆除
        //如果该隧道建立请求来自同一个服务级站的话,则没有必要进行终止.
         LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ),"ulFromBtsID:%x,%x\n",ulFromBtsID,ccb.GetServingBts());
        SendTunnelTerminate( ccb );;
        }

    ////
    ccb.SetIsServing( false );
    //设置ServingBTS
    ccb.SetServingBts( ulFromBtsID );
    //修改group
    ccb.setGroupId( msgTunnelEstablish.GetGroupId() );

    //Delete Eid Table.
    NotifyDelEidTable( ccb.GetEid() );

    //Update FT Entry.
    UpdateFTEntry( ccb, ucIpType );

    //Response.
    SendTunnelEstablishResponse( ccb, ulFromBtsID, msgTunnelEstablish.GetSenderBtsIP(), msgTunnelEstablish.GetSenderPort(), true );

#ifndef _NO_NVRAM_RECOVER_
    //更新到NVRam.
    AddToNVRAM( ccb );
#endif

    ccb.startHeartBeat();

    //不改变状态
    return ccb.GetCurrentState();
}


/*============================================================
MEMBER FUNCTION:
    CTunnelTerminateReqTrans::Action

DESCRIPTION:
    Tunnel Terminate Req消息的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CTunnelTerminateReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Tunnel Terminate Request Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    UINT32 tt = (UINT32)time( NULL );
	#if 0
    if((tt -ccb.getCreateTime())<=2)//如果和注册消息差2秒则认为是延迟消息，丢弃
    {
        LOG2( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->->%s Tunnel Terminate Request Action(eid:0x%x), time <=2, discard msg", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid());
        return ccb.GetCurrentState();
    }
	#endif
    DATA_assert( false == ccb.GetIsAnchor() );

    CTunnelTerminate msgTunnelTerminate( msg );
    UINT8 *pMac = msgTunnelTerminate.GetMac();

    //Delete FT Entry.
    DelFTEntry( pMac );

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    //Delete Eid Table.
    NotifyDelEidTable( ccb.GetEid() );

    //Response Tunnel Terminate.
    SendTunnelTerminateResponse( ccb, msgTunnelTerminate.GetSenderBtsID(),msgTunnelTerminate.GetSenderBtsIP(),msgTunnelTerminate.GetSenderPort(), true );

    //Reclaim CCB
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CTunnelChangeAnchorReqTrans::Action

DESCRIPTION:
    Tunnel Change Anchor Req消息的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CTunnelChangeAnchorReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    UINT32 tt = (UINT32)time( NULL );
	#if 0
    if((tt -ccb.getCreateTime())<=2)//如果和注册消息差2秒则认为是延迟消息，丢弃
    {
        LOG2( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->->%s Tunnel Change Anchor Request Action(eid:0x%x), time <=2, discard msg", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid());
        return ccb.GetCurrentState();
    }
	#endif
    CTunnelChangeAnchor msgTunnelChangeAnchorReq( msg );
    //Ip Type.
    UINT8 ucIpType = msgTunnelChangeAnchorReq.GetIpType();
    UINT32 ulFromBtsID = msgTunnelChangeAnchorReq.GetSenderBtsID();
  LOG3( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Tunnel Change Anchor Request Action(eid:0x%x),FromBTS:%x",
             (int) strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() ,ulFromBtsID);
    if ( false == ccb.GetIsAnchor() )
        {
        //不是AnchorBTS,异常
        //不处理，Response Success.
        //edit by yhw
        SendTunnelChangeAnchorResponse( ccb, ulFromBtsID, msgTunnelChangeAnchorReq.GetSenderBtsIP(),msgTunnelChangeAnchorReq.GetSenderPort(), true );
        //不改变状态
        return ccb.GetCurrentState();
        }
    else
        {
        //Anchor BTS.
        //创建临时隧道，10s后删除 FT Entry.
        DelFTEntry( ccb.GetMac(), true,  msgTunnelChangeAnchorReq.GetSenderBtsIP(), msgTunnelChangeAnchorReq.GetSenderPort() );

        //Change Anchor之前是否与其他BTS有隧道?
        if ( ( true == ccb.IsExistTunnel() )
            && ( IPTYPE_FIXIP != ucIpType ) )
            {
            //有隧道,拆除隧道
            SendTunnelTerminate( ccb );
            }

        if ( IPTYPE_PPPoE != ucIpType )
            {
            //DHCP或Fix IP用户，删ARP 的IP List
            ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);
            }

#ifndef _NO_NVRAM_RECOVER_
        //更新到NVRam.
        DelFromNVRAM( ccb );
#endif

        //Delete Eid Table.
        NotifyDelEidTable( ccb.GetEid() );

        //Delete timer.
        ccb.DeleteTimer();
        ccb.stopHeartBeat();
        }

    //Response Tunnel Change Anchor.
    SendTunnelChangeAnchorResponse( ccb, ulFromBtsID, msgTunnelChangeAnchorReq.GetSenderBtsIP(),msgTunnelChangeAnchorReq.GetSenderPort(),true );

    //Reclaim CCB.
    return TargetState;
}

/*============================================================
MEMBER FUNCTION:
    CRoamingTunnelEstablishRespTrans::Action

DESCRIPTION:
    Roaming状态收Tunnel Establish Response报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRoamingTunnelEstablishRespTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    CTunnelEstablishResp msgTunnelEstabishResp( msg );
    bool bSuccess = msgTunnelEstabishResp.GetResult();
    LOG4( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
           "->%s Tunnel Establish Response (%s) Action(eid:0x%x,%d)", (int)strSTATE[ ccb.GetCurrentState() ], (int)(bSuccess?"success":"fail"), ccb.GetEid(),bSuccess );
    if ( true == bSuccess )
        {
        //隧道建立成功
        //Delete timer.
        ccb.DeleteTimer();
        ccb.stopHeartBeat();
        if ( IPTYPE_DHCP == ccb.GetIpType() )
            {
            ////PPPoE和Fix IP用户都不要定时器
            UINT32 ulMillSecs = 1000 * ( ccb.GetData().stIpLease.ulLease - time( NULL ) );
            CTimer *pTimer = StartSnoopTimer( ccb.GetMac(), ulMillSecs );
            if ( NULL == pTimer )
                {
                LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start Snoop Timer failed." );
                return STATE_SNOOP_IDLE;
                }
            ccb.SetTimer( pTimer );
            }

        //认证通过??
        ccb.SetIsAuthed( true );

        ////
        ccb.SetIsServing( true );
        ccb.SetServingBts( ::bspGetBtsID() );

        //-->BOUND.
        return TargetState;
        }
    else
        {
        //隧道建立失败
        LOG1( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Rx tunnel establish FAIL respose Eid[%08x].",ccb.GetEid());
        //Synchronize.
        if ( IPTYPE_FIXIP != ccb.GetIpType() )
            {
            //FixIP不删除DM的IPLIST.
            Synchronize( M_SYNC_DELETE, false, ccb );
            }

        //Delete FT Entry.
        DelFTEntry( ccb.GetMac() );

        //Delete timer.
        ccb.DeleteTimer();
        ccb.stopHeartBeat();

        //Reclaim CCB.
        return STATE_SNOOP_IDLE;
        }
}


/*============================================================
MEMBER FUNCTION:
    CRoamingTimeOutTrans::Action

DESCRIPTION:
    Roaming状态定时器超时的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRoamingTimeOutTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Timer Expire Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    if ( IPTYPE_FIXIP != ccb.GetIpType() )
        {
        //Start Synchronization.
        Synchronize( M_SYNC_DELETE, false, ccb );
        }

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    LOG3( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Rx no tunnel response, please check if peer BTS is reachable eid[%08x],anchor BTS[%08x],serving BTS[%08x]", ccb.GetEid(),ccb.GetAnchorBts(),ccb.GetServingBts());

    //Reclaim CCB.
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRoamingDelEntryTrans::Action

DESCRIPTION:
    Roaming状态Delete Entry的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRoamingDelEntryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Del Entry Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    CDelILEntry msgDelILEntry( msg );
    UINT8 ucDelType = msgDelILEntry.GetOp();

    //Roaming状态只是在ServingBTS上才能进入
    //ServingBTS的IPLIST.
    if ( M_UT_REMOVE == ucDelType )
        {
        //UT注销的情况,,往Anchor发Sync(0)
        SendTunnelSync( ccb, true );
        }
    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    //Reclaim CCB.
    return TargetState;
}



/*============================================================
MEMBER FUNCTION:
    CEntryExpireTrans::Action

DESCRIPTION:
    Mac对应的转发表表项超时处理，PPPoE/FIXIP用户表项超时发出的

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CEntryExpireTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{    
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s FT Entry Expire Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    CFTEntryExpire  msgEntryExpire(msg) ;
    UINT32 tt = (UINT32)time( NULL );
    if((tt -ccb.getCreateTime())<=8)
    {
        LOG3( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s FT Entry Expire Action(eid:0x%x), time(%d) <=8, discard msg", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid(),(tt -ccb.getCreateTime()));
        return ccb.GetCurrentState();
    }
#ifndef _NO_NVRAM_RECOVER_
    if ( true == ccb.GetIsAnchor() )
        {
        //更新到NVRam.
        DelFromNVRAM( ccb );
        }
#endif

    if (( IPTYPE_FIXIP == ccb.GetIpType() )/*||(IPTYPE_DHCP == ccb.GetIpType())*/)//wangwenhua modify 20081015
        {
        //Delete ARP Entry
        ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);
        //FixIP 不同步到DM和CPE. 以便下次数据服务时可以查询到
        }
    else
        {
	         if((IPTYPE_DHCP == ccb.GetIpType()))
	         {
	               if(msgEntryExpire.GetFlag()==1)
                	{
                	    ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,1);
                	}
			 else
		        {
	               ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );
			}
	         }
        if ( false == ccb.GetIsAnchor() )
            {
            //Not Anchor BTS
            //往Anchor发Sync(0)
            SendTunnelSync( ccb, true );
            }

        //start synchronization.
        if(msgEntryExpire.GetFlag()==1)
        {
            LOG2( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ),"%s CEntryExpireTrans not sync DM (eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
        }
        else
        {
            Synchronize( M_SYNC_DELETE, false, ccb );
        }

        //delete timer.
        ccb.DeleteTimer();
        ccb.stopHeartBeat();
        }

    //Reclaim CCB.
    return TargetState;
}
