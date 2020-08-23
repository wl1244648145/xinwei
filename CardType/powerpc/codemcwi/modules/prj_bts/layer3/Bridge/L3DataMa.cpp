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
    CMac���캯��

ARGUMENTS:
    *aucMac: Mac��ַָ��

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
    CMac�������캯��

ARGUMENTS:
    const CMac&: ��������

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
    Ϊ����Mac��ַ�������ֱ�ۣ��ṩ�ú���

ARGUMENTS:
    *pOutMac:�����Mac��ַ����XX-XX-XX-XX-XX-XX��ʽ���

RETURN VALUE:
    UINT8*:

SIDE EFFECTS:
    ��������pOutMac���������Ȳ���18�ֽڣ��������ڴ汻��д
    �����������ʹ��ʱ��ע�⡣
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
    �ж�Mac�Ƿ�㲥��ַ

ARGUMENTS:
    NULL

RETURN VALUE:
    Mac = �鲥ʱ����true;�㲥���鲥��һ������
    ���򷵻�false;

SIDE EFFECTS:
    none
==============================================================*/
#include "log.h"
#include "logarea.h"
#include "L3DataEBErrCode.h"
bool CMac::IsBroadCast() const
{
    //IPV6,�ಥ��MAC:[.......1 ........ ........ ........] = multicast fram.
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
    �ж�Mac�Ƿ�Ϊ0

ARGUMENTS:
    NULL

RETURN VALUE:
    Mac = 00-00-00-00-00-00ʱ����true;
    ���򷵻�false;

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
    С�ڱȽϲ�������Mac��ַ��ÿ��Short���бȽ�

ARGUMENTS:
    const CMac&:�Ƚ϶��������

RETURN VALUE:
    true:CMac��Mac��ַ < �����CMac����
    false:����

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
        //MAC��ַ����ͬ
        return ( *(UINT16*)( m_aucMac + ucIdx ) < *(UINT16*)( pXMac + ucIdx ) );
        }

    //MAC��ַ��ͬ
    return false;
}


/*============================================================
MEMBER FUNCTION:
    CMac::operator == 

DESCRIPTION:
    ���ڱȽϲ�������Mac��ַ��ÿ��Short���бȽ�

ARGUMENTS:
    const CMac&:�Ƚ϶��������

RETURN VALUE:
    true:CMac��Mac��ַ = �����CMac����
    false:����

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
        //MAC��ַ��ͬ
        return true;
        }

    //MAC��ַ����ͬ
    return false;
}
