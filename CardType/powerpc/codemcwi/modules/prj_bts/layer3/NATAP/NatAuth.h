/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    NatAuth.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   2006-11-27 fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/
 

#ifndef __INC_NATAUTH_H
#define __INC_NATAUTH_H

/* includes */
#include "datatype.h"


#define NATAP_MAX_KEY_LEN	(32)
#define NATAP_SPI_COUNT		(32)

#define M_NATAP_AUTHRESULT_LEN_MD5	(16)
#define M_NATAP_AUTHRESULT_LEN_SHA	(20)

typedef enum _NAT_Auth_TypeT
{
	NATAP_NOAUTH=0,
	NATAP_MD5,
	NATAP_SHA,
	NATAP_INVALID_AUTHTYPE=0xff
}NatAuthTypeT;

typedef struct _NatAlgoCfgItem_T
{
	NatAuthTypeT algo;
	char key[NATAP_MAX_KEY_LEN];
}NatAlgoCfgItemT;

#ifdef __cplusplus
extern "C" {
#endif

NatAuthTypeT NATAPGetAuthTypeBySPI(UINT8 spi);
UINT16 NATAPGetAuthResultLength(UINT8 spi);
UINT16 NATAPComputeAuthResult( UINT8 spi, 
                               UINT8 *pBufInput, 
                               UINT16 nLenInput, 
                               UINT8 *pBufDigest );

#ifdef __cplusplus
}
#endif

#endif /* __INC_NATAUTH_H */

