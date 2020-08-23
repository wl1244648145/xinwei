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
 *   11/22/2007   xiaoweifang  新运营流程.
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#pragma warning (disable : 4786)

#ifndef _INC_L3OAMCPEFSM
#define _INC_L3OAMCPEFSM

#include <stdio.h>
#include <map>
using namespace std;

#include "fsm.h"

#ifndef _INC_L3OAMCPECOMMON
#include "L3oamCpeCommon.h"
#endif

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3CPEMESSAGEID
#include "L3CpeMessageId.h"
#endif

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3OAMCOMMON
#include "L3OamCommon.h"
#endif

#ifndef _INC_L3L3L2OAMCPEREGNOTIFY
#include "L3L3L2CpeRegNotify.h"
#endif

#ifndef _INC_TIMER
#include "Timer.h"
#endif

#include "L3OAMAuth.h"

enum CPESTATUS
{
    CPE_NORECORD = 0,
    CPE_SERVING,
    CPE_WAITFOREMS,
    CPE_MOVEDAWAY,
    CPE_SWITCHOFF,
    MAX_CPE_STATE  
};

/*************************************************
 *strCpeSTATE: CPE_STATE对应的字符串
 *************************************************/
#define M_CPE_STATE_STRLEN  (20)
const UINT8 strCpeSTATE[ MAX_CPE_STATE + 1 ][ M_CPE_STATE_STRLEN ] = {
    "IDLE",
    "Serving",
    "WAIT_for_EMS",
    "Moved Away",
    "Switch Off",
    "Err"
};


typedef enum
{
    CPE_ANYSTATE_REG_NOTIFY = 0,    //老注册流程
    CPE_ANYSTATE_REGISTER_REQ,      //新注册流程
    CPE_NORECORD_PROFILE_UPDATE_REQ,
    CPE_NORECORD_ModifyBWInfo_REQ,
    
////CPE_SERVING_REGISTER_REQ,         
    CPE_SERVING_PROFILE_UPDATE_REQ,
    CPE_SERVING_MOVED_AWAY_NOTIFY,
    CPE_SERVING_CPE_PROFILE_UPDATE,
    CPE_SERVING_ModifyBWInfo_REQ,

//#ifdef RPT_FOR_V6
	CPE_SERVING_RPT_BWINFO_RSP,
    CPE_SERVING_RPT_RFONOFF_REQ,
    CPE_SERVING_RPT_RFONOFF_RSP,
    CPE_SERVING_RPT_CFG_REQ,
    CPE_SERVING_RPT_CFG_RSP,
    CPE_SERVING_RPT_Get_REQ,
    CPE_SERVING_RPT_Get_RSP,
//	CPE_SERVING_RPT_TIMEOUT,
    
////CPE_WAITFOREMS_REGISTER_REQ,      
    CPE_WAITFOREMS_PROFILE_UPDATE_REQ,      
    CPE_REGISTER_TIMER,
    CPE_WAITFOREMS_ModifyBWInfo_REQ,

////CPE_MOVEDAWAY_REGISTER_REQ,    
    CPE_MOVEDAWAY_RECORD_KEEPING_TIMER,
    
////CPE_SWITCHEDOFF_REGISTER_REQ,    
    CPE_SWITCHEDOFF_PROFILE_UPDATE_REQ,    
    CPE_SWITCHEDOFF_MOVEDAWAY_NOTIFY, 
    CPE_SWITCHEDOFF_RECORD_KEEPING_TIMER,
    CPE_SWITCHEDOFF_ModifyBWInfo_REQ,

    CPE_ANYSTATE_SWITCHEDOFF,
    CPE_Z_REGISTER_REQ,     //only for SERVING & WAITFOREMS state.

    MAX_CPE_TRANS
}CPE_TRANS;

class CPECCB;
typedef map<UINT32, CPECCB *>::value_type CpeValType;
typedef map<UINT32, CPECCB *>::iterator   CpeIter;

#define M_CCB_TYPE_OLD  (0)
#define M_CCB_TYPE_NEW  (1)

