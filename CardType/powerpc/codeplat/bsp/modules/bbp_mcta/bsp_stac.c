
#include <stdio.h>
#include <stdlib.h>

#include "bsp_stack.h"

/*******************************************
*函 数 名: bsp_stack_init
*功能描述: 初始化栈
*参    数: bsp_stack_t *pStack： 栈
*		   unsigned int size： 栈大小
*返    回: 0 成功  -1 失败
*******************************************/
int bsp_stack_init(bsp_stack_t *pStack, unsigned int size){    
	if(pStack==NULL){
		printf("[%s]: pStack is NULL!\r\n", __func__);
		return STACK_ERROR;
	}
    if(pStack->data==NULL){
		pthread_mutex_init(&pStack->lock, NULL);
        pStack->data = (unsigned int *)malloc(size*sizeof(unsigned int));
        if(pStack->data == NULL){
            printf("[%s]: stack_init error!\r\n", __func__);
            return STACK_ERROR;
        }    
    }    
	pStack->stack_size = size;
	pStack->current_pos = 0;	
	return STACK_ERROR;
}
/*******************************************
*函 数 名: bsp_stack_push
*功能描述: 入栈
*参    数: bsp_stack_t *pStack： 栈
*		   int value：入栈数据
*返    回: 0 成功  -1 失败
*******************************************/
int bsp_stack_push(bsp_stack_t *pStack, int value){
	if( (pStack == NULL) || (pStack->data == NULL)){
		printf("[%s] Error! pStack=%p or stack data=%p!\r\n", __func__, pStack, pStack->data);
		return STACK_ERROR;
	}
    pthread_mutex_lock(&pStack->lock);
	if(pStack->current_pos == pStack->stack_size){
		printf("[%s] Error! stack is full!\r\n", __func__);	
        pthread_mutex_unlock(&pStack->lock);
		return STACK_ERROR;
	}
	pStack->data[pStack->current_pos] = value;
    //printf("stack release:%d, value=%d\r\n", pStack->current_pos, value);
	pStack->current_pos++;
    pthread_mutex_unlock(&pStack->lock);
	return STACK_OK;
}
/*******************************************
*函 数 名: bsp_stack_pop
*功能描述: 出栈
*参    数: bsp_stack_t *pStack： 栈
*          int *pValue：存放出栈数据的地址
*返    回: 0 成功  -1 失败
*******************************************/
int bsp_stack_pop(bsp_stack_t *pStack, int *pValue){
	if( (pStack == NULL) || (pStack->data == NULL)){
		printf("[%s] Error! pStack=%p or stack data=%p!\r\n", __func__, pStack, pStack->data);
		return STACK_ERROR;
	}
    pthread_mutex_lock(&pStack->lock);
	if(pStack->current_pos == 0){
		printf("[%s] Error! stack is empty\r\n", __func__);		
        pthread_mutex_unlock(&pStack->lock);
		return STACK_ERROR;		
	}
	pStack->current_pos--;
	*pValue = pStack->data[pStack->current_pos];	
    //printf("stack used:%d, value=%d\r\n", pStack->current_pos, *pValue);
    pthread_mutex_unlock(&pStack->lock);
	return STACK_OK;
}
void bsp_stack_print(bsp_stack_t *pStack, int count){    
    int pos = 0;
	if( (pStack == NULL) || (pStack->data == NULL)){
		printf("[%s] Error! pStack=%p or stack data=%p!\r\n", __func__, pStack, pStack->data);
		return STACK_ERROR;
	}
    pthread_mutex_lock(&pStack->lock);
    pos = pStack->current_pos-1;
    while(pos >= 0 && count-- > 0){
        printf("pos %d: %d\r\n", pos, pStack->data[pos]);
        pos--;
    }
    pthread_mutex_unlock(&pStack->lock);
	return STACK_OK;
}