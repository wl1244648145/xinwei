/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    DM.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   04/10/06   xiao weifang  BTS不支持切换漫游需要的处理
 *   03/28/06   xiao weifang  FixIp改成非永久性用户。提供FixIp查询接口
 *   09/26/05   yang huawei   initialization. 
 *
 *---------------------------------------------------------------------------*/

//禁用Winsock 1
#define _WINSOCKAPI_
#include <stdio.h>
#ifdef __WIN32_SIM__
    #include <Winsock2.h>
#endif

#include <string.h>
#include <time.h>

#include "Object.h"
#include "MsgQueue.h"
//#include "Message.h"
#include "Transaction.h"
#include "LogArea.h"

#include "L3dataCommon.h"
#include "L3dataMsgId.h"
#include "L3DataSyncIL.h"
#include "L3DataRoam.h"
#include "L3DataDelIL.h"
#include "L3DataFixIp.h"
#include "L3DataFTCheckVLAN.h"

#include "L3DataDm.h"
#include "L3DataEB.h"
#include "L3DataDmSyncMsg.h"
#include "L3DataACLConfig.h"
#include "L3DataDmConfig.h"
#include "L3DataDmMessage.h"
#include "L3OAMMessageId.h"
#include "ErrorCodeDef.h"
#include "L3DataAssert.h"

#ifdef __WIN32_SIM__
extern bool   WriteConfig(const UINT8 *pConfig, UINT32 ulLen);
extern UINT16 GetConfigLen();
extern bool   ReadConfig(UINT8 *pConfig);
#endif

typedef map<UINT32, UINT16>::value_type ValType;

#define M_DM_FLOWCTRL_CNT  (100)
//任务实例指针的初始化
CTaskDm* CTaskDm::s_ptaskDM = NULL;

//外部函数声明
extern     bool   GetCPEConfig(UINT32 ,UINT8*);
extern     UINT32 GetRouterAreaId();
extern "C" int    bspGetBtsID();
extern T_NvRamData *NvRamDataAddr;

#ifdef M_TGT_WANIF
extern UINT32  WanIfCpeEid;
extern UINT32  BakWanIfCpeEid;
extern UINT32  RelayWanifCpeEid[20] ;
#endif
unsigned int g_dm_no_freelist = 0;
/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProcessMessage

DESCRIPTION:
    DM任务消息处理函数
ARGUMENTS:
    *CMessage: 消息
RETURN VALUE:
    bool:TRUE or false,FrameWork根据返回值决定是否做PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskDm::ProcessMessage(CMessage &cMsg)
{
#ifdef __WIN32_SIM__
    swap16( cMsg );
#endif

    UINT16 usMsgId = cMsg.GetMessageId();
    switch ( usMsgId )
        {
        case MSGID_IPLIST_SYNC_REQ:
            // DYNIPLIST synchronize request
            ProcSyncIpListReq(cMsg);
            break;

        case MSGID_UT_BTS_DATASERVICE_REQ:
            // DataServerReq
            if ( ProcDataServiceReq( cMsg ) )
                {
                IncreaseCPEPerfMeasureByOne(cMsg.GetEID(),DATA_SERVICE_REQ);
                }
            break;
	 case MSGID_VEDIO_IPADDRESS_REPORT:
            // lijinan 20101020 for video
          	ProcCpeVideoAddrRep( cMsg );
            break;
        case MSGID_UT_BTS_DAIBUPDATE_RESP:
            // DAIB Response
            ProcSyncResponse(cMsg,usMsgId);
            IncreaseCPEPerfMeasureByOne(cMsg.GetEID(),RECV_SYNC_IP_RSP);
            break;

        case MSGID_UT_BTS_ADDRFLTR_TABLEUPDATE_RESP:
            // 
            ProcSyncResponse(cMsg,usMsgId);
            IncreaseCPEPerfMeasureByOne(cMsg.GetEID(),RECV_SYNC_IP_RSP);
            break;

        case MSGID_EID_DEL_TABLE:
            // Del eid table
            ProcFreeDAIB(cMsg.GetEID());
            break;

        case MSGID_TIMER_DM:
            // Timer message
            TimerExpire( cMsg );
            break;

        case M_CFG_DM_DATA_SERVICE_CFG_REQ:
            ProcDmDataConfig(cMsg);  
            break;

        case M_CPEM_DM_CPE_DATA_CFG_NOTIFY:
            if ( ProcCPEDataConfig(cMsg) )
                {
                IncreaseCPEPerfMeasureByOne(cMsg.GetEID(),CPE_CONIFG);
                }
            break;          

        case M_EMS_BTS_CFG_ACL_REQ:
            ProSetACLConfigReq(cMsg);
            break;

        case M_CM_DM_DELETE_UT_NOTIFY :
            ProCPELocationMove(cMsg.GetEID(),M_UT_REMOVE);
            break;

        case M_EMS_BTS_UT_MOVEAWAY_NOTIFY:
            ProCPELocationMove(cMsg.GetEID(),M_UT_MOVE_AWAY);
            break;

        case MSGID_UT_BTS_PROTOCOLFLTR_TABLEUPDATE_RESP:
            // ACLResponse
            ProcDownloadACLTableResp(cMsg);
            IncreaseCPEPerfMeasureByOne(cMsg.GetEID(),RECV_SYNC_ACL_RSP);
            break;

        case M_CPEM_DM_CPE_PROBE_REQ:
	     ProcCpeProbeReq(cMsg);
	     break;
        default :
            LOG1( LOG_DEBUG, LOGNO(DM,ERR_DM_UNEXPECTED_MSGID), "->ProcessMessage: Unexpected message, MsgId: 0x%x.", usMsgId );
            break;
        }

#ifdef __WIN32_SIM__
    showStatus();
#endif
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::GetInstance

DESCRIPTION:
    Get CTaskDm Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTaskDm* 

SIDE EFFECTS:
    none
==============================================================*/
CTaskDm* CTaskDm::GetInstance()
{
    if ( NULL == s_ptaskDM )
        {
        s_ptaskDM = new CTaskDm;
        }
    return s_ptaskDM;
}
/*============================================================
MEMBER FUNCTION:
    CTaskDm::CTaskDm

DESCRIPTION:
    CTDm构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/

CTaskDm::CTaskDm()
{
    LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->CTaskDm()" );

#ifndef NDEBUG
    if ( !Construct( CObject::M_OID_DM ) )
        {
        LOG( LOG_SEVERE, LOGNO(DM,ERR_DM_SYS_ERR), "->ERROR!!!CTaskDm()% Construct failed." );
        }
#endif

    memset( m_szName, 0, sizeof( m_szName ) );
    memcpy( m_szName, M_TASK_DM_TASKNAME, strlen( M_TASK_DM_TASKNAME ) );
    m_uPriority     = M_TP_L3DM;
    m_uOptions      = M_TASK_DM_OPTION;
    m_uStackSize    = M_TASK_DM_STACKSIZE;
    m_iMsgQMax      = M_TASK_DM_MAXMSG;
    m_iMsgQOption   = M_TASK_DM_MSGOPTION;
    m_ucWorkingMode = WM_NETWORK_AWARE;
    m_bMobilityEn   = true;

    InitFreeEIDList();
    memset((void*)m_CPETb,0,sizeof(m_CPETb));
    InitPrtclFltTb();
    InitCPEPerfMeasureTb();
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::~CTaskDm

DESCRIPTION:
    CTDm析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTaskDm::~CTaskDm()
{
    LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->~CTaskDm" );

#ifndef NDEBUG
    if ( !Destruct( CObject::M_OID_DM ) )
        {
        LOG( LOG_SEVERE,LOGNO(DM,ERR_DM_SYS_ERR), "->ERROR!!!~CTaskDm failed." );
        }
#endif
}

/*============================================================
MEMBER FUNCTION:
    CTaskDm::GetCPEProfile

DESCRIPTION:
    return profiles of the requested CPE,
ARGUMENTS:
    DstBuff with the format of CPEProfile
    {bool Mobility;
     bool DHCPRenew;
     bool IsFull; //true:DYNIPLIST have no space,disable to add new;otherwise enable
     }
RETURN VALUE:

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskDm::GetCPEProfile(UINT32 Eid, CPEProfile *tmpCPEpro )
{
   int i;
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->GetCPEProfile(0x%X)",Eid );
    if ( NULL ==tmpCPEpro )
        {
        LOG( LOG_SEVERE, LOGNO(DM,ERR_DM_PARAMETER), "->GetCPEProfile:parameter point is NULL" );
        return false;
        }
    CPETable *RectCPE=GetCPERecordByEID(Eid);
    if ( NULL == RectCPE )
        {
        LOG1( LOG_SEVERE, LOGNO(DM,ERR_DM_NO_CB), "->GetCPEProfile:EID[0x%.8X] no found.",Eid );
        return false;
        }
    bool isRelayCpe = false;

   if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
       for( i = 0; i<NvRamDataAddr->Relay_num ;i++)
       {
           if(RelayWanifCpeEid[i] ==Eid )
           {
               isRelayCpe = true;
               break;
           }
       
       }	
    tmpCPEpro->bDHCPRenew= (RectCPE->stConfig.ucRenew)?true:false;
    tmpCPEpro->bMobility= (RectCPE->stConfig.ucMobilityEn)?true:false;
    tmpCPEpro->bIsFull= false;
    if(isRelayCpe ==true)
    	{
    	return true;//rcpe user no limit 
    	}
    if ( RectCPE->ucMaxSize<=(RectCPE->ucFixSize+RectCPE->ucDynIpSize) )
        tmpCPEpro->bIsFull = true;

    return true;
}

//add by xiaoweifang: to support vlan.{{
/*============================================================
MEMBER FUNCTION:
    CTaskDm::GetVlanIDbyEid

DESCRIPTION:
    get vlan id.

ARGUMENTS:
    EID:

RETURN VALUE:
    vlan-id
    (actually group-id//2007.5.14)

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTaskDm::GetVlanIDbyEid(UINT32 Eid)
{
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->GetVlanIDbyEid(0x%X)",Eid );

    CPETable *RectCPE=GetCPERecordByEID(Eid);
    if ( NULL == RectCPE )
        {
        LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NO_CB), "->GetVlanIDbyEid:EID[0x%.8X] no found.",Eid );
        return M_NO_VLAN_TAG;
        }
    return RectCPE->stConfig.usVlanId;
}
//}}add by xiaoweifang.


////是否允许该用户移动漫游切换
bool CTaskDm::IsBTSsupportMobility(UINT32 ulLocalRAID, UINT32 ulEntryRAID, UINT32 ulEntryAnchorBts)
{

    return  !( 
               ( ( false == m_bMobilityEn )/* || ( false == pEidTable->Config.stConfig.ucMobilityEn )*/ )       /*CPE是否支持漫游由SNOOP判断*/
            && (   ( ulEntryRAID != ulLocalRAID ) 
               ||( /*( ulEntryRAID == ulLocalRAID ) &&*/ ( ulEntryAnchorBts != bspGetBtsID() ) )
               ) 
            );

   //return  1;
            
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProcDataServiceReq

DESCRIPTION:
    if MobilityEn,Deal with CPE dataservice Request,
    set up new EidNode,Roam DYNIPLIST to Snoop task,
    Update DAIB&AddressFltTable,
    down load ACLCtrlTb,
    otherwise,return FAIL message to CPE;
ARGUMENTS:

RETURN VALUE:

SIDE EFFECTS:

==============================================================*/
#define M_ROAM_FLAG     (0x80)
#include "L3DataSnoop.h"
bool CTaskDm::ProcDataServiceReq(CMessage &CMsg)
{
    UINT32  ulEid=CMsg.GetEID();
    LOG1(LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcDataServiceReq(0x%X)" ,ulEid);
    UINT32 rectRouterID = ::GetRouterAreaId();
    if ( M_RAID_INVALID == rectRouterID )
    {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_CFG_ERR), "->Router Area Id is InValid" );
        return false;
    }
    
    CDataServiceReq DataSerReq(CMsg);
    
    CPETable *RectCPE=GetCPERecordByEID(ulEid);
    if ( NULL == RectCPE )
    {//CPE first Regist
        
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "CPE register first time" );
        if ( (RectCPE = NewEidTableNode(ulEid)) == NULL )
        {
            LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NO_CB), "EID table create fail. CPE register FAIL." );
            //FAIL response.
            SendDataServsRSP(ulEid, 0, false);
            return false;
        }
        RectCPE->regTime = (UINT32)time( NULL );
        if(M_DATA_SOFT_VERSION !=DataSerReq.GetVersion())
        {//if SOFTVERSION not match,clear DAIB and Response
            LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "CPE VERSION not match with BTS" );
            RectCPE->pstSyncCB->ucType = M_SYNC_BOTH;
            RectCPE->ucPrtlCtrlState = STATUS_PEND;
            RectCPE->ucNeedDataRsp = STATUS_PEND;
            proDownLoadToCpe(RectCPE);
            return true;
        }
        UINT8  ucsynctype=0;
        UINT32 ucIpCount=DataSerReq.GetIpCount();
        if ( ( DataSerReq.GetDataLength() <(sizeof(DataServsReq)-M_MAX_USER_PER_UT*sizeof(IplistTLV)))
        || ( 0 == ucIpCount )||( M_MAX_USER_PER_UT < ucIpCount ) )
        {
            //DAIB = NULL
            //删除bts重启可能恢复的，该cpe的用户
            clearCPEData(ulEid);
            memset(RectCPE->pstIplstTbEnty+RectCPE->ucFixSize,0,(RectCPE->ucDynIpSize)*sizeof(UTILEntry));
            memset(RectCPE->pstAddrFltrTbEntry+RectCPE->ucFixSize,0,(RectCPE->ucDynIpSize)*sizeof(AddressFltrTb)); 
            RectCPE->ucDynIpSize = 0;
            if(0 != RectCPE->ucFixSize)
            {
                RectCPE->pstSyncCB->ucType |= M_SYNC_ADDRONLY;
            }
            RectCPE->ucPrtlCtrlState = STATUS_PEND;
            RectCPE->ucNeedDataRsp = STATUS_PEND;
            proDownLoadToCpe(RectCPE);
            return true;
        }
        
        UINT32 ulTime=time(NULL);
        UTILEntry *stIpLst=NULL;
        IplistTLV *piplstTLV = (IplistTLV*)DataSerReq.GetDAIB();
        for (UINT8 i=0;i<ucIpCount;i++)
        {
            if (RectCPE->ucDynIpSize + RectCPE->ucFixSize >= RectCPE->ucMaxSize)
            {
                //full.
                break;
            }
            
            UINT16 vLenth=piplstTLV->Len;
            if(vLenth != sizeof(UTILEntry))
            {
                piplstTLV++;
                continue;
            }
            UINT8 ipType = piplstTLV->Type;
            stIpLst = &( piplstTLV->Iplist);
            piplstTLV++;
            
            if ( DYNIPLIST == ipType )
            {
                if ( (IPTYPE_DHCP == stIpLst->ucIpType) &&
                ((ulTime > stIpLst->Data.stIpLease.ulLease)||(true == CTSnoop::GetInstance()->isLeaseUpdated(ulEid, stIpLst->Data.stIpLease.ulLease, stIpLst->aucMAC)))
                )
                {
                    /*out of lease date,cancel DYNIPLIST*/
                    LOG6( LOG_SEVERE, LOGNO(DM,ERR_DM_NORMAL), "User[%.2X-%.2X-%.2X-%.2X-%.2X-%.2X] is refused, possibly it is out of DHCP lease time or login from another terminal",
                    stIpLst->aucMAC[0],
                    stIpLst->aucMAC[1],
                    stIpLst->aucMAC[2],
                    stIpLst->aucMAC[3],
                    stIpLst->aucMAC[4],
                    stIpLst->aucMAC[5] );
                    ucsynctype |= M_SYNC_BOTH; //M_SYNC_IPLISTONLY//切换过程中,cpe可能保留有地址表
                    continue;
                }
                
                if ( false == IsBTSsupportMobility( rectRouterID, stIpLst->ulRouterAreaId, stIpLst->ulAnchorBts ) )
                {/*not supporting Mobility and not in the same RouterArea,cancel DYNIPLIST*/
                    LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcDataServiceReq:BTS mobility disable." );
                    ucsynctype |= M_SYNC_BOTH; //M_SYNC_IPLISTONLY//切换过程中,cpe可能保留有地址表
                    continue;
                }
                
                UINT8 uciplocation =(UINT8)( RectCPE->ucFixSize+RectCPE->ucDynIpSize);
                memcpy(RectCPE->pstIplstTbEnty+uciplocation,stIpLst,vLenth); 
                RectCPE->ucDynIpSize++;
                DispatchRoamIplst(ulEid,*stIpLst); //Roam DYNIPLIST To Snoop
                
                UpdateAddressFilterTable( RectCPE->pstAddrFltrTbEntry[uciplocation], *stIpLst );
                
                ucsynctype |=M_SYNC_ADDRONLY;
            }
        }
        RectCPE->pstSyncCB->ucType |= ucsynctype;
        RectCPE->ucPrtlCtrlState    = STATUS_PEND;
        RectCPE->ucNeedDataRsp      = STATUS_PEND;
        proDownLoadToCpe(RectCPE);
        return true;
    }
    else
    {
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcDataServiceReq:CPE Regist repeatly" );
        RectCPE->regTime = (UINT32)time( NULL );
        /*exist CPENodeTable*/
        SyncCB *pSyncCB = RectCPE->pstSyncCB;
        if ( NULL == pSyncCB )
        {
            pSyncCB= (SyncCB*) new UINT8[ sizeof(SyncCB) ];
            if ( NULL == pSyncCB )
            {
                LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->ProcDataServiceReq:Create SyncCB fail." );
                return false;
            }
            memset(pSyncCB,0,sizeof(SyncCB));
            RectCPE->pstSyncCB = pSyncCB;
        }
        
        if(M_DATA_SOFT_VERSION !=DataSerReq.GetVersion())
        {//if SOFTVERSION not match,clear DAIB and Response
            LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcDataServiceReq:CPE VERSION not match with BTS" );
            RectCPE->pstSyncCB->ucType = M_SYNC_BOTH;
            RectCPE->ucPrtlCtrlState = STATUS_PEND;
            RectCPE->ucNeedDataRsp = STATUS_PEND;
            proDownLoadToCpe(RectCPE);
            return true;
        }
        UINT32 ucIpCount=DataSerReq.GetIpCount();
        if ( (  DataSerReq.GetDataLength() <(sizeof(DataServsReq)-M_MAX_USER_PER_UT*sizeof(IplistTLV)))
        || ( 0 == ucIpCount )||( M_MAX_USER_PER_UT < ucIpCount ) )
        {
            //DAIB = NULL
            for (UINT8 i=RectCPE->ucFixSize;i<RectCPE->ucFixSize+RectCPE->ucDynIpSize;i++ )
            {
                DelIplistToSnoop(RectCPE->pstIplstTbEnty[i].aucMAC,M_UT_REMOVE); 
            }
            memset(RectCPE->pstIplstTbEnty+RectCPE->ucFixSize,0,(RectCPE->ucDynIpSize)*sizeof(UTILEntry));
            memset(RectCPE->pstAddrFltrTbEntry+RectCPE->ucFixSize,0,(RectCPE->ucDynIpSize)*sizeof(AddressFltrTb)); 
            RectCPE->ucDynIpSize = 0;
            if(0 != RectCPE->ucFixSize)
            {
                pSyncCB->ucType |= M_SYNC_ADDRONLY;
            }
            RectCPE->ucPrtlCtrlState = STATUS_PEND;
            RectCPE->ucNeedDataRsp = STATUS_PEND;
            proDownLoadToCpe(RectCPE);
            return true;
        }
        
        IplistTLV *piplstTLV = (IplistTLV*)DataSerReq.GetDAIB();
        
        UINT32 ulTime=time(NULL);
        UINT8  ucsynctype=0;
        UTILEntry tmpiplstbuf[M_MAX_USER_PER_UT];
        memset(tmpiplstbuf,0,M_MAX_USER_PER_UT*sizeof(UTILEntry));
        memcpy(tmpiplstbuf,RectCPE->pstIplstTbEnty,RectCPE->ucMaxSize*sizeof(UTILEntry));
        
        memset(RectCPE->pstIplstTbEnty+RectCPE->ucFixSize,0,RectCPE->ucDynIpSize*sizeof(UTILEntry));
        memset(RectCPE->pstAddrFltrTbEntry+RectCPE->ucFixSize,0,RectCPE->ucDynIpSize*sizeof(AddressFltrTb));
        RectCPE->ucDynIpSize = 0;
        for ( UINT8 i=0;i<ucIpCount;i++ )
        {
            if ( RectCPE->ucDynIpSize+RectCPE->ucFixSize >= RectCPE->ucMaxSize )
            {
                //full.
                break;
            }
            UINT16 vLenth = piplstTLV->Len;
            if ( vLenth!=sizeof(UTILEntry) )
            {
                LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->ProcDataServiceReq:DAIB format Err." );
                piplstTLV++;
                continue;
            }
            
            UINT8 ipType = piplstTLV->Type;
            UTILEntry *stIpLst = &(piplstTLV->Iplist);
            piplstTLV++;
            UINT8 uciplocation = 0;
            OPER brslt = SearchInIplist(tmpiplstbuf,RectCPE->ucMaxSize,stIpLst,&uciplocation);
            if(EQ == brslt)
            {
                tmpiplstbuf[uciplocation].ucIpType |= M_ROAM_FLAG;
            }
            if ( (SIMIEQ == brslt) && (DYNIPLIST == ipType))
            {
                //flag this old iplist which needs to notify SnoopTaks to delete in the following step 
                stIpLst->ucIpType |= M_ROAM_FLAG;
                ucsynctype |= M_SYNC_ADDRONLY;
            }
            else if ( NEQ == brslt)
            {
                if(DYNIPLIST == ipType)
                {
                    stIpLst->ucIpType |= M_ROAM_FLAG;
                    ucsynctype |= M_SYNC_ADDRONLY;
                }
            }
            if ( (IPTYPE_DHCP == (0x0f & stIpLst->ucIpType)) && 
            ((ulTime > stIpLst->Data.stIpLease.ulLease)||(true == CTSnoop::GetInstance()->isLeaseUpdated(ulEid, stIpLst->Data.stIpLease.ulLease, stIpLst->aucMAC)))
            )
            {//out of lease date,cancel DYNIPLIST
                ucsynctype |= M_SYNC_BOTH;  //M_SYNC_IPLISTONLY//切换过程中,cpe可能保留有地址表
                LOG6( LOG_SEVERE, LOGNO(DM,ERR_DM_NORMAL), "User[%.2X-%.2X-%.2X-%.2X-%.2X-%.2X] is refused, possibly it is out of DHCP lease time or login from another terminal",
                stIpLst->aucMAC[0],
                stIpLst->aucMAC[1],
                stIpLst->aucMAC[2],
                stIpLst->aucMAC[3],
                stIpLst->aucMAC[4],
                stIpLst->aucMAC[5] );
                continue;
            }
            if ( false == IsBTSsupportMobility( rectRouterID, stIpLst->ulRouterAreaId, stIpLst->ulAnchorBts ) )
            {//not supporting Mobility and not in the same RouterArea,cancel DYNIPLIST;
                LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcDataServiceReq:BTS mobility disable." );
                ucsynctype |= M_SYNC_BOTH;
                continue;
            }
            if(DYNIPLIST == ipType)
            {
                uciplocation = RectCPE->ucFixSize+RectCPE->ucDynIpSize;
                memcpy(RectCPE->pstIplstTbEnty+uciplocation,stIpLst,vLenth); 
                RectCPE->ucDynIpSize++;
            }
        }
        
        CMac srcMac;
        for (UINT8 i=RectCPE->ucFixSize;i<RectCPE->ucMaxSize;i++ )
        {
            /*notifying Snoop to delete the old Iplist */
            srcMac.SetMac(tmpiplstbuf[i].aucMAC);
            if ( !srcMac.IsZero() )
            {
                if(!(M_ROAM_FLAG & tmpiplstbuf[i].ucIpType))
                {
                DelIplistToSnoop(tmpiplstbuf[i].aucMAC,M_UT_REMOVE); 
                }
            }
        
            /*notifying Snoop to Add the new Iplist*/
            srcMac.SetMac(RectCPE->pstIplstTbEnty[i].aucMAC);
            if ( !srcMac.IsZero() )
            {
                UpdateAddressFilterTable( RectCPE->pstAddrFltrTbEntry[i], RectCPE->pstIplstTbEnty[i] );
                
                // if ( M_ROAM_FLAG & RectCPE->pstIplstTbEnty[i].ucIpType )//wangwenhua mark 20090505
                {
                RectCPE->pstIplstTbEnty[i].ucIpType &= ~M_ROAM_FLAG;
                DispatchRoamIplst(ulEid, RectCPE->pstIplstTbEnty[i]);
                }
            }
        }
            
        ucsynctype |= M_SYNC_ADDRONLY;
        pSyncCB->ucType |= ucsynctype;
        RectCPE->ucPrtlCtrlState = STATUS_PEND;
        RectCPE->ucNeedDataRsp = STATUS_PEND;
        proDownLoadToCpe(RectCPE);
        return true;        
    }
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::clearCPEData

DESCRIPTION:
    cpe注册后如果DAIB为空,则通知snoop检查是否存在该cpe的数据
    有则删除。fixedip除外

ARGUMENTS:

RETURN VALUE:

SIDE EFFECTS:
  
==============================================================*/
void CTaskDm::clearCPEData(UINT32 eid)
{
    LOG1(LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "DAIB is NULL, clear ALL CPE[%.8x] related data from BTS." ,eid);
    CclearCPEData msgClearCPEData;
    if(true == msgClearCPEData.CreateMessage(*this))
        {
        msgClearCPEData.SetDstTid( M_TID_SNOOP );
        msgClearCPEData.SetSrcTid( M_TID_DM );
        msgClearCPEData.SetEID(eid);

        if ( false == msgClearCPEData.Post() )
            {
            msgClearCPEData.DeleteMessage();
            } 
        }
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::TimerExpire

DESCRIPTION:
    deal with resenderTimer or SynchroTimer 
ARGUMENTS:

RETURN VALUE:

SIDE EFFECTS:
  
==============================================================*/
void CTaskDm::TimerExpire(CMessage &msg)
{
    UINT16 TransID;   
    CDmTimerExpire msgTimer(msg);
    UINT32 ulEid=msgTimer.GetInEid();
    
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->TimerExpire(0x%X)" ,ulEid);
    //直接删除消息，避免下面退出了jiaying20100813
    TransID = msgTimer.GetTransactionId();
    if ( 0!=TransID )//cancel Trans
    {
        CancelTransMsg(TransID);
    }
    CPETable *RectCPE=GetCPERecordByEID(ulEid);
    if ( NULL == RectCPE )
    {//no exist EidNode
        LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NO_CB), "->TimerExpire:EID[0x%.8X]no found.",ulEid );
        return;
    }
    if ( RECEND_TIMER == msgTimer.GetTimerType() )
    {//ACL
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->TimerExpire:ACL Timeout." );
        if ( STATUS_IDLE == RectCPE->ucPrtlCtrlState )
        {
            return;
        }        
        RectCPE->usPrtlCtrlTransId=0;
        RectCPE->ucPrtlCtrlState=STATUS_IDLE;
        //统计包
        IncreaseCPEPerfMeasureByOne(ulEid,SYNC_ACL_FAIL);
        return;
    }
    if ( SYNC_TIMER== msgTimer.GetTimerType() )
    {//Sync
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->TimerExpire:DAIB Timeout." );
        if ( NULL == RectCPE->pstSyncCB )
        {
            return;
        }        
        if ( TransID ==  RectCPE->pstSyncCB->usSyncDAIBTransId)//cancel Trans
        {
            RectCPE->pstSyncCB->usSyncDAIBTransId = 0;
        }
	 else if ( TransID ==  RectCPE->pstSyncCB->usSyncDFTransId)//cancel Trans
        {
            RectCPE->pstSyncCB->usSyncDFTransId = 0;
        }        
        ProcsyncFail(RectCPE,ulEid);     //Recover Eid_Iplist from SyncCB while synchronize fail
        DelCPESyncCB(RectCPE);  //Delete SyncCtrlDB
        //统计包
        IncreaseCPEPerfMeasureByOne(ulEid,SYNC_IP_FAIL);
        
        proDownLoadToCpe(RectCPE);//check if other pro is needed
    }
}

/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProcDownloadACLTableResp

DESCRIPTION:
    Deal with Response of ACLDownload ,if find ACLCtrl,cancel transID,Send dataConfig Response Msg
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
void CTaskDm::ProcDownloadACLTableResp(CMessage &msg)
{
    UINT32 ulEid=msg.GetEID();
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcDownloadACLTableResp(0x%X)" ,ulEid);
    CPETable *RectCPE=GetCPERecordByEID(ulEid);
    if ( NULL == RectCPE )
        {//no exist EidNode
        LOG1( LOG_WARN, LOGNO(DM,ERR_DM_NO_CB), "->DldACLResp:EID[0x%.8X] no found.",ulEid );
        return;
        }
    CUTACLUpdtRsp ProDldACLRsp(msg);
    UINT16 ustrandid=ProDldACLRsp.GetXid();
    if ( !ProDldACLRsp.GetResult() )
        {//
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NO_CB), "->ProcDownloadACLTableResp:Result is Fail.");
        return;
        }
    if ( (STATUS_IDLE==RectCPE->ucPrtlCtrlState) || (ustrandid !=RectCPE->usPrtlCtrlTransId) )
        {
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NO_CB), "->ProcDownloadACLTableResp:ACL Status or ID is not match");
        return;
        }
    /*delete transectoin*/
    if ( 0!=ustrandid )
        CancelTransMsg(ustrandid);
    RectCPE->usPrtlCtrlTransId=0;
    RectCPE->ucPrtlCtrlState=STATUS_IDLE;
    proDownLoadToCpe(RectCPE);
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProcSyncResponse

