#include "L3DataEB.h"
#include "L3DataSnoop.h"
#include "L3DataTunnel.h"
#include "L3DataTDR.h"
#include "L3DataTCR.h"
#include "L3DataDM.h"
#include "L3DataARP.h"
#include "DAC.h"
#include "stdhdr.h"
#include "Personator.h"
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include "timertask.h"
#include "Transaction.h"

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

    CPersonator* pPersonator = CPersonator::GetInstance();
    pPersonator->Begin();

	CTaskDAC *taskDAC = CTaskDAC::GetInstance();
	taskDAC->Begin();

	CTBridge *taskEB = CTBridge::GetInstance();
	taskEB->Begin();

	CTSnoop *taskSnoop = CTSnoop::GetInstance();
	taskSnoop->Begin();

	CTunnel *taskTunnel = CTunnel::GetInstance();
	taskTunnel->Begin();

	CTaskTDR *taskTDR = CTaskTDR::GetInstance();
	taskTDR->Begin();

	CTaskTCR *taskTCR = CTaskTCR::GetInstance();
	taskTCR->Begin();

	CTaskDm *taskDM = CTaskDm::GetInstance();
	taskDM->Begin();

	CTaskARP *taskARP = CTaskARP::GetInstance();
	taskARP->Begin();

	StartTaskUTDM();

	printf(pWelCome);

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


extern "C" void EBShow();
extern "C" void SNShow();
extern "C" void DMShow();
extern "C" void UTDMShow();
extern "C" void ARPShow();
extern "C" void TunnelShow();
extern "C" void TCRShow();
extern "C" void TDRShow();

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

            case '0':
                EBShow();
                break;
            case '1':
                SNShow();
                break;
            case '2':
                DMShow();
                break;
            case '3':
                UTDMShow();
                break;
            case '4':
                ARPShow();
                break;
            case '5':
                TunnelShow();
                break;
            case '6':
                TCRShow();
                break;
            case '7':
                TDRShow();
                break;

            default:
                break;
            }
        printf("\r\n");
        printf("\r\ncommand:\\IT\\>");
        }
}

