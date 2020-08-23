/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    Dm.h
 *
 * DESCRIPTION: 
 *   details of class Dm
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   04/10/06   xiao weifang  BTS不支持切换漫游需要的处理
 *   09/26/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_DM_H__
#define __DATA_DM_H__

#include <map>
using namespace std;

#include <list>

#include "BizTask.h"
#include "ComMessage.h"
#include "log.h"

#include "L3DataMacAddress.h"
#include "L3dataTypes.h"
#include "L3DataDmTimerExpire.h"
#include "L3DataDmMessage.h"
#include "L3DataDmComm.h"
#include "L3DataAssert.h"


//DM 任务参数定义
#define M_TASK_DM_TASKNAME      "tDM"
#ifdef __WIN32_SIM__
    #define M_TASK_DM_OPTION        (0x0008)
    #define M_TASK_DM_MSGOPTION     (0x02)
#else
    #define M_TASK_DM_OPTION        ( VX_FP_TASK )
    #define M_TASK_DM_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_DM_STACKSIZE     (20480)
#define M_TASK_DM_MAXMSG        (1024)

#define M_SYNC_BOTH             (0x11)
#define M_SYNC_ADDRONLY         (0x10)
#define M_SYNC_IPLISTONLY       (0x01)
#define M_SYNC_IDLE             (0x00)

#define M_MEASURE_MAX       M_MAX_UT_PER_BTS+1

#define SOFT_VERSION            (1)


class CTaskDm : public CBizTask
{
friend void   DMShow(UINT8);
friend UINT16 getServingEIDnum();
public:
    static CTaskDm* GetInstance();
    bool BuildFixIpContext(const CMac &, const UINT32);
    void showStatus( );
    void showDMbyEid(UINT32 ulEid);
    //interface for other Task
    bool   GetCPEProfile(UINT32,CPEProfile*);
////是否允许该用户移动漫游切换
    bool   IsBTSsupportMobility(UINT32, UINT32, UINT32);
    //调用者确保pData内存足够大
    void   GetPerfData(UINT8 *pData)   { memcpy( pData, &(CPEPerfMeasureTb[ M_MAX_UT_PER_BTS ]), sizeof( CPEPerfMeasureTb[ M_MAX_UT_PER_BTS ] ) );}
    UINT16 GetVlanIDbyEid(UINT32);    //add by xiaoweifang: to support vlan
    void   ClearMeasure(UINT32);

#ifdef UNITEST  
public:
#else
private:
#endif
    CTaskDm();
    ~CTaskDm();
    bool ProcessMessage(CMessage&);
    inline TID GetEntityId() const 
    {
        return M_TID_DM;
    }

    #define DM_MAX_BLOCKED_TIME_IN_10ms_TICK (200)
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return DM_MAX_BLOCKED_TIME_IN_10ms_TICK ;};

    //function members
    bool    ProcDataServiceReq(CMessage&);//build EidTbNode with Eid DAIB,Download Prtl,roam Iplist
    void    ProcDownloadACLTableResp(CMessage&);
    void    TimerExpire(CMessage&);
    void    ProcSyncResponse(CMessage&,const UINT16);
    bool    ProcSyncIpListReq(CMessage&);
    void    ProSetACLConfigReq(CMessage&);
    void    ProcDmDataConfig(CMessage&);
    bool    ProcCPEDataConfig(CMessage&);
    void    ProCPELocationMove(UINT32,UINT8);
	
	//lijinan 20101020 for video 
	void ProcCpeVideoAddrRep(CMessage &);

    void    DispatchRoamIplst(const UINT32,const UTILEntry& );  //RoamIplist to Snoop Task
    bool    DispatchRoamFixIplst(const UINT32,const UTILEntry& ); 
    UINT32  DownloadACLTable(CPETable *);

    void    ProcsyncFail(CPETable*,UINT32  );
    void    DispatchSyncResponse(SyncCB*,UINT32);
    bool    SynchronizeDAIBToCPE(CPETable *);
    bool    SynchronizeAddrToCPE(CPETable *);

    void    DelCPETbl_UTIpLst(CPETable*,UINT8);//delete iplist according to MAC
    void    DelCPETbl_AddressFltrTbl(CPETable*,UINT8);//delete Addr according to MAC
    void    DelCPESyncCB(CPETable* );
    bool    CancelTransMsg(UINT16);
    void    DelIplistToSnoop(const UINT8* ,UINT8);
    OPER    SearchInIplist(const UTILEntry*,UINT8,const UTILEntry*,UINT8*); 
    OPER    SearchInIplist(const OPLIST* ,UINT8 ,const UTILEntry* ,UINT8* ) ;
    void    proDownLoadToCpe(CPETable* );
    bool    SendDataServsRSP(const UINT32, const UINT16, bool = true);
    bool    SendSyncRspToSnoop(UINT32 ,UINT8 ,const UINT8* ,bool );

    void    UpdateAddressFilterTable(AddressFltrTb&, UTILEntry&);
    bool    AddToOpList(CPETable*, const UTILEntry*, UINT8&);
    void    SendVLIDToEB(UINT32,UINT16);
    void    ProcFreeDAIB(UINT32);
    void    showEIDTable(UINT32);
    void    showACLTable();
    void    printACL(const PrtclFltrEntry&,UINT8);
    void    clearCPEData(UINT32 eid);
     void ProcCpeProbeReq(CMessage&);
