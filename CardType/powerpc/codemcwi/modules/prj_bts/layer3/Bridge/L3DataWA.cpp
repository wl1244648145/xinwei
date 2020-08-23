/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataEB.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
*
 *
 *---------------------------------------------------------------------------*/

//禁用Winsock 1
#define _WINSOCKAPI_

#include <string.h>
#ifndef __WIN32_SIM__
//VxWorks.
#include <intLib.h >
#include <logLib.h>
#endif
#include <taskLib.h>
#include "Object.h"
#include "biztask.h"
#include "MsgQueue.h"
#include "Message.h"
#include "SFIDdef.h"

#include "taskDef.h"
#include "ErrorCodeDef.h"
#include "L3DataCommon.h"
#include "L3DataFTCheckVLAN.h"
#include "L3DataMsgId.h"
#include "L3OAMMessageId.h"
#include "L3OAMCommonRsp.h"
#include "L3DataMACAddress.h"
#ifdef WBBU_CODE
#include "L3dataTypes.h"
#endif
#include "L3DataWANIF.h"


#ifdef __WIN32_SIM__
//Win32:
#include <ws2tcpip.h>
#else
//VxWorks:
#endif

#ifdef UNITEST
//包含桩函数
#include "L3DataStub.h"
#endif
#include "L3DataAssert.h"

extern "C" {
typedef void (*pfEBFreeCallBack) (UINT32 param);
#ifndef WBBU_CODE
typedef void (*pfWANIFRxCallBack)( M_BLK_ID    pMblk,UINT32 VLANID,UINT32 towho);
BOOL   mv643xxRecvMsgFromWANIF(char *,UINT16,pfEBFreeCallBack,UINT32,UINT8);
#else
typedef void (*pfWANIFRxCallBack)( char* pdataptr,UINT32 dataLen,UINT32 toWho,UINT32 hasVlan);
BOOL   mv643xxRecvMsgFromWANIF(char * buf, UINT16 len, pfEBFreeCallBack ComMsgFreeFunc, UINT32 param,unsigned char flag_type);
#endif
//mv643xxRecvMsgFromWANIF(char * buf, UINT16 len, pfEBFreeCallBack ComMsgFreeFunc, UINT32 param)
void   Drv_Reclaim_WAN(void *pCluster);
void   Drv_RegisterWANIF(pfWANIFRxCallBack);
void CsiMonitorDeadlock(UINT32 tid, UINT32 maxBlockedTime); 
void CsiEnableStackTrace(UINT32 tid);
STATUS  GetBtsMac(unsigned char *pMac);
#ifdef WBBU_CODE
 UINT16 ui_checksum_BBU
(
unsigned short *           pAddr,                  /* pointer to buffer  */
int                 len                     /* length of buffer   */
);
#endif
}

extern UINT32  WorkingWcpeEid ;
extern UINT32 m_cpe_to_myBTS_Eid;
//任务实例指针的初始化
CTWANIF*   CTWANIF::  s_ptaskWANIF = NULL;
LOCAL unsigned int countwan0  = 0;
LOCAL unsigned int countwan1 = 0;
LOCAL unsigned int countwan2 = 0;
LOCAL unsigned int countwan3 = 0;
LOCAL unsigned int countwan4 = 0;
LOCAL unsigned int countwan5 = 0;
LOCAL  unsigned int countwan6 =  0;
LOCAL  unsigned int countwan7 =  0;
#ifdef WBBU_CODE
LOCAL  unsigned int countwan8 =  0;
LOCAL  unsigned int countwan9 =  0;

