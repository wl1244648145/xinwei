#ifndef _INC_TSOCKET
#include "L3DataSocket.h"
#endif

bool InitL3VoiceSvc()
{
	return true;
}

bool InitL3Oam()
{
	CTask* pTask = new CSOCKET;
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


