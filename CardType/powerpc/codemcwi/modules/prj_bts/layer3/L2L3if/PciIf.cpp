/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:  implementation of PCI Interface class
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   11/17/2005   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/

#include <intLib.h>

#include <logLib.h>
#include <taskLib.h>

#include "Object.h"
#include "biztask.h"
#include "MsgQueue.h"
#include "Message.h"
#include "SFIDdef.h"

#include "taskDef.h"

#include "PciIf.h"
#include "L3L2MessageId.h"
//#include ""
//#ifndef MEM_ALLOC
//#define MEM_ALLOC
//#endif

extern "C" {
typedef void (*pfL2L3FreeCallBack) (UINT32 param);
typedef void (*pfL2L3RxCallBack)(void*,char *, UINT16, char *,unsigned char);
BOOL   L2RecvMsgFromL3(char * buf, UINT16 len, pfL2L3FreeCallBack ComMsgFreeFunc, UINT32 param,UINT8 module);
BOOL L1RecvMsgFromL3(char * buf, UINT16 len, pfL2L3FreeCallBack ComMsgFreeFunc, UINT32 param,UINT8 module);
void   L2L3_Drv_Reclaim(void *);
void   L2L3_Drv_Register(pfL2L3RxCallBack);

void CsiMonitorDeadlock(UINT32 tid, UINT32 maxBlockedTime); 
void CsiEnableStackTrace(UINT32 tid);
 void send_2_wan(unsigned char *pdata,unsigned short len);

}

#define MSGID_TRAFFIC_INGRESS_L2L3  0x5555
#define MSGID_TRAFFIC_INGRESS_L2L3_CORE1  0xaaaa
/*----------------------------------------------------------------------------
 *  Global data definition
 *---------------------------------------------------------------------------*/

CTaskPciIf * CTaskPciIf::Instance=NULL;
CTaskPciIf *PciIfObj = NULL;

#ifdef M_TGT_WANIF
extern UINT32  WorkingWcpeEid;
extern UINT16   Wanif_Switch;
#endif
const TID CTaskPciIf::ProxyTIDs[PROXY_TID_NUM ] = { 
	                         M_TID_L2IF,
                           M_TID_L2MAIN,      // 101  
                           M_TID_L2BOOT,      // 102
                           M_TID_DIAG,    // 103
                           M_TID_VAC,	      // 106 CPE voice task
                           M_TID_DAC,    
                           M_TID_L2OAM,
                           M_TID_PCISIO,
                          // M_TID_WRRU
                        };
extern "C" void rebootL2();

L2L3IF_STATIC    l2l3_count,l2l3_count1;



#define M_MAX_LIST_SIZE             (40000)


#define M_TASK_L2L3_STACKSIZE     (40960+10240)
#define M_TASK_L2L3_MAXMSG        (20000)


#define M_TASK_L2L3_OPTION        ( VX_FP_TASK )
#define M_TASK_L2L3_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )


/*****************************************************************************
 *
 *   Method:     CTaskPciIf::GetInstance()
 *
 *   Description: The only interface to create a singleton object
 *
 *   Parameters:  None
 *
 *   Returns:  object of the class
 *
 *****************************************************************************
 */
CTaskPciIf* CTaskPciIf::GetInstance()
{
    if (NULL == Instance)
    {
        Instance = new CTaskPciIf;
    }
    return Instance;
}




/*****************************************************************************
 *
 *   Method:     CTaskPciIf::CTaskPciIf()
 *
 *   Description: Constructor, 
 *
 *   Parameters:  None
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
CTaskPciIf::CTaskPciIf()
{
	 LOG( LOG_DEBUG3, LOG_ERR_PCI_DEBUG_INFO, "CTaskPciIf::CTaskPciIf()" );
#ifndef NDEBUG
       if ( !Construct( CObject::M_OID_L2IF ) )
        {
        //LOG( LOG_SEVERE, LOGNO( EB, EC_EB_SYS_FAIL ), "ERROR!!!CTBridge::CTBridge()% Construct failed." );
        }
#endif
    strcpy(m_szName, "tL2If");
    m_uPriority   = M_TP_L2IF;
  
    


      m_uOptions      = M_TASK_L2L3_OPTION;
    m_uStackSize    = M_TASK_L2L3_STACKSIZE;

    m_iMsgQMax      = 60000/*M_TASK_L2L3_MAXMSG*/;
    m_iMsgQOption   = M_TASK_L2L3_MSGOPTION;
    
    CurrentTid = M_TID_L2IF;
  
   memset( m_aulDirTrafficMeasure, 0, sizeof( m_aulDirTrafficMeasure ) );


     m_plistComMessage = NULL;


}
CTaskPciIf::~CTaskPciIf()
{

	LOG(LOG_DEBUG3, LOG_ERR_PCI_DEBUG_INFO, "CTaskPciIf::~CTaskPciIf()");
#ifndef NDEBUG
	if (!Destruct(CObject::M_OID_L2IF))
	{
        	LOG(LOG_SEVERE, LOG_ERR_PCI_DEBUG_INFO, "ERROR!!!CTaskPciIf::~CTaskPciIf failed.");
	}
#endif
}

/*****************************************************************************
 *
 *   Method:     CTaskPciIf::Initialize()
 *
 *   Description: function to initialize the data structure used by the entity.
 *                Called by framework before mainloop
 *
 *   Parameters:  None
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
bool CTaskPciIf::Initialize()
{

     if( false == InitComMessageList() )
        {
        LOG( LOG_SEVERE, LOG_ERR_PCI_DEBUG_INFO , "Fail to initialize EB message pool!" );
 
        return false;
        }

   

    for (unsigned int i=0; i<SIZEOF(ProxyTIDs); i++)
    {
        CurrentTid = ProxyTIDs[i];
        RegisterEntity(false);
        LOG1(LOG_DEBUG, LOG_ERR_PCI_DEBUG_INFO, "Register proxy for task %d finished\n", CurrentTid);
    }

    



   L2L3_Drv_Register( CTaskPciIf::L2L3RxDriverPacketCallBack );


    UINT8 ucInit = CBizTask::Initialize();
    if ( false == ucInit )
    {
    	      delete m_pMsgQueue;  
  
           return false;
    }
  memset((char*)&l2l3_count,0,sizeof(L2L3IF_STATIC));
    return true;
}

bool CTaskPciIf::InitComMessageList()
{
    for( UINT16 usIdx = 0; usIdx < M_MAX_LIST_SIZE; ++usIdx )
    {
#ifndef  MEM_ALLOC
        CComMessage      *pComMsg     = new ( this, 0 )CComMessage;
#else

       CComMessage      *pComMsg     = new ( this, 1600 )CComMessage;
#endif
        if ( NULL == pComMsg )
            {
            //delete pComMsgNode;
            return false;
            }

        //pComMsgNode->pstComMsg = pComMsg;
        //只是网卡驱动给EB任务发送消息
        pComMsg->SetDstTid( M_TID_L2IF ); 
        pComMsg->SetSrcTid( M_TID_L2IF ); 
        pComMsg->SetMessageId( MSGID_TRAFFIC_INGRESS_L2L3);
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_plistComMessage);
        m_plistComMessage = pComMsg;
        #ifdef RELEASE_COM
           m_plistComMessage_bak[usIdx] = pComMsg;
      #endif
        ::intUnlock( intKey );
        ::taskUnlock();
     }
   for( UINT16 Idx = 0; Idx < M_TASK_L2L3_MAXMSG; ++Idx )
    {

        CComMessage      *pComMsg     = new ( this, 0 )CComMessage;
        if ( NULL == pComMsg )
        {
           
            return false;
        }

        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_listcomMsg);
        m_listcomMsg = pComMsg;
        ::intUnlock( intKey );
        ::taskUnlock();
     }

    return true;
}
/*============================================================
MEMBER FUNCTION:
    CTBridge::GetComMessage

DESCRIPTION:
    从ComMessage链表中获取ComMessage.

ARGUMENTS:
    0-从m_plistComMessage得到一个CComMessage;
    1-从m_listcomMsg得到一个CComMessage

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
CComMessage* CTaskPciIf::GetComMessage(UINT8 flag)
{
    //Get first Node.

    //空闲表头插入到空闲表头链表
     CComMessage *pComMsg;
    ::taskLock();
    UINT32 intKey = ::intLock();
    if(flag==0)
    	{
	    if (NULL == m_plistComMessage)
	        {
	        ::intUnlock( intKey );
	        ::taskUnlock();
	        return NULL;
	        }
	    //lstAdd( &m_listFreeNode, &( pNode->lstHdr ) );
	    pComMsg = m_plistComMessage;
	    m_plistComMessage    = m_plistComMessage->getNext();
    	}
	else
	{
	      if (NULL == m_listcomMsg)
	        {
	        ::intUnlock( intKey );
	        ::taskUnlock();
	        return NULL;
	        }
	    //lstAdd( &m_listFreeNode, &( pNode->lstHdr ) );
	   pComMsg = m_listcomMsg;
	    m_listcomMsg    = m_listcomMsg->getNext();
	}
    ::intUnlock( intKey );
    ::taskUnlock();
    //CComMessage *pComMsg = pNode->pstComMsg;
    return pComMsg;
}
void   CTaskPciIf::ReleaseComMsg()
{
 #ifdef RELEASE_COM
  for( UINT16 usIdx = 0; usIdx < M_MAX_LIST_SIZE; ++usIdx )
 	{
   	      CComMessage      *pComMsg  = m_plistComMessage_bak[usIdx] ;
   	      pComMsg->SetFlag(M_CREATOR_DRV_SW_NO_FRAG);
   	      DeallocateComMessage(pComMsg);
   	       
   	}
  #endif
}
extern "C" void ReleaseL2L3Com()
{
 #ifdef RELEASE_COM
    CTaskPciIf::GetInstance()->ReleaseComMsg();
 #endif
}
/*************************************************************

该函数供驱动调用，收到一个完整的Ethernet包，组成一个commessage;

如果不要组包的话，则直接发送到ingress函数进行处理，flag标志置为M_CREATOR_DRV_SW_NO_FRAG
否则需要在ingress模块中进行copy，将多个包组成一个commessage，标志置蜯_CREATOR_TASK_FRA

前期调试时认为不要组包处理
************************************************************/
unsigned char printFlag  = 0; 
void settestFlag(unsigned char flag)
{
    printFlag = flag;
}

