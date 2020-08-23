/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    DefaultConfig.h
 *
 * DESCRIPTION: This module defines default configuration.
 *
 * HISTORY:
 *
 *   Date       Author    Description
 *   ---------  ------    ----------------------------------------------------
 *   04/12/06   Hui Zhou  Initial file creation. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __DEFAULT_CONFIG_H__
#define __DEFAULT_CONFIG_H__

// UTDefaultServiceDescConfRequest
#define		M_Default_UL_MaxBw			1500	// K unit 1.5M
#define		M_Default_UL_MinBw			8		// K unit, 8K
#define		M_Default_DL_MaxBw			1500
#define		M_Default_DL_MinBw			8

#define		M_UT_UserClass				1	
#define		M_UT_UL_MaxBw				1500
#define		M_UT_UL_MinBw				8		
#define		M_UT_DL_MaxBw				1500
#define		M_UT_DL_MinBw				8

// stUTDacProfile
#define		M_BwReqInterval				20		// frames	
#define		M_SessionRelThreshold		300		// frames

#endif
