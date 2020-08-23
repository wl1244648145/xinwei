/*****************************************************************************

 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   define the class for PciIf entity
 *                deliver packets between L2 and L3 through PCI interface
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   12/11/2005   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef __INC_PCIIF
#define __INC_PCIIF

#include "vxWorks.h" 
#include "sockLib.h" 
#include "inetLib.h" 
#include "stdioLib.h" 
#include "strLib.h" 
#include "hostLib.h" 
#include "ioLib.h" 
#include <netBufLib.h>
#include <msgQLib.h>
#include <lstLib.h>
#include <eventLib.h>

#include "Task.h"
#include "ComMessage.h"
#include "taskDef.h"

#include "logArea.h"
#include "BizTask.h"
#include "ComMessage.h"
#include "LogArea.h"
#include "log.h"

#include "L3DataTypes.h"
#define PROXY_TID_NUM  8 /*9*/

/***************************************
 *M_CREATOR_DRV   : 驱动分配的payload
 *M_CREATOR_MYSELF: EB自己分配的payload.
 */
#define M_CREATOR_DRV_SW_NO_FRAG               (0xFFFFFFbb)/***如果是不用组包的话，直接释放驱动的MBLK即可**/
#define M_CREATOR_TASK_FRAG              (0xaaaaaaaa)/**如果需要组包的话，则需要进行copy一次，再进行释放MBLK***/
//#define M_CREATOR_MYSELF            (0xEFEFEFEF)
typedef enum
{
    TYPE_COMMSG = 0,
    TYPE_IPPACKET = 1
}PCI_IF_PKT_TYPE;

typedef struct 
{
    PCI_IF_PKT_TYPE type;
    union
    {
        M_BLK_ID mBlk;
        CComMessage* comMsg;
    };
}PCI_IF_Q_MSG;   // data type in outbound  message queue

#ifdef M_TGT_L3
typedef enum
{
    L2_SYSTEM_STATE_REBOOT = 0x55,
    L2_SYSTEM_STATE_RUNNING = 0xaa
}L2_SYSTEM_STATE;
#endif

typedef struct
{
    UINT16 DestTid;
    UINT16 SrcTid;
    UINT32 EID;
    UINT16 MsgId;
    UINT16 MsgLen;
    UINT16 Reserved;
    UINT16 UID;
}L2L3_MSG_HEADER;

#define IP_NET_BUF_NUM  200
#define IP_NET_BUF_SIZE   2048   //1600, 1600 is not enough for big packets
typedef struct 
{
    NET_POOL_ID pNetPool;
    M_CL_CONFIG mclBlkConfig;
    CL_DESC     clDescTbl;
} PCIIF_MEMPOOL;


typedef enum 
{
    L2L3_FROM_L2 = 0,
    L2L3_FROM_L3,

    L2L3_FROM_MAX
}L2L3FROMDIR;

/*************************************************
 *TRAFFICTYPE: traffic类型
 *************************************************/
typedef enum
{
    L2L3_TYPE_TRAFFIC_VOICE= 0,
    L2L3_TYPE_TRAFFIC_OAM,
    L2L3_TYPE_TRAFFIC_DATA,
    L2L3_TYPE_TRAFFIC_MAX
}L2L3TRAFFICTYPE;


/*********************
 *Ethernet Header
 *********************/
typedef struct _tag_EthHdr_L2L3
{
    UINT8       aucDstMAC[ M_MAC_ADDRLEN ];
    UINT8       aucSrcMAC[ M_MAC_ADDRLEN ]; 
    UINT16      usProto;                    /*network byte order*/
    UINT16      cominfo;
	
} EtherHdrL2L3;

class CTaskPciIf: public CBizTask
{
public:
    static CTaskPciIf* GetInstance();
    void   ReleaseComMsg();
private:
    bool PostMessage(CComMessage*, SINT32, bool isUrgent = false);
    TID GetEntityId() const { return CurrentTid;} ;
    /*void ShowStatus(int);*/
    bool DeallocateComMessage(CComMessage* pComMsg);

   static void L2L3RxDriverPacketCallBack(void *,char *, UINT16, char *,unsigned char);/***驱动调用该函数进行包发送 ***/
   static void L2L3FreeMsgCallBack (UINT32 param);/**驱动调用该函数进行commessage释放****/
   CComMessage* GetComMessage(UINT8 flag);/***从队列中得到commessage***/
  // CComMessage* GetL3ComMessage(UINT8 flag);/***从队列中得到commessage***/


private:
    