unsigned int   L2_NO_ComMsg = 0;
void CTaskPciIf::L2L3RxDriverPacketCallBack(void *pmblk,char *pRxData, UINT16 usDataLength, char *pRxBuf,unsigned char flag)
{

// char *ptr;
 if(pmblk==NULL)
 {
 	      l2l3_count.count_l2_35++;
 }
   // usDataLength += 4;/*a driver error.*/
 if(flag==0)//cope core0 msg
 {
    static CTaskPciIf *pBridgeInstance = NULL;
    pBridgeInstance = CTaskPciIf::GetInstance();
    
   

    CComMessage *pComMsg = pBridgeInstance->GetComMessage(0);
   

    if(( NULL == pComMsg ))
        {
        //释放RDR.
        //DATA_assert( 0 );
      //  logMsg("\r\nL2L3 Message pool is used up", 0, 0, 0, 0, 0, 0);
       l2l3_count.count_l2_19++;
        ::L2L3_Drv_Reclaim( pmblk );
        L2_NO_ComMsg++;
        if(L2_NO_ComMsg==1000)
        {
                //此处复位基站
                l2l3_count.count_l2_39++;
                ReleaseL2L3Com();
        }
        return;
        }
   
      	    if((usDataLength<=16))
        {
        //释放RDR.
        //DATA_assert( 0 );
      //  logMsg("\r\nL2L3 Message pool is used up", 0, 0, 0, 0, 0, 0);
         l2l3_count.count_l2_33++;
        ::L2L3_Drv_Reclaim( pmblk );
        return;
        }
      	    L2_NO_ComMsg = 0;
     //CComMessage      *pComMsg     = new ( this, 0 )CComMessage;
   
     
    if(printFlag==1)
    	{
    	    if( l2l3_count.count_l2_0%1000==0)
    	    	{
    	         logMsg("data:%x,%x,%x,%x,%x,%x\n",pRxData[15],pRxData[16],pRxData[17],pRxData[18],pRxData[19],pRxData[20]);
    	         logMsg("data:%x,%x,%x,%x,%x,%x\n",pRxData[21],pRxData[22],pRxData[23],pRxData[24],pRxData[25],pRxData[26]);
    	         logMsg("data:%x,%x,%x,%x,%x,%x\n",pRxData[27],pRxData[28],pRxData[29],pRxData[30],pRxData[31],pRxData[32]);
    	    	}
    	}
    #if 0
   char  *pData =  new char[usDataLength +M_DEFAULT_RESERVED];
   if(pData==NULL)
   	{
   	    ::L2L3_Drv_Reclaim( pmblk );
   	    pComMsg->Destroy();
   	}
   else
   	{
   	    memcpy((pData+M_DEFAULT_RESERVED),pRxData,usDataLength);
   	     ::L2L3_Drv_Reclaim( pmblk );
   	}
   #endif
   #if 1
   #ifndef  MEM_ALLOC
   pComMsg->SetMblk( pmblk);
    pComMsg->SetBuffer( pRxBuf, usDataLength + M_DEFAULT_RESERVED );
    pComMsg->SetDataPtr( pRxData );
    pComMsg->SetDataLength( usDataLength );
    #else
    ptr =( char *) pComMsg->GetDataPtr();
    memcpy(ptr,pRxData,usDataLength);
    pComMsg->SetDataLength( usDataLength );
    ::L2L3_Drv_Reclaim( pmblk );
    #endif
    pComMsg->SetTimeStamp( 0 );
    #endif
    #if 0
     pComMsg->SetBuffer( pData, usDataLength + M_DEFAULT_RESERVED );
    pComMsg->SetDataPtr( pData+M_DEFAULT_RESERVED );
    pComMsg->SetDataLength( usDataLength );
    pComMsg->SetTimeStamp( 0 );
    #endif
    //如果是拆包的话，这个标志不置，在ingress处理后会将驱动分配的Mblk释放
    pComMsg->SetFlag( M_CREATOR_DRV_SW_NO_FRAG );
   // count_l2_0++;
    l2l3_count.count_l2_0++;
    if( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
          //count_l2_6++;
          l2l3_count.count_l2_1++;
           #ifndef  MEM_ALLOC
           ::L2L3_Drv_Reclaim( pComMsg->GetMblk());
           #endif
        //DATA_assert( 0 );
        //logMsg("\r\nDriver post packets to EB failed, EB pool usage[%d/%d].", g_ulOccupiedCounter, M_MAX_LIST_SIZE, 0, 0, 0, 0);
        //pComMsg->Destroy();
        pBridgeInstance->DeallocateComMessage( pComMsg );
        }
 }
 else if(flag == 1)
 {
 	 #ifdef DIAG_TOOL
 	   //直接发送给L3网口
 	  send_2_wan((unsigned char*)pRxData,usDataLength);
 	 #endif
 }
 else if(flag==2)//core1 来的消息
 {
     static CTaskPciIf *pBridgeInstance1 = NULL;
    pBridgeInstance1 = CTaskPciIf::GetInstance();
    
   

    CComMessage *pComMsg1 = pBridgeInstance1->GetComMessage(0);
   

    if(( NULL == pComMsg1 ))
        {
        //释放RDR.
        //DATA_assert( 0 );
      //  logMsg("\r\nL2L3 Message pool is used up", 0, 0, 0, 0, 0, 0);
       l2l3_count1.count_l2_19++;
        ::L2L3_Drv_Reclaim( pmblk );
        L2_NO_ComMsg++;
        if(L2_NO_ComMsg==1000)
        {
                //此处复位基站
                l2l3_count1.count_l2_39++;
                ReleaseL2L3Com();
        }
        return;
        }
   
      	    if((usDataLength<=16))
        {
        //释放RDR.
        //DATA_assert( 0 );
      //  logMsg("\r\nL2L3 Message pool is used up", 0, 0, 0, 0, 0, 0);
         l2l3_count1.count_l2_33++;
        ::L2L3_Drv_Reclaim( pmblk );
        return;
        }
      	    L2_NO_ComMsg = 0;
     //CComMessage      *pComMsg     = new ( this, 0 )CComMessage;
   
     
    if(printFlag==1)
    	{
    	    if( l2l3_count1.count_l2_0%1000==0)
    	    	{
    	         logMsg("data:%x,%x,%x,%x,%x,%x\n",pRxData[15],pRxData[16],pRxData[17],pRxData[18],pRxData[19],pRxData[20]);
    	         logMsg("data:%x,%x,%x,%x,%x,%x\n",pRxData[21],pRxData[22],pRxData[23],pRxData[24],pRxData[25],pRxData[26]);
    	         logMsg("data:%x,%x,%x,%x,%x,%x\n",pRxData[27],pRxData[28],pRxData[29],pRxData[30],pRxData[31],pRxData[32]);
    	    	}
    	}
    #if 0
   char  *pData =  new char[usDataLength +M_DEFAULT_RESERVED];
   if(pData==NULL)
   	{
   	    ::L2L3_Drv_Reclaim( pmblk );
   	    pComMsg->Destroy();
   	}
   else
   	{
   	    memcpy((pData+M_DEFAULT_RESERVED),pRxData,usDataLength);
   	     ::L2L3_Drv_Reclaim( pmblk );
   	}
   #endif
   #if 1
   #ifndef  MEM_ALLOC
   pComMsg1->SetMblk( pmblk);
    pComMsg1->SetBuffer( pRxBuf, usDataLength + M_DEFAULT_RESERVED );
    pComMsg1->SetDataPtr( pRxData );
    pComMsg1->SetDataLength( usDataLength );
    #else
    ptr =( char *) pComMsg1->GetDataPtr();
    memcpy(ptr,pRxData,usDataLength);
    pComMsg1->SetDataLength( usDataLength );
    ::L2L3_Drv_Reclaim( pmblk );
    #endif
    pComMsg1->SetMessageId(MSGID_TRAFFIC_INGRESS_L2L3_CORE1);
    pComMsg1->SetTimeStamp( 0 );
    #endif
    #if 0
     pComMsg->SetBuffer( pData, usDataLength + M_DEFAULT_RESERVED );
    pComMsg->SetDataPtr( pData+M_DEFAULT_RESERVED );
    pComMsg->SetDataLength( usDataLength );
    pComMsg->SetTimeStamp( 0 );
    #endif
    //如果是拆包的话，这个标志不置，在ingress处理后会将驱动分配的Mblk释放
    pComMsg1->SetFlag( M_CREATOR_DRV_SW_NO_FRAG );
   // count_l2_0++;
    l2l3_count1.count_l2_0++;
    if( false == CComEntity::PostEntityMessage( pComMsg1 ) )
        {
          //count_l2_6++;
          l2l3_count1.count_l2_1++;
           #ifndef  MEM_ALLOC
           ::L2L3_Drv_Reclaim( pComMsg1->GetMblk());
           #endif
        //DATA_assert( 0 );
        //logMsg("\r\nDriver post packets to EB failed, EB pool usage[%d/%d].", g_ulOccupiedCounter, M_MAX_LIST_SIZE, 0, 0, 0, 0);
        //pComMsg->Destroy();
        pBridgeInstance1->DeallocateComMessage( pComMsg1 );
        }
 	}
    return;
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
void CTaskPciIf::L2L3FreeMsgCallBack (UINT32 param)
{
    ((CComMessage*)param)->Destroy();
    l2l3_count.count_l2_12++;
    return;
}

/*****************************************************************************
 *
 *   Method:     CTaskPciIf::PostMessage()
 *
 *   Description: Interface function provided to other ComEntity objects to 
 *                pass ComMessage to peer CPU. 
 *
 *   Parameters:  None
 *
 *   Returns:  none

 这个函数有2个方向，对于上行包，则是发送到自己任务队列进行处理；
 对于下行包，则直接调用驱动的发送函数
 *
 *****************************************************************************
 */
bool CTaskPciIf::PostMessage(CComMessage* pcomMsg, SINT32 timeOut, bool isUrgent)
{

 // 上行包处理
     UINT8 module;
     UINT16 m_uMsgId = pcomMsg->GetMessageId();
     static int count = 0;
 //    static int  count1 =0;
      //pcomMsg->AddRef();
      switch(m_uMsgId)
 	{
 	   case MSGID_TRAFFIC_INGRESS_L2L3:
 	   
 	  //  count_l2_2++;
 	    l2l3_count.count_l2_2++;
 	   	 if(false==CBizTask::PostMessage(pcomMsg,timeOut,isUrgent))//调用基类的函数进行处理
 	   	 	{
 	   	 	      pcomMsg->Destroy();
 	   	 	      //DeallocateComMessage( pcomMsg );
 	   	 	      l2l3_count.count_l2_29++;
 	   	 	      return false;
 	   	 	}
 	   	break;
 	   	case MSGID_TRAFFIC_INGRESS_L2L3_CORE1:
 	  //  count_l2_2++;
 	    l2l3_count1.count_l2_2++;
 	   	 if(false==CBizTask::PostMessage(pcomMsg,timeOut,isUrgent))//调用基类的函数进行处理
 	   	 	{
 	   	 	      pcomMsg->Destroy();
 	   	 	      //DeallocateComMessage( pcomMsg );
 	   	 	      l2l3_count1.count_l2_29++;
 	   	 	      return false;
 	   	 	}
 	   	break;
 	  default:
 	  //	count_l2_3++;
 	  	      count++;
 	       l2l3_count.count_l2_10++;
 	  	  //     logMsg("CTaskPciIf::PostMessage:%d\n",count,1,2,3,4,5);
 	           L2L3_MSG_HEADER *header = (L2L3_MSG_HEADER*)(((UINT32) pcomMsg->GetDataPtr())- sizeof(L2L3_MSG_HEADER));
            header->DestTid = pcomMsg->GetDstTid();
            header->SrcTid = pcomMsg->GetSrcTid();
            header->UID = pcomMsg->GetUID();
            header->EID = pcomMsg->GetEID();
            header->MsgId = pcomMsg->GetMessageId();
            header->MsgLen = pcomMsg->GetDataLength();	//进行拆包处理，并发送给驱动进行处理
            if(header->DestTid == M_TID_VAC)
            	{
            	    module = 1;
            	}
            else
            	{
            	    module = pcomMsg->GetModule();
            	}
            if(module==0)
            	{
            	   l2l3_count.count_l2_30++;
            	}
            else if(module==1)
            	{
            	l2l3_count.count_l2_31++;
            	}
            else
            	{
            	l2l3_count.count_l2_32++;
            	}
            #if 0
            #define MSGID_HI_PRIORITY_UNICAST_OAM               (0x8000)
							#define MSGID_LOW_PRIORITY_UNICAST_OAM               (0x8001)
							#define MSGID_BROADCAST_OAM                         (0x8F00)
							#define MSGID_BROADCAST_TRAFFIC_TO_UT               (0x8f01)
							#define MSGID_HIGH_PRIORITY_TRAFFIC                 (0x8010)
							#define MSGID_LOW_PRIORITY_TRAFFIC                  (0x8011)
							#define MSGID_REALTIME_TRAFFIC                      (0x8012)
							#endif
						/*	if(header->MsgId==MSGID_HI_PRIORITY_UNICAST_OAM)||(header->MsgId==MSGID_HI_PRIORITY_UNICAST_OAM)
								||(header->MsgId==MSGID_HI_PRIORITY_UNICAST_OAM)||(header->MsgId==MSGID_HI_PRIORITY_UNICAST_OAM)
									||(header->MsgId==MSGID_HI_PRIORITY_UNICAST_OAM)(header->MsgId==MSGID_HI_PRIORITY_UNICAST_OAM)
								if(header->MsgId==MSGID_HI_PRIORITY_UNICAST_OAM)*/
 	          pcomMsg->AddRef();
    //发送WAN
    //    module = 0;
    if(module<2)
    	{
#if 1
    return L2RecvMsgFromL3
    (
        (char*)header/*pcomMsg->GetDataPtr()*/,       //Data to send
        (UINT16)pcomMsg->GetDataLength() +16,   //Data length
        CTaskPciIf::L2L3FreeMsgCallBack,        //function.
        (UINT32)pcomMsg  ,                   //ComMessage ptr.
        module
        );	
#endif
    	}
    else
    	{
    	         return L1RecvMsgFromL3
        (
        (char*)header/*pcomMsg->GetDataPtr()*/,       //Data to send
        (UINT16)pcomMsg->GetDataLength() +16,   //Data length
        CTaskPciIf::L2L3FreeMsgCallBack,        //function.
        (UINT32)pcomMsg  ,                   //ComMessage ptr.
        module
        );	
    	}
 	   break;
 	}

   return true;
}


/*****************************************************************************
 *
 *   Method:     CTaskPciIf::DeallocateComMessage()
 *
 *   Description: routine called by other ComEntity to free the Com Message
 *                created by PciIf. All the ComMessages from peer has the 
 *                flag field != 0xffffffff
 *
 *   Returns:  none
 *
 *****************************************************************************
 */
bool CTaskPciIf::DeallocateComMessage(CComMessage* pComMsg)
{
     
     //logMsg("DeallocateComMessage:%x\n",pComMsg->GetFlag(),1,2,3,4,5);
     #ifdef RELEASE_COM
     if(pComMsg==m_plistComMessage)
     	{
     	     l2l3_count.count_l2_38++;
     	      return true;
     	}
     #endif
   if(pComMsg->GetFlag()==M_CREATOR_DRV_SW_NO_FRAG)
   	{
      //  count_l2_5++;
   	     //驱动提供的缓存，调用驱动的释放函数
   	#ifndef  MEM_ALLOC
   	 
   	     if( pComMsg->GetMblk()!=NULL)
   	     	{
        		::L2L3_Drv_Reclaim( pComMsg->GetMblk());
        		 l2l3_count.count_l2_34++;
   	     	}
   	     else
   	     	{
   	     	     l2l3_count.count_l2_36++;
   	     	}
   	   //  ::L2L3_Drv_Reclaim( pComMsg->GetMblk());
   	     pComMsg->DeleteBuffer();
   	     pComMsg->SetDataPtr( NULL );
       
        #endif
        
        l2l3_count.count_l2_13++;

        //VxWorks.
        pComMsg->SetDstTid( M_TID_L2IF); 
        pComMsg->SetSrcTid( M_TID_L2IF ); 
        pComMsg->SetMessageId( MSGID_TRAFFIC_INGRESS_L2L3 );
        pComMsg->SetFlag( 0 );
        pComMsg->SetEID( 0 );
        pComMsg->SetDataLength( 0 );
       
        pComMsg->SetTimeStamp( 0xffffffff );
     
        pComMsg->SetMblk(NULL);/**wangwenhua add 20090325**/

        //空闲的ComMessage插入到链表头
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_plistComMessage);
        m_plistComMessage = pComMsg;
      
        ::intUnlock( intKey );
        ::taskUnlock();
   	}
   else if(pComMsg->GetFlag()==M_CREATOR_TASK_FRAG)
   {
               //空闲的ComMessage插入到链表头
          //这里增加记述
         // count_l2_7++;
          if( pComMsg->GetMblk()!=NULL)
   	    {
        		::L2L3_Drv_Reclaim( pComMsg->GetMblk());
        		 l2l3_count.count_l2_34++;
   	    }
		  
         l2l3_count.count_l2_11++;

     //   logMsg("hello call here\n",0,1,2,3,4,5);
        ::taskLock();
        UINT32 intKey = ::intLock();
        pComMsg->setNext(m_listcomMsg);
        m_listcomMsg = pComMsg;
	  UINT8*  Ptr = (UINT8*)pComMsg->GetBufferPtr();
        delete [] Ptr;
        Ptr = NULL;
	  pComMsg->DeleteBuffer();
	  pComMsg->SetFlag( 0 );
        pComMsg->SetEID( 0 );
        pComMsg->SetDataLength( 0 );
        pComMsg->SetDataPtr( NULL );
        pComMsg->SetTimeStamp( 0xffffffff );
        pComMsg->SetFlag(0);
        ::intUnlock( intKey );
        ::taskUnlock();
   }
   else
   {
            l2l3_count.count_l2_28++;
              return CComEntity::DeallocateComMessage(pComMsg);
              
   }
   return true;
}

