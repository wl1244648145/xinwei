/*******************************************************************************
* Copyright (c) 2010 by Beijing Arrowping Communication Co.Ltd.All Rights Reserved   
* File Name      : ExternalIf.h
* Create Date    : 15-Dec-2010
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/

#ifndef	__ExternalIF_H
#define	__ExternalIF_H

//------------------------------------------------------------------------------
//common
#include "L3OamCfgCommon.h"
#include "sysBtsConfigData.h"
#include "L3OamSystem.h"
//#include "l3OamCpeM.h"

extern T_NvRamData *NvRamDataAddr;

//------------------------------------------------------------------------------
#ifdef __VXWORKS__
#ifndef WBBU_CODE
extern struct sockaddr_in stSagAddr0;
extern struct sockaddr_in stSagAddr1;
extern struct sockaddr_in stSagAddr2;
#endif
#endif
//------------------------------------------------------------------------------
#ifdef WBBU_CODE
#include "sysBootLoad.h"



#endif
//------------------------------------------------------------------------------
#ifdef DSP_BIOS

#include "time.h"

extern "C" UINT32 l3oamGetEIDByUID(UINT32 UID);//wangwenhua add 20081208
extern "C" UINT32 l3oamGetUIDByEID(UINT32 EID);//通过EID得到UID

extern "C" void mySwitchFxn(int tid,unsigned int inOut,void* pMsg);
extern "C" UINT32 GetBtsIpAddr();
extern "C" int bspGetBtsID();
extern "C" T_TimeDate bspGetDateTime();
extern "C" RESET_REASON bspGetBtsResetReason();
extern  "C" unsigned int  getSysSec();
extern  unsigned int  getTimeNUll();

#define taskDelay(ticks) TSK_sleep(ticks)
inline time_t __GetTime__(time_t *pTime)
{
	time_t ret = (time_t)getTimeNUll();
	if(pTime)
	{
		*pTime = ret;
	}
	return ret;
}

#else

#define __GetTime__ time
extern UINT32 l3oamGetEIDByUID(UINT32 UID);//wangwenhua add 20081208
extern UINT32 l3oamGetUIDByEID(UINT32 EID);//通过EID得到UID

#endif

//------------------------------------------------------------------------------


#ifdef M_SYNC_BROADCAST
extern UINT16 gMBMSGPSFlag;//记录GPS是否同步过，提供voice使用
extern UINT16 gMBMSParaFlag;//记录同播功能是否打开变量，提供voice使用
//extern "C" bool gpsEverOK(){return gMBMSGPSFlag;};
//extern "C" bool usingSyncBroadcast(){return gMBMSParaFlag;};
inline bool canUseSyncBroadcast(){return gMBMSParaFlag&&gMBMSGPSFlag;};
#endif//M_SYNC_BROADCAST



#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif /* __ExternalIF_H */