DESCRIPTION:
    Deal with Response of Synchro DAIB-Addr
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
void CTaskDm::ProcSyncResponse(CMessage &msg,UINT16 MsgCode)
{
    UINT32 ulEid=msg.GetEID();
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcSyncResponse(0x%X)" ,ulEid);
    CDtSyncIpListRsp RspMsg(msg);
    UINT16 ustrandid=RspMsg.GetXid();
	
    if ( 0!=ustrandid )//cancel Trans
        CancelTransMsg(ustrandid);
    CPETable *RectCPE=GetCPERecordByEID(ulEid);
    if ( NULL == RectCPE )
    {//no exist EidNode
        LOG1( LOG_WARN, LOGNO(DM,ERR_DM_NO_CB), "->ProcSyncResponse:EID[0x%.8X] no found.",ulEid );
        return;
    }
    if ( NULL == RectCPE->pstSyncCB )
    {
        LOG( LOG_WARN,LOGNO(DM,ERR_DM_NO_CB) , "->ProcSyncResponse:SynchCB is NULL" );
        return;
    }    
    //分别比较两条消息是否tranid正确
    if((MsgCode == MSGID_UT_BTS_DAIBUPDATE_RESP))
    {
        if ( ustrandid != RectCPE->pstSyncCB->usSyncDAIBTransId )
        	{
       		 LOG2( LOG_DEBUG3,LOGNO(DM,ERR_DM_NO_CB) , "->ProcSyncResponse:SynchCB transID is not match.msg:DAIBUPDATE_RESP, utTranid:%x,btsTranid:%x" ,ustrandid, RectCPE->pstSyncCB->usSyncDAIBTransId);
        		return;
    		}
    		else
    		{
       		 RectCPE->pstSyncCB->usSyncDAIBTransId = 0;
    		}
    	}
    if((MsgCode == MSGID_UT_BTS_ADDRFLTR_TABLEUPDATE_RESP))
    {
       if ( ustrandid != RectCPE->pstSyncCB->usSyncDFTransId )
       	{
        		LOG2( LOG_DEBUG3,LOGNO(DM,ERR_DM_NO_CB) , "->ProcSyncResponse:SynchCB transID is not match.msg:TABLEUPDATE_RESP, utTranid:%x,btsTranid:%x" ,ustrandid, RectCPE->pstSyncCB->usSyncDFTransId);
        		return;
       	}

	    else
	    {
	        RectCPE->pstSyncCB->usSyncDFTransId = 0;
	    }
    	}
    if ( REST_FAIL== RspMsg.GetResult() )
    {
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NO_CB), "Rx from CPE, SYNC failed.");
        ProcsyncFail(RectCPE,ulEid);
        RectCPE->pstSyncCB->ucType = M_SYNC_IDLE;
        //return;
    }
    UINT8 ucType = RectCPE->pstSyncCB->ucType;
    if ( M_SYNC_IDLE == ucType )
    {
        //Free Sync CB
        DispatchSyncResponse(RectCPE->pstSyncCB,ulEid);//notify finished to Snoop 
        DelCPESyncCB(RectCPE);
    }
    proDownLoadToCpe(RectCPE);//check if other pro is needed 
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProcSynchIpListReq

DESCRIPTION:
   Synchronize DYNIPLIST or AddrTb according to Type
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
bool CTaskDm::ProcSyncIpListReq(CMessage &msg)
{
    CSyncIL msgSyncIplst(msg);
    UINT32 ulEid=msgSyncIplst.GetEidInPayload();
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcSyncIpListReq(0x%X)",ulEid );
    CPETable *RectCPE=GetCPERecordByEID(ulEid);
    if ( NULL == RectCPE )
        {
        LOG1( LOG_WARN, LOGNO(DM,ERR_DM_NO_CB), "->ProcSyncIpListReq:EID[0x%.8X] no found.",ulEid );
        return false;
        }

    SyncCB *pstSyncCB = RectCPE->pstSyncCB;
    if ( NULL == pstSyncCB )
        {
        pstSyncCB = (SyncCB*) new UINT8[sizeof(SyncCB)];
        if ( NULL == pstSyncCB )
            {
            LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->ProcSyncIpListReq:Create SyncCB fail." );
            return false;
            }
        memset(pstSyncCB,0,sizeof(SyncCB));
        RectCPE->pstSyncCB = pstSyncCB;
        }

    UINT8 Oper = msgSyncIplst.GetOp();
    UTILEntry tmpiplist;
    memcpy(&tmpiplist,&( msgSyncIplst.GetEntry()),sizeof(UTILEntry));
    switch ( Oper )
        {
        case M_SYNC_ADD:
            {
                LOG4( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->FromSnoop:Add Iplist[X-X-%.2X-%.2X-%.2X-%.2X]",
                    tmpiplist.aucMAC[2],tmpiplist.aucMAC[3],tmpiplist.aucMAC[4],tmpiplist.aucMAC[5]);
                if ( RectCPE->ucDynIpSize+RectCPE->ucFixSize >= RectCPE->ucMaxSize )
                    {
                    //the iplist is full
                    LOG( LOG_WARN, LOGNO(DM,ERR_DM_CB_USEDUP), "->ProcSyncIpLstReq:Add Iplist Fail cause DAIB FULL!" );
                    SendSyncRspToSnoop(ulEid,tmpiplist.ucIpType,tmpiplist.aucMAC,false);
                    return true;
                    }
                UINT8 ucIpLoc=0; 
                UINT8 ucsynctype=0;
                UINT8 ucsyncop=0;
                OPER result=SearchInIplist(RectCPE->pstIplstTbEnty,RectCPE->ucMaxSize,&tmpiplist,&ucIpLoc);
                if ( EQ == result )
                    {
                    SendSyncRspToSnoop(ulEid,tmpiplist.ucIpType,tmpiplist.aucMAC,true);
                    return true;
                    }
                if ( SIMIEQ == result )
                    {
                    ucsynctype = M_SYNC_IPLISTONLY;
                    ucsyncop   = M_SYNC_UPD_IPONLY;
                    }
                if ( (NEQ == result) && (0xff != ucIpLoc) )
                    {
                    ucsynctype = M_SYNC_BOTH;
                    ucsyncop   = M_SYNC_ADD_BOTH;
                    }

                RectCPE->pstSyncCB->ucType |= ucsynctype;

                if(msgSyncIplst.GetNeedResp())
                    {
                    bool result=AddToOpList( RectCPE, &tmpiplist, ucsyncop );
                    if(!result)
                        {/*apply Oplist Fail*,free memory of SynchrCtrlBlock*/
                        LOG( LOG_WARN, LOGNO(DM,ERR_DM_CB_USEDUP), "->ProcSyncIpLstReq:Add Iplist Fail cause SYNCOP FULL!" );
                        DelCPESyncCB(RectCPE);
                        SendSyncRspToSnoop(ulEid,tmpiplist.ucIpType,tmpiplist.aucMAC,false);
                        return false;
                        }
                    }

                memcpy(RectCPE->pstIplstTbEnty+ucIpLoc,&tmpiplist,sizeof(UTILEntry));
                if ( M_SYNC_ADD_BOTH == ucsyncop )
                    {
                    RectCPE->ucDynIpSize++;
                    UpdateAddressFilterTable( RectCPE->pstAddrFltrTbEntry[ucIpLoc], tmpiplist );
                    }
            }
            break;

        case M_SYNC_DELETE:
            {
                LOG4( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->FromSnoop:Del Iplist[X-X-%.2X-%.2X-%.2X-%.2X]",
                    tmpiplist.aucMAC[2],tmpiplist.aucMAC[3],tmpiplist.aucMAC[4],tmpiplist.aucMAC[5]);
                UINT8 ucIpLoc=0; 
                UINT8 ucsynctype=0;
                UINT8 ucsyncop=0;
                OPER result=SearchInIplist(RectCPE->pstIplstTbEnty,RectCPE->ucMaxSize,&tmpiplist,&ucIpLoc);
                if ( NEQ == result )
                    {
                    SendSyncRspToSnoop(ulEid,tmpiplist.ucIpType,tmpiplist.aucMAC,true);
                    return true;
                    }
                ucsynctype = M_SYNC_BOTH;
                ucsyncop   = M_SYNC_DEL_BOTH;
                RectCPE->pstSyncCB->ucType |= ucsynctype;
                if(msgSyncIplst.GetNeedResp())
                    {
                    bool result=AddToOpList( RectCPE, &tmpiplist, ucsyncop );
                    if(!result)
                        {/*apply Oplist Fail*,free memory of SynchrCtrlBlock*/
                        LOG( LOG_WARN, LOGNO(DM,ERR_DM_CB_USEDUP), "->ProcSyncIpLstReq:Delete Iplist Fail cause SYNCOP FULL!" );
                        DelCPESyncCB(RectCPE);
                        SendSyncRspToSnoop(ulEid,tmpiplist.ucIpType,tmpiplist.aucMAC,false);
                        return false;
                        }
                    }
                DelCPETbl_UTIpLst(RectCPE,ucIpLoc);
                DelCPETbl_AddressFltrTbl(RectCPE,ucIpLoc);
            }
            break;

        case M_SYNC_UPDATE:
            {
                LOG6( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->FromSnoop:Update Ip[X-X-%.2X-%.2X-%.2X-%.2X] ulRouterAreaId[%x], ulAnchorBts[0x%x]",
                    tmpiplist.aucMAC[2],tmpiplist.aucMAC[3],tmpiplist.aucMAC[4],tmpiplist.aucMAC[5],tmpiplist.ulRouterAreaId,tmpiplist.ulAnchorBts);

	
                UINT8 ucIpLoc=0; 
                UINT8 ucsynctype=0;
                UINT8 ucsyncop=0;
                OPER result=SearchInIplist(RectCPE->pstIplstTbEnty,RectCPE->ucMaxSize,&tmpiplist,&ucIpLoc);
                if ( (NEQ == result) || (EQ == result) )
                    {
                    SendSyncRspToSnoop(ulEid,tmpiplist.ucIpType,tmpiplist.aucMAC,true);
                    return true;
                    }
                if ( SIMIEQ == result )
                    {
                    ucsynctype = M_SYNC_IPLISTONLY;
                    ucsyncop   = M_SYNC_UPD_IPONLY;
                    }
                RectCPE->pstSyncCB->ucType |= ucsynctype;
                if(msgSyncIplst.GetNeedResp())
                    {
                    bool result=AddToOpList( RectCPE, &tmpiplist, ucsyncop );
                    if(!result)
                        {/*apply Oplist Fail*,free memory of SynchrCtrlBlock*/
                        LOG( LOG_WARN, LOGNO(DM,ERR_DM_CB_USEDUP), "->ProcSyncIpListReq:Update Iplist Fail cause SYNCOP FULL!" );
                        DelCPESyncCB(RectCPE);
                        SendSyncRspToSnoop(ulEid,tmpiplist.ucIpType,tmpiplist.aucMAC,false);
                        return false;
                        }
                    }
                memcpy(RectCPE->pstIplstTbEnty+ucIpLoc,&tmpiplist,sizeof(UTILEntry));
            }
            break;
        }

    proDownLoadToCpe(RectCPE);

    return true;
}


