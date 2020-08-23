/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: l3oamcfgcommon.h
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMCFGCOMMON
#define _INC_L3OAMCFGCOMMON

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#pragma pack(1)
/////////////////////////////////////////////////////
struct T_BtsGDataCfgEle
{   //data ok 2005-09-07
    UINT32 BtsIPAddr;
    UINT32 DefGateway;
    UINT32 SubnetMask;
    UINT32 SAGID;
    UINT32 SAGVoiceIP;
    UINT32 SAGSignalIP;
    UINT16 SAGRxPortV;          //UDP port used to communicate with SAG Voice Rx
    UINT16 SAGTxPortV;          //UDP port used to communicate with SAG Voice Tx
    UINT16 SAGRxPortS;          //UDP port used to communicate with SAG Signal  Rx
    UINT16 SAGTxPortS;          //UDP port used to communicate with SAG Signal  Tx
    UINT32 LocAreaID;
    UINT16 SAGSPC;//SAGsignal point code    2       
    UINT16 BTSSPC;//BTS signal point code   2       
    UINT32 EmsIPAddr;
    UINT16 NetworkID;
    UINT16 BtsBootSource;
    UINT8   NatAPKey;
    UINT8   SAGVlanUsage;    // 0 - no use 1 - usage
    UINT16  SAGVlanID;         // SAG Vlan ID(0 ~ 65535)
    #ifndef NUCLEAR_CODE
    UINT16  reserved;
	#else
	UINT8 bLimited;
	UINT8 reserved;
	#endif
};

///////////////////////////////////////////////////////
struct T_DataServiceCfgEle
{   //data ok 2005-09-07
    UINT32  RoutingAreaID;          
    UINT8   Mobility;       //  0 - disabled; 1 - enabled
    UINT8   AccessControl;  //  0 - disabled; 1-enabled (should be 1 if mobility is enabled)
    UINT16  LBATimerLen;    //      Learned bridge aging timer length , 1800 seconds    
    UINT8   P2PBridging;    //  0 - disabled;  1-enabled 允许ut不经过router通信
    UINT8   EgrARPRroxy;    //  0 - disabled; 1 - enabled (should be 1 if mobility is enabled)，Egress指router到ut，BTS缓存ut上设备的mac
    UINT8   EgrBCFilter;    //  0 - disabled; 1 - enabled (discard broadcast packets other then ARP and DHCP packets)
    UINT16  PPPSessionLen;  //  PPP session keep alive timer length,    30seconds   
    UINT8   TargetBTSID;       //  as agent circuit ID sub-option   1   0   0 -disabled; 1-enabled 
    UINT8   TargetEID;         //  as agent remote ID sub-option    1   0   0-disabled; 1--enabled
    UINT8   TargetPPPoEEID;    //  as agent remote ID sub-option    1   0   0-disabled; 1--enabled
};

struct T_SnoopDataServiceCfgEle
{
    UINT8   TargetBTSID;       //  as agent circuit dhcp ID sub-option   1   0   0 -disabled; 1-enabled 
    UINT8   TargetEID;         //  as agent remote dhcp  ID sub-option    1   0   0-disabled; 1--enabled
	UINT8   TargetPPPoEEID;    //  as agent remote pppoe ID sub-option    1   0   0-disabled; 1--enabled
	UINT32  RoutingAreaID;
}; 

struct T_DmDataServiceCfgEle
{
    UINT8   Mobility;           //0 - disabled; 1 - enabled
    UINT8   AccessControl;      //0 - disabled; 1-enabled 
    UINT16  LBATimerLen;        //1800 seconds  
};

struct T_ArpDataServiceCfgEle
{
    UINT8   P2PBridging;        //0 - disabled;  1-enabled 
    UINT8   EgrARPRroxy;        //0 - disabled; 1 - enabled
};

struct T_EbDataServiceCfgEle
{  
    UINT8   EgrBCFilter;        //0 - disabled; 1 - enabled
    UINT16  LBATimerLen;        //1800 seconds  
    UINT16  PPPSessionLen;      //alive timer length,    30seconds
    UINT8   AccessControl;      //0 - disabled; 1-enabled     
};

struct T_VlanGroupPAIR
{
    UINT16 usGroupID;
    UINT16 usVlanID;    
};

#define M_VLAN_GROUP_MAX    (200)
struct T_VlanGroupCfgEle
{
    UINT16 number;
    T_VlanGroupPAIR group[M_VLAN_GROUP_MAX];
};

//////////////////////////////////////////////////
typedef struct 
{               
    UINT8  RRCHSCGIndex;    //      0xff is invalid
    UINT8  RRCHTSIndex;     //      0xff is invalid
}T_RRCHInfor;

typedef struct 
{               
    UINT8  BCHSCGIndex;     //   0xff is invalid
    UINT8  BCHTSIndex;      //    0xff is invalid
}T_BCHInfor;

typedef struct 
{               
    UINT8  RACHPairSCGIndex;//RACH Pair SCG index   1       0xff is invalid
    UINT8  RACHPairTSIndex;//RACH Pair TS index 1       0xff is invalid
}T_RACHPairInfor;           

