//½ûÓÃWinsock 1
#define _WINSOCKAPI_
#include "pcap.h"
extern "C"
{
#include "remote-ext.h"
}
#include "pcap-bpf.h"
//#include "sockstorage.h"
#include "packet32.h"
#include "ntddndis.h"

#include "l3l2tunnel.h"
#include "L3l2tcr.h"
#include "L3DataCPESM.h"
#include "stdhdr.h"
#include "Personator.h"
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include "timertask.h"
#include "Transaction.h"
#include "winPCAP.h"

//externs:
extern void IT_EB();
extern void StartTaskUTDM();

#ifdef M_TGT_L3
#define M_TARGET M_TGT_L3
#elif M_TGT_L2
#define M_TARGET M_TGT_L2
#elif M_TGT_CPE
#define M_TARGET M_TGT_CPE
#else
#error "M_TGT_xxx undefined"
#endif

void CMDLine();

char* pWelCome = "\r\n\r\n\t Welcome to Integate Test! \
                  \r\n\r\n\t Press ? or H for help\r\n\r\n";

void ITusage()
{
    printf("\r\nIntegrate Test:");
    printf("\r\n 0: EBShow()");
    printf("\r\n 1: SNShow()");
    printf("\r\n 2: DMShow()");
    printf("\r\n 3: UTDMShow()");
    printf("\r\n 4: ARPShow()");
    printf("\r\n 5: TNShow()");
    printf("\r\n 6: TCRShow()");
    printf("\r\n 7: TDRShow()");

	printf("\r\nPlease Enter a choice:");
}


int main(void)
{
#ifndef NDEBUG
    if (!CObject::InitObjectClass())
        return false;
#endif

    if (!CComEntity::InitComEntityClass())
        return false;
    
    if (!CTimerTask::InitTimerTask())
        return false;

#ifndef __NUCLEUS__
    if (!CTransaction::InitTransactionClass())
        return false;
#endif

    init_pcap_vars();

	CPersonator* pPersonator = CPersonator::GetInstance();
    pPersonator->Begin();

	CTaskL3L2Tunnel *taskl3l2Tunnel = CTaskL3L2Tunnel::GetInstance();
	taskl3l2Tunnel->Begin();

	CTaskL3L2Tcr *taskl3l2Tcr = CTaskL3L2Tcr::GetInstance();
	taskl3l2Tcr->Begin();

	CTaskCPESM *taskCPESM = CTaskCPESM::GetInstance();
	taskCPESM->Begin();

	StartTaskUTDM();

	//printf(pWelCome);

	CMDLine();

	while(true)
	{
		Sleep(10000);
	}

#if 0
    char ch;
    printf("\r\ncommand:\\>");
    while (  ch = _getche() )
    {
        switch (ch)
            {
            case '?':
            case 'h':
                ITusage();
                break;

            case '1':
                IT_EB();
                break;

            default:
                break;
            }

        printf("\r\ncommand:\\>");
    }
#endif
	return true;
}


extern "C" void UTDMShow();

void CMDLine()
{
    char ch;
    ITusage();

    while( ch = _getche() )
        {
        switch(ch)
            {
            case '?':
            case 'h':
				ITusage();
                break;

            case '3':
                UTDMShow();
                break;

            default:
                break;
            }
        printf("\r\n");
        printf("\r\ncommand:\\IT\\>");
        }
}