bool CTaskDm::AddToOpList(CPETable *pEidTable, const UTILEntry *pIpList, UINT8 &ucOp )
{
    LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_SYS_ERR), "->Add iplist To Synch OpList" );
    if ( ( NULL == pEidTable )
        || ( NULL == pIpList ) )
        {
        return false;
        }

    UINT8 ucIdx = 0;
    OPER result = SearchInIplist(pEidTable->pstSyncCB->astOpList, M_MAX_USER_PER_UT, pIpList, &ucIdx);
    if ( (NEQ == result) && (0xff == ucIdx) )
        {//OpIplist is Full
        return false;
        }
    pEidTable->pstSyncCB->astOpList[ucIdx].ucNeedResp = true;
    pEidTable->pstSyncCB->astOpList[ucIdx].ucOp = ucOp;
    memcpy( &(pEidTable->pstSyncCB->astOpList[ucIdx].stUTILEntry), pIpList, sizeof(UTILEntry) );

    return true;
}
/*============================================================
MEMBER FUNCTION:
    CTaskDm::SendVLIDToEB

DESCRIPTION:
   Notify VLanID to EB if changed
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/

void  CTaskDm::SendVLIDToEB(UINT32 ulEid,UINT16 usLanid)
{
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->Modify new VLanID(%d) To EB",usLanid );

    CFTCheckVLAN msgResp;
    if ( !msgResp.CreateMessage(*this) )
        return ;
    msgResp.SetEID(ulEid);
    msgResp.SetVlanID(usLanid);
    msgResp.SetDstTid(M_TID_EB);
    if ( !msgResp.Post() )
        {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->SendVLIDToEB:Post VLanID To EB Fail" );
        msgResp.DeleteMessage();
        return ;
        }
    
}

/*============================================================
MEMBER FUNCTION:
    CTaskDm::FreeDAIB

DESCRIPTION:
   Delete EidNode with the Eid
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/

void CTaskDm::ProcFreeDAIB(UINT32 ulEid)
{
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL) , "->ProcFreeDAIB(0x%X)",ulEid );
    UINT16 usTransID;
    CPETable *RectCPE=GetCPERecordByEID(ulEid);
    if ( NULL == RectCPE )
        return;

    if ( !(NULL == RectCPE->pstSyncCB) )
    {  //delete sizeof ProtocolCtrlBlk
        usTransID=RectCPE->pstSyncCB->usSyncDAIBTransId;
        if ( 0!=usTransID )
        {
           CancelTransMsg(usTransID);	    
        }
	 usTransID=RectCPE->pstSyncCB->usSyncDFTransId;
        if ( 0!=usTransID )
        {
            CancelTransMsg(usTransID);
        }
        delete RectCPE->pstSyncCB;
        RectCPE->pstSyncCB=NULL;
        }

    if ( !(NULL == RectCPE->pstIplstTbEnty) )
        {
        delete  RectCPE->pstIplstTbEnty;
        RectCPE->pstIplstTbEnty=NULL;
        }
    if ( !(NULL == RectCPE->pstAddrFltrTbEntry) )
        {
        delete RectCPE->pstAddrFltrTbEntry;
        RectCPE->pstAddrFltrTbEntry=NULL;
        }
    memset(RectCPE,0,sizeof(RectCPE));

    //清除CPE统计包
    ClearMeasure(ulEid);

    UINT16 usIdx = BPtreeFind(ulEid);
    BPtreeDel(ulEid);
    InsertFreeEIDList(usIdx);
    return;
}   

/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProcDmDataConfig

DESCRIPTION:
   Config Dm with workmode and MobilityEn,then response 
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/

void CTaskDm::ProcDmDataConfig(CMessage& msg)
{
    LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcDmDataConfig( )" );

    CDMConfigReq msgconfig(msg);

    m_bMobilityEn = true; //msgconfig.GetMobility()?true:false;
    m_ucWorkingMode = msgconfig.GetAccessCtrl();

    CDMDataConfigRsp msgRsp;
    if ( !msgRsp.CreateMessage(*this) )
        return;
    msgRsp.SetResult(ERR_SUCCESS);
    msgRsp.SetXid(msgconfig.GetXid());
    msgRsp.SetDstTid(msg.GetSrcTid());  

    if ( !msgRsp.Post() )
        {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->ProcDmDataConfig:Post Msg-Response failed." );
        msgRsp.DeleteMessage();
        }
    return;   

}    


/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProcCPEDataConfig

DESCRIPTION:
   Config CPE with Renew & Mobility,then response 
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
#define EQUAL_EID  (0xffffffff)
bool CTaskDm::ProcCPEDataConfig(CMessage &msg)
{
    CCPEDataConfigReq msgDataConfg(msg);  
    UINT32 ulEid=msg.GetEID();
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcCPEDataConfig(0x%X)",ulEid );
    CPETable *RectCPE=GetCPERecordByEID(ulEid);
    if ( NULL == RectCPE )
        {
        LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NO_CB), "->ProcCPEDataConfig:EID[0x%X] no found.",ulEid );
#ifdef __WIN32_SIM__
        WriteConfig( (UINT8*)msgDataConfg.GetDataPtr(), msgDataConfg.GetDataLength() );
#endif
        return false;
        }


    UINT8 ucsynctype=0;
    bool  bVlanIdModified = false;
    //exist CPE
    UINT16 usVLid=msgDataConfg.GetVlanId();
    if(RectCPE->stConfig.usVlanId != usVLid)
        {
        RectCPE->stConfig.usVlanId = usVLid;
        bVlanIdModified = true;
        SendVLIDToEB(ulEid,usVLid);
        }
    RectCPE->stConfig.ucMobilityEn=msgDataConfg.GetMobility();
    RectCPE->stConfig.ucRenew = msgDataConfg.GetDHCPRenew();
    UINT8 OldMaxSize = RectCPE->ucMaxSize;
    UINT8 OldFixSize = RectCPE->ucFixSize;
    RectCPE->ucFixSize = 0;
    UINT8 ucNewFixSize = msgDataConfg.GetFixIpNum();
    RectCPE->ucMaxSize = ( (msgDataConfg.GetMaxIpNum()>M_MAX_USER_PER_UT) )?M_MAX_USER_PER_UT:msgDataConfg.GetMaxIpNum();
    if(ucNewFixSize > RectCPE->ucMaxSize)
        {
        ucNewFixSize  =  RectCPE->ucMaxSize;
        }
    if ( (0==OldFixSize)  && (0==ucNewFixSize) )
        {
        return true;
        }
    UTILEntry *IplstEntry = RectCPE->pstIplstTbEnty;  
    UTILEntry UTILRam[M_MAX_USER_PER_UT];
    memset(&UTILRam,0,M_MAX_USER_PER_UT*sizeof(UTILEntry));
    memcpy(&UTILRam,IplstEntry,OldMaxSize*sizeof(UTILEntry));  
    memset(IplstEntry,0,OldMaxSize*sizeof(UTILEntry));

    //new memory to have the copy of old Iplists
    CpeFixIpInfo FixIplstEntry[M_MAX_USER_PER_UT];   
    UINT16 Lenth=(UINT16)(ucNewFixSize*sizeof(CpeFixIpInfo));
    msgDataConfg.GetCpeFixIpInfo((SINT8 *)(&FixIplstEntry), Lenth);

    CMac srcMac;
    CMac dstMac;
    UINT8 i=0;
    for ( i=0;i<ucNewFixSize;i++ )
        {
        srcMac.SetMac(FixIplstEntry[i].MAC);
        for ( UINT8 j=0;j<OldFixSize+RectCPE->ucDynIpSize;j++ )
            {
            dstMac.SetMac(UTILRam[j].aucMAC);

            if (srcMac == dstMac)
                {
                if( (FixIplstEntry[i].FixIP == UTILRam[j].Data.stIpLease.ulIp) 
                    &&(FixIplstEntry[i].AnchorBTSID == UTILRam[j].ulAnchorBts)
                    &&(FixIplstEntry[i].RouterAreaID == UTILRam[j].ulRouterAreaId)
                    &&(IPTYPE_FIXIP == UTILRam[j].ucIpType)
                    &&(false == bVlanIdModified))
                    {
                    FixIplstEntry[i].Eid=EQUAL_EID;
                    memset(&UTILRam[j],0,sizeof(UTILEntry));
                    break;
                    }

                DelIplistToSnoop(srcMac.GetMac(),M_UT_REMOVE);
                if((IPTYPE_FIXIP != UTILRam[j].ucIpType) && (RectCPE->ucDynIpSize>0))
                    {
                    RectCPE->ucDynIpSize--;
                    ucsynctype = M_SYNC_BOTH;
                    }
                memset(&UTILRam[j],0,sizeof(UTILEntry));
                break;
                }
            }
        
        if( GetFixIPList(&IplstEntry[RectCPE->ucFixSize],&FixIplstEntry[i]) )
            {
           // if(EQUAL_EID != FixIplstEntry[i].Eid )//wangwenhua mark 20090505
                {
                DispatchRoamFixIplst(ulEid,IplstEntry[RectCPE->ucFixSize]);//add new FixIplist
                ucsynctype |= M_SYNC_ADDRONLY;
                }
            RectCPE->ucFixSize++;
            }
        }
    
    for ( i=0;i<OldFixSize;i++ )
        {
        //Delete OldFixIplist
        dstMac.SetMac(UTILRam[i].aucMAC);
        if ( !dstMac.IsZero() )
            {
            DelIplistToSnoop(dstMac.GetMac(),M_UT_REMOVE);
            ucsynctype|= M_SYNC_ADDRONLY;
            }
        }
    UINT8 dynIplstSize=RectCPE->ucDynIpSize;
    if(dynIplstSize>0)
        {
        if ( dynIplstSize > (RectCPE->ucMaxSize-RectCPE->ucFixSize) )
            {
            LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->ProcCPEDataConfig:Modify DynIplst cause more size." );
            dynIplstSize=(UINT8)(RectCPE->ucMaxSize-RectCPE->ucFixSize);
            ucsynctype= M_SYNC_BOTH;
            }

       // memcpy(&IplstEntry[RectCPE->ucFixSize],&UTILRam[dynIplstHead],dynIplstSize*sizeof(UTILEntry)); 
        UINT8 j=0;
        for(i=OldFixSize;i<OldMaxSize;i++)
            {
            dstMac.SetMac(UTILRam[i].aucMAC);
            if(!dstMac.IsZero())
                {
                if(j<dynIplstSize)
                    {
                    memcpy(&IplstEntry[RectCPE->ucFixSize+j],&UTILRam[i],sizeof(UTILEntry)); 
                    j++;
                    }
                else
                    {
                    DelIplistToSnoop(dstMac.GetMac(),M_UT_REMOVE);
                    }
                }   
            }
        RectCPE->ucDynIpSize = dynIplstSize;
        }

    //modify AddressFltTb
    memset(RectCPE->pstAddrFltrTbEntry,0,M_MAX_USER_PER_UT*sizeof(AddressFltrTb));
    for ( UINT8 i=0; i<RectCPE->ucFixSize+RectCPE->ucDynIpSize; i++ )
        {
        UpdateAddressFilterTable( RectCPE->pstAddrFltrTbEntry[i], RectCPE->pstIplstTbEnty[i]);
        }
    if ( 0 != ucsynctype )
        {
        SyncCB *pSyncCB = RectCPE->pstSyncCB;
        if ( NULL == pSyncCB )
            {
            pSyncCB= (SyncCB*) new UINT8[sizeof(SyncCB)];
            if ( NULL == pSyncCB )
                {
                LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->ProcCPEDataConfig:Create SyncCB fail." );
                return false;
                }
            memset( pSyncCB, 0, sizeof(SyncCB) );
            RectCPE->pstSyncCB = pSyncCB;
            }
        //synchrIplist&AddrFltr for AddrFiltTb changed,
        RectCPE->pstSyncCB->ucType |= ucsynctype;
        proDownLoadToCpe(RectCPE);
        }

    return true;   
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::ACLConfigReq

DESCRIPTION:
   Config ACL ,then response 
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
 
==============================================================*/
void CTaskDm::ProSetACLConfigReq(CMessage &msg)
{
    LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->SetACLConfigReq( )" );
    SetACLConfigReq msgACLCnfg(msg);

    const ACLConfig *tmpPrtl = msgACLCnfg.GetACLConfigEntry();

    memset(m_ACLTb.PrtlFltTbEntry,0,M_MAX_PROTOCOL_REC*sizeof(PrtclFltrEntry));
    m_ACLTb.ucCount = msgACLCnfg.GetACLCount();
    for ( UINT8 i=0;i<m_ACLTb.ucCount;i++ )
        {
        m_ACLTb.PrtlFltTbEntry[i].ucFlag = 1;
        m_ACLTb.PrtlFltTbEntry[i].ucOccupied = 1;
        m_ACLTb.PrtlFltTbEntry[i].usProtocol = (UINT16)(tmpPrtl[i].Protocol);
        m_ACLTb.PrtlFltTbEntry[i].ulSrcAddr  = ntohl(tmpPrtl[i].SourceIp);
        m_ACLTb.PrtlFltTbEntry[i].ulDstAddr  = ntohl(tmpPrtl[i].DestIp);
        m_ACLTb.PrtlFltTbEntry[i].ulSrcMask  = ntohl(tmpPrtl[i].SourceWildCard);
        m_ACLTb.PrtlFltTbEntry[i].ulDstMask  = ntohl(tmpPrtl[i].DestWildCard);
        m_ACLTb.PrtlFltTbEntry[i].usSrcPort  = ntohs(tmpPrtl[i].SourcePort);
        m_ACLTb.PrtlFltTbEntry[i].usDstPort  = ntohs(tmpPrtl[i].DestPort);
        m_ACLTb.PrtlFltTbEntry[i].ucSrcOp    = tmpPrtl[i].SourceOpr;
        m_ACLTb.PrtlFltTbEntry[i].ucDstOp    = tmpPrtl[i].DestOpr;
        m_ACLTb.PrtlFltTbEntry[i].ucPermit   = tmpPrtl[i].Permit;
        }

    SetACLConfigRsp msgACLCnfgRsp;
    if ( !msgACLCnfgRsp.CreateMessage(*this) )
        return;
    msgACLCnfgRsp.SetResult(ERR_SUCCESS);
    msgACLCnfgRsp.SetXid(msgACLCnfg.GetXid());
    msgACLCnfgRsp.SetDstTid(msg.GetSrcTid());
    if ( !msgACLCnfgRsp.Post() )
        {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->SetACLConfigReq:Post Msg-Response failed." );
        msgACLCnfgRsp.DeleteMessage();
        }
    return;       
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProCPELocMove

DESCRIPTION:
    deal with CPE location changed,delete related CPETable,
    and notify delIp to Snoop Task

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTaskDm::ProCPELocationMove(UINT32 Eid,UINT8 oper)
{
    if(oper==M_UT_REMOVE)
        LOG1( LOG_DEBUG3,LOGNO(DM,ERR_DM_NORMAL), "->ProCPELocMove(0x%X),Oper[Delete]",Eid );
    else
        LOG1( LOG_DEBUG3,LOGNO(DM,ERR_DM_NORMAL), "->ProCPELocMove(0x%X),Oper[MoveAway]",Eid);
    CPETable *RectCPE=GetCPERecordByEID(Eid);
    if ( NULL == RectCPE )
    {
        LOG1( LOG_DEBUG, LOGNO(DM,ERR_DM_NO_CB), "->CPELocMove:EID[0x%.8X] no found.",Eid );
        return;
    }
    if(oper!=M_UT_REMOVE)//对于moveaway消息，如果与注册差2秒，则认为是延迟消息，丢弃
    {
        UINT32 tt = (UINT32)time( NULL );
	#if 0
	 if((tt - RectCPE->regTime)<=2)
	 {
	     LOG1( LOG_SEVERE,LOGNO(DM,ERR_DM_NORMAL), "->ProCPELocMove(0x%X),Oper[MoveAway], time <=2, discard msg",Eid);
          return;
	 }
	 
	#endif
    }
    if ( NULL == RectCPE->pstIplstTbEnty )
    {
        LOG( LOG_DEBUG, LOGNO(DM,ERR_DM_NO_CB), "->ProCPELocationMove:IPList is NULL");
        ProcFreeDAIB(Eid);   
        return;
    }
    UTILEntry *IplstEntry = RectCPE->pstIplstTbEnty;   
    CMac srcMac;
    for ( UINT8 count=0;count<RectCPE->ucFixSize+RectCPE->ucDynIpSize;count++ )
    {
        srcMac.SetMac(IplstEntry[count].aucMAC);
        if ( !srcMac.IsZero() )
        {
            DelIplistToSnoop(srcMac.GetMac(),oper);
        }
    }
    ProcFreeDAIB(Eid);   
    return;
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::DispatchRoamIplst

DESCRIPTION:
   Send RoamIplist Message to Snoop
ARGUMENTS:
 
RETURN VALUE:
   TrandID
SIDE EFFECTS:
  
==============================================================*/
void CTaskDm::DispatchRoamIplst(const UINT32 Eid,const UTILEntry &tmpiplist)
{
    LOG4( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->DispIpToSnoop:[X-X-%.2X-%.2X-%.2X-%.2X]",
                    tmpiplist.aucMAC[2],tmpiplist.aucMAC[3],tmpiplist.aucMAC[4],tmpiplist.aucMAC[5]);
    CRoam msgRoamIlReq;
    if ( !msgRoamIlReq.CreateMessage( *this ) )
        return;
    msgRoamIlReq.SetEidInPayload(Eid);
    msgRoamIlReq.SetEntry(tmpiplist);
    msgRoamIlReq.SetDstTid( M_TID_SNOOP );

    if ( ! msgRoamIlReq.Post() )
        {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->DispatchIpToSnoop:Post Msg failed." );
        msgRoamIlReq.DeleteMessage();
        }
    return;
}


bool CTaskDm::DispatchRoamFixIplst(const UINT32 Eid,const UTILEntry &tmpiplist)
{
    LOG4( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->DispFixIpToSnoop:[X-X-%.2X-%.2X-%.2X-%.2X]",
                    tmpiplist.aucMAC[2],tmpiplist.aucMAC[3],tmpiplist.aucMAC[4],tmpiplist.aucMAC[5]);
    CADDFixIP msgRoamIlReq;
    if ( !msgRoamIlReq.CreateMessage( *CTaskDm::GetInstance() ) )
        return false;
    msgRoamIlReq.SetEidInPayload(Eid);
    msgRoamIlReq.SetEntry(tmpiplist);
    msgRoamIlReq.SetDstTid( M_TID_SNOOP );

    if ( ! msgRoamIlReq.Post() )
        {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->Dispatch FixIpToSnoop:Post Msg failed." );
        msgRoamIlReq.DeleteMessage();
        return false;
        }
    return true;
}

/*============================================================
MEMBER FUNCTION:
    CTaskDm::DownloadACLTable

DESCRIPTION:
    Creat Data buf of ACL,set Xid,Lenth,send UpdateACLRequest msg
ARGUMENTS:
 
RETURN VALUE:
   TrandID
SIDE EFFECTS:
  
==============================================================*/
UINT32  CTaskDm::DownloadACLTable(CPETable *RectCPE)
{
    if(NULL == RectCPE)
        return 0;
    UINT32 Eid=RectCPE->ulEid;
    LOG( LOG_DEBUG3,  LOGNO(DM,ERR_DM_NORMAL), "->DownloadACLTable" );
    //Get ACL Value
    PrtclFltrEntry arACL[M_MAX_PROTOCOL_REC+ACL_DEFAULTNO];
    memset(&arACL,0,(M_MAX_PROTOCOL_REC+ACL_DEFAULTNO)*sizeof(PrtclFltrEntry));
    UINT32 ucNm=0;
    memcpy(&arACL[0],&PRTL_ARP,sizeof(PrtclFltrEntry));
    memcpy(&arACL[1],&PRTL_DHCP,sizeof(PrtclFltrEntry));
    memcpy(&arACL[2],&PRTL_PPP,sizeof(PrtclFltrEntry)); 
    ucNm=3;
    for ( UINT8 i=0;i<m_ACLTb.ucCount;i++ )
    {
        memcpy(&arACL[ucNm],&(m_ACLTb.PrtlFltTbEntry[i]),sizeof(PrtclFltrEntry));
        ucNm++;
    }
    memcpy(&arACL[ucNm],&PRTL_PERMIT_AUTHED_ALL,sizeof(PrtclFltrEntry)); 
    ucNm++;
    memcpy(&arACL[ucNm],&PRTL_DENY_OTHERS,sizeof(PrtclFltrEntry)); 
    ucNm++;
    memcpy(&arACL[ucNm],&PRTL_ForbidBroadcast,sizeof(PrtclFltrEntry)); 
    ucNm++;

    CDtACLUpdtReq msgUpdtACLReq;
    if ( !msgUpdtACLReq.CreateMessage(*this) )
    {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->DownloadACLTable:Create ACLReq Msg failed." );
        return 0;
    }
    msgUpdtACLReq.SetDstTid(M_TID_UTDM);
    msgUpdtACLReq.SetEID(Eid);
    msgUpdtACLReq.SetACLNum(ucNm);
    msgUpdtACLReq.SetACLVersion(M_DATA_SOFT_VERSION);
    UINT32 Lenth = sizeof(ACLDldReq)-(M_MAX_PROTOCOL_REC+ACL_DEFAULTNO-ucNm)*sizeof(PrtclFltrEntry);//sizeof(UINT8)+sizeof(PrtclFltrEntry)*ucNm;
    msgUpdtACLReq.SetPayLoadLenth(Lenth);
    msgUpdtACLReq.SetValue(arACL,ucNm);
    //构造配置失败消息       
    CDmTimerExpire FailNotify;
    if ( !FailNotify.CreateMessage(*this) )
    {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->DownloadACLTable:Create TimeExpire Msg failed." );
        msgUpdtACLReq.DeleteMessage();
        return 0;
    }
    FailNotify.SetInEid(RectCPE->ulEid);
    FailNotify.SetTimerType(RECEND_TIMER);
    FailNotify.SetDstTid(M_TID_DM);

    //创建配置数据transaction,发配置请求消息
    CTransaction* pCfgTransaction = CreateTransact(msgUpdtACLReq, FailNotify, RESEND_CNT3, M_DM_FTIMER_INTERVAL_2Sec);
    if ( NULL == pCfgTransaction )
    {
        LOG( LOG_WARN,  LOGNO(DM,ERR_DM_SYS_ERR), "->DownloadACLTable:ACL Transaction Fail");
        msgUpdtACLReq.DeleteMessage();
        FailNotify.DeleteMessage();
        return 0;
    }
    UINT16 TransID = pCfgTransaction->GetId();
    msgUpdtACLReq.SetACLXid(TransID);
    FailNotify.SetTransactionId(TransID);    
    if(!pCfgTransaction->BeginTransact())//如果失败释放处理jiaying20100811
    {
        pCfgTransaction->EndTransact();
	 delete pCfgTransaction;
	 return 0;
    }
    LOG2( LOG_DEBUG3,  LOGNO(DM,ERR_DM_NORMAL), "->DownloadACLTable Success Eid(0x%X),TransID(0x%X)",Eid,TransID );
    //性能包统计
    IncreaseCPEPerfMeasureByOne(Eid,SEND_SYNC_ACL_REQ);
    return TransID;
}

/*============================================================
MEMBER FUNCTION:
    CTaskDm::ProcsyncFail

DESCRIPTION:
    Synchornize DAib Fail,Recover DAib
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
void CTaskDm::ProcsyncFail(CPETable *RectCPE ,UINT32 ulEid)
{
    LOG1( LOG_DEBUG3,  LOGNO(DM,ERR_DM_NORMAL), "->ProcsyncFail(0x%X)",ulEid );
    if ( NULL == RectCPE )
        return;
    if ( NULL == RectCPE->pstSyncCB )
        return;
    OPLIST  *OpLst=RectCPE->pstSyncCB->astOpList;

    UINT8 count=0;
    CMac srcMac(OpLst[count].stUTILEntry.aucMAC);
    while ( !srcMac.IsZero() && (count<M_MAX_USER_PER_UT) )
        {
        if ( OpLst[count].ucNeedResp )
            {
            SendSyncRspToSnoop(ulEid,OpLst[count].stUTILEntry.ucIpType,srcMac.GetMac(),false);
            }
        UINT8 ucipLoc=0;
        OPER result;
        switch ( OpLst[count].ucOp )
            {
            case M_SYNC_ADD_BOTH:
                result=SearchInIplist(RectCPE->pstIplstTbEnty,RectCPE->ucMaxSize,&OpLst[count].stUTILEntry,&ucipLoc);
                if ( EQ == result )
                    {
                    DelCPETbl_UTIpLst(RectCPE,ucipLoc);
                    DelCPETbl_AddressFltrTbl(RectCPE,ucipLoc);
                    }
                break;
            case M_SYNC_DEL_BOTH:
                result=SearchInIplist(RectCPE->pstIplstTbEnty,RectCPE->ucMaxSize,&OpLst[count].stUTILEntry,&ucipLoc);
                if ( (NEQ==result) && (0xff!=ucipLoc) )
                    {
                    memcpy(&RectCPE->pstIplstTbEnty[ucipLoc],&OpLst[count].stUTILEntry,sizeof(UTILEntry));
                    RectCPE->ucDynIpSize++;
                    UpdateAddressFilterTable( RectCPE->pstAddrFltrTbEntry[ucipLoc], OpLst[count].stUTILEntry );
                    }
                break;
            case M_SYNC_UPD_IPONLY://recover DYNIPLIST
                result=SearchInIplist(RectCPE->pstIplstTbEnty,RectCPE->ucMaxSize,&OpLst[count].stUTILEntry,&ucipLoc);
                if ( EQ == result )
                    {
                    memcpy(&RectCPE->pstIplstTbEnty[ucipLoc],&OpLst[count].stUTILEntry,sizeof(UTILEntry));
                    }
                break;
            case M_SYNC_DEL_IPONLY:
                break;             
            }
        count++;
        srcMac.SetMac(OpLst[count].stUTILEntry.aucMAC);
        }
    return;
}


void CTaskDm::UpdateAddressFilterTable(AddressFltrTb &stAddrTable, UTILEntry &stUTILEntry)
{
    LOG4( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->UpdateAddsFltTb:[X-X-%.2X-%.2X-%.2X-%.2X]",
                    stUTILEntry.aucMAC[2],stUTILEntry.aucMAC[3],stUTILEntry.aucMAC[4],stUTILEntry.aucMAC[5]);
    memcpy( stAddrTable.ucSrcMAC, stUTILEntry.aucMAC, M_MAC_ADDRLEN );
    if ( IPTYPE_PPPoE == (stUTILEntry.ucIpType&0x0f) )
        {
        memcpy( stAddrTable.ucPeerMAC, stUTILEntry.Data.stSessionMac.aucServerMac, M_MAC_ADDRLEN );                   
        stAddrTable.usSessionId = stUTILEntry.Data.stSessionMac.usSessionId;
        stAddrTable.ulSrcIpAddr = 0;
        }
    else
        {
        stAddrTable.ulSrcIpAddr = stUTILEntry.Data.stIpLease.ulIp;
        stAddrTable.usSessionId = 0;
        memset( stAddrTable.ucPeerMAC, 0, M_MAC_ADDRLEN );
        }
}


//Del DYNIPLIST according to Mac
void CTaskDm::DelCPETbl_UTIpLst(CPETable *RectCPE,UINT8 uciploc)
{
    if ( NULL == RectCPE->pstIplstTbEnty )
        return;
    UTILEntry *Iplst=RectCPE->pstIplstTbEnty;
    if ( IPTYPE_FIXIP == Iplst[uciploc].ucIpType )
        {
        if ( RectCPE->ucFixSize>0 )
            RectCPE->ucFixSize--;
        }
    else
        {
        if ( RectCPE->ucDynIpSize>0 )
            RectCPE->ucDynIpSize--;
        }
    
    LOG4( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->DelIPlist:[X-X-%.2X-%.2X-%.2X-%.2X]",
                    Iplst[uciploc].aucMAC[2],Iplst[uciploc].aucMAC[3],Iplst[uciploc].aucMAC[4],Iplst[uciploc].aucMAC[5]);
    memset(&Iplst[uciploc],0,sizeof(UTILEntry));    
    uciploc++;
    CMac srcMac;
    for ( ;uciploc<M_MAX_USER_PER_UT;uciploc++ )
        {
        srcMac.SetMac( Iplst[uciploc].aucMAC );
        if ( srcMac.IsZero() )
            break;
        memcpy(&Iplst[uciploc-1],&Iplst[uciploc],sizeof(UTILEntry));

        }
    uciploc--;
    memset(&Iplst[uciploc],0,sizeof(UTILEntry));    
    return;
}

/*============================================================
MEMBER FUNCTION:
   ArrayEID::DelEidTb_AddressFltrTbl

DESCRIPTION:
   Delete AddrFltrTb according to srcMac,remove following list
ARGUMENTS:
  srcMac:source Mac

RETURN VALUE:

SIDE EFFECTS:
   none
==============================================================*/
void CTaskDm::DelCPETbl_AddressFltrTbl(CPETable *ProCPE,UINT8 ucaddrloc)
{
    CMac srcMac;
    if ( NULL == ProCPE->pstAddrFltrTbEntry )
        return;
    AddressFltrTb *AddrFltTb=ProCPE->pstAddrFltrTbEntry;
    
    LOG4( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->DelAddrFltrTb:[X-X-%.2X-%.2X-%.2X-%.2X]",
                    AddrFltTb[ucaddrloc].ucSrcMAC[2],AddrFltTb[ucaddrloc].ucSrcMAC[3],AddrFltTb[ucaddrloc].ucSrcMAC[4],AddrFltTb[ucaddrloc].ucSrcMAC[5]);

    memset(&AddrFltTb[ucaddrloc],0,sizeof(AddressFltrTb));  
    ucaddrloc++;
    for ( ;ucaddrloc<M_MAX_USER_PER_UT;ucaddrloc++ )
        {
        srcMac.SetMac( AddrFltTb[ucaddrloc].ucSrcMAC ); 
        if ( srcMac.IsZero() )
            break;
        memcpy(&AddrFltTb[ucaddrloc-1],&AddrFltTb[ucaddrloc],sizeof(AddressFltrTb));
        }
    //zero the last DYNIPLIST
    ucaddrloc--;
    memset(&AddrFltTb[ucaddrloc],0,sizeof(AddressFltrTb));  
    return;
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::DispatchSyncResponse

DESCRIPTION:
    Send Synchro success Message to Snoop
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
void CTaskDm::DispatchSyncResponse(SyncCB* SynchrCtrl,UINT32 ulEid)
{
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->DispatchSyncResponse(0x%X)", ulEid);
    if ( NULL == SynchrCtrl )
        {
        return;
        }
    UINT8 count=0;
    OPLIST  *Oplst=SynchrCtrl->astOpList;
    if ( NULL == Oplst )
        return;
    CMac srcMac(Oplst[count].stUTILEntry.aucMAC);
    while ( !srcMac.IsZero() && (count<M_MAX_USER_PER_UT) )
        {
        if ( !Oplst[count].ucNeedResp )
            {
            count++;
            srcMac.SetMac(Oplst[count].stUTILEntry.aucMAC);
            continue;
            }
        SendSyncRspToSnoop(ulEid,Oplst[count].stUTILEntry.ucIpType,srcMac.GetMac(),true);
        count++;
        srcMac.SetMac(Oplst[count].stUTILEntry.aucMAC);
        }
    return;
}
/*============================================================
MEMBER FUNCTION:
    CTaskDm::Synchronize

DESCRIPTION:
   Synchronize DYNIPLIST or AddrTb according to Type
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
bool CTaskDm::SynchronizeDAIBToCPE(CPETable *RectCPE)
{
    LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->SynchronizeDAIBToCPE" );
    DATA_assert(NULL != RectCPE);
    DATA_assert(NULL != RectCPE->pstSyncCB);
    UINT16 TransID;
    UINT32 DataLenth;
    UINT32 count=0;
//    UINT8 ucSyncType=RectCPE->pstSyncCB->ucType;
    //SynchroIPList
    if ( NULL == RectCPE->pstIplstTbEnty )
    {
        LOG( LOG_DEBUG, LOGNO(DM,ERR_DM_NO_CB), "->SynchronizeDAIBToCPE:IPLIST is NULL" );
        return false;
    }
    UTILEntry* tmpiplist=RectCPE->pstIplstTbEnty+RectCPE->ucFixSize;
    count=(UINT32)(RectCPE->ucDynIpSize);
    CDtSyncIplistReq msgSynIplist;  
    if ( !msgSynIplist.CreateMessage(*this) )
        return false;
    DataLenth=sizeof(DAIBReq)-(M_MAX_USER_PER_UT-count)*sizeof(IplistTLV);
    msgSynIplist.SetPayLoadLenth(DataLenth);
    msgSynIplist.SetTimeStamp((UINT32)time(NULL));
    msgSynIplist.SetVersion(M_DATA_SOFT_VERSION);
    msgSynIplist.SetIpCount(count);
    if ( 0!=count )
    {
        msgSynIplist.Set_DAIBTLV(tmpiplist,count);
    }
    msgSynIplist.SetEID(RectCPE->ulEid);
    msgSynIplist.SetDstTid(M_TID_UTDM);       

    //构造配置失败消息       
    CDmTimerExpire FailNotify;
    if ( !FailNotify.CreateMessage(*this) )
    {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->SynchronizeDAIBToCPE:Create TimeExpire Msg failed." );
        msgSynIplist.DeleteMessage();
        return false;
    }
    FailNotify.SetInEid(RectCPE->ulEid);
    FailNotify.SetTimerType(SYNC_TIMER);
    FailNotify.SetDstTid(M_TID_DM);

    //创建配置数据transaction,发配置请求消息
    CTransaction* pCfgTransaction = CreateTransact(msgSynIplist, FailNotify, RESEND_CNT3, M_DM_FTIMER_INTERVAL_2Sec);
    if ( NULL == pCfgTransaction )
    {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->SynchronizeDAIBToCPE:CreatTransact Fail" );
        msgSynIplist.DeleteMessage();
        FailNotify.DeleteMessage();
        return false;
    }
    TransID=pCfgTransaction->GetId();
    msgSynIplist.SetXid(TransID);
    FailNotify.SetTransactionId(TransID);
    if ( pCfgTransaction->BeginTransact() )
    {
        if ( 0!=RectCPE->pstSyncCB->usSyncDAIBTransId )
        {
            CancelTransMsg(RectCPE->pstSyncCB->usSyncDAIBTransId);
        }
        RectCPE->pstSyncCB->usSyncDAIBTransId = TransID;
    }
    else//如果失败释放处理jiaying20100811
    {
        pCfgTransaction->EndTransact();
        delete pCfgTransaction;
	 return false;
    }
    LOG2( LOG_DEBUG3,  LOGNO(DM,ERR_DM_NORMAL), "->SynchronizeDAIBToCPE Success Eid(0x%X),TransID(0x%X)",RectCPE->ulEid,TransID );
    IncreaseCPEPerfMeasureByOne(RectCPE->ulEid,SEND_SYNC_IP_REQ);
    return true;
}
bool CTaskDm::SynchronizeAddrToCPE(CPETable *RectCPE)
{
    LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->SynchronizeAddrToCPE" );
    DATA_assert(NULL != RectCPE);
    DATA_assert(NULL != RectCPE->pstSyncCB);
    UINT16 TransID;
    UINT32 DataLenth;
    UINT32 count=0;
    UINT8 ucSyncType=RectCPE->pstSyncCB->ucType;
    if ( (ucSyncType & M_SYNC_ADDRONLY) )
        {//SynchroAddr
        AddressFltrTb *pstAddrFlt=RectCPE->pstAddrFltrTbEntry;
        if ( NULL == pstAddrFlt )
            {
            LOG( LOG_DEBUG, LOGNO(DM,ERR_DM_NO_CB), "->SynchronizeAddrToCPE:AddrFltTb is NULL" );
            return false;
            }
        count=(UINT32)(RectCPE->ucFixSize+RectCPE->ucDynIpSize);

        CDtSyncAddrFBReq msgSyncAddr;   
        if ( !msgSyncAddr.CreateMessage(*this) )
            return false;
        DataLenth=sizeof(AddrFltTbReq)-sizeof(AddressFltrTb)*(M_MAX_USER_PER_UT-count);
        msgSyncAddr.SetPayLoadLenth(DataLenth);
        msgSyncAddr.SetEID(RectCPE->ulEid);
        msgSyncAddr.SetDstTid(M_TID_UTDM);   
        msgSyncAddr.SetAddrCount(count);
        msgSyncAddr.SetAddrVersion(M_DATA_SOFT_VERSION);
        if ( 0 != count )
            {
            msgSyncAddr.SetAddrFB(pstAddrFlt, count);
            }

        //构造配置失败消息       
        CDmTimerExpire FailNotify;
        if ( !FailNotify.CreateMessage(*this) )
            {
            LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->SynchronizeAddrToCPE:Create TimeExpire Msg failed." );
            msgSyncAddr.DeleteMessage();
            return false;
            }
        FailNotify.SetInEid(RectCPE->ulEid);
        FailNotify.SetTimerType(SYNC_TIMER);
        FailNotify.SetDstTid(M_TID_DM);        

        //创建配置数据transaction,发配置请求消息
        CTransaction* pCfgTransaction = CreateTransact(msgSyncAddr, FailNotify, RESEND_CNT3, M_DM_FTIMER_INTERVAL_2Sec);
        if ( NULL == pCfgTransaction )
        {
            LOG( LOG_WARN, LOGNO(DM,ERR_DM_NO_CB), "->SynchronizeAddrToCPE:CreatTransact Fail" );
            msgSyncAddr.DeleteMessage();
            FailNotify.DeleteMessage();
            return false;
        }
        TransID=pCfgTransaction->GetId();
        msgSyncAddr.SetAddrXid(TransID);
	    FailNotify.SetTransactionId(TransID);
        if ( pCfgTransaction->BeginTransact() )
        {
            if ( 0!=RectCPE->pstSyncCB->usSyncDFTransId )
            {
                CancelTransMsg(RectCPE->pstSyncCB->usSyncDFTransId);
            }
            RectCPE->pstSyncCB->usSyncDFTransId = TransID;
        }
        else//如果失败释放处理jiaying20100811
        {
            pCfgTransaction->EndTransact();
            delete pCfgTransaction;
	     return false;
        }
        LOG2( LOG_DEBUG3,  LOGNO(DM,ERR_DM_NORMAL), "->SynchronizeAddrToCPE Success Eid(0x%X),TransID(0x%X)",RectCPE->ulEid,TransID );

    }
    
    
    IncreaseCPEPerfMeasureByOne(RectCPE->ulEid,SEND_SYNC_IP_REQ);
    return true;
}

//Delete DYNIPLIST message to Snoop Task 
void CTaskDm::DelIplistToSnoop(const UINT8 *arMac,UINT8 DelType )
{
    LOG4( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->Send DelIpToSnoop:[X-X-%.2X-%.2X-%.2X-%.2X]",
                    arMac[2],arMac[3],arMac[4],arMac[5]);

    CDelILEntry msgReq;
    if ( !msgReq.CreateMessage(*this) )
        return;
    msgReq.SetMac(arMac);
    msgReq.SetOp(DelType);
    msgReq.SetDstTid(M_TID_SNOOP);
    if ( !msgReq.Post() )
        {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->DelIplistToSnoop:Post Msg failed." );
        msgReq.DeleteMessage();
        }
    return;   
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::InitFreeEIDList

DESCRIPTION:
    初始化空闲转发表表项链表m_listFreeFT
ARGUMENTS:
    NULL
RETURN VALUE:
    void
SIDE EFFECTS:
    none
==============================================================*/
void CTaskDm::InitFreeEIDList()
{
    m_listFreeEID.clear();
    for ( UINT16 usIdx = 0; usIdx < M_MAX_UT_PER_BTS; usIdx++ )
        {
        m_listFreeEID.push_back( usIdx );
        }
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::InsertFreeEIDList

DESCRIPTION:
    插入空闲转发表表项下标到链表m_listFreeEID尾部

ARGUMENTS:
    usIdx:表项下标

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
void CTaskDm::InsertFreeEIDList(UINT16 usIdx )
{
    if ( usIdx < M_MAX_UT_PER_BTS )
        {
        m_listFreeEID.push_back( usIdx );
        }
    else
        {
        DATA_assert( 0 );
        }
}

/*============================================================
MEMBER FUNCTION:
    CTaskDm::GetFreeEIDIdxFromList

DESCRIPTION:
    从空闲链表m_listFreeFT取空闲转发表表项下标;(从链表头部取)

ARGUMENTS:
    NULL

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTaskDm::GetFreeEIDIdxFromList(void)
{
    if ( true == m_listFreeEID.empty() )
        {
        g_dm_no_freelist++;
        LOG( LOG_WARN,LOGNO(DM,ERR_DM_CB_USEDUP) , "No Free Forwarding Entries!" );
        return M_DATA_INDEX_ERR;
        }

    UINT16 usIdx = *m_listFreeEID.begin();
    m_listFreeEID.pop_front();

    if ( M_MAX_UT_PER_BTS <= usIdx )
        {
        //下标错误
        g_dm_no_freelist++;
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_CB_INDEX_ERR), "Err! Free Forwarding Entry Index Err. " );
        return M_DATA_INDEX_ERR;
        }
	g_dm_no_freelist = 0;
    return usIdx;
}



/*============================================================
MEMBER FUNCTION:
    CTaskDm::BPtreeAdd

DESCRIPTION:
    转发表索引树插入节点；转发表索引树以Eid地址作键值，把转发表
    表项下标插入到索引树

ARGUMENTS:
    Eid:Eid
    usIdx:表项下标

RETURN VALUE:
    bool:插入成功/失败

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskDm::BPtreeAdd(UINT32 ulEid, UINT16 usIdx)
{

    pair<map<UINT32, UINT16>::iterator, bool> stPair;

    stPair = m_EIDptree.insert( ValType( ulEid, usIdx ) );
    return stPair.second;
}



/*============================================================
MEMBER FUNCTION:
    CTaskDm::BPtreeDel

DESCRIPTION:
    转发表索引树删除Eid对应的节点；

ARGUMENTS:
    Eid

RETURN VALUE:
    bool:删除成功/失败

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskDm::BPtreeDel(UINT32 ulEid)
{
    map<UINT32, UINT16>::iterator it;

    if ( ( it = m_EIDptree.find( ulEid ) ) != m_EIDptree.end() )
        {
        //find, and erase;
        m_EIDptree.erase( it );
        }
    //not find
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::BPtreeFind

DESCRIPTION:
    从BPtree搜索ulEid地址对应的转发表表项下标

ARGUMENTS:
    Mac:Mac地址

RETURN VALUE:
    usIdx:表项下标

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTaskDm::BPtreeFind(UINT32 ulEid)
{
    map<UINT32, UINT16>::iterator it = m_EIDptree.find( ulEid );

    if ( it != m_EIDptree.end() )
        {
        return it->second;
        }
    return M_DATA_INDEX_ERR;
}

/*============================================================
MEMBER FUNCTION:
    CTaskDm::GetCPERecordByEID

DESCRIPTION:
   return EidNode by Eid
ARGUMENTS:

RETURN VALUE:
   
SIDE EFFECTS:
 
==============================================================*/
CPETable* CTaskDm::GetCPERecordByEID(UINT32 EidNo)
{
    UINT16 usIdx = BPtreeFind(EidNo);
    if ( M_DATA_INDEX_ERR == usIdx )
        {
        return NULL;
        }
    return &( m_CPETb[ usIdx ] );
}
/*============================================================
MEMBER FUNCTION:
    CTaskDm::DelCPESyncCtrlDB

DESCRIPTION:
    Delete syncCtrlDb,free memeory
ARGUMENTS:
   srcMac

RETURN VALUE:
    UTILEntry

SIDE EFFECTS:
    none
==============================================================*/
void CTaskDm::DelCPESyncCB(CPETable* RectCPE)
{
    CMac srcMac;
    UINT16 usTransID;
    if (NULL != RectCPE->pstSyncCB)
        {  //delete sizeof ProtocolCtrlBlk
        usTransID=RectCPE->pstSyncCB->usSyncDAIBTransId;
        if ( 0!=usTransID )
        {
            CancelTransMsg(usTransID);  
        }
	 usTransID=RectCPE->pstSyncCB->usSyncDFTransId;
        if ( 0!=usTransID )
        {
            CancelTransMsg(usTransID);  
        }
        delete RectCPE->pstSyncCB;
        RectCPE->pstSyncCB=NULL;
        }
    return;
}
/*============================================================
MEMBER FUNCTION:
    CTaskDm::NewEidTableNode

DESCRIPTION:
    build EidNode with Eid,malloc size of DYNIPLIST and AddrTb
ARGUMENTS:
   ulEid:

RETURN VALUE:
    TRUE:;false

SIDE EFFECTS:
    none
==============================================================*/
CPETable* CTaskDm::NewEidTableNode(UINT32 ulEid)
{
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->NewEidTableNode(0x%X)",ulEid );
    UINT16 usIdx = GetFreeEIDIdxFromList();
    if ( M_DATA_INDEX_ERR == usIdx )
        {
        LOG1( LOG_DEBUG, LOGNO(DM,ERR_DM_CB_INDEX_ERR) , "->NewEidTableNode( %d ):New EidNode Fail cause no empty", ulEid );
        return NULL;//用户满
        }
    //新建转发表表项
    CPETable *pRstEid= &( m_CPETb[ usIdx ] );
    memset(pRstEid,0,sizeof(CPETable))  ; 
    if ( NULL == pRstEid->pstIplstTbEnty )
        {
        pRstEid->pstIplstTbEnty=(UTILEntry*)new UINT8[ M_MAX_USER_PER_UT*sizeof(UTILEntry) ];
        if ( NULL == pRstEid->pstIplstTbEnty )
            {
            LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR) , "->NewEidTableNode: Malloc Fail" );
            InsertFreeEIDList( usIdx );
            return NULL;
            }
        memset(pRstEid->pstIplstTbEnty,0,M_MAX_USER_PER_UT*sizeof(UTILEntry) );
        }
    if ( NULL == pRstEid->pstAddrFltrTbEntry )
        {
        pRstEid->pstAddrFltrTbEntry= (AddressFltrTb*)new UINT8[ M_MAX_USER_PER_UT * sizeof( AddressFltrTb )] ;
        if ( NULL == pRstEid->pstAddrFltrTbEntry )
            {
            LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR) , "->NewEidTableNode: Malloc Fail" );
            delete pRstEid->pstIplstTbEnty;
            pRstEid->pstIplstTbEnty = NULL;    
            InsertFreeEIDList( usIdx );
            return NULL;
            }
        memset(pRstEid->pstAddrFltrTbEntry,0,M_MAX_USER_PER_UT * sizeof( AddressFltrTb ));  
        }
    if ( NULL == pRstEid->pstSyncCB )
        {
        pRstEid->pstSyncCB= (SyncCB*) new UINT8[sizeof(SyncCB)];
        if ( NULL == pRstEid->pstSyncCB )
            {
            LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR) , "->NewEidTableNode: Malloc Fail" );
            delete pRstEid->pstIplstTbEnty;
            pRstEid->pstIplstTbEnty = NULL;
            delete pRstEid->pstAddrFltrTbEntry;
            pRstEid->pstAddrFltrTbEntry = NULL;
            InsertFreeEIDList( usIdx );
            return NULL;
            }
        memset(pRstEid->pstSyncCB,0,sizeof(SyncCB));
        }
    /*Build CPE dataconfig */
    CPEDataConfigReq dataCfg;
    memset( &dataCfg, 0, sizeof( dataCfg ) );
    dataCfg.Mobility=1;
    dataCfg.DHCPRenew=1;
    bool result =false;
    if((result=::GetCPEConfig(ulEid,(UINT8*)&dataCfg )) == false)
        {/*Get CPE dataconfig from OAM*/
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_SYS_ERR) , "->NewEidTableNode: GetDataConfig Fail" );
        delete pRstEid->pstIplstTbEnty;
        pRstEid->pstIplstTbEnty = NULL;
        delete pRstEid->pstAddrFltrTbEntry;
        pRstEid->pstAddrFltrTbEntry = NULL;
        delete pRstEid->pstSyncCB;
        pRstEid->pstSyncCB = NULL;
        InsertFreeEIDList( usIdx );
        return NULL;
        }//Edit by yanghuawei on 06-09-2006
    pRstEid->ulEid = ulEid;
    pRstEid->ucDynIpSize = 0;
    pRstEid->ucMaxSize = dataCfg.MaxIpNum;
    pRstEid->stConfig.usVlanId  = dataCfg.usVlanId;      //by xiaoweifang; to support vlan.
    SendVLIDToEB(ulEid,dataCfg.usVlanId);//notify VLanId to EB

    pRstEid->stConfig.ucMobilityEn= dataCfg.Mobility;
    pRstEid->stConfig.ucRenew= dataCfg.DHCPRenew;
    BPtreeAdd( ulEid,usIdx );
    //初始化CPE统计包
    SetCPEPerfMeasure(ulEid,(UINT32)(time(NULL)) );
    UINT8 ucFixSize = ( dataCfg.FixIpNum > M_MAX_USER_PER_UT )?0:dataCfg.FixIpNum;
    if ( 0 == ucFixSize)
        {
        pRstEid->ucFixSize = 0;
        return pRstEid;
        }
    /*build the valid FixIp */
    AddressFltrTb *pAddrFltr =  pRstEid->pstAddrFltrTbEntry;    
    for(UINT8 i=0;i<ucFixSize;i++)
        {
        UTILEntry *pIplistEntry = &pRstEid->pstIplstTbEnty[pRstEid->ucFixSize];
        if(GetFixIPList(pIplistEntry,&dataCfg.stCpeFixIpInfo[i]))
            {
            UpdateAddressFilterTable(pAddrFltr[pRstEid->ucFixSize],*pIplistEntry);
            DispatchRoamFixIplst(ulEid, *pIplistEntry);
            pRstEid->ucFixSize++;
            }
        }
    

    pRstEid->pstSyncCB->ucType |= M_SYNC_ADDRONLY;
    return pRstEid; 
}


