
#ifndef __BSP_STACK_H__
#define __BSP_STACK_H__

#include <pthread.h> 

#define STACK_OK		0
#define STACK_ERROR		-1

typedef struct{
	unsigned int *data;
	int stack_size;
	int current_pos;
    int current_used;
    pthread_mutex_t lock;
}bsp_stack_t;
/*******************************************
*函 数 名: stack_init
*功能描述: 初始化栈
*参    数: unsigned int size： 栈大小
*返    回: 0 成功  -1 失败
*******************************************/
int stack_init(unsigned int size);

/*******************************************
*函 数 名: stack_push
*功能描述: 入栈
*参    数: int value：入栈数据
*返    回: 0 成功  -1 失败
*******************************************/
int stack_push(int value);

/*******************************************
*函 数 名: stack_pop
*功能描述: 出栈
*参    数: int *pValue：存放出栈数据的地址
*返    回: 0 成功  -1 失败
*******************************************/
int stack_pop(int *pValue);

#endif //__BSP_STACK_H__
