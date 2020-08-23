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

#ifndef _INC_L3OAMBCCPESWREQ
#include "L3OamBCCPESWReq.h"
#endif
#include <string.h>
#if 0
CBCUTSWReq :: CBCUTSWReq(CMessage &rMsg)
:CMessage(rMsg)
{  }

CBCUTSWReq :: CBCUTSWReq()
{
}

bool CBCUTSWReq :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_BC_UPGRADE_UT_SW_REQ);
	return true;
}

UINT32 CBCUTSWReq :: GetDefaultDataLen() const
{
    return sizeof(T_BCUTSWReq);
}

UINT16 CBCUTSWReq :: GetTransactionId() const
{
    return ((T_BCUTSWReq *)GetDataPtr())->TransId;
}

UINT16 CBCUTSWReq :: SetTransactionId(UINT16 Id)
{
    ((T_BCUTSWReq *)GetDataPtr())->TransId = Id;
	return 0;
}

UINT8 CBCUTSWReq :: GetRetryTimes()
{
    return ((T_BCUTSWReq *)GetDataPtr())->m_RetryTimes;
}

void CBCUTSWReq :: SetRetryTimes(UINT8 Times)
{
    ((T_BCUTSWReq*)GetDataPtr())->m_RetryTimes = Times;
}

UINT8  CBCUTSWReq :: GetModelType() const
{
    return ((T_BCUTSWReq*)GetDataPtr())->m_ModelType;
}

void   CBCUTSWReq :: SetModelType(UINT8)
{}

UINT8  CBCUTSWReq :: GetSWType() const
{
    return ((T_BCUTSWReq*)GetDataPtr())->m_SWType;
}

void   CBCUTSWReq :: SetSWType(UINT8 T)
{
    ((T_BCUTSWReq*)GetDataPtr())->m_SWType = T;
}

bool CBCUTSWReq :: GetSWVer(SINT8* SWVer, UINT8 Len) const
{
    if(NULL == SWVer)
    {
        return false;
    }
    else
    {
        memcpy(SWVer, ((T_BCUTSWReq*)GetDataPtr())->m_SWVer, Len);
        return true;
    }    
}

bool CBCUTSWReq :: SetSWVer(SINT8* SWVer, UINT8 Len)
{
    if(NULL == SWVer)
    {
        return false;
    }
    else
    {
        memcpy(((T_BCUTSWReq*)GetDataPtr())->m_SWVer, SWVer, Len);
        return true;
    }    
}

bool CBCUTSWReq :: GetFileName(SINT8* Name) const
{
    if(!Name)
    {
        return false;
    }

    SINT8 NameBuff[FILE_NAME_LEN];
    memset(NameBuff, 0, sizeof(NameBuff));
    UINT8 ModType = GetModelType();
    //UINT8 SWType  = GetSWType();
    UINT8 hwtype1 = (ModType & 0xf0)>>4;
    UINT8 hwtype2 = (ModType & 0x0f);
    if(0 == hwtype1)
    {
        if(1 == hwtype2)
        {
            strcpy(NameBuff, "hs.om.");
        }
        else if(2 == hwtype2)
        {
            strcpy(NameBuff, "pcmcia.om.");
        }
        else if(3 == hwtype2)
        {
            strcpy(NameBuff, "cpe.om.");
        }
        else if(4 == hwtype2)
        {
            strcpy(NameBuff, "cpe.bm.");
        }        
        else
        {
            return false;
        }
    }

    if(1 == hwtype1)
    {
        if(1 == hwtype2)
        {
            strcpy(NameBuff, "hs.or.");
        }
        else if(2 == hwtype2)
        {
            strcpy(NameBuff, "pcmcia.or.");
        }
        else if(3 == hwtype2)
        {
            strcpy(NameBuff, "cpe.or.");
        }
        else if(4 == hwtype2)
        {
            strcpy(NameBuff, "cpe.br.");
        }        
        else
        {
            return false;
        }
    }
    
    if(8 == hwtype1)
    {
        if(3 == hwtype2)
        {
            strcpy(NameBuff, "cpez.om.");
        }
        else if(4 == hwtype2)
        {
            strcpy(NameBuff, "cpez.bm.");
        }
        else
        {
            return false;
        }
    }

    if(9 == hwtype1)
    {
        if(3 == hwtype2)
        {
            strcpy(NameBuff, "cpez.or.");
        }
        else if(4 == hwtype2)
        {
            strcpy(NameBuff, "cpez.br.");
        }
        else
        {
            return false;
        }
    }
   
/*    
    if(0 == ModType)
    {
        if(1 == SWType)
        {
            strcpy(NameBuff, "handset.");
        }
        else if(2 == SWType)
        {
            strcpy(NameBuff, "pcmcia.");
        }
        else if(3 == SWType)
        {
            strcpy(NameBuff, "cpe_1m.");
        }
        else if(4 == SWType)
        {
            strcpy(NameBuff, "cpe_5m.");
        }        
        else
        {
            false;
        }
    }

    if(1 == ModType)
    {
        if(1 == SWType)
        {
            strcpy(NameBuff, "handset_rda.");
        }
        else if(2 == SWType)
        {
            strcpy(NameBuff, "pcmcia_rda.");
        }
        else if(3 == SWType)
        {
            strcpy(NameBuff, "cpe_rda_1m.");
        }
        else if(4 == SWType)
        {
            strcpy(NameBuff, "cpe_rda_5m.");
        }        
        else
        {
            false;
        }
    }
    
    if(8 == ModType)
    {
        if(3 == SWType)
        {
            strcpy(NameBuff, "cpez_1m.");
        }
        else if(4 == SWType)
        {
            strcpy(NameBuff, "cpez_5m.");
        }
        else
        {
            false;
        }
    }
    else
    {
        false;
    }

    if(9 == ModType)
    {
        if(3 == SWType)
        {
            strcpy(NameBuff, "cpez_rda_1m.");
        }
        else if(4 == SWType)
        {
            strcpy(NameBuff, "cpez_rda_5m.");
        }
        else
        {
            false;
        }
    }
    else
    {
        false;
    }
*/
    SINT8 VerBuff[FILE_NAME_LEN];
    memset(VerBuff, 0, sizeof(VerBuff));
    GetSWVer(VerBuff);
    strcat(NameBuff, VerBuff);
    strcat(NameBuff, ".bin");
    
    memcpy(Name, NameBuff, strlen(NameBuff));

    return true;
}

