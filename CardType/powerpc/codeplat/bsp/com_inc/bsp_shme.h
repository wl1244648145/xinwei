/*******************************************************************************
*  COPYRIGHT XINWEI Communications Equipment CO.,LTD
********************************************************************************
* 源文件名:           bbu_config.h 
* 功能:                  
* 版本:                                                                  
* 编制日期:                              
* 作者:                                              
*******************************************************************************/
#ifndef BSP_SHMEM_EXT_H
#define BSP_SHMEM_EXT_H

Export s32 bsp_shmem_init(void);
typedef struct tagtT_ShmemRegInfo
{
	unsigned char  *pName;
	WORD32         index;
    WORD32         dwPhyAddr;
	WORD32         dwVirAddr;
	WORD32         dwLen;
	unsigned char  isFlag;
}T_ShmemRegInfo;


#define SHMEM_RESERVE_BANK (1<<20)
#define DPASHMEMSIZE       (13<<20) 
#define BMPOOLSHMEMSIZE    (13<<20) 
#define P2PSHMEMSIZE       (40<<20)
#define SECSHMEMSIZ	      (1 << 20)
#define UBPOOLSIZE        (500<<20)
static T_ShmemRegInfo gaUserShmemConfigTable[] =
{
	{"dpaheap",  0,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
	{"dpaheap",  1,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
	{"dpaheap",  2,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
	{"dpaheap",  3,   0,   0,   DPASHMEMSIZE    + SHMEM_RESERVE_BANK,    TRUE},
	{"bmpool",   0,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
	{"bmpool",   1,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
	{"bmpool",   2,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
	{"bmpool",   3,   0,   0,   BMPOOLSHMEMSIZE + SHMEM_RESERVE_BANK,    TRUE},
	{"p2p",      0,   0,   0,   P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,    FALSE},
	{"p2p",      1,   0,   0,   P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,    FALSE},
	{"p2p", 	 2,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  FALSE},
	{"p2p", 	 3,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  FALSE},
	{"p2p", 	 4,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  FALSE},
	{"p2p", 	 5,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  FALSE},
	{"p2p", 	 6,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  FALSE},
	{"p2p", 	 7,   0,   0,	P2PSHMEMSIZE    + SHMEM_RESERVE_BANK,	  FALSE},
    {"sec", 	 0,   0,   0,	SECSHMEMSIZ	    + SHMEM_RESERVE_BANK,	  TRUE},
    {"ubpool",   0,   0,   0,   UBPOOLSIZE     + SHMEM_RESERVE_BANK,     TRUE},
};


#endif
/******************************* 头文件结束 ********************************/