bool CTaskDm::CancelTransMsg(UINT16 TransID)
{
    CTransaction * pTransaction = FindTransact(TransID);
    if ( !pTransaction )
        {
        return false;
        }

    pTransaction->EndTransact();
    delete pTransaction;
    return true;
}

bool CTaskDm::SendDataServsRSP(const UINT32 ulEid, const UINT16 vlanID, bool resulst)
{
    int i;
    bool isRelayCpe = false;
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->Send data service response(eid:0x%X)." ,ulEid);
    CDtDataServsRsp msgDataServsRsp;
    if ( !msgDataServsRsp.CreateMessage(*this) )
        return false;
    msgDataServsRsp.SetResult(resulst);
   #ifdef M_TGT_WANIF
   if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
       for( i = 0; i<NvRamDataAddr->Relay_num ;i++)
       {
           if(RelayWanifCpeEid[i] ==ulEid )
           {
               isRelayCpe = true;
               break;
           }
       
       }
    if((WanIfCpeEid== ulEid)||(BakWanIfCpeEid == ulEid)||(isRelayCpe == true))
    	{
    	      msgDataServsRsp.SetWorkMode(WM_LEARNED_BRIDGING);
    	}
	else
	{
	     msgDataServsRsp.SetWorkMode(m_ucWorkingMode);
	}
   #else

           msgDataServsRsp.SetWorkMode(m_ucWorkingMode);

   #endif
    msgDataServsRsp.SetVlanID(vlanID);//not group ID.
    msgDataServsRsp.SetEID(ulEid);
    msgDataServsRsp.SetDstTid(M_TID_UTDM);

    if ( !msgDataServsRsp.Post() )
        {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->SendDataServsRSP:Post DataSevice Reponse failed." );
        msgDataServsRsp.DeleteMessage();
        }
    IncreaseCPEPerfMeasureByOne(ulEid,DATA_SERVICE_RSP);

    return true;
}


