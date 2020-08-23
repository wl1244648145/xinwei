 
#ifndef HEADER_BSPQMAN_H
#define HEADER_BSPWMAN_H

/***********************************************************
 *                    其它条件编译选项                     *
***********************************************************/
/***********************************************************
 *                   标准、非标准头文件                    *
***********************************************************/
/***********************************************************
 *                        常量定义                         *
***********************************************************/

/***********************************************************
 *                       全局宏                            *
***********************************************************/

#include "TBuf.h"
#include "compat.h"


 
//#define CONFIG_SYS_QMAN_MEM_BASE	(unsigned long)0x71600000
#define CONFIG_SYS_QMAN_MEM_BASE	0xff4200000

#define QMAN_STASH_CTX_CL(p) \
({ \
	__always_unused const typeof(*(p)) *foo = (p); \
	int foolen = sizeof(*foo) / 64; \
	if (foolen > 3) \
		foolen = 3; \
	foolen; \
})



#define QMAN_CCSR_OFFSET  0x318000

#define  FQD_BARE           0xC00    /*     Extended Base Address Register R/W 0x0000_0000                                         */ 
#define  FQD_BAR         0xC04    /*    Queue Descriptor (FQD) Base Address Register R/W 0x0000_0000 6.3.4.39/6-83              */  
#define  FQD_AR          0xC10    /*        Attributes Register R/W 0x0000_0000 6.3.4.40/6-86                                   */  
#define  PFDR_BARE       0xC20    /*       Extended Base Address Register R/W 0x0000_0000 6.3.4.39/6-83                         */  
#define  PFDR_BAR         0xC24   /*          —Packed Frame Descriptor Record (PFDR) Base Addr R/W 0x0000_0000 6.3.4.39/6-83   */  
#define  PFDR_AR          0xC30   /*        —PFDR Attributes Register R/W 0x0000_0000 6.3.4.40/6-86                            */  
#define  QCSP_BARE        0xC80   /*        —QCSP Extended Base Address R/W 0x0000_0000 6.3.4.41/6-88                          */  
#define  QCSP_BAR         0xC84   /*    —QMan Software Portal Base Address R/W 0x0000_0000 6.3.4.41/6-88                       */  
#define  CI_SCHED_CFG     0xD00   /*        —Initiator Scheduling Configuration R/W 0x0000_0000 6.3.4.42/6-89                  */  
#define  QMAN_SRCIDR      0xD04   /*       —QMan Source ID Register R 0x0000_003C 6.3.4.43/6-90                                */  
#define  QMAN_LIODNR      0xD08   /*    —QMan Logical I/O Device Number Register R/W 0x0000_0000 6.3.4.44/6-91                 */  
#define  CI_RLM_CFG       0xD10   /*  —Initiator Read Latency Monitor Configuration R/W 0x0000_0000 6.3.4.45/6-91              */  
#define  CI_RLM_AVG       0xD14    /*   —Initiator Read Latency Monitor Average R/W 0x0000_0000 6.3.4.46/6-92                   */ 


#define  QMAN_MCR     0xB00   /*      —QMan Management Command/Result Register R/W 0x0000_0000 6.3.4.32/6-78      */
#define  QMAN_MCP0    0xB04   /*      —QMan Management Command Parameter 0 Register R/W 0x0000_0000 6.3.4.33/6-79 */
#define  QMAN_MCP1    0xB08   /*      —QMan Management Command Parameter 1 Register R/W 0x0000_0000 6.3.4.34/6-80 */


/* Assists for QMAN_MCR */
#define MCR_INIT_PFDR		0x01000000
#define MCR_get_rslt(v)		(u8)((v) >> 24)
#define MCR_rslt_idle(r)	(!rslt || (rslt >= 0xf0))
#define MCR_rslt_ok(r)		(rslt == 0xf0)
#define MCR_rslt_eaccess(r)	(rslt == 0xf8)
#define MCR_rslt_inval(r)	(rslt == 0xff)

#endif /* HEADER_BSPQMAN_H */




