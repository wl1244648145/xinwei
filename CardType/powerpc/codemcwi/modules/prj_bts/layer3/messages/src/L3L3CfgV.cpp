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
 *   08/03/2005   �ﾲΰ       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3OAMMESSAGEID
#include "L3OamMessageId.h"
#endif

#ifndef _INC_L3L3CFGVOICEPORTNOTIFY
#include "L3L3CfgVoicePortNotify.h"     
#endif


CCfgVoicePortNotify :: CCfgVoicePortNotify(CMessage &rMsg)
:CMessage(rMsg)
{  
}

CCfgVoicePortNotify :: CCfgVoicePortNotify()
{

}

bool CCfgVoicePortNotify :: CreateMessage(CComEntity &Entity)
{
    return CMessage :: CreateMessage(Entity);
}

UINT32 CCfgVoicePortNotify :: GetDefaultDataLen() const
{
    return (sizeof(T_CfgNotify));
}

#if 0
UINT16 CCfgVoicePortNotify :: GetTransactionId()const
{
    return ((T_CfgNotify*)GetDataPtr())->TransId;
}

UINT16 CCfgVoicePortNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_CfgNotify*)GetDataPtr())->TransId = TransId;
	return 0;
}
#endif
UINT8  CCfgVoicePortNotify :: GetVoicePort() const
{
    return ((T_CfgNotify*)GetDataPtr())->VoicePort;
}

void   CCfgVoicePortNotify :: SetVoicePort(UINT8 VoicePort)
{
    ((T_CfgNotify*)GetDataPtr())->VoicePort = VoicePort;
}

UINT32 CCfgVoicePortNotify :: GetVoicePortID() const
{
    return ((T_CfgNotify*)GetDataPtr())->VoicePortID;
}

void  CCfgVoicePortNotify :: SetVoicePortID(UINT32 VoicePortID)
{
    ((T_CfgNotify*)GetDataPtr())->VoicePortID = VoicePortID;
}

UINT8 CCfgVoicePortNotify::getIsCpeZ() const
{
    return ((T_CfgNotify*)GetDataPtr())->isCpeZ;
}

void CCfgVoicePortNotify::setIsCpeZ(bool flag)
{
    ((T_CfgNotify*)GetDataPtr())->isCpeZ = (UINT8)flag;
}

CCfgVoicePortNotify ::  ~CCfgVoicePortNotify() 
{

}
