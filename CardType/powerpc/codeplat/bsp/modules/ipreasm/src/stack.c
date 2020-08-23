
#include <stdio.h>
#include <stdlib.h>
#include "../inc/stack.h"

static unsigned int *pStack = NULL;
static int current_pos = 0;
static int StackSize = 0;

/*******************************************
*函 数 名: stack_init
*功能描述: 初始化栈
*参    数: unsigned int size： 栈大小
*返    回: 0 成功  -1 失败
*******************************************/
int stack_init(unsigned int size){
	pStack = (unsigned int *)malloc(size*sizeof(unsigned int));
	if(pStack == NULL){
		printf("[%s]: stack_init error!\r\n", __func__);
		return STACK_ERROR;
	}
	StackSize = size;
	current_pos = 0;
	return STACK_OK;
}

/*******************************************
*函 数 名: stack_push
*功能描述: 入栈
*参    数: int value：入栈数据
*返    回: 0 成功  -1 失败
*******************************************/
int stack_push(int value){
	if(pStack == NULL){
		printf("[%s] Error! stack == NULL\r\n", __func__);	
		return STACK_ERROR;
	}
	if(current_pos == StackSize){
		printf("[%s] Error! stack is full!\r\n", __func__);	
		return STACK_ERROR;
	}
	pStack[current_pos] = value;
	current_pos++;
	return STACK_OK;
}
/*******************************************
*函 数 名: stack_pop
*功能描述: 出栈
*参    数: int *pValue：存放出栈数据的地址
*返    回: 0 成功  -1 失败
*******************************************/
int stack_pop(int *pValue){
	if(pStack==NULL){
		printf("[%s] Error! stack == NULL\r\n", __func__);		
		return STACK_ERROR;
	}
	if(current_pos == 0){
		printf("[%s] Error! stack is empty\r\n", __func__);		
		return STACK_ERROR;		
	}
	current_pos--;
	*pValue = pStack[current_pos];	
	return STACK_OK;
}
