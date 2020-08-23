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
 *   04/10/06   xiao weifang  BTS��֧���л�������Ҫ�Ĵ���
 *   03/30/06   xiao weifang  NVRAM�ָ��û���Ϣ. 
 *   09/12/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

//����Winsock 1
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
//����׮����
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
    Idle״̬��PADI���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
        //�û��ѴﵽUT���õ���������������û�����
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
    Selecting״̬��PADI���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
        //ԭ����DHCP�û�,��Ҫ�ı�ccb��iptype;�Լ�ת�����ttl
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
    Selecting״̬��PADO���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
    Selecting/Request״̬��ʱ����ʱ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
    Selecting/Request״̬ɾ��CCB����Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
    Selecting״̬��PADR���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSelectingPADRTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADR Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    ForwardTrafficToWan( msg );
    //״̬-->Requesting
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingPADITrans::Action

DESCRIPTION:
    Requesting״̬��PADI���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
    Requesting״̬��PADR���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
    Requesting״̬��PADS���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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

        //�޸Ķ�ʱ��
        CTimer *pTimer = ccb.GetTimer();
        if ( NULL != pTimer )
            {
            pTimer->Stop();
            pTimer->SetInterval( M_SNOOP_SYNC_INTERVAL_2Sec );
            pTimer->Start();
            }
        else
            {
            //???�쳣
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
    Syncing״̬��PADI���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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

    //�ͷ�Syncing״̬�������Ϣ
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

    //�޸Ķ�ʱ��
    CTimer *pTimer = ccb.GetTimer();
    if ( NULL != pTimer )
        {
        pTimer->Stop();
        pTimer->SetInterval( M_SNOOP_FTIMER_INTERVAL_8Sec );
        pTimer->Start();
        }
    else
        {
        //???�쳣�����´�����ʱ��
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
    Syncing״̬PPPoE�û���Synchronize OK��Ϣ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CSyncingPPPoESyncOKTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PPPoE SYNCOK Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    CMessage *pMsg = (CMessage*)ccb.GetMsg();
    if ( NULL == pMsg )
        {
        //�쳣
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), "Find no PADS packet to forward when synchronize success." );
        return STATE_SNOOP_IDLE;
        }

    //forward traffic.
    ForwardTrafficToAI( *pMsg, ccb.GetEid() );

    //Delete timer.
    ccb.DeleteTimer();

    //Delete packet.
    ccb.DeleteMsg();

    //CCB��֤�ɹ�
    ccb.SetIsAuthed( true );

    //����PPPoE�û�ת����
    UpdateFTEntry( ccb, IPTYPE_PPPoE );

#ifndef _NO_NVRAM_RECOVER_
    //���µ�NVRam.
    AddToNVRAM( ccb );
#endif

    //״̬-->BOUND
    return TargetState;
}


