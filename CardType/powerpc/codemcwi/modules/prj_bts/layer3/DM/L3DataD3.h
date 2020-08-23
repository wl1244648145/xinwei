/*---------------------------------------------------------------------------*/
#ifndef __DM_MSGS_STRUCT_H
#define __DM_MSGS_STRUCT_H

#include "L3dataCommon.h"
#include "L3dataTypes.h"
#include "L3DataDmComm.h"

#ifdef WIN32
    #pragma pack(push, 1)
#else
    #pragma pack(1)
#endif
const UINT8  ACL_DEFAULTNO=0x06;

/*********************
 *Address Filter Table
 *********************/
typedef struct _tag_AddressFltrTb
    {
    UINT8       ucSrcMAC[M_MAC_ADDRLEN];
    UINT8       ucPeerMAC[M_MAC_ADDRLEN];
    UINT32      usSessionId;
    UINT32      ulSrcIpAddr; 
    } AddressFltrTb;


/************************
 *Protocol Filter Table
 ***********************/
typedef struct _tag_PrtclFltrEntry
    {
    UINT8       ucOccupied:1;
    UINT8       ucFlag:1;
    UINT8       ucPermit:1;
    UINT8       ucIsBroadcast:1;
    UINT8       :0;     //填充位
    //下一个字节
    UINT8       ucSrcOp:4;
    UINT8       ucDstOp:4;
    UINT16      usProtocol;     /*与PktAnalyzer 中的保持一致*/
    UINT16      usSrcPort;
    UINT16      usDstPort;
    UINT32      ulSrcAddr;
    UINT32      ulSrcMask;
    UINT32      ulDstAddr;
    UINT32      ulDstMask;
    } PrtclFltrEntry;


/****************************************************
 *M_MAX_PROTOCOL_REC不可定义太大；                           
 *sizeof(ProtocolFltrTbl) < BTS和CPE间的最大消息长度；
 ****************************************************/
#define M_MAX_PROTOCOL_REC (50)

typedef struct _tag_PrtclFltrTb
    {
    UINT32 ucCount;
    PrtclFltrEntry PrtlFltTbEntry[M_MAX_PROTOCOL_REC];
    }PrtclFltrTb;

typedef struct _tag_OpList
    {
    UINT8       ucOp; //0:Add;Delete;Update
    UINT8       ucNeedResp;//1:need response to snoop
    UTILEntry   stUTILEntry;
    } OPLIST;


typedef struct _tag_SyncCB
{
    OPLIST      astOpList[M_MAX_USER_PER_UT];
    UINT8       ucType; /*0x01: DAIB only;  0x11: Both DAIB and Addr; 0x10:Addr only*/
    UINT16      usSyncDAIBTransId;
    UINT16      usSyncDFTransId;
} SyncCB;

/*********************
 *EID Table
 *********************/
typedef struct _tag_CPETable
{
    UINT32      ulEid;
    struct _stConfig
    {
        UINT8 ucMobilityEn;
        UINT8 ucRenew;
        UINT16 usVlanId;
    }stConfig;
    UINT8       ucMaxSize;    //max records of DAIB
    UINT8       ucFixSize;    //
    UINT8       ucDynIpSize;
    UINT8       ucNeedDataRsp;
    UINT8       ucPrtlCtrlState;    /* Idle, ACL, Config, End */
    UINT16      usPrtlCtrlTransId;  /* transaction ID*/
    SyncCB          *pstSyncCB;
    UTILEntry       *pstIplstTbEnty;
    AddressFltrTb   *pstAddrFltrTbEntry;
    UINT32 regTime;//记录数据注册的时间，用来判断是否是延迟消息
} CPETable;

typedef enum 
    {
    REST_FAIL,
    REST_SUCCESS
    }REST_TYPE;

//define update ACL Request to Ut
typedef struct T_ACLDldReq
    {
    UINT16 Xid;
    UINT16 Version;
    UINT32 ucNo;
    PrtclFltrEntry arACLTb[M_MAX_PROTOCOL_REC+ACL_DEFAULTNO];
    }ACLDldReq;

typedef struct T_UtBtsResponse//AddrFltUpdtRsp,ProtlFltUpdtRsp
    {
    UINT16   Xid;
    UINT16   Version;
    UINT16   Result;
    }UtBtsResponse;

