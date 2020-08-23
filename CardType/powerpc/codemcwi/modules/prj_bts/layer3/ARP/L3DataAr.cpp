/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    ARP.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   11/23/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/
#ifdef __WIN32_SIM__
#include <winsock2.h>
#else   //VxWorks:
    #include "vxWorks.h" 
    #include "sockLib.h" 
    #include "inetLib.h" 
    #include "stdioLib.h" 
    #include "strLib.h" 
    #include "hostLib.h" 
    #include "ioLib.h" 
#endif


#include <stdio.h>
#include <string.h>
#ifndef WBBU_CODE
/* system */
#include "msgQLib.h"
#include "ioLib.h"
#include "tasklib.h"
#include "sysLib.h"
#include "string.h"
#include "stdio.h"
#include "semLib.h"
#include "wdLib.h"
#include "sockLib.h"
#include "in.h"
#include "pipedrv.h"
#include "kernelLib.h"
#include "rebootLib.h"
#include "wdLib.h"
#include "net/inet.h"
#include "ctype.h"
#include "routeLib.h"
#include "intLib.h"
#include "usrlib.h"
#include "net\Mbuf.h"
#include "ipProto.h"
#include "net\inet.h"
#include "muxlib.h"
#include "netbuflib.h"
#include "rnglib.h"
#include "netinet\in_var.h"
//#include "netinet\in.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"
#endif
#include "L3dataMsgId.h"
#include "L3DataEB.h"
#include "L3DataArp.h"
#include "L3DataMacAddress.h"
#include "L3dataArpConfig.h"
#include "ErrorCodeDef.h"
#include "L3OamAlminfo.h"
#include <pinglib.h>
#ifdef RCPE_SWITCH
extern UINT8 getIPLastD(UINT32 ip);
#endif

//任务实例指针的初始化
CTaskARP* CTaskARP::s_ptaskARP = NULL;
extern T_NvRamData *NvRamDataAddr;
extern UINT8 compwithLocalMac(UINT8 * macaddr);
extern UINT16   Wanif_Switch;
extern UINT32  WorkingWcpeEid;  
typedef map<UINT32, UINT16>::value_type ValType;

static UINT32 dwMask;
static UINT32 dwGateway;
static char bMac[6];
#ifndef WBBU_CODE
static UINT32 dwIP;
#else
 UINT32 dwIP;
 UINT32 GateWayIP = 0;
 extern  void sendCdrForReboot();
extern "C"  BOOL bspGetIsRebootBtsWanIfDiscEnabled();
unsigned char g_Mac[6]={0,0,0,0,0,0};
extern "C" typedef void (*pfARPRxCallBack)(unsigned int );
extern "C" void Arp_Drv_Register(pfARPRxCallBack   arpFunc);
#endif
RF_Operation_Para    g_rf_openation;
UINT8   g_Close_RF_flag = 0;
extern bool sagStatusFlag;
 extern bool AlarmReport(UINT8   Flag, UINT16  EntityType,
                 UINT16  EntityIndex, UINT16  AlarmCode,      
                 UINT8   Severity, const char format[],...);
 extern void  RF_config( BOOL closeflag);
 unsigned int g_arp_no_freelist =0;//wangwenhua add 2012-4-26
/*============================================================
MEMBER FUNCTION:
    ARP_AddILEntry

DESCRIPTION:
    全局函数，对外提供的增加ARP IP List的函数

ARGUMENTS:
    ulEid
    pMac
    ulIp

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void ARP_AddILEntry(UINT32 ulEid, const UINT8 *pMac, UINT32 ulIp, UINT8 flag)
{
    ////
    CTaskARP::GetInstance()->AddILEntry(ulEid, pMac, ulIp, flag);
}

/*============================================================
MEMBER FUNCTION:
    ARP_DelILEntry

DESCRIPTION:
    全局函数，对外提供的删除ARP IP List的函数

ARGUMENTS:
    ulIp
    Mac address

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
void ARP_DelILEntry(UINT32 ulIp, UINT8 *pMac,UINT8 flag)
{
    ////
    CTaskARP::GetInstance()->DelILEntry(ulIp, pMac,flag);
}

unsigned int CTaskARP::m_last_ip = 0;
unsigned int CTaskARP::m_gaprp_count = 0;//wangwenhua add 20101011
#ifdef WBBU_CODE
unsigned int  CTaskARP::m_err_count = 0;//wangwenhua add 20110516
#endif
/*============================================================
MEMBER FUNCTION:
    CTaskARP::CTaskARP

DESCRIPTION:
    CTDm构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/


CTaskARP::CTaskARP()
{
#ifndef WBBU_CODE
    BOOT_PARAMS params;
    struct ifnet 		*ifp;
    char	buf[100]; 
#else
       BOOT_PARAMS       bootParams;
    (void)bootStringToStruct(BOOT_LINE_ADRS, &bootParams);
	GateWayIP = inet_addr(bootParams.gad);
#endif
    LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "->CTaskARP()" );

#ifndef NDEBUG
    if ( !Construct( CObject::M_OID_ARP ) ) {
        LOG( LOG_SEVERE, LOGNO(ARP,ERR_ARP_SYS_ERR), "ERROR!!!CTaskARP()Construct failed." );
    }
#endif

    memset( m_szName, 0, sizeof( m_szName ) );
    memcpy( m_szName, M_TASK_ARP_TASKNAME, strlen( M_TASK_ARP_TASKNAME ) );
    m_uPriority     = M_TP_L3ARP;
    m_uOptions      = M_TASK_ARP_OPTION;
    m_uStackSize    = M_TASK_ARP_STACKSIZE;
    m_iMsgQMax =M_TASK_ARP_MAXMSG;
    m_iMsgQOption =M_TASK_ARP_MSGOPTION;

    ClearMeasure();
    initArpIplstTb();
    InitFreeArpLst();
    SetEgressArpEn(true);
    SetIngressArpEn(true);
#ifndef WBBU_CODE
	/* get mac addr */
	ifp = ifunit("mv0");
	if(!ifp)
	{
		printf("\nget mac addr failure!");
		return ;
	}	
	bcopy((char*)((struct arpcom *)ifp)->ac_enaddr, bMac, 6);

	/* get  ip and subnet mask */
	if(ifAddrGet("mv0", buf) != OK)
	{
		printf("\nget ip addr failure!");		
		return ;
	}
	dwIP = (UINT32)inet_addr(buf);
	if(ifMaskGet("mv0", (int*)&dwMask) != OK)
	{
		printf("\nget subnetmask failure!");		
		return ;
	}


	/* get default gateway */
#if 0
	bootStringToStruct (BOOT_LINE_ADRS, &params);
	/*route.ipRouteDest = 0;
	if(m2IpRouteTblEntryGet(M2_EXACT_VALUE, &route) == OK)
		dwGateway = route.ipRouteNextHop;
	else
		dwGateway = 0;*/
	dwGateway = inet_addr(params.gad);
	
#endif
   /*     AddILEntry(0xfffffffd,
                   (UINT8*)bMac,
                   (UINT32)dwIP);    
    
    */
#else
   dwIP =     inet_addr(bootParams.ead);
    
  memcpy(bMac,(char*)g_Mac,6);//wangwenhua add 20110516
   bootNetmaskExtract( bootParams.bad,(int*)&dwMask);
    pArpTimer= NULL;//sys_Createtimer(MSGID_ARP_Timer, 1, 30*1000);
    m_err_count = 0;
#endif
	memset((unsigned char*)&g_rf_openation,0,sizeof(RF_Operation_Para));
      if(NvRamDataAddr->Wcpe_Switch==0x5a5a)
      	{
      	    return;
      	}
      if(NvRamDataAddr->rf_operation.flag==0x55aa55aa)//初始化阶段将配置值赋给全局变量
      	{
      	     // memcpy(g_rf_openation,NvRamDataAddr->rf_operation,sizeof(g_rf_openation));
      	     if( NvRamDataAddr->rf_operation.type>4)
      	     	{
      	     	    g_rf_openation.type  = 0;
      	     	}
      	     else
      	     	{
      	     	
      	        g_rf_openation.type = NvRamDataAddr->rf_operation.type;
      	     	}
      	     if(NvRamDataAddr->rf_operation.Close_RF_Time_Len<60)
      	     	{
      	     	     g_rf_openation.Close_RF_Time_Len = 60;
      	     	}
      	     else
      	     	{
	   		g_rf_openation.Close_RF_Time_Len = NvRamDataAddr->rf_operation.Close_RF_Time_Len;
      	     	}
      	     if( g_rf_openation.Open_RF_Time_Len <30)
      	     	{
      	     	   g_rf_openation.Open_RF_Time_Len = 30;
      	     	}
      	     else
      	     	{
	   g_rf_openation.Open_RF_Time_Len = NvRamDataAddr->rf_operation.Open_RF_Time_Len;
      	     	}
	   g_rf_openation.GateWayIP1 = NvRamDataAddr->rf_operation.GateWayIP1;
	   g_rf_openation.GateWayIP2 = NvRamDataAddr->rf_operation.GateWayIP2;
	
      	}
}

/*============================================================
MEMBER FUNCTION:
    CTaskARP::~CTaskARP

DESCRIPTION:
    CTDm析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTaskARP::~CTaskARP()
{
    LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "->~CTaskARP" );

#ifndef NDEBUG
    if ( !Destruct( CObject::M_OID_ARP ) ) {
        LOG( LOG_SEVERE,LOGNO(ARP,ERR_ARP_SYS_ERR), "ERROR!!!~CTaskARP failed." );
    }
#endif
}


   bool CTaskARP::Initialize()
    {

         if ( !CBizTask::Initialize() )
	    {
	        LOG(LOG_CRITICAL,0,"CWRRU Initialize failed.");
	        return false;
	    }
	  m_SemId = semMCreate(SEM_Q_PRIORITY |SEM_INVERSION_SAFE | SEM_DELETE_SAFE );
	  if (NULL == m_SemId)
    {
        return false;
    }
	   #ifdef WBBU_CODE
         pArpTimer= sys_Createtimer(MSGID_ARP_Timer, 1, 30*1000);/***30s ****/
	  if(pArpTimer)
	  {
         	pArpTimer->Start();
	  }
         Arp_Drv_Register(CTaskARP::arpCallBack);
	#endif
	    UINT32 tid;
    if ( ( tid = ::taskSpawn( "tNetMonitor", 
                M_TASK_CLEANUP_PRIORITY, 
                M_TASK_CLEANUP_OPTION,
                M_TASK_CLEANUP_STACKSIZE,
                (FUNCPTR)RunNetWorksUp, (int)this, 0, 0, 0, 0, 0, 0, 0, 0, 0) ) == ERROR )
    {
        LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "task tNetMonitor failed." );

        return false;
    }
         return true;
    }

/*============================================================
MEMBER FUNCTION:
    CTaskARP::GetInstance

DESCRIPTION:
    Get CTaskARP Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTaskARP* 

SIDE EFFECTS:
    none
==============================================================*/
CTaskARP* CTaskARP::GetInstance()
{
    if ( NULL == s_ptaskARP ) {
        s_ptaskARP = new CTaskARP;
    }
    return s_ptaskARP;
}
#ifdef WBBU_CODE
unsigned int arp_right = 0;
unsigned int arp_err = 0;
  void  CTaskARP::arpCallBack(unsigned int ip)
   {

      //  static CTaskARP *pARPInstance = CTaskARP::GetInstance();
      if(ip==GateWayIP)
      	{
           m_err_count = 0;
           arp_right++;
      	}
      else
      	{
      	    arp_err++;
      	}
   
   }