typedef struct 
{               
    UINT16 BCHScale;  // BCH_SCALE          
    UINT16 TCHScale;  
}T_ScaleInfor;          
struct T_W0Info
{
    SINT16 WI;
    SINT16 WQ;
}; 
struct T_N_Parameter
{
    UINT16  N_Algorithm_Switch;
    UINT16  Ci_Jump_detection;
    UINT16  UT_PowerLock_Timer_Th;
    UINT16  N_para[7];
};
struct T_AirLinkCfgEle
{
    UINT8  SequenceID;   // 0~6 For N=1(N equals one)
    UINT8  SubCGrpMask;  //Sub-carrier group Mask   1       
    UINT8  TimeSlotNum;  // 8 for 10ms TDD     4 for 5ms TDD    
    UINT8  DLTSNum;      // 1~Total time slot number    Time slot number used for downlink
    T_BCHInfor  BCHInfo[BCH_INFO_NUM];
    T_RRCHInfor RRCHInfor[RRCH_INFO_NUM];
    T_RACHPairInfor RACHPairInfor[RACH_RARCH_CFG_NUM]; 
    UINT16 MaxScale;
    UINT16 PremScale; // REAMBLE_SCALE          
    T_ScaleInfor CHScale[DL_TIME_SLOT_NUM];  // First N effective，N=downlink time slot number
    T_W0Info W0Info[8]; 
    UINT16 forbid_ts_mask[SCG_NUM];
};

struct T_BtsNeighborCfgChInfo
{
    T_BCHInfor  BCHInfo[BCH_INFO_NUM];
    T_RRCHInfor RRCHInfor[RRCH_INFO_NUM];
    T_RACHPairInfor RACHPairInfor[RACH_RARCH_CFG_NUM]; 
};
struct T_BtsInfoIE
{
    UINT32 BTSID;
    UINT16 FrequencyIndex;      
    UINT8  SequenceID;           //0~6  For N=1(N equals one)
    UINT8  SubcarrierGroupMask;     
    T_BtsNeighborCfgChInfo BtsNeighborCfgChInfo;
    UINT8  N_ANT;         //天线 L1 General
    SINT8  TRANSMIT_PWR;  //    发射功率
    UINT8  N_TS;          //    时隙
    UINT8  N_DN_TS;       //    下行时隙
    UINT8  RECEIVE_SENSITIVITY;         
    UINT8  MAX_SCALE;           
    UINT8  PREAMBLE_SCALE;          
    UINT8  TCH_SCALE0;          // 0 - not used
    UINT8  TCH_SCALE1;          // 0 - not used
    UINT8  TCH_SCALE2;          // 0 - not used
    UINT8  TCH_SCALE3;          // 0 - not used
    UINT8  TCH_SCALE4;          // 0 - not used
    UINT8  TCH_SCALE5;          // 0 - not used
    UINT8  TCH_SCALE6;          // 0 - not used
};
//////////////////////////////////@@@@@@@@@@@@@@@@@@@@@@@@@@@ FOR CPE BEGIN
typedef struct 
{               
    UINT8  BCHSCGIndex:5;   //   0xff is invalid
    UINT8  BCHTSIndex:3;    //    0xff is invalid
}T_BCHInforForCPE;

typedef struct 
{               
    UINT8  RRCHSCGIndex:5;  //      0xff is invalid
    UINT8  RRCHTSIndex:3;   //      0xff is invalid
}T_RRCHInforForCPE;

typedef struct 
{               
    UINT8  RACHPairSCGIndex:5;//RACH Pair SCG index 1       0xff is invalid
    UINT8  RACHPairTSIndex:3;//RACH Pair TS index   1       0xff is invalid
}T_RACHPairInforForCPE;         

struct T_BtsNeighborCfgChInfoForCPE
{
    T_BCHInforForCPE      BCHInfo[BCH_INFO_NUM];
    T_RRCHInforForCPE     RRCHInfor[RRCH_INFO_NUM];
    T_RACHPairInforForCPE RACHPairInfor[RACH_RARCH_CFG_NUM]; 
};

struct T_BtsInfoIEForCPE
{
    UINT32 BTSID;
    UINT16 FrequencyIndex;      
    UINT8  SequenceID;           //0~6  For N=1(N equals one)
    UINT8  SubcarrierGroupMask;     
    T_BtsNeighborCfgChInfoForCPE BtsNeighborCfgChInfo;
    UINT8  N_ANT;         //天线 L1 General
    SINT8  TRANSMIT_PWR;  //    发射功率
    UINT8  N_TS;          //    时隙
    UINT8  N_DN_TS;       //    下行时隙
    UINT8  RECEIVE_SENSITIVITY;         
    UINT8  MAX_SCALE;           
    UINT8  PREAMBLE_SCALE;          
    UINT8  TCH_SCALE0;          // 0 - not used
    UINT8  TCH_SCALE1;          // 0 - not used
    UINT8  TCH_SCALE2;          // 0 - not used
    UINT8  TCH_SCALE3;          // 0 - not used
    UINT8  TCH_SCALE4;          // 0 - not used
    UINT8  TCH_SCALE5;          // 0 - not used
    UINT8  TCH_SCALE6;          // 0 - not used
};
//////////////////////////////////@@@@@@@@@@@@@@@@@@@@@@@@@@@ FOR CPE BEGIN

