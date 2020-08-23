/*****************************************************************************
             (C) Copyright 2010: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    NatPiApp.h
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ------------------------------------------------
 *   2010-11-23 fengbing  initialization. 
 *
 *---------------------------------------------------------------------------*/

#ifndef __INC_NATPIAPP_H
#define __INC_NATPIAPP_H

#include "NatPi.h"

#ifdef __cplusplus
extern "C" {
#endif

NatPiSession* getVcrNatpiSession();
NatPiSession* getVcr1NatpiSession();
NatPiSession* getVdrNatpiSession();
NatPiSession* getVdr1NatpiSession();
NatPiSession* getDcsNatpiSession();
void initNatPiApp();


void showNatApInfo();
void clearNatApPktCounters();
void closeNatApFun();
void closeVDRNatApFunc();
void closeVCRNatApFunc();
void closeDCSNatApFunc();
void setNatApSpiVal(UINT8 val);
void setNatApSpiVal1(UINT8 val);
void setDcsNatApKey(UINT8 val);

#ifdef __cplusplus
}
#endif

#endif /*__INC_NATPIAPP_H*/


