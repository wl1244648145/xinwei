/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   08/03/2005   田静伟       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3L2MESSAGEID
#include "L3L2MessageId.h"
#endif

#ifndef _INC_L3L3L2RFSTATENOTIFY
#include "L3L3L2RFStateNotify.h"
#endif

CL3L2RFStateNoitfy :: CL3L2RFStateNoitfy()
{
}

CL3L2RFStateNoitfy :: CL3L2RFStateNoitfy(CMessage& rMsg)
:CMessage(rMsg)
{
}

bool CL3L2RFStateNoitfy :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_L2_L3_RFSTATE_NOTIFY);
    return true;
}

UINT32 CL3L2RFStateNoitfy :: GetDefaultDataLen() const
{
    return (sizeof(T_Notify));
}

UINT16 CL3L2RFStateNoitfy :: GetTransactionId()const
{
    return ((T_Notify*)GetDataPtr())->TransId;
}

UINT16 CL3L2RFStateNoitfy :: SetTransactionId(UINT16 TransId)
{
    ((T_Notify*)GetDataPtr())->TransId = TransId;
	return 0;
}

UINT8  CL3L2RFStateNoitfy :: GetErrorAntenna()   
{
    UINT8 State = 0;//每一位标识一个天线的状态 0 - disable     1 - enable
  
    for(UINT8 i = 0; i < ANTENNA_NUM; i++) 
    {
        if((((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].BoardVoltminorAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].BoardVoltseriousAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].BoardCurrminorAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].BoardCurrseriousAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].TTAVoltminorAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].TTAVoltserirousAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].TTACurrminorAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].TTACurrSeriousAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].TxPowerminorAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].TxPowerSeriousAlarm != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].SSPChkErr != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].RFChkErr != 0)||
            (((T_Notify*)GetDataPtr())->RFStateInfo.RFChStateInfo[i].RFNoRsp != 0))
        {
            State = State | 1;
        }
        State = State << 1;
    }

    return State;
}

const T_RFStateInfo* CL3L2RFStateNoitfy ::  GetRFStateInfo()const
{
    return (T_RFStateInfo*)&(((T_Notify*)GetDataPtr())->RFStateInfo);
}

CL3L2RFStateNoitfy :: ~CL3L2RFStateNoitfy()
{

}
