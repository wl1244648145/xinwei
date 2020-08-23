/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataDmConfig.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------        ----------------------------------------------------
 *   10/12/05   yang huawei  initialization. 
 *
 *---------------------------------------------------------------------------*/

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
#include <string.h>

#include "L3DataDmConfig.h"
#include "L3OAMMessageId.h"

/*============================================================
MEMBER FUNCTION:
    CCPEDataConfigReq

DESCRIPTION:
   Define function of class  CCPEDataConfigReq
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
//
UINT32  CCPEDataConfigReq::GetDefaultDataLen() const
{
    return sizeof(CPEDataConfigReq);  
}
UINT16  CCPEDataConfigReq::GetDefaultMsgId() const
{
    return M_CPEM_DM_CPE_DATA_CFG_NOTIFY;
}

UINT16 CCPEDataConfigReq :: GetTransactionId()const
{
    return((CPEDataConfigReq*)GetDataPtr())->TransId;
}

UINT16 CCPEDataConfigReq :: SetTransactionId(UINT16 TransId)
{
    return((CPEDataConfigReq*)GetDataPtr())->TransId = TransId;
}

UINT8  CCPEDataConfigReq :: GetMobility() const
{
    return((CPEDataConfigReq*)GetDataPtr())->Mobility;
}

void   CCPEDataConfigReq :: SetMobility(UINT8 Mobility)
{
    ((CPEDataConfigReq*)GetDataPtr())->Mobility = Mobility;
}

UINT8 CCPEDataConfigReq :: GetDHCPRenew() const
{
    return((CPEDataConfigReq*)GetDataPtr())->DHCPRenew;
}

void  CCPEDataConfigReq :: SetDHCPRenew(UINT8 DHCPRenew)
{
    ((CPEDataConfigReq*)GetDataPtr())->DHCPRenew = DHCPRenew;
}


//by xiaoweifang; to support vlan.
UINT16 CCPEDataConfigReq::GetVlanId() const
{
    return((CPEDataConfigReq*)GetDataPtr())->usVlanId;
}


UINT8 CCPEDataConfigReq :: GetMaxIpNum() const
{
    return((CPEDataConfigReq*)GetDataPtr())->MaxIpNum;
}

void  CCPEDataConfigReq ::  SetMaxIpNum(UINT8 MaxIpNum)
{
    ((CPEDataConfigReq*)GetDataPtr())->MaxIpNum = MaxIpNum;
}

UINT8 CCPEDataConfigReq :: GetFixIpNum() const
{
    return((CPEDataConfigReq*)GetDataPtr())->FixIpNum;
}

void  CCPEDataConfigReq ::  SetFixIpNum(UINT8 FixIpNum)
{
    ((CPEDataConfigReq*)GetDataPtr())->FixIpNum = FixIpNum;
}

bool   CCPEDataConfigReq :: GetCpeFixIpInfo(SINT8* DstBuff, UINT16 Len)const
{
    if ( NULL == DstBuff )
        {
        return false; 
        }
    else
        {
        memcpy(DstBuff, (void*)(((CPEDataConfigReq*)GetDataPtr())->stCpeFixIpInfo), Len);
        return true;
        }
}

bool   CCPEDataConfigReq :: SetCpeFixIpInfo(SINT8* SrcBuff, UINT16 Len)
{
    if ( NULL == SrcBuff )
        {
        return false; 
        }
    else
        {
        memcpy((SINT8*)(((CPEDataConfigReq*)GetDataPtr())->stCpeFixIpInfo), 
               SrcBuff, 
               Len);
        return true;
        }
}


/*============================================================
MEMBER FUNCTION:
    CDMConfigReq

DESCRIPTION:
   Define function of class  CDMConfigReq
ARGUMENTS:
 
RETURN VALUE:
   
SIDE EFFECTS:
  
==============================================================*/
UINT32  CDMConfigReq::GetDefaultDataLen() const
{
    return sizeof(DMConfigReq);  
}
UINT16  CDMConfigReq::GetDefaultMsgId() const
{
    return M_CFG_DM_DATA_SERVICE_CFG_REQ;
}
UINT16   CDMConfigReq::GetXid() const
{
    return( (DMConfigReq*)GetDataPtr() )->Xid;
}
UINT16   CDMConfigReq::SetXid(UINT16 id)
{
    return( (DMConfigReq*)GetDataPtr() )->Xid=id;
}

UINT8   CDMConfigReq::GetMobility() const
{
    return( (DMConfigReq*)GetDataPtr() )->Mobility;
}
UINT8    CDMConfigReq::SetMobility(UINT8 ucMb)
{
    return( (DMConfigReq*)GetDataPtr() )->Mobility=ucMb;
}

UINT8   CDMConfigReq::GetAccessCtrl() const
{
    return( (DMConfigReq*)GetDataPtr() )->AccessCtrl;
}
UINT8    CDMConfigReq::SetAccessCtrl(UINT8 ucM)
{
    return( (DMConfigReq*)GetDataPtr() )->AccessCtrl=ucM;
}


