/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataMacAddress.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   07/28/05   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>

#include "L3DataMacAddress.h"


/*============================================================
MEMBER FUNCTION:
    CMac::CMac

DESCRIPTION:
    CMac构造函数

ARGUMENTS:
    *aucMac: Mac地址指针

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CMac::CMac(const UINT8 *pMac)
{
    if ( NULL != pMac )
        {
        memcpy( m_aucMac, pMac, M_MAC_ADDRLEN );
        }
    else
        {
        memset( m_aucMac, 0, M_MAC_ADDRLEN );
        }
}


/*============================================================
MEMBER FUNCTION:
    CMac::CMac

DESCRIPTION:
    CMac拷贝构造函数

ARGUMENTS:
    const CMac&: 自身引用

RETURN VALUE:
    NULL

SIDE EFFECTS:
    none
==============================================================*/
CMac::CMac(const CMac& constMac)
{
    memcpy( m_aucMac, constMac.m_aucMac, M_MAC_ADDRLEN );
}


/*============================================================
MEMBER FUNCTION:
    CMac::str

DESCRIPTION:
    为方便Mac地址的输出更直观，提供该函数

ARGUMENTS:
    *pOutMac:输出的Mac地址，以XX-XX-XX-XX-XX-XX形式输出

RETURN VALUE:
    UINT8*:

SIDE EFFECTS:
    如果传入的pOutMac缓冲区长度不够18字节，将导致内存被改写
    的严重情况，使用时请注意。
==============================================================*/
const UINT8* CMac::str(UINT8 *pOutMac) const
{
    if ( NULL != pOutMac )
        {
        sprintf( (SINT8*)pOutMac, "%.2X-%.2X-%.2X-%.2X-%.2X-%.2X",
            m_aucMac[ 0 ], m_aucMac[ 1 ], m_aucMac[ 2 ], 
            m_aucMac[ 3 ], m_aucMac[ 4 ], m_aucMac[ 5 ] );
        pOutMac[ M_MACADDR_STRLEN - 1 ] = 0;
        return pOutMac;
        }
    return (const UINT8*)"NULL";
}



/*============================================================
MEMBER FUNCTION:
    CMac::IsBroadCast

DESCRIPTION:
    判断Mac是否广播地址

ARGUMENTS:
    NULL

RETURN VALUE:
    Mac = 组播时返回true;广播是组播的一个特例
    否则返回false;

SIDE EFFECTS:
    none
==============================================================*/
#include "log.h"
#include "logarea.h"
#include "L3DataEBErrCode.h"
bool CMac::IsBroadCast() const
{
    //IPV6,多播。MAC:[.......1 ........ ........ ........] = multicast fram.
    if (0x01 == (m_aucMac[0] & 0x01))
        {
        return true;
        }
    return false;
#if 0
    UINT8 ucIdx = 0;
    while ( ucIdx < M_MAC_ADDRLEN )
        {
        if ( 0xffff != *(UINT16*)( m_aucMac + ucIdx ) )
            {
            return false;
            }
        ucIdx += 2;
        }
    //broad-cast;
    return true;
#endif
}
#ifdef WBBU_CODE
bool CMac::IsL2Addr()const
{
#if 0
   if(m_aucMac[0]==0x00)
   	{
   	    if(m_aucMac[1]==0x01)
   	    	{
   	    	      if(m_aucMac[2]==0x02)
   	    	      	{
   	    	      	     if(m_aucMac[3]==0x03)
   	    	      	     	{
   	    	      	     	    if(m_aucMac[4]==0x04)
   	    	      	     	    	{
   	    	      	     	    	     if(m_aucMac[5]==0x05)
   	    	      	     	    	     	{
   	    	      	     	    	     	     return true;
   	    	      	     	    	     	}
   	    	      	     	    	}
   	    	      	     	}
   	    	      	}
   	    	}
   	    
   	}
   #else
        if(m_aucMac[0]==0x00)
   	{
   	    if(m_aucMac[1]==0xa0)
   	    	{
   	    	      if(m_aucMac[2]==0x1e)
   	    	      	{
   	    	      	     if(m_aucMac[3]==0x01)
   	    	      	     	{
   	    	      	     	    if(m_aucMac[4]==0x01)
   	    	      	     	    	{
   	    	      	     	    	     if(m_aucMac[5]==0x01)
   	    	      	     	    	     	{
   	    	      	     	    	     	     return true;
   	    	      	     	    	     	}
   	    	      	     	    	}
   	    	      	     	}
   	    	      	}
   	    	}
   	    
   	}



   #endif
   return false;
}
#endif
bool CMac:: IsMultiCast() const
{
 //the front 3 bytes of  multicast address are 0x01 ,0x00,0x5e 
    if(m_aucMac[0] == 0x01)
    {
       if(m_aucMac[1] == 0x00)
       {
          if(m_aucMac[2] == 0x5e)
          {
             return true;
          }
       }
    }
	return false;
}

/*============================================================
MEMBER FUNCTION:
    CMac::IsZero

DESCRIPTION:
    判断Mac是否为0

ARGUMENTS:
    NULL

RETURN VALUE:
    Mac = 00-00-00-00-00-00时返回true;
    否则返回false;

SIDE EFFECTS:
    none
==============================================================*/
bool CMac::IsZero() const
{
    UINT8 ucIdx = 0;

    while ( ucIdx < M_MAC_ADDRLEN )
        {
        if ( 0 != *(UINT16*)( m_aucMac + ucIdx ) )
            {
            return false;
            }
        ucIdx += 2;
        }

    return true;
}


/*============================================================
MEMBER FUNCTION:
    CMac::operator < 

DESCRIPTION:
    小于比较操作符，Mac地址的每个Short进行比较

ARGUMENTS:
    const CMac&:比较对象的引用

RETURN VALUE:
    true:CMac的Mac地址 < 传入的CMac对象
    false:否则

SIDE EFFECTS:
    none
==============================================================*/
bool CMac::operator < (const CMac& X) const
{
    UINT8 ucIdx = 0;
    const UINT8 *pXMac = X.GetMac();

    while ( ucIdx < M_MAC_ADDRLEN )
        {
        if ( *(UINT16*)( pXMac + ucIdx )
            != *(UINT16*)( m_aucMac + ucIdx ) )
            {
            break;
            }
        ucIdx += 2;
        }

    if ( M_MAC_ADDRLEN != ucIdx )
        {
        //MAC地址不相同
        return ( *(UINT16*)( m_aucMac + ucIdx ) < *(UINT16*)( pXMac + ucIdx ) );
        }

    //MAC地址相同
    return false;
}


/*============================================================
MEMBER FUNCTION:
    CMac::operator == 

DESCRIPTION:
    等于比较操作符，Mac地址的每个Short进行比较

ARGUMENTS:
    const CMac&:比较对象的引用

RETURN VALUE:
    true:CMac的Mac地址 = 传入的CMac对象
    false:否则

SIDE EFFECTS:
    none
==============================================================*/
bool CMac::operator == (const CMac& X) const
{
    UINT8 ucIdx = 0;
    const UINT8 *pXMac = X.GetMac();

    while ( ucIdx < M_MAC_ADDRLEN )
        {
        if ( *(UINT16*)( pXMac + ucIdx )
            != *(UINT16*)( m_aucMac + ucIdx ) )
            {
            break;
            }
        ucIdx += 2;
        }

    if ( M_MAC_ADDRLEN == ucIdx )
        {
        //MAC地址相同
        return true;
        }

    //MAC地址不相同
    return false;
}
