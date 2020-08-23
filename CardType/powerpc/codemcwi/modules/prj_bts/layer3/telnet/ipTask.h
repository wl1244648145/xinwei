/************************************************************************
*   ��Ȩ���У���������ͨ�Źɷ����޹�˾
*   �ļ�����ipTask.h
*	�ļ�����: IPģ�鳣����������ݽṹ����
*   �޸ļ�¼��
*   	1.  2006.7.5   Li Wei   ����
************************************************************************/
#ifndef _IP_TASK_H_
#define _IP_TASK_H_

//#include "../Public/mpu.h"
#include "SysOs.h"


#ifdef  __cplusplus
extern  "C"
{
#endif


/*���յ���IP�������*/
#define MAX_IP_DATA_LEN                 2090

#define MAX_IP_CCB_NUM             32
#define TELNET_PORT                   32
#define TELNET_MAX_BLOCKED_TIME (240) //240 seconds
#define SIZEOF(slice)         (sizeof(slice)/sizeof((slice)[0]))

/*�ڲ���Ϣ����*/
enum
{
    IP_MSG_TM_CONNECT = 0,
    IP_MSG_INNER_MAX
};

/* socket  ״̬����*/
enum
{
    SYS_IP_STATE_IDLE = 0,              /*����*/
    SYS_IP_STATE_OCCUPY,              /*ռ��*/
    SYS_IP_STATE_OPENED,              /*����*/
    SYS_IP_STATE_LISTENED,            /*���ڼ���*/
    SYS_IP_STATE_CONNECTTING,     /*��������*/
    SYS_IP_STATE_CONNECTTED,      /*������*/
    SYS_IP_STATE_RUNNING,            /*����,���Դ�������*/
    SYS_IP_STATE_MAX
};

enum
{
    SYS_IP_CLASS_SERVER = 0,        /*��������*/
    SYS_IP_CLASS_SERVERCLIENT,    /*�������������Ŀͻ�����*/
    SYS_IP_CLASS_CLIENT,               /*�ͻ���*/
    SYS_IP_CLASS_MAX
};

/*TCP�ͻ������ӵ����ȷ�ʽ*/
typedef enum
{
    TCP_CLIENT_PRIO_SQUENCE = 0,    /*˳��ʽ*/
    TCP_CLIENT_PRIO_FORCE,             /*ǿ��ʽ������ռʽ*/

    MAX_TCP_CLIENT_PRIO
}TCP_CLIENT_PRIO;

/*IPҵ������*/
enum
{
    IP_MAINTAIN=0,     /*ά��������*/
    IP_CONTROL,            /*����ƽ����*/
    IP_USER                  /*�û�ƽ����*/
};

#pragma pack(1)

typedef struct _tagSysIPCcb
{
    _UCHAR                     ucState;        /*״̬*/
    _UCHAR                     ucIndex;     /*CCB�±�*/
    _UCHAR                     ucUser;         /*ҵ��ʹ���ߣ�ָʹ��socket  ������*/
    _UCHAR                     ucProvider;     /*ҵ���ṩ��*/
    _INT                        iType;           /*��ʽ(1:TCP 2:UDP 3:RAW)*/
    _INT                        iProt;            /*Э���*/
    _UINT                       uiClass;        /*����,�����TCP��ʽ,�����ʾ:0=����,1=�������������Ŀͻ�����,2=�ͻ�*/
    _UCHAR                    ucMinCcb, ucMaxCcb;/*�����������SOCKET,���������ӵĿͻ�CCB*/
    _UCHAR                    ucClientPrio;   /*�ͻ���������ʱ�����ȼ���,��ö��[TCP_CLIENT_PRIO](����TCP����ʱ��Ч)*/
    _UINT                       uiTmConn;       /*��������ӵĿͻ���,������Connect���ʱ��,��λΪms*/
    
    SYS_IP_ADDR             stMyAddr;       /*�Լ���IP��ַ*/
    SYS_IP_ADDR              stPeerAddr;     /*�Զ˵�IP��ַ*/
    SYS_SOCKET_ID           sock;           /*SOCKET*/
    //HTIMER                     usTimerId;
}PKD SYS_IP_CCB;

typedef struct tagSysIPData
{

    SYS_SOCKET_ID           maxFd;          //selectʹ�õ�maxFd
    _INT        PipeFd;
    _INT       CliPipeFd;
    fd_set                  fdRead;         //���ļ�������
    fd_set                  fdWrite;        //д�ļ�������
    fd_set                  fdException;    //�쳣�ļ�������
    _UCHAR                  ucBuffer[sizeof(COMMON_HEADER) + MAX_IP_DATA_LEN];   //������Ϣ������
    SYS_IP_CCB              stCcb[MAX_IP_CCB_NUM];//���ƿ�
}PKD SYS_IP_DATA;



typedef struct tagSysIPTCPConfig
{
    _UINT                   uiClass;        /*�����,�����TCP��ʽ,�����ʾ:R3_SYS_IP_CLASS_SERVER=����,R3_SYS_IP_CLASS_CLIENT=�ͻ�*/
    _UINT                   uiTmConn;       /*�����TCP�Ŀͻ���,�����ʾconnect��ʱ���,��λΪms*/
    _UCHAR                 ucMinCcb;       /*�����TCP�ķ�������,�����ʾ���Կ��Ƶ���С��CCB�±�*/
    _UCHAR                  ucMaxCcb;       /*�����TCP�ķ�������,�����ʾ���Կ��Ƶ�����CCB�±�*/
    _UCHAR                  ucClientPrio;   /*�ͻ���������ʱ�����ȼ���,��ö��[TCP_CLIENT_PRIO]*/
}PKD SYS_IP_TCP_CONFIG;

typedef struct tagSysIPConfig
{    
    _UCHAR               ucCcbIndex;     /*CCB�±�*/
    _UCHAR               ucUser;         /*ҵ��ʹ���ߣ�ָsocket ʹ������*/
    _INT                    iType;          /*SOCKET����,R3_SYS_IP_TYPE_TCP/R3_SYS_IP_TYPE_UDP/R3_SYS_IP_TYPE_RAW*/
    _INT                    iProt;          /*Э���*/
    
    SYS_IP_ADDR             stMyAddr;       /*�Լ���IP��ַ*/
    SYS_IP_ADDR             stPeerAddr;     /*�Զ�IP��ַ*/
    SYS_IP_TCP_CONFIG       stTCPCfg;       /*�����TCP�ڵ�,�����������*/
}PKD SYS_IP_CONFIG;

#pragma pack()


#define SYS_IPFdSet(sock, set) \
{ \
    if (sock >= m_pTelnet->maxFd) \
        m_pTelnet->maxFd = sock + 1; \
    FD_SET(sock, set);\
}





_INT SYS_IPStart(_USHORT usPort);
_INT SYS_MsgSend2IP(LONG_MESSAGE *pstMsg);

//Telnet ���
extern  _ULONG CLI_IniProc();
extern _INT SendMsgTelnet(LONG_MESSAGE *pstMsg);


#ifdef  __cplusplus
}
#endif
#endif