#endif
void CTaskARP::showStatus()
{
#ifdef __WIN32_SIM__
    //等showStatus信号量
    WAIT();
#endif
    printf( "\r\n**********************" );
    printf( "\r\n*ARP Task Attributes *" );
    printf( "\r\n**********************" );
    printf( "\r\n%-20s: %d", "Task   stack   size",  M_TASK_ARP_STACKSIZE );
    printf( "\r\n%-20s: %d", "Task   Max  messages", M_TASK_ARP_MAXMSG );
    printf( "\r\n" );
    printf( "\r\n%-20s: %-10s","EgressArpProxyEn",m_blEgressArpEn?("True"):("False") );
    printf( "\r\n%-20s: %-10s","IngressArpProxyEn",m_blIngressArpEn?("True"):("False") );  


    printf( "\r\n" );
    printf( "\r\n*******************" );
    printf( "\r\n*ARP IPlist Table *" );
    printf( "\r\n*******************" );
    //Free CPE Entries
    printf( "\r\n%-20s: %-5d Records", "Free IPLIST Records", m_listFreeArp.size() );
    //BPtree

    if ( true == m_Arptree.empty() ) {
        printf( "\r\n%-20s: %s", "ARP IPlist", "0     Records" );
        printf( "\r\n" );
    }
    else {
        printf( "\r\n%-20s: %-5d Records", "ARP IPlist", m_Arptree.size() );
        printf( "\r\n%-5s%-10s%-20s%-20s%-20s%-20s", "No","Eid(hex)","Mac-Address","IP-Address","normal(0)/relay(1)","active(0)/Inactive(1)");
        printf( "\r\n----------------------------------------------" );

        map<UINT32, UINT16>::iterator it = m_Arptree.begin();
        UINT32 count=0;
        while ( m_Arptree.end() != it ) {
            if ( MAX_ARP_NODE <= it->second ) {
                it++;
                continue;
            }
            ArpILEntry *rectarp=&m_ArpIplstTb[it->second];
#if 0
            //by xiao. rectarp never null.
            if ( NULL == rectarp ) {
                it++;
                continue;
            }
#endif
            struct in_addr IpAddr;
#ifdef __WIN32_SIM__
            IpAddr.S_un.S_addr = htonl(rectarp->ulIpAddr);
#else
            IpAddr.s_addr = htonl(rectarp->ulIpAddr );
#endif
            SINT8 strIpAddr[ INET_ADDR_LEN ] = {0};
            inet_ntoa_b( IpAddr, strIpAddr );
            printf( "\r\n%-5d%.8X  %.2X-%.2X-%.2X-%.2X-%.2X-%.2X   %-20s %-5d  %-5d",
                    count,
                    rectarp->ulEid,
                    rectarp->aucMAC[0],rectarp->aucMAC[1],rectarp->aucMAC[2],
                    rectarp->aucMAC[3],rectarp->aucMAC[4],rectarp->aucMAC[5],
                    strIpAddr,rectarp->flag,rectarp->Active_flag);

            count++;
            it++;
        }
    }

    printf( "\r\n" );
    printf( "\r\n*******************************" );
    printf( "\r\n* ARP  Traffic  Measure       *" );
    printf( "\r\n*******************************" );
    printf( "\r\n%-15s","From Direct");

    int from,to;
    for ( from=ARP_FROM_AI;from<ARP_FROM_MAX;++from ) {
        printf( "%-15s",strARPFromDir[from]);
    }
    printf( "\r\n----------------------------------------------" );
    for ( to=ARP_TO_AI;to<ARP_TO_MAX;++to ) {
        printf( "\r\n%-15s",strARPToDir[to]);
        for ( from=ARP_FROM_AI;from<ARP_FROM_MAX;++from ) {
            printf( "%-15d",m_arDirTrafficMeasure[from][to]);
        }
    }

    printf( "\r\n" );

#ifdef __WIN32_SIM__
    //释放showStatus信号量
    RELEASE();
#endif
    return;
}


/*============================================================
MEMBER FUNCTION:
    CTaskARP::ProcessMessage

DESCRIPTION:
    消息处理函数
ARGUMENTS:
    *CMessage: 消息
RETURN VALUE:
    bool:TRUE or false,FrameWork根据返回值决定是否做PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskARP::ProcessComMessage(CComMessage *pComMsg)
{
    UINT16 usMsgId = pComMsg->GetMessageId();
    switch ( usMsgId ) {
    case MSGID_ARP_PROXY_REQ:
        //arp proxy request
        proArpProxyReq(pComMsg);
        break;
    case M_CFG_ARP_DATA_SERVICE_CFG_REQ:
        //arp config
        proArpConfig(pComMsg);
        break;
#ifdef UNITEST
    case MSGID_IPLIST_ARP_ADD_NOTIFICATION:
        AddILEntry(((ArpILEntry*)pComMsg->GetDataPtr())->ulEid,
                   ((ArpILEntry*)pComMsg->GetDataPtr())->aucMAC,
                   ((ArpILEntry*)pComMsg->GetDataPtr())->ulIpAddr);
        break;
    case MSGID_IPLIST_ARP_DEL_NOTIFICATION:
        DelILEntry(((ArpILEntry*)pComMsg->GetDataPtr())->ulIpAddr);
        break;
#endif  
#ifdef WBBU_CODE //wangwenhua add 20110517 
      case MSGID_ARP_Timer:
           GateWayIPsend_arp_2GateWay();
          
	    m_err_count++;
	    if(m_err_count>20)
	    	{
	    	       if (bspGetIsRebootBtsWanIfDiscEnabled())
        		{
                            
                               LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "Reboot BTS due to arp gateway no RSP OVer 10 mins");
		                bspSetBtsResetReason(RESET_REASON_ARPNOT_GTAEWAY/*RESET_REASON_SW_ABNORMAL*/);
				  sendCdrForReboot();
		                taskDelay(50);
		                rebootBTS(BOOT_CLEAR);
        
        		}
	    	       m_err_count = 0;
	    }
	break;
#endif
    default :
        LOG1( LOG_DEBUG, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "->ProcessMessage: Unexpected message, MsgId: 0x%x.", usMsgId );
        break;
    }
    //destroy message.
    pComMsg->Destroy();

#ifdef __WIN32_SIM__
    showStatus();
#endif
    return true;
}
#ifdef M_TGT_WANIF

