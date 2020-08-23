	

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <memory.h>

#include "../inc/stack.h"
#include "../inc/pkg_mag.h"
#include "../../spinlock/inc/spinlockapi.h"

//#define USB_STD_MALLOC 

static unsigned long long pkg_mem_used = 0;
static T_SpinLock lock;

static unsigned char *bufaddr = NULL;

/*******************************************
*�� �� ��: bsp_init_bfpoll
*��������: �ڴ�س�ʼ��
*��    ��: ��
*��    ��: 0 �ɹ�  -1 ʧ��
*******************************************/
int bsp_init_bfpoll(void){
#ifdef LOCK
	BspSpinLockInit(&lock);
#endif	
	int i = 0;
	//printf("bufaddr_size=%d\n", MEM_BLOCK_SIZE*MEM_BLOCK_NUMBER);
	bufaddr = malloc(MEM_BLOCK_SIZE*MEM_BLOCK_NUMBER);
	if(bufaddr==NULL){
		printf("[%s]malloc error!\n", __func__);
		return -1;
	}
	memset(bufaddr, 0, MEM_BLOCK_SIZE*MEM_BLOCK_NUMBER);
	stack_init(MEM_BLOCK_NUMBER);
	for(i=0;i<MEM_BLOCK_NUMBER;i++){
		stack_push(i);
	}
	return 0;
}

/*******************************************
*�� �� ��: malloc_buf
*��������: ���ڴ��������һ���ڴ�
*��    ��: unsigned int len: �����ڴ泤��
*��    ��: NULL ����ʧ��  ��NULL�����뵽���ڴ��ַ
*******************************************/
void *malloc_buf(unsigned int len){
	int index = 0;	
#ifdef USB_STD_MALLOC
	return malloc(len);
#else
	if(len>MEM_BLOCK_SIZE){	
		printf("error malloc_buf too big, len=%d\n", len);	
		return NULL;
	}	
	
	if(stack_pop(&index)==0){
		pkg_mem_used++;
		return bufaddr + index*MEM_BLOCK_SIZE;
	}else{
		return NULL;
	}
#endif
}

/*******************************************
*�� �� ��: free_buf
*��������: �ͷ��ڴ�飨���ڴ�أ�
*��    ��: unsigned char *addr: �ͷŵ��ڴ���ַ
*��    ��: 0�ɹ�  -1ʧ��
*******************************************/
int free_buf(unsigned char *addr){
#ifdef USB_STD_MALLOC
	free(addr);
	return 0;
#else	
	int index = 0;
	if( (addr<bufaddr) || (addr > bufaddr+MEM_BLOCK_NUMBER*MEM_BLOCK_SIZE) ){
		return -1;
	}
	BspSpinLock(&lock);	
	index = (addr - bufaddr)/MEM_BLOCK_SIZE;
	stack_push(index);
	pkg_mem_used--;
	BspSpinUnLock(&lock);
	return 0;
#endif	
}

