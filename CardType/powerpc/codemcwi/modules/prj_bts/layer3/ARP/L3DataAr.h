/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataArp.h
 *
 * DESCRIPTION: 
 *   details of Task Arp
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   11/08/05    yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_ARP_H__
#define __DATA_ARP_H__

#include <map>
using namespace std;
#include <list>

#include "BizTask.h"
#include "MsgQueue.h"
#include "ComMessage.h"
#include "log.h"
#include "logArea.h"

#include "L3DataCommon.h"
#include "L3Datatypes.h"
#include "L3DataARPMeasure.h"
#include "L3DataAssert.h"


//ARP 任务参数定义
#define M_TASK_ARP_TASKNAME      "tARP"
#ifdef __WIN32_SIM__
    #define M_TASK_ARP_OPTION        (0x0008)
    #define M_TASK_ARP_MSGOPTION     (0x02)
#else
    #define M_TASK_ARP_OPTION        ( VX_FP_TASK )
    #define M_TASK_ARP_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_ARP_STACKSIZE     (20480)
#define M_TASK_ARP_MAXMSG        (1024)

#define MAX_ARP_NODE             (6400)


//ARP错误码定义
const UINT16 ERR_ARP_NORMAL              = 0x0000;  //正常流程
const UINT16 ERR_ARP_UNEXPECTED_MSGID    = 0x0001;  //非法Message ID
const UINT16 ERR_ARP_SYS_ERR             = 0x0002;  //系统错误
const UINT16 ERR_ARP_PARAMETER           = 0x0004;  //参数错误
const UINT16 ERR_ARP_CB_USEDUP           = 0x0005;  //CB用光
const UINT16 ERR_ARP_CB_INDEX_ERR        = 0x0006;  //CB下标错误
const UINT16 ERR_ARP_NO_CB               = 0x0008;  //没找到对应的控制块
#ifdef WBBU_CODE
#define MSGID_ARP_Timer                 (0x23a0) //30s timer 
#endif

/*********************
 *ARP IpList Entry
 *********************/
#pragma pack (1)
typedef struct _tag_ArpILEntry {
    UINT32      ulEid;
    UINT8       aucMAC[M_MAC_ADDRLEN];
    UINT32      ulIpAddr;               /*Host Byte Order*/
    UINT8       flag;//0-normal,1-relay
    UINT8      Active_flag ;/****0----aictive ,1----inacitve************/
} ArpILEntry;
#pragma pack ()

class CTaskARP : public CBizTask {
public:
    static CTaskARP* GetInstance();
    void showStatus();

    bool AddILEntry( UINT32, const UINT8* ,UINT32,UINT8 = 0);//eid,mac,ip
    bool DelILEntry(UINT32, const UINT8*,UINT8 = 0);//ip
    bool sendG_ARP(const UINT8* ,UINT32);//mac,ip
    void sendAllGARP();
    #ifdef WBBU_CODE
    static void arpCallBack(unsigned int ip);
  
    void GateWayIPsend_arp_2GateWay();
    CTimer *pArpTimer;
   CTimer* sys_Createtimer(UINT16 MsgId, UINT8 IsPeriod, UINT32 TimerPeriod);
   static unsigned int m_err_count ;
    #endif
  SEM_ID m_SemId;
    //---------------------------------------------
    //清除所有转发方向上的某种类型包统计值 
    //---------------------------------------------
    inline void ClearMeasure()
    {
        memset(m_arDirTrafficMeasure,0, sizeof(m_arDirTrafficMeasure));
    }

    void GetPerfData(UINT8 *pData)   { memcpy( pData, m_arDirTrafficMeasure, sizeof( m_arDirTrafficMeasure ) );}

#ifdef UNITEST  
public:
#else
private:
#endif
    CTaskARP();
    ~CTaskARP();
 
        bool Initialize();
 
    inline TID GetEntityId() const 
    {
        return M_TID_ARP;
    }
    inline bool IsNeedTransaction()
    {
        return false;
    }
    bool ProcessComMessage(CComMessage*);

