#ifndef bootpacket
#define bootpacket

/*typedef short int INT16;
typedef int INT32;
typedef unsigned short int UINT16;
typedef unsigned int UINT32;*/

#define DST_MAC_H 0x0008
#define DST_MAC_M 0x7433
#define DST_MAC_L 0x2630

#define SRC_MAC_H 0x00a0
#define SRC_MAC_M 0x1e01
#define SRC_MAC_L 0x0101

#define SRC_IP_ADDR_H 0x0a00
#define SRC_IP_ADDR_L 0x0002
#define DST_IP_ADDR_H 0x9EDA
#define DST_IP_ADDR_L 0x6723

#define UDP_SRC_PORT 0x0
#define UDP_SRC_PORT_BTPKT 0xBEEF  /* Bill's bt-pkt.exe use this value */
#define UDP_DST_PORT 0x9

#define ETHER_TYPE_IP 0x800
#define IP_TYPE_UDP 0x11    

#define MAGICNO 0x544B

/* Actual packets has 14 +20 + 8 + 244 = 42 + 244 = 286 bytes */

#define MAC_HEADER_LEN     14
#define IP_HEADER_LEN      20
#define UDP_HEADER_LEN     8
#define BOOTTBL_HEADER_LEN 4
#define TOTAL_HEADER_LEN   46  /* 14 + 20 + 8 + 4 = 46 bytes */

#define MAX_PAYLOAD_LEN    1472/*1408*//*244*/
//#define MAX_PAYLOAD_LEN    1176
#define MAX_BOOTTBL_LEN (MAX_PAYLOAD_LEN - BOOTTBL_HEADER_LEN) /* 240 bytes */

#define SYMBOL 0xEA00
#define BGN_BYTE_LEN 0x04  /* ex: 0x0000, 0x00A2 */
#define END_BYTE_LEN 0x06  /* ex: EA00, 0000, 0000 */


struct BootPacket {
		INT16 dstMAC_H;       /* MAC */
		INT16 dstMAC_M;
		INT16 dstMAC_L;
		INT16 srcMAC_H;
		INT16 srcMAC_M;
		INT16 srcMAC_L;
		INT16 EtherType;
		INT16 VerTOS;         /* IP */
		UINT16 IPlength;
		INT16 ID;
		INT16 FlagsFrag;
		INT16 TTLProtocol;
		UINT16 IPchecksum;
		INT16 srcIPAddr_H;
		INT16 srcIPAddr_L;
		INT16 dstIPAddr_H;
		INT16 dstIPAddr_L;
		INT16 srcPort;        /* UDP */
		INT16 dstPort;
		UINT16 UDPlength;
		UINT16 UDPchecksum;
		INT16 MagicNo;        /* Payload */
		INT16 SeqNo;
		INT16 BootTable[MAX_BOOTTBL_LEN/2];
};



#endif