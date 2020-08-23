/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopTransDHCP.cpp
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
#include <taskLib.h>
#include "Timer.h"
#include "LogArea.h"

#include "L3DataDM.h"
#include "L3DataSnoop.h"
#include "L3DataSnoopTransDHCP.h"
#include "L3DataSnoopState.h"
#include "L3DataSnoopTimer.h"
#include "L3DataTunnelSync.h"
#include "L3DataTunnelEstablish.h"
#include "L3DataTunnelTerminate.h"
#include "L3DataTunnelChangeAnchor.h"
#include "L3DataDhcp.h"
#include "L3DataDelIL.h"
#include "L3DataFixIp.h"
#include "L3DataSnoopErrCode.h"
#include "L3DataArp.h"

#ifdef UNITEST
//包含桩函数
#include "L3DataStub.h"
#endif

//externs.
extern "C" int bspGetBtsID();
extern     UINT32 GetRouterAreaId();
extern     void   ARP_DelILEntry(UINT32 ulIp, UINT8 *pMac,UINT8 flag);
extern     void   ARP_AddILEntry(UINT32, const UINT8 *, UINT32, UINT8);

/*============================================================
MEMBER FUNCTION:
    CIdleDiscoveryTrans::Action

DESCRIPTION:
    Idle状态收Discovery报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CIdleDiscoveryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Discovery Action(eid:0x%x)",
             (int)strSTATE[ ccb.GetCurrentState() ], msg.GetEID() );

    DATA Data;
    memset( (void*)&Data, 0, sizeof( DATA ) );

    UINT8 *pMac  = ccb.GetMac();
    UINT32 ulEid = msg.GetEID();
    if ( false == IsUTAccessEnable( ulEid ) )
        {
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "User is not permit to access in EID:%X, please check EID table", ulEid );
        return STATE_SNOOP_IDLE;
        }

    UINT32 ulRaid = ( CTSnoop::GetInstance() )->GetRouterAreaId();
    if ( M_RAID_INVALID == ulRaid )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        return STATE_SNOOP_IDLE;
        }

    CTimer *pTimer = StartSnoopTimer( pMac, M_SNOOP_FTIMER_INTERVAL_8Sec );
    if ( NULL == pTimer )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create Ftimer failed." );
        return STATE_SNOOP_IDLE;
        }

    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_DHCP );
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
        pMac,               /*const UINT8 * pMac */
        true,               /*bool bIsServing, */
        false,              /*bool bIsTunnel, */
        0,                  /*UINT32 ulPeerBtsAddr, */
        IPTYPE_DHCP,        /*UINT8 ucIpType, */
        false,               /*bool bIsAuthed*/
        ccb
        );

    //Forward Discovery to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}
/*============================================================
MEMBER FUNCTION:
    CIdleRequestTrans::Action

DESCRIPTION:
    Idle状态收Request报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
  notes:wangwenhua add 20081016 to cope the request msg from pc under ccb idle state
==============================================================*/
FSMStateIndex CIdleRequestTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Request Action(eid:0x%x)",
             (int)strSTATE[ ccb.GetCurrentState() ], msg.GetEID() );

    DATA Data;
    memset( (void*)&Data, 0, sizeof( DATA ) );

    UINT8 *pMac  = ccb.GetMac();
    UINT32 ulEid = msg.GetEID();
    if ( false == IsUTAccessEnable( ulEid ) )
        {
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "User is not permit to access in EID:%X, please check EID table", ulEid );
        return STATE_SNOOP_IDLE;
        }

    UINT32 ulRaid = ( CTSnoop::GetInstance() )->GetRouterAreaId();
    if ( M_RAID_INVALID == ulRaid )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        return STATE_SNOOP_IDLE;
        }

    CTimer *pTimer = StartSnoopTimer( pMac, M_SNOOP_FTIMER_INTERVAL_8Sec );
    if ( NULL == pTimer )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create Ftimer failed." );
        return STATE_SNOOP_IDLE;
        }

    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_DHCP );
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
        pMac,               /*const UINT8 * pMac */
        true,               /*bool bIsServing, */
        false,              /*bool bIsTunnel, */
        0,                  /*UINT32 ulPeerBtsAddr, */
        IPTYPE_DHCP,        /*UINT8 ucIpType, */
        false,               /*bool bIsAuthed*/
        ccb
        );

    //Forward Discovery to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}
