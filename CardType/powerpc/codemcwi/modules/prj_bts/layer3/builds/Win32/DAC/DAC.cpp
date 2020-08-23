/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    DAC.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   11/29/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/
#include <assert.h>
#include <stdio.h>

#include "Object.h"
#include "biztask.h"
#include "MsgQueue.h"
#include "Message.h"
#include "taskDef.h"
#include "LogArea.h"
#include "Timer.h"

#include "DAC.h"


//任务实例指针的初始化
CTaskDAC* CTaskDAC::s_ptaskDAC = NULL;


/*============================================================
MEMBER FUNCTION:
    CTaskDAC::ProcessMessage

DESCRIPTION:
    DAC任务消息处理函数

ARGUMENTS:
    CMessage: 消息

RETURN VALUE:
    bool:true or false,FrameWork根据返回值决定是否做PostProcess()

SIDE EFFECTS:
    none
==============================================================*/
bool CTaskDAC::ProcessMessage(CMessage &msg)
{
    TID srcTid = msg.GetSrcTid();
    if ( M_TID_EB == srcTid )
        {
        msg.SetDstTid( M_TID_UTDM );
        }
    if ( M_TID_UTDM == srcTid )
        {
        msg.SetDstTid( M_TID_EB );
	    swap16( (UINT8*)msg.GetDataPtr(), msg.GetDataLength() );
        if ( 0 != msg.GetDataLength() % 2 )
            {
            assert( 0 );
            printf("!!!!!!!!!!!!!!!!!!!!!!!!!");
            }
        }

    msg.SetSrcTid( M_TID_L2MAIN );
    msg.Post();

    return true;
}



/*============================================================
MEMBER FUNCTION:
    CTaskDAC::CTaskDAC

DESCRIPTION:
    CTaskDAC构造函数

ARGUMENTS:
    NULL

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CTaskDAC::CTaskDAC()
{
    memset( m_szName, 0, M_TASK_NAME_LEN );
    memcpy( m_szName, M_TASK_DAC_TASKNAME, strlen( M_TASK_DAC_TASKNAME ) );
    m_uPriority     = M_TP_L2DAC;
    m_uOptions      = M_TASK_DAC_OPTION;
    m_uStackSize    = M_TASK_DAC_STACKSIZE;
    m_iMsgQMax      = M_TASK_DAC_MAXMSG;
    m_iMsgQOption   = M_TASK_DAC_MSGOPTION;
}


/*============================================================
MEMBER FUNCTION:
    CTaskDAC::GetInstance

DESCRIPTION:
    Get CTaskDAC Task Instance.

ARGUMENTS:
    NULL

RETURN VALUE:
    CTaskDAC* 

SIDE EFFECTS:
    none
==============================================================*/
CTaskDAC* CTaskDAC::GetInstance()
{
    if ( NULL == s_ptaskDAC )
        {
        s_ptaskDAC = new CTaskDAC;
        }
    return s_ptaskDAC;
}


void CTaskDAC::swap16(UINT8 *pData, UINT32 ulLen)
{
    for( UINT32 ulIdx = 0; ulIdx < ulLen / 2; ++ulIdx )
        {
        UINT32 ulOffset = ulIdx * 2;
        UINT8 temp      = pData[ ulOffset ];
        pData[ ulOffset ]       = pData[ ulOffset + 1 ];
        pData[ ulOffset + 1 ]   = temp;
        }
}

