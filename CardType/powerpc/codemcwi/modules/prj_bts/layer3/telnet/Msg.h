/*-----------------------------------------------------------
    MPUMsg.h -  MPUӦ�ó��򹫹�������Ϣ���Ʋ���ͷ�ļ�

    ��Ȩ���� ����������˾

    �޸���ʷ��¼
    --------------------
    2005-11-08,      ��ΰ��      ����
-----------------------------------------------------------*/

#ifndef __MSG_H__
#define __MSG_H__



#ifdef  __cplusplus
extern  "C"
{
#endif

/*****************************************************************************/

#define MAX_MSG_LEN		0x200
#define SYS_MSG_HEAD_LEN   6

/*****************************************************************************/

#pragma pack(1)

/*������Ϣͷ�ṹ*/
typedef struct tagCommonHeader
{
	_UCHAR       ucDstTaskNo;    /*������*/
	_UCHAR	     ucSrcTaskNo;    /*������*/
	_USHORT      usMsgId;        /*��Ϣ��*/
	_USHORT      usMsgLen;       /*��Ϣ����*/
}PKD COMMON_HEADER;
	
/*����Ϣ�ṹ*/
typedef struct tagLongMessage
{
	COMMON_HEADER   stHeader;
	_UCHAR          ucBuffer[MAX_MSG_LEN-sizeof(COMMON_HEADER)];
}PKD LONG_MESSAGE;




#define    IP_FSM_CLI_FIRST_CLIENT    12      /*telnet ��һ���ͻ���*/
#define    IP_FSM_CLI_SERVER   IP_FSM_CLI_FIRST_CLIENT + 7 //�����е����������
#define    MAX_IP_FSM     IP_FSM_CLI_SERVER+1    

/*������������Ƶĺ궨��*/



#define IP_TASK_ID      0x06
#define CLI_TASK_ID     0x0F
#define TASK_NO_CLCK    0x10

#define    MSG_IP_CONFIG                  1        /*����          OAM->IP*/
#define    MSG_IP_ADDRCHANGE        2          /*IP��ַ�ı�   OAM->IP*/
#define    MSG_IP_CONNECTIND	   3	/*����ָʾ      IP->USER*/
#define    MSG_IP_CLOSEREQ              4 /*�ر�����      USER->IP*/
#define    MSG_IP_CLOSEIND              5 /*�ر�ָʾ      IP->USER*/
#define    MSG_IP_DATAREQ               6 /*��������      USER->IP*/
#define    MSG_IP_DATAIND               7 /*����ָʾ      IP->USER*/





#ifdef  __cplusplus
}
#endif

#endif