/*============================================================
MEMBER FUNCTION:
    CSyncingSyncFAILTrans::Action

DESCRIPTION:
    Syncing״̬��Synchronize FAIL��Ϣ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
            //����ARP�ṩ�Ľӿ�ɾ��IP List
            if ( true == ccb.GetIsAuthed() )
                {
                ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );

#ifndef _NO_NVRAM_RECOVER_
                //���µ�NVRam.
                DelFromNVRAM( ccb );
#endif
                }
            }
        else
            {
            //Serving BTS
            //��Anchor��Sync(0)
            SendTunnelSync( ccb, true );
            }
        }

    //PPPoE ���������
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
    Syncing״̬��Delete Entry��Ϣ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
        //UT�����뿪,Anchor BTS�Ѿ���֤��,,��ɾ��
        return ccb.GetCurrentState();
        }

    if ( IPTYPE_DHCP == ccb.GetIpType() )
        {
        //DHCP,SYNCING״̬������Fix IP�����
        if ( true == ccb.GetIsAnchor() )
            {
            //Anchor BTS
            //����ARP�ṩ�Ľӿ�ɾ��IP List
            if ( true == ccb.GetIsAuthed() )
                {
                ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );

#ifndef _NO_NVRAM_RECOVER_
                //���µ�NVRam.
                DelFromNVRAM( ccb );
#endif
                }
            }
        else if ( M_UT_REMOVE == ucDelType )
            {
            //��Anchor��Sync(0)
            SendTunnelSync( ccb, true );
            }
        }

    //PPPoE ���������
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
    Bound״̬��PADR���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
    Bound״̬��PADS���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
    Bound/ROAMING״̬��PADI���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
        //FIXIP�û����������Ϣ
        return ccb.GetCurrentState();
        }

////���ԭ����DHCP�û��ĳ�PPPoE���ţ�PPPoE�û�����ʹ�á�
    if ( false == ccb.GetIsAnchor() )
        {
        //Serving BTS
        //��Anchor��Sync(0)
        SendTunnelSync( ccb, true );
        }
#ifndef _NO_NVRAM_RECOVER_
    else
        {
        //���µ�NVRam.
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

    //�޸Ķ�ʱ��
    CTimer *pTimer = ccb.GetTimer();
    if ( NULL != pTimer )
        {
        pTimer->Stop();
        pTimer->SetInterval( M_SNOOP_FTIMER_INTERVAL_8Sec );
        pTimer->Start();
        }
    else
        {
        //���´�����ʱ��
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
    Bound״̬��PADT���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundPADTTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    if ( IPTYPE_PPPoE != ccb.GetIpType() )
        {
        //FIXIP�û����������Ϣ
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
                    //ת����UT
                    msg.SetDirection( DIR_TO_AI );
                    //Eid?
                    msg.SetEID( ccb.GetEid() );
                    }
                else
                    {
                    //UT�Ѿ������뿪Anchor BTS.
                    msg.SetDirection( DIR_TO_TDR );
                    //ת����Serving BTS.
                    msg.SetBTS( ccb.GetServingBts() );
                    }
            break;

        case DIR_FROM_TDR:
                //PADT From Tunnel.
                LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s PADT(from Tunnel) Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
                if ( true == ccb.GetIsServing() )
                    {
                    //Ӧ����PPPoE Server��Client����PADT
                    //ת����UT
                    msg.SetDirection( DIR_TO_AI );
                    //Eid?
                    msg.SetEID( ccb.GetEid() );
                    }
                else
                    {
                    //Ӧ����PPPoE Client��Server����PADT
                    //ת����WAN
                    msg.SetDirection( DIR_TO_WAN );
                    }
            break;

        default:
            //����������
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), "Err. Ambiguous direction found in PADT snoop request." );
            return STATE_SNOOP_BOUND;
        }

    //֪ͨEB,ת��PADT.
    ForwardTraffic( msg );

    if ( false == bIsAnchor )
        {
        //Not Anchor BTS.
        //��AnchorBTS��SYNC(0)
        SendTunnelSync( ccb, true );
        }
#ifndef _NO_NVRAM_RECOVER_
    else
        {
        //���µ�NVRam.
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
    Bound״̬��ʱ����ʱ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CBoundTimeOutTrans::Action(CSnoopCCB &ccb, CMessage &)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Timer Expire Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );

    DATA_assert( IPTYPE_DHCP == ccb.GetIpType() );
    if ( true == ccb.GetIsAnchor() )
        {
        //����ARP�ṩ�Ľӿ�ɾ��IP List
        ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );

#ifndef _NO_NVRAM_RECOVER_
        //���µ�NVRam.
        DelFromNVRAM( ccb );
#endif
        }
    else
        {
        //��Anchor��Sync(0)
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
    Tunnel Establish Req��Ϣ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CTunnelEstablishReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Tunnel Establish Request Action(eid:0x%x)", 
          (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    UINT32 tt = (UINT32)time( NULL );
	#if 0
    if((tt -ccb.getCreateTime())<=2)//�����ע����Ϣ��2������Ϊ���ӳ���Ϣ������
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
        //�� ʧ����Ӧ
        SendTunnelEstablishResponse( ccb, ulFromBtsID,msgTunnelEstablish.GetSenderBtsIP(),msgTunnelEstablish.GetSenderPort(), false );
        return ccb.GetCurrentState();
        }

    if ( false == ccb.GetIsAnchor() )
        {
        //�������AnchorBTS, ����������ԭ״̬
        //�� ʧ����Ӧ
        SendTunnelEstablishResponse( ccb, ulFromBtsID,msgTunnelEstablish.GetSenderBtsIP(),msgTunnelEstablish.GetSenderPort(), false );
        return ccb.GetCurrentState();
        }

    //�Ƿ�������?
    UINT8 ucIpType = ccb.GetIpType();
    if ( ( true == ccb.IsExistTunnel() )
        && ( IPTYPE_FIXIP != ucIpType ) &&(ulFromBtsID!=ccb.GetServingBts()))/***wangwenhua add condition ,***/
        {
        //AnchorBTS��������BTS���������Ҫ�Ȳ��
        //��������������������ͬһ������վ�Ļ�,��û�б�Ҫ������ֹ.
         LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ),"ulFromBtsID:%x,%x\n",ulFromBtsID,ccb.GetServingBts());
        SendTunnelTerminate( ccb );;
        }

    ////
    ccb.SetIsServing( false );
    //����ServingBTS
    ccb.SetServingBts( ulFromBtsID );
    //�޸�group
    ccb.setGroupId( msgTunnelEstablish.GetGroupId() );

    //Delete Eid Table.
    NotifyDelEidTable( ccb.GetEid() );

    //Update FT Entry.
    UpdateFTEntry( ccb, ucIpType );

    //Response.
    SendTunnelEstablishResponse( ccb, ulFromBtsID, msgTunnelEstablish.GetSenderBtsIP(), msgTunnelEstablish.GetSenderPort(), true );