LOCAL  unsigned int countwan10 =  0;
LOCAL  unsigned int countwan11 =  0;
extern "C" void printwanif()
{
    printf("wanif(0-5):%d,%d,%d,%d,%d,%d\n",countwan0,countwan1,countwan2,countwan3,countwan4,countwan5);
     printf("wanif(6-11):%d,%d,%d,%d,%d,%d\n",countwan6,countwan7,countwan8,countwan9,countwan10,countwan11);
}
extern "C" void clearwanif()
{
	countwan0  = 0;
	countwan1 = 0;
	 countwan2 = 0;
	 countwan3 = 0;
	countwan4 = 0;
	countwan5 = 0;
	 countwan6 =  0;
	 countwan7 =  0;
	 countwan8 = 0;
	 countwan9 = 0;
	 countwan10 = 0;
	 countwan11 = 0;
}
#else
void printwanif()
{
    logMsg("wanif:%x,%x,%x,%x,%x,%x\n",countwan0,countwan1,countwan2,countwan3,countwan4,countwan5);
}
void clearwanif()
{
	countwan0  = 0;
	countwan1 = 0;
	 countwan2 = 0;
	 countwan3 = 0;
	countwan4 = 0;
	countwan5 = 0;
	 countwan6 =  0;
	 countwan7 =  0;
}
#endif
/*============================================================
MEMBER FUNCTION:
    CTBridge::ProcessComMessage

DESCRIPTION:
    Bridge任务消息处理函数

ARGUMENTS:
    *pComMsg: 消息

RETURN VALUE:
    bool:true or false,FrameWork根据返回值决定是否做PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
bool CTWANIF::ProcessComMessage(CComMessage *pComMsg)
{
    UINT16 usMsgId = pComMsg->GetMessageId();
    switch ( usMsgId )
        {
        case MSGID_HIGH_PRIORITY_TRAFFIC:
        case MSGID_LOW_PRIORITY_TRAFFIC:
        case MSGID_REALTIME_TRAFFIC:
                // Ingress traffic process.
                Ingress( pComMsg );
            break;

        case MSGID_TRAFFIC_EGRESS:
                // Egress traffic process.
                Egress( pComMsg );
            break;

       

        default:
            //    LOG1( LOG_WARN, LOGNO( EB, EC_EB_UNEXPECTED_MSGID ), 
           //      "Bridge receive unexpected message[Id: 0x%x].", usMsgId );
            break;
        }

    //destroy message.
    pComMsg->Destroy();

#ifdef __WIN32_SIM__
    showStatus();
#endif
    return true;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::CTBridge

DESCRIPTION:
    CTBridge构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTWANIF::CTWANIF()
{
 //   LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "CTBridge::CTBridge()" );

#ifndef NDEBUG
    if ( !Construct( CObject::M_OID_WANIF ) )
        {
  //      LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "ERROR!!!CTBridge::CTBridge()% Construct failed." );
        }
#endif

    memset( m_szName, 0, sizeof( m_szName ) );
    memcpy( m_szName, M_TASK_WANIF_TASKNAME, strlen( M_TASK_WANIF_TASKNAME ) );
    m_uPriority     = M_TP_L3WANIF;
    m_uOptions      = M_TASK_WANIF_OPTION;
    m_uStackSize    = M_TASK_WANIF_STACKSIZE;

    m_iMsgQMax      = M_TASK_WANIF_MAXMSG;
    m_iMsgQOption   = M_TASK_WANIF_MSGOPTION;

    //初始化ComMessage链表
#ifndef __WIN32_SIM__
////VxWorks.
////lstInit( &m_listFreeNode );
////lstInit( &m_listComMessage );
    m_plistComMessage = NULL;
#endif


}



/*============================================================
MEMBER FUNCTION:
    CTBridge::~CTBridge

DESCRIPTION:
    CTBridge析够函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTWANIF::~CTWANIF()
{
//    LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "CTBridge::~CTBridge" );

#ifdef __WIN32_SIM__
    ::WSACleanup();
#endif
#ifndef NDEBUG
    if ( !Destruct( CObject::M_OID_WANIF ) )
        {
      //  LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "ERROR!!!CTBridge::~CTBridge failed." );
        }
#endif
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::GetInstance

DESCRIPTION:
    Get CTBridge Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTBridge* 

SIDE EFFECTS:
    none
==============================================================*/
CTWANIF* CTWANIF::GetInstance()
{
    if ( NULL == s_ptaskWANIF )
        {
        s_ptaskWANIF = new CTWANIF;
        }
    return s_ptaskWANIF;
}