bool CTaskDm::SendSyncRspToSnoop(UINT32 ulEid,UINT8 ipType,const UINT8* mac,bool rest)
{
    LOG1( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "->Send sync response(%s) to SNOOP.", (int)(rest?"success":"fail") );
    CSyncILResp msgResp;
    if ( !msgResp.CreateMessage(*this) )
        return false;
    msgResp.SetEidInPayload(ulEid);
    msgResp.SetIpType(ipType);
    msgResp.SetMac(mac);
    msgResp.SetResult(rest);
    msgResp.SetDstTid(M_TID_SNOOP);
    if ( !msgResp.Post() )
        {
        LOG( LOG_WARN, LOGNO(DM,ERR_DM_SYS_ERR), "->SendSyncRspToSnoop:Post Synchr Response Fail" );
        msgResp.DeleteMessage();
        return false;
        }
    return true;

}


/*============================================================
MEMBER FUNCTION:
    CTaskDm::SearchInIplist

DESCRIPTION:
   search aucIplst in srcIplist,and return the result
ARGUMENTS:
   UTILEntry:source of Iplist;ucIplen:Ipcount;UTILEntry:DestIplist;ucLocation:the iplist location of searching
RETURN VALUE:
   
SIDE EFFECTS:
 
==============================================================*/
OPER CTaskDm::SearchInIplist(const UTILEntry* srcIplist,UINT8 ucIplen,const UTILEntry* aucIplst,UINT8* ucLocation)
{
    CMac srcMc,dstMc;
    OPER result=NEQ;
    UINT8 i = 0;
    for ( i =0;i<ucIplen;i++ )
        {
        result=NEQ;
        srcMc.SetMac(srcIplist[i].aucMAC);
        dstMc.SetMac(aucIplst->aucMAC);
        if ( srcMc.IsZero() )
            {
            result=NEQ;
            break;
            }
        if ( !(srcMc == dstMc) )
            {
            continue;
            }
        if ( aucIplst->Data.stIpLease.ulIp != srcIplist[i].Data.stIpLease.ulIp )
            {
            result= SIMIEQ;
            break;
            }
        if ( aucIplst->Data.stIpLease.ulLease != srcIplist[i].Data.stIpLease.ulLease )
            {
            result= SIMIEQ;
            break;
            }
        if ( aucIplst->ulAnchorBts !=srcIplist[i].ulAnchorBts )
            {
            result= SIMIEQ;
            break;
            }
        if ( aucIplst->ulRouterAreaId != srcIplist[i].ulRouterAreaId )
            {
            result= SIMIEQ;
            break;
            }
        result = EQ;
        break;
        }
    if ( i == ucIplen )
        {//missing
        *ucLocation = 0xff; 
        return NEQ;
        }
    *ucLocation=i;
    return result;
}
OPER CTaskDm::SearchInIplist(const OPLIST* srcoplist,UINT8 ucIplen,const UTILEntry* aucIplst,UINT8* ucLocation)
{
    CMac srcMc,dstMc;
    OPER result=NEQ;
    UINT8 i = 0;
    for ( i=0;i<ucIplen;i++ )
        {
        result=NEQ;
        dstMc.SetMac(aucIplst->aucMAC);
        srcMc.SetMac(srcoplist[i].stUTILEntry.aucMAC);
        if ( srcMc.IsZero() )
            {
            result = NEQ;
            break;
            }
        if ( !(srcMc == dstMc) )
            {
            continue;
            }
        if ( aucIplst->Data.stIpLease.ulIp != srcoplist[i].stUTILEntry.Data.stIpLease.ulIp )
            {
            result= SIMIEQ;
            break;
            }
        if ( aucIplst->Data.stIpLease.ulLease != srcoplist[i].stUTILEntry.Data.stIpLease.ulLease )
            {
            result= SIMIEQ;
            break;
            }
        if ( aucIplst->ulRouterAreaId != srcoplist[i].stUTILEntry.ulRouterAreaId )
            {
            result= SIMIEQ;
            break;
            }
        result = EQ;
        break;
        }
    if ( i == ucIplen )
        {//missing
        *ucLocation = 0xff; 
        return NEQ;
        }
    *ucLocation=i;
    return result;
}
/*
As the process of downloading to CPE,switch control which should do following,
*/
void CTaskDm::proDownLoadToCpe(CPETable* RectCPE)
{
    if(NULL == RectCPE)
        return;
    
    if(STATUS_PEND == RectCPE->ucNeedDataRsp)
        {
        SendDataServsRSP(RectCPE->ulEid, CTBridge::GetInstance()->getVlanID(RectCPE->stConfig.usVlanId));
        RectCPE->ucNeedDataRsp=STATUS_IDLE;
        //return;
        }

    if ((WM_LEARNED_BRIDGING == m_ucWorkingMode) && (STATUS_PEND == RectCPE->ucPrtlCtrlState)) 
        {//学习模式下，不同步ACL
            RectCPE->ucPrtlCtrlState = STATUS_IDLE;
        }
    if (STATUS_PEND == RectCPE->ucPrtlCtrlState )
        {
        if(0 !=(RectCPE->usPrtlCtrlTransId = DownloadACLTable(RectCPE)) );
            {
            RectCPE->ucPrtlCtrlState = STATUS_ACL;
            }
        return;
        }

    if(NULL != RectCPE->pstSyncCB)
        {
        if ((WM_LEARNED_BRIDGING == m_ucWorkingMode) && (RectCPE->pstSyncCB->ucType & M_SYNC_ADDRONLY)) 
            {//学习模式下，不同步地址表
            RectCPE->pstSyncCB->ucType &= ~M_SYNC_ADDRONLY;
            }
        if (RectCPE->pstSyncCB->ucType & M_SYNC_ADDRONLY )
            {
            SynchronizeAddrToCPE(RectCPE);
            RectCPE->pstSyncCB->ucType &= ~M_SYNC_ADDRONLY;
            return;
            }
        
        if(RectCPE->pstSyncCB->ucType & M_SYNC_IPLISTONLY)
            {
            SynchronizeDAIBToCPE(RectCPE);
            //清标志
            RectCPE->pstSyncCB->ucType &= ~M_SYNC_IPLISTONLY;
            return;
            }
       }
}
bool CTaskDm::GetFixIPList(UTILEntry* pFixIplst, CpeFixIpInfo* pstConfig)
{
    if((NULL == pFixIplst) ||(NULL == pstConfig))
        return false;
    UINT32 rectRouterID = ::GetRouterAreaId();
    UINT32 ulRouterAreaId;
    UINT32 ulAnchorBtsID;
    ulRouterAreaId = ntohl(pstConfig->RouterAreaID);
    ulAnchorBtsID = ntohl(pstConfig->AnchorBTSID);
    if( !IsBTSsupportMobility( rectRouterID, ulRouterAreaId, ulAnchorBtsID ) )
        {
        LOG( LOG_DEBUG3, LOGNO(DM,ERR_DM_NORMAL), "BTS mobility disable." );
        return false;
        }
    memcpy( pFixIplst->aucMAC, pstConfig->MAC, M_MAC_ADDRLEN );
    pFixIplst->ucIpType  = IPTYPE_FIXIP;
    pFixIplst->Data.stIpLease.ulIp = ntohl(pstConfig->FixIP);
    pFixIplst->Data.stIpLease.ulLease   = 0;
    pFixIplst->ulRouterAreaId = ulRouterAreaId;
    pFixIplst->ulAnchorBts=ulAnchorBtsID;
    return true;
}
void CTaskDm::ClearMeasure(UINT32 eid)
{
    if(0 == eid)
        {
        ClearAllMeasure();
        return;
        }
    UINT16 idx=BPtreeFind(eid);
    if ( M_DATA_INDEX_ERR == idx )
        return ;
    for ( UINT8 type=0;type<MAX_PERFMC;type++ )
        CPEPerfMeasureTb[M_MAX_UT_PER_BTS].arPfmMeasure[type]-=CPEPerfMeasureTb[idx].arPfmMeasure[type];
    if ( CPEPerfMeasureTb[M_MAX_UT_PER_BTS].ulEid>0 )
        CPEPerfMeasureTb[M_MAX_UT_PER_BTS].ulEid--;
    memset( &CPEPerfMeasureTb[idx], 0, sizeof( CPEPerfm ) );
}

