#ifndef _INC_L3OAMDIAG
#include "L3OamDiag.h"
#endif
#include "Log.h"
#include "stdio.h"
bool InitL3VoiceSvc()
{
	return true;
}

bool InitL3Oam()
{
	//CTask* pTask = new CSOCKET;
	//pTask->Begin();
	CTask* pTask = new CL3OamDiagEMSL3L2;
	pTask->Begin();

	return true;
}

bool InitL3DataSvc()
{
	return true;
}

extern "C"
UINT16 bspGetBtsUDPRcvPort()
{
	return 8002;
}

extern "C"
UINT16 bspGetEmsUDPRcvPort()
{
	return 3999;
}

extern "C"
char* bspGetEmsIpAddr()
{
	return "127.0.0.1";
}

extern "C"
UINT32 bspGetBtsID()
{
	return 0x3333;
}

extern "C"
int bspSetNvRamWrite(char * a, UINT32 b, UINT8 c)
{
	return 1;
}


//void OAM_LOGSTR1(LOGLEVEL level, UINT32 errcode, const char* text, int arg1)
//{
//	printf("aaaa");
//}

UINT32 gVx_System_Clock = 0;    /*系统持续运行的时间(秒)*/
//FILE* pgL3oamLogFile;