/*************************************
这个函数主要是将从驱动收到包组成一个comMessage，然后再发送出去。

DSt_MAC(6B) SRC_MAC(6B)TYPE(2B)INFO(2B)L2L3HEADER(16B)+PayLOAD

      15 |    14|13  |12  |11  |10  |9  |8  |7  |6  |5  |4  |3  |2  |1  |0  |
      bit 0:0-表示第一包，1-表示后续包
      bit 1-5:当bit0为0时，1-5表示包的总数，否则表示包的序号
      bit 6-9:表示模块id这个只在发送给L2时有用，L3不用处理

*********************************************/

void CTaskPciIf::L2L3Ingress(CComMessage* pComsg)
{
//  EtherHdrL2L3 *pEtherEnd = (EtherHdrL2L3*)( pComsg->GetDataPtr() );
	UINT16 info = GetPacketInfo(pComsg);
	static UINT8 pack_count  = 0;
	static UINT8 last_seq = 0;
	static CComMessage *pMsg = NULL;
	static UINT8  rec_first_flag = 0;
	UINT16 datalen;
	UINT8* pPload ;//= (UINT8*)((UINT8*) pComsg->GetDataPtr() + 32);
	UINT8 current_seq;
  	l2l3_count.count_l2_26++;

	if((info&1)==0)//第一包
	{
	   if(rec_first_flag==0x55)/**需要释放**/
	   	{
	   	     DeallocateComMessage(pMsg);
	   	     rec_first_flag = 0xff;
	   	     l2l3_count.count_l2_20++;
	   	}
	   last_seq = 0;
	   L2L3_MSG_HEADER *pL2L3Header = (L2L3_MSG_HEADER*)((UINT8*)pComsg->GetDataPtr()+16);
	   pPload = (UINT8*)((UINT8*) pComsg->GetDataPtr() + 32);
	   pack_count =((info>>1)&0x1f);
	  // printf("info:%x,%d\n",info,pComsg->GetDataLength());
	   datalen = pComsg->GetDataLength();
	  if(datalen<=32)
	  	{
	  	     l2l3_count.count_l2_21++;
	  	    return;
	  	}
	   if(pack_count==1)//只有一包的话,则直接发送不需要组报进行处理
	   {
	      
	         pComsg->SetDstTid((TID)pL2L3Header->DestTid);
           pComsg->SetSrcTid((TID)pL2L3Header->SrcTid);
           pComsg->SetMessageId(pL2L3Header->MsgId);
           pComsg->SetUID(pL2L3Header->UID);
           pComsg->SetEID(pL2L3Header->EID);
		  			pComsg->SetDataLength(pL2L3Header->MsgLen);
		  			pComsg->SetDataPtr((void*)pPload);
		  #if 1
		  			if((pL2L3Header->DestTid!=M_TID_L2IF)&&
		  				(pL2L3Header->DestTid!=M_TID_L2MAIN)&&
		  				(pL2L3Header->DestTid!=M_TID_L2BOOT)&&
		  				(pL2L3Header->DestTid!=M_TID_DIAG)&&
		  				(pL2L3Header->DestTid!=M_TID_VAC)&&
		  				(pL2L3Header->DestTid!=M_TID_DAC)&&
		  				(pL2L3Header->DestTid!=M_TID_L2OAM)&&
		  				(pL2L3Header->DestTid!=M_TID_PCISIO)&&
		  				(pL2L3Header->DestTid!=M_TID_WBBU)&&
		  				(pL2L3Header->DestTid<M_TID_MAX))
		  				{
		  				       #ifndef MEM_LEAK
		  				         if(pL2L3Header->DestTid==M_TID_EB)
		  				         	{
		  				         	    l2l3_count.count_l2_37++;
		  				         	  //   pComsg->Destroy();
		  				         	  //   return;
		  				         	}
		  				            else if(pL2L3Header->DestTid == M_TID_WRRU)
		  				            	{
		  				            	      l2l3_count.count_l2_40++;
		  				            	}
		                                     #endif
									#ifdef M_TGT_WANIF

                                                             if(Wanif_Switch==0x5a5a)
                                                             {
							                if((WorkingWcpeEid==pL2L3Header->EID)&&((pL2L3Header->MsgId==MSGID_HIGH_PRIORITY_TRAFFIC)||(pL2L3Header->MsgId==MSGID_LOW_PRIORITY_TRAFFIC)
												||(pL2L3Header->MsgId==MSGID_REALTIME_TRAFFIC)))
							                {
							                     pComsg->SetDstTid((TID)M_TID_WANIF);
										  l2l3_count.count_l2_44++;
							                }
                                                             }
									#endif
									  if(false==CComEntity::PostEntityMessage(pComsg))
									  	{
                                                                               if(l2l3_count.count_l2_4%100==0)
                                                                               	{
										     logMsg("TID:%x\n",pComsg->GetDstTid(),0,1,2,3,4);
                                                                               	}
									  	     l2l3_count.count_l2_4++;
									  	   //  pComsg->Destroy();
								    	           DeallocateComMessage(pComsg);//20090921 add 
									  	     return;
									  	}
									     else
									     	{
									     	    l2l3_count.count_l2_45++;
									     	}
		 
		  				}
		  			else
		  				{
		  				      l2l3_count.count_l2_15++;
		  				}
		  			#endif
		  //			rec_first_flag = 0;
		    l2l3_count.count_l2_5++;
	   }
	   else//多个包
	   {
	      #if 1
	       if((datalen>pack_count*1536))
	       	{
	       	l2l3_count.count_l2_24++;
	       	    return;
	       	}
	   			pMsg  =    GetComMessage(1);
				 if( NULL == pMsg )
				 {
				       //count_l2_8++;
				       l2l3_count.count_l2_6++;
				  //      logMsg("\r\nL2L3Message pool is used up", 0, 0, 0, 0, 0, 0);
				        ::L2L3_Drv_Reclaim(  pComsg->GetMblk());
				        return;
				  }
				
		 char *pSrcData = new char[pack_count*1536/*pL2L3Header->MsgLen+M_DEFAULT_RESERVED*/];
		 if(pSrcData==NULL)
		 	{
		 	    l2l3_count.count_l2_22++;
		 	    return;
		 	}
		 memcpy(pSrcData+M_DEFAULT_RESERVED,pPload,(datalen-32));
		  pMsg->SetDstTid((TID)pL2L3Header->DestTid);
     pMsg->SetSrcTid((TID)pL2L3Header->SrcTid);
     pMsg->SetMessageId(pL2L3Header->MsgId);
     pMsg->SetUID(pL2L3Header->UID);
     pMsg->SetEID(pL2L3Header->EID);
		  pMsg->SetDataLength(datalen-32/*pL2L3Header->MsgLen*/);
		  pMsg->SetDataPtr(pSrcData+M_DEFAULT_RESERVED);
		  pMsg->SetBuffer(pSrcData, pack_count*1536/*pL2L3Header->MsgLen+M_DEFAULT_RESERVED*/);
		  pMsg->SetFlag(M_CREATOR_TASK_FRAG);
		  #ifdef L2_L3_PRINT
		  printf("datalen:%d,%x\n",datalen,pMsg->GetDstTid());
               for(int ll =0;ll < 5;ll++)
               {
                   for(int kk = 0; kk<34;kk++)
                   	{
                   	    printf("%02x,",pSrcData[64+ll*34+kk]);
                   	    
                   	}
                   printf("end:\n");
               }
                
		  #endif
           //    DeallocateComMessage(pComsg);
           #endif
		 rec_first_flag = 0x55;
     
       l2l3_count.count_l2_14++;
	   }
	}
	else//后续包
	{
	   current_seq = (info>>1)&0x1f;
	 //  	printf("info:%x,%x,%x,%x,%d\n",info,current_seq,last_seq,pack_count,pComsg->GetDataLength());
	if((pMsg!=NULL)&&(rec_first_flag==0x55))
	{
	   if(pMsg->GetBufferLength()!=pack_count*1536)
	   	{
	   	     l2l3_count.count_l2_23++;
	   	     return;
	   	}
	   if((current_seq-last_seq)==1)//包是连续的话
	   {
	       l2l3_count.count_l2_25++;
	     
	       last_seq = current_seq;
	       #if 1
	   	pPload = (UINT8*)((UINT8*) pComsg->GetDataPtr() + 16);
	   	datalen = pComsg->GetDataLength() - 16;
	   	UINT8 *currentPtr =(UINT8*)pMsg->GetDataPtr();
		UINT16  currentLen = pMsg->GetDataLength();
		if((currentLen+datalen)<(pack_count*1536))
			{
					memcpy((UINT8*)(currentPtr+currentLen),pPload,datalen);
					pMsg->SetDataLength(1514/*datalen+currentLen -10*/);
			}
		else
			{
			  l2l3_count.count_l2_19++;
			}
		  #endif
	        if(current_seq==(pack_count-1))//最后一包的话
	        {
	        #ifdef L2_L3_PRINT
               printf("se datalen:%d,%x\n",datalen,pMsg->GetDstTid());
                
                 printf("datalen:%d \n",(datalen+currentLen));
                int lll;
               for(lll =0;lll <( (datalen+currentLen)/34);lll++)
               {
                   for(int kkk = 0; kkk<34;kkk++)
                   	{
                   	    printf("%02x,",currentPtr[lll*34+kkk]);
                   	    
                   	}
                   printf("\n");
               }
               for(int jjj = lll*34;jjj<(datalen+currentLen);jjj++)
               {
               	    printf("%02x,",currentPtr[jjj]);
               }
                
		  #endif


	        
	        #if 1
	           if((pMsg->GetDstTid()!=M_TID_L2IF)&&
		  				(pMsg->GetDstTid()!=M_TID_L2MAIN)&&
		  				(pMsg->GetDstTid()!=M_TID_L2BOOT)&&
		  				(pMsg->GetDstTid()!=M_TID_DIAG)&&
		  				(pMsg->GetDstTid()!=M_TID_VAC)&&
		  				(pMsg->GetDstTid()!=M_TID_DAC)&&
		  				(pMsg->GetDstTid()!=M_TID_L2OAM)&&
		  				(pMsg->GetDstTid()!=M_TID_PCISIO)&&
		  				(pMsg->GetDstTid()<M_TID_MAX))
	           	{
	           		#ifdef M_TGT_WANIF

				 if(Wanif_Switch==0x5a5a)//add judge 20100719
				 {
		                if((WorkingWcpeEid==pMsg->GetEID())&&((pMsg->GetMessageId()==MSGID_HIGH_PRIORITY_TRAFFIC)||(pMsg->GetMessageId()==MSGID_LOW_PRIORITY_TRAFFIC)
							||(pMsg->GetMessageId()==MSGID_REALTIME_TRAFFIC)))
		                {
		                     pComsg->SetDstTid((TID)M_TID_WANIF);
		                }
				 }
				 #endif
	             if(false==CComEntity::PostEntityMessage(pMsg))
	             	{
	             	   // count_l2_10++;
	             	   l2l3_count.count_l2_7++;
	             	 //    DeallocateComMessage(pMsg);
	             	}
				 else
				 	{
				 	      l2l3_count.count_l2_46++;
				 	}
	            }
	           else
	           	{
	           	       l2l3_count.count_l2_16++;
	           	      // pMsg->Destroy();
	           	      DeallocateComMessage(pMsg);
	           	}
	           #endif
	             rec_first_flag = 0xff;
	             l2l3_count.count_l2_8++;
	        }
	     
	   }
	   else//包不连续的话，则直接释放先前分配的pMsg；
	   {
	    //增加static
	   // count_l2_9++;
	       l2l3_count.count_l2_9++;
	    #if 1
	    if(rec_first_flag==0x55/*pMsg->GetFlag()==M_CREATOR_TASK_FRAG*/)
	    	{
	          DeallocateComMessage(pMsg);//内存可能泄露在这里
	         l2l3_count.count_l2_18++;
	         rec_first_flag = 0xff;
	    	}
	    #endif
	   }
	 }
	else
		{
		    l2l3_count.count_l2_17++;
		     rec_first_flag = 0xff;
		}
	   // DeallocateComMessage(pComsg);
	}
	
	
}
/*************************************
这个函数主要是将从驱动收到包组成一个comMessage，然后再发送出去。

DSt_MAC(6B) SRC_MAC(6B)TYPE(2B)INFO(2B)L2L3HEADER(16B)+PayLOAD

      15 |    14|13  |12  |11  |10  |9  |8  |7  |6  |5  |4  |3  |2  |1  |0  |
      bit 0:0-表示第一包，1-表示后续包
      bit 1-5:当bit0为0时，1-5表示包的总数，否则表示包的序号
      bit 6-9:表示模块id这个只在发送给L2时有用，L3不用处理

*********************************************/

