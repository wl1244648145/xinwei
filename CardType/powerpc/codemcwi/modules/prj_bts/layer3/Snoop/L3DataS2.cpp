/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopState.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   09/08/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include "LogArea.h"

#include "L3DataSnoopState.h"
#include "L3DataDhcp.h"
#include "L3DataSyncIL.h"
#include "L3DataTunnelEstablish.h"
#include "L3DataSnoopErrCode.h"

#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif



/*============================================================
MEMBER FUNCTION:
    CSnoopStateBase::ParseDhcpOpt

DESCRIPTION:
    解析DHCP选项的内容

ARGUMENTS:
    *pInOption:  待分析的DHCP选项的指针
    *pOutOption: 分析结果，包含关注的DHCP选项

RETURN VALUE:
    bool:        true,正确解析； false,DHCP选项有错误
    *pOutOption: 返回了主机序的DHCP选项值

SIDE EFFECTS:
    none
==============================================================*/
bool CSnoopStateBase::ParseDhcpOpt(UINT8 *pInOption, const SINT16 InOptLen, DhcpOption *pOutOption)
{
    if ( ( NULL == pInOption ) || ( NULL == pOutOption ) || ( InOptLen <= 0 ) )
        {
        LOG3( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PARAMETER ), 
              "Input Err. InOption:0X%X, OutOption:0X%X, Option Length:%d", 
              (int)pInOption, (int)pOutOption, (int)InOptLen );
        return false;
        }

    UINT8 *pOption = pInOption;
    UINT32 ulMagicNo = *( (UINT32*)pOption );
    UINT8 aucDhcpMagic[ M_DHCP_OPTION_LEN_MAGIC ] = RFC1048_MAGIC;

    if ( *(UINT32*)aucDhcpMagic != ulMagicNo )
        {
        LOG1( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PACKET_ERR ), 
                 "Err DHCP option magic cookie :0X%X", ulMagicNo );
        return false;
        }
    pOption += M_DHCP_OPTION_LEN_MAGIC;

    UINT16 usParsedOptLen = 0;  /*Id域*/
    UINT8 ucOptId   = *pOption++;
    ++usParsedOptLen;
    UINT8 ucOptLen  = 0;

    while ( M_DHCP_OPTION_END != ucOptId )
        {
        ucOptLen = *pOption++;
        usParsedOptLen += ucOptLen + 1; /*Length, Value域*/
        if ( InOptLen < usParsedOptLen )
            {
            LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PACKET_ERR ), "Dhcp Options Err. Possible resean: no END flag" );
            return false;
            }
        switch ( ucOptId )
            {
            case M_DHCP_OPTION_REQUEST_IPADDR:
                if ( M_DHCP_OPTION_LEN_REQUESTIP == ucOptLen )
                    {
                    UINT32 ulReqIp = *( (UINT32*)pOption );
                    pOutOption->ulRequestIp = ntohl( ulReqIp );
                    }
                pOption += ucOptLen;
                break;

            case M_DHCP_OPTION_LEASE_TIME:
                if ( M_DHCP_OPTION_LEN_LEASETIME == ucOptLen )
                    {
                    UINT32 ulLeaseTime = *( (UINT32*)pOption );
                    pOutOption->ulIpLeaseTime = ntohl( ulLeaseTime ) * 110 /100;//BTS定时器不准确，经常在dhcp客户端地址到期之前把转发表删除了,所以适当增加定时器长度
                    }
                pOption += ucOptLen;
                break;

            case M_DHCP_OPTION_MSGTYPE:
                if ( M_DHCP_OPTION_LEN_MESSAGETYPE == ucOptLen )
                    {
                    pOutOption->ucMessageType = *pOption;
                    }
                pOption += ucOptLen;
                break;

            case M_DHCP_OPTION_SERVER_ID:
                if ( M_DHCP_OPTION_LEN_SERVERID == ucOptLen )
                    {
                    UINT32 ulSrvId = *( (UINT32*)pOption );
                    pOutOption->ulServerId = ntohl( ulSrvId );
                    }
                pOption += ucOptLen;
                break;

            default:
                pOption += ucOptLen;
                break;
            }

        ucOptId = *pOption++;
        ++usParsedOptLen;
        }
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CSnoopStateBase::ParseDhcpPacket

