#ifndef _INC_L3_BTS_MESSAGEID
#define _INC_L3_BTS_MESSAGEID

#ifndef DATATYPE_H
#include "datatype.h"   
#endif

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#define    PM_UPLOAD_DIR      "system/perfupload/"
#define SIG_UPLOAD_DIR "system/signal/"
#define PM_UPLOAD_DIR_NEW "system/performance"

const    UINT16    M_L3TOL2_PERFDATA_REQ        =    0x0204;
const    UINT16    M_L3TOL2_PERFDATA_RSP        =    0x0205;
const    UINT16    M_L3TOL2_DOWN_PERFDATA_REQ    =    0x3501;
const    UINT16    M_L3TOL2_DOWN_PERFDATA_RSP    =    0x3502;
const    UINT16    M_L3TOL2_BTS_USER_REQ    =    0x3503;
const    UINT16    M_L2TOL3_BTS_USER_RSP    =    0x3504;

const UINT16 M_L3TOL2_BTS_RTMONITOR_REQ     = 0x3505;
const UINT16 M_L2TOL3_BTS_RTMONITOR_RSP     = 0x3506;
const UINT16 M_L3TOL2_ACTIVE_USER_LIST_REQ     = 0x3507;
const UINT16 M_L2TOL3_ACTIVE_USER_LIST_RSP     = 0x3508;
const UINT16 M_L3TOL2_ACTIVE_USER_INFO_LIST_REQ     = 0x350A;
const UINT16 M_L2TOL3_ACTIVE_USER_INFO_LIST_RSP     = 0x350B;
//new stat
const    UINT16    M_L3TOL2_PERFDATA_REQ_new = 0x350c;
const    UINT16    M_L3TOL2_PERFDATA_RSP_new = 0x350d;
const    UINT16    M_L2_CUR_PERF_REQ_TIMEOUT_NOTIFY        =        0x0291;        //L2实时数据请求超时
const    UINT16    M_L2_PERIOD_PERF_REQ_TIMEOUT_NOTIFY     =        0x0292;        //L2周期数据请求超时
const    UINT16    M_CPE_CUR_PERF_REQ_TIMEOUT_NOTIFY       =        0x0293;        //CPE实时数据请求超时



#ifdef WBBU_CODE

const   UINT16    M_BOOT_ONE_Minter_Timer    = 0x350e;


#endif

/*****************************************************************************************
*** UT性能数据                                                                          **/ 
struct T_UT_L2_DACRxPerfData
{
	UINT32  DAC_Sdu;
	UINT32  DAC_Pdu;
	UINT32  ARQ_Block;
	UINT32  ARQ_Block_Duplicate;
	UINT32  ARQ_Block_Out_Of_Window;
	UINT32  ARQ_Block_Crc_Fail;
	UINT32  ARQ_Ack;
	UINT32  ARQ_Ack_Cumulative;
	UINT32  ARQ_Ack_Cumulative_Selective;
	UINT32  DAC_L3_sdu;
	UINT32  L3_pdu;
	UINT32  RSV[4];

}; 

struct T_UT_L2_DACTxPerfData
{
	UINT32  DAC_Sdu;
	UINT32  DAC_Pdu;
	UINT32  ARQ_Block;
	UINT32  ARQ_Retransmit;
	UINT32  ARQ_Ack;
	UINT32  ARQ_Ack_Cumulative;
	UINT32  ARQ_Ack_Cumulative_Selective;	
	UINT32	SDU_DISCARD_BUFF;
	UINT32	SDU_DISCARD_LEN;
	UINT32	L3_pdu;
	UINT32	BW_Req;
	UINT32	RSV[4];

};
struct T_UT_L2_VACPerfData
{
	UINT32	TX_ARQ_VOICE_CTRL;
	UINT32	RX_ARQ_VOICE_CTRL;
	UINT32	TX_PDU_VOICE_DATA;
	UINT32	RX_PDU_VOICE_DATA;
	UINT32	JBUF_PUT_LOST;
	UINT32	JBUF_GET_NULL;
	UINT32	VAC_setup_req;
	UINT32	VAC_setup_confirm;
	UINT32	VAC_setup_Indication;
	UINT32	VAC_setup_response;
	UINT32	VAC_release_Request;
	UINT32	VAC_release_Indication;
	UINT32	RSV[4];
};