class CPECCB: public CCBBase
{
public:
    CPECCB (CCBBase &):m_pCpeRegTimer(NULL),m_updUTProfileTransId(0),m_BWInfoReqTransId(0),m_ulRegTime(0),m_RegisterReqTransId(0),m_ulEID(0),m_ulUID(0),m_ucCCBtype(M_CCB_TYPE_NEW)
    {
        memset(&m_UTBaseInfo,    0, sizeof(m_UTBaseInfo));
        memset(&m_UTProfile,     0, sizeof(m_UTProfile));
        memset(&m_ZmoduleBlock,  0, sizeof(m_ZmoduleBlock));
        memset(&m_L2specialFlag, 0, sizeof(m_L2specialFlag));
	memset(&UTHoldBW,0,sizeof(UTHoldBW));//wangwenhua add 20080916
	m_CFG_TransID = 0xffff;
	m_Query_TransID = 0xffff;//wangwenhua add 20081119
	m_UtDlFlag = 0;
       m_RptFlag = 0;
       m_WcpeORRcpeFlag = 0;
       m_rcvProfileinValidFlag = 0;
       m_sagDefaultFlag = 0;
    };
    
    CPECCB ():CCBBase(CPE_NORECORD),m_pCpeRegTimer(NULL),m_updUTProfileTransId(0),m_BWInfoReqTransId(0),m_ulRegTime(0),m_RegisterReqTransId(0),m_ulEID(0),m_ulUID(0),m_ucCCBtype(M_CCB_TYPE_NEW)
    {
        memset(&m_UTBaseInfo,    0, sizeof(m_UTBaseInfo));
        memset(&m_UTProfile,     0, sizeof(m_UTProfile));
        memset(&m_ZmoduleBlock,  0, sizeof(m_ZmoduleBlock));
        memset(&m_L2specialFlag, 0, sizeof(m_L2specialFlag));
	memset(&UTHoldBW,0,sizeof(UTHoldBW));//wangwenhua add 20080916
	m_CFG_TransID = 0xffff;
	m_Query_TransID = 0xffff;//wangwenhua add 20081119
	m_UtDlFlag = 0;
       m_RptFlag = 0;
       m_WcpeORRcpeFlag = 0;
       m_rcvProfileinValidFlag = 0;
       m_sagDefaultFlag = 0;
    };
public:
    UINT8  getAdminStatus()const;
    void   setAdminStatus(const UINT8&);
    UINT8  getMobility()const;
    UINT8  getDHCPrenew()const;
    UINT16 getVlanID()const;
    UINT16 getMaxIPnum()const;
    UINT16 getFixIpnum()const;
    UINT8  getPerfLogStatus()const;
    UINT16 getPerfDataCollectInterval()const;
    UINT32 getVoicePortMask()const;
    const T_UTSDCfgInfo& getUTSDCfg()const;
    const T_CpeFixIpInfo* getFixIPinfo()const;

    void setUTProfile(const T_UTProfile&);
    void setUTProfile(const T_UTProfileNew&);
    const T_UTProfile & getUTProfile()const;

    bool ProfileCompare(const T_UTProfileNew&);
    bool ProfileCompare(const T_UTProfile &);

    bool   setUTBaseInfo(const T_CpeBaseInfo&);
    bool   setUTBaseInfo(const T_UTBaseInfo&);
#if 0
    void   getCpeBaseInfo(T_CpeBaseInfo&)const;
#endif
    const T_UTBaseInfo& getCpeBaseInfo()const;
    void   setEid(UINT32 eid){ m_ulEID = eid; }
    UINT32 getEid()const     { return m_ulEID; }
    void   setUid(UINT32 uid){ m_ulUID = uid; }
    UINT32 getUid()const     { return m_ulUID; }
////UINT32 getCid()const     { return m_UTBaseInfo.ulCID; }
    UINT8  getPrdRegTimeValue()const 
        {
        UINT8 val = m_UTProfile.UTProfileIEBase.ucPeriodicalRegisterTimerValue;
        if(0 == val)
            return gDefaultPrdRegTime;
        if(val > gMAXPrdRegTime)
            return gMAXPrdRegTime;
        return val;
        }

    void   setUpdUTProfileTransId(UINT16 transid) {m_updUTProfileTransId = transid;}
    UINT16 getUpdUTProfileTransId()const          {return m_updUTProfileTransId;}
    void   setBWInfoTransId(UINT16 transid) {m_BWInfoReqTransId = transid;}
    UINT16 getBWInfoTransId()const          {return m_BWInfoReqTransId;}
    void   setCCBType(UINT8 type)   {m_ucCCBtype = type;}
    UINT8  getCCBType()const        {return m_ucCCBtype;}

////Add for Z_module.
    UINT16 getCpeHWType() const    {return m_UTBaseInfo.usHWtype;}
    void   setZmoduleBlock(T_ZmoduleBlock *pZB);
    T_ZmoduleBlock* getZmoduleBlock();