DESCRIPTION:
    状态机分析输入的DHCP包类型

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    bool: 分析结果

SIDE EFFECTS:
    none
==============================================================*/
bool  CSnoopStateBase::ParseDhcpPacket(const CMessage &msg, DhcpOption &OutOption)
{
    void *pTraffic = msg.GetDataPtr();
    if ( NULL == pTraffic )
        {
        //异常
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), "Snoop message without payload." );
        return false;
        }

    DATA_assert ( IPTYPE_DHCP == msg.GetIpType() );
    //DHCP Packet.
    UINT8 *pDhcp = (UINT8*)( msg.GetDhcpPtr() );
    if ( NULL == pDhcp )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), "not dhcp packet." );
        return false;
        }

    //计算DHCP Option的长度;
    //定位到Udp头
    UdpHdr *pUdp = (UdpHdr*)msg.GetUdpPtr();
    //选项长度 = 
    SINT16 InOptLen = pUdp->usLen - sizeof( UdpHdr ) - sizeof( DhcpHdr );
    if ( true != ParseDhcpOpt( (UINT8*)pDhcp + sizeof( DhcpHdr ), InOptLen, &OutOption ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PACKET_ERR ), "DHCP Option parsed err." );
        return false;
        }

    //返回包类型//DHCP Discover?Offer?Request?在OutOption中
    return true;
};



/*============================================================
MEMBER FUNCTION:
    CSnoopStateBase::ParsePPPoEPacket

DESCRIPTION:
    状态机分析输入的PPPoE包类型

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    UINT8: 返回的包类型.

SIDE EFFECTS:
    none
==============================================================*/
UINT8  CSnoopStateBase::ParsePPPoEPacket(const CMessage &msg)
{
	EtherHdr* pEtherHeader = (EtherHdr*)( msg.GetDataPtr() );
    if ( NULL == pEtherHeader )
        {
        //异常
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), "Snoop message without payload." );
        return M_MSGTYPE_ERR;
        }

    DATA_assert( IPTYPE_PPPoE == msg.GetIpType() );
    //PPPoE Packet.
	PPPoEHdr *pPPPoE;
	if(IS_8023_PACKET(ntohs(pEtherHeader->usProto)))
	{
		pPPPoE = (PPPoEHdr*)( (UINT8*)pEtherHeader + sizeof(EtherHdr) + sizeof(LLCSNAP));
	}
	else
	{
		pPPPoE = (PPPoEHdr*)( (UINT8*)( pEtherHeader ) + sizeof( EtherHdr ) );
	}

    return pPPPoE->ucCode;
}



