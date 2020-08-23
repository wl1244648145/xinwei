/*******************************************************************************
* Copyright (c) 2009 by Beijing AP Co.Ltd.All Rights Reserved   
* File Name      : localSagFsm.h
* Create Date    : 10-Oct-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__LOCALSAGFSM_H
#define	__LOCALSAGFSM_H

#include "message.h"
#include "localSagCCB.h"


#ifdef __cplusplus
extern "C" {
#endif

void injectFsm(CMessage& msg);
CCCB* findCCB4ULSignal(CMessage& msg);
CCCB* findCCB(CMessage& msg);
UINT16 parseEvent(CMessage& msg, UINT16 state);


#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAGFSM_H */



