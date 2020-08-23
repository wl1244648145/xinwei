/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    packetFactor.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   08/05/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/


#include <memory>
#include <stdio.h>
#include <assert.h>

#include "L3DataStub.h"
#include "L3DataDhcp.h"

/*==================================================================
MEMBER FUNCTION:
    Packet_factor::make

DESCRIPTION:
    生产指定类型的包，包括生成Ethernet头。
    ppBuf内存由make申请
    使用:
    1) 确定准备生成的包是否需要Ethernet头，IP,TCP,UDP,DHCP,PPPoE头?
    2) 调用相应的Set成员函数(如SetEther())，设置后Ethernet...头的格式
    3) 确定需要发送的其他数据，如:PPPoE头后的PPP包，DHCP头后的选项等
    4) 调用make()生成包, *ppBuf就是生成的包

ARGUMENTS:
    Type: 指定生成包的类型
    pPayLoad: 其他附加的数据
    **ppBuf: 指向生成的包

RETURN VALUE:
    UINT16:返回数据包的长度

SIDE EFFECTS:
    none
==================================================================*/
UINT16 Packet_factor::make( PACKET Type, const char *pPayLoad, char **ppBuf )
{
    UINT16 usLen = 0;
    UINT16 usPacketLen;

    if ( NULL != pPayLoad )
        {
        usLen = strlen( pPayLoad );
        }

    switch( Type )
        {
        case ARP_REQ:
            usPacketLen = sizeof( EtherHdr ) + sizeof( ArpHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset(*ppBuf, 0, usPacketLen);

            m_Ether.usProto = htons( M_ETHER_TYPE_ARP );
            makeEther(*ppBuf);
            makeArpRequest( (*ppBuf) + sizeof( EtherHdr ) );
            break;

        case ARP_REPLY:
            usPacketLen = sizeof( EtherHdr ) + sizeof( ArpHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset(*ppBuf, 0, usPacketLen);

            m_Ether.usProto = htons( M_ETHER_TYPE_ARP );
            makeEther(*ppBuf);
            makeArpReply( (*ppBuf) + sizeof( EtherHdr ) );
            break;

        case PADI:
            usPacketLen = sizeof( EtherHdr ) + sizeof( PPPoEHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            memset(m_Ether.aucDstMAC, 0xff, M_MAC_ADDRLEN);//broadcast:ff-ff-ff-ff-ff-ff
            m_Ether.usProto = htons( M_ETHER_TYPE_PPPoE_DISCOVERY );
            makeEther( *ppBuf );
            makePADI( (*ppBuf) + sizeof(EtherHdr) );
            break;

        case PADO:
            usPacketLen = sizeof( EtherHdr ) + sizeof( PPPoEHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_PPPoE_DISCOVERY );
            makeEther( *ppBuf );
            makePADO( (*ppBuf) + sizeof(EtherHdr) );
            break;

        case PADR:
            usPacketLen = sizeof( EtherHdr ) + sizeof( PPPoEHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_PPPoE_DISCOVERY );
            makeEther( *ppBuf );
            makePADR( (*ppBuf) + sizeof(EtherHdr) );
            break;

        case FailPADS:
            usPacketLen = sizeof( EtherHdr ) + sizeof( PPPoEHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_PPPoE_DISCOVERY );
            makeEther( *ppBuf );
            makeFailPADS( (*ppBuf) + sizeof(EtherHdr) );
            break;

        case SuccessPADS:
            usPacketLen = sizeof( EtherHdr ) + sizeof( PPPoEHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_PPPoE_DISCOVERY );
            makeEther(*ppBuf);
            makeSuccessPADS( (*ppBuf) + sizeof(EtherHdr) );
            break;

        case PADT:
            usPacketLen = sizeof( EtherHdr ) + sizeof( PPPoEHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset(*ppBuf, 0, usPacketLen);

            m_Ether.usProto = htons( M_ETHER_TYPE_PPPoE_DISCOVERY );
            makeEther(*ppBuf);
            makePADT( (*ppBuf) + sizeof( EtherHdr ) );
            break;

        case PPPOE_SESSION:
            usPacketLen = sizeof( EtherHdr ) + sizeof( PPPoEHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset(*ppBuf, 0, usPacketLen);

            m_Ether.usProto = htons( M_ETHER_TYPE_PPPoE_SESSION );
            makeEther(*ppBuf);
            makePPPoESession( (*ppBuf) + sizeof( EtherHdr ) );
            //copy payload, begin with PPP Protoco-ID.
            if ( NULL != pPayLoad )
                {
                memcpy(*ppBuf + sizeof(EtherHdr) + sizeof(PPPoEHdr), pPayLoad, usLen );
                }
            break;

        case DHCP_DISCOVERY:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_SERVER );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_CLIENT );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPDiscovery( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_DISCOVERY;
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case DHCP_OFFER:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_CLIENT );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_SERVER );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPOffer( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_OFFER;
            assert(m_DhcpOption.ulIpLeaseTime != 0);
            assert(m_DhcpOption.ulServerId != 0);
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case DHCP_REQUEST_INIT:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_SERVER );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_CLIENT );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPInitREQUEST( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_REQUEST;
            assert( m_DhcpOption.ulRequestIp != 0 );
            assert( m_DhcpOption.ulServerId != 0 );
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case DHCP_REQUEST_RENEW:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_SERVER );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_CLIENT );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPRenewREQUEST( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_REQUEST;
            assert( m_DhcpOption.ulRequestIp == 0 );
            assert( m_DhcpOption.ulServerId == 0 );
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case DHCP_ACK:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_CLIENT );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_SERVER );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPAck( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_ACK;
            assert( m_DhcpOption.ulRequestIp == 0 );
            assert( m_DhcpOption.ulIpLeaseTime != 0 );
            assert( m_DhcpOption.ulServerId != 0 );
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case DHCP_NAK:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_CLIENT );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_SERVER );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPNak( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_NAK;
            assert( m_DhcpOption.ulRequestIp == 0 );
            assert( m_DhcpOption.ulIpLeaseTime == 0 );
            assert( m_DhcpOption.ulServerId != 0 );
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case DHCP_RELEASE:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_SERVER );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_CLIENT );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPRelease( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_RELEASE;
            assert( m_DhcpOption.ulRequestIp == 0 );
            assert( m_DhcpOption.ulIpLeaseTime == 0 );
            assert( m_DhcpOption.ulServerId != 0 );
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case DHCP_DECLINE:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_SERVER );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_CLIENT );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPDecline( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_DECLINE;
            assert( m_DhcpOption.ulRequestIp != 0 );
            assert( m_DhcpOption.ulIpLeaseTime == 0 );
            assert( m_DhcpOption.ulServerId != 0 );
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case DHCP_INFORM:
            usPacketLen = ( sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) + 0x100 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ether.usProto = htons( M_ETHER_TYPE_IP );
            makeEther(*ppBuf);
            m_Ip.usTotalLen = sizeof( IpHdr ) + sizeof( UdpHdr ) 
                + sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Ip.ucProto = M_PROTOCOL_TYPE_UDP;
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            m_Udp.usLen = sizeof( DhcpHdr ) + 0x100;//0x100是Option的长度
            m_Udp.usDstPort = htons( M_DHCP_PORT_SERVER );
            m_Udp.usSrcPort = htons( M_DHCP_PORT_CLIENT );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );
            makeDHCPInform( *ppBuf + sizeof( EtherHdr ) 
                + sizeof( IpHdr ) + sizeof(UdpHdr ) );
            m_DhcpOption.ucMessageType = M_DHCP_INFORM;
            assert( m_DhcpOption.ulRequestIp == 0 );
            assert( m_DhcpOption.ulIpLeaseTime == 0 );
            assert( m_DhcpOption.ulServerId == 0 );
            makeDHCPOption( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) 
                + sizeof( UdpHdr ) + sizeof( DhcpHdr ) );
            break;

        case ETHERIP:
            {
            assert( usLen != 0 );
            usPacketLen = ( sizeof( IpHdr ) + sizeof( EtherIpHdr)  + usLen + 1 );
            *ppBuf = new char[ usPacketLen ];
            memset( *ppBuf, 0, usPacketLen );

            m_Ip.usTotalLen = usPacketLen;
            m_Ip.ucProto = M_PROTOCOL_TYPE_ETHERIP;
            makeIp( *ppBuf );

            EtherIpHdr *pEtherIpHdr = (EtherIpHdr*)( *ppBuf + sizeof( IpHdr ) );
            pEtherIpHdr->usVer = htons( 3 << 12 );

            memcpy(*ppBuf + sizeof( IpHdr ) + sizeof( EtherIpHdr ), pPayLoad, usLen);
            }
            break;

        case EthernetPacket:

            assert( 0 != usLen );

            usPacketLen = sizeof( EtherHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset(*ppBuf, 0, usPacketLen);
            makeEther(*ppBuf);

            memcpy( (UINT8*)( *ppBuf + sizeof( EtherHdr ) ), pPayLoad, usLen );
            break;

        case IpPacket:

            assert( 0 != usLen );

            usPacketLen = sizeof( EtherHdr ) + sizeof( IpHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset(*ppBuf, 0, usPacketLen);
            makeEther( *ppBuf );
            makeIp( *ppBuf + sizeof( EtherHdr ) );

            memcpy( (UINT8*)( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) ), pPayLoad, usLen );
            break;

        case UdpPacket:

            assert( 0 != usLen );

            usPacketLen = sizeof( EtherHdr ) + sizeof( IpHdr ) + sizeof( UdpHdr ) + usLen + 1;
            *ppBuf = new char[ usPacketLen ];
            memset(*ppBuf, 0, usPacketLen);
            makeEther( *ppBuf );
            makeIp( *ppBuf + sizeof( EtherHdr ) );
            makeUdp( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) );

            memcpy( (UINT8*)( *ppBuf + sizeof( EtherHdr ) + sizeof( IpHdr ) + sizeof( UdpHdr ) ), pPayLoad, usLen );
            break;

        default:
            assert(0);
            break;
        }

    return usPacketLen;
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeEther(char *pBuf)
{
    EtherHdr* pEther = (EtherHdr*)pBuf;
    memcpy( pEther->aucDstMAC, m_Ether.aucDstMAC, M_MAC_ADDRLEN );
    memcpy( pEther->aucSrcMAC, m_Ether.aucSrcMAC, M_MAC_ADDRLEN );
    pEther->usProto = m_Ether.usProto;

    return;
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeArpRequest(char *pBuf)
{
    ArpHdr* pArp = (ArpHdr*)pBuf;
    pArp->usHtype = htons( M_HARDWARE_TYPE_ETHERNET );      /*Ethernet*/
    pArp->usPtype = htons( M_ETHER_TYPE_IP );  /*Ip*/
    pArp->ucHlen  = M_MAC_ADDRLEN;
    pArp->ucPlen  = M_IP_ADDRLEN;      /*Ip V4*/
    pArp->usOp    = htons( M_ARP_REQUEST );

    memcpy( pArp->aucSenderHaddr, m_Arp.aucSenderHaddr, M_MAC_ADDRLEN );
    pArp->ulSenderPaddr = m_Arp.ulSenderPaddr;
//    memcpy( pArp->aucDestHaddr, m_pArp->aucDestHaddr, M_MAC_ADDRLEN );
    pArp->ulDestPaddr = m_Arp.ulDestPaddr;

    return;
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeArpReply(char *pBuf)
{
    ArpHdr* pArp = (ArpHdr*)pBuf;
    pArp->usHtype = htons( M_HARDWARE_TYPE_ETHERNET );      /*Ethernet*/
    pArp->usPtype = htons( M_ETHER_TYPE_IP );  /*Ip*/
    pArp->ucHlen  = M_MAC_ADDRLEN;
    pArp->ucPlen  = M_IP_ADDRLEN;      /*Ip V4*/
    pArp->usOp    = htons( M_ARP_REPLY );

    memcpy( pArp->aucSenderHaddr, m_Arp.aucSenderHaddr, M_MAC_ADDRLEN );
    pArp->ulSenderPaddr = m_Arp.ulSenderPaddr;
    memcpy( pArp->aucDestHaddr, m_Arp.aucDestHaddr, M_MAC_ADDRLEN );
    pArp->ulDestPaddr = m_Arp.ulDestPaddr;

    return;
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makePADI(char *pBuf)
{
    PPPoEHdr *pPPPoE = (PPPoEHdr*)pBuf;
    pPPPoE->ucVerType = M_PPPoE_VERTYPE;    //must be 0x11
    pPPPoE->ucCode = M_PPPoE_PADI;
    pPPPoE->usSessionId = htons( 0x0 );
    pPPPoE->usLength = htons( 0x0 );                 //Length of payload.
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makePADO(char *pBuf)
{
    PPPoEHdr *pPPPoE = (PPPoEHdr*)pBuf;
    pPPPoE->ucVerType = M_PPPoE_VERTYPE;    //must be 0x11
    pPPPoE->ucCode = M_PPPoE_PADO;
    pPPPoE->usSessionId = htons( 0x0 );
    pPPPoE->usLength = htons( 0x0 );                 //Length of payload.
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makePADR(char *pBuf)
{
    PPPoEHdr *pPPPoE = (PPPoEHdr*)pBuf;
    pPPPoE->ucVerType = M_PPPoE_VERTYPE;    //must be 0x11
    pPPPoE->ucCode = M_PPPoE_PADR;
    pPPPoE->usSessionId = htons( 0x0 );
    pPPPoE->usLength = htons( 0x0 );                 //Length of payload.
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeSuccessPADS(char *pBuf)
{
    PPPoEHdr *pPPPoE = (PPPoEHdr*)pBuf;
    pPPPoE->ucVerType = M_PPPoE_VERTYPE;    //must be 0x11
    pPPPoE->ucCode = M_PPPoE_PADS;
    assert( 0 != m_PPPoE.usSessionId );
    pPPPoE->usSessionId = m_PPPoE.usSessionId;
    pPPPoE->usLength = 0x0;                 //Length of payload.
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeFailPADS(char *pBuf)
{
    PPPoEHdr *pPPPoE = (PPPoEHdr*)pBuf;
    pPPPoE->ucVerType = M_PPPoE_VERTYPE;    //must be 0x11
    pPPPoE->ucCode = M_PPPoE_PADS;
    pPPPoE->usSessionId = htons( 0x0 );
    pPPPoE->usLength = htons( 0x0 );                 //Length of payload.
}



//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makePADT(char *pBuf)
{
    PPPoEHdr *pPPPoE = (PPPoEHdr*)pBuf;
    pPPPoE->ucVerType = M_PPPoE_VERTYPE;    //must be 0x11
    pPPPoE->ucCode = M_PPPoE_PADT;
    assert( 0 != m_PPPoE.usSessionId );
    pPPPoE->usSessionId = m_PPPoE.usSessionId;
    pPPPoE->usLength = htons( 0x0 );                 //Length of payload.
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makePPPoESession(char *pBuf)
{
    PPPoEHdr *pPPPoE = (PPPoEHdr*)pBuf;
    pPPPoE->ucVerType = M_PPPoE_VERTYPE;    //must be 0x11
    pPPPoE->ucCode = M_PPPoE_SESSION;
    assert( 0 != m_PPPoE.usSessionId );
    pPPPoE->usSessionId = m_PPPoE.usSessionId;
    pPPPoE->usLength = htons( 0x2 );                 //Length of payload. only PPP Protoco-ID
}

//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeIp(char *pBuf)
{
    IpHdr *pIp = ( IpHdr* )pBuf;
    pIp->ucLenVer = 0x45;   //
    pIp->ucProto = ( m_Ip.ucProto == 0 )?M_PROTOCOL_TYPE_UDP:m_Ip.ucProto;
    pIp->ucTOS = m_Ip.ucTOS;
    pIp->ucTTL = 128;
    pIp->ulDstIp = ( m_Ip.ulDstIp == 0 )?htonl( 0x02020202 ):m_Ip.ulDstIp;
    pIp->ulSrcIp = ( m_Ip.ulSrcIp == 0 )?htonl( 0x01010101 ):m_Ip.ulSrcIp;
    pIp->usCheckSum = htons( 0 );
    pIp->usFragAndFlags = 0;
    pIp->usId = 0;
    pIp->usTotalLen = ( m_Ip.usTotalLen == 0 )?0x100:m_Ip.usTotalLen;
    //需要根据后面的包长度动态计算，但是在测试时可以不关心；
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeUdp(char *pBuf)
{
    UdpHdr *pUdp = ( UdpHdr* )pBuf;
    pUdp->usSrcPort = m_Udp.usSrcPort;
    pUdp->usDstPort = m_Udp.usDstPort;
    pUdp->usLen = ( m_Udp.usLen == 0 )?0x120:m_Udp.usLen;
    pUdp->usCheckSum = htons( 0 );//不关心
}



//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPDiscovery(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REQUEST;
    pDhcp->ulCiaddr = 0; //discovery
    pDhcp->ulGiaddr = m_Dhcp.ulGiaddr;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    pDhcp->ulYiaddr = 0;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = htons( 0 );//
}



//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPOffer(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REPLY;
    pDhcp->ulCiaddr = 0; //discovery
    pDhcp->ulGiaddr = m_Dhcp.ulGiaddr;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    assert( m_Dhcp.ulYiaddr != 0 );
    pDhcp->ulYiaddr = m_Dhcp.ulYiaddr;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = 0;//
}




//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPInitREQUEST(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REQUEST;
    pDhcp->ulCiaddr = 0; //init
    pDhcp->ulGiaddr = m_Dhcp.ulGiaddr;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    assert( m_Dhcp.ulYiaddr != 0 );
    pDhcp->ulYiaddr = 0;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = 0;//
}



//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPRenewREQUEST(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REQUEST;
    pDhcp->ulCiaddr = ( m_Dhcp.ulCiaddr == 0 )?htonl( 0x03030303 ):m_Dhcp.ulCiaddr; 
    pDhcp->ulGiaddr = m_Dhcp.ulGiaddr;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    assert( m_Dhcp.ulYiaddr != 0 );
    pDhcp->ulYiaddr = 0;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = 0;//
}



//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPAck(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REPLY;
    assert( m_Dhcp.ulCiaddr != 0 );
    pDhcp->ulCiaddr = ( m_Dhcp.ulCiaddr == 0 )?htonl( 0x03030303 ):m_Dhcp.ulCiaddr; 
    pDhcp->ulGiaddr = m_Dhcp.ulGiaddr;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    assert( m_Dhcp.ulYiaddr != 0 );
    pDhcp->ulYiaddr = m_Dhcp.ulYiaddr;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = 0;//
}



//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPNak(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REPLY;
    pDhcp->ulCiaddr = 0; 
    pDhcp->ulGiaddr = m_Dhcp.ulGiaddr;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    pDhcp->ulYiaddr = 0;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = 0;//
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPRelease(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REQUEST;
    assert( m_Dhcp.ulCiaddr != 0 );
    pDhcp->ulCiaddr = m_Dhcp.ulCiaddr; 
    pDhcp->ulGiaddr = 0;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    pDhcp->ulYiaddr = 0;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = 0;//
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPDecline(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REQUEST;
    pDhcp->ulCiaddr = 0; 
    pDhcp->ulGiaddr = 0;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    pDhcp->ulYiaddr = 0;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = 0;//
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPInform(char *pBuf)
{
    DhcpHdr *pDhcp = ( DhcpHdr* )pBuf;
    memcpy( pDhcp->aucChaddr, m_Dhcp.aucChaddr, M_DHCP_CLIENT_HADDR_LEN );
    memcpy( pDhcp->aucFile, "FileName", strlen("FileName") );
    memcpy( pDhcp->aucSname, "ServerName", strlen("ServerName") );
    pDhcp->ucHlen = M_MAC_ADDRLEN;
    pDhcp->ucHop = 0;
    pDhcp->ucHtype = M_HARDWARE_TYPE_ETHERNET;
    pDhcp->ucOpCode = M_DHCP_OP_REQUEST;
    pDhcp->ulCiaddr = m_Dhcp.ulCiaddr; 
    pDhcp->ulGiaddr = 0;
    pDhcp->ulSiaddr = 0;
    pDhcp->ulXid = m_Dhcp.ulXid;//不关心
    pDhcp->ulYiaddr = 0;
    pDhcp->usFlag = 0;//
    pDhcp->usSec = 0;//
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::makeDHCPOption(char *pBuf)
{
    UINT8 aucDhcpMagic[ M_DHCP_OPTION_LEN_MAGIC ] = RFC1048_MAGIC;
    memcpy( pBuf, aucDhcpMagic, M_DHCP_OPTION_LEN_MAGIC );
    char *pDhcpOption = pBuf + M_DHCP_OPTION_LEN_MAGIC;

    //Message Type
    *pDhcpOption++ = M_DHCP_OPTION_MSGTYPE;
    *pDhcpOption++ = M_DHCP_OPTION_LEN_MESSAGETYPE;
    *pDhcpOption++ = m_DhcpOption.ucMessageType;

    if ( 0 != ntohl( m_DhcpOption.ulIpLeaseTime ) )
        {
        //增加Leaset Time选项
        *pDhcpOption++ = M_DHCP_OPTION_LEASE_TIME;
        *pDhcpOption++ = M_DHCP_OPTION_LEN_LEASETIME;
        *(UINT32*)pDhcpOption = m_DhcpOption.ulIpLeaseTime;
        pDhcpOption += M_DHCP_OPTION_LEN_LEASETIME;
        }

    if ( 0 != ntohl( m_DhcpOption.ulServerId ) )
        {
        //增加Leaset Time选项
        *pDhcpOption++ = M_DHCP_OPTION_SERVER_ID;
        *pDhcpOption++ = M_DHCP_OPTION_LEN_SERVERID;
        *(UINT32*)pDhcpOption = m_DhcpOption.ulServerId;
        pDhcpOption += M_DHCP_OPTION_LEN_SERVERID;
        }

    if ( 0 != ntohl( m_DhcpOption.ulRequestIp ) )
        {
        //增加Request Ip选项
        *pDhcpOption++ = M_DHCP_OPTION_REQUEST_IPADDR;
        *pDhcpOption++ = M_DHCP_OPTION_LEN_REQUESTIP;
        *(UINT32*)pDhcpOption = m_DhcpOption.ulRequestIp;
        pDhcpOption += M_DHCP_OPTION_LEN_REQUESTIP;
        }
    //add End Flag:
    *pDhcpOption = (char)0xff;
}


//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::SetEther(EtherHdr *pEther)
{
    memcpy( &m_Ether, pEther, sizeof(EtherHdr) );
}

//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::SetArp(ArpHdr *pArp)
{
    memcpy( &m_Arp, pArp, sizeof(ArpHdr) );
}

//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::SetIp(IpHdr *pIp)
{
    memcpy( &m_Ip, pIp, sizeof(IpHdr) );
}

//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::SetTcp(TcpHdr *pTcp)
{
    memcpy( &m_Tcp, pTcp, sizeof(TcpHdr) );
}

//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::SetUdp(UdpHdr *pUdp)
{
    memcpy( &m_Udp, pUdp, sizeof(UdpHdr) );
}

//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::SetPPPoE(PPPoEHdr *pPPPoE)
{
    memcpy( &m_PPPoE, pPPPoE, sizeof(PPPoEHdr) );
}

//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::SetDhcp(DhcpHdr *pDhcp)
{
    memcpy( &m_Dhcp, pDhcp, sizeof(DhcpHdr) );
}

//-----------------------------------------------
//-----------------------------------------------
void Packet_factor::SetDchpOption(DhcpOption *pDhcpOption)
{
    memcpy( &m_DhcpOption, pDhcpOption, sizeof(DhcpOption) );
}

