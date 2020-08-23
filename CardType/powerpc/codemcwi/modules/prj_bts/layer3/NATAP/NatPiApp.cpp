#include "voiceCommon.h"
#include "NatPiApp.h"

static NatPiSession* pVcrNatpiSession=NULL;
static NatPiSession* pVcr1NatpiSession=NULL;
static NatPiSession* pVdrNatpiSession=NULL;
static NatPiSession* pVdr1NatpiSession=NULL;
static NatPiSession* pDcsNatpiSession=NULL;

NatPiSession* getVcrNatpiSession()
{
	if(!pVcrNatpiSession)
	{
		pVcrNatpiSession = new NatPiSession(false, NAT_APP_TCP, M_TID_VCR, "VCR");
	}
	return pVcrNatpiSession;
}
NatPiSession* getVcr1NatpiSession()
{
	if(!pVcr1NatpiSession)
	{
		pVcr1NatpiSession = new NatPiSession(false, NAT_APP_TCP, M_TID_VCR1, "VCR1");
	}
	return pVcr1NatpiSession;
}
NatPiSession* getVdrNatpiSession()
{
	if(!pVdrNatpiSession)
	{
		pVdrNatpiSession = new NatPiSession(false, NAT_APP_UDP, M_TID_VDR, "VDR");
	}
	return pVdrNatpiSession;
}
NatPiSession* getVdr1NatpiSession()
{
	if(!pVdr1NatpiSession)
	{
		pVdr1NatpiSession = new NatPiSession(false, NAT_APP_UDP, M_TID_VDR1, "VDR1");
	}
	return pVdr1NatpiSession;
}
NatPiSession* getDcsNatpiSession()
{
	if(!pDcsNatpiSession)
	{
		pDcsNatpiSession = new NatPiSession(false, NAT_APP_TCP, M_TID_DGRV_LINK, "DGRP");
	}
	return pDcsNatpiSession;
}
void initNatPiApp()
{
	getVcrNatpiSession();
	getVdrNatpiSession();
	
	getVcr1NatpiSession();
	getVdr1NatpiSession();

	getDcsNatpiSession();
}


void showNatApInfo()
{
	getVcrNatpiSession()->showStatus();
	getVdrNatpiSession()->showStatus();
	getDcsNatpiSession()->showStatus();
}

void clearNatApPktCounters()
{
	getVcrNatpiSession()->clearCounters();
	getVdrNatpiSession()->clearCounters();

	getVcr1NatpiSession()->clearCounters();
	getVdr1NatpiSession()->clearCounters();

	getDcsNatpiSession()->clearCounters();
}

void closeNatApFun()
{
	getVcrNatpiSession()->stop();
	getVcrNatpiSession()->setEffect(false);
	getVdrNatpiSession()->stop();
	getVdrNatpiSession()->setEffect(false);

	getVcr1NatpiSession()->stop();
	getVcr1NatpiSession()->setEffect(false);
	getVdr1NatpiSession()->stop();
	getVdr1NatpiSession()->setEffect(false);

	getDcsNatpiSession()->stop();
	getDcsNatpiSession()->setEffect(false);
	
}

void closeVDRNatApFunc()
{
	getVdrNatpiSession()->stop();
	getVdrNatpiSession()->setEffect(false);
	
	getVdr1NatpiSession()->stop();
	getVdr1NatpiSession()->setEffect(false);
}

void closeVCRNatApFunc()
{
	getVcrNatpiSession()->stop();
	getVcrNatpiSession()->setEffect(false);
	
	getVcr1NatpiSession()->stop();
	getVcr1NatpiSession()->setEffect(false);
}

void closeDCSNatApFunc()
{
	getDcsNatpiSession()->stop();
	getDcsNatpiSession()->setEffect(false);
}

void setNatApSpiVal(UINT8 val)
{
	getVcrNatpiSession()->setSPIVal(val);
	getVdrNatpiSession()->setSPIVal(val);	
}

void setNatApSpiVal1(UINT8 val)
{
	getVcr1NatpiSession()->setSPIVal(val);
	getVdr1NatpiSession()->setSPIVal(val);
}

void setDcsNatApKey(UINT8 val)
{
	getDcsNatpiSession()->setSPIVal(val);
}



