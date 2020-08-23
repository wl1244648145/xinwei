
#ifndef __PKG_MAG__H__
#define __PKG_MAG__H__


#define MEM_BLOCK_SIZE			(1600)
#define MEM_BLOCK_NUMBER		(4000)

/*
	初始化缓存数组
*/
int bsp_init_bfpoll(void);

/*
	从缓存数组中申请一块，len要小于MEM_BLOCK_SIZE
*/
void *malloc_buf(unsigned len);

/*
	释放缓存块
*/
int free_buf(unsigned char *addr);


#endif
