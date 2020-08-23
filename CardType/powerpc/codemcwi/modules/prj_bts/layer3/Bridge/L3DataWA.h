/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataEB.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   06/06/07   xin wang      增加 MAC Filter
 *   03/28/06   xiao weifang  FixIp改成非永久性的用户
 *   07/28/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_WANIF_H__
#define __DATA_WANIF_H__

//socket:
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

#include <map>
using namespace std;
#include <list>

#include "BizTask.h"
#include "ComMessage.h"
#include "LogArea.h"

#include "L3DataTypes.h"



#include "log.h"

#include "L3DataAssert.h"




//Ether Bridge任务参数定义
#define M_TASK_WANIF_TASKNAME      "tWANIF"
#ifdef __WIN32_SIM__
#define M_TASK_WANIF_OPTION        (0x0008)
#define M_TASK_WANIF_MSGOPTION     (0x02)
#else
#define M_TASK_WANIF_OPTION        ( VX_FP_TASK )
#define M_TASK_WANIF_MSGOPTION     ( MSG_Q_FIFO | MSG_Q_EVENTSEND_ERR_NOTIFY )
#endif
#define M_TASK_WANIF_STACKSIZE     (20480)
#define M_TASK_WANIF_MAXMSG        (30000)



/***************************************
 *M_CREATOR_DRV   : 驱动分配的payload
 *M_CREATOR_MYSELF: EB自己分配的payload.
 */
#define M_CREATOR_DRV_WANIF               (0xFFFFFFFF)
//#define M_CREATOR_MYSELF            (0xEFEFEFEF)





//ComMessage链表的长度
#define M_MAX_LIST_SIZE_WANIF            (6000)
#define WANIF_MAX_BLOCKED_TIME_IN_10ms_TICK  (100)



//Ether Bridge task
class CTWANIF : public CBizTask
{



public:

    static CTWANIF* GetInstance();
#ifndef WBBU_CODE
    static void RxDriverPacketCallBack( M_BLK_ID   pMblk,UINT32 VLANID,UINT32 toWho);
#else
    static void RxDriverPacketCallBack( char* pdataptr,UINT32 dataLen,UINT32 toWho,UINT32 hasVlan);
#endif
   static void WANIFFreeMsgCallBack (UINT32 param);

    //////////////////////////////////////////////////////////////////////////
    //以下是测试代码，用来模拟DHCP和PPPOE PADI数据包,还有一种错误数据包,暂且
    //注释掉,使用时打开
    //////////////////////////////////////////////////////////////////////////
    //void sendDHCPPkt();
    //void sendPPPOEPkt();
    //void sendIlligalMAC();


    //调用者确保pData内存足够大
    

#ifndef __WIN32_SIM__
    CComMessage* GetComMessage();
#endif

#ifdef UNITEST
    /*UNITEST时,所有的成员函数都是Public*/
public:
#else
private:
#endif
    CTWANIF();
    ~CTWANIF();

    bool Initialize();
    bool ProcessComMessage(CComMessage*);
    inline TID GetEntityId() const 
        {
        return M_TID_WANIF;
        }
    inline bool IsNeedTransaction()
        {
        return false;
        }
    virtual bool DeallocateComMessage(CComMessage*);
    bool IsMonitoredForDeadlock()  { return true; };
    int  GetMaxBlockedTime() { return  WANIF_MAX_BLOCKED_TIME_IN_10ms_TICK ;};



private:

#ifndef __WIN32_SIM__
    bool   InitComMessageList();
#endif

    

    inline UINT8* GetDstMac(const CComMessage *pComMessge)
        {
        EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
        return pEtherHdr->aucDstMAC;
        }

    inline UINT8* GetSrcMac(CComMessage *pComMessge)
        {
        EtherHdr *pEtherHdr = (EtherHdr*) ( pComMessge->GetDataPtr() );
        return pEtherHdr->aucSrcMAC;
        }
    inline UINT16 GetProtoType(const CComMessage *pComMessge)
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




   // UINT16 GetRelayMsgID(const CComMessage*);


    //
    void   Ingress(CComMessage*);
    

    void   Egress(CComMessage*);

   char  getMacType(CComMessage*);
    bool   SendToWAN(CComMessage*, UINT16 grpID,UINT8 flag);
private:
#ifndef __WIN32_SIM__
//VxWorks.
    CComMessage *m_plistComMessage;
#endif
   
   unsigned char m_btsMac[6];

    //static members:
    static CTWANIF* s_ptaskWANIF;
};



#endif /*__DATA_BRIDGE_H__*/
