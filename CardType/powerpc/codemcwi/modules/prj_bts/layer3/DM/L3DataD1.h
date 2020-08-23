#ifndef __L3_DATADMCOMMON_H__
#define __L3_DATADMCOMMON_H__

#include "L3dataCommon.h"


//TUNNEL�����붨��
const UINT16 ERR_DM_NORMAL              = 0x0000;  //��������
const UINT16 ERR_DM_UNEXPECTED_MSGID    = 0x0001;  //�Ƿ�Message ID
const UINT16 ERR_DM_SYS_ERR             = 0x0002;  //ϵͳ����

const UINT16 ERR_DM_PARAMETER           = 0x0003;  //��������
const UINT16 ERR_DM_CB_USEDUP           = 0x0004;  //CB�ù�
const UINT16 ERR_DM_CB_INDEX_ERR        = 0x0005;  //CB�±����
const UINT16 ERR_DM_CB_BUSY             = 0x0006;  //���ڽ���Tunnel�������Ե�
const UINT16 ERR_DM_NO_CB               = 0x0007;  //û�ҵ���Ӧ�Ŀ��ƿ�
const UINT16 ERR_DM_CFG_ERR             = 0x0008;  //�������ô���


/******************************
 *OPER: IPlist�ıȽϲ�����
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

//CPEͳ�ư�
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
 *strperformance of DM: ��Ӧ���ַ���
 *����������ͳ��ʱ����Ҫ����20�ֽ�
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