extern UINT32  RelayWanifCpeEid[20];
#endif
//deal with ArpProxyRequest
void CTaskARP::proArpProxyReq(CComMessage *pComMsg)
{
    UINT32 eid=pComMsg->GetEID();
    CComMessage *pComMsg_Copy;
    unsigned short datalen = pComMsg->GetDataLength();
    LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "->proArpProxyReq" );
    
    if ( M_ETHER_TYPE_ARP != GetProtoType(pComMsg) )  //add by xiao.
    {
        LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_SYS_ERR), "Receive unexpected packet(not ARP)." );
        return;
    }
    
    EtherHdr *pEtherPkt = (EtherHdr*) ( pComMsg->GetDataPtr() );
    ArpHdr *pArphdr;
    
    if(true == hasVlanTag(pComMsg))
    {
        EtherHdrEX *pEtherHdr1 = (EtherHdrEX*) ( pComMsg->GetDataPtr() );
        //modify by wangx
        if(IS_8023_PACKET(ntohs(pEtherHdr1->usProto)))
        {
            pArphdr = (ArpHdr*)((UINT8*)pEtherPkt + sizeof(EtherHdrEX) + sizeof(LLCSNAP));
        }
        else
        {
            pArphdr = (ArpHdr*)((UINT8*)pEtherPkt + sizeof(EtherHdrEX));
        }
    }
    else
    {
        if(IS_8023_PACKET(ntohs(pEtherPkt->usProto)))
        {
            pArphdr = (ArpHdr*)((UINT8*)pEtherPkt + sizeof(EtherHdr) + sizeof(LLCSNAP));
        }
        else
        {
            pArphdr = (ArpHdr*)((UINT8*)pEtherPkt + sizeof(EtherHdr));
        }
    }
    if ( M_ARP_REQUEST == ntohs(pArphdr->usOp) ) 
    {
        //如果源mac或者源ip为0或全f，丢弃 add for liuweidong heishui test 20120816
        if((pArphdr->ulSenderPaddr==0)||(pArphdr->ulSenderPaddr==0xffffffff))
        {
            LOG2( LOG_DEBUG1, LOGNO(ARP,ERR_ARP_SYS_ERR), "arp srcip is %x, eid:%x, discard msg.", \
                pArphdr->ulSenderPaddr, pComMsg->GetEID());
            return;
        }
        UINT8 tmpmac1[6], tmpmac2[6];
        memset(tmpmac1, 0, 6);
        memset(tmpmac2, 0xff, 6);
        
        if((memcmp(pArphdr->aucSenderHaddr, tmpmac1, pArphdr->ucHlen)==0)\
            ||(memcmp(pArphdr->aucSenderHaddr, tmpmac2, pArphdr->ucHlen)==0))
        {
            LOG2( LOG_DEBUG1, LOGNO(ARP,ERR_ARP_SYS_ERR), "arp srcmac is %x, eid:%x, discard msg.", \
                pArphdr->aucSenderHaddr[0], pComMsg->GetEID());
            return;
        }
        //deal with Arp Request       
        #ifdef RCPE_SWITCH
        //针对RCPE+，ARP表记录有问题，接收到GARP后应修改记录
        int iIpEqual = memcmp( (UINT8*)&pArphdr->ulSenderPaddr, (UINT8*)&pArphdr->ulDestPaddr, pArphdr->ucPlen );
        int iMacEqual = memcmp( pArphdr->aucSenderHaddr, pArphdr->aucDestHaddr, pArphdr->ucHlen );
        int sendRcpeFlag = 0;
        if( 0==iIpEqual && 0==iMacEqual )
        {
            LOG4( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "MRCPE GET GARP REQ: IP[%-3d], MAC[%02X-%02X-%02X]", 
            getIPLastD(pArphdr->ulSenderPaddr),*(pArphdr->aucSenderHaddr+3), *(pArphdr->aucSenderHaddr+4), *(pArphdr->aucSenderHaddr+5) );
            UINT32 ulIdx = BPtreeFind(pArphdr->ulSenderPaddr);
            if( M_DATA_INDEX_ERR != ulIdx )
            {
                if( 0 == pComMsg->GetEID() /*|| trunkIsTrunkRcpe(pComMsg->GetEID())*/ )
                {
	                  semTake (m_SemId, WAIT_FOREVER);
                    BPtreeDel( pArphdr->ulSenderPaddr );
                    InsertFreeArpLst(ulIdx);
			     semGive (m_SemId);
                }
            }
            
            if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)  //forward the arq require to rcpe ,evern we cannot find it yet.
            {
                //LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "d:[Downlink]ARP proxy cant find ARP entry for IP[%-3d], discard packet.",getIPLastD(ntohl(pArphdr->ulDestPaddr)));
                for(int i = 0; i<NvRamDataAddr->Relay_num ;i++)
                {
                    if( ( (RelayWanifCpeEid[i]!=0)  && (pComMsg->GetEID()!=RelayWanifCpeEid[i]) ) /*||
                    
                    ( (RelayWanifCpeEid[i]!=0) && ( DIR_FROM_WAN == from) )*/ )
                    {
                        LOG4( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "MRCPE Send GARP: IP[%-3d], MAC[%02X-%02X] TO EID[%08X]", 
                        getIPLastD(pArphdr->ulSenderPaddr), *(pArphdr->aucSenderHaddr+4), *(pArphdr->aucSenderHaddr+5), RelayWanifCpeEid[i] );
                        trunkSendMsg( M_TID_EB, 
                        MSGID_TRAFFIC_FORWARD, 
                        (UINT8*)pComMsg->GetDataPtr(), 
                        pComMsg->GetDataLength(), 
                        DIR_TO_AI, 
                        RelayWanifCpeEid[i],
                        pComMsg->GetVlanID());
                    }
                }
                sendRcpeFlag = 1;//已经发给rcpe了
            }   
            
        }
        #endif
        DIRECTION from =(DIRECTION)pComMsg->GetDirection();
	    switch(from)
        {
            case DIR_FROM_WAN:
                proArpProxyReq_WAN( pComMsg, pEtherPkt, pArphdr, sendRcpeFlag);
                break;
            case DIR_FROM_AI:
                proArpProxyReq_AI( pComMsg, pEtherPkt, pArphdr, sendRcpeFlag);
                break;
            case DIR_FROM_TDR:
                proArpProxyReq_TDR( pComMsg, pEtherPkt, pArphdr);
                break;
            default:
                break;
        }
	       if(g_rf_openation.type!=0)//wangwenhua add 2012-3-2 
        {
                if(g_rf_openation.GateWayIP1!=0)
                	{
                	  if(g_rf_openation.GateWay1_valid )
                	  	{
			                 if(pArphdr->ulDestPaddr == g_rf_openation.GateWayIP1)
			                 {
			                     //  memcpy(g_rf_openation.GateWay1_MAC,pArphdr->aucSenderHaddr,6);   //将MAC 地址给记录下
			                       g_rf_openation.GateWay1_UP++;
						  
			                 }
                	  	}
                	}
                 if(g_rf_openation.GateWayIP2!=0)
                 {
                    if(g_rf_openation.GateWay2_valid )
                    	{
			       if(pArphdr->ulDestPaddr == g_rf_openation.GateWayIP2)
	                 {
	                      // memcpy(g_rf_openation.GateWay2_MAC,pArphdr->aucSenderHaddr,6);
				 g_rf_openation.GateWay2_UP++;
	                 }
                    	}
                 }
		   
        }
	 
    }
    else if ( M_ARP_REPLY == ntohs(pArphdr->usOp) )
    {
        LOG1(LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "Receive ARP Reply packet(IP:0x%x), forward it.", ntohl(pArphdr->ulSenderPaddr));
        //deal with Arp Reply
          //wangwenhua add 2012-6-20 
      
        DIRECTION from =(DIRECTION)pComMsg->GetDirection();
        if(from==DIR_FROM_AI)
        {
            ArpILEntry *pSrcARPEntry = GetArpIplstByIp(ntohl(pArphdr->ulSenderPaddr));
            if(pSrcARPEntry!=NULL)
            {
                memcpy(pSrcARPEntry->aucMAC,pArphdr->aucSenderHaddr,M_MAC_ADDRLEN);
                pSrcARPEntry->ulEid = eid;
                pSrcARPEntry->ulIpAddr = pArphdr->ulSenderPaddr;
                
                pSrcARPEntry->Active_flag = 0;//wangwenhua add 2012-6-20  只要是增加的都设置为active状态
                LOG2(LOG_SEVERE, LOGNO(ARP,ERR_ARP_NORMAL), "Receive ARP Reply packet from AI(eid:0x%x,IP:0x%x)", eid,ntohl(pArphdr->ulSenderPaddr));
            }
        }
        
        pComMsg->SetDstTid( M_TID_EB);
        pComMsg->SetSrcTid(M_TID_ARP);
        //设置MsgID;
        pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
        CComEntity::PostEntityMessage( pComMsg );
        //measure performance to Snoop 
        IncreaseDirTrafficMeasureByOne((ARPFROMMEASURE)from,ARP_REPLY);
        if(g_rf_openation.type!=0)
        {
                if(g_rf_openation.GateWayIP1!=0)
                	{
	                 if(pArphdr->ulSenderPaddr == g_rf_openation.GateWayIP1)
	                 {
	                       memcpy(g_rf_openation.GateWay1_MAC,pArphdr->aucSenderHaddr,6);   //将MAC 地址给记录下
	                       g_rf_openation.GateWay1_valid = 1;
				  
	                 }
                	}
                 if(g_rf_openation.GateWayIP2!=0)
                 {
			       if(pArphdr->ulSenderPaddr == g_rf_openation.GateWayIP2)
	                 {
	                       memcpy(g_rf_openation.GateWay2_MAC,pArphdr->aucSenderHaddr,6);
				 g_rf_openation.GateWay2_valid = 1;
	                 }
                 }
		   
        }
    }
    return;
}
void CTaskARP::proArpProxyReq_WAN(CComMessage *pComMsg, EtherHdr *pEtherPkt, ArpHdr *pArphdr, int sendRcpeFlag)
{
    UINT32 eid=0;
    CComMessage *pComMsg_Copy;
    unsigned short datalen = pComMsg->GetDataLength();
    LOG( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "->proArpProxyReq_WAN" ); 
     int iIpEqual = memcmp( (UINT8*)&pArphdr->ulSenderPaddr, (UINT8*)&pArphdr->ulDestPaddr, pArphdr->ucPlen );
    
    if ( !GetEgressArpEn() ) 
    {
        ArpILEntry *pDstARPEntry = GetArpIplstByIp(pArphdr->ulDestPaddr);
        UINT32 para;
        if ( NULL == pDstARPEntry )
        {
            //missing arpip,broadcast arpreq to AI
            para=M_DATA_BROADCAST_EID;
            //rcpe+switch+bts情况下，arp请求丢失，增加以下处理
            if((NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)&&(sendRcpeFlag==0))  //forward the arq require to rcpe ,evern we cannot find it yet.
            {
                for(UINT8 uc = 0; uc<NvRamDataAddr->Relay_num ;uc++)
                {
                    if( RelayWanifCpeEid[uc]==0 )
                        continue;
                    trunkSendMsg( M_TID_EB, 
                        MSGID_TRAFFIC_FORWARD, 
                        (UINT8*)pComMsg->GetDataPtr(), 
                        pComMsg->GetDataLength(), 
                        DIR_TO_AI, 
                        RelayWanifCpeEid[uc],
                        pComMsg->GetVlanID());
                }
                sendRcpeFlag = 1;//已经发给rcpe了
            }   
        }
        else 
        {
            para = pDstARPEntry->ulEid;
            #ifdef RCPE_SWITCH
            if(pDstARPEntry->flag==1)
            {
                if((NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)&&(sendRcpeFlag==0))  //forward the arq require to rcpe ,evern we cannot find it yet.
                {
                    LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "send arp request from wan [IP=%-3d] to all rcpe.",getIPLastD(ntohl(pArphdr->ulDestPaddr)));
                    for(int i = 0; i<NvRamDataAddr->Relay_num ;i++)
                    {
                        if(RelayWanifCpeEid[i]!=0)//send to other rcpe
                        {
                            trunkSendMsg( M_TID_EB, 
                                MSGID_TRAFFIC_FORWARD, 
                                (UINT8*)pComMsg->GetDataPtr(), 
                                pComMsg->GetDataLength(), 
                                DIR_TO_AI, 
                                RelayWanifCpeEid[i],
                                pComMsg->GetVlanID());
                        }
                    }
                    sendRcpeFlag = 1;//已经发给rcpe了
                }   
            }
            
            #endif
        }
        
        pComMsg->SetDstTid(M_TID_EB);
        pComMsg->SetSrcTid(M_TID_ARP);
        pComMsg->SetDirection(DIR_TO_AI);
        pComMsg->SetEID(para);
        //设置MsgID;
        pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
        LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "Downlink ARP Proxy is disabled. forward to CPE[0x%X]", para );
        CComEntity::PostEntityMessage( pComMsg );
        
        IncreaseDirTrafficMeasureByOne(ARP_FROM_WAN,ARP_TO_AI);
        return;
    }  
    
    
    LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy is enabled.");
    ArpILEntry *pDstARPEntry = GetArpIplstByIp(ntohl(pArphdr->ulDestPaddr));
    if ( NULL == pDstARPEntry ) 
    {        
        LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "[Downlink]ARP proxy cant find ARP entry for IP(:0x%X), discard packet.",ntohl(pArphdr->ulDestPaddr));
        IncreaseDirTrafficMeasureByOne(ARP_FROM_WAN,ARP_UNKNOW);
        //rcpe修改，将arp包转给rcpe wangwenhua20100721
        if((NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)&&(sendRcpeFlag==0))  //forward the arq require to rcpe ,evern we cannot find it yet.
        {
            for(int i = 0; i<NvRamDataAddr->Relay_num ;i++)
            {
                if( /*( (RelayWanifCpeEid[i]!=0)  && (DIR_FROM_AI == from) && (eid!=RelayWanifCpeEid[i]) ) ||*/
                
                ( (RelayWanifCpeEid[i]!=0)) )
                {
                    pComMsg_Copy= new(this, datalen)CComMessage;
		if(pComMsg_Copy!=NULL)
			{
                    memcpy((unsigned char*)pComMsg_Copy->GetDataPtr(),(unsigned char*)pComMsg->GetDataPtr(),datalen);
                    pComMsg_Copy->SetDstTid(M_TID_EB);
                    pComMsg_Copy->SetSrcTid(M_TID_ARP);
                    pComMsg_Copy->SetDirection(DIR_TO_AI);
                    pComMsg_Copy->SetEID(RelayWanifCpeEid[i]);
                    //设置MsgID;
                    pComMsg_Copy->SetMessageId( MSGID_TRAFFIC_FORWARD );
                    UINT16 usVlanId = pComMsg->GetVlanID();
                    pComMsg_Copy->SetVlanID(usVlanId);
                    if (false == CComEntity::PostEntityMessage( pComMsg_Copy ))
                    {
                        LOG1(LOG_SEVERE, LOGNO(ARP, ERR_ARP_SYS_ERR) , "send ARP to RCPE EID:%x,fail.",RelayWanifCpeEid[i]);
                        pComMsg_Copy->Destroy();
                    
                    }	
			}
                }
            }
            sendRcpeFlag = 1;//已经发给rcpe了            
        }
        //如果是学习模式，则广播给终端，下面的wcpe也会收到，所以这里就直接return
        if (WM_LEARNED_BRIDGING == CTBridge::GetInstance()->GetWorkingMode())
        {
            //如果是学习模式，往AI转发，否则，丢弃该请求报
            pComMsg->SetDstTid(M_TID_EB);
            pComMsg->SetSrcTid(M_TID_ARP);
            pComMsg->SetDirection(DIR_TO_AI);
            pComMsg->SetEID(M_DATA_BROADCAST_EID);
            //设置MsgID;
            pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
            CComEntity::PostEntityMessage( pComMsg );
            return;
        } 
        if(Wanif_Switch==0x5a5a)
        {
            UINT8 flag = compwithLocalMac(pEtherPkt->aucSrcMAC); 
            if(flag == 2)
            {
                pComMsg->SetDstTid(M_TID_EB);
                pComMsg->SetSrcTid(M_TID_ARP);
                pComMsg->SetDirection(DIR_TO_AI);
                pComMsg->SetEID(WorkingWcpeEid);
                //设置MsgID;
                pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
                CComEntity::PostEntityMessage( pComMsg );
            } 
            return;
        }  
        return;      
    }
    else 
    {
        LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy find ARP entry for IP(:0x%X).", ntohl(pArphdr->ulDestPaddr));
        
        if((pDstARPEntry->flag==1)||(pDstARPEntry->Active_flag==1)||(iIpEqual==0))//如果是GARP包，也直接送给 EID对应的终端，用于检测IP地址冲突
        {
            #ifdef RCPE_SWITCH
            if((NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)&&(sendRcpeFlag==0))  //forward the arq require to rcpe ,evern we cannot find it yet.
            {
                LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "send arp request from ai [IP=%-3d] to all rcpe.",getIPLastD(ntohl(pArphdr->ulDestPaddr)));
                for(int i = 0; i<NvRamDataAddr->Relay_num ;i++)
                {
                    if(RelayWanifCpeEid[i]!=0)//send to other rcpe
                    {
                        trunkSendMsg( M_TID_EB, 
                            MSGID_TRAFFIC_FORWARD, 
                            (UINT8*)pComMsg->GetDataPtr(), 
                            pComMsg->GetDataLength(), 
                            DIR_TO_AI, 
                            RelayWanifCpeEid[i],
                            pComMsg->GetVlanID());
                    }
                }
                sendRcpeFlag = 1;//已经发给rcpe了
            }   
            #endif
            pComMsg->SetDstTid(M_TID_EB);
            pComMsg->SetSrcTid(M_TID_ARP);
            pComMsg->SetDirection(DIR_TO_AI);
            pComMsg->SetEID(pDstARPEntry->ulEid);
            //设置MsgID;
            pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
            CComEntity::PostEntityMessage( pComMsg );
            return;
        }
        
        pArphdr->usOp = htons(M_ARP_REPLY);
        memcpy(pArphdr->aucDestHaddr, pArphdr->aucSenderHaddr,M_MAC_ADDRLEN);
        UINT32 tmpip = htonl(pArphdr->ulDestPaddr);
        pArphdr->ulDestPaddr = pArphdr->ulSenderPaddr;
        
        memcpy(pArphdr->aucSenderHaddr,pDstARPEntry->aucMAC,M_MAC_ADDRLEN); 
        pArphdr->ulSenderPaddr = tmpip;
        
        memcpy(pEtherPkt->aucDstMAC, pEtherPkt->aucSrcMAC,M_MAC_ADDRLEN);
        memcpy(pEtherPkt->aucSrcMAC, pDstARPEntry->aucMAC,M_MAC_ADDRLEN);
        
        LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy send an ARP reply(IP:0x%X) to WAN.", ntohl(pArphdr->ulSenderPaddr));
        pComMsg->SetDstTid(M_TID_EB);
        pComMsg->SetSrcTid(M_TID_ARP);
        pComMsg->SetDirection(DIR_TO_WAN);
        //设置MsgID;
        pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
        CComEntity::PostEntityMessage( pComMsg );
        
        IncreaseDirTrafficMeasureByOne(ARP_FROM_WAN,ARP_TO_WAN);
        return;
        
    }       
    
}
void CTaskARP::proArpProxyReq_AI(CComMessage *pComMsg, EtherHdr *pEtherPkt, ArpHdr *pArphdr, int sendRcpeFlag)
{
    UINT32 eid=0;
    CComMessage *pComMsg_Copy;
    unsigned short datalen = pComMsg->GetDataLength();
    LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "->proArpProxyReq_AI" ); 
   	 	
    eid = pComMsg->GetEID();
    if(NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)
    {
        for(int i = 0; i<NvRamDataAddr->Relay_num ;i++)
        {
            if((eid==RelayWanifCpeEid[i])&&(eid!=0))
            {
                AddILEntry(  eid,  pArphdr->aucSenderHaddr,pArphdr->ulSenderPaddr,1 ) ;   
                break;
            }
        }
    }   
    if( pArphdr->ulDestPaddr == dwIP) //有用户试图访问本基站
    {
        if((pArphdr->ulSenderPaddr & dwMask) == (dwIP & dwMask)) //同一网段内的终端,直接做代理,否则交给网络正常处理
            {
            pArphdr->usOp = htons(M_ARP_REPLY);
            memcpy(pArphdr->aucDestHaddr, pArphdr->aucSenderHaddr,M_MAC_ADDRLEN);
            UINT32 tmpip = htonl(pArphdr->ulDestPaddr);
            pArphdr->ulDestPaddr = pArphdr->ulSenderPaddr;
            
            memcpy(pArphdr->aucSenderHaddr,bMac,M_MAC_ADDRLEN); 
            pArphdr->ulSenderPaddr = tmpip;
            
            memcpy(pEtherPkt->aucDstMAC, pEtherPkt->aucSrcMAC,M_MAC_ADDRLEN);
            memcpy(pEtherPkt->aucSrcMAC, bMac,M_MAC_ADDRLEN);
            
            LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "some local user try to visit BTS,ARP proxy send an BTS reply(IP:0x%X) to AI.", ntohl(pArphdr->ulSenderPaddr));
            pComMsg->SetDstTid(M_TID_EB);
            pComMsg->SetSrcTid(M_TID_ARP);
            pComMsg->SetDirection(DIR_TO_AI);
            //设置MsgID;
            pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
            CComEntity::PostEntityMessage( pComMsg );
            
            IncreaseDirTrafficMeasureByOne(ARP_FROM_AI,ARP_TO_AI);
            return;
        
        }
        else 
        {
            LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "some local user try to visit BTS,but in different subnetwork,not proxy(IP:0x%X) to AI.", ntohl(pArphdr->ulSenderPaddr));
        
        }
    }
    
    if (!GetIngressArpEn() )
    {
        if((NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)&&(sendRcpeFlag==0))  //forward the arq require to rcpe ,evern we cannot find it yet.
        {
            for(int i = 0; i<NvRamDataAddr->Relay_num ;i++)
            {
                if((RelayWanifCpeEid[i]!=0)&& (eid!=RelayWanifCpeEid[i]))
                {
                    pComMsg_Copy= new(this, datalen)CComMessage;
			if(pComMsg_Copy!=NULL)
				{
                    memcpy((unsigned char*)pComMsg_Copy->GetDataPtr(),(unsigned char*)pComMsg->GetDataPtr(),datalen);
                    pComMsg_Copy->SetDstTid(M_TID_EB);
                    pComMsg_Copy->SetSrcTid(M_TID_ARP);
                    pComMsg_Copy->SetDirection(DIR_TO_AI);
                    pComMsg_Copy->SetEID(RelayWanifCpeEid[i]);
                    UINT16 usVlanId = pComMsg->GetVlanID();
                    pComMsg_Copy->SetVlanID(usVlanId);
                    //设置MsgID;
                    pComMsg_Copy->SetMessageId( MSGID_TRAFFIC_FORWARD );
                    if (false == CComEntity::PostEntityMessage( pComMsg_Copy ))
                    {
                        LOG1(LOG_SEVERE, LOGNO(ARP, ERR_ARP_SYS_ERR) , " send ARP2 to RCPE  EID:%x,fail.",RelayWanifCpeEid[i]);
                        pComMsg_Copy->Destroy();                            
                    }
                    }									
                }
            }
	        sendRcpeFlag = 1;
        }
        pComMsg->SetDstTid(M_TID_EB);
        pComMsg->SetSrcTid(M_TID_ARP);
        pComMsg->SetDirection(DIR_TO_WAN);
        //设置MsgID;
        pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
        LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "Uplink ARP Proxy is disabled. forward to WAN");
        CComEntity::PostEntityMessage( pComMsg );
        
        IncreaseDirTrafficMeasureByOne(ARP_FROM_AI,ARP_TO_WAN);
        
        return;
    }
    
    LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy is enabled.");
    ArpILEntry *pDstARPEntry = GetArpIplstByIp(ntohl(pArphdr->ulDestPaddr));
    if ( NULL == pDstARPEntry ) 
    {        
        LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "[Uplink]ARP proxy cant find ARP entry for IP(:0x%X), forward packet to EB.",ntohl(pArphdr->ulDestPaddr));
        //rcpe修改，将arp包转给rcpe wangwenhua20100721
        if((NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)&&(sendRcpeFlag==0))  //forward the arq require to rcpe ,evern we cannot find it yet.
        {
            for(int i = 0; i<NvRamDataAddr->Relay_num ;i++)
            {
                if( ( (RelayWanifCpeEid[i]!=0)&& (eid!=RelayWanifCpeEid[i]) ) /*||
                
                ( (RelayWanifCpeEid[i]!=0) && ( DIR_FROM_WAN == from) )*/ )
                {
                    pComMsg_Copy= new(this, datalen)CComMessage;
			if(pComMsg_Copy!=NULL)
				{
                    memcpy((unsigned char*)pComMsg_Copy->GetDataPtr(),(unsigned char*)pComMsg->GetDataPtr(),datalen);
                    pComMsg_Copy->SetDstTid(M_TID_EB);
                    pComMsg_Copy->SetSrcTid(M_TID_ARP);
                    pComMsg_Copy->SetDirection(DIR_TO_AI);
                    pComMsg_Copy->SetEID(RelayWanifCpeEid[i]);
                    UINT16 usVlanId = pComMsg->GetVlanID();
                    pComMsg_Copy->SetVlanID(usVlanId);
                    //设置MsgID;
                    pComMsg_Copy->SetMessageId( MSGID_TRAFFIC_FORWARD );
                    if (false == CComEntity::PostEntityMessage( pComMsg_Copy ))
                    {
                        LOG1(LOG_SEVERE, LOGNO(ARP, ERR_ARP_SYS_ERR) , " send ARP2 to RCPE  EID:%x,fail.",RelayWanifCpeEid[i]);
                        pComMsg_Copy->Destroy();                            
                    }	
				}
                }
            }
	        sendRcpeFlag = 1;
        }
        pComMsg->SetDstTid(M_TID_EB);
        pComMsg->SetSrcTid(M_TID_ARP);
        //设置MsgID;
        pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
        CComEntity::PostEntityMessage( pComMsg );
        IncreaseDirTrafficMeasureByOne(ARP_FROM_AI,ARP_TO_AI); 
        return;        
    }
    else 
    {
        LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy find ARP entry for IP(:0x%X).", ntohl(pArphdr->ulDestPaddr)); 
        
        #ifdef RCPE_SWITCH
        if(pDstARPEntry->flag==1)
        {
            int iIpEqual = memcmp( (UINT8*)&pArphdr->ulSenderPaddr, (UINT8*)&pArphdr->ulDestPaddr, pArphdr->ucPlen );
            int iMacEqual = memcmp( pArphdr->aucSenderHaddr, pArphdr->aucDestHaddr, pArphdr->ucHlen );
            if((NvRamDataAddr->Relay_WcpeEid_falg ==0xa5a5a5a5)&&(sendRcpeFlag==0))  //forward the arq require to rcpe ,evern we cannot find it yet.
            {
                LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "send arp request from ai [IP=%-3d] to all rcpe.",getIPLastD(ntohl(pArphdr->ulDestPaddr)));
                for(int i = 0; i<NvRamDataAddr->Relay_num ;i++)
                {
                    if( (RelayWanifCpeEid[i]!=0)&& (eid!=RelayWanifCpeEid[i]))//send to other rcpe
                    {
                        trunkSendMsg( M_TID_EB, 
                            MSGID_TRAFFIC_FORWARD, 
                            (UINT8*)pComMsg->GetDataPtr(), 
                            pComMsg->GetDataLength(), 
                            DIR_TO_AI, 
                            RelayWanifCpeEid[i],
                            pComMsg->GetVlanID());
                    }
                }
		        sendRcpeFlag = 1;
            }   
            if( 0==iIpEqual && 0==iMacEqual )//rcpe上传的garp包直接转给网络侧
            {
                pComMsg->SetDstTid(M_TID_EB);
                pComMsg->SetSrcTid(M_TID_ARP);
                pComMsg->SetDirection(DIR_TO_WAN);
                //设置MsgID;
                pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
                            LOG1( LOG_DEBUG, LOGNO(ARP,ERR_ARP_NORMAL), "Uplink GARP to WAN, IP:%x", getIPLastD(ntohl(pArphdr->ulDestPaddr)));
                CComEntity::PostEntityMessage( pComMsg );
                return;							
            }
            if(eid!=pDstARPEntry->ulEid)
            {
                pComMsg->SetDstTid(M_TID_EB);
                pComMsg->SetSrcTid(M_TID_ARP);
                pComMsg->SetDirection(DIR_TO_AI);
                pComMsg->SetEID(pDstARPEntry->ulEid);
                //设置MsgID;
                pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
                CComEntity::PostEntityMessage( pComMsg );
                LOG2( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "pDstARPEntry&&DIR_FROM_AI&&pDstARPEntry->flag IP[%-3d] EID(:0x%08X).", getIPLastD(ntohl(pArphdr->ulDestPaddr)), pDstARPEntry->ulEid );
                
            }
            else
            {
                LOG2( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "wwh pDstARPEntry&&DIR_FROM_AI&&pDstARPEntry->flag IP[%-3d] EID(:0x%08X).", getIPLastD(ntohl(pArphdr->ulDestPaddr)), pDstARPEntry->ulEid );
            }
            return;
        }
        #endif
        
        CMac MacInARPReq(pArphdr->aucSenderHaddr);
        CMac MacInARPEntry(pDstARPEntry->aucMAC);
        if (MacInARPReq == MacInARPEntry)//GARP包发送给WAN
        {
            //发ARP Request的MAC跟ARP表的MAC一样
            //该ARP Request多半是ARP探测是否有重复IP
            /*******************wangwenhua add mark 20101011
            
            上次修改增加m_last_ip记录为了防止多次送 GARP包给网络侧，当终端切换
            到另外一个基站时，可能删除服务基站上的转发表。
            现在修改是这样的。增加对同一IP的记录，保证只发送1次，第2，3次不发送。
            当终端重新发起GARP包时，继续发送第一次。
            主要是考虑到网络中只有一个终端时的情况下。
            ****************************************************/		
            if(m_last_ip == ntohl(pArphdr->ulSenderPaddr))
            {
                m_gaprp_count++;
                if(m_gaprp_count<3)
                {
                    LOG2( LOG_DEBUG1, LOGNO(ARP,ERR_ARP_NORMAL), "Uplink GARP . not forward to WAN, (IP:0x%X,count:%x)",ntohl(pArphdr->ulSenderPaddr),m_gaprp_count);
                    return;
                }
                else
                {
                    m_gaprp_count  =0;
                }
            }
            pComMsg->SetDstTid(M_TID_EB);
            pComMsg->SetSrcTid(M_TID_ARP);
            //	pComMsg->SetDirection(DIR_TO_WAN);//wangwenhua mask 20081120
            //设置MsgID;
            pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
            LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "Uplink GARP . forward to WAN, (IP:0x%X)",ntohl(pArphdr->ulSenderPaddr));
            m_last_ip = ntohl(pArphdr->ulSenderPaddr);
            m_gaprp_count = 0; //wangwenhua add 20101011
            CComEntity::PostEntityMessage( pComMsg );
            // LOG(LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "Receive ARP probe request. Discard packet.");
            return;
        }
        //check src and dst if from the same CPE,if yes,do nothing
        ArpILEntry *senderarp = GetArpIplstByIp(ntohl(pArphdr->ulSenderPaddr));
        if ( (NULL != senderarp) && (senderarp->ulEid == pDstARPEntry->ulEid) )
        {
            LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "Source IP and Dest IP locate in the same LAN. need not ARP proxy.");
            IncreaseDirTrafficMeasureByOne(ARP_FROM_AI,ARP_UNKNOW);
            return;
        }
        
        
        pArphdr->usOp = htons(M_ARP_REPLY);
        memcpy(pArphdr->aucDestHaddr, pArphdr->aucSenderHaddr,M_MAC_ADDRLEN);
        UINT32 tmpip = htonl(pArphdr->ulDestPaddr);
        pArphdr->ulDestPaddr = pArphdr->ulSenderPaddr;
        
        memcpy(pArphdr->aucSenderHaddr,pDstARPEntry->aucMAC,M_MAC_ADDRLEN); 
        pArphdr->ulSenderPaddr = tmpip;
        
        memcpy(pEtherPkt->aucDstMAC, pEtherPkt->aucSrcMAC,M_MAC_ADDRLEN);
        memcpy(pEtherPkt->aucSrcMAC, pDstARPEntry->aucMAC,M_MAC_ADDRLEN);
        
       
        LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy send an ARP reply(IP:0x%X) to AI.", ntohl(pArphdr->ulSenderPaddr));
        pComMsg->SetDstTid(M_TID_EB);
        pComMsg->SetSrcTid(M_TID_ARP);
        pComMsg->SetDirection(DIR_TO_AI);
        //设置MsgID;
        pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
        CComEntity::PostEntityMessage( pComMsg );
        
        IncreaseDirTrafficMeasureByOne(ARP_FROM_AI,ARP_TO_AI);
        return;
               
    }    
    
}
void CTaskARP::proArpProxyReq_TDR(CComMessage *pComMsg, EtherHdr *pEtherPkt, ArpHdr *pArphdr)
{
    UINT32 eid=0;
    CComMessage *pComMsg_Copy;
    unsigned short datalen = pComMsg->GetDataLength(); 
    
    LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy is enabled.");
    ArpILEntry *pDstARPEntry = GetArpIplstByIp(ntohl(pArphdr->ulDestPaddr));
    if ( NULL == pDstARPEntry ) 
    {
        LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy cant find ARP entry for IP.");
        LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "[from TDR]ARP proxy cant find ARP entry for IP(:0x%X), forward packet to WAN.",ntohl(pArphdr->ulDestPaddr));        
        pComMsg->SetDstTid(M_TID_EB);
        pComMsg->SetSrcTid(M_TID_ARP);
        pComMsg->SetDirection(DIR_TO_WAN);
        //设置MsgID;
        pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
        CComEntity::PostEntityMessage( pComMsg );
        IncreaseDirTrafficMeasureByOne(ARP_FROM_TDR,ARP_TO_WAN);
        return;
    
    }
    else 
    {
        LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy find ARP entry for IP(:0x%X).", ntohl(pArphdr->ulDestPaddr));       
        pArphdr->usOp = htons(M_ARP_REPLY);
        memcpy(pArphdr->aucDestHaddr, pArphdr->aucSenderHaddr,M_MAC_ADDRLEN);
        UINT32 tmpip = htonl(pArphdr->ulDestPaddr);
        pArphdr->ulDestPaddr = pArphdr->ulSenderPaddr;
        
        memcpy(pArphdr->aucSenderHaddr,pDstARPEntry->aucMAC,M_MAC_ADDRLEN); 
        pArphdr->ulSenderPaddr = tmpip;
        
        memcpy(pEtherPkt->aucDstMAC, pEtherPkt->aucSrcMAC,M_MAC_ADDRLEN);
        memcpy(pEtherPkt->aucSrcMAC, pDstARPEntry->aucMAC,M_MAC_ADDRLEN);
        
        
        
        LOG1( LOG_DEBUG2, LOGNO(ARP,ERR_ARP_NORMAL), "ARP proxy send an ARP reply(IP:0x%X) to TUNNEL.", ntohl(pArphdr->ulSenderPaddr));
        pComMsg->SetDstTid(M_TID_EB);
        pComMsg->SetSrcTid(M_TID_ARP);
        pComMsg->SetDirection(DIR_TO_TDR);
        DATA_assert((0 != pComMsg->GetBtsAddr()) && (0 != pComMsg->GetBtsPort()));
        //设置MsgID;
        pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );
        CComEntity::PostEntityMessage( pComMsg );
        
        IncreaseDirTrafficMeasureByOne(ARP_FROM_TDR,ARP_TO_TDR);
        return;
    
    }    
    
}
//set ARP config
bool CTaskARP::proArpConfig(CComMessage *pComMsg)
{
    LOG( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "->proArpConfig" );
    CMessage cmsg(pComMsg);
    ARPConfigReq arpcnfgmsg(cmsg); 
    SetEgressArpEn(arpcnfgmsg.GetEgressProxEn()?true:false);
    SetIngressArpEn(arpcnfgmsg.GetIngressProxEn()?true:false);

    ARPConfigRsp msgrsp;
    if ( !msgrsp.CreateMessage(*this) ) {
        return false;
    }
    msgrsp.SetResult( ERR_SUCCESS );
    msgrsp.SetXid(arpcnfgmsg.GetXid()); 
    msgrsp.SetDstTid( arpcnfgmsg.GetSrcTid() );
    if ( !msgrsp.Post() ) {
        LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_SYS_ERR), "->proArpConfig:Post Config Response failed." );
        msgrsp.DeleteMessage();
        return false;
    }
    return true;
}
//Add ArpIp, flag: 0=no rcpe, 1=rcpe
bool CTaskARP::AddILEntry( UINT32 eid, const UINT8* pmac, UINT32 ip,UINT8 flag )
{
    LOG2( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "Add ARP Entry(EID:0x%x, IP:0x%x, mac...)", eid, ip );
    if ( NULL == pmac ) {
        LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_PARAMETER), "Add ARP Entry fail, MAC address error." );
        return false;
    }
    ArpILEntry *rectarpip=GetArpIplstByIp(ip);
    if ( NULL == rectarpip ) {
        UINT16 idx=GetFreeArpIdxFromLst();
        if ( M_DATA_INDEX_ERR == idx ) {//ARP is full
            LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_CB_USEDUP), "ARP Table is full, add ARP entry fail." );
            return false;
        }
	 semTake (m_SemId, WAIT_FOREVER); //wangwenhua add protect for arp list
        rectarpip = &m_ArpIplstTb[idx];
        memcpy(rectarpip->aucMAC,pmac,M_MAC_ADDRLEN);
        rectarpip->ulEid = eid;
        rectarpip->ulIpAddr = ip;
	rectarpip->flag = flag;
	rectarpip->Active_flag = 0;//wangwenhua add 2012-6-20  只要是增加的都设置为active状态
        BPtreeAdd(ip,idx);
	 semGive (m_SemId);	
        return true;
    }
    else {//checking Mac&Eid eq or not is nessary?
        memcpy(rectarpip->aucMAC,pmac,M_MAC_ADDRLEN);
        rectarpip->ulEid = eid;
        rectarpip->ulIpAddr = ip;
	rectarpip->flag = flag;
	rectarpip->Active_flag = 0;//wangwenhua add 2012-6-20  只要是增加的都设置为active状态
        return true;
    }
}

