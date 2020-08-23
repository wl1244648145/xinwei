/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopTrans.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   03/30/06   xiao weifang  NVRAM�ָ��û���Ϣ. 
 *   09/09/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

//����Winsock 1
#define _WINSOCKAPI_
#include <time.h>

#include "Timer.h"
#include "LogArea.h"

#include "L3DataFTAddEntry.h"
#include "L3DataFTDelEntry.h"
#include "L3DataSnoop.h"
#include "L3DataSnoopTrans.h"
#include "L3DataSyncIL.h"
#include "L3DataDelEidTable.h"
#include "L3DataSnoopTimer.h"
#include "L3DataTunnelEstablish.h"
#include "L3DataTunnelTerminate.h"
#include "L3DataTunnelChangeAnchor.h"
#include "L3DataTunnelSync.h"
#include "L3DataRoam.h"
//#include "L3DataDM.h"
#include "L3DataSnoopErrCode.h"
#include "L3DataDhcp.h"
#include "L3DataDm.h"
#include "L3DataArp.h"
#include "L3OamCfgCommon.h"
#ifdef UNITEST
//����׮����
#include "L3DataStub.h"
#endif

//externs.
extern "C" int    bspGetBtsPubIp();
extern "C" int    bspGetBtsID();
extern "C" int    bspGetBtsPubPort();

extern "C" int    bspSetNvRamWrite(char *, UINT32, UINT8);
extern void ARP_DelILEntry(UINT32 ulIp, UINT8 *pMac,UINT8 flag);
extern void ARP_AddILEntry(UINT32, const UINT8 *, UINT32, UINT8);
extern UINT32  l3oamGetUIDByEID(UINT32 EID);
#ifdef WBBU_CODE
extern void Reset_CPE(UINT32 EID);
#endif
extern "C" bool notifyOneCpeToRegister(UINT32 EID,bool blStopDataSrv);
extern T_NvRamData *NvRamDataAddr;
extern UINT32  RelayWanifCpeEid[20];
//��̬���ݵĳ�ʼ��
UINT32 CSnoopTrans::m_aulOutMeasure[ OUT_TYPE_MAX ] = {0};


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::PrintMeaure

DESCRIPTION:
    ��ӡ����ͳ������

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopTrans::PrintMeaure()
{
    printf( "\r\n*********************" );
    for ( UINT8 type = OUT_TRAFFIC_EB; type < OUT_TYPE_MAX; ++type )
        {
        printf( "\r\n%-20s: %d", strOutType[ type ], m_aulOutMeasure[ type ] );
        }
    printf( "\r\n***************************************" );
    printf( "\r\n" );
    return ;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::AddFTEntry

DESCRIPTION:
    ֪ͨEB,����FT����

ARGUMENTS:
    *pMac:  Mac��ַ

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::AddFTEntry(UINT32 ulEid, const UINT8 *pMac, bool bIsServing, bool bIsTunnel, UINT32 ulPeerBtsID, UINT8 ucIpType, bool bIsAuthed, CSnoopCCB &ccb)
{
    if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
    {        
        for(int i=0; i<NvRamDataAddr->Relay_num; i++)
        {
            if((RelayWanifCpeEid[i]!=0)&& (ccb.GetEid()==RelayWanifCpeEid[i]))
            {
               ccb.setRcpeFlag(1);
            }
            else//����Ѿ����õ�
            {
               ccb.setRcpeFlag(0);
            }
                
        }        
    }
    //Add FT Entry.
    if ( NULL == pMac )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Err parameter: pMac = NULL when sending Add FT Entry message" );
        return false;
        }

    CFTAddEntry msgFTAddEntry;
    if ( false == msgFTAddEntry.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create add F.T. message failed." );
        return false;
        }
    msgFTAddEntry.SetEidInPayload( ulEid );
    msgFTAddEntry.SetMac( pMac );
    msgFTAddEntry.SetServing( bIsServing );
    msgFTAddEntry.SetTunnel( bIsTunnel );
    msgFTAddEntry.SetPeerBtsID( ulPeerBtsID );
    msgFTAddEntry.SetIpType( ucIpType );
    msgFTAddEntry.SetAuth( bIsAuthed );
    msgFTAddEntry.setGroupId( getGroupIDbyEid(ulEid) );

    msgFTAddEntry.SetDstTid( M_TID_EB );

    if ( false == msgFTAddEntry.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Add FT Entry Message failed." );
        msgFTAddEntry.DeleteMessage();
        return false;
        }
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::DelFTEntry

DESCRIPTION:
    ֪ͨEB,ɾ��FT����

ARGUMENTS:
    *pMac:  Mac��ַ

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::DelFTEntry(const UINT8 *pMac, bool bCreateTempTunnel, UINT32 ulPeerBtsIP, UINT16 usPeerBtsPort)
{
    //Delete FT Entry.
    if ( NULL == pMac )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Err parameter: pMac = NULL when sending Delete FT Entry message." );
        return false;
        }

    CFTDelEntry msgFTDelEntry;
    if ( false == msgFTDelEntry.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create delete F.T. message failed." );
        return false;
        }
    msgFTDelEntry.SetDstTid( M_TID_EB );

    msgFTDelEntry.SetMac( pMac );
    msgFTDelEntry.SetTunnel(bCreateTempTunnel);
    msgFTDelEntry.setTunnelPeerBtsIP(ulPeerBtsIP);
    msgFTDelEntry.setTunnelPeerBtsPort(usPeerBtsPort);
    if ( false == msgFTDelEntry.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Delete FT Entry Message failed." );
        msgFTDelEntry.DeleteMessage();
        return false;
        }
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::UpdateFTEntry

DESCRIPTION:
    ֪ͨEB,����FT����

ARGUMENTS:
    *pMac:  Mac��ַ
    bRefreshTTL: EB���ݴ˱�־ȷ���Ƿ�ֻˢ��TTL.(��ҪӦ��������Ϣ)

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::UpdateFTEntry(CSnoopCCB &ccb, UINT8 ucIpType, bool bRefreshTTL)
{
    //Update FT Entry.
    bool bIsTunnel = ccb.IsExistTunnel();
    /*****************************************************
     *bIsTunnel = false,--> PeerBTS = 0;
     *bIsTunnel = true, then
     *  if IsAnchor = true, -->PeerBTS = Serving BTS Addr.
     *  if IsAnchor = false,-->PeerBTS = Anchor  BTS Addr.
     */
    UINT32 ulPeerBts = ( ( true == bIsTunnel )?\
        ( ( true == ccb.GetIsAnchor() )?ccb.GetServingBts():ccb.GetAnchorBts() ):0 );

    CFTUpdateEntry msgFTUpdateEntry;
    if ( false == msgFTUpdateEntry.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create update F.T. message failed." );
        return false;
        }
    msgFTUpdateEntry.SetEidInPayload( ccb.GetEid() );
    msgFTUpdateEntry.SetMac( ccb.GetMac() );
    msgFTUpdateEntry.SetServing( ccb.GetIsServing() );
    msgFTUpdateEntry.SetTunnel( bIsTunnel );
    msgFTUpdateEntry.SetPeerBtsID( ulPeerBts );
    msgFTUpdateEntry.SetIpType( ucIpType );
    //UpdateʱĿǰIsAuthed��Ӧ����true.
    msgFTUpdateEntry.SetAuth( true );
    msgFTUpdateEntry.setGroupId( ccb.getGroupId() );
    msgFTUpdateEntry.SetDstTid( M_TID_EB );
    msgFTUpdateEntry.setRefreshTTLOnly(bRefreshTTL);

    if ( false == msgFTUpdateEntry.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Update FT Entry Message failed." );
        msgFTUpdateEntry.DeleteMessage();
        return false;
        }
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::ForwardTraffic

DESCRIPTION:
    ֪ͨEB��ת������

ARGUMENTS:
    CMessage :��ת����Ϣ

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::ForwardTraffic(CMessage &msg)
{
    msg.SetDstTid( M_TID_EB );
    msg.SetSrcTid( M_TID_SNOOP );
    msg.SetMessageId( MSGID_TRAFFIC_FORWARD );
    if ( false == msg.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Forward Traffic Message failed." );
        //ת��������ProcessMessage()��ɾ��
        //msg.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TRAFFIC_EB );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::ForwardTrafficToWan

DESCRIPTION:
    ֪ͨEB����WAN����ת��

ARGUMENTS:
    CMessage :��ת����Ϣ

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopTrans::ForwardTrafficToWan(CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Forward traffic to WAN" );
    AddDHCPTag( msg );
    AddPPPoETag(msg);	
    msg.SetDirection( DIR_TO_WAN );
    ForwardTraffic( msg );
    return;
}



/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::ForwardTrafficToAI

DESCRIPTION:
    ֪ͨEB����AI����ת��

ARGUMENTS:
    CMessage :��ת����Ϣ

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopTrans::ForwardTrafficToAI(CMessage &msg, UINT32 ulEid)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Forward traffic to AI(eid:0x%x)", ulEid );
    DelDHCPTag( msg );
    DelPPPoETag(msg);	
    msg.SetDirection( DIR_TO_AI );
    msg.SetEID( ulEid );
    ForwardTraffic( msg );
    return;
}



/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::ForwardTrafficToTDR

DESCRIPTION:
    ֪ͨEB����TDR����ת��

ARGUMENTS:
    CMessage :��ת����Ϣ

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
#include "L3DataSocketTable.h"
#include "L3DataSocket.h"
void CSnoopTrans::ForwardTrafficToTDR(CMessage &msg, UINT32 ulDstBts)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Forward traffic to TUNNEL(BTS-%d)", ulDstBts );
    msg.SetDirection( DIR_TO_TDR );
    /*
     *ARP from TDRû��bts ID����Ϣ,ARP responseֻ��ͨ��ip port�ķ�ʽ֪ͨeb.
     *��������Ҳֻ���������ַ�ʽ
     */
    //msg.SetBTS( ulDstBts );
    BtsAddr tmpBtsAddr;
    tmpBtsAddr.IP   = 0;
    tmpBtsAddr.Port = 0;
    bool tBool = CSOCKET::GetInstance()->GetBtsPubAddrByData(ulDstBts, &tmpBtsAddr);
    if(tBool)
    {
        msg.SetBtsAddr(tmpBtsAddr.IP);
        msg.SetBtsPort(tmpBtsAddr.Port);    
        ForwardTraffic( msg );
    }
    return;
}



/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::Synchronize

DESCRIPTION:
    ֪ͨDM,ͬ��Ip List

ARGUMENTS:
    ucOp:   ������
    bNeedResp:  �Ƿ���ջ�Ӧ
    CSnoopCCB:  SnoopCCB

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::Synchronize(UINT8 ucOp, bool bNeedResp, CSnoopCCB &ccb)
{
    //Start Synchronization.
    CSyncIL msgSync;
    if ( false == msgSync.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create synchronize message failed." );
        return false;
        }
    msgSync.SetEidInPayload( ccb.GetEid() );
    msgSync.SetOp( ucOp );
    msgSync.SetIpType( ccb.GetIpType() );
    msgSync.SetNeedResp( bNeedResp );

    UTILEntry Entry;
    memcpy( Entry.aucMAC, ccb.GetMac(), M_MAC_ADDRLEN );
    Entry.ucIpType = ccb.GetIpType();
    memcpy( &( Entry.Data ), &( ccb.GetData() ), sizeof( DATA ) );
    Entry.ulRouterAreaId = ccb.GetRAID();
    Entry.ulAnchorBts = ccb.GetAnchorBts();
    msgSync.SetEntry( Entry );

    msgSync.SetDstTid( M_TID_DM );

    if ( false == msgSync.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send DAIB Sync Message failed." );
        msgSync.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_DAIB_SYNC_REQ );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::Synchronize

DESCRIPTION:
    ֪ͨDM,ͬ��Ip List
    �ṩ�˺�����Ŀ���������δ���ʱ�����CCB�쳣����£�֪ͨ
    DM����ͬ�������ķ���

ARGUMENTS:
    ucOp:   ������
    bNeedResp:  �Ƿ���ջ�Ӧ
    ulEid:  EID
    UTILEntry:  ����ʱ��Ip List.

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::Synchronize(UINT8 ucOp, bool bNeedResp, UINT32 ulEid, const UTILEntry &Entry)
{
    //Start Synchronization.
    CSyncIL msgSync;
    if ( false == msgSync.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create synchronize message failed." );
        return false;
        }
    msgSync.SetEidInPayload( ulEid );
    msgSync.SetOp( ucOp );
    msgSync.SetIpType( Entry.ucIpType );
    msgSync.SetNeedResp( bNeedResp );
    msgSync.SetEntry( Entry );
    msgSync.SetDstTid( M_TID_DM );

    if ( false == msgSync.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send DAIB Sync Message failed." );
        msgSync.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_DAIB_SYNC_REQ );
    return true;
}



/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::StartSnoopTimer

DESCRIPTION:
    ֪ͨDM,ͬ��Ip List

ARGUMENTS:
    pMac:   ��ʱ����ʱ��Ϣ��Я������Ϣ��
    ulMillSecs: ��ʱ��ʱ��(����):

RETURN VALUE:
    CTimer*:���ض�ʱ��ָ��

SIDE EFFECTS:
    none
==============================================================*/
CTimer* CSnoopTrans::StartSnoopTimer(const UINT8 *pMac, UINT32 ulMillSecs )
{
    if ( NULL == pMac )
        {
        return NULL;
        }
    CSnoopTimerExpire msgTimerExpire;
    if ( false == msgTimerExpire.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create snoop timer message failed." );
        return NULL;
        }
    msgTimerExpire.SetMac( pMac );
    msgTimerExpire.SetDstTid( M_TID_SNOOP );
    CTimer *pTimer = new CTimer( false, ulMillSecs, msgTimerExpire );
    if ( NULL == pTimer )
        {
        ////System Exception!
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create Timer failed" );
        //ɾ����Ϣ
        msgTimerExpire.DeleteMessage();
        return NULL;
        }
//    pTimer->Start();
    if ( false == pTimer->Start() )
        {
        ////System Exception!
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Timer start err." );

        delete pTimer;

        //ɾ����Ϣ��
        msgTimerExpire.DeleteMessage();
        return NULL;
        }

    return pTimer;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelSync

DESCRIPTION:
    ����Tunnel Sync��Ϣ����.

ARGUMENTS:
    *pMac:  Mac Address
RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelSync(CSnoopCCB &ccb, bool bSync0)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Sync(%d) Request msg(eid:0x%x)", !bSync0, ccb.GetEid() );
    UINT32 ulDstBtsID = ccb.GetAnchorBts();
    if ( ( 0 == ulDstBtsID ) || ( 0xFFFFFFFF == ulDstBtsID ) )
        {
        //Illegal Destination BTS.
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), 
            "Input Illegal Destination BTS: 0x%X when trying to send Tunnel Sync Request", ulDstBtsID );
        ccb.showCCB();
        return false;
        }

    //��Anchor��Sync(0)
    CTunnelSync msgTunnelSync;
    if ( false == msgTunnelSync.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel sync message failed." );
        return false;
        }
///    msgTunnelSync.SetMsgCode();
    msgTunnelSync.SetEidInPayload( ccb.GetEid() );
    msgTunnelSync.SetMac( ccb.GetMac() );
    msgTunnelSync.SetDstBtsID( ulDstBtsID );
    msgTunnelSync.SetSenderBtsID( ::bspGetBtsID() );
    msgTunnelSync.SetSenderBtsIP( ::bspGetBtsPubIp() );
    msgTunnelSync.SetSenderPort( ::bspGetBtsPubPort() );            //edit by yhw
    msgTunnelSync.SetIpType( ccb.GetIpType() );

    DATA Data;
    if ( true == bSync0 )
        {
        memset( (void*)&Data, 0, sizeof( DATA ) );
        }
    else
        {
        memcpy( (void*)&Data, (const void*)&( ccb.GetData() ), sizeof( DATA ) );
        }
    msgTunnelSync.SetDATA( Data );

    msgTunnelSync.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelSync.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Sync Request failed." );
        msgTunnelSync.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_SYNC_REQ );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelSyncResponse

DESCRIPTION:
    ����Tunnel Sync Response��Ϣ����.

ARGUMENTS:
    *pMac:  Mac address
    ulEid:  Eid
    ulDstBts: Destination BTS Address
    bSuccess: Result.

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelSyncResponse(CSnoopCCB &ccb, UINT32 ulDstBtsID,UINT32 ulDstBtsIP,UINT16 usDstPort, bool bSuccess)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Sync Response (%s) msg(eid:0x%x)", (int)( ( true == bSuccess )?"success":"fail" ), ccb.GetEid() );
    if ( ( 0 == ulDstBtsID ) || ( 0xFFFFFFFF == ulDstBtsID ) )
        {
        //Illegal Destination BTS.
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Illegal Destination BTS: 0x%X when trying to send Tunnel Sync Response", ulDstBtsID );
        return false;
        }

    CTunnelSyncResp msgTunnelSyncResp;
    if ( false == msgTunnelSyncResp.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel sync response message failed." );
        return false;
        }
 //   msgTunnelSyncResp.SetMsgCode();
    //IDLE״̬����ʱ��Eid������0��
    msgTunnelSyncResp.SetEidInPayload( ccb.GetEid() );
    msgTunnelSyncResp.SetMac( ccb.GetMac() );
    msgTunnelSyncResp.SetDstBtsID( ulDstBtsID );
    msgTunnelSyncResp.SetDstBtsIP( ulDstBtsIP );
    msgTunnelSyncResp.SetDstPort( usDstPort );
    msgTunnelSyncResp.SetResult( bSuccess );

    msgTunnelSyncResp.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelSyncResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Sync Response failed." );
        msgTunnelSyncResp.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_SYNC_RESP );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelTerminate

DESCRIPTION:
    ����Tunnel Terminate��Ϣ����.

ARGUMENTS:
    SNoopCCB:

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelTerminate(CSnoopCCB &ccb)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Terminate Request msg(eid:0x%x)", ccb.GetEid() );
    UINT32 ulDstBtsID = ccb.GetServingBts();
    if ( ( 0 == ulDstBtsID ) || ( 0xFFFFFFFF == ulDstBtsID ) )
        {
        //Illegal Destination BTS.
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Illegal Destination BTS: 0x%X when trying to send Tunnel Terminate Request", ulDstBtsID );
        return false;
        }

    CTunnelTerminate msgTunnelTerminate;
    if ( false == msgTunnelTerminate.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel terminate message failed." );
        return false;
        }
//    msgTunnelTerminate.SetMsgCode();
    msgTunnelTerminate.SetEidInPayload( ccb.GetEid() );
    msgTunnelTerminate.SetMac( ccb.GetMac() );
    msgTunnelTerminate.SetDstBtsID( ulDstBtsID );
    msgTunnelTerminate.SetSenderBtsID( ::bspGetBtsID() );
    msgTunnelTerminate.SetSenderBtsIP( ::bspGetBtsPubIp() );
    msgTunnelTerminate.SetSenderPort( ::bspGetBtsPubPort() );
    msgTunnelTerminate.SetIpType( ccb.GetIpType() );

    msgTunnelTerminate.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelTerminate.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Terminate Request failed." );
        msgTunnelTerminate.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_TERMINATE_REQ );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelTerminate

DESCRIPTION:
    ����Tunnel Terminate��Ϣ����.

ARGUMENTS:
    SNoopCCB:

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelTerminate( UINT32 ulEid, const UINT8 *pMac, UINT32 ulDstBtsID, UINT8 ucIpType)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Terminate Request msg(eid:0x%x)", ulEid );
    if ( ( NULL == pMac ) || ( 0 == ulDstBtsID ) || ( 0xFFFFFFFF == ulDstBtsID ) )
        {
        //Illegal Destination BTS.
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Illegal Destination BTS: 0x%X when trying to send Tunnel Terminate Request", ulDstBtsID );
        return false;
        }

    CTunnelTerminate msgTunnelTerminate;
    if ( false == msgTunnelTerminate.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel terminate message failed." );
        return false;
        }
//    msgTunnelTerminate.SetMsgCode();
    msgTunnelTerminate.SetEidInPayload( ulEid );
    msgTunnelTerminate.SetMac( pMac );
    msgTunnelTerminate.SetDstBtsID( ulDstBtsID );
    msgTunnelTerminate.SetSenderBtsID( ::bspGetBtsID() );
    msgTunnelTerminate.SetSenderBtsIP( ::bspGetBtsPubIp() );
    msgTunnelTerminate.SetSenderPort( ::bspGetBtsPubPort() );
    msgTunnelTerminate.SetIpType( ucIpType );

    msgTunnelTerminate.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelTerminate.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Terminate Request failed." );
        msgTunnelTerminate.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_TERMINATE_REQ );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelTerminateResponse

DESCRIPTION:
    ����Tunnel Terminate Response��Ϣ����.

ARGUMENTS:
    SNoopCCB:

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelTerminateResponse(CSnoopCCB &ccb, UINT32 ulDstBtsID,UINT32 ulDstBtsIP, UINT16 ulDstPort,  bool bSuccess)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Terminate Response (%s) msg(eid:0x%x)", (int)( ( true == bSuccess )?"success":"fail" ), ccb.GetEid() );
    if ( ( 0 == ulDstBtsID ) || ( 0xFFFFFFFF == ulDstBtsID ) )
        {
        //Illegal Destination BTS.
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Illegal Destination BTS: 0x%X when trying to send Tunnel Terminate Response", ulDstBtsID );
        return false;
        }

    CTunnelTerminateResp msgTunnelTerminateResp;
    if ( false == msgTunnelTerminateResp.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel terminate response message failed." );
        return false;
        }
//    msgTunnelTerminateResp.SetMsgCode();
    msgTunnelTerminateResp.SetEidInPayload( ccb.GetEid() );
    msgTunnelTerminateResp.SetMac( ccb.GetMac() );
    msgTunnelTerminateResp.SetDstBtsID( ulDstBtsID );
    msgTunnelTerminateResp.SetDstBtsIP( ulDstBtsIP );
    msgTunnelTerminateResp.SetDstPort( ulDstPort );
    msgTunnelTerminateResp.SetResult( bSuccess );

    msgTunnelTerminateResp.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelTerminateResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Terminate Response failed." );
        msgTunnelTerminateResp.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_TERMINATE_RESP );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelChangeAnchor

