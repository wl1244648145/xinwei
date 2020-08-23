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
    ����DHCPѡ�������

ARGUMENTS:
    *pInOption:  ��������DHCPѡ���ָ��
    *pOutOption: ���������������ע��DHCPѡ��

RETURN VALUE:
    bool:        true,��ȷ������ false,DHCPѡ���д���
    *pOutOption: �������������DHCPѡ��ֵ

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

    UINT16 usParsedOptLen = 0;  /*Id��*/
    UINT8 ucOptId   = *pOption++;
    ++usParsedOptLen;
    UINT8 ucOptLen  = 0;

    while ( M_DHCP_OPTION_END != ucOptId )
        {
        ucOptLen = *pOption++;
        usParsedOptLen += ucOptLen + 1; /*Length, Value��*/
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
                    pOutOption->ulIpLeaseTime = ntohl( ulLeaseTime ) * 110 /100;//BTS��ʱ����׼ȷ��������dhcp�ͻ��˵�ַ����֮ǰ��ת����ɾ����,�����ʵ����Ӷ�ʱ������
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
    ״̬�����������DHCP������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    bool: �������

SIDE EFFECTS:
    none
==============================================================*/
bool  CSnoopStateBase::ParseDhcpPacket(const CMessage &msg, DhcpOption &OutOption)
{
    void *pTraffic = msg.GetDataPtr();
    if ( NULL == pTraffic )
        {
        //�쳣
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

    //����DHCP Option�ĳ���;
    //��λ��Udpͷ
    UdpHdr *pUdp = (UdpHdr*)msg.GetUdpPtr();
    //ѡ��� = 
    SINT16 InOptLen = pUdp->usLen - sizeof( UdpHdr ) - sizeof( DhcpHdr );
    if ( true != ParseDhcpOpt( (UINT8*)pDhcp + sizeof( DhcpHdr ), InOptLen, &OutOption ) )
        {
        LOG( LOG_WARN, LOGNO( SNOOP, EC_SNOOP_PACKET_ERR ), "DHCP Option parsed err." );
        return false;
        }

    //���ذ�����//DHCP Discover?Offer?Request?��OutOption��
    return true;
};



/*============================================================
MEMBER FUNCTION:
    CSnoopStateBase::ParsePPPoEPacket

DESCRIPTION:
    ״̬�����������PPPoE������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    UINT8: ���صİ�����.

SIDE EFFECTS:
    none
==============================================================*/
UINT8  CSnoopStateBase::ParsePPPoEPacket(const CMessage &msg)
{
	EtherHdr* pEtherHeader = (EtherHdr*)( msg.GetDataPtr() );
    if ( NULL == pEtherHeader )
        {
        //�쳣
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
    IDLE״̬�¼���������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ���ص�transition.

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
                        //����PADI.
                        return INVALID_EVENT;
                        }

                    //renturn PADI transition��
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
                            //renturn Discovery transition��
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

                    //�������󣬲���Discovery.���߲���ȷ��Discovery(Option����ȷ)
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
                    //�̶�IP�����
                    return TRANS_SNOOP_IDLE_FIXIP_TUNNEL_ESTABLISH_REQ;
                    }
                else
                    {
                    //IDLE״̬������
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
                //IDLE״̬�յ���Tunnel Sync Req.
                return TRANS_SNOOP_IDLE_TUNNEL_SYNC_REQ;
            break;

        case MSGID_IPLIST_ADD_FIXIP:
                //ADD Fix IP.
                return TRANS_SNOOP_IDLE_ADD_FIXIP;
            break;

        default:
            //Idle״̬������
            LOG1( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_UNEXPECTED_MSGID ), "Exit...IDLE state has no corresponding transition(%d).", msg.GetMessageId() );
            return INVALID_EVENT;
        }
}