    #define ARP_MAX_BLOCKED_TIME_IN_10ms_TICK (200)
    bool IsMonitoredForDeadlock()  { return true; };
    int GetMaxBlockedTime() { return ARP_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    //define function 
    void proArpProxyReq(CComMessage*);
    void proArpProxyReq_WAN(CComMessage *pComMsg, EtherHdr *pEtherPkt, ArpHdr *pArphdr, int sendRcpeFlag);
    void proArpProxyReq_AI(CComMessage *pComMsg, EtherHdr *pEtherPkt, ArpHdr *pArphdr, int sendRcpeFlag);
    void proArpProxyReq_TDR(CComMessage *pComMsg, EtherHdr *pEtherPkt, ArpHdr *pArphdr);
    bool proArpConfig(CComMessage*);

    //---------------------------------------------
    //设置指定转发方向上的某种类型包统计值 +1
    //---------------------------------------------
    inline UINT32 IncreaseDirTrafficMeasureByOne( ARPFROMMEASURE from ,ARPTOMEASURE to )
    {
        DATA_assert(  from < ARP_FROM_MAX );
        DATA_assert(  to < ARP_TO_MAX );
        return m_arDirTrafficMeasure[ from ][to] += 1;
    }


    inline bool GetEgressArpEn() const
    {
        return m_blEgressArpEn;
    }
    inline bool GetIngressArpEn() const
    {
        return m_blIngressArpEn;
    }
    inline bool SetEgressArpEn(bool bl) 
    {
        return m_blEgressArpEn = bl;
    }
    inline bool SetIngressArpEn(bool bl) 
    {
        return m_blIngressArpEn = bl;
    }
    // Free List methods
    void InitFreeArpLst();
    void InsertFreeArpLst(UINT16);
    UINT16 GetFreeArpIdxFromLst();

    // BPtree methods
    bool BPtreeAdd(UINT32, UINT16);
    bool BPtreeDel(UINT32);
    UINT16 BPtreeFind(UINT32);
    ArpILEntry* GetArpIplstByIp(UINT32);
    inline void initArpIplstTb()
    {
        memset(m_ArpIplstTb,0,MAX_ARP_NODE*sizeof(ArpILEntry));
    }
	inline UINT16 GetProtoType(CComMessage *pComMessge)
     {
           UINT8    *pData = (UINT8*)( pComMessge->GetDataPtr() );
          VLAN_hdr *pVlan = (VLAN_hdr*)( pData + 12 );
        if ( M_ETHER_TYPE_VLAN == ntohs( pVlan->usProto_vlan ) )
        {
                 EtherHdrEX *pEtherHdr1 = (EtherHdrEX*) ( pComMessge->GetDataPtr() );
            //modify by wangx
            if(IS_8023_PACKET(ntohs(pEtherHdr1->usProto)))
            {
                return ntohs(*(UINT16*)((UINT8*)pEtherHdr1 + sizeof(EtherHdrEX)-2 + sizeof(LLCSNAP)));
            }
            else
            {
                return ntohs( pEtherHdr1->usProto );
            }
        }
       else
        {
            EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
            //modify by wangx
            if(IS_8023_PACKET(ntohs(pEtherHdr->usProto)))
            {
                return ntohs(*(UINT16*)((UINT8*)pEtherHdr + sizeof(EtherHdr)-2 + sizeof(LLCSNAP)));
            }
            else
            {
                return ntohs( pEtherHdr->usProto );
            }
        }
    }
    inline bool hasVlanTag(CComMessage *pComMessge)
    {
         UINT8    *pData = (UINT8*)( pComMessge->GetDataPtr() );
          VLAN_hdr *pVlan = (VLAN_hdr*)( pData + 12 );
        if ( M_ETHER_TYPE_VLAN == ntohs( pVlan->usProto_vlan ) )
        {
            return true;
        }
        else
            return false;
    }
  static STATUS RunNetWorksUp(CTaskARP *);//wangwenhua add 2012-3-5
   void doNetWork();
private:
    static CTaskARP* s_ptaskARP;
    ArpILEntry m_ArpIplstTb[MAX_ARP_NODE];
    list<UINT16> m_listFreeArp; //空闲转发表表项的链表
    map<UINT32, UINT16> m_Arptree;  // 转发表索引树

    bool m_blEgressArpEn;
    bool m_blIngressArpEn;
    UINT32 m_arDirTrafficMeasure[ ARP_FROM_MAX ][ARP_TO_MAX];
    static unsigned int  m_last_ip;
    static unsigned int m_gaprp_count;//wangwenhua add 20101011 for avoid sending garp packets to wan if there is only a cpe
#ifdef RCPE_SWITCH
	bool trunkSendMsg(TID tid, UINT16 usMsgID, UINT8* pd, UINT32 ulLen, DIRECTION dir, UINT32 ulEID, UINT16 vlantag=1);
#endif
};

#endif