DESCRIPTION:
    ����Tunnel Change Anchor��Ϣ����.

ARGUMENTS:
    

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelChangeAnchor(UINT32 ulEid, const UINT8 *pMac, UINT32 ulAnchorBtsID, UINT8 ucIpType)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Change Anchor Request msg(eid:0x%x)", ulEid );
    if ( NULL == pMac )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Err parameter: pMac = NULL when trying to send Tunnel Change Anchor Request" );
        return false;
        }

    CTunnelChangeAnchor msgTunnelChangAnchor;
    if ( false == msgTunnelChangAnchor.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel change anchor message failed." );
        return false;
        }
//    msgTunnelChangAnchor.SetMsgCode();
    msgTunnelChangAnchor.SetEidInPayload( ulEid );
    msgTunnelChangAnchor.SetMac( pMac );
    msgTunnelChangAnchor.SetDstBtsID( ulAnchorBtsID );
    msgTunnelChangAnchor.SetSenderBtsID( ::bspGetBtsID() );
    msgTunnelChangAnchor.SetSenderBtsIP( ::bspGetBtsPubIp() );
    msgTunnelChangAnchor.SetSenderPort( ::bspGetBtsPubPort() );
    msgTunnelChangAnchor.SetIpType( ucIpType );

    msgTunnelChangAnchor.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelChangAnchor.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Change Anchor Request failed." );
        msgTunnelChangAnchor.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_CHANGE_ANCHOR_REQ );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelChangeAnchorResponse

DESCRIPTION:
    ����Tunnel Change Anchor Response��Ϣ����.

ARGUMENTS:
    SNoopCCB:

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelChangeAnchorResponse(CSnoopCCB &ccb, UINT32 ulDstBtsID,UINT32 ulDstBtsIP,UINT16 usDstPort, bool bSuccess)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Change Anchor Response (%s) msg(eid:0x%x)", (int)( ( true == bSuccess )?"success":"fail" ), ccb.GetEid() );
    if ( ( 0 == ulDstBtsID ) || ( 0xFFFFFFFF == ulDstBtsID ) )
        {
        //Illegal Destination BTS.
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Illegal Destination BTS: 0x%X when trying to send Tunnel Change Anchor Response", ulDstBtsID );
        return false;
        }

    CTunnelChangeAnchorResp msgTunnelChangeAnchorResp;
    if ( false == msgTunnelChangeAnchorResp.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel change anchor response message failed." );
        return false;
        }
//    msgTunnelChangeAnchorResp.SetMsgCode();
    msgTunnelChangeAnchorResp.SetEidInPayload( ccb.GetEid() );
    msgTunnelChangeAnchorResp.SetMac( ccb.GetMac() );
    msgTunnelChangeAnchorResp.SetDstBtsID( ulDstBtsID );
    msgTunnelChangeAnchorResp.SetDstBtsIP( ulDstBtsIP );
    msgTunnelChangeAnchorResp.SetDstPort( usDstPort );
    msgTunnelChangeAnchorResp.SetResult( bSuccess );

    msgTunnelChangeAnchorResp.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelChangeAnchorResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Change Anchor Response failed." );
        msgTunnelChangeAnchorResp.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_CHANGE_ANCHOR_RESP );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelEstablish

DESCRIPTION:
    ����Tunnel Establish��Ϣ����.

ARGUMENTS:
    

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelEstablish(UINT32 ulEid, const  UINT8 *pMac, UINT32 ulAnchorBtsID, UINT8 ucIpType, UINT32 ulFixIp)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Establish Request msg(eid:0x%x)", ulEid );
    if ( NULL == pMac )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Err parameter: pMac = NULL when trying to send Tunnel Establish Request" );
        return false;
        }

    CTunnelEstablish msgTunnelEstablish;
    if ( false == msgTunnelEstablish.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel establish message failed." );
        return false;
        }
//    msgTunnelEstablish.SetMsgCode();
    msgTunnelEstablish.SetBTS(ulAnchorBtsID);
    msgTunnelEstablish.SetEidInPayload( ulEid );
    msgTunnelEstablish.SetMac( pMac );
    //edit by yhw
    msgTunnelEstablish.SetDstBtsID( ulAnchorBtsID );
    msgTunnelEstablish.SetSenderBtsID( ::bspGetBtsID() );
    msgTunnelEstablish.SetSenderBtsIP( ::bspGetBtsPubIp() );
    msgTunnelEstablish.SetSenderPort( ::bspGetBtsPubPort() );
    msgTunnelEstablish.SetIpType( ucIpType );
    msgTunnelEstablish.SetFixIp( ulFixIp );
    msgTunnelEstablish.SetGroupId( getGroupIDbyEid(ulEid) );

    msgTunnelEstablish.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelEstablish.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Establish Request failed." );
        msgTunnelEstablish.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_ESTABLISH_REQ );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelEstablishResponse

DESCRIPTION:
    ����Tunnel Establish Response��Ϣ����.

ARGUMENTS:
    SNoopCCB:

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelEstablishResponse(CSnoopCCB &ccb, UINT32 ulDstBtsID,UINT32 ulDstBtsIP,UINT16 usDstPort,bool bSuccess)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Establish Response (%s) msg(eid:0x%x)", (int)( ( true == bSuccess )?"success":"fail" ), ccb.GetEid() );
    if ( ( 0 == ulDstBtsID ) || ( 0xFFFFFFFF == ulDstBtsID ) )
        {
        //Illegal Destination BTS.
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Illegal Destination BTS: 0x%X when trying to send Tunnel Establish Response", ulDstBtsID );
        return false;
        }

    CTunnelEstablishResp msgTunnelEstablishResp;
    if ( false == msgTunnelEstablishResp.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel establish response message failed." );
        return false;
        }
//    msgTunnelEstablishResp.SetMsgCode();
    msgTunnelEstablishResp.SetEidInPayload( ccb.GetEid() );
    msgTunnelEstablishResp.SetMac( ccb.GetMac() );
    msgTunnelEstablishResp.SetDstBtsID( ulDstBtsID );
    msgTunnelEstablishResp.SetDstBtsIP( ulDstBtsIP );
    msgTunnelEstablishResp.SetDstPort( usDstPort );
    msgTunnelEstablishResp.SetResult( bSuccess );

    msgTunnelEstablishResp.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelEstablishResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Establish Response failed." );
        msgTunnelEstablishResp.DeleteMessage();
        return false;
        }

    //������Ӧ�����ܼ���ֵ
    IncreaseMeasureByOne( OUT_TUNNEL_ESTABLISH_RESP );
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelHeartBeat

DESCRIPTION:
    ����Tunnel Heart beat��Ϣ����.

ARGUMENTS:
    

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelHeartBeat(CSnoopCCB &ccb, UINT32 ulDestBtsID)
{
    UINT8 *pMac = ccb.GetMac();
    LOG4( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Heart Beat.[MAC:xx-xx-%.2x-%.2x-%.2x-%.2x]",
        pMac[2], pMac[3], pMac[4], pMac[5] );

    CTunnelHeartBeat msgTunnelHeartBeat;
    if ( false == msgTunnelHeartBeat.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel heart beat message failed." );
        return false;
        }
    msgTunnelHeartBeat.SetEidInPayload(ccb.GetEid());
    msgTunnelHeartBeat.SetMac(ccb.GetMac());
    msgTunnelHeartBeat.SetDstBtsID(ulDestBtsID);
    msgTunnelHeartBeat.SetSenderBtsID(::bspGetBtsID());
    msgTunnelHeartBeat.SetSenderBtsIP(::bspGetBtsPubIp());
    msgTunnelHeartBeat.SetSenderPort(::bspGetBtsPubPort());
    msgTunnelHeartBeat.SetIpType(ccb.GetIpType());

    msgTunnelHeartBeat.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelHeartBeat.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Heart Beat failed." );
        msgTunnelHeartBeat.DeleteMessage();
        return false;
        }
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::SendTunnelHeartBeatResponse

DESCRIPTION:
    ����Tunnel heart beat Response��Ϣ����.