/*============================================================
MEMBER FUNCTION:
    CIdleTunnelSyncReqTrans::Action

DESCRIPTION:
    Idle状态收Tunnel Sync Request消息的处理
    仅仅做成功响应

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CIdleTunnelSyncReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Tunnel Sync Request Action", 
            (int)strSTATE[ ccb.GetCurrentState() ] );
    CTunnelSync msgTunnelSyncReq( msg );
    DATA Data = msgTunnelSyncReq.GetDATA();
    UINT32 ulFromBts = msgTunnelSyncReq.GetSenderBtsID();
    bool bIsSync0 = false;

    if ( IPTYPE_DHCP == msgTunnelSyncReq.GetIpType() )
        {
        if ( ( 0 == Data.stIpLease.ulIp ) 
            && ( 0 == Data.stIpLease.ulLease ) )
            {
            //Sync(0)
            bIsSync0 = true;
            }
        }
    else
        {
        //PPPoE或Fix IP
        CMac Mac( Data.stSessionMac.aucServerMac );
        if ( ( 0 == Data.stSessionMac.usSessionId ) && ( true == Mac.IsZero() ) )
            {
            //Sync(0)
            bIsSync0 = true;
            }
        }

    if ( true == bIsSync0 )
        {
        //Sync0, Send Success Response.
        SendTunnelSyncResponse( ccb, ulFromBts,msgTunnelSyncReq.GetSenderBtsIP(),msgTunnelSyncReq.GetSenderPort(), true );

        //Reclaim CCB.
        return TargetState;
        }
    else
        {
        //SyncIp.
        //不处理
        //Send Tunnel Sync Response.FAIL
        SendTunnelSyncResponse( ccb, ulFromBts,msgTunnelSyncReq.GetSenderBtsIP(),msgTunnelSyncReq.GetSenderPort(), false );
        //不改变状态
        return ccb.GetCurrentState();
        }
}


/*============================================================
MEMBER FUNCTION:
    CIdleFixIpTunnelEstReqTrans::Action

DESCRIPTION:
    CIdleFixIpTunnelEstReqTrans类: 
    IDLE状态收Fixed IP的创建隧道请求

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CIdleFixIpTunnelEstReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    CTunnelEstablish   msgTunnelEstablishReq( msg );
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ),
           "->%s Fixed IP Tunnel Establish Request Action(eid:0x%x)", 
           (int)strSTATE[ ccb.GetCurrentState() ], msgTunnelEstablishReq.GetEidInPayload() );

    UINT32 ulFromBtsID = msgTunnelEstablishReq.GetSenderBtsID();

    if ( false == IsBTSsupportMobility( GetRouterAreaId(), ccb.GetRAID(), ccb.GetAnchorBts() ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "[FIXIP] BTS mobility disabled" );
        //Response.
        SendTunnelEstablishResponse( ccb, ulFromBtsID,msgTunnelEstablishReq.GetSenderBtsIP(),msgTunnelEstablishReq.GetSenderPort(), false );
        return ccb.GetCurrentState();
        }

    ccb.SetIsAuthed( true );
    ccb.SetEid( msgTunnelEstablishReq.GetEidInPayload() );
    ccb.SetMac( msgTunnelEstablishReq.GetMac() );
    DATA_assert( IPTYPE_FIXIP == msgTunnelEstablishReq.GetIpType() );
    ccb.SetIpType( IPTYPE_FIXIP );
    DATA Data;
    Data.stIpLease.ulIp     = msgTunnelEstablishReq.GetFixIp();
    Data.stIpLease.ulLease  = 0xFFFFFFFF;
    ccb.SetDATA( Data );
    ccb.SetRAID( ( CTSnoop::GetInstance() )->GetRouterAreaId() );
    ccb.SetIsAnchor( true );
    ccb.SetIsServing( false );
    ccb.SetAnchorBts( ::bspGetBtsID() );
    ccb.SetServingBts( ulFromBtsID );
    ccb.setGroupId( msgTunnelEstablishReq.GetGroupId() );   //Tunnel Establish Req should carry the group ID;

    //Delete Eid Table.
    //NotifyDelEidTable( ccb.GetEid() );

    //Update FT Entry.
    UpdateFTEntry( ccb, IPTYPE_FIXIP );

    //Add ARP Entry.
    ARP_AddILEntry( ccb.GetEid(), ccb.GetMac(), Data.stIpLease.ulIp, ccb.getIsRcpeFlag());

    //Response.
    SendTunnelEstablishResponse( ccb, ulFromBtsID,msgTunnelEstablishReq.GetSenderBtsIP(),msgTunnelEstablishReq.GetSenderPort(), true );

#ifndef _NO_NVRAM_RECOVER_
    //更新到NVRam.
    AddToNVRAM( ccb );
#endif

    ccb.startHeartBeat();

    //-->ROAMING.
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CIdleTunnelChgAnchorReqTrans::Action

DESCRIPTION:
    CIdleFixIpTunnelEstReqTrans类: 
    IDLE状态收Tunnel Change Anchor请求

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CIdleTunnelChgAnchorReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ),
        "->%s (Fixed IP)Tunnel Change Anchor Request Action",
         (int)strSTATE[ ccb.GetCurrentState() ] );

    //做成功响应
    //edit by yhw
    CTunnelChangeAnchor mTunnleMsg(msg);
    SendTunnelChangeAnchorResponse( ccb, mTunnleMsg.GetSenderBtsID(), mTunnleMsg.GetSenderBtsIP(), mTunnleMsg.GetSenderPort(), true );
    //-->IDLE
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CIdleTunnelTerminateReqTrans::Action

DESCRIPTION:
    CIdleFixIpTunnelTerminateReqTrans类: 
    IDLE状态收Tunnel Terminate请求

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CIdleTunnelTerminateReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ),
          "->%s Tunnel Terminate Request Action", 
           (int)strSTATE[ ccb.GetCurrentState() ] );
    CTunnelTerminate msgTnlTerminate(msg);
    //做成功响应
    SendTunnelTerminateResponse( ccb, msgTnlTerminate.GetSenderBtsID(),msgTnlTerminate.GetSenderBtsIP(),msgTnlTerminate.GetSenderPort(), true );
    //-->IDLE
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CIdleAddFixIPTrans::Action

DESCRIPTION:
    CIdleAddFixIPTrans类: 
    IDLE状态收ADD Fixed IP的消息

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CIdleAddFixIPTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    CADDFixIP msgAddFixIp( msg );
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
    "->%s ADD Fix IP Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], msgAddFixIp.GetEidInPayload() );
    
    UTILEntry ILEntry = msgAddFixIp.GetEntry();
    UINT32 ulEid = msgAddFixIp.GetEidInPayload();
    UINT8  *pMac = ILEntry.aucMAC;
    UINT32 ulRouterAreaId = ILEntry.ulRouterAreaId;
    UINT32 ulAnchorBts = ILEntry.ulAnchorBts;
    UINT32 ulLocalBts = ::bspGetBtsID();
    
    UINT32 ulLocalRAID = CTSnoop::GetInstance()->GetRouterAreaId();
    if ( M_RAID_INVALID == ulLocalRAID )
    {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        //Reclaim CCB.
        return STATE_SNOOP_IDLE;
    }
    
    bool bSuccess = true;
    if ( ulLocalRAID != ulRouterAreaId )
    {
        if (ulAnchorBts != ulLocalBts)
        {
            //其他Raid的BTS上配置FixIp
            bSuccess &= OutRaid( ulEid, ILEntry, ccb);
            
            //新增IpList,并启动同步
            bSuccess &= AddFixIpEntry( ccb,
            ulEid, 
            false,                      /*bIsAnchor*/
            ulAnchorBts,                /*Anchor of CCB*/
            ulLocalRAID,                /*本地RAID*/
            ILEntry );                  /*UTILEntry **/
            
            if ( true == bSuccess )
            {
                //进入ROAMING状态，等待Tunnel Establish Response.
                return STATE_SNOOP_ROAMING;
            }
        }
        else
        {
            LOG2( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "BTS[%d] doesn't match its RAID[%d], UT data need update", ulAnchorBts, ulRouterAreaId );
            bSuccess &= AddFTEntry( ulEid,
            pMac,
            true,       /*bool bIsServing*/
            false,      /*bool bIsTunnel*/ 
            0,          /*UINT32 ulPeerBtsAddr*/
            IPTYPE_FIXIP, /*UINT8 ucIpType*/
            true,       /*bool bIsAuthed*/
            ccb);     
            
            //新增IpList,并启动同步
            bSuccess &= AddFixIpEntry( ccb,
            ulEid, 
            true,               /*bIsAnchor*/
            ulLocalBts,         /*ulAnchorBts*/
            ulLocalRAID,        /*本地RAID*/
            ILEntry );          /*UTILEntry **/
            
            if ( true == bSuccess )
            {
                //Fix IP用户,,Add ARP IpList Entry.
                ::ARP_AddILEntry( ulEid,  pMac, ILEntry.Data.stIpLease.ulIp, ccb.getIsRcpeFlag());
                
                #ifndef _NO_NVRAM_RECOVER_
                //更新到NVRam.
                AddToNVRAM( ccb );
                #endif
                //-->Bound
                return TargetState;
            }
        }
    }
    
    ////if ( ( ulLocalRAID == ulRouterAreaId ) && ( ulLocalBts != ulAnchorBts ) )
    else
    {
    #if 0
        //同一Raid，但不是Anchor BTS配置FixIp
        if(ulAnchorBts != ulLocalBts)//to send changing anchor msg to the bts   wangwenhua add 20081023
        {        
            SendTunnelChangeAnchor( ulEid, 
            pMac,
            ulAnchorBts,
            IPTYPE_FIXIP ) ;
            ccb.SetIsRealAnchor(false);//wangwenhua add 20081023        
            	 // if (IPTYPE_PPPoE != ccb.GetIpType())//wangwenhua add 20110718
	        {
	        	CTaskARP::GetInstance()->sendG_ARP(pMac, ccb.GetData().stIpLease.ulIp);
			CTaskARP::GetInstance()->sendG_ARP(pMac, ccb.GetData().stIpLease.ulIp);
	        }
        }
	#endif
        bSuccess &= AddFTEntry( ulEid,
         pMac,
         true,       /*bool bIsServing*/
         false,      /*bool bIsTunnel*/ 
         0,          /*UINT32 ulPeerBtsAddr*/
         IPTYPE_FIXIP, /*UINT8 ucIpType*/
         true,/*bool bIsAuthed*/
         ccb);     
        
        //新增IpList,并启动同步
        bSuccess &= AddFixIpEntry( ccb,
        ulEid, 
        true,               /*bIsAnchor*/
        ulLocalBts,         /*ulAnchorBts*/
        ulLocalRAID,        /*本地RAID*/
        ILEntry );          /*UTILEntry **/

		 //同一Raid，但不是Anchor BTS配置FixIp
        if(ulAnchorBts != ulLocalBts)//to send changing anchor msg to the bts   wangwenhua add 20081023
        {        
            SendTunnelChangeAnchor( ulEid, 
            pMac,
            ulAnchorBts,
            IPTYPE_FIXIP ) ;
            ccb.SetIsRealAnchor(false);//wangwenhua add 20081023        
      }
            	 // if (IPTYPE_PPPoE != ccb.GetIpType())//wangwenhua add 20110718
	        {
	        	CTaskARP::GetInstance()->sendG_ARP(pMac, ccb.GetData().stIpLease.ulIp);
			CTaskARP::GetInstance()->sendG_ARP(pMac, ccb.GetData().stIpLease.ulIp);
	        }
        if ( true == bSuccess )
        {
            //Fix IP用户,,Add ARP IpList Entry.
            ::ARP_AddILEntry( ulEid,  pMac, ILEntry.Data.stIpLease.ulIp, ccb.getIsRcpeFlag());
            
            #ifndef _NO_NVRAM_RECOVER_
            //更新到NVRam.
            AddToNVRAM( ccb );
            #endif
            //-->Bound
            return TargetState;
        }
    }
    
    //FixIP 不同步到DM和CPE. 以便下次数据服务时可以查询到
    //Synchronize( M_SYNC_DELETE, false, ulEid, ILEntry );
    
    //delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();
    
    //delete FT entry
    DelFTEntry( ccb.GetMac() );
    
    //Reclaim CCB.
    return STATE_SNOOP_IDLE;
    
}


