/*****************************************************************************
             (C) Copyright 2005: Arrowping Networks, Inc.
                    Arrowping Confidential Proprietary
*****************************************************************************/

/*----------------------------------------------------------------------------
 * FILENAME:    L3DataAPI.cpp
 *
 * DESCRIPTION: 
 *
 * HISTORY:
 *
 *   Date       Author         Description
 *   ---------  ------        ----------------------------------------------------
 *   03/14/06   xiao weifang  initialization. 
 *
 *---------------------------------------------------------------------------*/

#include "L3DataAPI.h"
#include "L3DataEB.h"
#include "L3DataDM.h"
#include "L3DataSnoop.h"
#include "L3DataTunnel.h"
#include "L3DataARP.h"
#include "L3DataSocket.h"
#include "PciIf.h"
#include "utAgent.h"

#include "L2PciShell.h"

#include "L3dataWANIF.h"
bool InitL3DataSvc()
{
#ifndef __WIN32_SIM__
    CTaskPciIf *taskPciIf =  CTaskPciIf::GetInstance();
    taskPciIf->Begin();

    CUtAgent *utAgent = CUtAgent::GetInstance();

    CTBridge *taskEB = CTBridge::GetInstance();
    taskEB->Begin();

    CTaskARP *taskARP = CTaskARP::GetInstance();
    taskARP->Begin(); 

    CTunnel *taskTunnel = CTunnel::GetInstance();
    taskTunnel->Begin();

	CTaskDm *taskDM = CTaskDm::GetInstance();
    taskDM->Begin();

    CTSnoop *taskSnoop = CTSnoop::GetInstance();
    taskSnoop->Begin();

#ifndef WBBU_CODE
    CL2Shell *taskL2Shell = CL2Shell::GetInstance();
    taskL2Shell->Begin();
#endif
 #ifdef M_TGT_WANIF
   CTWANIF *taskWANIF = CTWANIF::GetInstance();
    taskWANIF->Begin();
 #endif
#endif
    return true;
}



/*============================================================
MEMBER FUNCTION:
    GetDataPerfData

DESCRIPTION:
    数据模块性能统计接口函数

ARGUMENTS:
    ucType: 性能类别，EB/DM/SNOOP/...
    UINT8*: 

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
void GetDataPerfData(UINT8 ucType, UINT8 *pData)
{
    if ( NULL == pData )
        {
        return;
        }

    switch( ucType )
        {
        case BTS_PERF_TYPE_EB:
            CTBridge::GetInstance()->GetPerfData(pData);
            break;
        case BTS_PERF_TYPE_SNOOP:
            CTSnoop::GetInstance()->GetPerfData(pData);
            break;
        case BTS_PERF_TYPE_DM:
            CTaskDm::GetInstance()->GetPerfData(pData);
            break;
        case BTS_PERF_TYPE_TUNNEL:
            CTunnel::GetInstance()->GetPerfData(pData);
            break;
        case BTS_PERF_TYPE_ARP:
            CTaskARP::GetInstance()->GetPerfData(pData);
            break;
		case BTS_PERF_TYPE_TCR:
		case BTS_PERF_TYPE_TDR:
			CSOCKET::GetInstance()->GetPerfData(pData,ucType);
			break;
/*            
        case BTS_PERF_TYPE_TCR:
            CTaskTCR::GetInstance()->GetPerfData(pData);
            break;
        case BTS_PERF_TYPE_TDR:
            CTaskTDR::GetInstance()->GetPerfData(pData);
            break;
*/           
        }

    return;
}



/*============================================================
MEMBER FUNCTION:
    clear

DESCRIPTION:
    根据Data任务清除性能数据

ARGUMENTS:
    ucType: 性能类别，EB/DM/SNOOP/...

RETURN VALUE:
    void 

SIDE EFFECTS:
    none
==============================================================*/
extern "C" int getSTDNum(int*);
void perfclear(UINT8 ucType)
{
    UINT32 ulEid = 0;
    switch( ucType )
        {
        case 0:
            //没有输入参数.
            printf( "\r\nclear 1:   clear EB     performance data." );
            printf( "\r\nclear 2:   clear SNOOP  performance data." );
            printf( "\r\nclear 3:   clear DM     performance data." );
            printf( "\r\nclear 4:   clear TUNNEL performance data." );
            printf( "\r\nclear 5:   clear ARP    performance data." );
            printf( "\r\nclear 6:   clear SOCKET performance data." );
            //printf( "\r\nclear 7:   clear TDR    performance data." );
            printf( "\r\nclear 255: clear ALL    performance data." );
            printf( "\r\n" );
            break;
        case 255:/*clear a*/
            CTBridge::GetInstance()->ClearMeasure();
            CTSnoop::GetInstance()->ClearMeasure();
            CTaskDm::GetInstance()->ClearMeasure( 0 );
            CTunnel::GetInstance()->ClearMeasure();
            CTaskARP::GetInstance()->ClearMeasure();
			CSOCKET::GetInstance()->ClearMeasure();
/*            
            CTaskTCR::GetInstance()->ClearMeasure();
            CTaskTDR::GetInstance()->ClearMeasure();
*/            
            break;
        case BTS_PERF_TYPE_EB+1:
            printf( "\r\nclear EB performance data...", ulEid, ulEid );
            CTBridge::GetInstance()->ClearMeasure();
            break;
        case BTS_PERF_TYPE_SNOOP+1:
            printf( "\r\nclear SNOOP performance data...", ulEid, ulEid );
            CTSnoop::GetInstance()->ClearMeasure();
            break;
        case BTS_PERF_TYPE_DM+1:
            printf("\r\nPlease Enter EID:");
            getSTDNum((int*)&ulEid);
            printf( "\r\nclear EID[ = %d = 0x%x] perf data...", ulEid, ulEid );
            CTaskDm::GetInstance()->ClearMeasure( ulEid );
            break;
        case BTS_PERF_TYPE_TUNNEL+1:
            printf( "\r\nclear TUNNEL performance data..." );
            CTunnel::GetInstance()->ClearMeasure();
            break;
        case BTS_PERF_TYPE_ARP+1:
            printf( "\r\nclear ARP performance data..." );
            CTaskARP::GetInstance()->ClearMeasure();
            break;
           
        case BTS_PERF_TYPE_TCR+1:
            printf( "\r\nclear SOCKET performance data..." );
            CSOCKET::GetInstance()->ClearMeasure();
            break;
/*
        case BTS_PERF_TYPE_TDR+1:
            printf( "\r\nclear TDR performance data...", ulEid, ulEid );
            CTaskTDR::GetInstance()->ClearMeasure();
            break;
*/            
        }

    return;
}

void GetDataPerfDataNew(UINT8 ucType, UINT32 *pData)
{
    if(BTS_PERF_TYPE_EB == ucType)
    {
        CTBridge::GetInstance()->GetPerfDataNew(pData);
    }
}
void ClearDataPerfDataNew(UINT8 ucType)
{
    if(BTS_PERF_TYPE_EB == ucType)
    {
        CTBridge::GetInstance()->ClearPerfDataNew();
    }
}