/*============================================================
MEMBER FUNCTION:
    CTBridge::Initialize

DESCRIPTION:
    创建Message Queue;创建Socket

ARGUMENTS:
    NULL

RETURN VALUE:
    true of false 

SIDE EFFECTS:
    none
==============================================================*/
bool CTWANIF::Initialize()
{
  //  LOG( LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "CTBridge::Initialize" );
#ifdef __WIN32_SIM__
    ::WSAStartup( MAKEWORD( 2, 2 ), &m_wsaData );
#endif

    //create socket



#ifndef __WIN32_SIM__
    //VxWorks.
    if( false == InitComMessageList() )
        {
      //  LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "Fail to initialize EB message pool!" );
      //  ::close( m_EtherIpSocket );
        return false;
        }
#endif

    UINT8 ucInit = CBizTask::Initialize();
    if ( false == ucInit )
        {

        return false;
        }
      if(GetBtsMac(m_btsMac)!=OK)
      	{
      	     logMsg("GetBtsMac error\n",1,2,3,4,5,6);
	      
      	}
	 else
	 {
	     logMsg("bts mac :%x,%x,%x,%x,%x,%x\n",m_btsMac[0],m_btsMac[1],m_btsMac[2],m_btsMac[3],m_btsMac[4],m_btsMac[5]);
	 }


#if 1
    //配置完成以后才注册
    //向网卡注册报文接收函数
    Drv_RegisterWANIF( CTWANIF::RxDriverPacketCallBack );

#endif
    return true;
}



/*============================================================
MEMBER FUNCTION:
    CTBridge::DeallocateComMessage

DESCRIPTION:
    释放ComMessage内存

ARGUMENTS:
    *pComMsg

RETURN VALUE:
    bool: true,成功；false,失败

SIDE EFFECTS:
    none
==============================================================*/
bool CTWANIF::DeallocateComMessage(CComMessage *pComMsg)
{
//    DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge DeallocateComMessage()" );

    UINT32 ulFlag = pComMsg->GetFlag();
    if (M_CREATOR_DRV_WANIF == ulFlag) 
        {
        //驱动提供的缓存，调用驱动的释放函数
      //  ::Drv_Reclaim_WAN( pComMsg->GetBufferPtr() );
    //    pComMsg->DeleteBuffer();
            countwan5++;
		//printf("countwan5:%x\n",countwan5);
#ifdef __WIN32_SIM__
        //Windows.
        CComEntity::DeallocateComMessage( pComMsg );
#else
        //VxWorks.
        pComMsg->SetDstTid( M_TID_WANIF ); 
        pComMsg->SetSrcTid( M_TID_WANIF ); 
        pComMsg->SetMessageId( MSGID_TRAFFIC_EGRESS );
        pComMsg->SetFlag( 0 );
        pComMsg->SetEID( 0 );
    //    pComMsg->SetDataLength( 0 );
      //  pComMsg->SetDataPtr( NULL );
       // pComMsg->SetTimeStamp( 0xffffffff );
     //   pComMsg->SetDirection( 0xff );
      //  pComMsg->SetIpType( 0xff );
   //     pComMsg->SetBTS( 0 );
   //     pComMsg->SetUdpPtr( NULL );
   //     pComMsg->SetDhcpPtr( NULL );
 //       pComMsg->SetKeyMac( NULL );

        //空闲的ComMessage插入到链表头
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_plistComMessage);
        m_plistComMessage = pComMsg;
      //  --g_ulOccupiedCounter;
        ::intUnlock( intKey );
        ::taskUnlock();
#endif
        }
    else
        {
#ifdef WBBU_CODE
        countwan8++;
#endif
        CComEntity::DeallocateComMessage( pComMsg );
        }

    return true;
}