struct T_BtsNeighborCfgData  //按最大长度定义
{
    UINT32 BtsIP;   //by xiaoweifang.
    T_BtsInfoIE BtsInfoIE;
    UINT16 RepeaterNumber;      
    UINT16 RepeaterStartFreq[NEIGHBOR_BTS_NUM];     

    //该neighborBTS占用的数据长度:-1 错误
    UINT16 length()
        {
        if (RepeaterNumber > NEIGHBOR_BTS_NUM)
            {
            return 0xFFFF;
            }
        return sizeof(BtsIP)+sizeof(BtsInfoIE)+sizeof(RepeaterNumber)+RepeaterNumber*sizeof(UINT16);
        }
};

struct T_BtsNeighborCfgEle   //按最大长度定义
{ 
    UINT16  NeighborBtsNum;       // 0 -- 20  Neighbor BTS number
    T_BtsNeighborCfgData BtsNeighborCfgData[NEIGHBOR_BTS_NUM];
};

//5.2.13    BTS Load Info (BTS->EMS)
struct T_BTSLoadInfo
{
    UINT16 CurrentUserNumber;       
    UINT16 AverageFreeDLchannel;        
    UINT16 AverageFreeULchannel;        
    UINT16 AverageFreePowerUsage;
	UINT16 WeightedUserNo;
    UINT32 Reserve;
};
            
//5.2.14    Neighbot BTS load info (EMS->BTS)
struct T_NeighbotBTSLoadInfo
{           
    UINT32 BTSID;   
    T_BTSLoadInfo BTSLoadInfo;
};          

struct T_NeighbotBTSLoadInfoEle
{
    UINT16 NeighborBTSNum;  
    T_NeighbotBTSLoadInfo NeighbotBTSLoadInfo[NEIGHBOR_BTS_NUM];
};          

struct T_BTSRepeaterEle   //最大配置
{
    UINT16 RepeaterFreqNum; 
    UINT16 RepeaterFreqInd[NEIGHBOR_BTS_NUM];
};          


///////////////////////////////////////////////////
struct T_UTSDCfgInfo
{
    UINT8  Class;         // 0 -- 7
    UINT8  Reserved;     // for 16bit align
    UINT16 ULMaxBW;
    UINT16 ULMinBW;
    UINT16 DLMaxBW;
    UINT16 DLMinBW;
};

struct T_UTSDCfgEle
{
    T_UTSDCfgInfo Info;
};


///////////////////////////////////////////////////
struct T_ToSCfgEle
{
    UINT8  SFMapping; // 0:High Priority Data   1:Low Priority Data
};

struct T_CpeToSCfgEle
{
    UINT8  TOSValue;
    UINT8  Priority; // 0:High Priority Data   1:Low Priority Data
};
///////////////////////////////////////////////////////
struct T_PerfLogCfgEle
{
    SINT32 FTPserverIP;
    UINT16 FTPserverPort;
    UINT8  UserNameLen;
    SINT8  UserName[FILE_NAME_LEN];
    UINT8  FTPPasswordLen;
    SINT8  FTPPassword[USER_PASSWORD_LEN];
    UINT16 PerfRptInter;    //  0 - do not report    15 minutes    30 minutes   60 minutes
    UINT16 PerfCollectInter;//  0 - do not collect   15/30/60 minutes
};


////////////////////////////////////////////////////////////
struct T_TempAlarmCfgEle
{
    SINT16  AlarmH;     //Alarm high temperature     //有符号数   
    SINT16  AlarmL;     //Alarm low temperature     
    SINT16  ShutdownH;  //Shutdown high temperature     
    SINT16  ShutdownL;  //Shutdown low temperature      
};



/////////////////////////////////////////////////////////
struct T_GpsDataCfgEle
{
    SINT32  Latitude;       
    SINT32  Longitude;      
    SINT32  Height;     
    SINT32  GMTOffset;      
    UINT8   SatelliteCnt;  //Minimum Tracking satellite #   1       Default = 3
};

/////////////////////////////////////////////////////////
struct T_GpsDataNotifyEle
{
    UINT32  Latitude;       
    UINT32  Longitude;      
    UINT32  Height;     
};


//////////////////////////////////////////////////////////////
struct T_BtsRstEle
{
    UINT16 DataSource; // 0 - BTS   1-EMS   
};

struct T_ACLCfgEleInfo
{
    UINT8   Protocol;    // 
    UINT32  SourceIp;    // 
    UINT32  Srcwildcard; // ACL wildcard
    UINT16  SrcPort;     // 
    UINT8   SrcOperate;  // 0:eq  1:neq  2:lt  3:gt
    UINT32  DestIp;      // 
    UINT32  Destwildcard;// ACL wildcard
    UINT16  DestPort;    // 
    UINT8   DestOperate; // 0:eq  1:neq  2:lt  3:gt
    UINT8   Permit;      // 0:Dely  1:Permit
};