#include "L3DataMacAddress.h"
//flag 默认为0，如果为1，则不删除ARP表，将其状态置为inactive状态
bool CTaskARP::DelILEntry(UINT32 ip, const UINT8 *pMac,UINT8 flag)
{
    LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "Delete ARP Entry(IP:0x%x)", ip );
    ArpILEntry *rectarpip=GetArpIplstByIp(ip);
    if ( NULL == rectarpip ) {
        return true;
    }
    else {
        //checking Mac&Eid eq or not is nessary?
        CMac delMac(pMac);
        CMac entryMac(rectarpip->aucMAC);
        if(delMac == entryMac){
	    
	     if(flag==1)
	     	{
	     	    LOG1( LOG_SEVERE, LOGNO(ARP,ERR_ARP_NORMAL), "Not DelILEntry(IP:0x%x) to inactive ", ip );
	     	    rectarpip->Active_flag = 1;//将标志设置为inactive状态	     	    
	     	}
	     	else
	     	{
              semTake (m_SemId, WAIT_FOREVER);
              memset(rectarpip,0,sizeof(ArpILEntry));
              UINT16 idx = BPtreeFind(ip);
              BPtreeDel(ip);
              InsertFreeArpLst(idx);
              semGive (m_SemId);
	     	}
	      	
        }
        else
            LOG1( LOG_DEBUG3, LOGNO(ARP,ERR_ARP_NORMAL), "Cant delete ARP Entry(IP:0x%x) when IP-MAC not match", ip );

        return true;
    }

}