void CTaskDm::ClearAllMeasure()
{
    map<UINT32, UINT16>::iterator it = m_EIDptree.begin();
    UINT8 ucFlowCtrl=0;
    while ( m_EIDptree.end() != it )
        {

        if ( M_DATA_INDEX_ERR == it->second )
            {
            ++it;
            continue;
            }
        for ( UINT8 type=0;type<MAX_PERFMC;type++ )
            CPEPerfMeasureTb[M_MAX_UT_PER_BTS].arPfmMeasure[type]-=CPEPerfMeasureTb[it->second].arPfmMeasure[type];
        if ( CPEPerfMeasureTb[M_MAX_UT_PER_BTS].ulEid>0 )
            CPEPerfMeasureTb[M_MAX_UT_PER_BTS].ulEid--;
        memset( &CPEPerfMeasureTb[it->second], 0, sizeof( CPEPerfm ) );
        ++it;
        if ( M_DM_FLOWCTRL_CNT == ++ucFlowCtrl )
            {
#ifdef __WIN32_SIM__
            //Win32:
            ::Sleep( 100 );//释放CPU
#else
            //VxWorks:
            ::taskDelay( 1 );
#endif
            ucFlowCtrl = 0;
            }
        }
}

/*===========================================
 *EB查询Mac对应的用户是否是FixIp用户
 *如果是，则往SNOOP发消息，重建FixIp用户
 */
bool CTaskDm::BuildFixIpContext(const CMac &Mac, const UINT32 ulEid)
{
    CPETable *pEidTable = GetCPERecordByEID( ulEid );
    if ( ( NULL == pEidTable )
         || ( NULL == pEidTable->pstIplstTbEnty ) )
        {
        return false;
        }

    CMac srcMac;

    for ( UINT8 idx = 0; idx < pEidTable->ucFixSize; ++idx )
        {
        UTILEntry *pstILEnty = pEidTable->pstIplstTbEnty + idx ;
        srcMac.SetMac( pstILEnty->aucMAC );
        if ( true == srcMac.IsZero() )
            {
            return false;
            }

        if ( srcMac == Mac )
            {
            //查询到Mac
            if ( IPTYPE_FIXIP == ( pstILEnty->ucIpType & 0x0f ) )
                {
                //重建FixIp用户信息
                return DispatchRoamFixIplst(ulEid, *pstILEnty );
                }

            return false;
            }
        }

    return false;
}


void CTaskDm::printACL(const PrtclFltrEntry &tmpACL,UINT8 num)
{
    struct in_addr IpAddr1,IpAddr2;
#ifdef __WIN32_SIM__
    IpAddr1.S_un.S_addr = htonl(tmpACL.ulSrcAddr );
    IpAddr2.S_un.S_addr = htonl(tmpACL.ulDstAddr );
#else
    IpAddr1.s_addr = htonl(tmpACL.ulSrcAddr );
    IpAddr2.s_addr = htonl(tmpACL.ulDstAddr );
#endif
    SINT8 strIpAddr1[ INET_ADDR_LEN ] = {0};
    SINT8 strIpAddr2[ INET_ADDR_LEN ] = {0};
    inet_ntoa_b( IpAddr1, strIpAddr1 );
    inet_ntoa_b( IpAddr2, strIpAddr2 );
    printf( "\r\n%-3d%-5d%-5d%-5X%-16s%-9X%-3d%-5d%-16s%-9X%-3d%-5d%-2d",
            num,
            tmpACL.ucFlag,
            tmpACL.ucIsBroadcast,
            tmpACL.usProtocol,
            strIpAddr1,tmpACL.ulSrcMask,tmpACL.ucSrcOp,tmpACL.usSrcPort,
            strIpAddr2,tmpACL.ulDstMask,tmpACL.ucDstOp,tmpACL.usDstPort,
            tmpACL.ucPermit);
}
/*============================================================
MEMBER FUNCTION:
    CTBridge::showDMbyEid

DESCRIPTION:
    打印DmTask中CPE表，ACL表by eid

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTaskDm::showDMbyEid(UINT32 ulEid)
{
	showEIDTable(ulEid);
}
/*============================================================
MEMBER FUNCTION:
    CTBridge::showStatus

DESCRIPTION:
    打印DmTask中CPE表，ACL表

ARGUMENTS:
    NULL

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTaskDm::showStatus()
{
#ifdef __WIN32_SIM__
    //等showStatus信号量
    WAIT();
#endif
    printf( "\r\n*******************************" );
    printf( "\r\n*DM Task Attributes *" );
    printf( "\r\n*******************************" );
    printf( "\r\n%-22s: %d", "Task   stack   size",  M_TASK_DM_STACKSIZE );
    printf( "\r\n%-22s: %d", "Task   Max  messages", M_TASK_DM_MAXMSG );
    printf( "\r\n" );
    printf( "\r\n%-22s: %s", "Access Control", \
            ( WM_LEARNED_BRIDGING == m_ucWorkingMode )?"Learned Bridging Mode":"Network Layer Aware Mode" );
    printf( "\r\n%-22s: %-10s","Mobile enable",m_bMobilityEn?("True"):("False") );  
    if ( true == m_EIDptree.empty() )
        {
        printf( "\r\n%-22s: %s", "CPE Table", "0     Records" );
        }
    else
        {
        printf( "\r\n%-22s: %-5d Records", "CPE Table", m_EIDptree.size() );
        }
    //Free CPE Entries
    printf( "\r\n%-22s: %-5d Records", "Free CPE Records", m_listFreeEID.size() );
    //BPtree
    printf( "\r\n" );
#ifdef __WIN32_SIM__
    //释放showStatus信号量
    RELEASE();
#endif


}
void CTaskDm::showEIDTable(UINT32 eid)
{
#ifdef __WIN32_SIM__
    //等showStatus信号量
    WAIT();
#endif
    UINT8 count;
    CMac srcMac;
    map<UINT32, UINT16>::iterator it = m_EIDptree.begin();
    UINT8 ucFlowCtrl=0;
    while ( m_EIDptree.end() != it )
        {

        if ( M_DATA_INDEX_ERR == it->second )
            {
            ++it;
            continue;
            }
        CPETable *RectCPE=&m_CPETb[it->second];
        if ( NULL == RectCPE )
            {
            printf( "\r\n no related CPE Record of Eid" );
            }
        else
            {
            if ( (eid!=0) && (eid != RectCPE->ulEid) )
                {
                ++it;
                continue;
                }
            printf( "\r\n\r\n%-22s: 0x%.8X", "Eid number",RectCPE->ulEid );
            printf( "\r\n---------------------------------");
            printf( "\r\n%-22s: %-d", "Group ID",RectCPE->stConfig.usVlanId);
            printf( "\r\n%-22s: %-s", "Mobile enable",RectCPE->stConfig.ucMobilityEn?"Yes":"No");
            printf( "\r\n%-22s: %-s", "DHCP renew enable",RectCPE->stConfig.ucRenew?"Yes":"No");
            printf( "\r\n%-22s: %-d", "Max IP number",RectCPE->ucMaxSize);
            printf( "\r\n%-22s: %-d", "Fix IP number",RectCPE->ucFixSize);
            printf( "\r\n%-22s: %-d", "Dynamic IP number",RectCPE->ucDynIpSize);
            printf( "\r\n" );
            printf( "\r\n%-22s: %-d\r\n%-22s: %-d ",
                    "ProtocolCB state",RectCPE->ucPrtlCtrlState,
                    "ProtocolCB transId",RectCPE->usPrtlCtrlTransId);   

            printf( "\r\n%-22s:","Sync CB state");                                
            if ( NULL == RectCPE->pstSyncCB )
                printf( "-NULL-" );
            else
                {
                SyncCB *Sync=RectCPE->pstSyncCB;
                printf( "\r\n%10s: %-2X", "type",Sync->ucType);
                printf( "\r\n%10s: %d","DAIBTransId, ",Sync->usSyncDAIBTransId);
		        printf( "\r\n%10s: %d","AddrTransId, ",Sync->usSyncDFTransId);
                }
            //printf measure of performance of each CPE
            printf( "\r\n***************************************" );
            printf( "\r\n*Performance Measurement Status       *" );
            printf( "\r\n***************************************" );
            for ( UINT8 type = DATA_SERVICE_REQ; type < MAX_PERFMC; ++type )
                {
                printf( "\r\n%-22s: %d", strCPEPerformance[ type ], CPEPerfMeasureTb[it->second].arPfmMeasure[ type ] );
                }
            printf( "\r\n" );

            printf( "\r\n%s:","Address filter table");                       
            if ( NULL == RectCPE->pstAddrFltrTbEntry )
                printf( "\r\n-NULL-" );
            else
                {
                AddressFltrTb *AddrTb=RectCPE->pstAddrFltrTbEntry;
                count=0;
                srcMac.SetMac(AddrTb[count].ucSrcMAC);
                printf( "\r\n%-4s%-20s%-10s%-20s%-16s",
                        "No","Source-Mac","SessionId","Server-Mac","Source-IP");   
                printf( "\r\n----------------------------------------------------------------------" );   
                while ( !srcMac.IsZero()&& (count < M_MAX_USER_PER_UT) )
                    {
                    struct in_addr IpAddr;
#ifdef __WIN32_SIM__
                    IpAddr.S_un.S_addr = htonl( AddrTb[count].ulSrcIpAddr );
#else
                    IpAddr.s_addr = htonl( AddrTb[count].ulSrcIpAddr);
#endif
                    SINT8 strIpAddr[ INET_ADDR_LEN ] = {0};
                    inet_ntoa_b( IpAddr, strIpAddr );
                    printf( "\r\n%-4d%.2X-%.2X-%.2X-%.2X-%.2X-%.2X   %-8d  %.2X-%.2X-%.2X-%.2X-%.2X-%.2X   %-17s",
                            count,                    
                            AddrTb[count].ucSrcMAC[0],AddrTb[count].ucSrcMAC[1],AddrTb[count].ucSrcMAC[2],
                            AddrTb[count].ucSrcMAC[3],AddrTb[count].ucSrcMAC[4],AddrTb[count].ucSrcMAC[5],
                            AddrTb[count].usSessionId,
                            AddrTb[count].ucPeerMAC[0],AddrTb[count].ucPeerMAC[1],AddrTb[count].ucPeerMAC[2],
                            AddrTb[count].ucPeerMAC[3],AddrTb[count].ucPeerMAC[4],AddrTb[count].ucPeerMAC[5],                    
                            strIpAddr);
                    count++;
                    srcMac.SetMac(AddrTb[count].ucSrcMAC);   
                    }

                }
            printf("\r\n"); 
            printf( "\r\n%-22s:","DAIB state");             
            if ( NULL == RectCPE->pstIplstTbEnty )
                printf( "\r\n-NULL-" );
            else
                {
                UTILEntry *UtIplst =RectCPE->pstIplstTbEnty;
                count=0;
                srcMac.SetMac(UtIplst[count].aucMAC);
                printf( "\r\n%-4s %-17s %-6s%-16s %-10s %-16s",
                        "No","MAC-address","type","IP-address","RAID(h)","Anchor-BtsID" );
                printf( "\r\n----------------------------------------------------------------------" );   
                while ( !srcMac.IsZero()&& (count < RectCPE->ucMaxSize) )
                    {
                    struct in_addr IpAddr;
                    //struct in_addr IpAddr1;
#ifdef __WIN32_SIM__
                    //IpAddr1.S_un.S_addr = htonl(UtIplst[count].ulAnchorBts);     
                    IpAddr.S_un.S_addr = 0;
                    if ( IPTYPE_PPPoE != UtIplst[count].ucIpType )
                        IpAddr.S_un.S_addr = htonl( UtIplst[count].Data.stIpLease.ulIp );
#else
                    //IpAddr1.s_addr = htonl(UtIplst[count].ulAnchorBts);     
                    IpAddr.s_addr = 0;
                    if ( IPTYPE_PPPoE != UtIplst[count].ucIpType )
                        IpAddr.s_addr = htonl( UtIplst[count].Data.stIpLease.ulIp);
#endif
                    SINT8 strIpAddr[ INET_ADDR_LEN ] = {0};
                    //SINT8 strIpAddr1[ INET_ADDR_LEN ] = {0};
                    inet_ntoa_b( IpAddr, strIpAddr );
                    //inet_ntoa_b( IpAddr1, strIpAddr1 );
                    printf( "\r\n%-4d %.2X-%.2X-%.2X-%.2X-%.2X-%.2X %-6s%-16s 0x%.8X %8x",
                            count,                   
                            UtIplst[count].aucMAC[0],UtIplst[count].aucMAC[1],UtIplst[count].aucMAC[2],
                            UtIplst[count].aucMAC[3],UtIplst[count].aucMAC[4],UtIplst[count].aucMAC[5],
                            ( IPTYPE_DHCP == UtIplst[count].ucIpType )?"DHCP":( ( IPTYPE_PPPoE == UtIplst[count].ucIpType )?"PPPoE":"FIXIP" ),
                            strIpAddr,
                            UtIplst[count].ulRouterAreaId,
                            UtIplst[count].ulAnchorBts/*strIpAddr1*/);
                    count++;
                    srcMac.SetMac(UtIplst[count].aucMAC);   
                    }

                }
            }
        it++;
        if ( M_DM_FLOWCTRL_CNT == ++ucFlowCtrl )
            {
#ifdef __WIN32_SIM__
            //Win32:
            ::Sleep( 100 );//释放CPU
#else
            //VxWorks:
            ::taskDelay( 1 );
#endif
            ucFlowCtrl = 0;
            }
        if ( eid!=0 )
            break;
        }
    printf( "\r\n" );
