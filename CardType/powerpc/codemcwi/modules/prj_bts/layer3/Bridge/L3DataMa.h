/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMacAddress.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   07/28/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DATA_MACADDR_H__
#define __DATA_MACADDR_H__

#include <string.h>	//for VxWorks.

#include "BizTask.h"
#include "L3DataCommon.h"
#include "L3DataTypes.h"
#include "L3DataAssert.h"


/***********************
 *MAC地址类
 ***********************/
class CMac
{
public:
    CMac()
        {
        memset( m_aucMac, 0, M_MAC_ADDRLEN );
        }
    CMac(const UINT8*);
    CMac(const CMac&);
    ~CMac(){}

    const UINT8* GetMac() const
        {
        return m_aucMac;
        }
    void SetMac(const UINT8 *pMac)
        {
        if ( NULL == pMac )
            {
            DATA_assert( 0 );
            return;
            }
        memcpy( m_aucMac, pMac, M_MAC_ADDRLEN );
        }

    //将Mac 地址以 XX-XX-XX-XX-XX-XX
    //的形式输出
    const UINT8* str(UINT8 *pOutMac) const;
    bool IsZero() const;
    bool IsBroadCast() const;
#ifdef WBBU_CODE
    bool IsL2Addr()const;
#endif
    bool IsMultiCast() const;//wangwenhua add 20080829
    //重载运算符
    bool operator <  (const CMac&) const;
    bool operator == (const CMac&) const;
    void operator =  (const CMac &X)
        {
        memcpy( m_aucMac, X.GetMac(), M_MAC_ADDRLEN );
        }

private:
    UINT8 m_aucMac[ M_MAC_ADDRLEN ];
};


#endif /*__DATA_MACADDR_H__*/