void CTaskPciIf::L2L3IngressCore1(CComMessage* pComsg)
{
 //	 EtherHdrL2L3 *pEtherEnd = (EtherHdrL2L3*)( pComsg->GetDataPtr() );
	UINT16 info = GetPacketInfo(pComsg);
	static UINT8 pack_count  = 0;
	static UINT8 last_seq = 0;
	static CComMessage *pMsg = NULL;
	static UINT8  rec_first_flag = 0;
	UINT16 datalen;
	UINT8* pPload ;//= (UINT8*)((UINT8*) pComsg->GetDataPtr() + 32);
	UINT8 current_seq;
  l2l3_count1.count_l2_26++;

	if((info&1)==0)//第一包
	{
	   if(rec_first_flag==0x55)/**需要释放**/
	   	{
	   	     DeallocateComMessage(pMsg);
	   	     rec_first_flag = 0xff;
	   	     l2l3_count1.count_l2_20++;
	   	}
	   last_seq = 0;
	   L2L3_MSG_HEADER *pL2L3Header = (L2L3_MSG_HEADER*)((UINT8*)pComsg->GetDataPtr()+16);
	   pPload = (UINT8*)((UINT8*) pComsg->GetDataPtr() + 32);
	   pack_count =((info>>1)&0x1f);
	  // printf("info:%x,%d\n",info,pComsg->GetDataLength());
	   datalen = pComsg->GetDataLength();
	  if(datalen<=32)
	  	{
	  	     l2l3_count1.count_l2_21++;
	  	    return;
	  	}
	   if(pack_count==1)//只有一包的话,则直接发送不需要组报进行处理
	   {
	      
	         pComsg->SetDstTid((TID)pL2L3Header->DestTid);
           pComsg->SetSrcTid((TID)pL2L3Header->SrcTid);
           pComsg->SetMessageId(pL2L3Header->MsgId);
           pComsg->SetUID(pL2L3Header->UID);
           pComsg->SetEID(pL2L3Header->EID);
		  			pComsg->SetDataLength(pL2L3Header->MsgLen);
		  			pComsg->SetDataPtr((void*)pPload);
		  #if 1
		  			if((pL2L3Header->DestTid!=M_TID_L2IF)&&
		  				(pL2L3Header->DestTid!=M_TID_L2MAIN)&&
		  				(pL2L3Header->DestTid!=M_TID_L2BOOT)&&
		  				(pL2L3Header->DestTid!=M_TID_DIAG)&&
		  				(pL2L3Header->DestTid!=M_TID_VAC)&&
		  				(pL2L3Header->DestTid!=M_TID_DAC)&&
		  				(pL2L3Header->DestTid!=M_TID_L2OAM)&&
		  				(pL2L3Header->DestTid!=M_TID_PCISIO)&&
		  				(pL2L3Header->DestTid!=M_TID_WBBU)&&
		  				(pL2L3Header->DestTid<M_TID_MAX))
		  				{
		  				       #ifdef MEM_LEAK
		  				         if(pL2L3Header->DestTid==M_TID_EB)
		  				         	{
		  				         	    l2l3_count1.count_l2_37++;
		  				         	     pComsg->Destroy();
		  				         	     return;
		  				         	}
		                                     #endif
							#ifdef M_TGT_WANIF

                                               if(Wanif_Switch==0x5a5a)//add judge 20100719
                                               	{
					                if((WorkingWcpeEid==pL2L3Header->EID)&&((pL2L3Header->MsgId==MSGID_HIGH_PRIORITY_TRAFFIC)||(pL2L3Header->MsgId==MSGID_LOW_PRIORITY_TRAFFIC)
										||(pL2L3Header->MsgId==MSGID_REALTIME_TRAFFIC)))
					                {
					                     pComsg->SetDstTid((TID)M_TID_WANIF);
					                }
                                               	}
							#endif
									  if(false==CComEntity::PostEntityMessage(pComsg))
									  	{
									  	     
									  	     l2l3_count1.count_l2_4++;
									  	     pComsg->Destroy();
								    	     //DeallocateComMessage(pComsg);//20090921 add 
									  	     return;
									  	}
									  else
									  	{
									  	     l2l3_count1.count_l2_45++;
									  	}
		 
		  				}
		  			else
		  				{
		  				      l2l3_count1.count_l2_15++;
		  				}
		  			#endif
		  //			rec_first_flag = 0;
		    l2l3_count1.count_l2_5++;
	   }
	   else//多个包
	   {
	      #if 1
	       if((datalen>pack_count*1536))
	       	{
	       	l2l3_count1.count_l2_24++;
	       	    return;
	       	}
	   			pMsg  =    GetComMessage(1);
				 if( NULL == pMsg )
				 {
				       //count_l2_8++;
				       l2l3_count1.count_l2_6++;
				  //      logMsg("\r\nL2L3Message pool is used up", 0, 0, 0, 0, 0, 0);
				        ::L2L3_Drv_Reclaim(  pComsg->GetMblk());
				        return;
				  }
				
		 char *pSrcData = new char[pack_count*1536/*pL2L3Header->MsgLen+M_DEFAULT_RESERVED*/];
		 if(pSrcData==NULL)
		 	{
		 	    l2l3_count1.count_l2_22++;
		 	    return;
		 	}
		 memcpy(pSrcData+M_DEFAULT_RESERVED,pPload,(datalen-32));
		  pMsg->SetDstTid((TID)pL2L3Header->DestTid);
     pMsg->SetSrcTid((TID)pL2L3Header->SrcTid);
     pMsg->SetMessageId(pL2L3Header->MsgId);
     pMsg->SetUID(pL2L3Header->UID);
     pMsg->SetEID(pL2L3Header->EID);
		  pMsg->SetDataLength(datalen-32/*pL2L3Header->MsgLen*/);
		  pMsg->SetDataPtr(pSrcData+M_DEFAULT_RESERVED);
		  pMsg->SetBuffer(pSrcData, pack_count*1536/*pL2L3Header->MsgLen+M_DEFAULT_RESERVED*/);
		  pMsg->SetFlag(M_CREATOR_TASK_FRAG);
		  #ifdef L2_L3_PRINT
		  printf("datalen:%d,%x\n",datalen,pMsg->GetDstTid());
               for(int ll =0;ll < 5;ll++)
               {
                   for(int kk = 0; kk<34;kk++)
                   	{
                   	    printf("%02x,",pSrcData[64+ll*34+kk]);
                   	    
                   	}
                   printf("end:\n");
               }
                
		  #endif
           //    DeallocateComMessage(pComsg);
           #endif
		 rec_first_flag = 0x55;
     
       l2l3_count1.count_l2_14++;
	   }
	}
	else//后续包
	{
	   current_seq = (info>>1)&0x1f;
	 //  	printf("info:%x,%x,%x,%x,%d\n",info,current_seq,last_seq,pack_count,pComsg->GetDataLength());
	if((pMsg!=NULL)&&(rec_first_flag==0x55))
	{
	   if(pMsg->GetBufferLength()!=pack_count*1536)
	   	{
	   	     l2l3_count1.count_l2_23++;
	   	     return;
	   	}
	   if((current_seq-last_seq)==1)//包是连续的话
	   {
	       l2l3_count1.count_l2_25++;
	     
	       last_seq = current_seq;
	       #if 1
	   	pPload = (UINT8*)((UINT8*) pComsg->GetDataPtr() + 16);
	   	datalen = pComsg->GetDataLength() - 16;
	   	UINT8 *currentPtr =(UINT8*)pMsg->GetDataPtr();
		UINT16  currentLen = pMsg->GetDataLength();
		if((currentLen+datalen)<(pack_count*1536))
			{
					memcpy((UINT8*)(currentPtr+currentLen),pPload,datalen);
					pMsg->SetDataLength(1514/*datalen+currentLen -10*/);
			}
		else
			{
			  l2l3_count1.count_l2_19++;
			}
		  #endif
	        if(current_seq==(pack_count-1))//最后一包的话
	        {
	        #ifdef L2_L3_PRINT
               printf("se datalen:%d,%x\n",datalen,pMsg->GetDstTid());
                
                 printf("datalen:%d \n",(datalen+currentLen));
                int lll;
               for(lll =0;lll <( (datalen+currentLen)/34);lll++)
               {
                   for(int kkk = 0; kkk<34;kkk++)
                   	{
                   	    printf("%02x,",currentPtr[lll*34+kkk]);
                   	    
                   	}
                   printf("\n");
               }
               for(int jjj = lll*34;jjj<(datalen+currentLen);jjj++)
               {
               	    printf("%02x,",currentPtr[jjj]);
               }
                
		  #endif


	        
	        #if 1
	           if((pMsg->GetDstTid()!=M_TID_L2IF)&&
		  				(pMsg->GetDstTid()!=M_TID_L2MAIN)&&
		  				(pMsg->GetDstTid()!=M_TID_L2BOOT)&&
		  				(pMsg->GetDstTid()!=M_TID_DIAG)&&
		  				(pMsg->GetDstTid()!=M_TID_VAC)&&
		  				(pMsg->GetDstTid()!=M_TID_DAC)&&
		  				(pMsg->GetDstTid()!=M_TID_L2OAM)&&
		  				(pMsg->GetDstTid()!=M_TID_PCISIO)&&
		  				(pMsg->GetDstTid()<M_TID_MAX))
	           	{
	           	     #ifdef M_TGT_WANIF
				   if(Wanif_Switch==0x5a5a)//add judge 20100719
				   	{
		                if((WorkingWcpeEid==pMsg->GetEID())&&((pMsg->GetMessageId()==MSGID_HIGH_PRIORITY_TRAFFIC)||(pMsg->GetMessageId()==MSGID_LOW_PRIORITY_TRAFFIC)
							||(pMsg->GetMessageId()==MSGID_REALTIME_TRAFFIC)))
		                {
		                     pComsg->SetDstTid((TID)M_TID_WANIF);
		                }
				   	}
				#endif
	             if(false==CComEntity::PostEntityMessage(pMsg))
	             	{
	             	   // count_l2_10++;
	             	   l2l3_count1.count_l2_7++;
	             	 //    DeallocateComMessage(pMsg);
	             	}
				 else
				 {
				 	   l2l3_count1.count_l2_46++;
				 }
				 
	            }
	           else
	           	{
	           	       l2l3_count1.count_l2_16++;
	           	      // pMsg->Destroy();
	           	      DeallocateComMessage(pMsg);
	           	}
	           #endif
	             rec_first_flag = 0xff;
	             l2l3_count1.count_l2_8++;
	        }
	     
	   }
	   else//包不连续的话，则直接释放先前分配的pMsg；
	   {
	    //增加static
	   // count_l2_9++;
	       l2l3_count1.count_l2_9++;
	    #if 1
	    if(rec_first_flag==0x55/*pMsg->GetFlag()==M_CREATOR_TASK_FRAG*/)
	    	{
	          DeallocateComMessage(pMsg);//内存可能泄露在这里
	         l2l3_count1.count_l2_18++;
	         rec_first_flag = 0xff;
	    	}
	    #endif
	   }
	 }
	else
		{
		    l2l3_count1.count_l2_17++;
		     rec_first_flag = 0xff;
		}
	   // DeallocateComMessage(pComsg);
	}
	
	
}
/*********************************************

处理收到L2的消息，处理后，转发给L3的其他任务，这里不释放neicun





***************************************************/
bool CTaskPciIf::ProcessComMessage(CComMessage *pComMsg)
{
    UINT16 usMsgId = pComMsg->GetMessageId();
    switch ( usMsgId )
        {
        case MSGID_TRAFFIC_INGRESS_L2L3:
               //  count_l2_4++;
                // Ingress traffic process.
                l2l3_count.count_l2_3++;
                L2L3Ingress( pComMsg );
            break;
      case  MSGID_TRAFFIC_INGRESS_L2L3_CORE1:
      	    
          	 l2l3_count1.count_l2_3++;
                L2L3IngressCore1( pComMsg );
                break;

        default:
                LOG1( LOG_WARN, LOG_ERR_PCI_DEBUG_INFO, 
                 "Bridge receive unexpected message[Id: 0x%x].", usMsgId );
            break;
        }

    //destroy message.
    pComMsg->Destroy();


    return true;
}
  unsigned int   voice10ms ;
