/*----------------------------------------------------------------------------
 * FILENAME:    L2L3MsgDef.h
 *
 * DESCRIPTION: This .h file defines all messages for L2 Tx Rx . 
 *              
 *
 * HISTORY:
 *
 *   Date       Author        Description
 *   ---------  ------------  ------------------------------------------------
 *   06/18/07   Yushu Shi     Inititial file creation. 
 *
 *---------------------------------------------------------------------------*/
#ifndef _L2_L3_MSG_H_
#define _L2_L3_MSG_H_

#include "dataType.h"
#define L3_ToL2Tx_L2Reset_Notify		0x4400

struct stL2RxResetNotify
{
    UINT16 transactionID;
	UINT16 rsv;
};

#endif