struct T_UT_L2PerfData
{
	UINT32		L1_Tx_Word;
	UINT32		L1_Tx_FEC_Block;
	UINT32		L1_Rx_Word;
	UINT32		L1_Rx_FEC_Block;
	UINT32		L1_Rx_FEC_RS_Pass;
	UINT32		L1_Rx_FEC_RS_Recoverable;
	UINT32		L1_Rx_FEC_RS_Fail;

	UINT32		RSV[4];

	UINT32		MAC_Tx_IDLE_Word;
	UINT32		MAC_Rx_IDLE_Word;	
	UINT32		TX_CTRL_MSG;
	UINT32		RX_CTRL_MSG;	
	UINT32		MAC_Random_Access;
	UINT32		MAC_Random_Access_FAIL;
	UINT32		MAC_Paging;
	UINT32		MAC_Paging_Fail;
	UINT32		MAC_Paging_Response;
	UINT32		MAC_paging_no_response;
	UINT32		MAC_Ranging;
	UINT32		MAC_Ranging_Response;

	UINT32		MAC_Broadcast_Data;
	UINT32		MAC_Broadcast_Info;
	UINT32		MAC_BW_Reconfiguration;
	UINT32		MAC_BW_Reconfiguration_Rsp;
	UINT32		MAC_Handoff_Req;
	UINT32		MAC_Handoff_Complete;
	UINT32		MAC_Handoff_Fail;

	UINT32		RSV2[4];

	T_UT_L2_DACRxPerfData		dacRX;
	T_UT_L2_DACTxPerfData       dacTX;
	T_UT_L2_VACPerfData			VacPf;    
};
struct T_UT_L3DATAPerfData
{
	UINT32   NETIngressTraffic;        //    Ingress Traffic when under NET mode                     
	UINT32   NETEgressTraffic;         //    Egress  Traffic when under NET mode                     
	UINT32   LBIngressTraffic;         //    Ingress Traffic when under Learned Brdige mode          
	UINT32   LBEgressBroadcastTraffic; //    Ingress broadcast Traffic when under Learned Brdige mode
	UINT32   LBEgressUnicastTraffic;   //    Ingress unicast Traffic when under Learned Brdige mode  
	UINT32   DisableMessage;           //     Received DISABLE message.                               
	UINT32   EnableMessage;            //     Received ENABLED message                                
	UINT32   DataServiceRequest;       //     Sent Data Service Request.                              
	UINT32   DataServiceResponse;      //     Received Data Service Response        
	UINT32   FilterDenied;
	UINT32   UserNumber;
};

struct T_UT_L3VOICEPerfDataEle
{
	UINT32  Setup_Num;
	UINT32  CallProc_Num;
	UINT32  Alerting_Num;
	UINT32  Connect_Num;
	UINT32  ConnectAck_Num;
	UINT32  Disconnect_Num;
	UINT32  Release_Num;
	UINT32  ReleaseComplete_Num;
	UINT32  ModiMediaReq_Num;
	UINT32  ModiMediaRsp_Num;
	UINT32  Information_Num;
	UINT32  AuthCmdReq_Num;
	UINT32  AuthCmdRsp_Num;
	UINT32  Login_Num;
	UINT32  LoginRsp_Num;
	UINT32  Logout_Num;
	UINT32  HandOverReq_Num;
	UINT32  HandOverRsp_Num;
	UINT32  HandOverComplete_Num;
	UINT32  MOSmsDataReq_Num;
	UINT32  MOSmsDataRsp_Num;
	UINT32  MTSmsDataReq_Num;
	UINT32  MTSmsDataRsp_Num;
	UINT32  SMSMemAvailReq_Num;
	UINT32  SMSMemAvailRsp_Num;
};
struct T_UT_L3VOICEPerfData
{
	T_UT_L3VOICEPerfDataEle  RxPerformanceDATA;
	T_UT_L3VOICEPerfDataEle  TxPerformanceDATA;
};
struct T_UT_L3OAMPerfData
{
	UINT32 uReSyncCount;
	UINT32 uContinueSyncCount;
	UINT32 uSyncFoundCount;
	UINT32 uSyncLostCount;
	UINT32 uProfileUpdateActiveCount;
	UINT32 uProfileUpdateInvalidCount;
	UINT32 uProfileUpdateSuspendCount;
	UINT32 uHandOverCount;
	UINT32 uResetCountChangeCount;
	UINT32 uConfigListCount;
	UINT32 uProbeResponseCount;
	UINT32 uSignalStrengthCount;
	UINT32 uLoadingInfoCount;
};
struct T_UT_L3PerfData        //最长784
{
	//    T_UT_L3OAMPerfData   OAMPerfData;
	T_UT_L3DATAPerfData  DATAPerfData;
	T_UT_L3VOICEPerfData VOICEPerfData;
	T_UT_L3OAMPerfData   OAMPerfData;
};
struct T_UTPerfData
{
	T_UT_L2PerfData tagL2Data;
	T_UT_L3PerfData tagL3Data;
};