void printL2L3Count()
{
    //logMsg("count:%x,%x,%x,%x,%x,%x\n",count_l2_0,count_l2_1,count_l2_2,count_l2_3,count_l2_4,count_l2_5);

    printf("L2core 0count(0-9):%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",l2l3_count.count_l2_0,l2l3_count.count_l2_1,l2l3_count.count_l2_2,l2l3_count.count_l2_3,l2l3_count.count_l2_4,l2l3_count.count_l2_5,l2l3_count.count_l2_6,l2l3_count.count_l2_7,l2l3_count.count_l2_8,l2l3_count.count_l2_9);
    printf("core 0count(10-18):%d,%d,%d,%d,%d,%d,%d,%d,%d\n",l2l3_count.count_l2_10,l2l3_count.count_l2_11,l2l3_count.count_l2_12,l2l3_count.count_l2_13, l2l3_count.count_l2_14,l2l3_count.count_l2_15,l2l3_count.count_l2_16,l2l3_count.count_l2_17,l2l3_count.count_l2_18);
    printf("core 0count(19-29):%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", l2l3_count.count_l2_19, l2l3_count.count_l2_20, l2l3_count.count_l2_21, l2l3_count.count_l2_22,l2l3_count.count_l2_23,l2l3_count.count_l2_24,l2l3_count.count_l2_25,l2l3_count.count_l2_26,l2l3_count.count_l2_27,l2l3_count.count_l2_28,l2l3_count.count_l2_29);
    printf("core 0count(30-39):%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", l2l3_count.count_l2_30, l2l3_count.count_l2_31, l2l3_count.count_l2_32,l2l3_count.count_l2_33,l2l3_count.count_l2_34,l2l3_count.count_l2_35,l2l3_count.count_l2_36,l2l3_count.count_l2_37,l2l3_count.count_l2_38,l2l3_count.count_l2_39,l2l3_count.count_l2_40,voice10ms);
     printf("core 0count(44-46):%d,%d,%d\n",l2l3_count.count_l2_44, l2l3_count.count_l2_45, l2l3_count.count_l2_46);
    
}
void printL2L3CountCore1()
{
    //logMsg("count:%x,%x,%x,%x,%x,%x\n",count_l2_0,count_l2_1,count_l2_2,count_l2_3,count_l2_4,count_l2_5);

    printf("L2core 1count(0-9):%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",l2l3_count1.count_l2_0,l2l3_count1.count_l2_1,l2l3_count1.count_l2_2,l2l3_count1.count_l2_3,l2l3_count1.count_l2_4,l2l3_count1.count_l2_5,l2l3_count1.count_l2_6,l2l3_count1.count_l2_7,l2l3_count1.count_l2_8,l2l3_count1.count_l2_9);
    printf("core 1count(10-18):%d,%d,%d,%d,%d,%d,%d,%d,%d\n",l2l3_count1.count_l2_10,l2l3_count1.count_l2_11,l2l3_count1.count_l2_12,l2l3_count1.count_l2_13, l2l3_count1.count_l2_14,l2l3_count1.count_l2_15,l2l3_count1.count_l2_16,l2l3_count1.count_l2_17,l2l3_count1.count_l2_18);
    printf("core 1count(19-29):%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", l2l3_count1.count_l2_19, l2l3_count1.count_l2_20, l2l3_count1.count_l2_21, l2l3_count1.count_l2_22,l2l3_count1.count_l2_23,l2l3_count1.count_l2_24,l2l3_count1.count_l2_25,l2l3_count1.count_l2_26,l2l3_count1.count_l2_27,l2l3_count1.count_l2_28,l2l3_count1.count_l2_29);
    printf("core 1count(30-39):%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", l2l3_count1.count_l2_30, l2l3_count1.count_l2_31, l2l3_count1.count_l2_32,l2l3_count1.count_l2_33,l2l3_count1.count_l2_34,l2l3_count1.count_l2_35,l2l3_count1.count_l2_36,l2l3_count1.count_l2_37,l2l3_count1.count_l2_38,l2l3_count1.count_l2_39,voice10ms);
    printf("core 1count(44-46):%d,%d,%d\n", l2l3_count1.count_l2_44,l2l3_count1.count_l2_45, l2l3_count1.count_l2_46);
    
}
void testL1(UINT8  core,UINT32 times,UINT16 len)
{
     int i = 0,r;
     //
   UINT8  core1;
    for(;;)
  //   for (i = 0; i< times; i++)
     	{

     r=rand()%2000;
     len=max(r,60);
	    core1 = core%2;
     	     	CComMessage *pComMsg = NULL;
     	     		pComMsg = new (CTaskPciIf::GetInstance(), len) CComMessage;
     	     		if(pComMsg==NULL)
     	     			{
     	     			 printf("mem alloc fail\n");
     	     			continue;
     	     			}
     	     		pComMsg->SetDstTid(M_TID_L2MAIN);
     	     		pComMsg->SetSrcTid(M_TID_CM);
     	     		pComMsg->SetMoudlue(core1);
     	     		pComMsg->SetMessageId(0x99);
     	     		char *p=(char *)pComMsg->GetDataPtr();
     	     		for(int j = 0;j< len; j++, p++)
     	     			{
     	     			      *p =(char)j;
     	     			}
     	     		if(false==CComEntity::PostEntityMessage(pComMsg))
     	     			{
     	     			         logMsg("hello\n",0,1,2,3,4,5);
     	     			}
     	     		i++;
     	     		#if 1
     	     		if((i%511)==0)
     	     			{
     	     		//	memShow(1);
     	     		//	printL2L3Count();
     	     			taskDelay(100);
     	     			}
     	     		#endif
            core++;
     	     		//len++;
     	     		//len = len%12000;
     	     		
     	     	
     	}
}