/*============================================================
MEMBER FUNCTION:
    CIdleState::DoParseEvent

DESCRIPTION:
    IDLE状态事件分析函数

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回的transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CIdleState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->IDLE state parser." );
    switch( msg.GetMessageId() )
        {
        case MSGID_TRAFFIC_SNOOP_REQ:
                {
                UINT8 ucIpType = msg.GetIpType();
                if ( IPTYPE_PPPoE == ucIpType )
                    {
                    UINT8 ucPktType = ParsePPPoEPacket( msg );
                    if ( M_PPPoE_PADI != ucPktType )
                        {
                        //不是PADI.
                        return INVALID_EVENT;
                        }

                    //renturn PADI transition。
                    return TRANS_SNOOP_IDLE_PADI;
                    }
                else if ( IPTYPE_DHCP == ucIpType )
                    {
                    DhcpOption OutOption;
                    memset( (void*)&OutOption, 0, sizeof( DhcpOption ) );
                    if ( true == ParseDhcpPacket( msg, OutOption ) )
                        {
                        if ( ( M_DHCP_DISCOVERY == OutOption.ucMessageType )
                            && ( 0 == OutOption.ulServerId ) )
                            {
                            //RFC2131
                            //renturn Discovery transition。
                            return TRANS_SNOOP_IDLE_DISC;
                            }
				else if( M_DHCP_REQUEST == OutOption.ucMessageType   )		//wangwenhua add 20081016
				{
				     return TRANS_SNOOP_IDLE_REQUEST;
				}
				else
				{
				     LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->IDLE state parser err:%d." ,OutOption.ucMessageType);
				}
                        }

                    //解析错误，不是Discovery.或者不正确的Discovery(Option不正确)
                    return INVALID_EVENT;
                    }
                else
                    {
                    //what packet?
                    LOG2( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), 
                        "Receive unexpected packet[IpType:%d] under %s state.",
                        ucIpType, (int)GetStateName() );
                    return INVALID_EVENT;
                    }
                }
            break;

        case MSGID_TUNNEL_ESTABLISH_REQ:
                {
                //Tunnel Establish.
                CTunnelEstablish msgTunnelEstablish( msg );
                if ( IPTYPE_FIXIP == msgTunnelEstablish.GetIpType() )
                    {
                    //固定IP的情况
                    return TRANS_SNOOP_IDLE_FIXIP_TUNNEL_ESTABLISH_REQ;
                    }
                else
                    {
                    //IDLE状态不处理
                    return INVALID_EVENT;
                    }
                }
            break;

        case MSGID_TUNNEL_TERMINATE_REQ:
                //Tunnel Terminate Request.
                return TRANS_SNOOP_IDLE_TUNNEL_TERMINATE_REQ;
            break;

        case MSGID_TUNNEL_CHANGE_ANCHOR_REQ:
                //Tunnel Change Anchor.
                return TRANS_SNOOP_IDLE_TUNNEL_CHANGE_ANCHOR_REQ;
            break;

        case MSGID_TUNNEL_SYNC_REQ:
                //IDLE状态收到的Tunnel Sync Req.
                return TRANS_SNOOP_IDLE_TUNNEL_SYNC_REQ;
            break;

        case MSGID_IPLIST_ADD_FIXIP:
                //ADD Fix IP.
                return TRANS_SNOOP_IDLE_ADD_FIXIP;
            break;

        default:
            //Idle状态不处理
            LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_UNEXPECTED_MSGID ), "Exit...IDLE state has no corresponding transition(%d).", msg.GetMessageId() );
            return INVALID_EVENT;
        }
}


/*============================================================
MEMBER FUNCTION:
    CSelectingState::DoParseEvent

DESCRIPTION:
    Selecting状态事件分析函数

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回的transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CSelectingState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->SELECTING state parser." );

    switch( msg.GetMessageId() )
        {
        case MSGID_TRAFFIC_SNOOP_REQ:
                {
                UINT8 ucIpType = msg.GetIpType();
                if ( IPTYPE_PPPoE == ucIpType )
                    {
                    switch( ParsePPPoEPacket( msg ) )
                        {
                        case M_PPPoE_PADI:
                            return TRANS_SNOOP_SELECTING_PADI;

                        case M_PPPoE_PADO:
                            return TRANS_SNOOP_SELECTING_PADO;

                        case M_PPPoE_PADR:
                            return TRANS_SNOOP_SELECTING_PADR;

                        default:
                            //不处理。
                            return INVALID_EVENT;
                        }
                    }
                else if ( IPTYPE_DHCP == ucIpType )
                    {
                    DhcpOption OutOption;
                    memset( (void*)&OutOption, 0, sizeof( DhcpOption ) );
                    if ( true == ParseDhcpPacket( msg, OutOption ) )
                        {
                        UINT8  ucMessageType = OutOption.ucMessageType;
                        UINT32 ulServerId = OutOption.ulServerId;
                        UINT32 ulRequestIp = OutOption.ulRequestIp;
                        UINT32 ulIpLeaseTime = OutOption.ulIpLeaseTime;
                        switch ( ucMessageType )
                            {
                            case M_DHCP_DISCOVERY:
                                if ( 0 == ulServerId )
                                    {
                                    return TRANS_SNOOP_SELECTING_DISC;
                                    }

                            case M_DHCP_OFFER:
                                if ( ( 0 == ulRequestIp )
                                    && ( 0 != ulIpLeaseTime )
                                    && ( 0 != ulServerId ) )
                                    {
                                    return TRANS_SNOOP_SELECTING_OFFER;
                                    }

                            case M_DHCP_REQUEST:
                                if ( ( 0 != ulRequestIp )
                                    && ( 0 != ulServerId ) )
                                    {
                                    return TRANS_SNOOP_SELECTING_REQ;
                                    }

                            default:
                                //不处理
                                return INVALID_EVENT;
                            }
                        }

                    //不处理
                    return INVALID_EVENT;
                    }
                else
                    {
                    //what packet?
                    LOG2( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), 
                          "Receive unexpected packet[IpType:%d] under %s state.", 
                          (int)ucIpType, (int)GetStateName() );
                    return INVALID_EVENT;
                    }
                }
            break;

        case MSGID_TIMER_SNOOP:
                //Selecting状态的定时器超时
                return TRANS_SNOOP_SELREQ_TIMEOUT;
            break;

        case MSGID_IPLIST_DELETE:
                //删除Selecting状态的CCB
                return TRANS_SNOOP_SELREQ_DELENTRY;
            break;

        default:
                //Selecting状态不处理
                LOG2( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_UNEXPECTED_MSGID ), 
                   "Exit...Receive unexpected message under %s state, msgId:%x", (int)GetStateName(), msg.GetMessageId());
                return INVALID_EVENT;
            break;
        }

    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CRequestingState::DoParseEvent

DESCRIPTION:
    Requesting状态事件分析函数

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回的transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CRequestingState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->REQUESTING state parser." );

    switch( msg.GetMessageId() )
        {
        case MSGID_TRAFFIC_SNOOP_REQ:
                {
                UINT8 ucIpType = msg.GetIpType();
                if ( IPTYPE_PPPoE == ucIpType )
                    {
                    switch( ParsePPPoEPacket( msg ) )
                        {
                        case M_PPPoE_PADI:
                            return TRANS_SNOOP_REQUESTING_PADI;

                        case M_PPPoE_PADR:
                            return TRANS_SNOOP_REQUESTING_PADR;

                        case M_PPPoE_PADS:
                            return TRANS_SNOOP_REQUESTING_PADS;

                        default:
                            //不处理。
                            return INVALID_EVENT;
                        }
                    }
                else if ( IPTYPE_DHCP == ucIpType )
                    {
                    DhcpOption OutOption;
                    memset( (void*)&OutOption, 0, sizeof( DhcpOption ) );
                    if ( true == ParseDhcpPacket( msg, OutOption ) )
                        {
                        UINT8  ucMessageType = OutOption.ucMessageType;
                        UINT32 ulServerId = OutOption.ulServerId;
                        UINT32 ulRequestIp = OutOption.ulRequestIp;
                        UINT32 ulIpLeaseTime = OutOption.ulIpLeaseTime;
                        switch ( ucMessageType )
                            {
                            case M_DHCP_DISCOVERY:
                                if ( 0 == ulServerId )
                                    {
                                    return TRANS_SNOOP_REQUESTING_DISC;
                                    }

                            case M_DHCP_REQUEST:
                                if ( ( 0 != ulRequestIp ) && ( 0 != ulServerId ) )
                                    {
                                    return TRANS_SNOOP_REQUESTING_REQ;
                                    }

                            case M_DHCP_NAK:
                                if ( ( 0 == ulRequestIp )
                                    && ( 0 == ulIpLeaseTime )
                                    /*&& ( 0 != ulServerId ) */)    /**/
                                    {
                                    return TRANS_SNOOP_REQUESTING_NAK;
                                    }

                            case M_DHCP_ACK:
                                if ( ( 0 == ulRequestIp )
                                    && ( 0 != ulServerId )
                                    && ( 0 != ulIpLeaseTime ) )
                                    {
                                    return TRANS_SNOOP_REQUESTING_ACK;
                                    }

                            default:
                                //不处理
                                return INVALID_EVENT;
                            }
                        }

                    //不处理
                    return INVALID_EVENT;
                    }
                else
                    {
                    //what packet?
                    LOG2( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_MSG_ERR ), 
                        "Receive unexpected packet[IpType:%d] under %s state.", 
                        ucIpType, (int)GetStateName() );
                    return INVALID_EVENT;
                    }
                }
            break;

        case MSGID_TIMER_SNOOP:
                //Requesting状态的定时器超时
                return TRANS_SNOOP_SELREQ_TIMEOUT;
            break;

        case MSGID_IPLIST_DELETE:
                //删除Requesting状态的CCB
                return TRANS_SNOOP_SELREQ_DELENTRY;
            break;

        default:
                //Requesting状态不处理
                LOG2( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_UNEXPECTED_MSGID ), 
                    "Receive unexpected message under %s state, msgId:%x.", (int)GetStateName(), msg.GetMessageId());
                return INVALID_EVENT;
            break;
        
        }

    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CSyncingState::DoParseEvent

