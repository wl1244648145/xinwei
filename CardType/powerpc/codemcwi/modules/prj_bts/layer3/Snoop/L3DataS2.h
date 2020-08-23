/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopFSM.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   03/30/06   xiao weifang  NVRAM恢复用户信息. 
 *   09/05/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __SNOOP_FSM_H__
#define __SNOOP_FSM_H__

#include <stdio.h>

#include <map>
using namespace std;
#include <list>

#include "Object.h"
#include "biztask.h"
#include "MsgQueue.h"
#include "Message.h"
#include "log.h"

#include "fsm.h"
#include "L3DataTypes.h"
#include "L3DataMacAddress.h"
#include "L3DataCommon.h"
#include "L3DataMsgId.h"

#ifndef _NO_NVRAM_RECOVER_
#include "L3DataNVRAM.h"
#endif
#include "L3DataAssert.h"


/****************************
 *SNOOP_CCB_NUM: 
 ****************************/
#define SNOOP_CCB_NUM       (M_MAX_USER_PER_BTS)


/****************************
 *SNOOP_STATE: 状态的枚举值
 ****************************/
typedef enum {
    STATE_SNOOP_IDLE = 0,
    STATE_SNOOP_SELECTING,
    STATE_SNOOP_REQUESTING,
    STATE_SNOOP_SYNCING,
    STATE_SNOOP_BOUND,
    STATE_SNOOP_RENEWING,
    STATE_SNOOP_ROAMING,
    STATE_SNOOP_MAX
}SNOOP_STATE;


/*************************************************
 *strSTATE: SNOOP_STATE对应的字符串
 *************************************************/
#define M_STATE_STRLEN  (11)
const UINT8 strSTATE[ STATE_SNOOP_MAX ][ M_STATE_STRLEN ] = {
    "IDLE",         //STATE_SNOOP_IDLE = 0,
    "SELECTING",    //STATE_SNOOP_SELECTING,
    "REQUESTING",   //STATE_SNOOP_REQUESTING,
    "SYNCING",      //STATE_SNOOP_SYNCING,
    "BOUND",        //STATE_SNOOP_BOUND,
    "RENEWING",     //STATE_SNOOP_RENEWING,
    "ROAMING"       //STATE_SNOOP_ROAMING,
};


/****************************
 *SNOOP_TRANS: 
 ****************************/
typedef enum {
    TRANS_SNOOP_IDLE_PADI = 0,
    TRANS_SNOOP_IDLE_DISC,
    TRANS_SNOOP_IDLE_TUNNEL_SYNC_REQ,
    TRANS_SNOOP_IDLE_TUNNEL_TERMINATE_REQ,
    TRANS_SNOOP_IDLE_TUNNEL_CHANGE_ANCHOR_REQ,
    TRANS_SNOOP_IDLE_FIXIP_TUNNEL_ESTABLISH_REQ,
    TRANS_SNOOP_IDLE_ADD_FIXIP, /*6*/
    TRANS_SNOOP_IDLE_REQUEST,/*在idle状态下可能收到REQUEST消息*/

    TRANS_SNOOP_SELECTING_REQ = 10,      
    TRANS_SNOOP_SELECTING_DISC,
    TRANS_SNOOP_SELECTING_OFFER,
    TRANS_SNOOP_SELECTING_PADR,
    TRANS_SNOOP_SELECTING_PADI,
    TRANS_SNOOP_SELECTING_PADO, /*15*/
    TRANS_SNOOP_SELREQ_TIMEOUT,
    TRANS_SNOOP_SELREQ_DELENTRY,

    TRANS_SNOOP_REQUESTING_ACK = 20,
    TRANS_SNOOP_REQUESTING_NAK,
    TRANS_SNOOP_REQUESTING_REQ,
    TRANS_SNOOP_REQUESTING_DISC,    /*23*/
    TRANS_SNOOP_REQUESTING_PADS,
    TRANS_SNOOP_REQUESTING_PADR,
    TRANS_SNOOP_REQUESTING_PADI,    /*26*/

    TRANS_SNOOP_SYNCING_DHCP_SYNC_SUCCESS = 30,
    TRANS_SNOOP_SYNCING_PPPoE_SYNC_SUCCESS,
    TRANS_SNOOP_SYNCING_FAIL,
    TRANS_SNOOP_SYNCING_PADI,
    TRANS_SNOOP_SYNCING_DISC,
    //TRANS_SNOOP_SYNCING_TIMEOUT, //= TRANS_SNOOP_SYNCING_FAIL
    TRANS_SNOOP_SYNCING_DELENTRY,

    TRANS_SNOOP_BOUND_REQ_SELECTING = 40,
    TRANS_SNOOP_BOUND_REQ_RENEWING,
////TRANS_SNOOP_BOUND_ACK,
    TRANS_SNOOP_BOUND_PADR,
    TRANS_SNOOP_BOUND_PADS,     /*45*/
    TRANS_SNOOP_BOUND_PADT,
    TRANS_SNOOP_BOUND_TIMEOUT,

    TRANS_SNOOP_RENEWING_ACK = 50,
    TRANS_SNOOP_RENEWING_NAK,
    TRANS_SNOOP_RENEWING_REQ,
    TRANS_SNOOP_RENBND_DELENTRY,

    TRANS_SNOOP_ROAMING_TUNNEL_ESTABLISH_RESP = 60,
    TRANS_SNOOP_ROAMING_TIMEOUT,
    TRANS_SNOOP_ROAMING_DELENTRY,

    TRANS_SNOOP_PARENT_TUNNEL_SYNC_REQ = 70,
    TRANS_SNOOP_PARENT_ROAM_REQ,
    TRANS_SNOOP_PARENT_TUNNEL_ESTABLISH_REQ,
    TRANS_SNOOP_PARENT_TUNNEL_TERMINATE_REQ,
    TRANS_SNOOP_PARENT_ADD_FIXIP,
    TRANS_SNOOP_PARENT_TUNNEL_CHGANCHOR_RESP,   /*75*/
    TRANS_SNOOP_PARENT_HEARTBEAT_TIMER,
    TRANS_SNOOP_PARENT_HEARTBEAT,
    TRANS_SNOOP_PARENT_HEARTBEAT_RESP,

    //有多个状态共用的transition.
    TRANS_SNOOP_TUNNEL_ESTABLISH_REQ = 80,          //Bound & Renew
    TRANS_SNOOP_TUNNEL_TERMINATE_REQ,               //Bound & Renew
    TRANS_SNOOP_TUNNEL_CHANGE_ANCHOR_REQ,   /*82*/  //Bound & Renew
    TRANS_SNOOP_DISCOVERY,                          //Bound & Renew & Roaming
    TRANS_SNOOP_PADI,                               //Bound & Roaming
    TRANS_SNOOP_RELEASE_DECLINE,                    //BOUND & Renew & Roaming
    TRANS_SNOOP_ENTRY_EXPIRE,                       //Bound & Roaming

    TRANS_SNOOP_MAX
}SNOOP_TRANS;
	