#ifdef __WIN32_SIM__
    //释放showStatus信号量
    RELEASE();
#endif

}
void CTaskDm::showACLTable()
{
#ifdef __WIN32_SIM__
    //等showStatus信号量
    WAIT();
#endif

    printf( "\r\n" );
    printf( "\r\n*******************************" );
    printf( "\r\n*ACL Table *" );
    printf( "\r\n*******************************" );
    printf( "\r\n%-22s:%4d %s", "ACL Table",m_ACLTb.ucCount, "   Records" );
    printf( "\r\n-------------------------------------------------------------------------------------" );

    printf( "\r\n%-3s%-5s%-5s%-5s%-16s%-9s%-3s%-5s%-16s%-9s%-3s%-5s%-2s",
            "No","Flag","IsBC","Prot",
            "SrcAddr","Mask","Op","Port",
            "DstAddr","Mask","Op","Port",
            "P");

    printACL(PRTL_ARP,0);
    printACL(PRTL_DHCP,1);
    printACL(PRTL_PPP,2);
    printACL(PRTL_ForbidBroadcast,3);
    printACL(PRTL_PERMIT_AUTHED_ALL,4);
    printACL(PRTL_DENY_OTHERS,5);

    for ( UINT8 count=0;count<m_ACLTb.ucCount;++count )
        {
        printACL(m_ACLTb.PrtlFltTbEntry[count],count);  
        }
    printf( "\r\n" );
#ifdef __WIN32_SIM__
    //释放showStatus信号量
    RELEASE();
#endif
    return ;
}

/*ems发来终端请求,um接收后转发给dm,dm查找cpe table后代
um给ems回应答.
*/
void CTaskDm::ProcCpeProbeReq(CMessage &msg)
{
	UINT32 eid;
	UINT16  i;
	CPEProbeReq Req;

	UINT16 ReqTranid = *(UINT16*)(msg.GetDataPtr());
	eid = msg.GetEID();
	CPETable *RectCPE=CTaskDm::GetInstance()->GetCPERecordByEID(eid);
	if ( NULL == RectCPE )
	{
		Req.result = 1;//fail
		Req.ulEid = 0;
		Req.num = 0;
		CComMessage* pComMsg = new (this, 9) CComMessage;
		if (pComMsg==NULL)
		{
			LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in ProcCpeProbeReq.");
			return;
		}
		pComMsg->SetDstTid(M_TID_EMSAGENTTX);
		pComMsg->SetSrcTid(M_TID_UM);	//代um发送这个应答	
		pComMsg->SetMessageId(M_BTS_EMS_UT_IPLIST_RSP);
		*(UINT16*)((SINT8*)pComMsg->GetDataPtr()) = ReqTranid;
		memcpy(((UINT8*)(pComMsg->GetDataPtr())+2), &Req, 7);
		if(!CComEntity::PostEntityMessage(pComMsg))
		{
			pComMsg->Destroy();
			pComMsg = NULL;
			OAM_LOGSTR(LOG_DEBUG1, 0, "[tDM] post ProbeCPE message fail in ProcCpeProbeReq");
		}
		return;
	}
	Req.result = 0;
	Req.ulEid = eid;
	Req.num = RectCPE->ucFixSize + RectCPE->ucDynIpSize;
	for(i=0; i<Req.num;i++)
	{
		Req.cpeiplist[i].Ipaddr = RectCPE->pstIplstTbEnty[i].Data.stIpLease.ulIp;
		memcpy(Req.cpeiplist[i].MAC, RectCPE->pstIplstTbEnty[i].aucMAC, 6);
		
		Req.cpeiplist[i].type = RectCPE->pstIplstTbEnty[i].ucIpType;
	}
	CComMessage* pComMsg = new (this, (Req.num*11+7+2)) CComMessage;
	if (pComMsg==NULL)
	{
		LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in ProcCpeProbeReq.");
		return;
	}
	pComMsg->SetDstTid(M_TID_EMSAGENTTX);
	pComMsg->SetSrcTid(M_TID_UM);	//代um发送这个应答	
	pComMsg->SetMessageId(M_BTS_EMS_UT_IPLIST_RSP);
	*(UINT16*)((SINT8*)pComMsg->GetDataPtr()) = ReqTranid;
	memcpy(((UINT8*)(pComMsg->GetDataPtr())+2), &Req, (Req.num*11+7));
	if(!CComEntity::PostEntityMessage(pComMsg))
	{
		pComMsg->Destroy();
		pComMsg = NULL;
		OAM_LOGSTR(LOG_DEBUG1, 0, "[tCpeM] post ProbeCPE message fail in ProcCpeProbeReq");
	}
	return;
}
int video_debug = 0;
void openvideodebug()
{
	video_debug =1;
}
void closevideodebug()
{
	video_debug =0;
}

//lijinan 20101020 for video
void CTaskDm::ProcCpeVideoAddrRep(CMessage &msg)
{
		char *p = (char*)msg.GetDataPtr();
		char type  = *(p+4);
		char ip_num = *(p+5);
		UINT32 eid = msg.GetEID();
		if(video_debug)
		{
			LOG3(LOG_WARN, 0,"\nrec eid:0x%x msg type:%d,ip_num:%d\n",eid,type,ip_num);
		}
		 if(type==1||type==0)
		{
			CComMessage* pComMsg1 = new (this, 16) CComMessage;
			if (pComMsg1==NULL)
			{
				LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in ProcCpeVideoAddrRep.");
				return;
			}
			pComMsg1->SetEID(eid);
			pComMsg1->SetDstTid(M_TID_EB);
			pComMsg1->SetSrcTid(M_TID_UM);	//代um发送这个应答	
			pComMsg1->SetMessageId(MSGID_VEDIO_IPADDRESS_REPORT);
			char* pData = (char*)pComMsg1->GetDataPtr();
			if(ip_num==0||type==0)
			{
				pData[0] = 0;
			}
			else if(ip_num==1)
			{
				pData[0] = 1;
				memcpy((pData+1),(p+6),4);
			}
			else if(ip_num==2)
			{
				pData[0] = 2;
				memcpy((pData+1),(p+6),8);
			}
			else
			{
				pComMsg1->Destroy();
				pComMsg1 = NULL;
				return;
			}
			if(!CComEntity::PostEntityMessage(pComMsg1))
			{
				pComMsg1->Destroy();
				pComMsg1 = NULL;
				OAM_LOGSTR(LOG_DEBUG1, 0, "[tDM] post ProcCpeVideoAddrRep message1 fail ");
			}
		
		}
		else
			return;
		CComMessage* pComMsg = new (this, 4) CComMessage;
		if (pComMsg==NULL)
		{
			LOG(LOG_CRITICAL, 0, "Allocate empty commessage object failed in ProcCpeVideoAddrRep.");
			return;
		}
		pComMsg->SetDstTid(M_TID_UTDM);
		pComMsg->SetSrcTid(M_TID_UM);	//代um发送这个应答	
		pComMsg->SetMessageId(MSGID_VEDIO_IPADDRESS_RESULT);
		pComMsg->SetEID(eid);
		*(UINT16*)((SINT8*)pComMsg->GetDataPtr()) = 0;
		if(!CComEntity::PostEntityMessage(pComMsg))
		{
			pComMsg->Destroy();
			pComMsg = NULL;
			OAM_LOGSTR(LOG_DEBUG1, 0, "[tDM] post ProcCpeVideoAddrRep message fail ");
		}
		return;
}


/*============================================================
MEMBER FUNCTION:
    DMShow

DESCRIPTION:
    用于Tornado Shell上调用执行

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/

extern "C" int getSTDNum(int*);
void DMShow(UINT8 ucType)
{
    CTaskDm *taskDM = CTaskDm::GetInstance();
    switch ( ucType )
        {
        case 0:
            printf("\r\n DMShow(1): Show basic information");
            printf("\r\n DMShow(2): Show EID Table ");
            printf("\r\n DMShow(3): Show ACL Table");
            printf("\r\n");
            break;
        case 1:
            printf("\r\nShow EB basic information");
            taskDM->showStatus();
            break;
        case 2:
            {
                printf("\r\nShow EID table information");
                printf("\r\nPlease Enter EID(input 0 show all):");
                UINT32 ulEid = 0;
                getSTDNum((int*)&ulEid);
                printf( "EID = %d = 0x%x", ulEid, ulEid );
                taskDM->showEIDTable( ulEid );
            }
            break;
        case 3:
            {
                taskDM->showACLTable( );
            }
            break;
        case 4:
            {
                printf("\r\nDelete EID table information");
                printf("\r\nPlease Enter EID(input 0 show all):");
                UINT32 ulEid = 0;
                getSTDNum((int*)&ulEid);
                printf( "EID = %d = 0x%x", ulEid, ulEid );
                taskDM->ProCPELocationMove( ulEid, M_UT_REMOVE );
            }
            break;
        }

    return;
}


UINT16 getServingEIDnum()
{
    const CTaskDm *taskDM = CTaskDm::GetInstance();
    return taskDM->m_EIDptree.size();
}

#ifdef __WIN32_SIM__
/**************************************
 *模拟硬件，把Payload部分的数据，每16位
 *的高8位和低8位做一次倒换(ntohs)
 */
void CTaskDm::swap16(CMessage &msg)
{

    UINT16 usMsgId = msg.GetMessageId();
    if ( ( MSGID_UT_BTS_DATASERVICE_REQ == usMsgId )
         || ( MSGID_UT_BTS_DAIBUPDATE_RESP == usMsgId )
         || ( MSGID_UT_BTS_ADDRFLTR_TABLEUPDATE_RESP == usMsgId )
         || ( MSGID_UT_BTS_PROTOCOLFLTR_TABLEUPDATE_RESP == usMsgId )
       )
        {
        UINT8 *pData = (UINT8*)msg.GetDataPtr();
        UINT32 ulLen = msg.GetDataLength();

        for ( UINT32 ulIdx = 0; ulIdx < ulLen / 2; ++ulIdx )
            {
            UINT32 ulOffset = ulIdx * 2;
            UINT8 temp      = pData[ ulOffset ];
            pData[ ulOffset ]       = pData[ ulOffset + 1 ];
            pData[ ulOffset + 1 ]   = temp;
            }

        if ( 0 != ulLen % 2 )
            {
            DATA_assert( 0 );
            printf("!!!!!!!!!!!!!!!!!!!!!!!!!");
            }
        }

    return;
}
#endif

#ifdef __WIN32_SIM__
bool GetCPEConfig(UINT32 uleid,UINT8 *pConfig)
{
    UINT8 dataArray[30]=
    {01,00,01,01,01,01,14,00,
        00,00,00,00,
        00,00,00,00,00,00,
        00,00,00,00,
        00,00,00,00,
        00,00,00,00
    };
    memcpy(pConfig,dataArray,sizeof(dataArray));
    return true;
}
#endif