#ifndef __WIN32_SIM__
/*============================================================
MEMBER FUNCTION:
    CTBridge::InitComMessageList

DESCRIPTION:
    初始化ComMessage链表，初始化=2000个。
    这个链表只允许给网卡驱动使用

ARGUMENTS:
    NULL

RETURN VALUE:
    bool

SIDE EFFECTS:
    none
==============================================================*/
bool CTWANIF::InitComMessageList()
{
    for( UINT16 usIdx = 0; usIdx < M_MAX_LIST_SIZE_WANIF; ++usIdx )
        {
#if 0
        stComMessageNode *pComMsgNode = (stComMessageNode *)new stComMessageNode;
        if ( NULL == pComMsgNode )
            {
            return false;
            }
#endif
        CComMessage      *pComMsg     = new ( this, 0x604 )CComMessage;
        if ( NULL == pComMsg )
            {
            //delete pComMsgNode;
            return false;
            }

        //pComMsgNode->pstComMsg = pComMsg;
        //只是网卡驱动给EB任务发送消息
        pComMsg->SetDstTid( M_TID_WANIF ); 
        pComMsg->SetSrcTid( M_TID_WANIF ); 
        pComMsg->SetMessageId( MSGID_TRAFFIC_EGRESS );
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_plistComMessage);
        m_plistComMessage = pComMsg;
        ::intUnlock( intKey );
        ::taskUnlock();
        }

    return true;
}
#endif
/*****************************
according mac judge packet to cpe or to bts ip stack;
0-bts;
1-cpe;
2=broadcast packet
*********************************/
extern unsigned char  Local_Sac_Mac_Addr[5][6] ;
 char  CTWANIF::getMacType(CComMessage* pComMessge)
{
//   char type = 0xff;
   EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
     if ((pEtherHdr->aucDstMAC[0] == m_btsMac[0])&& (pEtherHdr->aucDstMAC[1] == m_btsMac[1] ) &&(pEtherHdr->aucDstMAC[2] == m_btsMac[2]) &&
	 ( pEtherHdr->aucDstMAC[3] == m_btsMac[3]) &&
	   (pEtherHdr->aucDstMAC[4] == m_btsMac[4]) &&(pEtherHdr->aucDstMAC[5] == m_btsMac[5]))
     	{
     	     return 0;
     	}
	 else
	 {
	     for(int i=0; i<5; i++)
	     {
	         if ((pEtherHdr->aucDstMAC[0] == Local_Sac_Mac_Addr[i][0])&& (pEtherHdr->aucDstMAC[1] == Local_Sac_Mac_Addr[i][1] ) \
			 	&&(pEtherHdr->aucDstMAC[2] == Local_Sac_Mac_Addr[i][2]) &&( pEtherHdr->aucDstMAC[3] == Local_Sac_Mac_Addr[i][3]) \
	                    &&(pEtherHdr->aucDstMAC[4] == Local_Sac_Mac_Addr[i][4]) &&(pEtherHdr->aucDstMAC[5] == Local_Sac_Mac_Addr[i][5]))
          	 {
          	     return 2;
          	 }
	     }
	 }
	 return 1;
    // return type;
}
/*============================================================
MEMBER FUNCTION:
    CTBridge::EBFreeMsgCallBack

DESCRIPTION:
    Driver释放ComMessage的回调函数

ARGUMENTS:
    UINT32 :ComMessage ptr.

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void CTWANIF::WANIFFreeMsgCallBack (UINT32 param)
{
    if((void*)param!=NULL)
    ((CComMessage*)param)->Destroy();
    return;
}

/*============================================================
MEMBER FUNCTION:
    CTBridge::SendToWAN

DESCRIPTION:
    将数据发送给IP stack 进行处理

ARGUMENTS:
    *pComMsg:

RETURN VALUE:
    bool: 发送成功/失败

SIDE EFFECTS:
    none
==============================================================*/
bool CTWANIF::SendToWAN(CComMessage *pComMsg, UINT16 grpID,unsigned char flag)
{


    //增加计数
   // pComMsg->AddRef();
    //发送WAN
#ifndef WBBU_CODE
    return mv643xxRecvMsgFromWANIF
        (
        (char*)pComMsg->GetDataPtr(),       //Data to send
        (UINT16)pComMsg->GetDataLength()+6,   //Data length
        CTWANIF::WANIFFreeMsgCallBack,        //function.
        (UINT32)pComMsg    ,flag                 //ComMessage ptr.
        );
#else
    return mv643xxRecvMsgFromWANIF
        (
        (char*)pComMsg->GetDataPtr(),       //Data to send
        (UINT16)pComMsg->GetDataLength()/*+6*/,   //Data length
        CTWANIF::WANIFFreeMsgCallBack,        //function.
        /*(UINT32)pComMsg*/ NULL   ,flag                 //ComMessage ptr.
        );
#endif
}
/*============================================================
the msg from wcpe 's uplink
analyze the packet's type according mac addr;
if the packet is broadcast,then send ip stack and eb module both;
if the packet is bts's packet then send to ip stack;
if other packet ,send to eb module to deal;
it simulates the packet from wan
==============================================================*/
void CTWANIF::Ingress(CComMessage *pComMsg)
{
    //DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge Ingress()" );
     countwan1++;
    if (pComMsg->GetDataLength() >= 1600)
        {
      //  DATA_log(pComMsg, LOG_CRITICAL, 0, "invalid uplink Data length %d", pComMsg->GetDataLength());
#ifdef WBBU_CODE
      countwan11++;
#endif
        return; 
        }

    //增加MAC filter功能
    bool  iszero = false;
    CMac DstMac( GetDstMac( pComMsg ) );
   if(true==DstMac.IsZero())//wangwenhua add 20080926
   {
#ifdef WBBU_CODE
      countwan9++;
#endif
       iszero = true;
	 return;
	
   }
   if ( true == DstMac.IsBroadCast() )
   {
        //
     //   DATA_log(pComMsg, LOG_WARN, LOGNO( EB, EC_EB_MSG_EXCEPTION ), "Err! source MAC address is a broad/multicast address, discard packet.");
       SendToWAN(pComMsg,0,0);
	SendToWAN(pComMsg,0,2);
     pComMsg->SetDstTid(M_TID_EB);
     pComMsg->SetMessageId(MSGID_TRAFFIC_EGRESS);
     CComEntity::PostEntityMessage( pComMsg ) ;
    countwan2++;
        return;
   }
  char type = getMacType(pComMsg);
  if(type == 0)// to bts ip stack;
  {
      SendToWAN(pComMsg,0,1);
	  countwan3++;
  }
  else if(type==1)// to cpe 
  {
     pComMsg->SetDstTid(M_TID_EB);
     pComMsg->SetMessageId(MSGID_TRAFFIC_EGRESS);
     CComEntity::PostEntityMessage( pComMsg ) ;
	 countwan4++;
  }
  else if(type == 2)
  {
        SendToWAN(pComMsg,0,2);
		 countwan7++;
  }
  //按照如下流程进行处理
 // 1、广播包如何处理，送给协议栈和EB模块各一份；
 // 2、bts's packet send ip stack to deal;
 // 3    other packet send eb to deal;
 
        
}