struct T_ACLCfgEle
{   
    UINT8 Type;
    UINT8 TotalACLEnt;
    T_ACLCfgEleInfo ACLCfgEleInfo[MAX_ACL_CFG_SIZE]; 
};

struct T_SFIDCfgEle
{
    UINT8 SFID[MAX_SFID_CFG_CNT];
};



struct T_RMPoliceEle
{
    UINT16  BWReqInterval;    //  Bandwidth request interval            In the unit of 5ms
    UINT16  SRelThreshold;    //  Session release threshold         In the unit of 5ms
    SINT16  MinULSS;          //  Sustained Min UL signal strength      -30~0   Dbm
    UINT16  MaxDLPPUser;      //  Max DL power per user(%)      0~100   
    UINT16  DLBWPerUser;      //  Sustained DL BW per user          Kbps
    UINT16  ULBWPerUser;      //  Sustained UL BW per user          Kbps
    UINT16  RsvTCH;           //  Reserved TCH for access       0~320   
    UINT16  RsvPower;         //  Reserved Power for PC (%)     0~100   Power Control
    UINT16  PCRange;          //  PC range      0~19    Default = 19
    UINT16  StepSize;         //  Reassignment step size            Kbps,default=128,unit=4k
    UINT16  MaxUser;          //  Max simultaneous user     1~320   Default = 320
    UINT8   BWDistClass[DL_TIME_SLOT_NUM];      //  0~100   %
    UINT8   ULModMask;   // UL modulation mask  Bit 0 - QPSK    Bit 1 - 8PSK  Bit 2 - QAM16  Bit 3 - QAM32  Bit 4 - QAM64  
    UINT8   DLModMask;   // DL modulation mask  Same as above
    
};

/////////////////////////////////////////////////////////////////
struct T_AirLinkMisCfgEle
{
    SINT8  SSTSync;           //  Signal strength threshold for sync;dBm    Handover Threshold
    UINT8  SNRTLeave;         //  SNR threshold for leaving         Same as above
    UINT8  SNRTEnter;         //  SNR threshold for entering    ;       Same as above
    UINT8  BTSLTLeave;        //  BTS load threshold for leaving    ;       Same as above
    UINT8  BTSLTLEnter;       //  BTS load threshold for entering   ;       Same as above
    UINT8  WakeupInterval;    //  Sleep mode wakeup interval    ;       In the unit of frames
};


///////////////////////////////////////////////////////////
struct T_BillDataCfgEle
{
    UINT16  UploadInter;  //  Billing record upload interval        In seconds
};


//////////////////////////////////////////////////////
//T_L1GenCfgReq 于 2005-09-07按文档修改成如下:
struct T_L1GenCfgEle
{   
    UINT16  SyncSrc;     //00：BTS本地同步,01：GPS同步,02：E1同步,03：光口同步,04：1588同步,05：485同步
    UINT16  GpsOffset;   //         
    UINT16  AntennaMask; //  0-disable, 1-enable       LSB is antenna 0
};

//5.4.2 RF Configuration（EMS）
struct T_PSNorm
{
    UINT16 PS_Norm_X;
    UINT16 PS_Norm_Y;
};

struct T_RfCfgEle
{
    UINT32 StartFreqIndex;  //  32  Unit 50K ;start Freq Index use base of band ID passing from AUX,
                            //  After AUX receive this, it will do floor((start Freq index-PLLmin)/PLLstep), and look up the PLL table to do the configuration
    SINT16 AntennaPower;    //  16  Typical range from -5dbm to 35 dbm;
    SINT16 RxSensitivity;   //  16  Typical range from -90dbm to -65 dbm;
    UINT16 Cable_Loss;      //  16  
    UINT16 PS_Loss;         //  16  Power splitter loss, typical 30db
    T_PSNorm PS_NormS[ANTENNA_NUM];   //    16  Phase difference between RF chan to SYN cal port and RF chan to antenna port
};

//5.4.3 Calibration General (EMS)
//配置命令中是含8个结构的数组
struct T_CaliGenCfgAnnetaInfo
{
    UINT16 PRED_H_calibration_result[PRED_H_WORD_CNT];       //24 *8 bit   8 antenna
    UINT16 TX_GAIN;     
    UINT16 RX_GAIN;     
};

//5.4.4 Calibration CalData (EMS) 和
//5.4.6.2   Calibration Result CalData Notification（BTS）是同一个结构
typedef struct
{
    UINT16 Type;  // 1:TXCAL_I  2:TXCAL_Q  3:RXCAL_I  4:RXCAL_Q
    UINT16 AntennalIndex;  //   0~7
    UINT16 SCStartIndex;
    UINT16 SC;   //SubCarrier num = 640     
    UINT16 SubCarrierCalData[SUB_CARRIER_NUM];/*For (i=SC starting index;i<start+#SC;i++)
                                {       
                                CAL data for Subcarrieri 2
                                }   */
} T_CaliDataEle, T_CaliResultNotifyEle;