    CTaskPciIf();
    bool Initialize();
    ~CTaskPciIf();
    inline bool IsNeedTransaction()
    {
          return false;
    }
    #define PCIIF_MAX_BLOCKED_TIME_IN_10ms_TICK 100
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return PCIIF_MAX_BLOCKED_TIME_IN_10ms_TICK ;};
    
    bool   InitComMessageList();
    bool ProcessComMessage(CComMessage*);

    //提供给其他任务的转发消息接口
  

    //
    void   L2L3Ingress(CComMessage*);//处理core0的消息
    void   L2L3IngressCore1(CComMessage*);//处理core1的消息
	/*******************************************************

      15 |    14|13  |12  |11  |10  |9  |8  |7  |6  |5  |4  |3  |2  |1  |0  |
      bit 0:0-表示第一包，1-表示后续包
      bit 1-5:当bit0为0时，1-5表示包的总数，否则表示包的序号
      bit 6-9:表示模块id这个只在发送给L2时有用，L3不用处理


	**************************************************************************/
    inline UINT16 GetPacketInfo(const CComMessage *pComMessge)//得到包的信息
    {
		 EtherHdrL2L3 *pEtherHdr = (EtherHdrL2L3*) ( pComMessge->GetDataPtr() );
	      return ntohs( pEtherHdr->cominfo );		
     }

	//VxWorks.
    CComMessage *m_plistComMessage;//该队列用于接收
      #ifdef RELEASE_COM
      CComMessage *m_plistComMessage_bak[40000];//wangwenhua add 20090926
     #endif
     TID CurrentTid;
 
    static const TID ProxyTIDs[PROXY_TID_NUM]; 
    CComMessage *m_listcomMsg ;//该队列用于对于需要组包的队列



    UINT32 m_aulDirTrafficMeasure[ L2L3_FROM_MAX ][ L2L3_TYPE_TRAFFIC_MAX ];

    static CTaskPciIf *Instance;

};

#define LOG_ERR_PCI_DEBUG_INFO          LOGNO(PCIIF, 0)
#define LOG_ERR_PCI_L2_REBOOT           LOGNO(PCIIF, 1)
#define LOG_ERR_PCI_INIT_ERROR          LOGNO(PCIIF, 2)
#define LOG_ERR_PCI_QUEUE_OVERFLOW      LOGNO(PCIIF, 3)
#define LOG_ERR_PCI_NOT_ENOUGH_LEADING  LOGNO(PCIIF, 4)
#define LOG_ERR_PCI_FREE_BUFFER_INVALID LOGNO(PCIIF, 5)
#define LOG_ERR_PCI_FREE_BUF_FREE       LOGNO(PCIIF, 6)
#define LOG_ERR_PCI_POST_BUSY_RX_BUF    LOGNO(PCIIF, 7)
#define LOG_ERR_PCI_FREE_EMPTY_RX_BUF   LOGNO(PCIIF, 8)
#define LOG_ERR_PCI_APP_FREE_INVALID_BUF   LOGNO(PCIIF, 9)
typedef struct  _L2L3IF_STATIC
{
 unsigned int count_l2_0;
 unsigned int count_l2_1 ;
 unsigned int count_l2_2 ;
 unsigned int count_l2_3 ;
 unsigned int count_l2_4 ;
unsigned int count_l2_5 ;
 unsigned int count_l2_6 ;
 unsigned int count_l2_7 ;
 unsigned int count_l2_8 ;
 unsigned int count_l2_9 ;
unsigned int count_l2_10 ;
unsigned int count_l2_11 ;
 unsigned int count_l2_12 ;
  unsigned int count_l2_13 ;
  unsigned int count_l2_14;
  unsigned int count_l2_15;
  unsigned int count_l2_16;
    unsigned int count_l2_17;
   unsigned int count_l2_18;
   unsigned int count_l2_19;
   unsigned int count_l2_20;
   unsigned int count_l2_21;
   unsigned int count_l2_22;
   unsigned int count_l2_23;
   unsigned int count_l2_24;
   unsigned int count_l2_25;
   unsigned int count_l2_26;
   unsigned int count_l2_27;
   unsigned int count_l2_28;
   unsigned int count_l2_29;
   unsigned int count_l2_30;
   unsigned int count_l2_31;
  unsigned int  count_l2_32;
  unsigned int  count_l2_33;
  unsigned int  count_l2_34;
    unsigned int  count_l2_35;
    unsigned int  count_l2_36;
  unsigned int  count_l2_37;
  unsigned int  count_l2_38;
  unsigned int count_l2_39;
  unsigned int count_l2_40;
  unsigned int count_l2_41;
  unsigned int count_l2_42;
  unsigned int count_l2_43;
  unsigned int count_l2_44;
   unsigned int count_l2_45;
  unsigned int count_l2_46;

}L2L3IF_STATIC;
#endif //__INC_PCIIF