ARGUMENTS:
    SNoopCCB:

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::SendTunnelHeartBeatResponse(CSnoopCCB &ccb, UINT32 ulDstBtsID,UINT32 ulDstBtsIP,UINT16 usDstPort, bool bSuccess)
{
    LOG2( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Send Tunnel Heart Beat Response (%s) to BTS-%d", (int)( ( true == bSuccess )?"success":"fail" ), ulDstBtsID );
    CTunnelHeartBeatResp msgTunnelHeartBeatResp;
    if ( false == msgTunnelHeartBeatResp.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create tunnel heart beat response message failed." );
        return false;
        }

    msgTunnelHeartBeatResp.SetEidInPayload( ccb.GetEid() );
    msgTunnelHeartBeatResp.SetMac( ccb.GetMac() );
    msgTunnelHeartBeatResp.SetDstBtsID( ulDstBtsID );
    msgTunnelHeartBeatResp.SetDstBtsIP( ulDstBtsIP );
    msgTunnelHeartBeatResp.SetDstPort( usDstPort );
    msgTunnelHeartBeatResp.SetResult( bSuccess );

    msgTunnelHeartBeatResp.SetDstTid( M_TID_TUNNEL );

    if ( false == msgTunnelHeartBeatResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel heart beat Response failed." );
        msgTunnelHeartBeatResp.DeleteMessage();
        return false;
        }
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::DelEidTable

DESCRIPTION:
    ֪ͨDM��ɾ��ָ��Eid��

ARGUMENTS:
    ulEid:  ָ��EID

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::NotifyDelEidTable(UINT32 ulEid)
{
    CDelEidTable msgDelEidTable;
    if ( false == msgDelEidTable.CreateMessage( *( CTSnoop::GetInstance() ) ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create delete EID table message failed." );
        return false;
        }
    msgDelEidTable.SetEidInPayload( ulEid );
    msgDelEidTable.SetEID( ulEid );
    msgDelEidTable.SetDstTid( M_TID_DM );

    if ( false == msgDelEidTable.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Delete EID table Message failed." );
        msgDelEidTable.DeleteMessage();
        return false;
        }
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::IsUTRenewEnable

DESCRIPTION:
    ��DM��ѯEID�Ƿ�����Renew.

ARGUMENTS:
    ulEid: 

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::IsUTRenewEnable(UINT32 ulEid) const
{
    CPEProfile stDMConfig;
    if ( true == CTaskDm::GetInstance()->GetCPEProfile( ulEid, &stDMConfig ) )
        {
        return stDMConfig.bDHCPRenew;
        }
    return false;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::IsUTMobilityEnable

DESCRIPTION:
    ��DM��ѯEID�Ƿ�����

ARGUMENTS:
    ulEid: 

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::IsUTMobilityEnable(UINT32 ulEid) const
{
    CPEProfile stDMConfig;
    if ( true == CTaskDm::GetInstance()->GetCPEProfile( ulEid, &stDMConfig ) )
        {
        return stDMConfig.bMobility;
        }
        else
        {
        #if 1/*def WBBU_CODE*/
              LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "notifyOneCpeToRegister1." );
            notifyOneCpeToRegister(ulEid, 1);//wangwenhua add 20110119
        #endif
        }
    return false;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::IsUTAccessEnable

DESCRIPTION:
    ��DM��ѯEID�Ƿ����������û����ߡ�
    ����û��ﵽ����������������û�����

ARGUMENTS:
    ulEid: 

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::IsUTAccessEnable(UINT32 ulEid) const
{
    CPEProfile stDMConfig;
    if ( true == CTaskDm::GetInstance()->GetCPEProfile( ulEid, &stDMConfig ) )
        {
        return !( stDMConfig.bIsFull );
        }
        else
        {
        #if 1/*def WBBU_CODE*/
              LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "notifyOneCpeToRegister2." );
              notifyOneCpeToRegister(ulEid, 1);//wangwenhua add 20110119
        #endif
        }
    return false;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::IsBTSsupportMobility

DESCRIPTION:
    ��DM��ѯBTS�Ƿ�֧������/�л�

ARGUMENTS:
    ulEid: 

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopTrans::IsBTSsupportMobility(UINT32 ulLocalRAID, UINT32 ulEntryRAID, UINT32 ulEntryAnchorBts) const
{
    return CTaskDm::GetInstance()->IsBTSsupportMobility( ulLocalRAID, ulEntryRAID, ulEntryAnchorBts );
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::AddDHCPTag

DESCRIPTION:
    �����DHCP�����ж��Ƿ���Ҫ׷��82ѡ��

ARGUMENTS:
    msg: 

RETURN VALUE:
    N/A

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopTrans::AddDHCPTag(CMessage &msg)
{
    if ( ( IPTYPE_DHCP != msg.GetIpType() )
        || ( false == CTSnoop::GetInstance()->GetRelayOptionEnable() )
        || ( NULL == msg.GetDhcpPtr() ) )
        {
        return;
        }

    UINT32 uid;
    UINT32 eid;	
    UINT8 ucTagLen = 0;
    UINT8 idx = 0;
    UINT8 aucDHCPTag[ 14 ] = {0};
    aucDHCPTag[ idx++ ] = M_DHCP_OPTION_RELAY_AGENT_INFO;
    aucDHCPTag[ idx++ ] = ucTagLen;

    //DHCP�����Ƿ���Ҫ����82ѡ��
    if ( true == CTSnoop::GetInstance()->GetAgentCircuitIdEnable() )
        {
        ucTagLen += 6;
        aucDHCPTag[ idx++ ] = M_DHCP_OPTION_AGENT_CIRCUIT_ID;
        aucDHCPTag[ idx++ ]   = 4;
        *( (UINT32*)&(aucDHCPTag[ idx ]) ) = htonl( bspGetBtsID() );
        idx += 4;
        }

    if ( true == CTSnoop::GetInstance()->GetRemoteCircuitIdEnable() )
        {
        ucTagLen += 6;
        aucDHCPTag[ idx++ ] = M_DHCP_OPTION_REMOTE_CIRCUIT_ID;
        aucDHCPTag[ idx++ ]   = 4;

        eid = msg.GetEID();
        uid = l3oamGetUIDByEID(eid);
        
        *( (UINT32*)&(aucDHCPTag[ idx ]) ) = htonl( uid  );
        
        idx += 4;
        }

    DATA_assert( idx <= 14 );
    aucDHCPTag[ 1 ] = ucTagLen;
    /*+------------------------------------------------
      | 82 | 12 | 1 | 4 | ..BTSID.. | 2 | 4 | ..EID.. |
      +------------------------------------------------
    */

    UINT8 ucCopyLen = ucTagLen + 2;
    UINT8 *ptrData = (UINT8*)( msg.GetDataPtr() );
    DATA_assert( NULL != ptrData );
    UINT8 *pNewDataPtr = ptrData - ucCopyLen;

    memcpy( pNewDataPtr, ptrData, msg.GetDataLength() );
    msg.SetDataPtr( pNewDataPtr );
    msg.SetDataLength( msg.GetDataLength() + ucCopyLen );
    UINT8 *pOption = ( (UINT8*)( msg.GetDhcpPtr() ) ) + sizeof( DhcpHdr );

    //����Magic Number.
    pOption += M_DHCP_OPTION_LEN_MAGIC;

    while ( M_DHCP_OPTION_END != *pOption++ )
        {
        register UINT8 ucOptLen = *pOption++;
        pOption += ucOptLen;
        }

    //pOptionָ��ѡ��0xFF�ĺ�һ���ֽڣ�
    UINT8 *pEnd = pOption - 1;
    memcpy( pEnd, aucDHCPTag, ucCopyLen );

    //���Ͻ���ѡ��0xFF.
    *( pEnd + ucCopyLen ) = M_DHCP_OPTION_END;

    //�޸�IPͷ��Ϣ;total_len & checksum;
	EtherHdr* pEtherHeader = (EtherHdr*)(msg.GetDataPtr());
	IpHdr *pIp;
	if(IS_8023_PACKET(ntohs(pEtherHeader->usProto)))
	{
		pIp = (IpHdr*)((UINT8*)pEtherHeader + sizeof(EtherHdr) + sizeof(LLCSNAP));
	}
	else
	{
		pIp = (IpHdr*)(pEtherHeader + 1 );
	}
    
    pIp->usTotalLen += ucCopyLen;
    pIp->usCheckSum  = 0;
    //����IPͷУ���
    pIp->usCheckSum  = ip_checksum( pIp );

    //�޸�UDPͷ��Ϣ;total_len & checksum;
    UINT16 usIpHdrLen = ( ( pIp->ucLenVer ) & 0x0f ) * 4;
    UdpHdr *pUdp = (UdpHdr*)( (UINT8*)pIp + usIpHdrLen );
    pUdp->usLen += ucCopyLen;
    pUdp->usCheckSum = 0;
    pUdp->usCheckSum = udp_checksum( pIp, pUdp );

    return;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::DelDHCPTag

DESCRIPTION:
    �����DHCP�����ж��Ƿ���Ҫɾ��82ѡ��

ARGUMENTS:
    msg: 

RETURN VALUE:
    N/A

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopTrans::DelDHCPTag(CMessage &msg)
{
    if ( ( IPTYPE_DHCP != msg.GetIpType() )
        || ( false == CTSnoop::GetInstance()->GetRelayOptionEnable() )
        || ( NULL == msg.GetDhcpPtr() ) )
        {
        return;
        }

   UINT8 *pOption = ( (UINT8*)( msg.GetDhcpPtr() ) ) + sizeof( DhcpHdr );

    //����Magic Number.
    pOption += M_DHCP_OPTION_LEN_MAGIC;

    UINT8 ucOptId = *pOption++;

    while ( M_DHCP_OPTION_RELAY_AGENT_INFO != ucOptId )
        {
        if ( M_DHCP_OPTION_END == ucOptId )
            {
            break;
            }

        register UINT8 ucOptLen = *pOption++;
        pOption += ucOptLen;
        ucOptId  = *pOption++;
        }

    if ( M_DHCP_OPTION_RELAY_AGENT_INFO == ucOptId )
        {
        //pOptionָ��ѡ��Relay Option�ĺ�һ���ֽڣ�
        UINT8 ucRelayOptLen = 2 + *pOption;
        //ɾ��Relay Option.
        *( pOption - 1 ) = M_DHCP_OPTION_END;
        msg.SetDataLength( msg.GetDataLength() - ucRelayOptLen );

        //�޸�IPͷ��Ϣ;total_len & checksum;
		EtherHdr* pEtherHeader = (EtherHdr*)( msg.GetDataPtr() );
		IpHdr *pIp;
		if(IS_8023_PACKET(ntohs(pEtherHeader->usProto)))
		{
			pIp = (IpHdr*)( (UINT8*)pEtherHeader + sizeof(EtherHdr) + sizeof(LLCSNAP));
		}
		else
		{
			pIp = (IpHdr*)( pEtherHeader + 1 );
		}
       
        pIp->usTotalLen -= ucRelayOptLen;
        pIp->usCheckSum  = 0;
        //����IPͷУ���
        pIp->usCheckSum = ip_checksum( pIp );

        //�޸�UDPͷ��Ϣ;total_len & checksum;
        UINT16 usIpHdrLen = ( ( pIp->ucLenVer ) & 0x0f ) * 4;
        UdpHdr *pUdp = (UdpHdr*)( (UINT8*)pIp + usIpHdrLen );
        pUdp->usLen -= ucRelayOptLen;
        pUdp->usCheckSum = 0;
        pUdp->usCheckSum = udp_checksum( pIp, pUdp );
        }

    return;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::AddPPPoE+Tag

DESCRIPTION:
    �����PPPoE�����ж��Ƿ���Ҫ׷��Remote-ID tag ѡ��

ARGUMENTS:
    msg: 

RETURN VALUE:
    N/A

SIDE EFFECTS:
    none
==============================================================*/
#define PPPOEREMOTEIDLEN   (10)
#define PPPOE_REMOTEID_TAG_TYPE         (0x105)
#define PPPOE_DSL_FORUM                     (0xde9)
void CSnoopTrans::AddPPPoETag(CMessage &msg)
{
    if ( ( IPTYPE_PPPoE != msg.GetIpType() )
        || ( false == CTSnoop::GetInstance()->GetPPPoERemoteIdEnable() )   )
        {
        return;
        }
    EtherHdr *pEther = (EtherHdr*)msg.GetDataPtr();

    PPPoEHdr *pPPPoE = (PPPoEHdr*)( (UINT8*)pEther + sizeof( EtherHdr ) );

    if( (M_PPPoE_PADI!=pPPPoE->ucCode) && (M_PPPoE_PADR!=pPPPoE->ucCode) )
        {
        return;
        }
#pragma pack (1)
    struct 
        {
        UINT16 tag_type;
        UINT16 tag_lenth;
        UINT32 dsl_Forum;  //0xde9
        UINT8   sub_tag_Num;
        UINT8   sub_tag_len;
        UINT32 remote_ID;
        }remoteIdTag;
#pragma pack ()

    remoteIdTag.tag_type    = htons(PPPOE_REMOTEID_TAG_TYPE);
    remoteIdTag.tag_lenth   = htons(10);
    remoteIdTag.dsl_Forum   = htonl(PPPOE_DSL_FORUM);
    remoteIdTag.sub_tag_Num = 2;
    remoteIdTag.sub_tag_len = 4;
    remoteIdTag.remote_ID   = htonl( msg.GetEID() );

/*
                      1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          TAG_TYPE 0x0105      |        TAG_LENGTH  4          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          DSL_Forum=0xde9        
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | sub-tab-Num| sub-tag-len  |     sub-tag-value
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         sub-tag-value
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   
*/

    UINT8 ucCopyLen = sizeof(remoteIdTag);
    pPPPoE->usLength +=ucCopyLen;     //add SessoinLenth with lenth of RemoteID

    UINT8 *ptrData = (UINT8*)( pEther);
    DATA_assert( NULL != ptrData );
    UINT8 *pNewDataPtr = ptrData - ucCopyLen;

    memcpy( pNewDataPtr, ptrData, sizeof(EtherHdr) +sizeof(PPPoEHdr));
    msg.SetDataPtr( pNewDataPtr );
    msg.SetDataLength( msg.GetDataLength() + ucCopyLen );

    ptrData = pNewDataPtr + sizeof(EtherHdr) +sizeof(PPPoEHdr);
    memcpy(ptrData,&remoteIdTag,ucCopyLen);	

    return;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::DelPPPoETag

DESCRIPTION:
    �����PPPoE�����ж��Ƿ���Ҫɾ��Remote-ID tagѡ��

ARGUMENTS:
    msg: 

RETURN VALUE:
    N/A

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopTrans::DelPPPoETag(CMessage &msg)
{
       if ( ( IPTYPE_PPPoE != msg.GetIpType() )
        || ( false == CTSnoop::GetInstance()->GetPPPoERemoteIdEnable() )   )
        {
        return;
        }
    EtherHdr *pEther = (EtherHdr*)msg.GetDataPtr();

    PPPoEHdr *pPPPoE = (PPPoEHdr*)( (UINT8*)pEther + sizeof( EtherHdr ) );
    UINT16 usSessionLen = pPPPoE->usLength;

    if(usSessionLen<PPPOEREMOTEIDLEN)
		return;
    if( (M_PPPoE_PADO!=pPPPoE->ucCode) && (M_PPPoE_PADS!=pPPPoE->ucCode) )
    	{
    	return;
    	}
    UINT8 *pOption  =  (UINT8*)pEther + sizeof( EtherHdr ) +sizeof(PPPoEHdr);
    UINT8 *pbegin=pOption;  
    UINT16 usTagType = *(UINT16*)pOption;
    pOption+=2;	
    while ( PPPOE_REMOTEID_TAG_TYPE != usTagType )
        {
        if ( (0 == usTagType ) ||( (pOption-pbegin)>=usSessionLen))
            {
            break;
            }
        register UINT16 ucOptLen = *(UINT16*)pOption;
	    pOption+=(2+ucOptLen);
        usTagType  = *(UINT16*)pOption;
	    pOption+=2;
        }

    if ( PPPOE_REMOTEID_TAG_TYPE == usTagType )
        {
        //pOptionָ��ѡ��Relay Option�ĺ�һ���ֽڣ�
        UINT16 usRelayOptLen = *(UINT16*)pOption;
        //ɾ��Relay Option.
        UINT8 *pnewDataptr = pOption +usRelayOptLen+2;
        UINT16 movLen=pnewDataptr-pbegin;
        if(movLen>=usSessionLen)
            return;
	    memcpy(pOption-2,pnewDataptr,usSessionLen-movLen);	
	    usRelayOptLen+=4;
	    pPPPoE->usLength -=usRelayOptLen;
        msg.SetDataLength( msg.GetDataLength() - usRelayOptLen);
        }

    return;
}


UINT16 CSnoopTrans::udp_checksum(IpHdr *pIp, UdpHdr *pUdp)
{
#pragma pack (1)
    typedef struct _tag_PsdUdpHdr
    {
        UINT32      ulSrcIp;        /*Դ��ַ */
        UINT32      ulDstIp;        /*Ŀ�ĵ�ַ network byte order*/
        UINT8       ucMbz;          /* 0*/
        UINT8       ucProtocol;     /*Э������ */
        UINT16      usLen;          /* Total length */
    } PsdUdpHdr;
#pragma pack ()

    if ( ( NULL == pIp ) || ( NULL == pUdp ) )
        {
        return 0;
        }

    //���psudo IP ͷ
    PsdUdpHdr psdUdpHdr;
    memset(&psdUdpHdr, 0, sizeof( PsdUdpHdr ) );
    psdUdpHdr.ulDstIp = pIp->ulDstIp;
    psdUdpHdr.ulSrcIp = pIp->ulSrcIp;
    psdUdpHdr.ucMbz = 0;
    psdUdpHdr.ucProtocol = pIp->ucProto;
    psdUdpHdr.usLen = pUdp->usLen;     //������UDP�ĳ���

    //�ѱ��Ŀ�����һ���ټ���
    //memcpy( (UINT8*)( pPsdIpHdr + 1 ), pUdp, usUpdPktLen );

    UINT32 ulSum = 0;
    UINT16 *w    = (UINT16 *)&psdUdpHdr;
    for (UINT8 i = 0; i < 6; i++) 
    {
        ulSum += *w++;
    }

    w = (UINT16*)pUdp;
    UINT32 ulLeft = ntohs( pUdp->usLen );
    while (ulLeft > 1)
        {
        ulSum  += *w++;
        ulLeft -= 2;
        }

    if ( ulLeft == 1 )
#if 1
        //big endian
        ulSum += 0 | ((*(u_char *) w) << 8);
#else
        //little endian
        ulSum += *(u_char *) w;
#endif

    ulSum  = (ulSum >> 16) + (ulSum & 0xffff);
    ulSum += (ulSum >> 16);

    return ~ulSum & 0xffff;
}


UINT16 CSnoopTrans::ip_checksum(IpHdr *pIp)
{
    if ( NULL == pIp )
        {
        return 0;
        }

    UINT16 ulLeft = ( ( pIp->ucLenVer ) & 0x0f ) * 4;
    UINT32 ulSum = 0;
    UINT16 *w    = (UINT16 *)pIp;
    while (ulLeft > 1)
        {
        ulSum  += *w++;
        ulLeft -= 2;
        }

    if ( ulLeft == 1 )
#if 1
        //big endian
        ulSum += 0 | ((*(u_char *) w) << 8);
#else
        //little endian
        ulSum += *(u_char *) w;
#endif

    ulSum  = (ulSum >> 16) + (ulSum & 0xffff);
    ulSum += (ulSum >> 16);

    return ~ulSum & 0xffff;
}

#ifndef _NO_NVRAM_RECOVER_
/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::AddToNVRAM

DESCRIPTION:
    NVRAM������Ӧ��CCB.

ARGUMENTS:
    ccb      :CSnoopCCB

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopTrans::AddToNVRAM(CSnoopCCB &ccb)
{
    CNVRamCCBTable *pNVRamCCBTable = CSnoopFSM::s_pNVRamCCBTable;

    CMac Mac( ccb.GetMac() );
    stNVRamCCB *pNVRamCCB = pNVRamCCBTable->GetCCBByIdx( pNVRamCCBTable->BPtreeFind( Mac ) );
    if ( NULL == pNVRamCCB )
        {
        //NULL,Create CCB??
        UINT16 usFreeIdx = pNVRamCCBTable->GetFreeCCBIdxFromList();
        pNVRamCCB = pNVRamCCBTable->GetCCBByIdx( usFreeIdx );
        if ( NULL == pNVRamCCB )
            {
            LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NO_CCB ), "NVRAM used up, user is not saved and cant be properly recovered when system unexpected reboot." );
            return;
            }

        //Add to BPtree.
        pNVRamCCBTable->BPtreeAdd( Mac, usFreeIdx );
        }

////��NVRAM��д����
////bspSetNvRamWrite( (char*)pNVRamCCB, sizeof( stNVRamCCB ), 1 );

    pNVRamCCB->ucIsOccupied     = 1;
    pNVRamCCB->ulEid            = ccb.GetEid();
    memcpy( pNVRamCCB->aucMac, ccb.GetMac(), M_MAC_ADDRLEN );
    pNVRamCCB->ucIpType         = ccb.GetIpType();
    memcpy( &(pNVRamCCB->Data), &(ccb.GetData()), sizeof( DATA ) );
    pNVRamCCB->ucIsAnchor       = (UINT8)( ccb.GetIsAnchor()  );
    pNVRamCCB->ucIsServing      = (UINT8)( ccb.GetIsServing() );
    pNVRamCCB->ulServingBts     = ccb.GetServingBts();
    pNVRamCCB->usGroupID        = ccb.getGroupId();
    pNVRamCCB->ucIsRcpeFlag     = ccb.getIsRcpeFlag();
////�ر�
////bspSetNvRamWrite( (char*)pNVRamCCB, sizeof( stNVRamCCB ), 0 );
    return;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopTrans::DelFromNVRAM

DESCRIPTION:
    ��NVRAMɾ���Ѿ����ڵ�CCB.

ARGUMENTS:
    ccb      :CSnoopCCB

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CSnoopTrans::DelFromNVRAM(CSnoopCCB &ccb)
{
    CNVRamCCBTable *pNVRamCCBTable = CSnoopFSM::s_pNVRamCCBTable;

    CMac Mac( ccb.GetMac() );
    UINT16 idx = pNVRamCCBTable->BPtreeFind( Mac );
    stNVRamCCB *pNVRamCCB = pNVRamCCBTable->GetCCBByIdx( idx );
    if ( NULL != pNVRamCCB )
        {
        pNVRamCCBTable->BPtreeDel( Mac );
        pNVRamCCBTable->InsertFreeCCB( idx );
        //����֮
        pNVRamCCB->ucIsOccupied = 0;
////////UINT8 ucOccupied = 0;
////////bspNvRamWrite( (char*)&( pNVRamCCB->ucIsOccupied ), (char*)&ucOccupied, sizeof( ucOccupied ) );
        }
}
#endif


/*============================================================
MEMBER FUNCTION:
    CParentTunnelSyncReqTrans::Action

DESCRIPTION:
    Parent״̬��Tunnel Synchronize Request�Ĵ���

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CParentTunnelSyncReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "-->Parent Tunnel Sync Request Action(eid:0x%x)", ccb.GetEid() );
    CTunnelSync msgTunnelSyncReq( msg );
    UINT8 ucIpType = msgTunnelSyncReq.GetIpType();
    DATA Data = msgTunnelSyncReq.GetDATA();
    UINT32 ulFromBtsID = msgTunnelSyncReq.GetSenderBtsID();

    if ( IPTYPE_DHCP == ucIpType )
        {
        UINT32 ulKeyIp = Data.stIpLease.ulIp;
        UINT32 ulLeaseMoment = Data.stIpLease.ulLease;
        if ( ( 0 == ulKeyIp ) && ( 0 == ulLeaseMoment ) )
            {
            //Sync(0)
            ProcSync0( ccb, ulFromBtsID,msgTunnelSyncReq.GetSenderBtsIP(),msgTunnelSyncReq.GetSenderPort() );

            //Reclaim CCB.
            return TargetState;
            }
        else
            {
            //ͬ���޸Ĺ���ʱ��
            ProcSyncIPLease( ccb, ulFromBtsID,msgTunnelSyncReq.GetSenderBtsIP(),msgTunnelSyncReq.GetSenderPort(), ulKeyIp, ulLeaseMoment );

            ////
            return ccb.GetCurrentState();
            }
        }
    else
        {
        //PPPoE��Fix IP
        CMac Mac( Data.stSessionMac.aucServerMac );
        if ( ( 0 == Data.stSessionMac.usSessionId ) && ( true == Mac.IsZero() ) )
            {
            //Sync(0)
            ProcSync0( ccb, ulFromBtsID,msgTunnelSyncReq.GetSenderBtsIP(),msgTunnelSyncReq.GetSenderPort() );

            //Reclaim CCB.
            return TargetState;
            }
        else
            {
            //Send Tunnel Sync Response. FAIL
            SendTunnelSyncResponse( ccb, ulFromBtsID,msgTunnelSyncReq.GetSenderBtsIP(),msgTunnelSyncReq.GetSenderPort(), false );

            //PPPoE�û����ᷢ����������Ϣ
            DATA_assert(0);

            ///
            return ccb.GetCurrentState();
            }
        }
}


/*============================================================
MEMBER FUNCTION:
    CParentTunnelSyncReqTrans::ProcSync0

DESCRIPTION:
    ����Sync0��Ϣ  

ARGUMENTS:
    Mac: Mac��ַ
    ulDstBts: Destination BTS Address.

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CParentTunnelSyncReqTrans::ProcSync0(CSnoopCCB &ccb, UINT32 ulDstBtsID,UINT32 ulDstBtsIP,UINT16 usDstPort)
{
    if ( IPTYPE_FIXIP == ccb.GetIpType() )
        {
        //FIXIP�û���Sync0�Ĵ���
        //FIXIP�û���ServingBTS-1���ε���һ��ServingBTS-2��
        //ServingBTS-1�ᷢһ��Sync0.ServingBTS-2�ᷢTunnelEstablish Request.
        //���Sync0����TunnelEstablish Request.�򲻴���
        if ( ( true != ccb.IsExistTunnel() )
            || ( ccb.GetServingBts() != ulDstBtsID ) )
            {
            //�Ѿ�ִ����TunnelEstablish Request.
            return;
            }
        }

    //FT Del Entry.
    DelFTEntry( ccb.GetMac() );

    //Delete timer.
    ccb.DeleteTimer();

    ccb.stopHeartBeat();

    if ( true == ccb.GetIsServing() )
        {
        //Start Synchronization.
        Synchronize( M_SYNC_DELETE, false, ccb );
        }

    if ( true == ccb.GetIsAuthed() )
        {
        if ( IPTYPE_PPPoE != ccb.GetIpType() )
            {
            //DHCP��Fix IP�û�,,Delete ARP Ip List Entry.
            ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );
            }

#ifndef _NO_NVRAM_RECOVER_
        //����NVRam.
        DelFromNVRAM( ccb );
#endif
        }

    //Response
    SendTunnelSyncResponse( ccb, ulDstBtsID,ulDstBtsIP,usDstPort, true );

    return;
}



/*============================================================
MEMBER FUNCTION:
    CParentTunnelSyncReqTrans::ProcSyncIPLease

DESCRIPTION:
    ����Sync Ip Lease��Ϣ  
    ֻ��DHCP�û��ſ��ܴ������Ϣ

ARGUMENTS:
    Mac: Mac��ַ

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CParentTunnelSyncReqTrans::ProcSyncIPLease(CSnoopCCB &ccb, UINT32 ulDstBtsID,UINT32 ulDstBtsIP,UINT16 usDstPort, UINT32 ulKeyIp, UINT32 ulLeaseMoment)
{
    DATA Data = ccb.GetData();
    if ( ulKeyIp == Data.stIpLease.ulIp )
        {
        //����CCB����ʱ��
        Data.stIpLease.ulLease = ulLeaseMoment; //ulLeaseInSeconds + time( NULL );��Ϊ�������ľ���Moment.
        ccb.SetDATA( Data );

        //Start Synchronization.
        Synchronize( M_SYNC_UPDATE, false, ccb );

        //Send Tunnel Sync Response.
        SendTunnelSyncResponse( ccb, ulDstBtsID,ulDstBtsIP,usDstPort, true );

        //Reset Timer.
        CTimer *pTimer = ccb.GetTimer();
        if ( NULL != pTimer )
            {
            pTimer->Stop();
            pTimer->SetInterval( ( ulLeaseMoment - time( NULL ) ) * 1000 );
            pTimer->Start();
            }
        else
            {
            pTimer = StartSnoopTimer( ccb.GetMac(), ( ulLeaseMoment - time( NULL ) ) * 1000 );
            ccb.SetTimer( pTimer );
            }

#ifndef _NO_NVRAM_RECOVER_
        //���µ�NVRam.
        AddToNVRAM( ccb );
#endif
        }
    else
        {
        //Send Tunnel Sync Response.
        SendTunnelSyncResponse( ccb, ulDstBtsID,ulDstBtsIP,usDstPort, false );
        //Assert
        DATA_assert(0);
        }
}


/*============================================================
MEMBER FUNCTION:
    CParentRoamReqTrans::Action

DESCRIPTION:
    Parent״̬��Roam Request�Ĵ���
    ������DHCP/PPPoE�û�,FixIP�û����ڴ˺�������

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CParentRoamReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    CRoam msgRoamReq( msg );
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "-->Parent Roam Request Action(eid:0x%x)", msgRoamReq.GetEidInPayload() );
    UTILEntry ILEntry = msgRoamReq.GetEntry();
    UINT8  *pMac = ILEntry.aucMAC;
    UINT8  ucIpType = ILEntry.ucIpType;
    UINT32 ulRouterAreaId = ILEntry.ulRouterAreaId;
    UINT32 ulAnchorBts = ILEntry.ulAnchorBts;
    UINT32 ulLocalBts = ::bspGetBtsID();
    UINT32 ulEid = msgRoamReq.GetEidInPayload();
    UINT32 ulLocalRAID = CTSnoop::GetInstance()->GetRouterAreaId();
    
    //ɾ��ԭccb��Ķ�ʱ������Դ��Ϣ
    ccb.DeleteMsg();
    ccb.DeleteTimer();
    ccb.stopHeartBeat();
    ccb.setCreateTime((UINT32)time( NULL ) );
    
    if ( ( IPTYPE_DHCP == ucIpType )
    && ( ILEntry.Data.stIpLease.ulLease < (UINT32)time( NULL ) ) )
    {
        //DHCP�û���IP��ַ�Ѿ�����
        //��ֹ�û�����
        CMac Mac( pMac );
        UINT8 strMac[ M_MACADDR_STRLEN ];
        LOG1( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "User[%s] lease expired.",(int) Mac.str( strMac ) );
        
        Synchronize( M_SYNC_DELETE, false, ulEid, ILEntry );
        
        //Reclaim CCB.
        return STATE_SNOOP_IDLE;
    }
    
    bool   bSuccess = true;
    bool   bUTMobilityEnable = IsUTMobilityEnable( ulEid );
    
    if ( ulLocalRAID != ulRouterAreaId )
    {
        //���ε�����Raid��BTS��
        if ( ulLocalBts != ulAnchorBts )
        {
            //���ε�����Raid��BTS�ϣ���BTS֧��UT����
            bSuccess &= RoamOutRaid( ulEid, ILEntry, ccb);
            
            //����IpList,������ͬ��
            bSuccess &= RoamAddIp( ccb,
             ulEid, 
             false,                      /*bIsAnchor*/
             ulAnchorBts,                /*Anchor of CCB*/
             ulLocalRAID,                /*����RAID*/
             ILEntry );                  /*UTILEntry **/
            
            if ( true == bSuccess )
            {
                //����ROAMING״̬���ȴ�Tunnel Establish Response.
                return STATE_SNOOP_ROAMING;
            }
        }
        else//·������ͬ,��վ����ͬ,��Ϊ��ͬһ����վ,���Ӵ���jiaying20100721
        {
            LOG2( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "BTS[%d] doesn't match its RAID[%d], UT data need update", ulAnchorBts, ulRouterAreaId );
            //�ص�Anchor BTS
            bSuccess &= RoamBackToAnchor( ulEid, pMac, ccb.GetServingBts(), ucIpType, ccb.IsExistTunnel(), ccb);
            
            //����IpList,������ͬ��
            bSuccess &= RoamAddIp( ccb,
             ulEid, 
             true,               /*bIsAnchor*/
             ulLocalBts,         /*ulAnchorBts*/
             ulLocalRAID,        /*����RAID*/
             ILEntry );          /*UTILEntry **/
            
            ccb.stopHeartBeat();
            ccb.SetRAID( ulLocalRAID );//wangwenhua add 20110329
            if ( true == bSuccess )
            {
                if ( ( IPTYPE_PPPoE != ucIpType )
                && ( false == ccb.IsExistTunnel() ) )
                {
                    //�ظ�ע���DHCP��Fix IP�û�,,Add ARP IpList Entry.
                    ::ARP_AddILEntry( ulEid,  pMac, ILEntry.Data.stIpLease.ulIp, ccb.getIsRcpeFlag());
                }
                
                #ifndef _NO_NVRAM_RECOVER_
                //���µ�NVRam.
                AddToNVRAM( ccb );
                #endif
                Synchronize( M_SYNC_UPDATE, false, ccb );//ͬ����dm������iplist
                return STATE_SNOOP_BOUND;
            }
        }
    }
    
    if ( ( ulLocalRAID == ulRouterAreaId ) && ( ulLocalBts != ulAnchorBts ) )
    {
        //ͬһRaid��������Anchor BTS        
        bSuccess &= RoamInSameRaid( ulEid, pMac, ulAnchorBts, ucIpType, ccb);
        ccb.SetIsRealAnchor(false);//wangwenhua add 20081022
        //����IpList,������ͬ��
        bSuccess &= RoamAddIp( ccb,
        ulEid, 
        true,               /*bIsAnchor*/
        ulLocalBts,         /*ulAnchorBts*/
        ulLocalRAID,        /*����RAID*/
        ILEntry );          /*UTILEntry **/
		 if (IPTYPE_PPPoE != ccb.GetIpType())//wangwenhua add 20110718 send two times 
	       {
	        	CTaskARP::GetInstance()->sendG_ARP(pMac, ccb.GetData().stIpLease.ulIp);//
			CTaskARP::GetInstance()->sendG_ARP(pMac, ccb.GetData().stIpLease.ulIp);
	        }
        
        if ( true == bSuccess )
        {
            if ( IPTYPE_PPPoE != ucIpType )
            {
                //DHCP��Fix IP�û�,,Add ARP IpList Entry.
                ::ARP_AddILEntry( ulEid,  pMac, ILEntry.Data.stIpLease.ulIp, ccb.getIsRcpeFlag());
            }
            
            #ifndef _NO_NVRAM_RECOVER_
            //���µ�NVRam.
            AddToNVRAM( ccb );
            #endif
            //AnchorBTS�޸ģ�ͬ����DM��CPE.
            ILEntry.ulAnchorBts = ulLocalBts;
            Synchronize( M_SYNC_UPDATE, false, ulEid, ILEntry );
            
            return STATE_SNOOP_BOUND;
        }
       
    }
    
    if ( ( ulLocalRAID == ulRouterAreaId ) && ( ulLocalBts == ulAnchorBts ) )
    {
        //�ص�Anchor BTS
        bSuccess &= RoamBackToAnchor( ulEid, pMac, ccb.GetServingBts(), ucIpType, ccb.IsExistTunnel(), ccb);
        
        //����IpList,������ͬ��
        bSuccess &= RoamAddIp( ccb,
         ulEid, 
         true,               /*bIsAnchor*/
         ulLocalBts,         /*ulAnchorBts*/
         ulLocalRAID,        /*����RAID*/
         ILEntry );          /*UTILEntry **/
        
        ccb.stopHeartBeat();
        
        if ( true == bSuccess )
        {
            if ( ( IPTYPE_PPPoE != ucIpType )
            && ( false == ccb.IsExistTunnel() ) )
            {
                //�ظ�ע���DHCP��Fix IP�û�,,Add ARP IpList Entry.
                ::ARP_AddILEntry( ulEid,  pMac, ILEntry.Data.stIpLease.ulIp, ccb.getIsRcpeFlag());
            }
            
            #ifndef _NO_NVRAM_RECOVER_
            //���µ�NVRam.
            AddToNVRAM( ccb );
            #endif
            return STATE_SNOOP_BOUND;
        }
    }
    
    // bSuccess = false;
    //ĳ�������ڳ���֪ͨDMɾ�������ε�Ip List��
    Synchronize( M_SYNC_DELETE, false, ulEid, ILEntry );
    
    //Delete FT Entry
    DelFTEntry( ccb.GetMac() );
    
    //delete timer.
    ccb.DeleteTimer();
    ccb.stopHeartBeat();
    
    //Reclaim CCB.
    return STATE_SNOOP_IDLE;
}


/*============================================================
MEMBER FUNCTION:
    CParentRoamReqTrans::RoamOut

DESCRIPTION:
    ���ε�����RouterArea Id��BTS��

ARGUMENTS:
    *pMac: Mac��ַ

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CParentRoamReqTrans::RoamOutRaid(UINT32 ulEid, const UTILEntry &ILEntry, CSnoopCCB &ccb)
{
    UINT8  ucIpType = ILEntry.ucIpType;
    UINT32 ulFixIp = ( IPTYPE_FIXIP == ucIpType )?ILEntry.Data.stIpLease.ulIp:0;
    //Tunnel Establish
    if ( true != SendTunnelEstablish( ulEid, 
                    ILEntry.aucMAC,
                    ILEntry.ulAnchorBts,
                    ucIpType,
                    ulFixIp ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Establish Request failed when roam out." );
        return false;
        }

    //֪ͨEB������FT Entry
    return AddFTEntry( ulEid,
        ILEntry.aucMAC,
        true,       /*bool bIsServing*/
        true,       /*bool bIsTunnel*/ 
        ILEntry.ulAnchorBts,    /*UINT32 ulPeerBtsAddr*/
        ucIpType,   /*UINT8 ucIpType*/
        true,       /*bool bIsAuthed*/
        ccb);     
}


/*============================================================
MEMBER FUNCTION:
    CParentRoamReqTrans::RoamInSameRaid

DESCRIPTION:
    ���ε�ͬһRouterArea Id��������Anchor BTS��

ARGUMENTS:
    Mac: Mac��ַ

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CParentRoamReqTrans::RoamInSameRaid(UINT32 ulEid, const UINT8 *pMac, UINT32 ulAnchorBts, UINT8 ucIpType, CSnoopCCB &ccb)
{
    if ( NULL == pMac )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Err parameter: pMac = NULL when roam internal." );
        return false;
        }

    //Change Anchor
    if ( false == SendTunnelChangeAnchor( ulEid, 
                    pMac,
                    ulAnchorBts,
                    ucIpType ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Change Anchor Request failed when roam internal." );
        return false;
        }

    //֪ͨEB������FT Entry
    return AddFTEntry( ulEid,
        pMac,
        true,       /*bool bIsServing*/
        false,      /*bool bIsTunnel*/ 
        0,          /*UINT32 ulPeerBtsAddr*/
        ucIpType,   /*UINT8 ucIpType*/
        true,       /*bool bIsAuthed*/
        ccb);     
}