//5.4.6 Calibration Result Notification
//5.4.6.1   Calibration Result General Notification（BTS）
struct T_CaliGenCfgEle
{
    T_CaliGenCfgAnnetaInfo  AnnetaInfo[ANTENNA_NUM];
    UINT16 SynTxGain;       
    UINT16 SynRxGain;       
};

struct T_CalibrationError
{
    UINT16     CaliAntErr[ANTENNA_NUM];  // 0 -- ok, other else is error
    UINT16     SYNError : 6;             // 0 -- ok, other else is error
    UINT16     PreDistortErr: 1;         // 0 -- ok, other else is error
    UINT16     PreCalibrationErr:2;      // 0 -- ok, other else is error
    UINT16     CalibrationRsv: 7;        // 0 -- ok, other else is error
    UINT16     calCorrFlag;              // 1 -- OK, other else is error.校准结果是否正确,不包含板不在位的情况
};

struct T_CaliResultGenNotifyEle
{
    T_CaliGenCfgEle     GenCfgEle; 
    T_CalibrationError  CalibrationError; 
};

//////////////////////////////////////////////////////////
struct T_CalCfgEle
{
    UINT16  CalIner;  //  Periodic calibration interval 2   0, 30~ 1440     In minutes, 0 is to disable periodic calibration
    UINT16  CalType;  //  Periodic calibration type 2   0 - online 1 -- full    
};

struct T_InstantCalEle
{
    UINT16  CalType:1;      //  0 - online   1 -- full  
    UINT16  CalTrigger:1;   //  0 - manual   1 -- period
    UINT16  rsv:14;
};

struct T_BTSCommonDataEle
{
    UINT16 BtsRstCnt;
    UINT32 BtsRegCnt;
    UINT16 InitFromNvramFailCnt;
    UINT16 InitFromEmsFailCnt;
    UINT32 BtsBootPlane;
};


struct T_BTSUserCfgEle
{
    SINT8  user[40];
    SINT8  password[40];
};
#ifdef WBBU_CODE
#pragma pack(1)
struct T_WRRUDataEle
{
    unsigned int  IsValid;
     unsigned int FPGA_Version;
     unsigned int StartFreqindex;
     unsigned char TDD_timeslot;
     unsigned char DLSTNum;
     unsigned short TDD10ms_Sel; //10ms source select  暂时不用
     ////////Set SYN & RF
     unsigned char RXGAIN_RFB[8];//射频增益
     unsigned char TXGAIN_RFB[8];
     unsigned short RXGAIN_SYN;//SYN增益
     unsigned short TXGAIN_SYN;
     unsigned short count_30M_10s;//频偏，计算TcoOffset
     unsigned short TRSYN_Power_SW;//SYN&RF Power ON/OFF	
     ///////////////threshold
      char Max_Temp; 
     char Min_Temp; 
   unsigned  char Max_current;
   unsigned   char Min_current;
     ////////////////not in use currently	
     unsigned short DbgCtrl;
	//unsigned short RSV;

};
#pragma pack()
#endif
#pragma pack(1)
struct T_UtHandoverPara
{
    UINT8 M_HO_PWR_THD;
    UINT8 M_HO_PWR_OFFSET1;
    UINT8 M_HO_PWR_OFFSET2;    
    UINT8 M_HO_PWR_FILTERCOEF_STAT;
    UINT8 M_HO_PWR_FILTERCOEF_MOBILE;
    UINT8 TIME_TO_TRIGGER;
    UINT16 M_CPE_CM_HO_PERIOD;
    UINT16 write_flag;
};
struct T_UtHandoverPara2
{
    UINT8 StrictArea_Pwr_THD;
    UINT8 StrictArea_TIME_TO_TRIGGER;
    UINT8 StrictArea_HO_PWR_OFFSET;
	UINT8 reserve;
    UINT16 write_flag;
};
struct T_CLUSTER_PARA
{
    UINT16 flag;
    UINT16 sleep_period;
    UINT16 Rsv_Ch_Resourse_Num;
    UINT16 rsv1;//used for 6.2.1.2.2.39　集群功能及参数配置请求<McWiLL 46.02-V3.0 McWiLL V6 BS-EMS接口技术规范>
    UINT16 rsv2;
    UINT16 rsv3;    
    UINT16 cfg_flag;//是否配置标识,配置后为0x5a5a;
};
#pragma pack()
#pragma pack(1)
struct T_RangingPara
{    
    UINT8 Ranging_Switch;
    UINT8 Enable_Shreashold;
    UINT8 Disable_Shreahold;    
    UINT8 Ratio_Shreahold;
    UINT8 SNR_Shreahold;
    UINT8 Ranging_Offset_Shreahold;    
    UINT16 cfg_flag;//是否配置标识,配置后为0x5a5a;
};
#pragma pack()

