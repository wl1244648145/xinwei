/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bbu_config.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
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
/******************************* 头文件结束 ********************************/