/*============================================================
MEMBER FUNCTION:
    CSelectingState::DoParseEvent

DESCRIPTION:
    Selecting״̬�¼���������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ���ص�transition.

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
                            //������
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
                                //������
                                return INVALID_EVENT;
                            }
                        }

                    //������
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
                //Selecting״̬�Ķ�ʱ����ʱ
                return TRANS_SNOOP_SELREQ_TIMEOUT;
            break;

        case MSGID_IPLIST_DELETE:
                //ɾ��Selecting״̬��CCB
                return TRANS_SNOOP_SELREQ_DELENTRY;
            break;

        default:
                //Selecting״̬������
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
    Requesting״̬�¼���������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ���ص�transition.

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
                            //������
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
                                //������
                                return INVALID_EVENT;
                            }
                        }

                    //������
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
                //Requesting״̬�Ķ�ʱ����ʱ
                return TRANS_SNOOP_SELREQ_TIMEOUT;
            break;

        case MSGID_IPLIST_DELETE:
                //ɾ��Requesting״̬��CCB
                return TRANS_SNOOP_SELREQ_DELENTRY;
            break;

        default:
                //Requesting״̬������
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
    Syncing״̬�¼���������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ���ص�transition.

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
                //ͬ��Ӧ����Ϣ
                CSyncILResp msgSyncILResp( msg );
                if ( true == msgSyncILResp.GetResult() )
                    {
                    //ͬ���ɹ�
                    switch( msgSyncILResp.GetIpType() )
                        {
                        case IPTYPE_DHCP:
                            //DHCP
                            return TRANS_SNOOP_SYNCING_DHCP_SYNC_SUCCESS;

                        case IPTYPE_PPPoE:
                            //PPPoE
                            return TRANS_SNOOP_SYNCING_PPPoE_SYNC_SUCCESS;

                        default:
                            //Syncing״̬������
                            return INVALID_EVENT;
                        }
                    }
                else 
                    {
                    //ͬ��ʧ��
                    return TRANS_SNOOP_SYNCING_FAIL;
                    }
                }
            break;

        case MSGID_TIMER_SNOOP:
                //���غ�ͬ��ʧ����ͬ��transition.
                return TRANS_SNOOP_SYNCING_FAIL;
            break;

        case MSGID_IPLIST_DELETE:
                //���غ�ͬ��ʧ����ͬ��transition.
                return TRANS_SNOOP_SYNCING_DELENTRY;
            break;

        default:
            //Syncing״̬������
            return INVALID_EVENT;
        }

    //Syncing״̬������
    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CBoundState::DoParseEvent

DESCRIPTION:
    Bound״̬�¼���������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ���ص�transition.

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
                        //Bound״̬������
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
                                //��ַ����ɹ�������β����ߣ�ulServerId = 0;
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
                                 *BOUND�յ�init-reboot DHCPREQUEST����ӦDHCPACK.
                                 *client����µ�ַ��Чʱ��lease time.
                                 *����BTS����ͬ�����£�����BTS�ϵ�DHCP�û����ܻ�����PC�ͷŵ�ַ
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
                            //Bound״̬������
                            return INVALID_EVENT;
                        }
                    }
                }
            }
          break;
        case MSGID_FT_ENTRY_EXPIRE:
            //�յ�Ether Bridge֪ͨ��FT Entry��ʱ��Ϣ
            return TRANS_SNOOP_ENTRY_EXPIRE;

        case MSGID_TIMER_SNOOP:
            //BOUND״̬�Ķ�ʱ����ʱ
            return TRANS_SNOOP_BOUND_TIMEOUT;

        case MSGID_IPLIST_DELETE:
                //ɾ��BOUND״̬��CCB
            return TRANS_SNOOP_RENBND_DELENTRY;

        default:
            //Bound״̬������
            return INVALID_EVENT;
        }

    //Bound״̬������
    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CRenewingState::DoParseEvent

DESCRIPTION:
    Renewing״̬�¼���������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ���ص�transition.

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
            //RENEWING״̬�Ķ�ʱ����ʱ
            //����BOUND״̬�Ķ�ʱ����ʱ��transition.
            return TRANS_SNOOP_BOUND_TIMEOUT;

        case MSGID_IPLIST_DELETE:
            //ɾ��RENEWING״̬��CCB
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
                            //Renewing״̬������
                            return INVALID_EVENT;
                        }
                    }
                }
            }
        
        default:
            //Renewing״̬������
            return INVALID_EVENT;
        }

    //Renewing״̬������
    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CRoamingState::DoParseEvent

DESCRIPTION:
    Roaming״̬�¼���������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ���ص�transition.

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
            //�յ�Ether Bridge֪ͨ��FT Entry��ʱ��Ϣ
            return TRANS_SNOOP_ENTRY_EXPIRE;

        case MSGID_TIMER_SNOOP:
            //ROAMING״̬��ʱ����ʱ
            return TRANS_SNOOP_ROAMING_TIMEOUT;

        case MSGID_IPLIST_DELETE:
            //ɾ��ROAMING״̬��CCB
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
                        //������
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
                            //������
                            return INVALID_EVENT;
                        }
                    }
                }
            }

        default:
            //Roaming״̬������
            return INVALID_EVENT;
        }

    //Roaming״̬������
    return INVALID_EVENT;
}


/*============================================================
MEMBER FUNCTION:
    CParentState::DoParseEvent

DESCRIPTION:
    ����״̬�����봦����¼���������

ARGUMENTS:
    CMessage :������Ϣ

RETURN VALUE:
    FSMTransIndex: ���ص�transition.

SIDE EFFECTS:
    none
==============================================================*/
FSMTransIndex  CParentState::DoParseEvent(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO( SNOOP, EC_SNOOP_NORMAL ), "->PARENT state parser." );
    switch ( msg.GetMessageId() )
        {
        case MSGID_ROAM_REQ:
            //�յ�Roam Request��Ϣ
            return TRANS_SNOOP_PARENT_ROAM_REQ;

        case MSGID_TIMER_HEARTBEAT:
            return TRANS_SNOOP_PARENT_HEARTBEAT_TIMER;

        case MSGID_TUNNEL_HEARTBEAT:
            return TRANS_SNOOP_PARENT_HEARTBEAT;

        case MSGID_TUNNEL_HEARTBEAT_RESP:
            return TRANS_SNOOP_PARENT_HEARTBEAT_RESP;

        case MSGID_TUNNEL_SYNC_REQ:
            //�յ�Tunnel Sync��Ϣ
            return TRANS_SNOOP_PARENT_TUNNEL_SYNC_REQ;

        case MSGID_TUNNEL_ESTABLISH_REQ:
            //�յ�Tunnel Establish Request
            return TRANS_SNOOP_PARENT_TUNNEL_ESTABLISH_REQ;

        case MSGID_TUNNEL_TERMINATE_REQ:
            //�յ�Tunnel Terminiate Request
            return TRANS_SNOOP_PARENT_TUNNEL_TERMINATE_REQ;

        case MSGID_TUNNEL_CHANGE_ANCHOR_RESP:
            //�յ�Tunnel ChangeAnchor Response
            return TRANS_SNOOP_PARENT_TUNNEL_CHGANCHOR_RESP;

        case MSGID_IPLIST_ADD_FIXIP:
            //IDLE״̬֮���յ�FIXIP������Ϣ
            return TRANS_SNOOP_PARENT_ADD_FIXIP;

        default:
            //���е�״̬��������
            return INVALID_EVENT;
        }
}