void CTaskARP::InitFreeArpLst()
{
    m_listFreeArp.clear();
    for ( UINT16 usIdx = 0; usIdx < MAX_ARP_NODE; usIdx++ ) {
        m_listFreeArp.push_back( usIdx );
    }

}
void CTaskARP::InsertFreeArpLst(UINT16 usidx)
{
    if ( usidx < MAX_ARP_NODE ) {
        m_listFreeArp.push_back( usidx );
    }
    else {
        DATA_assert( 0 );
    }

}
UINT16 CTaskARP::GetFreeArpIdxFromLst()
{
    if ( true == m_listFreeArp.empty() ) {
		g_arp_no_freelist++;
        LOG( LOG_WARN,LOGNO(ARP,ERR_ARP_CB_USEDUP) , "->No Free Forwarding Entries!" );
        return M_DATA_INDEX_ERR;
    }

    UINT16 usIdx = *m_listFreeArp.begin();
    m_listFreeArp.pop_front();

    if ( MAX_ARP_NODE <= usIdx ) {
        //下标错误
        g_arp_no_freelist++;
        LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_CB_INDEX_ERR), "->Err! Free Forwarding Entry Index Err. " );
        return M_DATA_INDEX_ERR;
    }
	g_arp_no_freelist = 0;
    return usIdx;

}