/*============================================================
MEMBER FUNCTION:
    CParentRoamReqTrans::RoamBackToAnchor

DESCRIPTION:
    ���λ�Anchor BTS��

ARGUMENTS:
    Mac: Mac��ַ

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CParentRoamReqTrans::RoamBackToAnchor(UINT32 ulEid, const UINT8 *pMac, UINT32 ulPreServingBts, UINT8 ucIpType, bool bExistTunnel, CSnoopCCB &ccb)
{
    if ( NULL == pMac )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), "Input Err parameter: pMac = NULL when roam back." );
        return false;
        }

    //Send Tunnel Terminate.
    if ( ( true == bExistTunnel )
        && true != SendTunnelTerminate( ulEid, 
                    pMac,
                    ulPreServingBts,
                    ucIpType ) )
        {
        //�����λ�����CCB������BTS�������
        //��BTS������AnchorBTS����Ҫ��������
        //FIX IP�û�����Tunnel Terminate Req.
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send Tunnel Terminate Request failed when roam back." );
        return false;
        }

    //֪ͨEB������FT Entry
    return AddFTEntry( ulEid,
        pMac,
        true,       /*bool bIsServing*/
        false,      /*bool bIsTunnel*/
        0,          /*UINT32 ulPeerBtsAddr*/
        ucIpType,   /*UINT16 ucIpType*/
        true,       /*bool bIsAuthed*/
        ccb);     
}