/****************************
 *SnoopCCB类: 
 ****************************/
class CSnoopCCB: public CCBBase
{
public:
    CSnoopCCB():CCBBase( STATE_SNOOP_IDLE ) 
        {
        InitCCB();
        }

    void ResetCCB();    //Reset.不是Init.
    void showCCB();     //打印

    /*******************************************
     *判断本CCB是否和其他BTS存在隧道?
     *BTS既是CCB的AnchorBTS，又是CCB的ServingBTS,
     *或者二者都不是(初始化的情况)
     *那么必然不存在隧道，否则存在隧道。
     */
    bool IsExistTunnel()                  {  return GetIsAnchor() ^ GetIsServing(); }

//    bool GetIsOccupied() const            {return m_bIsOccupied;}
//    bool SetIsOccupied( bool bIsOccupied ){return m_bIsOccupied = bIsOccupied;}

    bool GetIsAuthed() const              {return (bool)m_ucIsAuthed;}
    bool SetIsAuthed( bool bIsAuthed )    {return m_ucIsAuthed = (UINT8)bIsAuthed;}

    UINT32 GetEid() const                 {return m_ulEid;}
    UINT32 SetEid( UINT32 ulEid )         {return m_ulEid = ulEid;}

    UINT8* GetMac()                       {return m_aucMac;}
    void   SetMac(const UINT8 *);

    UINT8 GetIpType() const               {return m_ucIpType;}
    UINT8 SetIpType( UINT8 ucIpType )     {return m_ucIpType = ucIpType;}

    DATA& GetData()                       {return m_Data;}
    void  SetDATA( const DATA &Data )         
        {
        memcpy( (void*)&m_Data, &Data, sizeof( DATA ) );
        }

    UINT32 GetRAID() const                {return m_ulRouterAreaId;}
    UINT32 SetRAID( UINT32 ulRaid )       {return m_ulRouterAreaId = ulRaid;}

    bool GetIsAnchor() const              {return (bool)m_ucIsAnchor;}
    bool SetIsAnchor( bool bIsAnchor )    {return m_ucIsAnchor = (UINT8)bIsAnchor;}
	//wangwenhua add 20081023
     bool GetIsRealAnchor() const              {return (bool)m_IsRealAnchor;}
    bool SetIsRealAnchor( bool bIsRealAnchor )    {return m_IsRealAnchor = (UINT8)bIsRealAnchor;}

	

    bool GetIsServing() const             {return (bool)m_ucIsServing;}
    bool SetIsServing( bool bIsServing )  {return m_ucIsServing = (UINT8)bIsServing;}

    UINT32 GetAnchorBts() const                 {return m_ulAnchorBts;}
    UINT32 SetAnchorBts( UINT32 ulAnchorBts )   {return m_ulAnchorBts = ulAnchorBts;}

    UINT32 GetServingBts() const                {return m_ulServingBts;}
    UINT32 SetServingBts( UINT32 ulServingBts ) {return m_ulServingBts = ulServingBts;}