DESCRIPTION:
    Syncing状态事件分析函数

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回的transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CSyncingState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->SYNCING state parser." );

    switch( msg.GetMessageId() )
        {
        case MSGID_TRAFFIC_SNOOP_REQ:
                {
                //Traffic.
                UINT8 ucIpType = msg.GetIpType();
                if ( ( IPTYPE_PPPoE == ucIpType ) 
                    && ( M_PPPoE_PADI == ParsePPPoEPacket( msg ) ) )
                    {
                    return TRANS_SNOOP_SYNCING_PADI;
                    }
                else if ( IPTYPE_DHCP == ucIpType )
                    {
                    DhcpOption OutOption;
                    memset( (void*)&OutOption, 0, sizeof( DhcpOption ) );
                    if ( true == ParseDhcpPacket( msg, OutOption ) )
                        {
                        if ( ( M_DHCP_DISCOVERY == OutOption.ucMessageType )
                                && ( 0 == OutOption.ulServerId ) )
                            {
                            return TRANS_SNOOP_SYNCING_DISC;
                            }
                        }
                    }
                }
            break;

        case MSGID_IPLIST_SYNC_RESP:
                {
                //同步应答消息
                CSyncILResp msgSyncILResp( msg );
                if ( true == msgSyncILResp.GetResult() )
                    {
                    //同步成功
                    switch( msgSyncILResp.GetIpType() )
                        {
                        case IPTYPE_DHCP:
                            //DHCP
                            return TRANS_SNOOP_SYNCING_DHCP_SYNC_SUCCESS;

                        case IPTYPE_PPPoE:
                            //PPPoE
                            return TRANS_SNOOP_SYNCING_PPPoE_SYNC_SUCCESS;

                        default:
                            //Syncing状态不处理
                            return INVALID_EVENT;
                        }
                    }
                else 
                    {
                    //同步失败
                    return TRANS_SNOOP_SYNCING_FAIL;
                    }
                }
            break;

        case MSGID_TIMER_SNOOP:
                //返回和同步失败相同的transition.
                return TRANS_SNOOP_SYNCING_FAIL;
            break;

        case MSGID_IPLIST_DELETE:
                //返回和同步失败相同的transition.
                return TRANS_SNOOP_SYNCING_DELENTRY;
            break;

        default:
            //Syncing状态不处理
            return INVALID_EVENT;
        }

    //Syncing状态不处理
    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CBoundState::DoParseEvent