/*============================================================
MEMBER FUNCTION:
    CIdleAddFixIPTrans::OutRaid

DESCRIPTION:
    配置的固定IP在其他RouterArea Id的BTS上

ARGUMENTS:
    *pMac: Mac地址

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CIdleAddFixIPTrans::OutRaid(UINT32 ulEid, const UTILEntry &ILEntry, CSnoopCCB &ccb)
{
    UINT32 ulFixIp = ILEntry.Data.stIpLease.ulIp;
    //Tunnel Establish
    if ( true != SendTunnelEstablish( ulEid, 
                    ILEntry.aucMAC,
                    ILEntry.ulAnchorBts,
                    IPTYPE_FIXIP,
                    ulFixIp ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Establish Request failed when Add Fix IP." );
        return false;
        }

    //通知EB，增加FT Entry
    return AddFTEntry( ulEid,
        ILEntry.aucMAC,
        true,       /*bool bIsServing*/
        true,       /*bool bIsTunnel*/ 
        ILEntry.ulAnchorBts,    /*UINT32 ulPeerBtsAddr*/
        IPTYPE_FIXIP, /*UINT8 ucIpType*/
        true,       /*bool bIsAuthed*/
        ccb);     
}


/*============================================================
MEMBER FUNCTION:
    CIdleAddFixIPTrans::AddFixIpEntry

DESCRIPTION:
    增加固定IP的CCB

ARGUMENTS:
    Mac: Mac地址

RETURN VALUE:
    bool: true,建立成功；false,异常失败

SIDE EFFECTS:
    none
==============================================================*/
bool CIdleAddFixIPTrans::AddFixIpEntry(CSnoopCCB &ccb, UINT32 ulEid, bool bIsAnchor, UINT32 ulAnchorBts, UINT32 ulLocalRAID, const UTILEntry &ILEntry)
{
    const UINT8  *pMac = ILEntry.aucMAC;
    UINT32 ulRouterAreaId = ILEntry.ulRouterAreaId;
    UINT32 ulLocalBts = ::bspGetBtsID();
    if(( ulLocalRAID != ulRouterAreaId )&&(ulLocalBts!=ulAnchorBts))    
    {
        //Start timer.
        CTimer *pTimer = StartSnoopTimer( pMac, M_SNOOP_FTIMER_INTERVAL_8Sec*60 );
        if ( NULL == pTimer )
        {
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create timer failed when add FixIP." );
            return false;
        }
        ccb.SetTimer( pTimer );
    }

    //Add CCB
    ccb.SetIsAuthed( true );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_FIXIP );
    ccb.SetDATA( ILEntry.Data );
    ccb.SetRAID( ulRouterAreaId );
    ccb.SetIsAnchor( bIsAnchor );
    ccb.SetIsServing( true );
    ccb.SetAnchorBts( ulAnchorBts );
    ccb.SetServingBts( ::bspGetBtsID() );
    ccb.setGroupId( getGroupIDbyEid(ulEid) );

    return true;
}