//#pragma pack(1) 
struct T_SagBkp 
{     
        UINT32 ulSagID; 
        UINT32 ulSagVoiceAddr; 
        UINT32 ulSagSignalAddr; 
        UINT16 usVoiceRxPort; 
        UINT16 usVoiceTxPort; 
        UINT16 usSignalRxPort; 
        UINT16 usSignalTxPort; 
        UINT32 ulLocationAreaID; 
        UINT16 ulSagSignalPointCode; 
        UINT16 ulBtsSignalPointCode; 
        UINT8 ucRsv[8]; 
        UINT8 NatAPKey; 
}; 
struct T_JitterBuf 
{             
        UINT16 usJtrBufEnable; 
        UINT16 usJtrBufZEnable; 
        UINT16 usJtrBufLength; 
        UINT16 usJtrBufPackMax; 
}; 
struct T_SagTos 
{             
        UINT8  ucSagTosVoice; 
}; 

struct T_SavePwrCfg 
{             
        UINT16 SavePwrFlag; 
        UINT16 TS1Channel; 
        UINT16 TS1User; 
        UINT16 TS2Channel; 
        UINT16 TS2User; 
        SINT16 Fan1; //有符号数
        SINT16 Fan2; 
        SINT16 Fan3; 
        SINT16 Fan4; 
}; 
struct T_Qam64Cfg 
{             
        UINT8 Qam64Ppc0;
		UINT8 Qam4ForQam64;
}; 
struct T_AlarmFlag
{
    UINT16 alarmFlag;
    UINT16 cfgFlag;
};

	struct T_PayloadBalanceCfg
	{
		UINT16 usFlag;
		UINT16 usLi;
		UINT16 usPeriod;
		UINT16 usLd;
		UINT16 usCount;
		UINT16 usSignal;
		UINT16 usLdPeriod;
		UINT16 usParam;
	};
#ifdef PAYLOAD_BALANCE_2ND
#define BANDSWITCHFLAG (0x5a5a);
	struct T_PayloadBalanceCfg2nd
	{
		UINT16 usFlag;
		UINT16 usLi;
		UINT16 usPeriod;
		UINT16 usLd;
		UINT16 usCount;
		UINT16 usSignal;
		UINT16 usLdPeriod;
		UINT16 usParam;
        UINT16 usBandSwitch;
	};
#endif

//#ifdef LJF_RPT_ALTER_2_NVRAMLIST
#define M_RPT_LIST_FLAG 	(0x81E29B7A)
#define M_RPT_LIST_MAX (10)

//20091101 add by fengbing for localSAG begin
#ifndef M_LOCALSAG_CFG_VALID_FLAG
#define M_LOCALSAG_CFG_VALID_FLAG 	(0xA7B92E18)
#endif
struct T_localSagCfg
{
	UINT8 blUseLocalSag;
	UINT8 blUseUserListFile;
	char szAreaCode[10];
	UINT8 dialPlanBuf[120];
	UINT8 maxGrpIdleTime[2];//组呼空闲时长	2	M		默认值5s，0表示不使用
	UINT8 maxGrpTalkTime[2];//讲话方最大时长	2	M		默认值5s，0表示不使用
	UINT8 maxGrpAliveTime[2];//组呼最大时长	2	M		默认值0，0表示不使用
	UINT8 GrpLePagingLoopTime[2];//迟后进入周期	2	M		默认值5s，0表示不使用
	UINT8 blUseLocalUserInfoFile;//是否使用内部用户信息文件定义的用户信息
	UINT8 blUseLocalGrpInfoFile;//是否使用内部组信息文件定义的组信息
	UINT8 validFlag[4];//muset be M_LOCALSAG_CFG_VALID_FLAG after configuation
};
//20091101 add by fengbing for localSAG end

#define M_DCS_CFG_VALID_FLAG 	(0xC7A90E3F)
struct T_DcsCfg
{
	UINT8 DCS_IP[4];
	UINT8 DCS_Port[2];
	UINT8 BTS_Port[2];
	UINT8 NatApKey;	
};
struct T_DcsCfgBuffer
{
	T_DcsCfg dcsCfg;
	UINT8 validFlag[4];//muset be M_DCS_CFG_VALID_FLAG after configuation	
};

#ifdef RCPE_SWITCH
#define M_TRUNK_MRCPE_FLAG (0x9BDF)
#define M_TRUNK_MRCPE_MAXNUM (10)
typedef struct T_TRUNKMRCPE
{
	UINT16 usflag;
	UINT16 usNum;
	UINT32 aulMRCpe[M_TRUNK_MRCPE_MAXNUM];
}TrunkMRCpeRcd;
#endif
struct T_STime_CFG
{             
       UINT32 valid;
       UINT32 T_server_ip;
	UINT16 T_server_port;
	UINT16 STime_period;
}; 
#ifndef WBBU_CODE

  struct T_Multicast_CFG
{
     UINT32 valid;/*****0x87654321 为valid******/
     UINT32  flag;/*******0x55-open,0xaa-close,other-close*************/
};