/*============================================================
msg from ip stack,send to eb to deal;
the best way to add a message type to eb ;
to this msg,eb to call sendtowan(this function in eb must be modified)
==============================================================*/
void CTWANIF::Egress(CComMessage *pComMsg)
{
  //  DATA_log(pComMsg, LOG_DEBUG3, LOGNO( EB, EC_EB_NORMAL ), "->Bridge Egress()" );

    //add by xiaoweifang to support vlan{{
 
       countwan6++;
//    UINT8    *pData = (UINT8*)( pComMsg->GetDataPtr() );
    //to send eb to deal;
   pComMsg->SetDstTid(M_TID_EB);
   pComMsg->SetMessageId(MSGID_TRAFFIC_IPSTACK);
   CComEntity::PostEntityMessage( pComMsg ) ;
   //printf("countwan6:%x\n",countwan6);
    return;
}



#if 0
/*============================================================
MEMBER FUNCTION:
    CTBridge::GetRelayMsgID

DESCRIPTION:
    根据SFID，返回相应的MsgID;

ARGUMENTS:
    ucSFID:

RETURN VALUE:
    UINT16: MsgID

SIDE EFFECTS:
    none
==============================================================*/
UINT16 CTWANIF::GetRelayMsgID(const CComMessage *pComMsg)
{
    if ( pComMsg->GetDataLength() < 100 )
        {
        //数据比较小的包，优先级提高
        return MSGID_HIGH_PRIORITY_TRAFFIC;
        }

    switch( GetTosSFID( pComMsg )  )
        {
        case M_SFID_HIGH:
            //高优先级
            return MSGID_HIGH_PRIORITY_TRAFFIC;                

        case M_SFID_LOW:
            //低优先级
            return MSGID_LOW_PRIORITY_TRAFFIC;                

        case M_SFID_REALTIME:
            //实时
            return MSGID_REALTIME_TRAFFIC;                

        default:
            //其他...返回低优先级
            return MSGID_LOW_PRIORITY_TRAFFIC;                
        }
}
#endif