/*============================================================
MEMBER FUNCTION:
    CSelectingDiscoveryTrans::Action

DESCRIPTION:
    Selecting状态收Discovery报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelectingDiscoveryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s Discovery Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    ForwardTrafficToWan( msg );
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSelectingOfferTrans::Action

DESCRIPTION:
    Selecting状态收Offer报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelectingOfferTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
           "->%s OFFER Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    ForwardTrafficToAI( msg, ccb.GetEid() );
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSelectingRequestTrans::Action

DESCRIPTION:
    Selecting状态收Request报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelectingRequestTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
           "->%s REQUEST Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    ForwardTrafficToWan( msg );
    //状态-->Requesting
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingDiscoveryTrans::Action

DESCRIPTION:
    Requesting状态收Discovery报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRequestingDiscoveryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
        "->%s DISCOVERY Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    DATA Data;
    memset( (void*)&Data, 0, sizeof( DATA ) );
    UINT32 ulEid = msg.GetEID();
    UINT8 *pMac  = ccb.GetMac();

    UINT32 ulRaid = ( CTSnoop::GetInstance() )->GetRouterAreaId();
    if ( M_RAID_INVALID == ulRaid )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        return STATE_SNOOP_IDLE;
        }

    CTimer *pTimer = ccb.GetTimer();
    if ( NULL != pTimer )
        {
        pTimer->Stop();
        pTimer->Start();
        }

    //Set CCB.
    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_DHCP );
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
        IPTYPE_DHCP,        /*UINT8 ucIpType, */
        false,              /*bool bIsAuthed*/
        ccb);            

    //Forward Discovery to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingRequestTrans::Action