#endif
#ifdef WBBU_CODE
#pragma pack(1)
struct T_CalHardWare_Para
{    
    UINT32  IsValid;
    char Para[1200];
};
#pragma pack()
#endif
#define M_WAKEUP_CONFIG_FLAG 	(0x5a5a)
struct T_WAKEUP_CONFIG
{
    UINT16  usFlag;
    UINT16  usSwitch;   // 0表示关闭强制唤醒功能； 1表示打开。
    UINT32  ulPeriod;  //强制唤醒周期，单位为秒
};

struct T_RTMONITORCFG
{
       UINT16 usGlobalIdx;
	UINT16  usRTMonitorTmLen;
	 UINT16 cfg_flag;
};
typedef struct 
{
	UINT32 Initialized;
	UINT32 LocalBtsId;
	UINT8 cdrOnOff;
	UINT8 cdrPeriod;
	UINT32 IP_CB3000;
	UINT16 intval;
}stCdrNVRAMhdr;
struct T_CLUSTERNUMLMT
{    
    UINT16 grpUserNumUpperLimit;
    UINT16 grpPagingRspLimit;
    UINT16 cfg_flag;
};
#ifdef WBBU_CODE
typedef struct
{
    UINT32  initialized;
    UINT32  Voltage;
    UINT32  Current;
    UINT32  Tx_Power;
    UINT32  Rx_Power;
}FiberInfoLimint;
#endif
//wangwenhua add 2012-2-24   网络开关RF操作
#pragma pack(1)
typedef struct
{
     UINT32 flag;  //55aa55aa  表示为有效，other value is invalid

     UINT16 type;
/****************type 定义如下*****************************
0	不启用关射频功能；（基站默认不启用关射频功能）
1	启用数据传输断关射频功能；
2	启用语音传输断关射频功能；
3	启用数据/语音同时断关射频功能；
4	启用数据语音之一断关射频功能
other invalid
     *******************************************/
     UINT16 Close_RF_Time_Len;//1    
     //	关射频保护时长（含义：基站检测出传输异常后多长时间才关闭射频）：默认60秒
     UINT16 Open_RF_Time_Len;//2
     //	恢复射频保护时长（含义：基站检测出传输正常后多长时间才打开射频）：默认30秒
     UINT32 GateWayIP1;//3
     //	数据网关1的IP地址
     UINT32 GateWayIP2;//4
     //	数据网关2的IP地址
}RF_Operation;




typedef struct
{
 
     UINT16 type;
/****************type 定义如下*****************************
0	不启用关射频功能；（基站默认不启用关射频功能）
1	启用数据传输断关射频功能；
2	启用语音传输断关射频功能；
3	启用数据/语音同时断关射频功能；
4	启用数据语音之一断关射频功能
other invalid
     *******************************************/
     UINT16 Close_RF_Time_Len;//1    
     //	关射频保护时长（含义：基站检测出传输异常后多长时间才关闭射频）：默认60秒
     UINT16 Open_RF_Time_Len;//2
     //	恢复射频保护时长（含义：基站检测出传输正常后多长时间才打开射频）：默认30秒
     UINT32 GateWayIP1;//3
     //	数据网关1的IP地址
     UINT32 GateWayIP2;//4
     //	数据网关2的IP地址
     UINT8 GateWay1_MAC[6];
     UINT8   GateWay1_valid;
     UINT8   GateWay2_valid;
     UINT8  GateWay1_alarm;//数据网关1告警0-----正常，1-表示告警
     UINT8  GateWay2_alarm;//数据网关2告警0-----正常，1-表示告警
     UINT8 GateWay2_MAC[6];
     UINT32   Gateway1_bad_time ;
      UINT32   Gateway2_bad_time ; 
	UINT32   Voice_Bad_Time;
     UINT32   GateWay1_UP;
     UINT32  GateWay1_down;
     UINT32   GateWay2_UP;
     UINT32  GateWay2_down;
	 
     UINT32  state ;//0-not begin初始化完成还没有得到MAC地址 ,1---close RF; 2---open RF(normal)
	 
}RF_Operation_Para;
#pragma pack()

