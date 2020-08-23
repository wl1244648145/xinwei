/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:  implementation of UTAgent module on BTS L3
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   11/27/2005   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#include <stdio.h>

#include "UtAgent.h"
#include "log.h"

CUtAgent * CUtAgent::Instance = NULL;
const TID CUtAgent::ProxyTIDs[6] = { 
                           M_TID_CPECM,      // 101  
                           M_TID_CPESM,      // 102
                           M_TID_CPEDIAG,    // 103
                           M_TID_UTV,	      // 106 CPE voice task
                           M_TID_UTDM,      
                           M_TID_UTAGENT    
                        };
#ifdef WBBU_CODE
unsigned char print_flag_ut = 0;
#endif


/*****************************************************************************
 *
 *   Method:     CUtAgent::CUtAgent()
 *
 *   Description:  Constructor, register to ComEntity for all the 
 *                 OAM tasks on CPE side
 *
 *   Parameters:  None
 *
 *   Returns:  None
 *
 *****************************************************************************
 */
CUtAgent::CUtAgent()
{
    
    for (int i=0; i<SIZEOF(ProxyTIDs); i++)
    {
        CurrentTid = ProxyTIDs[i];
        RegisterEntity(false);
        LOG1(LOG_DEBUG, 0, "Register proxy for task %d finished\n", CurrentTid);
    }

    CurrentTid = ProxyTIDs[0];

    ChecksumErrorCount= 0;
    FromUtOamMsgCount = 0;
    ToUtOamMsgCount = 0; 

}


/*****************************************************************************
 *
 *   Method:     CUtAgent::GetInstance()
 *
 *   Description:  Singleton
 *                
 *   Parameters:  none
 *
 *   Returns:  None
 *
 *****************************************************************************
 */
CUtAgent *CUtAgent::GetInstance()
{
    if ( NULL == Instance )
    {
        Instance = new CUtAgent;
    }
    return Instance;
}


/*****************************************************************************
 *
 *   Method:     CUtAgent::PostMessage()
 *
 *   Description:  interface of CComEntity to accept messages from other modules
 *                
 *   Parameters:  ComMessage and timeout value
 *
 *   Returns:  None
 *
 *****************************************************************************
 */
bool CUtAgent::PostMessage(CComMessage* msg, SINT32 timeout, bool isUrgent)
{
    if ( M_TID_UTAGENT != msg->GetDstTid() )
    {   // message from BTS to UT
        ToUtOamMsgCount ++;

        if ( msg->GetBufferLength()- msg->GetDataLength() < sizeof(T_SecondHeader))
        {
            LOG2(LOG_CRITICAL, 0, "Msg From %d to %d without Enough Leading Space\n", msg->GetSrcTid(), msg->GetDstTid());
            return false;  // can not relay messages without enough leading space
        }

        CComMessage *relayMsg = new (this, 0)CComMessage();
        if ( NULL == relayMsg)
        {
            return false;
        }

        relayMsg->SetBuffer(msg->GetBufferPtr(), msg->GetBufferLength());
        relayMsg->SetSrcTid(msg->GetSrcTid());
#ifndef WBBU_CODE
        relayMsg->SetDstTid(M_TID_L2_TXINL3);
#else
       relayMsg->SetDstTid(M_TID_L2MAIN);
#endif
        relayMsg->SetEID(msg->GetEID());
        relayMsg->SetUID(msg->GetUID());
        relayMsg->SetDataPtr((void *)( (UINT8*)msg->GetDataPtr() - sizeof(T_SecondHeader)));
        relayMsg->SetDataLength(msg->GetDataLength() + sizeof(T_SecondHeader));
        relayMsg->SetFlag( (UINT32) msg );
#ifdef WBBU_CODE
        relayMsg->SetMoudlue(1);/**表示给1核进行处理***/
#endif
        T_SecondHeader *header = (T_SecondHeader*)relayMsg->GetDataPtr();
        
        // compose the second header and change the ComMessage fields
        header->DstTID = (UINT16)msg->GetDstTid();
        header->SrcTID = (UINT16)msg->GetSrcTid();
        header->MsgID  = msg->GetMessageId();
#ifdef WBBU_CODE
		if(print_flag_ut==1)
		{
	  		printf("%x,%x,%x,%x\n", header->DstTID , header->SrcTID, header->MsgID,msg->GetEID());
		}
#endif
        if ( 0xFFFFFFFE != msg->GetEID() )//wangwenhua modify 20080715
        {   // unicast
            relayMsg->SetMessageId(MSGID_HI_PRIORITY_UNICAST_OAM);
        }
        else
        {
            relayMsg->SetMessageId(MSGID_BROADCAST_OAM);
        }
        header->Length = relayMsg->GetDataLength();

        // calculate the checksum
        UINT8 chksum=0;
        UINT8* dataAddr = (UINT8*)&(header->DstTID);
        for (int i=0; i<relayMsg->GetDataLength()-2; i++)  // -2 to exclude the checksum itself and reserve field
        {
            chksum ^= *dataAddr++ ;
        }

        header->Checksum = chksum;
        header->Reserved = 0;

        msg->AddRef();  
        if ( ! CComEntity::PostEntityMessage(relayMsg, timeout, isUrgent))
        {
            // failed to post message, destroy the duplicate and return false to caller
            relayMsg->Destroy();
        }
        return true;  // always return true since the msg is destroyed , otherwise the
                      // sender will call destroy again
    }
    else
    {   // messages from UT to BTS

        FromUtOamMsgCount++;

        UINT8 chksum=0;
        T_SecondHeader *header = (T_SecondHeader *)msg->GetDataPtr();
        UINT8* dataPtr = (UINT8*)&(header->DstTID);

        for (int i=0; i<msg->GetDataLength()-2; i++) // -2 to exclude the checksum and reserve field
        {
            chksum ^= *dataPtr++;
        }
        if (chksum != header->Checksum)
        {
            LOG(LOG_CRITICAL, 0, "From UT OAM message Checksum error\n");
            ChecksumErrorCount++;
            msg->Destroy();   // delete the message
            return true; 
        }

        // strip off the second header in payload and change the ComMessage 
        msg->SetDstTid((TID) header->DstTID );
        msg->SetSrcTid((TID)header->SrcTID);
        msg->SetMessageId( header->MsgID );
        msg->SetDataLength( msg->GetDataLength()- sizeof(T_SecondHeader));
        msg->SetDataPtr( (void *)( (UINT8*)msg->GetDataPtr() + sizeof(T_SecondHeader)));

        return CComEntity::PostEntityMessage(msg, timeout, isUrgent);
    }

}

bool CUtAgent::DeallocateComMessage(CComMessage* pComMsg)
{
    CComMessage *realMsg = (CComMessage*)pComMsg->GetFlag();
    realMsg->Destroy();
    pComMsg->SetBuffer(NULL, 0);
    return CComEntity::DeallocateComMessage(pComMsg);
}

void CUtAgent::ShowStatus()
{
    printf("ChecksumErrorCount = %d\n",ChecksumErrorCount);
    printf("FromUtOamMsgCount = %d\n", FromUtOamMsgCount);
    printf("ToUtOamMsgCount = %d\n", ToUtOamMsgCount);  
};


extern "C"
STATUS utAgentStateShow(unsigned char flag )
{
#ifdef WBBU_CODE
   print_flag_ut = flag;
#endif
    CUtAgent::GetInstance()->ShowStatus();
    return OK;
}




