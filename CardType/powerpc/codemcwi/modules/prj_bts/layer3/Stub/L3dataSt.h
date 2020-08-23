/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataStub.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   07/28/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_STUB_H__
#define __DATA_STUB_H__

#include <stdio.h>

#include "L3DataEB.h"
#include "L3DataSnoop.h"


#ifdef __WIN32_SIM__
#define INET_ADDR_LEN  (17)
#define inet_ntoa_b( inetAddress,  pString )     pString = inet_ntoa(inetAddress )  
#endif

void ARP_DelILEntry(UINT32);
void ARP_AddILEntry(UINT32 , const UINT8 * , UINT32);
//bool DM_QueryRenewFlag(UINT32);
//bool DM_QueryMobilityFlag(UINT32);
//bool DM_AllowAccess(UINT32);

typedef enum
{
    FT_AddEntry = 0,
    FT_UpdateEntry
}FTEntryOP;

typedef enum
{
    ARP_REQ = 0,
    ARP_REPLY,
    PADI,
    PADO,
    PADR,
    FailPADS,//Fail to begin a session
    SuccessPADS,//Sucess.
    PADT,
    PPPOE_SESSION,
    DHCP_DISCOVERY,
    DHCP_OFFER,
    DHCP_REQUEST_INIT,
    DHCP_REQUEST_RENEW,
    DHCP_ACK,
    DHCP_NAK,
    DHCP_RELEASE,
    DHCP_DECLINE,
    DHCP_INFORM,
    ETHERIP,
    //Other Types.
    EthernetPacket, // 任意以太网报文
    IpPacket,       //任意IP报文
    UdpPacket,      //任意UDP报文
    
    MAX_TYPE
}PACKET;


class Packet_factor
{
private:
    EtherHdr    m_Ether;
    ArpHdr      m_Arp;
    IpHdr       m_Ip;
    TcpHdr      m_Tcp;
    UdpHdr      m_Udp;
    PPPoEHdr    m_PPPoE;
    DhcpHdr     m_Dhcp;
    DhcpOption  m_DhcpOption;

public:
    Packet_factor()
        {
        memset( &m_DhcpOption, 0, sizeof( DhcpOption ) );
        }
    ~Packet_factor(){}

    UINT16 make(PACKET, const char*, char**);

    void SetEther(EtherHdr*);
    void SetArp(ArpHdr*);
    void SetIp(IpHdr*);
    void SetTcp(TcpHdr*);
    void SetUdp(UdpHdr*);
    void SetPPPoE(PPPoEHdr*);
    void SetDhcp(DhcpHdr*);
    void SetDchpOption(DhcpOption*);

private:
    void makeEther(char*);
    void makeArpRequest(char*);
    void makeArpReply(char*);
    void makePADI(char*);
    void makePADO(char*);
    void makePADR(char*);
    void makeSuccessPADS(char*);
    void makeFailPADS(char*);
    void makePADT(char*);
    void makePPPoESession(char*);
    void makeIp(char*);
    void makeUdp(char*);
    void makeDHCPDiscovery(char*);
    void makeDHCPOffer(char*);
    void makeDHCPInitREQUEST(char*);
    void makeDHCPRenewREQUEST(char*);
    void makeDHCPAck(char*);
    void makeDHCPNak(char*);
    void makeDHCPRelease(char*);
    void makeDHCPDecline(char*);
    void makeDHCPInform(char*);
    void makeDHCPOption(char*);
    void makeTcp(char*);
};

#endif /*__DATA_STUB_H__*/
