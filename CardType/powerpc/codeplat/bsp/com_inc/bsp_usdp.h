/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* Դ�ļ���:           bbu_config.h 
* ����:                  
* �汾:                                                                  
* ��������:                              
* ����:                                              
*******************************************************************************/
#ifndef BSP_USDPAA_EXT_H
#define BSP_USDPAA_EXT_H


#define MAXSIZE          (1024*10)
#define MAX_BUF_LEN      (8*1024)

typedef struct Tag_Queue
{
    unsigned int qu[MAXSIZE];
    unsigned int sizelen[MAXSIZE];
	unsigned int front;
	unsigned int rear;
	unsigned int tag;
}Queue;
#endif
/******************************* ͷ�ļ����� ********************************/