DESCRIPTION:
    Requesting状态收Request报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRequestingRequestTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ),
       "->%s REQUEST Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    ForwardTrafficToWan( msg );
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingNAKTrans::Action

DESCRIPTION:
    Requesting状态收NAK报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRequestingNAKTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s NAK Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    ForwardTrafficToAI( msg, ccb.GetEid() );
    DelFTEntry( ccb.GetMac() );
    //Reclaim CCB.
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingACKTrans::Action

DESCRIPTION:
    Requesting状态收ACK报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRequestingACKTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s ACK Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    DhcpHdr *pDhcp = (DhcpHdr*)msg.GetDhcpPtr();
    DhcpOption OutOption;
    //计算DHCP Option的长度;
    //定位到Udp头
    UdpHdr *pUdp = (UdpHdr*)msg.GetUdpPtr();
    //选项长度 = 
    SINT16 OptLen = pUdp->usLen - sizeof( UdpHdr ) - sizeof( DhcpHdr );

    if ( false == CSnoopStateBase::ParseDhcpOpt( (UINT8*)pDhcp + sizeof( DhcpHdr ), OptLen, &OutOption ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PACKET_ERR ), "DHCP Option parsed err under REQUESTING state." );
        return STATE_SNOOP_REQUESTING;
        }

    UINT32 ulIp    = ntohl( pDhcp->ulYiaddr ); //网络序
    UINT32 ulLease = OutOption.ulIpLeaseTime;
    DATA Data;
    Data.stIpLease.ulIp    = ulIp;
    //需要同步到UT，所以Lease必须为到期的时刻。
    Data.stIpLease.ulLease = ulLease + time( NULL );    //moment.
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
            return STATE_SNOOP_IDLE;
            }
        //Set timer.
        ccb.SetTimer( pTimer );
        }

    //start synchronization.
    Synchronize( M_SYNC_ADD, true, ccb );

    //make copy.
    CMessage *pMsg = msg.Clone();
    //Save ACK Packet.
    ccb.SaveMsg( pMsg );

    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSyncingDiscoveryTrans::Action

DESCRIPTION:
    Syncing状态收Discovery报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSyncingDiscoveryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
         "->%s DISCOVERY Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    if ( true == ccb.GetIsAnchor() )
        {
        //BTS是CCB的Anchor BTS.
        if ( true == ccb.GetIsAuthed() )
            {
            //CCB已经认证通过，可能是Renew的情况，删除ARP Ip List
            DATA_assert( IPTYPE_DHCP == msg.GetIpType() );
            ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );

#ifndef _NO_NVRAM_RECOVER_
            //更新到NVRam.
            DelFromNVRAM( ccb );
#endif
            }
        }
    else
        {
        //BTS不是CCB的Anchor BTS.
        //通知Anchor BTS: SYNC(0).
        SendTunnelSync( ccb, true );
        }

    //Delete. Synchronize.
    Synchronize( M_SYNC_DELETE, false, ccb );

    //释放Syncing状态保存的消息
    ccb.DeleteMsg();

    DATA Data;
    memset( (void*)&Data, 0, sizeof( DATA ) );
    UINT32 ulEid = msg.GetEID();
    UINT8  *pMac = ccb.GetMac();

    //Router Area Id
    UINT32 ulRaid = ( CTSnoop::GetInstance() )->GetRouterAreaId();
    if ( M_RAID_INVALID == ulRaid )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
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
            return STATE_SNOOP_IDLE;
            }
        ccb.SetTimer( pTimer );
        }

    //Set CCB.
    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_DHCP );
    ccb.SetDATA( Data );
    ccb.SetRAID( ulRaid );
    ccb.SetIsAnchor( true );
    ccb.SetIsServing( true );
    UINT32 ulLocalAddr = :: bspGetBtsID();
    ccb.SetAnchorBts( ulLocalAddr );
    ccb.SetServingBts( ulLocalAddr );
    ccb.setGroupId( getGroupIDbyEid(ulEid) );

    //Notify EB, Add FT Entry.
    AddFTEntry( ulEid,      /*UINT32 ulEid, */
        pMac,               /*const UINT8 * pMac, */
        true,               /*bool bIsServing, */
        false,              /*bool bIsTunnel, */
        0,                  /*UINT32 ulPeerBtsAddr, */
        IPTYPE_DHCP,        /*UINT8 ucIpType, */
        false,              /*bool bIsAuthed*/
        ccb);            

    //Forward Discovery to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSyncingDHCPSyncOKTrans::Action

DESCRIPTION:
    Syncing状态DHCP用户收Synchronize OK消息的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSyncingDHCPSyncOKTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s DHCP SYNCOK Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    CMessage *pMsg = (CMessage*)ccb.GetMsg();
    if ( NULL == pMsg )
        {
        //异常
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), "Find no DHCPACK packet to forward when synchronize success." );
        return STATE_SNOOP_IDLE;
        }

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    //Delete timer.
    UINT32 ulMillSecs = 1000 * ( ccb.GetData().stIpLease.ulLease - time( NULL ) );
    CTimer *pTimer = StartSnoopTimer( ccb.GetMac(), ulMillSecs );
    if ( NULL == pTimer )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start lease timer failed." );
        return STATE_SNOOP_IDLE;
        }
    ccb.DeleteTimer();
    ccb.SetTimer( pTimer );

    if ( false == ccb.GetIsAnchor() )
        {
        //Not Anchor
        //往AnchorBTS发Sync(IP,Lease)
        SendTunnelSync( ccb, false );
        }
    else
        {
#ifndef _NO_NVRAM_RECOVER_
        //更新到NVRam.
        AddToNVRAM( ccb );
#endif
        }

    //forward traffic.
    ForwardTrafficToAI( *pMsg, ccb.GetEid() );

    //Delete packet.
    ccb.DeleteMsg();

    //增加ARP的Ip List.
    //BOUND状态收init request时，auth = ture,但是仍然需要修改arp表
    ::ARP_AddILEntry( ccb.GetEid(), ccb.GetMac(), ccb.GetData().stIpLease.ulIp, ccb.getIsRcpeFlag());

    if ( false == ccb.GetIsAuthed() )
        {
        //更新转发表
        UpdateFTEntry( ccb, IPTYPE_DHCP );

        //CCB认证成功
        ccb.SetIsAuthed( true );
        }

    //状态-->BOUND
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CBoundRelDecTrans::Action

DESCRIPTION:
    Bound状态收Release或Decline报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundRelDecTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ),
           "->%s RELEASE/DECLINE Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        //FIXIP用户不处理该消息
        return ccb.GetCurrentState();
        }

    if ( true == ccb.GetIsAnchor() )
        {
        //BTS是CCB的Anchor BTS.
        ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);

        //转发报文给WAN.
        ForwardTrafficToWan( msg );

#ifndef _NO_NVRAM_RECOVER_
        //更新NVRam.
        DelFromNVRAM( ccb );
#endif
        }
    else
        {
        //BTS不是CCB的Anchor BTS.
        //往AnchorBTS方向转发报文
        ForwardTrafficToTDR( msg, ccb.GetAnchorBts() );

#ifdef __WIN32_SIM__
//Win32:
        ::Sleep( 10 );//10毫秒
#else
//VxWorks:
        ::taskDelay( 1 );
#endif

        //通知Anchor BTS: SYNC(0).
        SendTunnelSync( ccb, true );
        }

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    //Delete. Synchronize.
    Synchronize( M_SYNC_DELETE, false, ccb );

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    //Reclaim CCB.
    return TargetState;
}