int testL2_stop=0;

void testL2(unsigned short  msgid,UINT8  core,UINT32 times,UINT16 len)
{
     int i = 0;
     //

     testL2_stop=0;
 
     for (;;)
     	{
     	i++;
     	if(testL2_stop)
     		break;
     		
     	     	CComMessage *pComMsg = NULL;
     	     		pComMsg = new (CTaskPciIf::GetInstance(), len) CComMessage;
     	     		pComMsg->SetDstTid(M_TID_L2MAIN);
     	     		pComMsg->SetSrcTid(M_TID_EB);
     	     		if(i%2==0)
     	     		{
     	     		pComMsg->SetMoudlue(0);
     	     		}
     	     		else
     	     			{
     	     			  pComMsg->SetMoudlue(1);
     	     			}
     	     		pComMsg->SetMessageId(msgid);
     	     		pComMsg->SetEID(0x12345678);
     	     		char *p=(char *)pComMsg->GetDataPtr();
     	     		for(int j = 0;j< len; j++, p++)
     	     			{
     	     			      *p =(char)j;
     	     			}
     	     		if(false==CComEntity::PostEntityMessage(pComMsg))
     	     			{
     	     			         logMsg("hello2\n",0,1,2,3,4,5);
     	     			}
     	     		if((i%100000)==0){
     	     			taskDelay(10);
     	     		}
     	}
}

void test_stop() {
	testL2_stop=1;
}


void clearL2Count()
{
      memset((char *)&l2l3_count,0,sizeof(L2L3IF_STATIC));
      memset((char *)&l2l3_count1,0,sizeof(L2L3IF_STATIC));
      voice10ms=0;
}
    
  /* extern "C"  void  clearcount0();
    extern  "C" void    clearcount1();
    extern  "C" void    printcount0();
    extern  "C" void    printcount1();
    extern  "C" void    Ebprint();
    extern "C" void CEbprint();
    extern "C"  void print_diag(unsigned char flag);
	extern "C" void printwanif();
	extern "C" void clearwanif();
	extern "C" void print_wcpe_count(unsigned char flag);*/
void printok(unsigned char flag)
{
  /*if(flag == 0)
  	{
    clearL2Count();
 //  clearcount0();
 //   clearcount1();
      CEbprint();
	  print_diag(0);
	  clearwanif();
	  print_wcpe_count(0);
  	}
  else
  	{
 
      printcount0();
   	printcount1();
     printL2L3Count();
     printL2L3CountCore1();
     Ebprint();
    print_diag(1);
    printwanif();
    print_wcpe_count(1);
  	}*/
}