/*============================================================
MEMBER FUNCTION:
    CParentRoamReqTrans::RoamAddIp

DESCRIPTION:
    Ϊ�����û�����Ip List

ARGUMENTS:
    Mac: Mac��ַ

RETURN VALUE:
    bool: true,�����ɹ���false,�쳣ʧ��

SIDE EFFECTS:
    none
==============================================================*/
bool CParentRoamReqTrans::RoamAddIp(CSnoopCCB &ccb, UINT32 ulEid, bool bIsAnchor, UINT32 ulAnchorBts, UINT32 ulLocalRAID, UTILEntry &ILEntry)
{
    UINT8  *pMac = ILEntry.aucMAC;
    bool   bSuccess = true;
    UINT32 ulRouterAreaId = ILEntry.ulRouterAreaId;
    UINT8  ucIpType = ILEntry.ucIpType;
    UINT32 ulLocalBts = ::bspGetBtsID();
    if (( ulLocalRAID == ulRouterAreaId )||(ulLocalBts==ulAnchorBts))
        {
        if ( IPTYPE_DHCP == ucIpType )
            {
            //////���ñ���ʣ�����ʱ�䣻
            UINT32 ulMillSecs = 1000 * ( ILEntry.Data.stIpLease.ulLease - time( NULL ) );
            //Start timer.
            CTimer *pTimer = StartSnoopTimer( pMac, ulMillSecs );
            if ( NULL == pTimer )
                {
                return false;
                }
            ccb.DeleteTimer();
            ccb.SetTimer( pTimer );
            }
        ////PPPoE��FIX IP�û�����Ҫ��ʱ��
        }
    else
        {
        //�ѷ���Tunnel Establish Req,�趨�ȴ�ʱ��
        //Start timer.��ʱ���ĳ���Ӧ�ñ�Tunnel�ȴ���Ӧ��ʱ������ź���
        CTimer *pTimer = StartSnoopTimer( pMac, M_SNOOP_FTIMER_INTERVAL_8Sec*60 );
        if ( NULL == pTimer )
            {
            return false;
            }
        ccb.SetTimer( pTimer );
        }

    //Add CCB
    ccb.SetIsAuthed( true );
    ccb.SetEid( ulEid );
    ccb.SetMac( pMac );
    ccb.SetIpType( ucIpType );
    ccb.SetDATA( ILEntry.Data );
    ccb.SetRAID( ulRouterAreaId );
    ccb.SetIsAnchor( bIsAnchor );
    ccb.SetIsServing( true );
    ccb.SetAnchorBts( ulAnchorBts );
    ccb.SetServingBts( ::bspGetBtsID() );
    ccb.setGroupId( getGroupIDbyEid(ulEid) );
    //ccb.SaveMsg( NULL );

    //start synchronization.
    //bSuccess = Synchronize( M_SYNC_UPDATE, false, ccb );

    return bSuccess;
}