DESCRIPTION:
    Bound状态事件分析函数

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回的transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CBoundState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->BOUND state parser." );
    switch ( msg.GetMessageId() )
        {
        case MSGID_TUNNEL_ESTABLISH_REQ:
            //Tunnel Establish.
            return TRANS_SNOOP_TUNNEL_ESTABLISH_REQ;

        case MSGID_TUNNEL_TERMINATE_REQ:
            //Tunnel Terminate.
            return TRANS_SNOOP_TUNNEL_TERMINATE_REQ;

        case MSGID_TUNNEL_CHANGE_ANCHOR_REQ:
            //Tunnel Change Anchor.
            return TRANS_SNOOP_TUNNEL_CHANGE_ANCHOR_REQ;

        case MSGID_TRAFFIC_SNOOP_REQ:
            {
            //Traffic.
            UINT8 ucIpType = msg.GetIpType();
            if ( IPTYPE_PPPoE == ucIpType )
                {
                switch( ParsePPPoEPacket( msg ) )
                    {
                    case M_PPPoE_PADI:
                        //PADI.
                        return TRANS_SNOOP_PADI;

                    case M_PPPoE_PADR:
                        //PADR.
                        return TRANS_SNOOP_BOUND_PADR;

                    case M_PPPoE_PADS:
                        //PADS.
                        return TRANS_SNOOP_BOUND_PADS;

                    case M_PPPoE_PADT:
                        //PADT.
                        return TRANS_SNOOP_BOUND_PADT;

                    default:
                        //Bound状态不处理
                        return INVALID_EVENT;
                    }
                }
            else if ( IPTYPE_DHCP == ucIpType )
                {
                DhcpOption OutOption;
                memset( (void*)&OutOption, 0, sizeof( DhcpOption ) );
                if ( true == ParseDhcpPacket( msg, OutOption ) )
                    {
                    UINT8  ucMessageType = OutOption.ucMessageType;
                    UINT32 ulServerId = OutOption.ulServerId;
                    UINT32 ulRequestIp = OutOption.ulRequestIp;
                    UINT32 ulIpLeaseTime = OutOption.ulIpLeaseTime;
                    switch ( ucMessageType )
                        {
                        case M_DHCP_DISCOVERY:
                            if ( 0 == ulServerId )
                                {
                                //Discovery.
                                return TRANS_SNOOP_DISCOVERY;
                                }

                        case M_DHCP_REQUEST:
                            if ( ( 0 != ulRequestIp )/* && ( 0 != ulServerId ) */)
                                {
                                //Request from SELECTING.
                                //地址申请成功后，如果拔插网线，ulServerId = 0;
                                return TRANS_SNOOP_BOUND_REQ_SELECTING;
                                }
                            else if ( ( 0 == ulRequestIp ) && ( 0 == ulServerId ) )
                                {
                                //Request from RENEWING.
                                return TRANS_SNOOP_BOUND_REQ_RENEWING;
                                }

                        case M_DHCP_ACK:
                            if ( ( 0 == ulRequestIp )
                                && ( 0 != ulServerId )
                                && ( 0 != ulIpLeaseTime ) )
                                {
                                //ACK.
                                /*
                                 *BOUND收到init-reboot DHCPREQUEST的响应DHCPACK.
                                 *client会更新地址有效时间lease time.
                                 *所以BTS必须同步更新，否则BTS上的DHCP用户可能会先于PC释放地址
                                 */
                                return TRANS_SNOOP_RENEWING_ACK;
                                }

                        case M_DHCP_DECLINE:
                        case M_DHCP_RELEASE:
                            if ( ( 0 != ulServerId ) && ( 0 == ulIpLeaseTime ) )
                                {
                                //Release or Decline.
                                return TRANS_SNOOP_RELEASE_DECLINE;
                                }

                        default:
                            //Bound状态不处理
                            return INVALID_EVENT;
                        }
                    }
                }
            }
          break;
        case MSGID_FT_ENTRY_EXPIRE:
            //收到Ether Bridge通知的FT Entry超时消息
            return TRANS_SNOOP_ENTRY_EXPIRE;

        case MSGID_TIMER_SNOOP:
            //BOUND状态的定时器超时
            return TRANS_SNOOP_BOUND_TIMEOUT;

        case MSGID_IPLIST_DELETE:
                //删除BOUND状态的CCB
            return TRANS_SNOOP_RENBND_DELENTRY;

        default:
            //Bound状态不处理
            return INVALID_EVENT;
        }

    //Bound状态不处理
    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CRenewingState::DoParseEvent

