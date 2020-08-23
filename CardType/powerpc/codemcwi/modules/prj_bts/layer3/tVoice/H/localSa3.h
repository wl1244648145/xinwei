/*******************************************************************************
* Copyright (c) 2009 by Beijing Jiaxun Feihong Electrical Co.Ltd.All Rights Reserved   
* File Name      : localSagCommon.h
* Create Date    : 13-Oct-2009
* programmer     :
* description    :
* functions      : 
* Modify History :
*******************************************************************************/
#ifndef	__LOCALSAGCOMMON_H
#define	__LOCALSAGCOMMON_H
#include "voiceCommon.h"
#include "CallSignalMsg.h"

#define M_MAX_CCB_NUM		VOICE_CCB_NUM
#define M_MAX_GRPCCB_NUM	GRP_CCB_NUM

#define M_MAX_PHONE_NUMBER_LEN			(30)
#define M_MAX_PHONE_NUMBER_PREFIX_LEN	(6)

#define M_LOCALSAG_ASSIGNRES_REASON_GRP_SETUP (0xAA)
#define M_LOCALSAG_ASSIGNRES_REASON_GRP_TALKING (0xAB)

#define SAG_INNER_CallArrive_Msg (InvalidSignal_MSG+0x1000)

typedef CMsg_Signal_VCR CSAbisSignal;
typedef VoiceVCRCtrlMsgT SAbisSignalT;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __LOCALSAGCOMMON_H */



