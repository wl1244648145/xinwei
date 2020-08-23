#ifndef __L3_DATADMCOMMON_H__
#define __L3_DATADMCOMMON_H__

#include "L3dataCommon.h"


//TUNNEL错误码定义
const UINT16 ERR_DM_NORMAL              = 0x0000;  //正常流程
const UINT16 ERR_DM_UNEXPECTED_MSGID    = 0x0001;  //非法Message ID
const UINT16 ERR_DM_SYS_ERR             = 0x0002;  //系统错误

const UINT16 ERR_DM_PARAMETER           = 0x0003;  //参数错误
const UINT16 ERR_DM_CB_USEDUP           = 0x0004;  //CB用光
const UINT16 ERR_DM_CB_INDEX_ERR        = 0x0005;  //CB下标错误
const UINT16 ERR_DM_CB_BUSY             = 0x0006;  //正在进行Tunnel操作，稍等
const UINT16 ERR_DM_NO_CB               = 0x0007;  //没找到对应的控制块
const UINT16 ERR_DM_CFG_ERR             = 0x0008;  //数据配置错误


/******************************
 *OPER: IPlist的比较操作符
 ******************************/
typedef enum 
    {
    EQ = 0,
    SIMIEQ,
    NEQ
    }OPER;


/*************************************************
 *(TLV)->IPTYPE: IPLIST
 *************************************************/
typedef enum 
    {
    DYNIPLIST,
    FIXIPLIST
    }IPLIST_TYPE;

typedef enum 
    {
    STATUS_IDLE = 0,
    STATUS_PEND,
    STATUS_ACL
   // STATUS_CONFIG

    }ACLDB_STATUS;

typedef enum 
    {
    M_SYNC_ADD_BOTH= 0,
    M_SYNC_DEL_BOTH,
    M_SYNC_UPD_IPONLY,
    M_SYNC_DEL_IPONLY

    }SYNC_OPER;

//CPE统计包
typedef enum 
    {
    DATA_SERVICE_REQ= 0,
    DATA_SERVICE_RSP,
    SEND_SYNC_ACL_REQ,
    RECV_SYNC_ACL_RSP,
    SEND_SYNC_IP_REQ,
    RECV_SYNC_IP_RSP,
    SYNC_ACL_FAIL,
    SYNC_IP_FAIL,
    CPE_CONIFG,
    MAX_PERFMC

    }CPE_PERFORMANCE;
/*************************************************
 *strperformance of DM: 对应的字符串
 *增加新性能统计时，不要超过20字节
 *************************************************/
#define M_PERFORM_STRLEN    (22)
const UINT8 strCPEPerformance[MAX_PERFMC][M_PERFORM_STRLEN] = {
    "Rx data service req",
    "Tx data service resp",
    "Tx sync ACL req",
    "Rx sync ACL resp",
    "Tx sync DAIB req",
    "Rx sync DAIB resp",
    "Sync ACL  timeout",
    "Sync DAIB timeout",
    "System config",
};

#endif