DESCRIPTION:
    Renewing状态事件分析函数

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回的transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CRenewingState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->RENEWING state parser." );
    switch( msg.GetMessageId() )
        {
        case MSGID_TUNNEL_ESTABLISH_REQ:
            //Tunnel Establish.
            return TRANS_SNOOP_TUNNEL_ESTABLISH_REQ;

        case MSGID_TUNNEL_TERMINATE_REQ:
            //Tunnel Terminate.
            return TRANS_SNOOP_TUNNEL_TERMINATE_REQ;

        case MSGID_TUNNEL_CHANGE_ANCHOR_REQ:
            //Tunnel Change Anchor.
            return TRANS_SNOOP_TUNNEL_CHANGE_ANCHOR_REQ;

        case MSGID_TIMER_SNOOP:
            //RENEWING状态的定时器超时
            //返回BOUND状态的定时器超时的transition.
            return TRANS_SNOOP_BOUND_TIMEOUT;

        case MSGID_IPLIST_DELETE:
            //删除RENEWING状态的CCB
            return TRANS_SNOOP_RENBND_DELENTRY;

        case MSGID_TRAFFIC_SNOOP_REQ:
            {
            //Traffic.
            if ( IPTYPE_DHCP == msg.GetIpType() )
                {
                DhcpOption OutOption;
                memset( (void*)&OutOption, 0, sizeof( DhcpOption ) );
                if ( true == ParseDhcpPacket( msg, OutOption ) )
                    {
                    UINT8  ucMessageType = OutOption.ucMessageType;
                    UINT32 ulServerId = OutOption.ulServerId;
                    UINT32 ulRequestIp = OutOption.ulRequestIp;
                    UINT32 ulIpLeaseTime = OutOption.ulIpLeaseTime;
                    switch ( ucMessageType )
                        {
                        case M_DHCP_DISCOVERY:
                            if ( 0 == ulServerId )
                                {
                                //Discovery.
                                return TRANS_SNOOP_DISCOVERY;
                                }

                        case M_DHCP_REQUEST:
                            if ( ( 0 == ulRequestIp ) && ( 0 == ulServerId ) )
                                {
                                //Request when renew.
                                return TRANS_SNOOP_RENEWING_REQ;
                                }

                        case M_DHCP_ACK:
                            if ( ( 0 == ulRequestIp )
                                && ( 0 != ulServerId )
                                && ( 0 != ulIpLeaseTime ) )
                                {
                                //ACK.
                                return TRANS_SNOOP_RENEWING_ACK;
                                }

                        case M_DHCP_NAK:
                            if ( ( 0 == ulRequestIp )
                                && ( 0 == ulIpLeaseTime )
                                /*&& ( 0 != ulServerId ) */)
                                {
                                //NAK
                                return TRANS_SNOOP_RENEWING_NAK;
                                }

                        case M_DHCP_RELEASE:
                            if ( ( 0 != ulServerId ) && ( 0 == ulIpLeaseTime ) )
                                {
                                //Release
                                return TRANS_SNOOP_RELEASE_DECLINE;
                                }

                        default:
                            //Renewing状态不处理
                            return INVALID_EVENT;
                        }
                    }
                }
            }
        
        default:
            //Renewing状态不处理
            return INVALID_EVENT;
        }

    //Renewing状态不处理
    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CRoamingState::DoParseEvent

