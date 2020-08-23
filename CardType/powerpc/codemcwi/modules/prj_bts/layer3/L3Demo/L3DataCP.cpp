/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataCPESM.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ------------------------------------------------
 *   1/12/06    xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include "pcap.h"
extern "C"
{
#include "remote-ext.h"
}
#include "pcap-bpf.h"
//#include "sockstorage.h"
#include "packet32.h"
#include "ntddndis.h"

#include "Object.h"
#include "MsgQueue.h"
#include "Message.h"
#include "taskDef.h"
#include "LogArea.h"
#include "Timer.h"

#include "L3DataMsgId.h"
#include "L3DataCPESM.h"
#include "WinPCAP.h"

extern int g_bts_card_id;

//任务实例指针的初始化
CTaskCPESM* CTaskCPESM::s_ptaskCPESM = NULL;


/*============================================================
MEMBER FUNCTION:
    CTaskCPESM::ProcessMessage

DESCRIPTION:
    CPESM任务消息处理函数

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    bool:true or false,FrameWork根据返回值决定是否做PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskCPESM::ProcessMessage(CMessage &msg)
{
#ifdef TEST_UTDM
	if ( 0xFFAB == msg.GetMessageId() )
	{
		UINT8 *pData = (u_char*)msg.GetDataPtr();
		UINT32 ulLen = msg.GetDataLength();
		swap16( pData, ulLen );
		PutToCard( g_bts_card_id, ulLen, pData );
		return true;
	}
#endif
	for(int card_id=1;card_id<get_adpter_num();card_id++)
        {
		if(card_id==g_bts_card_id)
			continue;//skip bts card id
		UINT8 *pData = (u_char*)msg.GetDataPtr();
		UINT32 ulLen = msg.GetDataLength();
		swap16( pData, ulLen );
		PutToCard( card_id, ulLen, pData );
        }
    return true;
}



/*============================================================
MEMBER FUNCTION:
    CTaskCPESM::CTaskCPESM

DESCRIPTION:
    CTaskCPESM构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTaskCPESM::CTaskCPESM()
{
    memcpy( m_szName, M_TASK_CPESM_TASKNAME, strlen( M_TASK_CPESM_TASKNAME ) );
    m_uPriority     = 100;
    m_uOptions      = M_TASK_CPESM_OPTION;
    m_uStackSize    = M_TASK_CPESM_STACKSIZE;
    m_lMaxMsgs      = M_TASK_CPESM_MAXMSG;
    m_lMsgQOption   = M_TASK_CPESM_MSGOPTION;

    m_hMsgQEvent    = NULL;
    m_hTrafficQEvent= NULL;
    memset( m_arrHandle, 0, sizeof( m_arrHandle ) );

    m_pMsgQ = NULL;
}


/*============================================================
MEMBER FUNCTION:
    CTaskCPESM::GetInstance

DESCRIPTION:
    Get CTaskCPESM Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTaskCPESM* 

SIDE EFFECTS:
    none
==============================================================*/
CTaskCPESM* CTaskCPESM::GetInstance()
{
    if ( NULL == s_ptaskCPESM )
        {
        s_ptaskCPESM = new CTaskCPESM;
        }
    return s_ptaskCPESM;
}

bool CTaskCPESM::PostMessage(CComMessage *pComMsg, SINT32 timeout, bool)
{
    if ( false == m_pMsgQ->PostMessage( pComMsg, timeout ) )
        {
        return false;
        }
    ::SetEvent( m_hMsgQEvent );
    return true;
}