//define reponse to OAM
typedef struct T_DataResponse//AddrFltUpdtRsp,ProtlFltUpdtRsp
    {
    UINT16   Xid;
    UINT16    Result;
    }DataResponse;

typedef struct T_IplistTLV
    {
    UINT8  Type;
    UINT16 Len;
    UTILEntry Iplist;
    }IplistTLV;

//define DataServiceRequest Message struct
typedef struct T_DataServsReq
    {
    UINT16 Version;
    UINT32 TimeStamp;
    UINT32  IpCount;
    IplistTLV IplstTLV[M_MAX_USER_PER_UT];
    }DataServsReq;

//define Synchro DAIB Message struct
typedef struct T_DAIBReq
    {
    UINT16 Xid;
    UINT16 Version;
    UINT32 TimeStamp;
    UINT32  IpCount;
    IplistTLV IplstTLV[M_MAX_USER_PER_UT];
    }DAIBReq;

//define Synchro AddressFilter Message struct
typedef struct T_AddrFltTbReq
    {
    UINT16 Xid;
    UINT16 Version;
    UINT32 addrCount;
    AddressFltrTb AddrsFltrTb[M_MAX_USER_PER_UT];
    }AddrFltTbReq;

//define DataServerse Response struct
typedef struct T_DataServsRsp 
    {
    UINT8   Result;
    UINT8   WorkMode;
    UINT16  usVlanID;   //UT profile中携带的group id对应于注册bts上的vlan id;
    }DataServsRsp;
typedef struct T_ACLConfig
    {
    UINT8   Protocol;
    UINT32  SourceIp;
    UINT32  SourceWildCard;
    UINT16  SourcePort;
    UINT8   SourceOpr;
    UINT32  DestIp;
    UINT32  DestWildCard;
    UINT16  DestPort;
    UINT8   DestOpr;
    UINT8   Permit;

    }ACLConfig;

//define ACL config Request struct from OAM
typedef  struct T_ACLConfigReq
    {
    UINT16  Xid;
    UINT8   Index;
    UINT8   ACLCount;
    ACLConfig ACLConfigEntry[M_MAX_PROTOCOL_REC];
    }ACLConfigReq;


typedef struct _tag_CpeFixIpInfo
    {
    UINT32 Eid;
    UINT8  MAC[M_MAC_ADDRLEN];
    UINT32 FixIP;
    UINT32 AnchorBTSID;
    UINT32 RouterAreaID;
    }CpeFixIpInfo;

//define CPE DateConfig Resquest
typedef struct T_CPEDataConfigReq
    {
    UINT16 TransId;
    UINT8  Mobility;   //0 -- disabled     1 -- enabled
    UINT8  DHCPRenew;  //Allow DHCP Renew in serving BTS 0-- disabled  1-- enabled
    UINT16 usVlanId;   //VLAN ID [0~4095]
    UINT8  MaxIpNum;   //Max IP ddress number	1~20	
    UINT8  FixIpNum;   //Fixed Ip number		0~20
    CpeFixIpInfo stCpeFixIpInfo[M_MAX_USER_PER_UT];
    }CPEDataConfigReq;

//define DM DateConfig Resquest
typedef struct T_DMConfigReq
    {
    UINT16 Xid;     
    UINT8  Mobility;
    UINT8  AccessCtrl;
    }DMConfigReq;

//CPE统计包结构
typedef struct T_CPEPerfm
    {
    UINT32 ulEid;
    UINT32 ulStartT;
    UINT32 arPfmMeasure[MAX_PERFMC];
    }CPEPerfm;
//cpe probe msg
typedef struct T_CPEIplistbyProbe
{
    UINT32 Ipaddr;
    UINT8 MAC[M_MAC_ADDRLEN];
    UINT8 type;//0:dhcp,1:pppoe,2:fixed ip
}CPEIplistbyProbe;
typedef struct T_CPEProbeReq
{
    UINT16 result;
    UINT32 ulEid;
    UINT8 num;
    CPEIplistbyProbe cpeiplist[M_MAX_USER_PER_UT];
}CPEProbeReq;

const UINT16 PPPoE  = M_ETHER_TYPE_PPPoE_DISCOVERY; 
const UINT16 T_ARP = M_ETHER_TYPE_ARP;

