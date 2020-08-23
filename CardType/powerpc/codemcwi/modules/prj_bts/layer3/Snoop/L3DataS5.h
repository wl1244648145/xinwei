/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataSnoopTrans.h
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

#ifndef __SNOOP_TRANS_H__
#define __SNOOP_TRANS_H__
#ifndef __WIN32_SIM__
//VxWorks:
#include "inetLib.h" 
#endif

#include "L3DataSnoopFSM.h"
#include "L3DataSnoopMeasure.h"
#include "L3DataFixIp.h"
#include "L3DataDM.h"

/****************************
 *CSnoopTrans类: 
 ****************************/
class CSnoopTrans: public FSMTransition
{
public:
    CSnoopTrans(FSMStateIndex target): FSMTransition(target) 
        {
        memset( m_aulOutMeasure, 0, sizeof( m_aulOutMeasure ) );
        };
    virtual FSMStateIndex Action(CSnoopCCB&, CMessage&) = 0;
    virtual FSMStateIndex Action(CCBBase &ccb, CMessage &msg) 
        {
        return Action( (CSnoopCCB&)ccb, msg );
        };

    static void PrintMeaure();
    //调用者确保pData内存足够大
    static void GetPerfData(UINT8 *pData)    { memcpy( pData, m_aulOutMeasure, sizeof( m_aulOutMeasure ) ); }

    static void ClearMeasure()
        {
        memset( m_aulOutMeasure, 0, sizeof( m_aulOutMeasure ) );
        }

protected:
    CTimer* StartSnoopTimer(const UINT8 *, UINT32);

    //消息发送函数，因为其他transition都可能用到，
    //所以放在基类
    bool AddFTEntry(UINT32, const UINT8*, bool, bool, UINT32, UINT8, bool, CSnoopCCB &);
    bool DelFTEntry(const UINT8 *, bool = false, UINT32 = 0, UINT16 = 0);
    bool UpdateFTEntry(CSnoopCCB &, UINT8, bool = false);
    bool ForwardTraffic(CMessage &);
    void ForwardTrafficToWan(CMessage &);
    void ForwardTrafficToAI(CMessage &, UINT32);
    void ForwardTrafficToTDR(CMessage &, UINT32);
    bool Synchronize(UINT8, bool, CSnoopCCB&);
    bool Synchronize(UINT8, bool, UINT32, const UTILEntry &);
    bool SendTunnelSync(CSnoopCCB &, bool);
    bool SendTunnelSyncResponse(CSnoopCCB &, UINT32 ,UINT32 ,UINT16 , bool );
    bool SendTunnelTerminate(UINT32, const UINT8 *, UINT32, UINT8);
    bool SendTunnelTerminate(CSnoopCCB &);
    bool SendTunnelTerminateResponse(CSnoopCCB &, UINT32 ,UINT32,UINT16, bool);
    bool SendTunnelEstablish(UINT32, const UINT8 *, UINT32, UINT8, UINT32 = 0);
    bool SendTunnelEstablishResponse(CSnoopCCB &, UINT32 ,UINT32 ,UINT16 , bool );
    bool SendTunnelChangeAnchor(UINT32 , const UINT8 *, UINT32 , UINT8  );
    bool SendTunnelChangeAnchorResponse(CSnoopCCB &, UINT32 ,UINT32,UINT16, bool);
    bool SendTunnelHeartBeat(CSnoopCCB &, UINT32);
    bool SendTunnelHeartBeatResponse(CSnoopCCB &, UINT32,UINT32,UINT16, bool);
    bool NotifyDelEidTable(UINT32);

    //往DM查询的接口
    bool IsUTRenewEnable(UINT32) const;
    bool IsUTMobilityEnable(UINT32) const;
    bool IsUTAccessEnable(UINT32) const;
    bool IsBTSsupportMobility(UINT32, UINT32, UINT32) const;

    void   DelDHCPTag(CMessage&);
    void   AddDHCPTag(CMessage&);
    void   AddPPPoETag(CMessage &);
    void   DelPPPoETag(CMessage &);
    UINT16 udp_checksum(IpHdr*,UdpHdr*);
    UINT16 ip_checksum (IpHdr*);
    UINT16 getGroupIDbyEid(UINT32 ulEid) const  {return CTaskDm::GetInstance()->GetVlanIDbyEid(ulEid);}

#ifndef _NO_NVRAM_RECOVER_
    void   AddToNVRAM(CSnoopCCB &);
    void   DelFromNVRAM(CSnoopCCB &);
#endif

private:
    //---------------------------------------------
    //获取指定转发方向(Ingress/Egress/TDR)的包统计值
    //---------------------------------------------
    UINT32 GetMeasure( OUT_TYPE type )
        {
        DATA_assert( type < OUT_TYPE_MAX );
        return m_aulOutMeasure[ type ];
        }

