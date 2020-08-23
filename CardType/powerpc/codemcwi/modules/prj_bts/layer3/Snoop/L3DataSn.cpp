/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoop.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   03/30/06   xiao weifang  NVRAM恢复用户信息. 
 *   09/02/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

//禁用Winsock 1
#define _WINSOCKAPI_
#include <taskLib.h>
#include "Object.h"
#include "biztask.h"
#include "MsgQueue.h"
#include "Message.h"
#include "LogArea.h"
#include "ErrorCodeDef.h"

#include "L3DataSnoop.h"
#include "L3DataMsgId.h"
#include "L3OAMMessageId.h"
#include "L3DataSnoopTimer.h"
#include "L3DataSyncIL.h"
#include "L3DataRoam.h"
#include "L3DataTunnelTerminate.h"
#include "L3DataTunnelChangeAnchor.h"
#include "L3DataTunnelEstablish.h"
#include "L3DataTunnelSync.h"
#include "L3DataFTEntryExpire.h"
#include "L3DataSnoopTrans.h"
#include "L3DataDelIL.h"
#include "L3DataFixIp.h"
#include "L3DataSnoopErrCode.h"

//任务实例指针的初始化
CTSnoop* CTSnoop::s_ptaskSnoop = NULL;


/*============================================================
MEMBER FUNCTION:
    GetRouterAreaId

DESCRIPTION:
    全局函数，对外提供的函数接口

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
UINT32 GetRouterAreaId()
{
    //通过任务实例获取
    return CTSnoop::GetInstance()->GetRouterAreaId();
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::ProcessMessage

DESCRIPTION:
    CTSnoop任务消息处理函数

ARGUMENTS:
    msg: 消息

RETURN VALUE:
    bool:true or false,FrameWork根据返回值决定是否做PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
bool CTSnoop::ProcessMessage(CMessage &msg)
{
    UINT16 usMsgId = msg.GetMessageId();
    switch ( usMsgId )
        {
        case MSGID_TRAFFIC_SNOOP_REQ:
                //Traffic.
                IncreaseMeasureByOne( IN_TRAFFIC );
                m_FSM.InjectMsg( msg );
            break;

        case MSGID_TUNNEL_HEARTBEAT:
                {
                CTunnelHeartBeat msgTunnelHeartBeat( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTunnelHeartBeat.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_TUNNEL_HEARTBEAT_RESP:
                {
                CTunnelHeartBeatResp msgTunnelHeartBeatResp( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTunnelHeartBeatResp.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        /***************************************************
         *不同的消息，取Mac的方式不一样( BPtree 索引的键值 )
         *所以进入状态机前都统一一次
         ***************************************************/
        case MSGID_IPLIST_SYNC_RESP:
                {
                //性能计数值+1
                IncreaseMeasureByOne( IN_DAIB_SYNC_RESP );
                CSyncILResp msgSyncIlResp( msg );
                //设置KeyMac
                msg.SetKeyMac( msgSyncIlResp.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_ROAM_REQ:
                {
                //性能计数值+1
                IncreaseMeasureByOne( IN_ROAM_REQ );
                CRoam msgRoam( msg );
                //设置KeyMac
                msg.SetKeyMac( msgRoam.GetEntry().aucMAC );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_TUNNEL_ESTABLISH_REQ:
                {
                //性能计数值+1
                IncreaseMeasureByOne( IN_TUNNEL_ESTABLISH_REQ );
                CTunnelEstablish msgTunnelEstablish( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTunnelEstablish.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_TUNNEL_ESTABLISH_RESP:
                {
                //性能计数值+1
                IncreaseMeasureByOne( IN_TUNNEL_ESTABLISH_RESP );
                CTunnelEstablishResp msgTunnelEstablishResp( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTunnelEstablishResp.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_TUNNEL_TERMINATE_REQ:
                {
                //性能计数值+1
                IncreaseMeasureByOne( IN_TUNNEL_TERMINATE_REQ );
                CTunnelTerminate msgTunnelTerminate( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTunnelTerminate.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_TUNNEL_TERMINATE_RESP:
                //性能计数值+1
                IncreaseMeasureByOne( IN_TUNNEL_TERMINATE_RESP );
                //此消息不做状态机处理
            break;

        case MSGID_TUNNEL_SYNC_REQ:
                {
                //性能计数值+1
                IncreaseMeasureByOne( IN_TUNNEL_SYNC_REQ );
                CTunnelSync msgTunnelSync( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTunnelSync.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_TUNNEL_SYNC_RESP:
                //性能计数值+1
                IncreaseMeasureByOne( IN_TUNNEL_SYNC_RESP );
                //此消息不做状态机处理
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_REQ:
                {
                //性能计数值+1
                IncreaseMeasureByOne( IN_TUNNEL_CHANGE_ANCHOR_REQ );
                CTunnelChangeAnchor msgTunnelChangeAnchor( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTunnelChangeAnchor.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_RESP:
                {
                //性能计数值+1
                IncreaseMeasureByOne( IN_TUNNEL_CHANGE_ANCHOR_RESP );
                CTunnelChangeAnchorResp msgTunnelChangeAnchorResp( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTunnelChangeAnchorResp.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_TIMER_HEARTBEAT:
        case MSGID_TIMER_SNOOP:
                {
                CSnoopTimerExpire msgTimer( msg );
                //设置KeyMac
                msg.SetKeyMac( msgTimer.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_IPLIST_ADD_FIXIP:
                {
                ////配置固定IP;
                CADDFixIP msgAddFixIp( msg );
                //设置KeyMac
                msg.SetKeyMac( msgAddFixIp.GetEntry().aucMAC );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_IPLIST_DELETE:
                {
                //删除IPLIST
                ////
                CDelILEntry msgDelIL( msg );
                //设置KeyMac
                msg.SetKeyMac( msgDelIL.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case MSGID_FT_ENTRY_EXPIRE:
                {
                CFTEntryExpire msgFTEntryExpire( msg ); 
                //设置KeyMac
                msg.SetKeyMac( msgFTEntryExpire.GetMac() );
                //状态机处理
                m_FSM.InjectMsg( msg );
                }
            break;

        case M_CFG_SNOOP_DATA_SERVICE_CFG_REQ:
                //Router Area Id配置消息
                SnoopConfig( msg );
            break;

        case MSGID_FT_CHECK_VLAN:
                //EID对应的group修改
                SnoopCheckGroup( msg );
            break;

        case MSGID_IPLIST_DELETE_BY_CPE:
                clearCPEData(msg.GetEID());
            break;

        default :
                LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_UNEXPECTED_MSGID ), "Snoop receive unexpected message[Id: 0x%x].", usMsgId );
            break;
        }

    //destroy message.
    //pComMsg->Destroy();

#ifdef __WIN32_SIM__
    showStatus();
#endif
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::CTSnoop

DESCRIPTION:
    CTSnoop构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTSnoop::CTSnoop()
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "CTSnoop::CTSnoop()" );

#ifndef NDEBUG
    if ( !Construct( CObject::M_OID_SNOOP ) )
        {
        LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "ERROR!!!CTSnoop::CTSnoop()% Construct failed." );
        }
#endif
    memset( m_szName, 0, sizeof( m_szName ) );
    memcpy( m_szName, M_TASK_SNOOP_TASKNAME, strlen( M_TASK_SNOOP_TASKNAME ) );
    m_uPriority         = M_TP_L3SNOOP;
    m_uOptions          = M_TASK_SNOOP_OPTION;
    m_uStackSize        = M_TASK_SNOOP_STACKSIZE + 10240;

    m_iMsgQMax          = M_TASK_SNOOP_MAXMSG;
    m_iMsgQOption       = M_TASK_SNOOP_MSGOPTION;

    m_ulRouterAreaId    = M_RAID_INVALID;
    m_bAgentCircuitId   = false;
    m_bRemoteCircuitId  = false;
    m_bPPPoERemoteID    = false;	

    //初始化性能统计值
    ClearMeasure();
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::~CTSnoop

DESCRIPTION:
    CTSnoop析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTSnoop::~CTSnoop()
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "CTSnoop::~CTSnoop" );

#ifndef NDEBUG
    if ( !Destruct( CObject::M_OID_SNOOP ) )
        {
        LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "ERROR!!!CTSnoop::~CTSnoop failed." );
        }
#endif
}


#ifndef _NO_NVRAM_RECOVER_
/*============================================================
MEMBER FUNCTION:
    CTSnoop::MainLoop

DESCRIPTION:
    main function.

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::MainLoop()
{
    CSnoopFSM::s_pNVRamCCBTable->init();
    m_FSM.RecoverUserInfoFromNVRAM();
    CBizTask::MainLoop();
}
#endif

/*============================================================
MEMBER FUNCTION:
    CTSnoop::GetInstance

DESCRIPTION:
    Get CTSnoop Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTSnoop* 

SIDE EFFECTS:
    none
==============================================================*/
CTSnoop* CTSnoop::GetInstance()
{
    if ( NULL == s_ptaskSnoop )
        {
        s_ptaskSnoop = new CTSnoop;
        }
    return s_ptaskSnoop;
}


#ifdef UNITEST
void CTSnoop::DeleteInstance()
{
    if ( NULL != s_ptaskSnoop )
        {
        delete s_ptaskSnoop;
        s_ptaskSnoop = NULL;
        }
    return;
}
#endif


/*============================================================
MEMBER FUNCTION:
    CTSnoop::showStatus

DESCRIPTION:
    打印Snoop任务各属性值

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::showStatus()
{
#ifdef __WIN32_SIM__
    //等showStatus信号量
    WAIT();
#endif
    printf( "\r\n******************************" );
    printf( "\r\n*  Snoop Task Attributes     *" );
    printf( "\r\n******************************" );
    printf( "\r\nTask   stack   size           : %d", M_TASK_SNOOP_STACKSIZE );
    printf( "\r\nTask   Max  messages          : %d", M_TASK_SNOOP_MAXMSG );
    printf( "\r\n" );
    //Router Area ID.
    printf( "\r\nRouter Area ID                : 0x%.8X", m_ulRouterAreaId );
    printf( "\r\nRelay Agent Information Option: %s", ( m_bAgentCircuitId || m_bRemoteCircuitId )?"true":"false" );
    printf( "\r\n  Agent Circuit ID Sub-option : %s", m_bAgentCircuitId ?"true":"false" );
    printf( "\r\n  Agent Remote  ID Sub-option : %s", m_bRemoteCircuitId?"true":"false" );
    printf( "\r\n  PPPoE Remote  ID Sub-option : %s", m_bPPPoERemoteID?"true":"false" );

    printf( "\r\n" );

#ifdef __WIN32_SIM__
    //释放showStatus信号量
    RELEASE();
#endif
    return ;
}



/*============================================================
MEMBER FUNCTION:
    CTSnoop::showCCBinMem

DESCRIPTION:
    打印内存中CCB信息

ARGUMENTS:
    ulEid

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::showCCBinMem(UINT32 ulEid)
{
    //打印状态机相关的属性
    m_FSM.showStatus( ulEid );
    return ;
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::showCCBinNVRAM

DESCRIPTION:
    打印NVRAM中CCB信息

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::showCCBinNVRAM(UINT32 ulEid)
{
    //打印状态机相关的属性
#ifndef _NO_NVRAM_RECOVER_
    CSnoopFSM::s_pNVRamCCBTable->showStatus( ulEid );
#endif
    return ;
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::showPerf

DESCRIPTION:
    打印Snoop任务性能统计值

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::showPerf()
{
    printf( "\r\n" );
    printf( "\r\n***************************************" );
    printf( "\r\n*Performance Measurement Status       *" );
    printf( "\r\n***************************************" );
    for ( UINT8 type = IN_TRAFFIC; type < IN_TYPE_MAX; ++type )
        {
        printf( "\r\n%-20s: %d", strInType[ type ], GetMeasure( (IN_TYPE)type ) );
        }

    //打印Snoop任务发出消息的统计值
    CSnoopTrans::PrintMeaure();
    printf( "\r\n" );

    return ;
}

UINT32 CTSnoop::getIpByMac(UINT8* mac)
{
	CMac MAC(mac);
	return m_FSM.get_ip_by_mac(MAC);
	
}
/*============================================================
MEMBER FUNCTION:
    CTSnoop::showCCBbyMac

DESCRIPTION:
    打印CCB信息by mac

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::showCCBbyMac(CMac &Mac)
{
	//打印状态机相关的属性
	m_FSM.showStatus( Mac );
	return ;
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::SnoopConfig

DESCRIPTION:
    Router Area Id配置消息处理

ARGUMENTS:
    msgRaidConfig: 配置消息

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTSnoop::SnoopConfig(const CRAIDConfig& msgRaidConfig)
{
    UINT32  ulRaid = msgRaidConfig.GetRAID();
    UINT16  usResult = ERR_SUCCESS;
    if ( M_RAID_INVALID == ulRaid )
        {
        //Response Fail
        LOG( LOG_SEVERE, LOGNO( SNOOP, EC_SNOOP_CFG_ERR ), "Router area ID Configure fail." );
        usResult = ERR_FAIL;
        }
    else
        {
        m_ulRouterAreaId   = ulRaid;
        m_bAgentCircuitId  = ( 0 != msgRaidConfig.GetAgentCircuitIDSubOption() )?true:false;
        m_bRemoteCircuitId = ( 0 != msgRaidConfig.GetAgentRemoteIDSubOption () )?true:false;
        m_bPPPoERemoteID = ( 0 != msgRaidConfig.GetPPPoERemoteIDSubOption () )?true:false;
        }

#ifndef _NO_NVRAM_RECOVER_
    if ( m_ulRouterAreaId != CSnoopFSM::s_pNVRamCCBTable->GetLastRAID() )
        {
        //第一次启动或启动前后配置的RouterAreaId不一致，NVRam的用户不恢复
        CSnoopFSM::s_pNVRamCCBTable->clearCCB();
        CSnoopFSM::s_pNVRamCCBTable->SetCurrRAID( m_ulRouterAreaId );
        }
#endif

    //应答
    CRAIDConfigResp msgRaidConfigResp;
    if ( false == msgRaidConfigResp.CreateMessage( *this ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Create SNOOP configure response message failed." );
        return false;
        }
    msgRaidConfigResp.SetTransactionId( msgRaidConfig.GetTransactionId() );
    msgRaidConfigResp.SetResult( usResult );
    msgRaidConfigResp.SetDstTid( msgRaidConfig.GetSrcTid() );
    if ( false == msgRaidConfigResp.Post() )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_SYS_FAIL ), "Send RAID configure Message failed." );
        msgRaidConfigResp.DeleteMessage();
        return false;
        }
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::SnoopCheckGroup

DESCRIPTION:
    CPE对应的group改变,snoop中对应ccb及nvram中的ccb都跟着修改

ARGUMENTS:
    msgGroupModify

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTSnoop::SnoopCheckGroup(const CFTCheckVLAN& msgGroupModify)
{
    UINT32 ulEID    = msgGroupModify.GetEID();
    UINT16 usGid    = msgGroupModify.GetVlanID();

    m_FSM.GetCCBTable()->groupModify(ulEID, usGid);
#ifndef _NO_NVRAM_RECOVER_
    CSnoopFSM::s_pNVRamCCBTable->groupModify(ulEID, usGid);
#endif
}



/*============================================================
MEMBER FUNCTION:
    CTSnoop::clearCPEData

DESCRIPTION:
    cpe注册后如果DAIB为空,则通知snoop检查是否存在该cpe的数据
    有则删除。

ARGUMENTS:
    
RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTSnoop::clearCPEData(UINT32 eid)
{
    m_FSM.GetCCBTable()->clearCPEData(eid);
	return true;
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::isLeaseUpdated

DESCRIPTION:
    现场问题:
    DHCP获取地址,CPE断电,PC拔插到另一个CPE并获取地址
    但是旧CPE开电,CPE保留的DAIB恢复成BTS信息后导致错误
    所以在恢复DAIB前看是否DHCP地址重新续约过,
    如果是,而且ccb保留的eid跟当前eid不一致。则不恢复DAIB.

ARGUMENTS:
    eid: current EID;
    lease time: from DAIB;
    macAddr:
    
RETURN VALUE:
    bool true:DHCP续约过,而且PC更换过eid.不恢复

SIDE EFFECTS:
    none
==============================================================*/
bool CTSnoop::isLeaseUpdated(const UINT32 eid, const UINT32 leasetime, const CMac &macAddr)
{
    return m_FSM.GetCCBTable()->isLeaseUpdated(eid, leasetime, macAddr);
}



/*============================================================
MEMBER FUNCTION:
    CTSnoop::GetPerfData

DESCRIPTION:
    取性能数据的接口函数

ARGUMENTS:
    UINT8*

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::GetPerfData(UINT8 *pData)
{
    memcpy( pData, m_aulInMeasure,   sizeof( m_aulInMeasure ) );
    CSnoopTrans::GetPerfData( pData + sizeof( m_aulInMeasure ) );
}


/*============================================================
MEMBER FUNCTION:
    CTSnoop::ClearMeasure

DESCRIPTION:
    性能数据清0

ARGUMENTS:
    UINT8*

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTSnoop::ClearMeasure() 
{
    memset( m_aulInMeasure, 0, sizeof( m_aulInMeasure ) ); 
    CSnoopTrans::ClearMeasure();
}



/*============================================================
MEMBER FUNCTION:
    SNShow

DESCRIPTION:
    用于Tornado Shell上调用执行

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
extern "C" int getSTDNum(int*);
extern "C" void SNShow(UINT8 ucType)
{
    CTSnoop *taskSN = CTSnoop::GetInstance();
    switch ( ucType )
        {
        case 0:
            printf("\r\n SNShow(1): Show basic information");
            printf("\r\n SNShow(2): Show CCBs in memory");
            printf("\r\n SNShow(3): Show CCBs in NVRAM");
            printf("\r\n SNShow(4): Show performance information");
            printf("\r\n SNShow(5/6):5->set PPPoERemote=False;6->set True");
            printf("\r\n");
            break;

        case 1:
            printf("\r\nShow Snoop basic information");
            taskSN->showStatus();
            break;

        case 2:
            {
            printf("\r\nShow memory CCBs information");
            printf("\r\nPlease Enter EID(0:all):");
            UINT32 ulEid = 0;
            getSTDNum((int*)&ulEid);
            printf( "EID = %d = 0x%x", ulEid, ulEid );
            taskSN->showCCBinMem( ulEid );
            }
            break;

        case 3:
            {
            printf("\r\nShow NVRAM CCBs information");
            printf("\r\nPlease Enter EID(0:all):");
            UINT32 ulEid = 0;
            getSTDNum((int*)&ulEid);
            printf( "EID = %d = 0x%x", ulEid, ulEid );
            taskSN->showCCBinNVRAM( ulEid );
            }
            break;

        case 4:
            {
            printf("\r\nShow performance information");
            taskSN->showPerf();
            }
            break;
            
        case 5:
            {
           // printf("\r\nShow performance information");
            taskSN->SetPPPoERemoteIdEnable(false);
            }
            break;
        case 6:
            {
           // printf("\r\nShow performance information");
            taskSN->SetPPPoERemoteIdEnable(true);
            }
            break;
        default:
            break;
        }

    return;
}
UINT32 getDhcpIpByMac(UINT8 *mac)
{
	CTSnoop *taskSN = CTSnoop::GetInstance();
	return taskSN->getIpByMac(mac);

}