/*============================================================
MEMBER FUNCTION:
    CParentTunnelEstablishReqTrans::Action

DESCRIPTION:
    Tunnel Establish Requestֻ��BOUND/RENEWING״̬����
    ������״̬��ֻ��ʧ����Ӧ�����������ط���Ϣ

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CParentTunnelEstablishReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "-->Parent Tunnel Establish Request Action(eid:0x%x)", ccb.GetEid() );
    UINT32 tt = (UINT32)time( NULL );
	#if 0
    if((tt -ccb.getCreateTime())<=2)//�����ע����Ϣ��2������Ϊ���ӳ���Ϣ������
    {
        LOG1( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Parent Tunnel Establish Request Action(eid:0x%x), time <=2, discard msg", ccb.GetEid());
        return ccb.GetCurrentState();
    }
	#endif
    CTunnelEstablish msgTunnelEstablishReq( msg );
    //��Ӧʧ��
    //edit by yhw 
    SendTunnelEstablishResponse( ccb, msgTunnelEstablishReq.GetSenderBtsID(),msgTunnelEstablishReq.GetSenderBtsIP(),msgTunnelEstablishReq.GetSenderPort(), false );
    //���ı�CCB״̬
    return ccb.GetCurrentState();
}


/*============================================================
MEMBER FUNCTION:
    CParentTunnelTerminateReqTrans::Action

DESCRIPTION:
    Tunnel Terminate Requestֻ��BOUND/RENEWING״̬����
    ������״̬��ֻ��ʧ����Ӧ�����������ط���Ϣ

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CParentTunnelTerminateReqTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "-->Parent Tunnel Terminate Request Action(eid:0x%x)", ccb.GetEid() );
    UINT32 tt = (UINT32)time( NULL );
	#if 0
    if((tt -ccb.getCreateTime())<=2)//�����ע����Ϣ��2������Ϊ���ӳ���Ϣ������
    {
        LOG1( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->Parent Tunnel Terminate Request Action(eid:0x%x), time <=2, discard msg", ccb.GetEid());
        return ccb.GetCurrentState();
    }
	#endif
    CTunnelTerminate msgTunnelTerminateReq( msg );
    //��Ӧʧ��
    SendTunnelTerminateResponse( ccb, msgTunnelTerminateReq.GetSenderBtsID(),msgTunnelTerminateReq.GetSenderBtsIP(),msgTunnelTerminateReq.GetSenderPort(), false );
    //���ı�CCB״̬
    return ccb.GetCurrentState();
}


/*============================================================
MEMBER FUNCTION:
    CParentTunnelChgAnchorRespTrans::Action

DESCRIPTION:
    �յ�ChangeAnchor Response��Ϣ����֪ͨARP��ARP probe.

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/

FSMStateIndex CParentTunnelChgAnchorRespTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "-->Parent Tunnel ChangeAnchor Response Action(eid:0x%x)", ccb.GetEid() );

    if (IPTYPE_PPPoE != ccb.GetIpType())
        {
  //      CTaskARP::GetInstance()->sendG_ARP(ccb.GetMac(), ccb.GetData().stIpLease.ulIp);//wangwenhua mark 20110718
        }
    //���ı�CCB״̬
    return ccb.GetCurrentState();
}


/*============================================================
MEMBER FUNCTION:
    CParentAddFixIPTrans::Action

DESCRIPTION:
    Add FixIP��Ϣ��IDLE֮��״̬�Ĵ���,�������ͬһ������,
    ��ɾ��ԭ�û�,����FixIP�û�

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CParentAddFixIPTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "-->Parent Add Fix IP Action(eid:0x%x)", ccb.GetEid() );
    
    CADDFixIP msgAddFixIp( msg );
    UTILEntry stILEntry = msgAddFixIp.GetEntry();
    UINT32 ulEid = msgAddFixIp.GetEidInPayload();
    UINT8  *pMac = stILEntry.aucMAC;
    UINT32 ulRouterAreaId = stILEntry.ulRouterAreaId;
    UINT32 ulAnchorBts = stILEntry.ulAnchorBts;
    UINT32 ulLocalBts = ::bspGetBtsID();
    
    ////ɾ��ԭ�е�CCB.
    if ( true == ccb.GetIsAuthed() )
    {
        if ( true == ccb.GetIsAnchor() )
        {
            //Anchor BTS
            #ifndef _NO_NVRAM_RECOVER_
            //���µ�NVRam.
            DelFromNVRAM( ccb );
            #endif
            if ( IPTYPE_PPPoE != ccb.GetIpType() )
            {
                ::ARP_DelILEntry( ccb.GetData().stIpLease.ulIp, ccb.GetMac(),0 );
            }
        }
    }
    
    ccb.DeleteMsg();
    ccb.DeleteTimer();
    ////DelFTEntry( ccb.GetMac() );     //�������������ݱ���.
    ccb.stopHeartBeat();
    
    if ( ulEid != ccb.GetEid() )
    {
        //֪ͨԭEID��ɾ������
        DATA_assert( 0 );
        Synchronize( M_SYNC_DELETE, false, ccb.GetEid(), stILEntry );
    }
    
    ////ɾ����,�ؽ�FixIP��CCB.
    UINT32 ulLocalRAID = CTSnoop::GetInstance()->GetRouterAreaId();
    bool   bSuccess = true;
    if ( ulLocalRAID != ulRouterAreaId )//��ͬ·������Ҫ�������
    {
        if (ulAnchorBts != ulLocalBts)
        {
            //����Raid��BTS������FixIp
            bSuccess &= RoamOut( ulEid, stILEntry, ccb);
            
            //����IpList,������ͬ��
            bSuccess &= AddFixIpEntry( ccb,
            ulEid, 
            false,                      /*bIsAnchor*/
            ulAnchorBts,                /*Anchor of CCB*/
            ulLocalRAID,                /*����RAID*/
            stILEntry );                /*UTILEntry **/
            
            if ( true == bSuccess )
            {
                //����ROAMING״̬���ȴ�Tunnel Establish Response.
                return STATE_SNOOP_ROAMING;
            }
        }
        else//·������ͬ,��վ����ͬ,��Ϊ��ͬһ����վ,���Ӵ���jiaying20100721
        {
            LOG2( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "BTS[%d] doesn't match its RAID[%d], UT data need update", ulAnchorBts, ulRouterAreaId );
            bSuccess &= AddFTEntry( ulEid,
            pMac,
            true,       /*bool bIsServing*/
            false,      /*bool bIsTunnel*/ 
            0,          /*UINT32 ulPeerBtsAddr*/
            IPTYPE_FIXIP, /*UINT8 ucIpType*/
            true,        /*bool bIsAuthed*/
            ccb);     
            
            //����IpList,������ͬ��
            bSuccess &= AddFixIpEntry( ccb,
             ulEid, 
             true,               /*bIsAnchor*/
             ulLocalBts,         /*ulAnchorBts*/
             ulLocalRAID,        /*����RAID*/
             stILEntry );        /*UTILEntry **/
            
            if ( true == bSuccess )
            {
                //Fix IP�û�,,Add ARP IpList Entry.
                ::ARP_AddILEntry( ulEid,  pMac, stILEntry.Data.stIpLease.ulIp, ccb.getIsRcpeFlag());
                
                #ifndef _NO_NVRAM_RECOVER_
                //���µ�NVRam.
                AddToNVRAM( ccb );
                #endif
                //-->Bound
                return TargetState;
            }
        }
    }
    else //��ͬ·��������β����?
    {
    #if 0
        if(ulAnchorBts != ulLocalBts)//to send changing anchor msg to the bts   wangwenhua add 20081023
        {            
            SendTunnelChangeAnchor( ulEid, 
            pMac,
            ulAnchorBts,
            IPTYPE_FIXIP ) ;
            ccb.SetIsRealAnchor(false);//wangwenhua add 20081022            
            	 // if (IPTYPE_PPPoE != ccb.GetIpType()) //wangwenhua add 20110718
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
         true,       /*bool bIsAuthed*/
         ccb);     
        
        //����IpList,������ͬ��
        bSuccess &= AddFixIpEntry( ccb,
         ulEid, 
         true,               /*bIsAnchor*/
         ulLocalBts,         /*ulAnchorBts*/
         ulLocalRAID,        /*����RAID*/
         stILEntry );        /*UTILEntry **/
		
        if(ulAnchorBts != ulLocalBts)//to send changing anchor msg to the bts   wangwenhua add 20081023
        {            
            SendTunnelChangeAnchor( ulEid, 
            pMac,
            ulAnchorBts,
            IPTYPE_FIXIP ) ;
            ccb.SetIsRealAnchor(false);//wangwenhua add 20081022            
           }
            	 // if (IPTYPE_PPPoE != ccb.GetIpType()) //wangwenhua add 20110718
	        {
	        	CTaskARP::GetInstance()->sendG_ARP(pMac, ccb.GetData().stIpLease.ulIp);
			CTaskARP::GetInstance()->sendG_ARP(pMac, ccb.GetData().stIpLease.ulIp);
	        }
		
        if ( true == bSuccess )
        {
            //Fix IP�û�,,Add ARP IpList Entry.
            ::ARP_AddILEntry( ulEid,  pMac, stILEntry.Data.stIpLease.ulIp, ccb.getIsRcpeFlag());
            
            #ifndef _NO_NVRAM_RECOVER_
            //���µ�NVRam.
            AddToNVRAM( ccb );
            #endif
            //-->Bound
            return TargetState;
        }
    }
    
    //FixIP ��ͬ����DM��CPE. �Ա��´����ݷ���ʱ���Բ�ѯ��
    //Synchronize( M_SYNC_DELETE, false, ulEid, stILEntry );
    
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
    CParentAddFixIPTrans::RoamOut

