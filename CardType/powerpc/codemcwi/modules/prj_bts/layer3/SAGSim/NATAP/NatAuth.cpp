/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    NatAuth.cpp
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
#include "md5.h"
#include "sha1.h"
#include "NatAuth.h"
#include "string.h"
                   
NatAlgoCfgItemT g_NatAuthAlgoCfgTbl[NATAP_SPI_COUNT]=
{
	{ NATAP_NOAUTH, ""	},				//00

	{ NATAP_MD5,	"As1!@#$%^._="	},	//01
	{ NATAP_MD5,	"As1!@#$%^._="	},	//02
	{ NATAP_MD5,	"As1!@#$%^._="	},	//03
	{ NATAP_MD5,	"As1!@#$%^._="	},	//04
	{ NATAP_MD5,	"As1!@#$%^._="	},	//05
	{ NATAP_MD5,	"As1!@#$%^._="	},	//06
	{ NATAP_MD5,	"As1!@#$%^._="	},	//07
	{ NATAP_MD5,	"As1!@#$%^._="	},	//08
	{ NATAP_MD5,	"As1!@#$%^._="	},	//09
	{ NATAP_MD5,	"As1!@#$%^._="	},	//10
	{ NATAP_MD5,	"As1!@#$%^._="	},	//11		
	{ NATAP_MD5,	"As1!@#$%^._="	},	//12
	{ NATAP_MD5,	"As1!@#$%^._="	},	//13
	{ NATAP_MD5,	"As1!@#$%^._="	},	//14
	{ NATAP_MD5,	"As1!@#$%^._="	},	//15
	{ NATAP_MD5,	"As1!@#$%^._="	},	//16
	{ NATAP_SHA,	"As1!@#$%^._="	},	//17
	{ NATAP_SHA,	"As1!@#$%^._="	},	//18
	{ NATAP_SHA,	"As1!@#$%^._="	},	//19
	{ NATAP_SHA,	"As1!@#$%^._="	},	//20	
	{ NATAP_SHA,	"As1!@#$%^._="	},	//21
	{ NATAP_SHA,	"As1!@#$%^._="	},	//22
	{ NATAP_SHA,	"As1!@#$%^._="	},	//23
	{ NATAP_SHA,	"As1!@#$%^._="	},	//24
	{ NATAP_SHA,	"As1!@#$%^._="	},	//25
	{ NATAP_SHA,	"As1!@#$%^._="	},	//26
	{ NATAP_SHA,	"As1!@#$%^._="	},	//27
	{ NATAP_SHA,	"As1!@#$%^._="	},	//28
	{ NATAP_SHA,	"As1!@#$%^._="	},	//29
	{ NATAP_SHA,	"As1!@#$%^._="	},	//30
	{ NATAP_SHA,	"As1!@#$%^._="	}	//31
};

NatAuthTypeT NATAPGetAuthTypeBySPI(UINT8 spi)
{
	if(spi>=NATAP_SPI_COUNT)
		return NATAP_INVALID_AUTHTYPE;
	else
		return g_NatAuthAlgoCfgTbl[spi].algo;
}

UINT16 NATAPGetAuthResultLength(UINT8 spi)
{
	NatAuthTypeT algo = NATAPGetAuthTypeBySPI(spi);
	if(NATAP_MD5==algo)
		return M_NATAP_AUTHRESULT_LEN_MD5;
	if(NATAP_SHA==algo)
		return M_NATAP_AUTHRESULT_LEN_SHA;
	//default
	return 0;
}

/*
* compute NATAP AUTH result field, return length of AUTH result
* pBufDigest buffer size should be at least 20
*/
UINT16 NATAPComputeAuthResult( UINT8 spi, 
                               UINT8 *pBufInput, 
                               UINT16 nLenInput, 
                               UINT8 *pBufDigest )
{
	char tmpBuf[500];
	md5_state_t state;
	SHA1Context sha;
	
	if(NATAP_SPI_COUNT<=spi || NATAP_NOAUTH==spi)
		return 0;
	
	//构造加密源 由消息和key组成,消息buf不带加密结果,
	//此时的AH头里的msglen已包括加密结果的长度
	memcpy( (void*)tmpBuf, (void*)pBufInput, nLenInput);
	memcpy( (void*)&tmpBuf[nLenInput], 
			g_NatAuthAlgoCfgTbl[spi].key, 
			strlen(g_NatAuthAlgoCfgTbl[spi].key) );
//MD5使用:
	if( NATAP_MD5==g_NatAuthAlgoCfgTbl[spi].algo )
	{
		md5_init( &state );
		md5_append( &state,
					(const md5_byte_t *)tmpBuf,
					nLenInput+strlen(g_NatAuthAlgoCfgTbl[spi].key) );
		md5_finish( &state, pBufDigest );
		return(M_NATAP_AUTHRESULT_LEN_MD5);
	}
//SHA1使用:
	if( NATAP_SHA==g_NatAuthAlgoCfgTbl[spi].algo )
	{
		if(shaSuccess!= SHA1Reset(&sha))
			return 0;
		if(shaSuccess!=SHA1Input(&sha,
								 (const uint8_t*)tmpBuf,
								 nLenInput+strlen(g_NatAuthAlgoCfgTbl[spi].key)))
			return 0;
		if(shaSuccess!= SHA1Result(&sha, pBufDigest))
			return 0;
		return(M_NATAP_AUTHRESULT_LEN_SHA);
	}

	return 0;
}                                                                     