CBCUTSWReq :: ~CBCUTSWReq()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
CBCUTSWReqNew :: CBCUTSWReqNew(CMessage &rMsg)
:CMessage(rMsg)
{  }

CBCUTSWReqNew :: CBCUTSWReqNew()
{
}

bool CBCUTSWReqNew :: CreateMessage(CComEntity& Entity)
{
    CMessage :: CreateMessage(Entity);
    SetMessageId(M_EMS_BTS_BC_UPGRADE_UT_SW_REQ_NEW);
	return true;
}

UINT32 CBCUTSWReqNew :: GetDefaultDataLen() const
{
    return sizeof(T_BCUTSWReqNew);
}

UINT16 CBCUTSWReqNew :: GetTransactionId() const
{
    return ((T_BCUTSWReqNew *)GetDataPtr())->TransId;
}

UINT16 CBCUTSWReqNew :: SetTransactionId(UINT16 Id)
{
    ((T_BCUTSWReqNew *)GetDataPtr())->TransId = Id;
	return 0;
}

UINT8 CBCUTSWReqNew :: GetRetryTimes()
{
    return ((T_BCUTSWReqNew *)GetDataPtr())->m_RetryTimes;
}

void CBCUTSWReqNew :: SetRetryTimes(UINT8 Times)
{
    ((T_BCUTSWReqNew*)GetDataPtr())->m_RetryTimes = Times;
}

UINT8 CBCUTSWReqNew :: GetFileNameLen()
{
    return ((T_BCUTSWReqNew *)GetDataPtr())->m_FileNameLen;
}

void CBCUTSWReqNew :: SetFileNameLen(UINT8 len)
{
    ((T_BCUTSWReqNew*)GetDataPtr())->m_FileNameLen = len;
}

bool  CBCUTSWReqNew :: GetFileName(SINT8 *FileName, UINT8 Len) const
{
    if(NULL == FileName)
    {
        return false;
    }
    else
    {
        memcpy(FileName, ((T_BCUTSWReqNew*)GetDataPtr())->m_FileName, Len);
        return true;
    }  
}

bool   CBCUTSWReqNew :: SetFileName(SINT8 *FileName,UINT8 Len)
{
    if(NULL != FileName)
    {
        memcpy(((T_BCUTSWReqNew*)GetDataPtr())->m_FileName, FileName, Len);
        return true;
    }
    else
    {
        return false;
    }
}

CBCUTSWReqNew :: ~CBCUTSWReqNew()
{
}
  
#endif //0