bool CTaskARP::BPtreeAdd(UINT32 ip, UINT16 idx)
{
    pair<map<UINT32, UINT16>::iterator, bool> stPair;

    stPair = m_Arptree.insert( ValType( ip, idx ) );
    return stPair.second;

}
bool CTaskARP::BPtreeDel(UINT32 ip)
{
    map<UINT32, UINT16>::iterator it;

    if ( ( it = m_Arptree.find( ip ) ) != m_Arptree.end() ) {
        //find, and erase;
        m_Arptree.erase( it );
    }
    //not find
    return true;
}
UINT16 CTaskARP::BPtreeFind(UINT32 ip)
{
    map<UINT32, UINT16>::iterator it = m_Arptree.find( ip );

    if ( it != m_Arptree.end() ) {
        return it->second;
    }
    return M_DATA_INDEX_ERR;
}
ArpILEntry* CTaskARP::GetArpIplstByIp(UINT32 ip)
{
    UINT16 usIdx = BPtreeFind(ip);
    if ( M_DATA_INDEX_ERR == usIdx ) {
        LOG1( LOG_DEBUG, LOGNO(ARP,ERR_ARP_CB_INDEX_ERR) , "Cant find ARP entry with IP[0x%X].", ip );
        return NULL;
    }
    return &( m_ArpIplstTb[ usIdx ] );

}


bool CTaskARP::sendG_ARP(const UINT8 *mac, UINT32 ip)
{
    if (NULL == mac)
        {
        return false;
        }

    CComMessage *pComMsg = new(this, sizeof(EtherHdr) + sizeof(ArpHdr))CComMessage;
    if (NULL == pComMsg)
        {
        LOG(LOG_SEVERE, LOGNO(ARP, ERR_ARP_SYS_ERR) , "tARP new ComMessage fail, ARP probe failed." );
        return false;
        }

    pComMsg->SetDstTid(M_TID_EB);
    pComMsg->SetSrcTid(M_TID_ARP);
    pComMsg->SetDirection(DIR_TO_WAN);
////设置MsgID;
    pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );

////填充Ether header.
    EtherHdr *pEtherPkt = (EtherHdr*) ( pComMsg->GetDataPtr() );
    for (UINT8 idx = 0; idx < M_MAC_ADDRLEN; ++idx)
        {
        pEtherPkt->aucDstMAC[idx] = 0xFF;
        }
    memcpy(pEtherPkt->aucSrcMAC, mac,  M_MAC_ADDRLEN);
    pEtherPkt->usProto = htons(M_ETHER_TYPE_ARP);
////填充ARP.
    ArpHdr *pArphdr = (ArpHdr*) ( (UINT8*)pEtherPkt + sizeof( EtherHdr ) );
    pArphdr->usHtype    = htons(0x1);     //Ethernet.
    pArphdr->usPtype    = 0x800;   //IP
    pArphdr->ucHlen     = 0x6;
    pArphdr->ucPlen     = 0x4;
    pArphdr->usOp       = htons(M_ARP_REQUEST);
    memcpy(pArphdr->aucSenderHaddr, mac, M_MAC_ADDRLEN);
    pArphdr->ulSenderPaddr  = htonl(ip);
    //memcpy(pArphdr->aucDestHaddr, mac, M_MAC_ADDRLEN);
    memset(pArphdr->aucDestHaddr, 0, M_MAC_ADDRLEN);//lijinan 20121103
    pArphdr->ulDestPaddr    = htonl(ip);
	//pArphdr->ulDestPaddr    = 0 ;//lijinan 20121103

    if (false == CComEntity::PostEntityMessage( pComMsg ))
        {
        LOG2(LOG_SEVERE, LOGNO(ARP, ERR_ARP_SYS_ERR) , "send ARP probe[IP:%x, MAC:%s] fail.", ip, (int)mac );
        pComMsg->Destroy();
        return false;
        }
    LOG2(LOG_DEBUG2, LOGNO(ARP, ERR_ARP_NORMAL) , "send ARP probe IP:%x, MAC:%s.", ip, (int)mac );
    return true;
}

void CTaskARP::sendAllGARP()
{
  LOG(LOG_SEVERE, LOGNO(ARP, ERR_ARP_NORMAL) , "send All ARP to Wan for change Wcpe");
      if ( true == m_Arptree.empty() ) 
   {
 		return;
    }
    else 
 {
         map<UINT32, UINT16>::iterator it = m_Arptree.begin();
        UINT32 count=0;
        while ( m_Arptree.end() != it ) 
	{
            if ( MAX_ARP_NODE <= it->second ) 
	    {
                it++;
                continue;
            }
            ArpILEntry *rectarp=&m_ArpIplstTb[it->second];

         sendG_ARP(rectarp->aucMAC, rectarp->ulIpAddr);

        LOG2(LOG_DEBUG1, LOGNO(ARP, ERR_ARP_NORMAL) , "send ARP probe IP:%x, MAC:%s.", rectarp->ulIpAddr, (int)rectarp->aucMAC );
            count++;
            it++;
        }
    }
}
   #ifdef WBBU_CODE


   CTimer* CTaskARP::sys_Createtimer(UINT16 MsgId, UINT8 IsPeriod, UINT32 TimerPeriod)
   {
              CComMessage *pComMsg = new(this, 2)CComMessage;
	    if (NULL == pComMsg)
	        return NULL;
	 //  printf("MsgId:%x\n",MsgId);
	    pComMsg->SetDstTid(M_TID_ARP);
	    pComMsg->SetSrcTid(M_TID_ARP);
	    pComMsg->SetMessageId(MsgId);
	    return new CTimer(IsPeriod, TimerPeriod, pComMsg);
   }
    void CTaskARP::GateWayIPsend_arp_2GateWay()
   {
       //  UINT8 trunkBtsMac[M_MAC_ADDRLEN] = { 0x00, 0x18, 0x12, 0xff, 0xff, 0xff };
	  
	UINT32 trunkip = dwIP;
    	CComMessage *pComMsg = new(this, sizeof(EtherHdr) + sizeof(ArpHdr))CComMessage;
    if (NULL == pComMsg)
    {
        LOG(LOG_SEVERE, LOGNO(ARP, ERR_ARP_SYS_ERR) , "tARP new ComMessage fail, ARP probe failed." );
        return ;
    }

    pComMsg->SetDstTid(M_TID_EB);
    pComMsg->SetSrcTid(M_TID_ARP);
	pComMsg->SetIpType( M_PROTOCOL_TYPE_TCP );
	
    	pComMsg->SetDirection(DIR_TO_WAN);
	
////设置MsgID;
    pComMsg->SetMessageId( MSGID_TRAFFIC_FORWARD );

////填充Ether header.
    EtherHdr *pEtherPkt = (EtherHdr*) ( pComMsg->GetDataPtr() );
    for (UINT8 idx = 0; idx < M_MAC_ADDRLEN; ++idx)
    {
        pEtherPkt->aucDstMAC[idx] = 0xFF;
    }
    
    memcpy(pEtherPkt->aucSrcMAC, g_Mac,  M_MAC_ADDRLEN);
    pEtherPkt->usProto = htons(M_ETHER_TYPE_ARP);
////填充ARP.
    ArpHdr *pArphdr = (ArpHdr*) ( (UINT8*)pEtherPkt + sizeof( EtherHdr ) );
    pArphdr->usHtype    = htons(0x1);     //Ethernet.
    pArphdr->usPtype    = 0x800;   //IP
    pArphdr->ucHlen     = 0x6;
    pArphdr->ucPlen     = 0x4;
    pArphdr->usOp       = htons(M_ARP_REQUEST);
    memcpy(pArphdr->aucSenderHaddr, (UINT8*)g_Mac, M_MAC_ADDRLEN);
    pArphdr->ulSenderPaddr  = htonl(trunkip);
    memset(pArphdr->aucDestHaddr, 0, M_MAC_ADDRLEN);
    pArphdr->ulDestPaddr    = htonl(GateWayIP);

    if (false == CComEntity::PostEntityMessage( pComMsg ))
    {
       
        pComMsg->Destroy();
        return ;
    }
   
    return ;
   }
    #endif