bool CTaskCPESM::Initialize()
{
    m_hMsgQEvent    = ::CreateEvent( NULL, false/*Auto Reset*/, false/*inital nonsignaled.*/, M_EVENT_MSGQ );
    m_hTrafficQEvent= ::CreateEvent( NULL, false/*Auto Reset*/, true/*inital signaled.*/, M_EVENT_TRAFFIC);

    //create message queue
    m_pMsgQ = new CMsgQueue( m_lMaxMsgs, m_lMsgQOption );
    if ( NULL == m_pMsgQ )
        {
        //Close Event.
        ::CloseHandle( m_hMsgQEvent );
        ::CloseHandle( m_hTrafficQEvent );
        return false;
        }

    m_arrHandle[ 0 ] = m_hTrafficQEvent;
    m_arrHandle[ 1 ] = m_hMsgQEvent;

    return true;
}


void CTaskCPESM::MainLoop()
{
    for(;;)
        {
//        DWORD hEvent = WaitForMultipleObjects( 2, m_arrHandle, FALSE, /*INFINITE*/200/*Never Time Out*/ );
        DWORD hEvent = WaitForSingleObject( m_hMsgQEvent, 200 );
        if ( WAIT_OBJECT_0 == hEvent )
            {
            //Control Queue.
            InBoundMsgProcess();
            }
		else 
			CaptureTraffic();
        }
}


void CTaskCPESM::InBoundMsgProcess()
{
    CComMessage *pComMsg = NULL;
    //每次事件位置位，则把对应队列中所有的消息都处理完!
    while ( m_pMsgQ->GetCount() > 0 )
        {
        pComMsg = m_pMsgQ->GetMessage( WAIT_FOREVER );
        if ( !ASSERT_VALID( pComMsg ) )
            {
            continue;
            }

        //处理
        ProcessComMessage( pComMsg );
        }
}


void CTaskCPESM::CaptureTraffic()
{
    ether_pcap();
}


/*********************
*收到CPE发到BTS的数据包
*/
void CTaskCPESM::SendToUTDM(int len,UINT8 *pkt_data)
{
    CComMessage *pComMsg = new( CTaskCPESM::GetInstance(), len )CComMessage;
    if ( NULL == pComMsg )
        {
        //ReclaimFreeBuffer( pBuf );
        return;
        }

    pComMsg->SetMessageId( MSGID_TRAFFIC_INGRESS );
    pComMsg->SetDstTid( M_TID_UTDM );
    pComMsg->SetSrcTid( M_TID_CPESM );
    memcpy( pComMsg->GetDataPtr(), pkt_data, len );

    if ( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        //释放消息；回收缓冲区
        pComMsg->Destroy();
        }
}


/**************************************
 *模拟硬件，把Payload部分的数据，每16位
 *的高8位和低8位做一次倒换(ntohs)
 */
void CTaskCPESM::swap16(UINT8 *pData, UINT32 ulLen)
{
    for( UINT32 ulIdx = 0; ulIdx < ulLen / 2; ++ulIdx )
        {
        UINT32 ulOffset = ulIdx * 2;
        UINT8 temp      = pData[ ulOffset ];
        pData[ ulOffset ]       = pData[ ulOffset + 1 ];
        pData[ ulOffset + 1 ]   = temp;
        }

    if ( 0 != ulLen % 2 )
        {
        assert( 0 );
		printf("!!!!!!!!ERORORORORORORORORORORORO!!!!!!!CTaskCPESM::swap16!!!!!!!!!!");
        }

}

#ifdef TEST_UTDM
/*********************
*收到BTS的数据包
*/
void CTaskCPESM::Egress(int len,UINT8 *pkt_data)
{
    CComMessage *pComMsg = new( CTaskCPESM::GetInstance(), len )CComMessage;
    if ( NULL == pComMsg )
        {
        //ReclaimFreeBuffer( pBuf );
        return;
        }

    pComMsg->SetMessageId( MSGID_REALTIME_TRAFFIC );
    pComMsg->SetDstTid( M_TID_UTDM );
    pComMsg->SetSrcTid( M_TID_CPESM );
    memcpy( pComMsg->GetDataPtr(), pkt_data, len );

    if ( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        //释放消息；回收缓冲区
        pComMsg->Destroy();
        }
}
#endif