DESCRIPTION:
    ���õĹ̶�IP������RouterArea Id��BTS��

ARGUMENTS:
    *pMac: Mac��ַ

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CParentAddFixIPTrans::RoamOut(UINT32 ulEid, const UTILEntry &ILEntry, CSnoopCCB &ccb)
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

    //֪ͨEB������FT Entry
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
    CParentAddFixIPTrans::AddFixIpEntry

DESCRIPTION:
    ���ӹ̶�IP��CCB

ARGUMENTS:
    Mac: Mac��ַ

RETURN VALUE:
    bool: true,�����ɹ���false,�쳣ʧ��

SIDE EFFECTS:
    none
==============================================================*/
bool CParentAddFixIPTrans::AddFixIpEntry(CSnoopCCB &ccb, UINT32 ulEid, bool bIsAnchor, UINT32 ulAnchorBts, UINT32 ulLocalRAID, const UTILEntry &ILEntry)
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
    CParentHeartBeatTimerTrans::Action

DESCRIPTION:
    HeartBeat��ʱ����Ϣֻ��SYNCING/BOUND/RENEWING/ROAM״̬����
    ������anchorBTS��servingBTS������,���ջ�Ӧ������TTL
    servingBTS������������������Ӧ

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CParentHeartBeatTimerTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "-->Parent heart beat timer Action" );

    //timer out
    DATA_assert((false == ccb.GetIsServing()) && (true == ccb.IsExistTunnel()));
    //�յ��쳣��Ϣ,������,jiaying20100720
    if(!((false == ccb.GetIsServing()) && (true == ccb.IsExistTunnel())))//������Ϣ,��Ӧ��
    {
        ccb.stopHeartBeat();
        return ccb.GetCurrentState();
    }
    //��������Ϣ
    SendTunnelHeartBeat(ccb, ccb.GetServingBts());

    //���ı�CCB״̬
    return ccb.GetCurrentState();
}

/*============================================================
MEMBER FUNCTION:
    CParentHeartBeatTrans::Action

DESCRIPTION:
    ����������
    ������anchorBTS��servingBTS������
    servingBTS������������������Ӧ
    anchorBTS����������Ӧ������TTL

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CParentHeartBeatTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    if (STATE_SNOOP_IDLE != ccb.GetCurrentState())
        {
        UINT8 *pMac = ccb.GetMac();
        LOG4( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Rx MAC(XX-XX-%.2X-%.2X-%.2X-%.2X) Tunnel Heart Beat, and Tx Heart Beat Response",
            pMac[2], pMac[3], pMac[4], pMac[5] );

        //anchor->serving
//        DATA_assert((true == ccb.GetIsServing()) && (true == ccb.IsExistTunnel()));
	 //�յ��쳣��Ϣ,������,jiaying20100720
        if(!((true == ccb.GetIsServing()) && (true == ccb.IsExistTunnel())))//������Ϣ,��Ӧ��
        {        
            return ccb.GetCurrentState();
        }
        CTunnelHeartBeat msgTunnelHeartBeat(msg);
        //������ ��Ӧ
        SendTunnelHeartBeatResponse(ccb, msgTunnelHeartBeat.GetSenderBtsID(), msgTunnelHeartBeat.GetSenderBtsIP(),msgTunnelHeartBeat.GetSenderPort(), true);
        }

    //���ı�CCB״̬
    return ccb.GetCurrentState();
}


/*============================================================
MEMBER FUNCTION:
    CParentHeartBeatRespTrans::Action

DESCRIPTION:
    ������Ӧ������
    ������anchorBTS��servingBTS������
    servingBTS������������������Ӧ
    anchorBTS����������Ӧ������TTL

ARGUMENTS:
    ccb      :CSnoopCCB
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ������Ϣ������Ŀ��״̬.

SIDE EFFECTS:
    none
==============================================================*/
FSMStateIndex CParentHeartBeatRespTrans::Action(CSnoopCCB &ccb, CMessage &msg)
{
    CTunnelHeartBeatResp msgTunnelHeartBeat(msg);
    UINT8 strMac[ M_MACADDR_STRLEN ];
    CMac mac(msgTunnelHeartBeat.GetMac());
    LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "Rx MAC(%s) Tunnel Heart Beat Response.", (int)mac.str( strMac ) );

    //anchor<-serving
    DATA_assert((true == ccb.GetIsAnchor()) && (true == ccb.IsExistTunnel()));
    //�յ��쳣��Ϣ,������,jiaying20100720
    if(!((true == ccb.GetIsAnchor()) && (true == ccb.IsExistTunnel())))//������Ϣ,��Ӧ��
     {        
            return ccb.GetCurrentState();
     }
    UpdateFTEntry(ccb, ccb.GetIpType(), true);

    //���ı�CCB״̬
    return ccb.GetCurrentState();
}