DESCRIPTION:
    Roaming状态事件分析函数

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回的transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CRoamingState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->ROAMING state parser." );
    switch( msg.GetMessageId() )
        {
        case MSGID_TUNNEL_ESTABLISH_RESP:
            //Tunnel Establish.
            return TRANS_SNOOP_ROAMING_TUNNEL_ESTABLISH_RESP;

        case MSGID_FT_ENTRY_EXPIRE:
            //收到Ether Bridge通知的FT Entry超时消息
            return TRANS_SNOOP_ENTRY_EXPIRE;

        case MSGID_TIMER_SNOOP:
            //ROAMING状态定时器超时
            return TRANS_SNOOP_ROAMING_TIMEOUT;

        case MSGID_IPLIST_DELETE:
            //删除ROAMING状态的CCB
            return TRANS_SNOOP_ROAMING_DELENTRY;

        case MSGID_TRAFFIC_SNOOP_REQ:
            {
            //Traffic.
            UINT8 ucIpType = msg.GetIpType();

            if ( IPTYPE_PPPoE == ucIpType )
                {
                switch( ParsePPPoEPacket( msg ) )
                    {
                    case M_PPPoE_PADI:
                        //PADI.
                        return TRANS_SNOOP_PADI;

                    case M_PPPoE_PADT:
                        //PADT.
                        return TRANS_SNOOP_BOUND_PADT;

                    default:
                        //不处理
                        return INVALID_EVENT;
                    }
                }
            else if ( IPTYPE_DHCP == ucIpType )
                {
                DhcpOption OutOption;
                memset( (void*)&OutOption, 0, sizeof( DhcpOption ) );
                if ( true == ParseDhcpPacket( msg, OutOption ) )
                    {
                    switch ( OutOption.ucMessageType )
                        {
                        case M_DHCP_DISCOVERY:
                            if ( 0 == OutOption.ulServerId )
                                {
                                //Discovery.
                                return TRANS_SNOOP_DISCOVERY;
                                }

                        case M_DHCP_RELEASE:
                            if ( ( 0 != OutOption.ulServerId ) && ( 0 == OutOption.ulIpLeaseTime ) )
                                {
                                //Release
                                return TRANS_SNOOP_RELEASE_DECLINE;
                                }

                        default:
                            //不处理
                            return INVALID_EVENT;
                        }
                    }
                }
            }

        default:
            //Roaming状态不处理
            return INVALID_EVENT;
        }

    //Roaming状态不处理
    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CParentState::DoParseEvent