    //---------------------------------------------
    //设置指定类型包统计值 +1
    //---------------------------------------------
    UINT32 IncreaseMeasureByOne( OUT_TYPE type )
        {
        DATA_assert( type < OUT_TYPE_MAX );
        return m_aulOutMeasure[ type ] += 1;
        }

    //---------------------------------------------
    //清除所有类型包统计值 
    //---------------------------------------------
private:
    /*************************************************************
     *performance Measurement:
     *m_aulOutMeasure[ OUT_TYPE_MAX ]记录Snoop发送的
     *不同数据类型的统计值
     ************************************************************/
    static UINT32 m_aulOutMeasure[ OUT_TYPE_MAX ];
};


/****************************
 *CParentTunnelSyncReqTrans类: 
 ****************************/
class CParentTunnelSyncReqTrans: public CSnoopTrans
{
public:
    CParentTunnelSyncReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
    void ProcSync0(CSnoopCCB &, UINT32 ,UINT32 ,UINT16 );
    void ProcSyncIPLease(CSnoopCCB &, UINT32,UINT32 ,UINT16, UINT32, UINT32);
};


/****************************
 *CParentRoamReqTrans类: 
 ****************************/
class CParentRoamReqTrans: public CSnoopTrans
{
public:
    CParentRoamReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
private:
    bool RoamOutRaid(UINT32, const UTILEntry &, CSnoopCCB &);
    bool RoamInSameRaid(UINT32, const UINT8 *, UINT32, UINT8, CSnoopCCB &);
    bool RoamBackToAnchor(UINT32, const UINT8 *, UINT32, UINT8, bool, CSnoopCCB &);
    bool RoamAddIp(CSnoopCCB &, UINT32, bool, UINT32, UINT32, UTILEntry &);
};


/*****************************************************
 *CParentTunnelEstablishReqTrans类: 
 *Tunnel Establish Request只在BOUND/RENEWING状态处理，
 *在其他状态，只做失败响应，避免请求方重发消息
 *****************************************************/
class CParentTunnelEstablishReqTrans: public CSnoopTrans
{
public:
    CParentTunnelEstablishReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/*****************************************************
 *CParentTunnelTerminateReqTrans类: 
 *Tunnel Terminate Request只在BOUND/RENEWING状态处理，
 *在其他状态，只做失败响应，避免请求方重发消息
 *****************************************************/
class CParentTunnelTerminateReqTrans: public CSnoopTrans
{
public:
    CParentTunnelTerminateReqTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/*****************************************************
 *CParentAddFixIPTrans类: 
 *Add FixIP消息在IDLE之外状态的处理
 *****************************************************/
class CParentAddFixIPTrans: public CSnoopTrans
{
public:
    CParentAddFixIPTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);

private:
    bool RoamOut(UINT32, const UTILEntry&, CSnoopCCB &);
    bool AddFixIpEntry(CSnoopCCB &, UINT32, bool, UINT32, UINT32, const UTILEntry&);
};



/*****************************************************
 *CParentTunnelChgAnchorRespTrans类: 
 *****************************************************/
class CParentTunnelChgAnchorRespTrans: public CSnoopTrans
{
public:
    CParentTunnelChgAnchorRespTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};



/*****************************************************
 *CParentHeartBeatTimerTrans类: 
 *****************************************************/
class CParentHeartBeatTimerTrans: public CSnoopTrans
{
public:
    CParentHeartBeatTimerTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/*****************************************************
 *CParentHeartBeatTrans类: 
 *****************************************************/
class CParentHeartBeatTrans: public CSnoopTrans
{
public:
    CParentHeartBeatTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};


/*****************************************************
 *CParentHeartBeatRespTrans类: 
 *****************************************************/
class CParentHeartBeatRespTrans: public CSnoopTrans
{
public:
    CParentHeartBeatRespTrans(FSMStateIndex target): CSnoopTrans(target) {};
    virtual FSMStateIndex Action(CSnoopCCB &ccb, CMessage &msg);
};

#endif/*__SNOOP_TRANS_H__*/