//允许ARP
const PrtclFltrEntry PRTL_ARP=
{
    1,      //UINT8       ucOccupied;
    0,      //UINT8        bFlag;
    1,      //UINT8        bPermit;
    0,      //UINT8        bIsBroadcast;
    0,      //UINT8       ucSrcOp;
    0,      //UINT8       ucDstOp;
    T_ARP,  //UINT16      usProtocol;  
    0,      //UINT16      usSrcPort;
    0,      //UINT16      usDstPort;
    0,      //UINT32      ulSrcAddr;
    0,      //UINT32      ulSrcMask;
    0,      //UINT32      ulDstAddr;
    0      //UINT32      ulDstMask;
};

//允许DHCP Client
const PrtclFltrEntry PRTL_DHCP=
{
    1,      //UINT8       ucOccupied;
    0,      //UINT8        bFlag;
    1,      //UINT8        bPermit;
    0,      //UINT8        bIsBroadcast;
    eq,     //UINT8       ucSrcOp;
    eq,     //UINT8       ucDstOp;
    M_PROTOCOL_TYPE_UDP,    //UINT16      usProtocol;   
    M_DHCP_PORT_CLIENT,      //UINT16      usSrcPort;
    M_DHCP_PORT_SERVER,      //UINT16      usDstPort;
    0,      //UINT32      ulSrcAddr;
    0,      //UINT32      ulSrcMask;
    0,      //UINT32      ulDstAddr;
    0      //UINT32      ulDstMask;
};

//允许PPPoE
const PrtclFltrEntry PRTL_PPP=
{
    1,      //UINT8       ucOccupied;
    0,      //UINT8        bFlag;
    1,       //UINT8        bPermit;
    0,      //UINT8        bIsBroadcast;
    0,      //UINT8       ucSrcOp;
    0,      //UINT8       ucDstOp;
    PPPoE,  //UINT16      usProtocol;  
    0,      //UINT16      usSrcPort;
    0,      //UINT16      usDstPort;
    0,      //UINT32      ulSrcAddr;
    0,      //UINT32      ulSrcMask;
    0,      //UINT32      ulDstAddr;
    0      //UINT32      ulDstMask;
};

//禁止其他类型的广播
const PrtclFltrEntry PRTL_ForbidBroadcast=
{
    0,      //UINT8       ucOccupied;
    0,      //UINT8        bFlag;
    0,      //UINT8        bPermit;
    0,      //UINT8        bIsBroadcast;
    0,      //UINT8       ucSrcOp;
    0,      //UINT8       ucDstOp;
    0,      //UINT16      usProtocol;   
    0,      //UINT16      usSrcPort;
    0,      //UINT16      usDstPort;
    0,      //UINT32      ulSrcAddr;
    0,      //UINT32      ulSrcMask;
    0,      //UINT32      ulDstAddr;
    0      //UINT32      ulDstMask;
};

//允许认证用户的其他报文
const PrtclFltrEntry PRTL_PERMIT_AUTHED_ALL=
{
    1,      //UINT8       ucOccupied;
    1,      //UINT8        bFlag;
    1,      //UINT8        bPermit;
    0,      //UINT8        bIsBroadcast;
    0,      //UINT8       ucSrcOp;
    0,      //UINT8       ucDstOp;
    0,      //UINT16      usProtocol;   
    0,      //UINT16      usSrcPort;
    0,      //UINT16      usDstPort;
    0,      //UINT32      ulSrcAddr;
    0,      //UINT32      ulSrcMask;
    0,      //UINT32      ulDstAddr;
    0      //UINT32      ulDstMask;
};

//拒绝未认证用户的其他报文
const PrtclFltrEntry PRTL_DENY_OTHERS=
{
    1,      //UINT8       ucOccupied;
    0,      //UINT8        bFlag;
    0,      //UINT8        bPermit;
    0,      //UINT8        bIsBroadcast;
    0,      //UINT8       ucSrcOp;
    0,      //UINT8       ucDstOp;
    0,      //UINT16      usProtocol;   
    0,      //UINT16      usSrcPort;
    0,      //UINT16      usDstPort;
    0,      //UINT32      ulSrcAddr;
    0,      //UINT32      ulSrcMask;
    0,      //UINT32      ulDstAddr;
    0      //UINT32      ulDstMask;
};

#ifdef WIN32
    #pragma pack(pop)
#else
    #pragma pack()
#endif

#endif /* __CPE_MSGS_STRUCT_H */  
