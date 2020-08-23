/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: L3L3L2BtsConfigSYNPower.h
 *
 * DESCRIPTION:
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ------------------------------------------------
 *   09/18/2006   –§Œ¿∑Ω       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3L2BTSCONFIGSYNPOWER_H
#define _INC_L3L2BTSCONFIGSYNPOWER_H

#include "Message.h"

class CL3L2BtsConfigSYNPower : public CMessage
{
public: 
    CL3L2BtsConfigSYNPower(const CMessage &rMsg):CMessage(rMsg){}
    CL3L2BtsConfigSYNPower(){}
    ~CL3L2BtsConfigSYNPower(){}

    bool CreateMessage(CComEntity& Entity);
    UINT16 SetTransactionId(UINT16);
    UINT16 SetOp(UINT16);
protected:
    UINT32 GetDefaultDataLen() const;

private:
#define M_SYNC_POWER_OFF     (0x00)
#define M_SYNC_POWER_ON      (0x01)
#define M_SYNC_POWER_RESET   (0x02)
#pragma pack(1)
typedef struct _tag_ConfigSYNPower
{
    UINT16  usTransId;
    UINT16  usConfigBit;
}stConfigSYNPower;
#pragma pack()
};

#endif

