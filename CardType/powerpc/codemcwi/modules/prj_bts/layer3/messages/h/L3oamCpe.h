/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
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

#ifndef _INC_L3OAMCPECOMMON
#define _INC_L3OAMCPECOMMON

#ifndef  DATATYPE_H
#include "datatype.h"
#endif

#ifndef _INC_L3OAMCFGCOMMON
#include "L3OamCfgCommon.h"
#endif

#include <string.h>

const UINT32 MAX_CPE_REMANENT_CYCLE = 60*24*2;  /*minutes, 2 days.*/
const UINT32 MIN_CPE_REMANENT_CYCLE = 60;       /*minutes, 1 hour.*/

const UINT8  MAX_VOICE_PORT_NUM    = 32;
const UINT8  HW_VERSION_LEN        = 16;
const UINT16 MAX_CPE_PROFILE_SIZE  = 700;
const UINT16 MAX_MESSAGE_SIZE      = 1400;
#define FIX_SIZE_OFMsg_PROFUPDATA_REQ    18    //包括    version(2), transid(2), 
                                               //LocAreaId(4)，BWReqInter(2), SessRelThres(2)  ProfileDataFlag(2)   
                                               //以及 QosEntryNum(2), Neiberlistnum(1), reserve(1)
#pragma pack(1)
struct T_CpeBaseInfo  //UT Registration Request（BTS）to ems
{
    UINT16  NetworkID;      //  Home Network ID 
    UINT8   HardwareType;   //  0 - Handset  1 - desktop  2 - PCMCIA   3 - Z module
    UINT8   SoftwareType;   //  0 - a     1 - b     2 - c
    UINT32  ActiveSWVer;    //  Major-Minor-build
    UINT32  StandbySWVer;   //  Major-Minor_build
    UINT8   HWVer[HW_VERSION_LEN];      //  16 byte 
};

struct T_CpeFixIpInfo
{
    UINT8  MAC[MAC_ADDR_LEN];
    UINT32 FixIP;
    UINT32 AnchorBTSID;
    UINT32 RouterAreaID;
};

struct T_L2SpecialFlag
{
    UINT16  L2_Special_Flag1;
    UINT16  L2_Special_Flag2;
    UINT16  L2_Special_Flag3;
    UINT16  L2_Special_Flag4;
	UINT8   RFprofile[12];
};
//mem特殊处理,添加以下结构
struct T_MEM_CFG
{
    UINT8 rev;
    UINT8 MemIpType;
    UINT32 DNSServer;
    UINT32 MemIp;
    UINT32 SubMask;
    UINT32 GateWay;
    UINT32 Rev;
};
struct T_CpeFixIpInfoForData
{
    UINT32 CPEID;
    UINT8  MAC[MAC_ADDR_LEN];
    UINT32 FixIP;       
    UINT32 AnchorBTSID;     
    UINT32 RouterAreaID;        
};

struct T_UTProfileIEBase  //26 byte     
{
    UINT8   AdminStatus;  //    0 - active  1 - suspended  2 -- invalid 
    UINT8   LogStatus;    //  Perf Log Status   1   0 - disabled  1 - enabled   
    UINT16  DataCInter;   //  Perf data collection Interval 2   Seconds 
    UINT8   Mobility;     //  0 - disabled  1 - enabled 
    UINT8   DHCPRenew;    //  Allow DHCP renew in serving BTS   1   0 - disabled  1 - enabled   
    UINT16  WLANID;       //  0 - No WLAN TAG   ohter - WLAN ID
    UINT8   isDefaultProfile;    //
    UINT8   MaxIpNum;     //  Max IP address number     1~20    
    UINT32  VoicePortMask;
    T_UTSDCfgInfo   UTSDCfgInfo;
    UINT8  ucPeriodicalRegisterTimerValue;
    UINT8  FixIpNum;     //Fixed Ip number      0~20
};

struct T_UTProfile
{        
    T_UTProfileIEBase UTProfileIEBase; 
    T_CpeFixIpInfo    CpeFixIpInfo[MAX_FIX_IP_NUM];
    UINT32 length() const
        {
        UINT8 fixIPnum = (UTProfileIEBase.FixIpNum > MAX_FIX_IP_NUM)?MAX_FIX_IP_NUM:UTProfileIEBase.FixIpNum;
        return sizeof(UTProfileIEBase) +  fixIPnum * sizeof(T_CpeFixIpInfo);
        }
};

struct  T_UT_HOLD_BW
{
    UINT16  UL_Hold_BW;
    UINT16  DL_Hold_BW;
};
struct T_L3CpeProfUpdateReqBase
{
    UINT16 TransId;         // 0
    UINT16 Version;         // 2
    
    UINT32 LocAreaId;       // 4 
    UINT16 BWReqInter;      // 
    UINT16 SessRelThres;    // 
    UINT16 ProfileDataFlag;    // 0 --  profile ie data 无效，消息中不带profile ie 数据 
                               // 1 --  profile ie data 有效，消息中带有profile ie 数据
    
    T_UTProfileIEBase UTProfileIEBase;
};

struct T_L3CpeProfUpdateReqBaseNoIE
{
    UINT16 TransId;         // 0
    UINT16 Version;         // 2
    
    UINT32 LocAreaId;       // 4 
    UINT16 BWReqInter;      // 
    UINT16 SessRelThres;    // 
    UINT16 ProfileDataFlag;    // 0 --  profile ie data 无效，消息中不带profile ie 数据 
                               // 1 --  profile ie data 有效，消息中带有profile ie 数据
};


//Add for Z_module
const UINT16 MAX_CID_NUM    = 32;
const UINT16 MAX_Z_MOD_NUM  = 8;
struct T_CID_UID_PAIR
{
    UINT8  cid;
    UINT32 UID;
};
struct T_UID_VER_PAIR
{
    UINT32 UID;             /*每个Z模块0端口的UID*/
    UINT32 ProductType;
    UINT32 SWVer;
};
struct T_ZmoduleBlock
{
#if 0
    UINT8 cidNum;
    UINT8 LoginFlag;
    T_CID_UID_PAIR cid_uid[MAX_CID_NUM];
    UINT8 Znum;
    T_UID_VER_PAIR uid_ver[MAX_Z_MOD_NUM];
    UINT32 Znum_Offset()
        {
        return sizeof(cidNum) + sizeof(LoginFlag) + cidNum*sizeof(T_CID_UID_PAIR);
        }
    UINT32 UidVer_Offset()
        {
        return Znum_Offset() + sizeof(Znum);
        }
#endif
    UINT8  cidNum;
    UINT8  LoginFlag;
    UINT8  Znum;
    UINT8  reserve;
    T_CID_UID_PAIR cid_uid[MAX_CID_NUM];
    T_UID_VER_PAIR uid_ver[MAX_Z_MOD_NUM];
    UINT32 UidVer_Offset()
        {
        return sizeof(cidNum) + sizeof(LoginFlag) + sizeof(Znum) + sizeof(reserve) + cidNum*sizeof(T_CID_UID_PAIR);
        }
};
#pragma pack()
#endif
