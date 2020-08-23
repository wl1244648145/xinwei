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

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

#ifndef _INC_L3OAMBTSREGNOTIFY
#include "L3OamBtsRegNotify.h"
#endif

#include <string.h>

CBtsRegNotify :: CBtsRegNotify(CMessage &rMsg)    
:CMessage(rMsg)
{
}


CBtsRegNotify :: CBtsRegNotify()
{
}

bool CBtsRegNotify :: CreateMessage(CComEntity &Entity)
{
    if (false == CMessage :: CreateMessage(Entity))
        return false;
    SetMessageId(M_BTS_EMS_REG_NOTIFY);
    return true;
}

UINT32 CBtsRegNotify :: GetDefaultDataLen() const
{
    return sizeof(T_BtsRegNotify);
}


UINT16 CBtsRegNotify :: GetTransactionId() const
{
    return ((T_BtsRegNotify *)GetDataPtr())->TransId;
}

UINT16 CBtsRegNotify :: SetTransactionId(UINT16 TransId)
{
    ((T_BtsRegNotify *)GetDataPtr())->TransId = TransId;
		return 0;
}

UINT32 CBtsRegNotify :: GetBtsHWVersion() const
{
    return ((T_BtsRegNotify *)GetDataPtr())->BtsHwVersion;
}

void CBtsRegNotify :: SetBtsHWVersion(UINT32 Version)
{
    ((T_BtsRegNotify *)GetDataPtr())->BtsHwVersion = Version;
}
UINT32 CBtsRegNotify :: GetBtsSWVersion() const
{
    return ((T_BtsRegNotify *)GetDataPtr())->BtsSwVersion;
}

void CBtsRegNotify :: SetBtsSWVersion(UINT32 Version)
{
    ((T_BtsRegNotify *)GetDataPtr())->BtsSwVersion = Version;
}

void   CBtsRegNotify :: SetBtsIp(UINT32 ip)
{
    ((T_BtsRegNotify *)GetDataPtr())->BtsCfgIp = ip;
}

bool   CBtsRegNotify :: GetBtsID(SINT8* DstBuff, UINT8 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_BtsRegNotify *)GetDataPtr())->EncrypedBtsID),
               Len);
        return true;
    }
}

bool   CBtsRegNotify :: SetBtsID(SINT8* SrcBuff, UINT8 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_BtsRegNotify*)GetDataPtr())->EncrypedBtsID),
               SrcBuff, 
               Len);
        return true;
    }
}


UINT16 CBtsRegNotify :: GetBtsRcvPort() const
{
    return ((T_BtsRegNotify *)GetDataPtr())->BtsRcvPort;
}

void   CBtsRegNotify :: SetBtsRcvPort(UINT16 Port)  
{
    ((T_BtsRegNotify *)GetDataPtr())->BtsRcvPort = Port;
}

CBtsRegNotify :: ~CBtsRegNotify()
{
}