#ifndef __WIN32_SIM__
/*============================================================
MEMBER FUNCTION:
    CTBridge::GetComMessage

DESCRIPTION:
    从ComMessage链表中获取ComMessage.

ARGUMENTS:
    void

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
CComMessage* CTWANIF::GetComMessage()
{
    //Get first Node.
#if 0
    stComMessageNode *pNode = (stComMessageNode *)lstGet( &m_listComMessage );
    if ( NULL == pNode )
        {
        ::intUnlock( intKey );
        return NULL;
        }
#endif
    //空闲表头插入到空闲表头链表
    ::taskLock();
    UINT32 intKey = ::intLock();
    if (NULL == m_plistComMessage)
        {
        ::intUnlock( intKey );
        ::taskUnlock();
        return NULL;
        }
    //lstAdd( &m_listFreeNode, &( pNode->lstHdr ) );
    CComMessage *pComMsg = m_plistComMessage;
    m_plistComMessage    = m_plistComMessage->getNext();
//    ++g_ulOccupiedCounter;
    ::intUnlock( intKey );
    ::taskUnlock();
    //CComMessage *pComMsg = pNode->pstComMsg;
    return pComMsg;
}
#endif
#define MV_RX_BUF_SIZE  ( 0x604)

/*============================================================
MEMBER FUNCTION:
    CTBridge::RxDriverPacketCallBack

DESCRIPTION:
    接收网卡上送的报文

ARGUMENTS:
    char* : data to be sent.
    UINT16: data length
    char* : buffer ptr.

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
#ifndef WBBU_CODE
void CTWANIF::RxDriverPacketCallBack( M_BLK_ID   pMblk,UINT32 VLANID,UINT32 toWho)
{

    static CTWANIF *s_ptaskWANIF = CTWANIF::GetInstance();
   int  len;
   //int offsetlen = 0;
   int  i;
    CComMessage *pComMsg = s_ptaskWANIF->GetComMessage();

  
    if( NULL == pComMsg )
        {
        //释放RDR.
        //DATA_assert( 0 );
        logMsg("\r\nCTWANIF Message pool is used up", 0, 0, 0, 0, 0, 0);
       // ::Drv_Reclaim_WAN( pRxBuf );
        return;
        }
	 char *pdata = (char *)pComMsg->GetDataPtr();
  if ( 1 < VLANID )
    {
        char     *pDataPtr = NULL;
        VLAN_hdr *pVlan    = NULL;

	
	    len = netMblkToBufCopy (pMblk, pdata+4/*(char *)PHYS_TO_VIRT(pTDR->buf_ptr + 4)*/,
	                            (FUNCPTR)NULL);

	    /* don't send zero length pkts */

	    if (len == 0 )
		{
			return;
		}

		len += 4;	/*4: vlan tag&ID;*/

		if ( len> MV_RX_BUF_SIZE ) 
		{
			logMsg("mv643xxPktXmit with len too long %d\n", len, 0,0,0,0,0);
			return;
		}

        /*添加VLAN tag.*/
        pDataPtr = (char *)pdata;
        memcpy( pDataPtr, pDataPtr + 4, 12 );
        pVlan = (VLAN_hdr*)( pDataPtr + 12 );
        pVlan->usProto_vlan = htons( M_ETHER_TYPE_VLAN );
        pVlan->usVlanID = htons( VLANID );
        /*消息其他字段不用修改*/

    }
	else
	{
	
	    len = netMblkToBufCopy (pMblk,pdata,
	                            (FUNCPTR)NULL);

	    /* don't send zero length pkts */

	    if (len == 0 )
		{
			return;
		}

		if ( len> MV_RX_BUF_SIZE ) 
		{
			logMsg("mv643xxPktXmit with len too long %d\n", len, 0,0,0,0,0);
			return;
		}
		
		
	}
	/*************to avoid the packet length less than 64 being discared in the network*****************/
	if(len<64)
	{
	   // offsetlen = 64- len;
	    for(i = len; i< 64 ;i ++)
	    	{
	    	    pdata[i] = 0;
	    	}
		len = 64;
	}
	countwan0++;
	//printf("countwan0:%x\n",countwan0);
   // pComMsg->SetBuffer( pRxBuf, usDataLength + M_DEFAULT_RESERVED );
   // pComMsg->SetDataPtr( pRxData );
    pComMsg->SetDataLength( len );
    pComMsg->SetTimeStamp( 0 );
    pComMsg->SetFlag( M_CREATOR_DRV_WANIF );
    if(toWho==0)
      pComMsg->SetEID(WorkingWcpeEid);
    else
      pComMsg->SetEID(m_cpe_to_myBTS_Eid);
      
    if( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        //DATA_assert( 0 );
       // logMsg("\r\nDriver post packets to EB failed, EB pool usage[%d/%d].", g_ulOccupiedCounter, M_MAX_LIST_SIZE, 0, 0, 0, 0);
        //pComMsg->Destroy();
        s_ptaskWANIF->DeallocateComMessage( pComMsg );
        }

    return;
}
#else
/*********************************************************************
hasVlan -表示是否已经加了VLAN，对于有VLAN的包，驱动已经计算了checksum,不用重新计算。



********************************************************************/
void CTWANIF::RxDriverPacketCallBack( char* pdataptr,UINT32 dataLen,UINT32 toWho,UINT32 hasVlan)
{
    static CTWANIF *s_ptaskWANIF1 =NULL;

   int  len;
   //int offsetlen = 0;
   int  i;
   	s_ptaskWANIF1 = CTWANIF::GetInstance();


 //  countwan0++;
  //  return;
    CComMessage *pComMsg = s_ptaskWANIF1->GetComMessage();

  
    if(( NULL == pComMsg )||(pdataptr==NULL)||(dataLen==0))
        {
        //释放RDR.
        //DATA_assert( 0 );
        logMsg("\r\nCTWANIF Message pool is used up", 0, 0, 0, 0, 0, 0);
       // ::Drv_Reclaim_WAN( pRxBuf );
        return;
        }
	 char *pdata = (char *)pComMsg->GetDataPtr();
	 
	 memcpy(pdata,pdataptr,dataLen);
        len =dataLen;
	#if 1  //对于UDP和TCP包必须计算checksum,如果有VLAN的话，已经在驱动增加了UDP或者TCP的CheckSUM
	if (  hasVlan == 0 )
	 {
	      
	   if(pdata[23]==0x11)/*udp packet **/
	    {
	        pdata[40] = 0;
		 pdata[41] = 0;
	      	char ipbuf[20];
		UINT16 usUdpLen = *((short *)(pdata+14+20+4));
		memcpy(ipbuf,pdata+14,20);
		char *pIpHead = (char *)pdata+14; /*ip head*/
		IpHdr *pIp = (IpHdr *)(pdata+14);/*ip head*/
		UdpHdr*pUdpHead = (UdpHdr*)(pIpHead+20);/*udp head*/
		/* set udp pseudo head */
		pseudoHeadTEb* phead = &((pseudoPktTEb*)pIpHead)->phead;
		phead->srcIP = pIp->ulDstIp;
		phead->dstIP= pIp->ulSrcIp;
		phead->proto = 0x11;
		phead->len = usUdpLen /*+16*/ ;

					/* count checksum (plus pseudo head) */
		UINT16 sum = ui_checksum_BBU((UINT16*)phead, usUdpLen /*+16*/+ sizeof(pseudoHeadTEb));
		if(sum == 0)
			sum = 0xffff;
		pUdpHead->usCheckSum = sum;
		memcpy(pdata+14,ipbuf,20);
	     }
	      else if(pdata[23]==0x6)/**tcp packet***/
	      	{
	      	  pdata[50] = 0;
		 pdata[51] = 0;
	      	char ipbuf[20];
		UINT16 usTotalLen = *((short *)(pdata+16));/*ip header(20)+20 tcp header + data ***/
		memcpy(ipbuf,pdata+14,20);
		char *pIpHead = (char *)pdata+14; /*ip head*/
		IpHdr *pIp = (IpHdr *)(pdata+14);/*ip head*/
	/*	UdpHdr*pUdpHead = (UdpHdr*)(pIpHead+20);*//*udp head*/
		/* set udp pseudo head */
		pseudoHeadTEb* phead = &((pseudoPktTEb*)pIpHead)->phead;
		phead->srcIP = pIp->ulDstIp;
		phead->dstIP= pIp->ulSrcIp;
		phead->proto = 0x6;
		phead->len = usTotalLen-20 /*+16*/ ;

					/* count checksum (plus pseudo head) */
		UINT16 sum = ui_checksum_BBU((UINT16*)phead, (usTotalLen-20) /*+16*/+ sizeof(pseudoHeadTEb));
		if(sum == 0)
			sum = 0xffff;
		/*pUdpHead->usCheckSum = sum;*/
		pdata[50] = sum>>8;
		pdata[51] = sum;
		memcpy(pdata+14,ipbuf,20);
	      	}
	 }
	
	#endif
	/*************to avoid the packet length less than 64 being discared in the network*****************/
	if(len<64)
	{
	   // offsetlen = 64- len;
	    for(i = len; i< 64 ;i ++)
	    	{
	    	    pdata[i] = 0;
	    	}
		
		len = 64;
	}
	
	countwan0++;
	//printf("countwan0:%x\n",countwan0);
   // pComMsg->SetBuffer( pRxBuf, usDataLength + M_DEFAULT_RESERVED );
   // pComMsg->SetDataPtr( pRxData );
     pComMsg->SetDstTid( M_TID_WANIF ); 
     pComMsg->SetSrcTid( M_TID_WANIF ); 
     pComMsg->SetMessageId( MSGID_TRAFFIC_EGRESS );
    pComMsg->SetDataLength( len );
    pComMsg->SetTimeStamp( 0 );
    pComMsg->SetFlag( M_CREATOR_DRV_WANIF );
    if(toWho==0)
      pComMsg->SetEID(WorkingWcpeEid);
    else
      pComMsg->SetEID(m_cpe_to_myBTS_Eid);
      
    if( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
          countwan10++;
        //DATA_assert( 0 );
       // logMsg("\r\nDriver post packets to EB failed, EB pool usage[%d/%d].", g_ulOccupiedCounter, M_MAX_LIST_SIZE, 0, 0, 0, 0);
        //pComMsg->Destroy();
        s_ptaskWANIF1->DeallocateComMessage( pComMsg );
        }

    return;
}
extern "C"
void Twanif()
{
   static CTWANIF *s_ptaskWANIF1 =NULL;

//   int  len;
   //int offsetlen = 0;
 //  int  i;
   	s_ptaskWANIF1 = CTWANIF::GetInstance();
 //   CComMessage *pComMsg = s_ptaskWANIF1->GetComMessage();
   countwan0++;
}
extern "C" void SendToEtsc()
{

   char pdata[100];
  int i = 0;
  for( i = 0; i < 100; i++)
  {
      pdata[i] = i;
  }
	    mv643xxRecvMsgFromWANIF
        (
        pdata,       //Data to send
        100,   //Data length
        CTWANIF::WANIFFreeMsgCallBack,        //function.
        NULL    ,0                 //ComMessage ptr.
        );
}

#endif