#ifndef _NO_NVRAM_RECOVER_
    //���µ�NVRam.
    AddToNVRAM( ccb );
#endif

    ccb.startHeartBeat();

    //���ı�״̬
    return ccb.GetCurrentState();
}


/*============================================================
MEMBER FUNCTION:
    CTunnelTerminateReqTrans::Action

DESCRIPTION:
    Tunnel Terminate Req��Ϣ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CTunnelTerminateReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Tunnel Terminate Request Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    UINT32 tt = (UINT32)time( NULL );
	#if 0
    if((tt -ccb.getCreateTime())<=2)//�����ע����Ϣ��2������Ϊ���ӳ���Ϣ������
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
    Tunnel Change Anchor Req��Ϣ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CTunnelChangeAnchorReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    UINT32 tt = (UINT32)time( NULL );
	#if 0
    if((tt -ccb.getCreateTime())<=2)//�����ע����Ϣ��2������Ϊ���ӳ���Ϣ������
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
        //����AnchorBTS,�쳣
        //������Response Success.
        //edit by yhw
        SendTunnelChangeAnchorResponse( ccb, ulFromBtsID, msgTunnelChangeAnchorReq.GetSenderBtsIP(),msgTunnelChangeAnchorReq.GetSenderPort(), true );
        //���ı�״̬
        return ccb.GetCurrentState();
        }
    else
        {
        //Anchor BTS.
        //������ʱ�����10s��ɾ�� FT Entry.
        DelFTEntry( ccb.GetMac(), true,  msgTunnelChangeAnchorReq.GetSenderBtsIP(), msgTunnelChangeAnchorReq.GetSenderPort() );

        //Change Anchor֮ǰ�Ƿ�������BTS�����?
        if ( ( true == ccb.IsExistTunnel() )
            && ( IPTYPE_FIXIP != ucIpType ) )
            {
            //�����,������
            SendTunnelTerminate( ccb );
            }

        if ( IPTYPE_PPPoE != ucIpType )
            {
            //DHCP��Fix IP�û���ɾARP ��IP List
            ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);
            }

#ifndef _NO_NVRAM_RECOVER_
        //���µ�NVRam.
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
    Roaming״̬��Tunnel Establish Response���ĵĴ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
        //��������ɹ�
        //Delete timer.
        ccb.DeleteTimer();
        ccb.stopHeartBeat();
        if ( IPTYPE_DHCP == ccb.GetIpType() )
            {
            ////PPPoE��Fix IP�û�����Ҫ��ʱ��
            UINT32 ulMillSecs = 1000 * ( ccb.GetData().stIpLease.ulLease - time( NULL ) );
            CTimer *pTimer = StartSnoopTimer( ccb.GetMac(), ulMillSecs );
            if ( NULL == pTimer )
                {
                LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Start Snoop Timer failed." );
                return STATE_SNOOP_IDLE;
                }
            ccb.SetTimer( pTimer );
            }

        //��֤ͨ��??
        ccb.SetIsAuthed( true );

        ////
        ccb.SetIsServing( true );
        ccb.SetServingBts( ::bspGetBtsID() );

        //-->BOUND.
        return TargetState;
        }
    else
        {
        //�������ʧ��
        LOG1( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Rx tunnel establish FAIL respose Eid[%08x].",ccb.GetEid());
        //Synchronize.
        if ( IPTYPE_FIXIP != ccb.GetIpType() )
            {
            //FixIP��ɾ��DM��IPLIST.
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
    Roaming״̬��ʱ����ʱ�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
    Roaming״̬Delete Entry�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CRoamingDelEntryTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->%s Del Entry Action(eid:0x%x)", (int)strSTATE[ ccb.GetCurrentState() ], ccb.GetEid() );
    CDelILEntry msgDelILEntry( msg );
    UINT8 ucDelType = msgDelILEntry.GetOp();

    //Roaming״ֻ̬����ServingBTS�ϲ��ܽ���
    //ServingBTS��IPLIST.
    if ( M_UT_REMOVE == ucDelType )
        {
        //UTע�������,,��Anchor��Sync(0)
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
    Mac��Ӧ��ת������ʱ����PPPoE/FIXIP�û����ʱ������

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

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
        //���µ�NVRam.
        DelFromNVRAM( ccb );
        }
#endif

    if (( IPTYPE_FIXIP == ccb.GetIpType() )/*||(IPTYPE_DHCP == ccb.GetIpType())*/)//wangwenhua modify 20081015
        {
        //Delete ARP Entry
        ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac() ,0);
        //FixIP ��ͬ����DM��CPE. �Ա��´����ݷ���ʱ���Բ�ѯ��
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
            //��Anchor��Sync(0)
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
