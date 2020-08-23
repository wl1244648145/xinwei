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
 *   ----------  ----------  ------------------------------------------------
----
 *   08/03/2005   Ìï¾²Î°       Initial file creation.
 *---------------------------------------------------------------------------*/

#ifndef _INC_L3EMSMESSAGEID
#include "L3EmsMessageId.h"
#endif

// Performance Logging Configuration Request£¨EMS£©
#ifndef _INC_L3OAMCFGPERFLOGREQ
#include "L3OamCfgPerfLogReq.h"     
#endif

#include <string.h>
CCfgPerfLogReq :: CCfgPerfLogReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CCfgPerfLogReq :: CCfgPerfLogReq()
{
}

bool CCfgPerfLogReq :: CreateMessage(CComEntity &Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_PERF_LOGGING_CFG_REQ);
	return true;
}

UINT32 CCfgPerfLogReq :: GetDefaultDataLen() const
{
    return sizeof(T_PerfLogCfgReq);
}

UINT16 CCfgPerfLogReq :: GetTransactionId() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->TransId;
}

UINT16 CCfgPerfLogReq :: SetTransactionId(UINT16 TransId)
{
    ((T_PerfLogCfgReq *)GetDataPtr())->TransId = TransId;
	return 0;
}

SINT32 CCfgPerfLogReq :: GetFTPserverIP() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->Ele.FTPserverIP;
}

void CCfgPerfLogReq ::   SetFTPserverIP(SINT32 FTPserverIP)
{
    ((T_PerfLogCfgReq *)GetDataPtr())->Ele.FTPserverIP = FTPserverIP;
}

UINT16 CCfgPerfLogReq :: GetFTPserverPort() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->Ele.FTPserverPort;
}

void CCfgPerfLogReq ::   SetFTPserverPort(UINT16 FTPserverPort)
{
    ((T_PerfLogCfgReq *)GetDataPtr())->Ele.FTPserverPort = FTPserverPort;
}

UINT8 CCfgPerfLogReq ::  GetUserNameLen() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->Ele.UserNameLen;
}

void CCfgPerfLogReq ::   SetUserNameLen(UINT8 UserNameLen)
{
    ((T_PerfLogCfgReq *)GetDataPtr())->Ele.UserNameLen = UserNameLen;
}

SINT8* CCfgPerfLogReq :: GetUserName() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->Ele.UserName;
}

void CCfgPerfLogReq ::   SetUserName(SINT8* )
{
//    ((T_PerfLogCfgReq *)GetDataPtr())->Ele.UserName;
}

UINT8 CCfgPerfLogReq ::  GetFTPPasswordLen() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->Ele.FTPPasswordLen;
}

void CCfgPerfLogReq ::   SetFTPPasswordLen(UINT8 FTPPasswordLen)
{
    ((T_PerfLogCfgReq *)GetDataPtr())->Ele.FTPPasswordLen = FTPPasswordLen;
}

SINT8* CCfgPerfLogReq :: GetFTPPassword() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->Ele.FTPPassword;
}

void CCfgPerfLogReq ::   SetFTPPassword(SINT8* )
{
    //((T_PerfLogCfgReq *)GetDataPtr())->Ele.FTPPassword;
}

UINT16 CCfgPerfLogReq :: GetPerfRptInter() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->Ele.PerfRptInter;
}

void CCfgPerfLogReq ::   SetPerfRptInter(UINT16 PerfRptInter)
{
    ((T_PerfLogCfgReq *)GetDataPtr())->Ele.PerfRptInter = PerfRptInter;
}

UINT16 CCfgPerfLogReq :: GetPerfCollectInter() const
{
    return ((T_PerfLogCfgReq *)GetDataPtr())->Ele.PerfCollectInter;
}

void CCfgPerfLogReq ::   SetPerfCollectInter(UINT16 PerfCollectInter)
{
    ((T_PerfLogCfgReq *)GetDataPtr())->Ele.PerfCollectInter = PerfCollectInter;
}

bool   CCfgPerfLogReq :: GetEle(SINT8* DstBuff, UINT16 Len)const
{
    if(NULL == DstBuff)
    {
        return false; 
    }
    else
    {
        memcpy(DstBuff, 
               &(((T_PerfLogCfgReq*)GetDataPtr())->Ele),
               Len);
        return true;
    }
}

bool   CCfgPerfLogReq :: SetEle(SINT8* SrcBuff, UINT16 Len)
{
    if(NULL == SrcBuff)
    {
        return false;
    }
    else
    {
        memcpy(&(((T_PerfLogCfgReq*)GetDataPtr())->Ele),
               SrcBuff, 
               Len);
        return true;
    }
}

CCfgPerfLogReq :: ~CCfgPerfLogReq()
{

}

