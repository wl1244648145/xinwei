#ifndef __DIAG_DEF_H__
#define __DIAG_DEF_H__

#include "dataType.h"
#pragma pack(1)
typedef enum
{
    CPU_L2 = 0,
    CPU_AUX,
    CPU_MCP,
#ifndef WBBU_CODE
    CPU_FEP
#else
    CPU_FEP,
    CPU_CORE9
#endif
}DIAG_CPU_TYPE;


typedef struct 
{
    UINT16   TransactionId;
    UINT16   cpuType;
    UINT16   cpuInstance;
}T_DiagGeneral;


typedef struct 
{
    T_DiagGeneral  cpuInfo;
    UINT32         peekAddress;
    UINT32         peekLength;
}T_DiagMemoryPeekCmd;
    
typedef struct 
{
    T_DiagGeneral cpuInfo;
    UINT32        peekAddress;
    UINT32        peekLength;
    UINT32        MemroyValue[1];
}T_DiagMemoryPeekResp;


typedef struct 
{
    T_DiagGeneral cpuInfo;
    UINT32        pokeAddress;
    UINT32        setValue;
}T_DiagMemoryPoke;


typedef struct
{
    T_DiagGeneral cpuInfo;
    UINT32        verStrLen;
    char          verStr[1];
}T_DiagSwVersion;


typedef struct
{
    T_DiagGeneral cpuInfo;
    UINT32        statusDataLen;
    UINT32        statusData[1];
}T_DiagStatusQueryResp;


typedef struct 
{
    T_DiagGeneral cpuInfo;
    UINT32        RpcIndex;
    UINT32        arg0;
    UINT32        arg1;
}T_DiagRpcReq;

typedef struct
{
    T_DiagGeneral cpuInfo;
    UINT32        result;
}T_DiagRpcResult;

typedef struct
{
    T_DiagGeneral cpuInfo;
    UINT32        displayInfoLen;
    char *        displayInfo[1];
}T_DiagDisplayInfo;


typedef struct
{
    UINT32    ComMsgNodeNum;
    UINT32    CpuOccupencyPeak;
    UINT32    CpuOccupencyAverage;
}T_DiagL3Status;

#pragma pack()
#endif  // __DIAG_DEF_H__
