/*****************************************************************************
 *              (C) Copyright 2005: Arrowping Telecom
 *                   Arrowping Confidential Proprietary
 *****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME: 
 *
 * DESCRIPTION:   defines the load version string
 *
 *
 * HISTORY:
 *
 *   Date         Author       Description
 *   ----------  ----------  ----------------------------------------------------
 *   03/27/2006   Yushu Shi      Initial file creation.
 *---------------------------------------------------------------------------*/
#ifndef LOAD_VERSION_H
#define LOAD_VERSION_H

#if defined DMBT
#define VERSION "McWill 3.6.0.71"     
#elif defined WBBU_CODE
#define VERSION "McWill 2.7.0.31"  
#elif defined WBBU
#define VERSION "McWill 2.7.0.6"  
#else
#define VERSION "McWill 1.6.10.11"//LARGE_BTS
#endif
#endif