/*****************************************************************************************
*** L2性能数据   －－－ 与UT一致                                                        **/        
typedef T_UT_L2PerfData T_L2PerfDataEle;    //BTS UT 二层性能数据一致

#pragma pack(1)
struct T_L2PerfData
{
	UINT16 Type;            
	UINT32 EID;        //与别的消息相比多出此项
	UINT16 Result;    //        0:OK    1:EID not exist
	T_L2PerfDataEle L2PerfDataEle;
};
#pragma pack()


/*****************************************************************************************
*** L3性能数据                                                                            **/        
struct T_L3OAMPerfData      //2Bytes
{
	UINT32  BtsResetCnt;    //    BTS 复位计数
	UINT32  BtsRegCnt;      //    BTS 注册计数
};

struct T_L3DATAEBEle        //4Bytes
{
	UINT32 DHCPPackets;                 
	UINT32 ARPPackets;                  
	UINT32 PPPoEDiscoverystagePackets;  
	UINT32 ToWANPackets;                
	UINT32 ToTDRPackets;                
	UINT32 ToAIPackets;                 
	UINT32 BroadcastToWANPackets;       
	UINT32 BroadcastToTDRPackets;       
	UINT32 BroadcastToAIPackets;        
	UINT32 UnknownPackets;              
	UINT32 NotAuthedPackets;            
	UINT32 BroadcastPacketsbutforbidden;
	UINT32 EgressDHCPClientPackets;     
	UINT32 NoLicense;                   
	UINT32 IllegalUserPackets;          
	UINT32 LostPackets;                 
	
};         
           
struct T_L3DATAEBGenPerfData    //64Bytes
{          
	UINT32 UserMoveaway;
	UINT32 ServingUsers;
	UINT32 ServingAnchorUsers;
	UINT32 ServingButNotAnchorUsers;
	T_L3DATAEBEle From_AI;       
	T_L3DATAEBEle From_WAN;      
	T_L3DATAEBEle From_TDR;      
	T_L3DATAEBEle From_Internal; 
};                           
                             
struct T_L3DATASnoopPerfData
{
	UINT32  InTraffic;                                  //Traffic from EB, to be snooped.              
	UINT32  InTunnelEstablishRequest;                   //Received Tunnel Establish Request     
	UINT32  InTunnelTerminateRequest;                   //Received Tunnel Terminate Request     
	UINT32  InTunnelChangeAnchorRequest;                //Received Tunnel Change Anchor Request 
	UINT32  InTunnelSyncRequest;                        //Received Tunnel Sync Request          
	UINT32  InTunnelEstablishResponse;                  //Received Tunnel Establish Response    
	UINT32  InTunnelTerminateResponse;                  //Received Tunnel Terminate Response    
	UINT32  InTunnelChangeAnchorResponse;               //Received Tunnel Change Anchor Response
	UINT32  InTunnelSyncResponse;                       //Received Tunnel Sync Response         
	UINT32  InRoamRequest;                              //Received Roam Request                 
	UINT32  InDAIBSyncResponse;                         //Received DAIB Synchronize Response    
	UINT32  OutTraffic;                                 //Traffic after snooped, to be forwarded
	UINT32  OutTunnelEstablishRequest;                  //Sent Tunnel Establish Request         
	UINT32  OutTunnelTerminateRequest;                  //Sent Tunnel Terminate Request         
	UINT32  OutTunnelChangeAnchorRequest;               //Sent Tunnel Change Anchor Request     
	UINT32  OutTunnelSyncRequest;                       //Sent Tunnel Sync Request              
	UINT32  OutTunnelEstablishResponse;                 //Sent Tunnel Establish Response        
	UINT32  OutTunnelTerminateResponse;                 //Sent Tunnel Terminate Response        
	UINT32  OutTunnelChangeAnchorResponse;              //Sent Tunnel Change Anchor Response    
	UINT32  OutTunnelSyncResponse;                      //Sent Tunnel Sync Response             
	UINT32  OutDAIBSyncRequest;                         //Sent DAIB Synchronize Request         
};