/*============================================================
MEMBER FUNCTION:
    ARPShow

DESCRIPTION:
    用于Tornado Shell上调用执行

ARGUMENTS:
    NULL

RETURN VALUE:
    void

SIDE EFFECTS:
    none
==============================================================*/
extern "C" void ARPShow()
{
    CTaskARP *taskARP = CTaskARP::GetInstance();
    taskARP->showStatus();
}
#ifdef RCPE_SWITCH
/*============================================================
MEMBER FUNCTION:
    CTaskARP::trunkSendMsg

DESCRIPTION:
    封装创建及发送消息

ARGUMENTS:
	TID tid, 目标tid
	UINT16 usMsgID, 
	UINT8* pd, 发送的数据地址
	UINT32 ulLen, 发送的数据长度
	DIRECTION dir, 发送到网口或则空口
	UINT32 ulEID，目的终端对应的eid
	
RETURN VALUE:
	true:成功，false:失败

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskARP::trunkSendMsg( TID tid, UINT16 usMsgID, UINT8* pd, UINT32 ulLen, DIRECTION dir, UINT32 ulEID, UINT16 vlantag)
{
    CComMessage *RspMsg = new ( this, ulLen ) CComMessage;
    if( RspMsg != NULL )
    {
        RspMsg->SetDstTid( tid );
        RspMsg->SetMessageId( usMsgID );
		RspMsg->SetEID(ulEID);
        RspMsg->SetSrcTid( this->GetEntityId());
		RspMsg->SetIpType( M_PROTOCOL_TYPE_TCP );
		RspMsg->SetDirection( dir );
        RspMsg->SetVlanID(vlantag);
		if( 0 != ulLen )
        	memcpy( (UINT8*)RspMsg->GetDataPtr(), pd, ulLen );

		
        if( ! CComEntity :: PostEntityMessage( RspMsg ) )
        {
            RspMsg->Destroy();
			return false;
        }
    }
    return true;
}
#endif
void   print_rf()
{
      printf("rf status:\n");
      printf("type:%d\n",g_rf_openation.type);
      printf("Close_RF_Time_Len:%d\n",g_rf_openation.Close_RF_Time_Len);
      printf("Open_RF_Time_Len:%d\n",g_rf_openation.Open_RF_Time_Len);
      
      printf("IP1:%x,valid:%d,alarm:%d\n",g_rf_openation.GateWayIP1,g_rf_openation.GateWay1_valid,g_rf_openation.GateWay1_alarm);	
     printf("GateWay1_MAC:%x,%x,%x,%x,%x,%x\n",g_rf_openation.GateWay1_MAC[0],g_rf_openation.GateWay1_MAC[1],g_rf_openation.GateWay1_MAC[2],g_rf_openation.GateWay1_MAC[3],g_rf_openation.GateWay1_MAC[4],g_rf_openation.GateWay1_MAC[5]);
      printf("IP2:%x,valid:%d,alarm:%d\n",g_rf_openation.GateWayIP2,g_rf_openation.GateWay2_valid,g_rf_openation.GateWay2_alarm);	
	printf("GateWay2_MAC:%x,%x,%x,%x,%x,%x\n",g_rf_openation.GateWay2_MAC[0],g_rf_openation.GateWay2_MAC[1],g_rf_openation.GateWay2_MAC[2],g_rf_openation.GateWay2_MAC[3],g_rf_openation.GateWay2_MAC[4],g_rf_openation.GateWay2_MAC[5]);
	printf("Gateway1_bad_time:%d\n",g_rf_openation.Gateway1_bad_time);
	printf("Gateway2_bad_time:%d\n",g_rf_openation.Gateway2_bad_time);
	printf("Voice_Bad_Time:%d\n",g_rf_openation.Voice_Bad_Time);
	printf("GateWay1_UP:%x,GateWay1_down:%x\n",g_rf_openation.GateWay1_UP,g_rf_openation.GateWay1_down);
	printf("GateWay2_UP:%x,GateWay2_down:%x\n",g_rf_openation.GateWay2_UP,g_rf_openation.GateWay2_down);
	printf("state:%d\n",g_rf_openation.state);
	printf("g_Close_RF_flag:%d\n ",g_Close_RF_flag);
	if(sagStatusFlag)
	{
      		 printf("sagStatusFlag ok\n ");
	}
	else
	{
	       printf("sagStatusFlag err\n ");
	}
	
	

	
	 
}
 void CTaskARP::doNetWork()
 {
         unsigned char flag1 =0;
	 unsigned char flag2 = 0;
	 unsigned char flag3= 0;
	 unsigned char flag4 =0;
	 unsigned char flag5 =0;
	 unsigned char flag6 =0;
	 unsigned char flag7 =0;
	 char pString[20];
	  struct in_addr iaddr;
        while ( true )
        {
              flag1 =0;
		flag2 = 0;
		flag3= 0;
		flag4 =0;
		flag5 =0;
		flag6 =0;
		flag7= 0;
              if(( g_rf_openation.type==0)||(g_rf_openation.type>4))
   		{
   	   //不处理
   	                if(g_Close_RF_flag==1)//wangwenhua add 2012-4-5  
   	                {
   	                      	RF_config(false);
				       AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_Business_Fail_Close_RF,
		                       ALM_CLASS_MAJOR,
		                       STR_CloseRF_Warn1); 
			     	    g_Close_RF_flag = 0;
				   g_rf_openation.state =2;
				   LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), " open RF alarm");
   	                }
   		}
   		else
	   	{

		 	if(g_Close_RF_flag==0)//如果处于正常阶段
			{
		        if((g_rf_openation.GateWay1_valid)&&(g_rf_openation.GateWayIP1!=0))//网关1的处理
		        {
		              if((g_rf_openation.GateWay1_UP>0)&&(g_rf_openation.GateWay1_down==0))
		              {
		                    g_rf_openation.Gateway1_bad_time+=6;
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 has up packet no down packet:1");
				      if(g_rf_openation.Gateway1_bad_time>=g_rf_openation.Close_RF_Time_Len)
				      	{
		                         flag1 = 1;
					    LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 no down packet time over cfg time:2 ");
                                      //  g_rf_openation.Gateway1_bad_time =0;
					  }
		              }
				else
				{
				    g_rf_openation.Gateway1_bad_time = 0;
				  if(g_rf_openation.GateWay1_down>0)
				  {
				       if(g_rf_openation.GateWay1_alarm == 1)//将告警清除
				       {
				              AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay1_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay1_Fail,g_rf_openation.GateWayIP1); 
					g_rf_openation.GateWay1_alarm = 0;
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 alarm Recover:3");
				       }
				  }
				    
				}
				g_rf_openation.GateWay1_UP = 0;	  
				g_rf_openation.GateWay1_down = 0;
		        }
			else
			{
			    flag1 = 2;//表示无效
			    g_rf_openation.Gateway1_bad_time =0;
			}

			
			 if((g_rf_openation.GateWay2_valid)&&(g_rf_openation.GateWayIP2!=0))//网关2的处理
		        {
		              if((g_rf_openation.GateWay2_UP>0)&&(g_rf_openation.GateWay2_down==0))
		              {
		                   g_rf_openation.Gateway2_bad_time+=6;
				    LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 has up packet no down packet:4");
				      if(g_rf_openation.Gateway2_bad_time>=g_rf_openation.Close_RF_Time_Len)
				      	{
		                         flag2 =1;
					   LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 no down packet time over cfg time:5");
                                     //   g_rf_openation.Gateway2_bad_time = 0;
					  }
		                  
		              }
				else
				{
				     g_rf_openation.Gateway2_bad_time = 0;
				     	  if(g_rf_openation.GateWay2_down>0)
					  {
					       if(g_rf_openation.GateWay2_alarm ==1)//将告警清除
					       {
					              AlarmReport(ALM_FLAG_CLEAR,
			                       ALM_ENT_L3PPC, 
			                       ALM_ENT_INDEX0, 
			                       ALM_ID_GateWay2_Fail,
			                       ALM_CLASS_MAJOR,
			                       STR_GateWay2_Fail,g_rf_openation.GateWayIP2); 
							g_rf_openation.GateWay2_alarm = 0;
							LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 alarm Recover:6");
					       }
					  }
				}
				g_rf_openation.GateWay2_UP = 0;	  
				g_rf_openation.GateWay2_down = 0;
		        }
			else
			{
			    flag2 =2;
			    g_rf_openation.Gateway2_bad_time =0;
			}


			  if(!sagStatusFlag) //sag的处理
			     {
			     	
			     	    g_rf_openation.Voice_Bad_Time+=6;
				   if(g_rf_openation.Voice_Bad_Time>=g_rf_openation.Close_RF_Time_Len)
				   {
				   	   flag6=1; 
					//   g_rf_openation.Voice_Bad_Time =0;//wangwenhua add 2012-3-23
					   LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "Voice  alarm over Time :21");
				   }
				    LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "Voice  alarm false :22");
				   
			     }
			     else
			     	{
			     	      g_rf_openation.Voice_Bad_Time = 0;
			     	}

				 
			if((flag1==0)&&(flag2==1))//网关1正常，网关2不正常，产生告警，不关RF
			{
			    //ping 网关2，如果不通，则告警
		        	iaddr.s_addr = g_rf_openation.GateWayIP2;

		           	inet_ntoa_b (iaddr, pString);
				if(ping(pString,1,0)==OK)
				{
				   g_rf_openation.Gateway2_bad_time = 0;
				}
				else
				{
				    //产生告警
				    if(g_rf_openation.GateWay2_alarm==0)
				    	{
					      AlarmReport(ALM_FLAG_SET,
			                       ALM_ENT_L3PPC, 
			                       ALM_ENT_INDEX0, 
			                       ALM_ID_GateWay2_Fail,
			                       ALM_CLASS_MAJOR,
			                       STR_GateWay2_Fail,g_rf_openation.GateWayIP2); 
				    	}
					g_rf_openation.GateWay2_alarm = 1;
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 alarm:7 ");
				}
			}
			else if((flag1==1) &&(flag2==0))//网关1不正常，网关2正常，产生告警，不关RF
			{
			//ping 网关1 ，如果不通，则告警
			    	iaddr.s_addr = g_rf_openation.GateWayIP1;
		  
		           	inet_ntoa_b (iaddr, pString);
				if(ping(pString,1,0)==OK)
				{
				    g_rf_openation.Gateway1_bad_time =0;
				}
				else//产生告警
				{
				       //产生告警
				         if(g_rf_openation.GateWay1_alarm==0)
				         	{
				      AlarmReport(ALM_FLAG_SET,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay1_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay1_Fail,g_rf_openation.GateWayIP1); 
					}
					   g_rf_openation.GateWay1_alarm = 1;
					   LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 alarm :8");
				}
			}
			else if((flag1==1) &&(flag2==1))//网关1不正常，网关2不正常，产生告警，关RF
			{
			   //ping网关1和2，如果都不通，则置flag3
			   	iaddr.s_addr = g_rf_openation.GateWayIP1;
			       inet_ntoa_b (iaddr, pString);
			   if(ping(pString,1,0)==ERROR)
			     {
			          if( g_rf_openation.GateWay1_alarm==0)
			          	{
			              AlarmReport(ALM_FLAG_SET,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay1_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay1_Fail,g_rf_openation.GateWayIP1); 
			          	}
					 g_rf_openation.GateWay1_alarm = 1;
			        iaddr.s_addr = g_rf_openation.GateWayIP2;
				  inet_ntoa_b (iaddr, pString);
				  LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 alarm:9");
			         if(ping(pString,1,0)==ERROR)
			         {
			           	 if(g_rf_openation.GateWay2_alarm==0)
			           	 {
					AlarmReport(ALM_FLAG_SET,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay2_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay2_Fail,g_rf_openation.GateWayIP2); 
			           	 }
					g_rf_openation.GateWay2_alarm = 1;
					flag3 =1;
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 alarm :10");
			         }
					 else
					 	{
					 	    g_rf_openation.Gateway2_bad_time = 0;
					 	}
			     }
			   else
			   	{
			   	g_rf_openation.Gateway1_bad_time = 0;
			   	    iaddr.s_addr = g_rf_openation.GateWayIP2;
				  inet_ntoa_b (iaddr, pString);
				//  LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 alarm:9");
			         if(ping(pString,1,0)==ERROR)
			         {
			           	 if(g_rf_openation.GateWay2_alarm==0)
			           	 	{
							AlarmReport(ALM_FLAG_SET,
				                       ALM_ENT_L3PPC, 
				                       ALM_ENT_INDEX0, 
				                       ALM_ID_GateWay2_Fail,
				                       ALM_CLASS_MAJOR,
				                       STR_GateWay2_Fail,g_rf_openation.GateWayIP2); 
			           	 	}
					g_rf_openation.GateWay2_alarm = 1;
					
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 alarm :23");
			         }
				  else
				  	{
				  	    g_rf_openation.Gateway2_bad_time = 0;
				  	}
			   	}
			   
			}
			else if((flag1==1) &&(flag2==2))//网关1不正常，网关2没有产生告警，关FR
			{
			//ping 网关1，如果不通，则置flag
			    	iaddr.s_addr = g_rf_openation.GateWayIP1;
		  
		           	inet_ntoa_b (iaddr, pString);
				if(ping(pString,1,0)==OK)
				{
				    g_rf_openation.Gateway1_bad_time = 0;
				}
				else
				{
				      if(g_rf_openation.GateWay1_alarm==0)
				      	{
				      	AlarmReport(ALM_FLAG_SET,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay1_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay1_Fail,g_rf_openation.GateWayIP1); 
				      	}
					g_rf_openation.GateWay1_alarm = 1;
		     			flag3 =1;
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 alarm:11 ");
				}
			}
			else if((flag1==2)&&(flag2==1))//网关1没有，网关2不正常，产生告警，关RF
			{
			   //ping 网关2 
			    iaddr.s_addr = g_rf_openation.GateWayIP2;
		  
		           inet_ntoa_b (iaddr, pString);
			    if(ping(pString,1,0)==OK)
			   {
			       g_rf_openation.Gateway2_bad_time = 0;
			   }
			   else
			   {
			    if(g_rf_openation.GateWay2_alarm==0)
			    	{
			   	AlarmReport(ALM_FLAG_SET,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay2_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay2_Fail,g_rf_openation.GateWayIP2); 
			    	}
				 g_rf_openation.GateWay2_alarm = 1;
			     flag3 =1;
				 LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 alarm:12 ");
			   }
			}
			if(g_rf_openation.type==1)//2//)	启用数据传输断关射频功能；
			{
			     if(flag3==1)
			     {
			     	    //关闭RF
			     	    	AlarmReport(ALM_FLAG_SET,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_Business_Fail_Close_RF,
		                       ALM_CLASS_MAJOR,
		                       STR_CloseRF_Warn1); 
			     	    RF_config(true);
			     	    g_Close_RF_flag =1;
				  g_rf_openation.state =1;
				  LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "data close RF alarm :13");
			     }
			}
			
			else if(g_rf_openation.type==2)//3/////)	启用语音传输断关射频功能
			{
			     if(flag6==1)
			     	{
			     	   //关闭RF
			     	    //关闭RF
			     	
				     	    	AlarmReport(ALM_FLAG_SET,
			                       ALM_ENT_L3PPC, 
			                       ALM_ENT_INDEX0, 
			                       ALM_ID_Business_Fail_Close_RF,
			                       ALM_CLASS_MAJOR,
			                       STR_CloseRF_Warn2); 
					     	    RF_config(true);
					     	   g_Close_RF_flag = 1;
						  g_rf_openation.state =1;
						  LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "voice close RF alarm:14 ");
				   
			     	}
			}
		
			else if(g_rf_openation.type==3)//4//)	启用数据/语音同时断关射频功能；
			{
			      if((flag3==1)&&(flag6==1))
			      	{
			      	    RF_config(true);
				    	AlarmReport(ALM_FLAG_SET,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_Business_Fail_Close_RF,
		                       ALM_CLASS_MAJOR,
		                       STR_CloseRF_Warn3); 	
			      	    g_Close_RF_flag = 1;
				   g_rf_openation.state =1;
				   LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "voice and data   close RF alarm:15");
			      	}
			}
		      	else if(g_rf_openation.type==4)//4//)	5)	启用数据语音之一断关射频功能。
			{
			        if((flag3==1)||(flag6==1))
			      	{
			      	 RF_config(true);
				  	AlarmReport(ALM_FLAG_SET,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_Business_Fail_Close_RF,
		                       ALM_CLASS_MAJOR,
		                       STR_CloseRF_Warn4); 
			      	    g_Close_RF_flag = 1;
				   g_rf_openation.state =1;
				   LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "voice or data   close RF alarm:16");
			      	}
			}
		}
	else//已经关RF了的话
		{
		       if((g_rf_openation.GateWay1_valid)&&((g_rf_openation.GateWayIP1!=0)))
		       {
		              if(g_rf_openation.GateWay1_down>0)
		              {
		                 //网关1恢复
		                 flag4 = 1;
				      if(g_rf_openation.GateWay1_alarm == 1)//将告警清除
				       {
				              AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay1_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay1_Fail,g_rf_openation.GateWayIP1); 
					g_rf_openation.GateWay1_alarm = 0;
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 alarm Recover:17");
				       }
					g_rf_openation.Gateway1_bad_time =0;
		              }
				else
				{
				   //ping 网关1
				    	iaddr.s_addr = g_rf_openation.GateWayIP1;
		  
		               	inet_ntoa_b (iaddr, pString);
					if(ping(pString,1,0)==OK)
					{
					        if(g_rf_openation.GateWay1_alarm == 1)//将告警清除
					       {
					              AlarmReport(ALM_FLAG_CLEAR,
			                       ALM_ENT_L3PPC, 
			                       ALM_ENT_INDEX0, 
			                       ALM_ID_GateWay1_Fail,
			                       ALM_CLASS_MAJOR,
			                       STR_GateWay1_Fail,g_rf_openation.GateWayIP1); 
						g_rf_openation.GateWay1_alarm = 0;
						LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway1 alarm Recover:18");
					       }
					   flag4 = 1;
					   g_rf_openation.Gateway1_bad_time =0;
					}
					else
						{
						    flag4 = 2;
						}
				}

				
		       }
			else
			{
				flag4 = 0;
			      g_rf_openation.GateWay1_down = 0;
				g_rf_openation.GateWay1_UP = 0;
			}
			  	
			if((g_rf_openation.GateWay2_valid)&&((g_rf_openation.GateWayIP2!=0)))
			{
			          if(g_rf_openation.GateWay2_down>0)
		              {
		                 //网关2恢复
		                       if(g_rf_openation.GateWay2_alarm == 1)//将告警清除
				       {
				              AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay2_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay2_Fail,g_rf_openation.GateWayIP2); 
					g_rf_openation.GateWay2_alarm = 0;
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 alarm Recover:19");
				       }
		                 flag5= 1;
				   g_rf_openation.Gateway2_bad_time = 0;
		              }
				else
				{
				    //ping 网关2
				    	iaddr.s_addr = g_rf_openation.GateWayIP2;
		  
		               	inet_ntoa_b (iaddr, pString);
					if(ping(pString,1,0)==OK)
					{
					 if(g_rf_openation.GateWay2_alarm == 1)//将告警清除
				       {
				              AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_GateWay2_Fail,
		                       ALM_CLASS_MAJOR,
		                       STR_GateWay2_Fail,g_rf_openation.GateWayIP2); 
					g_rf_openation.GateWay2_alarm = 0;
					LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "gateway2 alarm Recover:20");
				       }
					   flag5 =1;
					    g_rf_openation.Gateway2_bad_time = 0;
					}
					else
						{
						    flag5= 2;
						}
				}
			}
			else
			{
				 flag5 = 0;
			      g_rf_openation.GateWay2_down = 0;
				g_rf_openation.GateWay2_UP = 0;
			}
			//决定是否开RF
			if(sagStatusFlag)
			{
			    g_rf_openation.Voice_Bad_Time =0;
			}
			if((flag4==0)&&(flag5==0))
			{
			     flag7= 1;
			}
			
			if(g_rf_openation.type==1)//2//)	启用数据传输断关射频功能；
			{
			     if((flag4==1)||(flag5==1)||(flag7==1))
			     {
			     	    //打开RF
			     	     RF_config(false);
				       	AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_Business_Fail_Close_RF,
		                       ALM_CLASS_MAJOR,
		                       STR_CloseRF_Warn1); 
			     	    g_Close_RF_flag = 0;
				   g_rf_openation.state =2;
				   LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "data open RF alarm");
			     }
			}
			else if(g_rf_openation.type==2)//3/////)	启用语音传输断关射频功能
			{
			     if(sagStatusFlag)
			     	{
			     	   //打开RF
			     	    RF_config(false);
					   AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_Business_Fail_Close_RF,
		                       ALM_CLASS_MAJOR,
		                       STR_CloseRF_Warn2); 
			     	   g_Close_RF_flag = 0;
				  g_rf_openation.state =2;
				  LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "voice open RF alarm");
			     	}
			}
			else if(g_rf_openation.type==3)//4//)	启用数据/语音同时断关射频功能；
			{
			      if(((flag4==1)||(flag5==1))||(sagStatusFlag)||(flag7==1))//只要有一个状态OK即可
			      	{
			      	   //打开RF
			      	    RF_config(false);
				   AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_Business_Fail_Close_RF,
		                       ALM_CLASS_MAJOR,
		                       STR_CloseRF_Warn3); 
			      	   g_Close_RF_flag = 0;
				   g_rf_openation.state =2;
				   LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "data and voice open RF alarm");
			      	}
			}
		      	else if(g_rf_openation.type==4)//4//)	5)	启用数据语音之一断关射频功能。
			{
			        if(((flag4==1)||(flag5==1)||(flag7==1))&&(sagStatusFlag))//都恢复才打开RF
			      	{
			      	  //打开RF
			      	   RF_config(false);
				   AlarmReport(ALM_FLAG_CLEAR,
		                       ALM_ENT_L3PPC, 
		                       ALM_ENT_INDEX0, 
		                       ALM_ID_Business_Fail_Close_RF,
		                       ALM_CLASS_MAJOR,
		                       STR_CloseRF_Warn4); 
				   g_rf_openation.state =2;
			      	  g_Close_RF_flag = 0;
				LOG( LOG_WARN, LOGNO(ARP,ERR_ARP_UNEXPECTED_MSGID), "data or voice open RF alarm");
			      	}
			}
			  
		          
		}
	}
             taskDelay(6*100);
        }
 }
 STATUS CTaskARP::RunNetWorksUp(CTaskARP *pTask)//wangwenhua add 2012-3-5
 {
 	    ( (CTaskARP*)pTask )->doNetWork();
    return 0;
 }
extern "C" int getMacFromSTD(char*);
void delArp(UINT32 ulIp)
{
    printf("\r\nPlease Enter the MAC Address[format:xx-xx-xx-xx-xx-xx]");
    UINT8 strMAC[7] = {0};
    getMacFromSTD( (char*)strMAC );
    CTaskARP::GetInstance()->DelILEntry(ulIp, strMAC);
}

 
