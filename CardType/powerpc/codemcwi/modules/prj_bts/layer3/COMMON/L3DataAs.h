/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataAssert.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   04/20/06   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef _INC_DATA_ASSERT
#define _INC_DATA_ASSERT

#include <stdio.h>

#define DATA_assert(condition)  \
    { \
    if ( !( condition ) )\
        { \
        printf( "\r\n!!!WARNING!!!Encounter exceptions: %s, line %d\r\n", __FILE__, __LINE__ ); \
        } \
    }

#endif