/*
*BOUND收到init-reboot DHCPREQUEST的响应DHCPACK.
*client会更新地址有效时间lease time.
*所以BTS必须同步更新，否则BTS上的DHCP用户可能会先于PC释放地址
*/
#if 0
/*============================================================
MEMBER FUNCTION:
    CBoundACKTrans::Action

DESCRIPTION:
    Bound状态收ACK报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundACKTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
           "->%s ACK Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        DATA_assert( 0 );
        //FIXIP用户不处理该消息
        return ccb.GetCurrentState();
        }

    //转发给AI
    ForwardTrafficToAI( msg, ccb.GetEid() );
    return TargetState;
}
#endif

/*============================================================
MEMBER FUNCTION:
    CBoundRequestInitTrans::Action

DESCRIPTION:
    Bound状态收Initial Request报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundRequestInitTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    UINT32 eid = msg.GetEID();
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
        "->%s INITIAL REQUEST Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], eid );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        DATA_assert( 0 );
        //FIXIP用户不处理该消息
        return ccb.GetCurrentState();
        }

    DhcpHdr *pDhcp = (DhcpHdr*)msg.GetDhcpPtr();
    if ( 0 == pDhcp->ulCiaddr )
        {
        //Request after Selecting or init-reboot
        //仅转发，不改变状态
        if(eid != ccb.GetEid())
            {
            //BTS是CCB的Anchor BTS.
            ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);

#ifndef _NO_NVRAM_RECOVER_
            //更新到NVRam.
            DelFromNVRAM( ccb );
#endif
            //从旧的cpe删除. Synchronize.
            Synchronize( M_SYNC_DELETE, false, ccb );

            ccb.SetEid(eid);
            //Notify EB, Update FT Entry.
            UpdateFTEntry( ccb, IPTYPE_DHCP );

            //增加到新的cpe，并同步
            Synchronize( M_SYNC_ADD, true, ccb );
            }

        ForwardTrafficToWan( msg );
        }

    //其他消息，不处理，也不改变状态
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CBoundRequestRenewTrans::Action

DESCRIPTION:
    Bound状态收Renew Request报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundRequestRenewTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
         "->%s RENEW REQUEST Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], msg.GetEID() );

    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        LOG3( LOG_DEBUG1, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
         "->%s RENEW REQUEST Action(eid:0x%x), IP type(%d) is not DHCP", (int)strSTATE[ ccb.GetCurrentState() ], msg.GetEID(),  ccb.GetIpType());

        //DATA_assert( 0 );//这个没有必要，去掉
        //FIXIP用户不处理该消息
        return ccb.GetCurrentState();
        }

    DhcpHdr *pDhcp = (DhcpHdr*)msg.GetDhcpPtr();
    if ( 0 == pDhcp->ulCiaddr )
        {
        //confirming correctness of previously allocated address after, e.g., system reboot
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Confirming correctness of previously allocated address after system reboot, forward DHCPREQUEST to WAN.(not TDR)" );
        //转发给WAN。
        ForwardTrafficToWan( msg );
        return STATE_SNOOP_BOUND;
        }

    //Request after BOUND, Renewing or Rebinding.
    if ( true == ccb.GetIsAnchor() )
        {
        //在AnchorBTS上发生的Renew行为
        //转发给WAN。
        ForwardTrafficToWan( msg );
        }
    else
        {
        //在ServingBTS(非Anchor)上发生的Renew行为
        bool bRenewEnable = IsUTRenewEnable( ccb.GetEid() );
        if ( false == bRenewEnable )
            {
            //Renew is forbidden.
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "DHCP renew is forbidden." );
            return STATE_SNOOP_BOUND;
            }

        //转发Request给AnchorBTS.
        ForwardTrafficToTDR( msg, ccb.GetAnchorBts() );
        }

    //状态->Renewing
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRenewingRequestTrans::Action

DESCRIPTION:
    Renewing状态收Rewew Request报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRenewingRequestTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s REQUEST Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    if ( true == ccb.GetIsAnchor() )
        {
        //Anchor BTS
        //转给WAN.
        ForwardTrafficToWan( msg );
        }
    else
        {
        //Not Anchor BTS
        //转发给Anchor BTS.
        ForwardTrafficToTDR( msg, ccb.GetAnchorBts() );
        }

    //状态不变
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRenBndDelEntryTrans::Action

DESCRIPTION:
    BOUND/Renewing状态收Delete Entry消息的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRenBndDelEntryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s Del Entry Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    CDelILEntry msgDelILEntry( msg );
    UINT8 ucDelType = msgDelILEntry.GetOp();
    UINT8 ucIpType  = ccb.GetIpType();
    if(M_UT_MOVE_AWAY == ucDelType)
    {
       //send tunnel timer delete msg to tunnel wangwenhua add 20080606
    
       CTUNNELDelete msgFTDelTunnel;
       if ( TRUE == msgFTDelTunnel.CreateMessage( *( CTSnoop::GetInstance() ) ) )
       {
       
            msgFTDelTunnel.SetDstTid( M_TID_TUNNEL );

            msgFTDelTunnel.SetMac( ccb.GetMac());

            if( false == msgFTDelTunnel.Post() )
             {
                LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Delete TUNNEL Entry Message failed." );
                msgFTDelTunnel.DeleteMessage();
                 
             }
            LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s WWH_CTUNNELDelete(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
        }
   
    }
    if ( true == ccb.GetIsAnchor() )
        {
         LOG4( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s hehe ,(eid:0x%x),%d,%d", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() ,ucDelType,ccb.GetIsRealAnchor());
        if ( M_UT_MOVE_AWAY == ucDelType )
            {
            //UT是漫游离开,,以本BTS为Anchor的IPLIST不删除.
            //返回源状态
            if(true == ccb.GetIsRealAnchor())//wangwenhua add 20081022
            	{
                    return ccb.GetCurrentState();
            	}
            }
        if ( IPTYPE_PPPoE != ucIpType )
            {
            ////Anchor上删除ARP的IpList
            ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);
            }

#ifndef _NO_NVRAM_RECOVER_
        //更新到NVRam.
        DelFromNVRAM( ccb );
#endif
        }
    else
        {
        //ServingBTS的IPLIST.
        if ( M_UT_REMOVE == ucDelType )
            {
            //UT注销的情况
            SendTunnelSync( ccb, true );
            }
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
    CRenewingNAKTrans::Action

DESCRIPTION:
    Renewing状态收NAK报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRenewingNAKTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s NAK Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    //转发给UT
    ForwardTrafficToAI( msg, ccb.GetEid() );
#if 0
    if ( false == ccb.GetIsAnchor() )
        {
        //Not Anchor BTS.
        //往AnchorBTS发SYNC(0)
        SendTunnelSync( ccb, true );
        }
    else
        {
        //Anchor BTS.
        //Delete ARP Ip List
        ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() );

#ifndef _NO_NVRAM_RECOVER_
        //更新到NVRam.
        DelFromNVRAM( ccb );
#endif
        }

    //Delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();

    //Delete FT Entry.
    DelFTEntry( ccb.GetMac() );

    //Synchronize.
    Synchronize( M_SYNC_DELETE, false, ccb );
#endif
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRenewingACKTrans::Action

DESCRIPTION:
    Renewing状态收ACK报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRenewingACKTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), 
          "->%s ACK Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    DhcpHdr *pDhcp = (DhcpHdr*)msg.GetDhcpPtr();
    DhcpOption OutOption;
    //计算DHCP Option的长度;
    //定位到Udp头
    UdpHdr *pUdp = (UdpHdr*)msg.GetUdpPtr();
    //选项长度 = 
    SINT16 OptLen = pUdp->usLen - sizeof( UdpHdr ) - sizeof( DhcpHdr );

    if ( false == CSnoopStateBase::ParseDhcpOpt( (UINT8*)pDhcp + sizeof( DhcpHdr ), OptLen, &OutOption ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PACKET_ERR), "DHCP Option parsed err under RENEWING state." );
        return STATE_SNOOP_RENEWING;
        }

    UINT32 ulIp = ntohl( pDhcp->ulYiaddr ); //网络序
    UINT32 ulLease = OutOption.ulIpLeaseTime;
    DATA Data;
    Data.stIpLease.ulIp = ulIp;
    //需要同步到UT，所以Lease必须为到期的时刻。
    Data.stIpLease.ulLease = ulLease + time( NULL );    //moment.
    ccb.SetDATA( Data );

    //start synchronization. Update Lease timer Only.
#if 1
    Synchronize( M_SYNC_UPDATE, false, ccb );

    //转发给UT
    ForwardTrafficToAI( msg, ccb.GetEid() );

    CTimer *pTimer = StartSnoopTimer( ccb.GetMac(), 1000 * ulLease );
    if ( NULL == pTimer )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start lease timer failed." );
        return STATE_SNOOP_IDLE;
        }

    //Delete timer.
    ccb.DeleteTimer();
    ccb.SetTimer( pTimer );

    if ( false == ccb.GetIsAnchor() )
        {
        //Not Anchor
        //往AnchorBTS发Sync(IP,Lease)
        SendTunnelSync( ccb, false );
        }
    else
        {
#ifndef _NO_NVRAM_RECOVER_
        //更新到NVRam.
        AddToNVRAM( ccb );
#endif
        }
#else
    Synchronize( M_SYNC_UPDATE, true, ccb );

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
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start sync timer failed." );
            return STATE_SNOOP_IDLE;
            }
        //Set timer.
        ccb.SetTimer( pTimer );
        }

    //make copy.
    CMessage *pMsg = msg.Clone();
    //Save ACK Packet.
    ccb.SaveMsg( pMsg );
#endif
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CBRRDiscoveryTrans::Action

DESCRIPTION:
    BOUND/RENEWING/ROAMING状态Discovery报文的处理

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回消息处理后的目标状态.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBRRDiscoveryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ),
           "->%s DISCOVERY Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    if ( IPTYPE_DHCP != ccb.GetIpType() )
        {
        //FIXIP/PPPoE用户不处理该消息
        LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "IP type error, discarded");
        return ccb.GetCurrentState();
        }

    if ( true == ccb.GetIsAnchor() )
        {
        //BTS是CCB的Anchor BTS.
        ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);

#ifndef _NO_NVRAM_RECOVER_
        //更新到NVRam.
        DelFromNVRAM( ccb );
#endif
        }
    else
        {
        //BTS不是CCB的Anchor BTS.
        //通知Anchor BTS: SYNC(0).
        SendTunnelSync( ccb, true );
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
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router Area Id is not configured" );
        return ccb.GetCurrentState();
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
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start snoop timer failed." );
            return STATE_SNOOP_IDLE;
            }
        ccb.SetTimer( pTimer );
        }

    //Set CCB.
    ccb.SetIsAuthed( false );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( IPTYPE_DHCP );
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
        IPTYPE_DHCP,        /*UINT8 ucIpType, */
        false,              /*bool bIsAuthed*/
        ccb);            

    //Forward Discovery to WAN.
    ForwardTrafficToWan( msg );

    return TargetState;
}
