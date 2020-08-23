/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataEBBuffer.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   03/08/06   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include <tickLib.h>

#include "L3DataEB.h"
#include "L3DataEBBuffer.h"
#include "L3DataAssert.h"


/*============================================================
MEMBER FUNCTION:
    CBufferList::Buffer

DESCRIPTION:
    �������ݰ�

ARGUMENTS:
    *pComMsg: ��Ϣ

RETURN VALUE:
    bool:true or false

SIDE EFFECTS:
    none
==============================================================*/
bool CBufferList::Buffer(CComMessage *pComMsg)
{
    DATA_assert( NULL != pComMsg );

    if ( M_MAGIC_BUFF_TIMESTAMP == pComMsg->GetTimeStamp() )
        {
        ////�������ݵ��ط������ٻ���
        return true;
        }

    //�Ȼ��չ��ڵ����ݰ�
    Reclaim();
#if 0
    UINT16 usBufLength = pComMsg->GetDataLength() + M_DEFAULT_RESERVED;
    UINT8 *pBuf = new UINT8[ usBufLength ];
    if ( NULL == pBuf )
        {
        DATA_assert( 0 );
        return false;
        }
    memcpy( pBuf + M_DEFAULT_RESERVED, pComMsg->GetDataPtr(), pComMsg->GetDataLength() );
#else
    UINT8 *pBuf = (UINT8 *)(pComMsg->GetBufferPtr());
    pComMsg->AddRef();
#endif
    buffernode *pNode   = (buffernode *)pBuf;
    pNode->ulTimeStamp  = tickGet();    //time( NULL );
////pNode->usBufLength  = usBufLength;
    pNode->ptr2msgHdr   = pComMsg;
    pNode->next         = NULL;

    if ( NULL == rear )
        {
        head = rear = pNode;
        }
    else
        {
        rear->next = pNode;
        rear = pNode;
        }
    ////
    ++usNodeCount;

    return true;
}



/*============================================================
MEMBER FUNCTION:
    CBufferList::ReForward

DESCRIPTION:
    ����ת��

ARGUMENTS:
    void

RETURN VALUE:
    N/A

SIDE EFFECTS:
    none
==============================================================*/
void CBufferList::ReForward()
{
    //�Ȼ��չ��ڵ����ݰ�
    Reclaim();

    //Ȼ���ʣ�µĻ������ת����ȥ
    buffernode *cur = head;
    while ( NULL != cur )
        {
        head = cur->next;
        CComMessage *pComMsg = cur->ptr2msgHdr;
        if(NULL != pComMsg)
            {
            ForwardPacket( pComMsg );
            //����ComMsg
            pComMsg->Destroy();
            }
        cur = head;
        }
    ////
    usNodeCount = 0;
    head = NULL;
    rear = NULL;

    return;
}


/*============================================================
MEMBER FUNCTION:
    CBufferList::Reclaim

DESCRIPTION:
    ���չ��ڵ�buffer.

ARGUMENTS:
    void

RETURN VALUE:
    N/A

SIDE EFFECTS:
    none
==============================================================*/
int g_buffer_lease = 100;
void CBufferList::Reclaim()
{
#ifndef __WIN32_SIM__
////VxWorks
    ::taskLock();
#endif
    buffernode *cur = head;
    while ( NULL != cur )
        {
        if ( tickGet() - cur->ulTimeStamp < g_buffer_lease )
            {
            //û�г�ʱ����������
            break;
            }

        //�Ѿ����ڣ�����buffer.
        head = cur->next;
        //delete (UINT8*)cur;
        CComMessage *pComMsg = cur->ptr2msgHdr;
        if(NULL != pComMsg)
            pComMsg->Destroy();
        --usNodeCount;
        cur = head;
        }

    if ( NULL == head )
        {
        rear = NULL;
        DATA_assert( 0 == usNodeCount );
        usNodeCount = 0;
        }

#ifndef __WIN32_SIM__
////VxWorks
    ::taskUnlock();
#endif
    return;
}


/*============================================================
MEMBER FUNCTION:
    CBufferList::DeleteAll

DESCRIPTION:
    �������л����buffer.

ARGUMENTS:
    void

RETURN VALUE:
    N/A

SIDE EFFECTS:
    none
==============================================================*/
void CBufferList::DeleteAll()
{
#ifndef __WIN32_SIM__
////VxWorks
    ::taskLock();
#endif
    buffernode *cur = head;
    while ( NULL != cur )
        {
        //delete (UINT8*)cur;
        head = cur->next;
        CComMessage *pComMsg = cur->ptr2msgHdr;
        if(NULL != pComMsg)
            pComMsg->Destroy();
        cur = head;
        }

    usNodeCount = 0;
    head = NULL;
    rear = NULL;

#ifndef __WIN32_SIM__
////VxWorks
    ::taskUnlock();
#endif
    return;
}


/*============================================================
MEMBER FUNCTION:
    CBufferList::ForwardPacket

DESCRIPTION:
    �ѻ�������ݰ�����comMessage,���͸�EBת��

ARGUMENTS:
    void

RETURN VALUE:
    N/A

SIDE EFFECTS:
    none
==============================================================*/
void CBufferList::ForwardPacket(CComMessage *pComMsg)
{
////CComMessage *pComMsg = CTBridge::GetInstance()->GetComMessage();
    if( NULL == pComMsg )
        {
        ////ɾ
        DATA_assert( 0 );
        //delete pBuf;
        return;
        }
#if 0
    pComMsg->SetBuffer( pBuf, usBufLength );
    pComMsg->SetDataPtr( pBuf + M_DEFAULT_RESERVED );
    pComMsg->SetDataLength( usBufLength - M_DEFAULT_RESERVED );
    pComMsg->SetFlag( M_CREATOR_MYSELF );
#endif
////LOG(LOG_WARN, LOGNO( EB, EC_EB_NORMAL ), "EB reforward packets from buffer.");
    pComMsg->SetTimeStamp( M_MAGIC_BUFF_TIMESTAMP );
    pComMsg->SetDstTid(M_TID_EB);
    pComMsg->SetMessageId(MSGID_TRAFFIC_EGRESS);
    if( false == CComEntity::PostEntityMessage( pComMsg ) )
        {
        DATA_assert( 0 );
        }

    return;
}