struct T_L3DATATunnelPerfData
{
	UINT32   TxEstablishReq; //    Num of  Sent Tunnel Esatablish Request            
	UINT32   TxTerminateReq; //    Num of  Sent Tunnel Terminate Request             
	UINT32   TxSyncReq;      //    Num of  Sent Tunnel Sync Request                      
	UINT32   TxChgAnchorReq; //    Num of  Sent Tunnel Change Anchor Request         
	UINT32   RxEstablishResp;//    Num of  Received Tunnel Esatablish Response       
	UINT32   RxTerminateResp;//    Num of  Received Tunnel Terminate Response    
	UINT32   RxSyncResp;     //    Num of  Received Tunnel Sync Response                 
	UINT32   RxChgAnchorResp;//    Num of  Received Tunnel Change Anchor Response
	UINT32   RxEstablishReq; //    Num of  Received Tunnel Esatablish Request        
	UINT32   RxTerminateReq; //    Num of  Received Tunnel Terminate Request         
	UINT32   RxSyncReq;      //    Num of  Received Tunnel Sync Request                  
	UINT32   RxChgAnchorReq; //    Num of  Received Tunnel Change Anchor Request     
	UINT32   TxEstablishResp;//    Num of  Sent Tunnel Esatablish Response           
	UINT32   TxTerminateResp;//    Num of  Sent Tunnel Terminate Response        
	UINT32   TxSyncResp;     //    Num of  Sent Tunnel Sync Response                     
	UINT32   TxChgAnchorResp;//    Num of  Sent Tunnel Change Anchor Response    
};  

struct T_L3DATATDRPerfData
{
	UINT32   SocketError;           //Socket Error counter.    
	UINT32   RxEtherIpPackets;      //Received EtherIp Packets.
	UINT32   BufferExhausted;       //Buffer Pool exhausted    
};

struct T_L3DATAEBEle2        //4Bytes
{
	UINT32 ARP_TO_AI;        //Numbers of ARP Request To AI     
	UINT32 ARP_TO_WAN;       //Numbers of ARP Request To WAN      
	UINT32 ARP_TO_TDR;       //Numbers of ARP Request To TDR    
	UINT32 ARP_TO_UNKNOW;    //Numbers of ARP Request to UNKNOW 
	UINT32 ARP_REPLY;        //Numbers of ARP Reply             
};
struct T_L3DATAARPPerfData
{
	T_L3DATAEBEle2 From_AI; 
	T_L3DATAEBEle2 From_WAN;
	T_L3DATAEBEle2 From_TDR;
};    


struct T_L3DATATCRPerfData
{
	UINT32   SocketError;           //Socket Error counter.
	UINT32   RxUDPPackets;          //Received UDP Packets.
};    


struct T_L3DATADMPerfData
{
	UINT32   EidNum;               //CPE Index                                       
	UINT32   StartT;               //The time of creating CPE                        
	UINT32   DATA_SERVICE_REQ;     //Num of Received data service request            
	UINT32   DATA_SERVICE_RSP;     //Num of send dataservice reponse                 
	UINT32   SEND_SYNC_ACL_REQ;    //Num of send synchorize ACL table Request        
	UINT32   RECV_SYNC_ACL_RSP;    //Num of receive synchorize ACL table Response    
	UINT32   SEND_SYNC_IP_REQ;     //Num of send synchorize Iplist table Request     
	UINT32   RECV_SYNC_IP_RSP;     //Num of receive synchorize Iplist table Response
	UINT32   SYNC_ACL_FAIL;        //Num of synchorize ACL TimerExpire              
	UINT32   SYNC_IP_FAIL;         //Num of synchorize Iplist TimerExpire           
	UINT32   CPE_CONIFG;           //Num of received CPE confige                    
};    

