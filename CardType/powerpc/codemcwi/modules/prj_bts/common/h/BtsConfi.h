/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    btsConfig.h
 *
 * DESCRIPTION: This module defines types common for any processor and
 *              specifies the specifics for that processor's compiler.
 *
 * HISTORY:
 *
 *   Date          Author    Description
 *   ---------    ------    ----------------------------------------------------
 *   03/07/2006   Yushu Shi  Initial file creation. 
 *
 *---------------------------------------------------------------------------*/
#ifndef __BTS_CONFIG_H__
#define __BTS_CONFIG_H__
typedef enum {MCP0 = 0, MCP1, MCP2, MCP3, MCP4, MCP5, MCP6, MCP7, AUXDSP} DSPNAME;

#define M_NumberOfDSP	(AUXDSP + 1)
#define M_NumberOfMCP	AUXDSP
#define M_NumberOfFEP	2

#endif //__BTS_CONFIG_H__

