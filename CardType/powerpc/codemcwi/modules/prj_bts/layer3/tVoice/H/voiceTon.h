/*******************************************************************************
* Copyright (c) 2009 by Beijing Arrowping Communication Co.Ltd.All Rights Reserved   
* File Name      : voiceTone.h
* Create Date    : 26-Jun-2009
* programmer     :fb
* description    :
* functions      : 
* Modify History :
*******************************************************************************/

#ifndef	__VOICETONE_H
#define	__VOICETONE_H

enum
{
	DIAL_TONE=0,
	RINGBACK_TONE,
	BUSY_TONE,

	TONE_COUNT
};
typedef struct __toneInfoT
{
	unsigned char* pG729Tone;
	unsigned long nToneSize;
	char toneName[20];
	unsigned long nCurOffset;
	unsigned char sn;
}toneInfoT;
extern toneInfoT g_ToneTbl[TONE_COUNT];

#ifdef __cplusplus
extern "C" {
#endif

#define G729TONE_TEST
extern unsigned char *pG729TestTone;
extern unsigned long nLenG729TestTone;



#ifdef __cplusplus
}
#endif

#endif /* __VOICETONE_H */