struct T_L3DATAPerfData
{
	T_L3DATAEBGenPerfData   EBGenPerfData;
	T_L3DATASnoopPerfData   SnoopPerfData;
	T_L3DATATunnelPerfData  TunnelPerfData;
	T_L3DATATDRPerfData     TDRPerfData;
	T_L3DATAARPPerfData     ARPPerfData;
	T_L3DATATCRPerfData     TCRPerfData;
	T_L3DATADMPerfData      DMPerfData;
};

struct T_L3VOICEPerfDataEle
{
	UINT32  LAPaging_Num;
	UINT32  LAPagingRsp_Num;
	UINT32  DELAPagingReq_Num;
	UINT32  DELAPagingRsp_Num;
	UINT32  AssignResReq_Num;
	UINT32  AssignResRsp_Num;
	UINT32  RlsResReq_Num;
	UINT32  RlsResRsp_Num;
	UINT32  Reset_Num;
	UINT32  ResetAck_Num;
	UINT32  BeatHeart_Num;
	UINT32  BeatHeartAck_Num;
	UINT32  CongestionCtrlReq_Num;
	UINT32  CongestionCtrlRsp_Num;
	UINT32  ErrNotifyReq_Num;
	UINT32  ErrNotifyRsp_Num;
	UINT32  Setup_Num;
	UINT32  CallProc_Num;
	UINT32  Alerting_Num;
	UINT32  Connect_Num;
	UINT32  ConnectAck_Num;
	UINT32  Disconnect_Num;
	UINT32  Release_Num;
	UINT32  ReleaseComplete_Num;
	UINT32  ModiMediaReq_Num;
	UINT32  ModiMediaRsp_Num;
	UINT32  Information_Num;
	UINT32  AuthCmdReq_Num;
	UINT32  AuthCmdRsp_Num;
	UINT32  Login_Num;
	UINT32  LoginRsp_Num;
	UINT32  Logout_Num;
	UINT32  HandOverReq_Num;
	UINT32  HandOverRsp_Num;
	UINT32  HandOverComplete_Num;
	UINT32  StatusQry_Num;
	UINT32  Status_Num;
	UINT32  MOSmsDataReq_Num;
	UINT32  MOSmsDataRsp_Num;
	UINT32  MTSmsDataReq_Num;
	UINT32  MTSmsDataRsp_Num;
	UINT32  SMSMemAvailReq_Num;
	UINT32  SMSMemAvailRsp_Num;
	UINT32  UTSAG_L3Addr_Num;
	UINT32  UTSAG_UID_Num;
	UINT32  InvalidSignal_Num;
};
struct T_L3VOICEPerfData
{
	T_L3VOICEPerfDataEle  RxPerformanceDATA;
	T_L3VOICEPerfDataEle  TxPerformanceDATA;
};

struct T_L3PerfData        //最长864
{
	T_L3DATAPerfData  DATAPerfData;
	T_L3VOICEPerfData VOICEPerfData;
	T_L3OAMPerfData   OAMPerfData;        //发送消息获得的
};

//new stat
#pragma pack(1)
struct T_L3PerfData_new
{
    UINT16 tag;
    UINT16 length;
    UINT32 reguser;//注册在某基站上的用户数的当前值,eb模块
    UINT32 dataflow[2];//统计时间内单基站下用户使用数据业务产生费用得总流量
    UINT32 datatime[2];//统计时间内单基站下用户使用数据业务的在线时长    
    UINT32 dataToWan;//基站WAN口发送吞吐率，不包括协议栈数据包
    UINT32 dataFromWan;//基站WAN口接收吞吐率，不包括协议栈数据包    
    UINT32 dataToTDR;//基站向所有隧道的发送吞吐率
    UINT32 dataFromTDR;//基站从所有隧道的接收吞吐率
    UINT32 M_SAGHeartTimeoutNum;//基站向主SAG发送握手请求之后，在握手间隔时间内都没有接收到握手应答，计为握手超时一次
    UINT32 M_SAGHeartTimelongAvg;//基站向主SAG发送握手请求之后，在握手间隔时间内接收到握手应答，基站计算握手请求发送与握手应答接收的时延
    UINT32 S_SAGHeartTimeoutNum;//基站向备SAG发送握手请求之后，在握手间隔时间内都没有接收到握手应答，计为握手超时一次
    UINT32 S_SAGHeartTimelongAvg;//基站向备SAG发送握手请求之后，在握手间隔时间内接收到握手应答，基站计算握手请求发送与握手应答接收的时延
};
#pragma pack()
#endif