    bool   getRegisterStatus(const UINT16);
    void   setSpecialFlag(T_L2SpecialFlag flag)         {memcpy(&m_L2specialFlag, &flag, sizeof(flag));}
    void   getSpecialFlag(T_L2SpecialFlag &flag)const   {memcpy(&flag, &m_L2specialFlag, sizeof(flag) );}
   void  setUTHoldBW(const T_UT_HOLD_BW&);//wangwenhua 20080916
    T_UT_HOLD_BW* getUTHoldBW() {return &UTHoldBW;}
	 bool HoldBWCompare(const T_UT_HOLD_BW&);    
     void  setMemCfg(const T_MEM_CFG&);
     const T_MEM_CFG& getMemCfg()const;
    bool MemCfgCompare(const T_MEM_CFG &memCfg);
   bool setCfgTranid(const UINT16);
   bool setQueryTransid(const UINT16);
   UINT16 getCfgTranid();
   UINT16 getQueryTransid();
   void setUtDlFlag(UINT8 flag){m_UtDlFlag = flag;};
   UINT8 getUtDlFlag(){return m_UtDlFlag;};
   void setRptFlag(UINT16 flag){m_RptFlag = flag;};
   UINT16 getRptFlag(){return m_RptFlag;};
   void setWcpeORRcpeFlag(UINT16 flag){m_WcpeORRcpeFlag = flag;};
   UINT16 getWcpeORRcpeFlag()const{return m_WcpeORRcpeFlag;};
   void setProfileInvalidFlag(UINT16 flag){m_rcvProfileinValidFlag = flag;};
   UINT16 getProfileInvalidFlag()const{return m_rcvProfileinValidFlag;};
   void setSagDefaultFlag(UINT16 flag){m_sagDefaultFlag = flag;};
   UINT16 getSagDefaultFlag()const{return m_sagDefaultFlag;};
////data members:
public:
    CTimer          *m_pCpeRegTimer;
private:
    UINT8           m_ucCCBtype;          //0-old CPE registration CB; 1-new CPE registration CB
    UINT32          m_ulUID;
    UINT32          m_ulEID;
    T_UTBaseInfo    m_UTBaseInfo;
    T_UTProfile     m_UTProfile;
    //UINT16          m_usPeriodicalRegisterTimerValue;   //周期注册定时器, hours
    UINT16          m_updUTProfileTransId;  //CPE update profile的 transaction.
    UINT16          m_BWInfoReqTransId;     //send BWInfo的 transaction.
    UINT32          m_ulRegTime;        //过滤突发性的CPE注册请求,系统有时候1s内接受到从同一CPE上来的很多注册请求
    UINT16          m_RegisterReqTransId;
    T_ZmoduleBlock  m_ZmoduleBlock;
    T_L2SpecialFlag m_L2specialFlag;
     T_UT_HOLD_BW UTHoldBW;//wangwenhua add 20080916   
     T_MEM_CFG m_MemCfg;
    UINT16        m_CFG_TransID;  //wangwenhua add 20081119
    UINT16        m_Query_TransID;//wangwenhua add 20081119
    UINT16 m_UtDlFlag;//终端加载标志
    UINT16 m_RptFlag;
    UINT16 m_WcpeORRcpeFlag;
    UINT16 m_rcvProfileinValidFlag;
    UINT16 m_sagDefaultFlag;//0:not default, 1: yes
};



class CPECCBTable: public CCBTableBase
{
public:
    CPECCBTable(){};
    CCBBase *FindCCB(CMessage &msg);
    CCBBase *FindCCB(const UINT32);
    void DeleteCCB(UINT32 CpeID);
//static
    UINT32 size()   {return CpeCCBTable.size();};
private:
//static    
    map<UINT32, CPECCB*> CpeCCBTable;  // UINT32 CPEID, CPECCB *CpeCcb        
};

class CPEFSM: public FSM
{
public:
    CPEFSM ();
    UINT8 GetStateNumber() { return  MAX_CPE_STATE; }
    UINT8 GetTransNumber() { return  MAX_CPE_TRANS; }
    CCBBase *FindCCB(CMessage &msg){ return m_pCCBTable->FindCCB(msg);};
    CCBBase *FindCCB(const UINT32 eid){ return m_pCCBTable->FindCCB(eid);};
    void DeleteCCB(UINT32 CpeID){ m_pCCBTable->DeleteCCB(CpeID);};
    UINT32 size(){return m_pCCBTable->size();};
    void Reclaim(CCBBase*);
private:
    CPECCBTable *m_pCCBTable;
};
#endif 