    UINT16 getGroupId() const                   {return m_usGroupID;}
    UINT16 setGroupId(UINT16 usGroupID)         {return m_usGroupID = usGroupID;}

    CMessage*  GetMsg() const    {return m_pMsg;}
    CMessage*  SaveMsg(CMessage *);
    void       DeleteMsg();

    CTimer*    GetTimer() const    {return m_pTimer;}
    CTimer*    SetTimer(CTimer *);
    void       DeleteTimer();

    bool       startHeartBeat();
    void       stopHeartBeat();
    void setCreateTime(UINT32 tt){ m_createTime = tt;}
    UINT32 getCreateTime() {return m_createTime;}
    bool getIsRcpeFlag(){return m_IsRcpeFlag;}
    void setRcpeFlag(UINT8 rcpeFlag){m_IsRcpeFlag = rcpeFlag;}
private:
    void InitCCB();     //初始化

private:
    //members
    UINT8    m_ucIsAuthed:1;        /*has been authenticated*/
    UINT8    m_ucIsAnchor:1;        /*BTS是CCB的Anchor BTS*/
    UINT8    m_ucIsServing:1;       /*CCB对应的UT在BTS服务区*/
    UINT8    m_IsRealAnchor:1;//wangwenhua add 20081022
    UINT8    m_ucIpType:4;          /*0: DHCP; 1: PPPoE*/
    UINT32   m_ulEid;
    UINT8    m_aucMac[ M_MAC_ADDRLEN ];
    DATA     m_Data;
    UINT32   m_ulRouterAreaId;
    UINT32   m_ulAnchorBts;      /*CCB的Anchor BTSID*/
    UINT32   m_ulServingBts;     /*CCB的Serving BTSID*/
    UINT16   m_usGroupID;        /**/
    CMessage *m_pMsg;            /*Message Saved before starting synchronization under SYNCING state.*/
    CTimer   *m_pTimer;
    CTimer   *m_pHeartBeatTimer; /*切换用户和anchor之间的隧道维护消息*/
    UINT32 m_createTime;
    UINT8 m_IsRcpeFlag;
};


/****************************
 *CSnoopCCBTable类: 
 ****************************/
class CSnoopCCBTable: public CCBTableBase
{
public:
    CSnoopCCBTable()
        {
        InitFreeCCB();
        }
    ~CSnoopCCBTable(){}

    void showStatus(UINT32);
    void showStatus(CMac &Mac);
    UINT32 getIpByMAC(CMac &Mac);
    CCBBase *FindCCB(CMessage &);

    // Free CCB list methods
    void InitFreeCCB();
    void InsertFreeCCB(UINT16);
    UINT16 GetFreeCCBIdxFromList();

    // BPtree methods
    bool BPtreeAdd(CMac&, UINT16);
    bool BPtreeDel(CMac&);
    UINT16 BPtreeFind(CMac&);

    //---------------------------------------------
    //返回指定下标的CCB
    //---------------------------------------------
    CSnoopCCB* GetCCBByIdx(UINT16);

    //打印指定下标的CCB
    void showCCB(UINT16);
    //打印指定Mac地址的CCB
    void showCCB(CMac &Mac){ showCCB( BPtreeFind( Mac ) ); }

    void groupModify(UINT32, UINT16);
	void clearCPEData(UINT32);

	bool isLeaseUpdated(const UINT32, const UINT32, const CMac&);

private:
    //members.
    list<UINT16>      m_listFreeCCB;
    CSnoopCCB         m_CCBTable[ SNOOP_CCB_NUM ];
    map<CMac, UINT16> m_CCBtree;    //CCB的索引树
};


/****************************
 *CSnoopFSM类: 
 ****************************/
class CSnoopFSM: public FSM
{
#ifndef _NO_NVRAM_RECOVER_
public:
    static CNVRamCCBTable *s_pNVRamCCBTable;

    void RecoverUserInfoFromNVRAM();
private:
    CTimer* StartTimer(const UINT8*, UINT32);
    bool    AddFTEntry(CSnoopCCB *);

private:
    //members.
    bool           m_bRecoverFinished;
#endif
public:
    CSnoopFSM();
    UINT8 GetStateNumber()  {return  STATE_SNOOP_MAX;}
    UINT8 GetTransNumber()  {return  TRANS_SNOOP_MAX;}
    CCBBase *FindCCB(CMessage &);
    //回收CCB.
    void Reclaim(CCBBase *);

    //打印FSM状态
    void showStatus(UINT32 ulEid) { m_pCCBTable->showStatus( ulEid ); }
    void showStatus(CMac &Mac) { m_pCCBTable->showStatus( Mac ); }
    UINT32 get_ip_by_mac(CMac &Mac) {return m_pCCBTable->getIpByMAC(Mac);}

    CSnoopCCBTable* GetCCBTable() const   { return m_pCCBTable; }

private:
    //members.
    CSnoopCCBTable *m_pCCBTable;
};

#endif /*__SNOOP_FSM_H__*/