DESCRIPTION:
    所有状态都必须处理的事件分析函数

ARGUMENTS:
    CMessage :输入消息

RETURN VALUE:
    FSMTransIndex: 返回的transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CParentState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->PARENT state parser." );
    switch ( msg.GetMessageId() )
        {
        case MSGID_ROAM_REQ:
            //收到Roam Request消息
            return TRANS_SNOOP_PARENT_ROAM_REQ;

        case MSGID_TIMER_HEARTBEAT:
            return TRANS_SNOOP_PARENT_HEARTBEAT_TIMER;

        case MSGID_TUNNEL_HEARTBEAT:
            return TRANS_SNOOP_PARENT_HEARTBEAT;

        case MSGID_TUNNEL_HEARTBEAT_RESP:
            return TRANS_SNOOP_PARENT_HEARTBEAT_RESP;

        case MSGID_TUNNEL_SYNC_REQ:
            //收到Tunnel Sync消息
            return TRANS_SNOOP_PARENT_TUNNEL_SYNC_REQ;

        case MSGID_TUNNEL_ESTABLISH_REQ:
            //收到Tunnel Establish Request
            return TRANS_SNOOP_PARENT_TUNNEL_ESTABLISH_REQ;

        case MSGID_TUNNEL_TERMINATE_REQ:
            //收到Tunnel Terminiate Request
            return TRANS_SNOOP_PARENT_TUNNEL_TERMINATE_REQ;

        case MSGID_TUNNEL_CHANGE_ANCHOR_RESP:
            //收到Tunnel ChangeAnchor Response
            return TRANS_SNOOP_PARENT_TUNNEL_CHGANCHOR_RESP;

        case MSGID_IPLIST_ADD_FIXIP:
            //IDLE状态之外收到FIXIP配置消息
            return TRANS_SNOOP_PARENT_ADD_FIXIP;

        default:
            //所有的状态都不处理
            return INVALID_EVENT;
        }
}