#pragma pack(1)
//有效频点集配置
typedef struct
{
    UINT8 validFreqsInd;
    UINT8 validFreqsNum;
    UINT16 validFreqs[20];
    UINT16 validFlag;//0x5a5a表示配置过
}VOLID_FREQS_PARA;
#pragma pack()
typedef struct
{
    UINT32                   init_code_high;        //20060823
    T_BTSCommonDataEle       BTSCommonDataEle;
    T_BtsGDataCfgEle         BtsGDataCfgEle;
    T_DataServiceCfgEle      DataServiceCfgEle;
    T_UTSDCfgEle             UTSDCfgEle;
    T_ToSCfgEle              ToSCfgEle[MAX_TOS_ELE_NUM];
    T_PerfLogCfgEle          PerfLogCfgEle;
    T_TempAlarmCfgEle        TempAlarmCfgEle;
    T_GpsDataCfgEle          GpsDataCfgEle;
    T_BtsRstEle              BtsRstEle;
    T_AirLinkCfgEle          AirLinkCfgEle;
    T_RMPoliceEle            RMPoliceEle;
    T_AirLinkMisCfgEle       AirLinkMisCfgEle;
    T_BillDataCfgEle         BillDataCfgEle;
    T_L1GenCfgEle            L1GenCfgEle;
    T_RfCfgEle               RfCfgEle;
    T_CalCfgEle              CalCfgEle;
    T_CaliGenCfgEle          CaliGenCfgEle;
    T_CaliDataEle            CaliDataEle[CALIBRAT_DATA_ELE_NUM];
    T_ACLCfgEle              ACLCfgEle;
    T_SFIDCfgEle             SFIDCfgEle;
    T_BtsNeighborCfgEle      BtsNeighborCfgEle;
    //T_NeighbotBTSLoadInfoEle NeighbotBTSLoadInfoEle;
    T_BTSRepeaterEle         BTSRepeaterEle;
    T_BTSUserCfgEle          BTSUserCfgEle;
    T_VlanGroupCfgEle        VlanGroupCfgEle;

	//#ifdef M_TGT_WANIF
	UINT32                   WanIfCpeEid;//主wcpe 
	//#endif
	T_N_Parameter            N_parameter;
	T_BtsNeighborCfgEle    BtsNeighborCommCfgEle;//同频信息	
	T_UtHandoverPara UtHandoverPara;//终端切换算法参数
	T_RangingPara RangingPara;//远距离Ranging参数
#ifdef WBBU_CODE
       T_WRRUDataEle           WRRUCfgEle;
#endif       
	T_CLUSTER_PARA ClusterPara;//集群功能及参数配置
#ifdef WBBU_CODE
	T_CalHardWare_Para Cal_Para;//wangwenhua add 20091030
#endif
       UINT32                 BakWanIfCpeEid;//备用
	UINT32                 Rev;//
	UINT16                  Wcpe_Switch;//开关
	UINT8                   ucRev[6];//	
#ifndef WBBU_CODE
	
	//#endif
    UINT32                   init_code_low;         //0xBADBAD
#endif
    UINT8 ucVacPrefScg;
    UINT8 ucSagFlag; 
    T_SagBkp SagBkp; 
    T_JitterBuf JitterBuf; 
    T_SagTos SagTos; 

    T_SavePwrCfg SavePwr; 
	T_Qam64Cfg Qam64Cfg;
    T_AlarmFlag AlarmFlag;
   // #ifdef M_TGT_WANIF
    UINT32   Relay_WcpeEid_falg;
    UINT32 Relay_num;
    UINT32   Relay_WcpeEid[10];//
    UINT8                   Sac_Mac_Addr[5][6];
  //  #endif
	UINT8 Reserved4Voice[64];	//fengbing 20091016 
    UINT32                 Enable_Wcpe_Flag_Judge;
	T_localSagCfg localSagCfg;		//fengbing 20091101
//#ifdef LJF_WAKEUPCONFIG 
	T_WAKEUP_CONFIG stWakeupConf;
       UINT32   Relay_WcpeEid_New[10];//rcpe增加10配置jiaying20100927
#ifdef WBBU_CODE

	UINT32                   init_code_low;         //0xBADBAD
#endif
	T_DcsCfgBuffer dcsCfgBuffer;
//    #ifdef LJF_RPT_ALTER_2_NVRAMLIST
	UINT32 ulRptListFlag;
	UINT32 ulRptList[M_RPT_LIST_MAX];
	#ifdef NUCLEAR_CODE
	T_UtHandoverPara2 stUtHandoverPara2;
	#endif
       #ifdef WBBU_CODE
      UINT8   Last_Calibration_Antenna;
       UINT8  RSVd;
       #endif
        T_PayloadBalanceCfg PayloadCfg;
	UINT32 pay_load_crc;
	//全网组呼时钟同步参数
	T_STime_CFG sTimePara;
	#ifndef WBBU_CODE
       T_Multicast_CFG     MulticastPara;

	#endif
       T_RTMONITORCFG RTMonitorCfg;
#ifdef RCPE_SWITCH
	TrunkMRCpeRcd stTrunkMRCpe;
#endif
	stCdrNVRAMhdr stCdr_para;
    T_CLUSTERNUMLMT clusterNumLimit;
      #ifdef WBBU_CODE

      FiberInfoLimint  fiber_para;//wangwenhua add 20111017
            
     UINT32     DSP2_Except_Protect;
     UINT32    Core9_Info_Save_Flag;
     char      Cal_Para_Seccond[1200];//wangwenhua add 2012-6-5
      #endif
	  RF_Operation      rf_operation;//wangwenhua add 2012-2-24
	  VOLID_FREQS_PARA volidFreqPara;
#ifdef PAYLOAD_BALANCE_2ND
    UINT16 usBandSwitchFlag;
    UINT16 usBandSwitch;
#endif      
}T_NvRamData;
#pragma pack()
#endif