#ifdef __WIN32_SIM__
    void    swap16(CMessage&);
#endif

private:
    /*define1 Task member..............<*/
    UINT8  m_ucWorkingMode;
    bool   m_bMobilityEn; //true: permit Roam of CPE ; false:forbit Roam of CPE
    static CTaskDm* s_ptaskDM;
    /*end define1...............................>*/ 
#ifdef UNITEST  
public:
#else
private:
#endif
    inline void SetWorkingMode(UINT8 ucWorkMd)
    {
        m_ucWorkingMode=ucWorkMd;
    }
    inline UINT8 GetWorkingMode()
    {
        return m_ucWorkingMode;
    }

    //Roam mode methods
    inline void SetMobilityEn(UINT8 ucMobility)
    {
        m_bMobilityEn=true;   //(ucMobility)?true:false; liuweidong
    }
    inline bool GetMobilityEn()
    {
        return m_bMobilityEn;
    }

private:
    /*define2 CPETable member..............<*/
    CPETable m_CPETb[M_MAX_UT_PER_BTS];  // 转发表

    list<UINT16> m_listFreeEID; //空闲转发表表项的链表
    map<UINT32, UINT16> m_EIDptree;  // 转发表索引树
    /*end define2...............................>*/ 
#ifdef UNITEST  
public:
#else
private:
#endif
    // Free EID Records List methods
    void   InitFreeEIDList();
    void   InsertFreeEIDList(UINT16);
    UINT16 GetFreeEIDIdxFromList(void);
    // BPtree methods
    bool   BPtreeAdd(UINT32, UINT16);
    bool   BPtreeDel(UINT32);
    UINT16 BPtreeFind(UINT32);

    CPETable*   GetCPERecordByEID(UINT32 );   
    CPETable*   NewEidTableNode(UINT32 );   //add new eidnode with Eid

private:
    /*define3 ProtocolFilterTb  member..............<*/
    PrtclFltrTb m_ACLTb;
    /*end define3...............................>*/ 
#ifdef UNITEST  
public:
#else
private:
#endif
    inline void  InitPrtclFltTb(void)
    {
        m_ACLTb.ucCount = 0;
        memset(m_ACLTb.PrtlFltTbEntry,0,M_MAX_PROTOCOL_REC*sizeof(PrtclFltrEntry));
    }
    bool GetFixIPList(UTILEntry* , CpeFixIpInfo* );
private:

    CPEPerfm CPEPerfMeasureTb[M_MEASURE_MAX]; //M_MAX_UT_PER_BTS is used to reserve all Measure
#ifdef UNITEST  
public:
#else
private:
#endif
	void   ClearAllMeasure();
    inline void  InitCPEPerfMeasureTb(void)
    {
        memset(CPEPerfMeasureTb,0,M_MEASURE_MAX*sizeof(CPEPerfm));
    }
    //---------------------------------------------
    //获取指定CPE的包统计值
    //---------------------------------------------
    inline UINT32 GetCPEPerfMeasure(UINT32 eid, CPE_PERFORMANCE type )
    {
        DATA_assert( type < MAX_PERFMC );
        UINT16 idx=BPtreeFind(eid);
        if ( M_DATA_INDEX_ERR == idx )
            return M_DATA_PERFMEASURE_ERR;
        return CPEPerfMeasureTb[idx].arPfmMeasure[type];
    }
    //---------------------------------------------
    //获取所有包统计值
    //---------------------------------------------
    inline UINT32 GetCPEPerfMeasure(CPE_PERFORMANCE type )
    {
        DATA_assert( type < MAX_PERFMC );
        return CPEPerfMeasureTb[M_MAX_UT_PER_BTS].arPfmMeasure[type];
    }
    //---------------------------------------------
    //设置指定CPE的某种类型包统计值 +1
    //---------------------------------------------
    inline UINT32 IncreaseCPEPerfMeasureByOne( UINT32 eid, CPE_PERFORMANCE type )
    {
        DATA_assert( type < MAX_PERFMC );
        UINT16 idx=BPtreeFind(eid);
        if ( M_DATA_INDEX_ERR == idx )
            return M_DATA_PERFMEASURE_ERR;
        CPEPerfMeasureTb[M_MAX_UT_PER_BTS].arPfmMeasure[type]+=1;
        return CPEPerfMeasureTb[ idx ].arPfmMeasure[type] += 1;
    }

    //---------------------------------------------
    //设置CPE中Eid,StartTime
    //---------------------------------------------
    inline void SetCPEPerfMeasure(UINT32 eid,UINT32 ultime)
    {
        UINT16 idx=BPtreeFind(eid);
        if ( M_DATA_INDEX_ERR == idx )
            return ;
        CPEPerfMeasureTb[idx].ulEid = eid;
        CPEPerfMeasureTb[idx].ulStartT =ultime;
        if ( !CPEPerfMeasureTb[M_MAX_UT_PER_BTS].ulStartT )
            CPEPerfMeasureTb[M_MAX_UT_PER_BTS].ulStartT=ultime;
        CPEPerfMeasureTb[M_MAX_UT_PER_BTS].ulEid++;
    }
    };

#endif /*__DATA_DM_H__*/
