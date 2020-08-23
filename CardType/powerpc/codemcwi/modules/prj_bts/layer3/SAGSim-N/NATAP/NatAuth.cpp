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

	{ NATAP_MD5,	"7vbil(wJ4Etm*lf@Hn2(#Zk50I"	},	//01
	{ NATAP_SHA,	"HU!!#g$xG~!W7jbEcEZTS-H5v"	},	//02
	{ NATAP_SHA,	"js`DxVZFOHl*3ra2s-UUkHR`8"	},	//03
	{ NATAP_MD5,	"DI3w0-7v~qp&gAwBFLn_^AFo*8C"	},	//04
	{ NATAP_SHA,	"Le@@4cFp(Y-ASDQLf#u~5r@L!tilT"	},	//05
	{ NATAP_MD5,	"Wc2EEB+4Z#zBd5R)~`dM_Jr2(!1"	},	//06
	{ NATAP_MD5,	"P^UFaJu9-0GS1ki9JOB)woTD=F"	},	//07
	{ NATAP_MD5,	"S2XKQI2d1*W=KV6--Qdgk`Xp75AA"	},	//08
	{ NATAP_MD5,	"yYj@8sy61HlV06xfs*GPC))2$*B"	},	//09
	{ NATAP_MD5,	"bc+#LzGL1Vo&cd7x*jRuw+L)3#0"	},	//10
	{ NATAP_SHA,	"AffYGS3I(I^72IaKeLcG1CZ*Ux_Q"	},	//11		
	{ NATAP_MD5,	"`$jGWL)=e$p`sBO#_kBmk-#uFl"	},	//12
	{ NATAP_SHA,	"JoXWbRVM~`v_-M#vC(CsD+pm7"	},	//13
	{ NATAP_MD5,	"$aya79XyJWp48(JWm26mcEcPc"	},	//14
	{ NATAP_SHA,	"vn5gg6dr2eoH+Ll7eZ`&G^h1#2G"	},	//15
	{ NATAP_MD5,	"zTKE1o1zobcy7iud6RKGr3pcI"	},	//16
	{ NATAP_SHA,	"WQ5AdEb+Il`+I3=rFIs^I=)S+Y"	},	//17
	{ NATAP_SHA,	"R&!y8L8XX$6iHan-TAS=D0ZdrB"	},	//18
	{ NATAP_MD5,	"WvLWHqv~`&J=u^B(jMJJ$(7~T1M"	},	//19
	{ NATAP_MD5,	"bq$li6eSEoXU@58(TK*WFxr~l_fi"	},	//20	
	{ NATAP_MD5,	"C@#$#&O$gkgwl8zb1t`Za`(HHK*SW"	},	//21
	{ NATAP_SHA,	"ohWOjOlVxF*Z2U6xaB_WAj-uW"	},	//22
	{ NATAP_MD5,	"i+(V9i*E)fs)fb-75ws6pBoewonG"	},	//23
	{ NATAP_SHA,	"!lb(ztBf2X7DYgkF(@PE^@ULnc4-"	},	//24
	{ NATAP_MD5,	"HUu#Z66p$-icXy-)ltvaX^Q@)M_kg"	},	//25
	{ NATAP_SHA,	"M26RAX82lz1)J^&u(K-BRS+pYuqp@"	},	//26
	{ NATAP_SHA,	"THBXEg-8zwLCsl7gBZdV~w5Lz8yKD"	},	//27
	{ NATAP_MD5,	"&HPmg9YHoQvB@C@f^540S^3qcJx5"	},	//28
	{ NATAP_SHA,	"PbpK3rpv7j=xZM1PJ4qc&EvU81"	},	//29
	{ NATAP_MD5,	"h)karCsC#xP(pIHP$Y#z1IX58@iB"	},	//30
	{ NATAP_SHA,	"OVAOt+2p*xDhTZfhJP~hE^Rhh"	}	//31
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


